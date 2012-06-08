#include "common.h"
#include <shlobj.h>
#pragma hdrstop

static INT_PTR CALLBACK DlgProcTTBBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern int ttbOptionsChanged();
extern HINSTANCE hInst;

extern TopButtonInt Buttons[MAX_BUTTONS];
extern int nButtonsCount;

extern SortData arrangedbuts[MAX_BUTTONS];

bool OptionsOpened = false;
HWND OptionshWnd = 0;

struct OrderData
{
	int dragging;
	HTREEITEM hDragItem;
};

TCHAR *SetEndSlash(TCHAR *str)
{
	if (str == NULL || *str == 0)
		return NULL;

	TCHAR *outs = str;
	if (str[_tcslen(str)-1] != '\\') {
		outs = (TCHAR*)calloc( sizeof(TCHAR), _tcslen(str)+3);
		_tcscat(outs, str);		
		outs[_tcslen(str)] = '\\';
	}
	return outs;
}

bool FileExists(TCHAR *fname)
{
	WIN32_FIND_DATA wfd = { 0 };
	HANDLE h = FindFirstFile(fname, &wfd);
	if (h == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(h);
	return TRUE;
}

TCHAR *ReplaceBadChar(TCHAR *str, TCHAR bad, TCHAR toreplace)
{
	for (int i = 0; str[i] != 0; i++)
		if (str[i] == bad)
			str[i] = toreplace;

	return str;
}

TCHAR *ReplaceAll(TCHAR *str)
{
	ReplaceBadChar(str, '\\', '_');
	ReplaceBadChar(str, '/', '_');
	ReplaceBadChar(str, '"', '_');
	ReplaceBadChar(str, '?', '_');
	ReplaceBadChar(str, '|', '_');
	ReplaceBadChar(str, '>', '_');
	ReplaceBadChar(str, '<', '_');
	ReplaceBadChar(str, ':', '_');
	ReplaceBadChar(str, '*', '_');
	return str;
}

void AssignBitmapsFromDir(TCHAR *dir)
{
	if (dir == NULL || *dir == 0 )
		return;

	dir = SetEndSlash(dir);
	lockbut();	
	for (int i = 0; i < nButtonsCount; i++) {
		TCHAR curnameUP[512], curnameDN[512], ChangedName[512];
		_tcscpy(ChangedName, _A2T(Buttons[i].name));
		ReplaceAll(ChangedName);

		wsprintf(curnameUP, _T("%s%s_UP.bmp"), dir, ChangedName);
		wsprintf(curnameDN, _T("%s%s_DN.bmp"), dir, ChangedName);

		if ( FileExists(curnameUP)) {
			if (Buttons[i].UserDefinedbmUp != NULL){free(Buttons[i].UserDefinedbmUp);}
			Buttons[i].UserDefinedbmUp = _tcsdup(curnameUP);
		}

		if ( FileExists(curnameDN)) {
			if (Buttons[i].UserDefinedbmDown != NULL){free(Buttons[i].UserDefinedbmDown);}
			Buttons[i].UserDefinedbmDown = _tcsdup(curnameDN);
		}

		wsprintf(curnameUP, _T("%s%s_UP.ico"), dir, ChangedName);
		wsprintf(curnameDN, _T("%s%s_DN.ico"), dir, ChangedName);

		if ( FileExists(curnameUP)) {
			if (Buttons[i].UserDefinedbmUp != NULL){free(Buttons[i].UserDefinedbmUp);}
			Buttons[i].UserDefinedbmUp = _tcsdup(curnameUP);
		}

		if ( FileExists(curnameDN)) {
			if (Buttons[i].UserDefinedbmDown != NULL){free(Buttons[i].UserDefinedbmDown);}
			Buttons[i].UserDefinedbmDown = _tcsdup(curnameDN);
		}
	}

	ulockbut();		
	SetAllBitmaps();
	SaveAllButtonsOptions();
}

void ApplyNewDir(HWND hwnd)
{
	TCHAR buf[512], *buf2;
	GetDlgItemText(hwnd, IDC_IMGDIR, buf, SIZEOF(buf));
	buf2 = DBGetStringT(0, TTB_OPTDIR, "ImgDir");
	if ( buf2 == NULL || lstrcmp(buf, buf2)) {
		DBWriteContactSettingTString(0, TTB_OPTDIR, "ImgDir", buf);
		AssignBitmapsFromDir(buf);
	}
}

int BuildTree(HWND hwndDlg)
{
	PostMessage(GetDlgItem(hwndDlg, IDC_BMPUP), STM_SETIMAGE, IMAGE_BITMAP, 0);
	PostMessage(GetDlgItem(hwndDlg, IDC_BMPDN), STM_SETIMAGE, IMAGE_BITMAP, 0);

	OrderData *dat = (struct OrderData*)GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), GWLP_USERDATA);

	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;	
	TreeView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));

	if (nButtonsCount == 0)
		return FALSE;

	for (int i = 0; i < nButtonsCount; i++) {
		ButtonOptData *PD = (ButtonOptData*)malloc(sizeof(ButtonOptData));
		PD->name = Buttons[arrangedbuts[i].oldpos].name;			
		PD->show = Buttons[arrangedbuts[i].oldpos].dwFlags&TTBBF_VISIBLE?TRUE:FALSE;			
		PD->pos = arrangedbuts[i].oldpos;					
		tvis.item.lParam = (LPARAM)PD;
		tvis.item.pszText = TranslateTS( mir_a2t( PD->name));
		tvis.item.iImage = tvis.item.iSelectedImage = PD->show;
		TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvis);
		mir_free( tvis.item.pszText );

		tvis.item.iImage = tvis.item.iSelectedImage = PD->show;
	}

	return (TRUE);
}

//call this when options opened and buttons added/removed
int OptionsPageRebuild()
{
	if (OptionsOpened)	
		BuildTree(OptionshWnd);

	return 0;
}

int SaveTree(HWND hwndDlg)
{
	TCHAR idstr[100];
	TVITEM tvi;
	tvi.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));
	tvi.cchTextMax = 99;
	tvi.mask = TVIF_TEXT|TVIF_PARAM|TVIF_HANDLE;
	tvi.pszText = idstr;

	int count = 0;
	lockbut();

	while(tvi.hItem != NULL) {
		TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);
		char* name = ((ButtonOptData *)tvi.lParam)->name;
		int pos = ((ButtonOptData *)tvi.lParam)->pos;
		if (pos >= 0 && pos < nButtonsCount) {
			Buttons[pos].dwFlags &= ~(TTBBF_VISIBLE);
			Buttons[pos].dwFlags |= (((ButtonOptData *)tvi.lParam)->show == TRUE ) ? TTBBF_VISIBLE : 0;
			Buttons[pos].arrangedpos = count;
		}

		tvi.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), tvi.hItem);
		count++;
	}

	ulockbut();
	ttbOptionsChanged();
	return (TRUE);
}

void SetImagesForCurrent (HWND hwndDlg, int curselect)
{
	int t = IMAGE_BITMAP;

	PostMessage(GetDlgItem(hwndDlg, IDC_BMPUP), STM_SETIMAGE, t, 0);
	PostMessage(GetDlgItem(hwndDlg, IDC_BMPDN), STM_SETIMAGE, t, 0);

	TCHAR *curname = Buttons[curselect].UserDefinedbmUp;
	if (curname != NULL) {
		if ( _tcsstr(curname, _T(".ico")))
			t = IMAGE_ICON;
		int st = WS_CHILDWINDOW | SS_NOTIFY | WS_VISIBLE;
		st |= (t == IMAGE_ICON) ? SS_ICON : SS_BITMAP;
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BMPUP), GWL_STYLE, st);
	}
	else {									
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BMPUP), GWL_STYLE, WS_CHILDWINDOW|WS_VISIBLE|SS_NOTIFY|(Buttons[curselect].hbBitmapDown == NULL?SS_ICON:SS_BITMAP));
		t = IMAGE_ICON;
	}

	PostMessage(GetDlgItem(hwndDlg, IDC_BMPUP), STM_SETIMAGE, t, (Buttons[curselect].hbBitmapUp == NULL) ? (LPARAM)Buttons[curselect].hIconUp : (LPARAM)Buttons[curselect].hbBitmapUp);

	curname = Buttons[curselect].UserDefinedbmDown;
	if (curname != NULL) {
		if ( _tcsstr(curname, _T(".ico")))
			t = IMAGE_ICON;
		int st = WS_CHILDWINDOW | SS_NOTIFY | WS_VISIBLE;
		st |= (t == IMAGE_ICON) ? SS_ICON : SS_BITMAP;
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BMPDN), GWL_STYLE, st);
	}
	else {
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BMPDN), GWL_STYLE, WS_CHILDWINDOW|WS_VISIBLE|SS_NOTIFY|(Buttons[curselect].hbBitmapDown == NULL?SS_ICON:SS_BITMAP));
		t = IMAGE_ICON;
	}

	PostMessage(GetDlgItem(hwndDlg, IDC_BMPDN), STM_SETIMAGE, t, (Buttons[curselect].hbBitmapDown == NULL) ? (LPARAM)Buttons[curselect].hIconDn : (LPARAM)Buttons[curselect].hbBitmapDown);
}

static INT_PTR CALLBACK ButOrderOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct OrderData *dat = (struct OrderData*)GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			dat = (struct OrderData*)malloc(sizeof(struct OrderData));
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), GWLP_USERDATA, (LONG)dat);
			dat->dragging = 0;

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), GWL_STYLE)|TVS_NOHSCROLL);
			{
				HIMAGELIST himlCheckBoxes;
				himlCheckBoxes = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR4|ILC_MASK, 2, 2);
				ImageList_AddIcon(himlCheckBoxes, LoadIcon(hInst, MAKEINTRESOURCE(IDI_NOTICK)));
				ImageList_AddIcon(himlCheckBoxes, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TICK)));
				TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), himlCheckBoxes, TVSIL_NORMAL);
			}

			PostMessage(GetDlgItem(hwndDlg, IDC_BMPUP), STM_SETIMAGE, IMAGE_BITMAP, 0);
			PostMessage(GetDlgItem(hwndDlg, IDC_BMPDN), STM_SETIMAGE, IMAGE_BITMAP, 0);
			SetDlgItemInt(hwndDlg, IDC_BUTTHEIGHT, DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTHEIGHT", 16), FALSE);
			SetDlgItemInt(hwndDlg, IDC_BUTTWIDTH, DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTWIDTH", 20), FALSE);
			CheckDlgButton(hwndDlg, IDC_USEMIRANDABUTTON, DBGetContactSettingByte(0, TTB_OPTDIR, "UseMirandaButtonClass", UseMirandaButtonClassDefaultValue));
			CheckDlgButton(hwndDlg, IDC_USEFLAT, DBGetContactSettingByte(0, TTB_OPTDIR, "UseFlatButton", 1));

			if ( !ServiceExists(MS_SKIN2_ADDICON))
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEICOLIB), FALSE);

			CheckDlgButton(hwndDlg, IDC_USEICOLIB, ServiceExists(MS_SKIN2_ADDICON)&&DBGetContactSettingByte(0, TTB_OPTDIR, "UseIcoLib", UseIcoLibDefaultValue));

			BuildTree(hwndDlg);
			OptionsOpened = true;
			EnableWindow(GetDlgItem(hwndDlg, IDC_ENAME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EPATH), FALSE);							
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELLBUTTON), FALSE);

			SetDlgItemText(hwndDlg, IDC_IMGDIR, DBGetStringT(0, TTB_OPTDIR, "ImgDir"));
			SendMessage(hwndDlg, WM_COMMAND, 0, 0);
			OptionshWnd = hwndDlg;
		}
		return TRUE;

	case WM_COMMAND:
		ShowWindow(GetDlgItem(hwndDlg, IDC_BMPUP), IsDlgButtonChecked(hwndDlg, IDC_USEICOLIB)?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_BMPDN), IsDlgButtonChecked(hwndDlg, IDC_USEICOLIB)?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_DEFAULT), IsDlgButtonChecked(hwndDlg, IDC_USEICOLIB)?SW_HIDE:SW_SHOW);

		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC1), IsDlgButtonChecked(hwndDlg, IDC_USEICOLIB)?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC2), IsDlgButtonChecked(hwndDlg, IDC_USEICOLIB)?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC3), IsDlgButtonChecked(hwndDlg, IDC_USEICOLIB)?SW_HIDE:SW_SHOW);

		EnableWindow(GetDlgItem(hwndDlg, IDC_USEFLAT), IsDlgButtonChecked(hwndDlg, IDC_USEMIRANDABUTTON));

		if (HIWORD(wParam) == EN_CHANGE ) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}

		if ((HIWORD(wParam) == BN_CLICKED|| HIWORD(wParam) == BN_DBLCLK)) {
			int ctrlid = LOWORD(wParam);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			if (ctrlid == IDC_OPENIMGDIR) {
				TCHAR buf[512];
				BROWSEINFO bi = { 0 };
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = buf;
				bi.lpszTitle = TranslateT("Select Directory");
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				PCIDLIST_ABSOLUTE res = SHBrowseForFolder(&bi);
				if (res != NULL) {
					if (SHGetPathFromIDList(res, buf) == TRUE) {
						SetDlgItemText(hwndDlg, IDC_IMGDIR, buf);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
				break;
			}

			if (ctrlid == IDC_LBUTTONSET) {
				int curselect;
				TVITEM tvi;
				tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));
				if (tvi.hItem == NULL){break;}

				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);

				curselect = ((ButtonOptData *)tvi.lParam)->pos; 
				if (Buttons[curselect].dwFlags & TTBBF_ISLBUTTON) {
					LBUTOPT lbo = { 0 };
					TCHAR buf[256];
					GetDlgItemText(hwndDlg, IDC_ENAME, buf, 255);
					lbo.name = _strdup( _T2A(buf));

					GetDlgItemText(hwndDlg, IDC_EPATH, buf, 255);
					lbo.lpath = _tcsdup(buf);

					CallService(TTB_MODIFYLBUTTON, Buttons[curselect].lParamDown, (LPARAM)&lbo);
					free(lbo.name);
					free(lbo.lpath);
				}
				break;
			}

			if (ctrlid == IDC_ADDLBUTTON) {				
				if (CallService(TTB_ADDLBUTTON, 0, 0) == 0) {
					//	BuildTree(hwndDlg);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);					
				}
				break;
			}

			if (ctrlid == IDC_DELLBUTTON) {
				int curselect;
				TVITEM tvi;
				tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));
				if (tvi.hItem == NULL)
					break;

				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);

				curselect = ((ButtonOptData *)tvi.lParam)->pos; 
				if (Buttons[curselect].dwFlags & TTBBF_ISLBUTTON) {
					CallService(TTB_REMOVELBUTTON, Buttons[curselect].lParamDown, 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ENAME), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EPATH), FALSE);
					BuildTree(hwndDlg);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}				

			if (ctrlid == IDC_ADDSEP) {
				if (CallService(TTB_ADDSEPARATOR, 0, 0) == 0) {
					//	BuildTree(hwndDlg);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);					
				}
				break;
			}

			if (ctrlid == IDC_REMOVESEP) {
				TVITEM tvi;
				memset(&tvi, 0, sizeof(tvi));
				tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));
				if (tvi.hItem == NULL){break;}

				//MessageBoxA(0, "GetSelItem", "log", 0);
				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);

				if (tvi.lParam == 0)
					break;

				if ( IsBadCodePtr(( FARPROC )tvi.lParam)) {
					MessageBoxA(0, "Bad Code Ptr: tvi.lParam", "log", 0);
					break;
				}
				
				int curselect = ((ButtonOptData *)tvi.lParam)->pos; 
				if ( curselect >= 0 && curselect < nButtonsCount ) {
					if (Buttons[curselect].dwFlags&TTBBF_ISSEPARATOR) {
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						CallService(TTB_REMOVESEPARATOR, Buttons[curselect].lParamDown, 0);
					}
				}
				break;
			}

			if (ctrlid == IDC_DEFAULT) {
				TVITEM tvi;
				tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));
				if (tvi.hItem == NULL)
					break;

				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);

				int curselect = ((ButtonOptData *)tvi.lParam)->pos; 
				lockbut();

				if (Buttons[curselect].UserDefinedbmUp != NULL) free(Buttons[curselect].UserDefinedbmUp);
				if (Buttons[curselect].UserDefinedbmDown != NULL) free(Buttons[curselect].UserDefinedbmDown);
				Buttons[curselect].UserDefinedbmUp = NULL;
				Buttons[curselect].UserDefinedbmDown = NULL;
				applyuserbitmaps(curselect);
				SetImagesForCurrent(hwndDlg, curselect);

				ulockbut();
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);					
			}
		}

		if ((HIWORD(wParam) == STN_CLICKED|| HIWORD(wParam) == STN_DBLCLK)) {
			int ctrlid = LOWORD(wParam);
			if (ctrlid == IDC_BMPUP||ctrlid == IDC_BMPDN) {
				TVITEM tvi;
				tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));
				if (tvi.hItem == NULL)
					break;

				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);
				if (tvi.lParam == 0)
					break;
				
				int curselect = ((ButtonOptData *)tvi.lParam)->pos;

				TCHAR filename[MAX_PATH], filter[512];
				filename[0] = 0;
				_tcscpy(filter, TranslateT("Bitmap/Icon files"));
				_tcscat(filter, _T(" (*.bmp;*.ico)"));

				TCHAR *pfilter = filter+_tcslen(filter)+1;					
				_tcscpy(pfilter, _T("*.bmp;*.ico"));
				pfilter += _tcslen(pfilter)+1;

				_tcscpy(pfilter, TranslateT("All Files"));
				_tcscat(pfilter, _T("(*)"));
				pfilter = pfilter + _tcslen(pfilter)+1;

				_tcscpy(pfilter, _T("*"));
				pfilter = pfilter + _tcslen(pfilter)+1;
				*pfilter = '\0';

				OPENFILENAME ofn = {0};
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = NULL;
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = filename;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.nMaxFile = sizeof(filename);
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = _T("bmp");
				ofn.lpstrTitle = (ctrlid == IDC_BMPUP) ? TranslateT("Select Up Bitmap") : TranslateT("Select Down Bitmap");
				if (GetOpenFileName(&ofn)) {
					lockbut();
					if (ctrlid == IDC_BMPUP){
						if (Buttons[curselect].UserDefinedbmUp != NULL){free(Buttons[curselect].UserDefinedbmUp);}
						Buttons[curselect].UserDefinedbmUp = _tcsdup(ofn.lpstrFile);
					}
					if (ctrlid == IDC_BMPDN){
						if (Buttons[curselect].UserDefinedbmDown != NULL){free(Buttons[curselect].UserDefinedbmDown);}
						Buttons[curselect].UserDefinedbmDown = _tcsdup(ofn.lpstrFile);
					}
					applyuserbitmaps(curselect);
					SetImagesForCurrent(hwndDlg, curselect);

					ulockbut();
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0: 
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				DBWriteContactSettingByte(0, TTB_OPTDIR, "BUTTHEIGHT", GetDlgItemInt(hwndDlg, IDC_BUTTHEIGHT, FALSE, FALSE));
				DBWriteContactSettingByte(0, TTB_OPTDIR, "BUTTWIDTH", GetDlgItemInt(hwndDlg, IDC_BUTTWIDTH, FALSE, FALSE));
				DBWriteContactSettingByte(0, TTB_OPTDIR, "UseMirandaButtonClass", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USEMIRANDABUTTON));
				DBWriteContactSettingByte(0, TTB_OPTDIR, "UseFlatButton", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USEFLAT));

				DBWriteContactSettingByte(0, TTB_OPTDIR, "UseIcoLib", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USEICOLIB));


				SaveTree(hwndDlg);
				ApplyNewDir(hwndDlg);
				RecreateWindows();
				ArrangeButtons();
			}
			break;

		case IDC_BUTTONORDERTREE:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_BEGINDRAG:
				SetCapture(hwndDlg);
				dat->dragging = 1;
				dat->hDragItem = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
				TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), dat->hDragItem);
				break;

			case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x = (short)LOWORD(GetMessagePos());
					hti.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
					if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {						
						if (hti.flags&TVHT_ONITEMICON) {
							TVITEM tvi;
							tvi.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
							tvi.hItem = hti.hItem;
							TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
							tvi.iImage = tvi.iSelectedImage = !tvi.iImage;
							((ButtonOptData *)tvi.lParam)->show = tvi.iImage;
							TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
					}
				}
				break;

			case TVN_SELCHANGED:
				{
					TVITEM tvi;
					HTREEITEM hti;

					hti = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE));
					if (hti == NULL){break;}
					tvi.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
					tvi.hItem = hti;
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);


					lockbut();
					EnableWindow(GetDlgItem(hwndDlg, IDC_BMPUP), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BMPDN), FALSE);							

					SetImagesForCurrent(hwndDlg, ((ButtonOptData *)tvi.lParam)->pos);
					//PostMessage(GetDlgItem(hwndDlg, IDC_BMPUP), STM_SETIMAGE, IMAGE_BITMAP, Buttons[((ButtonOptData *)tvi.lParam)->pos].hbBitmapUp);
					//PostMessage(GetDlgItem(hwndDlg, IDC_BMPDN), STM_SETIMAGE, IMAGE_BITMAP, Buttons[((ButtonOptData *)tvi.lParam)->pos].hbBitmapDown);

					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVESEP), FALSE);
					if (Buttons[((ButtonOptData *)tvi.lParam)->pos].dwFlags&TTBBF_ISSEPARATOR)
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVESEP), TRUE);							
					else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_BMPUP), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BMPDN), TRUE);							
					}

					EnableWindow(GetDlgItem(hwndDlg, IDC_ENAME), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EPATH), FALSE);							
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELLBUTTON), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_LBUTTONSET), FALSE);
					SetDlgItemTextA(hwndDlg, IDC_ENAME, "");
					SetDlgItemTextA(hwndDlg, IDC_EPATH, "");

					if (Buttons[((ButtonOptData *)tvi.lParam)->pos].dwFlags & TTBBF_ISLBUTTON) {
						LBUTOPT lbo = { 0 };
						CallService(TTB_GETLBUTTON, Buttons[((ButtonOptData *)tvi.lParam)->pos].lParamDown, (LPARAM)&lbo);
						if (lbo.hframe != 0) {
							EnableWindow(GetDlgItem(hwndDlg, IDC_ENAME), TRUE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_EPATH), TRUE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_DELLBUTTON), TRUE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_LBUTTONSET), TRUE);

							if (lbo.name != NULL)
								SetDlgItemTextA(hwndDlg, IDC_ENAME, lbo.name);
							if (lbo.lpath != NULL)
								SetDlgItemText(hwndDlg, IDC_EPATH, lbo.lpath);
						}
					}

					ulockbut();
				}
			}
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if (dat->dragging) {
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &hti.pt);
			TreeView_HitTest(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &hti);
			if (hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
				hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE))/2;
				TreeView_HitTest(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &hti);
				TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), hti.hItem, 1);
			}
			else {
				if (hti.flags&TVHT_ABOVE) SendDlgItemMessage(hwndDlg, IDC_BUTTONORDERTREE, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags&TVHT_BELOW) SendDlgItemMessage(hwndDlg, IDC_BUTTONORDERTREE, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), NULL, 0);
			}
		}
		break;

	case WM_DESTROY:
		OptionsOpened = false;
		return 0;

	case WM_LBUTTONUP:
		if (dat->dragging) {
			TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), NULL, 0);
			dat->dragging = 0;
			ReleaseCapture();

			TVHITTESTINFO hti;
			TVITEM tvi;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &hti.pt);
			hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE))/2;
			TreeView_HitTest(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &hti);
			if (dat->hDragItem == hti.hItem) break;
			tvi.mask = TVIF_HANDLE|TVIF_PARAM;
			tvi.hItem = hti.hItem;
			TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvi);
			if (hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
				TVINSERTSTRUCT tvis;
				TCHAR name[128];
				tvis.item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
				tvis.item.stateMask = 0xFFFFFFFF;
				tvis.item.pszText = name;
				tvis.item.cchTextMax = SIZEOF(name);
				tvis.item.hItem = dat->hDragItem;
				//
				tvis.item.iImage = tvis.item.iSelectedImage = ((ButtonOptData *)tvi.lParam)->show;

				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvis.item);

				TreeView_DeleteItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), dat->hDragItem);
				tvis.hParent = NULL;
				tvis.hInsertAfter = hti.hItem;
				TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE), &tvis));
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				SaveTree(hwndDlg);
			}
		}
		break; 
	}
	return FALSE;
}

int TTBOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.pszGroup = Translate("TopToolBar");

	if ( !ServiceExists(MS_BACKGROUNDCONFIG_REGISTER)) {	
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TTBBKG);
		odp.pszTitle = Translate("TTBBackground");
		odp.pfnDlgProc = DlgProcTTBBkgOpts;
		odp.flags = ODPF_BOLDGROUPS;
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	}

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = -1000000000;
	odp.hInstance = hInst;//GetModuleHandle(NULL);
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_BUTORDER);
	odp.pszGroup = Translate("TopToolBar");
	odp.pszTitle = Translate("Buttons");
	odp.pfnDlgProc = ButOrderOpts;
	odp.flags = ODPF_BOLDGROUPS|ODPF_EXPERTONLY;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

static INT_PTR CALLBACK DlgProcTTBBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_BITMAP, DBGetContactSettingByte(NULL, TTB_OPTDIR, "UseBitmap", TTBDEFAULT_USEBITMAP)?BST_CHECKED:BST_UNCHECKED);
		SendMessage(hwndDlg, WM_USER+10, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, TTBDEFAULT_BKCOLOUR);
		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, TTB_OPTDIR, "BkColour", TTBDEFAULT_BKCOLOUR));
		SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETDEFAULTCOLOUR, 0, TTBDEFAULT_SELBKCOLOUR);
		SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, TTB_OPTDIR, "SelBkColour", TTBDEFAULT_SELBKCOLOUR));
		{
			DBVARIANT dbv;
			if ( !DBGetContactSetting(NULL, TTB_OPTDIR, "BkBitmap", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_FILENAME, dbv.ptszVal);
				DBFreeVariant(&dbv);
			}

			WORD bmpUse = DBGetContactSettingWord(NULL, TTB_OPTDIR, "BkBmpUse", TTBDEFAULT_BKBMPUSE);
			CheckDlgButton(hwndDlg, IDC_STRETCHH, bmpUse&CLB_STRETCHH?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STRETCHV, bmpUse&CLB_STRETCHV?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEH, bmpUse&CLBF_TILEH?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEV, bmpUse&CLBF_TILEV?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SCROLL, bmpUse&CLBF_SCROLL?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, bmpUse&CLBF_PROPORTIONAL?BST_CHECKED:BST_UNCHECKED);

			HRESULT (STDAPICALLTYPE *MySHAutoComplete)(HWND, DWORD);
			MySHAutoComplete = (HRESULT (STDAPICALLTYPE*)(HWND, DWORD))GetProcAddress(GetModuleHandleA("shlwapi"), "SHAutoComplete");
			if (MySHAutoComplete)
				MySHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
		}
		return TRUE;

	case WM_USER+10:
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHH), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHV), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TILEH), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TILEV), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_PROPORTIONAL), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BROWSE) {
			TCHAR str[MAX_PATH];
			OPENFILENAME ofn = {0};
			TCHAR filter[512];

			GetDlgItemText(hwndDlg, IDC_FILENAME, str, sizeof(str));
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.hInstance = NULL;
			CallService(MS_UTILS_GETBITMAPFILTERSTRINGST, SIZEOF(filter), (LPARAM)filter);
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = sizeof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = _T("bmp");
			if (!GetOpenFileName(&ofn))
				break;

			SetDlgItemText(hwndDlg, IDC_FILENAME, str);
		}
		else if (LOWORD(wParam) == IDC_FILENAME && HIWORD(wParam) != EN_CHANGE)
			break;

		if (LOWORD(wParam) == IDC_BITMAP) SendMessage(hwndDlg, WM_USER+10, 0, 0);
		if (LOWORD(wParam) == IDC_FILENAME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				DBWriteContactSettingByte(NULL, TTB_OPTDIR, "UseBitmap", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
				{	
					COLORREF col = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);
					if (col == TTBDEFAULT_BKCOLOUR)
						DBDeleteContactSetting(NULL, TTB_OPTDIR, "BkColour");
					else
						DBWriteContactSettingDword(NULL, TTB_OPTDIR, "BkColour", col);
					col = SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_GETCOLOUR, 0, 0);
					if (col == TTBDEFAULT_SELBKCOLOUR)
						DBDeleteContactSetting(NULL, TTB_OPTDIR, "SelBkColour");
					else
						DBWriteContactSettingDword(NULL, TTB_OPTDIR, "SelBkColour", col);

					TCHAR str[MAX_PATH];
					GetDlgItemText(hwndDlg, IDC_FILENAME, str, SIZEOF(str));
					DBWriteContactSettingTString(NULL, TTB_OPTDIR, "BkBitmap", str);

					WORD flags = 0;
					if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHH)) flags |= CLB_STRETCHH;
					if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHV)) flags |= CLB_STRETCHV;
					if (IsDlgButtonChecked(hwndDlg, IDC_TILEH)) flags |= CLBF_TILEH;
					if (IsDlgButtonChecked(hwndDlg, IDC_TILEV)) flags |= CLBF_TILEV;
					if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL)) flags |= CLBF_SCROLL;
					if (IsDlgButtonChecked(hwndDlg, IDC_PROPORTIONAL)) flags |= CLBF_PROPORTIONAL;
					DBWriteContactSettingWord(NULL, TTB_OPTDIR, "BkBmpUse", flags);
				}
							
				ttbOptionsChanged();
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}
