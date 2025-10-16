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

typedef struct cbl {
    DWORD   Offset;
    DWORD   Size;
    LPVOID  Audio1;
    DWORD   AudioSize1;
    LPVOID  Audio2;
    DWORD   AudioSize2;
} cbl;

typedef struct cblc cblc;

HRESULT DELTACALL cblc_create(allocator* pAlloc, cblc** ppOut);
VOID DELTACALL cblc_release(cblc* pLock);

HRESULT DELTACALL cblc_add_item(cblc* pLock, cbl* pItem);
HRESULT DELTACALL cblc_get_item(cblc* pLock, DWORD dwIndex, cbl** ppItem);
HRESULT DELTACALL cblc_remove_item(cblc* pLock, DWORD dwIndex);

DWORD DELTACALL cblc_get_count(cblc* pLock);
