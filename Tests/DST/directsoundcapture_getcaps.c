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

static BOOL TestDirectSoundCaptureGetCapsInvalidInputs(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    {
        const HRESULT ra = IDirectSoundCapture_GetCaps(a, NULL);
        const HRESULT rb = IDirectSoundCapture_GetCaps(b, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCCAPS ca, cb;
        ZeroMemory(&ca, sizeof(DSCCAPS));
        ZeroMemory(&cb, sizeof(DSCCAPS));

        HRESULT ra = IDirectSoundCapture_GetCaps(a, &ca);
        HRESULT rb = IDirectSoundCapture_GetCaps(b, &cb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCCAPS ca;
        ZeroMemory(&ca, sizeof(DSCCAPS));

        ca.dwSize = UINT_MAX;

        DSCCAPS cb;
        ZeroMemory(&cb, sizeof(DSCCAPS));

        cb.dwSize = UINT_MAX;

        const HRESULT ra = IDirectSoundCapture_GetCaps(a, &ca);
        const HRESULT rb = IDirectSoundCapture_GetCaps(b, &cb);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureGetCapsValidInputs(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    DSCCAPS ca;
    ZeroMemory(&ca, sizeof(DSCCAPS));

    ca.dwSize = sizeof(DSCCAPS);

    DSCCAPS cb;
    ZeroMemory(&cb, sizeof(DSCCAPS));

    cb.dwSize = sizeof(DSCCAPS);

    const HRESULT ra = IDirectSoundCapture_GetCaps(a, &ca);
    const HRESULT rb = IDirectSoundCapture_GetCaps(b, &cb);

    if (ra != rb) {
        return FALSE;
    }

    return memcmp(&ca, &cb, sizeof(DSCCAPS)) == 0;
}

BOOL TestDirectSoundCaptureGetCaps(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

    const HRESULT ra = dsca(NULL, &dsa, NULL);
    const HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    if (!TestDirectSoundCaptureGetCapsInvalidInputs(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCaptureGetCapsValidInputs(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
