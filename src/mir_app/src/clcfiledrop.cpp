/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);

	HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT STDMETHODCALLTYPE DragLeave(void);
	HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
}
static dropTarget;

static HWND hwndCurrentDrag = nullptr;
static int originalSelection;

HRESULT CDropTarget::QueryInterface(REFIID riid, LPVOID * ppvObj)
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
	DWORD hitFlags;
	ClcContact *contact;
	int hit = cli.pfnHitTest(hwnd, dat, x, y, &contact, nullptr, &hitFlags);
	if (hit == -1 || !(hitFlags & (CLCHT_ONITEMLABEL | CLCHT_ONITEMICON)) || contact->type != CLCIT_CONTACT)
		return 0;

	char *szProto = GetContactProto(contact->hContact);
	if (szProto == nullptr)
		return 0;

	DWORD protoCaps = CallProtoServiceInt(0,szProto, PS_GETCAPS, PFLAGNUM_1, 0);
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
	cli.pfnTrayIconPauseAutoHide(0, 0);
	ClcData *dat = (ClcData*)GetWindowLongPtr(hwndCurrentDrag, 0);
	shortPt.x = pt.x;
	shortPt.y = pt.y;
	ScreenToClient(hwndCurrentDrag, &shortPt);
	GetClientRect(hwndCurrentDrag, &clRect);

	if (shortPt.y < dat->dragAutoScrollHeight || shortPt.y >= clRect.bottom - dat->dragAutoScrollHeight) {
		*pdwEffect |= DROPEFFECT_SCROLL;
		cli.pfnScrollTo(hwndCurrentDrag, dat, dat->yScroll + (shortPt.y < dat->dragAutoScrollHeight ? -1 : 1) * dat->rowHeight * 2, 0);
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
		cli.pfnInvalidateRect(hwndCurrentDrag, nullptr, FALSE);
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
		cli.pfnInvalidateRect(hwndCurrentDrag, nullptr, FALSE);
	}
	hwndCurrentDrag = nullptr;
	return S_OK;
}

static void AddToFileList(wchar_t ***pppFiles, int *totalCount, const wchar_t *szFilename)
{
	*pppFiles = (wchar_t **) mir_realloc(*pppFiles, (++*totalCount + 1) * sizeof(wchar_t *));
	(*pppFiles)[*totalCount] = nullptr;
	(*pppFiles)[*totalCount - 1] = mir_wstrdup(szFilename);
	if (GetFileAttributes(szFilename) & FILE_ATTRIBUTE_DIRECTORY) {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		wchar_t szPath[MAX_PATH];
		mir_wstrcpy(szPath, szFilename);
		mir_wstrcat(szPath, L"\\*");
		if (hFind = FindFirstFile(szPath, &fd)) {
			do {
				if (!mir_wstrcmp(fd.cFileName, L".") || !mir_wstrcmp(fd.cFileName, L".."))
					continue;
				mir_wstrcpy(szPath, szFilename);
				mir_wstrcat(szPath, L"\\");
				mir_wstrcat(szPath, fd.cFileName);
				AddToFileList(pppFiles, totalCount, szPath);
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}
	}
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
	hDrop = (HDROP) stg.hGlobal;
	ClcData *dat = (ClcData *) GetWindowLongPtr(hwndCurrentDrag, 0);

	shortPt.x = pt.x;
	shortPt.y = pt.y;
	ScreenToClient(hwndCurrentDrag, &shortPt);
	MCONTACT hContact = HContactFromPoint(hwndCurrentDrag, dat, shortPt.x, shortPt.y, nullptr);
	if (hContact != 0) {
		wchar_t **ppFiles = nullptr;
		wchar_t szFilename[MAX_PATH];
		int fileCount, totalCount = 0, i;

		fileCount = DragQueryFile(hDrop, -1, nullptr, 0);
		ppFiles = nullptr;
		for (i=0; i < fileCount; i++) {
			DragQueryFile(hDrop, i, szFilename, _countof(szFilename));
			AddToFileList(&ppFiles, &totalCount, szFilename);
		}

		if (!CallService(MS_FILE_SENDSPECIFICFILEST, hContact, (LPARAM)ppFiles))
			*pdwEffect = DROPEFFECT_COPY;

		for (i=0; ppFiles[i]; i++)
			mir_free(ppFiles[i]);
		mir_free(ppFiles);
	}

	if (stg.pUnkForRelease)
		stg.pUnkForRelease->Release();
	else
		GlobalFree(stg.hGlobal);

	DragLeave();
	return S_OK;
}

static VOID CALLBACK CreateDropTargetHelperTimerProc(HWND hwnd, UINT, UINT_PTR idEvent, DWORD)
{
	KillTimer(hwnd, idEvent);
	//This is a ludicrously slow function (~200ms) so we delay load it a bit.
	if (S_OK != CoCreateInstance(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER,
		IID_IDropTargetHelper, (LPVOID*)&dropTarget.pDropTargetHelper))
		dropTarget.pDropTargetHelper = nullptr;
}

void InitFileDropping(void)
{
	// Disabled as this function loads tons of dlls for no apparenet reason
	// we will se what the reaction will be
//	SetTimer(nullptr, 1, 1000, CreateDropTargetHelperTimerProc);
}

void fnRegisterFileDropping(HWND hwnd)
{
	RegisterDragDrop(hwnd, (IDropTarget *) & dropTarget);
}

void fnUnregisterFileDropping(HWND hwnd)
{
	RevokeDragDrop(hwnd);
}
