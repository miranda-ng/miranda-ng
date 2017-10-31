/*
Copyright (c) 2015-17 Miranda NG project (https://miranda-ng.org)

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

void SkypeHttpResponse(const NETLIBHTTPREQUEST *response, void *arg)
{
	SkypeResponseDelegateBase *delegate = (SkypeResponseDelegateBase*)arg;
	delegate->Invoke(response);
	delete delegate;
}

void CSkypeProto::PushRequest(HttpRequest *request)
{
	requestQueue->Push(request);
}

void CSkypeProto::PushRequest(HttpRequest *request, SkypeResponseCallback response)
{
	SkypeResponseDelegateBase *delegate = new SkypeResponseDelegate(this, response);
	requestQueue->Push(request, SkypeHttpResponse, delegate);
}

void CSkypeProto::PushRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg)
{
	SkypeResponseDelegateBase *delegate = new SkypeResponseDelegateWithArg(this, response, arg);
	requestQueue->Push(request, SkypeHttpResponse, delegate);
}

void CSkypeProto::SendRequest(HttpRequest *request)
{
	requestQueue->Send(request, NULL, NULL);
}

void CSkypeProto::SendRequest(HttpRequest *request, SkypeResponseCallback response)
{
	SkypeResponseDelegateBase *delegate = new SkypeResponseDelegate(this, response);
	requestQueue->Send(request, SkypeHttpResponse, delegate);
}

void CSkypeProto::SendRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg)
{
	SkypeResponseDelegateBase *delegate = new SkypeResponseDelegateWithArg(this, response, arg);
	requestQueue->Send(request, SkypeHttpResponse, delegate);
}
