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

	bool OnInitDialog() override
	{
		edit_PORT.SetInt(g_plugin.getWord("Port", 6600));
		edit_SERVER.SetText(ptrW(db_get_wsa(0, MODULENAME, "Server", L"127.0.0.1")));
		edit_PASSWORD.SetText(ptrW(db_get_wsa(0, MODULENAME, "Password", L"")));
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setWord("Port", (uint16_t)edit_PORT.GetInt());
		gbPort = edit_PORT.GetInt();
		g_plugin.setWString("Server", edit_SERVER.GetText());
		mir_wstrcpy(gbHost, edit_SERVER.GetText());
		g_plugin.setWString("Password", edit_PASSWORD.GetText());
		mir_wstrcpy(gbPassword, edit_PASSWORD.GetText());
		return true;
	}

private:
	CCtrlSpin edit_PORT;
	CCtrlEdit edit_SERVER, edit_PASSWORD;
};


int WaMpdOptInit(WPARAM wParam,LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = LPGENW("Winamp Track");
	odp.szGroup.w = LPGENW("Plugins");
	odp.szTab.w = LPGENW("Watrack MPD");
	odp.flags=ODPF_BOLDGROUPS|ODPF_UNICODE;
	odp.pDialog = new COptWaMpdDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
