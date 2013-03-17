//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "event_information.h"
#include "speak.h"

#include <general/debug/debug.h>

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
EventInformation::EventInformation()
	:
	m_event_strings(),
	m_event_info()
{
	CLASSCERR("EventInformation::EventInformation");

	// insert the event strings into a map for easy access
	m_event_strings[EVENTTYPE_MESSAGE]     
        = Translate("incoming message from %u");
	m_event_strings[EVENTTYPE_URL]         
        = Translate("incoming U R L from %u");
	m_event_strings[EVENTTYPE_ADDED]       
        = Translate("you have been added to %u's contact list");
	m_event_strings[EVENTTYPE_AUTHREQUEST]
        = Translate("%u requests your authorization");
	m_event_strings[EVENTTYPE_FILE]        
        = Translate("there is an incoming file from %u");

	ZeroMemory(&m_event_info, sizeof(m_event_info));
}

//------------------------------------------------------------------------------
EventInformation::~EventInformation()
{
	CLASSCERR("EventInformation::~EventInformation");
}

//------------------------------------------------------------------------------
bool
EventInformation::isValidEvent(HANDLE event)
{
	CLASSCERR("EventInformation::isValidEvent()");

	// clean up the old event
	if (m_event_info.pBlob)
	{
		delete m_event_info.pBlob;
	}
	ZeroMemory(&m_event_info, sizeof(m_event_info));

	// find out and assign the space we need for the new event
	m_event_info.cbSize = sizeof(m_event_info);
	m_event_info.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, 
		reinterpret_cast<LPARAM>(event), 0);

    if (-1 == m_event_info.cbBlob)
    {
        return false;
    }

	m_event_info.pBlob = new unsigned char[m_event_info.cbBlob];

	// get the event info
	CallService(MS_DB_EVENT_GET, reinterpret_cast<LPARAM>(event), 
		reinterpret_cast<LPARAM>(&m_event_info));

	// if the event has already been read or was sent by me then exit
	if (m_event_info.flags & (DBEF_SENT | DBEF_READ))
	{
		return false;
	}

	// if the event string doesn't exist in our list then exit
	if (m_event_strings.find(m_event_info.eventType) == m_event_strings.end())
	{
		return false;
	}

	// event was good
	return true;
}

//------------------------------------------------------------------------------
std::string 
EventInformation::getMessage()
{
	CLASSCERR("EventInformation::getMessage");

    const std::string intro = Translate("%u says");

	return intro + " " + (char *)m_event_info.pBlob;
}

//------------------------------------------------------------------------------
unsigned int 
EventInformation::getMessageSize()
{
	CLASSCERR("EventInformation::getMessageSize");

	return std::string((char *)m_event_info.pBlob).size();
}

//------------------------------------------------------------------------------
std::string 
EventInformation::eventString()
{
	CLASSCERR("EventInformation::eventString");

	return m_event_strings[m_event_info.eventType];
}

//==============================================================================
