//***********************************************************
//	Copyright © 2003-2008 Alexander S. Kiselev, Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#include "boltun.h"

int hLangpack;

TalkBot* bot = NULL;


#define MAX_WARN_TEXT    1024
#define MAX_MIND_FILE    1024

HINSTANCE hInst;
BOOL blInit = FALSE;
UINT pTimer = 0;
TCHAR *path;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {488C5C84-56DA-434F-96F1-B18900DEF760}
	{0x488c5c84, 0x56da, 0x434f, {0x96, 0xf1, 0xb1, 0x89, 0x0, 0xde, 0xf7, 0x60}}
};

static HGENMENU hMenuItemAutoChat, hMenuItemNotToChat, hMenuItemStartChatting;

#define MIND_DIALOG_FILTER _T("%s (*.mindw)\1*.mindw\1%s (*.*)\1*.*\1")

#ifdef DEBUG_LOAD_TIME
#include <intrin.h>
#endif

void UpdateEngine()
{
	if (bot)
	{
		bot->SetSilent(Config.EngineStaySilent);
		bot->SetLowercase(Config.EngineMakeLowerCase);
		bot->SetUnderstandAlways(Config.EngineUnderstandAlways);
	}
}

TCHAR* GetFullName(const TCHAR* filename)
{
	size_t flen = _tcslen(filename);
	TCHAR* fullname = const_cast<TCHAR*>(filename);
	if (!_tcschr(filename, _T(':')))
	{
		size_t plen = _tcslen(path);
		fullname = new TCHAR[plen+flen+1];
		fullname[0] = NULL;
		_tcscat(fullname, path);
		_tcscat(fullname, filename);
	}
	return fullname;
}

static bool LoadMind(const TCHAR* filename, int &line)
{
	TCHAR* fullname = GetFullName(filename);
	HCURSOR newCur = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
	HCURSOR oldCur = SetCursor(newCur);
#ifdef DEBUG_LOAD_TIME
	unsigned __int64 t = __rdtsc();
#endif
	Mind* mind = new Mind();
	line = -1;
	try
	{
		mind->Load(fullname);
	}
	catch(Mind::CorruptedMind c)
	{
		line = c.line;
		delete mind;
		if (fullname != filename)
			delete[] fullname;
		SetCursor(oldCur);
		return false;
	}
	catch(...)
	{
		delete mind;
		if (fullname != filename)
			delete[] fullname;
		SetCursor(oldCur);
		return false;
	}
	if (fullname != filename)
		delete[] fullname;

#ifdef DEBUG_LOAD_TIME
	t = __rdtsc() - t;
	char dest[101];
	mir_snprintf(dest, SIZEOF(dest), "%I64d ticks\n", t / 3200000);
	MessageBoxA(NULL, dest, NULL, 0);
	//exit(0);
#endif
	SetCursor(oldCur);
	HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_SMILES), _T("SMILES"));
	if (!hRes)
	{
		delete mind;
		return false;
	}
	DWORD size = SizeofResource(hInst, hRes);
	if (!size)
	{
		delete mind;
		return false;
	}
	HGLOBAL hGlob = LoadResource(hInst, hRes);
	if (!hGlob)
	{
		delete mind;
		return false;
	}
	void *data = LockResource(hGlob);
	if (!data)
	{
		FreeResource(hGlob);
		delete mind;
		return false;
	}
	bool res = true;
	try
	{
		mind->LoadSmiles(data, size);
	}
	catch(...)
	{
		res = false;
	}
	UnlockResource(data);
	FreeResource(hGlob);
	if (!res)
	{
		delete mind;
		return false;
	}
	delete bot;
	bot = new TalkBot(*mind);
	delete mind;
	UpdateEngine();
	return true;
}

/*static bool SaveMind(const TCHAR* filename)
{
	if (!bot)
		return false;
	bot->GetMind().Save(filename);
	return true;
}*/

static bool BoltunAutoChat(MCONTACT hContact)
{
	if (db_get_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_NOT_TO_CHAT
		, FALSE) == TRUE)
		return false;

	if (Config.TalkWithEverybody)
		return true;

	if (Config.TalkEveryoneWhileAway)
	{
		int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
		if (status == ID_STATUS_AWAY       ||
			status == ID_STATUS_DND        ||
			status == ID_STATUS_NA         ||
			status == ID_STATUS_OCCUPIED   ||
			status == ID_STATUS_ONTHEPHONE ||
			status == ID_STATUS_OUTTOLUNCH)
			return true;
	}

	if ((db_get_b(hContact,"CList","NotOnList",0) == 1) &&
		Config.TalkWithNotInList)
		return true;

	if (db_get_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_AUTO_CHAT,
		FALSE) == TRUE)
		return true;

	return false;
}

static int MessageEventAdded(WPARAM hContact, LPARAM lParam)
{
	if (!BoltunAutoChat(hContact))
		return 0;

	DBEVENTINFO dbei;
	memset(&dbei, 0, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = 0;

	HANDLE hDbEvent = (HANDLE)lParam;
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbei.cbBlob == -1)
		return 0;

	dbei.pBlob = (PBYTE)malloc(dbei.cbBlob);
	if (dbei.pBlob == NULL)
		return 0;

	db_event_get(hDbEvent, &dbei);
	if (dbei.flags & DBEF_SENT || dbei.flags & DBEF_READ || dbei.eventType != EVENTTYPE_MESSAGE)
		return 0;
	DBEVENTGETTEXT egt;
	egt.codepage = CP_ACP;
	egt.datatype = DBVT_TCHAR;
	egt.dbei = &dbei;
	TCHAR* s = (TCHAR*)(void*)CallService(MS_DB_EVENT_GETTEXT, 0, (LPARAM)&egt);
	free(dbei.pBlob);
	if (Config.MarkAsRead)
		db_event_markRead(hContact, hDbEvent);

	AnswerToContact(hContact, s);
	mir_free(s);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

void UpdateEverybodyCheckboxes(HWND hwndDlg)
{
	bool Enable = BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_EVERYBODY) == BST_CHECKED;
	HWND wnd;
	wnd = GetDlgItem(hwndDlg, IDC_NOTINLIST);
	EnableWindow(wnd, Enable);
	wnd = GetDlgItem(hwndDlg, IDC_AUTOAWAY);
	EnableWindow(wnd, Enable);
}

static INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bTranslated = FALSE;
	static bool loading = true;
	switch (uMsg)
	{
		case WM_INITDIALOG:
			loading = true;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_EVERYBODY, Config.TalkWithEverybody ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTINLIST, Config.TalkWithNotInList ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOAWAY,  Config.TalkEveryoneWhileAway  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_WARN,      Config.TalkWarnContacts ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MARKREAD,  Config.MarkAsRead  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PAUSEDEPENDS,  Config.PauseDepends ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PAUSERANDOM,  Config.PauseRandom ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_WAITTIME, EM_SETLIMITTEXT, 3, 0);
			SetDlgItemInt(hwndDlg, IDC_WAITTIME, Config.AnswerPauseTime, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_THINKTIME, EM_SETLIMITTEXT, 3, 0);
			SetDlgItemInt(hwndDlg, IDC_THINKTIME, Config.AnswerThinkTime, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_WARNTXT, EM_SETLIMITTEXT, MAX_WARN_TEXT, 0);
			SetDlgItemText(hwndDlg, IDC_WARNTXT, Config.WarnText);
			UpdateEverybodyCheckboxes(hwndDlg);
			loading = false;
			return TRUE;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_EVERYBODY && HIWORD(wParam) == BN_CLICKED)
				UpdateEverybodyCheckboxes(hwndDlg);
			if (!loading)
			{
				bool notify = true;
				switch (LOWORD(wParam))
				{
					case IDC_WARNTXT:
					case IDC_WAITTIME:
					case IDC_THINKTIME:
						if (HIWORD(wParam) != EN_CHANGE)
							notify = false;
						break;
				}
				if (notify)
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
				case PSN_APPLY:
				case PSN_KILLACTIVE:
				{
	                Config.TalkWithEverybody = IsDlgButtonChecked(hwndDlg, IDC_EVERYBODY) == BST_CHECKED ? TRUE : FALSE;
					Config.TalkWithNotInList = IsDlgButtonChecked(hwndDlg, IDC_NOTINLIST) == BST_CHECKED ? TRUE : FALSE;
					Config.TalkEveryoneWhileAway  = IsDlgButtonChecked(hwndDlg, IDC_AUTOAWAY)  == BST_CHECKED ? TRUE : FALSE;
					Config.TalkWarnContacts = IsDlgButtonChecked(hwndDlg, IDC_WARN)      == BST_CHECKED ? TRUE : FALSE;
					Config.MarkAsRead  = IsDlgButtonChecked(hwndDlg, IDC_MARKREAD)  == BST_CHECKED ? TRUE : FALSE;
					Config.PauseDepends = IsDlgButtonChecked(hwndDlg, IDC_PAUSEDEPENDS)  == BST_CHECKED ? TRUE : FALSE;
					Config.PauseRandom = IsDlgButtonChecked(hwndDlg, IDC_PAUSERANDOM)  == BST_CHECKED ? TRUE : FALSE;
					Config.AnswerPauseTime = GetDlgItemInt(hwndDlg, IDC_WAITTIME, &bTranslated, FALSE);
					if (!bTranslated)
						Config.AnswerPauseTime = 2;
					Config.AnswerThinkTime = GetDlgItemInt(hwndDlg, IDC_THINKTIME, &bTranslated, FALSE);
					if (!bTranslated)
						Config.AnswerThinkTime = 4;
					TCHAR c[MAX_WARN_TEXT];
					bTranslated = GetDlgItemText(hwndDlg, IDC_WARNTXT, c, SIZEOF(c));
					if(bTranslated)
						Config.WarnText = c;
					else
						Config.WarnText = TranslateTS(DEFAULT_WARN_TEXT);
				}
	            return TRUE;
			}
			break;
		}
		break;
	}
	return 0;
}

void UpdateUnderstandAlwaysCheckbox(HWND hwndDlg)
{
	bool Enable = BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENGINE_SILENT) == BST_CHECKED;
	HWND wnd;
	wnd = GetDlgItem(hwndDlg, IDC_ENGINE_UNDERSTAND_ALWAYS);
	EnableWindow(wnd, Enable);
}

static INT_PTR CALLBACK EngineDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD param;
	BOOL bTranslated = FALSE;
	static bool loading = true;
	static int changeCount = 0;
	switch (uMsg)
	{
		case WM_INITDIALOG:
			loading = true;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_ENGINE_SILENT,  Config.EngineStaySilent ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ENGINE_LOWERCASE,  Config.EngineMakeLowerCase ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ENGINE_UNDERSTAND_ALWAYS,  Config.EngineUnderstandAlways ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_MINDFILE, Config.MindFileName);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSAVE), blInit);
			UpdateUnderstandAlwaysCheckbox(hwndDlg);
			loading = false;
			return TRUE;
		case WM_COMMAND:
			param = LOWORD(wParam);
			if (param == IDC_ENGINE_SILENT && HIWORD(wParam) == BN_CLICKED)
				UpdateUnderstandAlwaysCheckbox(hwndDlg);
			OPENFILENAME ofn;
			switch(param)
			{
				case IDC_BTNPATH:
					{
						const size_t fileNameSize = 5000;
						TCHAR *filename = new TCHAR[fileNameSize];
						TCHAR *fullname = GetFullName(Config.MindFileName);
						_tcscpy(filename, fullname);
						if (fullname != Config.MindFileName)
							delete[] fullname;

						memset(&ofn, 0, sizeof(ofn));
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = GetParent(hwndDlg);

						TCHAR* mind = TranslateTS(MIND_FILE_DESC);
						TCHAR* anyfile = TranslateTS(ALL_FILES_DESC);
						size_t l = _tcslen(MIND_DIALOG_FILTER)
							+ _tcslen(mind) + _tcslen(anyfile);
						TCHAR *filt = new TCHAR[l];
						mir_sntprintf(filt, l, MIND_DIALOG_FILTER, mind, anyfile);
						for (size_t i = 0; i < l; i++)
							if (filt[i] == '\1')
								filt[i] = '\0';
						ofn.lpstrFilter = filt;

						ofn.lpstrFile = filename;
						ofn.nMaxFile = fileNameSize;
						ofn.Flags = OFN_FILEMUSTEXIST;
						ofn.lpstrInitialDir = path;
						if (!GetOpenFileName(&ofn))
						{
							delete[] filename;
							delete[] filt;
							break;
						}
						delete[] filt;
						TCHAR* origf = filename;
						TCHAR* f = filename;
						TCHAR* p = path;
						while (*p && *f)
						{
							TCHAR p1 = (TCHAR)CharLower((TCHAR*)(long)*p++);
							TCHAR f1 = (TCHAR)CharLower((TCHAR*)(long)*f++);
							if (p1 != f1)
								break;
						}
						if (!*p)
							filename = f;
						Config.MindFileName = filename;
						SetDlgItemText(hwndDlg, IDC_MINDFILE, filename);
						delete[] origf;
					}
				case IDC_BTNRELOAD:
				{
					const TCHAR *c = Config.MindFileName;
					int line;
					bTranslated = blInit = LoadMind(c, line);
					if (!bTranslated)
					{
						TCHAR message[5000];
						mir_sntprintf(message, SIZEOF(message), TranslateTS(FAILED_TO_LOAD_BASE), line, c);
						MessageBox(NULL, message, TranslateTS(BOLTUN_ERROR), MB_ICONERROR|MB_TASKMODAL|MB_OK);
					}
					break;
				}
				default:
					if (!loading)
					{
						if (param == IDC_MINDFILE/* && HIWORD(wParam) != EN_CHANGE*/)
							break;
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
			}
			break;
		case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
				case PSN_APPLY:
				case PSN_KILLACTIVE:
				{
					Config.EngineStaySilent = IsDlgButtonChecked(hwndDlg, IDC_ENGINE_SILENT) == BST_CHECKED ? TRUE : FALSE;
					Config.EngineMakeLowerCase = IsDlgButtonChecked(hwndDlg, IDC_ENGINE_LOWERCASE) == BST_CHECKED ? TRUE : FALSE;
					Config.EngineUnderstandAlways = IsDlgButtonChecked(hwndDlg, IDC_ENGINE_UNDERSTAND_ALWAYS) == BST_CHECKED ? TRUE : FALSE;
					UpdateEngine();
					TCHAR c[MAX_MIND_FILE];
					bTranslated = GetDlgItemText(hwndDlg, IDC_MINDFILE, c, SIZEOF(c));
					if (bTranslated)
						Config.MindFileName = c;
					else
						Config.MindFileName = DEFAULT_MIND_FILE;
				}
	            return TRUE;
			}
			break;
		}
		break;
	}
	return 0;
}

static int MessageOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 910000000;
	odp.hInstance = hInst;
	odp.pszGroup = BOLTUN_GROUP;
	odp.pszTitle = BOLTUN_NAME;
	odp.pfnDlgProc = MainDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN);
	odp.pszTab = TAB_GENERAL;
	Options_AddPage(wParam, &odp);
	
	odp.pfnDlgProc = EngineDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_ENGINE);
	odp.pszTab = TAB_ENGINE;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int ContactClick(WPARAM hContact, LPARAM lParam, BOOL clickNotToChat)
{
	BOOL boltunautochat = db_get_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_AUTO_CHAT, FALSE);
	BOOL boltunnottochat = db_get_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_NOT_TO_CHAT, FALSE);

	if (clickNotToChat)
	{
		boltunnottochat = !boltunnottochat;
		if(boltunnottochat)
		{
			boltunautochat = FALSE;
		}
	}
	else
	{
		boltunautochat = !boltunautochat;
		if(boltunautochat)
		{
			boltunnottochat = FALSE;
		}
		else
		{
			db_set_b(hContact, BOLTUN_KEY, DB_CONTACT_WARNED, FALSE);
		}
	}

	db_set_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_AUTO_CHAT, (BYTE)boltunautochat);
	db_set_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_NOT_TO_CHAT, (BYTE)boltunnottochat);

	return 0;
}

static INT_PTR ContactClickAutoChat(WPARAM hContact, LPARAM lParam)
{
	return ContactClick(hContact, lParam, 0);
}

static INT_PTR ContactClickNotToChat(WPARAM hContact, LPARAM lParam)
{
	return ContactClick(hContact, lParam, 1);
}

static INT_PTR ContactClickStartChatting(WPARAM hContact, LPARAM lParam)
{
	StartChatting(hContact);
	return 0;
}

static int MessagePrebuild(WPARAM hContact, LPARAM lParam)
{
	CLISTMENUITEM clmi = { sizeof(clmi) };

	if (!blInit || (db_get_b(hContact,"CList","NotOnList",0) == 1)) {
		clmi.flags = CMIM_FLAGS | CMIF_GRAYED;

		Menu_ModifyItem(hMenuItemAutoChat, &clmi);
		Menu_ModifyItem(hMenuItemNotToChat, &clmi);
	}
	else {
		BOOL boltunautochat = db_get_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_AUTO_CHAT, FALSE);
		BOOL boltunnottochat = db_get_b(hContact, BOLTUN_KEY, DB_CONTACT_BOLTUN_NOT_TO_CHAT, FALSE);

		clmi.flags  = CMIM_FLAGS | CMIM_ICON | (boltunautochat ? CMIF_CHECKED : 0);
		clmi.hIcon = LoadIcon( GetModuleHandle(NULL), MAKEINTRESOURCE((boltunautochat ? IDI_TICK : IDI_NOTICK)));
		Menu_ModifyItem(hMenuItemAutoChat, &clmi);

		clmi.flags  = CMIM_FLAGS | CMIM_ICON | (boltunnottochat ? CMIF_CHECKED : 0);
		clmi.hIcon = LoadIcon( GetModuleHandle(NULL), MAKEINTRESOURCE((boltunnottochat ? IDI_TICK : IDI_NOTICK)));
		Menu_ModifyItem(hMenuItemNotToChat, &clmi);
	}
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	path = new TCHAR[MAX_PATH];
	int len = GetModuleFileName(hInst, path, MAX_PATH);
	if (len > MAX_PATH)
	{
		delete[] path;
		TCHAR *path = new TCHAR[len];
		int len2 = GetModuleFileName(hInst, path, len);
		if (len2 != len)
		{
			delete[] path;
			return false;
		}
	}
	*(_tcsrchr(path, _T('\\'))+1) = _T('\0');

	/*initialize miranda hooks and services on options dialog*/
	HookEvent(ME_OPT_INITIALISE, MessageOptInit);
	/*initialize miranda hooks and services*/
	HookEvent(ME_DB_EVENT_ADDED, MessageEventAdded);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MessagePrebuild);

	CreateServiceFunction(SERV_CONTACT_AUTO_CHAT, ContactClickAutoChat);
	CreateServiceFunction(SERV_CONTACT_NOT_TO_CHAT, ContactClickNotToChat);
	CreateServiceFunction(SERV_CONTACT_START_CHATTING, ContactClickStartChatting);
	{
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position            = -50010002; //TODO: check the warning
		mi.pszName             = BOLTUN_AUTO_CHAT;
		mi.pszService          = SERV_CONTACT_AUTO_CHAT;
		hMenuItemAutoChat      = Menu_AddContactMenuItem(&mi);

		mi.position            = -50010001; //TODO: check the warning
		mi.pszName             = BOLTUN_NOT_TO_CHAT;
		mi.pszService          = SERV_CONTACT_NOT_TO_CHAT;
		hMenuItemNotToChat     = Menu_AddContactMenuItem(&mi);

		mi.flags               = CMIF_NOTOFFLINE;
		mi.position            = -50010000; //TODO: check the warning
		mi.hIcon               = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RECVMSG));
		mi.pszName             = BOLTUN_START_CHATTING;
		mi.pszService          = SERV_CONTACT_START_CHATTING;
		hMenuItemStartChatting = Menu_AddContactMenuItem(&mi);
	}

	int line;
	blInit = LoadMind(Config.MindFileName, line);
	if (!blInit)
	{
		TCHAR path[2000];
		mir_sntprintf(path, SIZEOF(path), TranslateTS(FAILED_TO_LOAD_BASE), line, (const TCHAR*)Config.MindFileName);
		MessageBox(NULL, path, TranslateTS(BOLTUN_ERROR), MB_ICONERROR|MB_TASKMODAL|MB_OK);
	}
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (pTimer)
		KillTimer(NULL, pTimer);
	if(blInit)
	{
#if 0 //No need to save, we don't have studying algorithm
		if(Config.MindFileName && !SaveMind(Config.MindFileName))
		{
//This causes errors with development core when calling MessageBox.
//It seems that it's now a Boltun problem.
//So in case of saving error we will remain silent
#if 0
			TCHAR path[MAX_PATH];
			mir_sntprintf(path, SIZEOF(path), TranslateTS(FAILED_TO_SAVE_BASE), (const TCHAR*)Config.MindFileName);
			TCHAR* err = TranslateTS(BOLTUN_ERROR);
			MessageBox(NULL, path, err, MB_ICONERROR|MB_TASKMODAL|MB_OK);*/
#endif
		}
#endif
		delete bot;
	}
	delete[] path;
	return 0;
}
