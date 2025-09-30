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

#include "dscb.h"
#include "idscb.h"

typedef struct dsc dsc;

HRESULT DELTACALL idscb_get_caps(idscb* self, LPDSCBCAPS pDSCBCaps);
HRESULT DELTACALL idscb_get_current_position(idscb* self, LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition);
HRESULT DELTACALL idscb_get_format(idscb* self, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
HRESULT DELTACALL idscb_get_status(idscb* self, LPDWORD pdwStatus);
HRESULT DELTACALL idscb_initialize(idscb* self, dsc* pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc);
HRESULT DELTACALL idscb_lock(idscb* self, DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
HRESULT DELTACALL idscb_start(idscb* self, DWORD dwFlags);
HRESULT DELTACALL idscb_stop(idscb* self);
HRESULT DELTACALL idscb_unlock(idscb* self, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);

struct idscb_vft {
    LPIDSCBQUERYINTERFACE       QueryInterface;
    LPIDSCBADDREF               AddRef;
    LPIDSCBRELEASE              Release;
    LPIDSCBGETCAPS              GetCaps;
    LPIDSCBGETCURRENTPOSITION   GetCurrentPosition;
    LPIDSCBGETFORMAT            GetFormat;
    LPIDSCBGETSTATUS            GetStatus;
    LPIDSCBINITIALIZE           Initialize;
    LPIDSCBLOCK                 Lock;
    LPIDSCBSTART                Start;
    LPIDSCBSTOP                 Stop;
    LPIDSCBUNLOCK               Unlock;
};

const static idscb_vft idscb_self = {
    idscb_query_interface,
    idscb_add_ref,
    idscb_remove_ref,
    idscb_get_caps,
    idscb_get_current_position,
    idscb_get_format,
    idscb_get_status,
    idscb_initialize,
    idscb_lock,
    idscb_start,
    idscb_stop,
    idscb_unlock
};

HRESULT DELTACALL idscb_create(allocator* pAlloc, REFIID riid, idscb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idscb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idscb), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &idscb_self;
        CopyMemory(&instance->ID, riid, sizeof(GUID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idscb_release(idscb* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idscb_query_interface(idscb* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return dscb_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL idscb_add_ref(idscb* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idscb_remove_ref(idscb* self) {
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
            dscb_remove_ref(self->Instance, self);
            idscb_release(self);
        }
    }

    return result;
}

HRESULT DELTACALL idscb_get_caps(idscb* self, LPDSCBCAPS pCaps) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pCaps == NULL) {
        return E_INVALIDARG;
    }

    if (pCaps->dwSize != sizeof(DSCBCAPS)) {
        return E_INVALIDARG;
    }

    return dscb_get_caps(self->Instance, pCaps);
}

HRESULT DELTACALL idscb_get_current_position(idscb* self,
    LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwCapturePosition == NULL && pdwReadPosition == NULL) {
        return E_INVALIDARG;
    }

    return dscb_get_current_position(self->Instance, pdwCapturePosition, pdwReadPosition);
}

HRESULT DELTACALL idscb_get_format(idscb* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxFormat == NULL && pdwSizeWritten == NULL) {
        return E_INVALIDARG;
    }

    return dscb_get_format(self->Instance, pwfxFormat, dwSizeAllocated, pdwSizeWritten);
}

HRESULT DELTACALL idscb_get_status(idscb* self, LPDWORD pdwStatus) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwStatus == NULL) {
        return E_INVALIDARG;
    }

    return dscb_get_status(self->Instance, pdwStatus);
}


HRESULT DELTACALL idscb_initialize(idscb* self, dsc* pDSC, LPCDSCBUFFERDESC pcDesc) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDSC == NULL || pcDesc == NULL) {
        return E_INVALIDARG;
    }

    if (pcDesc->dwSize != sizeof(dscb_desc_min)
        && pcDesc->dwSize != sizeof(dscb_desc_max)) {
        return E_INVALIDARG;
    }

    return dscb_initialize(self->Instance, pDSC, pcDesc);
}

HRESULT DELTACALL idscb_lock(idscb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1,
    LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppvAudioPtr1 == NULL || pdwAudioBytes1 == NULL) {
        if (ppvAudioPtr1 != NULL) {
            *ppvAudioPtr1 = NULL;
        }

        if (pdwAudioBytes1 != NULL) {
            *pdwAudioBytes1 = 0;
        }

        return E_INVALIDARG;
    }

    return dscb_lock(self->Instance, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
}

HRESULT DELTACALL idscb_start(idscb* self, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dscb_start(self->Instance, dwFlags);
}

HRESULT DELTACALL idscb_stop(idscb* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dscb_stop(self->Instance);
}

HRESULT DELTACALL idscb_unlock(idscb* self, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dscb_unlock(self->Instance, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
}
