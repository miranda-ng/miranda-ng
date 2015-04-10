#ifndef _TOX_DIALOGS_H_
#define _TOX_DIALOGS_H_

typedef CProtoDlgBase<CToxProto> CToxDlgBase;

class CToxPasswordEditor : public CToxDlgBase
{
private:
	CCtrlEdit password;
	CCtrlCheck savePermanently;

	CCtrlButton ok;

protected:
	void OnOk(CCtrlButton*);

public:
	CToxPasswordEditor(CToxProto *proto);
};

#endif //_TOX_DIALOGS_H_