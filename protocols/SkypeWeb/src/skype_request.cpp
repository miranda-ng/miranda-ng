/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

class SkypeResponseDelegate
{
private:
	CSkypeProto *proto;
	SkypeResponseCallback responseCallback;
	SkypeResponseWithArgCallback responseWithArgCallback;

	void *arg;
	bool hasArg;

public:
	SkypeResponseDelegate(CSkypeProto *proto, SkypeResponseCallback responseCallback)
		: proto(proto), responseCallback(responseCallback), responseWithArgCallback(NULL), arg(NULL), hasArg(false) {}

	SkypeResponseDelegate(CSkypeProto *proto, SkypeResponseWithArgCallback responseCallback, void *arg)
		: proto(proto), responseCallback(NULL), responseWithArgCallback(responseCallback), arg(arg), hasArg(true) { }

	void Invoke(const NETLIBHTTPREQUEST *response)
	{
		if (hasArg)
			(proto->*(responseWithArgCallback))(response, arg);
		else
			(proto->*(responseCallback))(response);
	}
};

static void SkypeHttpResponse(const NETLIBHTTPREQUEST *response, void *arg)
{
	((SkypeResponseDelegate*)arg)->Invoke(response);
}

void CSkypeProto::PushRequest(HttpRequest *request)
{
	requestQueue->Push(request);
}

void CSkypeProto::PushRequest(HttpRequest *request, SkypeResponseCallback response)
{
	SkypeResponseDelegate *delegate = new SkypeResponseDelegate(this, response);
	requestQueue->Push(request, SkypeHttpResponse, delegate);
}

void CSkypeProto::PushRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg)
{
	SkypeResponseDelegate *delegate = new SkypeResponseDelegate(this, response, arg);
	requestQueue->Push(request, SkypeHttpResponse, delegate);
}

void CSkypeProto::SendRequest(HttpRequest *request)
{
	requestQueue->Send(request, NULL, NULL);
}

void CSkypeProto::SendRequest(HttpRequest *request, SkypeResponseCallback response)
{
	SkypeResponseDelegate *delegate = new SkypeResponseDelegate(this, response);
	requestQueue->Send(request, SkypeHttpResponse, delegate);
}

void CSkypeProto::SendRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg)
{
	SkypeResponseDelegate *delegate = new SkypeResponseDelegate(this, response, arg);
	requestQueue->Send(request, SkypeHttpResponse, delegate);
}