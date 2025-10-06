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

#include "capture.h"
#include "dsc.h"
#include "dscb.h"
#include "uuid.h"

HRESULT DELTACALL capture_create(allocator* pAlloc, dsc* pDSC, device_info* pInfo, capture** ppOut) {
    if (pAlloc == NULL) {
        return E_INVALIDARG;
    }

    if (pInfo == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    capture* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(capture), &instance))) {
        instance->Allocator = pAlloc;
        instance->Instance = pDSC;

        CopyMemory(&instance->Info, pInfo, sizeof(device_info));

        // TODO NOT IMPLEMENTED

        *ppOut = instance;

        return S_OK;
    }

    return hr;
}

VOID DELTACALL capture_release(capture* self) {
    if (self == NULL) { return; }

    // TODO NOT IMPLEMENTED

    allocator_free(self->Allocator, self);
}
