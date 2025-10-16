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

#include "synth.h"

#define _USE_MATH_DEFINES
#include <math.h>

static INT Convert(FLOAT fIn, DWORD dwBits) {
    if (dwBits == 8 || dwBits == 16) {
        // Normalize value to [-1, 1]
        const FLOAT value = min(max(fIn, -1.0f), 1.0f);

        if (dwBits == 8) {
            // Range [0 to 255]
            return (INT)((value + 1.0) * 127.0f);
        }
        else if (dwBits == 16) {
            // Range of [-32,768 to 32,767]
            return (INT)(value * 32767.0f);
        }
    }

    return 0;   // NOT SUPPORTED
}

BOOL Synthesise(LPCWAVEFORMATEX pcwfxFormat,
    FLOAT fFrequency, FLOAT fDuration, LPVOID* ppvAudio, LPDWORD pdwSize) {
    if (pcwfxFormat == NULL || ppvAudio == NULL || pdwSize == NULL) {
        return FALSE;
    }

    if (fFrequency < 0.0f || _isnan(fFrequency) || isinf(fFrequency)
        || fDuration < 0.0f || _isnan(fDuration) || isinf(fDuration)) {
        return FALSE;
    }

    if (pcwfxFormat->wBitsPerSample != 8
        && pcwfxFormat->wBitsPerSample != 16) {
        return FALSE;   // NOT SUPPORTED
    }

    const DWORD samples =
        (DWORD)(fDuration * pcwfxFormat->nChannels * pcwfxFormat->nSamplesPerSec);
    const DWORD length = samples * (pcwfxFormat->wBitsPerSample >> 3);

    LPVOID buffer = malloc(length);

    if (buffer == NULL) {
        return FALSE;
    }

    const FLOAT multiplier = 2.0f * (FLOAT)M_PI * fFrequency;
    const DWORD frames = samples / pcwfxFormat->nChannels;

    for (DWORD i = 0; i < frames; i++) {
        const FLOAT value = (FLOAT)sin(multiplier * ((FLOAT)i / (FLOAT)pcwfxFormat->nSamplesPerSec));
        const DWORD converted = Convert(value, pcwfxFormat->wBitsPerSample);
        const DWORD block_offset = i * pcwfxFormat->nChannels * (pcwfxFormat->wBitsPerSample >> 3);

        for (DWORD j = 0; j < pcwfxFormat->nChannels; j++) {
            const DWORD sample_offset = block_offset + j * (pcwfxFormat->wBitsPerSample >> 3);

            if (pcwfxFormat->wBitsPerSample == 8) {
                *(PBYTE)((SIZE_T)buffer + sample_offset) = (BYTE)converted;
            }
            else if (pcwfxFormat->wBitsPerSample == 16) {
                *(PSHORT)((SIZE_T)buffer + sample_offset) = (SHORT)converted;
            }
        }
    }

    *ppvAudio = buffer;
    *pdwSize = length;

    return TRUE;
}
