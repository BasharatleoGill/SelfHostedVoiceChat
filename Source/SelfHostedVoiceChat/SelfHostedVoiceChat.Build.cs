using UnrealBuildTool;
using System.IO;

public class SelfHostedVoiceChat : ModuleRules
{
	public SelfHostedVoiceChat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"NetCore",
				"AudioMixer",
				"AudioCapture",
				"Voice",
				"Projects",
				"DeveloperSettings",
				"Media",
				"MediaUtils"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore"
			});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.Add("SHV_PLATFORM_CAPTURE_SUPPORTED=1");
		}
		else
		{
			PublicDefinitions.Add("SHV_PLATFORM_CAPTURE_SUPPORTED=0");
		}

		bool bUseEngineOpus = Target.Platform == UnrealTargetPlatform.Win64 &&
			Directory.Exists(Path.Combine(EngineDirectory, "Source", "ThirdParty", "libOpus"));

		bool bUseLocalOpus = Target.Platform == UnrealTargetPlatform.Win64 &&
			File.Exists(Path.Combine(ModuleDirectory, "..", "ThirdParty", "SHVOpus", "Include", "opus.h")) &&
			File.Exists(Path.Combine(ModuleDirectory, "..", "ThirdParty", "SHVOpus", "Lib", "Win64", "Release", "opus.lib"));

		if (bUseLocalOpus)
		{
			PrivateDependencyModuleNames.Add("SHVOpus");
			PublicDefinitions.Add("SHV_WITH_OPUS=1");
			PublicDefinitions.Add("SHV_OPUS_SOURCE=1");
		}
		else if (bUseEngineOpus)
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "libOpus");
			PublicDefinitions.Add("SHV_WITH_OPUS=1");
			PublicDefinitions.Add("SHV_OPUS_SOURCE=2");
		}
		else
		{
			PublicDefinitions.Add("SHV_WITH_OPUS=0");
			PublicDefinitions.Add("SHV_OPUS_SOURCE=0");
		}
	}
}
