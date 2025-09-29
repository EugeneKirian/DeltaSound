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

#include "directsoundbuffer_secondary_lock.h"
#include "wnd.h"

#define WINDOW_NAME "DirectSound Secondary Buffer Lock"

#define BUFFER_FLAG_COUNT       18

const static DWORD BufferFlags[BUFFER_FLAG_COUNT] = {
    0,
    DSBCAPS_STATIC,
    DSBCAPS_LOCHARDWARE,
    DSBCAPS_LOCSOFTWARE,
    DSBCAPS_CTRL3D,
    DSBCAPS_CTRLFREQUENCY,
    DSBCAPS_CTRLPAN,
    DSBCAPS_CTRLVOLUME,
    DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
    DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY,
    DSBCAPS_CTRLPOSITIONNOTIFY,
    DSBCAPS_STICKYFOCUS,
    DSBCAPS_GLOBALFOCUS,
    DSBCAPS_STICKYFOCUS | DSBCAPS_GLOBALFOCUS,
    DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_TRUEPLAYPOSITION,
    DSBCAPS_LOCHARDWARE | DSBCAPS_LOCDEFER
};

static BOOL TestDirectSoundBufferValidLocks(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    HRESULT ra = IDirectSoundBuffer_GetCaps(a, &capsa);
    HRESULT rb = IDirectSoundBuffer_GetCaps(b, &capsb);

    if (ra != rb) {
        return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

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

        ra = IDirectSoundBuffer_Lock(a, cwa, 10111, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, cwb, 10111, &a21, &al12, &a22, &al22, 0);

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

    // SetCurrentPosition

    ra = IDirectSoundBuffer_SetCurrentPosition(a, 12000);
    rb = IDirectSoundBuffer_SetCurrentPosition(b, 12000);

    if (ra != rb) {
        return FALSE;
    }

    // GetCurrentPosition
    cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        return FALSE;
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, cwa, 10111, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, cwb, 10111, &a21, &al12, &a22, &al22, 0);

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

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, cwa, capsa.dwBufferBytes, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, cwb, capsb.dwBufferBytes, &a21, &al12, &a22, &al22, 0);

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

    // DSBLOCK_FROMWRITECURSOR

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 28000, &a11, &al11, &a12, &al21, DSBLOCK_FROMWRITECURSOR);
        rb = IDirectSoundBuffer_Lock(b, 0, 28000, &a21, &al12, &a22, &al22, DSBLOCK_FROMWRITECURSOR);

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

    // DSBLOCK_ENTIREBUFFER

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, cwa, 0, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
        rb = IDirectSoundBuffer_Lock(b, cwb, 0, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

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
    
    // DSBLOCK_FROMWRITECURSOR | DSBLOCK_ENTIREBUFFER

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, &al11, &a12, &al21,
            DSBLOCK_FROMWRITECURSOR | DSBLOCK_ENTIREBUFFER);
        rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, &al12, &a22, &al22,
            DSBLOCK_FROMWRITECURSOR | DSBLOCK_ENTIREBUFFER);

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

    // SetCurrentPosition

    ra = IDirectSoundBuffer_SetCurrentPosition(a, 0);
    rb = IDirectSoundBuffer_SetCurrentPosition(b, 0);

    if (ra != rb) {
         return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferSecondaryLockDetails(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD flags, DWORD level) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.nSamplesPerSec = 22050;
    format.nAvgBytesPerSec = 22050;
    format.nBlockAlign = 1;
    format.wBitsPerSample = 8;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;
    desca.dwBufferBytes = 176400;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;
    descb.dwBufferBytes = 176400;
    descb.lpwfxFormat = &format;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, level);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, level);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb, NULL);

    if (ra != rb && !(flags & DSBCAPS_LOCHARDWARE)) {
        result = FALSE;
        goto exit;
    }

    if (flags & DSBCAPS_LOCHARDWARE) {
        goto exit;
    }

    if (dsba == NULL && dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferValidLocks(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

exit:

    if (dsba != NULL) {
        IDirectSoundBuffer_Release(dsba);
    }

    if (dsbb != NULL) {
        IDirectSoundBuffer_Release(dsbb);
    }

    if (dsa != NULL) {
        RELEASE(dsa);
    }

    if (dsb != NULL) {
        RELEASE(dsb);
    }

    return result;
}

BOOL TestDirectSoundBufferSecondaryLock(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    HWND wa = InitWindow(WINDOW_NAME);
    HWND wb = InitWindow(WINDOW_NAME);

    if (wa == NULL || wb == NULL) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < COOPERATIVE_LEVEL_COUNT; i++) {
        for (int k = 0; k < BUFFER_FLAG_COUNT; k++) {
            if (!TestDirectSoundBufferSecondaryLockDetails(dsca, wa, dscb, wb, BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                goto exit;
            }
        }
    }

exit:
    if (wa != NULL) {
        DestroyWindow(wa);
    }

    if (wb != NULL) {
        DestroyWindow(wb);
    }

    UnregisterClassA(WINDOW_NAME, GetModuleHandleA(NULL));

    return result;
}
