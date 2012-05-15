/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#pragma once

#ifndef INTERFACE_VERSION_ONLY

#define MIRANDA_VER 0x0800
#include <windows.h>
#include "newpluginapi.h"
#include "m_plugins.h"
#include "m_system.h"
#include "m_utils.h"

#include "m_langpack.h"

#include "m_dbx_tree.h"

extern HINSTANCE   hInstance;
extern PLUGINLINK *pluginLink;
extern MM_INTERFACE mmi;
extern UTF8_INTERFACE utfi;

extern DATABASELINK gDBLink;
#endif 


#define gVersion 0x00000012
#define gResVersion 0,0,0,18
#define gResVersionString "0.0.0.18"
#define gInternalName "dbx_tree"
#define gInternalNameLong "Miranda dbx_tree database driver"
#define gDescription "Provides extended Miranda database support"
#define gAutor "Michael 'Protogenes' Kunz"
#define gAutorEmail "Michael.Kunz@s2005.TU-Chemnitz.de"
#define gCopyright "2007 - 2010 Michael 'Protogenes' Kunz"



