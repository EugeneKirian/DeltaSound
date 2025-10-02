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

#include "icf.h"
#include "cf.h"

HRESULT DELTACALL icf_create_instance(icf* self, LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppOut);
HRESULT DELTACALL icf_lock_server(icf* self, BOOL bLock);

struct icf_vft {
    LPICFQUERYINTERFACE         QueryInterface;
    LPICFADDREF                 AddRef;
    LPICFRELEASE                Release;
    LPCIFCREATEINSTANCE         CreateInstance;
    LPICFLOCKSERVER             LockServer;
};

const static icf_vft icf_self = {
    icf_query_interface,
    icf_add_ref,
    icf_remove_ref,
    icf_create_instance,
    icf_lock_server
};

HRESULT DELTACALL icf_create(allocator* pAlloc, REFIID riid, icf** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    icf* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(icf), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &icf_self;
        CopyMemory(&instance->ID, riid, sizeof(IID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL icf_release(icf* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL icf_query_interface(icf* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return cf_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL icf_add_ref(icf* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL icf_remove_ref(icf* self) {
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
            cf_remove_ref(self->Instance, self);
        }

        icf_release(self);
    }

    return result;
}

HRESULT DELTACALL icf_create_instance(icf* self, LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pUnkOuter != NULL) {
        return CLASS_E_NOAGGREGATION;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return cf_create_instance(self->Instance, riid, ppOut);
}

HRESULT DELTACALL icf_lock_server(icf* self, BOOL bLock) {
    if (self == NULL) {
        return E_POINTER;
    }

    return S_OK;
}
