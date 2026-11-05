# Troubleshooting

## `Get Local Voice Chat Actor` returns none

Make sure the server spawned `ASHVVoiceActor` and set the owner to the player's `PlayerController`.

## No microphone audio

- Windows is the only production capture backend in this first pass.
- Check OS microphone permission.
- Use `Get Audio Input Devices` and select the intended microphone with `Set Audio Input Device`.
- Make sure `Start Voice Capture` is called on the owning client.
- Check the log category `LogSelfHostedVoiceChat`.

## Voice plays from the wrong speaker/headset

- Use `Get Audio Output Devices` and pass the selected `DeviceId` into `Set Audio Output Device`.
- Dedicated servers do not have output devices.
- Unreal's audio mixer output swap affects the main game output device. Voice-only output routing would require a separate submix endpoint implementation.

## Other clients cannot hear voice

- Confirm voice actor exists for each player.
- Check server packet drop stats on the sender actor.
- Check packet size and rate limits in settings.
- Confirm sender and receiver have matching sample rate, channel count, frame duration, and codec support.
- For radio mode, both sender and receiver must be in the same channel.
- For proximity mode, both pawns must be valid and inside range.

## Opus is not active

- Call `Is Opus Available`.
- Check that `CodecMode` is `Auto` or `Opus`.
- On UE 5.7 Win64, the plugin should link engine `libOpus`.
- For standalone distribution, add local Opus headers/libs to `Source/ThirdParty/SHVOpus`.
- If Opus initialization fails, logs will show the reason and the plugin falls back to Raw PCM.
