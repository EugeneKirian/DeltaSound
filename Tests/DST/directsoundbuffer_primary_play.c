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

#define WINDOW_NAME "DirectSound Primary Buffer Play"

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b,
    DWORD seconds, LPVOID wave, DWORD wave_length, DWORD priority, DWORD flags) {
    if (a == NULL || b == NULL || wave == NULL || wave_length == 0) {
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

    CopyMemory(a11, wave, al11);
    CopyMemory(a21, wave, al12);

    // Unlock

    ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
    rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    // Play A
    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(a, 0, priority, flags))) {
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(a);
    }

    // Play B
    if (SUCCEEDED(rb = IDirectSoundBuffer_Play(b, 0, priority, flags))) {
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(b);
    }

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

static HRESULT TestPlayBufferStream(LPDIRECTSOUNDBUFFER buff, LPDSBCAPS caps,
    LPVOID wave, DWORD wave_length, DWORD checkpoint, DWORD priority, DWORD flags) {
    if (buff == NULL || wave == NULL || wave_length == 0) {
        DebugBreak(); return E_ABORT;
    }

    UINT iteration = 0;

    DWORD last_play = 0, last_write = 0;
    DWORD total_play = 0, total_write = 0;
    DWORD current_play = 0, current_write = 0;

    LPVOID audio1 = NULL, audio2 = NULL;
    DWORD audio1len = 0, audio2len = 0;

    DWORD status = 0;
    HRESULT hr = IDirectSoundBuffer_GetStatus(buff, &status);

    // And fill the buffer in chunks until the end of the buffer.
    if (SUCCEEDED(hr = IDirectSoundBuffer_Play(buff, 0, priority, flags))) {
        hr = IDirectSoundBuffer_GetStatus(buff, &status);
        while (total_write < wave_length
            && SUCCEEDED(hr = IDirectSoundBuffer_GetCurrentPosition(buff, &current_play, &current_write))) {
            hr = IDirectSoundBuffer_GetStatus(buff, &status);

            if (last_write == current_write) {
                Sleep(1);
                continue;
            }

            const DWORD pd = current_play < last_play
                ? caps->dwBufferBytes - last_play + current_play : current_play - last_play;
            const DWORD wd = current_write < last_write
                ? caps->dwBufferBytes - last_write + current_write : current_write - last_write;

            last_write = current_write;
            last_play = current_play;

            total_play += pd;
            total_write += wd;

            iteration++;

            // If the current write position is larger than the monitored buffer usage
            // then fill the buffer with more data and update the checkpoint position.

            const ptrdiff_t left = wave_length - total_write;

            if (checkpoint < total_write && left > 0) {
                if (FAILED(hr = IDirectSoundBuffer_Lock(buff,
                    current_write, 0, &audio1, &audio1len, &audio2, &audio2len, DSBLOCK_ENTIREBUFFER))) {
                    DebugBreak(); return hr;
                }

                {
                    // TODO
                    // NOTE. Original DirectSound ignores distance between read and write,
                    // and manages to return complete size of the buffer,
                    // including the "unaccessible" region...
                    const DWORD distance = current_play < current_write
                        ? 32768 + current_play - current_write
                        : 32768 + current_write - current_play;

                    if (distance != audio1len + audio2len) {
                        int todo = 1;/// TODO
                    }
                }

                {
                    // TODO test.
                    // Lock with offset between read and write cursors...
                }

                const DWORD written1 = (ptrdiff_t)audio1len < left ? audio1len : left;

                {
                    // Copy the data
                    CopyMemory(audio1, (LPVOID)((size_t)wave + total_write), written1);

                    // TODO need to generate silence
                    ZeroMemory((LPVOID)((size_t)audio1 + written1), audio1len - written1);
                }

                const ptrdiff_t left2 = wave_length - total_write - written1;
                const DWORD written2 = audio2 == NULL
                    ? 0 : ((ptrdiff_t)audio2len < left2 ? audio2len : left2);

                if (audio2 != NULL)
                {
                    // Copy the data
                    CopyMemory(audio2, (LPVOID)((size_t)wave + total_write + written1), written2);

                    // TODO need to generate silence
                    ZeroMemory((LPVOID)((size_t)audio2 + written2), audio2len - written2);
                }

                checkpoint = total_write + (written1 + written2) * 2 / 3;

                if (FAILED(hr = IDirectSoundBuffer_Unlock(buff, audio1, written1, audio2, written2))) {
                    DebugBreak(); return hr;
                }
            }
        }

        // Wait for the buffer to play to the end.
        while (SUCCEEDED(hr = IDirectSoundBuffer_GetCurrentPosition(buff, &current_play, &current_write))) {
            total_play += current_play;
            total_write += current_write;

            iteration++;

            if (wave_length < total_play) {
                break;
            }
        }
    }

    // Stop

    if (FAILED(hr = IDirectSoundBuffer_Stop(buff))) {
        DebugBreak(); return hr;
    }

    hr = IDirectSoundBuffer_GetStatus(buff, &status);

    return hr;
}

static BOOL TestDirectSoundBufferStreamWave(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b,
    LPVOID wave, DWORD wave_length, DWORD priority, DWORD flags) {
    if (a == NULL || b == NULL || wave == NULL || wave_length == 0) {
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

    CopyMemory(a11, wave, al11);
    CopyMemory(a21, wave, al12);

    // Unlock

    ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
    rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    // Play A
    ra = TestPlayBufferStream(a, &capsa, wave, wave_length, al11 * 2 / 3, priority, flags);

    // Play B
    rb = TestPlayBufferStream(b, &capsb, wave, wave_length, al12 * 2 / 3, priority, flags);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
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

#define PLAY_FLAGS_COUNT    39

const static PlayFlags[PLAY_FLAGS_COUNT] = {
    0,
    DSBPLAY_LOOPING,

    DSBPLAY_LOCHARDWARE,
    DSBPLAY_LOCSOFTWARE,
    DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE,

    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE,
    DSBPLAY_LOOPING | DSBPLAY_LOCSOFTWARE,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE,

    DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_TERMINATEBY_PRIORITY,

    DSBPLAY_LOOPING | DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_LOOPING | DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_LOOPING | DSBPLAY_TERMINATEBY_PRIORITY,
    DSBPLAY_LOOPING | DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY,

    DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_PRIORITY,
    DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY,

    DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_PRIORITY,
    DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY,

    DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_PRIORITY,
    DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY,

    DSBPLAY_LOOPING | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_LOOPING | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_LOOPING | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_PRIORITY,
    DSBPLAY_LOOPING | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY,

    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_PRIORITY,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY,

    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_DISTANCE,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_PRIORITY,
    DSBPLAY_LOOPING | DSBPLAY_LOCHARDWARE | DSBPLAY_LOCSOFTWARE | DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_DISTANCE | DSBPLAY_TERMINATEBY_PRIORITY,
};

static BOOL TestDirectSoundBufferPrimaryPlaySynthetic(
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

    LPVOID wave = NULL;
    DWORD wave_length = 0;

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
        DSBCAPS capsa;
        ZeroMemory(&capsa, sizeof(DSBCAPS));

        capsa.dwSize = sizeof(DSBCAPS);

        DSBCAPS capsb;
        ZeroMemory(&capsb, sizeof(DSBCAPS));

        capsb.dwSize = sizeof(DSBCAPS);
        ra = IDirectSoundBuffer_GetCaps(dsba, &capsa);
        rb = IDirectSoundBuffer_GetCaps(dsbb, &capsb);

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

    if (!Synthesise(&fa, 440.0f, 3.0f /* seconds */, &wave, &wave_length)) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < PLAY_PRIORITY_COUNT; i++) {
        for (int k = 0; k < PLAY_FLAGS_COUNT; k++) {
            // TODO tests with Volume
            // TODO tests with Pan
            // TODO tests with Frequency
            if (!TestDirectSoundBufferSingleWave(dsba, dsbb, 4, wave, wave_length, PlayPriority[i], PlayFlags[k])) {
                result = FALSE;
                goto exit;
            }
        }
    }

    for (int i = 0; i < PLAY_PRIORITY_COUNT; i++) {
        for (int k = 0; k < PLAY_FLAGS_COUNT; k++) {
            // TODO tests with Volume
            // TODO tests with Pan
            // TODO tests with Frequency
            if (!TestDirectSoundBufferStreamWave(dsba, dsbb, wave, wave_length, PlayPriority[i], PlayFlags[k])) {
                result = FALSE;
                goto exit;
            }
        }
    }

exit:

    if (wave != NULL) {
        free(wave);
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

#define BUFFER_FLAG_COUNT       9

static const DWORD BufferFlags[BUFFER_FLAG_COUNT] =
{
    DSBCAPS_PRIMARYBUFFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCDEFER,
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
            if (!TestDirectSoundBufferPrimaryPlaySynthetic(dsca, wa, dscb, wb,
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
