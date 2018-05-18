// Copyright © 2008 sss, chaos.persei
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"

class COptWaMpdDlg : public CDlgBase
{
public:
	COptWaMpdDlg() : CDlgBase(g_plugin, IDD_OPT_WA_MPD),
		edit_PORT(this, IDC_PORT), edit_SERVER(this, IDC_SERVER), edit_PASSWORD(this, IDC_PASSWORD)
	{}
	virtual void OnInitDialog() override
	{
		edit_PORT.SetInt(db_get_w(NULL, szModuleName, "Port", 6600));
		wchar_t *tmp = UniGetContactSettingUtf(NULL, szModuleName, "Server", L"127.0.0.1");
		edit_SERVER.SetText(tmp);
		mir_free(tmp);
		tmp = UniGetContactSettingUtf(NULL, szModuleName, "Password", L"");
		edit_PASSWORD.SetText(tmp);
		mir_free(tmp);
	}
	virtual void OnApply() override
	{
		db_set_w(NULL, szModuleName, "Port", (WORD)edit_PORT.GetInt());
		gbPort = edit_PORT.GetInt();
		db_set_ws(NULL, szModuleName, "Server", edit_SERVER.GetText());
		mir_wstrcpy(gbHost, edit_SERVER.GetText());
		db_set_ws(NULL, szModuleName, "Password", edit_PASSWORD.GetText());
		mir_wstrcpy(gbPassword, edit_PASSWORD.GetText());
	}
private:
	CCtrlSpin edit_PORT;
	CCtrlEdit edit_SERVER, edit_PASSWORD;
};


int WaMpdOptInit(WPARAM wParam,LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.szTitle.w = LPGENW("Winamp Track");
	odp.szGroup.w = LPGENW("Plugins");
	odp.szTab.w = LPGENW("Watrack MPD");
	odp.flags=ODPF_BOLDGROUPS|ODPF_UNICODE;
	odp.pDialog = new COptWaMpdDlg();
	Options_AddPage(wParam, &odp);
	return 0;
}
