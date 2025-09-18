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
#include "dssl.h"

HRESULT DELTACALL dssl_allocate(allocator* pAlloc, dssl** ppOut);

HRESULT DELTACALL dssl_create(allocator* pAlloc, REFIID riid, dssl** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dssl* instance = NULL;

    if (SUCCEEDED(hr = dssl_allocate(pAlloc, &instance))) {
        CopyMemory(&instance->ID, riid, sizeof(GUID));

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            *ppOut = instance;

            return S_OK;
        }

        dssl_release(instance);
    }

    return hr;
}

VOID DELTACALL dssl_release(dssl* self) {
    if (self == NULL) { return; }

    for (DWORD i = 0; i < intfc_get_count(self->Interfaces); i++) {
        idssl* instance = NULL;
        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idssl_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dssl_query_interface(dssl* self, REFIID riid, LPVOID* ppOut) {
    // TODO synchronization

    idssl* instance = NULL;

    if (SUCCEEDED(intfc_query_item(self->Interfaces, riid, &instance))) {
        idssl_add_ref(instance);
        *ppOut = instance;
        return S_OK;
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSound3DListener, riid)) {
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr = idssl_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dssl_add_ref(self, instance))) {
                instance->Instance = self;
                *ppOut = instance;
                return S_OK;
            }

            idssl_release(instance);
        }

        return hr;
    }

    // TODO NOT IMPLEMENTED

    return E_NOINTERFACE;
}

HRESULT DELTACALL dssl_add_ref(dssl* self, idssl* pIDSSL) {
    return intfc_add_item(self->Interfaces, &pIDSSL->ID, pIDSSL);
}

HRESULT DELTACALL dssl_remove_ref(dssl* self, idssl* pIDSSL) {
    intfc_remove_item(self->Interfaces, &pIDSSL->ID);

    // TODO NOT IMPLEMENTED
    // what to do when the listener is released? Keep the actual settings and conntinue using them?

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dssl_allocate(allocator* pAlloc, dssl** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dssl* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dssl), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}
