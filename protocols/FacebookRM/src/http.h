/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-15 Robert P�sel

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

#define HTTP_PROTO_SECURE           "https://"

#define HTTP_CODE_FAKE_DISCONNECTED		0
#define HTTP_CODE_FAKE_ERROR			1
#define HTTP_CODE_FAKE_OFFLINE			2

namespace http
{
	struct response
	{
		response() : code(0), error_number(0) {}

		int code;
		std::map<std::string, std::string> headers;
		std::string data;

		// Facebook's error data
		unsigned int error_number;
		std::string error_title;
		std::string error_text;		

		bool isValid() {
			return (code == HTTP_CODE_OK && error_number == 0);
		}
	};
}
