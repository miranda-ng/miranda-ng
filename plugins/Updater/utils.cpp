#include "common.h"
#include "utils.h"

bool VersionFromString(const char *szVer, DWORD *pdwVer) 
{
	char *p = (char *)szVer;
	*pdwVer = 0;

	int bytes = 1; // we start in the first 'byte'
	int digit_count = 0;
	while(*p && bytes <= 4 && digit_count <= 3) 
	{
		if(*p >= '0' && *p <= '9') 
		{
			*pdwVer = (*pdwVer & 0xFFFFFF00) + (*pdwVer & 0xFF) * 10 + (*p - '0');
			digit_count++;
		} 
		else if(*p == '.') 
		{
			*pdwVer = *pdwVer << 8;
			bytes++;
			digit_count = 0;
		} 
		else 
		{
			if(bytes < 3) // allow other chars on the end (e.g. space)
				return false; // incompatible version string
			else 
				return true;
		}
		p++;
	}

	// version must be x.x.x.x format (for now - until a convention is established
	// whereby we assume '0' bytes as either prefix or suffix)
	// 15/3/06 - allowing 3 digit version numbers (ostensibly for spamfilter definition files which use date for version)
	return (bytes >= 3);

}

int CheckForFileID(char *update_url, char *version_url, char *name) 
{
	if (strlen(update_url) > 45 && strncmp(update_url, MIM_DOWNLOAD_URL_PREFIX, 45) == 0)
	{
		char *p = update_url + 45;
		return atoi(p);
	}
	if (strlen(update_url) > 51 && strncmp(update_url, "http://www.miranda-im.org/download/feed.php?dlfile=", 51) == 0)
	{
		char *p = update_url + 51;
		return atoi(p);
	}
	if(strlen(update_url) > 47 && strncmp(update_url, "http://miranda-im.org/download/feed.php?dlfile=", 47) == 0)
	{
		char *p = update_url + 47;
		return atoi(p);
	}
	return -1;
}

bool CreatePath(const TCHAR *szDir) 
{
	if (!szDir) return false;

	DWORD dwAttributes;
	TCHAR *pszLastBackslash, szTestDir[ MAX_PATH ];

	lstrcpyn( szTestDir, szDir, SIZEOF( szTestDir ));
	if (( dwAttributes = GetFileAttributes( szTestDir )) != INVALID_FILE_ATTRIBUTES && ( dwAttributes & FILE_ATTRIBUTE_DIRECTORY ))
		return true;

	pszLastBackslash = _tcsrchr( szTestDir, '\\' );
	if ( pszLastBackslash == NULL )
		return true;

	*pszLastBackslash = '\0';
	CreatePath( szTestDir );
	*pszLastBackslash = '\\';

	return CreateDirectory( szTestDir, NULL ) != 0;
}

// must 'mir_free' return val
TCHAR *GetTString(const char *asc)
{
	if (!asc) return NULL;
	return mir_a2t(asc);
//	return (TCHAR*)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)asc);
}

void RemoveFolder(const TCHAR *src_folder) 
{
	TCHAR szFilesPath[MAX_PATH];
	mir_sntprintf(szFilesPath, SIZEOF(szFilesPath), _T("%s\\*.*"), src_folder);
	TCHAR *p = _tcsrchr(szFilesPath, '\\') + 1;


	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(szFilesPath, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if(findData.cFileName[0] != _T('.')) 
			{
				_tcscpy(p, findData.cFileName);

				if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					// recurse
					RemoveFolder(szFilesPath);
				else
					DeleteFile(szFilesPath);
			}
		} while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}

	RemoveDirectory(src_folder);
}

bool FolderIsEmpty(const TCHAR *folder)
{
	TCHAR szFilesPath[MAX_PATH];
	mir_sntprintf(szFilesPath, SIZEOF(szFilesPath), _T("%s\\*.*"), folder);

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(szFilesPath, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) 
	{
		do {
			if (_tcscmp(findData.cFileName, _T(".")) && _tcscmp(findData.cFileName, _T("..")))
			{
				FindClose(hFileSearch);
				return false;
			}
		} while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}

	return true;
}

bool DeleteNonDlls(const TCHAR *folder) 
{
	TCHAR szFilesPath[MAX_PATH];

	{
		TCHAR buff[200];
		mir_sntprintf(buff, SIZEOF(buff), _T("Deleting non-dlls in %s"), folder);
		NLog(buff);
	}

	mir_sntprintf(szFilesPath, SIZEOF(szFilesPath), _T("%s\\*.*"), folder);
	TCHAR *p = _tcsrchr(szFilesPath, '\\') + 1;

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(szFilesPath, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) {
		do {
			if (_tcscmp(findData.cFileName, _T(".")) && _tcscmp(findData.cFileName, _T(".."))) 
			{
				_tcscpy(p, findData.cFileName);
				if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					DeleteNonDlls(szFilesPath);
				} else {
					if (_tcsstr(findData.cFileName, _T(".dll")) == 0) 
					{
						{
							TCHAR buff[200];
							mir_sntprintf(buff, SIZEOF(buff), _T("Deleting %s"), folder);
							NLog(buff);
						}
						DeleteFile(szFilesPath);	
					}
				}
			}
		} while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}

	return true;
}

void* memmem (const void *buf1, size_t size1, const void *buf2,  size_t size2)
{
	char *ptr;
	const char *const last = (const char *)buf1 + size1 - size2;

	if (size2 == 0)  return (void *)buf1;

	for (ptr = (char *)buf1; ptr <= last; ++ptr)
	{
		if (*ptr == *(char *)buf2 && !memcmp(ptr, buf2, size2)) 
			return ptr;
	}

	return NULL;
}

bool IsAdminRequired(void)
{
	TCHAR path[MAX_PATH];
	GetRootDir(path);
	_tcscat(path, _T("\\test_tmp.tmp"));

	HANDLE hDatFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
	if (hDatFile != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(hDatFile);
		DeleteFile(path);
		return false;
	}

	return true;
}

void GetRootDir(TCHAR *szPath)
{
	GetModuleFileName(NULL, szPath, MAX_PATH);
	TCHAR *p = _tcsrchr(szPath, '\\'); if (p) *p = 0;
}

void NLog(char *msg) 
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)msg);
}

void NLogF(const char *fmt, ...)
{
	va_list va;
	char szText[1024];

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);

	CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)szText);
}


#ifdef _UNICODE
void NLog(wchar_t *msg) 
{
	char* a = mir_utf8encodeW(msg);
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)a);
	mir_free(a);
}
#endif