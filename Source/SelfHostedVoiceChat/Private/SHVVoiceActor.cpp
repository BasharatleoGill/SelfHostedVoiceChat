#include "SHVVoiceActor.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "SHVCodecInterface.h"
#include "SHVSettings.h"
#include "SHVSubsystem.h"
#include "SHVVoiceComponent.h"

ASHVVoiceActor::ASHVVoiceActor()
{
	bReplicates = true;
	bAlwaysRelevant = false;
	SetReplicateMovement(false);
	PrimaryActorTick.bCanEverTick = false;

	VoiceComponent = CreateDefaultSubobject<USHVVoiceComponent>(TEXT("VoiceComponent"));
	VoiceRange = GetDefault<USHVSettings>()->DefaultProximityRange;
}

void ASHVVoiceActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (APlayerController* OwningPC = Cast<APlayerController>(GetOwner()))
		{
			OwnerPlayerState = OwningPC->PlayerState;
			OwningPC->OnDestroyed.AddDynamic(this, &ASHVVoiceActor::HandleOwnerDestroyed);
		}
		else
		{
			UE_LOG(LogSelfHostedVoiceChat, Warning, TEXT("Voice actor %s has no PlayerController owner on the server."), *GetName());
		}

		if (USHVSubsystem* Subsystem = GetSHVSubsystem())
		{
			Subsystem->RegisterServerVoiceActor(this);
		}
	}

	RegisterWithSubsystems();
}

void ASHVVoiceActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (VoiceComponent)
	{
		VoiceComponent->ShutdownAudio();
	}

	if (USHVSubsystem* Subsystem = GetSHVSubsystem())
	{
		Subsystem->UnregisterLocalVoiceActor(this);
		Subsystem->UnregisterServerVoiceActor(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ASHVVoiceActor::OnRep_Owner()
{
	Super::OnRep_Owner();
	RegisterWithSubsystems();
}

void ASHVVoiceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHVVoiceActor, TeamId);
	DOREPLIFETIME(ASHVVoiceActor, VoiceRange);
	DOREPLIFETIME(ASHVVoiceActor, bGlobalVoiceEnabled);
	DOREPLIFETIME(ASHVVoiceActor, bProximityVoiceEnabled);
	DOREPLIFETIME(ASHVVoiceActor, bTeamVoiceEnabled);
	DOREPLIFETIME(ASHVVoiceActor, bServerAllowsGlobalVoice);
	DOREPLIFETIME(ASHVVoiceActor, bServerAllowsProximityVoice);
	DOREPLIFETIME(ASHVVoiceActor, bServerAllowsTeamVoice);
	DOREPLIFETIME(ASHVVoiceActor, bServerMuted);
	DOREPLIFETIME(ASHVVoiceActor, RadioChannels);
	DOREPLIFETIME(ASHVVoiceActor, OwnerPlayerState);
}

APlayerState* ASHVVoiceActor::GetOwningPlayerState() const
{
	if (OwnerPlayerState)
	{
		return OwnerPlayerState;
	}

	if (const APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		return PC->PlayerState;
	}

	return nullptr;
}

void ASHVVoiceActor::SetTeamIdAuthority(int32 InTeamId)
{
	if (HasAuthority())
	{
		TeamId = InTeamId;
	}
}

void ASHVVoiceActor::SetServerMutedAuthority(bool bInServerMuted)
{
	if (HasAuthority())
	{
		bServerMuted = bInServerMuted;
	}
}

void ASHVVoiceActor::SetVoiceRangeAuthority(float InVoiceRange)
{
	if (HasAuthority())
	{
		const USHVSettings* Settings = GetDefault<USHVSettings>();
		VoiceRange = FMath::Clamp(InVoiceRange, 0.0f, Settings->MaxProximityRange);
	}
}

void ASHVVoiceActor::SetVoiceModesAuthority(bool bInGlobalEnabled, bool bInProximityEnabled, bool bInTeamEnabled)
{
	if (HasAuthority())
	{
		bServerAllowsGlobalVoice = bInGlobalEnabled;
		bServerAllowsProximityVoice = bInProximityEnabled;
		bServerAllowsTeamVoice = bInTeamEnabled;
	}
}

bool ASHVVoiceActor::IsLocalVoiceActor() const
{
	return IsOwnedByLocalPlayer();
}

bool ASHVVoiceActor::CheckJoinedRadioChannel(FName ChannelName) const
{
	return !ChannelName.IsNone() && RadioChannels.Contains(ChannelName);
}

void ASHVVoiceActor::ServerJoinRadioChannel_Implementation(FName ChannelName)
{
	if (ChannelName.IsNone())
	{
		return;
	}

	const USHVSettings* Settings = GetDefault<USHVSettings>();
	if (RadioChannels.Num() >= Settings->MaxRadioChannelsPerPlayer || RadioChannels.Contains(ChannelName))
	{
		return;
	}

	RadioChannels.Add(ChannelName);
}

void ASHVVoiceActor::ServerLeaveRadioChannel_Implementation(FName ChannelName)
{
	RadioChannels.Remove(ChannelName);
}

void ASHVVoiceActor::ServerSetVoiceRange_Implementation(float InVoiceRange)
{
	SetVoiceRangeAuthority(InVoiceRange);
}

void ASHVVoiceActor::ServerSetVoiceModes_Implementation(bool bInGlobalEnabled, bool bInProximityEnabled, bool bInTeamEnabled)
{
	bGlobalVoiceEnabled = bInGlobalEnabled;
	bProximityVoiceEnabled = bInProximityEnabled;
	bTeamVoiceEnabled = bInTeamEnabled;
}

void ASHVVoiceActor::ServerSubmitVoicePacket_Implementation(const FSHVVoicePacket& Packet)
{
	if (!ValidateIncomingPacket(Packet))
	{
		return;
	}

	RouteVoicePacket(Packet);
}

void ASHVVoiceActor::ClientReceiveVoicePacket_Implementation(APlayerState* SenderPlayerState, const FSHVVoicePacket& Packet)
{
	if (VoiceComponent && !IsRunningDedicatedServer())
	{
		VoiceComponent->ReceiveVoicePacket(SenderPlayerState, Packet);
	}
}

void ASHVVoiceActor::HandleOwnerDestroyed(AActor* DestroyedActor)
{
	if (HasAuthority() && !IsActorBeingDestroyed())
	{
		Destroy();
	}
}

void ASHVVoiceActor::RegisterWithSubsystems()
{
	if (USHVSubsystem* Subsystem = GetSHVSubsystem())
	{
		if (IsOwnedByLocalPlayer())
		{
			Subsystem->RegisterLocalVoiceActor(this);
		}
	}
}

USHVSubsystem* ASHVVoiceActor::GetSHVSubsystem() const
{
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<USHVSubsystem>();
		}
	}
	return nullptr;
}

bool ASHVVoiceActor::IsOwnedByLocalPlayer() const
{
	if (IsRunningDedicatedServer())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	APlayerController* LocalPC = World ? World->GetFirstPlayerController() : nullptr;
	return IsValid(LocalPC) && IsOwnedBy(LocalPC);
}

bool ASHVVoiceActor::ValidateIncomingPacket(const FSHVVoicePacket& Packet)
{
	const USHVSettings* Settings = GetDefault<USHVSettings>();
	if (!Settings)
	{
		return false;
	}

	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (!OwningPC || !GetOwningPlayerState())
	{
		DropStats.UnauthorizedPackets++;
		UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet from %s: invalid owner/player state."), *GetName());
		return false;
	}

	if (bServerMuted)
	{
		DropStats.UnauthorizedPackets++;
		UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet from %s: server muted."), *GetName());
		return false;
	}

	if (Packet.Payload.Num() <= 0 || Packet.Payload.Num() > Settings->MaxPacketBytes)
	{
		DropStats.OversizedPackets++;
		UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet from %s: invalid packet size %d."), *GetName(), Packet.Payload.Num());
		return false;
	}

	if (!PassesRateLimit())
	{
		DropStats.RateLimitedPackets++;
		UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet from %s: rate limited."), *GetName());
		return false;
	}

	if (Packet.SampleRate != Settings->SampleRate || Packet.NumChannels != Settings->NumChannels || Packet.FrameDurationMs != Settings->FrameDurationMs)
	{
		DropStats.UnsupportedModePackets++;
		UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet from %s: audio format mismatch."), *GetName());
		return false;
	}

	if (Packet.Codec == ESHVVoiceCodecId::Opus && !SHVIsOpusCodecAvailable())
	{
		DropStats.UnsupportedModePackets++;
		UE_LOG(LogSelfHostedVoiceChat, VeryVerbose, TEXT("Dropped voice packet from %s: Opus packet received but Opus is unavailable in this build."), *GetName());
		return false;
	}

	switch (Packet.VoiceMode)
	{
	case ESHVVoiceMode::Global:
		if (!Settings->bAllowGlobalVoice || !bServerAllowsGlobalVoice || !bGlobalVoiceEnabled)
		{
			DropStats.UnsupportedModePackets++;
			return false;
		}
		break;
	case ESHVVoiceMode::Proximity:
		if (!Settings->bAllowProximityVoice || !bServerAllowsProximityVoice || !bProximityVoiceEnabled || Packet.RequestedRange < 0.0f || Packet.RequestedRange > Settings->MaxProximityRange)
		{
			DropStats.UnsupportedModePackets++;
			return false;
		}
		break;
	case ESHVVoiceMode::Team:
		if (!Settings->bAllowTeamVoice || !bServerAllowsTeamVoice || !bTeamVoiceEnabled || TeamId == INDEX_NONE)
		{
			DropStats.UnsupportedModePackets++;
			return false;
		}
		break;
	case ESHVVoiceMode::Radio:
		if (!Settings->bAllowRadioVoice || Packet.RadioChannel.IsNone() || !RadioChannels.Contains(Packet.RadioChannel))
		{
			DropStats.UnsupportedModePackets++;
			return false;
		}
		break;
	default:
		DropStats.UnsupportedModePackets++;
		return false;
	}

	return true;
}

bool ASHVVoiceActor::PassesRateLimit()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - RateWindowStartSeconds >= 1.0f)
	{
		RateWindowStartSeconds = Now;
		PacketsInRateWindow = 0;
	}

	PacketsInRateWindow++;
	return PacketsInRateWindow <= GetDefault<USHVSettings>()->MaxPacketsPerSecond;
}

void ASHVVoiceActor::RouteVoicePacket(const FSHVVoicePacket& Packet)
{
	USHVSubsystem* Subsystem = GetSHVSubsystem();
	if (!Subsystem)
	{
		return;
	}

	APlayerState* SenderPlayerState = GetOwningPlayerState();
	if (!IsValid(SenderPlayerState))
	{
		DropStats.UnauthorizedPackets++;
		return;
	}

	for (ASHVVoiceActor* TargetActor : Subsystem->GetServerVoiceActors())
	{
		if (IsValid(TargetActor) && TargetActor != this && ShouldRouteTo(TargetActor, Packet))
		{
			TargetActor->ClientReceiveVoicePacket(SenderPlayerState, Packet);
		}
	}
}

bool ASHVVoiceActor::ShouldRouteTo(ASHVVoiceActor* TargetActor, const FSHVVoicePacket& Packet) const
{
	if (!IsValid(TargetActor) || TargetActor->bServerMuted || !Cast<APlayerController>(TargetActor->GetOwner()))
	{
		return false;
	}

	switch (Packet.VoiceMode)
	{
	case ESHVVoiceMode::Global:
		return true;
	case ESHVVoiceMode::Proximity:
		return GetDistanceToVoiceActor(TargetActor) <= FMath::Min(Packet.RequestedRange > 0.0f ? Packet.RequestedRange : VoiceRange, GetDefault<USHVSettings>()->MaxProximityRange);
	case ESHVVoiceMode::Team:
		return TeamId != INDEX_NONE && TeamId == TargetActor->TeamId;
	case ESHVVoiceMode::Radio:
		return !Packet.RadioChannel.IsNone() && TargetActor->RadioChannels.Contains(Packet.RadioChannel);
	default:
		return false;
	}
}

float ASHVVoiceActor::GetDistanceToVoiceActor(const ASHVVoiceActor* TargetActor) const
{
	const APlayerController* SourcePC = Cast<APlayerController>(GetOwner());
	const APlayerController* TargetPC = TargetActor ? Cast<APlayerController>(TargetActor->GetOwner()) : nullptr;
	const APawn* SourcePawn = SourcePC ? SourcePC->GetPawn() : nullptr;
	const APawn* TargetPawn = TargetPC ? TargetPC->GetPawn() : nullptr;

	if (!SourcePawn || !TargetPawn)
	{
		return TNumericLimits<float>::Max();
	}

	return FVector::Dist(SourcePawn->GetActorLocation(), TargetPawn->GetActorLocation());
}
