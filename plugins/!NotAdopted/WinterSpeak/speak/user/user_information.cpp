//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "user_information.h"

#include "speak.h"

#include <general/debug/debug.h>

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
UserInformation::UserInformation()
	:
	m_status_info(),
	m_status_strings()
{
	CLASSCERR("UserInformation::UserInformation");

	// insert the status strings into a map for easy access
	m_status_strings[ID_STATUS_OFFLINE]   = "%u is now offline";
	m_status_strings[ID_STATUS_ONLINE]    = "%u is now online";
	m_status_strings[ID_STATUS_AWAY]      = "%u is away";
	m_status_strings[ID_STATUS_INVISIBLE] = "%u is invisible";
	m_status_strings[ID_STATUS_NA]        = "%u is not available";
	m_status_strings[ID_STATUS_DND]       = "%u does not want to be disturbed";
	m_status_strings[ID_STATUS_OCCUPIED]  = "%u is occupied";
	m_status_strings[ID_STATUS_FREECHAT]  = "%u is free for chat";
}

//------------------------------------------------------------------------------
UserInformation::~UserInformation()
{
	CLASSCERR("UserInformation::~UserInformation");
}

//------------------------------------------------------------------------------
bool
UserInformation::updateStatus(HANDLE user, int status)
{
	CLASSCERR("UserInformation::updateStatus(," << status << ")");

	bool ret = false;

    // if the user exists and their status hasn't changed, then return false
	if ((m_status_info.find(user) != m_status_info.end())
		&& (m_status_info[user] != status))
	{
		ret = true;
	}

	// update the status
	m_status_info[user] = status;

	return ret;
}

//------------------------------------------------------------------------------
std::string 
UserInformation::statusString(HANDLE user)
{
	CLASSCERR("UserInformation::statusString()");

	return m_status_strings[m_status_info[user]];
}

//------------------------------------------------------------------------------
std::string 
UserInformation::statusModeString(HANDLE user)
{
	CLASSCERR("UserInformation::statusModeString()");

    int status = CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, 
        m_status_info[user], 0);

    if (NULL == status)
    {
        return "";
    }

    return reinterpret_cast<char *>(status);
}

//------------------------------------------------------------------------------
void 
UserInformation::insertName(std::string &str, HANDLE user) const
{
	CLASSCERR("UserInformation::insertName(" << str << ",)");
	
	// insert the user's name into the string
	str.replace(str.find("%u"), 2, nameString(user));
}

//------------------------------------------------------------------------------
std::string 
UserInformation::nameString(HANDLE user) const
{
	CLASSCERR("UserInformation::nameString()");

    char *ret = reinterpret_cast<char *>(
		CallService(MS_CLIST_GETCONTACTDISPLAYNAME,
		reinterpret_cast<unsigned int>(user), 
        0));

    if (0 == ret)
    {
        return "";
    }

    return Translate(ret);
}

//==============================================================================
