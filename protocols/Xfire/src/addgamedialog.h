#ifndef _ADDGAME_H
#define _ADDGAME_H

#include "baseProtocol.h"
#include "Xfire_gamelist.h"

//dialog funktion
INT_PTR CALLBACK DlgAddGameProc(HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	);

INT_PTR CALLBACK DlgAddGameProcMain(HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	);

//struct für den cache
struct lbInicache {
	char name[255];
	unsigned int gameid;
};

void AddGameDialog(HWND hwndDlg, Xfire_game* game = NULL);

#endif