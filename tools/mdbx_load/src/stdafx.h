/*
Copyright (C) 2018-20 Miranda NG team (https://miranda-ng.org)

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

#define DECLARE_VERSION() \
   mdbx_version_info MDBX_version; \
   mdbx_build_info MDBX_build; \
   char* MDBX_sourcery_anchor; \
   HINSTANCE hDll = LoadLibraryA("libmdbx.mir"); \
   MDBX_version = *(mdbx_version_info *)GetProcAddress(hDll, "mdbx_version"); \
   MDBX_build = *(mdbx_build_info*)GetProcAddress(hDll, "mdbx_build"); \
   MDBX_sourcery_anchor = (char*)GetProcAddress(hDll, "mdbx_sourcery_MDBX_BUILD_SOURCERY"); 
