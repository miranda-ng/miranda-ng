#include "stdafx.h"

CDropboxDlgBase::CDropboxDlgBase(CDropbox *instance, int idDialog)
	: CDlgBase(g_hInstance, idDialog), m_instance(instance)
{
}

void CDropboxDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, type, iValue);
}

void CDropboxDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, szValue);
}