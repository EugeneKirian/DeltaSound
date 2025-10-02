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

#include "idsc.h"
#include "idscb.h"
#include "dsc.h"
#include "dscb.h"
#include "wave.h"

HRESULT DELTACALL idsc_create_capture_buffer(idsc* self, LPCDSCBUFFERDESC pDSCBufferDesc, idscb** ppDSCBuffer, LPUNKNOWN pUnkOuter);
HRESULT DELTACALL idsc_get_caps(idsc* self, LPDSCCAPS pDSCCaps);
HRESULT DELTACALL idsc_initialize(idsc* self, LPCGUID pcGuidDevice);

struct idsc_vft {
    LPIDSCQUERYINTERFACE        QueryInterface;
    LPIDSCADDREF                AddRef;
    LPIDSCRELEASE               Release;
    LPIDSCCREATECAPTUREBUFFER   CreateCaptureBuffer;
    LPIDSCGETCAPS               GetCaps;
    LPIDSCINITIALIZE            Initialize;
};

const static idsc_vft idsc_self = {
    idsc_query_interface,
    idsc_add_ref,
    idsc_remove_ref,
    idsc_create_capture_buffer,
    idsc_get_caps,
    idsc_initialize
};

HRESULT DELTACALL idsc_create(allocator* pAlloc, REFIID riid, idsc** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idsc* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idsc), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &idsc_self;
        CopyMemory(&instance->ID, riid, sizeof(IID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idsc_release(idsc* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idsc_query_interface(idsc* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return dsc_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL idsc_add_ref(idsc* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idsc_remove_ref(idsc* self) {
    if (self == NULL) {
        return 0;
    }

    if (self->RefCount == 0) {
        return 0;
    }

    LONG result = InterlockedDecrement(&self->RefCount);

    if ((result = max(result, 0)) == 0) {
        self->RefCount = 0;

        if (self->Instance != NULL) {
            dsc_remove_ref(self->Instance, self);
        }

        idsc_release(self);
    }

    return result;
}

HRESULT DELTACALL idsc_create_capture_buffer(idsc* self,
    LPCDSCBUFFERDESC pcDesc, idscb** ppDSCBuffer, LPUNKNOWN pUnkOuter) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pcDesc == NULL || ppDSCBuffer == NULL) {
        return E_INVALIDARG;
    }

    // dwSize

    if (pcDesc->dwSize != sizeof(dscb_desc_min)
        && pcDesc->dwSize != sizeof(dscb_desc_max)) {
        return E_INVALIDARG;
    }

    // dwFlags

    if (pcDesc->dwFlags != DSCBCAPS_NONE
        && pcDesc->dwFlags != DSCBCAPS_WAVEMAPPED) {
        return E_INVALIDARG;
    }

    // dwBufferBytes

    if (pcDesc->dwBufferBytes < DSBSIZE_MIN || pcDesc->dwBufferBytes > DSBSIZE_MAX) {
        return E_INVALIDARG;
    }

    // dwReserved

    if (pcDesc->dwReserved != 0) {
        return E_INVALIDARG;
    }

    // lpwfxFormat

    if (pcDesc->lpwfxFormat == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if (FAILED(hr = wave_format_is_valid(pcDesc->lpwfxFormat, TRUE))) {
        return hr;
    }

    if (pcDesc->dwSize == sizeof(dscb_desc_max)) {
        // TODO NOT IMPLEMENTED

        // dwFXCount

        // lpDSCFXDesc
    }

    // pUnkOuter

    if (pUnkOuter != NULL) {
        return DSERR_NOAGGREGATION;
    }

    dscb* instance = NULL;

    REFIID riid = IsEqualIID(&IID_IDirectSoundCapture, &self->ID)
        ? &IID_IDirectSoundCaptureBuffer : &IID_IDirectSoundCaptureBuffer8;

    if (SUCCEEDED(hr = dsc_create_capture_buffer(self->Instance, riid, pcDesc, &instance))) {
        hr = dscb_query_interface(instance, riid, ppDSCBuffer);
    }

    return hr;
}

HRESULT DELTACALL idsc_get_caps(idsc* self, LPDSCCAPS pDSCCaps) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDSCCaps == NULL) {
        return E_INVALIDARG;
    }

    if (pDSCCaps->dwSize != sizeof(DSCCAPS)) {
        return E_INVALIDARG;
    }

    return dsc_get_caps(self->Instance, pDSCCaps);
}

HRESULT DELTACALL idsc_initialize(idsc* self, LPCGUID pcGuidDevice) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dsc_initialize(self->Instance, pcGuidDevice);
}
