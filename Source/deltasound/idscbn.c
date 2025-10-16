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

#include "dscbn.h"
#include "idscbn.h"

HRESULT DELTACALL idscbn_set_notification_positions(idscbn*, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies);

typedef struct idscbn_vft {
    LPIDSCBNQUERYINTERFACE              QueryInterface;
    LPIDSCBNADDREF                      AddRef;
    LPIDSCBNRELEASE                     Release;
    LPIDSCBNSETNOTIFICATIONPOSITIONS    SetNotificationPositions;
} idscbn_vft;

const static idscbn_vft idscbn_self = {
    idscbn_query_interface,
    idscbn_add_ref,
    idscbn_remove_ref,
    idscbn_set_notification_positions
};

HRESULT DELTACALL idscbn_create(allocator* pAlloc, REFIID riid, idscbn** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idscbn* instance = NULL;


    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idscbn), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &idscbn_self;
        CopyMemory(&instance->ID, riid, sizeof(IID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idscbn_release(idscbn* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idscbn_query_interface(idscbn* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return dscbn_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL idscbn_add_ref(idscbn* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idscbn_remove_ref(idscbn* self) {
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
            dscbn_remove_ref(self->Instance, self);
        }

        idscbn_release(self);
    }

    return result;
}

HRESULT DELTACALL idscbn_set_notification_positions(idscbn* self, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (DSBNOTIFICATIONS_MAX < dwPositionNotifies) {
        return E_INVALIDARG;
    }

    return dscbn_set_notification_positions(self->Instance, dwPositionNotifies, pcPositionNotifies);
}
