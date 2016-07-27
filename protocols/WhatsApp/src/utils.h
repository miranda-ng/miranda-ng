#if !defined(WHATS_NG_UTILS_H)
#define WHATS_NG_UTILS_H

#include "WhatsAPI++/IMutex.h"

class Mutex : public IMutex
{
	mir_cs m_cs;

public:
	Mutex() {}
	virtual ~Mutex() {}

	virtual void lock()
	{
		CRITICAL_SECTION &cs = m_cs;
		::EnterCriticalSection(&cs);
	}

	virtual void unlock()
	{
		CRITICAL_SECTION &cs = m_cs;
		::LeaveCriticalSection(&cs);
	}
};


std::string getLastErrorMsg();

__forceinline wchar_t* str2t(const std::string &str)
{	return mir_utf8decodeW(str.c_str());
}

std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

namespace utils
{
	wchar_t* removeA(wchar_t *str);
	void   copyText(HWND hwnd, const wchar_t *text);

	void setStatusMessage(MCONTACT hContact, const wchar_t *ptszMessage);
		
	BYTE* md5string(const BYTE*, int, BYTE* digest);
	__forceinline BYTE* md5string(const std::string &str, BYTE* digest)
	{
		return md5string((BYTE*)str.data(), (int)str.length(), digest);
	}
};


#endif