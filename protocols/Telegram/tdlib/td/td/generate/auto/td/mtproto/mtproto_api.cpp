#include "mtproto_api.h"

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
namespace mtproto_api {

std::string to_string(const BaseObject &value) {
  TlStorerToString storer;
  value.store(storer, "");
  return storer.move_as_string();
}

object_ptr<Object> Object::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case bad_msg_notification::ID:
      return bad_msg_notification::fetch(p);
    case bad_server_salt::ID:
      return bad_server_salt::fetch(p);
    case bind_auth_key_inner::ID:
      return bind_auth_key_inner::fetch(p);
    case client_DH_inner_data::ID:
      return client_DH_inner_data::fetch(p);
    case destroy_auth_key_ok::ID:
      return destroy_auth_key_ok::fetch(p);
    case destroy_auth_key_none::ID:
      return destroy_auth_key_none::fetch(p);
    case destroy_auth_key_fail::ID:
      return destroy_auth_key_fail::fetch(p);
    case future_salt::ID:
      return future_salt::fetch(p);
    case future_salts::ID:
      return future_salts::fetch(p);
    case gzip_packed::ID:
      return gzip_packed::fetch(p);
    case dummyHttpWait::ID:
      return dummyHttpWait::fetch(p);
    case msg_detailed_info::ID:
      return msg_detailed_info::fetch(p);
    case msg_new_detailed_info::ID:
      return msg_new_detailed_info::fetch(p);
    case msg_resend_req::ID:
      return msg_resend_req::fetch(p);
    case msgs_ack::ID:
      return msgs_ack::fetch(p);
    case msgs_all_info::ID:
      return msgs_all_info::fetch(p);
    case msgs_state_info::ID:
      return msgs_state_info::fetch(p);
    case msgs_state_req::ID:
      return msgs_state_req::fetch(p);
    case new_session_created::ID:
      return new_session_created::fetch(p);
    case p_q_inner_data_dc::ID:
      return p_q_inner_data_dc::fetch(p);
    case p_q_inner_data_temp_dc::ID:
      return p_q_inner_data_temp_dc::fetch(p);
    case pong::ID:
      return pong::fetch(p);
    case rsa_public_key::ID:
      return rsa_public_key::fetch(p);
    case resPQ::ID:
      return resPQ::fetch(p);
    case rpc_answer_unknown::ID:
      return rpc_answer_unknown::fetch(p);
    case rpc_answer_dropped_running::ID:
      return rpc_answer_dropped_running::fetch(p);
    case rpc_answer_dropped::ID:
      return rpc_answer_dropped::fetch(p);
    case rpc_error::ID:
      return rpc_error::fetch(p);
    case server_DH_params_ok::ID:
      return server_DH_params_ok::fetch(p);
    case server_DH_inner_data::ID:
      return server_DH_inner_data::fetch(p);
    case dh_gen_ok::ID:
      return dh_gen_ok::fetch(p);
    case dh_gen_retry::ID:
      return dh_gen_retry::fetch(p);
    case dh_gen_fail::ID:
      return dh_gen_fail::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

object_ptr<Function> Function::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case destroy_auth_key::ID:
      return destroy_auth_key::fetch(p);
    case get_future_salts::ID:
      return get_future_salts::fetch(p);
    case http_wait::ID:
      return http_wait::fetch(p);
    case ping_delay_disconnect::ID:
      return ping_delay_disconnect::fetch(p);
    case req_DH_params::ID:
      return req_DH_params::fetch(p);
    case req_pq_multi::ID:
      return req_pq_multi::fetch(p);
    case rpc_drop_answer::ID:
      return rpc_drop_answer::fetch(p);
    case set_client_DH_params::ID:
      return set_client_DH_params::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

object_ptr<BadMsgNotification> BadMsgNotification::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case bad_msg_notification::ID:
      return bad_msg_notification::fetch(p);
    case bad_server_salt::ID:
      return bad_server_salt::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

bad_msg_notification::bad_msg_notification(int64 bad_msg_id_, int32 bad_msg_seqno_, int32 error_code_)
  : bad_msg_id_(bad_msg_id_)
  , bad_msg_seqno_(bad_msg_seqno_)
  , error_code_(error_code_)
{}

const std::int32_t bad_msg_notification::ID;

object_ptr<BadMsgNotification> bad_msg_notification::fetch(TlParser &p) {
  return make_tl_object<bad_msg_notification>(p);
}

bad_msg_notification::bad_msg_notification(TlParser &p)
  : bad_msg_id_(TlFetchLong::parse(p))
  , bad_msg_seqno_(TlFetchInt::parse(p))
  , error_code_(TlFetchInt::parse(p))
{}

void bad_msg_notification::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(bad_msg_id_, s);
  TlStoreBinary::store(bad_msg_seqno_, s);
  TlStoreBinary::store(error_code_, s);
}

void bad_msg_notification::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(bad_msg_id_, s);
  TlStoreBinary::store(bad_msg_seqno_, s);
  TlStoreBinary::store(error_code_, s);
}

void bad_msg_notification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bad_msg_notification");
    s.store_field("bad_msg_id", bad_msg_id_);
    s.store_field("bad_msg_seqno", bad_msg_seqno_);
    s.store_field("error_code", error_code_);
    s.store_class_end();
  }
}

bad_server_salt::bad_server_salt(int64 bad_msg_id_, int32 bad_msg_seqno_, int32 error_code_, int64 new_server_salt_)
  : bad_msg_id_(bad_msg_id_)
  , bad_msg_seqno_(bad_msg_seqno_)
  , error_code_(error_code_)
  , new_server_salt_(new_server_salt_)
{}

const std::int32_t bad_server_salt::ID;

object_ptr<BadMsgNotification> bad_server_salt::fetch(TlParser &p) {
  return make_tl_object<bad_server_salt>(p);
}

bad_server_salt::bad_server_salt(TlParser &p)
  : bad_msg_id_(TlFetchLong::parse(p))
  , bad_msg_seqno_(TlFetchInt::parse(p))
  , error_code_(TlFetchInt::parse(p))
  , new_server_salt_(TlFetchLong::parse(p))
{}

void bad_server_salt::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(bad_msg_id_, s);
  TlStoreBinary::store(bad_msg_seqno_, s);
  TlStoreBinary::store(error_code_, s);
  TlStoreBinary::store(new_server_salt_, s);
}

void bad_server_salt::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(bad_msg_id_, s);
  TlStoreBinary::store(bad_msg_seqno_, s);
  TlStoreBinary::store(error_code_, s);
  TlStoreBinary::store(new_server_salt_, s);
}

void bad_server_salt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bad_server_salt");
    s.store_field("bad_msg_id", bad_msg_id_);
    s.store_field("bad_msg_seqno", bad_msg_seqno_);
    s.store_field("error_code", error_code_);
    s.store_field("new_server_salt", new_server_salt_);
    s.store_class_end();
  }
}

bind_auth_key_inner::bind_auth_key_inner(int64 nonce_, int64 temp_auth_key_id_, int64 perm_auth_key_id_, int64 temp_session_id_, int32 expires_at_)
  : nonce_(nonce_)
  , temp_auth_key_id_(temp_auth_key_id_)
  , perm_auth_key_id_(perm_auth_key_id_)
  , temp_session_id_(temp_session_id_)
  , expires_at_(expires_at_)
{}

const std::int32_t bind_auth_key_inner::ID;

object_ptr<bind_auth_key_inner> bind_auth_key_inner::fetch(TlParser &p) {
  return make_tl_object<bind_auth_key_inner>(p);
}

bind_auth_key_inner::bind_auth_key_inner(TlParser &p)
  : nonce_(TlFetchLong::parse(p))
  , temp_auth_key_id_(TlFetchLong::parse(p))
  , perm_auth_key_id_(TlFetchLong::parse(p))
  , temp_session_id_(TlFetchLong::parse(p))
  , expires_at_(TlFetchInt::parse(p))
{}

void bind_auth_key_inner::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(temp_auth_key_id_, s);
  TlStoreBinary::store(perm_auth_key_id_, s);
  TlStoreBinary::store(temp_session_id_, s);
  TlStoreBinary::store(expires_at_, s);
}

void bind_auth_key_inner::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(temp_auth_key_id_, s);
  TlStoreBinary::store(perm_auth_key_id_, s);
  TlStoreBinary::store(temp_session_id_, s);
  TlStoreBinary::store(expires_at_, s);
}

void bind_auth_key_inner::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bind_auth_key_inner");
    s.store_field("nonce", nonce_);
    s.store_field("temp_auth_key_id", temp_auth_key_id_);
    s.store_field("perm_auth_key_id", perm_auth_key_id_);
    s.store_field("temp_session_id", temp_session_id_);
    s.store_field("expires_at", expires_at_);
    s.store_class_end();
  }
}

client_DH_inner_data::client_DH_inner_data(UInt128 const &nonce_, UInt128 const &server_nonce_, int64 retry_id_, string const &g_b_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , retry_id_(retry_id_)
  , g_b_(g_b_)
{}

const std::int32_t client_DH_inner_data::ID;

object_ptr<client_DH_inner_data> client_DH_inner_data::fetch(TlParser &p) {
  return make_tl_object<client_DH_inner_data>(p);
}

client_DH_inner_data::client_DH_inner_data(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , retry_id_(TlFetchLong::parse(p))
  , g_b_(TlFetchString<string>::parse(p))
{}

void client_DH_inner_data::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(retry_id_, s);
  TlStoreString::store(g_b_, s);
}

void client_DH_inner_data::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(retry_id_, s);
  TlStoreString::store(g_b_, s);
}

void client_DH_inner_data::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "client_DH_inner_data");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("retry_id", retry_id_);
    s.store_field("g_b", g_b_);
    s.store_class_end();
  }
}

object_ptr<DestroyAuthKeyRes> DestroyAuthKeyRes::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case destroy_auth_key_ok::ID:
      return destroy_auth_key_ok::fetch(p);
    case destroy_auth_key_none::ID:
      return destroy_auth_key_none::fetch(p);
    case destroy_auth_key_fail::ID:
      return destroy_auth_key_fail::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t destroy_auth_key_ok::ID;

object_ptr<DestroyAuthKeyRes> destroy_auth_key_ok::fetch(TlParser &p) {
  return make_tl_object<destroy_auth_key_ok>();
}

void destroy_auth_key_ok::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void destroy_auth_key_ok::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void destroy_auth_key_ok::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "destroy_auth_key_ok");
    s.store_class_end();
  }
}

const std::int32_t destroy_auth_key_none::ID;

object_ptr<DestroyAuthKeyRes> destroy_auth_key_none::fetch(TlParser &p) {
  return make_tl_object<destroy_auth_key_none>();
}

void destroy_auth_key_none::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void destroy_auth_key_none::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void destroy_auth_key_none::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "destroy_auth_key_none");
    s.store_class_end();
  }
}

const std::int32_t destroy_auth_key_fail::ID;

object_ptr<DestroyAuthKeyRes> destroy_auth_key_fail::fetch(TlParser &p) {
  return make_tl_object<destroy_auth_key_fail>();
}

void destroy_auth_key_fail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void destroy_auth_key_fail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void destroy_auth_key_fail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "destroy_auth_key_fail");
    s.store_class_end();
  }
}

future_salt::future_salt(int32 valid_since_, int32 valid_until_, int64 salt_)
  : valid_since_(valid_since_)
  , valid_until_(valid_until_)
  , salt_(salt_)
{}

const std::int32_t future_salt::ID;

object_ptr<future_salt> future_salt::fetch(TlParser &p) {
  return make_tl_object<future_salt>(p);
}

future_salt::future_salt(TlParser &p)
  : valid_since_(TlFetchInt::parse(p))
  , valid_until_(TlFetchInt::parse(p))
  , salt_(TlFetchLong::parse(p))
{}

void future_salt::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(valid_since_, s);
  TlStoreBinary::store(valid_until_, s);
  TlStoreBinary::store(salt_, s);
}

void future_salt::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(valid_since_, s);
  TlStoreBinary::store(valid_until_, s);
  TlStoreBinary::store(salt_, s);
}

void future_salt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "future_salt");
    s.store_field("valid_since", valid_since_);
    s.store_field("valid_until", valid_until_);
    s.store_field("salt", salt_);
    s.store_class_end();
  }
}

future_salts::future_salts(int64 req_msg_id_, int32 now_, array<object_ptr<future_salt>> &&salts_)
  : req_msg_id_(req_msg_id_)
  , now_(now_)
  , salts_(std::move(salts_))
{}

const std::int32_t future_salts::ID;

object_ptr<future_salts> future_salts::fetch(TlParser &p) {
  return make_tl_object<future_salts>(p);
}

future_salts::future_salts(TlParser &p)
  : req_msg_id_(TlFetchLong::parse(p))
  , now_(TlFetchInt::parse(p))
  , salts_(TlFetchVector<TlFetchObject<future_salt>>::parse(p))
{}

void future_salts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(req_msg_id_, s);
  TlStoreBinary::store(now_, s);
  TlStoreVector<TlStoreObject>::store(salts_, s);
}

void future_salts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(req_msg_id_, s);
  TlStoreBinary::store(now_, s);
  TlStoreVector<TlStoreObject>::store(salts_, s);
}

void future_salts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "future_salts");
    s.store_field("req_msg_id", req_msg_id_);
    s.store_field("now", now_);
    { s.store_vector_begin("salts", salts_.size()); for (const auto &_value : salts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

gzip_packed::gzip_packed(string const &packed_data_)
  : packed_data_(packed_data_)
{}

const std::int32_t gzip_packed::ID;

object_ptr<gzip_packed> gzip_packed::fetch(TlParser &p) {
  return make_tl_object<gzip_packed>(p);
}

gzip_packed::gzip_packed(TlParser &p)
  : packed_data_(TlFetchString<string>::parse(p))
{}

void gzip_packed::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(packed_data_, s);
}

void gzip_packed::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(packed_data_, s);
}

void gzip_packed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "gzip_packed");
    s.store_field("packed_data", packed_data_);
    s.store_class_end();
  }
}

const std::int32_t dummyHttpWait::ID;

object_ptr<dummyHttpWait> dummyHttpWait::fetch(TlParser &p) {
  return make_tl_object<dummyHttpWait>();
}

void dummyHttpWait::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void dummyHttpWait::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void dummyHttpWait::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dummyHttpWait");
    s.store_class_end();
  }
}

object_ptr<MsgDetailedInfo> MsgDetailedInfo::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case msg_detailed_info::ID:
      return msg_detailed_info::fetch(p);
    case msg_new_detailed_info::ID:
      return msg_new_detailed_info::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

msg_detailed_info::msg_detailed_info(int64 msg_id_, int64 answer_msg_id_, int32 bytes_, int32 status_)
  : msg_id_(msg_id_)
  , answer_msg_id_(answer_msg_id_)
  , bytes_(bytes_)
  , status_(status_)
{}

const std::int32_t msg_detailed_info::ID;

object_ptr<MsgDetailedInfo> msg_detailed_info::fetch(TlParser &p) {
  return make_tl_object<msg_detailed_info>(p);
}

msg_detailed_info::msg_detailed_info(TlParser &p)
  : msg_id_(TlFetchLong::parse(p))
  , answer_msg_id_(TlFetchLong::parse(p))
  , bytes_(TlFetchInt::parse(p))
  , status_(TlFetchInt::parse(p))
{}

void msg_detailed_info::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(answer_msg_id_, s);
  TlStoreBinary::store(bytes_, s);
  TlStoreBinary::store(status_, s);
}

void msg_detailed_info::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(answer_msg_id_, s);
  TlStoreBinary::store(bytes_, s);
  TlStoreBinary::store(status_, s);
}

void msg_detailed_info::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "msg_detailed_info");
    s.store_field("msg_id", msg_id_);
    s.store_field("answer_msg_id", answer_msg_id_);
    s.store_field("bytes", bytes_);
    s.store_field("status", status_);
    s.store_class_end();
  }
}

msg_new_detailed_info::msg_new_detailed_info(int64 answer_msg_id_, int32 bytes_, int32 status_)
  : answer_msg_id_(answer_msg_id_)
  , bytes_(bytes_)
  , status_(status_)
{}

const std::int32_t msg_new_detailed_info::ID;

object_ptr<MsgDetailedInfo> msg_new_detailed_info::fetch(TlParser &p) {
  return make_tl_object<msg_new_detailed_info>(p);
}

msg_new_detailed_info::msg_new_detailed_info(TlParser &p)
  : answer_msg_id_(TlFetchLong::parse(p))
  , bytes_(TlFetchInt::parse(p))
  , status_(TlFetchInt::parse(p))
{}

void msg_new_detailed_info::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(answer_msg_id_, s);
  TlStoreBinary::store(bytes_, s);
  TlStoreBinary::store(status_, s);
}

void msg_new_detailed_info::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(answer_msg_id_, s);
  TlStoreBinary::store(bytes_, s);
  TlStoreBinary::store(status_, s);
}

void msg_new_detailed_info::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "msg_new_detailed_info");
    s.store_field("answer_msg_id", answer_msg_id_);
    s.store_field("bytes", bytes_);
    s.store_field("status", status_);
    s.store_class_end();
  }
}

msg_resend_req::msg_resend_req(array<int64> &&msg_ids_)
  : msg_ids_(std::move(msg_ids_))
{}

const std::int32_t msg_resend_req::ID;

object_ptr<msg_resend_req> msg_resend_req::fetch(TlParser &p) {
  return make_tl_object<msg_resend_req>(p);
}

msg_resend_req::msg_resend_req(TlParser &p)
  : msg_ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void msg_resend_req::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
}

void msg_resend_req::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
}

void msg_resend_req::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "msg_resend_req");
    { s.store_vector_begin("msg_ids", msg_ids_.size()); for (const auto &_value : msg_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

msgs_ack::msgs_ack(array<int64> &&msg_ids_)
  : msg_ids_(std::move(msg_ids_))
{}

const std::int32_t msgs_ack::ID;

object_ptr<msgs_ack> msgs_ack::fetch(TlParser &p) {
  return make_tl_object<msgs_ack>(p);
}

msgs_ack::msgs_ack(TlParser &p)
  : msg_ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void msgs_ack::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
}

void msgs_ack::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
}

void msgs_ack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "msgs_ack");
    { s.store_vector_begin("msg_ids", msg_ids_.size()); for (const auto &_value : msg_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

msgs_all_info::msgs_all_info(array<int64> &&msg_ids_, string const &info_)
  : msg_ids_(std::move(msg_ids_))
  , info_(info_)
{}

const std::int32_t msgs_all_info::ID;

object_ptr<msgs_all_info> msgs_all_info::fetch(TlParser &p) {
  return make_tl_object<msgs_all_info>(p);
}

msgs_all_info::msgs_all_info(TlParser &p)
  : msg_ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
  , info_(TlFetchString<string>::parse(p))
{}

void msgs_all_info::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
  TlStoreString::store(info_, s);
}

void msgs_all_info::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
  TlStoreString::store(info_, s);
}

void msgs_all_info::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "msgs_all_info");
    { s.store_vector_begin("msg_ids", msg_ids_.size()); for (const auto &_value : msg_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("info", info_);
    s.store_class_end();
  }
}

msgs_state_info::msgs_state_info(int64 req_msg_id_, string const &info_)
  : req_msg_id_(req_msg_id_)
  , info_(info_)
{}

const std::int32_t msgs_state_info::ID;

object_ptr<msgs_state_info> msgs_state_info::fetch(TlParser &p) {
  return make_tl_object<msgs_state_info>(p);
}

msgs_state_info::msgs_state_info(TlParser &p)
  : req_msg_id_(TlFetchLong::parse(p))
  , info_(TlFetchString<string>::parse(p))
{}

void msgs_state_info::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(req_msg_id_, s);
  TlStoreString::store(info_, s);
}

void msgs_state_info::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(req_msg_id_, s);
  TlStoreString::store(info_, s);
}

void msgs_state_info::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "msgs_state_info");
    s.store_field("req_msg_id", req_msg_id_);
    s.store_field("info", info_);
    s.store_class_end();
  }
}

msgs_state_req::msgs_state_req(array<int64> &&msg_ids_)
  : msg_ids_(std::move(msg_ids_))
{}

const std::int32_t msgs_state_req::ID;

object_ptr<msgs_state_req> msgs_state_req::fetch(TlParser &p) {
  return make_tl_object<msgs_state_req>(p);
}

msgs_state_req::msgs_state_req(TlParser &p)
  : msg_ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void msgs_state_req::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
}

void msgs_state_req::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_ids_, s);
}

void msgs_state_req::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "msgs_state_req");
    { s.store_vector_begin("msg_ids", msg_ids_.size()); for (const auto &_value : msg_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

new_session_created::new_session_created(int64 first_msg_id_, int64 unique_id_, int64 server_salt_)
  : first_msg_id_(first_msg_id_)
  , unique_id_(unique_id_)
  , server_salt_(server_salt_)
{}

const std::int32_t new_session_created::ID;

object_ptr<new_session_created> new_session_created::fetch(TlParser &p) {
  return make_tl_object<new_session_created>(p);
}

new_session_created::new_session_created(TlParser &p)
  : first_msg_id_(TlFetchLong::parse(p))
  , unique_id_(TlFetchLong::parse(p))
  , server_salt_(TlFetchLong::parse(p))
{}

void new_session_created::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(first_msg_id_, s);
  TlStoreBinary::store(unique_id_, s);
  TlStoreBinary::store(server_salt_, s);
}

void new_session_created::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(first_msg_id_, s);
  TlStoreBinary::store(unique_id_, s);
  TlStoreBinary::store(server_salt_, s);
}

void new_session_created::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "new_session_created");
    s.store_field("first_msg_id", first_msg_id_);
    s.store_field("unique_id", unique_id_);
    s.store_field("server_salt", server_salt_);
    s.store_class_end();
  }
}

object_ptr<P_Q_inner_data> P_Q_inner_data::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case p_q_inner_data_dc::ID:
      return p_q_inner_data_dc::fetch(p);
    case p_q_inner_data_temp_dc::ID:
      return p_q_inner_data_temp_dc::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

p_q_inner_data_dc::p_q_inner_data_dc(string const &pq_, string const &p_, string const &q_, UInt128 const &nonce_, UInt128 const &server_nonce_, UInt256 const &new_nonce_, int32 dc_)
  : pq_(pq_)
  , p_(p_)
  , q_(q_)
  , nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , new_nonce_(new_nonce_)
  , dc_(dc_)
{}

const std::int32_t p_q_inner_data_dc::ID;

object_ptr<P_Q_inner_data> p_q_inner_data_dc::fetch(TlParser &p) {
  return make_tl_object<p_q_inner_data_dc>(p);
}

p_q_inner_data_dc::p_q_inner_data_dc(TlParser &p)
  : pq_(TlFetchString<string>::parse(p))
  , p_(TlFetchString<string>::parse(p))
  , q_(TlFetchString<string>::parse(p))
  , nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , new_nonce_(TlFetchInt256::parse(p))
  , dc_(TlFetchInt::parse(p))
{}

void p_q_inner_data_dc::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(pq_, s);
  TlStoreString::store(p_, s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_, s);
  TlStoreBinary::store(dc_, s);
}

void p_q_inner_data_dc::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(pq_, s);
  TlStoreString::store(p_, s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_, s);
  TlStoreBinary::store(dc_, s);
}

void p_q_inner_data_dc::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "p_q_inner_data_dc");
    s.store_field("pq", pq_);
    s.store_field("p", p_);
    s.store_field("q", q_);
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("new_nonce", new_nonce_);
    s.store_field("dc", dc_);
    s.store_class_end();
  }
}

p_q_inner_data_temp_dc::p_q_inner_data_temp_dc(string const &pq_, string const &p_, string const &q_, UInt128 const &nonce_, UInt128 const &server_nonce_, UInt256 const &new_nonce_, int32 dc_, int32 expires_in_)
  : pq_(pq_)
  , p_(p_)
  , q_(q_)
  , nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , new_nonce_(new_nonce_)
  , dc_(dc_)
  , expires_in_(expires_in_)
{}

const std::int32_t p_q_inner_data_temp_dc::ID;

object_ptr<P_Q_inner_data> p_q_inner_data_temp_dc::fetch(TlParser &p) {
  return make_tl_object<p_q_inner_data_temp_dc>(p);
}

p_q_inner_data_temp_dc::p_q_inner_data_temp_dc(TlParser &p)
  : pq_(TlFetchString<string>::parse(p))
  , p_(TlFetchString<string>::parse(p))
  , q_(TlFetchString<string>::parse(p))
  , nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , new_nonce_(TlFetchInt256::parse(p))
  , dc_(TlFetchInt::parse(p))
  , expires_in_(TlFetchInt::parse(p))
{}

void p_q_inner_data_temp_dc::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(pq_, s);
  TlStoreString::store(p_, s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_, s);
  TlStoreBinary::store(dc_, s);
  TlStoreBinary::store(expires_in_, s);
}

void p_q_inner_data_temp_dc::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(pq_, s);
  TlStoreString::store(p_, s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_, s);
  TlStoreBinary::store(dc_, s);
  TlStoreBinary::store(expires_in_, s);
}

void p_q_inner_data_temp_dc::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "p_q_inner_data_temp_dc");
    s.store_field("pq", pq_);
    s.store_field("p", p_);
    s.store_field("q", q_);
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("new_nonce", new_nonce_);
    s.store_field("dc", dc_);
    s.store_field("expires_in", expires_in_);
    s.store_class_end();
  }
}

pong::pong(int64 msg_id_, int64 ping_id_)
  : msg_id_(msg_id_)
  , ping_id_(ping_id_)
{}

const std::int32_t pong::ID;

object_ptr<pong> pong::fetch(TlParser &p) {
  return make_tl_object<pong>(p);
}

pong::pong(TlParser &p)
  : msg_id_(TlFetchLong::parse(p))
  , ping_id_(TlFetchLong::parse(p))
{}

void pong::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(ping_id_, s);
}

void pong::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(ping_id_, s);
}

void pong::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pong");
    s.store_field("msg_id", msg_id_);
    s.store_field("ping_id", ping_id_);
    s.store_class_end();
  }
}

rsa_public_key::rsa_public_key(string const &n_, string const &e_)
  : n_(n_)
  , e_(e_)
{}

const std::int32_t rsa_public_key::ID;

object_ptr<rsa_public_key> rsa_public_key::fetch(TlParser &p) {
  return make_tl_object<rsa_public_key>(p);
}

rsa_public_key::rsa_public_key(TlParser &p)
  : n_(TlFetchString<string>::parse(p))
  , e_(TlFetchString<string>::parse(p))
{}

void rsa_public_key::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(n_, s);
  TlStoreString::store(e_, s);
}

void rsa_public_key::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(n_, s);
  TlStoreString::store(e_, s);
}

void rsa_public_key::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rsa_public_key");
    s.store_field("n", n_);
    s.store_field("e", e_);
    s.store_class_end();
  }
}

resPQ::resPQ(UInt128 const &nonce_, UInt128 const &server_nonce_, string const &pq_, array<int64> &&server_public_key_fingerprints_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , pq_(pq_)
  , server_public_key_fingerprints_(std::move(server_public_key_fingerprints_))
{}

const std::int32_t resPQ::ID;

object_ptr<resPQ> resPQ::fetch(TlParser &p) {
  return make_tl_object<resPQ>(p);
}

resPQ::resPQ(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , pq_(TlFetchString<string>::parse(p))
  , server_public_key_fingerprints_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void resPQ::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(pq_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(server_public_key_fingerprints_, s);
}

void resPQ::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(pq_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(server_public_key_fingerprints_, s);
}

void resPQ::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resPQ");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("pq", pq_);
    { s.store_vector_begin("server_public_key_fingerprints", server_public_key_fingerprints_.size()); for (const auto &_value : server_public_key_fingerprints_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<RpcDropAnswer> RpcDropAnswer::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case rpc_answer_unknown::ID:
      return rpc_answer_unknown::fetch(p);
    case rpc_answer_dropped_running::ID:
      return rpc_answer_dropped_running::fetch(p);
    case rpc_answer_dropped::ID:
      return rpc_answer_dropped::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t rpc_answer_unknown::ID;

object_ptr<RpcDropAnswer> rpc_answer_unknown::fetch(TlParser &p) {
  return make_tl_object<rpc_answer_unknown>();
}

void rpc_answer_unknown::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void rpc_answer_unknown::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void rpc_answer_unknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rpc_answer_unknown");
    s.store_class_end();
  }
}

const std::int32_t rpc_answer_dropped_running::ID;

object_ptr<RpcDropAnswer> rpc_answer_dropped_running::fetch(TlParser &p) {
  return make_tl_object<rpc_answer_dropped_running>();
}

void rpc_answer_dropped_running::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void rpc_answer_dropped_running::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void rpc_answer_dropped_running::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rpc_answer_dropped_running");
    s.store_class_end();
  }
}

rpc_answer_dropped::rpc_answer_dropped(int64 msg_id_, int32 seq_no_, int32 bytes_)
  : msg_id_(msg_id_)
  , seq_no_(seq_no_)
  , bytes_(bytes_)
{}

const std::int32_t rpc_answer_dropped::ID;

object_ptr<RpcDropAnswer> rpc_answer_dropped::fetch(TlParser &p) {
  return make_tl_object<rpc_answer_dropped>(p);
}

rpc_answer_dropped::rpc_answer_dropped(TlParser &p)
  : msg_id_(TlFetchLong::parse(p))
  , seq_no_(TlFetchInt::parse(p))
  , bytes_(TlFetchInt::parse(p))
{}

void rpc_answer_dropped::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(seq_no_, s);
  TlStoreBinary::store(bytes_, s);
}

void rpc_answer_dropped::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(seq_no_, s);
  TlStoreBinary::store(bytes_, s);
}

void rpc_answer_dropped::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rpc_answer_dropped");
    s.store_field("msg_id", msg_id_);
    s.store_field("seq_no", seq_no_);
    s.store_field("bytes", bytes_);
    s.store_class_end();
  }
}

rpc_error::rpc_error(int32 error_code_, string const &error_message_)
  : error_code_(error_code_)
  , error_message_(error_message_)
{}

const std::int32_t rpc_error::ID;

object_ptr<rpc_error> rpc_error::fetch(TlParser &p) {
  return make_tl_object<rpc_error>(p);
}

rpc_error::rpc_error(TlParser &p)
  : error_code_(TlFetchInt::parse(p))
  , error_message_(TlFetchString<string>::parse(p))
{}

void rpc_error::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(error_code_, s);
  TlStoreString::store(error_message_, s);
}

void rpc_error::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(error_code_, s);
  TlStoreString::store(error_message_, s);
}

void rpc_error::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rpc_error");
    s.store_field("error_code", error_code_);
    s.store_field("error_message", error_message_);
    s.store_class_end();
  }
}

server_DH_params_ok::server_DH_params_ok(UInt128 const &nonce_, UInt128 const &server_nonce_, string const &encrypted_answer_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , encrypted_answer_(encrypted_answer_)
{}

const std::int32_t server_DH_params_ok::ID;

object_ptr<server_DH_params_ok> server_DH_params_ok::fetch(TlParser &p) {
  return make_tl_object<server_DH_params_ok>(p);
}

server_DH_params_ok::server_DH_params_ok(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , encrypted_answer_(TlFetchString<string>::parse(p))
{}

void server_DH_params_ok::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(encrypted_answer_, s);
}

void server_DH_params_ok::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(encrypted_answer_, s);
}

void server_DH_params_ok::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "server_DH_params_ok");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("encrypted_answer", encrypted_answer_);
    s.store_class_end();
  }
}

server_DH_inner_data::server_DH_inner_data(UInt128 const &nonce_, UInt128 const &server_nonce_, int32 g_, string const &dh_prime_, string const &g_a_, int32 server_time_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , g_(g_)
  , dh_prime_(dh_prime_)
  , g_a_(g_a_)
  , server_time_(server_time_)
{}

const std::int32_t server_DH_inner_data::ID;

object_ptr<server_DH_inner_data> server_DH_inner_data::fetch(TlParser &p) {
  return make_tl_object<server_DH_inner_data>(p);
}

server_DH_inner_data::server_DH_inner_data(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , g_(TlFetchInt::parse(p))
  , dh_prime_(TlFetchString<string>::parse(p))
  , g_a_(TlFetchString<string>::parse(p))
  , server_time_(TlFetchInt::parse(p))
{}

void server_DH_inner_data::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(g_, s);
  TlStoreString::store(dh_prime_, s);
  TlStoreString::store(g_a_, s);
  TlStoreBinary::store(server_time_, s);
}

void server_DH_inner_data::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(g_, s);
  TlStoreString::store(dh_prime_, s);
  TlStoreString::store(g_a_, s);
  TlStoreBinary::store(server_time_, s);
}

void server_DH_inner_data::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "server_DH_inner_data");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("g", g_);
    s.store_field("dh_prime", dh_prime_);
    s.store_field("g_a", g_a_);
    s.store_field("server_time", server_time_);
    s.store_class_end();
  }
}

object_ptr<Set_client_DH_params_answer> Set_client_DH_params_answer::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case dh_gen_ok::ID:
      return dh_gen_ok::fetch(p);
    case dh_gen_retry::ID:
      return dh_gen_retry::fetch(p);
    case dh_gen_fail::ID:
      return dh_gen_fail::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

dh_gen_ok::dh_gen_ok(UInt128 const &nonce_, UInt128 const &server_nonce_, UInt128 const &new_nonce_hash1_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , new_nonce_hash1_(new_nonce_hash1_)
{}

const std::int32_t dh_gen_ok::ID;

object_ptr<Set_client_DH_params_answer> dh_gen_ok::fetch(TlParser &p) {
  return make_tl_object<dh_gen_ok>(p);
}

dh_gen_ok::dh_gen_ok(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , new_nonce_hash1_(TlFetchInt128::parse(p))
{}

void dh_gen_ok::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_hash1_, s);
}

void dh_gen_ok::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_hash1_, s);
}

void dh_gen_ok::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dh_gen_ok");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("new_nonce_hash1", new_nonce_hash1_);
    s.store_class_end();
  }
}

dh_gen_retry::dh_gen_retry(UInt128 const &nonce_, UInt128 const &server_nonce_, UInt128 const &new_nonce_hash2_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , new_nonce_hash2_(new_nonce_hash2_)
{}

const std::int32_t dh_gen_retry::ID;

object_ptr<Set_client_DH_params_answer> dh_gen_retry::fetch(TlParser &p) {
  return make_tl_object<dh_gen_retry>(p);
}

dh_gen_retry::dh_gen_retry(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , new_nonce_hash2_(TlFetchInt128::parse(p))
{}

void dh_gen_retry::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_hash2_, s);
}

void dh_gen_retry::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_hash2_, s);
}

void dh_gen_retry::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dh_gen_retry");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("new_nonce_hash2", new_nonce_hash2_);
    s.store_class_end();
  }
}

dh_gen_fail::dh_gen_fail(UInt128 const &nonce_, UInt128 const &server_nonce_, UInt128 const &new_nonce_hash3_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , new_nonce_hash3_(new_nonce_hash3_)
{}

const std::int32_t dh_gen_fail::ID;

object_ptr<Set_client_DH_params_answer> dh_gen_fail::fetch(TlParser &p) {
  return make_tl_object<dh_gen_fail>(p);
}

dh_gen_fail::dh_gen_fail(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , new_nonce_hash3_(TlFetchInt128::parse(p))
{}

void dh_gen_fail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_hash3_, s);
}

void dh_gen_fail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreBinary::store(new_nonce_hash3_, s);
}

void dh_gen_fail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dh_gen_fail");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("new_nonce_hash3", new_nonce_hash3_);
    s.store_class_end();
  }
}

const std::int32_t destroy_auth_key::ID;

object_ptr<destroy_auth_key> destroy_auth_key::fetch(TlParser &p) {
  return make_tl_object<destroy_auth_key>();
}

void destroy_auth_key::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-784117408);
}

void destroy_auth_key::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-784117408);
}

void destroy_auth_key::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "destroy_auth_key");
    s.store_class_end();
  }
}

destroy_auth_key::ReturnType destroy_auth_key::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<DestroyAuthKeyRes>::parse(p);
#undef FAIL
}

get_future_salts::get_future_salts(int32 num_)
  : num_(num_)
{}

const std::int32_t get_future_salts::ID;

object_ptr<get_future_salts> get_future_salts::fetch(TlParser &p) {
  return make_tl_object<get_future_salts>(p);
}

get_future_salts::get_future_salts(TlParser &p)
  : num_(TlFetchInt::parse(p))
{}

void get_future_salts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1188971260);
  TlStoreBinary::store(num_, s);
}

void get_future_salts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1188971260);
  TlStoreBinary::store(num_, s);
}

void get_future_salts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "get_future_salts");
    s.store_field("num", num_);
    s.store_class_end();
  }
}

get_future_salts::ReturnType get_future_salts::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<future_salts>, -1370486635>::parse(p);
#undef FAIL
}

http_wait::http_wait(int32 max_delay_, int32 wait_after_, int32 max_wait_)
  : max_delay_(max_delay_)
  , wait_after_(wait_after_)
  , max_wait_(max_wait_)
{}

const std::int32_t http_wait::ID;

object_ptr<http_wait> http_wait::fetch(TlParser &p) {
  return make_tl_object<http_wait>(p);
}

http_wait::http_wait(TlParser &p)
  : max_delay_(TlFetchInt::parse(p))
  , wait_after_(TlFetchInt::parse(p))
  , max_wait_(TlFetchInt::parse(p))
{}

void http_wait::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1835453025);
  TlStoreBinary::store(max_delay_, s);
  TlStoreBinary::store(wait_after_, s);
  TlStoreBinary::store(max_wait_, s);
}

void http_wait::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1835453025);
  TlStoreBinary::store(max_delay_, s);
  TlStoreBinary::store(wait_after_, s);
  TlStoreBinary::store(max_wait_, s);
}

void http_wait::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "http_wait");
    s.store_field("max_delay", max_delay_);
    s.store_field("wait_after", wait_after_);
    s.store_field("max_wait", max_wait_);
    s.store_class_end();
  }
}

http_wait::ReturnType http_wait::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<dummyHttpWait>, -919090642>::parse(p);
#undef FAIL
}

ping_delay_disconnect::ping_delay_disconnect(int64 ping_id_, int32 disconnect_delay_)
  : ping_id_(ping_id_)
  , disconnect_delay_(disconnect_delay_)
{}

const std::int32_t ping_delay_disconnect::ID;

object_ptr<ping_delay_disconnect> ping_delay_disconnect::fetch(TlParser &p) {
  return make_tl_object<ping_delay_disconnect>(p);
}

ping_delay_disconnect::ping_delay_disconnect(TlParser &p)
  : ping_id_(TlFetchLong::parse(p))
  , disconnect_delay_(TlFetchInt::parse(p))
{}

void ping_delay_disconnect::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-213746804);
  TlStoreBinary::store(ping_id_, s);
  TlStoreBinary::store(disconnect_delay_, s);
}

void ping_delay_disconnect::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-213746804);
  TlStoreBinary::store(ping_id_, s);
  TlStoreBinary::store(disconnect_delay_, s);
}

void ping_delay_disconnect::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "ping_delay_disconnect");
    s.store_field("ping_id", ping_id_);
    s.store_field("disconnect_delay", disconnect_delay_);
    s.store_class_end();
  }
}

ping_delay_disconnect::ReturnType ping_delay_disconnect::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<pong>, 880243653>::parse(p);
#undef FAIL
}

req_DH_params::req_DH_params(UInt128 const &nonce_, UInt128 const &server_nonce_, string const &p_, string const &q_, int64 public_key_fingerprint_, string const &encrypted_data_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , p_(p_)
  , q_(q_)
  , public_key_fingerprint_(public_key_fingerprint_)
  , encrypted_data_(encrypted_data_)
{}

const std::int32_t req_DH_params::ID;

object_ptr<req_DH_params> req_DH_params::fetch(TlParser &p) {
  return make_tl_object<req_DH_params>(p);
}

req_DH_params::req_DH_params(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , p_(TlFetchString<string>::parse(p))
  , q_(TlFetchString<string>::parse(p))
  , public_key_fingerprint_(TlFetchLong::parse(p))
  , encrypted_data_(TlFetchString<string>::parse(p))
{}

void req_DH_params::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-686627650);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(p_, s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(public_key_fingerprint_, s);
  TlStoreString::store(encrypted_data_, s);
}

void req_DH_params::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-686627650);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(p_, s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(public_key_fingerprint_, s);
  TlStoreString::store(encrypted_data_, s);
}

void req_DH_params::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "req_DH_params");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("p", p_);
    s.store_field("q", q_);
    s.store_field("public_key_fingerprint", public_key_fingerprint_);
    s.store_field("encrypted_data", encrypted_data_);
    s.store_class_end();
  }
}

req_DH_params::ReturnType req_DH_params::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<server_DH_params_ok>, -790100132>::parse(p);
#undef FAIL
}

req_pq_multi::req_pq_multi(UInt128 const &nonce_)
  : nonce_(nonce_)
{}

const std::int32_t req_pq_multi::ID;

object_ptr<req_pq_multi> req_pq_multi::fetch(TlParser &p) {
  return make_tl_object<req_pq_multi>(p);
}

req_pq_multi::req_pq_multi(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
{}

void req_pq_multi::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1099002127);
  TlStoreBinary::store(nonce_, s);
}

void req_pq_multi::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1099002127);
  TlStoreBinary::store(nonce_, s);
}

void req_pq_multi::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "req_pq_multi");
    s.store_field("nonce", nonce_);
    s.store_class_end();
  }
}

req_pq_multi::ReturnType req_pq_multi::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<resPQ>, 85337187>::parse(p);
#undef FAIL
}

rpc_drop_answer::rpc_drop_answer(int64 req_msg_id_)
  : req_msg_id_(req_msg_id_)
{}

const std::int32_t rpc_drop_answer::ID;

object_ptr<rpc_drop_answer> rpc_drop_answer::fetch(TlParser &p) {
  return make_tl_object<rpc_drop_answer>(p);
}

rpc_drop_answer::rpc_drop_answer(TlParser &p)
  : req_msg_id_(TlFetchLong::parse(p))
{}

void rpc_drop_answer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1491380032);
  TlStoreBinary::store(req_msg_id_, s);
}

void rpc_drop_answer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1491380032);
  TlStoreBinary::store(req_msg_id_, s);
}

void rpc_drop_answer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rpc_drop_answer");
    s.store_field("req_msg_id", req_msg_id_);
    s.store_class_end();
  }
}

rpc_drop_answer::ReturnType rpc_drop_answer::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<RpcDropAnswer>::parse(p);
#undef FAIL
}

set_client_DH_params::set_client_DH_params(UInt128 const &nonce_, UInt128 const &server_nonce_, string const &encrypted_data_)
  : nonce_(nonce_)
  , server_nonce_(server_nonce_)
  , encrypted_data_(encrypted_data_)
{}

const std::int32_t set_client_DH_params::ID;

object_ptr<set_client_DH_params> set_client_DH_params::fetch(TlParser &p) {
  return make_tl_object<set_client_DH_params>(p);
}

set_client_DH_params::set_client_DH_params(TlParser &p)
  : nonce_(TlFetchInt128::parse(p))
  , server_nonce_(TlFetchInt128::parse(p))
  , encrypted_data_(TlFetchString<string>::parse(p))
{}

void set_client_DH_params::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-184262881);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(encrypted_data_, s);
}

void set_client_DH_params::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-184262881);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(server_nonce_, s);
  TlStoreString::store(encrypted_data_, s);
}

void set_client_DH_params::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "set_client_DH_params");
    s.store_field("nonce", nonce_);
    s.store_field("server_nonce", server_nonce_);
    s.store_field("encrypted_data", encrypted_data_);
    s.store_class_end();
  }
}

set_client_DH_params::ReturnType set_client_DH_params::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Set_client_DH_params_answer>::parse(p);
#undef FAIL
}
}  // namespace mtproto_api
}  // namespace td
