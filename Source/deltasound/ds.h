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

#include "arr.h"
#include "intfc.h"

typedef struct deltasound deltasound;
typedef struct dsdevice dsdevice;
typedef struct dsb dsb;
typedef struct ids ids;

typedef struct ds {
    allocator*  Allocator;
    GUID        ID;
    deltasound* Instance;
    intfc*      Interfaces;

    // TODO Lock!!

    HWND        HWND;
    DWORD       Level;
    dsdevice*   Device;

    dsb*        Main;       // Primary Buffer
    arr*        Buffers;    // Secondary Buffers
} ds;

HRESULT DELTACALL ds_create(allocator* pAlloc, REFIID riid, ds** ppOut);
VOID DELTACALL ds_release(ds* pDS);

HRESULT DELTACALL ds_query_interface(ds* pDS, REFIID riid, ids** ppOut);
HRESULT DELTACALL ds_add_ref(ds* pDS, ids* pIDS);
HRESULT DELTACALL ds_remove_ref(ds* pDS, ids* pIDS);

HRESULT DELTACALL ds_create_dsb(ds* pDS, REFIID riid, LPCDSBUFFERDESC pcDesc, dsb** ppOut);
HRESULT DELTACALL ds_remove_dsb(ds* pDS, dsb* pDSB);

HRESULT DELTACALL ds_get_caps(ds* pDS, LPDSCAPS pCaps);

HRESULT DELTACALL ds_set_cooperative_level(ds* pDS, HWND hwnd, DWORD dwLevel);

HRESULT DELTACALL ds_initialize(ds* pDS, LPCGUID pcGuidDevice);
