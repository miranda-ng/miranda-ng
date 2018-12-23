
class CIcqProto;

struct AsyncHttpRequest : public MTHttpRequest<CIcqProto>
{
	bool m_bMainSite = true;
	GUID m_reqId;

	AsyncHttpRequest(int type, const char *szUrl, MTHttpRequestHandler pFunc = nullptr);
};
