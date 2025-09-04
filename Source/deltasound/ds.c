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

#include "device.h"
#include "device_info.h"
#include "ds.h"
#include "dsb.h"
#include "ids.h"

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut);

HRESULT DELTACALL ds_create(allocator* pAlloc, ds** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    if (SUCCEEDED(hr = ds_allocate(pAlloc, &instance))) {
        ids* intfc = NULL;

        if (SUCCEEDED(hr = ids_create(pAlloc, &intfc))) {
            if (SUCCEEDED(hr = ds_add_ref(instance, intfc))) {
                intfc->Instance = instance;

                dsb* main = NULL;

                if (SUCCEEDED(hr = dsb_create(pAlloc, FALSE, &main))) {
                    // TODO better initialization for main buffer properties

                    main->Caps.dwBufferBytes = DSB_MAX_PRIMARY_BUFFER_SIZE;

                    instance->Main = main;
                    *ppOut = instance;
                    return S_OK;
                }
            }

            ids_release(intfc);
        }

        ds_release(instance);
    }

    return hr;
}

VOID DELTACALL ds_release(ds* self) {
    for (LONG i = 0; i < self->InterfaceCount; i++) {
        ids_release(self->Interfaces[i]);
    }

    allocator_free(self->Allocator, self->Interfaces);

    if (self->Main != NULL) {
        dsb_release(self->Main);
    }

    if (self->Device != NULL) {
        device_release(self->Device);
    }

    // TODO cleanup logic

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL ds_add_ref(ds* self, ids* pIDS) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // TODO synchronization

    if (SUCCEEDED(hr = allocator_reallocate(self->Allocator,
        self->Interfaces, sizeof(ids*) * (self->InterfaceCount + 1), (LPVOID*)&self->Interfaces))) {
        self->Interfaces[self->InterfaceCount] = pIDS;
        self->InterfaceCount++;
    }

    return hr;
}

HRESULT DELTACALL ds_remove_ref(ds* self, ids* pIDS) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // TODO synchronization

    for (LONG i = 0; i < self->InterfaceCount; i++) {
        if (self->Interfaces[i] == pIDS) {
            for (LONG x = i; x < self->InterfaceCount - 1; x++) {
                self->Interfaces[x] = self->Interfaces[x + 1];
            }

            self->InterfaceCount--;

            break;
        }
    }

    if (self->InterfaceCount <= 0) {
        ds_release(self);
    }

    return hr;
}

HRESULT DELTACALL ds_create_dsb(ds* self, LPCDSBUFFERDESC pcDesc, dsb** ppOut) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (pcDesc->dwFlags & DSBCAPS_PRIMARYBUFFER) {
        dsb_set_flags(self->Main, pcDesc->dwFlags | DSBCAPS_LOCSOFTWARE);

        // TODO Refactor to use query interface

        if (self->Main->InterfaceCount == 0) {
            HRESULT hr = S_OK;
            idsb* intfc = NULL;

            if (FAILED(hr = idsb_create(self->Allocator, &intfc))) {
                return hr;
            }

            intfc->Instance = self->Main;

            if (FAILED(hr = dsb_add_ref(self->Main, intfc))) {
                idsb_release(intfc);
                return hr;
            }

            // TODO better way to set primary buffer properties

            WAVEFORMATEX format;
            ZeroMemory(&format, sizeof(WAVEFORMATEX));

            format.wFormatTag = WAVE_FORMAT_PCM;
            format.nChannels = 2;
            format.nSamplesPerSec = 22050;
            format.nAvgBytesPerSec = 44100;
            format.nBlockAlign = 2;
            format.wBitsPerSample = 8;

            dsb_set_format(self->Main, &format);
            dsb_set_pan(self->Main, DSB_CENTER_PAN);
            dsb_set_volume(self->Main, DSB_MAX_VOLUME);
        }
        else {
            idsb_add_ref(self->Main->Interfaces[0]);
        }

        *ppOut = self->Main;

        return S_OK;
    }

    // TODO process other flags properly

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = dsb_create(self->Allocator, TRUE, &instance))) {
        if (SUCCEEDED(hr = dsb_initialize(instance, self, pcDesc))) {
            *ppOut = instance;
            return S_OK;
        }

        dsb_release(instance);
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

    if (SUCCEEDED(hr = device_create(self->Allocator, info.Type, &info, &self->Device))) {
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

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut) {
    HRESULT hr = S_OK;
    ds* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(ds), &instance))) {
        ZeroMemory(instance, sizeof(ds));
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, 0, (LPVOID*)&instance->Interfaces))) {
            *ppOut = instance;
            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}
