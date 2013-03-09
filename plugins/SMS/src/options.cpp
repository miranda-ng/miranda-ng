/*
Miranda-IM SMS Plugin
Copyright (C) 2001-2002  Richard Hughes
Copyright (C) 2007-2009  Rozhuk Ivan

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
---------------------------------------------------------------------------

This was the original words.
This plugin was modified by Ariel Shulman (NuKe007).
For any comments, problems, etc. contact me at Miranda-IM forums or E-Mail or ICQ.
All the information needed you can find at www.nuke007.tk
Enjoy the code and use it smartly!
*/

#include "common.h"

WORD wSMSSignControlsList[] = { IDC_BEGIN, IDC_END, IDC_SIGNATURE, IDC_SIGNGROUP };

INT_PTR CALLBACK DlgProcEditorOptions(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WCHAR wszSign[1024];

	switch(msg){
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);

		if (DB_SMS_GetStaticStringW(NULL,"Signature",wszSign,SIZEOF(wszSign),NULL)==FALSE)
			mir_sntprintf(wszSign,SIZEOF(wszSign),TranslateT("From %s:\r\n\r\n"),GetContactNameW(NULL));

		SET_DLG_ITEM_TEXTW(hWndDlg,IDC_SIGNATURE,wszSign);
		{
			BOOL bUseSign = DB_SMS_GetByte(NULL,"UseSignature",SMS_DEFAULT_USESIGNATURE);
			CHECK_DLG_BUTTON(hWndDlg,IDC_USESIGNATURE,bUseSign);
			EnableControlsArray(hWndDlg,(WORD*)&wSMSSignControlsList,SIZEOF(wSMSSignControlsList),bUseSign);

			BOOL bSignBebefore=DB_SMS_GetByte(NULL,"SignaturePos",SMS_DEFAULT_SIGNATUREPOS);
			CHECK_DLG_BUTTON(hWndDlg,IDC_BEGIN,bSignBebefore);
			CHECK_DLG_BUTTON(hWndDlg,IDC_END,(!bSignBebefore));
		}

		CHECK_DLG_BUTTON(hWndDlg,IDC_SHOWACK,DB_SMS_GetByte(NULL,"ShowACK",SMS_DEFAULT_SHOWACK));
		CHECK_DLG_BUTTON(hWndDlg,IDC_AUTOPOP,DB_SMS_GetByte(NULL,"AutoPopup",SMS_DEFAULT_AUTOPOP));
		CHECK_DLG_BUTTON(hWndDlg,IDC_SAVEWINPOS,DB_SMS_GetByte(NULL,"SavePerContact",SMS_DEFAULT_SAVEWINPOS));
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hWndDlg),PSM_CHANGED,0,0);
		switch(LOWORD(wParam)){
		case IDC_USESIGNATURE:
			EnableControlsArray(hWndDlg,(WORD*)&wSMSSignControlsList,SIZEOF(wSMSSignControlsList),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_USESIGNATURE));
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom){
		case 0:
			switch (((LPNMHDR)lParam)->code){
			case PSN_APPLY:
				GET_DLG_ITEM_TEXTW(hWndDlg,IDC_SIGNATURE,wszSign,SIZEOF(wszSign));
				DB_SMS_SetStringW(NULL,"Signature",wszSign);

				DB_SMS_SetByte(NULL,"UseSignature",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_USESIGNATURE));
				DB_SMS_SetByte(NULL,"SignaturePos",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_BEGIN));
				DB_SMS_SetByte(NULL,"ShowACK",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_SHOWACK));
				DB_SMS_SetByte(NULL,"AutoPopup",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTOPOP));
				DB_SMS_SetByte(NULL,"SavePerContact",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_SAVEWINPOS));
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

int OptInitialise(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 910000000;
	odp.hInstance = ssSMSSettings.hInstance;
	odp.pszGroup = "Events";
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SMSPLUGIN);
	odp.pszTitle = PROTOCOL_DISPLAY_NAME_ORIG;
	odp.pfnDlgProc = DlgProcEditorOptions;
	Options_AddPage(wParam, &odp);	
	return 0;
}

