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
#include <string>
#include "StringConv.h"

std::string WideToMB(const std::wstring& str, UINT codePage) 
{
	std::string ret;
	if(str.length() > 0)
	{
		DWORD mbChars = WideCharToMultiByte(codePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
		_ASSERTE(mbChars > 0);
		if(mbChars > 0)
		{
			char* buf = new char[mbChars];
			_ASSERTE( buf != NULL );
			if( buf != NULL )
			{
				ZeroMemory(buf, mbChars);

				DWORD charsConverted = WideCharToMultiByte(codePage, 0, str.c_str(), -1, buf, mbChars, NULL, NULL);
				_ASSERTE( charsConverted > 0 );
				_ASSERTE( charsConverted <= mbChars );

				buf[mbChars - 1] = 0;
				ret = buf;

				delete[] buf;
			}
		}
	}
	return ret;
}

std::wstring MBToWide(const std::string& str, UINT codePage) 
{
	std::wstring ret;
	if(str.length() > 0)
	{
		DWORD wChars = MultiByteToWideChar(codePage, 0, str.c_str(), -1, NULL, 0);
		_ASSERTE(wChars > 0);
		if(wChars > 0)
		{
			wchar_t* buf = new wchar_t[wChars];
			_ASSERTE( buf != NULL );
			if( buf != NULL )
			{
				size_t bytesNeeded = sizeof(wchar_t)*wChars;
				ZeroMemory(buf, bytesNeeded);

				DWORD charsConverted = MultiByteToWideChar(codePage, 0, str.c_str(), -1, buf, wChars);
				_ASSERTE( charsConverted > 0 );
				_ASSERTE( charsConverted <= wChars );

				buf[wChars - 1] = 0;
				ret = buf;

				delete[] buf;
			}
		}
	}
	return ret;
}

