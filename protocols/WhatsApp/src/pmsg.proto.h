#ifdef _DEBUG
#define PROTOBUF_C_UNPACK_ERROR OutputDebugStringA
#endif

#include "pmsg.pb-c.h"

namespace proto
{
	#define PROTOBUF_PTR(T,D) \
	class T { \
	Wa__##T* p; \
	public: \
	T(const MBinBuffer &buf) : p((Wa__##T*)protobuf_c_message_unpack(&D, 0, buf.length(), buf.data())) {} \
	T(const ProtobufCBinaryData &data) : p((Wa__##T*)protobuf_c_message_unpack(&D, 0, data.len, data.data)) {} \
	T(const uint8_t *pData, size_t len) : p((Wa__##T*)protobuf_c_message_unpack(&D, 0, len, pData)) {} \
	~T() { protobuf_c_message_free_unpacked(p,0); } \
	Wa__##T* operator->() const { return p; } \
	operator Wa__##T*() const { return p; } \
	};

	struct SyncActionValue : public Wa__SyncActionValue
	{ };

	struct SyncdRecord : public Wa__SyncdRecord
	{ };

	struct SenderKeyDistributionMessage : public Wa__Message__SenderKeyDistributionMessage
	{ };

	struct WebMessageInfo : public Wa__WebMessageInfo
	{ };

	PROTOBUF_PTR(ADVDeviceIdentity, wa__advdevice_identity__descriptor);
	PROTOBUF_PTR(ADVSignedDeviceIdentity, wa__advsigned_device_identity__descriptor);
	PROTOBUF_PTR(ADVSignedDeviceIdentityHMAC, wa__advsigned_device_identity_hmac__descriptor);
	PROTOBUF_PTR(CertChain, wa__cert_chain__descriptor);
	PROTOBUF_PTR(CertChain__NoiseCertificate__Details, wa__cert_chain__noise_certificate__details__descriptor);
	PROTOBUF_PTR(ClientPayload, wa__client_payload__descriptor);
	PROTOBUF_PTR(ExternalBlobReference, wa__external_blob_reference__descriptor);
	PROTOBUF_PTR(HandshakeMessage, wa__handshake_message__descriptor);
	PROTOBUF_PTR(HistorySync, wa__history_sync__descriptor);
	PROTOBUF_PTR(Message, wa__message__descriptor);
	PROTOBUF_PTR(SyncActionData, wa__sync_action_data__descriptor);
	PROTOBUF_PTR(SyncdSnapshot, wa__syncd_snapshot__descriptor);
	PROTOBUF_PTR(SyncdPatch, wa__syncd_patch__descriptor);

	MBinBuffer Serialize(const ProtobufCMessage *msg);
	void CleanBinary(ProtobufCBinaryData &field);
	ProtobufCBinaryData SetBinary(const void *pData, size_t len);
};
