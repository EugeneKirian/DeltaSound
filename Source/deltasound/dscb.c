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


HRESULT DELTACALL dscb_create(allocator* pAlloc, REFIID riid, dscb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dscb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dscb), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, riid, sizeof(GUID));

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(WAVEFORMATEX) /* TODO */, &instance->Format))) {
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

    // TODO self->Play = DSBPLAY_NONE;
    // TODO self->Status = DSBSTATUS_NONE;

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

    // TODO if (self->Buffer != NULL) {
    // TODO     dsbcb_release(self->Buffer);
    // TODO }

    allocator_free(self->Allocator, self->Format);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dscb_query_interface(dscb* self, REFIID riid, LPVOID* ppOut) {

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
