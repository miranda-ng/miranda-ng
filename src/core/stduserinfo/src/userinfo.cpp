/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#define UPDATEANIMFRAMES 20

struct DetailsPageData : public MNonCopyable, public MZeroedObject
{
	CUserInfoPageDlg *pDialog;
	HWND              hwnd;
	HTREEITEM         hItem;
	MCONTACT          hContact;
	HPLUGIN           pPlugin;
	int               changed;
	uint32_t          dwFlags;
	wchar_t          *pwszTitle, *pwszGroup;
	INT_PTR           lParam;

	~DetailsPageData()
	{
		if (hwnd != nullptr)
			DestroyWindow(hwnd);
		mir_free(pwszTitle);
		mir_free(pwszGroup);
	}

	__forceinline wchar_t *getTitle() const {
		return (dwFlags & ODPF_DONTTRANSLATE) ? pwszTitle : TranslateW_LP(pwszTitle, pPlugin);
	}

	__forceinline wchar_t* getGroup() const
	{	return (dwFlags & ODPF_DONTTRANSLATE) ? pwszGroup : TranslateW_LP(pwszGroup, pPlugin);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

int DetailsInit(WPARAM wParam, LPARAM lParam);

static MWindowList hWindowList = nullptr;
static HANDLE hDetailsInitEvent;

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

static void ThemeDialogBackground(HWND hwnd)
{
	EnableThemeDialogTexture(hwnd, ETDT_ENABLETAB);
}

static int PageSortProc(const DetailsPageData *item1, const DetailsPageData *item2)
{
	wchar_t *s1 = item1->getGroup(), *s2 = item2->getGroup();
	if (int res = mir_wstrcmp(s1, s2))
		return res;

	s1 = item1->getTitle(), s2 = item2->getTitle();
	if (!mir_wstrcmp(s1, TranslateT("Summary"))) return -1;
	if (!mir_wstrcmp(s2, TranslateT("Summary"))) return 1;
	return mir_wstrcmp(s1, s2);
}

static int UserInfoContactDelete(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(hWindowList, wParam);
	if (hwnd != nullptr)
		DestroyWindow(hwnd);
	return 0;
}

#define M_PROTOACK    (WM_USER+10)
#define M_DLLUNLOAD   (WM_USER+12)

class CUserInfoDlg : public CDlgBase
{
	MCONTACT   m_hContact;
	HINSTANCE  m_hInstIcmp = 0;
	int        m_updateAnimFrame = 0;
	wchar_t    m_szUpdating[64];
	int       *m_infosUpdated = 0;
	bool       m_bIsMeta;

	HANDLE     m_hProtoAckEvent = 0;
	HANDLE     m_hDllUnloadEvent = 0;
	HIMAGELIST m_imageList = 0;

	OBJLIST<DetailsPageData> m_pages;
	DetailsPageData *m_pCurrent = nullptr;

	void PopulateContact(MCONTACT hContact, int iFolderImage, wchar_t *pwszRoot)
	{
		ptrW ptszLastTab(g_plugin.getWStringA("LastTab"));
		m_pCurrent = nullptr;

		LIST<DetailsPageData> items(1, PageSortProc);
		NotifyEventHooks(hDetailsInitEvent, (WPARAM)&items, hContact);
		
		for (auto &it : items.rev_iter()) {
			it->pDialog->SetContact(hContact);
			if (!it->pDialog->IsEmpty())
				continue;

			delete it;
			items.remove(items.indexOf(&it));
		}

		if (items.getCount() == 0)
			return;

		HTREEITEM hParent;
		{
			TVINSERTSTRUCT tvis = {};
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.lParam = (LPARAM)items[0];
			tvis.item.iImage = tvis.item.iSelectedImage = iFolderImage;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvis.item.state = tvis.item.stateMask = TVIS_EXPANDED;
			tvis.item.pszText = pwszRoot;
			hParent = m_tree.InsertItem(&tvis);
		}

		for (auto &it : items) {
			int iImage = 1;
			if ((it->dwFlags & ODPF_ICON) && it->lParam) {
				HICON hIcon = IcoLib_GetIconByHandle((HANDLE)it->lParam);
				if (hIcon) {
					iImage = ImageList_AddIcon(m_imageList, hIcon);
					IcoLib_ReleaseIcon(hIcon);
				}
			}

			TVINSERTSTRUCT tvis;
			tvis.hParent = hParent;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.iImage = tvis.item.iSelectedImage = iImage;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvis.item.lParam = (LPARAM)it;
			tvis.item.pszText = it->getTitle();
			if (ptszLastTab && !mir_wstrcmp(tvis.item.pszText, ptszLastTab))
				m_pCurrent = it;

			it->hItem = m_tree.InsertItem(&tvis);

			it->hContact = hContact;
			m_pages.insert(it);
		}

		if (!m_pCurrent)
			m_pCurrent = &m_pages[0];
	}

	void BuildTree()
	{
		if (m_imageList)
			ImageList_Destroy(m_imageList);
		m_imageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 1);

		ImageList_AddIcon(m_imageList, g_plugin.getIcon(IDI_FOLDER));
		ImageList_AddIcon(m_imageList, g_plugin.getIcon(IDI_ITEM));

		m_tree.SetImageList(m_imageList, TVSIL_NORMAL);

		PopulateContact(m_hContact, 0, TranslateT("General"));

		if (m_bIsMeta) {
			int nSubs = db_mc_getSubCount(m_hContact);

			for (int i = 0; i < nSubs; i++) {
				MCONTACT hSub = db_mc_getSub(m_hContact, i);
				if (hSub > 0) {
					auto *szProto = Proto_GetBaseAccountName(hSub);
					auto *pa = Proto_GetAccount(szProto);
					PopulateContact(hSub, 
						ImageList_AddIcon(m_imageList, IcoLib_GetIconByHandle(Skin_GetProtoIcon(szProto, ID_STATUS_ONLINE))),
						pa ? pa->tszAccountName : _A2T(szProto));
				}
			}
		}
	}

	void CheckOnline()
	{
		if (m_hContact == 0)
			return;

		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr || m_bIsMeta)
			btnUpdate.Disable();
		else {
			if (Proto_GetStatus(szProto) < ID_STATUS_ONLINE)
				btnUpdate.Disable();
			else
				btnUpdate.Enable(!IsWindowVisible(GetDlgItem(m_hwnd, IDC_UPDATING)));
		}
	}

	void CreateDetailsPageWindow(DetailsPageData *ppg)
	{
		auto *pDlg = ppg->pDialog;
		if (pDlg == nullptr)
			return;

		pDlg->SetParent(m_hwnd);
		pDlg->SetContact(ppg->hContact);
		pDlg->Create();
		ppg->hwnd = pDlg->GetHwnd();

		::ThemeDialogBackground(ppg->hwnd);

		pDlg->OnRefresh();
	}

	void ResizeCurrent()
	{
		RECT rc;
		GetWindowRect(m_place.GetHwnd(), &rc);

		POINT pt = { 0, 0 };
		ClientToScreen(m_hwnd, &pt);
		OffsetRect(&rc, -pt.x, -pt.y);
		SetWindowPos(m_pCurrent->hwnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0);
	}

	CCtrlBase m_place;
	CCtrlButton btnUpdate;
	CCtrlTreeView m_tree;
	CTimer updateTimer;

public:
	CUserInfoDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_DETAILS),
		m_hContact(hContact),
		m_bIsMeta(db_mc_isMeta(hContact)),
		m_pages(1),
		m_tree(this, IDC_PAGETREE),
		m_place(this, IDC_TABS),
		btnUpdate(this, IDC_UPDATE),
		updateTimer(this, 1)
	{
		SetMinSize(480, 382);

		m_tree.OnSelChanged = Callback(this, &CUserInfoDlg::onSelChanged_Tree);
		m_tree.OnSelChanging = Callback(this, &CUserInfoDlg::onSelChanging);

		btnUpdate.OnClick = Callback(this, &CUserInfoDlg::onClick_Update);

		updateTimer.OnEvent = Callback(this, &CUserInfoDlg::onTimer);
	}

	bool OnInitDialog() override
	{
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_USERDETAILS);
		Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "main");

		m_hProtoAckEvent = HookEventMessage(ME_PROTO_ACK, m_hwnd, M_PROTOACK);
		m_hDllUnloadEvent = HookEventMessage(ME_SYSTEM_MODULEUNLOAD, m_hwnd, M_DLLUNLOAD);
		WindowList_Add(hWindowList, m_hwnd, m_hContact);

		//////////////////////////////////////////////////////////////////////
		wchar_t *name, oldTitle[256], newTitle[256];
		if (m_hContact == NULL)
			name = TranslateT("Owner");
		else
			name = Clist_GetContactDisplayName(m_hContact);

		GetWindowText(m_hwnd, oldTitle, _countof(oldTitle));
		mir_snwprintf(newTitle, oldTitle, name);
		SetWindowText(m_hwnd, newTitle);

		BuildTree();

		//////////////////////////////////////////////////////////////////////

		m_tree.SelectItem(m_pCurrent->hItem);

		//////////////////////////////////////////////////////////////////////
		m_updateAnimFrame = 0;
		GetDlgItemText(m_hwnd, IDC_UPDATING, m_szUpdating, _countof(m_szUpdating));
		CheckOnline();
		if (!ProtoChainSend(m_hContact, PSS_GETINFO, SGIF_ONOPEN, 0)) {
			btnUpdate.Disable();
			SetTimer(m_hwnd, 1, 100, nullptr);
		}
		else ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_HIDE);

		SetFocus(m_tree.GetHwnd());
		return true;
	}

	bool OnApply() override
	{
		PSHNOTIFY pshn;
		pshn.hdr.idFrom = 0;
		pshn.lParam = (LPARAM)m_hContact;
		if (m_pCurrent) {
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = m_pCurrent->hwnd;
			if (SendMessage(m_pCurrent->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn))
				return false;
		}

		pshn.hdr.code = PSN_APPLY;
		for (auto &odp : m_pages) {
			if (odp->hwnd == nullptr || !odp->changed)
				continue;
			pshn.hdr.hwndFrom = odp->hwnd;
			if (SendMessage(odp->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn) == PSNRET_INVALID_NOCHANGEPAGE) {
				m_tree.Select(odp->hItem, TVGN_CARET);
				if (m_pCurrent)
					ShowWindow(m_pCurrent->hwnd, SW_HIDE);
				m_pCurrent = odp;
				ShowWindow(m_pCurrent->hwnd, SW_SHOW);
				return false;
			}
		}
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_TABS:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDOK:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

		case IDC_HEADERBAR:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

		case IDC_PAGETREE:
			return RD_ANCHORX_LEFT | RD_ANCHORY_HEIGHT;
		}

		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}

	void OnDestroy() override
	{
		if (m_pCurrent) {
			wchar_t name[128];
			TVITEMEX tvi;
			tvi.mask = TVIF_TEXT;
			tvi.hItem = m_pCurrent->hItem;
			tvi.pszText = name;
			tvi.cchTextMax = _countof(name);
			m_tree.GetItem(&tvi);
			g_plugin.setWString("LastTab", name);
		}

		if (m_imageList)
			ImageList_Destroy(m_imageList);

		Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "main");

		Window_FreeIcon_IcoLib(m_hwnd);
		WindowList_Remove(hWindowList, m_hwnd);

		UnhookEvent(m_hProtoAckEvent);
		UnhookEvent(m_hDllUnloadEvent);

		m_pages.destroy();
		mir_free(m_infosUpdated);
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		PSHNOTIFY pshn;

		switch (uMsg) {
		case PSM_CHANGED:
			if (m_pCurrent)
				m_pCurrent->changed = 1;
			return TRUE;

		case PSM_FORCECHANGED:
			pshn.hdr.code = PSN_INFOCHANGED;
			pshn.hdr.idFrom = 0;
			pshn.lParam = (LPARAM)m_hContact;
			for (auto &odp : m_pages) {
				pshn.hdr.hwndFrom = odp->hwnd;
				if (odp->hwnd != nullptr)
					SendMessage(odp->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
			}
			break;

		case M_DLLUNLOAD:
			{
				bool bRemoved = false;
				HINSTANCE hInst = (HINSTANCE)lParam;
				for (auto &odp : m_pages.rev_iter()) {
					if (odp->pPlugin->getInst() == hInst) {
						if (!bRemoved) {
							m_tree.DeleteAllItems();
							bRemoved = true;
						}
						m_pages.removeItem(&odp);
					}
				}

				if (bRemoved)
					BuildTree();
			}
			break;

		case M_PROTOACK:
			{
				ACKDATA *ack = (ACKDATA *)lParam;
				if (ack->hContact == NULL && ack->type == ACKTYPE_STATUS) {
					CheckOnline();
					break;
				}
				
				if (ack->type != ACKTYPE_GETINFO)
					break;
				if (ack->hContact != m_hContact && !(m_bIsMeta && db_mc_getMeta(ack->hContact) == m_hContact))
					break;

				SendMessage(m_hwnd, PSM_FORCECHANGED, 0, 0);
				/* if they're not gonna send any more ACK's don't let that mean we should crash */
				if (!ack->hProcess && !ack->lParam) {
					ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_HIDE);
					updateTimer.Stop();
					CheckOnline();
					break;
				}
				
				if (m_infosUpdated == nullptr)
					m_infosUpdated = (int *)mir_calloc(sizeof(int) * (INT_PTR)ack->hProcess);
				
				if (ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED)
					m_infosUpdated[ack->lParam] = 1;

				int i;
				for (i = 0; i < (INT_PTR)ack->hProcess; i++)
					if (m_infosUpdated[i] == 0)
						break;

				if (i == (INT_PTR)ack->hProcess) {
					ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_HIDE);
					updateTimer.Stop();
					CheckOnline();
				}
			}
			break;
		}

		INT_PTR res = CDlgBase::DlgProc(uMsg, wParam, lParam);
		
		if (uMsg == WM_SIZE && m_pCurrent)
			ResizeCurrent();
		
		return res;

	}

	void onSelChanging(CCtrlTreeView *)
	{
		if (m_pCurrent && m_pCurrent->hwnd != NULL) {
			PSHNOTIFY pshn;
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = m_pCurrent->hwnd;
			pshn.hdr.idFrom = 0;
			pshn.lParam = (LPARAM)m_hContact;
			if (SendMessage(m_pCurrent->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn))
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
		}
	}

	void onSelChanged_Tree(CCtrlTreeView::TEventInfo *ev)
	{
		if (m_pCurrent && m_pCurrent->hwnd != NULL)
			ShowWindow(m_pCurrent->hwnd, SW_HIDE);

		LPNMTREEVIEW pnmtv = ev->nmtv;
		TVITEM tvi = pnmtv->itemNew;
		m_pCurrent = (DetailsPageData*)tvi.lParam;

		if (m_pCurrent) {
			if (m_pCurrent->hwnd == NULL)
				CreateDetailsPageWindow(m_pCurrent);

			ResizeCurrent();
			ShowWindow(m_pCurrent->hwnd, SW_SHOWNA);
		}
	}

	void onClick_Cancel(CCtrlButton *)
	{
		PSHNOTIFY pshn;
		pshn.hdr.idFrom = 0;
		pshn.hdr.code = PSN_RESET;
		pshn.lParam = (LPARAM)m_hContact;

		for (auto &odp : m_pages) {
			if (odp->hwnd == nullptr || !odp->changed)
				continue;
			pshn.hdr.hwndFrom = odp->hwnd;
			SendMessage(odp->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
		}
	}

	void onClick_Update(CCtrlButton *)
	{
		if (m_infosUpdated != NULL) {
			mir_free(m_infosUpdated);
			m_infosUpdated = NULL;
		}

		if (m_hContact != NULL) {
			if (!ProtoChainSend(m_hContact, PSS_GETINFO, 0, 0)) {
				btnUpdate.Disable();
				ShowWindow(GetDlgItem(m_hwnd, IDC_UPDATING), SW_SHOW);
				updateTimer.Start(100);
			}
		}
	}

	void onTimer(CTimer *)
	{
		wchar_t str[128];
		mir_snwprintf(str, L"%.*s%s%.*s", m_updateAnimFrame % 10, L".........", m_szUpdating, m_updateAnimFrame % 10, L".........");
		SetDlgItemText(m_hwnd, IDC_UPDATING, str);
		if (++m_updateAnimFrame == UPDATEANIMFRAMES)
			m_updateAnimFrame = 0;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Services

static INT_PTR AddDetailsPage(WPARAM wParam, LPARAM lParam)
{
	auto *uip = (USERINFOPAGE*)lParam;
	auto *pList = (LIST<DetailsPageData>*)wParam;
	if (uip == nullptr || pList == nullptr)
		return 1;

	auto *pNew = new DetailsPageData();
	pNew->pPlugin = uip->pPlugin;
	pNew->pDialog = uip->pDialog;

	if (uip->flags & ODPF_UNICODE) {
		pNew->pwszTitle = (uip->szTitle.w == nullptr) ? nullptr : mir_wstrdup(uip->szTitle.w);
		pNew->pwszGroup = (uip->flags & ODPF_USERINFOTAB) ? mir_wstrdup(uip->szGroup.w) : nullptr;
	}
	else {
		pNew->pwszTitle = mir_a2u(uip->szTitle.a);
		pNew->pwszGroup = (uip->flags & ODPF_USERINFOTAB) ? mir_a2u(uip->szGroup.a) : nullptr;
	}

	pNew->dwFlags = uip->flags;
	pNew->lParam = uip->dwInitParam;
	pList->insert(pNew);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR ShowDetailsDialogCommand(WPARAM hContact, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList, hContact)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		return 0;
	}

	auto *pDlg = new CUserInfoDlg(hContact);
	pDlg->Show();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module events

static int ShutdownUserInfo(WPARAM, LPARAM)
{
	WindowList_Broadcast(hWindowList, WM_DESTROY, 0, 0);
	WindowList_Destroy(hWindowList);
	return 0;
}

static int OnTopToolBarLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_USERINFO_SHOWDIALOG;
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	ttb.name = ttb.pszTooltipUp = LPGEN("User &details");
	g_plugin.addTTB(&ttb);
	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("Folder"),   "folder",   IDI_FOLDER   },
	{ LPGEN("Page"),     "page",     IDI_ITEM     },
	{ LPGEN("Contact"),  "contact",  IDI_CONTACT  },
	{ LPGEN("Location"), "location", IDI_LOCATION },
	{ LPGEN("Notes"),    "notes",    IDI_NOTES    },
	{ LPGEN("Page"),     "Page",     IDI_ITEM     },
	{ LPGEN("Summary"),  "summary",  IDI_SUMMARY  },
	{ LPGEN("Work"),     "work",     IDI_WORK     },
};

int LoadUserInfoModule(void)
{
	g_plugin.registerIcon(MODULENAME, iconList);

	CreateServiceFunction("UserInfo/AddPage", AddDetailsPage);
	CreateServiceFunction(MS_USERINFO_SHOWDIALOG, ShowDetailsDialogCommand);

	// this event is already created inside the core, don't destroy it
	hDetailsInitEvent = CreateHookableEvent(ME_USERINFO_INITIALISE);

	HookEvent(ME_TTB_MODULELOADED, OnTopToolBarLoaded);
	HookEvent(ME_USERINFO_INITIALISE, DetailsInit);
	HookEvent(ME_DB_CONTACT_DELETED, UserInfoContactDelete);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownUserInfo);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xe8731d53, 0x95af, 0x42cf, 0xae, 0x27, 0xc7, 0xa7, 0xa, 0xbf, 0x14, 0x1c);
	mi.position = 1000050000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	mi.name.a = LPGEN("User &details");
	mi.pszService = MS_USERINFO_SHOWDIALOG;
	Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x42852ca4, 0x4941, 0x4219, 0x8b, 0x98, 0x33, 0x46, 0x8c, 0x32, 0xd8, 0xb8);
	mi.position = 500050000;
	mi.name.a = LPGEN("View/change my &details...");
	Menu_AddMainMenuItem(&mi);

	hWindowList = WindowList_Create();
	return 0;
}
