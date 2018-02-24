/*
Copyright (C) 2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __UTF8_HELPERS_H__
# define __UTF8_HELPERS_H__

#include <windows.h>
#include <newpluginapi.h>
#include <m_system.h>


class TcharToUtf8
{
public:
	TcharToUtf8(const char *str) : utf8(nullptr)
	{
		int size = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
		if (size <= 0)
			throw L"Could not convert string to WCHAR";

		WCHAR *tmp = (WCHAR *) mir_alloc(size * sizeof(WCHAR));
		if (tmp == nullptr)
			throw L"mir_alloc returned NULL";

		MultiByteToWideChar(CP_ACP, 0, str, -1, tmp, size);

		init(tmp);

		mir_free(tmp);
	}


	TcharToUtf8(const WCHAR *str) : utf8(nullptr)
	{
		init(str);
	}


	~TcharToUtf8()
	{
		if (utf8 != nullptr)
			mir_free(utf8);
	}

	char *detach()
	{
		char *ret = utf8;
		utf8 = nullptr;
		return ret;
	}

	const char * get() const
	{
		return utf8;
	}

	operator const char *() const
	{
		return utf8;
	}

	const char & operator[](int pos) const
	{
		return utf8[pos];
	}

private:
	char *utf8;

	void init(const WCHAR *str)
	{
		int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 0)
			throw L"Could not convert string to UTF8";

		utf8 = (char *) mir_alloc(size);
		if (utf8 == nullptr)
			throw L"mir_alloc returned NULL";

		WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8, size, nullptr, nullptr);
	}
};



class Utf8ToTchar
{
public:
	Utf8ToTchar(const char *str) : tchar(nullptr)
	{
		if (str == nullptr)
			return;

		int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
		if (size <= 0)
			throw L"Could not convert string to WCHAR";

		WCHAR *tmp = (WCHAR *) mir_alloc(size * sizeof(WCHAR));
		if (tmp == nullptr)
			throw L"mir_alloc returned NULL";

		MultiByteToWideChar(CP_UTF8, 0, str, -1, tmp, size);

#ifdef UNICODE

		tchar = tmp;

#else

		size = WideCharToMultiByte(CP_ACP, 0, tmp, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 0)
		{
			mir_free(tmp);
			throw L"Could not convert string to ACP";
		}

		tchar = (wchar_t *) mir_alloc(size * sizeof(char));
		if (tchar == nullptr)
		{
			mir_free(tmp);
			throw L"mir_alloc returned NULL";
		}

		WideCharToMultiByte(CP_ACP, 0, tmp, -1, tchar, size, nullptr, nullptr);

		mir_free(tmp);

#endif
	}

	~Utf8ToTchar()
	{
		if (tchar != nullptr)
			mir_free(tchar);
	}

	wchar_t *detach()
	{
		wchar_t *ret = tchar;
		tchar = nullptr;
		return ret;
	}

	wchar_t * get() const
	{
		return tchar;
	}

	operator wchar_t *() const
	{
		return tchar;
	}

	wchar_t & operator[](int pos)
	{
		return tchar[pos];
	}

private:
	wchar_t *tchar;
};


class CharToTchar
{
public:
	CharToTchar(const char *str) : tchar(nullptr)
	{
		if (str == nullptr)
			return;

#ifdef UNICODE

		tchar = mir_a2u(str);

#else

		tchar = str;

#endif
	}


	~CharToTchar()
	{
#ifdef UNICODE
		if (tchar != nullptr)
			mir_free(tchar);
#endif
	}

	wchar_t *detach()
	{
#ifdef UNICODE
		wchar_t *ret = tchar;
#else
		wchar_t *ret = (tchar == nullptr ? nullptr : mir_strdup(tchar));
#endif

		tchar = nullptr;
		return ret;
	}

	const wchar_t * get() const
	{
		return tchar;
	}

	operator const wchar_t *() const
	{
		return tchar;
	}

	const wchar_t & operator[](int pos) const
	{
		return tchar[pos];
	}

private:
#ifdef UNICODE
	wchar_t *tchar;
#else
	const wchar_t *tchar;
#endif
};


class WcharToTchar
{
public:
	WcharToTchar(const WCHAR *str) : tchar(nullptr)
	{
		if (str == nullptr)
			return;

#ifdef UNICODE

		tchar = str;

#else

		tchar = mir_u2a(str);

#endif
	}


	~WcharToTchar()
	{
#ifndef UNICODE
		if (tchar != nullptr)
			mir_free(tchar);
#endif
	}

	wchar_t *detach()
	{
#ifdef UNICODE
		wchar_t *ret = (tchar == nullptr ? nullptr : mir_wstrdup(tchar));
#else
		wchar_t *ret = tchar;		
#endif

		tchar = nullptr;
		return ret;
	}

	const wchar_t * get() const
	{
		return tchar;
	}

	operator const wchar_t *() const
	{
		return tchar;
	}

	const wchar_t & operator[](int pos) const
	{
		return tchar[pos];
	}

private:
#ifdef UNICODE
	const wchar_t *tchar;
#else
	wchar_t *tchar;
#endif
};




class CharToWchar
{
public:
	CharToWchar(const char *str) : wchar(nullptr)
	{
		if (str == nullptr)
			return;

		wchar = mir_a2u(str);
	}


	~CharToWchar()
	{
		if (wchar != nullptr)
			mir_free(wchar);
	}

	WCHAR *detach()
	{
		WCHAR *ret = wchar;
		wchar = nullptr;
		return ret;
	}

	const WCHAR * get() const
	{
		return wchar;
	}

	operator const WCHAR *() const
	{
		return wchar;
	}

	const WCHAR & operator[](int pos) const
	{
		return wchar[pos];
	}

private:
	WCHAR *wchar;
};



class TcharToChar
{
public:
	TcharToChar(const wchar_t *str) : val(nullptr)
	{
		if (str == nullptr)
			return;

#ifdef UNICODE

		val = mir_u2a(str);

#else

		val = str;

#endif
	}


	~TcharToChar()
	{
#ifdef UNICODE
		if (val != nullptr)
			mir_free(val);
#endif
	}

	char *detach()
	{
#ifdef UNICODE
		char *ret = val;
#else
		char *ret = (val == nullptr ? nullptr : mir_strdup(val));
#endif

		val = nullptr;
		return ret;
	}

	const char * get() const
	{
		return val;
	}

	operator const char *() const
	{
		return val;
	}

	const char & operator[](int pos) const
	{
		return val[pos];
	}

private:
#ifdef UNICODE
	char *val;
#else
	const char *val;
#endif
};


class TcharToWchar
{
public:
	TcharToWchar(const wchar_t *str) : val(nullptr)
	{
		if (str == nullptr)
			return;

#ifdef UNICODE

		val = str;

#else

		val = mir_a2u(str);

#endif
	}


	~TcharToWchar()
	{
#ifndef UNICODE
		if (val != nullptr)
			mir_free(val);
#endif
	}

	WCHAR *detach()
	{
#ifdef UNICODE
		WCHAR *ret = (val == nullptr ? nullptr : mir_wstrdup(val));
#else
		WCHAR *ret = val;
#endif

		val = nullptr;
		return ret;
	}

	const WCHAR * get() const
	{
		return val;
	}

	operator const WCHAR *() const
	{
		return val;
	}

	const WCHAR & operator[](int pos) const
	{
		return val[pos];
	}

private:
#ifdef UNICODE
	const WCHAR *val;
#else
	WCHAR *val;
#endif
};




class BstrToTchar
{
public:
	BstrToTchar() : bstr(nullptr)
#ifndef UNICODE
		, tchar(NULL)
#endif
	{
	}

	BstrToTchar(const WCHAR *str) : bstr(nullptr)
#ifndef UNICODE
		, tchar(nullptr)
#endif
	{
		if (str == nullptr)
			return;

		bstr = SysAllocString(str);
	}

	BstrToTchar(const char *str) : bstr(nullptr)
#ifndef UNICODE
		, tchar(nullptr)
#endif
	{
		if (str == nullptr)
			return;

		bstr = SysAllocString(CharToWchar(str));
	}


	~BstrToTchar()
	{
		if (bstr != nullptr)
			SysFreeString(bstr);

#ifndef UNICODE
		freeTchar();
#endif
	}

	BSTR detach()
	{
		BSTR ret = bstr;
		bstr = nullptr;
		return ret;
	}

	operator const wchar_t *()
	{
#ifdef UNICODE

		return bstr;

#else

		if (tchar == nullptr)
			tchar = mir_u2a(bstr);

		return tchar;

#endif
	}

	operator const BSTR() const 
	{
		return bstr;
	}

	operator BSTR *()
	{
#ifndef UNICODE
		freeTchar();
#endif

		return &bstr;
	}

private:
	BSTR bstr;

#ifndef UNICODE

	wchar_t *tchar;

	void freeTchar()
	{
		if (tchar != nullptr)
		{
			mir_free(tchar);
			tchar = nullptr;
		}
	}

#endif
};


#endif // __UTF8_HELPERS_H__
