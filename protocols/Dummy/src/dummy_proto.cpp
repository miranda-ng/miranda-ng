/*
Copyright (c) 2014-17 Robert Pösel, 2017-22 Miranda NG team

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

const ttemplate templates[DUMMY_PROTO_COUNT] =
{
	{ LPGEN("Custom"), "", "" },
	{ "AIM", "SN", LPGEN("Screen name") },
	{ "Discord", "id", LPGEN("Discord ID") },
	{ "EmLAN", "Nick", LPGEN("User name") },
	{ "Facebook", "ID", LPGEN("Facebook ID") },
	{ "GG", "UIN", LPGEN("Gadu-Gadu number") },
	{ "ICQ", "UIN", LPGEN("User ID") },
	{ "ICQCorp", "UIN", LPGEN("ICQ number") },
	{ "IRC", "Nick", LPGEN("Nickname") },
	{ "Jabber", "jid", LPGEN("JID") },
	{ "MinecraftDynmap", "Nick", LPGEN("Visible name") },
	{ "MRA", "e-mail", LPGEN("E-mail address") },
	{ "MSN", "wlid", LPGEN("Live ID") },
	{ "Omegle", "nick", LPGEN("Visible name") },
	{ "Sametime", "stid", LPGEN("ID") },
	{ "Skype (SkypeKit)", "sid", LPGEN("Skype name") },
	{ "Skype (Classic)", "Username", LPGEN("Skype name") },
	{ "Skype (Web)", "Username", LPGEN("Skype name") },
	{ "Steam", "SteamID", LPGEN("Steam ID") },
	{ "Tlen", "jid", LPGEN("Tlen login") },
	{ "Tox", "ToxID", LPGEN("Tox ID") },
	{ "Twitter", "Username", LPGEN("Username") },
	{ "VK", "ID", LPGEN("VKontakte ID") },
	{ "WhatsApp", "ID", LPGEN("WhatsApp ID") },
	{ "XFire", "Username", LPGEN("Username") },
	{ "Yahoo", "yahoo_id", LPGEN("ID") },
};

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

CDummyProto::CDummyProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CDummyProto>(szModuleName, ptszUserName)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CDummyProto::SvcCreateAccMgrUI);

	msgid = 0;

	int id = getTemplateId();
	ptrA setting(id > 0 ? mir_strdup(templates[id].setting) : getStringA(DUMMY_ID_SETTING));
	if (setting != NULL) {
		strncpy_s(uniqueIdText, setting, _TRUNCATE);
		Proto_SetUniqueId(m_szModuleName, uniqueIdText);
	}
	else uniqueIdText[0] = '\0';

	uniqueIdSetting[0] = '\0';
}

CDummyProto::~CDummyProto()
{
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::getTemplateId()
{
	int id = this->getByte(DUMMY_ID_TEMPLATE, -1);
	if (id >= 0 && id < _countof(templates))
		return id;
	
	CMStringA szProto(getMStringA("AM_BaseProto"));
	for (auto &it : templates)
		if (!stricmp(it.name, szProto))
			return int(&it - templates);

	return 0;
}

INT_PTR CDummyProto::GetCaps(int type, MCONTACT)
{
	switch(type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_ADDSEARCHRES;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAGNUM_3:
		return 0;

	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOAUTHDENYREASON | PF4_NOCUSTOMAUTH;

	case PFLAGNUM_5:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAG_MAXLENOFMESSAGE:
		return 0;

	case PFLAG_UNIQUEIDTEXT:
		if (uniqueIdSetting[0] == '\0') {
			int id = getTemplateId();
			ptrW setting(id > 0 ? mir_a2u(Translate(templates[id].text)) : getWStringA(DUMMY_ID_TEXT));
			if (setting != NULL)
				wcsncpy_s(uniqueIdSetting, setting, _TRUNCATE);
		}
		return (INT_PTR)uniqueIdSetting;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::SendMsg(MCONTACT hContact, int, const char *msg)
{
	std::string message = msg;
	unsigned int id = InterlockedIncrement(&this->msgid);

	if (getByte(DUMMY_KEY_ALLOW_SENDING, 0))
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)id);
	else
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id, (LPARAM)TranslateT("This Dummy account has disabled sending messages. Enable it in account options."));
	return id;
}

int CDummyProto::SetStatus(int)
{
	return 0;
}

HANDLE CDummyProto::SearchBasic(const wchar_t* id)
{
	if (uniqueIdSetting[0] == '\0')
		return nullptr;

	wchar_t *tid = mir_wstrdup(id);
	ForkThread(&CDummyProto::SearchIdAckThread, tid);
	return tid;
}

MCONTACT CDummyProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	if (psr->id.w == nullptr)
		return NULL;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	if (flags & PALF_TEMPORARY) {
		Contact::Hide(hContact);
		Contact::RemoveFromList(hContact);
	}
	else if (!Contact::OnList(hContact)) {
		Contact::Hide(hContact, false);
		Contact::PutOnList(hContact);
	}
	setWString(hContact, _T2A(uniqueIdSetting), psr->id.w);
	setWString(hContact, "Nick", psr->id.w);

	return hContact;
}
