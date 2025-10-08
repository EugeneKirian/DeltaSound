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
#include "render.h"
#include "uuid.h"
#include "wave.h"

#define REFTIMES_PER_SEC                    10000000
#define TARGET_BUFFER_PADDING_IN_SECONDS    (1.0f / 60.0f)

#define AUDCLNT_BUFFERFLAGS_NONE            0

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }
#define RELEASEHANDLE(X) if((X)) { CloseHandle((X)); (X) = NULL; }

typedef struct render_thread_context {
    render*     Device;
    HANDLE      Init;
} render_thread_context;

DWORD WINAPI render_thread(render_thread_context* ctx);

HRESULT DELTACALL render_initialize(render* pRender);
HRESULT DELTACALL render_get_format(render* pRender, LPWAVEFORMATEX* ppwfxFormat);

HRESULT DELTACALL render_render(render* pRender, DWORD dwBuffers, dsb** ppBuffers);
HRESULT DELTACALL render_get_active_buffers(render* pRender, LPDWORD pdwCount, dsb*** ppBuffers);

HRESULT DELTACALL render_create(allocator* pAlloc, ds* pDS, device_info* pInfo, render** ppOut) {
    if (pAlloc == NULL) {
        return E_INVALIDARG;
    }

    if (pInfo == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    render* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(render), &instance))) {
        instance->Allocator = pAlloc;
        instance->Instance = pDS;

        CopyMemory(&instance->Info, pInfo, sizeof(device_info));

        if (SUCCEEDED(hr = arena_create(pAlloc, &instance->Arena))) {
            if (SUCCEEDED(hr = mixer_create(pAlloc, &instance->Mixer))) {
                render_thread_context* ctx;

                if (FAILED(hr = allocator_allocate(pAlloc, sizeof(render_thread_context), &ctx))) {
                    render_release(instance);
                    return hr;
                }

                ctx->Init = CreateEventA(NULL, FALSE, FALSE, NULL);
                if (ctx->Init == NULL) {
                    render_release(instance);
                    return E_FAIL;
                }

                for (DWORD i = 0; i < RENDER_MAX_EVENT_COUNT; i++) {
                    instance->Events[i] = CreateEventA(NULL, FALSE, FALSE, NULL);

                    if (instance->Events[i] == NULL) {
                        render_release(instance);
                        return E_FAIL;
                    }
                }

                instance->ThreadEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
                if (instance->ThreadEvent == NULL) {
                    render_release(instance);
                    return E_FAIL;
                }

                ctx->Device = instance;

                instance->Thread = CreateThread(NULL, 0, render_thread, ctx, 0, NULL);

                if (instance->Thread == NULL) {
                    render_release(instance);
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

        arena_release(instance->Arena);
    }

    return hr;
}

VOID DELTACALL render_release(render* self) {
    if (self == NULL) { return; }

    if (self->Thread != NULL) {
        SetEvent(self->Events[RENDER_CLOSE_EVENT_INDEX]);

        // NOTE. Cannot wait for thread handle,
        // because it does not fire when thread is being
        // terminated through the FreeLibrary function call.

        WaitForSingleObject(self->ThreadEvent, INFINITE);
        CloseHandle(self->ThreadEvent);
        CloseHandle(self->Thread);
    }

    arena_release(self->Arena);

    mixer_release(self->Mixer);

    allocator_free(self->Allocator, self);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL render_initialize(render* self) {
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

    if (FAILED(hr = render_get_format(self, &wfx))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_Initialize(self->AudioClient,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_NOPERSIST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        REFTIMES_PER_SEC, 0, wfx, NULL))) {
        goto exit;
    }

    if (FAILED(hr = allocator_allocate(self->Allocator, SIZEOFFORMATEX(wfx), &self->Format))) {
        goto exit;
    }

    CopyMemory(self->Format, wfx, SIZEOFFORMATEX(wfx));

    CoTaskMemFree(wfx);

    if (FAILED(hr = IAudioClient_SetEventHandle(self->AudioClient,
        self->Events[RENDER_AUDIO_EVENT_INDEX]))) {
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

HRESULT DELTACALL render_get_format(render* self, LPWAVEFORMATEX* ppwfxFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppwfxFormat == NULL) {
        return E_INVALIDARG;
    }

    return IAudioClient_GetMixFormat(self->AudioClient, ppwfxFormat);
}

HRESULT DELTACALL render_render(render* self, DWORD dwBuffers, dsb** ppBuffers) {
    if (self->Instance == NULL) {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    UINT32 padding = 0;

    if (SUCCEEDED(hr = IAudioClient_GetCurrentPadding(self->AudioClient, &padding))) {
        const UINT32 frames =
            (UINT32)(self->AudioClientBufferSize * TARGET_BUFFER_PADDING_IN_SECONDS) - padding;

        if (frames != 0) {
            BYTE* lock = NULL;

            if (SUCCEEDED(hr = IAudioRenderClient_GetBuffer(self->AudioRenderer, frames, &lock))) {
                LPVOID buffer = NULL;
                DWORD available = 0;

                if (SUCCEEDED(hr = mixer_mix(self->Mixer, dwBuffers, ppBuffers,
                    self->Format, frames, &buffer, &available))) {
                    CopyMemory(lock, buffer, available * self->Format->Format.nBlockAlign);

                    hr = IAudioRenderClient_ReleaseBuffer(self->AudioRenderer, available, AUDCLNT_BUFFERFLAGS_NONE);
                }
                else {
                    hr = IAudioRenderClient_ReleaseBuffer(self->AudioRenderer, 0, AUDCLNT_BUFFERFLAGS_SILENT);
                }
            }
        }
    }

    return hr;
}

HRESULT DELTACALL render_get_active_buffers(render* self, LPDWORD pdwCount, dsb*** ppBuffers) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwCount == NULL || ppBuffers == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ds* instance = self->Instance;

    if (FAILED(hr = arena_clear(self->Arena))) {
        return hr;
    }

    if (instance->Level == DSSCL_WRITEPRIMARY
        && (instance->Main->Status & DSBSTATUS_PLAYING)) {
        dsb** buffers = NULL;

        if (FAILED(hr = arena_allocate(self->Arena, sizeof(dsb**), (LPVOID*)&buffers))) {
            return hr;
        }

        buffers[0] = instance->Main;

        *pdwCount = 1;
        *ppBuffers = buffers;
    }
    else {
        dsb** buffers = NULL;
        DWORD count = 0;

        const DWORD available = arr_get_count(instance->Buffers);

        if (FAILED(hr = arena_allocate(self->Arena, available * sizeof(dsb**), (LPVOID*)&buffers))) {
            return hr;
        }

        for (DWORD i = 0; i < available; i++) {
            dsb* buffer = NULL;

            if (SUCCEEDED(arr_get_item(instance->Buffers, i, &buffer))) {
                DWORD status = DSBSTATUS_NONE;

                if (SUCCEEDED(dsb_get_status(buffer, &status))) {
                    if (status & DSBSTATUS_PLAYING) {
                        buffers[count++] = buffer;
                    }
                }
            }
        }

        *pdwCount = count;
        *ppBuffers = buffers;
    }

    return hr;
}

DWORD WINAPI render_thread(render_thread_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    render* device = ctx->Device;

    if (FAILED(hr = render_initialize(device))) {
        goto exit;
    }

    SetEvent(ctx->Init);

    while (TRUE) {
        const DWORD result =
            WaitForMultipleObjects(RENDER_MAX_EVENT_COUNT, device->Events, FALSE, INFINITE);

        if (result == RENDER_CLOSE_EVENT_INDEX
            || result == RENDER_MAX_EVENT_COUNT) {
            break;
        }
        else if (result == RENDER_AUDIO_EVENT_INDEX) {
            dsb** buffers = NULL;
            DWORD count = 0;

            if (SUCCEEDED(hr = render_get_active_buffers(device, &count, &buffers))) {
                if (count != 0) {
                    hr = render_render(device, count, buffers);
                }
            }
        }
    }

    IAudioClient_Stop(device->AudioClient);

    if (device->Format != NULL) {
        allocator_free(device->Allocator, device->Format);
    }

    RELEASE(device->AudioRenderer);
    RELEASE(device->AudioClient);
    RELEASE(device->Device);

    allocator_free(device->Allocator, ctx);

exit:

    CoUninitialize();

    SetEvent(device->ThreadEvent);

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
