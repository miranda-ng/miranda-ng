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

class HttpRequest : public MHttpRequest
{
	void Init(int type)
	{
		requestType = type;
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
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
	}

	void AddUrlParameter(const char *urlFormat, ...)
	{
		va_list urlArgs;
		va_start(urlArgs, urlFormat);
		m_szUrl += m_szUrl.Find('?') == -1 ? '?' : '&';
		m_szUrl.AppendFormatV(urlFormat, urlArgs);
		va_end(urlArgs);
	}

	MHttpResponse* Send(HNETLIBUSER hNetlibConnection)
	{
		m_szUrl.Replace('\\', '/');
		return Netlib_HttpTransaction(hNetlibConnection, this);
	}
};

#endif //_HTTP_REQUEST_H_