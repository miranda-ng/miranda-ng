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

#ifndef _FACEBOOK_REQUEST_NOTIFICATIONS_H_
#define _FACEBOOK_REQUEST_NOTIFICATIONS_H_

// getting notifications
// revised 17.8.2016
class GetNotificationsRequest : public HttpRequest
{
public:
	GetNotificationsRequest(facebook_client *fc, int count) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/notifications/client/get.php")
	{
		Url
			<< "dpr=1";

		Body
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< "cursor=" // when loading more
			<< INT_VALUE("length", count) // number of items to load
			<< "businessID=" // probably for pages?
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< "__pc=PHASED:DEFAULT"
			<< "__be=-1"
			<< "__a=1";
	}
};

// marking notifications read
// request revised 11.2.2016 (we're not using the main website request, as it doesn't work, but still the old one with GET parameters)
class MarkNotificationReadRequest : public HttpRequest
{
public:
	MarkNotificationReadRequest(facebook_client *fc, const char *id) :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/ajax/notifications/mark_read.php")
	{
		Url
			<< "__a=1"
			<< "seen=0"
			<< "asyncSignal="
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< CHAR_VALUE("alert_ids%5B0%5D", ptrA(mir_urlEncode(id)));
	}
};

#endif //_FACEBOOK_REQUEST_NOTIFICATIONS_H_
