#include "common.h"
#include "MirandaOptions.h"


extern CMirfoxMiranda mirfoxMiranda;
extern HINSTANCE hInst;


/*
 *	callback function for tab 1 options page
 */
INT_PTR CALLBACK DlgProcOpts_Tab1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
	{
		//executed once, during each tab initialization during each miranda options open
		mirfoxMiranda.getMirfoxData().setTab1OptionsState(MFENUM_OPTIONS_INIT);

		TranslateDialogDefault(hwndDlg);

		//IDC1_COMBO1
		SendDlgItemMessage(hwndDlg, IDC1_COMBO1, CB_ADDSTRING, 0, (LPARAM)TranslateT("just send message"));
		SendDlgItemMessage(hwndDlg, IDC1_COMBO1, CB_ADDSTRING, 0, (LPARAM)TranslateT("send message and open message window"));
		SendDlgItemMessage(hwndDlg, IDC1_COMBO1, CB_ADDSTRING, 0, (LPARAM)TranslateT("show message window with pasted message"));
		SendDlgItemMessage(hwndDlg, IDC1_COMBO1, CB_SETCURSEL, (int)mirfoxMiranda.getMirfoxData().leftClickSendMode - 1, 0);


		//IDC1_COMBO2
		SendDlgItemMessage(hwndDlg, IDC1_COMBO2, CB_ADDSTRING, 0, (LPARAM)TranslateT("just send message"));
		SendDlgItemMessage(hwndDlg, IDC1_COMBO2, CB_ADDSTRING, 0, (LPARAM)TranslateT("send message and open message window"));
		SendDlgItemMessage(hwndDlg, IDC1_COMBO2, CB_ADDSTRING, 0, (LPARAM)TranslateT("show message window with pasted message"));
		SendDlgItemMessage(hwndDlg, IDC1_COMBO2, CB_SETCURSEL, (int)mirfoxMiranda.getMirfoxData().rightClickSendMode - 1, 0);

		//IDC1_COMBO3
		SendDlgItemMessage(hwndDlg, IDC1_COMBO3, CB_ADDSTRING, 0, (LPARAM)TranslateT("just send message"));
		//disabled due to Firefox bug. opening other window while firefox menu is still open -> firefox crash
		//SendDlgItemMessage(hwndDlg, IDC1_COMBO3, CB_ADDSTRING, 0, (LPARAM)TranslateT("send message and open message window"));
		//SendDlgItemMessage(hwndDlg, IDC1_COMBO3, CB_ADDSTRING, 0, (LPARAM)TranslateT("show message window with pasted message"));
		SendDlgItemMessage(hwndDlg, IDC1_COMBO3, CB_SETCURSEL, (int)mirfoxMiranda.getMirfoxData().middleClickSendMode - 1, 0);


		//other options initialization
		SetDlgItemText(hwndDlg, IDC1_EDIT1, mirfoxMiranda.getMirfoxData().getClientsProfilesFilterStringPtr()->c_str());

		if (mirfoxMiranda.getMirfoxData().getClientsProfilesFilterCheckbox()){
			CheckDlgButton(hwndDlg, IDC1_CHECK1, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC1_EDIT1), TRUE);
		} else {
			CheckDlgButton(hwndDlg, IDC1_CHECK1, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC1_EDIT1), FALSE);
		}
	  //other options initialization - end

		mirfoxMiranda.getMirfoxData().setTab1OptionsState(MFENUM_OPTIONS_WORK);
		return FALSE;

	}
	case WM_COMMAND:
	{
		if (mirfoxMiranda.getMirfoxData().getTab1OptionsState() != MFENUM_OPTIONS_WORK){
			break; //options not inited yet
		}

		//if user changed some options controls, send info to miranda to activate ok button
		if (
			(
				((HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())	//edit control AND control from message has focus now
				||
				((HIWORD(wParam) == BN_CLICKED) && (HWND)lParam == GetFocus())	//button or checkbox clicked AND control from message has focus now
				||
				(HIWORD(wParam) == CBN_DROPDOWN)								//COMBOBOX clicked
			)
			&&
			(
				LOWORD(wParam) != IDC1_BUTTON_INVALIDATE						//invalidate button click doesn't activate [Apply] button.
			)
		   )
		{
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}


		if (LOWORD(wParam) == IDC1_CHECK1){
			if (IsDlgButtonChecked(hwndDlg, IDC1_CHECK1) == BST_CHECKED){
				EnableWindow(GetDlgItem(hwndDlg, IDC1_EDIT1), TRUE);
			} else {
				EnableWindow(GetDlgItem(hwndDlg, IDC1_EDIT1), FALSE);
			}
		}


		//if (LOWORD(wParam) == IDC1_BUTTON_INVALIDATE && HIWORD(wParam) == BN_CLICKED){
			//TODO  invalidate button clicked - refresh MSM's (now this button has visable=false at .rc file)
			//break;
		//}

		break;
	}
	case WM_NOTIFY:
	{

		//executed on each move to another options tab or after [OK]

		if (mirfoxMiranda.getMirfoxData().getTab1OptionsState() != MFENUM_OPTIONS_WORK){
			break; //options not inited yet
		}


		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY){
			//SaveOptions from tab1 mirfoxData to miranda database
			mirfoxMiranda.getMirfoxData().leftClickSendMode = (MFENUM_SEND_MESSAGE_MODE)( 1 + (WORD)SendDlgItemMessage(hwndDlg, IDC1_COMBO1, CB_GETCURSEL, 0, 0));
			db_set_b(0, PLUGIN_DB_ID, "leftClickSendMode", (int)mirfoxMiranda.getMirfoxData().leftClickSendMode);

			mirfoxMiranda.getMirfoxData().rightClickSendMode = (MFENUM_SEND_MESSAGE_MODE)( 1 + (WORD)SendDlgItemMessage(hwndDlg, IDC1_COMBO2, CB_GETCURSEL, 0, 0));
			db_set_b(0, PLUGIN_DB_ID, "rightClickSendMode", (int)mirfoxMiranda.getMirfoxData().rightClickSendMode);

			mirfoxMiranda.getMirfoxData().middleClickSendMode = (MFENUM_SEND_MESSAGE_MODE)( 1 + (WORD)SendDlgItemMessage(hwndDlg, IDC1_COMBO3, CB_GETCURSEL, 0, 0));
			db_set_b(0, PLUGIN_DB_ID, "middleClickSendMode", (int)mirfoxMiranda.getMirfoxData().middleClickSendMode);

			if (IsDlgButtonChecked(hwndDlg, IDC1_CHECK1) == BST_CHECKED){
				mirfoxMiranda.getMirfoxData().setClientsProfilesFilterCheckbox(true);
				db_set_b(0, PLUGIN_DB_ID, "clientsProfilesFilterCheckbox", 1);
			} else {
				mirfoxMiranda.getMirfoxData().setClientsProfilesFilterCheckbox(false);
				db_set_b(0, PLUGIN_DB_ID, "clientsProfilesFilterCheckbox", 2);
			}


			int opt2Len = SendDlgItemMessage(hwndDlg, IDC1_EDIT1, WM_GETTEXTLENGTH, 0, 0);
			wchar_t * opt2Buffer = new WCHAR[opt2Len+1];
			UINT opt2NumCharCopy = GetDlgItemText(hwndDlg, IDC1_EDIT1, opt2Buffer, opt2Len+1);
			mirfoxMiranda.getMirfoxData().getClientsProfilesFilterStringPtr()->assign(opt2Buffer);
			delete[] opt2Buffer;
			mirfoxMiranda.getMirfoxData().normalizeClientsProfilesFilterString(SMUCONST_CSM_RECORD_VISABLETO_SIZEC_DEF);
			SetDlgItemText(hwndDlg, IDC1_EDIT1, mirfoxMiranda.getMirfoxData().getClientsProfilesFilterStringPtr()->c_str());

			MFLogger* logger = MFLogger::getInstance();
			logger->log_p(L"Options. Save clientsProfilesFilterString: [%s]", mirfoxMiranda.getMirfoxData().getClientsProfilesFilterStringPtr()->c_str());
			db_set_ts(0, PLUGIN_DB_ID, "clientsProfilesFilterString", mirfoxMiranda.getMirfoxData().getClientsProfilesFilterStringPtr()->c_str());

			//TODO fire actualization visableTo field at CSM record

		}//end of PSN_APPLY

		break;

	}
	case WM_DESTROY:

		break;

	}

	return 0;
}

//--------------------------------------TAB 2------------------------------------------------------------------
static void setListContactIcons(HWND hwndList){


	//trick
	//http://forums.miranda-im.org/showthread.php?t=11800
	//give IDC2_CONTACTS_LIST control time needed to rebuild internal data list
	SendMessage(hwndList, WM_TIMER, TIMERID_REBUILDAFTER, 0); //TIMERID_REBUILDAFTER def at m_clistint.h

	//get contacts data from mirfoxData
	boost::ptr_list<MirandaContact> * mirandaContactsPtr = mirfoxMiranda.getMirfoxData().getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){

		MCONTACT hContact = mirandaContactsIter->contactHandle;
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if(hItem) {
			//if icon on 0th extracolumn is not set
			if(SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0,0)) == 0xFF){ 
				//check contact state on/off
				if (MFENUM_MIRANDACONTACT_STATE_ON == mirandaContactsIter->contactState){
					//if on - set icon 1 on 0th extracolumn
					SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0,1));  
				} else {
					//if off (default) - set icon 0 on 0th extracolumn
					SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0,0));  
				}
			}
		} else {
			//no contact at options list, probably deleted,
			//TODO refresh mirfoxData.contacts ??
		}

	}

	//TODO new contacts on list support - add to mirfoxData


}

static void setListGroupIcons(HWND hwndList, HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount){

	int iconOn[1]={1};
	int childCount[1]={0};
	int i;
	int iImage;
	HANDLE hItem;
	HANDLE hChildItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);

	//check groups
	if(typeOfFirst == CLCIT_GROUP){
		hItem = hFirstItem;
	} else {
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	}

	while(hItem) {
		hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if(hChildItem) {
			setListGroupIcons(hwndList, hChildItem, hItem, childCount); //recursion
		}
		for( i=0; i < SIZEOF(iconOn); i++){
			if(iconOn[i] && SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem,i) == 0){
				iconOn[i] = 0;
			}
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}

	//check contacts
	if(typeOfFirst == CLCIT_CONTACT){
		hItem = hFirstItem;
	} else {
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	}

	while(hItem) {
		for ( i=0; i < SIZEOF(iconOn); i++) {
			iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i);
			if(iconOn[i] && iImage==0){
				iconOn[i]=0;
			}
			if(iImage!=0xFF){
				childCount[i]++;
			}
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}

	//set icons
	for( i=0; i < SIZEOF(iconOn); i++) {
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(i,childCount[i]?(iconOn[i]?i+1:0):0xFF));
		if(groupChildCount){
			groupChildCount[i]+=childCount[i];
		}
	}

}

static void setAllChildIcons(HWND hwndList, HANDLE hFirstItem, int iColumn, int iImage)
{

	HANDLE hItem;
	HANDLE hChildItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);

	//check groups
	if(typeOfFirst==CLCIT_GROUP){
		hItem = hFirstItem;
	} else {
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	}

	while(hItem) {
		hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if(hChildItem){
			setAllChildIcons(hwndList, hChildItem, iColumn, iImage); //recursion
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}


	//check contacts
	if(typeOfFirst==CLCIT_CONTACT){
		hItem = hFirstItem;
	} else {
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	}

	while(hItem) {
		int iOldIcon = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if(iOldIcon!=0xFF && iOldIcon!=iImage){
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn,iImage));
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
}



/**
 * set colors, margins, indents, styles etc of list
 */
static void resetListOptions(HWND hwndList)
{
	SetWindowLongPtr(hwndList, GWL_STYLE, GetWindowLongPtr(hwndList,GWL_STYLE)|CLS_SHOWHIDDEN);
}



HICON icoHandle_ICON_OFF;
HICON icoHandle_ICON_FF;

/*
 *	funkcja callback obs³uguj¹ca stronê z opcjami tab 2
 */
INT_PTR CALLBACK DlgProcOpts_Tab2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	static HANDLE hItemAll;

	switch ( msg ) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);

		mirfoxMiranda.getMirfoxData().setTab2OptionsState(MFENUM_OPTIONS_INIT);

		//load icons
		HIMAGELIST hIml;
		int smCx = GetSystemMetrics(SM_CXSMICON);
		int smCy = GetSystemMetrics(SM_CYSMICON);
		hIml = ImageList_Create(smCx,smCy,((LOBYTE(LOWORD(GetVersion()))>=5 && LOWORD(GetVersion())!=5) ? ILC_COLOR32 : ILC_COLOR16) | ILC_MASK, 4, 4);

		//load icons (direct)
		icoHandle_ICON_OFF = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_OFF));
		icoHandle_ICON_FF = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_FF));

		//TODO load icons (icolib) - no need to use DestroyIcon
		//HICON icoHandle_ICON_OFF = (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)mirfoxData.icoHandle_ICON_OFF);
		//HICON icoHandle_ICON_FF = (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)mirfoxData.icoHandle_ICON_FF);

		//add icons to ImageList list
		ImageList_AddIcon(hIml, icoHandle_ICON_OFF);
		ImageList_AddIcon(hIml, icoHandle_ICON_FF);
		SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml); //m_clc.h


		//list params init
		resetListOptions(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST));
		SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_SETEXTRACOLUMNS, 1, 0);

		//add 'All contacts' list record
		{
			CLCINFOITEM cii = {0};
			cii.cbSize = sizeof(cii);
			cii.flags = CLCIIF_GROUPFONT;
			cii.pszText =TranslateT("** All contacts **");
			hItemAll = (HANDLE)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		}

		//set contacts and groups icons
		setListContactIcons(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST));
		setListGroupIcons(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);


		mirfoxMiranda.getMirfoxData().setTab2OptionsState(MFENUM_OPTIONS_WORK);
		return FALSE;

	}
	case WM_SETFOCUS:

		SetFocus(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST));
		break;

	case WM_NOTIFY:

		if (mirfoxMiranda.getMirfoxData().getTab2OptionsState() != MFENUM_OPTIONS_WORK){
			break; //options not inited yet
		}

		switch(((LPNMHDR)lParam)->idFrom) {
		case IDC2_CONTACTS_LIST:

			switch (((LPNMHDR)lParam)->code){

			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				setListContactIcons(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST));
				//fall through
			case CLN_CONTACTMOVED:
				setListGroupIcons(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
				break;
			case CLN_OPTIONSCHANGED:
				resetListOptions(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST));
				break;
			case NM_CLICK:
			{

				NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;

				// Make sure we have an extra column
				if (nm->iColumn == -1){
					break;
				}

				// Find clicked item
				DWORD hitFlags;
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));

				// Nothing was clicked
				if (hItem == NULL){
					break;
				}
				// It was not a visbility icon
				if (!(hitFlags & CLCHT_ONITEMEXTRA)){
					break;
				}

				// Get image in clicked column (0=off, 1=on)
				int iImage = SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
				if (iImage == 0){
					iImage=nm->iColumn + 1;
				} else {
					if (iImage == 1){
						iImage = 0;
					}
				}

				// Get item type (contact, group, etc...)
				int itemType = SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETITEMTYPE, (WPARAM)hItem, 0);

				// Update list
				if (itemType == CLCIT_CONTACT) { // A contact
					SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, iImage));
				} else if (itemType == CLCIT_INFO) {	 // All Contacts
					setAllChildIcons(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST), hItem, nm->iColumn, iImage);
				} else if (itemType == CLCIT_GROUP) { // A group
					hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
					if (hItem) {
						setAllChildIcons(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST), hItem, nm->iColumn, iImage);
					}
				}

				// Update the all/none icons
				setListGroupIcons(GetDlgItem(hwndDlg, IDC2_CONTACTS_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);

				// Activate Apply button
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

				break;
			}//end case NM_CLICK

			}//end switch

			break;

		case 0:
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{

					for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)){

						HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_FINDCONTACT, hContact, 0);
						if(hItem) {

							int iImage = SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0,0));
							MFENUM_MIRANDACONTACT_STATE contactState;

							if (iImage == 0xFF){ //TODO impossible??
							} else {
								if (iImage == 1){
									contactState = MFENUM_MIRANDACONTACT_STATE_ON;
								} else {
									contactState = MFENUM_MIRANDACONTACT_STATE_OFF;
								}
							}

							//save to mirfoxData
							int result = mirfoxMiranda.getMirfoxData().updateMirandaContactState(hContact, contactState);
							if (result != 0){
								//todo errors handling
							}

							//save to db	1 - on, 2 - off
							if (contactState == MFENUM_MIRANDACONTACT_STATE_OFF){
								db_set_b(hContact, PLUGIN_DB_ID, "state", 2);
							} else {
								db_set_b(hContact, PLUGIN_DB_ID, "state", 1);
							}


						}//TODO else { ...    (and at others if(hItem))
						//TODO contacts witch are not ay mirfoxData but on list
						//( check hash concat(all id) on mirfoxData and on list, if doesn't match - refresh mirfoxData
						//same for protocols
						//for now it schould be ok

					}

					//TODO contacts at MirfoxData but not on list now

					return TRUE;
				}
			}
			break;
		}

		break;

	case WM_DESTROY:
	{
		HIMAGELIST hIml=(HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC2_CONTACTS_LIST, CLM_GETEXTRAIMAGELIST, 0, 0); //m_clc.h
		ImageList_Destroy(hIml);

		//   use DestroyIcon only witchout icolib
		DestroyIcon(icoHandle_ICON_OFF);
		icoHandle_ICON_OFF = NULL;
		DestroyIcon(icoHandle_ICON_FF);
		icoHandle_ICON_FF = NULL;

		break;

	}
	}//end switch

	return 0;
}

/*
 *	callback function for tab 3 options page
 */
INT_PTR CALLBACK DlgProcOpts_Tab3(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
	{
		//executed once during init of each tab, on each enter to miranda options

		mirfoxMiranda.getMirfoxData().setTab3OptionsState(MFENUM_OPTIONS_INIT);

		TranslateDialogDefault(hwndDlg);

		//protocol list initialization
		HWND hAccountsList = GetDlgItem(hwndDlg, IDC3_PROTOCOLS_LIST);

		ListView_SetExtendedListViewStyleEx(hAccountsList, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

		LVCOLUMN lvCol = {0};
		lvCol.mask = LVCF_WIDTH|LVCF_TEXT;
		lvCol.pszText=TranslateT("Protocol");
		lvCol.cx = 260;	//column width
		ListView_InsertColumn(hAccountsList, 0, &lvCol);

		LVITEM lvItem = {0};
		lvItem.mask=LVIF_TEXT|LVIF_PARAM;
		lvItem.iItem=0;
		lvItem.iSubItem=0;

		//get accounts data from mirfoxData
		boost::ptr_list<MirandaAccount> * mirandaAccountsPtr = mirfoxMiranda.getMirfoxData().getMirandaAccounts();
		boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
		for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){

			lvItem.pszText = mirandaAccountsIter->tszAccountName;
			//http://www.experts-exchange.com/Programming/Languages/CPP/Q_20175412.html - must duplicate string
			lvItem.lParam = (LPARAM)_strdup(mirandaAccountsIter->szModuleName);
			ListView_InsertItem(hAccountsList,&lvItem);//winapi function

			MFENUM_MIRANDAACCOUNT_STATE accountState = mirandaAccountsIter->accountState;
			if (accountState == MFENUM_MIRANDAACCOUNT_STATE_ON){
				ListView_SetCheckState(hAccountsList, lvItem.iItem, 1 );
			} else {
				ListView_SetCheckState(hAccountsList, lvItem.iItem, 0 );
			}

			lvItem.iItem++;
		}
	    //protocol list initialization - end

		mirfoxMiranda.getMirfoxData().setTab3OptionsState(MFENUM_OPTIONS_WORK);
		return FALSE;

	}
	case WM_COMMAND:
	{
		if (mirfoxMiranda.getMirfoxData().getTab3OptionsState() != MFENUM_OPTIONS_WORK){
			break; //options not inited yet
		}

		break;
	}
	case WM_NOTIFY:
	{

		//executed on each change tab at options or after [OK]

		if (mirfoxMiranda.getMirfoxData().getTab3OptionsState() != MFENUM_OPTIONS_WORK){
			break; //options not inited yet
		}


		if (((NMHDR*)lParam)->idFrom == IDC3_PROTOCOLS_LIST) {
			if (((NMHDR*)lParam)->code== LVN_ITEMCHANGED) {
				NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
				if (/*IsWindowVisible(GetDlgItem(hwndDlg,IDC_PROTOCOLLIST)) && */ ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK)) {
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
		}

		if (((LPNMHDR)lParam)->idFrom == 0 &&
			((LPNMHDR)lParam)->code == PSN_APPLY
		){
			//SaveOptions from tab3 mirfoxData to miranda database
			HWND hList = GetDlgItem(hwndDlg, IDC3_PROTOCOLS_LIST);
			LVITEM lvItem = {0};
			lvItem.mask = LVIF_PARAM;
			for (int i=0; i<ListView_GetItemCount(hList); i++) {

				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				ListView_GetItem(hList, &lvItem);

				char * accountId ((char *)lvItem.lParam);
				BYTE accountStateB = (BYTE)ListView_GetCheckState(hList, lvItem.iItem);

				MFENUM_MIRANDAACCOUNT_STATE accountState;
				if (accountStateB == 0){
					accountState = MFENUM_MIRANDAACCOUNT_STATE_OFF;
				} else if (accountStateB == 1){
					accountState = MFENUM_MIRANDAACCOUNT_STATE_ON;
				} else {
					//todo errors handling
					accountState = MFENUM_MIRANDAACCOUNT_STATE_ON;
				}

				//save to mirfoxData
				int result = mirfoxMiranda.getMirfoxData().updateMirandaAccountState(accountId, accountState);
				if (result != 0){
					//todo errors handling
				}

				//save to db	1 - on, 2 - off
				std::string mirandaAccountDBKey("ACCOUNTSTATE_");
				mirandaAccountDBKey += accountId;
				if (accountState == MFENUM_MIRANDAACCOUNT_STATE_OFF){
					db_set_b(0, PLUGIN_DB_ID, mirandaAccountDBKey.c_str(), 2);
				} else {
					db_set_b(0, PLUGIN_DB_ID, mirandaAccountDBKey.c_str(), 1);
				}


			} //end for

		}

		break;

	}
	case WM_DESTROY:
		break;
	}

	return 0;

}


/**
 *
 * called from: options.cpp.InitOptions()
 */
int OptInit(WPARAM wParam, LPARAM) {

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -790000000;
	odp.hInstance = hInst;
	odp.ptszTitle = LPGENT(PLUGIN_OPTIONS_NAME);
	odp.ptszGroup = LPGENT("Services");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	//1 - options
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT1);
	odp.ptszTab = LPGENT("Options");
	odp.pfnDlgProc = DlgProcOpts_Tab1;
	Options_AddPage(wParam, &odp);

	//2 - contacts
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT2);
	odp.ptszTab = LPGENT("Contacts");
	odp.pfnDlgProc = DlgProcOpts_Tab2;
	Options_AddPage(wParam, &odp);

	//3 - accounts
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT3);
	odp.ptszTab = LPGENT("Accounts");
	odp.pfnDlgProc = DlgProcOpts_Tab3;
	Options_AddPage(wParam, &odp);
	return 0;
}



HANDLE hEventOptInit;

/**
 *
 * called from: MirfoxMiranda.cpp.Load(PLUGINLINK *link)
 */
void InitOptions() {
	hEventOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);
}

/**
 *
 * called from: MirfoxMiranda.cpp.Unload()
 */
void DeinitOptions() {
	UnhookEvent(hEventOptInit);
}
