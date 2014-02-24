#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include "common.h"

enum HTTP_STATUS
{
	OK = 200,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	TOO_MANY_REQUESTS = 429,
	SERVICE_UNAVAILABLE = 503,
	INSUFICIENTE_STORAGE = 507
};

class HttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
public:
	HttpRequest(HANDLE hNetlibUser, int requestType, LPCSTR url)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		flags = NLHRF_HTTP11;
		this->requestType = requestType;

		m_hNetlibUser = hNetlibUser;
		m_szUrl = mir_strdup(url);
	}

	~HttpRequest()
	{
		for (int i=0; i < headersCount; i++)
		{
			mir_free(headers[i].szName);
			mir_free(headers[i].szValue);
		}
		mir_free(headers);
		mir_free(pData);
	}


	void AddHeader(LPCSTR szName, LPCSTR szValue)
	{
		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount+1));
		headers[headersCount].szName = mir_strdup(szName);
		headers[headersCount].szValue = mir_strdup(szValue);
		headersCount++;
	}

	void AddBasicAuthHeader(LPCSTR szLogin, LPCSTR szPassword)
	{
		char cPair[128];
		mir_snprintf(
			cPair,
			SIZEOF(cPair),
			"%s:%s",
			szLogin,
			szPassword);

		char *ePair = (char *)mir_base64_encode((BYTE*)cPair, strlen(cPair));

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
	}

	/*void AddParameter(LPCSTR szName, LPCSTR szValue)
	{
		if (m_szUrl.Find('?') == -1)
			m_szUrl.AppendFormat("?%s=%s", szName, szValue);
		else
			m_szUrl.AppendFormat("&%s=%s", szName, szValue);
	}

	void AddParameter(LPCSTR szName, int value)
	{
		if (m_szUrl.Find('?') == -1)
			m_szUrl.AppendFormat("?%s=%i", szName, value);
		else
			m_szUrl.AppendFormat("&%s=%i", szName, value);
	}*/

	NETLIBHTTPREQUEST *Send()
	{
		szUrl = m_szUrl.GetBuffer();
		return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)this);
	}

private:
	CMStringA m_szUrl;
	HANDLE m_hNetlibUser;
};

#endif //_HTTP_REQUEST_H_