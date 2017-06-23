/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel

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

#ifndef _FACEBOOK_REQUEST_STATUS_H_
#define _FACEBOOK_REQUEST_STATUS_H_

// getting info about channel and connecting to it
// revised 17.8.2016
class ReconnectRequest : public HttpRequest
{
public:
	ReconnectRequest(facebook_client *fc) :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/ajax/presence/reconnect.php")
	{
		Url
			<< "__a=1"
			<< "__pc=PHASED:DEFAULT"
			<< "__be=-1"
			<< CHAR_VALUE("reason", fc->chat_reconnect_reason_.empty() ? "6" : fc->chat_reconnect_reason_.c_str())
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev());
	}
};

// setting chat visibility
// revised 17.8.2016
class SetVisibilityRequest : public HttpRequest
{
public:
	SetVisibilityRequest(facebook_client *fc, bool online) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/chat/privacy/visibility.php")
	{
		Url
			<< "dpr=1";

		Body
			<< (online ? "visibility=1" : "visibility=0")
			<< "window_id=0"
			<< "__a=1"
			<< "__pc=PHASED:DEFAULT"
			<< "__be=-1"			
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev());
		;
	}
};

#endif //_FACEBOOK_REQUEST_STATUS_H_
