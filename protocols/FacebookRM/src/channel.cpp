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

/////////////////////////////////////////////////////////////////////////////////////////
// receiving updates, sending activity ping

HttpRequest* facebook_client::channelRequest(facebook_client::Type type)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FORMAT, FACEBOOK_SERVER_CHAT,
		chat_conn_num_.empty() ? "0" : chat_conn_num_.c_str(),
		chat_channel_host_.c_str(), 
		(type == PULL) ? "pull" : "active_ping");

	if (type == PULL) {
		p->timeout = 65 * 1000;
		p->Persistent = p->CHANNEL;
	}

	bool isPing = (type == PING);

	p << CHAR_PARAM("channel", chat_channel_.empty() ? ("p_" + self_.user_id).c_str() : chat_channel_.c_str());
	if (!isPing)
		p << CHAR_PARAM("seq", chat_sequence_num_.empty() ? "0" : chat_sequence_num_.c_str());

	p << CHAR_PARAM("partition", chat_channel_partition_.empty() ? "0" : chat_channel_partition_.c_str())
		<< CHAR_PARAM("clientid", chat_clientid_.c_str())
		<< CHAR_PARAM("cb", utils::text::rand_string(4, "0123456789abcdefghijklmnopqrstuvwxyz", &random_).c_str());

	int idleSeconds = parent->IdleSeconds();
	p << INT_PARAM("idle", idleSeconds); // Browser is sending "idle" always, even if it's "0"

	if (!isPing) {
		p << CHAR_PARAM("qp", "y") << CHAR_PARAM("pws", "fresh") << INT_PARAM("isq", 487632);
		p << INT_PARAM("msgs_recv", chat_msgs_recv_);
		// TODO: sometimes there is &tur=1711 and &qpmade=<some actual timestamp> and &isq=487632
		// Url << "request_batch=1"; // it somehow batches up more responses to one - then response has special "t=batched" type and "batches" array with the data
		// Url << "msgr_region=LLA"; // it was here only for first pull, same as request_batch
	}

	p << INT_PARAM("cap", 8) // TODO: what's this item? Sometimes it's 0, sometimes 8
		<< CHAR_PARAM("uid", self_.user_id.c_str())
		<< CHAR_PARAM("viewer_uid", self_.user_id.c_str());

	if (!chat_sticky_num_.empty() && !chat_sticky_pool_.empty()) {
		p << CHAR_PARAM("sticky_token", chat_sticky_num_.c_str());
		p << CHAR_PARAM("sticky_pool", chat_sticky_pool_.c_str());
	}

	if (!isPing && !chat_traceid_.empty())
		p << CHAR_PARAM("traceid", chat_traceid_.c_str());

	if (parent->isInvisible())
		p << CHAR_PARAM("state", "offline");
	else if (isPing || idleSeconds < 60)
		p << CHAR_PARAM("state", "active");
	
	return p;
}
