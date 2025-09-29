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

#include "directsound_createsoundbuffer_primary.h"

#define MAX_PRIMARY_BUFFER_INVALID_FLAG_COUNT   9

const static DWORD CreatePrimaryBufferInvalidFlags[MAX_PRIMARY_BUFFER_INVALID_FLAG_COUNT] = {
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_STATIC,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLFREQUENCY,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPOSITIONNOTIFY,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLFX,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GLOBALFOCUS,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE | DSBCAPS_LOCDEFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE | DSBCAPS_LOCDEFER,
};

#define MAX_PRIMARY_BUFFER_SUCCESS_FLAG_COUNT   10

const static DWORD CreatePrimaryBufferSuccessFlags[MAX_PRIMARY_BUFFER_SUCCESS_FLAG_COUNT] = {
    DSBCAPS_PRIMARYBUFFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCDEFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_TRUEPLAYPOSITION
};

static BOOL TestDirectSoundCreateBufferInvalidInputs(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    {
        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, NULL, NULL, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, NULL, NULL, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, NULL, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, NULL, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, (LPUNKNOWN)&desca);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, (LPUNKNOWN)&descb);

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
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    // dwSize

    {
        DSBUFFERDESC1 desca;
        ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

        DSBUFFERDESC1 descb;
        ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

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

        const HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
        const HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundCreateBufferPrimaryInvalidFlags(LPDIRECTSOUND a, LPDIRECTSOUND b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    DSBUFFERDESC1 desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC1));

    desca.dwSize = sizeof(DSBUFFERDESC1);
    desca.dwFlags = dwFlags;

    DSBUFFERDESC1 descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC1));

    descb.dwSize = sizeof(DSBUFFERDESC1);
    descb.dwFlags = dwFlags;

    const HRESULT ra = IDirectSound_CreateSoundBuffer(a, (LPDSBUFFERDESC)&desca, &dsba, NULL);
    const HRESULT rb = IDirectSound_CreateSoundBuffer(b, (LPDSBUFFERDESC)&descb, &dsbb, NULL);

    if (ra != rb || ra == S_OK) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundCreateBufferPrimary(LPDIRECTSOUND a, LPDIRECTSOUND b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    DSBUFFERDESC desca, descb;
    InitializeDirectSoundBufferDesc(&desca, dwFlags, NULL);
    InitializeDirectSoundBufferDesc(&descb, dwFlags, NULL);

    HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
    HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

    if (ra != rb && !(dwFlags & DSBCAPS_LOCHARDWARE)) {
        return FALSE;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (FAILED(CompareDirectSoundBufferCaps(dsba, dsbb))) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsba);
    RELEASE(dsbb);

    return result;
}

static BOOL TestDirectSoundCreateBufferPrimaryMuliple(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba1 = NULL, dsba2 = NULL;
    LPDIRECTSOUNDBUFFER dsbb1 = NULL, dsbb2 = NULL;

    DSBUFFERDESC desca, descb;
    InitializeDirectSoundBufferDesc(&desca, DSBCAPS_PRIMARYBUFFER, NULL);
    InitializeDirectSoundBufferDesc(&descb, DSBCAPS_PRIMARYBUFFER, NULL);

    const HRESULT ra1 = IDirectSound_CreateSoundBuffer(a, &desca, &dsba1, NULL);
    const HRESULT ra2 = IDirectSound_CreateSoundBuffer(a, &desca, &dsba2, NULL);
    const HRESULT rb1 = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb1, NULL);
    const HRESULT rb2 = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb2, NULL);

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
        const ULONG rca1 = IDirectSoundBuffer_AddRef(dsba1);
        IDirectSoundBuffer_Release(dsba1);

        const ULONG rcb1 = IDirectSoundBuffer_AddRef(dsbb1);
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

    RELEASE(dsba1);
    RELEASE(dsba2);
    RELEASE(dsbb1);
    RELEASE(dsbb2);

    return result;
}

BOOL TestDirectSoundCreateSoundBufferPrimary(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL, dsb = NULL;

    const HRESULT ra = dsca(NULL, &dsa, NULL);
    const HRESULT rb = dscb(NULL, &dsb, NULL);

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

    for (int i = 0; i < MAX_PRIMARY_BUFFER_INVALID_FLAG_COUNT; i++) {
        if (!TestDirectSoundCreateBufferPrimaryInvalidFlags(dsa, dsb, CreatePrimaryBufferInvalidFlags[i])) {
            result = FALSE;
            goto exit;
        }
    }

    for (int i = 0; i < MAX_PRIMARY_BUFFER_SUCCESS_FLAG_COUNT; i++) {
        if (!TestDirectSoundCreateBufferPrimary(dsa, dsb, CreatePrimaryBufferSuccessFlags[i])) {
            result = FALSE;
            goto exit;
        }
    }

    if (!TestDirectSoundCreateBufferPrimaryMuliple(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }


exit:

    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
