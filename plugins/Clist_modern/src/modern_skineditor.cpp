/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_sync.h"

typedef struct _OPT_OBJECT_DATA
{
	char  *szName;
	TCHAR *szPath;
	TCHAR *szValue;
	TCHAR *szTempValue;
} OPT_OBJECT_DATA;

static char *gl_Mask = NULL;
HWND gl_Dlg = NULL;
int  gl_controlID = 0;
int EnableGroup(HWND hwndDlg, HWND first, BOOL bEnable);
int ShowGroup(HWND hwndDlg, HWND first, BOOL bEnable);
BOOL glOtherSkinWasLoaded = FALSE;
BYTE glSkinWasModified = 0;  //1- but not applied, 2-stored to DB

HTREEITEM FindChild(HWND hTree, HTREEITEM Parent, TCHAR *Caption)
{
  HTREEITEM res = NULL, tmp = NULL;
  if (Parent)
    tmp = TreeView_GetChild(hTree,Parent);
  else
	tmp = TreeView_GetRoot(hTree);
  while (tmp)
  {
    TVITEM tvi;
    TCHAR buf[255];
    tvi.hItem = tmp;
    tvi.mask = TVIF_TEXT|TVIF_HANDLE;
    tvi.pszText = (LPTSTR)&buf;
    tvi.cchTextMax = 254;
    TreeView_GetItem(hTree,&tvi);
    if (lstrcmpi(Caption,tvi.pszText))
      return tmp;
    tmp = TreeView_GetNextSibling(hTree,tmp);
  }
  return tmp;
}

int TreeAddObject(HWND hwndDlg, int ID, OPT_OBJECT_DATA * data)
{
	HTREEITEM rootItem = NULL;
	HTREEITEM cItem = NULL;
	TCHAR *ptr, *ptrE;
	BOOL ext = FALSE;
	TCHAR *path = data->szPath ? mir_tstrdup(data->szPath):(data->szName[1] == '$') ? mir_a2t(data->szName+2):NULL;
	if (!path) {
		TCHAR buf[255];
		mir_sntprintf(buf, SIZEOF(buf), _T("$(other)/%S"), data->szName+1);
		path = mir_tstrdup(buf);
	}

	ptr = path;
	ptrE = path;
	do
	{
		while (*ptrE != '/' && *ptrE != '\0') ptrE++;
		if (*ptrE == '/')
		{
			*ptrE = '\0';
			ptrE++;
			// find item if not - create;
			{
				cItem = FindChild(GetDlgItem(hwndDlg,ID), rootItem, ptr);
				if (!cItem) // not found - create node
				{
					TVINSERTSTRUCT tvis;
					tvis.hParent = rootItem;
					tvis.hInsertAfter = TVI_SORT;
					tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
					tvis.item.pszText = ptr;
					tvis.item.lParam = 0;
					cItem = TreeView_InsertItem(GetDlgItem(hwndDlg, ID), &tvis);

				}
				rootItem = cItem;
			}
			ptr = ptrE;
		}
		else ext = TRUE;
	}while (!ext);

	//Insert item node
	{
		TVINSERTSTRUCT tvis;
		tvis.hParent = rootItem;
		tvis.hInsertAfter = TVI_SORT;
		tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
		tvis.item.pszText = ptr;
		tvis.item.lParam = (LPARAM)data;
		TreeView_InsertItem(GetDlgItem(hwndDlg, ID), &tvis);
	}
	mir_free(path);
	return 0;
}

int enumDB_SkinObjectsForEditorProc(const char *szSetting,LPARAM lParam)
{
	if (wildcmp(szSetting,gl_Mask) || wildcmp(szSetting,"$*"))
	{
		char *descKey = mir_strdup(szSetting);
		descKey[0] = '%';
		TCHAR *value = db_get_tsa(NULL,SKIN,szSetting);
		TCHAR *desc = db_get_tsa(NULL,SKIN,descKey);
		if ( wildcmpt(value, _T("?lyph*")))
		{
			OPT_OBJECT_DATA * a = (OPT_OBJECT_DATA*)mir_alloc(sizeof(OPT_OBJECT_DATA));
			a->szPath = desc;
			a->szName = mir_strdup(szSetting);
			a->szValue = value;
			a->szTempValue = NULL;
			TreeAddObject(gl_Dlg,gl_controlID,a);
		}
		else {
			mir_free(value);
			mir_free(desc);
		}
		mir_free(descKey);
	}
	return 0;
}

int FillObjectTree(HWND hwndDlg, int ObjectTreeID, char * wildmask)
{
	DBCONTACTENUMSETTINGS dbces;
	gl_Dlg = hwndDlg;
	gl_controlID = ObjectTreeID;
	gl_Mask = wildmask;
	dbces.pfnEnumProc = enumDB_SkinObjectsForEditorProc;
	dbces.szModule = SKIN;
	dbces.ofsSettings = 0;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);
	return 0;
}
TCHAR *TYPES[] = {LPGENT("- Empty - (do not draw this object)"),LPGENT("Solid fill object"),LPGENT("Image (draw image)"),LPGENT("Fragment (draw portion of image)")};
TCHAR *FITMODES[] = {LPGENT("Stretch both directions"),LPGENT("Stretch vertical, tile horizontal"),LPGENT("Tile vertical, stretch horizontal"),LPGENT("Tile both directions")};

void SetAppropriateGroups(HWND hwndDlg, int Type)
//str contains default values
{
	switch (Type)
	{
	case -1:
		{
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_1),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_COLOUR),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_ALPHA),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_2),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_3),FALSE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_S_SIZE),FALSE);
		}
		break;
	case 0:
		{
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_1),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_COLOUR),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_ALPHA),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_2),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_3),FALSE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_S_SIZE),FALSE);
		}
		break;
	case 1:
		{
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_1),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_COLOUR),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_ALPHA),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_2),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_3),FALSE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_S_SIZE),FALSE);
			//Set Color and alpha
		}
		break;
	case 2:
		{
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_1),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_COLOUR),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_ALPHA),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_2),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_3),FALSE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_S_SIZE),TRUE);
			//Set alpha, image, margins etc.
		}
		break;
	case 3:
		{
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_1),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_COLOUR),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_ALPHA),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_2),TRUE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_3),TRUE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_S_SIZE),TRUE);
			//Set alpha, image, margins etc.
		}
		break;
	}
}

void SetControls(HWND hwndDlg, TCHAR *str)
{
	if (!str)
	{
		SetAppropriateGroups(hwndDlg,-1);
		return;
	}

	TCHAR buf[250];
	GetParamN(str,buf,SIZEOF(buf),1,',',TRUE);

	int Type=0;
	if (!lstrcmpi(buf,_T("Solid"))) Type=1;
	else if (!lstrcmpi(buf, _T("Image"))) Type=2;
	else if (!lstrcmpi(buf, _T("Fragment"))) Type=3;
	SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_SETCURSEL,(WPARAM)Type,0);
	SetAppropriateGroups(hwndDlg,Type);
	switch (Type)
	{
	case 1:
		{
			int r = _ttoi(GetParamN(str,buf,SIZEOF(buf),2,',',TRUE));
			int g = _ttoi(GetParamN(str,buf,SIZEOF(buf),3,',',TRUE));
			int b = _ttoi(GetParamN(str,buf,SIZEOF(buf),4,',',TRUE));
			int a = _ttoi(GetParamN(str,buf,SIZEOF(buf),5,',',TRUE));
			SendDlgItemMessage(hwndDlg,IDC_COLOR,CPM_SETCOLOUR, 0, (LPARAM)RGB(r,g,b));
			SendDlgItemMessage(hwndDlg,IDC_COLOR,CPM_SETDEFAULTCOLOUR, 0, (LPARAM)RGB(r,g,b));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS, 0, MAKELONG(a,0));
		}
		break;
	 case 2:
		{
			int fitmode = 0;

			int l = _ttoi(GetParamN(str,buf,SIZEOF(buf),4,',',TRUE));
			int t = _ttoi(GetParamN(str,buf,SIZEOF(buf),5,',',TRUE));
			int r = _ttoi(GetParamN(str,buf,SIZEOF(buf),6,',',TRUE));
			int b = _ttoi(GetParamN(str,buf,SIZEOF(buf),7,',',TRUE));
			int a = _ttoi(GetParamN(str,buf,SIZEOF(buf),8,',',TRUE));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS, 0, MAKELONG(a,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_SETPOS, 0, MAKELONG(l,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_SETPOS, 0, MAKELONG(t,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_SETPOS, 0, MAKELONG(r,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_SETPOS, 0, MAKELONG(b,0));

			GetParamN(str,buf,SIZEOF(buf),2,',',TRUE);
			SetDlgItemText(hwndDlg,IDC_FILE,buf);

			GetParamN(str,buf,SIZEOF(buf),3,',',TRUE);
			if (!lstrcmpi(buf,_T("TileBoth"))) fitmode = FM_TILE_BOTH;
            else if (!lstrcmpi(buf,_T("TileVert"))) fitmode = FM_TILE_VERT;
            else if (!lstrcmpi(buf,_T("TileHorz"))) fitmode = FM_TILE_HORZ;
            else fitmode = 0;
			SendDlgItemMessage(hwndDlg,IDC_FIT,CB_SETCURSEL,(WPARAM)fitmode,0);
		}

		break;
	 case 3:
		{
			int fitmode = 0;

			int x = _ttoi(GetParamN(str,buf,SIZEOF(buf),3,',',TRUE));
			int y = _ttoi(GetParamN(str,buf,SIZEOF(buf),4,',',TRUE));
			int w = _ttoi(GetParamN(str,buf,SIZEOF(buf),5,',',TRUE));
			int h = _ttoi(GetParamN(str,buf,SIZEOF(buf),6,',',TRUE));

			int l = _ttoi(GetParamN(str,buf,SIZEOF(buf),8,',',TRUE));
			int t = _ttoi(GetParamN(str,buf,SIZEOF(buf),9,',',TRUE));
			int r = _ttoi(GetParamN(str,buf,SIZEOF(buf),10, ',',TRUE));
			int b = _ttoi(GetParamN(str,buf,SIZEOF(buf),11,',',TRUE));
			int a = _ttoi(GetParamN(str,buf,SIZEOF(buf),12,',',TRUE));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS, 0, MAKELONG(a,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_SETPOS, 0, MAKELONG(l,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_SETPOS, 0, MAKELONG(t,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_SETPOS, 0, MAKELONG(r,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_SETPOS, 0, MAKELONG(b,0));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_POSLEFT,UDM_SETPOS, 0, MAKELONG(x,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_POSTOP,UDM_SETPOS, 0, MAKELONG(y,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_WIDTH,UDM_SETPOS, 0, MAKELONG(w,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_HEIGHT,UDM_SETPOS, 0, MAKELONG(h,0));

			GetParamN(str,buf,SIZEOF(buf),2,',',TRUE);
			SetDlgItemText(hwndDlg,IDC_FILE,buf);

			GetParamN(str,buf,SIZEOF(buf),7,',',TRUE);
			if (!lstrcmpi(buf,_T("TileBoth"))) fitmode = FM_TILE_BOTH;
            else if (!lstrcmpi(buf,_T("TileVert"))) fitmode = FM_TILE_VERT;
            else if (!lstrcmpi(buf,_T("TileHorz"))) fitmode = FM_TILE_HORZ;
            else fitmode = 0;
			SendDlgItemMessage(hwndDlg,IDC_FIT,CB_SETCURSEL,(WPARAM)fitmode,0);
		}

		break;
	}
}

int GetShortFileName(TCHAR *FullFile)
{
	TCHAR buf[MAX_PATH] = {0};
	TCHAR *f = _tcsrchr(FullFile,'\\');
	TCHAR *file = f ? mir_tstrdup(f+1) : 0;
	if (!file)
		return 0;

	ske_GetFullFilename(buf, file, 0, TRUE);
	if ( mir_bool_tstrcmpi(buf, FullFile)) {
		_tcsncpy(FullFile, file, MAX_PATH);
		mir_free(file);
		return 1; //skin folder relative
	}

	PathToRelativeT(FullFile, buf);
	if (buf[0] != '\\' && buf[1] != ':')
		mir_sntprintf(FullFile, MAX_PATH, _T("\\%s"), buf);
	else
		_tcsncpy(FullFile, buf, MAX_PATH);

	mir_free(file);
	return 2; //mirand folder relative
}

TCHAR* MadeString(HWND hwndDlg)
{
	char buf[MAX_PATH*2] = {0};
	int i = SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_GETCURSEL, 0, 0);
	switch (i)
	{
	case 0:
		mir_snprintf(buf,SIZEOF(buf),"Glyph,Skip");
		break;
	case 1:
		{
			BYTE a;
			DWORD col;
			a = (BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_GETPOS, 0, 0);
			col = (DWORD)SendDlgItemMessage(hwndDlg,IDC_COLOR,CPM_GETCOLOUR, 0, 0);
			mir_snprintf(buf,SIZEOF(buf),"Glyph,Solid,%d,%d,%d,%d",GetRValue(col),GetGValue(col),GetBValue(col),a);
		}
		break;
	case 2:
		{
			BYTE a;
			WORD l,t,b,r;
			char buf_name[MAX_PATH] = {0};
			int i=0;
			a = (BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_GETPOS, 0, 0);
			l = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_GETPOS, 0, 0);
			t = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_GETPOS, 0, 0);
			r = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_GETPOS, 0, 0);
			b = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_GETPOS, 0, 0);
			GetDlgItemTextA(hwndDlg, IDC_FILE, buf_name, MAX_PATH);
			i = SendDlgItemMessage(hwndDlg,IDC_FIT,CB_GETCURSEL, 0, 0);
			mir_snprintf(buf,SIZEOF(buf),"Glyph,Image,%s,%s,%d,%d,%d,%d,%d",buf_name,
				//fitmode
				(i == FM_TILE_BOTH)?"TileBoth":
					(i == FM_TILE_VERT)?"TileVert":
					   (i == FM_TILE_HORZ)?"TileHorz":"StretchBoth",
				l,t,r,b,a);
		}
		break;
	case 3:
		{
			BYTE a = (BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_GETPOS, 0, 0);
			WORD l = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_GETPOS, 0, 0);
			WORD t = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_GETPOS, 0, 0);
			WORD r = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_GETPOS, 0, 0);
			WORD b = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_GETPOS, 0, 0);

			WORD x = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_POSLEFT,UDM_GETPOS, 0, 0);
			WORD y = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_POSTOP,UDM_GETPOS, 0, 0);

			WORD w = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_WIDTH,UDM_GETPOS, 0, 0);
			WORD h = (WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_HEIGHT,UDM_GETPOS, 0, 0);

			char buf_name[MAX_PATH] = {0};
			GetDlgItemTextA(hwndDlg, IDC_FILE, buf_name, MAX_PATH);
			int i = SendDlgItemMessage(hwndDlg,IDC_FIT,CB_GETCURSEL, 0, 0);
			mir_snprintf(buf,SIZEOF(buf),"Glyph,Fragment,%s,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d",buf_name,x,y,w,h,
				//fitmode
				(i == FM_TILE_BOTH)?"TileBoth":
					(i == FM_TILE_VERT)?"TileVert":
					   (i == FM_TILE_HORZ)?"TileHorz":"StretchBoth",
				l,t,r,b,a);
		}
		break;
	}
	if (buf[0] != '\0') return mir_a2t(buf);
	return 0;
}

void UpdateInfo(HWND hwndDlg)
{
	TCHAR *b = MadeString(hwndDlg);
	if (!b)
	{
		SetDlgItemText(hwndDlg,IDC_EDIT1,_T(""));
		return;
	}
	SetDlgItemText(hwndDlg,IDC_EDIT1,b);
	{
		HTREEITEM hti = TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_OBJECT_TREE));
		if (hti)
		{
			TVITEM tvi = {0};
			tvi.hItem = hti;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			TreeView_GetItem(GetDlgItem(hwndDlg,IDC_OBJECT_TREE),&tvi);
			OPT_OBJECT_DATA *sd = (OPT_OBJECT_DATA*)(tvi.lParam);
			if (sd)
			{
				mir_free(sd->szValue);
				sd->szValue = mir_tstrdup(b);
			}
		}
	}
	mir_free(b);
	glSkinWasModified = 1;
}

void StoreTreeNode(HWND hTree, HTREEITEM node, char * section)
{
  HTREEITEM tmp = node;
  while (tmp)
  {
	HTREEITEM tmp2 = NULL;
    TVITEMA tvi;
    char buf[255];
    tvi.hItem = tmp;
    tvi.mask = TVIF_TEXT|TVIF_HANDLE;
    tvi.pszText = (LPSTR)&buf;
    tvi.cchTextMax = 254;
    TreeView_GetItem(hTree, &tvi);
	if (tvi.lParam)
	{
		OPT_OBJECT_DATA * dat  = (OPT_OBJECT_DATA*)(tvi.lParam);
		if (dat->szName && dat->szValue)
			db_set_ts(NULL,section,dat->szName,dat->szValue);
	}
	tmp2 = TreeView_GetChild(hTree,tmp);
	if (tmp2) StoreTreeNode(hTree,tmp2,section);
    tmp = TreeView_GetNextSibling(hTree,tmp);
  }
  return;
}

void StoreTreeToDB(HWND hTree, char * section)
{
  HTREEITEM tmp = NULL;
  tmp = TreeView_GetRoot(hTree);
  StoreTreeNode(hTree,tmp,section);
  db_set_b(NULL,section,"Modified",1);
  glSkinWasModified = 2;
}
static BOOL fileChanged = FALSE;
static TCHAR *object_clipboard = NULL;
int GetFileSizes(HWND hwndDlg)
{
	TCHAR buf[MAX_PATH];
	SIZE sz = {0};
	GetDlgItemText(hwndDlg, IDC_FILE, buf, MAX_PATH);
	ske_GetFullFilename(buf, buf, 0, TRUE);

	HBITMAP hbmp = ske_LoadGlyphImage(buf);
	if (hbmp) {
		BITMAP bm = {0};
		GetObject(hbmp,sizeof(BITMAP),&bm);
		sz.cx = bm.bmWidth;
		sz.cy = bm.bmHeight;
		ske_UnloadGlyphImage(hbmp);
	}

	mir_sntprintf(buf, MAX_PATH, _T("%s %d x %d %s"),TranslateT("Image size is"), sz.cx, sz.cy, TranslateT("pixels"));
	SetDlgItemText(hwndDlg, IDC_S_SIZE, buf);
	return 0;
}

INT_PTR CALLBACK DlgSkinEditorOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_WINDOWPOSCHANGED:
		{
			WINDOWPOS * wp = (WINDOWPOS *)lParam;
			if (lParam && wp->flags&SWP_SHOWWINDOW)
			{
				if (glOtherSkinWasLoaded)
				{
					TreeView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_OBJECT_TREE));
					FillObjectTree(hwndDlg,IDC_OBJECT_TREE,"$$*");
					glSkinWasModified = 0;
					glOtherSkinWasLoaded = FALSE;
				}
			}
			break;
		}

	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		FillObjectTree(hwndDlg,IDC_OBJECT_TREE,"$$*");
		{	//Fill types combo
			int i=0;
			for (i=0; i < SIZEOF(TYPES); i++)
				SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_ADDSTRING, 0, (LPARAM)TranslateTS(TYPES[i]));
		}
		{	//Fill fit combo
			int i=0;
			for (i=0; i < SIZEOF(FITMODES); i++)
				SendDlgItemMessage(hwndDlg,IDC_FIT,CB_ADDSTRING, 0, (LPARAM)TranslateTS(FITMODES[i]));
		}
		//SPIN Ranges
		{
			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETRANGE, 0, MAKELONG(255,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS, 0, MAKELONG(255,0));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_SETRANGE, 0, MAKELONG(900, 0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_SETRANGE, 0, MAKELONG(900, 0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_SETRANGE, 0, MAKELONG(900, 0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_SETRANGE, 0, MAKELONG(900, 0));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_POSLEFT,UDM_SETRANGE, 0, MAKELONG(1000, 0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_POSTOP,UDM_SETRANGE, 0, MAKELONG(1000, 0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_WIDTH,UDM_SETRANGE, 0, MAKELONG(1000, 0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_HEIGHT,UDM_SETRANGE, 0, MAKELONG(1000, 0));
		}
		EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_1),FALSE);
		EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_COLOUR),FALSE);
		EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_ALPHA),FALSE);
		EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_2),FALSE);
		EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_3),FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_COPY),FALSE);
		glSkinWasModified = 0;
		glOtherSkinWasLoaded = FALSE;
		break;

	case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDC_TYPE)
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					int i = SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_GETCURSEL, 0, 0);
					//if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_TYPE)))
					SetAppropriateGroups(hwndDlg,i);
					if (GetFocus() == GetDlgItem(hwndDlg,IDC_TYPE))
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					UpdateInfo(hwndDlg);
					break;
				}
			}
			else if (LOWORD(wParam) == IDC_COPY)
			{
				if (HIWORD(wParam) == BN_CLICKED)
				{
					mir_free_and_nil(object_clipboard);

					OPT_OBJECT_DATA *sd = NULL;
					HTREEITEM hti = TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_OBJECT_TREE));
					if (hti != 0)
					{
						TVITEM tvi = {0};
						tvi.hItem = hti;
						tvi.mask = TVIF_HANDLE|TVIF_PARAM;
						TreeView_GetItem(GetDlgItem(hwndDlg,IDC_OBJECT_TREE),&tvi);
						sd = (OPT_OBJECT_DATA*)(tvi.lParam);
					}
					if (sd && sd->szValue)
						object_clipboard = mir_tstrdup(sd->szValue);

					EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),object_clipboard != NULL);
					return 0;
				}
			}
			else if (LOWORD(wParam) == IDC_PASTE)
			{
				if (HIWORD(wParam) == BN_CLICKED)
				{
					if (object_clipboard)
						SetControls(hwndDlg, object_clipboard);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					UpdateInfo(hwndDlg);
					return 0;
				}
			}
			else if (LOWORD(wParam) == IDC_COLOR)
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				UpdateInfo(hwndDlg);
			}
			else if (LOWORD(wParam) == IDC_BROWSE) {
				if (HIWORD(wParam) == BN_CLICKED) {
					TCHAR str[MAX_PATH] = {0};
					OPENFILENAME ofn = {0};
					int res = 0;
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = _T("Images (*.png,*.jpg,*.bmp,*.gif,*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tga\0All files (*.*)\0*.*\0\0");
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
					GetDlgItemText(hwndDlg, IDC_FILE, str, SIZEOF(str));
					if (str[0] == '\0' || _tcschr(str,'%')) {
						ofn.Flags |= OFN_NOVALIDATE;
						str[0] = '\0';
					}
					else ske_GetFullFilename(str, str, 0, TRUE);

					ofn.lpstrFile = str;

					ofn.nMaxFile = SIZEOF(str);
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.lpstrDefExt = _T("*.*");
					res = GetOpenFileName(&ofn);
					if (res) {
						GetShortFileName(ofn.lpstrFile);
						SetDlgItemText(hwndDlg, IDC_FILE, ofn.lpstrFile);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						UpdateInfo(hwndDlg);
					}
				}
			}
			else if (LOWORD(wParam) == IDC_FILE)
			{
				if (HIWORD(wParam) == EN_CHANGE)
				{
					fileChanged = TRUE;
					if ((HWND)lParam != GetFocus())
					{
						GetFileSizes(hwndDlg);
						fileChanged = FALSE;
					}
				}
				else if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (fileChanged)
					{
						GetFileSizes(hwndDlg);
						fileChanged = FALSE;
					}
				}
			}
			else if ((
				(LOWORD(wParam) == IDC_E_TOP
				|| LOWORD(wParam) == IDC_E_BOTTOM
				|| LOWORD(wParam) == IDC_E_LEFT
				|| LOWORD(wParam) == IDC_E_RIGHT
				|| LOWORD(wParam) == IDC_E_X
				|| LOWORD(wParam) == IDC_E_Y
				|| LOWORD(wParam) == IDC_E_W
				|| LOWORD(wParam) == IDC_E_H
				|| LOWORD(wParam) == IDC_EDIT_ALPHA
				)
				&&  HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			{
				return 0;
			}
			else if (LOWORD(wParam) != IDC_EDIT1)
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				UpdateInfo(hwndDlg);
			}
			break;
		}
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_OBJECT_TREE:
			{
				NMTREEVIEWA * nmtv = (NMTREEVIEWA *) lParam;
				if (!nmtv) return 0;
				if (nmtv->hdr.code == TVN_SELCHANGEDA || nmtv->hdr.code == TVN_SELCHANGEDW)
				{
					if (nmtv->itemOld.lParam)
					{
						OPT_OBJECT_DATA * dataOld = (OPT_OBJECT_DATA*)nmtv->itemOld.lParam;
						if (dataOld->szValue) {
							mir_free(dataOld->szValue);
							dataOld->szValue = MadeString(hwndDlg);
						}
					}
					if (nmtv->itemNew.lParam)
					{
						OPT_OBJECT_DATA * data = (OPT_OBJECT_DATA*)nmtv->itemNew.lParam;
						char buf[255];

						mir_snprintf(buf,SIZEOF(buf),"%s = %s",data->szName, data->szValue);
						SetDlgItemTextA(hwndDlg,IDC_EDIT1,buf);
						SetControls(hwndDlg,data->szValue);
						EnableWindow(GetDlgItem(hwndDlg,IDC_COPY),TRUE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),object_clipboard != NULL);
					}
					else
					{
						SetDlgItemText(hwndDlg,IDC_EDIT1,_T(""));
						SetControls(hwndDlg,NULL);
						EnableWindow(GetDlgItem(hwndDlg,IDC_COPY),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),FALSE);
					}

				}
				else if (nmtv->hdr.code == TVN_DELETEITEMA)
				{
					OPT_OBJECT_DATA * dataOld = (OPT_OBJECT_DATA*)nmtv->itemOld.lParam;
					if (dataOld) {
						mir_free_and_nil(dataOld->szName);
						mir_free_and_nil(dataOld->szPath);
						mir_free_and_nil(dataOld->szTempValue);
						mir_free_and_nil(dataOld->szValue);
					}
				}
				return 0;
			}

		case 0:
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_APPLY:
				if (!glOtherSkinWasLoaded) //store only if skin is same
				{
					StoreTreeToDB(GetDlgItem(hwndDlg,IDC_OBJECT_TREE),SKIN);
					//ReloadSkin
					ske_LoadSkinFromDB();
					pcli->pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0);
					Sync(CLUIFrames_OnClistResize_mod, 0, 0);
					ske_RedrawCompleteWindow();
					Sync(CLUIFrames_OnClistResize_mod, 0, 0);
					{
						HWND hwnd = pcli->hwndContactList;
						RECT rc = {0};
						GetWindowRect(hwnd, &rc);
						Sync(CLUIFrames_OnMoving,hwnd,&rc);
					}
					return TRUE;
				}
				else
					return TRUE;
			}
			break;
		}

	case WM_DESTROY:
		mir_free(object_clipboard);
		break;

	}
	return FALSE;
}

int EnableGroup(HWND hwndDlg, HWND first, BOOL bEnable)
{
	HWND hwnd = first;
	BOOL exit = FALSE;
	if (!hwnd) return 0;
	do
	{
		EnableWindow(hwnd,bEnable);
		hwnd = GetWindow(hwnd,GW_HWNDNEXT);
		if (!hwnd || GetWindowLongPtr(hwnd,GWL_STYLE)&WS_GROUP) exit = TRUE;
	}
		while (!exit);
	return 0;
}

int ShowGroup(HWND hwndDlg, HWND first, BOOL bEnable)
{
	HWND hwnd = first;
	BOOL exit = FALSE;
	if (!hwnd) return 0;
	do
	{
		ShowWindow(hwnd,bEnable?SW_SHOW:SW_HIDE);
		hwnd = GetWindow(hwnd,GW_HWNDNEXT);
		if (!hwnd || GetWindowLongPtr(hwnd,GWL_STYLE)&WS_GROUP) exit = TRUE;
	}
		while (!exit);
	return 0;
}
