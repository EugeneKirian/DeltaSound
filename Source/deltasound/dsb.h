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

#include "dsbcb.h"
#include "idsb.h"
#include "intfc.h"

#define DSB_LEFT_PAN    (-1.0f)
#define DSB_CENTER_PAN  (0.0f)
#define DSB_RIGHT_PAN   (1.0f)

#define DSB_MIN_VOLUME  (0.0f)
#define DSB_MAX_VOLUME  (1.0f)

#define DSB_DEFAULT_PRIMARY_BUFFER_SIZE     32768

typedef struct ds ds;
typedef struct ksp ksp;
typedef struct dsn dsn;
typedef struct dssl dssl;
typedef struct dssb dssb;

typedef struct dsb {
    allocator*      Allocator;
    GUID            ID;
    ds*             Instance;
    intfc*          Interfaces;
    ksp*            PropertySet;
    dssl*           SpatialListener;
    dssb*           SpatialBuffer;
    dsn*            Notifications;

    // TODO Lock

    DSBCAPS         Caps;
    DWORD           Priority;
    dsbcb*          Buffer;

    LPWAVEFORMATEX  Format;
    FLOAT           Volume;
    FLOAT           Pan;
    DWORD           Frequency;
    DWORD           Play;
    DWORD           Status;
} dsb;

HRESULT DELTACALL dsb_create(allocator* pAlloc, REFIID riid, dsb** ppOut);
VOID DELTACALL dsb_release(dsb* pDSB);

HRESULT DELTACALL dsb_set_flags(dsb* pDSB, DWORD dwFlags);

HRESULT DELTACALL dsb_query_interface(dsb* pDSB, REFIID riid, LPVOID* ppOut);
HRESULT DELTACALL dsb_add_ref(dsb* pDSB, idsb* pIDSB);
HRESULT DELTACALL dsb_remove_ref(dsb* pDSB, idsb* pIDSB);

HRESULT DELTACALL dsb_get_caps(dsb* pDSB, LPDSBCAPS pCaps);
HRESULT DELTACALL dsb_get_current_position(dsb* pDSB,
    LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
HRESULT DELTACALL dsb_get_format(dsb* pDSB,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
HRESULT DELTACALL dsb_get_volume(dsb* pDSB, PFLOAT pfVolume);
HRESULT DELTACALL dsb_get_pan(dsb* pDSB, PFLOAT pfPan);
HRESULT DELTACALL dsb_get_frequency(dsb* pDSB, LPDWORD pdwFrequency);
HRESULT DELTACALL dsb_get_status(dsb* pDSB, LPDWORD pdwStatus);
HRESULT DELTACALL dsb_initialize(dsb* pDSB, ds* pDS, LPCDSBUFFERDESC pcDesc);
HRESULT DELTACALL dsb_lock(dsb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1,
    LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
HRESULT DELTACALL dsb_play(dsb* self, DWORD dwPriority, DWORD dwFlags);
HRESULT DELTACALL dsb_set_current_position(dsb* pDSB, DWORD dwNewPosition);
HRESULT DELTACALL dsb_set_format(dsb* pDSB, LPCWAVEFORMATEX pcfxFormat);
HRESULT DELTACALL dsb_set_volume(dsb* pDSB, FLOAT fVolume);
HRESULT DELTACALL dsb_set_pan(dsb* pDSB, FLOAT fPan);
HRESULT DELTACALL dsb_set_frequency(dsb* pDSB, DWORD dwFrequency);
HRESULT DELTACALL dsb_stop(dsb* pDSB);
HRESULT DELTACALL dsb_unlock(dsb* self, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
HRESULT DELTACALL dsb_restore(dsb* pDSB);
