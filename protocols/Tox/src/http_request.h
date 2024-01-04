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
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
		szUrl = nullptr;
		headers = nullptr;
		headersCount = 0;
		pData = nullptr;
		dataLength = 0;
		resultCode = 0;
		szResultDescr = nullptr;
		nlc = nullptr;
		timeout = 0;
	}

protected:
	enum HttpRequestUrlFormat { FORMAT };

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

	void AddHeader(LPCSTR szName, LPCSTR szValue)
	{
		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER) * (headersCount + 1));
		headers[headersCount].szName = mir_strdup(szName);
		headers[headersCount].szValue = mir_strdup(szValue);
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

	void SetData(const char *data, size_t size)
	{
		if (pData != nullptr)
			mir_free(pData);

		dataLength = (int)size;
		pData = (char*)mir_alloc(size);
		memcpy(pData, data, size);
	}

	NETLIBHTTPREQUEST* Send(HNETLIBUSER hNetlibConnection)
	{
		m_szUrl.Replace('\\', '/');
		szUrl = m_szUrl.GetBuffer();
		return Netlib_HttpTransaction(hNetlibConnection, this);
	}
};

#endif //_HTTP_REQUEST_H_