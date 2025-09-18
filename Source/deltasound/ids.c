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

#include "ids.h"
#include "idsb.h"
#include "ds.h"
#include "dsb.h"
#include "wave_format.h"

HRESULT DELTACALL ids_create_sound_buffer(ids* self, LPCDSBUFFERDESC pcDSBufferDesc, idsb** ppDSBuffer, LPUNKNOWN pUnkOuter);
HRESULT DELTACALL ids_get_caps(ids* self, LPDSCAPS pDSCaps);
HRESULT DELTACALL ids_duplicate_sound_buffer(ids* self, idsb* pDSBufferOriginal, idsb** ppDSBufferDuplicate);
HRESULT DELTACALL ids_set_cooperative_level(ids* self, HWND hwnd, DWORD dwLevel);
HRESULT DELTACALL ids_compact(ids* self);
HRESULT DELTACALL ids_get_speaker_config(ids* self, LPDWORD pdwSpeakerConfig);
HRESULT DELTACALL ids_set_speaker_config(ids* self, DWORD dwSpeakerConfig);
HRESULT DELTACALL ids_initialize(ids* self, LPCGUID pcGuidDevice);

struct ids_vft {
    LPIDSQUERYINTERFACE         QueryInterface;
    LPIDSADDREF                 AddRef;
    LPIDSRELEASE                Release;
    LPIDSCREATESOUNDBUFFER      CreateSoundBuffer;
    LPIDSGETCAPS                GetCaps;
    LPIDSDUPLICATESOUNDBUFFER   DuplicateSoundBuffer;
    LPIDSSETCOOPERATIVELEVEL    SetCooperativeLevel;
    LPIDSCOMPACT                Compact;
    LPIDSGETSPEAKERCONFIG       GetSpeakerConfig;
    LPIDSSETSPEAKERCONFIG       SetSpeakerConfig;
    LPIDSINITIALIZE             Initialize;
};

const static ids_vft ids_self = {
    ids_query_interface,
    ids_add_ref,
    ids_remove_ref,
    ids_create_sound_buffer,
    ids_get_caps,
    ids_duplicate_sound_buffer,
    ids_set_cooperative_level,
    ids_compact,
    ids_get_speaker_config,
    ids_set_speaker_config,
    ids_initialize
};

HRESULT DELTACALL ids_allocate(allocator* pAlloc, ids** ppOut);
HRESULT DELTACALL ids_validate_primary_buffer_desc(ids* self, LPCDSBUFFERDESC pcDesc);
HRESULT DELTACALL ids_validate_secondary_buffer_desc(ids* self, LPCDSBUFFERDESC pcDesc);

HRESULT DELTACALL ids_create(allocator* pAlloc, REFIID riid, ids** ppOut) {
    if (pAlloc == NULL || riid  == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ids* instance = NULL;

    if (SUCCEEDED(hr = ids_allocate(pAlloc, &instance))) {
        instance->Self = &ids_self;
        CopyMemory(&instance->ID, riid, sizeof(GUID));
        instance->RefCount = 1;
        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL ids_release(ids* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL ids_query_interface(ids* self, REFIID riid, LPVOID* ppvObject) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppvObject == NULL) {
        return E_INVALIDARG;
    }

    return ds_query_interface(self->Instance, riid, ppvObject);
}

ULONG DELTACALL ids_add_ref(ids* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL ids_remove_ref(ids* self) {
    if (self == NULL) {
        return 0;
    }

    if (self->RefCount == 0) {
        return 0;
    }

    if (InterlockedDecrement(&self->RefCount) <= 0) {
        self->RefCount = 0;

        if (self->Instance != NULL) {
            ds_remove_ref(self->Instance, self);
        }

        ids_release(self);
    }

    return self->RefCount;
}

HRESULT DELTACALL ids_create_sound_buffer(ids* self,
    LPCDSBUFFERDESC pcDesc, idsb** ppBuffer, LPUNKNOWN pUnkOuter) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pcDesc == NULL || ppBuffer == NULL) {
        return E_INVALIDARG;
    }

    // dwSize

    if (pcDesc->dwSize != sizeof(dsb_desc_min)
        && pcDesc->dwSize != sizeof(dsb_desc_max)) {
        return E_INVALIDARG;
    }

    // dwFlags

    if ((pcDesc->dwFlags & DSBCAPS_LOCSOFTWARE)
        && (pcDesc->dwFlags & DSBCAPS_LOCHARDWARE)) {
        return E_INVALIDARG;
    }

    if (!(pcDesc->dwFlags & DSBCAPS_CTRL3D)
        && (pcDesc->dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE)) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    if (pcDesc->dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (FAILED(hr = ids_validate_primary_buffer_desc(self, pcDesc))) {
            return hr;
        }
    }
    else {
        if (FAILED(hr = ids_validate_secondary_buffer_desc(self, pcDesc))) {
            return hr;
        }
    }

    // dwReserved

    if (pcDesc->dwReserved != 0) {
        return E_INVALIDARG;
    }

    // guid3DAlgorithm

    if (pcDesc->dwSize == sizeof(dsb_desc_max)
        && !(pcDesc->dwFlags & DSBCAPS_CTRL3D)
        && !IsEqualGUID(&pcDesc->guid3DAlgorithm, &DS3DALG_DEFAULT)) {
        return E_INVALIDARG;
    }

    if (pcDesc->dwSize == sizeof(dsb_desc_max)
        && (pcDesc->dwFlags & DSBCAPS_CTRL3D)) {
        if (!IsEqualGUID(&pcDesc->guid3DAlgorithm, &DS3DALG_DEFAULT)
            && !IsEqualGUID(&pcDesc->guid3DAlgorithm, &DS3DALG_NO_VIRTUALIZATION)
            && !IsEqualGUID(&pcDesc->guid3DAlgorithm, &DS3DALG_HRTF_FULL)
            && !IsEqualGUID(&pcDesc->guid3DAlgorithm, &DS3DALG_HRTF_LIGHT)) {
            return E_INVALIDARG;
        }
    }

    // TODO Additional guid3DAlgorithm value checks
    // TODO guid3DAlgorithm is used in 3d buffer in secondary buffers.

    if (pUnkOuter != NULL) {
        return DSERR_NOAGGREGATION;
    }

    dsb* instance = NULL;

    REFIID id = IsEqualIID(&self->ID, &IID_IDirectSound)
        ? &IID_IDirectSoundBuffer : &IID_IDirectSoundBuffer8;

    if (SUCCEEDED(hr = ds_create_dsb(self->Instance, id, pcDesc, &instance))) {
        hr = dsb_query_interface(instance, id, ppBuffer);
    }

    return hr;
}

HRESULT DELTACALL ids_get_caps(ids* self, LPDSCAPS pDSCaps) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDSCaps == NULL) {
        return E_INVALIDARG;
    }

    if (pDSCaps->dwSize != sizeof(DSCAPS)) {
        return E_INVALIDARG;
    }

    return ds_get_caps(self->Instance, pDSCaps);
}

HRESULT DELTACALL ids_duplicate_sound_buffer(ids* self, idsb* pDSBufferOriginal, idsb** ppDSBufferDuplicate) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_set_cooperative_level(ids* self, HWND hwnd, DWORD dwLevel) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (!IsWindow(hwnd)) {
        return E_INVALIDARG;
    }

    if (dwLevel != DSSCL_NORMAL && dwLevel != DSSCL_PRIORITY
        && dwLevel != DSSCL_EXCLUSIVE && dwLevel != DSSCL_WRITEPRIMARY) {
        return E_INVALIDARG;
    }

    return ds_set_cooperative_level(self->Instance, hwnd, dwLevel);
}

HRESULT DELTACALL ids_compact(ids* self) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_get_speaker_config(ids* self, LPDWORD pdwSpeakerConfig) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_set_speaker_config(ids* self, DWORD dwSpeakerConfig) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_initialize(ids* self, LPCGUID pcGuidDevice) {
    if (self == NULL) {
        return E_POINTER;
    }

    return ds_initialize(self->Instance, pcGuidDevice);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL ids_allocate(allocator* pAlloc, ids** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ids* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(ids), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL ids_validate_primary_buffer_desc(ids* self, LPCDSBUFFERDESC pcDesc) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pcDesc == NULL) {
        return E_INVALIDARG;
    }

    // dwFlags

    if (pcDesc->dwFlags & DSBCAPS_INVALID_PRIMARYBUFFER) {
        return E_INVALIDARG;
    }

    if ((pcDesc->dwFlags & DSBCAPS_LOCDEFER)
        && (pcDesc->dwFlags & (DSBCAPS_LOCSOFTWARE | DSBCAPS_LOCHARDWARE))) {
        return E_INVALIDARG;
    }

    if (pcDesc->dwFlags & DSBCAPS_CTRLFX) {
        return E_INVALIDARG;
    }

    // dwBufferBytes

    if (pcDesc->dwBufferBytes != 0) {
        return E_INVALIDARG;
    }

    // lpwfxFormat

    if (pcDesc->lpwfxFormat != NULL) {
        return E_INVALIDARG;
    }

    // guid3DAlgorithm

    if (pcDesc->dwSize == sizeof(dsb_desc_max)
        && (pcDesc->dwFlags & DSBCAPS_CTRL3D)
        && !IsEqualGUID(&pcDesc->guid3DAlgorithm, &DS3DALG_DEFAULT)) {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT DELTACALL ids_validate_secondary_buffer_desc(ids* self, LPCDSBUFFERDESC pcDesc) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pcDesc == NULL) {
        return E_INVALIDARG;
    }

    // dwFlags

    if (pcDesc->dwFlags & DSBCAPS_LOCDEFER) {
        return E_INVALIDARG;
    }

    if ((pcDesc->dwFlags & DSBCAPS_CTRLFX)
        && !IsEqualIID(&IID_IDirectSound8, &self->ID)) {
        return DSERR_DS8_REQUIRED;
    }

    // dwBufferBytes

    if (pcDesc->dwBufferBytes < DSBSIZE_MIN || pcDesc->dwBufferBytes > DSBSIZE_MAX) {
        return E_INVALIDARG;
    }

    // lpwfxFormat

    if (pcDesc->lpwfxFormat == NULL) {
        return E_INVALIDARG;
    }

    return wave_format_is_valid(pcDesc->lpwfxFormat, TRUE);
}
