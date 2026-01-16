#pragma once

#include "main.h"

PVOID WINAPI EnableTrampoline(PVOID Original, PVOID Detour, CONST SIZE_T Length);
