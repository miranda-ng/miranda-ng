#if !defined(WHATS_NG_UTILS_H)
#define WHATS_NG_UTILS_H

#include "WhatsAPI++/IMutex.h"

template<typename T>
void CreateProtoService(const char *module,const char *service,
	int (__cdecl T::*serviceProc)(WPARAM,LPARAM),T *self)
{
	char temp[MAX_PATH*2];

	mir_snprintf(temp,sizeof(temp),"%s%s",module,service);
	CreateServiceFunctionObj(temp,( MIRANDASERVICEOBJ )*(void**)&serviceProc, self );
}

template<typename T>
void HookProtoEvent(const char* evt, int (__cdecl T::*eventProc)(WPARAM,LPARAM), T *self)
{
	::HookEventObj(evt,(MIRANDAHOOKOBJ)*(void**)&eventProc,self);
}

template<typename T>
HANDLE ForkThreadEx(void (__cdecl T::*thread)(void*),T *self,void *data = 0)
{
	return reinterpret_cast<HANDLE>( mir_forkthreadowner(
		(pThreadFuncOwner)*(void**)&thread,self,data,0));
}

template<typename T>
void ForkThread(void (__cdecl T::*thread)(void*),T *self,void *data = 0)
{
	CloseHandle(ForkThreadEx(thread,self,data));
}

class ScopedLock
{
public:
	ScopedLock(HANDLE h, int t = INFINITE) : handle_(h), timeout_(t)
	{
		WaitForSingleObject(handle_,timeout_);
	}
	~ScopedLock()
	{
		if(handle_)
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
      if (this->handle != NULL)
      {
         ReleaseMutex(this->handle);
      }
   }

   virtual void lock()
   {
      if (this->handle == NULL)
      {
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

void UnixTimeToFileTime(time_t t, LPFILETIME pft);

namespace utils
{
   namespace debug
	{
		int log(std::string file_name, std::string text);
	};

	namespace conversion
	{
		DWORD to_timestamp( std::string data );
   };

   namespace text
   {
      std::string source_get_value(std::string* data, unsigned int argument_count, ...);
   };
};


#endif