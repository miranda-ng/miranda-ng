/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#define COLOR_USERINFO RGB(138, 190, 160)
#define COLOR_MBIRTHDAY RGB(222, 222, 88)
#define COLOR_BIRTHDAYREMINDER RGB(200, 120, 240)
#define COLOR_PROTOCOL RGB(255, 153, 153)
#define COLOR_MICQBIRTHDAY RGB(88, 88, 240)

#define UPCOMING_TIMER_ID 1002

static WNDPROC OldBirthdaysListProc = NULL;

const TCHAR *szAdvancedIcons[] = {_T("RES0"), _T("Email"), _T("Web"), _T("SMS"), _T("Advanced 1"), _T("Advanced 2"), _T("Advanced 3"), _T("Client"), _T("Advanced 4"), _T("Protocol"), _T("RES2/VisMode")};
const int cAdvancedIcons = sizeof(szAdvancedIcons) / sizeof(szAdvancedIcons[0]); //don't forget to modify icons.cpp

const TCHAR *szShowAgeMode[] = {_T("Upcoming age"), _T("Current age")};
const int cShowAgeMode = sizeof(szShowAgeMode) / sizeof(szShowAgeMode[0]);

const TCHAR *szSaveModule[] = {_T("UserInfo module"), _T("Protocol module"), _T("mBirthday module")};
const int cSaveModule = sizeof(szSaveModule) / sizeof(szSaveModule[0]);

const TCHAR *szPopupClick[] = {_T("Nothing"), _T("Dismiss"), _T("Message window")};
const int cPopupClick = sizeof(szPopupClick) / sizeof(szPopupClick[0]);

const TCHAR *szNotifyFor[] = {_T("All contacts"), _T("All contacts except hidden ones"), _T("All contacts except ignored ones"), _T("All contacts except hidden and ignored ones")};
const int cNotifyFor = sizeof(szNotifyFor) / sizeof(szNotifyFor[0]);

#define MIN_BIRTHDAYS_WIDTH 200
#define MIN_BIRTHDAYS_HEIGHT 200

#include "commctrl.h"
void CreateToolTip(HWND target, TCHAR* tooltip, LPARAM width)
{
	 HWND hwndToolTip;

	 hwndToolTip = CreateWindow(TOOLTIPS_CLASS, NULL, 
															WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
															CW_USEDEFAULT, CW_USEDEFAULT,
															CW_USEDEFAULT, CW_USEDEFAULT,
															target,
															NULL,
															NULL,
															NULL);
	 if (hwndToolTip)
	 {
			TOOLINFO ti = {0};
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_TRANSPARENT | TTF_SUBCLASS;
			ti.hwnd = target;
			ti.uId = 0;
			ti.hinst = NULL;
			ti.lpszText = tooltip;
			GetClientRect(target, &ti.rect);
			SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM) &ti );
			SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, width);
			SendMessage(hwndToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000);
	 }
}

int EnablePopupsGroup(HWND hWnd, int enable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_POPUPS_STATIC), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_POPUP_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_FOREGROUND), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_BACKGROUND), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_NOBIRTHDAYS_POPUP), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_IGNORE_SUBCONTACTS), (ServiceExists(MS_MC_GETMETACONTACT)) ? enable : FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_PREVIEW), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_LEFT_CLICK), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_RIGHT_CLICK), enable);
	
	return enable;
}

int EnableClistGroup(HWND hWnd, int enable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_CLIST_STATIC), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_ADVANCED_ICON), enable);
	return enable;
}

int EnableDialogGroup(HWND hWnd, int enable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_DLG_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_OPENINBACKGROUND), enable);
	
	return enable;
}

int AddInfoToComboBoxes(HWND hWnd)
{
	int i;
	TCHAR *buffer;
	for (i = 0; i < cAdvancedIcons; i++)
		{
			buffer = TranslateTS(szAdvancedIcons[i]);
			SendMessage(GetDlgItem(hWnd, IDC_ADVANCED_ICON), CB_ADDSTRING, 0, (LPARAM) buffer);
		}
		
	for (i = 0; i < cShowAgeMode; i++)
		{
			SendMessage(GetDlgItem(hWnd, IDC_AGE_COMBOBOX), CB_ADDSTRING, 0, (LPARAM) TranslateTS(szShowAgeMode[i]));
		}
		
	for (i = 0; i < cSaveModule; i++)
		{
			SendMessage(GetDlgItem(hWnd, IDC_DEFAULT_MODULE), CB_ADDSTRING, 0, (LPARAM) TranslateTS(szSaveModule[i]));
		}
		
	for (i = 0; i < cPopupClick; i++)
		{
			SendMessage(GetDlgItem(hWnd, IDC_LEFT_CLICK), CB_ADDSTRING, 0, (LPARAM) TranslateTS(szPopupClick[i]));
			SendMessage(GetDlgItem(hWnd, IDC_RIGHT_CLICK), CB_ADDSTRING, 0, (LPARAM) TranslateTS(szPopupClick[i]));
		}
		
	for (i = 0; i < cNotifyFor; i++)
	{
		SendMessage(GetDlgItem(hWnd, IDC_NOTIFYFOR), CB_ADDSTRING, 0, (LPARAM) TranslateTS(szNotifyFor[i]));
	}
		
	return i;
}

SIZE GetControlTextSize(HWND hCtrl)	
{
	HDC hDC = GetDC(hCtrl);
	HFONT font = (HFONT) SendMessage(hCtrl, WM_GETFONT, 0, 0);
	HFONT oldFont = (HFONT) SelectObject(hDC, font);
	const size_t maxSize = 2048;
	TCHAR buffer[maxSize];
	SIZE size;
	GetWindowText(hCtrl, buffer, maxSize);
	GetTextExtentPoint32(hDC, buffer, (int) _tcslen(buffer), &size);
	SelectObject(hDC, oldFont);
	ReleaseDC(hCtrl, hDC);
	return size;
}

int EnlargeControl(HWND hCtrl, HWND hGroup, SIZE oldSize)
{
	SIZE size = GetControlTextSize(hCtrl);
	int offset = 0;
	RECT rect;
	GetWindowRect(hCtrl, &rect);
	offset = (rect.right - rect.left) - oldSize.cx;
	SetWindowPos(hCtrl, HWND_TOP, 0, 0, size.cx + offset, oldSize.cy, SWP_NOMOVE);
	SetWindowPos(hCtrl, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	//RedrawWindow(hCtrl, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASENOW);
	
	return 0;
}

TCHAR *strtrim(TCHAR *str)
{
	size_t i = 0;
	size_t len = _tcslen(str);
	while ((i < len) && (str[i] == _T(' '))) { i++; }
	if (i)
	{
		memmove(str, str + i, len - i + 1);
		len -= i;
	}
		
	while ((len > 0)  && (str[--len] == _T(' ')))	{	str[len] = 0;	}
	
	return str;
}

INT_PTR CALLBACK DlgProcOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					bInitializing = 1;
					
					SIZE oldPopupsSize = GetControlTextSize(GetDlgItem(hWnd, IDC_USE_POPUPS));
					SIZE oldClistIconSize = GetControlTextSize(GetDlgItem(hWnd, IDC_USE_CLISTICON));
					SIZE oldDialogSize = GetControlTextSize(GetDlgItem(hWnd, IDC_USE_DIALOG));
					
					TranslateDialogDefault(hWnd);
					
					EnlargeControl(GetDlgItem(hWnd, IDC_USE_POPUPS), GetDlgItem(hWnd, IDC_POPUPS_STATIC), oldPopupsSize);
					EnlargeControl(GetDlgItem(hWnd, IDC_USE_CLISTICON), GetDlgItem(hWnd, IDC_CLIST_STATIC), oldClistIconSize);
					EnlargeControl(GetDlgItem(hWnd, IDC_USE_DIALOG), GetDlgItem(hWnd, IDC_DIALOG_STATIC), oldDialogSize);

					AddInfoToComboBoxes(hWnd);
					
					SendMessage(GetDlgItem(hWnd, IDC_FOREGROUND), CPM_SETDEFAULTCOLOUR, 0, FOREGROUND_COLOR);
					SendMessage(GetDlgItem(hWnd, IDC_BACKGROUND), CPM_SETDEFAULTCOLOUR, 0, BACKGROUND_COLOR);
					
					SendMessage(GetDlgItem(hWnd, IDC_FOREGROUND), CPM_SETCOLOUR, 0, commonData.foreground);
					SendMessage(GetDlgItem(hWnd, IDC_BACKGROUND), CPM_SETCOLOUR, 0, commonData.background);
					
					SendMessage(GetDlgItem(hWnd, IDC_ADVANCED_ICON), CB_SETCURSEL, commonData.clistIcon, 0);
					SendMessage(GetDlgItem(hWnd, IDC_DEFAULT_MODULE), CB_SETCURSEL, commonData.cDefaultModule, 0);
					SendMessage(GetDlgItem(hWnd, IDC_LEFT_CLICK), CB_SETCURSEL, commonData.lPopupClick, 0);
					SendMessage(GetDlgItem(hWnd, IDC_RIGHT_CLICK), CB_SETCURSEL, commonData.rPopupClick, 0);
					SendMessage(GetDlgItem(hWnd, IDC_NOTIFYFOR), CB_SETCURSEL, commonData.notifyFor, 0);
					
					CreateToolTip(GetDlgItem(hWnd, IDC_POPUP_TIMEOUT), TranslateT("Set popup delay when notifying of upcoming birthdays.\nFormat: default delay [ | delay for birthdays occuring today]"), 400);
					
					TCHAR buffer[1024];
					_itot(commonData.daysInAdvance, buffer, 10);
					SetWindowText(GetDlgItem(hWnd, IDC_DAYS_IN_ADVANCE), buffer);
					_itot(commonData.checkInterval, buffer, 10);
					SetWindowText(GetDlgItem(hWnd, IDC_CHECK_INTERVAL), buffer);
					_sntprintf(buffer, 1024, _T("%d|%d"), commonData.popupTimeout, commonData.popupTimeoutToday);
					SetWindowText(GetDlgItem(hWnd, IDC_POPUP_TIMEOUT), buffer);
					_itot(commonData.cSoundNearDays, buffer, 10);
					SetWindowText(GetDlgItem(hWnd, IDC_SOUND_NEAR_DAYS_EDIT), buffer);
					_itot(commonData.cDlgTimeout, buffer, 10);
					SetWindowText(GetDlgItem(hWnd, IDC_DLG_TIMEOUT), buffer);
					_itot(commonData.daysAfter, buffer, 10);
					SetWindowText(GetDlgItem(hWnd, IDC_DAYS_AFTER), buffer);
					
					CheckDlgButton(hWnd, IDC_OPENINBACKGROUND, (commonData.bOpenInBackground) ? BST_CHECKED : BST_UNCHECKED);
					
					CheckDlgButton(hWnd, IDC_NOBIRTHDAYS_POPUP, (commonData.bNoBirthdaysPopup) ? BST_CHECKED : BST_UNCHECKED);
					//CheckDlgButton(hWnd, IDC_SHOW_CURRENT_AGE, (bShowCurrentAge) ? BST_CHECKED : BST_UNCHECKED);
					SendMessage(GetDlgItem(hWnd, IDC_AGE_COMBOBOX), CB_SETCURSEL, commonData.cShowAgeMode, 0);
					
					CheckDlgButton(hWnd, IDC_IGNORE_SUBCONTACTS, (commonData.bIgnoreSubcontacts) ? BST_CHECKED : BST_UNCHECKED);
					
					CheckDlgButton(hWnd, IDC_ONCE_PER_DAY, (commonData.bOncePerDay) ? BST_CHECKED : BST_UNCHECKED);
					EnableWindow(GetDlgItem(hWnd, IDC_CHECK_INTERVAL), !commonData.bOncePerDay);
					
					CheckDlgButton(hWnd, IDC_USE_DIALOG, (commonData.bUseDialog) ? BST_CHECKED : BST_UNCHECKED);
					EnableDialogGroup(hWnd, commonData.bUseDialog);
					
					if (ServiceExists(MS_POPUP_ADDPOPUPEX))
						{
							CheckDlgButton(hWnd, IDC_USE_POPUPS, commonData.bUsePopups ? BST_CHECKED : BST_UNCHECKED);
							EnablePopupsGroup(hWnd, commonData.bUsePopups);
						}
						else{
							EnableWindow(GetDlgItem(hWnd, IDC_USE_POPUPS), FALSE);
							EnablePopupsGroup(hWnd, FALSE);
						}
					
					if (ServiceExists(MS_CLIST_EXTRA_SET_ICON) && (!ServiceExists(MS_EXTRAICON_REGISTER)))
						{
							CheckDlgButton(hWnd, IDC_USE_CLISTICON, commonData.bUseClistIcon ? BST_CHECKED : BST_UNCHECKED);
							EnableClistGroup(hWnd, commonData.bUseClistIcon);
						}
						else{
							CheckDlgButton(hWnd, IDC_USE_CLISTICON, BST_CHECKED);
							EnableWindow(GetDlgItem(hWnd, IDC_USE_CLISTICON), FALSE);
							EnableClistGroup(hWnd, FALSE);
						}
					bInitializing = 0;
					
					return TRUE;
				}
			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDC_CHECK_INTERVAL:
							case IDC_POPUP_TIMEOUT:
							case IDC_DAYS_IN_ADVANCE:
							case IDC_DLG_TIMEOUT:
							case IDC_SOUND_NEAR_DAYS_EDIT:
							case IDC_DAYS_AFTER:
								{
									if ((HIWORD(wParam) == EN_CHANGE) && (!bInitializing))// || (HIWORD(wParam) == CBN_SELENDOK))
										{
											SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
										}
										
									break;
								}
								
							case IDC_USE_POPUPS:
							case IDC_USE_CLISTICON:
							case IDC_USE_DIALOG:
								{
									EnablePopupsGroup(hWnd, IsDlgButtonChecked(hWnd, IDC_USE_POPUPS));
									EnableClistGroup(hWnd, IsDlgButtonChecked(hWnd, IDC_USE_CLISTICON));
									EnableDialogGroup(hWnd, IsDlgButtonChecked(hWnd, IDC_USE_DIALOG));
									RedrawWindow(GetDlgItem(hWnd, IDC_USE_POPUPS), NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
									RedrawWindow(GetDlgItem(hWnd, IDC_USE_CLISTICON), NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
									RedrawWindow(GetDlgItem(hWnd, IDC_USE_DIALOG), NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
								}//fallthrough
								
							case IDC_FOREGROUND:
							case IDC_BACKGROUND:
							case IDC_ADVANCED_ICON:
							case IDC_IGNORE_SUBCONTACTS:
							case IDC_AGE_COMBOBOX:
							case IDC_NOBIRTHDAYS_POPUP:
							case IDC_DEFAULT_MODULE:
							case IDC_LEFT_CLICK:
							case IDC_RIGHT_CLICK:
							case IDC_ONCE_PER_DAY:
							case IDC_NOTIFYFOR:
								{
									SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
									EnableWindow(GetDlgItem(hWnd, IDC_CHECK_INTERVAL), !IsDlgButtonChecked(hWnd, IDC_ONCE_PER_DAY));
									
									break;
								}
							
							case IDC_PREVIEW:
								{
									HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
									int dtb, age;
									dtb = rand() % 11; //0..10
									age = rand() % 50 + 1; //1..50
									PopupNotifyBirthday(hContact, dtb, age);
									
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
												commonData.foreground = SendMessage(GetDlgItem(hWnd, IDC_FOREGROUND), CPM_GETCOLOUR, 0, 0);
												commonData.background = SendMessage(GetDlgItem(hWnd, IDC_BACKGROUND), CPM_GETCOLOUR, 0, 0);
												commonData.popupTimeout = POPUP_TIMEOUT;
												commonData.popupTimeoutToday = POPUP_TIMEOUT;
												commonData.clistIcon = SendMessage(GetDlgItem(hWnd, IDC_ADVANCED_ICON), CB_GETCURSEL, 0, 0);
												int oldClistIcon = DBGetContactSettingByte(NULL, ModuleName, "AdvancedIcon", -1);
												commonData.bUsePopups = IsDlgButtonChecked(hWnd, IDC_USE_POPUPS);
												commonData.bUseClistIcon = IsDlgButtonChecked(hWnd, IDC_USE_CLISTICON);
												commonData.bUseDialog = IsDlgButtonChecked(hWnd, IDC_USE_DIALOG);
												commonData.bIgnoreSubcontacts = IsDlgButtonChecked(hWnd, IDC_IGNORE_SUBCONTACTS);
												commonData.bNoBirthdaysPopup = IsDlgButtonChecked(hWnd, IDC_NOBIRTHDAYS_POPUP);
												commonData.cShowAgeMode = SendMessage(GetDlgItem(hWnd, IDC_AGE_COMBOBOX), CB_GETCURSEL, 0, 0);
												commonData.cDefaultModule = SendMessage(GetDlgItem(hWnd, IDC_DEFAULT_MODULE), CB_GETCURSEL, 0, 0);
												commonData.lPopupClick = SendMessage(GetDlgItem(hWnd, IDC_LEFT_CLICK), CB_GETCURSEL, 0, 0);
												commonData.rPopupClick = SendMessage(GetDlgItem(hWnd, IDC_RIGHT_CLICK), CB_GETCURSEL, 0, 0);
												commonData.bOncePerDay = IsDlgButtonChecked(hWnd, IDC_ONCE_PER_DAY);
												commonData.notifyFor = SendMessage(GetDlgItem(hWnd, IDC_NOTIFYFOR), CB_GETCURSEL, 0, 0);
												commonData.bOpenInBackground = IsDlgButtonChecked(hWnd, IDC_OPENINBACKGROUND);
																									
												const int maxSize = 1024;
												TCHAR buffer[maxSize];
												
												GetWindowText(GetDlgItem(hWnd, IDC_DAYS_IN_ADVANCE), buffer, maxSize);
												TCHAR *stop = NULL;
												commonData.daysInAdvance = _tcstol(buffer, &stop, 10);

												if (*stop) { commonData.daysInAdvance = DAYS_TO_NOTIFY; }
												
												GetWindowText(GetDlgItem(hWnd, IDC_DAYS_AFTER), buffer, maxSize);
												commonData.daysAfter = _tcstol(buffer, &stop, 10);
												
												if (*stop) { commonData.daysAfter = DAYS_TO_NOTIFY_AFTER; }
												
												GetWindowText(GetDlgItem(hWnd, IDC_CHECK_INTERVAL), buffer, maxSize);
												commonData.checkInterval = _ttol(buffer);
												if (!commonData.checkInterval) { commonData.checkInterval = CHECK_INTERVAL; }
												
												GetWindowText(GetDlgItem(hWnd, IDC_POPUP_TIMEOUT), buffer, maxSize);
												TCHAR *pos;
												pos = _tcschr(buffer, _T('|'));
												if (pos)
												{
													TCHAR tmp[128];
													*pos = 0;
													_tcscpy(tmp, buffer);
													strtrim(tmp);
													commonData.popupTimeout = _ttol(tmp);
													
													_tcscpy(tmp, pos + 1);
													strtrim(tmp);
													commonData.popupTimeoutToday = _ttol(tmp);
													
												}
												else{
													commonData.popupTimeout = commonData.popupTimeoutToday = _ttol(buffer);
												}
												
												GetWindowText(GetDlgItem(hWnd, IDC_SOUND_NEAR_DAYS_EDIT), buffer, maxSize);
												//cSoundNearDays = _ttol(buffer);
												commonData.cSoundNearDays = _tcstol(buffer, &stop, 10);
												if (*stop) { commonData.cSoundNearDays = BIRTHDAY_NEAR_DEFAULT_DAYS; }
												
												GetWindowText(GetDlgItem(hWnd, IDC_DLG_TIMEOUT), buffer, maxSize);
												commonData.cDlgTimeout = _tcstol(buffer, &stop, 10);
												if (*stop) { commonData.cDlgTimeout = POPUP_TIMEOUT; }
												
												DBWriteContactSettingByte(NULL, ModuleName, "IgnoreSubcontacts", commonData.bIgnoreSubcontacts);
												DBWriteContactSettingByte(NULL, ModuleName, "UsePopups", commonData.bUsePopups);
												DBWriteContactSettingByte(NULL, ModuleName, "UseClistIcon", commonData.bUseClistIcon);
												DBWriteContactSettingByte(NULL, ModuleName, "UseDialog", commonData.bUseDialog);
												DBWriteContactSettingByte(NULL, ModuleName, "AdvancedIcon", commonData.clistIcon);
												DBWriteContactSettingWord(NULL, ModuleName, "CheckInterval", commonData.checkInterval);
												DBWriteContactSettingWord(NULL, ModuleName, "DaysInAdvance", commonData.daysInAdvance);
												DBWriteContactSettingWord(NULL, ModuleName, "DaysAfter", commonData.daysAfter);
												DBWriteContactSettingWord(NULL, ModuleName, "PopupTimeout", commonData.popupTimeout);
												DBWriteContactSettingWord(NULL, ModuleName, "PopupTimeoutToday", commonData.popupTimeoutToday);
												
												DBWriteContactSettingByte(NULL, ModuleName, "ShowCurrentAge", commonData.cShowAgeMode);
												DBWriteContactSettingByte(NULL, ModuleName, "NoBirthdaysPopup", commonData.bNoBirthdaysPopup);
												
												DBWriteContactSettingByte(NULL, ModuleName, "OpenInBackground", commonData.bOpenInBackground);
												
												DBWriteContactSettingByte(NULL, ModuleName, "SoundNearDays", commonData.cSoundNearDays);
												
												DBWriteContactSettingByte(NULL, ModuleName, "DefaultModule", commonData.cDefaultModule);
												
												DBWriteContactSettingByte(NULL, ModuleName, "PopupLeftClick", commonData.lPopupClick);
												DBWriteContactSettingByte(NULL, ModuleName, "PopupRightClick", commonData.rPopupClick);
												
												DBWriteContactSettingDword(NULL, ModuleName, "Foreground", commonData.foreground);
												DBWriteContactSettingDword(NULL, ModuleName, "Background", commonData.background);
												
												DBWriteContactSettingByte(NULL, ModuleName, "OncePerDay", commonData.bOncePerDay);
												
												DBWriteContactSettingWord(NULL, ModuleName, "DlgTimeout", commonData.cDlgTimeout);
												
												DBWriteContactSettingByte(NULL, ModuleName, "NotifyFor", commonData.notifyFor);
												
												RefreshAllContactListIcons(oldClistIcon);
												
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





INT_PTR CALLBACK DlgProcAddBirthday(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					TranslateDialogDefault(hWnd);
					
					SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hiAddBirthdayContact);
					int i;
					for (i = 0; i < cSaveModule; i++)
						{
							SendMessage(GetDlgItem(hWnd, IDC_COMPATIBILITY), CB_ADDSTRING, 0, (LPARAM) TranslateTS(szSaveModule[i]));
						}
					i = commonData.cDefaultModule; //DBGetContactSettingByte(NULL, ModuleName, "DefaultModule", 0);
					SendMessage(GetDlgItem(hWnd, IDC_COMPATIBILITY), CB_SETCURSEL, i, 0);
					
					break;
				}
				
			case WM_SHOWWINDOW:
				{
					int year, month, day;
					TCHAR *szTooltipText = TranslateT("Please select the module where you want the date of birth to be saved.\r\n\"UserInfo\" is the default location.\r\nUse \"Protocol module\" to make the data visible in User Details.\n\"mBirthday module\" uses the same module as mBirthday plugin.");
					TCHAR *szCurrentModuleTooltip = NULL;
					HANDLE hContact = (HANDLE) GetWindowLongPtr(hWnd, GWLP_USERDATA);
					const int maxSize = 2048;
					TCHAR buffer[maxSize];
					char protocol[256];
					GetContactProtocol(hContact, protocol, sizeof(protocol));
					
					TCHAR *name = GetContactName(hContact, protocol);
					_stprintf(buffer, TranslateT("Set birthday for %s:"), name);
					free(name);
					SetWindowText(hWnd, buffer);
					HWND hDate = GetDlgItem(hWnd, IDC_DATE);
					int loc = GetContactDOB(hContact, year, month, day);
					if (IsDOBValid(year, month, day))
						{
							SYSTEMTIME st = {0};
							st.wDay = day;
							st.wMonth = month;
							st.wYear = year;
							DateTime_SetSystemtime(hDate, GDT_VALID, &st);
						}
						else{
							DateTime_SetSystemtime(hDate, GDT_NONE, NULL);
						}
					switch (loc)
						{
							case DOB_MBIRTHDAY:
								{
									DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_MBIRTHDAY);
									szCurrentModuleTooltip = _T("mBirthday");
									
									break;
								}
								
							case DOB_PROTOCOL:
								{
									DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_PROTOCOL);
#ifdef _UNICODE
									_stprintf(buffer, TranslateT("%S protocol"), protocol);
#else
									_stprintf(buffer, TranslateT("%s protocol"), protocol);
#endif
									
									szCurrentModuleTooltip = buffer;
									
									break;
								}
								
							case DOB_BIRTHDAYREMINDER:
								{
									DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_BIRTHDAYREMINDER);
									szCurrentModuleTooltip = _T("Birthday Reminder");
									
									break;
								}
								
							case DOB_USERINFO:
								{
									DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_USERINFO);
									szCurrentModuleTooltip = _T("UserInfo");
									
									break;
								}
								
							case DOB_MICQBIRTHDAY:
								{
									DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_MICQBIRTHDAY);
									szCurrentModuleTooltip = _T("mICQBirthday");

									break;
								}

							default:
								{
									szCurrentModuleTooltip = NULL;
								
									break;
								}
						}
					//CreateToolTip(GetDlgItem(hWnd, IDC_COMPATIBILITY), hWnd, szTooltipText);
					CreateToolTip(GetDlgItem(hWnd, IDC_COMPATIBILITY), szTooltipText, 500);
					if (szCurrentModuleTooltip)
						{
							CreateToolTip(hDate, szCurrentModuleTooltip, 400);
						}
					
					break;
				}
				
			case WM_DESTROY:
				{
					HANDLE hContact = (HANDLE) GetWindowLongPtr(hWnd, GWLP_USERDATA);
					OnExtraImageApply((WPARAM) hContact, NULL); //the birthday might be changed, refresh icon.
					WindowList_Remove(hAddBirthdayWndsList, hWnd);
					
					break;
				}
			
			case WM_CLOSE:
				{
					DestroyWindow(hWnd);
					
					break;
				}
				
			case WM_COMMAND:
				{
					switch (LOWORD(wParam))
						{
							case IDOK:
								{
									SYSTEMTIME st;
									HANDLE hContact = (HANDLE) GetWindowLongPtr(hWnd, GWLP_USERDATA);
									HWND hDate = GetDlgItem(hWnd, IDC_DATE);
									if (DateTime_GetSystemtime(hDate, &st) == GDT_VALID)
										{
											int mode = SendMessage(GetDlgItem(hWnd, IDC_COMPATIBILITY), CB_GETCURSEL, 0, 0); //SAVE modes  in date_utils.h are synced
											SaveBirthday(hContact, st.wYear, st.wMonth, st.wDay, mode);
										}
										else{
											SaveBirthday(hContact, 0, 0, 0, SAVE_MODE_DELETEALL);
										}
									if (hBirthdaysDlg != NULL)
										{
											SendMessage(hBirthdaysDlg, WWIM_UPDATE_BIRTHDAY, (WPARAM) hContact, NULL);
										}	
										
									SendMessage(hWnd, WM_CLOSE, 0, 0);
									
									break;
								}
						}
						
					break;
				}
			
		}
		
	return FALSE;
}

void AddAnchorWindowToDeferList(HDWP &hdWnds, HWND window, RECT *rParent, WINDOWPOS *wndPos, int anchors)
{
	RECT rChild = AnchorCalcPos(window, rParent, wndPos, anchors);
	hdWnds = DeferWindowPos(hdWnds, window, HWND_NOTOPMOST, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, SWP_NOZORDER);
}


#define NA TranslateT("N/A")

TCHAR *GetBirthdayModule(int module, HANDLE hContact, TCHAR *birthdayModule, int size)
{
	switch (module)
		{
			case DOB_MBIRTHDAY:
				{
					_tcsncpy(birthdayModule, _T("mBirthday"), size);
					
					break;
				}
				
			case DOB_PROTOCOL:
				{
					char protocol[512];
					GetContactProtocol(hContact, protocol, sizeof(protocol));
#ifdef _UNICODE
					_sntprintf(birthdayModule, size, TranslateT("%S protocol"), protocol);
#else
					_sntprintf(birthdayModule, size, TranslateT("%s protocol"), protocol);
#endif
					
					break;
				}
				
			case DOB_BIRTHDAYREMINDER:
				{
					_tcsncpy(birthdayModule, _T("Birthday Reminder"), size);
					
					break;
				}
				
			case DOB_USERINFO:
				{
					_tcsncpy(birthdayModule, _T("UserInfo"), size);
					
					break;
				}

			case DOB_MICQBIRTHDAY:
				{
					_tcsncpy(birthdayModule, _T("mICQBirthday"), size);

					break;
				}
				
			default:
				{
					_tcsncpy(birthdayModule, NA, size);
				
					break;
				}
		}
	return birthdayModule;
}

static int lastColumn = -1;

struct BirthdaysSortParams{
	HWND hList;
	int column;
};

INT_PTR CALLBACK BirthdaysCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	BirthdaysSortParams params = *(BirthdaysSortParams *) myParam;
	const int maxSize = 1024;
	TCHAR text1[maxSize];
	TCHAR text2[maxSize];
	long value1, value2;
	ListView_GetItemText(params.hList, (int) lParam1, params.column, text1, maxSize);
	ListView_GetItemText(params.hList, (int) lParam2, params.column, text2, maxSize);
	
	int res = 0;
	
	if ((params.column == 2) || (params.column == 4))
		{
			TCHAR *err1, *err2;
			value1 = _tcstol(text1, &err1, 10);
			value2 = _tcstol(text2, &err2, 10);
			
			if ((err1[0]) || (err2[0]))
				{
					res = (err1[0]) ? 1 : -1;
				}
				else{
					if (value1 < value2)
						{
							res = -1;
						}
						else{
							if (value1 == value2)
								{
									res = 0;
								}
								else{
									res = 1;
								}
						}
				}
		}
		else{
			res = _tcsicmp(text1, text2);
		}
	res = (params.column == lastColumn) ? -res : res;
	return res;
}

//only updates the birthday part of the list view entry. Won't update the protocol and the contact name (those shouldn't change anyway :) )
int UpdateBirthdayEntry(HWND hList, HANDLE hContact, int entry, int bShowAll, int bShowCurrentAge, int bAdd)
{
	const int maxSize = 2048;
	TCHAR buffer[maxSize];
	
	int age;
	int dtb;
	int year, month, day;
	TCHAR *name;
	int module;
	char protocol[512];
	LVITEM item = {0};

	int currentYear;
	int currentMonth;
	int currentDay;
	
	int res = entry;
	
	if (bShowCurrentAge)
		{
			time_t now = Today();
			struct tm *today = gmtime(&now);
			currentDay = today->tm_mday + 1;
			currentMonth = today->tm_mon + 1;
			currentYear = today->tm_year;
		}

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iItem = entry;

	module = GetContactDOB(hContact, year, month, day);
	if ((bShowAll) || (IsDOBValid(year, month, day)))
		{
			lastColumn = -1; //list isn't sorted anymore
			dtb = DaysToBirthday(Today(), year, month, day);
			age = GetContactAge(hContact);
			if (bShowCurrentAge)
				{
					if ((month > currentMonth) || ((month == currentMonth) && (day > currentDay))) //birthday still to come
						{
							age--;
						}
				}
				
			item.lParam = (LPARAM) hContact;

			GetContactProtocol(hContact, protocol, sizeof(protocol));
#ifdef _UNICODE
			MultiByteToWideChar(CP_ACP, MB_USEGLYPHCHARS, protocol, -1, buffer, maxSize);
#else
			strcpy(buffer, protocol);
#endif
			item.pszText = buffer;
			
			if (bAdd)
				{
					ListView_InsertItem(hList, &item);
				}
				else{
					ListView_SetItemText(hList, entry, 0, buffer);
				}

			name = GetContactName(hContact, protocol);
			ListView_SetItemText(hList, entry, 1, name);
			free(name);
			
			if ((dtb <= 366) && (dtb >= 0))
				{
					_stprintf(buffer, _T("%d"), dtb);
				}
				else{
					_stprintf(buffer, NA);
				}
			ListView_SetItemText(hList, entry, 2, buffer);
			if ((year != 0) && (month != 0) && (day != 0))
				{
					_stprintf(buffer, _T("%04d-%02d-%02d"), year, month, day);
				}
				else{
					_stprintf(buffer, NA);
				}
			ListView_SetItemText(hList, entry, 3, buffer);
			
			if (age < 400) //hopefully noone lives longer than this :)
				{
					_stprintf(buffer, _T("%d"), age);
				}
				else{
					_stprintf(buffer, NA);
				}
			ListView_SetItemText(hList, entry, 4, buffer);
			
			GetBirthdayModule(module, hContact, buffer, maxSize);
			ListView_SetItemText(hList, entry, 5, buffer);

			res++;
		}
		else{
			if ((!bShowAll) && (!bAdd))
				{
					ListView_DeleteItem(hList, entry);
				}
		}
	return res;	
}

INT_PTR CALLBACK BirthdaysListSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
					HANDLE hContact;
					LVITEM item = {0};
//					char buffer[1024];
//					item.pszText = buffer;
//					item.cchTextMax = sizeof(buffer);
					item.mask = LVIF_PARAM;
					for (i = 0; i < count; i++)	
						{
							if (ListView_GetItemState(hWnd, i, LVIS_SELECTED))
								{//TODO
									item.iItem = i;
									ListView_GetItem(hWnd, &item);
									hContact = (HANDLE) item.lParam;
									CallService(MS_WWI_ADD_BIRTHDAY, (WPARAM) hContact, 0);
								}
						}
					break;
				}
		}
	return CallWindowProc(OldBirthdaysListProc, hWnd, msg, wParam, lParam);
}

void SetBirthdaysCount(HWND hWnd)
{
	int count = ListView_GetItemCount((GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST)));
	TCHAR title[512];
	_stprintf(title, TranslateT("Birthday list (%d)"), count);
	SetWindowText(hWnd, title);
}

int LoadBirthdays(HWND hWnd, int bShowAll)
{
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	HWND hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
	//int bShowCurrentAge = DBGetContactSettingByte(NULL, ModuleName, "ShowCurrentAge", 0);
	
	ListView_DeleteAllItems(hList);
	
	int count = 0;
	while (hContact)
		{
			count = UpdateBirthdayEntry(hList, hContact, count, bShowAll, commonData.cShowAgeMode, 1); 
			
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	SetBirthdaysCount(hWnd);
	
	return 0;
}

INT_PTR CALLBACK DlgProcBirthdays(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_INITDIALOG:
				{
					TranslateDialogDefault(hWnd);
					SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hiListMenu);
					HWND hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
					
					ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
					
					OldBirthdaysListProc = (WNDPROC) SetWindowLongPtr(hList, GWLP_WNDPROC, (LONG_PTR) BirthdaysListSubclassProc);
					
					LVCOLUMN col;
					col.mask = LVCF_TEXT | LVCF_WIDTH;
					col.pszText = TranslateT("Protocol");
					col.cx = 80;
					ListView_InsertColumn(hList, 0, &col);
					col.pszText = TranslateT("Contact");
					col.cx = 180;
					ListView_InsertColumn(hList, 1, &col);
					col.pszText = TranslateT("DTB");
					col.cx = 50;
					ListView_InsertColumn(hList, 2, &col);
					col.pszText = TranslateT("Birthday");
					col.cx = 80;
					ListView_InsertColumn(hList, 3, &col);
					col.pszText = TranslateT("Age");
					col.cx = 50;
					ListView_InsertColumn(hList, 4, &col);
					col.pszText = TranslateT("Module");
					col.cx = 110;
					ListView_InsertColumn(hList, 5, &col);
					
					LoadBirthdays(hWnd, 0);
					int column = DBGetContactSettingByte(NULL, ModuleName, "SortColumn", 0);
					
					BirthdaysSortParams params = {0};
					params.hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
					params.column = column;
					ListView_SortItemsEx(params.hList, BirthdaysCompare, (LPARAM) &params);
					
					Utils_RestoreWindowPosition(hWnd,NULL,ModuleName,"BirthdayList");
					
					return TRUE;
					break;
				}
				
			case WM_DESTROY:
				{
					hBirthdaysDlg = NULL;
					Utils_SaveWindowPosition(hWnd,NULL,ModuleName,"BirthdayList");
					lastColumn = -1;
					
					break;
				}
				
			case WM_CLOSE:
				{
					DestroyWindow(hWnd);
					
					break;
				}
				
			case WWIM_UPDATE_BIRTHDAY:
				{//wParam = hContact
					HWND hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
					HANDLE hContact = (HANDLE) wParam;
					int i;
					int count = ListView_GetItemCount(hList);
					//int bShowCurrentAge = DBGetContactSettingByte(NULL, ModuleName, "ShowCurrentAge", 0);
					LVITEM item = {0};
					int found = 0;
					
					item.mask = LVIF_PARAM;
					for (i = 0; (i < count) && (!found); i++)
						{
							item.iItem = i;
							ListView_GetItem(hList, &item);
							if (hContact == (HANDLE) item.lParam)
								{
									UpdateBirthdayEntry(hList, hContact, i, IsDlgButtonChecked(hWnd, IDC_SHOW_ALL), commonData.cShowAgeMode, 0); 
									found = 1;
								}
						}
					if (!found)
						{
							UpdateBirthdayEntry(hList, hContact, count, IsDlgButtonChecked(hWnd, IDC_SHOW_ALL), commonData.cShowAgeMode, 1);
						}
						
					SetBirthdaysCount(hWnd);
				
					break;
				}
				
			case WM_WINDOWPOSCHANGING:
				{
					HDWP hdWnds = BeginDeferWindowPos(2);
					RECT rParent;
					WINDOWPOS *wndPos = (WINDOWPOS *) lParam;
					GetWindowRect(hWnd, &rParent);

					if (wndPos->cx < MIN_BIRTHDAYS_WIDTH)
						{
							wndPos->cx = MIN_BIRTHDAYS_WIDTH;
						}
					if (wndPos->cy < MIN_BIRTHDAYS_HEIGHT)
						{
							wndPos->cy = MIN_BIRTHDAYS_HEIGHT;
						}
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_CLOSE), &rParent, wndPos, ANCHOR_RIGHT | ANCHOR_BOTTOM);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_SHOW_ALL), &rParent, wndPos, ANCHOR_LEFT | ANCHOR_BOTTOM);
					AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST), &rParent, wndPos, ANCHOR_ALL);
					
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
									
									break;
								}
							case IDC_SHOW_ALL:
								{
									LoadBirthdays(hWnd, IsDlgButtonChecked(hWnd, IDC_SHOW_ALL));
									
									break;
								}
						}
					break;
				}
				
			case WM_NOTIFY:
				{
					switch (((LPNMHDR) lParam)->idFrom)
						{
							case IDC_BIRTHDAYS_LIST:
								{
									switch (((LPNMHDR)lParam)->code)
										{
											case LVN_COLUMNCLICK:
												{
													LPNMLISTVIEW lv = (LPNMLISTVIEW) lParam;
													int column = lv->iSubItem;
													DBWriteContactSettingByte(NULL, ModuleName, "SortColumn", column);
													BirthdaysSortParams params = {0};
													params.hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
													params.column = column;
													ListView_SortItemsEx(params.hList, BirthdaysCompare, (LPARAM) &params);
													lastColumn = (params.column == lastColumn) ? -1 : params.column;

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


INT_PTR CALLBACK UpcomingCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	HWND hList = GetDlgItem(hUpcomingDlg, IDC_UPCOMING_LIST);
	LVITEM item = {0};
	item.iItem = (int) lParam1;
	item.mask = LVIF_PARAM;
	ListView_GetItem(hList, &item);
	int dtb1 = item.lParam;
	
	item.iItem = (int) lParam2;
	ListView_GetItem(hList, &item);
	int dtb2 = item.lParam;
	
	if (dtb1 != dtb2)
	{
		return (dtb1 > dtb2) ? 1 : -1;
	}
	
	return 0;
}

INT_PTR CALLBACK DlgProcUpcoming(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int timeout;
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			timeout = commonData.cDlgTimeout;
			TranslateDialogDefault(hWnd);
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) hiListMenu);
			HWND hList = GetDlgItem(hWnd, IDC_UPCOMING_LIST);
			
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
			
			LVCOLUMN col;
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = TranslateT("Contact");
			col.cx = 300;
			ListView_InsertColumn(hList, 0, &col);
			col.pszText = TranslateT("Age");
			col.cx = 60;
			ListView_InsertColumn(hList, 1, &col);
			
			if (timeout > 0)
			{
				SetTimer(hWnd, UPCOMING_TIMER_ID, 1000, NULL);
			}
		
			return TRUE;
			break;
		}
		
		case WM_TIMER:
		{
			const int MAX_SIZE = 512;
			TCHAR buffer[MAX_SIZE];
			_stprintf(buffer, (timeout != 2) ? TranslateT("Closing in %d seconds") : TranslateT("Closing in %d second"), --timeout);
			SetWindowText(GetDlgItem(hWnd, IDC_CLOSE), buffer);
			
			if (timeout <= 0)
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			
			break;
		}
		
		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
			
			break;
		}
		
		case WM_DESTROY:
		{
			hUpcomingDlg = NULL;
			KillTimer(hWnd, UPCOMING_TIMER_ID);
			
			break;
		}
		
		case WWIM_ADD_UPCOMING_BIRTHDAY:
		{
			PUpcomingBirthday data = (PUpcomingBirthday) wParam;
			
			HWND hList = GetDlgItem(hWnd, IDC_UPCOMING_LIST);
			LVITEM item = {0};
			int index = ListView_GetItemCount(hList);
			item.iItem = index;
			item.mask = LVIF_PARAM | LVIF_TEXT;
			//item.lParam = (LPARAM) data->hContact;
			item.lParam = (LPARAM) data->dtb;
			item.pszText = data->message;
			ListView_InsertItem(hList, &item);
			
			TCHAR buffer[512];
			mir_sntprintf(buffer, 512, _T("%d"), data->age);
			ListView_SetItemText(hList, index, 1, buffer);
			
			ListView_SortItemsEx(hList, UpcomingCompare, NULL);
			
			break;
		}
		
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CLOSE:
				{
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					
					break;
				}
			}
			
			break;
		}
	}
	
	return 0;
}

DWORD WINAPI OpenMessageWindowThread(void *data)
{
	HANDLE hContact = (HANDLE) data;
	CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM) hContact, 0);
	CallServiceSync("SRMsg/LaunchMessageWindow", (WPARAM) hContact, 0);
	
	return 0;
}

int HandlePopupClick(HWND hWnd, int action)
{
	switch (action)
		{
			case 2: //OPEN MESSAGE WINDOW
				{
					HANDLE hContact = (HANDLE) PUGetContact(hWnd);
					if (hContact)
						{
							DWORD threadID;
							HANDLE thread = CreateThread(NULL, NULL, OpenMessageWindowThread, hContact, 0, &threadID);
						}
					
				}//fallthrough
				
			case 1: //DISMISS
				{
					PUDeletePopUp(hWnd);
					
					break;
				}
				
			case 0: //NOTHING
			default:
				{
				
					break;
				}
			
		}
		
	return 0;
}

INT_PTR CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
		{
			case WM_COMMAND:
				{
					switch (HIWORD(wParam))
						{
							case STN_CLICKED:
								{
									HandlePopupClick(hWnd, commonData.lPopupClick);
									
									break;
								}
						}
					break;
				}
				
			case WM_CONTEXTMENU:
				{
					HandlePopupClick(hWnd, commonData.rPopupClick);
					
					break;
				}
		}
		
	return DefWindowProc(hWnd, msg, wParam, lParam);
}