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

#include "Services.h"

HANDLE gServices[40] = {0};

INT_PTR DBEntityGetRoot(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getEntities().getRootEntity();
}
INT_PTR DBEntityChildCount(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().getChildCount(hEntity);
}
INT_PTR DBEntityGetParent(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().getParent(hEntity);
}
INT_PTR DBEntityMove(WPARAM hEntity, LPARAM hParent)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().setParent(hEntity, hParent);
}
INT_PTR DBEntityGetFlags(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().getFlags(hEntity);
}
INT_PTR DBEntityIterInit(WPARAM pFilter, LPARAM hParent)
{
	TDBTEntityIterFilter fil = {0,0,0,0};
	if (pFilter == NULL)
	{
		pFilter = reinterpret_cast<WPARAM>(&fil);
		fil.cbSize = sizeof(fil);
	}

	if (reinterpret_cast<PDBTEntityIterFilter>(pFilter)->cbSize != sizeof(TDBTEntityIterFilter))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTEntityIterFilter>(pFilter)->fDontHasFlags & ((PDBTEntityIterFilter)pFilter)->fHasFlags)
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().IterationInit(*reinterpret_cast<PDBTEntityIterFilter>(pFilter), hParent);
}
INT_PTR DBEntityIterNext(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEntities().IterationNext(hIteration);
}
INT_PTR DBEntityIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEntities().IterationClose(hIteration);
}
INT_PTR DBEntityDelete(WPARAM hEntity, LPARAM lParam)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return gDataBase->getEntities().DeleteEntity(hEntity);
}
INT_PTR DBEntityCreate(WPARAM pEntity, LPARAM lParam)
{
	if (reinterpret_cast<PDBTEntity>(pEntity)->bcSize != sizeof(TDBTEntity))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTEntity>(pEntity)->hParentEntity == 0)
		reinterpret_cast<PDBTEntity>(pEntity)->hParentEntity = gDataBase->getEntities().getRootEntity();

	reinterpret_cast<PDBTEntity>(pEntity)->fFlags = reinterpret_cast<PDBTEntity>(pEntity)->fFlags & ~(DBT_NF_IsRoot | DBT_NF_HasChildren | DBT_NF_IsVirtual | DBT_NF_HasVirtuals); // forbidden flags...
	return gDataBase->getEntities().CreateEntity(*reinterpret_cast<PDBTEntity>(pEntity));
}

INT_PTR DBEntityGetAccount(WPARAM hEntity, LPARAM lParam)
{
	return gDataBase->getEntities().getAccount(hEntity);
}

INT_PTR DBVirtualEntityCreate(WPARAM hEntity, LPARAM hParent)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEntities().VirtualCreate(hEntity, hParent);
}
INT_PTR DBVirtualEntityGetParent(WPARAM hVirtualEntity, LPARAM lParam)
{
	if ((hVirtualEntity == 0) || (hVirtualEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return gDataBase->getEntities().VirtualGetParent(hVirtualEntity);
}
INT_PTR DBVirtualEntityGetFirst(WPARAM hEntity, LPARAM lParam)
{
	if ((hEntity == 0) || (hEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return gDataBase->getEntities().VirtualGetFirst(hEntity);
}
INT_PTR DBVirtualEntityGetNext(WPARAM hVirtualEntity, LPARAM lParam)
{
	if ((hVirtualEntity == 0) || (hVirtualEntity == gDataBase->getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return gDataBase->getEntities().VirtualGetNext(hVirtualEntity);
}


INT_PTR DBSettingFind(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().FindSetting(*reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor));
}
INT_PTR DBSettingDelete(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().DeleteSetting(*reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor));
}
INT_PTR DBSettingDeleteHandle(WPARAM hSetting, LPARAM lParam)
{
	if (hSetting == 0)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().DeleteSetting(hSetting);
}
INT_PTR DBSettingWrite(WPARAM pSetting, LPARAM lParam)
{
	if (pSetting == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTSetting>(pSetting)->Type & DBT_STF_VariableLength) && (reinterpret_cast<PDBTSetting>(pSetting)->Value.pBlob == NULL))
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().WriteSetting(*reinterpret_cast<PDBTSetting>(pSetting));
}
INT_PTR DBSettingWriteHandle(WPARAM pSetting, LPARAM hSetting)
{
	if (pSetting == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().WriteSetting(*reinterpret_cast<PDBTSetting>(pSetting), hSetting);
}
INT_PTR DBSettingRead(WPARAM pSetting, LPARAM lParam)
{
	if (pSetting == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().ReadSetting(*reinterpret_cast<PDBTSetting>(pSetting));
}
INT_PTR DBSettingReadHandle(WPARAM pSetting, LPARAM hSetting)
{
	if ((pSetting == NULL) || (hSetting == 0))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTSetting>(pSetting)->Descriptor != NULL) && (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor)))
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().ReadSetting(*((PDBTSetting)pSetting), hSetting);
}
INT_PTR DBSettingIterInit(WPARAM pFilter, LPARAM lParam)
{
	if (pFilter == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingIterFilter>(pFilter)->cbSize != sizeof(TDBTSettingIterFilter))
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTSettingIterFilter>(pFilter)->Descriptor != NULL) && (reinterpret_cast<PDBTSettingIterFilter>(pFilter)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor)))
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTSettingIterFilter>(pFilter)->Setting != NULL) && (reinterpret_cast<PDBTSettingIterFilter>(pFilter)->Setting->cbSize != sizeof(TDBTSetting)))
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTSettingIterFilter>(pFilter)->Setting != NULL) && (reinterpret_cast<PDBTSettingIterFilter>(pFilter)->Setting->Descriptor != NULL) && (reinterpret_cast<PDBTSettingIterFilter>(pFilter)->Setting->Descriptor->cbSize != sizeof(TDBTSettingIterFilter)))
		return DBT_INVALIDPARAM;

	return gDataBase->getSettings().IterationInit(*reinterpret_cast<PDBTSettingIterFilter>(pFilter));
}
INT_PTR DBSettingIterNext(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getSettings().IterationNext(hIteration);
}
INT_PTR DBSettingIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getSettings().IterationClose(hIteration);
}

INT_PTR DBEventGetBlobSize(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().GetBlobSize(hEvent);
}

INT_PTR DBEventGet(WPARAM hEvent, LPARAM pEvent)
{
	if ((pEvent == NULL) || (reinterpret_cast<PDBTEvent>(pEvent)->cbSize != sizeof(TDBTEvent)))
		return DBT_INVALIDPARAM;

	return gDataBase->getEvents().Get(hEvent, *reinterpret_cast<PDBTEvent>(pEvent));
}

INT_PTR DBEventGetCount(WPARAM hEntity, LPARAM lParam)
{
	return gDataBase->getEvents().GetCount(hEntity);
}

INT_PTR DBEventDelete(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().Delete(hEvent);
}

INT_PTR DBEventAdd(WPARAM hEntity, LPARAM pEvent)
{
	if ((pEvent == NULL) || (reinterpret_cast<PDBTEvent>(pEvent)->cbSize != sizeof(TDBTEvent)) || (reinterpret_cast<PDBTEvent>(pEvent)->pBlob == NULL) || (reinterpret_cast<PDBTEvent>(pEvent)->cbBlob == 0))
		return DBT_INVALIDPARAM;

	return gDataBase->getEvents().Add(hEntity, *((PDBTEvent)pEvent));
}

INT_PTR DBEventMarkRead(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().MarkRead(hEvent);
}

INT_PTR DBEventWriteToDisk(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().WriteToDisk(hEvent);
}

INT_PTR DBEventGetEntity(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().getEntity(hEvent);
}

INT_PTR DBEventIterInit(WPARAM pFilter, LPARAM lParam)
{
	if ((pFilter == NULL) || (reinterpret_cast<PDBTEventIterFilter>(pFilter)->cbSize != sizeof(TDBTEventIterFilter)))
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTEventIterFilter>(pFilter)->Event != NULL) && (reinterpret_cast<PDBTEventIterFilter>(pFilter)->Event->cbSize != sizeof(TDBTEvent)))
		return DBT_INVALIDPARAM;

	return gDataBase->getEvents().IterationInit(*reinterpret_cast<PDBTEventIterFilter>(pFilter));
}

INT_PTR DBEventIterNext(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEvents().IterationNext(hIteration);
}

INT_PTR DBEventIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return gDataBase->getEvents().IterationClose(hIteration);
}


bool RegisterServices()
{
	gServices[ 0] = CreateServiceFunction(MS_DBT_ENTITY_GETROOT,          DBEntityGetRoot);
	gServices[ 1] = CreateServiceFunction(MS_DBT_ENTITY_CHILDCOUNT,       DBEntityChildCount);
	gServices[ 2] = CreateServiceFunction(MS_DBT_ENTITY_GETPARENT,        DBEntityGetParent);
	gServices[ 3] = CreateServiceFunction(MS_DBT_ENTITY_MOVE,             DBEntityMove);
	gServices[ 8] = CreateServiceFunction(MS_DBT_ENTITY_GETFLAGS,         DBEntityGetFlags);
	gServices[ 9] = CreateServiceFunction(MS_DBT_ENTITY_ITER_INIT,        DBEntityIterInit);
	gServices[10] = CreateServiceFunction(MS_DBT_ENTITY_ITER_NEXT,        DBEntityIterNext);
	gServices[11] = CreateServiceFunction(MS_DBT_ENTITY_ITER_CLOSE,       DBEntityIterClose);
	gServices[12] = CreateServiceFunction(MS_DBT_ENTITY_DELETE,           DBEntityDelete);
	gServices[13] = CreateServiceFunction(MS_DBT_ENTITY_CREATE,           DBEntityCreate);
	gServices[13] = CreateServiceFunction(MS_DBT_ENTITY_GETACCOUNT,       DBEntityGetAccount);

	gServices[14] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_CREATE,    DBVirtualEntityCreate);
	gServices[15] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_GETPARENT, DBVirtualEntityGetParent);
	gServices[16] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_GETFIRST,  DBVirtualEntityGetFirst);
	gServices[17] = CreateServiceFunction(MS_DBT_VIRTUALENTITY_GETNEXT,   DBVirtualEntityGetNext);

	gServices[18] = CreateServiceFunction(MS_DBT_SETTING_FIND,             DBSettingFind);
	gServices[19] = CreateServiceFunction(MS_DBT_SETTING_DELETE,           DBSettingDelete);
	gServices[20] = CreateServiceFunction(MS_DBT_SETTING_DELETEHANDLE,     DBSettingDeleteHandle);
	gServices[21] = CreateServiceFunction(MS_DBT_SETTING_WRITE,            DBSettingWrite);
	gServices[22] = CreateServiceFunction(MS_DBT_SETTING_WRITEHANDLE,      DBSettingWriteHandle);
	gServices[23] = CreateServiceFunction(MS_DBT_SETTING_READ,             DBSettingRead);
	gServices[24] = CreateServiceFunction(MS_DBT_SETTING_READHANDLE,       DBSettingReadHandle);
	gServices[25] = CreateServiceFunction(MS_DBT_SETTING_ITER_INIT,        DBSettingIterInit);
	gServices[26] = CreateServiceFunction(MS_DBT_SETTING_ITER_NEXT,        DBSettingIterNext);
	gServices[27] = CreateServiceFunction(MS_DBT_SETTING_ITER_CLOSE,       DBSettingIterClose);

	gServices[28] = CreateServiceFunction(MS_DBT_EVENT_GETBLOBSIZE,        DBEventGetBlobSize);
	gServices[29] = CreateServiceFunction(MS_DBT_EVENT_GET,                DBEventGet);
	gServices[30] = CreateServiceFunction(MS_DBT_EVENT_GETCOUNT,           DBEventGetCount);
	gServices[31] = CreateServiceFunction(MS_DBT_EVENT_DELETE,             DBEventDelete);
	gServices[32] = CreateServiceFunction(MS_DBT_EVENT_ADD,                DBEventAdd);
	gServices[33] = CreateServiceFunction(MS_DBT_EVENT_MARKREAD,           DBEventMarkRead);
	gServices[34] = CreateServiceFunction(MS_DBT_EVENT_WRITETODISK,        DBEventWriteToDisk);
	gServices[35] = CreateServiceFunction(MS_DBT_EVENT_GETENTITY,          DBEventGetEntity);
	gServices[36] = CreateServiceFunction(MS_DBT_EVENT_ITER_INIT,          DBEventIterInit);
	gServices[37] = CreateServiceFunction(MS_DBT_EVENT_ITER_NEXT,          DBEventIterNext);
	gServices[38] = CreateServiceFunction(MS_DBT_EVENT_ITER_CLOSE,         DBEventIterClose);


	return true;
}
