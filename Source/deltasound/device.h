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

#include "allocator.h"
#include "device_info.h"

// TODO rename this to ds_device
// so that there can be dsc_device for capture...

typedef struct ds ds;

typedef struct device {
    allocator*              Allocator;
    LONG                    RefCount;   // TODO is RefCount needed here??? Device isn't shared...
    ds*                     Instance;

    device_info             Info;

    IMMDevice*              Device;
    IAudioClient*           AudioClient;
    IAudioRenderClient*     AudioRenderer;
    // IAudioStreamVolume*     AudioVolume; //  TODO  Is this needed?

    UINT32                  AudioClientBufferSize;  // In frames

    PWAVEFORMATEXTENSIBLE   WaveFormat;

    HANDLE                  AudioEvent;

    HANDLE                  Thread;
    HANDLE                  ThreadEvent;
    BOOL                    Close;
} device;

HRESULT DELTACALL device_create(
    allocator* pAlloc, ds* pDS, DWORD dwType, device_info* pInfo, device** ppOut);

ULONG DELTACALL device_add_ref(device* pDev);
ULONG DELTACALL device_remove_ref(device* pDev);

VOID DELTACALL device_release(device* pDev);
