#pragma once

#include "CoreMinimal.h"
#include "SHVTypes.h"

struct FSHVCodecConfig
{
	int32 SampleRate = 16000;
	int32 NumChannels = 1;
	int32 FrameDurationMs = 20;
	int32 Bitrate = 24000;
	int32 OpusComplexity = 5;
	int32 OpusPacketLossPercent = 0;
	int32 MaxPacketBytes = 12000;
	bool bUseVbr = true;
	bool bEnableDebugLogs = false;
};

class SELFHOSTEDVOICECHAT_API ISHVVoiceCodec
{
public:
	virtual ~ISHVVoiceCodec() = default;
	virtual bool Initialize(const FSHVCodecConfig& InConfig) = 0;
	virtual bool Encode(const TArray<uint8>& RawPcm, TArray<uint8>& OutEncoded) = 0;
	virtual bool Decode(const TArray<uint8>& Encoded, TArray<uint8>& OutRawPcm) = 0;
	virtual ESHVVoiceCodecId GetCodecId() const = 0;
	virtual FString GetDebugName() const = 0;
	virtual int32 GetFrameBytes() const = 0;
	virtual int32 GetFrameDurationMs() const = 0;
};

class SELFHOSTEDVOICECHAT_API FSHVRawPcmCodec final : public ISHVVoiceCodec
{
public:
	virtual bool Initialize(const FSHVCodecConfig& InConfig) override
	{
		Config = InConfig;
		Config.FrameDurationMs = FMath::Max(1, Config.FrameDurationMs);
		FrameBytes = (Config.SampleRate * Config.FrameDurationMs * Config.NumChannels * static_cast<int32>(sizeof(int16))) / 1000;
		return Config.SampleRate > 0 && Config.NumChannels > 0 && FrameBytes > 0;
	}

	virtual bool Encode(const TArray<uint8>& RawPcm, TArray<uint8>& OutEncoded) override
	{
		OutEncoded = RawPcm;
		return OutEncoded.Num() > 0;
	}

	virtual bool Decode(const TArray<uint8>& Encoded, TArray<uint8>& OutRawPcm) override
	{
		if (Encoded.Num() <= 0 || Encoded.Num() > Config.MaxPacketBytes)
		{
			OutRawPcm.Reset();
			return false;
		}

		OutRawPcm = Encoded;
		return OutRawPcm.Num() > 0;
	}

	virtual ESHVVoiceCodecId GetCodecId() const override
	{
		return ESHVVoiceCodecId::RawPCM;
	}

	virtual FString GetDebugName() const override
	{
		return TEXT("Raw PCM Development Codec");
	}

	virtual int32 GetFrameBytes() const override
	{
		return FrameBytes;
	}

	virtual int32 GetFrameDurationMs() const override
	{
		return Config.FrameDurationMs;
	}

private:
	FSHVCodecConfig Config;
	int32 FrameBytes = 640;
};

SELFHOSTEDVOICECHAT_API TUniquePtr<ISHVVoiceCodec> SHVCreateRawPcmCodec();
SELFHOSTEDVOICECHAT_API TUniquePtr<ISHVVoiceCodec> SHVCreateOpusCodec();
SELFHOSTEDVOICECHAT_API TUniquePtr<ISHVVoiceCodec> SHVCreatePreferredCodec(ESHVCodecMode CodecMode, const FSHVCodecConfig& Config, ESHVVoiceCodecId& OutCodecId);
SELFHOSTEDVOICECHAT_API bool SHVIsOpusCodecAvailable();
SELFHOSTEDVOICECHAT_API FString SHVCodecIdToString(ESHVVoiceCodecId CodecId);
