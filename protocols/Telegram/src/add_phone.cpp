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

class CAddPhoneContactDlg : public CTelegramDlgBase
{
	CCtrlEdit edtFirstName, edtLastName, edtPhone;
	CCtrlCombo cmbCountry;
	CCtrlButton btnOk;

public:
	CAddPhoneContactDlg(CTelegramProto *ppro) :
		CTelegramDlgBase(ppro, IDD_ADD_PHONE),
		btnOk(this, IDOK),
		edtPhone(this, IDC_PHONE),
		cmbCountry(this, IDC_COUNTRY),
		edtLastName(this, IDC_LAST_NAME),
		edtFirstName(this, IDC_FIRST_NAME)
	{
		cmbCountry.OnChange = Callback(this, &CAddPhoneContactDlg::onChange_Country);
	}

	bool OnInitDialog() override
	{
		int iCount;
		CountryListEntry *pList;
		CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&iCount, (LPARAM)&pList);

		for (int i = 0; i < iCount; i++) {
			unsigned countryCode = pList[i].id;
			int idx = cmbCountry.AddString(TranslateW(_A2T(pList[i].szName).get()), countryCode);
			if (countryCode == m_proto->m_iCountry)
				cmbCountry.SetCurSel(idx);
		}

		onChange_Country(0);
		return true;
	}

	bool OnApply() override
	{
		CMStringW wszCountry(FORMAT, L"+%d%s", (int)cmbCountry.GetCurData(), ptrW(edtPhone.GetText()).get());

		auto *cc = new TD::contact;
		cc->first_name_ = T2Utf(ptrW(edtFirstName.GetText()));
		cc->last_name_ = T2Utf(ptrW(edtLastName.GetText()));
		cc->phone_number_ = T2Utf(wszCountry);

		TD::array<TD::object_ptr<TD::contact>> contacts;
		contacts.push_back(TD::object_ptr<TD::contact>(cc));

		m_proto->SendQuery(new TD::importContacts(std::move(contacts)));
		return true;
	}

	void OnChange() override
	{
		ptrW wszFirstName(edtFirstName.GetText()), wszPhone(edtPhone.GetText());
		btnOk.Enable(mir_wstrlen(wszFirstName) && mir_wstrlen(wszPhone) && cmbCountry.GetCurData() != 9999);
	}

	void onChange_Country(CCtrlCombo *)
	{
		CMStringA buf;
		switch (int iCode = cmbCountry.GetCurData()) {
		case 9999:
		case -1:
			buf = "---";
			break;
		default:
			buf.Format("+%d", iCode);
		}
		SetDlgItemTextA(m_hwnd, IDC_CODE, buf);
	}
};

INT_PTR CTelegramProto::SvcAddByPhone(WPARAM, LPARAM)
{
	(new CAddPhoneContactDlg(this))->Show();
	return 0;
}
