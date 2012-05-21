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
char PLUGIN_UUID_MARK[cPLUGIN_UUID_MARK];

#define PLUGIN_UNCERTAIN_MARK "?"

#define RJUST 70

CPlugin::CPlugin() {
	lpzFileName = "";
	lpzShortName = "";
	lpzVersion = "";
	lpzTimestamp = "";
	lpzLinkedModules = "";
	pluginID = UUID_NULL;
};

CPlugin::CPlugin(std::string eFileName, std::string eShortName, MUUID pluginID, std::string eUnicodeInfo, DWORD eVersion, std::string eTimestamp, std::string eLinkedModules) {
	lpzFileName = std::string(eFileName);
	lpzShortName = std::string(eShortName);
	lpzUnicodeInfo = std::string(eUnicodeInfo);
	lpzTimestamp = std::string(eTimestamp);
	lpzLinkedModules = std::string(eLinkedModules);

	char aux[128];
	wsprintf(aux,"%d.%d.%d.%d",	(eVersion>>24)&0xFF, (eVersion>>16)&0xFF, (eVersion>>8)&0xFF, (eVersion)&0xFF);
	lpzVersion = std::string(aux);
	
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

void CPlugin::SetErrorMessage(std::string error)
{
	lpzLinkedModules = error;
}

bool CPlugin::operator<(CPlugin &anotherPlugin) {
	std::string anotherFileName = anotherPlugin.getFileName();

	char szThis[MAX_PATH]; lstrcpy(szThis, lpzFileName.c_str());
	char szThat[MAX_PATH]; lstrcpy(szThat, anotherFileName.c_str());

	if (lstrcmpi(szThis, szThat) < 0)
		return TRUE;
	else
		return FALSE;
}

bool CPlugin::operator>(CPlugin &anotherPlugin) {
	return !((*this) < anotherPlugin);
}
bool CPlugin::operator==(CPlugin &anotherPlugin) {
	return !((*this) < anotherPlugin || (*this) > anotherPlugin);
}

std::string CPlugin::getFileName() {
	return this->lpzFileName;
}

std::string CPlugin::getInformations(DWORD flags, char *szHighlightHeader, char *szHighlightFooter) {
//	std::string lpzInformations = std::string(lpzFileName + "\t\t" + lpzVersion + "\r\n");
//	std::string lpzInformations = std::string(lpzFileName + " - " + lpzShortName + " [" + lpzTimestamp + "], version: " + lpzVersion +"\r\n");
//	std::string lpzInformations = std::string(lpzShortName + " [" + lpzFileName + " · " + lpzTimestamp + "] - version " + lpzVersion + "\r\n");
//	std::string lpzInformations = std::string(lpzFileName + " [" + lpzShortName + " · " + lpzTimestamp + "] - version: " + lpzVersion +"\r\n");	
	std::string lpzInformations;
	if (flags & VISF_SHOWUUID)
	{
		char aux[128];
		UUIDToString(pluginID, aux, sizeof(aux));
		lpzInformations = std::string(aux);
	}
	else{
		lpzInformations = (IsUUIDNull(pluginID)) ? " " : PLUGIN_UUID_MARK;
	}
	lpzInformations += std::string(" " + lpzFileName + " v." + szHighlightHeader + lpzVersion + szHighlightFooter + " [" + lpzTimestamp + "] - " + lpzShortName);
	if (lpzUnicodeInfo.size() > 0)
	{
		char *lwr = _strlwr(_strdup(lpzShortName.c_str()));
		if ((strstr(lwr, "unicode") == NULL) && (strstr(lwr, "2in1") == NULL))
		{
			lpzInformations.append(" |" + lpzUnicodeInfo + "|");
		}
		free(lwr);
	}
	//lpzInformations.append("\t");
	//lpzInformations.append(lpzPluginID);
	lpzInformations.append("\r\n");
	
	if (lpzLinkedModules.size() > 0)
		{
			lpzInformations.append(lpzLinkedModules);
			lpzInformations.append("\r\n");
		}
//	std::string lpzInformations = std::string(lpzFileName + " - " + lpzShortName + " [" + lpzTimestamp + " · " + lpzVersion +"]\r\n");
	return lpzInformations;
};