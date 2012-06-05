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

#pragma once

#include <map>
#include <vector>
#include <string>

#include "http.h"
#include <tchar.h>

#define tstring wstring 
typedef unsigned long long twitter_id;

struct twitter_status
{
	std::tstring text;
	twitter_id id;
	time_t time;
};

struct twitter_user
{
	std::string username;
	std::tstring real_name;
	std::string profile_image_url;
	twitter_status status;
};

time_t parse_time(const std::string &);

class bad_response : public std::exception
{
public:
	virtual const char * what() const
	{
		return "bad http response";
	}
};

class twitter
{
public:
	typedef std::vector<twitter_user> status_list;
	typedef std::map<std::tstring,status_list> status_map;

	twitter();

	bool set_credentials(const std::string &username,const std::string &password, bool test = true);
	void set_base_url(const std::string &base_url);

	const std::string & get_username() const;
	const std::string & get_base_url() const;

	bool get_info(const std::tstring &name,twitter_user *);
	bool get_info_by_email(const std::tstring &email,twitter_user *);
	std::vector<twitter_user> get_friends();

	twitter_user add_friend(const std::tstring &name);
	void remove_friend(const std::tstring &name);

	void set_status(const std::tstring &text);
	std::vector<twitter_user> get_statuses(int count=20,twitter_id id=0);

	void send_direct(const std::tstring &name,const std::tstring &text);
	std::vector<twitter_user> get_direct(twitter_id id=0);

protected:
	virtual http::response slurp(const std::string &,http::method, const std::string & = "") = 0;

	std::string username_;
	std::string password_;
	std::string base_url_;
};