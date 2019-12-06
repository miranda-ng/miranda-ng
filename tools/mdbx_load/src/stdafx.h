/*
Copyright (C) 2018-19 Miranda NG team (https://miranda-ng.org)

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

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define MDBX_TOOLS /* Avoid using internal mdbx_assert() */
#include "../../libs/libmdbx/src/src/elements/internals.h"

#define CMP_UINT(x, y) { if ((x) != (y)) return (x) < (y) ? -1 : 1; }

struct DBEventSortingKey
{
	uint32_t hContact;
	uint32_t hEvent;
	uint64_t ts;

	static int Compare(const MDBX_val *ax, const MDBX_val *bx)
	{
		const DBEventSortingKey *a = (DBEventSortingKey*)ax->iov_base;
		const DBEventSortingKey *b = (DBEventSortingKey*)bx->iov_base;

		CMP_UINT(a->hContact, b->hContact);
		CMP_UINT(a->ts, b->ts);
		CMP_UINT(a->hEvent, b->hEvent);
		return 0;
	}
};

struct DBEventIdKey
{
	uint32_t iModuleId;	    // offset to a DBModuleName struct of the name of
	char     szEventId[256]; // string id

	static int Compare(const MDBX_val *ax, const MDBX_val *bx)
	{
		const DBEventIdKey *a = (DBEventIdKey*)ax->iov_base;
		const DBEventIdKey *b = (DBEventIdKey*)bx->iov_base;
		CMP_UINT(a->iModuleId, b->iModuleId);
		return strcmp(a->szEventId, b->szEventId);
	}
};

struct DBSettingKey
{
	uint32_t hContact;
	uint32_t dwModuleId;
	char     szSettingName[1];

	static int Compare(const MDBX_val *ax, const MDBX_val *bx)
	{
		const DBSettingKey *a = (DBSettingKey*)ax->iov_base;
		const DBSettingKey *b = (DBSettingKey*)bx->iov_base;

		CMP_UINT(a->hContact, b->hContact);
		CMP_UINT(a->dwModuleId, b->dwModuleId);
		return strcmp(a->szSettingName, b->szSettingName);
	}
};
