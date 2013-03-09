/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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

#ifndef M_FOLDERS_COMMONHEADERS_H
#define M_FOLDERS_COMMONHEADERS_H

#define MAX_FOLDER_SIZE 2048

#include <windows.h>
#include <richedit.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_system_cpp.h>
#include <m_options.h>

#include <m_variables.h>
#include <m_folders.h>
#include <m_langpack.h>

#include "version.h"
#include "utils.h"
#include "resource.h"
#include "services.h"
#include "events.h"
#include "folderItem.h"

#ifndef MS_DB_GETPROFILEPATH_BASIC //db3xSA
#define MS_DB_GETPROFILEPATH_BASIC	"DB/GetProfilePathBasic"
#endif

#define MS_FOLDERS_TEST_PLUGIN "Folders/Test/Plugin"

extern char ModuleName[];
extern HINSTANCE hInstance;
extern OBJLIST<CFolderItem> lstRegisteredFolders;

#endif //FOLDERS_COMMONHEADERS_H