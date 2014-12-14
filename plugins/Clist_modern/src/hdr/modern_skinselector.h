/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#pragma once

#ifndef M_MOD_SKIN_SELECTOR_H_INC
#define M_MOD_SKIN_SELECTOR_H_INC

#include "newpluginapi.h"
#include "modern_commonheaders.h"

#define MAXVALUE 20

#define MPF_EQUAL		1
#define MPF_DIFF		2
#define MPF_NOT_EQUAL	( MPF_DIFF|MPF_EQUAL )
#define MPF_HASHED		4

struct MASKPARAM
{
	DWORD	dwId;
	BYTE	bMaskParamFlag;
	char*	szName;
	DWORD	dwValueHash;
	char*	szValue;
};

struct MODERNMASK
{
	MASKPARAM*	    pl_Params;
	DWORD				dwParamCnt;
	union
	{
		void*			pObject;
		char*			szObjectName;
	};
	DWORD				dwMaskId;
	BOOL				bObjectFound;
};

struct LISTMODERNMASK
{
	MODERNMASK*	pl_Masks;
	DWORD			dwMaskCnt;
};

/// PROTOTYPES
int AddModernMaskToList(MODERNMASK *mm, LISTMODERNMASK *mmTemplateList);
int AddStrModernMaskToList(DWORD maskID, char *szStr, char *objectName, LISTMODERNMASK *mmTemplateList);
int SortMaskList(LISTMODERNMASK *mmList);

int DeleteMaskByItID(DWORD mID, LISTMODERNMASK *mmTemplateList);
int ClearMaskList(LISTMODERNMASK *mmTemplateList);
int ExchangeMasksByID(DWORD mID1, DWORD mID2, LISTMODERNMASK *mmTemplateList);

int ParseToModernMask(MODERNMASK *mm, char *szText);
BOOL CompareModernMask(MODERNMASK *mmValue, MODERNMASK *mmTemplate);
BOOL CompareStrWithModernMask(char * szValue, MODERNMASK *mmTemplate);
MODERNMASK * FindMaskByStr(char * szValue, LISTMODERNMASK * mmTemplateList);
DWORD mod_CalcHash(const char * a);
char * ModernMaskToString(MODERNMASK *mm, char *buf, UINT bufsize);
int RegisterObjectByParce(char * ObjectName, char *Params);
SKINOBJECTDESCRIPTOR* skin_FindObjectByRequest(char *szValue, LISTMODERNMASK *mmTemplateList);
SKINOBJECTDESCRIPTOR* skin_FindObjectByMask(MODERNMASK *mm, LISTMODERNMASK *mmTemplateList);
TCHAR * GetParamNT(char * string, TCHAR * buf, int buflen, BYTE paramN, char Delim, BOOL SkipSpaces);
int SkinDrawGlyphMask(HDC hdc, RECT *rcSize, RECT *rcClip, MODERNMASK *ModernMask);
#endif
