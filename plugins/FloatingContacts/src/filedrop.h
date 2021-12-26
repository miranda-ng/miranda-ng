/* This file is a modification of clcfiledrop.h originally 
 written by Richard Hughes*/

#pragma once

static HWND	hwndCurDrag	 = nullptr;
class CDropTarget: public IDropTarget
{
private:
	unsigned long refCount;

public:
	CDropTarget() : 
		refCount(0)
	{}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE DragOver(DWORD fKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pData, DWORD fKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT STDMETHODCALLTYPE DragLeave();
	HRESULT STDMETHODCALLTYPE Drop(IDataObject *pData, DWORD fKeyState, POINTL pt, DWORD *pdwEffect);
};

void RegisterFileDropping(HWND hwnd, CDropTarget *pdropTarget);
void UnregisterFileDropping(HWND hwnd);
