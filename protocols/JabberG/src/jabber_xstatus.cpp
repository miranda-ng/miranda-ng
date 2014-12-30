/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-14  Miranda NG project

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

#include "jabber.h"
#include "jabber_caps.h"

static CIconPool g_MoodIcons, g_ActivityIcons;

///////////////////////////////////////////////////////////////////////////////
// Simple dialog with timer and ok/cancel buttons

class CJabberDlgPepBase: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;
public:
	CJabberDlgPepBase(CJabberProto *proto, int id);

protected:
	CPepService *m_pepService;

	CCtrlButton m_btnOk;
	CCtrlButton m_btnCancel;

	void OnInitDialog();
	int Resizer(UTILRESIZECONTROL *urc);
	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void StopTimer();

private:
	int m_time;
};

CJabberDlgPepBase::CJabberDlgPepBase(CJabberProto *proto, int id):
	CJabberDlgBase(proto, id, NULL),
	m_btnOk(this, IDOK),
	m_btnCancel(this, IDCANCEL)
{
}

void CJabberDlgPepBase::OnInitDialog()
{
	CSuper::OnInitDialog();

	m_time = 5;
	SetTimer(m_hwnd, 1, 1000, NULL);

	TCHAR buf[128];
	mir_sntprintf(buf, SIZEOF(buf), TranslateT("OK (%d)"), m_time);
	m_btnOk.SetText(buf);
}

int CJabberDlgPepBase::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDOK:
	case IDCANCEL:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
	}

	return CSuper::Resizer(urc);
}

INT_PTR CJabberDlgPepBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_TIMER:
		if (wParam == 1) {
			TCHAR buf[128];
			mir_sntprintf(buf, SIZEOF(buf), TranslateT("OK (%d)"), --m_time);
			m_btnOk.SetText(buf);

			if (m_time < 0) {
				KillTimer(m_hwnd, 1);
				UIEmulateBtnClick(m_hwnd, IDOK);
			}

			return TRUE;
		}

		break;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}

void CJabberDlgPepBase::StopTimer()
{
	KillTimer(m_hwnd, 1);
	m_btnOk.SetText(TranslateT("OK"));
}

///////////////////////////////////////////////////////////////////////////////
// Simple PEP status
class CJabberDlgPepSimple: public CJabberDlgPepBase
{
	typedef CJabberDlgPepBase CSuper;
public:
	CJabberDlgPepSimple(CJabberProto *proto, TCHAR *title);
	~CJabberDlgPepSimple();

	bool OkClicked() { return m_bOkClicked; }
	void AddStatusMode(LPARAM id, char *name, HICON hIcon, TCHAR *title, bool subitem = false);
	void SetActiveStatus(LPARAM id, TCHAR *text);
	LPARAM GetStatusMode();
	TCHAR *GetStatusText();

protected:
	CCtrlCombo m_cbModes;
	CCtrlEdit m_txtDescription;

	void OnInitDialog();
	int Resizer(UTILRESIZECONTROL *urc);

	UI_MESSAGE_MAP(CJabberDlgPepSimple, CSuper);
		UI_MESSAGE(WM_MEASUREITEM, OnWmMeasureItem);
		UI_MESSAGE(WM_DRAWITEM, OnWmDrawItem);
		UI_MESSAGE(WM_GETMINMAXINFO, OnWmGetMinMaxInfo);
	UI_MESSAGE_MAP_END();

	BOOL OnWmMeasureItem(UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL OnWmDrawItem(UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL OnWmGetMinMaxInfo(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	struct CStatusMode
	{
		LPARAM m_id;
		char *m_name;
		HICON m_hIcon;
		TCHAR *m_title;
		bool m_subitem;

		CStatusMode(LPARAM id, char *name, HICON hIcon, TCHAR *title, bool subitem): m_id(id), m_name(name), m_hIcon(hIcon), m_title(title), m_subitem(subitem) {}
		~CStatusMode() { g_ReleaseIcon(m_hIcon); }
	};

	OBJLIST<CStatusMode> m_modes;
	TCHAR *m_text;
	TCHAR *m_title;
	int m_time;
	int m_prevSelected;
	int m_selected;
	bool m_bOkClicked;

	LPARAM m_active;
	TCHAR *m_activeText;

	void btnOk_OnClick(CCtrlButton *btn);
	void global_OnChange(CCtrlData *);
	void cbModes_OnChange(CCtrlData *);
};

CJabberDlgPepSimple::CJabberDlgPepSimple(CJabberProto *proto, TCHAR *title):
	CJabberDlgPepBase(proto, IDD_PEP_SIMPLE),
	m_cbModes(this, IDC_CB_MODES),
	m_txtDescription(this, IDC_TXT_DESCRIPTION),
	m_modes(10),
	m_text(NULL),
	m_selected(0),
	m_prevSelected(-1),
	m_active(-1),
	m_bOkClicked(false),
	m_title(title)
{
	m_btnOk.OnClick = Callback(this, &CJabberDlgPepSimple::btnOk_OnClick);
	m_cbModes.OnChange = Callback(this, &CJabberDlgPepSimple::cbModes_OnChange);
	m_cbModes.OnDropdown =
	m_txtDescription.OnChange = Callback(this, &CJabberDlgPepSimple::global_OnChange);

	m_modes.insert(new CStatusMode(-1, "<none>", LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT), TranslateT("None"), false));
}

CJabberDlgPepSimple::~CJabberDlgPepSimple()
{
	mir_free(m_text);
}

void CJabberDlgPepSimple::AddStatusMode(LPARAM id, char *name, HICON hIcon, TCHAR *title, bool subitem)
{
	m_modes.insert(new CStatusMode(id, name, hIcon, title, subitem));
}

void CJabberDlgPepSimple::SetActiveStatus(LPARAM id, TCHAR *text)
{
	m_active = id;
	m_activeText = text;
}

LPARAM CJabberDlgPepSimple::GetStatusMode()
{
	return m_modes[m_selected].m_id;
}

TCHAR *CJabberDlgPepSimple::GetStatusText()
{
	return m_text;
}

void CJabberDlgPepSimple::OnInitDialog()
{
	CSuper::OnInitDialog();

	WindowSetIcon(m_hwnd, m_proto, "main");
	SetWindowText(m_hwnd, m_title);

	m_txtDescription.Enable(false);
	for (int i=0; i < m_modes.getCount(); i++) {
		int idx = m_cbModes.AddString(m_modes[i].m_title, i);
		if ((m_modes[i].m_id == m_active) || !idx) {
			m_prevSelected = idx;
			m_cbModes.SetCurSel(idx);
			if (idx) m_txtDescription.Enable();
		}
	}

	if (m_activeText)
		m_txtDescription.SetText(m_activeText);
}

int CJabberDlgPepSimple::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_CB_MODES:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_TOP;
	case IDC_TXT_DESCRIPTION:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
	}

	return CSuper::Resizer(urc);
}

void CJabberDlgPepSimple::btnOk_OnClick(CCtrlButton*)
{
	m_text = m_txtDescription.GetText();
	m_selected = m_cbModes.GetCurSel();
	m_bOkClicked = true;
}

void CJabberDlgPepSimple::global_OnChange(CCtrlData *)
{
	StopTimer();
}

void CJabberDlgPepSimple::cbModes_OnChange(CCtrlData *)
{
	StopTimer();

	if (m_prevSelected == m_cbModes.GetCurSel())
		return;

	char szSetting[128];

	if ((m_prevSelected >= 0) && (m_modes[m_cbModes.GetItemData(m_prevSelected)].m_id >= 0)) {
		TCHAR *txt = m_txtDescription.GetText();
		mir_snprintf(szSetting, SIZEOF(szSetting), "PepMsg_%s", m_modes[m_cbModes.GetItemData(m_prevSelected)].m_name);
		m_proto->setTString(szSetting, txt);
		mir_free(txt);
	}

	m_prevSelected = m_cbModes.GetCurSel();
	if ((m_prevSelected >= 0) && (m_modes[m_cbModes.GetItemData(m_prevSelected)].m_id >= 0)) {
		mir_snprintf(szSetting, SIZEOF(szSetting), "PepMsg_%s", m_modes[m_cbModes.GetItemData(m_prevSelected)].m_name);

		ptrT szDescr( m_proto->getTStringA(szSetting));
		m_txtDescription.SetText((szDescr != NULL) ? szDescr : _T(""));
		m_txtDescription.Enable(true);
	}
	else {
		m_txtDescription.SetTextA("");
		m_txtDescription.Enable(false);
	}
}

BOOL CJabberDlgPepSimple::OnWmMeasureItem(UINT, WPARAM, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
	if (lpmis->CtlID != IDC_CB_MODES)
		return FALSE;

	TEXTMETRIC tm = {0};
	HDC hdc = GetDC(m_cbModes.GetHwnd());
	GetTextMetrics(hdc, &tm);
	ReleaseDC(m_cbModes.GetHwnd(), hdc);

	lpmis->itemHeight = max(tm.tmHeight, 18);
	if (lpmis->itemHeight < 18) lpmis->itemHeight = 18;

	return TRUE;
}

BOOL CJabberDlgPepSimple::OnWmDrawItem(UINT, WPARAM, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
	if (lpdis->CtlID != IDC_CB_MODES)
		return FALSE;

	if (lpdis->itemData == -1)
		return FALSE;

	CStatusMode *mode = &m_modes[lpdis->itemData];

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
		TCHAR text[128];
		if (mode->m_subitem) {
			for (int i = lpdis->itemData; i >= 0; --i)
				if (!m_modes[i].m_subitem) {
					mir_sntprintf(text, SIZEOF(text), _T("%s [%s]"), m_modes[i].m_title, mode->m_title);
					break;
				}
		}
		else mir_tstrncpy(text, mode->m_title, SIZEOF(text));

		DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 2, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2, mode->m_hIcon, 16, 16, 0, NULL, DI_NORMAL);
		TextOut(lpdis->hDC, lpdis->rcItem.left + 23, (lpdis->rcItem.top + lpdis->rcItem.bottom - tm.tmHeight) / 2, text, (int)mir_tstrlen(text));
	}
	else {
		TCHAR text[128];
		mir_sntprintf(text, SIZEOF(text), _T("...%s"), mode->m_title);
		DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 23, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2, mode->m_hIcon, 16, 16, 0, NULL, DI_NORMAL);
		TextOut(lpdis->hDC, lpdis->rcItem.left + 44, (lpdis->rcItem.top + lpdis->rcItem.bottom - tm.tmHeight) / 2, text, (int)mir_tstrlen(text));
	}

	return TRUE;
}

BOOL CJabberDlgPepSimple::OnWmGetMinMaxInfo(UINT, WPARAM, LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
	lpmmi->ptMinTrackSize.x = 200;
	lpmmi->ptMinTrackSize.y = 200;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CPepService base class

CPepService::CPepService(CJabberProto *proto, char *name, TCHAR *node):
	m_proto(proto),
	m_name(name),
	m_node(node),
	m_hMenuItem(NULL)
{
}

CPepService::~CPepService()
{
}

void CPepService::Publish()
{
	XmlNodeIq iq(_T("set"), m_proto->SerialNext());
	CreateData(
		iq << XCHILDNS(_T("pubsub"), JABBER_FEAT_PUBSUB)
			<< XCHILD(_T("publish")) << XATTR(_T("node"), m_node)
				<< XCHILD(_T("item")) << XATTR(_T("id"), _T("current")));
	m_proto->m_ThreadInfo->send(iq);

	m_wasPublished = TRUE;
}

void CPepService::Retract()
{
	TCHAR *tempName = mir_a2t(m_name);
	_tcslwr(tempName);

	m_proto->m_ThreadInfo->send(
		XmlNodeIq(_T("set"), m_proto->SerialNext())
			<< XCHILDNS(_T("pubsub"), JABBER_FEAT_PUBSUB)
				<< XCHILD(_T("publish")) << XATTR(_T("node"), m_node)
					<< XCHILD(_T("item"))
						<< XCHILDNS(tempName, m_node));

	mir_free(tempName);
}

void CPepService::ResetPublish()
{
	m_wasPublished = FALSE;
}

void CPepService::ForceRepublishOnLogin()
{
	if (!m_wasPublished)
		Publish();
}

///////////////////////////////////////////////////////////////////////////////
// CPepGuiService base class

CPepGuiService::CPepGuiService(CJabberProto *proto, char *name, TCHAR *node):
	CPepService(proto, name, node),
	m_bGuiOpen(false),
	m_hIcolibItem(NULL),
	m_szText(NULL),
	m_hMenuService(NULL)
{
}

CPepGuiService::~CPepGuiService()
{
	if (m_hMenuService) {
		DestroyServiceFunction(m_hMenuService);
		m_hMenuService = NULL;
	}

	if (m_szText) mir_free(m_szText);
}

void CPepGuiService::InitGui()
{
	char szService[128];
	mir_snprintf(szService, SIZEOF(szService), "%s/AdvStatusSet/%s", m_proto->m_szModuleName, m_name);

	int(__cdecl CPepGuiService::*serviceProc)(WPARAM, LPARAM);
	serviceProc = &CPepGuiService::OnMenuItemClick;
	m_hMenuService = CreateServiceFunctionObj(szService, (MIRANDASERVICEOBJ)*(void **)&serviceProc, this);

	RebuildMenu();
}

void CPepGuiService::RebuildMenu()
{
	HGENMENU hJabberRoot = MO_GetProtoRootMenu(m_proto->m_szModuleName);
	if (hJabberRoot == NULL)
		return;

	char szService[128];
	mir_snprintf(szService, SIZEOF(szService), "%s/AdvStatusSet/%s", m_proto->m_szModuleName, m_name);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.hParentMenu = hJabberRoot;
	mi.pszService = szService;
	mi.position = 200010;
	mi.flags = CMIF_TCHAR | CMIF_HIDDEN | CMIF_ROOTHANDLE;
	mi.icolibItem = m_hIcolibItem;
	mi.ptszName = m_szText ? m_szText : _T("<advanced status slot>");
	m_hMenuItem = Menu_AddProtoMenuItem(&mi);
}

bool CPepGuiService::LaunchSetGui(BYTE bQuiet)
{
	if (m_bGuiOpen) return false;

	m_bGuiOpen = true;
	ShowSetDialog(bQuiet);
	m_bGuiOpen = false;

	return true;
}

void CPepGuiService::UpdateMenuItem(HANDLE hIcolibIcon, TCHAR *text)
{
	m_hIcolibItem = hIcolibIcon;
	if (m_szText) mir_free(m_szText);
	m_szText = text ? mir_tstrdup(text) : NULL;

	if (!m_hMenuItem) return;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR | CMIM_ICON | CMIM_NAME;
	mi.icolibItem = m_hIcolibItem;
	mi.ptszName = m_szText ? m_szText : _T("<advanced status slot>");
	Menu_ModifyItem(m_hMenuItem, &mi);
}

int CPepGuiService::OnMenuItemClick(WPARAM, LPARAM)
{
	LaunchSetGui(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CPepMood

struct
{
	TCHAR	*szName;
	char* szTag;
}
static g_arrMoods[] =
{
	{ LPGENT("None"),         NULL            },
	{ LPGENT("Afraid"),       "afraid"        },
	{ LPGENT("Amazed"),       "amazed"        },
	{ LPGENT("Amorous"),      "amorous"       },
	{ LPGENT("Angry"),        "angry"         },
	{ LPGENT("Annoyed"),      "annoyed"       },
	{ LPGENT("Anxious"),      "anxious"       },
	{ LPGENT("Aroused"),      "aroused"       },
	{ LPGENT("Ashamed"),      "ashamed"       },
	{ LPGENT("Bored"),        "bored"         },
	{ LPGENT("Brave"),        "brave"         },
	{ LPGENT("Calm"),         "calm"          },
	{ LPGENT("Cautious"),     "cautious"      },
	{ LPGENT("Cold"),         "cold"          },
	{ LPGENT("Confident"),    "confident"     },
	{ LPGENT("Confused"),     "confused"      },
	{ LPGENT("Contemplative"),"contemplative" },
	{ LPGENT("Contented"),    "contented"     },
	{ LPGENT("Cranky"),       "cranky"        },
	{ LPGENT("Crazy"),        "crazy"         },
	{ LPGENT("Creative"),     "creative"      },
	{ LPGENT("Curious"),      "curious"       },
	{ LPGENT("Dejected"),     "dejected"      },
	{ LPGENT("Depressed"),    "depressed"     },
	{ LPGENT("Disappointed"), "disappointed"  },
	{ LPGENT("Disgusted"),    "disgusted"     },
	{ LPGENT("Dismayed"),     "dismayed"      },
	{ LPGENT("Distracted"),   "distracted"    },
	{ LPGENT("Embarrassed"),  "embarrassed"   },
	{ LPGENT("Envious"),      "envious"       },
	{ LPGENT("Excited"),      "excited"       },
	{ LPGENT("Flirtatious"),  "flirtatious"   },
	{ LPGENT("Frustrated"),   "frustrated"    },
	{ LPGENT("Grateful"),     "grateful"      },
	{ LPGENT("Grieving"),     "grieving"      },
	{ LPGENT("Grumpy"),       "grumpy"        },
	{ LPGENT("Guilty"),       "guilty"        },
	{ LPGENT("Happy"),        "happy"         },
	{ LPGENT("Hopeful"),      "hopeful"       },
	{ LPGENT("Hot"),          "hot"           },
	{ LPGENT("Humbled"),      "humbled"       },
	{ LPGENT("Humiliated"),   "humiliated"    },
	{ LPGENT("Hungry"),       "hungry"        },
	{ LPGENT("Hurt"),         "hurt"          },
	{ LPGENT("Impressed"),    "impressed"     },
	{ LPGENT("In awe"),       "in_awe"        },
	{ LPGENT("In love"),      "in_love"       },
	{ LPGENT("Indignant"),    "indignant"     },
	{ LPGENT("Interested"),   "interested"    },
	{ LPGENT("Intoxicated"),  "intoxicated"   },
	{ LPGENT("Invincible"),   "invincible"    },
	{ LPGENT("Jealous"),      "jealous"       },
	{ LPGENT("Lonely"),       "lonely"        },
	{ LPGENT("Lost"),         "lost"          },
	{ LPGENT("Lucky"),        "lucky"         },
	{ LPGENT("Mean"),         "mean"          },
	{ LPGENT("Moody"),        "moody"         },
	{ LPGENT("Nervous"),      "nervous"       },
	{ LPGENT("Neutral"),      "neutral"       },
	{ LPGENT("Offended"),     "offended"      },
	{ LPGENT("Outraged"),     "outraged"      },
	{ LPGENT("Playful"),      "playful"       },
	{ LPGENT("Proud"),        "proud"         },
	{ LPGENT("Relaxed"),      "relaxed"       },
	{ LPGENT("Relieved"),     "relieved"      },
	{ LPGENT("Remorseful"),   "remorseful"    },
	{ LPGENT("Restless"),     "restless"      },
	{ LPGENT("Sad"),          "sad"           },
	{ LPGENT("Sarcastic"),    "sarcastic"     },
	{ LPGENT("Satisfied"),    "satisfied"     },
	{ LPGENT("Serious"),      "serious"       },
	{ LPGENT("Shocked"),      "shocked"       },
	{ LPGENT("Shy"),          "shy"           },
	{ LPGENT("Sick"),         "sick"          },
	{ LPGENT("Sleepy"),       "sleepy"        },
	{ LPGENT("Spontaneous"),  "spontaneous"   },
	{ LPGENT("Stressed"),     "stressed"      },
	{ LPGENT("Strong"),       "strong"        },
	{ LPGENT("Surprised"),    "surprised"     },
	{ LPGENT("Thankful"),     "thankful"      },
	{ LPGENT("Thirsty"),      "thirsty"       },
	{ LPGENT("Tired"),        "tired"         },
	{ LPGENT("Undefined"),    "undefined"     },
	{ LPGENT("Weak"),         "weak"          },
	{ LPGENT("Worried"),      "worried"       },
};

CPepMood::CPepMood(CJabberProto *proto) :
	CPepGuiService(proto, "Mood", JABBER_FEAT_USER_MOOD),
	m_text(NULL),
	m_mode(-1)
{
	UpdateMenuItem(LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), LPGENT("Set mood..."));
}

CPepMood::~CPepMood()
{
	mir_free(m_text);
}

void CPepMood::ProcessItems(const TCHAR *from, HXML itemsNode)
{
	MCONTACT hContact = NULL, hSelfContact = NULL;
	if (!m_proto->IsMyOwnJID(from)) {
		hContact = m_proto->HContactFromJID(from);
		if (!hContact) return;
	}
	else hSelfContact = m_proto->HContactFromJID(from);

	if (xmlGetChild(itemsNode, _T("retract"))) {
		if (hSelfContact)
			SetMood(hSelfContact, NULL, NULL);
		SetMood(hContact, NULL, NULL);
		return;
	}

	HXML n, moodNode = XPath(itemsNode, _T("item/mood[@xmlns='") JABBER_FEAT_USER_MOOD _T("']"));
	if (!moodNode) return;

	LPCTSTR moodType = NULL, moodText = NULL;
	for (int i=0; n = xmlGetChild(moodNode, i); i++) {
		if (!_tcscmp(xmlGetName(n), _T("text")))
			moodText = xmlGetText(n);
		else
			moodType = xmlGetName(n);
	}

	TCHAR *fixedText = JabberStrFixLines(moodText);
	if (hSelfContact)
		SetMood(hSelfContact, moodType, fixedText);
	SetMood(hContact, moodType, fixedText);
	mir_free(fixedText);

	if (!hContact && m_mode >= 0)
		ForceRepublishOnLogin();
}

void CPepMood::CreateData(HXML n)
{
	HXML moodNode = n << XCHILDNS(_T("mood"), JABBER_FEAT_USER_MOOD);
	moodNode << XCHILD(_A2T(g_arrMoods[m_mode].szTag));
	if (m_text)
		moodNode << XCHILD(_T("text"), m_text);
}

void CPepMood::ResetExtraIcon(MCONTACT hContact)
{
	char *szMood = m_proto->ReadAdvStatusA(hContact, ADVSTATUS_MOOD, "id");
	SetExtraIcon(hContact, szMood);
	mir_free(szMood);
}

void CPepMood::SetExtraIcon(MCONTACT hContact, char *szMood)
{
	ExtraIcon_SetIcon(hExtraMood, hContact, szMood == NULL ? NULL : g_MoodIcons.GetIcolibHandle(szMood));
}

void CPepMood::SetMood(MCONTACT hContact, const TCHAR *szMood, const TCHAR *szText)
{
	int mood = -1;
	if (szMood) {
		char* p = mir_t2a(szMood);

		for (int i = 1; i < SIZEOF(g_arrMoods); i++)
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
		replaceStrT(m_text, szText);

		HANDLE hIcon = (mood >= 0) ? g_MoodIcons.GetIcolibHandle(g_arrMoods[mood].szTag) : LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT);
		TCHAR title[128];

		if (m_proto->m_pInfoFrame) {
			if (mood >= 0) {
				mir_sntprintf(title, SIZEOF(title), TranslateT("Mood: %s"), TranslateTS(g_arrMoods[mood].szName));
				m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/mood", g_MoodIcons.GetIcolibHandle(g_arrMoods[mood].szTag), TranslateTS(g_arrMoods[mood].szName));
			}
			else {
				mir_tstrcpy(title, LPGENT("Set mood..."));
				m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/mood", LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), TranslateT("Set mood..."));
			}
		}

		UpdateMenuItem(hIcon, title);
	}
	else SetExtraIcon(hContact, mood < 0 ? NULL : g_arrMoods[mood].szTag);

	if (szMood) {
		m_proto->setByte(hContact, DBSETTING_XSTATUSID, mood);
		m_proto->setTString(hContact, DBSETTING_XSTATUSNAME, TranslateTS(g_arrMoods[mood].szName));
		if (szText)
			m_proto->setTString(hContact, DBSETTING_XSTATUSMSG, szText);
		else
			m_proto->delSetting(hContact, DBSETTING_XSTATUSMSG);

		m_proto->WriteAdvStatus(hContact, ADVSTATUS_MOOD, szMood, g_MoodIcons.GetIcolibName(g_arrMoods[mood].szTag), TranslateTS(g_arrMoods[mood].szName), szText);
	}
	else {
		m_proto->delSetting(hContact, DBSETTING_XSTATUSID);
		m_proto->delSetting(hContact, DBSETTING_XSTATUSNAME);
		m_proto->delSetting(hContact, DBSETTING_XSTATUSMSG);

		m_proto->ResetAdvStatus(hContact, ADVSTATUS_MOOD);
	}

	NotifyEventHooks(m_proto->m_hEventXStatusChanged, hContact, 0);
}

void CPepMood::ShowSetDialog(BYTE bQuiet)
{
	if (!bQuiet) {
		CJabberDlgPepSimple dlg(m_proto, TranslateT("Set Mood"));
		for (int i = 1; i < SIZEOF(g_arrMoods); i++)
			dlg.AddStatusMode(i, g_arrMoods[i].szTag, g_MoodIcons.GetIcon(g_arrMoods[i].szTag), TranslateTS(g_arrMoods[i].szName));

		dlg.SetActiveStatus(m_mode, m_text);
		dlg.DoModal();
		if (!dlg.OkClicked())
			return;

		m_mode = dlg.GetStatusMode();
		replaceStrT(m_text, dlg.GetStatusText());
	}

	if (m_proto->m_pInfoFrame) {
		if (m_mode >= 0) {
			Publish();

			UpdateMenuItem(g_MoodIcons.GetIcolibHandle(g_arrMoods[m_mode].szTag), g_arrMoods[m_mode].szName);
			m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/mood", g_MoodIcons.GetIcolibHandle(g_arrMoods[m_mode].szTag), TranslateTS(g_arrMoods[m_mode].szName));
		}
		else {
			Retract();
			UpdateMenuItem(LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), LPGENT("Set mood..."));
			m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/mood", LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), TranslateT("Set mood..."));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// CPepActivity

#define ACTIVITY_ICON(section, item)	-(300 + (section) * 20 + (item))

struct
{
	char *szFirst;
	char *szSecond;
	TCHAR *szTitle;
	int iconid;
}
static g_arrActivities[] =
{
	{ "doing_chores", NULL,       LPGENT("Doing chores"),       ACTIVITY_ICON(0,  0) },
	{ NULL, "buying_groceries",   LPGENT("buying groceries"),   ACTIVITY_ICON(0,  1) },
	{ NULL, "cleaning",           LPGENT("cleaning"),           ACTIVITY_ICON(0,  2) },
	{ NULL, "cooking",            LPGENT("cooking"),            ACTIVITY_ICON(0,  3) },
	{ NULL, "doing_maintenance",  LPGENT("doing maintenance"),  ACTIVITY_ICON(0,  4) },
	{ NULL, "doing_the_dishes",   LPGENT("doing the dishes"),   ACTIVITY_ICON(0,  5) },
	{ NULL, "doing_the_laundry",  LPGENT("doing the laundry"),  ACTIVITY_ICON(0,  6) },
	{ NULL, "gardening",          LPGENT("gardening"),          ACTIVITY_ICON(0,  7) },
	{ NULL, "running_an_errand",  LPGENT("running an errand"),  ACTIVITY_ICON(0,  8) },
	{ NULL, "walking_the_dog",    LPGENT("walking the dog"),    ACTIVITY_ICON(0,  9) },
	{ "drinking", NULL,           LPGENT("Drinking"),           ACTIVITY_ICON(1,  0) },
	{ NULL, "having_a_beer",      LPGENT("having a beer"),      ACTIVITY_ICON(1,  1) },
	{ NULL, "having_coffee",      LPGENT("having coffee"),      ACTIVITY_ICON(1,  2) },
	{ NULL, "having_tea",         LPGENT("having tea"),         ACTIVITY_ICON(1,  3) },
	{ "eating", NULL,             LPGENT("Eating"),             ACTIVITY_ICON(2,  0) },
	{ NULL, "having_a_snack",     LPGENT("having a snack"),     ACTIVITY_ICON(2,  1) },
	{ NULL, "having_breakfast",   LPGENT("having breakfast"),   ACTIVITY_ICON(2,  2) },
	{ NULL, "having_dinner",      LPGENT("having dinner"),      ACTIVITY_ICON(2,  3) },
	{ NULL, "having_lunch",       LPGENT("having lunch"),       ACTIVITY_ICON(2,  4) },
	{ "exercising", NULL,         LPGENT("Exercising"),         ACTIVITY_ICON(3,  0) },
	{ NULL, "cycling",            LPGENT("cycling"),            ACTIVITY_ICON(3,  1) },
	{ NULL, "dancing",            LPGENT("dancing"),            ACTIVITY_ICON(3,  2) },
	{ NULL, "hiking",             LPGENT("hiking"),             ACTIVITY_ICON(3,  3) },
	{ NULL, "jogging",            LPGENT("jogging"),            ACTIVITY_ICON(3,  4) },
	{ NULL, "playing_sports",     LPGENT("playing sports"),     ACTIVITY_ICON(3,  5) },
	{ NULL, "running",            LPGENT("running"),            ACTIVITY_ICON(3,  6) },
	{ NULL, "skiing",             LPGENT("skiing"),             ACTIVITY_ICON(3,  7) },
	{ NULL, "swimming",           LPGENT("swimming"),           ACTIVITY_ICON(3,  8) },
	{ NULL, "working_out",        LPGENT("working out"),        ACTIVITY_ICON(3,  9) },
	{ "grooming", NULL,           LPGENT("Grooming"),           ACTIVITY_ICON(4,  0) },
	{ NULL, "at_the_spa",         LPGENT("at the spa"),         ACTIVITY_ICON(4,  1) },
	{ NULL, "brushing_teeth",     LPGENT("brushing teeth"),     ACTIVITY_ICON(4,  2) },
	{ NULL, "getting_a_haircut",  LPGENT("getting a haircut"),  ACTIVITY_ICON(4,  3) },
	{ NULL, "shaving",            LPGENT("shaving"),            ACTIVITY_ICON(4,  4) },
	{ NULL, "taking_a_bath",      LPGENT("taking a bath"),      ACTIVITY_ICON(4,  5) },
	{ NULL, "taking_a_shower",    LPGENT("taking a shower"),    ACTIVITY_ICON(4,  6) },
	{ "having_appointment", NULL, LPGENT("Having appointment"), ACTIVITY_ICON(5,  0) },
	{ "inactive", NULL,           LPGENT("Inactive"),           ACTIVITY_ICON(6,  0) },
	{ NULL, "day_off",            LPGENT("day off"),            ACTIVITY_ICON(6,  1) },
	{ NULL, "hanging_out",        LPGENT("hanging out"),        ACTIVITY_ICON(6,  2) },
	{ NULL, "hiding",             LPGENT("hiding"),             ACTIVITY_ICON(6,  3) },
	{ NULL, "on_vacation",        LPGENT("on vacation"),        ACTIVITY_ICON(6,  4) },
	{ NULL, "praying",            LPGENT("praying"),            ACTIVITY_ICON(6,  5) },
	{ NULL, "scheduled_holiday",  LPGENT("scheduled holiday"),  ACTIVITY_ICON(6,  6) },
	{ NULL, "sleeping",           LPGENT("sleeping"),           ACTIVITY_ICON(6,  7) },
	{ NULL, "thinking",           LPGENT("thinking"),           ACTIVITY_ICON(6,  8) },
	{ "relaxing", NULL,           LPGENT("Relaxing"),           ACTIVITY_ICON(7,  0) },
	{ NULL, "fishing",            LPGENT("fishing"),            ACTIVITY_ICON(7,  1) },
	{ NULL, "gaming",             LPGENT("gaming"),             ACTIVITY_ICON(7,  2) },
	{ NULL, "going_out",          LPGENT("going out"),          ACTIVITY_ICON(7,  3) },
	{ NULL, "partying",           LPGENT("partying"),           ACTIVITY_ICON(7,  4) },
	{ NULL, "reading",            LPGENT("reading"),            ACTIVITY_ICON(7,  5) },
	{ NULL, "rehearsing",         LPGENT("rehearsing"),         ACTIVITY_ICON(7,  6) },
	{ NULL, "shopping",           LPGENT("shopping"),           ACTIVITY_ICON(7,  7) },
	{ NULL, "smoking",            LPGENT("smoking"),            ACTIVITY_ICON(7,  8) },
	{ NULL, "socializing",        LPGENT("socializing"),        ACTIVITY_ICON(7,  9) },
	{ NULL, "sunbathing",         LPGENT("sunbathing"),         ACTIVITY_ICON(7,  10) },
	{ NULL, "watching_tv",        LPGENT("watching TV"),        ACTIVITY_ICON(7,  11) },
	{ NULL, "watching_a_movie",   LPGENT("watching a movie"),   ACTIVITY_ICON(7,  12) },
	{ "talking", NULL,            LPGENT("Talking"),            ACTIVITY_ICON(8,  0) },
	{ NULL, "in_real_life",       LPGENT("in real life"),       ACTIVITY_ICON(8,  1) },
	{ NULL, "on_the_phone",       LPGENT("on the phone"),       ACTIVITY_ICON(8,  2) },
	{ NULL, "on_video_phone",     LPGENT("on video phone"),     ACTIVITY_ICON(8,  3) },
	{ "traveling", NULL,          LPGENT("Traveling"),          ACTIVITY_ICON(9,  0) },
	{ NULL, "commuting",          LPGENT("commuting"),          ACTIVITY_ICON(9,  1) },
	{ NULL, "cycling",            LPGENT("cycling"),            ACTIVITY_ICON(9,  2) },
	{ NULL, "driving",            LPGENT("driving"),            ACTIVITY_ICON(9,  3) },
	{ NULL, "in_a_car",           LPGENT("in a car"),           ACTIVITY_ICON(9,  4) },
	{ NULL, "on_a_bus",           LPGENT("on a bus"),           ACTIVITY_ICON(9,  5) },
	{ NULL, "on_a_plane",         LPGENT("on a plane"),         ACTIVITY_ICON(9,  6) },
	{ NULL, "on_a_train",         LPGENT("on a train"),         ACTIVITY_ICON(9,  7) },
	{ NULL, "on_a_trip",          LPGENT("on a trip"),          ACTIVITY_ICON(9,  8) },
	{ NULL, "walking",            LPGENT("walking"),            ACTIVITY_ICON(9,  9) },
	{ "working", NULL,            LPGENT("Working"),            ACTIVITY_ICON(10,  0) },
	{ NULL, "coding",             LPGENT("coding"),             ACTIVITY_ICON(10,  1) },
	{ NULL, "in_a_meeting",       LPGENT("in a meeting"),       ACTIVITY_ICON(10,  2) },
	{ NULL, "studying",           LPGENT("studying"),           ACTIVITY_ICON(10,  3) },
	{ NULL, "writing",            LPGENT("writing"),            ACTIVITY_ICON(10,  4) },
	{ NULL, NULL, NULL } // the end, don't delete this
};

inline char *ActivityGetId(int id)
{
	return g_arrActivities[id].szSecond ? g_arrActivities[id].szSecond : g_arrActivities[id].szFirst;
}

// -1 if not found, otherwise activity number
static int ActivityCheck(LPCTSTR szFirstNode, LPCTSTR szSecondNode)
{
	if (!szFirstNode) return 0;

	char *s1 = mir_t2a(szFirstNode), *s2 = mir_t2a(szSecondNode);

	int i = 0, nFirst = -1, nSecond = -1;
	while (g_arrActivities[i].szFirst || g_arrActivities[i].szSecond) {
		// check first node
		if (g_arrActivities[i].szFirst && !strcmp(s1, g_arrActivities[i].szFirst)) {
			// first part found
			nFirst = i;
			if (!s2) {
				nSecond = i;
				break;
			}
			i++; // move to next
			while (g_arrActivities[i].szSecond) {
				if (!strcmp(g_arrActivities[i].szSecond, s2)) {
					nSecond = i;
					break;
				}
				i++;
			}
			break;
		}
		i++;
	}

	mir_free(s1);
	mir_free(s2);

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
	return NULL;
}

char* ActivityGetFirst(int id)
{
	if (id >= SIZEOF(g_arrActivities) - 1)
		return NULL;

	while (id >= 0) {
		if (g_arrActivities[id].szFirst)
			return g_arrActivities[id].szFirst;
		--id;
	}

	return NULL;
}

char *ActivityGetFirst(char *szId)
{
	if (!szId) return NULL;

	int id = SIZEOF(g_arrActivities) - 1;
	bool found_second = false;

	while (id >= 0) {
		if (g_arrActivities[id].szFirst && (found_second || !mir_strcmp(g_arrActivities[id].szFirst, szId)))
			return g_arrActivities[id].szFirst;
		if (g_arrActivities[id].szSecond && !found_second && !mir_strcmp(g_arrActivities[id].szSecond, szId))
			found_second = true;
		--id;
	}

	return NULL;
}

char *ActivityGetSecond(int id)
{
	return (id >= 0) ? g_arrActivities[id].szSecond : NULL;
}

TCHAR *ActivityGetFirstTitle(int id)
{
	if (id >= SIZEOF(g_arrActivities) - 1)
		return NULL;

	while (id >= 0) {
		if (g_arrActivities[id].szFirst)
			return g_arrActivities[id].szTitle;
		--id;
	}

	return NULL;
}

TCHAR *ActivityGetSecondTitle(int id)
{
	return ((id >= 0) && g_arrActivities[id].szSecond) ? g_arrActivities[id].szTitle : NULL;
}

void ActivityBuildTitle(int id, TCHAR *buf, int size)
{
	TCHAR *szFirst = ActivityGetFirstTitle(id);
	TCHAR *szSecond = ActivityGetSecondTitle(id);

	if (szFirst) {
		if (szSecond)
			mir_sntprintf(buf, size, _T("%s [%s]"), TranslateTS(szFirst), TranslateTS(szSecond));
		else
			mir_tstrncpy(buf, TranslateTS(szFirst), size);
	}
	else *buf = 0;
}

CPepActivity::CPepActivity(CJabberProto *proto):
	CPepGuiService(proto, "Activity", JABBER_FEAT_USER_ACTIVITY),
	m_text(NULL),
	m_mode(-1)
{
	UpdateMenuItem(LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), LPGENT("Set activity..."));
}

CPepActivity::~CPepActivity()
{
	mir_free(m_text);
}

void CPepActivity::ProcessItems(const TCHAR *from, HXML itemsNode)
{
	MCONTACT hContact = NULL, hSelfContact = NULL;
	if (!m_proto->IsMyOwnJID(from)) {
		hContact = m_proto->HContactFromJID(from);
		if (!hContact) return;
	}
	else hSelfContact = m_proto->HContactFromJID(from);

	if (xmlGetChild(itemsNode, "retract")) {
		if (hSelfContact)
			SetActivity(hSelfContact, NULL, NULL, NULL);
		SetActivity(hContact, NULL, NULL, NULL);
		return;
	}

	HXML actNode = XPath(itemsNode, _T("item/activity[@xmlns='") JABBER_FEAT_USER_ACTIVITY _T("']"));
	if (!actNode)
		return;

	LPCTSTR szText = XPathT(actNode, "text");
	LPCTSTR szFirstNode = NULL, szSecondNode = NULL;

	HXML n;
	for (int i=0; n = xmlGetChild(actNode, i); i++) {
		if (mir_tstrcmp(xmlGetName(n), _T("text"))) {
			szFirstNode = xmlGetName(n);
			HXML secondNode = xmlGetChild(n, 0);
			if (szFirstNode && secondNode && xmlGetName(secondNode))
				szSecondNode = xmlGetName(secondNode);
			break;
		}
	}

	TCHAR *fixedText = JabberStrFixLines(szText);
	if (hSelfContact)
		SetActivity(hSelfContact, szFirstNode, szSecondNode, fixedText);
	SetActivity(hContact, szFirstNode, szSecondNode, fixedText);
	mir_free(fixedText);

	if (!hContact && m_mode >= 0)
		ForceRepublishOnLogin();
}

void CPepActivity::CreateData(HXML n)
{
	char *szFirstNode = ActivityGetFirst(m_mode);
	char *szSecondNode = ActivityGetSecond(m_mode);

	HXML activityNode = n << XCHILDNS(_T("activity"), JABBER_FEAT_USER_ACTIVITY);
	HXML firstNode = activityNode << XCHILD(_A2T(szFirstNode));

	if (firstNode && szSecondNode)
		firstNode << XCHILD(_A2T(szSecondNode));

	if (m_text)
		activityNode << XCHILD(_T("text"), m_text);
}

void CPepActivity::ResetExtraIcon(MCONTACT hContact)
{
	char *szActivity = m_proto->ReadAdvStatusA(hContact, ADVSTATUS_ACTIVITY, "id");
	SetExtraIcon(hContact, szActivity);
	mir_free(szActivity);
}

void CPepActivity::SetExtraIcon(MCONTACT hContact, char *szActivity)
{
	ExtraIcon_SetIcon(hExtraActivity, hContact, szActivity == NULL ? NULL : g_ActivityIcons.GetIcolibHandle(szActivity));
}

void CPepActivity::SetActivity(MCONTACT hContact, LPCTSTR szFirst, LPCTSTR szSecond, LPCTSTR szText)
{
	int activity = -1;
	if (szFirst || szSecond) {
		activity = ActivityCheck(szFirst, szSecond);
		if (activity < 0)
			return;
	}

	TCHAR activityTitle[128];
	ActivityBuildTitle(activity, activityTitle, SIZEOF(activityTitle));

	if (!hContact) {
		m_mode = activity;
		replaceStrT(m_text, szText);

		HANDLE hIcon = (activity >= 0) ? g_ActivityIcons.GetIcolibHandle(returnActivity(activity)) : LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT);
		TCHAR title[128];

		if (m_proto->m_pInfoFrame) {
			if (activity >= 0) {
				mir_sntprintf(title, SIZEOF(title), TranslateT("Activity: %s"), activityTitle);
				m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/activity", g_ActivityIcons.GetIcolibHandle(returnActivity(activity)), activityTitle);
			}
			else {
				mir_tstrcpy(title, LPGENT("Set activity..."));
				m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/activity", LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), TranslateT("Set activity..."));
			}
		}

		UpdateMenuItem(hIcon, title);
	}
	else SetExtraIcon(hContact, activity < 0 ? NULL : returnActivity(activity));

	if (activity >= 0) {
		TCHAR *p = mir_a2t(ActivityGetId(activity));
		m_proto->WriteAdvStatus(hContact, ADVSTATUS_ACTIVITY, p, g_ActivityIcons.GetIcolibName(returnActivity(activity)), activityTitle, szText);
		mir_free(p);
	}
	else m_proto->ResetAdvStatus(hContact, ADVSTATUS_ACTIVITY);
}

void CPepActivity::ShowSetDialog(BYTE)
{
	CJabberDlgPepSimple dlg(m_proto, TranslateT("Set Activity"));
	for (int i=0; i < SIZEOF(g_arrActivities); i++)
		if (g_arrActivities[i].szFirst || g_arrActivities[i].szSecond)
			dlg.AddStatusMode(i, ActivityGetId(i), g_ActivityIcons.GetIcon(returnActivity(i)), TranslateTS(g_arrActivities[i].szTitle), (g_arrActivities[i].szSecond != NULL));

	dlg.SetActiveStatus(m_mode, m_text);
	dlg.DoModal();

	if (!dlg.OkClicked()) return;

	m_mode = dlg.GetStatusMode();
	if (m_mode >= 0) {
		replaceStrT(m_text, dlg.GetStatusText());
		Publish();

		UpdateMenuItem(g_ActivityIcons.GetIcolibHandle(returnActivity(m_mode)), g_arrActivities[m_mode].szTitle);
		if (m_proto->m_pInfoFrame)
			m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/activity", g_ActivityIcons.GetIcolibHandle(returnActivity(m_mode)), TranslateTS(g_arrActivities[m_mode].szTitle));
	}
	else {
		Retract();
		UpdateMenuItem(LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), LPGENT("Set activity..."));
		if (m_proto->m_pInfoFrame)
			m_proto->m_pInfoFrame->UpdateInfoItem("$/PEP/activity", LoadSkinnedIconHandle(SKINICON_OTHER_SMALLDOT), TranslateT("Set activity..."));
	}
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

	if (wParam < 1 || wParam >= SIZEOF(g_arrMoods))
		return 0;

	int flags = 0;
	if (lParam & LR_SHARED)  flags |= LR_SHARED;
	if (lParam & LR_BIGICON) flags |= LR_BIGICON;

	return (INT_PTR)GetXStatusIcon(wParam, flags);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SendPepMood - sends mood

BOOL CJabberProto::SendPepTune(TCHAR* szArtist, TCHAR* szLength, TCHAR* szSource, TCHAR* szTitle, TCHAR* szTrack, TCHAR* szUri)
{
	if (!m_bJabberOnline || !m_bPepSupported)
		return FALSE;

	XmlNodeIq iq(_T("set"), SerialNext());
	HXML tuneNode = iq << XCHILDNS(_T("pubsub"), JABBER_FEAT_PUBSUB)
							<< XCHILD(_T("publish")) << XATTR(_T("node"), JABBER_FEAT_USER_TUNE)
							<< XCHILD(_T("item")) << XCHILDNS(_T("tune"), JABBER_FEAT_USER_TUNE);

	if (szArtist || szLength || szSource || szTitle || szUri) {
		if (szArtist) tuneNode << XCHILD(_T("artist"), szArtist);
		if (szLength) tuneNode << XCHILD(_T("length"), szLength);
		if (szSource) tuneNode << XCHILD(_T("source"), szSource);
		if (szTitle) tuneNode << XCHILD(_T("title"), szTitle);
		if (szTrack) tuneNode << XCHILD(_T("track"), szTrack);
		if (szUri) tuneNode << XCHILD(_T("uri"), szUri);
	}
	m_ThreadInfo->send(iq);

	return TRUE;
}

void CJabberProto::SetContactTune(MCONTACT hContact, LPCTSTR szArtist, LPCTSTR szLength, LPCTSTR szSource, LPCTSTR szTitle, LPCTSTR szTrack)
{
	if (!szArtist && !szTitle) {
		delSetting(hContact, "ListeningTo");
		ResetAdvStatus(hContact, ADVSTATUS_TUNE);
		return;
	}

	TCHAR *szListeningTo;
	if (ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT)) {
		LISTENINGTOINFO li;
		memset(&li, 0, sizeof(li));
		li.cbSize = sizeof(li);
		li.dwFlags = LTI_TCHAR;
		li.ptszArtist = (TCHAR*)szArtist;
		li.ptszLength = (TCHAR*)szLength;
		li.ptszAlbum = (TCHAR*)szSource;
		li.ptszTitle = (TCHAR*)szTitle;
		li.ptszTrack = (TCHAR*)szTrack;
		szListeningTo = (TCHAR *)CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM)_T("%title% - %artist%"), (LPARAM)&li);
	}
	else {
		szListeningTo = (TCHAR *) mir_alloc(2048 * sizeof(TCHAR));
		mir_sntprintf(szListeningTo, 2047, _T("%s - %s"), szTitle ? szTitle : _T(""), szArtist ? szArtist : _T(""));
	}

	setTString(hContact, "ListeningTo", szListeningTo);

	char tuneIcon[128];
	mir_snprintf(tuneIcon, SIZEOF(tuneIcon), "%s_%s", m_szModuleName, "main");
	WriteAdvStatus(hContact, ADVSTATUS_TUNE, _T("listening_to"), tuneIcon, TranslateT("Listening To"), szListeningTo);

	mir_free(szListeningTo);
}

TCHAR* a2tf(const TCHAR *str, BOOL unicode)
{
	if (str == NULL)
		return NULL;

	return (unicode) ? mir_tstrdup(str) : mir_a2t((char*)str);
}

void overrideStr(TCHAR*& dest, const TCHAR *src, BOOL unicode, const TCHAR *def = NULL)
{
	if (dest != NULL) {
		mir_free(dest);
		dest = NULL;
	}

	if (src != NULL)
		dest = a2tf(src, unicode);
	else if (def != NULL)
		dest = mir_tstrdup(def);
}

INT_PTR __cdecl CJabberProto::OnSetListeningTo(WPARAM, LPARAM lParam)
{
	LISTENINGTOINFO *cm = (LISTENINGTOINFO *)lParam;
	if (!cm || cm->cbSize != sizeof(LISTENINGTOINFO)) {
		SendPepTune(NULL, NULL, NULL, NULL, NULL, NULL);
		delSetting("ListeningTo");
	}
	else {
		TCHAR *szArtist = NULL, *szLength = NULL, *szSource = NULL;
		TCHAR *szTitle = NULL, *szTrack = NULL;

		BOOL unicode = cm->dwFlags & LTI_UNICODE;

		overrideStr(szArtist, cm->ptszArtist, unicode);
		overrideStr(szSource, cm->ptszAlbum, unicode);
		overrideStr(szTitle, cm->ptszTitle, unicode);
		overrideStr(szTrack, cm->ptszTrack, unicode);
		overrideStr(szLength, cm->ptszLength, unicode);

		TCHAR szLengthInSec[ 32 ];
		szLengthInSec[ 0 ] = 0;
		if (szLength) {
			unsigned int multiplier = 1, result = 0;
			for (TCHAR *p = szLength; *p; p++)
				if (*p == _T(':'))
					multiplier *= 60;

			if (multiplier <= 3600) {
				TCHAR *szTmp = szLength;
				while (szTmp[0]) {
					result += (_ttoi(szTmp) * multiplier);
					multiplier /= 60;
					szTmp = _tcschr(szTmp, _T(':'));
					if (!szTmp)
						break;
					szTmp++;
				}
			}
			mir_sntprintf(szLengthInSec, SIZEOF(szLengthInSec), _T("%d"), result);
		}

		SendPepTune(szArtist, szLength ? szLengthInSec : NULL, szSource, szTitle, szTrack, NULL);
		SetContactTune(NULL, szArtist, szLength, szSource, szTitle, szTrack);

		mir_free(szArtist);
		mir_free(szLength);
		mir_free(szSource);
		mir_free(szTitle);
		mir_free(szTrack);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// process InfoFrame clicks

void CJabberProto::InfoFrame_OnUserMood(CJabberInfoFrame_Event*)
{
	((CPepGuiService *)m_pepServices.Find(JABBER_FEAT_USER_MOOD))->LaunchSetGui(0);
}

void CJabberProto::InfoFrame_OnUserActivity(CJabberInfoFrame_Event*)
{
	((CPepGuiService *)m_pepServices.Find(JABBER_FEAT_USER_ACTIVITY))->LaunchSetGui(0);
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
	if (pepMood == NULL)
		return 1;

	// fill status member
	if (pData->flags & CSSF_MASK_STATUS)
		*pData->status = pepMood->m_mode;

	// fill status name member
	if (pData->flags & CSSF_MASK_NAME) {
		if (pData->flags & CSSF_DEFAULT_NAME) {
			DWORD dwXStatus = (pData->wParam == NULL) ? pepMood->m_mode : *pData->wParam;
			if (dwXStatus >= SIZEOF(g_arrMoods))
				return 1;

			if (pData->flags & CSSF_UNICODE)
				mir_wstrncpy(pData->pwszName, g_arrMoods[dwXStatus].szName, (STATUS_TITLE_MAX + 1));
			else {
				size_t dwStatusTitleSize = mir_wstrlen(g_arrMoods[dwXStatus].szName);
				if (dwStatusTitleSize > STATUS_TITLE_MAX)
					dwStatusTitleSize = STATUS_TITLE_MAX;

				WideCharToMultiByte(CP_ACP, 0, g_arrMoods[dwXStatus].szName, (DWORD)dwStatusTitleSize, pData->pszName, MAX_PATH, NULL, NULL);
				pData->pszName[dwStatusTitleSize] = 0;
			}
		}
		else {
			*pData->ptszName = 0;
			if (pData->flags & CSSF_UNICODE) {
				ptrT title(ReadAdvStatusT(hContact, ADVSTATUS_MOOD, ADVSTATUS_VAL_TITLE));
				if (title)
					_tcsncpy_s(pData->ptszName, STATUS_TITLE_MAX, title, _TRUNCATE);
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
			ptrT title(ReadAdvStatusT(hContact, ADVSTATUS_MOOD, ADVSTATUS_VAL_TEXT));
			if (title)
				_tcsncpy_s(pData->ptszMessage, STATUS_TITLE_MAX, title, _TRUNCATE);
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
			*pData->wParam = SIZEOF(g_arrMoods);

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
	if (status >= 0 && status < SIZEOF(g_arrMoods)) {
		pepMood->m_mode = status;
		pepMood->m_text = (pData->flags & CSSF_MASK_MESSAGE) ? JabberStrFixLines(pData->ptszMessage) : NULL;
		pepMood->LaunchSetGui(1);
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
	mir_snprintf(szSetting, SIZEOF(szSetting), "AdvStatus/%s/%s", m_szModuleName, pszSlot);
	db_set_resident(szSetting, "id");
	db_set_resident(szSetting, "icon");
	db_set_resident(szSetting, "title");
	db_set_resident(szSetting, "text");
}

void CJabberProto::ResetAdvStatus(MCONTACT hContact, const char *pszSlot)
{
	char szSetting[128];
	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/id", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/icon", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/title", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/text", m_szModuleName, pszSlot);
	db_unset(hContact, "AdvStatus", szSetting);
}

void CJabberProto::WriteAdvStatus(MCONTACT hContact, const char *pszSlot, const TCHAR *pszMode, const char *pszIcon, const TCHAR *pszTitle, const TCHAR *pszText)
{
	char szSetting[128];

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/id", m_szModuleName, pszSlot);
	db_set_ts(hContact, "AdvStatus", szSetting, pszMode);

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/icon", m_szModuleName, pszSlot);
	db_set_s(hContact, "AdvStatus", szSetting, pszIcon);

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/title", m_szModuleName, pszSlot);
	db_set_ts(hContact, "AdvStatus", szSetting, pszTitle);

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/text", m_szModuleName, pszSlot);
	if (pszText)
		db_set_ts(hContact, "AdvStatus", szSetting, pszText);
	else
		db_unset(hContact, "AdvStatus", szSetting);
}

char* CJabberProto::ReadAdvStatusA(MCONTACT hContact, const char *pszSlot, const char *pszValue)
{
	char szSetting[128];
	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/%s", m_szModuleName, pszSlot, pszValue);
	return db_get_sa(hContact, "AdvStatus", szSetting);
}

TCHAR* CJabberProto::ReadAdvStatusT(MCONTACT hContact, const char *pszSlot, const char *pszValue)
{
	char szSetting[128];
	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/%s", m_szModuleName, pszSlot, pszValue);
	return db_get_tsa(hContact, "AdvStatus", szSetting);
}

///////////////////////////////////////////////////////////////////////////////
// Global functions

void g_XstatusIconsInit()
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, SIZEOF(szFile));
	if (TCHAR *p = _tcsrchr(szFile, '\\'))
		_tcscpy(p + 1, _T("..\\Icons\\xstatus_jabber.dll"));

	TCHAR szSection[100];
	_tcscpy(szSection, _T("Protocols/Jabber/")LPGENT("Moods"));

	for (int i = 1; i < SIZEOF(g_arrMoods); i++)
		g_MoodIcons.RegisterIcon(g_arrMoods[i].szTag, szFile, -(200 + i), szSection, TranslateTS(g_arrMoods[i].szName));

	_tcscpy(szSection, _T("Protocols/Jabber/")LPGENT("Activities"));
	for (int k = 0; k < SIZEOF(g_arrActivities); k++) {
		if (g_arrActivities[k].szFirst)
			g_ActivityIcons.RegisterIcon(g_arrActivities[k].szFirst, szFile, g_arrActivities[k].iconid, szSection, TranslateTS(g_arrActivities[k].szTitle));
		if (g_arrActivities[k].szSecond)
			g_ActivityIcons.RegisterIcon(g_arrActivities[k].szSecond, szFile, g_arrActivities[k].iconid, szSection, TranslateTS(g_arrActivities[k].szTitle));
	}
}
