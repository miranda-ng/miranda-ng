/*
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
	if (hbmPic != 0)
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
		return NULL;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL || !db_get_b(NULL, AVS_MODULE, szProto, 1))
		return NULL;

	avatarCacheEntry tmp;
	tmp.hContact = hContact;

	mir_cslock lck(cachecs);

	CacheNode *cc = arCache.find((CacheNode*)&tmp);
	if (cc) {
		cc->t_lastAccess = time(NULL);
		return (cc->loaded || findAny) ? cc : NULL;
	}

	// not found
	if (!add)
		return NULL;

	cc = new CacheNode();
	cc->hContact = hContact;
	arCache.insert(cc);

	PushAvatarRequest(cc);

	SetEvent(hLoaderEvent);    // wake him up
	return NULL;
}

// output a notification message.
// may accept a hContact to include the contacts nickname in the notification message...
// the actual message is using printf() rules for formatting and passing the arguments...
//
// can display the message either as systray notification (baloon popup) or using the
// popup plugin.

void NotifyMetaAware(MCONTACT hContact, CacheNode *node = NULL, AVATARCACHEENTRY *ace = (AVATARCACHEENTRY*)-1)
{
	if (g_shutDown)
		return;

	if (ace == (AVATARCACHEENTRY*)-1)
		ace = node;

	NotifyEventHooks(hEventChanged, hContact, (LPARAM)ace);

	MCONTACT hMasterContact = db_mc_getMeta(hContact);
	if (hMasterContact && db_mc_getMostOnline(hMasterContact) == hContact && !db_get_b(hMasterContact, "ContactPhoto", "Locked", 0))
		NotifyEventHooks(hEventChanged, (WPARAM)hMasterContact, (LPARAM)ace);

	if (node->dwFlags & AVH_MUSTNOTIFY) {
		// Fire the event for avatar history
		node->dwFlags &= ~AVH_MUSTNOTIFY;
		if (node->szFilename[0] != '\0') {
			CONTACTAVATARCHANGEDNOTIFICATION cacn = { 0 };
			cacn.cbSize = sizeof(CONTACTAVATARCHANGEDNOTIFICATION);
			cacn.hContact = hContact;
			cacn.format = node->pa_format;
			_tcsncpy(cacn.filename, node->szFilename, MAX_PATH);
			cacn.filename[MAX_PATH - 1] = 0;

			// Get hash
			char *szProto = GetContactProto(hContact);
			if (szProto != NULL) {
				DBVARIANT dbv = { 0 };
				if (!db_get_s(hContact, szProto, "AvatarHash", &dbv)) {
					if (dbv.type == DBVT_TCHAR)
						_tcsncpy_s(cacn.hash, dbv.ptszVal, _TRUNCATE);
					else if (dbv.type == DBVT_BLOB) {
						ptrA szHash(mir_base64_encode(dbv.pbVal, dbv.cpbVal));
						_tcsncpy_s(cacn.hash, _A2T(szHash), _TRUNCATE);
					}
					db_free(&dbv);
				}
			}

			// Default value
			if (cacn.hash[0] == '\0')
				mir_sntprintf(cacn.hash, SIZEOF(cacn.hash), _T("AVS-HASH-%x"), GetFileHash(cacn.filename));

			NotifyEventHooks(hEventContactAvatarChanged, (WPARAM)cacn.hContact, (LPARAM)&cacn);
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

	avatarCacheEntry tmp;
	tmp.hContact = GetContactThatHaveTheAvatar(hContact);

	mir_cslock lck(cachecs);
	int idx = arCache.getIndex((CacheNode*)&tmp);
	if (idx == -1) {
		CacheNode temp_node;
		memset(&temp_node, 0, sizeof(temp_node));
		NotifyMetaAware(hContact, &temp_node, (AVATARCACHEENTRY *)GetProtoDefaultAvatar(hContact));
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

int SetAvatarAttribute(MCONTACT hContact, DWORD attrib, int mode)
{
	if (g_shutDown)
		return 0;

	avatarCacheEntry tmp;
	tmp.hContact = hContact;

	mir_cslock lck(cachecs);
	CacheNode *cc = arCache.find((CacheNode*)&tmp);
	if (cc != NULL) {
		DWORD dwFlags = cc->dwFlags;
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
	DWORD dwDelay = db_get_dw(NULL, AVS_MODULE, "picloader_sleeptime", 80);

	if (dwDelay < 30)
		dwDelay = 30;
	else if (dwDelay > 100)
		dwDelay = 100;

	while (!g_shutDown) {
		while (!g_shutDown) {
			CacheNode *node;
			{
				mir_cslock all(alloccs);
				node = arQueue[0];
				if (node)
					arQueue.remove(0);
			}
			if (node == NULL)
				break;

			if (db_get_b(node->hContact, "ContactPhoto", "NeedUpdate", 0))
				QueueAdd(node->hContact);

			AVATARCACHEENTRY ace_temp;
			memcpy(&ace_temp, node, sizeof(AVATARCACHEENTRY));
			ace_temp.hbmPic = 0;

			int result = CreateAvatarInCache(node->hContact, &ace_temp, NULL);
			if (result == -2) {
				char *szProto = GetContactProto(node->hContact);
				if (szProto == NULL || Proto_NeedDelaysForAvatars(szProto))
					QueueAdd(node->hContact);
				else if (FetchAvatarFor(node->hContact, szProto) == GAIR_SUCCESS) // Try to create again
					result = CreateAvatarInCache(node->hContact, &ace_temp, NULL);
			}

			if (result == 1 && ace_temp.hbmPic != 0) { // Loaded
				HBITMAP oldPic = node->hbmPic;
				{
					mir_cslock l(cachecs);
					memcpy(node, &ace_temp, sizeof(AVATARCACHEENTRY));
					node->loaded = TRUE;
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
					node->loaded = FALSE;
				}
				if (oldPic)
					DeleteObject(oldPic);
				NotifyMetaAware(node->hContact, node);
			}

			mir_sleep(dwDelay);
		}
		WaitForSingleObject(hLoaderEvent, INFINITE);
		ResetEvent(hLoaderEvent);
	}
}
