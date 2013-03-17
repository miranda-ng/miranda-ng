//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "speak_announce.h"

#include "m_speak.h"

#include <general/debug/debug.h>

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
SpeakAnnounce::SpeakAnnounce(HINSTANCE instance)
	:
    m_instance(instance),
	m_db(),
    m_dialog(m_db),
    m_user_info(),
	m_event_info(),
    m_protocol_info()
{
	CLASSCERR("SpeakAnnounce::SpeakAnnounce");
}

//------------------------------------------------------------------------------
SpeakAnnounce::~SpeakAnnounce()
{
	CLASSCERR("SpeakAnnounce::~SpeakAnnounce");
}

//------------------------------------------------------------------------------
void 
SpeakAnnounce::statusChange(DBCONTACTWRITESETTING *write_setting, HANDLE user)
{
	const std::string STATUS = "Status";

	// if the user is myself (NULL) then return
    // if it's not a status change then return
	// check and update the user's status, if status didn't change the return
	if ((NULL == user)
        || (STATUS != write_setting->szSetting)
        || (!m_user_info.updateStatus(user, write_setting->value.wVal)))
	{
		return;
	}
    
    // check if we just connected, and want to suppress status changes
    if (!m_db.getStatusFlag(AnnounceDatabase::StatusFlag_SuppressConnect) 
        && m_protocol_info.isDisabled(
        (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)user, 0)))
    {
        return;
    }

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
    {
        return;
    }

    // translate, insert name then speak
	std::string status_str = Translate(m_user_info.statusString(user).c_str());
	m_user_info.insertName(status_str, user);
	say(status_str, user);
}

//------------------------------------------------------------------------------
void 
SpeakAnnounce::incomingEvent(HANDLE user, HANDLE event)
{
	CLASSCERR("SpeakAnnounce::incomingEvent(" << user << "," << event << ")");

	if (m_event_info.isValidEvent(event))
	{
        bool speak = false;

        switch (m_event_info.getLastEvent())
        {
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
        {
            return;
        }

		std::string event_str = "";

        if (EVENTTYPE_MESSAGE == m_event_info.getLastEvent())
        {
            if (!readMessage(user))
            {
                // message dialog is open so just leave without saying anything
                return;
            }

            if ((m_db.getEventFlag(AnnounceDatabase::EventFlag_ReadMsgLength))
                && (m_event_info.getMessageSize() <= m_db.getMaxMsgSize()))
            {
                // conditions met to read the message
                event_str = m_event_info.getMessage();
            }
            else
            {
                event_str = m_event_info.eventString();
            }
        }
		else
		{
			event_str = m_event_info.eventString();
		}
	
        // translate the string, insert the name, then speak it
	    m_user_info.insertName(event_str, user);
		say(event_str, user);
	}
}

//------------------------------------------------------------------------------
void 
SpeakAnnounce::protocolAck(ACKDATA *ack)
{
    if (ACKTYPE_STATUS != ack->type)
    {
        return;
    }

    if (ID_STATUS_CONNECTING != (int)ack->hProcess)
    {
        return;
    }

    m_protocol_info.disable((char *)ack->szModule);
}

//------------------------------------------------------------------------------
void
SpeakAnnounce::say(const std::string &sentence, HANDLE user)
{
	CLASSCERR("SpeakAnnounce::say(" << sentence << ", " << (int)user << ")");

    CallService(ME_SPEAK_SAY, reinterpret_cast<LPARAM>(user),
        reinterpret_cast<WPARAM>(sentence.c_str()));
}

//------------------------------------------------------------------------------
bool 
SpeakAnnounce::readMessage(HANDLE contact)
{
    std::string title = m_user_info.nameString(contact) + " ("
        + m_user_info.statusModeString(contact) + "): ";

    HWND window = NULL;
        
    window = FindWindow("#32770", (title + Translate("Message Session")).c_str());
    if (window)
    {
        // check if we dont want to read message if dialog is open
        if (m_db.getEventFlag(AnnounceDatabase::EventFlag_DialogOpen))
        {
            return false;
        }

        // check if we dont want to read message if dialog if focused
        if ((window == GetForegroundWindow())
            && m_db.getEventFlag(AnnounceDatabase::EventFlag_DialogFocused))
        {
            return false;
        }
    }

    window = FindWindow("#32770", (title + Translate("Message Received")).c_str());
    if (window)
    {
        // check if we dont want to read message if dialog is open
        if (m_db.getEventFlag(AnnounceDatabase::EventFlag_DialogOpen))
        {
            return false;
        }

        // check if we dont want to read message if dialog if focused
        if ((window == GetForegroundWindow())
            && m_db.getEventFlag(AnnounceDatabase::EventFlag_DialogFocused))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
