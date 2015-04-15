#ifndef _TOX_MULTIMEDIA_H_
#define _TOX_MULTIMEDIA_H_

class CToxAudioCall : public CToxDlgBase
{
private:
	int callId;

	CCtrlButton ok;
	CCtrlButton cancel;

protected:
	void OnInitDialog();
	void OnOk(CCtrlBase*);
	void OnCancel(CCtrlBase*);
	void OnClose();

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CToxAudioCall(CToxProto *proto, int callId);
};

#endif //_TOX_MULTIMEDIA_H_