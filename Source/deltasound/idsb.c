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

HRESULT DELTACALL idsb_query_interface(idsb* self, REFIID riid, LPVOID* ppvObject);
ULONG DELTACALL idsb_add_ref(idsb* self);
ULONG DELTACALL idsb_release(idsb* self);

HRESULT DELTACALL idsb_get_caps(idsb* self, LPDSBCAPS pCaps);
HRESULT DELTACALL idsb_get_current_position(idsb* self, LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
HRESULT DELTACALL idsb_get_format(idsb* self, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
HRESULT DELTACALL idsb_get_volume(idsb* self, LPLONG plVolume);
HRESULT DELTACALL idsb_get_pan(idsb* self, LPLONG plPan);
HRESULT DELTACALL idsb_get_frequency(idsb* self, LPDWORD pdwFrequency);
HRESULT DELTACALL isdb_get_status(idsb* self, LPDWORD pdwStatus);
HRESULT DELTACALL idsb_initialize(idsb* self, LPDIRECTSOUND pDS, LPCDSBUFFERDESC pcDesc);
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

typedef struct idsb_vft {
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
} idsb_vft;

const static idsb_vft idsb_self = {
    idsb_query_interface,
    idsb_add_ref,
    idsb_release,
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

HRESULT DELTACALL idsb_create(idsb* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    self->Self = &idsb_self;

    return S_OK;
}

HRESULT DELTACALL idsb_query_interface(idsb* self, REFIID riid, LPVOID* ppvObject) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

ULONG DELTACALL idsb_add_ref(idsb* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dsb_add_ref((dsb*)self);
}
ULONG DELTACALL idsb_release(idsb* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return dsb_remove_ref((dsb*)self);
}

HRESULT DELTACALL idsb_get_caps(idsb* self, LPDSBCAPS pCaps) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_get_current_position(idsb* self, LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_get_format(idsb* self, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_get_volume(idsb* self, LPLONG plVolume) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_get_pan(idsb* self, LPLONG plPan) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_get_frequency(idsb* self, LPDWORD pdwFrequency) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL isdb_get_status(idsb* self, LPDWORD pdwStatus) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_initialize(idsb* self, LPDIRECTSOUND pDS, LPCDSBUFFERDESC pcDesc) {
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

    return dsb_initialize((dsb*)self, (deltasound*)pDS, pcDesc);
}

HRESULT DELTACALL idsb_lock(idsb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1,
    LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_play(idsb* self, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_set_curent_position(idsb* self, DWORD dwNewPosition) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_set_format(idsb* self, LPCWAVEFORMATEX pcfxFormat) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_set_volume(idsb* self, LONG lVolume) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_set_pan(idsb* self, LONG lPan) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_set_frequency(idsb* self, DWORD dwFrequency) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_stop(idsb* self) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_unlock(idsb* self, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL idsb_restore(idsb* self) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}
