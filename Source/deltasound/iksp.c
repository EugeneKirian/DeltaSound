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

#include "iksp.h"
#include "ksp.h"

HRESULT DELTACALL iksp_get(iksp*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned);
HRESULT DELTACALL iksp_set(iksp*,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength);
HRESULT DELTACALL iksp_query_support(iksp*, REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport);

struct iksp_vft {
    LPIKSPQUERYINTERFACE    QueryInterface;
    LPIKSPADDREF            AddRef;
    LPIKSPRELEASE           Release;
    LPIKSPGET               Get;
    LPIKSPSET               Set;
    LPIKSPQUERYSUPPORT      QuerySupport;
};

const static iksp_vft iksp_self = {
    iksp_query_interface,
    iksp_add_ref,
    iksp_remove_ref,
    iksp_get,
    iksp_set,
    iksp_query_support
};

HRESULT DELTACALL iksp_create(allocator* pAlloc, REFIID riid, iksp** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    iksp* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(iksp), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &iksp_self;
        CopyMemory(&instance->ID, riid, sizeof(GUID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL iksp_release(iksp* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL iksp_query_interface(iksp* self, REFIID riid, LPVOID* ppvObject) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppvObject == NULL) {
        return E_INVALIDARG;
    }

    return ksp_query_interface(self->Instance, riid, ppvObject);
}

ULONG DELTACALL iksp_add_ref(iksp* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL iksp_remove_ref(iksp* self) {
    if (self == NULL) {
        return 0;
    }

    if (self->RefCount == 0) {
        return 0;
    }

    LONG result = self->RefCount;

    if (InterlockedDecrement(&self->RefCount) <= 0) {
        result = self->RefCount = 0;

        if (self->Instance != NULL) {
            ksp_remove_ref(self->Instance, self);
        }

        iksp_release(self);
    }

    return result;
}

HRESULT DELTACALL iksp_get(iksp* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL iksp_set(iksp* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL iksp_query_support(iksp* self, REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}
