#include "smtc.h"

#include <initguid.h>
#include <roapi.h>
#include <activation.h>
#include "loc_windows.media.control.h"
#include <winstring.h>
#include <stdio.h>

HANDLE Event;
UINT32 RefCountSM = 0;


HRESULT STDMETHODCALLTYPE HandlerInvoke(
    __FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This,
    __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *asyncInfo,
    AsyncStatus asyncStatus) {
    
    UNREFERENCED_PARAMETER(This);

    if (asyncStatus == Completed) {
        //printf("\n%s", "invoked");
        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionManager *GSmtcSm;
        asyncInfo->lpVtbl->GetResults(asyncInfo, &GSmtcSm);
        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSession *GSmtcS;
        GSmtcSm->lpVtbl->GetCurrentSession(GSmtcSm, &GSmtcS);
        __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionMediaProperties *IGsmtcMProp;
        HRESULT FactoryStatus = GSmtcS->lpVtbl->TryGetMediaPropertiesAsync(GSmtcS, &IGsmtcMProp);
        if (SUCCEEDED(FactoryStatus)) printf("\n%s\n", "success");

        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionMediaProperties *IGSmtcSMTP;
        IGsmtcMProp->lpVtbl->GetResults(IGsmtcMProp, &IGSmtcSMTP);
        HSTRING SongTitle;
        IGSmtcSMTP->lpVtbl->get_Title(IGSmtcSMTP, &SongTitle);
        UINT32 StringSize;

        printf("\n%ls\n", WindowsGetStringRawBuffer(SongTitle, &StringSize));
        SetEvent(Event);
    }
    return S_OK;
}

ULONG STDMETHODCALLTYPE HandlerAddRef (__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This) {
    //printf("\n%s", "added reference");
    UNREFERENCED_PARAMETER(This);
    RefCountSM++;
    return 0;
}

HRESULT STDMETHODCALLTYPE HandleQueryInterface (
        __FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This,
        REFIID riid,
        void** ppvObject) {
        //printf("\n%s\n", "query");

        IID AsyncHandlerIID;

        IIDFromString(L"{10F0074E-923D-5510-8F4A-DDE37754CA0E}", &AsyncHandlerIID);
        
        if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &AsyncHandlerIID)) {
            *ppvObject = This;
            RefCountSM++;
            return S_OK;
        }
        return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE HandlerRelease(__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This) {
    (void)This;

    //printf("\n%s\n", "release");

   if (!RefCountSM) return 0;


    RefCountSM--;
    return RefCountSM;
}

__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManagerVtbl AsyncHndlVtable = {
    .Invoke = HandlerInvoke,
    .AddRef = HandlerAddRef,
    .QueryInterface = HandleQueryInterface,
    .Release = HandlerRelease
};

__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager AsyncHndl = {
    .lpVtbl = &AsyncHndlVtable
};

BOOL WINAPI SmtcGetCurrTrackData(SmtcTrackMeta *TrackMetadata) {
    UNREFERENCED_PARAMETER(TrackMetadata);

    IID GuidGsmtc;

    IIDFromString(L"{2050c4ee-11a0-57de-aed7-c97c70338245}", &GuidGsmtc);

    //HRESULT WinRtStatus = RoInitialize(RO_INIT_MULTITHREADED);

    //if (SUCCEEDED(WinRtStatus)) printf("\n%s \n", "Loaded winrt");
    //i'll add proper winrt init later
  
    __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionManagerStatics *IGlobSmtcS = NULL;
    __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *IAsyncGlobSmtc = NULL;
    HSTRING ClassString;

    WindowsCreateString(L"Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager", (UINT32)wcslen(L"Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager"), &ClassString);

    RoGetActivationFactory(ClassString, &GuidGsmtc, (PVOID*)&IGlobSmtcS);

    IGlobSmtcS->lpVtbl->RequestAsync(IGlobSmtcS, &IAsyncGlobSmtc);

    RefCountSM = 1;
    Event = CreateEventW(NULL, FALSE, FALSE, L"SMTCWaitTimer");

    HRESULT FactoryStatus = IAsyncGlobSmtc->lpVtbl->put_Completed(IAsyncGlobSmtc, &AsyncHndl);
    
    WaitForSingleObject(Event, INFINITE);

    AsyncHndl.lpVtbl->Release(&AsyncHndl);

    if (FAILED(FactoryStatus)) {
        printf("%lx", FactoryStatus);
    }

    return 0;
}
