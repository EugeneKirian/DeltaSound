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

#include "directsoundcapturebuffer.h"



static BOOL TestDirectSoundCaptureBufferAddRef(LPDIRECTSOUNDCAPTUREBUFFER a, LPDIRECTSOUNDCAPTUREBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    ULONG ac = IDirectSoundCaptureBuffer_AddRef(a);
    ULONG bc = IDirectSoundCaptureBuffer_AddRef(b);

    if (ac != bc) {
        return FALSE;
    }

    ac = IDirectSoundCaptureBuffer_AddRef(a);
    bc = IDirectSoundCaptureBuffer_AddRef(b);

    if (ac != bc) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureBufferRelease(LPDIRECTSOUNDCAPTUREBUFFER a, LPDIRECTSOUNDCAPTUREBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    ULONG ac = IDirectSoundCaptureBuffer_Release(a);
    ULONG bc = IDirectSoundCaptureBuffer_Release(b);

    if (ac != bc) {
        return FALSE;
    }

    ac = IDirectSoundCaptureBuffer_Release(a);
    bc = IDirectSoundCaptureBuffer_Release(b);

    if (ac != bc) {
        return FALSE;
    }

    return TRUE;
}

BOOL TestDirectSoundCaptureBufferBasics(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

    HRESULT ra = dsca(NULL, &dsa, NULL);
    HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 1, 22050, 8);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, 0, 1764000, &format);

    ra = IDirectSoundCapture_CreateCaptureBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSoundCapture_CreateCaptureBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // AddRef
    if (!TestDirectSoundCaptureBufferAddRef(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

    // Release
    if (!TestDirectSoundCaptureBufferRelease(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
