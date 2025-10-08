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

#include "capture.h"
#include "convertor.h"
#include "dsc.h"
#include "dscb.h"
#include "uuid.h"
#include "wave.h"

#define REFTIMES_PER_SEC                    10000000
#define TARGET_BUFFER_PADDING_IN_SECONDS    (1.0f / 60.0f)

#define AUDCLNT_BUFFERFLAGS_NONE            0

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }
#define RELEASEHANDLE(X) if((X)) { CloseHandle((X)); (X) = NULL; }

typedef struct capture_thread_context {
    capture*    Device;
    HANDLE      Init;
} capture_thread_context;

DWORD WINAPI capture_thread(capture_thread_context* ctx);

HRESULT DELTACALL capture_initialize(capture* pCapture);
HRESULT DELTACALL capture_get_format(capture* pCapture, LPWAVEFORMATEX* ppwfxFormat);
HRESULT DELTACALL capture_capture(capture* pCapture);

HRESULT DELTACALL capture_create(allocator* pAlloc, dsc* pDSC, device_info* pInfo, capture** ppOut) {
    if (pAlloc == NULL) {
        return E_INVALIDARG;
    }

    if (pInfo == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    capture* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(capture), &instance))) {
        instance->Allocator = pAlloc;
        instance->Instance = pDSC;

        CopyMemory(&instance->Info, pInfo, sizeof(device_info));

        if (SUCCEEDED(hr = convertor_create(pAlloc, &instance->Convertor))) {
            capture_thread_context* ctx;

            if (FAILED(hr = allocator_allocate(pAlloc, sizeof(capture_thread_context), &ctx))) {
                capture_release(instance);
                return hr;
            }

            ctx->Init = CreateEventA(NULL, FALSE, FALSE, NULL);
            if (ctx->Init == NULL) {
                capture_release(instance);
                return E_FAIL;
            }

            for (DWORD i = 0; i < CAPTURE_MAX_EVENT_COUNT; i++) {
                instance->Events[i] = CreateEventA(NULL, FALSE, FALSE, NULL);

                if (instance->Events[i] == NULL) {
                    capture_release(instance);
                    return E_FAIL;
                }
            }

            instance->ThreadEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
            if (instance->ThreadEvent == NULL) {
                capture_release(instance);
                return E_FAIL;
            }

            ctx->Device = instance;

            instance->Thread = CreateThread(NULL, 0, capture_thread, ctx, 0, NULL);

            if (instance->Thread == NULL) {
                capture_release(instance);
                return E_FAIL;
            }

            SetThreadPriority(instance->Thread, THREAD_PRIORITY_TIME_CRITICAL);

            WaitForSingleObject(ctx->Init, INFINITE);
            CloseHandle(ctx->Init);

            // TODO check if thread exited prematurely...

            *ppOut = instance;

            return S_OK;
        }

        convertor_release(instance->Convertor);
    }

    return hr;
}

VOID DELTACALL capture_release(capture* self) {
    if (self == NULL) { return; }

    if (self->Thread != NULL) {
        SetEvent(self->Events[CAPTURE_CLOSE_EVENT_INDEX]);

        // NOTE. Cannot wait for thread handle,
        // because it does not fire when thread is being
        // terminated through the FreeLibrary function call.

        WaitForSingleObject(self->ThreadEvent, INFINITE);
        CloseHandle(self->ThreadEvent);
        CloseHandle(self->Thread);
    }

    convertor_release(self->Convertor);

    allocator_free(self->Allocator, self);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL capture_initialize(capture* self) {
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

    if (FAILED(hr = capture_get_format(self, &wfx))) {
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
        self->Events[CAPTURE_AUDIO_EVENT_INDEX]))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_GetService(self->AudioClient,
        &IID_IAudioCaptureClient, &self->AudioCapturer))) {
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

    RELEASE(self->AudioCapturer);
    RELEASE(self->AudioClient);
    RELEASE(self->Device);
    RELEASE(enumerator);

    return hr;
}

HRESULT DELTACALL capture_get_format(capture* self, LPWAVEFORMATEX* ppwfxFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppwfxFormat == NULL) {
        return E_INVALIDARG;
    }

    return IAudioClient_GetMixFormat(self->AudioClient, ppwfxFormat);
}

HRESULT DELTACALL capture_capture(capture* self) {
    if (self->Instance == NULL) {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    UINT32 packet = 0; // In frames

    if (SUCCEEDED(hr = IAudioCaptureClient_GetNextPacketSize(self->AudioCapturer, &packet))) {
        if (packet != 0) {
            BYTE* lock = NULL;
            UINT32 frames = 0;
            DWORD flags = AUDCLNT_BUFFERFLAGS_NONE;

            if (SUCCEEDED(hr = IAudioCaptureClient_GetBuffer(self->AudioCapturer, &lock, &frames, &flags, NULL, NULL))) {
                
                // TODO NOT IMPLEMENTED

                // convertor!!
                
                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    // TODO silence
                }
                else {

                }

                hr = IAudioCaptureClient_ReleaseBuffer(self->AudioCapturer, frames);
            }
        }
    }

    return hr;
}

DWORD WINAPI capture_thread(capture_thread_context* ctx) {
    if (FAILED(CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY))) {
        return EXIT_FAILURE;
    }

    HRESULT hr = S_OK;
    capture* device = ctx->Device;

    if (FAILED(hr = capture_initialize(device))) {
        goto exit;
    }

    SetEvent(ctx->Init);

    while (TRUE) {
        const DWORD result =
            WaitForMultipleObjects(CAPTURE_MAX_EVENT_COUNT, device->Events, FALSE, INFINITE);

        if (result == CAPTURE_CLOSE_EVENT_INDEX
            || result == CAPTURE_MAX_EVENT_COUNT) {
            break;
        }
        else if (result == CAPTURE_AUDIO_EVENT_INDEX) {
            if (device->Instance->Buffer != NULL) {
                if (device->Instance->Buffer->Status & DSCBSTATUS_CAPTURING) {
                    hr = capture_capture(device);
                }
            }
        }
    }

    if (device->Format != NULL) {
        allocator_free(device->Allocator, device->Format);
    }

    RELEASE(device->AudioCapturer);
    RELEASE(device->AudioClient);
    RELEASE(device->Device);

    allocator_free(device->Allocator, ctx);

exit:

    CoUninitialize();

    SetEvent(device->ThreadEvent);

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
