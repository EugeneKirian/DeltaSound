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
#include "convertor.h"

struct convertor {
    allocator*  Allocator;
    arena*      Arena;
};

HRESULT DELTACALL convertor_create(allocator* pAlloc, convertor** ppOut) {
    if (pAlloc == NULL || ppOut == NULL) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    convertor* instance = NULL;

    if (SUCCEEDED(hr = allocator_allocate(pAlloc, sizeof(convertor), &instance))) {
        instance->Allocator = pAlloc;

        if (SUCCEEDED(hr = arena_create(pAlloc, &instance->Arena))) {

            *ppOut = instance;

            return S_OK;
        }

        allocator_free(pAlloc, instance);
    }

    return hr;
}

VOID DELTACALL convertor_release(convertor* self) {
    if (self == NULL) { return; }

    arena_release(self->Arena);

    allocator_free(self->Allocator, self);
}
