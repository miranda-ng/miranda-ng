//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "config_database.h"

#include "speak.h"

#include <general/debug/debug.h>

//------------------------------------------------------------------------------
namespace
{
    const char *SPEAK        = "speak_config";
    const char *ACTIVE_FLAGS = "active_flags";
    const char *ACTIVE_STATE = "active_state";
    const char *WELCOME_MSG  = "welcome_msg";
    const char *ENGINE       = "engine";
    const char *VOICE        = "voice";
    const char *VOLUME       = "volume";
    const char *RATE         = "rate";
    const char *PITCH        = "pitch";
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
ConfigDatabase::ConfigDatabase()
	:
    m_voice_desc(),
    m_active_flags(0),
    m_welcome_msg(""),
    m_active_users()
{	
	CLASSCERR("ConfigDatabase::ConfigDatabase");

	// load the database from miranda
	load();
}

//------------------------------------------------------------------------------
ConfigDatabase::~ConfigDatabase()
{
	CLASSCERR("ConfigDatabase::~ConfigDatabase");
}

//------------------------------------------------------------------------------
bool 
ConfigDatabase::getActiveFlag(ActiveFlag flag) const
{
    return ((m_active_flags & (1 << flag)) != 0);
}

//------------------------------------------------------------------------------
void 
ConfigDatabase::setActiveFlag(ActiveFlag flag, bool state)
{ 
    if (state)
    {
        m_active_flags |= (1 << flag);
    }
    else
    {
        m_active_flags &= ~(1 << flag);
    }
}

//------------------------------------------------------------------------------
bool 
ConfigDatabase::getActiveUser(HANDLE user) const
{
    ActiveUsersMap::const_iterator iter = m_active_users.find(user);

	if (iter == m_active_users.end())
    {
        // get the unknown user status
        iter = m_active_users.find(0);
        
        if (iter == m_active_users.end())
        {
            CLASSCERR("ConfigDatabase::getActiveUser user error");
            return false;
        }
    }

    return iter->second;
}

//------------------------------------------------------------------------------
void 
ConfigDatabase::setActiveUser(HANDLE user, bool state)
{
    m_active_users[user] = state;
}

//------------------------------------------------------------------------------
void
ConfigDatabase::load()
{
	CLASSCERR("ConfigDatabase::load");

	m_voice_desc.engine = DBGetContactSettingString(SPEAK, ENGINE, "");
	m_voice_desc.voice  = DBGetContactSettingString(SPEAK, VOICE, "");
    m_voice_desc.volume = DBGetContactSettingDword(NULL, SPEAK, VOLUME, 50);
    m_voice_desc.pitch  = DBGetContactSettingDword(NULL, SPEAK, PITCH, 50);
    m_voice_desc.rate   = DBGetContactSettingDword(NULL, SPEAK, RATE, 50);

    m_active_flags = DBGetContactSettingDword(NULL, SPEAK, ACTIVE_FLAGS, 0xffff);

	m_welcome_msg = DBGetContactSettingString(SPEAK, WELCOME_MSG, 
        "welcome to i c q");

    // iterate through all the users and add them to the list if active
	HANDLE contact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

	while (NULL != contact)
	{
        m_active_users[contact]
            = DBGetContactSettingByte(contact, SPEAK, ACTIVE_STATE, true);

		contact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)contact, 0);
	}

    // load unknown users setting
    m_active_users[0] = DBGetContactSettingByte(NULL, SPEAK, ACTIVE_STATE, true);
}

//------------------------------------------------------------------------------
void 
ConfigDatabase::save()
{
	CLASSCERR("ConfigDatabase::save");

	DBWriteContactSettingString(NULL, SPEAK, ENGINE, m_voice_desc.engine.c_str());
	DBWriteContactSettingString(NULL, SPEAK, VOICE, m_voice_desc.voice.c_str());
	DBWriteContactSettingDword(NULL, SPEAK, VOLUME, m_voice_desc.volume);
	DBWriteContactSettingDword(NULL, SPEAK, PITCH, m_voice_desc.pitch);
	DBWriteContactSettingDword(NULL, SPEAK, RATE, m_voice_desc.rate);

	DBWriteContactSettingDword(NULL, SPEAK, ACTIVE_FLAGS, m_active_flags);

	DBWriteContactSettingString(NULL, SPEAK, WELCOME_MSG, m_welcome_msg.c_str());

    for (ActiveUsersMap::iterator i = m_active_users.begin(); 
        i != m_active_users.end(); ++i)
    {
        DBWriteContactSettingByte(i->first, SPEAK, ACTIVE_STATE, i->second);
    }

    // notify the subjects that things have changed
    notify();
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
std::string
ConfigDatabase::DBGetContactSettingString(const char *szModule,
	const char *szSetting, const char *def)
{
	std::string ret = def;
	DBVARIANT dbv;

	if (!DBGetContactSetting(NULL, szModule, szSetting, &dbv))
	{
		ret = dbv.pszVal;
	}
    
	return ret;
}

//==============================================================================
