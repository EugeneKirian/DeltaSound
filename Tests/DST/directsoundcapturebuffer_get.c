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

#define BUFFER_FLAG_COUNT       2
#define BUFFER_FORMAT_COUNT     4

const static DWORD BufferFlags[BUFFER_FLAG_COUNT] = {
    0,
    DSCBCAPS_WAVEMAPPED
};

const static WAVEFORMATEX Formats[BUFFER_FORMAT_COUNT] = {
    { WAVE_FORMAT_PCM, 1, 22050, 22050, 1, 8, 0 },
    { WAVE_FORMAT_PCM, 2, 44100, 176400, 4, 16, 0 },
    { WAVE_FORMAT_PCM, 1, 96000, 192000, 2, 16, 0 },
    { WAVE_FORMAT_PCM, 2, 128000, 512000, 4, 16, 0 }
};

static BOOL TestDirectSoundCaptureBufferGetProperties(LPDIRECTSOUNDCAPTUREBUFFER a, LPDIRECTSOUNDCAPTUREBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundCaptureBufferCaps(a, b))) {
        return FALSE;
    }

    // GetCurrentPosition
    {
        DWORD cpca = 0, cwca = 0, cpcb = 0, cwcb = 0;

        HRESULT ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, NULL, NULL);
        HRESULT rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, NULL, NULL);

        if (ra != rb || cpca != cpcb || cwca != cwcb) {
            return FALSE;
        }

        ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpca, &cwca);
        rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpcb, &cwcb);

        if (ra != rb || cpca != cpcb || cwca != cwcb) {
            return FALSE;
        }
    }

    // GetFormat
    {
        const size_t size = 2 * sizeof(WAVEFORMATEX);
        LPWAVEFORMATEX wf1 = malloc(size);

        if (wf1 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf1, size);

        LPWAVEFORMATEX wf2 = malloc(size);

        if (wf2 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf2, size);

        HRESULT ra = IDirectSoundCaptureBuffer_GetFormat(a, NULL, 17, NULL);
        HRESULT rb = IDirectSoundCaptureBuffer_GetFormat(b, NULL, 17, NULL);

        if (ra != rb) {
            return FALSE;
        }

        DWORD sa = 0, sb = 0;

        ra = IDirectSoundCaptureBuffer_GetFormat(a, NULL, 17, &sa);
        rb = IDirectSoundCaptureBuffer_GetFormat(b, NULL, 17, &sb);

        if (ra != rb && sa != sb) {
            return FALSE;
        }

        ra = IDirectSoundCaptureBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), &sa);
        rb = IDirectSoundCaptureBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), &sb);

        if (ra != rb || sa != sb) {
            return FALSE;
        }

        if (memcmp(wf1, wf2, sa) != 0) {
            return FALSE;
        }

        sa = 0, sb = 0;

        ra = IDirectSoundCaptureBuffer_GetFormat(a, NULL, size, &sa);
        rb = IDirectSoundCaptureBuffer_GetFormat(b, NULL, size, &sb);

        if (ra != rb || sa != sb) {
            return FALSE;
        }

        if (memcmp(wf1, wf2, sa) != 0) {
            return FALSE;
        }

        ZeroMemory(wf1, size);
        ZeroMemory(wf2, size);

        ra = IDirectSoundCaptureBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX) - 1, &sa);
        rb = IDirectSoundCaptureBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX) - 1, &sb);

        if (ra != rb || sa != sb) {
            return FALSE;
        }

        if (memcmp(wf1, wf2, sa) != 0) {
            return FALSE;
        }

        free(wf1);
        free(wf2);
    }

    // GetStatus
    {
        DWORD sa = 0, sb = 0;

        HRESULT ra = IDirectSoundCaptureBuffer_GetStatus(a, NULL);
        HRESULT rb = IDirectSoundCaptureBuffer_GetStatus(b, NULL);

        if (ra != rb || sa != sb) {
            return FALSE;
        }

        ra = IDirectSoundCaptureBuffer_GetStatus(a, &sa);
        rb = IDirectSoundCaptureBuffer_GetStatus(b, &sb);

        if (ra != rb || sa != sb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureBufferGetDetails(
    LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b, LPCWAVEFORMATEX pcwfxFormat, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, dwFlags, 176400, pcwfxFormat);

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    ra = IDirectSoundCapture_CreateCaptureBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSoundCapture_CreateCaptureBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL && dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCaptureBufferGetProperties(dsba, dsbb)) {
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

BOOL TestDirectSoundCaptureBufferGet(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    for (int i = 0; i < BUFFER_FORMAT_COUNT; i++) {
        for (int j = 0; j < BUFFER_FLAG_COUNT; j++) {
            if (!TestDirectSoundCaptureBufferGetDetails(dsca, dscb, &Formats[i], BufferFlags[j])) {
                return FALSE;
            }
        }
    }

    return TRUE;
}
