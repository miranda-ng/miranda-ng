/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __COMMONS_H__
# define __COMMONS_H__


#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <win2k.h>
#include <commctrl.h>
#include <stdio.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_awaymsg.h>
#include <m_contacts.h>
#include <m_options.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_proto_listeningto.h>
#include <m_listeningto.h>

#include <m_NewAwaySys.h>
#include <m_updater.h>
#include <m_fontservice.h>
#include <m_variables.h>
#include <m_avatars.h>
#include <m_statusplugins.h>
#include <m_ersatz.h>
#include <m_icq.h>
#include <m_icolib.h>
#include "m_cluiframes.h"
#include "m_simpleaway.h"

#include <richedit.h>
#include <m_smileyadd.h>

#include <io.h>

#include "resource.h"


#define MODULE_NAME "MyDetails"

#define SETTING_FRAME_VISIBLE "FrameVisible"
#define SETTING_DEFAULT_NICK "DefaultNick"


extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;

extern long nickname_dialog_open;
extern long status_msg_dialog_open;


#include "m_mydetails.h"
#include "data.h"
#include "options.h"
#include "frame.h"
#include "../utils/mir_smileys.h"
#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/mir_icons.h"


#define PS_SETMYAVATAR "/SetMyAvatar"
#define PS_GETMYAVATAR "/GetMyAvatar"
#define PS_GETMYAVATARMAXSIZE "/GetMyAvatarMaxSize"

#define PS_SETMYNICKNAME "/SetNickname"

#define PS_GETMYNICKNAMEMAXLENGTH "/GetMyNicknameMaxLength"


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


// See if a protocol service exists
__inline static int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}


// Helper
static __inline int DRAW_TEXT(HDC hDC, LPCSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, const char *protocol, 
					 SmileysParseInfo parseInfo)
{
	if (opts.replace_smileys)
	{
		return Smileys_DrawText(hDC, lpString, nCount, lpRect, uFormat | (opts.resize_smileys ? DT_RESIZE_SMILEYS : 0), 
			opts.use_contact_list_smileys ? "clist" : protocol, parseInfo);
	}
	else
	{
		return DrawText(hDC, lpString, nCount, lpRect, uFormat);
	}
}




#endif // __COMMONS_H__
