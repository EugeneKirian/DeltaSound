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

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WdsbcblcANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WdsbcblcANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "cblc.h"

#define DEFAULT_CAPACITY            8
#define DEFAULT_CAPACITY_MULTIPLIER 2

struct cblc {
    allocator*          Allocator;
    CRITICAL_SECTION    Lock;

    DWORD               Count;
    DWORD               Capacity;

    cbl*                Items;
};

HRESULT DELTACALL cblc_resize(cblc* pLock);

HRESULT DELTACALL cblc_create(allocator* pAlloc, cblc** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    cblc* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(cblc), &instance))) {
        instance->Allocator = pAlloc;

        instance->Count = 0;
        instance->Capacity = DEFAULT_CAPACITY;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc,
            instance->Capacity * sizeof(cbl), &instance->Items))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL cblc_release(cblc* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    allocator_free(self->Allocator, self->Items);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL cblc_add_item(cblc* self, cbl* pItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pItem == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&self->Lock);

    if (self->Capacity < self->Count + 1) {
        if (FAILED(hr = cblc_resize(self))) {
            goto exit;
        }
    }

    CopyMemory(&self->Items[self->Count], pItem, sizeof(cbl));

    self->Count++;

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL cblc_get_item(cblc* self, DWORD dwIndex, cbl** ppItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < dwIndex + 1 || ppItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    *ppItem = &self->Items[dwIndex];

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL cblc_remove_item(cblc* self, DWORD dwIndex) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < dwIndex + 1) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (self->Count != dwIndex + 1) {
        MoveMemory(&self->Items[dwIndex],
            &self->Items[dwIndex + 1], (self->Count - dwIndex - 1) * sizeof(cbl));
    }

    self->Count--;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

DWORD DELTACALL cblc_get_count(cblc* self) {
    return self == NULL ? 0 : self->Count;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL cblc_resize(cblc* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    const DWORD capacity = max(self->Capacity, 1) * DEFAULT_CAPACITY_MULTIPLIER;
    const DWORD size = capacity * sizeof(cbl);

    if (FAILED(hr = allocator_reallocate(self->Allocator, self->Items, size, &self->Items))) {
        return hr;
    }

    self->Capacity = capacity;

    return hr;
}
