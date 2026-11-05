using UnrealBuildTool;
using System.IO;

public class SHVOpus : ModuleRules
{
	public SHVOpus(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string BasePath = ModuleDirectory;
		string IncludePath = Path.Combine(BasePath, "Include");
		string Win64Lib = Path.Combine(BasePath, "Lib", "Win64", "Release", "opus.lib");

		if (Target.Platform == UnrealTargetPlatform.Win64 && File.Exists(Path.Combine(IncludePath, "opus.h")) && File.Exists(Win64Lib))
		{
			PublicSystemIncludePaths.Add(IncludePath);
			PublicAdditionalLibraries.Add(Win64Lib);
		}
	}
}
