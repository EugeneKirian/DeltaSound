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

#include "directsoundcapture.h"

#define MAX_STORAGE_COUNT           128

typedef struct callback_context {
    UINT    Count;
    GUID*   Items;
} callback_context;

#define MAX_INPUTDEVICE_COUNT  2

static const LPCGUID input_device_tests[MAX_INPUTDEVICE_COUNT] = {
    &DSDEVID_DefaultPlayback,
    &DSDEVID_DefaultVoicePlayback
};

#define MAX_OUTPUTDEVICE_COUNT  2

static const LPCGUID output_device_tests[MAX_OUTPUTDEVICE_COUNT] = {
    &DSDEVID_DefaultCapture,
    &DSDEVID_DefaultVoiceCapture
};

static BOOL CALLBACK EnumerateDeviceCallBackA(LPGUID guid, LPCSTR desc, LPCSTR module, LPVOID ctx) {
    if (guid == NULL) { return TRUE; }

    callback_context* context = (callback_context*)ctx;

    CopyMemory(&context->Items[context->Count], guid, sizeof(GUID));

    context->Count++;

    return context->Count < MAX_STORAGE_COUNT;
}

static BOOL TestDirectSoundCaptureCreateInvalidInputs(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    {
        const HRESULT ra = a(NULL, NULL, NULL);
        const HRESULT rb = b(NULL, NULL, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        const HRESULT ra = a(NULL, NULL, (LPUNKNOWN)a);
        const HRESULT rb = b(NULL, NULL, (LPUNKNOWN)b);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        const HRESULT ra = a(NULL, &dsa, (LPUNKNOWN)a);
        const HRESULT rb = b(NULL, &dsb, (LPUNKNOWN)b);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        const HRESULT ra = a(&CLSID_DirectSoundCapture, &dsa, NULL);
        const HRESULT rb = b(&CLSID_DirectSoundCapture, &dsb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureCreateNullDevice(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        const HRESULT ra = a(NULL, &dsa, NULL);
        const HRESULT rb = b(NULL, &dsb, NULL);

        if (ra != rb) {
            return FALSE;
        }

        if (dsa == NULL || dsb == NULL) {
            return FALSE;
        }

        RELEASE(dsa);
        RELEASE(dsb);
    }

    {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        const HRESULT ra = a(&GUID_NULL, &dsa, NULL);
        const HRESULT rb = b(&GUID_NULL, &dsb, NULL);

        if (ra != rb) {
            return FALSE;
        }

        if (dsa == NULL || dsb == NULL) {
            return FALSE;
        }

        RELEASE(dsa);
        RELEASE(dsb);
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureCreateOutputDevices(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    for (UINT i = 0; i < MAX_OUTPUTDEVICE_COUNT; i++) {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        const HRESULT ra = a(output_device_tests[i], &dsa, NULL);
        const HRESULT rb = b(output_device_tests[i], &dsb, NULL);

        if (ra != rb) {
            return FALSE;
        }

        if (dsa == NULL || dsb == NULL) {
            return FALSE;
        }

        RELEASE(dsa);
        RELEASE(dsb);
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureCreateInputDevices(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    for (UINT i = 0; i < MAX_INPUTDEVICE_COUNT; i++) {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        const HRESULT ra = a(input_device_tests[i], &dsa, NULL);
        const HRESULT rb = b(input_device_tests[i], &dsb, NULL);

        if (ra != rb) {
            return FALSE;
        }

        if (dsa != NULL || dsb != NULL) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL TestDirectSoundCaptureCreate(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    // Invalid inputs
    if (!TestDirectSoundCaptureCreateInvalidInputs(dsca, dscb)) {
        return FALSE;
    }

    // NULL Device
    if (!TestDirectSoundCaptureCreateNullDevice(dsca, dscb)) {
        return FALSE;
    }

    // Predefined output devices
    if (!TestDirectSoundCaptureCreateOutputDevices(dsca, dscb)) {
        return FALSE;
    }

    // Predefined input devices
    if (!TestDirectSoundCaptureCreateInputDevices(dsca, dscb)) {
        return FALSE;
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

    LPDIRECTSOUNDENUMERATEA ea =
        (LPDIRECTSOUNDENUMERATEA)GetProcAddress(a, "DirectSoundCaptureEnumerateA");
    LPDIRECTSOUNDENUMERATEA eb =
        (LPDIRECTSOUNDENUMERATEA)GetProcAddress(b, "DirectSoundCaptureEnumerateA");

    if (ea == NULL || eb == NULL) {
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

        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        const HRESULT ra = dsca(&ca.Items[i], &dsa, NULL);
        const HRESULT rb = dscb(&cb.Items[i], &dsb, NULL);

        if (ra != rb) {
            result = FALSE;
            goto exit;
        }

        if (dsa == NULL || dsb == NULL) {
            result = FALSE;
            goto exit;
        }

        RELEASE(dsa);
        RELEASE(dsb);
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
