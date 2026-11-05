#include "SHVSubsystem.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "IMediaCaptureSupport.h"
#include "MediaCaptureSupport.h"
#include "SHVSettings.h"
#include "SHVTypes.h"
#include "SHVVoiceActor.h"
#include "SHVVoiceComponent.h"

void USHVSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const USHVSettings* Settings = GetDefault<USHVSettings>();
	InputVolume = Settings ? Settings->DefaultInputVolume : 1.0f;
	OutputVolume = Settings ? Settings->DefaultOutputVolume : 1.0f;
	SelectedAudioInputDevice = Settings ? Settings->DefaultInputDeviceName : FString();
	SelectedAudioOutputDeviceId = Settings ? Settings->DefaultOutputDeviceId : FString();
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &USHVSubsystem::HandleWorldCleanup);

	UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("Voice subsystem initialized."));

	if (!SelectedAudioOutputDeviceId.IsEmpty())
	{
		SetAudioOutputDevice(SelectedAudioOutputDeviceId, FSHVOnAudioOutputDeviceSet());
	}
}

void USHVSubsystem::Deinitialize()
{
	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}

	CleanupRuntimeState();
	UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("Voice subsystem deinitialized."));
	Super::Deinitialize();
}

void USHVSubsystem::RegisterLocalVoiceActor(ASHVVoiceActor* VoiceActor)
{
	if (!IsValid(VoiceActor))
	{
		return;
	}

	LocalVoiceActor = VoiceActor;
	UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("Registered local voice actor: %s"), *VoiceActor->GetName());
}

void USHVSubsystem::UnregisterLocalVoiceActor(ASHVVoiceActor* VoiceActor)
{
	if (LocalVoiceActor.Get() == VoiceActor)
	{
		LocalVoiceActor.Reset();
		UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("Unregistered local voice actor."));
	}
}

void USHVSubsystem::RegisterServerVoiceActor(ASHVVoiceActor* VoiceActor)
{
	if (!IsValid(VoiceActor))
	{
		return;
	}

	ServerVoiceActors.RemoveAll([](const TWeakObjectPtr<ASHVVoiceActor>& Entry)
	{
		return !Entry.IsValid();
	});

	if (!ServerVoiceActors.Contains(VoiceActor))
	{
		ServerVoiceActors.Add(VoiceActor);
		UE_LOG(LogSelfHostedVoiceChat, Verbose, TEXT("Registered server voice actor: %s"), *VoiceActor->GetName());
	}
}

void USHVSubsystem::UnregisterServerVoiceActor(ASHVVoiceActor* VoiceActor)
{
	ServerVoiceActors.Remove(VoiceActor);
}

ASHVVoiceActor* USHVSubsystem::GetLocalVoiceActor() const
{
	return LocalVoiceActor.Get();
}

ASHVVoiceActor* USHVSubsystem::FindVoiceActorForController(APlayerController* PlayerController) const
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}

	for (const TWeakObjectPtr<ASHVVoiceActor>& Entry : ServerVoiceActors)
	{
		ASHVVoiceActor* Actor = Entry.Get();
		if (IsValid(Actor) && Actor->GetOwner() == PlayerController)
		{
			return Actor;
		}
	}

	return nullptr;
}

TArray<ASHVVoiceActor*> USHVSubsystem::GetServerVoiceActors() const
{
	TArray<ASHVVoiceActor*> Result;
	for (const TWeakObjectPtr<ASHVVoiceActor>& Entry : ServerVoiceActors)
	{
		if (ASHVVoiceActor* Actor = Entry.Get())
		{
			Result.Add(Actor);
		}
	}
	return Result;
}

void USHVSubsystem::MutePlayer(APlayerState* PlayerState)
{
	if (IsValid(PlayerState))
	{
		MutedPlayerIds.Add(PlayerState->GetPlayerId());
	}
}

void USHVSubsystem::UnmutePlayer(APlayerState* PlayerState)
{
	if (IsValid(PlayerState))
	{
		MutedPlayerIds.Remove(PlayerState->GetPlayerId());
	}
}

bool USHVSubsystem::IsPlayerMuted(APlayerState* PlayerState) const
{
	return IsValid(PlayerState) && MutedPlayerIds.Contains(PlayerState->GetPlayerId());
}

void USHVSubsystem::SetDeafened(bool bInDeafened)
{
	bDeafened = bInDeafened;
}

void USHVSubsystem::SetInputVolume(float InVolume)
{
	InputVolume = FMath::Max(0.0f, InVolume);
}

void USHVSubsystem::SetOutputVolume(float InVolume)
{
	OutputVolume = FMath::Max(0.0f, InVolume);
}

void USHVSubsystem::GetAudioInputDevices(TArray<FSHVAudioInputDevice>& OutDevices) const
{
	OutDevices.Reset();

#if PLATFORM_WINDOWS
	TArray<FMediaCaptureDeviceInfo> DeviceInfos;
	MediaCaptureSupport::EnumerateAudioCaptureDevices(DeviceInfos);
	for (const FMediaCaptureDeviceInfo& DeviceInfo : DeviceInfos)
	{
		FSHVAudioInputDevice Device;
		Device.DisplayName = DeviceInfo.DisplayName.ToString();
		Device.DeviceName = Device.DisplayName;
		Device.Info = DeviceInfo.Info;
		OutDevices.Add(MoveTemp(Device));
	}
#endif
}

bool USHVSubsystem::SetAudioInputDevice(const FString& DeviceName)
{
	TArray<FSHVAudioInputDevice> Devices;
	GetAudioInputDevices(Devices);

	bool bValidDevice = DeviceName.IsEmpty();
	for (const FSHVAudioInputDevice& Device : Devices)
	{
		if (Device.DeviceName == DeviceName || Device.DisplayName == DeviceName)
		{
			bValidDevice = true;
			break;
		}
	}

	if (!bValidDevice)
	{
		UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Audio input device '%s' was not found. Keeping current device '%s'."), *DeviceName, *SelectedAudioInputDevice);
		return false;
	}

	SelectedAudioInputDevice = DeviceName;
	if (ASHVVoiceActor* VoiceActor = LocalVoiceActor.Get())
	{
		if (VoiceActor->VoiceComponent)
		{
			VoiceActor->VoiceComponent->UseInputDevice(SelectedAudioInputDevice);
		}
	}

	return true;
}

void USHVSubsystem::GetAudioOutputDevices(const FSHVOnAudioOutputDevicesObtained& OnDevices)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		TArray<FSHVAudioOutputDevice> EmptyDevices;
		OnDevices.ExecuteIfBound(EmptyDevices);
		return;
	}

	if (OnDevices.IsBound())
	{
		PendingOutputDeviceDelegates.Add(OnDevices);
	}

	FOnAudioOutputDevicesObtained EngineDelegate;
	EngineDelegate.BindDynamic(this, &USHVSubsystem::HandleAudioOutputDevicesObtained);
	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(World, EngineDelegate);
}

bool USHVSubsystem::SetAudioOutputDevice(const FString& DeviceId, const FSHVOnAudioOutputDeviceSet& OnCompleted)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		const FString Message = TEXT("Audio output devices are not available on a dedicated server.");
		OnCompleted.ExecuteIfBound(false, Message);
		UE_LOG(LogSelfHostedVoiceChat, Verbose, TEXT("%s"), *Message);
		return false;
	}

	if (OnCompleted.IsBound())
	{
		PendingOutputDeviceSetDelegates.Add(OnCompleted);
	}

	FOnCompletedDeviceSwap EngineDelegate;
	EngineDelegate.BindDynamic(this, &USHVSubsystem::HandleAudioOutputDeviceSwapCompleted);
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(World, DeviceId, EngineDelegate);
	return true;
}

void USHVSubsystem::HandleAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices)
{
	TArray<FSHVAudioOutputDevice> Devices;
	Devices.Reserve(AvailableDevices.Num());

	for (const FAudioOutputDeviceInfo& DeviceInfo : AvailableDevices)
	{
		FSHVAudioOutputDevice Device;
		Device.DisplayName = DeviceInfo.Name;
		Device.DeviceId = DeviceInfo.DeviceId;
		Device.NumChannels = DeviceInfo.NumChannels;
		Device.SampleRate = DeviceInfo.SampleRate;
		Device.bIsSystemDefault = DeviceInfo.bIsSystemDefault;
		Device.bIsCurrentDevice = DeviceInfo.bIsCurrentDevice;
		Devices.Add(MoveTemp(Device));

		if (DeviceInfo.bIsCurrentDevice)
		{
			SelectedAudioOutputDeviceId = DeviceInfo.DeviceId;
		}
	}

	for (const FSHVOnAudioOutputDevicesObtained& Delegate : PendingOutputDeviceDelegates)
	{
		Delegate.ExecuteIfBound(Devices);
	}
	PendingOutputDeviceDelegates.Reset();
}

void USHVSubsystem::HandleAudioOutputDeviceSwapCompleted(const FSwapAudioOutputResult& SwapResult)
{
	const bool bSuccess = SwapResult.Result == ESwapAudioOutputDeviceResultState::Success;
	if (bSuccess)
	{
		SelectedAudioOutputDeviceId = SwapResult.RequestedDeviceId;
	}

	const FString Message = bSuccess
		? FString::Printf(TEXT("Audio output device changed to '%s'."), *SwapResult.RequestedDeviceId)
		: FString::Printf(TEXT("Failed to change audio output device from '%s' to '%s'."), *SwapResult.CurrentDeviceId, *SwapResult.RequestedDeviceId);

	if (!bSuccess)
	{
		UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("%s"), *Message);
	}
	else
	{
		UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("%s"), *Message);
	}

	for (const FSHVOnAudioOutputDeviceSet& Delegate : PendingOutputDeviceSetDelegates)
	{
		Delegate.ExecuteIfBound(bSuccess, Message);
	}
	PendingOutputDeviceSetDelegates.Reset();
}

void USHVSubsystem::CleanupRuntimeState()
{
	LocalVoiceActor.Reset();
	ServerVoiceActors.Reset();
	MutedPlayerIds.Reset();
	bDeafened = false;
	PendingOutputDeviceDelegates.Reset();
	PendingOutputDeviceSetDelegates.Reset();
}

void USHVSubsystem::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (!World || World->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	CleanupRuntimeState();
	UE_LOG(LogSelfHostedVoiceChat, Verbose, TEXT("Voice subsystem cleaned runtime state for world cleanup."));
}
