#include "StdAfx.h"

#define ERROR_MSG LPGENW("This plugin requires a personal key. Press Yes to obtain it at the site and then enter the result in the Options dialog, otherwise this plugin will fail")

void CALLBACK waitStub()
{
	if (IDYES == MessageBox(0, TranslateW(ERROR_MSG), _A2W(CURRENCYRATES_MODULE_NAME), MB_YESNOCANCEL))
		Utils_OpenUrl("https://free.currencyconverterapi.com/free-api-key");
}

class CHTTPSession::CImpl
{
public:
	CImpl() {}
	virtual ~CImpl() {}

	virtual bool OpenURL(const tstring &rsURL) = 0;
	virtual bool ReadResponce(tstring &rsResponce) const = 0;
};

int find_header(const NETLIBHTTPREQUEST* pRequest, const char* hdr)
{
	for (int i = 0; i < pRequest->headersCount; ++i)
	{
		if (0 == _stricmp(pRequest->headers[i].szName, hdr))
		{
			return i;
		}
	}

	return -1;
}
 
class CImplMI : public CHTTPSession::CImpl
{
public:
	CImplMI() {}

	static bool Init()
	{
		assert(nullptr == g_hNetLib);

		ptrA szApiKey(g_plugin.getStringA(DB_KEY_ApiKey));
		if (szApiKey == nullptr)
			Miranda_WaitOnHandle(waitStub);

		NETLIBUSER nlu = {};
		nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
		nlu.szSettingsModule = CURRENCYRATES_PROTOCOL_NAME;
		nlu.szDescriptiveName.w = TranslateT("CurrencyRates HTTP connections");
		g_hNetLib = Netlib_RegisterUser(&nlu);
		return (nullptr != g_hNetLib);
	}

	static bool IsValid() { return nullptr != g_hNetLib; }

	virtual bool OpenURL(const tstring& rsURL)
	{
		m_aURL.clear();

		std::string s = currencyrates_t2a(rsURL.c_str());
		const char* psz = s.c_str();
		m_aURL.insert(m_aURL.begin(), psz, psz + mir_strlen(psz) + 1);
		return true;
	}

	virtual bool ReadResponce(tstring &rsResponce) const
	{
		if (true == m_aURL.empty())
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
		nlhr.szUrl = &*(m_aURL.begin());
		nlhr.headersCount = _countof(headers);
		nlhr.headers = headers;

		bool bResult = false;
		NETLIBHTTPREQUEST *pReply = nullptr;
		{
			mir_cslock lck(m_mx);
			pReply = Netlib_HttpTransaction(g_hNetLib, &nlhr);
		}

		if (pReply) {
			if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
				TBuffer apBuffer;
				apBuffer.insert(apBuffer.begin(), pReply->pData, pReply->pData + pReply->dataLength);
				apBuffer.push_back('\0');

				char* pResult = &*(apBuffer.begin());
				int nIndex = find_header(pReply, "Content-Type");
				if ((-1 != nIndex) && (nullptr != strstr(_strlwr(pReply->headers[nIndex].szValue), "utf-8"))) {
					rsResponce = ptrW(mir_utf8decodeW(pResult));
				}
				else {
					rsResponce = currencyrates_a2t(pResult);
				}

				bResult = true;
			}

			Netlib_FreeHttpRequest(pReply);
		}
		return bResult;
	}

private:
	static HNETLIBUSER g_hNetLib;
	typedef std::vector<char> TBuffer;
	mutable TBuffer m_aURL;
	mutable mir_cs m_mx;
};

HNETLIBUSER CImplMI::g_hNetLib = nullptr;

CHTTPSession::CHTTPSession()
	: m_pImpl(new CImplMI)
{
}

CHTTPSession::~CHTTPSession()
{
}

bool CHTTPSession::OpenURL(const tstring& rsURL)
{
	return m_pImpl->OpenURL(rsURL);
}

bool CHTTPSession::ReadResponce(tstring& rsResponce) const
{
	return m_pImpl->ReadResponce(rsResponce);
}

bool CHTTPSession::Init()
{
	return CImplMI::Init();
}