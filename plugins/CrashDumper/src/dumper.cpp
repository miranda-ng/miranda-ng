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

#include "stdafx.h"

extern wchar_t* vertxt;
extern wchar_t* profname;
extern wchar_t* profpath;

void CreateMiniDump(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr)
{
	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = exc_ptr;
	exceptionInfo.ClientPointers = false;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &exceptionInfo, nullptr, nullptr);
}

void WriteBBFile(CMStringW &buffer, bool hdr)
{
	static const wchar_t header[] = L"[spoiler=VersionInfo][quote]";
	static const wchar_t footer[] = L"[/quote][/spoiler]";

	buffer.Append(hdr ? header : footer);
}

void WriteUtfFile(HANDLE hDumpFile, char *bufu)
{
	DWORD bytes;
	static const unsigned char bytemark[] = { 0xEF, 0xBB, 0xBF };
	WriteFile(hDumpFile, bytemark, 3, &bytes, nullptr);
	WriteFile(hDumpFile, bufu, (uint32_t)mir_strlen(bufu), &bytes, nullptr);
}

BOOL CALLBACK LoadedModules64(LPCSTR, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	const HMODULE hModule = (HMODULE)ModuleBase;

	wchar_t path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);

	CMStringW &buffer = *(CMStringW*)UserContext;
	buffer.AppendFormat(L"%s  %p - %p", path, (void*)ModuleBase, (void*)(ModuleBase + ModuleSize));

	GetVersionInfo(hModule, buffer);

	wchar_t timebuf[30] = L"";
	GetLastWriteTime(path, timebuf, 30);

	buffer.AppendFormat(L" [%s]\r\n", timebuf);
	return TRUE;
}

struct FindData
{
	DWORD64 Offset;
	IMAGEHLP_MODULE64* pModule;
};

BOOL CALLBACK LoadedModulesFind64(LPCSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	FindData *data = (FindData*)UserContext;

	if ((uint32_t)(data->Offset - ModuleBase) < ModuleSize) {
		const size_t len = _countof(data->pModule->ModuleName);
		strncpy(data->pModule->ModuleName, ModuleName, len);
		data->pModule->ModuleName[len - 1] = 0;

		data->pModule->BaseOfImage = ModuleBase;

		const HMODULE hModule = (HMODULE)ModuleBase;
		GetModuleFileNameA(hModule, data->pModule->LoadedImageName, _countof(data->pModule->LoadedImageName));

		return FALSE;
	}
	return TRUE;
}

void GetLinkedModulesInfo(wchar_t *moduleName, CMStringW &buffer)
{
	HANDLE hDllFile = CreateFile(moduleName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hDllFile == INVALID_HANDLE_VALUE)
		return;

	HANDLE hDllMapping = CreateFileMapping(hDllFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (hDllMapping == INVALID_HANDLE_VALUE) {
		CloseHandle(hDllFile);
		return;
	}

	LPVOID dllAddr = MapViewOfFile(hDllMapping, FILE_MAP_READ, 0, 0, 0);

	__try {
		PIMAGE_NT_HEADERS nthdrs = ImageNtHeader(dllAddr);

		ULONG tableSize;
		PIMAGE_IMPORT_DESCRIPTOR importData = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(dllAddr, FALSE, IMAGE_DIRECTORY_ENTRY_IMPORT, &tableSize);
		if (importData) {
			CMStringW wszEnvPath;
			uint32_t dwLen = GetEnvironmentVariableW(L"Path", nullptr, 0);
			wszEnvPath.Preallocate(dwLen + 1);
			GetEnvironmentVariableW(L"Path", wszEnvPath.GetBuffer(), dwLen);

			while (importData->Name) {
				char *szImportModule = (char*)ImageRvaToVa(nthdrs, dllAddr, importData->Name, nullptr);
				if (!SearchPathW(wszEnvPath, _A2T(szImportModule), nullptr, NULL, nullptr, nullptr))
					buffer.AppendFormat(L"    Plugin statically linked to missing module: %S\r\n", szImportModule);

				importData++; //go to next record
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}

	UnmapViewOfFile(dllAddr);
	CloseHandle(hDllMapping);
	CloseHandle(hDllFile);
}

static int CompareDlls(const wchar_t *p1, const wchar_t *p2)
{
	return mir_wstrcmpi(p1, p2);
}

static void GetPluginsString(CMStringW &buffer, unsigned &flags)
{
	buffer.AppendFormat(L"Service Mode: %s\r\n", g_plugin.bServiceMode ? L"Yes" : L"No");

	wchar_t path[MAX_PATH];
	GetModuleFileName(nullptr, path, MAX_PATH);

	LPTSTR fname = wcsrchr(path, '\\');
	if (fname == nullptr)
		fname = path;
	mir_snwprintf(fname, MAX_PATH - (fname - path), L"\\plugins\\*.dll");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	size_t count = 0, ucount = 0;

	CMStringW ubuffer, tmp;
	LIST<wchar_t> arDlls(10, CompareDlls);

	do {
		bool loaded = false;
		mir_snwprintf(fname, MAX_PATH - (fname - path), L"\\plugins\\%s", FindFileData.cFileName);
		HMODULE hModule = GetModuleHandle(path);
		if (hModule == nullptr && g_plugin.bServiceMode) {
			hModule = LoadLibrary(path);
			loaded = true;
		}

		if (hModule == nullptr) {
			if ((flags & VI_FLAG_PRNVAR) && IsPluginOnWhiteList(_T2A(FindFileData.cFileName))) {
				wchar_t timebuf[30] = L"";
				GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);

				ubuffer.AppendFormat(L"\xa4 %s v.%s%d.%d.%d.%d%s [%s] - %S %s\r\n", FindFileData.cFileName,
					(flags & VI_FLAG_FORMAT) ? L"[b]" : L"",
					0, 0, 0, 0,
					(flags & VI_FLAG_FORMAT) ? L"[/b]" : L"",
					timebuf, "<unknown>", L"");

				GetLinkedModulesInfo(path, ubuffer);
				ubuffer.Append(L"\r\n");

				++ucount;
			}
			continue;
		}

		const PLUGININFOEX *pi = GetMirInfo(hModule);
		if (pi != nullptr) {
			wchar_t timebuf[30] = L"";
			GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);

			const wchar_t *unica = !(((PLUGININFOEX*)pi)->flags & UNICODE_AWARE) ? L"|ANSI|" : L"";

			int v1, v2, v3, v4;
			DWORD unused, verInfoSize = GetFileVersionInfoSize(path, &unused);
			if (verInfoSize != 0) {
				UINT blockSize;
				VS_FIXEDFILEINFO *fi;
				void *pVerInfo = mir_alloc(verInfoSize);
				GetFileVersionInfo(path, 0, verInfoSize, pVerInfo);
				VerQueryValue(pVerInfo, L"\\", (LPVOID*)&fi, &blockSize);
				v1 = HIWORD(fi->dwFileVersionMS), v2 = LOWORD(fi->dwFileVersionMS), v3 = HIWORD(fi->dwFileVersionLS), v4 = LOWORD(fi->dwFileVersionLS);
				mir_free(pVerInfo);
			}
			else {
				uint32_t ver = pi->version;
				v1 = HIBYTE(HIWORD(ver)), v2 = LOBYTE(HIWORD(ver)), v3 = HIBYTE(LOWORD(ver)), v4 = LOBYTE(LOWORD(ver));
			}

			tmp.Format(L"\xa4 %s v.%s%d.%d.%d.%d%s [%s] - %S %s\r\n", FindFileData.cFileName,
				(flags & VI_FLAG_FORMAT) ? L"[b]" : L"",
				v1, v2, v3, v4,
				(flags & VI_FLAG_FORMAT) ? L"[/b]" : L"",
				timebuf, pi->shortName ? pi->shortName : "", unica);
			arDlls.insert(tmp.Detach());

			if (mir_wstrcmpi(FindFileData.cFileName, L"weather.dll") == 0)
				flags |= VI_FLAG_WEATHER;

			++count;
		}
		if (loaded)
			FreeLibrary(hModule);
	}
		while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);

	buffer.AppendFormat(L"\r\n%sActive Plugins (%u):%s\r\n", (flags & VI_FLAG_FORMAT) ? L"[b]" : L"", count, (flags & VI_FLAG_FORMAT) ? L"[/b]" : L"");

	for (auto &str : arDlls) {
		buffer.Append(str);
		mir_free(str);
	}

	if (ucount) {
		buffer.AppendFormat(L"\r\n%sUnloadable Plugins (%u):%s\r\n", (flags & VI_FLAG_FORMAT) ? L"[b]" : L"", ucount, (flags & VI_FLAG_FORMAT) ? L"[/b]" : L"");
		buffer.Append(ubuffer);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

struct ProtoCount : public MZeroedObject
{
	ProtoCount(char *p) :
		szProto(p)
	{}

	char *szProto;
	int   countse;
	int   countsd;
	bool  nloaded;

	static int Compare(const ProtoCount *p1, const ProtoCount *p2)
	{	return mir_strcmp(p1->szProto, p2->szProto);
	}
};

static void GetProtocolStrings(CMStringW &buffer)
{
	int protoCount;
	PROTOCOLDESCRIPTOR **protoList;
	Proto_EnumProtocols(&protoCount, &protoList);

	OBJLIST<ProtoCount> arProtos(10, &ProtoCount::Compare);

	// add first all declared protocols, both old & new
	for (int i = 0; i < protoCount; i++)
		switch (protoList[i]->type) {
		case PROTOTYPE_PROTOCOL:
		case PROTOTYPE_PROTOWITHACCS:
			arProtos.insert(new ProtoCount(protoList[i]->szName));
		}

	// try to gather all missing protocols from accounts
	for (auto &pa : Accounts()) {
		ProtoCount *p = arProtos.find((ProtoCount*)&pa->szProtoName);
		if (p == nullptr)
			continue;

		p->nloaded = pa->bDynDisabled;
		if (pa->IsEnabled())
			++p->countse;
		else
			++p->countsd;
	}

	for (auto &p : arProtos) 
		if (p->countsd != 0 || p->countse != 0)
			buffer.AppendFormat(L"%-24S %d - Enabled %d - Disabled  %sLoaded\r\n", p->szProto, p->countse, p->countsd, p->nloaded ? L"Not " : L"");
}

static void GetWeatherStrings(CMStringW &buffer, unsigned flags)
{
	wchar_t path[MAX_PATH];
	GetModuleFileName(nullptr, path, MAX_PATH);

	LPTSTR fname = wcsrchr(path, L'\\');
	if (fname == nullptr)
		fname = path;
	mir_snwprintf(fname, MAX_PATH - (fname - path), L"\\plugins\\weather\\*.ini");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		mir_snwprintf(fname, MAX_PATH - (fname - path), L"\\plugins\\weather\\%s", FindFileData.cFileName);
		HANDLE hDumpFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hDumpFile != INVALID_HANDLE_VALUE) {
			char buf[8192];

			DWORD bytes = 0;
			ReadFile(hDumpFile, buf, 8190, &bytes, nullptr);
			buf[bytes] = 0;

			char *ver = strstr(buf, "Version=");
			if (ver != nullptr) {
				char *endid = strchr(ver, '\r');
				if (endid != nullptr) *endid = 0;
				else {
					endid = strchr(ver, '\n');
					if (endid != nullptr) *endid = 0;
				}
				ver += 8;
			}

			char *id = strstr(buf, "Name=");
			if (id != nullptr) {
				char *endid = strchr(id, '\r');
				if (endid != nullptr) *endid = 0;
				else {
					endid = strchr(id, '\n');
					if (endid != nullptr)
						*endid = 0;
				}
				id += 5;
			}

			wchar_t timebuf[30] = L"";
			GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);

			buffer.AppendFormat(L" %s v.%s%S%s [%s] - %S\r\n", FindFileData.cFileName,
				(flags & VI_FLAG_FORMAT) ? L"[b]" : L"",
				ver,
				(flags & VI_FLAG_FORMAT) ? L"[/b]" : L"",
				timebuf, id);
			CloseHandle(hDumpFile);
		}
	}
		while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
}

static void GetIconStrings(CMStringW& buffer)
{
	wchar_t path[MAX_PATH];
	GetModuleFileName(nullptr, path, MAX_PATH);

	LPTSTR fname = wcsrchr(path, L'\\');
	if (fname == nullptr) fname = path;
	mir_snwprintf(fname, MAX_PATH - (fname - path), L"\\Icons\\*.*");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		wchar_t timebuf[30] = L"";
		GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);

		buffer.AppendFormat(L" %s [%s]\r\n", FindFileData.cFileName, timebuf);
	} while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
}

void PrintVersionInfo(CMStringW& buffer, unsigned flags)
{
	GetProcessorString(buffer);
	buffer.Append(L"\r\n");

	GetFreeMemoryString(buffer);
	buffer.Append(L"\r\n");

	char szOsVer[200];
	OS_GetDisplayString(szOsVer, _countof(szOsVer));
	buffer.Append(_A2T(szOsVer));
	buffer.Append(L"\r\n");

	GetInternetExplorerVersion(buffer);
	buffer.Append(L"\r\n");

	GetAdminString(buffer);
	buffer.Append(L"\r\n");

	GetLanguageString(buffer);
	buffer.Append(L"\r\n");

	wchar_t *profpathfull = Utils_ReplaceVarsW(profpath);
	if (flags & VI_FLAG_PRNVAR) {
		GetFreeDiskString(profpathfull, buffer);
		buffer.Append(L"\r\n");
	}

	buffer.AppendFormat(L"\r\nMiranda NG Version: %s", vertxt);
	GetWow64String(buffer);
	buffer.Append(L"\r\n");

	wchar_t path[MAX_PATH], mirtime[30];
	GetModuleFileName(nullptr, path, MAX_PATH);
	GetLastWriteTime(path, mirtime, 30);
	buffer.AppendFormat(L"Build time: %s\r\n", mirtime);

	wchar_t profpn[MAX_PATH];
	mir_snwprintf(profpn, L"%s\\%s", profpathfull, profname);
	buffer.AppendFormat(L"Profile: %s\r\n", profpn);

	if (auto *pDb = db_get_current())
		if (auto *dbLink = pDb->GetDriver())
			buffer.AppendFormat(L"Database driver: %s\r\n", dbLink ->szFullName);

	if (flags & VI_FLAG_PRNVAR) {
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(profpn, &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE) {
			FindClose(hFind);

			unsigned __int64 fsize = (unsigned __int64)FindFileData.nFileSizeHigh << 32 | FindFileData.nFileSizeLow;
			buffer.AppendFormat(L"Profile size: %I64u Bytes\r\n", fsize), GetLastWriteTime(&FindFileData.ftCreationTime, mirtime, 30);
			buffer.AppendFormat(L"Profile creation date: %s\r\n", mirtime);
		}
	}
	mir_free(profpathfull);

	GetLanguagePackString(buffer);
	buffer.Append(L"\r\n");

	GetPluginsString(buffer, flags);

	if (flags & VI_FLAG_WEATHER) {
		buffer.AppendFormat(L"\r\n%sWeather ini files:%s\r\n-------------------------------------------------------------------------------\r\n",
			(flags & VI_FLAG_FORMAT) ? L"[b]" : L"",
			(flags & VI_FLAG_FORMAT) ? L"[/b]" : L"");
		GetWeatherStrings(buffer, flags);
	}

	if (flags & VI_FLAG_PRNVAR && !g_plugin.bServiceMode) {
		buffer.AppendFormat(L"\r\n%sProtocols and Accounts:%s\r\n-------------------------------------------------------------------------------\r\n",
			(flags & VI_FLAG_FORMAT) ? L"[b]" : L"",
			(flags & VI_FLAG_FORMAT) ? L"[/b]" : L"");
		GetProtocolStrings(buffer);
	}

	if (flags & VI_FLAG_PRNVAR) {
		buffer.AppendFormat(L"\r\n%sIcon Packs:%s\r\n-------------------------------------------------------------------------------\r\n",
			(flags & VI_FLAG_FORMAT) ? L"[b]" : L"",
			(flags & VI_FLAG_FORMAT) ? L"[/b]" : L"");
		GetIconStrings(buffer);
	}

	if (flags & VI_FLAG_PRNDLL) {
		__try {
			buffer.Append(L"\r\nLoaded Modules:\r\n-------------------------------------------------------------------------------\r\n");
			EnumerateLoadedModules64(GetCurrentProcess(), LoadedModules64, &buffer);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}
}

void CreateCrashReport(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr, const wchar_t* msg)
{
	if (exc_ptr->ContextRecord == nullptr || (exc_ptr->ContextRecord->ContextFlags & CONTEXT_CONTROL) == 0)
		return;

	CONTEXT context = *exc_ptr->ContextRecord;

	STACKFRAME64 frame = {};

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

	wchar_t curtime[30];
	GetISO8061Time(nullptr, curtime, 30);

	CMStringW buffer;
	buffer.AppendFormat(L"Miranda Crash Report from %s. Crash Dumper v.%d.%d.%d.%d\r\n", 
		curtime, __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);

	int crashpos = buffer.GetLength();

	ReadableExceptionInfo(exc_ptr->ExceptionRecord, buffer);
	buffer.Append(L"\r\n");

	const HANDLE hProcess = GetCurrentProcess();

	if (&SymSetOptions)
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	SymInitialize(hProcess, nullptr, TRUE);

	buffer.Append(L"\r\nStack Trace:\r\n---------------------------------------------------------------\r\n");

	for (int i = 81; --i;) {
		char symbuf[sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME * sizeof(wchar_t) + 4] = {};
		PIMAGEHLP_SYMBOL64 pSym = (PIMAGEHLP_SYMBOL64)symbuf;
		pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		pSym->MaxNameLength = MAX_SYM_NAME;

		IMAGEHLP_LINE64 Line = {};
		Line.SizeOfStruct = sizeof(Line);

		IMAGEHLP_MODULE64 Module = {};
		Module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);

		char undName[MAX_SYM_NAME] = "";
		char undFullName[MAX_SYM_NAME] = "";

		DWORD64 offsetFromSmybol = 0;
		DWORD offsetFromLine = 0;

		if (!StackWalk64(IMAGE_FILE_MACHINE, hProcess, GetCurrentThread(), &frame, &context,
			nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) break;

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

		const char *name;
		if (undFullName[0] != 0)
			name = undFullName;
		else if (undName[0] != 0)
			name = undName;
		else if (pSym->Name[0] != 0)
			name = pSym->Name;
		else
			name = "(function-name not available)";

		const char *lineFileName = Line.FileName ? Line.FileName : "(filename not available)";
		const char *moduleName = Module.ModuleName[0] ? Module.ModuleName : "(module-name not available)";

		if (crashpos != 0) {
			HMODULE hModule = (HMODULE)Module.BaseOfImage;
			const PLUGININFOEX *pi = GetMirInfo(hModule);
			if (pi != nullptr) {
				if (pi->shortName) {
					CMStringW crashcause;
					crashcause.AppendFormat(L"\r\nLikely cause of the crash plugin: %S\r\n\r\n", pi->shortName);
					buffer.Insert(crashpos, crashcause);
				}
				crashpos = 0;
			}
		}

		buffer.AppendFormat(L"%p (%S %p): %S (%d): %S\r\n", (void*)frame.AddrPC.Offset, moduleName, (void*)Module.BaseOfImage, lineFileName, Line.LineNumber, name);
	}
	
	SymCleanup(hProcess);
	buffer.Append(L"\r\n");

	PrintVersionInfo(buffer, VI_FLAG_PRNDLL);

	int len = WideCharToMultiByte(CP_UTF8, 0, buffer.c_str(), -1, nullptr, 0, nullptr, nullptr);
	char* dst = (char*)(len > 8192 ? malloc(len) : alloca(len));
	WideCharToMultiByte(CP_UTF8, 0, buffer.c_str(), -1, dst, len, nullptr, nullptr);

	WriteUtfFile(hDumpFile, dst);

	if (len > 8192)
		free(dst);

	if (g_plugin.getByte("ShowCrashMessageBox", 1) && msg && IDYES == MessageBox(nullptr, msg, L"Miranda Crash Dumper", MB_YESNO | MB_ICONERROR | MB_TASKMODAL | MB_DEFBUTTON2 | MB_TOPMOST))
		Utils_ClipboardCopy(buffer);
}
