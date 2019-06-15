/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include "../../../miranda-private-keys/Facebook/app_secret.h"

class FacebookProto;

struct AsyncHttpRequest : public MTHttpRequest<FacebookProto>
{
	void CalcSig();
};

class FacebookProto : public PROTO<FacebookProto>
{
	AsyncHttpRequest* CreateGraphql(const char *szName, const char *szMethod);

	CMStringA szDeviceID;

public:
	FacebookProto(const char *proto_name, const wchar_t *username);
	~FacebookProto();

	////////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	void OnModulesLoaded() override;
};

struct CMPlugin : public ACCPROTOPLUGIN<FacebookProto>
{
	CMPlugin();

	int Load() override;
};
