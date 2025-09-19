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
#include "rcm.h"

typedef struct dsbcb {
    allocator*          Allocator;
    CRITICAL_SECTION    Lock;

    rcm*                Buffer;

    DWORD               ReadPosition;
    DWORD               WritePosition;

    dsbcblc*            Locks;
} dsbcb;

HRESULT DELTACALL dsbcb_locks_overlap(DWORD dwStart1, DWORD dwEnd1, DWORD dwStart2, DWORD dwEnd2);

HRESULT DELTACALL dsbcb_create(allocator* pAlloc, DWORD dwBytes, dsbcb** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsbcb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsbcb), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = rcm_create(pAlloc, dwBytes, &instance->Buffer))) {
            if (SUCCEEDED(hr = dsbcblc_create(pAlloc, &instance->Locks))) {
                InitializeCriticalSection(&instance->Lock);

                *ppOut = instance;

                return S_OK;
            }

            rcm_remove_ref(instance->Buffer);
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL dsbcb_release(dsbcb* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    dsbcblc_release(self->Locks);

    rcm_remove_ref(self->Buffer);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsbcb_duplicate(dsbcb* self, dsbcb** ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsbcb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(self->Allocator, sizeof(dsbcb), &instance))) {
        instance->Allocator = self->Allocator;

        if (SUCCEEDED(hr = dsbcblc_create(self->Allocator, &instance->Locks))) {
            InitializeCriticalSection(&instance->Lock);

            instance->Buffer = self->Buffer;
            rcm_add_ref(instance->Buffer);

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(self->Allocator, instance);
    }

    return hr;
}

HRESULT DELTACALL dsbcb_get_current_position(dsbcb* self, LPDWORD pdwReadBytes, LPDWORD pdwWriteBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwReadBytes == NULL && pdwWriteBytes == NULL) {
        return E_INVALIDARG;
    }

    if (pdwReadBytes != NULL) {
        *pdwReadBytes = self->ReadPosition;
    }

    if (pdwWriteBytes != NULL) {
        *pdwWriteBytes = self->WritePosition;
    }

    return S_OK;
}

HRESULT DELTACALL dsbcb_set_current_position(dsbcb* self, DWORD dwReadBytes, DWORD dwWriteBytes, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    DWORD size = 0;

    if (SUCCEEDED(hr = rcm_get_size(self->Buffer, &size))) {
        if (dwFlags & DSBCB_SETPOSITION_WRAP) {
            dwReadBytes = dwReadBytes % size;
            dwWriteBytes = dwWriteBytes % size;
        }

        if (size < dwReadBytes || size < dwWriteBytes) {
            return E_INVALIDARG;
        }

        self->ReadPosition = dwReadBytes;
        self->WritePosition = dwWriteBytes;
    }

    return hr;
}

HRESULT DELTACALL dsbcb_get_size(dsbcb* self, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    return rcm_get_size(self->Buffer, pdwBytes);
}

HRESULT DELTACALL dsbcb_get_lockable_size(dsbcb* self, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD size = 0;

    if (SUCCEEDED(hr = rcm_get_size(self->Buffer, &size))) {
        *pdwBytes = self->ReadPosition < self->WritePosition
            ? size + self->ReadPosition - self->WritePosition
            : size - self->ReadPosition + self->WritePosition;
    }

    return hr;
}

HRESULT DELTACALL dsbcb_lock(dsbcb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    DWORD size = 0;

    if (FAILED(hr = rcm_get_size(self->Buffer, &size))) {
        return hr;
    }

    if (dwBytes == 0 || size < dwOffset || size < dwBytes) {
        return E_INVALIDARG;
    }

    DWORD lockable = 0;

    if (FAILED(hr = dsbcb_get_lockable_size(self, &lockable))) {
        return hr;
    }

    if (lockable < dwBytes) {
        return E_INVALIDARG;
    }

    LPVOID buffer = NULL;

    if (FAILED(hr = rcm_get_data(self->Buffer, &buffer))) {
        return hr;
    }

    const DWORD wrapped = size < dwOffset + dwBytes
        ? dwOffset + dwBytes - size : 0;

    dsbcbl lock;
    ZeroMemory(&lock, sizeof(dsbcbl));

    lock.Offset = dwOffset;
    lock.Size = dwBytes;
    lock.Audio1 = (LPVOID)((size_t)buffer + dwOffset);
    lock.AudioSize1 = dwBytes - wrapped;

    if (wrapped != 0) {
        lock.Audio2 = buffer;

        if (pdwAudioBytes2 != NULL) {
            lock.AudioSize2 = wrapped;
        }
    }

    EnterCriticalSection(&self->Lock);

    const DWORD count = dsbcblc_get_count(self->Locks);

    for (DWORD i = 0; i < count; i++) {
        dsbcbl* l = NULL;

        if (SUCCEEDED(dsbcblc_get_item(self->Locks, i, &l))) {
            // Match
            if (l->Audio1 == lock.Audio1 && l->Audio2 == lock.Audio2) {
                LeaveCriticalSection(&self->Lock);
                return E_INVALIDARG;
            }

            // Overlap
            if (SUCCEEDED(dsbcb_locks_overlap(l->Offset, l->Offset + l->Size, lock.Offset, lock.Offset + lock.Size))) {
                LeaveCriticalSection(&self->Lock);
                return E_INVALIDARG;
            }
        }
    }

    dsbcblc_add_item(self->Locks, &lock);

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

HRESULT DELTACALL dsbcb_unlock(dsbcb* self, LPVOID pvAudioPtr1, LPVOID pvAudioPtr2) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pvAudioPtr1 == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    const DWORD count = dsbcblc_get_count(self->Locks);

    for (DWORD i = 0; i < count; i++) {
        dsbcbl* l = NULL;

        if (SUCCEEDED(dsbcblc_get_item(self->Locks, i, &l))) {
            if (l->Audio1 == pvAudioPtr1 && l->Audio2 == pvAudioPtr2) {
                dsbcblc_remove_item(self->Locks, i);
                LeaveCriticalSection(&self->Lock);
                return S_OK;
            }
        }
    }

    LeaveCriticalSection(&self->Lock);

    return E_INVALIDARG;
}

HRESULT DELTACALL dsbcb_read(dsbcb* self, DWORD dwBytes, LPVOID pData, LPDWORD pdwBytesRead) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pData == NULL && pdwBytesRead == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD size = 0;

    if (FAILED(hr = rcm_get_size(self->Buffer, &size))) {
        return hr;
    }

    const DWORD maximum = self->ReadPosition < self->WritePosition
        ? self->WritePosition - self->ReadPosition
        : size + self->WritePosition - self->ReadPosition;

    if (maximum < dwBytes) {
        dwBytes = maximum;
    }

    LPVOID buffer = NULL;

    if (SUCCEEDED(hr = rcm_get_data(self->Buffer, &buffer))) {
        EnterCriticalSection(&self->Lock);

        if (pData != NULL) {
            const DWORD read = min(dwBytes, size - self->ReadPosition);
            CopyMemory(pData, (LPVOID)((size_t)buffer + self->ReadPosition), read);

            if (read < dwBytes) {
                CopyMemory((LPVOID)((size_t)pData + read), buffer, dwBytes - read);
            }
        }

        LeaveCriticalSection(&self->Lock);

        if (pdwBytesRead != NULL) {
            *pdwBytesRead = dwBytes;
        }
    }

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsbcb_locks_overlap(DWORD dwStart1, DWORD dwEnd1, DWORD dwStart2, DWORD dwEnd2) {
    const DWORD l1min = (dwStart1 < dwEnd1) ? dwStart1 : dwEnd1;
    const DWORD l1max = (dwStart1 > dwEnd1) ? dwStart1 : dwEnd1;
    const DWORD l2min = (dwStart2 < dwEnd2) ? dwStart2 : dwEnd2;
    const DWORD l2max = (dwStart2 > dwEnd2) ? dwStart2 : dwEnd2;

    return (l1max <= l2min || l2max <= l1min) ? E_FAIL : S_OK;
}
