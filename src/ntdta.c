/* NTDTA.DLL - the few ntdll functions the Windows XP WIC DLLs import that Windows 98's
 * ntdll lacks (their ntdll import is renamed to NTDTA). Debug output is dropped and
 * NtQuerySystemInformation reports "not implemented". */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>

#define STATUS_NOT_IMPLEMENTED ((LONG)0xC0000002L)

typedef struct _RTL_BITMAP {
    DWORD SizeOfBitMap;
    DWORD *Buffer;
} RTL_BITMAP, *PRTL_BITMAP;

ULONG __cdecl DbgPrintEx(ULONG componentId, ULONG level, const char *format, ...)
{
    (void)componentId;
    (void)level;
    (void)format;
    return 0;
}

ULONG __cdecl DbgPrompt(const char *prompt, char *response, ULONG maximumResponseLength)
{
    (void)prompt;
    if (response && maximumResponseLength) {
        response[0] = 0;
    }
    return 0;
}

void __stdcall DbgBreakPoint(void)
{
}

LONG __stdcall NtQuerySystemInformation(DWORD cls, PVOID info, DWORD len, PDWORD retLen)
{
    (void)cls;
    (void)info;
    (void)len;
    if (retLen) {
        *retLen = 0;
    }
    return STATUS_NOT_IMPLEMENTED;
}

void __stdcall RtlInitializeBitMap(PRTL_BITMAP bitMapHeader, PDWORD bitMapBuffer, DWORD sizeOfBitMap)
{
    if (!bitMapHeader) {
        return;
    }
    bitMapHeader->SizeOfBitMap = sizeOfBitMap;
    bitMapHeader->Buffer = bitMapBuffer;
}

void __stdcall RtlSetBits(PRTL_BITMAP bitMapHeader, DWORD startingIndex, DWORD numberToSet)
{
    DWORD i;

    if (!bitMapHeader || !bitMapHeader->Buffer) {
        return;
    }

    for (i = 0; i < numberToSet; i++) {
        DWORD bit = startingIndex + i;
        if (bit >= bitMapHeader->SizeOfBitMap) {
            break;
        }
        bitMapHeader->Buffer[bit >> 5] |= (1UL << (bit & 31));
    }
}
