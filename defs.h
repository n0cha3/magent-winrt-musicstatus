#pragma once 
#include "main.h"

enum WMPFlags {
    FLAG_PLAYING = 0x0002,
    FLAG_STOPPED = 0x0003,
    FLAG_PAUSED  = 0x0004
};

#define PLUGIN_GUID (PWSTR)L"Global\\{1DE128B2-A096-4e98-9F70-329EF9B20DC7}"