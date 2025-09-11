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
#include <stdint.h>

static int32_t Convert(double in, uint32_t bits) {
    if (bits == 8 || bits == 16) {
        // Normalize value to [-1, 1]
        const double value = min(max(in, -1.0), 1.0);

        if (bits == 8) {
            // Range [0 to 255]
            return (int32_t)((value + 1.0) * 127.0);
        }
        else if (bits == 16) {
            // Range of [-32,768 to 32,767]
            return (int32_t)(value * 32767.0);
        }
    }

    return 0;   // NOT SUPPORTED
}

BOOL Synthesise(LPWAVEFORMATEX format,
    float frequency, float duration, void** wave, unsigned* size) {
    if (format == NULL
        || frequency < 0.0f || duration < 0.0f
        || wave == NULL || size == NULL) {
        return FALSE;
    }

    if (format->wBitsPerSample != 8 && format->wBitsPerSample != 16) {
        return FALSE;   // NOT SUPPORTED
    }

    const uint32_t samples = (uint32_t)(duration * format->nChannels * format->nSamplesPerSec);
    const uint32_t length = samples * (format->wBitsPerSample >> 3);

    void* data = malloc(length);

    if (data == NULL) {
        return FALSE;
    }

    const double multiplier = 2.0 * M_PI * frequency;
    const uint32_t frames = samples / format->nChannels;

    for (uint32_t i = 0; i < frames; i++) {
        const double value = sin(multiplier * ((double)(i) / format->nSamplesPerSec));
        const uint32_t converted = Convert(value, format->wBitsPerSample);
        const size_t block_offset = i * format->nChannels * (format->wBitsPerSample >> 3);

        for (uint32_t j = 0; j < format->nChannels; j++) {
            const size_t sample_offset = block_offset + j * (format->wBitsPerSample >> 3);

            if (format->wBitsPerSample == 8) {
                *(uint8_t*)((size_t)data + sample_offset) = (uint8_t)converted;
            }
            else if (format->wBitsPerSample == 16) {
                *(int16_t*)((size_t)data + sample_offset) = (int16_t)converted;
            }
        }
    }

    *wave = data;
    *size = length;

    return TRUE;
}
