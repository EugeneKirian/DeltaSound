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

#include "directsound_duplicate_secondary.h"

#define MAX_SECONDARY_BUFFER_SUCCESS_FLAG_COUNT   17

const static DWORD CreateSecondaryBufferSuccessFlags[MAX_SECONDARY_BUFFER_SUCCESS_FLAG_COUNT] = {
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
    DSBCAPS_TRUEPLAYPOSITION
};

static BOOL TestDirectSoundDuplicateSoundBuffer(LPDIRECTSOUND a, LPDIRECTSOUND b, DWORD flags) {
    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    LPDIRECTSOUNDBUFFER dsbac = NULL;
    LPDIRECTSOUNDBUFFER dsbbc = NULL;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 22050;
    format.nAvgBytesPerSec = 44100;
    format.nBlockAlign = 2;
    format.wBitsPerSample = 8;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;
    desca.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;
    descb.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    descb.lpwfxFormat = &format;

    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    DWORD cpa = 0, cwa = 0, cpb = 0, cwb = 0;

    DWORD fa = 0, fb = 0;
    DWORD va = 0, vb = 0;
    DWORD pa = 0, pb = 0;

    HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
    HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

    if (dsba == NULL || dsbb == NULL) {
        if (flags & DSBCAPS_LOCHARDWARE) {
            goto exit;
        }

        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, dsba, NULL);
    rb = IDirectSound_DuplicateSoundBuffer(b, dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, NULL, NULL);
    rb = IDirectSound_DuplicateSoundBuffer(b, NULL, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, NULL, &dsbac);
    rb = IDirectSound_DuplicateSoundBuffer(b, NULL, &dsbbc);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // SetFrequency

    ra = IDirectSoundBuffer_SetFrequency(dsba, 96000);
    rb = IDirectSoundBuffer_SetFrequency(dsbb, 96000);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // SetPan

    ra = IDirectSoundBuffer_SetPan(dsba, DSBPAN_LEFT);
    rb = IDirectSoundBuffer_SetPan(dsbb, DSBPAN_LEFT);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // SetVolume

    ra = IDirectSoundBuffer_SetVolume(dsba, DSBVOLUME_MIN / 2);
    rb = IDirectSoundBuffer_SetVolume(dsbb, DSBVOLUME_MIN / 2);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // SetCurrentPosition

    ra = IDirectSoundBuffer_SetCurrentPosition(dsba, 10000);
    rb = IDirectSoundBuffer_SetCurrentPosition(dsbb, 10000);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // Duplicate

    ra = IDirectSound_DuplicateSoundBuffer(a, dsba, &dsbac);
    rb = IDirectSound_DuplicateSoundBuffer(b, dsbb, &dsbbc);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (ra != S_OK || rb != S_OK) {
        result = FALSE;
        goto exit;
    }

    // GetCaps

    ra = IDirectSoundBuffer_GetCaps(dsbac, &capsa);
    rb = IDirectSoundBuffer_GetCaps(dsbbc, &capsb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        result = FALSE;
        goto exit;
    }

    // GetCurrentPosition

    ra = IDirectSoundBuffer_GetCurrentPosition(dsbac, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(dsbbc, &cpb, &cwa);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (cpa != cpb || cwa != cwa) {
        result = FALSE;
        goto exit;
    }

    // GetFrequency

    ra = IDirectSoundBuffer_GetFrequency(dsbac, &fa);
    rb = IDirectSoundBuffer_GetFrequency(dsbbc, &fb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (pa != pb) {
        result = FALSE;
        goto exit;
    }

    // GetPan

    ra = IDirectSoundBuffer_GetPan(dsbac, &pa);
    rb = IDirectSoundBuffer_GetPan(dsbbc, &pb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (pa != pb) {
        result = FALSE;
        goto exit;
    }

    // GetVolume

    ra = IDirectSoundBuffer_GetVolume(dsbac, &va);
    rb = IDirectSoundBuffer_GetVolume(dsbbc, &vb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (va != vb) {
        result = FALSE;
        goto exit;
    }

exit:

    if (dsbac != NULL) {
        IDirectSoundBuffer_Release(dsbac);
    }

    if (dsbbc != NULL) {
        IDirectSoundBuffer_Release(dsbbc);
    }

    if (dsba != NULL) {
        IDirectSoundBuffer_Release(dsba);
    }

    if (dsbb != NULL) {
        IDirectSoundBuffer_Release(dsbb);
    }

    return result;
}

BOOL TestDirectSoundDuplicateSecondary(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = (LPDIRECTSOUNDCREATE)GetProcAddress(a, "DirectSoundCreate");
    LPDIRECTSOUNDCREATE dscb = (LPDIRECTSOUNDCREATE)GetProcAddress(b, "DirectSoundCreate");

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = dsca(NULL, &dsa, NULL);
    HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    for (int i = 0; i < MAX_SECONDARY_BUFFER_SUCCESS_FLAG_COUNT; i++) {
        if (!TestDirectSoundDuplicateSoundBuffer(dsa, dsb, CreateSecondaryBufferSuccessFlags[i])) {
            result = FALSE;
            goto exit;
        }
    }


    // TODO test while playing, check priority, status, etc...
    // TODO in what state buffer is created if the original is playing?
    // TODO what happens to the flags?
    // TODO todo what happens with notifications?

exit:

    IDirectSound_Release(dsa);
    IDirectSound_Release(dsb);

    return result;
}
