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

#include "cf.h"
#include "deltasound.h"
#include "ds.h"
#include "dsc.h"

HRESULT DELTACALL cf_create(allocator* pAlloc, REFCLSID rclsid, cf** ppOut) {
    if (pAlloc == NULL || rclsid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    cf* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(cf), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, rclsid, sizeof(CLSID));

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL cf_release(cf* self) {
    if (self == NULL) { return; }

    if (self->Instance != NULL) {
        deltasound_remove_class_factory(self->Instance, self);
    }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL cf_query_interface(cf* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    {
        icf* instance = NULL;

        if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
            icf_add_ref(instance);

            *ppOut = instance;

            goto exit;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid) || IsEqualIID(&IID_IClassFactory, riid)) {
        icf* instance = NULL;

        if (SUCCEEDED(hr = icf_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = cf_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            icf_release(instance);
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL cf_add_ref(cf* self, icf* pICF) {
    return intfc_add_item(self->Interfaces, &pICF->ID, pICF);
}

HRESULT DELTACALL cf_remove_ref(cf* self, icf* pICF) {
    intfc_remove_item(self->Interfaces, &pICF->ID);

    if (intfc_get_count(self->Interfaces) == 0) {
        cf_release(self);
    }

    return S_OK;
}

HRESULT DELTACALL cf_create_instance(cf* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    if (IsEqualCLSID(&CLSID_DirectSound, &self->ID)
        || IsEqualCLSID(&CLSID_DirectSound8, &self->ID)) {
        ds* instance = NULL;

        if (SUCCEEDED(hr = ds_create(self->Allocator, &self->ID, &instance))) {
            instance->Instance = self->Instance;

            ids* intfc = NULL;

            if (SUCCEEDED(hr = ds_query_interface(instance, riid, &intfc))) {
                if (SUCCEEDED(hr = arr_add_item(self->Instance->Renderers, instance))) {

                    *ppOut = intfc;

                    goto exit;
                }
            }

            ds_release(instance);
        }
    }
    else if (IsEqualCLSID(&CLSID_DirectSoundCapture, &self->ID)
        || IsEqualCLSID(&CLSID_DirectSoundCapture8, &self->ID)) {
        dsc* instance = NULL;

        if (SUCCEEDED(hr = dsc_create(self->Allocator, &self->ID, &instance))) {
            instance->Instance = self->Instance;

            ids* intfc = NULL;

            if (SUCCEEDED(hr = dsc_query_interface(instance, riid, &intfc))) {
                if (SUCCEEDED(hr = arr_add_item(self->Instance->Capturers, instance))) {

                    *ppOut = intfc;

                    goto exit;
                }
            }

            dsc_release(instance);
        }
    }
    else if (IsEqualCLSID(&CLSID_DirectSoundCapture8, &self->ID)) {
        // TODO
    }
    else if (IsEqualCLSID(&CLSID_DirectSoundFullDuplex, &self->ID)) {
        // TODO
    }
    else if (IsEqualCLSID(&CLSID_DirectSoundPrivate, &self->ID)) {
        // TODO
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}
