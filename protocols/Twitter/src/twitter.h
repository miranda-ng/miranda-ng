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

using std::string;
using std::wstring;
using std::map;
using std::vector;

#include "http.h"

#define tstring wstring 

typedef unsigned long long twitter_id;
typedef std::map<std::wstring, std::wstring> OAuthParameters;

struct twitter_status
{
	std::string text;
	twitter_id id;
	time_t time;
};

struct twitter_user
{
	std::string username;
	std::string real_name;
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
	typedef std::map<std::string,status_list> status_map;

	twitter();

	bool twitter::set_credentials(const std::string&, const std::wstring&, const std::wstring&, 
		const std::wstring&, const std::wstring&, const std::wstring &, bool);

	http::response twitter::request_token();
	http::response twitter::request_access_tokens();


	void set_base_url(const std::string &base_url);

	const std::string & get_username() const;
	const std::string & get_base_url() const;

	bool get_info(const std::string &name,twitter_user *);
	bool get_info_by_email(const std::string &email,twitter_user *);
	std::vector<twitter_user> get_friends();
	
	//js::array buildFriendList();

	twitter_user add_friend(const std::string &name);
	void remove_friend(const std::string &name);

	void set_status(const std::string &text);
	std::vector<twitter_user> get_statuses(int count=20,twitter_id id=0);

	void send_direct(const std::string &name,const std::string &text);
	std::vector<twitter_user> get_direct(twitter_id id=0);

	std::string urlencode(const std::string &c);
	std::wstring UrlEncode( const std::wstring& url );
	std::string char2hex( char dec );

protected:
	virtual http::response slurp(const std::string &,http::method,
		 OAuthParameters postParams = OAuthParameters()) = 0;

	std::string username_;
	std::string password_;
	std::string base_url_;
	std::wstring consumerKey_;
	std::wstring consumerSecret_;
	std::wstring oauthAccessToken_;
	std::wstring oauthAccessTokenSecret_;
	std::wstring pin_;
};
