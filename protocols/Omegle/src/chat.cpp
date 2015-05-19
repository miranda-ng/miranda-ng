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

void OmegleProto::UpdateChat(const TCHAR *name, const TCHAR *message, bool addtolog)
{
	// replace % to %% to not interfere with chat color codes
	std::tstring smessage = message;
	utils::text::treplace_all(&smessage, _T("%"), _T("%%"));

	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = ::time(NULL);
	gce.ptszText = smessage.c_str();

	if (name == NULL) {
		gcd.iType = GC_EVENT_INFORMATION;
		name = TranslateT("Server");
		gce.bIsMe = false;
	}
	else gce.bIsMe = !mir_tstrcmp(name, this->facy.nick_);

	if (addtolog)
		gce.dwFlags  |= GCEF_ADDTOLOG;

	gce.ptszNick = name;
	gce.ptszUID  = gce.ptszNick;
	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));
}

int OmegleProto::OnChatEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);

	if(strcmp(hook->pDest->pszModule,m_szModuleName))
		return 0;

	switch(hook->pDest->iType)
	{
	case GC_USER_MESSAGE:
	{		
		std::string text = mir_t2a_cp(hook->ptszText,CP_UTF8);

		// replace %% back to %, because chat automatically does this to sent messages
		utils::text::replace_all(&text, "%%", "%");

		if (text.empty())
			break;

		if (text.substr(0,1) == "/")
		{ // Process commands
			
			std::string command = "";
			std::string params = "";						

			std::string::size_type pos = 0;
			if ((pos = text.find(" ")) != std::string::npos) {
				command = text.substr(1, pos-1);
				params = text.substr(pos+1);
			} else {
				command = text.substr(1);
			}

			if (!stricmp(command.c_str(), "new"))
			{
				facy.spy_mode_ = false;
				facy.question_.clear();

				ForkThread(&OmegleProto::NewChatWorker, NULL);
				break;
			}
			else if (!stricmp(command.c_str(), "quit"))
			{
				ForkThread(&OmegleProto::StopChatWorker, NULL);
				break;
			}
			else if (!stricmp(command.c_str(), "spy"))
			{
				facy.spy_mode_ = true;
				facy.question_.clear();
				
				ForkThread(&OmegleProto::NewChatWorker, NULL);
				break;
			}
			else if (!stricmp(command.c_str(), "ask"))
			{						
				if (params.empty()) {
					// Load last question
					DBVARIANT dbv;
					if ( !getU8String( OMEGLE_KEY_LAST_QUESTION,&dbv )) {
						params = dbv.pszVal;
						db_free(&dbv);
					}
					
					if (params.empty()) {
						UpdateChat(NULL, TranslateT("Last question is empty."), false);
						break;
					}
				} else {
					// Save actual question as last question
					if (params.length() >= OMEGLE_QUESTION_MIN_LENGTH)
					{
						setU8String( OMEGLE_KEY_LAST_QUESTION, params.c_str());
					}
				}

				if (params.length() < OMEGLE_QUESTION_MIN_LENGTH)
				{
					UpdateChat(NULL, TranslateT("Your question is too short."), false);
					break;
				}

				facy.spy_mode_ = true;
				facy.question_ = params;
				ForkThread(&OmegleProto::NewChatWorker, NULL);
				break;
			}
			else if (!stricmp(command.c_str(), "asl"))
			{
				DBVARIANT dbv;
				if ( !getU8String( OMEGLE_KEY_ASL,&dbv )) {
					text = dbv.pszVal;
					db_free(&dbv);

					SendChatMessage(text);
				} else {
					UpdateChat(NULL, TranslateT("Your '/asl' setting is empty."), false);
					break;
				}
			}
			else if (!stricmp(command.c_str(), "help"))
			{
				UpdateChat(NULL, TranslateT("There are three different modes of chatting:\
\n1) Standard mode\t - You chat with random stranger privately\
\n2) Question mode\t - You ask two strangers a question and see how they discuss it (you can't join their conversation, only watch)\
\n3) Spy mode\t - You and stranger got a question to discuss from third stranger (he can't join your conversation, only watch)\
\n\nSend '/commands' for available commands."), false);
			}
			else if (!stricmp(command.c_str(), "commands"))
			{
				UpdateChat(NULL, TranslateT("You can use different commands:\
\n/help\t - show info about chat modes\
\n/new\t - start standard mode\
\n/ask <question> - start question mode with your question\
\n/ask\t - start question mode with your last asked question\
\n/spy\t - start spy mode\
\n/quit\t - disconnect from stranger or stop connecting\
\n/asl\t - send your predefined ASL message\
\n\nNote: You can reconnect to different stranger without disconnecting from current one."), false);
				break;
			}
			else
			{
				UpdateChat(NULL, TranslateT("Unknown command. Send '/commands' for list."), false);
				break;
			}

		} else {
			// Outgoing message
			SendChatMessage(text);
		}
	
		break;
	}

	case GC_USER_LEAVE:
	case GC_SESSION_TERMINATE:
		facy.nick_ = NULL;
		ForkThread(&OmegleProto::StopChatWorker, NULL);
		break;
	}

	return 0;
}

void OmegleProto::SendChatMessage(std::string text)
{
	switch (facy.state_) {
	case STATE_ACTIVE:
		debugLogA("**Chat - Outgoing message: %s", text.c_str());
		ForkThread(&OmegleProto::SendMsgWorker, new std::string(text));
		break;

	case STATE_INACTIVE:
		UpdateChat(NULL, TranslateT("You aren't connected to any stranger. Send '/help' or '/commands' for help."), false);
		break;

	case STATE_SPY:
		UpdateChat(NULL, TranslateT("You can't send messages in question mode."), false);
		break;

	//case STATE_WAITING:
	//case STATE_DISCONNECTING:
	default:
		break;
	}
}

/*void OmegleProto::SendChatEvent(int type)
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT,WINDOW_CLEARLOG,reinterpret_cast<LPARAM>(&gce));
}*/

void OmegleProto::AddChatContact(const TCHAR *name)
{	
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_JOIN };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = DWORD(time(0));
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = name;
	gce.ptszUID = gce.ptszNick;

	if (name == NULL)
		gce.bIsMe = false;
	else 
		gce.bIsMe = mir_tstrcmp(name, this->facy.nick_);

	if (gce.bIsMe)
		gce.ptszStatus = _T("Admin");
	else
		gce.ptszStatus = _T("Normal");

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));
}

void OmegleProto::DeleteChatContact(const TCHAR *name)
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_PART };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = name;
	gce.ptszUID = gce.ptszNick;
	gce.time = DWORD(time(0));
	if (name == NULL)
		gce.bIsMe = false;
	else 
		gce.bIsMe = mir_tstrcmp(name, this->facy.nick_);

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));
}

INT_PTR OmegleProto::OnJoinChat(WPARAM,LPARAM suppress)
{	
	// Create the group chat session
	GCSESSION gcw = {sizeof(gcw)};
	gcw.iType = GCW_PRIVMESS;
	gcw.ptszID = m_tszUserName;
	gcw.ptszName = m_tszUserName;
	gcw.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	if(m_iStatus == ID_STATUS_OFFLINE)
		return 0;

	// Create a group
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	
	gce.ptszStatus = _T("Admin");
	CallServiceSync( MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce));
	
	gce.ptszStatus = _T("Normal");
	CallServiceSync( MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce));

	SetTopic();
		
	// Note: Initialization will finish up in SetChatStatus, called separately
	if (!suppress)
		SetChatStatus(m_iStatus);

	return 0;
}

void OmegleProto::SetTopic(const TCHAR *topic)
{		
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_TOPIC };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = ::time(NULL);

	if (topic == NULL)
		gce.ptszText = TranslateT("Omegle is a great way of meeting new friends!");
	else
		gce.ptszText = topic;

	CallServiceSync(MS_GC_EVENT,0,  reinterpret_cast<LPARAM>(&gce));
}

INT_PTR OmegleProto::OnLeaveChat(WPARAM,LPARAM)
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = ::time(NULL);

	CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,  reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT,SESSION_TERMINATE,reinterpret_cast<LPARAM>(&gce));

	return 0;
}

void OmegleProto::SetChatStatus(int status)
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = ::time(NULL);
	
	if(status == ID_STATUS_ONLINE)
	{		
		// Load actual name from database
		facy.nick_ = db_get_tsa(NULL, m_szModuleName, OMEGLE_KEY_NAME);
		if (facy.nick_ == NULL) {
			facy.nick_ = mir_tstrdup(TranslateT("You"));
			db_set_ts(NULL, m_szModuleName, OMEGLE_KEY_NAME, facy.nick_);
		}

		// Add self contact
		AddChatContact(facy.nick_);

		CallServiceSync(MS_GC_EVENT,SESSION_INITDONE,reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT,SESSION_ONLINE,  reinterpret_cast<LPARAM>(&gce));
	}
	else
	{
		CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,reinterpret_cast<LPARAM>(&gce));
	}
}

void OmegleProto::ClearChat()
{
	if (getByte(OMEGLE_KEY_NO_CLEAR, 0))
		return;

	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, WINDOW_CLEARLOG, reinterpret_cast<LPARAM>(&gce));
}

// TODO: Could this be done better?
MCONTACT OmegleProto::GetChatHandle()
{
	/*if (facy.chatHandle_ != NULL)
		return facy.chatHandle_;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (db_get_b(hContact, m_szModuleName, "ChatRoom", 0) > 0) {
			ptrA id = db_get_sa(hContact, m_szModuleName, "ChatRoomId");
			if (id != NULL && !strcmp(id, m_szModuleName))
				return hContact;
		}
	}

	return NULL;*/

	GC_INFO gci = {0};
	gci.Flags = GCF_HCONTACT;
	gci.pszModule = m_szModuleName;
	gci.pszID = m_tszUserName;
	CallService(MS_GC_GETINFO, 0, (LPARAM)&gci);

	return gci.hContact;
}