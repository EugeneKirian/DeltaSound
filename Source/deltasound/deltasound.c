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

#include "ds.h"
#include "device_info.h"

#define DELTASOUNDDEVICE_INVALID_COUNT ((DWORD)-1)

HRESULT DELTACALL deltasound_allocate(allocator* pAlloc, deltasound** ppOut);
HRESULT DELTACALL deltasound_add_device(deltasound* pDS, device* pDevice);

HRESULT DELTACALL deltasound_create(allocator* pAlloc, deltasound** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    // TODO LOG

    HRESULT hr = S_OK;
    deltasound* instance = NULL;
    if (FAILED(hr = deltasound_allocate(pAlloc, &instance))) {
        return hr;
    }

    if (FAILED(hr = allocator_allocate(pAlloc, 0, (LPVOID*)&instance->Devices))) {
        deltasound_release(instance);
        return hr;
    }

    InitializeCriticalSection(&instance->Lock);

    *ppOut = instance;

    return hr;
}

VOID DELTACALL deltasound_release(deltasound* self) {
    if (self == NULL) {
        return;
    }

    // TODO LOG

    for (UINT i = self->DeviceCount; i != 0; i--) {
        device_release(self->Devices[i - 1]);
    }

    allocator_free(self->Allocator, self->Devices);

    DeleteCriticalSection(&self->Lock);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL deltasound_get_device(
    deltasound* self, DWORD dwType, LPCGUID pcGuidDevice, device** ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwType != DEVICETYPE_AUDIO && dwType != DEVICETYPE_RECORD) {
        return E_INVALIDARG;
    }

    if (pcGuidDevice == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    for (UINT i = 0; i < self->DeviceCount; i++) {
        if (dwType == self->Devices[i]->Info.Type
            && IsEqualGUID(pcGuidDevice, &self->Devices[i]->Info.ID)) {
            *ppOut = self->Devices[i];
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT DELTACALL deltasound_create_device(
    deltasound* self, DWORD dwType, LPCGUID pcGuidDevice, device** ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwType != DEVICETYPE_AUDIO && dwType != DEVICETYPE_RECORD) {
        return E_INVALIDARG;
    }

    if (pcGuidDevice == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    device* instance = NULL;
    if (SUCCEEDED(hr = device_create(self->Allocator, dwType, pcGuidDevice, &instance))) {
        if (FAILED(hr = deltasound_add_device(self, instance))) {
            device_release(instance);
            return hr;
        }

        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL deltasound_create_ds(deltasound* self, REFIID riid, LPDIRECTSOUND* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    ds* instance = NULL;
    HRESULT hr = S_OK;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = ds_create(self->Allocator, &instance))) {

        instance->DeltaSound = self;

        // TODO Keep track of all created instances

    }
    *ppOut = (LPDIRECTSOUND)instance;

    LeaveCriticalSection(&self->Lock);

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL deltasound_allocate(allocator* pAlloc, deltasound** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    deltasound* instance = NULL;
    if (FAILED(hr = allocator_allocate(pAlloc, sizeof(deltasound), &instance))) {
        return hr;
    }

    ZeroMemory(instance, sizeof(deltasound));

    instance->Allocator = pAlloc;

    *ppOut = instance;

    return S_OK;
}

HRESULT DELTACALL deltasound_add_device(deltasound* self, device* pDevice) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDevice == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    const size_t length = (self->DeviceCount + 1) * sizeof(device*);

    if (SUCCEEDED(hr = allocator_reallocate(self->Allocator,
        self->Devices, length, (LPVOID*)&self->Devices))) {
        self->Devices[self->DeviceCount] = pDevice;
        self->DeviceCount++;
    }

    return hr;
}
