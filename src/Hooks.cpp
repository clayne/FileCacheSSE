#include "Hooks.h"

#include "skse64_common/SafeWrite.h"

#include "WinAPICacher.h"

#include "REL/Relocation.h"


void InstallHooks()
{
	WinAPICacher::InstallHooks();
}
