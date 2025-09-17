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

#include "arena.h"
#include "ds.h"
#include "mixer.h"

#include <math.h>

struct mixer {
    allocator*  Allocator;
    arena*      Arena;
};

HRESULT DELTACALL mixer_allocate(allocator* pAlloc, mixer** ppOut);

HRESULT DELTACALL mixer_convert_to_float(mixer* pMix,
    LPWAVEFORMATEX pwfxFormat, DWORD dwFrequency,
    LPVOID pInBuffer, DWORD dwInBufferBytes, FLOAT** ppOutBuffer, LPDWORD pOutBufferBytes);
HRESULT DELTACALL mixer_resample(mixer* pMix,
    DWORD dwChannels, DWORD dwInFrequency, FLOAT* pfInBuffer,
    DWORD dwInBufferBytes, DWORD dwOutFrequency, FLOAT** ppfOutBuffer, LPDWORD pdwOutBufferBytes);

inline FLOAT linear_interpolate(FLOAT v1, FLOAT v2, FLOAT t) {
    return v1 * (1.0f - t) + v2 * t;
}

HRESULT DELTACALL mixer_create(allocator* pAlloc, mixer** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    mixer* instance = NULL;

    if (SUCCEEDED(hr = mixer_allocate(pAlloc, &instance))) {
        if (SUCCEEDED(hr = arena_create(pAlloc, &instance->Arena))) {
            *ppOut = instance;
            return S_OK;
        }

        mixer_release(instance);
    }

    return hr;
}

VOID DELTACALL mixer_release(mixer* self) {
    if (self == NULL) { return; }

    arena_release(self->Arena);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL mixer_mix(mixer* self, PWAVEFORMATEXTENSIBLE pwfxFormat,
    DWORD dwFrames, dsb* pMain, arr* pSecondary, LPVOID* pOutBuffer, LPDWORD ppdwOutBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxFormat == NULL || dwFrames == 0
        || pMain == NULL || pOutBuffer == NULL || ppdwOutBufferBytes == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    if (FAILED(hr = arena_clear(self->Arena))) {
        return hr;
    }

    const BOOL main = pMain->Instance->Level == DSSCL_WRITEPRIMARY
        && (pMain->Status & DSBSTATUS_PLAYING);

    if (main) {
        const DWORD in_frequency = pMain->Frequency == DSBFREQUENCY_ORIGINAL
            ? pMain->Format->nSamplesPerSec : pMain->Frequency;

        // Compute out buffer size based on the format and number of frames.
        DWORD in_bytes = (DWORD)((FLOAT)in_frequency
            / (FLOAT)pwfxFormat->Format.nSamplesPerSec
            * dwFrames * pMain->Format->nBlockAlign);

        // TODO Round down to the closest complete frame.
        if (in_bytes % pMain->Format->nBlockAlign != 0) {
            in_bytes -= in_bytes % pMain->Format->nBlockAlign;
        }

        // TODO use BLOCKALIGN ?

        const DWORD in_frames = in_bytes / pMain->Format->nBlockAlign;
        DWORD out_bytes = (DWORD)((FLOAT)pwfxFormat->Format.nSamplesPerSec
            / (FLOAT)in_frequency * in_frames * pwfxFormat->Format.nBlockAlign);

        // TODO Round down to the closest complete frame.
        if (out_bytes % pwfxFormat->Format.nBlockAlign != 0) {
            out_bytes -= out_bytes % pwfxFormat->Format.nBlockAlign;
        }

        // TODO use BLOCKALIGN ?

        // Read the data from buffer's circular buffer into a linear block of memory.

        LPVOID in_buffer = NULL;
        DWORD in_buffer_length = 0;

        if (FAILED(hr = arena_allocate(self->Arena, in_bytes, &in_buffer))) {
            return hr;
        }

        if (FAILED(hr = dsbcb_read(pMain->Buffer, in_bytes, in_buffer, &in_buffer_length))) {
            return hr;
        }

        // Convert input buffer's data to floats.
        FLOAT* float_buf = NULL;
        DWORD float_buf_len = 0;

        if (FAILED(hr = mixer_convert_to_float(self, pMain->Format,
            in_frequency, in_buffer, in_buffer_length, &float_buf, &float_buf_len))) {
            return hr;
        }

        // TODO upmix/downmix - number of channels
        // TODO apply volume and pan

        // Resample the buffer to requested frequency.
        FLOAT* resample_buf = NULL;
        DWORD resample_buf_len = 0;

        if (FAILED(hr = mixer_resample(self, pwfxFormat->Format.nChannels,
            in_frequency, float_buf, float_buf_len, pwfxFormat->Format.nSamplesPerSec,
            &resample_buf, &resample_buf_len))) {
            return hr;
        }

        // TODO convert back to intended format

        // Update the read and write positions but only if the buffer is still playing.
        // This is to avoid race condition with ::Stop method being called at ill-timed moment.

        DWORD status = 0;

        if (SUCCEEDED(hr = dsb_get_status(pMain, &status))) {
            if (status & DSBSTATUS_PLAYING) {
                DWORD read = 0, write = 0;
                if (SUCCEEDED(hr = dsbcb_get_current_position(pMain->Buffer, &read, &write))) {
                    hr = dsbcb_set_current_position(pMain->Buffer,
                        read + in_buffer_length, write + in_buffer_length, DSBCB_SETPOSITION_WRAP);
                }
            }
        }

        *pOutBuffer = resample_buf;
        *ppdwOutBufferBytes = resample_buf_len;
    }
    else {
        // TODO nothing to play for now
        // TODO support secondary buffers
        // TODO if secondary buffer is not looping - stop it when it reaches the end 
        // TODO handle position notifications.
        // TODO handle position notifications when buffer stopped in the middle of playback
    }

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL mixer_allocate(allocator* pAlloc, mixer** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    mixer* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(mixer), &instance))) {
        instance->Allocator = pAlloc;
        *ppOut = instance;
    }

    return hr;
}

HRESULT DELTACALL mixer_convert_to_float(mixer* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwFrequency,
    LPVOID pInBuffer, DWORD dwInBufferBytes, FLOAT** ppOutBuffer, LPDWORD pOutBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxFormat == NULL || dwFrequency == 0
        || pInBuffer == NULL || dwInBufferBytes == 0
        || ppOutBuffer == NULL || pOutBufferBytes == NULL) {
        return E_INVALIDARG;
    }

    if (pwfxFormat->wBitsPerSample != 8
        && pwfxFormat->wBitsPerSample != 16) {
        return E_NOTIMPL;
    }

    const DWORD samples = dwInBufferBytes / (pwfxFormat->wBitsPerSample >> 3);
    const DWORD length = samples * sizeof(FLOAT);

    HRESULT hr = S_OK;
    FLOAT* buf = NULL;

    if (FAILED(hr = arena_allocate(self->Arena, length, &buf))) {
        return hr;
    }

    // Convert to float [-1.0, 1.0]

    if (pwfxFormat->wBitsPerSample == 8) {
        const PBYTE buffer = (PBYTE)pInBuffer;

        for (DWORD i = 0; i < samples; i++) {
            buf[i] = ((FLOAT)buffer[i] - 128.0f) / 128.0f;
        }
    }
    else if (pwfxFormat->wBitsPerSample == 16) {
        const PSHORT buffer = (PSHORT)pInBuffer;

        for (DWORD i = 0; i < samples; i++) {
            buf[i] = (FLOAT)buffer[i] / 32768.0f;
        }
    }

    *ppOutBuffer = buf;
    *pOutBufferBytes = length;

    return S_OK;
}

/*
TODO

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

HRESULT DELTACALL mixer_resample(mixer* self,
    DWORD dwChannels, DWORD dwInFrequency, FLOAT* pfInBuffer,
    DWORD dwInBufferBytes, DWORD dwOutFrequency, FLOAT** ppfOutBuffer, LPDWORD pdwOutBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwChannels == 0 || dwInFrequency == 0
        || pfInBuffer == NULL || dwInBufferBytes == 0
        || dwOutFrequency == 0 || ppfOutBuffer == NULL || pdwOutBufferBytes == NULL) {
        return E_INVALIDARG;
    }

    const FLOAT ratio = (FLOAT)dwInFrequency / (FLOAT)dwOutFrequency;

    const DWORD in_samples = dwInBufferBytes / sizeof(FLOAT);
    const DWORD in_frames = in_samples / dwChannels;

    const DWORD out_samples = (DWORD)((FLOAT)in_samples / (FLOAT)dwInFrequency * (FLOAT)dwOutFrequency);
    const DWORD out_frames = out_samples / dwChannels;

    HRESULT hr = S_OK;
    FLOAT* buffer = NULL;
    const DWORD length = out_samples * sizeof(FLOAT);

    if (FAILED(hr = arena_allocate(self->Arena, length, &buffer))) {
        return hr;
    }

    for (DWORD i = 0; i < out_frames; i++) {
        for (DWORD c = 0; c < dwChannels; c++) {
            // Find the floating-point position
            // in the input buffer for the current output sample.
            const FLOAT t = i * ratio;

            DWORD index0 = (DWORD)t;
            DWORD index1 = index0 + 1;

            // Handle edge case for the last sample.
            if (index1 >= in_samples / dwChannels) {
                index1 = in_samples / dwChannels - 1;
                index0 = index1 - 1;
            }

            // Perform linear interpolation.
            const FLOAT y0 = pfInBuffer[index0 * dwChannels + c];
            const FLOAT y1 = pfInBuffer[index1 * dwChannels + c];

            buffer[i * dwChannels + c] = linear_interpolate(y0, y1, t - index0);
        }
    }

    *ppfOutBuffer = buffer;
    *pdwOutBufferBytes = length;

    return S_OK;
}
