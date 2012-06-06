/*

Copyright (c) 2010 Brook Miles

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <windows.h>

#ifndef StringConv_h__
#define StringConv_h__

#pragma once


std::string  WideToMB(const std::wstring& str, UINT codePage = CP_ACP);
std::wstring MBToWide(const std::string& str,  UINT codePage = CP_ACP);

inline std::string  WideToUTF8(const std::wstring& str) { return WideToMB(str, CP_UTF8); }
inline std::wstring UTF8ToWide(const std::string& str)  { return MBToWide(str, CP_UTF8); }

inline std::string ANSIToUTF8(const std::string& str, UINT codePage = CP_ACP) { return WideToUTF8(MBToWide(str, codePage)); }
inline std::string UTF8ToANSI(const std::string& str, UINT codePage = CP_ACP) { return WideToMB(UTF8ToWide(str), codePage); }

#ifdef _UNICODE
#define TCHARToUTF8 WideToUTF8
#define UTF8ToTCHAR UTF8ToWide
#define TCHARToWide
#define WideToTCHAR
#define TCHARToMB WideToMB 
#define MBToTCHAR MBToWide
#else
#define TCHARToUTF8 ANSIToUTF8
#define UTF8ToTCHAR UTF8ToANSI
#define TCHARToWide MBToWide
#define WideToTCHAR WideToMB
#define TCHARToMB
#define MBToTCHAR
#endif

#endif // StringConv_h__
