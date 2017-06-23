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

#ifndef _FACEBOOK_REQUEST_HISTORY_H_
#define _FACEBOOK_REQUEST_HISTORY_H_

// getting thread info and messages
// revised 17.8.2016
class ThreadInfoRequest : public HttpRequest
{
public:
	// Request only messages history
	ThreadInfoRequest(facebook_client *fc, bool isChat, const char *id, int offset, const char *timestamp, int limit) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/thread_info.php")
	{
		Url
			<< "dpr=1";

		setCommonBody(fc);

		const char *type = isChat ? "thread_fbids" : "user_ids";
		std::string id_ = id; // FIXME: Rewrite this without std::string...
		if (isChat) {
			// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
			if (id_.substr(0, 3) == "id.")
				id_ = id_.substr(3);
		}
		ptrA idEncoded(mir_urlEncode(id_.c_str()));

		//if (loadMessages) {
			// Grrr, offset doesn't work at all, we need to use timestamps to get back in history...
			// And we don't know, what's timestamp of first message, so we need to get from latest to oldest
			CMStringA begin(::FORMAT, "messages[%s][%s]", type, idEncoded);

			Body
				<< CMStringA(::FORMAT, "%s[offset]=%i", begin.c_str(), offset).c_str()
				<< CMStringA(::FORMAT, "%s[timestamp]=%s", begin.c_str(), timestamp).c_str()
				<< CMStringA(::FORMAT, "%s[limit]=%i", begin.c_str(), limit).c_str();
		//}

		/*if (loadThreadInfo) {
			data += "&threads[" + type + "][0]=" + idEncoded;
		}*/
	}

	// Request only thread info // TODO: Make it array of ids
	ThreadInfoRequest(facebook_client *fc, bool isChat, const char *id) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/thread_info.php")
	{
		Url
			<< "dpr=1";

		setCommonBody(fc);

		const char *type = isChat ? "thread_fbids" : "user_ids";
		std::string id_ = id; // FIXME: Rewrite this without std::string...
		if (isChat) {
			// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
			if (id_.substr(0, 3) == "id.")
				id_ = id_.substr(3);
		}
		ptrA idEncoded(mir_urlEncode(id_.c_str()));

		// Load only thread info
		Body << CMStringA(::FORMAT, "threads[%s][0]=%s", type, idEncoded).c_str();
	}

	// Request both thread info and messages for single contact/chat
	ThreadInfoRequest(facebook_client *fc, bool isChat, const char *id, int limit) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/thread_info.php")
	{
		Url
			<< "dpr=1";

		setCommonBody(fc);

		const char *type = isChat ? "thread_fbids" : "user_ids";
		std::string id_ = id; // FIXME: Rewrite this without std::string...
		if (isChat) {
			// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
			if (id_.substr(0, 3) == "id.")
				id_ = id_.substr(3);
		}
		ptrA idEncoded(mir_urlEncode(id_.c_str()));

		// Load messages
		CMStringA begin(::FORMAT, "messages[%s][%s]", type, idEncoded);

		Body
			<< CMStringA(::FORMAT, "%s[offset]=%i", begin.c_str(), 0).c_str()
			<< CMStringA(::FORMAT, "%s[timestamp]=%s", begin.c_str(), "").c_str()
			<< CMStringA(::FORMAT, "%s[limit]=%i", begin.c_str(), limit).c_str();

		// Load thread info
		Body << CMStringA(::FORMAT, "threads[%s][0]=%s", type, idEncoded).c_str();
	}

	// Request both thread info and messages for more threads
	ThreadInfoRequest(facebook_client *fc, const LIST<char> &ids, int offset, int limit) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/thread_info.php")
	{
		Url
			<< "dpr=1";

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
				<< CMStringA(::FORMAT, "%s[offset]=%i", begin.c_str(), offset).c_str()
				//<< CMStringA(::FORMAT, "%s[timestamp]=%s", begin.c_str(), "").c_str()
				<< CMStringA(::FORMAT, "%s[limit]=%i", begin.c_str(), limit).c_str();

			// Load thread info
			Body << CMStringA(::FORMAT, "threads[%s][%i]=%s", "thread_fbids", i, idEncoded).c_str();
		}
	}

private:
	void setCommonBody(facebook_client *fc)
	{
		Body
			<< "client=mercury"
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< "__a=1"
			<< "__pc=PHASED:DEFAULT"
			<< "__be=-1";
	}
};

// getting unread threads
// revised 17.8.2016
class UnreadThreadsRequest : public HttpRequest
{
public:
	UnreadThreadsRequest(facebook_client *fc) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/unread_threads.php")
	{
		Url
			<< "dpr=1";

		Body
			<< "folders[0]=inbox"
			<< "folders[1]=other" // TODO: "other" is probably unused, and there is now "pending" instead
			<< "client=mercury"
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< "__a=1"
			<< "__pc=PHASED:DEFAULT"
			<< "__be=-1";
	}
};

#endif //_FACEBOOK_REQUEST_HISTORY_H_
