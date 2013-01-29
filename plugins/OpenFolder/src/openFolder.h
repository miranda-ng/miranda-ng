#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_hotkeys.h>
#include <m_toptoolbar.h>
#include <win2k.h>

#define MS_OPENFOLDER_OPEN                "openFolder/Open"

#define OPENFOLDER_VERSION                PLUGIN_MAKE_VERSION( 1, 1, 0, 0 )
#define OPENFOLDER_DESCRIPTION            "Adds a menu/toobar item which opens the main Miranda NG folder."
//#define OPENFOLDER_DB_MODULENAME          "openFolder"
#define OPENFOLDER_MODULE_NAME            "openfolder"
