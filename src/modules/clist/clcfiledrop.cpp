/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
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

static HWND hwndCurrentDrag = NULL;
static int originalSelection;

HRESULT CDropTarget::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (riid == IID_IDropTarget) {
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	*ppvObj = NULL;
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

static MCONTACT HContactFromPoint(HWND hwnd, struct ClcData *dat, int x, int y, int *hitLine)
{
	DWORD hitFlags;
	ClcContact *contact;
	int hit = cli.pfnHitTest(hwnd, dat, x, y, &contact, NULL, &hitFlags);
	if (hit == -1 || !(hitFlags & (CLCHT_ONITEMLABEL | CLCHT_ONITEMICON)) || contact->type != CLCIT_CONTACT)
		return NULL;

	char *szProto = GetContactProto(contact->hContact);
	if (szProto == NULL)
		return NULL;

	DWORD protoCaps = CallProtoServiceInt(NULL,szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	if (!(protoCaps & PF1_FILESEND))
		return NULL;
	if (ID_STATUS_OFFLINE == db_get_w(contact->hContact, szProto, "Status", ID_STATUS_OFFLINE))
		return NULL;
	if (hitLine)
		*hitLine = hit;
	return contact->hContact;
}

HRESULT CDropTarget::DragOver(DWORD /*grfKeyState*/, POINTL pt, DWORD * pdwEffect)
{
	POINT shortPt;
	struct ClcData *dat;
	RECT clRect;
	int hit;
	MCONTACT hContact;

	if (pDropTargetHelper && hwndCurrentDrag)
		pDropTargetHelper->DragOver((POINT*)&pt, *pdwEffect);

	*pdwEffect = 0;
	if (hwndCurrentDrag == NULL) {
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}
	cli.pfnTrayIconPauseAutoHide(0, 0);
	dat = (struct ClcData *) GetWindowLongPtr(hwndCurrentDrag, 0);
	shortPt.x = pt.x;
	shortPt.y = pt.y;
	ScreenToClient(hwndCurrentDrag, &shortPt);
	GetClientRect(hwndCurrentDrag, &clRect);

	if (shortPt.y < dat->dragAutoScrollHeight || shortPt.y >= clRect.bottom - dat->dragAutoScrollHeight) {
		*pdwEffect |= DROPEFFECT_SCROLL;
		cli.pfnScrollTo(hwndCurrentDrag, dat, dat->yScroll + (shortPt.y < dat->dragAutoScrollHeight ? -1 : 1) * dat->rowHeight * 2, 0);
	}
	hContact = HContactFromPoint(hwndCurrentDrag, dat, shortPt.x, shortPt.y, &hit);
	if (hContact == NULL) {
		hit = -1;
		*pdwEffect |= DROPEFFECT_NONE;
	}
	else
		*pdwEffect |= DROPEFFECT_COPY;

	if (dat->selection != hit) {
		dat->selection = hit;
		cli.pfnInvalidateRect(hwndCurrentDrag, NULL, FALSE);
		if (pDropTargetHelper) pDropTargetHelper->Show(FALSE);
		UpdateWindow(hwndCurrentDrag);
		if (pDropTargetHelper) pDropTargetHelper->Show(TRUE);
	}

	return S_OK;
}

HRESULT CDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	HWND hwnd;
	TCHAR szWindowClass[64];
	POINT shortPt;

	shortPt.x = pt.x;
	shortPt.y = pt.y;
	hwnd = WindowFromPoint(shortPt);
	GetClassName(hwnd, szWindowClass, SIZEOF(szWindowClass));
	if (!mir_tstrcmp(szWindowClass, _T(CLISTCONTROL_CLASS))) {
		struct ClcData *dat;
		hwndCurrentDrag = hwnd;
		dat = (struct ClcData *) GetWindowLongPtr(hwndCurrentDrag, 0);
		originalSelection = dat->selection;
		dat->showSelAlways = 1;
	}
	if (pDropTargetHelper && hwndCurrentDrag)
		pDropTargetHelper->DragEnter(hwndCurrentDrag, pDataObj, (POINT*)&pt, *pdwEffect);
	return DragOver(grfKeyState, pt, pdwEffect);
}

HRESULT CDropTarget::DragLeave(void)
{
	if (hwndCurrentDrag) {
		struct ClcData *dat;
		if (pDropTargetHelper)
			pDropTargetHelper->DragLeave();
		dat = (struct ClcData *) GetWindowLongPtr(hwndCurrentDrag, 0);
		dat->showSelAlways = 0;
		dat->selection = originalSelection;
		cli.pfnInvalidateRect(hwndCurrentDrag, NULL, FALSE);
	}
	hwndCurrentDrag = NULL;
	return S_OK;
}

static void AddToFileList(TCHAR ***pppFiles, int *totalCount, const TCHAR *szFilename)
{
	*pppFiles = (TCHAR **) mir_realloc(*pppFiles, (++*totalCount + 1) * sizeof(TCHAR *));
	(*pppFiles)[*totalCount] = NULL;
	(*pppFiles)[*totalCount - 1] = mir_tstrdup(szFilename);
	if (GetFileAttributes(szFilename) & FILE_ATTRIBUTE_DIRECTORY) {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		TCHAR szPath[MAX_PATH];
		mir_tstrcpy(szPath, szFilename);
		mir_tstrcat(szPath, _T("\\*"));
		if (hFind = FindFirstFile(szPath, &fd)) {
			do {
				if (!mir_tstrcmp(fd.cFileName, _T(".")) || !mir_tstrcmp(fd.cFileName, _T("..")))
					continue;
				mir_tstrcpy(szPath, szFilename);
				mir_tstrcat(szPath, _T("\\"));
				mir_tstrcat(szPath, fd.cFileName);
				AddToFileList(pppFiles, totalCount, szPath);
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}
	}
}

HRESULT CDropTarget::Drop(IDataObject * pDataObj, DWORD /*fKeyState*/, POINTL pt, DWORD * pdwEffect)
{
	FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg;
	HDROP hDrop;
	POINT shortPt;
	struct ClcData *dat;

	if (pDropTargetHelper && hwndCurrentDrag)
		pDropTargetHelper->Drop(pDataObj, (POINT*)&pt, *pdwEffect);

	*pdwEffect = DROPEFFECT_NONE;
	if (hwndCurrentDrag == NULL || S_OK != pDataObj->GetData(&fe, &stg))
		return S_OK;
	hDrop = (HDROP) stg.hGlobal;
	dat = (struct ClcData *) GetWindowLongPtr(hwndCurrentDrag, 0);

	shortPt.x = pt.x;
	shortPt.y = pt.y;
	ScreenToClient(hwndCurrentDrag, &shortPt);
	MCONTACT hContact = HContactFromPoint(hwndCurrentDrag, dat, shortPt.x, shortPt.y, NULL);
	if (hContact != NULL) {
		TCHAR **ppFiles = NULL;
		TCHAR szFilename[MAX_PATH];
		int fileCount, totalCount = 0, i;

		fileCount = DragQueryFile(hDrop, -1, NULL, 0);
		ppFiles = NULL;
		for (i=0; i < fileCount; i++) {
			DragQueryFile(hDrop, i, szFilename, SIZEOF(szFilename));
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
	if (S_OK != CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
		IID_IDropTargetHelper, (LPVOID*)&dropTarget.pDropTargetHelper))
		dropTarget.pDropTargetHelper = NULL;
}

void InitFileDropping(void)
{
	// Disabled as this function loads tons of dlls for no apparenet reason
	// we will se what the reaction will be
//	SetTimer(NULL, 1, 1000, CreateDropTargetHelperTimerProc);
}

void fnRegisterFileDropping(HWND hwnd)
{
	RegisterDragDrop(hwnd, (IDropTarget *) & dropTarget);
}

void fnUnregisterFileDropping(HWND hwnd)
{
	RevokeDragDrop(hwnd);
}
