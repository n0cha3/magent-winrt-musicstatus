#include "main.h"
#include "hooks.h"
#include "smtc.h"

__declspec(dllexport) VOID __cdecl importme(VOID) {
	return;
}

VOID NTAPI InitHook(VOID) {
    ((PPEB)__readfsdword(0x30))->PostProcessInitRoutine = NULL;
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&SmtcGetCurrTrackData, NULL, 0, NULL);
    PrepareHooks();
}

BOOL WINAPI DllMain(HINSTANCE Instance, DWORD Reason, LPVOID Reserved) {
	switch (Reason) {
		case DLL_PROCESS_ATTACH: {
            ((PPEB)__readfsdword(0x30))->PostProcessInitRoutine = InitHook;
			DisableThreadLibraryCalls(Instance);
			break;
		}

        case DLL_THREAD_ATTACH: {

		}

        case DLL_THREAD_DETACH: {
			break;
		}

        case DLL_PROCESS_DETACH: {
			if (Reserved != NULL) {
                break;
            }
		}
            
        break;
	}
	
	return TRUE;
}