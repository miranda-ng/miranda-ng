/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-15 Robert Pösel

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

namespace utils
{
	namespace url
	{
		std::string encode(const std::string &s);
	};

	namespace text
	{
		void replace_first( std::string* data, const std::string &from, const std::string &to );
		void replace_all( std::string* data, const std::string &from, const std::string &to );
		void treplace_all(std::tstring* data, const std::tstring &from, const std::tstring &to);
		std::string special_expressions_decode( std::string data );
		std::string slashu_to_utf8(const std::string &data );
		std::string trim(const std::string &data );
	};

	namespace debug
	{
		int log(const std::string &file_name, const std::string &text);
	};

	namespace mem
	{
		void __fastcall detract(char** str );
		void __fastcall detract(void** p);
		void __fastcall detract(void* p);
		void* __fastcall allocate(size_t size);
	};
};

class ScopedLock
{
public:
	ScopedLock(HANDLE h) : handle_(h)
	{
		WaitForSingleObject(handle_,INFINITE);
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
};
