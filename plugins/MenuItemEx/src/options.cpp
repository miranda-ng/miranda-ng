#include "stdafx.h"

struct {
	int idc;
	int flag;
}
static const checkboxes[] = {
	{ IDC_COPYID, VF_CID },
	{ IDC_STATUSMSG, VF_STAT },
	{ IDC_COPYIP, VF_CIP },
	{ IDC_COPYMIRVER, VF_CMV },
	{ IDC_VIS, VF_VS },
	{ IDC_SHOWALPHAICONS, VF_SAI },
	{ IDC_HIDE, VF_HFL },
	{ IDC_IGNORE, VF_IGN },
	{ IDC_IGNOREHIDE, VF_IGNH },
	{ IDC_PROTOS, VF_PROTO },
	{ IDC_SHOWID, VF_SHOWID },
	{ IDC_COPYIDNAME, VF_CIDN },
	{ IDC_RECVFILES, VF_RECV },
	{ IDC_SMNAME, VF_SMNAME },
	{ IDC_TRIMID, VF_TRIMID }
};

class COptDialog : public CDlgBase
{
	uint32_t m_flags;

	CCtrlCheck m_chkVis, m_chkAlpha, m_chkHide, m_chkIgnore, m_chkProtos, m_chkRecvFiles, m_chkCopyIP, m_chkCopyMirver, m_chkStatusMsg,
		m_chkSMName, m_chkCopyID, m_chkCopyIDName, m_chkShowID, m_chkTrimID, m_chkIgnoreHide;
	CCtrlLabel m_lblHint;

public:
	COptDialog() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_chkVis(this, IDC_VIS),
		m_chkAlpha(this, IDC_SHOWALPHAICONS),
		m_chkHide(this, IDC_HIDE),
		m_chkIgnore(this, IDC_IGNORE),
		m_chkProtos(this, IDC_PROTOS),
		m_chkRecvFiles(this, IDC_RECVFILES),
		m_chkCopyIP(this, IDC_COPYIP),
		m_chkCopyMirver(this, IDC_COPYMIRVER),
		m_chkStatusMsg(this, IDC_STATUSMSG),
		m_chkSMName(this, IDC_SMNAME),
		m_chkCopyID(this, IDC_COPYID),
		m_chkCopyIDName(this, IDC_COPYIDNAME),
		m_chkShowID(this, IDC_SHOWID),
		m_chkTrimID(this, IDC_TRIMID),
		m_lblHint(this, IDC_HINT1),
		m_chkIgnoreHide(this, IDC_IGNOREHIDE)
	{
		m_flags = g_plugin.getDword("flags", vf_default);

		m_chkVis.OnChange = Callback(this, &COptDialog::onChange_Vis);
		m_chkIgnore.OnChange = Callback(this, &COptDialog::onChange_Ignore);
		m_chkStatusMsg.OnChange = Callback(this, &COptDialog::onChange_StatusMsg);
		m_chkCopyID.OnChange = m_chkShowID.OnChange = Callback(this, &COptDialog::onChange_CopyID);
	}

	bool OnInitDialog() override
	{
		for (auto &it : checkboxes) {
			CCtrlCheck &item = *(CCtrlCheck*)FindControl(it.idc);
			item.SetState(m_flags & it.flag);
		}

		for (int i = 0; i < 4; i++) {
			CCtrlCheck &item = *(CCtrlCheck*)FindControl(checkboxes[i].idc);
			CMStringW buffer;
			buffer.Format(L"%s *", item.GetText());
			item.SetText(buffer);
		}
		return true;
	}

	bool OnApply() override
	{
		uint32_t mod_flags = 0;

		for (auto &it : checkboxes) {
			CCtrlCheck &item = *(CCtrlCheck*)FindControl(it.idc);
			mod_flags |= item.GetState() ? it.flag : 0;
		}

		g_plugin.setDword("flags", mod_flags);
		return true;
	}

	void onChange_Vis(CCtrlBase*)
	{
		m_chkAlpha.Enable(m_chkVis.GetState());
	}

	void onChange_Ignore(CCtrlBase*)
	{
		m_chkIgnoreHide.Enable(m_chkIgnore.GetState());
	}

	void onChange_CopyID(CCtrlBase*)
	{
		m_chkCopyIDName.Enable(m_chkCopyID.GetState());
		m_chkShowID.Enable(m_chkCopyID.GetState());
		m_chkTrimID.Enable(m_chkCopyID.GetState() && m_chkShowID.GetState());
	}

	void onChange_StatusMsg(CCtrlBase*)
	{
		m_chkSMName.Enable(m_chkStatusMsg.GetState());
	}
};

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = MODULENAME;
	odp.pDialog = new COptDialog;
	odp.szGroup.a = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wparam, &odp);
	return 0;
}
