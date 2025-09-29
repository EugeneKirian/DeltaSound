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

typedef struct iksp_vft iksp_vft;
typedef struct ksp ksp;

typedef struct iksp {
    const iksp_vft* Self;
    allocator*      Allocator;
    GUID            ID;
    LONG            RefCount;
    ksp*            Instance;
} iksp;

typedef HRESULT(DELTACALL* LPIKSPQUERYINTERFACE)(iksp*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIKSPADDREF)(iksp*);
typedef ULONG(DELTACALL* LPIKSPRELEASE)(iksp*);

typedef HRESULT(DELTACALL* LPIKSPGET)(iksp*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned);
typedef HRESULT(DELTACALL* LPIKSPSET)(iksp*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength);
typedef HRESULT(DELTACALL* LPIKSPQUERYSUPPORT)(iksp*, REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport);

HRESULT DELTACALL iksp_create(allocator* pAlloc, REFIID riid, iksp** ppOut);
VOID DELTACALL iksp_release(iksp* pIKSP);

HRESULT DELTACALL iksp_query_interface(iksp* pIKSP, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL iksp_add_ref(iksp* pIKSP);
ULONG DELTACALL iksp_remove_ref(iksp* pIKSP);
