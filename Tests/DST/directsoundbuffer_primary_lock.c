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

#include "directsoundbuffer_primary_get.h"
#include "wnd.h"

#include <dsound.h>

#define WINDOW_NAME "DirectSound Primary Buffer Get Properties"

typedef HRESULT(WINAPI* LPDIRECTSOUNDCREATE)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);

static BOOL TestDirectSoundBufferInvalidLocks(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    HRESULT ra = IDirectSoundBuffer_GetCaps(a, &capsa);
    HRESULT rb = IDirectSoundBuffer_GetCaps(b, &capsb);

    if (ra != rb) {
        return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        return FALSE;
    }

    // Lock
    {
        ra = IDirectSoundBuffer_Lock(a, 0, 0, NULL, NULL, NULL, NULL, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, 0, NULL, NULL, NULL, NULL, 0);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        ra = IDirectSoundBuffer_Lock(a, cwa, 0, NULL, NULL, NULL, NULL, 0);
        rb = IDirectSoundBuffer_Lock(b, cwb, 0, NULL, NULL, NULL, NULL, 0);

        if (ra != rb) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, NULL, NULL, NULL, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, NULL, NULL, NULL, 0);

        if (ra != rb) {
            return FALSE;
        }

        if (a11 != a21) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, &al11, NULL, NULL, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, &al12, NULL, NULL, 0);

        if (ra != rb) {
            return FALSE;
        }

        if (a11 != a21 || al11 != al12) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, &al11, &a12, NULL, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, &al12, &a22, NULL, 0);

        if (ra != rb) {
            return FALSE;
        }

        if (a11 != a21 || al11 != al12 || a12 != a22) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if (a11 != a21 || al11 != al12 || a12 != a22 || al21 != al22) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, capsa.dwBufferBytes + 1, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, capsb.dwBufferBytes + 1, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 11111, 0, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 11111, 0, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, capsa.dwBufferBytes, 0, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, capsb.dwBufferBytes, 0, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, capsa.dwBufferBytes + 1, 0, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, capsb.dwBufferBytes + 1, 0, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, capsa.dwBufferBytes + 1, capsa.dwBufferBytes, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, capsb.dwBufferBytes + 1, capsb.dwBufferBytes, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, capsa.dwBufferBytes, capsa.dwBufferBytes + 1, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, capsb.dwBufferBytes, capsb.dwBufferBytes + 1, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }
    }

    // DSBLOCK_FROMWRITECURSOR

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, &al11, &a12, &al21, DSBLOCK_FROMWRITECURSOR);
        rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, &al12, &a22, &al22, DSBLOCK_FROMWRITECURSOR);

        if (ra != rb || ra == S_OK) {
            return FALSE;
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, capsa.dwBufferBytes + 1, &a11, &al11, &a12, &al21, DSBLOCK_FROMWRITECURSOR);
        rb = IDirectSoundBuffer_Lock(b, 0, capsb.dwBufferBytes + 1, &a21, &al12, &a22, &al22, DSBLOCK_FROMWRITECURSOR);

        if (ra != rb || ra == S_OK) {
            return FALSE;
        }
    }

    // DSBLOCK_ENTIREBUFFER



    return TRUE;
}

static BOOL TestDirectSoundBufferUnlockable(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
    DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

    HRESULT ra = IDirectSoundBuffer_Lock(a, 12000, 28000, &a11, &al11, NULL, &al21, 0);
    HRESULT rb = IDirectSoundBuffer_Lock(b, 12000, 28000, &a21, &al12, NULL, &al22, 0);

    if (ra != rb) {
        return FALSE;
    }

    if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
        return FALSE;
    }

    if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
        return FALSE;
    }

    if (al11 != al12 || al21 != al22) {
        return FALSE;
    }

    if (ra == S_OK || rb == S_OK) {
        ra = IDirectSoundBuffer_Unlock(a, a11, al11, NULL, al21);
        rb = IDirectSoundBuffer_Unlock(b, a21, al12, NULL, al22);

        if (ra != rb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferValidLocks(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    HRESULT ra = IDirectSoundBuffer_GetCaps(a, &capsa);
    HRESULT rb = IDirectSoundBuffer_GetCaps(b, &capsb);

    if (ra != rb) {
        return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        return FALSE;
    }

    // Lock

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, cwa, 10111, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, cwb, 10111, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 10111, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, 10111, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, capsa.dwBufferBytes, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 0, capsb.dwBufferBytes, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 11111, capsa.dwBufferBytes - 12000, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 11111, capsa.dwBufferBytes - 12000, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 12000, 20767, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 12000, 20767, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 12000, 20768, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 12000, 20768, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 12000, 28000, &a11, &al11, &a12, NULL, 0);
        rb = IDirectSoundBuffer_Lock(b, 12000, 28000, &a21, &al12, &a22, NULL, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if ((size_t)a11 - (size_t)a12 != (size_t)a21 - (size_t)a22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 12000, 28000, &a11, &al11, &a12, &al21, 0);
        rb = IDirectSoundBuffer_Lock(b, 12000, 28000, &a21, &al12, &a22, &al22, 0);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if ((size_t)a11 - (size_t)a12 != (size_t)a21 - (size_t)a22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }
    
    // DSBLOCK_FROMWRITECURSOR

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 12000, &a11, &al11, &a12, &al21, DSBLOCK_FROMWRITECURSOR);
        rb = IDirectSoundBuffer_Lock(b, 0, 12000, &a21, &al12, &a22, &al22, DSBLOCK_FROMWRITECURSOR);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, capsa.dwBufferBytes, &a11, &al11, &a12, &al21, DSBLOCK_FROMWRITECURSOR);
        rb = IDirectSoundBuffer_Lock(b, 0, capsb.dwBufferBytes, &a21, &al12, &a22, &al22, DSBLOCK_FROMWRITECURSOR);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }



    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, capsa.dwBufferBytes + 1, capsa.dwBufferBytes, &a11, &al11, &a12, &al21, DSBLOCK_FROMWRITECURSOR);
        rb = IDirectSoundBuffer_Lock(b, capsb.dwBufferBytes + 1, capsb.dwBufferBytes, &a21, &al12, &a22, &al22, DSBLOCK_FROMWRITECURSOR);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    // DSBLOCK_ENTIREBUFFER

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, 12000, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
        rb = IDirectSoundBuffer_Lock(b, 0, 12000, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 0, capsa.dwBufferBytes + 1, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
        rb = IDirectSoundBuffer_Lock(b, 0, capsb.dwBufferBytes + 1, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a, 15000, capsa.dwBufferBytes + 1, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
        rb = IDirectSoundBuffer_Lock(b, 15000, capsb.dwBufferBytes + 1, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

        if (ra != rb) {
            return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            return FALSE;
        }

        if (al11 != al12 || al21 != al22) {
            return FALSE;
        }

        if ((size_t)a11 - (size_t)a12 != (size_t)a21 - (size_t)a22) {
            return FALSE;
        }

        if (ra == S_OK || rb == S_OK) {
            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferDuplicateLocks(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    HRESULT ra = IDirectSoundBuffer_GetCaps(a, &capsa);
    HRESULT rb = IDirectSoundBuffer_GetCaps(b, &capsb);

    if (ra != rb) {
        return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        return FALSE;
    }

    // Lock
    {
        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        const HRESULT ra1 = IDirectSoundBuffer_Lock(a, 0, 10111, &a11, &al11, &a12, &al21, 0);
        const HRESULT rb1 = IDirectSoundBuffer_Lock(b, 0, 10111, &a21, &al12, &a22, &al22, 0);

        if (ra1 != rb1) {
            return FALSE;
        }

        // Lock with the same offset and length
        if (ra1 == S_OK || rb1 == S_OK) {
            // NOTE. Cannot lock the same bytes multiple times.
            // Expected result: E_INVALIDARG

            const HRESULT ra2 = IDirectSoundBuffer_Lock(a, 0, 10111, &a11, &al11, &a12, &al21, 0);
            const HRESULT rb2 = IDirectSoundBuffer_Lock(b, 0, 10111, &a21, &al12, &a22, &al22, 0);

            if (ra2 != rb2) {
                return FALSE;
            }

            ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
            rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

            if (ra != rb) {
                return FALSE;
            }

            if (ra1 == ra2 || rb1 == rb2) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferPrimaryLockDetails(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD flags, DWORD level) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, level);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, level);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL && dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (level == 4 && flags == 1) {
        int kkk = 1; // TODO
    }

    if (!TestDirectSoundBufferInvalidLocks(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferValidLocks(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferDuplicateLocks(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

    // todo lock of overlapping offset and length

    // TODO Unlock tests

    //if (!TestDirectSoundBufferUnlockable(dsba, dsbb)) {
    //    result = FALSE;
    //    goto exit;
    //}

exit:

    if (dsba != NULL) {
        IDirectSoundBuffer_Release(dsba);
    }

    if (dsbb != NULL) {
        IDirectSoundBuffer_Release(dsbb);
    }

    if (dsa != NULL) {
        IDirectSound_Release(dsa);
    }

    if (dsb != NULL) {
        IDirectSound_Release(dsb);
    }

    return result;
}

#define COOPERATIVE_LEVEL_COUNT 4

static const DWORD CooperativeLevels[COOPERATIVE_LEVEL_COUNT] = {
    DSSCL_NORMAL, DSSCL_PRIORITY, DSSCL_EXCLUSIVE, DSSCL_WRITEPRIMARY
};

#define BUFFER_FLAG_COUNT       7

static const DWORD BufferFlags[BUFFER_FLAG_COUNT] =
{
    DSBCAPS_PRIMARYBUFFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2
};

BOOL TestDirectSoundBufferPrimaryLock(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = (LPDIRECTSOUNDCREATE)GetProcAddress(a, "DirectSoundCreate");
    LPDIRECTSOUNDCREATE dscb = (LPDIRECTSOUNDCREATE)GetProcAddress(b, "DirectSoundCreate");

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    HWND wa = InitWindow(WINDOW_NAME);
    HWND wb = InitWindow(WINDOW_NAME);

    if (wa == NULL || wb == NULL) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < COOPERATIVE_LEVEL_COUNT; i++) {
        for (int k = 0; k < BUFFER_FLAG_COUNT; k++) {
            if (!TestDirectSoundBufferPrimaryLockDetails(dsca, wa, dscb, wb, BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                goto exit;
            }
        }
    }

exit:
    if (wa != NULL) {
        DestroyWindow(wa);
    }

    if (wb != NULL) {
        DestroyWindow(wb);
    }

    UnregisterClassA(WINDOW_NAME, GetModuleHandleA(NULL));

    return result;
}
