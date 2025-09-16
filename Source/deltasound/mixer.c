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
    LPVOID pBuffer, DWORD dwBytes, FLOAT** ppOut, LPDWORD pOutBytes);
HRESULT DELTACALL resample(mixer* pMix, DWORD in_freq, FLOAT* in_buf, DWORD in_buf_len,
    DWORD out_freq, FLOAT** out_buf, DWORD* out_buf_len); // TODO

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

    arena_clear(self->Arena);

    const BOOL main = pMain->Instance->Level == DSSCL_WRITEPRIMARY
        && (pMain->Status & DSBSTATUS_PLAYING);

    HRESULT hr = S_OK;

    if (main) {
        const DWORD in_frequency =
            pMain->Frequency == DSBFREQUENCY_ORIGINAL ? pMain->Format->nSamplesPerSec : pMain->Frequency;

        // Compute out buffer size based on the format and number of frames.
        DWORD in_bytes = (DWORD)((FLOAT)in_frequency
            / (FLOAT)pwfxFormat->Format.nSamplesPerSec
            * dwFrames * pMain->Format->nBlockAlign);

        // TODO Round down to the closest complete frame
        if (in_bytes % pMain->Format->nBlockAlign) {
            in_bytes -= in_bytes % pMain->Format->nBlockAlign;
        }

        const DWORD in_frames = in_bytes / pMain->Format->nBlockAlign;
        DWORD out_bytes = (DWORD)((FLOAT)pwfxFormat->Format.nSamplesPerSec
            / (FLOAT)in_frequency * in_frames * pwfxFormat->Format.nBlockAlign);

        // TODO Round down to the closest complete frame
        if (out_bytes % pwfxFormat->Format.nBlockAlign) {
            out_bytes -= out_bytes % pwfxFormat->Format.nBlockAlign;
        }

        LPVOID buffer_read = NULL;
        
        // TODO
        arena_allocate(self->Arena, in_bytes, &buffer_read);

        DWORD real_read = 0;
        dsbcb_read(pMain->Buffer, in_bytes, buffer_read, &real_read);

        // TODO convert to floats
        FLOAT* float_buf = NULL;
        DWORD float_buf_len = 0;
        mixer_convert_to_float(self, pMain->Format, pMain->Frequency,
            buffer_read, real_read, &float_buf, &float_buf_len);

        // TODO upmix/downmix
        // TODO apply volume and pan
        // TODO resample

        FLOAT* resample_buf = NULL;
        DWORD resample_buf_len = 0;
        resample(self, in_frequency, float_buf, float_buf_len,
            pwfxFormat->Format.nSamplesPerSec, &resample_buf, &resample_buf_len);

        // TODO convert back to intended format

        DWORD read = 0, write = 0;
        dsbcb_get_read_position(pMain->Buffer, &read);
        dsbcb_get_write_position(pMain->Buffer, &write);

        dsbcb_set_read_position(pMain->Buffer, read + real_read, DSBCB_SETPOSITION_WRAP);
        dsbcb_set_write_position(pMain->Buffer, write + real_read, DSBCB_SETPOSITION_WRAP);

        if (pOutBuffer != NULL) {
            *pOutBuffer = resample_buf;
        }

        if (ppdwOutBufferBytes != NULL) {
            *ppdwOutBufferBytes = resample_buf_len;
        }
    }
    else {
        // TODO nothing to play for now
        // TODO support secondary buffers
        goto exit;
    }

    return hr;

exit:
    if (pOutBuffer != NULL) {
        *pOutBuffer = NULL;
    }

    if (ppdwOutBufferBytes != NULL) {
        *ppdwOutBufferBytes = 0;
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

// TODO refactor!
// return data type, etc...
// parameter names, etc...
HRESULT DELTACALL mixer_convert_to_float(mixer* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwFrequency,
    LPVOID pBuffer, DWORD dwBytes, FLOAT** ppOut, LPDWORD pOutBytes) {
    // TODO params check

    const DWORD bits = pwfxFormat->wBitsPerSample;
    const DWORD samples = dwBytes / (pwfxFormat->wBitsPerSample >> 3);

    const DWORD buf_len = samples * sizeof(FLOAT);

    FLOAT* buf = NULL;

    arena_allocate(self->Arena, buf_len, &buf); // TODO success check

    // Convert to float [-1.0, 1.0]

    if (bits == 8) {
        const PBYTE buffer = (PBYTE)pBuffer;

        for (DWORD i = 0; i < samples; i++) {
            buf[i] = ((FLOAT)buffer[i] - 128.0f) / 128.0f;
        }
    }
    else if (bits == 16) {
        const PSHORT buffer = (PSHORT)pBuffer;

        for (DWORD i = 0; i < samples; i++) {
            buf[i] = (FLOAT)buffer[i] / 32768.0f;
        }
    }
    else {
        // TODO NOT IMPLEMENTED
        // return e_notimpl
    }

    *ppOut = buf;
    *pOutBytes = buf_len;

    return S_OK; // TODO
}


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

HRESULT DELTACALL resample(mixer* self, DWORD in_freq, FLOAT* in_buf, DWORD in_buf_len,
    DWORD out_freq, FLOAT** out_buf, DWORD* out_buf_len) {
    // TODO this is mono!
    // NEED To support multiple channels

    const DWORD in_samples = in_buf_len / sizeof(FLOAT);
    const DWORD out_samples = (DWORD)((FLOAT)in_samples / (FLOAT)in_freq * (FLOAT)out_freq);
    const DWORD buf_len = out_samples * sizeof(FLOAT);

    FLOAT* buf = NULL;

    arena_allocate(self->Arena, buf_len, &buf); // TODO success check

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

    return S_OK; // TODO
}
