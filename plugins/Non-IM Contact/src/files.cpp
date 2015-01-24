#include "commonheaders.h"

INT_PTR exportContacts(WPARAM wParam,LPARAM lParam) 
{
	char fn[MAX_PATH];
	if (!Openfile(fn, 0))
		return 0;

	FILE* file;
	if (MessageBox(0, TranslateT("Do you want to overwrite the contents of the file?\r\n\r\nPressing No will append these contacts to the end of the file."),_T(modFullname), MB_YESNO) == IDYES)
		file = fopen(fn, "w");
	else
		file = fopen(fn, "a");
	if (!file)
		return 0;

	for (MCONTACT hContact = db_find_first(MODNAME); hContact; hContact = db_find_next(hContact, MODNAME)) {
		int tmp;
		char DBVar[1024];
		if (db_get_static(hContact, MODNAME, "Name", DBVar)) {
			fprintf(file, "\r\n[Non-IM Contact]\r\nName=%s\r\n", DBVar);
			if (db_get_static(hContact, MODNAME, "ProgramString", DBVar))
				fprintf(file, "ProgramString=%s\r\n", DBVar);
			if (db_get_static(hContact, MODNAME, "ProgramParamString", DBVar))
				fprintf(file, "ProgramParamString=%s\r\n", DBVar);
			if (db_get_static(hContact, MODNAME, "ToolTip", DBVar))
				fprintf(file, "ToolTip=%s</tooltip>\r\n", DBVar);
			if (db_get_static(hContact, "CList", "Group", DBVar))
				fprintf(file, "Group=%s\r\n", DBVar);
			if (tmp = db_get_w(hContact, MODNAME, "Icon", 40072))
				fprintf(file, "Icon=%d\r\n", tmp);
			if (tmp = db_get_b(hContact, MODNAME, "UseTimer", 0))
				fprintf(file, "UseTimer=%d\r\n", tmp);
			if (tmp = db_get_b(hContact, MODNAME, "Minutes", 1))
				fprintf(file, "Minutes=%d\r\n", tmp);
			if (tmp = db_get_w(hContact, MODNAME, "Timer", 0))
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
		strcpy(title, "Open file");
	else strcpy(title, "Save to file");

	OPENFILENAMEA ofn = { sizeof(ofn) };
	ofn.lpstrFile = filename;
	ofn.lpstrFilter = filter;
	ofn.Flags = saveOpen? OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_SHAREAWARE | OFN_PATHMUSTEXIST: OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST; 
	ofn.lpstrTitle = title;
	ofn.nMaxFile = MAX_PATH;

	if (saveOpen)
		r = GetOpenFileNameA(&ofn);
	else
		r = GetSaveFileNameA(&ofn);
	if (!r)
		return 0;
	mir_strcpy(outputFile,filename);
	return 1;
}


void reloadFiles(HWND fileList)
{
	int i, index;
	char file[MAX_PATH], fn[6];
	SendMessage(fileList,CB_RESETCONTENT, 0,0);
	for (i=0; ;i++)
	{
		mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
		if (db_get_static(NULL, MODNAME, fn, file)) {
			index = SendMessageA(fileList, CB_ADDSTRING,0, (LPARAM)file);
			SendMessage(fileList, CB_SETITEMDATA, index,  (LPARAM)i);
			SendMessage(fileList, CB_SETCURSEL, index,0);
			SetDlgItemTextA(GetParent(fileList), IDC_FN, _itoa(i, fn, 10));
			/* add the file contents to the edit box */
		}
		else break;
	}
}

int savehtml(char* outFile)
{
	FILE* file = fopen(outFile, "w");
	if (!file) 
	{
		fclose(file);		
		return 0;
	}
	fprintf(file, "%s", szInfo);
	fclose(file);
	return 1;
}

void readFile(HWND hwnd)
{
	int lineNumber, fileLength=0, width=0;
	char temp[MAX_STRING_LENGTH], szFileName[512], temp1[MAX_STRING_LENGTH], fn[8];
	int fileNumber = SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_GETCURSEL, 0,0);
	mir_snprintf(fn, SIZEOF(fn), "fn%d", fileNumber);
	if (!db_get_static(NULL, MODNAME, fn, szFileName)) {
		msg(Translate("File couldn't be opened"),fn);
		return;
	}

	if ( !strncmp("http://", szFileName, strlen("http://")) || !strncmp("https://", szFileName, strlen("https://")))
		mir_snprintf(szFileName, SIZEOF(szFileName), "%s\\plugins\\fn%d.html", getMimDir(temp), fileNumber);

	FILE *filen = fopen(szFileName,"r");
	if (!filen) {
		MessageBox(0, TranslateT("File couldn't be opened,2"), _T(modFullname),MB_OK);
		return;
	}
	lineNumber = 0;
	SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS,LB_RESETCONTENT, 0,0);
	while (lineNumber < (MAXLINES) && (fgets(temp, MAX_STRING_LENGTH, filen)))
	{
		if (temp[0] == '\t') temp[0] = ' ';
		if (temp[strlen(temp)-1]=='\n' && temp[strlen(temp)-2]=='\r') 
			temp[strlen(temp)-2]='\0';
		else if (temp[strlen(temp)-1]=='\n') 
			temp[strlen(temp)-1]='\0';
        else temp[strlen(temp)]='\0';
		mir_snprintf(temp1, SIZEOF(temp1), Translate("line(%-3d) = | %s"), lineNumber, temp);
		SendDlgItemMessageA(hwnd, IDC_FILE_CONTENTS,LB_ADDSTRING,0,(LPARAM)temp1);
		lineNumber++;
		fileLength++;
		if ((unsigned int)SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS,LB_GETHORIZONTALEXTENT,0,0) <= (strlen(temp1)*db_get_b(NULL, MODNAME, "WidthMultiplier", 5)))
			SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS,LB_SETHORIZONTALEXTENT,(strlen(temp1)*db_get_b(NULL, MODNAME, "WidthMultiplier", 5)),0);
	}
	fclose(filen);
}

#define WM_RELOADWINDOW (WM_USER+11)

INT_PTR CALLBACK DlgProcFiles(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_RELOADWINDOW:
		{
			char fn[MAX_PATH], string[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
			reloadFiles(GetDlgItem(hwnd, IDC_FILE_LIST));
			int i = SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_GETCURSEL, 0 ,0);
			mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
			SendDlgItemMessage(hwnd, IDC_FILE_CONTENTS,LB_RESETCONTENT, 0,0);
			if (db_get_static(NULL, MODNAME, fn, string) )
			{
				if ( (!strncmp("http://", string, strlen("http://"))) || (!strncmp("https://", string, strlen("https://"))) )
				{
					SetDlgItemTextA(hwnd,IDC_URL, string);
					mir_snprintf(fn,SIZEOF(fn),"fn%d_timer", i);
					SetDlgItemTextA(hwnd, IDC_WWW_TIMER, _itoa(db_get_w(NULL, MODNAME, fn, 60), tmp, 10));
				}
				readFile(hwnd);
			}
		}
		break;

	case WM_INITDIALOG:
		{
			SendMessage(hwnd, WM_RELOADWINDOW, 0,0);
			TranslateDialogDefault(hwnd);
		}
		return TRUE;
	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		switch(LOWORD(wParam)) {
		case IDC_ADD_URL:
			{
				if (GetWindowTextLength(GetDlgItem(hwnd,IDC_URL))) 
				{
					char text[512], url[512], fn[10] = "fn0", szFileName[MAX_PATH], temp[512];
					int i, timer;
					GetDlgItemTextA(hwnd,IDC_URL,text,SIZEOF(text));
					strcpy(url, text);
					if (!InternetDownloadFile(text))
					{
						for (i=0; ;i++)
						{
							mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
							if (!db_get_static(NULL, MODNAME, fn, text))
								break;
						}
						mir_snprintf(szFileName, SIZEOF(szFileName), "%s\\plugins\\%s.html", getMimDir(temp), fn);
						if (savehtml(szFileName))
						{
							mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
							db_set_s(NULL, MODNAME, fn, url);
							if (!GetWindowTextLength(GetDlgItem(hwnd,IDC_WWW_TIMER))) 
								timer = 60;
							else
							{
								GetDlgItemTextA(hwnd,IDC_WWW_TIMER,text,SIZEOF(text));
								timer = atoi(text);
							}
							mir_snprintf(fn, SIZEOF(fn), "fn%d_timer", i);
							db_set_w(NULL, MODNAME, fn, (WORD)timer);
							SendMessage(hwnd, WM_RELOADWINDOW, 0,0);
						}
					}
				}
			}
			break;
		case IDC_WWW_TIMER:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
			break;
		case IDC_ADD_FILE:
			{
				int i, index;
				char file[MAX_PATH], fn[6];
				for (i=0; ;i++)
				{
					mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
					if (!db_get_static(NULL, MODNAME, fn, file))
						break;
				}
				if (Openfile(file,1))
				{
					db_set_s(NULL, MODNAME, fn, file);
					index = SendDlgItemMessageA(hwnd, IDC_FILE_LIST, CB_ADDSTRING, 0, (LPARAM)file);
					SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_SETITEMDATA,index,(LPARAM)i);
					SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_SETCURSEL, index ,0);
					SetDlgItemTextA(hwnd, IDC_FN, _itoa(i, fn, 10));
					mir_snprintf(fn, SIZEOF(fn), "fn%d", index);
					readFile(hwnd);
				}

			}
			break;
		case IDC_DEL_FILE:
			{
				int index = SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_GETCURSEL, 0,0),i= (int)SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_GETITEMDATA, index,0);
				char fn[6], fn1[4], tmp[256];
				int count = SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_GETCOUNT, 0,0) -1;
				if (index == count)
				{
					mir_snprintf(fn, SIZEOF(fn), "fn%d", index);
					db_unset(NULL, MODNAME, fn);
					SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_DELETESTRING, index ,0);
					SendMessage(hwnd, WM_RELOADWINDOW, 0,0);
					if (!index) {
						SetDlgItemText(hwnd, IDC_FN,_T(""));
						SetDlgItemText(hwnd, IDC_FILE_CONTENTS,_T(""));
					}

				}
				else
				{
					mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
					while (db_get_static(NULL, MODNAME, fn,tmp))
					{
						mir_snprintf(fn1, SIZEOF(fn1), "fn%d", i-1);
						db_set_s(NULL, MODNAME, fn1 , tmp);
						mir_snprintf(fn, SIZEOF(fn), "fn%d", ++i);
					}
					mir_snprintf(fn, SIZEOF(fn), "fn%d", --i);
					db_unset(NULL, MODNAME, fn);
					SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_DELETESTRING, index ,0);
					SendMessage(hwnd, WM_RELOADWINDOW, 0,0);
				}

			}
			break;


		case IDC_FILE_LIST:
			if (HIWORD(wParam) == CBN_SELCHANGE )
			{
				int index = SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_GETCURSEL, 0,0);
				char fn[20], tmp[MAX_PATH];
				SetDlgItemTextA(hwnd, IDC_FN, _itoa(index, fn, 10));
				mir_snprintf(fn, SIZEOF(fn), "fn%d", index);
				if (db_get_static(NULL, MODNAME, fn, tmp) )
				{
					if (!strncmp("http://", tmp, strlen("http://")) || !strncmp("https://", tmp, strlen("https://")))
					{
						SetDlgItemTextA(hwnd,IDC_URL, tmp);
						mir_snprintf(fn, SIZEOF(fn), "fn%d_timer", index);
						SetDlgItemTextA(hwnd, IDC_WWW_TIMER, _itoa(db_get_w(NULL, MODNAME, fn, 60), tmp, 10));
					}
					else 
					{
						SetDlgItemText(hwnd,IDC_URL, _T(""));
						SetDlgItemText(hwnd, IDC_WWW_TIMER,_T(""));
					}
					readFile(hwnd);
				}
			}
			break;
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int i = SendDlgItemMessage(hwnd, IDC_FILE_LIST,CB_GETCURSEL, 0 ,0);
				int timer;
				char fn[MAX_PATH], string[1000];
				mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
				if (GetWindowTextLength(GetDlgItem(hwnd,IDC_WWW_TIMER))) {
					TCHAR text[5];
					GetDlgItemText(hwnd,IDC_WWW_TIMER,text,SIZEOF(text));
					timer = _ttoi(text);
				}
				else timer = 60;

				if (db_get_static(NULL, MODNAME, fn, string))
					if (!strncmp("http://", string, strlen("http://")) || !strncmp("https://", string, strlen("https://"))) {
						mir_snprintf(fn, SIZEOF(fn), "fn%d_timer", i);
						db_set_w(NULL, MODNAME, fn, (WORD)timer);
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
	GetModuleFileNameA(NULL, file, MAX_PATH);

	char *p1 = strrchr(file,'\\');
	if (p1)
		*p1 = '\0';

	if (file[0] == '\\')
		file[strlen(file)-1] = '\0';

	return file;
}
