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
#include "dsn.h"
#include "dssb.h"
#include "dssl.h"
#include "ids.h"
#include "ksp.h"
#include "wave.h"

#define DSB_PLAY_WRITE_CURSOR_FRAME_COUNT   800

#define ADVANCEWRITEPOSITION(X, ALIGN) (X + DSB_PLAY_WRITE_CURSOR_FRAME_COUNT * ALIGN)

HRESULT DELTACALL dsb_trigger_notifications(dsb* pDSB, DWORD dwPosition, DWORD dwAdvance);

HRESULT DELTACALL dsb_create(allocator* pAlloc, REFIID riid, dsb** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(dsb), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, riid, sizeof(GUID));

        if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(WAVEFORMATEXTENSIBLE), &instance->Format))) {
            if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
                InitializeCriticalSection(&instance->Lock);

                instance->Caps.dwSize = sizeof(DSBCAPS);

                *ppOut = instance;

                return S_OK;
            }

            allocator_free(pAlloc, instance->Format);
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL dsb_release(dsb* self) {
    if (self == NULL) { return; }

    self->Play = DSBPLAY_NONE;
    self->Status = DSBSTATUS_NONE;

    DeleteCriticalSection(&self->Lock);

    const DWORD count = intfc_get_count(self->Interfaces);

    for (DWORD i = 0; i < count; i++) {
        idsb* instance = NULL;

        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            idsb_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    if (self->Instance != NULL) {
        ds_remove_sound_buffer(self->Instance, self);
    }

    if (self->PropertySet != NULL) {
        ksp_release(self->PropertySet);
    }

    if (self->Buffer != NULL) {
        dsbcb_release(self->Buffer);
    }

    allocator_free(self->Allocator, self->Format);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL dsb_duplicate(dsb* self, dsb** ppOut) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        return DSERR_INVALIDCALL;
    }

    HRESULT hr = S_OK;
    dsb* instance = NULL;

    EnterCriticalSection(&self->Lock);

    if (SUCCEEDED(hr = allocator_allocate(self->Allocator, sizeof(dsb), &instance))) {
        instance->Allocator = self->Allocator;

        CopyMemory(&instance->ID, &self->ID, sizeof(GUID));

        if (SUCCEEDED(hr = allocator_allocate(self->Allocator, SIZEOFFORMATEX(self->Format), &instance->Format))) {
            if (SUCCEEDED(hr = intfc_create(self->Allocator, &instance->Interfaces))) {
                if (SUCCEEDED(hr = dsbcb_duplicate(self->Buffer, &instance->Buffer))) {
                    InitializeCriticalSection(&instance->Lock);

                    instance->Instance = self->Instance;

                    // TODO PropertySet ?
                    // TODO SpatialBuffer ?

                    CopyMemory(&instance->Caps, &self->Caps, sizeof(DSBCAPS));
                    CopyMemory(instance->Format, self->Format, SIZEOFFORMATEX(self->Format));

                    instance->Volume = self->Volume;
                    instance->Pan = self->Pan;
                    instance->Frequency = self->Frequency;
                    instance->Priority = self->Priority;

                    CopyMemory(&instance->SpatialAlgorithm, &self->SpatialAlgorithm, sizeof(GUID));

                    instance->Play = DSBPLAY_NONE;
                    instance->Status = DSBSTATUS_NONE;

                    if (SUCCEEDED(hr = arr_add_item(self->Instance->Buffers, instance))) {

                        *ppOut = instance;

                        goto exit;
                    }

                    dsbcb_release(instance->Buffer);
                }

                intfc_release(instance->Interfaces);
            }

            allocator_free(self->Allocator, instance->Format);
        }

        allocator_free(self->Allocator, instance);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL dsb_query_interface(dsb* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    {
        idsb* instance = NULL;

        if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
            idsb_add_ref(instance);

            *ppOut = instance;

            goto exit;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IDirectSoundBuffer, riid)
        // TODO CLSID
        || (IsEqualIID(&IID_IDirectSoundBuffer8, &self->ID) && IsEqualIID(&IID_IDirectSoundBuffer8, riid))) {
        idsb* instance = NULL;

        if (SUCCEEDED(hr = idsb_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = dsb_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            idsb_release(instance);
        }
    }
    else if (IsEqualIID(&IID_IDirectSound3DListener, riid)) {
        if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
            if (self->Caps.dwFlags & DSBCAPS_CTRL3D) {
                if (self->SpatialListener == NULL) {
                    dssl* instance = NULL;

                    if (FAILED(hr = dssl_create(self->Allocator, riid, &instance))) {
                        goto exit;
                    }

                    instance->Instance = self;
                    self->SpatialListener = instance;
                }

                hr = dssl_query_interface(self->SpatialListener, riid, ppOut);
            }
        }
    }
    else if (IsEqualIID(&IID_IDirectSound3DBuffer, riid)) {
        if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
            if (self->Caps.dwFlags & DSBCAPS_CTRL3D) {
                if (self->SpatialBuffer == NULL) {
                    dssb* instance = NULL;

                    if (FAILED(hr = dssb_create(self->Allocator, riid, &instance))) {
                        goto exit;
                    }

                    instance->Instance = self;
                    self->SpatialBuffer = instance;
                }

                hr = dssb_query_interface(self->SpatialBuffer, riid, ppOut);
            }
        }
    }
    else if (IsEqualIID(&IID_IDirectSoundNotify, riid)) {
        if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
            if (self->Caps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY) {
                if (self->Notifications == NULL) {
                    dsn* instance = NULL;

                    if (FAILED(hr = dsn_create(self->Allocator, riid, &instance))) {
                        goto exit;
                    }

                    instance->Instance = self;
                    self->Notifications = instance;
                }

                hr = dsn_query_interface(self->Notifications, riid, ppOut);
            }
        }
    }
    else if (IsEqualIID(&IID_IKsPropertySet, riid)) {
        if (self->PropertySet == NULL) {
            ksp* instance = NULL;

            if (FAILED(hr = ksp_create(self->Allocator, riid, &instance))) {
                goto exit;
            }

            instance->Instance = self;
            self->PropertySet = instance;
        }

        hr = ksp_query_interface(self->PropertySet, riid, ppOut);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
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
        self->Play = DSBPLAY_NONE;
        self->Status = DSBSTATUS_BUFFERLOST;

        return DSERR_BUFFERLOST;
    }

    return dsbcb_get_current_position(self->Buffer, pdwCurrentPlayCursor, pdwCurrentWriteCursor);
}

HRESULT DELTACALL dsb_get_format(dsb* self,
    LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;
    const DWORD size = SIZEOFFORMATEX(self->Format);

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

    if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
        if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
            self->Play = DSBPLAY_NONE;
            self->Status = DSBSTATUS_BUFFERLOST;
        }
    }

    *pdwStatus = self->Status;

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
        self->Caps.dwBufferBytes = pcDesc->dwBufferBytes;
        CopyMemory(self->Format, pcDesc->lpwfxFormat, SIZEOFFORMAT(pcDesc->lpwfxFormat));
    }

    if (pcDesc->dwSize == sizeof(DSBUFFERDESC)) {
        CopyMemory(&self->SpatialAlgorithm, &pcDesc->guid3DAlgorithm, sizeof(GUID));
    }

    return dsbcb_create(self->Allocator, self->Caps.dwBufferBytes, &self->Buffer);
}

HRESULT DELTACALL dsb_lock(dsb* self, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1,
    LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            hr = DSERR_PRIOLEVELNEEDED;
            goto fail;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        self->Play = DSBPLAY_NONE;
        self->Status = DSBSTATUS_BUFFERLOST;

        hr = DSERR_BUFFERLOST;
        
        goto fail;
    }

    if (self->Status & DSBSTATUS_BUFFERLOST) {
        hr = DSERR_BUFFERLOST;

        goto fail;
    }

    if (dwFlags & DSBLOCK_FROMWRITECURSOR) {
        if (FAILED(hr = dsb_get_current_position(self, NULL, &dwOffset))) {
            goto fail;
        }
    }

    DWORD lockable = 0;
    if (FAILED(hr = dsbcb_get_lockable_length(self->Buffer, &lockable))) {
        goto fail;
    }

    if (dwFlags & DSBLOCK_ENTIREBUFFER) {
        dwBytes = lockable;
    }

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

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        self->Play = DSBPLAY_NONE;
        self->Status = DSBSTATUS_BUFFERLOST;

        return DSERR_BUFFERLOST;
    }

    if (self->Status & DSBSTATUS_BUFFERLOST) {
        return DSERR_BUFFERLOST;
    }

    if ((dwFlags & DSBPLAY_TERMINATEBY_TIME)
        && (dwFlags & DSBPLAY_TERMINATEBY_PRIORITY)) {
        return E_INVALIDARG;
    }

    if (self->Instance->Level == DSSCL_NONE) {
        return DSERR_PRIOLEVELNEEDED;
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

        const DWORD count = arr_get_count(self->Instance->Buffers);

        for (DWORD i = 0; i < count; i++) {
            dsb* instance = NULL;

            if (SUCCEEDED(arr_get_item(self->Instance->Buffers, i, &instance))) {
                instance->Play = DSBPLAY_NONE;
                instance->Status = DSBSTATUS_BUFFERLOST;
            }
        }
    }
    else {
        if (!(self->Caps.dwFlags & DSBCAPS_LOCDEFER)) {
            if (dwPriority != 0) {
                return E_INVALIDARG;
            }

            if (dwFlags & (DSBPLAY_LOCSOFTWARE | DSBPLAY_LOCHARDWARE)) {
                return E_INVALIDARG;
            }

            if (dwFlags & (DSBPLAY_TERMINATEBY_TIME
                | DSBPLAY_TERMINATEBY_PRIORITY | DSBPLAY_TERMINATEBY_DISTANCE)) {
                return E_INVALIDARG;
            }
        }
    }

    HRESULT hr = S_OK;
    DWORD read = 0, write = 0;

    if (SUCCEEDED(hr = dsbcb_get_current_position(self->Buffer, &read, &write))) {
        const DWORD advance = min(self->Caps.dwBufferBytes,
            ADVANCEWRITEPOSITION(write, self->Format->nBlockAlign));

        if (SUCCEEDED(hr = dsbcb_set_current_position(self->Buffer, read, advance, DSBCB_SETPOSITION_NONE))) {

            self->Play = dwFlags;
            self->Priority = dwPriority;

            self->Status = DSBSTATUS_PLAYING;

            if (dwFlags & DSBPLAY_LOOPING) {
                self->Status = self->Status | DSBSTATUS_LOOPING;
            }

            if (self->Caps.dwFlags & DSBCAPS_LOCDEFER) {
                self->Status = self->Status | DSBSTATUS_LOCSOFTWARE;
            }
        }
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
        self->Play = DSBPLAY_NONE;
        self->Status = DSBSTATUS_BUFFERLOST;

        return DSERR_BUFFERLOST;
    }

    if (self->Caps.dwBufferBytes < dwNewPosition) {
        return E_INVALIDARG;
    }

    const DWORD write = (self->Status & DSBSTATUS_PLAYING)
        ? min(self->Caps.dwBufferBytes, ADVANCEWRITEPOSITION(dwNewPosition, self->Format->nBlockAlign))
        : dwNewPosition;

    return dsbcb_set_current_position(self->Buffer,
        BLOCKALIGN(dwNewPosition, self->Format->nBlockAlign), write, DSBCB_SETPOSITION_NONE);
}

HRESULT DELTACALL dsb_set_format(dsb* self, LPCWAVEFORMATEX pcfxFormat) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
        return DSERR_INVALIDCALL;
    }

    if (self->Instance->Level == DSSCL_NONE
        || self->Instance->Level == DSSCL_NORMAL) {
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

    // TODO support larger structs

    CopyMemory(self->Format, pcfxFormat, sizeof(WAVEFORMATEX));

    self->Format->cbSize = 0;

    return S_OK;

    // TODO
    // DirectSound recognizes the WAVE_FORMAT_EXTENSIBLE format tag
    // and correctly plays multiple-channel and compressed formats in hardware buffers,
    // as long as these formats are supported by the driver.
    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee419020(v=vs.85)
}

HRESULT DELTACALL dsb_set_volume(dsb* self, FLOAT fVolume) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
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

    if (!(self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER)) {
        if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
            self->Play = DSBPLAY_NONE;
            self->Status = DSBSTATUS_BUFFERLOST;

            return DSERR_BUFFERLOST;
        }
    }

    if (self->Status & DSBSTATUS_BUFFERLOST) {
        return DSERR_BUFFERLOST;
    }

    HRESULT hr = S_OK;

    if (self->Status & DSBSTATUS_PLAYING) {

        self->Play = DSBPLAY_NONE;
        self->Status = DSBSTATUS_NONE;

        if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
            if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
                hr = dsbcb_set_current_position(self->Buffer, 0, 0, DSBCB_SETPOSITION_NONE);
            }
        }
        else if (self->Caps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY) {
            hr = dsb_trigger_notifications(self, self->Caps.dwBufferBytes, 0);
        }
    }

    return hr;
}

HRESULT DELTACALL dsb_unlock(dsb* self,
    LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) {
    if (self->Instance == NULL) {
        return DSERR_UNINITIALIZED;
    }

    if (self->Caps.dwFlags & DSBCAPS_PRIMARYBUFFER) {
        if (self->Instance->Level != DSSCL_WRITEPRIMARY) {
            return DSERR_PRIOLEVELNEEDED;
        }
    }
    else if (self->Instance->Level == DSSCL_WRITEPRIMARY) {
        self->Play = DSBPLAY_NONE;
        self->Status = DSBSTATUS_BUFFERLOST;

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
            self->Play = DSBPLAY_NONE;
            self->Status = DSBSTATUS_BUFFERLOST;

            return DSERR_BUFFERLOST;
        }
    }

    self->Status = DSBSTATUS_NONE;

    return dsbcb_set_current_position(self->Buffer, 0, 0, DSBCB_SETPOSITION_NONE);
}

HRESULT DELTACALL dsb_update_current_position(dsb* self, DWORD dwAdvance) {
    if (self == NULL) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    DWORD read = 0, write = 0;

    if (SUCCEEDED(hr = dsbcb_get_current_position(self->Buffer, &read, &write))) {
        if (self->Status & DSBSTATUS_LOOPING) {
            if (SUCCEEDED(hr = dsbcb_set_current_position(self->Buffer,
                read + dwAdvance, write + dwAdvance, DSBCB_SETPOSITION_LOOPING))) {
                if (self->Caps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY) {
                    hr = dsb_trigger_notifications(self, read, dwAdvance);
                }
            }
        }
        else {
            if (self->Caps.dwBufferBytes < read + dwAdvance) {
                self->Play = DSBPLAY_NONE;
                self->Status = DSBSTATUS_NONE;

                if (SUCCEEDED(hr = dsbcb_set_current_position(self->Buffer, 0, 0, DSBCB_SETPOSITION_NONE))) {
                    if (self->Caps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY) {
                        hr = dsb_trigger_notifications(self, read, dwAdvance);
                    }
                }
            }
            else {
                const DWORD rad = min(read + dwAdvance, self->Caps.dwBufferBytes);
                const DWORD wad = min(write + dwAdvance, self->Caps.dwBufferBytes);

                if (SUCCEEDED(hr = dsbcb_set_current_position(self->Buffer,
                    rad, wad, DSBCB_SETPOSITION_NONE))) {
                    if (self->Caps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY) {
                        hr = dsb_trigger_notifications(self, read, dwAdvance);
                    }
                }
            }
        }
    }

    return hr;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL dsb_trigger_notifications(dsb* self, DWORD dwPosition, DWORD dwAdvance) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (self->Caps.dwBufferBytes < dwPosition) {
        return E_INVALIDARG;
    }

    if (!(self->Caps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY)) {
        return DSERR_INVALIDCALL;
    }

    HRESULT hr = S_OK;

    if (self->Notifications != NULL) {
        DWORD count = 0;
        LPDSBPOSITIONNOTIFY notes = NULL;

        if (SUCCEEDED(hr = dsn_get_notification_positions(self->Notifications, &count, &notes))) {
            if (count != 0) {
                if (self->Status & DSBSTATUS_LOOPING) {
                    DWORD length = self->Caps.dwBufferBytes < dwPosition + dwAdvance
                        ? self->Caps.dwBufferBytes - dwPosition : dwAdvance;
                    DWORD pending = dwAdvance - length;
                    DWORD position = dwPosition;

                loop:

                    for (DWORD i = 0; i < count; i++) {
                        if (notes[i].dwOffset < position) {
                            continue;
                        }

                        if (position + length < notes[i].dwOffset) {
                            break;
                        }

                        SetEvent(notes[i].hEventNotify);
                    }

                    if (pending != 0) {
                        position = 0;
                        length = self->Caps.dwBufferBytes < pending
                            ? self->Caps.dwBufferBytes : pending;
                        pending -= length;

                        goto loop;
                    }
                }
                else {
                    for (DWORD i = 0; i < count; i++) {
                        if (notes[i].dwOffset < dwPosition) {
                            continue;
                        }

                        if (dwPosition + dwAdvance < notes[i].dwOffset) {
                            break;
                        }

                        SetEvent(notes[i].hEventNotify);
                    }

                    if (self->Caps.dwBufferBytes <= dwPosition + dwAdvance) {
                        if (notes[count - 1].dwOffset == DSBPN_OFFSETSTOP) {
                            SetEvent(notes[count - 1].hEventNotify);
                        }
                    }
                }
            }
        }
    }

    return hr;
}