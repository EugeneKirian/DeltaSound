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

#include "base.h"

const CLSID CLSID_DirectSoundPrivate = {
    0x11AB3EC0, 0x25EC, 0x11D1, { 0xA4, 0xD8, 0x00, 0xC0, 0x4F, 0xC2, 0x8A, 0xCA }
};

const IID IID_IDirectSoundPrivate = {
    0xd6E525AE, 0xB125, 0x4EC4, { 0xBE, 0x13, 0x12, 0x6D, 0x0C, 0xF7, 0xAF, 0xB6 }
};

const GUID DSPROPSETID_DirectSoundDevice = {
    0x84624F82, 0x25EC, 0x11D1, { 0xA4, 0xD8, 0x00, 0xC0, 0x4F, 0xC2, 0x8A, 0xCA }
};

const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {
    0x0000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 }
};

LPDIRECTSOUNDCREATE GetDirectSoundCreate(HMODULE module) {
    if (module == NULL) {
        return NULL;
    }

    return (LPDIRECTSOUNDCREATE)GetProcAddress(module, "DirectSoundCreate");
}

LPDIRECTSOUNDCAPTURECREATE GetDirectSoundCaptureCreate(HMODULE module) {
    if (module == NULL) {
        return NULL;
    }

    return (LPDIRECTSOUNDCAPTURECREATE)GetProcAddress(module, "DirectSoundCaptureCreate");
}

LPFNGETCLASSOBJECT GetDllGetClassObject(HMODULE module) {
    if (module == NULL) {
        return NULL;
    }

    return (LPFNGETCLASSOBJECT)GetProcAddress(module, "DllGetClassObject");
}

HRESULT InitializeWaveFormat(LPWAVEFORMATEX self, DWORD dwChannels, DWORD dwFrequency, DWORD dwBits) {
    if (self == NULL) {
        return E_POINTER;
    }

    ZeroMemory(self, sizeof(WAVEFORMATEX));

    self->wFormatTag = WAVE_FORMAT_PCM;
    self->nChannels = (WORD)dwChannels;
    self->nSamplesPerSec = dwFrequency;
    self->nAvgBytesPerSec = dwFrequency * dwChannels * (dwBits >> 3);
    self->nBlockAlign = (WORD)(dwChannels * (dwBits >> 3));
    self->wBitsPerSample = (WORD)dwBits;

    return S_OK;
}

HRESULT InitializeDirectSoundBufferDesc(LPDSBUFFERDESC self,
    DWORD dwFlags, DWORD dwBufferSize, LPWAVEFORMATEX pwfxFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    ZeroMemory(self, sizeof(DSBUFFERDESC));

    self->dwSize = sizeof(DSBUFFERDESC);
    self->dwFlags = dwFlags;
    self->dwBufferBytes = dwBufferSize;
    self->lpwfxFormat = pwfxFormat;

    return S_OK;
}

HRESULT InitializeDirectSoundCaptureBufferDesc(LPDSCBUFFERDESC self,
    DWORD dwFlags, DWORD dwBufferSize, LPWAVEFORMATEX pwfxFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    ZeroMemory(self, sizeof(DSCBUFFERDESC));

    self->dwSize = sizeof(DSCBUFFERDESC);
    self->dwFlags = dwFlags;
    self->dwBufferBytes = dwBufferSize;
    self->lpwfxFormat = pwfxFormat;

    return S_OK;
}

HRESULT InitializeDirectSoundCaptureBufferCaps(LPDSCBCAPS self, DWORD dwFlags, DWORD dwBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    ZeroMemory(self, sizeof(DSCBCAPS));

    self->dwSize = sizeof(DSCBCAPS);
    self->dwFlags = dwFlags;
    self->dwBufferBytes = dwBufferBytes;

    return S_OK;
}

HRESULT InitializeDirectSoundBufferCaps(LPDSBCAPS self, DWORD dwFlags, DWORD dwBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    ZeroMemory(self, sizeof(DSBCAPS));

    self->dwSize = sizeof(DSBCAPS);
    self->dwFlags = dwFlags;
    self->dwBufferBytes = dwBufferBytes;

    return S_OK;
}

HRESULT CompareDirectSoundBufferFormat(LPDIRECTSOUNDBUFFER pDSBA, LPDIRECTSOUNDBUFFER pDSBB) {
    if (pDSBA == NULL || pDSBB == NULL) {
        return E_INVALIDARG;
    }

    BYTE fa[128];
    ZeroMemory(fa, 128);

    BYTE fb[128];
    ZeroMemory(fb, 128);

    DWORD fsa = 0, fsb = 0;

    const HRESULT ra = IDirectSoundBuffer_GetFormat(pDSBA, (LPWAVEFORMATEX)fa, 128, &fsa);
    const HRESULT rb = IDirectSoundBuffer_GetFormat(pDSBB, (LPWAVEFORMATEX)fb, 128, &fsb);

    return (ra == rb && fsa == fsb && memcmp(fa, fb, 128) == 0) ? S_OK : E_FAIL;
}

HRESULT CompareDirectSoundBufferCaps(LPDIRECTSOUNDBUFFER pDSBA, LPDIRECTSOUNDBUFFER pDSBB) {
    if (pDSBA == NULL || pDSBB == NULL) {
        return E_INVALIDARG;
    }

    DSBCAPS capsa, capsb;
    InitializeDirectSoundBufferCaps(&capsa, 0, 0);
    InitializeDirectSoundBufferCaps(&capsb, 0, 0);

    const HRESULT ra = IDirectSoundBuffer_GetCaps(pDSBA, &capsa);
    const HRESULT rb = IDirectSoundBuffer_GetCaps(pDSBB, &capsb);

    return (ra == rb && memcmp(&capsa, &capsb, sizeof(DSBCAPS)) == 0) ? S_OK : E_FAIL;
}

HRESULT CompareDirectSoundCaptureBufferCaps(LPDIRECTSOUNDCAPTUREBUFFER pDSBA, LPDIRECTSOUNDCAPTUREBUFFER pDSBB) {
    if (pDSBA == NULL || pDSBB == NULL) {
        return E_INVALIDARG;
    }

    DSCBCAPS capsa, capsb;
    InitializeDirectSoundCaptureBufferCaps(&capsa, 0, 0);
    InitializeDirectSoundCaptureBufferCaps(&capsb, 0, 0);

    const HRESULT ra = IDirectSoundCaptureBuffer_GetCaps(pDSBA, &capsa);
    const HRESULT rb = IDirectSoundCaptureBuffer_GetCaps(pDSBB, &capsb);

    return (ra == rb && memcmp(&capsa, &capsb, sizeof(DSCBCAPS)) == 0) ? S_OK : E_FAIL;
}