/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "utils.h"

extern TCHAR* vertxt;
extern TCHAR* profname;
extern TCHAR* profpath;

void CreateMiniDump(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr)
{
	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = exc_ptr;
	exceptionInfo.ClientPointers = false;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
		hDumpFile, MiniDumpNormal, &exceptionInfo, NULL, NULL);
}


void WriteBBFile(CMString& buffer, bool hdr)
{
	static const TCHAR header[] = TEXT("[spoiler=VersionInfo][quote]");
	static const TCHAR footer[] = TEXT("[/quote][/spoiler]");

	buffer.Append(hdr ? header : footer);
}


void WriteUtfFile(HANDLE hDumpFile, char* bufu)
{
	DWORD bytes;

	static const unsigned char bytemark[] = { 0xEF, 0xBB, 0xBF };
	WriteFile(hDumpFile, bytemark, 3, &bytes, NULL);
	WriteFile(hDumpFile, bufu, (DWORD)strlen(bufu), &bytes, NULL);
}


BOOL CALLBACK LoadedModules64(LPCSTR, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	CMString& buffer = *(CMString*)UserContext;

	const HMODULE hModule = (HMODULE)ModuleBase;

	TCHAR path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);

	buffer.AppendFormat(TEXT("%s  %p - %p"), path, (LPVOID)ModuleBase, (LPVOID)(ModuleBase + ModuleSize));

	GetVersionInfo(hModule, buffer);

	TCHAR timebuf[30] = TEXT("");
	GetLastWriteTime(path, timebuf, 30);

	buffer.AppendFormat(TEXT(" [%s]\r\n"), timebuf);

	return TRUE;
}

struct FindData
{
	DWORD64 Offset;
	IMAGEHLP_MODULE64* pModule;
};

BOOL CALLBACK LoadedModulesFind64(LPCSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	FindData* data = (FindData*)UserContext;

	if ((DWORD)(data->Offset - ModuleBase) < ModuleSize) {
		const size_t len = SIZEOF(data->pModule->ModuleName);
		strncpy(data->pModule->ModuleName, ModuleName, len);
		data->pModule->ModuleName[len-1] = 0;

		data->pModule->BaseOfImage = ModuleBase;

		const HMODULE hModule = (HMODULE)ModuleBase;
		GetModuleFileNameA(hModule, data->pModule->LoadedImageName, SIZEOF(data->pModule->LoadedImageName));

		return FALSE;
	}
	return TRUE;
}


void GetLinkedModulesInfo(TCHAR *moduleName, CMString &buffer)
{
	HANDLE hDllFile = CreateFile(moduleName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDllFile == INVALID_HANDLE_VALUE)
		return;

	HANDLE hDllMapping = CreateFileMapping(hDllFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hDllMapping == INVALID_HANDLE_VALUE) {
		CloseHandle(hDllFile);
		return;
	}

	LPVOID dllAddr = MapViewOfFile(hDllMapping, FILE_MAP_READ, 0, 0, 0);

	static const TCHAR format[] = TEXT("    Plugin statically linked to missing module: %S\r\n");

	__try {
		PIMAGE_NT_HEADERS nthdrs = ImageNtHeader(dllAddr);

		ULONG tableSize;
		PIMAGE_IMPORT_DESCRIPTOR importData = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(dllAddr, FALSE, 
			IMAGE_DIRECTORY_ENTRY_IMPORT, &tableSize);
		if (importData) {
			while (importData->Name) {
				char* moduleName = (char*)ImageRvaToVa(nthdrs, dllAddr, importData->Name, NULL);
				if (!SearchPathA(NULL, moduleName, NULL, NULL, 0, NULL))
					buffer.AppendFormat(format, moduleName);

				importData++; //go to next record
			}
		}

		bool found = false;
		PIMAGE_EXPORT_DIRECTORY exportData = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(dllAddr, FALSE, 
			IMAGE_DIRECTORY_ENTRY_EXPORT, &tableSize);
		if (exportData) {
			ULONG* funcAddr = (ULONG*)ImageRvaToVa(nthdrs, dllAddr, exportData->AddressOfNames, NULL);
			for(unsigned i=0; i<exportData->NumberOfNames && !found; ++i) {
				char* funcName = (char*)ImageRvaToVa(nthdrs, dllAddr, funcAddr[i], NULL);
				found = strcmp(funcName, "MirandaPluginInfoEx") == 0 || strcmp(funcName, "MirandaPluginInfo") == 0; 
				if (strcmp(funcName, "DatabasePluginInfo") == 0) {
					buffer.Append(TEXT("    This dll is a Miranda database plugin, another database is active right now\r\n"));
					found = true;
				}
			}
		}
		if (!found) 
			buffer.Append(TEXT("    This dll is not a Miranda plugin and should be removed from plugins directory\r\n"));
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {}

	UnmapViewOfFile(dllAddr);
	CloseHandle(hDllMapping);
	CloseHandle(hDllFile);
}


struct ListItem
{
	ListItem() : str(), next(NULL) {}

	CMString str;
	ListItem *next;
};

static void GetPluginsString(CMString& buffer, unsigned& flags)
{
	buffer.AppendFormat(TEXT("Service Mode: %s\r\n"), servicemode ? TEXT("Yes") : TEXT("No"));

	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	LPTSTR fname = _tcsrchr(path, TEXT('\\'));
	if (fname == NULL) fname = path;
	mir_sntprintf(fname, MAX_PATH-(fname-path), TEXT("\\plugins\\*.dll"));

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	size_t count = 0, ucount = 0;

	CMString ubuffer;
	ListItem* dlllist = NULL;

	static const TCHAR format[] = TEXT("%c %s v.%s%d.%d.%d.%d%s [%s] - %S %s\r\n");

	do {
		bool loaded = false;
		mir_sntprintf(fname, MAX_PATH-(fname-path), TEXT("\\plugins\\%s"), FindFileData.cFileName);
		HMODULE hModule = GetModuleHandle(path);
		if (hModule == NULL && servicemode) {
			hModule = LoadLibrary(path);
			loaded = true;
		}
		if (hModule == NULL) {	
			if ((flags & VI_FLAG_PRNVAR) && IsPluginEnabled(FindFileData.cFileName)) {
				TCHAR timebuf[30] = TEXT("");
				GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);

				ubuffer.AppendFormat(format, TEXT(' '), FindFileData.cFileName, 
					(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""),
					0, 0, 0, 0, 
					(flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""),
					timebuf, "<unknown>", TEXT(""));

				GetLinkedModulesInfo(path, ubuffer);
				ubuffer.Append(TEXT("\r\n"));

				++ucount;
			}
			continue;
		}

		PLUGININFOEX* pi = GetMirInfo(hModule);
		if (pi != NULL) {
			TCHAR timebuf[30] = TEXT("");
			GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);

			bool ep = (size_t)pi->cbSize > sizeof(PLUGININFOEX);
			const TCHAR *unica = (ep && ((PLUGININFOEX*)pi)->flags & 1) ? TEXT("|Unicode aware|") : TEXT("");

			ListItem* lst = new ListItem;
			int v1, v2, v3, v4;

			DWORD unused, verInfoSize = GetFileVersionInfoSize(path, &unused);
			if (verInfoSize != 0) {
				UINT blockSize;
				VS_FIXEDFILEINFO* fi;
				void* pVerInfo = mir_alloc(verInfoSize);
				GetFileVersionInfo(path, 0, verInfoSize, pVerInfo);
				VerQueryValue(pVerInfo, _T("\\"), (LPVOID*)&fi, &blockSize);
				v1 = HIWORD(fi->dwProductVersionMS), v2 = LOWORD(fi->dwProductVersionMS),
				v3 = HIWORD(fi->dwProductVersionLS), v4 = LOWORD(fi->dwProductVersionLS);
				mir_free(pVerInfo);
			}
			else {
				DWORD ver = pi->version;
				v1 = HIBYTE(HIWORD(ver)), v2 = LOBYTE(HIWORD(ver)), v3 = HIBYTE(LOWORD(ver)), v4 = LOBYTE(LOWORD(ver));
			}
		
			lst->str.AppendFormat(format, ep ? TEXT('\xa4') : TEXT(' '), FindFileData.cFileName, 
				(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""),
				v1, v2, v3, v4, 
				(flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""),
				timebuf, pi->shortName ? pi->shortName : "", unica);

			ListItem* lsttmp = dlllist;
			ListItem* lsttmppv = NULL;
			while (lsttmp != NULL) {
				size_t sz = min(lsttmp->str.GetLength(), lst->str.GetLength()) - 2;
				if (lsttmp->str.CompareNoCase(lst->str) > 0)
					break;
				lsttmppv = lsttmp;
				lsttmp = lsttmp->next;
			}
			lst->next = lsttmp;
			if (lsttmppv == NULL)
				dlllist = lst;
			else
				lsttmppv->next = lst;

			if (_tcsicmp(FindFileData.cFileName, TEXT("weather.dll")) == 0)
				flags |= VI_FLAG_WEATHER;

			++count;
		}
		if (loaded) FreeLibrary(hModule);
	}
	while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);

	buffer.AppendFormat(TEXT("\r\n%sActive Plugins (%u):%s\r\n"), 
		(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""), count, (flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""));

	ListItem* lsttmp = dlllist;
	while (lsttmp != NULL) {
		buffer.Append(lsttmp->str);
		ListItem* lsttmp1 = lsttmp->next;
		delete lsttmp;
		lsttmp = lsttmp1;
	} 

	if (ucount) {
		buffer.AppendFormat(TEXT("\r\n%sUnloadable Plugins (%u):%s\r\n"), 
			(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""), ucount, (flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""));
		buffer.Append(ubuffer);
	}
}


struct ProtoCount
{
	char countse;
	char countsd;
	bool nloaded;
};

static void GetProtocolStrings(CMString& buffer)
{
	PROTOACCOUNT **accList;
	int accCount;
	int i, j;

	ProtoEnumAccounts(&accCount, &accList);

	int protoCount;
	PROTOCOLDESCRIPTOR **protoList;
	CallService(MS_PROTO_ENUMPROTOS, (WPARAM)&protoCount, (LPARAM)&protoList);

	int protoCountMy = 0;
	char** protoListMy = (char**)alloca((protoCount + accCount) * sizeof(char*));

	for (i = 0; i < protoCount; i++) {
		if (protoList[i]->type != PROTOTYPE_PROTOCOL)
			continue;
		protoListMy[protoCountMy++] = protoList[i]->szName;
	}

	for (j = 0; j < accCount; j++) {
		for (i = 0; i < protoCountMy; i++) 
			if ( !strcmp(protoListMy[i], accList[j]->szProtoName))
				break;

		if (i == protoCountMy)
			protoListMy[protoCountMy++] = accList[j]->szProtoName;
	}

	ProtoCount *protos = (ProtoCount*)alloca(sizeof(ProtoCount) * protoCountMy);
	memset(protos, 0, sizeof(ProtoCount) * protoCountMy);

	for (j = 0; j < accCount; j++) 
		for (i = 0; i < protoCountMy; i++)
			if ( !strcmp(protoListMy[i], accList[j]->szProtoName)) {
				protos[i].nloaded = accList[j]->bDynDisabled != 0;
				if ( IsAccountEnabled(accList[j]))
					++protos[i].countse;
				else
					++protos[i].countsd;
				break;
			}

	for (i = 0; i < protoCountMy; i++)
		buffer.AppendFormat(TEXT("%-24s %d - Enabled %d - Disabled  %sLoaded\r\n"), 
			(TCHAR*)_A2T(protoListMy[i]), protos[i].countse, 
			protos[i].countsd, protos[i].nloaded ? _T("Not ") : _T(""));
}


static void GetWeatherStrings(CMString& buffer, unsigned flags)
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	LPTSTR fname = _tcsrchr(path, TEXT('\\'));
	if (fname == NULL) fname = path;
	mir_sntprintf(fname, MAX_PATH-(fname-path), TEXT("\\plugins\\weather\\*.ini"));

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		mir_sntprintf(fname, MAX_PATH-(fname-path), TEXT("\\plugins\\weather\\%s"), FindFileData.cFileName);
		HANDLE hDumpFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hDumpFile != INVALID_HANDLE_VALUE) {
			char buf[8192];

			DWORD bytes = 0;
			ReadFile(hDumpFile, buf, 8190, &bytes, NULL);
			buf[bytes] = 0;

			char* ver = strstr(buf, "Version=");
			if (ver != NULL) {
				char *endid = strchr(ver, '\r');
				if (endid != NULL) *endid = 0;
				else {
					endid = strchr(ver, '\n');
					if (endid != NULL) *endid = 0;
				}
				ver += 8;
			}

			char *id = strstr(buf, "Name=");
			if (id != NULL) {
				char *endid = strchr(id, '\r');
				if (endid != NULL) *endid = 0;
				else {
					endid = strchr(id, '\n');
					if (endid != NULL) *endid = 0;
				}
				id += 5; 
			}

			TCHAR timebuf[30] = TEXT("");
			GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);


			static const TCHAR format[] = TEXT(" %s v.%s%S%s [%s] - %S\r\n");

			buffer.AppendFormat(format, FindFileData.cFileName, 
				(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""),
				ver,
				(flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""),
				timebuf, id);
			CloseHandle(hDumpFile);
		}
	}
	while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
}


static void GetIconStrings(CMString& buffer)
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	LPTSTR fname = _tcsrchr(path, TEXT('\\'));
	if (fname == NULL) fname = path;
	mir_sntprintf(fname, MAX_PATH-(fname-path), TEXT("\\Icons\\*.*"));

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		TCHAR timebuf[30] = TEXT("");
		GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);

		buffer.AppendFormat(TEXT(" %s [%s]\r\n"), FindFileData.cFileName, timebuf);
	}
	while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
}


void PrintVersionInfo(CMString& buffer, unsigned flags)
{
	GetProcessorString(buffer);
	buffer.Append(TEXT("\r\n"));

	GetFreeMemoryString(buffer);
	buffer.Append(TEXT("\r\n"));

	GetOSDisplayString(buffer);
	buffer.Append(TEXT("\r\n"));

	GetInternetExplorerVersion(buffer);
	buffer.Append(TEXT("\r\n"));

	GetAdminString(buffer);
	buffer.Append(TEXT("\r\n"));

	GetLanguageString(buffer);
	buffer.Append(TEXT("\r\n"));

	TCHAR *profpathfull = Utils_ReplaceVarsT(profpath);
	if (flags & VI_FLAG_PRNVAR) {
		GetFreeDiskString(profpathfull, buffer);
		buffer.Append(TEXT("\r\n"));
	}

	buffer.AppendFormat(TEXT("\r\nMiranda NG Version: %s"), vertxt); 
	GetWow64String(buffer);
	buffer.Append(TEXT("\r\n"));

	TCHAR path[MAX_PATH], mirtime[30];
	GetModuleFileName(NULL, path, MAX_PATH); 
	GetLastWriteTime(path, mirtime, 30);
	buffer.AppendFormat(TEXT("Build time: %s\r\n"), mirtime); 

	TCHAR profpn[MAX_PATH];
	mir_sntprintf(profpn, SIZEOF(profpn), TEXT("%s\\%s"), profpathfull, profname);

	buffer.AppendFormat(TEXT("Profile: %s\r\n"), profpn);

	if (flags & VI_FLAG_PRNVAR) {
		WIN32_FIND_DATA FindFileData;

		HANDLE hFind = FindFirstFile(profpn, &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);

			unsigned __int64 fsize = (unsigned __int64)FindFileData.nFileSizeHigh << 32 | FindFileData.nFileSizeLow;
			buffer.AppendFormat(TEXT("Profile size: %I64u Bytes\r\n"), fsize), 

				GetLastWriteTime(&FindFileData.ftCreationTime, mirtime, 30);
			buffer.AppendFormat(TEXT("Profile creation date: %s\r\n"), mirtime);
		}
	}
	mir_free(profpathfull);

	GetLanguagePackString(buffer);
	buffer.Append(TEXT("\r\n"));

	// buffer.AppendFormat(TEXT("Nightly: %s\r\n"), _tcsstr(vertxt, TEXT("alpha")) ? TEXT("Yes") : TEXT("No")); 
	// buffer.AppendFormat(TEXT("Unicode: %s\r\n"), _tcsstr(vertxt, TEXT("Unicode")) ? TEXT("Yes") : TEXT("No")); 

	GetPluginsString(buffer, flags);

	if (flags & VI_FLAG_WEATHER) {
		buffer.AppendFormat(TEXT("\r\n%sWeather ini files:%s\r\n-------------------------------------------------------------------------------\r\n"),
			(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""),
			(flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""));
		GetWeatherStrings(buffer, flags);
	}

	if (flags & VI_FLAG_PRNVAR && !servicemode) {
		buffer.AppendFormat(TEXT("\r\n%sProtocols and Accounts:%s\r\n-------------------------------------------------------------------------------\r\n"),
			(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""),
			(flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""));
		GetProtocolStrings(buffer);
	}

	if (flags & VI_FLAG_PRNVAR) {
		buffer.AppendFormat(TEXT("\r\n%sIcon Packs:%s\r\n-------------------------------------------------------------------------------\r\n"),
			(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""),
			(flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""));
		GetIconStrings(buffer);
	}

	if (flags & VI_FLAG_PRNDLL) {
		__try {
			buffer.Append(TEXT("\r\nLoaded Modules:\r\n-------------------------------------------------------------------------------\r\n"));
			EnumerateLoadedModules64(GetCurrentProcess(), LoadedModules64, &buffer);
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {}
	}
}


void CreateCrashReport(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr, const TCHAR* msg)
{
	if (exc_ptr->ContextRecord == NULL || (exc_ptr->ContextRecord->ContextFlags & CONTEXT_CONTROL) == 0) 
		return;

	CONTEXT context = *exc_ptr->ContextRecord;

	STACKFRAME64 frame = {0};

#if defined(_AMD64_)
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_AMD64
	frame.AddrPC.Offset = context.Rip;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrStack.Offset = context.Rsp;
#elif defined(_IA64_)
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_IA64
	frame.AddrPC.Offset = context.StIIP;
	frame.AddrFrame.Offset = context.AddrBStore;
	frame.AddrStack.Offset = context.SP;
#else
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_I386
	frame.AddrPC.Offset = context.Eip;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrStack.Offset = context.Esp;
#endif

	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;

	const PLUGININFOEX *pluginInfoEx = GetPluginInfoEx();

	TCHAR curtime[30];
	GetISO8061Time(NULL, curtime, 30);

	CMString buffer;
	buffer.AppendFormat(TEXT("Miranda Crash Report from %s. Crash Dumper v.%d.%d.%d.%d\r\n"),
		curtime, 
		HIBYTE(HIWORD(pluginInfoEx->version)), LOBYTE(HIWORD(pluginInfoEx->version)), 
		HIBYTE(LOWORD(pluginInfoEx->version)), LOBYTE(LOWORD(pluginInfoEx->version)));

	int crashpos = buffer.GetLength();

	ReadableExceptionInfo(exc_ptr->ExceptionRecord, buffer);
	buffer.Append(TEXT("\r\n"));

	const HANDLE hProcess = GetCurrentProcess();

	if (&SymSetOptions)
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	SymInitialize(hProcess, NULL, TRUE);

	buffer.Append(TEXT("\r\nStack Trace:\r\n---------------------------------------------------------------\r\n"));

	for (int i=81; --i;) {
		char symbuf[sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME * sizeof(TCHAR) + 4] = {0};
		PIMAGEHLP_SYMBOL64 pSym = (PIMAGEHLP_SYMBOL64)symbuf;
		pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		pSym->MaxNameLength = MAX_SYM_NAME;

		IMAGEHLP_LINE64 Line = {0};
		Line.SizeOfStruct = sizeof(Line);

		IMAGEHLP_MODULE64 Module = {0};
		Module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);

		char undName[MAX_SYM_NAME] = "";
		char undFullName[MAX_SYM_NAME] = "";

		DWORD64 offsetFromSmybol = 0;
		DWORD offsetFromLine = 0;

		if (!StackWalk64(IMAGE_FILE_MACHINE, hProcess, GetCurrentThread(), &frame, &context,
			NULL, SymFunctionTableAccess64,  SymGetModuleBase64, NULL)) break;

		if (frame.AddrPC.Offset == frame.AddrReturn.Offset) break;

		if (frame.AddrPC.Offset != 0) {
			if (SymGetSymFromAddr64(hProcess, frame.AddrPC.Offset, &offsetFromSmybol, pSym)) {
				UnDecorateSymbolName(pSym->Name, undName, MAX_SYM_NAME, UNDNAME_NAME_ONLY);
				UnDecorateSymbolName(pSym->Name, undFullName, MAX_SYM_NAME, UNDNAME_COMPLETE);
			}

			SymGetLineFromAddr64(hProcess, frame.AddrPC.Offset, &offsetFromLine, &Line);
			SymGetModuleInfo64(hProcess, frame.AddrPC.Offset, &Module);
			if (Module.ModuleName[0] == 0) {
				FindData data;
				data.Offset = frame.AddrPC.Offset;
				data.pModule = &Module;
				EnumerateLoadedModules64(hProcess, LoadedModulesFind64, &data);
			}
		}

		const char* name;
		if (undFullName[0] != 0)
			name = undFullName;
		else if (undName[0] != 0)
			name = undName;
		else if (pSym->Name[0] != 0)
			name =  pSym->Name;
		else
			name = "(function-name not available)";

		const char *lineFileName = Line.FileName ? Line.FileName : "(filename not available)";
		const char *moduleName = Module.ModuleName[0] ? Module.ModuleName : "(module-name not available)";

		if (crashpos != 0) {
			HMODULE hModule = (HMODULE)Module.BaseOfImage;
			PLUGININFOEX *pi = GetMirInfo(hModule);
			if (pi != NULL) {
				static const TCHAR formatc[] = TEXT("\r\nLikely cause of the crash plugin: %S\r\n\r\n");

				if (pi->shortName) {
					CMString crashcause;
					crashcause.AppendFormat(formatc, pi->shortName);
					buffer.Insert(crashpos, crashcause);
				}
				crashpos = 0;
			}
		}


		static const TCHAR formatd[] = TEXT("%p (%S %p): %S (%d): %S\r\n");

		buffer.AppendFormat(formatd, 
			(LPVOID)frame.AddrPC.Offset, moduleName, (LPVOID)Module.BaseOfImage, 
			lineFileName, Line.LineNumber, name);
	}
	SymCleanup(hProcess);
	buffer.Append(TEXT("\r\n"));

	PrintVersionInfo(buffer, VI_FLAG_PRNDLL);


	int len = WideCharToMultiByte(CP_UTF8, 0, buffer.c_str(), -1, NULL, 0, NULL, NULL);
	char* dst = (char*)(len > 8192 ? malloc(len) : alloca(len));
	WideCharToMultiByte(CP_UTF8, 0, buffer.c_str(), -1, dst, len, NULL, NULL);

	WriteUtfFile(hDumpFile, dst);

	if (len > 8192) free(dst);


	if (db_get_b(0,PluginName,"ShowCrashMessageBox",1) && msg && MessageBox(NULL, msg, TEXT("Miranda Crash Dumper"), MB_YESNO | MB_ICONERROR | MB_TASKMODAL | MB_DEFBUTTON2 | MB_TOPMOST) == IDYES)
		StoreStringToClip(buffer);
}
