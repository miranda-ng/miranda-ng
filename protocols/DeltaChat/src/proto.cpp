/*
Copyright © 2025 Miranda NG team

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

#include "stdafx.h"

CDeltaChatProto::CDeltaChatProto(const char *szModuleName, const wchar_t *wszUserName) :
	PROTO<CDeltaChatProto>(szModuleName, wszUserName)
{
}

CDeltaChatProto::~CDeltaChatProto()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDeltaChatProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_AUTHREQ | PF1_SERVERCLIST | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH | PF4_SUPPORTIDLE | PF4_SUPPORTTYPING | PF4_SERVERMSGID;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT(DB_KEY_ID);
	}

	return 0;
}
