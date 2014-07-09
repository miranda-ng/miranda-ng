/*

Copyright 2000-12 Miranda IM, 2012-14 Miranda NG project,
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

#include "commonheaders.h"

GlobalMessageData g_dat;

IconItem iconList[] =
{
	{ LPGEN("Incoming message (10x10)"), "INCOMING", IDI_INCOMING, 10 },
	{ LPGEN("Outgoing message (10x10)"), "OUTGOING", IDI_OUTGOING, 10 },
	{ LPGEN("Notice (10x10)"),           "NOTICE",   IDI_NOTICE,   10 },
};

static void InitIcons(void)
{
	Icon_Register(g_hInst, LPGEN("Messaging"), iconList, SIZEOF(iconList), "SRMM");
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	WindowList_Destroy(g_dat.hMessageWindowList);
	return 0;
}

static int OnMetaChanged(WPARAM hMeta, LPARAM)
{
	if (hMeta) {
		HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hMeta);
		if (hwnd != NULL)
			SendMessage(hwnd, DM_GETAVATAR, 0, 0);
	}
	return 0;
}

static int dbaddedevent(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		HWND h = WindowList_Find(g_dat.hMessageWindowList, hContact);
		if (h)
			SendMessage(h, HM_DBEVENTADDED, hContact, lParam);

		MCONTACT hEventContact = db_event_getContact((HANDLE)lParam);
		if (hEventContact != hContact)
			if ((h = WindowList_Find(g_dat.hMessageWindowList, hEventContact)) != NULL)
				SendMessage(h, HM_DBEVENTADDED, hEventContact, lParam);
	}
	return 0;
}

static int ackevent(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (pAck && pAck->type == ACKTYPE_MESSAGE) {
		msgQueue_processack(pAck->hContact, (int)pAck->hProcess, pAck->result == ACKRESULT_SUCCESS, (char*)pAck->lParam);

		if (pAck->result == ACKRESULT_SUCCESS)
			SkinPlaySound("SendMsg");
	}
	return 0;
}

int AvatarChanged(WPARAM hContact, LPARAM lParam)
{
	HWND h = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (h)
		SendMessage(h, HM_AVATARACK, hContact, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void ReloadGlobals()
{
	g_dat.flags = 0;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWINFOLINE, SRMSGDEFSET_SHOWINFOLINE))
		g_dat.flags |= SMF_SHOWINFO;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE))
		g_dat.flags |= SMF_SHOWBTNS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDBUTTON, SRMSGDEFSET_SENDBUTTON))
		g_dat.flags |= SMF_SENDBTN;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		g_dat.flags |= SMF_SHOWTYPING;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN))
		g_dat.flags |= SMF_SHOWTYPINGWIN;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN))
		g_dat.flags |= SMF_SHOWTYPINGTRAY;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST))
		g_dat.flags |= SMF_SHOWTYPINGCLIST;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS))
		g_dat.flags |= SMF_SHOWICONS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME))
		g_dat.flags |= SMF_SHOWTIME;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AVATARENABLE, SRMSGDEFSET_AVATARENABLE))
		g_dat.flags |= SMF_AVATAR;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE))
		g_dat.flags |= SMF_SHOWDATE;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWSECS, SRMSGDEFSET_SHOWSECS))
		g_dat.flags |= SMF_SHOWSECS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES))
		g_dat.flags |= SMF_HIDENAMES;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_CHARCOUNT, SRMSGDEFSET_CHARCOUNT))
		g_dat.flags |= SMF_SHOWREADCHAR;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER))
		g_dat.flags |= SMF_SENDONENTER;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER))
		g_dat.flags |= SMF_SENDONDBLENTER;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE))
		g_dat.flags |= SMF_AUTOCLOSE;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOMIN, SRMSGDEFSET_AUTOMIN))
		g_dat.flags |= SMF_AUTOMIN;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		g_dat.flags |= SMF_TYPINGUNKNOWN;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_CTRLSUPPORT, SRMSGDEFSET_CTRLSUPPORT))
		g_dat.flags |= SMF_CTRLSUPPORT;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWFORMAT, SRMSGDEFSET_SHOWFORMAT))
		g_dat.flags |= SMF_SHOWFORMAT;

	g_dat.openFlags = db_get_dw(NULL, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS);
	g_dat.nFlashMax = db_get_b(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);

	g_dat.msgTimeout = db_get_dw(NULL, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
	if (g_dat.msgTimeout < SRMSGSET_MSGTIMEOUT_MIN) g_dat.msgTimeout = SRMSGDEFSET_MSGTIMEOUT;
}

void InitGlobals()
{
	g_dat.hMessageWindowList = WindowList_Create();

	HookEvent(ME_DB_EVENT_ADDED, dbaddedevent);
	HookEvent(ME_PROTO_ACK, ackevent);
	HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);
	HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	HookEvent(ME_MC_DEFAULTTCHANGED, OnMetaChanged);

	ReloadGlobals();
	InitIcons();
}
