/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////
// Popups

void CMTProto::InitPopups(void)
{
	g_plugin.addPopupOption(CMStringW(FORMAT, TranslateT("%s error notifications"), m_tszUserName), m_bUsePopups);

	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	wchar_t desc[256];
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Errors"));

	POPUPCLASS ppc = {};
	ppc.flags = PCF_UNICODE;
	ppc.pszName = name;
	ppc.pszDescription.w = desc;
	ppc.hIcon = IcoLib_GetIconByHandle(m_hProtoIcon);
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClass = Popup_RegisterClass(&ppc);

	IcoLib_ReleaseIcon(ppc.hIcon);
}

void CMTProto::Popup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle)
{
	if (!m_bUsePopups)
		return;

	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	CMStringW wszTitle(szTitle);
	if (hContact == 0) {
		wszTitle.Insert(0, L": ");
		wszTitle.Insert(0, m_tszUserName);
	}

	POPUPDATACLASS ppd = {};
	ppd.szTitle.w = wszTitle;
	ppd.szText.w = szMsg;
	ppd.pszClassName = name;
	ppd.hContact = hContact;
	Popup_AddClass(&ppd);
}
