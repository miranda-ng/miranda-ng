/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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
#include "m_clui.h"
#include "hdr/modern_clist.h"
#include "m_clc.h"
#include "io.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_sync.h"

typedef struct _OPT_OBJECT_DATA
{
	char * szName;
	char * szPath;
	char * szValue;
	char * szTempValue;
} OPT_OBJECT_DATA;

static char *gl_Mask=NULL;
HWND gl_Dlg=NULL;
int  gl_controlID=0;
int EnableGroup(HWND hwndDlg, HWND first, BOOL bEnable);
int ShowGroup(HWND hwndDlg, HWND first, BOOL bEnable);
BOOL glOtherSkinWasLoaded=FALSE;
BYTE glSkinWasModified=0;  //1- but not applied, 2-stored to DB 
HTREEITEM FindChild(HWND hTree, HTREEITEM Parent, char * Caption)
{
  HTREEITEM res=NULL, tmp=NULL;
  if (Parent) 
    tmp=TreeView_GetChild(hTree,Parent);
  else 
	tmp=TreeView_GetRoot(hTree);
  while (tmp)
  {
    TVITEMA tvi;
    char buf[255];
    tvi.hItem=tmp;
    tvi.mask=TVIF_TEXT|TVIF_HANDLE;
    tvi.pszText=(LPSTR)&buf;
    tvi.cchTextMax=254;
    TreeView_GetItemA(hTree,&tvi);
    if (mir_bool_strcmpi(Caption,tvi.pszText))
      return tmp;
    tmp=TreeView_GetNextSibling(hTree,tmp);
  }
  return tmp;
}

int TreeAddObject(HWND hwndDlg, int ID, OPT_OBJECT_DATA * data)
{
	HTREEITEM rootItem=NULL;
	HTREEITEM cItem=NULL;
	char * path;
	char * ptr;
	char * ptrE;
	char buf[255];
	BOOL ext=FALSE;
	path=data->szPath?mir_strdup(data->szPath):(data->szName[1]=='$')?mir_strdup((data->szName)+2):NULL;
	if (!path)
	{
		mir_snprintf(buf,SIZEOF(buf),"$(other)/%s",(data->szName)+1);
		path=mir_strdup(buf);
	}

	ptr=path;
	ptrE=path;	
	do 
	{
		
		while (*ptrE!='/' && *ptrE!='\0') ptrE++;
		if (*ptrE=='/')
		{
			*ptrE='\0';
			ptrE++;
			// find item if not - create;
			{
				cItem=FindChild(GetDlgItem(hwndDlg,ID),rootItem,ptr);
				if (!cItem) // not found - create node
				{
					TVINSERTSTRUCTA tvis;
					tvis.hParent=rootItem;
					tvis.hInsertAfter=TVI_SORT;
					tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
					tvis.item.pszText=ptr;
					tvis.item.lParam=(LPARAM)NULL;
					cItem=TreeView_InsertItemA(GetDlgItem(hwndDlg,ID),&tvis);
					
				}	
				rootItem=cItem;
			}
			ptr=ptrE;
		}
		else ext=TRUE;
	}while (!ext);
	//Insert item node
	{
		TVINSERTSTRUCTA tvis;
		tvis.hParent=rootItem;
		tvis.hInsertAfter=TVI_SORT;
		tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
		tvis.item.pszText=ptr;
		tvis.item.lParam=(LPARAM)data;
		TreeView_InsertItemA(GetDlgItem(hwndDlg,ID),&tvis);
	}
	mir_free_and_nill(path);
	return 0;
}

int enumDB_SkinObjectsForEditorProc(const char *szSetting,LPARAM lParam)
{
	if (wildcmp((char *)szSetting,gl_Mask,0)||wildcmp((char *)szSetting,"$*",0))
	{
		char * value;
		char *desc;
		char *descKey;
		descKey=mir_strdup(szSetting);
		descKey[0]='%';
		value= ModernGetStringA(NULL,SKIN,szSetting);
		desc= ModernGetStringA(NULL,SKIN,descKey);
		if (wildcmp(value,"?lyph*",0))
		{
			OPT_OBJECT_DATA * a=(OPT_OBJECT_DATA*)mir_alloc(sizeof(OPT_OBJECT_DATA));
			a->szPath=desc;
			a->szName=mir_strdup(szSetting);
			a->szValue=value;
			a->szTempValue=NULL;
			TreeAddObject(gl_Dlg,gl_controlID,a);
		}
		else
		{
			if (value) mir_free_and_nill(value);
			if (desc) mir_free_and_nill(desc);
		}
		mir_free_and_nill(descKey);		
	}	
	return 0;
}

int FillObjectTree(HWND hwndDlg, int ObjectTreeID, char * wildmask)
{
	DBCONTACTENUMSETTINGS dbces;
	gl_Dlg=hwndDlg;
	gl_controlID=ObjectTreeID;
	gl_Mask=wildmask;
	dbces.pfnEnumProc=enumDB_SkinObjectsForEditorProc;
	dbces.szModule=SKIN;
	dbces.ofsSettings=0;
	CallService(MS_DB_CONTACT_ENUMSETTINGS,0,(LPARAM)&dbces);
	return 0;
}
TCHAR *TYPES[]={_T("- Empty - (do not draw this object)"),_T("Solid fill object"),_T("Image (draw image)"),_T("Fragment (draw portion of image)")};
TCHAR *FITMODES[]={_T("Stretch Both directions"),_T("Stretch Vertical, Tile Horizontal"),_T("Tile Vertical, Stretch Horizontal"),_T("Tile Both directions")};

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

void SetControls(HWND hwndDlg, char * str)
{
	char buf[250];
	int Type=0;
	if (!str)
	{
		SetAppropriateGroups(hwndDlg,-1); 
		return;
	}
	GetParamN(str,buf,SIZEOF(buf),1,',',TRUE);
	if (mir_bool_strcmpi(buf,"Solid")) Type=1;
	else if (mir_bool_strcmpi(buf,"Image")) Type=2;
	else if (mir_bool_strcmpi(buf,"Fragment")) Type=3;
	SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_SETCURSEL,(WPARAM)Type,(LPARAM)0);
	SetAppropriateGroups(hwndDlg,Type);
	switch (Type)
	{
	case 1:
		{
			int r,g,b,a;
			r=g=b=200;
			a=255;
			r=atoi(GetParamN(str,buf,SIZEOF(buf),2,',',TRUE));
			g=atoi(GetParamN(str,buf,SIZEOF(buf),3,',',TRUE));
			b=atoi(GetParamN(str,buf,SIZEOF(buf),4,',',TRUE));
			a=atoi(GetParamN(str,buf,SIZEOF(buf),5,',',TRUE));
			SendDlgItemMessage(hwndDlg,IDC_COLOR,CPM_SETCOLOUR,(WPARAM)0,(LPARAM)RGB(r,g,b));
			SendDlgItemMessage(hwndDlg,IDC_COLOR,CPM_SETDEFAULTCOLOUR,(WPARAM)0,(LPARAM)RGB(r,g,b));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS,0,MAKELONG(a,0));
		}
		break;
	 case 2:
		{
			int a;
			int l,t,r,b;
			int fitmode=0;
			l=t=r=b=0;
			a=255;
			
			l=atoi(GetParamN(str,buf,SIZEOF(buf),4,',',TRUE));
			t=atoi(GetParamN(str,buf,SIZEOF(buf),5,',',TRUE));
			r=atoi(GetParamN(str,buf,SIZEOF(buf),6,',',TRUE));
			b=atoi(GetParamN(str,buf,SIZEOF(buf),7,',',TRUE));
			a=atoi(GetParamN(str,buf,SIZEOF(buf),8,',',TRUE));
			
			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS,0,MAKELONG(a,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_SETPOS,0,MAKELONG(l,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_SETPOS,0,MAKELONG(t,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_SETPOS,0,MAKELONG(r,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_SETPOS,0,MAKELONG(b,0));
			
			GetParamN(str,buf,SIZEOF(buf),2,',',TRUE);
			SendDlgItemMessageA(hwndDlg,IDC_FILE,WM_SETTEXT,0,(LPARAM)buf);
			
			GetParamN(str,buf,SIZEOF(buf),3,',',TRUE);
			if (mir_bool_strcmpi(buf,"TileBoth")) fitmode=FM_TILE_BOTH;
            else if (mir_bool_strcmpi(buf,"TileVert")) fitmode=FM_TILE_VERT;
            else if (mir_bool_strcmpi(buf,"TileHorz")) fitmode=FM_TILE_HORZ;
            else fitmode=0;  
			SendDlgItemMessage(hwndDlg,IDC_FIT,CB_SETCURSEL,(WPARAM)fitmode,(LPARAM)0);
		}

		break;
	 case 3:
		{
			int a;
			int l,t,r,b;
			int x,y,w,h;
			int fitmode=0;
			l=t=r=b=0;
			x=y=w=h=0;
			a=255;
			
			x=atoi(GetParamN(str,buf,SIZEOF(buf),3,',',TRUE));
			y=atoi(GetParamN(str,buf,SIZEOF(buf),4,',',TRUE));
			w=atoi(GetParamN(str,buf,SIZEOF(buf),5,',',TRUE));
			h=atoi(GetParamN(str,buf,SIZEOF(buf),6,',',TRUE));

			l=atoi(GetParamN(str,buf,SIZEOF(buf),8,',',TRUE));
			t=atoi(GetParamN(str,buf,SIZEOF(buf),9,',',TRUE));
			r=atoi(GetParamN(str,buf,SIZEOF(buf),10,',',TRUE));
			b=atoi(GetParamN(str,buf,SIZEOF(buf),11,',',TRUE));
			a=atoi(GetParamN(str,buf,SIZEOF(buf),12,',',TRUE));
			
			SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS,0,MAKELONG(a,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_SETPOS,0,MAKELONG(l,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_SETPOS,0,MAKELONG(t,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_SETPOS,0,MAKELONG(r,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_SETPOS,0,MAKELONG(b,0));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_POSLEFT,UDM_SETPOS,0,MAKELONG(x,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_POSTOP,UDM_SETPOS,0,MAKELONG(y,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_WIDTH,UDM_SETPOS,0,MAKELONG(w,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_HEIGHT,UDM_SETPOS,0,MAKELONG(h,0));
			
			GetParamN(str,buf,SIZEOF(buf),2,',',TRUE);
			SendDlgItemMessageA(hwndDlg,IDC_FILE,WM_SETTEXT,0,(LPARAM)buf);
			
			GetParamN(str,buf,SIZEOF(buf),7,',',TRUE);
			if (mir_bool_strcmpi(buf,"TileBoth")) fitmode=FM_TILE_BOTH;
            else if (mir_bool_strcmpi(buf,"TileVert")) fitmode=FM_TILE_VERT;
            else if (mir_bool_strcmpi(buf,"TileHorz")) fitmode=FM_TILE_HORZ;
            else fitmode=0;  
			SendDlgItemMessage(hwndDlg,IDC_FIT,CB_SETCURSEL,(WPARAM)fitmode,(LPARAM)0);
		}

		break;
	}
}



int GetShortFileName(char * FullFile)
{
	char buf[MAX_PATH]={0};
	char * f=strrchr(FullFile,'\\');
	char * file=f?mir_strdup(f+1):0;
	if (!file) return 0;
	ske_GetFullFilename(buf,file,0,TRUE);
	if (mir_bool_strcmpi(buf,FullFile))
	{
		_snprintf(FullFile,MAX_PATH,"%s",file);
		mir_free_and_nill(file);
		return 1; //skin folder relative
	}
	else
	{
		CallService(MS_UTILS_PATHTORELATIVE,(WPARAM)FullFile,(LPARAM)buf);
		if (buf[0]!='\\' && buf[1]!=':')
			_snprintf(FullFile,MAX_PATH,"\\%s",buf);
		else
			_snprintf(FullFile,MAX_PATH,"%s",buf);
	}
	mir_free_and_nill(file);
	return 2; //mirand folder relative
}

char * MadeString(HWND hwndDlg)
{
	char buf[MAX_PATH*2]={0};
	int i=SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
	switch (i)
	{
	case 0:
		mir_snprintf(buf,SIZEOF(buf),"Glyph,Skip");
		break;
	case 1:
		{
			BYTE a;
			DWORD col;
			a=(BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_GETPOS,0,0);
			col=(DWORD)SendDlgItemMessage(hwndDlg,IDC_COLOR,CPM_GETCOLOUR,(WPARAM)0,(LPARAM)0);
			mir_snprintf(buf,SIZEOF(buf),"Glyph,Solid,%d,%d,%d,%d",GetRValue(col),GetGValue(col),GetBValue(col),a);
		}
		break;
	case 2:
		{
			BYTE a;
			WORD l,t,b,r;
			char buf_name[MAX_PATH]={0};
			int i=0;
			a=(BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_GETPOS,0,0);
			l=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_GETPOS,0,0);
			t=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_GETPOS,0,0);
			r=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_GETPOS,0,0);
			b=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_GETPOS,0,0);
			SendDlgItemMessageA(hwndDlg,IDC_FILE,WM_GETTEXT,(WPARAM)MAX_PATH,(LPARAM)buf_name);
			i=SendDlgItemMessage(hwndDlg,IDC_FIT,CB_GETCURSEL,0,0);
			mir_snprintf(buf,SIZEOF(buf),"Glyph,Image,%s,%s,%d,%d,%d,%d,%d",buf_name,
				//fitmode
				(i==FM_TILE_BOTH)?"TileBoth":
					(i==FM_TILE_VERT)?"TileVert":
					   (i==FM_TILE_HORZ)?"TileHorz":"StretchBoth",
				l,t,r,b,a);
		}
		break;
	case 3:
		{
			BYTE a;
			WORD l,t,b,r;
			WORD x,y,w,h;
			char buf_name[MAX_PATH]={0};
			int i=0;
			a=(BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_GETPOS,0,0);
			l=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_GETPOS,0,0);
			t=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_GETPOS,0,0);
			r=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_GETPOS,0,0);
			b=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_GETPOS,0,0);

			x=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_POSLEFT,UDM_GETPOS,0,0);
			y=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_POSTOP,UDM_GETPOS,0,0);

			w=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_WIDTH,UDM_GETPOS,0,0);
			h=(WORD)SendDlgItemMessage(hwndDlg,IDC_SPIN_HEIGHT,UDM_GETPOS,0,0);

			SendDlgItemMessageA(hwndDlg,IDC_FILE,WM_GETTEXT,(WPARAM)MAX_PATH,(LPARAM)buf_name);
			i=SendDlgItemMessage(hwndDlg,IDC_FIT,CB_GETCURSEL,0,0);
			mir_snprintf(buf,SIZEOF(buf),"Glyph,Fragment,%s,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d",buf_name,x,y,w,h,
				//fitmode
				(i==FM_TILE_BOTH)?"TileBoth":
					(i==FM_TILE_VERT)?"TileVert":
					   (i==FM_TILE_HORZ)?"TileHorz":"StretchBoth",
				l,t,r,b,a);
		}
		break;
	}
	if (buf[0]!='\0') return mir_strdup(buf);
	return 0;
}
void UpdateInfo(HWND hwndDlg)
{
	char *b=MadeString(hwndDlg);
	if (!b) 
	{
		SendDlgItemMessageA(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM)"");
		return;
	}
	SendDlgItemMessageA(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM)b);
	{
		OPT_OBJECT_DATA *sd=NULL;  
		HTREEITEM hti=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_OBJECT_TREE));				
		if (hti)
		{
			TVITEMA tvi={0};
			tvi.hItem=hti;
			tvi.mask=TVIF_HANDLE|TVIF_PARAM;
			TreeView_GetItem(GetDlgItem(hwndDlg,IDC_OBJECT_TREE),&tvi);
			sd=(OPT_OBJECT_DATA*)(tvi.lParam);
			if (sd)
			{
				if (sd->szValue) mir_free_and_nill(sd->szValue);
				sd->szValue=mir_strdup(b);
			}
		}
	}
	mir_free_and_nill(b);
	glSkinWasModified=1;
}

void StoreTreeNode(HWND hTree, HTREEITEM node, char * section)
{
  HTREEITEM tmp;
  tmp=node;
  while (tmp)
  {
	HTREEITEM tmp2=NULL;
    TVITEMA tvi;
    char buf[255];
    tvi.hItem=tmp;
    tvi.mask=TVIF_TEXT|TVIF_HANDLE;
    tvi.pszText=(LPSTR)&buf;
    tvi.cchTextMax=254;
    TreeView_GetItemA(hTree,&tvi);
	if (tvi.lParam)
	{
		OPT_OBJECT_DATA * dat =(OPT_OBJECT_DATA*)(tvi.lParam);
		if (dat->szName && dat->szValue)
			ModernWriteSettingString(NULL,section,dat->szName,dat->szValue);
	}
	tmp2=TreeView_GetChild(hTree,tmp);
	if (tmp2) StoreTreeNode(hTree,tmp2,section);
    tmp=TreeView_GetNextSibling(hTree,tmp);
  }
  return;
}

void StoreTreeToDB(HWND hTree, char * section)
{
  HTREEITEM tmp=NULL;
  tmp=TreeView_GetRoot(hTree);
  StoreTreeNode(hTree,tmp,section);
  ModernWriteSettingByte(NULL,section,"Modified",1);
  glSkinWasModified=2;
}
static BOOL fileChanged=FALSE;
static char * object_clipboard=NULL;
int GetFileSizes(HWND hwndDlg)
{
	char buf[MAX_PATH];
	SIZE sz={0};
	SendDlgItemMessageA(hwndDlg,IDC_FILE,WM_GETTEXT,(WPARAM)MAX_PATH,(LPARAM)buf);
	ske_GetFullFilename(buf,buf,0,TRUE);
	{
		HBITMAP hbmp=ske_LoadGlyphImage(buf);
		if (hbmp)
		{
			BITMAP bm={0};
			GetObject(hbmp,sizeof(BITMAP),&bm);
			sz.cx=bm.bmWidth;
			sz.cy=bm.bmHeight;
			ske_UnloadGlyphImage(hbmp);
		}
	}
	_snprintf(buf, MAX_PATH, "%s %d x %d %s",Translate("Image size is"),sz.cx, sz.cy, Translate("pixels")); 
	SendDlgItemMessageA(hwndDlg,IDC_S_SIZE,WM_SETTEXT,0,(LPARAM)buf);
	return 0;
}
INT_PTR CALLBACK DlgSkinEditorOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY: 
		{
			if (object_clipboard) mir_free_and_nill(object_clipboard);
			break;
		}
	case WM_WINDOWPOSCHANGED:
		{
			WINDOWPOS * wp=(WINDOWPOS *)lParam;
			if (lParam && wp->flags&SWP_SHOWWINDOW)
			{
				if (glOtherSkinWasLoaded)
				{
					TreeView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_OBJECT_TREE));
					FillObjectTree(hwndDlg,IDC_OBJECT_TREE,"$$*");
					glSkinWasModified=0;
					glOtherSkinWasLoaded=FALSE;
				}
			}
			break;
		}
	case WM_INITDIALOG:
		{ 			
			TranslateDialogDefault(hwndDlg);
			FillObjectTree(hwndDlg,IDC_OBJECT_TREE,"$$*");
			{	//Fill types combo
				int i=0;
				for (i=0; i<SIZEOF(TYPES); i++)
					SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_ADDSTRING,0,(LPARAM)TranslateTS(TYPES[i]));
			}
			{	//Fill fit combo
				int i=0;
				for (i=0; i<SIZEOF(FITMODES); i++)
					SendDlgItemMessage(hwndDlg,IDC_FIT,CB_ADDSTRING,0,(LPARAM)TranslateTS(FITMODES[i]));
			}
			//SPIN Ranges
			{
				SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETRANGE,0,MAKELONG(255,0));
				SendDlgItemMessage(hwndDlg,IDC_SPIN_ALPHA,UDM_SETPOS,0,MAKELONG(255,0));

				SendDlgItemMessage(hwndDlg,IDC_SPIN_TOP,UDM_SETRANGE,0,MAKELONG(900,0));
				SendDlgItemMessage(hwndDlg,IDC_SPIN_LEFT,UDM_SETRANGE,0,MAKELONG(900,0));
				SendDlgItemMessage(hwndDlg,IDC_SPIN_BOTTOM,UDM_SETRANGE,0,MAKELONG(900,0));
				SendDlgItemMessage(hwndDlg,IDC_SPIN_RIGHT,UDM_SETRANGE,0,MAKELONG(900,0));
				
				SendDlgItemMessage(hwndDlg,IDC_SPIN_POSLEFT,UDM_SETRANGE,0,MAKELONG(1000,0));
				SendDlgItemMessage(hwndDlg,IDC_SPIN_POSTOP,UDM_SETRANGE,0,MAKELONG(1000,0));
				SendDlgItemMessage(hwndDlg,IDC_SPIN_WIDTH,UDM_SETRANGE,0,MAKELONG(1000,0));
				SendDlgItemMessage(hwndDlg,IDC_SPIN_HEIGHT,UDM_SETRANGE,0,MAKELONG(1000,0));
			}		
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_1),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_COLOUR),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_ST_ALPHA),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_2),FALSE);
			EnableGroup(hwndDlg,GetDlgItem(hwndDlg,IDC_GROUP_3),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_COPY),FALSE);
			glSkinWasModified=0;
			glOtherSkinWasLoaded=FALSE;
			break;
		}

	case WM_COMMAND:	
		{	
			if (LOWORD(wParam)==IDC_TYPE)
			{
				if (HIWORD(wParam)==CBN_SELCHANGE)
				{
					int i=SendDlgItemMessage(hwndDlg,IDC_TYPE,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
					//if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_TYPE)))
						SetAppropriateGroups(hwndDlg,i);
					if (GetFocus()==GetDlgItem(hwndDlg,IDC_TYPE))
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					UpdateInfo(hwndDlg);
					break;
				}
			}
			else if (LOWORD(wParam)==IDC_COPY)
			{
				if (HIWORD(wParam)==BN_CLICKED)
				{
					if (object_clipboard) mir_free_and_nill(object_clipboard);
					object_clipboard=NULL;
					{
							OPT_OBJECT_DATA *sd=NULL;  
							HTREEITEM hti=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_OBJECT_TREE));				
							if (hti!=0)
							{
								TVITEM tvi={0};
								tvi.hItem=hti;
								tvi.mask=TVIF_HANDLE|TVIF_PARAM;
								TreeView_GetItem(GetDlgItem(hwndDlg,IDC_OBJECT_TREE),&tvi);
								sd=(OPT_OBJECT_DATA*)(tvi.lParam);
							}
							if (sd && sd->szValue) 
								object_clipboard=mir_strdup(sd->szValue);
					}
					EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),object_clipboard!=NULL);
					return 0;
				}

			}
			else if (LOWORD(wParam)==IDC_PASTE)
			{
				if (HIWORD(wParam)==BN_CLICKED)
				{
					if (object_clipboard) 
						SetControls(hwndDlg, object_clipboard);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					UpdateInfo(hwndDlg);
					return 0;
				}
			}
			else if (LOWORD(wParam)==IDC_COLOR)
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				UpdateInfo(hwndDlg);
			}
			else if (LOWORD(wParam)==IDC_BROWSE)
			{
				if (HIWORD(wParam)==BN_CLICKED)
				{
					{   		
						char str[MAX_PATH]={0};
						OPENFILENAMEA ofn={0};
						char filter[512]={0};
						int res=0;
						ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
						ofn.hwndOwner = hwndDlg;
						ofn.hInstance = NULL;					
						ofn.lpstrFilter = "Images (*.png,*.jpg,*.bmp,*.gif,*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tga\0All files (*.*)\0*.*\0\0";
						ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY);
						SendDlgItemMessageA(hwndDlg,IDC_FILE,WM_GETTEXT,(WPARAM)SIZEOF(str),(LPARAM)str);
						if (str[0]=='\0' || strchr(str,'%'))
						{
							ofn.Flags|=OFN_NOVALIDATE;
							str[0]='\0';
						}
						else
						{
							ske_GetFullFilename(str,str,(char*)0,TRUE);
						}
						ofn.lpstrFile = str;
						
						ofn.nMaxFile = SIZEOF(str);
						ofn.nMaxFileTitle = MAX_PATH;
						ofn.lpstrDefExt = "*.*";
//						{
//							DWORD tick=GetTickCount();
							res=GetOpenFileNameA(&ofn);
//							if(!res) 
//								if (GetTickCount()-tick<100)
//								{
//									res=GetOpenFileNameA(&ofn);
//									if(!res) break;
//								}
//								else break;
						//}
						if (res)
						{
							GetShortFileName(ofn.lpstrFile);
							SendDlgItemMessageA(hwndDlg,IDC_FILE,WM_SETTEXT,(WPARAM)0,(LPARAM)ofn.lpstrFile);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							UpdateInfo(hwndDlg);						
						}
					}
				}
			}
			else if (LOWORD(wParam)==IDC_FILE)
			{
				if (HIWORD(wParam) == EN_CHANGE)
				{
					fileChanged=TRUE;
					if ((HWND)lParam != GetFocus())
					{
						GetFileSizes(hwndDlg);
						fileChanged=FALSE;
					}
				}
				else if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (fileChanged)
					{
						GetFileSizes(hwndDlg);
						fileChanged=FALSE;
					}
				}
			}
			else if ((
				(LOWORD(wParam)==IDC_E_TOP
			   ||LOWORD(wParam)==IDC_E_BOTTOM
			   ||LOWORD(wParam)==IDC_E_LEFT
			   ||LOWORD(wParam)==IDC_E_RIGHT
			   ||LOWORD(wParam)==IDC_E_X
			   ||LOWORD(wParam)==IDC_E_Y
			   ||LOWORD(wParam)==IDC_E_W
			   ||LOWORD(wParam)==IDC_E_H
			   ||LOWORD(wParam)==IDC_EDIT_ALPHA
			   ) 
				&& HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			{
				return 0;
			}
			else if (LOWORD(wParam)!=IDC_EDIT1)
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				UpdateInfo(hwndDlg);
			}
			//check (LOWORD(wParam))
			//SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) 
		{
		case IDC_OBJECT_TREE:
				{
					//Save existed object
					//Change to new object
					NMTREEVIEWA * nmtv = (NMTREEVIEWA *) lParam;
					if (!nmtv) return 0;
					if (nmtv->hdr.code==TVN_SELCHANGEDA || nmtv->hdr.code==TVN_SELCHANGEDW)
					{
						if (nmtv->itemOld.lParam)
						{
								OPT_OBJECT_DATA * dataOld=(OPT_OBJECT_DATA*)nmtv->itemOld.lParam;
								if (dataOld->szValue)								
								{
									mir_free_and_nill(dataOld->szValue);
									dataOld->szValue=MadeString(hwndDlg);
								}
						}
						if (nmtv->itemNew.lParam)
						{
						
							OPT_OBJECT_DATA * data=(OPT_OBJECT_DATA*)nmtv->itemNew.lParam;
							char buf[255];
							
							mir_snprintf(buf,SIZEOF(buf),"%s=%s",data->szName, data->szValue);						
							SendDlgItemMessageA(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM)buf);					
							SetControls(hwndDlg,data->szValue);
							EnableWindow(GetDlgItem(hwndDlg,IDC_COPY),TRUE);
							EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),object_clipboard!=NULL);
						}
						else
						{
							SendDlgItemMessageA(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM)"");
							SetControls(hwndDlg,NULL);
							EnableWindow(GetDlgItem(hwndDlg,IDC_COPY),FALSE);
							EnableWindow(GetDlgItem(hwndDlg,IDC_PASTE),FALSE);
						}

					}
					else if (nmtv->hdr.code==TVN_DELETEITEMA)
					{
						OPT_OBJECT_DATA * dataOld=(OPT_OBJECT_DATA*)nmtv->itemOld.lParam;
						if (dataOld)
						{
							if (dataOld->szName) mir_free_and_nill(dataOld->szName);
							if (dataOld->szPath) mir_free_and_nill(dataOld->szPath);
							if (dataOld->szTempValue) mir_free_and_nill(dataOld->szTempValue);
							if (dataOld->szValue) mir_free_and_nill(dataOld->szValue);
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
					pcli->pfnClcBroadcast( INTM_RELOADOPTIONS,0,0);
					Sync(CLUIFrames_OnClistResize_mod,0,0);
					ske_RedrawCompleteWindow();        
					Sync(CLUIFrames_OnClistResize_mod, (WPARAM)0, (LPARAM)0);
					{
						HWND hwnd=pcli->hwndContactList;
						RECT rc={0};
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
	}
	return FALSE;
}

int EnableGroup(HWND hwndDlg, HWND first, BOOL bEnable)
{
	HWND hwnd=first;
	BOOL exit=FALSE;
	if (!hwnd) return 0;
    do
	{
		EnableWindow(hwnd,bEnable);
		hwnd=GetWindow(hwnd,GW_HWNDNEXT);
		if (!hwnd || GetWindowLong(hwnd,GWL_STYLE)&WS_GROUP) exit=TRUE;
	}while (!exit);
	return 0;
}

int ShowGroup(HWND hwndDlg, HWND first, BOOL bEnable)
{
	HWND hwnd=first;
	BOOL exit=FALSE;
	if (!hwnd) return 0;
    do
	{
		ShowWindow(hwnd,bEnable?SW_SHOW:SW_HIDE);
		hwnd=GetWindow(hwnd,GW_HWNDNEXT);
		if (!hwnd || GetWindowLong(hwnd,GWL_STYLE)&WS_GROUP) exit=TRUE;
	}while (!exit);
	return 0;
}

