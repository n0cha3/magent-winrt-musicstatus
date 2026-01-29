#pragma once

#include "main.h"

typedef struct _SmtcTrackMeta {
    PWSTR TrackName;
    PWSTR ArtistName;
    BOOL IsPaused;
} SmtcTrackMeta;

BOOL WINAPI SmtcGetCurrTrackData(SmtcTrackMeta *TrackMetadata);