/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-18 Miranda NG team

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

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////////////
// getting info about channel and connecting to it

ReconnectRequest::ReconnectRequest(facebook_client *fc) :
	HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/ajax/presence/reconnect.php")
{
	Url
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__be", -1)
		<< CHAR_PARAM("reason", fc->chat_reconnect_reason_.empty() ? "6" : fc->chat_reconnect_reason_.c_str())
		<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
		<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", fc->__dyn())
		<< CHAR_PARAM("__req", fc->__req())
		<< CHAR_PARAM("__rev", fc->__rev());
}

//////////////////////////////////////////////////////////////////////////////////////////
// setting chat visibility

SetVisibilityRequest::SetVisibilityRequest(facebook_client *fc, bool online) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/chat/privacy/visibility.php")
{
	Url << INT_PARAM("dpr", 1);

	Body
		<< INT_PARAM("visibility", online ? 1 : 0)
		<< INT_PARAM("window_id", 0)
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__be", -1)			
		<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str())
		<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", fc->__dyn())
		<< CHAR_PARAM("__req", fc->__req())
		<< CHAR_PARAM("__rev", fc->__rev());
}
