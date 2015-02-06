/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-15 Robert Pösel

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

#define HTTP_PROTO_REGULAR          "http://"
#define HTTP_PROTO_SECURE           "https://"

#define HTTP_CODE_CONTINUE				100
#define HTTP_CODE_SWITCHING_PROTOCOLS   101
#define HTTP_CODE_PROCESSING			102
#define HTTP_CODE_OK					200
#define HTTP_CODE_CREATED				201
#define HTTP_CODE_ACCEPTED				202
#define HTTP_CODE_NON_AUTHORITATIVE_INFORMATION 203
#define HTTP_CODE_NO_CONTENT			204
#define HTTP_CODE_RESET_CONTENT			205
#define HTTP_CODE_PARTIAL_CONTENT		206
#define HTTP_CODE_MULTI_STATUS			207
#define HTTP_CODE_MULTIPLE_CHOICES		300
#define HTTP_CODE_MOVED_PERMANENTLY		301
#define HTTP_CODE_FOUND					302
#define HTTP_CODE_SEE_OTHER				303
#define HTTP_CODE_NOT_MODIFIED			304
#define HTTP_CODE_USE_PROXY				305
#define HTTP_CODE_SWITCH_PROXY			306
#define HTTP_CODE_TEMPORARY_REDIRECT    307
#define HTTP_CODE_BAD_REQUEST			400
#define HTTP_CODE_UNAUTHORIZED			401
#define HTTP_CODE_PAYMENT_REQUIRED		402
#define HTTP_CODE_FORBIDDEN				403
#define HTTP_CODE_NOT_FOUND				404
#define HTTP_CODE_METHOD_NOT_ALLOWED    405
#define HTTP_CODE_NOT_ACCEPTABLE		406
#define HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED 407
#define HTTP_CODE_REQUEST_TIMEOUT		408
#define HTTP_CODE_CONFLICT				409
#define HTTP_CODE_GONE					410
#define HTTP_CODE_LENGTH_REQUIRED		411
#define HTTP_CODE_PRECONDITION_REQUIRED 412
#define HTTP_CODE_REQUEST_ENTITY_TOO_LARGE  413
#define HTTP_CODE_REQUEST_URI_TOO_LONG  414
#define HTTP_CODE_UNSUPPORTED_MEDIA_TYPE    415
#define HTTP_CODE_REQUESTED_RANGE_NOT_SATISFIABLE   416
#define HTTP_CODE_EXPECTATION_FAILED    417
#define HTTP_CODE_UNPROCESSABLE_ENTITY  422
#define HTTP_CODE_LOCKED				423
#define HTTP_CODE_FAILED_DEPENDENCY		424
#define HTTP_CODE_UNORDERED_COLLECTION  425
#define HTTP_CODE_UPGRADE_REQUIRED		426
#define HTTP_CODE_RETRY_WITH			449
#define HTTP_CODE_INTERNAL_SERVER_ERROR 500
#define HTTP_CODE_NOT_IMPLEMENTED		501
#define HTTP_CODE_BAD_GATEWAY			502
#define HTTP_CODE_SERVICE_UNAVAILABLE   503
#define HTTP_CODE_GATEWAY_TIMEOUT		504
#define HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED    505
#define HTTP_CODE_VARIANT_ALSO_NEGOTIATES   506
#define HTTP_CODE_INSUFFICIENT_STORAGE  507
#define HTTP_CODE_BANDWIDTH_LIMIT_EXCEEDED  509
#define HTTP_CODE_NOT_EXTENDED			510

#define HTTP_CODE_FAKE_DISCONNECTED		0
#define HTTP_CODE_FAKE_ERROR			1
#define HTTP_CODE_FAKE_OFFLINE			2

namespace http
{
	enum method
	{
		get,
		post
	};

	struct response
	{
		response() : code(0), error_number(0) {}
		int code;
		std::map< std::string, std::string > headers;
		std::string data;

		// Facebook's error data
		unsigned int error_number;
		std::string error_text;
		std::string error_title;

		bool isValid() {
			return (code == HTTP_CODE_OK && error_number == 0);
		}
	};
}
