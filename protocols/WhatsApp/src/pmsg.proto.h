#include "pmsg.pb-c.h"

namespace proto
{
	#define PROTOBUF_PTR(T,D) \
	class T : public _Wa__##T { \
	T* p; \
	public: \
	T(const MBinBuffer &buf) : p((T*)protobuf_c_message_unpack(&D, 0, buf.length(), buf.data())) {} \
	T(const ProtobufCBinaryData &data) : p((T*)protobuf_c_message_unpack(&D, 0, data.len, data.data)) {} \
	T(const uint8_t *pData, size_t len) : p((T*)protobuf_c_message_unpack(&D, 0, len, pData)) {} \
	~T() { protobuf_c_message_free_unpacked((ProtobufCMessage*)p,0); } \
	operator T& () const { return *p; } \
	operator const ProtobufCMessage*() const { return (const ProtobufCMessage*)p; } \
	};

	struct SyncActionValue : public _Wa__SyncActionValue
	{ };

	struct SyncdRecord : public _Wa__SyncdRecord
	{ };

	struct SenderKeyDistributionMessage : public _Wa__Message__SenderKeyDistributionMessage
	{ };

	struct WebMessageInfo : public _Wa__WebMessageInfo
	{ };

	PROTOBUF_PTR(ADVDeviceIdentity, wa__advdevice_identity__descriptor);
	PROTOBUF_PTR(ADVSignedDeviceIdentity, wa__advsigned_device_identity__descriptor);
	PROTOBUF_PTR(ADVSignedDeviceIdentityHMAC, wa__advsigned_device_identity_hmac__descriptor);
	PROTOBUF_PTR(CertChain, wa__cert_chain__descriptor);
	PROTOBUF_PTR(CertChain__NoiseCertificate__Details, wa__cert_chain__noise_certificate__details__descriptor);
	PROTOBUF_PTR(ClientPayload, wa__client_payload__descriptor);
	PROTOBUF_PTR(ExternalBlobReference, wa__external_blob_reference__descriptor);
	PROTOBUF_PTR(HandshakeMessage, wa__handshake_message__descriptor);
	PROTOBUF_PTR(Message, wa__message__descriptor);
	PROTOBUF_PTR(SyncActionData, wa__sync_action_data__descriptor);
	PROTOBUF_PTR(SyncdSnapshot, wa__syncd_snapshot__descriptor);
	PROTOBUF_PTR(SyncdPatch, wa__syncd_patch__descriptor);

	MBinBuffer Serialize(const ProtobufCMessage *msg);
	void CleanBinary(ProtobufCBinaryData &field);
	ProtobufCBinaryData SetBinary(const void *pData, size_t len);
};
