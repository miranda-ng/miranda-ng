#include <Windows.h>

#include <m_string.h>

#include "mir_signal.h"

SignalBuffer::SignalBuffer(const MBinBuffer &buf)
{
	m_buf = signal_buffer_create(buf.data(), buf.length());
}

SignalBuffer::SignalBuffer(const ec_public_key *key)
{
	ec_public_key_serialize(&m_buf, key);
}

SignalBuffer::SignalBuffer(const ec_private_key *key)
{
	ec_private_key_serialize(&m_buf, key);
}

SignalBuffer::SignalBuffer(const session_pre_key *pre_key)
{
	session_pre_key_serialize(&m_buf, pre_key);
}

SignalBuffer::SignalBuffer(const session_signed_pre_key *pre_key)
{
	session_signed_pre_key_serialize(&m_buf, pre_key);
}

SignalBuffer::~SignalBuffer()
{
	signal_buffer_free(m_buf);
}

CMStringA SignalBuffer::toBase64() const
{
	return ptrA(mir_base64_encode(m_buf->data, m_buf->len)).get();
}