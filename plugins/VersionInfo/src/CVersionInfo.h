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

#ifndef PF_NX_ENABLED
	#define PF_NX_ENABLED 12
#endif

class CVersionInfo
{
	private:
		//Informations related to Miranda: main informations.
		std::tstring lpzMirandaVersion;
		std::tstring lpzMirandaPath;
		std::tstring lpzProfilePath;
		std::tstring lpzProfileSize;
		std::tstring lpzProfileCreationDate;
		std::tstring lpzNightly;
		std::tstring lpzUnicodeBuild;
		std::tstring lpzCPUName;
		std::tstring lpzCPUIdentifier;
		std::tstring lpzBuildTime;
		std::tstring lpzShell;
		std::tstring lpzIEVersion;
		std::tstring lpzAdministratorPrivileges;
		std::tstring lpzOSLanguages;
		std::tstring lpzLangpackInfo;
		std::tstring lpzLangpackModifiedDate;
		//Informations related to plugins
		std::list<CPlugin> listActivePlugins;
		std::list<CPlugin> listInactivePlugins;
		std::list<CPlugin> listUnloadablePlugins;
		//OS and hardware informations.
		std::tstring lpzOSName;
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
		//Configuration
		
		bool GetLinkedModulesInfo(TCHAR *moduleName, std::tstring &linkedModules);

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
		void PrintInformationsToFile(const TCHAR *info);
		void PrintInformationsToDialogBox();
		void PrintInformationsToMessageBox();
		void PrintInformationsToOutputDebugString();
		void PrintInformationsToClipboard(bool);
		
		std::tstring GetListAsString(std::list<CPlugin>&, DWORD flags, int beautify);
		std::tstring GetInformationsAsString(int bDisableForumStyle = 0);
		void BeautifyReport(int, LPCTSTR, LPCTSTR, std::tstring &);
		void AddInfoHeader(int, int, int, std::tstring &);
		void AddInfoFooter(int, int, int, std::tstring &);
};

#endif