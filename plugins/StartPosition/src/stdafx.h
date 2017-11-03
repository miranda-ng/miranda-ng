/*
StartPosition plugin for Miranda NG

Copyright (C) 2005-2008 Felipe Brahm - souFrag
ICQ#50566818
http://www.soufrag.cl

Copyright (C) 2012-17 Miranda NG project (https://miranda-ng.org)

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
#include <windows.h>

#include <newpluginapi.h>
#include <m_gui.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_plugin.h>

#include "resource.h"
#include "version.h"

extern HINSTANCE g_hInst;

#define MODULE_NAME "StartPosition"
#define CLIST_MODULE_NAME "CList"

class StartPositionPlugin;
#include "options.h"
#include "startposition.h"
