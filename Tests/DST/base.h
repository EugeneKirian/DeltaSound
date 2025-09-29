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

#define COBJMACROS
#include <windows.h>
#include <dsound.h>
#include <mmreg.h>

#define RELEASE(X)  if((X) != NULL) { IUnknown_Release((X)); (X) = NULL;}

typedef HRESULT(WINAPI* LPDIRECTSOUNDCREATE)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);

typedef BOOL(CALLBACK* LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef HRESULT(WINAPI* LPDIRECTSOUNDENUMERATEA)(LPDSENUMCALLBACKA, LPVOID);

#define COOPERATIVE_LEVEL_COUNT 5

const extern DWORD CooperativeLevels[COOPERATIVE_LEVEL_COUNT];

#define BUFFER_PLAY_FLAGS_COUNT    39

const extern DWORD BufferPlayFlags[BUFFER_PLAY_FLAGS_COUNT];

const extern GUID IID_IDirectSoundPrivate;
const extern GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

typedef IReferenceClock* LPREFERENCECLOCK;

LPDIRECTSOUNDCREATE GetDirectSoundCreate(HMODULE module);

HRESULT InitializeWaveFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwChannels, DWORD dwFrequency, DWORD dwBits);

HRESULT InitializeDirectSoundBufferDesc(LPDSBUFFERDESC pDSBD,
    DWORD dwFlags, DWORD dwBufferSize, LPWAVEFORMATEX pwfxFormat);

HRESULT InitializeDirectSoundBufferCaps(LPDSBCAPS pDSBC, DWORD dwFlags, DWORD dwBufferBytes);

HRESULT CompareDirectSoundBufferCaps(LPDIRECTSOUNDBUFFER pDSBA, LPDIRECTSOUNDBUFFER pDSBB);
