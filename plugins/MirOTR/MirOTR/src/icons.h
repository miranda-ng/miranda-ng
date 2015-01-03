#pragma once
#define ICON_FINISHED (MODULENAME"_finished")
#define ICON_UNVERIFIED (MODULENAME"_unverified")
#define ICON_PRIVATE (MODULENAME"_private")
#define ICON_NOT_PRIVATE (MODULENAME"_not_private")
#define ICON_OTR (MODULENAME"_otr")
#define ICON_REFRESH (MODULENAME"_refresh")

void InitIcons();
HICON LoadIcon(const char* name, int big);
HANDLE GetIconHandle(const char* name);
void ReleaseIcon(const char* name, int big);
void ReleaseIcon(HICON handle, int big);