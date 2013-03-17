//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "speak_config.h"

#include "defs/voice_desc.h"
#include "config/speech_interface.h"
#include "config/dialog_config_engine.h"
#include "config/dialog_config_active.h"

#include <general/debug/debug.h>
#include <general/text_to_speech/text_to_speech/text_to_speech.h>
//#include <general/multimedia/winamp_2/winamp_2.h>

#include <sstream>

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
SpeakConfig::SpeakConfig(HINSTANCE instance)
	:
    m_instance(instance),
	m_db(),
    m_tts(0),
	m_dialog_engine(m_db),
    m_dialog_active(m_db)
{
	CLASSCERR("SpeakConfig::SpeakConfig");

	// create and configure the tts
    SpeechInterface si;
    VoiceDesc desc = m_db.getVoiceDesc();
	m_tts = std::auto_ptr<TextToSpeech>(si.createTts(desc.engine));
    si.configureTts(m_tts.get(), desc);
    
    // observer the database for changes
    m_db.attach(*this);

    say(m_db.getWelcomeMessage());
}

//------------------------------------------------------------------------------
SpeakConfig::~SpeakConfig()
{
	CLASSCERR("SpeakConfig::~SpeakConfig");
}

//------------------------------------------------------------------------------
void
SpeakConfig::update(Subject &subject)
{
    CLASSCERR("SpeakConfig::update(" << &subject << ")");

    ConfigDatabase &db = static_cast<ConfigDatabase &>(subject);

    SpeechInterface si;
    VoiceDesc desc = db.getVoiceDesc();
  	m_tts = std::auto_ptr<TextToSpeech>(si.createTts(desc.engine));
    si.configureTts(m_tts.get(), desc);
}

//------------------------------------------------------------------------------
bool 
SpeakConfig::say(const std::string &sentence, HANDLE user)
{
	CLASSCERR("SpeakConfig::say(" << sentence << ")");

	if (!m_tts.get())
	{
		return false;
	}

    bool active = true;

    if (NULL != user)
    {
        // get the status of the protocol of this user
        const char *protocol = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, 
            (WPARAM)user, 0);

        if (ID_STATUS_OFFLINE == CallProtoService(protocol, PS_GETSTATUS, 0, 0))
        {
            // if we are offline for this protocol, then don't speak the 
            // sentence this is so we don't announce users offline status if 
            // we are disconnected.
            active = false;
        }
        else
        {
            switch (CallService(MS_CLIST_GETSTATUSMODE, 0, 0))
            {
              case ID_STATUS_ONLINE:
                active = m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Online);
                break;
              case ID_STATUS_AWAY:
                active = m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Away);
                break;
              case ID_STATUS_DND:
                active = m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Dnd);
                break;
              case ID_STATUS_NA:
                active = m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Na);
                break;
              case ID_STATUS_OCCUPIED:
                active = m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Occupied);
                break;
              case ID_STATUS_FREECHAT:
                active = m_db.getActiveFlag(ConfigDatabase::ActiveFlag_FreeForChat);
                break;
              case ID_STATUS_INVISIBLE:
                active = m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Invisible);
                break;
              default:
                active = false;
                break;
            }
        }

        // if its a user say, then check the users status
        if (active && (NULL != user))
        {
            active = m_db.getActiveUser(user);
        }
    }

    if (!active)
    {
        return false;
    }

	bool ret = m_tts->say(sentence);

    return ret;
}

//------------------------------------------------------------------------------
/*int
SpeakConfig::dialogEngine(HWND window, unsigned int message, WPARAM wparam, 
    LPARAM lparam)
{
    if (WM_INITDIALOG == message)
    {
        m_dialog_engine = std::auto_ptr<DialogConfigEngine>
            (new DialogConfigEngine(window, m_db));
    }
    else if (WM_DESTROY == message)
    {
        m_dialog_engine = std::auto_ptr<DialogConfigEngine>(0);
    }

    if (!m_dialog_engine.get())
    {
        return 1;
    }

    // process the message and if settings are changed, reload the tts
    if (m_dialog_engine->process(message, wparam, lparam))
    {
        m_db.load();
        SpeechInterface si;
    	m_tts = std::auto_ptr<TextToSpeech>(si.createTts(m_db.getVoiceDesc()));
    }

    return 0;
}*/

//==============================================================================
