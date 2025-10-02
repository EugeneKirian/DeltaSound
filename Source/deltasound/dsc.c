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
#include "dsc.h"
#include "dscb.h"
#include "dscdevice.h"
#include "idsc.h"

HRESULT DELTACALL dsc_create(allocator* pAlloc, REFCLSID rclsid, dsc** ppOut) {
    if (pAlloc == NULL || rclsid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsc* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsc), &instance))) {
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

VOID DELTACALL dsc_release(dsc* self) {
    if (self == NULL) { return; }

    if (self->Device != NULL) {
        dscdevice_release(self->Device);
    }

    DeleteCriticalSection(&self->Lock);

    {
        const DWORD count = intfc_get_count(self->Interfaces);

        for (DWORD i = 0; i < count; i++) {
            idsc* instance = NULL;

            if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
                idsc_release(instance);
            }
        }
    }

    intfc_release(self->Interfaces);

    if (self->Buffer != NULL) {
        dscb_release(self->Buffer);
    }

    if (self->Instance != NULL) {
        deltasound_remove_direct_sound_capture(self->Instance, self);
    }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsc_query_interface(dsc* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;
    idsc* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
        idsc_add_ref(instance);

        *ppOut = instance;

        goto exit;
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSoundCapture, riid)) {
        if (SUCCEEDED(hr = idsc_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dsc_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            idsc_release(instance);
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL dsc_add_ref(dsc* self, idsc* pIDSC) {
    return intfc_add_item(self->Interfaces, &pIDSC->ID, pIDSC);
}

HRESULT DELTACALL dsc_remove_ref(dsc* self, idsc* pIDSC) {
    intfc_remove_item(self->Interfaces, &pIDSC->ID);

    if (intfc_get_count(self->Interfaces) == 0) {
        dsc_release(self);
    }

    return S_OK;
}

HRESULT DELTACALL dsc_create_capture_buffer(dsc* self, REFIID riid, LPCDSCBUFFERDESC pcDesc, dscb** ppOut) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Buffer != NULL) {
        return DSERR_ALLOCATED;
    }

    if ((IsEqualCLSID(&CLSID_DirectSoundCapture, &self->ID) && !IsEqualIID(&IID_IDirectSoundCaptureBuffer, riid))
        || (IsEqualCLSID(&CLSID_DirectSoundCapture8, &self->ID) && !IsEqualIID(&IID_IDirectSoundCaptureBuffer8, riid))) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dscb* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = dscb_create(self->Allocator, riid, &instance))) {
        if (SUCCEEDED(hr = dscb_initialize(instance, self, pcDesc))) {
            self->Buffer = instance;

            *ppOut = instance;

            goto exit;
        }

        dscb_release(instance);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL dsc_remove_capture_buffer(dsc* self, dscb* pDSCB) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Buffer == NULL || self->Buffer != pDSCB) {
        return DSERR_INVALIDCALL;
    }

    self->Buffer = NULL;

    return S_OK;
}

HRESULT DELTACALL dsc_get_caps(dsc* self, LPDSCCAPS pDSCCaps) {
    if (self->Device == NULL) {
        return DSERR_UNINITIALIZED;
    }

    pDSCCaps->dwFlags = DSCCAPS_MULTIPLECAPTURE;
    pDSCCaps->dwFormats =
        WAVE_FORMAT_96S16 | WAVE_FORMAT_96M16 | WAVE_FORMAT_96S08 | WAVE_FORMAT_96M08
        | WAVE_FORMAT_48S16 | WAVE_FORMAT_48M16 | WAVE_FORMAT_48S08 | WAVE_FORMAT_48M08
        | WAVE_FORMAT_44S16 | WAVE_FORMAT_44M16 | WAVE_FORMAT_44S08 | WAVE_FORMAT_44M08
        | WAVE_FORMAT_4S16 | WAVE_FORMAT_4M16 | WAVE_FORMAT_4S08 | WAVE_FORMAT_4M08
        | WAVE_FORMAT_2S16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_2S08 | WAVE_FORMAT_2M08
        | WAVE_FORMAT_1S16 | WAVE_FORMAT_1M16 | WAVE_FORMAT_1S08 | WAVE_FORMAT_1M08;
    pDSCCaps->dwChannels = 2;

    return S_OK;
}

HRESULT DELTACALL dsc_initialize(dsc* self, LPCGUID pcGuidDevice) {
    if (self->Device != NULL) {
        return DSERR_ALREADYINITIALIZED;
    }

    if (pcGuidDevice == NULL || IsEqualGUID(&GUID_NULL, pcGuidDevice)) {
        pcGuidDevice = &DSDEVID_DefaultCapture;
    }

    if (IsEqualGUID(&DSDEVID_DefaultPlayback, pcGuidDevice) ||
        IsEqualGUID(&DSDEVID_DefaultVoicePlayback, pcGuidDevice)) {
        return DSERR_NODRIVER;
    }

    HRESULT hr = S_OK;

    device_info info;
    ZeroMemory(&info, sizeof(device_info));

    if (IsEqualGUID(&DSDEVID_DefaultCapture, pcGuidDevice) ||
        IsEqualGUID(&DSDEVID_DefaultVoiceCapture, pcGuidDevice)) {
        const DWORD kind = IsEqualGUID(&DSDEVID_DefaultCapture, pcGuidDevice)
            ? DEVICEKIND_MULTIMEDIA : DEVICEKIND_COMMUNICATION;

        if (FAILED(hr = device_info_get_default_device(DEVICETYPE_CAPTURE, kind, &info))) {
            return DSERR_NODRIVER;
        }
    }
    else if (FAILED(hr = device_info_get_device(DEVICETYPE_CAPTURE, pcGuidDevice, &info))) {
        return DSERR_NODRIVER;
    }

    EnterCriticalSection(&self->Lock);

    hr = dscdevice_create(self->Allocator, self, &info, &self->Device);

    LeaveCriticalSection(&self->Lock);

    return hr;
}
