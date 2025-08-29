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

#define WASAPIDEVICETYPE_AUDIO  0
#define WASAPIDEVICETYPE_RECORD 1

#define MAX_WASAPI_DEVICE_IDENTITY_LENGTH   256

typedef struct wasapi_device {
    GUID    ID;
    DWORD   Type;
    WCHAR   Name[MAX_WASAPI_DEVICE_IDENTITY_LENGTH];
    WCHAR   Module[MAX_WASAPI_DEVICE_IDENTITY_LENGTH];
} wasapi_device;

HRESULT DELTACALL wasapi_device_get_count(DWORD dwType, UINT* pdwCount);
HRESULT DELTACALL wasapi_device_get_devices(DWORD dwType, UINT* pdwCount, wasapi_device* pDevices);