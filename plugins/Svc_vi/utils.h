#ifndef _M_VERSIONINFO_UTILS_H
#define _M_VERSIONINFO_UTILS_H

#include "common.h"

//utils.cpp
void MB(char*);
void Log(char*);
char *StrTrim(char *, const char *);

//logging functions
//utils.cpp
void LogToFileInit();
void LogToFile(char *format, ...);

//utils.cpp
char *RelativePathToAbsolute(char *szRelative, char *szAbsolute, size_t size);
char *AbsolutePathToRelative(char *szAbsolute, char *szRelative, size_t size);

//returns a string from the database and uses MirandaFree to deallocate the string, leaving only the local copy
//utils.cpp
int GetStringFromDatabase(char *szSettingName, char *szError, char *szResult, size_t size);

//a string of the form %s(start) | %s(end) is split into the two strings (start and end)
//utils.cpp
int SplitStringInfo(const char *szWholeText, char *szStartText, char *szEndText);

//utils.cpp
bool DoesDllExist(char *dllName);

//utils.cpp
void GetModuleTimeStamp(char*, char*);
void NotifyError(DWORD, char*, int);

//utils.cpp
PLUGININFOEX *GetPluginInfo(const char *,HINSTANCE *);
PLUGININFOEX *CopyPluginInfo(PLUGININFOEX *);
void FreePluginInfo(PLUGININFOEX *);

//utils.cpp

BOOL IsCurrentUserLocalAdministrator();

char *GetLanguageName(LANGID language);
char *GetLanguageName(LCID locale);

BOOL GetWindowsShell(char *shellPath, size_t shSize);
BOOL GetInternetExplorerVersion(char *ieVersion, size_t ieSize);

BOOL UUIDToString(MUUID uuid, char *str, size_t len);

BOOL IsUUIDNull(MUUID uuid);

#endif