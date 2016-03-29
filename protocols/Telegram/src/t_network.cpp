#include "stdafx.h"

#define PING_TIMEOUT 10

/*static void start_ping_timer(connection *c);

void __stdcall ping_alarm(PVOID arg, BOOLEAN TimerOrWaitFired) 
{
	struct connection *c = (connection*)arg;
	if (tglt_get_double_time() - c->last_receive_time > 6 * PING_TIMEOUT) 
	{
		c->TLS->callback.logprintf("fail connection: reason: ping timeout");
	}
	else if (tglt_get_double_time() - c->last_receive_time > 3 * PING_TIMEOUT) 
	{
		tgl_do_send_ping(c->TLS, c);
		start_ping_timer(c);
	}
	else
	{
		start_ping_timer(c);
	}
}

static void start_ping_timer(connection *c)
{
	CreateTimerQueueTimer(&c->ping_timer, hQueue, ping_alarm, c, PING_TIMEOUT * 1000, 0, 0);
}

static void stop_ping_timer(struct connection *c)
{
	DeleteTimerQueueTimer(hQueue, c->ping_timer, 0);
	c->ping_timer = 0;
}


int mtgln_write_out(struct connection *c, const void *_data, int len) 
{
	return Netlib_Send(c->hNetlibConn, (char*)_data, len, 0);
}

int mtgln_read_in(struct connection *c, void *_data, int len) 
{
	return Netlib_Recv(c->hNetlibConn, (char*)_data, len, 0);
}

int mtgln_read_in_lookup(struct connection *c, void *_data, int len)
{
	return Netlib_Recv(c->hNetlibConn, (char*)_data, len, MSG_PEEK);
}

connection* mtgln_create_connection(struct tgl_state *TLS, const char *host, int port, struct tgl_session *session, struct tgl_dc *dc, struct mtproto_methods *methods)
{
	connection *conn = new connection;
	conn->TLS = TLS;
	conn->ip = mir_strdup(host);
	conn->port = port;
	conn->dc = dc;
	conn->session = session;
	conn->methods = methods;

	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.szHost = conn->ip;
	nloc.wPort = port;
	
	if (!(conn->hNetlibConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)((MirTLS*)TLS)->m_proto->m_hNetlibUser, (LPARAM)&nloc)))
	{
		delete conn;
		return 0;
	}

	char b = 0xfe;
	mtgln_write_out(conn, &b, 1);

	conn->last_receive_time = tglt_get_double_time();
	start_ping_timer(conn);

	return conn;
}

static void incr_out_packet_num(struct connection *c) {
}

static struct tgl_dc *get_dc(struct connection *c) {
	return c->dc;
}

static struct tgl_session *get_session(struct connection *c) {
	return c->session;
}

static void tgln_free(struct connection *c)
{
	Netlib_CloseHandle(c->hNetlibConn);
	mir_free(c->ip);
	delete c;
}

void mtgln_flush_out(struct connection *c) {
}*/

#define vlogprintf(verbosity_level,...) \
do { \
if (TLS->verbosity >= verbosity_level) {\
	TLS->callback.logprintf(__VA_ARGS__); \
} \
} while (0)

#define assert(x) x



static void fail_connection(struct connection *c);

#define PING_TIMEOUT 10

static void start_ping_timer(struct connection *c);

void __stdcall ping_alarm(PVOID arg, BOOLEAN TimerOrWaitFired)
{
	struct connection *c = (connection*)arg;
	if (tglt_get_double_time() - c->last_receive_time > 6 * PING_TIMEOUT)
	{
		c->TLS->callback.logprintf("fail connection: reason: ping timeout");
	}
	else if (tglt_get_double_time() - c->last_receive_time > 3 * PING_TIMEOUT)
	{
		tgl_do_send_ping(c->TLS, c);
		start_ping_timer(c);
	}
	else
	{
		start_ping_timer(c);
	}
}

static void start_ping_timer(connection *c)
{
	CreateTimerQueueTimer(&c->ping_timer, hQueue, ping_alarm, c, PING_TIMEOUT * 1000, 0, 0);
}

static void stop_ping_timer(struct connection *c)
{
	DeleteTimerQueueTimer(hQueue, c->ping_timer, 0);
	c->ping_timer = 0;
}

static void restart_connection(struct connection *c);

void __stdcall fail_alarm(PVOID arg, BOOLEAN TimerOrWaitFired)
{
	struct connection *c = (connection*)arg;
	c->in_fail_timer = 0;
	restart_connection(c);
}

static void start_fail_timer(struct connection *c) {
	if (c->in_fail_timer) { return; }
	CreateTimerQueueTimer(&c->in_fail_timer, hQueue, fail_alarm, c, PING_TIMEOUT * 1000, 0, 0);
}

connection_buffer *new_connection_buffer(int size) {
	connection_buffer *b = (connection_buffer*)talloc0(sizeof(*b));
	b->start = (PBYTE)talloc(size);
	b->end = b->start + size;
	b->rptr = b->wptr = b->start;
	return b;
}

static void delete_connection_buffer(struct connection_buffer *b) {
	tfree(b->start, b->end - b->start);
	tfree(b, sizeof (*b));
}

int mtgln_write_out(struct connection *c, const void *_data, int len) 
{
	struct tgl_state *TLS = c->TLS;
	vlogprintf(E_DEBUG, "write_out: %d bytes\n", len);
	const unsigned char *data = (PBYTE)_data;
	if (!len) { return 0; }
	assert(len > 0);
	int x = 0;
	if (!c->out_bytes) {
		NotifyEventHooks(c->write_ev, (WPARAM)c);
	}
	if (!c->out_head) {
		struct connection_buffer *b = new_connection_buffer(1 << 20);
		c->out_head = c->out_tail = b;
	}
	while (len) {
		if (c->out_tail->end - c->out_tail->wptr >= len) {
			memcpy(c->out_tail->wptr, data, len);
			c->out_tail->wptr += len;
			c->out_bytes += len;
			return x + len;
		}
		else {
			int y = c->out_tail->end - c->out_tail->wptr;
			assert(y < len);
			memcpy(c->out_tail->wptr, data, y);
			x += y;
			len -= y;
			data += y;
			struct connection_buffer *b = new_connection_buffer(1 << 20);
			c->out_tail->next = b;
			b->next = 0;
			c->out_tail = b;
			c->out_bytes += y;
		}
	}
	return x;
}

int tgln_read_in(struct connection *c, void *_data, int len) {
	unsigned char *data = (PBYTE)_data;
	if (!len) { return 0; }
	assert(len > 0);
	if (len > c->in_bytes) {
		len = c->in_bytes;
	}
	int x = 0;
	while (len) {
		int y = c->in_head->wptr - c->in_head->rptr;
		if (y > len) {
			memcpy(data, c->in_head->rptr, len);
			c->in_head->rptr += len;
			c->in_bytes -= len;
			return x + len;
		}
		else {
			memcpy(data, c->in_head->rptr, y);
			c->in_bytes -= y;
			x += y;
			data += y;
			len -= y;
			void *old = c->in_head;
			c->in_head = c->in_head->next;
			if (!c->in_head) {
				c->in_tail = 0;
			}
			delete_connection_buffer((connection_buffer*)old);
		}
	}
	return x;
}

int tgln_read_in_lookup(struct connection *c, void *_data, int len) {
	unsigned char *data = (PBYTE)_data;
	if (!len || !c->in_bytes) { return 0; }
	assert(len > 0);
	if (len > c->in_bytes) {
		len = c->in_bytes;
	}
	int x = 0;
	struct connection_buffer *b = c->in_head;
	while (len) {
		int y = b->wptr - b->rptr;
		if (y >= len) {
			memcpy(data, b->rptr, len);
			return x + len;
		}
		else {
			memcpy(data, b->rptr, y);
			x += y;
			data += y;
			len -= y;
			b = b->next;
		}
	}
	return x;
}

static void try_write(struct connection *c);

void tgln_flush_out(struct connection *c) 
{
//	try_write(c);
}

#define MAX_CONNECTIONS 100
static struct connection *Connections[MAX_CONNECTIONS];
static int max_connection_fd;

static void rotate_port(struct connection *c) {
	switch (c->port) {
	case 443:
		c->port = 80;
		break;
	case 80:
		c->port = 25;
		break;
	case 25:
		c->port = 443;
		break;
	}
}

static void try_read(struct connection *c);
static void try_write(struct connection *c);


void __stdcall conn_try_read(void *param, BYTE){
	struct connection *c = (connection*)param;
	struct tgl_state *TLS = c->TLS;
	vlogprintf(E_DEBUG + 1, "Try read. Fd = %d\n", c->hNetlibConn);
	try_read(c);
}

static int conn_try_write(WPARAM w, LPARAM) 
{
	struct connection *c = (connection*)w;
	struct tgl_state *TLS = c->TLS;
	if (c->state == conn_connecting) {
		c->state = conn_ready;
		c->methods->ready(TLS, c);
	}
	try_write(c);
	if (c->out_bytes) {
		NotifyEventHooks(c->write_ev, (WPARAM)c);
	}
	return 0;
}

struct connection *tgln_create_connection(struct tgl_state *TLS, const char *host, int port, struct tgl_session *session, struct tgl_dc *dc, struct mtproto_methods *methods) 
{
	struct connection *c = new connection;
	c->TLS = TLS;
	c->ip = tstrdup(host);
	c->port = port;

	
	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.szHost = c->ip;
	nloc.wPort = port;

	if (!(c->hNetlibConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)((MirTLS*)TLS)->m_proto->m_hNetlibUser, (LPARAM)&nloc)))
	{
		delete c;
		return 0;
	}

	c->state = conn_connecting;
	c->last_receive_time = tglt_get_double_time();
	c->flags = 0;

	char szTmp[2048] = "TGL_NET_TMPHOOK/";
	char *dst = &szTmp[strlen(szTmp)];
	MUUID uuid;
	CoCreateGuid((GUID*)&uuid);
	mir_snprintf(dst, 2048 - strlen(szTmp) - 1, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);

	c->write_ev = CreateHookableEvent(szTmp);
	HookEvent(szTmp, conn_try_write);





	//CoCreateGuid((GUID*)&uuid);
	//mir_snprintf(dst, 2048 - strlen(szTmp) - 1, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
	//	uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);
	//
	//c->read_ev = CreateHookableEvent(szTmp);
	//HookEvent(szTmp, conn_try_read);


	start_ping_timer(c);

	c->dc = dc;
	c->session = session;
	c->methods = methods;

	char byte = 0xef;
	assert(mtgln_write_out(c, &byte, 1) == 1);
	tgln_flush_out(c);

	CreateTimerQueueTimer(&c->write_ev, hQueue, conn_try_read, c, 1, 5000, 0);

	return c;
}

static void restart_connection(struct connection *c) 
{
	struct tgl_state *TLS = c->TLS;

	if (c->last_connect_time == time(0)) {
		start_fail_timer(c);
		return;
	}

	c->state = conn_connecting;
	c->last_receive_time = tglt_get_double_time();

	Netlib_CloseHandle(c->hNetlibConn);
	
	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.szHost = c->ip;
	nloc.wPort = c->port;

	c->hNetlibConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)((MirTLS*)TLS)->m_proto->m_hNetlibUser, (LPARAM)&nloc);

	char byte = 0xef;
	assert(mtgln_write_out(c, &byte, 1) == 1);
	tgln_flush_out(c);
}

static void fail_connection(struct connection *c)
{
	struct tgl_state *TLS = c->TLS;
	if (c->state == conn_ready || c->state == conn_connecting) {
		stop_ping_timer(c);
	}

//	DestroyHookableEvent(c->read_ev);

	rotate_port(c);
	struct connection_buffer *b = c->out_head;
	while (b) {
		struct connection_buffer *d = b;
		b = b->next;
		delete_connection_buffer(d);
	}
	b = c->in_head;
	while (b) {
		struct connection_buffer *d = b;
		b = b->next;
		delete_connection_buffer(d);
	}
	c->out_head = c->out_tail = c->in_head = c->in_tail = 0;
	c->state = conn_failed;
	c->out_bytes = c->in_bytes = 0;

	Netlib_CloseHandle(c->hNetlibConn);

	vlogprintf(E_NOTICE, "Lost connection to server... %s:%d\n", c->ip, c->port);
	restart_connection(c);
}

//extern FILE *log_net_f;
static void try_write(struct connection *c) 
{
	struct tgl_state *TLS = c->TLS;
	vlogprintf(E_DEBUG, "try write: fd = %d\n", c->hNetlibConn);
	int x = 0;
	while (c->out_head) {
		int r = Netlib_Send(c->hNetlibConn, (char*)c->out_head->rptr, c->out_head->wptr - c->out_head->rptr, MSG_RAW);
		if (r != SOCKET_ERROR) {
  
			x += r;
			c->out_head->rptr += r;
			if (c->out_head->rptr != c->out_head->wptr) {
				break;
			}
			struct connection_buffer *b = c->out_head;
			c->out_head = b->next;
			if (!c->out_head) {
				c->out_tail = 0;
			}
			delete_connection_buffer(b);
		}
		else {
			if (/*WSAGetLastError() != EAGAIN && */WSAGetLastError() != WSAEWOULDBLOCK) {
				vlogprintf(E_NOTICE, "fail_connection: write_error %s\n", GetLastErrorStr(WSAGetLastError()));
				fail_connection(c);
				return;
			}
			else {
				break;
			}
		}
	}
	vlogprintf(E_DEBUG, "Sent %d bytes to %d\n", x, c->hNetlibConn);
	c->out_bytes -= x;
}

static void try_rpc_read(struct connection *c) {
	assert(c->in_head);
	struct tgl_state *TLS = c->TLS;

	while (1) {
		if (c->in_bytes < 1) { return; }
		unsigned len = 0;
		unsigned t = 0;
		assert(tgln_read_in_lookup(c, &len, 1) == 1);
		if (len >= 1 && len <= 0x7e) {
			if (c->in_bytes < (int)(1 + 4 * len)) { return; }
		}
		else {
			if (c->in_bytes < 4) { return; }
			assert(tgln_read_in_lookup(c, &len, 4) == 4);
			len = (len >> 8);
			if (c->in_bytes < (int)(4 + 4 * len)) { return; }
			len = 0x7f;
		}

		if (len >= 1 && len <= 0x7e) {
			assert(tgln_read_in(c, &t, 1) == 1);
			assert(t == len);
			assert(len >= 1);
		}
		else {
			assert(len == 0x7f);
			assert(tgln_read_in(c, &len, 4) == 4);
			len = (len >> 8);
			assert(len >= 1);
		}
		len *= 4;
		int op;
		assert(tgln_read_in_lookup(c, &op, 4) == 4);
		if (c->methods->execute(TLS, c, op, len) < 0) {
			return;
		}
	}
}

static void try_read(struct connection *c) {
	struct tgl_state *TLS = c->TLS;
	vlogprintf(E_DEBUG, "try read: fd = %d\n", c->hNetlibConn);
	if (!c->in_tail) {
		c->in_head = c->in_tail = new_connection_buffer(1 << 20);
	}
#ifdef EVENT_V1
	struct timeval tv = { 5, 0 };
	event_add(c->read_ev, &tv);
#endif
	int x = 0;
	while (1) {
		int r = Netlib_Recv(c->hNetlibConn, (char*)c->in_tail->wptr, c->in_tail->end - c->in_tail->wptr, MSG_RAW);
		if (r != SOCKET_ERROR) {
			c->last_receive_time = tglt_get_double_time();
			stop_ping_timer(c);
			start_ping_timer(c);
		}
		if (r >= 0) {
			c->in_tail->wptr += r;
			x += r;
			if (c->in_tail->wptr != c->in_tail->end) {
				break;
			}
			struct connection_buffer *b = new_connection_buffer(1 << 20);
			c->in_tail->next = b;
			c->in_tail = b;
		}
		else {
			if (/*WSAGetLastError() != EAGAIN &&*/ WSAGetLastError() != WSAEWOULDBLOCK) {
				vlogprintf(E_NOTICE, "fail_connection: read_error %s\n", GetLastErrorStr(WSAGetLastError()));
				fail_connection(c);
				return;
			}
			else {
				break;
			}
		}
	}
	vlogprintf(E_DEBUG, "Received %d bytes from %d\n", x, c->hNetlibConn);
	c->in_bytes += x;
	if (x) {
		try_rpc_read(c);
	}
}

static void incr_out_packet_num(struct connection *c) {
	c->out_packet_num++;
}

static struct tgl_dc *get_dc(struct connection *c) {
	return c->dc;
}

static struct tgl_session *get_session(struct connection *c) {
	return c->session;
}

static void tgln_free(struct connection *c) {
	if (c->ip) { tfree_str(c->ip); }
	if (c->ping_ev) {  }
	if (c->fail_ev) {  }
//	if (c->read_ev) { DestroyHookableEvent(c->read_ev); }
	if (c->write_ev) { DestroyHookableEvent(c->write_ev); }

	struct connection_buffer *b = c->out_head;
	while (b) {
		struct connection_buffer *d = b;
		b = b->next;
		delete_connection_buffer(d);
	}
	b = c->in_head;
	while (b) {
		struct connection_buffer *d = b;
		b = b->next;
		delete_connection_buffer(d);
	}

	
	if (c->hNetlibConn) Netlib_CloseHandle(c->hNetlibConn);
	delete c;
}







struct tgl_net_methods mtgl_conn_methods = {
	mtgln_write_out,
	tgln_read_in,
	tgln_read_in_lookup,
	tgln_flush_out,
	incr_out_packet_num,
	tgln_free,
	get_dc,
	get_session,
	tgln_create_connection
};

void CTelegramProto::InitNetwork()
{
	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = L"TGL";
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);


	tgl_set_net_methods(TLS, &mtgl_conn_methods);
}