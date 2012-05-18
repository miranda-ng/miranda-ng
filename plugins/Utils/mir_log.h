/* 
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

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


#ifndef __LOG_H__
# define __LOG_H__

#include <windows.h>
#include <string>


int mlog(const char *module, const char *function, const char *fmt, ...);
int mlog(const char *module, const char *function, const char *fmt, va_list va);
int mlogC(const char *module, const char *function, HANDLE hContact, const char *fmt, ...);


#ifdef __cplusplus

class MLog
{
private:
	std::string module;
	std::string function;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	LARGE_INTEGER total;

	static int deep;

	void StartTimer();
	void StopTimer();
	double GetElapsedTimeMS();
	double GetTotalTimeMS();

public:
	MLog(const char *aModule, const char *aFunction);
	~MLog();

	int log(const char *fmt, ...);
};


#endif __cplusplus



#endif // __LOG_H__
