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

#define MAX_ENUMERATE_DEVICE_COUNT  128

#define MAX_DEVICE_NAME_LENGTH      128

typedef struct DeviceEnumerateContext {
    DWORD Count;
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA Items;
} DeviceEnumerateContext;

typedef struct DeviceDescriptionA {
    DIRECTSOUNDDEVICE_TYPE      Type;
    DIRECTSOUNDDEVICE_DATAFLOW  DataFlow;
    GUID                        DeviceId;
    CHAR                        Description[0x100];
    CHAR                        Module[MAX_PATH];
    CHAR                        Interface[0x100];
    ULONG                       WaveDeviceId;
} DeviceDescriptionA;

typedef struct DeviceEnumerateContextA {
    DWORD Count;
    DeviceDescriptionA* Items;
} DeviceEnumerateContextA;

typedef struct DeviceDescriptionW {
    DIRECTSOUNDDEVICE_TYPE      Type;
    DIRECTSOUNDDEVICE_DATAFLOW  DataFlow;
    GUID                        DeviceId;
    WCHAR                       Description[0x100];
    WCHAR                       Module[MAX_PATH];
    WCHAR                       Interface[0x100];
    ULONG                       WaveDeviceId;
} DeviceDescriptionW;

typedef struct DeviceEnumerateContextW {
    DWORD Count;
    DeviceDescriptionW* Items;
} DeviceEnumerateContextW;

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

static HRESULT CreateDescriptionA(DWORD dwSize, DIRECTSOUNDDEVICE_DATAFLOW dfDataFlow,
    LPCGUID lpcID, PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA* ppOut) {
    if (dwSize < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA)) {
        return E_INVALIDARG;
    }

    if (ppOut == NULL) {
        return E_INVALIDARG;
    }

    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA instance =
        (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA)malloc(dwSize);

    if (instance == NULL) {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(instance, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA));

    instance->DataFlow = dfDataFlow;

    if (lpcID != NULL) {
        CopyMemory(&instance->DeviceId, lpcID, sizeof(GUID));
    }

    *ppOut = instance;

    return S_OK;
}

static HRESULT CreateDescriptionW(DWORD dwSize, DIRECTSOUNDDEVICE_DATAFLOW dfDataFlow,
    LPCGUID lpcID, PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA* ppOut) {
    if (dwSize < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA)) {
        return E_INVALIDARG;
    }

    if (ppOut == NULL) {
        return E_INVALIDARG;
    }

    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA instance =
        (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA)malloc(dwSize);

    if (instance == NULL) {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(instance, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA));

    instance->DataFlow = dfDataFlow;

    if (lpcID != NULL) {
        CopyMemory(&instance->DeviceId, lpcID, sizeof(GUID));
    }

    *ppOut = instance;

    return S_OK;
}

static BOOL CompareDeviceData(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA a,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (!IsEqualGUID(&a->DeviceId, &b->DeviceId)) {
        DebugBreak(); return FALSE;
    }

    if (a->Type != b->Type || a->DataFlow != b->DataFlow
        || a->WaveDeviceId != b->WaveDeviceId || a->Devnode != b->Devnode) {
        DebugBreak(); return FALSE;
    }

    if (strcmp(a->DescriptionA, b->DescriptionA) != 0) {
        DebugBreak(); return FALSE;
    }

    if (strcmp(a->ModuleA, b->ModuleA) != 0) {
        DebugBreak(); return FALSE;
    }

    if (wcscmp(a->DescriptionW, b->DescriptionW) != 0) {
        DebugBreak(); return FALSE;
    }

    if (wcscmp(a->ModuleW, b->ModuleW) != 0) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

static BOOL CompareDeviceDescriptionA(DeviceDescriptionA* a, DeviceDescriptionA* b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (a->Type != b->Type
        || a->DataFlow != b->DataFlow || a->WaveDeviceId != b->WaveDeviceId) {
        DebugBreak(); return FALSE;
    }

    if (!IsEqualGUID(&a->DeviceId, &b->DeviceId)) {
        DebugBreak(); return FALSE;
    }

    if (strcmp(a->Description, b->Description) != 0) {
        DebugBreak(); return FALSE;
    }

    if (strcmp(a->Module, b->Module) != 0) {
        DebugBreak(); return FALSE;
    }

    if (stricmp(a->Interface, b->Interface) != 0) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

static BOOL CompareDeviceDescriptionW(DeviceDescriptionW* a, DeviceDescriptionW* b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (a->Type != b->Type
        || a->DataFlow != b->DataFlow || a->WaveDeviceId != b->WaveDeviceId) {
        DebugBreak(); return FALSE;
    }

    if (!IsEqualGUID(&a->DeviceId, &b->DeviceId)) {
        DebugBreak(); return FALSE;
    }

    if (wcscmp(a->Description, b->Description) != 0) {
        DebugBreak(); return FALSE;
    }

    if (wcscmp(a->Module, b->Module) != 0) {
        DebugBreak(); return FALSE;
    }

    if (wcsicmp(a->Interface, b->Interface) != 0) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

static BOOL CompareDeviceDataA(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA a,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (!IsEqualGUID(&a->DeviceId, &b->DeviceId)) {
        DebugBreak(); return FALSE;
    }

    if (a->Type != b->Type || a->DataFlow != b->DataFlow
        || a->WaveDeviceId != b->WaveDeviceId) {
        DebugBreak(); return FALSE;
    }

    if (a->Description != NULL && b->Description != NULL) {
        if (strcmp(a->Description, b->Description) != 0) {
            DebugBreak(); return FALSE;
        }
    }

    if (a->Module != NULL && b->Module != NULL) {
        if (strcmp(a->Module, b->Module) != 0) {
            DebugBreak(); return FALSE;
        }
    }

    if (a->Interface != NULL && b->Interface != NULL) {
        // NOTE. DirectSound converts Interface value to uppercase for RENDER devices...
        if (stricmp(a->Interface, b->Interface) != 0) {
            DebugBreak(); return FALSE;
        }
    }

    return TRUE;
}

static BOOL CompareDeviceDataW(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA a,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (!IsEqualGUID(&a->DeviceId, &b->DeviceId)) {
        DebugBreak(); return FALSE;
    }

    if (a->Type != b->Type || a->DataFlow != b->DataFlow
        || a->WaveDeviceId != b->WaveDeviceId) {
        DebugBreak(); return FALSE;
    }

    if (a->Description != NULL && b->Description != NULL) {
        if (wcscmp(a->Description, b->Description) != 0) {
            DebugBreak(); return FALSE;
        }
    }

    if (a->Module != NULL && b->Module != NULL) {
        if (wcscmp(a->Module, b->Module) != 0) {
            DebugBreak(); return FALSE;
        }
    }

    if (a->Interface != NULL && b->Interface != NULL) {
        // NOTE. DirectSound converts Interface value to uppercase for RENDER devices...
        if (wcsicmp(a->Interface, b->Interface) != 0) {
            DebugBreak(); return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetQuerySupport(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    HRESULT ra = S_OK, rb = S_OK;

    {
        ra = IKsPropertySet_QuerySupport(a, NULL, 0, NULL);
        rb = IKsPropertySet_QuerySupport(b, NULL, 0, NULL);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ra = IKsPropertySet_QuerySupport(a, &DSPROPSETID_DirectSoundDevice, 0, NULL);
        rb = IKsPropertySet_QuerySupport(b, &DSPROPSETID_DirectSoundDevice, 0, NULL);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    for (int i = 0; i < DIRECTSOUND_DEVICE_PROPERTY_COUNT; i++) {
        ULONG sa = 0, sb = 0;

        ra = IKsPropertySet_QuerySupport(a,
            &DSPROPSETID_DirectSoundDevice, DirectSoundDeviceProperties[i], &sa);
        rb = IKsPropertySet_QuerySupport(b,
            &DSPROPSETID_DirectSoundDevice, DirectSoundDeviceProperties[i], &sb);

        if (ra != rb || sa != sb) {
            DebugBreak(); return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingA(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
        }

        if (memcmp(&da, &db, length) != 0) {
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
        }

        if (memcmp(&da, &db, length) != 0) {
            DebugBreak(); return FALSE;
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
                    DebugBreak(); return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    DebugBreak(); return FALSE;
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
                    DebugBreak(); return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    DebugBreak(); return FALSE;
                }
            }
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingW(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
        }

        if (memcmp(&da, &db, length) != 0) {
            DebugBreak(); return FALSE;
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
                    DebugBreak(); return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    DebugBreak(); return FALSE;
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
                    DebugBreak(); return FALSE;
                }

                if (memcmp(&da, &db, length) != 0) {
                    DebugBreak(); return FALSE;
                }
            }
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescription1Render(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    GUID valid;
    ZeroMemory(&valid, sizeof(GUID));

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
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        CopyMemory(&da.DeviceId, &DSDEVID_DefaultPlayback, sizeof(GUID));

        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        CopyMemory(&db.DeviceId, &DSDEVID_DefaultPlayback, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }

        CopyMemory(&valid, &da.DeviceId, sizeof(GUID));
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
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_RENDER & DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        CopyMemory(&da.DeviceId, &valid, sizeof(GUID));

        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        CopyMemory(&db.DeviceId, &valid, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescription1Capture(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    GUID valid;
    ZeroMemory(&valid, sizeof(GUID));

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
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        CopyMemory(&da.DeviceId, &DSDEVID_DefaultCapture, sizeof(GUID));

        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        CopyMemory(&db.DeviceId, &DSDEVID_DefaultCapture, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }

        CopyMemory(&valid, &da.DeviceId, sizeof(GUID));
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
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE & DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        da.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        CopyMemory(&da.DeviceId, &valid, sizeof(GUID));

        db.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        CopyMemory(&db.DeviceId, &valid, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceData(&da, &db)) {
            DebugBreak(); return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescription1(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &da, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, NULL, 0, &db, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
        }
    }

    if (!TestDirectSoundPrivateKsPropertySetGetDescription1Render(a, b)) {
        DebugBreak(); return FALSE;
    }

    if (!TestDirectSoundPrivateKsPropertySetGetDescription1Capture(a, b)) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescriptionRenderA(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    GUID valid;
    ZeroMemory(&valid, sizeof(GUID));

    // DIRECTSOUNDDEVICE_DATAFLOW_RENDER (Default Device)
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &da);
            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(la, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &da);
        CreateDescriptionA(lb, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &da);
            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(la, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &da);
        CreateDescriptionA(lb, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        CopyMemory(&valid, &da->DeviceId, sizeof(GUID));

        free(da);
        free(db);
    }

    // DIRECTSOUNDDEVICE_DATAFLOW_RENDER & Invalid DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &da);
        CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &db);

        memset(&da->DeviceId, 0xFF, sizeof(GUID));
        memset(&db->DeviceId, 0xFF, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_RENDER & DeviceID
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &da);
            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(la, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &da);
        CreateDescriptionA(lb, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescriptionCaptureA(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    GUID valid;
    ZeroMemory(&valid, sizeof(GUID));

    // DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE (Default Device)
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &da);
            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(la, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &da);
        CreateDescriptionA(lb, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &da);
            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(la, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &da);
        CreateDescriptionA(lb, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        CopyMemory(&valid, &da->DeviceId, sizeof(GUID));

        free(da);
        free(db);
    }

    // DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE & Invalid DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &da);
        CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &db);

        memset(&da->DeviceId, 0xFF, sizeof(GUID));
        memset(&db->DeviceId, 0xFF, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE & DeviceID
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &da);
            CreateDescriptionA(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da = NULL, db = NULL;

        CreateDescriptionA(la, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &da);
        CreateDescriptionA(lb, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataA(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescriptionA(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, &da, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, &db, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, &da, length - 4, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, NULL, 0, &db, length - 4, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }
    }

    if (!TestDirectSoundPrivateKsPropertySetGetDescriptionRenderA(a, b)) {
        DebugBreak(); return FALSE;
    }

    if (!TestDirectSoundPrivateKsPropertySetGetDescriptionCaptureA(a, b)) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescriptionRenderW(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    GUID valid;
    ZeroMemory(&valid, sizeof(GUID));

    // DIRECTSOUNDDEVICE_DATAFLOW_RENDER (Default Device)
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &da);
            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(la, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &da);
        CreateDescriptionW(lb, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &da);
            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(la, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &da);
        CreateDescriptionW(lb, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &DSDEVID_DefaultPlayback, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        CopyMemory(&valid, &da->DeviceId, sizeof(GUID));

        free(da);
        free(db);
    }

    // DIRECTSOUNDDEVICE_DATAFLOW_RENDER & Invalid DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &da);
        CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_RENDER, NULL, &db);

        memset(&da->DeviceId, 0xFF, sizeof(GUID));
        memset(&db->DeviceId, 0xFF, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_RENDER & DeviceID
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &da);
            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(la, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &da);
        CreateDescriptionW(lb, DIRECTSOUNDDEVICE_DATAFLOW_RENDER, &valid, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescriptionCaptureW(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    GUID valid;
    ZeroMemory(&valid, sizeof(GUID));

    // DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE (Default Device)
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &da);
            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(la, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &da);
        CreateDescriptionW(lb, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &da);
            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(la, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &da);
        CreateDescriptionW(lb, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &DSDEVID_DefaultCapture, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        CopyMemory(&valid, &da->DeviceId, sizeof(GUID));

        free(da);
        free(db);
    }

    // DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE & Invalid DeviceID
    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &da);
        CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
            DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, NULL, &db);

        memset(&da->DeviceId, 0xFF, sizeof(GUID));
        memset(&db->DeviceId, 0xFF, sizeof(GUID));

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    // TODO DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE & DeviceID
    {
        ULONG la = 0, lb = 0;

        {
            const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);
            PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &da);
            CreateDescriptionW(sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA),
                DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &db);

            // Length

            const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, length, &la);
            const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, length, &lb);

            if (ra != rb || la != lb) {
                free(da);
                free(db);
                DebugBreak(); return FALSE;
            }

            free(da);
            free(db);
        }

        // Values

        PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da = NULL, db = NULL;

        CreateDescriptionW(la, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &da);
        CreateDescriptionW(lb, DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE, &valid, &db);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, da, la, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, db, lb, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }

        if (!CompareDeviceDataW(da, db)) {
            free(da);
            free(db);

            DebugBreak(); return FALSE;
        }

        free(da);
        free(db);
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetGetDescriptionW(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    // Invalid
    {
        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, NULL, 0, NULL);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, NULL, 0, NULL);

        if (ra != rb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, NULL, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, NULL, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, &da, 0, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, &db, 0, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }
    }

    {
        ULONG la = 0, lb = 0;
        const DWORD length = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);

        DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA da, db;
        ZeroMemory(&da, length);
        ZeroMemory(&db, length);

        const HRESULT ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, &da, length - 4, &la);
        const HRESULT rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, NULL, 0, &db, length - 4, &lb);

        if (ra != rb || la != lb) {
            DebugBreak(); return FALSE;
        }
    }

    if (!TestDirectSoundPrivateKsPropertySetGetDescriptionRenderA(a, b)) {
        DebugBreak(); return FALSE;
    }

    if (!TestDirectSoundPrivateKsPropertySetGetDescriptionCaptureA(a, b)) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

BOOL CALLBACK TestEnumerateCallback(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA lpData, LPVOID lpContext) {
    if (lpContext == NULL) {
        return FALSE;
    }

    DeviceEnumerateContext* ctx = (DeviceEnumerateContext*)lpContext;

    if (MAX_ENUMERATE_DEVICE_COUNT <= ctx->Count) {
        return FALSE;
    }

    CopyMemory(&ctx->Items[ctx->Count], lpData, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA));

    ctx->Count++;

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetEnumerate1(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;
    HRESULT ra = S_OK, rb = S_OK;
    ULONG la = 0, lb = 0;
    const DWORD size = MAX_ENUMERATE_DEVICE_COUNT
        * sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

    DeviceEnumerateContext ca, cb;
    ZeroMemory(&ca, sizeof(DeviceEnumerateContext));
    ZeroMemory(&cb, sizeof(DeviceEnumerateContext));

    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1_DATA da, db;

    da.Callback = TestEnumerateCallback;
    da.Context = &ca;

    db.Callback = TestEnumerateCallback;
    db.Context = &cb;

    ca.Items = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA)malloc(size);
    cb.Items = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA)malloc(size);

    if (ca.Items == NULL || cb.Items == NULL) {
        result = FALSE;
        goto exit;
    }

    ZeroMemory(ca.Items, size);
    ZeroMemory(cb.Items, size);

    ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
        DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1, NULL, 0,
        &da, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1_DATA), &la);
    rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
        DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1, NULL, 0,
        &db, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1_DATA), &lb);

    if (ra != rb || la != lb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (ca.Count != cb.Count) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    for (DWORD i = 0; i < ca.Count; i++) {
        if (!CompareDeviceData(&ca.Items[i], &cb.Items[i])) {
            result = FALSE;
            DebugBreak(); goto exit;
        }
    }

exit:

    if (ca.Items != NULL) {
        free(ca.Items);
    }

    if (cb.Items != NULL) {
        free(cb.Items);
    }

    return result;
}

BOOL CALLBACK TestEnumerateCallbackA(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA lpData, LPVOID lpContext) {
    if (lpContext == NULL) {
        return FALSE;
    }

    DeviceEnumerateContextA* ctx = (DeviceEnumerateContextA*)lpContext;

    if (MAX_ENUMERATE_DEVICE_COUNT <= ctx->Count) {
        return FALSE;
    }

    ctx->Items[ctx->Count].Type = lpData->Type;
    ctx->Items[ctx->Count].DataFlow = lpData->DataFlow;
    CopyMemory(&ctx->Items[ctx->Count].DeviceId, &lpData->DeviceId, sizeof(GUID));
    strcpy(ctx->Items[ctx->Count].Description, lpData->Description);
    strcpy(ctx->Items[ctx->Count].Module, lpData->Module);
    strcpy(ctx->Items[ctx->Count].Interface, lpData->Interface);
    ctx->Items[ctx->Count].WaveDeviceId = lpData->WaveDeviceId;

    ctx->Count++;

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetEnumerateA(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;
    HRESULT ra = S_OK, rb = S_OK;
    ULONG la = 0, lb = 0;
    const DWORD size = MAX_ENUMERATE_DEVICE_COUNT
        * sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);

    DeviceEnumerateContextA ca, cb;
    ZeroMemory(&ca, sizeof(DeviceEnumerateContextA));
    ZeroMemory(&cb, sizeof(DeviceEnumerateContextA));

    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A_DATA da, db;

    da.Callback = TestEnumerateCallbackA;
    da.Context = &ca;

    db.Callback = TestEnumerateCallbackA;
    db.Context = &cb;

    ca.Items = (DeviceDescriptionA*)malloc(size);
    cb.Items = (DeviceDescriptionA*)malloc(size);

    if (ca.Items == NULL || cb.Items == NULL) {
        result = FALSE;
        goto exit;
    }

    ZeroMemory(ca.Items, size);
    ZeroMemory(cb.Items, size);

    ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
        DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A, NULL, 0,
        &da, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A_DATA), &la);
    rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
        DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A, NULL, 0,
        &db, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A_DATA), &lb);

    if (ra != rb || la != lb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (ca.Count != cb.Count) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    for (DWORD i = 0; i < ca.Count; i++) {
        if (!CompareDeviceDescriptionA(&ca.Items[i], &cb.Items[i])) {
            result = FALSE;
            DebugBreak(); goto exit;
        }
    }

exit:

    if (ca.Items != NULL) {
        free(ca.Items);
    }

    if (cb.Items != NULL) {
        free(cb.Items);
    }

    return result;
}

BOOL CALLBACK TestEnumerateCallbackW(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA lpData, LPVOID lpContext) {
    if (lpContext == NULL) {
        return FALSE;
    }

    DeviceEnumerateContextW* ctx = (DeviceEnumerateContextW*)lpContext;

    if (MAX_ENUMERATE_DEVICE_COUNT <= ctx->Count) {
        return FALSE;
    }

    ctx->Items[ctx->Count].Type = lpData->Type;
    ctx->Items[ctx->Count].DataFlow = lpData->DataFlow;
    CopyMemory(&ctx->Items[ctx->Count].DeviceId, &lpData->DeviceId, sizeof(GUID));
    wcscpy(ctx->Items[ctx->Count].Description, lpData->Description);
    wcscpy(ctx->Items[ctx->Count].Module, lpData->Module);
    wcscpy(ctx->Items[ctx->Count].Interface, lpData->Interface);
    ctx->Items[ctx->Count].WaveDeviceId = lpData->WaveDeviceId;

    ctx->Count++;

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySetEnumerateW(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;
    HRESULT ra = S_OK, rb = S_OK;
    ULONG la = 0, lb = 0;
    const DWORD size = MAX_ENUMERATE_DEVICE_COUNT
        * sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA);

    DeviceEnumerateContextW ca, cb;
    ZeroMemory(&ca, sizeof(DeviceEnumerateContextW));
    ZeroMemory(&cb, sizeof(DeviceEnumerateContextW));

    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA da, db;

    da.Callback = TestEnumerateCallbackW;
    da.Context = &ca;

    db.Callback = TestEnumerateCallbackW;
    db.Context = &cb;

    ca.Items = (DeviceDescriptionW*)malloc(size);
    cb.Items = (DeviceDescriptionW*)malloc(size);

    if (ca.Items == NULL || cb.Items == NULL) {
        result = FALSE;
        goto exit;
    }

    ZeroMemory(ca.Items, size);
    ZeroMemory(cb.Items, size);

    ra = IKsPropertySet_Get(a, &DSPROPSETID_DirectSoundDevice,
        DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W, NULL, 0,
        &da, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA), &la);
    rb = IKsPropertySet_Get(b, &DSPROPSETID_DirectSoundDevice,
        DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W, NULL, 0,
        &db, sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA), &lb);

    if (ra != rb || la != lb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (ca.Count != cb.Count) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    for (DWORD i = 0; i < ca.Count; i++) {
        if (!CompareDeviceDescriptionW(&ca.Items[i], &cb.Items[i])) {
            result = FALSE;
            DebugBreak(); goto exit;
        }
    }

exit:

    if (ca.Items != NULL) {
        free(ca.Items);
    }

    if (cb.Items != NULL) {
        free(cb.Items);
    }

    return result;
}

static BOOL TestDirectSoundPrivateKsPropertySetGet(LPKSPROPERTYSET a, LPKSPROPERTYSET b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A
    if (!TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingA(a, b)) {
        DebugBreak(); return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1
    if (!TestDirectSoundPrivateKsPropertySetGetDescription1(a, b)) {
        DebugBreak(); return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1
    if (!TestDirectSoundPrivateKsPropertySetEnumerate1(a, b)) {
        DebugBreak(); return FALSE;
    }

    //DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W
    if (!TestDirectSoundPrivateKsPropertySetGetWaveDeviceMappingW(a, b)) {
        DebugBreak(); return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A
    if (!TestDirectSoundPrivateKsPropertySetGetDescriptionA(a, b)) {
        DebugBreak(); return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W
    if (!TestDirectSoundPrivateKsPropertySetGetDescriptionW(a, b)) {
        DebugBreak(); return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A
    if (!TestDirectSoundPrivateKsPropertySetEnumerateA(a, b)) {
        DebugBreak(); return FALSE;
    }

    // DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W
    if (!TestDirectSoundPrivateKsPropertySetEnumerateW(a, b)) {
        DebugBreak(); return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundPrivateKsPropertySet(LPCLASSFACTORY a, LPCLASSFACTORY b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
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

    // TODO SET
    // TODO SET
    // TODO SET
    // TODO SET
    // TODO SET
    // TODO SET
    // TODO SET

exit:

    RELEASE(kspa);
    RELEASE(kspb);

    return result;
}

static BOOL TestDirectSoundPrivateInstance(LPCLASSFACTORY a, LPCLASSFACTORY b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
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
        DebugBreak(); return FALSE;
    }

    LPFNGETCLASSOBJECT gcoa = GetDllGetClassObject(a);
    LPFNGETCLASSOBJECT gcob = GetDllGetClassObject(b);

    if (gcoa == NULL || gcob == NULL) {
        DebugBreak(); return FALSE;
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
