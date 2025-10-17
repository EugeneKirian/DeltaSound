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

#include <windows.h>

#include <dsound.h>
#include <dsconf.h>
#include <vfwmsgs.h>

#define CINTERFACE
#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#define KSPROPERTY_SUPPORT_NONE 0

#define DELTACALL   __stdcall
#define CDECLCALL   __cdecl
#define UNUSED(X)   ((VOID)X)

#define IS_VALID_HANDLE(h)  (((h) != NULL) && ((h) != INVALID_HANDLE_VALUE))

#define AUDCLNT_BUFFERFLAGS_NONE                0

#define WASAPI_REFTIMES_PER_SEC                 10000000
#define WASAPI_10_MILLISECONDS                  (1.0f / 100.0f)

#define WASAPI_10_MILLISECONDS_TIME(FREQ)       (FREQ * WASAPI_10_MILLISECONDS)

#define ADVANCEPOSITION(X, FREQ, ALIGN)         (DWORD)(X + WASAPI_10_MILLISECONDS_TIME(FREQ) * ALIGN)

#define RELEASE(X) if ((X) != NULL) { (X)->lpVtbl->Release(X); (X) = NULL; }
#define RELEASEHANDLE(X) if((X)) { CloseHandle((X)); (X) = NULL; }
