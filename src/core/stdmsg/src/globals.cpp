/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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
	for (int i = 0; i < pci->arSessions.getCount(); i++) {
		SESSION_INFO *si = pci->arSessions[i];
		if (si->pDlg)
			si->pDlg->Close();
	}

	UninitTabs();
	ImageList_Destroy(hIconsList);
	return 0;
}

static int OnMetaChanged(WPARAM hMeta, LPARAM)
{
	if (hMeta) {
		HWND hwnd = Srmm_FindWindow(hMeta);
		if (hwnd != nullptr)
			SendMessage(hwnd, DM_GETAVATAR, 0, 0);
	}
	return 0;
}

static int dbaddedevent(WPARAM hContact, LPARAM hDbEvent)
{
	if (hContact) {
		HWND h = Srmm_FindWindow(hContact);
		if (h)
			SendMessage(h, HM_DBEVENTADDED, hContact, hDbEvent);

		MCONTACT hEventContact = db_event_getContact(hDbEvent);
		if (hEventContact != hContact)
			if ((h = Srmm_FindWindow(hEventContact)) != nullptr)
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
			Skin_PlaySound("SendMsg");
	}
	return 0;
}

static int AvatarChanged(WPARAM hContact, LPARAM lParam)
{
	HWND h = Srmm_FindWindow(hContact);
	if (h)
		SendMessage(h, HM_AVATARACK, hContact, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void ReloadGlobals()
{
	memset(&g_dat, 0, sizeof(g_dat));
	g_dat.bShowDate = db_get_b(0, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE) != 0;
	g_dat.bShowTime = db_get_b(0, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME) != 0;
	g_dat.bShowSecs = db_get_b(0, SRMMMOD, SRMSGSET_SHOWSECS, SRMSGDEFSET_SHOWSECS) != 0;
	g_dat.bShowIcons = db_get_b(0, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS) != 0;
	g_dat.bShowAvatar = db_get_b(0, SRMMMOD, SRMSGSET_AVATARENABLE, SRMSGDEFSET_AVATARENABLE) != 0;
	g_dat.bShowButtons = db_get_b(0, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE) != 0;
	g_dat.bShowTyping = db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING) != 0;
	g_dat.bShowTypingWin = db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN) != 0;
	g_dat.bShowTypingTray = db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN) != 0;
	g_dat.bShowTypingClist = db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) != 0;

	g_dat.bCascade = db_get_b(0, SRMMMOD, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE) != 0;
	g_dat.bAutoMin = db_get_b(0, SRMMMOD, SRMSGSET_AUTOMIN, SRMSGDEFSET_AUTOMIN) != 0;
	g_dat.bAutoClose = db_get_b(0, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE) != 0;
	g_dat.bHideNames = db_get_b(0, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES) != 0;
	g_dat.bShowFormat = db_get_b(0, SRMMMOD, SRMSGSET_SHOWFORMAT, SRMSGDEFSET_SHOWFORMAT) != 0;
	g_dat.bSendButton = db_get_b(0, SRMMMOD, SRMSGSET_SENDBUTTON, SRMSGDEFSET_SENDBUTTON) != 0;
	g_dat.bSendOnEnter = db_get_b(0, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER) != 0;
	g_dat.bCtrlSupport = db_get_b(0, SRMMMOD, SRMSGSET_CTRLSUPPORT, SRMSGDEFSET_CTRLSUPPORT) != 0;
	g_dat.bShowReadChar = db_get_b(0, SRMMMOD, SRMSGSET_CHARCOUNT, SRMSGDEFSET_CHARCOUNT) != 0;
	g_dat.bSendOnDblEnter = db_get_b(0, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER) != 0;
	g_dat.bTypingUnknown = db_get_b(0, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN) != 0;
	g_dat.bDeleteTempCont = db_get_b(0, SRMMMOD, SRMSGSET_DELTEMP, SRMSGDEFSET_DELTEMP) != 0;
	g_dat.bSavePerContact = db_get_b(0, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT) != 0;
	g_dat.bUseStatusWinIcon = db_get_b(0, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON) != 0;
	g_dat.bDoNotStealFocus = db_get_b(0, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS) != 0;

	g_dat.openFlags = db_get_dw(0, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS);
	g_dat.nFlashMax = db_get_b(0, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
	g_dat.iGap = db_get_b(0, SRMSGMOD, SRMSGSET_BUTTONGAP, SRMSGDEFSET_BUTTONGAP);

	g_dat.msgTimeout = db_get_dw(0, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
	if (g_dat.msgTimeout < SRMSGSET_MSGTIMEOUT_MIN)
		g_dat.msgTimeout = SRMSGDEFSET_MSGTIMEOUT;
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
