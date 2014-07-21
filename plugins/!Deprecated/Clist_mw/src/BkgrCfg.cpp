
#include "commonheaders.h"

#define DEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define DEFAULT_USEBITMAP     0
#define DEFAULT_BKBMPUSE      CLB_STRETCH
#define DEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)


extern HINSTANCE g_hInst;

char **bkgrList = NULL;
int bkgrCount = 0;
HANDLE hEventBkgrChanged;

#define M_BKGR_UPDATE	(WM_USER+10)
#define M_BKGR_SETSTATE	(WM_USER+11)
#define M_BKGR_GETSTATE	(WM_USER+12)

#define M_BKGR_BACKCOLOR	0x01
#define M_BKGR_SELECTCOLOR	0x02
#define M_BKGR_ALLOWBITMAPS	0x04
#define M_BKGR_STRETCH		0x08
#define M_BKGR_TILE			0x10

#define ARRAY_SIZE(arr)	(sizeof(arr)/sizeof(arr[0]))
static const int bitmapRelatedControls[] = {
	IDC_FILENAME,IDC_BROWSE,IDC_STRETCHH,IDC_STRETCHV,IDC_TILEH,IDC_TILEV,
	IDC_SCROLL,IDC_PROPORTIONAL,IDC_TILEVROWH
};
struct BkgrItem
{
	BYTE changed;
	BYTE useBitmap;
	COLORREF bkColor, selColor;
	char filename[MAX_PATH];
	WORD flags;
};
struct BkgrData
{
	struct BkgrItem *item;
	int indx;
	int count;
};
static INT_PTR CALLBACK DlgProcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct BkgrData *dat = (struct BkgrData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			int indx;
			HWND hList = GetDlgItem(hwndDlg, IDC_BKGRLIST);
			TranslateDialogDefault(hwndDlg);

			dat = (struct BkgrData*)mir_alloc(sizeof(struct BkgrData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->count = bkgrCount;
			dat->item = (struct BkgrItem*)mir_alloc(sizeof(struct BkgrItem)*dat->count);
			dat->indx = CB_ERR;
			for (indx = 0; indx < dat->count; indx++)
			{
				char *module = bkgrList[indx] + strlen(bkgrList[indx]) + 1;
				int jndx;

				dat->item[indx].changed = FALSE;
				dat->item[indx].useBitmap = db_get_b(NULL,module, "UseBitmap", DEFAULT_USEBITMAP);
				dat->item[indx].bkColor = db_get_dw(NULL,module, "BkColour", DEFAULT_BKCOLOUR);
				dat->item[indx].selColor = db_get_dw(NULL,module, "SelBkColour", DEFAULT_SELBKCOLOUR);
				{	
					DBVARIANT dbv;
					if ( !db_get_s(NULL,module,"BkBitmap",&dbv))
					{
						int retval = PathToAbsolute(dbv.pszVal, dat->item[indx].filename);
						if ( !retval || retval == CALLSERVICE_NOTFOUND)
							lstrcpynA(dat->item[indx].filename, dbv.pszVal, MAX_PATH);
						mir_free(dbv.pszVal);
					}
					else
						*dat->item[indx].filename = 0;
				}
				dat->item[indx].flags = db_get_w(NULL,module,"BkBmpUse", DEFAULT_BKBMPUSE);
				jndx = SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)Translate(bkgrList[indx]));
				SendMessage(hList, CB_SETITEMDATA, jndx, indx);
			}
			SendMessage(hList, CB_SETCURSEL, 0, 0);
			PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_BKGRLIST, CBN_SELCHANGE), 0);
			SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
			return TRUE;
		}
		case WM_DESTROY:
			if (dat)
			{
				if (dat->item) mir_free(dat->item);
				mir_free(dat);
			}
		
			return TRUE;

		case M_BKGR_GETSTATE:
		{
			int indx = wParam;
			if (indx == CB_ERR || indx >= dat->count) break;
			indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);

			dat->item[indx].useBitmap = IsDlgButtonChecked(hwndDlg,IDC_BITMAP);
			dat->item[indx].bkColor = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR,0,0);
			dat->item[indx].selColor = SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_GETCOLOUR,0,0);
			GetDlgItemTextA(hwndDlg, IDC_FILENAME, dat->item[indx].filename, sizeof(dat->item[indx].filename));
			{
				WORD flags = 0;
				if (IsDlgButtonChecked(hwndDlg,IDC_STRETCHH)) flags |= CLB_STRETCHH;
				if (IsDlgButtonChecked(hwndDlg,IDC_STRETCHV)) flags |= CLB_STRETCHV;
				if (IsDlgButtonChecked(hwndDlg,IDC_TILEH)) flags |= CLBF_TILEH;
				if (IsDlgButtonChecked(hwndDlg,IDC_TILEV)) flags |= CLBF_TILEV;
				if (IsDlgButtonChecked(hwndDlg,IDC_SCROLL)) flags |= CLBF_SCROLL;
				if (IsDlgButtonChecked(hwndDlg,IDC_PROPORTIONAL)) flags |= CLBF_PROPORTIONAL;
				if (IsDlgButtonChecked(hwndDlg,IDC_TILEVROWH)) flags |= CLBF_TILEVTOROWHEIGHT;
				dat->item[indx].flags = flags;
			}	
			break;
		}
		case M_BKGR_SETSTATE:
		{
			int indx = wParam;
			int flags = dat->item[indx].flags;
			if (indx == CB_ERR || indx >= dat->count) break;
			indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);

			CheckDlgButton(hwndDlg, IDC_BITMAP, dat->item[indx].useBitmap?BST_CHECKED:BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, DEFAULT_BKCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, dat->item[indx].bkColor);
			SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETDEFAULTCOLOUR, 0, DEFAULT_SELBKCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETCOLOUR, 0, dat->item[indx].selColor);
			SetDlgItemTextA(hwndDlg, IDC_FILENAME, dat->item[indx].filename);	

			CheckDlgButton(hwndDlg,IDC_STRETCHH, flags&CLB_STRETCHH?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_STRETCHV,flags&CLB_STRETCHV?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TILEH,flags&CLBF_TILEH?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TILEV,flags&CLBF_TILEV?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_SCROLL,flags&CLBF_SCROLL?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_PROPORTIONAL,flags&CLBF_PROPORTIONAL?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TILEVROWH,flags&CLBF_TILEVTOROWHEIGHT?BST_CHECKED:BST_UNCHECKED);

			SendMessage(hwndDlg, M_BKGR_UPDATE, 0,0);
			break;
		}
		case M_BKGR_UPDATE:
		{
			int isChecked = IsDlgButtonChecked(hwndDlg,IDC_BITMAP);
			int indx;
			for (indx = 0; indx < ARRAY_SIZE(bitmapRelatedControls); indx++)
				EnableWindow(GetDlgItem(hwndDlg, bitmapRelatedControls[indx]),isChecked);
			break;
		}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_BROWSE) {
				char str[MAX_PATH];
				OPENFILENAMEA ofn = {0};
				char filter[512];

				GetDlgItemTextA(hwndDlg,IDC_FILENAME, str, sizeof(str));
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = NULL;
				CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof(filter), (LPARAM)filter);
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = str;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.nMaxFile = sizeof(str);
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = "bmp";
				if ( !GetOpenFileNameA(&ofn)) break;
				SetDlgItemTextA(hwndDlg, IDC_FILENAME, str);
			}
			else if (LOWORD(wParam) == IDC_FILENAME && HIWORD(wParam) != EN_CHANGE) 
				break;

			if (LOWORD(wParam) == IDC_BITMAP)
				SendMessage(hwndDlg, M_BKGR_UPDATE, 0,0);
			if (LOWORD(wParam) == IDC_FILENAME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			if (LOWORD(wParam) == IDC_BKGRLIST) {
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					SendMessage(hwndDlg, M_BKGR_GETSTATE, dat->indx, 0);
					SendMessage(hwndDlg, M_BKGR_SETSTATE, dat->indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0,0), 0);
				}
				return 0;
			}
			{
				int indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0,0);
				if (indx != CB_ERR && indx < dat->count) {
					indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);					
					dat->item[indx].changed = TRUE;
				
				}							
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0,0);
			}
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							int indx;
							SendMessage(hwndDlg, M_BKGR_GETSTATE, SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0,0), 0);
							for (indx = 0; indx < dat->count; indx++)
							if (dat->item[indx].changed)
							{
								char *module = bkgrList[indx] + strlen(bkgrList[indx]) + 1;
								db_set_b(NULL, module, "UseBitmap", (BYTE)dat->item[indx].useBitmap);
								{	
									COLORREF col;

									if ((col = dat->item[indx].bkColor) == DEFAULT_BKCOLOUR)
										db_unset(NULL, module, "BkColour");
									else
										db_set_dw(NULL, module, "BkColour", col);

									if ((col = dat->item[indx].selColor) == DEFAULT_SELBKCOLOUR)
										db_unset(NULL, module, "SelBkColour");
									else
										db_set_dw(NULL, module, "SelBkColour", col);
								}
								{
									char str[MAX_PATH];
									int retval = PathToAbsolute(dat->item[indx].filename, str);
									if ( !retval || retval == CALLSERVICE_NOTFOUND)
										db_set_s(NULL, module, "BkBitmap", dat->item[indx].filename);
									else
										db_set_s(NULL, module, "BkBitmap", str);
								}
								db_set_w(NULL, module, "BkBmpUse", dat->item[indx].flags);
								dat->item[indx].changed = FALSE;
								NotifyEventHooks(hEventBkgrChanged, (WPARAM)module, 0);
							}
							return TRUE;
						}
					}
					break;
			}
			break;
	}
	return FALSE;
}

static INT_PTR BkgrCfg_Register(WPARAM wParam, LPARAM lParam)
{
	char *szSetting = (char *)wParam;
	char *value, *tok;
	size_t len = strlen(szSetting) + 1;

	value = (char *)mir_alloc(len + 4); // add room for flags (DWORD)
	memcpy(value, szSetting, len);
	tok = strchr(value, '/');
	if (tok == NULL) {
		mir_free(value);
		return 1;
	}
	*tok = 0;
	*(DWORD*)(value + len) = lParam;

	bkgrList = (char **)mir_realloc(bkgrList, sizeof(char*)*(bkgrCount+1));
	bkgrList[bkgrCount] = value;
	bkgrCount++;
	
	return 0;
}

int OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInst;
	odp.pszGroup = LPGEN("Customize");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLCBKG2);
	odp.pszTitle = LPGEN("Backgrounds");
	odp.pfnDlgProc = DlgProcBkgOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

int BGModuleLoad()
{	
	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);

	CreateServiceFunction(MS_BACKGROUNDCONFIG_REGISTER, BkgrCfg_Register);

	hEventBkgrChanged = CreateHookableEvent(ME_BACKGROUNDCONFIG_CHANGED);
	return 0;
}

int BGModuleUnload(void)
{
	if (bkgrList != NULL)
	{
		int indx;
		for (indx = 0; indx < bkgrCount; indx++)
			if (bkgrList[indx] != NULL)
				mir_free(bkgrList[indx]);
		mir_free(bkgrList);
	}
	DestroyHookableEvent(hEventBkgrChanged);

	return 0;
}
