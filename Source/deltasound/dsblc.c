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

#include "dsblc.h"

#define DEFAULT_CAPACITY            8
#define DEFAULT_CAPACITY_MULTIPLIER 2

struct dsblc {
    allocator* Allocator;
    CRITICAL_SECTION    Lock;

    UINT                Count;
    UINT                Capacity;

    dsbl*               Items;
};

HRESULT DELTACALL dsblc_allocate(allocator* pAlloc, dsblc** ppOut);
HRESULT DELTACALL dsblc_resize(dsblc* self);

HRESULT DELTACALL dsblc_create(allocator* pAlloc, dsblc** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsblc* instance = NULL;

    if (SUCCEEDED(hr = dsblc_allocate(pAlloc, &instance))) {
        instance->Count = 0;
        instance->Capacity = DEFAULT_CAPACITY;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc,
            instance->Capacity * sizeof(dsbl), &instance->Items))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;
            return S_OK;
        }

        dsblc_release(instance);
    }

    return hr;
}

VOID DELTACALL dsblc_release(dsblc* self) {
    if (self != NULL) {
        DeleteCriticalSection(&self->Lock);
        allocator_free(self->Allocator, self->Items);
        allocator_free(self->Allocator, self);
    }
}

HRESULT DELTACALL dsblc_add_item(dsblc* self, dsbl* pItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (self->Capacity < self->Count + 1) {
        HRESULT hr = S_OK;
        if (FAILED(hr = dsblc_resize(self))) {
            LeaveCriticalSection(&self->Lock);
            return hr;
        }
    }

    CopyMemory(&self->Items[self->Count], pItem, sizeof(dsbl));

    self->Count++;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL dsblc_get_item(dsblc* self, UINT nIndex, dsbl** ppItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < nIndex + 1 || ppItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    *ppItem = &self->Items[nIndex];

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL dsblc_remove_item(dsblc* self, UINT nIndex) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < nIndex + 1) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (self->Count != nIndex + 1) {
        MoveMemory(&self->Items[nIndex],
            &self->Items[nIndex + 1], (self->Count - nIndex - 1) * sizeof(dsbl));
    }

    self->Count--;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

UINT DELTACALL dsblc_get_count(dsblc* self) {
    return self == NULL ? 0 : self->Count;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsblc_allocate(allocator* pAlloc, dsblc** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsblc* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsblc), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL dsblc_resize(dsblc* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    const UINT capacity = max(self->Capacity, 1) * DEFAULT_CAPACITY_MULTIPLIER;
    const UINT size = capacity * sizeof(dsbl);

    if (FAILED(hr = allocator_reallocate(self->Allocator, self->Items, size, &self->Items))) {
        return hr;
    }

    self->Capacity = capacity;

    return hr;
}
