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

#ifndef _STRINGUTIL_H_INCLUDED_
#define _STRINGUTIL_H_INCLUDED_

void Split(const tstring& str, std::vector<tstring>& out, TCHAR sep, bool includeEmpty);
tstring GetWord(const tstring& str, unsigned index, bool getRest = false);

std::string& replaceAll(std::string& context, const std::string& from, const std::string& to);

inline std::string WideToUTF8(const std::wstring& str)
{
	return (char*)ptrA(mir_utf8encodeW(str.c_str()));
}

inline std::wstring UTF8ToWide(const std::string& str)
{
	return (wchar_t*)ptrW(mir_utf8decodeW(str.c_str()));
}

inline bool Compare(const tstring& one, const tstring& two, bool caseSensitive)
{
	return caseSensitive ? (one == two) : (_tcsicmp(one.c_str(), two.c_str()) == 0);
}

#endif//_STRINGUTIL_H_INCLUDED_
