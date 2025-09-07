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

typedef struct deltasound deltasound;
typedef struct device device;
typedef struct dsb dsb;
typedef struct ids ids;

typedef struct ds {
    allocator*  Allocator;
    deltasound* Instance;

    // TODO resizeable array
    ids**       Interfaces;
    LONG        InterfaceCount;

    HWND        HWND;
    DWORD       Level;
    device*     Device;
    dsb*        Main;
} ds;

HRESULT DELTACALL ds_create(allocator* pAlloc, REFIID riid, ds** ppOut);
VOID DELTACALL ds_release(ds* pDS);

HRESULT DELTACALL ds_add_ref(ds* pDS, ids* pIDS);
HRESULT DELTACALL ds_remove_ref(ds* pDS, ids* pIDS);

HRESULT DELTACALL ds_create_dsb(ds* pDS, LPCDSBUFFERDESC pcDesc, dsb** ppOut);

HRESULT DELTACALL ds_get_caps(ds* pDS, LPDSCAPS pCaps);

HRESULT DELTACALL ds_set_cooperative_level(ds* self, HWND hwnd, DWORD dwLevel);

HRESULT DELTACALL ds_initialize(ds* pDS, LPCGUID pcGuidDevice);
