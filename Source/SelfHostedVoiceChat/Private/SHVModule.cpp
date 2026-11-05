#include "Modules/ModuleManager.h"
#include "SHVTypes.h"

DEFINE_LOG_CATEGORY(LogSelfHostedVoiceChat);

class FSelfHostedVoiceChatModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("SelfHostedVoiceChat module started."));
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogSelfHostedVoiceChat, Log, TEXT("SelfHostedVoiceChat module stopped."));
	}
};

IMPLEMENT_MODULE(FSelfHostedVoiceChatModule, SelfHostedVoiceChat)
