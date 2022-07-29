/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include "jabber_caps.h"

static CIconPool g_MoodIcons, g_ActivityIcons;

///////////////////////////////////////////////////////////////////////////////
// Simple dialog with timer and ok/cancel buttons

class CJabberDlgPepBase : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	int m_time = 5;
	CTimer timer;

protected:
	CPepService *m_pepService;

	CCtrlButton btnOk;

public:
	CJabberDlgPepBase(CJabberProto *proto, int id) :
		CJabberDlgBase(proto, id),
		timer(this, 1),
		btnOk(this, IDOK)
	{
		timer.OnEvent = Callback(this, &CJabberDlgPepBase::OnTimer);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		timer.Start(1000);

		wchar_t buf[128];
		mir_snwprintf(buf, TranslateT("OK (%d)"), m_time);
		btnOk.SetText(buf);
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDOK:
		case IDCANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}

		return CSuper::Resizer(urc);
	}

	void OnTimer(CTimer *)
	{
		wchar_t buf[128];
		mir_snwprintf(buf, TranslateT("OK (%d)"), --m_time);
		btnOk.SetText(buf);

		if (m_time < 0) {
			timer.Stop();
			UIEmulateBtnClick(m_hwnd, IDOK);
		}
	}

	void StopTimer()
	{
		timer.Stop();
		btnOk.SetText(TranslateT("OK"));
	}
};

///////////////////////////////////////////////////////////////////////////////
// Simple PEP status

class CJabberDlgPepSimple : public CJabberDlgPepBase
{
	typedef CJabberDlgPepBase CSuper;

	struct CStatusMode
	{
		LPARAM m_id;
		char *m_name;
		HICON m_hIcon;
		wchar_t *m_title;
		bool m_subitem;

		CStatusMode(LPARAM id, char *name, HICON hIcon, wchar_t *title, bool subitem) : m_id(id), m_name(name), m_hIcon(hIcon), m_title(title), m_subitem(subitem) {}
		~CStatusMode() { IcoLib_ReleaseIcon(m_hIcon); }
	};

	OBJLIST<CStatusMode> m_modes;
	wchar_t *m_text;
	wchar_t *m_title;
	int m_time;
	int m_prevSelected;
	int m_selected;

	LPARAM m_active;
	wchar_t *m_activeText;

	void global_OnChange(CCtrlData *)
	{
		StopTimer();
	}

	void cbModes_OnChange(CCtrlData *)
	{
		StopTimer();

		if (m_prevSelected == m_cbModes.GetCurSel())
			return;

		char szSetting[128];

		CStatusMode *pMode = (CStatusMode *)m_cbModes.GetItemData(m_prevSelected);
		if (m_prevSelected >= 0 && pMode->m_id >= 0) {
			wchar_t *txt = m_txtDescription.GetText();
			mir_snprintf(szSetting, "PepMsg_%s", pMode->m_name);
			m_proto->setWString(szSetting, txt);
			mir_free(txt);
		}

		m_prevSelected = m_cbModes.GetCurSel();
		pMode = (CStatusMode *)m_cbModes.GetItemData(m_prevSelected);
		if (m_prevSelected >= 0 && pMode->m_id >= 0) {
			mir_snprintf(szSetting, "PepMsg_%s", pMode->m_name);

			ptrW szDescr(m_proto->getWStringA(szSetting));
			m_txtDescription.SetText((szDescr != nullptr) ? szDescr : L"");
			m_txtDescription.Enable(true);
		}
		else {
			m_txtDescription.SetTextA("");
			m_txtDescription.Enable(false);
		}
	}

	CCtrlCombo m_cbModes;
	CCtrlEdit m_txtDescription;

	UI_MESSAGE_MAP(CJabberDlgPepSimple, CSuper);
		UI_MESSAGE(WM_MEASUREITEM, OnWmMeasureItem);
		UI_MESSAGE(WM_DRAWITEM, OnWmDrawItem);
	UI_MESSAGE_MAP_END();

public:
	CJabberDlgPepSimple(CJabberProto *proto, wchar_t *title) :
		CJabberDlgPepBase(proto, IDD_PEP_SIMPLE),
		m_cbModes(this, IDC_CB_MODES),
		m_txtDescription(this, IDC_TXT_DESCRIPTION),
		m_modes(10),
		m_text(nullptr),
		m_selected(0),
		m_prevSelected(-1),
		m_active(-1),
		m_title(title)
	{
		SetMinSize(200, 200);

		m_cbModes.OnChange = Callback(this, &CJabberDlgPepSimple::cbModes_OnChange);
		m_cbModes.OnDropdown =
			m_txtDescription.OnChange = Callback(this, &CJabberDlgPepSimple::global_OnChange);

		m_modes.insert(new CStatusMode(-1, "<none>", Skin_LoadIcon(SKINICON_OTHER_SMALLDOT), TranslateT("None"), false));
	}

	~CJabberDlgPepSimple()
	{
		mir_free(m_text);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, m_proto->m_hProtoIcon);
		SetWindowText(m_hwnd, m_title);

		m_txtDescription.Enable(false);
		for (auto &it : m_modes) {
			int idx = m_cbModes.AddString(it->m_title, (LPARAM)it);
			if (it->m_id == m_active || !idx) {
				m_prevSelected = idx;
				m_cbModes.SetCurSel(idx);
				if (idx)
					m_txtDescription.Enable();
			}
		}

		if (m_activeText)
			m_txtDescription.SetText(m_activeText);
		return true;
	}

	bool OnApply() override
	{
		m_text = m_txtDescription.GetText();
		m_selected = m_cbModes.GetCurSel();
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_CB_MODES:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
		case IDC_TXT_DESCRIPTION:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		}

		return CSuper::Resizer(urc);
	}

	void AddStatusMode(LPARAM id, char *name, HICON hIcon, wchar_t *title, bool subitem = false)
	{
		m_modes.insert(new CStatusMode(id, name, hIcon, title, subitem));
	}

	void SetActiveStatus(LPARAM id, wchar_t *text)
	{
		m_active = id;
		m_activeText = text;
	}

	LPARAM GetStatusMode()
	{
		return m_modes[m_selected].m_id;
	}

	wchar_t *GetStatusText()
	{
		return m_text;
	}

	BOOL OnWmMeasureItem(UINT, WPARAM, LPARAM lParam)
	{
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
		if (lpmis->CtlID != IDC_CB_MODES)
			return FALSE;

		HDC hdc = GetDC(m_cbModes.GetHwnd());
		TEXTMETRIC tm = {};
		GetTextMetrics(hdc, &tm);
		ReleaseDC(m_cbModes.GetHwnd(), hdc);

		lpmis->itemHeight = max(tm.tmHeight, 18);
		if (lpmis->itemHeight < 18)
			lpmis->itemHeight = 18;
		return TRUE;
	}

	BOOL OnWmDrawItem(UINT, WPARAM, LPARAM lParam)
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->CtlID != IDC_CB_MODES)
			return FALSE;

		if (lpdis->itemData == -1)
			return FALSE;

		CStatusMode *mode = (CStatusMode *)lpdis->itemData;

		TEXTMETRIC tm = { 0 };
		GetTextMetrics(lpdis->hDC, &tm);

		SetBkMode(lpdis->hDC, TRANSPARENT);
		if (lpdis->itemState & ODS_SELECTED) {
			SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
		}
		else {
			SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_WINDOW));
		}

		if (!mode->m_subitem || (lpdis->itemState & ODS_COMBOBOXEDIT)) {
			wchar_t text[128];
			if (mode->m_subitem) {
				for (int i = mode->m_id; i >= 0; --i)
					if (!m_modes[i].m_subitem) {
						mir_snwprintf(text, L"%s [%s]", m_modes[i].m_title, mode->m_title);
						break;
					}
			}
			else mir_wstrncpy(text, mode->m_title, _countof(text));

			DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 2, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2, mode->m_hIcon, 16, 16, 0, nullptr, DI_NORMAL);
			TextOut(lpdis->hDC, lpdis->rcItem.left + 23, (lpdis->rcItem.top + lpdis->rcItem.bottom - tm.tmHeight) / 2, text, (int)mir_wstrlen(text));
		}
		else {
			wchar_t text[128];
			mir_snwprintf(text, L"...%s", mode->m_title);
			DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 23, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2, mode->m_hIcon, 16, 16, 0, nullptr, DI_NORMAL);
			TextOut(lpdis->hDC, lpdis->rcItem.left + 44, (lpdis->rcItem.top + lpdis->rcItem.bottom - tm.tmHeight) / 2, text, (int)mir_wstrlen(text));
		}

		return TRUE;
	}
};

///////////////////////////////////////////////////////////////////////////////
// CPepService base class

CPepService::CPepService(CJabberProto *proto, char *name, char *node) :
	m_proto(proto),
	m_name(name),
	m_node(node),
	m_hMenuItem(nullptr),
	m_wasPublished(false)
{
}

CPepService::~CPepService()
{
}

void CPepService::Publish()
{
	XmlNodeIq iq("set", m_proto->SerialNext());
	CreateData(
		iq << XCHILDNS("pubsub", JABBER_FEAT_PUBSUB)
		<< XCHILD("publish") << XATTR("node", m_node)
		<< XCHILD("item") << XATTR("id", "current"));
	m_proto->m_ThreadInfo->send(iq);

	m_wasPublished = true;
}

void CPepService::Retract()
{
	char *tempName = NEWSTR_ALLOCA(m_name);
	strlwr(tempName);

	m_proto->m_ThreadInfo->send(
		XmlNodeIq("set", m_proto->SerialNext())
		<< XCHILDNS("pubsub", JABBER_FEAT_PUBSUB)
		<< XCHILD("publish") << XATTR("node", m_node)
		<< XCHILD("item")
		<< XCHILDNS(tempName, m_node));
}

void CPepService::ResetPublish()
{
	m_wasPublished = false;
}

void CPepService::ForceRepublishOnLogin()
{
	if (!m_wasPublished)
		Publish();
}

///////////////////////////////////////////////////////////////////////////////
// CPepGuiService base class

CPepGuiService::CPepGuiService(CJabberProto *proto, char *name, char *node) :
	CPepService(proto, name, node),
	m_bGuiOpen(false),
	m_hIcolibItem(nullptr),
	m_szText(nullptr),
	m_hMenuService(nullptr)
{
}

CPepGuiService::~CPepGuiService()
{
	if (m_hMenuService) {
		DestroyServiceFunction(m_hMenuService);
		m_hMenuService = nullptr;
	}

	if (m_szText) mir_free(m_szText);
}

void CPepGuiService::InitGui()
{
	char szService[128];
	mir_snprintf(szService, "%s/AdvStatusSet/%s", m_proto->m_szModuleName, m_name);

	int(__cdecl CPepGuiService::*serviceProc)(WPARAM, LPARAM);
	serviceProc = &CPepGuiService::OnMenuItemClick;
	m_hMenuService = CreateServiceFunctionObj(szService, (MIRANDASERVICEOBJ)*(void **)&serviceProc, this);

	RebuildMenu();
}

void CPepGuiService::RebuildMenu()
{
	HGENMENU hJabberRoot = m_proto->m_hMenuRoot;
	if (hJabberRoot == nullptr)
		return;

	char szService[128];
	mir_snprintf(szService, "/AdvStatusSet/%s", m_name);

	CMenuItem mi(&g_plugin);
	mi.root = hJabberRoot;
	mi.pszService = szService;
	mi.position = 200010;
	mi.flags = CMIF_UNMOVABLE | CMIF_UNICODE | CMIF_HIDDEN;
	mi.hIcolibItem = m_hIcolibItem;
	mi.name.w = m_szText ? m_szText : LPGENW("<advanced status slot>");
	m_hMenuItem = Menu_AddProtoMenuItem(&mi, m_proto->m_szModuleName);
}

bool CPepGuiService::LaunchSetGui()
{
	if (m_bGuiOpen) return false;

	m_bGuiOpen = true;
	ShowSetDialog();
	m_bGuiOpen = false;

	return true;
}

void CPepGuiService::UpdateMenuItem(HANDLE hIcolibIcon, wchar_t *text)
{
	m_hIcolibItem = hIcolibIcon;
	replaceStrW(m_szText, text);

	if (m_hMenuItem)
		Menu_ModifyItem(m_hMenuItem, m_szText ? m_szText : TranslateT("<advanced status slot>"), m_hIcolibItem, CMIF_KEEPUNTRANSLATED);
}

int CPepGuiService::OnMenuItemClick(WPARAM, LPARAM)
{
	LaunchSetGui();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CPepMood

struct
{
	wchar_t *szName;
	char *szTag;
	int clistIconId;
}
static g_arrMoods[] =
{
	{ LPGENW("None"),         nullptr         },
	{ LPGENW("Afraid"),       "afraid"        },
	{ LPGENW("Amazed"),       "amazed"        },
	{ LPGENW("Amorous"),      "amorous"       },
	{ LPGENW("Angry"),        "angry"         },
	{ LPGENW("Annoyed"),      "annoyed"       },
	{ LPGENW("Anxious"),      "anxious"       },
	{ LPGENW("Aroused"),      "aroused"       },
	{ LPGENW("Ashamed"),      "ashamed"       },
	{ LPGENW("Bored"),        "bored"         },
	{ LPGENW("Brave"),        "brave"         },
	{ LPGENW("Calm"),         "calm"          },
	{ LPGENW("Cautious"),     "cautious"      },
	{ LPGENW("Cold"),         "cold"          },
	{ LPGENW("Confident"),    "confident"     },
	{ LPGENW("Confused"),     "confused"      },
	{ LPGENW("Contemplative"),"contemplative" },
	{ LPGENW("Contented"),    "contented"     },
	{ LPGENW("Cranky"),       "cranky"        },
	{ LPGENW("Crazy"),        "crazy"         },
	{ LPGENW("Creative"),     "creative"      },
	{ LPGENW("Curious"),      "curious"       },
	{ LPGENW("Dejected"),     "dejected"      },
	{ LPGENW("Depressed"),    "depressed"     },
	{ LPGENW("Disappointed"), "disappointed"  },
	{ LPGENW("Disgusted"),    "disgusted"     },
	{ LPGENW("Dismayed"),     "dismayed"      },
	{ LPGENW("Distracted"),   "distracted"    },
	{ LPGENW("Embarrassed"),  "embarrassed"   },
	{ LPGENW("Envious"),      "envious"       },
	{ LPGENW("Excited"),      "excited"       },
	{ LPGENW("Flirtatious"),  "flirtatious"   },
	{ LPGENW("Frustrated"),   "frustrated"    },
	{ LPGENW("Grateful"),     "grateful"      },
	{ LPGENW("Grieving"),     "grieving"      },
	{ LPGENW("Grumpy"),       "grumpy"        },
	{ LPGENW("Guilty"),       "guilty"        },
	{ LPGENW("Happy"),        "happy"         },
	{ LPGENW("Hopeful"),      "hopeful"       },
	{ LPGENW("Hot"),          "hot"           },
	{ LPGENW("Humbled"),      "humbled"       },
	{ LPGENW("Humiliated"),   "humiliated"    },
	{ LPGENW("Hungry"),       "hungry"        },
	{ LPGENW("Hurt"),         "hurt"          },
	{ LPGENW("Impressed"),    "impressed"     },
	{ LPGENW("In awe"),       "in_awe"        },
	{ LPGENW("In love"),      "in_love"       },
	{ LPGENW("Indignant"),    "indignant"     },
	{ LPGENW("Interested"),   "interested"    },
	{ LPGENW("Intoxicated"),  "intoxicated"   },
	{ LPGENW("Invincible"),   "invincible"    },
	{ LPGENW("Jealous"),      "jealous"       },
	{ LPGENW("Lonely"),       "lonely"        },
	{ LPGENW("Lost"),         "lost"          },
	{ LPGENW("Lucky"),        "lucky"         },
	{ LPGENW("Mean"),         "mean"          },
	{ LPGENW("Moody"),        "moody"         },
	{ LPGENW("Nervous"),      "nervous"       },
	{ LPGENW("Neutral"),      "neutral"       },
	{ LPGENW("Offended"),     "offended"      },
	{ LPGENW("Outraged"),     "outraged"      },
	{ LPGENW("Playful"),      "playful"       },
	{ LPGENW("Proud"),        "proud"         },
	{ LPGENW("Relaxed"),      "relaxed"       },
	{ LPGENW("Relieved"),     "relieved"      },
	{ LPGENW("Remorseful"),   "remorseful"    },
	{ LPGENW("Restless"),     "restless"      },
	{ LPGENW("Sad"),          "sad"           },
	{ LPGENW("Sarcastic"),    "sarcastic"     },
	{ LPGENW("Satisfied"),    "satisfied"     },
	{ LPGENW("Serious"),      "serious"       },
	{ LPGENW("Shocked"),      "shocked"       },
	{ LPGENW("Shy"),          "shy"           },
	{ LPGENW("Sick"),         "sick"          },
	{ LPGENW("Sleepy"),       "sleepy"        },
	{ LPGENW("Spontaneous"),  "spontaneous"   },
	{ LPGENW("Stressed"),     "stressed"      },
	{ LPGENW("Strong"),       "strong"        },
	{ LPGENW("Surprised"),    "surprised"     },
	{ LPGENW("Thankful"),     "thankful"      },
	{ LPGENW("Thirsty"),      "thirsty"       },
	{ LPGENW("Tired"),        "tired"         },
	{ LPGENW("Undefined"),    "undefined"     },
	{ LPGENW("Weak"),         "weak"          },
	{ LPGENW("Worried"),      "worried"       },
};

CPepMood::CPepMood(CJabberProto *proto) :
	CPepGuiService(proto, "Mood", JABBER_FEAT_USER_MOOD),
	m_text(nullptr),
	m_mode(-1)
{
	UpdateMenuItem(Skin_GetIconHandle(SKINICON_OTHER_SMALLDOT), LPGENW("Set mood..."));
}

CPepMood::~CPepMood()
{
	mir_free(m_text);
}

void CPepMood::ProcessItems(const char *from, const TiXmlElement *itemsNode)
{
	MCONTACT hContact = 0, hSelfContact = 0;
	if (!m_proto->IsMyOwnJID(from)) {
		hContact = m_proto->HContactFromJID(from);
		if (!hContact) return;
	}
	else hSelfContact = m_proto->HContactFromJID(from);

	if (XmlFirstChild(itemsNode, "retract")) {
		if (hSelfContact)
			SetMood(hSelfContact, nullptr, nullptr);
		SetMood(hContact, nullptr, nullptr);
		return;
	}

	auto *moodNode = XmlGetChildByTag(XmlFirstChild(itemsNode, "item"), "mood", "xmlns", JABBER_FEAT_USER_MOOD);
	if (!moodNode)
		return;

	const char *moodType = nullptr, *moodText = nullptr;
	for (auto *n : TiXmlEnum(moodNode)) {
		if (!mir_strcmp(n->Name(), "text"))
			moodText = n->GetText();
		else
			moodType = n->Name();
	}

	wchar_t *fixedText = JabberStrFixLines(Utf2T(moodText));
	if (hSelfContact)
		SetMood(hSelfContact, Utf2T(moodType), fixedText);
	SetMood(hContact, Utf2T(moodType), fixedText);
	mir_free(fixedText);

	if (!hContact && m_mode >= 0)
		ForceRepublishOnLogin();
}

void CPepMood::CreateData(TiXmlElement *n)
{
	TiXmlElement *moodNode = n << XCHILDNS("mood", JABBER_FEAT_USER_MOOD);
	moodNode << XCHILD(g_arrMoods[m_mode].szTag);
	if (m_text)
		moodNode << XCHILD("text", T2Utf(m_text));
}

void CPepMood::ResetExtraIcon(MCONTACT hContact)
{
	char *szMood = m_proto->ReadAdvStatusA(hContact, ADVSTATUS_MOOD, "id");
	SetExtraIcon(hContact, szMood);
	mir_free(szMood);
}

void CPepMood::SetExtraIcon(MCONTACT hContact, char *szMood)
{
	ExtraIcon_SetIcon(hExtraMood, hContact, szMood == nullptr ? nullptr : g_MoodIcons.GetIcolibHandle(szMood));
}

void CPepMood::SetMood(MCONTACT hContact, const wchar_t *szMood, const wchar_t *szText)
{
	int mood = -1;
	if (szMood) {
		char* p = mir_u2a(szMood);

		for (int i = 1; i < _countof(g_arrMoods); i++)
			if (!mir_strcmp(g_arrMoods[i].szTag, p)) {
				mood = i;
				break;
			}

		mir_free(p);

		if (mood < 0)
			return;
	}

	if (!hContact) {
		m_mode = mood;
		replaceStrW(m_text, szText);

		UpdateMenuView();
	}
	else SetExtraIcon(hContact, mood < 0 ? nullptr : g_arrMoods[mood].szTag);

	if (szMood) {
		m_proto->setByte(hContact, DBSETTING_XSTATUSID, mood);
		m_proto->setWString(hContact, DBSETTING_XSTATUSNAME, TranslateW(g_arrMoods[mood].szName));
		if (szText)
			m_proto->setWString(hContact, DBSETTING_XSTATUSMSG, szText);
		else
			m_proto->delSetting(hContact, DBSETTING_XSTATUSMSG);

		m_proto->WriteAdvStatus(hContact, ADVSTATUS_MOOD, szMood, g_MoodIcons.GetIcolibName(g_arrMoods[mood].szTag), TranslateW(g_arrMoods[mood].szName), szText);
	}
	else {
		m_proto->delSetting(hContact, DBSETTING_XSTATUSID);
		m_proto->delSetting(hContact, DBSETTING_XSTATUSNAME);
		m_proto->delSetting(hContact, DBSETTING_XSTATUSMSG);

		m_proto->ResetAdvStatus(hContact, ADVSTATUS_MOOD);
	}

	NotifyEventHooks(m_proto->m_hEventXStatusChanged, hContact, 0);
}

void CPepMood::ShowSetDialog()
{
	CJabberDlgPepSimple dlg(m_proto, TranslateT("Set Mood"));
	for (int i = 1; i < _countof(g_arrMoods); i++)
		dlg.AddStatusMode(i, g_arrMoods[i].szTag, g_MoodIcons.GetIcon(g_arrMoods[i].szTag), TranslateW(g_arrMoods[i].szName));

	dlg.SetActiveStatus(m_mode, m_text);
	if (!dlg.DoModal())
		return;

	m_mode = dlg.GetStatusMode();
	replaceStrW(m_text, dlg.GetStatusText());

	if (m_mode >= 0) {
		replaceStrW(m_text, dlg.GetStatusText());
		Publish();
	}
	else Retract();

	UpdateMenuView();
}

void CPepMood::UpdateMenuView()
{
	HANDLE hIcon;
	wchar_t title[128];
	if (m_mode >= 0) {
		mir_snwprintf(title, TranslateT("Mood: %s"), TranslateW(g_arrMoods[m_mode].szName));
		hIcon = g_MoodIcons.GetIcolibHandle(g_arrMoods[m_mode].szTag);
	}
	else {
		mir_wstrcpy(title, TranslateT("Set mood..."));
		hIcon = Skin_GetIconHandle(SKINICON_OTHER_SMALLDOT);
	}

	UpdateMenuItem(hIcon, title);
}

void ClearMoodIcons()
{
	for (auto &it : g_arrMoods)
		it.clistIconId = 0;
}

int GetMoodIconIdx(int xStatusId)
{
	if (xStatusId <= 0 || xStatusId >= _countof(g_arrMoods))
		return -1;

	auto &pIcon = g_arrMoods[xStatusId];
	if (pIcon.clistIconId == 0)
		pIcon.clistIconId = ImageList_AddIcon(Clist_GetImageList(), g_MoodIcons.GetIcon(pIcon.szTag));

	return pIcon.clistIconId;
}

///////////////////////////////////////////////////////////////////////////////
// CPepActivity

#define ACTIVITY_ICON(section, item)	-(1300 + (section) * 20 + (item))

struct
{
	char *szFirst;
	char *szSecond;
	wchar_t *szTitle;
	int iconid;
}
static g_arrActivities[] =
{
	{ "doing_chores", nullptr,       LPGENW("Doing chores"),       ACTIVITY_ICON(0,  0) },
	{ nullptr, "buying_groceries",   LPGENW("buying groceries"),   ACTIVITY_ICON(0,  1) },
	{ nullptr, "cleaning",           LPGENW("cleaning"),           ACTIVITY_ICON(0,  2) },
	{ nullptr, "cooking",            LPGENW("cooking"),            ACTIVITY_ICON(0,  3) },
	{ nullptr, "doing_maintenance",  LPGENW("doing maintenance"),  ACTIVITY_ICON(0,  4) },
	{ nullptr, "doing_the_dishes",   LPGENW("doing the dishes"),   ACTIVITY_ICON(0,  5) },
	{ nullptr, "doing_the_laundry",  LPGENW("doing the laundry"),  ACTIVITY_ICON(0,  6) },
	{ nullptr, "gardening",          LPGENW("gardening"),          ACTIVITY_ICON(0,  7) },
	{ nullptr, "running_an_errand",  LPGENW("running an errand"),  ACTIVITY_ICON(0,  8) },
	{ nullptr, "walking_the_dog",    LPGENW("walking the dog"),    ACTIVITY_ICON(0,  9) },
	{ "drinking", nullptr,           LPGENW("Drinking"),           ACTIVITY_ICON(1,  0) },
	{ nullptr, "having_a_beer",      LPGENW("having a beer"),      ACTIVITY_ICON(1,  1) },
	{ nullptr, "having_coffee",      LPGENW("having coffee"),      ACTIVITY_ICON(1,  2) },
	{ nullptr, "having_tea",         LPGENW("having tea"),         ACTIVITY_ICON(1,  3) },
	{ "eating", nullptr,             LPGENW("Eating"),             ACTIVITY_ICON(2,  0) },
	{ nullptr, "having_a_snack",     LPGENW("having a snack"),     ACTIVITY_ICON(2,  1) },
	{ nullptr, "having_breakfast",   LPGENW("having breakfast"),   ACTIVITY_ICON(2,  2) },
	{ nullptr, "having_dinner",      LPGENW("having dinner"),      ACTIVITY_ICON(2,  3) },
	{ nullptr, "having_lunch",       LPGENW("having lunch"),       ACTIVITY_ICON(2,  4) },
	{ "exercising", nullptr,         LPGENW("Exercising"),         ACTIVITY_ICON(3,  0) },
	{ nullptr, "cycling",            LPGENW("cycling"),            ACTIVITY_ICON(3,  1) },
	{ nullptr, "dancing",            LPGENW("dancing"),            ACTIVITY_ICON(3,  2) },
	{ nullptr, "hiking",             LPGENW("hiking"),             ACTIVITY_ICON(3,  3) },
	{ nullptr, "jogging",            LPGENW("jogging"),            ACTIVITY_ICON(3,  4) },
	{ nullptr, "playing_sports",     LPGENW("playing sports"),     ACTIVITY_ICON(3,  5) },
	{ nullptr, "running",            LPGENW("running"),            ACTIVITY_ICON(3,  6) },
	{ nullptr, "skiing",             LPGENW("skiing"),             ACTIVITY_ICON(3,  7) },
	{ nullptr, "swimming",           LPGENW("swimming"),           ACTIVITY_ICON(3,  8) },
	{ nullptr, "working_out",        LPGENW("working out"),        ACTIVITY_ICON(3,  9) },
	{ "grooming", nullptr,           LPGENW("Grooming"),           ACTIVITY_ICON(4,  0) },
	{ nullptr, "at_the_spa",         LPGENW("at the spa"),         ACTIVITY_ICON(4,  1) },
	{ nullptr, "brushing_teeth",     LPGENW("brushing teeth"),     ACTIVITY_ICON(4,  2) },
	{ nullptr, "getting_a_haircut",  LPGENW("getting a haircut"),  ACTIVITY_ICON(4,  3) },
	{ nullptr, "shaving",            LPGENW("shaving"),            ACTIVITY_ICON(4,  4) },
	{ nullptr, "taking_a_bath",      LPGENW("taking a bath"),      ACTIVITY_ICON(4,  5) },
	{ nullptr, "taking_a_shower",    LPGENW("taking a shower"),    ACTIVITY_ICON(4,  6) },
	{ "having_appointment", nullptr, LPGENW("Having appointment"), ACTIVITY_ICON(5,  0) },
	{ "inactive", nullptr,           LPGENW("Inactive"),           ACTIVITY_ICON(6,  0) },
	{ nullptr, "day_off",            LPGENW("day off"),            ACTIVITY_ICON(6,  1) },
	{ nullptr, "hanging_out",        LPGENW("hanging out"),        ACTIVITY_ICON(6,  2) },
	{ nullptr, "hiding",             LPGENW("hiding"),             ACTIVITY_ICON(6,  3) },
	{ nullptr, "on_vacation",        LPGENW("on vacation"),        ACTIVITY_ICON(6,  4) },
	{ nullptr, "praying",            LPGENW("praying"),            ACTIVITY_ICON(6,  5) },
	{ nullptr, "scheduled_holiday",  LPGENW("scheduled holiday"),  ACTIVITY_ICON(6,  6) },
	{ nullptr, "sleeping",           LPGENW("sleeping"),           ACTIVITY_ICON(6,  7) },
	{ nullptr, "thinking",           LPGENW("thinking"),           ACTIVITY_ICON(6,  8) },
	{ "relaxing", nullptr,           LPGENW("Relaxing"),           ACTIVITY_ICON(7,  0) },
	{ nullptr, "fishing",            LPGENW("fishing"),            ACTIVITY_ICON(7,  1) },
	{ nullptr, "gaming",             LPGENW("gaming"),             ACTIVITY_ICON(7,  2) },
	{ nullptr, "going_out",          LPGENW("going out"),          ACTIVITY_ICON(7,  3) },
	{ nullptr, "partying",           LPGENW("partying"),           ACTIVITY_ICON(7,  4) },
	{ nullptr, "reading",            LPGENW("reading"),            ACTIVITY_ICON(7,  5) },
	{ nullptr, "rehearsing",         LPGENW("rehearsing"),         ACTIVITY_ICON(7,  6) },
	{ nullptr, "shopping",           LPGENW("shopping"),           ACTIVITY_ICON(7,  7) },
	{ nullptr, "smoking",            LPGENW("smoking"),            ACTIVITY_ICON(7,  8) },
	{ nullptr, "socializing",        LPGENW("socializing"),        ACTIVITY_ICON(7,  9) },
	{ nullptr, "sunbathing",         LPGENW("sunbathing"),         ACTIVITY_ICON(7,  10) },
	{ nullptr, "watching_tv",        LPGENW("watching TV"),        ACTIVITY_ICON(7,  11) },
	{ nullptr, "watching_a_movie",   LPGENW("watching a movie"),   ACTIVITY_ICON(7,  12) },
	{ "talking", nullptr,            LPGENW("Talking"),            ACTIVITY_ICON(8,  0) },
	{ nullptr, "in_real_life",       LPGENW("in real life"),       ACTIVITY_ICON(8,  1) },
	{ nullptr, "on_the_phone",       LPGENW("on the phone"),       ACTIVITY_ICON(8,  2) },
	{ nullptr, "on_video_phone",     LPGENW("on video phone"),     ACTIVITY_ICON(8,  3) },
	{ "traveling", nullptr,          LPGENW("Traveling"),          ACTIVITY_ICON(9,  0) },
	{ nullptr, "commuting",          LPGENW("commuting"),          ACTIVITY_ICON(9,  1) },
	{ nullptr, "cycling",            LPGENW("cycling"),            ACTIVITY_ICON(9,  2) },
	{ nullptr, "driving",            LPGENW("driving"),            ACTIVITY_ICON(9,  3) },
	{ nullptr, "in_a_car",           LPGENW("in a car"),           ACTIVITY_ICON(9,  4) },
	{ nullptr, "on_a_bus",           LPGENW("on a bus"),           ACTIVITY_ICON(9,  5) },
	{ nullptr, "on_a_plane",         LPGENW("on a plane"),         ACTIVITY_ICON(9,  6) },
	{ nullptr, "on_a_train",         LPGENW("on a train"),         ACTIVITY_ICON(9,  7) },
	{ nullptr, "on_a_trip",          LPGENW("on a trip"),          ACTIVITY_ICON(9,  8) },
	{ nullptr, "walking",            LPGENW("walking"),            ACTIVITY_ICON(9,  9) },
	{ "working", nullptr,            LPGENW("Working"),            ACTIVITY_ICON(10,  0) },
	{ nullptr, "coding",             LPGENW("coding"),             ACTIVITY_ICON(10,  1) },
	{ nullptr, "in_a_meeting",       LPGENW("in a meeting"),       ACTIVITY_ICON(10,  2) },
	{ nullptr, "studying",           LPGENW("studying"),           ACTIVITY_ICON(10,  3) },
	{ nullptr, "writing",            LPGENW("writing"),            ACTIVITY_ICON(10,  4) },
	{ nullptr, nullptr, nullptr } // the end, don't delete this
};

inline char *ActivityGetId(int id)
{
	return g_arrActivities[id].szSecond ? g_arrActivities[id].szSecond : g_arrActivities[id].szFirst;
}

// -1 if not found, otherwise activity number
static int ActivityCheck(const char *szFirstNode, const char *szSecondNode)
{
	if (!szFirstNode) return 0;

	int i = 0, nFirst = -1, nSecond = -1;
	while (g_arrActivities[i].szFirst || g_arrActivities[i].szSecond) {
		// check first node
		if (g_arrActivities[i].szFirst && !mir_strcmp(szFirstNode, g_arrActivities[i].szFirst)) {
			// first part found
			nFirst = i;
			if (!szSecondNode) {
				nSecond = i;
				break;
			}
			i++; // move to next
			while (g_arrActivities[i].szSecond) {
				if (!mir_strcmp(g_arrActivities[i].szSecond, szSecondNode)) {
					nSecond = i;
					break;
				}
				i++;
			}
			break;
		}
		i++;
	}

	if (nSecond != -1)
		return nSecond;

	return nFirst;
}

char* returnActivity(int id)
{
	if (g_arrActivities[id].szFirst)
		return g_arrActivities[id].szFirst;
	if (g_arrActivities[id].szSecond)
		return g_arrActivities[id].szSecond;
	return nullptr;
}

char* ActivityGetFirst(int id)
{
	if (id >= _countof(g_arrActivities) - 1)
		return nullptr;

	while (id >= 0) {
		if (g_arrActivities[id].szFirst)
			return g_arrActivities[id].szFirst;
		--id;
	}

	return nullptr;
}

char *ActivityGetFirst(char *szId)
{
	if (!szId) return nullptr;

	int id = _countof(g_arrActivities) - 1;
	bool found_second = false;

	while (id >= 0) {
		if (g_arrActivities[id].szFirst && (found_second || !mir_strcmp(g_arrActivities[id].szFirst, szId)))
			return g_arrActivities[id].szFirst;
		if (g_arrActivities[id].szSecond && !found_second && !mir_strcmp(g_arrActivities[id].szSecond, szId))
			found_second = true;
		--id;
	}

	return nullptr;
}

char *ActivityGetSecond(int id)
{
	return (id >= 0) ? g_arrActivities[id].szSecond : nullptr;
}

wchar_t *ActivityGetFirstTitle(int id)
{
	if (id >= _countof(g_arrActivities) - 1)
		return nullptr;

	while (id >= 0) {
		if (g_arrActivities[id].szFirst)
			return g_arrActivities[id].szTitle;
		--id;
	}

	return nullptr;
}

wchar_t *ActivityGetSecondTitle(int id)
{
	return ((id >= 0) && g_arrActivities[id].szSecond) ? g_arrActivities[id].szTitle : nullptr;
}

void ActivityBuildTitle(int id, wchar_t *buf, int size)
{
	wchar_t *szFirst = ActivityGetFirstTitle(id);
	wchar_t *szSecond = ActivityGetSecondTitle(id);

	if (szFirst) {
		if (szSecond)
			mir_snwprintf(buf, size, L"%s [%s]", TranslateW(szFirst), TranslateW(szSecond));
		else
			mir_wstrncpy(buf, TranslateW(szFirst), size);
	}
	else *buf = 0;
}

CPepActivity::CPepActivity(CJabberProto *proto) :
	CPepGuiService(proto, "Activity", JABBER_FEAT_USER_ACTIVITY),
	m_text(nullptr),
	m_mode(-1)
{
	UpdateMenuItem(Skin_GetIconHandle(SKINICON_OTHER_SMALLDOT), LPGENW("Set activity..."));
}

CPepActivity::~CPepActivity()
{
	mir_free(m_text);
}

void CPepActivity::ProcessItems(const char *from, const TiXmlElement *itemsNode)
{
	MCONTACT hContact = 0, hSelfContact = 0;
	if (!m_proto->IsMyOwnJID(from)) {
		hContact = m_proto->HContactFromJID(from);
		if (!hContact) return;
	}
	else hSelfContact = m_proto->HContactFromJID(from);

	if (XmlFirstChild(itemsNode, "retract")) {
		if (hSelfContact)
			SetActivity(hSelfContact, nullptr, nullptr, nullptr);
		SetActivity(hContact, nullptr, nullptr, nullptr);
		return;
	}

	auto *actNode = XmlGetChildByTag(XmlFirstChild(itemsNode, "item"), "activity", "xmlns", JABBER_FEAT_USER_ACTIVITY);
	if (!actNode)
		return;

	const char *szText = XmlGetChildText(actNode, "text");
	const char *szFirstNode = nullptr, *szSecondNode = nullptr;

	for (auto *n : TiXmlFilter(actNode, "text")) {
		if (mir_strcmp(n->Name(), "text")) {
			szFirstNode = n->Name();
			auto *secondNode = XmlFirstChild(n);
			if (szFirstNode && secondNode && secondNode->Name())
				szSecondNode = secondNode->Name();
			break;
		}
	}

	ptrW fixedText(JabberStrFixLines(Utf2T(szText)));
	if (hSelfContact)
		SetActivity(hSelfContact, szFirstNode, szSecondNode, fixedText);
	SetActivity(hContact, szFirstNode, szSecondNode, fixedText);
	mir_free(fixedText);

	if (!hContact && m_mode >= 0)
		ForceRepublishOnLogin();
}

void CPepActivity::CreateData(TiXmlElement *n)
{
	char *szFirstNode = ActivityGetFirst(m_mode);
	char *szSecondNode = ActivityGetSecond(m_mode);

	TiXmlElement *activityNode = n << XCHILDNS("activity", JABBER_FEAT_USER_ACTIVITY);
	TiXmlElement *firstNode = activityNode << XCHILD(szFirstNode);

	if (firstNode && szSecondNode)
		firstNode << XCHILD(szSecondNode);

	if (m_text)
		activityNode << XCHILD("text", T2Utf(m_text));
}

void CPepActivity::ResetExtraIcon(MCONTACT hContact)
{
	char *szActivity = m_proto->ReadAdvStatusA(hContact, ADVSTATUS_ACTIVITY, "id");
	SetExtraIcon(hContact, szActivity);
	mir_free(szActivity);
}

void CPepActivity::SetExtraIcon(MCONTACT hContact, char *szActivity)
{
	ExtraIcon_SetIcon(hExtraActivity, hContact, szActivity == nullptr ? nullptr : g_ActivityIcons.GetIcolibHandle(szActivity));
}

void CPepActivity::SetActivity(MCONTACT hContact, const char *szFirst, const char *szSecond, const wchar_t *szText)
{
	int activity = -1;
	if (szFirst || szSecond) {
		activity = ActivityCheck(szFirst, szSecond);
		if (activity < 0)
			return;
	}

	if (!hContact) {
		m_mode = activity;
		replaceStrW(m_text, szText);

		UpdateMenuView();
	}
	else SetExtraIcon(hContact, activity < 0 ? nullptr : returnActivity(activity));

	if (activity >= 0) {
		wchar_t activityTitle[128];
		ActivityBuildTitle(activity, activityTitle, _countof(activityTitle));
		m_proto->WriteAdvStatus(hContact, ADVSTATUS_ACTIVITY, _A2T(ActivityGetId(activity)), g_ActivityIcons.GetIcolibName(returnActivity(activity)), activityTitle, szText);
	}
	else m_proto->ResetAdvStatus(hContact, ADVSTATUS_ACTIVITY);
}

void CPepActivity::ShowSetDialog()
{
	CJabberDlgPepSimple dlg(m_proto, TranslateT("Set Activity"));
	for (int i = 0; i < _countof(g_arrActivities); i++)
		if (g_arrActivities[i].szFirst || g_arrActivities[i].szSecond)
			dlg.AddStatusMode(i, ActivityGetId(i), g_ActivityIcons.GetIcon(returnActivity(i)), TranslateW(g_arrActivities[i].szTitle), (g_arrActivities[i].szSecond != nullptr));

	dlg.SetActiveStatus(m_mode, m_text);
	if (!dlg.DoModal())
		return;

	m_mode = dlg.GetStatusMode();
	if (m_mode >= 0) {
		replaceStrW(m_text, dlg.GetStatusText());
		Publish();
	}
	else Retract();

	UpdateMenuView();
}

void CPepActivity::UpdateMenuView()
{
	HANDLE hIcon;
	wchar_t title[128];

	if (m_mode >= 0) {
		wchar_t activityTitle[128];
		ActivityBuildTitle(m_mode, activityTitle, _countof(activityTitle));
		mir_snwprintf(title, TranslateT("Activity: %s"), activityTitle);
		hIcon = g_ActivityIcons.GetIcolibHandle(returnActivity(m_mode));
	}
	else {
		mir_wstrcpy(title, TranslateT("Set activity..."));
		hIcon = Skin_GetIconHandle(SKINICON_OTHER_SMALLDOT);
	}

	UpdateMenuItem(hIcon, title);
}

///////////////////////////////////////////////////////////////////////////////
// icq api emulation

HICON CJabberProto::GetXStatusIcon(int bStatus, UINT flags)
{
	HICON icon = g_MoodIcons.GetIcon(g_arrMoods[bStatus].szTag, (flags & LR_BIGICON) != 0);
	return (flags & LR_SHARED) ? icon : CopyIcon(icon);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetXStatusIcon - Retrieves specified custom status icon
// wParam = (int)N  // custom status id, 0 = my current custom status
// lParam = flags   // use LR_SHARED for shared HICON
// return = HICON   // custom status icon (use DestroyIcon to release resources if not LR_SHARED)

INT_PTR __cdecl CJabberProto::OnGetXStatusIcon(WPARAM wParam, LPARAM lParam)
{
	if (!m_bJabberOnline)
		return 0;

	if (!wParam)
		wParam = ((CPepMood*)m_pepServices.Find(JABBER_FEAT_USER_MOOD))->m_mode;

	if (wParam < 1 || wParam >= _countof(g_arrMoods))
		return 0;

	int flags = 0;
	if (lParam & LR_SHARED)  flags |= LR_SHARED;
	if (lParam & LR_BIGICON) flags |= LR_BIGICON;

	return (INT_PTR)GetXStatusIcon(wParam, flags);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SendPepMood - sends mood

bool CJabberProto::SendPepTune(wchar_t* szArtist, wchar_t* szLength, wchar_t* szSource, wchar_t* szTitle, wchar_t* szTrack, wchar_t* szUri)
{
	if (!m_bJabberOnline || !m_bPepSupported)
		return false;

	XmlNodeIq iq("set", SerialNext());
	TiXmlElement *tuneNode = iq << XCHILDNS("pubsub", JABBER_FEAT_PUBSUB)
		<< XCHILD("publish") << XATTR("node", JABBER_FEAT_USER_TUNE)
		<< XCHILD("item") << XCHILDNS("tune", JABBER_FEAT_USER_TUNE);

	if (szArtist || szLength || szSource || szTitle || szUri) {
		if (szArtist) tuneNode << XCHILD("artist", T2Utf(szArtist));
		if (szLength) tuneNode << XCHILD("length", T2Utf(szLength));
		if (szSource) tuneNode << XCHILD("source", T2Utf(szSource));
		if (szTitle) tuneNode << XCHILD("title", T2Utf(szTitle));
		if (szTrack) tuneNode << XCHILD("track", T2Utf(szTrack));
		if (szUri) tuneNode << XCHILD("uri", T2Utf(szUri));
	}
	m_ThreadInfo->send(iq);
	return true;
}

void CJabberProto::SetContactTune(MCONTACT hContact, const wchar_t *szArtist, const wchar_t *szLength, const wchar_t *szSource, const wchar_t *szTitle, const wchar_t *szTrack)
{
	if (!szArtist && !szTitle) {
		delSetting(hContact, "ListeningTo");
		ResetAdvStatus(hContact, ADVSTATUS_TUNE);
		return;
	}

	wchar_t *szListeningTo;
	if (ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT)) {
		LISTENINGTOINFO li;
		memset(&li, 0, sizeof(li));
		li.cbSize = sizeof(li);
		li.dwFlags = LTI_TCHAR;
		li.ptszArtist = (wchar_t*)szArtist;
		li.ptszLength = (wchar_t*)szLength;
		li.ptszAlbum = (wchar_t*)szSource;
		li.ptszTitle = (wchar_t*)szTitle;
		li.ptszTrack = (wchar_t*)szTrack;
		szListeningTo = (wchar_t*)CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM)L"%title% - %artist%", (LPARAM)&li);
	}
	else {
		szListeningTo = (wchar_t*)mir_alloc(2048 * sizeof(wchar_t));
		mir_snwprintf(szListeningTo, 2047, L"%s - %s", szTitle ? szTitle : L"", szArtist ? szArtist : L"");
	}

	setWString(hContact, "ListeningTo", szListeningTo);

	char tuneIcon[128];
	mir_snprintf(tuneIcon, "%s_%s", m_szModuleName, "main");
	WriteAdvStatus(hContact, ADVSTATUS_TUNE, L"listening_to", tuneIcon, TranslateT("Listening To"), szListeningTo);

	mir_free(szListeningTo);
}

wchar_t* a2tf(const wchar_t *str, BOOL unicode)
{
	if (str == nullptr)
		return nullptr;

	return (unicode) ? mir_wstrdup(str) : mir_a2u((char*)str);
}

void overrideStr(wchar_t*& dest, const wchar_t *src, BOOL unicode, const wchar_t *def = nullptr)
{
	if (dest != nullptr) {
		mir_free(dest);
		dest = nullptr;
	}

	if (src != nullptr)
		dest = a2tf(src, unicode);
	else if (def != nullptr)
		dest = mir_wstrdup(def);
}

INT_PTR __cdecl CJabberProto::OnSetListeningTo(WPARAM, LPARAM lParam)
{
	LISTENINGTOINFO *cm = (LISTENINGTOINFO *)lParam;
	if (!cm || cm->cbSize != sizeof(LISTENINGTOINFO)) {
		SendPepTune(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		delSetting("ListeningTo");
	}
	else {
		wchar_t *szArtist = nullptr, *szLength = nullptr, *szSource = nullptr;
		wchar_t *szTitle = nullptr, *szTrack = nullptr;

		BOOL unicode = cm->dwFlags & LTI_UNICODE;

		overrideStr(szArtist, cm->ptszArtist, unicode);
		overrideStr(szSource, cm->ptszAlbum, unicode);
		overrideStr(szTitle, cm->ptszTitle, unicode);
		overrideStr(szTrack, cm->ptszTrack, unicode);
		overrideStr(szLength, cm->ptszLength, unicode);

		wchar_t szLengthInSec[32];
		szLengthInSec[0] = 0;
		if (szLength) {
			unsigned int multiplier = 1, result = 0;
			for (wchar_t *p = szLength; *p; p++)
				if (*p == ':')
					multiplier *= 60;

			if (multiplier <= 3600) {
				wchar_t *szTmp = szLength;
				while (szTmp[0]) {
					result += (_wtoi(szTmp) * multiplier);
					multiplier /= 60;
					szTmp = wcschr(szTmp, ':');
					if (!szTmp)
						break;
					szTmp++;
				}
			}
			mir_snwprintf(szLengthInSec, L"%d", result);
		}

		SendPepTune(szArtist, szLength ? szLengthInSec : nullptr, szSource, szTitle, szTrack, nullptr);
		SetContactTune(0, szArtist, szLength, szSource, szTitle, szTrack);

		mir_free(szArtist);
		mir_free(szLength);
		mir_free(szSource);
		mir_free(szTitle);
		mir_free(szTrack);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// builds xstatus menu

void CJabberProto::XStatusInit()
{
	RegisterAdvStatusSlot(ADVSTATUS_MOOD);
	RegisterAdvStatusSlot(ADVSTATUS_TUNE);
	RegisterAdvStatusSlot(ADVSTATUS_ACTIVITY);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberSetXStatus - sets the extended status info (mood)

INT_PTR __cdecl CJabberProto::OnGetXStatusEx(WPARAM hContact, LPARAM lParam)
{
	if (!m_bPepSupported || !m_bJabberOnline)
		return 1;

	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;
	if (pData->cbSize < sizeof(CUSTOM_STATUS))
		return 1;

	CPepMood *pepMood = (CPepMood*)m_pepServices.Find(JABBER_FEAT_USER_MOOD);
	if (pepMood == nullptr)
		return 1;

	// fill status member
	if (pData->flags & CSSF_MASK_STATUS)
		*pData->status = pepMood->m_mode;

	// fill status name member
	if (pData->flags & CSSF_MASK_NAME) {
		if (pData->flags & CSSF_DEFAULT_NAME) {
			uint32_t dwXStatus = (pData->wParam == nullptr) ? pepMood->m_mode : *pData->wParam;
			if (dwXStatus >= _countof(g_arrMoods))
				return 1;

			if (pData->flags & CSSF_UNICODE)
				mir_wstrncpy(pData->pwszName, g_arrMoods[dwXStatus].szName, (STATUS_TITLE_MAX + 1));
			else {
				size_t dwStatusTitleSize = mir_wstrlen(g_arrMoods[dwXStatus].szName);
				if (dwStatusTitleSize > STATUS_TITLE_MAX)
					dwStatusTitleSize = STATUS_TITLE_MAX;

				WideCharToMultiByte(CP_ACP, 0, g_arrMoods[dwXStatus].szName, (uint32_t)dwStatusTitleSize, pData->pszName, MAX_PATH, nullptr, nullptr);
				pData->pszName[dwStatusTitleSize] = 0;
			}
		}
		else {
			*pData->ptszName = 0;
			if (pData->flags & CSSF_UNICODE) {
				ptrW title(ReadAdvStatusT(hContact, ADVSTATUS_MOOD, ADVSTATUS_VAL_TITLE));
				if (title)
					wcsncpy_s(pData->ptszName, STATUS_TITLE_MAX, title, _TRUNCATE);
			}
			else {
				ptrA title(ReadAdvStatusA(hContact, ADVSTATUS_MOOD, ADVSTATUS_VAL_TITLE));
				if (title)
					strncpy_s(pData->pszName, STATUS_TITLE_MAX, title, _TRUNCATE);
			}
		}
	}

	// fill status message member
	if (pData->flags & CSSF_MASK_MESSAGE) {
		*pData->pszMessage = 0;
		if (pData->flags & CSSF_UNICODE) {
			ptrW title(ReadAdvStatusT(hContact, ADVSTATUS_MOOD, ADVSTATUS_VAL_TEXT));
			if (title)
				wcsncpy_s(pData->ptszMessage, STATUS_TITLE_MAX, title, _TRUNCATE);
		}
		else {
			ptrA title(ReadAdvStatusA(hContact, ADVSTATUS_MOOD, ADVSTATUS_VAL_TEXT));
			if (title)
				strncpy_s(pData->pszMessage, STATUS_TITLE_MAX, title, _TRUNCATE);
		}
	}

	if (pData->flags & CSSF_DISABLE_UI)
		if (pData->wParam)
			*pData->wParam = true;

	if (pData->flags & CSSF_STATUSES_COUNT)
		if (pData->wParam)
			*pData->wParam = _countof(g_arrMoods);

	if (pData->flags & CSSF_STR_SIZES) {
		if (pData->wParam) *pData->wParam = STATUS_TITLE_MAX;
		if (pData->lParam) *pData->lParam = STATUS_DESC_MAX;
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnSetXStatusEx(WPARAM, LPARAM lParam)
{
	if (!m_bPepSupported || !m_bJabberOnline)
		return 1;

	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;
	if (pData->cbSize < sizeof(CUSTOM_STATUS))
		return 1;

	CPepMood *pepMood = (CPepMood*)m_pepServices.Find(JABBER_FEAT_USER_MOOD);

	int status = (pData->flags & CSSF_MASK_STATUS) ? *pData->status : pepMood->m_mode;
	if (status >= 0 && status < _countof(g_arrMoods)) {
		pepMood->m_mode = status;
		pepMood->m_text = (pData->flags & CSSF_MASK_MESSAGE) ? JabberStrFixLines(pData->ptszMessage) : nullptr;
		pepMood->UpdateMenuView();
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Advanced status slots
// DB data format:
//     Contact / AdvStatus / Proto/Status/id = mode_id
//     Contact / AdvStatus / Proto/Status/icon = icon
//     Contact / AdvStatus / Proto/Status/title = title
//     Contact / AdvStatus / Proto/Status/text = title

void CJabberProto::RegisterAdvStatusSlot(const char *pszSlot)
{
	char szSetting[256];
	mir_snprintf(szSetting, "AdvStatus/%s/%s", m_szModuleName, pszSlot);
	db_set_resident(szSetting, "id");
	db_set_resident(szSetting, "icon");
	db_set_resident(szSetting, "title");
	db_set_resident(szSetting, "text");
}

void CJabberProto::ResetAdvStatus(MCONTACT hContact, const char *pszSlot)
{
	char szSetting[128];
	mir_snprintf(szSetting, "%s/%s/id", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);

	mir_snprintf(szSetting, "%s/%s/icon", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);

	mir_snprintf(szSetting, "%s/%s/title", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);

	mir_snprintf(szSetting, "%s/%s/text", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);
}

void CJabberProto::WriteAdvStatus(MCONTACT hContact, const char *pszSlot, const wchar_t *pszMode, const char *pszIcon, const wchar_t *pszTitle, const wchar_t *pszText)
{
	char szSetting[128];

	mir_snprintf(szSetting, "%s/%s/id", m_szModuleName, pszSlot);
	db_set_ws(hContact, "AdvStatus", szSetting, pszMode);

	mir_snprintf(szSetting, "%s/%s/icon", m_szModuleName, pszSlot);
	db_set_s(hContact, "AdvStatus", szSetting, pszIcon);

	mir_snprintf(szSetting, "%s/%s/title", m_szModuleName, pszSlot);
	db_set_ws(hContact, "AdvStatus", szSetting, pszTitle);

	mir_snprintf(szSetting, "%s/%s/text", m_szModuleName, pszSlot);
	if (pszText)
		db_set_ws(hContact, "AdvStatus", szSetting, pszText);
	else
		db_unset(hContact, "AdvStatus", szSetting);
}

char* CJabberProto::ReadAdvStatusA(MCONTACT hContact, const char *pszSlot, const char *pszValue)
{
	char szSetting[128];
	mir_snprintf(szSetting, "%s/%s/%s", m_szModuleName, pszSlot, pszValue);
	return db_get_sa(hContact, "AdvStatus", szSetting);
}

wchar_t* CJabberProto::ReadAdvStatusT(MCONTACT hContact, const char *pszSlot, const char *pszValue)
{
	char szSetting[128];
	mir_snprintf(szSetting, "%s/%s/%s", m_szModuleName, pszSlot, pszValue);
	return db_get_wsa(hContact, "AdvStatus", szSetting);
}

///////////////////////////////////////////////////////////////////////////////
// Global functions

void g_XstatusIconsInit()
{
	wchar_t szFile[MAX_PATH];
	GetModuleFileName(g_plugin.getInst(), szFile, _countof(szFile));
	if (wchar_t *p = wcsrchr(szFile, '\\'))
		mir_wstrcpy(p + 1, L"..\\Icons\\xstatus_jabber.dll");

	wchar_t szSection[100];
	mir_wstrcpy(szSection, L"Protocols/Jabber/" LPGENW("Moods"));

	ClearMoodIcons();
	for (int i = 1; i < _countof(g_arrMoods); i++)
		g_MoodIcons.RegisterIcon(g_arrMoods[i].szTag, szFile, -(1200 + i), szSection, TranslateW(g_arrMoods[i].szName));

	mir_wstrcpy(szSection, L"Protocols/Jabber/" LPGENW("Activities"));
	for (int k = 0; k < _countof(g_arrActivities); k++) {
		if (g_arrActivities[k].szFirst)
			g_ActivityIcons.RegisterIcon(g_arrActivities[k].szFirst, szFile, g_arrActivities[k].iconid, szSection, TranslateW(g_arrActivities[k].szTitle));
		if (g_arrActivities[k].szSecond)
			g_ActivityIcons.RegisterIcon(g_arrActivities[k].szSecond, szFile, g_arrActivities[k].iconid, szSection, TranslateW(g_arrActivities[k].szTitle));
	}
}

void g_XstatusIconsUninit()
{
	g_MoodIcons.Clear();
	g_ActivityIcons.Clear();
}
