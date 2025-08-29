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

#include "directsoundenumerate.h"

#define MAX_DIRECTSOUND_DEVICE_NAME_LENGTH  32

typedef BOOL(CALLBACK* LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef HRESULT(WINAPI* LPDIRECTSOUNDENUMERATEA)(LPDSENUMCALLBACKA, LPVOID);

#define MAX_STORAGE_COUNT           128
#define MAX_STORAGE_STRING_LENGTH   512

typedef struct storage_a {
    GUID    ID;
    BOOL    IsNullID;
    CHAR    Name[MAX_STORAGE_STRING_LENGTH];
    CHAR    Module[MAX_STORAGE_STRING_LENGTH];
} storage_a;

typedef struct context_a {
    UINT        Count;
    storage_a* Items;
} context_a;

static BOOL storage_a_compare(storage_a* a, storage_a* b) {
    if (memcmp(&a->ID, &b->ID, sizeof(GUID)) != 0) {
        return FALSE;
    }

    if (a->IsNullID != b->IsNullID) {
        return FALSE;
    }

    if (strncmp(a->Name, b->Name, MAX_DIRECTSOUND_DEVICE_NAME_LENGTH)) {
        return FALSE;
    }

    if (strcmp(a->Module, b->Module)) {
        return FALSE;
    }

    return TRUE;
}

static BOOL CALLBACK EnumerateDeviceCallBackA(LPGUID guid, LPCSTR desc, LPCSTR module, LPVOID ctx) {
    context_a* context = (context_a*)ctx;

    if (context->Count < MAX_STORAGE_COUNT) {
        storage_a* item = &context->Items[context->Count];

        if (guid != NULL) {
            CopyMemory(&item->ID, guid, sizeof(GUID));
        }

        item->IsNullID = guid == NULL;

        strcpy(item->Name, desc);
        strcpy(item->Module, module);

        context->Count++;
    }

    return TRUE;
}

BOOL TestDirectSoundCaptureEnumerateA(HMODULE a, HMODULE b) {
    BOOL result = TRUE;

    context_a ca, cb;
    ZeroMemory(&ca, sizeof(context_a));
    ZeroMemory(&cb, sizeof(context_a));

    const size_t size = sizeof(storage_a) * MAX_STORAGE_COUNT;

    if ((ca.Items = (storage_a*)malloc(size)) == NULL) {
        result = FALSE;
        goto exit;
    }

    if ((cb.Items = (storage_a*)malloc(size)) == NULL) {
        result = FALSE;
        goto exit;
    }

    ZeroMemory(ca.Items, size);
    ZeroMemory(cb.Items, size);

    LPDIRECTSOUNDENUMERATEA aa =
        (LPDIRECTSOUNDENUMERATEA)GetProcAddress(a, "DirectSoundCaptureEnumerateA");

    LPDIRECTSOUNDENUMERATEA ba =
        (LPDIRECTSOUNDENUMERATEA)GetProcAddress(b, "DirectSoundCaptureEnumerateA");

    if (aa == NULL || ba == NULL) {
        result = FALSE;
        goto exit;
    }

    if (aa(EnumerateDeviceCallBackA, &ca) != ba(EnumerateDeviceCallBackA, &cb)) {
        result = FALSE;
        goto exit;
    }

    if (ca.Count != cb.Count) {
        result = FALSE;
        goto exit;
    }

    for (UINT i = 0; i < ca.Count; i++) {
        if (!storage_a_compare(&ca.Items[i], &cb.Items[i])) {
            result = FALSE;
            goto exit;
        }
    }

exit:
    if (ca.Items != NULL) {
        free(ca.Items);
    }

    if (cb.Items != NULL) {
        free(cb.Items);
    }

    return result;
}

/* ---------------------------------------------------------------------- */

typedef BOOL(CALLBACK* LPDSENUMCALLBACKW)(LPGUID, LPCWSTR, LPCWSTR, LPVOID);
typedef HRESULT(WINAPI* LPDIRECTSOUNDENUMERATEW)(LPDSENUMCALLBACKW, LPVOID);

typedef struct storage_w {
    GUID    ID;
    BOOL    IsNullID;
    WCHAR   Name[MAX_STORAGE_STRING_LENGTH];
    WCHAR   Module[MAX_STORAGE_STRING_LENGTH];
} storage_w;

typedef struct context_w {
    UINT        Count;
    storage_w* Items;
} context_w;

static BOOL storage_w_compare(storage_w* a, storage_w* b) {
    if (memcmp(&a->ID, &b->ID, sizeof(GUID)) != 0) {
        return FALSE;
    }

    if (a->IsNullID != b->IsNullID) {
        return FALSE;
    }

    if (wcsncmp(a->Name, b->Name, MAX_DIRECTSOUND_DEVICE_NAME_LENGTH)) {
        return FALSE;
    }

    if (wcscmp(a->Module, b->Module)) {
        return FALSE;
    }

    return TRUE;
}

static BOOL CALLBACK EnumerateDeviceCallBackW(LPGUID guid, LPCWSTR desc, LPCWSTR module, LPVOID ctx) {
    context_w* context = (context_w*)ctx;

    if (context->Count < MAX_STORAGE_COUNT) {
        storage_w* item = &context->Items[context->Count];

        if (guid != NULL) {
            CopyMemory(&item->ID, guid, sizeof(GUID));
        }

        item->IsNullID = guid == NULL;

        wcscpy(item->Name, desc);
        wcscpy(item->Module, module);

        context->Count++;
    }

    return TRUE;
}

BOOL TestDirectSoundCaptureEnumerateW(HMODULE a, HMODULE b)
{
    BOOL result = TRUE;

    context_w ca, cb;
    ZeroMemory(&ca, sizeof(context_w));
    ZeroMemory(&cb, sizeof(context_w));

    const size_t size = sizeof(storage_w) * MAX_STORAGE_COUNT;

    if ((ca.Items = (storage_w*)malloc(size)) == NULL) {
        result = FALSE;
        goto exit;
    }

    if ((cb.Items = (storage_w*)malloc(size)) == NULL) {
        result = FALSE;
        goto exit;
    }

    ZeroMemory(ca.Items, size);
    ZeroMemory(cb.Items, size);

    LPDIRECTSOUNDENUMERATEW aa =
        (LPDIRECTSOUNDENUMERATEW)GetProcAddress(a, "DirectSoundCaptureEnumerateW");

    LPDIRECTSOUNDENUMERATEW ba =
        (LPDIRECTSOUNDENUMERATEW)GetProcAddress(b, "DirectSoundCaptureEnumerateW");

    if (aa == NULL || ba == NULL) {
        result = FALSE;
        goto exit;
    }

    if (aa(EnumerateDeviceCallBackW, &ca) != ba(EnumerateDeviceCallBackW, &cb)) {
        result = FALSE;
        goto exit;
    }

    if (ca.Count != cb.Count) {
        result = FALSE;
        goto exit;
    }

    for (UINT i = 0; i < ca.Count; i++) {
        if (!storage_w_compare(&ca.Items[i], &cb.Items[i])) {
            result = FALSE;
            goto exit;
        }
    }

exit:
    if (ca.Items != NULL) {
        free(ca.Items);
    }

    if (cb.Items != NULL) {
        free(cb.Items);
    }

    return result;
}
