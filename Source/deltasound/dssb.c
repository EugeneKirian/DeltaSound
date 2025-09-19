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
#include "dssb.h"

HRESULT DELTACALL dssb_create(allocator* pAlloc, REFIID riid, dssb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dssb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dssb), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, riid, sizeof(GUID));

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL dssb_release(dssb* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    const DWORD count = intfc_get_count(self->Interfaces);

    for (DWORD i = 0; i < count; i++) {
        idssb* instance = NULL;

        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idssb_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dssb_query_interface(dssb* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    {
        idssb* instance = NULL;

        if (SUCCEEDED(intfc_query_item(self->Interfaces, riid, &instance))) {
            idssb_add_ref(instance);

            *ppOut = instance;

            goto exit;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSound3DBuffer, riid)) {
        idssb* instance = NULL;

        if (SUCCEEDED(hr = idssb_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dssb_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            idssb_release(instance);
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL dssb_add_ref(dssb* self, idssb* pIDSSB) {
    return intfc_add_item(self->Interfaces, &pIDSSB->ID, pIDSSB);
}

HRESULT DELTACALL dssb_remove_ref(dssb* self, idssb* pIDSSB) {
    intfc_remove_item(self->Interfaces, &pIDSSB->ID);

    // TODO NOT IMPLEMENTED
    // what to do when the buffer is released? Keep the actual settings and conntinue using them?

    return S_OK;
}
