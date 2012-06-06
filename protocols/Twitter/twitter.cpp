/*
Copyright © 2009 Jim Porter

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

//#include "common.h"

#include <windows.h>
#include <cstring>
#include <sstream>
#include <ctime>

#include "twitter.h"

#include "tinyjson.hpp"
#include <boost/lexical_cast.hpp>

typedef json::grammar<char> js;

// utility functions

template <typename T>
static T cast_and_decode(boost::any &a,bool allow_null)
{
	if (allow_null && a.type() == typeid(void))
		return T();
	return boost::any_cast<T>(a);
}

template <>
static std::string cast_and_decode<std::string>(boost::any &a,bool allow_null)
{
	if (allow_null && a.type() == typeid(void))
		return std::string();
	std::string s = boost::any_cast<std::string>(a);

	// Twitter *only* encodes < and >, so decode them
	size_t off;
	while( (off = s.find("&lt;")) != std::string::npos)
		s.replace(off,4,"<");
	while( (off = s.find("&gt;")) != std::string::npos)
		s.replace(off,4,">");

	return s;
}

template <typename T>
static T retrieve(const js::object &o,const std::string &key,bool allow_null = false)
{
	using boost::any_cast;

	js::object::const_iterator i = o.find(key);
	if (i == o.end())
		throw std::exception( ("unable to retrieve key '"+key+"'").c_str());
	try
	{
		return cast_and_decode<T>(*i->second,allow_null);
	}
	catch(const boost::bad_any_cast &)
	{
		throw std::exception( ("unable to cast key '"+key+"' to target type").c_str());
	}
}



twitter::twitter() : base_url_("https://twitter.com/")
{}

bool twitter::set_credentials(const std::string &username,const std::string &password,
	bool test)
{
	username_ = username;
	password_ = password;

	if (test)
		return slurp(base_url_+"account/verify_credentials.json",http::get).code == 200;
	else
		return true;
}

void twitter::set_base_url(const std::string &base_url)
{
	base_url_ = base_url;
}

const std::string & twitter::get_username() const
{
	return username_;
}

const std::string & twitter::get_base_url() const
{
	return base_url_;
}

std::vector<twitter_user> twitter::get_friends()
{
	std::vector<twitter_user> friends;
	http::response resp = slurp(base_url_+"statuses/friends.json",http::get);

	if (resp.code != 200)
		throw bad_response();

	const js::variant var = json::parse( resp.data.begin(),resp.data.end());
	if (var->type() != typeid(js::array))
		throw std::exception("unable to parse response");

	const js::array &list = boost::any_cast<js::array>(*var);
	for(js::array::const_iterator i=list.begin(); i!=list.end(); ++i)
	{
		if ((*i)->type() == typeid(js::object))
		{
			const js::object &one = boost::any_cast<js::object>(**i);

			twitter_user user;
			user.username          = retrieve<std::string>(one,"screen_name");
			user.real_name         = retrieve<std::tstring>(one,"name",true);
			user.profile_image_url = retrieve<std::string>(one,"profile_image_url",true);

			if (one.find("status") != one.end())
			{
				js::object &status  = retrieve<js::object>(one,"status");
				user.status.text    = retrieve<std::tstring>(status,"text");

				user.status.id      = retrieve<long long>(status,"id");

				std::string timestr = retrieve<std::string>(status,"created_at");
				user.status.time    = parse_time(timestr);
			}

			friends.push_back(user);
		}
	}

	return friends;
}

bool twitter::get_info(const std::tstring &name,twitter_user *info)
{
	if (!info)
		return false;

	std::string url = base_url_+"users/show/"+http::url_encode(name)+".json";

	http::response resp = slurp(url,http::get);
	if (resp.code != 200)
		throw bad_response();

	const js::variant var = json::parse( resp.data.begin(),resp.data.end());
	if (var->type() == typeid(js::object))
	{
		const js::object &user_info = boost::any_cast<js::object>(*var);
		if (user_info.find("error") != user_info.end())
			return false;

		info->username          = retrieve<std::string>(user_info,"screen_name");
		info->real_name         = retrieve<std::tstring>(user_info,"name",true);
		info->profile_image_url = retrieve<std::string>(user_info,"profile_image_url",true);

		return true;
	}
	else
		return false;
}

bool twitter::get_info_by_email(const std::tstring &email,twitter_user *info)
{
	if (!info)
		return false;

	std::string url = base_url_+"users/show.json?email="+http::url_encode(email);

	http::response resp = slurp(url,http::get);
	if (resp.code != 200)
		throw bad_response();

	js::variant var = json::parse( resp.data.begin(),resp.data.end());
	if (var->type() == typeid(js::object))
	{
		const js::object &user_info = boost::any_cast<js::object>(*var);
		if (user_info.find("error") != user_info.end())
			return false;

		info->username          = retrieve<std::string>(user_info,"screen_name");
		info->real_name         = retrieve<std::tstring>(user_info,"name",true);
		info->profile_image_url = retrieve<std::string>(user_info,"profile_image_url",true);

		return true;
	}
	else
		return false;
}

twitter_user twitter::add_friend(const std::tstring &name)
{
	std::string url = base_url_+"friendships/create/"+http::url_encode(name)+".json";

	twitter_user ret;
	http::response resp = slurp(url,http::post);
	if (resp.code != 200)
		throw bad_response();

	js::variant var = json::parse( resp.data.begin(),resp.data.end());
	if (var->type() != typeid(js::object))
		throw std::exception("unable to parse response");

	const js::object &user_info = boost::any_cast<js::object>(*var);
	ret.username          = retrieve<std::string>(user_info,"screen_name");
	ret.real_name         = retrieve<std::tstring>(user_info,"name",true);
	ret.profile_image_url = retrieve<std::string>(user_info,"profile_image_url",true);

	if (user_info.find("status") != user_info.end())
	{
		// TODO: fill in more fields
		const js::object &status = retrieve<js::object>(user_info,"status");
		ret.status.text          = retrieve<std::tstring>(status,"text");
	}

	return ret;
}

void twitter::remove_friend(const std::tstring &name)
{
	std::string url = base_url_+"friendships/destroy/"+http::url_encode(name)+".json";

	slurp(url,http::post);
}

void twitter::set_status(const std::tstring &text)
{
	if (text.size())
	{
		slurp(base_url_+"statuses/update.json",http::post,
			"status="+http::url_encode(text)+
			"&source=mirandaim");
	}
}

void twitter::send_direct(const std::tstring &name,const std::tstring &text)
{
	slurp(base_url_+"direct_messages/new.json",http::post,
		"user=" +http::url_encode(name)+
		"&text="+http::url_encode(text));
}

std::vector<twitter_user> twitter::get_statuses(int count,twitter_id id)
{
	using boost::lexical_cast;
	std::vector<twitter_user> statuses;

	std::string url = base_url_+"statuses/friends_timeline.json?count="+
		lexical_cast<std::string>(count);
	if (id != 0)
		url += "&since_id="+boost::lexical_cast<std::string>(id);

	http::response resp = slurp(url,http::get);
	if (resp.code != 200)
		throw bad_response();

	js::variant var = json::parse( resp.data.begin(),resp.data.end());
	if (var->type() != typeid(js::array))
		throw std::exception("unable to parse response");

	const js::array &list = boost::any_cast<js::array>(*var);
	for(js::array::const_iterator i=list.begin(); i!=list.end(); ++i)
	{
		if ((*i)->type() == typeid(js::object))
		{
			const js::object &one  = boost::any_cast<js::object>(**i);
			const js::object &user = retrieve<js::object>(one,"user");

			twitter_user u;
			u.username = retrieve<std::string>(user,"screen_name");

			u.status.text       = retrieve<std::tstring>(one,"text");
			u.status.id         = retrieve<long long>(one,"id");
			std::string timestr = retrieve<std::string>(one,"created_at");
			u.status.time       = parse_time(timestr);

			statuses.push_back(u);
		}
	}


	return statuses;
}

std::vector<twitter_user> twitter::get_direct(twitter_id id)
{
	std::vector<twitter_user> messages;

	std::string url = base_url_+"direct_messages.json";
	if (id != 0)
		url += "?since_id="+boost::lexical_cast<std::string>(id);

	http::response resp = slurp(url,http::get);
	if (resp.code != 200)
		throw bad_response();

	js::variant var = json::parse( resp.data.begin(),resp.data.end());
	if (var->type() != typeid(js::array))
		throw std::exception("unable to parse response");

	const js::array &list = boost::any_cast<js::array>(*var);
	for(js::array::const_iterator i=list.begin(); i!=list.end(); ++i)
	{
		if ((*i)->type() == typeid(js::object))
		{
			const js::object &one = boost::any_cast<js::object>(**i);

			twitter_user u;
			u.username = retrieve<std::string>(one,"sender_screen_name");

			u.status.text       = retrieve<std::tstring>(one,"text");
			u.status.id         = retrieve<long long>(one,"id");
			std::string timestr = retrieve<std::string>(one,"created_at");
			u.status.time       = parse_time(timestr);

			messages.push_back(u);
		}
	}


	return messages;
}

// Some Unices get this, now we do too!
time_t timegm(struct tm *t)
{
	_tzset();
	t->tm_sec -= _timezone;
	t->tm_isdst = 0;

	return mktime(t);
}

static char *month_names[] = { "Jan","Feb","Mar","Apr","May","Jun",
                               "Jul","Aug","Sep","Oct","Nov","Dec" };

int parse_month(const char *m)
{
	for(size_t i=0; i<12; i++)
	{
		if (strcmp(month_names[i],m) == 0)
			return i;
	}
	return -1;
}

time_t parse_time(const std::string &s)
{
	struct tm t;
	char day[4],month[4];
	char plus;
	int zone;
	if (sscanf(s.c_str(),"%3s %3s %d %d:%d:%d %c%d %d",
		day,month,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec,
		&plus,&zone,&t.tm_year) == 9)
	{
		t.tm_year -= 1900;
		t.tm_mon = parse_month(month);
		if (t.tm_mon == -1)
			return 0;
		return timegm(&t);
	}
	return 0;
}