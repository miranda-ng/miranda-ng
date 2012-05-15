/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "sessions.h"

HICON hIcon;
HICON hMarked,hNotMarked;

HWND hComboBox=NULL;
HWND hComboBoxEdit=NULL;

WNDPROC oldComboProc=0;

HWND hOpClistControl=NULL;

static BOOL bOptionsInit;
int opses_count;
BOOL bSesssionNameChanged=0;

DWORD session_list_t[255]={0};

HWND g_opHdlg;

int width;
HDC hdc=NULL;
RECT lprect;
RECT rc;
POINT pt;
BOOL bChecked=FALSE;

int OpLoadSessionContacts(WPARAM wparam,LPARAM lparam)
{
	HANDLE hContact;
	int i=0;
	ZeroMemory(session_list_t,SIZEOF(session_list_t));

	for (hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{	
		if(LoadContactsFromMask(hContact,1,lparam))
		{
			i=GetInSessionOrder(hContact,1, lparam);
			session_list_t[i]=(DWORD)hContact;
		}
	}
	
	i=0;
	while(session_list_t[i]!=0)
	{
		SendDlgItemMessage(g_opHdlg, IDC_OPCLIST, LB_ADDSTRING, 0,
			(LPARAM)(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,
			(WPARAM)session_list_t[i], GCDNF_TCHAR));
		i++;
	}
	return i;
}

static LRESULT CALLBACK ComboBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ERASEBKGND:
			return TRUE;
	
		case EM_SETSEL:
			if(!hOpClistControl)
				return HideCaret(hwnd);
			break;
		
		case WM_GETDLGCODE:
			if(!hOpClistControl)
				return DLGC_WANTARROWS;
			break;
			
		case WM_SETCURSOR:
		{
			if(!hOpClistControl)
			{
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				return TRUE;
			}
		}break;

		case WM_LBUTTONDOWN:
			if(hOpClistControl)
				break;
			HideCaret(hwnd);

		case WM_LBUTTONDBLCLK: 
		case WM_MBUTTONDOWN:   
		case WM_MBUTTONDBLCLK:
			SendMessage(hComboBox,CB_SHOWDROPDOWN,1,0);
			return TRUE;

		case WM_NCLBUTTONDBLCLK:
		case WM_NCLBUTTONDOWN:
			if(!bChecked)
			{
				MarkUserDefSession(opses_count,1);
				hIcon=hMarked;
				bChecked=TRUE;
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_FRAME);
			}
			else
			{
				MarkUserDefSession(opses_count,0);
				hIcon=hNotMarked;			
				bChecked=FALSE;
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_FRAME);
			}
			break;

		case WM_MOUSEMOVE:
			if(hOpClistControl)
				break;

		case WM_NCMOUSEMOVE:
			return TRUE;

		case WM_NCPAINT:
		{
			hdc=GetDC(hwnd);
			GetClientRect(hwnd, &rc);
			rc.left=rc.right;
			rc.right=rc.left+16;

			FillRect(hdc, &rc, (HBRUSH)GetSysColor(COLOR_WINDOW));
			DrawIconEx(hdc, rc.left, 0, hIcon, 16, 16, 0, NULL, DI_NORMAL);
			ReleaseDC(hwnd, hdc);
		}break;

		case WM_NCCALCSIZE:
		{
			NCCALCSIZE_PARAMS *ncsParam = (NCCALCSIZE_PARAMS*)lParam;
			ncsParam->rgrc[ 0 ].right -= 16;
		}break;

		case WM_NCHITTEST:
		{
			LRESULT lr = CallWindowProc( oldComboProc, hwnd, msg, wParam, lParam );
			if(lr==HTNOWHERE )
				lr = HTOBJECT;
			return lr;
		}
	}
	return CallWindowProc(oldComboProc, hwnd, msg, wParam, lParam);
}

INT_PTR CALLBACK OptionsProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{	
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			int startupmode,exitmode;
			COMBOBOXINFO cbi={0};
			cbi.cbSize = sizeof(cbi);

			opses_count=0;
			g_opHdlg=hdlg;
			bOptionsInit=TRUE;
			TranslateDialogDefault(hdlg); 
			hMarked=(HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)hibChecked);
			hNotMarked=(HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)hibNotChecked);
			
			hIcon=(bChecked=IsMarkedUserDefSession(opses_count))?hMarked:hNotMarked;

			SetDlgItemInt(hdlg, IDC_TRACK,ses_limit=DBGetContactSettingByte(0, __INTERNAL_NAME, "TrackCount", 10), FALSE);
			SendDlgItemMessage(hdlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(10, 1));
			SendDlgItemMessage(hdlg, IDC_SPIN1, UDM_SETPOS, 0, GetDlgItemInt(hdlg, IDC_TRACK, NULL, FALSE));

			SendDlgItemMessage(hdlg, IDC_OPCLIST, LB_RESETCONTENT, 0, 0);
			SetDlgItemInt(hdlg, IDC_STARTDELAY, DBGetContactSettingWord(NULL, __INTERNAL_NAME, "StartupModeDelay", 1500), FALSE);
			startupmode = DBGetContactSettingByte(NULL, __INTERNAL_NAME, "StartupMode", 3);
			exitmode = DBGetContactSettingByte(NULL, __INTERNAL_NAME, "ShutdownMode", 2);
			
			g_bExclHidden = DBGetContactSettingByte(NULL, __INTERNAL_NAME, "ExclHidden", 0);	
			g_bWarnOnHidden = DBGetContactSettingByte(NULL, __INTERNAL_NAME, "WarnOnHidden", 0);
			g_bOtherWarnings = DBGetContactSettingByte(NULL, __INTERNAL_NAME, "OtherWarnings", 1);
			g_bCrashRecovery = DBGetContactSettingByte(NULL, __INTERNAL_NAME, "CrashRecovery", 0);
			
			CheckDlgButton(hdlg,IDC_EXCLHIDDEN,g_bExclHidden?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_LASTHIDDENWARN,g_bWarnOnHidden?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_WARNINGS,g_bOtherWarnings?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_CRASHRECOVERY,g_bCrashRecovery?BST_CHECKED:BST_UNCHECKED);
			

			if(startupmode==1)
				CheckDlgButton(hdlg,IDC_STARTDIALOG,BST_CHECKED);
			else if (startupmode==3)
			{
				CheckDlgButton(hdlg,IDC_STARTDIALOG,BST_CHECKED);
				CheckDlgButton(hdlg,IDC_CHECKLAST,BST_CHECKED);
			}
			else if (startupmode==2)
			{
				CheckDlgButton(hdlg,IDC_RLOADLAST,BST_CHECKED);
				EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
			}
			else if (startupmode==0)
			{
				CheckDlgButton(hdlg,IDC_RNOTHING,BST_CHECKED);
				EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
			}
			if(exitmode==0)
			{
				CheckDlgButton(hdlg,IDC_REXDSAVE,BST_CHECKED);
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC1),FALSE);
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC2),FALSE);
				EnableWindow(GetDlgItem(hdlg,IDC_TRACK),FALSE);
				EnableWindow(GetDlgItem(hdlg,IDC_SPIN1),FALSE);
			}
			else if (exitmode==1) CheckDlgButton(hdlg,IDC_REXASK,BST_CHECKED);
			else if (exitmode==2) CheckDlgButton(hdlg,IDC_REXSAVE,BST_CHECKED);

			LoadSessionToCombobox (hdlg,1,255,"UserSessionDsc",0);
			if(SendDlgItemMessage(hdlg, IDC_LIST, CB_GETCOUNT, (WPARAM)0, 0))
			{
				EnableWindow(GetDlgItem(hdlg,IDC_EDIT),TRUE);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, (WPARAM)0, 0);
				if(!OpLoadSessionContacts(0,opses_count))
					EnableWindow(GetDlgItem(hdlg,IDC_DEL),FALSE);
			}

			GetComboBoxInfo(GetDlgItem(hdlg,IDC_LIST),&cbi);
			oldComboProc=(WNDPROC)SetWindowLongPtr(cbi.hwndItem, GWLP_WNDPROC, (LONG) ComboBoxSubclassProc);

			hComboBoxEdit=cbi.hwndItem;
			hComboBox=cbi.hwndCombo;

			SetWindowPos( hComboBoxEdit, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );

			bOptionsInit=FALSE;
		}break;

		case WM_CTLCOLORLISTBOX:
		{
			switch(GetDlgCtrlID((HWND) lparam)) 
			{
				case IDC_OPCLIST:
					SetBkMode((HDC) wparam, TRANSPARENT);
					return (BOOL) CreateSolidBrush(GetSysColor(COLOR_3DFACE));
				default:
					break;
			}
		}break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->code)
			{
				case PSN_APPLY:
				{
					int iDelay=GetDlgItemInt(hdlg, IDC_STARTDELAY,NULL, FALSE);
					DBWriteContactSettingWord(0, __INTERNAL_NAME, "StartupModeDelay", (WORD)iDelay);

					DBWriteContactSettingByte(0, __INTERNAL_NAME, "TrackCount", (BYTE)(ses_limit=GetDlgItemInt(hdlg, IDC_TRACK,NULL, FALSE)));
					if(IsDlgButtonChecked(hdlg, IDC_REXSAVE)) DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "ShutdownMode", 2);
					else if(IsDlgButtonChecked(hdlg, IDC_REXDSAVE)) DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "ShutdownMode", 0);
					else if(IsDlgButtonChecked(hdlg, IDC_REXASK)) DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "ShutdownMode", 1);			

					if(IsDlgButtonChecked(hdlg, IDC_STARTDIALOG))
					{ 
						if(!IsDlgButtonChecked(hdlg, IDC_CHECKLAST))
							DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "StartupMode", 1);
						else DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "StartupMode", 3);
					}
					else if(IsDlgButtonChecked(hdlg, IDC_RLOADLAST)) DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "StartupMode", 2);
					else if(IsDlgButtonChecked(hdlg, IDC_RNOTHING)) DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "StartupMode", 0);

					DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "ExclHidden", (BYTE)(IsDlgButtonChecked(hdlg, IDC_EXCLHIDDEN) ? (g_bExclHidden = 1) : (g_bExclHidden = 0)));
					DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "WarnOnHidden", (BYTE)(IsDlgButtonChecked(hdlg, IDC_LASTHIDDENWARN) ? (g_bWarnOnHidden = 1) : (g_bWarnOnHidden = 0)));
					DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "OtherWarnings", (BYTE)(IsDlgButtonChecked(hdlg, IDC_WARNINGS) ? (g_bOtherWarnings = 1) : (g_bOtherWarnings = 0)));
					DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "CrashRecovery", (BYTE)(IsDlgButtonChecked(hdlg, IDC_CRASHRECOVERY) ? (g_bCrashRecovery = 1) : (g_bCrashRecovery = 0)));

					return 1;
				}

				case CLN_CHECKCHANGED:
				{
					if (((LPNMHDR)lparam)->idFrom ==IDC_EMCLIST)
					{
						int iSelection = (int)((NMCLISTCONTROL *)lparam)->hItem;
						HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
						for ( ; hContact; hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
						if (SendDlgItemMessage(hdlg, IDC_EMCLIST, CLM_FINDCONTACT, (WPARAM)hContact, 0) == iSelection)
							break;
						if (hContact)
							EnableWindow(GetDlgItem(hdlg,IDC_SAVE),TRUE);
						else
							EnableWindow(GetDlgItem(hdlg,IDC_SAVE),FALSE);
					}
				}
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case IDC_LIST:
					switch(HIWORD(wparam))
					{
						case CBN_EDITCHANGE:
							EnableWindow(GetDlgItem(hdlg,IDC_SAVE),TRUE);
							bSesssionNameChanged=TRUE;
							break;

						case CBN_SELCHANGE:
						{
							HWND hCombo = GetDlgItem(hdlg, IDC_LIST);
							int index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
							if(index != CB_ERR)
							{
								opses_count = SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)index, 0);
								SendDlgItemMessage(hdlg, IDC_OPCLIST, LB_RESETCONTENT, 0, 0);
								if(IsMarkedUserDefSession(opses_count))
								{
									hIcon=hMarked;
									bChecked=TRUE;
									RedrawWindow(hComboBoxEdit, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
								}
								else
								{
									hIcon=hNotMarked;
									bChecked=FALSE;
									RedrawWindow(hComboBoxEdit, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
								}
								OpLoadSessionContacts(0,opses_count);
								if(!hOpClistControl)
									EnableWindow(GetDlgItem(hdlg,IDC_DEL),TRUE);
								else
								{
									int i;
									HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

									for ( ; hContact; hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
										SendMessage(hOpClistControl, CLM_SETCHECKMARK, (WPARAM)hContact,0);
									for (i=0 ; session_list_t[i]>0; i++)
									{
										hContact=(HANDLE)SendMessage(hOpClistControl,CLM_FINDCONTACT, (WPARAM)session_list_t[i], 0);
										//hItem=session_list[i];
										SendMessage(hOpClistControl, CLM_SETCHECKMARK, (WPARAM)hContact,1);
									}
									EnableWindow(GetDlgItem(hdlg,IDC_SAVE),FALSE);
								}
							}
						}break;
					}break;

				case IDC_EDIT:
				{
					if(!hOpClistControl)
					{
						int i;
						HANDLE hItem;
						ShowWindow(GetDlgItem(hdlg,IDC_OPCLIST),SW_HIDE);
						EnableWindow(GetDlgItem(hdlg,IDC_DEL),FALSE);
						//EnableWindow(GetDlgItem(hdlg,IDC_SAVE),TRUE);
						SetWindowText(GetDlgItem(hdlg,IDC_EDIT),TranslateT("View"));
						hOpClistControl = CreateWindowEx(WS_EX_STATICEDGE,CLISTCONTROL_CLASS, _T(""), 
							WS_TABSTOP |WS_VISIBLE | WS_CHILD , 
							14,198,161,163,hdlg, (HMENU)IDC_EMCLIST, hinstance, 0);

						SetWindowLongPtr(hOpClistControl, GWL_STYLE,
							GetWindowLongPtr(hOpClistControl, GWL_STYLE)|CLS_CHECKBOXES|CLS_HIDEEMPTYGROUPS|CLS_USEGROUPS|CLS_GREYALTERNATE|CLS_GROUPCHECKBOXES);
						SendMessage(hOpClistControl, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP|CLS_EX_TRACKSELECT, 0);

						SendMessage(hOpClistControl,WM_TIMER,TIMERID_REBUILDAFTER,0);

						for (i=0 ; session_list_t[i]>0; i++)
						{
							hItem=(HANDLE)SendMessage(hOpClistControl,CLM_FINDCONTACT, (WPARAM)session_list_t[i], 0);
							//hItem=session_list[i];
							SendMessage(hOpClistControl, CLM_SETCHECKMARK, (WPARAM)hItem,1);
						}		
					}
					else
					{
						ShowWindow(GetDlgItem(hdlg,IDC_OPCLIST),SW_SHOWNA);
						EnableWindow(GetDlgItem(hdlg,IDC_DEL),TRUE);
						EnableWindow(GetDlgItem(hdlg,IDC_SAVE),FALSE);
						SetWindowText(GetDlgItem(hdlg,IDC_EDIT),TranslateT("Edit"));
						DestroyWindow(hOpClistControl);
						hOpClistControl=NULL;
					}
				}break;

			case IDC_SAVE:
			{
				int i;
				HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
				for (i=0 ; hContact;hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
				{
					BYTE res =(BYTE)SendMessage(GetDlgItem(hdlg,IDC_EMCLIST), CLM_GETCHECKMARK, 
						SendMessage(GetDlgItem(hdlg,IDC_EMCLIST), CLM_FINDCONTACT, (WPARAM)hContact, 0), 0);
					if (res)
					{
						SetSessionMark(hContact,1,'1',opses_count);
						SetInSessionOrder(hContact,1,opses_count,i);
						i++;
					}
					else
					{
						SetSessionMark(hContact,1,'0',opses_count);
						SetInSessionOrder(hContact,1,opses_count,0);
					}
				}
				if(bSesssionNameChanged)
				{
					if(GetWindowTextLength(hComboBoxEdit))
					{
						TCHAR szUserSessionName[MAX_PATH]={'\0'};
						GetWindowText(hComboBoxEdit, szUserSessionName, SIZEOF(szUserSessionName));
						RenameUserDefSession(opses_count,szUserSessionName);
						SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT ,0,0);
						LoadSessionToCombobox (hdlg,1,255,"UserSessionDsc",0);
					}
					bSesssionNameChanged=FALSE;
				}
				EnableWindow(GetDlgItem(hdlg,IDC_SAVE),FALSE);
			}break;

			case IDC_DEL:
			{
				int i=0,index=0;
				DelUserDefSession(opses_count);
						
				SendDlgItemMessage(hdlg, IDC_OPCLIST, LB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);

				LoadSessionToCombobox (hdlg,1,255,"UserSessionDsc",0);

				opses_count=0;

				if(SendDlgItemMessage(hdlg, IDC_LIST, CB_GETCOUNT, (WPARAM)0, 0))
				{
					EnableWindow(GetDlgItem(hdlg,IDC_EDIT),TRUE);
					SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, (WPARAM)0, 0);
					if(!OpLoadSessionContacts(0,opses_count))
						EnableWindow(GetDlgItem(hdlg,IDC_DEL),FALSE);
				}
				else
				{
					EnableWindow(GetDlgItem(hdlg,IDC_EDIT),FALSE);
					EnableWindow(GetDlgItem(hdlg,IDC_DEL),FALSE);
				}
			}break;

			case IDC_STARTDIALOG:
			{
				EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), TRUE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), TRUE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), TRUE);	
				EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), TRUE);
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			}break;
				
			case IDC_RLOADLAST:
			{
				EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), TRUE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), TRUE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), TRUE);				
				EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			}break;
				
			case IDC_RNOTHING:
			{
				EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			}break;
				
			case IDC_REXSAVE:
			{
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC1),TRUE);
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC2),TRUE);
				EnableWindow(GetDlgItem(hdlg,IDC_TRACK),TRUE);	
				EnableWindow(GetDlgItem(hdlg,IDC_SPIN1),TRUE);
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			}break;

			case IDC_REXDSAVE:
			{
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC1),FALSE);
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC2),FALSE);
				EnableWindow(GetDlgItem(hdlg,IDC_TRACK),FALSE);
				EnableWindow(GetDlgItem(hdlg,IDC_SPIN1),FALSE);	
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			}break;

			case IDC_REXASK:
			{
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC1),TRUE);
				EnableWindow(GetDlgItem(hdlg,IDC_EXSTATIC2),TRUE);
				EnableWindow(GetDlgItem(hdlg,IDC_TRACK),TRUE);				
				EnableWindow(GetDlgItem(hdlg,IDC_SPIN1),TRUE);
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			}break;
		}

		if (HIWORD(wparam)==CBN_DROPDOWN&&!hOpClistControl)
		{
			SendMessage(hComboBoxEdit,EM_SETSEL ,0,0);
			SendMessage(hComboBoxEdit,EM_SCROLLCARET ,0,0);
			SendMessage(hComboBoxEdit,WM_KILLFOCUS ,0,0);
			HideCaret(hComboBoxEdit);
		}

		if((HIWORD(wparam)!=CBN_DROPDOWN)&&(LOWORD(wparam)==IDC_LIST)&&!hOpClistControl)
		{
			SendMessage(hComboBoxEdit,EM_SCROLLCARET ,0,0);
			HideCaret(hComboBoxEdit);
		}
 
		if ((LOWORD(wparam) == IDC_STARTDELAY) && (HIWORD(wparam)!=EN_CHANGE || (HWND)lparam != GetFocus()))
			return 0;

   		if (lparam&&!bOptionsInit&&(HIWORD(wparam)==BN_CLICKED)&& (GetFocus()==(HWND)lparam)
			&&((LOWORD(wparam)==IDC_CHECKLAST)||((LOWORD(wparam)>=IDC_EXCLHIDDEN)&&(LOWORD(wparam)<=IDC_CRASHRECOVERY))))
 			SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
		
		return 0;

		case WM_NCDESTROY:
			SetWindowLongPtr(hComboBoxEdit, GWLP_WNDPROC, (LONG)oldComboProc);
			break;

		case WM_CLOSE:
			EndDialog(hdlg,0);
			return 0;
	}
	return 0;
}

int OptionsInit(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp={0};

	odp.cbSize = sizeof(odp);
	odp.position=955000000;
	odp.hInstance=hinstance;
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszTitle=LPGENT(__INTERNAL_NAME);
	odp.pfnDlgProc=OptionsProc;
	odp.ptszGroup=LPGENT("Message Sessions");
	odp.flags=ODPF_BOLDGROUPS | ODPF_TCHAR;

	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);
	return 0;
}