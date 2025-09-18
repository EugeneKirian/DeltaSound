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

#include "dsb.h"
#include "idsb.h"
#include "wave_format.h"

typedef struct ds ds;

HRESULT DELTACALL idsb_get_caps(idsb* self, LPDSBCAPS pCaps);
HRESULT DELTACALL idsb_get_current_position(idsb* self, LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
HRESULT DELTACALL idsb_get_format(idsb* self, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
HRESULT DELTACALL idsb_get_volume(idsb* self, LPLONG plVolume);
HRESULT DELTACALL idsb_get_pan(idsb* self, LPLONG plPan);
HRESULT DELTACALL idsb_get_frequency(idsb* self, LPDWORD pdwFrequency);
HRESULT DELTACALL isdb_get_status(idsb* self, LPDWORD pdwStatus);
HRESULT DELTACALL idsb_initialize(idsb* self, ds* pDS, LPCDSBUFFERDESC pcDesc);
HRESULT DELTACALL idsb_lock(idsb* self, DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
HRESULT DELTACALL idsb_play(idsb* self, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags);
HRESULT DELTACALL idsb_set_curent_position(idsb* self, DWORD dwNewPosition);
HRESULT DELTACALL idsb_set_format(idsb* self, LPCWAVEFORMATEX pcfxFormat);
HRESULT DELTACALL idsb_set_volume(idsb* self, LONG lVolume);
HRESULT DELTACALL idsb_set_pan(idsb* self, LONG lPan);
HRESULT DELTACALL idsb_set_frequency(idsb* self, DWORD dwFrequency);
HRESULT DELTACALL idsb_stop(idsb* self);
HRESULT DELTACALL idsb_unlock(idsb* self, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
HRESULT DELTACALL idsb_restore(idsb* self);

struct idsb_vft {
    LPIDSBQUERYINTERFACE        QueryInterface;
    LPIDSBADDREF                AddRef;
    LPIDSBRELEASE               Release;
    LPIDSBGETCAPS               GetCaps;
    LPIDSBGETCURRENTPOSITION    GetCurrentPosition;
    LPIDSBGETFORMAT             GetFormat;
    LPIDSBGETVOLUME             GetVolume;
    LPIDSBGETPAN                GetPan;
    LPIDSBGETFREQUENCY          GetFrequency;
    LPIDSBGETSTATUS             GetStatus;
    LPIDSBINITIALIZE            Initialize;
    LPIDSBLOCK                  Lock;
    LPIDSBPLAY                  Play;
    LPIDSBSETCURRENTPOSITION    SetCurrentPosition;
    LPIDSBSETFORMAT             SetFormat;
    LPIDSBSETVOLUME             SetVolume;
    LPIDSBSETPAN                SetPan;
    LPIDSBSETFREQUENCY          SetFrequency;
    LPIDSBSTOP                  Stop;
    LPIDSBUNLOCK                Unlock;
    LPIDSBRESTORE               Restore;
};

const static idsb_vft idsb_self = {
    idsb_query_interface,
    idsb_add_ref,
    idsb_remove_ref,
    idsb_get_caps,
    idsb_get_current_position,
    idsb_get_format,
    idsb_get_volume,
    idsb_get_pan,
    idsb_get_frequency,
    isdb_get_status,
    idsb_initialize,
    idsb_lock,
    idsb_play,
    idsb_set_curent_position,
    idsb_set_format,
    idsb_set_volume,
    idsb_set_pan,
    idsb_set_frequency,
    idsb_stop,
    idsb_unlock,
    idsb_restore
};

HRESULT DELTACALL idsb_allocate(allocator* pAlloc, idsb** ppOut);

HRESULT DELTACALL idsb_create(allocator* pAlloc, REFIID riid, idsb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idsb* instance = NULL;

    if (SUCCEEDED(hr = idsb_allocate(pAlloc, &instance))) {
        instance->Self = &idsb_self;
        CopyMemory(&instance->ID, riid, sizeof(GUID));
        instance->RefCount = 1;

        *ppOut = instance;
    }

    return hr;
}

VOID DELTACALL idsb_release(idsb* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL idsb_query_interface(idsb* self, REFIID riid, LPVOID* ppvObject) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (riid == NULL || ppvObject == NULL) {
        return E_INVALIDARG;
    }

    return dsb_query_interface(self->Instance, riid, ppvObject);
}

ULONG DELTACALL idsb_add_ref(idsb* self) {
    if (self == NULL) {
        return 0;
    }

    return InterlockedIncrement(&self->RefCount);
}

ULONG DELTACALL idsb_remove_ref(idsb* self) {
    if (self == NULL) {
        return 0;
    }

    if (self->RefCount == 0) {
        return 0;
    }

    if (InterlockedDecrement(&self->RefCount) <= 0) {
        self->RefCount = 0;

        if (!(self->Instance->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
            if (self->Instance != NULL) {
                dsb_remove_ref(self->Instance, self);
                idsb_release(self);
            }
        }
    }

    return self->RefCount;
}

HRESULT DELTACALL idsb_get_caps(idsb* self, LPDSBCAPS pCaps) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pCaps == NULL) {
        return E_INVALIDARG;
    }

    if (pCaps->dwSize != sizeof(DSBCAPS)) {
        return E_INVALIDARG;
    }

    return dsb_get_caps(self->Instance, pCaps);
}

HRESULT DELTACALL idsb_get_current_position(idsb* self,
    LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwCurrentPlayCursor == NULL && pdwCurrentWriteCursor == NULL) {
        return E_INVALIDARG;
    }

    return dsb_get_current_position(self->Instance, pdwCurrentPlayCursor, pdwCurrentWriteCursor);
}

HRESULT DELTACALL idsb_get_format(idsb* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pwfxFormat == NULL && pdwSizeWritten == NULL) {
        return E_INVALIDARG;
    }

    return dsb_get_format(self->Instance, pwfxFormat, dwSizeAllocated, pdwSizeWritten);
}

HRESULT DELTACALL idsb_get_volume(idsb* self, LPLONG plVolume) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (plVolume == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    FLOAT volume = DSB_MIN_VOLUME;

    if (SUCCEEDED(hr = dsb_get_volume(self->Instance, &volume))) {
        *plVolume = (LONG)(volume * (DSBVOLUME_MAX - DSBVOLUME_MIN)) + DSBVOLUME_MIN;
    }

    return hr;
}

HRESULT DELTACALL idsb_get_pan(idsb* self, LPLONG plPan) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (plPan == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    FLOAT pan = DSB_CENTER_PAN;

    if (SUCCEEDED(hr = dsb_get_pan(self->Instance, &pan))) {
        *plPan = (LONG)(pan * DSBPAN_RIGHT);
    }

    return hr;
}

HRESULT DELTACALL idsb_get_frequency(idsb* self, LPDWORD pdwFrequency) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwFrequency == NULL) {
        return E_INVALIDARG;
    }

    return dsb_get_frequency(self->Instance, pdwFrequency);
}

HRESULT DELTACALL isdb_get_status(idsb* self, LPDWORD pdwStatus) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pdwStatus == NULL) {
        return E_INVALIDARG;
    }

    return dsb_get_status(self->Instance, pdwStatus);
}

HRESULT DELTACALL idsb_initialize(idsb* self, ds* pDS, LPCDSBUFFERDESC pcDesc) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pDS == NULL || pcDesc == NULL) {
        return E_INVALIDARG;
    }

    if (pcDesc->dwSize != sizeof(dsb_desc_min)
        && pcDesc->dwSize != sizeof(dsb_desc_max)) {
        return E_INVALIDARG;
    }

    return dsb_initialize(self->Instance, pDS, pcDesc);
}

HRESULT DELTACALL idsb_lock(idsb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1,
    LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppvAudioPtr1 == NULL || pdwAudioBytes1 == NULL) {
        if (ppvAudioPtr1 != NULL) {
            *ppvAudioPtr1 = NULL;
        }

        if (pdwAudioBytes1 != NULL) {
            *pdwAudioBytes1 = 0;
        }

        return E_INVALIDARG;
    }

    return dsb_lock(self->Instance, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
}

HRESULT DELTACALL idsb_play(idsb* self, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwReserved1 != 0) {
        return E_INVALIDARG;
    }

    return dsb_play(self->Instance, dwPriority, dwFlags);
}

HRESULT DELTACALL idsb_set_curent_position(idsb* self, DWORD dwNewPosition) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dsb_set_current_position(self->Instance, dwNewPosition);
}

HRESULT DELTACALL idsb_set_format(idsb* self, LPCWAVEFORMATEX pcfxFormat) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pcfxFormat == NULL) {
        return E_INVALIDARG;
    }

    if (pcfxFormat->wFormatTag == WAVE_FORMAT_PCM) {
        HRESULT hr = S_OK;

        if (FAILED(hr = wave_format_is_valid(pcfxFormat, FALSE))) {
            return hr;
        }
    }

    return dsb_set_format(self->Instance, pcfxFormat);
}

HRESULT DELTACALL idsb_set_volume(idsb* self, LONG lVolume) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (lVolume < DSBVOLUME_MIN || lVolume > DSBVOLUME_MAX) {
        return E_INVALIDARG;
    }

    const FLOAT volume = DSB_MAX_VOLUME - (FLOAT)lVolume / (DSBVOLUME_MIN - DSBVOLUME_MAX);

    return dsb_set_volume(self->Instance, volume);
}

HRESULT DELTACALL idsb_set_pan(idsb* self, LONG lPan) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (lPan < DSBPAN_LEFT || lPan > DSBPAN_RIGHT) {
        return E_INVALIDARG;
    }

    return dsb_set_pan(self->Instance, (FLOAT)lPan / (FLOAT)DSBPAN_RIGHT);
}

HRESULT DELTACALL idsb_set_frequency(idsb* self, DWORD dwFrequency) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwFrequency != DSBFREQUENCY_ORIGINAL
        && (dwFrequency < DSBFREQUENCY_MIN || dwFrequency > DSBFREQUENCY_MAX)) {
        return E_INVALIDARG;
    }

    return dsb_set_frequency(self->Instance, dwFrequency);
}

HRESULT DELTACALL idsb_stop(idsb* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dsb_stop(self->Instance);
}

HRESULT DELTACALL idsb_unlock(idsb* self, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dsb_unlock(self->Instance, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
}

HRESULT DELTACALL idsb_restore(idsb* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dsb_restore(self->Instance);
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL idsb_allocate(allocator* pAlloc, idsb** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    idsb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(idsb), &instance))) {
        instance->Allocator = pAlloc;

        *ppOut = instance;
    }

    return hr;
}
