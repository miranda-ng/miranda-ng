/*
Copyright (c) 2014-17 Robert Pösel, 2017-25 Miranda NG team

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

const ttemplate templates[] =
{
	{ LPGEN("Custom"),    "",         "",                        IDI_DUMMY     },
	{ "AIM",              "SN",       LPGEN("Screen name"),      IDI_AIM       },
	{ "Discord",          "id",       LPGEN("Discord ID"),       IDI_DISCORD   },
	{ "EmLAN",            "Nick",     LPGEN("User name"),        IDI_EMLAN     },
	{ "Facebook",         "ID",       LPGEN("Facebook ID"),      IDI_FACEBOOK  },
	{ "GG",               "UIN",      LPGEN("Gadu-Gadu number"), IDI_GG        },
	{ "ICQ",              "aimId",    LPGEN("User ID"),          IDI_ICQ       },
	{ "ICQCorp",          "UIN",      LPGEN("ICQ number"),       IDI_ICQ       },
	{ "IRC",              "Nick",     LPGEN("Nickname"),         IDI_IRC       },
	{ "Jabber",           "jid",      LPGEN("JID"),              IDI_JABBER    },
	{ "MinecraftDynmap",  "Nick",     LPGEN("Visible name"),     IDI_MINECRAFT },
	{ "MRA",              "e-mail",   LPGEN("E-mail address"),   IDI_MRA       },
	{ "MSN",              "wlid",     LPGEN("Live ID"),          IDI_MSN       },
	{ "Omegle",           "nick",     LPGEN("Visible name"),     IDI_OMEGLE    },
	{ "Sametime",         "stid",     LPGEN("ID"),               IDI_SAMETIME  },
	{ "Skype (SkypeKit)", "sid",      LPGEN("Skype name"),       IDI_SKYPE     },
	{ "Skype (Classic)",  "Username", LPGEN("Skype name"),       IDI_SKYPE     },
	{ "Skype (Web)",      "Username", LPGEN("Skype name"),       IDI_SKYPE     },
	{ "Steam",            "SteamID",  LPGEN("Steam ID"),         IDI_STEAM     },
	{ "Telegram",         "id",       LPGEN("Telegram ID"),      IDI_TELEGRAM  },
	{ "Tlen",             "jid",      LPGEN("Tlen login"),       IDI_TLEN      },
	{ "Tox",              "ToxID",    LPGEN("Tox ID"),           IDI_TOX       },
	{ "Twitter",          "Username", LPGEN("Username"),         IDI_TWITTER   },
	{ "VKontakte",        "ID",       LPGEN("VKontakte ID"),     IDI_VK        },
	{ "WhatsApp",         "ID",       LPGEN("WhatsApp ID"),      IDI_WHATSAPP  },
	{ "XFire",            "Username", LPGEN("Username"),         IDI_XFIRE     },
	{ "Yahoo",            "yahoo_id", LPGEN("ID"),               IDI_YAHOO     },
};

void FillTemplateCombo(HWND hwndDlg, int iCtrlId)
{
	for (auto &it : templates) {
		int i = &it - templates;
		SendDlgItemMessageA(hwndDlg, iCtrlId, CB_INSERTSTRING, i, LPARAM(i == 0 ? Translate(it.name) : it.name));
	}
}

void InitIcons()
{
	wchar_t wszDllName[MAX_PATH];
	GetModuleFileNameW(g_plugin.getInst(), wszDllName, _countof(wszDllName));

	SKINICONDESC sid = {};
	sid.flags = SIDF_PATH_UNICODE;
	sid.defaultFile.w = wszDllName;
	sid.section.a = "Protocols/Dummy";
	for (auto &it : templates) {
		sid.iDefaultIndex = -it.iconId;
		sid.pszName = (char *)it.name;
		sid.description.a = (char *)it.name;
		g_plugin.addIcon(&sid);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttCompareProtocols(const CDummyProto *p1, const CDummyProto *p2)
{
	return mir_wstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

CDummyProto::CDummyProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CDummyProto>(szModuleName, ptszUserName),
	bAllowSending(szModuleName, "AllowSending", false)
{
	msgid = 0;

	int id = getTemplateId();
	ptrA setting(id > 0 ? mir_strdup(templates[id].setting) : getStringA(DUMMY_ID_SETTING));
	if (setting != NULL) {
		strncpy_s(uniqueIdText, setting, _TRUNCATE);
		Proto_SetUniqueId(m_szModuleName, uniqueIdText);
	}
	else uniqueIdText[0] = '\0';

	uniqueIdSetting[0] = '\0';
	m_hProtoIcon = g_plugin.getIconHandle(templates[id].iconId);
}

CDummyProto::~CDummyProto()
{
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::getTemplateId()
{
	int id = getByte(DUMMY_ID_TEMPLATE, -1);
	if (id >= 0 && id < _countof(templates))
		return id;
	
	CMStringA szProto(getMStringA("AM_PrevProto"));
	if (szProto.IsEmpty())
		szProto = getMStringA("AM_BaseProto");

	for (auto &it : templates)
		if (szProto == it.name)
			return int(&it - templates);

	return 0;
}

void CDummyProto::selectTemplate(HWND hwndDlg, int templateId)
{
	// Enable custom fields when selected custom template
	EnableWindow(GetDlgItem(hwndDlg, IDC_ID_TEXT), templateId == 0);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ID_SETTING), templateId == 0);

	ptrA tszIdText(templateId > 0 ? mir_strdup(Translate(templates[templateId].text)) : getStringA(DUMMY_ID_TEXT));
	if (tszIdText != NULL)
		SetDlgItemTextA(hwndDlg, IDC_ID_TEXT, tszIdText);

	ptrA tszIdSetting(templateId > 0 ? mir_strdup(templates[templateId].setting) : getStringA(DUMMY_ID_SETTING));
	if (tszIdSetting != NULL)
		SetDlgItemTextA(hwndDlg, IDC_ID_SETTING, tszIdSetting);
}

INT_PTR CDummyProto::GetCaps(int type, MCONTACT)
{
	switch(type) {
	case PFLAGNUM_1:
		return PF1_BASICSEARCH | PF1_ADDSEARCHRES | (bAllowSending ? PF1_IM : 0);

	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOAUTHDENYREASON | PF4_NOCUSTOMAUTH;

	case PFLAG_UNIQUEIDTEXT:
		if (uniqueIdSetting[0] == '\0') {
			int id = getTemplateId();
			ptrW setting(id > 0 ? mir_a2u(Translate(templates[id].text)) : getWStringA(DUMMY_ID_TEXT));
			if (setting != NULL)
				wcsncpy_s(uniqueIdSetting, setting, _TRUNCATE);
		}
		return (INT_PTR)uniqueIdSetting;

	case 1000: // hidden caps
		return TRUE;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDummyProto::SearchIdAckThread(void *targ)
{
	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.id.w = (wchar_t *)targ;
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&psr);

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);

	mir_free(targ);
}

HANDLE CDummyProto::SearchBasic(const wchar_t *id)
{
	if (uniqueIdSetting[0] == '\0')
		return nullptr;

	wchar_t *tid = mir_wstrdup(id);
	ForkThread(&CDummyProto::SearchIdAckThread, tid);
	return tid;
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::SendMsg(MCONTACT hContact, MEVENT, const char *msg)
{
	std::string message = msg;
	unsigned int id = InterlockedIncrement(&msgid);

	if (bAllowSending)
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)id);
	else
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id, (LPARAM)TranslateT("This Dummy account has disabled sending messages. Enable it in account options."));
	return id;
}

int CDummyProto::SetStatus(int)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
