#pragma once

#include "CoreMinimal.h"
#include "SHVTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSelfHostedVoiceChat, Log, All);

UENUM(BlueprintType)
enum class ESHVVoiceMode : uint8
{
	Global UMETA(DisplayName = "Global"),
	Proximity UMETA(DisplayName = "Proximity"),
	Team UMETA(DisplayName = "Team"),
	Radio UMETA(DisplayName = "Radio")
};

UENUM(BlueprintType)
enum class ESHVCodecMode : uint8
{
	Auto UMETA(DisplayName = "Auto"),
	Opus UMETA(DisplayName = "Opus"),
	RawPCM UMETA(DisplayName = "Raw PCM")
};

UENUM(BlueprintType)
enum class ESHVVoiceCodecId : uint8
{
	RawPCM UMETA(DisplayName = "Raw PCM"),
	Opus UMETA(DisplayName = "Opus")
};

USTRUCT(BlueprintType)
struct FSHVVoicePacket
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	TArray<uint8> Payload;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	ESHVVoiceCodecId Codec = ESHVVoiceCodecId::RawPCM;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	ESHVVoiceMode VoiceMode = ESHVVoiceMode::Global;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	FName RadioChannel = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	float RequestedRange = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 Sequence = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 SampleRate = 16000;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 NumChannels = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 FrameDurationMs = 20;
};

USTRUCT(BlueprintType)
struct FSHVVoiceDropStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 OversizedPackets = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 RateLimitedPackets = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 UnauthorizedPackets = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 UnsupportedModePackets = 0;
};

USTRUCT(BlueprintType)
struct FSHVAudioInputDevice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	FString DeviceName;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	FString Info;
};

USTRUCT(BlueprintType)
struct FSHVAudioOutputDevice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	FString DeviceId;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 NumChannels = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	int32 SampleRate = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	bool bIsSystemDefault = false;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	bool bIsCurrentDevice = false;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FSHVOnAudioOutputDevicesObtained, const TArray<FSHVAudioOutputDevice>&, AvailableDevices);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FSHVOnAudioOutputDeviceSet, bool, bSuccess, const FString&, Message);
