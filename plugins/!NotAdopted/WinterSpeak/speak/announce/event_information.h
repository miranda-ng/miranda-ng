#ifndef guard_speak_announce_event_information_h
#define guard_speak_announce_event_information_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include <speak.h>

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
	bool isValidEvent(HANDLE event);

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
	std::string getMessage();

	//--------------------------------------------------------------------------
	// Description : get the size of the message from the last event
	// Return      : the size of the message
	//--------------------------------------------------------------------------
	unsigned int getMessageSize();

	//--------------------------------------------------------------------------
	// Description : get the event string
	//--------------------------------------------------------------------------
	std::string eventString();

  private:
	std::map<unsigned short, std::string> m_event_strings;

	DBEVENTINFO                           m_event_info;
};

//==============================================================================
//
//  Summary     : API encapsulation
//
//  Description : This encapsulates the SAPI 5.1 interface
//
//==============================================================================

#endif