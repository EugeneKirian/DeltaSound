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

#include "dsb.h"

HRESULT DELTACALL dsb_allocate(allocator* pAlloc, dsb** ppOut);

HRESULT DELTACALL dsb_create(allocator* pAlloc, dsb** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = dsb_allocate(pAlloc, &instance))) {
        if (SUCCEEDED(hr = idsb_create(&instance->Interface))) {
            instance->RefCount = 1;

            *ppOut = instance;

            return S_OK;
        }
    }

    return hr;
}

VOID DELTACALL dsb_release(dsb* self) {
    if (self == NULL) {
        return;
    }

    // TODO

    allocator_free(self->Allocator, self);
}

ULONG DELTACALL dsb_add_ref(dsb* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL dsb_remove_ref(dsb* self) {
    if (self == NULL) {
        return 0;
    }

    LONG count = InterlockedDecrement(&self->RefCount);

    if (count <= 0) {
        count = 0;

        // TODO release on primary buffer?

        dsb_release(self);
    }

    return count;
}

HRESULT DELTACALL dsb_initialize(dsb* self, deltasound* pDS, LPCDSBUFFERDESC pcDesc) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDS == NULL || pcDesc == NULL) {
        return E_INVALIDARG;
    }

    if (pcDesc->dwSize != sizeof(dsb_desc_min)
        && pcDesc->dwSize != sizeof(dsb_desc_max)) {
        return E_INVALIDARG;
    }

    if (self->Instance != NULL) {
        return DSERR_ALREADYINITIALIZED;
    }

    // TODO set flags, default values, etc

    self->Instance = pDS;

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsb_allocate(allocator* pAlloc, dsb** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsb), &instance))) {
        ZeroMemory(instance, sizeof(dsb));

        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}
