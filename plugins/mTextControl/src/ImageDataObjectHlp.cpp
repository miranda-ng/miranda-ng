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

#include "headers.h"

struct EMFCACHE
{
	HENHMETAFILE hEmf;
	HICON hIcon;
	EMFCACHE *prev;
	EMFCACHE *next;
} *emfCache = 0;
int emfCacheSize = 0;
mir_cs csEmfCache;

void UnloadEmfCache()
{
	while (emfCache)
	{
		EMFCACHE *tmp = emfCache->next;
		delete emfCache;
		emfCache = tmp;
	}
}

HENHMETAFILE CacheIconToEmf(HICON hIcon)
{
	HENHMETAFILE result = 0;
	mir_cslock lck(csEmfCache);
	for (EMFCACHE *p = emfCache; p; p = p->next)
		if (p->hIcon == hIcon)
		{
			if (p->prev)
			{
				p->prev->next = p->next;
				if (p->next) p->next->prev = p->prev;
				p->prev = 0;
				emfCache->prev = p;
				p->next = emfCache;
				emfCache = p;
				result = CopyEnhMetaFile(emfCache->hEmf, 0);
				break;
			}
		}

	// cache new item
	if (!result)
	{
		EMFCACHE *newItem = new EMFCACHE;
		newItem->prev = 0;
		newItem->next = emfCache;
		if (emfCache) emfCache->prev = newItem;
		emfCache = newItem;
		emfCacheSize++;

		HDC emfdc = CreateEnhMetaFile(NULL, NULL, NULL, _T("icon"));
		DrawIconEx(emfdc, 0, 0, (HICON)hIcon, 16, 16, 0, NULL, DI_NORMAL);
		emfCache->hIcon = hIcon;
		emfCache->hEmf = CloseEnhMetaFile(emfdc);
		result = CopyEnhMetaFile(emfCache->hEmf, 0);
	}

	// tail cutoff
	if (emfCacheSize > 20)
	{
		int n = 0;
		EMFCACHE *p;
		for (p = emfCache; p; p = p->next)
			if (++n > 20)
				break;
		while (p->next)
		{
			EMFCACHE *tmp = p->next;
			p->next = p->next->next;
			delete tmp;
		}
		if (p->next) p->next->prev = p;
		emfCacheSize = 20;
	}

	return result;
}

HRESULT CreateDataObject(const FORMATETC *fmtetc, const STGMEDIUM *stgmed, UINT count, IDataObject **ppDataObject);

// returns true on success, false on failure
//bool InsertBitmap(IRichEditOle* pRichEditOle, HBITMAP hBitmap, HGLOBAL hGlobal)
bool InsertBitmap(IRichEditOle* pRichEditOle, HENHMETAFILE hEmf)
{
	SCODE sc;

	// Get the image data object
	//
	static const FORMATETC lc_format[] =
	{
		{ CF_ENHMETAFILE, 0, DVASPECT_CONTENT, -1, TYMED_ENHMF }//,
		//		{ CF_BITMAP, 0, DVASPECT_CONTENT, -1, TYMED_GDI },
		//		{ CF_TEXT,   0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL } 
	};

	STGMEDIUM lc_stgmed[] =
	{
		{ TYMED_ENHMF, { (HBITMAP)hEmf }, 0 }//,
		//		{ TYMED_GDI, { hBitmap }, 0 },
		//		{ TYMED_HGLOBAL, { (HBITMAP)hGlobal }, 0 }
	};

	IDataObject *pods;
	CreateDataObject(lc_format, lc_stgmed, 1, &pods);

	// Get the RichEdit container site
	//
	IOleClientSite *pOleClientSite;
	pRichEditOle->GetClientSite(&pOleClientSite);

	// Initialize a Storage Object
	//
	LPLOCKBYTES lpLockBytes = NULL;
	sc = CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK)
	{
		pOleClientSite->Release();
		return false;
	}

	IStorage *pStorage;
	sc = StgCreateDocfileOnILockBytes(lpLockBytes,
		STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &pStorage);
	if (sc != S_OK)
	{
		lpLockBytes->Release();
		pOleClientSite->Release();
		return false;
	}

	// The final ole object which will be inserted in the richedit control
	//
	IOleObject *pOleObject;
	sc = OleCreateStaticFromData(pods, IID_IOleObject, OLERENDER_FORMAT,
		(LPFORMATETC)lc_format, pOleClientSite, pStorage, (void **)&pOleObject);
	if (sc != S_OK)
	{
		pStorage->Release();
		lpLockBytes->Release();
		pOleClientSite->Release();
		return false;
	}

	// all items are "contained" -- this makes our reference to this object
	//  weak -- which is needed for links to embedding silent update.
	OleSetContainedObject(pOleObject, TRUE);

	// Now Add the object to the RichEdit 
	//
	REOBJECT reobject = { 0 };

	reobject.cbStruct = sizeof(REOBJECT);
	reobject.cp = REO_CP_SELECTION;
	reobject.dvaspect = DVASPECT_CONTENT;
	reobject.poleobj = pOleObject;
	reobject.polesite = pOleClientSite;
	reobject.pstg = pStorage;
	reobject.dwFlags = REO_BELOWBASELINE;

	sc = pOleObject->GetUserClassID(&reobject.clsid);
	if (sc != S_OK)
	{
		pOleObject->Release();
		pStorage->Release();
		lpLockBytes->Release();
		pOleClientSite->Release();
		return false;
	}

	// Insert the bitmap at the current location in the richedit control
	//
	sc = pRichEditOle->InsertObject(&reobject);

	// Release all unnecessary interfaces
	//
	pOleObject->Release();
	pStorage->Release();
	lpLockBytes->Release();
	pOleClientSite->Release();
	pods->Release();

	return sc == S_OK;
}
