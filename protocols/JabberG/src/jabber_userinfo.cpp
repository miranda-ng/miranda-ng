/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2007     Maxim Mluhov
Copyright (C) 2012-13  Miranda NG Project

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

#include "jabber.h"

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include "jabber_list.h"

static HANDLE hUserInfoList = NULL;

struct UserInfoStringBuf
{
	enum { STRINGBUF_INCREMENT = 1024 };

	TCHAR *buf;
	int size;
	int offset;

	UserInfoStringBuf() { buf = 0; size = 0; offset = 0; }
	~UserInfoStringBuf() { mir_free(buf); }

	void append(TCHAR *str) {
		if ( !str) return;

		int length = lstrlen(str);
		if (size - offset < length + 1) {
			size += (length + STRINGBUF_INCREMENT);
			buf = (TCHAR *)mir_realloc(buf, size * sizeof(TCHAR));
		}
		lstrcpy(buf + offset, str);
		offset += length;
	}

	TCHAR *allocate(int length) {
		if (size - offset < length) {
			size += (length + STRINGBUF_INCREMENT);
			buf = (TCHAR *)mir_realloc(buf, size * sizeof(TCHAR));
		}
		return buf + offset;
	}

	void actualize() {
		if (buf) offset = lstrlen(buf);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserInfoDlgProc - main user info dialog

struct JabberUserInfoDlgData
{
	CJabberProto*     ppro;
	HANDLE            hContact;
	JABBER_LIST_ITEM *item;
	int               resourceCount;
};

enum
{
	INFOLINE_DELETE	= 0x80000000,
	INFOLINE_MASK	= 0x7fffffff,
	INFOLINE_BAD_ID	= 0x7fffffff,

	INFOLINE_NAME	= 1,
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

__forceinline DWORD sttInfoLineId(DWORD res, DWORD type, DWORD line=0)
{
	return
		(type << 24) & 0x7f000000 |
		(res  << 12) & 0x00fff000 |
		(line      ) & 0x00000fff;
}

static HTREEITEM sttFindInfoLine(HWND hwndTree, HTREEITEM htiRoot, LPARAM id=INFOLINE_BAD_ID)
{
	if (id == INFOLINE_BAD_ID) return NULL;
	for (HTREEITEM hti = TreeView_GetChild(hwndTree, htiRoot); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
	{
		TVITEMEX tvi = {0};
		tvi.mask = TVIF_HANDLE|TVIF_PARAM;
		tvi.hItem = hti;
		TreeView_GetItem(hwndTree, &tvi);
		if ((tvi.lParam&INFOLINE_MASK) == (id&INFOLINE_MASK))
			return hti;
	}
	return NULL;
}

void sttCleanupInfo(HWND hwndTree, int stage)
{
	HTREEITEM hItem = TreeView_GetRoot(hwndTree);
	while (hItem) {
		TVITEMEX tvi = {0};
		tvi.mask = TVIF_HANDLE|TVIF_PARAM;
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

		HTREEITEM hItemTmp = 0;
		if (hItemTmp = TreeView_GetChild(hwndTree, hItem))
			hItem = hItemTmp;
		else if (hItemTmp = TreeView_GetNextSibling(hwndTree, hItem))
			hItem = hItemTmp;
		else {
			while (1) {
				if ( !(hItem = TreeView_GetParent(hwndTree, hItem))) break;
				if (hItemTmp = TreeView_GetNextSibling(hwndTree, hItem)) {
					hItem = hItemTmp;
					break;
				}
			}
		}
	}
}

static HTREEITEM sttFillInfoLine(HWND hwndTree, HTREEITEM htiRoot, HICON hIcon, TCHAR *title, TCHAR *value, LPARAM id=INFOLINE_BAD_ID, bool expand=false)
{
	HTREEITEM hti = sttFindInfoLine(hwndTree, htiRoot, id);

	TCHAR buf[256];
	if (title)
		mir_sntprintf(buf, SIZEOF(buf), _T("%s: %s"), title, value);
	else
		lstrcpyn(buf, value, SIZEOF(buf));

	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = htiRoot;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT|TVIF_PARAM;
	tvis.itemex.pszText = buf;
	tvis.itemex.lParam = id;

	if (hIcon) {
		HIMAGELIST himl = TreeView_GetImageList(hwndTree, TVSIL_NORMAL);
		tvis.itemex.mask |= TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		tvis.itemex.iImage =
		tvis.itemex.iSelectedImage = ImageList_AddIcon(himl, hIcon);
		g_ReleaseIcon(hIcon);
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
	TCHAR buf[256];
	HTREEITEM htiResource = htiRoot;
	JABBER_RESOURCE_STATUS *res = resource ? &item->pResources[resource-1] : &item->itemResource;

	if (res->resourceName && *res->resourceName)
		htiResource = sttFillInfoLine(hwndTree, htiRoot, LoadSkinnedProtoIcon(ppro->m_szModuleName, res->status),
			TranslateT("Resource"), res->resourceName, sttInfoLineId(resource, INFOLINE_NAME), true);

	// StatusMsg
	sttFillInfoLine(hwndTree, htiResource, NULL /*LoadSkinnedIcon(SKINICON_EVENT_MESSAGE)*/,
		TranslateT("Message"), res->statusMessage ? res->statusMessage : TranslateT("<not specified>"),
		sttInfoLineId(resource, INFOLINE_MESSAGE));

	// Software
	HICON hIcon = NULL;
	if (ServiceExists(MS_FP_GETCLIENTICONT)) {
		if (res->software != NULL) {
			mir_sntprintf(buf, SIZEOF(buf), _T("%s %s"), res->software, res->version);
			hIcon = (HICON)CallService(MS_FP_GETCLIENTICONT, (WPARAM)buf, 0);
	}	}

	sttFillInfoLine(hwndTree, htiResource, hIcon, TranslateT("Software"),
		res->software ? res->software : TranslateT("<not specified>"),
		sttInfoLineId(resource, INFOLINE_SOFTWARE));

	if (hIcon)
		DestroyIcon(hIcon);

	// Version
	sttFillInfoLine(hwndTree, htiResource, NULL, TranslateT("Version"),
		res->version ? res->version : TranslateT("<not specified>"),
		sttInfoLineId(resource, INFOLINE_VERSION));

	// System
	sttFillInfoLine(hwndTree, htiResource, NULL, TranslateT("System"),
		res->system ? res->system : TranslateT("<not specified>"),
		sttInfoLineId(resource, INFOLINE_SYSTEM));

	// Resource priority
	TCHAR szPriority[128];
	mir_sntprintf(szPriority, SIZEOF(szPriority), _T("%d"), (int)res->priority);
	sttFillInfoLine(hwndTree, htiResource, NULL, TranslateT("Resource priority"), szPriority, sttInfoLineId(resource, INFOLINE_PRIORITY));

	// Idle
	if (res->idleStartTime > 0) {
		lstrcpyn(buf, _tctime(&res->idleStartTime), SIZEOF(buf));
		int len = lstrlen(buf);
		if (len > 0) buf[len-1] = 0;
	}
	else if ( !res->idleStartTime)
		lstrcpyn(buf, TranslateT("unknown"), SIZEOF(buf));
	else
		lstrcpyn(buf, TranslateT("<not specified>"), SIZEOF(buf));

	sttFillInfoLine(hwndTree, htiResource, NULL, TranslateT("Idle since"), buf, sttInfoLineId(resource, INFOLINE_IDLE));

	// caps
	mir_sntprintf(buf, SIZEOF(buf), _T("%s/%s"), item->jid, res->resourceName);
	JabberCapsBits jcb = ppro->GetResourceCapabilites(buf, TRUE);

	if ( !(jcb & JABBER_RESOURCE_CAPS_ERROR)) {
		HTREEITEM htiCaps = sttFillInfoLine(hwndTree, htiResource, ppro->LoadIconEx("main"), NULL, TranslateT("Client capabilities"), sttInfoLineId(resource, INFOLINE_CAPS));
		int i;
		for (i = 0; g_JabberFeatCapPairs[i].szFeature; i++)
			if (jcb & g_JabberFeatCapPairs[i].jcbCap) {
				TCHAR szDescription[ 1024 ];
				if (g_JabberFeatCapPairs[i].tszDescription)
					mir_sntprintf(szDescription, SIZEOF(szDescription), _T("%s (%s)"), TranslateTS(g_JabberFeatCapPairs[i].tszDescription), g_JabberFeatCapPairs[i].szFeature);
				else
					mir_sntprintf(szDescription, SIZEOF(szDescription), _T("%s"), g_JabberFeatCapPairs[i].szFeature);
				sttFillInfoLine(hwndTree, htiCaps, NULL, NULL, szDescription, sttInfoLineId(resource, INFOLINE_CAPS, i));
			}

		for (int j = 0; j < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); j++, i++)
			if (jcb & ppro->m_lstJabberFeatCapPairsDynamic[j]->jcbCap) {
				TCHAR szDescription[ 1024 ];
				if (ppro->m_lstJabberFeatCapPairsDynamic[j]->szDescription)
					mir_sntprintf(szDescription, SIZEOF(szDescription), _T("%s (%s)"), TranslateTS(ppro->m_lstJabberFeatCapPairsDynamic[j]->szDescription), ppro->m_lstJabberFeatCapPairsDynamic[j]->szFeature);
				else
					mir_sntprintf(szDescription, SIZEOF(szDescription), _T("%s"), ppro->m_lstJabberFeatCapPairsDynamic[j]->szFeature);
				sttFillInfoLine(hwndTree, htiCaps, NULL, NULL, szDescription, sttInfoLineId(resource, INFOLINE_CAPS, i));
			}
	}

	// Software info
	if (res->pSoftwareInfo) {
		HTREEITEM htiSoftwareInfo = sttFillInfoLine(hwndTree, htiResource, ppro->LoadIconEx("main"), NULL, TranslateT("Software information"), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION));
		int nLineId = 0;
		JABBER_XEP0232_SOFTWARE_INFO *p = res->pSoftwareInfo;
		if (p->tszOs)
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Operating system"), p->tszOs, sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (p->tszOsVersion)
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Operating system version"), p->tszOsVersion, sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (p->tszSoftware)
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Software"), p->tszSoftware, sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (p->tszSoftwareVersion)
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Software version"), p->tszSoftwareVersion, sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		if (p->tszXMirandaCoreVersion) {
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Miranda NG core version"), p->tszXMirandaCoreVersion, sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Unicode build"), p->bXMirandaIsUnicode ? TranslateT("Yes") : TranslateT("No"), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Alpha build"), p->bXMirandaIsAlpha ? TranslateT("Yes") : TranslateT("No"), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
			sttFillInfoLine(hwndTree, htiSoftwareInfo, NULL, TranslateT("Debug build"), p->bXMirandaIsDebug ? TranslateT("Yes") : TranslateT("No"), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		}
	}
}

static void sttFillAdvStatusInfo(CJabberProto *ppro, HWND hwndTree, HTREEITEM htiRoot, DWORD dwInfoLine, HANDLE hContact, TCHAR *szTitle, char *pszSlot)
{
	char *szAdvStatusIcon = ppro->ReadAdvStatusA(hContact, pszSlot, ADVSTATUS_VAL_ICON);
	TCHAR *szAdvStatusTitle = ppro->ReadAdvStatusT(hContact, pszSlot, ADVSTATUS_VAL_TITLE);
	TCHAR *szAdvStatusText = ppro->ReadAdvStatusT(hContact, pszSlot, ADVSTATUS_VAL_TEXT);

	if (szAdvStatusIcon && szAdvStatusTitle && *szAdvStatusTitle) {
		TCHAR szText[2048];
		if (szAdvStatusText && *szAdvStatusText)
			mir_sntprintf(szText, 2047, _T("%s (%s)"), TranslateTS(szAdvStatusTitle), szAdvStatusText);
		else
			mir_sntprintf(szText, 2047, _T("%s"), TranslateTS(szAdvStatusTitle));
		sttFillInfoLine(hwndTree, htiRoot, Skin_GetIcon(szAdvStatusIcon), szTitle, szText, dwInfoLine);
	}

	mir_free(szAdvStatusIcon);
	mir_free(szAdvStatusTitle);
	mir_free(szAdvStatusText);
}

static void sttFillUserInfo(CJabberProto *ppro, HWND hwndTree, JABBER_LIST_ITEM *item)
{
	SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0);

	sttCleanupInfo(hwndTree, 0);

	HTREEITEM htiRoot = sttFillInfoLine(hwndTree, NULL, ppro->LoadIconEx("main"), _T("JID"), item->jid, sttInfoLineId(0, INFOLINE_NAME), true);
	TCHAR buf[256];

	if (HANDLE hContact = ppro->HContactFromJID(item->jid)) {
		sttFillAdvStatusInfo(ppro, hwndTree, htiRoot, sttInfoLineId(0, INFOLINE_MOOD), hContact, TranslateT("Mood"), ADVSTATUS_MOOD);
		sttFillAdvStatusInfo(ppro, hwndTree, htiRoot, sttInfoLineId(0, INFOLINE_ACTIVITY), hContact, TranslateT("Activity"), ADVSTATUS_ACTIVITY);
		sttFillAdvStatusInfo(ppro, hwndTree, htiRoot, sttInfoLineId(0, INFOLINE_TUNE), hContact, TranslateT("Tune"), ADVSTATUS_TUNE);
	}

	// subscription
	switch (item->subscription) {
	case SUB_BOTH:
		sttFillInfoLine(hwndTree, htiRoot, NULL, TranslateT("Subscription"), TranslateT("both"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	case SUB_TO:
		sttFillInfoLine(hwndTree, htiRoot, NULL, TranslateT("Subscription"), TranslateT("to"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	case SUB_FROM:
		sttFillInfoLine(hwndTree, htiRoot, NULL, TranslateT("Subscription"), TranslateT("from"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	default:
		sttFillInfoLine(hwndTree, htiRoot, NULL, TranslateT("Subscription"), TranslateT("none"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
		break;
	}

	// logoff
	if (item->itemResource.idleStartTime > 0) {
		lstrcpyn(buf, _tctime(&item->itemResource.idleStartTime), SIZEOF(buf));
		int len = lstrlen(buf);
		if (len > 0) buf[len-1] = 0;
	}
	else if ( !item->itemResource.idleStartTime)
		lstrcpyn(buf, TranslateT("unknown"), SIZEOF(buf));
	else
		lstrcpyn(buf, TranslateT("<not specified>"), SIZEOF(buf));

	sttFillInfoLine(hwndTree, htiRoot, NULL,
		(item->jid && _tcschr(item->jid, _T('@'))) ? TranslateT("Last logoff time") : TranslateT("Uptime"), buf,
		sttInfoLineId(0, INFOLINE_LOGOFF));

	// logoff msg
	sttFillInfoLine(hwndTree, htiRoot, NULL, TranslateT("Logoff message"),
		item->itemResource.statusMessage ? item->itemResource.statusMessage : TranslateT("<not specified>"), sttInfoLineId(0, INFOLINE_LOGOFF_MSG));

	// activity
	if (item->pLastSeenResource)
		lstrcpyn(buf, item->pLastSeenResource->resourceName, SIZEOF(buf));
	else
		lstrcpyn(buf, TranslateT("<no information available>"), SIZEOF(buf));

	sttFillInfoLine(hwndTree, htiRoot, NULL, TranslateT("Last active resource"), buf,
		sttInfoLineId(0, INFOLINE_LASTACTIVE));

	// resources
	if (item->resourceCount) {
		for (int i = 0; i < item->resourceCount; i++)
			sttFillResourceInfo(ppro, hwndTree, htiRoot, item, i+1);
	}
	else if ( !_tcschr(item->jid, _T('@')) || (item->itemResource.status != ID_STATUS_OFFLINE))
		sttFillResourceInfo(ppro, hwndTree, htiRoot, item, 0);

	sttCleanupInfo(hwndTree, 1);
	SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0);

	RedrawWindow(hwndTree, NULL, NULL, RDW_INVALIDATE);
}

static void sttGetNodeText(HWND hwndTree, HTREEITEM hti, UserInfoStringBuf *buf, int indent = 0)
{
	for (int i = 0; i < indent; i++)
		buf->append(_T("\t"));

	TVITEMEX tvi = {0};
	tvi.mask = TVIF_HANDLE|TVIF_TEXT|TVIF_STATE;
	tvi.hItem = hti;
	tvi.cchTextMax = 256;
	tvi.pszText = buf->allocate(tvi.cchTextMax);
	if ( !TreeView_GetItem(hwndTree, &tvi)) { // failure, maybe item was removed...
		buf->buf[ buf->offset ] = 0;
		buf->actualize();
		return;
	}

	buf->actualize();
	buf->append(_T("\r\n"));

	if (tvi.state & TVIS_EXPANDED)
		for (hti = TreeView_GetChild(hwndTree, hti); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
			sttGetNodeText(hwndTree, hti, buf, indent + 1);
}

static INT_PTR CALLBACK JabberUserInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberUserInfoDlgData *dat = (JabberUserInfoDlgData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		// lParam is hContact
		TranslateDialogDefault(hwndDlg);

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIconBig(SKINICON_OTHER_USERDETAILS));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS));

		dat = (JabberUserInfoDlgData *)mir_alloc(sizeof(JabberUserInfoDlgData));
		ZeroMemory(dat, sizeof(JabberUserInfoDlgData));
		dat->resourceCount = -1;

		if (CallService(MS_DB_CONTACT_IS, (WPARAM)lParam, 0))
			dat->hContact = (HANDLE)lParam;
		else if ( !IsBadReadPtr((void*)lParam, sizeof(JABBER_LIST_ITEM))) {
			dat->hContact = NULL;
			dat->item = (JABBER_LIST_ITEM *)lParam;
		}

		{
			RECT rc; GetClientRect(hwndDlg, &rc);
			MoveWindow(GetDlgItem(hwndDlg, IDC_TV_INFO), 5, 5, rc.right-10, rc.bottom-10, TRUE);

			HIMAGELIST himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR|ILC_COLOR32|ILC_MASK, 5, 1);
			ImageList_AddIcon_Icolib(himl, LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));
			TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TV_INFO), himl, TVSIL_NORMAL);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			WindowList_Add(hUserInfoList, hwndDlg, dat->hContact);
		}
		break;

	case WM_JABBER_REFRESH:
		if ( !dat) break;

		if ( !dat->item) {
			DBVARIANT dbv = {0};
			if (dat->ppro->getTString(dat->hContact, "jid", &dbv))
				break;

			if ( !(dat->item = dat->ppro->ListGetItemPtr(LIST_VCARD_TEMP, dbv.ptszVal)))
				dat->item = dat->ppro->ListGetItemPtr(LIST_ROSTER, dbv.ptszVal);

			if ( !dat->item)
			{
				HWND hwndTree = GetDlgItem(hwndDlg, IDC_TV_INFO);
				TreeView_DeleteAllItems(hwndTree);
				HTREEITEM htiRoot = sttFillInfoLine(hwndTree, NULL, dat->ppro->LoadIconEx("main"), _T("JID"), dbv.ptszVal, sttInfoLineId(0, INFOLINE_NAME), true);
				sttFillInfoLine(hwndTree, htiRoot, dat->ppro->LoadIconEx("vcard"), NULL,
					TranslateT("Please switch online to see more details."));

				db_free(&dbv);
				break;
			}

			db_free(&dbv);
		}
		sttFillUserInfo(dat->ppro, GetDlgItem(hwndDlg, IDC_TV_INFO), dat->item);
		break;

	case WM_SIZE:
		MoveWindow(GetDlgItem(hwndDlg, IDC_TV_INFO), 5, 5, LOWORD(lParam)-10, HIWORD(lParam)-10, TRUE);
		break;

	case WM_CONTEXTMENU:
		if (GetWindowLongPtr((HWND)wParam, GWL_ID) == IDC_TV_INFO) {
			HWND hwndTree = GetDlgItem(hwndDlg, IDC_TV_INFO);
			POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
			HTREEITEM hItem = 0;

			if ((pt.x == -1) && (pt.y == -1)) {
				if (hItem = TreeView_GetSelection(hwndTree)) {
					RECT rc;
					TreeView_GetItemRect(hwndTree, hItem, &rc, TRUE);
					pt.x = rc.left;
					pt.y = rc.bottom;
					ClientToScreen(hwndTree, &pt);
				}
			}
			else {
				TVHITTESTINFO tvhti = {0};
				tvhti.pt = pt;
				ScreenToClient(hwndTree, &tvhti.pt);
				TreeView_HitTest(hwndTree, &tvhti);
				if (tvhti.flags & TVHT_ONITEM) {
					hItem = tvhti.hItem;
					TreeView_Select(hwndTree, hItem, TVGN_CARET);
			}	}

			if (hItem) {
				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)1, TranslateT("Copy"));
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)2, TranslateT("Copy only this value"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));
				int nReturnCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
				if (nReturnCmd == 1) {
					UserInfoStringBuf buf;
					sttGetNodeText(hwndTree, hItem, &buf);
					JabberCopyText(hwndDlg, buf.buf);
				}
				else if (nReturnCmd == 2) {
					TCHAR szBuffer[ 1024 ];
					TVITEMEX tvi = {0};
					tvi.mask = TVIF_HANDLE|TVIF_TEXT|TVIF_STATE;
					tvi.hItem = hItem;
					tvi.cchTextMax = SIZEOF(szBuffer);
					tvi.pszText = szBuffer;
					if (TreeView_GetItem(hwndTree, &tvi)) {
						if (TCHAR *str = _tcsstr(szBuffer, _T(": ")))
							JabberCopyText(hwndDlg, str+2);
						else
							JabberCopyText(hwndDlg, szBuffer);
				}	}
				DestroyMenu(hMenu);
		}	}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_INFOCHANGED:
				{
					HANDLE hContact = (HANDLE)((LPPSHNOTIFY) lParam)->lParam;
					SendMessage(hwndDlg, WM_JABBER_REFRESH, 0, (LPARAM)hContact);
				}
				break;

			case PSN_PARAMCHANGED:
				dat->ppro = (CJabberProto*)(CJabberProto*)((PSHNOTIFY*)lParam)->lParam;
				if (dat->hContact != NULL) {
					DBVARIANT dbv = {0};
					if (dat->ppro->getTString(dat->hContact, "jid", &dbv))
						break;

					if ( !(dat->item = dat->ppro->ListGetItemPtr(LIST_VCARD_TEMP, dbv.ptszVal)))
						dat->item = dat->ppro->ListGetItemPtr(LIST_ROSTER, dbv.ptszVal);
					db_free(&dbv);
				}
				break;
		}	}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		WindowList_Remove(hUserInfoList, hwndDlg);
		if (dat) {
			mir_free(dat);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		}
		ImageList_Destroy(TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TV_INFO), NULL, TVSIL_NORMAL));
		WindowFreeIcon(hwndDlg);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserPhotoDlgProc - Jabber photo dialog

struct USER_PHOTO_INFO
{
	HANDLE        hContact;
	HBITMAP       hBitmap;
	CJabberProto *ppro;
};

static INT_PTR CALLBACK JabberUserPhotoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	USER_PHOTO_INFO *photoInfo;

	photoInfo = (USER_PHOTO_INFO *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		// lParam is hContact
		TranslateDialogDefault(hwndDlg);
		photoInfo = (USER_PHOTO_INFO *) mir_alloc(sizeof(USER_PHOTO_INFO));
		photoInfo->hContact = (HANDLE)lParam;
		photoInfo->ppro = NULL;
		photoInfo->hBitmap = NULL;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) photoInfo);
		SendDlgItemMessage(hwndDlg, IDC_SAVE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_SAVE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
		SendDlgItemMessage(hwndDlg, IDC_SAVE, BUTTONSETASFLATBTN, TRUE, 0);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOAD), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_DELETE), SW_HIDE);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_INFOCHANGED:
				SendMessage(hwndDlg, WM_JABBER_REFRESH, 0, 0);
				break;

			case PSN_PARAMCHANGED:
				photoInfo->ppro = (CJabberProto*)((PSHNOTIFY*)lParam)->lParam;
				break;
			}
			break;
		}
		break;

	case WM_JABBER_REFRESH:
		{
			JABBER_LIST_ITEM *item;
			DBVARIANT dbv;

			if (photoInfo->hBitmap) {
				DeleteObject(photoInfo->hBitmap);
				photoInfo->hBitmap = NULL;
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_SAVE), SW_HIDE);
			if ( !photoInfo->ppro->getTString(photoInfo->hContact, "jid", &dbv)) {
				TCHAR *jid = dbv.ptszVal;
				if ((item = photoInfo->ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid)) == NULL)
					item = photoInfo->ppro->ListGetItemPtr(LIST_ROSTER, jid);
				if (item != NULL) {
					if (item->photoFileName) {
						photoInfo->ppro->Log("Showing picture from %S", item->photoFileName);
						char* p = mir_t2a(item->photoFileName);
						photoInfo->hBitmap = (HBITMAP) CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)p);
						mir_free(p);
						JabberBitmapPremultiplyChannels(photoInfo->hBitmap);
						ShowWindow(GetDlgItem(hwndDlg, IDC_SAVE), SW_SHOW);
					}
				}
				db_free(&dbv);
			}
			InvalidateRect(hwndDlg, NULL, TRUE);
			UpdateWindow(hwndDlg);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SAVE:
			{
				DBVARIANT dbv;
				JABBER_LIST_ITEM *item;
				HANDLE hFile;
				static TCHAR szFilter[512];
				unsigned char buffer[3];
				TCHAR szFileName[MAX_PATH];
				DWORD n;

				if (photoInfo->ppro->getTString(photoInfo->hContact, "jid", &dbv))
					break;

				TCHAR *jid = dbv.ptszVal;
				if ((item = photoInfo->ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid)) == NULL)
					item = photoInfo->ppro->ListGetItemPtr(LIST_ROSTER, jid);
				if (item != NULL) {
					if ((hFile=CreateFile(item->photoFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE) {
						if (ReadFile(hFile, buffer, 3, &n, NULL) && n==3) {
							if ( !strncmp((char*)buffer, "BM", 2)) {
								mir_sntprintf(szFilter, SIZEOF(szFilter), _T("BMP %s (*.bmp)"), TranslateT("format"));
								n = (DWORD)_tcslen(szFilter);
								_tcsncpy(szFilter+n+1, _T("*.BMP"), SIZEOF(szFilter)-n-2);
							}
							else if ( !strncmp((char*)buffer, "GIF", 3)) {
								mir_sntprintf(szFilter, SIZEOF(szFilter), _T("GIF %s (*.gif)"), TranslateT("format"));
								n = (DWORD)_tcslen(szFilter);
								_tcsncpy(szFilter+n+1, _T("*.GIF"), SIZEOF(szFilter)-n-2);
							}
							else if (buffer[0]==0xff && buffer[1]==0xd8 && buffer[2]==0xff) {
								mir_sntprintf(szFilter, SIZEOF(szFilter), _T("JPEG %s (*.jpg;*.jpeg)"), TranslateT("format"));
								n = (DWORD)_tcslen(szFilter);
								_tcsncpy(szFilter+n+1, _T("*.JPG;*.JPEG"), SIZEOF(szFilter)-n-2);
							}
							else {
								mir_sntprintf(szFilter, SIZEOF(szFilter), _T("%s (*.*)"), TranslateT("Unknown format"));
								n = (DWORD)_tcslen(szFilter);
								_tcsncpy(szFilter+n+1, _T("*.*"), SIZEOF(szFilter)-n-2);
							}
							szFilter[SIZEOF(szFilter)-1] = 0;

							OPENFILENAME ofn = { 0 };
							ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
							ofn.hwndOwner = hwndDlg;
							ofn.hInstance = NULL;
							ofn.lpstrFilter = szFilter;
							ofn.lpstrCustomFilter = NULL;
							ofn.nMaxCustFilter = 0;
							ofn.nFilterIndex = 0;
							ofn.lpstrFile = szFileName;
							ofn.nMaxFile = _MAX_PATH;
							ofn.lpstrFileTitle = NULL;
							ofn.nMaxFileTitle = 0;
							ofn.lpstrInitialDir = NULL;
							ofn.lpstrTitle = NULL;
							ofn.Flags = OFN_OVERWRITEPROMPT;
							ofn.nFileOffset = 0;
							ofn.nFileExtension = 0;
							ofn.lpstrDefExt = NULL;
							ofn.lCustData = 0L;
							ofn.lpfnHook = NULL;
							ofn.lpTemplateName = NULL;
							szFileName[0] = '\0';
							if (GetSaveFileName(&ofn)) {
								photoInfo->ppro->Log("File selected is %s", szFileName);
								CopyFile(item->photoFileName, szFileName, FALSE);
							}
						}
						CloseHandle(hFile);
					}
				}
				db_free(&dbv);

			}
			break;
		}
		break;

	case WM_PAINT:
		if ( !photoInfo->ppro->m_bJabberOnline)
			SetDlgItemText(hwndDlg, IDC_CANVAS, TranslateT("<Photo not available while offline>"));
		else if ( !photoInfo->hBitmap)
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
			GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bm);
			ptSize.x = bm.bmWidth;
			ptSize.y = bm.bmHeight;
			DPtoLP(hdcCanvas, &ptSize, 1);
			ptOrg.x = ptOrg.y = 0;
			DPtoLP(hdcMem, &ptOrg, 1);
			GetClientRect(hwndCanvas, &rect);
			InvalidateRect(hwndCanvas, NULL, TRUE);
			UpdateWindow(hwndCanvas);
			if (ptSize.x<=rect.right && ptSize.y<=rect.bottom) {
				pt.x = (rect.right - ptSize.x)/2;
				pt.y = (rect.bottom - ptSize.y)/2;
				ptFitSize = ptSize;
			}
			else {
				if (((float)(ptSize.x-rect.right))/ptSize.x > ((float)(ptSize.y-rect.bottom))/ptSize.y) {
					ptFitSize.x = rect.right;
					ptFitSize.y = (ptSize.y*rect.right)/ptSize.x;
					pt.x = 0;
					pt.y = (rect.bottom - ptFitSize.y)/2;
				}
				else {
					ptFitSize.x = (ptSize.x*rect.bottom)/ptSize.y;
					ptFitSize.y = rect.bottom;
					pt.x = (rect.right - ptFitSize.x)/2;
					pt.y = 0;
				}
			}

			if (JabberIsThemeActive && JabberDrawThemeParentBackground && JabberIsThemeActive()) {
				RECT rc; GetClientRect(hwndCanvas, &rc);
				JabberDrawThemeParentBackground(hwndCanvas, hdcCanvas, &rc);
			}
			else {
				RECT rc; GetClientRect(hwndCanvas, &rc);
				FillRect(hdcCanvas, &rc, (HBRUSH)GetSysColorBrush(COLOR_BTNFACE));
			}

			if (JabberAlphaBlend && bm.bmBitsPixel == 32) {
				BLENDFUNCTION bf = {0};
				bf.AlphaFormat = AC_SRC_ALPHA;
				bf.BlendOp = AC_SRC_OVER;
				bf.SourceConstantAlpha = 255;
				JabberAlphaBlend(hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, bf);
			}
			else {
				SetStretchBltMode(hdcCanvas, COLORONCOLOR);
				StretchBlt(hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, SRCCOPY);
			}

			DeleteDC(hdcMem);
		}
		break;

	case WM_DESTROY:
		DestroyIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_SAVE, BM_SETIMAGE, IMAGE_ICON, 0));
		if (photoInfo->hBitmap) {
			photoInfo->ppro->Log("Delete bitmap");
			DeleteObject(photoInfo->hBitmap);
		}
		if (photoInfo) mir_free(photoInfo);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnInfoInit - initializes user info option dialogs

int CJabberProto::OnUserInfoInit(WPARAM wParam, LPARAM lParam)
{
	if ( !CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)m_szModuleName))
		return 0;

	HANDLE hContact = (HANDLE)lParam;
	if (hContact == NULL) {
		// Show our vcard
		OnUserInfoInit_VCard(wParam, lParam);
		return 0;
	}

	char *szProto = GetContactProto(hContact);
	if (szProto != NULL && !strcmp(szProto, m_szModuleName)) {
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.hInstance = hInst;
		odp.dwInitParam = (LPARAM)this;

		odp.pfnDlgProc = JabberUserInfoDlgProc;
		odp.position = -2000000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_JABBER);
		odp.pszTitle = LPGEN("Account");
		UserInfo_AddPage(wParam, &odp);

		odp.pfnDlgProc = JabberUserPhotoDlgProc;
		odp.position = 2000000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_PHOTO);
		odp.pszTitle = LPGEN("Photo");
		UserInfo_AddPage(wParam, &odp);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserInfoUpdate

void JabberUserInfoInit()
{
	hUserInfoList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserInfoUpdate

void JabberUserInfoUpdate(HANDLE hContact)
{
	if ( !hContact)
		WindowList_BroadcastAsync(hUserInfoList, WM_JABBER_REFRESH, 0, 0);
	else if (HWND hwnd = WindowList_Find(hUserInfoList, hContact))
		PostMessage(hwnd, WM_JABBER_REFRESH, 0, 0);
}
