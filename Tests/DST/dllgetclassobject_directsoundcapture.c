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

#include "dllgetclassobject.h"

static BOOL TestDllGetClassObjectDirectSoundCaptureMethods(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    HRESULT ra = S_OK, rb = S_OK;

    // CreateCaptureBuffer
    {
        LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

        WAVEFORMATEX format;
        InitializeWaveFormat(&format, 2, 44100, 16);

        DSCBUFFERDESC desc;
        InitializeDirectSoundCaptureBufferDesc(&desc, 0, 176400, &format);

        ra = IDirectSoundCapture_CreateCaptureBuffer(a, &desc, &dsba, NULL);
        rb = IDirectSoundCapture_CreateCaptureBuffer(b, &desc, &dsbb, NULL);

        if (ra != rb || ra != DSERR_UNINITIALIZED) {
            return FALSE;
        }
    }

    // GetCaps
    {
        DSCCAPS capsa, capsb;
        ZeroMemory(&capsa, sizeof(DSCCAPS));
        ZeroMemory(&capsb, sizeof(DSCCAPS));

        capsa.dwSize = sizeof(DSCCAPS);
        capsb.dwSize = sizeof(DSCCAPS);

        ra = IDirectSoundCapture_GetCaps(a, &capsa);
        rb = IDirectSoundCapture_GetCaps(b, &capsb);

        if (ra != rb || ra != DSERR_UNINITIALIZED) {
            return FALSE;
        }
    }

    // Initialize
    {
        ra = IDirectSoundCapture_Initialize(a, &IID_IDirectSoundCaptureBuffer);
        rb = IDirectSoundCapture_Initialize(b, &IID_IDirectSoundCaptureBuffer);

        if (ra != rb || ra != DSERR_NODRIVER) {
            return FALSE;
        }

        ra = IDirectSoundCapture_Initialize(a, NULL);
        rb = IDirectSoundCapture_Initialize(b, NULL);

        if (ra != rb || ra != DS_OK) {
            return FALSE;
        }

        ra = IDirectSoundCapture_Initialize(a, NULL);
        rb = IDirectSoundCapture_Initialize(b, NULL);

        if (ra != rb || ra != DSERR_ALREADYINITIALIZED) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDllGetClassObjectDirectSoundCaptureInstance(LPCLASSFACTORY a, LPCLASSFACTORY b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

    const HRESULT ra = IClassFactory_CreateInstance(a, NULL, &IID_IDirectSoundCapture, &dsa);
    const HRESULT rb = IClassFactory_CreateInstance(b, NULL, &IID_IDirectSoundCapture, &dsb);

    if (ra != rb || dsa == NULL || dsb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDllGetClassObjectDirectSoundCaptureMethods(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDllGetClassObjectDirectSoundCapture(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPFNGETCLASSOBJECT gcoa = GetDllGetClassObject(a);
    LPFNGETCLASSOBJECT gcob = GetDllGetClassObject(b);

    if (gcoa == NULL || gcob == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    HRESULT ra = S_OK, rb = S_OK;
    LPCLASSFACTORY cfa = NULL, cfb = NULL;

    if (FAILED(ra = gcoa(&CLSID_DirectSoundCapture, &IID_IClassFactory, &cfa))) {
        result = FALSE;
        goto exit;
    }

    if (FAILED(rb = gcob(&CLSID_DirectSoundCapture, &IID_IClassFactory, &cfb))) {
        result = FALSE;
        goto exit;
    }

    if (ra != rb || cfa == NULL || cfb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDllGetClassObjectDirectSoundCaptureInstance(cfa, cfb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(cfa);
    RELEASE(cfb);

    return result;
}
