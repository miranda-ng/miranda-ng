#pragma once

/////////////////////////////////////////////////////////////////////////////////////////

class CDeltaChatProto : public PROTO<CDeltaChatProto>
{
	friend class CAccountOptionsDlg;

	dc_context_t *m_context = 0;

	bool m_bRunning;
	void Logout();
	void SendLogout();

	void HandleEvents();
	void __cdecl ServerThread(void *);

	CMOption<wchar_t *> m_imapUser, m_imapPass, m_imapHost;
	CMOption<uint32_t> m_imapPort;

public:
	CDeltaChatProto(const char*,const wchar_t*);
	~CDeltaChatProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	INT_PTR  GetCaps(int, MCONTACT) override;

	int      SetStatus(int) override;

	//////////////////////////////////////////////////////////////////////////////////////

	int  OnOptionsInit(WPARAM, LPARAM);
};

typedef CProtoDlgBase<CDeltaChatProto> CDeltaChatDlgBase;

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public ACCPROTOPLUGIN<CDeltaChatProto>
{
	CMPlugin();

	int Load() override;
};
