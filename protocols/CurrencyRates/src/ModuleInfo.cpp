#include "StdAfx.h"

static CModuleInfo mi;
static mir_cs g_lmParsers;

typedef std::map<std::string, MWindowList> THandles;
static THandles g_ahWindowLists;

MWindowList CModuleInfo::GetWindowList(const std::string& rsKey, bool bAllocateIfNonExist /*= true*/)
{
	MWindowList hResult = nullptr;
	THandles::const_iterator i = g_ahWindowLists.find(rsKey);
	if (i != g_ahWindowLists.end()) {
		hResult = i->second;
	}
	else if (bAllocateIfNonExist) {
		hResult = WindowList_Create();
		if (hResult)
			g_ahWindowLists.insert(std::make_pair(rsKey, hResult));
	}

	return hResult;
}

void CModuleInfo::OnMirandaShutdown()
{
	for (auto &p : g_ahWindowLists)
		WindowList_Broadcast(p.second, WM_CLOSE, 0, 0);
}
