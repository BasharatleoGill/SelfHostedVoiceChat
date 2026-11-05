#pragma once

#include "CoreMinimal.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SHVTypes.h"
#include "SHVSubsystem.generated.h"

class ASHVVoiceActor;
class APlayerState;

UCLASS()
class SELFHOSTEDVOICECHAT_API USHVSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterLocalVoiceActor(ASHVVoiceActor* VoiceActor);
	void UnregisterLocalVoiceActor(ASHVVoiceActor* VoiceActor);
	void RegisterServerVoiceActor(ASHVVoiceActor* VoiceActor);
	void UnregisterServerVoiceActor(ASHVVoiceActor* VoiceActor);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	ASHVVoiceActor* GetLocalVoiceActor() const;

	ASHVVoiceActor* FindVoiceActorForController(APlayerController* PlayerController) const;
	TArray<ASHVVoiceActor*> GetServerVoiceActors() const;

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void MutePlayer(APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void UnmutePlayer(APlayerState* PlayerState);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	bool IsPlayerMuted(APlayerState* PlayerState) const;

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetDeafened(bool bInDeafened);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	bool IsDeafened() const { return bDeafened; }

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetInputVolume(float InVolume);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetOutputVolume(float InVolume);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	float GetInputVolume() const { return InputVolume; }

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	float GetOutputVolume() const { return OutputVolume; }

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Input", meta = (DisplayName = "Get Audio Input Devices"))
	void GetAudioInputDevices(TArray<FSHVAudioInputDevice>& OutDevices) const;

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Input", meta = (DisplayName = "Set Audio Input Device"))
	bool SetAudioInputDevice(const FString& DeviceName);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat|Audio Input", meta = (DisplayName = "Get Selected Audio Input Device"))
	FString GetSelectedAudioInputDevice() const { return SelectedAudioInputDevice; }

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Output", meta = (DisplayName = "Get Audio Output Devices"))
	void GetAudioOutputDevices(const FSHVOnAudioOutputDevicesObtained& OnDevices);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Output", meta = (DisplayName = "Set Audio Output Device"))
	bool SetAudioOutputDevice(const FString& DeviceId, const FSHVOnAudioOutputDeviceSet& OnCompleted);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat|Audio Output", meta = (DisplayName = "Get Selected Audio Output Device Id"))
	FString GetSelectedAudioOutputDeviceId() const { return SelectedAudioOutputDeviceId; }

	void CleanupRuntimeState();

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	UFUNCTION()
	void HandleAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices);

	UFUNCTION()
	void HandleAudioOutputDeviceSwapCompleted(const FSwapAudioOutputResult& SwapResult);

	TWeakObjectPtr<ASHVVoiceActor> LocalVoiceActor;
	TArray<TWeakObjectPtr<ASHVVoiceActor>> ServerVoiceActors;
	TSet<int32> MutedPlayerIds;
	FDelegateHandle WorldCleanupHandle;
	bool bDeafened = false;
	float InputVolume = 1.0f;
	float OutputVolume = 1.0f;
	FString SelectedAudioInputDevice;
	FString SelectedAudioOutputDeviceId;
	TArray<FSHVOnAudioOutputDevicesObtained> PendingOutputDeviceDelegates;
	TArray<FSHVOnAudioOutputDeviceSet> PendingOutputDeviceSetDelegates;
};
