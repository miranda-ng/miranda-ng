/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#define MAXVALUE 20

#define MPF_EQUAL		1
#define MPF_DIFF		2
#define MPF_NOT_EQUAL	( MPF_DIFF|MPF_EQUAL )
#define MPF_HASHED		4

struct MASKPARAM
{
	uint32_t	dwId;
	uint8_t	bMaskParamFlag;
	char*	szName;
	uint32_t	dwValueHash;
	char*	szValue;
};

struct MODERNMASK
{
	MASKPARAM*	    pl_Params;
	uint32_t				dwParamCnt;
	union
	{
		void*			pObject;
		char*			szObjectName;
	};
	uint32_t				dwMaskId;
	BOOL				bObjectFound;
};

struct LISTMODERNMASK
{
	MODERNMASK*	pl_Masks;
	uint32_t			dwMaskCnt;
};

/// PROTOTYPES
int AddStrModernMaskToList(uint32_t maskID, char *szStr, char *objectName, LISTMODERNMASK *mmTemplateList);
int SortMaskList(LISTMODERNMASK *mmList);
int ClearMaskList(LISTMODERNMASK *mmTemplateList);

BOOL CompareStrWithModernMask(char *szValue, MODERNMASK *mmTemplate);
uint32_t mod_CalcHash(const char *a);
int RegisterObjectByParce(char *ObjectName, char *Params);
SKINOBJECTDESCRIPTOR* skin_FindObjectByRequest(char *szValue, LISTMODERNMASK *mmTemplateList);
SKINOBJECTDESCRIPTOR* skin_FindObjectByMask(MODERNMASK *mm, LISTMODERNMASK *mmTemplateList);
wchar_t * GetParamNT(char *string, wchar_t *buf, int buflen, uint8_t paramN, char Delim, BOOL SkipSpaces);
int SkinDrawGlyphMask(HDC hdc, RECT *rcSize, RECT *rcClip, MODERNMASK *ModernMask);
#endif
