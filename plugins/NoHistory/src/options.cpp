#include "common.h"

static void SetListGroupIcons(HWND hwndList,HANDLE hFirstItem,HANDLE hParentItem,int *groupChildCount)
{
	int iconOn = 1;
	int childCount = 0;
	int iImage;
	HANDLE hItem;

	int typeOfFirst = SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);

	//check groups
	if (typeOfFirst == CLCIT_GROUP)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		HANDLE hChildItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if (hChildItem)
			SetListGroupIcons(hwndList,hChildItem,hItem,&childCount);
		if (iconOn && SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,0) == 0)
			iconOn = 0;
		hItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}

	//check contacts
	if (typeOfFirst == CLCIT_CONTACT)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		iImage = SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,0);
		if (iconOn && iImage==0)
			iconOn = 0;
		if (iImage != EMPTY_EXTRA_ICON)
			childCount++;
		hItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
	//set icons
	SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hParentItem,MAKELPARAM(0, childCount ? (iconOn ? 1 : 0) : EMPTY_EXTRA_ICON));
	if (groupChildCount)
		*groupChildCount += childCount;
}

static void SetAllChildIcons(HWND hwndList,HANDLE hFirstItem,int iColumn,int iImage)
{
	HANDLE hItem;

	int typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if (typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		HANDLE hChildItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if (hChildItem)
			SetAllChildIcons(hwndList,hChildItem,iColumn,iImage);
		hItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if (typeOfFirst == CLCIT_CONTACT)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);

	while(hItem) {
		int iOldIcon = SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
		if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
			SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		hItem = (HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
}

static void ResetListOptions(HWND hwndList)
{
	SetWindowLongPtr(hwndList,GWL_STYLE,GetWindowLongPtr(hwndList,GWL_STYLE)|CLS_SHOWHIDDEN);
}

static void SetAllContactIcons(HWND hwndList)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		bool chat_room = (proto && db_get_b(hContact, proto, "ChatRoom", 0) != 0);

		if (!chat_room) {
			HANDLE hItem=(HANDLE)SendMessage(hwndList,CLM_FINDCONTACT,hContact,0);
			if (hItem) {
				bool disabled = (db_get_b(hContact, MODULE, DBSETTING_REMOVE, 0) == 1);
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(0,disabled?1:0));
			}
		}
	}
}
											 
static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hItemAll;
	static HWND hwndList;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		hwndList = GetDlgItem(hwndDlg, IDC_LIST);
		{
			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
			ImageList_AddIcon(hIml, hIconKeep);
			ImageList_AddIcon(hIml, hIconRemove);
			SendMessage(hwndList, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			SendDlgItemMessage(hwndDlg, IDC_PIC_KEEP, STM_SETICON,(WPARAM)hIconKeep, 0);
			SendDlgItemMessage(hwndDlg, IDC_PIC_REMOVE, STM_SETICON,(WPARAM)hIconRemove, 0);
		}
		ResetListOptions(hwndList);
		SendMessage(hwndList, CLM_SETEXTRACOLUMNS, 1, 0);
		{
			CLCINFOITEM cii = { sizeof(cii) };
			cii.flags = CLCIIF_GROUPFONT;
			cii.pszText = TranslateT("** All contacts **");
			hItemAll = (HANDLE)SendMessage(hwndList,CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		}

		SetAllContactIcons(hwndList);
		SetListGroupIcons(hwndList,(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);

		SendDlgItemMessage(hwndDlg, IDC_SPN_TIMEOUT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60 * 60, 1));
		return FALSE;

	case WM_SETFOCUS:
		SetFocus(hwndList);
		break;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case IDC_LIST:
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				SetAllContactIcons(hwndList);
				//fall through
			case CLN_CONTACTMOVED:
				SetListGroupIcons(hwndList,(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);
				break;

			case CLN_OPTIONSCHANGED:
				ResetListOptions(hwndList);
				break;

			case NM_CLICK:
				// Make sure we have an extra column
				NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
				if (nm->iColumn == -1)
					break;

				// Find clicked item
				DWORD hitFlags;
				HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x,nm->pt.y));
				if (hItem == NULL || !(hitFlags & CLCHT_ONITEMEXTRA))
					break;

				// Get image in clicked column (0=none, 1=visible, 2=invisible)
				int iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
				if (iImage == 0)
					iImage = nm->iColumn + 1;
				else if (iImage == 1)
					iImage = 0;

				// Update list, making sure that the options are mutually exclusive
				switch ( SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hItem, 0)) {
				case CLCIT_CONTACT:
					SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, iImage));
					break;
				case CLCIT_INFO: // All Contacts
					SetAllChildIcons(hwndList, hItem, nm->iColumn, iImage);
					break;
				case CLCIT_GROUP:
					hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
					if (hItem)
						SetAllChildIcons(hwndList, hItem, nm->iColumn, iImage);
				}
				// Update the all/none icons
				SetListGroupIcons(hwndList, (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);

				// Activate Apply button
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					char *proto = GetContactProto(hContact);
					bool chat_room = (proto && db_get_b(hContact, proto, "ChatRoom", 0) != 0);

					if (!chat_room) {							
						HANDLE hItem = (HANDLE)SendMessage(hwndList,CLM_FINDCONTACT,hContact,0);
						if (hItem) {
							int iImage = SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(0,0));
							db_set_b(hContact, MODULE, DBSETTING_REMOVE, iImage==1?1:0);
						}
					}
				}
				return TRUE;
			}
		}
		break;

	case WM_DESTROY:
		ImageList_Destroy((HIMAGELIST)SendMessage(hwndList,CLM_GETEXTRAIMAGELIST,0,0));
		break;
	}

	return 0;
}

int OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
   odp.flags       = ODPF_BOLDGROUPS|ODPF_TCHAR;
   odp.position    = -790000000;
   odp.hInstance   = hInst;
   odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
   odp.ptszTitle   = LPGENT("No History");
   odp.ptszGroup   = LPGENT("History");
   odp.ptszTab     = LPGENT("Logging");
   odp.pfnDlgProc  = DlgProcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OptInit);
}
