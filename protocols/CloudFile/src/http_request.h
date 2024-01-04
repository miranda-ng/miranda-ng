#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

class HttpRequestException
{
	CMStringA message;

public:
	HttpRequestException(const char *message) :
		message(message)
	{
	}

	const char* what() const throw()
	{
		return message.c_str();
	}
};

class HttpRequest : protected NETLIBHTTPREQUEST
{
private:
	CMStringA m_szUrl;

	void Init(int type)
	{
		requestType = type;
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;
		szUrl = NULL;
		headers = NULL;
		headersCount = 0;
		pData = NULL;
		dataLength = 0;
		resultCode = 0;
		szResultDescr = NULL;
		nlc = NULL;
		timeout = 0;
	}

protected:
	enum HttpRequestUrlFormat { FORMAT };

	void AddHeader(LPCSTR szName, LPCSTR szValue)
	{
		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER) * (headersCount + 1));
		headers[headersCount].szName = mir_strdup(szName);
		headers[headersCount].szValue = mir_strdup(szValue);
		headersCount++;
	}

	void AddBasicAuthHeader(LPCSTR szLogin, LPCSTR szPassword)
	{
		size_t length = mir_strlen(szLogin) + mir_strlen(szPassword) + 1;
		ptrA cPair((char*)mir_calloc(length + 1));
		mir_snprintf(
			cPair,
			length,
			"%s:%s",
			szLogin,
			szPassword);

		ptrA ePair(mir_base64_encode(cPair, length));

		length = mir_strlen(ePair) + 7;
		char *value = (char*)mir_calloc(length + 1);
		mir_snprintf(value, length, "Basic %s", ePair.get());

		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
		headers[headersCount].szName = mir_strdup("Authorization");
		headers[headersCount].szValue = value;
		headersCount++;
	}

	void AddBearerAuthHeader(LPCSTR szValue)
	{
		size_t length = mir_strlen(szValue) + 8;
		char *value = (char*)mir_calloc(length + 1);
		mir_snprintf(
			value,
			length,
			"Bearer %s",
			szValue);

		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
		headers[headersCount].szName = mir_strdup("Authorization");
		headers[headersCount].szValue = value;
		headersCount++;
	}

	void AddOAuthHeader(LPCSTR szValue)
	{
		size_t length = mir_strlen(szValue) + 7;
		char *value = (char*)mir_calloc(length + 1);
		mir_snprintf(
			value,
			length,
			"OAuth %s",
			szValue);

		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
		headers[headersCount].szName = mir_strdup("Authorization");
		headers[headersCount].szValue = value;
		headersCount++;
	}

	void AddUrlParameter(const char *urlFormat, ...)
	{
		va_list urlArgs;
		va_start(urlArgs, urlFormat);
		m_szUrl += m_szUrl.Find('?') == -1 ? '?' : '&';
		m_szUrl.AppendFormatV(urlFormat, urlArgs);
		va_end(urlArgs);
	}

	void AddUrlParameterWithEncode(const char *name, const char *valueFormat, ...)
	{
		va_list valueArgs;
		va_start(valueArgs, valueFormat);
		m_szUrl += m_szUrl.Find('?') == -1 ? '?' : '&';
		m_szUrl.AppendFormat("%s=", name);
		CMStringA value;
		value.AppendFormatV(valueFormat, valueArgs);
		m_szUrl += mir_urlEncode(value);
		va_end(valueArgs);
	}

	void SetData(const char *data, size_t size)
	{
		if (pData != NULL)
			mir_free(pData);

		dataLength = (int)size;
		pData = (char*)mir_alloc(size);
		memcpy(pData, data, size);
	}

public:
	HttpRequest(int type, LPCSTR url)
	{
		Init(type);

		m_szUrl = url;
	}

	HttpRequest(int type, HttpRequestUrlFormat, LPCSTR urlFormat, ...)
	{
		Init(type);

		va_list formatArgs;
		va_start(formatArgs, urlFormat);
		m_szUrl.AppendFormatV(urlFormat, formatArgs);
		va_end(formatArgs);
	}

	~HttpRequest()
	{
		for (int i = 0; i < headersCount; i++)
		{
			mir_free(headers[i].szName);
			mir_free(headers[i].szValue);
		}
		mir_free(headers);
		if (pData)
			mir_free(pData);
	}

	NETLIBHTTPREQUEST* Send(HNETLIBUSER hConnection)
	{
		m_szUrl.Replace('\\', '/');
		szUrl = m_szUrl.GetBuffer();
		return Netlib_HttpTransaction(hConnection, this);
	}
};

#endif //_HTTP_REQUEST_H_