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

#include "cf.h"
#include "iprvt.h"
#include "prvt.h"

HRESULT DELTACALL iprvt_get(iprvt*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned);
HRESULT DELTACALL iprvt_set(iprvt*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength);
HRESULT DELTACALL iprvt_query_support(iprvt*,
    REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport);

typedef struct iprvt_vft {
    LPIPRVTQUERYINTERFACE    QueryInterface;
    LPIPRVTADDREF            AddRef;
    LPIPRVTRELEASE           Release;
    LPIPRVTGET               Get;
    LPIPRVTSET               Set;
    LPIPRVTQUERYSUPPORT      QuerySupport;
} iprvt_vft;

const static iprvt_vft iprvt_self = {
    iprvt_query_interface,
    iprvt_add_ref,
    iprvt_remove_ref,
    iprvt_get,
    iprvt_set,
    iprvt_query_support
};

HRESULT DELTACALL iprvt_create(allocator* pAlloc, REFIID riid, iprvt** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    iprvt* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(iprvt), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &iprvt_self;
        CopyMemory(&instance->ID, riid, sizeof(IID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL iprvt_release(iprvt* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL iprvt_query_interface(iprvt* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return prvt_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL iprvt_add_ref(iprvt* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL iprvt_remove_ref(iprvt* self) {
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
            prvt_remove_ref(self->Instance, self);
        }

        iprvt_release(self);
    }

    return result;
}

HRESULT DELTACALL iprvt_get(iprvt* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (rguidPropSet == NULL || pulBytesReturned == NULL) {
        return E_INVALIDARG;
    }

    return prvt_get(self->Instance, rguidPropSet, ulId,
        pInstanceData, ulInstanceLength, pPropertyData, ulDataLength, pulBytesReturned);
}

HRESULT DELTACALL iprvt_set(iprvt* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (rguidPropSet == NULL) {
        return E_INVALIDARG;
    }

    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL iprvt_query_support(iprvt* self,
    REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (rguidPropSet == NULL || pulTypeSupport == NULL) {
        return E_INVALIDARG;
    }

    return prvt_query_support(self->Instance, rguidPropSet, ulId, pulTypeSupport);
}
