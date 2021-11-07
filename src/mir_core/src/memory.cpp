/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#define BLOCK_ALLOCED 0xABBABABA
#define BLOCK_FREED   0xDEADBEEF

static int CheckBlock(void* blk)
{
	int result = FALSE;
	char* p = (char*)blk - sizeof(DWORD)*2;
	DWORD size, *b, *e;

	__try
	{
		size = *(DWORD*)p;
		b = (DWORD*)&p[ sizeof(DWORD) ];
		e = (DWORD*)&p[ sizeof(DWORD)*2 + size ];

		if (*b != BLOCK_ALLOCED || *e != BLOCK_ALLOCED)
		{
         #ifdef _MSC_VER
            if (*b == BLOCK_FREED && *e == BLOCK_FREED)
               OutputDebugStringA("memory block is already deleted\n");
            else
               OutputDebugStringA("memory block is corrupted\n");
            #if defined(_DEBUG)
               DebugBreak();
            #endif
			#endif
		}
 		else result = TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
      #ifdef _MSC_VER
         OutputDebugStringA("access violation during checking memory block\n");
         #if defined(_DEBUG)
            DebugBreak();
         #endif
      #endif
	}

	return result;
}

/******************************************************************************/

MIR_C_CORE_DLL(void*) mir_alloc(size_t size)
{
	if (size == 0)
		return nullptr;

	char *p = (char*)malloc(size + sizeof(DWORD)* 3);
	if (p == nullptr) {
      #ifdef _MSC_VER
         OutputDebugStringA("memory overflow\n");
         #if defined(_DEBUG)
            DebugBreak();
         #endif
      #endif
		return nullptr;
	}

	*(DWORD*)p = (DWORD)size;
	*(DWORD*)&p[sizeof(DWORD)] = BLOCK_ALLOCED;
	*(DWORD*)&p[size + sizeof(DWORD)*2] = BLOCK_ALLOCED;
	return p + sizeof(DWORD)* 2;
}

/******************************************************************************/

MIR_C_CORE_DLL(void*) mir_calloc(size_t size)
{
	void* p = mir_alloc(size);
	if (p != nullptr)
		memset(p, 0, size);
	return p;
}

/******************************************************************************/

MIR_C_CORE_DLL(void*) mir_realloc(void* ptr, size_t size)
{
	char *p;

	if (ptr != nullptr) {
		if (!CheckBlock(ptr))
			return nullptr;
		p = (char*)ptr - sizeof(DWORD)*2;
	}
	else p = nullptr;

	p = (char*)realloc(p, size + sizeof(DWORD)*3);
	if (p == nullptr) {
      #ifdef _MSC_VER
         OutputDebugStringA("memory overflow\n");
         #if defined(_DEBUG)
            DebugBreak();
         #endif
      #endif
		return nullptr;
	}

	*(DWORD*)p = (DWORD)size;
	*(DWORD*)&p[sizeof(DWORD)] = BLOCK_ALLOCED;
	*(DWORD*)&p[size + sizeof(DWORD)*2] = BLOCK_ALLOCED;
	return p + sizeof(DWORD)*2;
}

/******************************************************************************/

MIR_C_CORE_DLL(void) mir_free(void* ptr)
{
	char* p;
	DWORD size;

	if (ptr == nullptr)
		return;
	if (!CheckBlock(ptr))
		return;

	p = (char*)ptr - sizeof(DWORD)*2;
	size = *(DWORD*)p;

	*(DWORD*)&p[sizeof(DWORD)] = BLOCK_FREED;
	*(DWORD*)&p[size + sizeof(DWORD)*2] = BLOCK_FREED;
	free(p);
}

/******************************************************************************/

MIR_CORE_DLL(char*) mir_strdup(const char *str)
{
	if (str == nullptr)
		return nullptr;

	char *p = (char*)mir_alloc(strlen(str)+1);
	if (p)
		strcpy(p, str);
	return p;
}

MIR_CORE_DLL(wchar_t*) mir_wstrdup(const wchar_t *str)
{
	if (str == nullptr)
		return nullptr;

	wchar_t *p = (wchar_t*)mir_alloc(sizeof(wchar_t)*(wcslen(str)+1));
	if (p)
		wcscpy(p, str);
	return p;
}

/******************************************************************************/

MIR_CORE_DLL(char*) mir_strndup(const char *str, size_t len)
{
	if (str == nullptr || len == 0)
		return nullptr;

	char *p = (char*)mir_alloc(len+1);
	if (p) {
		memcpy(p, str, len);
		p[len] = 0;
	}
	return p;
}

MIR_CORE_DLL(wchar_t*) mir_wstrndup(const wchar_t *str, size_t len)
{
	if (str == nullptr || len == 0)
		return nullptr;

	wchar_t *p = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len+1));
	if (p) {
		memcpy(p, str, sizeof(wchar_t)*len);
		p[len] = 0;
	}
	return p;
}

/******************************************************************************/

MIR_CORE_DLL(int) mir_snprintf(char *buffer, size_t count, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int len = _vsnprintf(buffer, count-1, fmt, va);
	va_end(va);
	buffer[count-1] = 0;
	return len;
}

/******************************************************************************/

MIR_CORE_DLL(int) mir_snwprintf(wchar_t *buffer, size_t count, const wchar_t* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int len = _vsnwprintf(buffer, count-1, fmt, va);
	va_end(va);
	buffer[count-1] = 0;
	return len;
}

/******************************************************************************/

MIR_CORE_DLL(int) mir_vsnprintf(char *buffer, size_t count, const char* fmt, va_list va)
{
	int len = _vsnprintf(buffer, count-1, fmt, va);
	buffer[count-1] = 0;
	return len;
}

/******************************************************************************/

MIR_CORE_DLL(int) mir_vsnwprintf(wchar_t *buffer, size_t count, const wchar_t* fmt, va_list va)
{
	int len = _vsnwprintf(buffer, count-1, fmt, va);
	buffer[count-1] = 0;
	return len;
}

/******************************************************************************/

#ifdef _MSC_VER
MIR_CORE_DLL(wchar_t*) mir_a2u_cp(const char* src, int codepage)
{
	if (src == nullptr)
		return nullptr;

	int cbLen = MultiByteToWideChar(codepage, 0, src, -1, nullptr, 0);
	wchar_t* result = (wchar_t*)mir_alloc(sizeof(wchar_t)*(cbLen+1));
	if (result == nullptr)
		return nullptr;

	MultiByteToWideChar(codepage, 0, src, -1, result, cbLen);
	result[cbLen] = 0;
	return result;
}

/******************************************************************************/

MIR_CORE_DLL(wchar_t*) mir_a2u(const char* src)
{
	return mir_a2u_cp(src, Langpack_GetDefaultCodePage());
}

/******************************************************************************/

MIR_CORE_DLL(char*) mir_u2a_cp(const wchar_t* src, int codepage)
{
	if (src == nullptr)
		return nullptr;

	int cbLen = WideCharToMultiByte(codepage, 0, src, -1, nullptr, 0, nullptr, nullptr);
	char* result = (char*)mir_alloc(cbLen+1);
	if (result == nullptr)
		return nullptr;

	WideCharToMultiByte(codepage, 0, src, -1, result, cbLen, nullptr, nullptr);
	result[cbLen] = 0;
	return result;
}

/******************************************************************************/

MIR_CORE_DLL(char*) mir_u2a(const wchar_t* src)
{
	return mir_u2a_cp(src, Langpack_GetDefaultCodePage());
}
#endif
