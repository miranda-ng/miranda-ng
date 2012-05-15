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
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: modplus.cpp 11848 2010-05-27 14:57:22Z silvercircle $
 *
 * implements features of the tabSRMM "MADMOD" patch, developed by
 * Mad Cluster in May 2008
 *
 * the "mad mod" patch added the following features:
 *
 * ) typing sounds
 * ) support for animated avatars through ACC (avs)
 * ) a fully customizable tool bar providing services useable by external plugins
 *   to add and change buttons
 * ) toolbar on the bottom
 * ) image tag button
 * ) client icon in status bar
 * ) close tab/window on send and the "hide container feature"
 * ) bug fixes
 *
 */

#include "../src/commonheaders.h"

static  HANDLE  hEventCBButtonPressed,hEventCBInit, hEventDbOptionsInit, hEventDbPluginsLoaded;

int     g_bStartup=0;

BOOL    g_bIMGtagButton;

static char* getMirVer(HANDLE hContact)
{
	char 		*szProto = NULL;
	char		*msg = NULL;
	DBVARIANT 	dbv = {0};

	szProto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if ( !szProto )
		return (NULL);

	if ( !DBGetContactSettingString(hContact, szProto, "MirVer", &dbv) ) {
		msg=mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	return (msg);
}

static TCHAR* getMenuEntry(int i)  {
	TCHAR		*msg = NULL;
	char 		MEntry[256] = {'\0'};
	DBVARIANT 	dbv = {0};

	mir_snprintf(MEntry, 255, "MenuEntry_%u", i);
	if ( !M->GetTString(NULL, "tabmodplus",MEntry, &dbv) ) {
		msg = mir_tstrdup(dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
	return (msg);
}

int ChangeClientIconInStatusBar(const TWindowData *dat)
{
	if(!ServiceExists(MS_FP_GETCLIENTICON))
		return(S_FALSE);

	char		*msg = getMirVer(dat->hContact);

	if ( !msg )
		return (S_FALSE);

	StatusIconData sid = {0};

	sid.cbSize = sizeof(sid);
	sid.szModule = (char *)"tabmodplus";
	sid.hIcon = sid.hIconDisabled = dat->hClientIcon;
	sid.dwId = 1;
	sid.szTooltip = msg;
	sid.flags = MBF_OWNERSTATE;
	CallService(MS_MSG_MODIFYICON,(WPARAM)dat->hContact, (LPARAM)&sid);
	mir_free(msg);
	return (S_OK);
}


int ModPlus_PreShutdown(WPARAM wparam, LPARAM lparam)
{
	if ( hEventCBButtonPressed )
		UnhookEvent(hEventCBButtonPressed);
	if ( hEventCBInit )
		UnhookEvent(hEventCBInit);
	UnhookEvent(hEventDbPluginsLoaded);
	UnhookEvent(hEventDbOptionsInit);

	return (0);
}

static int RegisterCustomButton(WPARAM wParam,LPARAM lParam)
{
	if ( ServiceExists(MS_BB_ADDBUTTON) ) {
		BBButton bbd = {0};
		bbd.cbSize = sizeof(BBButton);
		bbd.bbbFlags = BBBF_ISIMBUTTON|BBBF_ISLSIDEBUTTON|BBBF_ISPUSHBUTTON;
		bbd.dwButtonID = 1;
		bbd.dwDefPos =200;
		bbd.hIcon = PluginConfig.g_buttonBarIconHandles[3];
		bbd.pszModuleName = (char *)"Tabmodplus";
		bbd.ptszTooltip = const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_BB_IMGTOOLTIP));

		return (CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd));
	}
	return (1);
}

static int CustomButtonPressed(WPARAM wParam,LPARAM lParam)
{
	CustomButtonClickData *cbcd=(CustomButtonClickData *)lParam;

	CHARRANGE cr;
	TCHAR* pszMenu[256]={0};//=NULL;
	int i=0;
	TCHAR* pszText = (TCHAR *)_T("");
	TCHAR* pszFormatedText=NULL;
	UINT textlenght=0;
	BBButton bbd={0};

	int state=0;

	if ( strcmp(cbcd->pszModule,"Tabmodplus")||cbcd->dwButtonId!=1 ) return (0);

	bbd.cbSize=sizeof(BBButton);
	bbd.dwButtonID=1;
	bbd.pszModuleName = (char *)"Tabmodplus";
	CallService(MS_BB_GETBUTTONSTATE, wParam, (LPARAM)&bbd);

	cr.cpMin = cr.cpMax = 0;
	SendDlgItemMessage(cbcd->hwndFrom,IDC_MESSAGE, EM_EXGETSEL, 0, (LPARAM)&cr);
	textlenght=cr.cpMax-cr.cpMin;
	if ( textlenght ) {
		pszText = (TCHAR *)mir_alloc((textlenght+1)*sizeof(TCHAR));
		ZeroMemory(pszText,(textlenght+1)*sizeof(TCHAR));
		SendDlgItemMessage(cbcd->hwndFrom, IDC_MESSAGE,EM_GETSELTEXT, 0, (LPARAM)pszText);
	}

	if ( cbcd->flags & BBCF_RIGHTBUTTON )
		state=1;
	else if ( textlenght )
		state = 2;
	else if ( bbd.bbbFlags & BBSF_PUSHED )
		state = 3;
	else
		state = 4;

	switch ( state ) {
		case 1:
			{
				int res=0;
				int menunum;
				int menulimit;
				HMENU hMenu=NULL;

				menulimit = M->GetByte("tabmodplus","MenuCount", 0);
				if ( menulimit ) {
					hMenu = CreatePopupMenu();
					//pszMenu=malloc(menulimit*sizeof(TCHAR*));
				} else break;
				for ( menunum=0;menunum<menulimit;menunum++ ) {
					pszMenu[menunum]=getMenuEntry(menunum);
					AppendMenu(hMenu, MF_STRING,menunum+1,  pszMenu[menunum]);
				}
				res = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, NULL);
				if ( res==0 ) break;

				pszFormatedText = (TCHAR *)mir_alloc((textlenght+lstrlen(pszMenu[res-1])+2)*sizeof(TCHAR));
				ZeroMemory(pszFormatedText,(textlenght+lstrlen(pszMenu[res-1])+2)*sizeof(TCHAR));

				mir_sntprintf(pszFormatedText,(textlenght+lstrlen(pszMenu[res-1])+2)*sizeof(TCHAR),pszMenu[res-1],pszText);

			}break;
		case 2:
			{
				pszFormatedText = (TCHAR *)mir_alloc((textlenght+12)*sizeof(TCHAR));
				ZeroMemory(pszFormatedText,(textlenght+12)*sizeof(TCHAR));

				SendDlgItemMessage(cbcd->hwndFrom, IDC_MESSAGE,EM_GETSELTEXT, 0, (LPARAM)pszText);
				mir_sntprintf(pszFormatedText,(textlenght+12)*sizeof(TCHAR),_T("[img]%s[/img]"),pszText);

				bbd.ptszTooltip=0;
				bbd.hIcon=0;
				bbd.bbbFlags=BBSF_RELEASED;
				CallService(MS_BB_SETBUTTONSTATE, wParam, (LPARAM)&bbd);
			}break;

		case 3:
			{
				pszFormatedText = (TCHAR *)mir_alloc(6*sizeof(TCHAR));
				ZeroMemory(pszFormatedText,6*sizeof(TCHAR));

				_sntprintf(pszFormatedText,6*sizeof(TCHAR),_T("%s"),_T("[img]"));

				bbd.ptszTooltip = const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_BB_IMGTOOLTIP));
				CallService(MS_BB_SETBUTTONSTATE, wParam, (LPARAM)&bbd);

			}break;
		case 4:
			{

				pszFormatedText = (TCHAR *)mir_alloc(7*sizeof(TCHAR));
				ZeroMemory(pszFormatedText,7*sizeof(TCHAR));
				_sntprintf(pszFormatedText,7*sizeof(TCHAR),_T("%s"),_T("[/img]"));

				bbd.ptszTooltip = const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_BB_IMGTOOLTIP));
				CallService(MS_BB_SETBUTTONSTATE, wParam, (LPARAM)&bbd);

			}break;
	}

	while ( pszMenu[i] ) {
		mir_free(pszMenu[i]);
		i++;
	}

	if ( pszFormatedText ) SendDlgItemMessage(cbcd->hwndFrom, IDC_MESSAGE, EM_REPLACESEL, TRUE, (LPARAM)pszFormatedText);

	if ( textlenght ) mir_free(pszText);
	if ( pszFormatedText ) mir_free(pszFormatedText);
	return (1);

}

#define MBF_OWNERSTATE        0x04

int ModPlus_Init(WPARAM wparam,LPARAM lparam)
{
	g_bStartup = 1;

	hEventCBButtonPressed=HookEvent(ME_MSG_BUTTONPRESSED,CustomButtonPressed);
	hEventCBInit=HookEvent(ME_MSG_TOOLBARLOADED,RegisterCustomButton);

	if (PluginConfig.g_bClientInStatusBar&&ServiceExists(MS_MSG_ADDICON) ) {
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = (char *)"tabmodplus";
		sid.flags = MBF_OWNERSTATE|MBF_HIDDEN;
		sid.dwId = 1;
		sid.szTooltip = 0;
		sid.hIcon = sid.hIconDisabled = 0;
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
	}
	g_bStartup = 0;
	return (0);
}
