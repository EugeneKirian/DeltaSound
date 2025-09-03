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

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);

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

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, (LPUNKNOWN)&desca);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, (LPUNKNOWN)&descb);

        if (ra != rb) {
            return FALSE;
        }

        if (dsba != NULL || dsbb != NULL) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCreateBufferPrimaryInvalidDesc(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    // dwSize

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1) - 1;
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1) - 1;
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwFlags

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_STATIC;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_STATIC;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLFREQUENCY;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLFREQUENCY;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPOSITIONNOTIFY;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPOSITIONNOTIFY;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLFX;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLFX;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_GLOBALFOCUS;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_GLOBALFOCUS;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_MUTE3DATMAXDISTANCE;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_MUTE3DATMAXDISTANCE;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCDEFER;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCDEFER;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwBufferBytes

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER;
        desca.dwBufferBytes = 1;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER;
        descb.dwBufferBytes = 1;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // dwReserved

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER;
        desca.dwReserved = 1;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER;
        descb.dwReserved = 1;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // lpwfxFormat

    {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));

        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        desca.dwSize = sizeof(DSBUFFERDESC1);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER;
        desca.lpwfxFormat = &format;

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        descb.dwSize = sizeof(DSBUFFERDESC1);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER;
        descb.lpwfxFormat = &format;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // guid3DAlgorithm
    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER;
        desca.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER;
        descb.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        desca.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        descb.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        desca.guid3DAlgorithm = DS3DALG_HRTF_FULL;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        descb.guid3DAlgorithm = DS3DALG_HRTF_FULL;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSBUFFERDESC desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC));

        desca.dwSize = sizeof(DSBUFFERDESC);
        desca.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        desca.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;

        DSBUFFERDESC descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC));

        descb.dwSize = sizeof(DSBUFFERDESC);
        descb.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        descb.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;

        HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

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

static BOOL TestDirectSoundCreateBufferPrimaryMuliple(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba1 = NULL;
    LPDIRECTSOUNDBUFFER dsba2 = NULL;
    LPDIRECTSOUNDBUFFER dsbb1 = NULL;
    LPDIRECTSOUNDBUFFER dsbb2 = NULL;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = DSBCAPS_PRIMARYBUFFER;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = DSBCAPS_PRIMARYBUFFER;

    HRESULT ra1 = IDirectSound_CreateSoundBuffer(a, &desca, &dsba1, NULL);
    HRESULT ra2 = IDirectSound_CreateSoundBuffer(a, &desca, &dsba2, NULL);
    HRESULT rb1 = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb1, NULL);
    HRESULT rb2 = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb2, NULL);

    if ((ra1 != ra2) || (ra1 != rb1) || (rb1 != rb2)) {
        return FALSE;
    }

    if (dsba1 == NULL || dsba2 == NULL || dsbb1 == NULL || dsbb2 == NULL) {
        result = FALSE;
        goto exit;
    }

    if (dsba1 != dsba2 || dsbb1 != dsbb2) {
        result = FALSE;
        goto exit;
    }

    {
        ULONG rca1 = IDirectSoundBuffer_AddRef(dsba1);
        IDirectSoundBuffer_Release(dsba1);

        ULONG rcb1 = IDirectSoundBuffer_AddRef(dsbb1);
        IDirectSoundBuffer_Release(dsbb1);

        if (rca1 != rcb1) {
            result = FALSE;
            goto exit;
        }
    }

    {
        IDirectSoundBuffer_Release(dsba1);
        IDirectSoundBuffer_Release(dsba1);
        ULONG rca1 = IDirectSoundBuffer_Release(dsba1);

        IDirectSoundBuffer_Release(dsbb1);
        IDirectSoundBuffer_Release(dsbb1);
        ULONG rcb1 = IDirectSoundBuffer_Release(dsbb1);

        if (rca1 != rcb1) {
            result = FALSE;
            goto exit;
        }

        rca1 = IDirectSoundBuffer_AddRef(dsba1);
        rcb1 = IDirectSoundBuffer_AddRef(dsbb1);

        if (rca1 != rcb1) {
            result = FALSE;
            goto exit;
        }
    }

exit:

    if (dsba1 != NULL) {
        IDirectSoundBuffer_Release(dsba1);
    }

    if (dsba2 != NULL) {
        IDirectSoundBuffer_Release(dsba2);
    }

    if (dsbb1 != NULL) {
        IDirectSoundBuffer_Release(dsbb1);
    }

    if (dsbb2 != NULL) {
        IDirectSoundBuffer_Release(dsbb2);
    }

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

    if (!TestDirectSoundCreateBufferPrimaryInvalidDesc(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCreateBufferPrimary(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundCreateBufferPrimaryMuliple(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

     // TODO more tests - secondary buffers


exit:

    IDirectSound_Release(dsa);
    IDirectSound_Release(dsb);

    return result;
}
