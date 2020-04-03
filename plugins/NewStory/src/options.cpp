#include "stdafx.h"

Options options;

// Option dialog

static INT_PTR CALLBACK OptTemplatesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int CurrentTemplate;
	switch (msg) {
	case WM_INITDIALOG:
		CurrentTemplate = -1;
		{
			HTREEITEM hGroup = 0;
			HTREEITEM hFirst = 0;

			HIMAGELIST himgTree = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
			TreeView_SetImageList(GetDlgItem(hwnd, IDC_TEMPLATES), himgTree, TVSIL_NORMAL);

			ImageList_AddIcon(himgTree, g_plugin.getIcon(ICO_TPLGROUP));

			for (int i = 0; i < TPL_COUNT; i++) {
				if (!i || lstrcmp(templates[i].group, templates[i - 1].group)) {
					if (hGroup)
						TreeView_Expand(GetDlgItem(hwnd, IDC_TEMPLATES), hGroup, TVE_EXPAND);

					TVINSERTSTRUCT tvis;
					tvis.hParent = 0;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvis.item.state = tvis.item.stateMask = TVIS_BOLD;
					tvis.item.iSelectedImage = tvis.item.iImage = 0;
					tvis.item.pszText = templates[i].group;
					tvis.item.lParam = -1;
					hGroup = TreeView_InsertItem(GetDlgItem(hwnd, IDC_TEMPLATES), &tvis);

					if (!hFirst) hFirst = hGroup;
				}

				TVINSERTSTRUCT tvis;
				tvis.hParent = hGroup;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvis.item.pszText = templates[i].title;
				tvis.item.iSelectedImage = tvis.item.iImage =
					ImageList_AddIcon(himgTree, g_plugin.getIcon(templates[i].icon));
				tvis.item.lParam = i;
				TreeView_InsertItem(GetDlgItem(hwnd, IDC_TEMPLATES), &tvis);
			}

			if (hGroup)
				TreeView_Expand(GetDlgItem(hwnd, IDC_TEMPLATES), hGroup, TVE_EXPAND);

			TreeView_SelectItem(GetDlgItem(hwnd, IDC_TEMPLATES), hFirst);
			TreeView_EnsureVisible(GetDlgItem(hwnd, IDC_TEMPLATES), hFirst);
		}
		SendMessage(GetDlgItem(hwnd, IDC_DISCARD), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwnd, IDC_VARHELP), BUTTONSETASFLATBTN, 0, 0);

		SendMessage(GetDlgItem(hwnd, IDC_DISCARD), BUTTONADDTOOLTIP, (WPARAM)Translate("Cancel Edit"), 0);
		SendMessage(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), BUTTONADDTOOLTIP, (WPARAM)Translate("Update Preview"), 0);
		SendMessage(GetDlgItem(hwnd, IDC_VARHELP), BUTTONADDTOOLTIP, (WPARAM)Translate("Help on Variables"), 0);

		SendMessage(GetDlgItem(hwnd, IDC_DISCARD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_RESET));
		SendMessage(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_PREVIEW));
		SendMessage(GetDlgItem(hwnd, IDC_VARHELP), BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(ICO_VARHELP));

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDITTEMPLATE:
			SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);

		case IDC_UPDATEPREVIEW:
			if (templates[CurrentTemplate].tmpValue)
				mir_free(templates[CurrentTemplate].tmpValue);
			{
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDITTEMPLATE)) + 1;
				templates[CurrentTemplate].tmpValue = (wchar_t*)mir_alloc(length * sizeof(wchar_t));
				GetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].tmpValue, length);

				HistoryArray::ItemData item;
				item.hContact = db_find_first();
				while (item.hContact && !item.hEvent) {
					item.hEvent = db_event_first(item.hContact);
					if (!item.hEvent)
						item.hContact = db_find_next(item.hContact);
				}

				if (item.hContact && item.hEvent) {
					item.load(ELM_DATA);
					wchar_t* preview = TplFormatStringEx(CurrentTemplate, templates[CurrentTemplate].tmpValue, item.hContact, &item);
					SetWindowText(GetDlgItem(hwnd, IDC_PREVIEW), preview);
					//						SetWindowText(GetDlgItem(hwnd, IDC_GPREVIEW), preview);
					SetWindowText(GetDlgItem(hwnd, IDC_GPREVIEW), L"$hit :)");
					mir_free(preview);
				}
				else {
					SetWindowText(GetDlgItem(hwnd, IDC_PREVIEW), L"");
					SetWindowText(GetDlgItem(hwnd, IDC_GPREVIEW), L"");
				}
			}
			break;

		case IDC_DISCARD:
			if (templates[CurrentTemplate].tmpValue)
				mir_free(templates[CurrentTemplate].tmpValue);
			templates[CurrentTemplate].tmpValue = 0;
			if (templates[CurrentTemplate].value)
				SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].value);
			else
				SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].defvalue);

			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_UPDATEPREVIEW, 0), 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				for (int i = 0; i < TPL_COUNT; i++) {
					if (templates[i].tmpValue) {
						if (templates[i].tmpValue)
							mir_free(templates[i].tmpValue);
					}
				}
				return TRUE;

			case PSN_APPLY:
				for (int i = 0; i < TPL_COUNT; i++) {
					if (templates[i].tmpValue) {
						if (templates[i].value)
							mir_free(templates[i].value);
						templates[i].value = templates[i].tmpValue;
						templates[i].tmpValue = 0;
					}
				}
				SaveTemplates();
				return TRUE;
			}
			break;

		case IDC_TEMPLATES:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_SELCHANGED:
			case TVN_SELCHANGING:
			{
				LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

				TVITEM tvi;
				tvi.hItem = TreeView_GetSelection(GetDlgItem(hwnd, IDC_TEMPLATES));
				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwnd, IDC_TEMPLATES), &tvi);

				if ((tvi.lParam < 0) || (tvi.lParam >= TPL_COUNT)) {
					EnableWindow(GetDlgItem(hwnd, IDC_EDITTEMPLATE), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_GPREVIEW), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_PREVIEW), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_DISCARD), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_VARHELP), FALSE);
					/*								HTREEITEM hItem = TreeView_GetChild(GetDlgItem(hwnd, IDC_TEMPLATES), tvi.hItem);
													if (hItem)
													{
														TreeView_Expand(GetDlgItem(hwnd, IDC_TEMPLATES), tvi.hItem, TVE_EXPAND);
														TreeView_SelectItem(GetDlgItem(hwnd, IDC_TEMPLATES), hItem);
													}*/
					break;
				}
				else {
					EnableWindow(GetDlgItem(hwnd, IDC_EDITTEMPLATE), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_GPREVIEW), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_PREVIEW), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_DISCARD), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_UPDATEPREVIEW), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_VARHELP), TRUE);
				}

				if ((lpnmtv->itemOld.mask & TVIF_HANDLE) && lpnmtv->itemOld.hItem && (lpnmtv->itemOld.hItem != lpnmtv->itemNew.hItem) && (lpnmtv->itemOld.lParam >= 0) && (lpnmtv->itemOld.lParam < TPL_COUNT)) {
					if (templates[lpnmtv->itemOld.lParam].tmpValue)
						mir_free(templates[lpnmtv->itemOld.lParam].tmpValue);
					int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDITTEMPLATE)) + 1;
					templates[lpnmtv->itemOld.lParam].tmpValue = (wchar_t*)mir_alloc(length * sizeof(wchar_t));
					GetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[lpnmtv->itemOld.lParam].tmpValue, length);
				}

				CurrentTemplate = tvi.lParam;

				if (templates[CurrentTemplate].tmpValue)
					SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].tmpValue);
				else if (templates[CurrentTemplate].value)
					SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].value);
				else
					SetWindowText(GetDlgItem(hwnd, IDC_EDITTEMPLATE), templates[CurrentTemplate].defvalue);

				PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_UPDATEPREVIEW, 0), 0);
			}
			break;

			case TVN_KEYDOWN:
			case NM_CLICK:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;
			}
			break;
		}
		break;
	}
	return FALSE;
}

static int OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.szGroup.a = LPGEN("History");
	odp.szTitle.a = LPGEN("Newstory");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Templates");
	odp.pfnDlgProc = OptTemplatesDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TEMPLATES);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
}
