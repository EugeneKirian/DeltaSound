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

#define DSCBCAPS_NONE   0

typedef struct dsc dsc;
typedef struct idsc_vft idsc_vft;
typedef struct idscb idscb;

typedef struct idsc {
    const idsc_vft* Self;
    allocator*      Allocator;
    GUID            ID;
    LONG            RefCount;
    dsc*            Instance;
} idsc;

typedef HRESULT(DELTACALL* LPIDSCQUERYINTERFACE)(idsc*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSCADDREF)(idsc*);
typedef ULONG(DELTACALL* LPIDSCRELEASE)(idsc*);

typedef HRESULT(DELTACALL* LPIDSCCREATECAPTUREBUFFER)(idsc*,
    LPCDSCBUFFERDESC pcDSCBufferDesc, idscb** ppDSCBuffer, LPUNKNOWN pUnkOuter);
typedef HRESULT(DELTACALL* LPIDSCGETCAPS)(idsc*, LPDSCCAPS pDSCCaps);
typedef HRESULT(DELTACALL* LPIDSCINITIALIZE)(idsc*, LPCGUID pcGuidDevice);

HRESULT DELTACALL idsc_create(allocator* pAlloc, REFIID riid, idsc** ppOut);
VOID DELTACALL idsc_release(idsc* pIDSC);

HRESULT DELTACALL idsc_query_interface(idsc* pIDSC, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL idsc_add_ref(idsc* pIDSC);
ULONG DELTACALL idsc_remove_ref(idsc* pIDSC);
