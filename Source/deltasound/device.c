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

#include "device.h"
#include "uuid.h"

#define REFTIMES_PER_SEC    10000000

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }
#define RELEASEHANDLE(X) if((X)) { CloseHandle((X)); (X) = NULL; }

typedef struct device_thread_context {
    device* Device;
    HANDLE  Init;
} device_thread_context;

DWORD WINAPI device_thread(device_thread_context* ctx);

HRESULT DELTACALL device_allocate(allocator* pAlloc, device** ppOut);
HRESULT DELTACALL device_initialize(device* pDev);
HRESULT DELTACALL device_get_period(device* pDev, LPREFERENCE_TIME pDefaultPeriod, LPREFERENCE_TIME pMinPeriod);
HRESULT DELTACALL device_get_mix_format(device* pDev, LPWAVEFORMATEX* ppWaveFormat);

HRESULT DELTACALL device_create(
    allocator* pAlloc, DWORD dwType, device_info* pInfo, device** ppOut) {
    if (pAlloc == NULL) {
        return E_INVALIDARG;
    }

    if (dwType != DEVICETYPE_AUDIO && dwType != DEVICETYPE_RECORD) {
        return E_INVALIDARG;
    }

    if (pInfo == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    device* instance = NULL;

    if (SUCCEEDED(hr = device_allocate(pAlloc, &instance))) {
        instance->RefCount = 1;
        CopyMemory(&instance->Info, pInfo, sizeof(device_info));

        device_thread_context* ctx;
        if (FAILED(hr = allocator_allocate(pAlloc, sizeof(device_thread_context), &ctx))) {
            device_release(instance);
            return hr;
        }

        ctx->Init = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (ctx->Init == NULL) {
            device_release(instance);
            return E_FAIL;
        }

        instance->AudioEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (instance->AudioEvent == NULL) {
            device_release(instance);
            return E_FAIL;
        }

        ctx->Device = instance;

        instance->Thread = CreateThread(NULL, 0,
            (LPTHREAD_START_ROUTINE)device_thread, ctx, 0, NULL);

        if (instance->Thread == NULL) {
            device_release(instance);
            return E_FAIL;
        }

        SetThreadPriority(instance->Thread, THREAD_PRIORITY_TIME_CRITICAL);

        WaitForSingleObject(ctx->Init, INFINITE);

        // TODO check if thread exited prematurely...

        *ppOut = instance;
    }

    return hr;
}

ULONG DELTACALL device_add_ref(device* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL device_remove_ref(device* self) {
    if (self == NULL) {
        return 0;
    }

    if (InterlockedDecrement(&self->RefCount) <= 0) {
        self->RefCount = 0;

        device_release(self);
    }

    return self->RefCount;
}

VOID DELTACALL device_release(device* self) {
    if (self == NULL) {
        return;
    }

    if (self->Thread != NULL) {
        self->Close = TRUE;

        WaitForSingleObject(self->Thread, INFINITE);
        CloseHandle(self->Thread);
    }

    // TODO clean wave format

    allocator_free(self->Allocator, self);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL device_allocate(allocator* pAlloc, device** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    device* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(device), &instance))) {
        ZeroMemory(instance, sizeof(device));

        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL device_initialize(device* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    IMMDeviceEnumerator* enumerator = NULL;
    REFERENCE_TIME period = 0;
    LPWAVEFORMATEX wfx = NULL;

    if (FAILED(hr = CoCreateInstance(&CLSID_IMMDeviceEnumerator,
        NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator))) {
        goto exit;
    }

    if (FAILED(hr = IMMDeviceEnumerator_GetDevice(enumerator,
        self->Info.Module, &self->Device))) {
        goto exit;
    }

    if (FAILED(hr = IMMDevice_Activate(self->Device,
        &IID_IAudioClient, CLSCTX_INPROC_SERVER, NULL, &self->AudioClient))) {
        goto exit;
    }

    if (FAILED(hr = device_get_mix_format(self, &wfx))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_Initialize(self->AudioClient,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_NOPERSIST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        REFTIMES_PER_SEC, 0, wfx, NULL))) {
        goto exit;
    }

    {
        const size_t size = sizeof(WAVEFORMATEX) + wfx->cbSize;

        if (FAILED(hr = allocator_allocate(self->Allocator, size, &self->WaveFormat))) {
            goto exit;
        }

        CopyMemory(self->WaveFormat, wfx, size);
    }

    CoTaskMemFree(wfx);

    if (FAILED(hr = IAudioClient_SetEventHandle(self->AudioClient, self->AudioEvent))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_GetService(self->AudioClient,
        &IID_IAudioRenderClient, &self->AudioRenderer))) {
        goto exit;
    }

    // TODO
    //if (FAILED(hr = IAudioClient_GetService(self->AudioClient,
    //    &IID_IAudioStreamVolume, &self->AudioVolume))) {
    //    goto exit;
    //}

    if (FAILED(hr = IAudioClient_Start(self->AudioClient))) {
        goto exit;
    }

    RELEASE(enumerator);

    return S_OK;

exit:

    if (self->WaveFormat != NULL) {
        allocator_free(self->Allocator, self->WaveFormat);
    }

    // RELEASE(self->AudioVolume); // TODO
    RELEASE(self->AudioRenderer);
    RELEASE(self->AudioClient);
    RELEASE(self->Device);
    RELEASE(enumerator);

    return hr;
}

HRESULT DELTACALL device_get_period(device* self, LPREFERENCE_TIME pDefaultPeriod, LPREFERENCE_TIME pMinPeriod) {
    if (self == NULL) {
        return E_POINTER;
    }

    return IAudioClient_GetDevicePeriod(self->AudioClient, pDefaultPeriod, pMinPeriod);
}

HRESULT DELTACALL device_get_mix_format(device* self, LPWAVEFORMATEX* ppWaveFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppWaveFormat == NULL) {
        return E_INVALIDARG;
    }

    return IAudioClient_GetMixFormat(self->AudioClient, ppWaveFormat);
}

DWORD WINAPI device_thread(device_thread_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    device* dev = ctx->Device;

    if (FAILED(hr = device_initialize(dev))) {
        goto exit;
    }

    SetEvent(ctx->Init);

    while (!dev->Close) {
        
        //todo
        Sleep(1);
    }

    if (dev->WaveFormat != NULL) {
        allocator_free(dev->Allocator, dev->WaveFormat);
    }

    // RELEASE(dev->AudioVolume); // TODO
    RELEASE(dev->AudioRenderer);
    RELEASE(dev->AudioClient);
    RELEASE(dev->Device);

    allocator_free(dev->Allocator, ctx);

exit:

    CoUninitialize();

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
