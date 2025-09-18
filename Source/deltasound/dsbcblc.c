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

#include "dsbcblc.h"

#define DEFAULT_CAPACITY            8
#define DEFAULT_CAPACITY_MULTIPLIER 2

struct dsbcblc {
    allocator*          Allocator;
    CRITICAL_SECTION    Lock;

    DWORD               Count;
    DWORD               Capacity;

    dsbcbl*             Items;
};

HRESULT DELTACALL dsbcblc_allocate(allocator* pAlloc, dsbcblc** ppOut);
HRESULT DELTACALL dsbcblc_resize(dsbcblc* self);

HRESULT DELTACALL dsbcblc_create(allocator* pAlloc, dsbcblc** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsbcblc* instance = NULL;

    if (SUCCEEDED(hr = dsbcblc_allocate(pAlloc, &instance))) {
        instance->Count = 0;
        instance->Capacity = DEFAULT_CAPACITY;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc,
            instance->Capacity * sizeof(dsbcbl), &instance->Items))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        dsbcblc_release(instance);
    }

    return hr;
}

VOID DELTACALL dsbcblc_release(dsbcblc* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    allocator_free(self->Allocator, self->Items);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsbcblc_add_item(dsbcblc* self, dsbcbl* pItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (self->Capacity < self->Count + 1) {
        HRESULT hr = S_OK;
        if (FAILED(hr = dsbcblc_resize(self))) {
            LeaveCriticalSection(&self->Lock);
            return hr;
        }
    }

    CopyMemory(&self->Items[self->Count], pItem, sizeof(dsbcbl));

    self->Count++;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL dsbcblc_get_item(dsbcblc* self, DWORD dwIndex, dsbcbl** ppItem) {
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

HRESULT DELTACALL dsbcblc_remove_item(dsbcblc* self, DWORD dwIndex) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < dwIndex + 1) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (self->Count != dwIndex + 1) {
        MoveMemory(&self->Items[dwIndex],
            &self->Items[dwIndex + 1], (self->Count - dwIndex - 1) * sizeof(dsbcbl));
    }

    self->Count--;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

DWORD DELTACALL dsbcblc_get_count(dsbcblc* self) {
    return self == NULL ? 0 : self->Count;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsbcblc_allocate(allocator* pAlloc, dsbcblc** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsbcblc* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsbcblc), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL dsbcblc_resize(dsbcblc* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    const DWORD capacity = max(self->Capacity, 1) * DEFAULT_CAPACITY_MULTIPLIER;
    const DWORD size = capacity * sizeof(dsbcbl);

    if (FAILED(hr = allocator_reallocate(self->Allocator, self->Items, size, &self->Items))) {
        return hr;
    }

    self->Capacity = capacity;

    return hr;
}
