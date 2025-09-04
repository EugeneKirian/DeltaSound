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

HRESULT DELTACALL dsb_get_volume(dsb* self, PFLOAT pfVolume) {
    if (!self->Caps.dwFlags & DSBCAPS_CTRLVOLUME) {
        return DSERR_CONTROLUNAVAIL;
    }

    if (self->Instance == NULL) {
        return DSERR_UNSUPPORTED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    *pfVolume = self->Volume;

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
