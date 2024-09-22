/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

struct
{
	const char *ss;
	const wchar_t *defStatus;
	int defIcon;
}
static moods[] =
{
	{ "",             LPGENW("None") },
	{ "",             LPGENW("Custom emoji") },
	{ "brb",          LPGENW("Be right back") },
	{ "burger",       LPGENW("Out for lunch") },
	{ "wait",         LPGENW("In meetings") },
	{ "learn",        LPGENW("At school") },
	{ "movie",        LPGENW("At the movies") },
	{ "plane",        LPGENW("Traveling") },
	{ "party",        LPGENW("Celebrating") },
	{ "car",          LPGENW("Driving") },
	{ "skip",         LPGENW("At the gym") },
	{ "wfh",          LPGENW("Working from home") },
};

struct SetStatusMsgRequest : public AsyncHttpRequest
{
	SetStatusMsgRequest(CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_POST, HOST_API, "/users/self/profile/partial")
	{
		int m_iMood = ppro->m_iMood;
		auto &pMood = moods[m_iMood];

		JSONNode node, payload;
		payload.set_name("payload");

		CMStringW s1, s2;
		switch (m_iMood) {
		case 0: // none
			s1 = ppro->m_wstrMoodMessage;
			break;
		case 1: // custom
			s1.Format(L"(%x) %s", Utf16toUtf32(ppro->m_wstrMoodEmoji), (wchar_t *)ppro->m_wstrMoodMessage);
			break;
		default:
			s1.Format(L"(%S) %s", pMood.ss, (wchar_t *)ppro->m_wstrMoodMessage);
			break;
		}
		payload << WCHAR_PARAM("mood", s1);

		if (m_iMood > 1)
			s2.Format(L"<ss type=\"%S\">(%S)</ss>%s", pMood.ss, pMood.ss, (wchar_t*)ppro->m_wstrMoodMessage);
		else if (m_iMood == 1) {
			int code = Utf16toUtf32(ppro->m_wstrMoodEmoji);
			s2.Format(L"<ss type=\"%x\">(%x)</ss>%s", code, code, (wchar_t *)ppro->m_wstrMoodMessage);
		}

		if (!s2.IsEmpty())
			payload << WCHAR_PARAM("richMood", s2);

		node << payload;
		m_szParam = node.write().c_str();
	}
};

int getMoodIndex(const char *pszMood)
{
	for (auto &it : moods)
		if (!mir_strcmpi(it.ss, pszMood))
			return int(&it - moods);

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Mood dialog

class CMoodDialog : public CSkypeDlg
{
	CCtrlEdit edtText, edtEmoji;
	CCtrlCombo cmbMoods;

public:
	CMoodDialog(CSkypeProto *ppro) :
		CSkypeDlg(ppro, IDD_MOOD),
		edtText(this, IDC_MOOD_TEXT),
		edtEmoji(this, IDC_MOOD_EMOJI),
		cmbMoods(this, IDC_MOOD_COMBO)
	{
		CreateLink(edtText, ppro->m_wstrMoodMessage);
		CreateLink(edtEmoji, ppro->m_wstrMoodEmoji);

		cmbMoods.OnChange = Callback(this, &CMoodDialog::onChangeSel_Mood);
	}

	bool OnInitDialog() override
	{
		for (auto &it : moods)
			cmbMoods.AddString(TranslateW(it.defStatus), int(&it - moods));
		cmbMoods.SetCurSel(m_proto->m_iMood);
		onChangeSel_Mood(0);
		return true;
	}

	bool OnApply() override
	{
		m_proto->m_iMood = cmbMoods.GetCurSel();

		CMStringA szSetting(FORMAT, "Mood%d", (int)m_proto->m_iMood);
		m_proto->setWString(szSetting, m_proto->m_wstrMoodMessage);

		m_proto->PushRequest(new SetStatusMsgRequest(m_proto));
		return true;
	}

	void onChangeSel_Mood(CCtrlCombo *)
	{
		int m_iMood = cmbMoods.GetCurSel();
		edtEmoji.Enable(m_iMood == 1);

		CMStringA szSetting(FORMAT, "Mood%d", m_iMood);
		edtText.SetText(m_proto->getMStringW(szSetting));
	}
};

INT_PTR CSkypeProto::SvcSetMood(WPARAM, LPARAM)
{
	CMoodDialog(this).DoModal();
	return 0;
}
