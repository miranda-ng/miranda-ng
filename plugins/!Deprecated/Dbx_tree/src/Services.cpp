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

#include "Interface.h"
#include "Database.h"

INT_PTR CDataBase::DBEntityGetRoot(WPARAM wParam, LPARAM lParam)
{
	return getEntities().getRootEntity();
}

INT_PTR CDataBase::DBEntityChildCount(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = getEntities().getRootEntity();

	return getEntities().getChildCount(hEntity);
}

INT_PTR CDataBase::DBEntityGetParent(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = getEntities().getRootEntity();

	return getEntities().getParent(hEntity);
}

INT_PTR CDataBase::DBEntityMove(WPARAM hEntity, LPARAM hParent)
{
	if ((hEntity == 0) || (hEntity == getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = getEntities().getRootEntity();

	return getEntities().setParent(hEntity, hParent);
}

INT_PTR CDataBase::DBEntityGetFlags(WPARAM hEntity, LPARAM lParam)
{
	if (hEntity == 0)
		hEntity = getEntities().getRootEntity();

	return getEntities().getFlags(hEntity);
}

INT_PTR CDataBase::DBEntityIterInit(WPARAM pFilter, LPARAM hParent)
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
		hParent = getEntities().getRootEntity();

	return getEntities().IterationInit(*reinterpret_cast<PDBTEntityIterFilter>(pFilter), hParent);
}

INT_PTR CDataBase::DBEntityIterNext(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return getEntities().IterationNext(hIteration);
}

INT_PTR CDataBase::DBEntityIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return getEntities().IterationClose(hIteration);
}

INT_PTR CDataBase::DBEntityDelete(WPARAM hEntity, LPARAM lParam)
{
	if ((hEntity == 0) || (hEntity == getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return getEntities().DeleteEntity(hEntity);
}

INT_PTR CDataBase::DBEntityCreate(WPARAM pEntity, LPARAM lParam)
{
	if (reinterpret_cast<PDBTEntity>(pEntity)->bcSize != sizeof(TDBTEntity))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTEntity>(pEntity)->hParentEntity == 0)
		reinterpret_cast<PDBTEntity>(pEntity)->hParentEntity = getEntities().getRootEntity();

	reinterpret_cast<PDBTEntity>(pEntity)->fFlags = reinterpret_cast<PDBTEntity>(pEntity)->fFlags & ~(DBT_NF_IsRoot | DBT_NF_HasChildren | DBT_NF_IsVirtual | DBT_NF_HasVirtuals); // forbidden flags...
	return getEntities().CreateEntity(*reinterpret_cast<PDBTEntity>(pEntity));
}

INT_PTR CDataBase::DBEntityGetAccount(WPARAM hEntity, LPARAM lParam)
{
	return getEntities().getAccount(hEntity);
}

INT_PTR CDataBase::DBVirtualEntityCreate(WPARAM hEntity, LPARAM hParent)
{
	if ((hEntity == 0) || (hEntity == getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	if (hParent == 0)
		hParent = getEntities().getRootEntity();

	return getEntities().VirtualCreate(hEntity, hParent);
}
INT_PTR CDataBase::DBVirtualEntityGetParent(WPARAM hVirtualEntity, LPARAM lParam)
{
	if ((hVirtualEntity == 0) || (hVirtualEntity == getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return getEntities().VirtualGetParent(hVirtualEntity);
}
INT_PTR CDataBase::DBVirtualEntityGetFirst(WPARAM hEntity, LPARAM lParam)
{
	if ((hEntity == 0) || (hEntity == getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return getEntities().VirtualGetFirst(hEntity);
}
INT_PTR CDataBase::DBVirtualEntityGetNext(WPARAM hVirtualEntity, LPARAM lParam)
{
	if ((hVirtualEntity == 0) || (hVirtualEntity == getEntities().getRootEntity()))
		return DBT_INVALIDPARAM;

	return getEntities().VirtualGetNext(hVirtualEntity);
}


INT_PTR CDataBase::DBSettingFind(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return getSettings().FindSetting(*reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor));
}
INT_PTR CDataBase::DBSettingDelete(WPARAM pSettingDescriptor, LPARAM lParam)
{
	if (pSettingDescriptor == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->cbSize != sizeof(TDBTSettingDescriptor))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor)->pszSettingName == NULL)
		return DBT_INVALIDPARAM;

	return getSettings().DeleteSetting(*reinterpret_cast<PDBTSettingDescriptor>(pSettingDescriptor));
}
INT_PTR CDataBase::DBSettingDeleteHandle(WPARAM hSetting, LPARAM lParam)
{
	if (hSetting == 0)
		return DBT_INVALIDPARAM;

	return getSettings().DeleteSetting(hSetting);
}
INT_PTR CDataBase::DBSettingWrite(WPARAM pSetting, LPARAM lParam)
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

	return getSettings().WriteSetting(*reinterpret_cast<PDBTSetting>(pSetting));
}
INT_PTR CDataBase::DBSettingWriteHandle(WPARAM pSetting, LPARAM hSetting)
{
	if (pSetting == NULL)
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	return getSettings().WriteSetting(*reinterpret_cast<PDBTSetting>(pSetting), hSetting);
}
INT_PTR CDataBase::DBSettingRead(WPARAM pSetting, LPARAM lParam)
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

	return getSettings().ReadSetting(*reinterpret_cast<PDBTSetting>(pSetting));
}
INT_PTR CDataBase::DBSettingReadHandle(WPARAM pSetting, LPARAM hSetting)
{
	if ((pSetting == NULL) || (hSetting == 0))
		return DBT_INVALIDPARAM;

	if (reinterpret_cast<PDBTSetting>(pSetting)->cbSize != sizeof(TDBTSetting))
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTSetting>(pSetting)->Descriptor != NULL) && (reinterpret_cast<PDBTSetting>(pSetting)->Descriptor->cbSize != sizeof(TDBTSettingDescriptor)))
		return DBT_INVALIDPARAM;

	return getSettings().ReadSetting(*((PDBTSetting)pSetting), hSetting);
}
INT_PTR CDataBase::DBSettingIterInit(WPARAM pFilter, LPARAM lParam)
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

	return getSettings().IterationInit(*reinterpret_cast<PDBTSettingIterFilter>(pFilter));
}
INT_PTR CDataBase::DBSettingIterNext(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return getSettings().IterationNext(hIteration);
}
INT_PTR CDataBase::DBSettingIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return getSettings().IterationClose(hIteration);
}

INT_PTR CDataBase::DBEventGetBlobSize(WPARAM hEvent, LPARAM lParam)
{
	return getEvents().GetBlobSize(hEvent);
}

INT_PTR CDataBase::DBEventGet(WPARAM hEvent, LPARAM pEvent)
{
	if ((pEvent == NULL) || (reinterpret_cast<PDBTEvent>(pEvent)->cbSize != sizeof(TDBTEvent)))
		return DBT_INVALIDPARAM;

	return getEvents().Get(hEvent, *reinterpret_cast<PDBTEvent>(pEvent));
}

INT_PTR CDataBase::DBEventGetCount(WPARAM hEntity, LPARAM lParam)
{
	return getEvents().GetCount(hEntity);
}

INT_PTR CDataBase::DBEventDelete(WPARAM hEvent, LPARAM lParam)
{
	return getEvents().Delete(hEvent);
}

INT_PTR CDataBase::DBEventAdd(WPARAM hEntity, LPARAM pEvent)
{
	if ((pEvent == NULL) || (reinterpret_cast<PDBTEvent>(pEvent)->cbSize != sizeof(TDBTEvent)) || (reinterpret_cast<PDBTEvent>(pEvent)->pBlob == NULL) || (reinterpret_cast<PDBTEvent>(pEvent)->cbBlob == 0))
		return DBT_INVALIDPARAM;

	return getEvents().Add(hEntity, *((PDBTEvent)pEvent));
}

INT_PTR CDataBase::DBEventMarkRead(WPARAM hEvent, LPARAM lParam)
{
	return getEvents().MarkRead(hEvent);
}

INT_PTR CDataBase::DBEventWriteToDisk(WPARAM hEvent, LPARAM lParam)
{
	return getEvents().WriteToDisk(hEvent);
}

INT_PTR CDataBase::DBEventGetEntity(WPARAM hEvent, LPARAM lParam)
{
	return getEvents().getEntity(hEvent);
}

INT_PTR CDataBase::DBEventIterInit(WPARAM pFilter, LPARAM lParam)
{
	if ((pFilter == NULL) || (reinterpret_cast<PDBTEventIterFilter>(pFilter)->cbSize != sizeof(TDBTEventIterFilter)))
		return DBT_INVALIDPARAM;

	if ((reinterpret_cast<PDBTEventIterFilter>(pFilter)->Event != NULL) && (reinterpret_cast<PDBTEventIterFilter>(pFilter)->Event->cbSize != sizeof(TDBTEvent)))
		return DBT_INVALIDPARAM;

	return getEvents().IterationInit(*reinterpret_cast<PDBTEventIterFilter>(pFilter));
}

INT_PTR CDataBase::DBEventIterNext(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return getEvents().IterationNext(hIteration);
}

INT_PTR CDataBase::DBEventIterClose(WPARAM hIteration, LPARAM lParam)
{
	if ((hIteration == 0) || (hIteration == DBT_INVALIDPARAM))
		return hIteration;

	return getEvents().IterationClose(hIteration);
}

void CDataBase::CreateDbService(const char* szService, DbServiceFunc serviceProc)
{
	::CreateServiceFunctionObj(szService, ( MIRANDASERVICEOBJ )*( void** )&serviceProc, this );
}

bool CDataBase::RegisterServices()
{
	CreateDbService(MS_DBT_ENTITY_GETROOT, &CDataBase::DBEntityGetRoot);
	CreateDbService(MS_DBT_ENTITY_CHILDCOUNT,       &CDataBase::DBEntityChildCount);
	CreateDbService(MS_DBT_ENTITY_GETPARENT,        &CDataBase::DBEntityGetParent);
	CreateDbService(MS_DBT_ENTITY_MOVE,             &CDataBase::DBEntityMove);
	CreateDbService(MS_DBT_ENTITY_GETFLAGS,         &CDataBase::DBEntityGetFlags);
	CreateDbService(MS_DBT_ENTITY_ITER_INIT,        &CDataBase::DBEntityIterInit);
	CreateDbService(MS_DBT_ENTITY_ITER_NEXT,        &CDataBase::DBEntityIterNext);
	CreateDbService(MS_DBT_ENTITY_ITER_CLOSE,       &CDataBase::DBEntityIterClose);
	CreateDbService(MS_DBT_ENTITY_DELETE,           &CDataBase::DBEntityDelete);
	CreateDbService(MS_DBT_ENTITY_CREATE,           &CDataBase::DBEntityCreate);
	CreateDbService(MS_DBT_ENTITY_GETACCOUNT,       &CDataBase::DBEntityGetAccount);

	CreateDbService(MS_DBT_VIRTUALENTITY_CREATE,    &CDataBase::DBVirtualEntityCreate);
	CreateDbService(MS_DBT_VIRTUALENTITY_GETPARENT, &CDataBase::DBVirtualEntityGetParent);
	CreateDbService(MS_DBT_VIRTUALENTITY_GETFIRST,  &CDataBase::DBVirtualEntityGetFirst);
	CreateDbService(MS_DBT_VIRTUALENTITY_GETNEXT,   &CDataBase::DBVirtualEntityGetNext);

	CreateDbService(MS_DBT_SETTING_FIND,            &CDataBase::DBSettingFind);
	CreateDbService(MS_DBT_SETTING_DELETE,          &CDataBase::DBSettingDelete);
	CreateDbService(MS_DBT_SETTING_DELETEHANDLE,    &CDataBase::DBSettingDeleteHandle);
	CreateDbService(MS_DBT_SETTING_WRITE,           &CDataBase::DBSettingWrite);
	CreateDbService(MS_DBT_SETTING_WRITEHANDLE,     &CDataBase::DBSettingWriteHandle);
	CreateDbService(MS_DBT_SETTING_READ,            &CDataBase::DBSettingRead);
	CreateDbService(MS_DBT_SETTING_READHANDLE,      &CDataBase::DBSettingReadHandle);
	CreateDbService(MS_DBT_SETTING_ITER_INIT,       &CDataBase::DBSettingIterInit);
	CreateDbService(MS_DBT_SETTING_ITER_NEXT,       &CDataBase::DBSettingIterNext);
	CreateDbService(MS_DBT_SETTING_ITER_CLOSE,      &CDataBase::DBSettingIterClose);

	CreateDbService(MS_DBT_EVENT_GETBLOBSIZE,       &CDataBase::DBEventGetBlobSize);
	CreateDbService(MS_DBT_EVENT_GET,               &CDataBase::DBEventGet);
	CreateDbService(MS_DBT_EVENT_GETCOUNT,          &CDataBase::DBEventGetCount);
	CreateDbService(MS_DBT_EVENT_DELETE,            &CDataBase::DBEventDelete);
	CreateDbService(MS_DBT_EVENT_ADD,               &CDataBase::DBEventAdd);
	CreateDbService(MS_DBT_EVENT_MARKREAD,          &CDataBase::DBEventMarkRead);
	CreateDbService(MS_DBT_EVENT_WRITETODISK,       &CDataBase::DBEventWriteToDisk);
	CreateDbService(MS_DBT_EVENT_GETENTITY,         &CDataBase::DBEventGetEntity);
	CreateDbService(MS_DBT_EVENT_ITER_INIT,         &CDataBase::DBEventIterInit);
	CreateDbService(MS_DBT_EVENT_ITER_NEXT,         &CDataBase::DBEventIterNext);
	CreateDbService(MS_DBT_EVENT_ITER_CLOSE,        &CDataBase::DBEventIterClose);
	return true;
}
