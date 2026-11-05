#include "SHVCodecInterface.h"

#if SHV_WITH_OPUS
#include "opus.h"

namespace
{
	bool IsSupportedOpusSampleRate(int32 SampleRate)
	{
		return SampleRate == 8000 || SampleRate == 12000 || SampleRate == 16000 || SampleRate == 24000 || SampleRate == 48000;
	}

	bool IsSupportedOpusFrameDuration(int32 FrameDurationMs)
	{
		return FrameDurationMs == 5 || FrameDurationMs == 10 || FrameDurationMs == 20 || FrameDurationMs == 40 || FrameDurationMs == 60;
	}

	class FSHVOpusCodec final : public ISHVVoiceCodec
	{
	public:
		virtual ~FSHVOpusCodec() override
		{
			if (Encoder)
			{
				opus_encoder_destroy(Encoder);
				Encoder = nullptr;
			}

			if (Decoder)
			{
				opus_decoder_destroy(Decoder);
				Decoder = nullptr;
			}
		}

		virtual bool Initialize(const FSHVCodecConfig& InConfig) override
		{
			Config = InConfig;
			Config.OpusComplexity = FMath::Clamp(Config.OpusComplexity, 0, 10);
			Config.OpusPacketLossPercent = FMath::Clamp(Config.OpusPacketLossPercent, 0, 100);
			Config.MaxPacketBytes = FMath::Max(256, Config.MaxPacketBytes);

			if (!IsSupportedOpusSampleRate(Config.SampleRate))
			{
				UE_LOG(LogSelfHostedVoiceChat, Error, TEXT("Opus initialization failed: unsupported sample rate %d."), Config.SampleRate);
				return false;
			}

			if (Config.NumChannels < 1 || Config.NumChannels > 2)
			{
				UE_LOG(LogSelfHostedVoiceChat, Error, TEXT("Opus initialization failed: unsupported channel count %d."), Config.NumChannels);
				return false;
			}

			if (!IsSupportedOpusFrameDuration(Config.FrameDurationMs))
			{
				UE_LOG(LogSelfHostedVoiceChat, Error, TEXT("Opus initialization failed: unsupported frame duration %d ms. Use 5, 10, 20, 40, or 60."), Config.FrameDurationMs);
				return false;
			}

			FrameSamplesPerChannel = (Config.SampleRate * Config.FrameDurationMs) / 1000;
			FrameBytes = FrameSamplesPerChannel * Config.NumChannels * static_cast<int32>(sizeof(int16));
			if (FrameBytes <= 0)
			{
				return false;
			}

			int32 EncoderError = OPUS_OK;
			Encoder = opus_encoder_create(Config.SampleRate, Config.NumChannels, OPUS_APPLICATION_VOIP, &EncoderError);
			if (!Encoder || EncoderError != OPUS_OK)
			{
				UE_LOG(LogSelfHostedVoiceChat, Error, TEXT("Opus encoder creation failed: %s."), ANSI_TO_TCHAR(opus_strerror(EncoderError)));
				return false;
			}

			int32 DecoderError = OPUS_OK;
			Decoder = opus_decoder_create(Config.SampleRate, Config.NumChannels, &DecoderError);
			if (!Decoder || DecoderError != OPUS_OK)
			{
				UE_LOG(LogSelfHostedVoiceChat, Error, TEXT("Opus decoder creation failed: %s."), ANSI_TO_TCHAR(opus_strerror(DecoderError)));
				return false;
			}

			opus_encoder_ctl(Encoder, OPUS_SET_BITRATE(Config.Bitrate));
			opus_encoder_ctl(Encoder, OPUS_SET_COMPLEXITY(Config.OpusComplexity));
			opus_encoder_ctl(Encoder, OPUS_SET_VBR(Config.bUseVbr ? 1 : 0));
			opus_encoder_ctl(Encoder, OPUS_SET_PACKET_LOSS_PERC(Config.OpusPacketLossPercent));
			opus_encoder_ctl(Encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));

			EncodeBuffer.SetNumUninitialized(Config.MaxPacketBytes);
			DecodeBuffer.SetNumUninitialized(FrameBytes);

			UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("Opus codec initialized: %d Hz, %d channel(s), %d ms, %d bps, complexity %d, VBR %s."),
				Config.SampleRate,
				Config.NumChannels,
				Config.FrameDurationMs,
				Config.Bitrate,
				Config.OpusComplexity,
				Config.bUseVbr ? TEXT("on") : TEXT("off"));
			return true;
		}

		virtual bool Encode(const TArray<uint8>& RawPcm, TArray<uint8>& OutEncoded) override
		{
			OutEncoded.Reset();
			if (!Encoder || RawPcm.Num() != FrameBytes)
			{
				if (Config.bEnableDebugLogs)
				{
					UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Opus encode rejected frame size %d; expected %d."), RawPcm.Num(), FrameBytes);
				}
				return false;
			}

			const opus_int16* PcmSamples = reinterpret_cast<const opus_int16*>(RawPcm.GetData());
			const int32 EncodedBytes = opus_encode(Encoder, PcmSamples, FrameSamplesPerChannel, EncodeBuffer.GetData(), EncodeBuffer.Num());
			if (EncodedBytes < 0)
			{
				UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Opus encode failed: %s."), ANSI_TO_TCHAR(opus_strerror(EncodedBytes)));
				return false;
			}

			if (EncodedBytes <= 0 || EncodedBytes > Config.MaxPacketBytes)
			{
				UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Opus encode produced invalid packet size %d."), EncodedBytes);
				return false;
			}

			OutEncoded.Append(EncodeBuffer.GetData(), EncodedBytes);
			return true;
		}

		virtual bool Decode(const TArray<uint8>& Encoded, TArray<uint8>& OutRawPcm) override
		{
			OutRawPcm.Reset();
			if (!Decoder || Encoded.Num() <= 0 || Encoded.Num() > Config.MaxPacketBytes)
			{
				return false;
			}

			const int32 DecodedSamplesPerChannel = opus_decode(Decoder, Encoded.GetData(), Encoded.Num(), reinterpret_cast<opus_int16*>(DecodeBuffer.GetData()), FrameSamplesPerChannel, 0);
			if (DecodedSamplesPerChannel < 0)
			{
				UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Opus decode failed: %s."), ANSI_TO_TCHAR(opus_strerror(DecodedSamplesPerChannel)));
				return false;
			}

			const int32 DecodedBytes = DecodedSamplesPerChannel * Config.NumChannels * static_cast<int32>(sizeof(int16));
			if (DecodedBytes <= 0 || DecodedBytes > DecodeBuffer.Num())
			{
				return false;
			}

			OutRawPcm.Append(DecodeBuffer.GetData(), DecodedBytes);
			return true;
		}

		virtual ESHVVoiceCodecId GetCodecId() const override
		{
			return ESHVVoiceCodecId::Opus;
		}

		virtual FString GetDebugName() const override
		{
			return TEXT("Opus");
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
		OpusEncoder* Encoder = nullptr;
		OpusDecoder* Decoder = nullptr;
		int32 FrameSamplesPerChannel = 0;
		int32 FrameBytes = 0;
		TArray<uint8> EncodeBuffer;
		TArray<uint8> DecodeBuffer;
	};
}
#endif

bool SHVIsOpusCodecAvailable()
{
#if SHV_WITH_OPUS
	return true;
#else
	return false;
#endif
}

TUniquePtr<ISHVVoiceCodec> SHVCreateOpusCodec()
{
#if SHV_WITH_OPUS
	return MakeUnique<FSHVOpusCodec>();
#else
	return nullptr;
#endif
}
