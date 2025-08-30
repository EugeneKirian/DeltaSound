/*
MIT License

Copyright (c) 2025 Eugene Kirian

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "getdeviceid.h"

#include <dsound.h>

typedef HRESULT(WINAPI* LPGETDEVICEID)(LPCGUID pGuidSrc, LPGUID pGuidDest);

typedef BOOL(CALLBACK* LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef HRESULT(WINAPI* LPDIRECTSOUNDENUMERATEA)(LPDSENUMCALLBACKA, LPVOID);

#define MAX_STORAGE_COUNT           128

typedef struct callback_context {
    UINT    Count;
    GUID*   Items;
} callback_context;

static BOOL CALLBACK EnumerateDeviceCallBackA(LPGUID guid, LPCSTR desc, LPCSTR module, LPVOID ctx) {
    if (guid == NULL) { return TRUE; }

    callback_context* context = (callback_context*)ctx;

    CopyMemory(&context->Items[context->Count], guid, sizeof(GUID));

    context->Count++;

    return context->Count < MAX_STORAGE_COUNT;
}

#define MAX_GETDEVICEID_TEST_COUNT  5

static const LPCGUID get_device_id_tests[MAX_GETDEVICEID_TEST_COUNT] = {
    &GUID_NULL,
    &DSDEVID_DefaultPlayback,
    &DSDEVID_DefaultCapture,
    &DSDEVID_DefaultVoicePlayback,
    &DSDEVID_DefaultVoiceCapture
};

static BOOL CompareResults(LPCGUID a, LPCGUID b) {
    if (a != NULL && b != NULL) {
        return IsEqualGUID(a, b);
    }

    return FALSE;
}

BOOL TestGetDeviceID(HMODULE a, HMODULE b) {
    LPGETDEVICEID ga = (LPGETDEVICEID)GetProcAddress(a, "GetDeviceID");
    LPGETDEVICEID gb = (LPGETDEVICEID)GetProcAddress(b, "GetDeviceID");

    if (ga == NULL || gb == NULL) {
        return FALSE;
    }

    // NULL
    {
        GUID ra, rb;
        ZeroMemory(&ra, sizeof(GUID));
        ZeroMemory(&rb, sizeof(GUID));

        HRESULT ha = ga(NULL, &ra);
        HRESULT hb = gb(NULL, &rb);

        if (ha != hb || !CompareResults(&ra, &rb)) {
            return FALSE;
        }
    }

    // Predefined values
    for (UINT i = 0; i < MAX_GETDEVICEID_TEST_COUNT; i++) {
        GUID ra, rb;
        ZeroMemory(&ra, sizeof(GUID));
        ZeroMemory(&rb, sizeof(GUID));

        HRESULT ha = ga(get_device_id_tests[i], &ra);
        HRESULT hb = gb(get_device_id_tests[i], &rb);

        if (ha != hb || !CompareResults(&ra, &rb)) {
            return FALSE;
        }
    }

    // Enumerate devices...
    BOOL result = TRUE;

    callback_context ca, cb;
    ZeroMemory(&ca, sizeof(callback_context));
    ZeroMemory(&cb, sizeof(callback_context));

    const size_t size = sizeof(GUID) * MAX_STORAGE_COUNT;

    if ((ca.Items = (LPGUID)malloc(size)) == NULL) {
        result = FALSE;
        goto exit;
    }

    if ((cb.Items = (LPGUID)malloc(size)) == NULL) {
        result = FALSE;
        goto exit;
    }

    ZeroMemory(ca.Items, size);
    ZeroMemory(cb.Items, size);

    LPDIRECTSOUNDENUMERATEA ea = (LPDIRECTSOUNDENUMERATEA)GetProcAddress(a, "DirectSoundEnumerateA");
    LPDIRECTSOUNDENUMERATEA eb = (LPDIRECTSOUNDENUMERATEA)GetProcAddress(b, "DirectSoundEnumerateA");

    if (ga == NULL || gb == NULL) {
        return FALSE;
    }

    if (ea(EnumerateDeviceCallBackA, &ca) != eb(EnumerateDeviceCallBackA, &cb)) {
        result = FALSE;
        goto exit;
    }


    if (ca.Count != cb.Count) {
        result = FALSE;
        goto exit;
    }

    for (UINT i = 0; i < ca.Count; i++) {
        if (!IsEqualGUID(&ca.Items[i], &cb.Items[i])) {
            result = FALSE;
            goto exit;
        }

        GUID ra, rb;
        ZeroMemory(&ra, sizeof(GUID));
        ZeroMemory(&rb, sizeof(GUID));

        HRESULT ha = ga(&ca.Items[i], &ra);
        HRESULT hb = gb(&cb.Items[i], &rb);

        if (ha != hb || !CompareResults(&ra, &rb)) {
            result = FALSE;
            goto exit;
        }
    }

exit:
    if (ca.Items != NULL) {
        free(ca.Items);
    }

    if (cb.Items != NULL) {
        free(cb.Items);
    }

    return result;
}
