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
#include "deltasound.h"

#define DELTASOUNDDEVICE_INVALID_COUNT ((DWORD)-1)

struct deltasound {
    allocator*      Allocator;
};

HRESULT DELTACALL deltasound_allocate(allocator* pAlloc, deltasound** ppOut);

HRESULT DELTACALL deltasound_create(allocator* pAlloc, deltasound** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    // TODO LOG

    HRESULT hr = S_OK;
    deltasound* instance = NULL;
    if (FAILED(hr = deltasound_allocate(pAlloc, &instance))) {
        return hr;
    }

    *ppOut = instance;

    return hr;
}

VOID DELTACALL deltasound_release(deltasound* self) {
    if (self == NULL) {
        return;
    }

    // TODO LOG

    if (self != NULL) {
        // TODO

        allocator_free(self->Allocator, self);
    }
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL deltasound_allocate(allocator* pAlloc, deltasound** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    deltasound* instance = NULL;
    if (FAILED(hr = allocator_allocate(pAlloc, sizeof(deltasound), &instance))) {
        return hr;
    }

    ZeroMemory(instance, sizeof(deltasound));

    instance->Allocator = pAlloc;

    *ppOut = instance;

    return S_OK;
}
