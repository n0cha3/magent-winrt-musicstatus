#include "smtc.h"
#include "defs.h"

#include <initguid.h>
#include <roapi.h>
#include <activation.h>
#include "loc_windows.media.control.h"
#include <winstring.h>
#include <stdio.h>

static HANDLE Event;

SmtcTrackMeta CurrentTrackMetadata;

HRESULT STDMETHODCALLTYPE HandlerInvoke(
    __FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This,
    __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *asyncInfo,
    AsyncStatus asyncStatus);

ULONG STDMETHODCALLTYPE HandlerAddRef (__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This);

HRESULT STDMETHODCALLTYPE HandleQueryInterface(
        __FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This,
        REFIID riid,
        void** ppvObject);

ULONG STDMETHODCALLTYPE HandlerRelease(__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This);

__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManagerVtbl AsyncHandlerVtable = {
    .Invoke = HandlerInvoke,
    .AddRef = HandlerAddRef,
    .QueryInterface = HandleQueryInterface,
    .Release = HandlerRelease
};

typedef struct _AsyncHandlerObject {
    __FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager AsyncHandler;
    DWORD RefCountSM;
} AsyncHandlerObject;

AsyncHandlerObject SmtcAsyncOperation;

HRESULT STDMETHODCALLTYPE HandlerInvoke(
    __FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This,
    __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *asyncInfo,
    AsyncStatus asyncStatus) {
    
    UNREFERENCED_PARAMETER(This);

    if (asyncStatus == Completed) {
        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionManager *GSmtcSm;
        asyncInfo->lpVtbl->GetResults(asyncInfo, &GSmtcSm);
        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSession *GSmtcS;
        GSmtcSm->lpVtbl->GetCurrentSession(GSmtcSm, &GSmtcS);

        if (GSmtcS == NULL) {
            GSmtcSm->lpVtbl->Release(GSmtcSm);
            asyncInfo->lpVtbl->Release(asyncInfo);
            SetEvent(Event);
            return S_FALSE;
        }

        __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionMediaProperties *IGsmtcMProp;
        GSmtcS->lpVtbl->TryGetMediaPropertiesAsync(GSmtcS, &IGsmtcMProp);
        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionMediaProperties *IGSmtcSMTP;
        IGsmtcMProp->lpVtbl->GetResults(IGsmtcMProp, &IGSmtcSMTP);

        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionPlaybackInfo *IGSmtcPbInf;

        GSmtcS->lpVtbl->GetPlaybackInfo(GSmtcS, &IGSmtcPbInf);

        __x_ABI_CWindows_CMedia_CControl_CGlobalSystemMediaTransportControlsSessionPlaybackStatus SmtcPlaybackStatus;

        IGSmtcPbInf->lpVtbl->get_PlaybackStatus(IGSmtcPbInf, &SmtcPlaybackStatus);

        switch (SmtcPlaybackStatus) {

            case GlobalSystemMediaTransportControlsSessionPlaybackStatus_Stopped: {
                CurrentTrackMetadata.Status = FLAG_STOPPED;
                break;
            }

            case GlobalSystemMediaTransportControlsSessionPlaybackStatus_Paused: {
                CurrentTrackMetadata.Status = FLAG_PAUSED;
                break;
            }

            default: {
                CurrentTrackMetadata.Status = FLAG_PLAYING;
                break;
            }
        }

        HSTRING SongTitle,
            ArtistName;

        IGSmtcSMTP->lpVtbl->get_Title(IGSmtcSMTP, &SongTitle);
        IGSmtcSMTP->lpVtbl->get_Artist(IGSmtcSMTP, &ArtistName);

        UINT32 ArtistStringSize,
            TitleStringSize;

        IGSmtcSMTP->lpVtbl->Release(IGSmtcSMTP);
        IGsmtcMProp->lpVtbl->Release(IGsmtcMProp);
        IGSmtcPbInf->lpVtbl->Release(IGSmtcPbInf);
        GSmtcS->lpVtbl->Release(GSmtcS);
        GSmtcSm->lpVtbl->Release(GSmtcSm);
        asyncInfo->lpVtbl->Release(asyncInfo);
        
        PCWSTR TrackNameRawBuffer = WindowsGetStringRawBuffer(SongTitle, &TitleStringSize);

        if (TitleStringSize > 50) {
            WindowsDeleteString(SongTitle);
            return S_FALSE;
        }

        printf("\n%ls\n", TrackNameRawBuffer);
        wcscpy(CurrentTrackMetadata.TrackName, TrackNameRawBuffer);
        WindowsDeleteString(SongTitle);


        PCWSTR ArtistNameRawBuffer = WindowsGetStringRawBuffer(ArtistName, &ArtistStringSize);

        if (ArtistStringSize > 50) {
            WindowsDeleteString(ArtistName);
            return S_FALSE;
        }

        printf("\n%ls\n", ArtistNameRawBuffer);
        wcscpy(CurrentTrackMetadata.ArtistName, ArtistNameRawBuffer);
        WindowsDeleteString(ArtistName);

        CurrentTrackMetadata.TrackNameLen = TitleStringSize;
        CurrentTrackMetadata.ArtistNameLen = ArtistStringSize;


        SetEvent(Event);
    }
    return S_OK;
}

ULONG STDMETHODCALLTYPE HandlerAddRef(__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This) {
    UNREFERENCED_PARAMETER(This);
    return SmtcAsyncOperation.RefCountSM++;
}

HRESULT STDMETHODCALLTYPE HandleQueryInterface(
        __FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This,
        REFIID riid,
        void** ppvObject) {

        IID AsyncHandlerIID;

        IIDFromString(L"{10F0074E-923D-5510-8F4A-DDE37754CA0E}", &AsyncHandlerIID);
        
        if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &AsyncHandlerIID)) {
            *ppvObject = This;
            SmtcAsyncOperation.AsyncHandler.lpVtbl->AddRef(&SmtcAsyncOperation.AsyncHandler);
            return S_OK;
        }
        return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE HandlerRelease(__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This) {
    (void)This;

    if (!SmtcAsyncOperation.RefCountSM) return 0;


    return SmtcAsyncOperation.RefCountSM--;
}

VOID WINAPI SmtcGetCurrTrackData(VOID) {
    IID GuidGsmtc;

    IIDFromString(L"{2050c4ee-11a0-57de-aed7-c97c70338245}", &GuidGsmtc);

    RoInitialize(RO_INIT_MULTITHREADED);
    __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionManagerStatics *IGlobSmtcS = NULL;
    __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *IAsyncGlobSmtc = NULL;
    HSTRING ClassString;

    SmtcAsyncOperation.RefCountSM = 1;
    SmtcAsyncOperation.AsyncHandler.lpVtbl = &AsyncHandlerVtable;

    WindowsCreateString(L"Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager", (UINT32)wcslen(L"Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager"), &ClassString);
    RoGetActivationFactory(ClassString, &GuidGsmtc, (PVOID*)&IGlobSmtcS);
    Event = CreateEventW(NULL, FALSE, FALSE, L"SMTCWaitTimer");
    while (TRUE) {
        IGlobSmtcS->lpVtbl->RequestAsync(IGlobSmtcS, &IAsyncGlobSmtc);
        IAsyncGlobSmtc->lpVtbl->put_Completed(IAsyncGlobSmtc, &SmtcAsyncOperation.AsyncHandler);
        WaitForSingleObject(Event, INFINITE);
        Sleep(900);
    }
    
    /*SmtcAsyncOperation.AsyncHandler.lpVtbl->Release(&SmtcAsyncOperation.AsyncHandler);
    IAsyncGlobSmtc->lpVtbl->Release(IAsyncGlobSmtc);
    IGlobSmtcS->lpVtbl->Release(IGlobSmtcS);*/
}
