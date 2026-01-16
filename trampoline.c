#include "trampoline.h"

PVOID WINAPI EnableTrampoline(PVOID Original, PVOID Detour, CONST SIZE_T Length) {
    PVOID OldFuncPointer = VirtualAlloc(NULL, Length + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    CopyMemory(OldFuncPointer, Original, Length);
    *(PBYTE)((PBYTE)OldFuncPointer + Length) = 0xE9;

    DWORD OrigAddr = (DWORD)Original + 5,
        JmpTrampoline = (DWORD)OldFuncPointer + Length + 5,
        Offset = OrigAddr - JmpTrampoline;

    *(PDWORD)((PBYTE)OldFuncPointer + Length + 1) = Offset;
    
    DWORD OldProt;
    VirtualProtect(Original, Length, PAGE_EXECUTE_READWRITE, &OldProt);
    
    DWORD Addr = ((DWORD)Detour - (DWORD)Original) - 5;
    *(PBYTE)Original = 0xE9;
    *(PDWORD)((PBYTE)Original + 1) = Addr;

    VirtualProtect(Original, Length, OldProt, &OldProt);

    return OldFuncPointer;
}