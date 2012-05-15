/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"


/////////////////////////////////////////////////////////////////////////////////////////
// MSN_AddGroup - adds new server group to the list

void CMsnProto::MSN_AddGroup(const char* grpName, const char *grpId, bool init)
{
	ServerGroupItem* p = grpList.find((ServerGroupItem*)&grpId);
	if (p != NULL) return;

	p = (ServerGroupItem*)mir_alloc(sizeof(ServerGroupItem));
	p->id = mir_strdup(grpId);
	p->name = mir_strdup(grpName);
	
	grpList.insert(p);

	if (init)
	{
		TCHAR* szGroupName = mir_utf8decodeT(grpName);
		CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)szGroupName);
		mir_free(szGroupName);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_DeleteGroup - deletes a group from the list

void CMsnProto::MSN_DeleteGroup(const char* pId)
{
	int i = grpList.getIndex((ServerGroupItem*)&pId);
	if (i > -1) 
	{
		ServerGroupItem* p = grpList[i];
		mir_free(p->id);
		mir_free(p->name);
		mir_free(p);
		grpList.remove(i);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_DeleteServerGroup - deletes group from the server

void CMsnProto::MSN_DeleteServerGroup(LPCSTR szId)
{
	if (!MyOptions.ManageServer) return;

	MSN_ABAddDelContactGroup(NULL, szId, "ABGroupDelete");

	int count = -1;
	for (;;)
	{
		MsnContact *msc = Lists_GetNext(count);
		if (msc == NULL) break;

		char szGroupID[100];
		if (!getStaticString(msc->hContact, "GroupID", szGroupID, sizeof(szGroupID))) 
		{
			if (strcmp(szGroupID, szId) == 0)
				deleteSetting(msc->hContact, "GroupID");
		}
	}
	MSN_DeleteGroup(szId);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_FreeGroups - clears the server groups list

void CMsnProto::MSN_FreeGroups(void)
{
	for (int i=0; i < grpList.getCount(); i++) 
	{
		ServerGroupItem* p = grpList[i];
		mir_free(p->id);
		mir_free(p->name);
		mir_free (p);
	}
	grpList.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_GetGroupById - tries to return a group name associated with given UUID

LPCSTR CMsnProto::MSN_GetGroupById(const char* pId)
{
	ServerGroupItem* p = grpList.find((ServerGroupItem*)&pId);
	return p ? p->name : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_GetGroupByName - tries to return a group UUID associated with the given name 

LPCSTR CMsnProto::MSN_GetGroupByName(const char* pName)
{
	for (int i=0; i < grpList.getCount(); i++) 
	{
		const ServerGroupItem* p = grpList[i];
		if (strcmp(p->name, pName) == 0)
			return p->id;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_SetGroupName - sets a new name to a server group

void CMsnProto::MSN_SetGroupName(const char* pId, const char* pNewName)
{
	ServerGroupItem* p = grpList.find((ServerGroupItem*)&pId);
	if (p != NULL) replaceStr(p->name, pNewName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_MoveContactToGroup - sends a contact to the specified group 

void CMsnProto::MSN_MoveContactToGroup(HANDLE hContact, const char* grpName)
{
	if (!MyOptions.ManageServer) return;

	LPCSTR szId = NULL;
	char szContactID[100], szGroupID[100];
	if (getStaticString(hContact, "ID", szContactID, sizeof(szContactID)))
		return;

	if (getStaticString(hContact, "GroupID", szGroupID, sizeof(szGroupID)))
		szGroupID[0] = 0;

	bool bInsert = false, bDelete = szGroupID[0] != 0;

	if (grpName != NULL)
	{
		if (strcmp(grpName, "MetaContacts Hidden Group") == 0)
			return;

		szId = MSN_GetGroupByName(grpName);
		if (szId == NULL)
		{
			MSN_ABAddGroup(grpName);
			szId = MSN_GetGroupByName(grpName);
		}
		if (szId == NULL) return;
		if (_stricmp(szGroupID, szId) == 0) bDelete = false;
		else                                bInsert = true;
	}

	if (bDelete)
	{
		MSN_ABAddDelContactGroup(szContactID, szGroupID, "ABGroupContactDelete");
		deleteSetting(hContact, "GroupID");
	}

	if (bInsert)
	{
		MSN_ABAddDelContactGroup(szContactID, szId, "ABGroupContactAdd");
		setString(hContact, "GroupID", szId);
	}

	if (bDelete) MSN_RemoveEmptyGroups();
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_RemoveEmptyGroups - removes empty groups from the server list

void CMsnProto::MSN_RemoveEmptyGroups(void)
{
	if (!MyOptions.ManageServer) return;

	unsigned *cCount = (unsigned*)mir_calloc(grpList.getCount() * sizeof(unsigned));

	int count = -1;
	for (;;)
	{
		MsnContact *msc = Lists_GetNext(count);
		if (msc == NULL) break;

		char szGroupID[100];
		if (!getStaticString(msc->hContact, "GroupID", szGroupID, sizeof(szGroupID))) 
		{
			const char *pId = szGroupID;
			int i = grpList.getIndex((ServerGroupItem*)&pId);
			if (i > -1) ++cCount[i];
		}
	}

	for (int i=grpList.getCount(); i--;) 
	{
		if (cCount[i] == 0) MSN_DeleteServerGroup(grpList[i]->id);
	}
	mir_free(cCount);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_RenameServerGroup - renames group on the server

void CMsnProto::MSN_RenameServerGroup(LPCSTR szId, const char* newName)
{
	MSN_SetGroupName(szId, newName);
	MSN_ABRenameGroup(newName, szId);
}


/////////////////////////////////////////////////////////////////////////////////////////
// MSN_UploadServerGroups - adds a group to the server list and contacts into the group

void  CMsnProto::MSN_UploadServerGroups(char* group)
{
	if (!MyOptions.ManageServer) return;

	int count = -1;
	for (;;)
	{
		MsnContact *msc = Lists_GetNext(count);
		if (msc == NULL) break;

		DBVARIANT dbv;
		if (!DBGetContactSettingStringUtf(msc->hContact, "CList", "Group", &dbv)) 
		{
			char szGroupID[100];
			if (group == NULL || (strcmp(group, dbv.pszVal) == 0 &&
				getStaticString(msc->hContact, "GroupID", szGroupID, sizeof(szGroupID)) != 0)) 
			{
				MSN_MoveContactToGroup(msc->hContact, dbv.pszVal);
			}
			MSN_FreeVariant(&dbv);
		}	
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN_SyncContactToServerGroup - moves contact into appropriate group according to server
// if contact in multiple server groups it get removed from all of them other them it's
// in or the last one

void CMsnProto::MSN_SyncContactToServerGroup(HANDLE hContact, const char* szContId, ezxml_t cgrp)
{
	if (!MyOptions.ManageServer) return;

	const char* szGrpName = "";

	DBVARIANT dbv;
	if (!DBGetContactSettingStringUtf(hContact, "CList", "Group", &dbv))
	{
		if (strcmp(dbv.pszVal, "MetaContacts Hidden Group") == 0)
		{
			MSN_FreeVariant(&dbv);
			if (!DBGetContactSettingStringUtf(hContact, "MetaContacts", "OldCListGroup", &dbv))
			{
				szGrpName = NEWSTR_ALLOCA(dbv.pszVal);
				MSN_FreeVariant(&dbv);
			}
		}
		else
		{
			szGrpName = NEWSTR_ALLOCA(dbv.pszVal);
			MSN_FreeVariant(&dbv);
		}
	}

	const char* szGrpIdF = NULL;
	while(cgrp != NULL)
	{
		const char* szGrpId  = ezxml_txt(cgrp);
		cgrp = ezxml_next(cgrp);

		const char* szGrpNameById = MSN_GetGroupById(szGrpId);

		if (szGrpNameById && (strcmp(szGrpNameById, szGrpName) == 0 || 
			(cgrp == NULL && szGrpIdF == NULL))) 
			szGrpIdF = szGrpId;
		else 
			MSN_ABAddDelContactGroup(szContId, szGrpId, "ABGroupContactDelete");
	}

	if (szGrpIdF != NULL) 
	{
		setString(hContact, "GroupID", szGrpIdF);
		const char* szGrpNameById = MSN_GetGroupById(szGrpIdF);
		if (strcmp(szGrpNameById, szGrpName))
			DBWriteContactSettingStringUtf(hContact, "CList", "Group", szGrpNameById);
	}
	else 
	{
		if (szGrpName[0])
			DBDeleteContactSetting(hContact, "CList", "Group");
		deleteSetting(hContact, "GroupID");
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////
// Msn_SendNickname - update our own nickname on the server

void  CMsnProto::MSN_SendNicknameUtf(const char* nickname)
{
	if (nickname[0])
		setStringUtf(NULL, "Nick", nickname);
	else
		deleteSetting(NULL, "Nick");
	
	MSN_SetNicknameUtf(nickname[0] ? nickname : MyOptions.szEmail);

	ForkThread(&CMsnProto::msn_storeProfileThread, (void*)1);
}

void  CMsnProto::MSN_SetNicknameUtf(const char* nickname)
{
	const size_t urlNickSz = strlen(nickname) * 3 + 1;
	char* urlNick = (char*)alloca(urlNickSz);

	UrlEncode(nickname, urlNick, urlNickSz);
	msnNsThread->sendPacket("PRP", "MFN %s", urlNick);
}

/////////////////////////////////////////////////////////////////////////////////////////
// msn_storeAvatarThread - update our own avatar on the server

void CMsnProto::msn_storeAvatarThread(void* arg)
{
	StoreAvatarData* dat = (StoreAvatarData*)arg;
	char *szEncBuf = NULL;

	if (dat)
	{
		size_t szEncPngSize = Netlib_GetBase64EncodedBufferSize(dat->dataSize);
		szEncBuf = (char*)mir_alloc(szEncPngSize);
		NETLIBBASE64 nlb = { szEncBuf, (int)szEncPngSize, dat->data, (int)dat->dataSize };
		MSN_CallService(MS_NETLIB_BASE64ENCODE, 0, LPARAM(&nlb));
	}
 
	if (photoid[0] && dat)
	{
		MSN_StoreUpdateDocument(dat->szName, dat->szMimeType, szEncBuf);
	}
	else
	{
		MSN_StoreUpdateProfile(NULL, NULL, 1);

		if (photoid[0])
		{
			MSN_StoreDeleteRelationships(true);
			MSN_StoreDeleteRelationships(false);
			photoid[0] = 0;
		}

		if (dat)
		{
			MSN_StoreCreateDocument(dat->szName, dat->szMimeType, szEncBuf);
			MSN_StoreCreateRelationships();
		}

		MSN_StoreUpdateProfile(NULL, NULL, 0);
	}

	MSN_ABUpdateDynamicItem();

	if (dat)
	{
		mir_free(szEncBuf);
		mir_free(dat->szName);
		mir_free(dat->data);
		mir_free(dat);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// msn_storeProfileThread - update our own avatar on the server

void CMsnProto::msn_storeProfileThread(void* param)
{
	DBVARIANT dbv;
	char *szNick = NULL;
	bool needFree = false;
	if (!getStringUtf("Nick", &dbv))
	{
		szNick = dbv.pszVal[0] ? dbv.pszVal : NULL;
		needFree = true;
	}

	char** msgptr = GetStatusMsgLoc(m_iStatus);
	char *szStatus = msgptr ? *msgptr : NULL;

 	if (param || (msnLastStatusMsg != szStatus && 
		(msnLastStatusMsg && szStatus && strcmp(msnLastStatusMsg, szStatus))))
	{

		if (MSN_StoreUpdateProfile(szNick, szStatus, false))
			MSN_ABUpdateDynamicItem();
	//	MSN_ABUpdateNick(nickname, NULL);
	}

	if (needFree) MSN_FreeVariant(&dbv);
}
