struct Opts
{
	CMOption<BYTE> bUseAuthIcon;
	CMOption<BYTE> bUseGrantIcon;
	CMOption<BYTE> bContactMenuItem;
	CMOption<BYTE> bIconsForRecentContacts;

	Opts() :
		bUseAuthIcon(MODULENAME, "EnableAuthIcon", 1),
		bUseGrantIcon(MODULENAME, "EnableGrantIcon", 1),
		bContactMenuItem(MODULENAME, "MenuItem", 0),
		bIconsForRecentContacts(MODULENAME, "EnableOnlyForRecent", 0)
	{}
};

extern Opts Options;


class COptionsDialog : public CPluginDlgBase
{
	CCtrlCheck m_chkAuthIcon;
	CCtrlCheck m_chkGrantIcon;
	CCtrlCheck m_chkMenuItem;
	CCtrlCheck m_chkOnlyForRecent;
public:
	COptionsDialog() : 
		CPluginDlgBase(g_hInst, IDD_AUTHSTATE_OPT, MODULENAME),
		m_chkAuthIcon(this, IDC_AUTHICON),
		m_chkGrantIcon(this, IDC_GRANTICON),
		m_chkMenuItem(this, IDC_ENABLEMENUITEM),
		m_chkOnlyForRecent(this, IDC_ICONSFORRECENT)
	{
		CreateLink(m_chkAuthIcon, Options.bUseAuthIcon);
		CreateLink(m_chkGrantIcon, Options.bUseGrantIcon);
		CreateLink(m_chkMenuItem, Options.bContactMenuItem);
		CreateLink(m_chkOnlyForRecent, Options.bIconsForRecentContacts);
	}

	void OnApply() override
	{
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			onExtraImageApplying((WPARAM)hContact, 0);
	}

};