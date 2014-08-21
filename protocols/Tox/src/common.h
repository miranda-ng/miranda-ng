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

#include "tox\tox.h"

#include "version.h"
#include "resource.h"
#include "tox_proto.h"

extern HINSTANCE g_hInstance;

#define MODULE "Tox"

#define TOX_SETTINGS_ID "ToxID"
#define TOX_SETTINGS_GROUP "DefaultGroup"

#endif //_COMMON_H_