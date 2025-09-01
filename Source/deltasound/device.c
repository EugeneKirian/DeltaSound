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
#include "uuid.h"

#include <dsound.h>

#define CINTERFACE
#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#define RELEASEHANDLE(X) if((X)) { CloseHandle((X)); (X) = NULL; }

HRESULT DELTACALL device_allocate(allocator* pAlloc, device** ppOut);

HRESULT DELTACALL device_create(
    allocator * pAlloc, DWORD dwType, LPCGUID pcGuidDevice, device * *ppOut) {
    if (pAlloc == NULL) {
        return E_INVALIDARG;
    }

    if (dwType != DEVICETYPE_AUDIO && dwType != DEVICETYPE_RECORD) {
        return E_INVALIDARG;
    }

    if (pcGuidDevice == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    device_info dev;
    ZeroMemory(&dev, sizeof(device_info));

    if (FAILED(device_info_get_device(dwType, pcGuidDevice, &dev))) {
        return DSERR_NODRIVER;
    }

    HRESULT hr = S_OK;
    device* instance = NULL;
    if (SUCCEEDED(hr = device_allocate(pAlloc, &instance))) {
        // TODO start device thread

        CopyMemory(&instance->Info, &dev, sizeof(device_info));

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL device_release(device* self) {
    if (self == NULL) {
        return;
    }

    allocator_free(self->Allocator, self);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL device_allocate(allocator* pAlloc, device** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    device* instance = NULL;
    if (FAILED(hr = allocator_allocate(pAlloc, sizeof(device), &instance))) {
        return hr;
    }

    ZeroMemory(instance, sizeof(device));

    instance->Allocator = pAlloc;

    *ppOut = instance;

    return S_OK;
}
