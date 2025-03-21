struct HttpRequest : public MTHttpRequest<CTeamsProto>
{
	HttpRequest(int iRequestType, const char *pszUrl);
};

class CTeamsProto : public PROTO<CTeamsProto>
{
	friend class COptionsMainDlg;

	// options
	int __cdecl OnOptionsInit(WPARAM, LPARAM);

	CMOption<wchar_t*> m_wstrCListGroup;

public:
	// constructor
	CTeamsProto(const char *protoName, const wchar_t *userName);
	~CTeamsProto();

	MWindow  OnCreateAccMgrUI(MWindow) override;
};

typedef CProtoDlgBase<CTeamsProto> CTeamsDlgBase;

struct CMPlugin : public ACCPROTOPLUGIN<CTeamsProto>
{
	CMPlugin();

	int Load() override;
};
