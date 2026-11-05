#include "SHVBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "SHVCodecInterface.h"
#include "SHVSubsystem.h"
#include "SHVVoiceActor.h"
#include "SHVVoiceComponent.h"

ASHVVoiceActor* USHVBlueprintLibrary::SpawnVoiceActorForPlayer(const UObject* WorldContextObject, APlayerController* PlayerController, TSubclassOf<ASHVVoiceActor> VoiceActorClass)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || World->GetNetMode() == NM_Client || !IsValid(PlayerController))
	{
		return nullptr;
	}

	USHVSubsystem* Subsystem = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<USHVSubsystem>() : nullptr;
	if (Subsystem)
	{
		if (ASHVVoiceActor* ExistingActor = Subsystem->FindVoiceActorForController(PlayerController))
		{
			return ExistingActor;
		}
	}

	UClass* SpawnClass = VoiceActorClass ? *VoiceActorClass : ASHVVoiceActor::StaticClass();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PlayerController;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASHVVoiceActor* VoiceActor = World->SpawnActor<ASHVVoiceActor>(SpawnClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (VoiceActor && Subsystem)
	{
		Subsystem->RegisterServerVoiceActor(VoiceActor);
	}

	return VoiceActor;
}

bool USHVBlueprintLibrary::DestroyVoiceActorForPlayer(const UObject* WorldContextObject, APlayerController* PlayerController)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || World->GetNetMode() == NM_Client || !IsValid(PlayerController))
	{
		return false;
	}

	USHVSubsystem* Subsystem = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<USHVSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return false;
	}

	ASHVVoiceActor* VoiceActor = Subsystem->FindVoiceActorForController(PlayerController);
	if (!IsValid(VoiceActor))
	{
		return false;
	}

	return VoiceActor->Destroy();
}

ASHVVoiceActor* USHVBlueprintLibrary::GetLocalVoiceChatActor(const UObject* WorldContextObject)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		return Subsystem->GetLocalVoiceActor();
	}
	return nullptr;
}

bool USHVBlueprintLibrary::StartVoiceCapture(const UObject* WorldContextObject, ESHVVoiceMode Mode, FName RadioChannel)
{
	ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject);
	if (VoiceActor && Mode == ESHVVoiceMode::Radio && !RadioChannel.IsNone() && !VoiceActor->CheckJoinedRadioChannel(RadioChannel))
	{
		VoiceActor->ServerJoinRadioChannel(RadioChannel);
	}

	return VoiceActor && VoiceActor->VoiceComponent && VoiceActor->VoiceComponent->StartCapture(Mode, RadioChannel);
}

void USHVBlueprintLibrary::StopVoiceCapture(const UObject* WorldContextObject)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		if (VoiceActor->VoiceComponent)
		{
			VoiceActor->VoiceComponent->StopCapture();
		}
	}
}

void USHVBlueprintLibrary::SetVoiceEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	if (bEnabled)
	{
		StartVoiceCapture(WorldContextObject);
	}
	else
	{
		StopVoiceCapture(WorldContextObject);
	}
}

void USHVBlueprintLibrary::SetInputVolume(const UObject* WorldContextObject, float Volume)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		Subsystem->SetInputVolume(Volume);
	}

	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		if (VoiceActor->VoiceComponent)
		{
			VoiceActor->VoiceComponent->SetInputVolume(Volume);
		}
	}
}

void USHVBlueprintLibrary::SetOutputVolume(const UObject* WorldContextObject, float Volume)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		Subsystem->SetOutputVolume(Volume);
	}

	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		if (VoiceActor->VoiceComponent)
		{
			VoiceActor->VoiceComponent->SetOutputVolume(Volume);
		}
	}
}

void USHVBlueprintLibrary::GetAudioInputDevices(const UObject* WorldContextObject, TArray<FSHVAudioInputDevice>& OutDevices)
{
	OutDevices.Reset();
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		Subsystem->GetAudioInputDevices(OutDevices);
	}
}

bool USHVBlueprintLibrary::SetAudioInputDevice(const UObject* WorldContextObject, const FString& DeviceName)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		return Subsystem->SetAudioInputDevice(DeviceName);
	}
	return false;
}

FString USHVBlueprintLibrary::GetSelectedAudioInputDevice(const UObject* WorldContextObject)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		return Subsystem->GetSelectedAudioInputDevice();
	}
	return FString();
}

void USHVBlueprintLibrary::GetAudioOutputDevices(const UObject* WorldContextObject, FSHVOnAudioOutputDevicesObtained OnDevices)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		Subsystem->GetAudioOutputDevices(OnDevices);
		return;
	}

	TArray<FSHVAudioOutputDevice> EmptyDevices;
	OnDevices.ExecuteIfBound(EmptyDevices);
}

bool USHVBlueprintLibrary::SetAudioOutputDevice(const UObject* WorldContextObject, const FString& DeviceId, FSHVOnAudioOutputDeviceSet OnCompleted)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		return Subsystem->SetAudioOutputDevice(DeviceId, OnCompleted);
	}

	OnCompleted.ExecuteIfBound(false, TEXT("Self Hosted Voice Chat subsystem was not available."));
	return false;
}

FString USHVBlueprintLibrary::GetSelectedAudioOutputDeviceId(const UObject* WorldContextObject)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		return Subsystem->GetSelectedAudioOutputDeviceId();
	}
	return FString();
}

void USHVBlueprintLibrary::MutePlayer(const UObject* WorldContextObject, APlayerState* PlayerState)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		Subsystem->MutePlayer(PlayerState);
	}
}

void USHVBlueprintLibrary::UnmutePlayer(const UObject* WorldContextObject, APlayerState* PlayerState)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		Subsystem->UnmutePlayer(PlayerState);
	}
}

bool USHVBlueprintLibrary::IsPlayerMuted(const UObject* WorldContextObject, APlayerState* PlayerState)
{
	if (USHVSubsystem* Subsystem = GetSelfHostedVoiceSubsystem(WorldContextObject))
	{
		return Subsystem->IsPlayerMuted(PlayerState);
	}
	return false;
}

void USHVBlueprintLibrary::SetProximityVoiceEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		VoiceActor->ServerSetVoiceModes(VoiceActor->bGlobalVoiceEnabled, bEnabled, VoiceActor->bTeamVoiceEnabled);
	}
}

void USHVBlueprintLibrary::SetGlobalVoiceEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		VoiceActor->ServerSetVoiceModes(bEnabled, VoiceActor->bProximityVoiceEnabled, VoiceActor->bTeamVoiceEnabled);
	}
}

void USHVBlueprintLibrary::JoinRadioChannel(const UObject* WorldContextObject, FName ChannelName)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		VoiceActor->ServerJoinRadioChannel(ChannelName);
	}
}

bool USHVBlueprintLibrary::CheckJoinedRadioChannel(const UObject* WorldContextObject, FName ChannelName)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		return VoiceActor->CheckJoinedRadioChannel(ChannelName);
	}
	return false;
}

void USHVBlueprintLibrary::LeaveRadioChannel(const UObject* WorldContextObject, FName ChannelName)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		VoiceActor->ServerLeaveRadioChannel(ChannelName);
	}
}

void USHVBlueprintLibrary::SetTeamId(const UObject* WorldContextObject, int32 TeamId)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		if (VoiceActor->HasAuthority())
		{
			VoiceActor->SetTeamIdAuthority(TeamId);
		}
		else
		{
			UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("SetTeamId must be called from trusted server authority logic."));
		}
	}
}

void USHVBlueprintLibrary::SetVoiceRange(const UObject* WorldContextObject, float VoiceRange)
{
	if (ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject))
	{
		VoiceActor->ServerSetVoiceRange(VoiceRange);
	}
}

bool USHVBlueprintLibrary::IsLocalPlayerTalking(const UObject* WorldContextObject)
{
	ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject);
	return VoiceActor && VoiceActor->VoiceComponent && VoiceActor->VoiceComponent->IsTalking();
}

ESHVVoiceCodecId USHVBlueprintLibrary::GetCurrentActiveCodec(const UObject* WorldContextObject)
{
	ASHVVoiceActor* VoiceActor = GetLocalVoiceChatActor(WorldContextObject);
	return VoiceActor && VoiceActor->VoiceComponent ? VoiceActor->VoiceComponent->GetActiveCodec() : ESHVVoiceCodecId::RawPCM;
}

bool USHVBlueprintLibrary::IsOpusAvailable()
{
	return SHVIsOpusCodecAvailable();
}

USHVSubsystem* USHVBlueprintLibrary::GetSelfHostedVoiceSubsystem(const UObject* WorldContextObject)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World && World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<USHVSubsystem>() : nullptr;
}
