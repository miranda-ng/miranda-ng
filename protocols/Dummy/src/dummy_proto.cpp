/*
Copyright (c) 2014-17 Robert Pösel, 2017-24 Miranda NG team

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
	{ LPGEN("Custom"),    "",         ""                        },
	{ "AIM",              "SN",       LPGEN("Screen name")      },
	{ "EmLAN",            "Nick",     LPGEN("User name")        },
	{ "Facebook",         "ID",       LPGEN("Facebook ID")      },
	{ "GG",               "UIN",      LPGEN("Gadu-Gadu number") },
	{ "ICQ",              "aimId",    LPGEN("User ID")          },
	{ "ICQCorp",          "UIN",      LPGEN("ICQ number")       },
	{ "IRC",              "Nick",     LPGEN("Nickname")         },
	{ "Jabber",           "jid",      LPGEN("JID")              },
	{ "MinecraftDynmap",  "Nick",     LPGEN("Visible name")     },
	{ "MRA",              "e-mail",   LPGEN("E-mail address")   },
	{ "MSN",              "wlid",     LPGEN("Live ID")          },
	{ "Omegle",           "nick",     LPGEN("Visible name")     },
	{ "Sametime",         "stid",     LPGEN("ID")               },
	{ "Skype (SkypeKit)", "sid",      LPGEN("Skype name")       },
	{ "Skype (Classic)",  "Username", LPGEN("Skype name")       },
	{ "Skype (Web)",      "Username", LPGEN("Skype name")       },
	{ "Steam",            "SteamID",  LPGEN("Steam ID")         },
	{ "Telegram",         "id",       LPGEN("Telegram ID")      },
	{ "Tlen",             "jid",      LPGEN("Tlen login")       },
	{ "Tox",              "ToxID",    LPGEN("Tox ID")           },
	{ "Twitter",          "Username", LPGEN("Username")         },
	{ "VK",               "ID",       LPGEN("VKontakte ID")     },
	{ "WhatsApp",         "ID",       LPGEN("WhatsApp ID")      },
	{ "XFire",            "Username", LPGEN("Username")         },
	{ "Yahoo",            "yahoo_id", LPGEN("ID")               },
};

void FillTemplateCombo(HWND hwndDlg, int iCtrlId)
{
	for (auto &it : templates) {
		int i = &it - templates;
		SendDlgItemMessageA(hwndDlg, iCtrlId, CB_INSERTSTRING, i, LPARAM(i == 0 ? Translate(it.name) : it.name));
	}
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

/////////////////////////////////////////////////////////////////////////////////////////

static int sttCompareProtocols(const CDummyProto *p1, const CDummyProto *p2)
{
	return mir_wstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

CDummyProto::CDummyProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CDummyProto>(szModuleName, ptszUserName)
{
	msgid = 0;

	int id = getDummyProtocolId(m_szModuleName);
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

int getDummyProtocolId(const char *pszModuleName)
{
	int id = db_get_b(0, pszModuleName, DUMMY_ID_TEMPLATE, -1);
	if (id >= 0 && id < _countof(templates))
		return id;
	
	CMStringA szProto(db_get_sm(0, pszModuleName, "AM_BaseProto"));
	for (auto &it : templates)
		if (!stricmp(it.name, szProto))
			return int(&it - templates);

	return -1;
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
			int id = getDummyProtocolId(m_szModuleName);
			ptrW setting(id > 0 ? mir_a2u(Translate(templates[id].text)) : getWStringA(DUMMY_ID_TEXT));
			if (setting != NULL)
				wcsncpy_s(uniqueIdSetting, setting, _TRUNCATE);
		}
		return (INT_PTR)uniqueIdSetting;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CDummyProto::SendMsg(MCONTACT hContact, MEVENT, const char *msg)
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
