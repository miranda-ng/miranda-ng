#pragma once

#include "Common.h"
#include "ConfigDatabase.h"
#include "TextToSpeech.h"

class TextToSpeech;

class SpeakConfig : public Observer
{
public:
	SpeakConfig(HINSTANCE instance);
	virtual ~SpeakConfig();

	//--------------------------------------------------------------------------
	// Description : Called by a subject that this observer is observing
	//               to signify a change in state
	// Parameters  : subject - the subject that changed
	//--------------------------------------------------------------------------
	virtual void update(Subject &subject);

	//--------------------------------------------------------------------------
	// Description : speak a sentence
	// Parameters  : sentence - the sentence to speak
	//               user - the user to associate the say with
	//                      NULL = ignore user
	// Returns     : true - speak successful
	//               false - speak failed
	//--------------------------------------------------------------------------
	bool status(const std::wstring &sentence, MCONTACT user = NULL);
	bool message(const std::wstring &sentence, MCONTACT user = NULL);
	bool say(const std::wstring &sentence, MCONTACT user, bool message);

private:
	HINSTANCE                   m_instance;

	ConfigDatabase              m_db;
	std::auto_ptr<TextToSpeech> m_tts;
	DialogConfigEngine          m_dialog_engine;
	DialogConfigActive          m_dialog_active;
};