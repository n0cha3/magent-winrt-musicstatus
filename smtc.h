#pragma once

#include "main.h"

typedef struct _SmtcTrackMeta {
    WCHAR TrackName[256];
    WCHAR ArtistName[256];
    BOOL IsPaused;
    DWORD TrackNameLen;
    DWORD ArtistNameLen;
} SmtcTrackMeta;

extern SmtcTrackMeta CurrentTrackMetadata;

BOOL WINAPI SmtcGetCurrTrackData();