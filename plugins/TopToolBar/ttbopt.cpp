#include "common.h"
#include <shlobj.h>
#pragma hdrstop

HWND OptionshWnd = 0;

struct OrderData
{
	int dragging;
	HTREEITEM hDragItem;
	HIMAGELIST himlButtonIcons;
};

HTREEITEM AddLine(HWND hTree,TopButtonInt *b, HTREEITEM hItem, HIMAGELIST il)
{
	TVINSERTSTRUCT tvis = { 0 };
	tvis.hInsertAfter = hItem;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	int index;
	TCHAR* tmp;

	if (b->dwFlags & TTBBF_ISSEPARATOR) {
		tvis.item.pszText = L"------------------";
		index = -1;
	}
	else {
		if (b->dwFlags & TTBBF_ICONBYHANDLE) {
			HICON hIcon = Skin_GetIconByHandle(b->hIconHandleUp);
			index = ImageList_AddIcon(il, hIcon);
			Skin_ReleaseIcon(hIcon);
		}
		else index = ImageList_AddIcon(il, b->hIconUp);

		tmp = mir_a2t( b->name );
		tvis.item.pszText = TranslateTS(tmp);
	}
	tvis.item.iImage = tvis.item.iSelectedImage = index;

	tvis.item.lParam = (LPARAM)b;
	HTREEITEM hti = TreeView_InsertItem(hTree, &tvis);

	if (!(b->dwFlags & TTBBF_ISSEPARATOR))
		mir_free(tmp);

	TreeView_SetCheckState(hTree, hti, (b->dwFlags & TTBBF_VISIBLE) ? TRUE : FALSE);

	return hti;
}

int BuildTree(HWND hwndDlg)
{
	HWND hTree = GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE);
	OrderData *dat = (struct OrderData*)GetWindowLongPtr(hTree, GWLP_USERDATA);

	dat->himlButtonIcons = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
	TreeView_SetImageList(hTree, dat->himlButtonIcons, TVSIL_NORMAL);
	SetWindowLongPtr(hTree, GWL_STYLE, GetWindowLongPtr(hTree,GWL_STYLE)|TVS_NOHSCROLL);
	TreeView_DeleteAllItems(hTree);

	if (Buttons.getCount() == 0)
		return FALSE;

	for (int i = 0; i < Buttons.getCount(); i++) {
		TopButtonInt *b = Buttons[i];
		AddLine(hTree, b, TVI_LAST, dat->himlButtonIcons);
	}

	return (TRUE);
}

//call this when options opened and buttons added/removed
int OptionsPageRebuild()
{
	if (OptionshWnd)
		BuildTree(OptionshWnd);

	return 0;
}

int SaveTree(HWND hwndDlg)
{
	HWND hTree = GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE);

	TVITEM tvi = { 0 };
	tvi.hItem = TreeView_GetRoot(hTree);

	LIST<TopButtonInt> tmpList(8);

	while(tvi.hItem != NULL) {
		tvi.stateMask = TVIS_STATEIMAGEMASK;
		tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
		TreeView_GetItem(hTree, &tvi);

		TopButtonInt* btn = (TopButtonInt*)tvi.lParam;
		Buttons.remove(btn);

		if (TreeView_GetCheckState(hTree,tvi.hItem))
			btn->dwFlags |= TTBBF_VISIBLE;
		else
			btn->dwFlags &= ~TTBBF_VISIBLE;
		btn->dwFlags &= ~TTBBF_OPTIONAL;
		btn->arrangedpos = tmpList.getCount();

		tmpList.insert(btn);
		tvi.hItem = TreeView_GetNextSibling(hTree, tvi.hItem);
	}

	lockbut();
	for (int i=0; i < Buttons.getCount(); i++)
		delete Buttons[i];

	Buttons = tmpList;
	tmpList.destroy();	

	ulockbut();
	SaveAllButtonsOptions();
	return (TRUE);
}

int CancelProcess(HWND hwndDlg)
{
	HWND hTree = GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE);

	TVITEM tvi = { 0 };
	tvi.hItem = TreeView_GetRoot(hTree);

	while(tvi.hItem != NULL) {
		tvi.stateMask = TVIS_STATEIMAGEMASK;
		tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
		TreeView_GetItem(hTree, &tvi);

		TopButtonInt* btn = (TopButtonInt*)tvi.lParam;

		if (btn ->dwFlags & TTBBF_OPTIONAL)
			delete btn;

		tvi.hItem = TreeView_GetNextSibling(hTree, tvi.hItem);
	}

	return (TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options window: main

static INT_PTR CALLBACK ButOrderOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hTree = GetDlgItem(hwndDlg, IDC_BUTTONORDERTREE);
	struct OrderData *dat = (struct OrderData*)GetWindowLongPtr(hTree, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		dat = (struct OrderData*)malloc(sizeof(struct OrderData));
		SetWindowLongPtr(hTree, GWLP_USERDATA, (LONG)dat);
		dat->dragging = 0;

		SetWindowLongPtr(hTree, GWL_STYLE, GetWindowLongPtr(hTree, GWL_STYLE)|TVS_NOHSCROLL);

		SetDlgItemInt(hwndDlg, IDC_BUTTHEIGHT, BUTTHEIGHT, FALSE);
		SetDlgItemInt(hwndDlg, IDC_BUTTWIDTH, BUTTWIDTH, FALSE);
		SetDlgItemInt(hwndDlg, IDC_BUTTGAP, BUTTGAP, FALSE);
		CheckDlgButton(hwndDlg, IDC_USEFLAT, DBGetContactSettingByte(0, TTB_OPTDIR, "UseFlatButton", 1));

		BuildTree(hwndDlg);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ENAME), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EPATH), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELLBUTTON), FALSE);

		OptionshWnd = hwndDlg;
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE && OptionshWnd) {
			switch(LOWORD(wParam)) {
			case IDC_ENAME: case IDC_EPATH:
				break;
			default:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		}

		if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == BN_DBLCLK)) {
			int ctrlid = LOWORD(wParam);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			//----- Launch buttons -----

			if (ctrlid == IDC_LBUTTONSET) {
				TVITEM tvi ={0};
				tvi.hItem = TreeView_GetSelection(hTree);
				if (tvi.hItem == NULL)
					break;

				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(hTree, &tvi);

				TopButtonInt* btn = (TopButtonInt*)tvi.lParam;
				TCHAR buf [256];
				// probably, condition not needs
				if (btn->dwFlags & TTBBF_ISLBUTTON) {
					if (!(btn->dwFlags & TTBBF_OPTIONAL)) {
						// create button
						TTBButton ttb = { 0 };
						ttb.cbSize = sizeof(ttb);
						ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_RUN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
						ttb.dwFlags = TTBBF_VISIBLE | TTBBF_ISLBUTTON | TTBBF_INTERNAL | TTBBF_OPTIONAL;
						ttb.name = NULL;
						ttb.program = NULL;
						btn = CreateButton(&ttb);

						tvi.lParam = (LPARAM)btn;
						TreeView_SetItem(hTree, &tvi);
					}

					if (btn->name) free(btn->name);
					GetDlgItemText(hwndDlg, IDC_ENAME, buf, 255);
					btn->name = _strdup( _T2A(buf));

					if (btn->program) free(btn->program);
					GetDlgItemText(hwndDlg, IDC_EPATH, buf, 255);
					btn->program = _tcsdup(buf);

					tvi.mask = TVIF_TEXT;
					tvi.pszText = mir_a2t( btn->name );
					TreeView_SetItem(hTree, &tvi);
				}
				break;
			}

			if (ctrlid == IDC_ADDLBUTTON) {
				// create button
				TTBButton ttb = { 0 };
				ttb.cbSize = sizeof(ttb);
				ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_RUN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
				ttb.dwFlags = TTBBF_VISIBLE | TTBBF_ISLBUTTON | TTBBF_INTERNAL | TTBBF_OPTIONAL;
				ttb.name = LPGEN("Default");
				ttb.program = _T("Execute Path");
				TopButtonInt* b = CreateButton(&ttb);

				// get selection for insert
				TVITEM tvi = {0};
				tvi.hItem = TreeView_GetSelection(hTree);

				// insert item
				AddLine(hTree, b, tvi.hItem, dat->himlButtonIcons);

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}

			//----- Separators -----

			if (ctrlid == IDC_ADDSEP) {
				// create button
				TTBButton ttb = { 0 };
				ttb.cbSize = sizeof(ttb);
				ttb.dwFlags = TTBBF_VISIBLE | TTBBF_ISSEPARATOR | TTBBF_INTERNAL | TTBBF_OPTIONAL;
				TopButtonInt* b = CreateButton(&ttb);

				// get selection for insert
				TVITEM tvi = {0};
				tvi.hItem = TreeView_GetSelection(hTree);

				// insert item
				AddLine(hTree, b, tvi.hItem, dat->himlButtonIcons);

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}

			if (ctrlid == IDC_REMOVEBUTTON) {
				TVITEM tvi = {0};
				tvi.hItem = TreeView_GetSelection(hTree);
				if (tvi.hItem == NULL)
					break;

				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(hTree, &tvi);

				TopButtonInt* btn = (TopButtonInt*)tvi.lParam;
				// if button enabled for separator and launch only, no need condition
				// except possible service button introducing
				if (btn->dwFlags & (TTBBF_ISSEPARATOR | TTBBF_ISLBUTTON)) {
					// delete if was added in options
					if (btn->dwFlags & TTBBF_OPTIONAL)
						delete btn;

					TreeView_DeleteItem(hTree,tvi.hItem);

					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				BUTTHEIGHT = GetDlgItemInt(hwndDlg, IDC_BUTTHEIGHT, NULL, FALSE);
				BUTTWIDTH = GetDlgItemInt(hwndDlg, IDC_BUTTWIDTH, NULL, FALSE);
				BUTTGAP = GetDlgItemInt(hwndDlg, IDC_BUTTGAP, NULL, FALSE);
				DBWriteContactSettingByte(0, TTB_OPTDIR, "BUTTHEIGHT", BUTTHEIGHT);
				DBWriteContactSettingByte(0, TTB_OPTDIR, "BUTTWIDTH", BUTTWIDTH);
				DBWriteContactSettingByte(0, TTB_OPTDIR, "BUTTGAP", BUTTGAP);
				DBWriteContactSettingByte(0, TTB_OPTDIR, "UseFlatButton", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USEFLAT));

				SaveTree(hwndDlg);
				RecreateWindows();
				ArrangeButtons();
			}
			break;

		case IDC_BUTTONORDERTREE:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_BEGINDRAGA:
			case TVN_BEGINDRAGW:
				SetCapture(hwndDlg);
				dat->dragging = 1;
				dat->hDragItem = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
				TreeView_SelectItem(hTree, dat->hDragItem);
				break;

			case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x = (short)LOWORD(GetMessagePos());
					hti.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
					if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
						if (hti.flags & TVHT_ONITEMSTATEICON) {
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							TreeView_SelectItem(hTree, hti.hItem);
						}
				}
				break;

			case TVN_SELCHANGEDA:
			case TVN_SELCHANGEDW:
				{
					HTREEITEM hti = TreeView_GetSelection(hTree);
					if (hti == NULL)
						break;

					TopButtonInt *btn = (TopButtonInt*)((LPNMTREEVIEW)lParam)->itemNew.lParam;
					lockbut();

					if (btn->dwFlags & TTBBF_ISLBUTTON) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_ENAME), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_EPATH), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEBUTTON), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_LBUTTONSET), TRUE);
						if (btn->name != NULL)
							SetDlgItemTextA(hwndDlg, IDC_ENAME, btn->name);
						else
							SetDlgItemTextA(hwndDlg, IDC_ENAME, "");

						if (btn->program != NULL)
							SetDlgItemText(hwndDlg, IDC_EPATH, btn->program);
						else
							SetDlgItemTextA(hwndDlg, IDC_EPATH, "");
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVEBUTTON),
								(btn->dwFlags & TTBBF_ISSEPARATOR)?TRUE:FALSE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_ENAME), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_EPATH), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_LBUTTONSET), FALSE);
						SetDlgItemTextA(hwndDlg, IDC_ENAME, "");
						SetDlgItemTextA(hwndDlg, IDC_EPATH, "");
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
			ScreenToClient(hTree, &hti.pt);
			TreeView_HitTest(hTree, &hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				hti.pt.y -= TreeView_GetItemHeight(hTree)/2;
				TreeView_HitTest(hTree, &hti);
				TreeView_SetInsertMark(hTree, hti.hItem, 1);
			}
			else {
				if (hti.flags & TVHT_ABOVE) SendMessage(hTree, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW) SendMessage(hTree, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				TreeView_SetInsertMark(hTree, NULL, 0);
			}
		}
		break;

	case WM_DESTROY:
		if (dat) {
			ImageList_Destroy(dat->himlButtonIcons);
			free(dat);
		}
		OptionshWnd = NULL;
		return 0;

	case WM_LBUTTONUP:
		if (dat->dragging) {
			TreeView_SetInsertMark(hTree, NULL, 0);
			dat->dragging = 0;
			ReleaseCapture();

			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(hTree, &hti.pt);
			hti.pt.y -= TreeView_GetItemHeight(hTree)/2;
			TreeView_HitTest(hTree, &hti);
			if (dat->hDragItem == hti.hItem)
				break;

			TVITEM tvi;
			tvi.mask = TVIF_HANDLE|TVIF_PARAM;
			tvi.hItem = hti.hItem;
			TreeView_GetItem(hTree, &tvi);
			if (hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
				TVINSERTSTRUCT tvis;
				TCHAR name[128];
				tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
				tvis.item.stateMask = 0xFFFFFFFF;
				tvis.item.pszText = name;
				tvis.item.cchTextMax = SIZEOF(name);
				tvis.item.hItem = dat->hDragItem;
				TreeView_GetItem(hTree, &tvis.item);

				TreeView_DeleteItem(hTree, dat->hDragItem);
				tvis.hParent = NULL;
				tvis.hInsertAfter = hti.hItem;
				TreeView_SelectItem(hTree, TreeView_InsertItem(hTree, &tvis));

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options window: background

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

/////////////////////////////////////////////////////////////////////////////////////////

int TTBOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszGroup = LPGEN("TopToolBar");

	if ( !ServiceExists(MS_BACKGROUNDCONFIG_REGISTER)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TTBBKG);
		odp.pszTitle = LPGEN("TTBBackground");
		odp.pfnDlgProc = DlgProcTTBBkgOpts;
		odp.flags = ODPF_BOLDGROUPS;
		Options_AddPage(wParam, &odp);
	}

	odp.position = -1000000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_BUTORDER);
	odp.pszGroup = LPGEN("TopToolBar");
	odp.pszTitle = LPGEN("Buttons");
	odp.pfnDlgProc = ButOrderOpts;
	odp.flags = ODPF_BOLDGROUPS|ODPF_EXPERTONLY;
	Options_AddPage(wParam, &odp);
	return 0;
}
