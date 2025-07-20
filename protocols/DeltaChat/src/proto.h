#pragma once

class CDeltaChatProto : public PROTO<CDeltaChatProto>
{
	friend struct AsyncHttpRequest;

public:
	CDeltaChatProto(const char*,const wchar_t*);
	~CDeltaChatProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	INT_PTR  GetCaps(int, MCONTACT) override;
};

typedef CProtoDlgBase<CDeltaChatProto> CDeltaChatDlgBase;

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public ACCPROTOPLUGIN<CDeltaChatProto>
{
	dc_context_t *m_context;

	CMPlugin();

	int Load() override;
};
