/*

StartPosition plugin for Miranda NG

Copyright (C) 2005-2008 Felipe Brahm - souFrag
ICQ#50566818
http://www.soufrag.cl

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <windows.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_options.h>

#include "resource.h"
#include "version.h"

#define MODULE_NAME "StartPosition"

#define dbERROR -999999
#define RIGHT 1
#define LEFT 0
