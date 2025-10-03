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

#include "cf.h"
#include "device_info.h"
#include "intfc.h"
#include "prvt.h"
#include "uuid.h"

#include <mmddk.h>

#define MAX_INTERFACE_LENGTH    512

HRESULT DELTACALL prvt_get_device_interface(prvt* pPrvt, DWORD dwType,
    LPWSTR pszModule, LPWSTR pszInterface, DWORD dwLength, LPDWORD pdwBytes);
HRESULT DELTACALL prvt_get_render_device_interface(prvt* pPrvt,
    LPWSTR pszModule, LPWSTR pszInterface, DWORD dwLength, LPDWORD pdwBytes);
HRESULT DELTACALL prvt_get_capture_device_interface(prvt* pPrvt,
    LPWSTR pszModule, LPWSTR pszInterface, DWORD dwLength, LPDWORD pdwBytes);

HRESULT DELTACALL prvt_get_wave_device_mapping_ansi(prvt* pPrvt,
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned);
HRESULT DELTACALL prvt_get_wave_device_mapping_wide(prvt* pPrvt,
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned);

HRESULT DELTACALL prvt_get_description(prvt* pPrvt,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned);
HRESULT DELTACALL prvt_get_description_ansi(prvt* pPrvt,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned);
HRESULT DELTACALL prvt_get_description_wide(prvt* sepPrvtlf,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned);

HRESULT DELTACALL prvt_create(allocator* pAlloc, REFIID riid, prvt** ppOut) {
    if (pAlloc == NULL || riid == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    prvt* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(prvt), &instance))) {
        instance->Allocator = pAlloc;

        CopyMemory(&instance->ID, riid, sizeof(IID));

        if (SUCCEEDED(hr = intfc_create(pAlloc, &instance->Interfaces))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        prvt_release(instance);
    }

    return hr;
}

VOID DELTACALL prvt_release(prvt* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    const DWORD count = intfc_get_count(self->Interfaces);

    for (DWORD i = 0; i < count; i++) {
        iprvt* instance = NULL;

        if (SUCCEEDED(intfc_get_item(self->Interfaces, i, &instance))) {
            iprvt_release(instance);
        }
    }

    intfc_release(self->Interfaces);

    if (self->Instance != NULL) {
        cf_remove_private(self->Instance, self);
    }

    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL prvt_query_interface(prvt* self, REFIID riid, LPVOID* ppOut) {
    HRESULT hr = E_NOINTERFACE;

    EnterCriticalSection(&self->Lock);

    {
        iprvt* instance = NULL;

        if (SUCCEEDED(hr = intfc_query_item(self->Interfaces, riid, &instance))) {
            iprvt_add_ref(instance);

            *ppOut = instance;

            goto exit;
        }
    }

    if (IsEqualIID(&IID_IUnknown, riid)
        || IsEqualIID(&IID_IKsPropertySet, riid)) {
        iprvt* instance = NULL;

        if (SUCCEEDED(hr = iprvt_create(self->Allocator, riid, &instance))) {
            if (SUCCEEDED(hr = prvt_add_ref(self, instance))) {
                instance->Instance = self;

                *ppOut = instance;

                goto exit;
            }

            iprvt_release(instance);
        }
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL prvt_add_ref(prvt* self, iprvt* pIPrvt) {
    return intfc_add_item(self->Interfaces, &pIPrvt->ID, pIPrvt);
}

HRESULT DELTACALL prvt_remove_ref(prvt* self, iprvt* pIPrvt) {
    return intfc_remove_item(self->Interfaces, &pIPrvt->ID);
}

HRESULT DELTACALL prvt_get(prvt* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData,
    ULONG ulDataLength, PULONG pulBytesReturned) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (IsEqualGUID(&DSPROPSETID_DirectSoundDevice, rguidPropSet)) {
        switch (ulId) {
        case DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A: {
            return prvt_get_wave_device_mapping_ansi(self,
                (PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA)pPropertyData, ulDataLength, pulBytesReturned);
        }
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1: {
            return prvt_get_description(self,
                (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA)pPropertyData, ulDataLength, pulBytesReturned);
        }
        // TODO DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1
        case DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W: {
            return prvt_get_wave_device_mapping_wide(self,
                (PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA)pPropertyData, ulDataLength, pulBytesReturned);
        }
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A: {
            return prvt_get_description_ansi(self,
                (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA)pPropertyData, ulDataLength, pulBytesReturned);
        }
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W: {
            return prvt_get_description_wide(self,
                (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA)pPropertyData, ulDataLength, pulBytesReturned);
        }
        // TODO DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A
        // TODO DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W
        }
    }

    return E_PROP_ID_UNSUPPORTED;
}

HRESULT DELTACALL prvt_set(prvt* self,
    REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData,
    ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength) {
    // TODO NOT IMPLEMENTED
    return E_NOTIMPL;
}

HRESULT DELTACALL prvt_query_support(prvt* self,
    REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (IsEqualGUID(&DSPROPSETID_DirectSoundDevice, rguidPropSet)) {
        if (ulId == DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A
            || ulId == DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1
            || ulId == DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1
            || ulId == DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W
            || ulId == DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A
            || ulId == DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W
            || ulId == DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A
            || ulId == DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W) {
            *pulTypeSupport = KSPROPERTY_SUPPORT_GET;

            return S_OK;
        }

        return E_PROP_ID_UNSUPPORTED;
    }

    return E_PROP_SET_UNSUPPORTED;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL prvt_get_device_interface(prvt* self, DWORD dwType,
    LPWSTR pszModule, LPWSTR pszInterface, DWORD dwLength, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (dwType != DIRECTSOUNDDEVICE_DATAFLOW_RENDER
        && dwType != DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {
        return E_INVALIDARG;
    }

    return dwType == DIRECTSOUNDDEVICE_DATAFLOW_RENDER
        ? prvt_get_render_device_interface(self, pszModule, pszInterface, dwLength, pdwBytes)
        : prvt_get_capture_device_interface(self, pszModule, pszInterface, dwLength, pdwBytes);
}

HRESULT DELTACALL prvt_get_render_device_interface(prvt* self,
    LPWSTR pszModule, LPWSTR pszInterface, DWORD dwLength, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pszModule == NULL || pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    if (pszInterface == NULL && dwLength != 0) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    MMRESULT mmr = MMSYSERR_NOERROR;

    const UINT count = waveOutGetNumDevs();
    const UINT length = (wcslen(pszModule) + 1) * sizeof(WCHAR);

    for (UINT i = 0; i < count; i++) {
        ULONG size = 0;
        HWAVEOUT wave = (HWAVEOUT)IntToPtr(i);

        mmr = waveOutMessage(wave, DRV_QUERYFUNCTIONINSTANCEIDSIZE, (DWORD_PTR)&size, (DWORD_PTR)NULL);

        if (mmr == MMSYSERR_NOERROR && length == size) {
            LPWSTR module = NULL;

            if (SUCCEEDED(hr = allocator_allocate(self->Allocator, size, &module))) {
                mmr = waveOutMessage(wave, DRV_QUERYFUNCTIONINSTANCEID, (DWORD_PTR)module, size);

                if (mmr == MMSYSERR_NOERROR) {
                    if (wcscmp(pszModule, module) == 0) {
                        mmr = waveOutMessage(wave, DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&size, (DWORD_PTR)NULL);

                        if (mmr == MMSYSERR_NOERROR) {
                            LPWSTR intfc = NULL;

                            if (SUCCEEDED(hr = allocator_allocate(self->Allocator, size, &intfc))) {

                                mmr = waveOutMessage(wave, DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)intfc, size);

                                if (mmr == MMSYSERR_NOERROR) {
                                    *pdwBytes = size;

                                    if (pszInterface != NULL) {
                                        wcsncpy(pszInterface, intfc, dwLength);
                                    }
                                }

                                allocator_free(self->Allocator, intfc);
                            }
                        }
                    }
                }

                allocator_free(self->Allocator, module);
            }
        }
    }

    return hr;
}

HRESULT DELTACALL prvt_get_capture_device_interface(prvt* self,
    LPWSTR pszModule, LPWSTR pszInterface, DWORD dwLength, LPDWORD pdwBytes) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (pszModule == NULL || pdwBytes == NULL) {
        return E_INVALIDARG;
    }

    if (pszInterface == NULL && dwLength != 0) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    MMRESULT mmr = MMSYSERR_NOERROR;

    const UINT count = waveInGetNumDevs();
    const UINT length = (wcslen(pszModule) + 1) * sizeof(WCHAR);

    for (UINT i = 0; i < count; i++) {
        ULONG size = 0;
        HWAVEIN wave = (HWAVEIN)IntToPtr(i);

        mmr = waveInMessage(wave, DRV_QUERYFUNCTIONINSTANCEIDSIZE, (DWORD_PTR)&size, (DWORD_PTR)NULL);

        if (mmr == MMSYSERR_NOERROR && length == size) {
            LPWSTR module = NULL;

            if (SUCCEEDED(hr = allocator_allocate(self->Allocator, size, &module))) {
                mmr = waveInMessage(wave, DRV_QUERYFUNCTIONINSTANCEID, (DWORD_PTR)module, size);

                if (mmr == MMSYSERR_NOERROR) {
                    if (wcscmp(pszModule, module) == 0) {
                        mmr = waveInMessage(wave, DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&size, (DWORD_PTR)NULL);

                        if (mmr == MMSYSERR_NOERROR) {
                            LPWSTR intfc = NULL;

                            if (SUCCEEDED(hr = allocator_allocate(self->Allocator, size, &intfc))) {

                                mmr = waveInMessage(wave, DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)intfc, size);

                                if (mmr == MMSYSERR_NOERROR) {
                                    *pdwBytes = size;

                                    if (pszInterface != NULL) {
                                        wcsncpy(pszInterface, intfc, dwLength);
                                    }
                                }

                                allocator_free(self->Allocator, intfc);
                            }
                        }
                    }
                }

                allocator_free(self->Allocator, module);
            }
        }
    }

    return hr;
}

HRESULT DELTACALL prvt_get_wave_device_mapping_ansi(prvt* self,
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) {
    if (pPropertyData == NULL) {
        return E_INVALIDARG;
    }

    *pulBytesReturned = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA);

    if (ulDataLength != 0
        && ulDataLength < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA)) {
        return E_INVALIDARG;
    }

    if (sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA) <= ulDataLength) {
        if (pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_RENDER
            && pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {
            return E_INVALIDARG;
        }

        if (pPropertyData->DeviceName == NULL) {
            return DSERR_NODRIVER;
        }

        HRESULT hr = S_OK;
        DWORD count = 0;

        if (SUCCEEDED(hr = device_info_get_count(pPropertyData->DataFlow, &count))) {
            device_info* devices = NULL;

            if (SUCCEEDED(hr = allocator_allocate(self->Allocator, count * sizeof(device_info), &devices))) {
                if (SUCCEEDED(hr = device_info_get_devices(pPropertyData->DataFlow, &count, devices))) {
                    BOOL match = FALSE;

                    for (DWORD i = 0; i < count; i++) {
                        CHAR name[MAX_DEVICE_NAME_LENGTH];
                        ZeroMemory(name, MAX_DEVICE_NAME_LENGTH);

                        WideCharToMultiByte(CP_ACP, 0,
                            devices[i].Name, -1, name, MAX_DEVICE_NAME_LENGTH, NULL, NULL);

                        if (strncmp(pPropertyData->DeviceName, name, MAXPNAMELEN - 1) == 0) {
                            match = TRUE;

                            CopyMemory(&pPropertyData->DeviceId, &devices[i].ID, sizeof(GUID));

                            break;
                        }
                    }

                    if (!match) {
                        hr = DSERR_NODRIVER;
                    }
                }

                allocator_free(self->Allocator, devices);
            }
        }

        return hr;
    }

    return S_OK;
}

HRESULT DELTACALL prvt_get_wave_device_mapping_wide(prvt* self,
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) {
    if (pPropertyData == NULL) {
        return E_INVALIDARG;
    }

    *pulBytesReturned = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA);

    if (ulDataLength != 0
        && ulDataLength < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA)) {
        return E_INVALIDARG;
    }

    if (sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA) <= ulDataLength) {
        if (pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_RENDER
            && pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {
            return E_INVALIDARG;
        }

        if (pPropertyData->DeviceName == NULL) {
            return DSERR_NODRIVER;
        }

        HRESULT hr = S_OK;
        DWORD count = 0;

        if (SUCCEEDED(hr = device_info_get_count(pPropertyData->DataFlow, &count))) {
            device_info* devices = NULL;

            if (SUCCEEDED(hr = allocator_allocate(self->Allocator, count * sizeof(device_info), &devices))) {
                if (SUCCEEDED(hr = device_info_get_devices(pPropertyData->DataFlow, &count, devices))) {
                    BOOL match = FALSE;

                    for (DWORD i = 0; i < count; i++) {
                        if (wcsncmp(pPropertyData->DeviceName, devices[i].Name, MAXPNAMELEN - 1) == 0) {
                            match = TRUE;

                            CopyMemory(&pPropertyData->DeviceId, &devices[i].ID, sizeof(GUID));

                            break;
                        }
                    }

                    if (!match) {
                        hr = DSERR_NODRIVER;
                    }
                }

                allocator_free(self->Allocator, devices);
            }
        }

        return hr;
    }

    return S_OK;
}

HRESULT DELTACALL prvt_get_description(prvt* self,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) {
    if (pPropertyData == NULL) {
        return E_INVALIDARG;
    }

    *pulBytesReturned = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA);

    if (ulDataLength != 0
        && ulDataLength < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA)) {
        return E_INVALIDARG;
    }

    if (sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA) <= ulDataLength) {
        if (pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_RENDER
            && pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {
            return E_INVALIDARG;
        }

        HRESULT hr = S_OK;
        device_info* device = NULL;

        if (IsEqualGUID(&GUID_NULL, &pPropertyData->DeviceId)) {
            CopyMemory(&pPropertyData->DeviceId,
                pPropertyData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER
                ? &DSDEVID_DefaultPlayback : &DSDEVID_DefaultCapture, sizeof(GUID));
        }

        if (SUCCEEDED(hr = allocator_allocate(self->Allocator, sizeof(device_info), &device))) {
            if (IsEqualGUID(&DSDEVID_DefaultPlayback, &pPropertyData->DeviceId)
                || IsEqualGUID(&DSDEVID_DefaultCapture, &pPropertyData->DeviceId)) {
                if (FAILED(hr = device_info_get_default_device(pPropertyData->DataFlow, DEVICEKIND_MULTIMEDIA, device))) {
                    goto exit;
                }
            }
            else if (IsEqualGUID(&DSDEVID_DefaultVoicePlayback, &pPropertyData->DeviceId)
                || IsEqualGUID(&DSDEVID_DefaultVoiceCapture, &pPropertyData->DeviceId)) {
                if (FAILED(hr = device_info_get_default_device(pPropertyData->DataFlow, DEVICEKIND_COMMUNICATION, device))) {
                    goto exit;
                }
            }
            else if (FAILED(hr = device_info_get_device(pPropertyData->DataFlow, &pPropertyData->DeviceId, device))) {
                goto exit;
            }

            CopyMemory(&pPropertyData->DeviceId, &device->ID, sizeof(GUID));

            WideCharToMultiByte(CP_ACP, 0, device->Name, -1,
                pPropertyData->DescriptionA, MAX_DEVICE_NAME_LENGTH, NULL, NULL);
            WideCharToMultiByte(CP_ACP, 0, device->Module, -1,
                pPropertyData->ModuleA, MAX_DEVICE_MODULE_LENGTH, NULL, NULL);

            wcscpy_s(pPropertyData->DescriptionW, MAX_DEVICE_NAME_LENGTH - 1, device->Name);
            wcscpy_s(pPropertyData->ModuleW, MAX_DEVICE_MODULE_LENGTH - 1, device->Module);

            pPropertyData->Devnode = 0;
            pPropertyData->WaveDeviceId = 0;
            pPropertyData->Type = DIRECTSOUNDDEVICE_TYPE_WDM;

        exit:

            allocator_free(self->Allocator, device);
        }

        return hr;
    }

    return S_OK;
}

HRESULT DELTACALL prvt_get_description_ansi(prvt* self,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) {
    if (pPropertyData == NULL) {
        return E_INVALIDARG;
    }

    *pulBytesReturned = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);

    if (ulDataLength != 0
        && ulDataLength < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA)) {
        return E_INVALIDARG;
    }

    if (sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA) <= ulDataLength) {
        if (pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_RENDER
            && pPropertyData->DataFlow != DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {
            return E_INVALIDARG;
        }

        HRESULT hr = S_OK;
        device_info* device = NULL;

        if (IsEqualGUID(&GUID_NULL, &pPropertyData->DeviceId)) {
            CopyMemory(&pPropertyData->DeviceId,
                pPropertyData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER
                ? &DSDEVID_DefaultPlayback : &DSDEVID_DefaultCapture, sizeof(GUID));
        }

        if (SUCCEEDED(hr = allocator_allocate(self->Allocator, sizeof(device_info), &device))) {
            if (IsEqualGUID(&DSDEVID_DefaultPlayback, &pPropertyData->DeviceId)
                || IsEqualGUID(&DSDEVID_DefaultCapture, &pPropertyData->DeviceId)) {
                if (FAILED(hr = device_info_get_default_device(pPropertyData->DataFlow, DEVICEKIND_MULTIMEDIA, device))) {
                    goto exit;
                }
            }
            else if (IsEqualGUID(&DSDEVID_DefaultVoicePlayback, &pPropertyData->DeviceId)
                || IsEqualGUID(&DSDEVID_DefaultVoiceCapture, &pPropertyData->DeviceId)) {
                if (FAILED(hr = device_info_get_default_device(pPropertyData->DataFlow, DEVICEKIND_COMMUNICATION, device))) {
                    goto exit;
                }
            }
            else if (FAILED(hr = device_info_get_device(pPropertyData->DataFlow, &pPropertyData->DeviceId, device))) {
                goto exit;
            }

            CopyMemory(&pPropertyData->DeviceId, &device->ID, sizeof(GUID));

            DWORD length = 0;
            WCHAR intfc[MAX_INTERFACE_LENGTH];
            ZeroMemory(intfc, sizeof(WCHAR) * MAX_INTERFACE_LENGTH);

            DWORD bytes = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA)
                + WideCharToMultiByte(CP_ACP, 0, device->Name, -1, NULL, 0, NULL, NULL)
                + WideCharToMultiByte(CP_ACP, 0, device->Module, -1, NULL, 0, NULL, NULL);

            if (SUCCEEDED(hr = prvt_get_device_interface(self,
                pPropertyData->DataFlow, device->Module, intfc, MAX_INTERFACE_LENGTH, &length))) {
                bytes += WideCharToMultiByte(CP_ACP, 0, intfc, -1, NULL, 0, NULL, NULL);
            }

            *pulBytesReturned = bytes;

            if (bytes <= ulDataLength) {
                DWORD offset = sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA);

                // Description
                {
                    pPropertyData->Description = (LPSTR)((size_t)pPropertyData + offset);

                    offset += WideCharToMultiByte(CP_ACP, 0, device->Name, -1,
                        pPropertyData->Description, ulDataLength - offset, NULL, NULL);
                }

                // Module
                {
                    pPropertyData->Module = (LPSTR)((size_t)pPropertyData + offset);

                    offset += WideCharToMultiByte(CP_ACP, 0, device->Module, -1,
                        pPropertyData->Module, ulDataLength - offset, NULL, NULL);
                }

                // Interface
                {
                    pPropertyData->Interface = (LPSTR)((size_t)pPropertyData + offset);

                    offset += WideCharToMultiByte(CP_ACP, 0, intfc, -1,
                        pPropertyData->Interface, ulDataLength - offset, NULL, NULL);

                    strupr(pPropertyData->Interface);
                }
            }

            pPropertyData->WaveDeviceId = 0;
            pPropertyData->Type = DIRECTSOUNDDEVICE_TYPE_WDM;

        exit:

            allocator_free(self->Allocator, device);
        }

        return hr;
    }

    return S_OK;
}

HRESULT DELTACALL prvt_get_description_wide(prvt* self,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned) {
    // TODO
    return E_NOTIMPL;
}
