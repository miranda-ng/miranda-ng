#include "gmail.h"

int OpenBrowser(WPARAM hContact, LPARAM lParam)
{
	char *proto = GetContactProto(hContact);
	if (proto && !lstrcmpA(proto, pluginName)) {
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
	
	db_set_ts(curAcc->hContact, "CList", "MyHandle", curAcc->results.content);
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
				dbei.cbBlob = lstrlen(prst->content) + 1;
				dbei.pBlob = (PBYTE)prst->content;
				db_event_add(curAcc->hContact, &dbei);
				prst = prst->next;
			}
		}
		if (opt.notifierOnTray&&newMails > 0) {
			CLISTEVENT cle = { sizeof(cle) };
			cle.hContact = curAcc->hContact;
			cle.hDbEvent = (HANDLE)1;
			cle.flags = CLEF_URGENT | CLEF_TCHAR;
			cle.hIcon = LoadSkinnedProtoIcon(pluginName, ID_STATUS_OCCUPIED);
			cle.pszService = "GmailMNotifier/Notifying";
			cle.ptszTooltip = curAcc->results.next->content;
			CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)curAcc->hContact, (LPARAM)1);
			CallServiceSync(MS_CLIST_ADDEVENT, (WPARAM)curAcc->hContact, (LPARAM)& cle);
		}

		if (opt.notifierOnPop&&newMails > 0) {
			POPUPDATAT ppd = { 0 };

			ppd.lchContact = curAcc->hContact;
			ppd.lchIcon = LoadSkinnedProtoIcon(pluginName, ID_STATUS_OCCUPIED);
			lstrcpy(ppd.lptzContactName, curAcc->results.content);
			resultLink *prst = curAcc->results.next;
			for (int i = 0; i < 5 && i < newMails; i++) {
				_tcscat(ppd.lptzText, prst->content);
				_tcscat(ppd.lptzText, _T("\n"));
				prst = prst->next;
			}
			ppd.colorBack = opt.popupBgColor;
			ppd.colorText = opt.popupTxtColor;
			ppd.PluginWindowProc = PopupDlgProc;
			ppd.PluginData = NULL;
			ppd.iSeconds = opt.popupDuration;
			PUDeletePopup(curAcc->popUpHwnd);
			PUAddPopupT(&ppd);
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
	TCHAR szTempName[MAX_PATH];
	TCHAR buffer[1024];
	TCHAR *str_temp;
	TCHAR lpPathBuffer[1024];
	Account *curAcc = (Account *)lpParam;

	if (GetBrowser(lpPathBuffer)) {
		if (opt.AutoLogin == 0) {
			if (curAcc->hosted[0]) {
				lstrcat(lpPathBuffer, _T("https://mail.google.com/a/"));
				lstrcat(lpPathBuffer, curAcc->hosted);
				lstrcat(lpPathBuffer, _T("/?logout"));
			}
			else {
				lstrcat(lpPathBuffer, _T("https://mail.google.com/mail/?logout"));
			}
		}
		else {
			if (curAcc->hosted[0]) {
				GetTempPath(dwBufSize, buffer);
				GetTempFileName(buffer, _T("gmail"), 0, szTempName);

				hTempFile = CreateFile(szTempName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				lstrcpy(buffer, FORMDATA1);
				lstrcat(buffer, curAcc->hosted);
				lstrcat(buffer, FORMDATA2);
				lstrcat(buffer, curAcc->hosted);
				lstrcat(buffer, FORMDATA3);
				lstrcat(buffer, _T("<input type=hidden name=userName value="));
				lstrcat(buffer, curAcc->name);
				if ((str_temp = _tcsstr(buffer, _T("@"))) != NULL)
					*str_temp = '\0';
				lstrcat(buffer, _T("><input type=hidden name=password value="));
				lstrcat(buffer, curAcc->pass);
				lstrcat(buffer, _T("></form></body>"));
				WriteFile(hTempFile, buffer, lstrlen(buffer), &dwBytesWritten, NULL);
				CloseHandle(hTempFile);
				lstrcat(lpPathBuffer, szTempName);
			}
			else {
				lstrcat(lpPathBuffer, LINK);
				lstrcat(lpPathBuffer, _A2T(mir_urlEncode(_T2A(curAcc->name))));
				lstrcat(lpPathBuffer, _T("&Passwd="));
				lstrcat(lpPathBuffer, _A2T(mir_urlEncode(_T2A(curAcc->pass))));
				if (opt.AutoLogin == 1)
					lstrcat(lpPathBuffer, _T("&PersistentCookie=yes"));
			}
		}
	}

	STARTUPINFO suInfo = { 0 };
	PROCESS_INFORMATION procInfo;
	suInfo.cb = sizeof(suInfo);
	suInfo.wShowWindow = SW_MAXIMIZE;
	if (CreateProcess(NULL, lpPathBuffer, NULL, NULL, FALSE, 0, NULL, NULL, &suInfo, &procInfo))
		CloseHandle(procInfo.hProcess);

	if (curAcc->hosted[0]) {
		Sleep(30000);
		DeleteFile(szTempName);
	}
}
