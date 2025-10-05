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

typedef struct idsbps_vft idsbps_vft;
typedef struct dsbps dsbps;

typedef struct idsbps {
    const idsbps_vft*   Self;
    allocator*          Allocator;
    IID                 ID;
    LONG                RefCount;
    dsbps*              Instance;
} idsbps;

typedef HRESULT(DELTACALL* LPIDSBPSQUERYINTERFACE)(idsbps*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSBPSADDREF)(idsbps*);
typedef ULONG(DELTACALL* LPIDSBPSRELEASE)(idsbps*);

typedef HRESULT(DELTACALL* LPIDSBPSGET)(idsbps*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned);
typedef HRESULT(DELTACALL* LPIDSBPSSET)(idsbps*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength);
typedef HRESULT(DELTACALL* LPIDSBPSQUERYSUPPORT)(idsbps*, REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport);

HRESULT DELTACALL idsbps_create(allocator* pAlloc, REFIID riid, idsbps** ppOut);
VOID DELTACALL idsbps_release(idsbps* pIPS);

HRESULT DELTACALL idsbps_query_interface(idsbps* pIPS, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL idsbps_add_ref(idsbps* pIPS);
ULONG DELTACALL idsbps_remove_ref(idsbps* pIPS);
