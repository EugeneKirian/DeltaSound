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

#define MAX_DEVICE_NAME_LENGTH  128

#define DIRECTSOUND_DEVICE_PROPERTY_COUNT   8

static const DSPROPERTY_DIRECTSOUNDDEVICE DirectSoundDeviceProperties[DIRECTSOUND_DEVICE_PROPERTY_COUNT] = {
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A,
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1,
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W,
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A,
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W
};

static BOOL CompareDeviceData(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA a,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (!IsEqualGUID(&a->DeviceId, &b->DeviceId)) {
        return FALSE;
    }

    if (a->Type != b->Type || a->DataFlow != b->DataFlow
        || a->WaveDeviceId != b->WaveDeviceId || a->Devnode != b->Devnode) {
        return FALSE;
    }

    if (strcmp(a->DescriptionA, b->DescriptionA) != 0) {
        return FALSE;
    }

    if (strcmp(a->ModuleA, b->ModuleA) != 0) {
        return FALSE;
    }

    if (wcscmp(a->DescriptionW, b->DescriptionW) != 0) {
        return FALSE;
    }

    if (wcscmp(a->ModuleW, b->ModuleW) != 0) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetQuerySupport(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    HRESULT ra = S_OK, rb = S_OK;

    {
        ra = IKsPropertySet_QuerySupport(a, NULL, 0, NULL);
        rb = IKsPropertySet_QuerySupport(b, NULL, 0, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ra = IKsPropertySet_QuerySupport(a, &DSPROPSETID_DirectSoundDevice, 0, NULL);
        rb = IKsPropertySet_QuerySupport(b, &DSPROPSETID_DirectSoundDevice, 0, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    for (int i = 0; i < DIRECTSOUND_DEVICE_PROPERTY_COUNT; i++) {
        ULONG sa = 0, sb = 0;

        ra = IKsPropertySet_QuerySupport(a,
            &DSPROPSETID_DirectSoundDevice, DirectSoundDeviceProperties[i], &sa);
        rb = IKsPropertySet_QuerySupport(b,
            &DSPROPSETID_DirectSoundDevice, DirectSoundDeviceProperties[i], &sb);

        if (ra != rb || sa != sb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingA(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, &da, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, &db, 0, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, &da, length - 4, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, &db, length - 4, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    // NULL Device Name
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0,
            &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0,
            &db, length, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }

        if (memcmp(&da, &db, length) != 0) {
            return FALSE;
        }
    }

    // Invalid Device Name
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA);

        CHAR name[MAX_DEVICE_NAME_LENGTH] = "Invalid Device Name";

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DeviceName = name;
        db.DeviceName = name;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0,
            &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0,
            &db, length, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }

        if (memcmp(&da, &db, length) != 0) {
            return FALSE;
        }
    }

    // Valid In Device Name
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA);

        const UINT count = waveInGetNumDevs();

        for (UINT i = 0; i < count; i++) {
            WAVEINCAPSA caps;
            MMRESULT result = waveInGetDevCapsA(i, &caps, sizeof(WAVEINCAPSA));

            if (result == MMSYSERR_NOERROR) {
                DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA da, db;
                ZeroMemory(&da, length);
                ZeroMemory(&db, length);

                da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
                da.DeviceName = caps.szPname;

                db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
                db.DeviceName = caps.szPname;

                const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0,
                    &da, length, &la);
                const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0,
                    &db, length, &lb);

                if (ra != rb || la != lb) {
                    return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    return FALSE;
                }
            }
        }
    }

    // Valid Out Device Name
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA);

        const UINT count = waveOutGetNumDevs();

        for (UINT i = 0; i < count; i++) {
            WAVEOUTCAPSA caps;
            MMRESULT result = waveOutGetDevCapsA(i, &caps, sizeof(WAVEOUTCAPSA));

            if (result == MMSYSERR_NOERROR) {
                DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA da, db;
                ZeroMemory(&da, length);
                ZeroMemory(&db, length);

                da.DeviceName = caps.szPname;
                db.DeviceName = caps.szPname;

                const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, &da, length, &la);
                const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, &db, length, &lb);

                if (ra != rb || la != lb) {
                    return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingW(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, &da, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, &db, 0, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, &da, length - 4, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, &db, length - 4, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    // NULL Device Name
    // NOTE. There is a bug in DirectSound, dereferencing null-pointer for device name.

    // Invalid Device Name
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA);

        WCHAR name[MAX_DEVICE_NAME_LENGTH] = L"Invalid Device Name";

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DeviceName = name;
        db.DeviceName = name;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0,
            &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0,
            &db, length, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }

        if (memcmp(&da, &db, length) != 0) {
            return FALSE;
        }
    }

    // Valid In Device Name
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA);

        const UINT count = waveInGetNumDevs();

        for (UINT i = 0; i < count; i++) {
            WAVEINCAPSW caps;
            MMRESULT result = waveInGetDevCapsW(i, &caps, sizeof(WAVEINCAPSW));

            if (result == MMSYSERR_NOERROR) {
                DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA da, db;
                ZeroMemory(&da, length);
                ZeroMemory(&db, length);

                da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
                da.DeviceName = caps.szPname;

                db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
                db.DeviceName = caps.szPname;

                const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0,
                    &da, length, &la);
                const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0,
                    &db, length, &lb);

                if (ra != rb || la != lb) {
                    return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    return FALSE;
                }
            }
        }
    }

    // Valid Out Device Name
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA);

        const UINT count = waveOutGetNumDevs();

        for (UINT i = 0; i < count; i++) {
            WAVEOUTCAPSW caps;
            MMRESULT result = waveOutGetDevCapsW(i, &caps, sizeof(WAVEOUTCAPSW));

            if (result == MMSYSERR_NOERROR) {
                DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA da, db;
                ZeroMemory(&da, length);
                ZeroMemory(&db, length);

                da.DeviceName = caps.szPname;
                db.DeviceName = caps.szPname;

                const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0,
                    &da, length, &la);
                const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0,
                    &db, length, &lb);

                if (ra != rb || la != lb) {
                    return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescription1(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, 0, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length - 4, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length - 4, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }
    }

    // DIRECTSOUNDDEVICE_DATAFLOW_RENDER (Default Device)
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            return FALSE;
        }
    }

    // DIRECTSOUNDDEVICE_DATAFLOW_RENDER & Invalid DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        memset(&da.DeviceId, 0xFF, sizeof(GUID));

        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        memset(&db.DeviceId, 0xFF, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            return FALSE;
        }
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_RENDER & DeviceID



    // DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE (Default Device)
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            return FALSE;
        }
    }

    // DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE & Invalid DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        memset(&da.DeviceId, 0xFF, sizeof(GUID));

        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        memset(&db.DeviceId, 0xFF, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            return FALSE;
        }
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE & DeviceID



    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGet(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A
    if (!TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingA(a, b)) {
        return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1
    if (!TestDirectSoundPrivateKsPropertySetGetDescription1(a, b)) {
        return FALSE;
    }

    // TODO DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1

    //DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W
    if (!TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingW(a, b)) {
        return FALSE;
    }

    //TODO DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A
    //TODO DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W
    //TODO DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A
    //TODO DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySet(LPCLASSFACTORY a, LPCLASSFACTORY b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPKSPROPERTYSET kspa = NULL, kspb = NULL;

    const HRESULT ra = IClassFactory_CreateInstance(a, NULL, &IID_IKsPropertySet, &kspa);
    const HRESULT rb = IClassFactory_CreateInstance(b, NULL, &IID_IKsPropertySet, &kspb);

    if (ra != rb || kspa == NULL || kspb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundPrivateKsPropertySetQuerySupport(kspa, kspb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundPrivateKsPropertySetGet(kspa, kspb)) {
        result = FALSE;
        goto exit;
    }

    // TODO GET

    // TODO SET

exit:

    RELEASE(kspa);
    RELEASE(kspb);

    return result;
}

static BOOL TestDirectSoundPrivateInstance(LPCLASSFACTORY a, LPCLASSFACTORY b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPUNKNOWN dspa = NULL, dspb = NULL;

    const HRESULT ra = IClassFactory_CreateInstance(a, NULL, &IID_IDirectSoundPrivate, &dspa);
    const HRESULT rb = IClassFactory_CreateInstance(b, NULL, &IID_IDirectSoundPrivate, &dspb);

    if (ra != rb || dspa != NULL || dspb != NULL) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dspa);
    RELEASE(dspb);

    return result;
}

BOOL TestDllGetClassObjectDirectSoundPrivate(HMODULE a, HMODULE b) {
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

    if (FAILED(ra = gcoa(&CLSID_DirectSoundPrivate, &IID_IClassFactory, &cfa))) {
        result = FALSE;
        goto exit;
    }

    if (FAILED(rb = gcob(&CLSID_DirectSoundPrivate, &IID_IClassFactory, &cfb))) {
        result = FALSE;
        goto exit;
    }

    if (ra != rb || cfa == NULL || cfb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundPrivateInstance(cfa, cfb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundPrivateKsPropertySet(cfa, cfb)) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(cfa);
    RELEASE(cfb);

    return result;
}
