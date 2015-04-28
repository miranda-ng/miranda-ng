/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

#include "common.h"

IconInfo CSkypeProto::Icons[] =
{
	{ LPGENT("Protocol icon"),			"main",				IDI_SKYPE },
	{ LPGENT("Create new chat icon"),	"conference",  IDI_CONFERENCE },
	{ LPGENT("Sync history icon"),		"synchistory", IDI_SYNCHISTORY},
	{ LPGENT("Block user icon"),		"user_block",	IDI_BLOCKUSER },
	{ LPGENT("Unblock user icon"),	"user_unblock",	  IDI_UNBLOCKUSER }
};
void CSkypeProto::InitIcons()
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(g_hInstance, szFile, MAX_PATH);

	char szSettingName[100];
	TCHAR szSectionName[100];

	SKINICONDESC sid = { sizeof(SKINICONDESC) };
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.ptszSection = szSectionName;

	mir_sntprintf(szSectionName, SIZEOF(szSectionName), _T("%s/%s"), LPGENT("Protocols"), LPGENT(MODULE));
	for (int i = 0; i < SIZEOF(Icons); i++)
	{
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, Icons[i].Name);

		sid.ptszDescription = Icons[i].Description;
		sid.iDefaultIndex = -Icons[i].IconId;
		Icons[i].Handle = Skin_AddIcon(&sid);
	}

}

HANDLE CSkypeProto::GetIconHandle(const char *name)
{
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		if (mir_strcmpi(Icons[i].Name, name) == 0)
			return Icons[i].Handle;
	return 0;
}

HANDLE CSkypeProto::GetSkinIconHandle(const char *name)
{
	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, name);
	HANDLE hIcon = Skin_GetIconHandle(iconName);
	if (hIcon == NULL)
		hIcon = GetIconHandle(name);
	return hIcon;
}

void CSkypeProto::UninitIcons()
{
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		Skin_RemoveIcon(Icons[i].Name);
}