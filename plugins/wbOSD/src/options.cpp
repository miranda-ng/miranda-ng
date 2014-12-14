/*
Wannabe OSD
This plugin tries to become miranda's standard OSD ;-)

(C) 2005 Andrej Krutak

Distributed under GNU's GPL 2 or later
*/

#include "wbOSD.h"

COLORREF pencustcolors[16];

const static osdmsg defstr={_T(""), 0, RGB(0, 0, 0), 0, 0};

void FillCheckBoxTree(HWND hwndTree,DWORD style)
{
	logmsg("FillCheckBoxTree");

	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
	for ( WORD status = ID_STATUS_OFFLINE; status <=ID_STATUS_OUTTOLUNCH; status++ ) {
		tvis.item.lParam = status - ID_STATUS_OFFLINE;
		tvis.item.pszText = (TCHAR*) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM) status,GSMDF_TCHAR);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK(( style & ( 1 << tvis.item.lParam )) != 0 ? 2 : 1 );
		TreeView_InsertItem( hwndTree, &tvis );
	}
}

DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags=0;

	logmsg("MakeCheckBoxTreeFlags");

	TVITEM tvi = {0};
	tvi.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_STATE;
	tvi.hItem=TreeView_GetRoot(hwndTree);
	while(tvi.hItem) {
		TreeView_GetItem(hwndTree,&tvi);
		if(((tvi.state&TVIS_STATEIMAGEMASK)>>12==2)) flags|=1<<tvi.lParam;
		tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
	}
	return flags;
}

int selectColor(HWND hwnd, COLORREF *clr)
{
	logmsg("SelectColor");

	CHOOSECOLOR cc = {0};
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hwnd;
	cc.hInstance = (HWND)hI;
	cc.rgbResult = *clr;
	cc.lpCustColors = pencustcolors;
	cc.Flags = CC_FULLOPEN|CC_RGBINIT;
	if (!ChooseColor(&cc))
		return 1;
	
	*clr=cc.rgbResult;
	return 0;
}

int selectFont(HWND hDlg, LOGFONT *lf)
{
	COLORREF color=RGB(0, 0, 0);

	logmsg("SelectFont");

	HDC hDC = GetDC(hDlg);

	CHOOSEFONT cf;
	memset(&cf, 0, sizeof(CHOOSEFONT));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hDlg;
	cf.hDC = hDC;
	cf.lpLogFont = lf;
	cf.rgbColors = 0;
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_BOTH | CF_FORCEFONTEXIST;
	cf.nFontType = 0;
	cf.rgbColors=color;
	
	if (!ChooseFont(&cf)) {
		if (cf.hDC)
			DeleteDC(cf.hDC);
		
		ReleaseDC(hDlg, hDC);
		return 1;
	}
	
	if (cf.hDC)
		DeleteDC(cf.hDC);
	
	ReleaseDC(hDlg, hDC);
	return 0;
}

void loadDBSettings(plgsettings *ps)
{
	logmsg("loadDBSettings");

	ps->align=db_get_b(NULL,THIS_MODULE, "align", DEFAULT_ALIGN);
	ps->salign=db_get_b(NULL,THIS_MODULE, "salign", DEFAULT_SALIGN);
	ps->altShadow=db_get_b(NULL,THIS_MODULE, "altShadow", DEFAULT_ALTSHADOW);
	ps->transparent=db_get_b(NULL,THIS_MODULE, "transparent", DEFAULT_TRANPARENT);
	ps->showShadow=db_get_b(NULL,THIS_MODULE, "showShadow", DEFAULT_SHOWSHADOW);
	ps->messages=db_get_b(NULL,THIS_MODULE, "messages", DEFAULT_ANNOUNCEMESSAGES);
	ps->a_user=db_get_b(NULL,THIS_MODULE, "a_user", DEFAULT_ANNOUNCESTATUS);
	ps->distance=db_get_b(NULL,THIS_MODULE, "distance", DEFAULT_DISTANCE);
	ps->winx=db_get_dw(NULL,THIS_MODULE, "winx", DEFAULT_WINX);
	ps->winy=db_get_dw(NULL,THIS_MODULE, "winy", DEFAULT_WINY);
	ps->winxpos=db_get_dw(NULL,THIS_MODULE, "winxpos", DEFAULT_WINXPOS);
	ps->winypos=db_get_dw(NULL,THIS_MODULE, "winypos", DEFAULT_WINYPOS);
	ps->alpha=db_get_b(NULL,THIS_MODULE, "alpha", DEFAULT_ALPHA);
	ps->showmystatus=db_get_b(NULL,THIS_MODULE, "showMyStatus", DEFAULT_SHOWMYSTATUS);
	ps->timeout=db_get_dw(NULL,THIS_MODULE, "timeout", DEFAULT_TIMEOUT);
	ps->clr_msg=db_get_dw(NULL,THIS_MODULE, "clr_msg", DEFAULT_CLRMSG);
	ps->clr_status=db_get_dw(NULL,THIS_MODULE, "clr_status", DEFAULT_CLRSTATUS);
	ps->clr_shadow=db_get_dw(NULL,THIS_MODULE, "clr_shadow", DEFAULT_CLRSHADOW);
	ps->bkclr=db_get_dw(NULL,THIS_MODULE, "bkclr", DEFAULT_BKCLR);

	ps->showMsgWindow=db_get_b(NULL,THIS_MODULE, "showMessageWindow", DEFAULT_SHOWMSGWIN);
	ps->showWhen=db_get_dw(NULL,THIS_MODULE,"showWhen", DEFAULT_SHOWWHEN);
	
	DBVARIANT dbv;
	if (!db_get_ts( NULL, THIS_MODULE, "message_format", &dbv )) {
		_tcscpy(ps->msgformat, dbv.ptszVal);
		db_free(&dbv);
	}
	else _tcscpy(ps->msgformat, DEFAULT_MESSAGEFORMAT);

	ps->announce=db_get_dw(NULL,THIS_MODULE,"announce", DEFAULT_ANNOUNCE);

	ps->lf.lfHeight=db_get_dw(NULL,THIS_MODULE, "fntHeight", DEFAULT_FNT_HEIGHT);
	ps->lf.lfWidth=db_get_dw(NULL,THIS_MODULE, "fntWidth", DEFAULT_FNT_WIDTH);
	ps->lf.lfEscapement=db_get_dw(NULL,THIS_MODULE, "fntEscapement", DEFAULT_FNT_ESCAPEMENT);
	ps->lf.lfOrientation=db_get_dw(NULL,THIS_MODULE, "fntOrientation", DEFAULT_FNT_ORIENTATION);
	ps->lf.lfWeight=db_get_dw(NULL,THIS_MODULE, "fntWeight", DEFAULT_FNT_WEIGHT);
	ps->lf.lfItalic=db_get_b(NULL,THIS_MODULE, "fntItalic", DEFAULT_FNT_ITALIC);
	ps->lf.lfUnderline=db_get_b(NULL,THIS_MODULE, "fntUnderline", DEFAULT_FNT_UNDERLINE);
	ps->lf.lfStrikeOut=db_get_b(NULL,THIS_MODULE, "fntStrikeout", DEFAULT_FNT_STRIKEOUT);
	ps->lf.lfCharSet=db_get_b(NULL,THIS_MODULE, "fntCharSet", DEFAULT_FNT_CHARSET);
	ps->lf.lfOutPrecision=db_get_b(NULL,THIS_MODULE, "fntOutPrecision", DEFAULT_FNT_OUTPRECISION);
	ps->lf.lfClipPrecision=db_get_b(NULL,THIS_MODULE, "fntClipPrecision", DEFAULT_FNT_CLIPRECISION);
	ps->lf.lfQuality=db_get_b(NULL,THIS_MODULE, "fntQuality", DEFAULT_FNT_QUALITY);
	ps->lf.lfPitchAndFamily=db_get_b(NULL,THIS_MODULE, "fntPitchAndFamily", DEFAULT_FNT_PITCHANDFAM);
	
	if(!db_get_ts(NULL,THIS_MODULE,"fntFaceName",&dbv)) {
		_tcscpy(ps->lf.lfFaceName, dbv.ptszVal);
		db_free(&dbv);
	}
	else
		_tcscpy(ps->lf.lfFaceName, DEFAULT_FNT_FACENAME);
}

void saveDBSettings(plgsettings *ps)
{
	logmsg("saveDBSettings");

	db_set_b(NULL,THIS_MODULE,"showShadow", ps->showShadow);
	db_set_b(NULL,THIS_MODULE,"altShadow",ps->altShadow);
	db_set_b(NULL,THIS_MODULE,"distance",ps->distance);
	
	db_set_dw(NULL,THIS_MODULE,"winx",ps->winx);
	db_set_dw(NULL,THIS_MODULE,"winy",ps->winy);
	db_set_dw(NULL,THIS_MODULE,"winxpos", ps->winxpos);
	db_set_dw(NULL,THIS_MODULE,"winypos", ps->winypos);
	
	db_set_b(NULL,THIS_MODULE,"alpha",ps->alpha);
	db_set_dw(NULL,THIS_MODULE,"timeout", ps->timeout);

	db_set_b(NULL,THIS_MODULE,"transparent",ps->transparent); 
	db_set_b(NULL,THIS_MODULE,"messages",ps->messages); 
	db_set_b(NULL,THIS_MODULE,"a_user",ps->a_user); 
	db_set_ts(NULL,THIS_MODULE, "message_format", ps->msgformat);

	db_set_b(NULL,THIS_MODULE,"align",ps->align); 
	db_set_b(NULL,THIS_MODULE,"salign",ps->salign); 

	db_set_b(NULL,THIS_MODULE,"showMyStatus",ps->showmystatus); 

	db_set_dw(NULL,THIS_MODULE,"clr_msg", ps->clr_msg); 
	db_set_dw(NULL,THIS_MODULE,"clr_shadow", ps->clr_shadow); 
	db_set_dw(NULL,THIS_MODULE,"clr_status", ps->clr_status); 
	db_set_dw(NULL,THIS_MODULE,"bkclr", ps->bkclr); 

	db_set_dw(NULL,THIS_MODULE, "fntHeight", ps->lf.lfHeight);
	db_set_dw(NULL,THIS_MODULE, "fntWidth", ps->lf.lfWidth);
	db_set_dw(NULL,THIS_MODULE, "fntEscapement", ps->lf.lfEscapement);
	db_set_dw(NULL,THIS_MODULE, "fntOrientation", ps->lf.lfOrientation);
	db_set_dw(NULL,THIS_MODULE, "fntWeight", ps->lf.lfWeight);
	db_set_b(NULL,THIS_MODULE, "fntItalic", ps->lf.lfItalic);
	db_set_b(NULL,THIS_MODULE, "fntUnderline", ps->lf.lfUnderline);
	db_set_b(NULL,THIS_MODULE, "fntStrikeout", ps->lf.lfStrikeOut);
	db_set_b(NULL,THIS_MODULE, "fntCharSet", ps->lf.lfCharSet);
	db_set_b(NULL,THIS_MODULE, "fntOutPrecision", ps->lf.lfOutPrecision);
	db_set_b(NULL,THIS_MODULE, "fntClipPrecision", ps->lf.lfClipPrecision);
	db_set_b(NULL,THIS_MODULE, "fntQuality", ps->lf.lfQuality);
	db_set_b(NULL,THIS_MODULE, "fntPitchAndFamily", ps->lf.lfPitchAndFamily);
	db_set_ts(NULL,THIS_MODULE, "fntFaceName", ps->lf.lfFaceName);
	
	db_set_dw(NULL,THIS_MODULE,"announce", ps->announce);

	db_set_b(NULL,THIS_MODULE, "showMessageWindow", ps->showMsgWindow);
	db_set_dw(NULL,THIS_MODULE,"showWhen", ps->showWhen);
}

INT_PTR CALLBACK OptDlgProc(HWND hDlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	RECT rect;
	plgsettings *ps; //0: current; 1: original

	logmsg("OptDlgProc");

	switch(msg){
		case WM_INITDIALOG:
			logmsg("OptDlgProc::INITDIALOG");
			TranslateDialogDefault(hDlg);

			ps=(plgsettings*)malloc(sizeof(plgsettings)*2);
			loadDBSettings(&ps[0]);
			ps[1]=ps[0];
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ps);
			SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_SIZEBOX);
			SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_FRAMECHANGED);

			SetWindowLongPtr(GetDlgItem(hDlg,IDC_TREE1),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hDlg,IDC_TREE1),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
			SetWindowLongPtr(GetDlgItem(hDlg,IDC_TREE2),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hDlg,IDC_TREE1),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);

			CheckDlgButton(hDlg, IDC_RADIO1+ps->align-1, BST_CHECKED);
			CheckDlgButton(hDlg, IDC_RADIO10+9-ps->salign, BST_CHECKED);
			CheckDlgButton(hDlg, IDC_CHECK1, ps->altShadow ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK2, ps->showMsgWindow ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK3, ps->transparent ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK4, ps->showShadow ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK5, ps->messages ? BST_CHECKED : BST_UNCHECKED);
			
			SetDlgItemText(hDlg, IDC_EDIT2, ps->msgformat);

			CheckDlgButton(hDlg, IDC_CHECK6, ps->a_user ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK7, ps->showmystatus ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemInt(hDlg, IDC_EDIT1, ps->distance, 0);

			SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETRANGE, 0, MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETPOS, TRUE, (LPARAM)ps->alpha);

			{
				TCHAR buf[20];
				mir_sntprintf(buf, SIZEOF(buf), _T("%d %%"), ps->alpha*100/255);
				SetDlgItemText(hDlg, IDC_ALPHATXT, buf);
			}

			SetDlgItemInt(hDlg, IDC_EDIT5, ps->timeout, 0);
			FillCheckBoxTree(GetDlgItem(hDlg, IDC_TREE1), ps->announce);
			FillCheckBoxTree(GetDlgItem(hDlg, IDC_TREE2), ps->showWhen);
			return 0;

		case WM_HSCROLL:
			if (LOWORD(wparam)==SB_ENDSCROLL||LOWORD(wparam)==SB_THUMBPOSITION||LOWORD(wparam)==SB_ENDSCROLL)
				return 0;
			ps=(plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			ps->alpha=SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0);
			{
				TCHAR buf[20];
				mir_sntprintf(buf, SIZEOF(buf), _T("%d %%"), ps->alpha*100/255);
				SetDlgItemText(hDlg, IDC_ALPHATXT, buf);
			}
			goto xxx;
		case WM_DESTROY:
			logmsg("OptDlgProc::DESTROY");
			ps=(plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			ps[0]=ps[1];
			saveDBSettings(&ps[0]);

			SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
			SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_FRAMECHANGED);

			SetWindowPos(hwnd, 0, ps->winxpos, ps->winypos, ps->winx, ps->winy, SWP_NOZORDER|SWP_NOACTIVATE);
			SetLayeredWindowAttributes(hwnd, ps->bkclr, ps->alpha, (ps->transparent?LWA_COLORKEY:0)|LWA_ALPHA);

			free((void*)GetWindowLongPtr(hDlg, GWLP_USERDATA));
			return 0;
		case WM_COMMAND:
			logmsg("OptDlgProc::COMMAND");
			ps=(plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			switch (LOWORD(wparam)) {
			case IDC_BUTTON7:
				MessageBox(hDlg, TranslateT("Variables:\n  %n : Nick\n  %m : Message\n  %l : New line"), TranslateT("Help"), MB_ICONINFORMATION|MB_OK);
				return 0;
			case IDC_BUTTON5:
				SendMessage(hwnd, WM_USER+1, (WPARAM)TranslateT("Miranda NG is great and this is a long message."), 0);
				break;
			case IDC_BUTTON1:
				selectFont(hDlg, &(ps->lf));
				break;
			case IDC_BUTTON2:
				selectColor(hDlg, &ps->clr_status);
				break;
			case IDC_BUTTON6:
				selectColor(hDlg, &ps->clr_msg);
				break;
			case IDC_BUTTON3:
				selectColor(hDlg, &ps->clr_shadow);
				break;
			case IDC_BUTTON4:
				selectColor(hDlg, &ps->bkclr);
				break;
			case IDC_CHECK4:
				ps->showShadow=IsDlgButtonChecked(hDlg, IDC_CHECK4);
				break;
			case IDC_CHECK1:
				ps->altShadow=IsDlgButtonChecked(hDlg, IDC_CHECK1);
				break;
			case IDC_CHECK2:
				ps->showMsgWindow=IsDlgButtonChecked(hDlg, IDC_CHECK2);
			case IDC_EDIT1:
				ps->distance=GetDlgItemInt(hDlg, IDC_EDIT1, 0, 0);
				break;
			case IDC_EDIT5:
				ps->timeout=GetDlgItemInt(hDlg, IDC_EDIT5, 0, 0);
				break;
			case IDC_CHECK3:
				ps->transparent=IsDlgButtonChecked(hDlg, IDC_CHECK3);
				break;
			case IDC_CHECK5:
				ps->messages=IsDlgButtonChecked(hDlg, IDC_CHECK5);
				break;
			case IDC_CHECK6:
				ps->a_user=IsDlgButtonChecked(hDlg, IDC_CHECK6);
				break;
			case IDC_CHECK7:
				ps->showmystatus=IsDlgButtonChecked(hDlg, IDC_CHECK7);
				break;
			case IDC_RADIO1:
			case IDC_RADIO2:
			case IDC_RADIO3:
			case IDC_RADIO4:
			case IDC_RADIO5:
			case IDC_RADIO6:
			case IDC_RADIO7:
			case IDC_RADIO8:
			case IDC_RADIO9:
				if (IsDlgButtonChecked(hDlg, LOWORD(wparam)))
					ps->align=LOWORD(wparam)-IDC_RADIO1+1;
				break;
			case IDC_RADIO10:
			case IDC_RADIO11:
			case IDC_RADIO12:
			case IDC_RADIO13:
			case IDC_RADIO14:
			case IDC_RADIO15:
			case IDC_RADIO16:
			case IDC_RADIO17:
			case IDC_RADIO18:
				if (IsDlgButtonChecked(hDlg, LOWORD(wparam)))
					ps->salign=10-(LOWORD(wparam)-IDC_RADIO10+1);
				break;
			}
xxx:
			saveDBSettings(ps);
			SetWindowPos(hwnd, 0, 0, 0, ps->winx, ps->winy, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
			SetLayeredWindowAttributes(hwnd, db_get_dw(NULL,THIS_MODULE, "bkclr", DEFAULT_BKCLR), db_get_b(NULL,THIS_MODULE, "alpha", DEFAULT_ALPHA), (db_get_b(NULL,THIS_MODULE, "transparent", DEFAULT_TRANPARENT)?LWA_COLORKEY:0)|LWA_ALPHA);
			InvalidateRect(hwnd, 0, TRUE);
			SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);

			return 0;

		case WM_NOTIFY:
			logmsg("OptDlgProc::NOTIFY");
			switch(((LPNMHDR)lparam)->code){
				case TVN_SETDISPINFO:
				case NM_CLICK:
				case NM_RETURN:
				case TVN_SELCHANGED:
					if (((LPNMHDR)lparam)->idFrom==IDC_TREE1)
						SendMessage(GetParent(hDlg),PSM_CHANGED,0,0);
					break;
				case PSN_APPLY:
					ps=(plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
					
					GetWindowRect(hwnd, &rect);
					ps->winx=rect.right-rect.left;
					ps->winy=rect.bottom-rect.top;
					ps->winxpos=rect.left;
					ps->winypos=rect.top;
					ps->announce=MakeCheckBoxTreeFlags(GetDlgItem(hDlg, IDC_TREE1));
					ps->showWhen=MakeCheckBoxTreeFlags(GetDlgItem(hDlg, IDC_TREE2));
					GetDlgItemText(hDlg, IDC_EDIT2, ps->msgformat, 255);
					ps[1]=ps[0]; //apply current settings at closing

					saveDBSettings(ps);
					SetLayeredWindowAttributes(hwnd, db_get_dw(NULL,THIS_MODULE, "bkclr", DEFAULT_BKCLR), db_get_b(NULL,THIS_MODULE, "alpha", DEFAULT_ALPHA), (db_get_b(NULL,THIS_MODULE, "transparent", DEFAULT_TRANPARENT)?LWA_COLORKEY:0)|LWA_ALPHA);
					InvalidateRect(hwnd, 0, TRUE);
					break;
			}
			break;
	}

	return 0;
}

int OptionsInit(WPARAM wparam,LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position=150000000;
	odp.groupPosition=950000000;
	odp.hInstance=hI;
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_DIALOG1);
	odp.ptszGroup=LPGENT("Plugins");
	odp.ptszTitle=LPGENT("OSD");
	odp.pfnDlgProc=OptDlgProc;
	odp.flags=ODPF_BOLDGROUPS|ODPF_TCHAR;
	Options_AddPage(wparam,&odp);
	return 0;
}
