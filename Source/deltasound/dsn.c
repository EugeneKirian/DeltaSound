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

HRESULT DELTACALL dsn_allocate(allocator* pAlloc, dsn** ppOut);

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

    *pdwPositionNotifies = self->NotificationCount;

    if (ppcPositionNotifies != NULL) {
        *ppcPositionNotifies = self->Notifications;
    }

    return S_OK;
}

HRESULT DELTACALL dsn_set_notification_positions(dsn* self, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies) {
    HRESULT hr = S_OK;

    DWORD status = 0;
    if (FAILED(hr = dsb_get_status(self->Instance, &status))) {
        return hr;
    }

    if (!(status & DSBSTATUS_PLAYING)) {
        return DSERR_INVALIDCALL;
    }

    EnterCriticalSection(&self->Lock);

    if (self->NotificationCount < dwPositionNotifies) {
        if (FAILED(hr = allocator_reallocate(self->Allocator,
            self->Notifications, dwPositionNotifies * sizeof(DSBPOSITIONNOTIFY), &self->Notifications))) {
            goto exit;
        }
    }

    self->NotificationCount = dwPositionNotifies;
    CopyMemory(self->Notifications, pcPositionNotifies, dwPositionNotifies * sizeof(DSBPOSITIONNOTIFY));

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsn_allocate(allocator* pAlloc, dsn** ppOut) {
    HRESULT hr = S_OK;
    dsn* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsn), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, 0, &instance->Notifications))) {
            *ppOut = instance;
            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}