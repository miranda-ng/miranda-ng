#ifndef UNICODE	//FIXME Build without UNICODE flag
#define _AtlGetConversionACP() CP_THREAD_ACP
#endif

#include "MString.h"

typedef CMStringA astring;
typedef CMStringW wstring;

class mbstring : public astring
{
	// It is prohibited to initialize by char* outside, use L"xxx"
private:
	mbstring(const char * pChar) : astring(pChar) {};
	mbstring& operator=(const char * pChar)	{ this->operator =(pChar); return *this; }

public:
	mbstring() : astring() {};
	mbstring(const mbstring& uStr) : astring(uStr) {};


	mbstring(const wstring&  tStr)	{ *this = tStr.c_str(); }
	mbstring& operator=(const wstring&  tStr)	{ this->operator =(tStr.c_str()); return *this; }

	mbstring(const wchar_t * wChar);
	mbstring& operator=(const astring&  aStr);
	mbstring& operator=(const wchar_t * wChar);
	operator wstring();
	operator astring();
};


class tstring : public wstring
{
public:
	tstring() : wstring() {};
	tstring(const wchar_t * pwChar) : wstring(pwChar) {};


	tstring(const astring& aStr)		{ *this = aStr.c_str(); }
	tstring(const mbstring& utfStr)		{ *this = utfStr; }

	tstring(const char * pChar);
	tstring& operator=(const char * pChar);
	tstring& operator=(const astring& aStr);
	tstring& operator=(const mbstring& uStr);
	operator astring();
	operator mbstring()					{ return mbstring(this->c_str()); }
};

