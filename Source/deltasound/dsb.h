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
#include "idsb.h"

#define DSB_LEFT_PAN    (-1.0f)
#define DSB_CENTER_PAN  (0.0f)
#define DSB_RIGHT_PAN   (1.0f)

#define DSB_MIN_VOLUME  (0.0f)
#define DSB_MAX_VOLUME  (1.0f)

#define DSB_MAX_PRIMARY_BUFFER_SIZE 32768

typedef struct ds ds;

typedef struct dsb {
    allocator*      Allocator;
    ds*             Instance;

    // TODO resizeable array
    idsb**          Interfaces;
    LONG            InterfaceCount;

    DSBCAPS         Caps;

    // The write cursor indicates the position
    // at which it is safe to write new data to the buffer.
    // The write cursor always leads the play cursor,
    // typically by about 15 milliseconds' worth of audio data.
    DWORD           CurrentPlayCursor;
    DWORD           CurrentWriteCursor;
    LPBYTE          Buffer;     // TODO allocate. Size if Caps.dwBufferBytes

    LPWAVEFORMATEX  Format;
    FLOAT           Volume;
    FLOAT           Pan;
    DWORD           Frequency; // TODO is this separate from the format frequency?
} dsb;

HRESULT DELTACALL dsb_create(allocator* pAlloc, BOOL bInterface, dsb** ppOut);
VOID DELTACALL dsb_release(dsb* pDSB);

HRESULT DELTACALL dsb_set_flags(dsb* pDSB, DWORD dwFlags);

HRESULT DELTACALL dsb_add_ref(dsb* pDSB, idsb* pIDSB);
HRESULT DELTACALL dsb_remove_ref(dsb* pDSB, idsb* pIDSB);

HRESULT DELTACALL dsb_get_caps(dsb* self, LPDSBCAPS pCaps);
HRESULT DELTACALL dsb_get_current_position(dsb* self,
    LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
HRESULT DELTACALL dsb_get_format(dsb* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
HRESULT DELTACALL dsb_get_volume(dsb* self, PFLOAT pfVolume);
HRESULT DELTACALL dsb_get_pan(dsb* self, PFLOAT pfPan);
HRESULT DELTACALL dsb_get_frequency(dsb* self, LPDWORD pdwFrequency);
// STATUS
HRESULT DELTACALL dsb_initialize(dsb* pDSB, ds* pDS, LPCDSBUFFERDESC pcDesc);

HRESULT DELTACALL dsb_set_format(dsb* self, LPCWAVEFORMATEX pcfxFormat);
HRESULT DELTACALL dsb_set_volume(dsb* self, FLOAT fVolume);
HRESULT DELTACALL dsb_set_pan(dsb* self, FLOAT fPan);
