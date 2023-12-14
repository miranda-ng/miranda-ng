/*
Miranda SmileyAdd Plugin
Copyright (C) 2004-2005 Rein-Peter de Boer (peacow) and followers

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

//imagedataobject
//code taken partly from public example on the internet, source unknown.

#include "stdafx.h"
#include "ImageDataObjectHlp.h"
#include "dataobject.h"

// cache up to 1000 objects
#define MAX_CACHE_SIZE 2000

struct EMFCACHE
{
	CEMFObject *pEmfObj;
	IStorage *pStorage;
	IOleObject *pOleObject;
	HICON hIcon;
	HWND hwnd;
	EMFCACHE *prev;
	EMFCACHE *next;
} *emfCache = nullptr;
int emfCacheSize = 0;
mir_cs csEmfCache;

void UnloadEmfCache()
{
	while (emfCache) {
		EMFCACHE *tmp = emfCache->next;
		emfCache->pEmfObj->Release();
		emfCache->pStorage->Release();
		delete emfCache;
		emfCache = tmp;
	}
}

void CleanupEmfCache(HWND hwnd)
{
	EMFCACHE *p = emfCache;
	while (p) {
		if (p->hwnd == hwnd) {
			EMFCACHE *tmp = p;
			if (p->prev) p->prev->next = p->next;
			if (p->next) p->next->prev = p->prev;
			p = p->next;
			if (tmp == emfCache) emfCache = p;
			tmp->pEmfObj->Release();
			tmp->pOleObject->Release();
			tmp->pStorage->Release();
			delete tmp;
			emfCacheSize--;
		}
		else {
			p = p->next;
		}
	}
}

IDataObject *CacheIconToIDataObject(HWND hwnd, HICON hIcon, IOleClientSite *pOleClientSite, IStorage **ppStorage, IOleObject **ppOleObject)
{
	IDataObject *result = nullptr;
	static const FORMATETC lc_format[] =
	{
		{ CF_ENHMETAFILE, nullptr, DVASPECT_CONTENT, -1, TYMED_ENHMF }
	};

	mir_cslock lck(csEmfCache);
	for (EMFCACHE *p = emfCache; p; p = p->next)
		if (p->hIcon == hIcon && p->hwnd == hwnd) {
			if (p->prev) p->prev->next = p->next;
			if (p->next) p->next->prev = p->prev;
			p->prev = nullptr;
			emfCache->prev = p;
			p->next = emfCache;
			emfCache = p;
			result = emfCache->pEmfObj;
			*ppStorage = emfCache->pStorage;
			*ppOleObject = emfCache->pOleObject;
			break;
		}

	// cache new item
	if (!result) {
		EMFCACHE *newItem = new EMFCACHE;

		LPLOCKBYTES lpLockBytes = nullptr;
		SCODE sc = CreateILockBytesOnHGlobal(nullptr, TRUE, &lpLockBytes);
		if (sc != S_OK) {
			delete newItem;
			return nullptr;
		}

		sc = StgCreateDocfileOnILockBytes(lpLockBytes,
			STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &newItem->pStorage);

		lpLockBytes->Release();

		if (sc != S_OK) {
			delete newItem;
			return nullptr;
		}

		HDC emfdc = CreateEnhMetaFile(nullptr, nullptr, nullptr, L"icon");
		DrawIconEx(emfdc, 0, 0, (HICON)hIcon, 16, 16, 0, nullptr, DI_NORMAL);
		newItem->pEmfObj = new CEMFObject(CloseEnhMetaFile(emfdc));
		newItem->hIcon = hIcon;
		newItem->hwnd = hwnd;

		sc = OleCreateStaticFromData(newItem->pEmfObj, IID_IOleObject, OLERENDER_FORMAT,
			(LPFORMATETC)lc_format, pOleClientSite, newItem->pStorage, (void **)&newItem->pOleObject);

		if (sc != S_OK) {
			newItem->pStorage->Release();
			newItem->pEmfObj->Release();
			delete newItem;
			return nullptr;
		}

		OleSetContainedObject(newItem->pOleObject, TRUE);

		newItem->prev = nullptr;
		newItem->next = emfCache;
		if (emfCache) emfCache->prev = newItem;
		emfCache = newItem;
		emfCacheSize++;

		result = emfCache->pEmfObj;
		*ppStorage = emfCache->pStorage;
		*ppOleObject = emfCache->pOleObject;
	}

	// tail cutoff - cache up to MAX_CACHE_SIZE data sources (smaller than 1/2 of 10000 GDI objects)
	if (emfCacheSize > MAX_CACHE_SIZE) {
		int n = 0;
		EMFCACHE *p;
		for (p = emfCache; p; p = p->next)
			if (++n > MAX_CACHE_SIZE)
				break;
		
		while (p->next) {
			EMFCACHE *tmp = p->next;
			p->next = p->next->next;
			tmp->pEmfObj->Release();
			tmp->pStorage->Release();
			delete tmp;
		}
		if (p->next)
			p->next->prev = p;
		emfCacheSize = MAX_CACHE_SIZE;
	}

	return result;
}

// returns true on success, false on failure
bool InsertBitmap(HWND hwnd, IRichEditOle *pRichEditOle, HICON hIcon)
{
	// Get the RichEdit container site
	IOleClientSite *pOleClientSite;
	pRichEditOle->GetClientSite(&pOleClientSite);

	IStorage *pStorage;
	IOleObject *pOleObject;
	IDataObject *pods = CacheIconToIDataObject(hwnd, hIcon, pOleClientSite, &pStorage, &pOleObject);
	if (pods == nullptr) {
		pOleClientSite->Release();
		return false;
	}

	// all items are "contained" -- this makes our reference to this object
	//  weak -- which is needed for links to embedding silent update.

	// Now Add the object to the RichEdit 
	REOBJECT reobject = { 0 };

	reobject.cbStruct = sizeof(REOBJECT);
	reobject.cp = REO_CP_SELECTION;
	reobject.dvaspect = DVASPECT_CONTENT;
	reobject.poleobj = pOleObject;
	reobject.polesite = pOleClientSite;
	reobject.pstg = pStorage;
	reobject.dwFlags = REO_BELOWBASELINE;

	SCODE sc = pOleObject->GetUserClassID(&reobject.clsid);
	if (sc != S_OK)
		return false;

	// Insert the bitmap at the current location in the richedit control
	sc = pRichEditOle->InsertObject(&reobject);

	return sc == S_OK;
}
