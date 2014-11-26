/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-13 Robert Pösel

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

// C++ bool type
#define UTILS_CONV_BOOLEAN              0x0001 // true | false
// signed regular numbers
#define UTILS_CONV_SIGNED_NUMBER        0x0010 // 1234 | -1234
// unsigned regular numbers
#define UTILS_CONV_UNSIGNED_NUMBER      0x0020 // 1234
// miscellaneous
#define UTILS_CONV_TIME_T               0x0040 // 1234567890

namespace utils
{
	namespace url
	{
		std::string encode(const std::string &s);
		std::string decode(std::string data);
	};

	namespace time
	{
		std::string unix_timestamp();
		std::string mili_timestamp();
		DWORD fix_timestamp(unsigned __int64 mili_timestamp);
	};

	namespace number
	{
		int random(int min, int max);
	};

	namespace text
	{
		void replace_first(std::string* data, const std::string &from, const std::string &to);
		void replace_all(std::string* data, const std::string &from, const std::string &to);
		unsigned int count_all(std::string* data, const std::string &term);
		std::string html_entities_decode(std::string data);
		std::string edit_html(std::string data);
		std::string remove_html(const std::string &data);
		std::string slashu_to_utf8(const std::string &data);
		std::string trim(const std::string &data, bool rtrim = false);
		std::string source_get_value(std::string* data, unsigned int argument_count, ...);
		std::string source_get_value2(std::string* data, const char *term, const char *endings, bool wholeString = false);
		std::string source_get_form_data(std::string* data);
		std::string rand_string(int len, const char *chars = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz");
		void explode(std::string str, const std::string &separator, std::vector<std::string>* results);
		void append_ordinal(unsigned long value, std::string* data);
	};

	namespace conversion
	{
		DWORD to_timestamp(const std::string &data);
		std::string to_string(void*, WORD type);

		template <class T>
		bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&)) {
			std::istringstream iss(s);
			return !(iss >> f >> t).fail();				
		}
	};
};

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

void MB(const char* m);
void MBI(int a);
