#ifndef MENUEX_H__
#define MENUEX_H__

#define _CRT_SECURE_NO_DEPRECATE
#define MIRANDA_VER    0x0700

#include "m_stdhdr.h"
#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include "newpluginapi.h"
#include "win2k.h"
#include "m_system.h"
#include "m_utils.h"
#include "m_options.h"
#include "m_database.h"
#include "m_langpack.h"
#include "m_file.h"
#include "m_clist.h"
#include "m_protosvc.h"
#include "m_protomod.h"
#include "m_clui.h"
#include "m_ignore.h"
#include "m_icolib.h"
#include "m_skin.h"
#include "m_genmenu.h"
#include "m_popup.h"
#include "m_message.h"
#include "m_updater.h"
#include "m_fingerprint.h"
#include "m_metacontacts.h"
#include "m_msg_buttonsbar.h"

#pragma optimize("gsy",on)

#define VISPLG "MenuItemEx"
#define MODULENAME VISPLG

#define MIIM_STRING	0x00000040

extern PLUGINLINK *pluginLink;
extern PLUGININFO pluginInfo;
extern PLUGININFOEX pluginInfoEx;
extern MUUID interfaces[];

int OptionsInit(WPARAM,LPARAM);

#define MAX_PROTOS	32
#define MAX_GROUPS	32
#define MAX_IDLEN	16

#define VF_VS		0x0001
#define	VF_SMNAME	0x0002
#define	VF_HFL		0x0004
#define VF_IGN		0x0008
#define VF_PROTO	0x0010
#define VF_SHOWID	0x0020
#define VF_ADD		0x0040
#define VF_REQ		0x0080
#define VF_CID		0x0100
#define VF_RECV		0x0200
#define VF_STAT		0x0400
#define VF_CIDN		0x0800
#define VF_CIP		0x1000
#define VF_SAI		0x2000
#define VF_TRIMID	0x4000
#define VF_CMV		0x8000

#define CTRL_IS_PRESSED (GetAsyncKeyState(VK_CONTROL)&0x8000)

extern const int vf_default;
extern BOOL bMir_08;

#endif
