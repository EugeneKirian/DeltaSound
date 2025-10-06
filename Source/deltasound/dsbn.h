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

#include "idsbn.h"
#include "intfc.h"

typedef struct dsb dsb;

typedef struct dsbn {
    allocator*              Allocator;
    IID                     ID;
    dsb*                    Instance;
    intfc*                  Interfaces;

    CRITICAL_SECTION        Lock;

    LPDSBPOSITIONNOTIFY     Notifications;
    DWORD                   NotificationCount;
} dsbn;

HRESULT DELTACALL dsbn_create(allocator* pAlloc, REFIID riid, dsbn** ppOut);
VOID DELTACALL dsbn_release(dsbn* pDSBN);

HRESULT DELTACALL dsbn_query_interface(dsbn* pDSBN, REFIID riid, LPVOID* ppOut);
HRESULT DELTACALL dsbn_add_ref(dsbn* pDSBN, idsn* pIDSBN);
HRESULT DELTACALL dsbn_remove_ref(dsbn* pDSBN, idsn* pIDSBN);

HRESULT DELTACALL dsbn_get_notification_positions(dsbn* pDSBN, LPDWORD pdwPositionNotifies, LPCDSBPOSITIONNOTIFY* ppcPositionNotifies);
HRESULT DELTACALL dsbn_set_notification_positions(dsbn* pDSBN, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies);
