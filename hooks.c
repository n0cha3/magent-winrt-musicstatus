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

HANDLE WINAPI DetourOpenEventW(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName) {
    wprintf(L"\n%ls", lpName);
    if (lpName == NULL) return NULL;

    if (_wcsicmp(lpName, PLUGIN_GUID) == 0) {
        return (PDWORD)1337;
    }
    
    return OrigOpenEventW(dwDesiredAccess, bInheritHandle, lpName);
}

LONG WINAPI DetourRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
    //wprintf(L"\n%ls", lpValueName);

    if (lpValueName == NULL) return OrigRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (_wcsicmp(lpValueName, L"WMPLen") == 0) {
        *(PDWORD)lpData = 1337;
        *lpType = REG_DWORD;
        *lpcbData = sizeof(DWORD);

        return ERROR_SUCCESS;
    }

    if (_wcsicmp(lpValueName, L"WMPState") == 0) {
        *(PDWORD)lpData = (FLAG_PLAYING << 16) | (DWORD)26;
        *lpType = REG_DWORD;
        *lpcbData = sizeof(DWORD);

        return ERROR_SUCCESS;
    }

    return OrigRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

UINT WINAPI DetourGlobalGetAtomNameW(ATOM nAtom, LPWSTR lpBuffer, int nSize) {
    //wprintf(L"\nAtom - %u size - %u", nAtom, nSize);

    if (nAtom == 1337) {
        wcscpy(lpBuffer, L"player=\"WMP\" track=\"test\"");
        return nSize;
    }

    return OrigGlobalGetAtomNameW(nAtom, lpBuffer, nSize);
}

VOID WINAPI PrepareHooks(VOID) {
    HMODULE Kernel32 = GetModuleHandleW(L"Kernel32.dll"),
        Advapi32 = GetModuleHandleW(L"Advapi32.dll");

    FARPROC AddrOpenEventW = GetProcAddress(Kernel32, "OpenEventW"),
        AddrRegQueryValueExW = GetProcAddress(Advapi32, "RegQueryValueExW"),
        AddrGlobalGetAtomNameW = GetProcAddress(Kernel32, "GlobalGetAtomNameW");

    OrigOpenEventW = (_OpenEventW)EnableTrampoline((PVOID)AddrOpenEventW, (PVOID)DetourOpenEventW, 8);
    OrigRegQueryValueExW = (_RegQueryValueExW)EnableTrampoline((PVOID)AddrRegQueryValueExW, (PVOID)DetourRegQueryValueExW, 5);
    OrigGlobalGetAtomNameW = (_GlobalGetAtomNameW)EnableTrampoline((PVOID)AddrGlobalGetAtomNameW, (PVOID)DetourGlobalGetAtomNameW, 5);
}

