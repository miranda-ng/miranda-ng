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

char uniqueIdText[100] = { 0 };
char uniqueIdSetting[100] = { 0 };

void CDummyProto::SendMsgAck(void *param)
{
	MCONTACT hContact = (MCONTACT)param;
	Sleep(100);
	ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)0, (LPARAM)Translate("Dummy protocol is too dumb to send messages."));
}

static int sttCompareProtocols(const CDummyProto *p1, const CDummyProto *p2)
{
	return lstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CDummyProto> dummy_Instances(1, sttCompareProtocols);

CDummyProto::CDummyProto(const char *szModuleName, const TCHAR *ptszUserName) :
	PROTO<CDummyProto>(szModuleName, ptszUserName)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CDummyProto::SvcCreateAccMgrUI);

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
				mir_snprintf(uniqueIdSetting, SIZEOF(uniqueIdSetting), "%s", setting);
		}
		return (DWORD_PTR)uniqueIdSetting;

	case PFLAG_UNIQUEIDSETTING:
		if (uniqueIdText[0] == '\0') {
			ptrA setting(getStringA(DUMMY_ID_SETTING));
			if (setting != NULL)
				mir_snprintf(uniqueIdText, SIZEOF(uniqueIdText), "%s", setting);
		}
		return (DWORD_PTR)uniqueIdText;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	return 1;
}

int CDummyProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	return 0;
}

int CDummyProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	ForkThread(&CDummyProto::SendMsgAck, (void*)hContact);
	return 0;
}

int CDummyProto::SetStatus(int iNewStatus)
{
	return 0;
}

HANDLE CDummyProto::SearchBasic(const PROTOCHAR* id)
{
	return 0;
}

HANDLE CDummyProto::SearchByEmail(const PROTOCHAR* email)
{
	return 0;
}

HANDLE CDummyProto::SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName)
{
	return 0;
}

MCONTACT CDummyProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return NULL;
}

int CDummyProto::AuthRequest(MCONTACT hContact,const PROTOCHAR *message)
{
	return 0;
}

int CDummyProto::Authorize(HANDLE hDbEvent)
{
	return 1;
}

int CDummyProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR *reason)
{
	return 1;
}

int CDummyProto::UserIsTyping(MCONTACT hContact, int type)
{ 
	return 1;
}

MCONTACT CDummyProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
{
	return NULL;
}

int CDummyProto::AuthRecv(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

HANDLE CDummyProto::FileAllow(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *path)
{
	return NULL;
}

int CDummyProto::FileCancel(MCONTACT hContact,HANDLE hTransfer)
{
	return 1;
}

int CDummyProto::FileDeny(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *reason)
{
	return 1;
}

int CDummyProto::FileResume(HANDLE hTransfer,int *action,const PROTOCHAR **filename)
{
	return 1;
}

int CDummyProto::GetInfo(MCONTACT hContact, int infoType)
{
	return 1;
}

HWND CDummyProto::SearchAdvanced(HWND owner)
{
	return NULL;
}

HWND CDummyProto::CreateExtendedSearchUI(HWND owner)
{
	return NULL;
}

int CDummyProto::RecvContacts(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int CDummyProto::RecvFile(MCONTACT hContact,PROTORECVFILET *)
{
	return 1;
}

int CDummyProto::RecvUrl(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int CDummyProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	return 1;
}

HANDLE CDummyProto::SendFile(MCONTACT hContact,const PROTOCHAR *desc, PROTOCHAR **files)
{
	return NULL;
}

int CDummyProto::SendUrl(MCONTACT hContact,int flags,const char *url)
{
	return 1;
}

int CDummyProto::SetApparentMode(MCONTACT hContact,int mode)
{
	return 1;
}

int CDummyProto::RecvAwayMsg(MCONTACT hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}

HANDLE CDummyProto::GetAwayMsg(MCONTACT hContact)
{
	return 0;
}

int CDummyProto::SetAwayMsg(int status, const PROTOCHAR *msg)
{
	return 0;
}
