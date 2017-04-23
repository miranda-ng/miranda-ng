/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2011-17 Robert P�sel

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

#ifndef _FACEBOOK_REQUEST_CHANNEL_H_
#define _FACEBOOK_REQUEST_CHANNEL_H_

// receiving updates, sending activity ping
class ChannelRequest : public HttpRequest
{
public:
	enum Type { PULL, PING };

	ChannelRequest(facebook_client *fc, Type type) :
		HttpRequest(REQUEST_POST, FORMAT,
			type == PULL ? FACEBOOK_SERVER_CHAT "/pull" : FACEBOOK_SERVER_CHAT "/active_ping",
			fc->chat_conn_num_.empty() ? "0" : fc->chat_conn_num_.c_str(),
			fc->chat_channel_host_.c_str())
	{
		if (type == PULL) {
			timeout = 65 * 1000;
			Persistent = CHANNEL;
		}

		bool isPing = (type == PING);

		Url << CHAR_VALUE("channel", fc->chat_channel_.empty() ? ("p_" + fc->self_.user_id).c_str() : fc->chat_channel_.c_str());
		if (!isPing)
			Url << CHAR_VALUE("seq", fc->chat_sequence_num_.empty() ? "0" : fc->chat_sequence_num_.c_str());

		Url
			<< CHAR_VALUE("partition", fc->chat_channel_partition_.empty() ? "0" : fc->chat_channel_partition_.c_str())
			<< CHAR_VALUE("clientid", fc->chat_clientid_.c_str())
			<< CHAR_VALUE("cb", utils::text::rand_string(4, "0123456789abcdefghijklmnopqrstuvwxyz", &fc->random_).c_str());

		/*
		original cb = return (1048576 * Math.random() | 0).toString(36);
		char buffer[10];
		itoa(((int)(1048576 * (((double)rand()) / (RAND_MAX + 1))) | 0), buffer, 36);
		Url << CHAR_VALUE("cb", buffer);
		*/

		int idleSeconds = fc->parent->IdleSeconds();
		Url << INT_VALUE("idle", idleSeconds); // Browser is sending "idle" always, even if it's "0"

		if (!isPing) {
			Url << "qp=y"; // TODO: what's this item?
			Url << "pws=fresh"; // TODO: what's this item?
			Url << "isq=487632"; // TODO: what's this item?
			Url << INT_VALUE("msgs_recv", fc->chat_msgs_recv_);
			// TODO: sometimes there is &tur=1711 and &qpmade=<some actual timestamp> and &isq=487632
			// Url << "request_batch=1"; // it somehow batches up more responses to one - then response has special "t=batched" type and "batches" array with the data
			// Url << "msgr_region=LLA"; // it was here only for first pull, same as request_batch
		}

		Url << "cap=8" // TODO: what's this item? Sometimes it's 0, sometimes 8
			<< CHAR_VALUE("uid", fc->self_.user_id.c_str())
			<< CHAR_VALUE("viewer_uid", fc->self_.user_id.c_str());

		if (!fc->chat_sticky_num_.empty() && !fc->chat_sticky_pool_.empty()) {
			Url << CHAR_VALUE("sticky_token", fc->chat_sticky_num_.c_str());
			Url << CHAR_VALUE("sticky_pool", fc->chat_sticky_pool_.c_str());
		}

		if (!isPing && !fc->chat_traceid_.empty())
			Url << CHAR_VALUE("traceid", fc->chat_traceid_.c_str());

		if (fc->parent->isInvisible())
			Url << "state=offline";
		else if (isPing || idleSeconds < 60)
			Url << "state=active";

		/*
		Body
			<< "persistent=1"
			<< CHAR_VALUE("email", ptrA(mir_urlEncode(username)))
			<< CHAR_VALUE("pass", ptrA(mir_urlEncode(password)))
			<< CHAR_VALUE("lgndim", "eyJ3IjoxOTIwLCJoIjoxMDgwLCJhdyI6MTgzNCwiYWgiOjEwODAsImMiOjMyfQ==") // means base64 encoded: {"w":1920,"h":1080,"aw":1834,"ah":1080,"c":32}
			<< bodyData; // additional data parsed from form*/
	}
};

#endif //_FACEBOOK_REQUEST_CHANNEL_H_
