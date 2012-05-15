/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
#include "commonheaders.h"

static void SetListGroupIcons(HWND hwndList,HANDLE hFirstItem,HANDLE hParentItem,int *groupChildCount)
{
	int typeOfFirst;
	int iconOn[2]={1,1};
	int childCount[2]={0,0},i;
	int iImage;
	HANDLE hItem,hChildItem;

	typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetListGroupIcons(hwndList,hChildItem,hItem,childCount);
		for( i=0; i < SIZEOF(iconOn); i++)
			if(iconOn[i] && SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,i)==0) iconOn[i]=0;
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		for ( i=0; i < SIZEOF(iconOn); i++) {
			iImage=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,i);
			if(iconOn[i] && iImage==0) iconOn[i]=0;
			if(iImage!=0xFF) childCount[i]++;
		}
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
	//set icons
	for( i=0; i < SIZEOF(iconOn); i++) {
		SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hParentItem,MAKELPARAM(i,childCount[i]?(iconOn[i]?i+1:0):0xFF));
		if(groupChildCount) groupChildCount[i]+=childCount[i];
	}
}

static void SetAllChildIcons(HWND hwndList,HANDLE hFirstItem,int iColumn,int iImage)
{
	int typeOfFirst,iOldIcon;
	HANDLE hItem,hChildItem;

	typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem) SetAllChildIcons(hwndList,hChildItem,iColumn,iImage);
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		iOldIcon=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
		if(iOldIcon!=0xFF && iOldIcon!=iImage) SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
}

static void ResetListOptions(HWND hwndList)
{
	int i;

	SendMessage(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList,CLM_SETBKCOLOR,GetSysColor(COLOR_WINDOW),0);
	SendMessage(hwndList,CLM_SETGREYOUTFLAGS,0,0);
	SendMessage(hwndList,CLM_SETLEFTMARGIN,2,0);
	SendMessage(hwndList,CLM_SETINDENT,10,0);
	for(i=0;i<=FONTID_MAX;i++)
		SendMessage(hwndList,CLM_SETTEXTCOLOR,i,GetSysColor(COLOR_WINDOWTEXT));
	SetWindowLongPtr(hwndList,GWL_STYLE,GetWindowLongPtr(hwndList,GWL_STYLE)|CLS_SHOWHIDDEN);
}

static void SetAllContactIcons(HWND hwndList)
{
	HANDLE hContact,hItem;
	char *szProto;
	DWORD flags;
	WORD status;

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do {
		hItem=(HANDLE)SendMessage(hwndList,CLM_FINDCONTACT,(WPARAM)hContact,0);
		if(hItem) {
			szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
			if(szProto==NULL) {flags=0; status=0;}
			else {
				flags=CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0);
				status=DBGetContactSettingWord(hContact,szProto,"ApparentMode",0);
			}
			if(flags&PF1_INVISLIST) {
				if(SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(0,0))==0xFF)
					SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(0,status==ID_STATUS_ONLINE?1:0));
			}
			if(flags&PF1_VISLIST) {
				if(SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(1,0))==0xFF)
					SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(1,status==ID_STATUS_OFFLINE?2:0));
			}
		}
	} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
}

static INT_PTR CALLBACK DlgProcVisibilityOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	static HICON hVisibleIcon,hInvisibleIcon;
	static HANDLE hItemAll;

	switch (msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			{	HIMAGELIST hIml;
				hIml=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),(IsWinVerXPPlus()?ILC_COLOR32:ILC_COLOR16)|ILC_MASK,3,3);
				ImageList_AddIcon_IconLibLoaded(hIml,SKINICON_OTHER_SMALLDOT);
				ImageList_AddIcon_IconLibLoaded(hIml,SKINICON_STATUS_INVISIBLE);
				ImageList_AddIcon_IconLibLoaded(hIml,SKINICON_STATUS_OFFLINE);
				SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRAIMAGELIST,0,(LPARAM)hIml);
				hVisibleIcon=ImageList_GetIcon(hIml,1,ILD_NORMAL);
				SendDlgItemMessage(hwndDlg,IDC_VISIBLEICON,STM_SETICON,(WPARAM)hVisibleIcon,0);
				hInvisibleIcon=ImageList_GetIcon(hIml,2,ILD_NORMAL);
				SendDlgItemMessage(hwndDlg,IDC_INVISIBLEICON,STM_SETICON,(WPARAM)hInvisibleIcon,0);
			}

			ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
			SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_SETEXTRACOLUMNS,2,0);

			{	CLCINFOITEM cii={0};
				cii.cbSize=sizeof(cii);
				cii.flags=CLCIIF_GROUPFONT;
				cii.pszText=TranslateT("** All contacts **");
				hItemAll=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_ADDINFOITEM,0,(LPARAM)&cii);
			}

			SetAllContactIcons(GetDlgItem(hwndDlg,IDC_LIST));
			SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);
			return TRUE;
		case WM_SETFOCUS:
			SetFocus(GetDlgItem(hwndDlg,IDC_LIST));
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
						{	HANDLE hItem;
							NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
							DWORD hitFlags;
							int iImage;
							int itemType;

							// Make sure we have an extra column
							if (nm->iColumn == -1)
								break;

							// Find clicked item
							hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x,nm->pt.y));
							// Nothing was clicked
							if (hItem == NULL) break; 
							// It was not a visbility icon
							if (!(hitFlags & CLCHT_ONITEMEXTRA)) break;

							// Get image in clicked column (0=none, 1=visible, 2=invisible)
							iImage = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
							if (iImage == 0)
								iImage=nm->iColumn + 1;
							else
								if (iImage == 1 || iImage == 2)
									iImage = 0;

							// Get item type (contact, group, etc...)
							itemType = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETITEMTYPE, (WPARAM)hItem, 0);

							// Update list, making sure that the options are mutually exclusive
							if (itemType == CLCIT_CONTACT) { // A contact
								SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, iImage));
								if (iImage && SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(nm->iColumn?0:1,0))!=0xFF)
									SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn?0:1, 0));
							}
							else if (itemType == CLCIT_INFO) {	 // All Contacts
								SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
								if (iImage)
									SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn?0:1, 0);
							}
							else if (itemType == CLCIT_GROUP) { // A group
								hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
								if (hItem) {
									SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
									if (iImage)
										SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn?0:1, 0);
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
						{	HANDLE hContact,hItem;
							int set,i,iImage;

							hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
							do {
								hItem=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_FINDCONTACT,(WPARAM)hContact,0);
								if(hItem) {
									set=0;
									for(i=0;i<2;i++) {
										iImage=SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i,0));
										if(iImage==i+1) {
											CallContactService(hContact,PSS_SETAPPARENTMODE,iImage==1?ID_STATUS_ONLINE:ID_STATUS_OFFLINE,0);
											set=1;
											break;
										}
									}
									if(!set) CallContactService(hContact,PSS_SETAPPARENTMODE,0,0);
								}
							} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
							return TRUE;
						}
					}
					break;
			}
			break;
		case WM_DESTROY:
			DestroyIcon(hVisibleIcon);
			DestroyIcon(hInvisibleIcon);
			{	HIMAGELIST hIml=(HIMAGELIST)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGELIST,0,0);
				ImageList_Destroy(hIml);
			}
			break;
	}
	return FALSE;
}

static int VisibilityOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = 850000000;
	odp.hInstance = hMirandaInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_VISIBILITY);
	odp.pszTitle = LPGEN("Visibility");
	odp.pszGroup = LPGEN("Status");
	odp.pfnDlgProc = DlgProcVisibilityOpts;
	odp.flags = ODPF_BOLDGROUPS;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	return 0;
}

int LoadVisibilityModule(void)
{
	HookEvent(ME_OPT_INITIALISE,VisibilityOptInitialise);
	return 0;
}
