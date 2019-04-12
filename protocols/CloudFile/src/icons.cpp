#include "stdafx.h"

static IconItem iconList[] =
{
	{	LPGEN("Upload file(s)"), "upload",   IDI_UPLOAD   },
	{	LPGEN("Dropbox"),        "dropbox",  IDI_DROPBOX  },
	{	LPGEN("Google Drive"),   "gdrive",   IDI_GDRIVE   },
	{	LPGEN("OneDrive"),       "onedrive", IDI_ONEDRIVE },
	{	LPGEN("Yandex.Disk"),    "yadisk",   IDI_YADISK   }
};

void InitializeIcons()
{
	g_plugin.registerIcon("Protocols/" MODULENAME, iconList, MODULENAME);
}
