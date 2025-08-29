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
#include "wasapi_device.h"
#include "uuid.h"

#include <stdio.h>

#define CINTERFACE
#define COBJMACROS
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }

typedef struct wasapi_device_context {
    DWORD           Type;
    UINT*           Count;
    wasapi_device*  Devices;
} wasapi_device_context;

DWORD WINAPI wasapi_device_thread(wasapi_device_context* ctx);

HRESULT DELTACALL wasapi_device_get_count(DWORD dwType, UINT* pdwCount) {
    return wasapi_device_get_devices(dwType, pdwCount, NULL);
}

HRESULT DELTACALL wasapi_device_get_devices(DWORD dwType, UINT* pdwCount, wasapi_device* pDevices) {
    if (dwType != WASAPIDEVICETYPE_AUDIO
        && dwType != WASAPIDEVICETYPE_RECORD) {
        return E_INVALIDARG;
    }

    if (pdwCount == NULL) {
        return E_INVALIDARG;
    }

    *pdwCount = 0;

    wasapi_device_context ctx;

    ctx.Type = dwType;
    ctx.Count = pdwCount;
    ctx.Devices = pDevices;

    // Create new thread so that COM can be properly initialized
    // and uninitialized without affecting calling thread
    // that might be still needing COM objects...

    HANDLE thread = CreateThread(NULL, 0,
        (LPTHREAD_START_ROUTINE)wasapi_device_thread, &ctx, 0, NULL);

    if (thread == NULL) {
        return E_FAIL;
    }

    // Wait for the thread to exit...
    if (WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0) {
        DWORD code = EXIT_SUCCESS;
        if (GetExitCodeThread(thread, &code) && code == EXIT_SUCCESS) {
            CloseHandle(thread);
            return S_OK;
        }
    }

    CloseHandle(thread);
    return E_FAIL;
}

/* ---------------------------------------------------------------------- */

DWORD WINAPI wasapi_device_thread(wasapi_device_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    IMMDeviceEnumerator* enumerator = NULL;
    IMMDeviceCollection* collection = NULL;
    UINT written = 0, read = 0;

    if (FAILED(hr = CoCreateInstance(&CLSID_IMMDeviceEnumerator,
        NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator))) {
        goto exit;
    }

    if (FAILED(hr = IMMDeviceEnumerator_EnumAudioEndpoints(enumerator,
        (EDataFlow)ctx->Type, DEVICE_STATE_ACTIVE, &collection))) {
        goto exit;
    }

    if (FAILED(hr = IMMDeviceCollection_GetCount(collection, &read))) {
        goto exit;
    }

    if (ctx->Devices == NULL) {
        *ctx->Count = read;
        goto exit;
    }

    ZeroMemory(ctx->Devices, read * sizeof(wasapi_device));

    for (UINT i = 0; i < read; i++) {
        IMMDevice* dev = NULL;
        IPropertyStore* props = NULL;

        if (SUCCEEDED(hr = IMMDeviceCollection_Item(collection, i, &dev))) {
            if (SUCCEEDED(hr = IMMDevice_OpenPropertyStore(dev, STGM_READ, &props))) {

                LPWSTR instance = NULL;
                if (SUCCEEDED(hr = IMMDevice_GetId(dev, &instance))) {

                    // ID
                    {
                        GUID uuid;
                        ZeroMemory(&uuid, sizeof(GUID));

                        // ID has values like:
                        // {0.0.0.00000000}.{024e7d15-2f2f-4f02-8e3a-30e4d164e235}

                        LPWSTR start = wcsrchr(instance, L'{');
                        if (start != NULL) {
                            if (SUCCEEDED(hr = IIDFromString(start, &uuid))) {
                                ctx->Devices[written].ID = uuid;
                            }
                        }
                    }

                    // Type
                    ctx->Devices[written].Type = ctx->Type;

                    // Name
                    {
                        PROPVARIANT name;
                        PropVariantInit(&name);

                        if (SUCCEEDED(hr = IPropertyStore_GetValue(props, &PKEY_Device_FriendlyName, &name))) {
                            if (name.vt != VT_EMPTY) {
                                wcscpy_s(ctx->Devices[written].Name,
                                    MAX_WASAPI_DEVICE_IDENTITY_LENGTH - 1, name.pwszVal);
                            }
                        }

                        PropVariantClear(&name);
                    }

                    // Module
                    {
                        wcscpy_s(ctx->Devices[written].Module,
                            MAX_WASAPI_DEVICE_IDENTITY_LENGTH - 1, instance);
                    }

                    written++;
                }

                CoTaskMemFree(instance);
            }

            RELEASE(dev);
        }
    }

    *ctx->Count = written;

exit:

    RELEASE(collection);
    RELEASE(enumerator);

    CoUninitialize();

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
