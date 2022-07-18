#include <Windows.h>
#include <cstdio>
#include <cstdint>

namespace Steam
{
	namespace Offsets
	{
		uint64_t SteamPresentHook = 0x88D80;
		uint64_t SteamOrigPresent = 0x1935C8;
		uint64_t HookFunc = 0x82B00;
	}

	__int64(__fastcall* OrigPresent)(void*, __int64, __int64);
	__int64 __fastcall PresentHook(void* swap_chain, __int64 sync_interval, __int64 flags)
	{
		printf("Present hooked! %p, %llx, %llx\n", swap_chain, sync_interval, flags);
		return OrigPresent(swap_chain, sync_interval, flags);
	}

	bool HookSteamOverlay()
	{
		/*
		Steam::HookFunction(
        *(_QWORD *)(DXGIVTable + 64),
        Steam::Hooks::DXGIPresent,
        &Steam::Globals::OriginalPresent,
        1i64);
		*/
		auto base = (uint64_t)GetModuleHandleA("GameOverlayRenderer64.dll");
		if (!base) return false;

		auto hook_function = reinterpret_cast<__int64(__fastcall*)(uint64_t, uint64_t, uint64_t*, uint64_t)>(0);
		*(uint64_t*)&hook_function = base + Offsets::HookFunc;

		hook_function(base + Offsets::SteamPresentHook, (uint64_t)PresentHook, (uint64_t*)&OrigPresent, 1);
		return true;
	}
}

void __stdcall OnAttach(HMODULE hmodule)
{
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	printf("injected into %d\n", GetCurrentProcessId());

	if (!Steam::HookSteamOverlay())
	{
		printf("failed to hook steam\n");
		return;
	}

	while (true)
	{
		if (GetAsyncKeyState(VK_END) & 1)
			break;
	}

	printf("goodbye\n");
	FreeLibraryAndExitThread(hmodule, 0);
}

bool __stdcall DllMain(HMODULE hmodule, DWORD reason, void*)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(0, 0, (PTHREAD_START_ROUTINE)OnAttach, hmodule, 0, 0));
		break;
	}

	return TRUE;
}