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

typedef struct cf cf;
typedef struct icf_vft icf_vft;

typedef struct icf {
    const icf_vft*  Self;
    allocator*      Allocator;
    GUID            ID;
    LONG            RefCount;
    cf*             Instance;
} icf;

typedef HRESULT(DELTACALL* LPICFQUERYINTERFACE)(icf*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPICFADDREF)(icf*);
typedef ULONG(DELTACALL* LPICFRELEASE)(icf*);

typedef HRESULT(DELTACALL* LPCIFCREATEINSTANCE)(icf*, LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppOut);
typedef HRESULT(DELTACALL* LPICFLOCKSERVER)(icf*, BOOL bLock);

HRESULT DELTACALL icf_create(allocator* pAlloc, REFIID riid, icf** ppOut);
VOID DELTACALL icf_release(icf* pICF);

HRESULT DELTACALL icf_query_interface(icf* pICF, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL icf_add_ref(icf* pICF);
ULONG DELTACALL icf_remove_ref(icf* pICF);
