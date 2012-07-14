/*
	Buddy Expectator+ plugin for Miranda-IM (www.miranda-im.org)
	(c)2005 Anar Ibragimoff (ai91@mail.ru)
	(c)2006 Scott Ellis (mail@scottellis.com.au)
	(c)2007 Alexander Turyak (thief@miranda-im.org.ua)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	File name      : $URL: http://svn.miranda.im/mainrepo/buddyexpectator/trunk/common.h $
	Revision       : $Rev: 1392 $
	Last change on : $Date: 2009-04-22 17:46:08 +0300 (Ср, 22 апр 2009) $
	Last change by : $Author: Thief $
*/

#define _CRT_SECURE_NO_WARNINGS
#ifndef _COMMON_INC
#define _COMMON_INC
#define MIRANDA_VER 0x0A00

#include <stddef.h> // needed by m_icolib.h > r9256
#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <stdio.h>
#include <locale.h>
#include "resource.h"
#include <newpluginapi.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_contacts.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_ignore.h>
#include <m_message.h>
#include <m_userinfo.h>
#include <m_icolib.h>
#include "m_popup.h"
#include "m_popup2.h"
#include "m_cluiframes.h"

#define MODULE_NAME     "BuddyExpectator"

extern HINSTANCE hInst;

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
extern DWORD timer_id;

#endif
