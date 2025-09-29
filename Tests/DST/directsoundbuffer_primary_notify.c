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

#include "directsoundbuffer_primary.h"

static BOOL TestDirectSoundBufferPrimaryNotifyCreate(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDNOTIFY na = NULL, nb = NULL;

    const HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundNotify, &na);
    const HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundNotify, &nb);

    if (ra != rb || ra != E_NOINTERFACE) {
        return FALSE;
    }

    return TRUE;
}

BOOL TestDirectSoundBufferPrimaryNotify(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = dsca(NULL, &dsa, NULL);
    HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desc;
    ZeroMemory(&desc, sizeof(DSBUFFERDESC));

    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferPrimaryNotifyCreate(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

exit:
    IDirectSoundBuffer_Release(dsba);
    IDirectSoundBuffer_Release(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
