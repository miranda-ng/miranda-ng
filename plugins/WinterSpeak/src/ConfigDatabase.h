#pragma once

#include <map>
#include "voice_desc.h"
#include "Subject.h"

class ConfigDatabase : public Subject
{
public:
	ConfigDatabase(void);
	~ConfigDatabase(void);

	enum ActiveFlag
    {
        ActiveFlag_Online = 1,
        ActiveFlag_Away,
        ActiveFlag_Dnd,
        ActiveFlag_Na,
        ActiveFlag_Occupied,
        ActiveFlag_FreeForChat,
        ActiveFlag_Invisible
    };

	struct act {
		bool status;
		bool message;
	};

	typedef std::map<MCONTACT, act> ActiveUsersMap;

	//--------------------------------------------------------------------------
	// Description : get/set the voice description
	//--------------------------------------------------------------------------
	VoiceDesc getVoiceDesc() const           { return m_voice_desc; }
	void setVoiceDesc(const VoiceDesc &desc) { m_voice_desc = desc; }

	//--------------------------------------------------------------------------
	// Description : get/set the welcome message
	//--------------------------------------------------------------------------
	const std::wstring & getWelcomeMessage() const  { return m_welcome_msg; }
	void setWelcomeMessage(const std::wstring &msg) { m_welcome_msg = msg; }

	//--------------------------------------------------------------------------
	// Description : get/set an status flags
	//--------------------------------------------------------------------------
	bool getActiveFlag(ActiveFlag flag) const;
	void setActiveFlag(ActiveFlag flag, bool state);

	//--------------------------------------------------------------------------
	// Description : get/set the user active flag
	//--------------------------------------------------------------------------
	act getActiveUser(MCONTACT user) const;
	void setActiveUser(MCONTACT user, act state);
	ActiveUsersMap getActiveUsers() const { return m_active_users; }

	//--------------------------------------------------------------------------
	// Description : load/save the settings from the miranda database
	//--------------------------------------------------------------------------
	void load();
	void save();

private:
	//--------------------------------------------------------------------------
	// Description : For some reason this isn't implemented in miranda yet
	//               Just get a string from the db 
	// Parameters  : szModule  - the entrys' module
	//               szSetting - the entrys' setting
	//               def       - default string if entry doesn't exist
	//--------------------------------------------------------------------------
	static std::wstring DBGetContactSettingString(const char *szModule,
		const char *szSetting, const wchar_t *def);

	VoiceDesc      m_voice_desc;
	unsigned int   m_active_flags;
	std::wstring m_welcome_msg;
	//std::string    m_welcome_msg;
	ActiveUsersMap m_active_users;
};
