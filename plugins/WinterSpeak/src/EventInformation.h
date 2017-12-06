#pragma once

#include <map>
#include <string>

class EventInformation
{
  public:
	EventInformation();
	~EventInformation();

	//--------------------------------------------------------------------------
	// Description : is the event valid?
	// Return      : true = the event is valid
	//--------------------------------------------------------------------------
	bool isValidEvent(MEVENT event);

	//--------------------------------------------------------------------------
	// Description : get the last event received
	// Return      : the last event or 0 if none yet received
	//--------------------------------------------------------------------------
	unsigned short getLastEvent() { return m_event_info.eventType; }

	//--------------------------------------------------------------------------
	// Description : was the last event a messsage event?
	// Return      : true - yes it was
	//               false - no it wasn't
	//--------------------------------------------------------------------------
//    bool isMessageEvent() 
//        { return (m_event_info.eventType == EVENTTYPE_MESSAGE); }

	//--------------------------------------------------------------------------
	// Description : get the message from the last event
	// Return      : the last message
	//--------------------------------------------------------------------------
	std::wstring getMessage();

	//--------------------------------------------------------------------------
	// Description : get the size of the message from the last event
	// Return      : the size of the message
	//--------------------------------------------------------------------------
	size_t getMessageSize();

	//--------------------------------------------------------------------------
	// Description : get the event string
	//--------------------------------------------------------------------------
	std::wstring eventString();

  private:
	std::map<unsigned short, std::wstring> m_event_strings;

	DBEVENTINFO m_event_info;
};
