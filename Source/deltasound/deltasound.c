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

#include "deltasound.h"
#include "device_info.h"
#include "ds.h"

#define DELTASOUNDDEVICE_INVALID_COUNT ((DWORD)-1)

HRESULT DELTACALL deltasound_allocate(allocator* pAlloc, deltasound** ppOut);

HRESULT DELTACALL deltasound_create(allocator* pAlloc, deltasound** ppOut) {
    HRESULT hr = S_OK;
    deltasound* instance = NULL;

    if (SUCCEEDED(hr = deltasound_allocate(pAlloc, &instance))) {
        if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Items))) {
            InitializeCriticalSection(&instance->Lock);
            *ppOut = instance;
            return S_OK;
        }

        deltasound_release(instance);
    }

    return hr;
}

VOID DELTACALL deltasound_release(deltasound* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    for (UINT i = arr_get_count(self->Items); i != 0; i--) {
        ds* instance = NULL;
        if (SUCCEEDED(arr_remove_item(self->Items, i - 1, &instance))) {
            ds_release(instance);
        }
    }

    arr_release(self->Items);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL deltasound_create_directsound(deltasound* self,
    REFIID riid, LPCGUID pcGuidDevice, LPDIRECTSOUND* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = ds_create(self->Allocator, riid, &instance))) {
        instance->Instance = self;

        if (SUCCEEDED(hr = ds_initialize(instance, pcGuidDevice))) {
            ids* intfc = NULL;

            if (SUCCEEDED(hr = ds_query_interface(instance, riid, &intfc))) {
                if (SUCCEEDED(hr = arr_add_item(self->Items, instance))) {
                    *ppOut = (LPDIRECTSOUND)intfc;
                    goto exit;
                }
            }
        }

        ds_release(instance);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_remove_ds(deltasound* self, ds* pDS) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDS == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    for (UINT i = 0; i < arr_get_count(self->Items); i++) {
        ds* instance = NULL;

        if (SUCCEEDED(hr = arr_get_item(self->Items, i, &instance))) {
            if (instance == pDS) {
                return arr_remove_item(self->Items, i, NULL);
            }
        }
    }

    return hr;
}

HRESULT DELTACALL deltasound_can_unload(deltasound* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return arr_get_count(self->Items) == 0 ? S_OK : S_FALSE;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL deltasound_allocate(allocator* pAlloc, deltasound** ppOut) {
    HRESULT hr = S_OK;
    deltasound* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(deltasound), &instance))) {
        instance->Allocator = pAlloc;
        *ppOut = instance;
    }

    return hr;
}
