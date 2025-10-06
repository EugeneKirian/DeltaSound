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

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WdsblcANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WdsblcANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "cblc.h"
#include "dscbcb.h"

typedef struct dscbcb {
    allocator*          Allocator;
    CRITICAL_SECTION    Lock;

    LPVOID              Buffer;
    DWORD               Size;

    DWORD               CapturePosition;
    DWORD               ReadPosition;

    cblc*               Locks;
} dscbcb;

HRESULT DELTACALL dscbcb_locks_overlap(DWORD dwStart1, DWORD dwEnd1, DWORD dwStart2, DWORD dwEnd2);

HRESULT DELTACALL dscbcb_create(allocator* pAlloc, DWORD dwBytes, dscbcb** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dscbcb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dscbcb), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, dwBytes, &instance->Buffer))) {
            instance->Size = dwBytes;

            if (SUCCEEDED(hr = cblc_create(pAlloc, &instance->Locks))) {
                InitializeCriticalSection(&instance->Lock);

                *ppOut = instance;

                return S_OK;
            }

            allocator_free(pAlloc, instance->Buffer);
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL dscbcb_release(dscbcb* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    cblc_release(self->Locks);

    allocator_free(self->Allocator, self->Buffer);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dscbcb_get_current_position(dscbcb* self,
    LPDWORD pdwCaptureBytes, LPDWORD pdwReadBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwCaptureBytes == NULL && pdwReadBytes == NULL) {
        return E_INVALIDARG;
    }

    if (pdwCaptureBytes != NULL) {
        *pdwCaptureBytes = self->CapturePosition;
    }

    if (pdwReadBytes != NULL) {
        *pdwReadBytes = self->ReadPosition;
    }

    return S_OK;
}

HRESULT DELTACALL dscbcb_set_current_position(dscbcb* self,
    DWORD dwCaptureBytes, DWORD dwReadBytes, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwFlags & DSCBCB_SETPOSITION_LOOPING) {
        dwCaptureBytes = dwCaptureBytes % self->Size;
        dwReadBytes = dwReadBytes % self->Size;
    }

    if (self->Size < dwCaptureBytes || self->Size < dwReadBytes) {
        return E_INVALIDARG;
    }

    self->CapturePosition = dwCaptureBytes;
    self->ReadPosition = dwReadBytes;

    return S_OK;
}

HRESULT DELTACALL dscbcb_get_length(dscbcb* self, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    *pdwBytes = self->Size;

    return S_OK;
}

HRESULT DELTACALL dscbcb_get_lockable_length(dscbcb* self, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    *pdwBytes = self->CapturePosition < self->ReadPosition
        ? self->Size + self->CapturePosition - self->ReadPosition
        : self->Size - self->CapturePosition + self->ReadPosition;

    return S_OK;
}


HRESULT DELTACALL dscbcb_lock(dscbcb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2) {
    if (self == NULL) {
        return E_POINTER;
    }
    
    if (dwBytes == 0 || self->Size < dwOffset || self->Size < dwBytes) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD lockable = 0;

    if (FAILED(hr = dscbcb_get_lockable_length(self, &lockable))) {
        return hr;
    }

    if (lockable < dwBytes) {
        return E_INVALIDARG;
    }

    const DWORD wrapped = self->Size < dwOffset + dwBytes
        ? dwOffset + dwBytes - self->Size : 0;

    cbl lock;
    ZeroMemory(&lock, sizeof(cbl));

    lock.Offset = dwOffset;
    lock.Size = dwBytes;
    lock.Audio1 = (LPVOID)((size_t)self->Buffer + dwOffset);
    lock.AudioSize1 = dwBytes - wrapped;

    if (wrapped != 0) {
        lock.Audio2 = self->Buffer;

        if (pdwAudioBytes2 != NULL) {
            lock.AudioSize2 = wrapped;
        }
    }

    EnterCriticalSection(&self->Lock);

    const DWORD count = cblc_get_count(self->Locks);

    for (DWORD i = 0; i < count; i++) {
        cbl* l = NULL;

        if (SUCCEEDED(cblc_get_item(self->Locks, i, &l))) {
            // Match
            if (l->Audio1 == lock.Audio1 && l->Audio2 == lock.Audio2) {
                LeaveCriticalSection(&self->Lock);
                return E_INVALIDARG;
            }

            // Overlap
            if (SUCCEEDED(dscbcb_locks_overlap(l->Offset, l->Offset + l->Size, lock.Offset, lock.Offset + lock.Size))) {
                LeaveCriticalSection(&self->Lock);
                return E_INVALIDARG;
            }
        }
    }

    cblc_add_item(self->Locks, &lock);

    LeaveCriticalSection(&self->Lock);

    *ppvAudioPtr1 = lock.Audio1;
    *pdwAudioBytes1 = lock.AudioSize1;

    if (ppvAudioPtr2 != NULL) {
        *ppvAudioPtr2 = lock.Audio2;
    }

    if (pdwAudioBytes2 != NULL) {
        *pdwAudioBytes2 = lock.AudioSize2;
    }

    return S_OK;
}

HRESULT DELTACALL dscbcb_unlock(dscbcb* self, LPVOID pvAudioPtr1, LPVOID pvAudioPtr2) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pvAudioPtr1 == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    const DWORD count = cblc_get_count(self->Locks);

    for (DWORD i = 0; i < count; i++) {
        cbl* l = NULL;

        if (SUCCEEDED(cblc_get_item(self->Locks, i, &l))) {
            if (l->Audio1 == pvAudioPtr1 && l->Audio2 == pvAudioPtr2) {
                cblc_remove_item(self->Locks, i);

                LeaveCriticalSection(&self->Lock);

                return S_OK;
            }
        }
    }

    LeaveCriticalSection(&self->Lock);

    return E_INVALIDARG;
}

HRESULT DELTACALL dscbcb_read(dscbcb* self, DWORD dwBytes, LPVOID pData, LPDWORD pdwBytes, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pData == NULL && pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    if (!(dwFlags & DSCBCB_READ_LOOPING)) {
        dwBytes = min(dwBytes, self->Size - self->CapturePosition);
    }

    EnterCriticalSection(&self->Lock);

    if (pData != NULL) {
        DWORD bytes = min(dwBytes, self->Size - self->ReadPosition);

        CopyMemory(pData, (LPVOID)((size_t)self->Buffer + self->ReadPosition), bytes);

        DWORD offset = bytes;
        DWORD pending = dwBytes - bytes;

        while (pending != 0) {
            bytes = min(pending, self->Size);

            CopyMemory((LPVOID)((size_t)pData + offset), self->Buffer, bytes);

            pending -= bytes;
            offset += bytes;
        }
    }

    if (pdwBytes != NULL) {
        *pdwBytes = dwBytes;
    }

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dscbcb_locks_overlap(DWORD dwStart1, DWORD dwEnd1, DWORD dwStart2, DWORD dwEnd2) {
    const DWORD l1min = (dwStart1 < dwEnd1) ? dwStart1 : dwEnd1;
    const DWORD l1max = (dwStart1 > dwEnd1) ? dwStart1 : dwEnd1;
    const DWORD l2min = (dwStart2 < dwEnd2) ? dwStart2 : dwEnd2;
    const DWORD l2max = (dwStart2 > dwEnd2) ? dwStart2 : dwEnd2;

    return (l1max <= l2min || l2max <= l1min) ? E_FAIL : S_OK;
}
