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

#include "directsound_createsoundbuffer.h"

#include <dsound.h>

typedef HRESULT(WINAPI* LPDIRECTSOUNDCREATE)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);

BOOL TestDirectSoundCreateBufferInvalidInputs(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    {
        HRESULT ra = IDirectSound_CreateSoundBuffer(a, NULL, NULL, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, NULL, NULL, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, NULL, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, NULL, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, (LPUNKNOWN)&desca);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, (LPUNKNOWN)&descb);

        if (ra != rb) {
            return FALSE;
        }

        if (dsba != NULL || dsbb != NULL) {
            return FALSE;
        }
    }

    return result;
}

static BOOL TestDirectSoundCreateBufferInvalidDesc(LPDIRECTSOUND a, LPDIRECTSOUND b) {

    // TODO


    return TRUE;
}

static BOOL TestDirectSoundCreateBufferPrimary(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = DSBCAPS_PRIMARYBUFFER;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = DSBCAPS_PRIMARYBUFFER;

    HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
    HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsba == NULL || dsbb == NULL) {
        return FALSE;
    }

    IDirectSoundBuffer_Release(dsba);
    IDirectSoundBuffer_Release(dsbb);

    return result;
}

BOOL TestDirectSoundCreateSoundBuffer(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = (LPDIRECTSOUNDCREATE)GetProcAddress(a, "DirectSoundCreate");
    LPDIRECTSOUNDCREATE dscb = (LPDIRECTSOUNDCREATE)GetProcAddress(b, "DirectSoundCreate");

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = dsca(NULL, &dsa, NULL);
    HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    if (!TestDirectSoundCreateBufferInvalidInputs(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCreateBufferInvalidDesc(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCreateBufferPrimary(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

     // TODO more tests


exit:

    IDirectSound_Release(dsa);
    IDirectSound_Release(dsb);

    return result;
}
