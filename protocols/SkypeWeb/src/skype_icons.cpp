/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),        "main",             IDI_SKYPE         },
	{ LPGEN("Create new chat icon"), "conference",       IDI_CONFERENCE    },
	{ LPGEN("Block user icon"),      "user_block",       IDI_BLOCKUSER     },
	{ LPGEN("Unblock user icon"),    "user_unblock",     IDI_UNBLOCKUSER   },
	{ LPGEN("Incoming call icon"),   "inc_call",         IDI_CALL          },
	{ LPGEN("Notification icon"),    "notify",           IDI_NOTIFY        },
	{ LPGEN("Error icon"),           "error",            IDI_ERRORICON     },
	{ LPGEN("Action icon"),          "me_action",        IDI_ACTION_ME     }
};

void CSkypeProto::InitIcons()
{
	g_plugin.registerIcon(LPGEN("Protocols") "/" MODULE, iconList, MODULE);
}
