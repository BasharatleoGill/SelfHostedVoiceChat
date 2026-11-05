# Self Hosted Voice Chat

Self Hosted Voice Chat is a clean-room Unreal Engine runtime code plugin for self-hosted RPC voice communication. It does not depend on EOS, Vivox, PlayFab, or any hosted voice service.

## Current implementation

- Runtime C++ module: `SelfHostedVoiceChat`
- Game instance subsystem for local state, mute lists, and voice actor registration
- Server-spawned replicated voice actor owned by each player controller
- Windows microphone capture through Unreal's Voice module
- Blueprint microphone input device list/selection
- Blueprint audio output device list/selection through Unreal's audio mixer
- Opus codec through Unreal Engine `libOpus` on Win64 when available
- Raw PCM development codec fallback for compile and network-path testing
- Global, proximity, team, and radio routing on the server
- Client-side mute/deafen and output volume controls
- Optional GameMode base class that spawns/destroys voice actors automatically

## Codec modes

`USHVSettings::CodecMode` supports:

- `Auto`: prefer Opus when linked, fallback to Raw PCM with a warning
- `Opus`: request Opus, fallback to Raw PCM if unavailable
- `RawPCM`: use uncompressed PCM for development

Opus is recommended for production. Raw PCM is intentionally bandwidth-heavy and should mainly be used for debugging codec and transport behavior.

## Bandwidth expectations

With the default 16 kHz mono, 20 ms frames, and 24 kbps Opus bitrate, each speaking player sends roughly 50 packets per second and usually around 3-5 KB/s of encoded voice before Unreal packet overhead. Raw PCM at the same settings is about 32 KB/s before overhead.

## Opus libraries

On UE 5.7 Win64, this plugin links the engine bundled `libOpus` module when available. For standalone plugin distribution, optional local binaries can be placed in `Source/ThirdParty/SHVOpus` as described in that folder's README.

## Demo setup

This is a code-only plugin. See `Docs/DemoSetup.md` for the minimal GameMode `PostLogin` and push-to-talk setup.

## Required project config

Add this to your project's `Config/DefaultEngine.ini` so Unreal's Voice module is enabled for capture:

```ini
[OnlineSubsystem]
bHasVoiceEnabled=true

[Voice]
bEnabled=true
```

## Audio devices

Use `Get Audio Input Devices` and `Set Audio Input Device` to choose the microphone used by local capture. Use `Get Audio Output Devices` and `Set Audio Output Device` to choose the Unreal audio mixer's main output device. Unreal's built-in output swap changes the game's main audio output, so voice playback follows that device along with normal game audio.
