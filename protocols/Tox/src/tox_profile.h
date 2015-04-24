#ifndef _TOX_PROFILE_H_
#define _TOX_PROFILE_H_

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

#endif //_TOX_PROFILE_H_