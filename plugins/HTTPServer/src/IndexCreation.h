
//This file is part of HTTPServer a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef HTTP_INDEXCREATION_H
#define HTTP_INDEXCREATION_H

#include <windows.h>
#include "m_HTTPServer.h"
#include "FileShareNode.h"

const TCHAR szIndexHTMLTemplateFile[] = _T("HTTPServerIndex.html");

enum eIndexCreationMode {
	INDEX_CREATION_DISABLE = 0,
	INDEX_CREATION_HTML    = 1,
	INDEX_CREATION_XML     = 2,
	INDEX_CREATION_DETECT  = 4
};

extern eIndexCreationMode indexCreationMode;

bool bCreateIndexXML(const TCHAR *pszRealPath, const TCHAR *pszIndexPath, const TCHAR *pszSrvPath, DWORD dwRemoteIP);
bool bCreateIndexHTML(const TCHAR *pszRealPath, const TCHAR *pszIndexPath, const TCHAR *pszSrvPath, DWORD dwRemoteIP);
void FreeIndexHTMLTemplate();
bool LoadIndexHTMLTemplate();

#endif