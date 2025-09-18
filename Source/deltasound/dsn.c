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

#include "dsb.h"
#include "dsn.h"

#define IS_VALID_HANDLE(h)  (((h) != NULL) && ((h) != INVALID_HANDLE_VALUE))

HRESULT DELTACALL dsn_allocate(allocator* pAlloc, dsn** ppOut);

HRESULT DELTACALL dsn_validate_notification_positions(dsn* pDSN, DWORD dwPositionNotifies, LPDSBPOSITIONNOTIFY pPositionNotifies);

HRESULT DELTACALL dsn_create(allocator* pAlloc, REFIID riid, dsn** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsn* instance = NULL;

    if (SUCCEEDED(hr = dsn_allocate(pAlloc, &instance))) {
        CopyMemory(&instance->ID, riid, sizeof(GUID));
        InitializeCriticalSection(&instance->Lock);

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            *ppOut = instance;

            return S_OK;
        }

        dsn_release(instance);
    }

    return hr;
}

VOID DELTACALL dsn_release(dsn* self) {
    if (self == NULL) { return; }

    for (DWORD i = 0; i < intfc_get_count(self->Interfaces); i++) {
        idsn* instance = NULL;
        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idsn_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    DeleteCriticalSection(&self->Lock);

    if (self->Notifications != NULL) {
        allocator_free(self->Allocator, self->Notifications);
    }


    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsn_query_interface(dsn* self, REFIID riid, LPVOID* ppOut) {
    // TODO synchronization

    idsn* instance = NULL;

    if (SUCCEEDED(intfc_query_item(self->Interfaces, riid, &instance))) {
        idsn_add_ref(instance);
        *ppOut = instance;
        return S_OK;
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSoundNotify, riid)) {
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr = idsn_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dsn_add_ref(self, instance))) {
                instance->Instance = self;
                *ppOut = instance;
                return S_OK;
            }

            idsn_release(instance);
        }

        return hr;
    }
    else if (IsEqualIID(&IID_IDirectSoundBuffer, riid)
        || IsEqualIID(&IID_IKsPropertySet, riid)) {
        return dsb_query_interface(self->Instance, riid, ppOut);
    }

    return E_NOINTERFACE;
}

HRESULT DELTACALL dsn_add_ref(dsn* self, idsn* pIDSN) {
    return intfc_add_item(self->Interfaces, &pIDSN->ID, pIDSN);
}

HRESULT DELTACALL dsn_remove_ref(dsn* self, idsn* pIDSN) {
    intfc_remove_item(self->Interfaces, &pIDSN->ID);

    // TODO NOT IMPLEMENTED
    // what to do when the notifications are released? Remove the notifications?

    return S_OK;
}

HRESULT DELTACALL dsn_get_notification_positions(dsn* self, LPDWORD pdwPositionNotifies, LPCDSBPOSITIONNOTIFY* ppcPositionNotifies) {
    if (pdwPositionNotifies == NULL) {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&self->Lock);

    *pdwPositionNotifies = self->NotificationCount;

    if (ppcPositionNotifies != NULL) {
        *ppcPositionNotifies = self->Notifications;
    }

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

HRESULT DELTACALL dsn_set_notification_positions(dsn* self, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies) {
    if (!(self->Instance->Caps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY)) {
        return DSERR_CONTROLUNAVAIL;
    }

    if (self->Instance->Status & DSBSTATUS_PLAYING) {
        return DSERR_INVALIDCALL;
    }

    if (dwPositionNotifies == 0) {
        self->NotificationCount = 0;
        return S_OK;
    }

    if (pcPositionNotifies == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    const DWORD length = dwPositionNotifies * sizeof(DSBPOSITIONNOTIFY);
    LPDSBPOSITIONNOTIFY items = NULL;

    if (SUCCEEDED(hr = allocator_allocate(self->Allocator, length, &items))) {
        CopyMemory(items, pcPositionNotifies, length);

        if (SUCCEEDED(hr = dsn_validate_notification_positions(self, dwPositionNotifies, items))) {
            EnterCriticalSection(&self->Lock);

            self->NotificationCount = dwPositionNotifies;
            LPVOID notes = InterlockedExchangePointer(&self->Notifications, items);

            if (notes != NULL) {
                allocator_free(self->Allocator, notes);
            }

            LeaveCriticalSection(&self->Lock);
        }
    }

exit:

    if (items != NULL) {
        allocator_free(self->Allocator, items);
    }

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsn_allocate(allocator* pAlloc, dsn** ppOut) {
    HRESULT hr = S_OK;
    dsn* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsn), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;

    }

    return hr;
}

INT CDECLCALL dsn_position_notify_compare(LPCDSBPOSITIONNOTIFY a, LPCDSBPOSITIONNOTIFY b) {
    if (a->dwOffset < b->dwOffset) { return -1; }

    if (a->dwOffset > b->dwOffset) { return 1; }

    return 0;
}

HRESULT DELTACALL dsn_validate_notification_positions(dsn* self, DWORD dwPositionNotifies, LPDSBPOSITIONNOTIFY pPositionNotifies) {
    qsort(pPositionNotifies, dwPositionNotifies,
        sizeof(DSBPOSITIONNOTIFY), dsn_position_notify_compare);

    for (DWORD i = 0; i < dwPositionNotifies - 1; i++) {
        if (pPositionNotifies[i].dwOffset == pPositionNotifies[i + 1].dwOffset) {
            return E_INVALIDARG;
        }
    }

    const DWORD maximum = self->Instance->Caps.dwBufferBytes;

    for (DWORD i = 0; i < dwPositionNotifies; i++) {
        if (!IS_VALID_HANDLE(pPositionNotifies[i].hEventNotify)) {
            return E_INVALIDARG;
        }

        if (maximum < pPositionNotifies[i].dwOffset
            && pPositionNotifies[i].dwOffset != DSBPN_OFFSETSTOP) {
            return E_INVALIDARG;
        }
    }

    return S_OK;
}
