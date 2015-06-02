/*
 * test for gdi+
 */

#include <commonheaders.h>
#include <shobjidl.h>
#include "msapi/comptr.h"

#define _CLN_GDIP

#undef Translate

void RemoveFromTaskBar(HWND hWnd)
{
	CComPtr<ITaskbarList> pTaskbarList;
	if (SUCCEEDED(pTaskbarList.CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER)))
		if (SUCCEEDED(pTaskbarList->HrInit()))
			pTaskbarList->DeleteTab(hWnd);
}

void AddToTaskBar(HWND hWnd)
{
	CComPtr<ITaskbarList> pTaskbarList;
	if (SUCCEEDED(pTaskbarList.CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER)))
		if (SUCCEEDED(pTaskbarList->HrInit()))
			pTaskbarList->AddTab(hWnd);
}
