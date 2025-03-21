struct HttpRequest : public MTHttpRequest<CTeamsProto>
{
	HttpRequest(int iRequestType, const char *pszUrl);
};

class CTeamsProto : public PROTO<CTeamsProto>
{
public:
	// constructor
	CTeamsProto(const char *protoName, const wchar_t *userName);
	~CTeamsProto();
};

struct CMPlugin : public ACCPROTOPLUGIN<CTeamsProto>
{
	CMPlugin();

	int Load() override;
};
