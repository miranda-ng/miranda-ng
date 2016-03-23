#include "stdafx.h"

static void update_message_handler(tgl_state *TLS, tgl_message *msg)
{
	((MirTLS*)TLS)->m_proto->OnMessage(msg);
}

static void logprintf(const char *fmt, ...)
{
	CMStringA str("[Telegram]: ");
	va_list args;
	va_start(fmt, args);
	str.AppendFormatV(fmt, args);
	va_end(args);
	CallService(MS_NETLIB_LOG, 0, (LPARAM)str.GetString());
}

void CTelegramProto::InitCallbacks()
{
	tgl_update_callback cb = { 0 };
	cb.new_msg = update_message_handler;
	cb.msg_receive = update_message_handler;
	cb.logprintf = logprintf;

	tgl_set_callback(TLS, &cb);

}