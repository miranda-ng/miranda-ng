#pragma once

class CDeltaChatProto : public PROTO<CDeltaChatProto>
{
	friend struct AsyncHttpRequest;

	dc_context_t *m_context = 0;

	bool m_bRunning;
	void Logout();
	void SendLogout();

	void __cdecl ServerThread(void *);

public:
	CDeltaChatProto(const char*,const wchar_t*);
	~CDeltaChatProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	INT_PTR  GetCaps(int, MCONTACT) override;

	int      SetStatus(int) override;
};

typedef CProtoDlgBase<CDeltaChatProto> CDeltaChatDlgBase;

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public ACCPROTOPLUGIN<CDeltaChatProto>
{
	CMPlugin();

	int Load() override;
};
