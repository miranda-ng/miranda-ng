/**************************************************************************\

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
all portions of this code base are copyrighted to Artem Shpynov and/or
the people listed in contributors.txt.

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

****************************************************************************

Created: Mar 19, 2007

Author and Copyright:  Artem Shpynov aka FYR:  ashpynov@gmail.com

****************************************************************************

File contains realization of options procedures 
for modern_ext_frames.c module.

This file have to be excluded from compilation and need to be adde to project via
#include preprocessor derective in modern_ext_frames.c

\**************************************************************************/

#include "..\commonheaders.h"  //only for precompiled headers

#ifdef __modern_ext_frames_c__include_c_file   //protection from adding to compilation

static SortedList * extFrmOptList=NULL;

static int _ExtFrames_OptionsDlgInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	efcheck 0;
	if (MirandaExiting()) return 0;
	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=g_hInst;
	//odp.ptszGroup=TranslateT("Contact List");
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_EXTFRAMES);
	odp.ptszTitle=LPGENT("Contact List");
	odp.pfnDlgProc=_ExtFrames_DlgProcFrameOpts;
	odp.ptszTab=LPGENT("Frames");
	odp.flags=ODPF_BOLDGROUPS|ODPF_EXPERTONLY|ODPF_TCHAR;
#ifdef _DEBUG
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
#endif
	return 0;
}

static int _ExtFramesOptUtils_enumdb_Frames (const char *szSetting,LPARAM lParam)
{
	EXTFRAMEOPTDBENUMERATION * params=(EXTFRAMEOPTDBENUMERATION *)lParam;
	char * szName=NULL;

	if (wildcmpi((char*)szSetting,EXTFRAMEORDERDBPREFIX "?*"))
	{
		char szDBKey[100];
		DWORD nOrder;
		EXTFRAMEWND * pExtFrame;
		szName=(char*)szSetting+SIZEOF(EXTFRAMEORDERDBPREFIX)-1;

		_snprintf(szDBKey, SIZEOF(szDBKey), EXTFRAMEORDERDBPREFIX"%s", szName);
		nOrder=DBGetContactSettingDword(NULL, EXTFRAMEMODULE, szDBKey, 0);

		pExtFrame=mir_alloc(sizeof(EXTFRAMEWND));
		memset(pExtFrame, 0,  sizeof(EXTFRAMEWND));

		pExtFrame->efrm.szFrameNameID = mir_strdup(szName);
		pExtFrame->efrm.bNotRegistered = TRUE;
		pExtFrame->efrm.dwOrder = nOrder;
		li.List_Insert(params->pFrameList, pExtFrame, params->pFrameList->realCount);
	}
	return 0;
}

static void _ExtFramesOptUtils_FillListOfFrames(HWND hwndDlg, SortedList * pFrameList)
{
	DBCONTACTENUMSETTINGS dbces;
	EXTFRAMEOPTDBENUMERATION params={0};
	int i, j;

	//firstly add all known frames from db as not registered
	params.hwndDlg=hwndDlg;
	params.pFrameList=pFrameList;
	dbces.pfnEnumProc=_ExtFramesOptUtils_enumdb_Frames;
	dbces.szModule=EXTFRAMEMODULE;
	dbces.ofsSettings=0;
	dbces.lParam=(LPARAM)&params;
	CallService(MS_DB_CONTACT_ENUMSETTINGS,0,(LPARAM)&dbces);

	//now lets check registered frames and if it in list - set registered if not set add
	efcheck;
	eflock;
	{
		for (i=0; i<ExtFrames.List->realCount; i++)
		{
			EXTFRAMEWND * pRegExtFrm=(EXTFRAMEWND *)ExtFrames.List->items[i];
			EXTFRAMEWND * pKnownFrm=NULL;
			for (j=0; j<pFrameList->realCount; j++)
			{
				EXTFRAMEWND * pFrm=(EXTFRAMEWND *)pFrameList->items[j];
				if (!mir_strcmpi(pFrm->efrm.szFrameNameID, pRegExtFrm->efrm.szFrameNameID))
				{
					pKnownFrm=pFrm;
					break;
				}
			}
			if (!pKnownFrm)
			{
				pKnownFrm=mir_alloc(sizeof(EXTFRAMEWND));
				memset(pKnownFrm, 0,  sizeof(EXTFRAMEWND));				
				li.List_Insert(pFrameList, pKnownFrm, pFrameList->realCount);
			}
			if (pKnownFrm->efrm.szFrameNameID) mir_free(pKnownFrm->efrm.szFrameNameID);
			*pKnownFrm=*pRegExtFrm;
			pKnownFrm->efrm.szFrameNameID = mir_strdup(pRegExtFrm->efrm.szFrameNameID);		
		}
	}
	efunlock;
	li_SortList(pFrameList,NULL);
	//now lets add all Registered and visible frames to frame list
	for (i=0; i<pFrameList->realCount; i++)
	{
		EXTFRAMEWND * pFrm=(EXTFRAMEWND *)pFrameList->items[i];
		TCHAR * name=mir_a2t(pFrm->efrm.szFrameNameID);
		int idx=SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_ADDSTRING, 0, (LPARAM)TranslateTS(name));	
		mir_free(name);
		SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_SETITEMDATA, idx, (LPARAM) pFrm);
	}
}
static void _ExtFramesOptUtils_RenderPreview(HWND hwndDlg, SortedList * pList, HDC hDC)
{
	int i;
	RECT rcCLUIWindow;
	RECT rcPreviewWindow;
	RECT outRect={0};
	float scaleX=1, scaleY=1;
	HFONT hFont = (HFONT)SendMessage(hwndDlg,WM_GETFONT,0,0);
	LOGFONT lf;
	HFONT hNewFont;
	void * pCurrSelected=NULL;
	int idx=SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETCURSEL, 0,0);	
	int Num=0;
	if (idx>=0) 
		pCurrSelected=(void*)SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETITEMDATA, idx,0);	
	GetObject(hFont, sizeof(lf), &lf);

	GetWindowRect(pcli->hwndContactList,&rcCLUIWindow);
	OffsetRect(&rcCLUIWindow,-rcCLUIWindow.left, -rcCLUIWindow.top);

	GetWindowRect(GetDlgItem(hwndDlg,IDC_PREVIEW),&rcPreviewWindow);
	OffsetRect(&rcPreviewWindow,-rcPreviewWindow.left, -rcPreviewWindow.top);

	_ExtFrames_CalcFramesRect(pList, rcCLUIWindow.right, rcCLUIWindow.bottom, &outRect );
	OffsetRect(&outRect,-outRect.left, -outRect.top);

	if (rcCLUIWindow.right) scaleX=((float)rcPreviewWindow.right/outRect.right);
	if (rcCLUIWindow.bottom) scaleY=((float)rcPreviewWindow.bottom/outRect.bottom);

	Rectangle(hDC,0,0,rcPreviewWindow.right,rcPreviewWindow.bottom);
	for (i=0; i<pList->realCount; i++)
	{
		EXTFRAMEWND * pExtFrm=(EXTFRAMEWND *)pList->items[i];

		if (pExtFrm->efrm.dwFlags&F_VISIBLE && !pExtFrm->efrm.bNotRegistered && !pExtFrm->efrm.bFloat)
		{
			if (pExtFrm->efrm.nType&EFT_VERTICAL)
				lf.lfEscapement=900;
			else
				lf.lfEscapement=0;
			if (pCurrSelected==pExtFrm)
			{
				SelectObject(hDC,GetSysColorBrush(COLOR_HIGHLIGHT));
				SetTextColor(hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor(hDC,GetSysColor(COLOR_HIGHLIGHT));
			} else {
				SelectObject(hDC,GetSysColorBrush(COLOR_WINDOW));
				SetTextColor(hDC,GetSysColor(COLOR_WINDOWTEXT));
				SetBkColor(hDC,GetSysColor(COLOR_WINDOW));
			}
			hNewFont=CreateFontIndirect(&lf);
			SelectObject(hDC,hNewFont);

			outRect=pExtFrm->efrm.rcFrameRect;
			outRect.left=(int)(scaleX*outRect.left);
			outRect.top=(int)(scaleY*outRect.top);
			outRect.right=(int)(scaleX*outRect.right);
			outRect.bottom=(int)(scaleY*outRect.bottom);
			Rectangle(hDC,outRect.left,outRect.top,outRect.right,outRect.bottom);
			{
				char szText[100];
				Num++;
				_snprintf(szText,SIZEOF(szText),"%d. <%s>",Num, Translate(pExtFrm->efrm.szFrameNameID));			
				DrawTextA(hDC,szText,-1,&outRect,DT_VCENTER|DT_CENTER|DT_NOPREFIX|DT_SINGLELINE);
			}
			SelectObject(hDC,GetStockObject(SYSTEM_FONT));
			DeleteObject(hNewFont);
		}
	}

}

static void _ExtFramesOptUtils_UpdateFrameOptControl(HWND hwndDlg, SortedList * pList)
{
	int idx=SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETCURSEL, 0,0);	
	int count=SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETCOUNT, 0,0);
	RECT rcCLUIWindow;
	GetWindowRect(pcli->hwndContactList,&rcCLUIWindow);
	OffsetRect(&rcCLUIWindow,-rcCLUIWindow.left, -rcCLUIWindow.top);

	EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_UP),(idx>0 && idx<count-1));
	EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DOWN),(idx>=0 && idx<count-2));		
	InvalidateRect(GetDlgItem(hwndDlg,IDC_PREVIEW),NULL, TRUE);
}

static void _ExtFramesOptUtils_UpdateNotRegisteredFrames(SortedList * pList, DWORD startNewOrder, EXTFRAMEWND * pFrm1, EXTFRAMEWND * pFrm2)
{	
	int i;
	BOOL afterFrame=FALSE;	
	DWORD newOrder=startNewOrder+1;
	afterFrame=FALSE;
	for (i=0; i<pList->realCount; i++)
	{
		if (afterFrame && pList->items[i]==pFrm2) break;
		if (afterFrame)
		{
			EXTFRAMEWND * pFrm=(EXTFRAMEWND *)pList->items[i];
			if (pFrm->efrm.bNotRegistered)	pFrm->efrm.dwOrder=newOrder++;
			else break;
		}
		if (pList->items[i]==pFrm1) 
			afterFrame=TRUE;
	}

}

static void _ExtFramesOptUtils_ExchangeItems(HWND hwndDlg, SortedList * pList, int fst, int scnd)
{
	int first,second;
	first=min(fst,scnd);
	second=max(fst,scnd);
	if (first>=0 && second>=0 && first!=second)
	{
		EXTFRAMEWND * pFrm1=(EXTFRAMEWND *)SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETITEMDATA, first, 0);
		EXTFRAMEWND * pFrm2=(EXTFRAMEWND *)SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETITEMDATA, second, 0);
		int len=SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETTEXTLEN, second, 0);
		TCHAR * secondText=mir_alloc(len*sizeof(TCHAR)); 
		int idx;
		SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETTEXT, second, (LPARAM)secondText);
		SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_DELETESTRING, second, 0);
		idx=SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_INSERTSTRING, first, (LPARAM)secondText);
		SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_SETITEMDATA, idx, (LPARAM) pFrm2);	
		//TO DO: change order in li.list
		{
			int newFirstOrder=pFrm2->efrm.dwOrder;
			int newSecondOrder=pFrm1->efrm.dwOrder;

			pFrm1->efrm.dwOrder=newFirstOrder;
			pFrm2->efrm.dwOrder=newSecondOrder;
			_ExtFramesOptUtils_UpdateNotRegisteredFrames(pList,newFirstOrder, pFrm1, pFrm2 );
			_ExtFramesOptUtils_UpdateNotRegisteredFrames(pList,newSecondOrder, pFrm2, pFrm1 );
			li_SortList(pList,NULL);
		}
		SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_SETCURSEL, fst, 0);			
		_ExtFramesOptUtils_UpdateFrameOptControl(hwndDlg,pList);		
	}
}

static BOOL CALLBACK _ExtFrames_DlgProcFrameOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{	
	case WM_DRAWITEM:
		if (wParam==IDC_PREVIEW)
		{
			DRAWITEMSTRUCT * lpDrawItem=(DRAWITEMSTRUCT *)lParam;
			_ExtFramesOptUtils_RenderPreview(hwndDlg,extFrmOptList,lpDrawItem->hDC);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			extFrmOptList=li.List_Create(0,1);
			extFrmOptList->sortFunc=_ExtFramesUtils_CopmareFrames;
			_ExtFramesOptUtils_FillListOfFrames(hwndDlg,extFrmOptList);
			_ExtFramesOptUtils_UpdateFrameOptControl(hwndDlg,extFrmOptList);
			return TRUE;
		}	
	case WM_COMMAND:
		{
			if (LOWORD(wParam)==IDC_EXTRAORDER && 
				(HIWORD(wParam)==LBN_SELCHANGE || HIWORD(wParam)==LBN_SELCANCEL ) )
			{
				_ExtFramesOptUtils_UpdateFrameOptControl(hwndDlg, extFrmOptList);
				return TRUE;
			}
			else if (HIWORD(wParam)==BN_CLICKED &&
				(LOWORD(wParam)==IDC_BUTTON_UP || LOWORD(wParam)==IDC_BUTTON_DOWN))
			{
				int idx=SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, LB_GETCURSEL, 0,0);	 
				if (LOWORD(wParam)==IDC_BUTTON_UP)
					_ExtFramesOptUtils_ExchangeItems(hwndDlg, extFrmOptList, idx-1, idx);
				else
					_ExtFramesOptUtils_ExchangeItems(hwndDlg, extFrmOptList, idx+1, idx);
				return TRUE;
			}

			break;
		}
	case WM_DESTROY:
		{
			li_ListDestruct(extFrmOptList,_ExtFrames_DestructorOf_EXTFRAMEWND);
			extFrmOptList=NULL;
		}
		return TRUE;
	case 0:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			{

			}
			return TRUE;
		}
	}
	return FALSE;
}

#endif