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

#include "commonheaders.h"

static int g_maxBlock = 0, g_curBlock = 0;
static CacheNode **g_cacheBlocks = NULL;

static CacheNode *g_Cache = 0;
static CRITICAL_SECTION cachecs, alloccs;

/*
 * allocate a cache block and add it to the list of blocks
 * does not link the new block with the old block(s) - caller needs to do this
 */

static CacheNode *AllocCacheBlock()
{
	CacheNode *allocedBlock = (CacheNode*)malloc(CACHE_BLOCKSIZE * sizeof(struct CacheNode));
	ZeroMemory((void *)allocedBlock, sizeof(struct CacheNode) * CACHE_BLOCKSIZE);

	for (int i = 0; i < CACHE_BLOCKSIZE - 1; i++)
		allocedBlock[i].pNextNode = &allocedBlock[i + 1];				// pre-link the alloced block

	if (g_Cache == NULL)													// first time only...
		g_Cache = allocedBlock;

	// add it to the list of blocks

	if (g_curBlock == g_maxBlock) {
		g_maxBlock += 10;
		g_cacheBlocks = (CacheNode **)realloc(g_cacheBlocks, g_maxBlock * sizeof(CacheNode*));
	}
	g_cacheBlocks[g_curBlock++] = allocedBlock;

	return(allocedBlock);
}

void InitCache(void)
{
	InitializeCriticalSection(&cachecs);
	InitializeCriticalSection(&alloccs);
	AllocCacheBlock();
}

void UnloadCache(void)
{
	for (CacheNode *cc = g_Cache; cc; cc = cc->pNextNode)
		if (cc->ace.hbmPic != 0)
			DeleteObject(cc->ace.hbmPic);

	for (int i = 0; i < g_curBlock; i++)
		free(g_cacheBlocks[i]);
	free(g_cacheBlocks);

	DeleteCriticalSection(&alloccs);
	DeleteCriticalSection(&cachecs);
}

/*
 * link a new cache block with the already existing chain of blocks
 */

static CacheNode* AddToList(CacheNode *node)
{
	CacheNode *pCurrent = g_Cache;

	while(pCurrent->pNextNode != 0)
		pCurrent = pCurrent->pNextNode;

	pCurrent->pNextNode = node;
	return pCurrent;
}

CacheNode *FindAvatarInCache(MCONTACT hContact, BOOL add, BOOL findAny)
{
	if (g_shutDown)
		return NULL;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL || !db_get_b(NULL, AVS_MODULE, szProto, 1))
		return NULL;

	mir_cslock lck(cachecs);

	CacheNode *cc = g_Cache, *foundNode = NULL;
	while(cc) {
		if (cc->ace.hContact == hContact) {
			cc->ace.t_lastAccess = time(NULL);
			foundNode = cc->loaded || findAny ? cc : NULL;
			return foundNode;
		}

		// found an empty and usable node
		if (foundNode == NULL && cc->ace.hContact == 0)
			foundNode = cc;

		cc = cc->pNextNode;
	}

	// not found
	if (!add)
		return NULL;

	if (foundNode == NULL) {        // no free entry found, create a new and append it to the list
		mir_cslock all(alloccs);     // protect memory block allocation
		CacheNode *newNode = AllocCacheBlock();
		AddToList(newNode);
		foundNode = newNode;
	}

	foundNode->ace.hContact = hContact;
	foundNode->dwFlags |= (db_mc_isSub(hContact) ? MC_ISSUBCONTACT : 0);
	foundNode->loaded = FALSE;
	foundNode->mustLoad = 1;        // pic loader will watch this and load images
	SetEvent(hLoaderEvent);         // wake him up
	return NULL;
}

/*
 * output a notification message.
 * may accept a hContact to include the contacts nickname in the notification message...
 * the actual message is using printf() rules for formatting and passing the arguments...
 *
 * can display the message either as systray notification (baloon popup) or using the
 * popup plugin.
 */

void NotifyMetaAware(MCONTACT hContact, CacheNode *node = NULL, AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)-1)
{
	if (g_shutDown)
		return;

	if (ace == (AVATARCACHEENTRY *)-1)
		ace = &node->ace;

	NotifyEventHooks(hEventChanged, hContact, (LPARAM)ace);

	if ((node->dwFlags & MC_ISSUBCONTACT) && db_get_b(NULL, META_PROTO, "Enabled", 0)) {
		MCONTACT hMasterContact = db_mc_getMeta(hContact);
		if (hMasterContact && db_mc_getMostOnline(hMasterContact) == hContact && !db_get_b(hMasterContact, "ContactPhoto", "Locked", 0))
			NotifyEventHooks(hEventChanged, (WPARAM)hMasterContact, (LPARAM)ace);
	}

	if (node->dwFlags & AVH_MUSTNOTIFY) {
		// Fire the event for avatar history
		node->dwFlags &= ~AVH_MUSTNOTIFY;
		if (node->ace.szFilename[0] != '\0') {
			CONTACTAVATARCHANGEDNOTIFICATION cacn = {0};
			cacn.cbSize = sizeof(CONTACTAVATARCHANGEDNOTIFICATION);
			cacn.hContact = hContact;
			cacn.format = node->pa_format;
			_tcsncpy(cacn.filename, node->ace.szFilename, MAX_PATH);
			cacn.filename[MAX_PATH - 1] = 0;

			// Get hash
			char *szProto = GetContactProto(hContact);
			if (szProto != NULL) {
				DBVARIANT dbv = {0};
				if ( !db_get_s(hContact, szProto, "AvatarHash", &dbv)) {
					if (dbv.type == DBVT_TCHAR)
						_tcsncpy_s(cacn.hash, SIZEOF(cacn.hash), dbv.ptszVal, _TRUNCATE);
					else if (dbv.type == DBVT_BLOB) {
						ptrA szHash( mir_base64_encode(dbv.pbVal, dbv.cpbVal));
						_tcsncpy_s(cacn.hash, SIZEOF(cacn.hash), _A2T(szHash), _TRUNCATE);
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
void DeleteAvatarFromCache(MCONTACT hContact, BOOL forever)
{
	if (g_shutDown)
		return;

	hContact = GetContactThatHaveTheAvatar(hContact);
	CacheNode *node = FindAvatarInCache(hContact, FALSE);
	if (node == NULL) {
		struct CacheNode temp_node = {0};
		temp_node.dwFlags |= (db_mc_isSub(hContact) ? MC_ISSUBCONTACT : 0);
		NotifyMetaAware(hContact, &temp_node, (AVATARCACHEENTRY *)GetProtoDefaultAvatar(hContact));
		return;
	}
	node->mustLoad = -1;                        // mark for deletion
	if (forever)
		node->dwFlags |= AVS_DELETENODEFOREVER;
	SetEvent(hLoaderEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////

int SetAvatarAttribute(MCONTACT hContact, DWORD attrib, int mode)
{
	if (g_shutDown)
		return 0;

	for (CacheNode *cc = g_Cache; cc; cc = cc->pNextNode) {
		if (cc->ace.hContact != hContact)
			continue;

		DWORD dwFlags = cc->ace.dwFlags;
		cc->ace.dwFlags = mode ? (cc->ace.dwFlags | attrib) : (cc->ace.dwFlags & ~attrib);
		if (cc->ace.dwFlags != dwFlags)
			NotifyMetaAware(hContact, cc);
		break;
	}
	return 0;
}

/*
 * this thread scans the cache and handles nodes which have mustLoad set to > 0 (must be loaded/reloaded) or
 * nodes where mustLoad is < 0 (must be deleted).
 * its waken up by the event and tries to lock the cache only when absolutely necessary.
 */

void PicLoader(LPVOID param)
{
	DWORD dwDelay = db_get_dw(NULL, AVS_MODULE, "picloader_sleeptime", 80);

	if (dwDelay < 30)
		dwDelay = 30;
	else if (dwDelay > 100)
		dwDelay = 100;

	while(!g_shutDown) {
		CacheNode *node = g_Cache;

		while(!g_shutDown && node) {
			if (node->mustLoad > 0 && node->ace.hContact) {
				node->mustLoad = 0;
				AVATARCACHEENTRY ace_temp;

				if (db_get_b(node->ace.hContact, "ContactPhoto", "NeedUpdate", 0))
					QueueAdd(node->ace.hContact);

				CopyMemory(&ace_temp, &node->ace, sizeof(AVATARCACHEENTRY));
				ace_temp.hbmPic = 0;

				int result = CreateAvatarInCache(node->ace.hContact, &ace_temp, NULL);
				if (result == -2) {
					char *szProto = GetContactProto(node->ace.hContact);
					if (szProto == NULL || Proto_NeedDelaysForAvatars(szProto))
						QueueAdd(node->ace.hContact);
					else if (FetchAvatarFor(node->ace.hContact, szProto) == GAIR_SUCCESS) // Try to create again
						result = CreateAvatarInCache(node->ace.hContact, &ace_temp, NULL);
				}

				if (result == 1 && ace_temp.hbmPic != 0) { // Loaded
					HBITMAP oldPic = node->ace.hbmPic;

					EnterCriticalSection(&cachecs);
					CopyMemory(&node->ace, &ace_temp, sizeof(AVATARCACHEENTRY));
					node->loaded = TRUE;
					LeaveCriticalSection(&cachecs);
					if (oldPic)
						DeleteObject(oldPic);
					NotifyMetaAware(node->ace.hContact, node);
				}
				else if (result == 0 || result == -3) { // Has no avatar
					HBITMAP oldPic = node->ace.hbmPic;

					EnterCriticalSection(&cachecs);
					CopyMemory(&node->ace, &ace_temp, sizeof(AVATARCACHEENTRY));
					node->loaded = FALSE;
					node->mustLoad = 0;
					LeaveCriticalSection(&cachecs);
					if (oldPic)
						DeleteObject(oldPic);
					NotifyMetaAware(node->ace.hContact, node);
				}

				mir_sleep(dwDelay);
			}
			else if (node->mustLoad < 0 && node->ace.hContact) {         // delete this picture
				MCONTACT hContact = node->ace.hContact;
				EnterCriticalSection(&cachecs);
				node->mustLoad = 0;
				node->loaded = 0;
				if (node->ace.hbmPic)
					DeleteObject(node->ace.hbmPic);
				ZeroMemory(&node->ace, sizeof(AVATARCACHEENTRY));
				if (node->dwFlags & AVS_DELETENODEFOREVER)
					node->dwFlags &= ~AVS_DELETENODEFOREVER;
				else
					node->ace.hContact = hContact;

				LeaveCriticalSection(&cachecs);
				NotifyMetaAware(hContact, node, (AVATARCACHEENTRY *)GetProtoDefaultAvatar(hContact));
			}
			// protect this by changes from the cache block allocator as it can cause inconsistencies while working
			// on allocating a new block.
			EnterCriticalSection(&alloccs);
			node = node->pNextNode;
			LeaveCriticalSection(&alloccs);
		}
		WaitForSingleObject(hLoaderEvent, INFINITE);
		//_DebugTrace(0, "pic loader awake...");
		ResetEvent(hLoaderEvent);
	}
}
