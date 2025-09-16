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

#include "directsoundbuffer_primary_query_interface.h"
#include "wnd.h"

#define WINDOW_NAME "DirectSound Primary Buffer Query Interface"

typedef IReferenceClock* LPREFERENCECLOCK;

#define COOPERATIVE_LEVEL_COUNT 4

static const DWORD CooperativeLevels[COOPERATIVE_LEVEL_COUNT] = {
    DSSCL_NORMAL, DSSCL_PRIORITY, DSSCL_EXCLUSIVE, DSSCL_WRITEPRIMARY
};

#define BUFFER_FLAG_COUNT       10

static const DWORD BufferFlags[BUFFER_FLAG_COUNT] =
{
    DSBCAPS_PRIMARYBUFFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCDEFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_TRUEPLAYPOSITION
};

static BOOL TestDirectSoundBufferQueryInterfaces(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    {
        LPDIRECTSOUND dsa = NULL;
        LPDIRECTSOUND dsb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, NULL, &dsa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, NULL, &dsb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND dsa = NULL;
        LPDIRECTSOUND dsb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &GUID_NULL, &dsa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &GUID_NULL, &dsb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND dsa = NULL;
        LPDIRECTSOUND dsb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSound, &dsa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSound, &dsb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND3DBUFFER ds3dba = NULL;
        LPDIRECTSOUND3DBUFFER ds3dbb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSound3DBuffer, &ds3dba);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSound3DBuffer, &ds3dbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUND3DLISTENER ds3dla = NULL;
        LPDIRECTSOUND3DLISTENER ds3dlb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSound3DListener, &ds3dla);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSound3DListener, &ds3dlb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }

        if (ra == S_OK) {
            IDirectSound3DListener_Release(ds3dla);
            IDirectSound3DListener_Release(ds3dlb);
        }
    }

    {
        LPDIRECTSOUND8 ds8a = NULL;
        LPDIRECTSOUND8 ds8b = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSound8, &ds8a);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSound8, &ds8b);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDBUFFER dsba = NULL;
        LPDIRECTSOUNDBUFFER dsbb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundBuffer, &dsba);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundBuffer, &dsbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }

        if (dsba == NULL || dsbb == NULL) {
            DebugBreak(); return FALSE;
        }

        IDirectSoundBuffer_Release(dsba);
        IDirectSoundBuffer_Release(dsbb);
    }

    {
        LPDIRECTSOUNDBUFFER8 dsba = NULL;
        LPDIRECTSOUNDBUFFER8 dsbb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundBuffer8, &dsba);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundBuffer8, &dsbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTURE dsca = NULL;
        LPDIRECTSOUNDCAPTURE dscb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundCapture, &dsca);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundCapture, &dscb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER dscba = NULL;
        LPDIRECTSOUNDCAPTUREBUFFER dscbb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundCaptureBuffer, &dscba);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundCaptureBuffer, &dscbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER8 dscba = NULL;
        LPDIRECTSOUNDCAPTUREBUFFER8 dscbb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundCaptureBuffer8, &dscba);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundCaptureBuffer8, &dscbb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXAEC fxa = NULL;
        LPDIRECTSOUNDCAPTUREFXAEC fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundCaptureFXAec, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundCaptureFXAec, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS fxa = NULL;
        LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCHORUS fxa = NULL;
        LPDIRECTSOUNDFXCHORUS fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXChorus, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXChorus, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCOMPRESSOR fxa = NULL;
        LPDIRECTSOUNDFXCOMPRESSOR fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXCompressor, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXCompressor, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXDISTORTION fxa = NULL;
        LPDIRECTSOUNDFXDISTORTION fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXDistortion, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXDistortion, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXECHO fxa = NULL;
        LPDIRECTSOUNDFXECHO fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXEcho, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXEcho, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXFLANGER fxa = NULL;
        LPDIRECTSOUNDFXFLANGER fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXFlanger, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXFlanger, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXGARGLE fxa = NULL;
        LPDIRECTSOUNDFXGARGLE fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXGargle, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXGargle, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXI3DL2REVERB fxa = NULL;
        LPDIRECTSOUNDFXI3DL2REVERB fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXI3DL2Reverb, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXI3DL2Reverb, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXPARAMEQ fxa = NULL;
        LPDIRECTSOUNDFXPARAMEQ fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXParamEq, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXParamEq, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXWAVESREVERB fxa = NULL;
        LPDIRECTSOUNDFXWAVESREVERB fxb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFXWavesReverb, &fxa);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFXWavesReverb, &fxb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFULLDUPLEX fda = NULL;
        LPDIRECTSOUNDFULLDUPLEX fdb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundFullDuplex, &fda);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundFullDuplex, &fdb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPDIRECTSOUNDNOTIFY sna = NULL;
        LPDIRECTSOUNDNOTIFY snb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundNotify, &sna);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundNotify, &snb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPKSPROPERTYSET pa1 = NULL;
        LPKSPROPERTYSET pb1 = NULL;

        HRESULT ra1 = IDirectSoundBuffer_QueryInterface(a, &IID_IKsPropertySet, &pa1);
        HRESULT rb1 = IDirectSoundBuffer_QueryInterface(b, &IID_IKsPropertySet, &pb1);

        if (ra1 != rb1) {
            DebugBreak(); return FALSE;
        }

        if (pa1 == NULL || pb1 == NULL) {
            DebugBreak(); return FALSE;
        }

        LPKSPROPERTYSET pa2 = NULL;
        LPKSPROPERTYSET pb2 = NULL;

        HRESULT ra2 = IDirectSoundBuffer_QueryInterface(a, &IID_IKsPropertySet, &pa2);
        HRESULT rb2 = IDirectSoundBuffer_QueryInterface(b, &IID_IKsPropertySet, &pb2);

        if (ra2 != rb2) {
            DebugBreak(); return FALSE;
        }

        if (pa2 == NULL || pb2 == NULL) {
            DebugBreak(); return FALSE;
        }

        if (pa1 != pa2 || pb1 != pb2) {
            DebugBreak(); return FALSE;
        }

        IKsPropertySet_Release(pa1);
        IKsPropertySet_Release(pb1);
        IKsPropertySet_Release(pa2);
        IKsPropertySet_Release(pb2);
    }

    {
        LPREFERENCECLOCK ca = NULL;
        LPREFERENCECLOCK cb = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IReferenceClock, &ca);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IReferenceClock, &cb);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        LPUNKNOWN ua = NULL;
        LPUNKNOWN ub = NULL;

        HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IUnknown, &ua);
        HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IUnknown, &ub);

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
            LPDIRECTSOUNDBUFFER dsa = NULL;
            LPDIRECTSOUNDBUFFER dsb = NULL;

            HRESULT ria = IUnknown_QueryInterface(ua, &IID_IDirectSoundBuffer, &dsa);
            HRESULT rib = IUnknown_QueryInterface(ub, &IID_IDirectSoundBuffer, &dsb);

            if (ria != rib) {
                DebugBreak(); return FALSE;
            }

            if (a != dsa || b != dsb) {
                DebugBreak(); return FALSE;
            }

            IDirectSound_AddRef(dsa);
            IDirectSound_AddRef(dsb);

            ULONG rcda = IDirectSound_Release(dsa);
            ULONG rcdb = IDirectSound_Release(dsb);

            if (rcda != rcdb) {
                DebugBreak(); return FALSE;
            }

            IDirectSoundBuffer_Release(dsa);
            IDirectSoundBuffer_Release(dsb);
        }

        IUnknown_Release(ua);
        IUnknown_Release(ub);
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferPrimaryQueryInterfaces(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD flags, DWORD level) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desc;
    ZeroMemory(&desc, sizeof(DSBUFFERDESC));

    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = flags;

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsa == NULL || dsb == NULL) {
        DebugBreak(); return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, level);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, level);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferQueryInterfaces(dsba, dsbb)) {
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

    if (dsa != NULL) {
        IDirectSound_Release(dsa);
    }

    if (dsb != NULL) {
        IDirectSound_Release(dsb);
    }

    return result;
}

BOOL TestDirectSoundBufferPrimaryQueryInterface(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        DebugBreak(); return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = (LPDIRECTSOUNDCREATE)GetProcAddress(a, "DirectSoundCreate");
    LPDIRECTSOUNDCREATE dscb = (LPDIRECTSOUNDCREATE)GetProcAddress(b, "DirectSoundCreate");

    if (dsca == NULL || dscb == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;

    HWND wa = InitWindow(WINDOW_NAME);
    HWND wb = InitWindow(WINDOW_NAME);

    if (wa == NULL || wb == NULL) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < COOPERATIVE_LEVEL_COUNT; i++) {
        for (int k = 0; k < BUFFER_FLAG_COUNT; k++) {
            if (!TestDirectSoundBufferPrimaryQueryInterfaces(dsca, wa, dscb, wb,
                BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                goto exit;
            }
        }
    }

exit:

    if (wa != NULL) {
        DestroyWindow(wa);
    }

    if (wb != NULL) {
        DestroyWindow(wb);
    }

    UnregisterClassA(WINDOW_NAME, GetModuleHandleA(NULL));

    return result;
}
