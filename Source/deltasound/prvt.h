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

#include "iprvt.h"

typedef struct deltasound deltasound;
typedef struct intfc intfc;

typedef struct prvt {
    allocator*          Allocator;
    IID                 ID;
    deltasound*         Instance;
    intfc*              Interfaces;

    CRITICAL_SECTION    Lock;
} prvt;

HRESULT DELTACALL prvt_create(allocator* pAlloc, REFIID riid, prvt** ppOut);
VOID DELTACALL prvt_release(prvt* pPrvt);

HRESULT DELTACALL prvt_query_interface(prvt* pPrvt, REFIID riid, LPVOID* ppOut);
HRESULT DELTACALL prvt_add_ref(prvt* pPrvt, iprvt* pIPrvt);
HRESULT DELTACALL prvt_remove_ref(prvt* pPrvt, iprvt* pIPrvt);

HRESULT DELTACALL prvt_get(prvt* pPrvt,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned);
HRESULT DELTACALL prvt_set(prvt* pPrvt,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength);
HRESULT DELTACALL prvt_query_support(prvt* pPrvt,
    REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport);
