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

#define DSBCB_READ_NONE             0
#define DSBCB_READ_LOOPING          1

#define DSBCB_SETPOSITION_NONE      0
#define DSBCB_SETPOSITION_LOOPING   1

typedef struct dsbcb dsbcb;

HRESULT DELTACALL dsbcb_create(allocator* pAlloc, DWORD dwBytes, dsbcb** ppOut);
VOID DELTACALL dsbcb_release(dsbcb* pBuffer);

HRESULT DELTACALL dsbcb_duplicate(dsbcb* pBuffer, dsbcb** ppOut);

HRESULT DELTACALL dsbcb_get_current_position(dsbcb* pBuffer,
    LPDWORD pdwReadBytes, LPDWORD pdwWriteBytes);
HRESULT DELTACALL dsbcb_set_current_position(dsbcb* pBuffer,
    DWORD dwReadBytes, DWORD dwWriteBytes, DWORD dwFlags);

HRESULT DELTACALL dsbcb_get_length(dsbcb* pBuffer, LPDWORD pdwBytes);
HRESULT DELTACALL dsbcb_get_lockable_length(dsbcb* pBuffer, LPDWORD pdwBytes);

HRESULT DELTACALL dsbcb_lock(dsbcb* pBuffer, DWORD dwOffset, DWORD dwBytes,
    LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2);
HRESULT DELTACALL dsbcb_unlock(dsbcb* pBuffer, LPVOID pvAudioPtr1, LPVOID pvAudioPtr2);
HRESULT DELTACALL dsbcb_read(dsbcb* pBuffer, DWORD dwBytes,
    LPVOID pData, LPDWORD pdwBytesRead, DWORD dwFlags);
