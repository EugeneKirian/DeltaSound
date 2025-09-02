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

#pragma once

#include "base.h"

typedef struct idsb idsb;

typedef HRESULT(DELTACALL* LPIDSBQUERYINTERFACE)(idsb*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSBADDREF)(idsb*);
typedef ULONG(DELTACALL* LPIDSBRELEASE)(idsb*);

typedef HRESULT(DELTACALL* LPIDSBGETCAPS)(idsb*, LPDSBCAPS pDSBufferCaps);
typedef HRESULT(DELTACALL* LPIDSBGETCURRENTPOSITION)(idsb*, LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
typedef HRESULT(DELTACALL* LPIDSBGETFORMAT)(idsb*, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
typedef HRESULT(DELTACALL* LPIDSBGETVOLUME)(idsb*, LPLONG plVolume);
typedef HRESULT(DELTACALL* LPIDSBGETPAN)(idsb*, LPLONG plPan);
typedef HRESULT(DELTACALL* LPIDSBGETFREQUENCY)(idsb*, LPDWORD pdwFrequency);
typedef HRESULT(DELTACALL* LPIDSBGETSTATUS)(idsb*, LPDWORD pdwStatus);
typedef HRESULT(DELTACALL* LPIDSBINITIALIZE)(idsb*, LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc);
typedef HRESULT(DELTACALL* LPIDSBLOCK)(idsb*, DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
typedef HRESULT(DELTACALL* LPIDSBPLAY)(idsb*, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags);
typedef HRESULT(DELTACALL* LPIDSBSETCURRENTPOSITION)(idsb*, DWORD dwNewPosition);
typedef HRESULT(DELTACALL* LPIDSBSETFORMAT)(idsb*, LPCWAVEFORMATEX pcfxFormat);
typedef HRESULT(DELTACALL* LPIDSBSETVOLUME)(idsb*, LONG lVolume);
typedef HRESULT(DELTACALL* LPIDSBSETPAN)(idsb*, LONG lPan);
typedef HRESULT(DELTACALL* LPIDSBSETFREQUENCY)(idsb*, DWORD dwFrequency);
typedef HRESULT(DELTACALL* LPIDSBSTOP)(idsb*);
typedef HRESULT(DELTACALL* LPIDSBUNLOCK)(idsb*, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
typedef HRESULT(DELTACALL* LPIDSBRESTORE)(idsb*);

typedef struct idsb_vft idsb_vft;

struct idsb {
    const idsb_vft* Self;
};

// DSBUFFERDESC1
typedef struct dsb_desc_min {
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
} dsb_desc_min;

// DSBUFFERDESC
typedef struct dsb_desc_max {
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
    GUID            guid3DAlgorithm;
} dsb_desc_max;

HRESULT DELTACALL idsb_create(idsb* pIDSB);
