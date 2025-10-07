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

const static DWORD BufferFlags[BUFFER_FLAG_COUNT] = {
    0,
    DSCBCAPS_WAVEMAPPED
};

static BOOL TestDirectSoundCaptureBufferValidLocks(LPDIRECTSOUNDCAPTUREBUFFER a, LPDIRECTSOUNDCAPTUREBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    DSCBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSCBCAPS));
    capsa.dwSize = sizeof(DSCBCAPS);

    DSCBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSCBCAPS));
    capsb.dwSize = sizeof(DSCBCAPS);

    HRESULT ra = IDirectSoundCaptureBuffer_GetCaps(a, &capsa);
    HRESULT rb = IDirectSoundCaptureBuffer_GetCaps(b, &capsb);

    if (ra != rb) {
        return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSCBCAPS)) != 0) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        return FALSE;
    }

    // Lock

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundCaptureBuffer_Lock(a, cwa, 10111, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundCaptureBuffer_Lock(b, cwb, 10111, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    // GetCurrentPosition
    cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        return FALSE;
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundCaptureBuffer_Lock(a, cwa, 10111, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundCaptureBuffer_Lock(b, cwb, 10111, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundCaptureBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundCaptureBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundCaptureBuffer_Lock(a, cwa, capsa.dwBufferBytes, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundCaptureBuffer_Lock(b, cwb, capsb.dwBufferBytes, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundCaptureBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundCaptureBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    // DSCBLOCK_ENTIREBUFFER

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundCaptureBuffer_Lock(a, cwa, 0, &a11, &al11, &a12, &al21, DSCBLOCK_ENTIREBUFFER);
        rb = IDirectSoundCaptureBuffer_Lock(b, cwb, 0, &a21, &al12, &a22, &al22, DSCBLOCK_ENTIREBUFFER);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundCaptureBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundCaptureBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    // GetCurrentPosition

    cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureBufferLockDetails(
    LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b, DWORD dwFlags) {
    if (a == NULL|| b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 1, 22050, 8);

    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, dwFlags, 176400, &format);

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
        goto exit;
    }

    if (dsba == NULL && dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCaptureBufferValidLocks(dsba, dsbb)) {
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

BOOL TestDirectSoundCaptureBufferLock(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    for (int i = 0; i < BUFFER_FLAG_COUNT; i++) {
        if (!TestDirectSoundCaptureBufferLockDetails(dsca, dscb, BufferFlags[i])) {
            return FALSE;
        }
    }

    return TRUE;
}
