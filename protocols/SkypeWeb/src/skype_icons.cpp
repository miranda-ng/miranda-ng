/*
Copyright (c) 2015-17 Miranda NG project (https://miranda-ng.org)

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
	{ LPGENW("Protocol icon"),        "main",             IDI_SKYPE         },
	{ LPGENW("Create new chat icon"), "conference",       IDI_CONFERENCE    },
	{ LPGENW("Sync history icon"),    "synchistory",      IDI_SYNCHISTORY   },
	{ LPGENW("Block user icon"),      "user_block",       IDI_BLOCKUSER     },
	{ LPGENW("Unblock user icon"),    "user_unblock",     IDI_UNBLOCKUSER   },
	{ LPGENW("Incoming call icon"),   "inc_call",         IDI_CALL          },
	{ LPGENW("Notification icon"),    "notify",           IDI_NOTIFY        },
	{ LPGENW("Error icon"),           "error",            IDI_ERRORICON     },
	{ LPGENW("Action icon"),          "me_action",        IDI_ACTION_ME     }
};

void CSkypeProto::InitIcons()
{
	Icon_RegisterT(g_hInstance, LPGENW("Protocols") L"/" _A2W(MODULE), Icons, _countof(Icons), MODULE);
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