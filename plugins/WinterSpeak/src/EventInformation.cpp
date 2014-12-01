#include "Common.h"
#include "EventInformation.h"


//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
EventInformation::EventInformation() : m_event_strings(), m_event_info()
{
	// insert the event strings into a map for easy access
	m_event_strings[EVENTTYPE_MESSAGE]	    = TranslateW(L"incoming message from %u");
	m_event_strings[EVENTTYPE_URL]          = TranslateW(L"incoming URL from %u");
	m_event_strings[EVENTTYPE_ADDED]        = TranslateW(L"you have been added to %u's contact list");
	m_event_strings[EVENTTYPE_AUTHREQUEST]  = TranslateW(L"%u requests your authorization");
	m_event_strings[EVENTTYPE_FILE]         = TranslateW(L"there is an incoming file from %u");

	memset(&m_event_info, 0, sizeof(m_event_info));
}

//------------------------------------------------------------------------------
EventInformation::~EventInformation()
{
}

//------------------------------------------------------------------------------
bool EventInformation::isValidEvent(HANDLE event)
{
	// clean up the old event
	if (m_event_info.pBlob)
	{
		delete m_event_info.pBlob;
	}
	memset(&m_event_info, 0, sizeof(m_event_info));

	// find out and assign the space we need for the new event
	m_event_info.cbSize = sizeof(m_event_info);
	m_event_info.cbBlob = db_event_getBlobSize(event);// CallService(MS_DB_EVENT_GETBLOBSIZE, reinterpret_cast<LPARAM>(event), 0);

	if (-1 == m_event_info.cbBlob)
	{
		return false;
	}
	m_event_info.pBlob = new unsigned char[m_event_info.cbBlob];

	// get the event info
	db_event_get(event, &m_event_info);
	//CallService(MS_DB_EVENT_GET, reinterpret_cast<LPARAM>(event), reinterpret_cast<LPARAM>(&m_event_info));

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
std::wstring EventInformation::getMessage()
{
	const std::wstring intro = TranslateW(L"%u says");

	return intro + L" " + mir_a2t_cp((char*)m_event_info.pBlob, CP_UTF8);
}

//------------------------------------------------------------------------------
size_t EventInformation::getMessageSize()
{
	return std::wstring((WCHAR *)m_event_info.pBlob).size();
}

//------------------------------------------------------------------------------
std::wstring EventInformation::eventString()
{
	return m_event_strings[m_event_info.eventType];
}

//==============================================================================