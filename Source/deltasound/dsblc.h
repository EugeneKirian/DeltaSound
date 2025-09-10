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

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WdsblcANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WdsblcANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "allocator.h"

typedef struct dsbl {
    DWORD   Offset;
    DWORD   Size;
    LPVOID  Audio1;
    DWORD   AudioSize1;
    LPVOID  Audio2;
    DWORD   AudioSize2;
} dsbl;

typedef struct dsblc dsblc;

HRESULT DELTACALL dsblc_create(allocator* pAlloc, dsblc** ppOut);
VOID DELTACALL dsblc_release(dsblc* pLock);

HRESULT DELTACALL dsblc_add_item(dsblc* pLock, dsbl* pItem);
HRESULT DELTACALL dsblc_get_item(dsblc* pLock, UINT nIndex, dsbl** ppItem);
HRESULT DELTACALL dsblc_remove_item(dsblc* pLock, UINT nIndex);

UINT DELTACALL dsblc_get_count(dsblc* pLock);
