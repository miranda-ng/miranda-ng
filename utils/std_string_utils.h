/*

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-22 Miranda NG team

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

#define _CRT_RAND_S

#include <string>
#include <sstream>
#include <vector>

#include <windows.h>
#include <time.h>

#include <m_system.h>

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
		time_t from_string(const std::string &data);
	};

	namespace number
	{
		int random(int min, int max, unsigned int *value = nullptr);
	};

	namespace text
	{
		void replace_first(std::string* data, const std::string &from, const std::string &to);
		void replace_all(std::string* data, const std::string &from, const std::string &to);
		void treplace_all(std::wstring* data, const std::wstring &from, const std::wstring &to);
		unsigned int count_all(std::string* data, const std::string &term);
		std::string html_entities_decode(std::string data);
		std::string remove_html(const std::string &data);
		std::string slashu_to_utf8(const std::string &data);
		std::string trim(const std::string &data, bool rtrim = false);
		std::string source_get_value(std::string* data, unsigned int argument_count, ...);
		std::string source_get_value2(std::string* data, const char *term, const char *endings, bool wholeString = false);
		std::string source_get_form_data(std::string* data, bool hiddenOnly = false);
		std::string rand_string(int len, const char *chars = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz", unsigned int *number = nullptr);
		std::string truncate_utf8(const std::string &text, size_t maxLength);
		void explode(std::string str, const std::string &separator, std::vector<std::string>* results);
		void append_ordinal(unsigned long value, std::string* data);
		std::string prepare_name(const std::string &name, bool withSurnameLetter);
	};

	namespace conversion
	{
		std::string to_string(void*, WORD type);

		template <class T>
		bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&)) {
			std::istringstream iss(s);
			return !(iss >> f >> t).fail();
		}
	};
};
