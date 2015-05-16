/*
CmdLine plugin for Miranda IM

Copyright © 2007 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>

#define NO_MIMCMD_COMMANDS

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_system_cpp.h>

#include "..\..\src\mimcmd_ipc.h"
#include "version.h"
#include "..\..\src\utils.h"
#include "commands.h"

int lpprintf(const char *format, ...);
