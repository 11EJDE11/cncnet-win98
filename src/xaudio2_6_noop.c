/* XAudio2_6.dll - a no-op XAudio 2.6 engine. Windows 98 has no XAudio2, and XNA will not
 * start its audio without one. Every call succeeds and reports benign state; sounds played
 * through XNA are silent. OLET3's CoCreateInstance loads it for CLSID_XAudio2 (2.6). */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef const GUID *REFGUID_X;

typedef struct XA_WAVEFORMATEX {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
} XA_WAVEFORMATEX;

static const GUID XA_CLSID_XAudio2_6 =
    {0x3EDA9B49,0x2085,0x498B,{0x9B,0xB2,0x39,0xA6,0x77,0x84,0x93,0xDE}};
static const GUID XA_IID_IUnknown =
    {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID XA_IID_IClassFactory =
    {0x00000001,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID XA_IID_IXAudio2 =
    {0x8BCF1F58,0x9FE7,0x4583,{0x8A,0xC6,0xE2,0xAD,0xC4,0x65,0xC8,0xBB}};

static int xa_guid_eq(REFGUID_X a, REFGUID_X b)
{
    const BYTE *pa = (const BYTE *)a;
    const BYTE *pb = (const BYTE *)b;
    int i;
    for (i = 0; i < (int)sizeof(GUID); i++) {
        if (pa[i] != pb[i]) return 0;
    }
    return 1;
}

/* OFF unless XASHIM_LOG=1 in the environment - see the same note in olet3.c.
 * These entry points are called per voice / per submitted buffer during
 * playback, so one file open+append+close per line is a real per-frame cost. */
static int xa_log_enabled(void)
{
    static int state = -1;   /* -1 = not yet checked */
    if (state < 0) {
        char v[8];
        state = (GetEnvironmentVariableA("XASHIM_LOG", v, sizeof(v)) > 0 && v[0] == '1') ? 1 : 0;
    }
    return state;
}

static void xa_log(const char *line)
{
    HANDLE h;
    DWORD written;

    if (!xa_log_enabled()) return;
    h = CreateFileA("C:\\DOTNET48\\XASHIM.LOG", GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    SetFilePointer(h, 0, NULL, FILE_END);
    WriteFile(h, line, lstrlenA(line), &written, NULL);
    WriteFile(h, "\r\n", 2, &written, NULL);
    CloseHandle(h);
}

/* ---- minimal XAudio2 struct definitions (2.6 layout) ---- */

typedef struct XA_VOICE_DETAILS {
    UINT32 CreationFlags;
    UINT32 InputChannels;
    UINT32 InputSampleRate;
} XA_VOICE_DETAILS;

typedef struct XA_VOICE_STATE {
    void   *pCurrentBufferContext;
    UINT32  BuffersQueued;
    UINT64  SamplesPlayed;
} XA_VOICE_STATE;

typedef struct XA_FILTER_PARAMETERS {
    UINT32 FilterType;
    float  Frequency;
    float  OneOverQ;
} XA_FILTER_PARAMETERS;

typedef struct XA_PERFORMANCE_DATA {
    BYTE data[128];
} XA_PERFORMANCE_DATA;

typedef struct XA_DEVICE_DETAILS {
    WCHAR DeviceID[256];
    WCHAR DisplayName[256];
    UINT32 Role;
    BYTE  OutputFormat[80]; /* WAVEFORMATEXTENSIBLE, zeroed is fine */
} XA_DEVICE_DETAILS;

/* ---- IClassFactory ---- */

typedef struct XAFactory XAFactory;
typedef struct XAFactoryVtbl {
    HRESULT (WINAPI *QueryInterface)(XAFactory *This, REFGUID_X riid, void **ppv);
    ULONG   (WINAPI *AddRef)(XAFactory *This);
    ULONG   (WINAPI *Release)(XAFactory *This);
    HRESULT (WINAPI *CreateInstance)(XAFactory *This, void *pUnkOuter, REFGUID_X riid, void **ppv);
    HRESULT (WINAPI *LockServer)(XAFactory *This, BOOL fLock);
} XAFactoryVtbl;
struct XAFactory {
    XAFactoryVtbl *lpVtbl;
};

/* ---- IXAudio2 ---- */

typedef struct XAEngine XAEngine;
typedef struct XAEngineVtbl {
    HRESULT (WINAPI *QueryInterface)(XAEngine *This, REFGUID_X riid, void **ppv);
    ULONG   (WINAPI *AddRef)(XAEngine *This);
    ULONG   (WINAPI *Release)(XAEngine *This);
    HRESULT (WINAPI *GetDeviceCount)(XAEngine *This, UINT32 *pCount);
    HRESULT (WINAPI *GetDeviceDetails)(XAEngine *This, UINT32 Index, XA_DEVICE_DETAILS *pDeviceDetails);
    HRESULT (WINAPI *Initialize)(XAEngine *This, UINT32 Flags, UINT32 XAudio2Processor);
    HRESULT (WINAPI *RegisterForCallbacks)(XAEngine *This, void *pCallback);
    void    (WINAPI *UnregisterForCallbacks)(XAEngine *This, void *pCallback);
    HRESULT (WINAPI *CreateSourceVoice)(XAEngine *This, void **ppSourceVoice,
                 const void *pSourceFormat, UINT32 Flags, float MaxFrequencyRatio,
                 void *pCallback, const void *pSendList, const void *pEffectChain);
    HRESULT (WINAPI *CreateSubmixVoice)(XAEngine *This, void **ppSubmixVoice,
                 UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags,
                 UINT32 ProcessingStage, const void *pSendList, const void *pEffectChain);
    HRESULT (WINAPI *CreateMasteringVoice)(XAEngine *This, void **ppMasteringVoice,
                 UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags,
                 UINT32 DeviceIndex, const void *pEffectChain);
    HRESULT (WINAPI *StartEngine)(XAEngine *This);
    void    (WINAPI *StopEngine)(XAEngine *This);
    HRESULT (WINAPI *CommitChanges)(XAEngine *This, UINT32 OperationSet);
    void    (WINAPI *GetPerformanceData)(XAEngine *This, XA_PERFORMANCE_DATA *pPerfData);
    void    (WINAPI *SetDebugConfiguration)(XAEngine *This, const void *pDebugConfiguration, void *pReserved);
} XAEngineVtbl;
struct XAEngine {
    XAEngineVtbl *lpVtbl;
    LONG refcount;
};

/* ---- IXAudio2Voice / IXAudio2SourceVoice (shared struct, two vtables) ---- */

typedef struct XAVoice XAVoice;
typedef struct XAVoiceVtbl {
    void    (WINAPI *GetVoiceDetails)(XAVoice *This, XA_VOICE_DETAILS *pVoiceDetails);
    HRESULT (WINAPI *SetOutputVoices)(XAVoice *This, const void *pSendList);
    HRESULT (WINAPI *SetEffectChain)(XAVoice *This, const void *pEffectChain);
    HRESULT (WINAPI *EnableEffect)(XAVoice *This, UINT32 EffectIndex, UINT32 OperationSet);
    HRESULT (WINAPI *DisableEffect)(XAVoice *This, UINT32 EffectIndex, UINT32 OperationSet);
    void    (WINAPI *GetEffectState)(XAVoice *This, UINT32 EffectIndex, BOOL *pEnabled);
    HRESULT (WINAPI *SetEffectParameters)(XAVoice *This, UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet);
    HRESULT (WINAPI *GetEffectParameters)(XAVoice *This, UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize);
    HRESULT (WINAPI *SetFilterParameters)(XAVoice *This, const XA_FILTER_PARAMETERS *pParameters, UINT32 OperationSet);
    void    (WINAPI *GetFilterParameters)(XAVoice *This, XA_FILTER_PARAMETERS *pParameters);
    HRESULT (WINAPI *SetOutputFilterParameters)(XAVoice *This, XAVoice *pDestinationVoice, const XA_FILTER_PARAMETERS *pParameters, UINT32 OperationSet);
    void    (WINAPI *GetOutputFilterParameters)(XAVoice *This, XAVoice *pDestinationVoice, XA_FILTER_PARAMETERS *pParameters);
    HRESULT (WINAPI *SetVolume)(XAVoice *This, float Volume, UINT32 OperationSet);
    void    (WINAPI *GetVolume)(XAVoice *This, float *pVolume);
    HRESULT (WINAPI *SetChannelVolumes)(XAVoice *This, UINT32 Channels, const float *pVolumes, UINT32 OperationSet);
    void    (WINAPI *GetChannelVolumes)(XAVoice *This, UINT32 Channels, float *pVolumes);
    HRESULT (WINAPI *SetOutputMatrix)(XAVoice *This, XAVoice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet);
    void    (WINAPI *GetOutputMatrix)(XAVoice *This, XAVoice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix);
    void    (WINAPI *DestroyVoice)(XAVoice *This);
} XAVoiceVtbl;

typedef struct XASourceVoiceVtbl {
    /* IXAudio2Voice base, must match XAVoiceVtbl layout exactly */
    void    (WINAPI *GetVoiceDetails)(XAVoice *This, XA_VOICE_DETAILS *pVoiceDetails);
    HRESULT (WINAPI *SetOutputVoices)(XAVoice *This, const void *pSendList);
    HRESULT (WINAPI *SetEffectChain)(XAVoice *This, const void *pEffectChain);
    HRESULT (WINAPI *EnableEffect)(XAVoice *This, UINT32 EffectIndex, UINT32 OperationSet);
    HRESULT (WINAPI *DisableEffect)(XAVoice *This, UINT32 EffectIndex, UINT32 OperationSet);
    void    (WINAPI *GetEffectState)(XAVoice *This, UINT32 EffectIndex, BOOL *pEnabled);
    HRESULT (WINAPI *SetEffectParameters)(XAVoice *This, UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet);
    HRESULT (WINAPI *GetEffectParameters)(XAVoice *This, UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize);
    HRESULT (WINAPI *SetFilterParameters)(XAVoice *This, const XA_FILTER_PARAMETERS *pParameters, UINT32 OperationSet);
    void    (WINAPI *GetFilterParameters)(XAVoice *This, XA_FILTER_PARAMETERS *pParameters);
    HRESULT (WINAPI *SetOutputFilterParameters)(XAVoice *This, XAVoice *pDestinationVoice, const XA_FILTER_PARAMETERS *pParameters, UINT32 OperationSet);
    void    (WINAPI *GetOutputFilterParameters)(XAVoice *This, XAVoice *pDestinationVoice, XA_FILTER_PARAMETERS *pParameters);
    HRESULT (WINAPI *SetVolume)(XAVoice *This, float Volume, UINT32 OperationSet);
    void    (WINAPI *GetVolume)(XAVoice *This, float *pVolume);
    HRESULT (WINAPI *SetChannelVolumes)(XAVoice *This, UINT32 Channels, const float *pVolumes, UINT32 OperationSet);
    void    (WINAPI *GetChannelVolumes)(XAVoice *This, UINT32 Channels, float *pVolumes);
    HRESULT (WINAPI *SetOutputMatrix)(XAVoice *This, XAVoice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet);
    void    (WINAPI *GetOutputMatrix)(XAVoice *This, XAVoice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix);
    void    (WINAPI *DestroyVoice)(XAVoice *This);
    /* IXAudio2SourceVoice extra methods */
    HRESULT (WINAPI *Start)(XAVoice *This, UINT32 Flags, UINT32 OperationSet);
    HRESULT (WINAPI *Stop)(XAVoice *This, UINT32 Flags, UINT32 OperationSet);
    HRESULT (WINAPI *SubmitSourceBuffer)(XAVoice *This, const void *pBuffer, const void *pBufferWMA);
    HRESULT (WINAPI *FlushSourceBuffers)(XAVoice *This);
    HRESULT (WINAPI *Discontinuity)(XAVoice *This);
    HRESULT (WINAPI *ExitLoop)(XAVoice *This, UINT32 OperationSet);
    void    (WINAPI *GetState)(XAVoice *This, XA_VOICE_STATE *pVoiceState);
    HRESULT (WINAPI *SetFrequencyRatio)(XAVoice *This, float Ratio, UINT32 OperationSet);
    void    (WINAPI *GetFrequencyRatio)(XAVoice *This, float *pRatio);
    HRESULT (WINAPI *SetSourceSampleRate)(XAVoice *This, UINT32 NewSourceSampleRate);
} XASourceVoiceVtbl;

struct XAVoice {
    void *lpVtbl; /* points at either g_voice_vtbl or g_source_voice_vtbl */
    UINT32 channels;
    UINT32 sampleRate;
    UINT32 creationFlags;
};

/* ---- base voice method impls (shared by submix/master/source) ---- */

static void WINAPI v_GetVoiceDetails(XAVoice *This, XA_VOICE_DETAILS *pVoiceDetails)
{
    if (!pVoiceDetails) return;
    pVoiceDetails->CreationFlags = This->creationFlags;
    pVoiceDetails->InputChannels = This->channels;
    pVoiceDetails->InputSampleRate = This->sampleRate;
}
static HRESULT WINAPI v_SetOutputVoices(XAVoice *This, const void *pSendList) { (void)This; (void)pSendList; return S_OK; }
static HRESULT WINAPI v_SetEffectChain(XAVoice *This, const void *pEffectChain) { (void)This; (void)pEffectChain; return S_OK; }
static HRESULT WINAPI v_EnableEffect(XAVoice *This, UINT32 e, UINT32 o) { (void)This; (void)e; (void)o; return S_OK; }
static HRESULT WINAPI v_DisableEffect(XAVoice *This, UINT32 e, UINT32 o) { (void)This; (void)e; (void)o; return S_OK; }
static void WINAPI v_GetEffectState(XAVoice *This, UINT32 e, BOOL *pEnabled) { (void)This; (void)e; if (pEnabled) *pEnabled = FALSE; }
static HRESULT WINAPI v_SetEffectParameters(XAVoice *This, UINT32 e, const void *p, UINT32 sz, UINT32 o) { (void)This; (void)e; (void)p; (void)sz; (void)o; return S_OK; }
static HRESULT WINAPI v_GetEffectParameters(XAVoice *This, UINT32 e, void *p, UINT32 sz) { (void)This; (void)e; if (p && sz) ZeroMemory(p, sz); return S_OK; }
static HRESULT WINAPI v_SetFilterParameters(XAVoice *This, const XA_FILTER_PARAMETERS *p, UINT32 o) { (void)This; (void)p; (void)o; return S_OK; }
static void WINAPI v_GetFilterParameters(XAVoice *This, XA_FILTER_PARAMETERS *p) { (void)This; if (p) ZeroMemory(p, sizeof(*p)); }
static HRESULT WINAPI v_SetOutputFilterParameters(XAVoice *This, XAVoice *dst, const XA_FILTER_PARAMETERS *p, UINT32 o) { (void)This; (void)dst; (void)p; (void)o; return S_OK; }
static void WINAPI v_GetOutputFilterParameters(XAVoice *This, XAVoice *dst, XA_FILTER_PARAMETERS *p) { (void)This; (void)dst; if (p) ZeroMemory(p, sizeof(*p)); }
static HRESULT WINAPI v_SetVolume(XAVoice *This, float vol, UINT32 o) { (void)This; (void)vol; (void)o; return S_OK; }
static void WINAPI v_GetVolume(XAVoice *This, float *pVolume) { (void)This; if (pVolume) *pVolume = 1.0f; }
static HRESULT WINAPI v_SetChannelVolumes(XAVoice *This, UINT32 ch, const float *pv, UINT32 o) { (void)This; (void)ch; (void)pv; (void)o; return S_OK; }
static void WINAPI v_GetChannelVolumes(XAVoice *This, UINT32 ch, float *pv)
{
    UINT32 i;
    (void)This;
    if (!pv) return;
    for (i = 0; i < ch; i++) pv[i] = 1.0f;
}
static HRESULT WINAPI v_SetOutputMatrix(XAVoice *This, XAVoice *dst, UINT32 sc, UINT32 dc, const float *m, UINT32 o) { (void)This; (void)dst; (void)sc; (void)dc; (void)m; (void)o; return S_OK; }
static void WINAPI v_GetOutputMatrix(XAVoice *This, XAVoice *dst, UINT32 sc, UINT32 dc, float *m)
{
    UINT32 i;
    (void)This; (void)dst;
    if (!m) return;
    for (i = 0; i < sc * dc; i++) m[i] = 0.0f;
}
static void WINAPI v_DestroyVoice(XAVoice *This)
{
    xa_log("voice DestroyVoice");
    if (This) HeapFree(GetProcessHeap(), 0, This);
}

/* ---- source voice extra method impls ---- */

static HRESULT WINAPI sv_Start(XAVoice *This, UINT32 f, UINT32 o) { (void)This; (void)f; (void)o; xa_log("voice Start"); return S_OK; }
static HRESULT WINAPI sv_Stop(XAVoice *This, UINT32 f, UINT32 o) { (void)This; (void)f; (void)o; xa_log("voice Stop"); return S_OK; }
static HRESULT WINAPI sv_SubmitSourceBuffer(XAVoice *This, const void *pBuffer, const void *pBufferWMA)
{
    (void)This; (void)pBuffer; (void)pBufferWMA;
    xa_log("voice SubmitSourceBuffer");
    return S_OK;
}
static HRESULT WINAPI sv_FlushSourceBuffers(XAVoice *This) { (void)This; return S_OK; }
static HRESULT WINAPI sv_Discontinuity(XAVoice *This) { (void)This; return S_OK; }
static HRESULT WINAPI sv_ExitLoop(XAVoice *This, UINT32 o) { (void)This; (void)o; return S_OK; }
static void WINAPI sv_GetState(XAVoice *This, XA_VOICE_STATE *pVoiceState)
{
    (void)This;
    if (!pVoiceState) return;
    pVoiceState->pCurrentBufferContext = NULL;
    pVoiceState->BuffersQueued = 0;   /* report immediate completion: no real device is draining anything */
    pVoiceState->SamplesPlayed = 0;
}
static HRESULT WINAPI sv_SetFrequencyRatio(XAVoice *This, float r, UINT32 o) { (void)This; (void)r; (void)o; return S_OK; }
static void WINAPI sv_GetFrequencyRatio(XAVoice *This, float *pRatio) { (void)This; if (pRatio) *pRatio = 1.0f; }
static HRESULT WINAPI sv_SetSourceSampleRate(XAVoice *This, UINT32 rate) { (void)This; (void)rate; return S_OK; }

static XAVoiceVtbl g_voice_vtbl = {
    v_GetVoiceDetails, v_SetOutputVoices, v_SetEffectChain, v_EnableEffect, v_DisableEffect,
    v_GetEffectState, v_SetEffectParameters, v_GetEffectParameters, v_SetFilterParameters,
    v_GetFilterParameters, v_SetOutputFilterParameters, v_GetOutputFilterParameters,
    v_SetVolume, v_GetVolume, v_SetChannelVolumes, v_GetChannelVolumes,
    v_SetOutputMatrix, v_GetOutputMatrix, v_DestroyVoice
};

static XASourceVoiceVtbl g_source_voice_vtbl = {
    v_GetVoiceDetails, v_SetOutputVoices, v_SetEffectChain, v_EnableEffect, v_DisableEffect,
    v_GetEffectState, v_SetEffectParameters, v_GetEffectParameters, v_SetFilterParameters,
    v_GetFilterParameters, v_SetOutputFilterParameters, v_GetOutputFilterParameters,
    v_SetVolume, v_GetVolume, v_SetChannelVolumes, v_GetChannelVolumes,
    v_SetOutputMatrix, v_GetOutputMatrix, v_DestroyVoice,
    sv_Start, sv_Stop, sv_SubmitSourceBuffer, sv_FlushSourceBuffers, sv_Discontinuity,
    sv_ExitLoop, sv_GetState, sv_SetFrequencyRatio, sv_GetFrequencyRatio, sv_SetSourceSampleRate
};

static XAVoice *xa_new_voice(void *vtbl, UINT32 channels, UINT32 sampleRate, UINT32 flags)
{
    XAVoice *v = (XAVoice *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(XAVoice));
    if (!v) return NULL;
    v->lpVtbl = vtbl;
    v->channels = channels;
    v->sampleRate = sampleRate;
    v->creationFlags = flags;
    return v;
}

/* ---- IXAudio2 method impls ---- */

static HRESULT WINAPI xa_QueryInterface(XAEngine *This, REFGUID_X riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    if (xa_guid_eq(riid, &XA_IID_IUnknown) || xa_guid_eq(riid, &XA_IID_IXAudio2)) {
        This->refcount++;
        *ppv = This;
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}
static ULONG WINAPI xa_AddRef(XAEngine *This) { return (ULONG)++This->refcount; }
static ULONG WINAPI xa_Release(XAEngine *This)
{
    LONG r = --This->refcount;
    if (r <= 0) {
        xa_log("engine Release -> free");
        HeapFree(GetProcessHeap(), 0, This);
        return 0;
    }
    return (ULONG)r;
}
static HRESULT WINAPI xa_GetDeviceCount(XAEngine *This, UINT32 *pCount)
{
    (void)This;
    xa_log("GetDeviceCount");
    if (pCount) *pCount = 1;
    return S_OK;
}
static HRESULT WINAPI xa_GetDeviceDetails(XAEngine *This, UINT32 Index, XA_DEVICE_DETAILS *pDeviceDetails)
{
    (void)This; (void)Index;
    xa_log("GetDeviceDetails");
    if (!pDeviceDetails) return E_POINTER;
    ZeroMemory(pDeviceDetails, sizeof(*pDeviceDetails));
    lstrcpyW(pDeviceDetails->DeviceID, L"XASHIM_DEVICE_0");
    lstrcpyW(pDeviceDetails->DisplayName, L"No-op Audio Device");
    pDeviceDetails->Role = 0xF; /* GlobalDefaultDevice */
    return S_OK;
}
static HRESULT WINAPI xa_Initialize(XAEngine *This, UINT32 Flags, UINT32 Processor)
{
    (void)This; (void)Flags; (void)Processor;
    xa_log("Initialize");
    return S_OK;
}
static HRESULT WINAPI xa_RegisterForCallbacks(XAEngine *This, void *pCallback) { (void)This; (void)pCallback; return S_OK; }
static void WINAPI xa_UnregisterForCallbacks(XAEngine *This, void *pCallback) { (void)This; (void)pCallback; }
static HRESULT WINAPI xa_CreateSourceVoice(XAEngine *This, void **ppSourceVoice,
    const void *pSourceFormat, UINT32 Flags, float MaxFrequencyRatio,
    void *pCallback, const void *pSendList, const void *pEffectChain)
{
    XAVoice *v;
    const XA_WAVEFORMATEX *fmt = (const XA_WAVEFORMATEX *)pSourceFormat;
    UINT32 channels = 2, rate = 44100;

    (void)MaxFrequencyRatio; (void)pCallback; (void)pSendList; (void)pEffectChain;
    xa_log("CreateSourceVoice");
    if (!ppSourceVoice) return E_POINTER;
    if (fmt) { channels = fmt->nChannels; rate = fmt->nSamplesPerSec; }
    v = xa_new_voice(&g_source_voice_vtbl, channels, rate, Flags);
    if (!v) { *ppSourceVoice = NULL; return E_OUTOFMEMORY; }
    *ppSourceVoice = v;
    return S_OK;
}
static HRESULT WINAPI xa_CreateSubmixVoice(XAEngine *This, void **ppSubmixVoice,
    UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags,
    UINT32 ProcessingStage, const void *pSendList, const void *pEffectChain)
{
    XAVoice *v;
    (void)This; (void)ProcessingStage; (void)pSendList; (void)pEffectChain;
    xa_log("CreateSubmixVoice");
    if (!ppSubmixVoice) return E_POINTER;
    v = xa_new_voice(&g_voice_vtbl, InputChannels, InputSampleRate, Flags);
    if (!v) { *ppSubmixVoice = NULL; return E_OUTOFMEMORY; }
    *ppSubmixVoice = v;
    return S_OK;
}
static HRESULT WINAPI xa_CreateMasteringVoice(XAEngine *This, void **ppMasteringVoice,
    UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags,
    UINT32 DeviceIndex, const void *pEffectChain)
{
    XAVoice *v;
    (void)This; (void)DeviceIndex; (void)pEffectChain;
    xa_log("CreateMasteringVoice");
    if (!ppMasteringVoice) return E_POINTER;
    if (InputChannels == 0) InputChannels = 2;
    if (InputSampleRate == 0) InputSampleRate = 44100;
    v = xa_new_voice(&g_voice_vtbl, InputChannels, InputSampleRate, Flags);
    if (!v) { *ppMasteringVoice = NULL; return E_OUTOFMEMORY; }
    *ppMasteringVoice = v;
    return S_OK;
}
static HRESULT WINAPI xa_StartEngine(XAEngine *This) { (void)This; xa_log("StartEngine"); return S_OK; }
static void WINAPI xa_StopEngine(XAEngine *This) { (void)This; xa_log("StopEngine"); }
static HRESULT WINAPI xa_CommitChanges(XAEngine *This, UINT32 OperationSet) { (void)This; (void)OperationSet; return S_OK; }
static void WINAPI xa_GetPerformanceData(XAEngine *This, XA_PERFORMANCE_DATA *pPerfData) { (void)This; if (pPerfData) ZeroMemory(pPerfData, sizeof(*pPerfData)); }
static void WINAPI xa_SetDebugConfiguration(XAEngine *This, const void *pDebugConfiguration, void *pReserved) { (void)This; (void)pDebugConfiguration; (void)pReserved; }

static XAEngineVtbl g_engine_vtbl = {
    xa_QueryInterface, xa_AddRef, xa_Release,
    xa_GetDeviceCount, xa_GetDeviceDetails, xa_Initialize,
    xa_RegisterForCallbacks, xa_UnregisterForCallbacks,
    xa_CreateSourceVoice, xa_CreateSubmixVoice, xa_CreateMasteringVoice,
    xa_StartEngine, xa_StopEngine, xa_CommitChanges,
    xa_GetPerformanceData, xa_SetDebugConfiguration
};

/* ---- IClassFactory ---- */

static HRESULT WINAPI cf_QueryInterface(XAFactory *This, REFGUID_X riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    if (xa_guid_eq(riid, &XA_IID_IUnknown) || xa_guid_eq(riid, &XA_IID_IClassFactory)) {
        *ppv = This;
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}
static ULONG WINAPI cf_AddRef(XAFactory *This) { (void)This; return 1; }
static ULONG WINAPI cf_Release(XAFactory *This) { (void)This; return 1; }
static HRESULT WINAPI cf_CreateInstance(XAFactory *This, void *pUnkOuter, REFGUID_X riid, void **ppv)
{
    XAEngine *e;

    (void)This;
    xa_log("CreateInstance");
    if (ppv) *ppv = NULL;
    if (pUnkOuter) return CLASS_E_NOAGGREGATION;
    if (!ppv) return E_POINTER;
    if (!xa_guid_eq(riid, &XA_IID_IUnknown) && !xa_guid_eq(riid, &XA_IID_IXAudio2)) {
        return E_NOINTERFACE;
    }

    e = (XAEngine *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(XAEngine));
    if (!e) return E_OUTOFMEMORY;
    e->lpVtbl = &g_engine_vtbl;
    e->refcount = 1;
    *ppv = e;
    return S_OK;
}
static HRESULT WINAPI cf_LockServer(XAFactory *This, BOOL fLock) { (void)This; (void)fLock; return S_OK; }

static XAFactoryVtbl g_factory_vtbl = {
    cf_QueryInterface, cf_AddRef, cf_Release, cf_CreateInstance, cf_LockServer
};
static XAFactory g_factory = { &g_factory_vtbl };

/* ---- DLL exports ---- */

HRESULT WINAPI DllGetClassObject(REFGUID_X rclsid, REFGUID_X riid, LPVOID *ppv)
{
    xa_log("DllGetClassObject");
    if (!ppv) return E_POINTER;
    *ppv = NULL;
    if (!xa_guid_eq(rclsid, &XA_CLSID_XAudio2_6)) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
    if (!xa_guid_eq(riid, &XA_IID_IUnknown) && !xa_guid_eq(riid, &XA_IID_IClassFactory)) {
        return E_NOINTERFACE;
    }
    *ppv = &g_factory;
    return S_OK;
}

HRESULT WINAPI DllCanUnloadNow(void)
{
    return S_FALSE;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    (void)hinst;
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        xa_log("XASHIM DllMain attach");
    }
    return TRUE;
}
