#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SHVTypes.h"
#include "SHVVoiceActor.generated.h"

class USHVVoiceComponent;
class USHVSubsystem;

UCLASS()
class SELFHOSTEDVOICECHAT_API ASHVVoiceActor : public AActor
{
	GENERATED_BODY()

public:
	ASHVVoiceActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_Owner() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	TObjectPtr<USHVVoiceComponent> VoiceComponent;

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	APlayerState* GetOwningPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetTeamIdAuthority(int32 InTeamId);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetServerMutedAuthority(bool bInServerMuted);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetVoiceRangeAuthority(float InVoiceRange);

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat")
	void SetVoiceModesAuthority(bool bInGlobalEnabled, bool bInProximityEnabled, bool bInTeamEnabled);

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat")
	bool IsLocalVoiceActor() const;

	UFUNCTION(BlueprintPure, Category = "Self Hosted Voice Chat|Radio", meta = (DisplayName = "Check Joined Radio Channel", Keywords = "radio channel joined registered voice chat"))
	bool CheckJoinedRadioChannel(FName ChannelName) const;

	UFUNCTION(Server, Reliable, Category = "Self Hosted Voice Chat")
	void ServerJoinRadioChannel(FName ChannelName);

	UFUNCTION(Server, Reliable, Category = "Self Hosted Voice Chat")
	void ServerLeaveRadioChannel(FName ChannelName);

	UFUNCTION(Server, Reliable, Category = "Self Hosted Voice Chat")
	void ServerSetVoiceRange(float InVoiceRange);

	UFUNCTION(Server, Reliable, Category = "Self Hosted Voice Chat")
	void ServerSetVoiceModes(bool bInGlobalEnabled, bool bInProximityEnabled, bool bInTeamEnabled);

	UFUNCTION(Server, Unreliable, Category = "Self Hosted Voice Chat")
	void ServerSubmitVoicePacket(const FSHVVoicePacket& Packet);

	UFUNCTION(Client, Unreliable, Category = "Self Hosted Voice Chat")
	void ClientReceiveVoicePacket(APlayerState* SenderPlayerState, const FSHVVoicePacket& Packet);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	float VoiceRange = 2000.0f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	bool bGlobalVoiceEnabled = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	bool bProximityVoiceEnabled = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	bool bTeamVoiceEnabled = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	bool bServerAllowsGlobalVoice = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	bool bServerAllowsProximityVoice = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	bool bServerAllowsTeamVoice = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	bool bServerMuted = false;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	TArray<FName> RadioChannels;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Self Hosted Voice Chat")
	TObjectPtr<APlayerState> OwnerPlayerState = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	FSHVVoiceDropStats DropStats;

private:
	UFUNCTION()
	void HandleOwnerDestroyed(AActor* DestroyedActor);

	void RegisterWithSubsystems();
	USHVSubsystem* GetSHVSubsystem() const;
	bool IsOwnedByLocalPlayer() const;
	bool ValidateIncomingPacket(const FSHVVoicePacket& Packet);
	bool PassesRateLimit();
	void RouteVoicePacket(const FSHVVoicePacket& Packet);
	bool ShouldRouteTo(ASHVVoiceActor* TargetActor, const FSHVVoicePacket& Packet) const;
	float GetDistanceToVoiceActor(const ASHVVoiceActor* TargetActor) const;

	float RateWindowStartSeconds = 0.0f;
	int32 PacketsInRateWindow = 0;
};
