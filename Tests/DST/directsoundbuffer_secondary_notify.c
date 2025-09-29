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

#include "directsoundbuffer_secondary.h"

static BOOL TestDirectSoundBufferSecondaryNotifyCreate(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDNOTIFY na1 = NULL, nb1 = NULL;

    HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundNotify, &na1);
    HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundNotify, &nb1);

    if (ra != rb) {
        return FALSE;
    }

    if (na1 == NULL || nb1 == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDNOTIFY na2 = NULL, nb2 = NULL;

    ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundNotify, &na2);
    rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundNotify, &nb2);

    if (na1 != na2 || nb1 != nb2) {
        return FALSE;
    }

    RELEASE(na1);
    RELEASE(nb1);
    RELEASE(na2);
    RELEASE(nb2);

    return TRUE;
}

static BOOL TestDirectSoundNotifyQueryInterfaces(LPDIRECTSOUNDNOTIFY a, LPDIRECTSOUNDNOTIFY b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    {
        LPDIRECTSOUND dsa = NULL, dsb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, NULL, &dsa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, NULL, &dsb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND dsa = NULL, dsb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &GUID_NULL, &dsa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &GUID_NULL, &dsb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND dsa = NULL, dsb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSound, &dsa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSound, &dsb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND3DBUFFER ds3dba = NULL, ds3dbb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSound3DBuffer, &ds3dba);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSound3DBuffer, &ds3dbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUND3DLISTENER ds3dla = NULL, ds3dlb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSound3DListener, &ds3dla);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSound3DListener, &ds3dlb);

        if (ra != rb) {
            return FALSE;
        }

        RELEASE(ds3dla);
        RELEASE(ds3dlb);
    }

    {
        LPDIRECTSOUND8 ds8a = NULL, ds8b = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSound8, &ds8a);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSound8, &ds8b);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundBuffer, &dsba);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundBuffer, &dsbb);

        if (ra != rb) {
            return FALSE;
        }

        if (dsba == NULL || dsbb == NULL) {
            return FALSE;
        }

        IDirectSoundBuffer_Release(dsba);
        IDirectSoundBuffer_Release(dsbb);
    }

    {
        LPDIRECTSOUNDBUFFER8 dsba = NULL, dsbb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundBuffer8, &dsba);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundBuffer8, &dsbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTURE dsca = NULL ,dscb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundCapture, &dsca);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundCapture, &dscb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER dscba = NULL, dscbb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundCaptureBuffer, &dscba);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundCaptureBuffer, &dscbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREBUFFER8 dscba = NULL, dscbb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundCaptureBuffer8, &dscba);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundCaptureBuffer8, &dscbb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXAEC fxa = NULL ,fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundCaptureFXAec, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundCaptureFXAec, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundCaptureFXNoiseSuppress, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCHORUS fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXChorus, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXChorus, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXCOMPRESSOR fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXCompressor, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXCompressor, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXDISTORTION fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXDistortion, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXDistortion, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXECHO fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXEcho, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXEcho, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXFLANGER fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXFlanger, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXFlanger, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXGARGLE fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXGargle, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXGargle, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXI3DL2REVERB fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXI3DL2Reverb, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXI3DL2Reverb, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXPARAMEQ fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXParamEq, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXParamEq, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFXWAVESREVERB fxa = NULL, fxb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFXWavesReverb, &fxa);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFXWavesReverb, &fxb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDFULLDUPLEX fda = NULL, fdb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundFullDuplex, &fda);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundFullDuplex, &fdb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPDIRECTSOUNDNOTIFY sna = NULL, snb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IDirectSoundNotify, &sna);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IDirectSoundNotify, &snb);

        if (ra != rb) {
            return FALSE;
        }

        if (sna == NULL || snb == NULL) {
            return FALSE;
        }

        RELEASE(sna);
        RELEASE(snb);
    }

    {
        LPKSPROPERTYSET pa1 = NULL, pb1 = NULL;

        HRESULT ra1 = IDirectSoundNotify_QueryInterface(a, &IID_IKsPropertySet, &pa1);
        HRESULT rb1 = IDirectSoundNotify_QueryInterface(b, &IID_IKsPropertySet, &pb1);

        if (ra1 != rb1) {
            return FALSE;
        }

        if (pa1 == NULL || pb1 == NULL) {
            return FALSE;
        }

        LPKSPROPERTYSET pa2 = NULL, pb2 = NULL;

        HRESULT ra2 = IDirectSoundNotify_QueryInterface(a, &IID_IKsPropertySet, &pa2);
        HRESULT rb2 = IDirectSoundNotify_QueryInterface(b, &IID_IKsPropertySet, &pb2);

        if (ra2 != rb2) {
            return FALSE;
        }

        if (pa2 == NULL || pb2 == NULL) {
            return FALSE;
        }

        if (pa1 != pa2 || pb1 != pb2) {
            return FALSE;
        }

        RELEASE(pa1);
        RELEASE(pb1);
        RELEASE(pa2);
        RELEASE(pb2);
    }

    {
        LPREFERENCECLOCK ca = NULL, cb = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IReferenceClock, &ca);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IReferenceClock, &cb);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPUNKNOWN ua = NULL, ub = NULL;

        HRESULT ra = IDirectSoundNotify_QueryInterface(a, &IID_IUnknown, &ua);
        HRESULT rb = IDirectSoundNotify_QueryInterface(b, &IID_IUnknown, &ub);

        if (ra != rb) {
            return FALSE;
        }

        if (ua == NULL || ub == NULL) {
            return FALSE;
        }

        {
            LPDIRECTSOUNDBUFFER dsa = NULL, dsb = NULL;

            HRESULT ria = IUnknown_QueryInterface(ua, &IID_IDirectSoundBuffer, &dsa);
            HRESULT rib = IUnknown_QueryInterface(ub, &IID_IDirectSoundBuffer, &dsb);

            if (ria != rib) {
                return FALSE;
            }

            if (dsa == NULL || dsb == NULL) {
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

static BOOL TestDirectSoundBufferSecondaryNotifyQueryInterfaces(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDNOTIFY na = NULL, nb = NULL;

    HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundNotify, &na);
    HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundNotify, &nb);

    if (ra != rb) {
        return FALSE;
    }

    if (na == NULL || nb == NULL) {
        return FALSE;
    }

    const BOOL result = TestDirectSoundNotifyQueryInterfaces(na, nb);

    RELEASE(na);
    RELEASE(nb);

    return result;
}

static BOOL TestDirectSoundBufferSecondaryNotifySet(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDNOTIFY na = NULL, nb = NULL;

    HRESULT ra = IDirectSoundBuffer_QueryInterface(a, &IID_IDirectSoundNotify, &na);
    HRESULT rb = IDirectSoundBuffer_QueryInterface(b, &IID_IDirectSoundNotify, &nb);

    if (ra != rb) {
        return FALSE;
    }

    if (na == NULL || nb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    // Invalid
    {
        ra = IDirectSoundNotify_SetNotificationPositions(na, 1, NULL);
        rb = IDirectSoundNotify_SetNotificationPositions(nb, 1, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // No notifications
    {
        ra = IDirectSoundNotify_SetNotificationPositions(na, 0, NULL);
        rb = IDirectSoundNotify_SetNotificationPositions(nb, 0, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    // One notification
    {
        DSBPOSITIONNOTIFY notification;
        ZeroMemory(&notification, sizeof(DSBPOSITIONNOTIFY));

        // Invalid
        {
            ra = IDirectSoundNotify_SetNotificationPositions(na, 1, &notification);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 1, &notification);

            if (ra != rb) {
                return FALSE;
            }
        }

        // Invalid
        {
            notification.dwOffset = 256;
            notification.hEventNotify = (HANDLE)1;

            ra = IDirectSoundNotify_SetNotificationPositions(na, 1, &notification);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 1, &notification);

            if (ra != rb) {
                return FALSE;
            }
        }

        // Valid
        {
            notification.dwOffset = 0;
            notification.hEventNotify = (HANDLE)1;

            ra = IDirectSoundNotify_SetNotificationPositions(na, 1, &notification);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 1, &notification);

            if (ra != rb) {
                return FALSE;
            }
        }
        
        // Valid
        {
            notification.dwOffset = DSBPN_OFFSETSTOP;
            notification.hEventNotify = (HANDLE)1;

            ra = IDirectSoundNotify_SetNotificationPositions(na, 1, &notification);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 1, &notification);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    // Some notifications
    {
        DSBPOSITIONNOTIFY notifications[4];

        // Invalid
        {
            notifications[0].hEventNotify = (HANDLE)1;
            notifications[1].hEventNotify = (HANDLE)2;
            notifications[2].hEventNotify = (HANDLE)3;
            notifications[3].hEventNotify = (HANDLE)0xCCCCCCCC;

            ra = IDirectSoundNotify_SetNotificationPositions(na, 4, notifications);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 4, notifications);

            if (ra != rb) {
                return FALSE;
            }
        }

        // Invalid
        {
            notifications[0].dwOffset = 1;
            notifications[1].dwOffset = 2;
            notifications[2].dwOffset = 3;
            notifications[3].dwOffset = DSBPN_OFFSETSTOP - 1;

            ra = IDirectSoundNotify_SetNotificationPositions(na, 4, notifications);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 4, notifications);

            if (ra != rb) {
                return FALSE;
            }
        }

        // Valid
        {
            notifications[3].dwOffset = 4;

            ra = IDirectSoundNotify_SetNotificationPositions(na, 4, notifications);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 4, notifications);

            if (ra != rb) {
                return FALSE;
            }
        }

        // Valid
        {
            notifications[0].dwOffset = 8;
            notifications[1].dwOffset = 6;
            notifications[2].dwOffset = 4;
            notifications[3].dwOffset = 2;

            ra = IDirectSoundNotify_SetNotificationPositions(na, 4, notifications);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, 4, notifications);

            if (ra != rb) {
                return FALSE;
            }
        }

        // Max Notifications
        {
            ra = IDirectSoundNotify_SetNotificationPositions(na, DSBNOTIFICATIONS_MAX + 1, notifications);
            rb = IDirectSoundNotify_SetNotificationPositions(nb, DSBNOTIFICATIONS_MAX + 1, notifications);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    RELEASE(na);
    RELEASE(nb);

    return result;
}

BOOL TestDirectSoundBufferSecondaryNotify(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL, dsb = NULL;

    HRESULT ra = dsca(NULL, &dsa, NULL);
    HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 1, 22050, 8);

    DSBUFFERDESC desc;
    InitializeDirectSoundBufferDesc(&desc, DSBCAPS_CTRLPOSITIONNOTIFY, 176400, &format);

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferSecondaryNotifyCreate(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferSecondaryNotifyQueryInterfaces(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferSecondaryNotifySet(dsba, dsbb)) {
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
