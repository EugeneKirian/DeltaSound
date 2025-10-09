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
#include "convert.h"

#include <math.h>

#define MONO                1
#define STEREO              2

struct converter {
    allocator*  Allocator;
    arena*      Arena;
};

static INT DELTACALL convert_from_float(FLOAT fValue, DWORD dwBits);
HRESULT DELTACALL converter_resample(converter* self,
    DWORD dwInFrames, DWORD dwChannels, FLOAT fRatio, FLOAT* pInBuffer, FLOAT** ppOutBuffer);

HRESULT DELTACALL converter_create(allocator* pAlloc, converter** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    converter* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(converter), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = arena_create(pAlloc, &instance->Arena))) {

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL converter_release(converter* self) {
    if (self == NULL) { return; }

    arena_release(self->Arena);

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL converter_convert(converter* self,
    PWAVEFORMATEXTENSIBLE pwfxInFormat, LPVOID pInBuffer, DWORD dwFrames,
    LPWAVEFORMATEX pwfxOutFormat, LPVOID* pOutBuffer, LPDWORD pdwBytes, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxInFormat == NULL || pInBuffer == NULL || dwFrames == 0
        || pwfxOutFormat == NULL || pOutBuffer == NULL || pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    const DWORD inChannels = pwfxInFormat->Format.nChannels;

    if (inChannels != MONO && inChannels != STEREO) {
        // TODO NOT INPLEMENTED

        return E_NOTIMPL;
    }

    const DWORD outChannels = pwfxOutFormat->nChannels;

    if (outChannels != MONO && outChannels != STEREO) {
        // TODO NOT INPLEMENTED

        return E_NOTIMPL;
    }

    // TODO. Assumption is that inputs are IEEE PCM.

    if (pwfxInFormat->Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE
        && IsEqualGUID(&pwfxInFormat->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
        // TODO NOT INPLEMENTED

        return E_NOTIMPL;
    }

    // TODO: dwFlags
    // Support Silence flag

    HRESULT hr = S_OK;

    if (FAILED(hr = arena_clear(self->Arena))) {
        return hr;
    }

    // TODO frame calculation in one place
    // see calculations in converter_resample
    const FLOAT ratio =
        (FLOAT)pwfxOutFormat->nSamplesPerSec / (FLOAT)pwfxInFormat->Format.nSamplesPerSec;
    const DWORD frames = (DWORD)(dwFrames * ratio);

    FLOAT* resampled = NULL; // TODO name

    if (FAILED(hr = converter_resample(self, dwFrames,
        pwfxInFormat->Format.nChannels, ratio, pInBuffer, &resampled))) {
        return hr;
    }

    // Convert from IEEE to 8/16-bit PCM

    LPVOID buffer = NULL;
    const DWORD bytes =
        frames * pwfxOutFormat->nChannels * (pwfxOutFormat->wBitsPerSample >> 3);
    const DWORD outBits = pwfxOutFormat->wBitsPerSample;

    if (FAILED(hr = arena_allocate(self->Arena, bytes, &buffer))) {
        return hr;
    }

    DWORD offset = 0;

    for (DWORD i = 0; i < frames; i++) {
        if (outChannels == MONO) {
            const INT v = convert_from_float(resampled[i * inChannels], outBits);

            if (outBits == 8) {
                ((BYTE*)buffer)[i] = (BYTE)v;
            }
            else if (outBits == 16) {
                ((SHORT*)buffer)[i] = (SHORT)v;
            }
        }
        else {
            const INT v1 = convert_from_float(resampled[i * inChannels + 0], outBits);
            const INT v2 = convert_from_float(resampled[i * inChannels + 1], outBits);

            if (outBits == 8) {
                ((BYTE*)buffer)[i * outChannels + 0] = (BYTE)v1;
                ((BYTE*)buffer)[i * outChannels + 1] = (BYTE)v1;
            }
            else if (outBits == 16) {
                ((SHORT*)buffer)[i * outChannels + 0] = (SHORT)v1;
                ((SHORT*)buffer)[i * outChannels + 1] = (SHORT)v1;
            }
        }
    }

    *pOutBuffer = buffer;
    *pdwBytes = bytes;

    return S_OK;
}

/* ---------------------------------------------------------------------- */

INT DELTACALL convert_from_float(FLOAT fValue, DWORD dwBits) {
    if (dwBits == 8) {
        return (INT)((fValue * 128.0f) + 128.0f);
    }
    else if (dwBits == 16) {
        return (INT)(fValue * 32768.0f);
    }

    return 0;
}

#include <stdio.h>

// TODO
// Combine with mixer_resample
// TODO
// Better downsampling methods
HRESULT DELTACALL converter_resample(converter* self,
    DWORD dwInFrames, DWORD dwChannels, FLOAT fRatio, FLOAT* pInBuffer, FLOAT** ppOutBuffer) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwInFrames == 0 || pInBuffer == NULL || ppOutBuffer == NULL) {
        return E_INVALIDARG;
    }

    if (fRatio == 0.0f || _isnan(fRatio) || isinf(fRatio)) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    // TODO frame calculation in one place
    const DWORD frames = (DWORD)roundf(dwInFrames * fRatio);
    const DWORD size = frames * dwChannels * sizeof(FLOAT);

    FLOAT* buffer = NULL;

    if (FAILED(hr = arena_allocate(self->Arena, size, &buffer))) {
        return hr;
    }

    // Downsampling by decimation.
    for (DWORD i = 0; i < frames; i++) {
        for (DWORD j = 0; j < dwChannels; j++) {
            DWORD t = (DWORD)(i / fRatio);

            if (dwInFrames < t) {
                t = dwInFrames;
            }

            buffer[i * dwChannels + j] = pInBuffer[t * dwChannels + j];
        }
    }

    *ppOutBuffer = buffer;

    return S_OK;
}
