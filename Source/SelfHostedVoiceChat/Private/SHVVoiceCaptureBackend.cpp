#include "SHVVoiceCaptureBackend.h"

#include "SHVTypes.h"

#if PLATFORM_WINDOWS
#include "Interfaces/VoiceCapture.h"
#include "VoiceModule.h"
#endif

namespace
{
	class FSHVNullVoiceCaptureBackend final : public ISHVVoiceCaptureBackend
	{
	public:
		virtual bool Initialize(int32 InSampleRate, int32 InNumChannels, const FString& DeviceName) override
		{
			UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Voice capture is unsupported on this platform in the current plugin build."));
			return false;
		}

		virtual bool Start() override { return false; }
		virtual void Stop() override {}
		virtual bool IsCapturing() const override { return false; }
		virtual bool Capture(TArray<uint8>& OutPcmBytes, int32 MaxBytes) override
		{
			OutPcmBytes.Reset();
			return false;
		}
		virtual FString GetDebugName() const override { return TEXT("Unsupported Capture Backend"); }
	};

#if PLATFORM_WINDOWS
	class FSHVWindowsVoiceCaptureBackend final : public ISHVVoiceCaptureBackend
	{
	public:
		virtual bool Initialize(int32 InSampleRate, int32 InNumChannels, const FString& DeviceName) override
		{
			SampleRate = InSampleRate;
			NumChannels = InNumChannels;
			VoiceCapture = FVoiceModule::Get().CreateVoiceCapture(DeviceName, SampleRate, NumChannels);
			if (!VoiceCapture.IsValid())
			{
				UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Failed to create Windows voice capture backend."));
				return false;
			}
			return true;
		}

		virtual bool Start() override
		{
			if (!VoiceCapture.IsValid())
			{
				return false;
			}

			VoiceCapture->Start();
			bCapturing = true;
			return true;
		}

		virtual void Stop() override
		{
			if (VoiceCapture.IsValid())
			{
				VoiceCapture->Stop();
			}
			bCapturing = false;
		}

		virtual bool IsCapturing() const override
		{
			return bCapturing;
		}

		virtual bool Capture(TArray<uint8>& OutPcmBytes, int32 MaxBytes) override
		{
			OutPcmBytes.Reset();
			if (!VoiceCapture.IsValid() || MaxBytes <= 0)
			{
				return false;
			}

			uint32 AvailableBytes = 0;
			const EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);
			if (CaptureState != EVoiceCaptureState::Ok || AvailableBytes == 0)
			{
				return false;
			}

			const uint32 BytesToRead = FMath::Min<uint32>(AvailableBytes, static_cast<uint32>(MaxBytes));
			OutPcmBytes.SetNumUninitialized(BytesToRead);

			uint32 WrittenBytes = 0;
			VoiceCapture->GetVoiceData(OutPcmBytes.GetData(), BytesToRead, WrittenBytes);
			OutPcmBytes.SetNum(static_cast<int32>(WrittenBytes), EAllowShrinking::No);
			return WrittenBytes > 0;
		}

		virtual FString GetDebugName() const override { return TEXT("Windows Voice Capture"); }

	private:
		TSharedPtr<IVoiceCapture> VoiceCapture;
		int32 SampleRate = 16000;
		int32 NumChannels = 1;
		bool bCapturing = false;
	};
#endif
}

TUniquePtr<ISHVVoiceCaptureBackend> CreateSHVVoiceCaptureBackend()
{
#if PLATFORM_WINDOWS
	return MakeUnique<FSHVWindowsVoiceCaptureBackend>();
#else
	return MakeUnique<FSHVNullVoiceCaptureBackend>();
#endif
}
