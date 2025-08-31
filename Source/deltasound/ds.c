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
#include "ds.h"

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut);

HRESULT DELTACALL ds_create(allocator* pAlloc, ds** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;
    if (FAILED(hr = ds_allocate(pAlloc, &instance))) {
        return hr;
    }

    ZeroMemory(instance, sizeof(instance));

    instance->Allocator = pAlloc;

    if (FAILED(hr = ids_create(&instance->Interface))) {
        ds_release(instance);
        return hr;
    }

    *ppOut = instance;

    return S_OK;
}

VOID DELTACALL ds_release(ds* self) {
    if (self == NULL) {
        return;
    }

    // TODO cleanup logic

    allocator_free(self->Allocator, self);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;
    if (FAILED(hr = allocator_allocate(pAlloc, sizeof(instance), &instance))) {
        return hr;
    }

    ZeroMemory(instance, sizeof(instance));

    instance->Allocator = pAlloc;

    *ppOut = instance;

    return S_OK;
}
