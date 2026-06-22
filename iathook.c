#include "iathook.h"

PVOID HookIatFunc(PWSTR LibraryName, PSTR HookFunctionName, PVOID DetourFunction) {
    PVOID ImageBase = GetModuleHandleW(NULL),
        TargetFunctionAddress = (PVOID)GetProcAddress(GetModuleHandleW(LibraryName), HookFunctionName);

    if (!ImageBase || !TargetFunctionAddress) return NULL;

	PIMAGE_DOS_HEADER DosHeaders = (PIMAGE_DOS_HEADER)ImageBase;
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((PBYTE)ImageBase + DosHeaders->e_lfanew);
    IMAGE_DATA_DIRECTORY ImportsDirectory = NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)ImageBase + ImportsDirectory.VirtualAddress);
    
    for(; ImportDescriptor->Name; ImportDescriptor++) {
        DWORD ThunkData = ImportDescriptor->OriginalFirstThunk ? ImportDescriptor->OriginalFirstThunk : ImportDescriptor->FirstThunk;
        PIMAGE_THUNK_DATA OriginalThunk = (PIMAGE_THUNK_DATA)((PBYTE)ImageBase + ThunkData);
        PIMAGE_THUNK_DATA FirstThunk = (PIMAGE_THUNK_DATA)((PBYTE)ImageBase + ImportDescriptor->FirstThunk);
        
        for(; OriginalThunk->u1.AddressOfData; OriginalThunk++, FirstThunk++) {
            if (FirstThunk->u1.Function == (DWORD)TargetFunctionAddress) {
                DWORD OldProtect = 0;

				VirtualProtect((PVOID)(&FirstThunk->u1.Function), sizeof(PVOID), PAGE_READWRITE, &OldProtect);

                PVOID OriginalFunction = (PVOID)FirstThunk->u1.Function;
				FirstThunk->u1.Function = (DWORD)DetourFunction;

                VirtualProtect((PVOID)(&FirstThunk->u1.Function), sizeof(PVOID), OldProtect, &OldProtect);

                FlushInstructionCache(GetCurrentProcess(), NULL, 0);
                return OriginalFunction;
            }
        }
    }
    return NULL;
}