/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

static int CompareHashes(const ServListEntry *p1, const ServListEntry *p2)
{
	return _wcsicmp(p1->m_name, p2->m_name);
}

ServerConfig::ServerConfig() :
	arHashes(50, CompareHashes),
	arRename(50),
	arPackets(50)
{}

/////////////////////////////////////////////////////////////////////////////////////
// Loads server config from the server

bool ServerConfig::Load()
{
	REPLACEVARSARRAY vars[2];
	vars[0].key.w = L"platform";
	#ifdef _WIN64
	vars[0].value.w = L"64";
	#else
	vars[0].value.w = L"32";
	#endif
	vars[1].key.w = vars[1].value.w = nullptr;

	ptrW updateUrl(GetDefaultUrl());
	m_baseUrl = Utils_ReplaceVarsW(updateUrl, 0, vars);

	// Download version info
	FILEURL pFileUrl;
	mir_snwprintf(pFileUrl.wszDownloadURL, L"%s/hashes.zip", m_baseUrl.get());
	mir_snwprintf(pFileUrl.wszDiskPath, L"%s\\hashes.zip", g_wszTempPath);
	pFileUrl.CRCsum = 0;

	HNETLIBCONN nlc = nullptr;
	int ret = DownloadFile(&pFileUrl, nlc);
	Netlib_CloseHandle(nlc);

	if (ret != ERROR_SUCCESS) {
		Netlib_LogfW(g_hNetlibUser, L"Downloading list of available updates from %s failed with error %d", m_baseUrl.get(), ret);
		Skin_PlaySound("updatefailed");

		if (ret == 404) {
			ShowPopup(TranslateT("Plugin Updater"), TranslateT("Updates are temporarily disabled, try again later."), POPUP_TYPE_INFO);
			return true;
		}

		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		return false;
	}

	if (unzip(pFileUrl.wszDiskPath, g_wszTempPath, nullptr, true)) {
		Netlib_LogfW(g_hNetlibUser, L"Unzipping list of available updates from %s failed", m_baseUrl.get());
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		Skin_PlaySound("updatefailed");
		return false;
	}

	DeleteFile(pFileUrl.wszDiskPath);

	TFileName wszTmpIni;
	mir_snwprintf(wszTmpIni, L"%s\\hashes.txt", g_wszTempPath);
	FILE *fp = _wfopen(wszTmpIni, L"r");
	if (!fp) {
		Netlib_LogfW(g_hNetlibUser, L"Opening %s failed", g_wszTempPath);
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		return false;
	}

	bool bDoNotSwitchToStable = false;
	char str[200];
	while (fgets(str, _countof(str), fp) != nullptr) {
		rtrim(str);
		// Do not allow the user to switch back to stable
		if (!strcmp(str, "DoNotSwitchToStable")) {
			bDoNotSwitchToStable = true;
		}
		else if (str[0] != ';') { // ';' marks a comment
			Netlib_Logf(g_hNetlibUser, "Update: %s", str);
			char *p = strchr(str, ' ');
			if (p != nullptr) {
				*p++ = 0;
				_strlwr(p);

				int dwCrc32;
				char *p1 = strchr(p, ' ');
				if (p1 == nullptr)
					dwCrc32 = 0;
				else {
					*p1++ = 0;
					sscanf(p1, "%08x", &dwCrc32);
				}
				arHashes.insert(new ServListEntry(str, p, dwCrc32));
			}
		}
	}
	fclose(fp);
	DeleteFileW(wszTmpIni);

	// building table of rules 
	mir_snwprintf(wszTmpIni, L"%s\\rules.txt", g_wszTempPath);

	JSONNode root;
	if (file2json(wszTmpIni, root)) {
		for (auto &it : root["rules"]) {
			Utf2T wszName(it.name());
			if (it.isnull())
				arRename.insert(new RenameTableItem(wszName, nullptr));
			else
				arRename.insert(new RenameTableItem(wszName, it.as_mstring()));
		}

		for (auto &it : root["packets"]) {
			auto *pItem = new PacketTableItem(it["module"].as_mstring());
			if (auto val = it["minver"].as_int())
				pItem->osMin = val;
			if (auto val = it["maxver"].as_int())
				pItem->osMax = val;
			arPackets.insert(pItem);
		}
	}
	DeleteFileW(wszTmpIni);

	if (bDoNotSwitchToStable) {
		g_plugin.setByte(DB_SETTING_DONT_SWITCH_TO_STABLE, 1);
		// Reset setting if needed
		if (g_plugin.iUpdateMode == UPDATE_MODE_STABLE)
			g_plugin.iUpdateMode = UPDATE_MODE_TRUNK;
	}
	else g_plugin.setByte(DB_SETTING_DONT_SWITCH_TO_STABLE, 0);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Checks if file needs to be renamed and copies it in pNewName
// Returns true if smth. was copied

bool ServerConfig::CheckRename(const wchar_t *pwszFolder, const wchar_t *pwszOldName, wchar_t *pNewName)
{
	MFilePath fullOldPath;
	fullOldPath.Format(L"%s\\%s", pwszFolder, pwszOldName);

	for (auto &it : arRename) {
		if (wildcmpiw(pwszOldName, it->wszSearch)) {
			if (it->wszReplace == nullptr)
				*pNewName = 0;
			else {
				wcsncpy_s(pNewName, MAX_PATH, it->wszReplace, _TRUNCATE);
				size_t cbLen = wcslen(it->wszReplace) - 1;
				if (pNewName[cbLen] == '*')
					wcsncpy_s(pNewName + cbLen, MAX_PATH - cbLen, pwszOldName, _TRUNCATE);

				// don't try to rename a file to itself
				MFilePath fullNewPath;
				fullNewPath.Format(L"%s\\%s", g_mirandaPath.get(), pNewName);
				if (fullNewPath == fullOldPath)
					return false;
			}
			return true;
		}
	}

	return false;
}
