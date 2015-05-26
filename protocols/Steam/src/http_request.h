#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

class HttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
private:
	CMStringA m_url;

protected:
	enum HttpRequestUrlFormat { FORMAT };

	void Init(int type)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		requestType = type;
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
		AddHeader("user-agent", "Steam 1.2.0 / iPhone");
	}

	void AddHeader(LPCSTR szName, LPCSTR szValue)
	{
		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
		headers[headersCount].szName = mir_strdup(szName);
		headers[headersCount].szValue = mir_strdup(szValue);
		headersCount++;
	}

	void AddParameter(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_url += m_url.Find('?') == -1 ? '?' : '&';
		m_url.AppendFormatV(fmt, args);
		va_end(args);
	}

	void AddParameter(LPCSTR name, LPCSTR value)
	{
		AddParameter("%s=%s", name, value);
	}

	void SetData(const char *data, size_t size)
	{
		if (pData != NULL)
			mir_free(pData);

		dataLength = (int)size;
		pData = (char*)mir_alloc(size + 1);
		memcpy(pData, data, size);
		pData[size] = 0;
	}

public:
	HttpRequest(int type, LPCSTR url)
	{
		Init(type);

		m_url = url;
	}

	HttpRequest(int type, HttpRequestUrlFormat, LPCSTR urlFormat, ...)
	{
		Init(type);

		va_list formatArgs;
		va_start(formatArgs, urlFormat);
		m_url.AppendFormatV(urlFormat, formatArgs);
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
		
		if (pData != NULL)
			mir_free(pData);
	}

	NETLIBHTTPREQUEST* Send(HANDLE hConnection)
	{
		szUrl = m_url.GetBuffer();

		char message[1024];
		mir_snprintf(message, SIZEOF(message), "Send request to %s", szUrl);
		CallService(MS_NETLIB_LOG, (WPARAM)hConnection, (LPARAM)&message);

		return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hConnection, (LPARAM)this);
	}
};

#endif //_HTTP_REQUEST_H_