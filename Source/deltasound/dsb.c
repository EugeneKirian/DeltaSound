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
#include "dsb.h"
#include "ids.h"

HRESULT DELTACALL dsb_allocate(allocator* pAlloc, dsb** ppOut);

HRESULT DELTACALL dsb_create(allocator* pAlloc, BOOL bInterface, dsb** ppOut) {
    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = dsb_allocate(pAlloc, &instance))) {
        instance->Caps.dwSize = sizeof(DSBCAPS);

        if (!bInterface) {
            *ppOut = instance;
            return S_OK;
        }

        idsb* intfc = NULL;

        if (SUCCEEDED(hr = idsb_create(pAlloc, &intfc))) {
            intfc->Instance = instance;

            if (SUCCEEDED(hr = dsb_add_ref(instance, intfc))) {
                *ppOut = instance;
                return S_OK;
            }

            idsb_release(intfc);
        }

        dsb_release(instance);
    }

    return hr;
}

VOID DELTACALL dsb_release(dsb* self) {
    for (LONG i = 0; i < self->InterfaceCount; i++) {
        idsb_release(self->Interfaces[i]);
    }

    allocator_free(self->Allocator, self->Interfaces);

    // TODO

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsb_set_flags(dsb* self, DWORD dwFlags) {
    self->Caps.dwFlags = dwFlags;

    // TODO properly update things when new flags are set
    return S_OK;
}

HRESULT DELTACALL dsb_add_ref(dsb* self, idsb* pIDSB) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // TODO synchronization

    if (SUCCEEDED(hr = allocator_reallocate(self->Allocator,
        self->Interfaces, sizeof(idsb*) * (self->InterfaceCount + 1), (LPVOID*)&self->Interfaces))) {
        self->Interfaces[self->InterfaceCount] = pIDSB;
        self->InterfaceCount++;
    }

    return hr;
}

HRESULT DELTACALL dsb_remove_ref(dsb* self, idsb* pIDSB) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // TODO synchronization

    for (LONG i = 0; i < self->InterfaceCount; i++) {
        if (self->Interfaces[i] == pIDSB) {
            for (LONG x = i; x < self->InterfaceCount - 1; x++) {
                self->Interfaces[x] = self->Interfaces[x + 1];
            }

            self->InterfaceCount--;

            break;
        }
    }

    if (self->InterfaceCount <= 0) {
        if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
            dsb_release(self);
        }
    }

    return hr;
}

HRESULT DELTACALL dsb_get_caps(dsb* self, LPDSBCAPS pCaps) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    CopyMemory(pCaps, &self->Caps, sizeof(DSBCAPS));

    return S_OK;
}

HRESULT DELTACALL dsb_get_current_position(dsb* self,
    LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (pdwCurrentPlayCursor != NULL) {
        *pdwCurrentPlayCursor = self->CurrentPlayCursor;
    }

    if (pdwCurrentWriteCursor != NULL) {
        *pdwCurrentWriteCursor = self->CurrentWriteCursor;
    }

    return S_OK;
}

HRESULT DELTACALL dsb_get_format(dsb* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (pwfxFormat != NULL) {
        CopyMemory(pwfxFormat, self->Format, dwSizeAllocated);
    }

    if (pdwSizeWritten != NULL) {
        *pdwSizeWritten = sizeof(WAVEFORMATEX);
    }

    return S_OK;
}

HRESULT DELTACALL dsb_get_volume(dsb* self, PFLOAT pfVolume) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLVOLUME)) {
        return DSERR_CONTROLUNAVAIL;
    }

    *pfVolume = self->Volume;

    return S_OK;
}

HRESULT DELTACALL dsb_get_pan(dsb* self, PFLOAT pfPan) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLPAN)) {
        return DSERR_CONTROLUNAVAIL;
    }

    *pfPan = self->Pan;

    return S_OK;
}

HRESULT DELTACALL dsb_get_frequency(dsb* self, LPDWORD pdwFrequency) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLFREQUENCY)) {
        return DSERR_CONTROLUNAVAIL;
    }

    *pdwFrequency = self->Frequency;

    return S_OK;
}

HRESULT DELTACALL dsb_initialize(dsb* self, ds* pDS, LPCDSBUFFERDESC pcDesc) {
    if (self->Instance != NULL) {
        return DSERR_ALREADYINITIALIZED;
    }

    self->Instance = pDS;

    self->Caps.dwFlags = pcDesc->dwFlags;

    if (!self->Caps.dwFlags & (DSBCAPS_LOCSOFTWARE | DSBCAPS_LOCHARDWARE)) {
        self->Caps.dwFlags |= DSBCAPS_LOCSOFTWARE;
    }

    // TODO set flags, default values, etc

    return S_OK;
}

HRESULT DELTACALL dsb_set_format(dsb* self, LPCWAVEFORMATEX pcfxFormat) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;
    const size_t size = sizeof(WAVEFORMATEX) + pcfxFormat->cbSize;

    if (self->Format == NULL) {
        if (FAILED(hr = allocator_allocate(self->Allocator, size, &self->Format))) {
            return hr;
        }
    }
    else if (sizeof(WAVEFORMATEX) + self->Format->cbSize < size) {
        if (FAILED(hr = allocator_reallocate(self->Allocator, self->Format, size, &self->Format))) {
            return hr;
        }
    }

    CopyMemory(self->Format, pcfxFormat, size);

    // TODO other updates to playback? Hm?

    return hr;
}

HRESULT DELTACALL dsb_set_volume(dsb* self, FLOAT fVolume) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLVOLUME)) {
        return DSERR_CONTROLUNAVAIL;
    }

    self->Volume = min(max(fVolume, DSB_MIN_VOLUME), DSB_MAX_VOLUME);

    return S_OK;
}

HRESULT DELTACALL dsb_set_pan(dsb* self, FLOAT fPan) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLPAN)) {
        return DSERR_CONTROLUNAVAIL;
    }

    self->Pan = min(max(fPan, DSB_LEFT_PAN), DSB_RIGHT_PAN);

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsb_allocate(allocator* pAlloc, dsb** ppOut) {
    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsb), &instance))) {
        ZeroMemory(instance, sizeof(dsb));
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, 0, (LPVOID*)&instance->Interfaces))) {
            *ppOut = instance;
            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}
