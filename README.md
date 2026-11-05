# Self Hosted Voice Chat

A self-hosted Unreal Engine runtime plugin for server-routed multiplayer voice chat. It uses Unreal RPCs and does not require EOS Voice, Vivox, PlayFab, or another hosted voice provider.

## Features

- Server-authoritative global, proximity, team, and radio voice routing
- Per-player replicated voice actor owned by the player controller
- Win64 microphone capture through Unreal's Voice module
- Opus compression through Unreal Engine's bundled `libOpus` when available
- Raw PCM fallback for transport/debugging
- Blueprint microphone and output-device selection
- Local mute, deafen, input/output volume controls, and GameMode setup helpers

## Installation

1. Copy this `SelfHostedVoiceChat` folder into `<YourProject>/Plugins/`.
2. Regenerate project files and compile the project.
3. Enable **Self Hosted Voice Chat** in Unreal Editor if prompted.
4. Add the following to `Config/DefaultEngine.ini`:

```ini
[OnlineSubsystem]
bHasVoiceEnabled=true

[Voice]
bEnabled=true
```

5. Follow [Docs/QuickStart.md](Docs/QuickStart.md) for the required `PostLogin`, `Logout`, and push-to-talk Blueprint setup.

## Requirements

- Unreal Engine 5.7 or a compatible Win64 engine build
- A multiplayer session with a listen or dedicated server
- The same audio-format settings on the server and all clients

## Documentation

- [Quick Start](Docs/QuickStart.md)
- [Blueprint API](Docs/API.md)
- [Networking and authority](Docs/Networking.md)
- [Build instructions](Docs/BuildInstructions.md)
- [Troubleshooting](Docs/Troubleshooting.md)
- [Third-party notes](THIRD_PARTY_NOTICES.md)

## License

The plugin source in this repository is released under the [MIT License](LICENSE). Unreal Engine and its bundled `libOpus` module are not included and remain subject to Epic Games' and their respective licensors' terms.
