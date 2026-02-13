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


#ifndef __MIR_SMILEYS_H__
# define __MIR_SMILEYS_H__

#include <windows.h>
#include <m_system.h>

// Init settings needed to draw smileys using the contact list itens
// To use then, pass "clist" as the protocol name
// Need to be called on ME_SYSTEM_MODULESLOADED
int InitContactListSmileys();


// Pre-parse smileys
typedef struct _SmileysParseInfo
{
	SortedList *pieces;
	int max_height;
} *SmileysParseInfo;

SmileysParseInfo Smileys_PreParse(LPCSTR lpString, int nCount, const char *protocol);
void Smileys_FreeParse(SmileysParseInfo parseInfo);

// TODO:
// SmileysParseInfo Smileys_PreParseW(HDC hDC, LPCWSTR lpString, int nCount, const char *protocol);


#define DT_RESIZE_SMILEYS    0x10000000

// Similar to DrawText win32 api function
// Pass uFormat | DT_CALCRECT to calc rectangle to be returned by lpRect
// parseInfo is optional (pass NULL and it will be calculated and deleted inside function)
int Smileys_DrawText(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, const char *protocol, SmileysParseInfo parseInfo);

// TODO:
// int Smileys_DrawTextW(HDC hDC, LPCWSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, const char *protocol, SmileysParseInfo parseInfo);


int skin_DrawText(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);
int skin_DrawIconEx(HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags);



#endif // __MIR_SMILEYS_H__
