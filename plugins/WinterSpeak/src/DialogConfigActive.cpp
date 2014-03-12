#include "Common.h"
#include "DialogConfigActive.h"

//------------------------------------------------------------------------------
DialogConfigActive *DialogConfigActive::m_instance = 0;

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
DialogConfigActive::DialogConfigActive(ConfigDatabase &db) : m_db(db)
{
	m_instance = this;
}

//------------------------------------------------------------------------------
DialogConfigActive::~DialogConfigActive()
{
	m_instance = 0;
}

//------------------------------------------------------------------------------
INT_PTR CALLBACK DialogConfigActive::process(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (!m_instance)
	{
		return 1;
	}

	switch (message)
	{
	  case WM_INITDIALOG:
		  m_instance->load(window);
		break;

	  case WM_NOTIFY:
		switch(((LPNMHDR)lparam)->idFrom) {
			case IDC_ACTIVE_USERS: {
				m_instance->notify(window, lparam);
				
			} break;
			case 0: {
				switch (reinterpret_cast<LPNMHDR>(lparam)->code)
				{
				  case PSN_APPLY:
					m_instance->save(window);
					break;

				  case LVN_ITEMCHANGED:
					m_instance->changed(window);
					break;
				} break;
			} break;
		} break;
	  case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		  case IDC_ACTIVE_OFFLINE:
		  case IDC_ACTIVE_ONLINE:
		  case IDC_ACTIVE_AWAY:
		  case IDC_ACTIVE_DND:
		  case IDC_ACTIVE_NA:
		  case IDC_ACTIVE_OCCUPIED:
		  case IDC_ACTIVE_FREEFORCHAT:
		  case IDC_ACTIVE_INVISIBLE:
			m_instance->changed(window);
			break;

		/*case IDC_ACTIVE_ALL:
			m_instance->selectAllUsers(window, true);
			break;*/

		/*case IDC_ACTIVE_NONE:
			m_instance->selectAllUsers(window, false);
			break;*/

		  case IDC_ACTIVE_USERS:
			m_instance->changed(window);
			break;
		}
		break;
	}

	return 0;
}







//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void DialogConfigActive::notify(HWND hwndDlg, LPARAM lParam) {
	switch (((LPNMHDR)lParam)->code) {
		case CLN_NEWCONTACT:
		case CLN_LISTREBUILT:
			SetAllContactIcons( GetDlgItem(hwndDlg, IDC_ACTIVE_USERS), hwndDlg);
			//fall through
		case CLN_CONTACTMOVED:
			SetListGroupIcons( GetDlgItem(hwndDlg, IDC_ACTIVE_USERS), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_ACTIVE_USERS, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
			break;
		case CLN_OPTIONSCHANGED:
			
			ResetListOptions( GetDlgItem(hwndDlg, IDC_ACTIVE_USERS));
			break;
		case CLN_CHECKCHANGED:
			m_instance->changed(hwndDlg);
			break;
		case NM_CLICK:
			{
				NMCLISTCONTROL *nm = (NMCLISTCONTROL*)lParam;
				if (nm->iColumn == -1)
					break;
				
				DWORD hitFlags;
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_ACTIVE_USERS, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
				if (hItem == NULL || !(hitFlags & CLCHT_ONITEMEXTRA))
					break;

				if (nm->iColumn == 2) { // ignore all
					for (int iImage = 0;iImage<2;iImage++)
						SetIconsForColumn( GetDlgItem(hwndDlg, IDC_ACTIVE_USERS), hItem, hItemAll, iImage, iImage+3);
				}
				else if (nm->iColumn == 3) {	// ignore none
					for (int iImage = 0;iImage<2;iImage++)
						SetIconsForColumn( GetDlgItem(hwndDlg, IDC_ACTIVE_USERS), hItem, hItemAll, iImage, 0);
				}
				else {
					int iImage = SendDlgItemMessage(hwndDlg, IDC_ACTIVE_USERS, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
					if (iImage == 0)
						iImage = nm->iColumn+3;
					else if (iImage != EMPTY_EXTRA_ICON)
						iImage = 0;
					SetIconsForColumn( GetDlgItem(hwndDlg, IDC_ACTIVE_USERS), hItem, hItemAll, nm->iColumn, iImage);
				}
				SetListGroupIcons( GetDlgItem(hwndDlg, IDC_ACTIVE_USERS), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_ACTIVE_USERS, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
				m_instance->changed(hwndDlg);
			}
			break;
	}
}

void DialogConfigActive::load(HWND window)
{
	TranslateDialogDefault(window);

	// initialise the checkboxes
	CheckDlgButton(window, IDC_ACTIVE_ONLINE,         m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Online));
	CheckDlgButton(window, IDC_ACTIVE_AWAY,         m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Away));
	CheckDlgButton(window, IDC_ACTIVE_DND,         m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Dnd));
	CheckDlgButton(window, IDC_ACTIVE_NA,         m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Na));
	CheckDlgButton(window, IDC_ACTIVE_OCCUPIED,         m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Occupied));
	CheckDlgButton(window, IDC_ACTIVE_FREEFORCHAT,         m_db.getActiveFlag(ConfigDatabase::ActiveFlag_FreeForChat));
	CheckDlgButton(window, IDC_ACTIVE_INVISIBLE,         m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Invisible));

	HWND listview = GetDlgItem(window, IDC_ACTIVE_USERS);

	HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3, 3);
	ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_SMALLDOT);
	ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_FILLEDBLOB);
	ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_EMPTYBLOB);
	ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_EVENT_MESSAGE);
	ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_USERONLINE);

	SendDlgItemMessage(window, IDC_ACTIVE_USERS, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
	for (int i=0; i < SIZEOF(hIcons); i++)
		hIcons[i] = ImageList_GetIcon(hIml, 1+i, ILD_NORMAL);

	SendDlgItemMessage(window, IDC_ALLICON, STM_SETICON, (WPARAM)hIcons[0], 0);
	SendDlgItemMessage(window, IDC_NONEICON, STM_SETICON, (WPARAM)hIcons[1], 0);
	SendDlgItemMessage(window, IDC_MSGICON, STM_SETICON, (WPARAM)hIcons[2], 0);
	SendDlgItemMessage(window, IDC_ONLINEICON, STM_SETICON, (WPARAM)hIcons[3], 0);

	this->ResetListOptions(listview);

	SendDlgItemMessage(window, IDC_ACTIVE_USERS, CLM_SETEXTRACOLUMNS, 4, 0);
	{
		CLCINFOITEM cii = {0};
		cii.cbSize = sizeof(cii);
		cii.flags = CLCIIF_GROUPFONT;
		cii.pszText = TranslateT("** All contacts **");
		hItemAll = (HANDLE)SendDlgItemMessage(window, IDC_ACTIVE_USERS, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
	
		cii.pszText = TranslateT("** Unknown contacts **");
		hItemUnknown = (HANDLE)SendDlgItemMessage(window, IDC_ACTIVE_USERS, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		ConfigDatabase::ActiveUsersMap active_users = m_db.getActiveUsers();
		this->InitialiseItem(listview, hItemUnknown, active_users[0].message, active_users[0].status);
	}
	this->SetAllContactIcons(listview, window);
	this->SetListGroupIcons(GetDlgItem(window, IDC_ACTIVE_USERS), (HANDLE)SendDlgItemMessage(window, IDC_ACTIVE_USERS, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);

}

void DialogConfigActive::SetAllChildIcons(HWND hwndList, HANDLE hFirstItem, int iColumn, int iImage)
{
	HANDLE hItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	//check groups
	if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	while (hItem) {
		HANDLE hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem)
			SetAllChildIcons(hwndList, hChildItem, iColumn, iImage);
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	//check contacts
	if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	while (hItem) {
		int iOldIcon = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
}

void DialogConfigActive::SetIconsForColumn(HWND hwndList, HANDLE hItem, HANDLE hItemAll, int iColumn, int iImage) {
	switch ( SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hItem, 0)) {
	case CLCIT_CONTACT:
		{
			int oldiImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
			if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
				SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		}
		break;
	case CLCIT_INFO:
		if (hItem == hItemAll)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
		else
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage)); //hItemUnknown
		break;

	case CLCIT_GROUP:
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hItem)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
	}
}

void DialogConfigActive::InitialiseItem(HWND hwndList, HANDLE hItem, bool message, bool status) {
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0, (message)?0:3)); //Message
	
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(1, (status)?0:4)); //Online

	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(2, 1));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(3, 2));
}

void DialogConfigActive::SetAllContactIcons(HWND hwndList, HWND window) {
	ConfigDatabase::ActiveUsersMap active_users = m_db.getActiveUsers();
	ConfigDatabase::ActiveUsersMap::const_iterator iter;
	for (iter = active_users.begin(); iter != active_users.end(); ++iter)
	{
		MCONTACT hContact = iter->first;
		if(hContact == 0) {
			
		} else {
			HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
			if (hItem && SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(1, 0)) == EMPTY_EXTRA_ICON) {
				this->InitialiseItem(hwndList, hItem, iter->second.message, iter->second.status);
			}
		}
	}
}

void DialogConfigActive::ResetListOptions(HWND listview) {
	SendMessage(listview, CLM_SETBKBITMAP, 0, (LPARAM)(HBITMAP)NULL);
	SendMessage(listview, CLM_SETBKCOLOR, GetSysColor(COLOR_WINDOW), 0);
	SendMessage(listview, CLM_SETGREYOUTFLAGS, 0, 0);
	SendMessage(listview, CLM_SETLEFTMARGIN, 2, 0);
	SendMessage(listview, CLM_SETINDENT, 10, 0);
	SendMessage(listview, CLM_SETHIDEEMPTYGROUPS, 1, 0);

	for (int i=0; i <= FONTID_MAX; i++)
		SendMessage(listview, CLM_SETTEXTCOLOR, i, GetSysColor(COLOR_WINDOWTEXT));

	SetWindowLongPtr(listview, GWL_STYLE, GetWindowLongPtr(listview, GWL_STYLE) | CLS_SHOWHIDDEN);
}

void DialogConfigActive::SetListGroupIcons(HWND hwndList, HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount) {
	int iconOn[2] = {1, 1};
	int childCount[2] = {0, 0}, i;
	int iImage;
	HANDLE hItem, hChildItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	//check groups
	if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	while (hItem) {
		hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem) SetListGroupIcons(hwndList, hChildItem, hItem, childCount);
		for (i=0; i < SIZEOF(iconOn); i++)
			if (iconOn[i] && SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i) == 0) iconOn[i] = 0;
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	//check contacts
	if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	while (hItem) {
		for (i=0; i < SIZEOF(iconOn); i++) {
			iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i);
			if (iconOn[i] && iImage == 0) iconOn[i] = 0;
			if (iImage != EMPTY_EXTRA_ICON)
				childCount[i]++;
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
	//set icons
	for (i=0; i < SIZEOF(iconOn); i++) {
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(i, childCount[i]?(iconOn[i]?i+3:0) : EMPTY_EXTRA_ICON));
		if (groupChildCount) groupChildCount[i]+=childCount[i];
	}
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(2, 1));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(3, 2));
}

//------------------------------------------------------------------------------
void DialogConfigActive::save(HWND window)
{
	// store the checkboxes
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Online, 		(IsDlgButtonChecked(window, IDC_ACTIVE_ONLINE) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Away, 		(IsDlgButtonChecked(window, IDC_ACTIVE_AWAY) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Dnd, 		(IsDlgButtonChecked(window, IDC_ACTIVE_DND) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Na, 		(IsDlgButtonChecked(window, IDC_ACTIVE_NA) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Occupied, 		(IsDlgButtonChecked(window, IDC_ACTIVE_OCCUPIED) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_FreeForChat, 		(IsDlgButtonChecked(window, IDC_ACTIVE_FREEFORCHAT) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Invisible, 		(IsDlgButtonChecked(window, IDC_ACTIVE_INVISIBLE) != 0));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(window, IDC_ACTIVE_USERS, CLM_FINDCONTACT, hContact, 0);
		if (hItem) 
			SaveItemMask(GetDlgItem(window, IDC_ACTIVE_USERS), hContact, hItem);
	}

	SaveItemMask( GetDlgItem(window, IDC_ACTIVE_USERS), NULL, hItemUnknown);

	m_db.save();
}

void DialogConfigActive::SaveItemMask(HWND hwndList, MCONTACT hContact, HANDLE hItem) {
	ConfigDatabase::act mask;
	mask.message = true;
	mask.status = true;
	for (int i=0; i < 2; i++) {
		int iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, 0));
		if (iImage && iImage != EMPTY_EXTRA_ICON) {
			if(i == 1) { //Online
				mask.status = false;
			} 
			if(i == 0) { //message
				mask.message = false;
			}
		}
	}
	m_db.setActiveUser(hContact, mask);
}

//------------------------------------------------------------------------------
void DialogConfigActive::selectAllUsers(HWND window, bool state)
{
	HWND listview = GetDlgItem(window, IDC_ACTIVE_USERS);

	for (int i = 0; i < ListView_GetItemCount(listview); ++i)
	{
		ListView_SetCheckState(listview, i, state);
	}

	changed(window);
}

//==============================================================================
