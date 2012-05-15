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

void showPopupMsg(HANDLE hContact, LPCSTR lpzText, HICON hIcon, int type) {

        // type:
        // 0 - error
        // 1 - key sent
        // 2 - key recv
        // 3 - established
        // 4 - disabled
        // 5 - msg recv
        // 6 - msg sent
        //

	if(!bPopupExists) return;

	char nback[32]; mir_snprintf(nback,sizeof(nback),"popup%dback", $type);
	char ntext[32]; mir_snprintf(ntext,sizeof(ntext),"popup%dtext", $type);
	char ntime[32]; mir_snprintf(ntime,sizeof(ntime),"popup%dtime", $type);

	COLORREF colorBack = (COLORREF)DBGetContactSettingDword(0,szModuleName,nback,(DWORD)RGB(230,230,255));
	COLORREF colorText = (COLORREF)DBGetContactSettingDword(0,szModuleName,ntext,(DWORD)RGB(0,0,0));
	int timeout = (int)DBGetContactSettingWord(0,szModuleName,ntime,0);

	if( bCoreUnicode && bPopupUnicode ) {
		POPUPDATAW ppd;
		memset(&ppd,0,sizeof(POPUPDATAW));
		ppd.lchContact = hContact;
		ppd.lchIcon = hIcon;
		LPWSTR lpwzContactName = (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,GCMDF_UNICODE);
		wcscpy(ppd.lpwzContactName, lpwzContactName);
		LPWSTR lpwzText = mir_a2u(lpzText);
		wcscpy(ppd.lpwzText, TranslateW(lpwzText));
		mir_free(lpwzText);
		ppd.colorBack = colorBack;
		ppd.colorText = colorText;
		ppd.iSeconds = timeout;
//		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
//		ppd.PluginData = NULL;
		CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&ppd, 0);
	}
	else {
		POPUPDATAEX ppd;
		memset(&ppd,0,sizeof(POPUPDATAEX));
		ppd.lchContact = hContact;
		ppd.lchIcon = hIcon;
		LPSTR lpzContactName = (LPSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0);
		strcpy(ppd.lpzContactName, lpzContactName);
		strcpy(ppd.lpzText, Translate(lpzText));
		ppd.colorBack = colorBack;
		ppd.colorText = colorText;
		ppd.iSeconds = timeout;
//		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
//		ppd.PluginData = NULL;
		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
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
