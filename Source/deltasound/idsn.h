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

typedef struct dsn dsn;
typedef struct idsn_vft idsn_vft;

typedef struct idsn {
    const idsn_vft*     Self;
    allocator*          Allocator;
    GUID                ID;
    LONG                RefCount;
    dsn*                Instance;
} idsn;

typedef HRESULT(DELTACALL* LPIDSNQUERYINTERFACE)(idsn*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSNADDREF)(idsn*);
typedef ULONG(DELTACALL* LPIDSNRELEASE)(idsn*);

typedef HRESULT(DELTACALL* LPIDSNSETNOTIFICATIONPOSITIONS)(idsn*,
    DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies);

HRESULT DELTACALL idsn_create(allocator* pAlloc, REFIID riid, idsn** ppOut);
VOID DELTACALL idsn_release(idsn* pIDSN);

HRESULT DELTACALL idsn_query_interface(idsn* pIDSN, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL idsn_add_ref(idsn* pIDSN);
ULONG DELTACALL idsn_remove_ref(idsn* pIDSN);
