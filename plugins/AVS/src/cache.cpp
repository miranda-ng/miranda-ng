/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (C) 2006 Ricardo Pescuma Domenecci, Nightwish

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

CacheNode::CacheNode()
{
}

CacheNode::~CacheNode()
{
	if (hbmPic != nullptr)
		DeleteObject(hbmPic);
}

void CacheNode::wipeInfo()
{
	MCONTACT saveContact = hContact;
	if (hbmPic)
		DeleteObject(hbmPic);
	memset(this, 0, sizeof(CacheNode));
	hContact = saveContact;
}

static int CompareNodes(const CacheNode *p1, const CacheNode *p2)
{
	return INT_PTR(p1->hContact) - INT_PTR(p2->hContact);
}

static OBJLIST<CacheNode> arCache(100, CompareNodes);
static LIST<CacheNode> arQueue(10);
static mir_cs alloccs, cachecs;

// allocate a cache block and add it to the list of blocks
// does not link the new block with the old block(s) - caller needs to do this

void UnloadCache(void)
{
	arCache.destroy();
}

void PushAvatarRequest(CacheNode *cc)
{
	mir_cslock lck(alloccs);
	int idx = arQueue.getCount();
	if (idx > 0)
		idx--;
	arQueue.insert(cc, idx);
}

// link a new cache block with the already existing chain of blocks

CacheNode* FindAvatarInCache(MCONTACT hContact, bool add, bool findAny)
{
	if (g_shutDown)
		return nullptr;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr || !g_plugin.getByte(szProto, 1))
		return nullptr;

	CacheNode *cc;
	{
		mir_cslock lck(cachecs);
		cc = arCache.find((CacheNode*)&hContact);
		if (cc) {
			cc->t_lastAccess = time(0);
			return (cc->bLoaded || findAny) ? cc : nullptr;
		}

		// not found
		if (!add)
			return nullptr;

		cc = new CacheNode();
		cc->hContact = hContact;
		arCache.insert(cc);
	}

	switch (CreateAvatarInCache(hContact, cc, nullptr)) {
	case -2:  // no avatar data in settings, retrieve
		PushAvatarRequest(cc);
		SetEvent(hLoaderEvent);    // wake him up
		break;
			
	case 1: // loaded, everything is ok
		if (cc->hbmPic != nullptr)
			cc->bLoaded = true;
		break;
		
	default:
		cc->bLoaded = false;
		break;
	}

	return cc;
}

// output a notification message.
// may accept a hContact to include the contacts nickname in the notification message...
// the actual message is using printf() rules for formatting and passing the arguments...
//
// can display the message either as systray notification (baloon popup) or using the
// popup plugin.

void NotifyMetaAware(MCONTACT hContact, CacheNode *node, AVATARCACHEENTRY *ace)
{
	if (g_shutDown)
		return;

	if (ace == (AVATARCACHEENTRY*)-1)
		ace = node;

	NotifyEventHooks(hEventChanged, hContact, (LPARAM)ace);

	MCONTACT hMasterContact = db_mc_getMeta(hContact);
	if (hMasterContact && db_mc_getMostOnline(hMasterContact) == hContact && !db_get_b(hMasterContact, "ContactPhoto", "Locked", 0))
		NotifyEventHooks(hEventChanged, (WPARAM)hMasterContact, (LPARAM)ace);

	if (node->bNotify) {
		// Fire the event for avatar history
		node->bNotify = false;

		if (node->szFilename[0] != '\0') {
			CONTACTAVATARCHANGEDNOTIFICATION cacn = {};
			cacn.hContact = hContact;
			cacn.format = node->pa_format;
			wcsncpy_s(cacn.filename, node->szFilename, _TRUNCATE);

			// Get hash
			char *szProto = Proto_GetBaseAccountName(hContact);
			if (szProto != nullptr) {
				DBVARIANT dbv = { 0 };
				if (!db_get_s(hContact, szProto, "AvatarHash", &dbv)) {
					if (dbv.type == DBVT_WCHAR)
						wcsncpy_s(cacn.hash, dbv.pwszVal, _TRUNCATE);
					else if (dbv.type == DBVT_ASCIIZ)
						wcsncpy_s(cacn.hash, _A2T(dbv.pszVal), _TRUNCATE);
					else if (dbv.type == DBVT_BLOB) {
						ptrA szHash(mir_base64_encode(dbv.pbVal, dbv.cpbVal));
						wcsncpy_s(cacn.hash, _A2T(szHash), _TRUNCATE);
					}
					db_free(&dbv);
				}
			}

			// Default value
			if (cacn.hash[0] == '\0')
				mir_snwprintf(cacn.hash, L"AVS-HASH-%x", GetFileHash(cacn.filename));

			NotifyEventHooks(hEventContactAvatarChanged, hContact, (LPARAM)&cacn);
		}
		else NotifyEventHooks(hEventContactAvatarChanged, hContact, NULL);
	}
}

// Just delete an avatar from cache
// An cache entry is never deleted. What is deleted is the image handle inside it
// This is done this way to keep track of which avatars avs have to keep track

void DeleteAvatarFromCache(MCONTACT hContact, bool bForever)
{
	if (g_shutDown)
		return;

	MCONTACT tmp = GetContactThatHaveTheAvatar(hContact);

	mir_cslock lck(cachecs);
	int idx = arCache.getIndex((CacheNode*)&tmp);
	if (idx == -1) {
		CacheNode temp_node;
		memset(&temp_node, 0, sizeof(temp_node));
		NotifyMetaAware(hContact, &temp_node, (AVATARCACHEENTRY*)GetProtoDefaultAvatar(hContact));
	}
	else {
		NotifyMetaAware(hContact, &arCache[idx], (AVATARCACHEENTRY*)GetProtoDefaultAvatar(hContact));
		if (!bForever)
			arCache[idx].wipeInfo();
		else
			arCache.remove(idx);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int SetAvatarAttribute(MCONTACT hContact, uint32_t attrib, int mode)
{
	if (g_shutDown)
		return 0;

	mir_cslock lck(cachecs);
	CacheNode *cc = arCache.find((CacheNode*)&hContact);
	if (cc != nullptr) {
		uint32_t dwFlags = cc->dwFlags;
		cc->dwFlags = mode ? (cc->dwFlags | attrib) : (cc->dwFlags & ~attrib);
		if (cc->dwFlags != dwFlags)
			NotifyMetaAware(hContact, cc);
	}
	return 0;
}

// this thread scans the cache and handles nodes which have mustLoad set to > 0 (must be loaded/reloaded) or
// nodes where mustLoad is < 0 (must be deleted).
// its waken up by the event and tries to lock the cache only when absolutely necessary.

void PicLoader(LPVOID)
{
	Thread_SetName("AVS: PicLoader");

	uint32_t dwDelay = g_plugin.getDword("picloader_sleeptime", 80);

	if (dwDelay < 30)
		dwDelay = 30;
	else if (dwDelay > 100)
		dwDelay = 100;

	while (!g_shutDown) {
		CacheNode *node;
		{
			mir_cslock all(alloccs);
			node = arQueue[0];
			if (node)
				arQueue.remove(0);
		}

		if (node == nullptr) {
			WaitForSingleObject(hLoaderEvent, INFINITE);
			ResetEvent(hLoaderEvent);
			continue;
		}

		if (db_get_b(node->hContact, "ContactPhoto", "NeedUpdate", 0))
			QueueAdd(node->hContact);

		AVATARCACHEENTRY ace_temp;
		memcpy(&ace_temp, node, sizeof(AVATARCACHEENTRY));
		ace_temp.hbmPic = nullptr;

		int result = CreateAvatarInCache(node->hContact, &ace_temp, nullptr);
		if (result == -2) {
			char *szProto = Proto_GetBaseAccountName(node->hContact);
			if (szProto == nullptr || Proto_NeedDelaysForAvatars(szProto))
				QueueAdd(node->hContact);
			else if (FetchAvatarFor(node->hContact, szProto) == GAIR_SUCCESS) // Try to create again
				result = CreateAvatarInCache(node->hContact, &ace_temp, nullptr);
		}

		if (result == 1 && ace_temp.hbmPic != nullptr) { // Loaded
			HBITMAP oldPic = node->hbmPic;
			{
				mir_cslock l(cachecs);
				memcpy(node, &ace_temp, sizeof(AVATARCACHEENTRY));
				node->bLoaded = true;
			}
			if (oldPic)
				DeleteObject(oldPic);
			NotifyMetaAware(node->hContact, node);
		}
		else if (result == 0 || result == -3) { // Has no avatar
			HBITMAP oldPic = node->hbmPic;
			{
				mir_cslock l(cachecs);
				memcpy(node, &ace_temp, sizeof(AVATARCACHEENTRY));
				node->bLoaded = false;
			}
			if (oldPic)
				DeleteObject(oldPic);
			NotifyMetaAware(node->hContact, node);
		}

		mir_sleep(dwDelay);
	}
}
