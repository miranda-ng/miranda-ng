#ifndef _SLACK_OPTIONS_H_
#define _SLACK_OPTIONS_H_

class CSlackOptionsMain : public CProtoDlgBase<CSlackProto>
{
	typedef CProtoDlgBase<CSlackProto> CSuper;

private:

protected:
	void OnInitDialog();
	void OnApply();

public:
	CSlackOptionsMain(CSlackProto *proto, int idDialog);
};

#endif //_SLACK_OPTIONS_H_
