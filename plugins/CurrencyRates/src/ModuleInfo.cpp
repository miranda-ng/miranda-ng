#include "StdAfx.h"

static CModuleInfo mi;
static CModuleInfo::THTMLEnginePtr g_pHTMLEngine;
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

CModuleInfo::THTMLEnginePtr CModuleInfo::GetHTMLEngine()
{
	if (!g_pHTMLEngine) {
		mir_cslock lck(g_lmParsers);
		if (!g_pHTMLEngine)
			g_pHTMLEngine = THTMLEnginePtr(new CHTMLEngineMS);
	}

	return g_pHTMLEngine;
}

void CModuleInfo::SetHTMLEngine(THTMLEnginePtr pEngine)
{
	g_pHTMLEngine = pEngine;
}

bool CModuleInfo::Verify()
{
	INITCOMMONCONTROLSEX icc = { 0 };
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES | ICC_LINK_CLASS;
	if (FALSE == ::InitCommonControlsEx(&icc))
		return false;

	if (!g_pHTMLEngine && (false == CHTMLParserMS::IsInstalled())) {
		CurrencyRates_MessageBox(nullptr,
			TranslateT("Miranda could not load CurrencyRates plugin. Microsoft HTML parser is missing."),
			MB_YESNO | MB_ICONQUESTION);
		return false;
	}

	return true;
}
