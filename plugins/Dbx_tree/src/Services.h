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

#include "Interface.h"
#include "DataBase.h"

bool RegisterServices();


INT_PTR DBEntityGetRoot(WPARAM wParam, LPARAM lParam);
INT_PTR DBEntityChildCount(WPARAM hEntity, LPARAM lParam);
INT_PTR DBEntityGetParent(WPARAM hEntity, LPARAM lParam);
INT_PTR DBEntityMove(WPARAM hEntity, LPARAM hParent);
INT_PTR DBEntityGetFlags(WPARAM hEntity, LPARAM lParam);
INT_PTR DBEntityIterInit(WPARAM pFilter, LPARAM hParent);
INT_PTR DBEntityIterNext(WPARAM hIteration, LPARAM lParam);
INT_PTR DBEntityIterClose(WPARAM hIteration, LPARAM lParam);
INT_PTR DBEntityDelete(WPARAM hEntity, LPARAM lParam);
INT_PTR DBEntityCreate(WPARAM pEntity, LPARAM lParam);
INT_PTR DBEntityGetAccount(WPARAM hEntity, LPARAM lParam);

INT_PTR DBVirtualEntityCreate(WPARAM hEntity, LPARAM hParent);
INT_PTR DBVirtualEntityGetParent(WPARAM hVirtualEntity, LPARAM lParam);
INT_PTR DBVirtualEntityGetFirst(WPARAM hEntity, LPARAM lParam);
INT_PTR DBVirtualEntityGetNext(WPARAM hVirtualEntity, LPARAM lParam);

INT_PTR DBSettingFind(WPARAM pSettingDescriptor, LPARAM lParam);
INT_PTR DBSettingDelete(WPARAM pSettingDescriptor, LPARAM lParam);
INT_PTR DBSettingDeleteHandle(WPARAM hSetting, LPARAM lParam);
INT_PTR DBSettingWrite(WPARAM pSetting, LPARAM lParam);
INT_PTR DBSettingWriteHandle(WPARAM pSetting, LPARAM hSetting);
INT_PTR DBSettingRead(WPARAM pSetting, LPARAM lParam);
INT_PTR DBSettingReadHandle(WPARAM pSetting, LPARAM hSetting);
INT_PTR DBSettingIterInit(WPARAM pFilter, LPARAM lParam);
INT_PTR DBSettingIterNext(WPARAM hIteration, LPARAM lParam);
INT_PTR DBSettingIterClose(WPARAM hIteration, LPARAM lParam);

INT_PTR DBEventGetBlobSize(WPARAM hEvent, LPARAM lParam);
INT_PTR DBEventGet(WPARAM hEvent, LPARAM pEvent);
INT_PTR DBEventGetCount(WPARAM hEntity, LPARAM lParam);
INT_PTR DBEventDelete(WPARAM hEvent, LPARAM lParam);
INT_PTR DBEventAdd(WPARAM hEntity, LPARAM pEvent);
INT_PTR DBEventMarkRead(WPARAM hEvent, LPARAM lParam);
INT_PTR DBEventWriteToDisk(WPARAM hEvent, LPARAM lParam);
INT_PTR DBEventGetEntity(WPARAM hEvent, LPARAM lParam);
INT_PTR DBEventIterInit(WPARAM pFilter, LPARAM lParam);
INT_PTR DBEventIterNext(WPARAM hIteration, LPARAM lParam);
INT_PTR DBEventIterClose(WPARAM hIteration, LPARAM lParam);
