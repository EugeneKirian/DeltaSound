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

#include "idsn.h"
#include "intfc.h"

typedef struct dsb dsb;

typedef struct dsn {
    allocator*  Allocator;
    GUID        ID;
    dsb*        Instance;
    intfc*      Interfaces;
} dsn;

HRESULT DELTACALL dsn_create(allocator* pAlloc, REFIID riid, dsn** ppOut);
VOID DELTACALL dsn_release(dsn* pDSN);

HRESULT DELTACALL dsn_query_interface(dsn* pDSN, REFIID riid, idsn** ppOut);
HRESULT DELTACALL dsn_add_ref(dsn* pDSN, idsn* pIDSN);
HRESULT DELTACALL dsn_remove_ref(dsn* pDSN, idsn* pIDSN);
