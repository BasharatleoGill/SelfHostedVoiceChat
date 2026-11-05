#pragma once

#include "CoreMinimal.h"

class SELFHOSTEDVOICECHAT_API ISHVVoiceCaptureBackend
{
public:
	virtual ~ISHVVoiceCaptureBackend() = default;
	virtual bool Initialize(int32 InSampleRate, int32 InNumChannels, const FString& DeviceName) = 0;
	virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsCapturing() const = 0;
	virtual bool Capture(TArray<uint8>& OutPcmBytes, int32 MaxBytes) = 0;
	virtual FString GetDebugName() const = 0;
};

SELFHOSTEDVOICECHAT_API TUniquePtr<ISHVVoiceCaptureBackend> CreateSHVVoiceCaptureBackend();
