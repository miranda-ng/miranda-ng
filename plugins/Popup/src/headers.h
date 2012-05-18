/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*
===============================================================================
                           PopUp plugin

This plugin notifies you when a contact changes his/hers status with a PopUp.
You can customize its look (style of popup, font, text...) and behaviour (the
position in the screen, the time to live, the action on a mouse click).

File name: "headers.h"
This file has the purpose of becoming a precompiled header common to every
project file.

Written by: Hrk (Luca Santarelli)
Updated by: Zazoo (Victor Pavlychko)

Miranda IM can be found here:
http://www.miranda-im.org/

Miranda IM plugins and tools can be found here:
http://addons.miranda-im.org/

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/headers.h $
Revision       : $Revision: 1651 $
Last change on : $Date: 2010-07-15 20:31:06 +0300 (Ð§Ñ‚, 15 Ð¸ÑŽÐ» 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#ifndef HEADERS_H
#define HEADERS_H

	// disable security warnings about "*_s" functions
	#define _CRT_SECURE_NO_DEPRECATE

	// disable warnings about underscore in stdc functions
	#pragma warning(disable: 4996)

	#define _WIN32_WINNT 0x0501
	#define WINVER 0x0500
	#define OEMRESOURCE

	#include <windows.h>
	#include <windowsx.h>
	#include <winuser.h>

	//Standard headers
	#include <stdlib.h>
	#include <stdio.h>
	#include <time.h>
	#include <malloc.h>

	//Windows headers
	#include <process.h>
	#include <commctrl.h>
	#include <commdlg.h>

#ifdef ComboBox_SelectItemData
 // use Workaround for MS bug ComboBox_SelectItemData;
 #undef ComboBox_SelectItemData
#endif

	//Resources
	#include "../resource.h"

	#define NOWIN2K
	#define MIRANDA_VER 0x0A00

	//Miranda API (see Miranda svn)
	#include <newpluginapi.h>
	#include <win2k.h>
	#include <m_system.h>
	#include <m_plugins.h>
	#include <m_clui.h>
	#include <m_clist.h>
	#include <m_options.h>
	#include <m_skin.h>
	#include <m_langpack.h>
	#include <m_database.h>
		#undef DBGetContactSettingString
	#include <m_protocols.h>
	#include <m_protosvc.h>
	#include <m_utils.h>
	#include <m_button.h>
	#include <m_message.h>
	#include <m_userinfo.h>
	#include <m_addcontact.h>
	#include <m_png.h>
	#include <m_clc.h>
	#include <m_icolib.h>
	#include <m_hotkeys.h>
	#include <m_fontservice.h>
	#include <m_avatars.h>
	#include <m_metacontacts.h>

	#include <m_system_cpp.h>


	// API for 3rd party plugins (\include_API folder)
 	// this folder contain always the latest API
	#include <m_folders.h>
	#include <m_hpp.h>
	#include <m_ieview.h>
	#include <m_nconvers.h>
//	#include <m_notify.h>		//deprecatet
	#include <m_notify_popup.h>
	#include <m_smileyadd.h>
	#ifndef MTEXT_NOHELPERS
		#define MTEXT_NOHELPERS
	#endif // MTEXT_NOHELPERS
	#include <m_text.h>
	#include <m_ticker.h>
	#include <m_toolbar.h>
	#include <m_toptoolbar.h>
	#include <m_updater.h>
	#include <m_popup.h>			//core define see miranda\include\

	// API for 3rd party plugins (.\api folder)
	// this folder contain spetial edition API (not latest API !!!)
	#include "m_mathmodule.h"
	#include "m_popup2.h"

	//PopUp common handlers
	#include "defs.h"
	#include "../version.h"
	#include "config.h"
	#include "common.h"
	#include "def_settings.h"
//	#include "notify_imp.h"		//deprecatet
	#include "opttree.h"
//	#include "opt_old.h"		//deprecatet
	#include "opt_gen.h"
//	#include "opt_notify.h"		//deprecatet
	#include "opt_skins.h"
	#include "opt_contacts.h"
	#include "opt_adv.h"
	#include "history.h"
	#include "services.h"
	#include "srmm_menu.h"
	#include "bitmap_funcs.h"
	#include "icons.h"
	#include "font.h"
	#include "formula.h"
	#include "skin.h"
	#include "popup_thread.h"
	#include "actions.h"
	#include "notifications.h"
	#include "opt_class.h"
	#include "popup_wnd2.h"
	#include "effects.h"
//	#include "popup_queue.h"
//	#include "popup_list.h"

	#include "avatars.h"
	#include "avatars_simple.h"
	#include "avatars_flash.h"
	#include "avatars_gif.h"

	#include "popup_gdiplus.h"

INT_PTR svcEnableDisableMenuCommand(WPARAM, LPARAM);
extern HANDLE hSquareFad;

#endif //HEADERS_H
