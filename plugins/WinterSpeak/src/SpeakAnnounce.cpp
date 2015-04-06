#include "Common.h"
#include "SpeakAnnounce.h"


SpeakAnnounce::SpeakAnnounce(HINSTANCE instance) : m_instance(instance), m_db(), m_dialog(m_db), m_user_info(), m_event_info(), m_protocol_info()
{
}

//------------------------------------------------------------------------------
SpeakAnnounce::~SpeakAnnounce()
{
}

//------------------------------------------------------------------------------
void SpeakAnnounce::statusChange(DBCONTACTWRITESETTING *write_setting, MCONTACT user)
{
	const std::string STATUS = "Status";

	// if the user is myself (NULL) then return
	// if it's not a status change then return
	// check and update the user's status, if status didn't change the return
	if ((NULL == user) || (STATUS != write_setting->szSetting) || (!m_user_info.updateStatus(user, write_setting->value.wVal)))
		return;

	// check if we just connected, and want to suppress status changes
	if (!m_db.getStatusFlag(AnnounceDatabase::StatusFlag_SuppressConnect) && m_protocol_info.isDisabled((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)user, 0)))
		return;

	bool speak = false;

	switch (write_setting->value.wVal)
	{
	  case ID_STATUS_OFFLINE:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Offline);
		break;
	  case ID_STATUS_ONLINE:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Online);
		break;
	  case ID_STATUS_AWAY:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Away);
		break;
	  case ID_STATUS_DND:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Dnd);
		break;
	  case ID_STATUS_NA:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Na);
		break;
	  case ID_STATUS_OCCUPIED:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Occupied);
		break;
	  case ID_STATUS_FREECHAT:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_FreeForChat);
		break;
	  case ID_STATUS_INVISIBLE:
		speak = m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Invisible);
		break;
	}

	if (!speak)
		return;

	// translate, insert name then speak
	std::wstring status_str = TranslateW(m_user_info.statusString(user).c_str());
	m_user_info.insertName(status_str, user);
	status(status_str, user);
}

//------------------------------------------------------------------------------
void SpeakAnnounce::incomingEvent(MCONTACT user, MEVENT event)
{
	if (!m_event_info.isValidEvent(event))
		return;

	bool speak = false;
	switch (m_event_info.getLastEvent()) {
	case EVENTTYPE_MESSAGE:
		speak = m_db.getEventFlag(AnnounceDatabase::EventFlag_Message);
		break;

	case EVENTTYPE_URL:
		speak = m_db.getEventFlag(AnnounceDatabase::EventFlag_Url);
		break;

	case EVENTTYPE_ADDED:
		speak = m_db.getEventFlag(AnnounceDatabase::EventFlag_Added);
		break;

	case EVENTTYPE_AUTHREQUEST:
		speak = m_db.getEventFlag(AnnounceDatabase::EventFlag_AuthRequest);
		break;

	case EVENTTYPE_FILE:
		speak = m_db.getEventFlag(AnnounceDatabase::EventFlag_File);
		break;
	}

	if (!speak)
		return;

	std::wstring event_str = L"";

	if (EVENTTYPE_MESSAGE == m_event_info.getLastEvent()) {
		if (!readMessage(user))
			return; // message dialog is open so just leave without saying anything

		if ((m_db.getEventFlag(AnnounceDatabase::EventFlag_ReadMsgLength)) && (m_event_info.getMessageSize() <= m_db.getMaxMsgSize()))
			event_str = m_event_info.getMessage(); // conditions met to read the message
		else
			event_str = m_event_info.eventString();
	}
	else event_str = m_event_info.eventString();

	m_user_info.insertName(event_str, user); // translate the string, insert the name, then speak it
	message(event_str, user);
}

//------------------------------------------------------------------------------
void SpeakAnnounce::protocolAck(ACKDATA *ack)
{
	if (ACKTYPE_STATUS != ack->type)
	{
		return;
	}

	if (ID_STATUS_CONNECTING != (INT_PTR)ack->hProcess)
	{
		return;
	}

	m_protocol_info.disable((char *)ack->szModule);
}

//------------------------------------------------------------------------------
void SpeakAnnounce::message(const std::wstring &sentence, MCONTACT user)
{
	CallService(MS_SPEAK_MESSAGE, LPARAM(user), reinterpret_cast<WPARAM>(sentence.c_str()));
}

void SpeakAnnounce::status(const std::wstring &sentence, MCONTACT user)
{
	CallService(MS_SPEAK_STATUS, LPARAM(user), reinterpret_cast<WPARAM>(sentence.c_str()));
}

//------------------------------------------------------------------------------
bool SpeakAnnounce::readMessage(MCONTACT contact)
{
	// Check if message window exists
	if (m_db.getEventFlag(AnnounceDatabase::EventFlag_DialogOpen)) {
		// Do not notify if window is already  open
		MessageWindowInputData mwid;
		mwid.cbSize = sizeof(MessageWindowInputData);
		mwid.hContact = contact;
		mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		MessageWindowData mwd;
		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = contact;
		return ((CallService(MS_MSG_GETWINDOWDATA, (WPARAM) &mwid, (LPARAM) &mwd)) || ((mwd.hwndWindow != NULL) && (mwd.uState & MSG_WINDOW_STATE_EXISTS)));
	}
	return true;
}

//==============================================================================
