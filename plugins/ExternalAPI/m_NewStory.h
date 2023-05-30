#pragma once

struct NSMenuExecParam
{
	char *szServiceName;
	int iParam;
};

__forceinline HGENMENU Menu_AddNewStoryMenuItem(TMO_MenuItem *pmi, int param)
{
	return (HGENMENU)CallService("NSMenu/AddService", (WPARAM)pmi, param);
}

// event for changing NewStory menu items
// wparam = (MCONTACT)hContact - contact id
// lparam = (DB::EventInfo*)dbei - event
#define  ME_NS_PREBUILDMENU  "NewStory/PreBuildMenu"
