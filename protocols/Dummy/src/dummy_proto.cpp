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

void CDummyProto::SendMsgAck(void *p)
{
	if (p == NULL)
		return;

	message_data *data = static_cast<message_data*>(p);

	Sleep(100);

	if (getByte(DUMMY_KEY_ALLOW_SENDING, 0))
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)data->msgid, 0);
	else
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid, (LPARAM)Translate("This Dummy account has disabled sending messages. Enable it in account options."));

	delete data;
}

void CDummyProto::SearchIdAckThread(void *targ)
{
	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.id.w = (wchar_t*)targ;
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&psr);
	
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);

	mir_free(targ);
}

static int sttCompareProtocols(const CDummyProto *p1, const CDummyProto *p2)
{
	return mir_wstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CDummyProto> dummy_Instances(1, sttCompareProtocols);

CDummyProto::CDummyProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CDummyProto>(szModuleName, ptszUserName)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CDummyProto::SvcCreateAccMgrUI);

	msgid = 0;

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

int CDummyProto::getTemplateId()
{
	int id = this->getByte(DUMMY_ID_TEMPLATE, 0);
	if (id < 0 || id >= _countof(templates)) {
		return 0;
	}
	return id;
}

DWORD_PTR CDummyProto::GetCaps(int type, MCONTACT)
{
	switch(type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_ADDSEARCHRES;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

	case PFLAGNUM_3:
		return 0;

	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOAUTHDENYREASON | PF4_NOCUSTOMAUTH;

	case PFLAGNUM_5:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

	case PFLAG_MAXLENOFMESSAGE:
		return 0;

	case PFLAG_UNIQUEIDTEXT:
		if (uniqueIdSetting[0] == '\0') {
			int id = getTemplateId();
			ptrA setting(id > 0 ? mir_strdup(Translate(templates[id].text)) : getStringA(DUMMY_ID_TEXT));
			if (setting != NULL)
				strncpy_s(uniqueIdSetting, setting, _TRUNCATE);
		}
		return (DWORD_PTR)uniqueIdSetting;

	case PFLAG_UNIQUEIDSETTING:
		if (uniqueIdText[0] == '\0') {
			int id = getTemplateId();
			ptrA setting(id > 0 ? mir_strdup(templates[id].setting) : getStringA(DUMMY_ID_SETTING));
			if (setting != NULL)
				strncpy_s(uniqueIdText, setting, _TRUNCATE);
		}
		return (DWORD_PTR)uniqueIdText;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::SendMsg(MCONTACT hContact, int, const char *msg)
{
	std::string message = msg;
	unsigned int id = InterlockedIncrement(&this->msgid);

	ForkThread(&CDummyProto::SendMsgAck, new message_data(hContact, message, id));
	return id;
}

int CDummyProto::SetStatus(int)
{
	return 0;
}

HANDLE CDummyProto::SearchBasic(const wchar_t* id)
{
	if (uniqueIdSetting[0] == '\0')
		return 0;

	wchar_t *tid = mir_wstrdup(id);
	ForkThread(&CDummyProto::SearchIdAckThread, tid);
	return tid;
}

MCONTACT CDummyProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	if (psr->id.w == NULL)
		return NULL;

	MCONTACT hContact = db_add_contact();
	if (hContact && Proto_AddToContact(hContact, m_szModuleName) != 0) {
		db_delete_contact(hContact);
		hContact = NULL;
	}

	if (hContact) {
		if (flags & PALF_TEMPORARY) {
			db_set_b(hContact, "CList", "Hidden", 1);
			db_set_b(hContact, "CList", "NotOnList", 1);
		}
		else if (db_get_b(hContact, "CList", "NotOnList", 0)) {
			db_unset(hContact, "CList", "Hidden");
			db_unset(hContact, "CList", "NotOnList");
		}
		setWString(hContact, uniqueIdSetting, psr->id.w);
		setWString(hContact, "Nick", psr->id.w);
	}

	return hContact;
}
