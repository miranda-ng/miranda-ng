#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

class HttpRequest : protected NETLIBHTTPREQUEST, public MZeroedObject
{
protected:
	CMStringA url;

	HttpRequest()
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
	}

	HttpRequest(int httpMethod, LPCSTR urlFormat, va_list args)
	{
		this->HttpRequest::HttpRequest();

		requestType = httpMethod;
		flags = NLHRF_HTTP11 | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;

		url.AppendFormatV(urlFormat, args);
		szUrl = url.GetBuffer();
	}

	void AddHeader(LPCSTR szName, LPCSTR szValue)
	{
		headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
		headers[headersCount].szName = mir_strdup(szName);
		headers[headersCount].szValue = mir_strdup(szValue);
		headersCount++;
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
	HttpRequest(int type, LPCSTR urlFormat, ...)
	{
		va_list args;
		va_start(args, urlFormat);
		this->HttpRequest::HttpRequest(type, urlFormat, args);
		va_end(args);
	}

	~HttpRequest()
	{
		for (int i = 0; i < headersCount; i++)
		{
			mir_free(headers[i].szName);
			mir_free(headers[i].szValue);
		}
		mir_free(headers);
		mir_free(pData);
	}

	void SetCookie(LPCSTR szValue)
	{
		AddHeader("Set-Cookie", szValue);
	}

	NETLIBHTTPREQUEST * Send(HANDLE hConnection)
	{
		if (url.Find("http", 0) != 0)
			url.Insert(0, flags & NLHRF_SSL ? "https://" : "http://");
		szUrl = url.GetBuffer();

		char message[1024];
		mir_snprintf(message, SIZEOF(message), "Send request to %s", szUrl);
		CallService(MS_NETLIB_LOG, (WPARAM)hConnection, (LPARAM)&message);

		return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hConnection, (LPARAM)this);
	}
};

#endif //_HTTP_REQUEST_H_