#include "stdafx.h"

INT_PTR exportContacts(WPARAM, LPARAM)
{
	char fn[MAX_PATH];
	if (!Openfile(fn, 0))
		return 0;

	FILE* file;
	if (MessageBox(nullptr, TranslateT("Do you want to overwrite the contents of the file?\r\n\r\nPressing No will append these contacts to the end of the file."), _A2W(modFullname), MB_YESNO) == IDYES)
		file = fopen(fn, "w");
	else
		file = fopen(fn, "a");
	if (!file)
		return 0;

	for (auto &hContact : Contacts(MODNAME)) {
		int tmp;
		char DBVar[1024];
		if (!db_get_static(hContact, MODNAME, "Name", DBVar, _countof(DBVar))) {
			fprintf(file, "\r\n[Non-IM Contact]\r\nName=%s\r\n", DBVar);
			if (!db_get_static(hContact, MODNAME, "ProgramString", DBVar, _countof(DBVar)))
				fprintf(file, "ProgramString=%s\r\n", DBVar);
			if (!db_get_static(hContact, MODNAME, "ProgramParamString", DBVar, _countof(DBVar)))
				fprintf(file, "ProgramParamString=%s\r\n", DBVar);
			if (!db_get_static(hContact, MODNAME, "ToolTip", DBVar, _countof(DBVar)))
				fprintf(file, "ToolTip=%s</tooltip>\r\n", DBVar);
			if (!db_get_static(hContact, "CList", "Group", DBVar, _countof(DBVar)))
				fprintf(file, "Group=%s\r\n", DBVar);
			if (tmp = g_plugin.getWord(hContact, "Icon", 40072))
				fprintf(file, "Icon=%d\r\n", tmp);
			if (tmp = g_plugin.getByte(hContact, "UseTimer", 0))
				fprintf(file, "UseTimer=%d\r\n", tmp);
			if (tmp = g_plugin.getByte(hContact, "Minutes", 1))
				fprintf(file, "Minutes=%d\r\n", tmp);
			if (tmp = g_plugin.getWord(hContact, "Timer", 0))
				fprintf(file, "Timer=%d\r\n", tmp);
			fprintf(file, "[/Non-IM Contact]\r\n");
		}
	}
	fclose(file);
	return 0;
}

int Openfile(char *outputFile, int saveOpen) //0=save, 1=open
{
	char filename[MAX_PATH] = "";
	char *filter = "All Files\0*.*\0";
	int r;
	char title[16];
	if (saveOpen)
		mir_strcpy(title, "Open file");
	else mir_strcpy(title, "Save to file");

	OPENFILENAMEA ofn = { sizeof(ofn) };
	ofn.lpstrFile = filename;
	ofn.lpstrFilter = filter;
	ofn.Flags = saveOpen ? OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_SHAREAWARE | OFN_PATHMUSTEXIST : OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST;
	ofn.lpstrTitle = title;
	ofn.nMaxFile = MAX_PATH;

	if (saveOpen)
		r = GetOpenFileNameA(&ofn);
	else
		r = GetSaveFileNameA(&ofn);
	if (!r)
		return 0;
	mir_strcpy(outputFile, filename);
	return 1;
}

void reloadFiles(HWND fileList)
{
	SendMessage(fileList, CB_RESETCONTENT, 0, 0);
	for (int i = 0;; i++) {
		char file[MAX_PATH], fn[6];
		mir_snprintf(fn, "fn%d", i);
		if (db_get_static(NULL, MODNAME, fn, file, _countof(file))) 
			return;

		/* add the file contents to the edit box */
		int index = SendMessageA(fileList, CB_ADDSTRING, 0, (LPARAM)file);
		SendMessage(fileList, CB_SETITEMDATA, index, (LPARAM)i);
		SendMessage(fileList, CB_SETCURSEL, index, 0);
		SetDlgItemTextA(GetParent(fileList), IDC_FN, _itoa(i, fn, 10));
	}
}

int savehtml(char* outFile)
{
	FILE* file = fopen(outFile, "w");
	if (!file) {
		return 0;
	}
	fprintf(file, "%s", szInfo);
	fclose(file);
	return 1;
}

void readFile(HWND hwnd)
{
	int lineNumber, fileLength = 0;
	char temp[MAX_STRING_LENGTH], szFileName[512], temp1[MAX_STRING_LENGTH];
	int fileNumber = SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_GETCURSEL, 0, 0);
	{
		char fn[10];
		mir_snprintf(fn, "fn%d", fileNumber);
		if (db_get_static(NULL, MODNAME, fn, szFileName, _countof(szFileName))) {
			msg(Translate("File couldn't be opened"), fn);
			return;
		}
	}

	if (!strncmp("http://", szFileName, mir_strlen("http://")) || !strncmp("https://", szFileName, mir_strlen("https://")))
		mir_snprintf(szFileName, "%s\\plugins\\fn%d.html", getMimDir(temp), fileNumber);

	FILE *filen = fopen(szFileName, "r");
	if (!filen) {
		MessageBox(nullptr, TranslateT("File couldn't be opened,2"), _A2W(modFullname), MB_OK);
		return;
	}
	lineNumber = 0;
	SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS, LB_RESETCONTENT, 0, 0);
	while (lineNumber < (MAXLINES) && (fgets(temp, MAX_STRING_LENGTH, filen))) {
		if (temp[0] == '\t') temp[0] = ' ';
		if (temp[mir_strlen(temp) - 1] == '\n' && temp[mir_strlen(temp) - 2] == '\r')
			temp[mir_strlen(temp) - 2] = '\0';
		else if (temp[mir_strlen(temp) - 1] == '\n')
			temp[mir_strlen(temp) - 1] = '\0';
		else temp[mir_strlen(temp)] = '\0';
		mir_snprintf(temp1, Translate("line(%-3d) = | %s"), lineNumber, temp);
		SendDlgItemMessageA(hwnd, IDC_FILE_CONTENTS, LB_ADDSTRING, 0, (LPARAM)temp1);
		lineNumber++;
		fileLength++;
		if ((unsigned int)SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS, LB_GETHORIZONTALEXTENT, 0, 0) <= (mir_strlen(temp1)*g_plugin.getByte("WidthMultiplier", 5)))
			SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS, LB_SETHORIZONTALEXTENT, (mir_strlen(temp1)*g_plugin.getByte("WidthMultiplier", 5)), 0);
	}
	fclose(filen);
}

#define WM_RELOADWINDOW (WM_USER+11)

INT_PTR CALLBACK DlgProcFiles(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char tmp[MAX_PATH], fn[MAX_PATH];

	switch (msg) {
	case WM_RELOADWINDOW:
		{
			char string[MAX_STRING_LENGTH];
			reloadFiles(GetDlgItem(hwnd, IDC_FILE_LIST));

			int i = SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_GETCURSEL, 0, 0);
			mir_snprintf(fn, "fn%d", i);
			SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS, LB_RESETCONTENT, 0, 0);
			if (!db_get_static(NULL, MODNAME, fn, string, _countof(string))) {
				if ((!strncmp("http://", string, mir_strlen("http://"))) || (!strncmp("https://", string, mir_strlen("https://")))) {
					SetDlgItemTextA(hwnd, IDC_URL, string);
					mir_snprintf(fn, "fn%d_timer", i);
					SetDlgItemTextA(hwnd, IDC_WWW_TIMER, _itoa(g_plugin.getWord(fn, 60), tmp, 10));
				}
				readFile(hwnd);
			}
		}
		break;

	case WM_INITDIALOG:
		SendMessage(hwnd, WM_RELOADWINDOW, 0, 0);
		TranslateDialogDefault(hwnd);
		return TRUE;
	
	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_ADD_URL:
			if (GetWindowTextLength(GetDlgItem(hwnd, IDC_URL))) {
				char text[512], url[512], szFileName[MAX_PATH], temp[512];
				GetDlgItemTextA(hwnd, IDC_URL, text, _countof(text));
				mir_strcpy(url, text);
				if (!InternetDownloadFile(text)) {
					for (int i = 0;; i++) {
						mir_snprintf(fn, "fn%d", i);
						if (db_get_static(NULL, MODNAME, fn, text, _countof(text))) {
							mir_snprintf(szFileName, "%s\\plugins\\%s.html", getMimDir(temp), fn);
							if (savehtml(szFileName)) {
								mir_snprintf(fn, "fn%d", i);
								g_plugin.setString(fn, url);
								int timer;
								if (!GetWindowTextLength(GetDlgItem(hwnd, IDC_WWW_TIMER)))
									timer = 60;
								else {
									GetDlgItemTextA(hwnd, IDC_WWW_TIMER, text, _countof(text));
									timer = atoi(text);
								}
								mir_snprintf(fn, "fn%d_timer", i);
								g_plugin.setWord(fn, (uint16_t)timer);
								SendMessage(hwnd, WM_RELOADWINDOW, 0, 0);
							}
							break;
						}
					}
				}
			}
			break;
	
		case IDC_WWW_TIMER:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_ADD_FILE:
			for (int i = 0;; i++) {
				char file[MAX_PATH];
				mir_snprintf(fn, "fn%d", i);
				if (db_get_static(NULL, MODNAME, fn, file, _countof(file))) {
					if (Openfile(file, 1)) {
						g_plugin.setString(fn, file);
						int index = SendDlgItemMessageA(hwnd, IDC_FILE_LIST, CB_ADDSTRING, 0, (LPARAM)file);
						SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_SETITEMDATA, index, (LPARAM)i);
						SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_SETCURSEL, index, 0);
						SetDlgItemTextA(hwnd, IDC_FN, _itoa(i, fn, 10));
						mir_snprintf(fn, "fn%d", index);
						readFile(hwnd);
					}
					break;
				}
			}
			break;
		
		case IDC_DEL_FILE:
			{
				int index = SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_GETCURSEL, 0, 0), i = (int)SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_GETITEMDATA, index, 0);
				int count = SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_GETCOUNT, 0, 0) - 1;
				if (index == count) {
					mir_snprintf(fn, "fn%d", index);
					g_plugin.delSetting(fn);
					SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_DELETESTRING, index, 0);
					SendMessage(hwnd, WM_RELOADWINDOW, 0, 0);
					if (!index) {
						SetDlgItemText(hwnd, IDC_FN, L"");
						SetDlgItemText(hwnd, IDC_FILE_CONTENTS, L"");
					}

				}
				else {
					mir_snprintf(fn, "fn%d", i);
					while (!db_get_static(NULL, MODNAME, fn, tmp, _countof(tmp))) {
						char fn1[4];
						mir_snprintf(fn1, "fn%d", i - 1);
						g_plugin.setString(fn1, tmp);
						mir_snprintf(fn, "fn%d", ++i);
					}
					mir_snprintf(fn, "fn%d", --i);
					g_plugin.delSetting(fn);
					SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_DELETESTRING, index, 0);
					SendMessage(hwnd, WM_RELOADWINDOW, 0, 0);
				}
			}
			break;

		case IDC_FILE_LIST:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int index = SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_GETCURSEL, 0, 0);
				SetDlgItemTextA(hwnd, IDC_FN, _itoa(index, fn, 10));
				mir_snprintf(fn, "fn%d", index);
				if (!db_get_static(NULL, MODNAME, fn, tmp, _countof(tmp))) {
					if (!strncmp("http://", tmp, mir_strlen("http://")) || !strncmp("https://", tmp, mir_strlen("https://"))) {
						SetDlgItemTextA(hwnd, IDC_URL, tmp);
						mir_snprintf(fn, "fn%d_timer", index);
						SetDlgItemTextA(hwnd, IDC_WWW_TIMER, _itoa(g_plugin.getWord(fn, 60), tmp, 10));
					}
					else {
						SetDlgItemText(hwnd, IDC_URL, L"");
						SetDlgItemText(hwnd, IDC_WWW_TIMER, L"");
					}
					readFile(hwnd);
				}
			}
			break;
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int i = SendDlgItemMessage(hwnd, IDC_FILE_LIST, CB_GETCURSEL, 0, 0);
				int timer;
				char string[1000];
				mir_snprintf(fn, "fn%d", i);
				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_WWW_TIMER))) {
					wchar_t text[5];
					GetDlgItemText(hwnd, IDC_WWW_TIMER, text, _countof(text));
					timer = _wtoi(text);
				}
				else timer = 60;

				if (!db_get_static(NULL, MODNAME, fn, string, _countof(string)))
					if (!strncmp("http://", string, mir_strlen("http://")) || !strncmp("https://", string, mir_strlen("https://"))) {
						mir_snprintf(fn, "fn%d_timer", i);
						g_plugin.setWord(fn, (uint16_t)timer);
					}

				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

char* getMimDir(char* file)
{
	GetModuleFileNameA(nullptr, file, MAX_PATH);

	char *p1 = strrchr(file, '\\');
	if (p1)
		*p1 = '\0';

	if (file[0] == '\\')
		file[mir_strlen(file) - 1] = '\0';

	return file;
}
