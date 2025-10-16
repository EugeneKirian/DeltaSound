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

#include "dsbps.h"
#include "idsbps.h"

HRESULT DELTACALL idsbps_get(idsbps*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned);
HRESULT DELTACALL idsbps_set(idsbps*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength);
HRESULT DELTACALL idsbps_query_support(idsbps*,
    REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport);

struct idsbps_vft {
    LPIDSBPSQUERYINTERFACE  QueryInterface;
    LPIDSBPSADDREF          AddRef;
    LPIDSBPSRELEASE         Release;
    LPIDSBPSGET             Get;
    LPIDSBPSSET             Set;
    LPIDSBPSQUERYSUPPORT    QuerySupport;
};

const static idsbps_vft idsbps_self = {
    idsbps_query_interface,
    idsbps_add_ref,
    idsbps_remove_ref,
    idsbps_get,
    idsbps_set,
    idsbps_query_support
};

HRESULT DELTACALL idsbps_create(allocator* pAlloc, REFIID riid, idsbps** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idsbps* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idsbps), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &idsbps_self;
        CopyMemory(&instance->ID, riid, sizeof(IID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idsbps_release(idsbps* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idsbps_query_interface(idsbps* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return dsbps_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL idsbps_add_ref(idsbps* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idsbps_remove_ref(idsbps* self) {
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
            dsbps_remove_ref(self->Instance, self);
        }

        idsbps_release(self);
    }

    return result;
}

HRESULT DELTACALL idsbps_get(idsbps* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsbps_set(idsbps* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsbps_query_support(idsbps* self,
    REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}
