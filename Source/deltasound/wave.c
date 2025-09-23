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

#include "wave.h"

HRESULT DELTACALL wave_format_is_valid(LPCWAVEFORMATEX pcfxFormat, BOOL bRigid) {
    if (pcfxFormat == NULL) {
        return E_INVALIDARG;
    }

    if (pcfxFormat->wFormatTag != WAVE_FORMAT_PCM) {
        return E_INVALIDARG;
    }

    if (pcfxFormat->nChannels != 1 && pcfxFormat->nChannels != 2) {
        return E_INVALIDARG;
    }

    if (pcfxFormat->nAvgBytesPerSec == 0) {
        return E_INVALIDARG;
    }

    if (bRigid) {
        if (pcfxFormat->nAvgBytesPerSec
            != pcfxFormat->nSamplesPerSec * pcfxFormat->nChannels * (pcfxFormat->wBitsPerSample / 8)) {
            return E_INVALIDARG;
        }
    }

    if (pcfxFormat->nBlockAlign == 0) {
        return E_INVALIDARG;
    }

    if (pcfxFormat->wBitsPerSample == 0 || (pcfxFormat->wBitsPerSample % 8) != 0) {
        return E_INVALIDARG;
    }

    if (bRigid) {
        if (pcfxFormat->nAvgBytesPerSec
            != pcfxFormat->nSamplesPerSec * pcfxFormat->nBlockAlign) {
            return E_INVALIDARG;
        }
    }

    if (pcfxFormat->nBlockAlign != pcfxFormat->nChannels * (pcfxFormat->wBitsPerSample / 8)) {
        return E_INVALIDARG;
    }

    return S_OK;
}
