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

#include "arr.h"

#define DEFAULT_CAPACITY            8
#define DEFAULT_CAPACITY_MULTIPLIER 2

struct arr {
    allocator*          Allocator;
    CRITICAL_SECTION    Lock;

    DWORD               Count;
    DWORD               Capacity;

    LPVOID*             Items;
};

HRESULT DELTACALL arr_allocate(allocator* pAlloc, arr** ppOut);
HRESULT DELTACALL arr_resize(arr* pArray);

HRESULT DELTACALL arr_create(allocator* pAlloc, arr** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    arr* instance = NULL;

    if (SUCCEEDED(hr = arr_allocate(pAlloc, &instance))) {
        instance->Count = 0;
        instance->Capacity = DEFAULT_CAPACITY;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc,
            instance->Capacity * sizeof(LPVOID), (LPVOID*)&instance->Items))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        arr_release(instance);
    }

    return hr;
}

VOID DELTACALL arr_release(arr* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    allocator_free(self->Allocator, self->Items);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL arr_add_item(arr* self, LPVOID pItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (self->Capacity < self->Count + 1) {
        HRESULT hr = S_OK;
        if (FAILED(hr = arr_resize(self))) {
            LeaveCriticalSection(&self->Lock);
            return hr;
        }
    }

    self->Items[self->Count] = pItem;

    self->Count++;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL arr_get_item(arr* self, DWORD dwIndex, LPVOID* ppItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < dwIndex + 1 || ppItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    *ppItem = self->Items[dwIndex];

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL arr_remove_item(arr* self, DWORD dwIndex, LPVOID* ppItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < dwIndex + 1) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (ppItem != NULL) {
        *ppItem = self->Items[dwIndex];
    }

    if (self->Count != dwIndex + 1) {
        MoveMemory(&self->Items[dwIndex],
            &self->Items[dwIndex + 1], (self->Count - dwIndex - 1) * sizeof(LPVOID));
    }

    self->Count--;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

DWORD DELTACALL arr_get_count(arr* self) {
    return self == NULL ? 0 : self->Count;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL arr_allocate(allocator* pAlloc, arr** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    arr* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(arr), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL arr_resize(arr* self) {
    HRESULT hr = S_OK;

    const DWORD capacity = max(self->Capacity, 1) * DEFAULT_CAPACITY_MULTIPLIER;
    const DWORD size = capacity * sizeof(LPVOID);

    if (FAILED(hr = allocator_reallocate(self->Allocator, self->Items, size, (LPVOID*)&self->Items))) {
        return hr;
    }

    self->Capacity = capacity;

    return hr;
}
