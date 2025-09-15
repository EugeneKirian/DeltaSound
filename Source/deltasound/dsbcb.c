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

#include "dsbcb.h"
#include "dsbcblc.h"

typedef struct dsbcb {
    allocator*          Allocator;
    CRITICAL_SECTION    Lock;

    LPVOID              Buffer;
    DWORD               Size;

    DWORD               ReadPosition;
    DWORD               WritePosition;

    dsbcblc*            Locks;
} dsbcb;

HRESULT DELTACALL dsbcb_allocate(allocator* pAlloc, DWORD dwSize, dsbcb** ppOut);
HRESULT DELTACALL dsbcb_locks_overlap(DWORD dwStart1, DWORD dwEnd1, DWORD dwStart2, DWORD dwEnd2);

HRESULT DELTACALL dsbcb_create(allocator* pAlloc, DWORD dwBytes, dsbcb** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsbcb* instance = NULL;

    if (SUCCEEDED(hr = dsbcb_allocate(pAlloc, dwBytes, &instance))) {
        InitializeCriticalSection(&instance->Lock);

        if (SUCCEEDED(hr = dsbcblc_create(pAlloc, &instance->Locks))) {
            *ppOut = instance;
            return S_OK;
        }

        dsbcb_release(instance);
    }

    return hr;
}

VOID DELTACALL dsbcb_release(dsbcb* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    dsbcblc_release(self->Locks);

    allocator_free(self->Allocator, self->Buffer);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsbcb_get_read_position(dsbcb* self, LPDWORD pdwReadBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwReadBytes == NULL) {
        return E_INVALIDARG;
    }

    *pdwReadBytes = self->ReadPosition;

    return S_OK;
}

HRESULT DELTACALL dsbcb_get_write_position(dsbcb* self, LPDWORD pdwWriteBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwWriteBytes == NULL) {
        return E_INVALIDARG;
    }

    *pdwWriteBytes = self->WritePosition;

    return S_OK;
}

HRESULT DELTACALL dsbcb_set_read_position(dsbcb* self, DWORD dwReadBytes, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwFlags & DSBCB_SETPOSITION_WRAP) {
        dwReadBytes = dwReadBytes % self->Size;
    }

    if (self->Size < dwReadBytes) {
        return E_INVALIDARG;
    }

    self->ReadPosition = dwReadBytes;

    return S_OK;
}

HRESULT DELTACALL dsbcb_set_write_position(dsbcb* self, DWORD dwWriteBytes, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwFlags & DSBCB_SETPOSITION_WRAP) {
        dwWriteBytes = dwWriteBytes % self->Size;
    }

    if (self->Size < dwWriteBytes) {
        return E_INVALIDARG;
    }

    self->WritePosition = dwWriteBytes;

    return S_OK;
}

HRESULT DELTACALL dsbcb_get_size(dsbcb* self, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    *pdwBytes = self->Size;

    return S_OK;
}

HRESULT DELTACALL dsbcb_get_lockable_size(dsbcb* self, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    *pdwBytes = self->ReadPosition < self->WritePosition
        ? self->Size + self->ReadPosition - self->WritePosition
        : self->Size - self->ReadPosition + self->WritePosition;

    return S_OK;
}

HRESULT DELTACALL dsbcb_resize(dsbcb* self, DWORD dwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Size == dwBytes) {
        return S_OK;
    }

    if (dwBytes < self->Size) {
        // TODO Support decrease in size.
        // Need to check active locks and both read and write positions.
        return E_NOTIMPL;
    }

    HRESULT hr = S_OK;
    EnterCriticalSection(&self->Lock);

    if (self->Size < dwBytes) {
        if (SUCCEEDED(hr = allocator_reallocate(self->Allocator,
            self->Buffer, dwBytes, &self->Buffer))) {
            self->Size = dwBytes;
        }
    }

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL dsbcb_lock(dsbcb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwBytes == 0 || self->Size < dwOffset || self->Size < dwBytes) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD lockable = 0;

    if (FAILED(hr = dsbcb_get_lockable_size(self, &lockable))) {
        return hr;
    }

    if (lockable < dwBytes) {
        return E_INVALIDARG;
    }

    const DWORD wrapped = self->Size < dwOffset + dwBytes
        ? dwOffset + dwBytes - self->Size : 0;

    dsbcbl lock;
    ZeroMemory(&lock, sizeof(dsbcbl));

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

    for (UINT i = 0; i < dsbcblc_get_count(self->Locks); i++) {
        dsbcbl* l = NULL;
        if (SUCCEEDED(dsbcblc_get_item(self->Locks, i, &l))) {
            // Match
            if (l->Audio1 == lock.Audio1 && l->Audio2 == lock.Audio2) {
                return E_INVALIDARG;
            }

            // Overlap
            if (SUCCEEDED(dsbcb_locks_overlap(l->Offset, l->Offset + l->Size, lock.Offset, lock.Offset + lock.Size))) {
                return E_INVALIDARG;
            }
        }
    }

    dsbcblc_add_item(self->Locks, &lock);

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

HRESULT DELTACALL dsbcb_unlock(dsbcb* self, LPVOID pvAudioPtr1, LPVOID pvAudioPtr2) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pvAudioPtr1 == NULL) {
        return E_INVALIDARG;
    }

    for (DWORD i = 0; i < dsbcblc_get_count(self->Locks); i++) {
        dsbcbl* l = NULL;
        if (SUCCEEDED(dsbcblc_get_item(self->Locks, i, &l))) {
            if (l->Audio1 == pvAudioPtr1 && l->Audio2 == pvAudioPtr2) {
                dsbcblc_remove_item(self->Locks, i);
                return S_OK;
            }
        }
    }

    return E_INVALIDARG;
}

HRESULT DELTACALL dsbcb_read(dsbcb* self, DWORD dwBytes, LPVOID pData, LPDWORD pdwBytesRead) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pData == NULL && pdwBytesRead == NULL) {
        return E_INVALIDARG;
    }

    const DWORD maximum = self->ReadPosition < self->WritePosition
        ? self->WritePosition - self->ReadPosition
        : self->Size + self->WritePosition - self->ReadPosition;

    if (maximum < dwBytes) {
        dwBytes = maximum;
    }

    if (pData != NULL) {
        const DWORD read = min(dwBytes, self->Size - self->ReadPosition);
        CopyMemory(pData, (LPVOID)((size_t)self->Buffer + self->ReadPosition), read);

        if (read < dwBytes) {
            CopyMemory((LPVOID)((size_t)pData + read), self->Buffer, dwBytes - read);
        }
    }

    if (pdwBytesRead != NULL) {
        *pdwBytesRead = dwBytes;
    }

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsbcb_allocate(allocator* pAlloc, DWORD dwBytes, dsbcb** ppOut) {
    HRESULT hr = S_OK;
    dsbcb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsbcb), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, dwBytes, &instance->Buffer))) {
            *ppOut = instance;
            return S_OK;
        }

        allocator_free(pAlloc, instance->Buffer);
        allocator_free(pAlloc, instance);
    }

    return hr;
}

HRESULT DELTACALL dsbcb_locks_overlap(DWORD dwStart1, DWORD dwEnd1, DWORD dwStart2, DWORD dwEnd2) {
    const DWORD l1min = (dwStart1 < dwEnd1) ? dwStart1 : dwEnd1;
    const DWORD l1max = (dwStart1 > dwEnd1) ? dwStart1 : dwEnd1;
    const DWORD l2min = (dwStart2 < dwEnd2) ? dwStart2 : dwEnd2;
    const DWORD l2max = (dwStart2 > dwEnd2) ? dwStart2 : dwEnd2;

    return (l1max <= l2min || l2max <= l1min) ? E_FAIL : S_OK;
}
