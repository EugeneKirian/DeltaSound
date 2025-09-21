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
#include "wave_format.h"

#include <math.h>

struct mixer {
    allocator*  Allocator;
    arena*      Arena;
};

HRESULT DELTACALL mixer_convert_to_float(mixer* pMix,
    LPWAVEFORMATEX pwfxFormat, DWORD dwFrequency,
    LPVOID pInBuffer, DWORD dwInBufferBytes, FLOAT** ppOutBuffer, LPDWORD pOutBufferBytes);

HRESULT DELTACALL mixer_convert_to_ieee(mixer* self,
    DWORD dwBits, DWORD dwChannels, DWORD dwFrames,
    LPVOID pInBuffer, DWORD dwInBufferBytes, FLOAT** ppOutBuffer, LPDWORD pOutBufferBytes);
HRESULT DELTACALL mixer_resample(mixer* pMix,
    DWORD dwChannels, DWORD dwInFrequency, FLOAT* pfInBuffer,
    DWORD dwInBufferBytes, DWORD dwOutFrequency, FLOAT** ppfOutBuffer, LPDWORD pdwOutBufferBytes);
HRESULT DELTACALL mixer_attenuate(mixer* self, DWORD dwFrames, FLOAT fVolume, FLOAT fPan, FLOAT* pfBuffer);

inline FLOAT linear_interpolate(FLOAT v1, FLOAT v2, FLOAT t) {
    return v1 * (1.0f - t) + v2 * t;
}

HRESULT DELTACALL mixer_create(allocator* pAlloc, mixer** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    mixer* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(mixer), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = arena_create(pAlloc, &instance->Arena))) {

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL mixer_release(mixer* self) {
    if (self == NULL) { return; }

    arena_release(self->Arena);

    allocator_free(self->Allocator, self);
}

// TODO refactor this plug
// TODO this pretends that secondary buffer is same as primary
// TODO add support of multiple buffers
/*
HRESULT DELTACALL mixer_play_buffer(mixer* self, DWORD dwBufferCount, dsb* pBuffers,
    PWAVEFORMATEXTENSIBLE pwfxFormat, DWORD dwFrames, LPVOID* pOutBuffer, LPDWORD ppdwOutBufferBytes) {



    const DWORD in_frequency = buffer->Frequency == DSBFREQUENCY_ORIGINAL
        ? buffer->Format->nSamplesPerSec : buffer->Frequency;

    // Compute out buffer size based on the format and number of frames.
    DWORD in_bytes = (DWORD)((FLOAT)in_frequency
        / (FLOAT)pwfxFormat->Format.nSamplesPerSec
        * dwFrames * buffer->Format->nBlockAlign);

    // TODO Round down to the closest complete frame.
    if (in_bytes % buffer->Format->nBlockAlign != 0) {
        in_bytes -= in_bytes % buffer->Format->nBlockAlign;
    }

    // TODO use BLOCKALIGN ?

    const DWORD in_frames = in_bytes / buffer->Format->nBlockAlign;
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

    HRESULT hr = S_OK;

    if (FAILED(hr = arena_allocate(self->Arena, in_bytes, &in_buffer))) {
        return hr;
    }

    if (FAILED(hr = dsbcb_read(buffer->Buffer, in_bytes, in_buffer, &in_buffer_length))) {
        return hr;
    }

    // Convert input buffer's data to floats.
    FLOAT* float_buf = NULL;
    DWORD float_buf_len = 0;

    if (FAILED(hr = mixer_convert_to_float(self, buffer->Format,
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

    DWORD status = DSBSTATUS_NONE;

    if (SUCCEEDED(hr = dsb_get_status(buffer, &status))) {
        if (status & DSBSTATUS_PLAYING) {
            DWORD read = 0, write = 0;
            if (SUCCEEDED(hr = dsbcb_get_current_position(buffer->Buffer, &read, &write))) {
                hr = dsbcb_set_current_position(buffer->Buffer,
                    read + in_buffer_length, write + in_buffer_length, DSBCB_SETPOSITION_WRAP);
            }
        }
    }

    *pOutBuffer = resample_buf;
    *ppdwOutBufferBytes = resample_buf_len;

    return hr;
}
*/

typedef struct mix_buffer {
    dsb*    Instance;

    LPVOID  InData;
    DWORD   InDataSize;

    FLOAT*  WorkData;
    DWORD   WorkDataSize;

    FLOAT*  Out;
    DWORD   OutSize;
} mix_buffer; // TODO better names

// TODO
HRESULT DELTACALL mixer_get_buffer_length(LPCWAVEFORMATEX pwfxIn,
    DWORD dwFrequency, LPCWAVEFORMATEX pwfxOut, DWORD dwFrames, LPDWORD pdwBytes) {
    // TODO validations

    const DWORD frequency = dwFrequency == DSBFREQUENCY_ORIGINAL
        ? pwfxIn->nSamplesPerSec : dwFrequency;

    const FLOAT ratio = (FLOAT)frequency / (FLOAT)pwfxOut->nSamplesPerSec;

    *pdwBytes = (DWORD)roundf(ratio * dwFrames) * pwfxIn->nBlockAlign;

    return S_OK;
}

HRESULT DELTACALL mixer_mix(mixer* self, DWORD dwBuffers, dsb** ppBuffers,
    PWAVEFORMATEXTENSIBLE pwfxFormat, DWORD dwFrames, LPVOID* pOutBuffer, LPDWORD pdwOutBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxFormat == NULL || dwFrames == 0
        || dwBuffers == 0 || ppBuffers == NULL
        || pOutBuffer == NULL || pdwOutBufferBytes == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if (FAILED(hr = arena_clear(self->Arena))) {
        return hr;
    }

    mix_buffer* buffers = NULL;
    const DWORD buffers_length = dwBuffers * sizeof(mix_buffer);

    if (FAILED(hr = arena_allocate(self->Arena, buffers_length, &buffers))) {
        return hr;
    }

    ZeroMemory(buffers, buffers_length);

    // TODO multi-threaded mixing

    // Read the data from the user-defined buffers.
    for (DWORD i = 0; i < dwBuffers; i++) {
        buffers[i].Instance = ppBuffers[i];

        DWORD length = 0;

        if (FAILED(hr = mixer_get_buffer_length(ppBuffers[i]->Format,
            ppBuffers[i]->Frequency, &pwfxFormat->Format, dwFrames, &length))) {
            return hr;
        }

        if (FAILED(hr = arena_allocate(self->Arena, length, &buffers[i].InData))) {
            return hr;
        }

        if (FAILED(hr = dsbcb_read(ppBuffers[i]->Buffer,
            length, buffers[i].InData, &buffers[i].InDataSize,
            (ppBuffers[i]->Status & DSBSTATUS_LOOPING) ? DSBCB_READ_LOOPING : DSBCB_READ_NONE))) {
            return hr;
        }
    }

    // TODO. Assumption is that inputs are non-IEEE PCM.

    // Convert to IEEE, upmix/downmix audio to stereo, and fill missing frames with silence.
    for (DWORD i = 0; i < dwBuffers; i++) {
        if (FAILED(hr = mixer_convert_to_ieee(self,
            ppBuffers[i]->Format->wBitsPerSample, ppBuffers[i]->Format->nChannels,
            dwFrames, buffers[i].InData, buffers[i].InDataSize,
            &buffers[i].WorkData, &buffers[i].WorkDataSize))) {
            return hr;
        }
    }

    // At this point all audio data is IEEE and stereo.
    // Resample the audio to the requested audio frequency.
    for (DWORD i = 0; i < dwBuffers; i++) {
        const DWORD frequency = ppBuffers[i]->Frequency == DSBFREQUENCY_ORIGINAL
            ? ppBuffers[i]->Format->nSamplesPerSec : ppBuffers[i]->Frequency;

        if (FAILED(hr = mixer_resample(self,
            2 /* STEREO */, frequency,
            buffers[i].WorkData, buffers[i].WorkDataSize,
            pwfxFormat->Format.nSamplesPerSec, &buffers[i].Out, &buffers[i].OutSize))) {
            return hr;
        }
    }

    // Apply attenuation (volume and pan modifiers).
    for (DWORD i = 0; i < dwBuffers; i++) {
        if (FAILED(hr = mixer_attenuate(self, dwFrames,
            ppBuffers[i]->Volume, ppBuffers[i]->Pan, buffers[i].Out))) {
            return hr;
        }
    }

    // Mix all sound buffers together.
    FLOAT* result = NULL;
    const DWORD result_length = buffers[0].OutSize;

    if (FAILED(hr = arena_allocate(self->Arena, result_length, &result))) {
        return hr;
    }

    for (DWORD i = 0; i < dwFrames; i++) {
        FLOAT l = 0.0f;
        FLOAT r = 0.0f;

        for (DWORD k = 0; k < dwBuffers; k++) {
            l += buffers[k].Out[i * 2 /* STEREO */ + 0];
            r += buffers[k].Out[i * 2 /* STEREO */ + 1];
        }

        result[i * 2 /* STEREO */ + 0] = l / (FLOAT)dwBuffers;
        result[i * 2 /* STEREO */ + 1] = r / (FLOAT)dwBuffers;
    }

    // Convert audio data to requested wave format.
    if (pwfxFormat->Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE
        && IsEqualGUID(&pwfxFormat->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
        // TODO
        return E_NOTIMPL;
    }

    // TODO updating of statuses and triggering notifications should NOT be inside mixer

    for (DWORD i = 0; i < dwBuffers; i++) {
        DWORD status = DSBSTATUS_NONE;

        if (SUCCEEDED(hr = dsb_get_status(ppBuffers[i], &status))) {
            if (status & DSBSTATUS_PLAYING) {
                DWORD read = 0, write = 0;

                if (SUCCEEDED(hr = dsbcb_get_current_position(ppBuffers[i]->Buffer, &read, &write))) {
                    hr = dsbcb_set_current_position(ppBuffers[i]->Buffer,
                        read + buffers[i].InDataSize, write + buffers[i].InDataSize,
                        DSBCB_SETPOSITION_LOOPING /* TODO proper flags for single play buffers*/);
                }
            }
        }
    }

    // TODO  Trigger appropriate notifications.

    *pOutBuffer = result;
    *pdwOutBufferBytes = result_length;

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL mixer_attenuate(mixer* self, DWORD dwFrames, FLOAT fVolume, FLOAT fPan, FLOAT* pfBuffer) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwFrames == 0 || pfBuffer == NULL) {
        return E_INVALIDARG;
    }

    if (_isnan(fVolume) || fVolume < 0.0f || fVolume > 1.0f) {
        return E_INVALIDARG;
    }

    if (_isnan(fPan) || fPan < -1.0f || fPan > 1.0f) {
        return E_INVALIDARG;
    }

    if (fVolume == 1.0f && fPan == 0.0f) {
        return S_OK;
    }

    // Positive pan - attenuation of left channel.
    const FLOAT l = fVolume * (fPan > 0.0f ? (1.0f - fPan) : 1.0f);

    // Negative pan - attenuation of right channel.
    const FLOAT r = fVolume * (fPan < 0.0f ? (1.0f + fPan) : 1.0f);

    for (DWORD i = 0; i < dwFrames; i++) {
        pfBuffer[i * 2 /* STEREO */ + 0] = pfBuffer[i * 2 /* STEREO */ + 0] * l;
        pfBuffer[i * 2 /* STEREO */ + 1] = pfBuffer[i * 2 /* STEREO */ + 1] * r;
    }

    return S_OK;
}

// TODO name
// TODO forward declare
// TODO signature
inline FLOAT DELTACALL convert_to_float(DWORD dwBits, LPVOID pValue) {
    if (dwBits == 8) {
        return ((FLOAT)(*(PBYTE)pValue) - 128.0f) / 128.0f;
    }
    else if (dwBits == 16) {
        return  (FLOAT)(*(PSHORT)pValue) / 32768.0f;
    }

    return 0.0f;
}

HRESULT DELTACALL mixer_convert_to_ieee(mixer* self,
    DWORD dwBits, DWORD dwChannels, DWORD dwFrames,
    LPVOID pInBuffer, DWORD dwInBufferBytes, FLOAT** ppOutBuffer, LPDWORD pOutBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwBits == 0 || dwChannels == 0 || dwFrames == 0
        || pInBuffer == NULL || dwInBufferBytes == 0
        || ppOutBuffer == NULL || pOutBufferBytes == NULL) {
        return E_INVALIDARG;
    }

    if (dwBits != 8 && dwBits != 16) {
        return E_NOTIMPL;
    }

    if (dwChannels != 1 && dwChannels != 2) {
        return E_NOTIMPL;
    }

    HRESULT hr = S_OK;
    FLOAT* buffer = NULL;
    const DWORD length = dwFrames * 2 /* STEREO */ * sizeof(FLOAT);

    if (FAILED(hr = arena_allocate(self->Arena, length, &buffer))) {
        return hr;
    }

    ZeroMemory(buffer, length);

    const DWORD bytes = dwBits >> 3;
    DWORD offset = 0;

    for (DWORD i = 0; i < dwFrames; i++) {
        if (dwChannels == 1) {
            const FLOAT v =
                convert_to_float(dwBits, (LPVOID)((size_t)pInBuffer + offset));

            buffer[i * 2 /* STEREO */ + 0] = v;
            buffer[i * 2 /* STEREO */ + 1] = v;

            offset += bytes;
        }
        else {
            const FLOAT v1 =
                convert_to_float(dwBits, (LPVOID)((size_t)pInBuffer + offset + 0));
            const FLOAT v2 =
                convert_to_float(dwBits, (LPVOID)((size_t)pInBuffer + offset + 2));

            buffer[i * 2 /* STEREO */ + 0] = v1;
            buffer[i * 2 /* STEREO */ + 1] = v2;

            offset += bytes * dwChannels;
        }

        if (dwInBufferBytes <= offset) {
            // Input buffer has fewer frames than required.
            // Stop processing and leave the rest as silence (zeros).
            break;
        }
    }

    *ppOutBuffer = buffer;
    *pOutBufferBytes = length;

    return S_OK;
}

// TODO not used
HRESULT DELTACALL mixer_convert_to_float(mixer* self,
    LPWAVEFORMATEX pwfxIn, DWORD dwFrequency,
    LPVOID pInBuffer, DWORD dwInBufferBytes, FLOAT** ppOutBuffer, LPDWORD pOutBufferBytes) {
    if (pwfxIn == NULL || dwFrequency == 0
        || pInBuffer == NULL || dwInBufferBytes == 0
        || ppOutBuffer == NULL || pOutBufferBytes == NULL) {
        return E_INVALIDARG;
    }

    if (pwfxIn->wBitsPerSample != 8
        && pwfxIn->wBitsPerSample != 16) {
        return E_NOTIMPL;
    }

    const DWORD samples = dwInBufferBytes / (pwfxIn->wBitsPerSample >> 3);
    const DWORD length = samples * sizeof(FLOAT);

    HRESULT hr = S_OK;
    FLOAT* buf = NULL;

    if (FAILED(hr = arena_allocate(self->Arena, length, &buf))) {
        return hr;
    }

    // Convert to float [-1.0, 1.0]

    if (pwfxIn->wBitsPerSample == 8) {
        const PBYTE buffer = (PBYTE)pInBuffer;

        for (DWORD i = 0; i < samples; i++) {
            buf[i] = ((FLOAT)buffer[i] - 128.0f) / 128.0f;
        }
    }
    else if (pwfxIn->wBitsPerSample == 16) {
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
    DWORD dwChannels, DWORD dwInFrequency,
    FLOAT* pfInBuffer, DWORD dwInBufferBytes,
    DWORD dwOutFrequency, FLOAT** ppfOutBuffer, LPDWORD pdwOutBufferBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwChannels == 0 || dwInFrequency == 0
        || pfInBuffer == NULL || dwInBufferBytes == 0
        || dwOutFrequency == 0 || ppfOutBuffer == NULL || pdwOutBufferBytes == NULL) {
        return E_INVALIDARG;
    }

    if (dwInFrequency == dwOutFrequency) {
        *ppfOutBuffer = pfInBuffer;
        *pdwOutBufferBytes = dwInBufferBytes;
        return S_OK;
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
