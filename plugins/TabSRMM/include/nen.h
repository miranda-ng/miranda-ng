/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: nen.h 13750 2011-08-03 20:10:43Z george.hazan $
 *
 * This implements the event notification module for tabSRMM. The code
 * is largely based on the NewEventNotify plugin for Miranda IM. See
 * notices below for original copyright
 *
 *  Name: NewEventNotify - Plugin for Miranda ICQ
 * 	Description: Notifies you when you receive a message
 * 	Author: icebreaker, <icebreaker@newmail.net>
 * 	Date: 18.07.02 13:59 / Update: 16.09.02 17:45
 * 	Copyright: (C) 2002 Starzinger Michael
 *
 */

#ifndef _NEN_H_
#define _NEN_H_

//#include "m_popup.h"
//#include "m_popupw.h"

#define MODULE "tabSRMM_NEN"

int tabSRMM_ShowPopup(WPARAM wParam, LPARAM lParam, WORD eventType, int windowOpen, struct TContainerData *pContainer, HWND hwndChild, const char *szProto, struct TWindowData *dat);


#define DEFAULT_COLBACK RGB(255,255,128)
#define DEFAULT_COLTEXT RGB(0,0,0)
#define DEFAULT_MASKNOTIFY (MASK_MESSAGE|MASK_URL|MASK_FILE|MASK_OTHER)
#define DEFAULT_MASKACTL (MASK_OPEN|MASK_DISMISS)
#define DEFAULT_MASKACTR (MASK_DISMISS|MASK_REMOVE)
#define DEFAULT_DELAY 0

#define MASK_MESSAGE    0x0001
#define MASK_URL        0x0002
#define MASK_FILE       0x0004
#define MASK_OTHER      0x0008

#define MASK_DISMISS    0x0001
#define MASK_OPEN       0x0002
#define MASK_REMOVE     0x0004

#define PU_REMOVE_ON_FOCUS 1
#define PU_REMOVE_ON_TYPE 2
#define PU_REMOVE_ON_SEND 4

#define SETTING_LIFETIME_MIN		1
#define SETTING_LIFETIME_MAX		60
#define SETTING_LIFETIME_DEFAULT	4

//Entrys in the database, don't translate
#define OPT_PREVIEW "Preview"
#define OPT_COLDEFAULT_MESSAGE "DefaultColorMsg"
#define OPT_COLBACK_MESSAGE "ColorBackMsg"
#define OPT_COLTEXT_MESSAGE "ColorTextMsg"
#define OPT_COLDEFAULT_OTHERS "DefaultColorOthers"
#define OPT_COLDEFAULT_ERR "DefaultColorErr"
#define OPT_COLBACK_OTHERS "ColorBackOthers"
#define OPT_COLTEXT_OTHERS "ColorTextOthers"
#define OPT_COLBACK_ERR "ColorBackErr"
#define OPT_COLTEXT_ERR "ColorTextErr"
#define OPT_MASKNOTIFY "Notify"
#define OPT_MASKACTL "ActionLeft"
#define OPT_MASKACTR "ActionRight"
#define OPT_MASKACTTE "ActionTimeExpires"
#define OPT_MERGEPOPUP "MergePopup"
#define OPT_DELAY_MESSAGE "DelayMessage"
#define OPT_DELAY_OTHERS "DelayOthers"
#define OPT_DELAY_ERR "DelayErr"
#define OPT_SHOW_HEADERS "ShowHeaders"
#define OPT_NORSS "NoRSSAnnounces"
#define OPT_DISABLE "Disabled"
#define OPT_MUCDISABLE "MUCDisabled"
#define OPT_WINDOWCHECK "WindowCheck"
#define OPT_LIMITPREVIEW "LimitPreview"
#define OPT_REMOVEMASK "removemask"

struct NEN_OPTIONS {
	BOOL bPreview;
	BOOL bDefaultColorMsg;
	BOOL bDefaultColorOthers;
	BOOL bDefaultColorErr;
	BOOL bDisableNonMessage;
	COLORREF colBackMsg;
	COLORREF colTextMsg;
	COLORREF colBackOthers;
	COLORREF colTextOthers;
	COLORREF colBackErr;
	COLORREF colTextErr;
	UINT maskActL;
	UINT maskActR;
	UINT maskActTE;
	int iDelayMsg;
	int iDelayOthers;
	int iDelayErr;
	int iDelayDefault;
	BOOL bMergePopup;
	BOOL bShowHeaders;
	BOOL bNoRSS;
	int  iDisable;
	int	 iMUCDisable;
	int  dwStatusMask;
	BOOL bTraySupport;
	BOOL bTrayExist;
	BOOL iNoSounds;
	BOOL iNoAutoPopup;
	BOOL bWindowCheck;
	int  iLimitPreview;
	WORD wMaxRecent;
	WORD wMaxFavorites;
	DWORD dwRemoveMask;
};

typedef struct {
	HANDLE hEvent;
	TCHAR szText[MAX_SECONDLINE + 2];
	DWORD timestamp;
} EVENT_DATAT;

typedef struct {
	UINT 		eventType;
	HANDLE 		hContact;
	NEN_OPTIONS *pluginOptions;
	POPUPDATAT_V2* pud;
	HWND 		hWnd;
	long 		iSeconds;
	TCHAR 		szHeader[256];
	int  		nrMerged;
	EVENT_DATAT *eventData;
	int  		nrEventsAlloced;
	int  		iActionTaken;
	HWND     hContainer;
} PLUGIN_DATAT;

#define NR_MERGED 5

#define TIMER_TO_ACTION 50685

#define MAX_DATASIZE	50
#define MAX_POPUPS 20

#endif
