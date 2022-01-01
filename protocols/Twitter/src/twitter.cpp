/*
Copyright © 2012-22 Miranda NG team
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

// utility functions

http::response CTwitterProto::request_token()
{
	auto *req = new AsyncHttpRequest(REQUEST_GET, "https://api.twitter.com/oauth/request_token");
	return Execute(req);
}

http::response CTwitterProto::request_access_tokens()
{
	auto *req = new AsyncHttpRequest(REQUEST_GET, "https://api.twitter.com/oauth/access_token");
	return Execute(req);
}

bool CTwitterProto::get_info(const CMStringA &name, twitter_user *info)
{
	if (!info)
		return false;

	auto *req = new AsyncHttpRequest(REQUEST_GET, "/users/show/" + mir_urlEncode(name) + ".json");
	http::response resp = Execute(req);
	if (resp.code != 200)
		return false;

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

bool CTwitterProto::get_info_by_email(const CMStringA &email, twitter_user *info)
{
	if (!info)
		return false;

	auto *req = new AsyncHttpRequest(REQUEST_GET, "/users/show.json?email=" + mir_urlEncode(email));
	http::response resp = Execute(req);
	if (resp.code != 200)
		return false;

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

bool CTwitterProto::add_friend(const CMStringA &name, twitter_user &ret)
{
	auto *req = new AsyncHttpRequest(REQUEST_POST, "/friendships/create/" + mir_urlEncode(name) + ".json");
	http::response resp = Execute(req);
	if (resp.code != 200)
		return false;

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root)
		return false;

	ret.username = root["screen_name"].as_string();
	ret.real_name = root["name"].as_string();
	ret.profile_image_url = root["profile_image_url"].as_string();

	JSONNode &pStatus = root["status"];
	if (pStatus) {
		ret.status.text = pStatus["text"].as_string();
		ret.status.id = _atoi64(pStatus["id"].as_string().c_str());
	}

	return true;
}

void CTwitterProto::remove_friend(const CMStringA &name)
{
	auto *req = new AsyncHttpRequest(REQUEST_POST, "/friendships/destroy/" + mir_urlEncode(name) + ".json");
	Execute(req);
}

void CTwitterProto::mark_read(MCONTACT hContact, const CMStringA &msgId)
{
	CMStringA id(getMStringA(hContact, TWITTER_KEY_ID));
	if (id.IsEmpty())
		return;

	auto *req = new AsyncHttpRequest(REQUEST_POST, "/direct_messages/mark_read.json");
	req << CHAR_PARAM("recipient_id", id) << CHAR_PARAM("last_read_event_id", msgId);
	Execute(req);
}

void CTwitterProto::set_status(const CMStringA &text)
{
	if (text.IsEmpty())
		return;

	auto *req = new AsyncHttpRequest(REQUEST_POST, "/statuses/update.json");
	req << CHAR_PARAM("status", text);
	Execute(req);
}

void CTwitterProto::send_direct(const CMStringA &name, const CMStringA &text)
{
	auto *req = new AsyncHttpRequest(REQUEST_POST, "/direct_messages/events/new.json");
	JSONNode target; target.set_name("target"); target << CHAR_PARAM("recipient_id", name);
	JSONNode msgData; msgData.set_name("message_data"); msgData << CHAR_PARAM("text", text);
	JSONNode msgCreate; msgCreate.set_name("message_create"); msgCreate << msgData << target;
	JSONNode event; event.set_name("event"); event << CHAR_PARAM("type", "message_create") << msgCreate;	
	JSONNode body; body << event;
	req->m_szParam = body.write().c_str();
	Execute(req);
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

time_t parse_time(const CMStringA &s)
{
	struct tm t;
	char day[4], month[4];
	char plus;
	int zone;
	if (sscanf(s, "%3s %3s %d %d:%d:%d %c%d %d", day, month, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &plus, &zone, &t.tm_year) == 9) {
		t.tm_year -= 1900;
		t.tm_mon = parse_month(month);
		if (t.tm_mon == -1)
			return 0;
		return timegm(&t);
	}
	return 0;
}
