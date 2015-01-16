#pragma once

#include "Common.h"
#include <m_database.h>

class SpeakAnnounce
{
public:
	SpeakAnnounce(HINSTANCE instance);
	~SpeakAnnounce();

	//--------------------------------------------------------------------------
	// Description : handle a status change
	//--------------------------------------------------------------------------
	void statusChange(DBCONTACTWRITESETTING *write_setting, MCONTACT user);

	//--------------------------------------------------------------------------
	// Description : handle an event
	//--------------------------------------------------------------------------
	void incomingEvent(MCONTACT user, MEVENT event);

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
	void message(const std::wstring &sentence, MCONTACT user);
	void status(const std::wstring &sentence, MCONTACT user);

private:
	//--------------------------------------------------------------------------
	// Description : check if the users message window is open and focused
	// Parameters  : contact - the user to check for
	// Returns     : true = message window is open
	//               false = message window not open
	//--------------------------------------------------------------------------
	bool readMessage(MCONTACT contact);

	HINSTANCE           m_instance;

	AnnounceDatabase    m_db;
	AnnounceDialog      m_dialog;
	UserInformation     m_user_info;
	EventInformation    m_event_info;
	ProtocolInformation m_protocol_info;
};
