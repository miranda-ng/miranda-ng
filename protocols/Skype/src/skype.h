#pragma once

#include <windows.h>
#include <shlwapi.h>
#include <ShellAPI.h>
#include <malloc.h>
#include <tlhelp32.h>
#include <string>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include <map>
#include <string>

#include <newpluginapi.h>
#include <m_system_cpp.h>
#include <m_assocmgr.h>
#include <m_avatars.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_core.h>
#include <m_database.h>
#include <m_ignore.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_userinfo.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_history.h>
#include <m_xml.h>
#include <win2k.h>
#include <m_timezones.h>

#include <m_msg_buttonsbar.h>
#include <m_folders.h>

#include "resource.h"
#include "version.h"

#define MODULE "Skype"
#define SKYPE_SID_LIMIT 128
#define SKYPE_PASSWORD_LIMIT 128
#define SKYPE_GROUP_NAME_LIMIT 100

#define SKYPE_SETTINGS_STATUS "Status"
#define SKYPE_SETTINGS_SID "sid"
#define SKYPE_SETTINGS_PASSWORD "Password"
#define SKYPE_SETTINGS_DEF_GROUP "DefaultGroup"

#define SKYPE_SEARCH_BYSID 1001
#define SKYPE_SEARCH_BYEMAIL 1002
#define SKYPE_SEARCH_BYNAMES 1003

#define BBB_ID_CONF_INVITE 2001
#define BBB_ID_CONF_SPAWN 2002

#define SKYPE_DB_EVENT_TYPE_EMOTE 10001
#define SKYPE_DB_EVENT_TYPE_CONTACTS 10002
#define SKYPE_DB_EVENT_TYPE_CALL 10010

enum
{
	CMI_AUTH_REQUEST,
	CMI_AUTH_GRANT,
	CMI_AUTH_REVOKE,
	CMI_BLOCK,
	CMI_HISTORY,
	CMI_MAX   // this item shall be the last one
};

#define SMI_CHAT_CREATE 1
#define SMI_IGNORE_LIST 2

#include "string_list.h"

#include "skypekit\common.h"
#include "skypekit\group.h"
#include "skypekit\search.h"
#include "skypekit\account.h"
#include "skypekit\contact.h"
#include "skypekit\message.h"
#include "skypekit\transfer.h"
#include "skypekit\participant.h"
#include "skypekit\conversation.h"

#include "skype_proto.h"

extern HINSTANCE g_hInstance;

extern int g_cbCountries;
extern struct CountryListEntry* g_countries;