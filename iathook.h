#pragma once

#include "main.h"

PVOID HookIatFunc(PWSTR LibraryName, PSTR HookFunctionName, PVOID DetourFunction);