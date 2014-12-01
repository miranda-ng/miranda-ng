/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

//code taken partly from public example on the internet, source unknown.

#include "StdAfx.h"
#include "ImageDataObject.h"

HBITMAP CacheIconToBMP(HICON hIcon, COLORREF backgroundColor, int sizeX, int sizeY)
{
	RECT rc;
	HBRUSH hBkgBrush = CreateSolidBrush(backgroundColor);
	rc.top = rc.left = 0;
	rc.right = sizeY;
	rc.bottom = sizeX;
	HDC hdc = GetDC(0);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, sizeY, sizeX);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
	FillRect(hdcMem, &rc, hBkgBrush);
	DrawIconEx(hdcMem, 0, 0, hIcon, sizeY, sizeX, 0, NULL, DI_NORMAL);
	SelectObject(hdcMem, hoBmp);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);
	return hBmp;
}

// returns true on success, false on failure
bool ImageDataObject::InsertIcon(IRichEditOle* pRichEditOle, HICON hIcon,
	COLORREF backgroundColor, int sizeX, int sizeY)
{
	bool result;
	HBITMAP hBmp = CacheIconToBMP(hIcon, backgroundColor, sizeX, sizeY);
	result = InsertBitmap(pRichEditOle, hBmp);
	DeleteObject(hBmp);
	return result;
}

// returns true on success, false on failure
bool ImageDataObject::InsertBitmap(IRichEditOle* pRichEditOle, HBITMAP hBitmap)
{
	SCODE sc;
	BITMAP bminfo;

	// Get the image data object
	//
	ImageDataObject *pods = new ImageDataObject;

	GetObject(hBitmap, sizeof(bminfo), &bminfo);
	pods->SetBitmap(hBitmap);

	// Get the RichEdit container site
	//
	IOleClientSite *pOleClientSite;
	pRichEditOle->GetClientSite(&pOleClientSite);

	// Initialize a Storage Object
	//
	IStorage *pStorage;

	LPLOCKBYTES lpLockBytes = NULL;
	sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK) {
		pOleClientSite->Release();
		return false;
	}
	sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,
										STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &pStorage);
	if (sc != S_OK) {
		lpLockBytes = NULL;
		pOleClientSite->Release();
		return false;
	}
	// The final ole object which will be inserted in the richedit control
	//
	IOleObject *pOleObject;
	pOleObject = pods->GetOleObject(pOleClientSite, pStorage);
	if (pOleObject == NULL) {
		pStorage->Release();
		pOleClientSite->Release();
		return false;
	}

	// all items are "contained" -- this makes our reference to this object
	//  weak -- which is needed for links to embedding silent update.
	OleSetContainedObject(pOleObject, TRUE);

	// Now Add the object to the RichEdit
	//
	REOBJECT reobject;
	memset(&reobject, 0, sizeof(REOBJECT));
	reobject.cbStruct = sizeof(REOBJECT);

	CLSID clsid;
	sc = pOleObject->GetUserClassID(&clsid);
	if (sc != S_OK) {
		pOleObject->Release();
		pStorage->Release();
		pOleClientSite->Release();
		return false;
	}

	reobject.clsid = clsid;
	reobject.cp = REO_CP_SELECTION ;
	reobject.dvaspect = DVASPECT_CONTENT;
	reobject.poleobj = pOleObject;
	reobject.polesite = pOleClientSite;
	reobject.pstg = pStorage;
	reobject.dwFlags = bminfo.bmHeight <= 12 ? 0 : REO_BELOWBASELINE;

	// Insert the bitmap at the current location in the richedit control
	//
	sc = pRichEditOle->InsertObject(&reobject);

	// Release all unnecessary interfaces
	//
	pOleObject->Release();
	pOleClientSite->Release();
	lpLockBytes->Release();
	pStorage->Release();
	if (sc != S_OK)
		return false;
	else
		return true;
}


void ImageDataObject::SetBitmap(HBITMAP hBitmap)
{
	STGMEDIUM stgm;
	stgm.tymed = TYMED_GDI;                 // Storage medium = HBITMAP handle
	stgm.hBitmap = hBitmap;
	stgm.pUnkForRelease = NULL;       // Use ReleaseStgMedium

	FORMATETC fm;
	fm.cfFormat = CF_BITMAP;                // Clipboard format = CF_BITMAP
	fm.ptd = NULL;                              // Target Device = Screen
	fm.dwAspect = DVASPECT_CONTENT; // Level of detail = Full content
	fm.lindex = -1;                             // Index = Not applicaple
	fm.tymed = TYMED_GDI;                     // Storage medium = HBITMAP handle

	this->SetData(&fm, &stgm, TRUE);
}


IOleObject *ImageDataObject::GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage)
{
	SCODE sc;
	IOleObject *pOleObject;
	sc = ::OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_FORMAT,
								   &m_format, pOleClientSite, pStorage, (void **) & pOleObject);
	if (sc != S_OK)
		pOleObject = NULL;
	return pOleObject;
}
