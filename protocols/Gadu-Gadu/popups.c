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

typedef struct _tag_PopupData
{
	unsigned flags;
	char* title;
	char* text;
	GGPROTO* gg;
} PopupData;

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
					gg_sessions_view(puData->gg, 0, 0);
			}
			PUDeletePopUp(hWnd);
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopUp(hWnd);
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

void gg_initpopups(GGPROTO* gg)
{
	char szDescr[256], szName[256];
	POPUPCLASS puc = {0};

	puc.cbSize = sizeof(puc);
	puc.PluginWindowProc = PopupWindowProc;

	puc.ptszDescription = szDescr;
	puc.pszName = szName;
	puc.colorBack = RGB(173, 206, 247);
	puc.colorText =  GetSysColor(COLOR_WINDOWTEXT);
	puc.hIcon = CopyIcon(LoadIconEx("main", FALSE));
	ReleaseIconEx("main", FALSE);
	puc.iSeconds = 4;
	mir_snprintf(szDescr, SIZEOF(szDescr), "%s/%s", GG_PROTONAME, Translate("Notify"));
	mir_snprintf(szName, SIZEOF(szName), "%s_%s", GG_PROTO, "Notify");
	CallService(MS_POPUP_REGISTERCLASS, 0, (WPARAM)&puc);

	puc.ptszDescription = szDescr;
	puc.pszName = szName;
	puc.colorBack = RGB(191, 0, 0); // Red
	puc.colorText = RGB(255, 245, 225); // Yellow
	puc.iSeconds = 60;
	puc.hIcon = (HICON)LoadImage(NULL, IDI_WARNING, IMAGE_ICON, 0, 0, LR_SHARED);
	mir_snprintf(szDescr, SIZEOF(szDescr), "%s/%s", GG_PROTONAME, Translate("Error"));
	mir_snprintf(szName, SIZEOF(szName), "%s_%s", GG_PROTO, "Error");
	CallService(MS_POPUP_REGISTERCLASS, 0, (WPARAM)&puc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Show popup - popup plugin support

void CALLBACK sttMainThreadCallback(PVOID dwParam)
{
	PopupData* puData = (PopupData*)dwParam;
	GGPROTO* gg = puData->gg;

	if (ServiceExists(MS_POPUP_ADDPOPUPCLASS))
	{
		char szName[256];
		POPUPDATACLASS ppd = {sizeof(ppd)};
		ppd.ptszTitle = puData->title;
		ppd.ptszText = puData->text;
		ppd.PluginData = puData;
		ppd.pszClassName = szName;
		
		if (puData->flags & GG_POPUP_ERROR || puData->flags & GG_POPUP_WARNING)
			mir_snprintf(szName, SIZEOF(szName), "%s_%s", GG_PROTO, "Error");
		else
			mir_snprintf(szName, SIZEOF(szName), "%s_%s", GG_PROTO, "Notify");

		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&ppd);
	}
	else
	{
		if (puData->flags & GG_POPUP_ALLOW_MSGBOX) 
		{
			BOOL bShow = TRUE;

			if (puData->flags & GG_POPUP_ONCE)
			{
				HWND hWnd = FindWindow(NULL, GG_PROTONAME);
				while (hWnd != NULL)
				{
					if (FindWindowEx(hWnd, NULL, NULL, puData->text) != NULL)
					{
						bShow = FALSE;
						break;
					}
					hWnd = FindWindowEx(NULL, hWnd, NULL, GG_PROTONAME);
				}
			}

			if (bShow)
			{
				UINT uIcon = puData->flags & GG_POPUP_ERROR ? MB_ICONERROR : puData->flags & GG_POPUP_WARNING ? MB_ICONEXCLAMATION : MB_ICONINFORMATION;
				MessageBox(NULL, puData->text, GG_PROTONAME, MB_OK | uIcon);
			}
		}
		mir_free(puData->title);
		mir_free(puData->text);
		mir_free(puData);
	}
}

void gg_showpopup(GGPROTO* gg, const char* nickname, const char* msg, int flags)
{
	PopupData* puData;

	if (Miranda_Terminated()) return;

	puData = (PopupData*)mir_alloc(sizeof(PopupData));
	puData->flags = flags;
	puData->title = mir_strdup(nickname);
	puData->text = mir_strdup(msg);
	puData->gg = gg;

	CallFunctionAsync(sttMainThreadCallback, puData);
}
