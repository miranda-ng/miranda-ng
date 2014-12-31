/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// inserts a bitmap into a rich edit control using OLE
// code partially taken from public example on the internet, source unknown.
//
// originally part of the smileyadd plugin for Miranda NG

#include "commonheaders.h"

extern void ReleaseRichEditOle(IRichEditOle *ole)
{
	ole->Release();
}

extern void ImageDataInsertBitmap(IRichEditOle *ole, HBITMAP hBm)
{
	CImageDataObject::InsertBitmap(ole, hBm);
}

int CacheIconToBMP(TLogIcon *theIcon, HICON hIcon, COLORREF backgroundColor, int sizeX, int sizeY)
{
	int IconSizeX = sizeX;
	int IconSizeY = sizeY;

	if ((IconSizeX == 0) || (IconSizeY == 0)) {
		Utils::getIconSize(hIcon, IconSizeX, IconSizeY);
		if (sizeX != 0)
			IconSizeX = sizeX;
		if (sizeY != 0)
			IconSizeY = sizeY;
	}
	RECT rc;
	BITMAPINFOHEADER bih = {0};
	int widthBytes;
	theIcon->hBkgBrush = CreateSolidBrush(backgroundColor);
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biPlanes = 1;
	bih.biCompression = BI_RGB;
	bih.biHeight = IconSizeY;
	bih.biWidth = IconSizeX;
	widthBytes = ((bih.biWidth * bih.biBitCount + 31) >> 5) * 4;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;
	theIcon->hdc = GetDC(0);
	theIcon->hBmp = CreateCompatibleBitmap(theIcon->hdc, bih.biWidth, bih.biHeight);
	theIcon->hdcMem = CreateCompatibleDC(theIcon->hdc);
	theIcon->hoBmp = (HBITMAP)SelectObject(theIcon->hdcMem, theIcon->hBmp);
	FillRect(theIcon->hdcMem, &rc, theIcon->hBkgBrush);
	DrawIconEx(theIcon->hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
	SelectObject(theIcon->hdcMem, theIcon->hoBmp);
	return TRUE;
}

void DeleteCachedIcon(TLogIcon *theIcon)
{
	DeleteDC(theIcon->hdcMem);
	DeleteObject(theIcon->hBmp);
	ReleaseDC(NULL, theIcon->hdc);
	DeleteObject(theIcon->hBkgBrush);
}

// returns true on success, false on failure
bool CImageDataObject::InsertBitmap(IRichEditOle* pRichEditOle, HBITMAP hBitmap)
{
	SCODE sc;
	BITMAP bminfo;

	// Get the image data object
	//
	CImageDataObject *pods = new CImageDataObject;
	LPDATAOBJECT lpDataObject;
	pods->QueryInterface(IID_IDataObject, (void **)&lpDataObject);

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
	lpDataObject->Release();
	if (sc != S_OK)
		return false;
	else
		return true;
}


void CImageDataObject::SetBitmap(HBITMAP hBitmap)
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


IOleObject *CImageDataObject::GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage)
{
	SCODE sc;
	IOleObject *pOleObject;
	sc = ::OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_FORMAT,
								   &m_format, pOleClientSite, pStorage, (void **) & pOleObject);
	if (sc != S_OK)
		pOleObject = NULL;
	return pOleObject;
}
