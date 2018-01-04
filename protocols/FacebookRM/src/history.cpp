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

#include "stdafx.h"

ThreadInfoRequest::ThreadInfoRequest(facebook_client *fc, bool isChat, const char *id, const char* timestamp, int limit) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/api/graphqlbatch/")
{
	setCommonBody(fc);

	// FIXME: FB removed /ajax/mercury/thread_info requests and now all messaging stuff does through /api/graphqlbatch/ - all loading of threads, (unread) messages, list of contacts in groupchat, etc.
	// All these request must be rewritten to the new request. Preparation is below but unfinished.

	//const char *type = isChat ? "thread_fbids" : "user_ids";
	std::string id_ = id; // FIXME: Rewrite this without std::string...
	if (isChat) {
		// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
		if (id_.substr(0, 3) == "id.")
			id_ = id_.substr(3);
	}
	ptrA idEncoded(mir_urlEncode(id_.c_str()));

	JSONNode root, o0, query_params;
	query_params
		<< CHAR_PARAM("id", idEncoded)
		<< INT_PARAM("message_limit", (limit == -1) ? 50 : limit)
		<< INT_PARAM("load_messages", 0)
		<< BOOL_PARAM("load_read_receipts", false);

	if (timestamp != nullptr)
		query_params << CHAR_PARAM("before", timestamp);
	else
		query_params << NULL_PARAM("before");

	o0 << CHAR_PARAM("doc_id", "1549485615075443") << JSON_PARAM("query_params", query_params);
	root << JSON_PARAM("o0", o0);

	Body << CHAR_PARAM("queries", utils::url::encode(root.write()).c_str());
}

// Request both thread info and messages for more threads
ThreadInfoRequest::ThreadInfoRequest(facebook_client *fc, const LIST<char> &ids, int offset, int limit) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/api/graphqlbatch/")
{
	setCommonBody(fc);

	for (int i = 0; i < ids.getCount(); i++) {
		// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
		std::string id_ = ids[i]; // FIXME: Rewrite this without std::string...
		if (id_.substr(0, 3) == "id.")
			id_ = id_.substr(3);
		ptrA idEncoded(mir_urlEncode(id_.c_str()));

		// Load messages
		CMStringA begin(::FORMAT, "messages[%s][%s]", "thread_fbids", idEncoded);
		Body
			<< INT_PARAM(CMStringA(::FORMAT, "%s[offset]", begin.c_str()), offset)
			<< INT_PARAM(CMStringA(::FORMAT, "%s[limit]", begin.c_str()), limit)
			<< CHAR_PARAM(CMStringA(::FORMAT, "threads[%s][%i]", "thread_fbids", i), idEncoded);
	}
}

void ThreadInfoRequest::setCommonBody(facebook_client *fc)
{
	Body
		<< CHAR_PARAM("batch_name", "MessengerGraphQLThreadFetcherRe")
		<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("__dyn", fc->__dyn())
		<< CHAR_PARAM("__req", fc->__req())
		<< INT_PARAM("__be", 1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< CHAR_PARAM("__rev", fc->__rev())
		<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
		<< CHAR_PARAM("jazoest", "265816910476541027556899745586581711208287100122699749108");
}

UnreadThreadsRequest::UnreadThreadsRequest(facebook_client *fc) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/unread_threads.php")
{
	Url << INT_PARAM("dpr", 1);

	Body
		<< CHAR_PARAM("folders[0]", "inbox")
		<< CHAR_PARAM("folders[1]", "other") // TODO: "other" is probably unused, and there is now "pending" instead
		<< CHAR_PARAM("client", "mercury")
		<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", fc->__dyn())
		<< CHAR_PARAM("__req", fc->__req())
		<< CHAR_PARAM("__rev", fc->__rev())
		<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str())
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__be", -1);
}
