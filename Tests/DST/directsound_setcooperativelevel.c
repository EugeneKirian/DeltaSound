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

#include "directsound.h"
#include "wnd.h"

#define WINDOW_NAME "Set Cooperative Level"

static BOOL TestDirectSoundSetCooperativeLevelInvalidParams(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, NULL, 0);
        rb = IDirectSound_SetCooperativeLevel(dsb, NULL, 0);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, (HWND)dsa, 0);
        rb = IDirectSound_SetCooperativeLevel(dsb, (HWND)dsb, 0);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, GetDesktopWindow(), 0);
        rb = IDirectSound_SetCooperativeLevel(dsb, GetDesktopWindow(), 0);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, GetDesktopWindow(), DSSCL_WRITEPRIMARY + 1);
        rb = IDirectSound_SetCooperativeLevel(dsb, GetDesktopWindow(), DSSCL_WRITEPRIMARY + 1);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, wa, 0);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, 0);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_WRITEPRIMARY + 1);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_WRITEPRIMARY + 1);

        if (ra != rb) {
            return FALSE;
        }
    }

    RELEASE(dsa);
    RELEASE(dsb);

    return TRUE;
}

static BOOL TestDirectSoundSetCooperativeLevelValue(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD dwLevel) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, wa, dwLevel);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, dwLevel);

        if (ra != rb) {
            return FALSE;
        }
    }

    RELEASE(dsa);
    RELEASE(dsb);

    return TRUE;
}

static BOOL TestDirectSoundSetCooperativeLevelValidParams(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (!TestDirectSoundSetCooperativeLevelValue(a, wa, b, wb, DSSCL_NORMAL)) {
        return FALSE;
    }

    if (!TestDirectSoundSetCooperativeLevelValue(a, wa, b, wb, DSSCL_PRIORITY)) {
        return FALSE;
    }

    if (!TestDirectSoundSetCooperativeLevelValue(a, wa, b, wb, DSSCL_EXCLUSIVE)) {
        return FALSE;
    }

    if (!TestDirectSoundSetCooperativeLevelValue(a, wa, b, wb, DSSCL_WRITEPRIMARY)) {
        return FALSE;
    }

    return TRUE;
}

static BOOL TestDirectSoundSetCooperativeLevelAlreadySet(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_NORMAL);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_PRIORITY);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_PRIORITY);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_NORMAL);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSound_SetCooperativeLevel(dsa, NULL, DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(dsb, NULL, DSSCL_NORMAL);

        if (ra != rb) {
            return FALSE;
        }
    }

    RELEASE(dsa);
    RELEASE(dsb);

    return TRUE;
}

static BOOL TestDirectSoundSetCooperativeLevelChangeWindow(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, HWND wc) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_NORMAL);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSound_SetCooperativeLevel(dsa, wc, DSSCL_PRIORITY);
        rb = IDirectSound_SetCooperativeLevel(dsb, wc, DSSCL_PRIORITY);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_NORMAL);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSound_SetCooperativeLevel(dsa, wc, DSSCL_PRIORITY);
        rb = IDirectSound_SetCooperativeLevel(dsb, wc, DSSCL_PRIORITY);

        if (ra != rb) {
            return FALSE;
        }
    }

    RELEASE(dsa);
    RELEASE(dsb);

    return TRUE;
}

static BOOL TestDirectSoundSetCooperativeLevelMultipleInstances(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa1 = NULL;
    LPDIRECTSOUND dsb1 = NULL;

    HRESULT ra = a(NULL, &dsa1, NULL);
    HRESULT rb = b(NULL, &dsb1, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa1 == NULL || dsb1 == NULL) {
        return FALSE;
    }

    LPDIRECTSOUND dsa2 = NULL;
    LPDIRECTSOUND dsb2 = NULL;

    ra = a(NULL, &dsa2, NULL);
    rb = b(NULL, &dsb2, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa2 == NULL || dsb2 == NULL) {
        return FALSE;
    }

    {
        ra = IDirectSound_SetCooperativeLevel(dsa1, wa, DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(dsb1, wb, DSSCL_NORMAL);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSound_SetCooperativeLevel(dsa2, wa, DSSCL_NORMAL);
        rb = IDirectSound_SetCooperativeLevel(dsb2, wb, DSSCL_NORMAL);

        if (ra != rb) {
            return FALSE;
        }
    }

    RELEASE(dsa1);
    RELEASE(dsb1);
    RELEASE(dsa2);
    RELEASE(dsb2);

    return TRUE;
}

BOOL TestDirectSoundSetCooperativeLevel(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    HWND wa = InitWindow(WINDOW_NAME);
    HWND wb = InitWindow(WINDOW_NAME);
    HWND wc = InitWindow(WINDOW_NAME);

    if (wa == NULL || wb == NULL || wc == NULL) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundSetCooperativeLevelInvalidParams(dsca, wa, dscb, wb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundSetCooperativeLevelValidParams(dsca, wa, dscb, wb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundSetCooperativeLevelAlreadySet(dsca, wa, dscb, wb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundSetCooperativeLevelChangeWindow(dsca, wa, dscb, wb, wc)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundSetCooperativeLevelMultipleInstances(dsca, wa, dscb, wb)) {
        result = FALSE;
        goto exit;
    }

exit:
    if (wa != NULL) {
        DestroyWindow(wa);
    }

    if (wb != NULL) {
        DestroyWindow(wb);
    }

    if (wb != NULL) {
        DestroyWindow(wc);
    }

    UnregisterClassA(WINDOW_NAME, GetModuleHandleA(NULL));

    return result;
}
