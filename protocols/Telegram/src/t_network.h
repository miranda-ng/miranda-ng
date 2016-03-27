struct connection
{
	char *ip;
	int port;
	int flags;
	enum conn_state state;
	struct mtproto_methods *methods;
	struct tgl_state *TLS;
	struct tgl_session *session;
	struct tgl_dc *dc;
	void *extra;
	double last_receive_time;
	HANDLE hNetlibConn;
	HANDLE ping_timer;
};