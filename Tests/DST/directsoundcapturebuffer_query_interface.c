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

#include "directsoundcapturebuffer.h"

#define MAX_BUFFER_FLAG_COUNT       2

const static DWORD CreateBufferFlags[MAX_BUFFER_FLAG_COUNT] = {
    0,
    DSCBCAPS_WAVEMAPPED
};

static BOOL TestDirectSoundCaptureBufferInterfaces(LPDIRECTSOUNDCAPTUREBUFFER a, LPDIRECTSOUNDCAPTUREBUFFER b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, NULL, &dsa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, NULL, &dsb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &GUID_NULL, &dsa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &GUID_NULL, &dsb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND dsa = NULL, dsb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSound, &dsa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSound, &dsb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND3DBUFFER ds3dba = NULL, ds3dbb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSound3DBuffer, &ds3dba);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSound3DBuffer, &ds3dbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND3DLISTENER ds3dla = NULL, ds3dlb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSound3DListener, &ds3dla);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSound3DListener, &ds3dlb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND8 ds8a = NULL, ds8b = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSound8, &ds8a);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSound8, &ds8b);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundBuffer, &dsba);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundBuffer, &dsbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDBUFFER8 dsba = NULL, dsbb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundBuffer8, &dsba);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundBuffer8, &dsbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTURE dsca = NULL, dscb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundCapture, &dsca);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundCapture, &dscb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER dscba = NULL, dscbb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundCaptureBuffer, &dscba);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundCaptureBuffer, &dscbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }

        if (dscba == NULL || dscbb == NULL) {
            DebugBreak(); return FALSE;
        }

        if (a != dscba || b != dscbb) {
            DebugBreak(); return FALSE;
        }

        RELEASE(dscba);
        RELEASE(dscbb);
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER8 dscba = NULL, dscbb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundCaptureBuffer8, &dscba);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundCaptureBuffer8, &dscbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXAEC fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundCaptureFXAec, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundCaptureFXAec, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCHORUS fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXChorus, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXChorus, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCOMPRESSOR fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXCompressor, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXCompressor, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXDISTORTION fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXDistortion, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXDistortion, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXECHO fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXEcho, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXEcho, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXFLANGER fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXFlanger, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXFlanger, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXGARGLE fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXGargle, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXGargle, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXI3DL2REVERB fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXI3DL2Reverb, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXI3DL2Reverb, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXPARAMEQ fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXParamEq, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXParamEq, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXWAVESREVERB fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFXWavesReverb, &fxa);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFXWavesReverb, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFULLDUPLEX fda = NULL, fdb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundFullDuplex, &fda);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundFullDuplex, &fdb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDNOTIFY sna = NULL, snb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IDirectSoundNotify, &sna);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IDirectSoundNotify, &snb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }

        if (sna == NULL || snb == NULL) {
            DebugBreak(); return FALSE;
        }

        RELEASE(sna);
        RELEASE(snb);
    }

    {
        LPKSPROPERTYSET pa1 = NULL, pb1 = NULL;

        const HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IKsPropertySet, &pa1);
        const HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IKsPropertySet, &pb1);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPREFERENCECLOCK ca = NULL, cb = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IReferenceClock, &ca);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IReferenceClock, &cb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPUNKNOWN ua = NULL, ub = NULL;

        HRESULT ra = IDirectSoundCaptureBuffer_QueryInterface(a, &IID_IUnknown, &ua);
        HRESULT rb = IDirectSoundCaptureBuffer_QueryInterface(b, &IID_IUnknown, &ub);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }

        IUnknown_AddRef(ua);
        IUnknown_AddRef(ub);

        ULONG rcua = IUnknown_Release(ua);
        ULONG rcub = IUnknown_Release(ub);

        if (rcua != rcub) {
            DebugBreak(); return FALSE;
        }

        {
            LPDIRECTSOUNDCAPTUREBUFFER dsa = NULL, dsb = NULL;

            HRESULT ria = IUnknown_QueryInterface(ua, &IID_IDirectSoundCaptureBuffer, &dsa);
            HRESULT rib = IUnknown_QueryInterface(ub, &IID_IDirectSoundCaptureBuffer, &dsb);

            if (ria != rib) {
                DebugBreak(); return FALSE;
            }

            if (a != dsa || b != dsb) {
                DebugBreak(); return FALSE;
            }

            IDirectSoundCaptureBuffer_AddRef(dsa);
            IDirectSoundCaptureBuffer_AddRef(dsb);

            ULONG rcda = IDirectSoundCapture_Release(dsa);
            ULONG rcdb = IDirectSoundCapture_Release(dsb);

            if (rcda != rcdb) {
                DebugBreak(); return FALSE;
            }

            RELEASE(dsa);
            RELEASE(dsb);
        }

        RELEASE(ua);
        RELEASE(ub);
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureBufferQueryInterfaces(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 1, 22050, 8);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, dwFlags, 176400, &format);

    ra = IDirectSoundCapture_CreateCaptureBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSoundCapture_CreateCaptureBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsa == NULL || dsb == NULL) {
        DebugBreak(); return FALSE;
    }

    if (!TestDirectSoundCaptureBufferInterfaces(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDirectSoundCaptureBufferQueryInterface(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        DebugBreak(); return FALSE;
    }

    for (int i = 0; i < MAX_BUFFER_FLAG_COUNT; i++) {
        if (!TestDirectSoundCaptureBufferQueryInterfaces(dsca, dscb,
            CreateBufferFlags[i])) {
            DebugBreak(); return FALSE;
        }
    }

    return TRUE;
}
