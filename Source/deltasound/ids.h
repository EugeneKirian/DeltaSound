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

#include "base.h"

#include "idsb.h"

#define DSSCL_NONE  0
#define DSSCL_VALID (DSSCL_WRITEPRIMARY | DSSCL_EXCLUSIVE | DSSCL_PRIORITY | DSSCL_NORMAL)

typedef struct ids ids;

typedef HRESULT(DELTACALL* LPIDSQUERYINTERFACE)(ids*, REFIID, LPVOID*);
typedef ULONG(DELTACALL* LPIDSADDREF)(ids*);
typedef ULONG(DELTACALL* LPIDSRELEASE)(ids*);

typedef HRESULT(DELTACALL* LPIDSCREATESOUNDBUFFER)(ids*, LPCDSBUFFERDESC pcDSBufferDesc, idsb** ppDSBuffer, LPUNKNOWN pUnkOuter);
typedef HRESULT(DELTACALL* LPIDSGETCAPS)(ids*, LPDSCAPS pDSCaps);
typedef HRESULT(DELTACALL* LPIDSDUPLICATESOUNDBUFFER)(ids*, idsb* pDSBufferOriginal, idsb** ppDSBufferDuplicate);
typedef HRESULT(DELTACALL* LPIDSSETCOOPERATIVELEVEL)(ids*, HWND hwnd, DWORD dwLevel);
typedef HRESULT(DELTACALL* LPIDSCOMPACT)(ids*);
typedef HRESULT(DELTACALL* LPIDSGETSPEAKERCONFIG)(ids*, LPDWORD pdwSpeakerConfig);
typedef HRESULT(DELTACALL* LPIDSSETSPEAKERCONFIG)(ids*, DWORD dwSpeakerConfig);
typedef HRESULT(DELTACALL* LPIDSINITIALIZE)(ids*, LPCGUID pcGuidDevice);

typedef struct ids_vft ids_vft;

struct ids {
    const ids_vft* Self;
};

HRESULT DELTACALL ids_create(ids* pIDS);
