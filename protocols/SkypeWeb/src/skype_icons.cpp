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

#include "stdafx.h"

IconItemT CSkypeProto::Icons[] =
{
	{ LPGENT("Protocol icon"),        "main",             IDI_SKYPE         },
	{ LPGENT("Create new chat icon"), "conference",       IDI_CONFERENCE    },
	{ LPGENT("Sync history icon"),    "synchistory",      IDI_SYNCHISTORY   },
	{ LPGENT("Block user icon"),      "user_block",       IDI_BLOCKUSER     },
	{ LPGENT("Unblock user icon"),    "user_unblock",     IDI_UNBLOCKUSER   },
	{ LPGENT("Incoming call icon"),   "inc_call",         IDI_CALL          },
	{ LPGENT("Notification icon"),    "notify",           IDI_NOTIFY        },
	{ LPGENT("Error icon"),           "error",            IDI_ERRORICON     },
	{ LPGENT("Action icon"),          "me_action",        IDI_ACTION_ME     }
};

void CSkypeProto::InitIcons()
{
	Icon_RegisterT(g_hInstance, LPGENT("Protocols") L"/" LPGENT(MODULE), Icons, _countof(Icons), MODULE);
}

HICON CSkypeProto::GetIcon(int iconId)
{
	for (size_t i = 0; i < _countof(Icons); i++)
		if (Icons[i].defIconID == iconId)
			return IcoLib_GetIconByHandle(Icons[i].hIcolib);
	return 0;
}

HANDLE CSkypeProto::GetIconHandle(int iconId)
{
	for (size_t i = 0; i < _countof(Icons); i++)
		if (Icons[i].defIconID == iconId)
			return Icons[i].hIcolib;
	return 0;
}