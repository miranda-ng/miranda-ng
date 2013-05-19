////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2011-2012 Bartosz Bia³ek
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

struct PopupData
{
	unsigned flags;
	TCHAR* title;
	TCHAR* text;
	GGPROTO* gg;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup plugin window proc

LRESULT CALLBACK PopupWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_COMMAND:
		{
			PopupData* puData = (PopupData*)PUGetPluginData(hWnd);
			if (puData != NULL)
			{
				if (puData->flags & GG_POPUP_MULTILOGON)
					puData->gg->sessions_view(0, 0);
			}
			PUDeletePopup(hWnd);
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopup(hWnd);
			break;

		case UM_FREEPLUGINDATA:
		{
			PopupData* puData = (PopupData*)PUGetPluginData(hWnd);
			if (puData != NULL && puData != (PopupData*)CALLSERVICE_NOTFOUND)
			{
				mir_free(puData->title);
				mir_free(puData->text);
				mir_free(puData);
			}
			break;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup plugin class registration

void GGPROTO::initpopups()
{
	TCHAR szDescr[256];
	char  szName[256];

	POPUPCLASS puc = {0};
	puc.cbSize = sizeof(puc);
	puc.PluginWindowProc = PopupWindowProc;
	puc.flags = PCF_TCHAR;
	puc.ptszDescription = szDescr;
	puc.pszName = szName;

	puc.colorBack = RGB(173, 206, 247);
	puc.colorText =  GetSysColor(COLOR_WINDOWTEXT);
	puc.hIcon = CopyIcon(LoadIconEx("main", FALSE));
	ReleaseIconEx("main", FALSE);
	puc.iSeconds = 4;
	mir_sntprintf(szDescr, SIZEOF(szDescr), _T("%s/%s"), m_tszUserName, TranslateT("Notify"));
	mir_snprintf(szName, SIZEOF(szName), "%s_%s", m_szModuleName, "Notify");
	hPopupNotify = Popup_RegisterClass(&puc);

	puc.colorBack = RGB(191, 0, 0); // Red
	puc.colorText = RGB(255, 245, 225); // Yellow
	puc.iSeconds = 60;
	puc.hIcon = (HICON)LoadImage(NULL, IDI_WARNING, IMAGE_ICON, 0, 0, LR_SHARED);
	mir_sntprintf(szDescr, SIZEOF(szDescr), _T("%s/%s"), m_tszUserName, TranslateT("Error"));
	mir_snprintf(szName, SIZEOF(szName), "%s_%s", m_szModuleName, "Error");
	hPopupError = Popup_RegisterClass(&puc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Show popup - popup plugin support

void CALLBACK sttMainThreadCallback(PVOID dwParam)
{
	PopupData* puData = (PopupData*)dwParam;
	GGPROTO* gg = puData->gg;

	if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		char szName[256];
		POPUPDATACLASS ppd = {sizeof(ppd)};
		ppd.ptszTitle = puData->title;
		ppd.ptszText = puData->text;
		ppd.PluginData = puData;
		ppd.pszClassName = szName;
		
		if (puData->flags & GG_POPUP_ERROR || puData->flags & GG_POPUP_WARNING)
			mir_snprintf(szName, SIZEOF(szName), "%s_%s", gg->m_szModuleName, "Error");
		else
			mir_snprintf(szName, SIZEOF(szName), "%s_%s", gg->m_szModuleName, "Notify");

		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&ppd);
		return;
	}

	if (puData->flags & GG_POPUP_ALLOW_MSGBOX) {
		BOOL bShow = TRUE;

		if (puData->flags & GG_POPUP_ONCE) {
			HWND hWnd = FindWindow(NULL, gg->m_tszUserName);
			while (hWnd != NULL) {
				if (FindWindowEx(hWnd, NULL, NULL, puData->text) != NULL) {
					bShow = FALSE;
					break;
				}
				hWnd = FindWindowEx(NULL, hWnd, NULL, gg->m_tszUserName);
			}
		}

		if (bShow) {
			UINT uIcon = puData->flags & GG_POPUP_ERROR ? MB_ICONERROR : puData->flags & GG_POPUP_WARNING ? MB_ICONEXCLAMATION : MB_ICONINFORMATION;
			MessageBox(NULL, puData->text, gg->m_tszUserName, MB_OK | uIcon);
		}
	}
	mir_free(puData->title);
	mir_free(puData->text);
	mir_free(puData);
}

void GGPROTO::showpopup(const TCHAR* nickname, const TCHAR* msg, int flags)
{
	if (Miranda_Terminated()) return;

	PopupData *puData = (PopupData*)mir_calloc(sizeof(PopupData));
	puData->flags = flags;
	puData->title = mir_tstrdup(nickname);
	puData->text = mir_tstrdup(msg);
	puData->gg = this;

	CallFunctionAsync(sttMainThreadCallback, puData);
}
