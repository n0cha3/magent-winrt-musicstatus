#include "main.h"
#include "hooks.h"

__declspec(dllexport) VOID __cdecl exportme(VOID) {
	return;
}

BOOL WINAPI DllMain(HINSTANCE Instance, DWORD Reason, LPVOID Reserved) {
	UNREFERENCED_PARAMETER(Instance);

	switch (Reason) {
		case DLL_PROCESS_ATTACH: {
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			PrepareHooks();
			break;
		}

        case DLL_THREAD_ATTACH: {

		}

        case DLL_THREAD_DETACH: {
			break;
		}

        case DLL_PROCESS_DETACH: {
			FreeConsole();
			if (Reserved != NULL) {
                break;
            }
		}
            
        break;
	}
	
	return TRUE;
}