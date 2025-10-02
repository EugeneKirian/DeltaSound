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

#include "directsound.h"

#define MAX_CAPTURE_BUFFER_INVALID_FLAG_COUNT   2

const static DWORD CreateCaptureBufferInvalidFlags[MAX_CAPTURE_BUFFER_INVALID_FLAG_COUNT] = {
    0x1,
    0x10
};

#define MAX_CAPTURE_BUFFER_SUCCESS_FLAG_COUNT   2

const static DWORD CreateCaptureBufferSuccessFlags[MAX_CAPTURE_BUFFER_SUCCESS_FLAG_COUNT] = {
    0,
    DSCBCAPS_WAVEMAPPED
};

static BOOL TestDirectSoundCaptureCreateBufferInvalidInputs(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    {
        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, NULL, NULL, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, NULL, NULL, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCBUFFERDESC desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC));

        desc.dwSize = sizeof(DSCBUFFERDESC);

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, &desc, NULL, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, &desc, NULL, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCBUFFERDESC desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC));

        desc.dwSize = sizeof(DSCBUFFERDESC);

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, &desc, &dsba, (LPUNKNOWN)&desc);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, &desc, &dsbb, (LPUNKNOWN)&desc);

        if (ra != rb) {
            return FALSE;
        }

        if (dsba != NULL || dsbb != NULL) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureCreateBufferInvalidDesc(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    // dwSize

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1) - 1;
        desc.dwFlags = DSCBCAPS_WAVEMAPPED;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwFlags

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);
        desc.dwFlags = 3;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCBUFFERDESC desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC));

        desc.dwSize = sizeof(DSCBUFFERDESC);
        desc.dwFlags = 0x100;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, &desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, &desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwBufferBytes

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);
        desc.dwFlags = 0;
        desc.dwBufferBytes = 1;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);
        desc.dwFlags = DSCBCAPS_WAVEMAPPED;
        desc.dwBufferBytes = 1;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);
        desc.dwFlags = DSCBCAPS_WAVEMAPPED;
        desc.dwBufferBytes = 0x7FFFFFFF;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwReserved

    {
        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);
        desc.dwFlags = 0;
        desc.dwReserved = 1;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // lpwfxFormat

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);
        desc.dwFlags = 0;
        desc.lpwfxFormat = &format;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 2;
        format.nSamplesPerSec = 128000;
        format.nAvgBytesPerSec = 128000 * 2 * 2;
        format.nBlockAlign = 2 * 16;
        format.wBitsPerSample = 16;

        DSCBUFFERDESC1 desc;
        ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

        desc.dwSize = sizeof(DSCBUFFERDESC1);
        desc.dwFlags = 0;
        desc.lpwfxFormat = &format;

        const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
        const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureCreateBufferInvalidFlags(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    DSCBUFFERDESC1 desc;
    ZeroMemory(&desc, sizeof(DSCBUFFERDESC1));

    desc.dwSize = sizeof(DSCBUFFERDESC1);
    desc.dwFlags = dwFlags;

    const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, (LPDSCBUFFERDESC)&desc, &dsba, NULL);
    const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, (LPDSCBUFFERDESC)&desc, &dsbb, NULL);

    if (ra != rb || ra == S_OK) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureCreateBuffer(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 1, 48000, 8);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, dwFlags, 32768, &format);

    const HRESULT ra = IDirectSoundCapture_CreateCaptureBuffer(a, &desc, &dsba, NULL);
    const HRESULT rb = IDirectSoundCapture_CreateCaptureBuffer(b, &desc, &dsbb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (FAILED(CompareDirectSoundCaptureBufferCaps(dsba, dsbb))) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsba);
    RELEASE(dsbb);

    return result;
}

static BOOL TestDirectSoundCaptureCreateBufferMuliple(LPDIRECTSOUNDCAPTURE a, LPDIRECTSOUNDCAPTURE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTUREBUFFER dsba1 = NULL, dsba2 = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsbb1 = NULL, dsbb2 = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 2, 96000, 16);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, 0, 96000, &format);

    const HRESULT ra1 = IDirectSoundCapture_CreateCaptureBuffer(a, &desc, &dsba1, NULL);
    const HRESULT ra2 = IDirectSoundCapture_CreateCaptureBuffer(a, &desc, &dsba2, NULL);
    const HRESULT rb1 = IDirectSoundCapture_CreateCaptureBuffer(b, &desc, &dsbb1, NULL);
    const HRESULT rb2 = IDirectSoundCapture_CreateCaptureBuffer(b, &desc, &dsbb2, NULL);

    if (ra1 != rb1 && rb2 != rb2) {
        return FALSE;
    }

    if (ra2 != DSERR_ALLOCATED && rb2 != DSERR_ALLOCATED) {
        result = FALSE;
        goto exit;
    }

    if (dsba1 == NULL || dsbb1 == NULL) {
        result = FALSE;
        goto exit;
    }

    {
        const ULONG rca1 = IDirectSoundCaptureBuffer_AddRef(dsba1);
        IDirectSoundCaptureBuffer_Release(dsba1);

        const ULONG rcb1 = IDirectSoundCaptureBuffer_AddRef(dsbb1);
        IDirectSoundCaptureBuffer_Release(dsbb1);

        if (rca1 != rcb1) {
            result = FALSE;
            goto exit;
        }
    }

    {
        const ULONG rca1 = IDirectSoundCaptureBuffer_Release(dsba1);
        const ULONG rcb1 = IDirectSoundCaptureBuffer_Release(dsbb1);

        if (rca1 != rcb1) {
            result = FALSE;
            goto exit;
        }

        dsba1 = NULL;
        dsbb1 = NULL;
    }

exit:

    RELEASE(dsba1);
    RELEASE(dsba2);
    RELEASE(dsbb1);
    RELEASE(dsbb2);

    return result;
}

BOOL TestDirectSoundCaptureCreateCaptureBuffer(HMODULE a, HMODULE b) {
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

    if (!TestDirectSoundCaptureCreateBufferInvalidInputs(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCaptureCreateBufferInvalidDesc(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < MAX_CAPTURE_BUFFER_INVALID_FLAG_COUNT; i++) {
        if (!TestDirectSoundCaptureCreateBufferInvalidFlags(dsa, dsb, CreateCaptureBufferInvalidFlags[i])) {
            result = FALSE;
            goto exit;
        }
    }

    for (int i = 0; i < MAX_CAPTURE_BUFFER_SUCCESS_FLAG_COUNT; i++) {
        if (!TestDirectSoundCaptureCreateBuffer(dsa, dsb, CreateCaptureBufferSuccessFlags[i])) {
            result = FALSE;
            goto exit;
        }
    }

    if (!TestDirectSoundCaptureCreateBufferMuliple(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
