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

#include <deltasound.h>

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved) {
    UNUSED(hinstDLL);

    // TODO NOT IMPLEMENTED


    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:

        if (lpvReserved != NULL)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

HRESULT WINAPI DirectSoundCreate(
    LPCGUID pcGuidDevice,
    LPDIRECTSOUND* ppDS,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundEnumerateA(
    LPDSENUMCALLBACKA pDSEnumCallback,
    LPVOID pContext) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundEnumerateW(
    LPDSENUMCALLBACKW pDSEnumCallback,
    LPVOID pContext) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureCreate(
    LPCGUID lpcGUID,
    LPDIRECTSOUNDCAPTURE* ppDSC,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureEnumerateA(
    LPDSENUMCALLBACKA pDSEnumCallback,
    LPVOID pContext) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureEnumerateW(
    LPDSENUMCALLBACKW pDSEnumCallback,
    LPVOID pContext) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCreate8(
    LPCGUID pcGuidDevice,
    LPDIRECTSOUND8* ppDS8,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundCaptureCreate8(
    LPCGUID pcGuidDevice,
    LPDIRECTSOUNDCAPTURE8* ppDSC8,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DirectSoundFullDuplexCreate(
    LPCGUID pcGuidCaptureDevice,
    LPCGUID pcGuidRenderDevice,
    LPCDSCBUFFERDESC pcDSCBufferDesc,
    LPCDSBUFFERDESC pcDSBufferDesc,
    HWND hWnd,
    DWORD dwLevel,
    LPDIRECTSOUNDFULLDUPLEX* ppDSFD,
    LPDIRECTSOUNDCAPTUREBUFFER8* ppDSCBuffer8,
    LPDIRECTSOUNDBUFFER8* ppDSBuffer8,
    LPUNKNOWN pUnkOuter) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI GetDeviceID(
    LPCGUID pGuidSrc,
    LPGUID pGuidDest) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT WINAPI DllCanUnloadNow() {
    // TODO NOT IMPLEMENTED
    return S_OK;
}

HRESULT DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID* ppv) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}
