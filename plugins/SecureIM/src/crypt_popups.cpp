#include "commonheaders.h"

// type=0 key colors
// type=1 session colors
// type=2 SR colors

void showPopup(LPCSTR lpzText, MCONTACT hContact, HICON hIcon, UINT type)
{
	if (!bPopupExists) return;

	COLORREF colorBack, colorText;
	int timeout = 0;

	DBVARIANT dbv;

	if (type == 0) {
		colorBack = db_get_dw(0, MODULENAME, "colorKeyb", RGB(230, 230, 255));
		colorText = db_get_dw(0, MODULENAME, "colorKeyt", RGB(0, 0, 0));
		if (!db_get_s(0, MODULENAME, "timeoutKey", &dbv)) {
			timeout = atoi(dbv.pszVal);
			db_free(&dbv);
		}
	}
	else if (type == 1) {
		colorBack = db_get_dw(0, MODULENAME, "colorSecb", RGB(255, 255, 200));
		colorText = db_get_dw(0, MODULENAME, "colorSect", RGB(0, 0, 0));
		if (!db_get_s(0, MODULENAME, "timeoutSec", &dbv)) {
			timeout = atoi(dbv.pszVal);
			db_free(&dbv);
		}
	}
	else if (type >= 2) {
		colorBack = db_get_dw(0, MODULENAME, "colorSRb", RGB(200, 255, 200));
		colorText = db_get_dw(0, MODULENAME, "colorSRt", RGB(0, 0, 0));
		if (!db_get_s(0, MODULENAME, "timeoutSR", &dbv)) {
			timeout = atoi(dbv.pszVal);
			db_free(&dbv);
		}
	}

	POPUPDATAW ppd = { 0 };
	ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
	ppd.lchIcon = hIcon;
	LPWSTR lpwzContactName = (LPWSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GSMDF_UNICODE);
	wcscpy(ppd.lpwzContactName, lpwzContactName);
	LPWSTR lpwzText = mir_a2u(lpzText);
	wcscpy(ppd.lpwzText, TranslateW(lpwzText));
	mir_free(lpwzText);
	ppd.colorBack = colorBack;
	ppd.colorText = colorText;
	ppd.iSeconds = timeout;
	PUAddPopupW(&ppd);
}

void showPopupDCmsg(MCONTACT hContact, LPCSTR msg)
{
	if (db_get_b(0, MODULENAME, "dc", 1))
		showPopup(msg, hContact, g_hPOP[POP_PU_DIS], 1);
}

void showPopupDC(MCONTACT hContact)
{
	if (db_get_b(0, MODULENAME, "dc", 1))
		showPopup(sim006, hContact, g_hPOP[POP_PU_DIS], 1);
}

void showPopupEC(MCONTACT hContact)
{
	if (db_get_b(0, MODULENAME, "ec", 1))
		showPopup(sim001, hContact, g_hPOP[POP_PU_EST], 1);
}

void showPopupKS(MCONTACT hContact)
{
	if (db_get_b(0, MODULENAME, "ks", 1))
		showPopup(sim007, hContact, g_hPOP[POP_PU_PRC], 0);
}

void showPopupKRmsg(MCONTACT hContact, LPCSTR msg)
{
	if (db_get_b(0, MODULENAME, "kr", 1))
		showPopup(msg, hContact, g_hPOP[POP_PU_PRC], 0);
}

void showPopupKR(MCONTACT hContact)
{
	if (db_get_b(0, MODULENAME, "kr", 1))
		showPopup(sim008, hContact, g_hPOP[POP_PU_PRC], 0);
}

void showPopupSM(MCONTACT hContact)
{
	if (db_get_b(0, MODULENAME, "ss", 0))
		showPopup(sim009, hContact, g_hPOP[POP_PU_MSS], 2);
	SkinPlaySound("OutgoingSecureMessage");
}

void showPopupRM(MCONTACT hContact)
{
	if (db_get_b(0, MODULENAME, "sr", 0))
		showPopup(sim010, hContact, g_hPOP[POP_PU_MSR], 2);
	SkinPlaySound("IncomingSecureMessage");
}
