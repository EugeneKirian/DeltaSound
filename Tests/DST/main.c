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

#include "directsound_basics.h"
#include "directsound_create.h"
#include "directsound_createsoundbuffer_primary.h"
#include "directsound_createsoundbuffer_secondary.h"
#include "directsound_enumerate.h"
#include "directsound_getcaps.h"
#include "directsound_setcooperativelevel.h"

#include "directsoundbuffer_primary_basics.h"
#include "directsoundbuffer_primary_get.h"
#include "directsoundbuffer_primary_set.h"
#include "directsoundbuffer_primary_lock.h"
#include "directsoundbuffer_primary_play.h"
#include "directsoundbuffer_primary_play_pan.h"
#include "directsoundbuffer_primary_play_volume.h"

#include "directsoundbuffer_secondary_basics.h"
#include "directsoundbuffer_secondary_get.h"
#include "directsoundbuffer_secondary_set.h"
#include "directsoundbuffer_secondary_lock.h"

#include "directsoundcapture_enumerate.h"

#include "getdeviceid.h"

#include <stdio.h>

#define TEST(X)                                         \
    printf("%s\t", #X);                                 \
    printf("%s\r\n", Test##X(a, b) ? "OK" : "ERROR");

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage:\r\n\t%s\t<path to dsound.dll> <path to dsound.dll>\r\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (FAILED(CoInitialize(NULL))) {
        printf("Unable to initialize COM\r\n");
        return EXIT_FAILURE;
    }

    HMODULE a = NULL;
    HMODULE b = NULL;

    if ((a = LoadLibraryA(argv[1])) == NULL) {
        printf("Unable to load %s\r\n", argv[1]);
        goto exit;
    }

    if ((b = LoadLibraryA(argv[2])) == NULL) {
        printf("Unable to load %s\r\n", argv[2]);
        goto exit;
    }

    TEST(DirectSoundEnumerateA);
    TEST(DirectSoundEnumerateW);

    TEST(DirectSoundCaptureEnumerateA);
    TEST(DirectSoundCaptureEnumerateW);

    TEST(GetDeviceID);

    TEST(DirectSoundCreate);
    TEST(DirectSoundBasics);
  
    TEST(DirectSoundCreateSoundBufferPrimary);
    TEST(DirectSoundCreateSoundBufferSecondary);
    TEST(DirectSoundSetCooperativeLevel);
    TEST(DirectSoundGetCaps);

    TEST(DirectSoundBufferPrimaryBasics);
    TEST(DirectSoundBufferPrimaryGet);
    TEST(DirectSoundBufferPrimarySet);
    TEST(DirectSoundBufferPrimaryLock);
    TEST(DirectSoundBufferPrimaryPlay);
    TEST(DirectSoundBufferPrimaryPlayPan);
    TEST(DirectSoundBufferPrimaryPlayVolume);

    TEST(DirectSoundBufferSecondaryBasics);
    TEST(DirectSoundBufferSecondaryGet);
    TEST(DirectSoundBufferSecondarySet);
    TEST(DirectSoundBufferSecondaryLock);

exit:

    if (a != NULL) {
        FreeLibrary(a);
    }

    if (b != NULL) {
        FreeLibrary(b);
    }

    CoUninitialize();

    return EXIT_SUCCESS;
}
