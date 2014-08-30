#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <dbghelp.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

// ordinals to names 
typedef std::map<DWORD, std::vector<const char*> > ExportMap;

void GetExports(PVOID pMapping, ExportMap& exports, WORD& imageType)
{
	ULONG size = 0;
	PIMAGE_NT_HEADERS pHead = ImageNtHeader(pMapping);
	PIMAGE_EXPORT_DIRECTORY pDir = static_cast<PIMAGE_EXPORT_DIRECTORY>(ImageDirectoryEntryToData(pMapping, FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size));
	if(pHead && pDir)
	{
		imageType = pHead->FileHeader.Machine;
		char* base = static_cast<char*>(pMapping);
		DWORD rvaExportDir = reinterpret_cast<char*>(pDir) - base;
		PIMAGE_SECTION_HEADER pSec = NULL;
		PDWORD namesAddr = static_cast<PDWORD>(ImageRvaToVa(pHead, pMapping, pDir->AddressOfNames, &pSec));
		PWORD ordinalAddr = static_cast<PWORD>(ImageRvaToVa(pHead, pMapping, pDir->AddressOfNameOrdinals, &pSec));
		DWORD numFuncs = pDir->NumberOfFunctions;
		DWORD numNames = pDir->NumberOfNames;
		DWORD baseOrd = pDir->Base;

		std::set<DWORD> ordinals;
		DWORD lastOrd = baseOrd + numFuncs;
		for(DWORD i = baseOrd; i < lastOrd; ++i)
		{
			ordinals.insert(i);
		}

		for(DWORD i = 0; i < numNames; ++i)
		{
			WORD thisZeroBasedOrd = ordinalAddr[i];
			DWORD thisBasedOrd = thisZeroBasedOrd + baseOrd;
			ordinals.erase(thisBasedOrd);
			LPCSTR name = static_cast<LPCSTR>(ImageRvaToVa(pHead, pMapping, namesAddr[i], &pSec));
			exports[thisBasedOrd].push_back(name);
		}

		for(std::set<DWORD>::const_iterator iter = ordinals.begin(), end = ordinals.end();
			iter != end;
			++iter
		)
		{
			exports.insert(std::make_pair(*iter, ExportMap::mapped_type()));
		}
	}
}

PVOID MapFile(HANDLE hFile, DWORD fileMapPerm, DWORD mappedPerms)
{
	PVOID base = NULL;
	HANDLE hMap = CreateFileMappingW(
			hFile,
			NULL,
			fileMapPerm,
			0,
			0,
			NULL
	);
	if(hMap)
	{
		base = MapViewOfFile(hMap, mappedPerms, 0, 0, 0);
		CloseHandle(hMap);
	}
	return base;
}

PVOID MapFile(const wchar_t* fileName, DWORD filePerms, DWORD fileMapPerm, DWORD mappedPerms)
{
	PVOID base = NULL;
	HANDLE hFile = CreateFileW(
		fileName,
		filePerms,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		base = MapFile(hFile, fileMapPerm, mappedPerms);
		CloseHandle(hFile);
	}
	return base;
}

PVOID MapFileRO(const wchar_t* fileName, BOOL image)
{
	return MapFile(fileName, GENERIC_READ, PAGE_READONLY | (image ? SEC_IMAGE : 0), FILE_MAP_READ);
}

std::wstring GetFileName(const wchar_t* pFilePath)
{
    const wchar_t* pAfterSlash = pFilePath, *pSlash = NULL;
    while((pSlash = wcspbrk(pAfterSlash, L"\\/")) != NULL)
    {
        pAfterSlash = pSlash + 1;
    }
    const wchar_t* pDot = wcschr(pAfterSlash, L'.');
    if(pDot != NULL)
    {
        return std::wstring(pAfterSlash, pDot);
    }
    else return std::wstring(pAfterSlash);
}

std::wstring MakeWString(const char* string)
{
	size_t reqd = mbstowcs(NULL, string, 0);
	if(reqd != static_cast<size_t>(-1))
	{
		std::wstring str(reqd, 0);
		mbstowcs(&str[0], string, reqd);
		return str;
	}
	return std::wstring(string, string + strlen(string) + 1);
}

void DoAndWaitForExecution(SHELLEXECUTEINFO& sei)
{
	if(!ShellExecuteEx(&sei))
	{
		std::wcerr << L"Couldn't execute \"" << sei.lpFile << L'\n';
	}
	else
	{
		WaitForSingleObject(sei.hProcess, INFINITE);
		CloseHandle(sei.hProcess);
	}
}

void CreateLibs(const std::wstring& defFile, const std::wstring& baseName, const std::wstring& outDir, WORD machineType)
{
	std::wstring curDir(1000, 0);
	curDir.resize(GetModuleFileName(GetModuleHandle(NULL), &curDir[0], curDir.length()));
	curDir.resize(curDir.find_last_of(L"\\/") + 1);
	SHELLEXECUTEINFO sei = {sizeof(sei), 0};
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = GetConsoleWindow();
	sei.lpVerb = L"open";
	sei.lpDirectory = outDir.c_str();

	// 32-bit libs
	if(machineType == IMAGE_FILE_MACHINE_I386)
	{
		std::wcout << L"Creating 32-bit libs...\n";
		// dlltool for mingw (x86)
		std::wcout << L"\tCreating lib" << baseName << L".a...\n";
		std::wostringstream pstream;
		pstream << L"-d \"" << defFile << L"\" -l lib" << baseName << L".a";

		std::wstring binary = curDir + L"dlltool.exe";
		std::wstring parameters = pstream.str();
		
		sei.lpFile = binary.c_str();
		sei.lpParameters = parameters.c_str();

		DoAndWaitForExecution(sei);

		pstream.str(L"");

		// link for msvc (x86)
		std::wcout << L"\tCreating " << baseName << L".lib...\n";
		pstream << L"/lib /nologo /machine:x86 /def:\"" << defFile << "\" /out:\"" << outDir << baseName << L".lib\"";

		binary = curDir + L"link.exe";
		parameters = pstream.str();
		sei.lpFile = binary.c_str();
		sei.lpParameters = parameters.c_str();
		DoAndWaitForExecution(sei);
	}
	else
	{
		std::wcout << L"Creating 64-bit libs...\n";
		// dlltool for mingw (x64) (my fail if running on 32-bit comp)
		std::wcout << L"\tlib" << baseName << L"-x64.a...\n";
		
		std::wstring binary = curDir + L"dlltool64.exe";
		std::wstring as64Pos = L"\"";
		as64Pos += (curDir + L"as64.exe\"");
		std::wostringstream pstream;
		pstream << L"-d \"" << defFile << L"\" -S " << as64Pos << L" -l lib" << baseName << L"-x64.a";

		std::wstring parameters = pstream.str();
		sei.lpFile = binary.c_str();
		sei.lpParameters = parameters.c_str();
		DoAndWaitForExecution(sei);
		
		pstream.str(L"");

		// link for msvc (x64)
		std::wcout << L'\t' << baseName << L"-x64.lib...\n";
		binary = curDir + L"link.exe";
		pstream << L"/lib /nologo /machine:x64 /def:\"" << defFile << "\" /out:\"" << outDir << baseName << L"-x64.lib\"";
		parameters = pstream.str();
		sei.lpFile = binary.c_str();
		sei.lpParameters = parameters.c_str();
		DoAndWaitForExecution(sei);
	}
}

BOOL CreateDef(LPCWSTR dllFile, const std::wstring& defFile, WORD& machineType)
{
	std::wcout << L"Creating " << defFile << L"...\n";
    // this is FALSE on purpose. If it's true, some dll's (mainly bcrypt.dll from Vista)
    // fail to map as an image because of some funkyness with the certificate or
    // something (STATUS_INVALID_IMAGE_HASH fyi). So this is just easier
	PVOID pMapping = MapFileRO(dllFile, FALSE);
	if(pMapping)
	{
		ExportMap exports;
		GetExports(pMapping, exports, machineType);
		if(!exports.empty())
		{
			std::wofstream defFile(defFile.c_str());
			std::wostringstream expDetails;
			defFile << L"LIBRARY \"" << GetFileName(dllFile) << L"\"\n\nEXPORTS\n";
			for(ExportMap::const_iterator iter = exports.begin(), end = exports.end();
				iter != end;
				++iter
			)
			{
				expDetails.str(L"");
				expDetails.clear();
				expDetails << L'\t';
				const ExportMap::mapped_type& names = iter->second;
				if(!names.empty())
				{
					const std::wstring& initialName = MakeWString(*(names.begin()));
					expDetails << initialName << L"\t@" << iter->first << L'\n';
					for(ExportMap::mapped_type::const_iterator namesIter = names.begin() + 1, namesEnd = names.end();
						namesIter != namesEnd;
						++namesIter
					)
					{
						expDetails << '\t' << MakeWString(*namesIter) << L'=' << initialName << L'\n';
					}
				}
				// unnamed export
				else
				{
					expDetails << L"Ordinal" << iter->first << L" NONAME\t@" << iter->first << L'\n';
				}
				defFile << expDetails.str();
			}
		}
		else
		{
			std::wcerr << dllFile << L" has no exports, nothing to do\n";
			return FALSE;
		}
		UnmapViewOfFile(pMapping);
	}
	return TRUE;
}

int __cdecl wmain(int argc, wchar_t** argv)
{
    std::wcout << L"DefLibCreator v1.0 - Airesoft.co.uk\n\n";
	if(argc < 2)
	{
		puts("Usage: DefLibCreator dll [outDir]");
		return 1;
	}
	else
	{
		std::wstring baseFileName = PathFindFileName(argv[1]);
		baseFileName.erase(baseFileName.find_last_of('.'));
		int outDirArg = (argc >= 3) ? 2 : 1;
		LPWSTR pFilePart = NULL;
		std::wstring outDir(GetFullPathName(argv[outDirArg], 0, NULL, NULL), 0);
		outDir.resize(GetFullPathName(argv[outDirArg], outDir.length(), &outDir[0], &pFilePart));
		// if the explicit output dir wasn't specified, we need to chop the dll filename off the end of outDir
		if(outDirArg == 1)
		{
			outDir.resize(pFilePart - outDir.c_str());
		}
		else
		{
			CreateDirectory(outDir.c_str(), NULL);
		}
		outDir.push_back(L'\\'); // ensure it ends with a slash
		std::wstring defFile(outDir);
		defFile += (baseFileName + L".def");
		WORD machineType = 0;
		if(CreateDef(argv[1], defFile, machineType))
		{
			CreateLibs(defFile, baseFileName, outDir, machineType);
		}		
	}
	return 0;
}
