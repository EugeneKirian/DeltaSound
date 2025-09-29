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

static BOOL TestDirectSoundAddRef(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    ULONG ac = IDirectSound_AddRef(a);
    ULONG bc = IDirectSound_AddRef(b);

    if (ac != bc) {
        return FALSE;
    }

    ac = IDirectSound_AddRef(a);
    bc = IDirectSound_AddRef(b);

    if (ac != bc) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundRelease(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    ULONG ac = IDirectSound_Release(a);
    ULONG bc = IDirectSound_Release(b);

    if (ac != bc) {
        return FALSE;
    }

    ac = IDirectSound_Release(a);
    bc = IDirectSound_Release(b);

    if (ac != bc) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundQueryInterface(LPDIRECTSOUND a, LPDIRECTSOUND b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    {
        LPDIRECTSOUND dsa = NULL, dsb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, NULL, &dsa);
        const HRESULT rb = IDirectSound_QueryInterface(b, NULL, &dsb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND dsa = NULL, dsb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &GUID_NULL, &dsa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &GUID_NULL, &dsb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND dsa = NULL, dsb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSound, &dsa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSound, &dsb);

        if (ra != rb) {
            return FALSE;
        }

        if (a != dsa || b != dsb) {
            return FALSE;
        }

        RELEASE(dsa);
        RELEASE(dsb);
    }

    {
        LPDIRECTSOUND3DBUFFER ds3dba = NULL, ds3dbb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSound3DBuffer, &ds3dba);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSound3DBuffer, &ds3dbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND3DLISTENER ds3dla = NULL, ds3dlb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSound3DListener, &ds3dla);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSound3DListener, &ds3dlb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND8 ds8a = NULL, ds8b = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSound8, &ds8a);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSound8, &ds8b);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundBuffer, &dsba);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundBuffer, &dsbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDBUFFER8 dsba = NULL, dsbb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundBuffer8, &dsba);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundBuffer8, &dsbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTURE dsca = NULL, dscb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundCapture, &dsca);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundCapture, &dscb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER dscba = NULL, dscbb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundCaptureBuffer, &dscba);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundCaptureBuffer, &dscbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER8 dscba = NULL, dscbb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundCaptureBuffer8, &dscba);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundCaptureBuffer8, &dscbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXAEC fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundCaptureFXAec, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundCaptureFXAec, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCHORUS fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXChorus, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXChorus, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCOMPRESSOR fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXCompressor, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXCompressor, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXDISTORTION fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXDistortion, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXDistortion, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXECHO fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXEcho, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXEcho, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXFLANGER fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXFlanger, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXFlanger, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXGARGLE fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXGargle, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXGargle, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXI3DL2REVERB fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXI3DL2Reverb, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXI3DL2Reverb, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXPARAMEQ fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXParamEq, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXParamEq, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXWAVESREVERB fxa = NULL, fxb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFXWavesReverb, &fxa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFXWavesReverb, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFULLDUPLEX fda = NULL, fdb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundFullDuplex, &fda);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundFullDuplex, &fdb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDNOTIFY sna = NULL, snb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundNotify, &sna);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundNotify, &snb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPKSPROPERTYSET pa = NULL, pb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IKsPropertySet, &pa);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IKsPropertySet, &pb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPREFERENCECLOCK ca = NULL, cb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IReferenceClock, &ca);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IReferenceClock, &cb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPREFERENCECLOCK ca = NULL, cb = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IDirectSoundPrivate, &ca);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IDirectSoundPrivate, &cb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPUNKNOWN ua = NULL, ub = NULL;

        const HRESULT ra = IDirectSound_QueryInterface(a, &IID_IUnknown, &ua);
        const HRESULT rb = IDirectSound_QueryInterface(b, &IID_IUnknown, &ub);

        if (ra != rb) {
            return FALSE;
        }

        IUnknown_AddRef(ua);
        IUnknown_AddRef(ub);

        const ULONG rcua = IUnknown_Release(ua);
        const ULONG rcub = IUnknown_Release(ub);

        if (rcua != rcub) {
            return FALSE;
        }

        if ((LPVOID)a == (LPVOID)ua || (LPVOID)b == (LPVOID)ub) {
            return FALSE;
        }

        {
            LPDIRECTSOUND dsa = NULL, dsb = NULL;

            const HRESULT ria = IUnknown_QueryInterface(ua, &IID_IDirectSound, &dsa);
            const HRESULT rib = IUnknown_QueryInterface(ub, &IID_IDirectSound, &dsb);

            if (ria != rib) {
                return FALSE;
            }

            if (a != dsa || b != dsb) {
                return FALSE;
            }

            IDirectSound_AddRef(dsa);
            IDirectSound_AddRef(dsb);

            const ULONG rcda = IDirectSound_Release(dsa);
            const ULONG rcdb = IDirectSound_Release(dsb);

            if (rcda != rcdb) {
                return FALSE;
            }

            RELEASE(dsa);
            RELEASE(dsb);
        }

        RELEASE(ua);
        RELEASE(ub);
    }

    return TRUE;
}

BOOL TestDirectSoundBasics(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL, dsb = NULL;

    const HRESULT ra = dsca(NULL, &dsa, NULL);
    const HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    BOOL result = TRUE;

    // AddRef
    if (!TestDirectSoundAddRef(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    // Release
    if (!TestDirectSoundRelease(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

    // QueryInterface
    if (!TestDirectSoundQueryInterface(dsa, dsb)) {
        result = FALSE;
        goto exit;
    }

exit:
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}
