/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-22 Miranda NG team

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

void OmegleProto::SendMsgWorker(void *p)
{
	if (p == nullptr)
		return;

	std::string data = *(std::string*)p;
	delete (std::string*)p;

	data = utils::text::trim(data);

	if (facy.state_ == STATE_ACTIVE && data.length() && facy.send_message(data)) {
		wchar_t *msg = mir_a2u_cp(data.c_str(), CP_UTF8);
		UpdateChat(facy.nick_, msg);
		mir_free(msg);
	}
}

void OmegleProto::SendTypingWorker(void *p)
{
	if (p == nullptr)
		return;

	// Save typing info
	bool typ = (*static_cast<int*>(p) == PROTOTYPE_SELFTYPING_ON);
	delete (int*)p;

	// Ignore same typing info
	if (facy.typing_ == typ)
		return;

	if (facy.state_ != STATE_ACTIVE)
		return;

	facy.typing_ = typ;

	if (typ)
		facy.typing_start();
	else
		facy.typing_stop();
}

void OmegleProto::NewChatWorker(void*)
{
	NewChat();
}

void OmegleProto::StopChatWorker(void*)
{
	StopChat();
}
