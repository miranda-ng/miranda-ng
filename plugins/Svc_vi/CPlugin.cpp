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

#include "CPlugin.h"
//#include "AggressiveOptimize.h"

#include "common.h"

const int cPLUGIN_UUID_MARK = 4;
TCHAR PLUGIN_UUID_MARK[cPLUGIN_UUID_MARK];

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

	TCHAR aux[128];
	mir_sntprintf(aux, SIZEOF(aux), _T("%d.%d.%d.%d"),	(eVersion>>24)&0xFF, (eVersion>>16)&0xFF, (eVersion>>8)&0xFF, (eVersion)&0xFF);
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

CPlugin::~CPlugin() {
	//Debug information
//	char str[64]; wsprintf(str, "~CPlugin(): %s", lpzFileName.c_str());
//	MB(str);
	//
	lpzFileName.~basic_string();
	lpzShortName.~basic_string();
	lpzVersion.~basic_string();
	lpzUnicodeInfo.~basic_string();
	lpzTimestamp.~basic_string();
	lpzLinkedModules.~basic_string();
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
	else lpzInformations = (IsUUIDNull(pluginID)) ? _T(" ") : PLUGIN_UUID_MARK;

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