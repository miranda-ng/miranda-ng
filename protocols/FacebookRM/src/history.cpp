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

HttpRequest* facebook_client::threadInfoRequest(bool isChat, const char *id, const char* timestamp, int limit, bool loadMessages)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/api/graphqlbatch/");

	p->Body
		<< CHAR_PARAM("batch_name", "MessengerGraphQLThreadFetcherRe")
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< INT_PARAM("__be", 1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str());

	std::string id_ = id;
	if (isChat) {
		if (id_.substr(0, 3) == "id.")
			id_ = id_.substr(3);
	}

	JSONNode root, o0, query_params;
	query_params
		<< CHAR_PARAM("id", id_.c_str())
		<< INT_PARAM("message_limit", (limit == -1) ? 50 : limit)
		<< BOOL_PARAM("load_messages", loadMessages)
		<< BOOL_PARAM("load_read_receipts", false);

	if (timestamp == nullptr || strcmp(timestamp, "") == 0)
		query_params << NULL_PARAM("before");
	else
		query_params << CHAR_PARAM("before", timestamp);

	o0 << CHAR_PARAM("doc_id", "1549485615075443") << JSON_PARAM("query_params", query_params);
	root << JSON_PARAM("o0", o0);

	p->Body << CHAR_PARAM("queries", root.write().c_str());

	return p;
}

// Request both thread info and messages for more threads
HttpRequest* facebook_client::threadInfoRequest(const LIST<char> &ids, int limit)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/api/graphqlbatch/");

	p->Body
		<< CHAR_PARAM("batch_name", "MessengerGraphQLThreadFetcherRe")
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< INT_PARAM("__be", 1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str());

	JSONNode root;
	for (int i = 0; i < ids.getCount(); i++) {
		// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
		std::string id_ = ids[i]; // FIXME: Rewrite this without std::string...
		if (id_.substr(0, 3) == "id.")
			id_ = id_.substr(3);

		// Build query
		JSONNode oX, query_params;
		query_params
			<< CHAR_PARAM("id", id_.c_str())
			<< INT_PARAM("message_limit", limit)
			<< BOOL_PARAM("load_messages", true)
			<< BOOL_PARAM("load_read_receipts", false)
			<< NULL_PARAM("before");

		oX << CHAR_PARAM("doc_id", "1508526735892416") << JSON_PARAM("query_params", query_params);
		root << JSON_PARAM(("o" + std::to_string(i)).c_str(), oX);
	}

	p->Body << CHAR_PARAM("queries", root.write().c_str());

	return p;
}

HttpRequest* facebook_client::unreadThreadsRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/unread_threads.php");

	p << INT_PARAM("dpr", 1);

	p->Body
		<< CHAR_PARAM("folders[0]", "inbox")
		<< CHAR_PARAM("folders[1]", "other") // TODO: "other" is probably unused, and there is now "pending" instead
		<< CHAR_PARAM("client", "mercury")
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__be", -1);

	return p;
}
