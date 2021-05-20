#include "StdAfx.h"

HNETLIBUSER CHTTPSession::g_hNetLib = nullptr;

#define ERROR_MSG LPGENW("This plugin requires a personal key. Press Yes to obtain it at the site and then enter the result in the Options dialog, otherwise this plugin will fail.")

void CALLBACK waitStub()
{
	if (IDYES == MessageBox(0, TranslateW(ERROR_MSG), _A2W(MODULENAME), MB_YESNOCANCEL))
		Utils_OpenUrl("https://free.currencyconverterapi.com/free-api-key");
}

static int find_header(const NETLIBHTTPREQUEST* pRequest, const char* hdr)
{
	for (int i = 0; i < pRequest->headersCount; ++i)
		if (0 == _stricmp(pRequest->headers[i].szName, hdr))
			return i;

	return -1;
}

bool CHTTPSession::OpenURL(const CMStringW &rsURL)
{
	m_szUrl = rsURL;
	return true;
}

bool CHTTPSession::ReadResponce(CMStringW &rsResponce)
{
	if (m_szUrl.IsEmpty())
		return false;

	NETLIBHTTPHEADER headers[] =
	{
		{ "User-Agent", NETLIB_USER_AGENT },
		{ "Connection", "close" },
		{ "Cache-Control", "no-cache" },
		{ "Pragma", "no-cache" }
	};

	NETLIBHTTPREQUEST nlhr = {};
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	nlhr.szUrl = m_szUrl.GetBuffer();
	nlhr.headersCount = _countof(headers);
	nlhr.headers = headers;

	bool bResult = false;
	NLHR_PTR pReply(0);
	{
		mir_cslock lck(m_mx);
		pReply = Netlib_HttpTransaction(g_hNetLib, &nlhr);
	}

	if (pReply) {
		if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
			CMStringA buf(pReply->pData, pReply->dataLength);
			int nIndex = find_header(pReply, "Content-Type");
			if ((-1 != nIndex) && (nullptr != strstr(_strlwr(pReply->headers[nIndex].szValue), "utf-8")))
				rsResponce = ptrW(mir_utf8decodeW(buf));
			else
				rsResponce = _A2T(buf);

			bResult = true;
		}
	}
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// module initialization

bool CHTTPSession::Init()
{
	assert(nullptr == g_hNetLib);

	ptrA szApiKey(g_plugin.getStringA(DB_KEY_ApiKey));
	if (szApiKey == nullptr)
		Miranda_WaitOnHandle(waitStub);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.w = TranslateT("CurrencyRates HTTP connections");
	g_hNetLib = Netlib_RegisterUser(&nlu);
	return (nullptr != g_hNetLib);
}
