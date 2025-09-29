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

#include "dllgetclassobject.h"

static BOOL TestDirectSoundClassFactory(LPFNGETCLASSOBJECT a, LPFNGETCLASSOBJECT b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // IClassFactory
    {
        BOOL result = TRUE;
        LPCLASSFACTORY cfa = NULL, cfb = NULL;
        LPDIRECTSOUND dsa = NULL, dsb = NULL;
        HRESULT ra = S_OK, rb = S_OK;
        ULONG refa = 0, refb = 0;

        if (FAILED(ra = a(&CLSID_DirectSound, &IID_IClassFactory, &cfa))) {
            result = FALSE;
            goto exit1;
        }

        if (FAILED(rb = b(&CLSID_DirectSound, &IID_IClassFactory, &cfb))) {
            result = FALSE;
            goto exit1;
        }

        if (ra != rb) {
            result = FALSE;
            goto exit1;
        }

        refa = IClassFactory_AddRef(cfa);
        refb = IClassActivator_AddRef(cfb);

        if (refa != refb) {
            result = FALSE;
            goto exit1;
        }

        refa = IClassFactory_Release(cfa);
        refb = IClassActivator_Release(cfb);

        if (refa != refb) {
            result = FALSE;
            goto exit1;
        }

        ra = IClassFactory_CreateInstance(cfa, NULL, &IID_IDirectSound, &dsa);
        rb = IClassFactory_CreateInstance(cfb, NULL, &IID_IDirectSound, &dsb);

        if (ra != rb || dsa == NULL || dsb == NULL) {
            result = FALSE;
            goto exit1;
        }

        RELEASE(dsa);
        RELEASE(dsb);

    exit1:

        RELEASE(cfa);
        RELEASE(cfb);

        if (!result) {
            return result;
        }
    }

    // IUnknown
    {
        BOOL result = TRUE;
        LPUNKNOWN ua = NULL, ub = NULL;
        HRESULT ra = S_OK, rb = S_OK;

        if (FAILED(ra = a(&CLSID_DirectSound, &IID_IUnknown, &ua))) {
            result = FALSE;
            goto exit2;
        }

        if (FAILED(rb = b(&CLSID_DirectSound, &IID_IUnknown, &ub))) {
            result = FALSE;
            goto exit2;
        }

        if (ra != rb) {
            result = FALSE;
            goto exit2;
        }

    exit2:
        RELEASE(ua);
        RELEASE(ub);

        if (!result) {
            return result;
        }
    }

    return TRUE;
}

BOOL TestDllGetClassObject(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPFNGETCLASSOBJECT gcoa = (LPFNGETCLASSOBJECT)GetProcAddress(a, "DllGetClassObject");
    LPFNGETCLASSOBJECT gcob = (LPFNGETCLASSOBJECT)GetProcAddress(b, "DllGetClassObject");

    if (gcoa == NULL || gcob == NULL) {
        return FALSE;
    }

    if (!TestDirectSoundClassFactory(gcoa, gcob)) {
        return FALSE;
    }

    // TODO Other classes...

    return TRUE;
}
