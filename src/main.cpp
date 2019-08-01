#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/skse_version.h"

#include <sstream>

#include "Hooks.h"
#include "version.h"

#include "SKSE/API.h"


namespace
{
	class StopWatch
	{
	public:
		static StopWatch* GetSingleton()
		{
			static StopWatch singleton;
			return &singleton;
		}

		void Start()
		{
			_start = std::chrono::high_resolution_clock::now();
		}

		void Stop()
		{
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> diff = end - _start;
			std::stringstream strstr;
			strstr << "Loading took: " << diff.count() << "s";
			_MESSAGE("%s", strstr.str().c_str());
			auto console = RE::ConsoleManager::GetSingleton();
			if (console) {
				console->Print("%s", strstr.str().c_str());
			}
		}

	private:
		StopWatch() = default;
		StopWatch(const StopWatch&) = delete;
		StopWatch(StopWatch&&) = delete;
		~StopWatch() = default;

		StopWatch& operator=(const StopWatch&) = delete;
		StopWatch& operator=(StopWatch&&) = delete;

		std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	};


	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kPostPostLoad:
			{
				auto stopWatch = StopWatch::GetSingleton();
				stopWatch->Start();
			}
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			{
				auto stopWatch = StopWatch::GetSingleton();
				stopWatch->Stop();
			}
			break;
		}
	}
}


extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		SKSE::Logger::OpenRelative(FOLDERID_Documents, L"\\My Games\\Skyrim Special Edition\\SKSE\\FileCacheSSE.log");
#if _DEBUG
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
#else
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kMessage);
#endif
		SKSE::Logger::UseLogStamp(false);

		_MESSAGE("FileCacheSSE v%s", FCSH_VERSION_VERSTRING);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = "FileCacheSSE";
		a_info->version = FCSH_VERSION_MAJOR;

		if (a_skse->IsEditor()) {
			_FATALERROR("Loaded in editor, marking as incompatible!\n");
			return false;
		}

		switch (a_skse->RuntimeVersion()) {
		case RUNTIME_VERSION_1_5_73:
		case RUNTIME_VERSION_1_5_80:
			break;
		default:
			_FATALERROR("Unsupported runtime version %08X!\n", a_skse->RuntimeVersion());
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		_MESSAGE("FileCacheSSE loaded");

		if (!SKSE::Init(a_skse)) {
			return false;
		}

		if (g_branchTrampoline.Create(1024 * 1)) {
			_MESSAGE("Branch trampoline creation successful");
		} else {
			_FATALERROR("Branch trampoline creation failed!\n");
			return false;
		}

		if (g_localTrampoline.Create(1024 * 1)) {
			_MESSAGE("Local trampoline creation successful");
		} else {
			_FATALERROR("Local trampoline creation failed!\n");
			return false;
		}

		auto messaging = SKSE::GetMessagingInterface();
		if (messaging->RegisterListener("SKSE", MessageHandler)) {
			_MESSAGE("Messaging interface registration successful");
		} else {
			_FATALERROR("Messaging interface registration failed!\n");
			return false;
		}

		InstallHooks();
		_MESSAGE("Installed hooks");

		return true;
	}
};
