/*
Copyright © 2012-15 Miranda NG team
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

#include "stdafx.h"
#include "twitter.h"
#include "utility.h"

// utility functions

twitter::twitter() : base_url_("https://api.twitter.com/")
{}

bool twitter::set_credentials(const std::string &username, const std::wstring &consumerKey, const std::wstring &consumerSecret,
	const std::wstring &oauthAccessToken, const std::wstring &oauthAccessTokenSecret, const std::wstring &pin, bool test)
{
	username_ = username;
	consumerKey_ = consumerKey;
	consumerSecret_ = consumerSecret;
	oauthAccessToken_ = oauthAccessToken;
	oauthAccessTokenSecret_ = oauthAccessTokenSecret;
	pin_ = pin;

	if (test)
		return slurp(base_url_ + "1.1/account/verify_credentials.json", http::get).code == 200;
	else
		return true;
}

http::response twitter::request_token()
{
	return slurp("https://api.twitter.com/oauth/request_token", http::get);
}

http::response twitter::request_access_tokens()
{
	return slurp("https://api.twitter.com/oauth/access_token", http::get);
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
	http::response resp = slurp(base_url_ + "1.1/statuses/friends.json", http::get);

	if (resp.code != 200)
		throw bad_response();

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root)
		throw std::exception("unable to parse response");

	for (auto it = root.begin(); it != root.end(); ++it) {
		twitter_user user;
		user.username = (*it)["screen_name"].as_string();
		user.real_name = (*it)["name"].as_string();
		user.profile_image_url = (*it)["profile_image_url"].as_string();

		const JSONNode &pStatus = (*it)["status"];
		if (pStatus) {
			user.status.text = pStatus["text"].as_string();
			user.status.id = str2int(pStatus["id"].as_string());

			std::string timestr = pStatus["created_at"].as_string();
			user.status.time = parse_time(timestr);
		}

		friends.push_back(user);
	}

	return friends;
}

bool twitter::get_info(const std::string &name, twitter_user *info)
{
	if (!info)
		return false;

	std::string url = base_url_ + "1.1/users/show/" + http::url_encode(name) + ".json";

	http::response resp = slurp(url, http::get);
	if (resp.code != 200)
		throw bad_response();

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root)
		return false;

	if (root.at("error"))
		return false;

	info->username = root["screen_name"].as_string();
	info->real_name = root["name"].as_string();
	info->profile_image_url = root["profile_image_url"].as_string();
	return true;
}

bool twitter::get_info_by_email(const std::string &email, twitter_user *info)
{
	if (!info)
		return false;

	std::string url = base_url_ + "1.1/users/show.json?email=" + http::url_encode(email);

	http::response resp = slurp(url, http::get);
	if (resp.code != 200)
		throw bad_response();

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root)
		return false;

	if (root["error"] != NULL)
		return false;

	info->username = root["screen_name"].as_string();
	info->real_name = root["name"].as_string();
	info->profile_image_url = root["profile_image_url"].as_string();
	return true;
}

twitter_user twitter::add_friend(const std::string &name)
{
	std::string url = base_url_ + "1.1/friendships/create/" + http::url_encode(name) + ".json";

	http::response resp = slurp(url, http::post);
	if (resp.code != 200)
		throw bad_response();

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root)
		throw std::exception("unable to parse response");

	twitter_user ret;
	ret.username = root["screen_name"].as_string();
	ret.real_name = root["name"].as_string();
	ret.profile_image_url = root["profile_image_url"].as_string();

	JSONNode &pStatus = root["status"];
	if (pStatus) {
		ret.status.text = pStatus["text"].as_string();
		ret.status.id = str2int(pStatus["id"].as_string());
	}

	return ret;
}

void twitter::remove_friend(const std::string &name)
{
	std::string url = base_url_ + "1.1/friendships/destroy/" + http::url_encode(name) + ".json";

	slurp(url, http::post);
}

void twitter::set_status(const std::string &text)
{
	if (text.size()) {
		std::wstring wTweet = UTF8ToWide(text);
		OAuthParameters postParams;
		postParams[L"status"] = UrlEncode(wTweet);

		slurp(base_url_ + "1.1/statuses/update.json", http::post, postParams);
	}
}

void twitter::send_direct(const std::string &name, const std::string &text)
{
	std::wstring temp = UTF8ToWide(text);
	OAuthParameters postParams;
	postParams[L"text"] = UrlEncode(temp);
	postParams[L"screen_name"] = UTF8ToWide(name);
	slurp(base_url_ + "1.1/direct_messages/new.json", http::post, postParams);
}

std::vector<twitter_user> twitter::get_statuses(int count, twitter_id id)
{
	std::vector<twitter_user> statuses;

	std::string url = base_url_ + "1.1/statuses/home_timeline.json?count=" +
		int2str(count);
	if (id != 0)
		url += "&since_id=" + int2str(id);

	http::response resp = slurp(url, http::get);
	if (resp.code != 200)
		throw bad_response();

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root)
		throw std::exception("unable to parse response");

	const JSONNode &pNodes = root.as_array();
	for (auto it = pNodes.begin(); it != pNodes.end(); ++it) {
		const JSONNode &pUser = (*it)["user"];

		twitter_user u;
		u.username = pUser["screen_name"].as_string();
		u.profile_image_url = pUser["profile_image_url"].as_string();

		// the tweet will be truncated unless we take action.  i hate you twitter API
		const JSONNode &pStatus = (*it)["retweeted_status"];
		if (pStatus) {
			// here we grab the "retweeted_status" um.. section?  it's in here that all the info we need is
			// at this point the user will get no tweets and an error popup if the tweet happens to be exactly 140 chars, start with
			// "RT @", end in " ...", and notactually be a real retweet.  it's possible but unlikely, wish i knew how to get
			// the retweet_count variable to work :(
			const JSONNode &pUser2 = pStatus["user"];
			std::string retweeteesName = pUser2["screen_name"].as_string(); // the user that is being retweeted
			std::string retweetText = pUser2["text"].as_string(); // their tweet in all it's untruncated glory

			// fix "&amp;" in the tweets :(
			for (size_t pos = 0; (pos = retweetText.find("&amp;", pos)) != std::string::npos; pos++)
				retweetText.replace(pos, 5, "&");

			u.status.text = "RT @" + retweeteesName + " " + retweetText; // mash it together in some format people will understand
		}
		else {
			// if it's not truncated, then the twitter API returns the native RT correctly anyway,        
			std::string rawText = (*it)["text"].as_string();
			// ok here i'm trying some way to fix all the "&amp;" things that are showing up
			// i dunno why it's happening, so i'll just find and replace each occurance :/
			for (size_t pos = 0; (pos = rawText.find("&amp;", pos)) != std::string::npos; pos++)
				rawText.replace(pos, 5, "&");

			u.status.text = rawText;
		}

		u.status.id = str2int((*it)["id"].as_string());
		std::string timestr = (*it)["created_at"].as_string();
		u.status.time = parse_time(timestr);

		statuses.push_back(u);
	}


	return statuses;
}

std::vector<twitter_user> twitter::get_direct(twitter_id id)
{
	std::vector<twitter_user> messages;

	std::string url = base_url_ + "1.1/direct_messages.json";
	if (id != 0)
		url += "?since_id=" + int2str(id);

	http::response resp = slurp(url, http::get);
	if (resp.code != 200)
		throw bad_response();

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root)
		throw std::exception("unable to parse response");

	const JSONNode &pNodes = root.as_array();
	for (auto it = pNodes.begin(); it != pNodes.end(); ++it) {
		twitter_user u;
		u.username = (*it)["sender_screen_name"].as_string();

		u.status.text = (*it)["text"].as_string();
		u.status.id = str2int((*it)["id"].as_string());
		std::string timestr = (*it)["created_at"].as_string();
		u.status.time = parse_time(timestr);
		messages.push_back(u);
	}

	return messages;
}

string twitter::urlencode(const string &c)
{

	string escaped;
	size_t max = c.length();
	for (int i = 0; i < max; i++) {
		if ((48 <= c[i] && c[i] <= 57) ||//0-9
			(65 <= c[i] && c[i] <= 90) ||//ABC...XYZ
			(97 <= c[i] && c[i] <= 122) || //abc...xyz
			(c[i] == '~' || c[i] == '-' || c[i] == '_' || c[i] == '.'))
		{
			escaped.append(&c[i], 1);
		}
		else {
			escaped.append("%");
			escaped.append(char2hex(c[i]));//converts char 255 to string "FF"
		}
	}
	return escaped;
}


wstring twitter::UrlEncode(const wstring& url)
{
	// multiple encodings r sux
	return UTF8ToWide(urlencode(WideToUTF8(url)));
}

// char2hex and urlencode from http://www.zedwood.com/article/111/cpp-urlencode-function
// modified according to http://oauth.net/core/1.0a/#encoding_parameters
//
//5.1.  Parameter Encoding
//
//All parameter names and values are escaped using the [RFC3986]  
//percent-encoding (%xx) mechanism. Characters not in the unreserved character set 
//MUST be encoded. Characters in the unreserved character set MUST NOT be encoded. 
//Hexadecimal characters in encodings MUST be upper case. 
//Text names and values MUST be encoded as UTF-8 
// octets before percent-encoding them per [RFC3629].
//
//  unreserved = ALPHA, DIGIT, '-', '.', '_', '~'

string twitter::char2hex(char dec)
{
	char dig1 = (dec & 0xF0) >> 4;
	char dig2 = (dec & 0x0F);
	if (0 <= dig1 && dig1 <= 9) dig1 += 48;    //0,48 in ascii
	if (10 <= dig1 && dig1 <= 15) dig1 += 65 - 10; //A,65 in ascii
	if (0 <= dig2 && dig2 <= 9) dig2 += 48;
	if (10 <= dig2 && dig2 <= 15) dig2 += 65 - 10;

	string r;
	r.append(&dig1, 1);
	r.append(&dig2, 1);
	return r;
}

// Some Unices get this, now we do too!
time_t timegm(struct tm *t)
{
	_tzset();
	t->tm_sec -= _timezone;
	t->tm_isdst = 0;

	return mktime(t);
}

static char *month_names[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

int parse_month(const char *m)
{
	for (int i = 0; i < 12; i++)
		if (mir_strcmp(month_names[i], m) == 0)
			return i;

	return -1;
}

time_t parse_time(const std::string &s)
{
	struct tm t;
	char day[4], month[4];
	char plus;
	int zone;
	if (sscanf(s.c_str(), "%3s %3s %d %d:%d:%d %c%d %d", day, month, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &plus, &zone, &t.tm_year) == 9) {
		t.tm_year -= 1900;
		t.tm_mon = parse_month(month);
		if (t.tm_mon == -1)
			return 0;
		return timegm(&t);
	}
	return 0;
}
