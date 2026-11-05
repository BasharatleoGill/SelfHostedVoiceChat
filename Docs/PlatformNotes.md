# Platform Notes

## Win64

Win64 uses Unreal's `Voice` module capture backend. Opus is linked through the engine `libOpus` module when present. Optional local Opus binaries can be supplied under `Source/ThirdParty/SHVOpus`.

## Linux, Mac, Android, iOS

The current release descriptor advertises Win64 only. Source code uses guarded unsupported-platform behavior so native capture backends can be added behind the `ISHVVoiceCaptureBackend` abstraction later. Opus can be enabled for those platforms by adding appropriate libraries to `SHVOpus.Build.cs` or relying on the engine `libOpus` module after validation.

## Codec

`Auto` mode prefers Opus and falls back to Raw PCM. Raw PCM is a development fallback and is not bandwidth efficient for public production.

Local Opus layout:

```text
Source/ThirdParty/SHVOpus/
  Include/opus.h
  Include/opus_defines.h
  Include/opus_types.h
  Lib/Win64/Release/opus.lib
```
