#include "stdafx.h"

static BOOL OnDropFiles(HDROP hDrop, ThumbInfo *pThumb);

HRESULT STDMETHODCALLTYPE CDropTarget::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	if (IsEqualIID(riid, IID_IDropTarget)) {
		*ppvObj = this;
		this->AddRef();
		return S_OK;
	}

	*ppvObj = nullptr;

	return (E_NOINTERFACE);
}

ULONG STDMETHODCALLTYPE CDropTarget::AddRef()
{
	return ++this->refCount;
}

ULONG STDMETHODCALLTYPE CDropTarget::Release()
{
	int res = --this->refCount;
	if (!res) delete this;
	return res;
}

HRESULT STDMETHODCALLTYPE CDropTarget::DragOver(DWORD, POINTL, DWORD *pdwEffect)
{
	*pdwEffect = 0;

	if (hwndCurDrag == nullptr) {
		*pdwEffect = DROPEFFECT_NONE;
	}
	else {
		*pdwEffect |= DROPEFFECT_COPY;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDropTarget::DragEnter(IDataObject *pData, DWORD fKeyState, POINTL pt, DWORD *pdwEffect)
{
	FORMATETC	feFile = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	FORMATETC	feText = { CF_TEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	if (S_OK == pData->QueryGetData(&feFile) ||
		S_OK == pData->QueryGetData(&feText)) {
		POINT shortPt;
		shortPt.x = pt.x;
		shortPt.y = pt.y;

		HWND hwnd = WindowFromPoint(shortPt);

		ThumbInfo *pThumb = thumbList.FindThumb(hwnd);
		if (pThumb) {
			hwndCurDrag = hwnd;
			pThumb->ThumbSelect(TRUE);
		}
	}

	return DragOver(fKeyState, pt, pdwEffect);
}


HRESULT STDMETHODCALLTYPE CDropTarget::DragLeave()
{
	ThumbInfo *pThumb = thumbList.FindThumb(hwndCurDrag);

	if (nullptr != pThumb) {
		pThumb->ThumbDeselect(TRUE);
	}

	hwndCurDrag = nullptr;

	return S_OK;
}


HRESULT STDMETHODCALLTYPE CDropTarget::Drop(IDataObject *pData, DWORD, POINTL, DWORD *pdwEffect)
{
	FORMATETC fe = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	*pdwEffect = DROPEFFECT_NONE;

	if (hwndCurDrag == nullptr)
		return S_OK;

	ThumbInfo *pThumb = (ThumbInfo*)GetWindowLongPtr(hwndCurDrag, GWLP_USERDATA);
	if (pThumb == nullptr)
		return S_OK;

	STGMEDIUM stg;
	bool bFormatText = false;
	if (S_OK != pData->GetData(&fe, &stg)) {
		fe.cfFormat = CF_UNICODETEXT;

		if (S_OK != pData->GetData(&fe, &stg)) {
			return S_OK;
		}
		else {
			bFormatText = true;
		}
	}

	if (!bFormatText) {
		HDROP hDrop = (HDROP)stg.hGlobal;
		if (hDrop != nullptr) {
			OnDropFiles(hDrop, pThumb);
		}
	}
	else {
		wchar_t *pText = (wchar_t*)GlobalLock(stg.hGlobal);
		if (pText != nullptr) {
			SendMsgDialog(hwndCurDrag, pText);
			GlobalUnlock(stg.hGlobal);
		}
	}

	if (stg.pUnkForRelease != nullptr) {
		stg.pUnkForRelease->Release();
	}
	else {
		GlobalFree(stg.hGlobal);
	}

	DragLeave();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Send files processing

static void SaveFiles(wchar_t *wszItem, wchar_t **ppFiles, int *pnCount)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFileW(wszItem, &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
				// Skip parent directories
				if ((0 != mir_wstrcmp(fd.cFileName, L".")) &&
					(0 != mir_wstrcmp(fd.cFileName, L".."))) {
					wchar_t wszDirName[MAX_PATH];
					wcsncpy(wszDirName, wszItem, MAX_PATH - 1);

					if (nullptr != wcsstr(wszItem, L"*.*")) {
						size_t offset = mir_wstrlen(wszDirName) - 3;
						mir_snwprintf(wszDirName + offset, _countof(wszDirName) - offset, L"%s\0", fd.cFileName);
					}

					ppFiles[*pnCount] = _wcsdup(wszDirName);
					++(*pnCount);

					mir_wstrcat(wszDirName, L"\\*.*");
					SaveFiles(wszDirName, ppFiles, pnCount);

				}
			}
			else {
				size_t nSize = mir_wstrlen(wszItem) + mir_wstrlen(fd.cFileName) + 1;
				wchar_t *wszFile = (wchar_t *)malloc(nSize * sizeof(wchar_t));

				wcsncpy(wszFile, wszItem, nSize);

				if (nullptr != wcsstr(wszFile, L"*.*")) {
					wszFile[mir_wstrlen(wszFile) - 3] = '\0';
					mir_wstrncat(wszFile, fd.cFileName, nSize - mir_wstrlen(wszFile));
				}

				ppFiles[*pnCount] = wszFile;
				++(*pnCount);
			}
		} while (FALSE != FindNextFileW(hFind, &fd));
	}
}

static void ProcessDroppedItems(wchar_t **ppDroppedItems, int nCount, wchar_t **ppFiles)
{
	int fileCount = 0;

	for (int i = 0; i < nCount; ++i)
		SaveFiles(ppDroppedItems[i], ppFiles, &fileCount);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CountFiles(wchar_t *wszItem)
{
	int nCount = 0;
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFileW(wszItem, &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
				// Skip parent directories
				if ((0 != mir_wstrcmp(fd.cFileName, L".")) &&
					(0 != mir_wstrcmp(fd.cFileName, L".."))) {
					wchar_t szDirName[MAX_PATH];
					wcsncpy(szDirName, wszItem, MAX_PATH - 1);

					if (nullptr != wcsstr(wszItem, L"*.*")) {
						size_t offset = mir_wstrlen(szDirName) - 3;
						mir_snwprintf(szDirName + offset, _countof(szDirName) - offset, L"%s\0", fd.cFileName);
					}

					++nCount;
					mir_wstrcat(szDirName, L"\\*.*");
					nCount += CountFiles(szDirName);
				}
			}
			else ++nCount;
		} while (FALSE != FindNextFileW(hFind, &fd));
	}

	return nCount;
}

static int CountDroppedFiles(wchar_t **ppDroppedItems, int nCount)
{
	int fileCount = 0;

	for (int i = 0; i < nCount; ++i)
		fileCount += CountFiles(ppDroppedItems[i]);

	return fileCount;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL OnDropFiles(HDROP hDrop, ThumbInfo *pThumb)
{
	UINT nDroppedItemsCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

	wchar_t **ppDroppedItems = (wchar_t **)malloc(sizeof(wchar_t *)*(nDroppedItemsCount + 1));
	if (ppDroppedItems == nullptr)
		return FALSE;

	ppDroppedItems[nDroppedItemsCount] = nullptr;

	wchar_t wszFilename[MAX_PATH];
	for (UINT iItem = 0; iItem < nDroppedItemsCount; ++iItem) {
		DragQueryFileW(hDrop, iItem, wszFilename, _countof(wszFilename));
		ppDroppedItems[iItem] = _wcsdup(wszFilename);
	}

	UINT nFilesCount = CountDroppedFiles(ppDroppedItems, nDroppedItemsCount);

	wchar_t **ppFiles = (wchar_t **)malloc(sizeof(wchar_t *)* (nFilesCount + 1));

	BOOL bSuccess = FALSE;
	if (ppFiles != nullptr) {
		ppFiles[nFilesCount] = nullptr;

		ProcessDroppedItems(ppDroppedItems, nDroppedItemsCount, ppFiles);

		bSuccess = File::Send(pThumb->hContact, ppFiles) != 0;

		for (UINT iItem = 0; iItem < nFilesCount; ++iItem)
			free(ppFiles[iItem]);

		free(ppFiles);
	}

	// Cleanup
	for (UINT iItem = 0; ppDroppedItems[iItem]; ++iItem) {
		free(ppDroppedItems[iItem]);
	}

	free(ppDroppedItems);

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
// Init/destroy

void RegisterFileDropping(HWND hwnd, CDropTarget* pdropTarget)
{
	RegisterDragDrop(hwnd, (IDropTarget*)pdropTarget);
}

void UnregisterFileDropping(HWND hwnd)
{
	RevokeDragDrop(hwnd);
}
