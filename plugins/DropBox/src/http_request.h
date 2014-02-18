#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include "common.h"

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

	void AddParameter(LPCSTR szName, LPCSTR szValue)
	{
		if(m_szUrl.Find('?') == -1)
			m_szUrl.AppendFormat("?%s=%s", szName, szValue);
		else
			m_szUrl.AppendFormat("&%s=%s", szName, szValue);
	}

	void AddParameter(LPCSTR szName, int value)
	{
		if(m_szUrl.Find('?') == -1)
			m_szUrl.AppendFormat("?%s=%i", szName, value);
		else
			m_szUrl.AppendFormat("&%s=%i", szName, value);
	}

	NETLIBHTTPREQUEST *Send()
	{
		szUrl = m_szUrl.GetBuffer();
		return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)this);
	}

	//void SendAsync(typename CBaseProto<T>::AsyncHttpRequest callback)
	//{
	//	szUrl = m_szUrl.GetBuffer();
	//	AsyncParam param = { this, proto, callback };
	//	/*HANDLE hThread = */mir_forkthread(SendAsync, &param);
	//	//WaitForSingleObject(hThread, INFINITE);
	//}

private:

	CMStringA m_szUrl;
	HANDLE m_hNetlibUser;

	/*static void SendAsync(void *arg)
	{
		AsyncParam *param = (AsyncParam*)arg;
		NETLIBHTTPREQUEST* response = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)param->m_proto->m_hNetlibUser, (LPARAM)param->m_request);
		
		CBaseProto<T> *proto = param->m_proto;
		AsyncRequestCallback callback = param->m_callback;
		proto->*callback(response);

		delete response;
	}*/
};

#endif //_HTTP_REQUEST_H_