
class CIcqProto;

enum IcqConnection
{
	CONN_NONE = -1, CONN_MAIN = 0, CONN_FETCH = 1, CONN_LAST = 2
};

struct AsyncHttpRequest : public MTHttpRequest<CIcqProto>
{
	IcqConnection m_conn;
	GUID m_reqId;

	AsyncHttpRequest(IcqConnection, int type, const char *szUrl, MTHttpRequestHandler pFunc = nullptr);
};

/////////////////////////////////////////////////////////////////////////////////////////

class JsonReply
{
	JSONNode *m_root = nullptr;
	int m_errorCode = 0;
	JSONNode* m_data = nullptr;

public:
	JsonReply(NETLIBHTTPREQUEST*);
	~JsonReply();

	__forceinline JSONNode& data() const { return *m_data; }
	__forceinline int error() const { return m_errorCode; }
};
