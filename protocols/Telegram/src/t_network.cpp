#include "stdafx.h"

#define PING_TIMEOUT 10

static void start_ping_timer(connection *c);

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
}

struct tgl_net_methods mtgl_conn_methods = {
	mtgln_write_out,
	mtgln_read_in,
	mtgln_read_in_lookup,
	mtgln_flush_out,
	incr_out_packet_num,
	tgln_free,
	get_dc,
	get_session,
	mtgln_create_connection
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