# Build Instructions

## Development Editor

Run UnrealBuildTool for your project editor target:

```powershell
<UE_ROOT>\Engine\Build\BatchFiles\Build.bat <ProjectName>Editor Win64 Development -Project="<PROJECT_ROOT>\<ProjectName>.uproject" -WaitMutex
```

## Shipping

Use your game target in Shipping configuration:

```powershell
<UE_ROOT>\Engine\Build\BatchFiles\Build.bat <ProjectName> Win64 Shipping -Project="<PROJECT_ROOT>\<ProjectName>.uproject" -WaitMutex
```

## Notes

- Win64 capture is implemented through Unreal's Voice module.
- Win64 Opus links through Unreal Engine's bundled `libOpus` when available.
- Non-Windows platforms compile with guarded unsupported-capture behavior until native backends are added.
- Raw PCM remains available as a development fallback.
