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
#include "dsb.h"
#include "ids.h"
#include "ksp.h"
#include "wave_format.h"

#define DSB_PLAY_WRITE_CURSOR_FRAME_COUNT   800

HRESULT DELTACALL dsb_allocate(allocator* pAlloc, dsb** ppOut);

HRESULT DELTACALL dsb_create(allocator* pAlloc, REFIID riid, dsb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = dsb_allocate(pAlloc, &instance))) {
        CopyMemory(&instance->ID, riid, sizeof(GUID));

        if (SUCCEEDED(hr = dsbcb_create(pAlloc, 0, &instance->Buffer))) {
            if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
                instance->Caps.dwSize = sizeof(DSBCAPS);
                *ppOut = instance;
                return S_OK;
            }
        }

        dsb_release(instance);
    }

    return hr;
}

VOID DELTACALL dsb_release(dsb* self) {
    if (self == NULL) { return; }

    for (DWORD i = 0; i < intfc_get_count(self->Interfaces); i++) {
        idsb* instance = NULL;
        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idsb_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    if (self->Instance != NULL) {
        ds_remove_dsb(self->Instance, self);
    }

    if (self->PropertySet != NULL) {
        ksp_release(self->PropertySet);
    }

    dsbcb_release(self->Buffer);

    allocator_free(self->Allocator, self->Format);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsb_set_flags(dsb* self, DWORD dwFlags) { // TODO Is this needed?
    self->Caps.dwFlags = dwFlags;

    // TODO properly update things when new flags are set

    return S_OK;
}

HRESULT DELTACALL dsb_query_interface(dsb* self, REFIID riid, LPVOID* ppOut) {
    // TODO synchronization

    {
        idsb* instance = NULL;

        if (SUCCEEDED(intfc_query_item(self->Interfaces, riid, &instance))) {
            idsb_add_ref(instance);
            *ppOut = instance;
            return S_OK;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSoundBuffer, riid)
        || (IsEqualIID(&IID_IDirectSoundBuffer8, &self->ID) && IsEqualIID(&IID_IDirectSoundBuffer8, riid))) {
        HRESULT hr = S_OK;
        idsb* instance = NULL;

        if (SUCCEEDED(hr = idsb_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dsb_add_ref(self, instance))) {
                instance->Instance = self;
                *ppOut = instance;
                return S_OK;
            }

            idsb_release(instance);
        }

        return hr;
    }
    else if (IsEqualIID(&IID_IKsPropertySet, riid)) {
        if (self->PropertySet == NULL) {
            HRESULT hr = S_OK;
            ksp* instance = NULL;

            if (FAILED(hr = ksp_create(self->Allocator, riid, &instance))) {
                return hr;
            }

            instance->Instance = self;
            self->PropertySet = instance;
        }

        return ksp_query_interface(self->PropertySet, riid, (iksp**)ppOut);
    }

    return E_NOINTERFACE;
}

HRESULT DELTACALL dsb_add_ref(dsb* self, idsb* pIDSB) {
    return intfc_add_item(self->Interfaces, &pIDSB->ID, pIDSB);
}

HRESULT DELTACALL dsb_remove_ref(dsb* self, idsb* pIDSB) {
    intfc_remove_item(self->Interfaces, &pIDSB->ID);

    if (intfc_get_count(self->Interfaces) == 0) {
        if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
            dsb_release(self);
        }
    }

    return S_OK;
}

HRESULT DELTACALL dsb_get_caps(dsb* self, LPDSBCAPS pCaps) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    CopyMemory(pCaps, &self->Caps, sizeof(DSBCAPS));

    return S_OK;
}

HRESULT DELTACALL dsb_get_current_position(dsb* self,
    LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        return DSERR_BUFFERLOST;
    }

    HRESULT hr = S_OK;

    if (pdwCurrentPlayCursor != NULL) {
        if (FAILED(hr = dsbcb_get_read_position(self->Buffer, pdwCurrentPlayCursor))) {
            return hr;
        }
    }

    if (pdwCurrentWriteCursor != NULL) {
        if (FAILED(hr = dsbcb_get_write_position(self->Buffer, pdwCurrentWriteCursor))) {
            return hr;
        }
    }

    return hr;
}

HRESULT DELTACALL dsb_get_format(dsb* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;
    const DWORD size = sizeof(WAVEFORMATEX) + self->Format->cbSize;

    if (pwfxFormat != NULL) {
        if (size <= dwSizeAllocated) {
            CopyMemory(pwfxFormat, self->Format, min(size, dwSizeAllocated));
        }
        else {
            hr = E_INVALIDARG;
        }
    }

    if (pdwSizeWritten != NULL) {
        *pdwSizeWritten = size;
    }

    return hr;
}

HRESULT DELTACALL dsb_get_volume(dsb* self, PFLOAT pfVolume) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level == DSSCL_NONE) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLVOLUME)) {
        return DSERR_CONTROLUNAVAIL;
    }

    *pfVolume = self->Volume;

    return S_OK;
}

HRESULT DELTACALL dsb_get_pan(dsb* self, PFLOAT pfPan) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level == DSSCL_NONE) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLPAN)) {
        return DSERR_CONTROLUNAVAIL;
    }

    *pfPan = self->Pan;

    return S_OK;
}

HRESULT DELTACALL dsb_get_frequency(dsb* self, LPDWORD pdwFrequency) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level == DSSCL_NONE) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLFREQUENCY)) {
        return DSERR_CONTROLUNAVAIL;
    }

    *pdwFrequency = self->Frequency == DSBFREQUENCY_ORIGINAL
        ? self->Format->nSamplesPerSec : self->Frequency;

    return S_OK;
}

HRESULT DELTACALL dsb_get_status(dsb* self, LPDWORD pdwStatus) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    DWORD status = self->Status;

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level == DSSCL_NONE) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }
    else {
        if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
            status |= DSBSTATUS_BUFFERLOST;
        }
    }

    *pdwStatus = status;

    return S_OK;
}

HRESULT DELTACALL dsb_initialize(dsb* self, ds* pDS, LPCDSBUFFERDESC pcDesc) {
    if (self->Instance != NULL) {
        return DSERR_ALREADYINITIALIZED;
    }

    self->Instance = pDS;

    self->Caps.dwFlags = pcDesc->dwFlags;

    if (!(self->Caps.dwFlags & (DSBCAPS_LOCSOFTWARE | DSBCAPS_LOCHARDWARE))) {
        self->Caps.dwFlags |= DSBCAPS_LOCSOFTWARE;
    }

    self->Pan = DSB_CENTER_PAN;
    self->Volume = DSB_MAX_VOLUME;

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        self->Caps.dwBufferBytes = DSB_DEFAULT_PRIMARY_BUFFER_SIZE;

        self->Format->wFormatTag = WAVE_FORMAT_PCM;
        self->Format->nChannels = 2;
        self->Format->nSamplesPerSec = 22050;
        self->Format->nAvgBytesPerSec = 44100;
        self->Format->nBlockAlign = 2;
        self->Format->wBitsPerSample = 8;
        self->Format->cbSize = 0;
    }
    else {
        // TODO, what to do with cbSize extra bytes ?
        // Need tests
        self->Caps.dwBufferBytes = pcDesc->dwBufferBytes;
        CopyMemory(self->Format, pcDesc->lpwfxFormat, sizeof(WAVEFORMATEX));
    }

    return dsbcb_resize(self->Buffer, self->Caps.dwBufferBytes);
}

HRESULT DELTACALL dsb_lock(dsb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1,
    LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    HRESULT hr = S_OK;

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            hr = DSERR_PRIOLEVELNEEDED;
            goto fail;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        hr = DSERR_BUFFERLOST;
        goto fail;
    }

    if (dwFlags & DSBLOCK_FROMWRITECURSOR) {
        if (FAILED(hr = dsb_get_current_position(self, NULL, &dwOffset))) {
            goto fail;
        }
    }

    DWORD lockable = 0;
    if (FAILED(hr = dsbcb_get_lockable_size(self->Buffer, &lockable))) {
        goto fail;
    }

    if (dwFlags & DSBLOCK_ENTIREBUFFER) {
        dwBytes = lockable;
    }

    // TODO what if offset in between read and write cursors
    // + wrapped test

    if (dwBytes == 0
        || self->Caps.dwBufferBytes < dwOffset || lockable < dwBytes) {
        hr = E_INVALIDARG;
        goto fail;
    }

    if (SUCCEEDED(hr = dsbcb_lock(self->Buffer, dwOffset, dwBytes,
        ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2))) {
        return hr;
    }

fail:
    if (ppvAudioPtr1 != NULL) {
        *ppvAudioPtr1 = NULL;
    }

    if (pdwAudioBytes1 != NULL) {
        *pdwAudioBytes1 = 0;
    }

    if (ppvAudioPtr2 != NULL) {
        *ppvAudioPtr2 = NULL;
    }

    if (pdwAudioBytes2 != NULL) {
        *pdwAudioBytes2 = 0;
    }

    return hr;
}

HRESULT DELTACALL dsb_play(dsb* self, DWORD dwPriority, DWORD dwFlags) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        return DSERR_BUFFERLOST;
    }

    if ((dwFlags & DSBPLAY_TERMINATEBY_TIME)
        && (dwFlags & DSBPLAY_TERMINATEBY_PRIORITY)) {
        return E_INVALIDARG;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (dwPriority != 0) {
            return E_INVALIDARG;
        }

        if (!(dwFlags & DSBPLAY_LOOPING)) {
            return E_INVALIDARG;
        }

        if (dwFlags & (DSBPLAY_LOCSOFTWARE | DSBPLAY_LOCHARDWARE)) {
            return E_INVALIDARG;
        }

        if (dwFlags & (DSBPLAY_TERMINATEBY_TIME
            | DSBPLAY_TERMINATEBY_PRIORITY | DSBPLAY_TERMINATEBY_DISTANCE)) {
            return E_INVALIDARG;
        }

        // TODO iterate through all secondary buffers,
        // stop them, and mark as lost.
    }
    else {
        if (!(self->Caps.dwFlags & (DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE))
            && (dwFlags & DSBPLAY_TERMINATEBY_DISTANCE)) {
            return E_INVALIDARG;
        }

        if (!(self->Caps.dwFlags & DSBCAPS_LOCDEFER)) {
            if (dwPriority != 0) {
                return E_INVALIDARG;
            }

            if (dwFlags & (DSBPLAY_LOCSOFTWARE | DSBPLAY_LOCHARDWARE)) {
                return E_INVALIDARG;
            }

            if (dwFlags & (DSBPLAY_TERMINATEBY_TIME | DSBPLAY_TERMINATEBY_PRIORITY)) {
                return E_INVALIDARG;
            }
        }
    }

    self->Priority = dwPriority;

    self->Status = self->Status | DSBSTATUS_PLAYING;

    if (dwFlags & DSBPLAY_LOOPING) {
        self->Status = self->Status | DSBSTATUS_LOOPING;
    }

    if (self->Caps.dwFlags & DSBCAPS_LOCDEFER) {
        self->Status = self->Status | DSBSTATUS_LOCSOFTWARE;
    }

    // TODO store play flags for future use

    HRESULT hr = S_OK;
    DWORD position = 0;

    if (SUCCEEDED(hr = dsbcb_get_write_position(self->Buffer, &position))) {
        hr = dsbcb_set_write_position(self->Buffer,
            position + DSB_PLAY_WRITE_CURSOR_FRAME_COUNT * self->Format->nBlockAlign,
            DSBCB_SETPOSITION_WRAP);
    }

    return hr;
}

HRESULT DELTACALL dsb_set_current_position(dsb* self, DWORD dwNewPosition) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        return DSERR_INVALIDCALL;
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        return DSERR_BUFFERLOST;
    }

    if (self->Caps.dwBufferBytes < dwNewPosition) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if (FAILED(hr = dsbcb_set_read_position(self->Buffer, dwNewPosition, DSBCB_SETPOSITION_NONE))) {
        return hr;
    }

    if (FAILED(hr = dsbcb_set_write_position(self->Buffer, dwNewPosition , DSBCB_SETPOSITION_NONE))) {
        return hr;
    }

    return S_OK;
}

HRESULT DELTACALL dsb_set_format(dsb* self, LPCWAVEFORMATEX pcfxFormat) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
        return DSERR_INVALIDCALL;
    }

    if (self->Instance->Level == DSSCL_NORMAL) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (self->Instance->Level == DSSCL_NORMAL) {
        if (pcfxFormat->wFormatTag != WAVE_INVALIDFORMAT
            && pcfxFormat->wFormatTag != WAVE_FORMAT_PCM) {
            return E_NOTIMPL;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        if (pcfxFormat->wFormatTag == WAVE_INVALIDFORMAT) {
            return E_NOTIMPL;
        }
    }

    if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        if (self->Status & DSBSTATUS_PLAYING) {
            // TODO buffer must be stopped by the user
        }

        // TODO update format...
    }
    else {
        // TODO stop the buffer, update the format, and resume playback
    }

    CopyMemory(self->Format, pcfxFormat, sizeof(WAVEFORMATEX)); // TODO see note below
    self->Format->cbSize = 0;

    return S_OK;

    // TODO
    // WAVEFORMATEXTENSIBLE can safely be cast to WAVEFORMATEX,
    // because it simply configures the extra bytes specified by WAVEFORMATEX.cbSize.
    // DirectSound recognizes the WAVE_FORMAT_EXTENSIBLE format tag
    // and correctly plays multiple-channel and compressed formats in hardware buffers, as long as these formats are supported by the driver.

    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee419020(v=vs.85)
}

HRESULT DELTACALL dsb_set_volume(dsb* self, FLOAT fVolume) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLVOLUME)) {
        return DSERR_CONTROLUNAVAIL;
    }

    self->Volume = fVolume;

    return S_OK;
}

HRESULT DELTACALL dsb_set_pan(dsb* self, FLOAT fPan) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLPAN)) {
        return DSERR_CONTROLUNAVAIL;
    }

    self->Pan = fPan;

    return S_OK;
}

HRESULT DELTACALL dsb_set_frequency(dsb* self, DWORD dwFrequency) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLFREQUENCY)) {
        return DSERR_CONTROLUNAVAIL;
    }

    self->Frequency = dwFrequency;

    return S_OK;
}

HRESULT DELTACALL dsb_stop(dsb* self) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }

    self->Status &= ~(DSBSTATUS_LOOPING | DSBSTATUS_PLAYING);

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
            dsbcb_set_read_position(self->Buffer, 0, DSBCB_SETPOSITION_NONE);
            dsbcb_set_write_position(self->Buffer, 0, DSBCB_SETPOSITION_NONE);
        }
    }

    return S_OK;
}

HRESULT DELTACALL dsb_unlock(dsb* self,
    LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        return DSERR_BUFFERLOST;
    }

    if (pvAudioPtr1 == NULL && pvAudioPtr2 == NULL) {
        return S_OK;
    }

    if (pvAudioPtr1 == NULL) {
        return E_INVALIDARG;
    }

    if (pvAudioPtr2 == NULL && dwAudioBytes2 != 0) {
        return E_INVALIDARG;
    }

    return dsbcb_unlock(self->Buffer, pvAudioPtr1, pvAudioPtr2);
}

HRESULT DELTACALL dsb_restore(dsb* self) {
    if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
        if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
            return DSERR_BUFFERLOST;
        }
    }

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsb_allocate(allocator* pAlloc, dsb** ppOut) {
    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsb), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(WAVEFORMATEX), &instance->Format))) {
            *ppOut = instance;
            return S_OK;
        }

        allocator_free(pAlloc, instance->Format);
        allocator_free(pAlloc, instance);
    }

    return hr;
}
