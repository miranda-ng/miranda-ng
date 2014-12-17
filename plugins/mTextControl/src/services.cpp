/*
Miranda Text Control - Plugin for Miranda IM
Copyright (C) 2005 Victor Pavlychko (nullbie@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "headers.h"

static HANDLE hService[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

INT_PTR MText_Register(WPARAM, LPARAM);
INT_PTR MText_Create(WPARAM, LPARAM);
INT_PTR MText_CreateW(WPARAM, LPARAM);
INT_PTR MText_CreateEx(WPARAM, LPARAM);
INT_PTR MText_Measure(WPARAM, LPARAM);
INT_PTR MText_Display(WPARAM, LPARAM);
INT_PTR MText_SetParent(WPARAM, LPARAM);
INT_PTR MText_SendMessage(WPARAM, LPARAM);
INT_PTR MText_CreateProxy(WPARAM, LPARAM);
INT_PTR MText_Destroy(WPARAM, LPARAM);
INT_PTR MText_GetInterface(WPARAM, LPARAM);

struct TextObject
{
	DWORD options;
	IFormattedTextDraw *ftd;
	TextObject() : options(0), ftd(0) {}
	~TextObject() { if (ftd) delete ftd; }
};

//---------------------------------------------------------------------------
// elper functions
void MText_InitFormatting0(IFormattedTextDraw *ftd, DWORD)
{
	LRESULT lResult;

	// urls
	ftd->getTextService()->TxSendMessage(EM_AUTOURLDETECT, TRUE, 0, &lResult);
}

void MText_InitFormatting1(TextObject *text)
{
	// bbcodes
	bbCodeParse(text->ftd);

	// smilies
	//	HWND hwnd = (HWND)CallServiceSync(MS_TEXT_CREATEPROXY, (WPARAM)text, 0);
	HWND hwnd = CreateProxyWindow(text->ftd->getTextService());
	SMADD_RICHEDIT3 sm = { 0 };
	sm.cbSize = sizeof(sm);
	sm.hwndRichEditControl = hwnd;
	sm.rangeToReplace = 0;
	sm.Protocolname = 0;
	sm.flags = SAFLRE_INSERTEMF;
	CallService(MS_SMILEYADD_REPLACESMILEYS, RGB(0xff, 0xff, 0xff), (LPARAM)&sm);
	DestroyWindow(hwnd);

	//	text->ftd->getTextService()->TxSendMessage(EM_SETSEL, 0, -1, &lResult);
	/*
		// rtl stuff
		PARAFORMAT2 pf2;
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_ALIGNMENT|PFM_RTLPARA;
		pf2.wEffects = PFE_RTLPARA;
		pf2.wAlignment = PFA_RIGHT;
		text->ftd->getTextService()->TxSendMessage(EM_SETSEL, 0, -1, &lResult);
		text->ftd->getTextService()->TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf2, &lResult);
		text->ftd->getTextService()->TxSendMessage(EM_SETSEL, 0, 0, &lResult);
		*/
}

//---------------------------------------------------------------------------
// NEW mtextcontrol interface:
//
// obtain the full mtextcontrol interface from the library. it is much faster as use of 
// miranda core CallService to access to mtextcontrol (no core traffic).
// This interface provides full access to mtextcontrol internal functions,
// thus enabling devs to fully utilize the mtextcontrol API.
// All functions will be exported as miranda services for compatibility.
//
// the interface is populated during the Load(void) handler, so you can assume it is ready when Miranda
// throw the ME_SYSTEM_MODULESLOADED event and you can generate a warning in your ModulesLoaded() when
// it depends on the mtextcontrol interface and the mtextcontrol plugin is missing.
// 
// example:
// 
// MTEXT_INTERFACE MText = {0};
// 
// 	mir_getMTI(&MText);
// 
// all interface function designed as old mtextcontrol helper functions.
// therefore it is easy to convert your old plugin code to new interface.
//
// example:
//
// old code: MTextCreate (...
// new code: MText.Create(...

//---------------------------------------------------------------------------
// subscribe to MText services
INT_PTR MText_Register(WPARAM wParam, LPARAM lParam)
{
	DWORD userOptions = (DWORD)wParam;
	char *userTitle = (char *)lParam;
	return (INT_PTR)MTI_TextUserAdd(userTitle, userOptions);
}

//---------------------------------------------------------------------------
// allocate text object (unicode)
HANDLE DLL_CALLCONV
MTI_MTextCreateW(HANDLE userHandle, WCHAR *text)
{

	TextObject *result = new TextObject;
	result->options = TextUserGetOptions(userHandle);
	result->ftd = new CFormattedTextDraw;
	result->ftd->Create();
	InitRichEdit(result->ftd->getTextService());

	MText_InitFormatting0(result->ftd, result->options);
	result->ftd->putTextW(text);
	MText_InitFormatting1(result);

	return (HANDLE)result;

}

INT_PTR MText_CreateW(WPARAM wParam, LPARAM lParam)
{

	//HANDLE userHandle = (HANDLE)wParam;
	//WCHAR *wtext = (WCHAR *)lParam;
	return (INT_PTR)(HANDLE)MTI_MTextCreateW((HANDLE)wParam, (WCHAR *)lParam);

}

//---------------------------------------------------------------------------
// allocate text object (advanced)
HANDLE DLL_CALLCONV
MTI_MTextCreateEx(HANDLE userHandle, void *text, DWORD flags)
{
	TextObject *result = new TextObject;
	result->options = TextUserGetOptions(userHandle);
	result->ftd = new CFormattedTextDraw;
	result->ftd->Create();
	InitRichEdit(result->ftd->getTextService());

	MText_InitFormatting0(result->ftd, result->options);
	if (flags & MTEXT_FLG_WCHAR) result->ftd->putTextW((WCHAR *)text);
	else result->ftd->putTextA((char *)text);
	MText_InitFormatting1(result);

	return 0;
}

INT_PTR MText_CreateEx(WPARAM wParam, LPARAM lParam)
{
	HANDLE userHandle = (HANDLE)wParam;
	MTEXTCREATE *textCreate = (MTEXTCREATE *)lParam;
	MTI_MTextCreateEx(userHandle, textCreate->text, textCreate->flags);
	return 0;
}

//---------------------------------------------------------------------------
// measure text object
int DLL_CALLCONV
MTI_MTextMeasure(HDC dc, SIZE *sz, HANDLE text)
{
	if (!text) return 0;

	long lWidth = sz->cx, lHeight = sz->cy;
	((TextObject *)text)->ftd->get_NaturalSize(dc, &lWidth, &lHeight);
	sz->cx = lWidth;
	sz->cy = lHeight;
	//	FancyMeasure(((TextObject *)text)->fancy, displayInfo);

	return 0;
}

INT_PTR MText_Measure(WPARAM wParam, LPARAM)
{
	LPMTEXTDISPLAY displayInfo = (LPMTEXTDISPLAY)wParam;
	if (!displayInfo) return 0;
	if (!(TextObject *)displayInfo->text) return 0;
	MTI_MTextMeasure(displayInfo->dc, &displayInfo->sz, displayInfo->text);
	return 0;
}

int DLL_CALLCONV
//---------------------------------------------------------------------------
// display text object
MTI_MTextDisplay(HDC dc, POINT pos, SIZE sz, HANDLE text)
{
	if (!text) return 0;

	COLORREF cl = GetTextColor(dc);
	//	if (GetTextColor(dc)&0xffffff != 0)
	{
		LRESULT lResult;
		CHARFORMAT cf = { 0 };
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_COLOR;
		cf.crTextColor = cl;
		((TextObject *)text)->ftd->getTextService()->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &lResult);
	}

	SetBkMode(dc, TRANSPARENT);

	long lWidth = sz.cx, lHeight;
	((TextObject *)text)->ftd->get_NaturalSize(dc, &lWidth, &lHeight);
	RECT rt;
	rt.left = pos.x;
	rt.top = pos.y;
	rt.right = pos.x + lWidth;
	rt.bottom = pos.y + lHeight;
	((TextObject *)text)->ftd->Draw(dc, &rt);

	return 0;
}

INT_PTR MText_Display(WPARAM wParam, LPARAM)
{
	LPMTEXTDISPLAY displayInfo = (LPMTEXTDISPLAY)wParam;
	if (!displayInfo) return 0;
	if (!displayInfo->text) return 0;
	MTI_MTextDisplay(displayInfo->dc, displayInfo->pos, displayInfo->sz, displayInfo->text);
	return 0;
}

int DLL_CALLCONV
//---------------------------------------------------------------------------
// set parent window for text object (this is required for mouse handling, etc)
MTI_MTextSetParent(HANDLE text, HWND hwnd, RECT rect)
{
	if (!text) return 0;
	((TextObject *)text)->ftd->setParentWnd(hwnd, rect);
	return 0;
}

INT_PTR MText_SetParent(WPARAM wParam, LPARAM)
{
	LPMTEXTSETPARENT info = (LPMTEXTSETPARENT)wParam;
	//TextObject *text = (TextObject *)info->text;
	if (!info) return 0;
	if (!info->text) return 0;
	MTI_MTextSetParent(info->text, info->hwnd, info->rc);
	return 0;
}

//---------------------------------------------------------------------------
// send message to an object
int DLL_CALLCONV
MTI_MTextSendMessage(HWND hwnd, HANDLE text, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	if (!text) return 0;
	((TextObject *)text)->ftd->getTextService()->TxSendMessage(msg, wParam, lParam, &lResult);

	if (hwnd && (msg == WM_MOUSEMOVE)) {
		HDC hdc = GetDC(hwnd);
		((TextObject *)text)->ftd->getTextService()->OnTxSetCursor(DVASPECT_CONTENT, 0, NULL, NULL, hdc, NULL, NULL, LOWORD(0), HIWORD(0));
		ReleaseDC(hwnd, hdc);
	}

	return lResult;
}

INT_PTR MText_SendMessage(WPARAM wParam, LPARAM)
{
	LPMTEXTMESSAGE message = (LPMTEXTMESSAGE)wParam;
	if (!message->text) return 0;
	return (INT_PTR)MTI_MTextSendMessage(message->hwnd, message->text, message->msg, message->wParam, message->lParam);
}

//---------------------------------------------------------------------------
// create a proxy window
HWND DLL_CALLCONV
MTI_MTextCreateProxy(HANDLE text)
{
	if (!text) return 0;
	return CreateProxyWindow(((TextObject *)text)->ftd->getTextService());
}

INT_PTR MText_CreateProxy(WPARAM wParam, LPARAM)
{
	if (!wParam) return 0;
	return (INT_PTR)MTI_MTextCreateProxy((HANDLE)wParam);
}

int DLL_CALLCONV
//---------------------------------------------------------------------------
// destroy text object
MTI_MTextDestroy(HANDLE text)
{
	//HANDLE textHandle = (HANDLE)wParam;
	//TextObject *text = (TextObject *)textHandle;
	if (text) delete (TextObject *)text;
	return 0;
}

INT_PTR MText_Destroy(WPARAM wParam, LPARAM)
{
	HANDLE textHandle = (HANDLE)wParam;
	TextObject *text = (TextObject *)textHandle;
	if (text) delete text;
	return 0;
}

//---------------------------------------------------------------------------
// populate the interface
INT_PTR MText_GetInterface(WPARAM, LPARAM lParam)
{
	MTEXT_INTERFACE *MText = (MTEXT_INTERFACE *)lParam;
	if (MText == NULL)
		return CALLSERVICE_NOTFOUND;

	MText->version = pluginInfoEx.version;
	MText->Register = MTI_TextUserAdd;
	MText->Create = MTI_MTextCreateW;
	MText->CreateEx = MTI_MTextCreateEx;
	MText->Measure = MTI_MTextMeasure;
	MText->Display = MTI_MTextDisplay;
	MText->SetParent = MTI_MTextSetParent;
	MText->SendMsg = MTI_MTextSendMessage;
	MText->CreateProxy = MTI_MTextCreateProxy;
	MText->Destroy = MTI_MTextDestroy;

	return S_OK;
}

//---------------------------------------------------------------------------
// Load / Unload services
void LoadServices()
{
	CreateServiceFunction(MS_TEXT_REGISTER, MText_Register);
	CreateServiceFunction(MS_TEXT_CREATEW, MText_CreateW);
	CreateServiceFunction(MS_TEXT_CREATEEX, MText_CreateEx);
	CreateServiceFunction(MS_TEXT_MEASURE, MText_Measure);
	CreateServiceFunction(MS_TEXT_DISPLAY, MText_Display);
	CreateServiceFunction(MS_TEXT_SETPARENT, MText_SetParent);
	CreateServiceFunction(MS_TEXT_SENDMESSAGE, MText_SendMessage);
	CreateServiceFunction(MS_TEXT_CREATEPROXY, MText_CreateProxy);
	CreateServiceFunction(MS_TEXT_DESTROY, MText_Destroy);
	CreateServiceFunction(MS_TEXT_GETINTERFACE, MText_GetInterface);
}
