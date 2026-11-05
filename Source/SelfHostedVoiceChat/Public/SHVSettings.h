#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SHVTypes.h"
#include "SHVSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Self Hosted Voice Chat"))
class SELFHOSTEDVOICECHAT_API USHVSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Codec")
	ESHVCodecMode CodecMode = ESHVCodecMode::Auto;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Networking", meta = (ClampMin = "256", ClampMax = "65535"))
	int32 MaxPacketBytes = 12000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Networking", meta = (ClampMin = "1", ClampMax = "200"))
	int32 MaxPacketsPerSecond = 60;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Networking", meta = (ClampMin = "1", ClampMax = "32"))
	int32 MaxRadioChannelsPerPlayer = 8;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Routing")
	bool bAllowGlobalVoice = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Routing")
	bool bAllowProximityVoice = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Routing")
	bool bAllowTeamVoice = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Routing")
	bool bAllowRadioVoice = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Routing", meta = (ClampMin = "100.0", ClampMax = "100000.0"))
	float MaxProximityRange = 4000.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Routing", meta = (ClampMin = "100.0", ClampMax = "100000.0"))
	float DefaultProximityRange = 2000.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "8000", ClampMax = "48000"))
	int32 SampleRate = 16000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "1", ClampMax = "2"))
	int32 NumChannels = 1;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FString DefaultInputDeviceName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FString DefaultOutputDeviceId;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Codec", meta = (ClampMin = "5", ClampMax = "60"))
	int32 FrameDurationMs = 20;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Codec", meta = (ClampMin = "6000", ClampMax = "510000"))
	int32 Bitrate = 24000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Codec", meta = (ClampMin = "0", ClampMax = "10"))
	int32 OpusComplexity = 5;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Codec")
	bool bOpusUseVbr = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Codec", meta = (ClampMin = "0", ClampMax = "100"))
	int32 OpusPacketLossPercent = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.01", ClampMax = "5.0"))
	float DefaultInputVolume = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DefaultOutputVolume = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0", ClampMax = "32767"))
	int32 SpeakingAmplitudeThreshold = 500;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float CaptureTickInterval = 0.02f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float RemoteSpeakerTimeout = 5.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.05", ClampMax = "2.0"))
	float RemoteSpeakingTimeout = 0.35f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0", ClampMax = "20"))
	int32 JitterBufferTargetFrames = 2;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableDebugLogs = false;
};
