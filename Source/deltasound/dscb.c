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

#include "dsc.h"
#include "dscb.h"
#include "dscbn.h"
#include "wave.h"

HRESULT DELTACALL dscb_create(allocator* pAlloc, REFIID riid, dscb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dscb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dscb), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, riid, sizeof(IID));

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(WAVEFORMATEXTENSIBLE), &instance->Format))) {
            if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
                InitializeCriticalSection(&instance->Lock);

                instance->Caps.dwSize = sizeof(DSCBCAPS);

                *ppOut = instance;

                return S_OK;
            }

            allocator_free(pAlloc, instance->Format);
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL dscb_release(dscb* self) {
    if (self == NULL) { return; }

    self->Start = DSCBSTART_NONE;
    self->Status = DSCBSTATUS_NONE;

    DeleteCriticalSection(&self->Lock);

    const DWORD count = intfc_get_count(self->Interfaces);

    for (DWORD i = 0; i < count; i++) {
        idscb* instance = NULL;

        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idscb_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    if (self->Instance != NULL) {
        dsc_remove_capture_buffer(self->Instance, self);
    }

    if (self->Buffer != NULL) {
        dscbcb_release(self->Buffer);
    }

    if (self->Notifications != NULL) {
        dscbn_release(self->Notifications);
    }

    allocator_free(self->Allocator, self->Format);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dscb_query_interface(dscb* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    {
        idscb* instance = NULL;

        if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
            idscb_add_ref(instance);

            *ppOut = instance;

            goto exit;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSoundCaptureBuffer, riid)
        || (IsEqualIID(&IID_IDirectSoundCaptureBuffer8, &self->ID) && IsEqualIID(&IID_IDirectSoundCaptureBuffer8, riid))) {
        idscb* instance = NULL;

        if (SUCCEEDED(hr = idscb_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dscb_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            idscb_release(instance);
        }
    }
    else if (IsEqualIID(&IID_IDirectSoundNotify, riid)) {
        if (self->Notifications == NULL) {
            dscbn* instance = NULL;

            if (FAILED(hr = dscbn_create(self->Allocator, riid, &instance))) {
                goto exit;
            }

            instance->Instance = self;
            self->Notifications = instance;
        }

        hr = dscbn_query_interface(self->Notifications, riid, ppOut);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL dscb_add_ref(dscb* self, idscb* pIDSCB) {
    return intfc_add_item(self->Interfaces, &pIDSCB->ID, pIDSCB);
}

HRESULT DELTACALL dscb_remove_ref(dscb* self, idscb* pIDSCB) {
    intfc_remove_item(self->Interfaces, &pIDSCB->ID);

    if (intfc_get_count(self->Interfaces) == 0) {
        dscb_release(self);
    }

    return S_OK;
}

HRESULT DELTACALL dscb_get_caps(dscb* self, LPDSCBCAPS pCaps) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    CopyMemory(pCaps, &self->Caps, sizeof(DSCBCAPS));

    return S_OK;
}

HRESULT DELTACALL dscb_get_current_position(dscb* self,
    LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    return dscbcb_get_current_position(self->Buffer, pdwCapturePosition, pdwReadPosition);
}

HRESULT DELTACALL dscb_get_format(dscb* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;
    const DWORD size = SIZEOFFORMATEX(self->Format);

    if (pwfxFormat != NULL) {
        if (size <= dwSizeAllocated) {
            CopyMemory(pwfxFormat, self->Format, min(size, dwSizeAllocated));
        }
        else {
            hr = E_INVALIDARG;
        }
    }

    if (pdwSizeWritten != NULL) {
        *pdwSizeWritten = size;
    }

    return hr;
}

HRESULT DELTACALL dscb_get_status(dscb* self, LPDWORD pdwStatus) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    *pdwStatus = self->Status;

    return S_OK;
}

HRESULT DELTACALL dscb_initialize(dscb* self, dsc* pDSC, LPCDSCBUFFERDESC pcDesc) {
    if (self->Instance != NULL) {
        return DSERR_ALREADYINITIALIZED;
    }

    self->Instance = pDSC;

    self->Caps.dwFlags = pcDesc->dwFlags;

    // TODO Set DSCBCAPS_WAVEMAPPED caps when the format is not one of the standard ones?

    self->Caps.dwBufferBytes = pcDesc->dwBufferBytes;
    CopyMemory(self->Format, pcDesc->lpwfxFormat, SIZEOFFORMAT(pcDesc->lpwfxFormat));

    return dscbcb_create(self->Allocator, self->Caps.dwBufferBytes, &self->Buffer);
}

HRESULT DELTACALL dscb_lock(dscb* self, DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;
    DWORD lockable = 0;

    if (FAILED(hr = dscbcb_get_lockable_length(self->Buffer, &lockable))) {
        goto exit;
    }

    if (dwFlags & DSCBLOCK_ENTIREBUFFER) {
        dwBytes = lockable;
    }

    if (dwBytes == 0
        || self->Caps.dwBufferBytes < dwOffset || lockable < dwBytes) {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (SUCCEEDED(hr = dscbcb_lock(self->Buffer, dwOffset, dwBytes,
        ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2))) {
        return hr;
    }

exit:

    if (ppvAudioPtr1 != NULL) {
        *ppvAudioPtr1 = NULL;
    }

    if (pdwAudioBytes1 != NULL) {
        *pdwAudioBytes1 = 0;
    }

    if (ppvAudioPtr2 != NULL) {
        *ppvAudioPtr2 = NULL;
    }

    if (pdwAudioBytes2 != NULL) {
        *pdwAudioBytes2 = 0;
    }

    return hr;
}

HRESULT DELTACALL dscb_start(dscb* self, DWORD dwFlags) {
    return E_NOTIMPL;
}

HRESULT DELTACALL dscb_stop(dscb* self) {
    return E_NOTIMPL;
}

HRESULT DELTACALL dscb_unlock(dscb* self, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (pvAudioPtr1 == NULL && pvAudioPtr2 == NULL) {
        return S_OK;
    }

    if (pvAudioPtr1 == NULL) {
        return E_INVALIDARG;
    }

    if (pvAudioPtr2 == NULL && dwAudioBytes2 != 0) {
        return E_INVALIDARG;
    }

    return dscbcb_unlock(self->Buffer, pvAudioPtr1, pvAudioPtr2);
}
