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

#include "ds.h"

HRESULT DELTACALL ids_query_interface(ids* self, REFIID riid, LPVOID* ppvObject);
ULONG DELTACALL ids_add_ref(ids* self);
ULONG DELTACALL ids_release(ids* self);

HRESULT DELTACALL ids_create_sound_buffer(ids* self , LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDSBuffer, LPUNKNOWN pUnkOuter);
HRESULT DELTACALL ids_get_caps(ids*, LPDSCAPS pDSCaps);
HRESULT DELTACALL ids_duplicate_sound_buffer(ids*, LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER* ppDSBufferDuplicate);
HRESULT DELTACALL ids_set_cooperative_level(ids*, HWND hwnd, DWORD dwLevel);
HRESULT DELTACALL ids_compact(ids*);
HRESULT DELTACALL ids_get_speaker_config(ids*, LPDWORD pdwSpeakerConfig);
HRESULT DELTACALL ids_set_speaker_config(ids*, DWORD dwSpeakerConfig);
HRESULT DELTACALL ids_initialize(ids*, LPCGUID pcGuidDevice);

typedef struct ids_vft {
    LPIDSQUERYINTERFACE         QueryInterface;
    LPIDSADDREF                 AddRef;
    LPIDSRELEASE                Release;
    LPIDSCREATESOUNDBUFFER      CreateSoundBuffer;
    LPIDSGETCAPS                GetCaps;
    LPIDSDUPLICATESOUNDBUFFER   DuplicateSoundBuffer;
    LPIDSSETCOOPERATIVELEVEL    SetCooperativeLevel;
    LPIDSCOMPACT                Compact;
    LPIDSGETSPEAKERCONFIG       GetSpeakerConfig;
    LPIDSSETSPEAKERCONFIG       SetSpeakerConfig;
    LPIDSINITIALIZE             Initialize;
} ids_vft;

const static ids_vft ids_self = {
    ids_query_interface,
    ids_add_ref,
    ids_release,
    ids_create_sound_buffer,
    ids_get_caps,
    ids_duplicate_sound_buffer,
    ids_set_cooperative_level,
    ids_compact,
    ids_get_speaker_config,
    ids_set_speaker_config,
    ids_initialize
};

HRESULT DELTACALL ids_create(ids* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    self->Self = &ids_self;

    return S_OK;
}

HRESULT DELTACALL ids_query_interface(ids* self, REFIID riid, LPVOID* ppvObject) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

ULONG DELTACALL ids_add_ref(ids* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return ds_add_ref((ds*)self);
}

ULONG DELTACALL ids_release(ids* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    return ds_remove_ref((ds*)self);
}

HRESULT DELTACALL ids_create_sound_buffer(ids* self,
    LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDSBuffer, LPUNKNOWN pUnkOuter) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pcDSBufferDesc == NULL || ppDSBuffer == NULL) {
        return E_INVALIDARG;
    }

    if (pcDSBufferDesc->dwSize != sizeof(dsb_desc_min)
        && pcDSBufferDesc->dwSize != sizeof(dsb_desc_max)) {
        return E_INVALIDARG;
    }

    if (pUnkOuter != NULL) {
        return DSERR_NOAGGREGATION;
    }

    return ds_create_dsb((ds*)self, pcDSBufferDesc, ppDSBuffer);
}

HRESULT DELTACALL ids_get_caps(ids* self, LPDSCAPS pDSCaps) {
    if (self == NULL) {
        return E_POINTER;
    }

    return ds_get_caps((ds*)self, pDSCaps);
}
HRESULT DELTACALL ids_duplicate_sound_buffer(ids* self, LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER* ppDSBufferDuplicate) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_set_cooperative_level(ids* self, HWND hwnd, DWORD dwLevel) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_compact(ids* self) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_get_speaker_config(ids* self, LPDWORD pdwSpeakerConfig) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_set_speaker_config(ids* self, DWORD dwSpeakerConfig) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL ids_initialize(ids* self, LPCGUID pcGuidDevice) {
    if (self == NULL) {
        return E_POINTER;
    }

    return ds_initialize((ds*)self, pcGuidDevice);
}

