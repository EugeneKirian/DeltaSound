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

#include "allocator.h"

struct allocator {
    HANDLE Heap;
    // TODO Collect statistics such as allocation count, and allocation info
};

HRESULT DELTACALL allocator_create(allocator** ppAlloc) {
    if (ppAlloc == NULL) {
        return E_INVALIDARG;
    }

    HANDLE heap = GetProcessHeap();

    if (heap == NULL) {
        return E_FAIL;
    }

    allocator* alloc = HeapAlloc(heap, 0, sizeof(allocator));

    if (alloc == NULL) {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(alloc, sizeof(allocator));

    alloc->Heap = heap;

    *ppAlloc = alloc;

    return S_OK;
}

VOID DELTACALL allocator_release(allocator* self) {
    if (self == NULL) { return; }

    // TODO LOG Memory leaks

    HeapFree(self->Heap, 0, self);
}

HRESULT DELTACALL allocator_allocate(allocator* self, size_t nBytes, LPVOID* ppMem) {
    if (self == NULL || ppMem == NULL) {
        return E_INVALIDARG;
    }

    // TODO Collect allocation stats

    LPVOID memory = HeapAlloc(self->Heap, 0, nBytes);

    if (memory == NULL) {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(memory, nBytes);

    *ppMem = memory;

    return S_OK;
}

HRESULT DELTACALL allocator_reallocate(allocator* self, LPVOID pMem, size_t nBytes, LPVOID* ppMem) {
    if (self == NULL || pMem == NULL || ppMem == NULL) {
        return E_INVALIDARG;
    }

    // TODO Collect allocation stats

    LPVOID memory = HeapReAlloc(self->Heap, 0, pMem, nBytes);

    if (memory == NULL) {
        return E_OUTOFMEMORY;
    }

    *ppMem = memory;

    return S_OK;
}

HRESULT DELTACALL allocator_free(allocator* self, LPVOID pMem) {
    if (self == NULL) {
        return E_INVALIDARG;
    }

    // TODO Collect release stats

    return HeapFree(self->Heap, 0, pMem) ? S_OK : E_FAIL;
}
