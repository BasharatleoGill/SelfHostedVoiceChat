#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SHVTypes.h"
#include "SHVBlueprintLibrary.generated.h"

class ASHVVoiceActor;
class USHVSubsystem;

UCLASS()
class SELFHOSTEDVOICECHAT_API USHVBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Setup", meta = (WorldContext = "WorldContextObject", DisplayName = "SHV Spawn Voice Actor For Player", Keywords = "voice chat post login game mode spawn player self hosted"))
	static ASHVVoiceActor* SpawnVoiceActorForPlayer(const UObject* WorldContextObject, APlayerController* PlayerController, TSubclassOf<ASHVVoiceActor> VoiceActorClass);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Setup", meta = (WorldContext = "WorldContextObject", DisplayName = "Destroy Voice Actor For Player", Keywords = "voice chat logout game mode destroy cleanup player self hosted"))
	static bool DestroyVoiceActorForPlayer(const UObject* WorldContextObject, APlayerController* PlayerController);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Local Voice Chat Actor"))
	static ASHVVoiceActor* GetLocalVoiceChatActor(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Start Voice Capture"))
	static bool StartVoiceCapture(const UObject* WorldContextObject, ESHVVoiceMode Mode = ESHVVoiceMode::Global, FName RadioChannel = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Stop Voice Capture"))
	static void StopVoiceCapture(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Voice Enabled"))
	static void SetVoiceEnabled(const UObject* WorldContextObject, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Input Volume"))
	static void SetInputVolume(const UObject* WorldContextObject, float Volume);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Output Volume"))
	static void SetOutputVolume(const UObject* WorldContextObject, float Volume);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Input", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Audio Input Devices", Keywords = "microphone mic input device list"))
	static void GetAudioInputDevices(const UObject* WorldContextObject, TArray<FSHVAudioInputDevice>& OutDevices);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Input", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Audio Input Device", Keywords = "microphone mic input select set"))
	static bool SetAudioInputDevice(const UObject* WorldContextObject, const FString& DeviceName);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat|Audio Input", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Selected Audio Input Device", Keywords = "microphone mic input selected"))
	static FString GetSelectedAudioInputDevice(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Output", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Audio Output Devices", Keywords = "speaker headphone output device list"))
	static void GetAudioOutputDevices(const UObject* WorldContextObject, FSHVOnAudioOutputDevicesObtained OnDevices);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Output", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Audio Output Device", Keywords = "speaker headphone output select set"))
	static bool SetAudioOutputDevice(const UObject* WorldContextObject, const FString& DeviceId, FSHVOnAudioOutputDeviceSet OnCompleted);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat|Audio Output", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Selected Audio Output Device Id", Keywords = "speaker headphone output selected"))
	static FString GetSelectedAudioOutputDeviceId(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Mute Player"))
	static void MutePlayer(const UObject* WorldContextObject, APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Unmute Player"))
	static void UnmutePlayer(const UObject* WorldContextObject, APlayerState* PlayerState);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Is Player Muted"))
	static bool IsPlayerMuted(const UObject* WorldContextObject, APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Proximity Voice Enabled"))
	static void SetProximityVoiceEnabled(const UObject* WorldContextObject, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Global Voice Enabled"))
	static void SetGlobalVoiceEnabled(const UObject* WorldContextObject, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Join Radio Channel"))
	static void JoinRadioChannel(const UObject* WorldContextObject, FName ChannelName);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat|Radio", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Joined Radio Channel", Keywords = "radio channel joined registered"))
	static bool CheckJoinedRadioChannel(const UObject* WorldContextObject, FName ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Leave Radio Channel"))
	static void LeaveRadioChannel(const UObject* WorldContextObject, FName ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Team Id"))
	static void SetTeamId(const UObject* WorldContextObject, int32 TeamId);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Voice Range"))
	static void SetVoiceRange(const UObject* WorldContextObject, float VoiceRange);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Is Local Player Talking"))
	static bool IsLocalPlayerTalking(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Current Active Codec"))
	static ESHVVoiceCodecId GetCurrentActiveCodec(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat", DisplayName = "Is Opus Available")
	static bool IsOpusAvailable();

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat", meta = (WorldContext = "WorldContextObject"))
	static USHVSubsystem* GetSelfHostedVoiceSubsystem(const UObject* WorldContextObject);
};
