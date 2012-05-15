/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/
#include "hdr/modern_commonheaders.h"
#include "hdr/modern_popup.h"


BOOL EnablePopups = FALSE;

static LRESULT CALLBACK DumbPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// Show an error popup
void ShowErrPopup(const char *title, const char *description)
{
	ShowPopup(title == NULL ? "Modern Contact List Error" : title, description, POPUP_TYPE_ERROR);
}


// Show an trace popup
void ShowTracePopup(const char *text)
{
#ifdef _DEBUG
	static int num = 0;
	char tmp[1024];

	mir_snprintf(tmp, SIZEOF(tmp), "CM TRACE %d: %s", num, text);
	TRACE(tmp);
	TRACE("\n");
	num++;

	ShowPopup(tmp, "", POPUP_TYPE_NORMAL);
#endif
}


// Show an popup
void ShowPopup(const char *title, const char *description, int type)
{
	POPUPDATAEX ppd;
	int ret;

	if(!ServiceExists(MS_POPUP_ADDPOPUPEX) || !EnablePopups) 
	{
		return;
	}

	// Make popup
	ZeroMemory(&ppd, sizeof(ppd)); 
	ppd.lchContact = 0; 
	ppd.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);

	strncpy(ppd.lpzContactName, title == NULL ? "Modern Contact List" : title, SIZEOF(ppd.lpzContactName)-1);
	ppd.lpzContactName[SIZEOF(ppd.lpzContactName)-1] = '\0';

	if (description != NULL)
	{
		strncpy(ppd.lpzText, description, SIZEOF(ppd.lpzText)-1);
		ppd.lpzText[SIZEOF(ppd.lpzText)-1] = '\0';
	}

	if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST)
	{
		ppd.colorBack = 0;
		ppd.colorText = 0;
	}
	else // if (type == POPUP_TYPE_ERROR)
	{
		ppd.colorBack = -1;
		ppd.colorText = RGB(255,255,255);
	}

	ppd.PluginWindowProc = DumbPopupDlgProc;
	
	if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST)
	{
		ppd.iSeconds = 0;
	}
	else // if (type == POPUP_TYPE_ERROR)
	{
		ppd.iSeconds = 0;
	}

	// Now that every field has been filled, we want to see the popup.
	ret = CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd,0);

	ret++;
}



// Handle to popup events
static LRESULT CALLBACK DumbPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
		{
			PUDeletePopUp(hWnd);
			return TRUE;
		}

		case WM_CONTEXTMENU: 
		{
			PUDeletePopUp(hWnd);
			return TRUE;
		}

		case UM_FREEPLUGINDATA: 
		{
			return TRUE;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

