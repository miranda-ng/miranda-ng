#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

struct VALUE
{
	LPCSTR szName;
	__forceinline VALUE(LPCSTR _name) : szName(_name) { }
};

struct INT_VALUE : public VALUE
{
	int iValue;
	__forceinline INT_VALUE(LPCSTR _name, int _value)
		: VALUE(_name), iValue(_value) { }
};

struct CHAR_VALUE : public VALUE
{
	LPCSTR szValue;
	__forceinline CHAR_VALUE(LPCSTR _name, LPCSTR _value)
		: VALUE(_name), szValue(_value) { }
};

struct FORMAT_VALUE : public VALUE
{
	CMStringA szValue;
	__forceinline FORMAT_VALUE(LPCSTR _name, LPCSTR _valueFormat, ...)
		: VALUE(_name)
	{
		va_list args;
		va_start(args, _valueFormat);
		szValue.AppendFormatV(_valueFormat, args);
		va_end(args);
	}
};

class HttpRequest : protected NETLIBHTTPREQUEST, public MZeroedObject
{
protected:
	class HttpRequestUrl
	{
		friend HttpRequest;

	private:
		CMStringA content;

		void AppendSeparator()
		{
			if (!content.IsEmpty())
			{
				if (content.Find("?") == -1)
					content.AppendChar('?');
				else
					content.AppendChar('&');
			}
		}

	public:
		HttpRequestUrl & operator<<(const VALUE &param)
		{
			AppendSeparator();
			content.Append(param.szName);
			return *this;
		}

		HttpRequestUrl & operator<<(const INT_VALUE &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%i", param.szName, param.iValue);
			return *this;
		}

		HttpRequestUrl & operator<<(const CHAR_VALUE &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%s", param.szName, param.szValue);
			return *this;
		}

		char * ToString()
		{
			return content.GetBuffer();
		}
	};

	class HttpRequestHeaders
	{
	private:
		HttpRequest &request;

		void Add(LPCSTR szName)
		{
			Add(szName, "");
		}

		void Add(LPCSTR szName, LPCSTR szValue)
		{
			request.headers = (NETLIBHTTPHEADER*)mir_realloc(
				request.headers, sizeof(NETLIBHTTPHEADER) * (request.headersCount + 1));
			request.headers[request.headersCount].szName = mir_strdup(szName);
			request.headers[request.headersCount].szValue = mir_strdup(szValue);
			request.headersCount++;
		}

	public:
		HttpRequestHeaders(HttpRequest &request) : request(request) { }

		HttpRequestHeaders & operator<<(const VALUE &param)
		{
			Add(param.szName);
			return *this;
		}

		HttpRequestHeaders & operator<<(const CHAR_VALUE &param)
		{
			Add(param.szName, param.szValue);
			return *this;
		}
	};

	class HttpRequestBody
	{
	private:
		CMStringA content;

		void AppendSeparator()
		{
			if (!content.IsEmpty())
			{
				content.AppendChar('&');
			}
		}

	public:
		HttpRequestBody() { }

		HttpRequestBody & operator<<(const VALUE &param)
		{
			AppendSeparator();
			content.Append(param.szName);
			return *this;
		}

		HttpRequestBody & operator<<(const INT_VALUE &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%i", param.szName, param.iValue);
			return *this;
		}

		HttpRequestBody & operator<<(const CHAR_VALUE &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%s", param.szName, param.szValue);
			return *this;
		}

		HttpRequestBody & operator<<(const FORMAT_VALUE &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%s", param.szName, param.szValue);
			return *this;
		}

		char * ToString()
		{
			return content.GetBuffer();
		}
	};

	HttpRequest() : Headers(*this)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		flags = NLHRF_HTTP11 | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
	}

	HttpRequest(int httpMethod, LPCSTR urlFormat, va_list args)
		: Headers(*this)
	{
		requestType = httpMethod;
		Url.content.AppendFormatV(urlFormat, args);
	}

public:
	HttpRequestUrl Url;
	HttpRequestHeaders Headers;
	HttpRequestBody Body;

	HttpRequest(int type, LPCSTR urlFormat, ...)
		: Headers(*this)
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
		//mir_free(pData);
	}

	NETLIBHTTPREQUEST * Send(HANDLE hConnection)
	{
		if (Url.content.Find("://") == -1)
			Url.content.Insert(0, flags & NLHRF_SSL ? "https://" : "http://");
		szUrl = Url.ToString();

		pData = Body.ToString();
		dataLength = mir_strlen(pData);

		char message[1024];
		mir_snprintf(message, SIZEOF(message), "Send request to %s", szUrl);
		CallService(MS_NETLIB_LOG, (WPARAM)hConnection, (LPARAM)&message);

		return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hConnection, (LPARAM)this);
	}
};

class HttpGetRequest : public HttpRequest
{
public:
	HttpGetRequest(LPCSTR urlFormat, ...)
	{
		va_list args;
		va_start(args, urlFormat);
		this->HttpRequest::HttpRequest(REQUEST_GET, urlFormat, args);
		va_end(args);
	}
};

class HttpPostRequest : public HttpRequest
{
public:
	HttpPostRequest(LPCSTR urlFormat, ...)
	{
		va_list args;
		va_start(args, urlFormat);
		this->HttpRequest::HttpRequest(REQUEST_POST, urlFormat, args);
		va_end(args);

		//Headers << CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
	}
};

class HttpsRequest : public HttpRequest
{
protected:
	HttpsRequest() : HttpRequest()
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
	}

public:
	HttpsRequest(int type, LPCSTR urlFormat, ...)
	{
		va_list args;
		va_start(args, urlFormat);
		this->HttpRequest::HttpRequest(type, urlFormat, args);
		va_end(args);

		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
	}
};

class HttpsGetRequest : public HttpsRequest
{
public:
	HttpsGetRequest(LPCSTR urlFormat, ...)
	{
		va_list args;
		va_start(args, urlFormat);
		this->HttpRequest::HttpRequest(REQUEST_GET, urlFormat, args);
		va_end(args);
	}
};

class HttpsPostRequest : public HttpsRequest
{
public:
	HttpsPostRequest(LPCSTR urlFormat, ...)
	{
		va_list args;
		va_start(args, urlFormat);
		this->HttpRequest::HttpRequest(REQUEST_POST, urlFormat, args);
		va_end(args);

		//Headers << CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
	}
};

#endif //_HTTP_REQUEST_H_