#include "StdAfx.h"

class CHTTPSession::CImpl
{
public:
	CImpl() {}
	virtual ~CImpl() {}

	virtual bool OpenURL(const tstring& rsURL) = 0;
	virtual bool ReadResponce(tstring& rsResponce)const = 0;
};

namespace
{
	// 	class CImplMS : public CHTTPSession::CImpl
	// 	{
	// 	public:
	// 		CImplMS() 
	// 			: m_hSession(::InternetOpen(_T("Dioksin"),PRE_CONFIG_INTERNET_ACCESS,NULL,INTERNET_INVALID_PORT_NUMBER,0)),
	// 			m_hRequest(NULL)
	// 		{
	// 
	// 		}
	// 
	// 		~CImplMS()
	// 		{
	// 			if(m_hRequest)
	// 			{
	// 				::InternetCloseHandle(m_hRequest);
	// 			}
	// 
	// 			if(m_hSession)
	// 			{
	// 				::InternetCloseHandle(m_hSession);
	// 			}
	// 		}
	// 
	// 		virtual bool OpenURL(const tstring& rsURL)
	// 		{
	// 			if(NULL == m_hSession)
	// 			{
	// 				return false;
	// 			}
	// 
	// 			if(NULL != m_hRequest)
	// 			{
	// 				::InternetCloseHandle(m_hRequest);
	// 				m_hRequest = NULL;
	// 			}
	// 
	// 			m_hRequest = ::InternetOpenUrl(m_hSession,rsURL.c_str(),NULL,0,INTERNET_FLAG_RELOAD,0);
	// 			return NULL != m_hRequest;
	// 		}
	// 
	// 		virtual bool ReadResponce(tstring& rsResponce)const
	// 		{
	// 			if(NULL == m_hRequest)
	// 			{
	// 				return false;
	// 			}
	// 
	// 			std::string sBuffer;
	// 			bool bResult = true;
	// 			DWORD cbRead = 0;
	// 			char szBuffer[1024];
	// 			do{
	// 				if(FALSE == ::InternetReadFile(m_hRequest,szBuffer,1024,&cbRead))
	// 				{
	// 					bResult = false;
	// 					break;
	// 				}
	// 				if (0 == cbRead)
	// 				{
	// 					break;  // Stop.
	// 				}
	// 				else
	// 				{
	// 					sBuffer.insert(sBuffer.size(),szBuffer,cbRead);
	// 				}
	// 			}while(true);
	// 
	// 			if(true == bResult)
	// 			{
	// 				USES_CONVERSION;
	// 				rsResponce = A2CT(sBuffer.c_str());
	// 			}
	// 
	// 			return bResult;
	// 		}
	// 	private:
	// 		HINTERNET m_hSession;
	// 		HINTERNET m_hRequest;
	// 	};
	// 
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

			NETLIBUSER nlu = { 0 };
			nlu.cbSize = sizeof(nlu);
			nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_TCHAR;
			nlu.szSettingsModule = QUOTES_PROTOCOL_NAME;
			nlu.ptszDescriptiveName = TranslateT("Quotes HTTP connections");
			g_hNetLib = reinterpret_cast<HANDLE>(CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu));
			return (NULL != g_hNetLib);
		}

		static bool IsValid(){ return NULL != g_hNetLib; }

		virtual bool OpenURL(const tstring& rsURL)
		{
			// 			USES_CONVERSION;

			m_aURL.swap(TBuffer());

			std::string s = quotes_t2a(rsURL.c_str());
			const char* psz = s.c_str();//T2CA(rsURL.c_str());
			m_aURL.insert(m_aURL.begin(), psz, psz + mir_strlen(psz) + 1);
			return true;

		}
		virtual bool ReadResponce(tstring& rsResponce)const
		{
			if (true == m_aURL.empty())
			{
				return false;
			}


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
				pReply = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService(MS_NETLIB_HTTPTRANSACTION,
					reinterpret_cast<WPARAM>(g_hNetLib), reinterpret_cast<LPARAM>(&nlhr)));
			}

			if (pReply)
			{
				if ((200 == pReply->resultCode) && (pReply->dataLength > 0))
				{
					TBuffer apBuffer;
					apBuffer.insert(apBuffer.begin(), pReply->pData, pReply->pData + pReply->dataLength);
					apBuffer.push_back('\0');

					char* pResult = &*(apBuffer.begin());
					int nIndex = find_header(pReply, "Content-Type");
					if ((-1 != nIndex) && (NULL != strstr(_strlwr(pReply->headers[nIndex].szValue), "utf-8")))
					{
						TCHAR* p = mir_utf8decodeT(pResult);
						rsResponce = p;
						mir_free(p);
					}
					else
					{
						// 						USES_CONVERSION;
						// 						LPCTSTR p = A2CT(pResult);
						rsResponce = quotes_a2t(pResult);//p;
					}

					bResult = true;
				}

				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, reinterpret_cast<LPARAM>(pReply));
			}

			mir_free(nlhr.headers);

			return bResult;
		}

	private:
		static HANDLE g_hNetLib;
		typedef std::vector<char> TBuffer;
		mutable TBuffer m_aURL;
		mutable mir_cs m_mx;
	};

	HANDLE CImplMI::g_hNetLib = NULL;

	// 	CHTTPSession::CImpl* create_impl()
	// 	{
	// 		if(true == CImplMI::IsValid())
	// 		{
	// 			return new CImplMI;
	// 		}
	// 		else
	// 		{
	// 			return new CImplMS;
	// 		}
	// 	}
}


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