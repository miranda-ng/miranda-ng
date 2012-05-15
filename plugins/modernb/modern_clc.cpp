// refactored step 1.
/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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

/************************************************************************/
/*       Module responsible for working with contact list control       */
/************************************************************************/

#include "hdr/modern_commonheaders.h"
#include "m_clc.h"
#include "m_skin.h"
#include "hdr/modern_commonprototypes.h"

#include "hdr/modern_clc.h"
#include "hdr/modern_clist.h"
#include "hdr/modern_clcpaint.h"

#include "m_modernopt.h"

int ModernOptInit(WPARAM wParam,LPARAM lParam);
int ModernSkinOptInit(WPARAM wParam,LPARAM lParam);

/*
*	Private module variables
*/
static HANDLE	hShowInfoTipEvent;
static HANDLE	hSettingChanged;
static POINT	HitPoint;
static BOOL		fMouseUpped;
static BYTE		IsDragToScrollMode=0;
static int		StartDragPos=0;
static int		StartScrollPos=0;
HANDLE			hSmileyAddOptionsChangedHook=NULL;
HANDLE			hIconChangedHook=NULL;
HANDLE			hAckHook=NULL;
HANDLE			hAvatarChanged=NULL;
static BOOL		g_bSortTimerIsSet = FALSE;
static struct ClcContact * hitcontact=NULL;


static int clcHookSmileyAddOptionsChanged(WPARAM wParam,LPARAM lParam);
static int clcHookIconsChanged(WPARAM wParam, LPARAM lParam);
static int clcHookBkgndConfigChanged(WPARAM wParam,LPARAM lParam);
static int clcProceedDragToScroll(HWND hwnd, int Y);
static int clcExitDragToScroll();


static int clcHookModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	int i;
	if (MirandaExiting()) return 0;

	ModernHookEvent(ME_MODERNOPT_INITIALIZE,ModernOptInit);
	ModernHookEvent(ME_MODERNOPT_INITIALIZE,ModernSkinOptInit);

	if (ServiceExists(MS_MC_DISABLEHIDDENGROUP))
		CallService(MS_MC_DISABLEHIDDENGROUP, (WPARAM)TRUE, (LPARAM)0);
	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		g_szMetaModuleName = (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	// Get icons
	if(ServiceExists(MS_SKIN2_ADDICON)) 
	{
		SKINICONDESC sid={0};
		char szMyPath[MAX_PATH];
		int i;

		GetModuleFileNameA(g_hInst, szMyPath, MAX_PATH);

		sid.cbSize = sizeof(sid);
		sid.cx=16;
		sid.cy=16;
		sid.pszDefaultFile = szMyPath;

		sid.pszSection = LPGEN("Contact List");
		sid.pszDescription = LPGEN("Listening to");
		sid.pszName = "LISTENING_TO_ICON";
		sid.iDefaultIndex = - IDI_LISTENING_TO;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszSection = LPGEN("Contact List/Avatar Overlay");

		for (i = 0 ; i < MAX_REGS(g_pAvatarOverlayIcons) ; i++)
		{
			sid.pszDescription = g_pAvatarOverlayIcons[i].description;
			sid.pszName = g_pAvatarOverlayIcons[i].name;
			sid.iDefaultIndex = - g_pAvatarOverlayIcons[i].id;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}
		sid.pszSection = LPGEN("Contact List/Status Overlay");

		for (i = 0 ; i < MAX_REGS(g_pStatusOverlayIcons) ; i++)
		{
			sid.pszDescription = g_pStatusOverlayIcons[i].description;
			sid.pszName = g_pStatusOverlayIcons[i].name;
			sid.iDefaultIndex = - g_pStatusOverlayIcons[i].id;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}

		clcHookIconsChanged(0,0);

		hIconChangedHook=ModernHookEvent(ME_SKIN2_ICONSCHANGED, clcHookIconsChanged);
	}
	else 
	{
		if (hAvatarOverlays) ImageList_Destroy(hAvatarOverlays);
		hAvatarOverlays=ImageList_Create(16,16,ILC_MASK|ILC_COLOR32,MAX_REGS(g_pAvatarOverlayIcons)*2,1);
		for (i = 0 ; i < MAX_REGS(g_pAvatarOverlayIcons) ; i++)
		{
			HICON hIcon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(g_pAvatarOverlayIcons[i].id));
			g_pAvatarOverlayIcons[i].listID = ImageList_AddIcon(hAvatarOverlays,hIcon);
			DestroyIcon_protect(hIcon);
			hIcon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(g_pStatusOverlayIcons[i].id));
			g_pStatusOverlayIcons[i].listID = ImageList_AddIcon(hAvatarOverlays,hIcon);            
			DestroyIcon_protect(hIcon);
		}	
		g_hListeningToIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_LISTENING_TO), IMAGE_ICON, 16, 16, 0);
	}

	// Register smiley category
	if (ServiceExists(MS_SMILEYADD_REGISTERCATEGORY))
	{
		SMADD_REGCAT rc;

		rc.cbSize = sizeof(rc);
		rc.name = "clist";
		rc.dispname = Translate("Contact List smileys");

		CallService(MS_SMILEYADD_REGISTERCATEGORY, 0, (LPARAM)&rc);

		hSmileyAddOptionsChangedHook=ModernHookEvent(ME_SMILEYADD_OPTIONSCHANGED,clcHookSmileyAddOptionsChanged);
	}

	CallService(MS_BACKGROUNDCONFIG_REGISTER,(WPARAM)"List Background/CLC",0);
	CallService(MS_BACKGROUNDCONFIG_REGISTER,(WPARAM)"Menu Background/Menu",0);
	CallService(MS_BACKGROUNDCONFIG_REGISTER,(WPARAM)"StatusBar Background/StatusBar",0);
	CallService(MS_BACKGROUNDCONFIG_REGISTER,(WPARAM)"Frames TitleBar BackGround/FrameTitleBar",0);


	ModernHookEvent(ME_BACKGROUNDCONFIG_CHANGED,clcHookBkgndConfigChanged);
	ModernHookEvent(ME_BACKGROUNDCONFIG_CHANGED,BgStatusBarChange);
	ModernHookEvent(ME_BACKGROUNDCONFIG_CHANGED,OnFrameTitleBarBackgroundChange);
	ModernHookEvent(ME_COLOUR_RELOAD, OnFrameTitleBarBackgroundChange);

	AniAva_UpdateOptions();
	return 0;
}

static int clcHookSmileyAddOptionsChanged(WPARAM wParam,LPARAM lParam)
{
	if (MirandaExiting()) return 0;
	pcli->pfnClcBroadcast( CLM_AUTOREBUILD,0,0);
	pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);
	return 0;
}

static int clcHookProtoAck(WPARAM wParam,LPARAM lParam)
{
	return ClcDoProtoAck((HANDLE) wParam,(ACKDATA*) lParam);
}
static int clcHookIconsChanged(WPARAM wParam, LPARAM lParam) 
{
	int i;
	if (MirandaExiting()) return 0;
	for (i = 0 ; i < MAX_REGS(g_pAvatarOverlayIcons) ; i++)
	{
		g_pAvatarOverlayIcons[i].listID=-1;
		g_pStatusOverlayIcons[i].listID=-1;
	}
	if (hAvatarOverlays) ImageList_Destroy(hAvatarOverlays);
	hAvatarOverlays=ImageList_Create(16,16,ILC_MASK|ILC_COLOR32,MAX_REGS(g_pAvatarOverlayIcons)*2,1);
	for (i = 0 ; i < MAX_REGS(g_pAvatarOverlayIcons) ; i++)
	{
		HICON hIcon=(HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)g_pAvatarOverlayIcons[i].name);
		g_pAvatarOverlayIcons[i].listID = ImageList_AddIcon(hAvatarOverlays,hIcon);
		CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)g_pAvatarOverlayIcons[i].name);

		hIcon=(HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)g_pStatusOverlayIcons[i].name);    
		g_pStatusOverlayIcons[i].listID = ImageList_AddIcon(hAvatarOverlays,hIcon);            
		CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)g_pStatusOverlayIcons[i].name);
	}

	g_hListeningToIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"LISTENING_TO_ICON");

	pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);
	AniAva_UpdateOptions();
	return 0;
}

static int clcHookSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	if (MirandaExiting()) return 0;
	if ((HANDLE)wParam==NULL)
	{
		if (g_szMetaModuleName && !mir_strcmp(cws->szModule, g_szMetaModuleName))
		{
			if (!mir_strcmp(cws->szSetting, "Enabled"))
				pcli->pfnClcBroadcast( INTM_RELOADOPTIONS, wParam, lParam);
		}
		else if (!mir_strcmp(cws->szModule, "CListGroups")) 
		{
			pcli->pfnClcBroadcast( INTM_GROUPSCHANGED,wParam,lParam);
		}
		else if (!strcmp(cws->szSetting,"XStatusId") || !strcmp(cws->szSetting,"XStatusName") )
		{
			CLUIServices_ProtocolStatusChanged(0,(LPARAM)cws->szModule);	
		}
	}
	else // (HANDLE)wParam != NULL
	{
		if (!strcmp(cws->szSetting,"TickTS"))
		{
			pcli->pfnClcBroadcast( INTM_STATUSCHANGED,wParam,0);
		}
		else if (g_szMetaModuleName && !strcmp(cws->szModule,g_szMetaModuleName))
		{ 
			if(!strcmp(cws->szSetting,"Handle"))
			{
				pcli->pfnClcBroadcast( INTM_NAMEORDERCHANGED,0,0);	
			}
			else if (!strcmp(cws->szSetting,"Default"))
			{
				pcli->pfnClcBroadcast( INTM_NAMEORDERCHANGED,0,0);	
			}
		}
		else if (!strcmp(cws->szModule,"UserInfo"))
		{
			if (!strcmp(cws->szSetting,"Timezone"))
				pcli->pfnClcBroadcast( INTM_TIMEZONECHANGED,wParam,0);	
		}
		else if (!strcmp(cws->szModule,"CList")) 
		{
			/*
			if(!strcmp(cws->szSetting,"MyHandle"))
			pcli->pfnClcBroadcast( INTM_NAMECHANGED,wParam,lParam);
			else if(!strcmp(cws->szSetting,"Group"))
			pcli->pfnClcBroadcast( INTM_GROUPCHANGED,wParam,lParam);
			else if(!strcmp(cws->szSetting,"Hidden"))
			pcli->pfnClcBroadcast( INTM_HIDDENCHANGED,wParam,lParam);
			else if(!strcmp(cws->szSetting,"noOffline"))
			pcli->pfnClcBroadcast( INTM_NAMEORDERCHANGED,wParam,lParam);
			else if(!strcmp(cws->szSetting,"NotOnList"))
			pcli->pfnClcBroadcast( INTM_NOTONLISTCHANGED,wParam,lParam);

			else if(!strcmp(cws->szSetting,"NameOrder"))
			pcli->pfnClcBroadcast( INTM_NAMEORDERCHANGED,0,0);
			else

			else if(!strcmp(cws->szSetting,"Status"))
			pcli->pfnClcBroadcast( INTM_STATUSCHANGED,wParam,0);
			else
			*/	
			if(!strcmp(cws->szSetting,"StatusMsg")) 
				pcli->pfnClcBroadcast( INTM_STATUSMSGCHANGED,wParam,0); 

		}
		else if(!strcmp(cws->szModule,"ContactPhoto")) 
		{
			if (!strcmp(cws->szSetting,"File")) 
				pcli->pfnClcBroadcast( INTM_AVATARCHANGED,wParam,0);
		}
		else //if(0) //turn off
		{
			{					
				//				if (!strcmp(cws->szSetting,"UIN"))
				//					pcli->pfnClcBroadcast( INTM_NAMECHANGED,wParam,lParam);
				//				else if (!strcmp(cws->szSetting,"Nick") || !strcmp(cws->szSetting,"FirstName") 
				//					|| !strcmp(cws->szSetting,"e-mail") || !strcmp(cws->szSetting,"LastName") 
				//					|| !strcmp(cws->szSetting,"JID"))
				//					pcli->pfnClcBroadcast( INTM_NAMECHANGED,wParam,lParam);
				//				else if (!strcmp(cws->szSetting,"ApparentMode"))
				//					pcli->pfnClcBroadcast( INTM_APPARENTMODECHANGED,wParam,lParam);
				//				else if (!strcmp(cws->szSetting,"IdleTS"))
				//					pcli->pfnClcBroadcast( INTM_IDLECHANGED,wParam,lParam);
				//else 
				if ((!strcmp(cws->szSetting,"XStatusName") || !strcmp(cws->szSetting,"XStatusMsg")))
					pcli->pfnClcBroadcast( INTM_STATUSMSGCHANGED,wParam,0);
				else if (!strcmp(cws->szSetting,"XStatusId"))
					pcli->pfnClcBroadcast( INTM_STATUSCHANGED,wParam,0);
				else if (!strcmp(cws->szSetting,"Timezone"))
					pcli->pfnClcBroadcast( INTM_TIMEZONECHANGED,wParam,0);
				else if (!strcmp(cws->szSetting,"ListeningTo"))
					pcli->pfnClcBroadcast( INTM_STATUSMSGCHANGED,wParam,0);
				else if (!strcmp(cws->szSetting,"Transport") || !strcmp(cws->szSetting,"IsTransported") )
				{
					pcli->pfnInvalidateDisplayNameCacheEntry((HANDLE)wParam);
					pcli->pfnClcBroadcast( CLM_AUTOREBUILD,wParam,0);
				}
			}
		}
	}
	return 0;
}

static int clcHookDbEventAdded(WPARAM wParam,LPARAM lParam) 
{
	DBEVENTINFO dbei = {0};
	g_CluiData.t_now = time(NULL);
	if(wParam && lParam) 
	{
		dbei.cbSize = sizeof(dbei);
		dbei.pBlob = 0;
		dbei.cbBlob = 0;
		CallService(MS_DB_EVENT_GET, (WPARAM)lParam, (LPARAM)&dbei);
		if(dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT))
		{
			PDNCE pdnce=(PDNCE)pcli->pfnGetCacheEntry((HANDLE)wParam);
			ModernWriteSettingDword((HANDLE)wParam, "CList", "mf_lastmsg", dbei.timestamp);
			if (pdnce)
				pdnce->dwLastMsgTime=dbei.timestamp;
		}
	}
	return 0;
}

static int clcHookBkgndConfigChanged(WPARAM wParam,LPARAM lParam)
{
	pcli->pfnClcOptionsChanged();
	return 0;
}


static int clcHookSystemShutdown(WPARAM wParam,LPARAM lParam)
{
	ModernUnhookEvent(hAckHook);
	ModernUnhookEvent(hSettingChanged);
	return 0;
}
static int clcHookAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if (MirandaExiting()) return 0;
	pcli->pfnClcBroadcast(INTM_AVATARCHANGED, wParam, lParam);
	return 0;
}

static int clcExitDragToScroll()
{
	if (!IsDragToScrollMode) return 0;
	IsDragToScrollMode=0;
	ReleaseCapture();
	return 1;
}


static int clcProceedDragToScroll(HWND hwnd, int Y)
{
	int pos,dy;
	if (!IsDragToScrollMode) return 0;
	if(GetCapture()!=hwnd) clcExitDragToScroll();
	dy=StartDragPos-Y;
	pos=StartScrollPos+dy;
	if (pos<0) 
		pos=0;
	SendMessage(hwnd, WM_VSCROLL,MAKEWPARAM(SB_THUMBTRACK,pos),0);
	return 1;
}



static int clcSearchNextContact(HWND hwnd, struct ClcData *dat, int index, const TCHAR *text, int prefixOk, BOOL fSearchUp)
{
	struct ClcGroup *group = &dat->list;
	int testlen = lstrlen(text);
	BOOL fReturnAsFound=FALSE;
	int	 nLastFound=-1;
	if (index==-1) fReturnAsFound=TRUE;
	group->scanIndex = 0;
	for (;;) 
	{
		if (group->scanIndex == group->cl.count) 
		{
			group = group->parent;
			if (group == NULL)
				break;
			group->scanIndex++;
			continue;
		}
		if (group->cl.items[group->scanIndex]->type != CLCIT_DIVIDER) 
		{
			if ((prefixOk && CSTR_EQUAL==CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, text, -1, group->cl.items[group->scanIndex]->szText, testlen)) ||
				(!prefixOk && !lstrcmpi(text, group->cl.items[group->scanIndex]->szText))) 
			{
				struct ClcGroup *contactGroup = group;
				int contactScanIndex = group->scanIndex;					
				int foundindex;
				for (; group; group = group->parent)
					pcli->pfnSetGroupExpand(hwnd, dat, group, 1);
				foundindex=pcli->pfnGetRowsPriorTo(&dat->list, contactGroup, contactScanIndex);
				if (fReturnAsFound) 
					return foundindex;
				else if (nLastFound!=-1 && fSearchUp && foundindex==index)
					return nLastFound;
				else if (!fSearchUp && foundindex==index)
					fReturnAsFound=TRUE;
				else
					nLastFound=foundindex;
				group=contactGroup;
				group->scanIndex=contactScanIndex;
			}
			if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) 
			{
				if (!(dat->exStyle & CLS_EX_QUICKSEARCHVISONLY) || group->cl.items[group->scanIndex]->group->expanded) 
				{
					group = group->cl.items[group->scanIndex]->group;
					group->scanIndex = 0;
					continue;
				}
			}
		}
		group->scanIndex++;
	}
	return -1;
}

static BOOL clcItemNotHiddenOffline(struct ClcData * dat, struct ClcGroup* group, struct ClcContact * contact)
{
	PDNCE pdnce;

	if (!group) return FALSE;
	if (!contact) return FALSE;
	if (group->hideOffline) return FALSE;
	if (g_CluiData.bFilterEffective) return FALSE;				

	if (CLCItems_IsShowOfflineGroup(group)) return TRUE;

	pdnce=(PDNCE)pcli->pfnGetCacheEntry( contact->hContact);
	if (!pdnce) return FALSE;
	if (pdnce->m_cache_nNoHiddenOffline) return TRUE;

	return FALSE;

}
static LRESULT clcOnCreate(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	dat=(struct ClcData*)mir_calloc(sizeof(struct ClcData));
	SetWindowLongPtr(hwnd,0,(LONG_PTR)dat);
	dat->hCheckBoxTheme=xpt_AddThemeHandle(hwnd, L"BUTTON");
	dat->m_paintCouter=0;
	dat->hWnd=hwnd;
	dat->use_avatar_service = ServiceExists(MS_AV_GETAVATARBITMAP);
	if (dat->use_avatar_service)
	{
		if (!hAvatarChanged)
			hAvatarChanged=ModernHookEvent(ME_AV_AVATARCHANGED, clcHookAvatarChanged);
	}
	//else
	//{
	ImageArray_Initialize(&dat->avatar_cache, FALSE, 20); //this array will be used to keep small avatars too
	//}

	RowHeights_Initialize(dat);

	dat->NeedResort=1;
	dat->MetaIgnoreEmptyExtra=ModernGetSettingByte(NULL,"CLC","MetaIgnoreEmptyExtra",SETTING_METAIGNOREEMPTYEXTRA_DEFAULT);

	dat->IsMetaContactsEnabled=(!(GetWindowLong(hwnd,GWL_STYLE)&CLS_MANUALUPDATE)) &&
		g_szMetaModuleName && ModernGetSettingByte(NULL,g_szMetaModuleName,"Enabled",1) && ServiceExists(MS_MC_GETDEFAULTCONTACT);

	dat->expandMeta=ModernGetSettingByte(NULL,"CLC","MetaExpanding",SETTING_METAEXPANDING_DEFAULT);		
	dat->useMetaIcon=ModernGetSettingByte(NULL,"CLC","Meta",SETTING_USEMETAICON_DEFAULT);
	dat->drawOverlayedStatus=ModernGetSettingByte(NULL,"CLC","DrawOverlayedStatus",SETTING_DRAWOVERLAYEDSTATUS_DEFAULT);
	g_CluiData.bSortByOrder[0]=ModernGetSettingByte(NULL,"CList","SortBy1",SETTING_SORTBY1_DEFAULT);
	g_CluiData.bSortByOrder[1]=ModernGetSettingByte(NULL,"CList","SortBy2",SETTING_SORTBY2_DEFAULT);
	g_CluiData.bSortByOrder[2]=ModernGetSettingByte(NULL,"CList","SortBy3",SETTING_SORTBY3_DEFAULT);
	g_CluiData.fSortNoOfflineBottom=ModernGetSettingByte(NULL,"CList","NoOfflineBottom",SETTING_NOOFFLINEBOTTOM_DEFAULT);
	dat->menuOwnerID=-1;
	dat->menuOwnerType=CLCIT_INVALID;
	//InitDisplayNameCache(&dat->lCLCContactsCache);
	//LoadCLCOptions(hwnd,dat);
	corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);	
	LoadCLCOptions(hwnd,dat);
	if ( dat->contact_time_show || dat->second_line_type == TEXT_CONTACT_TIME || dat->third_line_type == TEXT_CONTACT_TIME )
		CLUI_SafeSetTimer( hwnd, TIMERID_INVALIDATE, 5000, NULL );
	else
		KillTimer( hwnd, TIMERID_INVALIDATE );
	//if (dat->force_in_dialog)
	//	pcli->pfnRebuildEntireList(hwnd,dat);		
	TRACE("Create New ClistControl TO END\r\n");		
	return 0;
}



static LRESULT clcOnHitTest(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd,WM_NCHITTEST,wParam,lParam);
}
static LRESULT clcOnCommand(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ClcContact *contact;
	int hit = pcli->pfnGetRowByIndex(dat, dat->selection, &contact, NULL);
	if (hit == -1)	return 0;
	if ( contact->type == CLCIT_CONTACT && CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM) contact->hContact))	return 0;

	switch (LOWORD(wParam)) 
	{
	case POPUP_NEWSUBGROUP:
		if (contact->type != CLCIT_GROUP)
			return 0;
		SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~CLS_HIDEEMPTYGROUPS);
		CallService(MS_CLIST_GROUPCREATE, contact->groupId, 0);
		return 0;
	case POPUP_RENAMEGROUP:
		pcli->pfnBeginRenameSelection(hwnd, dat);
		return 0;
	case POPUP_DELETEGROUP:
		if (contact->type != CLCIT_GROUP)
			return 0;
		CallService(MS_CLIST_GROUPDELETE, contact->groupId, 0);
		return 0;
	case POPUP_GROUPSHOWOFFLINE:
		if (contact->type != CLCIT_GROUP)
			return 0;
		CallService(MS_CLIST_GROUPSETFLAGS, contact->groupId,
			MAKELPARAM(CLCItems_IsShowOfflineGroup(contact->group) ? 0 : GROUPF_SHOWOFFLINE, GROUPF_SHOWOFFLINE));
		pcli->pfnClcBroadcast(CLM_AUTOREBUILD,0, 0);
		return 0;
	case POPUP_GROUPHIDEOFFLINE:
		if (contact->type != CLCIT_GROUP)
			return 0;
		CallService(MS_CLIST_GROUPSETFLAGS, contact->groupId,
			MAKELPARAM(contact->group->hideOffline ? 0 : GROUPF_HIDEOFFLINE, GROUPF_HIDEOFFLINE));
		return 0;
	}
	if (contact->type == CLCIT_GROUP)
		if (CallService(MO_PROCESSCOMMANDBYMENUIDENT,LOWORD(wParam),(LPARAM)hwnd))
			return 0;
	return 0;
}

static LRESULT clcOnSize(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	pcli->pfnEndRename(hwnd,dat,1);
	KillTimer(hwnd,TIMERID_INFOTIP);
	KillTimer(hwnd,TIMERID_RENAME);
	cliRecalcScrollBar(hwnd,dat);
	if ( g_CluiData.fDisableSkinEngine || dat->force_in_dialog )
	{
		HBITMAP hBmp, hBmpMask, hoBmp, hoMaskBmp;
		HDC hdc,hdcMem;
		RECT rc={0};
		int depth;
		HBRUSH hBrush;

		GetClientRect(hwnd, &rc);
		if (rc.right == 0)
			return corecli.pfnContactListControlWndProc(hwnd,msg,wParam,lParam);;
		
		rc.bottom = max ( dat->row_min_heigh, 1 );
		//rc.bottom=8;
		//rc.right=8;
		hdc = GetDC(hwnd);
		depth = GetDeviceCaps(hdc, BITSPIXEL);
		if (depth < 16)
			depth = 16;
		hBmp = CreateBitmap(rc.right, rc.bottom, 1, depth, NULL);
		hBmpMask = CreateBitmap(rc.right, rc.bottom, 1, 1, NULL);
		hdcMem = CreateCompatibleDC(hdc);
		hoBmp = (HBITMAP) SelectObject(hdcMem, hBmp);
		hBrush = CreateSolidBrush( ( dat->useWindowsColours || dat->force_in_dialog ) ? GetSysColor(COLOR_HIGHLIGHT) : dat->selBkColour);
		FillRect(hdcMem, &rc, hBrush);
		DeleteObject(hBrush);

		hoMaskBmp = (HBITMAP)SelectObject(hdcMem, hBmpMask);
		FillRect(hdcMem, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		SelectObject(hdcMem, hoMaskBmp);
		SelectObject(hdcMem, hoBmp);
		DeleteDC(hdcMem);
		ReleaseDC(hwnd, hdc);
		if (dat->himlHighlight)
			ImageList_Destroy(dat->himlHighlight);
		dat->himlHighlight = ImageList_Create(rc.right, rc.bottom, (IsWinVerXPPlus()? ILC_COLOR32 : ILC_COLOR16) | ILC_MASK, 1, 1);
		ImageList_Add(dat->himlHighlight, hBmp, hBmpMask);
		DeleteObject(hBmpMask);
		DeleteObject(hBmp);
	}
	else if (dat->himlHighlight)
	{
		ImageList_Destroy(dat->himlHighlight);
		dat->himlHighlight = NULL;
	}
	return 0;
}

static LRESULT clcOnChar(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (wParam==27 && dat->szQuickSearch[0] == '\0') //escape and not quick search
	{
		// minimize clist	
		CListMod_HideWindow(pcli->hwndContactList, SW_HIDE);
	}
	return corecli.pfnContactListControlWndProc(hwnd,msg,wParam,lParam);
}
static LRESULT clcOnPaint(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	HDC hdc;
	PAINTSTRUCT ps;
	if (IsWindowVisible(hwnd)) 
	{
		if (!g_CluiData.fLayered || GetParent(hwnd)!=pcli->hwndContactList)
		{       
			hdc=BeginPaint(hwnd,&ps);
			g_clcPainter.cliPaintClc(hwnd,dat,hdc,&ps.rcPaint);
			EndPaint(hwnd,&ps);
		}
		else CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE,(WPARAM)hwnd,0);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);           
}

static LRESULT clcOnEraseBkGround(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return 1;
}

static LRESULT clcOnKeyDown(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	int selMoved=0;
	int changeGroupExpand=0;
	int pageSize;
	if (wParam==VK_CONTROL) 
		return 0;
	pcli->pfnHideInfoTip(hwnd,dat);
	
	KillTimer(hwnd,TIMERID_INFOTIP);
	KillTimer(hwnd,TIMERID_RENAME);

	if(CallService(MS_CLIST_MENUPROCESSHOTKEY,wParam,MPCF_CONTACTMENU)) 
		return 0;
	
	{	
		RECT clRect;
		GetClientRect(hwnd,&clRect);
		if (dat->max_row_height) pageSize=clRect.bottom/dat->max_row_height;
		else pageSize=0;
	}
	
	switch(wParam) 
	{
	case VK_DOWN: 
	case VK_UP:
		{
			if (dat->szQuickSearch[0]!='\0' && dat->selection!=-1) //get next contact
			{
				//get next contact
				int index=clcSearchNextContact(hwnd,dat,dat->selection,dat->szQuickSearch,1,(wParam==VK_UP));
				if (index==-1)
				{
					MessageBeep(MB_OK);
					return 0;
				}
				else
				{
					dat->selection=index;
					pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
					pcli->pfnEnsureVisible(hwnd, dat, dat->selection, 0);
					return 0;
				}

			}
			else
			{
				if (wParam==VK_DOWN) dat->selection++; 
				if (wParam==VK_UP) dat->selection--; 
				selMoved=1; break;
			}				
		}
	case VK_PRIOR: dat->selection-=pageSize; selMoved=1; break;
	case VK_NEXT: dat->selection+=pageSize; selMoved=1; break;
	case VK_HOME: dat->selection=0; selMoved=1; break;
	case VK_END: dat->selection=pcli->pfnGetGroupContentsCount(&dat->list,1)-1; selMoved=1; break;
	case VK_LEFT: changeGroupExpand=1; break;
	case VK_RIGHT: changeGroupExpand=2; break;
	case VK_RETURN: pcli->pfnDoSelectionDefaultAction(hwnd,dat); SetCapture(hwnd); return 0;
	case VK_F2: cliBeginRenameSelection(hwnd,dat); /*SetCapture(hwnd);*/ return 0;
	case VK_DELETE: pcli->pfnDeleteFromContactList(hwnd,dat); SetCapture(hwnd);return 0;
	case VK_ESCAPE: 
		{
			if((dat->dragStage&DRAGSTAGEM_STAGE)==DRAGSTAGE_ACTIVE)
			{
				dat->iDragItem=-1;
				dat->iInsertionMark=-1;
				dat->dragStage=0;
				ReleaseCapture();
			}
			return 0;
		}
	default:
		{	
			NMKEY nmkey;
			nmkey.hdr.hwndFrom=hwnd;
			nmkey.hdr.idFrom=GetDlgCtrlID(hwnd);
			nmkey.hdr.code=NM_KEYDOWN;
			nmkey.nVKey=wParam;
			nmkey.uFlags=HIWORD(lParam);
			
			if(SendMessage(GetParent(hwnd),WM_NOTIFY,0,(LPARAM)&nmkey)) 
			{
				SetCapture(hwnd); 
				return 0;
			}
		}
	}
	if(changeGroupExpand) 
	{
		int hit;
		struct ClcContact *contact;
		struct ClcGroup *group;
		dat->szQuickSearch[0]=0;
		hit=cliGetRowByIndex(dat,dat->selection,&contact,&group);
		if(hit!=-1) 
		{
			if (contact->type==CLCIT_CONTACT &&(contact->isSubcontact || contact->SubAllocated>0))
			{
				if (contact->isSubcontact && changeGroupExpand==1)
				{
					dat->selection-=contact->isSubcontact;
					selMoved=1;
				}
				else if (!contact->isSubcontact && contact->SubAllocated>0)
				{
					if (changeGroupExpand==1 && !contact->SubExpanded) 
					{
						dat->selection=cliGetRowsPriorTo(&dat->list,group,-1);
						selMoved=1;   
					}
					else if (changeGroupExpand==1 && contact->SubExpanded)
					{
						//Contract       
						struct ClcContact * ht=NULL;
						KillTimer(hwnd,TIMERID_SUBEXPAND);
						contact->SubExpanded=0;
						ModernWriteSettingByte(contact->hContact,"CList","Expanded",0);
						ht=contact;
						dat->NeedResort=1;
						pcli->pfnSortCLC(hwnd,dat,1);		
						cliRecalcScrollBar(hwnd,dat);
						hitcontact=NULL;	
					}
					else if (changeGroupExpand==2 && contact->SubExpanded)
					{
						dat->selection++;
						selMoved=1;
					}
					else if (changeGroupExpand==2 && !contact->SubExpanded && dat->expandMeta)
					{
						struct ClcContact * ht=NULL;
						KillTimer(hwnd,TIMERID_SUBEXPAND);
						contact->SubExpanded=1;
						ModernWriteSettingByte(contact->hContact,"CList","Expanded",1);
						ht=contact;
						dat->NeedResort=1;
						pcli->pfnSortCLC(hwnd,dat,1);		
						cliRecalcScrollBar(hwnd,dat);
						if (ht) 
						{
							int i=0;
							struct ClcContact *contact2;
							struct ClcGroup *group2;
							if(FindItem(hwnd,dat,contact->hContact,&contact2,&group2,NULL,FALSE))
							{
								i=cliGetRowsPriorTo(&dat->list,group2,GetContactIndex(group2,contact2));
								pcli->pfnEnsureVisible(hwnd,dat,i+contact->SubAllocated,0);
							}
						}
						hitcontact=NULL;
					}		
				}
			}

			else
			{
				if(changeGroupExpand==1 && contact->type==CLCIT_CONTACT) {
					if(group==&dat->list) {SetCapture(hwnd); return 0;}
					dat->selection=cliGetRowsPriorTo(&dat->list,group,-1);
					selMoved=1;
				}
				else {
					if(contact->type==CLCIT_GROUP)
					{
						if (changeGroupExpand==1)
						{
							if (!contact->group->expanded)
							{
								dat->selection--;
								selMoved=1;
							}
							else
							{ 
								pcli->pfnSetGroupExpand(hwnd,dat,contact->group,0);
							}
						}
						else if (changeGroupExpand==2)
						{ 
							pcli->pfnSetGroupExpand(hwnd,dat,contact->group,1);
							dat->selection++;
							selMoved=1;
						}
						else {SetCapture(hwnd);return 0;}
					}//
					//						
				}

			}
		}
		else {SetCapture(hwnd);return 0; }
	}
	if(selMoved) 
	{
		dat->szQuickSearch[0]=0;
		if(dat->selection>=pcli->pfnGetGroupContentsCount(&dat->list,1))
			dat->selection=pcli->pfnGetGroupContentsCount(&dat->list,1)-1;
		if(dat->selection<0) dat->selection=0;
		if (dat->bCompactMode)
			SendMessage(hwnd,WM_SIZE,0,0);
		CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
		pcli->pfnEnsureVisible(hwnd,dat,dat->selection,0);
		UpdateWindow(hwnd);
		SetCapture(hwnd);
		return 0;
	}
	SetCapture(hwnd);
	return 0;

}

void clcSetDelayTimer( UINT_PTR uIDEvent, HWND hwnd, int nDelay)
{
	KillTimer( hwnd, uIDEvent );
	int delay = nDelay;
	if ( delay == -1)
	{
		switch ( uIDEvent ) 
		{
		case TIMERID_DELAYEDRESORTCLC:	delay = 10;		break;
		case TIMERID_RECALCSCROLLBAR:	delay = 10;		break;
		case TIMERID_REBUILDAFTER:		delay = 50;		break;
		default:						delay = 100;	break;
		}
	}
	CLUI_SafeSetTimer( hwnd, uIDEvent, delay, NULL );	
}

static LRESULT clcOnTimer(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case TIMERID_INVALIDATE_FULL:
		{
			KillTimer(hwnd,TIMERID_INVALIDATE_FULL);
			pcli->pfnRecalcScrollBar(hwnd,dat);
			pcli->pfnInvalidateRect(hwnd,NULL,0);
			return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
		}
	case TIMERID_INVALIDATE:
		{
			time_t cur_time=(time(NULL)/60);
			if (cur_time!=dat->last_tick_time)
			{
				CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
				dat->last_tick_time=cur_time;
			}
			return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
		}
	case TIMERID_SUBEXPAND:
		{		
			struct ClcContact * ht=NULL;
			KillTimer(hwnd,TIMERID_SUBEXPAND);
			if (hitcontact && dat->expandMeta)
			{
				if (hitcontact->SubExpanded) hitcontact->SubExpanded=0; else hitcontact->SubExpanded=1;
				ModernWriteSettingByte(hitcontact->hContact,"CList","Expanded",hitcontact->SubExpanded);
				if (hitcontact->SubExpanded)
					ht=&(hitcontact->subcontacts[hitcontact->SubAllocated-1]);
			}

			dat->NeedResort=1;
			pcli->pfnSortCLC(hwnd,dat,1);		
			cliRecalcScrollBar(hwnd,dat);
			if (ht) 
			{
				int i=0;
				struct ClcContact *contact;
				struct ClcGroup *group;
				if(FindItem(hwnd,dat,hitcontact->hContact,&contact,&group,NULL,FALSE))
				{
					i=cliGetRowsPriorTo(&dat->list,group,GetContactIndex(group,contact));          
					pcli->pfnEnsureVisible(hwnd,dat,i+hitcontact->SubAllocated,0);
				}
			}
			hitcontact=NULL;
			return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
		}
	case TIMERID_DELAYEDRESORTCLC:
		{
			TRACE("Do sort on Timer\n");
			KillTimer(hwnd,TIMERID_DELAYEDRESORTCLC);			
			pcli->pfnSortCLC(hwnd,dat,1);	
			pcli->pfnInvalidateRect(hwnd,NULL,FALSE);
			return 0;
		}
	case TIMERID_RECALCSCROLLBAR:
		{
			KillTimer(hwnd,TIMERID_RECALCSCROLLBAR);
			pcli->pfnRecalcScrollBar(hwnd,dat);
			return 0;
		}
	default:
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}


static LRESULT clcOnActivate(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRACE ("clcOnActivate\n");
	if (dat->bCompactMode)
	{
		cliRecalcScrollBar(hwnd,dat);
		if ( dat->hwndRenameEdit == NULL )
			PostMessage( hwnd, WM_SIZE, 0, 0);
	}
	dat->dragStage|= DRAGSTAGEF_SKIPRENAME;
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}
static LRESULT clcOnSetCursor(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int lResult; 
	
	if (!CLUI_IsInMainWindow(hwnd)) 
		return DefWindowProc(hwnd,msg,wParam,lParam);
	
	if (g_CluiData.nBehindEdgeState>0)  
		CLUI_ShowFromBehindEdge();
	
	if (g_CluiData.bBehindEdgeSettings) 
		CLUI_UpdateTimer(0);

	lResult=CLUI_TestCursorOnBorders();     
	return lResult ? lResult : DefWindowProc(hwnd,msg,wParam,lParam);
}
static LRESULT clcOnLButtonDown(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	{   
		POINT pt;
		int k=0;
		pt.x = LOWORD(lParam); 
		pt.y = HIWORD(lParam); 
		ClientToScreen(hwnd,&pt);
		k=CLUI_SizingOnBorder(pt,0);
		if (k) 
		{         
			int io=dat->iHotTrack;
			dat->iHotTrack=0;
			if(dat->exStyle&CLS_EX_TRACKSELECT) 
			{
				pcli->pfnInvalidateItem(hwnd,dat,io);
			}
			if (k && GetCapture()==hwnd) 
			{
				SendMessage(GetParent(hwnd),WM_PARENTNOTIFY,WM_LBUTTONDOWN,lParam);
			}
			return FALSE;
		}
	}
	{	
		struct ClcContact *contact;
		struct ClcGroup *group;
		int hit;
		DWORD hitFlags;
		fMouseUpped=FALSE;			
		pcli->pfnHideInfoTip(hwnd,dat);
		KillTimer(hwnd,TIMERID_INFOTIP);
		KillTimer(hwnd,TIMERID_RENAME);
		KillTimer(hwnd,TIMERID_SUBEXPAND);

		pcli->pfnEndRename(hwnd,dat,1);
		dat->ptDragStart.x=(short)LOWORD(lParam);
		dat->ptDragStart.y=(short)HIWORD(lParam);
		dat->szQuickSearch[0]=0;
		hit=cliHitTest(hwnd,dat,(short)LOWORD(lParam),(short)HIWORD(lParam),&contact,&group,&hitFlags);
		if(GetFocus()!=hwnd) SetFocus(hwnd);
		if(hit!=-1 && !(hitFlags&CLCHT_NOWHERE)) 
		{
			if( hit==dat->selection && hitFlags&CLCHT_ONITEMLABEL && dat->exStyle&CLS_EX_EDITLABELS) 
			{
				if ( !(dat->dragStage&DRAGSTAGEF_SKIPRENAME) )
				{
					SetCapture(hwnd);
					dat->iDragItem=dat->selection;
					dat->dragStage=DRAGSTAGE_NOTMOVED|DRAGSTAGEF_MAYBERENAME;
					dat->dragAutoScrolling=0;
					return TRUE;
				}
				else
				{
					dat->dragStage&= ~DRAGSTAGEF_SKIPRENAME;
					return TRUE;
				}

			}
		}
		if(hit!=-1 && !(hitFlags&CLCHT_NOWHERE) && contact->type==CLCIT_CONTACT && contact->SubAllocated && !contact->isSubcontact)
			if(hitFlags&CLCHT_ONITEMICON && dat->expandMeta) 
			{
				BYTE doubleClickExpand=ModernGetSettingByte(NULL,"CLC","MetaDoubleClick",SETTING_METAAVOIDDBLCLICK_DEFAULT);

				hitcontact=contact;				
				HitPoint.x= (short)LOWORD(lParam);
				HitPoint.y= (short)HIWORD(lParam);
				fMouseUpped=FALSE;
				if ((GetKeyState(VK_SHIFT)&0x8000)||(GetKeyState(VK_CONTROL)&0x8000) || (GetKeyState(VK_MENU)&0x8000))
				{
					fMouseUpped=TRUE;
					hitcontact=contact;	
					KillTimer(hwnd,TIMERID_SUBEXPAND);
					CLUI_SafeSetTimer(hwnd,TIMERID_SUBEXPAND,0,NULL);
				}
			}
			else
				hitcontact=NULL;

		if(hit!=-1 && !(hitFlags&CLCHT_NOWHERE) && contact->type==CLCIT_GROUP)
			if(hitFlags&CLCHT_ONITEMICON) 
			{
				struct ClcGroup *selgroup;
				struct ClcContact *selcontact;
				dat->selection=cliGetRowByIndex(dat,dat->selection,&selcontact,&selgroup);
				pcli->pfnSetGroupExpand(hwnd,dat,contact->group,-1);
				if(dat->selection!=-1) 
				{
					dat->selection=cliGetRowsPriorTo(&dat->list,selgroup,GetContactIndex(selgroup,selcontact));
					if(dat->selection==-1) dat->selection=cliGetRowsPriorTo(&dat->list,contact->group,-1);
				}
				if (dat->bCompactMode)
				{
					SendMessage(hwnd,WM_SIZE,0,0);
				}
				else
				{
					CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
					UpdateWindow(hwnd);
				}										
				return TRUE;
			}
			if(hit!=-1 && !(hitFlags&CLCHT_NOWHERE) && hitFlags&CLCHT_ONITEMCHECK) 
			{
				NMCLISTCONTROL nm;
				contact->flags^=CONTACTF_CHECKED;
				if(contact->type==CLCIT_GROUP) pcli->pfnSetGroupChildCheckboxes(contact->group,contact->flags&CONTACTF_CHECKED);
				pcli->pfnRecalculateGroupCheckboxes(hwnd,dat);
				CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
				nm.hdr.code=CLN_CHECKCHANGED;
				nm.hdr.hwndFrom=hwnd;
				nm.hdr.idFrom=GetDlgCtrlID(hwnd);
				nm.flags=0;
				nm.hItem=ContactToItemHandle(contact,&nm.flags);
				SendMessage(GetParent(hwnd),WM_NOTIFY,0,(LPARAM)&nm);
			}
			if(!(hitFlags&(CLCHT_ONITEMICON|CLCHT_ONITEMLABEL|CLCHT_ONITEMCHECK))) 
			{
				NMCLISTCONTROL nm;
				nm.hdr.code=NM_CLICK;
				nm.hdr.hwndFrom=hwnd;
				nm.hdr.idFrom=GetDlgCtrlID(hwnd);
				nm.flags=0;
				if(hit==-1 || hitFlags&CLCHT_NOWHERE) nm.hItem=NULL;
				else nm.hItem=ContactToItemHandle(contact,&nm.flags);
				nm.iColumn=hitFlags&CLCHT_ONITEMEXTRA?HIBYTE(HIWORD(hitFlags)):-1;
				nm.pt=dat->ptDragStart;
				SendMessage(GetParent(hwnd),WM_NOTIFY,0,(LPARAM)&nm);
			}
			if(hitFlags&(CLCHT_ONITEMCHECK|CLCHT_ONITEMEXTRA))
				return FALSE;

			dat->selection=(hitFlags&CLCHT_NOWHERE)?-1:hit;
			CLUI__cliInvalidateRect(hwnd,NULL,FALSE);

			UpdateWindow(hwnd);
			if(dat->selection!=-1 && (contact->type==CLCIT_CONTACT || contact->type==CLCIT_GROUP) && !(hitFlags&(CLCHT_ONITEMEXTRA|CLCHT_ONITEMCHECK|CLCHT_NOWHERE))) 
			{
				SetCapture(hwnd);
				dat->iDragItem=dat->selection;
				dat->dragStage=DRAGSTAGE_NOTMOVED;
				dat->dragAutoScrolling=0;
			}
			if (dat->bCompactMode)
			{						
				SendMessage(hwnd,WM_SIZE,0,0);					
			}
			if(dat->selection!=-1)	pcli->pfnEnsureVisible(hwnd,dat,hit,0);
			return TRUE;
	}
}



static LRESULT clcOnCaptureChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if ((HWND)lParam!=hwnd)
	{
		if (dat->iHotTrack!=-1)
		{
			int i;
			i=dat->iHotTrack;
			dat->iHotTrack=-1;
			pcli->pfnInvalidateItem(hwnd,dat,i); 
			pcli->pfnHideInfoTip(hwnd,dat);
		}
	}
	return 0;
}

static LRESULT clcOnMouseMove(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL isOutside=FALSE;
	if (CLUI_IsInMainWindow(hwnd))
	{
		if (g_CluiData.bBehindEdgeSettings) CLUI_UpdateTimer(0);
		CLUI_TestCursorOnBorders();
	}
	if (clcProceedDragToScroll(hwnd, (short)HIWORD(lParam)))
		return 0;		

	if ( dat->dragStage&DRAGSTAGEF_MAYBERENAME )
	{
		POINT pt;
		pt.x= (short)LOWORD(lParam);
		pt.y= (short)HIWORD(lParam);
		if (  abs(pt.x-dat->ptDragStart.x)>GetSystemMetrics(SM_CXDOUBLECLK) 
			||abs(pt.y-dat->ptDragStart.y)>GetSystemMetrics(SM_CYDOUBLECLK) )
		{
			KillTimer( hwnd, TIMERID_RENAME );
			dat->dragStage&=(~DRAGSTAGEF_MAYBERENAME);
		}
	}

	if (dat->iDragItem==-1)
	{
		POINT pt;
		HWND window;
		pt.x= (short)LOWORD(lParam);
		pt.y= (short)HIWORD(lParam);
		ClientToScreen(hwnd,&pt);
		window=WindowFromPoint(pt);
		if (window!=hwnd) isOutside=TRUE;
	}

	if(hitcontact!=NULL)
	{
		int x,y,xm,ym;
		x= (short)LOWORD(lParam);
		y= (short)HIWORD(lParam);
		xm=GetSystemMetrics(SM_CXDOUBLECLK);
		ym=GetSystemMetrics(SM_CYDOUBLECLK);
		if(abs(HitPoint.x-x)>xm || abs(HitPoint.y-y)>ym)
		{
			if (fMouseUpped)
			{
				KillTimer(hwnd,TIMERID_SUBEXPAND);
				CLUI_SafeSetTimer(hwnd,TIMERID_SUBEXPAND,0,NULL);
				fMouseUpped=FALSE;
			}
			else                                                   
			{
				KillTimer(hwnd,TIMERID_SUBEXPAND);
				hitcontact=NULL;
				fMouseUpped=FALSE;
			}
		}
	}


	if(dat->iDragItem==-1) 
	{
		DWORD flag=0;
		int iOldHotTrack=dat->iHotTrack;
		
		if(dat->hwndRenameEdit!=NULL || GetKeyState(VK_MENU)&0x8000 || GetKeyState(VK_F10)&0x8000)
			return 0;
		
		dat->iHotTrack= isOutside ? -1 : cliHitTest(hwnd,dat,(short)LOWORD(lParam),(short)HIWORD(lParam),NULL,NULL,&flag);

		if (flag&CLCHT_NOWHERE) 
			dat->iHotTrack=-1;
		
		if(iOldHotTrack!=dat->iHotTrack || isOutside) 
		{
			if(iOldHotTrack==-1 && !isOutside) 
				SetCapture(hwnd);
		
			if (dat->iHotTrack==-1 || isOutside)
				ReleaseCapture();

			if(dat->exStyle&CLS_EX_TRACKSELECT) 
			{
				pcli->pfnInvalidateItem(hwnd,dat,iOldHotTrack);
				pcli->pfnInvalidateItem(hwnd,dat,dat->iHotTrack);
			}
			
			pcli->pfnHideInfoTip(hwnd,dat);
		}

		KillTimer(hwnd,TIMERID_INFOTIP);

		if(wParam==0 && dat->hInfoTipItem==NULL) 
		{
			dat->ptInfoTip.x=(short)LOWORD(lParam);
			dat->ptInfoTip.y=(short)HIWORD(lParam);
			CLUI_SafeSetTimer(hwnd,TIMERID_INFOTIP,dat->infoTipTimeout,NULL);
		}
		return 0;
	}
	if((dat->dragStage&DRAGSTAGEM_STAGE)==DRAGSTAGE_NOTMOVED && !(dat->exStyle&CLS_EX_DISABLEDRAGDROP)) 
	{
		if(abs((short)LOWORD(lParam)-dat->ptDragStart.x)>=GetSystemMetrics(SM_CXDRAG) || abs((short)HIWORD(lParam)-dat->ptDragStart.y)>=GetSystemMetrics(SM_CYDRAG))
			dat->dragStage=(dat->dragStage&~DRAGSTAGEM_STAGE)|DRAGSTAGE_ACTIVE;
	}

	if((dat->dragStage&DRAGSTAGEM_STAGE)==DRAGSTAGE_ACTIVE) 
	{
		HCURSOR hNewCursor;
		RECT clRect;
		POINT pt;
		int target;

		GetClientRect(hwnd,&clRect);
		pt.x=(short)LOWORD(lParam); pt.y=(short)HIWORD(lParam);
		hNewCursor=LoadCursor(NULL, IDC_NO);
		CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
		if(dat->dragAutoScrolling)
		{KillTimer(hwnd,TIMERID_DRAGAUTOSCROLL); dat->dragAutoScrolling=0;}
		target=GetDropTargetInformation(hwnd,dat,pt);
		if(dat->dragStage&DRAGSTAGEF_OUTSIDE && target!=DROPTARGET_OUTSIDE) 
		{
			NMCLISTCONTROL nm;
			struct ClcContact *contact;
			cliGetRowByIndex(dat,dat->iDragItem,&contact,NULL);
			nm.hdr.code=CLN_DRAGSTOP;
			nm.hdr.hwndFrom=hwnd;
			nm.hdr.idFrom=GetDlgCtrlID(hwnd);
			nm.flags=0;
			nm.hItem=ContactToItemHandle(contact,&nm.flags);
			SendMessage(GetParent(hwnd),WM_NOTIFY,0,(LPARAM)&nm);
			dat->dragStage&=~DRAGSTAGEF_OUTSIDE;
		}
		switch(target) 
		{
			case DROPTARGET_ONSELF:
				break;
			
			case DROPTARGET_ONCONTACT:
				if (ServiceExists(MS_MC_ADDTOMETA))
				{
					struct ClcContact *contSour;
					cliGetRowByIndex(dat,dat->iDragItem,&contSour,NULL);
					if (contSour->type==CLCIT_CONTACT && g_szMetaModuleName && mir_strcmp(contSour->proto,g_szMetaModuleName))
					{
						if (!contSour->isSubcontact)
							hNewCursor=LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DROPUSER));  /// Add to meta
						else
							hNewCursor=LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_DROPMETA));
					}

				}
				break;
			
			case DROPTARGET_ONMETACONTACT:
				if (ServiceExists(MS_MC_ADDTOMETA)) 
				{
					struct ClcContact *contSour,*contDest;
					cliGetRowByIndex(dat,dat->selection,&contDest,NULL);  
					cliGetRowByIndex(dat,dat->iDragItem,&contSour,NULL);
					if (contSour->type==CLCIT_CONTACT && g_szMetaModuleName && mir_strcmp(contSour->proto,g_szMetaModuleName))
					{
						if (!contSour->isSubcontact)
							hNewCursor=LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DROPUSER));  /// Add to meta
						else
							if  (contSour->subcontacts==contDest)
								hNewCursor=LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_DEFAULTSUB)); ///MakeDefault
							else 
								hNewCursor=LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_REGROUP));
					}
				}
				break;
			case DROPTARGET_ONSUBCONTACT:
				if (ServiceExists(MS_MC_ADDTOMETA))
				{
					struct ClcContact *contSour,*contDest;
					cliGetRowByIndex(dat,dat->selection,&contDest,NULL);  
					cliGetRowByIndex(dat,dat->iDragItem,&contSour,NULL);
					if (contSour->type==CLCIT_CONTACT && g_szMetaModuleName && mir_strcmp(contSour->proto,g_szMetaModuleName))
					{
						if (!contSour->isSubcontact)
							hNewCursor=LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DROPUSER));  /// Add to meta
						else
							if (contDest->subcontacts==contSour->subcontacts)
								break;
							else  
								hNewCursor=LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_REGROUP));
					}
				}
				break;

			case DROPTARGET_ONGROUP:
				hNewCursor=LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DROPUSER));
				break;

			case DROPTARGET_INSERTION:
				hNewCursor=LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DROP));
				break;

			case DROPTARGET_OUTSIDE:
				{
					NMCLISTCONTROL nm;
					struct ClcContact *contact;

					if(pt.x>=0 && pt.x<clRect.right && ((pt.y<0 && pt.y>-dat->dragAutoScrollHeight) || (pt.y>=clRect.bottom && pt.y<clRect.bottom+dat->dragAutoScrollHeight))) 
					{
						if(!dat->dragAutoScrolling) 
						{
							if(pt.y<0) dat->dragAutoScrolling=-1;
							else dat->dragAutoScrolling=1;
							CLUI_SafeSetTimer(hwnd,TIMERID_DRAGAUTOSCROLL,dat->scrollTime,NULL);
						}
						SendMessage(hwnd,WM_TIMER,TIMERID_DRAGAUTOSCROLL,0);
					}

					dat->dragStage|=DRAGSTAGEF_OUTSIDE;
					cliGetRowByIndex(dat,dat->iDragItem,&contact,NULL);
					nm.hdr.code=CLN_DRAGGING;
					nm.hdr.hwndFrom=hwnd;
					nm.hdr.idFrom=GetDlgCtrlID(hwnd);
					nm.flags=0;
					nm.hItem=ContactToItemHandle(contact,&nm.flags);
					nm.pt=pt;
					if(SendMessage(GetParent(hwnd),WM_NOTIFY,0,(LPARAM)&nm))
						return 0;
					break;
				}
			default:
				{	
					struct ClcGroup *group=NULL;
					cliGetRowByIndex(dat,dat->iDragItem,NULL,&group);
					if(group && group->parent) 
					{
						struct ClcContact *contSour;
						cliGetRowByIndex(dat,dat->iDragItem,&contSour,NULL);
						if (!contSour->isSubcontact)
							hNewCursor=LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DROPUSER));
					}
					break;
				}
		}
		SetCursor(hNewCursor);
	}
	return 0;
}
static LRESULT clcOnLButtonUp(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{    
	if (clcExitDragToScroll()) 
		return 0;

	fMouseUpped=TRUE;

	if (hitcontact!=NULL && dat->expandMeta)
	{ 
		BYTE doubleClickExpand=ModernGetSettingByte(NULL,"CLC","MetaDoubleClick",SETTING_METAAVOIDDBLCLICK_DEFAULT);
		CLUI_SafeSetTimer(hwnd,TIMERID_SUBEXPAND,GetDoubleClickTime()*doubleClickExpand,NULL);
	}
	else if (dat->iHotTrack==-1 && dat->iDragItem==-1)
		ReleaseCapture();
	if(dat->iDragItem==-1) return 0;       
	SetCursor((HCURSOR)GetClassLongPtr(hwnd,GCLP_HCURSOR));
	if(dat->exStyle&CLS_EX_TRACKSELECT) 
	{
		DWORD flags;
		dat->iHotTrack=cliHitTest(hwnd,dat,(short)LOWORD(lParam),(short)HIWORD(lParam),NULL,NULL,&flags);
		if(dat->iHotTrack==-1) 
			ReleaseCapture();
	}
	else if (hitcontact==NULL) 
		ReleaseCapture();
	KillTimer(hwnd,TIMERID_DRAGAUTOSCROLL);
	if(dat->dragStage==(DRAGSTAGE_NOTMOVED|DRAGSTAGEF_MAYBERENAME))
		CLUI_SafeSetTimer(hwnd,TIMERID_RENAME,GetDoubleClickTime(),NULL);
	else if((dat->dragStage&DRAGSTAGEM_STAGE)==DRAGSTAGE_ACTIVE) 
	{
		POINT pt;
		int target;         
		TCHAR Wording[500];
		int res=0;
		pt.x=(short)LOWORD(lParam); pt.y=(short)HIWORD(lParam);
		target=GetDropTargetInformation(hwnd,dat,pt);
		switch(target) 
		{
		case DROPTARGET_ONSELF:
			break;
		case DROPTARGET_ONCONTACT:
			if (ServiceExists(MS_MC_ADDTOMETA))
			{
				struct ClcContact *contDest, *contSour;
				int res;
				HANDLE handle,hcontact;

				cliGetRowByIndex(dat,dat->iDragItem,&contSour,NULL);
				cliGetRowByIndex(dat,dat->selection,&contDest,NULL);
				hcontact=contSour->hContact;
				if (contSour->type==CLCIT_CONTACT)
				{

					if (g_szMetaModuleName && mir_strcmp(contSour->proto,g_szMetaModuleName))
					{
						if (!contSour->isSubcontact)
						{
							HANDLE hDest=contDest->hContact;
							mir_sntprintf(Wording,SIZEOF(Wording),TranslateT("Do You want contact '%s' to be converted to MetaContact and '%s' be added to it?"),contDest->szText, contSour->szText);
							res=MessageBox(hwnd,Wording,TranslateT("Converting to MetaContact"),MB_OKCANCEL|MB_ICONQUESTION);
							if (res==1)
							{
								handle=(HANDLE)CallService(MS_MC_CONVERTTOMETA,(WPARAM)hDest,0);
								if(!handle) return 0;
								CallService(MS_MC_ADDTOMETA,(WPARAM)hcontact,(LPARAM)handle);                            
							}
						}
						else
						{
							HANDLE handle,hcontact,hfrom,hdest;
							hcontact=contSour->hContact;
							hfrom=contSour->subcontacts->hContact;
							hdest=contDest->hContact;
							mir_sntprintf(Wording,SIZEOF(Wording),TranslateT("Do You want contact '%s' to be converted to MetaContact and '%s' be added to it (remove it from '%s')?"), contDest->szText,contSour->szText,contSour->subcontacts->szText);
							res=MessageBox(hwnd,Wording,TranslateT("Converting to MetaContact (Moving)"),MB_OKCANCEL|MB_ICONQUESTION);
							if (res==1)
							{

								handle=(HANDLE)CallService(MS_MC_CONVERTTOMETA,(WPARAM)hdest,0);
								if(!handle) return 0;

								CallService(MS_MC_REMOVEFROMMETA,(WPARAM)0,(LPARAM)hcontact);                            
								CallService(MS_MC_ADDTOMETA,(WPARAM)hcontact,(LPARAM)handle);                            
							}
						}
					}

				}
			}
			break;
		case DROPTARGET_ONMETACONTACT:
			{
				struct ClcContact *contDest, *contSour;
				int res;
				cliGetRowByIndex(dat,dat->iDragItem,&contSour,NULL);
				cliGetRowByIndex(dat,dat->selection,&contDest,NULL);  
				if (contSour->type==CLCIT_CONTACT)
				{

					if (g_szMetaModuleName && strcmp(contSour->proto,g_szMetaModuleName))
					{
						if (!contSour->isSubcontact)
						{   
							HANDLE handle,hcontact;
							hcontact=contSour->hContact;
							handle=contDest->hContact;
							mir_sntprintf(Wording,SIZEOF(Wording),TranslateT("Do you want to contact '%s' be added to metacontact '%s'?"),contSour->szText, contDest->szText);
							res=MessageBox(hwnd,Wording,TranslateT("Adding contact to MetaContact"),MB_OKCANCEL|MB_ICONQUESTION);
							if (res==1)
							{

								if(!handle) return 0;                   
								CallService(MS_MC_ADDTOMETA,(WPARAM)hcontact,(LPARAM)handle);                            
							}
						}
						else
						{
							if (contSour->subcontacts==contDest)
							{   
								HANDLE hsour;
								hsour=contSour->hContact;
								mir_sntprintf(Wording,SIZEOF(Wording),TranslateT("Do You want contact '%s' to be default ?"),contSour->szText);
								res=MessageBox(hwnd,Wording,TranslateT("Set default contact"),MB_OKCANCEL|MB_ICONQUESTION);

								if (res==1)
								{
									CallService(MS_MC_SETDEFAULTCONTACT,(WPARAM)contDest->hContact,(LPARAM)hsour);
								}
							}
							else
							{   
								HANDLE handle,hcontact,hfrom;
								hcontact=contSour->hContact;
								hfrom=contSour->subcontacts->hContact;
								handle=contDest->hContact;
								mir_sntprintf(Wording,SIZEOF(Wording),TranslateT("Do You want contact '%s' to be removed from MetaContact '%s' and added to '%s'?"), contSour->szText,contSour->subcontacts->szText,contDest->szText);
								res=MessageBox(hwnd,Wording,TranslateT("Changing MetaContacts (Moving)"),MB_OKCANCEL|MB_ICONQUESTION);
								if (res==1)
								{

									if(!handle) return 0;

									CallService(MS_MC_REMOVEFROMMETA,(WPARAM)0,(LPARAM)hcontact);                            
									CallService(MS_MC_ADDTOMETA,(WPARAM)hcontact,(LPARAM)handle);                            
								}
							}
						}
					}
				}
			}
			break;
		case DROPTARGET_ONSUBCONTACT:
			{
				struct ClcContact *contDest, *contSour;
				int res;
				cliGetRowByIndex(dat,dat->iDragItem,&contSour,NULL);
				cliGetRowByIndex(dat,dat->selection,&contDest,NULL);  
				if (contSour->type==CLCIT_CONTACT)
				{
					if (g_szMetaModuleName && strcmp(contSour->proto,g_szMetaModuleName))
					{
						if (!contSour->isSubcontact)
						{
							HANDLE handle,hcontact;
							hcontact=contSour->hContact;
							handle=contDest->subcontacts->hContact;
							mir_sntprintf(Wording,SIZEOF(Wording),TranslateT("Do You want contact '%s' to be added to MetaContact '%s'?"), contSour->szText,contDest->subcontacts->szText);
							res=MessageBox(hwnd,Wording,TranslateT("Changing MetaContacts (Moving)"),MB_OKCANCEL|MB_ICONQUESTION);
							if (res==1)
							{

								if(!handle) return 0;                   
								CallService(MS_MC_ADDTOMETA,(WPARAM)hcontact,(LPARAM)handle);                            
							}
						}
						else
						{
							if (contSour->subcontacts!=contDest->subcontacts)
							{    
								HANDLE handle,hcontact,hfrom;
								hcontact=contSour->hContact;
								hfrom=contSour->subcontacts->hContact;
								handle=contDest->subcontacts->hContact;                                     
								mir_sntprintf(Wording,SIZEOF(Wording),TranslateT("Do You want contact '%s' to be removed from MetaContact '%s' and added to '%s'?"), contSour->szText,contSour->subcontacts->szText,contDest->subcontacts->szText);
								res=MessageBox(hwnd,Wording,TranslateT("Changing MetaContacts (Moving)"),MB_OKCANCEL|MB_ICONQUESTION);
								if (res==1)
								{

									if(!handle) return 0;

									CallService(MS_MC_REMOVEFROMMETA,(WPARAM)0,(LPARAM)hcontact);                            
									CallService(MS_MC_ADDTOMETA,(WPARAM)hcontact,(LPARAM)handle); 
								}
							}
						}
					}
				}
			}
			break;
		case DROPTARGET_ONGROUP:
			corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
			break;
		case DROPTARGET_INSERTION:
			{
				struct ClcContact *contact, *destcontact;
				struct ClcGroup *group, *destgroup;
				BOOL NeedRename=FALSE;
				TCHAR newName[128]={0};
				int newIndex,i;
				pcli->pfnGetRowByIndex(dat, dat->iDragItem, &contact, &group);
				i=pcli->pfnGetRowByIndex(dat, dat->iInsertionMark, &destcontact, &destgroup);				
				if (i!=-1 && group->groupId!=destgroup->groupId)
				{
					TCHAR * groupName=mir_tstrdup(pcli->pfnGetGroupName(contact->groupId,0));
					TCHAR * shortGroup=NULL;
					TCHAR * sourceGrName=mir_tstrdup(pcli->pfnGetGroupName(destgroup->groupId,0));
					if (groupName)
					{
						int len=(int)_tcslen(groupName);
						do {len--;}while(len>=0 && groupName[len]!='\\');
						if (len>=0) shortGroup=groupName+len+1;
						else shortGroup=groupName;
					}
					if (shortGroup) 
					{
						NeedRename=TRUE;
						if (sourceGrName)
							mir_sntprintf(newName,SIZEOF(newName),_T("%s\\%s"),sourceGrName,shortGroup);
						else
							mir_sntprintf(newName,SIZEOF(newName),_T("%s"),shortGroup);
					}
					if (groupName) mir_free_and_nill(groupName);
					if (sourceGrName) mir_free_and_nill(sourceGrName);
				}
				newIndex=CallService(MS_CLIST_GROUPMOVEBEFORE, contact->groupId, (destcontact&&i!=-1)?destcontact->groupId:0);							
				newIndex=newIndex?newIndex:contact->groupId;
				if (NeedRename) pcli->pfnRenameGroup(newIndex,newName);
				break;
			}
		case DROPTARGET_OUTSIDE:
			corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
			break;
		default:
			corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
			break;

		}
	}

	CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
	dat->iDragItem=-1;
	dat->iInsertionMark=-1;
	return 0;
}
static LRESULT clcOnLButtonDblClick(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hwnd,TIMERID_SUBEXPAND);
	hitcontact=NULL;
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}
static LRESULT clcOnDestroy(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i=0;

	for(i=0;i<=FONTID_MODERN_MAX;i++) 
	{
		if(dat->fontModernInfo[i].hFont) DeleteObject(dat->fontModernInfo[i].hFont);
		dat->fontModernInfo[i].hFont=NULL;
	}
	if (dat->hMenuBackground)
	{
		DeleteObject(dat->hMenuBackground);
		dat->hMenuBackground=NULL;
	}
	if (!dat->bkChanged && dat->hBmpBackground)
	{
		DeleteObject(dat->hBmpBackground);
		dat->hBmpBackground=NULL;
	}

	ImageArray_Clear(&dat->avatar_cache);
	mod_DeleteDC(dat->avatar_cache.hdc);			
	ImageArray_Free(&dat->avatar_cache, FALSE);	
	if (dat->himlHighlight)
		ImageList_Destroy(dat->himlHighlight);

	RowHeights_Free(dat);
	corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);			
	xpt_FreeThemeForWindow(hwnd);
	return 0;
}
static LRESULT clcOnIntmGroupChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ClcContact *contact;
	BYTE iExtraImage[MAXEXTRACOLUMNS];
	WORD iWideExtraImage[MAXEXTRACOLUMNS];
	BYTE flags = 0;
	if (!pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, NULL, NULL))
	{
		memset(iExtraImage, 0xFF, SIZEOF(iExtraImage));
		memset((void*)iWideExtraImage, 0xFF, sizeof(iWideExtraImage));
	}
	else {
		CopyMemory(iExtraImage, contact->iExtraImage, SIZEOF(iExtraImage));
		CopyMemory((void*)iWideExtraImage, (void*)contact->iWideExtraImage, sizeof(iWideExtraImage));
		flags = contact->flags;
	}
	pcli->pfnDeleteItemFromTree(hwnd, (HANDLE) wParam);
	if (GetWindowLong(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN || !ModernGetSettingByte((HANDLE) wParam, "CList", "Hidden", 0)) {
		NMCLISTCONTROL nm;
		pcli->pfnAddContactToTree(hwnd, dat, (HANDLE) wParam, 1, 1);
		if (pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, NULL, NULL)) {
			CopyMemory(contact->iExtraImage, iExtraImage, SIZEOF(iExtraImage));
			CopyMemory((void*)contact->iWideExtraImage, (void*)iWideExtraImage, sizeof(iWideExtraImage));
			if(flags & CONTACTF_CHECKED)
				contact->flags |= CONTACTF_CHECKED;
		}
		nm.hdr.code = CLN_CONTACTMOVED;
		nm.hdr.hwndFrom = hwnd;
		nm.hdr.idFrom = GetDlgCtrlID(hwnd);
		nm.flags = 0;
		nm.hItem = (HANDLE) wParam;
		SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
		dat->NeedResort = 1;
	}
	SetTimer(hwnd,TIMERID_REBUILDAFTER,1,NULL);
	return 0;
}
static LRESULT clcOnIntmIconChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ClcContact *contact = NULL;
	struct ClcGroup *group = NULL;
	int recalcScrollBar = 0, shouldShow;
	BOOL needRepaint=FALSE;
	WORD status;
	char *szProto;
	int nHiddenStatus=0;
	BOOL image_is_special=FALSE;
	RECT iconRect={0};
	int contacticon=CallService(MS_CLIST_GETCONTACTICON, wParam, 1);
	HANDLE hSelItem = NULL;
	struct ClcContact *selcontact = NULL;

	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (szProto == NULL)
		status = ID_STATUS_OFFLINE;
	else
		status=GetContactCachedStatus((HANDLE) wParam);
	image_is_special=(LOWORD(contacticon) != (LOWORD(lParam))); //check only base icons

	nHiddenStatus=CLVM_GetContactHiddenStatus((HANDLE)wParam, szProto, dat);
    
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    bool isVisiblebyFilter  = ( ( ( style & CLS_SHOWHIDDEN ) && nHiddenStatus != -1 ) || !nHiddenStatus );
    bool ifVisibleByClui    = !pcli->pfnIsHiddenMode( dat, status );      
    bool isVisible          = g_CluiData.bFilterEffective&CLVM_FILTER_STATUS ? TRUE : ifVisibleByClui;
    bool isIconChanged      = CallService(MS_CLIST_GETCONTACTICON, wParam, 0) != LOWORD(lParam);
	
    shouldShow              = isVisiblebyFilter	&&  ( isVisible || isIconChanged ) ;  
    
    // XXX CLVM changed - this means an offline msg is flashing, so the contact should be shown
	
    if ( !pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, &group, NULL)) 
	{
		if (shouldShow && CallService(MS_DB_CONTACT_IS, wParam, 0)) 
		{
			if (dat->selection >= 0 && pcli->pfnGetRowByIndex(dat, dat->selection, &selcontact, NULL) != -1)
				hSelItem = pcli->pfnContactToHItem(selcontact);
			pcli->pfnAddContactToTree(hwnd, dat, (HANDLE) wParam, (style & CLS_CONTACTLIST) == 0, 0);
			recalcScrollBar = 1;
			needRepaint=TRUE;
			pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, NULL, NULL);
			if (contact) 
			{
				contact->iImage = lParam;
				contact->image_is_special=image_is_special;
				pcli->pfnNotifyNewContact(hwnd, (HANDLE) wParam);
				dat->NeedResort = 1;
			}
		}
	}
	else 
	{
		//item in list already
		if (contact->iImage == lParam)
			return 0;
		if ( !shouldShow && !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE) && clcItemNotHiddenOffline(dat, group, contact))
		{
			shouldShow=TRUE;
		}
		if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && ((style & CLS_HIDEOFFLINE) || group->hideOffline || g_CluiData.bFilterEffective) ) // CLVM changed
		{
			if (dat->selection >= 0 && pcli->pfnGetRowByIndex(dat, dat->selection, &selcontact, NULL) != -1)
				hSelItem = pcli->pfnContactToHItem(selcontact);
			pcli->pfnRemoveItemFromGroup(hwnd, group, contact, (style & CLS_CONTACTLIST) == 0);
			needRepaint=TRUE;
			recalcScrollBar = 1;
			dat->NeedResort = 1;
		}
		else if (contact)
		{
			contact->iImage = lParam;
			if (!pcli->pfnIsHiddenMode(dat, status))
				contact->flags |= CONTACTF_ONLINE;
			else
				contact->flags &= ~CONTACTF_ONLINE;
			contact->image_is_special=image_is_special;
			if (!image_is_special) //Only if it is status changing
			{
				dat->NeedResort = 1; 
				needRepaint=TRUE; 
			}
			else if (dat->m_paintCouter==contact->lastPaintCounter) //if contacts is visible
			{
				needRepaint=TRUE; 
			}
		}

	}
	if (hSelItem) {
		struct ClcGroup *selgroup;
		if (pcli->pfnFindItem(hwnd, dat, hSelItem, &selcontact, &selgroup, NULL))
			dat->selection = pcli->pfnGetRowsPriorTo(&dat->list, selgroup, li.List_IndexOf(( SortedList* )&selgroup->cl, selcontact));
		else
			dat->selection = -1;
	}
	//        dat->NeedResort = 1; 
	//        SortClcByTimer(hwnd);
	if (dat->NeedResort)
	{
		TRACE("Sort required\n");
		clcSetDelayTimer( TIMERID_DELAYEDRESORTCLC, hwnd );
	}
	else if (needRepaint) 
	{
		if (contact && contact->pos_icon.bottom!=0 && contact->pos_icon.right!=0)
			CLUI__cliInvalidateRect(hwnd,&(contact->pos_icon),FALSE);
		else
			CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
		//try only needed rectangle
	}

	return 0;
}

static LRESULT clcOnIntmAvatarChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ClcContact *contact;
	if (FindItem(hwnd,dat,(HANDLE)wParam,&contact,NULL,NULL,FALSE)) 
	{
		Cache_GetAvatar(dat, contact); 
	}
	else if (dat->use_avatar_service && !wParam)
	{
		UpdateAllAvatars(dat);
	}
	CLUI__cliInvalidateRect(hwnd, NULL, FALSE);
	return 0;
}

static LRESULT clcOnIntmTimeZoneChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ClcContact *contact;
	if(!FindItem(hwnd,dat,(HANDLE)wParam,&contact,NULL,NULL,FALSE)) 
		return corecli.pfnContactListControlWndProc(hwnd,msg,wParam,lParam);

	if (contact) //!IsBadWritePtr(contact, sizeof(struct ClcContact)))
	{
		Cache_GetTimezone(dat,contact->hContact);
		Cache_GetText(dat, contact,1);
		cliRecalcScrollBar(hwnd,dat);
	}
	return 0;
}
static LRESULT clcOnIntmNameChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ClcContact *contact;
	int ret=corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

	pcli->pfnInvalidateDisplayNameCacheEntry((HANDLE)wParam);
	if(!FindItem(hwnd,dat,(HANDLE)wParam,&contact,NULL,NULL,FALSE)) 
		return ret;

	lstrcpyn(contact->szText, pcli->pfnGetContactDisplayName((HANDLE)wParam,0),SIZEOF(contact->szText));
	if (contact)//!IsBadWritePtr(contact, sizeof(struct ClcContact)))
	{
		Cache_GetText(dat,contact,1);
		cliRecalcScrollBar(hwnd,dat);
	}
	dat->NeedResort=1;
	pcli->pfnSortContacts();

	return ret;
}

static LRESULT clcOnIntmApparentModeChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int lResult=corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	if (!ServiceExists("ExtraIcon/Register"))
		ExtraImage_SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
	return lResult;
}

static LRESULT clcOnIntmStatusMsgChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct ClcContact *contact;
	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL || IsHContactInfo(hContact) || IsHContactGroup(hContact))
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	if (!FindItem(hwnd,dat,hContact,&contact,NULL,NULL,FALSE)) 
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	if (contact)//!IsBadWritePtr(contact, sizeof(struct ClcContact)))
	{
		Cache_GetText(dat,contact,1);
		cliRecalcScrollBar(hwnd,dat);
		PostMessage(hwnd,INTM_INVALIDATE,0,0);
	}
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnIntmNotOnListChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
	struct ClcContact *contact;
	
	if(!FindItem(hwnd,dat,(HANDLE)wParam,&contact,NULL,NULL,TRUE))
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	
	if(contact->type!=CLCIT_CONTACT) 
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	
	if(dbcws->value.type==DBVT_DELETED || dbcws->value.bVal==0) 
		contact->flags&=~CONTACTF_NOTONLIST;
	else
		contact->flags|=CONTACTF_NOTONLIST;
	
	CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnIntmScrollBarChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (GetWindowLong(hwnd, GWL_STYLE) & CLS_CONTACTLIST) 
	{
		if (dat->noVScrollbar) ShowScrollBar(hwnd, SB_VERT, FALSE);
		else pcli->pfnRecalcScrollBar(hwnd, dat);
	}
	return 0;
}

static LRESULT clcOnIntmStatusChanged(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ret=corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	if (wParam != 0)
	{
		pdisplayNameCacheEntry pdnce = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry((HANDLE)wParam);
		if (pdnce && pdnce->m_cache_cszProto)
		{
			struct ClcContact *contact=NULL;
			pdnce___SetStatus( pdnce, GetStatusForContact(pdnce->m_cache_hContact,pdnce->m_cache_cszProto));
			if (!dat->force_in_dialog && (
				(dat->second_line_show)// && dat->second_line_type==TEXT_STATUS)
				|| (dat->third_line_show)// && dat->third_line_type==TEXT_STATUS)
				))
				gtaRenewText(pdnce->m_cache_hContact);
			SendMessage(hwnd,INTM_ICONCHANGED, wParam, (LPARAM) CallService(MS_CLIST_GETCONTACTICON, wParam, 1));
			if(FindItem(hwnd,dat,(HANDLE)wParam,&contact,NULL,NULL,TRUE))
			{
				if (contact && contact->type==CLCIT_CONTACT)
				{
					if (!contact->image_is_special && pdnce___GetStatus( pdnce )>ID_STATUS_OFFLINE) 
						contact->iImage=CallService(MS_CLIST_GETCONTACTICON, wParam, 1);
					if (contact->isSubcontact 
						&& contact->subcontacts 
						&& contact->subcontacts->type==CLCIT_CONTACT)
						pcli->pfnClcBroadcast( INTM_STATUSCHANGED,(WPARAM)contact->subcontacts->hContact,0); //forward status changing to host meta contact
				}
			}
		}
#ifdef _DEBUG
		else
		{
			DebugBreak();
		}
#endif


	}
	if (ModernGetSettingByte(NULL,"CList","PlaceOfflineToRoot",SETTING_PLACEOOFLINETOROOT_DEFAULT) )
	{
		SendMessage(hwnd,CLM_AUTOREBUILD,0,0);	
	}
	else
	{
		pcli->pfnSortContacts();
		PostMessage(hwnd,INTM_INVALIDATE,0,0);
	}
	return ret;
}



static LRESULT clcOnIntmReloadOptions(struct ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);	
	pcli->pfnLoadClcOptions(hwnd,dat);
	LoadCLCOptions(hwnd,dat);
	pcli->pfnSaveStateAndRebuildList(hwnd,dat);
	pcli->pfnSortCLC(hwnd,dat,1);
	if (IsWindowVisible(hwnd))
		pcli->pfnInvalidateRect(GetParent(hwnd), NULL, FALSE);
	return TRUE;
}


HRESULT ClcLoadModule()
{
	g_himlCListClc=(HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST,0,0);
	hSettingChanged=ModernHookEvent(ME_DB_CONTACT_SETTINGCHANGED,clcHookSettingChanged);
	ModernHookEvent(ME_OPT_INITIALISE,ClcOptInit);
	hAckHook=(HANDLE)ModernHookEvent(ME_PROTO_ACK,clcHookProtoAck);
	ModernHookEvent(ME_SYSTEM_MODULESLOADED, clcHookModulesLoaded);
	ModernHookEvent(ME_DB_EVENT_ADDED, clcHookDbEventAdded);
	ModernHookEvent(ME_SYSTEM_SHUTDOWN,clcHookSystemShutdown);
	return S_OK;
}


int ClcUnloadModule()
{
	if(hAvatarChanged!=0)
		ModernUnhookEvent(hAvatarChanged);
	if(hSmileyAddOptionsChangedHook!=0)
		ModernUnhookEvent(hSmileyAddOptionsChangedHook);
	if(hIconChangedHook!=0)
		ModernUnhookEvent(hIconChangedHook);

	if ( g_CluiData.bOldUseGroups != (BYTE)-1 )
		ModernWriteSettingByte(NULL,"CList","UseGroups",(BYTE)g_CluiData.bOldUseGroups );
	if ( g_CluiData.boldHideOffline != (BYTE)-1 )
		ModernWriteSettingByte(NULL,"CList","HideOffline",(BYTE)g_CluiData.boldHideOffline );

	return 0;
}
int ClcDoProtoAck(HANDLE wParam,ACKDATA * ack)
{	
	if (MirandaExiting()) return 0;
	if (ack->type == ACKTYPE_STATUS) 
	{ 
		int i;
		if (ack->result == ACKRESULT_SUCCESS) 
		{
			for (i = 0; i < pcli->hClcProtoCount; i++) 
			{
				if (!lstrcmpA(pcli->clcProto[i].szProto, ack->szModule)) 
				{
					pcli->clcProto[i].dwStatus = (WORD) ack->lParam;
					if (pcli->clcProto[i].dwStatus>=ID_STATUS_OFFLINE)
						pcli->pfnTrayIconUpdateBase(pcli->clcProto[i].szProto);
					if (!ServiceExists("ExtraIcon/Register"))
						if(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_VISMODE)!=-1)
							ExtraImage_SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)NULL);				
					return 0;
				}
			}
		}
	}
	else if (ack->type==ACKTYPE_AWAYMSG)
	{
		if (ack->result==ACKRESULT_SUCCESS && ack->lParam) {
			{//Do not change DB if it is IRC protocol    
				if (ack->szModule!= NULL) 
					if(ModernGetSettingByte(ack->hContact, ack->szModule, "ChatRoom", 0) != 0) return 0;
			}

			{
				DBVARIANT dbv={0};
				BOOL bUnicode = (!ModernGetSetting(ack->hContact, "CList", "StatusMsg", &dbv) && (dbv.type !=DBVT_ASCIIZ) );
				ModernDBFreeVariant(&dbv);
				if (!bUnicode)
				{
					char * val= ModernGetStringA(ack->hContact,"CList","StatusMsg");
					if (val) 
					{
						if (!mir_bool_strcmpi(val,(const char *)ack->lParam))
							ModernWriteSettingString(ack->hContact,"CList","StatusMsg",(const char *)ack->lParam);
						else
							gtaRenewText(ack->hContact);
						mir_free_and_nill(val);
					}
					else 
						ModernWriteSettingString(ack->hContact,"CList","StatusMsg",(const char *)ack->lParam);

					//pcli->pfnClcBroadcast( INTM_STATUSMSGCHANGED,(WPARAM)ack->hContact,(LPARAM)ack->lParam);      
				}
				gtaRenewText(ack->hContact);
			}
		} 
		else
		{
			//DBDeleteContactSetting(ack->hContact,"CList","StatusMsg");
			//char a='\0';
			{//Do not change DB if it is IRC protocol    
				if (ack->szModule!= NULL) 
					if(ModernGetSettingByte(ack->hContact, ack->szModule, "ChatRoom", 0) != 0) return 0;
			}
			if (ack->hContact) 
			{
				char * val= ModernGetStringA(ack->hContact,"CList","StatusMsg");
				if (val) 
				{
					if (!mir_bool_strcmpi(val,""))
						ModernWriteSettingString(ack->hContact,"CList","StatusMsg","");
					else
						gtaRenewText(ack->hContact);
					mir_free_and_nill(val);
				}
			}
			//pcli->pfnClcBroadcast( INTM_STATUSMSGCHANGED,(WPARAM)ack->hContact,&a);              
		}
	}
	else if (ack->type==ACKTYPE_AVATAR) 
	{
		if (ack->result==ACKRESULT_SUCCESS) 
		{
			PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *) ack->hProcess;

			if (pai != NULL && pai->hContact != NULL)
			{
				pcli->pfnClcBroadcast( INTM_AVATARCHANGED,(WPARAM)pai->hContact,0);
			}
		}
	}
	else if (ack->type == ACKTYPE_EMAIL) {
		CLUIUnreadEmailCountChanged(0,0);
	}
	return 0;
}


int ClcGetShortData(struct ClcData* pData, struct SHORTDATA *pShortData)
{
	if (!pData|| !pShortData) return -1;
	pShortData->hWnd=pData->hWnd;
	pShortData->text_replace_smileys=pData->text_replace_smileys;
	pShortData->text_smiley_height=pData->text_smiley_height;
	pShortData->text_use_protocol_smileys=pData->text_use_protocol_smileys;
	pShortData->contact_time_show_only_if_different=pData->contact_time_show_only_if_different;
	// Second line
	pShortData->second_line_show=pData->second_line_show;
	pShortData->second_line_draw_smileys=pData->second_line_draw_smileys;
	pShortData->second_line_type=pData->second_line_type;

	_tcsncpy(pShortData->second_line_text,pData->second_line_text,TEXT_TEXT_MAX_LENGTH);

	pShortData->second_line_xstatus_has_priority=pData->second_line_xstatus_has_priority;
	pShortData->second_line_show_status_if_no_away=pData->second_line_show_status_if_no_away;
	pShortData->second_line_show_listening_if_no_away=pData->second_line_show_listening_if_no_away;
	pShortData->second_line_use_name_and_message_for_xstatus=pData->second_line_use_name_and_message_for_xstatus;

	pShortData->third_line_show=pData->third_line_show;
	pShortData->third_line_draw_smileys=pData->third_line_draw_smileys;
	pShortData->third_line_type=pData->third_line_type;

	_tcsncpy(pShortData->third_line_text,pData->third_line_text,TEXT_TEXT_MAX_LENGTH);

	pShortData->third_line_xstatus_has_priority=pData->third_line_xstatus_has_priority;
	pShortData->third_line_show_status_if_no_away=pData->third_line_show_status_if_no_away;
	pShortData->third_line_show_listening_if_no_away=pData->third_line_show_listening_if_no_away;
	pShortData->third_line_use_name_and_message_for_xstatus=pData->third_line_use_name_and_message_for_xstatus;

	return 0;
}



int ClcEnterDragToScroll(HWND hwnd, int Y)
{
	struct ClcData * dat;
	if (IsDragToScrollMode) return 0;
	dat=(struct ClcData*)GetWindowLongPtr(hwnd,0);
	if (!dat) return 0;
	StartDragPos=Y;
	StartScrollPos=dat->yScroll;
	IsDragToScrollMode=1;
	SetCapture(hwnd);
	return 1;
}


/*
*	Contact list control window procedure
*/
LRESULT CALLBACK cli_ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{     

#define CASE_MSG_RET(msg, handler) case msg: return handler(dat, hwnd, msg, wParam, lParam); 

	struct ClcData *dat;
	dat=(struct ClcData*)GetWindowLongPtr(hwnd,0);

	if(msg>=CLM_FIRST && msg<CLM_LAST) 
		return cli_ProcessExternalMessages(hwnd,dat,msg,wParam,lParam);

	switch (msg) 
	{
		CASE_MSG_RET( INTM_GROUPCHANGED,		clcOnIntmGroupChanged			);
		CASE_MSG_RET( INTM_ICONCHANGED,			clcOnIntmIconChanged			);
		CASE_MSG_RET( INTM_AVATARCHANGED,		clcOnIntmAvatarChanged			);
		CASE_MSG_RET( INTM_TIMEZONECHANGED,		clcOnIntmTimeZoneChanged		);
		CASE_MSG_RET( INTM_NAMECHANGED,			clcOnIntmNameChanged			);
		CASE_MSG_RET( INTM_APPARENTMODECHANGED,	clcOnIntmApparentModeChanged	);
		CASE_MSG_RET( INTM_STATUSMSGCHANGED,	clcOnIntmStatusMsgChanged		);
		CASE_MSG_RET( INTM_NOTONLISTCHANGED,	clcOnIntmNotOnListChanged		);
		CASE_MSG_RET( INTM_SCROLLBARCHANGED,	clcOnIntmScrollBarChanged		);
		CASE_MSG_RET( INTM_STATUSCHANGED,		clcOnIntmStatusChanged			);
		CASE_MSG_RET( INTM_RELOADOPTIONS,		clcOnIntmReloadOptions			);

		CASE_MSG_RET( WM_CREATE,		clcOnCreate				);
		CASE_MSG_RET( WM_NCHITTEST,		clcOnHitTest			);
		CASE_MSG_RET( WM_COMMAND,		clcOnCommand			);
		CASE_MSG_RET( WM_SIZE,			clcOnSize				);
		CASE_MSG_RET( WM_CHAR,			clcOnChar				);
		CASE_MSG_RET( WM_PAINT,			clcOnPaint				);
		CASE_MSG_RET( WM_ERASEBKGND,	clcOnEraseBkGround		);
		CASE_MSG_RET( WM_KEYDOWN,		clcOnKeyDown			);
		CASE_MSG_RET( WM_TIMER,			clcOnTimer				);
		CASE_MSG_RET( WM_ACTIVATE,		clcOnActivate			);
		CASE_MSG_RET( WM_SETCURSOR,		clcOnSetCursor			);
		CASE_MSG_RET( WM_LBUTTONDOWN,	clcOnLButtonDown		);
		CASE_MSG_RET( WM_CAPTURECHANGED,clcOnCaptureChanged		);
		CASE_MSG_RET( WM_MOUSEMOVE,		clcOnMouseMove			);
		CASE_MSG_RET( WM_LBUTTONUP,		clcOnLButtonUp			);
		CASE_MSG_RET( WM_LBUTTONDBLCLK,	clcOnLButtonDblClick	);
		CASE_MSG_RET( WM_DESTROY,		clcOnDestroy			);

	default:
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}
