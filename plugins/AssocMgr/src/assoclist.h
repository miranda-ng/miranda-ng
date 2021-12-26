/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

struct ASSOCDATA : public MZeroedObject
{
	ASSOCDATA() {}

	ASSOCDATA(const ASSOCDATA &p) :
		pszClassName(mir_strdup(p.pszClassName)),
		pszDescription(mir_wstrdup(p.pszDescription)),
		pszService(mir_strdup(p.pszService)),
		pszFileExt(mir_strdup(p.pszFileExt)),
		pszMimeType(mir_strdup(p.pszMimeType)),
		pszVerbDesc(mir_wstrdup(p.pszVerbDesc))
	{
		flags = p.flags;
		hInstance = p.hInstance;
		nIconResID = p.nIconResID;
	}

	ptrA pszClassName;    // class name as used in registry and db
	ptrW pszDescription;
	HINSTANCE hInstance;   // allowed to be NULL for miranda32.exe
	uint16_t nIconResID;
	ptrA pszService;
	uint16_t flags;            // set of FTDF_* and UTDF_* flags
	ptrA pszFileExt;      // file type: NULL for url type
	ptrA pszMimeType;     // file type: allowed to be NULL
	ptrW pszVerbDesc;    // file type: allowed to be NULL
};

/* Assoc Enabled */
void CleanupAssocEnabledSettings(void);
/* Mime Reg */
void CleanupMimeTypeAddedSettings(void);
/* Assoc List Utils */
BOOL IsRegisteredAssocItem(const char *pszClassName);
/* Open Handler */
INT_PTR InvokeFileHandler(const wchar_t *pszFileName);
INT_PTR InvokeUrlHandler(const wchar_t *pszUrl);
/* Misc */
void InitAssocList(void);
void UninitAssocList(void);

#define M_REFRESH_ICONS  (WM_APP+1)
