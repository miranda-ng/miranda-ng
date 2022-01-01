/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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

#include "stdafx.h"

// returns true on success, false on failure
bool CImageDataObject::InsertBitmap(IRichEditOle *pRichEditOle, HBITMAP hBitmap)
{
	if (pRichEditOle == nullptr)
		return false;

	// Get the image data object
	CImageDataObject *pods = new CImageDataObject;
	CComPtr<IDataObject> lpDataObject;
	pods->QueryInterface(IID_IDataObject, (void**)&lpDataObject);

	BITMAP bminfo;
	GetObject(hBitmap, sizeof(bminfo), &bminfo);
	pods->SetBitmap(hBitmap);

	// Get the RichEdit container site
	CComPtr<IOleClientSite> pOleClientSite;
	pRichEditOle->GetClientSite(&pOleClientSite);

	CComPtr<ILockBytes> lpLockBytes;
	if (FAILED(::CreateILockBytesOnHGlobal(nullptr, TRUE, &lpLockBytes)))
		return false;

	// Initialize a Storage Object
	CComPtr<IStorage> pStorage;
	if (FAILED(::StgCreateDocfileOnILockBytes(lpLockBytes, STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &pStorage)))
		return false;

	// The final ole object which will be inserted in the richedit control
	CComPtr<IOleObject> pOleObject = pods->GetOleObject(pOleClientSite, pStorage);
	if (pOleObject == nullptr)
		return false;

	// all items are "contained" -- this makes our reference to this object
	//  weak -- which is needed for links to embedding silent update.
	OleSetContainedObject(pOleObject, TRUE);

	// Now Add the object to the RichEdit
	CLSID clsid;
	if (FAILED(pOleObject->GetUserClassID(&clsid)))
		return false;

	// Insert the bitmap at the current location in the richedit control
	REOBJECT reobject = {};
	reobject.cbStruct = sizeof(REOBJECT);
	reobject.clsid = clsid;
	reobject.cp = REO_CP_SELECTION;
	reobject.dvaspect = DVASPECT_CONTENT;
	reobject.poleobj = pOleObject;
	reobject.polesite = pOleClientSite;
	reobject.pstg = pStorage;
	reobject.dwFlags = bminfo.bmHeight <= 12 ? 0 : REO_BELOWBASELINE;
	return pRichEditOle->InsertObject(&reobject) == S_OK;
}

void CImageDataObject::SetBitmap(HBITMAP hBitmap)
{
	STGMEDIUM stgm;
	stgm.tymed = TYMED_GDI;                 // Storage medium = HBITMAP handle
	stgm.hBitmap = hBitmap;
	stgm.pUnkForRelease = nullptr;       // Use ReleaseStgMedium

	FORMATETC fm;
	fm.cfFormat = CF_BITMAP;                // Clipboard format = CF_BITMAP
	fm.ptd = nullptr;                              // Target Device = Screen
	fm.dwAspect = DVASPECT_CONTENT; // Level of detail = Full content
	fm.lindex = -1;                             // Index = Not applicaple
	fm.tymed = TYMED_GDI;                     // Storage medium = HBITMAP handle

	this->SetData(&fm, &stgm, TRUE);
}

IOleObject* CImageDataObject::GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage)
{
	IOleObject *pOleObject;
	if (FAILED(::OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_FORMAT, &m_format, pOleClientSite, pStorage, (void **)&pOleObject)))
		return nullptr;
	return pOleObject;
}
