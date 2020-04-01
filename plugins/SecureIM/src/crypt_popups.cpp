#include "commonheaders.h"

// type=0 key colors
// type=1 session colors
// type=2 SR colors

void showPopup(LPCSTR lpzText, MCONTACT hContact, HICON hIcon, UINT type)
{
	COLORREF colorBack, colorText;
	int timeout = 0;

	DBVARIANT dbv;

	if (type == 0) {
		colorBack = g_plugin.getDword("colorKeyb", RGB(230, 230, 255));
		colorText = g_plugin.getDword("colorKeyt", RGB(0, 0, 0));
		if (!g_plugin.getString("timeoutKey", &dbv)) {
			timeout = atoi(dbv.pszVal);
			db_free(&dbv);
		}
	}
	else if (type == 1) {
		colorBack = g_plugin.getDword("colorSecb", RGB(255, 255, 200));
		colorText = g_plugin.getDword("colorSect", RGB(0, 0, 0));
		if (!g_plugin.getString("timeoutSec", &dbv)) {
			timeout = atoi(dbv.pszVal);
			db_free(&dbv);
		}
	}
	else if (type >= 2) {
		colorBack = g_plugin.getDword("colorSRb", RGB(200, 255, 200));
		colorText = g_plugin.getDword("colorSRt", RGB(0, 0, 0));
		if (!g_plugin.getString("timeoutSR", &dbv)) {
			timeout = atoi(dbv.pszVal);
			db_free(&dbv);
		}
	}

	POPUPDATAW ppd;
	ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
	ppd.lchIcon = hIcon;
	LPWSTR lpwzContactName = Clist_GetContactDisplayName(hContact);
	wcsncpy(ppd.lpwzContactName, lpwzContactName, MAX_CONTACTNAME-1);
	LPWSTR lpwzText = mir_a2u(lpzText);
	wcsncpy(ppd.lpwzText, TranslateW(lpwzText),MAX_SECONDLINE-1);
	mir_free(lpwzText);
	ppd.colorBack = colorBack;
	ppd.colorText = colorText;
	ppd.iSeconds = timeout;
	PUAddPopupW(&ppd);
}

void showPopupDCmsg(MCONTACT hContact, LPCSTR msg)
{
	if (g_plugin.getByte("dc", 1))
		showPopup(msg, hContact, g_hPOP[POP_PU_DIS], 1);
}

void showPopupDC(MCONTACT hContact)
{
	if (g_plugin.getByte("dc", 1))
		showPopup(LPGEN("SecureIM disabled..."), hContact, g_hPOP[POP_PU_DIS], 1);
}

void showPopupEC(MCONTACT hContact)
{
	if (g_plugin.getByte("ec", 1))
		showPopup(LPGEN("SecureIM established..."), hContact, g_hPOP[POP_PU_EST], 1);
}

void showPopupKS(MCONTACT hContact)
{
	if (g_plugin.getByte("ks", 1))
		showPopup(LPGEN("Sending key..."), hContact, g_hPOP[POP_PU_PRC], 0);
}

void showPopupKRmsg(MCONTACT hContact, LPCSTR msg)
{
	if (g_plugin.getByte("kr", 1))
		showPopup(msg, hContact, g_hPOP[POP_PU_PRC], 0);
}

void showPopupKR(MCONTACT hContact)
{
	if (g_plugin.getByte("kr", 1))
		showPopup(LPGEN("Key received..."), hContact, g_hPOP[POP_PU_PRC], 0);
}

void showPopupSM(MCONTACT hContact)
{
	if (g_plugin.getByte("ss", 0))
		showPopup(LPGEN("Sending message..."), hContact, g_hPOP[POP_PU_MSS], 2);
	Skin_PlaySound("OutgoingSecureMessage");
}

void showPopupRM(MCONTACT hContact)
{
	if (g_plugin.getByte("sr", 0))
		showPopup(LPGEN("Message received..."), hContact, g_hPOP[POP_PU_MSR], 2);
	Skin_PlaySound("IncomingSecureMessage");
}
