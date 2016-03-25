#include "stdafx.h"

static void update_message_handler(tgl_state *TLS, tgl_message *msg)
{
	((MirTLS*)TLS)->m_proto->OnMessage(msg);
}

static void user_typing(tgl_state *TLS, tgl_user *U, enum tgl_typing_status status)
{
	((MirTLS*)TLS)->m_proto->OnUserTyping(U, status);
}

static void on_login_success(tgl_state *TLS)
{
//	write_auth_file(TLS);
}

static void on_login_failed(tgl_state *TLS)
{

}

static void on_ready(tgl_state *TLS)
{
	tgl_do_update_contact_list(TLS, 0, 0);
}

void request_value(struct tgl_state *TLS, enum tgl_value_type type, const char *prompt, int num_values,
	void(*callback) (struct tgl_state *TLS, const char *string[], void *arg), void *arg) 
{
	char *result = nullptr;
	((MirTLS*)TLS)->m_proto->TGLGetValue(type, prompt, num_values, &result);
	callback(TLS, (const char**)&result, arg);
}

static void logprintf(const char *fmt, ...)
{
	CMStringA str("[Telegram]: ");
	va_list args;
	va_start(args, fmt);
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
	cb.type_notification = user_typing;
	cb.logged_in = on_login_success;
	cb.started = on_ready;
	cb.get_values = request_value;

	

	tgl_set_callback(TLS, &cb);

}