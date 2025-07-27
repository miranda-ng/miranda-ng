#include "e2e_api.h"

#include "td/tl/tl_object_parse.h"
#include "td/tl/tl_object_store.h"

#include "td/utils/common.h"
#include "td/utils/format.h"
#include "td/utils/logging.h"
#include "td/utils/SliceBuilder.h"
#include "td/utils/tl_parsers.h"
#include "td/utils/tl_storers.h"
#include "td/utils/TlStorerToString.h"

namespace td {
namespace e2e_api {

std::string to_string(const BaseObject &value) {
  TlStorerToString storer;
  value.store(storer, "");
  return storer.move_as_string();
}

object_ptr<Object> Object::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case ok::ID:
      return ok::fetch(p);
    case e2e_callPacket::ID:
      return e2e_callPacket::fetch(p);
    case e2e_callPacketLargeMsgId::ID:
      return e2e_callPacketLargeMsgId::fetch(p);
    case e2e_handshakePrivateAccept::ID:
      return e2e_handshakePrivateAccept::fetch(p);
    case e2e_handshakePrivateFinish::ID:
      return e2e_handshakePrivateFinish::fetch(p);
    case e2e_handshakeQR::ID:
      return e2e_handshakeQR::fetch(p);
    case e2e_handshakeEncryptedMessage::ID:
      return e2e_handshakeEncryptedMessage::fetch(p);
    case e2e_handshakeLoginExport::ID:
      return e2e_handshakeLoginExport::fetch(p);
    case e2e_keyContactByUserId::ID:
      return e2e_keyContactByUserId::fetch(p);
    case e2e_keyContactByPublicKey::ID:
      return e2e_keyContactByPublicKey::fetch(p);
    case e2e_personalUserId::ID:
      return e2e_personalUserId::fetch(p);
    case e2e_personalName::ID:
      return e2e_personalName::fetch(p);
    case e2e_personalPhoneNumber::ID:
      return e2e_personalPhoneNumber::fetch(p);
    case e2e_personalContactState::ID:
      return e2e_personalContactState::fetch(p);
    case e2e_personalEmojiNonces::ID:
      return e2e_personalEmojiNonces::fetch(p);
    case e2e_personalData::ID:
      return e2e_personalData::fetch(p);
    case e2e_personalOnClient::ID:
      return e2e_personalOnClient::fetch(p);
    case e2e_personalOnServer::ID:
      return e2e_personalOnServer::fetch(p);
    case e2e_valueContactByUserId::ID:
      return e2e_valueContactByUserId::fetch(p);
    case e2e_valueContactByPublicKey::ID:
      return e2e_valueContactByPublicKey::fetch(p);
    case e2e_chain_block::ID:
      return e2e_chain_block::fetch(p);
    case e2e_chain_changeNoop::ID:
      return e2e_chain_changeNoop::fetch(p);
    case e2e_chain_changeSetValue::ID:
      return e2e_chain_changeSetValue::fetch(p);
    case e2e_chain_changeSetGroupState::ID:
      return e2e_chain_changeSetGroupState::fetch(p);
    case e2e_chain_changeSetSharedKey::ID:
      return e2e_chain_changeSetSharedKey::fetch(p);
    case e2e_chain_groupBroadcastNonceCommit::ID:
      return e2e_chain_groupBroadcastNonceCommit::fetch(p);
    case e2e_chain_groupBroadcastNonceReveal::ID:
      return e2e_chain_groupBroadcastNonceReveal::fetch(p);
    case e2e_chain_groupParticipant::ID:
      return e2e_chain_groupParticipant::fetch(p);
    case e2e_chain_groupState::ID:
      return e2e_chain_groupState::fetch(p);
    case e2e_chain_sharedKey::ID:
      return e2e_chain_sharedKey::fetch(p);
    case e2e_chain_stateProof::ID:
      return e2e_chain_stateProof::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

object_ptr<Function> Function::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_nop::ID:
      return e2e_nop::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t ok::ID;

object_ptr<ok> ok::fetch(TlParser &p) {
  return make_tl_object<ok>();
}

void ok::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void ok::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void ok::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "ok");
    s.store_class_end();
  }
}

const std::int32_t e2e_callPacket::ID;

object_ptr<e2e_callPacket> e2e_callPacket::fetch(TlParser &p) {
  return make_tl_object<e2e_callPacket>();
}

void e2e_callPacket::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void e2e_callPacket::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void e2e_callPacket::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.callPacket");
    s.store_class_end();
  }
}

const std::int32_t e2e_callPacketLargeMsgId::ID;

object_ptr<e2e_callPacketLargeMsgId> e2e_callPacketLargeMsgId::fetch(TlParser &p) {
  return make_tl_object<e2e_callPacketLargeMsgId>();
}

void e2e_callPacketLargeMsgId::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void e2e_callPacketLargeMsgId::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void e2e_callPacketLargeMsgId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.callPacketLargeMsgId");
    s.store_class_end();
  }
}

object_ptr<e2e_HandshakePrivate> e2e_HandshakePrivate::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_handshakePrivateAccept::ID:
      return e2e_handshakePrivateAccept::fetch(p);
    case e2e_handshakePrivateFinish::ID:
      return e2e_handshakePrivateFinish::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

e2e_handshakePrivateAccept::e2e_handshakePrivateAccept(UInt256 const &alice_PK_, UInt256 const &bob_PK_, int64 alice_user_id_, int64 bob_user_id_, UInt256 const &alice_nonce_, UInt256 const &bob_nonce_)
  : alice_PK_(alice_PK_)
  , bob_PK_(bob_PK_)
  , alice_user_id_(alice_user_id_)
  , bob_user_id_(bob_user_id_)
  , alice_nonce_(alice_nonce_)
  , bob_nonce_(bob_nonce_)
{}

const std::int32_t e2e_handshakePrivateAccept::ID;

object_ptr<e2e_HandshakePrivate> e2e_handshakePrivateAccept::fetch(TlParser &p) {
  return make_tl_object<e2e_handshakePrivateAccept>(p);
}

e2e_handshakePrivateAccept::e2e_handshakePrivateAccept(TlParser &p)
  : alice_PK_(TlFetchInt256::parse(p))
  , bob_PK_(TlFetchInt256::parse(p))
  , alice_user_id_(TlFetchLong::parse(p))
  , bob_user_id_(TlFetchLong::parse(p))
  , alice_nonce_(TlFetchInt256::parse(p))
  , bob_nonce_(TlFetchInt256::parse(p))
{}

void e2e_handshakePrivateAccept::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(alice_PK_, s);
  TlStoreBinary::store(bob_PK_, s);
  TlStoreBinary::store(alice_user_id_, s);
  TlStoreBinary::store(bob_user_id_, s);
  TlStoreBinary::store(alice_nonce_, s);
  TlStoreBinary::store(bob_nonce_, s);
}

void e2e_handshakePrivateAccept::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(alice_PK_, s);
  TlStoreBinary::store(bob_PK_, s);
  TlStoreBinary::store(alice_user_id_, s);
  TlStoreBinary::store(bob_user_id_, s);
  TlStoreBinary::store(alice_nonce_, s);
  TlStoreBinary::store(bob_nonce_, s);
}

void e2e_handshakePrivateAccept::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.handshakePrivateAccept");
    s.store_field("alice_PK", alice_PK_);
    s.store_field("bob_PK", bob_PK_);
    s.store_field("alice_user_id", alice_user_id_);
    s.store_field("bob_user_id", bob_user_id_);
    s.store_field("alice_nonce", alice_nonce_);
    s.store_field("bob_nonce", bob_nonce_);
    s.store_class_end();
  }
}

e2e_handshakePrivateFinish::e2e_handshakePrivateFinish(UInt256 const &alice_PK_, UInt256 const &bob_PK_, int64 alice_user_id_, int64 bob_user_id_, UInt256 const &alice_nonce_, UInt256 const &bob_nonce_)
  : alice_PK_(alice_PK_)
  , bob_PK_(bob_PK_)
  , alice_user_id_(alice_user_id_)
  , bob_user_id_(bob_user_id_)
  , alice_nonce_(alice_nonce_)
  , bob_nonce_(bob_nonce_)
{}

const std::int32_t e2e_handshakePrivateFinish::ID;

object_ptr<e2e_HandshakePrivate> e2e_handshakePrivateFinish::fetch(TlParser &p) {
  return make_tl_object<e2e_handshakePrivateFinish>(p);
}

e2e_handshakePrivateFinish::e2e_handshakePrivateFinish(TlParser &p)
  : alice_PK_(TlFetchInt256::parse(p))
  , bob_PK_(TlFetchInt256::parse(p))
  , alice_user_id_(TlFetchLong::parse(p))
  , bob_user_id_(TlFetchLong::parse(p))
  , alice_nonce_(TlFetchInt256::parse(p))
  , bob_nonce_(TlFetchInt256::parse(p))
{}

void e2e_handshakePrivateFinish::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(alice_PK_, s);
  TlStoreBinary::store(bob_PK_, s);
  TlStoreBinary::store(alice_user_id_, s);
  TlStoreBinary::store(bob_user_id_, s);
  TlStoreBinary::store(alice_nonce_, s);
  TlStoreBinary::store(bob_nonce_, s);
}

void e2e_handshakePrivateFinish::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(alice_PK_, s);
  TlStoreBinary::store(bob_PK_, s);
  TlStoreBinary::store(alice_user_id_, s);
  TlStoreBinary::store(bob_user_id_, s);
  TlStoreBinary::store(alice_nonce_, s);
  TlStoreBinary::store(bob_nonce_, s);
}

void e2e_handshakePrivateFinish::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.handshakePrivateFinish");
    s.store_field("alice_PK", alice_PK_);
    s.store_field("bob_PK", bob_PK_);
    s.store_field("alice_user_id", alice_user_id_);
    s.store_field("bob_user_id", bob_user_id_);
    s.store_field("alice_nonce", alice_nonce_);
    s.store_field("bob_nonce", bob_nonce_);
    s.store_class_end();
  }
}

object_ptr<e2e_HandshakePublic> e2e_HandshakePublic::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_handshakeQR::ID:
      return e2e_handshakeQR::fetch(p);
    case e2e_handshakeEncryptedMessage::ID:
      return e2e_handshakeEncryptedMessage::fetch(p);
    case e2e_handshakeLoginExport::ID:
      return e2e_handshakeLoginExport::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

e2e_handshakeQR::e2e_handshakeQR(UInt256 const &bob_ephemeral_PK_, UInt256 const &bob_nonce_)
  : bob_ephemeral_PK_(bob_ephemeral_PK_)
  , bob_nonce_(bob_nonce_)
{}

const std::int32_t e2e_handshakeQR::ID;

object_ptr<e2e_HandshakePublic> e2e_handshakeQR::fetch(TlParser &p) {
  return make_tl_object<e2e_handshakeQR>(p);
}

e2e_handshakeQR::e2e_handshakeQR(TlParser &p)
  : bob_ephemeral_PK_(TlFetchInt256::parse(p))
  , bob_nonce_(TlFetchInt256::parse(p))
{}

void e2e_handshakeQR::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(bob_ephemeral_PK_, s);
  TlStoreBinary::store(bob_nonce_, s);
}

void e2e_handshakeQR::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(bob_ephemeral_PK_, s);
  TlStoreBinary::store(bob_nonce_, s);
}

void e2e_handshakeQR::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.handshakeQR");
    s.store_field("bob_ephemeral_PK", bob_ephemeral_PK_);
    s.store_field("bob_nonce", bob_nonce_);
    s.store_class_end();
  }
}

e2e_handshakeEncryptedMessage::e2e_handshakeEncryptedMessage(bytes const &message_)
  : message_(std::move(message_))
{}

const std::int32_t e2e_handshakeEncryptedMessage::ID;

object_ptr<e2e_HandshakePublic> e2e_handshakeEncryptedMessage::fetch(TlParser &p) {
  return make_tl_object<e2e_handshakeEncryptedMessage>(p);
}

e2e_handshakeEncryptedMessage::e2e_handshakeEncryptedMessage(TlParser &p)
  : message_(TlFetchBytes<bytes>::parse(p))
{}

void e2e_handshakeEncryptedMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(message_, s);
}

void e2e_handshakeEncryptedMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(message_, s);
}

void e2e_handshakeEncryptedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.handshakeEncryptedMessage");
    s.store_bytes_field("message", message_);
    s.store_class_end();
  }
}

e2e_handshakeLoginExport::e2e_handshakeLoginExport(bytes const &accept_, bytes const &encrypted_key_)
  : accept_(std::move(accept_))
  , encrypted_key_(std::move(encrypted_key_))
{}

const std::int32_t e2e_handshakeLoginExport::ID;

object_ptr<e2e_HandshakePublic> e2e_handshakeLoginExport::fetch(TlParser &p) {
  return make_tl_object<e2e_handshakeLoginExport>(p);
}

e2e_handshakeLoginExport::e2e_handshakeLoginExport(TlParser &p)
  : accept_(TlFetchBytes<bytes>::parse(p))
  , encrypted_key_(TlFetchBytes<bytes>::parse(p))
{}

void e2e_handshakeLoginExport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(accept_, s);
  TlStoreString::store(encrypted_key_, s);
}

void e2e_handshakeLoginExport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(accept_, s);
  TlStoreString::store(encrypted_key_, s);
}

void e2e_handshakeLoginExport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.handshakeLoginExport");
    s.store_bytes_field("accept", accept_);
    s.store_bytes_field("encrypted_key", encrypted_key_);
    s.store_class_end();
  }
}

object_ptr<e2e_Key> e2e_Key::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_keyContactByUserId::ID:
      return e2e_keyContactByUserId::fetch(p);
    case e2e_keyContactByPublicKey::ID:
      return e2e_keyContactByPublicKey::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

e2e_keyContactByUserId::e2e_keyContactByUserId(int64 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t e2e_keyContactByUserId::ID;

object_ptr<e2e_Key> e2e_keyContactByUserId::fetch(TlParser &p) {
  return make_tl_object<e2e_keyContactByUserId>(p);
}

e2e_keyContactByUserId::e2e_keyContactByUserId(TlParser &p)
  : user_id_(TlFetchLong::parse(p))
{}

void e2e_keyContactByUserId::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
}

void e2e_keyContactByUserId::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
}

void e2e_keyContactByUserId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.keyContactByUserId");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

e2e_keyContactByPublicKey::e2e_keyContactByPublicKey(UInt256 const &public_key_)
  : public_key_(public_key_)
{}

const std::int32_t e2e_keyContactByPublicKey::ID;

object_ptr<e2e_Key> e2e_keyContactByPublicKey::fetch(TlParser &p) {
  return make_tl_object<e2e_keyContactByPublicKey>(p);
}

e2e_keyContactByPublicKey::e2e_keyContactByPublicKey(TlParser &p)
  : public_key_(TlFetchInt256::parse(p))
{}

void e2e_keyContactByPublicKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(public_key_, s);
}

void e2e_keyContactByPublicKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(public_key_, s);
}

void e2e_keyContactByPublicKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.keyContactByPublicKey");
    s.store_field("public_key", public_key_);
    s.store_class_end();
  }
}

object_ptr<e2e_Personal> e2e_Personal::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_personalUserId::ID:
      return e2e_personalUserId::fetch(p);
    case e2e_personalName::ID:
      return e2e_personalName::fetch(p);
    case e2e_personalPhoneNumber::ID:
      return e2e_personalPhoneNumber::fetch(p);
    case e2e_personalContactState::ID:
      return e2e_personalContactState::fetch(p);
    case e2e_personalEmojiNonces::ID:
      return e2e_personalEmojiNonces::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

e2e_personalUserId::e2e_personalUserId(int64 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t e2e_personalUserId::ID;

object_ptr<e2e_Personal> e2e_personalUserId::fetch(TlParser &p) {
  return make_tl_object<e2e_personalUserId>(p);
}

e2e_personalUserId::e2e_personalUserId(TlParser &p)
  : user_id_(TlFetchLong::parse(p))
{}

void e2e_personalUserId::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
}

void e2e_personalUserId::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
}

void e2e_personalUserId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalUserId");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

e2e_personalName::e2e_personalName(string const &first_name_, string const &last_name_)
  : first_name_(first_name_)
  , last_name_(last_name_)
{}

const std::int32_t e2e_personalName::ID;

object_ptr<e2e_Personal> e2e_personalName::fetch(TlParser &p) {
  return make_tl_object<e2e_personalName>(p);
}

e2e_personalName::e2e_personalName(TlParser &p)
  : first_name_(TlFetchString<string>::parse(p))
  , last_name_(TlFetchString<string>::parse(p))
{}

void e2e_personalName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
}

void e2e_personalName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
}

void e2e_personalName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalName");
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_class_end();
  }
}

e2e_personalPhoneNumber::e2e_personalPhoneNumber(string const &phone_number_)
  : phone_number_(phone_number_)
{}

const std::int32_t e2e_personalPhoneNumber::ID;

object_ptr<e2e_Personal> e2e_personalPhoneNumber::fetch(TlParser &p) {
  return make_tl_object<e2e_personalPhoneNumber>(p);
}

e2e_personalPhoneNumber::e2e_personalPhoneNumber(TlParser &p)
  : phone_number_(TlFetchString<string>::parse(p))
{}

void e2e_personalPhoneNumber::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
}

void e2e_personalPhoneNumber::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
}

void e2e_personalPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalPhoneNumber");
    s.store_field("phone_number", phone_number_);
    s.store_class_end();
  }
}

e2e_personalContactState::e2e_personalContactState()
  : flags_()
  , is_contact_()
{}

e2e_personalContactState::e2e_personalContactState(int32 flags_, bool is_contact_)
  : flags_(flags_)
  , is_contact_(is_contact_)
{}

const std::int32_t e2e_personalContactState::ID;

object_ptr<e2e_Personal> e2e_personalContactState::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<e2e_personalContactState> res = make_tl_object<e2e_personalContactState>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->is_contact_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void e2e_personalContactState::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (is_contact_ << 0)), s);
}

void e2e_personalContactState::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (is_contact_ << 0)), s);
}

void e2e_personalContactState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalContactState");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (is_contact_ << 0)));
    if (var0 & 1) { s.store_field("is_contact", true); }
    s.store_class_end();
  }
}

e2e_personalEmojiNonces::e2e_personalEmojiNonces()
  : flags_()
  , self_nonce_()
  , contact_nonce_hash_()
  , contact_nonce_()
{}

e2e_personalEmojiNonces::e2e_personalEmojiNonces(int32 flags_, UInt256 const &self_nonce_, UInt256 const &contact_nonce_hash_, UInt256 const &contact_nonce_)
  : flags_(flags_)
  , self_nonce_(self_nonce_)
  , contact_nonce_hash_(contact_nonce_hash_)
  , contact_nonce_(contact_nonce_)
{}

const std::int32_t e2e_personalEmojiNonces::ID;

object_ptr<e2e_Personal> e2e_personalEmojiNonces::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<e2e_personalEmojiNonces> res = make_tl_object<e2e_personalEmojiNonces>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->self_nonce_ = TlFetchInt256::parse(p); }
  if (var0 & 2) { res->contact_nonce_hash_ = TlFetchInt256::parse(p); }
  if (var0 & 4) { res->contact_nonce_ = TlFetchInt256::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void e2e_personalEmojiNonces::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBinary::store(self_nonce_, s); }
  if (var0 & 2) { TlStoreBinary::store(contact_nonce_hash_, s); }
  if (var0 & 4) { TlStoreBinary::store(contact_nonce_, s); }
}

void e2e_personalEmojiNonces::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBinary::store(self_nonce_, s); }
  if (var0 & 2) { TlStoreBinary::store(contact_nonce_hash_, s); }
  if (var0 & 4) { TlStoreBinary::store(contact_nonce_, s); }
}

void e2e_personalEmojiNonces::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalEmojiNonces");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("self_nonce", self_nonce_); }
    if (var0 & 2) { s.store_field("contact_nonce_hash", contact_nonce_hash_); }
    if (var0 & 4) { s.store_field("contact_nonce", contact_nonce_); }
    s.store_class_end();
  }
}

e2e_personalData::e2e_personalData(UInt256 const &public_key_, array<object_ptr<e2e_personalOnServer>> &&data_)
  : public_key_(public_key_)
  , data_(std::move(data_))
{}

const std::int32_t e2e_personalData::ID;

object_ptr<e2e_personalData> e2e_personalData::fetch(TlParser &p) {
  return make_tl_object<e2e_personalData>(p);
}

e2e_personalData::e2e_personalData(TlParser &p)
  : public_key_(TlFetchInt256::parse(p))
  , data_(TlFetchVector<TlFetchObject<e2e_personalOnServer>>::parse(p))
{}

void e2e_personalData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(public_key_, s);
  TlStoreVector<TlStoreObject>::store(data_, s);
}

void e2e_personalData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(public_key_, s);
  TlStoreVector<TlStoreObject>::store(data_, s);
}

void e2e_personalData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalData");
    s.store_field("public_key", public_key_);
    { s.store_vector_begin("data", data_.size()); for (const auto &_value : data_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

e2e_personalOnClient::e2e_personalOnClient(int32 signed_at_, object_ptr<e2e_Personal> &&personal_)
  : signed_at_(signed_at_)
  , personal_(std::move(personal_))
{}

const std::int32_t e2e_personalOnClient::ID;

object_ptr<e2e_personalOnClient> e2e_personalOnClient::fetch(TlParser &p) {
  return make_tl_object<e2e_personalOnClient>(p);
}

e2e_personalOnClient::e2e_personalOnClient(TlParser &p)
  : signed_at_(TlFetchInt::parse(p))
  , personal_(TlFetchObject<e2e_Personal>::parse(p))
{}

void e2e_personalOnClient::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signed_at_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(personal_, s);
}

void e2e_personalOnClient::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signed_at_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(personal_, s);
}

void e2e_personalOnClient::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalOnClient");
    s.store_field("signed_at", signed_at_);
    s.store_object_field("personal", static_cast<const BaseObject *>(personal_.get()));
    s.store_class_end();
  }
}

e2e_personalOnServer::e2e_personalOnServer(UInt512 const &signature_, int32 signed_at_, object_ptr<e2e_Personal> &&personal_)
  : signature_(signature_)
  , signed_at_(signed_at_)
  , personal_(std::move(personal_))
{}

const std::int32_t e2e_personalOnServer::ID;

object_ptr<e2e_personalOnServer> e2e_personalOnServer::fetch(TlParser &p) {
  return make_tl_object<e2e_personalOnServer>(p);
}

e2e_personalOnServer::e2e_personalOnServer(TlParser &p)
  : signature_(TlFetchInt512::parse(p))
  , signed_at_(TlFetchInt::parse(p))
  , personal_(TlFetchObject<e2e_Personal>::parse(p))
{}

void e2e_personalOnServer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store(signed_at_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(personal_, s);
}

void e2e_personalOnServer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store(signed_at_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(personal_, s);
}

void e2e_personalOnServer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.personalOnServer");
    s.store_field("signature", signature_);
    s.store_field("signed_at", signed_at_);
    s.store_object_field("personal", static_cast<const BaseObject *>(personal_.get()));
    s.store_class_end();
  }
}

object_ptr<e2e_Value> e2e_Value::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_valueContactByUserId::ID:
      return e2e_valueContactByUserId::fetch(p);
    case e2e_valueContactByPublicKey::ID:
      return e2e_valueContactByPublicKey::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

e2e_valueContactByUserId::e2e_valueContactByUserId(array<UInt256> &&public_keys_)
  : public_keys_(std::move(public_keys_))
{}

const std::int32_t e2e_valueContactByUserId::ID;

object_ptr<e2e_Value> e2e_valueContactByUserId::fetch(TlParser &p) {
  return make_tl_object<e2e_valueContactByUserId>(p);
}

e2e_valueContactByUserId::e2e_valueContactByUserId(TlParser &p)
  : public_keys_(TlFetchVector<TlFetchInt256>::parse(p))
{}

void e2e_valueContactByUserId::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreVector<TlStoreBinary>::store(public_keys_, s);
}

void e2e_valueContactByUserId::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreVector<TlStoreBinary>::store(public_keys_, s);
}

void e2e_valueContactByUserId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.valueContactByUserId");
    { s.store_vector_begin("public_keys", public_keys_.size()); for (const auto &_value : public_keys_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

e2e_valueContactByPublicKey::e2e_valueContactByPublicKey(array<object_ptr<e2e_personalOnClient>> &&entries_)
  : entries_(std::move(entries_))
{}

const std::int32_t e2e_valueContactByPublicKey::ID;

object_ptr<e2e_Value> e2e_valueContactByPublicKey::fetch(TlParser &p) {
  return make_tl_object<e2e_valueContactByPublicKey>(p);
}

e2e_valueContactByPublicKey::e2e_valueContactByPublicKey(TlParser &p)
  : entries_(TlFetchVector<TlFetchObject<e2e_personalOnClient>>::parse(p))
{}

void e2e_valueContactByPublicKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreVector<TlStoreObject>::store(entries_, s);
}

void e2e_valueContactByPublicKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreVector<TlStoreObject>::store(entries_, s);
}

void e2e_valueContactByPublicKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.valueContactByPublicKey");
    { s.store_vector_begin("entries", entries_.size()); for (const auto &_value : entries_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

e2e_chain_block::e2e_chain_block()
  : signature_()
  , flags_()
  , prev_block_hash_()
  , changes_()
  , height_()
  , state_proof_()
  , signature_public_key_()
{}

e2e_chain_block::e2e_chain_block(UInt512 const &signature_, int32 flags_, UInt256 const &prev_block_hash_, array<object_ptr<e2e_chain_Change>> &&changes_, int32 height_, object_ptr<e2e_chain_stateProof> &&state_proof_, UInt256 const &signature_public_key_)
  : signature_(signature_)
  , flags_(flags_)
  , prev_block_hash_(prev_block_hash_)
  , changes_(std::move(changes_))
  , height_(height_)
  , state_proof_(std::move(state_proof_))
  , signature_public_key_(signature_public_key_)
{}

const std::int32_t e2e_chain_block::ID;

object_ptr<e2e_chain_block> e2e_chain_block::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<e2e_chain_block> res = make_tl_object<e2e_chain_block>();
  int32 var0;
  res->signature_ = TlFetchInt512::parse(p);
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->prev_block_hash_ = TlFetchInt256::parse(p);
  res->changes_ = TlFetchVector<TlFetchObject<e2e_chain_Change>>::parse(p);
  res->height_ = TlFetchInt::parse(p);
  res->state_proof_ = TlFetchBoxed<TlFetchObject<e2e_chain_stateProof>, -692684314>::parse(p);
  if (var0 & 1) { res->signature_public_key_ = TlFetchInt256::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void e2e_chain_block::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(prev_block_hash_, s);
  TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>::store(changes_, s);
  TlStoreBinary::store(height_, s);
  TlStoreBoxed<TlStoreObject, -692684314>::store(state_proof_, s);
  if (var0 & 1) { TlStoreBinary::store(signature_public_key_, s); }
}

void e2e_chain_block::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(prev_block_hash_, s);
  TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>::store(changes_, s);
  TlStoreBinary::store(height_, s);
  TlStoreBoxed<TlStoreObject, -692684314>::store(state_proof_, s);
  if (var0 & 1) { TlStoreBinary::store(signature_public_key_, s); }
}

void e2e_chain_block::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.block");
  int32 var0;
    s.store_field("signature", signature_);
    s.store_field("flags", (var0 = flags_));
    s.store_field("prev_block_hash", prev_block_hash_);
    { s.store_vector_begin("changes", changes_.size()); for (const auto &_value : changes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("height", height_);
    s.store_object_field("state_proof", static_cast<const BaseObject *>(state_proof_.get()));
    if (var0 & 1) { s.store_field("signature_public_key", signature_public_key_); }
    s.store_class_end();
  }
}

object_ptr<e2e_chain_Change> e2e_chain_Change::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_chain_changeNoop::ID:
      return e2e_chain_changeNoop::fetch(p);
    case e2e_chain_changeSetValue::ID:
      return e2e_chain_changeSetValue::fetch(p);
    case e2e_chain_changeSetGroupState::ID:
      return e2e_chain_changeSetGroupState::fetch(p);
    case e2e_chain_changeSetSharedKey::ID:
      return e2e_chain_changeSetSharedKey::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

e2e_chain_changeNoop::e2e_chain_changeNoop(UInt256 const &nonce_)
  : nonce_(nonce_)
{}

const std::int32_t e2e_chain_changeNoop::ID;

object_ptr<e2e_chain_Change> e2e_chain_changeNoop::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_changeNoop>(p);
}

e2e_chain_changeNoop::e2e_chain_changeNoop(TlParser &p)
  : nonce_(TlFetchInt256::parse(p))
{}

void e2e_chain_changeNoop::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
}

void e2e_chain_changeNoop::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
}

void e2e_chain_changeNoop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.changeNoop");
    s.store_field("nonce", nonce_);
    s.store_class_end();
  }
}

e2e_chain_changeSetValue::e2e_chain_changeSetValue(bytes const &key_, bytes const &value_)
  : key_(std::move(key_))
  , value_(std::move(value_))
{}

const std::int32_t e2e_chain_changeSetValue::ID;

object_ptr<e2e_chain_Change> e2e_chain_changeSetValue::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_changeSetValue>(p);
}

e2e_chain_changeSetValue::e2e_chain_changeSetValue(TlParser &p)
  : key_(TlFetchBytes<bytes>::parse(p))
  , value_(TlFetchBytes<bytes>::parse(p))
{}

void e2e_chain_changeSetValue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(key_, s);
  TlStoreString::store(value_, s);
}

void e2e_chain_changeSetValue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(key_, s);
  TlStoreString::store(value_, s);
}

void e2e_chain_changeSetValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.changeSetValue");
    s.store_bytes_field("key", key_);
    s.store_bytes_field("value", value_);
    s.store_class_end();
  }
}

e2e_chain_changeSetGroupState::e2e_chain_changeSetGroupState(object_ptr<e2e_chain_groupState> &&group_state_)
  : group_state_(std::move(group_state_))
{}

const std::int32_t e2e_chain_changeSetGroupState::ID;

object_ptr<e2e_chain_Change> e2e_chain_changeSetGroupState::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_changeSetGroupState>(p);
}

e2e_chain_changeSetGroupState::e2e_chain_changeSetGroupState(TlParser &p)
  : group_state_(TlFetchBoxed<TlFetchObject<e2e_chain_groupState>, 500987268>::parse(p))
{}

void e2e_chain_changeSetGroupState::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 500987268>::store(group_state_, s);
}

void e2e_chain_changeSetGroupState::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 500987268>::store(group_state_, s);
}

void e2e_chain_changeSetGroupState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.changeSetGroupState");
    s.store_object_field("group_state", static_cast<const BaseObject *>(group_state_.get()));
    s.store_class_end();
  }
}

e2e_chain_changeSetSharedKey::e2e_chain_changeSetSharedKey(object_ptr<e2e_chain_sharedKey> &&shared_key_)
  : shared_key_(std::move(shared_key_))
{}

const std::int32_t e2e_chain_changeSetSharedKey::ID;

object_ptr<e2e_chain_Change> e2e_chain_changeSetSharedKey::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_changeSetSharedKey>(p);
}

e2e_chain_changeSetSharedKey::e2e_chain_changeSetSharedKey(TlParser &p)
  : shared_key_(TlFetchBoxed<TlFetchObject<e2e_chain_sharedKey>, -1971028353>::parse(p))
{}

void e2e_chain_changeSetSharedKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -1971028353>::store(shared_key_, s);
}

void e2e_chain_changeSetSharedKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -1971028353>::store(shared_key_, s);
}

void e2e_chain_changeSetSharedKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.changeSetSharedKey");
    s.store_object_field("shared_key", static_cast<const BaseObject *>(shared_key_.get()));
    s.store_class_end();
  }
}

object_ptr<e2e_chain_GroupBroadcast> e2e_chain_GroupBroadcast::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case e2e_chain_groupBroadcastNonceCommit::ID:
      return e2e_chain_groupBroadcastNonceCommit::fetch(p);
    case e2e_chain_groupBroadcastNonceReveal::ID:
      return e2e_chain_groupBroadcastNonceReveal::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

e2e_chain_groupBroadcastNonceCommit::e2e_chain_groupBroadcastNonceCommit(UInt512 const &signature_, int64 user_id_, int32 chain_height_, UInt256 const &chain_hash_, UInt256 const &nonce_hash_)
  : signature_(signature_)
  , user_id_(user_id_)
  , chain_height_(chain_height_)
  , chain_hash_(chain_hash_)
  , nonce_hash_(nonce_hash_)
{}

const std::int32_t e2e_chain_groupBroadcastNonceCommit::ID;

object_ptr<e2e_chain_GroupBroadcast> e2e_chain_groupBroadcastNonceCommit::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_groupBroadcastNonceCommit>(p);
}

e2e_chain_groupBroadcastNonceCommit::e2e_chain_groupBroadcastNonceCommit(TlParser &p)
  : signature_(TlFetchInt512::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , chain_height_(TlFetchInt::parse(p))
  , chain_hash_(TlFetchInt256::parse(p))
  , nonce_hash_(TlFetchInt256::parse(p))
{}

void e2e_chain_groupBroadcastNonceCommit::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(chain_height_, s);
  TlStoreBinary::store(chain_hash_, s);
  TlStoreBinary::store(nonce_hash_, s);
}

void e2e_chain_groupBroadcastNonceCommit::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(chain_height_, s);
  TlStoreBinary::store(chain_hash_, s);
  TlStoreBinary::store(nonce_hash_, s);
}

void e2e_chain_groupBroadcastNonceCommit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.groupBroadcastNonceCommit");
    s.store_field("signature", signature_);
    s.store_field("user_id", user_id_);
    s.store_field("chain_height", chain_height_);
    s.store_field("chain_hash", chain_hash_);
    s.store_field("nonce_hash", nonce_hash_);
    s.store_class_end();
  }
}

e2e_chain_groupBroadcastNonceReveal::e2e_chain_groupBroadcastNonceReveal(UInt512 const &signature_, int64 user_id_, int32 chain_height_, UInt256 const &chain_hash_, UInt256 const &nonce_)
  : signature_(signature_)
  , user_id_(user_id_)
  , chain_height_(chain_height_)
  , chain_hash_(chain_hash_)
  , nonce_(nonce_)
{}

const std::int32_t e2e_chain_groupBroadcastNonceReveal::ID;

object_ptr<e2e_chain_GroupBroadcast> e2e_chain_groupBroadcastNonceReveal::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_groupBroadcastNonceReveal>(p);
}

e2e_chain_groupBroadcastNonceReveal::e2e_chain_groupBroadcastNonceReveal(TlParser &p)
  : signature_(TlFetchInt512::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , chain_height_(TlFetchInt::parse(p))
  , chain_hash_(TlFetchInt256::parse(p))
  , nonce_(TlFetchInt256::parse(p))
{}

void e2e_chain_groupBroadcastNonceReveal::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(chain_height_, s);
  TlStoreBinary::store(chain_hash_, s);
  TlStoreBinary::store(nonce_, s);
}

void e2e_chain_groupBroadcastNonceReveal::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(signature_, s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(chain_height_, s);
  TlStoreBinary::store(chain_hash_, s);
  TlStoreBinary::store(nonce_, s);
}

void e2e_chain_groupBroadcastNonceReveal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.groupBroadcastNonceReveal");
    s.store_field("signature", signature_);
    s.store_field("user_id", user_id_);
    s.store_field("chain_height", chain_height_);
    s.store_field("chain_hash", chain_hash_);
    s.store_field("nonce", nonce_);
    s.store_class_end();
  }
}

e2e_chain_groupParticipant::e2e_chain_groupParticipant()
  : user_id_()
  , public_key_()
  , flags_()
  , add_users_()
  , remove_users_()
  , version_()
{}

e2e_chain_groupParticipant::e2e_chain_groupParticipant(int64 user_id_, UInt256 const &public_key_, int32 flags_, bool add_users_, bool remove_users_, int32 version_)
  : user_id_(user_id_)
  , public_key_(public_key_)
  , flags_(flags_)
  , add_users_(add_users_)
  , remove_users_(remove_users_)
  , version_(version_)
{}

const std::int32_t e2e_chain_groupParticipant::ID;

object_ptr<e2e_chain_groupParticipant> e2e_chain_groupParticipant::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<e2e_chain_groupParticipant> res = make_tl_object<e2e_chain_groupParticipant>();
  int32 var0;
  res->user_id_ = TlFetchLong::parse(p);
  res->public_key_ = TlFetchInt256::parse(p);
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->add_users_ = (var0 & 1) != 0;
  res->remove_users_ = (var0 & 2) != 0;
  res->version_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void e2e_chain_groupParticipant::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(public_key_, s);
  TlStoreBinary::store((var0 = flags_ | (add_users_ << 0) | (remove_users_ << 1)), s);
  TlStoreBinary::store(version_, s);
}

void e2e_chain_groupParticipant::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(public_key_, s);
  TlStoreBinary::store((var0 = flags_ | (add_users_ << 0) | (remove_users_ << 1)), s);
  TlStoreBinary::store(version_, s);
}

void e2e_chain_groupParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.groupParticipant");
  int32 var0;
    s.store_field("user_id", user_id_);
    s.store_field("public_key", public_key_);
    s.store_field("flags", (var0 = flags_ | (add_users_ << 0) | (remove_users_ << 1)));
    if (var0 & 1) { s.store_field("add_users", true); }
    if (var0 & 2) { s.store_field("remove_users", true); }
    s.store_field("version", version_);
    s.store_class_end();
  }
}

e2e_chain_groupState::e2e_chain_groupState(array<object_ptr<e2e_chain_groupParticipant>> &&participants_, int32 external_permissions_)
  : participants_(std::move(participants_))
  , external_permissions_(external_permissions_)
{}

const std::int32_t e2e_chain_groupState::ID;

object_ptr<e2e_chain_groupState> e2e_chain_groupState::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_groupState>(p);
}

e2e_chain_groupState::e2e_chain_groupState(TlParser &p)
  : participants_(TlFetchVector<TlFetchBoxed<TlFetchObject<e2e_chain_groupParticipant>, 418617119>>::parse(p))
  , external_permissions_(TlFetchInt::parse(p))
{}

void e2e_chain_groupState::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreVector<TlStoreBoxed<TlStoreObject, 418617119>>::store(participants_, s);
  TlStoreBinary::store(external_permissions_, s);
}

void e2e_chain_groupState::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreVector<TlStoreBoxed<TlStoreObject, 418617119>>::store(participants_, s);
  TlStoreBinary::store(external_permissions_, s);
}

void e2e_chain_groupState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.groupState");
    { s.store_vector_begin("participants", participants_.size()); for (const auto &_value : participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("external_permissions", external_permissions_);
    s.store_class_end();
  }
}

e2e_chain_sharedKey::e2e_chain_sharedKey(UInt256 const &ek_, string const &encrypted_shared_key_, array<int64> &&dest_user_id_, array<bytes> &&dest_header_)
  : ek_(ek_)
  , encrypted_shared_key_(encrypted_shared_key_)
  , dest_user_id_(std::move(dest_user_id_))
  , dest_header_(std::move(dest_header_))
{}

const std::int32_t e2e_chain_sharedKey::ID;

object_ptr<e2e_chain_sharedKey> e2e_chain_sharedKey::fetch(TlParser &p) {
  return make_tl_object<e2e_chain_sharedKey>(p);
}

e2e_chain_sharedKey::e2e_chain_sharedKey(TlParser &p)
  : ek_(TlFetchInt256::parse(p))
  , encrypted_shared_key_(TlFetchString<string>::parse(p))
  , dest_user_id_(TlFetchVector<TlFetchLong>::parse(p))
  , dest_header_(TlFetchVector<TlFetchBytes<bytes>>::parse(p))
{}

void e2e_chain_sharedKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(ek_, s);
  TlStoreString::store(encrypted_shared_key_, s);
  TlStoreVector<TlStoreBinary>::store(dest_user_id_, s);
  TlStoreVector<TlStoreString>::store(dest_header_, s);
}

void e2e_chain_sharedKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(ek_, s);
  TlStoreString::store(encrypted_shared_key_, s);
  TlStoreVector<TlStoreBinary>::store(dest_user_id_, s);
  TlStoreVector<TlStoreString>::store(dest_header_, s);
}

void e2e_chain_sharedKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.sharedKey");
    s.store_field("ek", ek_);
    s.store_field("encrypted_shared_key", encrypted_shared_key_);
    { s.store_vector_begin("dest_user_id", dest_user_id_.size()); for (const auto &_value : dest_user_id_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("dest_header", dest_header_.size()); for (const auto &_value : dest_header_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

e2e_chain_stateProof::e2e_chain_stateProof()
  : flags_()
  , kv_hash_()
  , group_state_()
  , shared_key_()
{}

e2e_chain_stateProof::e2e_chain_stateProof(int32 flags_, UInt256 const &kv_hash_, object_ptr<e2e_chain_groupState> &&group_state_, object_ptr<e2e_chain_sharedKey> &&shared_key_)
  : flags_(flags_)
  , kv_hash_(kv_hash_)
  , group_state_(std::move(group_state_))
  , shared_key_(std::move(shared_key_))
{}

const std::int32_t e2e_chain_stateProof::ID;

object_ptr<e2e_chain_stateProof> e2e_chain_stateProof::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<e2e_chain_stateProof> res = make_tl_object<e2e_chain_stateProof>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->kv_hash_ = TlFetchInt256::parse(p);
  if (var0 & 1) { res->group_state_ = TlFetchBoxed<TlFetchObject<e2e_chain_groupState>, 500987268>::parse(p); }
  if (var0 & 2) { res->shared_key_ = TlFetchBoxed<TlFetchObject<e2e_chain_sharedKey>, -1971028353>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void e2e_chain_stateProof::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(kv_hash_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 500987268>::store(group_state_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, -1971028353>::store(shared_key_, s); }
}

void e2e_chain_stateProof::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(kv_hash_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 500987268>::store(group_state_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, -1971028353>::store(shared_key_, s); }
}

void e2e_chain_stateProof::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.chain.stateProof");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("kv_hash", kv_hash_);
    if (var0 & 1) { s.store_object_field("group_state", static_cast<const BaseObject *>(group_state_.get())); }
    if (var0 & 2) { s.store_object_field("shared_key", static_cast<const BaseObject *>(shared_key_.get())); }
    s.store_class_end();
  }
}

const std::int32_t e2e_nop::ID;

object_ptr<e2e_nop> e2e_nop::fetch(TlParser &p) {
  return make_tl_object<e2e_nop>();
}

void e2e_nop::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1479594067);
}

void e2e_nop::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1479594067);
}

void e2e_nop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "e2e.nop");
    s.store_class_end();
  }
}

e2e_nop::ReturnType e2e_nop::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}
}  // namespace e2e_api
}  // namespace td
