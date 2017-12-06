// History Linklist Plus
// Copyright (C) 2010 Thomas Wendel, gureedo
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#pragma once

#ifndef _LINKLIST_FCT_H
#define _LINKLIST_FCT_H

void DrawLine(HWND, size_t);
BYTE GetFlags(HMENU);
void GetFilterText(HMENU, LPTSTR, size_t);
void GetListInfo(BYTE, LISTELEMENT*, LPCTSTR, size_t*, size_t*, size_t*);
void GetListOptions(LISTOPTIONS*);
void SetListOptions(LISTOPTIONS*);
void ClearLinePos(LISTELEMENT*);
int GetLastLinePos(LISTELEMENT*);
void WriteLinkList(HWND, BYTE, LISTELEMENT*, LPCTSTR, int);
int WriteOptionExample(HWND, DWORD, DWORD, DWORD, DWORD, LISTOPTIONS*);
void WriteMessage(HWND, LISTELEMENT*, int);
void GetColour(MYCOLOURSET*);
void GetDBColour(MYCOLOURSET*);
void SetDBColour(MYCOLOURSET*);
int GetMirandaColour(MYCOLOURSET*);
BYTE GetUpdateSetting(void);
int LinklistResizer(HWND,LPARAM,UTILRESIZECONTROL*);
// RTF Save functions
BOOL SaveEditAsStream( HWND );
DWORD CALLBACK RTFSaveStreamCallback( DWORD_PTR, LPBYTE, LONG, LONG * );

#endif //_LINKLIST_FCT_H