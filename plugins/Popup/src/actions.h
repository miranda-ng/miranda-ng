/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __actions_h__
#define __actions_h__

// Popup/RegisterNotification
// Registers your action in popup action list
// wParam = (WPARAM)(LPPOPUPNOTIFICATION)info
// lParam = 0
// Returns: handle of registered notification or sero on failure

#define PNAF_CALLBACK			0x01

#define POPUP_ACTION_NOTHING	LPGEN("Do nothing")
#define POPUP_ACTION_DISMISS	LPGEN("Dismiss popup")

struct POPUPNOTIFYACTION
{
	char lpzTitle[64];
	uint32_t dwFlags;
	union
	{
		struct
		{
			char lpzLModule[MAXMODULELABELLENGTH];
			char lpzLSetting[MAXMODULELABELLENGTH];
			DBVARIANT dbvLData;
			char lpzRModule[MAXMODULELABELLENGTH];
			char lpzRSetting[MAXMODULELABELLENGTH];
			DBVARIANT dbvRData;
		};
		struct
		{
			uint32_t dwCookie;
			void(*pfnCallback)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, uint32_t cookie);
		};
	};
};

#define PNF_CONTACT				0x01

struct POPUPNOTIFICATION
{
	int cbSize;
	uint32_t dwFlags;			// set of PNF_* flags
	char lpzGroup[MAXMODULELABELLENGTH];
	char lpzName[MAXMODULELABELLENGTH];
	HANDLE lchIcoLib;			// gotten from icolib
	COLORREF colorBack;		// this will be registered in fontservice
	COLORREF colorText;		// this will be registered in fontservice
	int iSeconds;			// default timeout
	int actionCount;		// for unified action comboboxes
	POPUPNOTIFYACTION *lpActions;
	char *lpzLAction;
	char *lpzRAction;
	char *pszReserved1;		// reserved for future use
	#ifdef _WINDOWS
	DLGPROC pfnReserved2;	// reserved for future use
	#endif
};

/////////////////////////////////////////////////////////////////////////////////////////

void LoadActions();
void UnloadActions();

void RegisterAction(POPUPACTION *action);
bool IsActionEnabled(POPUPACTION *action);
bool IsActionEnabled(char *name);

void LoadOption_Actions();
INT_PTR CALLBACK DlgProcPopupActions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // __actions_h__
