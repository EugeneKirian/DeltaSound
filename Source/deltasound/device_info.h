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

#include "base.h"

#define DEVICETYPE_RENDER           0
#define DEVICETYPE_CAPTURE          1
#define DEVICETYPE_ALL              2
#define DEVICETYPE_INVALID          (-1)

#define DEVICEKIND_AUDIO            0
#define DEVICEKIND_MULTIMEDIA       1
#define DEVICEKIND_COMMUNICATION    2
#define DEVICEKIND_INVALID          (-1)

#define MAX_DEVICE_ID_LENGTH        128

typedef struct device_info {
    GUID    ID;
    DWORD   Type;
    WCHAR   Name[MAX_DEVICE_ID_LENGTH];
    WCHAR   Module[MAX_DEVICE_ID_LENGTH];
} device_info;

HRESULT DELTACALL device_info_get_count(DWORD dwType, UINT* pdwCount);
HRESULT DELTACALL device_info_get_device(DWORD dwType, LPCGUID pcGuidDevice, device_info* pDevice);
HRESULT DELTACALL device_info_get_devices(
    DWORD dwType, UINT* pdwCount, device_info* pDevices);
HRESULT DELTACALL device_info_get_default_device(
    DWORD dwType, DWORD dwKind, device_info* pDevice);
