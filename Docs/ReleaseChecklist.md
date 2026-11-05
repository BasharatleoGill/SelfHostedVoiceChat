# Fab Release Checklist

## Metadata

- Friendly name is set.
- Description explains self-hosted RPC voice and Opus.
- Category is Networking.
- `CanContainContent=false` because no demo assets are included.
- `Resources/Icon128.png` is present.

## Code

- Builds in Development Editor.
- Builds in Shipping.
- Dedicated server does not initialize capture/playback audio.
- Voice packets are server validated.
- Voice actor ownership is required for client-to-server RPC.
- Actor references are weak or cleaned on EndPlay/world cleanup.
- No project-specific references.
- No references to studied third-party plugin names or code.

## Runtime Test

- Listen server plus client.
- Dedicated server plus two clients.
- Global, proximity, team, and radio voice.
- Mute/deafen.
- Disconnect/reconnect.
- Map travel.
- Unsupported microphone/platform behavior.

## Packaging

- Remove generated `Binaries`, `Intermediate`, and local saved artifacts before submission unless Fab specifically requests binaries.
- Verify third-party license notices for any bundled local Opus binaries.
- Confirm advertised platform list matches tested platforms. Initial release advertises Win64 only.
