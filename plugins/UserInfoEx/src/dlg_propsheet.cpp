/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/////////////////////////////////////////////////////////////////////////////////////////
// System & local includes:

#include "stdafx.h"
#include "dlg_propsheet.h"
#include "psp_base.h"
#include "ex_import/svc_ExImport.h"
#include "svc_reminder.h"

#define UPDATEANIMFRAMES		20

// internal dialog message handler
#define M_CHECKONLINE			(WM_USER + 10)
#define HM_PROTOACK				(WM_USER + 11)
#define HM_SETTING_CHANGED		(WM_USER + 12)
#define HM_RELOADICONS			(WM_USER + 13)
#define HM_SETWINDOWTITLE		(WM_USER + 14)
#define HM_UNLOADED           (WM_USER + 15)

#define TIMERID_UPDATING      1
#ifndef TIMERID_RENAME
#define TIMERID_RENAME        2
#endif

// flags for the PS structure
#define PSF_CHANGED				0x00000100
#define PSF_LOCKED				0x00000200
#define PSF_INITIALIZED			0x00000400

#define INIT_ICONS_NONE			0
#define INIT_ICONS_OWNER		1
#define INIT_ICONS_CONTACT		2
#define INIT_ICONS_ALL			(INIT_ICONS_OWNER | INIT_ICONS_CONTACT)

/////////////////////////////////////////////////////////////////////////////////////////
// internal variables

static uint8_t bInitIcons = INIT_ICONS_NONE;
static MWindowList g_hWindowList = nullptr;
static HANDLE g_hDetailsInitEvent = nullptr;

static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

CPsHdr::CPsHdr() :
	_ignore(10, wcscmp)
{
}

CPsHdr::~CPsHdr()
{
	// delete data
	for (auto &it : _ignore)
		mir_free(it);
}

void CPsHdr::Free_pPages()
{
	for (int i = 0; i < _numPages; i++)
		delete _pPages[i];
	_numPages = 0;
	MIR_FREE(_pPages);
}

/////////////////////////////////////////////////////////////////////////////////////////
// class CPsUpload

class CPsUpload
{
public:
	enum EPsUpReturn
	{
		UPLOAD_CONTINUE = 0,
		UPLOAD_FINISH = 1,
		UPLOAD_FINISH_CLOSE = 2
	};

private:
	PROTOACCOUNT **_pPd;
	int _numProto;
	uint8_t _bExitAfterUploading;
	HANDLE _hUploading;
	LPPS _pPs;

	////////////////////////////////////////////////////////////////////////////////////////
	// @name Upload
	// @class CPsUpload
	// @desc start upload process for the current protocol
	// @return 0 on success or 1 otherwise

	int Upload()
	{
		// check if icq is online
		if (!IsProtoOnline((*_pPd)->szModuleName))
			MsgBox(_pPs->hDlg, MB_ICON_WARNING, TranslateT("Upload details"),
				CMStringW(FORMAT, TranslateT("Protocol '%s' is offline"), _A2T((*_pPd)->szModuleName).get()),
				TranslateT("You are not currently connected to the ICQ network.\nYou must be online in order to update your information on the server.\n\nYour changes will be saved to database only."));

		// start uploading process
		else _hUploading = 0;

		return 1;
	}

public:
	////////////////////////////////////////////////////////////////////////////////////////
	// @name	CPsUpload
	// @class	CPsUpload
	// @desc	retrieves the list of installed protocols and initializes the class
	// @param	pPs			- the owning propertysheet
	// @param	bExitAfter	- whether the dialog is to close after upload or not

	CPsUpload(LPPS pPs, uint8_t bExitAfter)
	{
		_pPs = pPs;
		_pPd = nullptr;
		_numProto = 0;
		_hUploading = nullptr;
		_bExitAfterUploading = bExitAfter;
	}

	int UploadFirst()
	{
		// create a list of all protocols which support uploading contact information
		Proto_EnumAccounts(&_numProto, &_pPd);
		return UploadNext();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// @name	~CPsUpload
	// @class CPsUpload

	~CPsUpload()
	{
		_pPs->pUpload = nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// @name	Handle
	// @class CPsUpload
	// @desc	returns the handle of the current upload process
	// @return handle of the current upload process

	__inline HANDLE Handle() const
	{
		return _hUploading;
	};

	////////////////////////////////////////////////////////////////////////////////////////
	// @name	UploadNext
	// @class CPsUpload
	// @desc	Search the next protocol which supports uploading contact information
	//			and start uploading. Delete the object if ready

	int UploadNext()
	{
		return _bExitAfterUploading ? UPLOAD_FINISH_CLOSE : UPLOAD_FINISH;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// propertysheet

// @name	SortProc()
// @desc	used for sorting the tab pages
// @return	-1 or 0 or 1

static int SortProc(CPsTreeItem **item1, CPsTreeItem **item2)
{
	if (*item1 && *item2) {
		if ((*item2)->Pos() > (*item1)->Pos())
			return -1;
		if ((*item2)->Pos() < (*item1)->Pos())
			return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// This service routine creates the DetailsDialog
// @param	wParam		- handle to contact
// @param	lParam		- not used
// @retval	0 on success or 1 on failure

static INT_PTR ShowDialog(WPARAM wParam, LPARAM)
{
	// update some cached settings
	myGlobals.ShowPropsheetColours = g_plugin.bShowColours;
	myGlobals.WantAeroAdaption = g_plugin.bAero;

	// allow only one dialog per user
	if (HWND hWnd = WindowList_Find(g_hWindowList, wParam)) {
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		return 0;
	}

	CPsHdr psh;
	POINT metrics;
	bool bScanMetaSubContacts = false;

	// init the treeview options
	if (g_plugin.bSortTree)
		psh._dwFlags |= PSTVF_SORTTREE;
	if (g_plugin.bTreeGroups)
		psh._dwFlags |= PSTVF_GROUPS;

	// create imagelist
	metrics.x = GetSystemMetrics(SM_CXSMICON);
	metrics.y = GetSystemMetrics(SM_CYSMICON);
	if ((psh._hImages = ImageList_Create(metrics.x, metrics.y, ILC_COLOR32 | ILC_MASK, 0, 1)) == nullptr) {
		MsgErr(nullptr, LPGENW("Creating the image list failed!"));
		return 1;
	}

	HICON hDefIcon = g_plugin.getIcon(IDI_LST_FOLDER);
	if (!hDefIcon)
		hDefIcon = (HICON)g_plugin.getIcon(IDI_DEFAULT, false);

	// add the default icon to imagelist
	ImageList_AddIcon(psh._hImages, hDefIcon);

	// init contact
	psh._hContact = wParam;
	if (psh._hContact == NULL) {
		// mark owner icons as initiated
		bInitIcons |= INIT_ICONS_OWNER;
		psh._pszProto = nullptr;
		psh._pszPrefix = nullptr;
	}
	else {
		// get contact's protocol
		psh._pszPrefix = psh._pszProto = Proto_GetBaseAccountName(wParam);
		if (psh._pszProto == nullptr) {
			MsgErr(nullptr, LPGENW("Could not find contact's protocol. Maybe it is not active!"));
			return 1;
		}
		// prepare scanning for metacontact's subcontact's pages
		if (bScanMetaSubContacts = DB::Module::IsMetaAndScan(psh._pszProto))
			psh._dwFlags |= PSF_PROTOPAGESONLY_INIT;
	}

	// add the pages
	NotifyEventHooks(g_hDetailsInitEvent, (WPARAM)&psh, wParam);
	if (!psh._pPages || !psh._numPages) {
		MsgErr(nullptr, LPGENW("No pages have been added. Canceling dialog creation!"));
		return 1;
	}

	// metacontacts sub pages
	if (bScanMetaSubContacts) {
		int numSubs = db_mc_getSubCount(wParam);

		psh._dwFlags &= ~PSF_PROTOPAGESONLY_INIT;
		psh._dwFlags |= PSF_PROTOPAGESONLY;
		for (int i = 0; i < numSubs; i++) {
			psh._hContact = db_mc_getSub(wParam, i);
			psh._nSubContact = i;
			if (psh._hContact) {
				psh._pszProto = Proto_GetBaseAccountName(psh._hContact);
				if ((INT_PTR)psh._pszProto != CALLSERVICE_NOTFOUND)
					NotifyEventHooks(g_hDetailsInitEvent, (WPARAM)&psh, (LPARAM)psh._hContact);
			}
		}
		psh._hContact = wParam;
	}

	// sort the pages by the position read from database
	if (!(psh._dwFlags & PSTVF_SORTTREE))
		qsort(psh._pPages, psh._numPages, sizeof(CPsTreeItem*), (int(*)(const void*, const void*))SortProc);

	// create the dialog itself
	if (!CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DETAILS), nullptr, DlgProc, (LPARAM)&psh))
		MsgErr(nullptr, LPGENW("Details dialog failed to be created. Returning error is %d."), GetLastError());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	AddPage()
// @desc	this adds a new pages
// @param wParam - The List of pages we want to add the new one to
// @param lParam - it's the page to add

static INT_PTR AddPage(WPARAM wParam, LPARAM lParam)
{
	CPsHdr *pPsh = (CPsHdr *)wParam;
	USERINFOPAGE *uip = (USERINFOPAGE*)lParam;

	// check size of the handled structures
	if (pPsh == nullptr || uip == nullptr)
		return 1;

	// try to check whether the flag member is initialized or not
	uip->flags = uip->flags > (ODPF_UNICODE | ODPF_BOLDGROUPS | ODPF_ICON | PSPF_PROTOPREPENDED) ? 0 : uip->flags;

	if (pPsh->_dwFlags & (PSF_PROTOPAGESONLY | PSF_PROTOPAGESONLY_INIT)) {
		uint8_t bIsUnicode = (uip->flags & ODPF_UNICODE) == ODPF_UNICODE;
		wchar_t *ptszTitle = bIsUnicode ? mir_wstrdup(uip->szTitle.w) : mir_a2u(uip->szTitle.a);

		// avoid adding pages for a meta subcontact, which have been added for a metacontact.
		if (pPsh->_dwFlags & PSF_PROTOPAGESONLY) {
			if (pPsh->_ignore.getIndex(ptszTitle) != -1) {
				mir_free(ptszTitle);
				return 0;
			}
		}
		// init ignore list with pages added by metacontact
		else if (pPsh->_dwFlags & PSF_PROTOPAGESONLY_INIT)
			pPsh->_ignore.insert(mir_wstrdup(ptszTitle));

		mir_free(ptszTitle);
	}

	// create the new tree item
	CPsTreeItem *pNew = new CPsTreeItem();
	if (pNew) {
		if (pNew->Create(pPsh, uip)) {
			delete pNew;
			return 1;
		}

		// resize the array
		pPsh->_pPages = (CPsTreeItem **)mir_realloc(pPsh->_pPages, (pPsh->_numPages + 1) * sizeof(CPsTreeItem*));
		if (pPsh->_pPages != nullptr) {
			pPsh->_pPages[pPsh->_numPages++] = pNew;
			return 0;
		}
		pPsh->_numPages = 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	OnDeleteContact()
// @desc	a user was deleted, so need to close its details dialog, if one open

static int OnDeleteContact(WPARAM wParam, LPARAM)
{
	HWND hWnd = WindowList_Find(g_hWindowList, wParam);
	if (hWnd != nullptr)
		DestroyWindow(hWnd);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	OnShutdown()
// @desc	we need to emptify the windowlist

static int OnShutdown(WPARAM, LPARAM)
{
	WindowList_BroadcastAsync(g_hWindowList, WM_DESTROY, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	InitDetails
// @desc	is called by Miranda if user selects to display the userinfo dialog
// @param	wParam		- the propertysheet init structure to pass
// @param	lParam		- handle to contact whose information are read

static int InitDetails(WPARAM wParam, LPARAM lParam)
{
	CPsHdr *pPsh = (CPsHdr *)wParam;
	if (!(pPsh->_dwFlags & PSF_PROTOPAGESONLY)) {
		bool bChangeDetailsEnabled = myGlobals.CanChangeDetails && g_plugin.bChangeDetails;
		if (lParam || bChangeDetailsEnabled) {
			USERINFOPAGE uip = {};
			uip.flags = ODPF_ICON | ODPF_UNICODE;
			uip.szGroup.w = IcoLib_GetDefaultIconFileName();
			uip.pPlugin = &g_plugin;

			if (lParam) {
				// ignore common pages for weather contacts
				if (!pPsh->_pszProto || _stricmp(pPsh->_pszProto, "weather")) {
					InitGeneralDlg(wParam, uip);
					InitContactDlg(wParam, uip);
					InitOriginDlg(wParam, uip);
					InitAnniversaryDlg(wParam, uip);
					InitCompanyDlg(wParam, uip);
					InitOriginDlg(wParam, uip, false);
					InitProfileDlg(wParam, uip);
				}
				else InitOriginDlg(wParam, uip, true);
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	InitTreeIcons()
// @desc	initalize all treeview icons

void DlgContactInfoInitTreeIcons()
{
	// make sure this is run only once
	if (!(bInitIcons & INIT_ICONS_ALL)) {
		CPsHdr psh;
		POINT metrics = { 0 };

		psh._dwFlags = PSTVF_INITICONS;

		metrics.x = GetSystemMetrics(SM_CXSMICON);
		metrics.y = GetSystemMetrics(SM_CYSMICON);
		if (psh._hImages = ImageList_Create(metrics.x, metrics.y, ILC_COLOR32 | ILC_MASK, 0, 1)) {
			HICON hDefIcon = g_plugin.getIcon(IDI_LST_FOLDER);
			if (!hDefIcon)
				hDefIcon = (HICON)g_plugin.getIcon(IDI_DEFAULT, false);

			// add the default icon to imagelist
			ImageList_AddIcon(psh._hImages, hDefIcon);
		}

		// avoid pages from loading doubled
		if (!(bInitIcons & INIT_ICONS_CONTACT)) {
			psh._dwFlags |= PSF_PROTOPAGESONLY_INIT;

			// enumerate all protocols
			for (auto &pa : Accounts()) {
				// enumerate all contacts
				for (auto &cc: Contacts(pa->szModuleName)) {
					// compare contact's protocol to the current one, to add
					LPCSTR pszContactProto = Proto_GetBaseAccountName(cc);
					if ((INT_PTR)pszContactProto != CALLSERVICE_NOTFOUND && !mir_strcmp(pa->szModuleName, pszContactProto)) {
						// call a notification for the contact to retrieve all protocol specific tree items
						psh._hContact = cc;
						NotifyEventHooks(g_hDetailsInitEvent, (WPARAM)&psh, (LPARAM)psh._hContact);
						if (psh._pPages) {
							psh.Free_pPages();
							psh._dwFlags = PSTVF_INITICONS | PSF_PROTOPAGESONLY;
						}
						break;
					}
				}
			}
			bInitIcons |= INIT_ICONS_CONTACT;
		}
		// load all treeitems for owner contact
		if (!(bInitIcons & INIT_ICONS_OWNER)) {
			psh._hContact = NULL;
			psh._pszProto = nullptr;
			NotifyEventHooks(g_hDetailsInitEvent, (WPARAM)&psh, (LPARAM)psh._hContact);
			if (psh._pPages)
				psh.Free_pPages();

			bInitIcons |= INIT_ICONS_OWNER;
		}
		ImageList_Destroy(psh._hImages);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// PropertySheet's Dialog Procedures

static void ResetUpdateInfo(LPPS pPs)
{
	// free the array of accomblished acks
	for (int i = 0; i < (int)pPs->nSubContacts; i++)
		MIR_FREE(pPs->infosUpdated[i].acks);
	MIR_FREE(pPs->infosUpdated);
	pPs->nSubContacts = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	DlgProc()
// @desc	dialog procedure for the main propertysheet dialog box

static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPPS pPs = (LPPS)GetUserData(hDlg);

	// do not process any message if pPs is no longer existent
	if (!PtrIsValid(pPs) && uMsg != WM_INITDIALOG)
		return FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		{
			CPsHdr *pPsh = (CPsHdr *)lParam;
			if (!pPsh)
				return FALSE;

			// create data structures
			if (!(pPs = (LPPS)mir_alloc(sizeof(PS))))
				return FALSE;

			memset(pPs, 0, sizeof(PS));
			if (!(pPs->pTree = new CPsTree(pPs)))
				return FALSE;

			if (!(pPs->pTree->Create(GetDlgItem(hDlg, STATIC_TREE), pPsh)))
				return FALSE;

			SetUserData(hDlg, pPs);
			pPs->hDlg = hDlg;
			pPs->dwFlags |= PSF_LOCKED;
			pPs->hContact = pPsh->_hContact;
			pPs->hProtoAckEvent = HookEventMessage(ME_PROTO_ACK, hDlg, HM_PROTOACK);
			pPs->hModuleUnloaded = HookEventMessage(ME_SYSTEM_MODULEUNLOAD, hDlg, HM_UNLOADED);
			pPs->hSettingChanged = HookEventMessage(ME_DB_CONTACT_SETTINGCHANGED, hDlg, HM_SETTING_CHANGED);
			pPs->hIconsChanged = HookEventMessage(ME_SKIN_ICONSCHANGED, hDlg, HM_RELOADICONS);

			ShowWindow(GetDlgItem(hDlg, IDC_PAGETITLEBG), IsAeroMode());
			ShowWindow(GetDlgItem(hDlg, IDC_PAGETITLEBG2), !IsAeroMode());

			// set icons
			Window_SetIcon_IcoLib(hDlg, g_plugin.getIconHandle(IDI_MAIN));
			DlgProc(hDlg, HM_RELOADICONS, NULL, NULL);

			// load basic protocol for current contact (for faster load later on and better handling for owner protocol)
			if (pPs->hContact)
				mir_strncpy(pPs->pszProto, pPsh->_pszPrefix, MAXMODULELABELLENGTH);

			// set the windowtitle
			DlgProc(hDlg, HM_SETWINDOWTITLE, NULL, NULL);

			// translate Userinfo buttons
			SendDlgItemMessage(hDlg, BTN_UPDATE, BUTTONTRANSLATE, NULL, NULL);
			SendDlgItemMessage(hDlg, IDOK, BUTTONTRANSLATE, NULL, NULL);
			SendDlgItemMessage(hDlg, IDCANCEL, BUTTONTRANSLATE, NULL, NULL);
			SendDlgItemMessage(hDlg, IDAPPLY, BUTTONTRANSLATE, NULL, NULL);
			SendDlgItemMessage(hDlg, BTN_EXPORT, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Export to file"), MBBF_TCHAR);
			SendDlgItemMessage(hDlg, BTN_IMPORT, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Import from file"), MBBF_TCHAR);

			// set bold font for name in description area
			LOGFONT lf;
			HFONT hNormalFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);

			GetObject(hNormalFont, sizeof(lf), &lf);
			lf.lfHeight = 22;
			mir_wstrcpy(lf.lfFaceName, L"Segoe UI");
			pPs->hCaptionFont = CreateFontIndirect(&lf);
			SendDlgItemMessage(hDlg, IDC_PAGETITLE, WM_SETFONT, (WPARAM)pPs->hCaptionFont, 0);

			GetObject(hNormalFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			pPs->hBoldFont = CreateFontIndirect(&lf);

			// initialize the optionpages and tree control
			uint16_t needWidth = 0;
			if (!pPs->pTree->InitTreeItems(&needWidth))
				return FALSE;

			// move and resize dialog and its controls
			{
				RECT rcTree;
				POINT pt = { 0, 0 };
				int addWidth = 0;

				// at least add width of scrollbar
				needWidth += 8 + GetSystemMetrics(SM_CXVSCROLL);

				// get tree rectangle
				GetWindowRect(hDlg, &pPs->rcDisplay);
				GetWindowRect(pPs->pTree->Window(), &rcTree);
				ClientToScreen(hDlg, &pt);
				OffsetRect(&rcTree, -pt.x, -pt.y);

				// calculate the amout of pixels to resize the dialog by?
				if (needWidth > rcTree.right - rcTree.left) {
					RECT rcMax = { 0, 0, 0, 0 };

					rcMax.right = 280;
					MapDialogRect(hDlg, &rcMax);
					addWidth = min(needWidth, rcMax.right) - rcTree.right + rcTree.left;
					rcTree.right += addWidth;
					// resize tree
					MoveWindow(pPs->pTree->Window(), rcTree.left, rcTree.top, rcTree.right - rcTree.left, rcTree.bottom - rcTree.top, FALSE);

					pPs->rcDisplay.right += addWidth;
					MoveWindow(hDlg, pPs->rcDisplay.left, pPs->rcDisplay.top,
						pPs->rcDisplay.right - pPs->rcDisplay.left,
						pPs->rcDisplay.bottom - pPs->rcDisplay.top, FALSE);

				}

				RECT rc;
				GetClientRect(GetDlgItem(hDlg, IDC_PAGETITLEBG), &rc);

				// calculate dislpay area for pages
				OffsetRect(&pPs->rcDisplay, -pt.x, -pt.y);
				pPs->rcDisplay.bottom = rcTree.bottom;
				pPs->rcDisplay.left = rcTree.right + 2;
				pPs->rcDisplay.top = rcTree.top + rc.bottom;
				pPs->rcDisplay.right -= 2;

				// move and resize the rest of the controls
				if (addWidth > 0) {
					static const uint16_t idResize[] = { IDC_HEADERBAR, STATIC_LINE2 };
					static const uint16_t idMove[] = { IDC_PAGETITLE, IDC_PAGETITLEBG, IDC_PAGETITLEBG2, IDOK, IDCANCEL, IDAPPLY };
					HWND hCtrl;

					for (auto &it : idResize) {
						if (hCtrl = GetDlgItem(hDlg, it)) {
							GetWindowRect(hCtrl, &rc);
							OffsetRect(&rc, -pt.x, -pt.y);
							MoveWindow(hCtrl, rc.left, rc.top, rc.right - rc.left + addWidth, rc.bottom - rc.top, FALSE);
						}
					}
					for (auto &it : idMove) {
						if (hCtrl = GetDlgItem(hDlg, it)) {
							GetWindowRect(hCtrl, &rc);
							OffsetRect(&rc, -pt.x, -pt.y);
							MoveWindow(hCtrl, rc.left + addWidth, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
						}
					}
				}
				// restore window position and add required size
				Utils_RestoreWindowPositionNoSize(hDlg, NULL, MODULENAME, "DetailsDlg");
			}

			//
			// show the first propsheetpage
			//
			// finally add the dialog to the window list
			WindowList_Add(g_hWindowList, hDlg, pPs->hContact);

			// show the dialog
			pPs->dwFlags &= ~PSF_LOCKED;
			pPs->dwFlags |= PSF_INITIALIZED;

			//
			// initialize the "updating" button and statustext and check for online status
			//
			pPs->updateAnimFrame = 0;
			if (pPs->hContact && *pPs->pszProto) {
				GetDlgItemTextA(hDlg, TXT_UPDATING, pPs->szUpdating, _countof(pPs->szUpdating));
				ShowWindow(GetDlgItem(hDlg, TXT_UPDATING), SW_HIDE);

				if (DlgProc(hDlg, M_CHECKONLINE, NULL, NULL))
					DlgProc(hDlg, WM_COMMAND, MAKEWPARAM(BTN_UPDATE, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, BTN_UPDATE));
			}

			int nPage = pPs->pTree->CurrentItemIndex();
			if (!pPs->pTree->IsIndexValid(nPage))
				nPage = 0;
			TreeView_Select(pPs->pTree->Window(), NULL, TVGN_CARET);
			TreeView_Select(pPs->pTree->Window(), pPs->pTree->TreeItemHandle(nPage), TVGN_CARET);
			ShowWindow(hDlg, SW_SHOW);
		}
		return TRUE;

	case WM_TIMER: // is called to display the "updating" text in the status area
		switch (wParam) {
		case TIMERID_UPDATING:
			SetDlgItemTextA(hDlg, TXT_UPDATING, CMStringA().Format("%.*s%s%.*s", pPs->updateAnimFrame % 10, ".........", pPs->szUpdating, pPs->updateAnimFrame % 10, "........."));
			if (++pPs->updateAnimFrame == UPDATEANIMFRAMES)
				pPs->updateAnimFrame = 0;
			return FALSE;
		}
		break;

	case WM_DWMCOMPOSITIONCHANGED:
		ShowWindow(GetDlgItem(hDlg, IDC_PAGETITLEBG), IsAeroMode());
		InvalidateRect(hDlg, nullptr, TRUE);
		break;

	case WM_CTLCOLORSTATIC: // sets the colour of some of the dialog's static controls
		switch (GetWindowLongPtr((HWND)lParam, GWLP_ID)) {
		case TXT_UPDATING:
			{
				COLORREF textCol = GetSysColor(COLOR_BTNTEXT);
				COLORREF bgCol = GetSysColor(COLOR_3DFACE);
				int ratio = abs(UPDATEANIMFRAMES / 2 - pPs->updateAnimFrame) * 510 / UPDATEANIMFRAMES;
				COLORREF newCol = RGB(GetRValue(bgCol) + (GetRValue(textCol) - GetRValue(bgCol)) * ratio / 256,
					GetGValue(bgCol) + (GetGValue(textCol) - GetGValue(bgCol)) * ratio / 256,
					GetBValue(bgCol) + (GetBValue(textCol) - GetBValue(bgCol)) * ratio / 256);
				SetTextColor((HDC)wParam, newCol);
				SetBkColor((HDC)wParam, GetSysColor(COLOR_3DFACE));
			}
			return (INT_PTR)GetSysColorBrush(COLOR_3DFACE);
		case IDC_PAGETITLE:
		case IDC_PAGETITLEBG:
			if (IsAeroMode()) {
				SetTextColor((HDC)wParam, RGB(0, 90, 180));
				SetBkColor((HDC)wParam, RGB(255, 255, 255));
				return (INT_PTR)GetStockObject(WHITE_BRUSH);
			}

			SetBkColor((HDC)wParam, GetSysColor(COLOR_3DFACE));
			return (INT_PTR)GetSysColorBrush(COLOR_3DFACE);
		}
		SetBkMode((HDC)wParam, TRANSPARENT);
		return (INT_PTR)GetStockObject(NULL_BRUSH);

	case PSM_CHANGED: // indicates the propertysheet and the current selected page as changed
		if (!(pPs->dwFlags & PSF_LOCKED)) {
			pPs->dwFlags |= PSF_CHANGED;
			pPs->pTree->CurrentItem()->AddFlags(PSPF_CHANGED);
			EnableWindow(GetDlgItem(hDlg, IDAPPLY), TRUE);
			return TRUE;
		}
		break;

	case PSM_GETBOLDFONT: // returns the bold font
		if (pPs->hBoldFont && lParam) {
			*(HFONT *)lParam = pPs->hBoldFont;
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)pPs->hBoldFont);
			return TRUE;
		}
		*(HFONT *)lParam = nullptr;
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
		break;

	case PSM_GETCONTACT: // returns the handle to the contact, associated with this propertysheet
		if (lParam) {
			// prefer to return the contact accociated with the current page
			CPsTreeItem *pti = ((int)wParam != -1) ? pPs->pTree->TreeItem((int)wParam) : pPs->pTree->CurrentItem();
			if (pti && pti->hContact() != INVALID_CONTACT_ID) {
				*(MCONTACT *)lParam = pti->hContact();
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)pti->hContact());
				return TRUE;
			}

			// return contact who owns the details dialog
			if (pPs->hContact != INVALID_CONTACT_ID) {
				*(MCONTACT *)lParam = pPs->hContact;
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)pPs->hContact);
				return TRUE;
			}
			*(HANDLE *)lParam = nullptr;
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, NULL);
		}
		break;

	case PSM_GETBASEPROTO: // returns the basic protocol module for the associated contact
		if (lParam) {
			CPsTreeItem *pti = ((int)wParam != -1) ? pPs->pTree->TreeItem((int)wParam) : pPs->pTree->CurrentItem();
			if (pti && pti->Proto()) {
				// return custom protocol for the current page
				*(LPCSTR *)lParam = pti->Proto();
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)pti->Proto());
				return TRUE;
			}

			if (*pPs->pszProto) {
				// return global protocol
				*(LPSTR *)lParam = pPs->pszProto;
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)pPs->pszProto);
				return TRUE;
			}
		}
		*(LPCSTR *)lParam = nullptr;
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
		break;

	case PSM_ISLOCKED: // returns the lock state of the propertysheetpage
		{
			uint8_t bLocked = (pPs->dwFlags & PSF_LOCKED) == PSF_LOCKED;
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, bLocked);
			return bLocked;
		}

	case PSM_FORCECHANGED: // force all propertysheetpages to update their controls with new values from the database
		if (!(pPs->dwFlags & PSF_LOCKED)) {
			uint8_t bChanged;

			pPs->dwFlags |= PSF_LOCKED;
			if (bChanged = pPs->pTree->OnInfoChanged())
				pPs->dwFlags |= PSF_CHANGED;
			else
				pPs->dwFlags &= ~PSF_CHANGED;
			pPs->dwFlags &= ~PSF_LOCKED;
			EnableWindow(GetDlgItem(hDlg, IDAPPLY), bChanged);
		}
		break;

	case PSM_DLGMESSAGE: // Sends a message to a specified propertysheetpage
		{
			LPDLGCOMMAND pCmd = (LPDLGCOMMAND)lParam;
			CPsTreeItem *pti;

			if (pCmd && (pti = pPs->pTree->FindItemByResource(pCmd->hInst, pCmd->idDlg)) && pti->Wnd()) {
				if (!pCmd->idDlgItem)
					return SendMessage(pti->Wnd(), pCmd->uMsg, pCmd->wParam, pCmd->lParam);
				else
					return SendDlgItemMessage(pti->Wnd(), pCmd->idDlgItem, pCmd->uMsg, pCmd->wParam, pCmd->lParam);
			}
		}
		return E_FAIL;

	case PSM_GETPAGEHWND: // get the window handle for a specified propertysheetpage
		if (CPsTreeItem *pti = pPs->pTree->FindItemByResource((HINSTANCE)lParam, wParam)) {
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)pti->Wnd());
			return (pti->Wnd() != nullptr);
		}
		return FALSE;

	case PSM_ISAEROMODE:
		{
			uint8_t bIsAeroMode = IsAeroMode();
			if (lParam)
				*(uint8_t *)lParam = bIsAeroMode;
			return (INT_PTR)bIsAeroMode;
		}

	case HM_SETWINDOWTITLE: // set the window title and text of the infobar
		{
			DBCONTACTWRITESETTING *pdbcws = (DBCONTACTWRITESETTING *)lParam;

			LPCTSTR pszName;
			if (!pPs->hContact)
				pszName = TranslateT("Owner");
			else if (pdbcws && pdbcws->value.type == DBVT_WCHAR)
				pszName = pdbcws->value.pwszVal;
			else
				pszName = Clist_GetContactDisplayName(pPs->hContact);

			HWND hName = GetDlgItem(hDlg, TXT_NAME);
			SetWindowText(hName, pszName);
			SetWindowText(hDlg, CMStringW(FORMAT, L"%s - %s", pszName, TranslateT("edit contact information")));
			SetDlgItemText(hDlg, IDC_HEADERBAR, TranslateT("View personal user details and more"));

			// redraw the name control
			POINT pt = { 0, 0 };
			ScreenToClient(hDlg, &pt);
			RECT rc;
			GetWindowRect(hName, &rc);
			OffsetRect(&rc, pt.x, pt.y);
			InvalidateRect(hDlg, &rc, TRUE);
			break;
		}

	case HM_RELOADICONS: // handles the changed icon event from the icolib plugin and reloads all icons
		{
			HWND hCtrl;
			HICON hIcon;
			static const ICONCTRL idIcon[] = {
				{ IDI_DLG_DETAILS,	STM_SETIMAGE,	ICO_DLGLOGO	},
				{ IDI_BTN_UPDATE,	BM_SETIMAGE,	BTN_UPDATE	},
				{ IDI_BTN_OK,		BM_SETIMAGE,	IDOK		},
				{ IDI_BTN_CLOSE,	BM_SETIMAGE,	IDCANCEL	},
				{ IDI_BTN_APPLY,	BM_SETIMAGE,	IDAPPLY		}
			};

			const int numIconsToSet = g_plugin.bButtonIcons ? _countof(idIcon) : 1;

			IcoLib_SetCtrlIcons(hDlg, idIcon, numIconsToSet);

			if (hCtrl = GetDlgItem(hDlg, BTN_IMPORT)) {
				hIcon = g_plugin.getIcon(IDI_IMPORT);
				SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				SetWindowText(hCtrl, hIcon ? L"" : L"I");
			}
			if (hCtrl = GetDlgItem(hDlg, BTN_EXPORT)) {
				hIcon = g_plugin.getIcon(IDI_EXPORT);
				SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				SetWindowText(hCtrl, hIcon ? L"" : L"E");
			}
			// update page icons
			if (PtrIsValid(pPs) && (pPs->dwFlags & PSF_INITIALIZED))
				pPs->pTree->OnIconsChanged();
			break;
		}

	case M_CHECKONLINE: // determines whether miranda is online or not
		if (IsProtoOnline(pPs->pszProto)) {
			EnableWindow(GetDlgItem(hDlg, BTN_UPDATE), !IsWindowVisible(GetDlgItem(hDlg, TXT_UPDATING)));
			return TRUE;
		}

		EnableWindow(GetDlgItem(hDlg, BTN_UPDATE), FALSE);
		EnableWindow(GetDlgItem(hDlg, TXT_UPDATING), FALSE);
		break;

	case HM_UNLOADED:
		pPs->pTree->Remove((HINSTANCE)lParam);
		break;

	case HM_PROTOACK: // handles all acks from the protocol plugin
		{
			ACKDATA *ack = (ACKDATA *)lParam;
			int i, iSubContact = 0;

			if (!ack->hContact && ack->type == ACKTYPE_STATUS)
				return DlgProc(hDlg, M_CHECKONLINE, NULL, NULL);

			switch (ack->type) {
			case ACKTYPE_SETINFO:
				if (ack->hContact != pPs->hContact || !pPs->pUpload || pPs->pUpload->Handle() != ack->hProcess)
					break;
				if (ack->result == ACKRESULT_SUCCESS) {
					ShowWindow(GetDlgItem(hDlg, TXT_UPDATING), SW_HIDE);
					KillTimer(hDlg, TIMERID_UPDATING);
					// upload next protocols contact information
					switch (pPs->pUpload->UploadNext()) {
					case CPsUpload::UPLOAD_FINISH_CLOSE:
						MIR_DELETE(pPs->pUpload);
						DestroyWindow(hDlg);
					case CPsUpload::UPLOAD_CONTINUE:
						return FALSE;
					case CPsUpload::UPLOAD_FINISH:
						MIR_DELETE(pPs->pUpload);
						break;
					}
					DlgProc(hDlg, M_CHECKONLINE, NULL, NULL);
					EnableWindow(pPs->pTree->Window(), TRUE);
					if (CPsTreeItem *pti = pPs->pTree->CurrentItem())
						EnableWindow(pti->Wnd(), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
					pPs->dwFlags &= ~PSF_LOCKED;
				}
				else if (ack->result == ACKRESULT_FAILED) {
					MsgBox(hDlg, MB_ICON_WARNING,
						LPGENW("Upload ICQ details"),
						LPGENW("Upload failed"),
						LPGENW("Your details were not uploaded successfully.\nThey were written to database only."));
					KillTimer(hDlg, TIMERID_UPDATING);
					ShowWindow(GetDlgItem(hDlg, TXT_UPDATING), SW_HIDE);
					DlgProc(hDlg, M_CHECKONLINE, NULL, NULL);

					// upload next protocols contact information
					switch (pPs->pUpload->UploadNext()) {
					case CPsUpload::UPLOAD_FINISH_CLOSE:
						MIR_DELETE(pPs->pUpload);
						DestroyWindow(hDlg);
					case CPsUpload::UPLOAD_CONTINUE:
						return 0;
					case CPsUpload::UPLOAD_FINISH:
						MIR_DELETE(pPs->pUpload);
						break;
					}
					if (CPsTreeItem *pti = pPs->pTree->CurrentItem())
						EnableWindow(pti->Wnd(), TRUE);

					// activate all controls again
					EnableWindow(pPs->pTree->Window(), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
					pPs->dwFlags &= ~PSF_LOCKED;
				}
				break;

			case ACKTYPE_GETINFO:
				// is contact the owner of the dialog or any metasubcontact of the owner? skip handling otherwise!
				if (ack->hContact != pPs->hContact) {
					if (!g_plugin.bMetaScan)
						break;

					for (i = 0; i < pPs->nSubContacts; i++) {
						if (pPs->infosUpdated[i].hContact == ack->hContact) {
							iSubContact = i;
							break;
						}
					}
					if (i == pPs->nSubContacts)
						break;
				}
				else
					iSubContact = 0;

				// if they're not gonna send any more ACK's don't let that mean we should crash
				if (!pPs->infosUpdated || (!ack->hProcess && !ack->lParam)) {
					ResetUpdateInfo(pPs);

					ShowWindow(GetDlgItem(hDlg, TXT_UPDATING), SW_HIDE);
					KillTimer(hDlg, TIMERID_UPDATING);
					DlgProc(hDlg, M_CHECKONLINE, NULL, NULL);
					break;
				}

				if (iSubContact < pPs->nSubContacts) {
					// init the acks structure for a sub contact
					if (pPs->infosUpdated[iSubContact].acks == nullptr) {
						pPs->infosUpdated[iSubContact].acks = (LPINT)mir_calloc(sizeof(int) * (int)(INT_PTR)ack->hProcess);
						pPs->infosUpdated[iSubContact].count = (int)(INT_PTR)ack->hProcess;
					}

					if (ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED)
						pPs->infosUpdated[iSubContact].acks[ack->lParam] = 1;

					// check for pending tasks
					for (iSubContact = 0; iSubContact < pPs->nSubContacts; iSubContact++) {
						for (i = 0; i < pPs->infosUpdated[iSubContact].count; i++)
							if (pPs->infosUpdated[iSubContact].acks[i] == 0)
								break;

						if (i < pPs->infosUpdated[iSubContact].count)
							break;
					}
				}

				// all acks are done, finish updating
				if (iSubContact >= pPs->nSubContacts) {
					ResetUpdateInfo(pPs);
					ShowWindow(GetDlgItem(hDlg, TXT_UPDATING), SW_HIDE);
					KillTimer(hDlg, TIMERID_UPDATING);
					DlgProc(hDlg, M_CHECKONLINE, NULL, NULL);
				}
			}
			break;
		}

	case HM_SETTING_CHANGED:
		// This message is called by the ME_DB_CONTACT_SETTINGCHANGED event and forces all
		// unedited settings in the propertysheetpages to be updated
		if (!(pPs->dwFlags & PSF_LOCKED)) {
			MCONTACT hContact = wParam;
			DBCONTACTWRITESETTING *pdbcws = (DBCONTACTWRITESETTING *)lParam;

			if (hContact != pPs->hContact) {
				if (pPs->hContact != db_mc_getMeta(hContact))
					break;
				if (!g_plugin.bMetaScan)
					break;
			}

			if (!strcmp(pdbcws->szSetting, SET_CONTACT_MYHANDLE) || !strcmp(pdbcws->szSetting, SET_CONTACT_NICK)) {
				// force the update of all propertysheetpages
				DlgProc(hDlg, PSM_FORCECHANGED, NULL, NULL);
				// update the windowtitle
				DlgProc(hDlg, HM_SETWINDOWTITLE, NULL, lParam);
			}
			else if (!strcmp(pdbcws->szModule, USERINFO) || !strcmp(pdbcws->szModule, pPs->pszProto)) {
				// force the update of all propertysheetpages
				DlgProc(hDlg, PSM_FORCECHANGED, NULL, NULL);
			}
		}
		break;

	case WM_NOTIFY:
		switch (wParam) { 
		case STATIC_TREE: // Notification Messages sent by the TreeView
			switch (((LPNMHDR)lParam)->code) {
			case TVN_SELCHANGING:
				pPs->dwFlags |= PSF_LOCKED;
				pPs->pTree->OnSelChanging();
				pPs->dwFlags &= ~PSF_LOCKED;
				break;

			case TVN_SELCHANGED:
				if (pPs->dwFlags & PSF_INITIALIZED) {
					pPs->dwFlags |= PSF_LOCKED;
					pPs->pTree->OnSelChanged((LPNMTREEVIEW)lParam);
					if (pPs->pTree->CurrentItem()) {
						RECT rc;
						POINT pt = { 0, 0 };

						GetWindowRect(GetDlgItem(hDlg, IDC_PAGETITLE), &rc);
						ScreenToClient(hDlg, &pt);
						OffsetRect(&rc, pt.x, pt.y);
						SetDlgItemText(hDlg, IDC_PAGETITLE, pPs->pTree->CurrentItem()->Label());
						InvalidateRect(GetDlgItem(hDlg, IDC_PAGETITLEBG), &rc, TRUE);
						InvalidateRect(hDlg, &rc, TRUE);
					}
					pPs->dwFlags &= ~PSF_LOCKED;
				}
				break;

			case TVN_BEGINDRAG:
				{
					LPNMTREEVIEW nmtv = (LPNMTREEVIEW)lParam;
					if (nmtv->itemNew.hItem == TreeView_GetSelection(nmtv->hdr.hwndFrom)) {
						SetCapture(hDlg);
						pPs->pTree->BeginDrag(nmtv->itemNew.hItem);
					}
					TreeView_SelectItem(nmtv->hdr.hwndFrom, nmtv->itemNew.hItem);
				}
				break;

			case TVN_ITEMEXPANDED:
				pPs->pTree->AddFlags(PSTVF_STATE_CHANGED);
				break;

			case NM_KILLFOCUS:
				KillTimer(hDlg, TIMERID_RENAME);
				break;

			case NM_CLICK:
				TVHITTESTINFO hti;
				GetCursorPos(&hti.pt);
				ScreenToClient(pPs->pTree->Window(), &hti.pt);
				TreeView_HitTest(pPs->pTree->Window(), &hti);
				if ((hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) && hti.hItem == TreeView_GetSelection(pPs->pTree->Window()))
					SetTimer(hDlg, TIMERID_RENAME, 500, nullptr);
				break;

			case NM_RCLICK:
				pPs->pTree->PopupMenu();
				return 0;
			}
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if (pPs->pTree->IsDragging()) {
			TVHITTESTINFO hti;

			hti.pt.x = (SHORT)LOWORD(lParam);
			hti.pt.y = (SHORT)HIWORD(lParam);
			MapWindowPoints(hDlg, pPs->pTree->Window(), &hti.pt, 1);
			TreeView_HitTest(pPs->pTree->Window(), &hti);

			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				// check where over the item, the pointer is
				RECT rc;
				if (TreeView_GetItemRect(pPs->pTree->Window(), hti.hItem, &rc, FALSE)) {
					uint8_t height = (uint8_t)(rc.bottom - rc.top);

					if (hti.pt.y - (height / 3) < rc.top) {
						SetCursor(LoadCursor(nullptr, IDC_ARROW));
						TreeView_SetInsertMark(pPs->pTree->Window(), hti.hItem, 0);
					}
					else if (hti.pt.y + (height / 3) > rc.bottom) {
						SetCursor(LoadCursor(nullptr, IDC_ARROW));
						TreeView_SetInsertMark(pPs->pTree->Window(), hti.hItem, 1);
					}
					else {
						TreeView_SetInsertMark(pPs->pTree->Window(), NULL, 0);
						SetCursor(LoadCursor(g_plugin.getInst(), MAKEINTRESOURCE(CURSOR_ADDGROUP)));
					}
				}
			}
			else {
				if (hti.flags & TVHT_ABOVE) SendMessage(pPs->pTree->Window(), WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW) SendMessage(pPs->pTree->Window(), WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				TreeView_SetInsertMark(pPs->pTree->Window(), NULL, 0);
			}
		}
		break;

	case WM_LBUTTONUP:
		// drop item
		if (pPs->pTree->IsDragging()) {
			RECT rc;
			bool bAsChild = false;

			TreeView_SetInsertMark(pPs->pTree->Window(), NULL, 0);
			ReleaseCapture();
			SetCursor(LoadCursor(nullptr, IDC_ARROW));

			TVHITTESTINFO hti;
			hti.pt.x = (SHORT)LOWORD(lParam);
			hti.pt.y = (SHORT)HIWORD(lParam);
			MapWindowPoints(hDlg, pPs->pTree->Window(), &hti.pt, 1);
			TreeView_HitTest(pPs->pTree->Window(), &hti);

			if (hti.hItem == pPs->pTree->DragItem()) {
				pPs->pTree->EndDrag();
				break;
			}

			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;
			else if (hti.flags & (TVHT_NOWHERE | TVHT_BELOW))
				hti.hItem = TVI_LAST;
			else if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				// check where over the item, the pointer is
				if (!TreeView_GetItemRect(pPs->pTree->Window(), hti.hItem, &rc, FALSE)) {
					pPs->pTree->EndDrag();
					break;
				}
				uint8_t height = (uint8_t)(rc.bottom - rc.top);

				if (hti.pt.y - (height / 3) < rc.top) {
					HTREEITEM hItem = hti.hItem;

					if (!(hti.hItem = TreeView_GetPrevSibling(pPs->pTree->Window(), hItem))) {
						if (!(hti.hItem = TreeView_GetParent(pPs->pTree->Window(), hItem)))
							hti.hItem = TVI_FIRST;
						else
							bAsChild = true;
					}
				}
				else if (hti.pt.y + (height / 3) <= rc.bottom)
					bAsChild = true;
			}
			pPs->pTree->MoveItem(pPs->pTree->DragItem(), hti.hItem, bAsChild);
			pPs->pTree->EndDrag();
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pPs->pTree->OnCancel();
			DestroyWindow(hDlg);
			break;

		case IDOK:
		case IDAPPLY:
			if (pPs->dwFlags & PSF_CHANGED) {
				// kill focus from children to make sure all data can be saved (ComboboxEx)
				SetFocus(hDlg);

				pPs->dwFlags |= PSF_LOCKED;
				if (pPs->pTree->OnApply()) {
					pPs->dwFlags &= ~(PSF_LOCKED | PSF_CHANGED);
					break;
				}

				pPs->dwFlags &= ~PSF_CHANGED;
				EnableWindow(GetDlgItem(hDlg, IDAPPLY), FALSE);
				g_clistApi.pfnInvalidateDisplayNameCacheEntry(pPs->hContact);

				// need to upload owners settings
				if (!pPs->hContact && myGlobals.CanChangeDetails && g_plugin.bChangeDetails) {
					if (pPs->pUpload = new CPsUpload(pPs, LOWORD(wParam) == IDOK)) {
						if (pPs->pUpload->UploadFirst() == CPsUpload::UPLOAD_CONTINUE)
							break;
						MIR_DELETE(pPs->pUpload);
					}
				}
				pPs->dwFlags &= ~PSF_LOCKED;
			}
			if (LOWORD(wParam) == IDOK)
				DestroyWindow(hDlg);
			break;

		case BTN_UPDATE:
			if (pPs->hContact != NULL) {
				ResetUpdateInfo(pPs);

				mir_snprintf(pPs->szUpdating, "%s (%s)", Translate("updating"), pPs->pszProto);

				// need meta contact's subcontact information
				if (DB::Module::IsMetaAndScan(pPs->pszProto)) {
					// count valid subcontacts whose protocol supports the PSS_GETINFO service to update the information
					int numSubs = db_mc_getSubCount(pPs->hContact);
					for (int i = 0; i < numSubs; i++) {
						MCONTACT hSubContact = db_mc_getSub(pPs->hContact, i);
						if (hSubContact) {
							if (ProtoServiceExists(Proto_GetBaseAccountName(hSubContact), PSS_GETINFO)) {
								pPs->infosUpdated = (TAckInfo *)mir_realloc(pPs->infosUpdated, sizeof(TAckInfo) * (pPs->nSubContacts + 1));
								pPs->infosUpdated[pPs->nSubContacts].hContact = hSubContact;
								pPs->infosUpdated[pPs->nSubContacts].acks = nullptr;
								pPs->infosUpdated[pPs->nSubContacts].count = 0;
								pPs->nSubContacts++;
							}
						}
					}

					if (pPs->nSubContacts != 0) {
						uint8_t bDo = FALSE;

						// call the services
						for (int i = 0; i < pPs->nSubContacts; i++)
							if (!ProtoChainSend(pPs->infosUpdated[pPs->nSubContacts].hContact, PSS_GETINFO, NULL, NULL))
								bDo = TRUE;

						if (bDo) {
							EnableWindow(GetDlgItem(hDlg, BTN_UPDATE), FALSE);
							ShowWindow(GetDlgItem(hDlg, TXT_UPDATING), SW_SHOW);
							SetTimer(hDlg, TIMERID_UPDATING, 100, nullptr);
						}
					}
				}
				else if (!ProtoChainSend(pPs->hContact, PSS_GETINFO, NULL, NULL)) {
					pPs->infosUpdated = (TAckInfo *)mir_calloc(sizeof(TAckInfo));
					pPs->infosUpdated[0].hContact = pPs->hContact;
					pPs->nSubContacts = 1;

					EnableWindow(GetDlgItem(hDlg, BTN_UPDATE), FALSE);
					ShowWindow(GetDlgItem(hDlg, TXT_UPDATING), SW_SHOW);
					SetTimer(hDlg, TIMERID_UPDATING, 100, nullptr);
				}
			}
			break;

		case BTN_IMPORT:
			svcExIm_ContactImport_Service((WPARAM)pPs->hContact, 0);
			break;

		case BTN_EXPORT:
			// save changes before exporting data
			DlgProc(hDlg, WM_COMMAND, MAKEWPARAM(IDAPPLY, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, IDAPPLY));
			// do the exporting stuff
			svcExIm_ContactExport_Service((WPARAM)pPs->hContact, 0);
			break;
		}
		break;

	case WM_CLOSE:
		DlgProc(hDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, IDCANCEL));
		break;

	case WM_DESTROY:
		// hide before destroy
		ShowWindow(hDlg, SW_HIDE);

		ResetUpdateInfo(pPs);

		// avoid any further message processing for this dialog page
		Window_FreeIcon_IcoLib(hDlg);
		WindowList_Remove(g_hWindowList, hDlg);
		SetUserData(hDlg, NULL);

		// unhook events and stop timers
		KillTimer(hDlg, TIMERID_RENAME);
		UnhookEvent(pPs->hProtoAckEvent);
		UnhookEvent(pPs->hSettingChanged);
		UnhookEvent(pPs->hIconsChanged);
		UnhookEvent(pPs->hModuleUnloaded);

		// save my window position
		Utils_SaveWindowPosition(hDlg, NULL, MODULENAME, "DetailsDlg");

		// save current tree and destroy it
		if (pPs->pTree != nullptr) {
			// save tree's current look
			pPs->pTree->SaveState();
			delete pPs->pTree;
			pPs->pTree = nullptr;
		}

		DeleteObject(pPs->hCaptionFont);
		DeleteObject(pPs->hBoldFont);
		mir_free(pPs); pPs = nullptr;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	LoadModule()
// @desc	load the UserInfo Module

void DlgContactInfoLoadModule()
{
	// this event is already created inside the core, don't destroy it
	g_hDetailsInitEvent = CreateHookableEvent(ME_USERINFO_INITIALISE);

	CreateServiceFunction(MS_USERINFO_SHOWDIALOG, ShowDialog);
	CreateServiceFunction("UserInfo/AddPage", AddPage);

	HookEvent(ME_DB_CONTACT_DELETED, OnDeleteContact);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	HookEvent(ME_USERINFO_INITIALISE, InitDetails);
	g_hWindowList = WindowList_Create();

	// check whether changing my details via UserInfoEx is basically possible
	myGlobals.CanChangeDetails = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// @name	UnLoadModule()
// @desc	unload the UserInfo Module

void DlgContactInfoUnLoadModule()
{
	WindowList_Destroy(g_hWindowList);
}
