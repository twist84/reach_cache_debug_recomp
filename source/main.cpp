// reach_cache_debug - ReXGlue Recompiled Project
//
// This file is yours to edit. 'rexglue migrate' will NOT overwrite it.

#include "generated/reach_cache_debug_config.h"
#include "generated/reach_cache_debug_init.h"

#include "reach_cache_debug_app.h"

REX_DEFINE_APP(reach_cache_debug, ReachCacheDebugApp::Create)

#include "rex_macros.h"

// Time

REX_PPC_HOOK(GetTickCount);
REX_PPC_HOOK(GetLocalTime);
REX_PPC_HOOK(SwitchToThread);
REX_PPC_HOOK(SleepEx);
REX_PPC_HOOK(Sleep);
REX_PPC_HOOK(GetSystemTime);
REX_PPC_HOOK(_time64);

// XBDM

REX_PPC_STUB(DmCloseLoadedModules);
REX_PPC_STUB(DmCloseModuleSections);
REX_PPC_STUB(DmCloseNotificationSession);
REX_PPC_STUB(DmGetMemory);
REX_PPC_STUB(DmGetThreadContext);
REX_PPC_STUB(DmGetThreadList);
REX_PPC_STUB(DmGetXbeInfo);
REX_PPC_STUB(DmGetXboxName);
REX_PPC_STUB(DmIsDebuggerPresent);
REX_PPC_STUB(DmNotify);
REX_PPC_STUB(DmOpenNotificationSession);
REX_PPC_STUB(DmRegisterCommandProcessor);
REX_PPC_STUB(DmResumeThread);
REX_PPC_STUB(DmSendNotificationString);
REX_PPC_STUB(DmSuspendThread);
REX_PPC_STUB_RETURN(DmWalkLoadedModules, 0x82DA0104); // XBDM_ENDOFLIST
REX_PPC_STUB(DmWalkModuleSections);
//REX_PPC_STUB(__CAP_Enter_Function);
//REX_PPC_STUB(__CAP_Exit_Function);
REX_PPC_STUB_RETURN(DmRegisterCommandProcessorEx, 0);
REX_PPC_STUB(DmQueryMemoryStatistics);
REX_PPC_STUB(DmCaptureStackBackTrace);
REX_PPC_STUB(DmGetThreadInfoEx);
REX_PPC_STUB(DmWalkCommittedMemory);
REX_PPC_STUB(DmCloseCounters_2);
REX_PPC_STUB(DmGetConsoleType);
REX_PPC_STUB(DmMapDevkitDrive);
REX_PPC_STUB(DmWalkLoadedModulesEx);
REX_PPC_STUB(DmGetSystemInfo);
REX_PPC_STUB(DmPMCInstallSetup);
REX_PPC_STUB(DmPMCResetCounters);
REX_PPC_STUB(DmPMCSetTriggerProcessor);
REX_PPC_STUB(DmPMCStart);
REX_PPC_STUB(DmPMCStop);
REX_PPC_STUB(DmPMCGetCounter);
REX_PPC_STUB(DmGetDebugMemorySize);
REX_PPC_STUB(DmGetConsoleFeatures);
REX_PPC_STUB(DmCloseCommittedMemory);

//REX_PPC_EXTERN_IMPORT(game_state_shell_gobble_first_physical_allocation);
//void game_state_shell_gobble_first_physical_allocation(void)
//{
//	REX_PPC_INVOKE(game_state_shell_gobble_first_physical_allocation);
//}
//REX_PPC_HOOK(game_state_shell_gobble_first_physical_allocation);

// REX

#define PRINT_IMPORT_HELPER(name, print_name) \
	PPC_EXTERN_IMPORT(__imp__##print_name); \
	extern "C" PPC_FUNC(name) \
	{ \
		__imp__##print_name(ctx, base); \
	}

PRINT_IMPORT_HELPER(sub_82FBCCC0, _vsnprintf);
PRINT_IMPORT_HELPER(sub_82FB82B0, rex_sprintf);

#undef PRINT_HOOK_HELPER

// XAM

#include <rex/system/xsocket.h>

namespace rex
{
	using namespace rex::system;

	REX_PPC_EXTERN_IMPORT(connect);
	REX_PPC_EXTERN_IMPORT(recv);

	int connect(X_HANDLE s, const XSOCKADDR *name, int namelen);
	int recv(X_HANDLE s, uint8_t *buf, int len, int flags);

	REX_PPC_HOOK(connect);
	REX_PPC_HOOK(recv);

	int connect(X_HANDLE s, const XSOCKADDR *name, int namelen)
	{
		auto socket = kernel_state()->object_table()->LookupObject<XSocket>(s);
		if (!socket)
		{
			// WSAENOTSOCK
			XThread::SetLastError(0x2736);
			return -1;
		}

		N_XSOCKADDR address = name;

		X_STATUS status = socket->Connect(&address, namelen);
		if (XFAILED(status)) // skip over for testing
		{
			//XThread::SetLastError(socket->GetLastWSAError());
			//return -1;
		}

		return 0;
	}

	int recv(X_HANDLE s, uint8_t *buf, int len, int flags)
	{
		auto socket =
		kernel_state()->object_table()->LookupObject<XSocket>(s);
		if (!socket)
		{
			// WSAENOTSOCK
			XThread::SetLastError(0x2736);
			return -1;
		}

		int ret = socket->Recv(buf, len, flags);
		if (ret < 0)
		{
			//XThread::SetLastError(socket->GetLastWSAError());
		}
		return ret;
	}
}

// BLAM!

REX_PPC_EXTERN_IMPORT(XMemGetPageSize);

DWORD XMemGetPageSize(DWORD address)
{
	return REX_PPC_INVOKE(XMemGetPageSize, address);
}

REX_PPC_HOOK(memset);
REX_PPC_HOOK(memcpy);
REX_PPC_HOOK(memmove);

// k_tag_cache_minimum_address	0xA0000000
// k_tag_cache_maximum_address	0xBFC00000

long volatile g_allocation_reserve_adjustment = 0;

REX_PPC_EXTERN_IMPORT(physical_memory_compute_allocation_bounds);
REX_PPC_EXTERN_IMPORT(physical_memory_query_bounds);
REX_PPC_EXTERN_IMPORT(physical_memory_system_malloc);

void physical_memory_compute_allocation_bounds(void** out_base_address, unsigned long* out_allocation_size);
void physical_memory_query_bounds(unsigned long physical_memory_base_address, unsigned long physical_memory_query_address, unsigned long* out_physical_memory_start, unsigned long* out_physical_memory_end);
static void* physical_memory_system_malloc(unsigned long size, void* expected_address, unsigned long memory_protection);

REX_PPC_HOOK(physical_memory_compute_allocation_bounds);
REX_PPC_HOOK(physical_memory_query_bounds);

void physical_memory_compute_allocation_bounds(void** out_base_address, unsigned long* out_allocation_size)
{
	//REX_PPC_INVOKE(physical_memory_compute_allocation_bounds, out_base_address, out_allocation_size);

	REX_PPC_MEMBASE_PTR(base);

	assert(out_base_address);
	assert(out_allocation_size);

	unsigned long physical_memory_start = 0;
	unsigned long physical_memory_end = 0;
	physical_memory_query_bounds(0xA0000000, 0xBFC00000, &physical_memory_start, &physical_memory_end);

	if (physical_memory_end < 0xBFC00000)
	{
		*out_allocation_size = 0;
		*out_base_address = nullptr;
	}
	else
	{
		*out_allocation_size = 0xBFC00000 - physical_memory_start - (g_allocation_reserve_adjustment + 0x4D00000);
		void* base_address = memory->TranslateVirtual<void*>(0xBFC00000 - *out_allocation_size);
		*out_base_address = base_address;
	}
}

void physical_memory_query_bounds(unsigned long physical_memory_base_address, unsigned long physical_memory_query_address, unsigned long* out_physical_memory_start, unsigned long* out_physical_memory_end)
{
	REX_PPC_INVOKE(physical_memory_query_bounds, physical_memory_base_address, out_physical_memory_start, out_physical_memory_end);
}

static void* physical_memory_system_malloc(unsigned long size, void* expected_address, unsigned long memory_protection)
{
	return REX_PPC_INVOKE(physical_memory_system_malloc, size, expected_address, memory_protection);
}
