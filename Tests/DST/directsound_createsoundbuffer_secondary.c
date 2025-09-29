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

#include "directsound_createsoundbuffer_secondary.h"

#define MAX_SECONDARY_BUFFER_SUCCESS_FLAG_COUNT 17

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

#define MAX_SECONDARY_BUFFER_INVALID_FLAG_COUNT 6

static const DWORD CreateSecondaryBufferInvalidFlags[MAX_SECONDARY_BUFFER_INVALID_FLAG_COUNT] = {
    DSBCAPS_LOCHARDWARE,
    DSBCAPS_CTRLFX,
    DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_LOCDEFER,
    DSBCAPS_LOCSOFTWARE | DSBCAPS_LOCDEFER,
    DSBCAPS_LOCHARDWARE | DSBCAPS_LOCDEFER
};

static BOOL TestDirectSoundCreateBufferInvalidInputs(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    {
        HRESULT ra = IDirectSound_CreateSoundBuffer(a, NULL, NULL, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, NULL, NULL, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, NULL, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, NULL, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, (LPUNKNOWN)&desca);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, (LPUNKNOWN)&descb);

        if (ra != rb) {
            return FALSE;
        }

        if (dsba != NULL || dsbb != NULL) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCreateBufferSecondaryInvalidDesc(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    // dwSize

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwFlags

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1) - 1;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1) - 1;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwBufferBytes

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwBufferBytes = 1;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwBufferBytes = 1;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwBufferBytes = DSBSIZE_MAX + 1;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwBufferBytes = DSBSIZE_MAX + 1;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwReserved

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwReserved = 1;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwReserved = 1;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // lpwfxFormat

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.lpwfxFormat = &format;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // guid3DAlgorithm
    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_CTRL3D;
        desca.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_CTRL3D;
        descb.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_CTRL3D;
        desca.guid3DAlgorithm = DS3DALG_HRTF_FULL;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_CTRL3D;
        descb.guid3DAlgorithm = DS3DALG_HRTF_FULL;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_CTRL3D;
        desca.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_CTRL3D;
        descb.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCreateBufferSecondaryInvalidFlags(LPDIRECTSOUND a, LPDIRECTSOUND b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 22050;
    format.nAvgBytesPerSec = 44100;
    format.nBlockAlign = 2;
    format.wBitsPerSample = 8;

    DSBUFFERDESC1 desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

    desca.dwSize = sizeof(DSBUFFERDESC1);
    desca.dwFlags = dwFlags;
    desca.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC1 descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

    descb.dwSize = sizeof(DSBUFFERDESC1);
    descb.dwFlags = dwFlags;
    descb.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    descb.lpwfxFormat = &format;

    const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
    const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

    BOOL result = TRUE;

    if ((ra != rb || ra == S_OK) && !(dwFlags & DSBCAPS_LOCHARDWARE)) {
        result = FALSE;
    }

    RELEASE(dsba);
    RELEASE(dsbb);

    return result;
}

static BOOL TestDirectSoundCreateBufferSecondaryInvalidFormat(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }
    
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        format.wFormatTag = WAVE_FORMAT_PCM;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = &format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = &format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 1;
        format.nSamplesPerSec = 22050;
        format.nAvgBytesPerSec = 22050 * 8;
        format.nBlockAlign = 1;
        format.wBitsPerSample = 8;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = &format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 1;
        format.nSamplesPerSec = 22050;
        format.nAvgBytesPerSec = 22050;
        format.nBlockAlign = 1;
        format.wBitsPerSample = 7;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = &format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 4;
        format.nSamplesPerSec = 22050;
        format.nAvgBytesPerSec = 22050 * 4;
        format.nBlockAlign = 4;
        format.wBitsPerSample = 8;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = &format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 4;
        format.nSamplesPerSec = 88000;
        format.nAvgBytesPerSec = 352000 * 2;
        format.nBlockAlign = 4;
        format.wBitsPerSample = 16;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = &format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        WAVEFORMATEXTENSIBLE format;
        ZeroMemory(&format, sizeof(WAVEFORMATEXTENSIBLE));

        format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        format.Format.nChannels = 1;
        format.Format.nSamplesPerSec = 22050;
        format.Format.nAvgBytesPerSec = 22050;
        format.Format.nBlockAlign = 1;
        format.Format.wBitsPerSample = 8;
        format.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

        format.dwChannelMask = SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_LEFT;
        format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = (LPWAVEFORMATEX)&format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = (LPWAVEFORMATEX)&format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCreateBufferSecondaryValidFormat(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }
    
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    {
        WAVEFORMATEXTENSIBLE format;
        ZeroMemory(&format, sizeof(WAVEFORMATEXTENSIBLE));

        format.Format.wFormatTag = WAVE_FORMAT_PCM;
        format.Format.nChannels = 2;
        format.Format.nSamplesPerSec = 22050;
        format.Format.nAvgBytesPerSec = 44100;
        format.Format.nBlockAlign = 2;
        format.Format.wBitsPerSample = 8;
        format.Format.cbSize = 22;

        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwBufferBytes = 65536;
        desca.lpwfxFormat = (LPWAVEFORMATEX)&format;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwBufferBytes = 65536;
        descb.lpwfxFormat = (LPWAVEFORMATEX)&format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }

        if (ra == S_OK) {
            DWORD sa = 0, sb = 0;
            WAVEFORMATEXTENSIBLE fa;
            ZeroMemory(&fa, sizeof(WAVEFORMATEXTENSIBLE));

            WAVEFORMATEXTENSIBLE fb;
            ZeroMemory(&fb, sizeof(WAVEFORMATEXTENSIBLE));

            ra = IDirectSoundBuffer_GetFormat(dsba, (LPWAVEFORMATEX)&fa, sizeof(WAVEFORMATEXTENSIBLE), &sa);
            rb = IDirectSoundBuffer_GetFormat(dsba, (LPWAVEFORMATEX)&fb, sizeof(WAVEFORMATEXTENSIBLE), &sb);

            RELEASE(dsba);
            RELEASE(dsbb);
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCreateBufferSecondaryFlags(LPDIRECTSOUND a, LPDIRECTSOUND b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

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
    desca.dwFlags = dwFlags;
    desca.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = dwFlags;
    descb.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    descb.lpwfxFormat = &format;

    const HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
    const HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

    if (ra != rb && !(dwFlags & DSBCAPS_LOCHARDWARE)) {
        goto exit;
    }

    if (dwFlags & DSBCAPS_LOCHARDWARE) {
        goto exit;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (FAILED(CompareDirectSoundBufferCaps(dsba, dsbb))) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsba);
    RELEASE(dsbb);

    return result;
}

static BOOL TestDirectSoundCreateBufferSecondaryMuliple(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }
    
    BOOL result = TRUE;

    LPDIRECTSOUNDBUFFER dsba1 = NULL, dsba2 = NULL;
    LPDIRECTSOUNDBUFFER dsbb1 = NULL, dsbb2 = NULL;

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
    desca.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    descb.lpwfxFormat = &format;

    const HRESULT ra1 = IDirectSound_CreateSoundBuffer(a, &desca, &dsba1, NULL);
    const HRESULT ra2 = IDirectSound_CreateSoundBuffer(a, &desca, &dsba2, NULL);
    const HRESULT rb1 = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb1, NULL);
    const HRESULT rb2 = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb2, NULL);

    if ((ra1 != ra2) || (ra1 != rb1) || (rb1 != rb2)) {
        return FALSE;
    }

    if (dsba1 == NULL || dsba2 == NULL || dsbb1 == NULL || dsbb2 == NULL) {
        result = FALSE;
        goto exit;
    }

    if (dsba1 == dsba2 || dsbb1 == dsbb2) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsba1);
    RELEASE(dsba2);
    RELEASE(dsbb1);
    RELEASE(dsbb2);

    return result;
}

BOOL TestDirectSoundCreateSoundBufferSecondary(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUND dsa = NULL, dsb = NULL;

    const HRESULT ra = dsca(NULL, &dsa, NULL);
    const HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    if (!TestDirectSoundCreateBufferInvalidInputs(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCreateBufferSecondaryInvalidDesc(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < MAX_SECONDARY_BUFFER_INVALID_FLAG_COUNT; i++) {
        if (!TestDirectSoundCreateBufferSecondaryInvalidFlags(dsa, dsb, CreateSecondaryBufferInvalidFlags[i])) {
            result = FALSE;
            goto exit;
        }
    }

    if (!TestDirectSoundCreateBufferSecondaryInvalidFormat(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCreateBufferSecondaryValidFormat(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < MAX_SECONDARY_BUFFER_SUCCESS_FLAG_COUNT; i++) {
        if (!TestDirectSoundCreateBufferSecondaryFlags(dsa, dsb, CreateSecondaryBufferSuccessFlags[i])) {
            result = FALSE;
            goto exit;
        }
    }

    if (!TestDirectSoundCreateBufferSecondaryMuliple(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
