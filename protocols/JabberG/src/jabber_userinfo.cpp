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

class JabberUserInfoDlg : public CUserInfoPageDlg
{
	CJabberProto *ppro;
	JABBER_LIST_ITEM *item = nullptr;
	int resourcesCount = -1;

	CCtrlTreeView m_tree;

	UI_MESSAGE_MAP(JabberUserInfoDlg, CUserInfoPageDlg);
		UI_MESSAGE(WM_PROTO_CHECK_ONLINE, OnCheckOnline);
	UI_MESSAGE_MAP_END();

	INT_PTR OnCheckOnline(UINT, WPARAM, LPARAM)
	{
		if (!ppro->m_bJabberOnline)
			item = nullptr;
		else
			OnRefresh();
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// User information block

	void CleanupInfo(int stage)
	{
		HTREEITEM hItem = m_tree.GetRoot();
		while (hItem) {
			TVITEMEX tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = hItem;
			m_tree.GetItem(&tvi);

			switch (stage) {
			case 0:
				tvi.lParam |= INFOLINE_DELETE;
				m_tree.SetItem(&tvi);
				break;

			case 1:
				if (tvi.lParam & INFOLINE_DELETE) {
					hItem = m_tree.GetNextSibling(hItem);
					m_tree.DeleteItem(tvi.hItem);
					continue;
				}
				break;
			}

			HTREEITEM hItemTmp = nullptr;
			if (hItemTmp = m_tree.GetChild(hItem))
				hItem = hItemTmp;
			else if (hItemTmp = m_tree.GetNextSibling(hItem))
				hItem = hItemTmp;
			else {
				while (true) {
					if (!(hItem = m_tree.GetParent(hItem))) break;
					if (hItemTmp = m_tree.GetNextSibling(hItem)) {
						hItem = hItemTmp;
						break;
					}
				}
			}
		}
	}

	HTREEITEM FindInfoLine(HTREEITEM htiRoot, LPARAM id = INFOLINE_BAD_ID)
	{
		if (id == INFOLINE_BAD_ID) return nullptr;
		for (HTREEITEM hti = m_tree.GetChild(htiRoot); hti; hti = m_tree.GetNextSibling(hti)) {
			TVITEMEX tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = hti;
			m_tree.GetItem(&tvi);
			if ((tvi.lParam&INFOLINE_MASK) == (id&INFOLINE_MASK))
				return hti;
		}
		return nullptr;
	}

	HTREEITEM FillInfoLine(HTREEITEM htiRoot, HICON hIcon, const wchar_t *title, const char *value, LPARAM id = INFOLINE_BAD_ID, bool expand = false)
	{
		HTREEITEM hti = FindInfoLine(htiRoot, id);

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
			HIMAGELIST himl = m_tree.GetImageList(TVSIL_NORMAL);
			tvis.itemex.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvis.itemex.iImage =
				tvis.itemex.iSelectedImage = ImageList_AddIcon(himl, hIcon);
			IcoLib_ReleaseIcon(hIcon);
		}

		if (hti) {
			tvis.itemex.mask |= TVIF_HANDLE;
			tvis.itemex.hItem = hti;
			m_tree.SetItem(&tvis.itemex);
		}
		else {
			tvis.itemex.mask |= TVIF_STATE;
			tvis.itemex.stateMask = TVIS_EXPANDED;
			tvis.itemex.state = expand ? TVIS_EXPANDED : 0;
			hti = m_tree.InsertItem(&tvis);
		}

		return hti;
	}

	void FillAdvStatusInfo(HTREEITEM htiRoot, uint32_t dwInfoLine, MCONTACT hContact, wchar_t *szTitle, char *pszSlot)
	{
		ptrA szAdvStatusIcon(ppro->ReadAdvStatusA(hContact, pszSlot, ADVSTATUS_VAL_ICON));
		ptrW szAdvStatusTitle(ppro->ReadAdvStatusT(hContact, pszSlot, ADVSTATUS_VAL_TITLE));
		ptrW szAdvStatusText(ppro->ReadAdvStatusT(hContact, pszSlot, ADVSTATUS_VAL_TEXT));

		if (szAdvStatusIcon && szAdvStatusTitle && *szAdvStatusTitle) {
			wchar_t szText[2048];
			if (szAdvStatusText && *szAdvStatusText)
				mir_snwprintf(szText, L"%s (%s)", TranslateW(szAdvStatusTitle), szAdvStatusText.get());
			else
				wcsncpy_s(szText, TranslateW(szAdvStatusTitle), _TRUNCATE);
			FillInfoLine(htiRoot, IcoLib_GetIcon(szAdvStatusIcon), szTitle, T2Utf(szText), dwInfoLine);
		}
	}

	void FillResourceInfo(HTREEITEM htiRoot, int resource)
	{
		HTREEITEM htiResource = htiRoot;
		pResourceStatus r = resource ? item->arResources[resource - 1] : item->getTemp();

		if (r->m_szResourceName && *r->m_szResourceName)
			htiResource = FillInfoLine(htiRoot, Skin_LoadProtoIcon(ppro->m_szModuleName, r->m_iStatus),
				TranslateT("Resource"), r->m_szResourceName, sttInfoLineId(resource, INFOLINE_NAME), true);

		// StatusMsg
		FillInfoLine(htiResource, nullptr /*Skin_LoadIcon(SKINICON_EVENT_MESSAGE)*/,
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

			FillInfoLine(htiResource, hIcon, TranslateT("Software"), pCaps->GetSoft(), sttInfoLineId(resource, INFOLINE_SOFTWARE));

			// Version
			FillInfoLine(htiResource, nullptr, TranslateT("Version"), pCaps->GetSoftMir() ? pCaps->GetSoftMir() : pCaps->GetSoftVer(), sttInfoLineId(resource, INFOLINE_VERSION));

			// System
			FillInfoLine(htiResource, nullptr, TranslateT("System"), pCaps->GetOsVer() ? pCaps->GetOsVer() : pCaps->GetOs(), sttInfoLineId(resource, INFOLINE_SYSTEM));

			if (hIcon)
				DestroyIcon(hIcon);
		}

		// Resource priority
		char buf[256];
		itoa(r->m_iPriority, buf, 10);
		FillInfoLine(htiResource, nullptr, TranslateT("Resource priority"), buf, sttInfoLineId(resource, INFOLINE_PRIORITY));

		// Idle
		if (r->m_dwIdleStartTime != -1) {
			if (r->m_dwIdleStartTime != 0) {
				mir_strncpy(buf, ctime(&r->m_dwIdleStartTime), _countof(buf));
				size_t len = mir_strlen(buf);
				if (len > 0)
					buf[len - 1] = 0;
			}
			else mir_strncpy(buf, TranslateU("<currently online>"), _countof(buf));

			FillInfoLine(htiResource, nullptr, TranslateT("Last activity"), buf, sttInfoLineId(resource, INFOLINE_IDLE));
		}

		// caps
		JabberCapsBits jcb = ppro->GetResourceCapabilities(MakeJid(item->jid, r->m_szResourceName), r);
		if (!(jcb & JABBER_RESOURCE_CAPS_ERROR)) {
			HTREEITEM htiCaps = FillInfoLine(htiResource, IcoLib_GetIconByHandle(ppro->m_hProtoIcon), nullptr, TranslateU("Client capabilities"), sttInfoLineId(resource, INFOLINE_CAPS));
			int i;
			for (i = 0; i < g_cJabberFeatCapPairs; i++)
				if (jcb & g_JabberFeatCapPairs[i].jcbCap) {
					char szDescription[1024];
					if (g_JabberFeatCapPairs[i].tszDescription)
						mir_snprintf(szDescription, "%s (%s)", TranslateU(g_JabberFeatCapPairs[i].tszDescription), g_JabberFeatCapPairs[i].szFeature);
					else
						strncpy_s(szDescription, g_JabberFeatCapPairs[i].szFeature, _TRUNCATE);
					FillInfoLine(htiCaps, nullptr, nullptr, szDescription, sttInfoLineId(resource, INFOLINE_CAPS, i));
				}

			for (auto &it : ppro->m_lstJabberFeatCapPairsDynamic) {
				if (jcb & it->jcbCap) {
					char szDescription[1024];
					if (it->szDescription)
						mir_snprintf(szDescription, "%s (%s)", TranslateU(it->szDescription), it->szFeature);
					else
						strncpy_s(szDescription, it->szFeature, _TRUNCATE);
					FillInfoLine(htiCaps, nullptr, nullptr, szDescription, sttInfoLineId(resource, INFOLINE_CAPS, i++));
				}
			}
		}

		// Software info
		HTREEITEM htiSoftwareInfo = FillInfoLine(htiResource, IcoLib_GetIconByHandle(ppro->m_hProtoIcon), nullptr, TranslateU("Software information"), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION));
		int nLineId = 0;
		if (CJabberClientPartialCaps *pCaps = r->m_pCaps) {
			if (pCaps->GetOs())
				FillInfoLine(htiSoftwareInfo, nullptr, TranslateT("Operating system"), pCaps->GetOs(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
			if (pCaps->GetOsVer())
				FillInfoLine(htiSoftwareInfo, nullptr, TranslateT("Operating system version"), pCaps->GetOsVer(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
			if (pCaps->GetSoft())
				FillInfoLine(htiSoftwareInfo, nullptr, TranslateT("Software"), pCaps->GetSoft(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
			if (pCaps->GetSoftVer())
				FillInfoLine(htiSoftwareInfo, nullptr, TranslateT("Software version"), pCaps->GetSoftVer(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
			if (pCaps->GetSoftMir())
				FillInfoLine(htiSoftwareInfo, nullptr, TranslateT("Miranda core version"), pCaps->GetSoftMir(), sttInfoLineId(resource, INFOLINE_SOFTWARE_INFORMATION, nLineId++));
		}
	}

	void FillUserInfo()
	{
		m_tree.SendMsg(WM_SETREDRAW, FALSE, 0);

		CleanupInfo(0);

		HTREEITEM htiRoot = FillInfoLine(nullptr, IcoLib_GetIconByHandle(ppro->m_hProtoIcon), L"JID", item->jid, sttInfoLineId(0, INFOLINE_NAME), true);

		if (MCONTACT hContact = ppro->HContactFromJID(item->jid)) {
			FillAdvStatusInfo(htiRoot, sttInfoLineId(0, INFOLINE_MOOD), hContact, TranslateT("Mood"), ADVSTATUS_MOOD);
			FillAdvStatusInfo(htiRoot, sttInfoLineId(0, INFOLINE_ACTIVITY), hContact, TranslateT("Activity"), ADVSTATUS_ACTIVITY);
			FillAdvStatusInfo(htiRoot, sttInfoLineId(0, INFOLINE_TUNE), hContact, TranslateT("Tune"), ADVSTATUS_TUNE);
		}

		// subscription
		switch (item->subscription) {
		case SUB_BOTH:
			FillInfoLine(htiRoot, nullptr, TranslateT("Subscription"), TranslateU("both"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
			break;
		case SUB_TO:
			FillInfoLine(htiRoot, nullptr, TranslateT("Subscription"), TranslateU("to"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
			break;
		case SUB_FROM:
			FillInfoLine(htiRoot, nullptr, TranslateT("Subscription"), TranslateU("from"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
			break;
		default:
			FillInfoLine(htiRoot, nullptr, TranslateT("Subscription"), TranslateU("none"), sttInfoLineId(0, INFOLINE_SUBSCRIPTION));
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

			FillInfoLine(htiRoot, nullptr,
				(item->jid && strchr(item->jid, '@')) ? TranslateT("Last logoff time") : TranslateT("Uptime"), buf,
				sttInfoLineId(0, INFOLINE_LOGOFF));
		}

		if (r->m_szStatusMessage)
			FillInfoLine(htiRoot, nullptr, TranslateT("Logoff message"), r->m_szStatusMessage, sttInfoLineId(0, INFOLINE_LOGOFF_MSG));

		// activity
		if (item->m_pLastSeenResource)
			mir_strncpy(buf, item->m_pLastSeenResource->m_szResourceName, _countof(buf));
		else
			mir_strncpy(buf, TranslateU("<no information available>"), _countof(buf));
		FillInfoLine(htiRoot, nullptr, TranslateT("Last active resource"), buf, sttInfoLineId(0, INFOLINE_LASTACTIVE));

		// resources
		if (item->arResources.getCount()) {
			for (int i = 0; i < item->arResources.getCount(); i++)
				FillResourceInfo(htiRoot, i + 1);
		}
		else if (!strchr(item->jid, '@') || (r->m_iStatus != ID_STATUS_OFFLINE))
			FillResourceInfo(htiRoot, 0);

		CleanupInfo(1);
		m_tree.SendMsg(WM_SETREDRAW, TRUE, 0);

		RedrawWindow(m_tree.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
	}

public:
	JabberUserInfoDlg(CJabberProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_INFO_JABBER),
		ppro(_ppro),
		m_tree(this, IDC_TV_INFO)
	{
		m_tree.OnBuildMenu = Callback(this, &JabberUserInfoDlg::onMenu_Tree);
	}

	bool OnInitDialog() override
	{
		ppro->WindowSubscribe(m_hwnd);
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_USERDETAILS);

		RECT rc;
		GetClientRect(m_hwnd, &rc);
		MoveWindow(m_tree.GetHwnd(), 5, 5, rc.right - 10, rc.bottom - 10, TRUE);

		HIMAGELIST himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR | ILC_COLOR32 | ILC_MASK, 5, 1);
		ImageList_AddSkinIcon(himl, SKINICON_OTHER_SMALLDOT);
		TreeView_SetImageList(m_tree.GetHwnd(), himl, TVSIL_NORMAL);

		WindowList_Add(hUserInfoList, m_hwnd, m_hContact);
		return true;
	}

	void OnDestroy() override
	{
		ppro->WindowUnsubscribe(m_hwnd);
		WindowList_Remove(hUserInfoList, m_hwnd);
		ImageList_Destroy(m_tree.SetImageList(nullptr, TVSIL_NORMAL));
		Window_FreeIcon_IcoLib(m_hwnd);
	}

	int Resizer(UTILRESIZECONTROL*) override
	{
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	}

	bool OnRefresh() override
	{
		if (item == nullptr) {
			ptrA jid(ppro->getUStringA(m_hContact, "jid"));
			if (jid == nullptr)
				return false;

			if (ppro->m_bJabberOnline)
				if (!(item = ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid)))
					item = ppro->ListGetItemPtr(LIST_ROSTER, jid);

			if (item == nullptr) {
				m_tree.DeleteAllItems();
				HTREEITEM htiRoot = FillInfoLine(nullptr, IcoLib_GetIconByHandle(ppro->m_hProtoIcon), L"JID", jid, sttInfoLineId(0, INFOLINE_NAME), true);
				FillInfoLine(htiRoot, g_plugin.getIcon(IDI_VCARD), nullptr, TranslateU("Please switch online to see more details."));
				return false;
			}
		}
		FillUserInfo();
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Context menu

	void GetNodeText(HTREEITEM hti, CMStringW &buf, int indent = 0)
	{
		for (int i = 0; i < indent; i++)
			buf.AppendChar('\t');

		wchar_t wszText[256];
		TVITEMEX tvi = {};
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
		tvi.hItem = hti;
		tvi.cchTextMax = _countof(wszText);
		tvi.pszText = wszText;
		if (!m_tree.GetItem(&tvi)) // failure, maybe item was removed...
			return;

		buf.Append(wszText);
		buf.Append(L"\r\n");

		if (tvi.state & TVIS_EXPANDED)
			for (hti = m_tree.GetChild(hti); hti; hti = m_tree.GetNextSibling(hti))
				GetNodeText(hti, buf, indent + 1);
	}

	void onMenu_Tree(CContextMenuPos *pos)
	{
		if (!pos->hItem)
			return;

		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, (UINT_PTR)1, TranslateT("Copy"));
		AppendMenu(hMenu, MF_STRING, (UINT_PTR)2, TranslateT("Copy only this value"));
		AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));
		int nReturnCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_hwnd, nullptr);
		if (nReturnCmd == 1) {
			CMStringW buf;
			GetNodeText(pos->hItem, buf);
			JabberCopyText(m_hwnd, buf);
		}
		else if (nReturnCmd == 2) {
			wchar_t szBuffer[1024];
			TVITEMEX tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
			tvi.hItem = pos->hItem;
			tvi.cchTextMax = _countof(szBuffer);
			tvi.pszText = szBuffer;
			if (m_tree.GetItem(&tvi)) {
				if (wchar_t *str = wcsstr(szBuffer, L": "))
					JabberCopyText(m_hwnd, str + 2);
				else
					JabberCopyText(m_hwnd, szBuffer);
			}
		}
		DestroyMenu(hMenu);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserPhotoDlgProc - Jabber photo dialog

class JabberUserPhotoDlg : public CUserInfoPageDlg
{
	HBITMAP hBitmap = nullptr;
	CJabberProto *ppro;

	CCtrlButton btnSave;

	UI_MESSAGE_MAP(JabberUserInfoDlg, CUserInfoPageDlg);
		UI_MESSAGE(WM_PAINT, OnPaint);
	UI_MESSAGE_MAP_END();

public:
	JabberUserPhotoDlg(CJabberProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_VCARD_PHOTO),
		ppro(_ppro),
		btnSave(this, IDC_SAVE)
	{
		btnSave.OnClick = Callback(this, &JabberUserPhotoDlg::onClick_Save);
	}

	bool OnInitDialog() override
	{
		Button_SetIcon_IcoLib(m_hwnd, IDC_SAVE, g_plugin.getIconHandle(IDI_SAVE));
		ShowWindow(GetDlgItem(m_hwnd, IDC_LOAD), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_DELETE), SW_HIDE);
		return true;
	}

	void OnDestroy() override
	{
		Button_FreeIcon_IcoLib(m_hwnd, IDC_SAVE);

		if (hBitmap) {
			ppro->debugLogA("Delete bitmap");
			DeleteObject(hBitmap);
		}
	}

	bool OnRefresh() override
	{
		if (hBitmap) {
			DeleteObject(hBitmap);
			hBitmap = nullptr;
		}
		ShowWindow(GetDlgItem(m_hwnd, IDC_SAVE), SW_HIDE);
		{
			ptrA jid(ppro->getUStringA(m_hContact, "jid"));
			if (jid != nullptr) {
				JABBER_LIST_ITEM *item = ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid);
				if (item == nullptr)
					item = ppro->ListGetItemPtr(LIST_ROSTER, jid);
				if (item != nullptr) {
					if (item->photoFileName) {
						ppro->debugLogA("Showing picture from %s", item->photoFileName);
						hBitmap = Bitmap_Load(Utf2T(item->photoFileName));
						FreeImage_Premultiply(hBitmap);
						ShowWindow(GetDlgItem(m_hwnd, IDC_SAVE), SW_SHOW);
					}
				}
			}
		}
		InvalidateRect(m_hwnd, nullptr, TRUE);
		UpdateWindow(m_hwnd);
		return true;
	}

	void onClick_Save(CCtrlButton *)
	{
		wchar_t szFilter[512];

		ptrA jid(ppro->getUStringA(m_hContact, "jid"));
		if (jid == nullptr)
			return;

		JABBER_LIST_ITEM *item = ppro->ListGetItemPtr(LIST_VCARD_TEMP, jid);
		if (item == nullptr)
			if ((item = ppro->ListGetItemPtr(LIST_ROSTER, jid)) == nullptr)
				return;

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
		ofn.hwndOwner = m_hwnd;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = _MAX_PATH;
		ofn.Flags = OFN_OVERWRITEPROMPT;
		if (GetSaveFileName(&ofn)) {
			ppro->debugLogW(L"File selected is %s", szFileName);
			CopyFile(Utf2T(item->photoFileName), szFileName, FALSE);
		}
	}

	INT_PTR OnPaint(UINT, WPARAM, LPARAM)
	{
		if (!ppro->m_bJabberOnline)
			SetDlgItemText(m_hwnd, IDC_CANVAS, TranslateT("<Photo not available while offline>"));
		else if (!hBitmap)
			SetDlgItemText(m_hwnd, IDC_CANVAS, TranslateT("<No photo>"));
		else {
			BITMAP bm;
			POINT ptSize, ptOrg, pt, ptFitSize;
			RECT rect;

			SetDlgItemTextA(m_hwnd, IDC_CANVAS, "");
			HWND hwndCanvas = GetDlgItem(m_hwnd, IDC_CANVAS);
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
					ptFitSize.y = (ptSize.y * rect.right) / ptSize.x;
					pt.x = 0;
					pt.y = (rect.bottom - ptFitSize.y) / 2;
				}
				else {
					ptFitSize.x = (ptSize.x * rect.bottom) / ptSize.y;
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
		return FALSE;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserPhotoDlgProc - Jabber photo dialog

static int EnumOwnSessions(const char *szSetting, void *param)
{
	auto *pList = (LIST<char>*)param;

	if (!memcmp(szSetting, omemo::DevicePrefix, sizeof(omemo::DevicePrefix)-1))
		if (szSetting[sizeof(omemo::DevicePrefix)] != 0)
			pList->insert(mir_strdup(szSetting));

	return 0;
}

static int EnumOmemoSessions(const char *szSetting, void *param)
{
	auto *pList = (LIST<char>*)param;

	if (!memcmp(szSetting, omemo::IdentityPrefix, sizeof(omemo::IdentityPrefix) - 1)) 
		pList->insert(mir_strdup(szSetting + sizeof(omemo::IdentityPrefix)));

	return 0;
}

class JabberUserOmemoDlg : public CUserInfoPageDlg
{
	CJabberProto *ppro;

	CCtrlListView m_list;

	void AddListItem(const CMStringA &pszStr1, const wchar_t *pszStr2, const CMStringA &pszStr3)
	{
		LVITEM lvi = {};
		lvi.mask = LVIF_TEXT;
		lvi.pszText = mir_a2u(pszStr1);
		lvi.iItem = 100500;
		int idx = m_list.InsertItem(&lvi);
		mir_free(lvi.pszText);

		m_list.SetItemText(idx, 1, (wchar_t *)pszStr2);
		CMStringW fp = omemo::FormatFingerprint(pszStr3);
		m_list.SetItemText(idx, 2, fp.GetBuffer());
	}

public:
	JabberUserOmemoDlg(CJabberProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_INFO_OMEMO),
		ppro(_ppro),
		m_list(this, IDC_LIST)
	{
	}

	bool OnInitDialog() override
	{
		LV_COLUMN lvc = {};
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;

		lvc.cx = 90;
		lvc.pszText = TranslateT("Device ID");
		m_list.InsertColumn(1, &lvc);

		lvc.cx = 80;
		lvc.pszText = TranslateT("Status");
		m_list.InsertColumn(2, &lvc);

		lvc.cx = 550;
		lvc.pszText = TranslateT("Fingerprint");
		m_list.InsertColumn(3, &lvc);

		if (m_hContact == 0)
			OnRefresh();
		return true;
	}

	int Resizer(UTILRESIZECONTROL*) override
	{
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	}

	bool OnRefresh() override
	{
		m_list.DeleteAllItems();

		if (m_hContact == 0) {
			// GetOwnDeviceId() creates own keys if they don't exist
			CMStringA str1(FORMAT, "%d", ppro->m_omemo.GetOwnDeviceId());
			CMStringA str2(ppro->getMStringA("OmemoFingerprintOwn"));
			AddListItem(str1, TranslateT("Own device"), str2);
		}

		for (int i = 0;; i++) {
			CMStringA szSetting(FORMAT, "%s%d", omemo::DevicePrefix, i);
			uint32_t device_id = ppro->getDword(m_hContact, szSetting, 0);
			if (device_id == 0)
				break;

			char *jiddev = ppro->getStringA(m_hContact, "jid");
			if (jiddev == 0)
				continue;

			size_t len = strlen(jiddev);
			jiddev = (char *)mir_realloc(jiddev, len + sizeof(int32_t));
			memcpy(jiddev + len, &device_id, sizeof(int32_t));

			szSetting = omemo::IdentityPrefix;
			szSetting.Append(ptrA(mir_base64_encode(jiddev, len + sizeof(int32_t))));
			mir_free(jiddev);

			const wchar_t *pwszStatus = L"";
			CMStringA fp_hex;
			DBVARIANT dbv = { 0 };
			dbv.type = DBVT_BLOB;
			db_get(m_hContact, ppro->m_szModuleName, szSetting, &dbv);
			if (dbv.cpbVal == 33) {
				fp_hex.Truncate(33 * 2);
				bin2hex(dbv.pbVal, 33, fp_hex.GetBuffer());
				uint8_t trusted = ppro->getByte(m_hContact, "OmemoFingerprintTrusted_" + fp_hex);
				pwszStatus = trusted ? TranslateT("Trusted") : TranslateT("UNTRUSTED");
				//TODO: 3 states Trusted, Untrusted, TOFU
			}
			else if (dbv.cpbVal)
				pwszStatus = TranslateT("Unknown");

			db_free(&dbv);

			AddListItem(CMStringA(FORMAT, "%d", device_id), pwszStatus, fp_hex);
		}
		return false;
	}
};

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
		USERINFOPAGE uip = {};
		uip.dwInitParam = (LPARAM)this;
		uip.flags = ODPF_UNICODE | ODPF_USERINFOTAB;
		uip.szGroup.w = m_tszUserName;

		uip.pDialog = new JabberUserInfoDlg(this);
		uip.position = -2000000000;
		uip.szTitle.w = LPGENW("Account");
		g_plugin.addUserInfo(wParam, &uip);

		uip.pDialog = new JabberUserPhotoDlg(this);
		uip.position = 2000000000;
		uip.szTitle.w = LPGENW("Photo");
		g_plugin.addUserInfo(wParam, &uip);

		CheckOmemoUserInfo(wParam, uip);
	}

	return 0;
}

void CJabberProto::CheckOmemoUserInfo(WPARAM wParam, USERINFOPAGE &uip)
{
	if (m_bUseOMEMO) {
		uip.pDialog = new JabberUserOmemoDlg(this);
		uip.position = 2000000001;
		uip.szTitle.w = L"OMEMO";
		g_plugin.addUserInfo(wParam, &uip);
	}
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
