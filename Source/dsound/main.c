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

#include <deltasound.h>
#include <device_info.h>

#define DEVICEENUMERATE_ANSI    0
#define DEVICEENUMERATE_WIDE    1

typedef BOOL(CALLBACK* LPDEVICEENUMERATECALLBACK)(LPGUID, LPCVOID, LPCVOID, LPVOID);

VOID DELTACALL cleanup();

HRESULT DELTACALL enumerate_devices(DWORD dwType, DWORD dwWide, LPDEVICEENUMERATECALLBACK pCallback, LPVOID pContext);

static allocator* alc;
static deltasound* delta;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hinstDLL);

        if (SUCCEEDED(allocator_create(&alc))) {
            if (SUCCEEDED(deltasound_create(alc, &delta))) {
                return TRUE;
            }
        }

        cleanup();

        return FALSE;
    }
    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH: {
        if (lpvReserved == NULL) {
            cleanup();
        }

        break;
    }
    }

    return TRUE;
}

HRESULT WINAPI DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter) {
    if (ppDS == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (pUnkOuter != NULL) {
        return DSERR_NOAGGREGATION;
    }

    HRESULT hr = S_OK;
    LPDIRECTSOUND instance = NULL;

    if (SUCCEEDED(hr = deltasound_create_direct_sound(delta,
        &IID_IDirectSound, pcGuidDevice, &instance))) {
        *ppDS = instance;
    }

    return hr;
}

HRESULT WINAPI DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || delta == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(DEVICETYPE_RENDER, DEVICEENUMERATE_ANSI,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || delta == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(DEVICETYPE_RENDER, DEVICEENUMERATE_WIDE,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundCaptureCreate(LPCGUID lpcGUID, LPDIRECTSOUNDCAPTURE* ppDSC, LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || delta == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(DEVICETYPE_RECORD, DEVICEENUMERATE_ANSI,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || delta == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(DEVICETYPE_RECORD, DEVICEENUMERATE_WIDE,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8* ppDS8, LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8* ppDSC8, LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundFullDuplexCreate(
    LPCGUID pcGuidCaptureDevice,
    LPCGUID pcGuidRenderDevice,
    LPCDSCBUFFERDESC pcDSCBufferDesc,
    LPCDSBUFFERDESC pcDSBufferDesc,
    HWND hWnd,
    DWORD dwLevel,
    LPDIRECTSOUNDFULLDUPLEX* ppDSFD,
    LPDIRECTSOUNDCAPTUREBUFFER8* ppDSCBuffer8,
    LPDIRECTSOUNDBUFFER8* ppDSBuffer8,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest) {
    if (pGuidSrc == NULL || pGuidDest == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD type = DEVICETYPE_INVALID;
    DWORD kind = DEVICEKIND_INVALID;

    if (IsEqualGUID(&DSDEVID_DefaultPlayback, pGuidSrc)) {
        type = DEVICETYPE_RENDER;
        kind = DEVICEKIND_MULTIMEDIA;
    }
    else if (IsEqualGUID(&DSDEVID_DefaultVoicePlayback, pGuidSrc)) {
        type = DEVICETYPE_RENDER;
        kind = DEVICEKIND_COMMUNICATION;
    }
    else if (IsEqualGUID(&DSDEVID_DefaultCapture, pGuidSrc)) {
        type = DEVICETYPE_RECORD;
        kind = DEVICEKIND_MULTIMEDIA;
    }
    else if (IsEqualGUID(&DSDEVID_DefaultVoiceCapture, pGuidSrc)) {
        type = DEVICETYPE_RECORD;
        kind = DEVICEKIND_COMMUNICATION;
    }

    ZeroMemory(pGuidDest, sizeof(GUID));

    if (type != DEVICETYPE_INVALID && kind != DEVICEKIND_INVALID) {
        // Get device id for predefined DirectSound device types.
        device_info info;
        ZeroMemory(&info, sizeof(device_info));

        if (FAILED(hr = device_info_get_default_device(type, kind, &info))) {
            return DSERR_NODRIVER;
        }

        CopyMemory(pGuidDest, &info.ID, sizeof(GUID));

        return S_OK;
    }

    // Iterate through available devices to find a match...
    UINT count = 0;
    if (SUCCEEDED(hr = device_info_get_count(DEVICETYPE_ALL, &count))) {
        device_info* devices = NULL;

        if (SUCCEEDED(hr = allocator_allocate(alc, count * sizeof(device_info), &devices))) {
            if (SUCCEEDED(hr = device_info_get_devices(DEVICETYPE_ALL, &count, devices))) {
                for (UINT i = 0; i < count; i++) {
                    device_info* dev = &devices[i];

                    if (IsEqualGUID(pGuidSrc, &dev->ID)) {

                        CopyMemory(pGuidDest, &dev->ID, sizeof(GUID));

                        allocator_free(alc, devices);

                        return S_OK;
                    }
                }

                allocator_free(alc, devices);

                return DSERR_NODRIVER;
            }

            allocator_free(alc, devices);
        }
    }

    return hr;
}

HRESULT WINAPI DllCanUnloadNow() {
    if (alc == NULL || delta == NULL) {
        return S_OK;
    }

    return SUCCEEDED(deltasound_can_unload(delta)) ? S_OK : S_FALSE;
}

HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppOut) {
    if (rclsid == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    LPVOID instance = NULL;

    if (SUCCEEDED(hr = deltasound_create_class_factory(delta, rclsid, riid, &instance))) {
        *ppOut = instance;
    }

    return hr;
}

/* ---------------------------------------------------------------------- */

VOID DELTACALL cleanup() {
    if (delta != NULL) {
        deltasound_release(delta);
        delta = NULL;
    }

    if (alc != NULL) {
        allocator_release(alc);
        alc = NULL;
    }
}

const static LPCVOID AudioDriverNames[2][2] =
{
    { (LPCVOID)"Primary Sound Driver", (LPCVOID)L"Primary Sound Driver" },
    { (LPCVOID)"Primary Sound Capture Driver", (LPCVOID)L"Primary Sound Capture Driver" }
};

HRESULT DELTACALL enumerate_devices(DWORD dwType, DWORD dwWide, LPDEVICEENUMERATECALLBACK pCallback, LPVOID pContext) {
    UINT count = 0;
    HRESULT hr = S_OK;

    // Always report primary device, even when there are no audio devices...
    if (!pCallback(NULL, AudioDriverNames[dwType][dwWide],
        dwWide == DEVICEENUMERATE_ANSI ? (LPCVOID)"" : (LPCVOID)L"", pContext)) {
        return S_OK;
    }

    // Iterate through the rest of the system devices...
    if (SUCCEEDED(hr = device_info_get_count(dwType, &count))) {
        device_info* devices = NULL;

        if (SUCCEEDED(hr = allocator_allocate(alc, count * sizeof(device_info), &devices))) {
            if (SUCCEEDED(hr = device_info_get_devices(dwType, &count, devices))) {
                for (UINT i = 0; i < count; i++) {
                    device_info* dev = &devices[i];

                    if (dwWide == DEVICEENUMERATE_WIDE) {
                        if (!pCallback(&dev->ID, dev->Name, dev->Module, pContext)) {
                            break;
                        }
                    }
                    else {
                        CHAR name[MAX_DEVICE_ID_LENGTH];
                        ZeroMemory(name, MAX_DEVICE_ID_LENGTH);

                        WideCharToMultiByte(CP_ACP, 0,
                            dev->Name, -1, name, MAX_DEVICE_ID_LENGTH, NULL, NULL);

                        CHAR module[MAX_DEVICE_ID_LENGTH];
                        ZeroMemory(module, MAX_DEVICE_ID_LENGTH);

                        WideCharToMultiByte(CP_ACP, 0,
                            dev->Module, -1, module, MAX_DEVICE_ID_LENGTH, NULL, NULL);

                        if (!pCallback(&dev->ID, name, module, pContext)) {
                            break;
                        }
                    }
                }
            }

            allocator_free(alc, devices);
        }
    }

    return hr;
}
