////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2011-2012 Bartosz Białek
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
	wchar_t* title;
	wchar_t* text;
	GaduProto* gg;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup plugin window proc
//
LRESULT CALLBACK PopupWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
	{
		PopupData* puData = (PopupData*)PUGetPluginData(hWnd);
		if (puData != nullptr)
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
		if (puData != nullptr && puData != (PopupData*)CALLSERVICE_NOTFOUND)
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
//
void GaduProto::initpopups()
{
	wchar_t szDescr[256];
	char  szName[256];

	POPUPCLASS puc = {};
	puc.PluginWindowProc = PopupWindowProc;
	puc.flags = PCF_UNICODE;
	puc.pszName = szName;
	puc.pszDescription.w = szDescr;

	mir_snprintf(szName, "%s_%s", m_szModuleName, "Notify");
	mir_snwprintf(szDescr, L"%s/%s", m_tszUserName, TranslateT("Notifications"));
	puc.colorBack = RGB(173, 206, 247);
	puc.colorText = GetSysColor(COLOR_WINDOWTEXT);
	puc.hIcon = g_plugin.getIcon(IDI_GG);
	puc.iSeconds = 4;
	hPopupNotify = Popup_RegisterClass(&puc);

	mir_snprintf(szName, "%s_%s", m_szModuleName, "Error");
	mir_snwprintf(szDescr, L"%s/%s", m_tszUserName, TranslateT("Errors"));
	puc.colorBack = RGB(191, 0, 0); // Red
	puc.colorText = RGB(255, 245, 225); // Yellow
	puc.iSeconds = 60;
	puc.hIcon = (HICON)LoadImage(nullptr, IDI_WARNING, IMAGE_ICON, 0, 0, LR_SHARED);
	hPopupError = Popup_RegisterClass(&puc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Show popup - popup plugin support
//
void CALLBACK sttMainThreadCallback(PVOID dwParam)
{
	PopupData *puData = (PopupData*)dwParam;
	GaduProto *gg = puData->gg;

	char szName[256];
	if (puData->flags & GG_POPUP_ERROR || puData->flags & GG_POPUP_WARNING)
		mir_snprintf(szName, "%s_%s", gg->m_szModuleName, "Error");
	else
		mir_snprintf(szName, "%s_%s", gg->m_szModuleName, "Notify");

	POPUPDATACLASS ppd = {};
	ppd.szTitle.w = puData->title;
	ppd.szText.w = puData->text;
	ppd.PluginData = puData;
	ppd.pszClassName = szName;
	Popup_AddClass(&ppd);
}

void GaduProto::showpopup(const wchar_t* nickname, const wchar_t* msg, int flags)
{
	if (Miranda_IsTerminated())
		return;

	PopupData *puData = (PopupData*)mir_calloc(sizeof(PopupData));
	puData->flags = flags;
	puData->title = mir_wstrdup(nickname);
	puData->text = mir_wstrdup(msg);
	puData->gg = this;

	CallFunctionAsync(sttMainThreadCallback, puData);
}
