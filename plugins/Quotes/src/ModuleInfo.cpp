#include "StdAfx.h"

namespace
{
	CModuleInfo::TXMLEnginePtr g_pXMLEngine;
	CModuleInfo::THTMLEnginePtr g_pHTMLEngine;
	CLightMutex g_lmParsers;
}

CModuleInfo::CModuleInfo()
	: m_bExtendedStatusInfo(1 == db_get_b(NULL, QUOTES_MODULE_NAME, "ExtendedStatus", false))
{
}

CModuleInfo::~CModuleInfo()
{
}

CModuleInfo& CModuleInfo::GetInstance()
{
	static CModuleInfo mi;
	return mi;
}

HANDLE CModuleInfo::GetWindowList(const std::string& rsKey, bool bAllocateIfNonExist /*= true*/)
{
	HANDLE hResult = NULL;
	THandles::const_iterator i = m_ahWindowLists.find(rsKey);
	if (i != m_ahWindowLists.end())
	{
		hResult = i->second;
	}
	else if (bAllocateIfNonExist)
	{
		hResult = WindowList_Create();
		if (hResult)
			m_ahWindowLists.insert(std::make_pair(rsKey, hResult));
	}

	return hResult;
}

void CModuleInfo::OnMirandaShutdown()
{
	BOOST_FOREACH(THandles::value_type p, m_ahWindowLists)
	{
		WindowList_Broadcast(p.second, WM_CLOSE, 0, 0);
	}
}

CModuleInfo::TQuotesProvidersPtr CModuleInfo::GetQuoteProvidersPtr()
{
	static TQuotesProvidersPtr pProviders(new CQuotesProviders);
	return pProviders;
}

CModuleInfo::TXMLEnginePtr CModuleInfo::GetXMLEnginePtr()
{
	if (!g_pXMLEngine)
	{
		CGuard<CLightMutex> cs(g_lmParsers);
		if (!g_pXMLEngine)
		{
			mir_getXI(&xi);
			g_pXMLEngine = TXMLEnginePtr(new CXMLEngineMI);
		}
	}

	return g_pXMLEngine;
}

// void CModuleInfo::SetXMLEnginePtr(TXMLEnginePtr pEngine)
// {
// 	g_pXMLEngine = pEngine;
// }

CModuleInfo::THTMLEnginePtr CModuleInfo::GetHTMLEngine()
{
	if (!g_pHTMLEngine)
	{
		CGuard<CLightMutex> cs(g_lmParsers);
		if (!g_pHTMLEngine)
		{
			g_pHTMLEngine = THTMLEnginePtr(new CHTMLEngineMS);
		}
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
	{
		return false;
	}

	if (!GetXMLEnginePtr())
	{
		Quotes_MessageBox(NULL, TranslateT("Miranda could not load Quotes plugin. XML parser is missing."), MB_OK | MB_ICONERROR);
		return false;
	}

	if (!g_pHTMLEngine && (false == CHTMLParserMS::IsInstalled()))
	{
		Quotes_MessageBox(NULL,
			TranslateT("Miranda could not load Quotes plugin. Microsoft HTML parser is missing."),
			MB_YESNO | MB_ICONQUESTION);
		return false;
	}

	return true;
}

bool CModuleInfo::GetExtendedStatusFlag()const
{
	return m_bExtendedStatusInfo;
}
