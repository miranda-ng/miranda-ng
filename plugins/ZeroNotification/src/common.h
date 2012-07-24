#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <statusmodes.h>
#include <m_protosvc.h>

#define DEFAULT_NOSOUND		0x00000000
#define DEFAULT_NOBLINK 	0x00000000
#define DEFAULT_NOCLCBLINK 0x00000000

#define DISABLE_SOUND "Disable &Sounds"
#define ENABLE_SOUND "Enable &Sounds"

