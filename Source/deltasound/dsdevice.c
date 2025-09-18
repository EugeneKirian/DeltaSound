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

#include "ds.h"
#include "dsb.h"
#include "dsdevice.h"
#include "uuid.h"
#include "wave_format.h"

#define REFTIMES_PER_SEC                    10000000
#define TARGET_BUFFER_PADDING_IN_SECONDS    (1.0f / 60.0f)

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }
#define RELEASEHANDLE(X) if((X)) { CloseHandle((X)); (X) = NULL; }

typedef struct dsdevice_thread_context {
    dsdevice* Device;
    HANDLE  Init;
} dsdevice_thread_context;

DWORD WINAPI dsdevice_thread(dsdevice_thread_context* ctx);

HRESULT DELTACALL dsdevice_allocate(allocator* pAlloc, dsdevice** ppOut);
HRESULT DELTACALL dsdevice_initialize(dsdevice* pDev);
HRESULT DELTACALL dsdevice_get_mix_format(dsdevice* pDev, LPWAVEFORMATEX* ppFormat);

HRESULT DELTACALL dsdevice_play(dsdevice* pDev); // TODO

HRESULT DELTACALL dsdevice_create(
    allocator* pAlloc, ds* pDS, DWORD dwType, device_info* pInfo, dsdevice** ppOut) {
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
    dsdevice* instance = NULL;

    if (SUCCEEDED(hr = dsdevice_allocate(pAlloc, &instance))) {
        instance->Instance = pDS;

        CopyMemory(&instance->Info, pInfo, sizeof(device_info));

        if (SUCCEEDED(hr = mixer_create(pAlloc, &instance->Mixer))) {
            dsdevice_thread_context* ctx;

            if (FAILED(hr = allocator_allocate(pAlloc, sizeof(dsdevice_thread_context), &ctx))) {
                dsdevice_release(instance);
                return hr;
            }

            ctx->Init = CreateEventA(NULL, FALSE, FALSE, NULL);
            if (ctx->Init == NULL) {
                dsdevice_release(instance);
                return E_FAIL;
            }

            for (DWORD i = 0; i < DSDEVICE_MAX_EVENT_COUNT; i++) {
                instance->Events[i] = CreateEventA(NULL, FALSE, FALSE, NULL);

                if (instance->Events[i] == NULL) {
                    dsdevice_release(instance);
                    return E_FAIL;
                }
            }

            instance->ThreadEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
            if (instance->ThreadEvent == NULL) {
                dsdevice_release(instance);
                return E_FAIL;
            }

            ctx->Device = instance;

            instance->Thread = CreateThread(NULL, 0,
                (LPTHREAD_START_ROUTINE)dsdevice_thread, ctx, 0, NULL);

            if (instance->Thread == NULL) {
                dsdevice_release(instance);
                return E_FAIL;
            }

            SetThreadPriority(instance->Thread, THREAD_PRIORITY_TIME_CRITICAL);

            WaitForSingleObject(ctx->Init, INFINITE);
            CloseHandle(ctx->Init);

            // TODO check if thread exited prematurely...

            *ppOut = instance;

            return S_OK;
        }

        mixer_release(instance->Mixer);
    }

    return hr;
}

VOID DELTACALL dsdevice_release(dsdevice* self) {
    if (self == NULL) { return; }

    if (self->Thread != NULL) {
        SetEvent(self->Events[DSDEVICE_CLOSE_EVENT_INDEX]);

        // NOTE. Cannot wait for thread handle,
        // because it does not fire when thread is being
        // terminated through the FreeLibrary function call.

        WaitForSingleObject(self->ThreadEvent, INFINITE);
        CloseHandle(self->ThreadEvent);
        CloseHandle(self->Thread);
    }

    mixer_release(self->Mixer);

    allocator_free(self->Allocator, self);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsdevice_allocate(allocator* pAlloc, dsdevice** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsdevice* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsdevice), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL dsdevice_initialize(dsdevice* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    IMMDeviceEnumerator* enumerator = NULL;
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

    if (FAILED(hr = dsdevice_get_mix_format(self, &wfx))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_Initialize(self->AudioClient,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_NOPERSIST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        REFTIMES_PER_SEC, 0, wfx, NULL))) {
        goto exit;
    }

    {
        const DWORD size = sizeof(WAVEFORMATEX) + wfx->cbSize;

        if (FAILED(hr = allocator_allocate(self->Allocator, size, &self->Format))) {
            goto exit;
        }

        CopyMemory(self->Format, wfx, size);
    }

    CoTaskMemFree(wfx);

    if (FAILED(hr = IAudioClient_SetEventHandle(self->AudioClient,
        self->Events[DSDEVICE_AUDIO_EVENT_INDEX]))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_GetService(self->AudioClient,
        &IID_IAudioRenderClient, &self->AudioRenderer))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_GetBufferSize(self->AudioClient, &self->AudioClientBufferSize))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_Start(self->AudioClient))) {
        goto exit;
    }

    RELEASE(enumerator);

    return S_OK;

exit:

    if (self->Format != NULL) {
        allocator_free(self->Allocator, self->Format);
    }

    RELEASE(self->AudioRenderer);
    RELEASE(self->AudioClient);
    RELEASE(self->Device);
    RELEASE(enumerator);

    return hr;
}

HRESULT DELTACALL dsdevice_get_mix_format(dsdevice* self, LPWAVEFORMATEX* ppFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppFormat == NULL) {
        return E_INVALIDARG;
    }

    return IAudioClient_GetMixFormat(self->AudioClient, ppFormat);
}

HRESULT DELTACALL dsdevice_play(dsdevice* self) { // TODO name, etc...
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Instance == NULL) {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    DWORD status = DS_STATUS_NONE;

    if (SUCCEEDED(hr = ds_get_status(self->Instance, &status))) {
        if (status & DS_STATUS_PLAYING) {
            UINT32 padding = 0;

            if (SUCCEEDED(hr = IAudioClient_GetCurrentPadding(self->AudioClient, &padding))) {
                const UINT32 frames =
                    (UINT32)(self->AudioClientBufferSize * TARGET_BUFFER_PADDING_IN_SECONDS) - padding;

                if (frames != 0) {
                    BYTE* lock = NULL;

                    if (SUCCEEDED(hr = IAudioRenderClient_GetBuffer(self->AudioRenderer, frames, &lock))) {

                        LPVOID buffer = NULL;
                        DWORD buffer_size = 0;

                        if (SUCCEEDED(mixer_mix(self->Mixer,
                            self->Format, frames, self->Instance->Main,
                            self->Instance->Buffers, &buffer, &buffer_size))) {
                            CopyMemory(lock, buffer, buffer_size);

                            IAudioRenderClient_ReleaseBuffer(self->AudioRenderer,
                                buffer_size / self->Format->Format.nBlockAlign, 0);
                        }
                        else {
                            IAudioRenderClient_ReleaseBuffer(self->AudioRenderer,
                                0, AUDCLNT_BUFFERFLAGS_SILENT);
                        }
                    }
                }
            }
        }
    }

    return hr;
}

DWORD WINAPI dsdevice_thread(dsdevice_thread_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    dsdevice* dev = ctx->Device;

    if (FAILED(hr = dsdevice_initialize(dev))) {
        goto exit;
    }

    SetEvent(ctx->Init);

    while (TRUE) {
        const DWORD result =
            WaitForMultipleObjects(DSDEVICE_MAX_EVENT_COUNT, dev->Events, FALSE, INFINITE);

        if (result == DSDEVICE_CLOSE_EVENT_INDEX
            || result == DSDEVICE_MAX_EVENT_COUNT) {
            break;
        }
        else if (result == DSDEVICE_AUDIO_EVENT_INDEX) {
            // TODO pefrormance: play only when there are buffers that are playing
            hr = dsdevice_play(dev);
        }
    }

    if (dev->Format != NULL) {
        allocator_free(dev->Allocator, dev->Format);
    }

    RELEASE(dev->AudioRenderer);
    RELEASE(dev->AudioClient);
    RELEASE(dev->Device);

    allocator_free(dev->Allocator, ctx);

exit:

    CoUninitialize();

    SetEvent(dev->ThreadEvent);

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
