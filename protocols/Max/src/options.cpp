#include "stdafx.h"

class CMaxOptionsDlg : public CMaxDlgBase
{
	CCtrlEdit edtToken, edtDevice;

public:
	CMaxOptionsDlg(CMaxProto *ppro, int iDlgId) :
		CMaxDlgBase(ppro, iDlgId),
		edtToken(this, IDC_TOKEN),
		edtDevice(this, IDC_DEVICEID)
	{
		CreateLink(edtToken, ppro->m_szToken);
		CreateLink(edtDevice, ppro->m_szDeviceId);
	}
};

MWindow CMaxProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	auto *pDlg = new CMaxOptionsDlg(this, IDD_ACCMGRUI);
	pDlg->SetParent(hwndParent);
	pDlg->Create();
	return pDlg->GetHwnd();
}
