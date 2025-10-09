#include "stdafx.h"

static void __cdecl Login_ThreadFunc(Account *pAcc)
{
	if (pAcc == nullptr)
		return;

	HANDLE hTempFile;
	DWORD dwBytesWritten, dwBufSize = 1024;
	char szTempName[MAX_PATH];

	auto pszHosted = strchr(pAcc->szName, '@');
	if (pszHosted && !strcmp(pszHosted + 1, "gmail.com"))
		pszHosted = nullptr;
	else
		pszHosted++;

	CMStringA szUrl;
	if (g_plugin.AutoLogin != 0) {
		if (pszHosted)
			szUrl.Format("https://mail.google.com/a/%s/?logout", pszHosted);
		else
			szUrl = "https://mail.google.com/mail/?logout";
	}
	else {
		if (pszHosted) {
			char buffer[1024];
			GetTempPathA(dwBufSize, buffer);
			GetTempFileNameA(buffer, "gmail", 0, szTempName);

			hTempFile = CreateFileA(szTempName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			mir_strcpy(buffer, FORMDATA1);
			mir_strcat(buffer, pszHosted);
			mir_strcat(buffer, FORMDATA2);
			mir_strcat(buffer, pszHosted);
			mir_strcat(buffer, FORMDATA3);
			mir_strcat(buffer, "<input type=hidden name=userName value=");
			mir_strcat(buffer, pAcc->szName);
			if (auto *p = strstr(buffer, "@"))
				*p = '\0';
			mir_strcat(buffer, "><input type=hidden name=password value=");
			mir_strcat(buffer, "");
			mir_strcat(buffer, "></form></body>");
			WriteFile(hTempFile, buffer, (uint32_t)mir_strlen(buffer), &dwBytesWritten, nullptr);
			CloseHandle(hTempFile);
			szUrl = szTempName;
		}
		else {
			szUrl = "https://accounts.google.com/ServiceLogin?continue=https%3A%2F%2Fmail.google.com%2Fmail&service=mail&passive=true&Email=";
			szUrl += mir_urlEncode(pAcc->szName);
			if (g_plugin.AutoLogin == 1)
				szUrl += "&PersistentCookie=yes";
		}
	}

	Utils_OpenUrl(szUrl);

	if (pszHosted) {
		Sleep(30000);
		DeleteFileA(szTempName);
	}
}

int OpenBrowser(WPARAM hContact, LPARAM)
{
	char *proto = Proto_GetBaseAccountName(hContact);
	if (proto && !mir_strcmp(proto, MODULENAME)) {
		Account *pAcc = GetAccountByContact(hContact);
		PUDeletePopup(pAcc->popUpHwnd);
		Clist_RemoveEvent(pAcc->hContact, 1);
		mir_forkThread<Account>(Login_ThreadFunc, pAcc);
	}
	return FALSE;
}

INT_PTR Notifying(WPARAM, LPARAM lParam)
{
	OpenBrowser(((CLISTEVENT*)lParam)->hContact, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = PUGetContact(hWnd);
	Account *pAcc = GetAccountByContact(hContact);

	switch (message) {
	case UM_INITPOPUP:
		pAcc->popUpHwnd = hWnd;
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED)
			OpenBrowser((WPARAM)hContact, 0);
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		pAcc->popUpHwnd = nullptr;
		Clist_RemoveEvent(hContact, 1);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void NotifyUser(Account *pAcc)
{
	db_set_ws(pAcc->hContact, "CList", "MyHandle", pAcc->wszBrief);
	if (pAcc->bError)
		g_plugin.setWord(pAcc->hContact, "Status", ID_STATUS_AWAY);
	else if (!pAcc->arEmails.getCount()) {
		PUDeletePopup(pAcc->popUpHwnd);
		Clist_RemoveEvent(pAcc->hContact, 1);
		g_plugin.setWord(pAcc->hContact, "Status", ID_STATUS_NONEW);
	}
	else {
		g_plugin.setWord(pAcc->hContact, "Status", ID_STATUS_OCCUPIED);
		if (g_plugin.bLogThreads) {
			DBEVENTINFO dbei = {};
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.flags = DBEF_READ | DBEF_UTF;
			dbei.szModule = MODULENAME;
			dbei.iTimestamp = time(0);

			for (auto &it : pAcc->arEmails) {
				if (MEVENT hDbEvent = db_event_getById(MODULENAME, it->id))
					continue;

				T2Utf szBody(it->wszText);
				dbei.cbBlob = (uint32_t)mir_strlen(szBody);
				dbei.pBlob = szBody;
				db_event_add(pAcc->hContact, &dbei);
			}
		}

		if (g_plugin.bNotifierOnTray) {
			Clist_RemoveEvent(pAcc->hContact, 1);

			CLISTEVENT cle = {};
			cle.hContact = pAcc->hContact;
			cle.hDbEvent = 1;
			cle.flags = CLEF_URGENT | CLEF_UNICODE;
			cle.hIcon = Skin_LoadProtoIcon(MODULENAME, ID_STATUS_OCCUPIED);
			cle.pszService = "GmailMNotifier/Notifying";
			cle.szTooltip.w = pAcc->arEmails[0].wszText;
			g_clistApi.pfnAddEvent(&cle);
		}

		if (g_plugin.bNotifierOnPop) {
			POPUPDATAW ppd;
			ppd.lchContact = pAcc->hContact;
			ppd.lchIcon = Skin_LoadProtoIcon(MODULENAME, ID_STATUS_OCCUPIED);
			mir_wstrcpy(ppd.lpwzContactName, pAcc->wszBrief);
			for (int i = 0; i < 5 && i < pAcc->arEmails.getCount(); i++) {
				mir_wstrcat(ppd.lpwzText, pAcc->arEmails[i].wszText);
				mir_wstrcat(ppd.lpwzText, L"\n");
			}
			ppd.colorBack = g_plugin.popupBgColor;
			ppd.colorText = g_plugin.popupTxtColor;
			ppd.PluginWindowProc = PopupDlgProc;
			ppd.PluginData = nullptr;
			ppd.iSeconds = g_plugin.popupDuration;
			PUDeletePopup(pAcc->popUpHwnd);
			PUAddPopupW(&ppd);
		}

		Skin_PlaySound("Gmail");
	}

	pAcc->arEmails.destroy();
}
