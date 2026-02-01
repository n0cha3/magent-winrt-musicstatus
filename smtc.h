#pragma once

#include "main.h"

typedef struct _SmtcTrackMeta {
    WCHAR TrackName[256];
    WCHAR ArtistName[256];
    WORD Status;
    DWORD TrackNameLen;
    DWORD ArtistNameLen;
} SmtcTrackMeta;

extern SmtcTrackMeta CurrentTrackMetadata;

VOID WINAPI SmtcGetCurrTrackData(VOID);