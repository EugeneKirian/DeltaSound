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

#include "dsbn.h"
#include "idsbn.h"

HRESULT DELTACALL idsbn_set_notification_positions(idsn*, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies);

typedef struct idsbn_vft {
    LPIDSBNQUERYINTERFACE            QueryInterface;
    LPIDSBNADDREF                    AddRef;
    LPIDSBNRELEASE                   Release;
    LPIDSBNSETNOTIFICATIONPOSITIONS  SetNotificationPositions;
} idsbn_vft;

const static idsbn_vft idsbn_self = {
    idsbn_query_interface,
    idsbn_add_ref,
    idsbn_remove_ref,
    idsbn_set_notification_positions
};

HRESULT DELTACALL idsbn_create(allocator* pAlloc, REFIID riid, idsn** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idsn* instance = NULL;


    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idsn), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &idsbn_self;
        CopyMemory(&instance->ID, riid, sizeof(IID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idsbn_release(idsn* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idsbn_query_interface(idsn* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return dsbn_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL idsbn_add_ref(idsn* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idsbn_remove_ref(idsn* self) {
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
            dsbn_remove_ref(self->Instance, self);
        }

        idsbn_release(self);
    }

    return result;
}

HRESULT DELTACALL idsbn_set_notification_positions(idsn* self, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (DSBNOTIFICATIONS_MAX < dwPositionNotifies) {
        return E_INVALIDARG;
    }

    return dsbn_set_notification_positions(self->Instance, dwPositionNotifies, pcPositionNotifies);
}
