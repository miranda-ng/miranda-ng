/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-15 Robert Pösel

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
	if(p == NULL)
		return;
  
	ScopedLock s( facy.send_message_lock_ );

	std::string data = *(std::string*)p;
	delete (std::string*)p;

	data = utils::text::trim(data);

	if (facy.state_ == STATE_ACTIVE && data.length() && facy.send_message( data ))
	{
		TCHAR *msg = mir_a2t_cp(data.c_str(), CP_UTF8);
		UpdateChat(facy.nick_, msg);
		mir_free(msg);
	}
}

void OmegleProto::SendTypingWorker(void *p)
{
	if (p == NULL)
		return;

	// Save typing info
	bool typ = (*static_cast<int*>(p) == PROTOTYPE_SELFTYPING_ON);
	delete (int*)p;

	// Ignore same typing info
	if (facy.typing_ == typ)
		return;

	facy.typing_ = typ;
	// Wait for eventually changes to typing info by other thread and ignore if changed
	SleepEx(2000, true);
	if (facy.typing_ != typ || facy.old_typing_ == typ || facy.state_ != STATE_ACTIVE)
		return;	

	facy.old_typing_ = typ;
	if (typ)
		facy.typing_start();
	else
		facy.typing_stop();
}

void OmegleProto::NewChatWorker(void*p)
{
	NewChat();
}

void OmegleProto::StopChatWorker(void*p)
{
	StopChat();
}

int OmegleProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	// TODO: msg comes as Unicode (retyped wchar_t*), why should we convert it as ANSI to UTF-8? o_O
/*	if ( flags & PREF_UNICODE )
		msg = mir_utf8encode(msg);
  
	facy.msgid_ = (facy.msgid_ % 1024)+1;
	ForkThread( &OmegleProto::SendMsgWorker, this,new send_direct(hContact,msg,(HANDLE)facy.msgid_));
	return facy.msgid_;*/
	return 0;
}
