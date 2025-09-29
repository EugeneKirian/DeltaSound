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

#include "dssl.h"
#include "idssl.h"

HRESULT DELTACALL idssl_get_all_parameters(idssl* self, LPDS3DLISTENER pListener);
HRESULT DELTACALL idssl_get_distance_factor(idssl* self, D3DVALUE* pflDistanceFactor);
HRESULT DELTACALL idssl_get_doppler_factor(idssl* self, D3DVALUE* pflDopplerFactor);
HRESULT DELTACALL idssl_get_orientation(idssl* self, D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop);
HRESULT DELTACALL idssl_get_position(idssl* self, D3DVECTOR* pvPosition);
HRESULT DELTACALL idssl_get_rolloff_factor(idssl* self, D3DVALUE* pflRolloffFactor);
HRESULT DELTACALL idssl_get_velocity(idssl* self, D3DVECTOR* pvVelocity);
HRESULT DELTACALL idssl_set_all_parameters(idssl* self, LPCDS3DLISTENER pcListener, DWORD dwApply);
HRESULT DELTACALL idssl_set_distance_factor(idssl* self, D3DVALUE flDistanceFactor, DWORD dwApply);
HRESULT DELTACALL idssl_set_doppler_factor(idssl* self, D3DVALUE flDopplerFactor, DWORD dwApply);
HRESULT DELTACALL idssl_set_orientation(idssl* self, D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront,
    D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply);
HRESULT DELTACALL idssl_set_position(idssl* self, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
HRESULT DELTACALL idssl_set_rolloff_factor(idssl* self, D3DVALUE flRolloffFactor, DWORD dwApply);
HRESULT DELTACALL idssl_set_velocity(idssl* self, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
HRESULT DELTACALL idssl_commit_deferred_settings(idssl* self);

typedef struct idssl_vft {
    LPIDSSLQUERYINTERFACE           QueryInterface;
    LPIDSSLADDREF                   AddRef;
    LPIDSSLRELEASE                  Release;
    LPIDSSLGETALLPARAMETERS         GetAllParameters;
    LPIDSSLGETDISTANCEFACTOR        GetDistanceFactor;
    LPIDSSLGETDOPPLERFACTOR         GetDopplerFactor;
    LPIDSSLGETORIENTATION           GetOrientation;
    LPIDSSLGETPOSITION              GetPosition;
    LPIDSSLGETROLLOFFFACTOR         GetRolloffFactor;
    LPIDSSLGETVELOCITY              GetVelocity;
    LPIDSSLSETALLPARAMETERS         SetAllParameters;
    LPIDSSLSETDISTANCEFACTOR        SetDistanceFactor;
    LPIDSSLSETDOPPLERFACTOR         SetDopplerFactor;
    LPIDSSLSETORIENTATION           SetOrientation;
    LPIDSSLSETPOSITION              SetPosition;
    LPIDSSLSETROLLOFFFACTOR         SetRolloffFactor;
    LPIDSSLSETVELOCITY              SetVelocity;
    LPIDSSLCOMMITDEFERREDSETTINGS   CommitDeferredSettings;
} idssl_vft;

const static idssl_vft idssl_self = {
    idssl_query_interface,
    idssl_add_ref,
    idssl_remove_ref,
    idssl_get_all_parameters,
    idssl_get_distance_factor,
    idssl_get_doppler_factor,
    idssl_get_orientation,
    idssl_get_position,
    idssl_get_rolloff_factor,
    idssl_get_velocity,
    idssl_set_all_parameters,
    idssl_set_distance_factor,
    idssl_set_doppler_factor,
    idssl_set_orientation,
    idssl_set_position,
    idssl_set_rolloff_factor,
    idssl_set_velocity,
    idssl_commit_deferred_settings
};

HRESULT DELTACALL idssl_create(allocator* pAlloc, REFIID riid, idssl** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idssl* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idssl), &instance))) {
        instance->Allocator = pAlloc;

        instance->Self = &idssl_self;
        CopyMemory(&instance->ID, riid, sizeof(GUID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idssl_release(idssl* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idssl_query_interface(idssl* self, REFIID riid, LPVOID* ppOut) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    return dssl_query_interface(self->Instance, riid, ppOut);
}

ULONG DELTACALL idssl_add_ref(idssl* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idssl_remove_ref(idssl* self) {
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
            dssl_remove_ref(self->Instance, self);
        }

        idssl_release(self);
    }

    return result;
}

HRESULT DELTACALL idssl_get_all_parameters(idssl* self, LPDS3DLISTENER pListener) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_get_distance_factor(idssl* self, D3DVALUE* pflDistanceFactor) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_get_doppler_factor(idssl* self, D3DVALUE* pflDopplerFactor) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_get_orientation(idssl* self, D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_get_position(idssl* self, D3DVECTOR* pvPosition) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_get_rolloff_factor(idssl* self, D3DVALUE* pflRolloffFactor) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_get_velocity(idssl* self, D3DVECTOR* pvVelocity) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_set_all_parameters(idssl* self, LPCDS3DLISTENER pcListener, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_set_distance_factor(idssl* self, D3DVALUE flDistanceFactor, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_set_doppler_factor(idssl* self, D3DVALUE flDopplerFactor, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_set_orientation(idssl* self,
    D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront,
    D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_set_position(idssl* self, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_set_rolloff_factor(idssl* self, D3DVALUE flRolloffFactor, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_set_velocity(idssl* self, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssl_commit_deferred_settings(idssl* self) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}
