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

typedef struct iprvt_vft iprvt_vft;
typedef struct prvt prvt;

typedef struct iprvt {
    const iprvt_vft*    Self;
    allocator*          Allocator;
    IID                 ID;
    LONG                RefCount;
    prvt*               Instance;
} iprvt;

typedef HRESULT(DELTACALL* LPIPRVTQUERYINTERFACE)(iprvt*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIPRVTADDREF)(iprvt*);
typedef ULONG(DELTACALL* LPIPRVTRELEASE)(iprvt*);

typedef HRESULT(DELTACALL* LPIPRVTGET)(iprvt*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned);
typedef HRESULT(DELTACALL* LPIPRVTSET)(iprvt*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength);
typedef HRESULT(DELTACALL* LPIPRVTQUERYSUPPORT)(iprvt*, REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport);

HRESULT DELTACALL iprvt_create(allocator* pAlloc, REFIID riid, iprvt** ppOut);
VOID DELTACALL iprvt_release(iprvt* pIPrvt);

HRESULT DELTACALL iprvt_query_interface(iprvt* pIPrvt, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL iprvt_add_ref(iprvt* pIPrvt);
ULONG DELTACALL iprvt_remove_ref(iprvt* pIPrvt);
