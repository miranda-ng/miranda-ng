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

// getting newsfeed posts
HttpRequest* facebook_client::newsfeedRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/ajax/home/generic.php");

	BYTE feed_type = parent->getByte(FACEBOOK_KEY_FEED_TYPE, 0);
	if (feed_type >= _countof(feed_types))
		feed_type = 0;

	p << CHAR_PARAM("sk", feed_types[feed_type].id)
		<< CHAR_PARAM("key", (feed_type < 2) ? "nf" : feed_types[feed_type].id)
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< INT_PARAM("__a", 1);

	return p;
}

HttpRequest* facebook_client::memoriesRequest()
{
	HttpRequest * p = new HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/onthisday/story/query/");

	p << INT_PARAM("__a", 1)
		<< INT_PARAM("start_index", 0)
		<< INT_PARAM("num_stories", 20)
		<< INT_PARAM("last_section_header", 0)
		<< CHAR_PARAM("last_section_key", "regular_story")
		<< INT_PARAM("__be", -1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT64_PARAM("timestamp", ::time(0) * 1000)
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("__user", self_.user_id.c_str());

	return p;
}
