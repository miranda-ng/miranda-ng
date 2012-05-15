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
#include "findadd.h"

#define TIMERID_THROBBER   111

#define HM_SEARCHACK  (WM_USER+10)
#define M_SETGROUPVISIBILITIES  (WM_USER+11)

static HWND hwndFindAdd=NULL;
static HANDLE hHookModulesLoaded = 0;
static HANDLE hMainMenuItem = NULL;
static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam);

static int FindAddDlgResizer(HWND,LPARAM lParam,UTILRESIZECONTROL *urc)
{
	static int y,nextY,oldTop;
	struct FindAddDlgData *dat;

	dat=(struct FindAddDlgData*)lParam;
	switch(urc->wId) {
		case IDC_RESULTS:
			return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
		case IDOK:
			dat->minDlgHeight=nextY+urc->rcItem.bottom-urc->rcItem.top;
			return RD_ANCHORX_LEFT|RD_ANCHORY_BOTTOM;
		case IDC_ADD:
		case IDC_MOREOPTIONS:
			return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
		case IDC_STATUSBAR:
			return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
		case IDC_PROTOIDGROUP:	//the resize is always processed in template order
			nextY=y=urc->rcItem.top;
			if(dat->showProtoId) nextY=y+urc->rcItem.bottom-urc->rcItem.top+7;
			break;
		case IDC_EMAILGROUP:
			oldTop=urc->rcItem.top;
			y=nextY;
			if(dat->showEmail) nextY=y+urc->rcItem.bottom-urc->rcItem.top+7;
			OffsetRect(&urc->rcItem,0,y-oldTop);
			return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;
		case IDC_NAMEGROUP:
			oldTop=urc->rcItem.top;
			y=nextY;
			if(dat->showName) nextY=y+urc->rcItem.bottom-urc->rcItem.top+7;
			OffsetRect(&urc->rcItem,0,y-oldTop);
			return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;
		case IDC_ADVANCEDGROUP:
			oldTop=urc->rcItem.top;
			y=nextY;
			if(dat->showAdvanced) nextY=y+urc->rcItem.bottom-urc->rcItem.top+7;
			OffsetRect(&urc->rcItem,0,y-oldTop);
			return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;
		case IDC_TINYEXTENDEDGROUP:
			oldTop=urc->rcItem.top;
			y=nextY;
			if(dat->showTiny)
			{
				int height= urc->dlgNewSize.cy-y-(urc->dlgOriginalSize.cy-urc->rcItem.bottom);
				nextY=y+200;  //min height for custom dialog
				urc->rcItem.top=urc->rcItem.bottom-height;
			}
			return RD_ANCHORX_LEFT|RD_ANCHORY_BOTTOM;
		case IDC_BYEMAIL:
		case IDC_EMAIL:
		case IDC_BYNAME:
		case IDC_STNAMENICK:
		case IDC_STNAMEFIRST:
		case IDC_STNAMELAST:
		case IDC_NAMENICK:
		case IDC_NAMEFIRST:
		case IDC_NAMELAST:
		case IDC_BYADVANCED:
		case IDC_BYCUSTOM:
		case IDC_ADVANCED:
			OffsetRect(&urc->rcItem,0,y-oldTop);
			return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;
		case IDC_HEADERBAR:
			return RD_ANCHORX_LEFT|RD_ANCHORY_TOP|RD_ANCHORX_WIDTH;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

static void RenderThrobber(HDC hdc,RECT *rcItem,int *throbbing,int *pivot)
{
	HBRUSH hBr;
	HDC hMemDC;
	HBITMAP hBitmap;
	HPEN hPen;
	RECT rc;
	int x,width,height,height2;

	InflateRect(rcItem,-1,0);
	width=rcItem->right-rcItem->left;
	height=rcItem->bottom-rcItem->top;
	height2=height/2;

	if (*throbbing) {
		/* create memdc */
		hMemDC=CreateCompatibleDC(0);
		hBitmap=( HBITMAP )SelectObject(hMemDC, CreateCompatibleBitmap(hdc,width,height));
		/* flush it */
		rc.left=rc.top=0;
		rc.right=width;
		rc.bottom=height;
		hBr=GetSysColorBrush(COLOR_BTNFACE);
		FillRect(hMemDC,&rc,hBr);
		DeleteObject(hBr);
		/* set up the pen */
		hPen=(HPEN)SelectObject(hMemDC,CreatePen(PS_SOLID,4,GetSysColor(COLOR_BTNSHADOW)));
		/* draw everything before the pivot */
		x=*pivot;
		while (x>(-height)) {
			MoveToEx(hMemDC,x+height2,0,NULL);
			LineTo(hMemDC,x-height2,height);
			x-=12;
		}

		/* draw everything after the pivot */
		x = *pivot;
		while (x < width+height) {
			MoveToEx(hMemDC,x+height2,0,NULL);
			LineTo(hMemDC,x-height2,height);
			x+=12;
		}

		/* move the pivot */
		*pivot+=2;
		/* reset the pivot point if it gets past the rect */
		if (*pivot>width) *pivot=0;
		/* put back the old pen and delete the new one */
		DeleteObject(SelectObject(hMemDC,hPen));
		/* cap the top and bottom */
		hPen=(HPEN)SelectObject(hMemDC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNFACE)));
		MoveToEx(hMemDC,0,0,NULL);
		LineTo(hMemDC,width,0);
		MoveToEx(hMemDC,0,height-1,NULL);
		LineTo(hMemDC,width,height-1);
		/* select in the old pen and delete the new pen */
		DeleteObject(SelectObject(hMemDC,hPen));
		/* paint to screen */
		BitBlt(hdc,rcItem->left,rcItem->top,width,height,hMemDC,0,0,SRCCOPY);
		/* select back in the old bitmap and delete the created one, as well as freeing the mem dc. */
		hBitmap=( HBITMAP )SelectObject(hMemDC,hBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);
	}
	else {
		/* just flush the DC */
		hBr=GetSysColorBrush(COLOR_BTNFACE);
		FillRect(hdc,rcItem,hBr);
		DeleteObject(hBr);
}	}

static void StartThrobber(HWND hwndDlg,struct FindAddDlgData *dat)
{
	dat->throbbing=1;
	SetTimer(hwndDlg,TIMERID_THROBBER,25,NULL);
}

static void StopThrobber(HWND hwndDlg,struct FindAddDlgData *dat)
{
	KillTimer(hwndDlg,TIMERID_THROBBER);
	dat->throbbing=0;
	dat->pivot=0;
	InvalidateRect(GetDlgItem(hwndDlg,IDC_STATUSBAR),NULL,FALSE);
}

static void ShowAdvancedSearchDlg(HWND hwndDlg,struct FindAddDlgData *dat)
{
	char *szProto=(char*)SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETCURSEL,0,0),0);

	if(szProto==NULL) return;
	if(dat->hwndAdvSearch==NULL) {
		RECT rc;
		dat->hwndAdvSearch=(HWND)CallProtoService(szProto,PS_CREATEADVSEARCHUI,0,(LPARAM)hwndDlg);
		GetWindowRect(GetDlgItem(hwndDlg,IDC_RESULTS),&rc);
		SetWindowPos(dat->hwndAdvSearch,0,rc.left,rc.top,0,0,SWP_NOZORDER|SWP_NOSIZE);
	}
	if(animateWindow) {
		animateWindow(dat->hwndAdvSearch,150,AW_ACTIVATE|AW_SLIDE|AW_HOR_POSITIVE);
		RedrawWindow(dat->hwndAdvSearch,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
	} else ShowWindow(dat->hwndAdvSearch,SW_SHOW);
	CheckDlgButton(hwndDlg,IDC_ADVANCED,BST_CHECKED);
}

static void ReposTinySearchDlg(HWND hwndDlg,struct FindAddDlgData *dat)
{
	if ( dat->hwndTinySearch != NULL ) {
		RECT rc;
		RECT clientRect;
		POINT pt={0,0};
		GetWindowRect(GetDlgItem(hwndDlg,IDC_TINYEXTENDEDGROUP),&rc);
		GetWindowRect(dat->hwndTinySearch,&clientRect);
		pt.x=rc.left;
		pt.y=rc.top;
		ScreenToClient(hwndDlg,&pt);
		SetWindowPos(dat->hwndTinySearch,0,pt.x+5,pt.y+15,rc.right-rc.left-10,rc.bottom-rc.top-30,SWP_NOZORDER);
		//SetWindowPos(GetDlgItem(hwndDlg,IDC_TINYEXTENDEDGROUP),0,0,0,rc.right-rc.left,clientRect.bottom-clientRect.top+20,SWP_NOMOVE|SWP_NOZORDER);
}	}

static void ShowTinySearchDlg(HWND hwndDlg,struct FindAddDlgData *dat)
{
	char *szProto=(char*)SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETCURSEL,0,0),0);
	if(szProto==NULL) return;
	if(dat->hwndTinySearch==NULL) {
		dat->hwndTinySearch=(HWND)CallProtoService(szProto,PS_CREATEADVSEARCHUI,0,(LPARAM)/*GetDlgItem(*/hwndDlg/*,IDC_TINYEXTENDEDGROUP)*/);
		if (dat->hwndTinySearch)
			ReposTinySearchDlg(hwndDlg, dat);
		else
			dat->showTiny = false;
	}
	ShowWindow(dat->hwndTinySearch,SW_SHOW);
}

static void HideAdvancedSearchDlg(HWND hwndDlg,struct FindAddDlgData *dat)
{
	if(dat->hwndAdvSearch==NULL) return;
	if(animateWindow && IsWinVerXPPlus())  //blending is quite slow on win2k
		animateWindow(dat->hwndAdvSearch,150,AW_HIDE|AW_BLEND);
	else ShowWindow(dat->hwndAdvSearch,SW_HIDE);
	CheckDlgButton(hwndDlg,IDC_ADVANCED,BST_UNCHECKED);
}

void EnableResultButtons(HWND hwndDlg,int enable)
{
	EnableWindow(GetDlgItem(hwndDlg,IDC_ADD), enable || IsDlgButtonChecked(hwndDlg, IDC_BYPROTOID));
	EnableWindow(GetDlgItem(hwndDlg,IDC_MOREOPTIONS),enable);
}

static void CheckSearchTypeRadioButton(HWND hwndDlg,int idControl)
{
	int i;
	static const int controls[]={IDC_BYPROTOID,IDC_BYEMAIL,IDC_BYNAME,IDC_BYADVANCED,IDC_BYCUSTOM};
	for( i=0; i < SIZEOF(controls); i++ )
		CheckDlgButton(hwndDlg,controls[i],idControl==controls[i]?BST_CHECKED:BST_UNCHECKED);
}

#define sttErrMsg TranslateT("You haven't filled in the search field. Please enter a search term and try again.")
#define sttErrTitle TranslateT("Search")

static void SetListItemText( HWND hwndList, int idx, int col, TCHAR* szText )
{
	if ( szText && szText[0] )
	{
		ListView_SetItemText( hwndList, idx, col, szText );
	}
	else
	{
		ListView_SetItemText( hwndList, idx, col, TranslateT("<not specified>"));
	}
}

static INT_PTR CALLBACK DlgProcFindAdd(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct FindAddDlgData* dat = ( struct FindAddDlgData* )GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hwndDlg, IDC_RESULTS);

	switch (msg) {
		case WM_INITDIALOG:
		{	
			int i,netProtoCount;
			COMBOBOXEXITEM cbei;
			HICON hIcon;

			TranslateDialogDefault(hwndDlg);
			Window_SetIcon_IcoLib(hwndDlg, SKINICON_OTHER_FINDUSER);
			dat=(struct FindAddDlgData*)mir_calloc(sizeof(struct FindAddDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->notSearchedYet=1;
			dat->iLastColumnSortIndex=1;
			dat->bSortAscending=1;
			dat->hBmpSortUp=(HBITMAP)LoadImage(hMirandaInst,MAKEINTRESOURCE(IDB_SORTCOLUP),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
			dat->hBmpSortDown=(HBITMAP)LoadImage(hMirandaInst,MAKEINTRESOURCE(IDB_SORTCOLDOWN),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
			SendDlgItemMessage(hwndDlg,IDC_MOREOPTIONS,BUTTONSETARROW,1,0);
			ListView_SetExtendedListViewStyle(hwndList,LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

			{	LVCOLUMN lvc;
				RECT rc;
				LVITEM lvi;

				GetClientRect(hwndList,&rc);
				lvc.mask = LVCF_TEXT | LVCF_WIDTH;
				lvc.pszText = TranslateT("Results");
				lvc.cx = rc.right-1;
				ListView_InsertColumn(hwndList, 0, &lvc);
				lvi.mask=LVIF_TEXT;
				lvi.iItem=0;
				lvi.iSubItem=0;
				lvi.pszText=TranslateT("There are no results to display.");
				ListView_InsertItem(hwndList, &lvi);
			}

			// Allocate a reasonable amount of space in the status bar
			{	int partWidth[3];
				SIZE textSize;
				HDC hdc;

				hdc=GetDC(GetDlgItem(hwndDlg,IDC_STATUSBAR));
				SelectObject(hdc,(HFONT)SendDlgItemMessage(hwndDlg,IDC_STATUSBAR,WM_GETFONT,0,0));
				GetTextExtentPoint32(hdc,TranslateT("Searching"),lstrlen(TranslateT("Searching")),&textSize);
				partWidth[0]=textSize.cx;
				GetTextExtentPoint32(hdc,_T("01234567890123456789"), 20, &textSize );
				partWidth[0]+=textSize.cx;
				ReleaseDC(GetDlgItem(hwndDlg,IDC_STATUSBAR),hdc);
				partWidth[1]=partWidth[0]+150;
				partWidth[2]=-1;
				SendDlgItemMessage(hwndDlg,IDC_STATUSBAR,SB_SETPARTS,SIZEOF(partWidth),(LPARAM)partWidth);
				SendDlgItemMessage(hwndDlg,IDC_STATUSBAR,SB_SETTEXT,1|SBT_OWNERDRAW,0);
				SetStatusBarSearchInfo(GetDlgItem(hwndDlg,IDC_STATUSBAR),dat);
			}
			{
				TCHAR *szProto = NULL;
				int index = 0;
				DBVARIANT dbv={0};
				HDC hdc;
				SIZE textSize;
				RECT rect;
				int cbwidth = 0;

				if ( !DBGetContactSettingTString( NULL, "FindAdd", "LastSearched", &dbv ))
					szProto = dbv.ptszVal;
				
				for( i=0, netProtoCount=0; i < accounts.getCount(); i++ ) {
					if (!Proto_IsAccountEnabled( accounts[i] )) continue;
					DWORD caps = (DWORD)CallProtoService( accounts[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0 );
					if (caps & PF1_BASICSEARCH || caps & PF1_EXTSEARCH || caps & PF1_SEARCHBYEMAIL || caps & PF1_SEARCHBYNAME)
						netProtoCount++;
				}
				dat->himlComboIcons=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),(IsWinVerXPPlus()?ILC_COLOR32:ILC_COLOR16)|ILC_MASK,netProtoCount+1,netProtoCount+1);
				SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CBEM_SETIMAGELIST,0,(LPARAM)dat->himlComboIcons);
				cbei.mask=CBEIF_IMAGE|CBEIF_SELECTEDIMAGE|CBEIF_TEXT|CBEIF_LPARAM;
				cbei.iItem=0;
				hdc=GetDC(hwndDlg);
				SelectObject(hdc,(HFONT)SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,WM_GETFONT,0,0));
				if(netProtoCount>1) {
					cbei.pszText=TranslateT("All Networks");
					GetTextExtentPoint32(hdc,cbei.pszText,lstrlen(cbei.pszText),&textSize);
					if (textSize.cx>cbwidth) cbwidth = textSize.cx;
					cbei.iImage=cbei.iSelectedImage=ImageList_AddIcon_IconLibLoaded(dat->himlComboIcons, SKINICON_OTHER_SEARCHALL);
					cbei.lParam=0;
					SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CBEM_INSERTITEM,0,(LPARAM)&cbei);
					cbei.iItem++;
				}
				for( i=0; i < accounts.getCount(); i++ ) {
					PROTOACCOUNT* pa = accounts[i];
					if (!Proto_IsAccountEnabled(pa)) continue;
					DWORD caps=(DWORD)CallProtoService( pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0 );
					if ( !(caps&PF1_BASICSEARCH) && !(caps&PF1_EXTSEARCH) && !(caps&PF1_SEARCHBYEMAIL) && !(caps&PF1_SEARCHBYNAME))
						continue;
					
					cbei.pszText = pa->tszAccountName;
					GetTextExtentPoint32(hdc,cbei.pszText,lstrlen(cbei.pszText),&textSize);
					if (textSize.cx>cbwidth) cbwidth = textSize.cx;
					hIcon=(HICON)CallProtoService( pa->szModuleName,PS_LOADICON,PLI_PROTOCOL|PLIF_SMALL,0);
					cbei.iImage=cbei.iSelectedImage=ImageList_AddIcon(dat->himlComboIcons,hIcon);
					DestroyIcon(hIcon);
					cbei.lParam=(LPARAM)pa->szModuleName;
					SendDlgItemMessageA(hwndDlg,IDC_PROTOLIST,CBEM_INSERTITEM,0,(LPARAM)&cbei);
					if (szProto && cbei.pszText && !lstrcmp( szProto, pa->tszAccountName ))
						index = cbei.iItem;
					cbei.iItem++;
				}
				cbwidth+=32;
				SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETDROPPEDCONTROLRECT,0,(LPARAM)&rect);
				if ((rect.right-rect.left)<cbwidth)
					SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_SETDROPPEDWIDTH,cbwidth,0);
				SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_SETCURSEL,index,0);
				DBFreeVariant(&dbv); /* free string szProto was fetched with */
			}
			SendMessage(hwndDlg,M_SETGROUPVISIBILITIES,0,0);
			Utils_RestoreWindowPosition(hwndDlg,NULL,"FindAdd","");

			return TRUE;
		}
		case WM_SIZE:
		{	UTILRESIZEDIALOG urd={0};
			urd.cbSize=sizeof(urd);
			urd.hwndDlg=hwndDlg;
			urd.hInstance=hMirandaInst;
			urd.lpTemplate=MAKEINTRESOURCEA(IDD_FINDADD);
			urd.lParam=(LPARAM)dat;
			urd.pfnResizer=FindAddDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG,0,(LPARAM)&urd);
			ReposTinySearchDlg(hwndDlg, dat);
			SendDlgItemMessage(hwndDlg,IDC_STATUSBAR,WM_SIZE,0,0);
			if(dat->notSearchedYet) {
				RECT rc;
				GetClientRect(hwndList,&rc);
				ListView_SetColumnWidth(hwndList,0,rc.right);
			}
		}
			//fall through
		case WM_MOVE:
			if (dat && dat->hwndAdvSearch)
			{
				RECT rc;
				GetWindowRect(hwndList,&rc);
				SetWindowPos(dat->hwndAdvSearch,0,rc.left,rc.top,0,0,SWP_NOZORDER|SWP_NOSIZE);
			}
			break;
		case WM_GETMINMAXINFO:
		{	MINMAXINFO *mmi=(MINMAXINFO*)lParam;
			RECT rc,rc2;
			GetWindowRect(hwndList,&rc);
			GetWindowRect(hwndDlg,&rc2);
			mmi->ptMinTrackSize.x=rc.left-rc2.left+10+GetSystemMetrics(SM_CXFRAME);
			GetClientRect(GetDlgItem(hwndDlg,IDC_MOREOPTIONS),&rc);
			mmi->ptMinTrackSize.x+=rc.right+5;
			GetClientRect(GetDlgItem(hwndDlg,IDC_ADD),&rc);
			mmi->ptMinTrackSize.x+=rc.right+5;
			GetClientRect(GetDlgItem(hwndDlg,IDC_STATUSBAR),&rc);
			mmi->ptMinTrackSize.y=dat->minDlgHeight+20+GetSystemMetrics(SM_CYCAPTION)+2*GetSystemMetrics(SM_CYFRAME);
			GetClientRect(GetDlgItem(hwndDlg,IDC_STATUSBAR),&rc);
			mmi->ptMinTrackSize.y+=rc.bottom;
			return 0;
		}
		case M_SETGROUPVISIBILITIES:
		{	char *szProto;
			int i;
			DWORD protoCaps;
			MINMAXINFO mmi;
			RECT rc;
			int checkmarkVisible;

			dat->showAdvanced = dat->showEmail = dat->showName = dat->showProtoId = dat->showTiny = 0;
			szProto=(char*)SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETCURSEL,0,0),0);
			if ( szProto == (char *)CB_ERR )
				break;
			if ( szProto == NULL ) {
				for ( i=0; i < accounts.getCount(); i++ ) {
					PROTOACCOUNT* pa = accounts[i];
					if (!Proto_IsAccountEnabled(pa)) continue;
					protoCaps=(DWORD)CallProtoService(pa->szModuleName,PS_GETCAPS,PFLAGNUM_1,0);
					if(protoCaps&PF1_SEARCHBYEMAIL) dat->showEmail=1;
					if(protoCaps&PF1_SEARCHBYNAME) dat->showName=1;
				}
			}
			else {
				protoCaps=(DWORD)CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0);
				if(protoCaps&PF1_BASICSEARCH) dat->showProtoId=1;
				if(protoCaps&PF1_SEARCHBYEMAIL) dat->showEmail=1;
				if(protoCaps&PF1_SEARCHBYNAME) dat->showName=1;
				if(protoCaps&PF1_EXTSEARCH && !(protoCaps&PF1_EXTSEARCHUI))	dat->showTiny=1;
				if(protoCaps&PF1_EXTSEARCHUI) dat->showAdvanced=1;
				if(protoCaps&PF1_USERIDISEMAIL && dat->showProtoId) {dat->showProtoId=0; dat->showEmail=1;}
				if(dat->showProtoId) {
					char *szUniqueId;
					szUniqueId=(char*)CallProtoService(szProto,PS_GETCAPS,PFLAG_UNIQUEIDTEXT,0);
					if(szUniqueId) {
						#if defined( _UNICODE )
							TCHAR* p = mir_a2u(szUniqueId);
							SetDlgItemText(hwndDlg,IDC_BYPROTOID,p);
							mir_free(p);
						#else
							SetDlgItemTextA(hwndDlg,IDC_BYPROTOID,szUniqueId);
						#endif
					}
					else SetDlgItemText(hwndDlg,IDC_BYPROTOID,TranslateT("Handle"));
					if(protoCaps&PF1_NUMERICUSERID) SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_PROTOID),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_PROTOID),GWL_STYLE)|ES_NUMBER);
					else SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_PROTOID),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_PROTOID),GWL_STYLE)&~ES_NUMBER);
				}
			}

			if (dat->showTiny)
				ShowTinySearchDlg(hwndDlg, dat);
			else {
				if (dat->hwndTinySearch) {
					DestroyWindow(dat->hwndTinySearch);
					dat->hwndTinySearch=NULL;
				}	}

#define en(id,t) ShowWindow(GetDlgItem(hwndDlg,IDC_##id),dat->show##t?SW_SHOW:SW_HIDE)
			en(PROTOIDGROUP,ProtoId); en(BYPROTOID,ProtoId); en(PROTOID,ProtoId);
			en(EMAILGROUP,Email); en(BYEMAIL,Email); en(EMAIL,Email);
			en(NAMEGROUP,Name);  en(BYNAME,Name);
			en(STNAMENICK,Name); en(NAMENICK,Name);
			en(STNAMEFIRST,Name); en(NAMEFIRST,Name);
			en(STNAMELAST,Name); en(NAMELAST,Name);
			en(ADVANCEDGROUP,Advanced); en(BYADVANCED,Advanced); en(ADVANCED,Advanced);
			en(BYCUSTOM, Tiny); en(TINYEXTENDEDGROUP, Tiny);
#undef en
			
			checkmarkVisible=(dat->showAdvanced && IsDlgButtonChecked(hwndDlg,IDC_BYADVANCED)) ||
			                 (dat->showEmail && IsDlgButtonChecked(hwndDlg,IDC_BYEMAIL)) ||
							 (dat->showTiny && IsDlgButtonChecked(hwndDlg,IDC_BYCUSTOM)) ||
							 (dat->showName && IsDlgButtonChecked(hwndDlg,IDC_BYNAME)) ||
							 (dat->showProtoId && IsDlgButtonChecked(hwndDlg,IDC_BYPROTOID));
			if(!checkmarkVisible) {
				if(dat->showProtoId) CheckSearchTypeRadioButton(hwndDlg,IDC_BYPROTOID);
				else if(dat->showEmail) CheckSearchTypeRadioButton(hwndDlg,IDC_BYEMAIL);
				else if(dat->showName) CheckSearchTypeRadioButton(hwndDlg,IDC_BYNAME);
				else if(dat->showAdvanced) CheckSearchTypeRadioButton(hwndDlg,IDC_BYADVANCED);
				else if(dat->showTiny) CheckSearchTypeRadioButton(hwndDlg,IDC_BYCUSTOM);
			}

			SendMessage(hwndDlg,WM_SIZE,0,0);
			SendMessage(hwndDlg,WM_GETMINMAXINFO,0,(LPARAM)&mmi);
			GetWindowRect(hwndDlg,&rc);
			if(rc.bottom-rc.top<mmi.ptMinTrackSize.y) SetWindowPos(hwndDlg,0,0,0,rc.right-rc.left,mmi.ptMinTrackSize.y,SWP_NOZORDER|SWP_NOMOVE);
			break;
		}
		case WM_TIMER:
			if(wParam==TIMERID_THROBBER) {
				RECT rc;
				HDC hdc;
				int borders[3];
				SendDlgItemMessage(hwndDlg,IDC_STATUSBAR,SB_GETBORDERS,0,(LPARAM)borders);
				SendDlgItemMessage(hwndDlg,IDC_STATUSBAR,SB_GETRECT,1,(LPARAM)&rc);
				InflateRect(&rc,-borders[2]/2,-borders[1]/2);
				hdc=GetDC(GetDlgItem(hwndDlg,IDC_STATUSBAR));
				RenderThrobber(hdc,&rc,&dat->throbbing,&dat->pivot);
				ReleaseDC(GetDlgItem(hwndDlg,IDC_STATUSBAR),hdc);
			}
			break;
		case WM_DRAWITEM:
		{	DRAWITEMSTRUCT *dis=(DRAWITEMSTRUCT*)lParam;
			if(dis->CtlID==IDC_STATUSBAR && dis->itemID==1) {
				RenderThrobber(dis->hDC,&dis->rcItem,&dat->throbbing,&dat->pivot);
				return TRUE;
			}
			break;
		}
		case WM_NOTIFY:
			if (wParam == IDC_RESULTS) {
				switch(((LPNMHDR)lParam)->code) {
				case LVN_ITEMCHANGED:
					{	int count=ListView_GetSelectedCount(hwndList);
						if(dat->notSearchedYet) count=0;
						EnableResultButtons(hwndDlg,count);
						break;
					}
				case LVN_COLUMNCLICK:
					{
						LPNMLISTVIEW nmlv=(LPNMLISTVIEW)lParam;
						HDITEM hdi;

						hdi.mask=HDI_BITMAP|HDI_FORMAT;
						hdi.fmt=HDF_LEFT|HDF_STRING;
						Header_SetItem(ListView_GetHeader(hwndList),dat->iLastColumnSortIndex,&hdi);

						if(nmlv->iSubItem!=dat->iLastColumnSortIndex)
						{
							dat->bSortAscending=TRUE;
							dat->iLastColumnSortIndex=nmlv->iSubItem;
						}
						else dat->bSortAscending=!dat->bSortAscending;

						hdi.fmt=HDF_LEFT|HDF_BITMAP|HDF_STRING|HDF_BITMAP_ON_RIGHT;
						hdi.hbm=dat->bSortAscending?dat->hBmpSortDown:dat->hBmpSortUp;
						Header_SetItem(ListView_GetHeader(hwndList),dat->iLastColumnSortIndex,&hdi);

						ListView_SortItemsEx(hwndList, SearchResultsCompareFunc, (LPARAM)hwndDlg);
					}
					break;
			}	}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_PROTOLIST:
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						HideAdvancedSearchDlg(hwndDlg,dat);
						if(dat->hwndAdvSearch) {
							DestroyWindow(dat->hwndAdvSearch);
							dat->hwndAdvSearch=NULL;
						}
						if(dat->hwndTinySearch) {
							DestroyWindow(dat->hwndTinySearch);
							dat->hwndTinySearch=NULL;
						}
						SendMessage(hwndDlg,M_SETGROUPVISIBILITIES,0,0);
					}
					break;
				case IDC_BYPROTOID:
					EnableWindow(GetDlgItem(hwndDlg,IDC_ADD),TRUE);
					HideAdvancedSearchDlg(hwndDlg,dat);
					break;
				case IDC_BYEMAIL:
				case IDC_BYNAME:
					EnableWindow(GetDlgItem(hwndDlg,IDC_ADD), ListView_GetSelectedCount(hwndList) > 0);
					HideAdvancedSearchDlg(hwndDlg,dat);
					break;
				case IDC_PROTOID:
					if(HIWORD(wParam)==EN_CHANGE) {
						HideAdvancedSearchDlg(hwndDlg,dat);
						CheckSearchTypeRadioButton(hwndDlg,IDC_BYPROTOID);
						EnableWindow(GetDlgItem(hwndDlg,IDC_ADD),TRUE);
					}
					break;
				case IDC_EMAIL:
					if(HIWORD(wParam)==EN_CHANGE) {
						HideAdvancedSearchDlg(hwndDlg,dat);
						CheckSearchTypeRadioButton(hwndDlg,IDC_BYEMAIL);
					}
					break;
				case IDC_NAMENICK:
				case IDC_NAMEFIRST:
				case IDC_NAMELAST:
					if(HIWORD(wParam)==EN_CHANGE) {
						HideAdvancedSearchDlg(hwndDlg,dat);
						CheckSearchTypeRadioButton(hwndDlg,IDC_BYNAME);
					}
					break;
				case IDC_ADVANCED:
					EnableWindow(GetDlgItem(hwndDlg,IDC_ADD), ListView_GetSelectedCount(hwndList) > 0);
					if(IsDlgButtonChecked(hwndDlg,IDC_ADVANCED))
						ShowAdvancedSearchDlg(hwndDlg,dat);
					else
						HideAdvancedSearchDlg(hwndDlg,dat);
					CheckSearchTypeRadioButton(hwndDlg,IDC_BYADVANCED);
					break;
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
				case IDOK:
				{	
					HideAdvancedSearchDlg(hwndDlg,dat);
					if(dat->searchCount) {	 //cancel search
						SetDlgItemText(hwndDlg,IDOK,TranslateT("&Search"));
						if(dat->hResultHook) {UnhookEvent(dat->hResultHook); dat->hResultHook=NULL;}
						if(dat->search) {mir_free(dat->search); dat->search=NULL;}
						dat->searchCount=0;
						StopThrobber(hwndDlg,dat);
						SetStatusBarSearchInfo(GetDlgItem(hwndDlg,IDC_STATUSBAR),dat);
						break;
					}
					char *szProto=(char*)SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETCURSEL,0,0),0);
					if(dat->search) {mir_free(dat->search); dat->search=NULL;}
					dat->searchCount=0;
					dat->hResultHook=HookEventMessage(ME_PROTO_ACK,hwndDlg,HM_SEARCHACK);
					if (IsDlgButtonChecked(hwndDlg,IDC_BYCUSTOM))
					{
						BeginSearch(hwndDlg,dat,szProto,PS_SEARCHBYADVANCED,PF1_EXTSEARCHUI,dat->hwndTinySearch);
					}
					else if(IsDlgButtonChecked(hwndDlg,IDC_BYPROTOID)) {
						TCHAR str[256];
						GetDlgItemText(hwndDlg,IDC_PROTOID,str,SIZEOF(str));
						rtrim(str);
						if(str[0]==0)
							MessageBox(hwndDlg,sttErrMsg,sttErrTitle,MB_OK);
						else
							BeginSearch(hwndDlg,dat,szProto,PS_BASICSEARCHT,PF1_BASICSEARCH,str);
					}
					else if(IsDlgButtonChecked(hwndDlg,IDC_BYEMAIL)) {
						TCHAR str[256];
						GetDlgItemText(hwndDlg,IDC_EMAIL,str,SIZEOF(str));
						rtrim(str);
						if(str[0]==0)
							MessageBox(hwndDlg,sttErrMsg,sttErrTitle,MB_OK);
						else
							BeginSearch(hwndDlg,dat,szProto,PS_SEARCHBYEMAILT,PF1_SEARCHBYEMAIL,str);
					}
					else if(IsDlgButtonChecked(hwndDlg,IDC_BYNAME)) {
						TCHAR nick[256],first[256],last[256];
						PROTOSEARCHBYNAME psbn;
						GetDlgItemText(hwndDlg,IDC_NAMENICK,nick,SIZEOF(nick));
						GetDlgItemText(hwndDlg,IDC_NAMEFIRST,first,SIZEOF(first));
						GetDlgItemText(hwndDlg,IDC_NAMELAST,last,SIZEOF(last));
						psbn.pszFirstName = first;
						psbn.pszLastName = last;
						psbn.pszNick = nick;
						if(nick[0]==0 && first[0]==0 && last[0]==0)
							MessageBox(hwndDlg,sttErrMsg,sttErrTitle,MB_OK);
						else
							BeginSearch(hwndDlg,dat,szProto,PS_SEARCHBYNAMET,PF1_SEARCHBYNAME,&psbn);
					}
					else if(IsDlgButtonChecked(hwndDlg,IDC_BYADVANCED)) {
						if(dat->hwndAdvSearch==NULL)
							MessageBox(hwndDlg,sttErrMsg,sttErrTitle,MB_OK);
						else
							BeginSearch(hwndDlg,dat,szProto,PS_SEARCHBYADVANCED,PF1_EXTSEARCHUI,dat->hwndAdvSearch);
					}

					if(dat->searchCount==0) {
						if(dat->hResultHook) {UnhookEvent(dat->hResultHook); dat->hResultHook=NULL;}
						break;
					}

					dat->notSearchedYet=0;
					FreeSearchResults(hwndList);

					CreateResultsColumns(hwndList,dat,szProto);
					SetStatusBarSearchInfo(GetDlgItem(hwndDlg,IDC_STATUSBAR),dat);
					SetStatusBarResultInfo(hwndDlg);
					StartThrobber(hwndDlg,dat);
					SetDlgItemText(hwndDlg, IDOK, TranslateT("Cancel"));
					break;
				}
				case IDC_ADD:
				{	LVITEM lvi;
					struct ListSearchResult *lsr;
					ADDCONTACTSTRUCT acs = {0};

					if (ListView_GetSelectedCount(hwndList) == 1) 
					{
						lvi.mask = LVIF_PARAM;
						lvi.iItem = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
						ListView_GetItem(hwndList, &lvi);
						lsr = (struct ListSearchResult*)lvi.lParam;
						acs.szProto = lsr->szProto;
						acs.psr = &lsr->psr;
					}
					else
					{
						TCHAR str[256];
						GetDlgItemText(hwndDlg, IDC_PROTOID, str, SIZEOF(str));
						if (*rtrim(str) == 0) break;

						PROTOSEARCHRESULT psr = {0};
						psr.cbSize = sizeof(psr);
						psr.flags = PSR_TCHAR;
						psr.id = str;

						acs.psr = &psr;
						acs.szProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA,
							SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), 0);
					}

					acs.handleType = HANDLE_SEARCHRESULT;
					CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
					break;
				}
				case IDC_MOREOPTIONS:
				{	RECT rc;
					GetWindowRect(GetDlgItem(hwndDlg,IDC_MOREOPTIONS),&rc);
					ShowMoreOptionsMenu(hwndDlg,rc.left,rc.bottom);
					break;
				}
			}
			if (lParam && dat->hwndTinySearch==(HWND)lParam
				&& HIWORD(wParam)==EN_SETFOCUS && LOWORD(wParam)==0
				&& !IsDlgButtonChecked(hwndDlg, IDC_BYCUSTOM))	{
					CheckSearchTypeRadioButton(hwndDlg, IDC_BYCUSTOM);
			}
			break;
		case WM_CONTEXTMENU:
		{	POINT pt;
			LVHITTESTINFO lvhti;

			pt.x=(short)LOWORD(lParam); pt.y=(short)HIWORD(lParam);
			lvhti.pt=pt;
			ScreenToClient(hwndDlg,&pt);
			switch(GetDlgCtrlID(ChildWindowFromPoint(hwndDlg,pt))) {
				case IDC_RESULTS:
					if(dat->notSearchedYet) return TRUE;
					ScreenToClient(hwndList,&lvhti.pt);
					if(ListView_HitTest(hwndList,&lvhti)==-1) break;
					ShowMoreOptionsMenu(hwndDlg,(short)LOWORD(lParam),(short)HIWORD(lParam));
					return TRUE;
			}
			break;
		}
		case HM_SEARCHACK:
		{	ACKDATA *ack=(ACKDATA*)lParam;
			int i;

			if(ack->type!=ACKTYPE_SEARCH) break;
			for(i=0;i<dat->searchCount;i++)
				if(dat->search[i].hProcess==ack->hProcess && dat->search[i].hProcess != NULL && !lstrcmpA(dat->search[i].szProto,ack->szModule)) break;
			if(i==dat->searchCount) break;
			if(ack->result==ACKRESULT_SUCCESS || ack->result==ACKRESULT_FAILED) {
				dat->searchCount--;
				memmove(dat->search+i,dat->search+i+1,sizeof(struct ProtoSearchInfo)*(dat->searchCount-i));
				if(dat->searchCount==0) {
					mir_free(dat->search);
					dat->search=NULL;
					UnhookEvent(dat->hResultHook);
					dat->hResultHook=NULL;
					SetDlgItemText(hwndDlg, IDOK, TranslateT("&Search"));
					StopThrobber(hwndDlg,dat);
				}
				ListView_SortItemsEx(hwndList, SearchResultsCompareFunc, (LPARAM)hwndDlg);
				SetStatusBarSearchInfo(GetDlgItem(hwndDlg,IDC_STATUSBAR),dat);
			}
			else if(ack->result==ACKRESULT_SEARCHRESULT && ack->lParam) {

				PROTOSEARCHRESULT *psr;
				CUSTOMSEARCHRESULTS * csr=(CUSTOMSEARCHRESULTS*)ack->lParam;
				dat->bFlexSearchResult=TRUE;
				psr=&(csr->psr);
				// check if this is column names data (psr->cbSize==0)
				if ( psr->cbSize==0 ){ // blob contain info about columns

					int iColumn;
					LVCOLUMN lvc={0};

					//firstly remove all exist items
					FreeSearchResults(hwndList);
					ListView_DeleteAllItems(hwndList); //not sure if previous delete list items too
					//secondly remove all columns
					while (ListView_DeleteColumn(hwndList,1)); //will delete fist column till it possible
					//now will add columns and captions;
					lvc.mask=LVCF_TEXT;
					for (iColumn=0; iColumn<csr->nFieldCount; iColumn++)
					{
						lvc.pszText=TranslateTS(csr->pszFields[iColumn]);
						ListView_InsertColumn (hwndList, iColumn+1, &lvc) ;
					}
					// Column inserting Done
				} else {	//  blob contain info about found contacts

					LVITEM lvi = {0};
					int i, col;
					struct ListSearchResult *lsr;
					char *szComboProto;
					COMBOBOXEXITEM cbei = {0};

					lsr = (struct ListSearchResult*)mir_alloc(offsetof(struct ListSearchResult,psr)+psr->cbSize);
					lsr->szProto = ack->szModule;
					memcpy(&lsr->psr, psr, psr->cbSize);
					
					/* Next block is not needed but behavior will be kept */
					bool isUnicode = (psr->flags & PSR_UNICODE) != 0;
					if (psr->id)
					{
						BOOL validPtr = isUnicode ? IsBadStringPtrW((wchar_t*)psr->id, 25) : IsBadStringPtrA((char*)psr->id, 25);
						if (!validPtr)
						{
							isUnicode = false;
							lsr->psr.id = NULL;
						}
						else
							lsr->psr.id = isUnicode ? mir_u2t((wchar_t*)psr->id) : mir_a2t((char*)psr->id);
					}

					lsr->psr.nick = isUnicode ? mir_u2t((wchar_t*)psr->nick) : mir_a2t((char*)psr->nick);
					lsr->psr.firstName = isUnicode ? mir_u2t((wchar_t*)psr->firstName) : mir_a2t((char*)psr->firstName);
					lsr->psr.lastName = isUnicode ? mir_u2t((wchar_t*)psr->lastName) : mir_a2t((char*)psr->lastName);
					lsr->psr.email = isUnicode ? mir_u2t((wchar_t*)psr->email) : mir_a2t((char*)psr->email);
					lsr->psr.flags = psr->flags & ~PSR_UNICODE | PSR_TCHAR;

					lvi.mask = LVIF_PARAM | LVIF_IMAGE;
					lvi.lParam = (LPARAM)lsr;
					for (i = SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCOUNT, 0, 0); i--; ) 
					{
						szComboProto=(char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA, i, 0);
						if (szComboProto==NULL) continue;
						if (!lstrcmpA(szComboProto,ack->szModule)) 
						{
							cbei.mask = CBEIF_IMAGE;
							cbei.iItem = i;
							SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CBEM_GETITEM,0,(LPARAM)&cbei);
							lvi.iImage = cbei.iImage;
						}
					}
					i = ListView_InsertItem(hwndList, &lvi);
					for (col=0; col<csr->nFieldCount; col++) {
						SetListItemText(hwndList, i, col+1 , csr->pszFields[col] );
					}
					ListView_SortItemsEx(hwndList, SearchResultsCompareFunc, (LPARAM)hwndDlg);
					i=0;
					while (ListView_SetColumnWidth(hwndList, i++, LVSCW_AUTOSIZE_USEHEADER));
					SetStatusBarResultInfo(hwndDlg);
				}
				break;
			}
			else if(ack->result==ACKRESULT_DATA) {
				LVITEM lvi={0};
				int i,col;
				PROTOSEARCHRESULT *psr=(PROTOSEARCHRESULT*)ack->lParam;
				struct ListSearchResult *lsr;
				char *szComboProto;
				COMBOBOXEXITEM cbei={0};
				dat->bFlexSearchResult=FALSE;
				lsr=(struct ListSearchResult*)mir_alloc(offsetof(struct ListSearchResult,psr)+psr->cbSize);
				lsr->szProto=ack->szModule;

				CopyMemory(&lsr->psr, psr, psr->cbSize);
				lsr->psr.nick = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->nick) : mir_a2t((char*)psr->nick);
				lsr->psr.firstName = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->firstName) : mir_a2t((char*)psr->firstName);
				lsr->psr.lastName = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->lastName) : mir_a2t((char*)psr->lastName);
				lsr->psr.email = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->email) : mir_a2t((char*)psr->email);
				lsr->psr.id = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->id) : mir_a2t((char*)psr->id);
				lsr->psr.flags = psr->flags & ~PSR_UNICODE | PSR_TCHAR;

				lvi.mask = LVIF_PARAM|LVIF_IMAGE;
				lvi.lParam=(LPARAM)lsr;
				for(i = SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETCOUNT,0,0); i--; ) 
				{
					szComboProto=(char*)SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETITEMDATA,i,0);
					if(szComboProto==NULL) continue;
					if(!lstrcmpA(szComboProto,ack->szModule)) {
						cbei.mask=CBEIF_IMAGE;
						cbei.iItem=i;
						SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CBEM_GETITEM,0,(LPARAM)&cbei);
						lvi.iImage=cbei.iImage;
						break;
					}
				}
				i=ListView_InsertItem(hwndList, &lvi);
				col=1;
				SetListItemText(hwndList, i, col++, lsr->psr.id );
				SetListItemText(hwndList, i, col++, lsr->psr.nick );
				SetListItemText(hwndList, i, col++, lsr->psr.firstName );
				SetListItemText(hwndList, i, col++, lsr->psr.lastName );
				SetListItemText(hwndList, i, col++, lsr->psr.email );
				SetStatusBarResultInfo(hwndDlg);
			}
			break;
		}
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
		case WM_DESTROY:
			{
				TCHAR *szProto;
				int len = SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETLBTEXTLEN,SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETCURSEL,0,0),0);
				szProto = ( TCHAR* )alloca( sizeof(TCHAR)*( len+1 ));
				*szProto='\0';
				SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETLBTEXT,SendDlgItemMessage(hwndDlg,IDC_PROTOLIST,CB_GETCURSEL,0,0),(LPARAM)szProto);
				DBWriteContactSettingTString(NULL, "FindAdd", "LastSearched", szProto?szProto:_T(""));
			}
			SaveColumnSizes(hwndList);
			if(dat->hResultHook!=NULL) UnhookEvent(dat->hResultHook);
			FreeSearchResults(hwndList);
			ImageList_Destroy(dat->himlComboIcons);
			mir_free(dat->search);
			if(dat->hwndAdvSearch) {
				DestroyWindow(dat->hwndAdvSearch);
				dat->hwndAdvSearch=NULL;
			}
			if(dat->hwndTinySearch) {
				DestroyWindow(dat->hwndTinySearch);
				dat->hwndTinySearch=NULL;
			}
			DeleteObject(dat->hBmpSortDown);
			DeleteObject(dat->hBmpSortUp);
			mir_free(dat);
			Window_FreeIcon_IcoLib(hwndDlg);
			Utils_SaveWindowPosition(hwndDlg,NULL,"FindAdd","");

			break;
	}
	return FALSE;
}

static INT_PTR FindAddCommand(WPARAM, LPARAM)
{
	if(IsWindow(hwndFindAdd)) {
		ShowWindow(hwndFindAdd,SW_SHOWNORMAL);
		SetForegroundWindow(hwndFindAdd);
		SetFocus(hwndFindAdd);
	}
	else {
		int netProtoCount, i;

		// Make sure we have some networks to search on. This is not ideal since
		// this check will be repeated every time the dialog is requested, but it
		// must be done since this service can be called from other places than the menu.
		// One alternative would be to only create the service if we have network
		// protocols loaded but that would delay the creation until MODULE_LOADED and
		// that is not good either...
		for ( i=0, netProtoCount=0; i < accounts.getCount(); i++ ) {
			PROTOACCOUNT* pa = accounts[i];
			if (!Proto_IsAccountEnabled(pa)) continue;
			int protoCaps=CallProtoService( pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0 );
			if ( protoCaps&PF1_BASICSEARCH || protoCaps&PF1_SEARCHBYEMAIL || protoCaps&PF1_SEARCHBYNAME
				|| protoCaps&PF1_EXTSEARCHUI ) netProtoCount++;
		}
		if (netProtoCount > 0)
			hwndFindAdd=CreateDialog(hMirandaInst, MAKEINTRESOURCE(IDD_FINDADD), NULL, DlgProcFindAdd);
	}
	return 0;
}

int FindAddPreShutdown(WPARAM, LPARAM)
{
	if ( IsWindow( hwndFindAdd ))
		DestroyWindow(hwndFindAdd);
	hwndFindAdd = NULL;
	return 0;
}

int LoadFindAddModule(void)
{
	CreateServiceFunction(MS_FINDADD_FINDADD,FindAddCommand);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnSystemModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN,FindAddPreShutdown);

	CLISTMENUITEM mi = { 0 };
	mi.cbSize     = sizeof(mi);
	mi.position   = 500020000;
	mi.flags      = CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetSkinIconHandle( SKINICON_OTHER_FINDUSER );
	mi.pszName    = LPGEN("&Find/Add Contacts...");
	mi.pszService = MS_FINDADD_FINDADD;
	hMainMenuItem = ( HANDLE )CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
	return 0;
}

static int OnSystemModulesLoaded(WPARAM, LPARAM)
{
	int netProtoCount, i;

	// Make sure we have some networks to search on.
	for ( i=0, netProtoCount=0; i < accounts.getCount(); i++ ) {
		PROTOACCOUNT* pa = accounts[i];
		int protoCaps = CallProtoService( pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0 );
		if ( protoCaps & ( PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_EXTSEARCHUI ))
			netProtoCount++;
	}

	CLISTMENUITEM cmi = { 0 };
	cmi.cbSize = sizeof(cmi);
	cmi.flags = CMIM_FLAGS;
	if ( netProtoCount == 0 )
		cmi.flags |= CMIF_HIDDEN;
	CallService( MS_CLIST_MODIFYMENUITEM, (WPARAM)hMainMenuItem, (LPARAM)&cmi );
	return 0;
}
