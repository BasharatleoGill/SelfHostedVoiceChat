# Optional Local Opus Libraries

The plugin can use Unreal Engine's bundled `libOpus` module on Win64. For standalone distribution builds that must bundle their own Opus binaries, place files here:

```text
Source/ThirdParty/SHVOpus/
  Include/opus.h
  Include/opus_defines.h
  Include/opus_types.h
  Lib/Win64/Release/opus.lib
```

When these files are present, `SelfHostedVoiceChat.Build.cs` links this local `SHVOpus` module. Otherwise it attempts to use the engine `libOpus` module. If neither is available, `SHV_WITH_OPUS=0` and the plugin falls back to Raw PCM.
