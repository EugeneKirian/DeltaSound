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
#include "dsbn.h"

HRESULT DELTACALL dsbn_validate_notifications(dsbn* pDSN, DWORD dwPositionNotifies, LPDSBPOSITIONNOTIFY pPositionNotifies);

HRESULT DELTACALL dsbn_create(allocator* pAlloc, REFIID riid, dsbn** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsbn* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsbn), &instance))) {
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

VOID DELTACALL dsbn_release(dsbn* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    const DWORD count = intfc_get_count(self->Interfaces);

    for (DWORD i = 0; i < count; i++) {
        idsbn* instance = NULL;

        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idsbn_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    if (self->Notifications != NULL) {
        allocator_free(self->Allocator, self->Notifications);
    }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsbn_query_interface(dsbn* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    {
        idsbn* instance = NULL;

        if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
            idsbn_add_ref(instance);

            *ppOut = instance;

            goto exit;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSoundNotify, riid)) {
        idsbn* instance = NULL;

        if (SUCCEEDED(hr = idsbn_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dsbn_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            idsbn_release(instance);
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

HRESULT DELTACALL dsbn_add_ref(dsbn* self, idsbn* pIDSBN) {
    return intfc_add_item(self->Interfaces, &pIDSBN->ID, pIDSBN);
}

HRESULT DELTACALL dsbn_remove_ref(dsbn* self, idsbn* pIDSBN) {
    return intfc_remove_item(self->Interfaces, &pIDSBN->ID);
}

HRESULT DELTACALL dsbn_get_notification_positions(dsbn* self, LPDWORD pdwPositionNotifies, LPCDSBPOSITIONNOTIFY* ppcPositionNotifies) {
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

HRESULT DELTACALL dsbn_set_notification_positions(dsbn* self, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies) {
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
    LPDSBPOSITIONNOTIFY notes = NULL;
    const DWORD size = dwPositionNotifies * sizeof(DSBPOSITIONNOTIFY);

    if (SUCCEEDED(hr = allocator_allocate(self->Allocator, size, &notes))) {
        CopyMemory(notes, pcPositionNotifies, size);

        if (SUCCEEDED(hr = dsbn_validate_notifications(self, dwPositionNotifies, notes))) {
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

HRESULT DELTACALL dsbn_validate_notifications(dsbn* self, DWORD dwPositionNotifies, LPDSBPOSITIONNOTIFY pPositionNotifies) {
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
