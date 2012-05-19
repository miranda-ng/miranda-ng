#ifndef _UTILS_INC
#define _UTILS_INC

#include "xmldata.h"

bool VersionFromString(const char *szVer, DWORD *pdwVer);
int CheckForFileID(char *update_url, char *version_url, char *name);
bool CreatePath(const TCHAR *path);
bool IsAdminRequired(void);
void GetRootDir(TCHAR *szPath);

// must 'free' return val
TCHAR *GetTString(const char *asc);
// use system default codepage - called from external process where langpack codepage is not (easily) accessible

void RemoveFolder(const TCHAR *src_folder);
bool FolderIsEmpty(const TCHAR *folder);
bool DeleteNonDlls(const TCHAR *folder);

#endif
