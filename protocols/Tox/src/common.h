#ifndef _COMMON_H_
#define _COMMON_H_

#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <regex>
#include <map>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protomod.h>
#include <m_protosvc.h>

#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_message.h>

#include <tox.h>

#include "version.h"
#include "resource.h"
#include "tox_proto.h"

extern HINSTANCE g_hInstance;

#define MODULE "Tox"

#define TOX_ERROR -1

#define TOX_SETTINGS_ID "ToxID"
#define TOX_SETTINGS_GROUP "DefaultGroup"

#define TOX_DB_EVENT_TYPE_ACTION 10001

#endif //_COMMON_H_