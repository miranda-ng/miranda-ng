#include "Common.h"
#include "SpeakConfig.h"


SpeakConfig::SpeakConfig(HINSTANCE instance) : m_instance(instance), m_db(), m_tts(0), m_dialog_engine(m_db),  m_dialog_active(m_db)
{
	// create and configure the tts
	SpeechInterface si;
	VoiceDesc desc = m_db.getVoiceDesc();
	m_tts = std::auto_ptr<TextToSpeech>(si.createTts(desc.engine));
	si.configureTts(m_tts.get(), desc);
	
	// observer the database for changes
	m_db.attach(*this);

	message(m_db.getWelcomeMessage());
}

//------------------------------------------------------------------------------
SpeakConfig::~SpeakConfig()
{
}

//------------------------------------------------------------------------------
void SpeakConfig::update(Subject &subject)
{
	ConfigDatabase &db = static_cast<ConfigDatabase &>(subject);

	SpeechInterface si;
	VoiceDesc desc = db.getVoiceDesc();
	m_tts = std::auto_ptr<TextToSpeech>(si.createTts(desc.engine));
	si.configureTts(m_tts.get(), desc);
}

//------------------------------------------------------------------------------
bool SpeakConfig::status(const std::wstring &sentence, MCONTACT user) {
	return say(sentence, user, false);
}
bool SpeakConfig::message(const std::wstring &sentence, MCONTACT user) {
	return say(sentence, user, true);
}
bool SpeakConfig::say(const std::wstring &sentence, MCONTACT user, bool message)
{
	if (!m_tts.get()){
		return false;
	}

	bool active = true;

	if (NULL != user)
	{
		// get the status of the protocol of this user
		const char *protocol = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)user, 0);

		switch (CallProtoService(protocol, PS_GETSTATUS, 0, 0))
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
			case ID_STATUS_OFFLINE:
				// if we are offline for this protocol, then don't speak the 
				// sentence this is so we don't announce users offline status if 
				// we are disconnected.
				active = false;
				break;
			default:
				active = false;
				break;
		}

		// if its a user say, then check the users status
		if (active)
		{
			if(message)
				active = m_db.getActiveUser(user).message;
			else
				active = m_db.getActiveUser(user).status;
		}
	}

	if (!active)
	{
		return false;
	}

	bool ret = m_tts->say(sentence);
	return ret;
}
