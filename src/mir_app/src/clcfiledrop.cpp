/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "stdafx.h"
#include "clc.h"
#include <shlobj.h>

struct CDropTarget : IDropTarget
{
	LONG refCount;
	IDropTargetHelper *pDropTargetHelper;

	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);

	HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT STDMETHODCALLTYPE DragLeave(void);
	HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
}
static dropTarget;

static HWND hwndCurrentDrag = nullptr;
static int originalSelection;

HRESULT CDropTarget::QueryInterface(REFIID riid, void **ppvObj)
{
	if (riid == IID_IDropTarget) {
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

ULONG CDropTarget::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}

ULONG CDropTarget::Release(void)
{
	if (refCount == 1) {
		if (pDropTargetHelper)
			pDropTargetHelper->Release();
	}
	return InterlockedDecrement(&refCount);
}

static MCONTACT HContactFromPoint(HWND hwnd, ClcData *dat, int x, int y, int *hitLine)
{
	uint32_t hitFlags;
	ClcContact *contact;
	int hit = g_clistApi.pfnHitTest(hwnd, dat, x, y, &contact, nullptr, &hitFlags);
	if (hit == -1 || !(hitFlags & (CLCHT_ONITEMLABEL | CLCHT_ONITEMICON)) || contact->type != CLCIT_CONTACT)
		return 0;

	char *szProto = Proto_GetBaseAccountName(contact->hContact);
	if (szProto == nullptr)
		return 0;

	uint32_t protoCaps = CallProtoServiceInt(0,szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	if (!(protoCaps & PF1_FILESEND))
		return 0;
	if (ID_STATUS_OFFLINE == db_get_w(contact->hContact, szProto, "Status", ID_STATUS_OFFLINE))
		return 0;
	if (hitLine)
		*hitLine = hit;
	return contact->hContact;
}

HRESULT CDropTarget::DragOver(DWORD /*grfKeyState*/, POINTL pt, DWORD * pdwEffect)
{
	POINT shortPt;
	RECT clRect;
	int hit;
	MCONTACT hContact;

	if (pDropTargetHelper && hwndCurrentDrag)
		pDropTargetHelper->DragOver((POINT*)&pt, *pdwEffect);

	*pdwEffect = 0;
	if (hwndCurrentDrag == nullptr) {
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}
	g_clistApi.pfnTrayIconPauseAutoHide(0, 0);
	ClcData *dat = (ClcData*)GetWindowLongPtr(hwndCurrentDrag, 0);
	shortPt.x = pt.x;
	shortPt.y = pt.y;
	ScreenToClient(hwndCurrentDrag, &shortPt);
	GetClientRect(hwndCurrentDrag, &clRect);

	if (shortPt.y < dat->dragAutoScrollHeight || shortPt.y >= clRect.bottom - dat->dragAutoScrollHeight) {
		*pdwEffect |= DROPEFFECT_SCROLL;
		g_clistApi.pfnScrollTo(hwndCurrentDrag, dat, dat->yScroll + (shortPt.y < dat->dragAutoScrollHeight ? -1 : 1) * dat->rowHeight * 2, 0);
	}
	hContact = HContactFromPoint(hwndCurrentDrag, dat, shortPt.x, shortPt.y, &hit);
	if (hContact == 0) {
		hit = -1;
		*pdwEffect |= DROPEFFECT_NONE;
	}
	else
		*pdwEffect |= DROPEFFECT_COPY;

	if (dat->selection != hit) {
		dat->selection = hit;
		g_clistApi.pfnInvalidateRect(hwndCurrentDrag, nullptr, FALSE);
		if (pDropTargetHelper) pDropTargetHelper->Show(FALSE);
		UpdateWindow(hwndCurrentDrag);
		if (pDropTargetHelper) pDropTargetHelper->Show(TRUE);
	}

	return S_OK;
}

HRESULT CDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	HWND hwnd;
	wchar_t szWindowClass[64];
	POINT shortPt;

	shortPt.x = pt.x;
	shortPt.y = pt.y;
	hwnd = WindowFromPoint(shortPt);
	GetClassName(hwnd, szWindowClass, _countof(szWindowClass));
	if (!mir_wstrcmp(szWindowClass, CLISTCONTROL_CLASSW)) {
		hwndCurrentDrag = hwnd;
		ClcData *dat = (ClcData *) GetWindowLongPtr(hwndCurrentDrag, 0);
		originalSelection = dat->selection;
		dat->bShowSelAlways = true;
	}
	if (pDropTargetHelper && hwndCurrentDrag)
		pDropTargetHelper->DragEnter(hwndCurrentDrag, pDataObj, (POINT*)&pt, *pdwEffect);
	return DragOver(grfKeyState, pt, pdwEffect);
}

HRESULT CDropTarget::DragLeave(void)
{
	if (hwndCurrentDrag) {
		if (pDropTargetHelper)
			pDropTargetHelper->DragLeave();
		ClcData *dat = (ClcData *) GetWindowLongPtr(hwndCurrentDrag, 0);
		dat->bShowSelAlways = false;
		dat->selection = originalSelection;
		g_clistApi.pfnInvalidateRect(hwndCurrentDrag, nullptr, FALSE);
	}
	hwndCurrentDrag = nullptr;
	return S_OK;
}

HRESULT CDropTarget::Drop(IDataObject * pDataObj, DWORD /*fKeyState*/, POINTL pt, DWORD * pdwEffect)
{
	FORMATETC fe = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg;
	HDROP hDrop;
	POINT shortPt;

	if (pDropTargetHelper && hwndCurrentDrag)
		pDropTargetHelper->Drop(pDataObj, (POINT*)&pt, *pdwEffect);

	*pdwEffect = DROPEFFECT_NONE;
	if (hwndCurrentDrag == nullptr || S_OK != pDataObj->GetData(&fe, &stg))
		return S_OK;

	hDrop = (HDROP)stg.hGlobal;
	ClcData *dat = (ClcData *)GetWindowLongPtr(hwndCurrentDrag, 0);

	shortPt.x = pt.x;
	shortPt.y = pt.y;
	ScreenToClient(hwndCurrentDrag, &shortPt);
	MCONTACT hContact = HContactFromPoint(hwndCurrentDrag, dat, shortPt.x, shortPt.y, nullptr);
	if (hContact != 0)
		if (ProcessFileDrop(hDrop, hContact))
			*pdwEffect = DROPEFFECT_COPY;

	if (stg.pUnkForRelease)
		stg.pUnkForRelease->Release();
	else
		GlobalFree(stg.hGlobal);

	DragLeave();
	return S_OK;
}

void RegisterFileDropping(HWND hwnd)
{
	RegisterDragDrop(hwnd, (IDropTarget*)&dropTarget);
}

void UnregisterFileDropping(HWND hwnd)
{
	RevokeDragDrop(hwnd);
}
