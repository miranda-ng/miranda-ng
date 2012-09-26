#define MIRANDA_VER 0x0A00

#include <windows.h>

#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
//#include "m_cluiframes.h"
#include <m_database.h>
#include <m_history.h>
#include <m_idle.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <statusmodes.h>
#include <m_system.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_system_cpp.h>
#include <m_hotkeys.h>
#include <win2k.h>

#include "version.h"
#include "skype_proto.h"

// skype

#define SSL_LIB_CYASSL 
#define NO_FILESYSTEM

// Enable desktop video
//#define SKYPEKIT_SURFACE_RENDERING

// Additional flags for desktop video for non-Windows targets
//#define VIDEO_TRANSPORT_SYSV
//#define VIDEO_TRANSPORT_POSIX

extern HINSTANCE g_hInstance;