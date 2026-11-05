# Blueprint API

- `Spawn Voice Actor For Player`: server-only helper for `GameMode::PostLogin`.
- `Destroy Voice Actor For Player`: server-only helper for `GameMode::Logout`.
- `Get Local Voice Chat Actor`: returns the owning client's registered voice actor.
- `Start Voice Capture`: starts local microphone capture.
- `Stop Voice Capture`: stops local microphone capture.
- `Set Voice Enabled`: convenience start/stop wrapper.
- `Set Input Volume`: local microphone gain before sending.
- `Set Output Volume`: local playback volume for remote voices.
- `Get Audio Input Devices` / `Set Audio Input Device`: list and select the microphone used by local capture.
- `Get Audio Output Devices` / `Set Audio Output Device`: list and select the Unreal audio mixer's main output device.
- `Get Selected Audio Input Device` / `Get Selected Audio Output Device Id`: return the currently selected device values cached by the subsystem.
- `Mute Player` / `Unmute Player` / `Is Player Muted`: local mute list.
- `Set Proximity Voice Enabled`: server option for the local voice actor.
- `Set Global Voice Enabled`: server option for the local voice actor.
- `Join Radio Channel` / `Leave Radio Channel`: server-validated channel membership.
- `Check Joined Radio Channel`: returns whether the local voice actor is registered to a radio channel.
- `Set Team Id`: server-authoritative helper; call from trusted server logic.
- `Set Voice Range`: server-clamped proximity range.
- `Is Local Player Talking`: local speaking-state query.
- `Get Current Active Codec`: returns `RawPCM` or `Opus` for the local voice actor.
- `Is Opus Available`: returns whether this build linked Opus support.

## Main C++ classes

- `USHVSubsystem`
- `ASHVVoiceActor`
- `USHVVoiceComponent`
- `USHVBlueprintLibrary`
- `ASHVGameModeBase`

## Settings

- `CodecMode`
- `SampleRate`
- `NumChannels`
- `DefaultInputDeviceName`
- `DefaultOutputDeviceId`
- `FrameDurationMs`
- `Bitrate`
- `OpusComplexity`
- `bOpusUseVbr`
- `OpusPacketLossPercent`
- `MaxPacketBytes`
- `MaxPacketsPerSecond`
- `RemoteSpeakerTimeout`
- `JitterBufferTargetFrames`
- `bEnableDebugLogs`

## Radio Channels

Radio channels use `Name` values, not integers. A player can be registered to more than one radio channel at the same time, up to `MaxRadioChannelsPerPlayer`. `Start Voice Capture` auto-joins the requested radio channel before capture starts if it is not already joined. For single-channel radio behavior, call `Leave Radio Channel` on the previous channel before joining the new one.
