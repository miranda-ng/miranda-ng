#ifndef _TOX_MULTIMEDIA_H_
#define _TOX_MULTIMEDIA_H_

class CToxAudioCall : public CToxDlgBase
{
protected:
	MCONTACT hContact;
	bool isCallStarted;

	CCtrlButton ok;
	CCtrlButton cancel;

	void SetIcon(const char *name);

	virtual void OnInitDialog();
	void OnClose();

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnOk(CCtrlBase*) = 0;
	virtual void OnCancel(CCtrlBase*) = 0;

public:
	CToxAudioCall(CToxProto *proto, MCONTACT hContact);
	
	void OnStartCall();
};

class CToxIncomingAudioCall : public CToxAudioCall
{
protected:
	void OnInitDialog();

	void OnOk(CCtrlBase*);
	void OnCancel(CCtrlBase*);

public:
	CToxIncomingAudioCall(CToxProto *proto, MCONTACT hContact);
};

class CToxOutcomingAudioCall : public CToxAudioCall
{
protected:
	void OnInitDialog();

	void OnOk(CCtrlBase*);
	void OnCancel(CCtrlBase*);

public:
	CToxOutcomingAudioCall(CToxProto *proto, MCONTACT hContact);
};

#endif //_TOX_MULTIMEDIA_H_