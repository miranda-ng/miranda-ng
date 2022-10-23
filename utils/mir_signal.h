#ifndef __MIR_SIGNAL_H__
#define __MIR_SIGNAL_H__

#include <m_system.h>
#include <m_utils.h>

extern "C"
{
	#include "../libs/libsignal/src/curve.h"
	#include "../libs/libsignal/src/hkdf.h"
	#include "../libs/libsignal/src/key_helper.h"
	#include "../libs/libsignal/src/protocol.h"
	#include "../libs/libsignal/src/session_builder.h"
	#include "../libs/libsignal/src/session_cipher.h"
	#include "../libs/libsignal/src/signal_protocol.h"
	#include "../libs/libsignal/src/signal_protocol_types.h"
	#include "../libs/libsignal/src/curve25519/curve25519-donna.h"
}

struct signal_buffer
{
	size_t len;
	uint8_t data[1];
};

struct signal_type_base
{
	unsigned int ref_count = 0;
	void (*destroy)(signal_type_base *instance) = 0;
};

struct ec_public_key : public signal_type_base
{
	uint8_t data[32];
};

struct ec_private_key : public signal_type_base
{
	uint8_t data[32];
};

class SignalBuffer
{
	signal_buffer *m_buf;

public:
	SignalBuffer() : m_buf(nullptr) {}
	SignalBuffer(signal_buffer *buf) : m_buf(buf) {}
	SignalBuffer(const class MBinBuffer &buf);
	SignalBuffer(const ec_public_key *key);
	SignalBuffer(const ec_private_key *key);
	SignalBuffer(const session_pre_key *pre_key);
	SignalBuffer(const session_signed_pre_key *pre_key);
	~SignalBuffer();

	void operator=(signal_buffer *buf);

	__forceinline operator bool() const { return m_buf != nullptr; }
	__forceinline uint8_t* data() const { return m_buf->data; }
	__forceinline unsigned len() const { return unsigned(m_buf->len); }
	__forceinline void reset(size_t len) { m_buf->len = len; }

	CMStringA toBase64() const;
};

#endif // __MIR_SMILEYS_H__
