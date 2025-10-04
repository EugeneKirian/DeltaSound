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

HRESULT DELTACALL ds_create(allocator* pAlloc, REFCLSID rclsid, ds** ppOut) {
    if (pAlloc == NULL || rclsid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(ds), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, rclsid, sizeof(CLSID));

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Buffers))) {
                dsb* main = NULL;

                REFIID riid = IsEqualCLSID(&CLSID_DirectSound, rclsid)
                    ? &IID_IDirectSoundBuffer : &IID_IDirectSoundBuffer8;

                if (SUCCEEDED(hr = dsb_create(pAlloc, riid, &main))) {
                    InitializeCriticalSection(&instance->Lock);

                    instance->Main = main;

                    *ppOut = instance;

                    return S_OK;
                }

                arr_release(instance->Buffers);
            }

            intfc_release(instance->Interfaces);
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL ds_release(ds* self) {
    if (self == NULL) { return; }

    if (self->Device != NULL) {
        dsdevice_release(self->Device);
    }

    DeleteCriticalSection(&self->Lock);

    {
        const DWORD count = intfc_get_count(self->Interfaces);

        for (DWORD i = 0; i < count; i++) {
            ids* instance = NULL;

            if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
                ids_release(instance);
            }
        }
    }

    intfc_release(self->Interfaces);

    {
        const DWORD count = arr_get_count(self->Buffers);

        for (DWORD i = 0; i < count; i++) {
            dsb* instance = NULL;

            if (SUCCEEDED(arr_get_item(self->Buffers, i, &instance))) {
                dsb_release(instance);
            }
        }
    }

    arr_release(self->Buffers);

    dsb_release(self->Main);

    if (self->Instance != NULL) {
        deltasound_remove_direct_sound(self->Instance, self);
    }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL ds_query_interface(ds* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;
    ids* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
        ids_add_ref(instance);

        *ppOut = instance;

        goto exit;
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || (IsEqualCLSID(&CLSID_DirectSound, &self->ID) && IsEqualIID(&IID_IDirectSound, riid))
        || (IsEqualCLSID(&CLSID_DirectSound8, &self->ID) && IsEqualIID(&IID_IDirectSound8, riid))) {
        if (SUCCEEDED(hr = ids_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = ds_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            ids_release(instance);
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
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

HRESULT DELTACALL ds_create_sound_buffer(ds* self, REFIID riid, LPCDSBUFFERDESC pcDesc, dsb** ppOut) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (pcDesc->dwFlags & DSBCAPS_PRIMARYBUFFER) {
        self->Main->Caps.dwFlags = (pcDesc->dwFlags & DSBCAPS_LOCHARDWARE)
            ? (pcDesc->dwFlags & (~DSBCAPS_LOCHARDWARE)) | DSBCAPS_LOCSOFTWARE
            : pcDesc->dwFlags | DSBCAPS_LOCSOFTWARE;

        *ppOut = self->Main;

        return S_OK;
    }

    if ((IsEqualCLSID(&CLSID_DirectSound, &self->ID) && !IsEqualIID(&IID_IDirectSoundBuffer, riid))
        || (IsEqualCLSID(&CLSID_DirectSound8, &self->ID) && !IsEqualIID(&IID_IDirectSoundBuffer8, riid))) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = dsb_create(self->Allocator, riid, &instance))) {
        if (SUCCEEDED(hr = dsb_initialize(instance, self, pcDesc))) {
            if (SUCCEEDED(hr = arr_add_item(self->Buffers, instance))) {

                *ppOut = instance;

                goto exit;
            }
        }

        dsb_release(instance);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL ds_remove_sound_buffer(ds* self, dsb* pDSB) {
    HRESULT hr = S_OK;
    const DWORD count = arr_get_count(self->Buffers);

    for (DWORD i = 0; i < count; i++) {
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

HRESULT DELTACALL ds_duplicate_sound_buffer(ds* self, dsb* pDSBufferOriginal, dsb** ppDSBufferDuplicate) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Main == pDSBufferOriginal) {
        return DSERR_INVALIDCALL;
    }

    HRESULT hr = E_INVALIDARG;

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Buffers);

    for (DWORD i = 0; i < count; i++) {
        dsb* instance = NULL;

        if (SUCCEEDED(arr_get_item(self->Buffers, i, &instance))) {
            if (instance == pDSBufferOriginal) {
                hr = dsb_duplicate(pDSBufferOriginal, ppDSBufferDuplicate);
                goto exit;
            }
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
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

        if (FAILED(hr = device_info_get_default_device(DEVICETYPE_RENDER, kind, &info))) {
            return DSERR_NODRIVER;
        }
    }
    else if (FAILED(hr = device_info_get_device(DEVICETYPE_RENDER, pcGuidDevice, &info))) {
        return DSERR_NODRIVER;
    }

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = dsdevice_create(self->Allocator, self, &info, &self->Device))) {
        DSBUFFERDESC desc;
        ZeroMemory(&desc, sizeof(DSBUFFERDESC));

        desc.dwSize = sizeof(DSBUFFERDESC);
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE;

        hr = dsb_initialize(self->Main, self, &desc);
    }

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL ds_set_cooperative_level(ds* self, HWND hwnd, DWORD dwLevel) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    EnterCriticalSection(&self->Lock);

    self->HWND = hwnd; // TODO Get root parent window (topmost unowned window)
    self->Level = dwLevel;

    if (dwLevel == DSSCL_WRITEPRIMARY) {
        const DWORD count = arr_get_count(self->Buffers);

        for (DWORD i = 0; i < count; i++) {
            dsb* instance = NULL;

            if (SUCCEEDED(arr_get_item(self->Buffers, i, &instance))) {
                instance->Play = DSBPLAY_NONE;
                instance->Status = DSBSTATUS_BUFFERLOST;
            }
        }
    }
    else {
        self->Main->Play = DSBPLAY_NONE;
        self->Main->Status = DSBSTATUS_NONE;

        dsbcb_set_current_position(self->Main->Buffer, 0, 0, DSBCB_SETPOSITION_NONE);
    }

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL ds_compact(ds* self) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Level < DSSCL_PRIORITY) {
        return DSERR_PRIOLEVELNEEDED;
    }

    return S_OK;
}

HRESULT DELTACALL ds_get_status(ds* self, LPDWORD pdwStatus) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (pdwStatus == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD status = DSBSTATUS_NONE;

    if (FAILED(hr = dsb_get_status(self->Main, &status))) {
        return hr;
    }

    if (status & DSBSTATUS_PLAYING) {
        *pdwStatus = DS_STATUS_PLAYING;
        return hr;
    }

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Buffers);

    for (DWORD i = 0; i < count; i++) {
        dsb* instance = NULL;

        if (SUCCEEDED(arr_get_item(self->Buffers, i, &instance))) {
            if (SUCCEEDED(hr = dsb_get_status(instance, &status))) {
                if (status & DSBSTATUS_PLAYING) {

                    *pdwStatus = DS_STATUS_PLAYING;

                    goto exit;
                }
            }
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}
