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

#pragma once 

#include <tchar.h>
#include <string.h>

#ifndef min
#define min(A, B) ((A) < (B) ? (A) : (B)) 
#endif

class bkstring
{
public:
	typedef size_t size_type;
	typedef TCHAR  value_type;
	typedef value_type* iterator;
	typedef const value_type* const_iterator;

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	enum { npos = -1 };
#else
	static const size_type npos = size_type(-1);
#endif

private:
	value_type* buf;
	size_type sizeAlloced;
	size_type lenBuf;

	void areserve(size_type len) { reserve(lenBuf + len); }

public:

	explicit bkstring() : buf((TCHAR*)_T("")), sizeAlloced(0), lenBuf(0) 
	{}

	bkstring(const value_type* _Ptr, size_type _Count) : sizeAlloced(0), lenBuf(0)
	{ assign(_Ptr, _Count); }

	bkstring(const value_type* _Ptr) : sizeAlloced(0), lenBuf(0)
	{ assign(_Ptr); }

	bkstring(size_type _Count, value_type _Ch) : sizeAlloced(0), lenBuf(0)
	{ assign(_Count, _Ch); }

	bkstring(const bkstring& _Str) : sizeAlloced(0), lenBuf(0)
	{ assign(_Str); }

	bkstring(const bkstring& _Str, size_type _Off, size_type _Count) : sizeAlloced(0), lenBuf(0)
	{ assign(_Str, _Off, _Count); }

	~bkstring();

	size_type size(void) const { return lenBuf; }
	const value_type* c_str(void) const { return buf; }

	void clear(void) { if (lenBuf) { lenBuf = 0; buf[0] = 0; } }
	void insert(const value_type *txt);
	void reserve(size_type len);

	bkstring& assign(const value_type* _Ptr)
	{ return assign(_Ptr, _tcslen(_Ptr)); }

	bkstring& assign(const bkstring& _Str)
	{ return assign(_Str, 0, (size_type)npos); }

	bkstring& assign(const value_type* _Ptr, size_type _Count);
	bkstring& assign(const bkstring& _Str, size_type off, size_type _Count);
	bkstring& assign(size_type _Count, value_type _Ch);

	bkstring& append(const value_type* _Ptr);
	bkstring& append(const value_type* _Ptr, size_type _Count);
	bkstring& append(const bkstring& _Str, size_type _Off, size_type _Count);
	bkstring& append(const bkstring& _Str);
	bkstring& append(size_type _Count, value_type _Ch);

	int compare(const bkstring& _Str) const 
	{ return _tcscmp(buf, _Str.c_str()); }
	
	int compare(size_type _Pos1, size_type _Num1, const bkstring& _Str) const 
	{ return _tcsncmp(&buf[_Pos1], _Str.c_str(), _Num1); }
	
	int compare(size_type _Pos1, size_type _Num1, const bkstring& _Str, size_type _Off, size_type _Count) const
	{ return _tcsncmp(&buf[_Pos1], _Str.c_str()+_Off, min(_Num1, _Count)); }

	int compare(const value_type* _Ptr) const 
	{ return _tcscmp(buf, _Ptr); }
	
	int compare(size_type _Pos1, size_type _Num1, const value_type* _Ptr) const
	{ return _tcsncmp(&buf[_Pos1], _Ptr, _Num1); }
	
	int compare(size_type _Pos1, size_type _Num1, const value_type* _Ptr, size_type _Num2) const
	{ return _tcsncmp(&buf[_Pos1], _Ptr, min(_Num1, _Num2)); }

	int comparei(const bkstring& _Str) const 
	{ return _tcsicmp(buf, _Str.c_str()); }
	
	int comparei(size_type _Pos1, size_type _Num1, const bkstring& _Str) const 
	{ return _tcsnicmp(&buf[_Pos1], _Str.c_str(), _Num1); }
	
	int comparei(size_type _Pos1, size_type _Num1, const bkstring& _Str, size_type _Off, size_type _Count) const
	{ return _tcsnicmp(&buf[_Pos1], _Str.c_str()+_Off, min(_Num1, _Count)); }

	int comparei(const value_type* _Ptr) const 
	{ return _tcsicmp(buf, _Ptr); }
	
	int comparei(size_type _Pos1, size_type _Num1, const value_type* _Ptr) const
	{ return _tcsnicmp(&buf[_Pos1], _Ptr, _Num1); }
	
	int comparei(size_type _Pos1, size_type _Num1, const value_type* _Ptr, size_type _Num2) const
	{ return _tcsnicmp(&buf[_Pos1], _Ptr, min(_Num1, _Num2)); }

	bool empty(void) const { return lenBuf == 0; };
	bkstring& erase(size_type _Pos = 0, size_type _Count = npos);

	size_type find(value_type _Ch, size_type _Off = 0) const;
	size_type find(const value_type* _Ptr, size_type _Off = 0) const;
	size_type find(bkstring& _Str, size_type _Off = 0) const
	{ return find(_Str.c_str(), _Off); }

	size_type find_last_of(value_type _Ch, size_type _Off = npos) const;

	bkstring& insert(size_type _P0, const value_type* _Ptr)
	{ return insert(_P0, _Ptr, _tcslen(_Ptr)); }
	
	bkstring& insert(size_type _P0, const bkstring& _Str)
	{ return insert(_P0, _Str.c_str(), _Str.size()); };

	bkstring& insert(size_type _P0, const value_type* _Ptr, size_type _Count);
	bkstring& insert(size_type _P0, size_type _Count, value_type _Ch);

	bkstring substr(size_type _Off = 0, size_type _Count = npos) const
	{ return bkstring(*this, _Off, _Count); }

	bkstring& operator = (const bkstring& _Str)
	{ return assign(_Str); }

	bkstring& operator =(const value_type* _Ptr)
	{ return assign(_Ptr); }

	bkstring& operator = (const value_type _Ch)
	{ return assign(1, _Ch); }

	bkstring& operator +=(const bkstring& _Str)
	{ return append(_Str); }

	bkstring& operator += (const value_type* _Ptr)
	{ return append(_Ptr); }

	bkstring& operator += (const value_type _Ch)
	{ return append(1, _Ch); }

	value_type& operator[] (int ind) const
	{ return buf[ind]; }

	friend bkstring operator+ (const bkstring& _Str1, const bkstring& _Str2)
	{ bkstring s(_Str1); return s.append(_Str2); }

	friend bkstring operator+ (const bkstring& _Str1, const value_type* _Ptr2)
	{ bkstring s(_Str1); return s.append(_Ptr2); }

	friend bkstring operator+(const value_type* _Ptr1, const bkstring& _Str2)
	{ bkstring s(_Ptr1); return s.append(_Str2); }

	friend bkstring operator+ (const bkstring& _Str1, const value_type _Ch)
	{ bkstring s(_Str1); return s.append(1, _Ch); }

	friend bool operator==(const bkstring& _Str1, const bkstring& _Str2)
	{ return _Str1.compare(_Str2) == 0; }

	friend bool operator==(const bkstring& _Str1, const value_type* _Ptr2)
	{ return _Str1.compare(_Ptr2) == 0; }

	friend bool operator==(const value_type* _Ptr1, const bkstring& _Str2)
	{ return _Str2.compare(_Ptr1) == 0; }

	friend bool operator!=(const bkstring& _Str1, const bkstring& _Str2)
	{ return _Str1.compare(_Str2) != 0; }

	friend bool operator!=(const bkstring& _Str1, const value_type* _Ptr2)
	{ return _Str1.compare(_Ptr2) != 0; }

	friend bool operator!=(const value_type* _Ptr1, const bkstring& _Str2)
	{ return _Str2.compare(_Ptr1) != 0; }

	friend bool operator<(const bkstring& _Str1, const bkstring& _Str2)
	{ return _Str1.compare(_Str2) < 0; }

	friend bool operator<(const bkstring& _Str1, const value_type* _Ptr2)
	{ return _Str1.compare(_Ptr2) < 0; }

	friend bool operator<(const value_type* _Ptr1, const bkstring& _Str2)
	{ return _Str2.compare(_Ptr1) > 0; }

	friend bool operator>(const bkstring& _Str1, const bkstring& _Str2)
	{ return _Str1.compare(_Str2) > 0; }

	friend bool operator>(const bkstring& _Str1, const value_type* _Ptr2)
	{ return _Str1.compare(_Ptr2) > 0; }

	friend bool operator>(const value_type* _Ptr1, const bkstring& _Str2)
	{ return _Str2.compare(_Ptr1) < 0; }

	friend bool operator<=(const bkstring& _Str1, const bkstring& _Str2)
	{ return _Str1.compare(_Str2) <= 0; }

	friend bool operator<=(const bkstring& _Str1, const value_type* _Ptr2)
	{  return _Str1.compare(_Ptr2) <= 0; }

	friend bool  operator<=(const value_type* _Ptr1, const bkstring& _Str2)
	{ return _Str2.compare(_Ptr1) >= 0; }

	friend bool  operator>=(const bkstring& _Str1, const bkstring& _Str2)
	{ return _Str1.compare(_Str2) >= 0; }

	friend bool  operator>=(const bkstring& _Str1, const value_type* _Ptr2)
	{ return _Str1.compare(_Ptr2) >= 0; }

	friend bool  operator>=(const value_type* _Ptr1, const bkstring& _Str2)
	{ return _Str2.compare(_Ptr1) <= 0; }

	friend bool  operator==(const value_type _Ch1, const bkstring& _Str2)
	{  return (_Str2.size() == 1) && (_Str2[0] == _Ch1); }

	friend bool  operator==(const bkstring& _Str1, const value_type _Ch2)
	{ return (_Str1.size() == 1) && (_Str1[0] == _Ch2); }

	friend bool  operator!=(const value_type _Ch1, const bkstring& _Str2)
	{  return (_Str2.size() != 1) || (_Str2[0] != _Ch1); }

	friend bool  operator!=(const bkstring& _Str1, const value_type _Ch2)
	{ return (_Str1.size() != 1) || (_Str1[0] != _Ch2); }

	iterator begin(void)
	{ return buf; }
	
	const_iterator begin(void) const
	{ return buf; }
	
	iterator end(void)
	{ return buf + lenBuf; }

	const_iterator end(void) const
	{ return buf + lenBuf; }
	  
	// Custom extentions

	void appendfmt(const value_type *fmt, ...);

	size_type sizebytes(void) const { return lenBuf * sizeof(value_type); }
};

//const bkstring::size_type bkstring::npos = -1;
