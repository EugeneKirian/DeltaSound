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
    if (FAILED(hr = ds_allocate(pAlloc, &instance))) {
        return hr;
    }

    if (FAILED(hr = ids_create(&instance->Interface))) {
        ds_release(instance);
        return hr;
    }

    instance->RefCount = 1;

    *ppOut = instance;

    return S_OK;
}

VOID DELTACALL ds_release(ds* self) {
    if (self == NULL) {
        return;
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
        // TODO cleanup


        allocator_free(self->Allocator, self);
    }

    return count;
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
    device_info dev;
    ZeroMemory(&dev, sizeof(device_info));

    if (IsEqualGUID(&DSDEVID_DefaultPlayback, pcGuidDevice) ||
        IsEqualGUID(&DSDEVID_DefaultVoicePlayback, pcGuidDevice)) {
        const DWORD kind = IsEqualGUID(&DSDEVID_DefaultPlayback, pcGuidDevice)
            ? DEVICEKIND_MULTIMEDIA : DEVICEKIND_COMMUNICATION;

        if (FAILED(hr = device_info_get_default_device(DEVICETYPE_AUDIO, kind, &dev))) {
            return DSERR_NODRIVER;
        }
    }
    else if (FAILED(hr = device_info_get_device(DEVICETYPE_AUDIO, pcGuidDevice, &dev))) {
        return DSERR_NODRIVER;
    }

    EnterCriticalSection(&self->DeltaSound->Lock);

    if (FAILED(hr = deltasound_get_device(self->DeltaSound, dev.Type, &dev.ID, &self->Device))) {
        hr = deltasound_create_device(self->DeltaSound, dev.Type, &dev.ID, &self->Device);
    }

    LeaveCriticalSection(&self->DeltaSound->Lock);

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL ds_allocate(allocator* pAlloc, ds** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = NULL;
    if (FAILED(hr = allocator_allocate(pAlloc, sizeof(ds), &instance))) {
        return hr;
    }

    ZeroMemory(instance, sizeof(ds));

    instance->Allocator = pAlloc;

    *ppOut = instance;

    return S_OK;
}
