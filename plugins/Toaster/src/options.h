#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class COptions : public CDlgBase
{
private:
	CCtrlButton m_shortcut;
	CCtrlButton m_preview;

protected:
	void OnInitDialog();

	void Shortcut_OnClick(CCtrlBase*);
	void Preview_OnClick(CCtrlBase*);

public:
	COptions();
};

#endif //_OPTIONS_H_