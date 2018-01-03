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

#ifndef _FACEBOOK_REQUEST_FEEDS_H_
#define _FACEBOOK_REQUEST_FEEDS_H_

// getting newsfeed posts
class NewsfeedRequest : public HttpRequest
{
public:
	NewsfeedRequest(facebook_client *fc) :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/ajax/home/generic.php")
	{
		BYTE feed_type = fc->parent->getByte(FACEBOOK_KEY_FEED_TYPE, 0);
		if (feed_type >= _countof(feed_types))
			feed_type = 0;

		Url
			<< CHAR_PARAM("sk", feed_types[feed_type].id)
			<< CHAR_PARAM("key", (feed_type < 2) ? "nf" : feed_types[feed_type].id)
			<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
			<< INT_PARAM("__a", 1);
	}
};

// getting memories ("on this day") posts
class MemoriesRequest : public HttpRequest
{
public:
	MemoriesRequest(facebook_client *fc) :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/onthisday/story/query/")
	{
		Url
			<< INT_PARAM("__a", 1)
			<< INT_PARAM("start_index", 0)
			<< INT_PARAM("num_stories", 20)
			<< INT_PARAM("last_section_header", 0)
			<< CHAR_PARAM("last_section_key", "regular_story")
			<< INT_PARAM("__be", -1)
			<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
			<< INT64_PARAM("timestamp", ::time(nullptr))
			<< CHAR_PARAM("__dyn", fc->__dyn())
			<< CHAR_PARAM("__req", fc->__req())
			<< CHAR_PARAM("__rev", fc->__rev())
			<< CHAR_PARAM("__user", fc->self_.user_id.c_str());
	}
};

#endif //_FACEBOOK_REQUEST_FEEDS_H_
