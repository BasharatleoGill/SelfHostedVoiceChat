# Changelog

## 1.0.0

- Initial clean-room plugin skeleton.
- Subsystem, voice actor, voice component, Blueprint API, and GameMode helper.
- Raw PCM development codec.
- Server-side routing and validation for global, proximity, team, and radio voice.

## 1.1.0

- Added production Opus codec integration behind `ISHVVoiceCodec`.
- Added `CodecMode` setting with Auto, Opus, and Raw PCM.
- Added Opus bitrate, complexity, VBR, packet loss, and frame duration settings.
- Added codec metadata to voice packets.
- Added Blueprint functions for active codec and Opus availability.
- Added optional local `Source/ThirdParty/SHVOpus` integration structure.
