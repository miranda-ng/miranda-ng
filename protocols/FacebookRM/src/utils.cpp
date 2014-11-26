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

#include "common.h"

std::string utils::url::encode(const std::string &s)
{
	return (char*)ptrA(mir_urlEncode(s.c_str()));
}

std::string utils::url::decode(std::string data)
{
	// TODO: Better and universal method?
	utils::text::replace_all(&data, "%2F", "/");
	utils::text::replace_all(&data, "%3F", "?");
	utils::text::replace_all(&data, "%3D", "=");
	utils::text::replace_all(&data, "%26", "&");
	utils::text::replace_all(&data, "%3A", ":");

	return data;
}

std::string utils::time::unix_timestamp()
{
	time_t in = ::time(NULL);
	return utils::conversion::to_string((void*)&in, UTILS_CONV_TIME_T);
}

std::string utils::time::mili_timestamp()
{
	SYSTEMTIME st;
	std::string timestamp = utils::time::unix_timestamp();
	GetSystemTime(&st);
	timestamp.append(utils::conversion::to_string((void*)&st.wMilliseconds, UTILS_CONV_UNSIGNED_NUMBER));
	return timestamp.substr(0,13);
}

DWORD utils::time::fix_timestamp(unsigned __int64 mili_timestamp)
{
	// If it is really mili_timestamp
	if (mili_timestamp > 100000000000) {
		return (DWORD) (mili_timestamp / 1000);
	}
	return (DWORD) mili_timestamp;
}

DWORD utils::conversion::to_timestamp(const std::string &data)
{
	DWORD timestamp = NULL;
	if (!utils::conversion::from_string<DWORD>(timestamp, data, std::dec)) {
		timestamp = static_cast<DWORD>(::time(NULL));
	}
	return timestamp;
}

std::string utils::conversion::to_string(void* data, WORD type)
{
	std::stringstream out;

	switch (type)
	{
  	case UTILS_CONV_BOOLEAN:
		out << (data ? "true" : "false");

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
	if (position != std::string::npos)
	{
		data->replace(position, from.size(), to);
	}
}

void utils::text::replace_all(std::string* data, const std::string &from, const std::string &to)
{
	std::string::size_type position = 0;

	while ((position = data->find(from, position)) != std::string::npos)
	{
		data->replace(position, from.size(), to);
		position++;
	}
}

unsigned int utils::text::count_all(std::string* data, const std::string &term)
{
	unsigned int count = 0;
	std::string::size_type position = 0;

	while ((position = data->find(term, position)) != std::string::npos)
	{
		count++;
		position++;
	}

	return count;
}

void utils::text::append_ordinal(unsigned long value, std::string* data)
{
	if (value >= 128 && value <= 2047)
	{ // U+0080 .. U+07FF
		*data += (char)(192 + (value / 64));
		*data += (char)(128 + (value % 64));
	} 
	else if (value >= 2048 && value <= 65535)
	{ // U+0800 .. U+FFFF
		*data += (char)(224 + (value / 4096));
		*data += (char)(128 + ((value / 64) % 64));
		*data += (char)(128 + (value % 64));
	}
	else if (value <= 127)
	{ // U+0000 .. U+007F
		*data += (char)value;
	}
}

std::string utils::text::html_entities_decode(std::string data)
{
	utils::text::replace_all(&data, "&amp;", "&");
	utils::text::replace_all(&data, "&quot;", "\"");
	utils::text::replace_all(&data, "&lt;", "<");
	utils::text::replace_all(&data, "&gt;", ">");

	utils::text::replace_all(&data, "&hearts;", "\xE2\x99\xA5"); // direct byte replacement
//	utils::text::replace_all(&data, "&hearts;", "\\u2665");      // indirect slashu replacement

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
	for (std::string::size_type i = 0; i < data.length(); i++)
	{
		if (data.at(i) == '&' && (i+1) < data.length() && data.at(i+1) == '#')
		{
			std::string::size_type comma = data.find(";", i);
			if (comma != std::string::npos) {
				bool hexa = false;
				if ((i+2) < data.length() && data.at(i+2) == 'x') {
					hexa = true;
					i += 3;
				} else {
					i += 2;
				}

				std::string num = data.substr(i, comma - i);
				if (!num.empty()) {
					unsigned int udn = strtol(num.c_str(), NULL, hexa ? 16 : 10);
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

std::string utils::text::edit_html(std::string data)
{
	std::string::size_type end = 0;
	std::string::size_type start = 0;
	std::string new_string;
  
	while (end != std::string::npos)
	{
		end = data.find("<span class=\\\"text_exposed_hide", start);
		if (end != std::string::npos)
		{
			new_string += data.substr(start, end - start);
			start = data.find("<\\/span", end);
		} else {
			new_string += data.substr(start, data.length() - start);
		}
	}

	start = end = 0;
	data = new_string;
	new_string.clear();

	while (end != std::string::npos)
	{
		end = data.find("<span class=\\\"uiTooltipText", start);
		if (end != std::string::npos)
		{
			new_string += data.substr(start, end - start);
			start = data.find("<\\/span", end);
		} else {
			new_string += data.substr(start, data.length() - start);
		}
	}

	// Remove "Translate" link
	start = end = 0;
	data = new_string;
	new_string.clear();
	while (end != std::string::npos)
	{
		end = data.find("role=\\\"button\\\">", start);
		if (end != std::string::npos)
		{
			new_string += data.substr(start, end - start);
			start = data.find("<\\/a", end);
		} else {
			new_string += data.substr(start, data.length() - start);
		}
	}

	// Append newline after attachement title
	start = new_string.find("class=\\\"uiAttachmentTitle", 0);
	if (start != std::string::npos)
	{
		data = new_string.substr(0, start);
		data = utils::text::trim(data);

		start = new_string.find(">", start);
		if (start != std::string::npos)
			new_string.insert(start+1, "\n\n");

		start = new_string.find("<\\/div>", start);
		if (start != std::string::npos)
			new_string.insert(start, "\n");
	}

	// Append newline between attachement link and description
	start = new_string.find("uiAttachmentDesc", 0);
	if (start != std::string::npos)
	{
		start = new_string.find(">", start);
		if (start != std::string::npos)
			new_string.insert(start+1, "\n");

		start = new_string.find("<\\/div>", start);
		if (start != std::string::npos)
			new_string.insert(start, "\n");
	}
  
	utils::text::replace_all(&new_string, "<br \\/>", "\n");
	utils::text::replace_all(&new_string, "\n\n\n", "\n\n");
	//utils::text::replace_all(&new_string, "\\t", "");
	//utils::text::replace_all(&new_string, "\\n", "");
	return new_string;
}


std::string utils::text::remove_html(const std::string &data)
{
	std::string new_string;

	for (std::string::size_type i = 0; i < data.length(); i++)
	{
		if (data.at(i) == '<' && (i+1) < data.length() && data.at(i+1) != ' ')
		{
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

	for (std::string::size_type i = 0; i < data.length(); i++)
	{
		if (data.at(i) == '\\' && (i+1) < data.length() && data.at(i+1) == 'u')
		{
			unsigned int udn = strtol(data.substr(i + 2, 4).c_str(), NULL, 16);
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
			results->push_back(str.substr(0,pos));
		}
		str = str.substr(pos+1);
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
	
	for (unsigned int i = argument_count; i > 0; i--)
	{
		if (i == 1)
		{
			end = data->find(va_arg(arg, char*), start);
			if (start == std::string::npos || end == std::string::npos)
				break;
			ret = data->substr(start, end - start);
		} else {
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
		start += strlen(term);

		end = data->find_first_of(endings, start);
		if (end != std::string::npos) {
			ret = data->substr(start, end - start);
		} else if (wholeString) {
			ret = data->substr(start);
		}
	}

	return ret;
}

std::string utils::text::source_get_form_data(std::string* data)
{
	std::string values;

	std::string::size_type start = 0;
	start = data->find("<input", start);
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
		start = data->find("<input", start);
	}

	return values;
}

std::string utils::text::rand_string(int len, const char *chars)
{
	std::stringstream out;

	int strLen = (int)strlen(chars);
	for (int i = 0; i < len; ++i) {
		out << chars[utils::number::random(0, strLen)];
	}

	return out.str();
}

int utils::number::random(int min, int max)
{	
	return (rand() % (max - min)) + min;
}
