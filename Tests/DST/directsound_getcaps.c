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

#include "directsound_getcaps.h"

#include <dsound.h>

typedef HRESULT(WINAPI* LPDIRECTSOUNDCREATE)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);

static BOOL TestDirectSoundGetCapsInvalidInputs(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    {
        HRESULT ra = IDirectSound_GetCaps(a, NULL);
        HRESULT rb = IDirectSound_GetCaps(b, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCAPS ca;
        ZeroMemory(&ca, sizeof(DSCAPS));

        DSCAPS cb;
        ZeroMemory(&cb, sizeof(DSCAPS));

        HRESULT ra = IDirectSound_GetCaps(a, &ca);
        HRESULT rb = IDirectSound_GetCaps(b, &cb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        DSCAPS ca;
        ZeroMemory(&ca, sizeof(DSCAPS));

        ca.dwSize = UINT_MAX;

        DSCAPS cb;
        ZeroMemory(&cb, sizeof(DSCAPS));

        cb.dwSize = UINT_MAX;

        HRESULT ra = IDirectSound_GetCaps(a, &ca);
        HRESULT rb = IDirectSound_GetCaps(b, &cb);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundGetCapsValidInputs(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    DSCAPS ca;
    ZeroMemory(&ca, sizeof(DSCAPS));

    ca.dwSize = sizeof(DSCAPS);

    DSCAPS cb;
    ZeroMemory(&cb, sizeof(DSCAPS));

    cb.dwSize = sizeof(DSCAPS);

    HRESULT ra = IDirectSound_GetCaps(a, &ca);
    HRESULT rb = IDirectSound_GetCaps(b, &cb);

    if (ra != rb) {
        return FALSE;
    }

    return memcmp(&ca, &cb, sizeof(DSCAPS)) == 0;
}

BOOL TestDirectSoundGetCaps(HMODULE a, HMODULE b) {
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

    if (!TestDirectSoundGetCapsInvalidInputs(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundGetCapsValidInputs(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:

    IDirectSound_Release(dsa);
    IDirectSound_Release(dsb);

    return result;
}
