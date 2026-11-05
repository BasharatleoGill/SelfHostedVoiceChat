#include "SHVVoiceComponent.h"

#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerState.h"
#include "SHVCodecInterface.h"
#include "SHVSettings.h"
#include "SHVSubsystem.h"
#include "SHVTypes.h"
#include "SHVVoiceActor.h"
#include "SHVVoiceCaptureBackend.h"
#include "Sound/SoundWaveProcedural.h"

USHVVoiceComponent::USHVVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USHVVoiceComponent::BeginPlay()
{
	Super::BeginPlay();

	const USHVSettings* Settings = GetDefault<USHVSettings>();
	InputVolume = Settings->DefaultInputVolume;
	OutputVolume = Settings->DefaultOutputVolume;
	PrimaryComponentTick.TickInterval = Settings->CaptureTickInterval;

	FrameBytes = (Settings->SampleRate * Settings->FrameDurationMs * Settings->NumChannels * static_cast<int32>(sizeof(int16))) / 1000;
}

void USHVVoiceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ShutdownAudio();
	Super::EndPlay(EndPlayReason);
}

void USHVVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsRunningDedicatedServer())
	{
		return;
	}

	PruneStalePlayback();

	if (bCaptureActive && CaptureBackend.IsValid())
	{
		const USHVSettings* Settings = GetDefault<USHVSettings>();
		TArray<uint8> CapturedPcm;
		if (CaptureBackend->Capture(CapturedPcm, Settings->MaxPacketBytes))
		{
			SubmitCapturedBytes(CapturedPcm);
		}
		else
		{
			SetTalkingState(false);
		}
	}

	if (!bCaptureActive && RemotePlayback.Num() == 0)
	{
		SetComponentTickEnabled(false);
	}
}

bool USHVVoiceComponent::StartCapture(ESHVVoiceMode Mode, FName RadioChannel)
{
	if (!CanCaptureLocally())
	{
		UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Voice capture can only start on the owning local voice actor."));
		return false;
	}

	InitializeAudioPath();
	if (!CaptureBackend.IsValid())
	{
		CaptureBackend = CreateSHVVoiceCaptureBackend();
		FString DeviceName;
		if (const UWorld* World = GetWorld())
		{
			if (const UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (const USHVSubsystem* Subsystem = GameInstance->GetSubsystem<USHVSubsystem>())
				{
					DeviceName = Subsystem->GetSelectedAudioInputDevice();
				}
			}
		}

		if (!CaptureBackend->Initialize(GetDefault<USHVSettings>()->SampleRate, GetDefault<USHVSettings>()->NumChannels, DeviceName))
		{
			CaptureBackend.Reset();
		}
	}

	if (!CaptureBackend.IsValid())
	{
		return false;
	}

	ActiveMode = Mode;
	ActiveRadioChannel = RadioChannel;
	if (ASHVVoiceActor* VoiceActor = Cast<ASHVVoiceActor>(GetOwner()))
	{
		if (Mode == ESHVVoiceMode::Radio && !RadioChannel.IsNone() && !VoiceActor->CheckJoinedRadioChannel(RadioChannel))
		{
			VoiceActor->ServerJoinRadioChannel(RadioChannel);
		}
	}

	const bool bStarted = CaptureBackend->Start();
	bCaptureActive = bStarted;
	SetComponentTickEnabled(bStarted);
	UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("Voice capture %s using %s."), bStarted ? TEXT("started") : TEXT("failed"), *CaptureBackend->GetDebugName());
	return bStarted;
}

void USHVVoiceComponent::StopCapture()
{
	if (CaptureBackend.IsValid())
	{
		CaptureBackend->Stop();
	}

	bCaptureActive = false;
	SetTalkingState(false);
	SetComponentTickEnabled(false);
}

void USHVVoiceComponent::SetInputVolume(float InVolume)
{
	InputVolume = FMath::Max(0.0f, InVolume);
}

void USHVVoiceComponent::SetOutputVolume(float InVolume)
{
	OutputVolume = FMath::Max(0.0f, InVolume);
	for (TPair<int32, FSHVRemoteSpeakerPlayback>& Pair : RemotePlayback)
	{
		if (Pair.Value.AudioComponent)
		{
			Pair.Value.AudioComponent->SetVolumeMultiplier(OutputVolume);
		}
	}
}

void USHVVoiceComponent::UseInputDevice(const FString& DeviceName)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	const bool bWasCapturing = bCaptureActive;
	const ESHVVoiceMode PreviousMode = ActiveMode;
	const FName PreviousRadioChannel = ActiveRadioChannel;

	if (bWasCapturing)
	{
		StopCapture();
	}
	else if (CaptureBackend.IsValid())
	{
		CaptureBackend->Stop();
	}

	CaptureBackend.Reset();
	CaptureFrameBuffer.Reset();

	if (bWasCapturing)
	{
		StartCapture(PreviousMode, PreviousRadioChannel);
	}
}

void USHVVoiceComponent::ReceiveVoicePacket(APlayerState* SenderPlayerState, const FSHVVoicePacket& Packet)
{
	if (!IsValid(SenderPlayerState) || IsRunningDedicatedServer())
	{
		return;
	}

	const UWorld* World = GetWorld();
	USHVSubsystem* Subsystem = World && World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<USHVSubsystem>() : nullptr;
	if (Subsystem && (Subsystem->IsDeafened() || Subsystem->IsPlayerMuted(SenderPlayerState)))
	{
		return;
	}

	const USHVSettings* Settings = GetDefault<USHVSettings>();
	if (Packet.Payload.Num() <= 0 || Packet.Payload.Num() > Settings->MaxPacketBytes)
	{
		return;
	}
	if (Packet.SampleRate != Settings->SampleRate || Packet.NumChannels != Settings->NumChannels || Packet.FrameDurationMs != Settings->FrameDurationMs)
	{
		if (Settings->bEnableDebugLogs)
		{
			UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet with mismatched audio metadata."));
		}
		return;
	}

	InitializeAudioPath();
	ISHVVoiceCodec* DecodeCodec = GetDecodeCodec(Packet.Codec);
	if (!DecodeCodec)
	{
		if (Settings->bEnableDebugLogs)
		{
			UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet with unsupported codec %s."), *SHVCodecIdToString(Packet.Codec));
		}
		return;
	}

	TArray<uint8> DecodedPcm;
	if (!DecodeCodec->Decode(Packet.Payload, DecodedPcm) || DecodedPcm.Num() == 0)
	{
		return;
	}

	FSHVRemoteSpeakerPlayback& Playback = FindOrCreatePlayback(SenderPlayerState->GetPlayerId());
	if (!Playback.SoundWave || !Playback.AudioComponent)
	{
		return;
	}

	Playback.SoundWave->QueueAudio(DecodedPcm.GetData(), DecodedPcm.Num());
	Playback.LastPacketTime = World ? World->GetTimeSeconds() : 0.0f;
	SetComponentTickEnabled(true);

	if (!Playback.AudioComponent->IsPlaying())
	{
		Playback.AudioComponent->Play();
	}
}

void USHVVoiceComponent::ShutdownAudio()
{
	StopCapture();

	for (TPair<int32, FSHVRemoteSpeakerPlayback>& Pair : RemotePlayback)
	{
		if (Pair.Value.AudioComponent)
		{
			Pair.Value.AudioComponent->Stop();
			Pair.Value.AudioComponent->DestroyComponent();
		}
	}

	RemotePlayback.Reset();
	CaptureBackend.Reset();
	EncodeCodec.Reset();
	RawDecodeCodec.Reset();
	OpusDecodeCodec.Reset();
	CaptureFrameBuffer.Reset();
}

void USHVVoiceComponent::InitializeAudioPath()
{
	const USHVSettings* Settings = GetDefault<USHVSettings>();
	if (IsRunningDedicatedServer())
	{
		return;
	}

	if (!EncodeCodec.IsValid())
	{
		const FSHVCodecConfig CodecConfig = MakeCodecConfig();
		EncodeCodec = SHVCreatePreferredCodec(Settings->CodecMode, CodecConfig, ActiveCodecId);
		FrameBytes = EncodeCodec.IsValid() ? EncodeCodec->GetFrameBytes() : FrameBytes;
		UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("Voice codec initialized: %s."), EncodeCodec.IsValid() ? *EncodeCodec->GetDebugName() : TEXT("None"));
	}
}

FSHVCodecConfig USHVVoiceComponent::MakeCodecConfig() const
{
	const USHVSettings* Settings = GetDefault<USHVSettings>();
	FSHVCodecConfig Config;
	Config.SampleRate = Settings->SampleRate;
	Config.NumChannels = Settings->NumChannels;
	Config.FrameDurationMs = Settings->FrameDurationMs;
	Config.Bitrate = Settings->Bitrate;
	Config.OpusComplexity = Settings->OpusComplexity;
	Config.OpusPacketLossPercent = Settings->OpusPacketLossPercent;
	Config.MaxPacketBytes = Settings->MaxPacketBytes;
	Config.bUseVbr = Settings->bOpusUseVbr;
	Config.bEnableDebugLogs = Settings->bEnableDebugLogs;
	return Config;
}

ISHVVoiceCodec* USHVVoiceComponent::GetDecodeCodec(ESHVVoiceCodecId CodecId)
{
	const FSHVCodecConfig CodecConfig = MakeCodecConfig();
	if (CodecId == ESHVVoiceCodecId::RawPCM)
	{
		if (!RawDecodeCodec.IsValid())
		{
			RawDecodeCodec = SHVCreateRawPcmCodec();
			RawDecodeCodec->Initialize(CodecConfig);
		}
		return RawDecodeCodec.Get();
	}

	if (CodecId == ESHVVoiceCodecId::Opus)
	{
		if (!SHVIsOpusCodecAvailable())
		{
			return nullptr;
		}

		if (!OpusDecodeCodec.IsValid())
		{
			OpusDecodeCodec = SHVCreateOpusCodec();
			if (!OpusDecodeCodec.IsValid() || !OpusDecodeCodec->Initialize(CodecConfig))
			{
				OpusDecodeCodec.Reset();
				return nullptr;
			}
		}
		return OpusDecodeCodec.Get();
	}

	return nullptr;
}

void USHVVoiceComponent::SubmitCapturedBytes(const TArray<uint8>& RawPcm)
{
	ASHVVoiceActor* VoiceActor = Cast<ASHVVoiceActor>(GetOwner());
	if (!VoiceActor || !EncodeCodec.IsValid() || FrameBytes <= 0)
	{
		return;
	}

	const int32 AverageAmplitude = CalculateAverageAmplitude(RawPcm);
	const USHVSettings* Settings = GetDefault<USHVSettings>();
	SetTalkingState(AverageAmplitude >= Settings->SpeakingAmplitudeThreshold);

	if (!bIsTalking)
	{
		CaptureFrameBuffer.Reset();
		return;
	}

	TArray<uint8> GainAdjustedPcm = RawPcm;
	if (!FMath::IsNearlyEqual(InputVolume, 1.0f))
	{
		int16* Samples = reinterpret_cast<int16*>(GainAdjustedPcm.GetData());
		const int32 SampleCount = GainAdjustedPcm.Num() / sizeof(int16);
		for (int32 Index = 0; Index < SampleCount; ++Index)
		{
			const int32 Scaled = FMath::RoundToInt(static_cast<float>(Samples[Index]) * InputVolume);
			Samples[Index] = static_cast<int16>(FMath::Clamp(Scaled, -32768, 32767));
		}
	}

	CaptureFrameBuffer.Append(GainAdjustedPcm);
	while (CaptureFrameBuffer.Num() >= FrameBytes)
	{
		TArray<uint8> Frame;
		Frame.Append(CaptureFrameBuffer.GetData(), FrameBytes);
		CaptureFrameBuffer.RemoveAt(0, FrameBytes, EAllowShrinking::No);
		EncodeAndSendFrame(Frame);
	}
}

void USHVVoiceComponent::EncodeAndSendFrame(const TArray<uint8>& RawPcmFrame)
{
	ASHVVoiceActor* VoiceActor = Cast<ASHVVoiceActor>(GetOwner());
	if (!VoiceActor || !EncodeCodec.IsValid())
	{
		return;
	}

	const USHVSettings* Settings = GetDefault<USHVSettings>();
	TArray<uint8> Encoded;
	if (!EncodeCodec->Encode(RawPcmFrame, Encoded))
	{
		return;
	}
	if (Encoded.Num() <= 0 || Encoded.Num() > Settings->MaxPacketBytes)
	{
		UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropping local voice packet before send: encoded size %d exceeds configured bounds."), Encoded.Num());
		return;
	}

	FSHVVoicePacket Packet;
	Packet.Payload = MoveTemp(Encoded);
	Packet.Codec = ActiveCodecId;
	Packet.VoiceMode = ActiveMode;
	Packet.RadioChannel = ActiveRadioChannel;
	Packet.RequestedRange = VoiceActor->VoiceRange;
	Packet.Sequence = ++Sequence;
	Packet.SampleRate = Settings->SampleRate;
	Packet.NumChannels = Settings->NumChannels;
	Packet.FrameDurationMs = EncodeCodec->GetFrameDurationMs();

	VoiceActor->ServerSubmitVoicePacket(Packet);
}

void USHVVoiceComponent::SetTalkingState(bool bNewTalking)
{
	if (bIsTalking != bNewTalking)
	{
		bIsTalking = bNewTalking;
		OnSpeakingStateChanged.Broadcast(bIsTalking);
	}
}

int32 USHVVoiceComponent::CalculateAverageAmplitude(const TArray<uint8>& PcmBytes) const
{
	if (PcmBytes.Num() < static_cast<int32>(sizeof(int16)))
	{
		return 0;
	}

	const int16* Samples = reinterpret_cast<const int16*>(PcmBytes.GetData());
	const int32 SampleCount = PcmBytes.Num() / sizeof(int16);
	int64 Sum = 0;
	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		Sum += FMath::Abs(static_cast<int32>(Samples[Index]));
	}

	return static_cast<int32>(Sum / FMath::Max(1, SampleCount));
}

FSHVRemoteSpeakerPlayback& USHVVoiceComponent::FindOrCreatePlayback(int32 SpeakerId)
{
	if (FSHVRemoteSpeakerPlayback* ExistingPlayback = RemotePlayback.Find(SpeakerId))
	{
		return *ExistingPlayback;
	}

	FSHVRemoteSpeakerPlayback& Playback = RemotePlayback.Add(SpeakerId);

	const USHVSettings* Settings = GetDefault<USHVSettings>();
	Playback.SoundWave = NewObject<USoundWaveProcedural>(this);
	Playback.SoundWave->SetSampleRate(Settings->SampleRate);
	Playback.SoundWave->NumChannels = Settings->NumChannels;
	Playback.SoundWave->Duration = INDEFINITELY_LOOPING_DURATION;
	Playback.SoundWave->SoundGroup = SOUNDGROUP_Voice;
	Playback.SoundWave->bLooping = false;
	Playback.SoundWave->bCanProcessAsync = true;

	Playback.AudioComponent = NewObject<UAudioComponent>(GetOwner());
	Playback.AudioComponent->SetSound(Playback.SoundWave);
	Playback.AudioComponent->SetVolumeMultiplier(OutputVolume);
	Playback.AudioComponent->bAutoActivate = false;
	if (const AActor* OwnerActor = GetOwner())
	{
		if (USceneComponent* Root = OwnerActor->GetRootComponent())
		{
			Playback.AudioComponent->SetupAttachment(Root);
		}
	}
	Playback.AudioComponent->RegisterComponent();

	return Playback;
}

bool USHVVoiceComponent::CanCaptureLocally() const
{
	if (IsRunningDedicatedServer())
	{
		return false;
	}

	const ASHVVoiceActor* VoiceActor = Cast<ASHVVoiceActor>(GetOwner());
	return VoiceActor && VoiceActor->IsLocalVoiceActor();
}

void USHVVoiceComponent::PruneStalePlayback()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastPlaybackPruneTime < 1.0f)
	{
		return;
	}
	LastPlaybackPruneTime = Now;

	const float Timeout = GetDefault<USHVSettings>()->RemoteSpeakerTimeout;
	for (auto Iterator = RemotePlayback.CreateIterator(); Iterator; ++Iterator)
	{
		FSHVRemoteSpeakerPlayback& Playback = Iterator.Value();
		if (Now - Playback.LastPacketTime <= Timeout)
		{
			continue;
		}

		if (Playback.AudioComponent)
		{
			Playback.AudioComponent->Stop();
			Playback.AudioComponent->DestroyComponent();
		}
		Iterator.RemoveCurrent();
	}
}
