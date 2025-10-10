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

#define DSCBSTART_NONE              0

#define CAPTURE_DURATION            4
#define CAPTURE_LOOP_COUNT          3

#define BUFFER_FLAG_COUNT           2
#define BUFFER_FORMAT_COUNT         28

const static DWORD BufferFlags[BUFFER_FLAG_COUNT] = {
    0,
    DSCBCAPS_WAVEMAPPED
};

typedef struct WAVEFORMATDESCRIPTION {
    DWORD       nChannels;
    DWORD       nFrequency;
    DWORD       nBits;
} WAVEFORMATDESCRIPTION, * LPWAVEFORMATDESCRIPTION;

typedef const WAVEFORMATDESCRIPTION* LPCWAVEFORMATDESCRIPTION;

const static WAVEFORMATDESCRIPTION Formats[BUFFER_FORMAT_COUNT] = {
    { 1, 11025, 8 },            // WAVE_FORMAT_1M08
    { 2, 11025, 8 },            // WAVE_FORMAT_1S08
    { 1, 11025, 16 },           // WAVE_FORMAT_1M16
    { 2, 11025, 16 },           // WAVE_FORMAT_1S16
    { 1, 22050, 8 },            // WAVE_FORMAT_2M08
    { 2, 22050, 8 },            // WAVE_FORMAT_2S08
    { 1, 22050, 16 },           // WAVE_FORMAT_2M16
    { 2, 22050, 16 },           // WAVE_FORMAT_2S16
    { 1, 44100, 8 },            // WAVE_FORMAT_4M08
    { 2, 44100, 8 },            // WAVE_FORMAT_4S08
    { 1, 44100, 16 },           // WAVE_FORMAT_4M16
    { 2, 44100, 16 },           // WAVE_FORMAT_4S16
    { 1, 48000, 8 },            // WAVE_FORMAT_48M08
    { 2, 48000, 8 },            // WAVE_FORMAT_48S08
    { 1, 48000, 16 },           // WAVE_FORMAT_48M16
    { 2, 48000, 16 },           // WAVE_FORMAT_48S16
    { 1, 96000, 8 },            // WAVE_FORMAT_96M08
    { 2, 96000, 8 },            // WAVE_FORMAT_96S08
    { 1, 96000, 16 },           // WAVE_FORMAT_96M16
    { 2, 96000, 16 },           // WAVE_FORMAT_96S16
    { 1, 128000, 8 },
    { 2, 128000, 8 },
    { 1, 128000, 16 },
    { 2, 128000, 16 },
    { 1, 196000, 8 },
    { 2, 196000, 8 },
    { 1, 196000, 16 },
    { 2, 196000, 16 }
};

static BOOL InitializeCaptureWaveFormat(LPCWAVEFORMATDESCRIPTION pcwfdDesc, LPWAVEFORMATEX pwfxFormat) {
    if (pcwfdDesc == NULL || pwfxFormat == NULL) {
        return FALSE;
    }

    const DWORD bytes = pcwfdDesc->nBits >> 3;

    pwfxFormat->wFormatTag = WAVE_FORMAT_PCM;
    pwfxFormat->nChannels = (WORD)pcwfdDesc->nChannels;
    pwfxFormat->nSamplesPerSec = pcwfdDesc->nFrequency;
    pwfxFormat->nAvgBytesPerSec = bytes * pcwfdDesc->nChannels * pcwfdDesc->nFrequency;
    pwfxFormat->nBlockAlign = (WORD)(bytes * pcwfdDesc->nChannels);
    pwfxFormat->wBitsPerSample = (WORD)pcwfdDesc->nBits;
    pwfxFormat->cbSize = 0;

    return FALSE;
}

static DWORD CalculateBufferSize(DWORD dwChannels, DWORD dwFrequency, DWORD dwDuration, DWORD dwBits) {
    return dwChannels * dwFrequency * dwDuration * (dwBits >> 3);
}

static BOOL TestDirectSoundCaptureBufferCaptureAudio(LPDIRECTSOUNDCAPTUREBUFFER a,
    LPDIRECTSOUNDCAPTUREBUFFER b, DWORD dwDuration, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps

    if (FAILED(CompareDirectSoundCaptureBufferCaps(a, b))) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    HRESULT ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpa, &cwa);
    HRESULT rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundCaptureBuffer_Stop(a);
        IDirectSoundCaptureBuffer_Stop(b);
    }

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Start A

    if (SUCCEEDED(ra = IDirectSoundCaptureBuffer_Start(a, dwFlags))) {
        Sleep(dwDuration * 1000);
        IDirectSoundBuffer_Stop(a);
    }

    // Start B

    if (SUCCEEDED(rb = IDirectSoundCaptureBuffer_Start(b, dwFlags))) {
        Sleep(dwDuration * 1000);
        IDirectSoundBuffer_Stop(b);
    }

    if (ra != rb) {
        return FALSE;
    }

    ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundCaptureBuffer_Stop(a);
        IDirectSoundCaptureBuffer_Stop(b);
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureBufferStartCapture(LPDIRECTSOUNDCAPTURECREATE a,
    LPDIRECTSOUNDCAPTURECREATE b, LPCWAVEFORMATDESCRIPTION pcwfdDesc,
    DWORD dwDuration, DWORD dwStart, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    InitializeCaptureWaveFormat(pcwfdDesc, &format);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, dwFlags,
        CalculateBufferSize(pcwfdDesc->nChannels, pcwfdDesc->nFrequency, dwDuration, pcwfdDesc->nBits), &format);

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));

    DWORD fas = 0, fbs = 0;

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

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    // GetCaps

    if (FAILED(CompareDirectSoundCaptureBufferCaps(dsba, dsbb))) {
        result = FALSE;
        goto exit;
    }

    // GetFormat

    ra = IDirectSoundCaptureBuffer_GetFormat(dsba, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundCaptureBuffer_GetFormat(dsbb, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb || fas != fbs) {
        result = FALSE;
        goto exit;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCaptureBufferCaptureAudio(dsba, dsbb,
        dwStart & DSCBSTART_LOOPING ? (dwDuration * CAPTURE_LOOP_COUNT) : dwDuration, dwStart)) {
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

BOOL TestDirectSoundCaptureBufferStart(HMODULE a, HMODULE b) {
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
            if (!TestDirectSoundCaptureBufferStartCapture(
                dsca, dscb, &Formats[i], CAPTURE_DURATION, DSCBSTART_NONE, BufferFlags[j])) {
                return FALSE;
            }
        }
    }

    for (int i = 0; i < BUFFER_FLAG_COUNT; i++) {
        if (!TestDirectSoundCaptureBufferStartCapture(dsca, dscb,
            &Formats[i], CAPTURE_DURATION, DSCBSTART_LOOPING, BufferFlags[i])) {
            return FALSE;
        }
    }

    return TRUE;
}
