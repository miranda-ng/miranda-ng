#pragma once

#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////
// NS get selection

#define MS_NEWSTORY_GETSELECTION "NewStory/GetSelection"

__forceinline std::vector<MEVENT> NS_GetSelection(HANDLE hwnd)
{
	std::vector<MEVENT> ret;
	CallService(MS_NEWSTORY_GETSELECTION, WPARAM(hwnd), LPARAM(&ret));
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////
// NS menu item

struct NSMenuExecParam
{
	char *szServiceName;
	int iParam;
};

__forceinline HGENMENU Menu_AddNewStoryMenuItem(TMO_MenuItem *pmi, int param)
{
	return (HGENMENU)CallService("NSMenu/AddService", (WPARAM)pmi, param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// event for changing NewStory menu items
// wparam = (MCONTACT)hContact - contact id
// lparam = (DB::EventInfo*)dbei - event

#define ME_NS_PREBUILDMENU  "NewStory/PreBuildMenu"
