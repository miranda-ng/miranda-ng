#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

typedef void (CSlackProto::*HttpCallback)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);

class HttpRequest;
class HttpResponse;

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

struct LONG_VALUE : public VALUE
{
	LONGLONG llValue;
	__forceinline LONG_VALUE(LPCSTR _name, LONGLONG _value)
		: VALUE(_name), llValue(_value) { }
};

struct CHAR_VALUE : public VALUE
{
	LPCSTR szValue;
	__forceinline CHAR_VALUE(LPCSTR _name, LPCSTR _value)
		: VALUE(_name), szValue(_value) { }
};

struct ENCODED_VALUE : public VALUE
{
	LPSTR szValue;
	__forceinline ENCODED_VALUE(LPCSTR _name, LPCSTR _value)
		: VALUE(_name) { szValue = mir_urlEncode(_value); }
	__forceinline ~ENCODED_VALUE() { mir_free(szValue); }
};

class HttpUri
{
	friend class HttpRequest;

private:
	CMStringA m_uri;
	NETLIBHTTPREQUEST *m_request;

	HttpUri(NETLIBHTTPREQUEST *request, const char *uri)
		: m_request(request), m_uri(uri)
	{
		if (m_request)
			m_request->szUrl = m_uri.GetBuffer();
	}

	HttpUri(NETLIBHTTPREQUEST *request, const char *urlFormat, va_list args)
		: m_request(request)
	{
		m_uri.AppendFormatV(urlFormat, args);
		if (m_request)
			m_request->szUrl = m_uri.GetBuffer();
	}

	void Add(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_uri += (m_uri.Find('?') == -1) ? '?' : '&';
		m_uri.AppendFormatV(fmt, args);
		va_end(args);
		
		if (m_request)
			m_request->szUrl = m_uri.GetBuffer();
	}

	~HttpUri()
	{
		if (m_request)
			m_request->szUrl = NULL;
	}

public:
	HttpUri& operator=(const HttpUri&); // to prevent copying;

	operator const char*()
	{
		return m_request
			? m_request->szUrl
			: NULL;
	}

	HttpUri &operator<<(const VALUE &param)
	{
		Add(param.szName);
		return *this;
	}

	HttpUri &operator<<(const INT_VALUE &param)
	{
		Add("%s=%i", param.szName, param.iValue);
		return *this;
	}

	HttpUri &operator<<(const LONG_VALUE &param)
	{
		Add("%s=%lld", param.szName, param.llValue);
		return *this;
	}

	HttpUri &operator<<(const CHAR_VALUE &param)
	{
		Add("%s=%s", param.szName, param.szValue);
		return *this;
	}

	HttpUri &operator<<(const ENCODED_VALUE &param)
	{
		Add("%s=%s", param.szName, param.szValue);
		return *this;
	}
};

class HttpHeaders
{
	friend class HttpRequest;
	friend class HttpResponse;

private:
	NETLIBHTTPREQUEST *m_request;

	HttpHeaders(NETLIBHTTPREQUEST *request)
		: m_request(request)
	{
	}

	void Add(LPCSTR szName)
	{
		Add(szName, "");
	}

	void Add(LPCSTR szName, LPCSTR szValue)
	{
		if (!m_request)
			return;

		m_request->headers = (NETLIBHTTPHEADER*)mir_realloc(m_request->headers,
			sizeof(NETLIBHTTPHEADER)*(m_request->headersCount + 1));
		m_request->headers[m_request->headersCount].szName = mir_strdup(szName);
		m_request->headers[m_request->headersCount].szValue = mir_strdup(szValue);
		m_request->headersCount++;
	}

public:
	HttpHeaders& operator=(const HttpHeaders&); // to prevent copying;

	const NETLIBHTTPHEADER* operator[](size_t idx)
	{
		return m_request
			? &m_request->headers[idx]
			: NULL;
	}

	HttpHeaders& operator<<(const VALUE &param)
	{
		Add(param.szName);
		return *this;
	}

	HttpHeaders& operator<<(const CHAR_VALUE &param)
	{
		Add(param.szName, param.szValue);
		return *this;
	}

	HttpHeaders& operator<<(const ENCODED_VALUE &param)
	{
		Add(param.szName, param.szValue);
		return *this;
	}
};

class HttpContent
{
	friend class HttpRequest;
	friend class HttpResponse;

protected:
	CMStringA m_content;
	NETLIBHTTPREQUEST *m_request;

	HttpContent(NETLIBHTTPREQUEST *request)
		: m_request(request)
	{
	}

	~HttpContent()
	{
		if (m_request)
		{
			m_request->pData = NULL;
			m_request->dataLength = 0;
		}
	}

	void Add(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		if (!m_content.IsEmpty())
			m_content += '&';
		m_content.AppendFormatV(fmt, args);
		va_end(args);

		if (m_request)
		{
			m_request->pData = m_content.GetBuffer();
			m_request->dataLength = m_content.GetLength();
		}
	}

public:
	HttpContent& operator=(const HttpContent&); // to prevent copying;

	bool operator!() const
	{
		return !m_request || !m_request->pData || !m_request->dataLength;
	}

	operator const char*()
	{
		return m_request
			? m_request->pData
			: NULL;
	}

	virtual size_t GetSize() const
	{
		return m_request
			? m_request->dataLength
			: 0;
	}

	HttpContent & operator<<(const VALUE &param)
	{
		Add(param.szName);
		return *this;
	}

	HttpContent & operator<<(const INT_VALUE &param)
	{
		Add("%s=%i", param.szName, param.iValue);
		return *this;
	}

	HttpContent & operator<<(const LONG_VALUE &param)
	{
		Add("%s=%lld", param.szName, param.llValue);
		return *this;
	}

	HttpContent & operator<<(const CHAR_VALUE &param)
	{
		Add("%s=%s", param.szName, param.szValue);
		return *this;
	}

	HttpContent &operator<<(const ENCODED_VALUE &param)
	{
		Add("%s=%s", param.szName, param.szValue);
		return *this;
	}
};

enum HttpMethod
{
	HttpGet = 1,
	HttpPost
};

class HttpResponse
{
	friend class HttpRequest;

private:
	NETLIBHTTPREQUEST *m_response;

public:
	HttpRequest *Request;
	HttpHeaders Headers;
	HttpContent Content;

	HttpResponse(HttpRequest *request, NETLIBHTTPREQUEST *response)
		: Request(request), m_response(response),
		Headers(response), Content(response)
	{
	}

	~HttpResponse()
	{
		Netlib_FreeHttpRequest(m_response);
	}

	bool IsSuccess() const
	{
		return m_response->resultCode >= HTTP_CODE_OK &&
			m_response->resultCode <= HTTP_CODE_MULTI_STATUS;
	}

	int GetStatusCode() const
	{
		return m_response->resultCode;
	}
};

class HttpRequest : protected NETLIBHTTPREQUEST, public MZeroedObject
{
	friend class HttpUri;
	friend class HttpHeaders;
	friend class HttpContent;
	friend class FormContent;

public:
	HttpUri Uri;
	HttpHeaders Headers;
	HttpContent Content;

	HttpRequest(HttpMethod method, const char *url)
		: Uri(this, url), Headers(this), Content(this)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		requestType = method;
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPHEADERS;
	}

	~HttpRequest()
	{
	}

	operator NETLIBHTTPREQUEST*()
	{
		return this;
	}
};

#endif //_HTTP_REQUEST_H_