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

#include "device_info.h"
#include "ds.h"

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut);

HRESULT DELTACALL ds_create(allocator* pAlloc, ds** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    if (SUCCEEDED(hr = ds_allocate(pAlloc, &instance))) {
        if (SUCCEEDED(hr = ids_create(&instance->Interface))) {
            if (SUCCEEDED(hr = dsb_create(pAlloc, &instance->Main))) {
                instance->RefCount = 1;

                *ppOut = instance;

                return S_OK;
            }
        }

        ds_release(instance);
    }

    return hr;
}

VOID DELTACALL ds_release(ds* self) {
    if (self == NULL) {
        return;
    }

    if (self->Device != NULL) {
        device_release(self->Device);
    }

    // TODO cleanup logic

    allocator_free(self->Allocator, self);
}

ULONG ds_add_ref(ds* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL ds_remove_ref(ds* self) {
    if (self == NULL) {
        return 0;
    }

    LONG count = InterlockedDecrement(&self->RefCount);

    if (count <= 0) {
        count = 0;
        ds_release(self);
    }

    return count;
}

HRESULT DELTACALL ds_create_dsb(ds* self, LPCDSBUFFERDESC pcDesc, dsb** ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pcDesc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    if (pcDesc->dwSize != sizeof(dsb_desc_min)
        && pcDesc->dwSize != sizeof(dsb_desc_max)) {
        return E_INVALIDARG;
    }

    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (pcDesc->dwFlags & DSBCAPS_PRIMARYBUFFER) {
        // TODO process other flag validations

        dsb_add_ref(self->Main);

        *ppOut = self->Main;

        return S_OK;
    }

    // TODO process other flags properly

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = dsb_create(self->Allocator, &instance))) {
        if (SUCCEEDED(hr = dsb_initialize(instance, self->Instance, pcDesc))) {
            *ppOut = instance;

            return S_OK;
        }
        else if (hr == DSERR_ALREADYINITIALIZED) {
            *ppOut = instance;

            return S_OK;
        }

        dsb_release(instance);
    }

    return hr;
}

HRESULT DELTACALL ds_get_caps(ds* self, LPDSCAPS pCaps) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pCaps == NULL) {
        return E_INVALIDARG;
    }

    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    ZeroMemory(pCaps, sizeof(DSCAPS));

    pCaps->dwSize = sizeof(DSCAPS);
    pCaps->dwFlags = DSCAPS_SECONDARY16BIT | DSCAPS_SECONDARY8BIT
        | DSCAPS_SECONDARYSTEREO | DSCAPS_SECONDARYMONO
        | DSCAPS_CONTINUOUSRATE | DSCAPS_PRIMARY16BIT
        | DSCAPS_PRIMARY8BIT | DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARYMONO;

    pCaps->dwMinSecondarySampleRate = 100;
    pCaps->dwMaxSecondarySampleRate = 200000;
    pCaps->dwPrimaryBuffers = 1;
    pCaps->dwMaxHwMixingAllBuffers = 1;
    pCaps->dwMaxHwMixingStaticBuffers = 1;
    pCaps->dwMaxHwMixingStreamingBuffers = 1;

    return S_OK;
}

HRESULT DELTACALL ds_initialize(ds* self, LPCGUID pcGuidDevice) {
    if (self == NULL) {
        return E_POINTER;
    }

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

    hr = device_create(self->Allocator, info.Type, &info.ID, &self->Device);

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(ds), &instance))) {

        ZeroMemory(instance, sizeof(ds));

        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}
