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
	HWND hwnd;
	HICON hIcon;
	CEMFObject *pEmfObj = 0;
	IStorage *pStorage = 0;
	IOleObject *pOleObject = 0;

	~EMFCACHE()
	{
		if (pEmfObj)
			pEmfObj->Release();
		if (pStorage)
			pStorage->Release();
		if (pOleObject)
			pOleObject->Release();
	}
};

mir_cs csEmfCache;
static OBJLIST<EMFCACHE> g_arCache(50);

void UnloadEmfCache()
{
	g_arCache.destroy();
}

void CleanupEmfCache(HWND hwnd)
{
	for (auto &it : g_arCache.rev_iter())
		if (it->hwnd == hwnd)
			g_arCache.remove(g_arCache.indexOf(&it));
}

IDataObject* CacheIconToIDataObject(HWND hwnd, HICON hIcon, IOleClientSite *pOleClientSite, IStorage **ppStorage, IOleObject **ppOleObject)
{
	static const FORMATETC lc_format[] =
	{
		{ CF_ENHMETAFILE, nullptr, DVASPECT_CONTENT, -1, TYMED_ENHMF }
	};

	mir_cslock lck(csEmfCache);
	for (auto &it : g_arCache.rev_iter())
		if (it->hwnd == hwnd && it->hIcon == hIcon) {
			*ppStorage = it->pStorage;
			*ppOleObject = it->pOleObject;
			return it->pEmfObj;
		}

	// cache new item
	EMFCACHE *newItem = new EMFCACHE();

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
		delete newItem;
		return nullptr;
	}

	OleSetContainedObject(newItem->pOleObject, TRUE);

	g_arCache.insert(newItem);

	*ppStorage = newItem->pStorage;
	*ppOleObject = newItem->pOleObject;
	return newItem->pEmfObj;
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
	REOBJECT reobject = {};
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
