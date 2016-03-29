enum conn_state {
	conn_none,
	conn_connecting,
	conn_ready,
	conn_failed,
	conn_stopped
};

struct connection : public MZeroedObject
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
	HANDLE in_fail_timer;

	struct connection_buffer *in_head;
	struct connection_buffer *in_tail;
	struct connection_buffer *out_head;
	struct connection_buffer *out_tail;

	int in_bytes;
	int out_bytes;
	int packet_num;
	int out_packet_num;


	HANDLE ping_ev, fail_ev, write_ev, read_ev;

	time_t last_connect_time;

};

struct connection_buffer {
	unsigned char *start;
	unsigned char *end;
	unsigned char *rptr;
	unsigned char *wptr;
	struct connection_buffer *next;
};