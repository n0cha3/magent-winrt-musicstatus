#include "hooks.h"
#include "trampoline.h"
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
        if (WaitForSingleObject(SmtcEvent, 500) == WAIT_OBJECT_0) return (PDWORD)1337;

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
        wsprintf(Buffer, L"%ls\"%ls - %ls\"", L"player=\"WMP\" track=", CurrentTrackMetadata.ArtistName, CurrentTrackMetadata.TrackName);
        wcscpy(lpBuffer, Buffer);

        return nSize;
    }
    return OrigGlobalGetAtomNameW(nAtom, lpBuffer, nSize);
}

VOID WINAPI PrepareHooks(VOID) {
    HMODULE Kernel32 = GetModuleHandleW(L"Kernel32.dll"),
        Advapi32 = GetModuleHandleW(L"Advapi32.dll");

    FARPROC AddrOpenEventW = GetProcAddress(Kernel32, "OpenEventW"),
        AddrRegQueryValueExW = GetProcAddress(Advapi32, "RegQueryValueExW"),
        AddrGlobalGetAtomNameW = GetProcAddress(Kernel32, "GlobalGetAtomNameW"),
        AddrCloseHandle = GetProcAddress(Kernel32, "CloseHandle");

    OrigOpenEventW = (_OpenEventW)EnableTrampoline((PVOID)AddrOpenEventW, (PVOID)DetourOpenEventW, 8);
    OrigCloseHandle = (_CloseHandle)EnableTrampoline((PVOID)AddrCloseHandle, (PVOID)DetourCloseHandle, 8);
    OrigRegQueryValueExW = (_RegQueryValueExW)EnableTrampoline((PVOID)AddrRegQueryValueExW, (PVOID)DetourRegQueryValueExW, 5);
    OrigGlobalGetAtomNameW = (_GlobalGetAtomNameW)EnableTrampoline((PVOID)AddrGlobalGetAtomNameW, (PVOID)DetourGlobalGetAtomNameW, 5);
}

