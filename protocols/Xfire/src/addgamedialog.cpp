#include "stdafx.h"

#include "addgamedialog.h"
#include "variables.h"

extern HANDLE XFireWorkingFolder;
extern Xfire_gamelist xgamelist;

char inipath[MAX_PATH];
char* buffer = NULL;
BOOL dontClose = FALSE;

vector<lbInicache> Inicache;
Xfire_game* editgame = NULL;

HWND hwndTab, hPage;

void AddGameDialog(HWND hwndDlg, Xfire_game* game) {
	//übergebendes game, dem editgame zuordnen, damit wechselt es in den editmodus
	editgame = game;
	if (DialogBox(hinstance, MAKEINTRESOURCE(IDD_ADDGAMEMAIN), hwndDlg, DlgAddGameProcMain)) {
	}
	//wieder auf NULL setzen
	editgame = NULL;
}

static void FillGameList(LPVOID hwndDlg) {
	//liste.,suche und suchlabel unsichtbar machen
	ShowWindow(GetDlgItem((HWND)hwndDlg, IDC_GAMELIST), SW_HIDE);
	ShowWindow(GetDlgItem((HWND)hwndDlg, IDC_SEARCH), SW_HIDE);
	ShowWindow(GetDlgItem((HWND)hwndDlg, IDC_STC14), SW_HIDE);

	//handle von progressbar holen
	HWND hwndPB = GetDlgItem((HWND)hwndDlg, IDC_PROGRESS);

	//das schließen des dialogs verhindern
	dontClose = TRUE;

	if (Inicache.size() == 0)
	{
		//temp xfirebaseob für strlower
		Xfire_base tempxfire;
		//vector für doppelfilter
		vector<DWORD> dublBuffer;
		//Cache ist leer, Cache füllen

		strcpy(inipath, XFireGetFoldersPath("IniFile"));
		strcat_s(inipath, MAX_PATH, "xfire_games.ini");

		//ini soll in den RAM geladen werden, fürs schnellere ausparsen
		FILE* f = fopen(inipath, "rb");
		//wenn ini nicht aufrufbar, abbrechen
		if (f == NULL)
			return;
		//ans ende der datei springen
		fseek(f, 0, SEEK_END);
		//aktuelle position im filepoitner als größe zwischenspeichern
		int size = ftell(f);
		//buffer anelgen
		buffer = new char[size + 1];
		//wenn nicht genug ram, abbruch
		if (buffer == NULL)
			return;
		//buffer nullen
		memset(buffer, 0, size + 1);
		//zum anfang springen
		fseek(f, 0, SEEK_SET);
		//ini komplett in den ram lesen
		fread(buffer, size, 1, f);
		//datei schließen
		fclose(f);

		//parse einzelne sections aus

		//pointer vom buffer
		char*p = buffer;
		//zahlenbufer1 und pointer
		char zahlbuffer[10] = "";
		char*z = zahlbuffer;
		//zahlenbuffer2 uznd pointer
		char zahlbuffer2[10] = "";
		char*z2 = zahlbuffer2;

		//progressbar auf bytegröße anpassen
		SendMessage(hwndPB, PBM_SETRANGE32, 0, size);

		//solange bis wir bei 0 angekommen sind
		if (*p != 0)
			p++;
		while (*p != 0)
		{
			if (*p == '['&&*(p - 1) == '\n')
			{
				z = zahlbuffer;
				z2 = zahlbuffer2;
				p++;
				while (*p >= '0'&&*p <= '9'&&p != 0)
				{
					*z = *p;
					p++;
					z++;
				}
				*z = 0;
				if (*p == '_') {
					p++;
					while (*p >= '0'&&*p <= '9'&&p != 0)
					{
						*z2 = *p;
						p++;
						z2++;
					}
					*z2 = 0;
				}

				//erste zahl gefunden
				if (z != zahlbuffer)
				{
					int gameid = atoi(zahlbuffer);
					//prüfe ob das game schon in der gameliste ist
					if (!xgamelist.Gameinlist(gameid))
					{
						char*name = p;
						lbInicache listentry;
						char gameidtemp[10] = "";
						char ret[512];
						DWORD uniqid = 0;
						BYTE accLaunch = 0;
						BYTE accDetect = 0;

						//bis zum nächsten isgleich springen
						while (*name != '=')
							name++;
						name++;

						//jetzt den umbruch suchen
						p = name;
						while (*p != '\n'&&*p != '\r') p++;
						*p = 0;
						p++;

						//spielnamen in den listentry einfügen
						strcpy_s(listentry.name, 255, name);

						if (z2 != zahlbuffer2)
						{
							listentry.gameid = MAKELONG(gameid, atoi(zahlbuffer2));
							mir_snprintf(gameidtemp, 10, "%d_%d", gameid, atoi(zahlbuffer2));
						}
						else
						{
							listentry.gameid = gameid;
							mir_snprintf(gameidtemp, 10, "%d", gameid);
						}


						if (xfire_GetPrivateProfileString(gameidtemp, "LauncherExe", "", ret, 512, inipath)) {
							//lower launchstring
							tempxfire.strtolower(ret);
							//einzelnen ziechen zusammenrechnen
							if (z2 != zahlbuffer2)
							{
								//wenn pfad, dann exe vorher rausziehen
								if (strrchr(ret, '\\'))
									accLaunch = tempxfire.accStringByte(strrchr(ret, '\\'));
								else
									accLaunch = tempxfire.accStringByte(ret);
							}
							//Steam.exe als launcher?
							if ((ret[0] == 's') &&
								ret[1] == 't'&&
								ret[2] == 'e'&&
								ret[3] == 'a'&&
								ret[4] == 'm'&&
								ret[5] == '.'&&
								ret[6] == 'e'&&
								ret[7] == 'x'&&
								ret[8] == 'e')
							{
								strcat_s(listentry.name, 255, " (Steam)");
							}
						}

						if (z2 != zahlbuffer2) {
							if (xfire_GetPrivateProfileString(gameidtemp, "DetectExe", "", ret, 512, inipath)) {
								//lower launchstring
								tempxfire.strtolower(ret);
								//einzelnen ziechen zusammenrechnen
								//wenn pfad, dann exe vorher rausziehen
								if (strrchr(ret, '\\'))
									accDetect = tempxfire.accStringByte(strrchr(ret, '\\'));
								else
									accDetect = tempxfire.accStringByte(ret);
							}
						}

						//standardmäßig TRUE
						BOOL addtolist = TRUE;

						//schon ein eintrag vorhanden?
						if (z2 != zahlbuffer2) {
							//uniq id zusammen bauen aus spielid sowie zusammengerechneten launchstring und detectstring
							uniqid = MAKELONG(gameid, MAKEWORD(accLaunch, accDetect));
							for (uint i = 0; i < dublBuffer.size(); i++) {
								if (dublBuffer.at(i) == uniqid)
								{
									addtolist = FALSE;
									break;
								}
							}
						}

						//nur eintragen, wenn kein doppelter gefunden wurde
						if (addtolist) {
							//eintrag einfügen
							int idx = SendDlgItemMessageA((HWND)hwndDlg, IDC_GAMELIST, LB_ADDSTRING, 0, (LPARAM)listentry.name);
							SendDlgItemMessage((HWND)hwndDlg, IDC_GAMELIST, LB_SETITEMDATA, idx, listentry.gameid);
							//eintrag in den buffer für die dublikateprüfung
							dublBuffer.push_back(uniqid);
							//und eintrag in den cache
							Inicache.push_back(listentry);
							SendMessage(hwndPB, PBM_SETPOS, (WPARAM)(p - buffer), 0);
						}
					}
				}
			}
			else
				p++;
		}
	}
	else
	{
		//liste mit dem cache aufbauen
		for (uint i = 0; i < Inicache.size(); i++) {
			//spielid in der liste spielliste?
			if (!xgamelist.Gameinlist(LOWORD(Inicache.at(i).gameid)))
			{
				//eintrag in die listeeinfügen
				int idx = SendDlgItemMessageA((HWND)hwndDlg, IDC_GAMELIST, LB_ADDSTRING, 0, (LPARAM)Inicache.at(i).name);
				//gameid zuweisen
				SendDlgItemMessage((HWND)hwndDlg, IDC_GAMELIST, LB_SETITEMDATA, idx, Inicache.at(i).gameid);
			}
		}
	}

	//progressbar unsichtbar
	ShowWindow(hwndPB, SW_HIDE);
	//liste sichtbar machen
	ShowWindow(GetDlgItem((HWND)hwndDlg, IDC_GAMELIST), SW_SHOW);
	//suche und suchlabel sichtbar machen
	ShowWindow(GetDlgItem((HWND)hwndDlg, IDC_SEARCH), SW_SHOW);
	ShowWindow(GetDlgItem((HWND)hwndDlg, IDC_STC14), SW_SHOW);

	dontClose = FALSE;
}

BOOL OpenFileDialog(HWND hwndDlg, OPENFILENAMEA*ofn, char*exe) {
	//pointer zum exenamen
	char* exename = NULL;
	//buffer vom pfad
	static char szFile[260] = ""; //static damit noch nach dem aufruf lesbar bleibt
	//buffer vom filter
	char szFilter[260] = "";
	//backslash suchen
	exename = strrchr(exe, '\\') + 1;
	//kein backslash dann normal ret als exenamen verwenden
	if ((int)exename == 1) exename = exe;
	//filterstring aufbauen
	mir_snprintf(szFilter, SIZEOF(szFilter), "%s|%s|%s|*.*|", exename, exename, Translate("All Files"));
	//umbruch in 0 wandeln
	unsigned int sizeFilter = strlen(szFilter);
	for (unsigned int i = 0; i < sizeFilter; i++)
		if (szFilter[i] == '|') szFilter[i] = 0;
	//openfiledia vorbereiten
	ZeroMemory(ofn, sizeof(OPENFILENAMEA));
	ofn->lStructSize = sizeof(OPENFILENAMEA);
	ofn->hwndOwner = hwndDlg;
	ofn->lpstrFile = szFile;
	ofn->nMaxFile = SIZEOF(szFile);
	ofn->lpstrFilter = szFilter;
	ofn->nFilterIndex = 1;
	ofn->lpstrFileTitle = exe;
	ofn->nMaxFileTitle = 0;
	ofn->lpstrInitialDir = NULL;
	ofn->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return GetOpenFileNameA(ofn);
}

INT_PTR CALLBACK DlgAddGameProc(HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg) {
	case WM_INITDIALOG:
	{
		//übersetzen lassen
		TranslateDialogDefault(hwndDlg);


		//per thread liste füllen
		mir_forkthread(FillGameList, hwndDlg);
	}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SEARCH && HIWORD(wParam) == EN_CHANGE)
		{
			char temp[256];
			//eingabe bei der suche auslesen
			GetDlgItemTextA(hwndDlg, IDC_SEARCH, temp, SIZEOF(temp));
			//eingabe in der liste suchen
			int idx = SendDlgItemMessageA(hwndDlg, IDC_GAMELIST, LB_FINDSTRING, 0, (LPARAM)temp);
			//gefunden?
			if (idx != LB_ERR)
			{
				//als aktiv setzen
				SendDlgItemMessage(hwndDlg, IDC_GAMELIST, LB_SETCURSEL, idx, 0);
			}
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			//nicht schließen, wenn noch der thread läuft
			if (dontClose) {
				MessageBox(hwndDlg, TranslateT("Please wait, game.ini will be currently parsed..."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
				return FALSE;
			}
			return SendMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
		}
		else if (LOWORD(wParam) == IDC_CUSTOM)
		{
			int idx = SendDlgItemMessage(hwndDlg, IDC_GAMELIST, LB_GETCURSEL, 0, 0);
			//es wurde was ausgewählt?
			if (idx == LB_ERR) {
				MessageBox(hwndDlg, TranslateT("Please choose one game in the list!"), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
			}
			else
			{
				char ret[512] = "";
				char gameidtemp[10] = "";

				int gameids = SendDlgItemMessage((HWND)hwndDlg, IDC_GAMELIST, LB_GETITEMDATA, idx, 0);
				//gameid splitten
				int gameid1 = LOWORD(gameids);
				int gameid2 = HIWORD(gameids);

				if (gameid2 != 0)
					mir_snprintf(gameidtemp, 10, "%d_%d", gameid1, gameid2);
				else
					mir_snprintf(gameidtemp, 10, "%d", gameid1);

				//spielnamen holen
				if (xfire_GetPrivateProfileString(gameidtemp, "LongName", "", ret, 512, inipath)) {
					mir_snprintf(gameidtemp, 10, "%d", gameid1);

					//einige felder vorbelegen
					SetDlgItemTextA(hPage, IDC_ADD_NAME, ret);
					SetDlgItemText(hPage, IDC_ADD_DETECTEXE, _T(""));
					SetDlgItemText(hPage, IDC_ADD_LAUNCHEREXE, _T(""));
					SetDlgItemTextA(hPage, IDC_ADD_ID, gameidtemp);
					SetDlgItemText(hPage, IDC_ADD_STATUSMSG, _T(""));
					SetDlgItemText(hPage, IDC_ADD_CUSTOMPARAMS, _T(""));
					SetDlgItemTextA(hPage, IDC_ADD_SENDID, gameidtemp);

					//auf customeintrag edit tab wechseln
					TabCtrl_SetCurSel(hwndTab, 1);
					ShowWindow(hwndDlg, SW_HIDE);
					ShowWindow(hPage, SW_SHOW);
				}
			}
		}
		else if (LOWORD(wParam) == IDOK)
		{
			int idx = SendDlgItemMessage(hwndDlg, IDC_GAMELIST, LB_GETCURSEL, 0, 0);
			//es wurde was ausgewählt?
			if (idx == LB_ERR) {
				MessageBox(hwndDlg, TranslateT("Please choose one game in the list!"), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
			}
			else
			{
				//datei öffnen dialog
				OPENFILENAMEA ofn;
				//listdata auslesen, wo die gameid gespeihcert ist
				int gameids = SendDlgItemMessage((HWND)hwndDlg, IDC_GAMELIST, LB_GETITEMDATA, idx, 0);
				//gameid splitten
				int gameid1 = LOWORD(gameids);
				int gameid2 = HIWORD(gameids);
				char gameidtemp[10] = "";
				char ret[512];

				if (gameid2 != 0)
					mir_snprintf(gameidtemp, 10, "%d_%d", gameid1, gameid2);
				else
					mir_snprintf(gameidtemp, 10, "%d", gameid1);

				//neuen gameeintrag anlegen
				Xfire_game* newgame = new Xfire_game();
				//gameid und sendid setzen
				newgame->id = gameid1;
				newgame->send_gameid = gameid1;
				//es handelt sich um einen customeintrag, man staune xD
				newgame->custom = 1;

				//launcherexe abfragen
				if (xfire_GetPrivateProfileString(gameidtemp, "LauncherExe", "", ret, 512, inipath)) {
					//datei vom user öffnen lassen
					if (OpenFileDialog(hwndDlg, &ofn, ret))
					{
						//lowercase pfad
						newgame->strtolower(ofn.lpstrFile);
						//pfad dem spiel zuordnen
						newgame->setString(ofn.lpstrFile, &newgame->launchparams);
					}
					else
					{
						//speicher freigeben
						delete newgame;
						return FALSE;
					}
				}

				//detectexe abfragen
				if (xfire_GetPrivateProfileString(gameidtemp, "DetectExe", "", ret, 512, inipath)) {
					//datei vom user öffnen lassen
					if (OpenFileDialog(hwndDlg, &ofn, ret))
					{
						//lowercase pfad
						newgame->strtolower(ofn.lpstrFile);
						//pfad dem spiel zuordnen
						newgame->setString(ofn.lpstrFile, &newgame->path);
					}
					else
					{
						//speicher freigeben
						delete newgame;
						return FALSE;
					}
				} //MatchExe abfragen
				else if (xfire_GetPrivateProfileString(gameidtemp, "MatchExe", "", ret, 512, inipath)) {
					//datei vom user öffnen lassen
					if (OpenFileDialog(hwndDlg, &ofn, ret))
					{
						//lowercase pfad
						newgame->strtolower(ofn.lpstrFile);
						//pfad dem spiel zuordnen
						newgame->setString(ofn.lpstrFile, &newgame->path);
					}
					else
					{
						//speicher freigeben
						delete newgame;
						return FALSE;
					}
				}


				//prüfe ob schon ein launchstring festgelegt wurde, wenn nicht die detectexe nehmen
				if (!newgame->launchparams&&newgame->path) {
					newgame->setString(newgame->path, &newgame->launchparams);
				}

				//prüfe ob schon ein detectexe festgelegt wurde, wenn nicht die launchstring nehmen
				if (newgame->launchparams&&!newgame->path) {
					newgame->setString(newgame->launchparams, &newgame->path);
				}

				//LauncherUrl wird der launcherstring überschrieben
				if (xfire_GetPrivateProfileString(gameidtemp, "LauncherUrl", "", ret, 512, inipath)) {
					newgame->setString(ret, &newgame->launchparams);
				}
				else if (xfire_GetPrivateProfileString(gameidtemp, "Launch", "", ret, 512, inipath)) {
					str_replace(ret, "%UA_LAUNCHER_EXE_PATH%", ""); //erstmal unwichtige sachen entfernen
					//str_replace(ret,"%UA_LAUNCHER_EXTRA_ARGS%",""); // - auch entfernen	
					str_replace(ret, "%UA_LAUNCHER_LOGIN_ARGS%", ""); // - auch entfernen	

					//ein leerzeichen anfügen
					newgame->appendString(" ", &newgame->launchparams);
					//nun commandline des launchstringes dranhängen
					newgame->appendString(ret, &newgame->launchparams);
				}

				//restliche wichtige felder einfügen
				if (xfire_GetPrivateProfileString(gameidtemp, "LauncherPasswordArgs", "", ret, 512, inipath))
					newgame->setString(ret, &newgame->pwparams);
				if (xfire_GetPrivateProfileString(gameidtemp, "LauncherNetworkArgs", "", ret, 512, inipath))
					newgame->setString(ret, &newgame->networkparams);
				if (xfire_GetPrivateProfileString(gameidtemp, "CommandLineMustContain[0]", "", ret, 512, inipath))
					newgame->setString(ret, &newgame->mustcontain);
				if (xfire_GetPrivateProfileString(gameidtemp, "XUSERSendId", "", ret, 512, inipath))
					newgame->send_gameid = atoi(ret);
				if (xfire_GetPrivateProfileString(gameidtemp, "XUSERSetStatusMsg", "", ret, 512, inipath))
					newgame->setstatusmsg = atoi(ret);

				//namen setzen und icon laden
				newgame->setNameandIcon();
				//spiel in die gameliste einfügen
				xgamelist.Addgame(newgame);
				//derzeitige gameliste in die datenbank eintragen
				xgamelist.writeDatabase();
				//startmenu leeren
				xgamelist.clearStartmenu();
				//startmenu neuerzeugen
				xgamelist.createStartmenu();

				return SendMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
			}
		}
		break;
	case WM_CLOSE:
		//nicht schließen, wenn noch der thread läuft
		if (dontClose) {
			MessageBox(hwndDlg, TranslateT("Please wait, game.ini will be currently parsed..."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		//hauptfenster schließen
		SendMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
		break;
	}
	return FALSE;
}


INT_PTR CALLBACK DlgAddGameProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		//ein spiel wurde zum editieren geöffnet, felder alle vorbelegen
		if (editgame) {
			//add augf übernehmen umstellen
			SetDlgItemText(hwndDlg, IDOK, TranslateT("Apply"));

			//namen vorbelegen
			if (editgame->customgamename)
				SetDlgItemTextA(hwndDlg, IDC_ADD_NAME, editgame->customgamename);
			else if (editgame->name)
				SetDlgItemTextA(hwndDlg, IDC_ADD_NAME, editgame->name);

			//gameid setzen und feld schreibschützen
			char gameid[10] = "";
			_itoa_s(editgame->id, gameid, 10, 10);
			SetDlgItemTextA(hwndDlg, IDC_ADD_ID, gameid);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_ID), FALSE);

			//sendgameid setzen, bei -1 leer lassen
			if (editgame->send_gameid != -1)
			{
				_itoa_s(editgame->send_gameid, gameid, 10, 10);
				SetDlgItemTextA(hwndDlg, IDC_ADD_SENDID, gameid);
			}

			//launcherstring
			if (editgame->launchparams) {
				SetDlgItemTextA(hwndDlg, IDC_ADD_LAUNCHEREXE, editgame->launchparams);
			}

			//detectstring
			if (editgame->path) {
				SetDlgItemTextA(hwndDlg, IDC_ADD_DETECTEXE, editgame->path);
			}

			//statusmsg
			if (editgame->statusmsg) {
				SetDlgItemTextA(hwndDlg, IDC_ADD_STATUSMSG, editgame->statusmsg);
			}

			//mustcontain parameter
			if (editgame->mustcontain) {
				SetDlgItemTextA(hwndDlg, IDC_ADD_CUSTOMPARAMS, editgame->mustcontain);
			}
		}
		//dialog übersetzen
		TranslateDialogDefault(hwndDlg);

		return TRUE;

	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDC_SENDIDHELP)
		{
			MessageBox(hwndDlg, TranslateT("If you add a mod of an Xfire supported game, then you can specify what game ID will be sent to Xfire. So if you add a Half-Life mod, you can set the Half-Life game ID and if you start the game, your Xfire buddies will see the Half-Life game icon next to your name and the game time will be tracked."), TranslateT("XFire Options"), MB_OK | MB_ICONASTERISK);
		}
		else if (LOWORD(wParam) == IDC_GAMEIDHELP)
		{
			MessageBox(hwndDlg, TranslateT("Every game in Xfire needs an ID. Use a number above the last used ID to avoid problems with used IDs. Every number above 10000 should be save. This ID will not be sent to Xfire, when you start a game."), TranslateT("XFire Options"), MB_OK | MB_ICONASTERISK);
		}
		else if (LOWORD(wParam) == IDC_ADD_BROWSEDETECT)
		{
			OPENFILENAMEA ofn;
			if (OpenFileDialog(hwndDlg, &ofn, "*.exe"))
			{
				SetDlgItemTextA(hwndDlg, IDC_ADD_DETECTEXE, ofn.lpstrFile);
			}
		}
		else if (LOWORD(wParam) == IDC_ADD_BROWSELAUNCHER)
		{
			OPENFILENAMEA ofn;
			if (OpenFileDialog(hwndDlg, &ofn, "*.exe"))
			{
				SetDlgItemTextA(hwndDlg, IDC_ADD_LAUNCHEREXE, ofn.lpstrFile);
			}
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			//nicht schließen, wenn noch der thread läuft
			if (dontClose) {
				MessageBox(hwndDlg, TranslateT("Please wait, game.ini will be currently parsed..."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
				return FALSE;
			}
			return SendMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
		}
		else if (LOWORD(wParam) == IDOK)
		{
			char temp[256];

			//fillgames sucht noch
			if (dontClose) {
				MessageBox(hwndDlg, TranslateT("Please wait, game.ini will be currently parsed..."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
				return FALSE;
			}

			//neuen gameeintrag anlegen
			Xfire_game* newgame = NULL;

			if (editgame)
				newgame = editgame;
			else
				newgame = new Xfire_game();

			//Spielname
			GetDlgItemTextA(hwndDlg, IDC_ADD_NAME, temp, SIZEOF(temp));
			if (!strlen(temp))
			{
				if (!editgame) delete newgame;
				return MessageBox(hwndDlg, TranslateT("Please enter a game name."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
			}
			else
			{
				//spielname zuordnen
				newgame->setString(temp, &newgame->customgamename);
				//spielnamen fürs menü
				newgame->setString(temp, &newgame->name);
			}
			//spielid nur setzen/prüfen, wenn kein editgame
			if (!editgame) {
				GetDlgItemTextA(hwndDlg, IDC_ADD_ID, temp, SIZEOF(temp));
				if (!strlen(temp))
				{
					if (!editgame) delete newgame;
					return MessageBox(hwndDlg, TranslateT("Please enter a game ID."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
				}
				else
				{
					int gameid = atoi(temp);
					//negative gameid blocken
					if (gameid < 1)
					{
						if (!editgame) delete newgame;
						return MessageBox(hwndDlg, TranslateT("Please enter a game ID above 1."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
					}
					//gameid auf uniq prüfen
					else if (xgamelist.getGamebyGameid(gameid))
					{
						if (!editgame) delete newgame;
						return MessageBox(hwndDlg, TranslateT("This game ID is already in use."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
					}
					//gameid zuordnen
					newgame->id = gameid;
					//standardmäßig wird bei einem customeintrag keine id versendet
					newgame->send_gameid = -1;
				}
			}
			//zu sendene spielid
			GetDlgItemTextA(hwndDlg, IDC_ADD_SENDID, temp, SIZEOF(temp));
			if (strlen(temp))
			{
				//standardmäßig wird bei einem customeintrag keine id versendet
				int sendid = atoi(temp);
				if (sendid > 0)
					newgame->send_gameid = sendid;
			}

			//launcher exe
			GetDlgItemTextA(hwndDlg, IDC_ADD_LAUNCHEREXE, temp, SIZEOF(temp));
			if (strlen(temp))
			{
				//lowercase pfad
				newgame->strtolower(temp);
				//detect exe
				newgame->setString(temp, &newgame->launchparams);
			}
			//detectexe
			GetDlgItemTextA(hwndDlg, IDC_ADD_DETECTEXE, temp, SIZEOF(temp));
			if (!strlen(temp))
			{
				if (!editgame) delete newgame;
				return MessageBox(hwndDlg, TranslateT("Please select a game exe. Note: If you don't select a launcher exe, the game exe will be used in the game start menu."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
			}
			else
			{
				//lowercase pfad
				newgame->strtolower(temp);
				//detect exe
				newgame->setString(temp, &newgame->path);
				//wenn kein launcher exe/pfad angeben wurde, dann den gamepath nehmen
				if (!newgame->launchparams)
					newgame->setString(temp, &newgame->launchparams);

			}
			//mustcontain parameter
			GetDlgItemTextA(hwndDlg, IDC_ADD_CUSTOMPARAMS, temp, SIZEOF(temp));
			if (strlen(temp))
			{
				newgame->setString(temp, &newgame->mustcontain);
			}
			//statusmsg speichern
			GetDlgItemTextA(hwndDlg, IDC_ADD_STATUSMSG, temp, SIZEOF(temp));
			if (strlen(temp))
			{
				newgame->setString(temp, &newgame->statusmsg);
				newgame->setstatusmsg = 1;
			}
			//custom eintrag aktivieren
			newgame->custom = 1;
			//spiel in die gameliste einfügen, aber nur im nicht editmodus
			if (!editgame)
				xgamelist.Addgame(newgame);
			//derzeitige gameliste in die datenbank eintragen
			xgamelist.writeDatabase();
			//startmenu leeren
			xgamelist.clearStartmenu();
			//startmenu neuerzeugen
			xgamelist.createStartmenu();

			return SendMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
		}
		break;
	}
	}
	return FALSE;
}


INT_PTR CALLBACK DlgAddGameProcMain(HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		TCITEMA tci = { 0 };
		int iTotal;
		RECT rcClient;

		//icon des dialogs setzen
		SendMessage(hwndDlg, WM_SETICON, (WPARAM)false, (LPARAM)LoadIcon(hinstance, MAKEINTRESOURCE(IDI_TM)));

		hwndTab = GetDlgItem(hwndDlg, IDC_OPTIONSTAB);
		TabCtrl_DeleteAllItems(hwndTab);
		GetClientRect(GetParent(hwndTab), &rcClient);

		hPage = CreateDialog(hinstance, MAKEINTRESOURCE(IDD_ADDGAME), hwndDlg, DlgAddGameProc);

		//bei editgame keine spiellisteauswahl
		if (!editgame)
		{
			iTotal = TabCtrl_GetItemCount(hwndTab);
			tci.mask = TCIF_PARAM | TCIF_TEXT;
			tci.lParam = (LPARAM)hPage;
			tci.pszText = Translate("Supported Games");
			SendMessageA(hwndTab, TCM_INSERTITEMA, iTotal, (WPARAM)&tci);
			MoveWindow(hPage, 3, 24, rcClient.right - 10, rcClient.bottom - 28, 1);
			iTotal++;
		}
		else
			ShowWindow(hPage, FALSE);

		hPage = CreateDialog(hinstance, MAKEINTRESOURCE(IDD_ADDGAME2), hwndDlg, DlgAddGameProc2);
		iTotal = TabCtrl_GetItemCount(hwndTab);
		tci.mask = TCIF_PARAM | TCIF_TEXT;
		tci.lParam = (LPARAM)hPage;
		tci.pszText = Translate("Custom game");
		SendMessageA(hwndTab, TCM_INSERTITEMA, iTotal, (WPARAM)&tci);
		MoveWindow(hPage, 3, 24, rcClient.right - 10, rcClient.bottom - 28, 1);
		iTotal++;

		//bei editgame 2. registerkarte aktiv schalten
		if (!editgame) {
			ShowWindow(hPage, FALSE);
			TabCtrl_SetCurSel(hwndTab, 0);
		}

		return TRUE;
	}
	case WM_CLOSE:
		//nicht schließen, wenn noch der thread läuft
		if (dontClose) return FALSE;
		//buffer leeren
		if (buffer)
		{
			delete[] buffer;
			buffer = NULL;
		}
		EndDialog(hwndDlg, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case 0:
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_APPLY:
			{
				TCITEM tci;
				int i, count;

				tci.mask = TCIF_PARAM;
				count = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_OPTIONSTAB));
				for (i = 0; i < count; i++)
				{
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), i, &tci);
					SendMessage((HWND)tci.lParam, WM_NOTIFY, 0, lParam);
				}
				break;
			}
			}
			break;

		case IDC_OPTIONSTAB:
		{
			HWND hTabCtrl = GetDlgItem(hwndDlg, IDC_OPTIONSTAB);

			switch (((LPNMHDR)lParam)->code)
			{
			case TCN_SELCHANGING:
			{
				TCITEM tci;

				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hTabCtrl, TabCtrl_GetCurSel(hTabCtrl), &tci);
				ShowWindow((HWND)tci.lParam, SW_HIDE);
			}
				break;

			case TCN_SELCHANGE:
			{
				TCITEM tci;

				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hTabCtrl, TabCtrl_GetCurSel(hTabCtrl), &tci);
				ShowWindow((HWND)tci.lParam, SW_SHOW);
			}
				break;
			}
			break;
		}
		}
		break;
	}
	return FALSE;
}