#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHVCodecInterface.h"
#include "SHVTypes.h"
#include "SHVVoiceCaptureBackend.h"
#include "SHVVoiceComponent.generated.h"

class USoundWaveProcedural;
class UAudioComponent;
class APlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSHVSpeakingStateChanged, bool, bIsTalking);

USTRUCT()
struct FSHVRemoteSpeakerPlayback
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USoundWaveProcedural> SoundWave = nullptr;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent = nullptr;

	UPROPERTY()
	float LastPacketTime = 0.0f;
};

UCLASS(ClassGroup = (Voice), meta = (BlueprintSpawnableComponent))
class SELFHOSTEDVOICECHAT_API USHVVoiceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USHVVoiceComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	bool StartCapture(ESHVVoiceMode Mode = ESHVVoiceMode::Global, FName RadioChannel = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void StopCapture();

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	bool IsCapturing() const { return bCaptureActive; }

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	bool IsTalking() const { return bIsTalking; }

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	ESHVVoiceCodecId GetActiveCodec() const { return ActiveCodecId; }

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetInputVolume(float InVolume);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetOutputVolume(float InVolume);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Audio Input")
	void UseInputDevice(const FString& DeviceName);

	void ReceiveVoicePacket(APlayerState* SenderPlayerState, const FSHVVoicePacket& Packet);
	void ShutdownAudio();

	UPROPERTY(BlueprintAssignable, Category = "Self Hosted Voice Chat")
	FSHVSpeakingStateChanged OnSpeakingStateChanged;

private:
	void InitializeAudioPath();
	FSHVCodecConfig MakeCodecConfig() const;
	ISHVVoiceCodec* GetDecodeCodec(ESHVVoiceCodecId CodecId);
	void SubmitCapturedBytes(const TArray<uint8>& RawPcm);
	void EncodeAndSendFrame(const TArray<uint8>& RawPcmFrame);
	void SetTalkingState(bool bNewTalking);
	int32 CalculateAverageAmplitude(const TArray<uint8>& PcmBytes) const;
	FSHVRemoteSpeakerPlayback& FindOrCreatePlayback(int32 SpeakerId);
	bool CanCaptureLocally() const;
	void PruneStalePlayback();

	TUniquePtr<ISHVVoiceCaptureBackend> CaptureBackend;
	TUniquePtr<ISHVVoiceCodec> EncodeCodec;
	TUniquePtr<ISHVVoiceCodec> RawDecodeCodec;
	TUniquePtr<ISHVVoiceCodec> OpusDecodeCodec;

	UPROPERTY()
	TMap<int32, FSHVRemoteSpeakerPlayback> RemotePlayback;

	TArray<uint8> CaptureFrameBuffer;
	bool bCaptureActive = false;
	bool bIsTalking = false;
	float InputVolume = 1.0f;
	float OutputVolume = 1.0f;
	int32 Sequence = 0;
	int32 FrameBytes = 640;
	ESHVVoiceCodecId ActiveCodecId = ESHVVoiceCodecId::RawPCM;
	ESHVVoiceMode ActiveMode = ESHVVoiceMode::Global;
	FName ActiveRadioChannel = NAME_None;
	float LastPlaybackPruneTime = 0.0f;
};
