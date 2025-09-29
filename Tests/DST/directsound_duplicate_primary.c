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

#include "directsound.h"

static BOOL TestDirectSoundDuplicateSoundBuffer(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;
    LPDIRECTSOUNDBUFFER dsbac = NULL, dsbbc = NULL;

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

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, dsba, NULL);
    rb = IDirectSound_DuplicateSoundBuffer(b, dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, NULL, NULL);
    rb = IDirectSound_DuplicateSoundBuffer(b, NULL, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, NULL, &dsbac);
    rb = IDirectSound_DuplicateSoundBuffer(b, NULL, &dsbbc);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, dsba, &dsbac);
    rb = IDirectSound_DuplicateSoundBuffer(b, dsbb, &dsbbc);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (ra == S_OK || rb == S_OK) {
        result = FALSE;
        goto exit;
    }

exit:

    if (dsba != NULL) {
        IDirectSoundBuffer_Release(dsba);
    }

    if (dsbb != NULL) {
        IDirectSoundBuffer_Release(dsbb);
    }

    return result;
}

BOOL TestDirectSoundDuplicatePrimary(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

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

    if (!TestDirectSoundDuplicateSoundBuffer(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
