#include "commonheaders.h"

/*
static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_COMMAND:
			if (wParam == STN_CLICKED) { // It was a click on the Popup.
				PUDeletePopUp(hWnd);
				return TRUE;
			}
			break;
		case UM_FREEPLUGINDATA: {
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
			}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
*/

void showPopUp(LPCSTR lpzText,HANDLE hContact,HICON hIcon, UINT type) {
	//type=0 key colors
	//type=1 session colors
	//type=2 SR colors

	if(!bPopupExists) return;

	//hContact = A_VALID_HANDLE_YOU_GOT_FROM_SOMEWHERE;
	COLORREF colorBackKey = RGB(230,230,255);
	COLORREF colorTextKey = RGB(0,0,0);
	COLORREF colorBackSec = RGB(255,255,200);
	COLORREF colorTextSec = RGB(0,0,0);
	COLORREF colorBackSR = RGB(200,255,200);
	COLORREF colorTextSR = RGB(0,0,0);
	COLORREF colorBack = 0;
	COLORREF colorText = 0;
	int timeout=0;
	int res;

	DBVARIANT dbv_timeout;

	if (type==0) {
		colorBack=DBGetContactSettingDword(0,szModuleName,"colorKeyb",(UINT)-1);
		colorText=DBGetContactSettingDword(0,szModuleName,"colorKeyt",(UINT)-1);
		if (colorBack==(UINT)-1) colorBack=colorBackKey;
		if (colorText==(UINT)-1) colorText=colorTextKey;

		res=DBGetContactSetting(0,szModuleName,"timeoutKey",&dbv_timeout);
		if (res==0) timeout=atoi(dbv_timeout.pszVal);
		DBFreeVariant(&dbv_timeout);
	}
	else if (type==1) {
		colorBack=DBGetContactSettingDword(0,szModuleName,"colorSecb",(UINT)-1);
		colorText=DBGetContactSettingDword(0,szModuleName,"colorSect",(UINT)-1);
		if (colorBack==(UINT)-1) colorBack=colorBackSec;
		if (colorText==(UINT)-1) colorText=colorTextSec;

		res=DBGetContactSetting(0,szModuleName,"timeoutSec",&dbv_timeout);
		if (res==0) timeout=atoi(dbv_timeout.pszVal);
		DBFreeVariant(&dbv_timeout);
	}
	else if (type>=2) {
		colorBack=DBGetContactSettingDword(0, szModuleName, "colorSRb", (UINT)-1);
		colorText=DBGetContactSettingDword(0, szModuleName, "colorSRt", (UINT)-1);
		if (colorBack==(UINT)-1) colorBack=colorBackSR;
		if (colorText==(UINT)-1) colorText=colorTextSR;

		res=DBGetContactSetting(0,szModuleName,"timeoutSR",&dbv_timeout);
		if (res==0) timeout=atoi(dbv_timeout.pszVal);
		DBFreeVariant(&dbv_timeout);
	}

	if( bCoreUnicode && bPopupUnicode ) {
		POPUPDATAW ppd = {0};

		ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
		ppd.lchIcon = hIcon;
		LPWSTR lpwzContactName = (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,GSMDF_UNICODE);
		wcscpy(ppd.lpwzContactName, lpwzContactName);
		LPWSTR lpwzText = mir_a2u(lpzText);
		wcscpy(ppd.lpwzText, TranslateW(lpwzText));
		mir_free(lpwzText);
		ppd.colorBack = colorBack;
		ppd.colorText = colorText;
		ppd.iSeconds = timeout;
//		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;

		//Now that the plugin data has been filled, we add it to the PopUpData.
//		ppd.PluginData = NULL;

		//Now that every field has been filled, we want to see the popup.
		PUAddPopUpW(&ppd);
	}
	else {
		POPUPDATAEX ppd = {0};

		ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
		ppd.lchIcon = hIcon;
		LPSTR lpzContactName = (LPSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0);
		strcpy(ppd.lpzContactName, lpzContactName);
		strcpy(ppd.lpzText, Translate(lpzText));
		ppd.colorBack = colorBack;
		ppd.colorText = colorText;
		ppd.iSeconds = timeout;
//		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;

		//Now that the plugin data has been filled, we add it to the PopUpData.
//		ppd.PluginData = NULL;

		//Now that every field has been filled, we want to see the popup.
		PUAddPopUpEx(&ppd);
	}
}


void showPopUpDCmsg(HANDLE hContact,LPCSTR msg) {
	int indic=DBGetContactSettingByte(0, szModuleName, "dc",1);
	if (indic==1) showPopUp(msg,hContact,g_hPOP[POP_PU_DIS],1);
}
void showPopUpDC(HANDLE hContact) {
	int indic=DBGetContactSettingByte(0, szModuleName, "dc",1);
	if (indic==1) showPopUp(sim006,hContact,g_hPOP[POP_PU_DIS],1);
}
void showPopUpEC(HANDLE hContact) {
	int indic=DBGetContactSettingByte(0, szModuleName, "ec",1);
	if (indic==1) showPopUp(sim001,hContact,g_hPOP[POP_PU_EST],1);
}
void showPopUpKS(HANDLE hContact) {
	int indic=DBGetContactSettingByte(0, szModuleName, "ks",1);
	if (indic==1) showPopUp(sim007,hContact,g_hPOP[POP_PU_PRC],0);
}
void showPopUpKRmsg(HANDLE hContact,LPCSTR msg) {
	int indic=DBGetContactSettingByte(0, szModuleName, "kr",1);
	if (indic==1) showPopUp(msg,hContact,g_hPOP[POP_PU_PRC],0);
}
void showPopUpKR(HANDLE hContact) {
	int indic=DBGetContactSettingByte(0, szModuleName, "kr",1);
	if (indic==1) showPopUp(sim008,hContact,g_hPOP[POP_PU_PRC],0);
}
void showPopUpSM(HANDLE hContact) {
	int indic=DBGetContactSettingByte(0, szModuleName, "ss",0);
	if (indic==1) showPopUp(sim009,hContact,g_hPOP[POP_PU_MSS],2);
	SkinPlaySound("OutgoingSecureMessage");
}
void showPopUpRM(HANDLE hContact) {
	int indic=DBGetContactSettingByte(0, szModuleName, "sr",0);
	if (indic==1) showPopUp(sim010,hContact,g_hPOP[POP_PU_MSR],2);
	SkinPlaySound("IncomingSecureMessage");
}


// EOF
