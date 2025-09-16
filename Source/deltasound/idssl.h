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

typedef struct dssl dssl;
typedef struct idssl_vft idssl_vft;

typedef struct idssl {
    const idssl_vft*    Self;
    allocator*          Allocator;
    GUID                ID;
    LONG                RefCount;
    dssl*               Instance;
} idssl;

typedef HRESULT(DELTACALL* LPIDSSLQUERYINTERFACE)(idssl*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSSLADDREF)(idssl*);
typedef ULONG(DELTACALL* LPIDSSLRELEASE)(idssl*);

typedef HRESULT(DELTACALL* LPIDSSLGETALLPARAMETERS)(idssl*, LPDS3DLISTENER pListener);
typedef HRESULT(DELTACALL* LPIDSSLGETDISTANCEFACTOR)(idssl*, D3DVALUE* pflDistanceFactor);
typedef HRESULT(DELTACALL* LPIDSSLGETDOPPLERFACTOR)(idssl*, D3DVALUE* pflDopplerFactor);
typedef HRESULT(DELTACALL* LPIDSSLGETORIENTATION)(idssl*, D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop);
typedef HRESULT(DELTACALL* LPIDSSLGETPOSITION)(idssl*, D3DVECTOR* pvPosition);
typedef HRESULT(DELTACALL* LPIDSSLGETROLLOFFFACTOR)(idssl*, D3DVALUE* pflRolloffFactor);
typedef HRESULT(DELTACALL* LPIDSSLGETVELOCITY)(idssl*, D3DVECTOR* pvVelocity);
typedef HRESULT(DELTACALL* LPIDSSLSETALLPARAMETERS)(idssl*, LPCDS3DLISTENER pcListener, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSLSETDISTANCEFACTOR)(idssl*, D3DVALUE flDistanceFactor, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSLSETDOPPLERFACTOR)(idssl*, D3DVALUE flDopplerFactor, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSLSETORIENTATION)(idssl*, D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSLSETPOSITION)(idssl*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSLSETROLLOFFFACTOR)(idssl*, D3DVALUE flRolloffFactor, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSLSETVELOCITY)(idssl*, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
typedef HRESULT(DELTACALL* LPIDSSLCOMMITDEFERREDSETTINGS)(idssl*);

HRESULT DELTACALL idssl_create(allocator* pAlloc, REFIID riid, idssl** ppOut);
VOID DELTACALL idssl_release(idssl* pidssl);

HRESULT DELTACALL idssl_query_interface(idssl* pidssl, REFIID riid, LPVOID* ppvObject);
ULONG DELTACALL idssl_add_ref(idssl* pidssl);
ULONG DELTACALL idssl_remove_ref(idssl* pidssl);
