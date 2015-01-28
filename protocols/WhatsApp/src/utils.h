#if !defined(WHATS_NG_UTILS_H)
#define WHATS_NG_UTILS_H

#include "WhatsAPI++/IMutex.h"

class ScopedLock
{
public:
	ScopedLock(HANDLE h, int t = INFINITE) : handle_(h), timeout_(t)
	{
		WaitForSingleObject(handle_, timeout_);
	}
	~ScopedLock()
	{
		if (handle_)
			ReleaseMutex(handle_);
	}
	void Unlock()
	{
		ReleaseMutex(handle_);
		handle_ = 0;
	}
private:
	HANDLE handle_;
	int timeout_;
};

class Mutex : public IMutex
{
private:
	HANDLE handle;
public:
	Mutex() : handle(NULL) {}

	virtual ~Mutex()
	{
		if (this->handle != NULL) {
			ReleaseMutex(this->handle);
		}
	}

	virtual void lock()
	{
		if (this->handle == NULL) {
			this->handle = CreateMutex(NULL, FALSE, NULL);
		}
	}

	virtual void unlock()
	{
		ReleaseMutex(this->handle);
		this->handle = NULL;
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