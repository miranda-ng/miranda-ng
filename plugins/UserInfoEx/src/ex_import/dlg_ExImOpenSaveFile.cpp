/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "..\commonheaders.h"

#define HKEY_MIRANDA_PLACESBAR	_T("Software\\Miranda NG\\PlacesBar")
#define HKEY_WINPOL_PLACESBAR	_T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ComDlg32\\PlacesBar")

/**
 * This function maps the current users registry to a dummy key and
 * changes the policy hive which is responsible for the places to be displayed,
 * so the desired places are visible.
 *
 * @param		nothing
 * @return		nothing
 **/
static void InitAlteredPlacesBar()
{
	HKEY hkMiranda;
	LONG result;

	// create or open temporary hive for miranda specific places
	result = RegCreateKey(HKEY_CURRENT_USER, HKEY_MIRANDA_PLACESBAR, &hkMiranda);
	if (SUCCEEDED(result)) 
	{
		HKEY hkPlacesBar;

		// map the current users registry
		RegOverridePredefKey(HKEY_CURRENT_USER, hkMiranda);
		// open the policy key
		result = RegCreateKey(HKEY_CURRENT_USER, HKEY_WINPOL_PLACESBAR, &hkPlacesBar);
		// install the places bar
		if (SUCCEEDED(result)) 
		{
			DWORD dwFolderID;
			LPSTR p;
			CHAR szMirandaPath[MAX_PATH];
			CHAR szProfilePath[MAX_PATH];

			// default places: Desktop, My Documents, My Computer
			dwFolderID = 0;	 RegSetValueEx(hkPlacesBar, _T("Place0"), 0, REG_DWORD, (PBYTE)&dwFolderID, sizeof(DWORD));
			dwFolderID = 5;	RegSetValueEx(hkPlacesBar, _T("Place1"), 0, REG_DWORD, (PBYTE)&dwFolderID, sizeof(DWORD));
			dwFolderID = 17; RegSetValueEx(hkPlacesBar, _T("Place2"), 0, REG_DWORD, (PBYTE)&dwFolderID, sizeof(DWORD));

			// Miranda's installation path
			GetModuleFileNameA(GetModuleHandle(NULL), szMirandaPath, SIZEOF(szMirandaPath)); 
			p = mir_strrchr(szMirandaPath, '\\');
			if (p) 
			{
				RegSetValueExA(hkPlacesBar, "Place3", 0, REG_SZ, (PBYTE)szMirandaPath, (p - szMirandaPath) + 1);
			}

			// Miranda's profile path
			if (!CallService(MS_DB_GETPROFILEPATH, SIZEOF(szProfilePath), (LPARAM)szProfilePath))
			{
				// only add if different from profile path
				RegSetValueExA(hkPlacesBar, "Place4", 0, REG_SZ, (PBYTE)szProfilePath, (DWORD)mir_strlen(szProfilePath) + 1);
			}

			RegCloseKey(hkPlacesBar);
		}
		RegCloseKey(hkMiranda);
	}
}

/**
 * name:		ResetAlteredPlaceBars
 * desc:		Remove the mapping of current users registry
 *				and delete the temporary registry hive
 * params:		nothing
 * return:		nothing
 **/
static void ResetAlteredPlaceBars()
{
	RegOverridePredefKey(HKEY_CURRENT_USER, NULL);
	SHDeleteKey(HKEY_CURRENT_USER, HKEY_MIRANDA_PLACESBAR);
}

/**
 * name:		PlacesBarSubclassProc
 * params:		hWnd	- handle, to control's window
 *				uMsg	- the message to handle
 *				wParam	- message dependend parameter
 *				lParam	- message dependend parameter
 * return:		depends on message
 **/
static LRESULT CALLBACK PlacesBarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == TB_ADDBUTTONS) {
		TBBUTTON *tbb = (TBBUTTON *)lParam;
		TCHAR szBtnText[MAX_PATH];
		int iString;
		HWND hWndToolTip;

		if (tbb) {
			// miranda button
			switch (tbb->idCommand) {
			case 41063:
				mir_tstrncpy(szBtnText, TranslateT("Miranda NG"), SIZEOF(szBtnText));
				iString = SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)szBtnText);
				if (iString != -1) tbb->iString = iString;
				// set tooltip
				hWndToolTip = (HWND)SendMessage(hWnd, TB_GETTOOLTIPS, NULL, NULL);
				if (hWndToolTip) {
					TOOLINFO ti;

					memset(&ti, 0, sizeof(ti));
					ti.cbSize = sizeof(ti);
					ti.hwnd = hWnd;
					ti.lpszText = TranslateT("Shows Miranda's installation directory.");
					ti.uId = tbb->idCommand;
					SendMessage(hWndToolTip, TTM_ADDTOOL, NULL, (LPARAM)&ti);
				}
				break;
				// profile button
			case 41064:
				// set button text
				iString = SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM) TranslateT("Profile"));
				if (iString != -1) tbb->iString = iString;

				// set tooltip
				hWndToolTip = (HWND)SendMessage(hWnd, TB_GETTOOLTIPS, NULL, NULL);
				if (hWndToolTip) {
					TOOLINFO ti;

					memset(&ti, 0, sizeof(ti));
					ti.cbSize = sizeof(ti);
					ti.hwnd = hWnd;
					ti.lpszText = TranslateT("Shows the directory with all your Miranda's profiles.");
					ti.uId = tbb->idCommand;
					SendMessage(hWndToolTip, TTM_ADDTOOL, NULL, (LPARAM)&ti);
				}
				// unmap registry and delete keys
				ResetAlteredPlaceBars();
				break;
			}
		}
	}
	return mir_callNextSubclass(hWnd, PlacesBarSubclassProc, uMsg, wParam,lParam);
}

/**
 * name:		OpenSaveFileDialogHook
 * desc:		it subclasses the places bar to provide the own interface for adding places
 * params:		hDlg	- handle, to control's window
 *				uMsg	- the message to handle
 *				wParam	- message dependend parameter
 *				lParam	- message dependend parameter
 * return:		depends on message
 **/
static UINT_PTR CALLBACK OpenSaveFileDialogHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == CDN_INITDONE) {
			HWND hPlacesBar = GetDlgItem(GetParent(hDlg), ctl1);

			// we have a places bar?
			if (hPlacesBar != NULL) {
				InitAlteredPlacesBar();
				// finally subclass the places bar
				mir_subclassWindow(hPlacesBar, PlacesBarSubclassProc);
			}
		}
		break;
	case WM_DESTROY:
		// unmap registry and delete keys 
		// (is to make it sure, if somehow the last places button was not added which also calls this function)
		ResetAlteredPlaceBars();
		break;
	}
	return FALSE;
}



/**
 * name:		GetInitialDir
 * desc:		read the last vCard directory from database
 * pszInitialDir	- buffer to store the initial dir to (size must be MAX_PATH)
 * return:		nothing
 **/
static void GetInitialDir(LPSTR pszInitialDir)
{
	char szRelative[MAX_PATH];

	szRelative[0] = 0;
	// is some standard path defined
	if (!db_get_static(0, MODNAME, "vCardPath", szRelative, SIZEOF(szRelative))) {
		if (!PathToAbsolute(szRelative, pszInitialDir))
			mir_strcpy(pszInitialDir, szRelative);
	}
	else *pszInitialDir = 0;
}

/**
 * name:		SaveInitialDir
 * desc:		save the last vCard directory from database
 *				pszInitialDir	- buffer to store the initial dir to (size must be MAX_PATH)
 * return:		nothing
 **/
static void SaveInitialDir(LPSTR pszInitialDir)
{
	CHAR szRelative[MAX_PATH];
	LPSTR p;

	if (p = mir_strrchr(pszInitialDir, '\\')) {
		*p = 0;
		if ( PathToRelative(pszInitialDir, szRelative))
			db_set_s(0, MODNAME, "vCardPath", szRelative);
		else
			db_set_s(0, MODNAME, "vCardPath", pszInitialDir);
		*p = '\\';
	}	
}

/**
 * name:		InitOpenFileNameStruct
 * desc:		initialize the openfilename structure
 * params:		pofn			- OPENFILENAME structure to initialize
 *				hWndParent		- parent window
 *				pszTitle		- title for the dialog
 *				pszFilter		- the filters to offer
 *				pszInitialDir	- buffer to store the initial dir to (size must be MAX_PATH)
 *				pszFile			- this is the buffer to store the file to (size must be MAX_PATH)
 * return:		nothing
 **/
static void InitOpenFileNameStruct(OPENFILENAMEA *pofn, HWND hWndParent, LPCSTR pszTitle, LPCSTR pszFilter, LPSTR pszInitialDir, LPSTR pszFile)
{
	memset(pofn, 0, sizeof(OPENFILENAME));

	pofn->hwndOwner			= hWndParent;
	pofn->lpstrTitle		= pszTitle;
	pofn->lpstrFilter		= pszFilter;
	pofn->lpstrFile			= pszFile;
	pofn->nMaxFile			= MAX_PATH;
	pofn->lpstrDefExt		= "xml";

	GetInitialDir(pszInitialDir);
	pofn->lpstrInitialDir = pszInitialDir;
	pofn->lStructSize = sizeof (OPENFILENAME);
	pofn->Flags = OFN_NONETWORKBUTTON|OFN_ENABLESIZING|OFN_ENABLEHOOK|OFN_EXPLORER;
	pofn->lpfnHook = OpenSaveFileDialogHook;
}


/**
 * name:		DlgExIm_OpenFileName
 * desc:		displayes a slightly modified OpenFileName DialogBox
 * params:		hWndParent		- parent window
 *				pszTitle		- title for the dialog
 *				pszFilter		- the filters to offer
 *				pszFile			- this is the buffer to store the file to (size must be MAX_PATH)
 * return:		-1 on error/abort or filter index otherwise
 **/
int DlgExIm_OpenFileName(HWND hWndParent, LPCSTR pszTitle, LPCSTR pszFilter, LPSTR pszFile)
{
	OPENFILENAMEA ofn;
	CHAR szInitialDir[MAX_PATH];

	InitOpenFileNameStruct(&ofn, hWndParent, pszTitle, pszFilter, szInitialDir, pszFile);
	ofn.Flags |= OFN_PATHMUSTEXIST;
	if (!GetOpenFileNameA(&ofn)) {
		DWORD dwError = CommDlgExtendedError();
		if (dwError) MsgErr(ofn.hwndOwner, LPGENT("The OpenFileDialog returned an error: %d!"), dwError);
		return -1;
	}
	SaveInitialDir(pszFile);
	return ofn.nFilterIndex;
}

/**
 * name:		DlgExIm_SaveFileName
 * desc:		displayes a slightly modified SaveFileName DialogBox
 * params:		hWndParent		- parent window
 *				pszTitle		- title for the dialog
 *				pszFilter		- the filters to offer
 *				pszFile			- this is the buffer to store the file to (size must be MAX_PATH)
 * return:		-1 on error/abort or filter index otherwise
 **/
int DlgExIm_SaveFileName(HWND hWndParent, LPCSTR pszTitle, LPCSTR pszFilter, LPSTR pszFile)
{
	OPENFILENAMEA ofn;
	CHAR szInitialDir[MAX_PATH];

	InitOpenFileNameStruct(&ofn, hWndParent, pszTitle, pszFilter, szInitialDir, pszFile);
	ofn.Flags |= OFN_OVERWRITEPROMPT;

	if (!GetSaveFileNameA(&ofn)) {
		DWORD dwError = CommDlgExtendedError();

		if (dwError) MsgErr(ofn.hwndOwner, LPGENT("The SaveFileDialog returned an error: %d!"), dwError);
		return -1;
	}
	SaveInitialDir(pszFile);
	return ofn.nFilterIndex;
}
