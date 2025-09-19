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

#include "intfc.h"

#define DEFAULT_CAPACITY            8
#define DEFAULT_CAPACITY_MULTIPLIER 2

typedef struct intf {
    GUID    ID;
    LPVOID  Item;
} intf;

struct intfc {
    allocator*          Allocator;

    CRITICAL_SECTION    Lock;

    DWORD               Count;
    DWORD               Capacity;

    intf*               Items;
};

HRESULT DELTACALL intfc_resize(intfc* pIntfc);

HRESULT DELTACALL intfc_create(allocator* pAlloc, intfc** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    intfc* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(intfc), &instance))) {
        instance->Allocator = pAlloc;

        instance->Count = 0;
        instance->Capacity = DEFAULT_CAPACITY;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc,
            instance->Capacity * sizeof(intf), &instance->Items))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        intfc_release(instance);
    }

    return hr;
}

VOID DELTACALL intfc_release(intfc* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    allocator_free(self->Allocator, self->Items);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL intfc_get_item(intfc* self, DWORD dwIndex, LPVOID* ppItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Count < dwIndex + 1 || ppItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    *ppItem = self->Items[dwIndex].Item;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL intfc_query_item(intfc* self, REFIID riid, LPVOID* ppItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppItem == NULL) {
        return E_INVALIDARG;
    }

    if (self->Count == 0) {
        return E_NOINTERFACE;
    }

    LPVOID instance = NULL;

    EnterCriticalSection(&self->Lock);

    for (UINT i = 0; i < self->Count; i++) {
        if (IsEqualGUID(riid, &self->Items[i].ID)) {
            instance = self->Items[i].Item;
            break;
        }
    }

    *ppItem = instance;

    LeaveCriticalSection(&self->Lock);

    return instance == NULL ? E_NOINTERFACE : S_OK;
}

HRESULT DELTACALL intfc_add_item(intfc* self, REFIID riid, LPVOID pItem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || pItem == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    if (self->Capacity < self->Count + 1) {
        HRESULT hr = S_OK;
        if (FAILED(hr = intfc_resize(self))) {
            LeaveCriticalSection(&self->Lock);
            return hr;
        }
    }

    CopyMemory(&self->Items[self->Count].ID, riid, sizeof(GUID));
    self->Items[self->Count].Item = pItem;

    self->Count++;

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL intfc_remove_item(intfc* self, REFIID riid) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL) {
        return E_INVALIDARG;
    }

    if (self->Count == 0) {
        return S_OK;
    }

    EnterCriticalSection(&self->Lock);

    for (DWORD i = 0; i < self->Count; i++) {
        if (IsEqualGUID(riid, &self->Items[i].ID)) {
            for (DWORD k = i; k < self->Count - 1; k++) {
                CopyMemory(&self->Items[k], &self->Items[k + 1], sizeof(intf));
            }

            self->Count--;

            break;
        }
    }

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

DWORD DELTACALL intfc_get_count(intfc* self) {
    return self == NULL ? 0 : self->Count;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL intfc_resize(intfc* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    const DWORD capacity = max(self->Capacity, 1) * DEFAULT_CAPACITY_MULTIPLIER;
    const DWORD size = capacity * sizeof(intf);

    if (FAILED(hr = allocator_reallocate(self->Allocator, self->Items, size, &self->Items))) {
        return hr;
    }

    self->Capacity = capacity;

    return hr;
}
