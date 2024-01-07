/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

extern wchar_t g_szSkinLib[MAX_PATH];

/*
*	Prepare
*	prepares upperstring masks and registers them in IcoLib
*/

static wchar_t *getSectionName(int flag)
{
	switch (flag) {
	case MIRANDA_CASE:
		if (g_plugin.getByte("GroupMiranda", 1))
			return LPGENW("Client icons") L"/" LPGENW("Miranda clients");
		break;

	case MIRANDA_VERSION_CASE:
		if (g_plugin.getByte("GroupMirandaVersion", 1))
			return LPGENW("Client icons") L"/" LPGENW("Miranda clients");
		break;

	case MIRANDA_PACKS_CASE:
		if (g_plugin.getByte("GroupMirandaPacks", 1))
			return LPGENW("Client icons") L"/" LPGENW("Miranda clients") L"/" LPGENW("Pack overlays");
		break;

	case MULTI_CASE:
		if (g_plugin.getByte("GroupMulti", 1))
			return LPGENW("Client icons") L"/" LPGENW("Multi-proto clients");
		break;

	case GG_CASE:
		if (g_plugin.getByte("GroupGG", 1))
			return LPGENW("Client icons") L"/" LPGENW("Gadu-Gadu clients");
		break;

	case ICQ_CASE:
		if (g_plugin.getByte("GroupICQ", 1))
			return LPGENW("Client icons") L"/" LPGENW("ICQ clients");
		break;

	case IRC_CASE:
		if (g_plugin.getByte("GroupIRC", 1))
			return LPGENW("Client icons") L"/" LPGENW("IRC clients");
		break;

	case JABBER_CASE:
		if (g_plugin.getByte("GroupJabber", 1))
			return LPGENW("Client icons") L"/" LPGENW("Jabber clients");
		break;

	case RSS_CASE:
		if (g_plugin.getByte("GroupRSS", 1))
			return LPGENW("Client icons") L"/" LPGENW("RSS clients");
		break;

	case WEATHER_CASE:
		if (g_plugin.getByte("GroupWeather", 1))
			return LPGENW("Client icons") L"/" LPGENW("Weather clients");
		break;

	case FACEBOOK_CASE:
		if (g_plugin.getByte("GroupFacebook", 1))
			return LPGENW("Client icons") L"/" LPGENW("Facebook clients");
		break;

	case VK_CASE:
		if (g_plugin.getByte("GroupVK", 1))
			return LPGENW("Client icons") L"/" LPGENW("VKontakte clients");
		break;

	case OTHER_PROTOS_CASE:
		if (g_plugin.getByte("GroupOtherProtos", 1))
			return LPGENW("Client icons") L"/" LPGENW("Other protocols");
		break;

	case OTHERS_CASE:
		if (g_plugin.getByte("GroupOthers", 1))
			return LPGENW("Client icons") L"/" LPGENW("Other icons");
		break;

	case OVERLAYS_RESOURCE_CASE:
		if (g_plugin.getByte("GroupOverlaysResource", 1))
			return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Resource");
		break;

	case OVERLAYS_PLATFORM_CASE:
		if (g_plugin.getByte("GroupOverlaysPlatform", 1))
			return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Platform");
		break;

	case OVERLAYS_UNICODE_CASE:
		if (g_plugin.getByte("GroupOverlaysUnicode", 1))
			return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Platform");
		break;

	case OVERLAYS_SECURITY_CASE:
		if (g_plugin.getByte("GroupOverlaysSecurity", 1))
			return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Security");
		break;

	case OVERLAYS_PROTO_CASE:
		if (g_plugin.getByte("GroupOverlaysProtos", 1))
			return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Protocol");
		break;

	default:
		return LPGENW("Client icons");
	}
	return nullptr;
}

void Prepare(KN_FP_MASK *mask, bool bEnable, LIST<KN_FP_MASK> &dest)
{
	mask->szMaskUpper = nullptr;

	if (mask->hIcolibItem)
		IcoLib_RemoveIconByHandle(mask->hIcolibItem);
	mask->hIcolibItem = nullptr;

	if (!mask->szMask || !bEnable)
		return;

	size_t iMaskLen = mir_wstrlen(mask->szMask) + 1;
	LPTSTR pszNewMask = (LPTSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, iMaskLen * sizeof(wchar_t));
	wcscpy_s(pszNewMask, iMaskLen, mask->szMask);
	_wcsupr_s(pszNewMask, iMaskLen);
	mask->szMaskUpper = pszNewMask;

	wchar_t destfile[MAX_PATH];
	if (mask->iIconIndex == IDI_NOTFOUND || mask->iIconIndex == IDI_UNKNOWN || mask->iIconIndex == IDI_UNDETECTED)
		GetModuleFileName(g_plugin.getInst(), destfile, MAX_PATH);
	else {
		wcsncpy_s(destfile, g_szSkinLib, _TRUNCATE);
		if (0 != _waccess(destfile, 0))
			return;
	}

	LPTSTR SectName = getSectionName(mask->iSectionFlag);
	if (SectName == nullptr)
		return;

	SKINICONDESC sid = {};
	sid.flags = SIDF_ALL_UNICODE;
	sid.section.w = SectName;
	sid.pszName = mask->szIconName;
	sid.description.w = mask->szClientDescription;
	sid.defaultFile.w = destfile;
	sid.iDefaultIndex = -mask->iIconIndex;
	mask->hIcolibItem = g_plugin.addIcon(&sid);

	mask->idx = dest.getCount();
	dest.insert(mask);
}

/////////////////////////////////////////////////////////////////////////////////////////

static LIST<KN_FP_MASK> arCustom(10);

void UnregisterCustomIcons()
{
	for (auto &it : arCustom) {
		if (it->hIcolibItem)
			IcoLib_RemoveIconByHandle(it->hIcolibItem);

		mir_free(it->szClientDescription);
		mir_free(it->szIconName);
		mir_free(it->szMask);
		mir_free(it);
	}		
}

void RegisterCustomIcons()
{
	UnregisterCustomIcons();

	FILE *in = _wfopen(VARSW(L"%miranda_userdata%\\fingerprint.json"), L"r");
	if (in == nullptr)
		return;

	int cbSize = filelength(fileno(in));
	ptrA szJson((char*)mir_alloc(cbSize+1));
	fread(szJson, 1, cbSize, in);
	fclose(in);

	JSONNode root = JSONNode::parse(szJson);
	if (!root)
		return;

	for (auto &it : root["masks"]) {
		CMStringA szName = it["name"].as_mstring();
		CMStringW wszMask = it["mask"].as_mstring();
		CMStringW szDescription = it["descr"].as_mstring();
		if (wszMask.IsEmpty() || szName.IsEmpty() || szDescription.IsEmpty())
			continue;

		wchar_t wszFullPath[MAX_PATH];
		PathToAbsoluteW(it["iconFile"].as_mstring(), wszFullPath);
		if (0 != _waccess(wszFullPath, 0))
			continue;

		LPTSTR SectName = getSectionName(it["group"].as_int());
		if (SectName == nullptr)
			continue;

		auto *pMask = (KN_FP_MASK *)mir_calloc(sizeof(KN_FP_MASK));

		int iMaskLen = wszMask.GetLength() + 1;
		pMask->szMaskUpper = (LPTSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, iMaskLen * sizeof(wchar_t));
		wcscpy_s(pMask->szMaskUpper, iMaskLen, wszMask);
		_wcsupr_s(pMask->szMaskUpper, iMaskLen);
		pMask->szMask = pMask->szMaskUpper;

		pMask->iIconIndex = it["iconIndex"].as_int();
		pMask->szIconName = szName.Detach();
		pMask->szClientDescription = szDescription.Detach();

		SKINICONDESC sid = {};
		sid.flags = SIDF_ALL_UNICODE;
		sid.section.w = SectName;
		sid.pszName = pMask->szIconName;
		sid.description.w = pMask->szClientDescription;
		sid.defaultFile.w = wszFullPath;
		sid.iDefaultIndex = -pMask->iIconIndex;
		pMask->hIcolibItem = g_plugin.addIcon(&sid);

		pMask->idx = g_arCommon.getCount();
		g_arCommon.insert(pMask, 0);
	}
}
