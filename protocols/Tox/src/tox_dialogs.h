#ifndef _TOX_DIALOGS_H_
#define _TOX_DIALOGS_H_

class CCtrlLabel : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlLabel(CDlgBase *dlg, int ctrlId) :
		CSuper(dlg, ctrlId)
	{
	}

	virtual void OnInit()
	{
		CSuper::OnInit();
		OnReset();
	}

	virtual void OnReset()
	{
		if (GetDataType() == DBVT_TCHAR)
			SetText(LoadText());
		else if (GetDataType() != DBVT_DELETED)
			SetInt(LoadInt());
	}
};

///////////////////////////////////////////////

typedef CProtoDlgBase<CToxProto> CToxDlgBase;

#endif //_TOX_DIALOGS_H_