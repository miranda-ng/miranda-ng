/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "resource.h"
#include <m_userinfo.h>
#include "cluiframes.h"
#include "coolscroll.h"

void CSH_Destroy();

int DefaultImageListColorDepth = ILC_COLOR32;

extern HPEN g_hPenCLUIFrames;
extern FRAMEWND *wndFrameCLC;
extern ButtonItem *g_ButtonItems;

extern int during_sizing;

HIMAGELIST hCListImages;

HANDLE hSvc_GetContactStatusMsg = nullptr;

static HRESULT(WINAPI *MyCloseThemeData)(HANDLE);

LONG g_cxsmIcon, g_cysmIcon;

void  SetDBButtonStates(MCONTACT hContact);

int AvatarChanged(WPARAM wParam, LPARAM lParam)
{
	Clist_Broadcast(INTM_AVATARCHANGED, wParam, lParam);
	return 0;
}

int __forceinline __strcmp(const char * src, const char * dst)
{
	int ret = 0;

	while (!(ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
		++src, ++dst;
	return(ret);
}

static int ClcEventAdded(WPARAM hContact, LPARAM lParam)
{
	uint32_t new_freq = 0;

	cfg::dat.t_now = time(0);

	if (hContact && lParam) {
		DBEVENTINFO dbei = {};
		db_event_get(lParam, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
			uint32_t firstTime = g_plugin.getDword(hContact, "mf_firstEvent");
			uint32_t count = g_plugin.getDword(hContact, "mf_count");
			count++;
			new_freq = count ? (dbei.timestamp - firstTime) / count : 0x7fffffff;
			g_plugin.setDword(hContact, "mf_freq", new_freq);
			g_plugin.setDword(hContact, "mf_count", count);

			TExtraCache *p = cfg::getCache(hContact, nullptr);
			if (p) {
				p->dwLastMsgTime = dbei.timestamp;
				if (new_freq)
					p->msgFrequency = new_freq;
				Clist_Broadcast(INTM_FORCESORT, 0, 1);
			}
		}
	}
	return 0;
}

static int ClcMetamodeChanged(WPARAM bMetaEnabled, LPARAM)
{
	if (BOOL(bMetaEnabled) != cfg::dat.bMetaEnabled) {
		cfg::dat.bMetaEnabled = (uint8_t)bMetaEnabled;
		Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
	}
	return 0;
}

static int ClcSettingChanged(WPARAM hContact, LPARAM lParam)
{
	char *szProto = nullptr;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;

	if (hContact) {
		if (!__strcmp(cws->szModule, "CList")) {
			if (!__strcmp(cws->szSetting, "StatusMsg"))
				SendMessage(g_clistApi.hwndContactTree, INTM_STATUSMSGCHANGED, hContact, lParam);
		}
		else if (!__strcmp(cws->szModule, "UserInfo")) {
			if (!__strcmp(cws->szSetting, "ANSIcodepage"))
				Clist_Broadcast(INTM_CODEPAGECHANGED, hContact, lParam);
			else if (!__strcmp(cws->szSetting, "Timezone") || !__strcmp(cws->szSetting, "TzName"))
				ReloadExtraInfo(hContact);
		}
		else if (hContact != 0 && (szProto = Proto_GetBaseAccountName(hContact)) != nullptr) {
			if (!__strcmp(cws->szModule, "Protocol") && !__strcmp(cws->szSetting, "p")) {
				char *szProto_s;
				Clist_Broadcast(INTM_PROTOCHANGED, hContact, lParam);
				if (cws->value.type == DBVT_DELETED)
					szProto_s = nullptr;
				else
					szProto_s = cws->value.pszVal;
				Clist_ChangeContactIcon(hContact, IconFromStatusMode(szProto_s, szProto_s == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto_s, "Status", ID_STATUS_OFFLINE), hContact));
			}
			// something is being written to a protocol module
			if (!__strcmp(szProto, cws->szModule)) {
				// was a unique setting key written?
				if (!__strcmp(cws->szSetting, "Status")) {
					if (!Contact::IsHidden(hContact))
						if (cws->value.wVal == ID_STATUS_OFFLINE)
							if (Clist::HideOffline)
								return 0;

					SendMessage(g_clistApi.hwndContactTree, INTM_STATUSCHANGED, hContact, lParam);
					return 0;
				}

				if (strstr("YMsg|StatusDescr|XStatusMsg", cws->szSetting))
					SendMessage(g_clistApi.hwndContactTree, INTM_STATUSMSGCHANGED, hContact, lParam);
				else if (strstr(cws->szSetting, "XStatus"))
					SendMessage(g_clistApi.hwndContactTree, INTM_XSTATUSCHANGED, hContact, lParam);
				else if (!__strcmp(cws->szSetting, "Timezone") || !__strcmp(cws->szSetting, "TzName"))
					ReloadExtraInfo(hContact);

				if (!(cfg::dat.dwFlags & CLUI_USEMETAICONS) && !__strcmp(szProto, META_PROTO))
					if ((mir_strlen(cws->szSetting) > 6 && !strncmp(cws->szSetting, "Status", 6)) || strstr("Default,ForceSend,Nick", cws->szSetting))
						Clist_Broadcast(INTM_NAMEORDERCHANGED, hContact, lParam);
			}
			if (cfg::dat.bMetaEnabled && !__strcmp(cws->szModule, META_PROTO) && !__strcmp(cws->szSetting, "IsSubcontact"))
				Clist_Broadcast(INTM_HIDDENCHANGED, hContact, lParam);
		}
	}
	else if (!__strcmp(cws->szModule, "Skin") && !__strcmp(cws->szSetting, "UseSound")) {
		cfg::dat.soundsOff = db_get_b(0, cws->szModule, cws->szSetting, 0) ? 0 : 1;
		ClcSetButtonState(IDC_TBSOUND, cfg::dat.soundsOff ? BST_CHECKED : BST_UNCHECKED);
		SetButtonStates();
	}
	else if (!__strcmp(cws->szModule, "CList") && !__strcmp(cws->szSetting, "UseGroups")) {
		ClcSetButtonState(IDC_TBHIDEGROUPS, cws->value.bVal);
		SetButtonStates();
	}
	else if (!__strcmp(cws->szModule, "TopToolBar") && !__strcmp(cws->szSetting, "UseFlatButton")) {
		SetButtonToSkinned();
	}
	else if (szProto == nullptr) {
		if (!__strcmp(cws->szSetting, "XStatusId"))
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
	for (auto &it : arStatusItems)
		mir_free(it);
	return 0;
}

int LoadCLCModule(void)
{
	g_cxsmIcon = GetSystemMetrics(SM_CXSMICON);
	g_cysmIcon = GetSystemMetrics(SM_CYSMICON);

	hCListImages = Clist_GetImageList();

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

	ClcData *dat = (struct ClcData *)GetWindowLongPtr(hwnd, 0);
	if (msg >= CLM_FIRST && msg < CLM_LAST)
		return ProcessExternalMessages(hwnd, dat, msg, wParam, lParam);

	switch (msg) {
	case WM_CREATE:
		dat = new ClcData();
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat);

		RowHeight::Init(dat);
		dat->bForceScroll = false;
		dat->lastRepaint = 0;
		dat->hwndParent = GetParent(hwnd);
		dat->lastSort = GetTickCount();
		dat->bNeedsResort = false;
		{
			CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
			if (cs->lpCreateParams == (LPVOID)0xff00ff00) {
				dat->bisEmbedded = false;
				dat->bHideSubcontacts = true;
				cfg::clcdat = dat;
				if (cfg::dat.bShowLocalTime)
					SetTimer(hwnd, TIMERID_REFRESH, 65000, nullptr);
			}
			else
				dat->bisEmbedded = TRUE;
		}
		break;

	case WM_SIZE:
		Clist_EndRename(dat, 1);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		g_clistApi.pfnRecalcScrollBar(hwnd, dat);
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
			uint16_t iExtraImage[EXTRA_ICON_COUNT];
			uint8_t flags = 0;
			if (!Clist_FindItem(hwnd, dat, wParam, &contact))
				memset(iExtraImage, 0xFF, sizeof(iExtraImage));
			else {
				memcpy(iExtraImage, contact->iExtraImage, sizeof(iExtraImage));
				flags = contact->flags;
			}
			Clist_DeleteItemFromTree(hwnd, wParam);
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN || !CLVM_GetContactHiddenStatus(wParam, nullptr, dat)) {
				g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
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
			dat->bNeedsResort = true;
			PostMessage(hwnd, INTM_SORTCLC, 0, 1);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_ICONCHANGED:
		{
			int recalcScrollBar = 0;
			MCONTACT hContact = wParam;
			uint16_t status = ID_STATUS_OFFLINE;
			int  contactRemoved = 0;
			MCONTACT hSelItem = NULL;
			ClcContact *selcontact = nullptr;

			char *szProto = Proto_GetBaseAccountName(hContact);
			if (szProto == nullptr)
				status = ID_STATUS_OFFLINE;
			else
				status = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);

			int shouldShow = (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN ||
				!CLVM_GetContactHiddenStatus(hContact, szProto, dat)) && ((cfg::dat.bFilterEffective ? TRUE : !Clist_IsHiddenMode(dat, status)) ||
				Clist_GetContactIcon(hContact) != lParam); // XXX CLVM changed - this means an offline msg is flashing, so the contact should be shown

			if (!Clist_FindItem(hwnd, dat, hContact, &contact, &group)) {
				if (shouldShow && db_is_contact(wParam)) {
					if (dat->selection >= 0 && g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
						hSelItem = Clist_ContactToHItem(selcontact);
					g_clistApi.pfnAddContactToTree(hwnd, dat, hContact, 0, 0);
					recalcScrollBar = 1;
					Clist_FindItem(hwnd, dat, hContact, &contact);
					if (contact) {
						contact->iImage = (uint16_t)lParam;
						Clist_NotifyNewContact(hwnd, hContact);
					}
				}
			}
			else {
				// item in list already
				uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
				if (contact->iImage == (uint16_t)lParam)
					break;
				if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline || cfg::dat.bFilterEffective)) {        // CLVM changed
					if (dat->selection >= 0 && g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
						hSelItem = Clist_ContactToHItem(selcontact);
					Clist_RemoveItemFromGroup(hwnd, group, contact, 0);
					contactRemoved = TRUE;
					recalcScrollBar = 1;
				}
				else {
					contact->iImage = (uint16_t)lParam;
					if (!Clist_IsHiddenMode(dat, status))
						contact->flags |= CONTACTF_ONLINE;
					else
						contact->flags &= ~CONTACTF_ONLINE;
				}
			}
			if (hSelItem) {
				ClcGroup *selgroup;
				if (Clist_FindItem(hwnd, dat, hSelItem, &selcontact, &selgroup))
					dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
				else
					dat->selection = -1;
			}
			dat->bNeedsResort = true;
			PostMessage(hwnd, INTM_SORTCLC, 0, recalcScrollBar);
			PostMessage(hwnd, INTM_INVALIDATE, 0, contactRemoved ? 0 : wParam);
			if (recalcScrollBar)
				g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_METACHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;

		if (contact->bIsMeta && !(cfg::dat.dwFlags & CLUI_USEMETAICONS)) {
			contact->hSubContact = db_mc_getMostOnline(contact->hContact);
			contact->metaProto = Proto_GetBaseAccountName(contact->hSubContact);
			contact->iImage = Clist_GetContactIcon(contact->hSubContact);
			if (contact->pExtra) {
				TExtraCache *pSub = cfg::getCache(contact->hSubContact, contact->metaProto);
				ClcContact *subContact;
				if (!Clist_FindItem(hwnd, dat, contact->hSubContact, &subContact))
					break;

				contact->pExtra->proto_status_item = GetProtocolStatusItem(contact->metaProto);
				if (pSub) {
					contact->pExtra->status_item = pSub->status_item;
					memcpy(contact->iExtraImage, subContact->iExtraImage, sizeof(contact->iExtraImage));
				}
			}
		}
		SendMessage(hwnd, INTM_NAMEORDERCHANGED, wParam, lParam);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_METACHANGEDEVENT:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;
		if (lParam == 0)
			Clist_InitAutoRebuild(hwnd);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_NAMECHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;
		mir_wstrncpy(contact->szText, Clist_GetContactDisplayName(wParam), _countof(contact->szText));

		RTL_DetectAndSet(contact, 0);

		dat->bNeedsResort = true;
		PostMessage(hwnd, INTM_SORTCLC, 0, 0);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_CODEPAGECHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;

		contact->codePage = db_get_dw(wParam, "Tab_SRMsg", "ANSIcodepage", db_get_dw(wParam, "UserInfo", "ANSIcodepage", CP_ACP));
		PostMessage(hwnd, INTM_INVALIDATE, 0, 0);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_AVATARCHANGED:
		contact = nullptr;
		{
			AVATARCACHEENTRY *cEntry = (struct AVATARCACHEENTRY *)lParam;

			if (wParam == 0) {
				//RemoveFromImgCache(0, cEntry);
				cfg::dat.bForceRefetchOnPaint = TRUE;
				RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
				cfg::dat.bForceRefetchOnPaint = FALSE;
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}

			if (!Clist_FindItem(hwnd, dat, wParam, &contact))
				return 0;

			contact->ace = cEntry;
			if (cEntry == nullptr)
				contact->cFlags &= ~ECF_AVATAR;
			else {
				uint32_t dwFlags;

				if (contact->pExtra)
					dwFlags = contact->pExtra->dwDFlags;
				else
					dwFlags = g_plugin.getDword(contact->hContact, "CLN_Flags");
				if (cfg::dat.dwFlags & CLUI_FRAME_AVATARS)
					contact->cFlags = (dwFlags & ECF_HIDEAVATAR ? contact->cFlags & ~ECF_AVATAR : contact->cFlags | ECF_AVATAR);
				else
					contact->cFlags = (dwFlags & ECF_FORCEAVATAR ? contact->cFlags | ECF_AVATAR : contact->cFlags & ~ECF_AVATAR);
			}
			PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)contact->hContact);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_STATUSMSGCHANGED:
		{
			TExtraCache *p;
			char *szProto = nullptr;

			if (!Clist_FindItem(hwnd, dat, wParam, &contact))
				p = cfg::getCache(wParam, nullptr);
			else {
				p = contact->pExtra;
				szProto = contact->pce->szProto;
			}
			GetCachedStatusMsg(p, szProto);
			PostMessage(hwnd, INTM_INVALIDATE, 0, contact ? contact->hContact : 0);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_STATUSCHANGED:
		if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
			uint16_t wStatus = db_get_w(wParam, contact->pce->szProto, "Status", ID_STATUS_OFFLINE);
			if (cfg::dat.bNoOfflineAvatars && wStatus != ID_STATUS_OFFLINE && contact->wStatus == ID_STATUS_OFFLINE) {
				contact->wStatus = wStatus;
				if (cfg::dat.bAvatarServiceAvail && contact->ace == nullptr)
					LoadAvatarForContact(contact);
			}
			contact->wStatus = wStatus;
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		break;

	case INTM_PROTOCHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;

		contact->pce->szProto = Proto_GetBaseAccountName(wParam);
		g_clistApi.pfnInvalidateDisplayNameCacheEntry(wParam);
		mir_wstrncpy(contact->szText, Clist_GetContactDisplayName(wParam), _countof(contact->szText));

		RTL_DetectAndSet(contact, 0);

		dat->bNeedsResort = true;
		PostMessage(hwnd, INTM_SORTCLC, 0, 0);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_INVALIDATE:
		if (!dat->bNeedPaint) {
			KillTimer(hwnd, TIMERID_PAINT);
			SetTimer(hwnd, TIMERID_PAINT, 100, nullptr);
			dat->bNeedPaint = true;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_FORCESORT:
		dat->bNeedsResort = true;
		return SendMessage(hwnd, INTM_SORTCLC, wParam, lParam);

	case INTM_SORTCLC:
		if (dat->bNeedsResort) {
			g_clistApi.pfnSortCLC(hwnd, dat, TRUE);
			dat->bNeedsResort = false;
		}
		if (lParam)
			g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_IDLECHANGED:
		if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
			char *szProto = (char*)cws->szModule;
			if (szProto == nullptr)
				break;

			contact->flags &= ~CONTACTF_IDLE;
			if (db_get_dw(wParam, szProto, "IdleTS", 0)) {
				contact->flags |= CONTACTF_IDLE;
			}
			PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)contact->hContact);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		break;

	case INTM_XSTATUSCHANGED:
		{
			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
			char *szProto = (char *)cws->szModule;
			MCONTACT hContact = wParam;
			TExtraCache *p;

			if (!Clist_FindItem(hwnd, dat, hContact, &contact)) {
				p = cfg::getCache(hContact, szProto);
				if (!dat->bisEmbedded && szProto) {				// may be a subcontact, forward the xstatus
					MCONTACT hMasterContact = db_mc_tryMeta(hContact);
					if (hMasterContact != hContact)				// avoid recursive call of settings handler
						db_set_b(hMasterContact, META_PROTO, "XStatusId", (uint8_t)db_get_b(hContact, szProto, "XStatusId", 0));
					break;
				}
			}
			else {
				contact->xStatus = db_get_b(hContact, szProto, "XStatusId", 0);
				p = contact->pExtra;
			}

			if (szProto == nullptr)
				break;

			if (contact) {
				if (ProtoServiceExists(szProto, PS_GETADVANCEDSTATUSICON)) {
					int iconId = CallProtoService(szProto, PS_GETADVANCEDSTATUSICON, hContact, 0);
					if (iconId != -1)
						contact->xStatusIcon = iconId >> 16;
				}
			}

			GetCachedStatusMsg(p, szProto);
			PostMessage(hwnd, INTM_INVALIDATE, 0, contact ? contact->hContact : 0);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

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
			if (dat->selection != dat->oldSelection && !dat->bisEmbedded && g_ButtonItems != nullptr) {
				SetDBButtonStates(0);
				dat->oldSelection = dat->selection;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_MOUSEWHEEL:
		dat->bForceScroll = true;
		break;

	case WM_TIMER:
		if (wParam == TIMERID_PAINT) {
			KillTimer(hwnd, TIMERID_PAINT);
			InvalidateRect(hwnd, nullptr, FALSE);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		if (wParam == TIMERID_REFRESH) {
			InvalidateRect(hwnd, nullptr, FALSE);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		break;

	case WM_LBUTTONDBLCLK:
		ReleaseCapture();
		dat->iHotTrack = -1;
		Clist_HideInfoTip(dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);

		uint32_t hitFlags;
		dat->selection = HitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, nullptr, &hitFlags);
		if (hitFlags & CLCHT_ONITEMEXTRA)
			break;

		InvalidateRect(hwnd, nullptr, FALSE);
		if (dat->selection != -1)
			Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
		if (hitFlags & CLCHT_ONAVATAR && cfg::dat.bDblClkAvatars) {
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)contact->hContact, 0);
			return TRUE;
		}
		if (hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMLABEL | CLCHT_ONITEMSPACE)) {
			UpdateWindow(hwnd);
			Clist_DoSelectionDefaultAction(hwnd, dat);
		}
		return TRUE;

	case WM_CONTEXTMENU:
		Clist_EndRename(dat, 1);
		Clist_HideInfoTip(dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);
		if (GetFocus() != hwnd)
			SetFocus(hwnd);
		dat->iHotTrack = -1;
		dat->szQuickSearch[0] = 0;
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (pt.x == -1 && pt.y == -1) {
				dat->selection = g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr);
				if (dat->selection != -1)
					Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
				pt.x = dat->iconXSpace + 15;
				pt.y = RowHeight::getItemTopY(dat, dat->selection) - dat->yScroll + (int)(dat->row_heights[dat->selection] * .7);
				hitFlags = dat->selection == -1 ? CLCHT_NOWHERE : CLCHT_ONITEMLABEL;
			}
			else {
				ScreenToClient(hwnd, &pt);
				dat->selection = HitTest(hwnd, dat, pt.x, pt.y, &contact, nullptr, &hitFlags);
			}
			InvalidateRect(hwnd, nullptr, FALSE);
			if (dat->selection != -1)
				Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
			UpdateWindow(hwnd);

			HMENU hMenu = nullptr;
			if (dat->selection != -1 && hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMCHECK | CLCHT_ONITEMLABEL)) {
				if (contact->type == CLCIT_GROUP) {
					hMenu = Menu_BuildSubGroupMenu(contact->group);
					ClientToScreen(hwnd, &pt);
					TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_clistApi.hwndContactList, nullptr);
					CheckMenuItem(hMenu, POPUP_GROUPHIDEOFFLINE, contact->group->hideOffline ? MF_CHECKED : MF_UNCHECKED);
					DestroyMenu(hMenu);
					return 0;
				}
				else if (contact->type == CLCIT_CONTACT)
					hMenu = Menu_BuildContactMenu(contact->hContact);
			}
			else {
				//call parent for new group/hide offline menu
				PostMessage(GetParent(hwnd), WM_CONTEXTMENU, wParam, lParam);
				return 0;
			}
			if (hMenu != nullptr) {
				ClientToScreen(hwnd, &pt);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
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
