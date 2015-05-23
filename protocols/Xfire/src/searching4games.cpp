/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network.
 *
 *  Copyright (C) 2010 by
 *          dufte <dufte@justmail.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 *
 *  Miranda ICQ: the free icq client for MS Windows
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

#include "stdafx.h"
#include "baseProtocol.h"
#include "Xfire_gamelist.h"
#include "variables.h"
#include <string>
using std::string;

extern int foundgames;
extern HANDLE	 XFireAvatarFolder;
extern HANDLE	 XFireWorkingFolder;
extern INT_PTR StartGame(WPARAM wParam, LPARAM lParam, LPARAM fParam);
extern Xfire_gamelist xgamelist;

//versucht die * in den pfaden der ini mit dem korrekten eintrag zu ersetzen
BOOL CheckPath(char*ppath, char*pathwildcard = NULL)
{
	char* pos = 0;
	char* pos2 = 0;

	pos = strchr(ppath, '*');
	if (pos)
	{
		if (pathwildcard)
		{
			strcpy_s(pathwildcard, XFIRE_MAX_STATIC_STRING_LEN, ppath);
		}

		pos++;
		*pos = 0;
		pos++;

		//versuch die exe zu finden
		WIN32_FIND_DATAA wfd;
		HANDLE fHandle = FindFirstFileA(ppath, &wfd);  // . skippen
		if (fHandle == INVALID_HANDLE_VALUE)
			return FALSE;
		if(FindNextFileA(fHandle, &wfd)) { // .. auch skippen
			while (FindNextFileA(fHandle, &wfd)) // erstes file
			{
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // nur verzeichnisse sind interessant
				{
					char temp[XFIRE_MAX_STATIC_STRING_LEN];

					strncpy(temp, ppath,XFIRE_MAX_STATIC_STRING_LEN-1);
					*(temp + mir_strlen(temp) - 1) = 0;
					strncat(temp, wfd.cFileName, SIZEOF(temp) - mir_strlen(temp));
					strncat(temp, "\\", SIZEOF(temp) - mir_strlen(temp));
					strncat(temp, pos, SIZEOF(temp) - mir_strlen(temp));

					if (GetFileAttributesA(temp) != 0xFFFFFFFF) { //exe vorhanden???? unt hint?
						//gefundenes in path kopieren
						FindClose(fHandle);
						mir_strcpy(ppath, temp);
						return TRUE;
					}
				}
			}
		}
		FindClose(fHandle);
	}
	else
	{
		if (GetFileAttributesA(ppath) != 0xFFFFFFFF) { //exe vorhanden???? unt hint?
			//gefundenes in path kopieren
			return TRUE;
		}
	}

	return FALSE;
}

INT_PTR CALLBACK DlgSearchDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		return TRUE;
	}
	}

	return FALSE;
}

void ShowSearchDialog(LPVOID lparam)
{
	HWND* hwnd = (HWND*)lparam;
	HWND myhwnd = CreateDialog(hinstance, MAKEINTRESOURCE(IDD_SEARCHING), NULL, DlgSearchDialogProc);

	if (myhwnd == NULL)
		return;

	*hwnd = myhwnd;

	ShowWindow(myhwnd, SW_SHOW);

	//nachrichten schleife
	MSG msg;
	while (GetMessage(&msg, myhwnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Scan4Games(LPVOID lparam)
{
	int i = 2;
	unsigned int i2 = 1;
	BOOL split = FALSE;
	int notfound = 0;
	char* pos = 0;
	char* pos2 = 0;
	char *cutforlaunch = 0;
	char temp[10] = "";
	char inipath[XFIRE_MAX_STATIC_STRING_LEN] = "";
	char ret[XFIRE_MAX_STATIC_STRING_LEN] = "";
	char gamename[XFIRE_MAX_STATIC_STRING_LEN] = "";
	char ret2[XFIRE_MAX_STATIC_STRING_LEN] = "";
	char gamelist[XFIRE_MAX_STATIC_STRING_LEN] = "";
	BOOL multiexe = FALSE;
	BOOL somethingfound = TRUE;
	int i3 = 0;
	DWORD gpps = 1;
	DWORD last_gpps = 0;
	static BOOL searching = FALSE;

	//ich suche schon,also raushier
	if (searching)
	{
		MSGBOX("Game searching is already running!");
		return;
	}
	if (xgamelist.Ingame())
	{
		MSGBOX("A game is currently running, please close the game.");
		return;
	}

	//gamelist blocken, damit nur 1 thread es ausführt/nutzt
	xgamelist.Block(TRUE);

	searching = TRUE;

	//säubert die gameliste für eine neuauslesung
	xgamelist.clearGamelist();

	//prüfe ob schon gescannt wurde, dann das aus der db nehmen, beschleunigt den start
	foundgames = db_get_w(NULL, protocolname, "foundgames", -1);

	//um bei einer neuen version einen rescan zuforcen, bei bestimmten wert found auf 0 resetten
	if (db_get_w(NULL, protocolname, "scanver", 0) != XFIRE_SCAN_VAL)
		foundgames = -1;

	BOOL loadgamesfromdb = FALSE;
	if (db_get_b(NULL, protocolname, "scanalways", 0) == 0)
		loadgamesfromdb = TRUE;
	else
	{
		if (db_get_b(NULL, protocolname, "scanalways", 0) == 2)
		{
			time_t zeit;
			struct tm *t;
			time(&zeit);
			t = localtime(&zeit);

			if (t != NULL)
			{
				if (t->tm_yday != db_get_w(NULL, protocolname, "scanalways_t", 0))
				{
					db_set_w(NULL, protocolname, "scanalways_t", t->tm_yday);
				}
				else
					loadgamesfromdb = TRUE;
			}
		}
	}

	//spiele von db laden
	if (loadgamesfromdb)
		if (foundgames > 0)
		{
		//spieliste einlesen
		xgamelist.readGamelist(foundgames);
		//menüpunkte anlegen
		xgamelist.createStartmenu();

		//gamelist unblocken
		xgamelist.Block(FALSE);

		searching = FALSE;
		return;
		}
		else if (foundgames == 0)
		{
			searching = FALSE;
			//dummymenü punkt entfernen
			//CallService(MS_CLIST_REMOVEMAINMENUITEM, ( WPARAM )dummymenu, 0 );	

			//gamelist unblocken
			xgamelist.Block(FALSE);

			return;
		}
		else
			foundgames = 0;
	else
		foundgames = 0;

	HWND hwnd = NULL;

	//suche dialog anzeigen
	if (!db_get_b(NULL, protocolname, "dontdissstatus", 0))
	{
		mir_forkthread(ShowSearchDialog, &hwnd);
	}

	mir_strcpy(inipath, XFireGetFoldersPath("IniFile"));
	mir_strcat(inipath, "xfire_games.ini");

	//erstmal db säubern
	xgamelist.clearDatabase();

	//maximal 200 notfounds, um die nicht belegten id's zu überspringen
	while (notfound < 200)
	{
		//2 gameids?
		if (split)
			mir_snprintf(temp, SIZEOF(temp), "%i_%i", i, i2);
		else
			mir_snprintf(temp, SIZEOF(temp), "%i", i);

		//MessageBox(0,temp,temp,0);

		//letztes ergeniss sichern
		last_gpps = gpps;

		//las ma mal suchen...
		gpps = xfire_GetPrivateProfileString(temp, "LongName", "", gamename, 255, inipath);

		char entrytype[100] = "";
		if (gpps != NULL) xfire_GetPrivateProfileString(temp, "SoftwareType", "", entrytype, 100, inipath);

		//kein eintrag? voicechat? musiapplicationen? schon in der gameliste?
		if (gpps != NULL && mir_strcmp(entrytype, "Music") != 0 && mir_strcmp(entrytype, "VoiceChat") != 0 && !xgamelist.Gameinlist(i)) //was gefunden und noch nicht eintragen?
		{
			BOOL MatchExe = FALSE;
			//MatchExe Games in der automatischen Suche skippen
			if (xfire_GetPrivateProfileString(temp, "MatchExe", "", ret, 255, inipath))
				MatchExe = TRUE;

			//Registryschlüssel auslesen und pfad auf exe prüfen
			xfire_GetPrivateProfileString(temp, "LauncherDirKey", "", ret, 255, inipath);
			mir_strcpy(ret2, ret);

			//ersten part des registry schlüssel raustrennen
			pos = strchr(ret2, '\\');
			if (!MatchExe && pos != 0)
			{
				HKEY hkey, hsubk;

				*pos = 0; //string trennen
				pos++;

				pos2 = strrchr(pos, '\\'); //key trennen
				if (pos2 != 0)
				{
					*pos2 = 0;
					pos2++;

					//HKEY festlegen
					switch (*(ret2 + 6))
					{
					case 'L':
						hkey = HKEY_CLASSES_ROOT;
						break;
					case 'U':
						hkey = HKEY_CURRENT_USER;
						break;
					case 'O':
						hkey = HKEY_LOCAL_MACHINE;
						break;
					default:
						hkey = NULL;
					}
				}
				else
					hkey = NULL;

				if (hkey) //nur wenn der key erkannt wurde
				{
					if (RegOpenKeyA(hkey, pos, &hsubk) == ERROR_SUCCESS) //key versuchen zu "öffnen"
					{
						char path[XFIRE_MAX_STATIC_STRING_LEN] = "";
						char path_r[XFIRE_MAX_STATIC_STRING_LEN] = "";
						DWORD size = sizeof(path);

						//key lesen
						if (RegQueryValueExA(hsubk, pos2, NULL, NULL, (LPBYTE)path, &size) == ERROR_SUCCESS)
						{
							//zusätzlichen pfad anhängen
							if (xfire_GetPrivateProfileString(temp, "LauncherDirAppend", "", ret2, 255, inipath))
							{
								if (*(path + mir_strlen(path) - 1) == '\\'&&*(ret2) == '\\')
									mir_strcat(path, (ret2 + 1));
								else
									mir_strcat(path, ret2);
							}

							if (xfire_GetPrivateProfileString(temp, "LauncherDirTruncAt", "", ret2, 255, inipath))
							{
								//mögliches erstes anführungszeichen entfernen
								if (*(path) == '"')
								{
									pos2 = path;
									pos2++;

									mir_strcpy(path, pos2);
								}

								//mögliche weitere anführungszeichen entfernen
								pos = strchr(path, '"');
								if (pos != 0)
									*pos = 0;

								if (*(path + mir_strlen(path) - 1) != '\\')
									*(path + mir_strlen(path) - mir_strlen(ret2)) = 0;
							}

							if (*(path + mir_strlen(path) - 1) != '\\')
								mir_strcat(path, "\\");


							//dateiname auslesen
							if (xfire_GetPrivateProfileString(temp, "InstallHint", "", ret2, 255, inipath))
							{
								char pathtemp[XFIRE_MAX_STATIC_STRING_LEN];
								mir_strcpy(pathtemp, path);
								mir_strcat(pathtemp, ret2);

								if (CheckPath(pathtemp))
								{
									if (xfire_GetPrivateProfileString(temp, "DetectExe", "", ret, 255, inipath))
									{
										cutforlaunch = path + mir_strlen(path);
										mir_strcpy(pathtemp, path);

										//wenn backslash bei detectexe, dann diesen skippen (eveonline bug)
										if (ret[0] == '\\')
										{
											mir_strcat(pathtemp, (char*)&ret[1]);
										}
										else
										{
											mir_strcat(pathtemp, ret);
										}

										if (CheckPath(pathtemp))
										{
											mir_strcpy(path, pathtemp);
										}
										else
										{
											*(path) = 0;
										}
									}
									else if (xfire_GetPrivateProfileString(temp, "LauncherExe", "", ret2, 255, inipath))
									{
										cutforlaunch = path + mir_strlen(path);
										mir_strcat(path, ret2);
									}
								}
								else
								{
									*(path) = 0;
								}
							}
							else if (xfire_GetPrivateProfileString(temp, "DetectExe[0]", "", ret2, 255, inipath))
							{
								cutforlaunch = path + mir_strlen(path);
								mir_strcat(path, ret2);
								multiexe = TRUE;
								if (!CheckPath(path, path_r))
								{
									*(path) = 0;
								}
							}
							else if (xfire_GetPrivateProfileString(temp, "DetectExe", "", ret2, 255, inipath))
							{
								cutforlaunch = path + mir_strlen(path);

								//wenn backslash bei detectexe, dann diesen skippen (eveonline bug)
								if (ret2[0] == '\\')
								{
									mir_strcat(path, (char*)&ret2[1]);
								}
								else
								{
									mir_strcat(path, ret2);
								}

								if (!CheckPath(path, path_r))
								{
									*(path) = 0;
								}
							}
							else if (xfire_GetPrivateProfileString(temp, "LauncherExe", "", ret2, 255, inipath))
							{
								cutforlaunch = path + mir_strlen(path);
								mir_strcat(path, ret2);
							}


							//prüfe ob existent, dann ist das spiel installiert
							if (path[0] != 0 && GetFileAttributesA(path) != 0xFFFFFFFF)
							{
								Xfire_game* newgame = new Xfire_game();
								newgame->id = i;

								//8.3 pfade umwandeln
								//GetLongPathNameA(path,path,sizeof(path));

								//lowercase pfad
								for (unsigned int ii = 0; ii < mir_strlen(path); ii++)
									path[ii] = tolower(path[ii]);

								if (path_r[0] == 0)
									newgame->setString(path, &newgame->path);
								else
								{
									//lowercase wildcard pfad
									for (unsigned int ii = 0; ii < mir_strlen(path_r); ii++)
										path_r[ii] = tolower(path_r[ii]);
									newgame->setString(path_r, &newgame->path);
								}

								//spiel mit mehreren exefiles
								if (multiexe)
								{
									multiexe = FALSE;
									for (int i = 1; i < 9; i++)
									{
										mir_snprintf(ret, SIZEOF(ret), "DetectExe[%d]", i);
										if (xfire_GetPrivateProfileString(temp, ret, "", ret2, 512, inipath))
										{
											char* pos = strrchr(path, '\\');
											if (pos != 0)
											{
												pos++;
												*pos = 0;
											}
											mir_strcat(path, ret2);
											if (!CheckPath(path))
											{
												*(path) = 0;
											}
											else
											{
												for (unsigned int i2 = 0; i2 < mir_strlen(path); i2++)
													path[i2] = tolower(path[i2]);

												char* mpathtemp = new char[mir_strlen(path) + 1];
												mir_strcpy(mpathtemp, path);
												newgame->mpath.push_back(mpathtemp);
											}
										}
									}
								}

								//für launcherstring anpassen
								char* pos = strrchr(path, '\\');
								if (pos != 0)
								{
									pos++;
									*pos = 0;
								}
								xfire_GetPrivateProfileString(temp, "LauncherExe", "", ret2, 255, inipath); // anfügen
								if (cutforlaunch != 0) *cutforlaunch = 0;

								//pfad aufbereiten
								char launchpath[XFIRE_MAX_STATIC_STRING_LEN] = "";
								mir_strcpy(launchpath, path);
								//letzten backslash entfernen
								if (launchpath[mir_strlen(launchpath) - 1] == '\\') launchpath[mir_strlen(launchpath) - 1] = 0;

								mir_strcat(path, ret2);

								newgame->setString(path, &newgame->launchparams);
								newgame->appendString(" ", &newgame->launchparams);

								xfire_GetPrivateProfileString(temp, "Launch", "", ret2, 512, inipath);
								str_replace(ret2, "%UA_LAUNCHER_EXE_DIR%", launchpath);
								str_replace(ret2, "%UA_LAUNCHER_EXE_PATH%", ""); //erstmal unwichtige sachen entfernen
								//str_replace(ret2,"%UA_LAUNCHER_EXTRA_ARGS%",""); // - auch entfernen	
								str_replace(ret2, "%UA_LAUNCHER_LOGIN_ARGS%", ""); // - auch entfernen	

								newgame->appendString(ret2, &newgame->launchparams);

								/*if (xfire_GetPrivateProfileString(temp, "LauncherLoginArgs", "", ret2, 512, inipath))
								{
								str_replace(xf[foundgames].launchparams,"%UA_LAUNCHER_LOGIN_ARGS%",ret2); // - auch entfernen
								}
								else*/
								//	str_replace(xf[foundgames].launchparams,"%UA_LAUNCHER_LOGIN_ARGS%",""); // - auch entfernen	

								if (xfire_GetPrivateProfileString(temp, "LauncherPasswordArgs", "", ret2, 512, inipath))
									newgame->setString(ret2, &newgame->pwparams);

								if (xfire_GetPrivateProfileString(temp, "LauncherNetworkArgs", "", ret2, 512, inipath))
									newgame->setString(ret2, &newgame->networkparams);

								if (xfire_GetPrivateProfileString(temp, "CommandLineMustContain[0]", "", ret2, 512, inipath))
									newgame->setString(ret2, &newgame->mustcontain);

								if (xfire_GetPrivateProfileString(temp, "XUSERSendId", "", ret2, 512, inipath))
									newgame->send_gameid = atoi(ret2);

								if (xfire_GetPrivateProfileString(temp, "XUSERSetStatusMsg", "", ret2, 512, inipath))
									newgame->setstatusmsg = atoi(ret2);

								//launcherurl?
								if (xfire_GetPrivateProfileString(temp, "LauncherUrl", "", ret2, 512, inipath))
									newgame->setString(ret2, &newgame->launchparams);

								//soll alle string, welche nicht in der commandline eines spiels sein soll in einen string pakcen semikolon getrennt
								mir_snprintf(ret, SIZEOF(ret), "CommandLineMustNotContain[0]");
								int i = 0;

								while (xfire_GetPrivateProfileString(temp, ret, "", ret2, 512, inipath))
								{
									if (!newgame->notcontain) newgame->setString("", &newgame->notcontain);
									if (i > 0)
										newgame->appendString(";", &newgame->notcontain);
									newgame->appendString(ret2, &newgame->notcontain);

									i++;
									mir_snprintf(ret, SIZEOF(ret), "CommandLineMustNotContain[%d]", i);
								}

								newgame->setNameandIcon();

								mir_strcat(gamelist, gamename);
								mir_strcat(gamelist, ", ");
								if (foundgames % 2 == 1)
									mir_strcat(gamelist, "\r\n");

								xgamelist.Addgame(newgame);

								foundgames++;

								split = FALSE;
							}

						}

						RegCloseKey(hsubk);
					}
				}

			}
			else if (!MatchExe && xfire_GetPrivateProfileString(temp, "LauncherDirDefault", "", ret2, 255, inipath))
			{
				if (xfire_GetPrivateProfileString(temp, "LauncherExe", "", ret, 255, inipath))
				{
					mir_strcat(ret2, "\\");
					mir_strcat(ret2, ret);
				}

				str_replace(ret2, "%WINDIR%", getenv("WINDIR"));
				str_replace(ret2, "%ProgramFiles%", getenv("ProgramFiles"));

				//prüfe ob existent, dann ist das spiel installiert
				if (GetFileAttributesA(ret2) != 0xFFFFFFFF)
				{

					Xfire_game* newgame = new Xfire_game();

					newgame->id = i;

					//8.3 pfade umwandeln
					//GetLongPathNameA(ret2,ret2,sizeof(ret2));

					//lowercase pfad
					for (unsigned int i = 0; i < mir_strlen(ret2); i++)
						ret2[i] = tolower(ret2[i]);

					newgame->setString(ret2, &newgame->path);

					//launch parameterstring

					//pfad aufbereiten
					char launchpath[XFIRE_MAX_STATIC_STRING_LEN] = "";
					mir_strcpy(launchpath, ret2);
					if (strrchr(launchpath, '\\') != 0)
					{
						*(strrchr(launchpath, '\\')) = 0;
					}

					newgame->setString(ret2, &newgame->launchparams);
					newgame->appendString(" ", &newgame->launchparams);

					xfire_GetPrivateProfileString(temp, "Launch", "", ret2, 512, inipath);

					str_replace(ret2, "%UA_LAUNCHER_EXE_DIR%", launchpath);

					str_replace(ret2, "%UA_LAUNCHER_EXE_PATH%", ""); //unwichtige sachen entfernen
					//str_replace(ret2,"%UA_LAUNCHER_EXTRA_ARGS%",""); // - auch entfernen	
					str_replace(ret2, "%UA_LAUNCHER_LOGIN_ARGS%", ""); // - auch entfernen

					newgame->appendString(ret2, &newgame->launchparams);

					if (xfire_GetPrivateProfileString(temp, "LauncherPasswordArgs", "", ret2, 512, inipath))
						newgame->setString(ret2, &newgame->pwparams);

					if (xfire_GetPrivateProfileString(temp, "LauncherNetworkArgs", "", ret2, 512, inipath))
						newgame->setString(ret2, &newgame->networkparams);

					if (xfire_GetPrivateProfileString(temp, "CommandLineMustContain[0]", "", ret2, 512, inipath))
						newgame->setString(ret2, &newgame->mustcontain);

					if (xfire_GetPrivateProfileString(temp, "XUSERSendId", "", ret2, 512, inipath))
						newgame->send_gameid = atoi(ret2);

					if (xfire_GetPrivateProfileString(temp, "XUSERSetStatusMsg", "", ret2, 512, inipath))
						newgame->setstatusmsg = atoi(ret2);

					//soll alle string, welche nicht in der commandline eines spiels sein soll in einen string pakcen semikolon getrennt
					mir_snprintf(ret, SIZEOF(ret), "CommandLineMustNotContain[0]");
					int i = 0;

					while (xfire_GetPrivateProfileString(temp, ret, "", ret2, 512, inipath))
					{
						if (!newgame->notcontain) newgame->setString("", &newgame->notcontain);
						if (i > 0)
							newgame->appendString(";", &newgame->notcontain);
						newgame->appendString(ret2, &newgame->notcontain);

						i++;
						mir_snprintf(ret, SIZEOF(ret), "CommandLineMustNotContain[%d]", i);
					}

					newgame->setNameandIcon();

					mir_strcat(gamelist, gamename);
					mir_strcat(gamelist, ", ");
					if (foundgames % 2 == 1)
						mir_strcat(gamelist, "\r\n");

					xgamelist.Addgame(newgame);

					foundgames++;

					split = FALSE;
				}

			}

			if (split)
				i2++;
			else
				i++;
			notfound = 0;
		}
		else if (split == FALSE) // nichts gefunden, dann die 2. gameid probieren (games die zb über steam und normal installiert werden können, haben eine 2. id)
		{
			i2 = 1;
			split = TRUE;
		}
		else if (split == TRUE&&last_gpps != NULL) // keine weiteren einträge mit der 2. id gefunden, also wieder mit der normalen weitersuchen
		{
			split = FALSE;
			i++;
			i2 = 0;
		}
		else if (split == TRUE&&last_gpps == NULL) // überhaupt nix gefunden, vllt nicht belegete id's, überspringen und "notfound" hochsetzen
		{
			split = FALSE;
			i2 = 0;
			if (i == 35) //großer sprung unbenutzer id's
				i += 4000;
			i++;
			notfound++;
		}
	}

	if (hwnd)SetDlgItemText(hwnd, IDC_CURRENTGAME, TranslateT("Write to database..."));

	//gefundene games in db eintragen
	xgamelist.writeDatabase();

	//scanversion setzen, um ungewollten rescan zu vermeiden
	db_set_w(NULL, protocolname, "scanver", XFIRE_SCAN_VAL);

	EndDialog(hwnd, 0);

	if (!db_get_b(NULL, protocolname, "dontdisresults", 0))
	{
		int p = mir_strlen(gamelist) - 2;
		if (p > -1)
			gamelist[p] = 0; //letztes koma killen
		mir_snprintf(ret, SIZEOF(ret), Translate("Games found:%s%s"), "\r\n\r\n", gamelist);
		MSGBOX(ret);
	}

	searching = FALSE;

	xgamelist.createStartmenu();

	//gamelist unblocken
	xgamelist.Block(FALSE);
}