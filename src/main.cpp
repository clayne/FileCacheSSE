#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/skse_version.h"

#include "Hooks.h"
#include "version.h"

#include "SKSE/API.h"


extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		SKSE::Logger::OpenRelative(FOLDERID_Documents, L"\\My Games\\Skyrim Special Edition\\SKSE\\FileCacheSSE.log");
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
#if _DEBUG
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
#endif

		_MESSAGE("FileCacheSSE v%s", FCSH_VERSION_VERSTRING);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = "FileCacheSSE";
		a_info->version = FCSH_VERSION_MAJOR;

		if (a_skse->IsEditor()) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		}

		switch (a_skse->RuntimeVersion()) {
		case RUNTIME_VERSION_1_5_73:
		case RUNTIME_VERSION_1_5_80:
			break;
		default:
			_FATALERROR("[FATAL ERROR] Unsupported runtime version %08X!\n", a_skse->RuntimeVersion());
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		_MESSAGE("[MESSAGE] FileCacheSSE loaded");

		if (!SKSE::Init(a_skse)) {
			return false;
		}

		if (g_branchTrampoline.Create(1024 * 1)) {
			_MESSAGE("[MESSAGE] Branch trampoline creation successful");
		} else {
			_FATALERROR("[FATAL ERROR] Branch trampoline creation failed!\n");
			return false;
		}

		if (g_localTrampoline.Create(1024 * 1)) {
			_MESSAGE("[MESSAGE] Local trampoline creation successful");
		} else {
			_FATALERROR("[FATAL ERROR] Local trampoline creation failed!\n");
			return false;
		}

		InstallHooks();
		_MESSAGE("[MESSAGE] Installed hooks");

		return true;
	}
};
