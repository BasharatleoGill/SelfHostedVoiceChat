# Quick Start

## Option A: Blueprint/GameMode setup

1. Enable the `Self Hosted Voice Chat` plugin.
2. In your GameMode `Event OnPostLogin`, call `Spawn Voice Actor For Player`.
3. Pass `New Player` as the player controller.
4. On the owning client, wait until `Get Local Voice Chat Actor` returns valid.
5. Call `Start Voice Capture`.
6. Call `Stop Voice Capture` when push-to-talk is released or voice is disabled.

## Required DefaultEngine.ini

Add this to `Config/DefaultEngine.ini` in the project using the plugin:

```ini
[OnlineSubsystem]
bHasVoiceEnabled=true

[Voice]
bEnabled=true
```

## Codec setup

Open Project Settings > Self Hosted Voice Chat:

- Use `CodecMode=Auto` for normal development and production.
- Use `CodecMode=Opus` to require Opus preference with fallback logging.
- Use `CodecMode=RawPCM` only for local debugging.
- Defaults are 16 kHz, mono, 20 ms frames, 24 kbps Opus.

## Option B: C++ base GameMode

Use `ASHVGameModeBase` as a parent class. It automatically spawns a voice actor in `PostLogin` and destroys it in `Logout`.

## No demo content

This package is a code-only plugin and has `CanContainContent=false`. The minimal demo setup is the GameMode `PostLogin` flow above plus a widget or input binding that calls `Start Voice Capture` and `Stop Voice Capture`.

## Voice modes

- Global: `Start Voice Capture` with mode `Global`
- Proximity: call `Set Voice Range`, then start with mode `Proximity`
- Team: set team ID on the server, then start with mode `Team`
- Radio: call `Join Radio Channel`, then start with mode `Radio`; `Start Voice Capture` also auto-joins the requested channel if needed.

By default one player can join multiple radio channels. Limit this with `MaxRadioChannelsPerPlayer`, or manually leave the previous channel before joining a new one.

## Two-client test

1. Start a listen server or dedicated server.
2. Connect two clients.
3. Confirm each player has a spawned `ASHVVoiceActor`.
4. On client A call `Start Voice Capture`.
5. On client B verify remote voice playback and `Is Opus Available`.
6. Repeat with global, proximity, team, and radio modes.
