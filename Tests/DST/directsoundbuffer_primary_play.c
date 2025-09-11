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

#include "directsoundbuffer_primary_play.h"
#include "synth.h"
#include "wnd.h"

#include <stdio.h>

#define WINDOW_NAME "DirectSound Primary Buffer Play"

static BOOL TestDirectSoundBufferPlayWave(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b,
    LPVOID wavea, DWORD wavelena, LPVOID waveb, DWORD wavelenb, DWORD priority, DWORD flags) {
    if (a == NULL || b == NULL || wavea == NULL || wavelena == 0 || waveb == NULL || wavelenb == 0) {
        DebugBreak(); return FALSE;
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
        DebugBreak(); return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        DebugBreak(); return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        DebugBreak(); return FALSE;
    }

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Lock

    LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
    DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

    ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
    rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
        DebugBreak(); return FALSE;
    }

    if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
        DebugBreak(); return FALSE;
    }

    if (al11 != al12 || al21 != al22) {
        DebugBreak(); return FALSE;
    }

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Copy

    DWORD check_point1 = al11, check_point2 = al12;
    CopyMemory(a11, wavea, check_point1);
    CopyMemory(a21, waveb, check_point2);

    // Unlock

    ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
    rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    // Adjust the position when buffer has to be refilled at 2/3rds of the size.
    check_point1 = check_point1 * 2 / 3;
    check_point2 = check_point2 * 2 / 3;

    UINT ita = 0, itb = 0;

    DWORD lpa = 0, lwa = 0, lpb = 0, lwb = 0;
    DWORD tpa = 0, twa = 0, tpb = 0, twb = 0;

    // Play A
    printf("\r\nBuffer size %8d\r\n", wavelena);

    // And fill the buffer in chunks until the end of the buffer.
    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(a, 0, priority, flags))) {
        while (twa < wavelena
            && SUCCEEDED(ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa))) {

            if (lwa == cwa) {
                Sleep(1);
                continue;
            }

            // TODO move to another function

            const unsigned pd = cpa < lpa
                ? capsa.dwBufferBytes - lpa + cpa : cpa - lpa;
            const unsigned wd = cwa < lwa
                ? capsa.dwBufferBytes - lwa + cwa : cwa - lwa;

            lwa = cwa;
            lpa = cpa;

            tpa += pd;
            twa += wd;

            ita++;

            printf("It %8d, Play %8d, Write %8d, Tot.Play %8d Tot.Write %8d N.Pos %8d\r\n",
                ita, cpa, cwa, tpa, twa, check_point1);

            // If the current write position is larger than the monitored buffer usage
            // then fill the buffer with more data and update the checkpoint position.

            const ptrdiff_t left = wavelena - twa;

            if (check_point1 < twa && left > 0) {
                ra = IDirectSoundBuffer_Lock(a, cwa, 0, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);

                if (ra != S_OK) {
                    DebugBreak(); return FALSE;
                }

                const size_t written1 = al11 < left ? al11 : left;

                {
                    // Copy the data
                    CopyMemory(a11, (LPVOID)((size_t)wavea + twa), written1);

                    // TODO need to generate silence
                    ZeroMemory((LPVOID)((size_t)a11 + written1), al11 - written1);
                }

                const ptrdiff_t left2 = wavelena - twa - written1;
                const size_t written2 = a12 == NULL ? 0 : (al21 < left2 ? al21 : left2);

                if (a12 != NULL)
                {
                    // Copy the data
                    CopyMemory(a12, (LPVOID)((size_t)wavea + twa + written1), written2);

                    // TODO need to generate silence
                    ZeroMemory((LPVOID)((size_t)a12 + written2), al21 - written2);
                }

                printf("\r\n\tTot.Write %8d Left %d8 W1 %8d W2 %8d To %8d\r\n",
                    twa, left, written1, written2, twa + written1 + written2);

                check_point1 = twa + (written1 + written2) * 2 / 3;

                printf("\tL1 %8d L2 %8d W1 %8d W2 %8d N.Pos %8d\r\n\r\n",
                    al11, al21, written1, written2, check_point1);

                ra = IDirectSoundBuffer_Unlock(a, a11, written1, a12, written2);

                if (ra != S_OK) {
                    DebugBreak(); return FALSE;
                }
            }
        }

        printf("\r\n\tPLAY!!\r\n");

        // Wait for the buffer to play to the end.
        while (SUCCEEDED(ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa))) {
            tpa += cpa;
            twa += cwa;

            ita++;

            printf("It %8d, Play %8d, Write %8d, Tot.Play %8d Tot.Write %8d Len %d8\r\n",
                ita, cpa, cwa, tpa, twa, wavelena);

            if (wavelena < tpa) {
                break;
            }
        }
    }

    printf("--------------------------------------\r\n");

    // Play B

    // TODO

    // Stop

    ra = IDirectSoundBuffer_Stop(a);
    rb = IDirectSoundBuffer_Stop(b);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

#define PLAY_PRIORITY_COUNT 4

const static PlayPriority[PLAY_PRIORITY_COUNT] = {
    0,
    1,
    0xFFFFFFFF - 1,
    0xFFFFFFFF
};

#define PLAY_FLAGS_COUNT    8

const static PlayFlags[PLAY_FLAGS_COUNT] = {
    0,
    DSBPLAY_LOOPING,
    DSBPLAY_LOCHARDWARE,
    DSBPLAY_LOCSOFTWARE,
    DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE,
    DSBPLAY_LOOPING | DSBPLAY_LOCSOFTWARE,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE
};

static BOOL TestDirectSoundBufferPrimaryPlayWave(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD flags, DWORD level) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;

    WAVEFORMATEX fa;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));

    WAVEFORMATEX fb;
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));
    DWORD fas = 0, fbs = 0;

    LPVOID wavea = NULL, waveb = NULL;
    DWORD wavelena = 0, wavelenb = 0;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        DebugBreak(); return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, level);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, level);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    // Get Format

    ra = IDirectSoundBuffer_GetFormat(dsba, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundBuffer_GetFormat(dsbb, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb || fas != fbs) {
        result = FALSE;
        goto exit;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
        result = FALSE;
        goto exit;
    }

    // Synthesise Wave

    if (!Synthesise(&fa, 440.0f, 3.0f /* seconds */, &wavea, &wavelena)) {
        result = FALSE;
        goto exit;
    }

    if (!Synthesise(&fb, 440.0f, 3.0f /* seconds */, &waveb, &wavelenb)) {
        result = FALSE;
        goto exit;
    }

    if (wavelena != wavelenb) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < PLAY_PRIORITY_COUNT; i++) {
        for (int k = 0; k < PLAY_FLAGS_COUNT; k++) {
            if (!TestDirectSoundBufferPlayWave(dsba, dsbb,
                wavea, wavelena, waveb, wavelenb, PlayPriority[i], PlayFlags[k])) {
                result = FALSE;
                goto exit;
            }
        }
    }

exit:

    if (wavea != NULL) {
        free(wavea);
    }

    if (waveb != NULL) {
        free(waveb);
    }

    if (dsba != NULL) {
        IDirectSoundBuffer_Release(dsba);
    }

    if (dsbb != NULL) {
        IDirectSoundBuffer_Release(dsbb);
    }

    if (dsa != NULL) {
        IDirectSound_Release(dsa);
    }

    if (dsb != NULL) {
        IDirectSound_Release(dsb);
    }

    return result;
}

#define COOPERATIVE_LEVEL_COUNT 4

static const DWORD CooperativeLevels[COOPERATIVE_LEVEL_COUNT] = {
    DSSCL_NORMAL, DSSCL_PRIORITY, DSSCL_EXCLUSIVE, DSSCL_WRITEPRIMARY
};

#define BUFFER_FLAG_COUNT       7

static const DWORD BufferFlags[BUFFER_FLAG_COUNT] =
{
    DSBCAPS_PRIMARYBUFFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2
};

BOOL TestDirectSoundBufferPrimaryPlay(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        DebugBreak(); return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = (LPDIRECTSOUNDCREATE)GetProcAddress(a, "DirectSoundCreate");
    LPDIRECTSOUNDCREATE dscb = (LPDIRECTSOUNDCREATE)GetProcAddress(b, "DirectSoundCreate");

    if (dsca == NULL || dscb == NULL) {
        DebugBreak(); return FALSE;
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
            if (!TestDirectSoundBufferPrimaryPlayWave(dsca, wa, dscb, wb,
                BufferFlags[k], CooperativeLevels[i])) {
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
