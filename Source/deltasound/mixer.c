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

struct mixer {
    allocator*  Allocator;

    arena*      Arena;
};

HRESULT DELTACALL mixer_allocate(allocator* pAlloc, mixer** ppOut);

HRESULT DELTACALL mixer_convert_to_float(mixer* pMix,
    LPWAVEFORMATEX pwfxFormat, DWORD dwIndex, LPVOID pInput, DWORD dwBytes);

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
    DWORD dwFrames, dsb* pMain, arr* pSecondary, LPVOID* pOutBuffer, LPDWORD ppdwFrames) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxFormat == NULL || dwFrames == 0
        || pMain == NULL || pOutBuffer == NULL || ppdwFrames == NULL) {
        return E_INVALIDARG;
    }

    arena_clear(self->Arena);

    const BOOL main = pMain->Instance->Level == DSSCL_WRITEPRIMARY
        && (pMain->Status & DSBSTATUS_PLAYING);

    HRESULT hr = S_OK;
    if (!main) {
        // TODO nothing to play for now
        // TODO support secondary buffers
        goto exit;
    }



    if (main) {

    }



    // convert to floats
    // upmix/downmix
    // apply volume and pan
    // resample
    // convert back to intended format


    return hr;

exit:
    if (pOutBuffer != NULL) {
        *pOutBuffer = NULL;
    }

    if (ppdwFrames != NULL) {
        *ppdwFrames = 0;
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
