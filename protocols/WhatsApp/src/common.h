/*

WhatsApp plugin for Miranda NG
Copyright © 2013-14 Uli Hecht

*/

#pragma once

//#pragma warning(push)
//#pragma warning(disable:4312)
#pragma warning(disable:4996)
#pragma warning(disable:4290)

#define _WIN32_WINNT   0x0500
#define _WIN32_WINDOWS 0x0500

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <list>
#include <map>
#include <vector>
#include <ctime>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <io.h>
#include <iomanip>
#include <windows.h>
#include <win2k.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_database.h>
#include <m_history.h>
#include <m_idle.h>
#include <m_imgsrvc.h>
#include <m_ignore.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_protomod.h>
#include <m_skin.h>
#include <m_string.h>
#include <statusmodes.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_xml.h>
#include <m_hotkeys.h>
#include <m_folders.h>
#include <m_json.h>

#include "constants.h"
#include "utils.h"
#include "db.h"
#include "resource.h"
#include "dialogs.h"
#include "theme.h"
#include "definitions.h"
#include "WASocketConnection.h"
#include "proto.h"
#include "entities.h"

#if defined _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#endif

//#pragma warning(pop)

extern HINSTANCE g_hInstance;
extern std::string g_strUserAgent;
extern DWORD g_mirandaVersion;
