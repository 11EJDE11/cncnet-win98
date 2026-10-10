/* OLET3.DLL - stands in for ole32 in XNA's DLLs (their ole32 import is renamed to OLET3).
 * Everything is forwarded to the real ole32 except CoCreateInstance: when Windows 98 has no
 * registered server for a class, it loads the in-process server directly - WIC
 * (WindowsCodecs.dll) for image decoding, XAudio2_6.dll for audio, wmiutils for the WMI
 * path parser - and answers the Windows 7 taskbar progress object with a no-op.
 * Set OLET3_LOG=1 to log to C:\DOTNET48\OLET3.LOG. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static FARPROC p_CLIPFORMAT_UserFree;
static FARPROC p_CLIPFORMAT_UserMarshal;
static FARPROC p_CLIPFORMAT_UserSize;
static FARPROC p_CLIPFORMAT_UserUnmarshal;
static FARPROC p_CLSIDFromString;
static FARPROC p_CoCreateInstance;
static FARPROC p_CoInitializeEx;
static FARPROC p_CoGetMalloc;
static FARPROC p_CoLockObjectExternal;
static FARPROC p_CoTaskMemAlloc;
static FARPROC p_CoTaskMemFree;
static FARPROC p_CoTaskMemRealloc;
static FARPROC p_CoUninitialize;
static FARPROC p_CreateStreamOnHGlobal;
static FARPROC p_GetHGlobalFromStream;
static FARPROC p_HBITMAP_UserFree;
static FARPROC p_HBITMAP_UserMarshal;
static FARPROC p_HBITMAP_UserSize;
static FARPROC p_HBITMAP_UserUnmarshal;
static FARPROC p_HPALETTE_UserFree;
static FARPROC p_HPALETTE_UserMarshal;
static FARPROC p_HPALETTE_UserSize;
static FARPROC p_HPALETTE_UserUnmarshal;
static FARPROC p_PropVariantClear;
static FARPROC p_PropVariantCopy;
static FARPROC p_StringFromGUID2;
static FARPROC p_StringFromIID;

typedef const GUID *OLET3_REFGUID;
typedef void *OLET3_LPUNKNOWN;

typedef struct OLET3_IClassFactory OLET3_IClassFactory;
typedef struct OLET3_IClassFactoryVtbl {
    HRESULT (WINAPI *QueryInterface)(OLET3_IClassFactory *This, OLET3_REFGUID riid, void **ppvObject);
    ULONG (WINAPI *AddRef)(OLET3_IClassFactory *This);
    ULONG (WINAPI *Release)(OLET3_IClassFactory *This);
    HRESULT (WINAPI *CreateInstance)(OLET3_IClassFactory *This, OLET3_LPUNKNOWN pUnkOuter, OLET3_REFGUID riid, void **ppvObject);
    HRESULT (WINAPI *LockServer)(OLET3_IClassFactory *This, BOOL fLock);
} OLET3_IClassFactoryVtbl;
struct OLET3_IClassFactory {
    OLET3_IClassFactoryVtbl *lpVtbl;
};

typedef HRESULT (WINAPI *PFN_CoCreateInstance)(OLET3_REFGUID rclsid, OLET3_LPUNKNOWN pUnkOuter, DWORD dwClsContext, OLET3_REFGUID riid, LPVOID *ppv);
typedef HRESULT (WINAPI *PFN_DllGetClassObject)(OLET3_REFGUID rclsid, OLET3_REFGUID riid, LPVOID *ppv);

static int olet3_guid_eq(OLET3_REFGUID a, OLET3_REFGUID b);

static const GUID olet3_IID_IClassFactory = {0x00000001,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID olet3_IID_IUnknown = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID olet3_CLSID_WbemDefaultPathParser = {0xCF4CC405,0xE2C5,0x4DDD,{0xB3,0xCE,0x5E,0x75,0x82,0xD8,0xC9,0xFA}};
static const GUID olet3_CLSID_XAudio2_6 = {0x3EDA9B49,0x2085,0x498B,{0x9B,0xB2,0x39,0xA6,0x77,0x84,0x93,0xDE}};

/* Windows 7 taskbar progress (CLSID_TaskbarList / ITaskbarList3) has no server on Windows 98.
 * It is cosmetic, so a static object whose methods up to SetProgressState return S_OK. */
static const GUID olet3_CLSID_TaskbarList = {0x56FDF344,0xFD6D,0x11D0,{0x95,0x8A,0x00,0x60,0x97,0xC9,0xA0,0x90}};
static const GUID olet3_IID_ITaskbarList3 = {0xEA1AFB91,0x9E28,0x4B86,{0x90,0xE9,0x9E,0x9F,0x8A,0x5E,0xEF,0xAF}};

typedef struct Olet3TaskbarList Olet3TaskbarList;
typedef struct Olet3TaskbarListVtbl {
    HRESULT (WINAPI *QueryInterface)(Olet3TaskbarList *This, OLET3_REFGUID riid, void **ppv);
    ULONG   (WINAPI *AddRef)(Olet3TaskbarList *This);
    ULONG   (WINAPI *Release)(Olet3TaskbarList *This);
    HRESULT (WINAPI *HrInit)(Olet3TaskbarList *This);
    HRESULT (WINAPI *AddTab)(Olet3TaskbarList *This, HWND hwnd);
    HRESULT (WINAPI *DeleteTab)(Olet3TaskbarList *This, HWND hwnd);
    HRESULT (WINAPI *ActivateTab)(Olet3TaskbarList *This, HWND hwnd);
    HRESULT (WINAPI *SetActiveAlt)(Olet3TaskbarList *This, HWND hwnd);
    HRESULT (WINAPI *MarkFullscreenWindow)(Olet3TaskbarList *This, HWND hwnd, BOOL fFullscreen);
    HRESULT (WINAPI *SetProgressValue)(Olet3TaskbarList *This, HWND hwnd, ULONGLONG ullCompleted, ULONGLONG ullTotal);
    HRESULT (WINAPI *SetProgressState)(Olet3TaskbarList *This, HWND hwnd, DWORD state);
} Olet3TaskbarListVtbl;
struct Olet3TaskbarList {
    Olet3TaskbarListVtbl *lpVtbl;
};

static HRESULT WINAPI olet3_tbl_QueryInterface(Olet3TaskbarList *This, OLET3_REFGUID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    if (olet3_guid_eq(riid, &olet3_IID_IUnknown) || olet3_guid_eq(riid, &olet3_IID_ITaskbarList3)) {
        *ppv = This;
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}
static ULONG WINAPI olet3_tbl_AddRef(Olet3TaskbarList *This) { (void)This; return 1; }
static ULONG WINAPI olet3_tbl_Release(Olet3TaskbarList *This) { (void)This; return 1; }
static HRESULT WINAPI olet3_tbl_HrInit(Olet3TaskbarList *This) { (void)This; return S_OK; }
static HRESULT WINAPI olet3_tbl_AddTab(Olet3TaskbarList *This, HWND hwnd) { (void)This; (void)hwnd; return S_OK; }
static HRESULT WINAPI olet3_tbl_DeleteTab(Olet3TaskbarList *This, HWND hwnd) { (void)This; (void)hwnd; return S_OK; }
static HRESULT WINAPI olet3_tbl_ActivateTab(Olet3TaskbarList *This, HWND hwnd) { (void)This; (void)hwnd; return S_OK; }
static HRESULT WINAPI olet3_tbl_SetActiveAlt(Olet3TaskbarList *This, HWND hwnd) { (void)This; (void)hwnd; return S_OK; }
static HRESULT WINAPI olet3_tbl_MarkFullscreenWindow(Olet3TaskbarList *This, HWND hwnd, BOOL f) { (void)This; (void)hwnd; (void)f; return S_OK; }
static HRESULT WINAPI olet3_tbl_SetProgressValue(Olet3TaskbarList *This, HWND hwnd, ULONGLONG c, ULONGLONG t) { (void)This; (void)hwnd; (void)c; (void)t; return S_OK; }
static HRESULT WINAPI olet3_tbl_SetProgressState(Olet3TaskbarList *This, HWND hwnd, DWORD state) { (void)This; (void)hwnd; (void)state; return S_OK; }

static Olet3TaskbarListVtbl olet3_taskbarlist_vtbl = {
    olet3_tbl_QueryInterface, olet3_tbl_AddRef, olet3_tbl_Release,
    olet3_tbl_HrInit, olet3_tbl_AddTab, olet3_tbl_DeleteTab, olet3_tbl_ActivateTab,
    olet3_tbl_SetActiveAlt, olet3_tbl_MarkFullscreenWindow,
    olet3_tbl_SetProgressValue, olet3_tbl_SetProgressState
};
static Olet3TaskbarList olet3_taskbarlist_instance = { &olet3_taskbarlist_vtbl };

static int olet3_guid_eq(OLET3_REFGUID a, OLET3_REFGUID b)
{
    const BYTE *pa;
    const BYTE *pb;
    int i;

    pa = (const BYTE *)a;
    pb = (const BYTE *)b;
    for (i = 0; i < sizeof(GUID); i++) {
        if (pa[i] != pb[i]) {
            return 0;
        }
    }
    return 1;
}

static void olet3_guid_to_string(OLET3_REFGUID g, char *buf)
{
    wsprintfA(buf,
        "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        g->Data1,
        g->Data2,
        g->Data3,
        g->Data4[0],
        g->Data4[1],
        g->Data4[2],
        g->Data4[3],
        g->Data4[4],
        g->Data4[5],
        g->Data4[6],
        g->Data4[7]);
}

/* Logging is off unless OLET3_LOG=1: each line opens and closes the log file, and this is
 * on the CoCreateInstance path. */
static int olet3_log_enabled(void)
{
    static int state = -1;   /* -1 = not yet checked */
    if (state < 0) {
        char v[8];
        state = (GetEnvironmentVariableA("OLET3_LOG", v, sizeof(v)) > 0 && v[0] == '1') ? 1 : 0;
    }
    return state;
}

static void olet3_log_line(const char *line)
{
    HANDLE h;
    DWORD written;
    DWORD len;

    if (!olet3_log_enabled()) {
        return;
    }
    h = CreateFileA("C:\\DOTNET48\\OLET3.LOG", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    SetFilePointer(h, 0, NULL, FILE_END);
    len = lstrlenA(line);
    WriteFile(h, line, len, &written, NULL);
    WriteFile(h, "\r\n", 2, &written, NULL);
    CloseHandle(h);
}

static void olet3_log_hr(const char *tag, const char *server, OLET3_REFGUID clsid, OLET3_REFGUID riid, HRESULT hr, DWORD gle)
{
    char cbuf[64];
    char ibuf[64];
    char line[256];

    olet3_guid_to_string(clsid, cbuf);
    olet3_guid_to_string(riid, ibuf);
    wsprintfA(line, "CoCI %s server=%s clsid=%s iid=%s hr=%08lX gle=%lu",
              tag, server ? server : "-", cbuf, ibuf, (DWORD)hr, gle);
    olet3_log_line(line);
}

/* One LoadLibrary per server name. A server is asked for hundreds of times (WIC once per
 * texture), and loading it each time without FreeLibrary is slow and grows its load count
 * without limit. Textures are decoded on several threads, hence the lock. */
#define OLET3_MODCACHE_MAX 8
static char olet3_modcache_name[OLET3_MODCACHE_MAX][64];
static HMODULE olet3_modcache_mod[OLET3_MODCACHE_MAX];
static int olet3_modcache_count = 0;
static CRITICAL_SECTION olet3_modcache_lock;

static HMODULE olet3_load_cached(const char *server)
{
    int i;
    HMODULE mod;

    EnterCriticalSection(&olet3_modcache_lock);
    for (i = 0; i < olet3_modcache_count; i++) {
        if (lstrcmpiA(olet3_modcache_name[i], server) == 0) {
            mod = olet3_modcache_mod[i];
            LeaveCriticalSection(&olet3_modcache_lock);
            return mod;
        }
    }

    mod = LoadLibraryA(server);
    if (mod && olet3_modcache_count < OLET3_MODCACHE_MAX) {
        lstrcpynA(olet3_modcache_name[olet3_modcache_count], server,
                  sizeof(olet3_modcache_name[olet3_modcache_count]));
        olet3_modcache_mod[olet3_modcache_count] = mod;
        olet3_modcache_count++;
    }
    LeaveCriticalSection(&olet3_modcache_lock);
    return mod;
}

static HRESULT olet3_try_class_server(const char *server, OLET3_REFGUID rclsid, OLET3_LPUNKNOWN pUnkOuter, OLET3_REFGUID riid, LPVOID *ppv)
{
    HMODULE mod;
    PFN_DllGetClassObject dll_get_class_object;
    OLET3_IClassFactory *factory;
    HRESULT hr;
    HRESULT hr_create;
    DWORD gle;

    if (ppv) {
        *ppv = NULL;
    }

    mod = olet3_load_cached(server);
    if (!mod) {
        olet3_log_hr("loadfail", server, rclsid, riid, (HRESULT)0x8007007EL, GetLastError());
        return (HRESULT)0x80040154L;
    }

    dll_get_class_object = (PFN_DllGetClassObject)GetProcAddress(mod, "DllGetClassObject");
    if (!dll_get_class_object) {
        gle = GetLastError();
        olet3_log_hr("nogetclass", server, rclsid, riid, (HRESULT)0x8007007FL, gle);
        return (HRESULT)0x80040154L;
    }

    factory = NULL;
    hr = dll_get_class_object(rclsid, &olet3_IID_IClassFactory, (LPVOID *)&factory);
    olet3_log_hr("getclass", server, rclsid, &olet3_IID_IClassFactory, hr, GetLastError());
    if (FAILED(hr) || !factory) {
        return hr;
    }

    hr_create = factory->lpVtbl->CreateInstance(factory, pUnkOuter, riid, ppv);
    factory->lpVtbl->Release(factory);
    olet3_log_hr("create", server, rclsid, riid, hr_create, GetLastError());
    return hr_create;
}

static HRESULT WINAPI olet3_CoCreateInstance_impl(OLET3_REFGUID rclsid, OLET3_LPUNKNOWN pUnkOuter, DWORD dwClsContext, OLET3_REFGUID riid, LPVOID *ppv)
{
    PFN_CoCreateInstance real_create;
    HRESULT hr;
    HRESULT alt;

    if (ppv) {
        *ppv = NULL;
    }

    real_create = (PFN_CoCreateInstance)p_CoCreateInstance;
    if (!real_create) {
        hr = (HRESULT)0x80040154L;
    } else {
        hr = real_create(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }
    olet3_log_hr("real", "-", rclsid, riid, hr, GetLastError());
    if (SUCCEEDED(hr)) {
        return hr;
    }

    if ((DWORD)hr != 0x80040154UL) {
        return hr;
    }

    if (olet3_guid_eq(rclsid, &olet3_CLSID_XAudio2_6)) {
        alt = olet3_try_class_server("XAudio2_6.dll", rclsid, pUnkOuter, riid, ppv);
        if (SUCCEEDED(alt)) {
            return alt;
        }
    }

    if (olet3_guid_eq(rclsid, &olet3_CLSID_TaskbarList)) {
        if (ppv && (olet3_guid_eq(riid, &olet3_IID_IUnknown) || olet3_guid_eq(riid, &olet3_IID_ITaskbarList3))) {
            *ppv = &olet3_taskbarlist_instance;
            olet3_log_hr("taskbarlist noop", "-", rclsid, riid, S_OK, 0);
            return S_OK;
        }
        olet3_log_hr("taskbarlist noiface", "-", rclsid, riid, (HRESULT)0x80004002L, 0);
        return (HRESULT)0x80004002L; /* E_NOINTERFACE */
    }

    if (olet3_guid_eq(rclsid, &olet3_CLSID_WbemDefaultPathParser)) {
        alt = olet3_try_class_server("wmiutils.dll", rclsid, pUnkOuter, riid, ppv);
        if (SUCCEEDED(alt)) {
            return alt;
        }
        alt = olet3_try_class_server("wbem\\wmiutils.dll", rclsid, pUnkOuter, riid, ppv);
        if (SUCCEEDED(alt)) {
            return alt;
        }
    }

    alt = olet3_try_class_server("WindowsCodecs.dll", rclsid, pUnkOuter, riid, ppv);
    if (SUCCEEDED(alt)) {
        return alt;
    }

    return hr;
}

__declspec(naked) void CLIPFORMAT_UserFree(void) { __asm { jmp dword ptr [p_CLIPFORMAT_UserFree] } }
__declspec(naked) void CLIPFORMAT_UserMarshal(void) { __asm { jmp dword ptr [p_CLIPFORMAT_UserMarshal] } }
__declspec(naked) void CLIPFORMAT_UserSize(void) { __asm { jmp dword ptr [p_CLIPFORMAT_UserSize] } }
__declspec(naked) void CLIPFORMAT_UserUnmarshal(void) { __asm { jmp dword ptr [p_CLIPFORMAT_UserUnmarshal] } }
__declspec(naked) void CLSIDFromString(void) { __asm { jmp dword ptr [p_CLSIDFromString] } }
__declspec(naked) void CoCreateInstance(void) { __asm { jmp olet3_CoCreateInstance_impl } }
__declspec(naked) void CoInitializeEx(void) { __asm { jmp dword ptr [p_CoInitializeEx] } }
__declspec(naked) void CoGetMalloc(void) { __asm { jmp dword ptr [p_CoGetMalloc] } }
__declspec(naked) void CoLockObjectExternal(void) { __asm { jmp dword ptr [p_CoLockObjectExternal] } }
__declspec(naked) void CoTaskMemAlloc(void) { __asm { jmp dword ptr [p_CoTaskMemAlloc] } }
__declspec(naked) void CoTaskMemFree(void) { __asm { jmp dword ptr [p_CoTaskMemFree] } }
__declspec(naked) void CoTaskMemRealloc(void) { __asm { jmp dword ptr [p_CoTaskMemRealloc] } }
__declspec(naked) void CoUninitialize(void) { __asm { jmp dword ptr [p_CoUninitialize] } }
__declspec(naked) void CreateStreamOnHGlobal(void) { __asm { jmp dword ptr [p_CreateStreamOnHGlobal] } }
__declspec(naked) void GetHGlobalFromStream(void) { __asm { jmp dword ptr [p_GetHGlobalFromStream] } }
__declspec(naked) void HBITMAP_UserFree(void) { __asm { jmp dword ptr [p_HBITMAP_UserFree] } }
__declspec(naked) void HBITMAP_UserMarshal(void) { __asm { jmp dword ptr [p_HBITMAP_UserMarshal] } }
__declspec(naked) void HBITMAP_UserSize(void) { __asm { jmp dword ptr [p_HBITMAP_UserSize] } }
__declspec(naked) void HBITMAP_UserUnmarshal(void) { __asm { jmp dword ptr [p_HBITMAP_UserUnmarshal] } }
__declspec(naked) void HPALETTE_UserFree(void) { __asm { jmp dword ptr [p_HPALETTE_UserFree] } }
__declspec(naked) void HPALETTE_UserMarshal(void) { __asm { jmp dword ptr [p_HPALETTE_UserMarshal] } }
__declspec(naked) void HPALETTE_UserSize(void) { __asm { jmp dword ptr [p_HPALETTE_UserSize] } }
__declspec(naked) void HPALETTE_UserUnmarshal(void) { __asm { jmp dword ptr [p_HPALETTE_UserUnmarshal] } }
__declspec(naked) void PropVariantClear(void) { __asm { jmp dword ptr [p_PropVariantClear] } }
__declspec(naked) void PropVariantCopy(void) { __asm { jmp dword ptr [p_PropVariantCopy] } }
__declspec(naked) void StringFromGUID2(void) { __asm { jmp dword ptr [p_StringFromGUID2] } }
__declspec(naked) void StringFromIID(void) { __asm { jmp dword ptr [p_StringFromIID] } }

ULONG __stdcall HICON_UserSize(ULONG *pFlags, ULONG startingSize, HICON *pIcon)
{
    (void)pFlags;
    (void)pIcon;
    return startingSize + sizeof(HICON);
}

unsigned char *__stdcall HICON_UserMarshal(ULONG *pFlags, unsigned char *buffer, HICON *pIcon)
{
    (void)pFlags;
    if (buffer && pIcon) {
        *(HICON *)buffer = *pIcon;
        buffer += sizeof(HICON);
    }
    return buffer;
}

unsigned char *__stdcall HICON_UserUnmarshal(ULONG *pFlags, unsigned char *buffer, HICON *pIcon)
{
    (void)pFlags;
    if (buffer && pIcon) {
        *pIcon = *(HICON *)buffer;
        buffer += sizeof(HICON);
    }
    return buffer;
}

void __stdcall HICON_UserFree(ULONG *pFlags, HICON *pIcon)
{
    (void)pFlags;
    (void)pIcon;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    HMODULE ole;

    (void)hinst;
    (void)reserved;

    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    InitializeCriticalSection(&olet3_modcache_lock);
    ole = LoadLibraryA("OLE32.DLL");
    if (!ole) {
        return FALSE;
    }

    p_CLIPFORMAT_UserFree = GetProcAddress(ole, "CLIPFORMAT_UserFree");
    p_CLIPFORMAT_UserMarshal = GetProcAddress(ole, "CLIPFORMAT_UserMarshal");
    p_CLIPFORMAT_UserSize = GetProcAddress(ole, "CLIPFORMAT_UserSize");
    p_CLIPFORMAT_UserUnmarshal = GetProcAddress(ole, "CLIPFORMAT_UserUnmarshal");
    p_CLSIDFromString = GetProcAddress(ole, "CLSIDFromString");
    p_CoCreateInstance = GetProcAddress(ole, "CoCreateInstance");
    p_CoInitializeEx = GetProcAddress(ole, "CoInitializeEx");
    p_CoGetMalloc = GetProcAddress(ole, "CoGetMalloc");
    p_CoLockObjectExternal = GetProcAddress(ole, "CoLockObjectExternal");
    p_CoTaskMemAlloc = GetProcAddress(ole, "CoTaskMemAlloc");
    p_CoTaskMemFree = GetProcAddress(ole, "CoTaskMemFree");
    p_CoTaskMemRealloc = GetProcAddress(ole, "CoTaskMemRealloc");
    p_CoUninitialize = GetProcAddress(ole, "CoUninitialize");
    p_CreateStreamOnHGlobal = GetProcAddress(ole, "CreateStreamOnHGlobal");
    p_GetHGlobalFromStream = GetProcAddress(ole, "GetHGlobalFromStream");
    p_HBITMAP_UserFree = GetProcAddress(ole, "HBITMAP_UserFree");
    p_HBITMAP_UserMarshal = GetProcAddress(ole, "HBITMAP_UserMarshal");
    p_HBITMAP_UserSize = GetProcAddress(ole, "HBITMAP_UserSize");
    p_HBITMAP_UserUnmarshal = GetProcAddress(ole, "HBITMAP_UserUnmarshal");
    p_HPALETTE_UserFree = GetProcAddress(ole, "HPALETTE_UserFree");
    p_HPALETTE_UserMarshal = GetProcAddress(ole, "HPALETTE_UserMarshal");
    p_HPALETTE_UserSize = GetProcAddress(ole, "HPALETTE_UserSize");
    p_HPALETTE_UserUnmarshal = GetProcAddress(ole, "HPALETTE_UserUnmarshal");
    p_PropVariantClear = GetProcAddress(ole, "PropVariantClear");
    p_PropVariantCopy = GetProcAddress(ole, "PropVariantCopy");
    p_StringFromGUID2 = GetProcAddress(ole, "StringFromGUID2");
    p_StringFromIID = GetProcAddress(ole, "StringFromIID");

    return TRUE;
}
