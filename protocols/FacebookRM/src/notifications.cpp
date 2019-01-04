/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-19 Miranda NG team

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
// getting notifications

HttpRequest* facebook_client::getNotificationsRequest(int count)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/notifications/client/get.php");
	
	p << INT_PARAM("dpr", 1);

	p->Body
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< INT_PARAM("length", count) // number of items to load
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__be", -1)
		<< INT_PARAM("__a", 1);

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// marking notifications read

HttpRequest* facebook_client::markNotificationReadRequest(const char *id)
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/ajax/notifications/mark_read.php");

	p << INT_PARAM("__a", 1)
		<< INT_PARAM("seen", 0)
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("alert_ids[0]", id);

	return p;
}
