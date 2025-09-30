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
typedef struct dscdevice dscdevice;
typedef struct dscb dscb;
typedef struct idsc idsc;

typedef struct dsc {
    allocator*          Allocator;
    GUID                ID; // TODO CLSID
    deltasound*         Instance;
    intfc*              Interfaces;

    CRITICAL_SECTION    Lock;

    dscdevice*          Device;

    arr*                Buffers;
} dsc;

HRESULT DELTACALL dsc_create(allocator* pAlloc, REFIID riid, dsc** ppOut);
VOID DELTACALL dsc_release(dsc* pDSC);

HRESULT DELTACALL dsc_query_interface(dsc* pDSC, REFIID riid, LPVOID* ppOut);
HRESULT DELTACALL dsc_add_ref(dsc* pDSC, idsc* pIDSC);
HRESULT DELTACALL dsc_remove_ref(dsc* pDSC, idsc* pIDSC);

HRESULT DELTACALL dsc_create_capture_buffer(dsc* pDSC, REFIID riid, LPCDSCBUFFERDESC pcDSCBufferDesc, dscb** ppOut);
HRESULT DELTACALL dsc_remove_capture_buffer(dsc* pDSC, dscb* pDSCB);

HRESULT DELTACALL dsc_get_caps(dsc* pDSC, LPDSCCAPS pDSCCaps);
HRESULT DELTACALL dsc_initialize(dsc* pDSC, LPCGUID pcGuidDevice);
