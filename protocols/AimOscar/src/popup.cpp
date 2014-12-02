/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "aim.h"

struct CAimPopupData
{
	CAimPopupData(CAimProto* _ppro, char* _url) :
		ppro(_ppro),
		url(mir_strdup(_url))
	{}

	~CAimPopupData()
	{ mir_free(url); }

	CAimProto* ppro;
	char* url;
};

LRESULT CALLBACK PopupWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED)
		{
			CAimPopupData* p = (CAimPopupData*)PUGetPluginData(hWnd);
			if (p->url != NULL)
				ShellExecuteA(NULL, "open", p->url, NULL, NULL, SW_SHOW);

			PUDeletePopup(hWnd);
			return 0;
		}
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		CAimPopupData* p = (CAimPopupData*)PUGetPluginData(hWnd);
		ReleaseIconEx("aim");
		delete p;
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CAimProto::ShowPopup(const char* msg, int flags, char* url)
{
	POPUPDATAT ppd = {0};

	mir_sntprintf(ppd.lptzContactName, SIZEOF(ppd.lptzContactName), TranslateT("%s Protocol"), m_tszUserName);

	if (flags & ERROR_POPUP) 
	{
		if (flags & TCHAR_POPUP)
		{
			char* errmsg = mir_t2a((TCHAR*)msg);
			debugLogA(errmsg);
			mir_free(errmsg);
		}
		else
			debugLogA(msg);
	}

	TCHAR *msgt = (flags & TCHAR_POPUP) ? mir_tstrdup((TCHAR*)msg) : mir_a2t(msg);
	_tcsncpy_s(ppd.lptzText, TranslateTS(msgt), _TRUNCATE);
	mir_free(msgt);

	if (!ServiceExists(MS_POPUP_ADDPOPUPT))
	{	
		if (flags & MAIL_POPUP)
		{
			size_t len = _tcslen(ppd.lptzText);
			mir_sntprintf(&ppd.lptzText[len], SIZEOF(ppd.lptzText) - len, _T(" %s"), TranslateT("Open mail account?"));
			if (MessageBox(NULL, ppd.lptzText, ppd.lptzContactName, MB_YESNO | MB_ICONINFORMATION) == IDYES)
				ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOW);
		}
		else
		{
			MessageBox(NULL, ppd.lptzText, ppd.lptzContactName, MB_OK | MB_ICONINFORMATION);
		}
	}
	else
	{
		ppd.PluginWindowProc = PopupWindowProc;
		ppd.lchIcon = LoadIconEx("aim");
		if (flags & MAIL_POPUP)
		{
			ppd.PluginData = new CAimPopupData(this, url);
			ppd.iSeconds = -1;
		} 
		else 
			ppd.PluginData = new CAimPopupData(this, NULL);

		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);	
	}
}
