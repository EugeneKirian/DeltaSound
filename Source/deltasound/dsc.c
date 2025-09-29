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

#include "deltasound.h"
#include "device_info.h"
#include "dsc.h"
#include "dscb.h"
// TODO #include "dscdevice.h"
#include "idsc.h"

HRESULT DELTACALL dsc_create(allocator* pAlloc, REFIID riid, dsc** ppOut) {
    // TODO
    return E_NOTIMPL;
}

VOID DELTACALL dsc_release(dsc* pDSC) {
    // TODO
}


HRESULT DELTACALL dsc_query_interface(dsc* pDSC, REFIID riid, LPVOID* ppOut) {
    // TODO
    return E_NOTIMPL;
}

HRESULT DELTACALL dsc_add_ref(dsc* pDSC, idsc* pIDSC) {
    // TODO
    return E_NOTIMPL;
}

HRESULT DELTACALL dsc_remove_ref(dsc* pDSC, idsc* pIDSC) {
    // TODO
    return E_NOTIMPL;
}


HRESULT DELTACALL dsc_create_capture_buffer(dsc* pDSC, REFIID riid, LPCDSCBUFFERDESC pcDSCBufferDesc, dscb** ppOut) {
    // TODO
    return E_NOTIMPL;
}

HRESULT DELTACALL dsc_remove_capture_buffer(dsc* pDSC, dscb* pDSCB) {
    // TODO
    return E_NOTIMPL;
}

HRESULT DELTACALL dsc_get_caps(dsc* pDSC, LPDSCCAPS pDSCCaps) {
    // TODO
    return E_NOTIMPL;
}

HRESULT DELTACALL dsc_initialize(dsc* pDSC, LPCGUID pcGuidDevice) {
    // TODO
    return E_NOTIMPL;
}
