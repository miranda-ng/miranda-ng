/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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


#include "dlg_handlers.h"

#define MIN_EMAILS_WIDTH 300
#define MIN_EMAILS_HEIGHT 250

static WNDPROC OldListProc;

BOOL CALLBACK DlgProcOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing; //true when dialog is being created
	
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					TCHAR buffer[4096];
					char apassword[1024];
					long port;
					long interval;
					int bCheck;
					int portCheck;
					int retries;
					bInitializing = 1;
					DBVARIANT dbv = {0};
					
					
					dbv.type = DBVT_ASCIIZ;
					
					TranslateDialogDefault(hWnd);
					
					bCheck = db_get_b(NULL, ModuleName, "Check", 1);
					GetStringFromDatabase("Username", _T(""), buffer, sizeof(buffer));
					SetWindowText(GetDlgItem(hWnd, IDC_USER_EDIT), buffer);
					GetStringFromDatabase("Password", _T(""), buffer, sizeof(buffer));
					strcpy(apassword,mir_t2a(buffer));
					CallService(MS_DB_CRYPT_DECODESTRING, sizeof(apassword), (LPARAM) apassword);
					SetWindowText(GetDlgItem(hWnd, IDC_PASSWORD_EDIT), mir_a2t(apassword));
					GetStringFromDatabase("Server", _T(""), buffer, sizeof(buffer));
					SetWindowText(GetDlgItem(hWnd, IDC_SERVER_EDIT), buffer);
					
					port = db_get_dw(NULL, ModuleName, "Port", EXCHANGE_PORT);
					//_itoa(port, buffer, 10);
					//SetWindowText(GetDlgItem(hWnd, IDC_PORT_EDIT), buffer);
					SetDlgItemInt(hWnd,IDC_PORT_EDIT,port,FALSE);
					
					interval = db_get_dw(NULL, ModuleName, "Interval", DEFAULT_INTERVAL);
					//_itoa(interval, buffer, 10);
					//SetWindowText(GetDlgItem(hWnd, IDC_INTERVAL_EDIT), buffer);
					SetDlgItemInt(hWnd,IDC_INTERVAL_EDIT,interval,FALSE);

					CheckDlgButton(hWnd, IDC_RECONNECT, (db_get_b(NULL, ModuleName, "Reconnect", 0)) ? BST_CHECKED : BST_UNCHECKED);
					
					interval = db_get_dw(NULL, ModuleName, "ReconnectInterval", DEFAULT_RECONNECT_INTERVAL);
					//_itoa(interval, buffer, 10);
					//SetWindowText(GetDlgItem(hWnd, IDC_RECONNECT_INTERVAL), buffer);
					SetDlgItemInt(hWnd,IDC_RECONNECT_INTERVAL,interval,FALSE);
					CheckDlgButton(hWnd, IDC_USE_POPUPS, (BOOL) db_get_b(NULL, ModuleName, "UsePopups", 0) ? BST_CHECKED : BST_UNCHECKED);
					EnableWindow(GetDlgItem(hWnd, IDC_USE_POPUPS), ServiceExists(MS_POPUP_ADDPOPUP)); //disable the popups checkbox if no popup module is present
					
					CheckDlgButton(hWnd, IDC_CHECK_EMAILS, (bCheck) ? BST_CHECKED : BST_UNCHECKED);
					EnableWindow(GetDlgItem(hWnd, IDC_INTERVAL_EDIT), bCheck);
					
					portCheck = (BOOL) db_get_b(NULL, ModuleName, "UsePortCheck", 1);
					CheckDlgButton(hWnd, IDC_USE_PORTCHECK, (portCheck) ? BST_CHECKED : BST_UNCHECKED);
					EnableWindow(GetDlgItem(hWnd, IDC_PORT_EDIT), portCheck);
					
					//_itoa(db_get_b(NULL, ModuleName, "MaxRetries", MAX_EXCHANGE_CONNECT_RETRIES), buffer, 10);
					//SetWindowText(GetDlgItem(hWnd, IDC_MAX_RETRIES), buffer);
					retries=db_get_b(NULL, ModuleName, "MaxRetries", MAX_EXCHANGE_CONNECT_RETRIES);
					SetDlgItemInt(hWnd,IDC_MAX_RETRIES,retries,FALSE);

					EnableWindow(GetDlgItem(hWnd, IDC_RECONNECT_INTERVAL), IsDlgButtonChecked(hWnd, IDC_RECONNECT));

					bInitializing = 0;
					
					return TRUE;
				}
				
			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDC_USER_EDIT:
							case IDC_PASSWORD_EDIT:
							case IDC_SERVER_EDIT:
							case IDC_PORT_EDIT:
							case IDC_INTERVAL_EDIT:
							case IDC_RECONNECT_INTERVAL:
							case IDC_MAX_RETRIES:
								{
									if ((!bInitializing) && (HIWORD(wParam) == EN_CHANGE))// || (HIWORD(wParam) == CBN_SELENDOK))
										{
											SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
										}
										
									break;
								}
							case IDC_USE_POPUPS:
							case IDC_CHECK_EMAILS:
							case IDC_RECONNECT:
							case IDC_USE_PORTCHECK:
								{
									int portCheck = IsDlgButtonChecked(hWnd, IDC_USE_PORTCHECK);
									SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
									
									EnableWindow(GetDlgItem(hWnd, IDC_INTERVAL_EDIT), IsDlgButtonChecked(hWnd, IDC_CHECK_EMAILS));
									EnableWindow(GetDlgItem(hWnd, IDC_RECONNECT_INTERVAL), IsDlgButtonChecked(hWnd, IDC_RECONNECT));
									EnableWindow(GetDlgItem(hWnd, IDC_PORT_EDIT), portCheck);
									//EnableWindow(GetDlgItem(hWnd, IDC_MAX_RETRIES), portCheck);
									
									break;
								}
						}
						
					break;
				}
				
		case WM_NOTIFY:
			{
				switch(((LPNMHDR)lParam)->idFrom)
					{
						case 0:
							{
								switch (((LPNMHDR)lParam)->code)
									{
										case PSN_APPLY:
											{
												TCHAR buffer[4096];
												char apassword[1024];
												long port = 0;
												long interval = DEFAULT_INTERVAL;
												int bCheck = IsDlgButtonChecked(hWnd, IDC_CHECK_EMAILS);
												int retries = MAX_EXCHANGE_CONNECT_RETRIES;
												
												db_set_b(NULL, ModuleName, "Check", bCheck);
												GetWindowText(GetDlgItem(hWnd, IDC_USER_EDIT), buffer, sizeof(buffer));
												db_set_ts(NULL, ModuleName, "Username", buffer);
												GetWindowText(GetDlgItem(hWnd, IDC_PASSWORD_EDIT), buffer, sizeof(buffer));
												strcpy(apassword,mir_t2a(buffer));

												CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(apassword), (LPARAM) apassword);

												db_set_ts(NULL, ModuleName, "Password", mir_a2t(apassword));

												GetWindowText(GetDlgItem(hWnd, IDC_SERVER_EDIT), buffer, sizeof(buffer));
												db_set_ts(NULL, ModuleName, "Server", buffer);
												GetWindowText(GetDlgItem(hWnd, IDC_PORT_EDIT), buffer, sizeof(buffer));
												//port = atoi(buffer);
												//db_set_dw(NULL, ModuleName, "Port", port);
												db_set_dw(NULL, ModuleName, "Port", GetDlgItemInt(hWnd,IDC_PORT_EDIT,NULL,FALSE));

												//GetWindowText(GetDlgItem(hWnd, IDC_INTERVAL_EDIT), buffer, sizeof(buffer));
												//interval = atoi(buffer);
												interval=GetDlgItemInt(hWnd,IDC_INTERVAL_EDIT,NULL,FALSE);
												db_set_dw(NULL, ModuleName, "Interval", interval);
												
												db_set_b(NULL, ModuleName, "Reconnect", IsDlgButtonChecked(hWnd, IDC_RECONNECT));
												
												GetWindowText(GetDlgItem(hWnd, IDC_RECONNECT_INTERVAL), buffer, sizeof(buffer));
												//interval = atoi(buffer);
												interval=GetDlgItemInt(hWnd,IDC_RECONNECT_INTERVAL,NULL,FALSE);
												db_set_dw(NULL, ModuleName, "ReconnectInterval", interval);
												
												db_set_b(NULL, ModuleName, "UsePopups", IsDlgButtonChecked(hWnd, IDC_USE_POPUPS));
												db_set_b(NULL, ModuleName, "UsePortCheck", IsDlgButtonChecked(hWnd, IDC_USE_PORTCHECK));
												
												//GetWindowText(GetDlgItem(hWnd, IDC_MAX_RETRIES), buffer, sizeof(buffer));
												//retries = atoi(buffer);
												retries=GetDlgItemInt(hWnd,IDC_MAX_RETRIES,NULL,FALSE);
												db_set_b(NULL, ModuleName, "MaxRetries", retries);
												
												
												exchangeServer.Reconnect(); //login info may be changed
												UpdateTimers(); //interval might get changed
												
												break;
											}
									}
									
								break;
							}
					}
					
				break;
			}
	}
	
	return 0;
}

#include "commctrl.h"

void AddAnchorWindowToDeferList(HDWP &hdWnds, HWND window, RECT *rParent, WINDOWPOS *wndPos, int anchors)
{
	RECT rChild = AnchorCalcPos(window, rParent, wndPos, anchors);
	hdWnds = DeferWindowPos(hdWnds, window, HWND_NOTOPMOST, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, SWP_NOZORDER);
}

void SavePosition(HWND hWnd)
{
	RECT rWnd;
	GetWindowRect(hWnd, &rWnd);
	db_set_dw(0, ModuleName, "PosX", rWnd.left);
	db_set_dw(0, ModuleName, "PosY", rWnd.top);
	//DBWriteContactSettingDword(0, ModuleName, "Width", rWnd.right - rWnd.left);
	//DBWriteContactSettingDword(0, ModuleName, "Height", rWnd.bottom - rWnd.top);
}

void LoadPosition(HWND hWnd)
{
	int x, y;
	int width = 500, height = 300;
	x = db_get_dw(0, ModuleName, "PosX", 0);
	y = db_get_dw(0, ModuleName, "PosY", 0);
	//width = DBGetContactSettingDword(0, ModuleName, "Width", 500);
	//height = DBGetContactSettingDword(0, ModuleName, "Height", 300);
	SetWindowPos(hWnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);
}

int CALLBACK ListSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_KEYUP:
				{
					if (wParam == VK_ESCAPE)
						{
							SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
						}
						
					break;
				}
				
			case WM_SYSKEYDOWN:
				{
					if (wParam == 'X')
						{
							SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);						
						}
						
					break;
				}
				
			case WM_LBUTTONDBLCLK:
				{
					int i;
					int count = ListView_GetItemCount(hWnd);
					for (i = 0; i < count; i++)	
						{
							if (ListView_GetItemState(hWnd, i, LVIS_SELECTED))
								{
									TCHAR emailID[4096]; //uhh
									ListView_GetItemText(hWnd, i, 2, emailID, sizeof(emailID));
									exchangeServer.OpenMessage(emailID);
								}
						}
						
					break;
				}
		}
		
	return CallWindowProc(OldListProc, hWnd, msg, wParam, lParam);
}

BOOL CALLBACK DlgProcEmails(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					TranslateDialogDefault(hWnd);
					
					SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM) hiMailIcon);
					
					LVCOLUMN col = {0};
					HWND hList = GetDlgItem(hWnd, IDC_EMAILS_LIST);
					OldListProc = (WNDPROC) SetWindowLong(hList, GWLP_WNDPROC, (LONG) ListSubclassProc);
					ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
					col.mask = LVCF_TEXT | LVCF_WIDTH;
					col.cx = 100;
					col.pszText = TranslateT("Entry ID");
					col.cchTextMax = _tcslen(col.pszText) + 1;
					ListView_InsertColumn(hList, 0, &col);
					col.pszText = TranslateT("Subject");
					col.cx = 300;
					col.cchTextMax = _tcslen(col.pszText) + 1;
					ListView_InsertColumn(hList, 0, &col);
					col.cx = 200;
					col.iSubItem = 1;
					col.pszText = TranslateT("Sender");
					col.cchTextMax = _tcslen(col.pszText) + 1;
					ListView_InsertColumn(hList, 0, &col);
					
					//LoadPosition(hWnd);
					
					return TRUE;
					break;
				}
				
			case WM_DESTROY:
				{
					hEmailsDlg = NULL;
					
					break;
				}
				
			case WM_CLOSE:
				{
					//ShowWindow(hWnd, SW_HIDE);
					DestroyWindow(hWnd); //close the window - no need to keep it in memory
					//SavePosition(hWnd);
					
					break;
				}
			
			case EXM_UPDATE_EMAILS:
				{
					HWND hList = GetDlgItem(hWnd, IDC_EMAILS_LIST);
					ListView_DeleteAllItems(hList);
					int i;
					int count = GetWindowLong(hWnd, GWLP_USERDATA);
					LVITEM item = {0};
					TEmailHeader email = {0};
					email.cbSize = sizeof(TEmailHeader);
					TCHAR sender[1024] = _T("");
					TCHAR subject[1024] = _T("");
					//char buffer[4096];
					email.cSender = sizeof(sender);
					email.cSubject = sizeof(subject);
					email.szSender = sender;
					email.szSubject = subject;
					item.mask = LVIF_TEXT;
					
					for (i = 0; i < count; i++)
						{
							exchangeServer.GetEmailHeader(i, &email);
							item.iItem = i;
							item.iSubItem = 0;
							item.pszText = email.szSender;
							ListView_InsertItem(hList, &item);
							ListView_SetItemText(hList, i, 1, email.szSubject);
							ListView_SetItemText(hList, i, 2, mir_a2t(email.emailID));
						}
					SetFocus(hList);
					
					break;
				}
				
			case WM_SHOWWINDOW:
				{
					if (wParam)
						{
							SendMessage(hWnd, EXM_UPDATE_EMAILS, 0, 0);
						}
						
					break;
				}
				
			case WM_KEYUP:
				{
					if (wParam == VK_ESCAPE)
						{
							SendMessage(hWnd, WM_CLOSE, 0, 0);
						}
						
					break;
				}
				
			case WM_SYSKEYDOWN:
				{
					if (wParam == 'X')
						{
							SendMessage(hWnd, WM_CLOSE, 0, 0);
						}
						
					break;
				}
				
			case WM_WINDOWPOSCHANGING:
				{
					HDWP hdWnds = BeginDeferWindowPos(3);
					RECT rParent;
					WINDOWPOS *wndPos = (WINDOWPOS *) lParam;
					
					if ((!wndPos) || (wndPos->flags & SWP_NOSIZE))
						{
							break;
						}
					GetWindowRect(hWnd, &rParent);
					if (wndPos->cx < MIN_EMAILS_WIDTH)
						{
							wndPos->cx = MIN_EMAILS_WIDTH;
						}
					if (wndPos->cy < MIN_EMAILS_HEIGHT)
						{
							wndPos->cy = MIN_EMAILS_HEIGHT;
						}
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_EMAILS_LIST), &rParent, wndPos, ANCHOR_ALL);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_CLOSE), &rParent, wndPos, ANCHOR_BOTTOM | ANCHOR_RIGHT);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_MARK_READ), &rParent, wndPos, ANCHOR_BOTTOM | ANCHOR_LEFT);
					
					EndDeferWindowPos(hdWnds);
					
					break;
				}
				
			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDC_CLOSE:
								{
									SendMessage(hWnd, WM_CLOSE, 0, 0);
									//HWND hList = GetDlgItem(hWnd, IDC_EMAILS_LIST);
									//ListView_SetItemState(hList, -1, LVIS_SELECTED, LVIS_SELECTED);
									//SetFocus(hList);
									
									break;
								}
								
							case IDC_MARK_READ:
								{
									HWND hList = GetDlgItem(hWnd, IDC_EMAILS_LIST);
									int i;
									int count = ListView_GetItemCount(hList);
									for (i = 0; i < count; i++)
										{
											if (ListView_GetCheckState(hList, i))
												{
													//char sender[1024]; //nooo
													TCHAR emailID[2048]; //uhh ohh
													LVITEM item = {0};
													item.iItem = i;
													item.mask = LVIF_TEXT;
													//item.pszText = sender;
													//item.cchTextMax = sizeof(sender);
													//ListView_GetItem(hList, &item);
													
													item.iSubItem = 2;
													item.cchTextMax = sizeof(emailID);
													item.pszText = emailID;
													ListView_GetItem(hList, &item);
													exchangeServer.MarkEmailAsRead(emailID);
												}
										}
									count = exchangeServer.GetUnreadEmailsCount();
									if (count > 0)
										{
											SetWindowLong(hWnd, GWLP_USERDATA, count);
											SendMessage(hWnd, EXM_UPDATE_EMAILS, 0, 0);
										}
										else{
											SendMessage(hWnd, WM_CLOSE, 0, 0);
										}
										
									break;
								}
						}
						
					break;
				}
			default:
			
				break;
		}

	return 0;
}

BOOL CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_COMMAND:
				{
					switch (HIWORD(wParam))
						{
							case STN_CLICKED:
								{
									//HWND hParent = FindWindow(MIRANDACLASS, NULL);
									//CreateDialog(hInstance, MAKEINTRESOURCE(IDD_EMAILS), hParent, DlgProcEmails);
									int count = (int) PUGetPluginData(hWnd);
									ShowEmailsWindow(count);
									PUDeletePopup(hWnd);
									
									break;
								}
						}
						
					break;
				}
			case WM_CONTEXTMENU:
				{
					PUDeletePopup(hWnd);
					
					break;
				}
		}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}