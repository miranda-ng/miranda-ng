/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include "jabber_list.h"

static MWindowList hUserInfoList = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserInfoDlgProc - main user info dialog

struct JabberUserInfoDlgData
{
	MCONTACT hContact;
	CJabberProto *ppro;
	JABBER_LIST_ITEM *item;
	int resourcesCount;
};

enum
{
	INFOLINE_DELETE = 0x80000000,
	INFOLINE_MASK = 0x7fffffff,
	INFOLINE_BAD_ID = 0x7fffffff,

	INFOLINE_NAME = 1,
	INFOLINE_MOOD,
	INFOLINE_ACTIVITY,
	INFOLINE_TUNE,
	INFOLINE_OFFLINE,
	INFOLINE_MESSAGE,
	INFOLINE_SOFTWARE,
	INFOLINE_VERSION,
	INFOLINE_SYSTEM,
	INFOLINE_PRIORITY,
	INFOLINE_IDLE,
	INFOLINE_CAPS,
	INFOLINE_SOFTWARE_INFORMATION,
	INFOLINE_SUBSCRIPTION,
	INFOLINE_LOGOFF,
	INFOLINE_LOGOFF_MSG,
	INFOLINE_LASTACTIVE,
};

__forceinline uint32_t sttInfoLineId(uint32_t res, uint32_t type, uint32_t line = 0)
{
	return
		(type << 24) & 0x7f000000 |
		(res << 12) & 0x00fff000 |
		(line) & 0x00000fff;
}

static HTREEITEM sttFindInfoLine(HWND hwndTree, HTREEITEM htiRoot, LPARAM id = INFOLINE_BAD_ID)
{
	if (id == INFOLINE_BAD_ID) return nullptr;
	for (HTREEITEM hti = TreeView_GetChild(hwndTree, htiRoot); hti; hti = TreeView_GetNextSibling(hwndTree, hti)) {
		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hti;
		TreeView_GetItem(hwndTree, &tvi);
		if ((tvi.lParam&INFOLINE_MASK) == (id&INFOLINE_MASK))
			return hti;
	}
	return nullptr;
}

void sttCleanupInfo(HWND hwndTree, int stage)
{
	HTREEITEM hItem = TreeView_GetRoot(hwndTree);
	while (hItem) {
		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hItem;
		TreeView_GetItem(hwndTree, &tvi);

		switch (stage) {
		case 0:
			tvi.lParam |= INFOLINE_DELETE;
			TreeView_SetItem(hwndTree, &tvi);
			break;

		case 1:
			if (tvi.lParam & INFOLINE_DELETE) {
				hItem = TreeView_GetNextSibling(hwndTree, hItem);
				TreeView_DeleteItem(hwndTree, tvi.hItem);
				continue;
			}
			break;
		}

		HTREEITEM hItemTmp = nullptr;
		if (hItemTmp = TreeView_GetChild(hwndTree, hItem))
			hItem = hItemTmp;
		else if (hItemTmp = TreeView_GetNextSibling(hwndTree, hItem))
			hItem = hItemTmp;
		else {
			while (1) {
				if (!(hItem = TreeView_GetParent(hwndTree, hItem))) break;
				if (hItemTmp = TreeView_GetNextSibling(hwndTree, hItem)) {
					hItem = hItemTmp;
					break;
				}
			}
		}
	}
}

static HTREEITEM sttFillInfoLine(HWND hwndTree, HTREEITEM htiRoot, HICON hIcon, const wchar_t *title, const char *value, LPARAM id = INFOLINE_BAD_ID, bool expand = false)
{
	HTREEITEM hti = sttFindInfoLine(hwndTree, htiRoot, id);

	Utf2T wszValue(value);
	const wchar_t *pwszValue = (value == nullptr) ? TranslateT("<not specified>") : wszValue;
	wchar_t buf[256];
	if (title)
		mir_snwprintf(buf, L"%s: %s", title, pwszValue);
	else
		mir_wstrncpy(buf, pwszValue, _countof(buf));

	TVINSERTSTRUCT tvis = {};
	tvis.hParent = htiRoot;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.itemex.pszText = buf;
	tvis.itemex.lParam = id;

	if (hIcon) {
		HIMAGELIST himl = TreeView_GetImageList(hwndTree, TVSIL_NORMAL);
		tvis.itemex.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvis.itemex.iImage =
			tvis.itemex.iSelectedImage = ImageList_AddIcon(himl, hIcon);
		IcoLib_ReleaseIcon(hIcon);
	}

	if (hti) {
		tvis.itemex.mask |= TVIF_HANDLE;
		tvis.itemex.hItem = hti;
		TreeView_SetItem(hwndTree, &tvis.itemex);
	}
	else {
		tvis.itemex.mask |= TVIF_STATE;
		tvis.itemex.stateMask = TVIS_EXPANDED;
		tvis.itemex.state = expand ? TVIS_EXPANDED : 0;
		hti = TreeView_InsertItem(hwndTree, &tvis);
	}

	return hti;
}

static void sttFillResourceInfo(CJabberProto *ppro, HWND hwndTree, HTREEITEM htiRoot, JABBER_LIST_ITEM *item, int resource)
{
	HTREEITEM htiResource = htiRoot;
	pResourceStatus r = resource ? item->arResources[resource - 1] : item->getTemp();

	if (r->m_szResourceName && *r->m_szResourceName)
		htiResource = sttFillInfoLine(hwndTree, htiRoot, Skin_LoadProtoIcon(ppro->m_szModuleName, r->m_iStatus),
			TranslateT("Resource"), r->m_szResourceName, sttInfoLineId(resource, INFOLINE_NAME), true);

	// StatusMsg
	sttFillInfoLine(hwndTree, htiResource, nullptr /*Skin_LoadIcon(SKINICON_EVENT_MESSAGE)*/,
		TranslateT("Message"), r->m_szStatusMessage,
		sttInfoLineId(resource, INFOLINE_MESSAGE));

	// Software
	if (CJabberClientPartialCaps *pCaps = r->m_pCaps) {
		HICON hIcon = nullptr;

		if (ServiceExists(MS_FP_GETCLIENTICONT)) {
			if (pCaps->GetSoft()) {
				wchar_t buf[256];
				mir_snwprintf(buf, L"%s %s", pCaps->GetSoft(), pCaps->GetSoftVer());
				hIcon = Finger_GetClientIcon(buf, 0);
			}
		}

		sttFillInfoLine(hwndTree, htiResource, hIcon, TranslateT("Software"), pCaps->GetSoft(), sttInfoLineId(resource, INFOLINE_SOFTWARE));

		// Version
		sttFillInfoLine(hwndTree, htiResource, nullptr, TranslateT("Version"), pCaps->GetSoftMir() ? pCaps->GetSoftMir() : pCaps->GetSoftVer(), sttInfoLineId(resource, INFOLINE_VERSION));

		// System
		sttFillInfoLine(hwndTree, htiResource, nullptr, TranslateT("System"), pCaps->GetOsVer() ? pCaps->GetOsVer() : pCaps->GetOs(), sttInfoLineId(resource, INFOLINE_SYSTEM));

		if (hIcon)
			DestroyIcon(hIcon);
	}

	// Resource priority
	char buf[256];
	itoa(r->m_iPriority, buf, 10);
	sttFillInfoLine(hwndTree, htiResource, nullptr, TranslateT("Resource priority"), buf, sttInfoLineId(resource, INFOLINE_PRIORITY));

	// Idle
	if (r->m_dwIdleStartTime != -1) {
		if (r->m_dwIdleStartTime != 0) {
			mir_strncpy(buf, ctime(&r->m_dwIdleStartTime), _countof(buf));
			size_t len = mir_strlen(buf);
			if (len > 0)
				buf[len - 1] = 0;
		}
		else mir_strncpy(buf, TranslateU("<currently online>"), _countof(buf));

		sttFillInfoLine(hwndTree, htiResource, nullptr, TranslateT("Last activity"), buf, sttInfoLineId(resource, INFOLINE_IDLE));
	}

	// caps
	JabberCapsBits jcb = ppro->GetResourceCapabilities(MakeJid(item->jid, r->m_szResourceName), r);
	if (!(jcb & JABBER_RESOURCE_CAPS_ERROR)) {
		HTREEITEM htiCaps = sttFillInfoLine(hwndTree, htiResource, IcoLib_GetIconByHandle(ppro->m_hProtoIcon), nullptr, TranslateU("Client capabilities"), sttInfoLineId(resource, INFOLINE_CAPS));
		int i;
		for (i = 0; i < g_cJabberFeatCapPairs; i++)
			if (jcb & g_JabberFeatCapPairs[i].jcbCap) {
				char szDescription[1024];
				if (g_JabberFeatCapPairs[i].tszDescription)
					mir_snprintf(szDescription, "%s (%s)", TranslateU(g_JabberFeatCapPairs[i].tszDescription), g_JabberFeatCapPairs[i].szFeature);
				else
					strncpy_s(szDescription, g_JabberFeatCapPairs[i].szFeature, _TRUNCATE);
				sttFillInfoLine(hwndTree, htiCaps, nullptr, nullptr, szDescription, sttInfoLineId(resource, INFOLINE_CAPS, i));
			}

		for (auto &it : ppro->m_lstJabberFeatCapPairsDynamic) {
			if (jcb & it->jcbCap) {
				char szDescription[1024];
				if (it->szDescription)
					mir_snprintf(szDescription, "%s (%s)", TranslateU(it->szDescription), it->szFeature);
				else
					strncpy_s(szDescription, it->szFeature, _TRUNCATE);
				sttFillInfoLine(hwndTree, htiCaps, nullptr, nullptr, szDescription, sttInfoLineId(resource, INFOLINE_CAPS, i++));
			}
		}
	}

	// Software info
	HTREEITEM htiSoftwareInfo = sttFillInfoLine(hwndTree, htiResource, IcoLib_GetIconByHandle(ppro->m_hProtoIcon), nullptr, TranslateU("Software information"), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION));
	int nLineId = 0;
	if (CJabberClientPartialCaps *pCaps = r->m_pCaps) {
		if (pCaps->GetOs())
			sttFillInfoLine(hwndTree, htiSoftwareInfo, nullptr, TranslateT("Operating system"), pCaps->GetOs(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (pCaps->GetOsVer())
			sttFillInfoLine(hwndTree, htiSoftwareInfo, nullptr, TranslateT("Operating system version"), pCaps->GetOsVer(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (pCaps->GetSoft())
			sttFillInfoLine(hwndTree, htiSoftwareInfo, nullptr, TranslateT("Software"), pCaps->GetSoft(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (pCaps->GetSoftVer())
			sttFillInfoLine(hwndTree, htiSoftwareInfo, nullptr, TranslateT("Software version"), pCaps->GetSoftVer(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (pCaps->GetSoftMir())
			sttFillInfoLine(hwndTree, htiSoftwareInfo, nullptr, TranslateT("Miranda core version"), pCaps->GetSoftMir(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
	}
}

static void sttFillAdvStatusInfo(CJabberProto *ppro, HWND hwndTree, HTREEITEM htiRoot, uint32_t dwInfoLine, MCONTACT hContact, wchar_t *szTitle, char *pszSlot)
{
	char *szAdvStatusIcon = ppro->ReadAdvStatusA(hContact, pszSlot, ADVSTATUS_VAL_ICON);
	wchar_t *szAdvStatusTitle = ppro->ReadAdvStatusT(hContact, pszSlot, ADVSTATUS_VAL_TITLE);
	wchar_t *szAdvStatusText = ppro->ReadAdvStatusT(hContact, pszSlot, ADVSTATUS_VAL_TEXT);

	if (szAdvStatusIcon && szAdvStatusTitle && *szAdvStatusTitle) {
		wchar_t szText[2048];
		if (szAdvStatusText && *szAdvStatusText)
			mir_snwprintf(szText, L"%s (%s)", TranslateW(szAdvStatusTitle), szAdvStatusText);
		else
			wcsncpy_s(szText, TranslateW(szAdvStatusTitle), _TRUNCATE);
		sttFillInfoLine(hwndTree, htiRoot, IcoLib_GetIcon(szAdvStatusIcon), szTitle, T2Utf(szText), dwInfoLine);
	}

	mir_free(szAdvStatusIcon);
	mir_free(szAdvStatusTitle);
	mir_free(szAdvStatusText);
}

static void sttFillUserInfo(CJabberProto *ppro, HWND hwndTree, JABBER_LIST_ITEM *item)
{
	SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0);

	sttCleanupInfo(hwndTree, 0);

	HTREEITEM htiRoot = sttFillInfoLine(hwndTree, nullptr, IcoLib_GetIconByHandle(ppro->m_hProtoIcon), L"JID", item->jid, sttInfoLineId(0, INFOLINE_NAME), true);

	if (MCONTACT hContact = ppro->HContactFromJID(item->jid)) {
		sttFillAdvStatusInfo(ppro, hwndTree, htiRoot, sttInfoLineId(0, INFOLINE_MOOD), hContact, TranslateT("Mood"), ADVSTATUS_MOOD);
		sttFillAdvStatusInfo(ppro, hwndTree, htiRoot, sttInfoLineId(0, INFOLINE_ACTIVITY), hContact, TranslateT("Activity"), ADVSTATUS_ACTIVITY);
		sttFillAdvStatusInfo(ppro, hwndTree, htiRoot, sttInfoLineId(0, INFOLINE_TUNE), hContact, TranslateT("Tune"), ADVSTATUS_TUNE);
	}

	// subscription
	switch (item->subscription) {
	case SUB_BOTH:
		sttFillInfoLine(hwndTree, htiRoot, nullptr, TranslateT("Subscription"), TranslateU("both"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	case SUB_TO:
		sttFillInfoLine(hwndTree, htiRoot, nullptr, TranslateT("Subscription"), TranslateU("to"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	case SUB_FROM:
		sttFillInfoLine(hwndTree, htiRoot, nullptr, TranslateT("Subscription"), TranslateU("from"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	default:
		sttFillInfoLine(hwndTree, htiRoot, nullptr, TranslateT("Subscription"), TranslateU("none"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	}

	// logoff
	char buf[256];
	JABBER_RESOURCE_STATUS *r = item->getTemp();
	if (r->m_dwIdleStartTime != -1) {
		if (r->m_dwIdleStartTime > 0) {
			mir_strncpy(buf, ctime(&r->m_dwIdleStartTime), _countof(buf));
			size_t len = mir_strlen(buf);
			if (len > 0)
				buf[len - 1] = 0;
		}
		else mir_strncpy(buf, TranslateU("<currently online>"), _countof(buf));

		sttFillInfoLine(hwndTree, htiRoot, nullptr,
			(item->jid && strchr(item->jid, '@')) ? TranslateT("Last logoff time") : TranslateT("Uptime"), buf,
			sttInfoLineId(0, INFOLINE_LOGOFF));
	}

	if (r->m_szStatusMessage)
		sttFillInfoLine(hwndTree, htiRoot, nullptr, TranslateT("Logoff message"), r->m_szStatusMessage, sttInfoLineId(0, INFOLINE_LOGOFF_MSG));

	// activity
	if (item->m_pLastSeenResource)
		mir_strncpy(buf, item->m_pLastSeenResource->m_szResourceName, _countof(buf));
	else
		mir_strncpy(buf, TranslateU("<no information available>"), _countof(buf));
	sttFillInfoLine(hwndTree, htiRoot, nullptr, TranslateT("Last active resource"), buf, sttInfoLineId(0, INFOLINE_LASTACTIVE));

	// resources
	if (item->arResources.getCount()) {
		for (int i = 0; i < item->arResources.getCount(); i++)
			sttFillResourceInfo(ppro, hwndTree, htiRoot, item, i + 1);
	}
	else if (!strchr(item->jid, '@') || (r->m_iStatus != ID_STATUS_OFFLINE))
		sttFillResourceInfo(ppro, hwndTree, htiRoot, item, 0);

	sttCleanupInfo(hwndTree, 1);
	SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0);

	RedrawWindow(hwndTree, nullptr, nullptr, RDW_INVALIDATE);
}

static void sttGetNodeText(HWND hwndTree, HTREEITEM hti, CMStringW &buf, int indent = 0)
{
	for (int i = 0; i < indent; i++)
		buf.AppendChar('\t');

	wchar_t wszText[256];
	TVITEMEX tvi = { 0 };
	tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
	tvi.hItem = hti;
	tvi.cchTextMax = _countof(wszText);
	tvi.pszText = wszText;
	if (!TreeView_GetItem(hwndTree, &tvi)) // failure, maybe item was removed...
		return;

	buf.Append(wszText);
	buf.Append(L"\r\n");

	if (tvi.state & TVIS_EXPANDED)
		for (hti = TreeView_GetChild(hwndTree, hti); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
			sttGetNodeText(hwndTree, hti, buf, indent + 1);
}

static INT_PTR CALLBACK JabberUserInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberUserInfoDlgData *dat = (JabberUserInfoDlgData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	RECT rc;

	switch (msg) {
	case WM_INITDIALOG:
		// lParam is hContact
		TranslateDialogDefault(hwndDlg);

		Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_OTHER_USERDETAILS);

		dat = (JabberUserInfoDlgData *)mir_alloc(sizeof(JabberUserInfoDlgData));
		memset(dat, 0, sizeof(JabberUserInfoDlgData));
		dat->resourcesCount = -1;
		dat->hContact = lParam;

		GetClientRect(hwndDlg, &rc);
		MoveWindow(GetDlgItem(hwndDlg, IDC_TV_INFO), 5, 5, rc.right - 10, rc.bottom - 10, TRUE);
		{
			HIMAGELIST himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR | ILC_COLOR32 | ILC_MASK, 5, 1);
			ImageList_AddSkinIcon(himl, SKINICON_OTHER_SMALLDOT);
			TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TV_INFO), himl, TVSIL_NORMAL);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			WindowList_Add(hUserInfoList, hwndDlg, dat->hContact);
		}
		break;

	case WM_PROTO_CHECK_ONLINE:
		if (dat && dat->ppro)
			if (!dat->ppro->m_bJabberOnline) {
				dat->item = nullptr;
				break;
			}
		__fallthrough;

	case WM_PROTO_REFRESH:
		if (dat == nullptr) break;

		if (dat->item == nullptr) {
			ptrA jid(dat->ppro->getUStringA(dat->hContact, "jid"));
			if (jid == nullptr)
				break;

			if (dat->ppro->m_bJabberOnline)
				if (!(dat->item = dat->ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid)))
					dat->item = dat->ppro->ListGetItemPtr(LIST_ROSTER, jid);

			if (dat->item == nullptr) {
				HWND hwndTree = GetDlgItem(hwndDlg, IDC_TV_INFO);
				TreeView_DeleteAllItems(hwndTree);
				HTREEITEM htiRoot = sttFillInfoLine(hwndTree, nullptr, IcoLib_GetIconByHandle(dat->ppro->m_hProtoIcon), L"JID", jid, sttInfoLineId(0, INFOLINE_NAME), true);
				sttFillInfoLine(hwndTree, htiRoot, g_plugin.getIcon(IDI_VCARD), nullptr, TranslateU("Please switch online to see more details."));
				break;
			}
		}
		sttFillUserInfo(dat->ppro, GetDlgItem(hwndDlg, IDC_TV_INFO), dat->item);
		break;

	case WM_SIZE:
		MoveWindow(GetDlgItem(hwndDlg, IDC_TV_INFO), 5, 5, LOWORD(lParam) - 10, HIWORD(lParam) - 10, TRUE);
		break;

	case WM_CONTEXTMENU:
		if (GetWindowLongPtr((HWND)wParam, GWL_ID) == IDC_TV_INFO) {
			HWND hwndTree = GetDlgItem(hwndDlg, IDC_TV_INFO);
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			HTREEITEM hItem = nullptr;

			if ((pt.x == -1) && (pt.y == -1)) {
				if (hItem = TreeView_GetSelection(hwndTree)) {
					TreeView_GetItemRect(hwndTree, hItem, &rc, TRUE);
					pt.x = rc.left;
					pt.y = rc.bottom;
					ClientToScreen(hwndTree, &pt);
				}
			}
			else {
				TVHITTESTINFO tvhti = { 0 };
				tvhti.pt = pt;
				ScreenToClient(hwndTree, &tvhti.pt);
				TreeView_HitTest(hwndTree, &tvhti);
				if (tvhti.flags & TVHT_ONITEM) {
					hItem = tvhti.hItem;
					TreeView_Select(hwndTree, hItem, TVGN_CARET);
				}
			}

			if (hItem) {
				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)1, TranslateT("Copy"));
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)2, TranslateT("Copy only this value"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));
				int nReturnCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, nullptr);
				if (nReturnCmd == 1) {
					CMStringW buf;
					sttGetNodeText(hwndTree, hItem, buf);
					JabberCopyText(hwndDlg, buf);
				}
				else if (nReturnCmd == 2) {
					wchar_t szBuffer[1024];
					TVITEMEX tvi = { 0 };
					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
					tvi.hItem = hItem;
					tvi.cchTextMax = _countof(szBuffer);
					tvi.pszText = szBuffer;
					if (TreeView_GetItem(hwndTree, &tvi)) {
						if (wchar_t *str = wcsstr(szBuffer, L": "))
							JabberCopyText(hwndDlg, str + 2);
						else
							JabberCopyText(hwndDlg, szBuffer);
					}
				}
				DestroyMenu(hMenu);
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_INFOCHANGED:
				SendMessage(hwndDlg, WM_PROTO_REFRESH, 0, ((LPPSHNOTIFY)lParam)->lParam); // hContact
				break;

			case PSN_PARAMCHANGED:
				dat->ppro = (CJabberProto*)((PSHNOTIFY*)lParam)->lParam;
				if (dat->hContact != 0) {
					ptrA jid(dat->ppro->getUStringA(dat->hContact, "jid"));
					if (jid != nullptr)
						if (!(dat->item = dat->ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid)))
							dat->item = dat->ppro->ListGetItemPtr(LIST_ROSTER, jid);
				}

				dat->ppro->WindowSubscribe(hwndDlg);
				break;
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		if (dat) {
			dat->ppro->WindowUnsubscribe(hwndDlg);
			mir_free(dat);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		}
		WindowList_Remove(hUserInfoList, hwndDlg);
		ImageList_Destroy(TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TV_INFO), nullptr, TVSIL_NORMAL));
		Window_FreeIcon_IcoLib(hwndDlg);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserPhotoDlgProc - Jabber photo dialog

struct USER_PHOTO_INFO
{
	MCONTACT hContact;
	HBITMAP hBitmap;
	CJabberProto *ppro;
};

static INT_PTR CALLBACK JabberUserPhotoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	USER_PHOTO_INFO *photoInfo = (USER_PHOTO_INFO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		// lParam is hContact
		TranslateDialogDefault(hwndDlg);
		photoInfo = (USER_PHOTO_INFO *)mir_alloc(sizeof(USER_PHOTO_INFO));
		photoInfo->hContact = lParam;
		photoInfo->ppro = nullptr;
		photoInfo->hBitmap = nullptr;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)photoInfo);
		Button_SetIcon_IcoLib(hwndDlg, IDC_SAVE, g_plugin.getIconHandle(IDI_SAVE));
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOAD), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_DELETE), SW_HIDE);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_INFOCHANGED:
				SendMessage(hwndDlg, WM_PROTO_REFRESH, 0, 0);
				break;

			case PSN_PARAMCHANGED:
				photoInfo->ppro = (CJabberProto*)((PSHNOTIFY*)lParam)->lParam;
				break;
			}
			break;
		}
		break;

	case WM_PROTO_REFRESH:
		if (photoInfo->hBitmap) {
			DeleteObject(photoInfo->hBitmap);
			photoInfo->hBitmap = nullptr;
		}
		ShowWindow(GetDlgItem(hwndDlg, IDC_SAVE), SW_HIDE);
		{
			ptrA jid(photoInfo->ppro->getUStringA(photoInfo->hContact, "jid"));
			if (jid != nullptr) {
				JABBER_LIST_ITEM *item = photoInfo->ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid);
				if (item == nullptr)
					item = photoInfo->ppro->ListGetItemPtr(LIST_ROSTER, jid);
				if (item != nullptr) {
					if (item->photoFileName) {
						photoInfo->ppro->debugLogA("Showing picture from %s", item->photoFileName);
						photoInfo->hBitmap = Bitmap_Load(Utf2T(item->photoFileName));
						FreeImage_Premultiply(photoInfo->hBitmap);
						ShowWindow(GetDlgItem(hwndDlg, IDC_SAVE), SW_SHOW);
					}
				}
			}
		}
		InvalidateRect(hwndDlg, nullptr, TRUE);
		UpdateWindow(hwndDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SAVE:
			static wchar_t szFilter[512];

			ptrA jid(photoInfo->ppro->getUStringA(photoInfo->hContact, "jid"));
			if (jid == nullptr)
				break;

			JABBER_LIST_ITEM *item = photoInfo->ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid);
			if (item == nullptr)
				if ((item = photoInfo->ppro->ListGetItemPtr(LIST_ROSTER, jid)) == nullptr)
					break;

			switch (ProtoGetAvatarFileFormat(Utf2T(item->photoFileName))) {
			case PA_FORMAT_BMP:
				mir_snwprintf(szFilter, L"BMP %s (*.bmp)%c*.BMP", TranslateT("format"), 0);
				break;

			case PA_FORMAT_GIF:
				mir_snwprintf(szFilter, L"GIF %s (*.gif)%c*.GIF", TranslateT("format"), 0);
				break;

			case PA_FORMAT_JPEG:
				mir_snwprintf(szFilter, L"JPEG %s (*.jpg;*.jpeg)%c*.JPG;*.JPEG", TranslateT("format"), 0);
				break;

			default:
				mir_snwprintf(szFilter, L"%s (*.*)%c*.*", TranslateT("Unknown format"), 0);
			}

			wchar_t szFileName[MAX_PATH]; szFileName[0] = '\0';
			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = szFilter;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = _MAX_PATH;
			ofn.Flags = OFN_OVERWRITEPROMPT;
			if (GetSaveFileName(&ofn)) {
				photoInfo->ppro->debugLogW(L"File selected is %s", szFileName);
				CopyFile(Utf2T(item->photoFileName), szFileName, FALSE);
			}
		}
		break;

	case WM_PAINT:
		if (!photoInfo->ppro->m_bJabberOnline)
			SetDlgItemText(hwndDlg, IDC_CANVAS, TranslateT("<Photo not available while offline>"));
		else if (!photoInfo->hBitmap)
			SetDlgItemText(hwndDlg, IDC_CANVAS, TranslateT("<No photo>"));
		else {
			BITMAP bm;
			POINT ptSize, ptOrg, pt, ptFitSize;
			RECT rect;

			SetDlgItemTextA(hwndDlg, IDC_CANVAS, "");
			HBITMAP hBitmap = photoInfo->hBitmap;
			HWND hwndCanvas = GetDlgItem(hwndDlg, IDC_CANVAS);
			HDC hdcCanvas = GetDC(hwndCanvas);
			HDC hdcMem = CreateCompatibleDC(hdcCanvas);
			SelectObject(hdcMem, hBitmap);
			SetMapMode(hdcMem, GetMapMode(hdcCanvas));
			GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
			ptSize.x = bm.bmWidth;
			ptSize.y = bm.bmHeight;
			DPtoLP(hdcCanvas, &ptSize, 1);
			ptOrg.x = ptOrg.y = 0;
			DPtoLP(hdcMem, &ptOrg, 1);
			GetClientRect(hwndCanvas, &rect);
			InvalidateRect(hwndCanvas, nullptr, TRUE);
			UpdateWindow(hwndCanvas);
			if (ptSize.x <= rect.right && ptSize.y <= rect.bottom) {
				pt.x = (rect.right - ptSize.x) / 2;
				pt.y = (rect.bottom - ptSize.y) / 2;
				ptFitSize = ptSize;
			}
			else {
				if (((float)(ptSize.x - rect.right)) / ptSize.x > ((float)(ptSize.y - rect.bottom)) / ptSize.y) {
					ptFitSize.x = rect.right;
					ptFitSize.y = (ptSize.y*rect.right) / ptSize.x;
					pt.x = 0;
					pt.y = (rect.bottom - ptFitSize.y) / 2;
				}
				else {
					ptFitSize.x = (ptSize.x*rect.bottom) / ptSize.y;
					ptFitSize.y = rect.bottom;
					pt.x = (rect.right - ptFitSize.x) / 2;
					pt.y = 0;
				}
			}

			RECT rc;
			if (IsThemeActive()) {
				GetClientRect(hwndCanvas, &rc);
				DrawThemeParentBackground(hwndCanvas, hdcCanvas, &rc);
			}
			else {
				GetClientRect(hwndCanvas, &rc);
				FillRect(hdcCanvas, &rc, (HBRUSH)GetSysColorBrush(COLOR_BTNFACE));
			}

			if (bm.bmBitsPixel == 32) {
				BLENDFUNCTION bf = { 0 };
				bf.AlphaFormat = AC_SRC_ALPHA;
				bf.BlendOp = AC_SRC_OVER;
				bf.SourceConstantAlpha = 255;
				GdiAlphaBlend(hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, bf);
			}
			else {
				SetStretchBltMode(hdcCanvas, COLORONCOLOR);
				StretchBlt(hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, SRCCOPY);
			}

			DeleteDC(hdcMem);
		}
		break;

	case WM_DESTROY:
		Button_FreeIcon_IcoLib(hwndDlg, IDC_SAVE);
		if (!photoInfo)
			break;
		if (photoInfo->hBitmap) {
			photoInfo->ppro->debugLogA("Delete bitmap");
			DeleteObject(photoInfo->hBitmap);
		}
		mir_free(photoInfo);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnInfoInit - initializes user info option dialogs

int CJabberProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	if (!Proto_GetAccount(m_szModuleName))
		return 0;

	if (hContact == 0) {
		// Show our vcard
		OnUserInfoInit_VCard(wParam, hContact);
		return 0;
	}

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto != nullptr && !mir_strcmp(szProto, m_szModuleName)) {
		OPTIONSDIALOGPAGE odp = {};
		odp.dwInitParam = (LPARAM)this;

		odp.pfnDlgProc = JabberUserInfoDlgProc;
		odp.position = -2000000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_JABBER);
		odp.szTitle.a = LPGEN("Account");
		g_plugin.addUserInfo(wParam, &odp);

		odp.pfnDlgProc = JabberUserPhotoDlgProc;
		odp.position = 2000000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_PHOTO);
		odp.szTitle.a = LPGEN("Photo");
		g_plugin.addUserInfo(wParam, &odp);
	}
	//TODO: add omemo dialog to userinfo
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserInfoUpdate

void JabberUserInfoInit()
{
	hUserInfoList = WindowList_Create();
}

void JabberUserInfoUninit()
{
	WindowList_Destroy(hUserInfoList);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserInfoUpdate

void JabberUserInfoUpdate(MCONTACT hContact)
{
	if (!hContact)
		WindowList_BroadcastAsync(hUserInfoList, WM_PROTO_REFRESH, 0, 0);
	else if (HWND hwnd = WindowList_Find(hUserInfoList, hContact))
		PostMessage(hwnd, WM_PROTO_REFRESH, 0, 0);
}
