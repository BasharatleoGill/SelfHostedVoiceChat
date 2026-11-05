# Product Description

Self Hosted Voice Chat is a C++ Unreal Engine plugin for projects that want direct, server-routed voice communication without relying on hosted voice providers. It provides a replicated per-player voice actor, client microphone capture, Opus voice compression, server-side packet validation, global/proximity/team/radio routing, local mute/deafen controls, and Blueprint-friendly setup nodes.

## Packaging checklist

- Confirm `Resources/Icon128.png` is present.
- Compile Development Editor.
- Compile Shipping.
- Test listen server with two clients.
- Test dedicated server with two clients.
- Test map travel.
- Test player disconnect/reconnect.
- Test no microphone / permission denied.
- Verify Opus is active on all advertised target platforms.
- Remove unused generated build artifacts.
- Verify no dependency on any third-party studied plugin.
