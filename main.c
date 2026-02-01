#include "main.h"
#include "hooks.h"
#include "smtc.h"

__declspec(dllexport) VOID __cdecl exportme(VOID) {
	return;
}

BOOL WINAPI DllMain(HINSTANCE Instance, DWORD Reason, LPVOID Reserved) {
	UNREFERENCED_PARAMETER(Instance);

	switch (Reason) {
		case DLL_PROCESS_ATTACH: {
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&SmtcGetCurrTrackData, NULL, 0, NULL);
			PrepareHooks();
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