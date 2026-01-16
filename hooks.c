#include "hooks.h"
#include "trampoline.h"

typedef HANDLE (WINAPI *_OpenEventW) (DWORD dwDesiredAccess, WINBOOL bInheritHandle, LPCWSTR lpName);
typedef WINBOOL (WINAPI *_CloseHandle) (HANDLE hObject);
typedef UINT (WINAPI *_GlobalGetAtomNameW) (ATOM nAtom, LPWSTR lpBuffer, int nSize);
typedef ATOM (WINAPI *_GlobalDeleteAtom) (ATOM nAtom);
typedef LONG (WINAPI *_RegQueryValueExW) (HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

_OpenEventW OrigOpenEventW = NULL;
_CloseHandle OrigCloseHandle = NULL;
_GlobalGetAtomNameW OrigGlobalGetAtomNameW = NULL;
_GlobalDeleteAtom OrigGlobalDeleteAtom = NULL;
_RegQueryValueExW OrigRegQueryValueExW = NULL;

HANDLE WINAPI DetourOpenEventW(DWORD dwDesiredAccess, WINBOOL bInheritHandle, LPCWSTR lpName) {
    wprintf(L"\n%S", lpName);

    if (lpName == NULL) return NULL;

    PWSTR PluginGuid = L"Global\\{1DE128B2-A096-4e98-9F70-329EF9B20DC7}\0";

    if (_wcsicmp(lpName, PluginGuid) == 0) {
        return (PDWORD)123123;
    }
    return OrigOpenEventW(dwDesiredAccess, bInheritHandle, lpName);
}

LONG WINAPI DetourRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
    wprintf(L"\n%S", lpValueName);

    if (_wcsicmp(lpValueName, L"WMPLen") == 0) {
        *(PDWORD)lpData = 1337;
        *lpType = REG_DWORD;
        return ERROR_SUCCESS;
    }

    if (_wcsicmp(lpValueName, L"WMPState") == 0) {
        *(PDWORD)lpData = (0x0002 << 16) | (DWORD)26;
        *lpType = REG_DWORD;
        return ERROR_SUCCESS;
    }

    return OrigRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

UINT WINAPI DetourGlobalGetAtomNameW(ATOM nAtom, LPWSTR lpBuffer, int nSize) {
    wprintf(L"\nAtom - %u size - %u", nAtom, nSize);

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

    OrigOpenEventW = EnableTrampoline((PVOID)AddrOpenEventW, (PVOID)DetourOpenEventW, 8);
    OrigRegQueryValueExW = EnableTrampoline((PVOID)AddrRegQueryValueExW, (PVOID)DetourRegQueryValueExW, 5);
    OrigGlobalGetAtomNameW = EnableTrampoline((PVOID)AddrGlobalGetAtomNameW, (PVOID)DetourGlobalGetAtomNameW, 5);
}

