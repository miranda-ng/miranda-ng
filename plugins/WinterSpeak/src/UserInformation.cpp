#include "Common.h"
#include "UserInformation.h"


UserInformation::UserInformation() : m_status_info(), m_status_strings() {
	// insert the status strings into a map for easy access
	m_status_strings[ID_STATUS_OFFLINE]   = LPGENT("%u is now offline");
	m_status_strings[ID_STATUS_ONLINE]    = LPGENT("%u is now online");
	m_status_strings[ID_STATUS_AWAY]      = LPGENT("%u is away");
	m_status_strings[ID_STATUS_INVISIBLE] = LPGENT("%u is invisible");
	m_status_strings[ID_STATUS_NA]        = LPGENT("%u is not available");
	m_status_strings[ID_STATUS_DND]       = LPGENT("%u does not want to be disturbed");
	m_status_strings[ID_STATUS_OCCUPIED]  = LPGENT("%u is occupied");
	m_status_strings[ID_STATUS_FREECHAT]  = LPGENT("%u is free for chat");
}

//------------------------------------------------------------------------------
UserInformation::~UserInformation()
{
}

//------------------------------------------------------------------------------
bool UserInformation::updateStatus(MCONTACT user, int status)
{
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
std::wstring UserInformation::statusString(MCONTACT user)
{
	return m_status_strings[m_status_info[user]];
}

//------------------------------------------------------------------------------
std::wstring UserInformation::statusModeString(MCONTACT user)
{
	WCHAR *status = pcli->pfnGetStatusModeDescription(m_status_info[user], 0);
	if (NULL == status)
		return L"";

	return status;
}

//------------------------------------------------------------------------------
void UserInformation::insertName(std::wstring &str, MCONTACT user) const
{
	// insert the user's name into the string
	str.replace(str.find(L"%u"), 2, nameString(user));
}

//------------------------------------------------------------------------------
std::wstring UserInformation::nameString(MCONTACT user) const
{
	WCHAR *ret = pcli->pfnGetContactDisplayName(user, 0);
	if (0 == ret)
		return L"";

	return TranslateW(ret);
}

//==============================================================================
