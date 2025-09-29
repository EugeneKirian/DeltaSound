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

#include "directsoundbuffer_primary.h"
#include "synth.h"
#include "wnd.h"

#define WINDOW_NAME "DirectSound Primary Buffer Play"

#define BUFFER_FLAG_COUNT       10

const static DWORD BufferFlags[BUFFER_FLAG_COUNT] =
{
    DSBCAPS_PRIMARYBUFFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCDEFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_TRUEPLAYPOSITION
};

#define PLAY_PRIORITY_COUNT 4

const static DWORD PlayPriority[PLAY_PRIORITY_COUNT] = {
    0,
    1,
    0xFFFFFFFF - 1,
    0xFFFFFFFF
};

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b,
    DWORD seconds, LPVOID wave, DWORD wave_length, DWORD priority, DWORD flags) {
    if (a == NULL || b == NULL || wave == NULL || wave_length == 0) {
        return FALSE;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundBufferCaps(a, b))) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    HRESULT ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    HRESULT rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a);
        IDirectSoundBuffer_Stop(b);
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

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Copy

    if (a11 == NULL || a21 == NULL) {
        return FALSE;
    }

    CopyMemory(a11, wave, al11);
    CopyMemory(a21, wave, al12);

    // Unlock

    ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
    rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

    if (ra != rb) {
        return FALSE;
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
        return FALSE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a);
        IDirectSoundBuffer_Stop(b);
    }

    return TRUE;
}

static HRESULT TestPlayBufferStream(LPDIRECTSOUNDBUFFER buff, LPDSBCAPS caps,
    LPVOID wave, DWORD wave_length, DWORD checkpoint, DWORD priority, DWORD flags) {
    if (buff == NULL || wave == NULL || wave_length == 0) {
        return E_ABORT;
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
                    return hr;
                }

                const DWORD written1 = (ptrdiff_t)audio1len < left ? audio1len : left;

                {
                    CopyMemory(audio1, (LPVOID)((size_t)wave + total_write), written1);
                    ZeroMemory((LPVOID)((size_t)audio1 + written1), audio1len - written1);
                }

                const ptrdiff_t left2 = wave_length - total_write - written1;
                const DWORD written2 = audio2 == NULL
                    ? 0 : ((ptrdiff_t)audio2len < left2 ? audio2len : left2);

                if (audio2 != NULL){
                    CopyMemory(audio2, (LPVOID)((size_t)wave + total_write + written1), written2);
                    ZeroMemory((LPVOID)((size_t)audio2 + written2), audio2len - written2);
                }

                checkpoint = total_write + (written1 + written2) * 2 / 3;

                if (FAILED(hr = IDirectSoundBuffer_Unlock(buff, audio1, written1, audio2, written2))) {
                    return hr;
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
        return hr;
    }

    hr = IDirectSoundBuffer_GetStatus(buff, &status);

    return hr;
}

static BOOL TestDirectSoundBufferStreamWave(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b,
    LPVOID wave, DWORD wave_length, DWORD priority, DWORD flags) {
    if (a == NULL || b == NULL || wave == NULL || wave_length == 0) {
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
        IDirectSoundBuffer_Stop(a);
        IDirectSoundBuffer_Stop(b);
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

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Copy

    if (a11 == NULL || a21 == NULL) {
        return FALSE;
    }

    CopyMemory(a11, wave, al11);
    CopyMemory(a21, wave, al12);

    // Unlock

    ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
    rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

    if (ra != rb) {
        return FALSE;
    }

    // Play A
    ra = TestPlayBufferStream(a, &capsa, wave, wave_length, al11 * 2 / 3, priority, flags);

    // Play B
    rb = TestPlayBufferStream(b, &capsb, wave, wave_length, al12 * 2 / 3, priority, flags);

    if (ra != rb) {
        return FALSE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a);
        IDirectSoundBuffer_Stop(b);
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferPrimaryPlaySetFormat(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD dwFlags, DWORD dwLevel) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUND dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    DSBUFFERDESC desc;
    InitializeDirectSoundBufferDesc(&desc, dwFlags, 0, NULL);

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));

    DWORD fas = 0, fbs = 0;

    LPVOID wave = NULL;
    DWORD wave_length = 0;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, dwLevel);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, dwLevel);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundBufferCaps(dsba, dsbb))) {
        result = FALSE;
        goto exit;
    }

    // GetFormat

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
        for (int k = 0; k < BUFFER_PLAY_FLAGS_COUNT; k++) {
            if (!TestDirectSoundBufferSingleWave(dsba, dsbb, 4, wave, wave_length, PlayPriority[i], BufferPlayFlags[k])) {
                result = FALSE;
                goto exit;
            }
        }
    }

    for (int i = 0; i < PLAY_PRIORITY_COUNT; i++) {
        for (int k = 0; k < BUFFER_PLAY_FLAGS_COUNT; k++) {
            if (!TestDirectSoundBufferStreamWave(dsba, dsbb, wave, wave_length, PlayPriority[i], BufferPlayFlags[k])) {
                result = FALSE;
                goto exit;
            }
        }
    }

exit:

    if (wave != NULL) {
        free(wave);
    }

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

static BOOL TestDirectSoundBufferPrimaryLockDuringPlay(LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL, dsb = NULL;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    BOOL result = TRUE;

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_WRITEPRIMARY);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_WRITEPRIMARY);

    if (ra != rb) {
        return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    DSBUFFERDESC desc;
    InitializeDirectSoundBufferDesc(&desc, DSBCAPS_PRIMARYBUFFER, 0, NULL);

    LPVOID wavea = NULL, waveb = NULL;
    DWORD wavea_length = 0, waveb_length = 0;

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));

    DWORD fas = 0, fbs = 0;

    ra = IDirectSoundBuffer_GetFormat(dsba, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundBuffer_GetFormat(dsbb, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (!Synthesise(&fa, 440.0f, 3.0f /* seconds */, &wavea, &wavea_length)) {
        result = FALSE;
        goto exit;
    }

    if (!Synthesise(&fa, 293.66f, 3.0f /* seconds */, &waveb, &waveb_length)) {
        result = FALSE;
        goto exit;
    }

    LPVOID l1a = NULL, l2a = NULL, l1b = NULL, l2b = NULL;
    DWORD l1la = 0, l2la = 0, l1lb = 0, l2lb = 0;

    ra = IDirectSoundBuffer_Lock(dsba, 0, 0, &l1a, &l1la, &l2a, &l2la, DSBLOCK_ENTIREBUFFER);
    rb = IDirectSoundBuffer_Lock(dsbb, 0, 0, &l1b, &l1lb, &l2b, &l2lb, DSBLOCK_ENTIREBUFFER);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    CopyMemory(l1a, wavea, l1la);
    CopyMemory(l1b, wavea, l1lb);

    ra = IDirectSoundBuffer_Unlock(dsba, l1a, l1la, l2a, l2la);
    rb = IDirectSoundBuffer_Unlock(dsbb, l1b, l1lb, l2b, l2lb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSoundBuffer_Play(dsba, 0, 0, DSBPLAY_LOOPING);
    rb = IDirectSoundBuffer_Play(dsbb, 0, 0, DSBPLAY_LOOPING);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    Sleep(100);

    DWORD cra = 0, cwa = 0, crb = 0, cwb = 0;
    ra = IDirectSoundBuffer_GetCurrentPosition(dsba, &cra, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(dsbb, &crb, &cwb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSoundBuffer_Lock(dsba, cra, 0, &l1a, &l1la, &l2a, &l2la, DSBLOCK_ENTIREBUFFER);
    rb = IDirectSoundBuffer_Lock(dsbb, crb, 0, &l1b, &l1lb, &l2b, &l2lb, DSBLOCK_ENTIREBUFFER);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    CopyMemory(l1a, waveb, l1la);
    CopyMemory(l1b, waveb, l1lb);

    ra = IDirectSoundBuffer_Unlock(dsba, l1a, l1la, l2a, l2la);
    rb = IDirectSoundBuffer_Unlock(dsbb, l1b, l1lb, l2b, l2lb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    Sleep(1 * 1000);

    ra = IDirectSoundBuffer_Stop(dsba);
    rb = IDirectSoundBuffer_Stop(dsbb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

exit:

    if (wavea != NULL) {
        free(wavea);
    }

    if (waveb != NULL) {
        free(waveb);
    }

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDirectSoundBufferPrimaryPlay(HMODULE a, HMODULE b) {
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
            if (!TestDirectSoundBufferPrimaryPlaySetFormat(dsca, wa, dscb, wb,
                BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                goto exit;
            }
        }
    }

    if (!TestDirectSoundBufferPrimaryLockDuringPlay(dsca, wa, dscb, wb)) {
        result = FALSE;
        goto exit;
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
