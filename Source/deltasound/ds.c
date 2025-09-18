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
#include "dsb.h"
#include "dsdevice.h"
#include "ids.h"

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut);

HRESULT DELTACALL ds_create(allocator* pAlloc, REFIID riid, ds** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    if (SUCCEEDED(hr = ds_allocate(pAlloc, &instance))) {
        CopyMemory(&instance->ID, riid, sizeof(GUID));

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Buffers))) {
                dsb* main = NULL;
                LPCGUID id = IsEqualIID(&IID_IDirectSound, riid)
                    ? &IID_IDirectSoundBuffer : &IID_IDirectSoundBuffer8;

                if (SUCCEEDED(hr = dsb_create(pAlloc, id, &main))) {
                    instance->Main = main;
                    *ppOut = instance;
                    return S_OK;
                }
            }
        }

        ds_release(instance);
    }

    return hr;
}

VOID DELTACALL ds_release(ds* self) {
    if (self == NULL) { return; }

    for (DWORD i = 0; i < intfc_get_count(self->Interfaces); i++) {
        ids* instance = NULL;
        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            ids_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    for (DWORD i = arr_get_count(self->Buffers); i != 0; i--) {
        dsb* instane = NULL;
        if (SUCCEEDED(arr_remove_item(self->Buffers, i - 1, &instane))) {
            dsb_release(instane);
        }
    }

    arr_release(self->Buffers);

    if (self->Main != NULL) {
        dsb_release(self->Main);
    }

    if (self->Device != NULL) {
        dsdevice_release(self->Device);
    }

    if (self->Instance != NULL) {
        deltasound_remove_ds(self->Instance, self);
    }

    // TODO cleanup logic

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL ds_query_interface(ds* self, REFIID riid, LPVOID* ppOut) {
    // TODO synchronization

    ids* instance = NULL;

    if (SUCCEEDED(intfc_query_item(self->Interfaces, riid, &instance))) {
        ids_add_ref(instance);
        *ppOut = instance;
        return S_OK;
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSound, riid)
        || (IsEqualIID(&IID_IDirectSound8, &self->ID) && IsEqualIID(&IID_IDirectSound8, riid))) {
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr = ids_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = ds_add_ref(self, instance))) {
                instance->Instance = self;
                *ppOut = instance;
                return S_OK;
            }

            ids_release(instance);
        }

        return hr;
    }

    return E_NOINTERFACE;
}

HRESULT DELTACALL ds_add_ref(ds* self, ids* pIDS) {
    return intfc_add_item(self->Interfaces, &pIDS->ID, pIDS);
}

HRESULT DELTACALL ds_remove_ref(ds* self, ids* pIDS) {
    intfc_remove_item(self->Interfaces, &pIDS->ID);

    if (intfc_get_count(self->Interfaces) == 0) {
        ds_release(self);
    }

    return S_OK;
}

HRESULT DELTACALL ds_create_dsb(ds* self, REFIID riid, LPCDSBUFFERDESC pcDesc, dsb** ppOut) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (pcDesc->dwFlags & DSBCAPS_PRIMARYBUFFER) {
        const DWORD flags = (pcDesc->dwFlags & DSBCAPS_LOCHARDWARE)
            ? (pcDesc->dwFlags & (~DSBCAPS_LOCHARDWARE)) : pcDesc->dwFlags;
        dsb_set_flags(self->Main, flags | DSBCAPS_LOCSOFTWARE);
        *ppOut = self->Main;
        return S_OK;
    }

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = dsb_create(self->Allocator, riid, &instance))) {
        if (SUCCEEDED(hr = dsb_initialize(instance, self, pcDesc))) {
            if (SUCCEEDED(hr = arr_add_item(self->Buffers, instance))) {
                *ppOut = instance;
                return S_OK;
            }
        }

        dsb_release(instance);
    }

    return hr;
}

HRESULT DELTACALL ds_remove_dsb(ds* self, dsb* pDSB) {
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < arr_get_count(self->Buffers); i++) {
        dsb* instance = NULL;

        if (SUCCEEDED(hr = arr_get_item(self->Buffers, i, &instance))) {
            if (instance == pDSB) {
                return arr_remove_item(self->Buffers, i, NULL);
            }
        }
    }

    return hr;
}

HRESULT DELTACALL ds_get_caps(ds* self, LPDSCAPS pCaps) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    ZeroMemory(pCaps, sizeof(DSCAPS));

    pCaps->dwSize = sizeof(DSCAPS);
    pCaps->dwFlags = DSCAPS_SECONDARY16BIT | DSCAPS_SECONDARY8BIT
        | DSCAPS_SECONDARYSTEREO | DSCAPS_SECONDARYMONO
        | DSCAPS_CONTINUOUSRATE | DSCAPS_PRIMARY16BIT
        | DSCAPS_PRIMARY8BIT | DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARYMONO;

    pCaps->dwMinSecondarySampleRate = DSBFREQUENCY_MIN;
    pCaps->dwMaxSecondarySampleRate = DSBFREQUENCY_MAX;

    pCaps->dwPrimaryBuffers = 1;

    pCaps->dwMaxHwMixingAllBuffers = 1;
    pCaps->dwMaxHwMixingStaticBuffers = 1;
    pCaps->dwMaxHwMixingStreamingBuffers = 1;

    return S_OK;
}

HRESULT DELTACALL ds_initialize(ds* self, LPCGUID pcGuidDevice) {
    if (self->Device != NULL) {
        return DSERR_ALREADYINITIALIZED;
    }

    if (pcGuidDevice == NULL || IsEqualGUID(&GUID_NULL, pcGuidDevice)) {
        pcGuidDevice = &DSDEVID_DefaultPlayback;
    }

    if (IsEqualGUID(&DSDEVID_DefaultCapture, pcGuidDevice) ||
        IsEqualGUID(&DSDEVID_DefaultVoiceCapture, pcGuidDevice)) {
        return DSERR_NODRIVER;
    }

    HRESULT hr = S_OK;

    device_info info;
    ZeroMemory(&info, sizeof(device_info));

    if (IsEqualGUID(&DSDEVID_DefaultPlayback, pcGuidDevice) ||
        IsEqualGUID(&DSDEVID_DefaultVoicePlayback, pcGuidDevice)) {
        const DWORD kind = IsEqualGUID(&DSDEVID_DefaultPlayback, pcGuidDevice)
            ? DEVICEKIND_MULTIMEDIA : DEVICEKIND_COMMUNICATION;

        if (FAILED(hr = device_info_get_default_device(DEVICETYPE_AUDIO, kind, &info))) {
            return DSERR_NODRIVER;
        }
    }
    else if (FAILED(hr = device_info_get_device(DEVICETYPE_AUDIO, pcGuidDevice, &info))) {
        return DSERR_NODRIVER;
    }

    if (SUCCEEDED(hr = dsdevice_create(self->Allocator, self, info.Type, &info, &self->Device))) {
        DSBUFFERDESC desc;
        ZeroMemory(&desc, sizeof(DSBUFFERDESC));

        desc.dwSize = sizeof(DSBUFFERDESC);
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE;

        if (SUCCEEDED(hr = dsb_initialize(self->Main, self, &desc))) {
            return S_OK;
        }
    }

    return hr;
}

HRESULT DELTACALL ds_set_cooperative_level(ds* self, HWND hwnd, DWORD dwLevel) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    // TODO other validations

    self->HWND = hwnd;
    self->Level = dwLevel;

    // NOTE. Multiple changes are allowed.
    // They have to be properly handled.

    // TODO, if new level is DSSCL_WRITEPRIMARY
    // then stop all secondary buffers and mark them as lost?

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(ds), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}
