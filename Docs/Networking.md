# Networking

Each player receives one server-spawned `ASHVVoiceActor`. The actor owner must be the player's `APlayerController`; this gives the owning client permission to call server RPCs.

## Packet flow

1. Owning client captures microphone audio.
2. The component encodes audio through `ISHVVoiceCodec`.
3. The owning voice actor sends `ServerSubmitVoicePacket`.
4. The server validates codec, packet size, send rate, audio format, voice mode, range, radio membership, and mute state.
5. The server routes packets to target voice actors with client RPCs.
6. Receiving clients decode and queue procedural audio.

Private, team, radio, and proximity traffic uses targeted client RPCs. Global traffic also uses targeted RPCs in this first implementation so routing behavior remains consistent and easy to audit.

## Codec metadata

Every packet includes:

- codec id
- sample rate
- channel count
- frame duration
- sequence
- selected voice mode
- optional radio channel

The server routes compressed packets without decoding them. Receiving clients reject packets whose codec or format is unsupported by the current build/settings.

## Dedicated server behavior

Dedicated servers never initialize microphone capture, playback audio components, or procedural sound waves. They only validate and route compressed packets.
