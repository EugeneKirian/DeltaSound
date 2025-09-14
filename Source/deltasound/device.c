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
#include "ds.h"
#include "dsb.h"
#include "uuid.h"

#define REFTIMES_PER_SEC    10000000
#define TARGET_BUFFER_PADDING_IN_SECONDS  (1.0f / 60.0f) /* TODO NAME */

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }
#define RELEASEHANDLE(X) if((X)) { CloseHandle((X)); (X) = NULL; }

typedef struct device_thread_context {
    device* Device;
    HANDLE  Init;
} device_thread_context;

DWORD WINAPI device_thread(device_thread_context* ctx);

HRESULT DELTACALL device_allocate(allocator* pAlloc, device** ppOut);
HRESULT DELTACALL device_initialize(device* pDev);
HRESULT DELTACALL device_get_period(device* pDev, LPREFERENCE_TIME pDefaultPeriod, LPREFERENCE_TIME pMinPeriod); // TODO is this needed ?
HRESULT DELTACALL device_get_mix_format(device* pDev, LPWAVEFORMATEX* ppWaveFormat);

HRESULT DELTACALL device_play(device* pDev); // TODO

HRESULT DELTACALL device_create(
    allocator* pAlloc, ds* pDS, DWORD dwType, device_info* pInfo, device** ppOut) {
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
        instance->Instance = pDS;

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

        instance->ThreadEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (instance->ThreadEvent == NULL) {
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
        CloseHandle(ctx->Init);

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

    if (self->RefCount == 0) {
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

        // NOTE. Cannot wait for thread handle,
        // because it does not fire when thread is being
        // terminated through the FreeLibrary function call.

        WaitForSingleObject(self->ThreadEvent, INFINITE);
        CloseHandle(self->ThreadEvent);
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
    REFERENCE_TIME period = 0;// TODO is this needed?
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
        AUDCLNT_STREAMFLAGS_NOPERSIST, // TODO ??? | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
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

    //if (FAILED(hr = IAudioClient_SetEventHandle(self->AudioClient, self->AudioEvent))) {
    //    goto exit;
    //}

    if (FAILED(hr = IAudioClient_GetService(self->AudioClient,
        &IID_IAudioRenderClient, &self->AudioRenderer))) {
        goto exit;
    }

    if (FAILED(hr = IAudioClient_GetBufferSize(self->AudioClient, &self->AudioClientBufferSize))) {
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

HRESULT DELTACALL device_get_period(device* self, LPREFERENCE_TIME pDefaultPeriod, LPREFERENCE_TIME pMinPeriod) { // TODO is this needed?
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

// TODO refactor!
// return data type, etc...
// parameter names, etc...
void DELTACALL convert_to_float(WAVEFORMATEX* format,
    void* in_buf, DWORD in_buf_len, FLOAT** out_buf, DWORD* out_buf_len) {
    const DWORD bits = format->wBitsPerSample;
    const DWORD samples = in_buf_len / (format->wBitsPerSample >> 3);

    const DWORD buf_len = samples * sizeof(FLOAT);

    float* buf = (float*)calloc(buf_len, 1);

    if (buf == NULL) {
        return;
    }

    // Convert to float [-1.0, 1.0]

    if (bits == 8) {
        const PBYTE buffer = (PBYTE)in_buf;

        for (DWORD i = 0; i < samples; i++) {
            buf[i] = ((FLOAT)buffer[i] - 128.0f) / 128.0f;
        }
    }
    else if (bits == 16) {
        const PSHORT buffer = (PSHORT)in_buf;

        for (DWORD i = 0; i < samples; i++) {
            buf[i] = (FLOAT)buffer[i] / 32768.0f;
        }
    }
    else {
        // TODO NOT IMPLEMENTED
    }

    *out_buf_len = buf_len;
    *out_buf = buf;
}

#include <math.h>

// TODO refactor!
// Linear interpolation...

/*

Common PCM Resampling Algorithms:

    Linear Interpolation:

This is a simple method where new sample values are estimated by drawing a straight line between two existing samples. While computationally inexpensive, it can introduce audible artifacts, particularly for significant sample rate changes.
Cubic Spline Interpolation:
This method uses a cubic polynomial to interpolate between samples, resulting in a smoother curve and generally better audio quality than linear interpolation, but with increased computational cost.
Sinc Interpolation (Kaiser Window-Sinc Filter):
Considered a high-quality method, sinc interpolation uses a sinc function, often with a windowing function like the Kaiser window, to reconstruct the continuous-time signal from the discrete samples and then resample it at the desired rate. This offers excellent fidelity but is more computationally intensive.
Lagrange Interpolation:
This method uses Lagrange polynomials to create a polynomial that passes through all given data points, allowing for interpolation of new sample values.

Key Concepts in Resampling:

    Anti-aliasing Filter (Low-Pass Filter - LPF):

When downsampling (reducing the sample rate), it is crucial to apply an anti-aliasing filter before the resampling process. This filter removes frequencies above the new Nyquist frequency (half the new sample rate) to prevent aliasing, which can cause distortion and unwanted artifacts.
Dithering:
When reducing the bit depth during resampling, dithering can be applied to add a small amount of random noise to the signal. This helps to decorrelate quantization errors and reduce audible quantization noise.
Noise Shaping:
Often used in conjunction with dithering, noise shaping moves quantization noise to less audible frequency ranges, further improving perceived audio quality at lower bit depths.

Algorithm Steps (General Outline):

    Normalization (Optional):
    If necessary, convert PCM samples to a floating-point representation (e.g., -1.0 to 1.0) for easier processing, especially when mixing multiple sources.
    Filtering (for downsampling):
    Apply an appropriate anti-aliasing low-pass filter to prevent aliasing.
    Interpolation/Decimation:
    Calculate new sample values at the target sample rate using the chosen interpolation algorithm (e.g., linear, cubic, sinc). This involves either interpolating between existing samples (upsampling) or selecting a subset of samples (downsampling with decimation).
    Quantization and Dithering/Noise Shaping (for bit depth reduction):
    If the target bit depth is lower than the source, quantize the samples and apply dithering or noise shaping to minimize quantization noise.
    Conversion to Target Format:
    Convert the processed floating-point samples back to the desired PCM integer format and bit depth.
*/

VOID DELTACALL resample(size_t in_freq, FLOAT* in_buf, DWORD in_buf_len,
    DWORD out_freq, FLOAT** out_buf, DWORD* out_buf_len) {
    // TODO this is mono!
    // NEED To support multiple channels

    const DWORD in_samples = in_buf_len / sizeof(FLOAT);
    const DWORD out_samples = (DWORD)((FLOAT)in_samples / (FLOAT)in_freq * (FLOAT)out_freq);
    const DWORD buf_len = out_samples * sizeof(FLOAT);

    FLOAT* buf = (FLOAT*)calloc(buf_len, 1);

    if (buf == NULL) {
        return;
    }

    const FLOAT ratio = (FLOAT)in_freq / (FLOAT)out_freq;

    for (DWORD i = 0; i < out_samples; ++i) {
        // Find the floating-point position in the input buffer for the current output sample
        FLOAT source_position = i * ratio;

        // Get the integer index of the first sample (p1)
        DWORD index_p1 = (DWORD)floor(source_position);

        // Get the fractional part for interpolation
        float fraction = source_position - index_p1;

        // Handle edge case for the last sample
        if (index_p1 >= in_samples - 1) {
            buf[i] = in_buf[in_samples - 1];
            continue;
        }

        // Get the two adjacent samples for interpolation
        FLOAT p1 = in_buf[index_p1];
        FLOAT p2 = in_buf[index_p1 + 1];

        // Perform linear interpolation
        buf[i] = p1 + fraction * (p2 - p1);
    }

    *out_buf = buf;
    *out_buf_len = buf_len;
}

HRESULT DELTACALL read_from_circular_buffer(void* buffer, DWORD buffer_size,
    DWORD start, DWORD end, DWORD length, void* output, LPDWORD pdwLength) { // TODO name, var names
    // TODO input validation

    const DWORD max_read =
        start < end ? end - start : buffer_size - start + end;

    if (max_read < length) {
        length = max_read;
    }

    const DWORD read = min(length, buffer_size - start);
    CopyMemory(output, (void*)((size_t)buffer + start), read);

    if (read < length) {
        CopyMemory((void*)((size_t)output + read), buffer, length - read);
    }

    *pdwLength = length;

    return S_OK;
}

HRESULT DELTACALL device_play(device* self) { // TODO name, etc...
    if (self->Instance == NULL) { return E_FAIL; }
    if (self->Instance->Main == NULL) { return E_FAIL; }

    HRESULT hr = S_OK;

    dsb* main = self->Instance->Main;

    const UINT32 target =
        (UINT32)(self->AudioClientBufferSize * TARGET_BUFFER_PADDING_IN_SECONDS);

    if (main->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (main->Status & DSBSTATUS_PLAYING) {
            UINT32 padding = 0;

            if (SUCCEEDED(hr = IAudioClient_GetCurrentPadding(self->AudioClient, &padding))) {
                const UINT32 frames = target - padding;
                // TODO needed calculation for non-looping buffers
                // min(target - padding, wav->dwNumFrames - audio->nCurrentFrame);

                if (frames != 0) {
                    //if (frames == target) {
                    BYTE* lock = NULL;
                    if (SUCCEEDED(IAudioRenderClient_GetBuffer(self->AudioRenderer, frames, &lock))) {

                        // Compute out buffer size based on the format and number of frames.
                        DWORD in_bytes = (DWORD)((FLOAT)main->Format->nSamplesPerSec
                            / (FLOAT)self->WaveFormat->Format.nSamplesPerSec
                            * frames * main->Format->nBlockAlign);

                        // Round down to the closest complete frame
                        if (in_bytes % main->Format->nBlockAlign) {
                            in_bytes -= in_bytes % main->Format->nBlockAlign;
                        }

                        const DWORD in_frames = in_bytes / main->Format->nBlockAlign;
                        DWORD out_bytes = (DWORD)((FLOAT)self->WaveFormat->Format.nSamplesPerSec
                            / (FLOAT)main->Format->nSamplesPerSec
                            * in_frames * self->WaveFormat->Format.nBlockAlign);

                        // Round down to the closest complete frame
                        if (out_bytes % self->WaveFormat->Format.nBlockAlign) {
                            out_bytes -= out_bytes % self->WaveFormat->Format.nBlockAlign;
                        }

                        // Convert needed frames to input frames as a ratio of frequencies.
                        //const UINT32 in_frames =
                        //    (UINT32)((FLOAT)main->Format->nSamplesPerSec
                        //        / (FLOAT)self->WaveFormat->Format.nSamplesPerSec * (FLOAT)frames);

                        // TODO this is incorrect ^^
                        // There are frames dropped due to rounding, for example
                        // 800 frames becomes 798 frames (when calculating buffer size, etc.

                        // TODO
                        // this assumes that input and output formats
                        // have the same number of chanels

                        //const DWORD buffer_read_length = in_frames * main->Format->nBlockAlign;
                        //void* buffer_read = malloc(buffer_read_length);
                        void* buffer_read = malloc(in_bytes);

                        DWORD real_read = 0;
                        //read_from_circular_buffer(main->Buffer, main->Caps.dwBufferBytes,
                        //    main->CurrentPlayCursor, buffer_read_length, buffer_read);
                        
                        read_from_circular_buffer(main->Buffer, main->Caps.dwBufferBytes,
                            main->CurrentPlayCursor, main->CurrentWriteCursor, in_bytes, buffer_read, &real_read);

                        //{
                        //    real_read = in_bytes;
                        //    size_t copy = in_bytes;

                        //    if (copy + main->CurrentPlayCursor < main->Caps.dwBufferBytes) {
                        //        CopyMemory(buffer_read, (void*)((size_t)main->Buffer + main->CurrentPlayCursor), copy);
                        //    }
                        //    else {
                        //        size_t copy1 = main->Caps.dwBufferBytes - main->CurrentPlayCursor;
                        //        CopyMemory(buffer_read, (void*)((size_t)main->Buffer + main->CurrentPlayCursor), copy1);
                        //        CopyMemory((void*)((size_t)buffer_read + copy1), main->Buffer, copy - copy1);
                        //    }
                        //}

                        if (in_bytes != real_read) {
                            int kk = 1;// TODO
                        }

                        //fprintf(f, "Reading at: %d Size %d Read %d Input bytes %d Input frames %d Output bytes %d\r\n",
                        //    main->CurrentPlayCursor, in_bytes, real_read, in_bytes, in_frames, out_bytes);

                        float* float_buf = NULL;
                        DWORD float_buf_len = 0;
                        //convert_to_float(main->Format, buffer_read, buffer_read_length,
                        //    &float_buf, &float_buf_len);
                        convert_to_float(main->Format, buffer_read, real_read,
                            &float_buf, &float_buf_len);

                        float* resample_buf = NULL;
                        DWORD resample_buf_len = 0;
                        resample(main->Format->nSamplesPerSec, float_buf, float_buf_len,
                            self->WaveFormat->Format.nSamplesPerSec, &resample_buf, &resample_buf_len);

                        if (out_bytes != resample_buf_len) {
                            int kkk = 1; // TODO
                        }



                        const UINT32 out_frames = resample_buf_len / self->WaveFormat->Format.nBlockAlign;

                        //fprintf(f, "Resampled frames %d\r\n", out_frames);

                        //CopyMemory(lock, resample_buf, out_bytes); // resample_buf_len);
                        CopyMemory(lock, resample_buf, resample_buf_len);

                        // TODO memory management!!!
                        free(resample_buf);
                        free(float_buf);
                        free(buffer_read);

                        // Update play and write cursors.
                        //main->CurrentPlayCursor =
                        //    (main->CurrentPlayCursor + buffer_read_length) % main->Caps.dwBufferBytes;
                        //main->CurrentWriteCursor =
                        //    (main->CurrentWriteCursor + buffer_read_length) % main->Caps.dwBufferBytes;
                        main->CurrentPlayCursor =
                            (main->CurrentPlayCursor + real_read) % main->Caps.dwBufferBytes;
                        main->CurrentWriteCursor =
                            (main->CurrentWriteCursor + real_read) % main->Caps.dwBufferBytes;

                        //IAudioRenderClient_ReleaseBuffer(self->AudioRenderer, frames, 0);

                        IAudioRenderClient_ReleaseBuffer(self->AudioRenderer, out_frames, 0);

                        // TODO for non-looping buffers
                        //if (wav->dwNumFrames <= audio->nCurrentFrame) {
                        //    audio->dwState = AUDIOSTATE_IDLE;
                        //}
                    }
                }
            }
        }
        else {
            main->CurrentPlayCursor = 0;
            main->CurrentWriteCursor = 0;

            // TODO optimize, call this only when buffer stopps.
            //BYTE* lock = NULL;
            //if (SUCCEEDED(IAudioRenderClient_GetBuffer(self->AudioRenderer, 0, &lock))) {
            //    IAudioRenderClient_ReleaseBuffer(self->AudioRenderer, 0, AUDCLNT_BUFFERFLAGS_SILENT);
            //}

            return DSERR_BUFFERLOST; // TODO Better error!
        }
    }

    // TODO secondary buffers, mixing, etc...

    return hr;
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
        if (FAILED(device_play(dev))) {

            // TODO
            Sleep(1);
        }
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

    SetEvent(dev->ThreadEvent);

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
