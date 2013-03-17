#ifndef guard_speak_config_speak_config_h
#define guard_speak_config_speak_config_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include "speak.h"
#include "config/config_database.h"
#include "config/dialog_config_active.h"
#include "config/dialog_config_engine.h"

#include <general/observer/observer.h>

#include <wtypes.h>
#include <memory>

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
    bool say(const std::string &sentence, HANDLE user = NULL);

  private:
    HINSTANCE                   m_instance;

	ConfigDatabase              m_db;
    std::auto_ptr<TextToSpeech> m_tts;
    DialogConfigEngine          m_dialog_engine;
    DialogConfigActive          m_dialog_active;
};

//==============================================================================
//
//  Summary     : The main object for the speak plugins
//
//  Description : see summary
//
//==============================================================================

#endif