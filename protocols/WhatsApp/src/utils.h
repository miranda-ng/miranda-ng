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

namespace utils
{
	void setStatusMessage(MCONTACT hContact, const TCHAR *ptszMessage);
		
	BYTE* md5string(const BYTE*, int, BYTE* digest);
	__forceinline BYTE* md5string(const std::string &str, BYTE* digest)
	{
		return md5string((BYTE*)str.data(), (int)str.length(), digest);
	}
};


#endif