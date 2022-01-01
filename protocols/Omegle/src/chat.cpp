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

const wchar_t msgChatModes[] = 
LPGENW("There are three different modes of chatting:\
\n1) Standard mode\t - You chat with random stranger privately\
\n2) Question mode\t - You ask two strangers a question and see how they discuss it (you can't join their conversation, only watch)\
\n3) Spy mode\t - You and stranger got a question to discuss from third stranger (he can't join your conversation, only watch)\
\n\nSend '/commands' for available commands.");

const wchar_t msgChatCommands[] =
LPGENW("You can use different commands:\
\n/help\t - show info about chat modes\
\n/new\t - start standard mode\
\n/ask <question> - start question mode with your question\
\n/ask\t - start question mode with your last asked question\
\n/spy\t - start spy mode\
\n/quit\t - disconnect from stranger or stop connecting\
\n/asl\t - send your predefined ASL message\
\n\nNote: You can reconnect to different stranger without disconnecting from current one.");

void OmegleProto::UpdateChat(const wchar_t *name, const wchar_t *message, bool addtolog)
{
	if (message == nullptr)
		return;

	// replace % to %% to not interfere with chat color codes
	CMStringW smessage(message);
	smessage.Replace(L"%", L"%%");

	GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
	gce.pszID.w = m_tszUserName;
	gce.time = ::time(0);
	gce.pszText.w = smessage.c_str();

	if (name == nullptr) {
		gce.iType = GC_EVENT_INFORMATION;
		name = TranslateT("Server");
		gce.bIsMe = false;
	}
	else gce.bIsMe = !mir_wstrcmp(name, this->facy.nick_);

	if (addtolog)
		gce.dwFlags |= GCEF_ADDTOLOG;

	gce.pszNick.w = name;
	gce.pszUID.w = gce.pszNick.w;
	Chat_Event(&gce);
}

int OmegleProto::OnChatEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	if (mir_strcmp(hook->si->pszModule, m_szModuleName))
		return 0;

	switch (hook->iType) {
	case GC_USER_MESSAGE:
		{
			std::string text = mir_u2a_cp(hook->ptszText, CP_UTF8);

			// replace %% back to %, because chat automatically does this to sent messages
			utils::text::replace_all(&text, "%%", "%");

			if (text.empty())
				break;

			if (text.substr(0, 1) == "/") { // Process commands

				std::string command = "";
				std::string params = "";

				std::string::size_type pos = 0;
				if ((pos = text.find(" ")) != std::string::npos) {
					command = text.substr(1, pos - 1);
					params = text.substr(pos + 1);
				}
				else command = text.substr(1);

				if (!mir_strcmpi(command.c_str(), "new")) {
					facy.spy_mode_ = false;
					facy.question_.clear();

					ForkThread(&OmegleProto::NewChatWorker, nullptr);
					break;
				}
				else if (!mir_strcmpi(command.c_str(), "quit")) {
					ForkThread(&OmegleProto::StopChatWorker, nullptr);
					break;
				}
				else if (!mir_strcmpi(command.c_str(), "spy")) {
					facy.spy_mode_ = true;
					facy.question_.clear();

					ForkThread(&OmegleProto::NewChatWorker, nullptr);
					break;
				}
				else if (!mir_strcmpi(command.c_str(), "ask")) {
					if (params.empty()) {
						// Load last question
						DBVARIANT dbv;
						if (!getU8String(OMEGLE_KEY_LAST_QUESTION, &dbv)) {
							params = dbv.pszVal;
							db_free(&dbv);
						}

						if (params.empty()) {
							UpdateChat(nullptr, TranslateT("Last question is empty."), false);
							break;
						}
					}
					else {
						// Save actual question as last question
						if (params.length() >= OMEGLE_QUESTION_MIN_LENGTH)
							setU8String(OMEGLE_KEY_LAST_QUESTION, params.c_str());
					}

					if (params.length() < OMEGLE_QUESTION_MIN_LENGTH) {
						UpdateChat(nullptr, TranslateT("Your question is too short."), false);
						break;
					}

					facy.spy_mode_ = true;
					facy.question_ = params;
					ForkThread(&OmegleProto::NewChatWorker, nullptr);
					break;
				}
				else if (!mir_strcmpi(command.c_str(), "asl")) {
					DBVARIANT dbv;
					if (!getU8String(OMEGLE_KEY_ASL, &dbv)) {
						text = dbv.pszVal;
						db_free(&dbv);

						SendChatMessage(text);
					}
					else {
						UpdateChat(nullptr, TranslateT("Your '/asl' setting is empty."), false);
						break;
					}
				}
				else if (!mir_strcmpi(command.c_str(), "help")) {
					UpdateChat(nullptr, TranslateW(msgChatModes), false);
				}
				else if (!mir_strcmpi(command.c_str(), "commands")) {
					UpdateChat(nullptr, TranslateW(msgChatCommands), false);
					break;
				}
				else {
					UpdateChat(nullptr, TranslateT("Unknown command. Send '/commands' for list."), false);
					break;
				}
			}
			else // Outgoing message
				SendChatMessage(text);
		}
		break;

	case GC_SESSION_TERMINATE:
		facy.nick_ = nullptr;
		ForkThread(&OmegleProto::StopChatWorker, nullptr);
		break;
	}

	return 1;
}

void OmegleProto::SendChatMessage(std::string text)
{
	switch (facy.state_) {
	case STATE_ACTIVE:
		debugLogA("**Chat - Outgoing message: %s", text.c_str());
		ForkThread(&OmegleProto::SendMsgWorker, new std::string(text));
		break;

	case STATE_INACTIVE:
		UpdateChat(nullptr, TranslateT("You aren't connected to any stranger. Send '/help' or '/commands' for help."), false);
		break;

	case STATE_SPY:
		UpdateChat(nullptr, TranslateT("You can't send messages in question mode."), false);
		break;
	}
}

void OmegleProto::AddChatContact(const wchar_t *name)
{
	GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
	gce.pszID.w = m_tszUserName;
	gce.time = uint32_t(time(0));
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pszNick.w = name;
	gce.pszUID.w = gce.pszNick.w;

	if (name == nullptr)
		gce.bIsMe = false;
	else
		gce.bIsMe = mir_wstrcmp(name, this->facy.nick_);

	if (gce.bIsMe)
		gce.pszStatus.w = L"Admin";
	else
		gce.pszStatus.w = L"Normal";

	Chat_Event(&gce);
}

void OmegleProto::DeleteChatContact(const wchar_t *name)
{
	GCEVENT gce = { m_szModuleName, 0, GC_EVENT_PART };
	gce.pszID.w = m_tszUserName;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pszNick.w = name;
	gce.pszUID.w = gce.pszNick.w;
	gce.time = uint32_t(time(0));
	if (name == nullptr)
		gce.bIsMe = false;
	else
		gce.bIsMe = mir_wstrcmp(name, this->facy.nick_);

	Chat_Event(&gce);
}

INT_PTR OmegleProto::OnJoinChat(WPARAM, LPARAM suppress)
{
	// Create the group chat session
	SESSION_INFO *si = Chat_NewSession(GCW_PRIVMESS, m_szModuleName, m_tszUserName, m_tszUserName);
	if (!si || m_iStatus == ID_STATUS_OFFLINE)
		return 0;

	// Create a group
	Chat_AddGroup(si, TranslateT("Admin"));
	Chat_AddGroup(si, TranslateT("Normal"));

	SetTopic();

	// Note: Initialization will finish up in SetChatStatus, called separately
	if (!suppress)
		SetChatStatus(m_iStatus);

	return 0;
}

void OmegleProto::SetTopic(const wchar_t *topic)
{
	GCEVENT gce = { m_szModuleName, 0, GC_EVENT_TOPIC };
	gce.pszID.w = m_tszUserName;
	gce.time = ::time(0);

	if (topic == nullptr)
		gce.pszText.w = TranslateT("Omegle is a great way of meeting new friends!");
	else
		gce.pszText.w = topic;

	Chat_Event(&gce);
}

INT_PTR OmegleProto::OnLeaveChat(WPARAM, LPARAM)
{
	Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
	Chat_Terminate(m_szModuleName, m_tszUserName);
	return 0;
}

void OmegleProto::SetChatStatus(int status)
{
	if (status == ID_STATUS_ONLINE) {
		// Load actual name from database
		facy.nick_ = db_get_wsa(0, m_szModuleName, OMEGLE_KEY_NAME);
		if (facy.nick_ == NULL) {
			facy.nick_ = mir_wstrdup(TranslateT("You"));
			db_set_ws(0, m_szModuleName, OMEGLE_KEY_NAME, facy.nick_);
		}

		// Add self contact
		AddChatContact(facy.nick_);

		Chat_Control(m_szModuleName, m_tszUserName, SESSION_INITDONE);
		Chat_Control(m_szModuleName, m_tszUserName, SESSION_ONLINE);
	}
	else Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
}

void OmegleProto::ClearChat()
{
	if (!getByte(OMEGLE_KEY_NO_CLEAR, 0))
		Chat_Control(m_szModuleName, m_tszUserName, WINDOW_CLEARLOG);
}

// TODO: Could this be done better?
MCONTACT OmegleProto::GetChatHandle()
{
	GC_INFO gci = {};
	gci.Flags = GCF_HCONTACT;
	gci.pszModule = m_szModuleName;
	gci.pszID = m_tszUserName;
	Chat_GetInfo(&gci);

	return gci.hContact;
}
