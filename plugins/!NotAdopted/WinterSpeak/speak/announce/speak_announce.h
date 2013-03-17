#ifndef guard_speak_announce_speak_announce_h
#define guard_speak_announce_speak_announce_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include "speak.h"
#include "user/user_information.h"
#include "announce/event_information.h"
#include "announce/protocol_information.h"
#include "announce/announce_database.h"
#include "announce/dialog_announce.h"

#include <wtypes.h>
#include <string>
#include <memory>

class SpeakAnnounce
{
  public:
	SpeakAnnounce(HINSTANCE instance);
	~SpeakAnnounce();

	//--------------------------------------------------------------------------
	// Description : handle a status change
	//--------------------------------------------------------------------------
	void statusChange(DBCONTACTWRITESETTING *write_setting, HANDLE user);

	//--------------------------------------------------------------------------
	// Description : handle an event
	//--------------------------------------------------------------------------
	void incomingEvent(HANDLE user, HANDLE event);

	//--------------------------------------------------------------------------
	// Description : handle a protocol state change
	//--------------------------------------------------------------------------
    void protocolAck(ACKDATA *ack);

	//--------------------------------------------------------------------------
	// Description : speak a sentence
	// Parameters  : sentence - the sentence to speak
    //               user - the user who is speaking, or NULL for no user
	// Returns     : true - speak successful
	//               false - speak failed
	//--------------------------------------------------------------------------
    void say(const std::string &sentence, HANDLE user);

  private:
	//--------------------------------------------------------------------------
	// Description : check if the users message window is open and focused
	// Parameters  : contact - the user to check for
	// Returns     : true = message window is open
	//               false = message window not open
	//--------------------------------------------------------------------------
    bool readMessage(HANDLE contact);

    HINSTANCE           m_instance;

	AnnounceDatabase    m_db;
    AnnounceDialog      m_dialog;
    UserInformation     m_user_info;
	EventInformation    m_event_info;
    ProtocolInformation m_protocol_info;
};

//==============================================================================
//
//  Summary     : The main object for the speak plugins
//
//  Description : see summary
//
//==============================================================================

#endif