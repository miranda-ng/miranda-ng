/*
Miranda SmileyAdd Plugin
Copyright (C) 2008 - 2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GNUC__
#	ifdef _DEBUG
#		define _CRTDBG_MAP_ALLOC
#		include <stdlib.h>
#		include <crtdbg.h>
#  else
#		include <stdlib.h>
#  endif
#endif

#include <stdio.h>
#include <stdarg.h>

#include "bkstring.h"


bkstring::~bkstring() { if (sizeAlloced) free(buf); }

void bkstring::reserve(size_type len)
{
	if (len >= sizeAlloced || sizeAlloced == 0)
	{
		if (sizeAlloced == 0) buf = NULL;
		buf = (value_type*)realloc(buf, (len+1) * sizeof(value_type));
		if (sizeAlloced == 0) buf[0] = 0;
		sizeAlloced = len+1;
	}
}

void bkstring::appendfmt(const value_type *fmt, ...) 
{
	areserve(_tcslen(fmt)*2);

	va_list vararg;
	va_start(vararg, fmt);
	for (;;) 
	{
		int len = _vsntprintf(buf + lenBuf, sizeAlloced - lenBuf - 1, fmt, vararg);
		if (len < 0)
			reserve(sizeAlloced + 256);
		else
		{
			lenBuf += len;
			buf[lenBuf] = 0;
			break;
		}
	}
	va_end(vararg);
}

bkstring& bkstring::append(const value_type* _Ptr)
{
	size_type len = _tcslen(_Ptr);
	areserve(len);
	memcpy(buf+lenBuf, _Ptr, (len+1)*sizeof(value_type));
	lenBuf += len;
	return *this;
}

bkstring& bkstring::append(const value_type* _Ptr, size_type _Count)
{
	size_type len = min(_tcslen(_Ptr), _Count);
	areserve(len);
	memcpy(buf+lenBuf, _Ptr, len*sizeof(value_type));
	lenBuf += len;
	buf[lenBuf] = 0;
	return *this;
}

bkstring& bkstring::append(const bkstring& _Str, size_type _Off, size_type _Count)
{
	size_type len = min(_Count, _Str.size() - _Off);
	areserve(len);
	memcpy(buf+lenBuf, _Str.c_str()+_Off, len*sizeof(value_type));
	lenBuf += len;
	buf[lenBuf] = 0;
	return *this;
}

bkstring& bkstring::append(const bkstring& _Str)
{
	size_type len = _Str.size();
	areserve(len);
	memcpy(buf+lenBuf, _Str.c_str(), len*sizeof(value_type));
	lenBuf += len;
	buf[lenBuf] = 0;
	return *this;
}

bkstring& bkstring::append(size_type _Count, value_type _Ch)
{
	areserve(_Count);
	for(size_type i=0; i<_Count; ++i) buf[lenBuf+i] = _Ch;
	lenBuf += _Count;
	buf[lenBuf] = 0;
	return *this;
}


bkstring& bkstring::assign(const value_type* _Ptr, size_type _Count)
{
	if (_Count == 0 && sizeAlloced == 0)
	{
		buf = (TCHAR*)_T("");
	}
	else
	{
		reserve(_Count);
		memcpy(buf, _Ptr, _Count*sizeof(value_type));
		buf[_Count] = 0;
		lenBuf = _Count;
	}
	return *this;
}

bkstring& bkstring::assign(const bkstring& _Str, size_type _Off, size_type _Count)
{
	size_type len = min(_Count, _Str.size() - _Off);
	if (len == 0 && sizeAlloced == 0)
	{
		buf = (TCHAR*)_T("");
	}
	else
	{
		reserve(len);
		memcpy(buf, _Str.c_str() + _Off, len*sizeof(value_type));
		lenBuf = len;
		buf[len] = 0;
	}
	return *this;
}

bkstring& bkstring::assign(size_type _Count, value_type _Ch)
{
	reserve(_Count);
	for(size_type i=0; i<_Count; ++i) buf[i] = _Ch;
	buf[_Count] = 0;
	lenBuf = _Count;
	return *this;
}

bkstring::size_type bkstring::find(value_type _Ch, size_type _Off) const
{
	for (size_type i=_Off; i<=lenBuf; ++i)
		if (buf[i] == _Ch) return i;
	return (size_type)npos;
}

bkstring::size_type bkstring::find(const value_type* _Ptr, size_type _Off) const
{
	if (_Off > lenBuf) return (size_type)npos;

	value_type* pstr = _tcsstr(buf+_Off, _Ptr);
	return pstr ? pstr - buf : npos;
}

bkstring::size_type bkstring::find_last_of(value_type _Ch, size_type _Off) const
{
	for (size_type i=(_Off == npos ? lenBuf : _Off); i--;)
		if (buf[i] == _Ch) return i;
	return (size_type)npos;
}

bkstring& bkstring::insert(size_type _P0, const value_type* _Ptr, size_type _Count)
{
	size_type len = _tcslen(_Ptr);
	if (_Count < len) len = _Count;
	areserve(len);
	value_type *p = buf + _P0;
	memmove(p+len, p, (lenBuf-_P0+1)*sizeof(value_type));
	memcpy(p, _Ptr, _Count*sizeof(value_type));
	lenBuf += len;
	return *this;
}

bkstring& bkstring::insert(size_type _P0, size_type _Count, value_type _Ch)
{
	areserve(_Count);
	value_type *p = buf + _P0;
	memmove(p+_Count, p, (lenBuf-_P0+1)*sizeof(value_type));
	for(size_type i=0; i<_Count; ++i) p[i] = _Ch;
	lenBuf += _Count;
	return *this;
}

bkstring& bkstring::erase(size_type _Pos, size_type _Count)
{
	if (_Pos < lenBuf)
	{
		const size_type len = min(lenBuf - _Pos, _Count);
		value_type *p = buf + _Pos;
		lenBuf -= len;
		memmove(p, p+len, (lenBuf - _Pos)*sizeof(value_type));
		buf[lenBuf] = 0;
	}
	return *this;
}
