#include "SHVGameModeBase.h"

#include "GameFramework/PlayerController.h"
#include "SHVBlueprintLibrary.h"
#include "SHVSubsystem.h"
#include "SHVVoiceActor.h"

void ASHVGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	SpawnVoiceActorForNewPlayer(NewPlayer);
}

ASHVVoiceActor* ASHVGameModeBase::SpawnVoiceActorForNewPlayer(APlayerController* NewPlayer)
{
	return USHVBlueprintLibrary::SpawnVoiceActorForPlayer(this, NewPlayer, VoiceActorClass);
}

void ASHVGameModeBase::Logout(AController* Exiting)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Exiting))
	{
		if (UWorld* World = GetWorld())
		{
			if (USHVSubsystem* Subsystem = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<USHVSubsystem>() : nullptr)
			{
				if (ASHVVoiceActor* VoiceActor = Subsystem->FindVoiceActorForController(PlayerController))
				{
					VoiceActor->Destroy();
				}
			}
		}
	}

	Super::Logout(Exiting);
}
