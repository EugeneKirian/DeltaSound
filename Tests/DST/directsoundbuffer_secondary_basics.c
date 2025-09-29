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

#include "directsoundbuffer_secondary_basics.h"

typedef IReferenceClock* LPREFERENCECLOCK;

static BOOL TestDirectSoundBufferAddRef(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    ULONG ac = IDirectSoundBuffer_AddRef(a);
    ULONG bc = IDirectSoundBuffer_AddRef(b);

    if (ac != bc) {
        return FALSE;
    }

    ac = IDirectSoundBuffer_AddRef(a);
    bc = IDirectSoundBuffer_AddRef(b);

    if (ac != bc) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferRelease(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    ULONG ac = IDirectSoundBuffer_Release(a);
    ULONG bc = IDirectSoundBuffer_Release(b);

    if (ac != bc) {
        return FALSE;
    }

    ac = IDirectSoundBuffer_Release(a);
    bc = IDirectSoundBuffer_Release(b);

    if (ac != bc) {
        return FALSE;
    }

    return TRUE;
}

BOOL TestDirectSoundBufferSecondaryBasics(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = dsca(NULL, &dsa, NULL);
    HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.nSamplesPerSec = 22050;
    format.nAvgBytesPerSec = 22050;
    format.nBlockAlign = 1;
    format.wBitsPerSample = 8;

    DSBUFFERDESC desc;
    ZeroMemory(&desc, sizeof(DSBUFFERDESC));

    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwBufferBytes = 176400;
    desc.lpwfxFormat = &format;

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // AddRef
    if (!TestDirectSoundBufferAddRef(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

    // Release
    if (!TestDirectSoundBufferRelease(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

exit:
    IDirectSoundBuffer_Release(dsba);
    IDirectSoundBuffer_Release(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
