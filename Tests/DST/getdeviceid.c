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

#include "getdeviceid.h"

#include <dsound.h>

typedef HRESULT(WINAPI* LPGETDEVICEID)(LPCGUID pGuidSrc, LPGUID pGuidDest);

#define MAX_GETDEVICEID_TEST_COUNT  5

static const LPGUID tests[MAX_GETDEVICEID_TEST_COUNT] = {
    &GUID_NULL,
    &DSDEVID_DefaultPlayback,
    &DSDEVID_DefaultCapture,
    &DSDEVID_DefaultVoicePlayback,
    &DSDEVID_DefaultVoiceCapture
};

static BOOL CompareResults(LPCGUID a, LPCGUID b) {
    if (a != NULL && b != NULL) {
        return memcmp(a, b, sizeof(GUID)) == 0;
    }

    return FALSE;
}

BOOL TestGetDeviceID(HMODULE a, HMODULE b) {
    LPGETDEVICEID ga = (LPGETDEVICEID)GetProcAddress(a, "GetDeviceID");
    LPGETDEVICEID gb = (LPGETDEVICEID)GetProcAddress(b, "GetDeviceID");

    // NULL
    {
        GUID ra, rb;
        ZeroMemory(&ra, sizeof(GUID));
        ZeroMemory(&rb, sizeof(GUID));

        HRESULT ha = ga(NULL, &ra);
        HRESULT hb = gb(NULL, &rb);

        if (ha != hb || !CompareResults(&ra, &rb)) {
            return FALSE;
        }
    }

    // Predefined values
    for (UINT i = 0; i < MAX_GETDEVICEID_TEST_COUNT; i++) {
        GUID ra, rb;
        ZeroMemory(&ra, sizeof(GUID));
        ZeroMemory(&rb, sizeof(GUID));

        HRESULT ha = ga(tests[i], &ra);
        HRESULT hb = gb(tests[i], &rb);

        if (ha != hb || !CompareResults(&ra, &rb)) {
            printf("\r\nGuid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
                ra.Data1, ra.Data2, ra.Data3,
                ra.Data4[0], ra.Data4[1], ra.Data4[2], ra.Data4[3],
                ra.Data4[4], ra.Data4[5], ra.Data4[6], ra.Data4[7]);
            //return FALSE;
        }
    }

    return TRUE;
}
