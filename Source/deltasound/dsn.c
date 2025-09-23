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

HRESULT DELTACALL dsn_validate_notifications(dsn* pDSN, DWORD dwPositionNotifies, LPDSBPOSITIONNOTIFY pPositionNotifies);

HRESULT DELTACALL dsn_create(allocator* pAlloc, REFIID riid, dsn** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsn* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsn), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, riid, sizeof(GUID));

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL dsn_release(dsn* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    const DWORD count = intfc_get_count(self->Interfaces);

    for (DWORD i = 0; i < count; i++) {
        idsn* instance = NULL;

        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idsn_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    if (self->Notifications != NULL) {
        allocator_free(self->Allocator, self->Notifications);
    }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsn_query_interface(dsn* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    {
        idsn* instance = NULL;

        if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
            idsn_add_ref(instance);

            *ppOut = instance;

            goto exit;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSoundNotify, riid)) {
        idsn* instance = NULL;

        if (SUCCEEDED(hr = idsn_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dsn_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            idsn_release(instance);
        }
    }
    else if (IsEqualIID(&IID_IDirectSoundBuffer, riid)
        || IsEqualIID(&IID_IKsPropertySet, riid)) {
        hr = dsb_query_interface(self->Instance, riid, ppOut);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL dsn_add_ref(dsn* self, idsn* pIDSN) {
    return intfc_add_item(self->Interfaces, &pIDSN->ID, pIDSN);
}

HRESULT DELTACALL dsn_remove_ref(dsn* self, idsn* pIDSN) {
    return intfc_remove_item(self->Interfaces, &pIDSN->ID);
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
    LPDSBPOSITIONNOTIFY notes = NULL;

    if (SUCCEEDED(hr = allocator_allocate(self->Allocator, length, &notes))) {
        CopyMemory(notes, pcPositionNotifies, length);

        if (SUCCEEDED(hr = dsn_validate_notifications(self, dwPositionNotifies, notes))) {
            EnterCriticalSection(&self->Lock);

            if (self->Notifications != NULL) {
                allocator_free(self->Allocator, self->Notifications);
            }

            self->Notifications = notes;
            self->NotificationCount = dwPositionNotifies;

            LeaveCriticalSection(&self->Lock);

            return S_OK;
        }
    }

    if (notes != NULL) {
        allocator_free(self->Allocator, notes);
    }

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsn_validate_notifications(dsn* self, DWORD dwPositionNotifies, LPDSBPOSITIONNOTIFY pPositionNotifies) {
    // Sort notifications in ascending order.
    for (DWORD i = 0; i < dwPositionNotifies; i++) {
        for (DWORD j = 0; j < dwPositionNotifies; j++) {
            if (i == j) {
                continue;
            }

            if (pPositionNotifies[i].dwOffset < pPositionNotifies[j].dwOffset) {
                DSBPOSITIONNOTIFY t = pPositionNotifies[j];
                pPositionNotifies[j] = pPositionNotifies[i];
                pPositionNotifies[i] = t;
            }
        }
    }

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
