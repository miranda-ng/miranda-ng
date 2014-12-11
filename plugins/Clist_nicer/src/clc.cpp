/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include <commonheaders.h>
#include <resource.h>
#include <m_userinfo.h>
#include "../cluiframes/cluiframes.h"
#include "../coolsb/coolscroll.h"

void CSH_Destroy();

int DefaultImageListColorDepth = ILC_COLOR32;

extern HPEN g_hPenCLUIFrames;
extern FRAMEWND *wndFrameCLC;
extern ButtonItem *g_ButtonItems;

extern int during_sizing;

HIMAGELIST hCListImages;

HANDLE hSvc_GetContactStatusMsg = 0;

static HRESULT(WINAPI *MyCloseThemeData)(HANDLE);

LONG g_cxsmIcon, g_cysmIcon;

void  SetDBButtonStates(MCONTACT hContact);

HMENU BuildGroupPopupMenu(ClcGroup* group)
{
	return (HMENU)CallService(MS_CLIST_MENUBUILDSUBGROUP, (WPARAM)group, 0);
}

int AvatarChanged(WPARAM wParam, LPARAM lParam)
{
	pcli->pfnClcBroadcast(INTM_AVATARCHANGED, wParam, lParam);
	return 0;
}

int __forceinline __strcmp(const char * src, const char * dst)
{
	int ret = 0 ;

	while (!(ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
		++src, ++dst;
	return(ret);
}

static int ClcEventAdded(WPARAM hContact, LPARAM lParam)
{
	DWORD new_freq = 0;

	cfg::dat.t_now = time(NULL);

	if (hContact && lParam) {
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get((HANDLE)lParam, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
			DWORD firstTime = cfg::getDword(hContact, "CList", "mf_firstEvent", 0);
			DWORD count = cfg::getDword(hContact, "CList", "mf_count", 0);
			count++;
			new_freq = count ? (dbei.timestamp - firstTime) / count : 0x7fffffff;
			cfg::writeDword(hContact, "CList", "mf_freq", new_freq);
			cfg::writeDword(hContact, "CList", "mf_count", count);

			TExtraCache *p = cfg::getCache(hContact, NULL);
			if (p) {
				p->dwLastMsgTime = dbei.timestamp;
				if (new_freq)
					p->msgFrequency = new_freq;
				pcli->pfnClcBroadcast(INTM_FORCESORT, 0, 1);
			}
		}
	}
	return 0;
}

static int ClcMetamodeChanged(WPARAM bMetaEnabled, LPARAM)
{
	if (bMetaEnabled != cfg::dat.bMetaEnabled) {
		cfg::dat.bMetaEnabled = (BYTE)bMetaEnabled;
		pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
	}
	return 0;
}

static int ClcSettingChanged(WPARAM hContact, LPARAM lParam)
{
	char *szProto = NULL;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;

	if (hContact) {
		if ( !__strcmp(cws->szModule, "CList")) {
			if ( !__strcmp(cws->szSetting, "StatusMsg"))
				SendMessage(pcli->hwndContactTree, INTM_STATUSMSGCHANGED, hContact, lParam);
		}
		else if ( !__strcmp(cws->szModule, "UserInfo")) {
			if ( !__strcmp(cws->szSetting, "ANSIcodepage"))
				pcli->pfnClcBroadcast(INTM_CODEPAGECHANGED, hContact, lParam);
			else if ( !__strcmp(cws->szSetting, "Timezone") || !__strcmp(cws->szSetting, "TzName"))
				ReloadExtraInfo(hContact);
		}
		else if (hContact != 0 && (szProto = GetContactProto(hContact)) != NULL) {
			if ( !__strcmp(cws->szModule, "Protocol") && !__strcmp(cws->szSetting, "p")) {
				char *szProto_s;
				pcli->pfnClcBroadcast(INTM_PROTOCHANGED, hContact, lParam);
				if (cws->value.type == DBVT_DELETED)
					szProto_s = NULL;
				else
					szProto_s = cws->value.pszVal;
				pcli->pfnChangeContactIcon(hContact, IconFromStatusMode(szProto_s, szProto_s == NULL ? ID_STATUS_OFFLINE : cfg::getWord(hContact, szProto_s, "Status", ID_STATUS_OFFLINE), hContact, NULL), 0);
			}
			// something is being written to a protocol module
			if ( !__strcmp(szProto, cws->szModule)) {
				// was a unique setting key written?
				pcli->pfnInvalidateDisplayNameCacheEntry(hContact);
				if ( !__strcmp(cws->szSetting, "Status")) {
					if (!cfg::getByte(hContact, "CList", "Hidden", 0)) {
						if (cfg::getByte("CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT)) {
							// User's state is changing, and we are hideOffline-ing
							if (cws->value.wVal == ID_STATUS_OFFLINE) {
								pcli->pfnChangeContactIcon(hContact, IconFromStatusMode(cws->szModule, cws->value.wVal, hContact, NULL), 0);
								CallService(MS_CLUI_CONTACTDELETED, hContact, 0);
								return 0;
							}
							pcli->pfnChangeContactIcon(hContact, IconFromStatusMode(cws->szModule, cws->value.wVal, hContact, NULL), 1);
						}
						pcli->pfnChangeContactIcon(hContact, IconFromStatusMode(cws->szModule, cws->value.wVal, hContact, NULL), 0);
					}
					SendMessage(pcli->hwndContactTree, INTM_STATUSCHANGED, hContact, lParam);
					return 0;
				}
				else if (strstr("YMsg|StatusDescr|XStatusMsg", cws->szSetting))
					SendMessage(pcli->hwndContactTree, INTM_STATUSMSGCHANGED, hContact, lParam);
				else if (strstr(cws->szSetting, "XStatus"))
					SendMessage(pcli->hwndContactTree, INTM_XSTATUSCHANGED, hContact, lParam);
				else if ( !__strcmp(cws->szSetting, "Timezone") || !__strcmp(cws->szSetting, "TzName"))
					ReloadExtraInfo(hContact);

				if (!(cfg::dat.dwFlags & CLUI_USEMETAICONS) && !__strcmp(szProto, META_PROTO))
					if ((mir_strlen(cws->szSetting) > 6 && !strncmp(cws->szSetting, "Status", 6)) || strstr("Default,ForceSend,Nick", cws->szSetting))
						pcli->pfnClcBroadcast(INTM_NAMEORDERCHANGED, hContact, lParam);
			}
			if (cfg::dat.bMetaEnabled && !__strcmp(cws->szModule, META_PROTO) && !__strcmp(cws->szSetting, "IsSubcontact"))
				pcli->pfnClcBroadcast(INTM_HIDDENCHANGED, hContact, lParam);
		}
	}
	else if (!__strcmp(cws->szModule, "Skin") && !__strcmp(cws->szSetting, "UseSound")) {
		cfg::dat.soundsOff = cfg::getByte(cws->szModule, cws->szSetting, 0) ? 0 : 1;
		ClcSetButtonState(IDC_TBSOUND, cfg::dat.soundsOff ? BST_CHECKED : BST_UNCHECKED);
		SetButtonStates();
	}
	else if (!__strcmp(cws->szModule, "CList") && !__strcmp(cws->szSetting, "UseGroups")) {
		ClcSetButtonState(IDC_TBHIDEGROUPS, cfg::getByte(cws->szModule, cws->szSetting, SETTING_USEGROUPS_DEFAULT));
		SetButtonStates();
	}
	else if (!__strcmp(cws->szModule, "TopToolBar") && !__strcmp(cws->szSetting, "UseFlatButton")) {
		SetButtonToSkinned();
	}
	else if (szProto == NULL) {
		if ( !__strcmp(cws->szSetting, "XStatusId"))
			CluiProtocolStatusChanged(0, cws->szModule);
	}
	return 0;
}

static int ClcPreshutdown(WPARAM, LPARAM)
{
	cfg::shutDown = TRUE;
	if (hSvc_GetContactStatusMsg)
		DestroyServiceFunction(hSvc_GetContactStatusMsg);
	return 0;
}

int ClcShutdown(WPARAM, LPARAM)
{
	DeleteObject(cfg::dat.hPen3DBright);
	DeleteObject(cfg::dat.hPen3DDark);
	DeleteObject(cfg::dat.hBrushColorKey);
	DeleteObject(cfg::dat.hBrushCLCBk);
	DeleteObject(cfg::dat.hBrushAvatarBorder);
	DestroyMenu(cfg::dat.hMenuNotify);
	ClearIcons(1);

	CSH_Destroy();
	IMG_DeleteItems();
	for (int i=0; i < arStatusItems.getCount(); i++)
		mir_free(arStatusItems[i]);
	return 0;
}

int LoadCLCModule(void)
{
	g_cxsmIcon = GetSystemMetrics(SM_CXSMICON);
	g_cysmIcon = GetSystemMetrics(SM_CYSMICON);

	hCListImages = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);

	HookEvent(ME_MC_ENABLED, ClcMetamodeChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ClcSettingChanged);
	HookEvent(ME_DB_EVENT_ADDED, ClcEventAdded);
	HookEvent(ME_OPT_INITIALISE, ClcOptInit);
	HookEvent(ME_SYSTEM_SHUTDOWN, ClcPreshutdown);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// clist_nicer+ control window procedure

LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcContact *contact;
	ClcGroup *group;
	BOOL frameHasTitlebar = FALSE;

	if (wndFrameCLC)
		frameHasTitlebar = wndFrameCLC->TitleBar.ShowTitleBar;

	ClcData *dat = (struct ClcData *) GetWindowLongPtr(hwnd, 0);
	if (msg >= CLM_FIRST && msg < CLM_LAST)
		return ProcessExternalMessages(hwnd, dat, msg, wParam, lParam);

	switch (msg) {
	case WM_CREATE:
		dat = (struct ClcData *)mir_alloc(sizeof(struct ClcData));
		memset(dat, 0, sizeof(struct ClcData));
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat);

		RowHeight::Init(dat);
		dat->forceScroll = 0;
		dat->lastRepaint = 0;
		dat->hwndParent = GetParent(hwnd);
		dat->lastSort = GetTickCount();
		dat->needsResort = FALSE;
		{
			CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
			if (cs->lpCreateParams == (LPVOID)0xff00ff00) {
				dat->bisEmbedded = FALSE;
				dat->bHideSubcontacts = TRUE;
				cfg::clcdat = dat;
				if (cfg::dat.bShowLocalTime)
					SetTimer(hwnd, TIMERID_REFRESH, 65000, NULL);
			}
			else
				dat->bisEmbedded = TRUE;
		}
		break;

	case WM_SIZE:
		pcli->pfnEndRename(hwnd, dat, 1);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		pcli->pfnRecalcScrollBar(hwnd, dat);
LBL_Def:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NCCALCSIZE:
		return FrameNCCalcSize(hwnd, DefWindowProc, wParam, lParam, frameHasTitlebar);

		/*
		* scroll bar handling
		*/
	case WM_NCPAINT:
		return FrameNCPaint(hwnd, DefWindowProc, wParam, lParam, frameHasTitlebar);

	case INTM_GROUPCHANGED:
		{
			WORD iExtraImage[EXTRA_ICON_COUNT];
			BYTE flags = 0;
			if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				memset(iExtraImage, 0xFF, sizeof(iExtraImage));
			else {
				memcpy(iExtraImage, contact->iExtraImage, sizeof(iExtraImage));
				flags = contact->flags;
			}
			pcli->pfnDeleteItemFromTree(hwnd, wParam);
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN || !CLVM_GetContactHiddenStatus(wParam, NULL, dat)) {
				pcli->pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				if (FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL)) {
					memcpy(contact->iExtraImage, iExtraImage, sizeof(iExtraImage));
					if (flags & CONTACTF_CHECKED)
						contact->flags |= CONTACTF_CHECKED;
				}

				NMCLISTCONTROL nm;
				nm.hdr.code = CLN_CONTACTMOVED;
				nm.hdr.hwndFrom = hwnd;
				nm.hdr.idFrom = GetDlgCtrlID(hwnd);
				nm.flags = 0;
				nm.hItem = (HANDLE)wParam;
				SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
			}
			dat->needsResort = TRUE;
			PostMessage(hwnd, INTM_SORTCLC, 0, 1);
		}
		goto LBL_Def;

	case INTM_ICONCHANGED:
		{
			int recalcScrollBar = 0;
			MCONTACT hContact = wParam;
			WORD status = ID_STATUS_OFFLINE;
			int  contactRemoved = 0;
			MCONTACT hSelItem = NULL;
			ClcContact *selcontact = NULL;

			char *szProto = GetContactProto(hContact);
			if (szProto == NULL)
				status = ID_STATUS_OFFLINE;
			else
				status = cfg::getWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);

			int shouldShow = (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN ||
				!CLVM_GetContactHiddenStatus(hContact, szProto, dat)) && ((cfg::dat.bFilterEffective ? TRUE : !pcli->pfnIsHiddenMode(dat, status)) ||
				pcli->pfnGetContactIcon(hContact) != lParam); // XXX CLVM changed - this means an offline msg is flashing, so the contact should be shown

			if (!FindItem(hwnd, dat, (HANDLE)hContact, &contact, &group, NULL)) {
				if (shouldShow && CallService(MS_DB_CONTACT_IS, wParam, 0)) {
					if (dat->selection >= 0 && pcli->pfnGetRowByIndex(dat, dat->selection, &selcontact, NULL) != -1)
						hSelItem = (MCONTACT)pcli->pfnContactToHItem(selcontact);
					pcli->pfnAddContactToTree(hwnd, dat, hContact, 0, 0);
					recalcScrollBar = 1;
					FindItem(hwnd, dat, (HANDLE)hContact, &contact, NULL, NULL);
					if (contact) {
						contact->iImage = (WORD)lParam;
						pcli->pfnNotifyNewContact(hwnd, hContact);
					}
				}
			}
			else {
				//item in list already
				DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);
				if (contact->iImage == (WORD)lParam)
					break;
				if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline || cfg::dat.bFilterEffective)) {        // CLVM changed
					if (dat->selection >= 0 && pcli->pfnGetRowByIndex(dat, dat->selection, &selcontact, NULL) != -1)
						hSelItem = (MCONTACT)pcli->pfnContactToHItem(selcontact);
					pcli->pfnRemoveItemFromGroup(hwnd, group, contact, 0);
					contactRemoved = TRUE;
					recalcScrollBar = 1;
				}
				else {
					contact->iImage = (WORD)lParam;
					if (!pcli->pfnIsHiddenMode(dat, status))
						contact->flags |= CONTACTF_ONLINE;
					else
						contact->flags &= ~CONTACTF_ONLINE;
				}
			}
			if (hSelItem) {
				ClcGroup *selgroup;
				if (pcli->pfnFindItem(hwnd, dat, hSelItem, &selcontact, &selgroup, NULL))
					dat->selection = pcli->pfnGetRowsPriorTo(&dat->list, selgroup, List_IndexOf((SortedList*)& selgroup->cl, selcontact));
				else
					dat->selection = -1;
			}
			dat->needsResort = TRUE;
			PostMessage(hwnd, INTM_SORTCLC, 0, recalcScrollBar);
			PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)(contactRemoved ? 0 : wParam));
			if (recalcScrollBar)
				pcli->pfnRecalcScrollBar(hwnd, dat);
		}
		goto LBL_Def;

	case INTM_METACHANGED:
		if (!pcli->pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			break;

		if (contact->bIsMeta && !(cfg::dat.dwFlags & CLUI_USEMETAICONS)) {
			contact->hSubContact = db_mc_getMostOnline(contact->hContact);
			contact->metaProto = GetContactProto(contact->hSubContact);
			contact->iImage = pcli->pfnGetContactIcon(contact->hSubContact);
			if (contact->pExtra) {
				TExtraCache *pSub = cfg::getCache(contact->hSubContact, contact->metaProto);
				ClcContact *subContact;
				if (!pcli->pfnFindItem(hwnd, dat, contact->hSubContact, &subContact, NULL, NULL))
					break;

				contact->pExtra->proto_status_item = GetProtocolStatusItem(contact->metaProto);
				if (pSub) {
					contact->pExtra->status_item = pSub->status_item;
					memcpy(contact->iExtraImage, subContact->iExtraImage, sizeof(contact->iExtraImage));
				}
			}
		}
		SendMessage(hwnd, INTM_NAMEORDERCHANGED, wParam, lParam);
		goto LBL_Def;

	case INTM_METACHANGEDEVENT:
		if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
			break;
		if (lParam == 0)
			pcli->pfnInitAutoRebuild(hwnd);
		goto LBL_Def;

	case INTM_NAMECHANGED:
		ClcContact *contact;
		if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
			break;
		mir_tstrncpy(contact->szText, pcli->pfnGetContactDisplayName(wParam, 0), SIZEOF(contact->szText));

		RTL_DetectAndSet(contact, 0);

		dat->needsResort = TRUE;
		PostMessage(hwnd, INTM_SORTCLC, 0, 0);
		goto LBL_Def;

	case INTM_CODEPAGECHANGED:
		if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
			break;
		contact->codePage = cfg::getDword(wParam, "Tab_SRMsg", "ANSIcodepage", cfg::getDword(wParam, "UserInfo", "ANSIcodepage", CP_ACP));
		PostMessage(hwnd, INTM_INVALIDATE, 0, 0);
		goto LBL_Def;

	case INTM_AVATARCHANGED:
		contact = NULL;
		{
			avatarCacheEntry *cEntry = (struct avatarCacheEntry *)lParam;

			if (wParam == 0) {
				//RemoveFromImgCache(0, cEntry);
				cfg::dat.bForceRefetchOnPaint = TRUE;
				RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
				cfg::dat.bForceRefetchOnPaint = FALSE;
				goto LBL_Def;
			}

			if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;
			contact->ace = cEntry;
			if (cEntry == NULL)
				contact->cFlags &= ~ECF_AVATAR;
			else {
				DWORD dwFlags;

				if (contact->pExtra)
					dwFlags = contact->pExtra->dwDFlags;
				else
					dwFlags = cfg::getDword(contact->hContact, "CList", "CLN_Flags", 0);
				if (cfg::dat.dwFlags & CLUI_FRAME_AVATARS)
					contact->cFlags = (dwFlags & ECF_HIDEAVATAR ? contact->cFlags & ~ECF_AVATAR : contact->cFlags | ECF_AVATAR);
				else
					contact->cFlags = (dwFlags & ECF_FORCEAVATAR ? contact->cFlags | ECF_AVATAR : contact->cFlags & ~ECF_AVATAR);
			}
			PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)contact->hContact);
		}
		goto LBL_Def;

	case INTM_STATUSMSGCHANGED:
		{
			TExtraCache *p;
			char *szProto = NULL;

			if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				p = cfg::getCache(wParam, NULL);
			else {
				p = contact->pExtra;
				szProto = contact->proto;
			}
			GetCachedStatusMsg(p, szProto);
			PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)(contact ? contact->hContact : 0));
		}
		goto LBL_Def;

	case INTM_STATUSCHANGED:
		if (FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL)) {
			WORD wStatus = cfg::getWord(wParam, contact->proto, "Status", ID_STATUS_OFFLINE);
			if (cfg::dat.bNoOfflineAvatars && wStatus != ID_STATUS_OFFLINE && contact->wStatus == ID_STATUS_OFFLINE) {
				contact->wStatus = wStatus;
				if (cfg::dat.bAvatarServiceAvail && contact->ace == NULL)
					LoadAvatarForContact(contact);
			}
			contact->wStatus = wStatus;
			goto LBL_Def;
		}
		break;

	case INTM_PROTOCHANGED:
		if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
			break;

		contact->proto = GetContactProto(wParam);
		CallService(MS_CLIST_INVALIDATEDISPLAYNAME, wParam, 0);
		mir_tstrncpy(contact->szText, pcli->pfnGetContactDisplayName(wParam, 0), SIZEOF(contact->szText));

		RTL_DetectAndSet(contact, 0);

		dat->needsResort = TRUE;
		PostMessage(hwnd, INTM_SORTCLC, 0, 0);
		goto LBL_Def;

	case INTM_INVALIDATE:
		if (!dat->bNeedPaint) {
			KillTimer(hwnd, TIMERID_PAINT);
			SetTimer(hwnd, TIMERID_PAINT, 100, NULL);
			dat->bNeedPaint = TRUE;
		}
		goto LBL_Def;

	case INTM_INVALIDATECONTACT:
		if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, &group, NULL))
			break;

		if (contact && group) {
			int iItem = pcli->pfnGetRowsPriorTo(&dat->list, group, List_IndexOf((SortedList*) & group->cl, contact));
			pcli->pfnInvalidateItem(hwnd, dat, iItem);
			goto LBL_Def;
		}
		break;

	case INTM_FORCESORT:
		dat->needsResort = TRUE;
		return SendMessage(hwnd, INTM_SORTCLC, wParam, lParam);

	case INTM_SORTCLC:
		if (dat->needsResort) {
			pcli->pfnSortCLC(hwnd, dat, TRUE);
			dat->needsResort = FALSE;
		}
		if (lParam)
			pcli->pfnRecalcScrollBar(hwnd, dat);
		goto LBL_Def;

	case INTM_IDLECHANGED:
		if (FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL)) {
			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
			char *szProto = (char*)cws->szModule;
			if (szProto == NULL)
				break;

			contact->flags &= ~CONTACTF_IDLE;
			if (cfg::getDword(wParam, szProto, "IdleTS", 0)) {
				contact->flags |= CONTACTF_IDLE;
			}
			PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)contact->hContact);
			goto LBL_Def;
		}
		break;

	case INTM_XSTATUSCHANGED:
		{
			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
			char *szProto = (char *)cws->szModule;
			MCONTACT hContact = wParam;
			TExtraCache *p;

			if (!FindItem(hwnd, dat, (HANDLE)hContact, &contact, NULL, NULL)) {
				p = cfg::getCache(hContact, szProto);
				if (!dat->bisEmbedded && szProto) {				// may be a subcontact, forward the xstatus
					MCONTACT hMasterContact = db_mc_tryMeta(hContact);
					if (hMasterContact != hContact)				// avoid recursive call of settings handler
						cfg::writeByte(hMasterContact, META_PROTO, "XStatusId", (BYTE)cfg::getByte(hContact, szProto, "XStatusId", 0));
					break;
				}
			}
			else {
				contact->xStatus = cfg::getByte(hContact, szProto, "XStatusId", 0);
				p = contact->pExtra;
			}

			if (szProto == NULL)
				break;

			if (contact) {
				if (ProtoServiceExists(szProto, PS_GETADVANCEDSTATUSICON)) {
					int iconId = ProtoCallService(szProto, PS_GETADVANCEDSTATUSICON, hContact, 0);
					if (iconId != -1)
						contact->xStatusIcon = iconId >> 16;
				}
			}

			GetCachedStatusMsg(p, szProto);
			PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)(contact ? contact->hContact : 0));
		}
		goto LBL_Def;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			if (IsWindowVisible(hwnd) && !during_sizing && !cfg::shutDown) {
				PaintClc(hwnd, dat, hdc, &ps.rcPaint);
				dat->bNeedPaint = FALSE;
				dat->lastRepaint = GetTickCount();
			}
			EndPaint(hwnd, &ps);
			if (dat->selection != dat->oldSelection && !dat->bisEmbedded && g_ButtonItems != NULL) {
				SetDBButtonStates(0);
				dat->oldSelection = dat->selection;
			}
		}
		goto LBL_Def;

	case WM_MOUSEWHEEL:
		dat->forceScroll = TRUE;
		break;

	case WM_TIMER:
		if (wParam == TIMERID_PAINT) {
			KillTimer(hwnd, TIMERID_PAINT);
			InvalidateRect(hwnd, NULL, FALSE);
			goto LBL_Def;
		}

		if (wParam == TIMERID_REFRESH) {
			InvalidateRect(hwnd, NULL, FALSE);
			goto LBL_Def;
		}
		break;

	case WM_LBUTTONDBLCLK:
		ReleaseCapture();
		dat->iHotTrack = -1;
		pcli->pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);
		dat->szQuickSearch[0] = 0;
		{
			DWORD hitFlags;
			dat->selection = HitTest(hwnd, dat, (short) LOWORD(lParam), (short) HIWORD(lParam), &contact, NULL, &hitFlags);
			if (hitFlags & CLCHT_ONITEMEXTRA)
				break;

			InvalidateRect(hwnd, NULL, FALSE);
			if (dat->selection != -1)
				pcli->pfnEnsureVisible(hwnd, dat, dat->selection, 0);
			if (hitFlags & CLCHT_ONAVATAR && cfg::dat.bDblClkAvatars) {
				CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)contact->hContact, 0);
				return TRUE;
			}
			if (hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMLABEL | CLCHT_ONITEMSPACE)) {
				UpdateWindow(hwnd);
				pcli->pfnDoSelectionDefaultAction(hwnd, dat);
			}
		}
		return TRUE;

	case WM_CONTEXTMENU:
		{
			HMENU hMenu = NULL;
			POINT pt;
			DWORD hitFlags;

			pcli->pfnEndRename(hwnd, dat, 1);
			pcli->pfnHideInfoTip(hwnd, dat);
			KillTimer(hwnd, TIMERID_RENAME);
			KillTimer(hwnd, TIMERID_INFOTIP);
			if (GetFocus() != hwnd)
				SetFocus(hwnd);
			dat->iHotTrack = -1;
			dat->szQuickSearch[0] = 0;
			pt.x = (short) LOWORD(lParam);
			pt.y = (short) HIWORD(lParam);
			if (pt.x == -1 && pt.y == -1) {
				dat->selection = pcli->pfnGetRowByIndex(dat, dat->selection, &contact, NULL);
				if (dat->selection != -1)
					pcli->pfnEnsureVisible(hwnd, dat, dat->selection, 0);
				pt.x = dat->iconXSpace + 15;
				pt.y = RowHeight::getItemTopY(dat, dat->selection) - dat->yScroll + (int)(dat->row_heights[dat->selection] * .7);
				hitFlags = dat->selection == -1 ? CLCHT_NOWHERE : CLCHT_ONITEMLABEL;
			}
			else {
				ScreenToClient(hwnd, &pt);
				dat->selection = HitTest(hwnd, dat, pt.x, pt.y, &contact, NULL, &hitFlags);
			}
			InvalidateRect(hwnd, NULL, FALSE);
			if (dat->selection != -1)
				pcli->pfnEnsureVisible(hwnd, dat, dat->selection, 0);
			UpdateWindow(hwnd);

			if (dat->selection != -1 && hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMCHECK | CLCHT_ONITEMLABEL)) {
				if (contact->type == CLCIT_GROUP) {
					hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDSUBGROUP, (WPARAM)contact->group, 0);
					ClientToScreen(hwnd, &pt);
					TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
					CheckMenuItem(hMenu, POPUP_GROUPHIDEOFFLINE, contact->group->hideOffline ? MF_CHECKED : MF_UNCHECKED);
					DestroyMenu(hMenu);
					return 0;
				} else if (contact->type == CLCIT_CONTACT)
					hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) contact->hContact, 0);
			} else {
				//call parent for new group/hide offline menu
				PostMessage(GetParent(hwnd), WM_CONTEXTMENU, wParam, lParam);
				return 0;
			}
			if (hMenu != NULL) {
				ClientToScreen(hwnd, &pt);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				DestroyMenu(hMenu);
			}
		}
		return 0;

	case WM_COMMAND:
		if (LOWORD(wParam) == POPUP_NEWGROUP)
			SendMessage(GetParent(hwnd), msg, wParam, lParam);
		break;

	case WM_NCHITTEST:
		{
			LRESULT lr = SendMessage(GetParent(hwnd), WM_NCHITTEST, wParam, lParam);
			if (lr == HTLEFT || lr == HTRIGHT || lr == HTBOTTOM || lr == HTTOP || lr == HTTOPLEFT || lr == HTTOPRIGHT
				|| lr == HTBOTTOMLEFT || lr == HTBOTTOMRIGHT)
				return HTTRANSPARENT;
		}
		break;

	case WM_DESTROY:
		RowHeight::Free(dat);
		break;
	}

	return coreCli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}
