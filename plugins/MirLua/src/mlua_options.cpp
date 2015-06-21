#include "stdafx.h"

CLuaOptions::CLuaOptions(int idDialog) : CDlgBase(g_hInstance, idDialog),
	m_scripts(this, IDC_SCRIPTS), isScriptListInit(false),
	m_reload(this, IDC_RELOAD)
{
	m_reload.OnClick = Callback(this, &CLuaOptions::OnReload);
}

void CLuaOptions::CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, type, iValue);
}

void CLuaOptions::CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, szValue);
}

void CLuaOptions::LoadScripts(const TCHAR *scriptDir, int iGroup)
{
	TCHAR searchMask[MAX_PATH];
	mir_sntprintf(searchMask, _T("%s\\%s"), scriptDir, _T("*.lua"));

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				int iItem = m_scripts.AddItem(fd.cFileName, -1, NULL, iGroup);
				if (db_get_b(NULL, MODULE, _T2A(fd.cFileName), 1))
					m_scripts.SetCheckState(iItem, TRUE);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

void CLuaOptions::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_scripts.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_INFOTIP);
	m_scripts.EnableGroupView(TRUE);
	m_scripts.AddColumn(0, _T("Script"), 300);

	TCHAR scriptDir[MAX_PATH], relativeScriptDir[MAX_PATH], header[MAX_PATH + 100];
	FoldersGetCustomPathT(g_hCommonFolderPath, scriptDir, _countof(scriptDir), VARST(COMMON_SCRIPTS_PATHT));
	PathToRelativeT(scriptDir, relativeScriptDir, NULL);
	mir_sntprintf(header, _T("%s (%s)"), TranslateT("Common scripts"), relativeScriptDir);
	m_scripts.AddGroup(0, header);
	LoadScripts(scriptDir, 0);

	FoldersGetCustomPathT(g_hCustomFolderPath, scriptDir, _countof(scriptDir), VARST(CUSTOM_SCRIPTS_PATHT));
	PathToRelativeT(scriptDir, relativeScriptDir, NULL);
	mir_sntprintf(header, _T("%s (%s)"), TranslateT("Custom scripts"), relativeScriptDir);
	m_scripts.AddGroup(1, header);
	LoadScripts(scriptDir, 1);

	isScriptListInit = true;
}

void CLuaOptions::OnApply()
{
	int count = m_scripts.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++)
	{
		TCHAR fileName[MAX_PATH];
		m_scripts.GetItemText(iItem, 0, fileName, _countof(fileName));
		if (!m_scripts.GetCheckState(iItem))
			db_set_b(NULL, MODULE, _T2A(fileName), 0);
		else
			db_unset(NULL, MODULE, _T2A(fileName));
	}
}

INT_PTR CLuaOptions::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NOTIFY:
	{
		LPNMHDR lpnmHdr = (LPNMHDR)lParam;
		if (lpnmHdr->idFrom == m_scripts.GetCtrlId() && lpnmHdr->code == LVN_ITEMCHANGED)
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uChanged & LVIF_STATE && pnmv->uNewState & LVIS_STATEIMAGEMASK)
			{
				if (isScriptListInit)
					NotifyChange();
			}
		}
	}
	break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CLuaOptions::OnReload(CCtrlBase*)
{
	g_mLua->Reload();
}