#ifndef MENUEX_H__
#define MENUEX_H__

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_system_cpp.h>
#include <m_options.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_file.h>
#include <m_protomod.h>
#include <m_clui.h>
#include <m_ignore.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_genmenu.h>
#include <m_popup.h>
#include <m_message.h>

#include <m_fingerprint.h>
#include <m_metacontacts.h>
#include <m_msg_buttonsbar.h>

#include "images.h"
#include "version.h"
#include "resource.h"

#define MODULENAME "MenuItemEx"

#define MIIM_STRING	0x00000040

extern PLUGININFOEX pluginInfoEx;

int OptionsInit(WPARAM, LPARAM);

#define MAX_PROTOS	32
#define MAX_GROUPS	32
#define MAX_IDLEN	16

#define VF_VS		0x00000001
#define	VF_SMNAME	0x00000002
#define	VF_HFL		0x00000004
#define VF_IGN		0x00000008
#define VF_PROTO	0x00000010
#define VF_SHOWID	0x00000020
#define VF_ADD		0x00000040
#define VF_REQ		0x00000080
#define VF_CID		0x00000100
#define VF_RECV		0x00000200
#define VF_STAT		0x00000400
#define VF_CIDN		0x00000800
#define VF_CIP		0x00001000
#define VF_SAI		0x00002000
#define VF_TRIMID	0x00004000
#define VF_CMV		0x00008000
#define VF_IGNH		0x00010000

#define CTRL_IS_PRESSED (GetAsyncKeyState(VK_CONTROL)&0x8000)

extern const int vf_default;

#endif
