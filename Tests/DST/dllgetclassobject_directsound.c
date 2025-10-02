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

#include "dllgetclassobject.h"

static BOOL TestDllGetClassObjectDirectSoundMethods(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    HRESULT ra = S_OK, rb = S_OK;

    // CreateSoundBuffer
    {
        LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

        DSBUFFERDESC desc;
        InitializeDirectSoundBufferDesc(&desc, DSBCAPS_PRIMARYBUFFER, 0, NULL);

        ra = IDirectSound_CreateSoundBuffer(a, &desc, &dsba, NULL);
        rb = IDirectSound_CreateSoundBuffer(b, &desc, &dsbb, NULL);

        if (ra != rb || ra != DSERR_UNINITIALIZED) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

        WAVEFORMATEX format;
        InitializeWaveFormat(&format, 2, 8000, 8);

        DSBUFFERDESC desc;
        InitializeDirectSoundBufferDesc(&desc, 0, 96000, &format);

        ra = IDirectSound_CreateSoundBuffer(a, &desc, &dsba, NULL);
        rb = IDirectSound_CreateSoundBuffer(b, &desc, &dsbb, NULL);

        if (ra != rb || ra != DSERR_UNINITIALIZED) {
            return FALSE;
        }
    }

    // GetCaps
    {
        DSCAPS capsa, capsb;
        ZeroMemory(&capsa, sizeof(DSCAPS));
        ZeroMemory(&capsb, sizeof(DSCAPS));

        capsa.dwSize = sizeof(DSCAPS);
        capsb.dwSize = sizeof(DSCAPS);

        ra = IDirectSound_GetCaps(a, &capsa);
        rb = IDirectSound_GetCaps(b, &capsb);

        if (ra != rb || ra != DSERR_UNINITIALIZED) {
            return FALSE;
        }
    }

    // DuplicateSoundBuffer

    // SetCooperativeLevel
    {
        ra = IDirectSound_SetCooperativeLevel(a, GetDesktopWindow(), DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(b, GetDesktopWindow(), DSSCL_NORMAL);

        if (ra != rb || ra != DSERR_UNINITIALIZED) {
            return FALSE;
        }
    }

    // Compact
    {
        ra = IDirectSound_Compact(a);
        rb = IDirectSound_Compact(b);

        if (ra != rb || ra != DSERR_UNINITIALIZED) {
            return FALSE;
        }
    }

    // GetSpeakerConfig

    // TODO

    // SetSpeakerConfig

    // TODO

    // Initialize
    {
        ra = IDirectSound_Initialize(a, &IID_IDirectSoundBuffer);
        rb = IDirectSound_Initialize(b, &IID_IDirectSoundBuffer);

        if (ra != rb || ra != DSERR_NODRIVER) {
            return FALSE;
        }

        ra = IDirectSound_Initialize(a, NULL);
        rb = IDirectSound_Initialize(b, NULL);

        if (ra != rb || ra != DS_OK) {
            return FALSE;
        }

        ra = IDirectSound_Initialize(a, NULL);
        rb = IDirectSound_Initialize(b, NULL);

        if (ra != rb || ra != DSERR_ALREADYINITIALIZED) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDllGetClassObjectDirectSoundInstance(LPCLASSFACTORY a, LPCLASSFACTORY b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUND dsa = NULL, dsb = NULL;

    const HRESULT ra = IClassFactory_CreateInstance(a, NULL, &IID_IDirectSound, &dsa);
    const HRESULT rb = IClassFactory_CreateInstance(b, NULL, &IID_IDirectSound, &dsb);

    if (ra != rb || dsa == NULL || dsb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDllGetClassObjectDirectSoundMethods(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDllGetClassObjectDirectSound(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPFNGETCLASSOBJECT gcoa = GetDllGetClassObject(a);
    LPFNGETCLASSOBJECT gcob = GetDllGetClassObject(b);

    if (gcoa == NULL || gcob == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    HRESULT ra = S_OK, rb = S_OK;
    LPCLASSFACTORY cfa = NULL, cfb = NULL;

    if (FAILED(ra = gcoa(&CLSID_DirectSound, &IID_IClassFactory, &cfa))) {
        result = FALSE;
        goto exit;
    }

    if (FAILED(rb = gcob(&CLSID_DirectSound, &IID_IClassFactory, &cfb))) {
        result = FALSE;
        goto exit;
    }

    if (ra != rb || cfa == NULL || cfb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDllGetClassObjectDirectSoundInstance(cfa, cfb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(cfa);
    RELEASE(cfb);

    return result;
}
