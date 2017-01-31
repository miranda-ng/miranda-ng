#include "StdAfx.h"

class CHTTPSession::CImpl
{
public:
	CImpl() {}
	virtual ~CImpl() {}

	virtual bool OpenURL(const tstring& rsURL) = 0;
	virtual bool ReadResponce(tstring& rsResponce)const = 0;
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
		assert(NULL == g_hNetLib);

		NETLIBUSER nlu = {};
		nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
		nlu.szSettingsModule = QUOTES_PROTOCOL_NAME;
		nlu.szDescriptiveName.w = TranslateT("Quotes HTTP connections");
		g_hNetLib = Netlib_RegisterUser(&nlu);
		return (NULL != g_hNetLib);
	}

	static bool IsValid() { return NULL != g_hNetLib; }

	virtual bool OpenURL(const tstring& rsURL)
	{
		m_aURL.clear();

		std::string s = quotes_t2a(rsURL.c_str());
		const char* psz = s.c_str();
		m_aURL.insert(m_aURL.begin(), psz, psz + mir_strlen(psz) + 1);
		return true;
	}

	virtual bool ReadResponce(tstring& rsResponce)const
	{
		if (true == m_aURL.empty())
			return false;

		NETLIBHTTPREQUEST nlhr = { 0 };
		nlhr.cbSize = sizeof(nlhr);
		nlhr.requestType = REQUEST_GET;
		nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
		char* pURL = &*(m_aURL.begin());
		nlhr.szUrl = pURL;

		nlhr.headersCount = 4;
		nlhr.headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
		nlhr.headers[0].szName = "User-Agent";
		nlhr.headers[0].szValue = NETLIB_USER_AGENT;
		nlhr.headers[1].szName = "Connection";
		nlhr.headers[1].szValue = "close";
		nlhr.headers[2].szName = "Cache-Control";
		nlhr.headers[2].szValue = "no-cache";
		nlhr.headers[3].szName = "Pragma";
		nlhr.headers[3].szValue = "no-cache";
		// 			nlhr.headers[4].szName  = "Accept-Encoding";
		// 			nlhr.headers[4].szValue = "deflate, gzip";
		// 			nlhr.headers[5].szName  = "Cookie";
		// 			nlhr.headers[5].szValue = cookie;

		bool bResult = false;
		NETLIBHTTPREQUEST* pReply = NULL;
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
				if ((-1 != nIndex) && (NULL != strstr(_strlwr(pReply->headers[nIndex].szValue), "utf-8"))) {
					wchar_t* p = mir_utf8decodeW(pResult);
					rsResponce = p;
					mir_free(p);
				}
				else {
					// 						USES_CONVERSION;
					// 						LPCTSTR p = A2CT(pResult);
					rsResponce = quotes_a2t(pResult);//p;
				}

				bResult = true;
			}

			Netlib_FreeHttpRequest(pReply);
		}

		mir_free(nlhr.headers);

		return bResult;
	}

private:
	static HNETLIBUSER g_hNetLib;
	typedef std::vector<char> TBuffer;
	mutable TBuffer m_aURL;
	mutable mir_cs m_mx;
};

HNETLIBUSER CImplMI::g_hNetLib = NULL;

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

bool CHTTPSession::ReadResponce(tstring& rsResponce)const
{
	return m_pImpl->ReadResponce(rsResponce);
}

bool CHTTPSession::Init()
{
	return CImplMI::Init();
}