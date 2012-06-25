/*
  Name: NewEventNotify - Plugin for Miranda IM
  File: options.c - Manages Option Dialogs and Settings
  Version: 0.0.4
  Description: Notifies you about some events
  Author: icebreaker, <icebreaker@newmail.net>
  Date: 22.07.02 13:06 / Update: 16.09.02 17:45
  Copyright: (C) 2002 Starzinger Michael

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "neweventnotify.h"
#include <m_options.h>
#include <m_utils.h>

PLUGIN_OPTIONS* options;
BOOL bWmNotify;

int OptionsRead(void)
{
  options->bDisable = NENGetSettingBool(OPT_DISABLE, FALSE);
  options->bPreview = NENGetSettingBool(OPT_PREVIEW, TRUE);
  options->bMenuitem = NENGetSettingBool(OPT_MENUITEM, FALSE);
  options->bDefaultColorMsg = NENGetSettingBool(OPT_COLDEFAULT_MESSAGE, FALSE);
  options->bDefaultColorUrl = NENGetSettingBool(OPT_COLDEFAULT_URL, FALSE);
  options->bDefaultColorFile = NENGetSettingBool(OPT_COLDEFAULT_FILE, FALSE);
  options->bDefaultColorOthers = NENGetSettingBool(OPT_COLDEFAULT_OTHERS, FALSE);
  options->colBackMsg = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_MESSAGE, DEFAULT_COLBACK);
  options->colTextMsg = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_MESSAGE, DEFAULT_COLTEXT);
  options->colBackUrl = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_URL, DEFAULT_COLBACK);
  options->colTextUrl = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_URL, DEFAULT_COLTEXT);
  options->colBackFile = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_FILE, DEFAULT_COLBACK);
  options->colTextFile = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_FILE, DEFAULT_COLTEXT);
  options->colBackOthers = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLBACK_OTHERS, DEFAULT_COLBACK);
  options->colTextOthers = (COLORREF)DBGetContactSettingDword(NULL, MODULE, OPT_COLTEXT_OTHERS, DEFAULT_COLTEXT);
  options->maskNotify = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKNOTIFY, DEFAULT_MASKNOTIFY);
  options->maskActL = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKACTL, DEFAULT_MASKACTL);
  options->maskActR = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKACTR, DEFAULT_MASKACTR);
  options->maskActTE = (UINT)DBGetContactSettingByte(NULL, MODULE, OPT_MASKACTTE, DEFAULT_MASKACTE);
  options->bMsgWindowCheck = NENGetSettingBool(OPT_MSGWINDOWCHECK, TRUE);
  options->bMsgReplyWindow = NENGetSettingBool(OPT_MSGREPLYWINDOW, FALSE);
  options->bMergePopup = NENGetSettingBool(OPT_MERGEPOPUP, TRUE);
  options->iDelayMsg = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_MESSAGE, DEFAULT_DELAY);
  options->iDelayUrl = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_URL, DEFAULT_DELAY);
  options->iDelayFile = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_FILE, DEFAULT_DELAY);
  options->iDelayOthers = (int)DBGetContactSettingDword(NULL, MODULE, OPT_DELAY_OTHERS, DEFAULT_DELAY);
  options->iDelayDefault = (int)DBGetContactSettingRangedWord(NULL, "PopUp", "Seconds", SETTING_LIFETIME_DEFAULT, SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
  options->bShowDate = NENGetSettingBool(OPT_SHOW_DATE, TRUE);
  options->bShowTime = NENGetSettingBool(OPT_SHOW_TIME, TRUE);
  options->bShowHeaders = NENGetSettingBool(OPT_SHOW_HEADERS, TRUE);
  options->iNumberMsg = (BYTE)DBGetContactSettingByte(NULL, MODULE, OPT_NUMBER_MSG, TRUE);
  options->bShowON = NENGetSettingBool(OPT_SHOW_ON, TRUE);
  options->bHideSend = NENGetSettingBool(OPT_HIDESEND, TRUE);
  options->bNoRSS = NENGetSettingBool(OPT_NORSS, FALSE);
  options->bReadCheck = NENGetSettingBool(OPT_READCHECK, FALSE);
  return 0;
}

int OptionsWrite(void)
{
  NENWriteSettingBool(OPT_DISABLE,  options->bDisable);
  NENWriteSettingBool(OPT_PREVIEW,  options->bPreview);
  NENWriteSettingBool(OPT_MENUITEM, options->bMenuitem);
  NENWriteSettingBool(OPT_COLDEFAULT_MESSAGE, options->bDefaultColorMsg);
  NENWriteSettingBool(OPT_COLDEFAULT_URL,     options->bDefaultColorUrl);
  NENWriteSettingBool(OPT_COLDEFAULT_FILE,    options->bDefaultColorFile);
  NENWriteSettingBool(OPT_COLDEFAULT_OTHERS,  options->bDefaultColorOthers);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_MESSAGE, (DWORD)options->colBackMsg);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_MESSAGE, (DWORD)options->colTextMsg);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_URL, (DWORD)options->colBackUrl);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_URL, (DWORD)options->colTextUrl);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_FILE, (DWORD)options->colBackFile);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_FILE, (DWORD)options->colTextFile);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLBACK_OTHERS, (DWORD)options->colBackOthers);
  DBWriteContactSettingDword(NULL, MODULE, OPT_COLTEXT_OTHERS, (DWORD)options->colTextOthers);
  DBWriteContactSettingByte(NULL, MODULE, OPT_MASKNOTIFY, (BYTE)options->maskNotify);
  DBWriteContactSettingByte(NULL, MODULE, OPT_MASKACTL, (BYTE)options->maskActL);
  DBWriteContactSettingByte(NULL, MODULE, OPT_MASKACTR, (BYTE)options->maskActR);
  DBWriteContactSettingByte(NULL, MODULE, OPT_MASKACTTE, (BYTE)options->maskActTE);
  NENWriteSettingBool(OPT_MSGWINDOWCHECK, options->bMsgWindowCheck);
  NENWriteSettingBool(OPT_MSGREPLYWINDOW, options->bMsgReplyWindow);
  NENWriteSettingBool(OPT_MERGEPOPUP,     options->bMergePopup);
  DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_MESSAGE, (DWORD)options->iDelayMsg);
  DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_URL, (DWORD)options->iDelayUrl);
  DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_FILE, (DWORD)options->iDelayFile);
  DBWriteContactSettingDword(NULL, MODULE, OPT_DELAY_OTHERS, (DWORD)options->iDelayOthers);
  NENWriteSettingBool(OPT_SHOW_DATE,    options->bShowDate);
  NENWriteSettingBool(OPT_SHOW_TIME,    options->bShowTime);
  NENWriteSettingBool(OPT_SHOW_HEADERS, options->bShowHeaders);
  DBWriteContactSettingByte(NULL, MODULE, OPT_NUMBER_MSG, (BYTE)options->iNumberMsg);
  NENWriteSettingBool(OPT_SHOW_ON,    options->bShowON);
  NENWriteSettingBool(OPT_HIDESEND,   options->bHideSend);
  NENWriteSettingBool(OPT_NORSS,      options->bNoRSS);
  NENWriteSettingBool(OPT_READCHECK,  options->bReadCheck);
  return 0;
}

static void SetCheckBoxState(HWND hWnd, int iCtrl, BOOL bState)
{
  CheckDlgButton(hWnd, iCtrl, bState ? BST_CHECKED : BST_UNCHECKED);
}

static void EnableDlgItem(HWND hWnd, int iCtrl, BOOL bEnable)
{
  EnableWindow(GetDlgItem(hWnd, iCtrl), bEnable); 
}

static void UpdateOptionsDlgItemsState(HWND hWnd)
{
  //disable color picker when using default colors
  EnableDlgItem(hWnd, IDC_COLBACK_MESSAGE,  !options->bDefaultColorMsg);
  EnableDlgItem(hWnd, IDC_COLTEXT_MESSAGE,  !options->bDefaultColorMsg);
  EnableDlgItem(hWnd, IDC_COLBACK_URL,      !options->bDefaultColorUrl);
  EnableDlgItem(hWnd, IDC_COLTEXT_URL,      !options->bDefaultColorUrl);
  EnableDlgItem(hWnd, IDC_COLBACK_FILE,     !options->bDefaultColorFile);
  EnableDlgItem(hWnd, IDC_COLTEXT_FILE,     !options->bDefaultColorFile);
  EnableDlgItem(hWnd, IDC_COLBACK_OTHERS,   !options->bDefaultColorOthers);
  EnableDlgItem(hWnd, IDC_COLTEXT_OTHERS,   !options->bDefaultColorOthers);
	//disable merge messages options when is not using
	EnableDlgItem(hWnd, IDC_CHKSHOWDATE,    options->bMergePopup);
	EnableDlgItem(hWnd, IDC_CHKSHOWTIME,    options->bMergePopup);
	EnableDlgItem(hWnd, IDC_CHKSHOWHEADERS, options->bMergePopup);
	EnableDlgItem(hWnd, IDC_CMDEDITHEADERS, options->bMergePopup && options->bShowHeaders);
	EnableDlgItem(hWnd, IDC_NUMBERMSG,      options->bMergePopup);
	EnableDlgItem(hWnd, IDC_LBNUMBERMSG,    options->bMergePopup);
	EnableDlgItem(hWnd, IDC_RDNEW,          options->bMergePopup && options->iNumberMsg);
	EnableDlgItem(hWnd, IDC_RDOLD,          options->bMergePopup && options->iNumberMsg);
	//disable delay textbox when infinite is checked
	EnableDlgItem(hWnd, IDC_DELAY_MESSAGE,  options->iDelayMsg != -1);
	EnableDlgItem(hWnd, IDC_DELAY_URL,      options->iDelayUrl != -1);
	EnableDlgItem(hWnd, IDC_DELAY_FILE,     options->iDelayFile != -1);
	EnableDlgItem(hWnd, IDC_DELAY_OTHERS,   options->iDelayOthers != -1);
}

static BOOL CALLBACK OptionsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
  {
    case WM_INITDIALOG:
      TranslateDialogDefault(hWnd);
      //make dialog represent the current options
      bWmNotify = TRUE;
			SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_SETCOLOUR, 0, options->colBackMsg);
      SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_SETCOLOUR, 0, options->colTextMsg);
      SendDlgItemMessage(hWnd, IDC_COLBACK_URL, CPM_SETCOLOUR, 0, options->colBackUrl);
      SendDlgItemMessage(hWnd, IDC_COLTEXT_URL, CPM_SETCOLOUR, 0, options->colTextUrl);
      SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_SETCOLOUR, 0, options->colBackFile);
      SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_SETCOLOUR, 0, options->colTextFile);
      SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_SETCOLOUR, 0, options->colBackOthers);
      SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_SETCOLOUR, 0, options->colTextOthers);
      SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_MESSAGE, options->bDefaultColorMsg);
      SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_URL,     options->bDefaultColorUrl);
      SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_FILE,    options->bDefaultColorFile);
      SetCheckBoxState(hWnd, IDC_CHKDEFAULTCOL_OTHERS,  options->bDefaultColorOthers);
      SetCheckBoxState(hWnd, IDC_CHKMENUITEM,   options->bMenuitem);
      SetCheckBoxState(hWnd, IDC_CHKDISABLE,    options->bDisable);
      SetCheckBoxState(hWnd, IDC_CHKPREVIEW,    options->bPreview);
			SetCheckBoxState(hWnd, IDC_CHKMERGEPOPUP, options->bMergePopup);
      SetCheckBoxState(hWnd, IDC_CHKNOTIFY_MESSAGE, options->maskNotify & MASK_MESSAGE);
      SetCheckBoxState(hWnd, IDC_CHKNOTIFY_URL,     options->maskNotify & MASK_URL);
      SetCheckBoxState(hWnd, IDC_CHKNOTIFY_FILE,    options->maskNotify & MASK_FILE);
      SetCheckBoxState(hWnd, IDC_CHKNOTIFY_OTHER,   options->maskNotify & MASK_OTHER);
      SetCheckBoxState(hWnd, IDC_CHKACTL_DISMISS,   options->maskActL & MASK_DISMISS);
      SetCheckBoxState(hWnd, IDC_CHKACTL_OPEN,      options->maskActL & MASK_OPEN);
      SetCheckBoxState(hWnd, IDC_CHKACTL_REMOVE,    options->maskActL & MASK_REMOVE);
      SetCheckBoxState(hWnd, IDC_CHKACTR_DISMISS,   options->maskActR & MASK_DISMISS);
      SetCheckBoxState(hWnd, IDC_CHKACTR_OPEN,      options->maskActR & MASK_OPEN);
      SetCheckBoxState(hWnd, IDC_CHKACTR_REMOVE,    options->maskActR & MASK_REMOVE);
			SetCheckBoxState(hWnd, IDC_CHKACTTE_DISMISS,  options->maskActTE & MASK_DISMISS);
      SetCheckBoxState(hWnd, IDC_CHKACTTE_OPEN,     options->maskActTE & MASK_OPEN);
      SetCheckBoxState(hWnd, IDC_CHKACTTE_REMOVE,   options->maskActTE & MASK_REMOVE);
      SetCheckBoxState(hWnd, IDC_CHKWINDOWCHECK,  options->bMsgWindowCheck);
      SetCheckBoxState(hWnd, IDC_CHKREPLYWINDOW,  options->bMsgReplyWindow);
			SetCheckBoxState(hWnd, IDC_CHKSHOWDATE,     options->bShowDate);
			SetCheckBoxState(hWnd, IDC_CHKSHOWTIME,     options->bShowTime);
			SetCheckBoxState(hWnd, IDC_CHKSHOWHEADERS,  options->bShowHeaders);
			SetCheckBoxState(hWnd, IDC_RDNEW,           !options->bShowON);
			SetCheckBoxState(hWnd, IDC_RDOLD,           options->bShowON);
			SetCheckBoxState(hWnd, IDC_CHKHIDESEND,     options->bHideSend);
			SetCheckBoxState(hWnd, IDC_SUPRESSRSS,      options->bNoRSS);
			SetCheckBoxState(hWnd, IDC_READCHECK,       options->bReadCheck);
			SetCheckBoxState(hWnd, IDC_CHKINFINITE_MESSAGE, options->iDelayMsg == -1);
			SetCheckBoxState(hWnd, IDC_CHKINFINITE_URL,     options->iDelayUrl == -1);
			SetCheckBoxState(hWnd, IDC_CHKINFINITE_FILE,    options->iDelayFile == -1);
			SetCheckBoxState(hWnd, IDC_CHKINFINITE_OTHERS,  options->iDelayOthers == -1);
      SetDlgItemInt(hWnd, IDC_DELAY_MESSAGE,  options->iDelayMsg != -1?options->iDelayMsg:0, TRUE);
			SetDlgItemInt(hWnd, IDC_DELAY_URL,      options->iDelayUrl != -1?options->iDelayUrl:0, TRUE);
			SetDlgItemInt(hWnd, IDC_DELAY_FILE,     options->iDelayFile != -1?options->iDelayFile:0, TRUE);
			SetDlgItemInt(hWnd, IDC_DELAY_OTHERS,   options->iDelayOthers != -1?options->iDelayOthers:0, TRUE);
			SetDlgItemInt(hWnd, IDC_NUMBERMSG,      options->iNumberMsg, FALSE);
      //update items' states 
      UpdateOptionsDlgItemsState(hWnd);
      bWmNotify = FALSE;
      return TRUE;

    case WM_COMMAND:
			if (!bWmNotify) 
			{
				switch (LOWORD(wParam))
					{
						case IDC_PREVIEW:
				      PopupPreview(options);
							break;
						default:
						  //update options
							options->maskNotify = (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_MESSAGE)?MASK_MESSAGE:0) |
														        (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_URL)?MASK_URL:0) |
				                            (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_FILE)?MASK_FILE:0) |
					                          (IsDlgButtonChecked(hWnd, IDC_CHKNOTIFY_OTHER)?MASK_OTHER:0);
					    options->maskActL = (IsDlgButtonChecked(hWnd, IDC_CHKACTL_DISMISS)?MASK_DISMISS:0) |
							                    (IsDlgButtonChecked(hWnd, IDC_CHKACTL_OPEN)?MASK_OPEN:0) |
								                  (IsDlgButtonChecked(hWnd, IDC_CHKACTL_REMOVE)?MASK_REMOVE:0);
							options->maskActR = (IsDlgButtonChecked(hWnd, IDC_CHKACTR_DISMISS)?MASK_DISMISS:0) |
	                                (IsDlgButtonChecked(hWnd, IDC_CHKACTR_OPEN)?MASK_OPEN:0) |
		                              (IsDlgButtonChecked(hWnd, IDC_CHKACTR_REMOVE)?MASK_REMOVE:0);
							options->maskActTE = (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_DISMISS)?MASK_DISMISS:0) |
	                                 (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_OPEN)?MASK_OPEN:0) |
		                               (IsDlgButtonChecked(hWnd, IDC_CHKACTTE_REMOVE)?MASK_REMOVE:0);
			        options->bDefaultColorMsg = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_MESSAGE);
				      options->bDefaultColorUrl = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_URL);
					    options->bDefaultColorFile = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_FILE);
						  options->bDefaultColorOthers = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_OTHERS);
			    		options->bMenuitem = IsDlgButtonChecked(hWnd, IDC_CHKMENUITEM);
							options->bDisable = IsDlgButtonChecked(hWnd, IDC_CHKDISABLE);
	            options->bPreview = IsDlgButtonChecked(hWnd, IDC_CHKPREVIEW);
							options->iDelayMsg = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_MESSAGE)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_MESSAGE, NULL, FALSE);
							options->iDelayUrl = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_URL)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_URL, NULL, FALSE);
							options->iDelayFile = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_FILE)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_FILE, NULL, FALSE);
							options->iDelayOthers = IsDlgButtonChecked(hWnd, IDC_CHKINFINITE_OTHERS)?-1:(DWORD)GetDlgItemInt(hWnd, IDC_DELAY_OTHERS, NULL, FALSE);
							options->bMergePopup = IsDlgButtonChecked(hWnd, IDC_CHKMERGEPOPUP);
							options->bMsgWindowCheck = IsDlgButtonChecked(hWnd, IDC_CHKWINDOWCHECK);
							options->bMsgReplyWindow = IsDlgButtonChecked(hWnd, IDC_CHKREPLYWINDOW);
	            options->bShowDate = IsDlgButtonChecked(hWnd, IDC_CHKSHOWDATE);
							options->bShowTime = IsDlgButtonChecked(hWnd, IDC_CHKSHOWTIME);
							options->bShowHeaders = IsDlgButtonChecked(hWnd, IDC_CHKSHOWHEADERS);
							options->bShowON = IsDlgButtonChecked(hWnd, IDC_RDOLD);
							options->bShowON = !IsDlgButtonChecked(hWnd, IDC_RDNEW);
							options->bHideSend = IsDlgButtonChecked(hWnd, IDC_CHKHIDESEND);
							options->iNumberMsg = GetDlgItemInt(hWnd, IDC_NUMBERMSG, NULL, FALSE);
							options->bNoRSS = IsDlgButtonChecked(hWnd, IDC_SUPRESSRSS);
							options->bReadCheck = IsDlgButtonChecked(hWnd, IDC_READCHECK);
              //update items' states
              UpdateOptionsDlgItemsState(hWnd);
							if (HIWORD(wParam) == CPN_COLOURCHANGED)
							{
								options->colBackMsg = SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_GETCOLOUR, 0, 0);
								options->colTextMsg = SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_GETCOLOUR, 0, 0);
								options->colBackUrl = SendDlgItemMessage(hWnd, IDC_COLBACK_URL, CPM_GETCOLOUR, 0, 0);
								options->colTextUrl = SendDlgItemMessage(hWnd, IDC_COLTEXT_URL, CPM_GETCOLOUR, 0, 0);
								options->colBackFile = SendDlgItemMessage(hWnd, IDC_COLBACK_FILE, CPM_GETCOLOUR, 0, 0);
								options->colTextFile = SendDlgItemMessage(hWnd, IDC_COLTEXT_FILE, CPM_GETCOLOUR, 0, 0);
								options->colBackOthers = SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_GETCOLOUR, 0, 0);
								options->colTextOthers = SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_GETCOLOUR, 0, 0);
							}
							//send changes to menuitem
							MenuitemUpdate(!options->bDisable);
							//enable "Apply" button
							SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
							break;
					}
			}
      break;

    case WM_NOTIFY:
      switch (((LPNMHDR)lParam)->code)
				{
          case PSN_APPLY:
            OptionsWrite();
            break;

          case PSN_RESET:
            OptionsRead();

            //maybe something changed with the menuitem
            MenuitemUpdate(!options->bDisable);
            break;
				}
			break;

    default:
      break;
  }
  return FALSE;
}

int OptionsAdd(HINSTANCE hInst, WPARAM addInfo)
{
	OPTIONSDIALOGPAGE odp;
	WCHAR wsTitle[MAX_PATH];
	WCHAR wsGroup[MAX_PATH];

//  if (ServiceExists(MS_POPUP_ADDPOPUP)) {
//  do we need this dialog if popup.dll is not there???

	odp.cbSize = sizeof(odp);
//	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT);
	odp.ptszTitle = NENTranslateT(OPTIONS_TITLE, wsTitle);
	odp.ptszGroup = NENTranslateT(OPTIONS_GROUP, wsGroup);
//	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	if (g_UnicodeCore) odp.flags |= ODPF_UNICODE;
	odp.pfnDlgProc = OptionsDlgProc;
	Options_AddPage( addInfo, &odp);

  return 0;
}

int OptionsInit(PLUGIN_OPTIONS* pluginOptions)
{
  options = pluginOptions;

  OptionsRead();

  return 0;
}

int Opt_DisableNEN(BOOL Status)
{
  options->bDisable = Status;
  OptionsWrite(); // JK: really necessary to write everything here ????
    
    //BUG!
    //Update options dialog if open!

	return 0;
}
