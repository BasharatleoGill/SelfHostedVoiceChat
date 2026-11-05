#include "SHVCodecInterface.h"

TUniquePtr<ISHVVoiceCodec> SHVCreateRawPcmCodec()
{
	return MakeUnique<FSHVRawPcmCodec>();
}

TUniquePtr<ISHVVoiceCodec> SHVCreatePreferredCodec(ESHVCodecMode CodecMode, const FSHVCodecConfig& Config, ESHVVoiceCodecId& OutCodecId)
{
	if (CodecMode == ESHVCodecMode::Opus || CodecMode == ESHVCodecMode::Auto)
	{
		if (SHVIsOpusCodecAvailable())
		{
			TUniquePtr<ISHVVoiceCodec> OpusCodec = SHVCreateOpusCodec();
			if (OpusCodec.IsValid() && OpusCodec->Initialize(Config))
			{
				OutCodecId = ESHVVoiceCodecId::Opus;
				return OpusCodec;
			}
		}

		if (CodecMode == ESHVCodecMode::Opus)
		{
			UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Opus codec was requested but is unavailable or failed to initialize. Falling back to Raw PCM."));
		}
		else
		{
			UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Auto codec could not use Opus. Falling back to Raw PCM."));
		}
	}

	TUniquePtr<ISHVVoiceCodec> RawCodec = SHVCreateRawPcmCodec();
	RawCodec->Initialize(Config);
	OutCodecId = ESHVVoiceCodecId::RawPCM;
	return RawCodec;
}

FString SHVCodecIdToString(ESHVVoiceCodecId CodecId)
{
	switch (CodecId)
	{
	case ESHVVoiceCodecId::Opus:
		return TEXT("Opus");
	case ESHVVoiceCodecId::RawPCM:
	default:
		return TEXT("RawPCM");
	}
}
