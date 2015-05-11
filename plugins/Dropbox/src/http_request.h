#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

enum HTTP_STATUS
{
	HTTP_STATUS_ERROR = 0,
	HTTP_STATUS_OK = 200,
	HTTP_STATUS_BAD_REQUEST = 400,
	HTTP_STATUS_UNAUTHORIZED = 401,
	HTTP_STATUS_FORBIDDEN = 403,
	HTTP_STATUS_NOT_FOUND = 404,
	HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
	HTTP_STATUS_TOO_MANY_REQUESTS = 429,
	HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
	HTTP_STATUS_INSUFICIENTE_STORAGE = 507
};

class HttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
public:
	HttpRequest(HANDLE hNetlibConnection, int requestType, LPCSTR url)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		flags = NLHRF_HTTP11;
		this->requestType = requestType;

		m_hNetlibConnection = hNetlibConnection;
		m_szUrl = mir_strdup(url);
	}

	~HttpRequest()
	{
		for (int i = 0; i < headersCount; i++) {
			mir_free(headers[i].szName);
			mir_free(headers[i].szValue);
		}
		mir_free(headers);
		mir_free(pData);
	}


	void AddHeader(LPCSTR szName, LPCSTR szValue)
	{
		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
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

		char *ePair = (char *)mir_base64_encode((BYTE*)cPair, (UINT)strlen(cPair));

		char value[128];
		mir_snprintf(
			value,
			SIZEOF(value),
			"Basic %s",
			ePair);

		mir_free(ePair);

		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
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

		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
		headers[headersCount].szName = mir_strdup("Authorization");
		headers[headersCount].szValue = mir_strdup(value);
		headersCount++;
	}

	NETLIBHTTPREQUEST *Send()
	{
		szUrl = m_szUrl.GetBuffer();
		return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibConnection, (LPARAM)this);
	}

private:
	CMStringA m_szUrl;
	HANDLE m_hNetlibConnection;
};

#endif //_HTTP_REQUEST_H_