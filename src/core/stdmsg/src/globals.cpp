/*

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "stdafx.h"

GlobalMessageData g_dat;

IconItem iconList[] =
{
	{ LPGEN("Incoming message (10x10)"), "INCOMING", IDI_INCOMING, 10 },
	{ LPGEN("Outgoing message (10x10)"), "OUTGOING", IDI_OUTGOING, 10 },
	{ LPGEN("Notice (10x10)"),           "NOTICE",   IDI_NOTICE,   10 },
};

static void InitIcons(void)
{
	Icon_Register(g_hInst, LPGEN("Messaging"), iconList, _countof(iconList), "SRMM");
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	for (SESSION_INFO *si = pci->wndList; si; si = si->next)
		SendMessage(si->hWnd, WM_CLOSE, 0, 0);

	TabM_RemoveAll();
	ImageList_Destroy(hIconsList);
	return 0;
}

static int OnMetaChanged(WPARAM hMeta, LPARAM)
{
	if (hMeta) {
		HWND hwnd = WindowList_Find(pci->hWindowList, hMeta);
		if (hwnd != NULL)
			SendMessage(hwnd, DM_GETAVATAR, 0, 0);
	}
	return 0;
}

static int dbaddedevent(WPARAM hContact, LPARAM hDbEvent)
{
	if (hContact) {
		HWND h = WindowList_Find(pci->hWindowList, hContact);
		if (h)
			SendMessage(h, HM_DBEVENTADDED, hContact, hDbEvent);

		MCONTACT hEventContact = db_event_getContact(hDbEvent);
		if (hEventContact != hContact)
			if ((h = WindowList_Find(pci->hWindowList, hEventContact)) != NULL)
				SendMessage(h, HM_DBEVENTADDED, hEventContact, hDbEvent);
	}
	return 0;
}

static int ackevent(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (pAck && pAck->type == ACKTYPE_MESSAGE) {
		msgQueue_processack(pAck->hContact, (INT_PTR)pAck->hProcess, pAck->result == ACKRESULT_SUCCESS, (char*)pAck->lParam);

		if (pAck->result == ACKRESULT_SUCCESS)
			SkinPlaySound("SendMsg");
	}
	return 0;
}

int AvatarChanged(WPARAM hContact, LPARAM lParam)
{
	HWND h = WindowList_Find(pci->hWindowList, hContact);
	if (h)
		SendMessage(h, HM_AVATARACK, hContact, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void ReloadGlobals()
{
	memset(&g_dat.flags, 0, sizeof(g_dat.flags));
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE))
		g_dat.flags.bShowButtons = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDBUTTON, SRMSGDEFSET_SENDBUTTON))
		g_dat.flags.bSendButton = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		g_dat.flags.bShowTyping = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN))
		g_dat.flags.bShowTypingWin = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN))
		g_dat.flags.bShowTypingTray = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST))
		g_dat.flags.bShowTypingClist = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS))
		g_dat.flags.bShowIcons = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME))
		g_dat.flags.bShowTime = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AVATARENABLE, SRMSGDEFSET_AVATARENABLE))
		g_dat.flags.bShowAvatar = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE))
		g_dat.flags.bShowDate = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWSECS, SRMSGDEFSET_SHOWSECS))
		g_dat.flags.bShowSecs = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES))
		g_dat.flags.bHideNames = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_CHARCOUNT, SRMSGDEFSET_CHARCOUNT))
		g_dat.flags.bShowReadChar = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER))
		g_dat.flags.bSendOnEnter = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER))
		g_dat.flags.bSendOnDblEnter = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE))
		g_dat.flags.bAutoClose = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOMIN, SRMSGDEFSET_AUTOMIN))
		g_dat.flags.bAutoMin = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		g_dat.flags.bTypingUnknown = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_CTRLSUPPORT, SRMSGDEFSET_CTRLSUPPORT))
		g_dat.flags.bCtrlSupport = true;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWFORMAT, SRMSGDEFSET_SHOWFORMAT))
		g_dat.flags.bShowFormat = true;

	g_dat.openFlags = db_get_dw(NULL, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS);
	g_dat.nFlashMax = db_get_b(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
	g_dat.iGap = db_get_b(NULL, SRMSGMOD, SRMSGSET_BUTTONGAP, SRMSGDEFSET_BUTTONGAP);

	g_dat.msgTimeout = db_get_dw(NULL, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
	if (g_dat.msgTimeout < SRMSGSET_MSGTIMEOUT_MIN) g_dat.msgTimeout = SRMSGDEFSET_MSGTIMEOUT;
}

void InitGlobals()
{
	HookEvent(ME_DB_EVENT_ADDED, dbaddedevent);
	HookEvent(ME_PROTO_ACK, ackevent);
	HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);
	HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	HookEvent(ME_MC_DEFAULTTCHANGED, OnMetaChanged);

	ReloadGlobals();
	InitIcons();
}
