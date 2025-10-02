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

#include "allocator.h"

typedef struct dsc dsc;
typedef struct dscb dscb;
typedef struct idscb_vft idscb_vft;

typedef struct idscb {
    const idscb_vft*    Self;
    allocator*          Allocator;
    IID                 ID;
    LONG                RefCount;
    dscb*               Instance;
} idscb;

typedef HRESULT(DELTACALL* LPIDSCBQUERYINTERFACE)(idscb*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSCBADDREF)(idscb*);
typedef ULONG(DELTACALL* LPIDSCBRELEASE)(idscb*);

typedef HRESULT(DELTACALL* LPIDSCBGETCAPS)(idscb*, LPDSCBCAPS pDSCBCaps);
typedef HRESULT(DELTACALL* LPIDSCBGETCURRENTPOSITION)(idscb*, LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition);
typedef HRESULT(DELTACALL* LPIDSCBGETFORMAT)(idscb*, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
typedef HRESULT(DELTACALL* LPIDSCBGETSTATUS)(idscb*, LPDWORD pdwStatus);
typedef HRESULT(DELTACALL* LPIDSCBINITIALIZE)(idscb*, dsc* pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc);
typedef HRESULT(DELTACALL* LPIDSCBLOCK)(idscb*, DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
typedef HRESULT(DELTACALL* LPIDSCBSTART)(idscb*, DWORD dwFlags);
typedef HRESULT(DELTACALL* LPIDSCBSTOP)(idscb*);
typedef HRESULT(DELTACALL* LPIDSCBUNLOCK)(idscb*, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);

// DSCBUFFERDESC
typedef struct dscb_desc_min {
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
} dscb_desc_min;

// DSCBUFFERDESC
typedef struct dscb_desc_max {
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    LPWAVEFORMATEX  lpwfxFormat;
    DWORD           dwFXCount;
    LPDSCEFFECTDESC lpDSCFXDesc;
} dscb_desc_max;

HRESULT DELTACALL idscb_create(allocator* pAlloc, REFIID riid, idscb** ppOut);
VOID DELTACALL idscb_release(idscb* pIDSCB);

HRESULT DELTACALL idscb_query_interface(idscb* pIDSCB, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL idscb_add_ref(idscb* pIDSCB);
ULONG DELTACALL idscb_remove_ref(idscb* pIDSCB);
