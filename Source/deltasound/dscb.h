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

#include "idscb.h"
#include "intfc.h"

#define DSCBSTART_NONE      0
#define DSCBSTATUS_NONE     0

typedef struct dsc dsc;

typedef struct dscb {
    allocator*          Allocator;
    GUID                ID;
    dsc*                Instance;
    intfc*              Interfaces;
    // TODO Notifications

    CRITICAL_SECTION    Lock;

    DSCBCAPS            Caps;

    // TODO

    LPWAVEFORMATEX      Format;

    DWORD               Start;
    DWORD               Status;
} dscb;

HRESULT DELTACALL dscb_create(allocator* pAlloc, REFIID riid, dscb** ppOut);
VOID DELTACALL dscb_release(dscb* pDSB);

HRESULT DELTACALL dscb_query_interface(dscb* pDSB, REFIID riid, LPVOID* ppOut);
HRESULT DELTACALL dscb_add_ref(dscb* pDSB, idscb* pIDSCB);
HRESULT DELTACALL dscb_remove_ref(dscb* pDSB, idscb* pIDSCB);

HRESULT DELTACALL dscb_get_caps(dscb* pDSCB, LPDSCBCAPS pCaps);
HRESULT DELTACALL dscb_get_current_position(dscb* pDSCB, LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition);
HRESULT DELTACALL dscb_get_format(dscb* pDSCB, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
HRESULT DELTACALL dscb_get_status(dscb* pDSCB, LPDWORD pdwStatus);
HRESULT DELTACALL dscb_initialize(dscb* pDSCB, dsc* pDSC, LPCDSCBUFFERDESC pcDesc);
HRESULT DELTACALL dscb_lock(dscb* pDSCB, DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
HRESULT DELTACALL dscb_start(dscb* pDSCB, DWORD dwFlags);
HRESULT DELTACALL dscb_stop(dscb* pDSCB);
HRESULT DELTACALL dscb_unlock(dscb* pDSCB, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
