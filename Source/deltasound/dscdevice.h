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

#pragma once

#include "arena.h"
#include "device_info.h"
#include "mixer.h"

typedef struct dsc dsc;

#define DSDEVICE_AUDIO_EVENT_INDEX      0
#define DSDEVICE_CLOSE_EVENT_INDEX      1

#define DSDEVICE_MAX_EVENT_COUNT        2

typedef struct dscdevice {
    allocator* Allocator;
    dsc* Instance;
    arena* Arena;
    // mixer* Mixer; // TODO

    device_info             Info;

    IMMDevice*              Device;
    // IAudioClient*           AudioClient; // TODO
    //IAudioRenderClient*     AudioRenderer; // TODO

    UINT32                  AudioClientBufferSize;  // In frames

    PWAVEFORMATEXTENSIBLE   Format;

    HANDLE                  Events[DSDEVICE_MAX_EVENT_COUNT];

    HANDLE                  Thread;
    HANDLE                  ThreadEvent;
} dscdevice;

HRESULT DELTACALL dscdevice_create(allocator* pAlloc,
    dsc* pDSC, DWORD dwType, device_info* pInfo, dscdevice** ppOut);
VOID DELTACALL dscdevice_release(dscdevice* pDev);
