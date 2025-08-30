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
#include <wasapi_device.h>

#include <dsound.h>

#define DEVICEENUMERATE_ANSI    0
#define DEVICEENUMERATE_WIDE    1

typedef BOOL(CALLBACK* LPDEVICEENUMERATECALLBACK)(LPGUID, LPCVOID, LPCVOID, LPVOID);

HRESULT DELTACALL enumerate_devices(
    DWORD dwType,
    DWORD dwWide,
    LPDEVICEENUMERATECALLBACK pCallback,
    LPVOID pContext);

static allocator* alc;
static deltasound* ds;

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved) {
    UNUSED(hinstDLL);

    // TODO NOT IMPLEMENTED

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
        if (SUCCEEDED(allocator_create(&alc))) {
            if (SUCCEEDED(deltasound_create(alc, &ds))) {
                return TRUE;
            }
        }

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
            if (ds != NULL) {
                deltasound_release(ds);
                ds = NULL;

                allocator_release(alc);
                alc = NULL;
            }
        }

        break;
    }
    }

    return TRUE;
}

HRESULT WINAPI DirectSoundCreate(
    LPCGUID pcGuidDevice,
    LPDIRECTSOUND* ppDS,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundEnumerateA(
    LPDSENUMCALLBACKA pDSEnumCallback,
    LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || ds == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(WASAPIDEVICETYPE_AUDIO, DEVICEENUMERATE_ANSI,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundEnumerateW(
    LPDSENUMCALLBACKW pDSEnumCallback,
    LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || ds == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(WASAPIDEVICETYPE_AUDIO, DEVICEENUMERATE_WIDE,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundCaptureCreate(
    LPCGUID lpcGUID,
    LPDIRECTSOUNDCAPTURE* ppDSC,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureEnumerateA(
    LPDSENUMCALLBACKA pDSEnumCallback,
    LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || ds == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(WASAPIDEVICETYPE_RECORD, DEVICEENUMERATE_ANSI,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundCaptureEnumerateW(
    LPDSENUMCALLBACKW pDSEnumCallback,
    LPVOID pContext) {
    if (pDSEnumCallback == NULL) {
        return DSERR_INVALIDPARAM;
    }

    if (alc == NULL || ds == NULL) {
        return E_FAIL;
    }

    return enumerate_devices(WASAPIDEVICETYPE_RECORD, DEVICEENUMERATE_WIDE,
        (LPDEVICEENUMERATECALLBACK)pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundCreate8(
    LPCGUID pcGuidDevice,
    LPDIRECTSOUND8* ppDS8,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureCreate8(
    LPCGUID pcGuidDevice,
    LPDIRECTSOUNDCAPTURE8* ppDSC8,
    LPUNKNOWN pUnkOuter) {
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

HRESULT WINAPI GetDeviceID(
    LPCGUID pGuidSrc,
    LPGUID pGuidDest) {
    if (pGuidSrc == NULL || pGuidDest == NULL) {
        return E_INVALIDARG;
    }

    if (IsEqualGUID(&DSDEVID_DefaultPlayback, pGuidSrc)) {
        // TODO
    }
    else if (IsEqualGUID(&DSDEVID_DefaultVoicePlayback, pGuidSrc)) {
        // TODO
    }
    else if (IsEqualGUID(&DSDEVID_DefaultCapture, pGuidSrc)) {
        // TODO
    }
    else if (IsEqualGUID(&DSDEVID_DefaultVoiceCapture, pGuidSrc)) {
        // TODO
    }

    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DllCanUnloadNow() {
    // TODO NOT IMPLEMENTED
    return S_OK;
}

HRESULT DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID* ppv) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

/* ---------------------------------------------------------------------- */

const static LPCVOID AudioDriverNames[2][2] =
{
    { (LPCVOID)"Primary Sound Driver", (LPCVOID)L"Primary Sound Driver" },
    { (LPCVOID)"Primary Sound Capture Driver", (LPCVOID)L"Primary Sound Capture Driver" }
};

HRESULT DELTACALL enumerate_devices(
    DWORD dwType,
    DWORD dwWide,
    LPDEVICEENUMERATECALLBACK pCallback,
    LPVOID pContext) {
    UINT count = 0;
    HRESULT hr = DS_OK;

    // Always report primary device, even when there is no audio devices...
    if (!pCallback(NULL, AudioDriverNames[dwType][dwWide],
        dwWide == DEVICEENUMERATE_ANSI ? (LPCVOID)"" : (LPCVOID)L"", pContext)) {
        return S_OK;
    }

    // The rest of the system devices...
    if (SUCCEEDED(hr = wasapi_device_get_count(dwType, &count))) {
        wasapi_device* devices = NULL;
        if (SUCCEEDED(hr = allocator_allocate(alc, count * sizeof(wasapi_device), &devices))) {
            ZeroMemory(devices, count * sizeof(wasapi_device));
            if (SUCCEEDED(hr = wasapi_device_get_devices(dwType, &count, devices))) {
                for (UINT i = 0; i < count; i++) {
                    wasapi_device* dev = &devices[i];
                    if (dwWide == DEVICEENUMERATE_WIDE) {
                        if (!pCallback(&dev->ID, dev->Name, dev->Module, pContext)) {
                            break;
                        }
                    }
                    else {
                        CHAR name[MAX_WASAPI_DEVICE_ID_LENGTH];
                        ZeroMemory(name, MAX_WASAPI_DEVICE_ID_LENGTH);

                        WideCharToMultiByte(CP_ACP, 0,
                            dev->Name, -1, name, MAX_WASAPI_DEVICE_ID_LENGTH, NULL, NULL);

                        CHAR module[MAX_WASAPI_DEVICE_ID_LENGTH];
                        ZeroMemory(module, MAX_WASAPI_DEVICE_ID_LENGTH);

                        WideCharToMultiByte(CP_ACP, 0,
                            dev->Module, -1, module, MAX_WASAPI_DEVICE_ID_LENGTH, NULL, NULL);

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
