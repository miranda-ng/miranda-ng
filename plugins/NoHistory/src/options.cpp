#include "common.h"
#include "options.h"
#include "resource.h"
#include "icons.h"

static void SetListGroupIcons(HWND hwndList,HANDLE hFirstItem,HANDLE hParentItem,int *groupChildCount)
{
	int iconOn = 1;
	int childCount = 0;
	int iImage;
	HANDLE hItem;

	int typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		HANDLE hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetListGroupIcons(hwndList,hChildItem,hItem,&childCount);
		if(iconOn && SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,0)==0) iconOn=0;
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		iImage=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,0);
		if(iconOn && iImage==0) iconOn=0;
		if(iImage!=0xFF) childCount++;
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
	//set icons
	SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hParentItem,MAKELPARAM(0,childCount?(iconOn?1:0):0xFF));
	if(groupChildCount) *groupChildCount+=childCount;
}

static void SetAllChildIcons(HWND hwndList,HANDLE hFirstItem,int iColumn,int iImage)
{
	HANDLE hItem;

	int typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		HANDLE hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetAllChildIcons(hwndList,hChildItem,iColumn,iImage);
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		int iOldIcon=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
		if(iOldIcon!=0xFF && iOldIcon!=iImage) SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
}


static void ResetListOptions(HWND hwndList)
{
	SendMessage(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList,CLM_SETBKCOLOR,GetSysColor(COLOR_WINDOW),0);
	SendMessage(hwndList,CLM_SETGREYOUTFLAGS,0,0);
	SendMessage(hwndList,CLM_SETLEFTMARGIN,2,0);
	SendMessage(hwndList,CLM_SETINDENT,10,0);
	for(int i=0;i<=FONTID_MAX;i++)
		SendMessage(hwndList,CLM_SETTEXTCOLOR,i,GetSysColor(COLOR_WINDOWTEXT));
	SetWindowLong(hwndList,GWL_STYLE,GetWindowLong(hwndList,GWL_STYLE)|CLS_SHOWHIDDEN);
}

static void SetAllContactIcons(HWND hwndList)
{
	HANDLE hContact = db_find_first();
	do {
		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		bool chat_room = (proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) != 0);

		if(!chat_room) {
			HANDLE hItem=(HANDLE)SendMessage(hwndList,CLM_FINDCONTACT,(WPARAM)hContact,0);
			if(hItem) {
				bool disabled = (DBGetContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, 0) == 1);
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(0,disabled?1:0));
			}
		}
	} while(hContact = db_find_next(hContact));
}



static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HANDLE hItemAll;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{	HIMAGELIST hIml;
			hIml=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),(IsWinVerXPPlus()?ILC_COLOR32:ILC_COLOR16)|ILC_MASK,2,2);
			ImageList_AddIcon(hIml,hIconKeep);
			ImageList_AddIcon(hIml,hIconRemove);
			SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRAIMAGELIST,0,(LPARAM)hIml);
			SendDlgItemMessage(hwndDlg,IDC_PIC_KEEP,STM_SETICON,(WPARAM)hIconKeep,0);
			SendDlgItemMessage(hwndDlg,IDC_PIC_REMOVE,STM_SETICON,(WPARAM)hIconRemove,0);
		}

		ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
		SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRACOLUMNS,1,0);

		{	CLCINFOITEM cii={0};
			cii.cbSize=sizeof(cii);
			cii.flags=CLCIIF_GROUPFONT;
			cii.pszText=TranslateT("** All contacts **");
			hItemAll=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_ADDINFOITEM,0,(LPARAM)&cii);
		}

		SetAllContactIcons(GetDlgItem(hwndDlg,IDC_LIST));
		SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);

		SendDlgItemMessage(hwndDlg, IDC_SPN_TIMEOUT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60 * 60, 1));

		return FALSE;		
	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hwndDlg,IDC_LIST));
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
			case IDC_LIST:
				switch (((LPNMHDR)lParam)->code)
				{
					case CLN_NEWCONTACT:
					case CLN_LISTREBUILT:
						SetAllContactIcons(GetDlgItem(hwndDlg,IDC_LIST));
						//fall through
					case CLN_CONTACTMOVED:
						SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);
						break;
					case CLN_OPTIONSCHANGED:
						ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
						break;
					case NM_CLICK:
					{
						NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
						DWORD hitFlags;

						// Make sure we have an extra column
						if (nm->iColumn == -1)
							break;

						// Find clicked item
						HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x,nm->pt.y));
						// Nothing was clicked
						if (hItem == NULL) break; 
						// It was not a visbility icon
						if (!(hitFlags & CLCHT_ONITEMEXTRA)) break;

						// Get image in clicked column (0=none, 1=visible, 2=invisible)
						int iImage = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
						if (iImage == 0)
							iImage=nm->iColumn + 1;
						else
							if (iImage == 1)
								iImage = 0;

						// Get item type (contact, group, etc...)
						int itemType = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETITEMTYPE, (WPARAM)hItem, 0);

						// Update list, making sure that the options are mutually exclusive
						if (itemType == CLCIT_CONTACT) { // A contact
							SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, iImage));
						}
						else if (itemType == CLCIT_INFO) {	 // All Contacts
							SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
						}
						else if (itemType == CLCIT_GROUP) { // A group
							hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
							if (hItem) {
								SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
							}
						}
						// Update the all/none icons
						SetListGroupIcons(GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);

						// Activate Apply button
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						break;
					}
				}
				break;
			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
					{
						HANDLE hContact = db_find_first();
						do {
							char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
							bool chat_room = (proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) != 0);
					
							if(!chat_room) {							
								HANDLE hItem=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_FINDCONTACT,(WPARAM)hContact,0);
								if(hItem) {
									int iImage=SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(0,0));
									DBWriteContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, iImage==1?1:0);
								}
							}
						} while(hContact = db_find_next(hContact));

						return TRUE;
					}
				}
				break;
		}
		break;
	case WM_DESTROY:
		{	HIMAGELIST hIml=(HIMAGELIST)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGELIST,0,0);
			ImageList_Destroy(hIml);
		}
		break;
	}

	return 0;
}

int OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.flags						= ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.position					= -790000000;
	odp.hInstance					= hInst;

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT);
	odp.ptszTitle					= LPGENT("No History");
	odp.ptszGroup					= LPGENT("History");
	odp.ptszTab						= LPGENT("Logging");
	odp.pfnDlgProc					= DlgProcOpts;
	Options_AddPage(wParam,&odp);

	return 0;
}

HANDLE hEventOptInit;
void InitOptions() {
	hEventOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);
}

void DeinitOptions() {
	UnhookEvent(hEventOptInit);
}
