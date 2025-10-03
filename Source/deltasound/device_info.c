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

#include "allocator.h"
#include "device_info.h"
#include "uuid.h"

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }

const PROPERTYKEY PKEY_AudioEndpoint_GUID = {
    { 0x1DA5D803, 0xD492, 0x4EDD, { 0x8C, 0x23, 0xE0, 0xC0, 0xFF, 0xEE, 0x7F, 0x0E} }, 4
};

HRESULT DELTACALL device_info_thread_wait(HANDLE thread);

typedef struct get_device_context {
    LPCGUID         ID;
    DWORD           Type;
    device_info*    Device;
} get_device_context;

DWORD WINAPI device_info_get_device_thread(get_device_context* pContext);

typedef struct get_devices_context {
    DWORD           Type;
    UINT*           Count;
    device_info*    Devices;
} get_devices_context;

DWORD WINAPI device_info_get_devices_thread(get_devices_context* pContext);

typedef struct get_default_device_context {
    DWORD           Type;
    DWORD           Kind;
    device_info*    Device;
} get_default_device_context;

DWORD WINAPI device_info_get_default_device_thread(get_default_device_context* pContext);

HRESULT DELTACALL device_info_get_id(IMMDevice* pDevice, LPGUID pID);
HRESULT DELTACALL device_info_get_module(IMMDevice* pDevice, LPWSTR pszId);
HRESULT DELTACALL device_info_get_name(IMMDevice* pDevice, LPWSTR pszName);

HRESULT DELTACALL device_info_get_count(DWORD dwType, UINT* pdwCount) {
    return device_info_get_devices(dwType, pdwCount, NULL);
}

HRESULT DELTACALL device_info_get_device(DWORD dwType, LPCGUID pcGuidDevice, device_info* pDevice) {
    if (dwType != DEVICETYPE_RENDER
        && dwType != DEVICETYPE_CAPTURE && dwType != DEVICETYPE_ALL) {
        return E_INVALIDARG;
    }

    if (pcGuidDevice == NULL || pDevice == NULL) {
        return E_INVALIDARG;
    }

    get_device_context ctx;

    ctx.ID = pcGuidDevice;
    ctx.Type = dwType;
    ctx.Device = pDevice;

    return device_info_thread_wait(CreateThread(NULL, 0,
        device_info_get_device_thread, &ctx, 0, NULL));
}

HRESULT DELTACALL device_info_get_devices(DWORD dwType, UINT* pdwCount, device_info* pDevices) {
    if (dwType != DEVICETYPE_RENDER
        && dwType != DEVICETYPE_CAPTURE && dwType != DEVICETYPE_ALL) {
        return E_INVALIDARG;
    }

    if (pdwCount == NULL) {
        return E_INVALIDARG;
    }

    *pdwCount = 0;

    get_devices_context ctx;

    ctx.Type = dwType;
    ctx.Count = pdwCount;
    ctx.Devices = pDevices;

    return device_info_thread_wait(CreateThread(NULL, 0,
        (LPTHREAD_START_ROUTINE)device_info_get_devices_thread, &ctx, 0, NULL));
}

HRESULT DELTACALL device_info_get_default_device(DWORD dwType, DWORD dwKind, device_info* pDevice) {
    if (dwType != DEVICETYPE_RENDER && dwType != DEVICETYPE_CAPTURE) {
        return E_INVALIDARG;
    }

    if (dwKind != DEVICEKIND_AUDIO && dwKind != DEVICEKIND_MULTIMEDIA
        && dwKind != DEVICEKIND_COMMUNICATION) {
        return E_INVALIDARG;
    }

    if (pDevice == NULL) {
        return E_INVALIDARG;
    }

    get_default_device_context ctx;

    ctx.Type = dwType;
    ctx.Kind = dwKind;
    ctx.Device = pDevice;

    return device_info_thread_wait(CreateThread(NULL, 0,
        device_info_get_default_device_thread, &ctx, 0, NULL));
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL device_info_thread_wait(HANDLE thread) {
    if (thread == NULL) {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    // Wait for the thread to exit...
    if (WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0) {
        DWORD code = EXIT_SUCCESS;

        if (!GetExitCodeThread(thread, &code) || code != EXIT_SUCCESS) {
            hr = DSERR_NODRIVER;
        }
    }

    CloseHandle(thread);
    return hr;
}

HRESULT DELTACALL device_info_get_id(IMMDevice* pDevice, LPGUID pID) {
    if (pDevice == NULL) {
        return E_POINTER;
    }

    if (pID == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    IPropertyStore* props = NULL;

    if (SUCCEEDED(hr = IMMDevice_OpenPropertyStore(pDevice, STGM_READ, &props))) {

        PROPVARIANT id;
        PropVariantInit(&id);

        if (SUCCEEDED(hr = IPropertyStore_GetValue(props, &PKEY_AudioEndpoint_GUID, &id))) {
            if (id.vt != VT_EMPTY) {
                hr = CLSIDFromString(id.pwszVal, pID);
            }
        }

        PropVariantClear(&id);

        RELEASE(props);
    }

    return hr;
}

HRESULT DELTACALL device_info_get_module(IMMDevice* pDevice, LPWSTR pszId) {
    if (pDevice == NULL) {
        return E_POINTER;
    }

    if (pszId == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    LPWSTR instance = NULL;

    if (SUCCEEDED(hr = IMMDevice_GetId(pDevice, &instance))) {
        wcscpy_s(pszId, MAX_DEVICE_NAME_LENGTH - 1, instance);
        CoTaskMemFree(instance);
    }

    return hr;
}

HRESULT DELTACALL device_info_get_name(IMMDevice* pDevice, LPWSTR pszName) {
    if (pDevice == NULL) {
        return E_POINTER;
    }
    
    if (pszName == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    IPropertyStore* props = NULL;

    if (SUCCEEDED(hr = IMMDevice_OpenPropertyStore(pDevice, STGM_READ, &props))) {

        PROPVARIANT name;
        PropVariantInit(&name);

        if (SUCCEEDED(hr = IPropertyStore_GetValue(props, &PKEY_Device_FriendlyName, &name))) {
            if (name.vt != VT_EMPTY) {
                wcscpy_s(pszName, MAX_DEVICE_NAME_LENGTH - 1, name.pwszVal);
            }
        }

        PropVariantClear(&name);

        RELEASE(props);
    }

    return hr;
}

DWORD WINAPI device_info_get_device_thread(get_device_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    BOOL found = FALSE;
    IMMDeviceEnumerator* enumerator = NULL;
    IMMDeviceCollection* collection = NULL;

    if (FAILED(hr = CoCreateInstance(&CLSID_IMMDeviceEnumerator,
        NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator))) {
        goto exit;
    }

    if (FAILED(hr = IMMDeviceEnumerator_EnumAudioEndpoints(enumerator,
        (EDataFlow)ctx->Type, DEVICE_STATE_ACTIVE, &collection))) {
        goto exit;
    }

    UINT count = 0;
    if (FAILED(hr = IMMDeviceCollection_GetCount(collection, &count))) {
        goto exit;
    }

    for (UINT i = 0; i < count; i++) {
        IMMDevice* device = NULL;

        if (SUCCEEDED(hr = IMMDeviceCollection_Item(collection, i, &device))) {
            if (SUCCEEDED(hr = device_info_get_id(device, &ctx->Device->ID))) {
                if (IsEqualGUID(ctx->ID, &ctx->Device->ID)) {
                    if (SUCCEEDED(hr = device_info_get_module(device, ctx->Device->Module))) {
                        if (SUCCEEDED(hr = device_info_get_name(device, ctx->Device->Name))) {
                            ctx->Type = ctx->Type;
                            found = TRUE;
                        }
                    }
                }
            }

            RELEASE(device);

            if (found) { break; }
        }
    }

    if (!found) {
        ZeroMemory(ctx->Device, sizeof(device_info));
    }

exit:

    RELEASE(collection);
    RELEASE(enumerator);

    CoUninitialize();

    return found ? EXIT_SUCCESS : EXIT_FAILURE;
}

DWORD WINAPI device_info_get_devices_thread(get_devices_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    IMMDeviceEnumerator* enumerator = NULL;
    IMMDeviceCollection* collection = NULL;

    if (FAILED(hr = CoCreateInstance(&CLSID_IMMDeviceEnumerator,
        NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator))) {
        goto exit;
    }

    if (FAILED(hr = IMMDeviceEnumerator_EnumAudioEndpoints(enumerator,
        (EDataFlow)ctx->Type, DEVICE_STATE_ACTIVE, &collection))) {
        goto exit;
    }

    UINT written = 0, read = 0;
    if (FAILED(hr = IMMDeviceCollection_GetCount(collection, &read))) {
        goto exit;
    }

    if (ctx->Devices == NULL) {
        *ctx->Count = read;
        goto exit;
    }

    ZeroMemory(ctx->Devices, read * sizeof(device_info));

    for (UINT i = 0; i < read; i++) {
        IMMDevice* device = NULL;

        if (SUCCEEDED(hr = IMMDeviceCollection_Item(collection, i, &device))) {
            if (SUCCEEDED(hr = device_info_get_id(device, &ctx->Devices[written].ID))) {
                if (SUCCEEDED(hr = device_info_get_module(device, ctx->Devices[written].Module))) {
                    if (SUCCEEDED(hr = device_info_get_name(device, ctx->Devices[written].Name))) {
                        ctx->Devices[written].Type = ctx->Type;
                        written++;
                    }
                }
            }

            RELEASE(device);
        }
    }

    *ctx->Count = written;

exit:

    RELEASE(collection);
    RELEASE(enumerator);

    CoUninitialize();

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}

DWORD WINAPI device_info_get_default_device_thread(get_default_device_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    IMMDevice* device = NULL;
    IMMDeviceEnumerator* enumerator = NULL;

    if (FAILED(hr = CoCreateInstance(&CLSID_IMMDeviceEnumerator,
        NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator))) {
        goto exit;
    }

    if (FAILED(hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator,
        (EDataFlow)ctx->Type, (ERole)ctx->Kind, &device))) {
        goto exit;
    }

    ZeroMemory(ctx->Device, sizeof(device_info));

    if (SUCCEEDED(hr = device_info_get_id(device, &ctx->Device->ID))) {
        if (SUCCEEDED(hr = device_info_get_module(device, ctx->Device->Module))) {
            if (SUCCEEDED(hr = device_info_get_name(device, ctx->Device->Name))) {
                ctx->Device->Type = ctx->Type;
            }
        }
    }

    if (FAILED(hr)) {
        ZeroMemory(ctx->Device, sizeof(device_info));
    }

exit:

    RELEASE(device);
    RELEASE(enumerator);

    CoUninitialize();

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
