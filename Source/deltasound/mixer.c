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

#define BUFFERFREQUENCY(FREQUENCY, OVERRIDE) (OVERRIDE == DSBFREQUENCY_ORIGINAL ? FREQUENCY : OVERRIDE)

struct mixer {
    allocator*  Allocator;
    arena*      Arena;
};

typedef struct mix_buffer {
    dsb* Instance;
    DWORD   Frequency;

    DWORD   InFrames;
    DWORD   OutFrames;
    FLOAT   Ratio;

    LPVOID  Input;

    FLOAT* WorkData; // TODO better name
    DWORD   WorkDataSize; // TODO is this neeeded?

    FLOAT* Out;
    DWORD   OutSize; // TODO is this neeeded?
} mix_buffer; // TODO better names

HRESULT DELTACALL mixer_convert_to_float(mixer* pMix,
    LPWAVEFORMATEX pwfxFormat, DWORD dwFrequency,
    LPVOID pInBuffer, DWORD dwInBufferBytes, FLOAT** ppOutBuffer, LPDWORD pOutBufferBytes);

HRESULT DELTACALL mixer_convert_to_ieee(mixer* self, mix_buffer* pBuffer);
HRESULT DELTACALL mixer_resample(mixer* pMix, mix_buffer* pBuffer, LPWAVEFORMATEX pwfxFormat);
HRESULT DELTACALL mixer_attenuate(mixer* pMix, mix_buffer* pBuffer, FLOAT fVolume, FLOAT fPan);

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

// TODO
HRESULT DELTACALL mix_buffer_initialize(mix_buffer* self, DWORD dwFrames, DWORD dwRequiredFrequency) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwFrames == 0 || dwRequiredFrequency == 0) {
        return E_INVALIDARG;
    }

    self->Frequency = BUFFERFREQUENCY(self->Instance->Format->nSamplesPerSec, self->Instance->Frequency);

    self->Ratio = (FLOAT)self->Frequency / (FLOAT)dwRequiredFrequency;

    self->InFrames = (DWORD)truncf(self->Ratio * dwFrames);
    self->OutFrames = (DWORD)truncf(1.0f / self->Ratio * self->InFrames);

    return S_OK;
}

HRESULT DELTACALL mixer_mix(mixer* self, DWORD dwBuffers, dsb** ppBuffers,
    PWAVEFORMATEXTENSIBLE pwfxFormat, DWORD dwFrames, LPVOID* pOutBuffer, LPDWORD pdwOutFrames) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxFormat == NULL || dwFrames == 0
        || dwBuffers == 0 || ppBuffers == NULL
        || pOutBuffer == NULL || pdwOutFrames == NULL) {
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

        if (FAILED(hr = mix_buffer_initialize(&buffers[i],
            dwFrames, pwfxFormat->Format.nSamplesPerSec))) {
            return hr;
        }

        const DWORD alignment = buffers[i].Instance->Format->nBlockAlign;
        const DWORD length = buffers[i].InFrames * alignment;

        if (FAILED(hr = arena_allocate(self->Arena, length, &buffers[i].Input))) {
            return hr;
        }

        // TODO need indication if buffer was read to the end for non-looping buffers

        DWORD read = 0;

        if (FAILED(hr = dsbcb_read(ppBuffers[i]->Buffer,
            length, buffers[i].Input, &read,
            (ppBuffers[i]->Status & DSBSTATUS_LOOPING) ? DSBCB_READ_LOOPING : DSBCB_READ_NONE))) {
            return hr;
        }

        const DWORD frames = read / alignment;

        if (frames < buffers[i].InFrames) {
            buffers[i].InFrames = frames;
        }
    }

    // TODO. Assumption is that inputs are non-IEEE PCM.

    // Convert to IEEE, upmix/downmix audio to stereo.
    for (DWORD i = 0; i < dwBuffers; i++) {
        if (FAILED(hr = mixer_convert_to_ieee(self, &buffers[i]))) {
            return hr;
        }
    }

    // At this point all audio data is IEEE and stereo.
    // Resample the audio to the requested audio frequency.
    for (DWORD i = 0; i < dwBuffers; i++) {
        if (FAILED(hr = mixer_resample(self, &buffers[i], &pwfxFormat->Format))) {
            return hr;
        }
    }

    // Apply attenuation (volume and pan modifiers).
    for (DWORD i = 0; i < dwBuffers; i++) {
        if (FAILED(hr = mixer_attenuate(self, &buffers[i], ppBuffers[i]->Volume, ppBuffers[i]->Pan))) {
            return hr;
        }
    }

    // Mix all sound buffers together.
    FLOAT* result = NULL;
    DWORD frames = 0;

    if (FAILED(hr = arena_allocate(self->Arena, dwFrames * sizeof(FLOAT), &result))) {
        return hr;
    }

    for (DWORD i = 0; i < dwFrames; i++) {
        if (frames < buffers[i].OutFrames) {
            frames = buffers[i].OutFrames;
        }

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

                const DWORD advancement =
                    buffers[i].InFrames * buffers[i].Instance->Format->nBlockAlign;

                if (SUCCEEDED(hr = dsbcb_get_current_position(ppBuffers[i]->Buffer, &read, &write))) {
                    hr = dsbcb_set_current_position(ppBuffers[i]->Buffer,
                        read + advancement, write + advancement,
                        DSBCB_SETPOSITION_LOOPING /* TODO proper flags for single play buffers*/);
                }
            }
        }
    }

    // TODO  Trigger appropriate notifications.

    *pOutBuffer = result;
    *pdwOutFrames = frames;

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL mixer_attenuate(mixer* self, mix_buffer* pBuffer, FLOAT fVolume, FLOAT fPan) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pBuffer == NULL) {
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

    for (DWORD i = 0; i < pBuffer->OutFrames; i++) {
        pBuffer->WorkData[i * 2 /* STEREO */ + 0] = pBuffer->WorkData[i * 2 /* STEREO */ + 0] * l;
        pBuffer->WorkData[i * 2 /* STEREO */ + 1] = pBuffer->WorkData[i * 2 /* STEREO */ + 1] * r;
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

HRESULT DELTACALL mixer_convert_to_ieee(mixer* self, mix_buffer* pBuffer) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pBuffer == NULL) {
        return E_INVALIDARG;
    }

    const DWORD bits = pBuffer->Instance->Format->wBitsPerSample;

    if (bits != 8 && bits != 16) {
        return E_NOTIMPL;
    }

    const DWORD channels = pBuffer->Instance->Format->nChannels;

    if (channels != 1 && channels != 2) {
        return E_NOTIMPL;
    }

    HRESULT hr = S_OK;

    const DWORD frames = pBuffer->InFrames;
    const DWORD length = frames * 2 /* STEREO */ * sizeof(FLOAT);

    if (FAILED(hr = arena_allocate(self->Arena, length, &pBuffer->WorkData))) {
        return hr;
    }

    ZeroMemory(pBuffer->WorkData, length);

    const DWORD bytes = bits >> 3;
    DWORD offset = 0;

    for (DWORD i = 0; i < frames; i++) {
        if (channels == 1) {
            const FLOAT v =
                convert_to_float(bits, (LPVOID)((size_t)pBuffer->Input + offset));

            pBuffer->WorkData[i * 2 /* STEREO */ + 0] = v;
            pBuffer->WorkData[i * 2 /* STEREO */ + 1] = v;

            offset += bytes;
        }
        else {
            const FLOAT v1 =
                convert_to_float(bits, (LPVOID)((size_t)pBuffer->Input + offset + 0));
            const FLOAT v2 =
                convert_to_float(bits, (LPVOID)((size_t)pBuffer->Input + offset + bytes));

            pBuffer->WorkData[i * 2 /* STEREO */ + 0] = v1;
            pBuffer->WorkData[i * 2 /* STEREO */ + 1] = v2;

            offset += bytes * channels;
        }
    }

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

HRESULT DELTACALL mixer_resample(mixer* self, mix_buffer* pBuffer, LPWAVEFORMATEX pwfxFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pBuffer == NULL || pwfxFormat == NULL) {
        return E_INVALIDARG;
    }

    if (pBuffer->Frequency == pwfxFormat->nSamplesPerSec) {
        return S_OK;
    }

    HRESULT hr = S_OK;

    if (FAILED(hr = arena_allocate(self->Arena,
        pBuffer->OutFrames * 2 /* STEREO */ * sizeof(FLOAT), &pBuffer->Out))) {
        return hr;
    }

    for (DWORD i = 0; i < pBuffer->OutFrames; i++) {
        for (DWORD j = 0; j < 2 /* STEREO */; j++) {
            // Find the floating-point position
            // in the input buffer for the current output sample.
            const FLOAT t = i * pBuffer->Ratio;

            DWORD index0 = (DWORD)t;
            DWORD index1 = index0 + 1;

            // Handle edge case for the last sample.
            if (index1 >= pBuffer->OutFrames) {
                index1 = pBuffer->OutFrames - 1;
                index0 = index1 - 1;
            }

            // Perform linear interpolation.
            const FLOAT y0 = pBuffer->WorkData[index0 * 2 /* STEREO */ + j];
            const FLOAT y1 = pBuffer->WorkData[index1 * 2 /* STEREO */ + j];

            pBuffer->Out[i * 2 /* STEREO */ + j] = linear_interpolate(y0, y1, t - index0);
        }
    }

    return S_OK;
}
