/*
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

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

#include "common.h"

#define PLUGIN_UNCERTAIN_MARK "?"

#define RJUST 70

CPlugin::CPlugin() {
	pluginID = UUID_NULL;
}

CPlugin::CPlugin(LPCTSTR eFileName, LPCTSTR eShortName, MUUID pluginID, LPCTSTR eUnicodeInfo, DWORD eVersion, LPCTSTR eTimestamp, LPCTSTR eLinkedModules) {
	lpzFileName = eFileName;
	lpzShortName = eShortName;
	lpzUnicodeInfo = eUnicodeInfo;
	lpzTimestamp = eTimestamp;
	lpzLinkedModules = eLinkedModules;

	int v1, v2, v3, v4;

	DWORD unused, verInfoSize = GetFileVersionInfoSize(eFileName, &unused);
	if (verInfoSize != 0) {
		UINT blockSize;
		VS_FIXEDFILEINFO* fi;
		void* pVerInfo = mir_alloc(verInfoSize);
		GetFileVersionInfo(eFileName, 0, verInfoSize, pVerInfo);
		VerQueryValue(pVerInfo, _T("\\"), (LPVOID*)&fi, &blockSize);
		v1 = HIWORD(fi->dwProductVersionMS), v2 = LOWORD(fi->dwProductVersionMS),
		v3 = HIWORD(fi->dwProductVersionLS), v4 = LOWORD(fi->dwProductVersionLS);
		mir_free(pVerInfo);
	}
	else {
		DWORD ver = eVersion;
		v1 = HIBYTE(HIWORD(ver)), v2 = LOBYTE(HIWORD(ver)), v3 = HIBYTE(LOWORD(ver)), v4 = LOBYTE(LOWORD(ver));
	}

	TCHAR aux[128];
	mir_sntprintf(aux, SIZEOF(aux), _T("%d.%d.%d.%d"),	v1, v2, v3, v4);
	lpzVersion = aux;
	
	this->pluginID = pluginID;
};

CPlugin::CPlugin(const CPlugin& other) {
	lpzFileName  = other.lpzFileName;
	lpzShortName = other.lpzShortName;
	lpzUnicodeInfo = other.lpzUnicodeInfo;
	lpzVersion   = other.lpzVersion;
	lpzTimestamp = other.lpzTimestamp;
	lpzLinkedModules = other.lpzLinkedModules;
	pluginID = other.pluginID;
}

CPlugin::~CPlugin()
{
}

void CPlugin::SetErrorMessage(LPCTSTR error)
{
	lpzLinkedModules = error;
}

bool CPlugin::operator<(CPlugin &anotherPlugin)
{
	std::tstring anotherFileName = anotherPlugin.getFileName();

	TCHAR szThis[MAX_PATH]; lstrcpy(szThis, lpzFileName.c_str());
	TCHAR szThat[MAX_PATH]; lstrcpy(szThat, anotherFileName.c_str());

	if (lstrcmpi(szThis, szThat) < 0)
		return TRUE;
	else
		return FALSE;
}

bool CPlugin::operator>(CPlugin &anotherPlugin)
{
	return !((*this) < anotherPlugin);
}
bool CPlugin::operator==(CPlugin &anotherPlugin)
{
	return !((*this) < anotherPlugin || (*this) > anotherPlugin);
}

std::tstring CPlugin::getFileName()
{
	return this->lpzFileName;
}

std::tstring CPlugin::getInformations(DWORD flags, TCHAR *szHighlightHeader, TCHAR *szHighlightFooter)
{
	std::tstring lpzInformations;
	if (flags & VISF_SHOWUUID)
	{
		TCHAR aux[128];
		UUIDToString(pluginID, aux, SIZEOF(aux));
		lpzInformations = aux;
	}
	else lpzInformations = _T(" ");

	lpzInformations += std::tstring(_T(" ") + lpzFileName + _T(" v.") + szHighlightHeader + lpzVersion + szHighlightFooter + _T(" [") + lpzTimestamp + _T("] - ") + lpzShortName);
	if (lpzUnicodeInfo.size() > 0)
	{
		TCHAR *lwr = _tcslwr(_tcsdup(lpzShortName.c_str()));
		if ( !_tcsstr(lwr, _T("unicode")) && !_tcsstr(lwr, _T("2in1")))
			lpzInformations.append( _T(" |") + lpzUnicodeInfo + _T("|"));

		free(lwr);
	}
	//lpzInformations.append("\t");
	//lpzInformations.append(lpzPluginID);
	lpzInformations.append( _T("\r\n"));
	
	if (lpzLinkedModules.size() > 0)
		{
			lpzInformations.append(lpzLinkedModules);
			lpzInformations.append( _T("\r\n"));
		}
//	std::string lpzInformations = std::string(lpzFileName + " - " + lpzShortName + " [" + lpzTimestamp + " · " + lpzVersion +"]\r\n");
	return lpzInformations;
};