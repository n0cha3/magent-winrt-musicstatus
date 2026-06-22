#include "hooks.h"
#include "iathook.h"
#include "defs.h"
#include "smtc.h"

typedef HANDLE (WINAPI *_OpenEventW) (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName);
typedef BOOL (WINAPI *_CloseHandle) (HANDLE hObject);
typedef UINT (WINAPI *_GlobalGetAtomNameW) (ATOM nAtom, LPWSTR lpBuffer, int nSize);
typedef ATOM (WINAPI *_GlobalDeleteAtom) (ATOM nAtom);
typedef LONG (WINAPI *_RegQueryValueExW) (HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

_OpenEventW OrigOpenEventW = NULL;
_CloseHandle OrigCloseHandle = NULL;
_GlobalGetAtomNameW OrigGlobalGetAtomNameW = NULL;
_GlobalDeleteAtom OrigGlobalDeleteAtom = NULL;
_RegQueryValueExW OrigRegQueryValueExW = NULL;

BOOL WINAPI DetourCloseHandle(HANDLE hObject) {
    if ((DWORD)hObject == 1337) return TRUE;
    return OrigCloseHandle(hObject);
}

HANDLE WINAPI DetourOpenEventW(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName) {
    if (lpName == NULL) return NULL;
    
    if (_wcsicmp(lpName, PLUGIN_GUID) == 0) {
        if (WaitForSingleObject(SmtcEvent, 4) == WAIT_OBJECT_0) return (PDWORD)1337;

        else return NULL;
    }
    
    return OrigOpenEventW(dwDesiredAccess, bInheritHandle, lpName);
}

LONG WINAPI DetourRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
    if (lpValueName == NULL) return OrigRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (_wcsicmp(lpValueName, L"WMPLen") == 0) {
        *(PDWORD)lpData = 1337;
        *lpType = REG_DWORD;
        *lpcbData = sizeof(DWORD);

        return ERROR_SUCCESS;
    }

    if (_wcsicmp(lpValueName, L"WMPState") == 0) {
        *(PDWORD)lpData = (CurrentTrackMetadata.Status << 16) | (DWORD)256;
        *lpType = REG_DWORD;
        *lpcbData = sizeof(DWORD);
        return ERROR_SUCCESS;
    }

    return OrigRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

UINT WINAPI DetourGlobalGetAtomNameW(ATOM nAtom, LPWSTR lpBuffer, int nSize) {
    if (nAtom == 1337) {
        WCHAR Buffer[256];
        
        if (CurrentTrackMetadata.ArtistNameLen != 0) wsprintf(Buffer, L"%ls\"%ls - %ls\"", L"player=\"WMP\" track=", CurrentTrackMetadata.ArtistName, CurrentTrackMetadata.TrackName);
        else wsprintf(Buffer, L"%ls\"%ls\"", L"player=\"WMP\" track=", CurrentTrackMetadata.TrackName);
  
        wcscpy(lpBuffer, Buffer);

        return nSize;
    }
    return OrigGlobalGetAtomNameW(nAtom, lpBuffer, nSize);
}

ATOM WINAPI DetourGlobalDeleteAtom(ATOM nAtom) {
    if (nAtom == 1337) return 0;

    return OrigGlobalDeleteAtom(nAtom);
}

VOID WINAPI PrepareHooks(VOID) {
    OrigOpenEventW = (_OpenEventW)HookIatFunc(L"Kernel32.dll", "OpenEventW", (PVOID)DetourOpenEventW);
    OrigCloseHandle = (_CloseHandle)HookIatFunc(L"Kernel32.dll", "CloseHandle", DetourCloseHandle);
    OrigRegQueryValueExW = (_RegQueryValueExW)HookIatFunc(L"Advapi32.dll", "RegQueryValueExW", DetourRegQueryValueExW);
    OrigGlobalGetAtomNameW = (_GlobalGetAtomNameW)HookIatFunc(L"Kernel32.dll", "GlobalGetAtomNameW", DetourGlobalGetAtomNameW);
    OrigGlobalDeleteAtom = (_GlobalDeleteAtom)HookIatFunc(L"Kernel32.dll", "GlobalDeleteAtom", DetourGlobalDeleteAtom);
}

