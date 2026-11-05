# Minimal Demo Setup

This plugin intentionally ships without content assets. `CanContainContent=false` keeps the package clean for code-plugin distribution.

## GameMode

In Blueprint:

1. Open your GameMode.
2. Add `Event OnPostLogin`.
3. Call `Spawn Voice Actor For Player`.
4. Connect `New Player` to `Player Controller`.

In C++:

```cpp
void AMyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	USHVBlueprintLibrary::SpawnVoiceActorForPlayer(this, NewPlayer, nullptr);
}
```

## Push to talk

On the owning client:

1. On key/button pressed, call `Start Voice Capture`.
2. On key/button released, call `Stop Voice Capture`.
3. Use `Get Current Active Codec` to confirm Opus or Raw PCM.
4. Use `Is Local Player Talking` for UI mic indicators.

## Test

Run a listen server and one client, then repeat with a dedicated server and two clients.
