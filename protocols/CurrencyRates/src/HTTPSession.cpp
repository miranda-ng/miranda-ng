#include "StdAfx.h"

HNETLIBUSER CHTTPSession::g_hNetLib = nullptr;

#define ERROR_MSG LPGENW("This plugin requires a personal key. Open the Options dialog to obtain it.")

bool CHTTPSession::OpenURL(const CMStringW &rsURL)
{
	m_szUrl = rsURL;
	return true;
}

bool CHTTPSession::ReadResponce(CMStringW &rsResponce)
{
	if (m_szUrl.IsEmpty())
		return false;

	MHttpRequest nlhr(REQUEST_GET);
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	nlhr.m_szUrl = m_szUrl;
	nlhr.AddHeader("User-Agent", NETLIB_USER_AGENT);
	nlhr.AddHeader("Connection", "close");
	nlhr.AddHeader("Cache-Control", "no-cache");
	nlhr.AddHeader("Pragma", "no-cache");

	bool bResult = false;
	NLHR_PTR pReply(0);
	{
		mir_cslock lck(m_mx);
		pReply = Netlib_HttpTransaction(g_hNetLib, &nlhr);
	}

	if (pReply) {
		if ((200 == pReply->resultCode) && !pReply->body.IsEmpty()) {
			auto *pEncoding = pReply->FindHeader("Content-Type");
			if (pEncoding && strstr(pEncoding, "utf-8"))
				rsResponce = ptrW(mir_utf8decodeW(pReply->body));
			else
				rsResponce = _A2T(pReply->body);

			bResult = true;
		}
	}
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// module initialization

static void CALLBACK waitStub()
{
	show_popup(g_pCurrentProvider, 0, -1, TranslateW(ERROR_MSG));
}

bool CHTTPSession::Init()
{
	assert(nullptr == g_hNetLib);

	if (mir_wstrlen(g_plugin.wszApiKey) == 0 && g_pCurrentProvider->HasAuth())
		Miranda_WaitOnHandle(waitStub);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.a = MODULENAME;
	g_hNetLib = Netlib_RegisterUser(&nlu);
	return (nullptr != g_hNetLib);
}
