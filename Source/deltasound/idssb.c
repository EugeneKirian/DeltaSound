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

#include "dssb.h"
#include "idssb.h"

HRESULT DELTACALL idssb_get_all_parameters(idssb*, LPDS3DBUFFER pDs3dBuffer);
HRESULT DELTACALL idssb_get_cone_angles(idssb*, LPDWORD pdwInsideConeAngle, LPDWORD pdwOutsideConeAngle);
HRESULT DELTACALL idssb_get_cone_orientation(idssb*, D3DVECTOR* pvOrientation);
HRESULT DELTACALL idssb_get_cone_outside_volume(idssb*, LPLONG plConeOutsideVolume);
HRESULT DELTACALL idssb_get_max_distance(idssb*, D3DVALUE* pflMaxDistance);
HRESULT DELTACALL idssb_get_min_distance(idssb*, D3DVALUE* pflMinDistance);
HRESULT DELTACALL idssb_get_mode(idssb*, LPDWORD pdwMode);
HRESULT DELTACALL idssb_get_position(idssb*, D3DVECTOR* pvPosition);
HRESULT DELTACALL idssb_get_velocity(idssb*, D3DVECTOR* pvVelocity);
HRESULT DELTACALL idssb_set_all_parameters(idssb*, LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply);
HRESULT DELTACALL idssb_set_cone_angles(idssb*, DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply);
HRESULT DELTACALL idssb_set_cone_orientation(idssb*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
HRESULT DELTACALL idssb_set_cone_outside_volume(idssb*, LONG lConeOutsideVolume, DWORD dwApply);
HRESULT DELTACALL idssb_set_max_distance(idssb*, D3DVALUE flMaxDistance, DWORD dwApply);
HRESULT DELTACALL idssb_set_min_distance(idssb*, D3DVALUE flMinDistance, DWORD dwApply);
HRESULT DELTACALL idssb_set_mode(idssb*, DWORD dwMode, DWORD dwApply);
HRESULT DELTACALL idssb_set_position(idssb*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
HRESULT DELTACALL idssb_set_velocity(idssb*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);

typedef struct idssb_vft {
    LPIDSSBQUERYINTERFACE           QueryInterface;
    LPIDSSBADDREF                   AddRef;
    LPIDSSBRELEASE                  Release;
    LPIDSSBGETALLPARAMETERS         GetAllParameters;
    LPIDSSBGETCONEANGLES            GetConeAngles;
    LPIDSSBGETCONEORIENTATION       GetConeOrientation;
    LPIDSSBGETCONEOUTSIDEVOLUME     GetConeOutsideVolume;
    LPIDSSBGETMAXDISTANCE           GetMaxDistance;
    LPIDSSBGETMINDISTANCE           GetMinDistance;
    LPIDSSBGETMODE                  GetMode;
    LPIDSSBGETPOSITION              GetPosition;
    LPIDSSBGETVELOCITY              GetVelocity;
    LPIDSSBSETALLPARAMETERS         SetAllParameters;
    LPIDSSBSETCONEANGLES            SetConeAngles;
    LPIDSSBSETCONEORIENTATION       SetConeOrientation;
    LPIDSSBSETCONEOUTSIDEVOLUME     SetConeOutsideVolume;
    LPIDSSBSETMAXDISTANCE           SetMaxDistance;
    LPIDSSBSETMINDISTANCE           SetMinDistance;
    LPIDSSBSETMODE                  SetMode;
    LPIDSSBSETPOSITION              SetPosition;
    LPIDSSBSETVELOCITY              SetVelocity;
} idssb_vft;

const static idssb_vft idssb_self = {
    idssb_query_interface,
    idssb_add_ref,
    idssb_remove_ref,
    idssb_get_all_parameters,
    idssb_get_cone_angles,
    idssb_get_cone_orientation,
    idssb_get_cone_outside_volume,
    idssb_get_max_distance,
    idssb_get_min_distance,
    idssb_get_mode,
    idssb_get_position,
    idssb_get_velocity,
    idssb_set_all_parameters,
    idssb_set_cone_angles,
    idssb_set_cone_orientation,
    idssb_set_cone_outside_volume,
    idssb_set_max_distance,
    idssb_set_min_distance,
    idssb_set_mode,
    idssb_set_position,
    idssb_set_velocity
};

HRESULT DELTACALL idssb_allocate(allocator* pAlloc, idssb** ppOut);

HRESULT DELTACALL idssb_create(allocator* pAlloc, REFIID riid, idssb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idssb* instance = NULL;

    if (SUCCEEDED(hr = idssl_allocate(pAlloc, &instance))) {
        instance->Self = &idssb_self;
        CopyMemory(&instance->ID, riid, sizeof(GUID));
        instance->RefCount = 1;
        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idssb_release(idssb* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idssb_query_interface(idssb* self, REFIID riid, LPVOID* ppvObject) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppvObject == NULL) {
        return E_INVALIDARG;
    }

    return dssb_query_interface(self->Instance, riid, ppvObject);
}

ULONG DELTACALL idssb_add_ref(idssb* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idssb_remove_ref(idssb* self) {
    if (self == NULL) {
        return 0;
    }

    if (self->RefCount == 0) {
        return 0;
    }

    if (InterlockedDecrement(&self->RefCount) <= 0) {
        self->RefCount = 0;

        if (self->Instance != NULL) {
            dssb_remove_ref(self->Instance, self);
        }

        idssb_release(self);
    }

    return self->RefCount;
}

HRESULT DELTACALL idssb_get_all_parameters(idssb* self, LPDS3DBUFFER pDs3dBuffer) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_get_cone_angles(idssb* self, LPDWORD pdwInsideConeAngle, LPDWORD pdwOutsideConeAngle) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;

}
HRESULT DELTACALL idssb_get_cone_orientation(idssb* self, D3DVECTOR* pvOrientation) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_get_cone_outside_volume(idssb* self, LPLONG plConeOutsideVolume) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_get_max_distance(idssb* self, D3DVALUE* pflMaxDistance) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;

}
HRESULT DELTACALL idssb_get_min_distance(idssb* self, D3DVALUE* pflMinDistance) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_get_mode(idssb* self, LPDWORD pdwMode) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_get_position(idssb* self, D3DVECTOR* pvPosition) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_get_velocity(idssb* self, D3DVECTOR* pvVelocity) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_all_parameters(idssb* self, LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_cone_angles(idssb* self, DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_cone_orientation(idssb* self, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_cone_outside_volume(idssb* self, LONG lConeOutsideVolume, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_max_distance(idssb* self, D3DVALUE flMaxDistance, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_min_distance(idssb* self, D3DVALUE flMinDistance, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_mode(idssb* self, DWORD dwMode, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_position(idssb* self, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idssb_set_velocity(idssb* self, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL idssb_allocate(allocator* pAlloc, idssb** ppOut) {
    HRESULT hr = S_OK;
    idssb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idssb), &instance))) {
        instance->Allocator = pAlloc;
        *ppOut = instance;
    }

    return hr;
}
