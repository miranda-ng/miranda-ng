#include "Common.h"
#include "ConfigDatabase.h"


namespace
{
	const char *SPEAK        = "speak_config";
	const char *ACTIVE_FLAGS = "active_flags";
	const char *ACTIVE_STATE = "active_state";
	const char *ACTIVE_MSG   = "active_msg";
	const char *WELCOME_MSG  = "welcome_msg";
	const char *ENGINE       = "engine";
	const char *VOICE        = "voice";
	const char *VOLUME       = "volume";
	const char *RATE         = "rate";
	const char *PITCH        = "pitch";
}

ConfigDatabase::ConfigDatabase() : m_voice_desc(), m_active_flags(0), m_welcome_msg(L""), m_active_users()
{	
	// load the database from miranda
	load();
}


ConfigDatabase::~ConfigDatabase(void)
{
}


bool ConfigDatabase::getActiveFlag(ActiveFlag flag) const
{
	return ((m_active_flags & (1 << flag)) != 0);
}

//------------------------------------------------------------------------------
void ConfigDatabase::setActiveFlag(ActiveFlag flag, bool state)
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
ConfigDatabase::act ConfigDatabase::getActiveUser(MCONTACT user) const
{
	ActiveUsersMap::const_iterator iter = m_active_users.find(user);

	if (iter == m_active_users.end())
	{
		// get the unknown user status
		iter = m_active_users.find(0);
		
		if (iter == m_active_users.end())
		{
			ConfigDatabase::act ret;
			ret.message = false;
			ret.status = false;
			return ret;
		}
	}

	return iter->second;
}

//------------------------------------------------------------------------------
void ConfigDatabase::setActiveUser(MCONTACT user, act state)
{
	m_active_users[user].status = state.status;
	m_active_users[user].message = state.message;
}

//------------------------------------------------------------------------------
void ConfigDatabase::load()
{
	m_voice_desc.engine = DBGetContactSettingString(SPEAK, ENGINE, L"");
	m_voice_desc.voice  = DBGetContactSettingString(SPEAK, VOICE, L"");
	m_voice_desc.volume = db_get_dw(NULL, SPEAK, VOLUME, 50);
	m_voice_desc.pitch  = db_get_dw(NULL, SPEAK, PITCH, 50);
	m_voice_desc.rate   = db_get_dw(NULL, SPEAK, RATE, 50);

	m_active_flags = db_get_dw(NULL, SPEAK, ACTIVE_FLAGS, 0xffff);

	m_welcome_msg = DBGetContactSettingString(SPEAK, WELCOME_MSG, L"Welcome to Miranda");

	// iterate through all the users and add them to the list if active
	MCONTACT contact = db_find_first();
	while (NULL != contact) {
		m_active_users[contact].status = (db_get_b(contact, SPEAK, ACTIVE_STATE, true) != 0);
		m_active_users[contact].message = (db_get_b(contact, SPEAK, ACTIVE_MSG, true) != 0);

		contact = db_find_next(contact); 
	}

	// load unknown users setting
	m_active_users[0].status = (db_get_b(NULL, SPEAK, ACTIVE_STATE, true) != 0);
	m_active_users[0].message = (db_get_b(NULL, SPEAK, ACTIVE_MSG, true) != 0);
}

//------------------------------------------------------------------------------
void ConfigDatabase::save()
{
	db_set_ts(NULL, SPEAK, ENGINE, m_voice_desc.engine.c_str());
	db_set_ts(NULL, SPEAK, VOICE, m_voice_desc.voice.c_str());
	db_set_dw(NULL, SPEAK, VOLUME, m_voice_desc.volume);
	db_set_dw(NULL, SPEAK, PITCH, m_voice_desc.pitch);
	db_set_dw(NULL, SPEAK, RATE, m_voice_desc.rate);

	db_set_dw(NULL, SPEAK, ACTIVE_FLAGS, m_active_flags);

	db_set_ts(NULL, SPEAK, WELCOME_MSG, m_welcome_msg.c_str());

	for (ActiveUsersMap::iterator i = m_active_users.begin(); i != m_active_users.end(); ++i)
	{
		db_set_b(i->first, SPEAK, ACTIVE_STATE, i->second.status);
		db_set_b(i->first, SPEAK, ACTIVE_MSG, i->second.message);
	}

	// notify the subjects that things have changed
	notify();
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
std::wstring ConfigDatabase::DBGetContactSettingString(const char *szModule, const char *szSetting, const WCHAR *def)
{
	std::wstring ret = def;
	DBVARIANT dbv;
	
	if (!db_get_ts(NULL, szModule, szSetting, &dbv))
	{
		ret = dbv.pwszVal;
	}
	
	return ret;
}

//==============================================================================
