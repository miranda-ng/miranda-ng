/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libcurl.h"

LibCurl *LibCurl::instance = NULL;
LibCurl &curl = LibCurl::getInstance();

BOOL LibCurl::init()
{
	HMODULE h = LoadLibrary(_T("libcurl.dll"));
	if (h == NULL) 
	{
		h = LoadLibrary(_T("plugins\\libcurl.dll"));
		if (h == NULL) return FALSE;
	}

	version_info = (_curl_version_info)GetProcAddress(h, "curl_version_info");
	if (version_info == NULL) 
		return FALSE;

	PVERSION_INFO info = version_info(CURLVERSION_NOW);
	if (info == NULL) 
		return FALSE;

	if (info->version_num < PLUGIN_MAKE_VERSION(0, 7, 10, 0))
		return FALSE;

	if ((info->features & CURL_VERSION_SSL) == 0)
		return FALSE;

	bool found = false;
	for (int i = 0; info->protocols[i]; i++)
	{
		if (strcmp(info->protocols[i], "ftp") == 0)
			found = true;
	}

	if (!found) 
		return FALSE;

	global_init		= (_curl_global_init) GetProcAddress(h, "curl_global_init");
	global_cleanup	= (_curl_global_cleanup) GetProcAddress(h, "curl_global_cleanup");
	easy_init		= (_curl_easy_init) GetProcAddress(h, "curl_easy_init");
	easy_cleanup	= (_curl_easy_cleanup) GetProcAddress(h, "curl_easy_cleanup");
	easy_perform	= (_curl_easy_perform) GetProcAddress(h, "curl_easy_perform");
	easy_pause		= (_curl_easy_pause) GetProcAddress(h, "curl_easy_pause");
	easy_setopt		= (_curl_easy_setopt) GetProcAddress(h, "curl_easy_setopt");
	slist_append	= (_curl_slist_append) GetProcAddress(h, "curl_slist_append");
	slist_free_all	= (_curl_slist_free_all) GetProcAddress(h, "curl_slist_free_all");

	return TRUE;
}

void LibCurl::deinit()
{
	delete this;
}