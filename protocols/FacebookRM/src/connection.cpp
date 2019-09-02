/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

static void CALLBACK timerApcFunc(HWND, UINT, UINT_PTR param, DWORD)
{
	FacebookProto *ppro = (FacebookProto *)param;
	if (!ppro->isOffline())
		ppro->ForkThread(&FacebookProto::ProcessNotifications);
}

void FacebookProto::ChangeStatus(void*)
{
	mir_cslock s(signon_lock_);

	int new_status = m_iDesiredStatus;
	int old_status = m_iStatus;

	if (new_status == ID_STATUS_OFFLINE) {
		// Logout	
		debugLogA("### Beginning SignOff process");
		m_signingOut = true;

		::KillTimer(g_hwndHeartbeat, LPARAM(this));
		::SetEvent(update_loop_event);

		// Shutdown and close channel handle
		Netlib_Shutdown(facy.hChannelCon);
		if (facy.hChannelCon) {
			Netlib_CloseHandle(facy.hChannelCon);
			facy.hChannelCon = nullptr;
		}

		// Shutdown and close messages handle
		Netlib_Shutdown(facy.hMessagesCon);
		if (facy.hMessagesCon) {
			Netlib_CloseHandle(facy.hMessagesCon);
			facy.hMessagesCon = nullptr;
		}

		// Turn off chat on Facebook								  
		if (getByte(FACEBOOK_KEY_DISCONNECT_CHAT, DEFAULT_DISCONNECT_CHAT))
			facy.chat_state(false);

		facy.logout();

		OnLeaveChat(0, 0);
		setAllContactStatuses(ID_STATUS_OFFLINE);
		ToggleStatusMenuItems(false);
		delSetting(FACEBOOK_KEY_LOGON_TS);

		facy.chat_traceid_.clear();
		facy.chat_sticky_num_.clear();
		facy.chat_sticky_pool_.clear();
		facy.chat_msgs_recv_ = 0;
		facy.chat_req_ = 0;

		facy.clear_cookies();
		facy.clear_notifications();
		facy.clear_chatrooms();
		facy.clear_readers();
		facy.messages_ignore.clear();
		facy.messages_timestamp.clear();
		facy.pages.clear();
		facy.typers.clear();

		// Close connection handle
		if (facy.hFcbCon)
			Netlib_CloseHandle(facy.hFcbCon);
		facy.hFcbCon = nullptr;

		m_iStatus = facy.self_.status_id = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		m_signingOut = false;
		debugLogA("### SignOff complete");
		return;
	}
	else if (old_status == ID_STATUS_OFFLINE) {
		// Login
		SYSTEMTIME t;
		GetLocalTime(&t);
		debugLogA("[%d.%d.%d] Using Facebook Protocol RM %s", t.wDay, t.wMonth, t.wYear, __VERSION_STRING_DOTS);

		debugLogA("*** Beginning SignOn process");

		m_enableChat = getBool(FACEBOOK_KEY_ENABLE_CHATS, DEFAULT_ENABLE_CHATS);

		m_iStatus = facy.self_.status_id = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ResetEvent(update_loop_event);

		// Workaround for not working "mbasic." for some users - reset this flag at every login
		facy.mbasicWorks = true;

		if (NegotiateConnection() && facy.home() && facy.reconnect()) {
			// Load all friends
			ProcessFriendList(nullptr);

			// Process friendship requests
			ForkThread(&FacebookProto::ProcessFriendRequests, nullptr);

			// Get unread messages
			ForkThread(&FacebookProto::ProcessUnreadMessages, nullptr);

			// Get notifications
			if (getByte(FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
				ForkThread(&FacebookProto::ProcessNotifications, nullptr);

			// Load pages for post status dialog
			ForkThread(&FacebookProto::ProcessPages, nullptr);

			// Load on this day posts
			if (getByte(FACEBOOK_KEY_EVENT_ON_THIS_DAY_ENABLE, DEFAULT_EVENT_ON_THIS_DAY_ENABLE))
				ForkThread(&FacebookProto::ProcessMemories, nullptr);

			setDword(FACEBOOK_KEY_LOGON_TS, (DWORD)time(0));
			ForkThread(&FacebookProto::UpdateLoop, nullptr);
			ForkThread(&FacebookProto::MessageLoop, nullptr);

			if (getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS))
				ForkThread(&FacebookProto::SetAwayMsgWorker, nullptr);

			::SetTimer(g_hwndHeartbeat, LPARAM(this), 60000, timerApcFunc);
		}
		else {
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)old_status, m_iStatus);

			if (facy.hFcbCon)
				Netlib_CloseHandle(facy.hFcbCon);
			facy.hFcbCon = nullptr;

			facy.clear_cookies();

			// Set to offline
			m_iStatus = m_iDesiredStatus = facy.self_.status_id = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

			debugLogA("*** SignOn failed");

			return;
		}

		// Join all locally present chatrooms (if enabled)
		if (getBool(FACEBOOK_KEY_JOIN_EXISTING_CHATS, DEFAULT_JOIN_EXISTING_CHATS))
			JoinChatrooms();

		ToggleStatusMenuItems(true);
		debugLogA("*** SignOn complete");
	}
	else { // Change between online/away/invisible statuses
		if (new_status == ID_STATUS_INVISIBLE) 
			// When switching to invisible (from online/away), we need to set all contacts offline as we won't receive no status updates from Facebook
			setAllContactStatuses(ID_STATUS_OFFLINE);
	}

	bool wasAwayOrInvisible = (old_status == ID_STATUS_AWAY || old_status == ID_STATUS_INVISIBLE);
	bool isAwayOrInvisible = (new_status == ID_STATUS_AWAY || new_status == ID_STATUS_INVISIBLE);
	if (!wasAwayOrInvisible && isAwayOrInvisible) {
		// Switching from "not-away" to "away" state, remember timestamp of this change (and if we are idle already, use the idle time)
		m_awayTS = (m_idleTS > 0 ? m_idleTS : ::time(0));
	}
	else if (wasAwayOrInvisible && !isAwayOrInvisible) {
		// Switching from "away" to "not-away" state, reset the timestamp
		m_awayTS = 0;
	}

	m_invisible = (new_status == ID_STATUS_INVISIBLE);
	facy.chat_state(!m_invisible);

	m_iStatus = facy.self_.status_id = new_status;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

	debugLogA("*** ChangeStatus complete");
}

/** Return true on success, false on error. */
bool FacebookProto::NegotiateConnection()
{
	debugLogA("*** Negotiating connection with Facebook");

	ptrA username(getStringA(FACEBOOK_KEY_LOGIN));
	if (!username || !mir_strlen(username)) {
		facy.client_notify(TranslateT("Please enter a username."));
		return false;
	}

	ptrA password(getStringA(FACEBOOK_KEY_PASS));
	if (!password || !*password) {
		facy.client_notify(TranslateT("Please enter a password."));
		return false;
	}

	password = mir_utf8encode(password);

	// Refresh last time of feeds update
	facy.last_feeds_update_ = ::time(0);

	// Generate random clientid for this connection
	facy.chat_clientid_ = utils::text::rand_string(8, "0123456789abcdef", &facy.random_);

	// Create default group for new contacts
	if (m_tszDefaultGroup)
		Clist_GroupCreate(0, m_tszDefaultGroup);

	return facy.login(username, password);
}

void FacebookProto::UpdateLoop(void *)
{
	time_t tim = ::time(0);
	debugLogA(">>> Entering Facebook::UpdateLoop[%d]", tim);

	for (int i = -1; !isOffline(); i = (i + 1) % 50) {
		if (i != -1) {
			if (getByte(FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE))
				ProcessFeeds(nullptr);
		}

		ProcessBuddylistUpdate(nullptr);

		debugLogA("*** FacebookProto::UpdateLoop[%d] going to sleep...", tim);
		if (WaitForSingleObjectEx(update_loop_event, GetPollRate() * 1000, true) != WAIT_TIMEOUT)
			break;
		debugLogA("*** FacebookProto::UpdateLoop[%d] waking up...", tim);
	}

	ResetEvent(update_loop_event);
	debugLogA("<<< Exiting FacebookProto::UpdateLoop[%d]", tim);
}

void FacebookProto::MessageLoop(void *)
{
	time_t tim = ::time(0);
	debugLogA(">>> Entering Facebook::MessageLoop[%d]", tim);

	while (facy.channel()) {
		if (isOffline() || m_signingOut)
			break;

		// If we're not idle, send activity_ping every few minutes...
		if (!m_idleTS && (::time(0) - m_pingTS) > FACEBOOK_PING_TIME) {
			debugLogA("*** FacebookProto::MessageLoop[%d] pinging...", tim);
			facy.activity_ping();
		}

		debugLogA("*** FacebookProto::MessageLoop[%d] refreshing...", tim);
	}

	debugLogA("<<< Exiting FacebookProto::MessageLoop[%d]", tim);
}

BYTE FacebookProto::GetPollRate()
{
	BYTE poll_rate = getByte(FACEBOOK_KEY_POLL_RATE, FACEBOOK_DEFAULT_POLL_RATE);
	return ((poll_rate >= FACEBOOK_MINIMAL_POLL_RATE && poll_rate <= FACEBOOK_MAXIMAL_POLL_RATE) ? poll_rate : FACEBOOK_DEFAULT_POLL_RATE);
}
