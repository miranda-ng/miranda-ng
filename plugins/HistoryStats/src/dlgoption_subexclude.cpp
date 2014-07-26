#include "_globals.h"
#include "dlgoption.h"

#include "iconlib.h"
#include "main.h"

/*
 * DlgOption::SubExclude
 */

DlgOption::SubExclude::SubExclude() :
	m_hItemAll(NULL), m_bChanged(false)	
{
}

DlgOption::SubExclude::~SubExclude()
{
	// unlock exlucde contacts
	g_bExcludeLock = false;
}

BOOL DlgOption::SubExclude::handleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_WINDOWPOSCHANGED:
		{
			RECT rClient, rWnd;

			GetClientRect(getHWnd(), &rClient);

			// clist
			rWnd = utils::getWindowRect(getHWnd(), IDC_CONTACTS);
			rWnd.right = rClient.right;
			rWnd.bottom = rClient.bottom;
			utils::moveWindow(getHWnd(), IDC_CONTACTS, rWnd);
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR* p = reinterpret_cast<NMHDR*>(lParam);
			if (p->idFrom != IDC_CONTACTS)
				break;

			HWND hCList = GetDlgItem(getHWnd(), IDC_CONTACTS);

			switch (p->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				updateAllContacts(hCList);
				updateAllGroups(hCList, reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_ROOT, 0)), m_hItemAll);
				break;

			case CLN_CONTACTMOVED:
				updateAllGroups(hCList, reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_ROOT, 0)), m_hItemAll);
				break;

			case CLN_OPTIONSCHANGED:
				customizeList(hCList);
				break;

			case NM_CLICK:
				NMCLISTCONTROL* pNM = reinterpret_cast<NMCLISTCONTROL*>(p);
				if (pNM->iColumn == -1)
					break;

				DWORD dwHitFlags = 0;
				HANDLE hItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_HITTEST, reinterpret_cast<WPARAM>(&dwHitFlags), MAKELPARAM(pNM->pt.x, pNM->pt.y)));
				if (!hItem || !(dwHitFlags & CLCHT_ONITEMEXTRA))
					break;

				int iImage = SendMessage(hCList, CLM_GETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(pNM->iColumn, 0));
				if (iImage != 0xFF) {
					iImage = (iImage == 0) ? 1 : 0;

					int itemType = SendMessage(hCList, CLM_GETITEMTYPE, reinterpret_cast<WPARAM>(hItem), 0);

					if (itemType == CLCIT_CONTACT)
						setAll(hCList, hItem, iImage, false);
					else if (itemType == CLCIT_INFO)
						setAll(hCList, hItem, iImage, true);
					else if (itemType == CLCIT_GROUP) {
						if (hItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_CHILD, reinterpret_cast<LPARAM>(hItem))))
							setAll(hCList, hItem, iImage, true);
					}

					// update groups
					updateAllGroups(hCList, reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_ROOT, 0)), m_hItemAll);

					// mark as dirty
					m_bChanged = true;
					getParent()->settingsChanged();
				}
				break;
			}
		}
		break;

	case WM_DESTROY:
		IconLib::unregisterCallback(staticRecreateIcons, reinterpret_cast<LPARAM>(this));
		ImageList_Destroy(reinterpret_cast<HIMAGELIST>(SendDlgItemMessage(getHWnd(), IDC_CONTACTS, CLM_GETEXTRAIMAGELIST, 0, 0)));
		break;
	}

	return FALSE;
}

void DlgOption::SubExclude::onWMInitDialog()
{
	TranslateDialogDefault(getHWnd());

	// init clist
	HWND hCList = GetDlgItem(getHWnd(), IDC_CONTACTS);
	HIMAGELIST hIml = ImageList_Create(OS::smIconCX(), OS::smIconCY(), OS::imageListColor() | ILC_MASK, 2, 0);
	SendMessage(hCList, CLM_SETEXTRAIMAGELIST, 0, reinterpret_cast<LPARAM>(hIml));

	staticRecreateIcons(reinterpret_cast<LPARAM>(this));
	IconLib::registerCallback(staticRecreateIcons, reinterpret_cast<LPARAM>(this));

	customizeList(hCList);
	SendMessage(hCList, CLM_SETEXTRACOLUMNS, 1, 0);

	CLCINFOITEM cii = { 0 };

	cii.cbSize = sizeof(cii);
	cii.flags = CLCIIF_GROUPFONT;
	cii.pszText = TranslateT("** All contacts **");
	m_hItemAll = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_ADDINFOITEM, 0, reinterpret_cast<LPARAM>(&cii)));

	// lock exlucde contacts
	g_bExcludeLock = true;
}

void DlgOption::SubExclude::staticRecreateIcons(LPARAM lParam)
{
	SubExclude* pDlg = reinterpret_cast<SubExclude*>(lParam);
	HIMAGELIST hIml = reinterpret_cast<HIMAGELIST>(SendDlgItemMessage(pDlg->getHWnd(), IDC_CONTACTS, CLM_GETEXTRAIMAGELIST, 0, 0));

	static IconLib::IconIndex ExtraIcons[] = {
		IconLib::iiExcludeNo,
		IconLib::iiExcludeYes,
	};

	ImageList_RemoveAll(hIml);

	array_each_(i, ExtraIcons)
	{
		ImageList_AddIcon(hIml, IconLib::getIcon(ExtraIcons[i]));
	}
}

void DlgOption::SubExclude::loadSettings()
{
	HWND hCList = GetDlgItem(getHWnd(), IDC_CONTACTS);

	updateAllContacts(hCList);
	updateAllGroups(hCList, reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_ROOT, 0)), m_hItemAll);

	// reset dirty flag
	m_bChanged = false;
}

void DlgOption::SubExclude::saveSettings()
{
	if (m_bChanged) {
		// update db
		HWND hCList = GetDlgItem(getHWnd(), IDC_CONTACTS);
		MirandaSettings db;

		db.setModule(con::ModHistoryStats);

		MCONTACT hContact = db_find_first();
		while (hContact) {
			HANDLE hItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_FINDCONTACT, hContact, 0));
			if (hItem) {
				db.setContact(hContact);

				int iImage = SendMessage(hCList, CLM_GETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(0, 0));
				bool bExcludeOld = db.settingExists(con::SettExclude);

				if (bExcludeOld && iImage == 0)
					db.delSetting(con::SettExclude);
				else if (!bExcludeOld && iImage == 1)
					db.writeBool(con::SettExclude, true);
			}

			hContact = db_find_next(hContact);
		}

		// reset dirty flag
		m_bChanged = false;
	}
}

void DlgOption::SubExclude::customizeList(HWND hCList)
{
	SetWindowLongPtr(hCList, GWL_STYLE, GetWindowLongPtr(hCList, GWL_STYLE) | CLS_SHOWHIDDEN);
}

void DlgOption::SubExclude::updateAllGroups(HWND hCList, HANDLE hFirstItem, HANDLE hParentItem)
{
	bool bIconOn = true;
	bool bHasChilds = false;

	int typeOfFirst = SendMessage(hCList, CLM_GETITEMTYPE, reinterpret_cast<WPARAM>(hFirstItem), 0);

	// groups
	HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, reinterpret_cast<LPARAM>(hFirstItem)));

	// MEMO: no short-circuit, otherwise subgroups won't be updated
	while (hItem /* && (bIconOn || !bHasChilds) */) {
		HANDLE hChildItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_CHILD, reinterpret_cast<LPARAM>(hItem)));
		if (hChildItem)
			updateAllGroups(hCList, hChildItem, hItem);
	
		int iImage = SendMessage(hCList, CLM_GETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(0, 0));

		bIconOn = bIconOn && (iImage != 0);
		bHasChilds = bHasChilds || (iImage != 0xFF);

		hItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, reinterpret_cast<LPARAM>(hItem)));
	}

	// contacts
	hItem = (typeOfFirst == CLCIT_CONTACT) ? hFirstItem : reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, reinterpret_cast<LPARAM>(hFirstItem)));

	while (hItem && (bIconOn || !bHasChilds)) {
		int iImage = SendMessage(hCList, CLM_GETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(0, 0));

		bIconOn = bIconOn && (iImage != 0);
		bHasChilds = bHasChilds || (iImage != 0xFF);

		hItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, reinterpret_cast<LPARAM>(hItem)));
	}

	//set icon
	SendMessage(hCList, CLM_SETEXTRAIMAGE, reinterpret_cast<WPARAM>(hParentItem), MAKELPARAM(0, bHasChilds ? (bIconOn ? 1 : 0) : 0xFF));
}

void DlgOption::SubExclude::updateAllContacts(HWND hCList)
{
	MirandaSettings db;

	db.setModule(con::ModHistoryStats);

	MCONTACT hContact = db_find_first();
	while (hContact) {
		HANDLE hItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_FINDCONTACT, hContact, 0));
		if (hItem) {
			db.setContact(hContact);

			if (SendMessage(hCList, CLM_GETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(0, 0)) == 0xFF)
				SendMessage(hCList, CLM_SETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(0, db.settingExists(con::SettExclude) ? 1 : 0));
		}

		hContact = db_find_next(hContact);
	}
}

void DlgOption::SubExclude::setAll(HWND hCList, HANDLE hFirstItem, int iImage, bool bIterate)
{
	int typeOfFirst = SendMessage(hCList, CLM_GETITEMTYPE, reinterpret_cast<WPARAM>(hFirstItem), 0);

	if (bIterate) {
		// check groups
		HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, reinterpret_cast<LPARAM>(hFirstItem)));

		while (hItem) {
			HANDLE hChildItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_CHILD, reinterpret_cast<LPARAM>(hItem)));
			if (hChildItem)
				setAll(hCList, hChildItem, iImage, true);

			hItem = reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, reinterpret_cast<LPARAM>(hItem)));
		}
	}

	// check contacts
	HANDLE hItem = (typeOfFirst == CLCIT_CONTACT) ? hFirstItem : reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, reinterpret_cast<LPARAM>(hFirstItem)));
	while (hItem) {
		int iOldIcon = SendMessage(hCList, CLM_GETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(0, 0));
		if (iOldIcon != 0xFF && iOldIcon != iImage)
			SendMessage(hCList, CLM_SETEXTRAIMAGE, reinterpret_cast<WPARAM>(hItem), MAKELPARAM(0, iImage));

		hItem = bIterate ? reinterpret_cast<HANDLE>(SendMessage(hCList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, reinterpret_cast<LPARAM>(hItem))) : NULL;
	}
}
