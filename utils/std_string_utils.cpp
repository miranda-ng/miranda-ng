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

#include "std_string_utils.h"

std::string utils::url::encode(const std::string &s)
{
	return std::string(mir_urlEncode(s.c_str()));
}

std::string utils::url::decode(std::string data)
{
	std::string new_string;
	for (std::string::size_type i = 0; i < data.length(); i++) {
		if (data.at(i) == '%' && (i + 2) < data.length()) {
			std::string num = data.substr(i + 1, 2);
			unsigned long udn = strtoul(num.c_str(), nullptr, 16);
			utils::text::append_ordinal(udn, &new_string);
			i += 2;
			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::time::unix_timestamp()
{
	time_t in = ::time(0);
	return utils::conversion::to_string((void*)&in, UTILS_CONV_TIME_T);
}

std::string utils::time::mili_timestamp()
{
	SYSTEMTIME st;
	std::string timestamp = utils::time::unix_timestamp();
	GetSystemTime(&st);
	timestamp.append(utils::conversion::to_string((void*)&st.wMilliseconds, UTILS_CONV_UNSIGNED_NUMBER));
	return timestamp.substr(0, 13);
}

time_t utils::time::from_string(const std::string &data)
{
	long long timestamp = _strtoi64(data.c_str(), nullptr, 10);

	// If it is milli timestamp
	if (timestamp > 100000000000)
		timestamp /= 1000;

	// If conversion fails, use local time?
	//if (!timestamp)
	//	timestamp = ::time(NULL);

	return (time_t)timestamp;
}

std::string utils::conversion::to_string(void* data, uint16_t type)
{
	std::stringstream out;

	switch (type) {
	case UTILS_CONV_BOOLEAN:
		out << (data ? "true" : "false");
		break;

	case UTILS_CONV_TIME_T:
		out << (*(time_t*)data);
		break;

	case UTILS_CONV_SIGNED_NUMBER:
		out << (*(signed int*)data);
		break;

	case UTILS_CONV_UNSIGNED_NUMBER:
		out << (*(unsigned int*)data);
		break;
	}

	return out.str();
}

void utils::text::replace_first(std::string* data, const std::string &from, const std::string &to)
{
	std::string::size_type position = data->find(from);
	if (position != std::string::npos) {
		data->replace(position, from.size(), to);
	}
}

void utils::text::replace_all(std::string* data, const std::string &from, const std::string &to)
{
	std::string::size_type position = 0;

	while ((position = data->find(from, position)) != std::string::npos) {
		data->replace(position, from.size(), to);
		position += to.size();
	}
}

void utils::text::treplace_all(std::wstring* data, const std::wstring &from, const std::wstring &to)
{
	std::wstring::size_type position = 0;

	while ((position = data->find(from, position)) != std::wstring::npos) {
		data->replace(position, from.size(), to);
		position++;
	}
}

unsigned int utils::text::count_all(std::string* data, const std::string &term)
{
	unsigned int count = 0;
	std::string::size_type position = 0;

	while ((position = data->find(term, position)) != std::string::npos) {
		count++;
		position++;
	}

	return count;
}

void utils::text::append_ordinal(unsigned long value, std::string* data)
{
	if (value <= 127) { // U+0000 .. U+007F
		*data += (char)value;
	}
	else if (value >= 128 && value <= 2047) { // U+0080 .. U+07FF
		*data += (char)(192 + (value / 64));
		*data += (char)(128 + (value % 64));
	}
	else if (value >= 2048 && value <= 65535) { // U+0800 .. U+FFFF
		*data += (char)(224 + (value / 4096));
		*data += (char)(128 + ((value / 64) % 64));
		*data += (char)(128 + (value % 64));
	}
	else {
		*data += (char)((value >> 24) & 0xFF);
		*data += (char)((value >> 16) & 0xFF);
		*data += (char)((value >> 8) & 0xFF);
		*data += (char)((value) & 0xFF);
	}
}

std::string utils::text::html_entities_decode(std::string data)
{
	utils::text::replace_all(&data, "&amp;", "&");
	utils::text::replace_all(&data, "&quot;", "\"");
	utils::text::replace_all(&data, "&lt;", "<");
	utils::text::replace_all(&data, "&gt;", ">");
	utils::text::replace_all(&data, "&nbsp;", " ");

	utils::text::replace_all(&data, "&hearts;", "\xE2\x99\xA5"); // direct byte replacement
	//	utils::text::replace_all(&data, "&hearts;", "\\u2665");      // indirect slashu replacement

	utils::text::replace_all(&data, "\\r", "\r");
	utils::text::replace_all(&data, "\\n", "\n");
	utils::text::replace_all(&data, "\\\"", "\"");
	utils::text::replace_all(&data, "\\/", "/");
	utils::text::replace_all(&data, "\\\\", "\\");

	// TODO: Add more to comply general usage
	// http://www.utexas.edu/learn/html/spchar.html
	// http://www.webmonkey.com/reference/Special_Characters
	// http://www.degraeve.com/reference/specialcharacters.php
	// http://www.chami.com/tips/internet/050798i.html
	// http://www.w3schools.com/tags/ref_entities.asp
	// http://www.natural-innovations.com/wa/doc-charset.html
	// http://webdesign.about.com/library/bl_htmlcodes.htm

	std::string new_string;
	for (std::string::size_type i = 0; i < data.length(); i++) {
		if (data.at(i) == '&' && (i + 1) < data.length() && data.at(i + 1) == '#') {
			std::string::size_type comma = data.find(";", i);
			if (comma != std::string::npos) {
				bool hexa = false;
				if ((i + 2) < data.length() && data.at(i + 2) == 'x') {
					hexa = true;
					i += 3;
				}
				else {
					i += 2;
				}

				std::string num = data.substr(i, comma - i);
				if (!num.empty()) {
					unsigned long udn = strtoul(num.c_str(), nullptr, hexa ? 16 : 10);
					utils::text::append_ordinal(udn, &new_string);
				}

				i = comma;
				continue;
			}
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::remove_html(const std::string &data)
{
	std::string new_string;

	for (std::string::size_type i = 0; i < data.length(); i++) {
		if (data.at(i) == '<' && (i + 1) < data.length() && data.at(i + 1) != ' ') {
			i = data.find(">", i);
			if (i == std::string::npos)
				break;

			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::slashu_to_utf8(const std::string &data)
{
	std::string new_string;

	for (std::string::size_type i = 0; i < data.length(); i++) {
		if (data.at(i) == '\\' && (i + 1) < data.length() && data.at(i + 1) == 'u') {
			unsigned long udn = strtoul(data.substr(i + 2, 4).c_str(), nullptr, 16);
			append_ordinal(udn, &new_string);
			i += 5;
			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::trim(const std::string &data, bool rtrim)
{
	std::string spaces = " \t\r\n";
	std::string::size_type begin = rtrim ? 0 : data.find_first_not_of(spaces);
	std::string::size_type end = data.find_last_not_of(spaces);

	return (end != std::string::npos) ? data.substr(begin, end + 1 - begin) : "";
}

void utils::text::explode(std::string str, const std::string &separator, std::vector<std::string>* results)
{
	std::string::size_type pos;
	pos = str.find_first_of(separator);
	while (pos != std::string::npos) {
		if (pos > 0) {
			results->push_back(str.substr(0, pos));
		}
		str = str.substr(pos + 1);
		pos = str.find_first_of(separator);
	}
	if (str.length() > 0) {
		results->push_back(str);
	}
}

std::string utils::text::source_get_value(std::string* data, unsigned int argument_count, ...)
{
	va_list arg;
	std::string ret;
	std::string::size_type start = 0, end = 0;

	va_start(arg, argument_count);

	for (unsigned int i = argument_count; i > 0; i--) {
		if (i == 1) {
			end = data->find(va_arg(arg, char*), start);
			if (start == std::string::npos || end == std::string::npos)
				break;
			ret = data->substr(start, end - start);
		}
		else {
			std::string term = va_arg(arg, char*);
			start = data->find(term, start);
			if (start == std::string::npos)
				break;
			start += term.length();
		}
	}

	va_end(arg);
	return ret;
}

std::string utils::text::source_get_value2(std::string* data, const char *term, const char *endings, bool wholeString)
{
	std::string::size_type start = 0, end = 0;
	std::string ret;

	start = data->find(term);
	if (start != std::string::npos) {
		start += mir_strlen(term);

		end = data->find_first_of(endings, start);
		if (end != std::string::npos) {
			ret = data->substr(start, end - start);
		}
		else if (wholeString) {
			ret = data->substr(start);
		}
	}

	return ret;
}

std::string utils::text::source_get_form_data(std::string* data, bool hiddenOnly)
{
	const char *search = hiddenOnly ? "<input type=\"hidden\"" : "<input";
	std::string values;

	std::string::size_type start = 0;
	start = data->find(search, start);
	while (start != std::string::npos) {
		start++;
		std::string attr, value;

		std::string::size_type pos = data->find("name=\"", start);
		if (pos != std::string::npos) {
			pos += 6;
			std::string::size_type end = data->find("\"", pos);
			if (end != std::string::npos)
				attr = data->substr(pos, end - pos);


			end = data->find(">", pos);
			pos = data->find("value=\"", pos);
			if (pos != std::string::npos && end != std::string::npos && pos < end) {
				pos += 7;
				end = data->find("\"", pos);
				if (end != std::string::npos)
					value = data->substr(pos, end - pos);
			}
		}

		if (!attr.empty()) {
			if (!values.empty())
				values += "&";
			values += attr + "=" + value;
		}
		start = data->find(search, start);
	}

	return values;
}

std::string utils::text::prepare_name(const std::string &name, bool withSurnameLetter)
{
	std::string::size_type pos = name.find(" ");
	if (pos == std::wstring::npos)
		return name;

	std::string result = name.substr(0, pos);

	if (withSurnameLetter) {
		pos = name.rfind(" ") + 1; // we're sure there is some space in name so we can do +1 safely

		if (pos < name.length())
			result += " " + name.substr(pos, 1) + std::string(".");
	}

	return result;
}

std::string utils::text::rand_string(int len, const char *chars, unsigned int *number)
{
	std::stringstream out;

	int strLen = (int)mir_strlen(chars);
	for (int i = 0; i < len; ++i) {
		out << chars[utils::number::random(0, strLen, number)];
	}

	return out.str();
}

std::string utils::text::truncate_utf8(const std::string &text, size_t maxLength)
{
	// To not split some unicode character we need to transform it to wchar_t first, then split it, and then convert it back, because we want std::string as result
	// TODO: Probably there is much simpler and nicer way
	std::wstring wtext = ptrW(mir_utf8decodeW(text.c_str()));
	if (wtext.length() > maxLength) {
		wtext = wtext.substr(0, maxLength) + L"\x2026"; // unicode ellipsis
		return std::string(_T2A(wtext.c_str(), CP_UTF8));
	}
	// It's not longer, return given string
	return text;
}

int utils::number::random(int min, int max, unsigned int *number)
{
	if (number != nullptr) {
		errno_t err = rand_s(number);
		if (!err)
			return (*number % (max - min)) + min;
	}

	// If called didn't specified "number" or there was error, fallback to rand()
	return (rand() % (max - min)) + min;
}
