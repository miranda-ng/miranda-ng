#include "gmail.h"

int OpenBrowser(WPARAM hContact, LPARAM lParam)
{
	char *proto = GetContactProto(hContact);
	if (proto && !mir_strcmp(proto, pluginName)) {
		Account *curAcc = GetAccountByContact(hContact);
		PUDeletePopup(curAcc->popUpHwnd);
		CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)curAcc->hContact, (LPARAM)1);
		if (GetKeyState(VK_SHIFT) >> 8 || optionWindowIsOpen)
			return FALSE;

		if (curAcc->oldResults_num != 0) {
			db_set_w(curAcc->hContact, pluginName, "Status", ID_STATUS_NONEW);
			curAcc->oldResults_num = 0;
			DeleteResults(curAcc->results.next);
			curAcc->results.next = NULL;
		}
		mir_forkthread(Login_ThreadFunc, curAcc);
	}
	return FALSE;
}

INT_PTR Notifying(WPARAM, LPARAM lParam)
{
	OpenBrowser(((CLISTEVENT*)lParam)->hContact, 0);
	return 0;
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = PUGetContact(hWnd);
	Account *curAcc = GetAccountByContact(hContact);

	switch (message) {
	case UM_INITPOPUP:
		curAcc->popUpHwnd = hWnd;
		break;
	
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED)
			OpenBrowser((WPARAM)hContact, 0);
		break;
	
	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		curAcc->popUpHwnd = NULL;
		CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)hContact, (LPARAM)1);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void NotifyUser(Account *curAcc)
{
	if (optionWindowIsOpen)
		return;
	
	db_set_s(curAcc->hContact, "CList", "MyHandle", curAcc->results.content);
	switch (curAcc->results_num) {
	case 0:
		PUDeletePopup(curAcc->popUpHwnd);
		CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)curAcc->hContact, (LPARAM)1);
		if (curAcc->oldResults_num != 0)
			db_set_w(curAcc->hContact, pluginName, "Status", ID_STATUS_NONEW);
		break;
	
	case -1:
		db_set_w(curAcc->hContact, pluginName, "Status", ID_STATUS_AWAY);
		break;
	
	default:
		db_set_w(curAcc->hContact, pluginName, "Status", ID_STATUS_OCCUPIED);
		int newMails = (curAcc->oldResults_num == -1) ? (curAcc->results_num) : (curAcc->results_num - curAcc->oldResults_num);
		if (opt.LogThreads&&newMails > 0) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.flags = DBEF_READ;
			dbei.szModule = pluginName;
			dbei.timestamp = time(NULL);

			resultLink *prst = curAcc->results.next;
			for (int i = 0; i < newMails; i++) {
				dbei.cbBlob = mir_strlen(prst->content) + 1;
				dbei.pBlob = (PBYTE)prst->content;
				db_event_add(curAcc->hContact, &dbei);
				prst = prst->next;
			}
		}
		if (opt.notifierOnTray&&newMails > 0) {
			CLISTEVENT cle = { sizeof(cle) };
			cle.hContact = curAcc->hContact;
			cle.hDbEvent = 1;
			cle.flags = CLEF_URGENT;
			cle.hIcon = LoadSkinnedProtoIcon(pluginName, ID_STATUS_OCCUPIED);
			cle.pszService = "GmailMNotifier/Notifying";
			cle.pszTooltip = curAcc->results.next->content;
			CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)curAcc->hContact, (LPARAM)1);
			CallServiceSync(MS_CLIST_ADDEVENT, (WPARAM)curAcc->hContact, (LPARAM)& cle);
		}

		if (opt.notifierOnPop&&newMails > 0) {
			POPUPDATA ppd = { 0 };

			ppd.lchContact = curAcc->hContact;
			ppd.lchIcon = LoadSkinnedProtoIcon(pluginName, ID_STATUS_OCCUPIED);
			mir_strcpy(ppd.lpzContactName, curAcc->results.content);
			resultLink *prst = curAcc->results.next;
			for (int i = 0; i < 5 && i < newMails; i++) {
				strcat(ppd.lpzText, prst->content);
				strcat(ppd.lpzText, "\n");
				prst = prst->next;
			}
			ppd.colorBack = opt.popupBgColor;
			ppd.colorText = opt.popupTxtColor;
			ppd.PluginWindowProc = PopupDlgProc;
			ppd.PluginData = NULL;
			ppd.iSeconds = opt.popupDuration;
			PUDeletePopup(curAcc->popUpHwnd);
			PUAddPopup(&ppd);
		}
		if (newMails > 0)
			SkinPlaySound("Gmail");
	}
	curAcc->oldResults_num = curAcc->results_num;
	DeleteResults(curAcc->results.next);
	curAcc->results.next = NULL;
}

void DeleteResults(resultLink *prst)
{
	if (prst != NULL) {
		if (prst->next != NULL)
			DeleteResults(prst->next);
		free(prst);
	}
}

void __cdecl Login_ThreadFunc(void *lpParam)
{
	if (lpParam == NULL)
		return;

	HANDLE hTempFile;
	DWORD  dwBytesWritten, dwBufSize = 1024;
	char szTempName[MAX_PATH];
	char buffer[1024];
	char *str_temp;
	char lpPathBuffer[1024];
	Account *curAcc = (Account *)lpParam;

	if (GetBrowser(lpPathBuffer)) {
		if (opt.AutoLogin == 0) {
			if (curAcc->hosted[0]) {
				mir_strcat(lpPathBuffer, "https://mail.google.com/a/");
				mir_strcat(lpPathBuffer, curAcc->hosted);
				mir_strcat(lpPathBuffer, "/?logout");
			}
			else {
				mir_strcat(lpPathBuffer, "https://mail.google.com/mail/?logout");
			}
		}
		else {
			if (curAcc->hosted[0]) {
				GetTempPathA(dwBufSize, buffer);
				GetTempFileNameA(buffer, "gmail", 0, szTempName);

				hTempFile = CreateFileA(szTempName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				mir_strcpy(buffer, FORMDATA1);
				mir_strcat(buffer, curAcc->hosted);
				mir_strcat(buffer, FORMDATA2);
				mir_strcat(buffer, curAcc->hosted);
				mir_strcat(buffer, FORMDATA3);
				mir_strcat(buffer, "<input type=hidden name=userName value=");
				mir_strcat(buffer, curAcc->name);
				if ((str_temp = strstr(buffer, "@")) != NULL)
					*str_temp = '\0';
				mir_strcat(buffer, "><input type=hidden name=password value=");
				mir_strcat(buffer, curAcc->pass);
				mir_strcat(buffer, "></form></body>");
				WriteFile(hTempFile, buffer, mir_strlen(buffer), &dwBytesWritten, NULL);
				CloseHandle(hTempFile);
				mir_strcat(lpPathBuffer, szTempName);
			}
			else {
				mir_strcat(lpPathBuffer, LINK);
				mir_strcat(lpPathBuffer, mir_urlEncode(curAcc->name));
				//mir_strcat(lpPathBuffer, "&Passwd=");
				//mir_strcat(lpPathBuffer, mir_urlEncode(curAcc->pass));
				if (opt.AutoLogin == 1)
					mir_strcat(lpPathBuffer, "&PersistentCookie=yes");
			}
		}
	}

	STARTUPINFOA suInfo = { 0 };
	PROCESS_INFORMATION procInfo;
	suInfo.cb = sizeof(suInfo);
	suInfo.wShowWindow = SW_MAXIMIZE;
	if (CreateProcessA(NULL, lpPathBuffer, NULL, NULL, FALSE, 0, NULL, NULL, &suInfo, &procInfo))
		CloseHandle(procInfo.hProcess);

	if (curAcc->hosted[0]) {
		Sleep(30000);
		DeleteFileA(szTempName);
	}
}
