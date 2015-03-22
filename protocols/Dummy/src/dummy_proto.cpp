/*
Copyright (c) 2014 Robert Pösel

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

void CDummyProto::SendMsgAck(void *param)
{
	MCONTACT hContact = (MCONTACT)param;
	Sleep(100);
	ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)0, (LPARAM)Translate("Dummy protocol is too dumb to send messages."));
}

static int sttCompareProtocols(const CDummyProto *p1, const CDummyProto *p2)
{
	return mir_tstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CDummyProto> dummy_Instances(1, sttCompareProtocols);

CDummyProto::CDummyProto(const char *szModuleName, const TCHAR *ptszUserName) :
	PROTO<CDummyProto>(szModuleName, ptszUserName)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CDummyProto::SvcCreateAccMgrUI);

	uniqueIdText[0] = '\0';
	uniqueIdSetting[0] = '\0';

	dummy_Instances.insert(this);
}

CDummyProto::~CDummyProto()
{
	Netlib_CloseHandle(m_hNetlibUser); m_hNetlibUser = NULL;
	dummy_Instances.remove(this);
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR CDummyProto::GetCaps(int type, MCONTACT hContact)
{
	switch(type) {
	case PFLAGNUM_1:
		return PF1_IM;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

	case PFLAGNUM_3:
		return 0;

	case PFLAGNUM_4:
		return 0;

	case PFLAGNUM_5:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

	case PFLAG_MAXLENOFMESSAGE:
		return 0;

	case PFLAG_UNIQUEIDTEXT:
		if (uniqueIdSetting[0] == '\0') {
			ptrA setting(getStringA(DUMMY_ID_TEXT));
			if (setting != NULL)
				strncpy_s(uniqueIdSetting, setting, _TRUNCATE);
		}
		return (DWORD_PTR)uniqueIdSetting;

	case PFLAG_UNIQUEIDSETTING:
		if (uniqueIdText[0] == '\0') {
			ptrA setting(getStringA(DUMMY_ID_SETTING));
			if (setting != NULL)
				strncpy_s(uniqueIdText, setting, _TRUNCATE);
		}
		return (DWORD_PTR)uniqueIdText;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	ForkThread(&CDummyProto::SendMsgAck, (void*)hContact);
	return 0;
}

int CDummyProto::SetStatus(int iNewStatus)
{
	return 0;
}
