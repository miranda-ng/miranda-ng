/*
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "http.h"
#include "twitter.h"

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

std::string b64encode(const std::string &s);

class mir_twitter : public twitter
{
public:
	void set_handle(HANDLE h)
	{
		handle_ = h;
	}
protected:
	http::response slurp(const std::string &,http::method,const std::string &);
	HANDLE handle_;
};

inline void mbcs_to_tcs(UINT code_page,const char *mbstr,TCHAR *tstr,int tlen)
{
#ifdef UNICODE
	MultiByteToWideChar(code_page,0,mbstr,-1,tstr,tlen);
#else
	strncpy(tstr,mbstr,tlen);
#endif
}

inline void wcs_to_tcs(UINT code_page,const wchar_t *wstr,TCHAR *tstr,int tlen)
{
#ifdef UNICODE
	wcsncpy(tstr,wstr,tlen);
#else
	WideCharToMultiByte(code_page,0,wstr,-1,tstr,tlen,0,0);
#endif
}

class ScopedLock
{
public:
	ScopedLock(HANDLE h) : handle_(h)
	{
		WaitForSingleObject(handle_,INFINITE);
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
};

int ext_to_format(const std::string &ext);
bool save_url(HANDLE hNetlib,const std::string &url,const std::string &filename);