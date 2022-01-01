/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015-17 Robert Pösel, 2017-22 Miranda NG team

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

// DB macros
#define getU8String(setting, dest)        db_get_utf(NULL, m_szModuleName, setting, dest)
#define setU8String(setting, value)       db_set_utf(NULL, m_szModuleName, setting, value)

// HTTP constants and object
#define HTTP_CODE_FAKE_DISCONNECTED 0
#define HTTP_CODE_FAKE_ERROR        1

namespace http
{
	struct response
	{
		response() : code(0) {}
		int code;
		std::map< std::string, std::string > headers;
		std::string data;
	};
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
		handle_ = nullptr;
	}
private:
	HANDLE handle_;
};
