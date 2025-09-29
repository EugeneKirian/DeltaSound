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

#pragma once

#include "allocator.h"

typedef struct dssb dssb;
typedef struct idssb_vft idssb_vft;

typedef struct idssb {
    const idssb_vft*    Self;
    allocator*          Allocator;
    GUID                ID;
    LONG                RefCount;
    dssb*               Instance;
} idssb;

typedef HRESULT(DELTACALL* LPIDSSBQUERYINTERFACE)(idssb*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSSBADDREF)(idssb*);
typedef ULONG(DELTACALL* LPIDSSBRELEASE)(idssb*);

typedef HRESULT(DELTACALL* LPIDSSBGETALLPARAMETERS)(idssb*, LPDS3DBUFFER pDs3dBuffer);
typedef HRESULT(DELTACALL* LPIDSSBGETCONEANGLES)(idssb*, LPDWORD pdwInsideConeAngle, LPDWORD pdwOutsideConeAngle);
typedef HRESULT(DELTACALL* LPIDSSBGETCONEORIENTATION)(idssb*, D3DVECTOR* pvOrientation);
typedef HRESULT(DELTACALL* LPIDSSBGETCONEOUTSIDEVOLUME)(idssb*, LPLONG plConeOutsideVolume);
typedef HRESULT(DELTACALL* LPIDSSBGETMAXDISTANCE)(idssb*, D3DVALUE* pflMaxDistance);
typedef HRESULT(DELTACALL* LPIDSSBGETMINDISTANCE)(idssb*, D3DVALUE* pflMinDistance);
typedef HRESULT(DELTACALL* LPIDSSBGETMODE)(idssb*, LPDWORD pdwMode);
typedef HRESULT(DELTACALL* LPIDSSBGETPOSITION)(idssb*, D3DVECTOR* pvPosition);
typedef HRESULT(DELTACALL* LPIDSSBGETVELOCITY)(idssb*, D3DVECTOR* pvVelocity);
typedef HRESULT(DELTACALL* LPIDSSBSETALLPARAMETERS)(idssb*, LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETCONEANGLES)(idssb*, DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETCONEORIENTATION)(idssb*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETCONEOUTSIDEVOLUME)(idssb*, LONG lConeOutsideVolume, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETMAXDISTANCE)(idssb*, D3DVALUE flMaxDistance, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETMINDISTANCE)(idssb*, D3DVALUE flMinDistance, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETMODE)(idssb*, DWORD dwMode, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETPOSITION)(idssb*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSBSETVELOCITY)(idssb*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);

HRESULT DELTACALL idssb_create(allocator* pAlloc, REFIID riid, idssb** ppOut);
VOID DELTACALL idssb_release(idssb* pIDSSB);

HRESULT DELTACALL idssb_query_interface(idssb* pIDSSB, REFIID riid, LPVOID* ppOut);
ULONG DELTACALL idssb_add_ref(idssb* pIDSSB);
ULONG DELTACALL idssb_remove_ref(idssb* pIDSSB);
