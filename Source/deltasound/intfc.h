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

typedef struct intfc intfc;

HRESULT DELTACALL intfc_create(allocator* pAlloc, intfc** ppOut);
VOID DELTACALL intfc_release(intfc* pIntfc);

HRESULT DELTACALL intfc_get_item(intfc* pIntfc, DWORD dwIndex, LPVOID* ppItem);
HRESULT DELTACALL intfc_query_item(intfc* pIntfc, REFIID riid, LPVOID* ppItem);

HRESULT DELTACALL intfc_add_item(intfc* pIntfc, REFIID riid, LPVOID pItem);
HRESULT DELTACALL intfc_remove_item(intfc* pIntfc, REFIID riid);

DWORD DELTACALL intfc_get_count(intfc* pIntfc);
