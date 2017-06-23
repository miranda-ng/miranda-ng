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

#ifndef _FACEBOOK_REQUEST_FEEDS_H_
#define _FACEBOOK_REQUEST_FEEDS_H_

// getting newsfeed posts
class NewsfeedRequest : public HttpRequest
{
public:
	NewsfeedRequest(facebook_client *fc) :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/ajax/home/generic.php")
	{
		Url
			<< fc->get_newsfeed_type().c_str()
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< "__a=1";
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
			<< "__a=1"
			<< "start_index=0"
			<< "num_stories=20"
			<< "last_section_header=0"
			<< "last_section_key=regular_story"
			<< "__af="
			<< "__be=-1"
			<< "__pc=PHASED:DEFAULT"
			<< LONG_VALUE("timestamp", ::time(NULL))
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str());
	}
};

#endif //_FACEBOOK_REQUEST_FEEDS_H_
