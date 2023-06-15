#ifndef _STEAM_REQUEST_SESSION_H_
#define _STEAM_REQUEST_SESSION_H_

struct GetHostsRequest : public HttpRequest
{
	GetHostsRequest() :
		HttpRequest(REQUEST_GET, "/ISteamDirectory/GetCMList/v0001?cellid=0")
	{}
};

#endif //_STEAM_REQUEST_SESSION_H_
