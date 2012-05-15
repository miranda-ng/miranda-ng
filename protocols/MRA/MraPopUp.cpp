#include "Mra.h"
#include "MraPopUp.h"


INT_PTR CALLBACK	MraPopupDlgProcOpts	(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	MraPopupDlgProc		(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);



#define POPUPS_TYPES_COUNT	7
static LPCWSTR lpcwszPopupsTypes[]=
{
	L"None",
	L"Debug",
	L"Information",
	L"Question",
	L"Warning",
	L"Error",
	L"NewMail",
	NULL
};




int MraPopupOptInit(WPARAM wParam,LPARAM lParam)
{
	if (ServiceExists(MS_POPUP_ADDPOPUP)) {
		OPTIONSDIALOGPAGE odp = {0};

		odp.cbSize = sizeof(odp);
		odp.position = 100000000;
		odp.hInstance = masMraSettings.hInstance;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUPS);
		odp.ptszTitle = PROTOCOL_NAMEW;
		odp.ptszGroup = TranslateW(L"Popups");
		odp.groupPosition = 900000000;
		odp.flags = (ODPF_BOLDGROUPS | ODPF_UNICODE);
		odp.nIDBottomSimpleControl = IDC_GROUPMAIN;
		odp.pfnDlgProc = MraPopupDlgProcOpts;
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	}
return(0);
}


INT_PTR CALLBACK MraPopupDlgProcOpts(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			HWND hWndCombo = GetDlgItem(hWndDlg, IDC_COMBO_POPUP_TYPE);
			DWORD dwItem;

			SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
			for (SIZE_T i = 0; i < POPUPS_TYPES_COUNT; i++) {
				dwItem=SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateW(lpcwszPopupsTypes[i]));
				SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, i);
			}
			SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
			SendMessage(hWndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_POPUP_TYPE, CBN_SELCHANGE), 0);

			TranslateDialogDefault(hWndDlg);
		}
		return(TRUE);
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_COMBO_POPUP_TYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				BOOL bEnabled, bUseWinColors;
				char szBuff[MAX_PATH];
				DWORD dwType = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_COMBO_POPUP_TYPE);

				bEnabled = GetBit(DB_Mra_GetDword(NULL, "PopupsEventFilter", MRA_DEFAULT_POPUPS_EVENT_FILTER), dwType);
				CHECK_DLG_BUTTON(hWndDlg, IDC_CHK_ENABLE, bEnabled);

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SUseWinColors", lpcwszPopupsTypes[dwType]);
				bUseWinColors = DB_Mra_GetByte(NULL, szBuff, MRA_DEFAULT_POPUP_USE_WIN_COLORS);
				CHECK_DLG_BUTTON(hWndDlg, IDC_CHK_USE_WIN_COLORS, bUseWinColors);
				EnableWindow(GetDlgItem(hWndDlg, IDC_CHK_USE_WIN_COLORS), bEnabled);

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorBack", lpcwszPopupsTypes[dwType]);
				SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_POPUP_BACKCOLOR, CPM_SETCOLOUR, 0, DB_Mra_GetDword(NULL, szBuff, MRA_DEFAULT_POPUP_COLOR_BACK));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_BACKCOLOR),(bEnabled && bUseWinColors == FALSE));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorText", lpcwszPopupsTypes[dwType]);
				SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_POPUP_TEXTCOLOR, CPM_SETCOLOUR, 0, DB_Mra_GetDword(NULL, szBuff, MRA_DEFAULT_POPUP_COLOR_TEXT));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TEXTCOLOR), (bEnabled && bUseWinColors == FALSE));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%STimeout", lpcwszPopupsTypes[dwType]);
				SetDlgItemInt(hWndDlg, IDC_POPUP_TIMEOUT, DB_Mra_GetDword(NULL, szBuff, MRA_DEFAULT_POPUP_TIMEOUT), FALSE);
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TIMEOUT), bEnabled);
			}
			break;
		case IDC_PREVIEW:
			{
				for (SIZE_T i = 0; i < POPUPS_TYPES_COUNT; i++) {
					MraPopupShowFromAgentW(i, 0, TranslateW(lpcwszPopupsTypes[i]));
				}
			}
			break;
		case IDC_CHK_ENABLE:
		case IDC_CHK_USE_WIN_COLORS:
			{
				BOOL bEnabled, bUseWinColors;

				bEnabled = IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_ENABLE);
				bUseWinColors = IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_USE_WIN_COLORS);

				EnableWindow(GetDlgItem(hWndDlg, IDC_CHK_USE_WIN_COLORS), bEnabled);
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_BACKCOLOR), (bEnabled && bUseWinColors == FALSE));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TEXTCOLOR), (bEnabled && bUseWinColors == FALSE));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TIMEOUT), bEnabled);
			}
		default:
			if ((LOWORD(wParam) == IDC_POPUP_TIMEOUT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()) )
				return(FALSE);
			SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code){
		case PSN_APPLY:
			{
				char szBuff[MAX_PATH];
				DWORD dwPopupsEventFilter, dwType = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_COMBO_POPUP_TYPE);

				dwPopupsEventFilter = DB_Mra_GetDword(NULL, "PopupsEventFilter", MRA_DEFAULT_POPUPS_EVENT_FILTER);
				if (IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_CHK_ENABLE)) {
					dwPopupsEventFilter |= (1<<dwType);
				}else{
					dwPopupsEventFilter &= ~(1<<dwType);
				}
				DB_Mra_SetDword(NULL, "PopupsEventFilter", dwPopupsEventFilter);

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SUseWinColors", lpcwszPopupsTypes[dwType]);
				DB_Mra_SetByte(NULL, szBuff, IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_USE_WIN_COLORS));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorBack", lpcwszPopupsTypes[dwType]);
				DB_Mra_SetDword(NULL, szBuff, SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_POPUP_BACKCOLOR, CPM_GETCOLOUR, 0, 0));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorText", lpcwszPopupsTypes[dwType]);
				DB_Mra_SetDword(NULL, szBuff, SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_POPUP_TEXTCOLOR, CPM_GETCOLOUR, 0, 0));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%STimeout", lpcwszPopupsTypes[dwType]);
				DB_Mra_SetDword(NULL, szBuff, GetDlgItemInt(hWndDlg, IDC_POPUP_TIMEOUT, NULL, FALSE));
			}
			return(TRUE);
		}
		break;
	}
return(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// ShowPopup - popup plugin support
void CALLBACK MraPopupThreadMarandaCallback(ULONG_PTR dwParam)
{
	if (dwParam == 0)
		return;

	if ((SIZE_T)(((POPUPDATAW*)dwParam)->PluginData) == MRA_POPUP_TYPE_EMAIL_STATUS && masMraSettings.hWndEMailPopupStatus) {
		// 1: PUDeletePopUp(hWndDlg);
		// 1: PUAddPopUpW((POPUPDATAW*)dwParam);
		// 2: PUChangeW(masMraSettings.hWndEMailPopupStatus,(POPUPDATAW*)dwParam); //- crash :/
		PUChangeTextW(masMraSettings.hWndEMailPopupStatus,((POPUPDATAW*)dwParam)->lpwzText);
	}else{
		PUAddPopUpW((POPUPDATAW*)dwParam);
	}

	MEMFREE(dwParam);

}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup plugin window proc
LRESULT CALLBACK MraPopupDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case UM_INITPOPUP:
		if ((SIZE_T)PUGetPluginData(hWndDlg) == MRA_POPUP_TYPE_EMAIL_STATUS) {
			// update/load avatar
			MraAvatarsQueueGetAvatarSimple(masMraSettings.hAvatarsQueueHandle, GAIF_FORCE, PUGetContact(hWndDlg), 0);
			
			// delete old email popup
			if (masMraSettings.hWndEMailPopupStatus)
				PUDeletePopUp(masMraSettings.hWndEMailPopupStatus);

			masMraSettings.hWndEMailPopupStatus = hWndDlg;
		}
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
			if ((SIZE_T)PUGetPluginData(hWndDlg) == MRA_POPUP_TYPE_EMAIL_STATUS)
			{
				CallProtoService(PROTOCOL_NAMEA, MRA_GOTO_INBOX, 0, 0);
				masMraSettings.hWndEMailPopupStatus = NULL;
			}
		}
		PUDeletePopUp(hWndDlg);
		break;
	case WM_CONTEXTMENU:
		if ((SIZE_T)PUGetPluginData(hWndDlg) == MRA_POPUP_TYPE_EMAIL_STATUS)
			masMraSettings.hWndEMailPopupStatus = NULL;
		PUDeletePopUp(hWndDlg);
		break;
	case UM_FREEPLUGINDATA:
		if ((SIZE_T)PUGetPluginData(hWndDlg) == MRA_POPUP_TYPE_EMAIL_STATUS)
			masMraSettings.hWndEMailPopupStatus = NULL;
		break;
	}
return(DefWindowProc(hWndDlg, msg, wParam, lParam));
}


void MraPopupShowFromContactW(HANDLE hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszMessage)
{
	WCHAR szNick[MAX_EMAIL_LEN], szEMail[MAX_EMAIL_LEN], szTitle[MAX_CONTACTNAME];
	
	DB_Mra_GetStaticStringW(hContact, "Nick", szNick, SIZEOF(szNick), NULL);
	DB_Mra_GetStaticStringW(hContact, "e-mail", szEMail, SIZEOF(szEMail), NULL);
	if (hContact) {
		mir_sntprintf(szTitle, SIZEOF(szTitle), L"%s <%s>",szNick, szEMail);
	}else{
		mir_sntprintf(szTitle, SIZEOF(szTitle), L"%s:  %s <%s>", PROTOCOL_DISPLAY_NAMEW, szNick, szEMail);
	}
	MraPopupShowW(hContact, dwType, dwFlags, szTitle, lpszMessage);
}


void MraPopupShowW(HANDLE hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszTitle, LPWSTR lpszMessage)
{
	if (DB_Mra_GetByte(NULL,"PopupsEnabled", MRA_DEFAULT_POPUPS_ENABLED))
	if (GetBit(DB_Mra_GetDword(NULL,"PopupsEventFilter", MRA_DEFAULT_POPUPS_EVENT_FILTER), dwType))
	if (ServiceExists(MS_POPUP_ADDPOPUPW))
	{
		BOOL bUseWinColors;
		char szBuff[MAX_PATH];
		POPUPDATAW *ppd = (POPUPDATAW*)MEMALLOC(sizeof(POPUPDATAW));

		//if (ServiceExists(MS_POPUP2_SHOW)==FALSE)// yapp used
		if (dwType == MRA_POPUP_TYPE_EMAIL_STATUS && masMraSettings.hWndEMailPopupStatus) {
			// delete old email popup
			PUDeletePopUp(masMraSettings.hWndEMailPopupStatus);
			masMraSettings.hWndEMailPopupStatus = NULL;
		}

		// load icon
		switch(dwType){
		case MRA_POPUP_TYPE_NONE:// proto icon
			ppd->lchIcon = (HICON)LoadImage(masMraSettings.hInstance, MAKEINTRESOURCE(IDI_MRA), IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_DEBUG:// IDI_APPLICATION
			ppd->lchIcon = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_INFORMATION:// IDI_INFORMATION
			ppd->lchIcon = (HICON)LoadImage(NULL, IDI_INFORMATION, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_QUESTION:// IDI_QUESTION
			ppd->lchIcon = (HICON)LoadImage(NULL, IDI_QUESTION, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_WARNING:// IDI_WARNING
			ppd->lchIcon = (HICON)LoadImage(NULL, IDI_WARNING, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_ERROR:// IDI_ERROR
			ppd->lchIcon = (HICON)LoadImage(NULL, IDI_ERROR, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_EMAIL_STATUS:
			ppd->lchIcon = (HICON)LoadImage(masMraSettings.hInstance, MAKEINTRESOURCE(IDI_MAIL_NOTIFY), IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		}

		ppd->lchContact = hContact;
		if (lpszTitle)
			lstrcpynW(ppd->lpwzContactName, lpszTitle, SIZEOF(ppd->lpwzContactName));
		if (lpszMessage)
			lstrcpynW(ppd->lpwzText, lpszMessage, SIZEOF(ppd->lpwzText));
		ppd->PluginWindowProc = (WNDPROC)MraPopupDlgProc;
		ppd->PluginData = (LPVOID)dwType;

		mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SUseWinColors", lpcwszPopupsTypes[dwType]);
		bUseWinColors = DB_Mra_GetByte(NULL, szBuff, MRA_DEFAULT_POPUP_USE_WIN_COLORS);
		if (bUseWinColors) {
			ppd->colorBack = GetSysColor(COLOR_BTNFACE);
			ppd->colorText = GetSysColor(COLOR_WINDOWTEXT);
		}else{
			mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorBack", lpcwszPopupsTypes[dwType]);
			ppd->colorBack = DB_Mra_GetDword(NULL, szBuff, MRA_DEFAULT_POPUP_COLOR_BACK);

			mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorText", lpcwszPopupsTypes[dwType]);
			ppd->colorText = DB_Mra_GetDword(NULL, szBuff, MRA_DEFAULT_POPUP_COLOR_TEXT);
		}

		mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%STimeout", lpcwszPopupsTypes[dwType]);
		ppd->iSeconds = DB_Mra_GetDword(NULL, szBuff, MRA_DEFAULT_POPUP_TIMEOUT);

		//if (MraAPCQueueAdd(MraPopupThreadMarandaCallback,&masMraSettings.ffmtAPCQueue,(ULONG_PTR)ppd)!=NO_ERROR)
		{// sync call
			MraPopupThreadMarandaCallback((ULONG_PTR)ppd);
		}
	}else{
		if (dwFlags & MRA_POPUP_ALLOW_MSGBOX)
			MessageBox(NULL, lpszMessage, lpszTitle, MB_OK+(dwType == MRA_POPUP_TYPE_WARNING)?MB_ICONERROR:MB_ICONINFORMATION);
	}
}



