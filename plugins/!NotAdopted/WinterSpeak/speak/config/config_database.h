#ifndef guard_speak_config_config_database_h
#define guard_speak_config_config_database_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include "defs/voice_desc.h"

#include <general/observer/subject.h>

#include <windows.h>
#include <string>
#include <map>

class ConfigDatabase : public Subject
{
  public:
	ConfigDatabase();
	~ConfigDatabase();

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

    typedef std::map<HANDLE, bool> ActiveUsersMap;

	//--------------------------------------------------------------------------
	// Description : get/set the voice description
	//--------------------------------------------------------------------------
    VoiceDesc getVoiceDesc() const           { return m_voice_desc; }
    void setVoiceDesc(const VoiceDesc &desc) { m_voice_desc = desc; }

	//--------------------------------------------------------------------------
	// Description : get/set the welcome message
	//--------------------------------------------------------------------------
	const std::string & getWelcomeMessage() const  { return m_welcome_msg; }
	void setWelcomeMessage(const std::string &msg) { m_welcome_msg = msg; }

	//--------------------------------------------------------------------------
	// Description : get/set an status flags
	//--------------------------------------------------------------------------
    bool getActiveFlag(ActiveFlag flag) const;
	void setActiveFlag(ActiveFlag flag, bool state);

	//--------------------------------------------------------------------------
	// Description : get/set the user active flag
	//--------------------------------------------------------------------------
    bool getActiveUser(HANDLE user) const;
    void setActiveUser(HANDLE user, bool state);
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
	static std::string DBGetContactSettingString(const char *szModule,
		const char *szSetting, const char *def);

    VoiceDesc      m_voice_desc;
    unsigned int   m_active_flags;
	std::string    m_welcome_msg;
    ActiveUsersMap m_active_users;
};

//==============================================================================
//
//  Summary     : encapsulate the access to the miranda database
//
//  Description : Provide a subject that allows clients to know when changes
//                are made to the miranda database.
//
//==============================================================================

#endif