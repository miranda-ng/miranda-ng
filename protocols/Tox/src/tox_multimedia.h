#ifndef _TOX_MULTIMEDIA_H_
#define _TOX_MULTIMEDIA_H_

#define WM_CALL_END (WM_PROTO_LAST + 100)

class CToxCallDlgBase : public CToxDlgBase
{
protected:
	MCONTACT hContact;

	virtual void OnInitDialog();
	virtual void OnClose();

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void SetIcon(const char *name);
	void SetTitle(const TCHAR *title);

public:
	CToxCallDlgBase(CToxProto *proto, int idDialog, MCONTACT hContact);
};

///////////////////////////////////////////////

class CToxIncomingCall : public CToxCallDlgBase
{
private:
	CCtrlLabel from;
	CCtrlLabel date;
	
	CCtrlButton answer;
	CCtrlButton reject;

protected:
	void OnInitDialog();
	void OnClose();

	void OnAnswer(CCtrlBase*);

public:
	CToxIncomingCall(CToxProto *proto, MCONTACT hContact);
};

///////////////////////////////////////////////

class CToxOutgoingCall : public CToxCallDlgBase
{
private:
	CCtrlLabel to;
	CCtrlButton call;
	CCtrlButton cancel;

protected:
	void OnInitDialog();
	void OnClose();

	void OnCall(CCtrlBase*);
	void OnCancel(CCtrlBase*);

public:
	CToxOutgoingCall(CToxProto *proto, MCONTACT hContact);
};

///////////////////////////////////////////////

struct ToxCallDialogParam
{
	CToxProto *proto;
	MCONTACT hContact;
};

class CToxCallDialog : public CToxCallDlgBase
{
protected:
	CCtrlButton end;

	void OnInitDialog();
	void OnClose();

public:
	CToxCallDialog(CToxProto *proto, MCONTACT hContact);
};

#endif //_TOX_MULTIMEDIA_H_