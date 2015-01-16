/*
  Name: NewEventNotify - Plugin for Miranda IM
  File: neweventnotify.h - Main Header File
  Version: 0.2.2.2
  Description: Notifies you about some events
  Author: icebreaker, <icebreaker@newmail.net>
  Date: 18.07.02 13:59 / Update: 16.09.02 17:45
  Copyright: (C) 2002 Starzinger Michael

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
*/

//---------------------------
//---Includes
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stddef.h>
#include <windows.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_protosvc.h>
#include <m_message.h>
#include <m_options.h>
#include <m_icq.h>

#include <m_metacontacts.h>

#include "resource.h"
#include "Version.h"

//VERY_PUBLIC Begin ... will be moved to m_neweventnotify.h
#define MS_NEN_MENUNOTIFY "NewEventNotify/MenuitemNotifyCommand"
//VERY_PUBLIC End

//---------------------------
//---Definitions

#define MODULE "NewEventNotify"
#define MAX_POPUPS 20

#define DEFAULT_COLBACK RGB(255,255,128)
#define DEFAULT_COLTEXT RGB(0,0,0)
#define DEFAULT_MASKNOTIFY (MASK_MESSAGE|MASK_URL|MASK_FILE|MASK_OTHER)
#define DEFAULT_MASKACTL (MASK_OPEN|MASK_REMOVE|MASK_DISMISS)
#define DEFAULT_MASKACTR (MASK_REMOVE|MASK_DISMISS)
#define DEFAULT_MASKACTE (MASK_DISMISS)
#define DEFAULT_DELAY -1

#define MASK_MESSAGE    0x0001
#define MASK_URL        0x0002
#define MASK_FILE       0x0004
#define MASK_OTHER      0x0008

#define MASK_DISMISS    0x0001
#define MASK_OPEN       0x0002
#define MASK_REMOVE     0x0004

#define SETTING_LIFETIME_MIN		1
#define SETTING_LIFETIME_MAX		60
#define SETTING_LIFETIME_DEFAULT	20

#define MAX_DATASIZE	24

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL 0x020A
#endif
#define TIMER_TO_ACTION 50685

//Entries in the database, don't translate
#define OPT_DISABLE "Disabled"
#define OPT_PREVIEW "Preview"
#define OPT_MENUITEM "MenuItem"
#define OPT_COLDEFAULT_MESSAGE "DefaultColorMsg"
#define OPT_COLBACK_MESSAGE "ColorBackMsg"
#define OPT_COLTEXT_MESSAGE "ColorTextMsg"
#define OPT_COLDEFAULT_URL "DefaultColorUrl"
#define OPT_COLBACK_URL "ColorBackUrl"
#define OPT_COLTEXT_URL "ColorTextUrl"
#define OPT_COLDEFAULT_FILE "DefaultColorFile"
#define OPT_COLBACK_FILE "ColorBackFile"
#define OPT_COLTEXT_FILE "ColorTextFile"
#define OPT_COLDEFAULT_OTHERS "DefaultColorOthers"
#define OPT_COLBACK_OTHERS "ColorBackOthers"
#define OPT_COLTEXT_OTHERS "ColorTextOthers"
#define OPT_MASKNOTIFY "Notify"
#define OPT_MASKACTL "ActionLeft"
#define OPT_MASKACTR "ActionRight"
#define OPT_MASKACTTE "ActionTimeExpires"
#define OPT_MSGWINDOWCHECK "WindowCheck"
#define OPT_MSGREPLYWINDOW "ReplyWindow"
#define OPT_MERGEPOPUP "MergePopup"
#define OPT_DELAY_MESSAGE "DelayMessage"
#define OPT_DELAY_URL "DelayUrl"
#define OPT_DELAY_FILE "DelayFile"
#define OPT_DELAY_OTHERS "DelayOthers"
#define OPT_SHOW_DATE "ShowDate"
#define OPT_SHOW_TIME "ShowTime"
#define OPT_SHOW_HEADERS "ShowHeaders"
#define OPT_NUMBER_MSG "NumberMsg"
#define OPT_SHOW_ON "ShowOldOrNew"
#define OPT_HIDESEND "HideSend"
#define OPT_NORSS "NoRSSAnnounces"
#define OPT_READCHECK "ReadCheck"
//---------------------------
//---Translateable Strings

#define POPUP_COMMENT_MESSAGE LPGEN("Message")
#define POPUP_COMMENT_URL LPGEN("URL")
#define POPUP_COMMENT_FILE LPGEN("File")
#define POPUP_COMMENT_CONTACTS LPGEN("Contacts")
#define POPUP_COMMENT_ADDED LPGEN("You were added!")
#define POPUP_COMMENT_AUTH LPGEN("Requests your authorization")
#define POPUP_COMMENT_WEBPAGER LPGEN("ICQ Web pager")
#define POPUP_COMMENT_EMAILEXP LPGEN("ICQ Email express")

#define ICQEVENTTYPE_SMSCONFIRMATION 3001
#define POPUP_COMMENT_SMS LPGEN("SMS Message")
#define POPUP_COMMENT_SMSCONFIRMATION LPGEN("SMS Confirmation")

#define POPUP_COMMENT_OTHER LPGEN("Unknown Event")

#define MENUITEM_NAME LPGEN("Notify of new events")

#define MENUITEM_ENABLE LPGEN("Enable new event notification")
#define MENUITEM_DISABLE LPGEN("Disable new event notification")

//---------------------------
//---Structures

struct PLUGIN_OPTIONS
{
	HINSTANCE hInst;
	BOOL bDisable;
	BOOL bPreview;
	BOOL bMenuitem;
	BOOL bDefaultColorMsg;
	BOOL bDefaultColorUrl;
	BOOL bDefaultColorFile;
	BOOL bDefaultColorOthers;
	COLORREF colBackMsg;
	COLORREF colTextMsg;
	COLORREF colBackUrl;
	COLORREF colTextUrl;
	COLORREF colBackFile;
	COLORREF colTextFile;
	COLORREF colBackOthers;
	COLORREF colTextOthers;
	UINT maskNotify;
	UINT maskActL;
	UINT maskActR;
	UINT maskActTE;
	BOOL bMsgWindowCheck;
	BOOL bMsgReplyWindow;
	int iDelayMsg;
	int iDelayUrl;
	int iDelayFile;
	int iDelayOthers;
	int iDelayDefault;
	BOOL bMergePopup;
	BOOL bShowDate;
	BOOL bShowTime;
	BOOL bShowHeaders;
	BYTE iNumberMsg;
	BOOL bShowON;
	BOOL bHideSend;
	BOOL bNoRSS;
	BOOL bReadCheck;
};

struct EVENT_DATA_EX
{
	MEVENT hEvent;
	int number;
	struct EVENT_DATA_EX* next;
	struct EVENT_DATA_EX* prev;
};

struct PLUGIN_DATA
{
	UINT eventType;
	MCONTACT hContact;
	PLUGIN_OPTIONS* pluginOptions;
	HWND hWnd;
	struct EVENT_DATA_EX* firstEventData;
	struct EVENT_DATA_EX* firstShowEventData;
	struct EVENT_DATA_EX* lastEventData;
	long countEvent;
	long iSeconds;
	int iLock;
};

//---------------------------
//---External Procedure Definitions

int PopupShow(PLUGIN_OPTIONS* pluginOptions, MCONTACT hContact, MEVENT hEvent, UINT eventType);
int PopupUpdate(MCONTACT hContact, MEVENT hEvent);
int PopupPreview(PLUGIN_OPTIONS* pluginOptions);
int PopupAct(HWND hWnd, UINT mask, PLUGIN_DATA* pdata);
int OptionsInit(PLUGIN_OPTIONS* pluginOptions);
int OptionsAdd(HINSTANCE hInst, WPARAM addInfo);
int Opt_DisableNEN(BOOL Status);
int MenuitemInit(BOOL bStatus);
int MenuitemUpdate(BOOL bStatus);
int NumberPopupData(MCONTACT hContact, int eventType);
int CheckMsgWnd(MCONTACT hContact);

extern HINSTANCE hInst;
