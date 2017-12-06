#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class COptions : public CDlgBase
{
private:
	CCtrlButton m_shortcut;
	CCtrlButton m_preview;
	CCtrlCheck  m_enabled;

protected:
	void OnInitDialog();

	void Shortcut_OnClick(CCtrlBase*);
	void Preview_OnClick(CCtrlBase*);
	void Enabled_OnChange(CCtrlCheck*);

public:
	COptions();
};

#endif //_OPTIONS_H_