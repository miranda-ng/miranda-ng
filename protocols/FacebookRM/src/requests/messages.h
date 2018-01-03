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

#ifndef _FACEBOOK_REQUEST_MESSAGES_H_
#define _FACEBOOK_REQUEST_MESSAGES_H_

// sending messages
// revised 17.8.2016
class SendMessageRequest : public HttpRequest
{
public:
	SendMessageRequest(facebook_client *fc, const char *userId, const char *threadId, const char *messageId, const char *messageText, bool isChat, const char *captcha, const char *captchaPersistData) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/messaging/send/")
	{
		// Don't notify errors for this request, because we're getting them inline in messaging window
		NotifyErrors = false;

		// Use own persistent connection for sending messages
		Persistent = MESSAGES;

		Url << INT_PARAM("dpr", 1);

		if (mir_strlen(captcha) > 0) {
			Body
				<< CHAR_PARAM("captcha_persist_data", captchaPersistData)
				<< CHAR_PARAM("captcha_response", captcha);
		}

		Body
			<< CHAR_PARAM("client", "mercury") // or "web_messenger" (whole messages page)
			<< CHAR_PARAM("action_type", "ma-type:user-generated-message");

		// Experimental sticker sending support
		std::string message_text = messageText; // FIXME: Rewrite this without std::string...
		if (message_text.substr(0, 10) == "[[sticker:" && message_text.substr(message_text.length() - 2) == "]]") {
			Body
				<< "body="
				<< CHAR_PARAM("sticker_id", ptrA(mir_urlEncode(message_text.substr(10, message_text.length() - 10 - 2).c_str())))
				<< BOOL_PARAM("has_attachment", true);
				// TODO: For sending GIF images instead of "sticker_id=" there is "image_ids[0]=", otherwise it's same
		}
		else {
			Body
				<< CHAR_PARAM("body", ptrA(mir_urlEncode(messageText)))
				<< BOOL_PARAM("has_attachment", false);
		}

		Body
			<< INT_PARAM("ephemeral_ttl_mode", 0)
			<< CHAR_PARAM("message_id", messageId)
			<< CHAR_PARAM("offline_threading_id", messageId); // Same as message ID

		if (isChat) {
			// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
			std::string threadFbid = threadId; // FIXME: Rewrite this without std::string...
			if (threadFbid.substr(0, 3) == "id.")
				threadFbid = threadFbid.substr(3);

			Body << CHAR_PARAM("thread_fbid", threadFbid.c_str());
		}
		else {
			Body
				<< CHAR_PARAM("other_user_fbid", userId)
				<< CHAR_PARAM("specific_to_list[0]", CMStringA(::FORMAT, "fbid:%s", userId))
				<< CHAR_PARAM("specific_to_list[1]", CMStringA(::FORMAT, "fbid:%s", fc->self_.user_id.c_str()));
		}

		Body
			// << "signature_id=" // TODO: How to generate signature ID? It is present only when sending via "mercury"
			<< CHAR_PARAM("source", "source:chat:web") // or "source:titan:web" for web_messenger
			<< CHAR_PARAM("timestamp", utils::time::mili_timestamp().c_str())
			<< CHAR_PARAM("ui_push_phase", "V3")
			<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
			<< CHAR_PARAM("__dyn", fc->__dyn())
			<< CHAR_PARAM("__req", fc->__req())
			<< CHAR_PARAM("__rev", fc->__rev())
			<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str())
			<< INT_PARAM("__a", 1)
			<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
			<< INT_PARAM("__be", -1);
	}
};

// sending typing notification
// revised 17.8.2016
class SendTypingRequest : public HttpRequest
{
public:
	SendTypingRequest(facebook_client *fc, const char *userId, bool isChat, bool isTyping) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/messaging/typ.php")
	{
		Url << INT_PARAM("dpr", 1);

		ptrA idEncoded(mir_urlEncode(userId));

		Body
			<< INT_PARAM("typ", isTyping ? 1 : 0)
			<< CHAR_PARAM("to", isChat ? "" : idEncoded)
			<< CHAR_PARAM("thread", idEncoded)
			<< CHAR_PARAM("source", "mercury-chat")
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
};

// marking messages read
class MarkMessageReadRequest : public HttpRequest
{
public:
	MarkMessageReadRequest(facebook_client *fc, const LIST<char> &ids) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/change_read_status.php")
	{
		Url << INT_PARAM("__a", 1);

		for (int i = 0; i < ids.getCount(); i++) {
			std::string id_ = ids[i];
			// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
			if (id_.substr(0, 3) == "id.")
				id_ = id_.substr(3);

			CMStringA id(::FORMAT, "ids[%s]", ptrA(mir_urlEncode(id_.c_str())));
			Body << BOOL_PARAM(id, true);
		}

		Body
			<< CHAR_PARAM("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_PARAM("__user", fc->self_.user_id.c_str())
			<< CHAR_PARAM("__dyn", fc->__dyn())
			<< CHAR_PARAM("__req", fc->__req())
			<< CHAR_PARAM("__rev", fc->__rev())
			<< INT_PARAM("__a", 1);
	}
};

#endif //_FACEBOOK_REQUEST_MESSAGES_H_
