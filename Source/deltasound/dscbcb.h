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

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WdsblcANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WdsblcANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "allocator.h"

#define DSCBCB_READ_NONE             0
#define DSCBCB_READ_LOOPING          1

#define DSCBCB_SETPOSITION_NONE      0
#define DSCBCB_SETPOSITION_LOOPING   1

typedef struct dscbcb dscbcb;

HRESULT DELTACALL dscbcb_create(allocator* pAlloc, DWORD dwBytes, dscbcb** ppOut);
VOID DELTACALL dscbcb_release(dscbcb* pBuffer);

HRESULT DELTACALL dscbcb_get_current_position(dscbcb* pBuffer,
    LPDWORD pdwCaptureBytes, LPDWORD pdwReadBytes);
HRESULT DELTACALL dscbcb_set_current_position(dscbcb* pBuffer,
    DWORD dwCaptureBytes, DWORD dwReadBytes, DWORD dwFlags);

HRESULT DELTACALL dscbcb_get_length(dscbcb* pBuffer, LPDWORD pdwBytes);
HRESULT DELTACALL dscbcb_get_lockable_length(dscbcb* pBuffer, LPDWORD pdwBytes);

HRESULT DELTACALL dscbcb_lock(dscbcb* pBuffer, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2);
HRESULT DELTACALL dscbcb_unlock(dscbcb* pBuffer, LPVOID pvAudioPtr1, LPVOID pvAudioPtr2);
HRESULT DELTACALL dscbcb_read(dscbcb* pBuffer, DWORD dwBytes, LPVOID pData, LPDWORD pdwBytes, DWORD dwFlags);
