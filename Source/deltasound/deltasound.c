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
#include "device_info.h"
#include "ds.h"
#include "dsc.h"
#include "prvt.h"

#define DELTASOUNDDEVICE_INVALID_COUNT ((DWORD)-1)

HRESULT DELTACALL deltasound_create(allocator* pAlloc, deltasound** ppOut) {
    HRESULT hr = S_OK;
    deltasound* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(deltasound), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Render))) {
            if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Capture))) {
                if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Create))) {
                    if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Private))) {
                        InitializeCriticalSection(&instance->Lock);

                        *ppOut = instance;

                        return S_OK;
                    }

                    arr_release(instance->Private);
                }

                arr_release(instance->Capture);
            }

            arr_release(instance->Render);
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL deltasound_release(deltasound* self) {
    if (self == NULL) { return; }

    {
        const DWORD count = arr_get_count(self->Render);

        for (DWORD i = count; i != 0; i--) {
            ds* instance = NULL;

            if (SUCCEEDED(arr_remove_item(self->Render, i - 1, &instance))) {
                ds_release(instance);
            }
        }
    }

    {
        const DWORD count = arr_get_count(self->Capture);

        for (DWORD i = count; i != 0; i--) {
            dsc* instance = NULL;

            if (SUCCEEDED(arr_remove_item(self->Capture, i - 1, &instance))) {
                dsc_release(instance);
            }
        }
    }

    {
        const DWORD count = arr_get_count(self->Create);

        for (DWORD i = count; i != 0; i--) {
            cf* instance = NULL;

            if (SUCCEEDED(arr_remove_item(self->Create, i - 1, &instance))) {
                cf_release(instance);
            }
        }
    }

    {
        const DWORD count = arr_get_count(self->Private);

        for (DWORD i = count; i != 0; i--) {
            prvt* instance = NULL;

            if (SUCCEEDED(arr_remove_item(self->Private, i - 1, &instance))) {
                prvt_release(instance);
            }
        }
    }

    DeleteCriticalSection(&self->Lock);

    arr_release(self->Render);
    arr_release(self->Capture);
    arr_release(self->Create);
    arr_release(self->Private);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL deltasound_create_direct_sound(deltasound* self,
    REFCLSID rclsid, LPCGUID pcGuidDevice, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (rclsid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    if (!IsEqualCLSID(&CLSID_DirectSound, rclsid)
        && !IsEqualCLSID(&CLSID_DirectSound8, rclsid)) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = ds_create(self->Allocator, rclsid, &instance))) {
        instance->Instance = self;

        if (SUCCEEDED(hr = ds_initialize(instance, pcGuidDevice))) {
            ids* intfc = NULL;

            REFIID riid = IsEqualCLSID(&CLSID_DirectSound, rclsid)
                ? &IID_IDirectSound : &IID_IDirectSound8;

            if (SUCCEEDED(hr = ds_query_interface(instance, riid, &intfc))) {
                if (SUCCEEDED(hr = arr_add_item(self->Render, instance))) {

                    *ppOut = intfc;

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

HRESULT DELTACALL deltasound_remove_direct_sound(deltasound* self, ds* pDS) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDS == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Render);

    for (DWORD i = 0; i < count; i++) {
        ds* instance = NULL;

        if (SUCCEEDED(hr = arr_get_item(self->Render, i, &instance))) {
            if (instance == pDS) {
                hr = arr_remove_item(self->Render, i, NULL);
                goto exit;
            }
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_create_direct_sound_capture(deltasound* self,
    REFCLSID rclsid, LPCGUID pcGuidDevice, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (rclsid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    if (!IsEqualCLSID(&CLSID_DirectSoundCapture, rclsid)
        && !IsEqualCLSID(&CLSID_DirectSoundCapture8, rclsid)) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsc* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = dsc_create(self->Allocator, rclsid, &instance))) {
        instance->Instance = self;

        if (SUCCEEDED(hr = dsc_initialize(instance, pcGuidDevice))) {
            ids* intfc = NULL;

            if (SUCCEEDED(hr = dsc_query_interface(instance, &IID_IDirectSoundCapture, &intfc))) {
                if (SUCCEEDED(hr = arr_add_item(self->Capture, instance))) {

                    *ppOut = intfc;

                    goto exit;
                }
            }
        }

        dsc_release(instance);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_remove_direct_sound_capture(deltasound* self, dsc* pDSC) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDSC == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Capture);

    for (DWORD i = 0; i < count; i++) {
        dsc* instance = NULL;

        if (SUCCEEDED(hr = arr_get_item(self->Capture, i, &instance))) {
            if (instance == pDSC) {
                hr = arr_remove_item(self->Capture, i, NULL);
                goto exit;
            }
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_create_class_factory(deltasound* self,
    REFCLSID rclsid, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (rclsid == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    if (!IsEqualCLSID(&CLSID_DirectSound, rclsid)
        && !IsEqualCLSID(&CLSID_DirectSound8, rclsid)
        && !IsEqualCLSID(&CLSID_DirectSoundCapture, rclsid)
        && !IsEqualCLSID(&CLSID_DirectSoundCapture8, rclsid)
        && !IsEqualCLSID(&CLSID_DirectSoundFullDuplex, rclsid)
        && !IsEqualCLSID(&CLSID_DirectSoundPrivate, rclsid)) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    HRESULT hr = S_OK;
    cf* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = cf_create(self->Allocator, rclsid, &instance))) {
        instance->Instance = self;

        icf* intfc = NULL;

        if (SUCCEEDED(hr = cf_query_interface(instance, riid, &intfc))) {
            if (SUCCEEDED(hr = arr_add_item(self->Create, instance))) {

                *ppOut = intfc;

                goto exit;
            }
        }

        cf_release(instance);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_remove_class_factory(deltasound* self, cf* pCF) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pCF == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Create);

    for (DWORD i = 0; i < count; i++) {
        cf* instance = NULL;

        if (SUCCEEDED(hr = arr_get_item(self->Create, i, &instance))) {
            if (instance == pCF) {
                hr = arr_remove_item(self->Create, i, NULL);
                break;
            }
        }
    }

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_add_private(deltasound* self, prvt* pPrvt) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pPrvt == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&self->Lock);

    hr = arr_add_item(self->Private, pPrvt);

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_remove_private(deltasound* self, prvt* pPrvt) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pPrvt == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Private);

    for (DWORD i = 0; i < count; i++) {
        prvt* instance = NULL;

        if (SUCCEEDED(hr = arr_get_item(self->Private, i, &instance))) {
            if (instance == pPrvt) {
                hr = arr_remove_item(self->Private, i, NULL);
                break;
            }
        }
    }

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL deltasound_can_unload(deltasound* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    const BOOL result = arr_get_count(self->Render) == 0
        && arr_get_count(self->Capture) == 0
        && arr_get_count(self->Create) == 0;

    return result ? S_OK : S_FALSE;
}
