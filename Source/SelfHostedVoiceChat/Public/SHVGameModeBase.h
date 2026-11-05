#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SHVGameModeBase.generated.h"

class ASHVVoiceActor;

UCLASS()
class SELFHOSTEDVOICECHAT_API ASHVGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable, Category = "Self Hosted Voice Chat|Setup", meta = (DisplayName = "Spawn Voice Actor For New Player", Keywords = "voice chat post login game mode spawn"))
	ASHVVoiceActor* SpawnVoiceActorForNewPlayer(APlayerController* NewPlayer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Self Hosted Voice Chat")
	TSubclassOf<ASHVVoiceActor> VoiceActorClass;
};
