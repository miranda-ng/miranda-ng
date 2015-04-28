/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

///// structures and services to manage modern skin objects (mask mechanism)

//#include "windows.h"
#include "hdr/modern_commonheaders.h"
#include "hdr/modern_skinselector.h"
#include "hdr/modern_skinengine.h"
#include "m_skin_eng.h"
#include "m_api/m_skinbutton.h"
#include "hdr/modern_commonprototypes.h"

/// IMPLEMENTATIONS
char* ModernMaskToString(MODERNMASK *mm, char * buf, UINT bufsize)
{
	for (int i = 0; i < (int)mm->dwParamCnt; i++) {
		if (mm->pl_Params[i].bMaskParamFlag) {
			if (i>0)
				mir_snprintf(buf, bufsize, "%s%%", buf);
			if (mm->pl_Params[i].bMaskParamFlag &MPF_DIFF)
				mir_snprintf(buf, bufsize, "%s = %s", mm->pl_Params[i].szName, mm->pl_Params[i].szValue);
			else
				mir_snprintf(buf, bufsize, "%s^%s", mm->pl_Params[i].szName, mm->pl_Params[i].szValue);
		}
		else break;
	}
	return buf;
}
int SkinSelector_DeleteMask(MODERNMASK *mm)
{
	if (!mm->pl_Params) return 0;
	for (int i = 0; i < (int)mm->dwParamCnt; i++) {
		free(mm->pl_Params[i].szName);
		free(mm->pl_Params[i].szValue);
	}
	free(mm->pl_Params);
	return 1;
}

BOOL MatchMask(char * name, char * mask)
{
	if (!mask || !name) return mask == name;
	if (*mask != '|') return wildcmpi(name, mask);

	int s = 1, e = 1;
	char * temp;
	while (mask[e] != '\0') {
		s = e;
		while (mask[e] != '\0' && mask[e] != '|') e++;
		temp = (char*)malloc(e - s + 1);
		memcpy(temp, mask + s, e - s);
		temp[e - s] = '\0';
		if (wildcmpi(name, temp)) {
			free(temp);
			return TRUE;
		}
		free(temp);
		if (mask[e] != '\0') e++;
		else return FALSE;
	}
	return FALSE;
}
#if __GNUC__
#define NOINLINEASM
#endif

DWORD mod_CalcHash(const char *szStr)
{
#if defined _M_IX86 && !defined _NUMEGA_BC_FINALCHECK && !defined NOINLINEASM
	__asm {		   //this breaks if szStr is empty
		xor  edx, edx
			xor  eax, eax
			mov  esi, szStr
			mov  al, [esi]
			xor  cl, cl
lph_top :	 //only 4 of 9 instructions in here don't use AL, so optimal pipe use is impossible
		xor  edx, eax
			inc  esi
			xor  eax, eax
			and  cl, 31
			mov  al, [esi]
			add  cl, 5
			test al, al
			rol  eax, cl		 //rol is u-pipe only, but pairable
			//rol doesn't touch z-flag
			jnz  lph_top  //5 clock tick loop. not bad.

			xor  eax, edx
	}
#else
	DWORD hash = 0;
	int i;
	int shift = 0;
	for (i = 0; szStr[i]; i++) {
		hash ^= szStr[i] << shift;
		if (shift > 24) hash ^= (szStr[i] >> (32 - shift)) & 0x7F;
		shift = (shift + 5) & 0x1F;
	}
	return hash;
#endif
}

int AddModernMaskToList(MODERNMASK *mm, LISTMODERNMASK * mmTemplateList)
{
	if (!mmTemplateList || !mm) return -1;
	mmTemplateList->pl_Masks = (MODERNMASK *)mir_realloc(mmTemplateList->pl_Masks, sizeof(MODERNMASK)*(mmTemplateList->dwMaskCnt + 1));
	memmove(&(mmTemplateList->pl_Masks[mmTemplateList->dwMaskCnt]), mm, sizeof(MODERNMASK));
	mmTemplateList->dwMaskCnt++;
	return mmTemplateList->dwMaskCnt - 1;
}

int ClearMaskList(LISTMODERNMASK * mmTemplateList)
{
	if (!mmTemplateList) return -1;
	if (!mmTemplateList->pl_Masks) return -1;
	for (int i = 0; i < (int)mmTemplateList->dwMaskCnt; i++)
		SkinSelector_DeleteMask(&(mmTemplateList->pl_Masks[i]));
	mir_free_and_nil(mmTemplateList->pl_Masks);
	mmTemplateList->dwMaskCnt = 0;
	return 0;
}

int DeleteMaskByItID(DWORD mID, LISTMODERNMASK *mmTemplateList)
{
	if (!mmTemplateList) return -1;
	if (mID >= mmTemplateList->dwMaskCnt) return -1;
	if (mmTemplateList->dwMaskCnt == 1) {
		SkinSelector_DeleteMask(&(mmTemplateList->pl_Masks[0]));
		mir_free_and_nil(mmTemplateList->pl_Masks);
		mmTemplateList->pl_Masks = NULL;
		mmTemplateList->dwMaskCnt--;
	}
	else {
		SkinSelector_DeleteMask(&(mmTemplateList->pl_Masks[mID]));
		MODERNMASK *newAlocation = (MODERNMASK *)mir_alloc(sizeof(MODERNMASK)*mmTemplateList->dwMaskCnt - 1);
		memcpy(newAlocation, mmTemplateList->pl_Masks, sizeof(MODERNMASK)*(mID + 1));
		for (DWORD i = mID; i < mmTemplateList->dwMaskCnt - 1; i++) {
			newAlocation[i] = mmTemplateList->pl_Masks[i + 1];
			newAlocation[i].dwMaskId = i;
		}
		mir_free_and_nil(mmTemplateList->pl_Masks);
		mmTemplateList->pl_Masks = newAlocation;
		mmTemplateList->dwMaskCnt--;
	}
	return mmTemplateList->dwMaskCnt;
}


int ExchangeMasksByID(DWORD mID1, DWORD mID2, LISTMODERNMASK * mmTemplateList)
{
	if (!mmTemplateList) return 0;
	if (mID1 >= mmTemplateList->dwMaskCnt) return 0;
	if (mID2 >= mmTemplateList->dwMaskCnt) return 0;
	if (mID1 == mID2) return 0;
	{
		MODERNMASK mm;
		mm = mmTemplateList->pl_Masks[mID1];
		mmTemplateList->pl_Masks[mID1] = mmTemplateList->pl_Masks[mID2];
		mmTemplateList->pl_Masks[mID2] = mm;
	}
	return 1;
}

int SortMaskList(LISTMODERNMASK * mmList)
{
	DWORD pos = 1;
	if (mmList->dwMaskCnt < 2) return 0;
	do {
		if (mmList->pl_Masks[pos].dwMaskId < mmList->pl_Masks[pos - 1].dwMaskId) {
			ExchangeMasksByID(pos, pos - 1, mmList);
			pos--;
			if (pos < 1)
				pos = 1;
		}
		else pos++;
	}
		while (pos < mmList->dwMaskCnt);

	return 1;
}

enum
{
	EXCEPTION_EQUAL,
	EXCEPTION_NOT_EQUAL = 1,
	EXCEPTION_WILD = 2,
};
static BOOL _GetParamValue(char * szText, unsigned int& start, unsigned int length, char* &param, unsigned int& paramlen, char* &value, unsigned int& valuelen, int& except)
{
	char * curChar = szText + start;
	char * lastChar = szText + length;

	enum { STATE_PARAM, STATE_VALUE };
	int state = STATE_PARAM;
	if (start >= length) return FALSE;

	paramlen = 0;
	valuelen = 0;
	value = NULL;
	param = NULL;

	except = EXCEPTION_EQUAL;
	param = curChar;


	BOOL exitLoop = false;
	while (!exitLoop) {
		switch (*curChar) {
		case '^':
			if (state == STATE_VALUE) break;
			except |= EXCEPTION_NOT_EQUAL;
			exitLoop = TRUE;
			// fall trough
		case '=':
			if (state == STATE_VALUE) break;
			// search value end
			paramlen = curChar - param;
			exitLoop = TRUE;
			break;

		case ',':
		default:
			if (*curChar != ',' && curChar < lastChar) break;
			if (state == STATE_PARAM) {
				// no parameter name only value
				value = param;
				param = NULL;
				paramlen = 0;
				state = STATE_VALUE;
			}
			exitLoop = TRUE;
			break;
		case '*': case '?':
			if (state == STATE_PARAM) break;
			except |= EXCEPTION_WILD;
			break;
		}
		if (exitLoop) {
			if (state == STATE_PARAM) {
				paramlen = curChar - param;
				state = STATE_VALUE;
				curChar++; //skip Sign
				value = curChar;
				exitLoop = FALSE;
			}
			else if (state == STATE_VALUE) {
				valuelen = curChar - value;
			}
		}
		else {
			curChar++;
		}
	}


	start = curChar - szText + 1;
	// skip spaces
	if (value && valuelen) {
		while (*value == ' ' || *value == '\t') {
			value++;
			valuelen--;
		}
		while (*(value + valuelen - 1) == ' ' || *(value + valuelen - 1) == '\t')
			valuelen--;
	}

	if (param && paramlen) {
		while (*param == ' ' || *param == '\t') {
			param++;
			paramlen--;
		}
		while (*(param + paramlen - 1) == ' ' || *(param + paramlen - 1) == '\t')
			paramlen--;
	}

	return (value || param);
}

int ParseToModernMask(MODERNMASK *mm, char *szText)
{
	if (!mm || !szText) return -1;

	unsigned int textLen = (unsigned)mir_strlen(szText);
	BYTE curParam = 0;

	MASKPARAM param = { 0 };

	unsigned int startPos = 0;
	char * pszParam;
	char * pszValue;
	unsigned int paramlen;
	unsigned int valuelen;
	int except;

	while (_GetParamValue(szText, startPos, textLen, pszParam, paramlen, pszValue, valuelen, except)) {
		if (except & EXCEPTION_NOT_EQUAL)
			param.bMaskParamFlag = MPF_NOT_EQUAL;
		else
			param.bMaskParamFlag = MPF_EQUAL;

		//Get param name
		if (pszParam && paramlen) {
			param.szName = strdupn(pszParam, paramlen);
			param.dwId = mod_CalcHash(param.szName);
		}
		else //ParamName = 'Module'
		{
			param.szName = _strdup("Module");
			param.dwId = mod_CalcHash(param.szName);
		}


		param.szValue = strdupn(pszValue, valuelen);

		if (!(except & EXCEPTION_WILD)) {
			param.dwValueHash = mod_CalcHash(param.szValue);
			param.bMaskParamFlag |= MPF_HASHED;
		}
		if (curParam >= mm->dwParamCnt) {
			mm->pl_Params = (MASKPARAM*)realloc(mm->pl_Params, (mm->dwParamCnt + 1)*sizeof(MASKPARAM));
			mm->dwParamCnt++;
		}
		memmove(&(mm->pl_Params[curParam]), &param, sizeof(MASKPARAM));
		curParam++;
		memset(&param, 0, sizeof(MASKPARAM));
	}
	return 0;
};

BOOL CompareModernMask(MODERNMASK *mmValue, MODERNMASK *mmTemplate)
{
	//TODO
	BOOL res = TRUE;
	BYTE pVal = 0, pTemp = 0;
	while (pTemp < mmTemplate->dwParamCnt && pVal < mmValue->dwParamCnt) {
		// find pTemp parameter in mValue
		DWORD vh, ph;
		BOOL finded = 0;
		MASKPARAM p = mmTemplate->pl_Params[pTemp];
		ph = p.dwId;
		vh = p.dwValueHash;
		pVal = 0;
		if (p.bMaskParamFlag& MPF_HASHED)  //compare by hash
			while (pVal < mmValue->dwParamCnt && mmValue->pl_Params[pVal].bMaskParamFlag != 0) {
				if (mmValue->pl_Params[pVal].dwId == ph) {
					if (mmValue->pl_Params[pVal].dwValueHash == vh) { finded = 1; break; }
					else { finded = 0; break; }
				}
				pVal++;
			}
		else
			while (mmValue->pl_Params[pVal].bMaskParamFlag != 0) {
				if (mmValue->pl_Params[pVal].dwId == ph) {
					if (wildcmp(mmValue->pl_Params[pVal].szValue, p.szValue)) { finded = 1; break; }
					else { finded = 0; break; }
				}
				pVal++;
			}
		if (!((finded && !(p.bMaskParamFlag&MPF_DIFF)) || (!finded && (p.bMaskParamFlag&MPF_DIFF)))) {
			res = FALSE; break;
		}
		pTemp++;
	}
	return res;
};

BOOL CompareStrWithModernMask(char *szValue, MODERNMASK *mmTemplate)
{
	MODERNMASK mmValue = { 0 };
	if (!ParseToModernMask(&mmValue, szValue)) {
		BOOL res = CompareModernMask(&mmValue, mmTemplate);
		SkinSelector_DeleteMask(&mmValue);
		return res;
	}
	else return 0;
};

// AddingMask
int AddStrModernMaskToList(DWORD maskID, char *szStr, char *objectName, LISTMODERNMASK *mmTemplateList)
{
	if (!szStr || !mmTemplateList) return -1;

	MODERNMASK mm = { 0 };
	if (ParseToModernMask(&mm, szStr)) return -1;

	mm.bObjectFound = FALSE;
	mm.szObjectName = mir_strdup(objectName);
	//mm.pObject = (void*) ske_FindObjectByName(objectName, OT_ANY, (SKINOBJECTSLIST*) pObjectList);
	mm.dwMaskId = maskID;
	return AddModernMaskToList(&mm, mmTemplateList);
}

SKINOBJECTDESCRIPTOR *skin_FindObjectByMask(MODERNMASK *mm, LISTMODERNMASK *mmTemplateList)
{
	for (DWORD i = 0;i < mmTemplateList->dwMaskCnt;i++)
		if (CompareModernMask(mm, &(mmTemplateList->pl_Masks[i])))
			return (SKINOBJECTDESCRIPTOR*)mmTemplateList->pl_Masks[i].pObject;

	return NULL;
}

SKINOBJECTDESCRIPTOR *skin_FindObjectByRequest(char * szValue, LISTMODERNMASK *mmTemplateList)
{
	MODERNMASK mm = { 0 };
	if (!mmTemplateList)
		if (g_SkinObjectList.pMaskList)
			mmTemplateList = g_SkinObjectList.pMaskList;
		else
			return NULL;

	if (!mmTemplateList) return NULL;
	ParseToModernMask(&mm, szValue);
	SKINOBJECTDESCRIPTOR *res = skin_FindObjectByMask(&mm, mmTemplateList);
	SkinSelector_DeleteMask(&mm);
	return res;
}

TCHAR* GetParamNT(char *string, TCHAR *buf, int buflen, BYTE paramN, char Delim, BOOL SkipSpaces)
{
	char *ansibuf = (char*)mir_alloc(buflen / sizeof(TCHAR));
	GetParamN(string, ansibuf, buflen / sizeof(TCHAR), paramN, Delim, SkipSpaces);
	MultiByteToWideChar(CP_UTF8, 0, ansibuf, -1, buf, buflen);
	mir_free(ansibuf);
	return buf;
}

WCHAR* GetParamN(WCHAR *string, WCHAR *buf, int buflen, BYTE paramN, WCHAR Delim, BOOL SkipSpaces)
{
	size_t i = 0, start = 0, CurentCount = 0, len;
	while (i < mir_tstrlen(string)) {
		if (string[i] == Delim) {
			if (CurentCount == paramN) break;
			start = i + 1;
			CurentCount++;
		}
		i++;
	}
	if (CurentCount == paramN) {
		if (SkipSpaces) { //remove spaces
			while (string[start] == ' ' && (int)start < mir_tstrlen(string))
				start++;
			while (i>1 && string[i - 1] == ' ' && i > (int)start)
				i--;
		}
		len = ((int)(i - start) < buflen) ? i - start : buflen;
		_tcsncpy(buf, string + start, len);
		buf[len] = '\0';
	}
	else buf[0] = '\0';
	return buf;
}

char * GetParamN(char * string, char * buf, int buflen, BYTE paramN, char Delim, BOOL SkipSpaces)
{
	size_t i = 0, start = 0, CurentCount = 0, len;
	while (i < mir_strlen(string)) {
		if (string[i] == Delim) {
			if (CurentCount == paramN) break;
			start = i + 1;
			CurentCount++;
		}
		i++;
	}
	if (CurentCount == paramN) {
		if (SkipSpaces) { //remove spaces
			while (string[start] == ' ' && (int)start < mir_strlen(string))
				start++;
			while (i>1 && string[i - 1] == ' ' && i > (int)start)
				i--;
		}
		len = ((int)(i - start) < buflen) ? i - start : buflen;
		strncpy(buf, string + start, len);
		buf[len] = '\0';
	}
	else buf[0] = '\0';
	return buf;
}

//Parse DB string and add buttons
int RegisterButtonByParce(char * ObjectName, char * Params)
{
	char buf[255];
	int res;
	GetParamN(Params, buf, SIZEOF(buf), 0, ',', 0);
	// if (boolstrcmpi("Push",buf)
	{   //Push type
		char buf2[20] = { 0 };
		char pServiceName[255] = { 0 };
		char pStatusServiceName[255] = { 0 };
		int Left, Top, Right, Bottom;
		int MinWidth, MinHeight;
		char TL[9] = { 0 };
		TCHAR Hint[250] = { 0 };
		char Section[250] = { 0 };
		char Type[250] = { 0 };

		DWORD alingnto;
		int a = ((int)mir_bool_strcmpi(buf, "Switch")) * 2;

		GetParamN(Params, pServiceName, SIZEOF(pServiceName), 1, ',', 0);
		// if (a) GetParamN(Params,pStatusServiceName, sizeof(pStatusServiceName),a+1,',',0);
		Left = atoi(GetParamN(Params, buf2, SIZEOF(buf2), a + 2, ',', 0));
		Top = atoi(GetParamN(Params, buf2, SIZEOF(buf2), a + 3, ',', 0));
		Right = atoi(GetParamN(Params, buf2, SIZEOF(buf2), a + 4, ',', 0));
		Bottom = atoi(GetParamN(Params, buf2, SIZEOF(buf2), a + 5, ',', 0));
		GetParamN(Params, TL, SIZEOF(TL), a + 6, ',', 0);

		MinWidth = atoi(GetParamN(Params, buf2, SIZEOF(buf2), a + 7, ',', 0));
		MinHeight = atoi(GetParamN(Params, buf2, SIZEOF(buf2), a + 8, ',', 0));
		GetParamNT(Params, Hint, SIZEOF(Hint), a + 9, ',', 0);
		if (a) {
			GetParamN(Params, Section, SIZEOF(Section), 2, ',', 0);
			GetParamN(Params, Type, SIZEOF(Type), 3, ',', 0);
		}
		alingnto = ((TL[0] == 'R') ? SBF_ALIGN_TL_RIGHT : 0)
			+ ((TL[0] == 'C') ? SBF_ALIGN_TL_HCENTER : 0)
			+ ((TL[1] == 'B') ? SBF_ALIGN_TL_BOTTOM : 0)
			+ ((TL[1] == 'C') ? SBF_ALIGN_TL_VCENTER : 0)
			+ ((TL[2] == 'R') ? SBF_ALIGN_BR_RIGHT : 0)
			+ ((TL[2] == 'C') ? SBF_ALIGN_BR_HCENTER : 0)
			+ ((TL[3] == 'B') ? SBF_ALIGN_BR_BOTTOM : 0)
			+ ((TL[3] == 'C') ? SBF_ALIGN_BR_VCENTER : 0)
			+ ((TL[4] == 'I') ? SBF_CALL_ON_PRESS : 0);
		if (a) res = ModernSkinButton_AddButton(pcli->hwndContactList, ObjectName + 1, pServiceName, pStatusServiceName, "\0", Left, Top, Right, Bottom, alingnto, TranslateTS(Hint), Section, Type, MinWidth, MinHeight);
		else res = ModernSkinButton_AddButton(pcli->hwndContactList, ObjectName + 1, pServiceName, pStatusServiceName, "\0", Left, Top, Right, Bottom, alingnto, TranslateTS(Hint), NULL, NULL, MinWidth, MinHeight);
	}
	return res;
}

//Parse DB string and add object
// Params is:
// Glyph,None
// Glyph,Solid, < ColorR>, < ColorG>, < ColorB>, < Alpha>
// Glyph,Image,Filename,(TileBoth|TileVert|TileHor|StretchBoth), < MarginLeft>, < MarginTop>, < MarginRight>, < MarginBottom>, < Alpha>
int RegisterObjectByParce(char * ObjectName, char * Params)
{
	if (!ObjectName || !Params) return 0;
	{
		SKINOBJECTDESCRIPTOR obj = { 0 };
		char buf[250];
		obj.szObjectID = mir_strdup(ObjectName);
		GetParamN(Params, buf, SIZEOF(buf), 0, ',', 0);
		if (mir_bool_strcmpi(buf, "Glyph"))
			obj.bType = OT_GLYPHOBJECT;
		else if (mir_bool_strcmpi(buf, "Font"))
			obj.bType = OT_FONTOBJECT;

		switch (obj.bType) {
		case OT_GLYPHOBJECT:
		{
			GLYPHOBJECT gl = { 0 };
			GetParamN(Params, buf, SIZEOF(buf), 1, ',', 0);
			if (mir_bool_strcmpi(buf, "Solid")) {
				//Solid
				gl.Style = ST_BRUSH;
				int r = atoi(GetParamN(Params, buf, SIZEOF(buf), 2, ',', 0));
				int g = atoi(GetParamN(Params, buf, SIZEOF(buf), 3, ',', 0));
				int b = atoi(GetParamN(Params, buf, SIZEOF(buf), 4, ',', 0));
				gl.dwAlpha = atoi(GetParamN(Params, buf, SIZEOF(buf), 5, ',', 0));
				gl.dwColor = RGB(r, g, b);
			}
			else if (mir_bool_strcmpi(buf, "Image")) {
				//Image
				gl.Style = ST_IMAGE;
				gl.szFileName = mir_strdup(GetParamN(Params, buf, SIZEOF(buf), 2, ',', 0));
				gl.dwLeft = atoi(GetParamN(Params, buf, SIZEOF(buf), 4, ',', 0));
				gl.dwTop = atoi(GetParamN(Params, buf, SIZEOF(buf), 5, ',', 0));
				gl.dwRight = atoi(GetParamN(Params, buf, SIZEOF(buf), 6, ',', 0));
				gl.dwBottom = atoi(GetParamN(Params, buf, SIZEOF(buf), 7, ',', 0));
				gl.dwAlpha = atoi(GetParamN(Params, buf, SIZEOF(buf), 8, ',', 0));
				GetParamN(Params, buf, SIZEOF(buf), 3, ',', 0);
				if (mir_bool_strcmpi(buf, "TileBoth")) gl.FitMode = FM_TILE_BOTH;
				else if (mir_bool_strcmpi(buf, "TileVert")) gl.FitMode = FM_TILE_VERT;
				else if (mir_bool_strcmpi(buf, "TileHorz")) gl.FitMode = FM_TILE_HORZ;
				else gl.FitMode = 0;
			}
			else if (mir_bool_strcmpi(buf, "Fragment")) {
				//Image
				gl.Style = ST_FRAGMENT;
				gl.szFileName = mir_strdup(GetParamN(Params, buf, SIZEOF(buf), 2, ',', 0));

				gl.clipArea.x = atoi(GetParamN(Params, buf, SIZEOF(buf), 3, ',', 0));
				gl.clipArea.y = atoi(GetParamN(Params, buf, SIZEOF(buf), 4, ',', 0));
				gl.szclipArea.cx = atoi(GetParamN(Params, buf, SIZEOF(buf), 5, ',', 0));
				gl.szclipArea.cy = atoi(GetParamN(Params, buf, SIZEOF(buf), 6, ',', 0));

				gl.dwLeft = atoi(GetParamN(Params, buf, SIZEOF(buf), 8, ',', 0));
				gl.dwTop = atoi(GetParamN(Params, buf, SIZEOF(buf), 9, ',', 0));
				gl.dwRight = atoi(GetParamN(Params, buf, SIZEOF(buf), 10, ',', 0));
				gl.dwBottom = atoi(GetParamN(Params, buf, SIZEOF(buf), 11, ',', 0));
				gl.dwAlpha = atoi(GetParamN(Params, buf, SIZEOF(buf), 12, ',', 0));
				GetParamN(Params, buf, SIZEOF(buf), 7, ',', 0);
				if (mir_bool_strcmpi(buf, "TileBoth")) gl.FitMode = FM_TILE_BOTH;
				else if (mir_bool_strcmpi(buf, "TileVert")) gl.FitMode = FM_TILE_VERT;
				else if (mir_bool_strcmpi(buf, "TileHorz")) gl.FitMode = FM_TILE_HORZ;
				else gl.FitMode = 0;
			}
			else {
				//None
				gl.Style = ST_SKIP;
			}
			obj.Data = &gl;
			int res = ske_AddDescriptorToSkinObjectList(&obj, NULL);
			mir_free_and_nil(obj.szObjectID);
			mir_free_and_nil(gl.szFileName);
			return res;
		}
		break;
		}
	}
	return 0;
}


int SkinDrawGlyphMask(HDC hdc, RECT *rcSize, RECT *rcClip, MODERNMASK *ModernMask)
{
	if (!ModernMask) return 0;

	SKINDRAWREQUEST rq;
	rq.hDC = hdc;
	rq.rcDestRect = *rcSize;
	rq.rcClipRect = *rcClip;
	strncpy(rq.szObjectID, "Masked draw", SIZEOF(rq.szObjectID)-1);
	return ske_Service_DrawGlyph((WPARAM)&rq, (LPARAM)ModernMask);
}
