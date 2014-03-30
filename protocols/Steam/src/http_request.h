#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include "steam.h"

enum HTTP_STATUS
{
	HTTP_STATUS_OK = 200/*,
	HTTP_STATUS_BAD_REQUEST = 400,
	HTTP_STATUS_UNAUTHORIZED = 401,
	HTTP_STATUS_FORBIDDEN = 403,
	HTTP_STATUS_NOT_FOUND = 404,
	HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
	HTTP_STATUS_TOO_MANY_REQUESTS = 429,
	HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
	HTTP_STATUS_INSUFICIENTE_STORAGE = 507*/
};

class HttpRequest : private NETLIBHTTPREQUEST//, public MZeroedObject
{
public:
	HttpRequest(HANDLE hNetlibUser, int request, LPCSTR url)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		pData = NULL;
		szUrl = NULL;
		headers = NULL;
		dataLength = 0;
		headersCount = 0;
		szResultDescr = NULL;
		flags = NLHRF_HTTP11;
		requestType = request;

		m_hNetlibUser = hNetlibUser;
		m_szUrl = mir_strdup(url);
	}

	~HttpRequest()
	{
		if (headers != NULL)
		{
			for (int i = 0; i < headersCount; i++)
			{
				mir_free(headers[i].szName);
				mir_free(headers[i].szValue);
			}
			mir_free(headers);
		}
		if (pData != NULL)
			mir_free(pData);
	}


	void AddHeader(LPCSTR szName, LPCSTR szValue)
	{
		if (headers == NULL)
			headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER));
		else
			headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER) * (headersCount + 1));
		headers[headersCount].szName = mir_strdup(szName);
		headers[headersCount].szValue = mir_strdup(szValue);
		headersCount++;
	}

	void SetData(const char *data, size_t size)
	{
		if (pData != NULL)
			mir_free(pData);
		
		dataLength = size;
		pData = (char*)mir_alloc(size);
		memcpy(pData, data, size);
	}

	/*void AddBasicAuthHeader(LPCSTR szLogin, LPCSTR szPassword)
	{
		char cPair[128];
		mir_snprintf(
			cPair,
			SIZEOF(cPair),
			"%s:%s",
			szLogin,
			szPassword);

		char *ePair = (char *)mir_base64_encode((BYTE*)cPair, (UINT)strlen(cPair));

		char value[128];
		mir_snprintf(
			value,
			SIZEOF(value),
			"Basic %s",
			ePair);

		mir_free(ePair);

		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount+1));
		headers[headersCount].szName = mir_strdup("Authorization");
		headers[headersCount].szValue = mir_strdup(value);
		headersCount++;
	}

	void AddBearerAuthHeader(LPCSTR szValue)
	{
		char value[128];
		mir_snprintf(
			value,
			SIZEOF(value),
			"Bearer %s",
			szValue);

		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount+1));
		headers[headersCount].szName = mir_strdup("Authorization");
		headers[headersCount].szValue = mir_strdup(value);
		headersCount++;
	}*/

	void AddUrlPart(LPCSTR szPart)
	{
		m_szUrl += szPart;
	}

	void AddParameter(LPCSTR szName, LPCSTR szValue)
	{
		if (m_szUrl.Find('?') == -1)
			m_szUrl.AppendFormat("?%s=%s", szName, szValue);
		else
			m_szUrl.AppendFormat("&%s=%s", szName, szValue);
	}

	/*void AddParameter(LPCSTR szName, int value)
	{
		if (m_szUrl.Find('?') == -1)
			m_szUrl.AppendFormat("?%s=%i", szName, value);
		else
			m_szUrl.AppendFormat("&%s=%i", szName, value);
	}*/

	NETLIBHTTPREQUEST *Send()
	{
		szUrl = m_szUrl.GetBuffer();
		/*CMStringA message; message.AppendFormat("Send request to %s", szUrl);
		CallService(MS_NETLIB_LOG, (WPARAM)m_hNetlibUser, (LPARAM)message.GetBuffer());*/
		return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)this);
	}

private:
	CMStringA m_szUrl;
	HANDLE m_hNetlibUser;
};

#endif //_HTTP_REQUEST_H_