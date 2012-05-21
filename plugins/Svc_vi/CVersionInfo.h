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

#ifndef CVERSIONINFO_H
#define CVERSIONINFO_H

//#define STRICT
#define WIN32_LEAN_AND_MEAN
//#include "AggressiveOptimize.h"

#define _CRT_SECURE_NO_DEPRECATE

#ifndef PF_NX_ENABLED
	#define PF_NX_ENABLED 12
#endif

#include <list>
#include <string>
//using namespace std;

#include "CPlugin.h"

class CVersionInfo
{
	private:
		//Informations related to Miranda: main informations.
		std::string lpzMirandaVersion;
		std::string lpzMirandaPath;
		std::string lpzProfilePath;
		std::string lpzProfileSize;
		std::string lpzProfileCreationDate;
		std::string lpzNightly;
		std::string lpzUnicodeBuild;
		std::string lpzCPUName;
		std::string lpzCPUIdentifier;
		std::string lpzBuildTime;
		std::string lpzShell;
		std::string lpzIEVersion;
		std::string lpzAdministratorPrivileges;
		std::string lpzOSLanguages;
		std::string lpzLangpackInfo;
		std::string lpzLangpackModifiedDate;
		//Informations related to plugins
		std::list<CPlugin> listActivePlugins;
		std::list<CPlugin> listInactivePlugins;
		std::list<CPlugin> listUnloadablePlugins;
		//OS and hardware informations.
		std::string lpzOSVersion;
		std::string lpzOSName;
		unsigned int luiProcessors;
		unsigned int luiRAM;
		unsigned int luiFreeDiskSpace;
		int bDEPEnabled;
		BOOL bIsWOW64;
		//Additional Miranda informations.
		unsigned int luiContacts;
		unsigned int luiEvents;
		unsigned int luiUnreadEvents;
		unsigned int luiDBSize;
		bool bExpertSettingsOn;
		//Configuration
		
		bool GetLinkedModulesInfo(char *moduleName, std::string &linkedModules);

	public:
		//Constructor/Destructor
		CVersionInfo();
		~CVersionInfo();
		void Initialize();
		//Miranda informations
		bool GetMirandaVersion();
		bool GetProfileSettings();
		bool GetOSLanguages();
		bool GetLangpackInfo();
		bool GetPluginLists();
		bool GetEventCount(); //TODO
		//OSInformations
		bool GetOSVersion();
		bool GetHWSettings();
		//Plugins
		bool AddPlugin(CPlugin&, std::list<CPlugin>&);
		//Prints
		
		void PrintInformationsToFile();
		void PrintInformationsToFile(const char *info);
		void PrintInformationsToDialogBox();
		void PrintInformationsToMessageBox();
		void PrintInformationsToOutputDebugString();
		void PrintInformationsToClipboard(bool);
		void UploadToSite(char *text = NULL);
		
		std::string GetListAsString(std::list<CPlugin>&, DWORD flags, int beautify);
		std::string GetInformationsAsString(int bDisableForumStyle = 0);
		void BeautifyReport(int, char *, char *, std::string &);
		void AddInfoHeader(int, int, int, std::string &);
		void AddInfoFooter(int, int, int, std::string &);
};

#endif