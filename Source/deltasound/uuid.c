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

#pragma once

#include "uuid.h"

const CLSID CLSID_DirectSoundPrivate =
{ 0x11AB3EC0, 0x25EC, 0x11D1, { 0xA4, 0xD8, 0x00, 0xC0, 0x4F, 0xC2, 0x8A, 0xCA } };

const CLSID CLSID_IMMDeviceEnumerator =
{ 0xBCDE0395, 0xE52F, 0x467C, { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };

const IID IID_IAudioClient =
{ 0x1CB9AD4C, 0xDBFA, 0x4C32, { 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2 } };

const IID IID_IAudioRenderClient =
{ 0xF294ACFC, 0x3146, 0x4483, { 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2 } };

const IID IID_IAudioStreamVolume =
{ 0x93014887, 0x242D, 0x4068, { 0x8A, 0x15, 0xCF, 0x5E, 0x93, 0xB9, 0x0F, 0xE3 } };

const IID IID_IDirectSoundPrivate =
{ 0xd6E525AE, 0xB125, 0x4EC4, { 0xBE, 0x13, 0x12, 0x6D, 0x0C, 0xF7, 0xAF, 0xB6 } };

const IID IID_IMMDeviceEnumerator =
{ 0xA95664D2, 0x9614, 0x4F35, { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };

const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT =
{ 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

const GUID DSPROPSETID_DirectSoundDevice =
{ 0x84624F82, 0x25EC, 0x11D1, { 0xA4, 0xD8, 0x00, 0xC0, 0x4F, 0xC2, 0x8A, 0xCA } };
