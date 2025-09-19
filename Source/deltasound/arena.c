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

#include "arena.h"
#include "arr.h"

#define DEFAULT_BLOCK_SIZE      (256 * 1024)

// TODO align memory addresses
// TODO Both allocations, and offsets!

typedef struct block {
    allocator*  Allocator;
    LPVOID      Block;
    DWORD       Size;
    DWORD       Capacity;
} block;

typedef struct arena {
    allocator*          Allocator;
    CRITICAL_SECTION    Lock;
    arr*                Blocks;
} arena;

HRESULT DELTACALL block_create(allocator* pAlloc, DWORD dwBytes, block** ppOut);
VOID DELTACALL block_release(block* pBlock);

HRESULT DELTACALL arena_create(allocator* pAlloc, arena** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    arena* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(arena), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = arr_create(pAlloc, &instance->Blocks))) {
            InitializeCriticalSection(&instance->Lock);

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL arena_release(arena* self) {
    if (self == NULL) { return; }

    DeleteCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Blocks);

    for (DWORD i = 0; i < count; i++) {
        block* region = NULL;

        if (SUCCEEDED(arr_get_item(self->Blocks, i, &region))) {
            block_release(region);
        }
    }

    allocator_free(self->Allocator, self->Blocks);
    allocator_free(self->Allocator, self);
}

HRESULT DELTACALL arena_allocate(arena* self, DWORD dwBytes, LPVOID* ppMem) {
    if (self == NULL) {
        return E_POINTER;
    }

    if (ppMem == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    block* region = NULL;

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Blocks);

    for (DWORD i = 0; i < count; i++) {
        if (SUCCEEDED(hr = arr_get_item(self->Blocks, i, &region))) {
            if (dwBytes < region->Capacity - region->Size) {
                region->Size += dwBytes;

                *ppMem = (LPVOID)((size_t)region->Block + region->Size);

                goto exit;
            }
        }
    }

    if (SUCCEEDED(hr = block_create(self->Allocator, max(dwBytes, DEFAULT_BLOCK_SIZE), &region))) {
        region->Size = dwBytes;

        if (SUCCEEDED(hr = arr_add_item(self->Blocks, region))) {
            *ppMem = region->Block;
            goto exit;
        }

        block_release(region);
    }

exit:

    LeaveCriticalSection(&self->Lock);

    return hr;
}

HRESULT DELTACALL arena_clear(arena* self) {
    if (self == NULL) {
        return E_POINTER;
    }

    EnterCriticalSection(&self->Lock);

    const DWORD count = arr_get_count(self->Blocks);

    for (DWORD i = 0; i < count; i++) {
        block* region = NULL;

        if (SUCCEEDED(arr_get_item(self->Blocks, i, &region))) {
            region->Size = 0;
        }
    }

    LeaveCriticalSection(&self->Lock);

    return S_OK;
}

/* ---------------------------------------------------------------------- */

HRESULT DELTACALL block_create(allocator* pAlloc, DWORD dwBytes, block** ppOut) {
    if (pAlloc == NULL || dwBytes == 0 || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    block* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(block), &instance))) {
        if (SUCCEEDED(hr = allocator_allocate(pAlloc, dwBytes, &instance->Block))) {
            instance->Allocator = pAlloc;
            instance->Capacity = dwBytes;

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL block_release(block* self) {
    if (self == NULL) { return; }

    allocator_free(self->Allocator, self->Block);
    allocator_free(self->Allocator, self);
}
