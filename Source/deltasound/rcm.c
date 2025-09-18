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

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WrcmblcANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WrcmblcANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "rcm.h"

typedef struct rcm {
    allocator*  Allocator;
    LONG        RefCount;

    LPVOID      Buffer;
    DWORD       Size;
} rcm;

HRESULT DELTACALL rcm_create(allocator* pAlloc, DWORD dwBytes, rcm** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    rcm* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(rcm), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, dwBytes, &instance->Buffer))) {
            instance->RefCount = 1;
            instance->Size = dwBytes;

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL rcm_release(rcm* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self->Buffer);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL rcm_add_ref(rcm* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

HRESULT DELTACALL rcm_remove_ref(rcm* self) {
    if (self == NULL) {
        return 0;
    }

    if (self->RefCount == 0) {
        return 0;
    }

    if (InterlockedDecrement(&self->RefCount) <= 0) {
        self->RefCount = 0;

        rcm_release(self);
    }

    return self->RefCount;
}

HRESULT DELTACALL rcm_get_data(rcm* self, LPVOID* ppData) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppData == NULL) {
        return E_INVALIDARG;
    }

    *ppData = self->Buffer;
    
    return S_OK;
}

HRESULT DELTACALL rcm_get_size(rcm* self, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    *pdwBytes = self->Size;

    return S_OK;
}
