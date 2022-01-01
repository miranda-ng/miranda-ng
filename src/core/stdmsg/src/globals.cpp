/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

IconItem iconList[] =
{
	{ LPGEN("Incoming message (10x10)"), "INCOMING", IDI_INCOMING, 10 },
	{ LPGEN("Outgoing message (10x10)"), "OUTGOING", IDI_OUTGOING, 10 },
	{ LPGEN("Notice (10x10)"),           "NOTICE",   IDI_NOTICE,   10 },
};

static void InitIcons(void)
{
	g_plugin.registerIcon(LPGEN("Messaging"), iconList, "SRMM");
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	for (auto &si : g_chatApi.arSessions)
		if (si->pDlg)
			si->pDlg->Close();

	UninitTabs();
	return 0;
}

static int OnMetaChanged(WPARAM hMeta, LPARAM)
{
	if (hMeta) {
		auto *pDlg = Srmm_FindDialog(hMeta);
		if (pDlg != nullptr)
			pDlg->UpdateAvatar();
	}
	return 0;
}

static int dbaddedevent(WPARAM hContact, LPARAM hDbEvent)
{
	if (hContact) {
		HWND h = Srmm_FindWindow(hContact);
		if (h)
			::SendMessage(h, HM_DBEVENTADDED, hContact, hDbEvent);

		MCONTACT hEventContact = db_event_getContact(hDbEvent);
		if (hEventContact != hContact)
			if ((h = Srmm_FindWindow(hEventContact)) != nullptr)
				::SendMessage(h, HM_DBEVENTADDED, hEventContact, hDbEvent);
	}
	return 0;
}

static int ackevent(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (pAck && pAck->type == ACKTYPE_MESSAGE) {
		msgQueue_processack(pAck->hContact, (INT_PTR)pAck->hProcess, pAck->result == ACKRESULT_SUCCESS, pAck->lParam);

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

GlobalMessageData g_dat;

GlobalMessageData::GlobalMessageData() :
	bShowDate(SRMMMOD, "ShowDate", false),
	bShowTime(SRMMMOD, "ShowTime", true),
	bShowSecs(SRMMMOD, "ShowSeconds", true),
	bShowIcons(SRMMMOD, "ShowLogIcon", true),
	bShowAvatar(SRMMMOD, "AvatarEnable", true),
	bShowButtons(SRMMMOD, "ShowButtonLine", true),

	bTypingNew(SRMMMOD, "DefaultTyping", true),
	bTypingUnknown(SRMMMOD, "UnknownTyping", false),

	bShowTyping(SRMMMOD, "ShowTyping", true),
	bShowTypingWin(SRMMMOD, "ShowTypingWin", true),
	bShowTypingTray(SRMMMOD, "ShowTypingTray", false),
	bShowTypingClist(SRMMMOD, "ShowTypingClist", true),

	bCascade(SRMMMOD, "Cascade", true),
	bAutoMin(SRMMMOD, "AutoMin", false),
	bAutoClose(SRMMMOD, "AutoClose", false),
	bShowNames(SRMMMOD, "ShowNames", false),
	bShowFormat(SRMMMOD, "ShowFormatting", true),
	bSendButton(SRMMMOD, "UseSendButton", false),
	bSendOnEnter(SRMMMOD, "SendOnEnter", true),
	bCtrlSupport(SRMMMOD, "SupportCtrlUpDn", true),
	bShowReadChar(SRMMMOD, "ShowCharCount", false),
	bSendOnDblEnter(SRMMMOD, "SendOnDblEnter", false),
	bSendOnCtrlEnter(SRMMMOD, "SendOnCtrlEnter", false),
	bDeleteTempCont(SRMMMOD, "DeleteTempCont", false),
	bSavePerContact(SRMMMOD, "SavePerContact", false),
	bDoNotStealFocus(SRMMMOD, "DoNotStealFocus", false),
	bUseStatusWinIcon(SRMMMOD, "UseStatusWinIcon", true),

	bLimitAvatarHeight(SRMMMOD, "AvatarLimitHeight", true),
	iAvatarHeight(SRMMMOD, "AvatarHeight", 60),

	popupFlags(SRMMMOD, "PopupFlags", 0),
	nFlashMax(SRMMMOD, "FlashMax", 5),
	iGap(SRMSGMOD, "ButtonsBarGap", 1),

	msgTimeout(SRMMMOD, "MessageTimeout", 65000),

	iLoadHistory(SRMMMOD, "LoadHistory", LOADHISTORY_UNREAD),
	nLoadCount(SRMMMOD, "LoadCount", 10),
	nLoadTime(SRMMMOD, "LoadTime", 10)
{
}

void InitGlobals()
{
	int iOldValue = g_plugin.getByte("HideNames", -1);
	if (iOldValue != -1) {
		g_dat.bShowNames = !iOldValue;
		g_plugin.delSetting("HideNames");
	}

	if (!g_dat.bSendOnEnter && !g_dat.bSendOnDblEnter)
		g_dat.bSendOnCtrlEnter = true;

	HookEvent(ME_DB_EVENT_ADDED, dbaddedevent);
	HookEvent(ME_DB_EVENT_EDITED, dbaddedevent);
	HookEvent(ME_PROTO_ACK, ackevent);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	HookEvent(ME_MC_DEFAULTTCHANGED, OnMetaChanged);

	InitIcons();
}
