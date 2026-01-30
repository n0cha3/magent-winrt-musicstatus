#include "smtc.h"

#include <initguid.h>
#include <roapi.h>
#include <activation.h>
#include "loc_windows.media.control.h"
#include <winstring.h>
#include <stdio.h>

static HANDLE Event;

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
        printf("\n%s", "invoked");
        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionManager *GSmtcSm;
        asyncInfo->lpVtbl->GetResults(asyncInfo, &GSmtcSm);
        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSession *GSmtcS;
        GSmtcSm->lpVtbl->GetCurrentSession(GSmtcSm, &GSmtcS);
        __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionMediaProperties *IGsmtcMProp;
        GSmtcS->lpVtbl->TryGetMediaPropertiesAsync(GSmtcS, &IGsmtcMProp);

        __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionMediaProperties *IGSmtcSMTP;
        IGsmtcMProp->lpVtbl->GetResults(IGsmtcMProp, &IGSmtcSMTP);
        HSTRING SongTitle;
        IGSmtcSMTP->lpVtbl->get_Title(IGSmtcSMTP, &SongTitle);
        UINT32 StringSize;

        IGSmtcSMTP->lpVtbl->Release(IGSmtcSMTP);
        IGsmtcMProp->lpVtbl->Release(IGsmtcMProp);
        GSmtcS->lpVtbl->Release(GSmtcS);
        GSmtcSm->lpVtbl->Release(GSmtcSm);
        asyncInfo->lpVtbl->Release(asyncInfo);

        printf("\n%ls\n", WindowsGetStringRawBuffer(SongTitle, &StringSize));
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
        printf("\n%s\n", "query");

        IID AsyncHandlerIID;

        IIDFromString(L"{10F0074E-923D-5510-8F4A-DDE37754CA0E}", &AsyncHandlerIID);
        
        if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &AsyncHandlerIID)) {
            *ppvObject = This;
            SmtcAsyncOperation.RefCountSM++;
            return S_OK;
        }
        return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE HandlerRelease(__FIAsyncOperationCompletedHandler_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *This) {
    (void)This;
    return SmtcAsyncOperation.RefCountSM--;
}

BOOL WINAPI SmtcGetCurrTrackData(SmtcTrackMeta *TrackMetadata) {
    UNREFERENCED_PARAMETER(TrackMetadata);

    IID GuidGsmtc;

    IIDFromString(L"{2050c4ee-11a0-57de-aed7-c97c70338245}", &GuidGsmtc);

    HRESULT WinRtStatus = RoInitialize(RO_INIT_MULTITHREADED);

    if (SUCCEEDED(WinRtStatus)) printf("\n%s \n", "Loaded winrt");
  
    __x_ABI_CWindows_CMedia_CControl_CIGlobalSystemMediaTransportControlsSessionManagerStatics *IGlobSmtcS = NULL;
    __FIAsyncOperation_1_Windows__CMedia__CControl__CGlobalSystemMediaTransportControlsSessionManager *IAsyncGlobSmtc = NULL;
    HSTRING ClassString;

    WindowsCreateString(L"Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager", (UINT32)wcslen(L"Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager"), &ClassString);

    RoGetActivationFactory(ClassString, &GuidGsmtc, (PVOID*)&IGlobSmtcS);

    IGlobSmtcS->lpVtbl->RequestAsync(IGlobSmtcS, &IAsyncGlobSmtc);

    SmtcAsyncOperation.RefCountSM = 1;
    SmtcAsyncOperation.AsyncHandler.lpVtbl = &AsyncHandlerVtable;
    Event = CreateEventW(NULL, FALSE, FALSE, L"SMTCWaitTimer");
    
    HRESULT FactoryStatus = IAsyncGlobSmtc->lpVtbl->put_Completed(IAsyncGlobSmtc, &SmtcAsyncOperation.AsyncHandler);
    
    WaitForSingleObject(Event, INFINITE);

    SmtcAsyncOperation.AsyncHandler.lpVtbl->Release(&SmtcAsyncOperation.AsyncHandler);

    if (FAILED(FactoryStatus)) {
        printf("%lx", FactoryStatus);
    }

    return 0;
}
