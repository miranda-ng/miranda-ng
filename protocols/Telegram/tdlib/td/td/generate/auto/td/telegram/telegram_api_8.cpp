#include "telegram_api.h"

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
namespace telegram_api {


authorization::authorization()
  : flags_()
  , current_()
  , official_app_()
  , password_pending_()
  , encrypted_requests_disabled_()
  , call_requests_disabled_()
  , unconfirmed_()
  , hash_()
  , device_model_()
  , platform_()
  , system_version_()
  , api_id_()
  , app_name_()
  , app_version_()
  , date_created_()
  , date_active_()
  , ip_()
  , country_()
  , region_()
{}

const std::int32_t authorization::ID;

object_ptr<authorization> authorization::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<authorization> res = make_tl_object<authorization>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->current_ = (var0 & 1) != 0;
  res->official_app_ = (var0 & 2) != 0;
  res->password_pending_ = (var0 & 4) != 0;
  res->encrypted_requests_disabled_ = (var0 & 8) != 0;
  res->call_requests_disabled_ = (var0 & 16) != 0;
  res->unconfirmed_ = (var0 & 32) != 0;
  res->hash_ = TlFetchLong::parse(p);
  res->device_model_ = TlFetchString<string>::parse(p);
  res->platform_ = TlFetchString<string>::parse(p);
  res->system_version_ = TlFetchString<string>::parse(p);
  res->api_id_ = TlFetchInt::parse(p);
  res->app_name_ = TlFetchString<string>::parse(p);
  res->app_version_ = TlFetchString<string>::parse(p);
  res->date_created_ = TlFetchInt::parse(p);
  res->date_active_ = TlFetchInt::parse(p);
  res->ip_ = TlFetchString<string>::parse(p);
  res->country_ = TlFetchString<string>::parse(p);
  res->region_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void authorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorization");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (current_ << 0) | (official_app_ << 1) | (password_pending_ << 2) | (encrypted_requests_disabled_ << 3) | (call_requests_disabled_ << 4) | (unconfirmed_ << 5)));
    if (var0 & 1) { s.store_field("current", true); }
    if (var0 & 2) { s.store_field("official_app", true); }
    if (var0 & 4) { s.store_field("password_pending", true); }
    if (var0 & 8) { s.store_field("encrypted_requests_disabled", true); }
    if (var0 & 16) { s.store_field("call_requests_disabled", true); }
    if (var0 & 32) { s.store_field("unconfirmed", true); }
    s.store_field("hash", hash_);
    s.store_field("device_model", device_model_);
    s.store_field("platform", platform_);
    s.store_field("system_version", system_version_);
    s.store_field("api_id", api_id_);
    s.store_field("app_name", app_name_);
    s.store_field("app_version", app_version_);
    s.store_field("date_created", date_created_);
    s.store_field("date_active", date_active_);
    s.store_field("ip", ip_);
    s.store_field("country", country_);
    s.store_field("region", region_);
    s.store_class_end();
  }
}

const std::int32_t autoSaveException::ID;

object_ptr<autoSaveException> autoSaveException::fetch(TlBufferParser &p) {
  return make_tl_object<autoSaveException>(p);
}

autoSaveException::autoSaveException(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , settings_(TlFetchBoxed<TlFetchObject<autoSaveSettings>, -934791986>::parse(p))
{}

void autoSaveException::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autoSaveException");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

boost::boost()
  : flags_()
  , gift_()
  , giveaway_()
  , unclaimed_()
  , id_()
  , user_id_()
  , giveaway_msg_id_()
  , date_()
  , expires_()
  , used_gift_slug_()
  , multiplier_()
  , stars_()
{}

const std::int32_t boost::ID;

object_ptr<boost> boost::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<boost> res = make_tl_object<boost>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->gift_ = (var0 & 2) != 0;
  res->giveaway_ = (var0 & 4) != 0;
  res->unclaimed_ = (var0 & 8) != 0;
  res->id_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->user_id_ = TlFetchLong::parse(p); }
  if (var0 & 4) { res->giveaway_msg_id_ = TlFetchInt::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  res->expires_ = TlFetchInt::parse(p);
  if (var0 & 16) { res->used_gift_slug_ = TlFetchString<string>::parse(p); }
  if (var0 & 32) { res->multiplier_ = TlFetchInt::parse(p); }
  if (var0 & 64) { res->stars_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void boost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "boost");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (gift_ << 1) | (giveaway_ << 2) | (unclaimed_ << 3)));
    if (var0 & 2) { s.store_field("gift", true); }
    if (var0 & 4) { s.store_field("giveaway", true); }
    if (var0 & 8) { s.store_field("unclaimed", true); }
    s.store_field("id", id_);
    if (var0 & 1) { s.store_field("user_id", user_id_); }
    if (var0 & 4) { s.store_field("giveaway_msg_id", giveaway_msg_id_); }
    s.store_field("date", date_);
    s.store_field("expires", expires_);
    if (var0 & 16) { s.store_field("used_gift_slug", used_gift_slug_); }
    if (var0 & 32) { s.store_field("multiplier", multiplier_); }
    if (var0 & 64) { s.store_field("stars", stars_); }
    s.store_class_end();
  }
}

object_ptr<BotInlineResult> BotInlineResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case botInlineResult::ID:
      return botInlineResult::fetch(p);
    case botInlineMediaResult::ID:
      return botInlineMediaResult::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

botInlineResult::botInlineResult()
  : flags_()
  , id_()
  , type_()
  , title_()
  , description_()
  , url_()
  , thumb_()
  , content_()
  , send_message_()
{}

const std::int32_t botInlineResult::ID;

object_ptr<BotInlineResult> botInlineResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineResult> res = make_tl_object<botInlineResult>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchString<string>::parse(p);
  res->type_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->description_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->url_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->thumb_ = TlFetchObject<WebDocument>::parse(p); }
  if (var0 & 32) { res->content_ = TlFetchObject<WebDocument>::parse(p); }
  res->send_message_ = TlFetchObject<BotInlineMessage>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineResult");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    s.store_field("type", type_);
    if (var0 & 2) { s.store_field("title", title_); }
    if (var0 & 4) { s.store_field("description", description_); }
    if (var0 & 8) { s.store_field("url", url_); }
    if (var0 & 16) { s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get())); }
    if (var0 & 32) { s.store_object_field("content", static_cast<const BaseObject *>(content_.get())); }
    s.store_object_field("send_message", static_cast<const BaseObject *>(send_message_.get()));
    s.store_class_end();
  }
}

botInlineMediaResult::botInlineMediaResult()
  : flags_()
  , id_()
  , type_()
  , photo_()
  , document_()
  , title_()
  , description_()
  , send_message_()
{}

const std::int32_t botInlineMediaResult::ID;

object_ptr<BotInlineResult> botInlineMediaResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMediaResult> res = make_tl_object<botInlineMediaResult>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchString<string>::parse(p);
  res->type_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 2) { res->document_ = TlFetchObject<Document>::parse(p); }
  if (var0 & 4) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->description_ = TlFetchString<string>::parse(p); }
  res->send_message_ = TlFetchObject<BotInlineMessage>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMediaResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMediaResult");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    s.store_field("type", type_);
    if (var0 & 1) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 2) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 4) { s.store_field("title", title_); }
    if (var0 & 8) { s.store_field("description", description_); }
    s.store_object_field("send_message", static_cast<const BaseObject *>(send_message_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelMessagesFilterEmpty::ID;

void channelMessagesFilterEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void channelMessagesFilterEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void channelMessagesFilterEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelMessagesFilterEmpty");
    s.store_class_end();
  }
}

channelMessagesFilter::channelMessagesFilter(int32 flags_, bool exclude_new_messages_, array<object_ptr<messageRange>> &&ranges_)
  : flags_(flags_)
  , exclude_new_messages_(exclude_new_messages_)
  , ranges_(std::move(ranges_))
{}

const std::int32_t channelMessagesFilter::ID;

void channelMessagesFilter::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (exclude_new_messages_ << 1)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 182649427>>, 481674261>::store(ranges_, s);
}

void channelMessagesFilter::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (exclude_new_messages_ << 1)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 182649427>>, 481674261>::store(ranges_, s);
}

void channelMessagesFilter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelMessagesFilter");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (exclude_new_messages_ << 1)));
    if (var0 & 2) { s.store_field("exclude_new_messages", true); }
    { s.store_vector_begin("ranges", ranges_.size()); for (const auto &_value : ranges_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t chatAdminWithInvites::ID;

object_ptr<chatAdminWithInvites> chatAdminWithInvites::fetch(TlBufferParser &p) {
  return make_tl_object<chatAdminWithInvites>(p);
}

chatAdminWithInvites::chatAdminWithInvites(TlBufferParser &p)
  : admin_id_(TlFetchLong::parse(p))
  , invites_count_(TlFetchInt::parse(p))
  , revoked_invites_count_(TlFetchInt::parse(p))
{}

void chatAdminWithInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatAdminWithInvites");
    s.store_field("admin_id", admin_id_);
    s.store_field("invites_count", invites_count_);
    s.store_field("revoked_invites_count", revoked_invites_count_);
    s.store_class_end();
  }
}

connectedBotStarRef::connectedBotStarRef()
  : flags_()
  , revoked_()
  , url_()
  , date_()
  , bot_id_()
  , commission_permille_()
  , duration_months_()
  , participants_()
  , revenue_()
{}

const std::int32_t connectedBotStarRef::ID;

object_ptr<connectedBotStarRef> connectedBotStarRef::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<connectedBotStarRef> res = make_tl_object<connectedBotStarRef>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->revoked_ = (var0 & 2) != 0;
  res->url_ = TlFetchString<string>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->bot_id_ = TlFetchLong::parse(p);
  res->commission_permille_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->duration_months_ = TlFetchInt::parse(p); }
  res->participants_ = TlFetchLong::parse(p);
  res->revenue_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void connectedBotStarRef::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectedBotStarRef");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (revoked_ << 1)));
    if (var0 & 2) { s.store_field("revoked", true); }
    s.store_field("url", url_);
    s.store_field("date", date_);
    s.store_field("bot_id", bot_id_);
    s.store_field("commission_permille", commission_permille_);
    if (var0 & 1) { s.store_field("duration_months", duration_months_); }
    s.store_field("participants", participants_);
    s.store_field("revenue", revenue_);
    s.store_class_end();
  }
}

const std::int32_t dialogFilterSuggested::ID;

object_ptr<dialogFilterSuggested> dialogFilterSuggested::fetch(TlBufferParser &p) {
  return make_tl_object<dialogFilterSuggested>(p);
}

dialogFilterSuggested::dialogFilterSuggested(TlBufferParser &p)
  : filter_(TlFetchObject<DialogFilter>::parse(p))
  , description_(TlFetchString<string>::parse(p))
{}

void dialogFilterSuggested::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialogFilterSuggested");
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("description", description_);
    s.store_class_end();
  }
}

const std::int32_t emojiURL::ID;

object_ptr<emojiURL> emojiURL::fetch(TlBufferParser &p) {
  return make_tl_object<emojiURL>(p);
}

emojiURL::emojiURL(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void emojiURL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiURL");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

object_ptr<EncryptedMessage> EncryptedMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case encryptedMessage::ID:
      return encryptedMessage::fetch(p);
    case encryptedMessageService::ID:
      return encryptedMessageService::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t encryptedMessage::ID;

object_ptr<EncryptedMessage> encryptedMessage::fetch(TlBufferParser &p) {
  return make_tl_object<encryptedMessage>(p);
}

encryptedMessage::encryptedMessage(TlBufferParser &p)
  : random_id_(TlFetchLong::parse(p))
  , chat_id_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
  , file_(TlFetchObject<EncryptedFile>::parse(p))
{}

void encryptedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedMessage");
    s.store_field("random_id", random_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("date", date_);
    s.store_bytes_field("bytes", bytes_);
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_class_end();
  }
}

const std::int32_t encryptedMessageService::ID;

object_ptr<EncryptedMessage> encryptedMessageService::fetch(TlBufferParser &p) {
  return make_tl_object<encryptedMessageService>(p);
}

encryptedMessageService::encryptedMessageService(TlBufferParser &p)
  : random_id_(TlFetchLong::parse(p))
  , chat_id_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
{}

void encryptedMessageService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedMessageService");
    s.store_field("random_id", random_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("date", date_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

inlineBotWebView::inlineBotWebView(string const &text_, string const &url_)
  : text_(text_)
  , url_(url_)
{}

const std::int32_t inlineBotWebView::ID;

object_ptr<inlineBotWebView> inlineBotWebView::fetch(TlBufferParser &p) {
  return make_tl_object<inlineBotWebView>(p);
}

inlineBotWebView::inlineBotWebView(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void inlineBotWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void inlineBotWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void inlineBotWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineBotWebView");
    s.store_field("text", text_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

inputBotInlineResult::inputBotInlineResult(int32 flags_, string const &id_, string const &type_, string const &title_, string const &description_, string const &url_, object_ptr<inputWebDocument> &&thumb_, object_ptr<inputWebDocument> &&content_, object_ptr<InputBotInlineMessage> &&send_message_)
  : flags_(flags_)
  , id_(id_)
  , type_(type_)
  , title_(title_)
  , description_(description_)
  , url_(url_)
  , thumb_(std::move(thumb_))
  , content_(std::move(content_))
  , send_message_(std::move(send_message_))
{}

const std::int32_t inputBotInlineResult::ID;

void inputBotInlineResult::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(id_, s);
  TlStoreString::store(type_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreString::store(description_, s); }
  if (var0 & 8) { TlStoreString::store(url_, s); }
  if (var0 & 16) { TlStoreBoxed<TlStoreObject, -1678949555>::store(thumb_, s); }
  if (var0 & 32) { TlStoreBoxed<TlStoreObject, -1678949555>::store(content_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResult::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(id_, s);
  TlStoreString::store(type_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreString::store(description_, s); }
  if (var0 & 8) { TlStoreString::store(url_, s); }
  if (var0 & 16) { TlStoreBoxed<TlStoreObject, -1678949555>::store(thumb_, s); }
  if (var0 & 32) { TlStoreBoxed<TlStoreObject, -1678949555>::store(content_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineResult");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    s.store_field("type", type_);
    if (var0 & 2) { s.store_field("title", title_); }
    if (var0 & 4) { s.store_field("description", description_); }
    if (var0 & 8) { s.store_field("url", url_); }
    if (var0 & 16) { s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get())); }
    if (var0 & 32) { s.store_object_field("content", static_cast<const BaseObject *>(content_.get())); }
    s.store_object_field("send_message", static_cast<const BaseObject *>(send_message_.get()));
    s.store_class_end();
  }
}

inputBotInlineResultPhoto::inputBotInlineResultPhoto(string const &id_, string const &type_, object_ptr<InputPhoto> &&photo_, object_ptr<InputBotInlineMessage> &&send_message_)
  : id_(id_)
  , type_(type_)
  , photo_(std::move(photo_))
  , send_message_(std::move(send_message_))
{}

const std::int32_t inputBotInlineResultPhoto::ID;

void inputBotInlineResultPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(type_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResultPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(type_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResultPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineResultPhoto");
    s.store_field("id", id_);
    s.store_field("type", type_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("send_message", static_cast<const BaseObject *>(send_message_.get()));
    s.store_class_end();
  }
}

inputBotInlineResultDocument::inputBotInlineResultDocument(int32 flags_, string const &id_, string const &type_, string const &title_, string const &description_, object_ptr<InputDocument> &&document_, object_ptr<InputBotInlineMessage> &&send_message_)
  : flags_(flags_)
  , id_(id_)
  , type_(type_)
  , title_(title_)
  , description_(description_)
  , document_(std::move(document_))
  , send_message_(std::move(send_message_))
{}

const std::int32_t inputBotInlineResultDocument::ID;

void inputBotInlineResultDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(id_, s);
  TlStoreString::store(type_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreString::store(description_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(document_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResultDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(id_, s);
  TlStoreString::store(type_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreString::store(description_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(document_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResultDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineResultDocument");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    s.store_field("type", type_);
    if (var0 & 2) { s.store_field("title", title_); }
    if (var0 & 4) { s.store_field("description", description_); }
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_object_field("send_message", static_cast<const BaseObject *>(send_message_.get()));
    s.store_class_end();
  }
}

inputBotInlineResultGame::inputBotInlineResultGame(string const &id_, string const &short_name_, object_ptr<InputBotInlineMessage> &&send_message_)
  : id_(id_)
  , short_name_(short_name_)
  , send_message_(std::move(send_message_))
{}

const std::int32_t inputBotInlineResultGame::ID;

void inputBotInlineResultGame::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(short_name_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResultGame::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(short_name_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_message_, s);
}

void inputBotInlineResultGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineResultGame");
    s.store_field("id", id_);
    s.store_field("short_name", short_name_);
    s.store_object_field("send_message", static_cast<const BaseObject *>(send_message_.get()));
    s.store_class_end();
  }
}

object_ptr<InputDocument> InputDocument::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputDocumentEmpty::ID:
      return inputDocumentEmpty::fetch(p);
    case inputDocument::ID:
      return inputDocument::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputDocumentEmpty::ID;

object_ptr<InputDocument> inputDocumentEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputDocumentEmpty>();
}

void inputDocumentEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputDocumentEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputDocumentEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputDocumentEmpty");
    s.store_class_end();
  }
}

inputDocument::inputDocument(int64 id_, int64 access_hash_, bytes &&file_reference_)
  : id_(id_)
  , access_hash_(access_hash_)
  , file_reference_(std::move(file_reference_))
{}

const std::int32_t inputDocument::ID;

object_ptr<InputDocument> inputDocument::fetch(TlBufferParser &p) {
  return make_tl_object<inputDocument>(p);
}

inputDocument::inputDocument(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , file_reference_(TlFetchBytes<bytes>::parse(p))
{}

void inputDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
}

void inputDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
}

void inputDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputDocument");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_class_end();
  }
}

object_ptr<InputGroupCall> InputGroupCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputGroupCall::ID:
      return inputGroupCall::fetch(p);
    case inputGroupCallSlug::ID:
      return inputGroupCallSlug::fetch(p);
    case inputGroupCallInviteMessage::ID:
      return inputGroupCallInviteMessage::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

inputGroupCall::inputGroupCall(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputGroupCall::ID;

object_ptr<InputGroupCall> inputGroupCall::fetch(TlBufferParser &p) {
  return make_tl_object<inputGroupCall>(p);
}

inputGroupCall::inputGroupCall(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGroupCall");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputGroupCallSlug::inputGroupCallSlug(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t inputGroupCallSlug::ID;

object_ptr<InputGroupCall> inputGroupCallSlug::fetch(TlBufferParser &p) {
  return make_tl_object<inputGroupCallSlug>(p);
}

inputGroupCallSlug::inputGroupCallSlug(TlBufferParser &p)
  : slug_(TlFetchString<string>::parse(p))
{}

void inputGroupCallSlug::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputGroupCallSlug::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputGroupCallSlug::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGroupCallSlug");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

inputGroupCallInviteMessage::inputGroupCallInviteMessage(int32 msg_id_)
  : msg_id_(msg_id_)
{}

const std::int32_t inputGroupCallInviteMessage::ID;

object_ptr<InputGroupCall> inputGroupCallInviteMessage::fetch(TlBufferParser &p) {
  return make_tl_object<inputGroupCallInviteMessage>(p);
}

inputGroupCallInviteMessage::inputGroupCallInviteMessage(TlBufferParser &p)
  : msg_id_(TlFetchInt::parse(p))
{}

void inputGroupCallInviteMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
}

void inputGroupCallInviteMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
}

void inputGroupCallInviteMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGroupCallInviteMessage");
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

inputNotifyPeer::inputNotifyPeer(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t inputNotifyPeer::ID;

void inputNotifyPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void inputNotifyPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void inputNotifyPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputNotifyPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

const std::int32_t inputNotifyUsers::ID;

void inputNotifyUsers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputNotifyUsers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputNotifyUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputNotifyUsers");
    s.store_class_end();
  }
}

const std::int32_t inputNotifyChats::ID;

void inputNotifyChats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputNotifyChats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputNotifyChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputNotifyChats");
    s.store_class_end();
  }
}

const std::int32_t inputNotifyBroadcasts::ID;

void inputNotifyBroadcasts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputNotifyBroadcasts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputNotifyBroadcasts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputNotifyBroadcasts");
    s.store_class_end();
  }
}

inputNotifyForumTopic::inputNotifyForumTopic(object_ptr<InputPeer> &&peer_, int32 top_msg_id_)
  : peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
{}

const std::int32_t inputNotifyForumTopic::ID;

void inputNotifyForumTopic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(top_msg_id_, s);
}

void inputNotifyForumTopic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(top_msg_id_, s);
}

void inputNotifyForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputNotifyForumTopic");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("top_msg_id", top_msg_id_);
    s.store_class_end();
  }
}

inputSecureFileUploaded::inputSecureFileUploaded(int64 id_, int32 parts_, string const &md5_checksum_, bytes &&file_hash_, bytes &&secret_)
  : id_(id_)
  , parts_(parts_)
  , md5_checksum_(md5_checksum_)
  , file_hash_(std::move(file_hash_))
  , secret_(std::move(secret_))
{}

const std::int32_t inputSecureFileUploaded::ID;

void inputSecureFileUploaded::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(md5_checksum_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(secret_, s);
}

void inputSecureFileUploaded::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(md5_checksum_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(secret_, s);
}

void inputSecureFileUploaded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSecureFileUploaded");
    s.store_field("id", id_);
    s.store_field("parts", parts_);
    s.store_field("md5_checksum", md5_checksum_);
    s.store_bytes_field("file_hash", file_hash_);
    s.store_bytes_field("secret", secret_);
    s.store_class_end();
  }
}

inputSecureFile::inputSecureFile(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputSecureFile::ID;

void inputSecureFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputSecureFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputSecureFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSecureFile");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

object_ptr<JSONValue> JSONValue::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case jsonNull::ID:
      return jsonNull::fetch(p);
    case jsonBool::ID:
      return jsonBool::fetch(p);
    case jsonNumber::ID:
      return jsonNumber::fetch(p);
    case jsonString::ID:
      return jsonString::fetch(p);
    case jsonArray::ID:
      return jsonArray::fetch(p);
    case jsonObject::ID:
      return jsonObject::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t jsonNull::ID;

object_ptr<JSONValue> jsonNull::fetch(TlBufferParser &p) {
  return make_tl_object<jsonNull>();
}

void jsonNull::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void jsonNull::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void jsonNull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonNull");
    s.store_class_end();
  }
}

jsonBool::jsonBool(bool value_)
  : value_(value_)
{}

const std::int32_t jsonBool::ID;

object_ptr<JSONValue> jsonBool::fetch(TlBufferParser &p) {
  return make_tl_object<jsonBool>(p);
}

jsonBool::jsonBool(TlBufferParser &p)
  : value_(TlFetchBool::parse(p))
{}

void jsonBool::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBool::store(value_, s);
}

void jsonBool::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBool::store(value_, s);
}

void jsonBool::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonBool");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

jsonNumber::jsonNumber(double value_)
  : value_(value_)
{}

const std::int32_t jsonNumber::ID;

object_ptr<JSONValue> jsonNumber::fetch(TlBufferParser &p) {
  return make_tl_object<jsonNumber>(p);
}

jsonNumber::jsonNumber(TlBufferParser &p)
  : value_(TlFetchDouble::parse(p))
{}

void jsonNumber::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(value_, s);
}

void jsonNumber::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(value_, s);
}

void jsonNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonNumber");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

jsonString::jsonString(string const &value_)
  : value_(value_)
{}

const std::int32_t jsonString::ID;

object_ptr<JSONValue> jsonString::fetch(TlBufferParser &p) {
  return make_tl_object<jsonString>(p);
}

jsonString::jsonString(TlBufferParser &p)
  : value_(TlFetchString<string>::parse(p))
{}

void jsonString::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(value_, s);
}

void jsonString::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(value_, s);
}

void jsonString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonString");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

jsonArray::jsonArray(array<object_ptr<JSONValue>> &&value_)
  : value_(std::move(value_))
{}

const std::int32_t jsonArray::ID;

object_ptr<JSONValue> jsonArray::fetch(TlBufferParser &p) {
  return make_tl_object<jsonArray>(p);
}

jsonArray::jsonArray(TlBufferParser &p)
  : value_(TlFetchBoxed<TlFetchVector<TlFetchObject<JSONValue>>, 481674261>::parse(p))
{}

void jsonArray::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(value_, s);
}

void jsonArray::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(value_, s);
}

void jsonArray::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonArray");
    { s.store_vector_begin("value", value_.size()); for (const auto &_value : value_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

jsonObject::jsonObject(array<object_ptr<jsonObjectValue>> &&value_)
  : value_(std::move(value_))
{}

const std::int32_t jsonObject::ID;

object_ptr<JSONValue> jsonObject::fetch(TlBufferParser &p) {
  return make_tl_object<jsonObject>(p);
}

jsonObject::jsonObject(TlBufferParser &p)
  : value_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<jsonObjectValue>, -1059185703>>, 481674261>::parse(p))
{}

void jsonObject::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1059185703>>, 481674261>::store(value_, s);
}

void jsonObject::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1059185703>>, 481674261>::store(value_, s);
}

void jsonObject::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonObject");
    { s.store_vector_begin("value", value_.size()); for (const auto &_value : value_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<MediaArea> MediaArea::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case mediaAreaVenue::ID:
      return mediaAreaVenue::fetch(p);
    case inputMediaAreaVenue::ID:
      return inputMediaAreaVenue::fetch(p);
    case mediaAreaGeoPoint::ID:
      return mediaAreaGeoPoint::fetch(p);
    case mediaAreaSuggestedReaction::ID:
      return mediaAreaSuggestedReaction::fetch(p);
    case mediaAreaChannelPost::ID:
      return mediaAreaChannelPost::fetch(p);
    case inputMediaAreaChannelPost::ID:
      return inputMediaAreaChannelPost::fetch(p);
    case mediaAreaUrl::ID:
      return mediaAreaUrl::fetch(p);
    case mediaAreaWeather::ID:
      return mediaAreaWeather::fetch(p);
    case mediaAreaStarGift::ID:
      return mediaAreaStarGift::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

mediaAreaVenue::mediaAreaVenue(object_ptr<mediaAreaCoordinates> &&coordinates_, object_ptr<GeoPoint> &&geo_, string const &title_, string const &address_, string const &provider_, string const &venue_id_, string const &venue_type_)
  : coordinates_(std::move(coordinates_))
  , geo_(std::move(geo_))
  , title_(title_)
  , address_(address_)
  , provider_(provider_)
  , venue_id_(venue_id_)
  , venue_type_(venue_type_)
{}

const std::int32_t mediaAreaVenue::ID;

object_ptr<MediaArea> mediaAreaVenue::fetch(TlBufferParser &p) {
  return make_tl_object<mediaAreaVenue>(p);
}

mediaAreaVenue::mediaAreaVenue(TlBufferParser &p)
  : coordinates_(TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p))
  , geo_(TlFetchObject<GeoPoint>::parse(p))
  , title_(TlFetchString<string>::parse(p))
  , address_(TlFetchString<string>::parse(p))
  , provider_(TlFetchString<string>::parse(p))
  , venue_id_(TlFetchString<string>::parse(p))
  , venue_type_(TlFetchString<string>::parse(p))
{}

void mediaAreaVenue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
  TlStoreString::store(venue_type_, s);
}

void mediaAreaVenue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
  TlStoreString::store(venue_type_, s);
}

void mediaAreaVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaVenue");
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    s.store_field("title", title_);
    s.store_field("address", address_);
    s.store_field("provider", provider_);
    s.store_field("venue_id", venue_id_);
    s.store_field("venue_type", venue_type_);
    s.store_class_end();
  }
}

inputMediaAreaVenue::inputMediaAreaVenue(object_ptr<mediaAreaCoordinates> &&coordinates_, int64 query_id_, string const &result_id_)
  : coordinates_(std::move(coordinates_))
  , query_id_(query_id_)
  , result_id_(result_id_)
{}

const std::int32_t inputMediaAreaVenue::ID;

object_ptr<MediaArea> inputMediaAreaVenue::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaAreaVenue>(p);
}

inputMediaAreaVenue::inputMediaAreaVenue(TlBufferParser &p)
  : coordinates_(TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p))
  , query_id_(TlFetchLong::parse(p))
  , result_id_(TlFetchString<string>::parse(p))
{}

void inputMediaAreaVenue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBinary::store(query_id_, s);
  TlStoreString::store(result_id_, s);
}

void inputMediaAreaVenue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBinary::store(query_id_, s);
  TlStoreString::store(result_id_, s);
}

void inputMediaAreaVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaAreaVenue");
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_field("query_id", query_id_);
    s.store_field("result_id", result_id_);
    s.store_class_end();
  }
}

mediaAreaGeoPoint::mediaAreaGeoPoint()
  : flags_()
  , coordinates_()
  , geo_()
  , address_()
{}

mediaAreaGeoPoint::mediaAreaGeoPoint(int32 flags_, object_ptr<mediaAreaCoordinates> &&coordinates_, object_ptr<GeoPoint> &&geo_, object_ptr<geoPointAddress> &&address_)
  : flags_(flags_)
  , coordinates_(std::move(coordinates_))
  , geo_(std::move(geo_))
  , address_(std::move(address_))
{}

const std::int32_t mediaAreaGeoPoint::ID;

object_ptr<MediaArea> mediaAreaGeoPoint::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<mediaAreaGeoPoint> res = make_tl_object<mediaAreaGeoPoint>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->coordinates_ = TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p);
  res->geo_ = TlFetchObject<GeoPoint>::parse(p);
  if (var0 & 1) { res->address_ = TlFetchBoxed<TlFetchObject<geoPointAddress>, -565420653>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void mediaAreaGeoPoint::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -565420653>::store(address_, s); }
}

void mediaAreaGeoPoint::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -565420653>::store(address_, s); }
}

void mediaAreaGeoPoint::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaGeoPoint");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    if (var0 & 1) { s.store_object_field("address", static_cast<const BaseObject *>(address_.get())); }
    s.store_class_end();
  }
}

mediaAreaSuggestedReaction::mediaAreaSuggestedReaction()
  : flags_()
  , dark_()
  , flipped_()
  , coordinates_()
  , reaction_()
{}

mediaAreaSuggestedReaction::mediaAreaSuggestedReaction(int32 flags_, bool dark_, bool flipped_, object_ptr<mediaAreaCoordinates> &&coordinates_, object_ptr<Reaction> &&reaction_)
  : flags_(flags_)
  , dark_(dark_)
  , flipped_(flipped_)
  , coordinates_(std::move(coordinates_))
  , reaction_(std::move(reaction_))
{}

const std::int32_t mediaAreaSuggestedReaction::ID;

object_ptr<MediaArea> mediaAreaSuggestedReaction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<mediaAreaSuggestedReaction> res = make_tl_object<mediaAreaSuggestedReaction>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->dark_ = (var0 & 1) != 0;
  res->flipped_ = (var0 & 2) != 0;
  res->coordinates_ = TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p);
  res->reaction_ = TlFetchObject<Reaction>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void mediaAreaSuggestedReaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0) | (flipped_ << 1)), s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
}

void mediaAreaSuggestedReaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0) | (flipped_ << 1)), s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
}

void mediaAreaSuggestedReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaSuggestedReaction");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (dark_ << 0) | (flipped_ << 1)));
    if (var0 & 1) { s.store_field("dark", true); }
    if (var0 & 2) { s.store_field("flipped", true); }
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_class_end();
  }
}

mediaAreaChannelPost::mediaAreaChannelPost(object_ptr<mediaAreaCoordinates> &&coordinates_, int64 channel_id_, int32 msg_id_)
  : coordinates_(std::move(coordinates_))
  , channel_id_(channel_id_)
  , msg_id_(msg_id_)
{}

const std::int32_t mediaAreaChannelPost::ID;

object_ptr<MediaArea> mediaAreaChannelPost::fetch(TlBufferParser &p) {
  return make_tl_object<mediaAreaChannelPost>(p);
}

mediaAreaChannelPost::mediaAreaChannelPost(TlBufferParser &p)
  : coordinates_(TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p))
  , channel_id_(TlFetchLong::parse(p))
  , msg_id_(TlFetchInt::parse(p))
{}

void mediaAreaChannelPost::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBinary::store(channel_id_, s);
  TlStoreBinary::store(msg_id_, s);
}

void mediaAreaChannelPost::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBinary::store(channel_id_, s);
  TlStoreBinary::store(msg_id_, s);
}

void mediaAreaChannelPost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaChannelPost");
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_field("channel_id", channel_id_);
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

inputMediaAreaChannelPost::inputMediaAreaChannelPost(object_ptr<mediaAreaCoordinates> &&coordinates_, object_ptr<InputChannel> &&channel_, int32 msg_id_)
  : coordinates_(std::move(coordinates_))
  , channel_(std::move(channel_))
  , msg_id_(msg_id_)
{}

const std::int32_t inputMediaAreaChannelPost::ID;

object_ptr<MediaArea> inputMediaAreaChannelPost::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaAreaChannelPost>(p);
}

inputMediaAreaChannelPost::inputMediaAreaChannelPost(TlBufferParser &p)
  : coordinates_(TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p))
  , channel_(TlFetchObject<InputChannel>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
{}

void inputMediaAreaChannelPost::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
}

void inputMediaAreaChannelPost::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
}

void inputMediaAreaChannelPost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaAreaChannelPost");
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

mediaAreaUrl::mediaAreaUrl(object_ptr<mediaAreaCoordinates> &&coordinates_, string const &url_)
  : coordinates_(std::move(coordinates_))
  , url_(url_)
{}

const std::int32_t mediaAreaUrl::ID;

object_ptr<MediaArea> mediaAreaUrl::fetch(TlBufferParser &p) {
  return make_tl_object<mediaAreaUrl>(p);
}

mediaAreaUrl::mediaAreaUrl(TlBufferParser &p)
  : coordinates_(TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void mediaAreaUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreString::store(url_, s);
}

void mediaAreaUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreString::store(url_, s);
}

void mediaAreaUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaUrl");
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_field("url", url_);
    s.store_class_end();
  }
}

mediaAreaWeather::mediaAreaWeather(object_ptr<mediaAreaCoordinates> &&coordinates_, string const &emoji_, double temperature_c_, int32 color_)
  : coordinates_(std::move(coordinates_))
  , emoji_(emoji_)
  , temperature_c_(temperature_c_)
  , color_(color_)
{}

const std::int32_t mediaAreaWeather::ID;

object_ptr<MediaArea> mediaAreaWeather::fetch(TlBufferParser &p) {
  return make_tl_object<mediaAreaWeather>(p);
}

mediaAreaWeather::mediaAreaWeather(TlBufferParser &p)
  : coordinates_(TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p))
  , emoji_(TlFetchString<string>::parse(p))
  , temperature_c_(TlFetchDouble::parse(p))
  , color_(TlFetchInt::parse(p))
{}

void mediaAreaWeather::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreString::store(emoji_, s);
  TlStoreBinary::store(temperature_c_, s);
  TlStoreBinary::store(color_, s);
}

void mediaAreaWeather::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreString::store(emoji_, s);
  TlStoreBinary::store(temperature_c_, s);
  TlStoreBinary::store(color_, s);
}

void mediaAreaWeather::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaWeather");
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_field("emoji", emoji_);
    s.store_field("temperature_c", temperature_c_);
    s.store_field("color", color_);
    s.store_class_end();
  }
}

mediaAreaStarGift::mediaAreaStarGift(object_ptr<mediaAreaCoordinates> &&coordinates_, string const &slug_)
  : coordinates_(std::move(coordinates_))
  , slug_(slug_)
{}

const std::int32_t mediaAreaStarGift::ID;

object_ptr<MediaArea> mediaAreaStarGift::fetch(TlBufferParser &p) {
  return make_tl_object<mediaAreaStarGift>(p);
}

mediaAreaStarGift::mediaAreaStarGift(TlBufferParser &p)
  : coordinates_(TlFetchBoxed<TlFetchObject<mediaAreaCoordinates>, -808853502>::parse(p))
  , slug_(TlFetchString<string>::parse(p))
{}

void mediaAreaStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreString::store(slug_, s);
}

void mediaAreaStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, -808853502>::store(coordinates_, s);
  TlStoreString::store(slug_, s);
}

void mediaAreaStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaStarGift");
    s.store_object_field("coordinates", static_cast<const BaseObject *>(coordinates_.get()));
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

const std::int32_t nearestDc::ID;

object_ptr<nearestDc> nearestDc::fetch(TlBufferParser &p) {
  return make_tl_object<nearestDc>(p);
}

nearestDc::nearestDc(TlBufferParser &p)
  : country_(TlFetchString<string>::parse(p))
  , this_dc_(TlFetchInt::parse(p))
  , nearest_dc_(TlFetchInt::parse(p))
{}

void nearestDc::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "nearestDc");
    s.store_field("country", country_);
    s.store_field("this_dc", this_dc_);
    s.store_field("nearest_dc", nearest_dc_);
    s.store_class_end();
  }
}

const std::int32_t paymentFormMethod::ID;

object_ptr<paymentFormMethod> paymentFormMethod::fetch(TlBufferParser &p) {
  return make_tl_object<paymentFormMethod>(p);
}

paymentFormMethod::paymentFormMethod(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , title_(TlFetchString<string>::parse(p))
{}

void paymentFormMethod::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentFormMethod");
    s.store_field("url", url_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

peerSettings::peerSettings()
  : flags_()
  , report_spam_()
  , add_contact_()
  , block_contact_()
  , share_contact_()
  , need_contacts_exception_()
  , report_geo_()
  , autoarchived_()
  , invite_members_()
  , request_chat_broadcast_()
  , business_bot_paused_()
  , business_bot_can_reply_()
  , geo_distance_()
  , request_chat_title_()
  , request_chat_date_()
  , business_bot_id_()
  , business_bot_manage_url_()
  , charge_paid_message_stars_()
  , registration_month_()
  , phone_country_()
  , name_change_date_()
  , photo_change_date_()
{}

const std::int32_t peerSettings::ID;

object_ptr<peerSettings> peerSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<peerSettings> res = make_tl_object<peerSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->report_spam_ = (var0 & 1) != 0;
  res->add_contact_ = (var0 & 2) != 0;
  res->block_contact_ = (var0 & 4) != 0;
  res->share_contact_ = (var0 & 8) != 0;
  res->need_contacts_exception_ = (var0 & 16) != 0;
  res->report_geo_ = (var0 & 32) != 0;
  res->autoarchived_ = (var0 & 128) != 0;
  res->invite_members_ = (var0 & 256) != 0;
  res->request_chat_broadcast_ = (var0 & 1024) != 0;
  res->business_bot_paused_ = (var0 & 2048) != 0;
  res->business_bot_can_reply_ = (var0 & 4096) != 0;
  if (var0 & 64) { res->geo_distance_ = TlFetchInt::parse(p); }
  if (var0 & 512) { res->request_chat_title_ = TlFetchString<string>::parse(p); }
  if (var0 & 512) { res->request_chat_date_ = TlFetchInt::parse(p); }
  if (var0 & 8192) { res->business_bot_id_ = TlFetchLong::parse(p); }
  if (var0 & 8192) { res->business_bot_manage_url_ = TlFetchString<string>::parse(p); }
  if (var0 & 16384) { res->charge_paid_message_stars_ = TlFetchLong::parse(p); }
  if (var0 & 32768) { res->registration_month_ = TlFetchString<string>::parse(p); }
  if (var0 & 65536) { res->phone_country_ = TlFetchString<string>::parse(p); }
  if (var0 & 131072) { res->name_change_date_ = TlFetchInt::parse(p); }
  if (var0 & 262144) { res->photo_change_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void peerSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (report_spam_ << 0) | (add_contact_ << 1) | (block_contact_ << 2) | (share_contact_ << 3) | (need_contacts_exception_ << 4) | (report_geo_ << 5) | (autoarchived_ << 7) | (invite_members_ << 8) | (request_chat_broadcast_ << 10) | (business_bot_paused_ << 11) | (business_bot_can_reply_ << 12)));
    if (var0 & 1) { s.store_field("report_spam", true); }
    if (var0 & 2) { s.store_field("add_contact", true); }
    if (var0 & 4) { s.store_field("block_contact", true); }
    if (var0 & 8) { s.store_field("share_contact", true); }
    if (var0 & 16) { s.store_field("need_contacts_exception", true); }
    if (var0 & 32) { s.store_field("report_geo", true); }
    if (var0 & 128) { s.store_field("autoarchived", true); }
    if (var0 & 256) { s.store_field("invite_members", true); }
    if (var0 & 1024) { s.store_field("request_chat_broadcast", true); }
    if (var0 & 2048) { s.store_field("business_bot_paused", true); }
    if (var0 & 4096) { s.store_field("business_bot_can_reply", true); }
    if (var0 & 64) { s.store_field("geo_distance", geo_distance_); }
    if (var0 & 512) { s.store_field("request_chat_title", request_chat_title_); }
    if (var0 & 512) { s.store_field("request_chat_date", request_chat_date_); }
    if (var0 & 8192) { s.store_field("business_bot_id", business_bot_id_); }
    if (var0 & 8192) { s.store_field("business_bot_manage_url", business_bot_manage_url_); }
    if (var0 & 16384) { s.store_field("charge_paid_message_stars", charge_paid_message_stars_); }
    if (var0 & 32768) { s.store_field("registration_month", registration_month_); }
    if (var0 & 65536) { s.store_field("phone_country", phone_country_); }
    if (var0 & 131072) { s.store_field("name_change_date", name_change_date_); }
    if (var0 & 262144) { s.store_field("photo_change_date", photo_change_date_); }
    s.store_class_end();
  }
}

poll::poll()
  : id_()
  , flags_()
  , closed_()
  , public_voters_()
  , multiple_choice_()
  , quiz_()
  , question_()
  , answers_()
  , close_period_()
  , close_date_()
{}

poll::poll(int64 id_, int32 flags_, bool closed_, bool public_voters_, bool multiple_choice_, bool quiz_, object_ptr<textWithEntities> &&question_, array<object_ptr<pollAnswer>> &&answers_, int32 close_period_, int32 close_date_)
  : id_(id_)
  , flags_(flags_)
  , closed_(closed_)
  , public_voters_(public_voters_)
  , multiple_choice_(multiple_choice_)
  , quiz_(quiz_)
  , question_(std::move(question_))
  , answers_(std::move(answers_))
  , close_period_(close_period_)
  , close_date_(close_date_)
{}

const std::int32_t poll::ID;

object_ptr<poll> poll::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<poll> res = make_tl_object<poll>();
  int32 var0;
  res->id_ = TlFetchLong::parse(p);
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->closed_ = (var0 & 1) != 0;
  res->public_voters_ = (var0 & 2) != 0;
  res->multiple_choice_ = (var0 & 4) != 0;
  res->quiz_ = (var0 & 8) != 0;
  res->question_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p);
  res->answers_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<pollAnswer>, -15277366>>, 481674261>::parse(p);
  if (var0 & 16) { res->close_period_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->close_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void poll::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store((var0 = flags_ | (closed_ << 0) | (public_voters_ << 1) | (multiple_choice_ << 2) | (quiz_ << 3)), s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(question_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -15277366>>, 481674261>::store(answers_, s);
  if (var0 & 16) { TlStoreBinary::store(close_period_, s); }
  if (var0 & 32) { TlStoreBinary::store(close_date_, s); }
}

void poll::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store((var0 = flags_ | (closed_ << 0) | (public_voters_ << 1) | (multiple_choice_ << 2) | (quiz_ << 3)), s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(question_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -15277366>>, 481674261>::store(answers_, s);
  if (var0 & 16) { TlStoreBinary::store(close_period_, s); }
  if (var0 & 32) { TlStoreBinary::store(close_date_, s); }
}

void poll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "poll");
  int32 var0;
    s.store_field("id", id_);
    s.store_field("flags", (var0 = flags_ | (closed_ << 0) | (public_voters_ << 1) | (multiple_choice_ << 2) | (quiz_ << 3)));
    if (var0 & 1) { s.store_field("closed", true); }
    if (var0 & 2) { s.store_field("public_voters", true); }
    if (var0 & 4) { s.store_field("multiple_choice", true); }
    if (var0 & 8) { s.store_field("quiz", true); }
    s.store_object_field("question", static_cast<const BaseObject *>(question_.get()));
    { s.store_vector_begin("answers", answers_.size()); for (const auto &_value : answers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 16) { s.store_field("close_period", close_period_); }
    if (var0 & 32) { s.store_field("close_date", close_date_); }
    s.store_class_end();
  }
}

premiumGiftCodeOption::premiumGiftCodeOption()
  : flags_()
  , users_()
  , months_()
  , store_product_()
  , store_quantity_()
  , currency_()
  , amount_()
{}

premiumGiftCodeOption::premiumGiftCodeOption(int32 flags_, int32 users_, int32 months_, string const &store_product_, int32 store_quantity_, string const &currency_, int64 amount_)
  : flags_(flags_)
  , users_(users_)
  , months_(months_)
  , store_product_(store_product_)
  , store_quantity_(store_quantity_)
  , currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t premiumGiftCodeOption::ID;

object_ptr<premiumGiftCodeOption> premiumGiftCodeOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<premiumGiftCodeOption> res = make_tl_object<premiumGiftCodeOption>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->users_ = TlFetchInt::parse(p);
  res->months_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->store_product_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->store_quantity_ = TlFetchInt::parse(p); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void premiumGiftCodeOption::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(users_, s);
  TlStoreBinary::store(months_, s);
  if (var0 & 1) { TlStoreString::store(store_product_, s); }
  if (var0 & 2) { TlStoreBinary::store(store_quantity_, s); }
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void premiumGiftCodeOption::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(users_, s);
  TlStoreBinary::store(months_, s);
  if (var0 & 1) { TlStoreString::store(store_product_, s); }
  if (var0 & 2) { TlStoreBinary::store(store_quantity_, s); }
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void premiumGiftCodeOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumGiftCodeOption");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("users", users_);
    s.store_field("months", months_);
    if (var0 & 1) { s.store_field("store_product", store_product_); }
    if (var0 & 2) { s.store_field("store_quantity", store_quantity_); }
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

object_ptr<Reaction> Reaction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case reactionEmpty::ID:
      return reactionEmpty::fetch(p);
    case reactionEmoji::ID:
      return reactionEmoji::fetch(p);
    case reactionCustomEmoji::ID:
      return reactionCustomEmoji::fetch(p);
    case reactionPaid::ID:
      return reactionPaid::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t reactionEmpty::ID;

object_ptr<Reaction> reactionEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<reactionEmpty>();
}

void reactionEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void reactionEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void reactionEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionEmpty");
    s.store_class_end();
  }
}

reactionEmoji::reactionEmoji(string const &emoticon_)
  : emoticon_(emoticon_)
{}

const std::int32_t reactionEmoji::ID;

object_ptr<Reaction> reactionEmoji::fetch(TlBufferParser &p) {
  return make_tl_object<reactionEmoji>(p);
}

reactionEmoji::reactionEmoji(TlBufferParser &p)
  : emoticon_(TlFetchString<string>::parse(p))
{}

void reactionEmoji::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void reactionEmoji::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void reactionEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionEmoji");
    s.store_field("emoticon", emoticon_);
    s.store_class_end();
  }
}

reactionCustomEmoji::reactionCustomEmoji(int64 document_id_)
  : document_id_(document_id_)
{}

const std::int32_t reactionCustomEmoji::ID;

object_ptr<Reaction> reactionCustomEmoji::fetch(TlBufferParser &p) {
  return make_tl_object<reactionCustomEmoji>(p);
}

reactionCustomEmoji::reactionCustomEmoji(TlBufferParser &p)
  : document_id_(TlFetchLong::parse(p))
{}

void reactionCustomEmoji::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(document_id_, s);
}

void reactionCustomEmoji::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(document_id_, s);
}

void reactionCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionCustomEmoji");
    s.store_field("document_id", document_id_);
    s.store_class_end();
  }
}

const std::int32_t reactionPaid::ID;

object_ptr<Reaction> reactionPaid::fetch(TlBufferParser &p) {
  return make_tl_object<reactionPaid>();
}

void reactionPaid::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void reactionPaid::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void reactionPaid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionPaid");
    s.store_class_end();
  }
}

object_ptr<ReactionNotificationsFrom> ReactionNotificationsFrom::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case reactionNotificationsFromContacts::ID:
      return reactionNotificationsFromContacts::fetch(p);
    case reactionNotificationsFromAll::ID:
      return reactionNotificationsFromAll::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t reactionNotificationsFromContacts::ID;

object_ptr<ReactionNotificationsFrom> reactionNotificationsFromContacts::fetch(TlBufferParser &p) {
  return make_tl_object<reactionNotificationsFromContacts>();
}

void reactionNotificationsFromContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void reactionNotificationsFromContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void reactionNotificationsFromContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionNotificationsFromContacts");
    s.store_class_end();
  }
}

const std::int32_t reactionNotificationsFromAll::ID;

object_ptr<ReactionNotificationsFrom> reactionNotificationsFromAll::fetch(TlBufferParser &p) {
  return make_tl_object<reactionNotificationsFromAll>();
}

void reactionNotificationsFromAll::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void reactionNotificationsFromAll::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void reactionNotificationsFromAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionNotificationsFromAll");
    s.store_class_end();
  }
}

object_ptr<RichText> RichText::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case textEmpty::ID:
      return textEmpty::fetch(p);
    case textPlain::ID:
      return textPlain::fetch(p);
    case textBold::ID:
      return textBold::fetch(p);
    case textItalic::ID:
      return textItalic::fetch(p);
    case textUnderline::ID:
      return textUnderline::fetch(p);
    case textStrike::ID:
      return textStrike::fetch(p);
    case textFixed::ID:
      return textFixed::fetch(p);
    case textUrl::ID:
      return textUrl::fetch(p);
    case textEmail::ID:
      return textEmail::fetch(p);
    case textConcat::ID:
      return textConcat::fetch(p);
    case textSubscript::ID:
      return textSubscript::fetch(p);
    case textSuperscript::ID:
      return textSuperscript::fetch(p);
    case textMarked::ID:
      return textMarked::fetch(p);
    case textPhone::ID:
      return textPhone::fetch(p);
    case textImage::ID:
      return textImage::fetch(p);
    case textAnchor::ID:
      return textAnchor::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t textEmpty::ID;

object_ptr<RichText> textEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<textEmpty>();
}

void textEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEmpty");
    s.store_class_end();
  }
}

const std::int32_t textPlain::ID;

object_ptr<RichText> textPlain::fetch(TlBufferParser &p) {
  return make_tl_object<textPlain>(p);
}

textPlain::textPlain(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
{}

void textPlain::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textPlain");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

const std::int32_t textBold::ID;

object_ptr<RichText> textBold::fetch(TlBufferParser &p) {
  return make_tl_object<textBold>(p);
}

textBold::textBold(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textBold::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textBold");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textItalic::ID;

object_ptr<RichText> textItalic::fetch(TlBufferParser &p) {
  return make_tl_object<textItalic>(p);
}

textItalic::textItalic(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textItalic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textItalic");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textUnderline::ID;

object_ptr<RichText> textUnderline::fetch(TlBufferParser &p) {
  return make_tl_object<textUnderline>(p);
}

textUnderline::textUnderline(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textUnderline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textUnderline");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textStrike::ID;

object_ptr<RichText> textStrike::fetch(TlBufferParser &p) {
  return make_tl_object<textStrike>(p);
}

textStrike::textStrike(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textStrike::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textStrike");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textFixed::ID;

object_ptr<RichText> textFixed::fetch(TlBufferParser &p) {
  return make_tl_object<textFixed>(p);
}

textFixed::textFixed(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textFixed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textFixed");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textUrl::ID;

object_ptr<RichText> textUrl::fetch(TlBufferParser &p) {
  return make_tl_object<textUrl>(p);
}

textUrl::textUrl(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , url_(TlFetchString<string>::parse(p))
  , webpage_id_(TlFetchLong::parse(p))
{}

void textUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textUrl");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("url", url_);
    s.store_field("webpage_id", webpage_id_);
    s.store_class_end();
  }
}

const std::int32_t textEmail::ID;

object_ptr<RichText> textEmail::fetch(TlBufferParser &p) {
  return make_tl_object<textEmail>(p);
}

textEmail::textEmail(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , email_(TlFetchString<string>::parse(p))
{}

void textEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEmail");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("email", email_);
    s.store_class_end();
  }
}

const std::int32_t textConcat::ID;

object_ptr<RichText> textConcat::fetch(TlBufferParser &p) {
  return make_tl_object<textConcat>(p);
}

textConcat::textConcat(TlBufferParser &p)
  : texts_(TlFetchBoxed<TlFetchVector<TlFetchObject<RichText>>, 481674261>::parse(p))
{}

void textConcat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textConcat");
    { s.store_vector_begin("texts", texts_.size()); for (const auto &_value : texts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t textSubscript::ID;

object_ptr<RichText> textSubscript::fetch(TlBufferParser &p) {
  return make_tl_object<textSubscript>(p);
}

textSubscript::textSubscript(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textSubscript::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textSubscript");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textSuperscript::ID;

object_ptr<RichText> textSuperscript::fetch(TlBufferParser &p) {
  return make_tl_object<textSuperscript>(p);
}

textSuperscript::textSuperscript(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textSuperscript::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textSuperscript");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textMarked::ID;

object_ptr<RichText> textMarked::fetch(TlBufferParser &p) {
  return make_tl_object<textMarked>(p);
}

textMarked::textMarked(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void textMarked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textMarked");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t textPhone::ID;

object_ptr<RichText> textPhone::fetch(TlBufferParser &p) {
  return make_tl_object<textPhone>(p);
}

textPhone::textPhone(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , phone_(TlFetchString<string>::parse(p))
{}

void textPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textPhone");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("phone", phone_);
    s.store_class_end();
  }
}

const std::int32_t textImage::ID;

object_ptr<RichText> textImage::fetch(TlBufferParser &p) {
  return make_tl_object<textImage>(p);
}

textImage::textImage(TlBufferParser &p)
  : document_id_(TlFetchLong::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
{}

void textImage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textImage");
    s.store_field("document_id", document_id_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_class_end();
  }
}

const std::int32_t textAnchor::ID;

object_ptr<RichText> textAnchor::fetch(TlBufferParser &p) {
  return make_tl_object<textAnchor>(p);
}

textAnchor::textAnchor(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , name_(TlFetchString<string>::parse(p))
{}

void textAnchor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textAnchor");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("name", name_);
    s.store_class_end();
  }
}

searchPostsFlood::searchPostsFlood()
  : flags_()
  , query_is_free_()
  , total_daily_()
  , remains_()
  , wait_till_()
  , stars_amount_()
{}

const std::int32_t searchPostsFlood::ID;

object_ptr<searchPostsFlood> searchPostsFlood::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<searchPostsFlood> res = make_tl_object<searchPostsFlood>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->query_is_free_ = (var0 & 1) != 0;
  res->total_daily_ = TlFetchInt::parse(p);
  res->remains_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->wait_till_ = TlFetchInt::parse(p); }
  res->stars_amount_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void searchPostsFlood::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPostsFlood");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (query_is_free_ << 0)));
    if (var0 & 1) { s.store_field("query_is_free", true); }
    s.store_field("total_daily", total_daily_);
    s.store_field("remains", remains_);
    if (var0 & 2) { s.store_field("wait_till", wait_till_); }
    s.store_field("stars_amount", stars_amount_);
    s.store_class_end();
  }
}

starsTransaction::starsTransaction()
  : flags_()
  , refund_()
  , pending_()
  , failed_()
  , gift_()
  , reaction_()
  , stargift_upgrade_()
  , business_transfer_()
  , stargift_resale_()
  , posts_search_()
  , stargift_prepaid_upgrade_()
  , stargift_drop_original_details_()
  , id_()
  , amount_()
  , date_()
  , peer_()
  , title_()
  , description_()
  , photo_()
  , transaction_date_()
  , transaction_url_()
  , bot_payload_()
  , msg_id_()
  , extended_media_()
  , subscription_period_()
  , giveaway_post_id_()
  , stargift_()
  , floodskip_number_()
  , starref_commission_permille_()
  , starref_peer_()
  , starref_amount_()
  , paid_messages_()
  , premium_gift_months_()
  , ads_proceeds_from_date_()
  , ads_proceeds_to_date_()
{}

const std::int32_t starsTransaction::ID;

object_ptr<starsTransaction> starsTransaction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsTransaction> res = make_tl_object<starsTransaction>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->refund_ = (var0 & 8) != 0;
  res->pending_ = (var0 & 16) != 0;
  res->failed_ = (var0 & 64) != 0;
  res->gift_ = (var0 & 1024) != 0;
  res->reaction_ = (var0 & 2048) != 0;
  res->stargift_upgrade_ = (var0 & 262144) != 0;
  res->business_transfer_ = (var0 & 2097152) != 0;
  res->stargift_resale_ = (var0 & 4194304) != 0;
  res->posts_search_ = (var0 & 16777216) != 0;
  res->stargift_prepaid_upgrade_ = (var0 & 33554432) != 0;
  res->stargift_drop_original_details_ = (var0 & 67108864) != 0;
  res->id_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchObject<StarsAmount>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->peer_ = TlFetchObject<StarsTransactionPeer>::parse(p);
  if (var0 & 1) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->description_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  if (var0 & 32) { res->transaction_date_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->transaction_url_ = TlFetchString<string>::parse(p); }
  if (var0 & 128) { res->bot_payload_ = TlFetchBytes<bytes>::parse(p); }
  if (var0 & 256) { res->msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 512) { res->extended_media_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageMedia>>, 481674261>::parse(p); }
  if (var0 & 4096) { res->subscription_period_ = TlFetchInt::parse(p); }
  if (var0 & 8192) { res->giveaway_post_id_ = TlFetchInt::parse(p); }
  if (var0 & 16384) { res->stargift_ = TlFetchObject<StarGift>::parse(p); }
  if (var0 & 32768) { res->floodskip_number_ = TlFetchInt::parse(p); }
  if (var0 & 65536) { res->starref_commission_permille_ = TlFetchInt::parse(p); }
  if (var0 & 131072) { res->starref_peer_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 131072) { res->starref_amount_ = TlFetchObject<StarsAmount>::parse(p); }
  if (var0 & 524288) { res->paid_messages_ = TlFetchInt::parse(p); }
  if (var0 & 1048576) { res->premium_gift_months_ = TlFetchInt::parse(p); }
  if (var0 & 8388608) { res->ads_proceeds_from_date_ = TlFetchInt::parse(p); }
  if (var0 & 8388608) { res->ads_proceeds_to_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starsTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransaction");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (refund_ << 3) | (pending_ << 4) | (failed_ << 6) | (gift_ << 10) | (reaction_ << 11) | (stargift_upgrade_ << 18) | (business_transfer_ << 21) | (stargift_resale_ << 22) | (posts_search_ << 24) | (stargift_prepaid_upgrade_ << 25) | (stargift_drop_original_details_ << 26)));
    if (var0 & 8) { s.store_field("refund", true); }
    if (var0 & 16) { s.store_field("pending", true); }
    if (var0 & 64) { s.store_field("failed", true); }
    if (var0 & 1024) { s.store_field("gift", true); }
    if (var0 & 2048) { s.store_field("reaction", true); }
    if (var0 & 262144) { s.store_field("stargift_upgrade", true); }
    if (var0 & 2097152) { s.store_field("business_transfer", true); }
    if (var0 & 4194304) { s.store_field("stargift_resale", true); }
    if (var0 & 16777216) { s.store_field("posts_search", true); }
    if (var0 & 33554432) { s.store_field("stargift_prepaid_upgrade", true); }
    if (var0 & 67108864) { s.store_field("stargift_drop_original_details", true); }
    s.store_field("id", id_);
    s.store_object_field("amount", static_cast<const BaseObject *>(amount_.get()));
    s.store_field("date", date_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("description", description_); }
    if (var0 & 4) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 32) { s.store_field("transaction_date", transaction_date_); }
    if (var0 & 32) { s.store_field("transaction_url", transaction_url_); }
    if (var0 & 128) { s.store_bytes_field("bot_payload", bot_payload_); }
    if (var0 & 256) { s.store_field("msg_id", msg_id_); }
    if (var0 & 512) { { s.store_vector_begin("extended_media", extended_media_.size()); for (const auto &_value : extended_media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4096) { s.store_field("subscription_period", subscription_period_); }
    if (var0 & 8192) { s.store_field("giveaway_post_id", giveaway_post_id_); }
    if (var0 & 16384) { s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get())); }
    if (var0 & 32768) { s.store_field("floodskip_number", floodskip_number_); }
    if (var0 & 65536) { s.store_field("starref_commission_permille", starref_commission_permille_); }
    if (var0 & 131072) { s.store_object_field("starref_peer", static_cast<const BaseObject *>(starref_peer_.get())); }
    if (var0 & 131072) { s.store_object_field("starref_amount", static_cast<const BaseObject *>(starref_amount_.get())); }
    if (var0 & 524288) { s.store_field("paid_messages", paid_messages_); }
    if (var0 & 1048576) { s.store_field("premium_gift_months", premium_gift_months_); }
    if (var0 & 8388608) { s.store_field("ads_proceeds_from_date", ads_proceeds_from_date_); }
    if (var0 & 8388608) { s.store_field("ads_proceeds_to_date", ads_proceeds_to_date_); }
    s.store_class_end();
  }
}

theme::theme()
  : flags_()
  , creator_()
  , default_()
  , for_chat_()
  , id_()
  , access_hash_()
  , slug_()
  , title_()
  , document_()
  , settings_()
  , emoticon_()
  , installs_count_()
{}

const std::int32_t theme::ID;

object_ptr<theme> theme::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<theme> res = make_tl_object<theme>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->creator_ = (var0 & 1) != 0;
  res->default_ = (var0 & 2) != 0;
  res->for_chat_ = (var0 & 32) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->slug_ = TlFetchString<string>::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  if (var0 & 4) { res->document_ = TlFetchObject<Document>::parse(p); }
  if (var0 & 8) { res->settings_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<themeSettings>, -94849324>>, 481674261>::parse(p); }
  if (var0 & 64) { res->emoticon_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->installs_count_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void theme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "theme");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (creator_ << 0) | (default_ << 1) | (for_chat_ << 5)));
    if (var0 & 1) { s.store_field("creator", true); }
    if (var0 & 2) { s.store_field("default", true); }
    if (var0 & 32) { s.store_field("for_chat", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("slug", slug_);
    s.store_field("title", title_);
    if (var0 & 4) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 8) { { s.store_vector_begin("settings", settings_.size()); for (const auto &_value : settings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 64) { s.store_field("emoticon", emoticon_); }
    if (var0 & 16) { s.store_field("installs_count", installs_count_); }
    s.store_class_end();
  }
}

const std::int32_t topPeer::ID;

object_ptr<topPeer> topPeer::fetch(TlBufferParser &p) {
  return make_tl_object<topPeer>(p);
}

topPeer::topPeer(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , rating_(TlFetchDouble::parse(p))
{}

void topPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("rating", rating_);
    s.store_class_end();
  }
}

object_ptr<Update> Update::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case updateNewMessage::ID:
      return updateNewMessage::fetch(p);
    case updateMessageID::ID:
      return updateMessageID::fetch(p);
    case updateDeleteMessages::ID:
      return updateDeleteMessages::fetch(p);
    case updateUserTyping::ID:
      return updateUserTyping::fetch(p);
    case updateChatUserTyping::ID:
      return updateChatUserTyping::fetch(p);
    case updateChatParticipants::ID:
      return updateChatParticipants::fetch(p);
    case updateUserStatus::ID:
      return updateUserStatus::fetch(p);
    case updateUserName::ID:
      return updateUserName::fetch(p);
    case updateNewAuthorization::ID:
      return updateNewAuthorization::fetch(p);
    case updateNewEncryptedMessage::ID:
      return updateNewEncryptedMessage::fetch(p);
    case updateEncryptedChatTyping::ID:
      return updateEncryptedChatTyping::fetch(p);
    case updateEncryption::ID:
      return updateEncryption::fetch(p);
    case updateEncryptedMessagesRead::ID:
      return updateEncryptedMessagesRead::fetch(p);
    case updateChatParticipantAdd::ID:
      return updateChatParticipantAdd::fetch(p);
    case updateChatParticipantDelete::ID:
      return updateChatParticipantDelete::fetch(p);
    case updateDcOptions::ID:
      return updateDcOptions::fetch(p);
    case updateNotifySettings::ID:
      return updateNotifySettings::fetch(p);
    case updateServiceNotification::ID:
      return updateServiceNotification::fetch(p);
    case updatePrivacy::ID:
      return updatePrivacy::fetch(p);
    case updateUserPhone::ID:
      return updateUserPhone::fetch(p);
    case updateReadHistoryInbox::ID:
      return updateReadHistoryInbox::fetch(p);
    case updateReadHistoryOutbox::ID:
      return updateReadHistoryOutbox::fetch(p);
    case updateWebPage::ID:
      return updateWebPage::fetch(p);
    case updateReadMessagesContents::ID:
      return updateReadMessagesContents::fetch(p);
    case updateChannelTooLong::ID:
      return updateChannelTooLong::fetch(p);
    case updateChannel::ID:
      return updateChannel::fetch(p);
    case updateNewChannelMessage::ID:
      return updateNewChannelMessage::fetch(p);
    case updateReadChannelInbox::ID:
      return updateReadChannelInbox::fetch(p);
    case updateDeleteChannelMessages::ID:
      return updateDeleteChannelMessages::fetch(p);
    case updateChannelMessageViews::ID:
      return updateChannelMessageViews::fetch(p);
    case updateChatParticipantAdmin::ID:
      return updateChatParticipantAdmin::fetch(p);
    case updateNewStickerSet::ID:
      return updateNewStickerSet::fetch(p);
    case updateStickerSetsOrder::ID:
      return updateStickerSetsOrder::fetch(p);
    case updateStickerSets::ID:
      return updateStickerSets::fetch(p);
    case updateSavedGifs::ID:
      return updateSavedGifs::fetch(p);
    case updateBotInlineQuery::ID:
      return updateBotInlineQuery::fetch(p);
    case updateBotInlineSend::ID:
      return updateBotInlineSend::fetch(p);
    case updateEditChannelMessage::ID:
      return updateEditChannelMessage::fetch(p);
    case updateBotCallbackQuery::ID:
      return updateBotCallbackQuery::fetch(p);
    case updateEditMessage::ID:
      return updateEditMessage::fetch(p);
    case updateInlineBotCallbackQuery::ID:
      return updateInlineBotCallbackQuery::fetch(p);
    case updateReadChannelOutbox::ID:
      return updateReadChannelOutbox::fetch(p);
    case updateDraftMessage::ID:
      return updateDraftMessage::fetch(p);
    case updateReadFeaturedStickers::ID:
      return updateReadFeaturedStickers::fetch(p);
    case updateRecentStickers::ID:
      return updateRecentStickers::fetch(p);
    case updateConfig::ID:
      return updateConfig::fetch(p);
    case updatePtsChanged::ID:
      return updatePtsChanged::fetch(p);
    case updateChannelWebPage::ID:
      return updateChannelWebPage::fetch(p);
    case updateDialogPinned::ID:
      return updateDialogPinned::fetch(p);
    case updatePinnedDialogs::ID:
      return updatePinnedDialogs::fetch(p);
    case updateBotWebhookJSON::ID:
      return updateBotWebhookJSON::fetch(p);
    case updateBotWebhookJSONQuery::ID:
      return updateBotWebhookJSONQuery::fetch(p);
    case updateBotShippingQuery::ID:
      return updateBotShippingQuery::fetch(p);
    case updateBotPrecheckoutQuery::ID:
      return updateBotPrecheckoutQuery::fetch(p);
    case updatePhoneCall::ID:
      return updatePhoneCall::fetch(p);
    case updateLangPackTooLong::ID:
      return updateLangPackTooLong::fetch(p);
    case updateLangPack::ID:
      return updateLangPack::fetch(p);
    case updateFavedStickers::ID:
      return updateFavedStickers::fetch(p);
    case updateChannelReadMessagesContents::ID:
      return updateChannelReadMessagesContents::fetch(p);
    case updateContactsReset::ID:
      return updateContactsReset::fetch(p);
    case updateChannelAvailableMessages::ID:
      return updateChannelAvailableMessages::fetch(p);
    case updateDialogUnreadMark::ID:
      return updateDialogUnreadMark::fetch(p);
    case updateMessagePoll::ID:
      return updateMessagePoll::fetch(p);
    case updateChatDefaultBannedRights::ID:
      return updateChatDefaultBannedRights::fetch(p);
    case updateFolderPeers::ID:
      return updateFolderPeers::fetch(p);
    case updatePeerSettings::ID:
      return updatePeerSettings::fetch(p);
    case updatePeerLocated::ID:
      return updatePeerLocated::fetch(p);
    case updateNewScheduledMessage::ID:
      return updateNewScheduledMessage::fetch(p);
    case updateDeleteScheduledMessages::ID:
      return updateDeleteScheduledMessages::fetch(p);
    case updateTheme::ID:
      return updateTheme::fetch(p);
    case updateGeoLiveViewed::ID:
      return updateGeoLiveViewed::fetch(p);
    case updateLoginToken::ID:
      return updateLoginToken::fetch(p);
    case updateMessagePollVote::ID:
      return updateMessagePollVote::fetch(p);
    case updateDialogFilter::ID:
      return updateDialogFilter::fetch(p);
    case updateDialogFilterOrder::ID:
      return updateDialogFilterOrder::fetch(p);
    case updateDialogFilters::ID:
      return updateDialogFilters::fetch(p);
    case updatePhoneCallSignalingData::ID:
      return updatePhoneCallSignalingData::fetch(p);
    case updateChannelMessageForwards::ID:
      return updateChannelMessageForwards::fetch(p);
    case updateReadChannelDiscussionInbox::ID:
      return updateReadChannelDiscussionInbox::fetch(p);
    case updateReadChannelDiscussionOutbox::ID:
      return updateReadChannelDiscussionOutbox::fetch(p);
    case updatePeerBlocked::ID:
      return updatePeerBlocked::fetch(p);
    case updateChannelUserTyping::ID:
      return updateChannelUserTyping::fetch(p);
    case updatePinnedMessages::ID:
      return updatePinnedMessages::fetch(p);
    case updatePinnedChannelMessages::ID:
      return updatePinnedChannelMessages::fetch(p);
    case updateChat::ID:
      return updateChat::fetch(p);
    case updateGroupCallParticipants::ID:
      return updateGroupCallParticipants::fetch(p);
    case updateGroupCall::ID:
      return updateGroupCall::fetch(p);
    case updatePeerHistoryTTL::ID:
      return updatePeerHistoryTTL::fetch(p);
    case updateChatParticipant::ID:
      return updateChatParticipant::fetch(p);
    case updateChannelParticipant::ID:
      return updateChannelParticipant::fetch(p);
    case updateBotStopped::ID:
      return updateBotStopped::fetch(p);
    case updateGroupCallConnection::ID:
      return updateGroupCallConnection::fetch(p);
    case updateBotCommands::ID:
      return updateBotCommands::fetch(p);
    case updatePendingJoinRequests::ID:
      return updatePendingJoinRequests::fetch(p);
    case updateBotChatInviteRequester::ID:
      return updateBotChatInviteRequester::fetch(p);
    case updateMessageReactions::ID:
      return updateMessageReactions::fetch(p);
    case updateAttachMenuBots::ID:
      return updateAttachMenuBots::fetch(p);
    case updateWebViewResultSent::ID:
      return updateWebViewResultSent::fetch(p);
    case updateBotMenuButton::ID:
      return updateBotMenuButton::fetch(p);
    case updateSavedRingtones::ID:
      return updateSavedRingtones::fetch(p);
    case updateTranscribedAudio::ID:
      return updateTranscribedAudio::fetch(p);
    case updateReadFeaturedEmojiStickers::ID:
      return updateReadFeaturedEmojiStickers::fetch(p);
    case updateUserEmojiStatus::ID:
      return updateUserEmojiStatus::fetch(p);
    case updateRecentEmojiStatuses::ID:
      return updateRecentEmojiStatuses::fetch(p);
    case updateRecentReactions::ID:
      return updateRecentReactions::fetch(p);
    case updateMoveStickerSetToTop::ID:
      return updateMoveStickerSetToTop::fetch(p);
    case updateMessageExtendedMedia::ID:
      return updateMessageExtendedMedia::fetch(p);
    case updateUser::ID:
      return updateUser::fetch(p);
    case updateAutoSaveSettings::ID:
      return updateAutoSaveSettings::fetch(p);
    case updateStory::ID:
      return updateStory::fetch(p);
    case updateReadStories::ID:
      return updateReadStories::fetch(p);
    case updateStoryID::ID:
      return updateStoryID::fetch(p);
    case updateStoriesStealthMode::ID:
      return updateStoriesStealthMode::fetch(p);
    case updateSentStoryReaction::ID:
      return updateSentStoryReaction::fetch(p);
    case updateBotChatBoost::ID:
      return updateBotChatBoost::fetch(p);
    case updateChannelViewForumAsMessages::ID:
      return updateChannelViewForumAsMessages::fetch(p);
    case updatePeerWallpaper::ID:
      return updatePeerWallpaper::fetch(p);
    case updateBotMessageReaction::ID:
      return updateBotMessageReaction::fetch(p);
    case updateBotMessageReactions::ID:
      return updateBotMessageReactions::fetch(p);
    case updateSavedDialogPinned::ID:
      return updateSavedDialogPinned::fetch(p);
    case updatePinnedSavedDialogs::ID:
      return updatePinnedSavedDialogs::fetch(p);
    case updateSavedReactionTags::ID:
      return updateSavedReactionTags::fetch(p);
    case updateSmsJob::ID:
      return updateSmsJob::fetch(p);
    case updateQuickReplies::ID:
      return updateQuickReplies::fetch(p);
    case updateNewQuickReply::ID:
      return updateNewQuickReply::fetch(p);
    case updateDeleteQuickReply::ID:
      return updateDeleteQuickReply::fetch(p);
    case updateQuickReplyMessage::ID:
      return updateQuickReplyMessage::fetch(p);
    case updateDeleteQuickReplyMessages::ID:
      return updateDeleteQuickReplyMessages::fetch(p);
    case updateBotBusinessConnect::ID:
      return updateBotBusinessConnect::fetch(p);
    case updateBotNewBusinessMessage::ID:
      return updateBotNewBusinessMessage::fetch(p);
    case updateBotEditBusinessMessage::ID:
      return updateBotEditBusinessMessage::fetch(p);
    case updateBotDeleteBusinessMessage::ID:
      return updateBotDeleteBusinessMessage::fetch(p);
    case updateNewStoryReaction::ID:
      return updateNewStoryReaction::fetch(p);
    case updateStarsBalance::ID:
      return updateStarsBalance::fetch(p);
    case updateBusinessBotCallbackQuery::ID:
      return updateBusinessBotCallbackQuery::fetch(p);
    case updateStarsRevenueStatus::ID:
      return updateStarsRevenueStatus::fetch(p);
    case updateBotPurchasedPaidMedia::ID:
      return updateBotPurchasedPaidMedia::fetch(p);
    case updatePaidReactionPrivacy::ID:
      return updatePaidReactionPrivacy::fetch(p);
    case updateSentPhoneCode::ID:
      return updateSentPhoneCode::fetch(p);
    case updateGroupCallChainBlocks::ID:
      return updateGroupCallChainBlocks::fetch(p);
    case updateReadMonoForumInbox::ID:
      return updateReadMonoForumInbox::fetch(p);
    case updateReadMonoForumOutbox::ID:
      return updateReadMonoForumOutbox::fetch(p);
    case updateMonoForumNoPaidException::ID:
      return updateMonoForumNoPaidException::fetch(p);
    case updateGroupCallMessage::ID:
      return updateGroupCallMessage::fetch(p);
    case updateGroupCallEncryptedMessage::ID:
      return updateGroupCallEncryptedMessage::fetch(p);
    case updatePinnedForumTopic::ID:
      return updatePinnedForumTopic::fetch(p);
    case updatePinnedForumTopics::ID:
      return updatePinnedForumTopics::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

updateNewMessage::updateNewMessage(object_ptr<Message> &&message_, int32 pts_, int32 pts_count_)
  : message_(std::move(message_))
  , pts_(pts_)
  , pts_count_(pts_count_)
{}

const std::int32_t updateNewMessage::ID;

object_ptr<Update> updateNewMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateNewMessage>(p);
}

updateNewMessage::updateNewMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateNewMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

const std::int32_t updateMessageID::ID;

object_ptr<Update> updateMessageID::fetch(TlBufferParser &p) {
  return make_tl_object<updateMessageID>(p);
}

updateMessageID::updateMessageID(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
  , random_id_(TlFetchLong::parse(p))
{}

void updateMessageID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageID");
    s.store_field("id", id_);
    s.store_field("random_id", random_id_);
    s.store_class_end();
  }
}

updateDeleteMessages::updateDeleteMessages(array<int32> &&messages_, int32 pts_, int32 pts_count_)
  : messages_(std::move(messages_))
  , pts_(pts_)
  , pts_count_(pts_count_)
{}

const std::int32_t updateDeleteMessages::ID;

object_ptr<Update> updateDeleteMessages::fetch(TlBufferParser &p) {
  return make_tl_object<updateDeleteMessages>(p);
}

updateDeleteMessages::updateDeleteMessages(TlBufferParser &p)
  : messages_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateDeleteMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDeleteMessages");
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

updateUserTyping::updateUserTyping()
  : flags_()
  , user_id_()
  , top_msg_id_()
  , action_()
{}

const std::int32_t updateUserTyping::ID;

object_ptr<Update> updateUserTyping::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateUserTyping> res = make_tl_object<updateUserTyping>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->user_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->top_msg_id_ = TlFetchInt::parse(p); }
  res->action_ = TlFetchObject<SendMessageAction>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateUserTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserTyping");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("user_id", user_id_);
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateChatUserTyping::ID;

object_ptr<Update> updateChatUserTyping::fetch(TlBufferParser &p) {
  return make_tl_object<updateChatUserTyping>(p);
}

updateChatUserTyping::updateChatUserTyping(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
  , from_id_(TlFetchObject<Peer>::parse(p))
  , action_(TlFetchObject<SendMessageAction>::parse(p))
{}

void updateChatUserTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatUserTyping");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get()));
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateChatParticipants::ID;

object_ptr<Update> updateChatParticipants::fetch(TlBufferParser &p) {
  return make_tl_object<updateChatParticipants>(p);
}

updateChatParticipants::updateChatParticipants(TlBufferParser &p)
  : participants_(TlFetchObject<ChatParticipants>::parse(p))
{}

void updateChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatParticipants");
    s.store_object_field("participants", static_cast<const BaseObject *>(participants_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateUserStatus::ID;

object_ptr<Update> updateUserStatus::fetch(TlBufferParser &p) {
  return make_tl_object<updateUserStatus>(p);
}

updateUserStatus::updateUserStatus(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , status_(TlFetchObject<UserStatus>::parse(p))
{}

void updateUserStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserStatus");
    s.store_field("user_id", user_id_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateUserName::ID;

object_ptr<Update> updateUserName::fetch(TlBufferParser &p) {
  return make_tl_object<updateUserName>(p);
}

updateUserName::updateUserName(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , first_name_(TlFetchString<string>::parse(p))
  , last_name_(TlFetchString<string>::parse(p))
  , usernames_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<username>, -1274595769>>, 481674261>::parse(p))
{}

void updateUserName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserName");
    s.store_field("user_id", user_id_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    { s.store_vector_begin("usernames", usernames_.size()); for (const auto &_value : usernames_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateNewAuthorization::updateNewAuthorization()
  : flags_()
  , unconfirmed_()
  , hash_()
  , date_()
  , device_()
  , location_()
{}

const std::int32_t updateNewAuthorization::ID;

object_ptr<Update> updateNewAuthorization::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateNewAuthorization> res = make_tl_object<updateNewAuthorization>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->unconfirmed_ = (var0 & 1) != 0;
  res->hash_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->date_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->device_ = TlFetchString<string>::parse(p); }
  if (var0 & 1) { res->location_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateNewAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewAuthorization");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (unconfirmed_ << 0)));
    if (var0 & 1) { s.store_field("unconfirmed", true); }
    s.store_field("hash", hash_);
    if (var0 & 1) { s.store_field("date", date_); }
    if (var0 & 1) { s.store_field("device", device_); }
    if (var0 & 1) { s.store_field("location", location_); }
    s.store_class_end();
  }
}

const std::int32_t updateNewEncryptedMessage::ID;

object_ptr<Update> updateNewEncryptedMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateNewEncryptedMessage>(p);
}

updateNewEncryptedMessage::updateNewEncryptedMessage(TlBufferParser &p)
  : message_(TlFetchObject<EncryptedMessage>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateNewEncryptedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewEncryptedMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

const std::int32_t updateEncryptedChatTyping::ID;

object_ptr<Update> updateEncryptedChatTyping::fetch(TlBufferParser &p) {
  return make_tl_object<updateEncryptedChatTyping>(p);
}

updateEncryptedChatTyping::updateEncryptedChatTyping(TlBufferParser &p)
  : chat_id_(TlFetchInt::parse(p))
{}

void updateEncryptedChatTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateEncryptedChatTyping");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

const std::int32_t updateEncryption::ID;

object_ptr<Update> updateEncryption::fetch(TlBufferParser &p) {
  return make_tl_object<updateEncryption>(p);
}

updateEncryption::updateEncryption(TlBufferParser &p)
  : chat_(TlFetchObject<EncryptedChat>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void updateEncryption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateEncryption");
    s.store_object_field("chat", static_cast<const BaseObject *>(chat_.get()));
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t updateEncryptedMessagesRead::ID;

object_ptr<Update> updateEncryptedMessagesRead::fetch(TlBufferParser &p) {
  return make_tl_object<updateEncryptedMessagesRead>(p);
}

updateEncryptedMessagesRead::updateEncryptedMessagesRead(TlBufferParser &p)
  : chat_id_(TlFetchInt::parse(p))
  , max_date_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void updateEncryptedMessagesRead::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateEncryptedMessagesRead");
    s.store_field("chat_id", chat_id_);
    s.store_field("max_date", max_date_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t updateChatParticipantAdd::ID;

object_ptr<Update> updateChatParticipantAdd::fetch(TlBufferParser &p) {
  return make_tl_object<updateChatParticipantAdd>(p);
}

updateChatParticipantAdd::updateChatParticipantAdd(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , inviter_id_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
  , version_(TlFetchInt::parse(p))
{}

void updateChatParticipantAdd::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatParticipantAdd");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_field("inviter_id", inviter_id_);
    s.store_field("date", date_);
    s.store_field("version", version_);
    s.store_class_end();
  }
}

const std::int32_t updateChatParticipantDelete::ID;

object_ptr<Update> updateChatParticipantDelete::fetch(TlBufferParser &p) {
  return make_tl_object<updateChatParticipantDelete>(p);
}

updateChatParticipantDelete::updateChatParticipantDelete(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , version_(TlFetchInt::parse(p))
{}

void updateChatParticipantDelete::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatParticipantDelete");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_field("version", version_);
    s.store_class_end();
  }
}

const std::int32_t updateDcOptions::ID;

object_ptr<Update> updateDcOptions::fetch(TlBufferParser &p) {
  return make_tl_object<updateDcOptions>(p);
}

updateDcOptions::updateDcOptions(TlBufferParser &p)
  : dc_options_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<dcOption>, 414687501>>, 481674261>::parse(p))
{}

void updateDcOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDcOptions");
    { s.store_vector_begin("dc_options", dc_options_.size()); for (const auto &_value : dc_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateNotifySettings::ID;

object_ptr<Update> updateNotifySettings::fetch(TlBufferParser &p) {
  return make_tl_object<updateNotifySettings>(p);
}

updateNotifySettings::updateNotifySettings(TlBufferParser &p)
  : peer_(TlFetchObject<NotifyPeer>::parse(p))
  , notify_settings_(TlFetchBoxed<TlFetchObject<peerNotifySettings>, -1721619444>::parse(p))
{}

void updateNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNotifySettings");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("notify_settings", static_cast<const BaseObject *>(notify_settings_.get()));
    s.store_class_end();
  }
}

updateServiceNotification::updateServiceNotification()
  : flags_()
  , popup_()
  , invert_media_()
  , inbox_date_()
  , type_()
  , message_()
  , media_()
  , entities_()
{}

updateServiceNotification::updateServiceNotification(int32 flags_, bool popup_, bool invert_media_, int32 inbox_date_, string const &type_, string const &message_, object_ptr<MessageMedia> &&media_, array<object_ptr<MessageEntity>> &&entities_)
  : flags_(flags_)
  , popup_(popup_)
  , invert_media_(invert_media_)
  , inbox_date_(inbox_date_)
  , type_(type_)
  , message_(message_)
  , media_(std::move(media_))
  , entities_(std::move(entities_))
{}

const std::int32_t updateServiceNotification::ID;

object_ptr<Update> updateServiceNotification::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateServiceNotification> res = make_tl_object<updateServiceNotification>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->popup_ = (var0 & 1) != 0;
  res->invert_media_ = (var0 & 4) != 0;
  if (var0 & 2) { res->inbox_date_ = TlFetchInt::parse(p); }
  res->type_ = TlFetchString<string>::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  res->media_ = TlFetchObject<MessageMedia>::parse(p);
  res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateServiceNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateServiceNotification");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (popup_ << 0) | (invert_media_ << 2)));
    if (var0 & 1) { s.store_field("popup", true); }
    if (var0 & 4) { s.store_field("invert_media", true); }
    if (var0 & 2) { s.store_field("inbox_date", inbox_date_); }
    s.store_field("type", type_);
    s.store_field("message", message_);
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updatePrivacy::ID;

object_ptr<Update> updatePrivacy::fetch(TlBufferParser &p) {
  return make_tl_object<updatePrivacy>(p);
}

updatePrivacy::updatePrivacy(TlBufferParser &p)
  : key_(TlFetchObject<PrivacyKey>::parse(p))
  , rules_(TlFetchBoxed<TlFetchVector<TlFetchObject<PrivacyRule>>, 481674261>::parse(p))
{}

void updatePrivacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePrivacy");
    s.store_object_field("key", static_cast<const BaseObject *>(key_.get()));
    { s.store_vector_begin("rules", rules_.size()); for (const auto &_value : rules_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateUserPhone::ID;

object_ptr<Update> updateUserPhone::fetch(TlBufferParser &p) {
  return make_tl_object<updateUserPhone>(p);
}

updateUserPhone::updateUserPhone(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , phone_(TlFetchString<string>::parse(p))
{}

void updateUserPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserPhone");
    s.store_field("user_id", user_id_);
    s.store_field("phone", phone_);
    s.store_class_end();
  }
}

updateReadHistoryInbox::updateReadHistoryInbox()
  : flags_()
  , folder_id_()
  , peer_()
  , top_msg_id_()
  , max_id_()
  , still_unread_count_()
  , pts_()
  , pts_count_()
{}

const std::int32_t updateReadHistoryInbox::ID;

object_ptr<Update> updateReadHistoryInbox::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateReadHistoryInbox> res = make_tl_object<updateReadHistoryInbox>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->folder_id_ = TlFetchInt::parse(p); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 2) { res->top_msg_id_ = TlFetchInt::parse(p); }
  res->max_id_ = TlFetchInt::parse(p);
  res->still_unread_count_ = TlFetchInt::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  res->pts_count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateReadHistoryInbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadHistoryInbox");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("folder_id", folder_id_); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 2) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_field("max_id", max_id_);
    s.store_field("still_unread_count", still_unread_count_);
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

const std::int32_t updateReadHistoryOutbox::ID;

object_ptr<Update> updateReadHistoryOutbox::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadHistoryOutbox>(p);
}

updateReadHistoryOutbox::updateReadHistoryOutbox(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , max_id_(TlFetchInt::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateReadHistoryOutbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadHistoryOutbox");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_id", max_id_);
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

const std::int32_t updateWebPage::ID;

object_ptr<Update> updateWebPage::fetch(TlBufferParser &p) {
  return make_tl_object<updateWebPage>(p);
}

updateWebPage::updateWebPage(TlBufferParser &p)
  : webpage_(TlFetchObject<WebPage>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateWebPage");
    s.store_object_field("webpage", static_cast<const BaseObject *>(webpage_.get()));
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

updateReadMessagesContents::updateReadMessagesContents()
  : flags_()
  , messages_()
  , pts_()
  , pts_count_()
  , date_()
{}

const std::int32_t updateReadMessagesContents::ID;

object_ptr<Update> updateReadMessagesContents::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateReadMessagesContents> res = make_tl_object<updateReadMessagesContents>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  res->pts_count_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateReadMessagesContents::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadMessagesContents");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    if (var0 & 1) { s.store_field("date", date_); }
    s.store_class_end();
  }
}

updateChannelTooLong::updateChannelTooLong()
  : flags_()
  , channel_id_()
  , pts_()
{}

updateChannelTooLong::updateChannelTooLong(int32 flags_, int64 channel_id_, int32 pts_)
  : flags_(flags_)
  , channel_id_(channel_id_)
  , pts_(pts_)
{}

const std::int32_t updateChannelTooLong::ID;

object_ptr<Update> updateChannelTooLong::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateChannelTooLong> res = make_tl_object<updateChannelTooLong>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->channel_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->pts_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateChannelTooLong::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelTooLong");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("channel_id", channel_id_);
    if (var0 & 1) { s.store_field("pts", pts_); }
    s.store_class_end();
  }
}

const std::int32_t updateChannel::ID;

object_ptr<Update> updateChannel::fetch(TlBufferParser &p) {
  return make_tl_object<updateChannel>(p);
}

updateChannel::updateChannel(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
{}

void updateChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannel");
    s.store_field("channel_id", channel_id_);
    s.store_class_end();
  }
}

const std::int32_t updateNewChannelMessage::ID;

object_ptr<Update> updateNewChannelMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateNewChannelMessage>(p);
}

updateNewChannelMessage::updateNewChannelMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateNewChannelMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewChannelMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

updateReadChannelInbox::updateReadChannelInbox()
  : flags_()
  , folder_id_()
  , channel_id_()
  , max_id_()
  , still_unread_count_()
  , pts_()
{}

const std::int32_t updateReadChannelInbox::ID;

object_ptr<Update> updateReadChannelInbox::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateReadChannelInbox> res = make_tl_object<updateReadChannelInbox>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->folder_id_ = TlFetchInt::parse(p); }
  res->channel_id_ = TlFetchLong::parse(p);
  res->max_id_ = TlFetchInt::parse(p);
  res->still_unread_count_ = TlFetchInt::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateReadChannelInbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadChannelInbox");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("folder_id", folder_id_); }
    s.store_field("channel_id", channel_id_);
    s.store_field("max_id", max_id_);
    s.store_field("still_unread_count", still_unread_count_);
    s.store_field("pts", pts_);
    s.store_class_end();
  }
}

const std::int32_t updateDeleteChannelMessages::ID;

object_ptr<Update> updateDeleteChannelMessages::fetch(TlBufferParser &p) {
  return make_tl_object<updateDeleteChannelMessages>(p);
}

updateDeleteChannelMessages::updateDeleteChannelMessages(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateDeleteChannelMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDeleteChannelMessages");
    s.store_field("channel_id", channel_id_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

const std::int32_t updateChannelMessageViews::ID;

object_ptr<Update> updateChannelMessageViews::fetch(TlBufferParser &p) {
  return make_tl_object<updateChannelMessageViews>(p);
}

updateChannelMessageViews::updateChannelMessageViews(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , id_(TlFetchInt::parse(p))
  , views_(TlFetchInt::parse(p))
{}

void updateChannelMessageViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelMessageViews");
    s.store_field("channel_id", channel_id_);
    s.store_field("id", id_);
    s.store_field("views", views_);
    s.store_class_end();
  }
}

const std::int32_t updateChatParticipantAdmin::ID;

object_ptr<Update> updateChatParticipantAdmin::fetch(TlBufferParser &p) {
  return make_tl_object<updateChatParticipantAdmin>(p);
}

updateChatParticipantAdmin::updateChatParticipantAdmin(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , is_admin_(TlFetchBool::parse(p))
  , version_(TlFetchInt::parse(p))
{}

void updateChatParticipantAdmin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatParticipantAdmin");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_field("is_admin", is_admin_);
    s.store_field("version", version_);
    s.store_class_end();
  }
}

const std::int32_t updateNewStickerSet::ID;

object_ptr<Update> updateNewStickerSet::fetch(TlBufferParser &p) {
  return make_tl_object<updateNewStickerSet>(p);
}

updateNewStickerSet::updateNewStickerSet(TlBufferParser &p)
  : stickerset_(TlFetchObject<messages_StickerSet>::parse(p))
{}

void updateNewStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewStickerSet");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_class_end();
  }
}

updateStickerSetsOrder::updateStickerSetsOrder()
  : flags_()
  , masks_()
  , emojis_()
  , order_()
{}

const std::int32_t updateStickerSetsOrder::ID;

object_ptr<Update> updateStickerSetsOrder::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateStickerSetsOrder> res = make_tl_object<updateStickerSetsOrder>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->masks_ = (var0 & 1) != 0;
  res->emojis_ = (var0 & 2) != 0;
  res->order_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateStickerSetsOrder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStickerSetsOrder");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (masks_ << 0) | (emojis_ << 1)));
    if (var0 & 1) { s.store_field("masks", true); }
    if (var0 & 2) { s.store_field("emojis", true); }
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateStickerSets::updateStickerSets()
  : flags_()
  , masks_()
  , emojis_()
{}

const std::int32_t updateStickerSets::ID;

object_ptr<Update> updateStickerSets::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateStickerSets> res = make_tl_object<updateStickerSets>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->masks_ = (var0 & 1) != 0;
  res->emojis_ = (var0 & 2) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStickerSets");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (masks_ << 0) | (emojis_ << 1)));
    if (var0 & 1) { s.store_field("masks", true); }
    if (var0 & 2) { s.store_field("emojis", true); }
    s.store_class_end();
  }
}

const std::int32_t updateSavedGifs::ID;

object_ptr<Update> updateSavedGifs::fetch(TlBufferParser &p) {
  return make_tl_object<updateSavedGifs>();
}

void updateSavedGifs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedGifs");
    s.store_class_end();
  }
}

updateBotInlineQuery::updateBotInlineQuery()
  : flags_()
  , query_id_()
  , user_id_()
  , query_()
  , geo_()
  , peer_type_()
  , offset_()
{}

const std::int32_t updateBotInlineQuery::ID;

object_ptr<Update> updateBotInlineQuery::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateBotInlineQuery> res = make_tl_object<updateBotInlineQuery>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->query_id_ = TlFetchLong::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  res->query_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->geo_ = TlFetchObject<GeoPoint>::parse(p); }
  if (var0 & 2) { res->peer_type_ = TlFetchObject<InlineQueryPeerType>::parse(p); }
  res->offset_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateBotInlineQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotInlineQuery");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("query_id", query_id_);
    s.store_field("user_id", user_id_);
    s.store_field("query", query_);
    if (var0 & 1) { s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get())); }
    if (var0 & 2) { s.store_object_field("peer_type", static_cast<const BaseObject *>(peer_type_.get())); }
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

updateBotInlineSend::updateBotInlineSend()
  : flags_()
  , user_id_()
  , query_()
  , geo_()
  , id_()
  , msg_id_()
{}

const std::int32_t updateBotInlineSend::ID;

object_ptr<Update> updateBotInlineSend::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateBotInlineSend> res = make_tl_object<updateBotInlineSend>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->user_id_ = TlFetchLong::parse(p);
  res->query_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->geo_ = TlFetchObject<GeoPoint>::parse(p); }
  res->id_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->msg_id_ = TlFetchObject<InputBotInlineMessageID>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateBotInlineSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotInlineSend");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("user_id", user_id_);
    s.store_field("query", query_);
    if (var0 & 1) { s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get())); }
    s.store_field("id", id_);
    if (var0 & 2) { s.store_object_field("msg_id", static_cast<const BaseObject *>(msg_id_.get())); }
    s.store_class_end();
  }
}

updateEditChannelMessage::updateEditChannelMessage(object_ptr<Message> &&message_, int32 pts_, int32 pts_count_)
  : message_(std::move(message_))
  , pts_(pts_)
  , pts_count_(pts_count_)
{}

const std::int32_t updateEditChannelMessage::ID;

object_ptr<Update> updateEditChannelMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateEditChannelMessage>(p);
}

updateEditChannelMessage::updateEditChannelMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateEditChannelMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateEditChannelMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

updateBotCallbackQuery::updateBotCallbackQuery()
  : flags_()
  , query_id_()
  , user_id_()
  , peer_()
  , msg_id_()
  , chat_instance_()
  , data_()
  , game_short_name_()
{}

const std::int32_t updateBotCallbackQuery::ID;

object_ptr<Update> updateBotCallbackQuery::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateBotCallbackQuery> res = make_tl_object<updateBotCallbackQuery>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->query_id_ = TlFetchLong::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->msg_id_ = TlFetchInt::parse(p);
  res->chat_instance_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->data_ = TlFetchBytes<bytes>::parse(p); }
  if (var0 & 2) { res->game_short_name_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateBotCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotCallbackQuery");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("query_id", query_id_);
    s.store_field("user_id", user_id_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("chat_instance", chat_instance_);
    if (var0 & 1) { s.store_bytes_field("data", data_); }
    if (var0 & 2) { s.store_field("game_short_name", game_short_name_); }
    s.store_class_end();
  }
}

const std::int32_t updateEditMessage::ID;

object_ptr<Update> updateEditMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateEditMessage>(p);
}

updateEditMessage::updateEditMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateEditMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateEditMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

updateInlineBotCallbackQuery::updateInlineBotCallbackQuery()
  : flags_()
  , query_id_()
  , user_id_()
  , msg_id_()
  , chat_instance_()
  , data_()
  , game_short_name_()
{}

const std::int32_t updateInlineBotCallbackQuery::ID;

object_ptr<Update> updateInlineBotCallbackQuery::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateInlineBotCallbackQuery> res = make_tl_object<updateInlineBotCallbackQuery>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->query_id_ = TlFetchLong::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  res->msg_id_ = TlFetchObject<InputBotInlineMessageID>::parse(p);
  res->chat_instance_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->data_ = TlFetchBytes<bytes>::parse(p); }
  if (var0 & 2) { res->game_short_name_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateInlineBotCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateInlineBotCallbackQuery");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("query_id", query_id_);
    s.store_field("user_id", user_id_);
    s.store_object_field("msg_id", static_cast<const BaseObject *>(msg_id_.get()));
    s.store_field("chat_instance", chat_instance_);
    if (var0 & 1) { s.store_bytes_field("data", data_); }
    if (var0 & 2) { s.store_field("game_short_name", game_short_name_); }
    s.store_class_end();
  }
}

const std::int32_t updateReadChannelOutbox::ID;

object_ptr<Update> updateReadChannelOutbox::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadChannelOutbox>(p);
}

updateReadChannelOutbox::updateReadChannelOutbox(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , max_id_(TlFetchInt::parse(p))
{}

void updateReadChannelOutbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadChannelOutbox");
    s.store_field("channel_id", channel_id_);
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

updateDraftMessage::updateDraftMessage()
  : flags_()
  , peer_()
  , top_msg_id_()
  , saved_peer_id_()
  , draft_()
{}

const std::int32_t updateDraftMessage::ID;

object_ptr<Update> updateDraftMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateDraftMessage> res = make_tl_object<updateDraftMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 1) { res->top_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->saved_peer_id_ = TlFetchObject<Peer>::parse(p); }
  res->draft_ = TlFetchObject<DraftMessage>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateDraftMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDraftMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 2) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_object_field("draft", static_cast<const BaseObject *>(draft_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateReadFeaturedStickers::ID;

object_ptr<Update> updateReadFeaturedStickers::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadFeaturedStickers>();
}

void updateReadFeaturedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadFeaturedStickers");
    s.store_class_end();
  }
}

const std::int32_t updateRecentStickers::ID;

object_ptr<Update> updateRecentStickers::fetch(TlBufferParser &p) {
  return make_tl_object<updateRecentStickers>();
}

void updateRecentStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateRecentStickers");
    s.store_class_end();
  }
}

const std::int32_t updateConfig::ID;

object_ptr<Update> updateConfig::fetch(TlBufferParser &p) {
  return make_tl_object<updateConfig>();
}

void updateConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateConfig");
    s.store_class_end();
  }
}

const std::int32_t updatePtsChanged::ID;

object_ptr<Update> updatePtsChanged::fetch(TlBufferParser &p) {
  return make_tl_object<updatePtsChanged>();
}

void updatePtsChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePtsChanged");
    s.store_class_end();
  }
}

const std::int32_t updateChannelWebPage::ID;

object_ptr<Update> updateChannelWebPage::fetch(TlBufferParser &p) {
  return make_tl_object<updateChannelWebPage>(p);
}

updateChannelWebPage::updateChannelWebPage(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , webpage_(TlFetchObject<WebPage>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateChannelWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelWebPage");
    s.store_field("channel_id", channel_id_);
    s.store_object_field("webpage", static_cast<const BaseObject *>(webpage_.get()));
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

updateDialogPinned::updateDialogPinned()
  : flags_()
  , pinned_()
  , folder_id_()
  , peer_()
{}

const std::int32_t updateDialogPinned::ID;

object_ptr<Update> updateDialogPinned::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateDialogPinned> res = make_tl_object<updateDialogPinned>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 1) != 0;
  if (var0 & 2) { res->folder_id_ = TlFetchInt::parse(p); }
  res->peer_ = TlFetchObject<DialogPeer>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateDialogPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDialogPinned");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 0)));
    if (var0 & 1) { s.store_field("pinned", true); }
    if (var0 & 2) { s.store_field("folder_id", folder_id_); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

updatePinnedDialogs::updatePinnedDialogs()
  : flags_()
  , folder_id_()
  , order_()
{}

const std::int32_t updatePinnedDialogs::ID;

object_ptr<Update> updatePinnedDialogs::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePinnedDialogs> res = make_tl_object<updatePinnedDialogs>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 2) { res->folder_id_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->order_ = TlFetchBoxed<TlFetchVector<TlFetchObject<DialogPeer>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePinnedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePinnedDialogs");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 2) { s.store_field("folder_id", folder_id_); }
    if (var0 & 1) { { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

const std::int32_t updateBotWebhookJSON::ID;

object_ptr<Update> updateBotWebhookJSON::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotWebhookJSON>(p);
}

updateBotWebhookJSON::updateBotWebhookJSON(TlBufferParser &p)
  : data_(TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p))
{}

void updateBotWebhookJSON::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotWebhookJSON");
    s.store_object_field("data", static_cast<const BaseObject *>(data_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateBotWebhookJSONQuery::ID;

object_ptr<Update> updateBotWebhookJSONQuery::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotWebhookJSONQuery>(p);
}

updateBotWebhookJSONQuery::updateBotWebhookJSONQuery(TlBufferParser &p)
  : query_id_(TlFetchLong::parse(p))
  , data_(TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p))
  , timeout_(TlFetchInt::parse(p))
{}

void updateBotWebhookJSONQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotWebhookJSONQuery");
    s.store_field("query_id", query_id_);
    s.store_object_field("data", static_cast<const BaseObject *>(data_.get()));
    s.store_field("timeout", timeout_);
    s.store_class_end();
  }
}

const std::int32_t updateBotShippingQuery::ID;

object_ptr<Update> updateBotShippingQuery::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotShippingQuery>(p);
}

updateBotShippingQuery::updateBotShippingQuery(TlBufferParser &p)
  : query_id_(TlFetchLong::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , payload_(TlFetchBytes<bytes>::parse(p))
  , shipping_address_(TlFetchBoxed<TlFetchObject<postAddress>, 512535275>::parse(p))
{}

void updateBotShippingQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotShippingQuery");
    s.store_field("query_id", query_id_);
    s.store_field("user_id", user_id_);
    s.store_bytes_field("payload", payload_);
    s.store_object_field("shipping_address", static_cast<const BaseObject *>(shipping_address_.get()));
    s.store_class_end();
  }
}

updateBotPrecheckoutQuery::updateBotPrecheckoutQuery()
  : flags_()
  , query_id_()
  , user_id_()
  , payload_()
  , info_()
  , shipping_option_id_()
  , currency_()
  , total_amount_()
{}

const std::int32_t updateBotPrecheckoutQuery::ID;

object_ptr<Update> updateBotPrecheckoutQuery::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateBotPrecheckoutQuery> res = make_tl_object<updateBotPrecheckoutQuery>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->query_id_ = TlFetchLong::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  res->payload_ = TlFetchBytes<bytes>::parse(p);
  if (var0 & 1) { res->info_ = TlFetchBoxed<TlFetchObject<paymentRequestedInfo>, -1868808300>::parse(p); }
  if (var0 & 2) { res->shipping_option_id_ = TlFetchString<string>::parse(p); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateBotPrecheckoutQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotPrecheckoutQuery");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("query_id", query_id_);
    s.store_field("user_id", user_id_);
    s.store_bytes_field("payload", payload_);
    if (var0 & 1) { s.store_object_field("info", static_cast<const BaseObject *>(info_.get())); }
    if (var0 & 2) { s.store_field("shipping_option_id", shipping_option_id_); }
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_class_end();
  }
}

const std::int32_t updatePhoneCall::ID;

object_ptr<Update> updatePhoneCall::fetch(TlBufferParser &p) {
  return make_tl_object<updatePhoneCall>(p);
}

updatePhoneCall::updatePhoneCall(TlBufferParser &p)
  : phone_call_(TlFetchObject<PhoneCall>::parse(p))
{}

void updatePhoneCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePhoneCall");
    s.store_object_field("phone_call", static_cast<const BaseObject *>(phone_call_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateLangPackTooLong::ID;

object_ptr<Update> updateLangPackTooLong::fetch(TlBufferParser &p) {
  return make_tl_object<updateLangPackTooLong>(p);
}

updateLangPackTooLong::updateLangPackTooLong(TlBufferParser &p)
  : lang_code_(TlFetchString<string>::parse(p))
{}

void updateLangPackTooLong::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateLangPackTooLong");
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

const std::int32_t updateLangPack::ID;

object_ptr<Update> updateLangPack::fetch(TlBufferParser &p) {
  return make_tl_object<updateLangPack>(p);
}

updateLangPack::updateLangPack(TlBufferParser &p)
  : difference_(TlFetchBoxed<TlFetchObject<langPackDifference>, -209337866>::parse(p))
{}

void updateLangPack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateLangPack");
    s.store_object_field("difference", static_cast<const BaseObject *>(difference_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateFavedStickers::ID;

object_ptr<Update> updateFavedStickers::fetch(TlBufferParser &p) {
  return make_tl_object<updateFavedStickers>();
}

void updateFavedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFavedStickers");
    s.store_class_end();
  }
}

updateChannelReadMessagesContents::updateChannelReadMessagesContents()
  : flags_()
  , channel_id_()
  , top_msg_id_()
  , saved_peer_id_()
  , messages_()
{}

const std::int32_t updateChannelReadMessagesContents::ID;

object_ptr<Update> updateChannelReadMessagesContents::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateChannelReadMessagesContents> res = make_tl_object<updateChannelReadMessagesContents>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->channel_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->top_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->saved_peer_id_ = TlFetchObject<Peer>::parse(p); }
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateChannelReadMessagesContents::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelReadMessagesContents");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("channel_id", channel_id_);
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 2) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateContactsReset::ID;

object_ptr<Update> updateContactsReset::fetch(TlBufferParser &p) {
  return make_tl_object<updateContactsReset>();
}

void updateContactsReset::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateContactsReset");
    s.store_class_end();
  }
}

const std::int32_t updateChannelAvailableMessages::ID;

object_ptr<Update> updateChannelAvailableMessages::fetch(TlBufferParser &p) {
  return make_tl_object<updateChannelAvailableMessages>(p);
}

updateChannelAvailableMessages::updateChannelAvailableMessages(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , available_min_id_(TlFetchInt::parse(p))
{}

void updateChannelAvailableMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelAvailableMessages");
    s.store_field("channel_id", channel_id_);
    s.store_field("available_min_id", available_min_id_);
    s.store_class_end();
  }
}

updateDialogUnreadMark::updateDialogUnreadMark()
  : flags_()
  , unread_()
  , peer_()
  , saved_peer_id_()
{}

const std::int32_t updateDialogUnreadMark::ID;

object_ptr<Update> updateDialogUnreadMark::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateDialogUnreadMark> res = make_tl_object<updateDialogUnreadMark>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->unread_ = (var0 & 1) != 0;
  res->peer_ = TlFetchObject<DialogPeer>::parse(p);
  if (var0 & 2) { res->saved_peer_id_ = TlFetchObject<Peer>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateDialogUnreadMark::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDialogUnreadMark");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (unread_ << 0)));
    if (var0 & 1) { s.store_field("unread", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 2) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_class_end();
  }
}

updateMessagePoll::updateMessagePoll()
  : flags_()
  , poll_id_()
  , poll_()
  , results_()
{}

const std::int32_t updateMessagePoll::ID;

object_ptr<Update> updateMessagePoll::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateMessagePoll> res = make_tl_object<updateMessagePoll>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->poll_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->poll_ = TlFetchBoxed<TlFetchObject<poll>, 1484026161>::parse(p); }
  res->results_ = TlFetchBoxed<TlFetchObject<pollResults>, 2061444128>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateMessagePoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessagePoll");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("poll_id", poll_id_);
    if (var0 & 1) { s.store_object_field("poll", static_cast<const BaseObject *>(poll_.get())); }
    s.store_object_field("results", static_cast<const BaseObject *>(results_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateChatDefaultBannedRights::ID;

object_ptr<Update> updateChatDefaultBannedRights::fetch(TlBufferParser &p) {
  return make_tl_object<updateChatDefaultBannedRights>(p);
}

updateChatDefaultBannedRights::updateChatDefaultBannedRights(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , default_banned_rights_(TlFetchBoxed<TlFetchObject<chatBannedRights>, -1626209256>::parse(p))
  , version_(TlFetchInt::parse(p))
{}

void updateChatDefaultBannedRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatDefaultBannedRights");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("default_banned_rights", static_cast<const BaseObject *>(default_banned_rights_.get()));
    s.store_field("version", version_);
    s.store_class_end();
  }
}

const std::int32_t updateFolderPeers::ID;

object_ptr<Update> updateFolderPeers::fetch(TlBufferParser &p) {
  return make_tl_object<updateFolderPeers>(p);
}

updateFolderPeers::updateFolderPeers(TlBufferParser &p)
  : folder_peers_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<folderPeer>, -373643672>>, 481674261>::parse(p))
  , pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void updateFolderPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFolderPeers");
    { s.store_vector_begin("folder_peers", folder_peers_.size()); for (const auto &_value : folder_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

const std::int32_t updatePeerSettings::ID;

object_ptr<Update> updatePeerSettings::fetch(TlBufferParser &p) {
  return make_tl_object<updatePeerSettings>(p);
}

updatePeerSettings::updatePeerSettings(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , settings_(TlFetchBoxed<TlFetchObject<peerSettings>, -193510921>::parse(p))
{}

void updatePeerSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePeerSettings");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

const std::int32_t updatePeerLocated::ID;

object_ptr<Update> updatePeerLocated::fetch(TlBufferParser &p) {
  return make_tl_object<updatePeerLocated>(p);
}

updatePeerLocated::updatePeerLocated(TlBufferParser &p)
  : peers_(TlFetchBoxed<TlFetchVector<TlFetchObject<PeerLocated>>, 481674261>::parse(p))
{}

void updatePeerLocated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePeerLocated");
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateNewScheduledMessage::ID;

object_ptr<Update> updateNewScheduledMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateNewScheduledMessage>(p);
}

updateNewScheduledMessage::updateNewScheduledMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void updateNewScheduledMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewScheduledMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

updateDeleteScheduledMessages::updateDeleteScheduledMessages()
  : flags_()
  , peer_()
  , messages_()
  , sent_messages_()
{}

const std::int32_t updateDeleteScheduledMessages::ID;

object_ptr<Update> updateDeleteScheduledMessages::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateDeleteScheduledMessages> res = make_tl_object<updateDeleteScheduledMessages>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
  if (var0 & 1) { res->sent_messages_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateDeleteScheduledMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDeleteScheduledMessages");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    if (var0 & 1) { { s.store_vector_begin("sent_messages", sent_messages_.size()); for (const auto &_value : sent_messages_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

const std::int32_t updateTheme::ID;

object_ptr<Update> updateTheme::fetch(TlBufferParser &p) {
  return make_tl_object<updateTheme>(p);
}

updateTheme::updateTheme(TlBufferParser &p)
  : theme_(TlFetchBoxed<TlFetchObject<theme>, -1609668650>::parse(p))
{}

void updateTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateTheme");
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateGeoLiveViewed::ID;

object_ptr<Update> updateGeoLiveViewed::fetch(TlBufferParser &p) {
  return make_tl_object<updateGeoLiveViewed>(p);
}

updateGeoLiveViewed::updateGeoLiveViewed(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
{}

void updateGeoLiveViewed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGeoLiveViewed");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

const std::int32_t updateLoginToken::ID;

object_ptr<Update> updateLoginToken::fetch(TlBufferParser &p) {
  return make_tl_object<updateLoginToken>();
}

void updateLoginToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateLoginToken");
    s.store_class_end();
  }
}

const std::int32_t updateMessagePollVote::ID;

object_ptr<Update> updateMessagePollVote::fetch(TlBufferParser &p) {
  return make_tl_object<updateMessagePollVote>(p);
}

updateMessagePollVote::updateMessagePollVote(TlBufferParser &p)
  : poll_id_(TlFetchLong::parse(p))
  , peer_(TlFetchObject<Peer>::parse(p))
  , options_(TlFetchBoxed<TlFetchVector<TlFetchBytes<bytes>>, 481674261>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateMessagePollVote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessagePollVote");
    s.store_field("poll_id", poll_id_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

updateDialogFilter::updateDialogFilter()
  : flags_()
  , id_()
  , filter_()
{}

const std::int32_t updateDialogFilter::ID;

object_ptr<Update> updateDialogFilter::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateDialogFilter> res = make_tl_object<updateDialogFilter>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->filter_ = TlFetchObject<DialogFilter>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateDialogFilter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDialogFilter");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get())); }
    s.store_class_end();
  }
}

const std::int32_t updateDialogFilterOrder::ID;

object_ptr<Update> updateDialogFilterOrder::fetch(TlBufferParser &p) {
  return make_tl_object<updateDialogFilterOrder>(p);
}

updateDialogFilterOrder::updateDialogFilterOrder(TlBufferParser &p)
  : order_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void updateDialogFilterOrder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDialogFilterOrder");
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateDialogFilters::ID;

object_ptr<Update> updateDialogFilters::fetch(TlBufferParser &p) {
  return make_tl_object<updateDialogFilters>();
}

void updateDialogFilters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDialogFilters");
    s.store_class_end();
  }
}

const std::int32_t updatePhoneCallSignalingData::ID;

object_ptr<Update> updatePhoneCallSignalingData::fetch(TlBufferParser &p) {
  return make_tl_object<updatePhoneCallSignalingData>(p);
}

updatePhoneCallSignalingData::updatePhoneCallSignalingData(TlBufferParser &p)
  : phone_call_id_(TlFetchLong::parse(p))
  , data_(TlFetchBytes<bytes>::parse(p))
{}

void updatePhoneCallSignalingData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePhoneCallSignalingData");
    s.store_field("phone_call_id", phone_call_id_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

const std::int32_t updateChannelMessageForwards::ID;

object_ptr<Update> updateChannelMessageForwards::fetch(TlBufferParser &p) {
  return make_tl_object<updateChannelMessageForwards>(p);
}

updateChannelMessageForwards::updateChannelMessageForwards(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , id_(TlFetchInt::parse(p))
  , forwards_(TlFetchInt::parse(p))
{}

void updateChannelMessageForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelMessageForwards");
    s.store_field("channel_id", channel_id_);
    s.store_field("id", id_);
    s.store_field("forwards", forwards_);
    s.store_class_end();
  }
}

updateReadChannelDiscussionInbox::updateReadChannelDiscussionInbox()
  : flags_()
  , channel_id_()
  , top_msg_id_()
  , read_max_id_()
  , broadcast_id_()
  , broadcast_post_()
{}

const std::int32_t updateReadChannelDiscussionInbox::ID;

object_ptr<Update> updateReadChannelDiscussionInbox::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateReadChannelDiscussionInbox> res = make_tl_object<updateReadChannelDiscussionInbox>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->channel_id_ = TlFetchLong::parse(p);
  res->top_msg_id_ = TlFetchInt::parse(p);
  res->read_max_id_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->broadcast_id_ = TlFetchLong::parse(p); }
  if (var0 & 1) { res->broadcast_post_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateReadChannelDiscussionInbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadChannelDiscussionInbox");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("channel_id", channel_id_);
    s.store_field("top_msg_id", top_msg_id_);
    s.store_field("read_max_id", read_max_id_);
    if (var0 & 1) { s.store_field("broadcast_id", broadcast_id_); }
    if (var0 & 1) { s.store_field("broadcast_post", broadcast_post_); }
    s.store_class_end();
  }
}

const std::int32_t updateReadChannelDiscussionOutbox::ID;

object_ptr<Update> updateReadChannelDiscussionOutbox::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadChannelDiscussionOutbox>(p);
}

updateReadChannelDiscussionOutbox::updateReadChannelDiscussionOutbox(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , top_msg_id_(TlFetchInt::parse(p))
  , read_max_id_(TlFetchInt::parse(p))
{}

void updateReadChannelDiscussionOutbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadChannelDiscussionOutbox");
    s.store_field("channel_id", channel_id_);
    s.store_field("top_msg_id", top_msg_id_);
    s.store_field("read_max_id", read_max_id_);
    s.store_class_end();
  }
}

updatePeerBlocked::updatePeerBlocked()
  : flags_()
  , blocked_()
  , blocked_my_stories_from_()
  , peer_id_()
{}

const std::int32_t updatePeerBlocked::ID;

object_ptr<Update> updatePeerBlocked::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePeerBlocked> res = make_tl_object<updatePeerBlocked>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->blocked_ = (var0 & 1) != 0;
  res->blocked_my_stories_from_ = (var0 & 2) != 0;
  res->peer_id_ = TlFetchObject<Peer>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePeerBlocked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePeerBlocked");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (blocked_ << 0) | (blocked_my_stories_from_ << 1)));
    if (var0 & 1) { s.store_field("blocked", true); }
    if (var0 & 2) { s.store_field("blocked_my_stories_from", true); }
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    s.store_class_end();
  }
}

updateChannelUserTyping::updateChannelUserTyping()
  : flags_()
  , channel_id_()
  , top_msg_id_()
  , from_id_()
  , action_()
{}

const std::int32_t updateChannelUserTyping::ID;

object_ptr<Update> updateChannelUserTyping::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateChannelUserTyping> res = make_tl_object<updateChannelUserTyping>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->channel_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->top_msg_id_ = TlFetchInt::parse(p); }
  res->from_id_ = TlFetchObject<Peer>::parse(p);
  res->action_ = TlFetchObject<SendMessageAction>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateChannelUserTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelUserTyping");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("channel_id", channel_id_);
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get()));
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

updatePinnedMessages::updatePinnedMessages()
  : flags_()
  , pinned_()
  , peer_()
  , messages_()
  , pts_()
  , pts_count_()
{}

const std::int32_t updatePinnedMessages::ID;

object_ptr<Update> updatePinnedMessages::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePinnedMessages> res = make_tl_object<updatePinnedMessages>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 1) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  res->pts_count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePinnedMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePinnedMessages");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 0)));
    if (var0 & 1) { s.store_field("pinned", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

updatePinnedChannelMessages::updatePinnedChannelMessages()
  : flags_()
  , pinned_()
  , channel_id_()
  , messages_()
  , pts_()
  , pts_count_()
{}

const std::int32_t updatePinnedChannelMessages::ID;

object_ptr<Update> updatePinnedChannelMessages::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePinnedChannelMessages> res = make_tl_object<updatePinnedChannelMessages>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 1) != 0;
  res->channel_id_ = TlFetchLong::parse(p);
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  res->pts_count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePinnedChannelMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePinnedChannelMessages");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 0)));
    if (var0 & 1) { s.store_field("pinned", true); }
    s.store_field("channel_id", channel_id_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

const std::int32_t updateChat::ID;

object_ptr<Update> updateChat::fetch(TlBufferParser &p) {
  return make_tl_object<updateChat>(p);
}

updateChat::updateChat(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
{}

void updateChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

const std::int32_t updateGroupCallParticipants::ID;

object_ptr<Update> updateGroupCallParticipants::fetch(TlBufferParser &p) {
  return make_tl_object<updateGroupCallParticipants>(p);
}

updateGroupCallParticipants::updateGroupCallParticipants(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
  , participants_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<groupCallParticipant>, -341428482>>, 481674261>::parse(p))
  , version_(TlFetchInt::parse(p))
{}

void updateGroupCallParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallParticipants");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    { s.store_vector_begin("participants", participants_.size()); for (const auto &_value : participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("version", version_);
    s.store_class_end();
  }
}

updateGroupCall::updateGroupCall()
  : flags_()
  , chat_id_()
  , call_()
{}

const std::int32_t updateGroupCall::ID;

object_ptr<Update> updateGroupCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateGroupCall> res = make_tl_object<updateGroupCall>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->chat_id_ = TlFetchLong::parse(p); }
  res->call_ = TlFetchObject<GroupCall>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCall");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("chat_id", chat_id_); }
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

updatePeerHistoryTTL::updatePeerHistoryTTL()
  : flags_()
  , peer_()
  , ttl_period_()
{}

const std::int32_t updatePeerHistoryTTL::ID;

object_ptr<Update> updatePeerHistoryTTL::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePeerHistoryTTL> res = make_tl_object<updatePeerHistoryTTL>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 1) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePeerHistoryTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePeerHistoryTTL");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

updateChatParticipant::updateChatParticipant()
  : flags_()
  , chat_id_()
  , date_()
  , actor_id_()
  , user_id_()
  , prev_participant_()
  , new_participant_()
  , invite_()
  , qts_()
{}

const std::int32_t updateChatParticipant::ID;

object_ptr<Update> updateChatParticipant::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateChatParticipant> res = make_tl_object<updateChatParticipant>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->chat_id_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->actor_id_ = TlFetchLong::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->prev_participant_ = TlFetchObject<ChatParticipant>::parse(p); }
  if (var0 & 2) { res->new_participant_ = TlFetchObject<ChatParticipant>::parse(p); }
  if (var0 & 4) { res->invite_ = TlFetchObject<ExportedChatInvite>::parse(p); }
  res->qts_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateChatParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatParticipant");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("chat_id", chat_id_);
    s.store_field("date", date_);
    s.store_field("actor_id", actor_id_);
    s.store_field("user_id", user_id_);
    if (var0 & 1) { s.store_object_field("prev_participant", static_cast<const BaseObject *>(prev_participant_.get())); }
    if (var0 & 2) { s.store_object_field("new_participant", static_cast<const BaseObject *>(new_participant_.get())); }
    if (var0 & 4) { s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get())); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

updateChannelParticipant::updateChannelParticipant()
  : flags_()
  , via_chatlist_()
  , channel_id_()
  , date_()
  , actor_id_()
  , user_id_()
  , prev_participant_()
  , new_participant_()
  , invite_()
  , qts_()
{}

const std::int32_t updateChannelParticipant::ID;

object_ptr<Update> updateChannelParticipant::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateChannelParticipant> res = make_tl_object<updateChannelParticipant>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->via_chatlist_ = (var0 & 8) != 0;
  res->channel_id_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->actor_id_ = TlFetchLong::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->prev_participant_ = TlFetchObject<ChannelParticipant>::parse(p); }
  if (var0 & 2) { res->new_participant_ = TlFetchObject<ChannelParticipant>::parse(p); }
  if (var0 & 4) { res->invite_ = TlFetchObject<ExportedChatInvite>::parse(p); }
  res->qts_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateChannelParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelParticipant");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (via_chatlist_ << 3)));
    if (var0 & 8) { s.store_field("via_chatlist", true); }
    s.store_field("channel_id", channel_id_);
    s.store_field("date", date_);
    s.store_field("actor_id", actor_id_);
    s.store_field("user_id", user_id_);
    if (var0 & 1) { s.store_object_field("prev_participant", static_cast<const BaseObject *>(prev_participant_.get())); }
    if (var0 & 2) { s.store_object_field("new_participant", static_cast<const BaseObject *>(new_participant_.get())); }
    if (var0 & 4) { s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get())); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

const std::int32_t updateBotStopped::ID;

object_ptr<Update> updateBotStopped::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotStopped>(p);
}

updateBotStopped::updateBotStopped(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
  , stopped_(TlFetchBool::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotStopped::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotStopped");
    s.store_field("user_id", user_id_);
    s.store_field("date", date_);
    s.store_field("stopped", stopped_);
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

updateGroupCallConnection::updateGroupCallConnection()
  : flags_()
  , presentation_()
  , params_()
{}

const std::int32_t updateGroupCallConnection::ID;

object_ptr<Update> updateGroupCallConnection::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateGroupCallConnection> res = make_tl_object<updateGroupCallConnection>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->presentation_ = (var0 & 1) != 0;
  res->params_ = TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateGroupCallConnection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallConnection");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (presentation_ << 0)));
    if (var0 & 1) { s.store_field("presentation", true); }
    s.store_object_field("params", static_cast<const BaseObject *>(params_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateBotCommands::ID;

object_ptr<Update> updateBotCommands::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotCommands>(p);
}

updateBotCommands::updateBotCommands(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , bot_id_(TlFetchLong::parse(p))
  , commands_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<botCommand>, -1032140601>>, 481674261>::parse(p))
{}

void updateBotCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotCommands");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("bot_id", bot_id_);
    { s.store_vector_begin("commands", commands_.size()); for (const auto &_value : commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updatePendingJoinRequests::ID;

object_ptr<Update> updatePendingJoinRequests::fetch(TlBufferParser &p) {
  return make_tl_object<updatePendingJoinRequests>(p);
}

updatePendingJoinRequests::updatePendingJoinRequests(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , requests_pending_(TlFetchInt::parse(p))
  , recent_requesters_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void updatePendingJoinRequests::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePendingJoinRequests");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("requests_pending", requests_pending_);
    { s.store_vector_begin("recent_requesters", recent_requesters_.size()); for (const auto &_value : recent_requesters_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateBotChatInviteRequester::ID;

object_ptr<Update> updateBotChatInviteRequester::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotChatInviteRequester>(p);
}

updateBotChatInviteRequester::updateBotChatInviteRequester(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , date_(TlFetchInt::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , about_(TlFetchString<string>::parse(p))
  , invite_(TlFetchObject<ExportedChatInvite>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotChatInviteRequester::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotChatInviteRequester");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("date", date_);
    s.store_field("user_id", user_id_);
    s.store_field("about", about_);
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

updateMessageReactions::updateMessageReactions()
  : flags_()
  , peer_()
  , msg_id_()
  , top_msg_id_()
  , saved_peer_id_()
  , reactions_()
{}

const std::int32_t updateMessageReactions::ID;

object_ptr<Update> updateMessageReactions::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateMessageReactions> res = make_tl_object<updateMessageReactions>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->msg_id_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->top_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->saved_peer_id_ = TlFetchObject<Peer>::parse(p); }
  res->reactions_ = TlFetchBoxed<TlFetchObject<messageReactions>, 171155211>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateMessageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageReactions");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 2) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_object_field("reactions", static_cast<const BaseObject *>(reactions_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateAttachMenuBots::ID;

object_ptr<Update> updateAttachMenuBots::fetch(TlBufferParser &p) {
  return make_tl_object<updateAttachMenuBots>();
}

void updateAttachMenuBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAttachMenuBots");
    s.store_class_end();
  }
}

const std::int32_t updateWebViewResultSent::ID;

object_ptr<Update> updateWebViewResultSent::fetch(TlBufferParser &p) {
  return make_tl_object<updateWebViewResultSent>(p);
}

updateWebViewResultSent::updateWebViewResultSent(TlBufferParser &p)
  : query_id_(TlFetchLong::parse(p))
{}

void updateWebViewResultSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateWebViewResultSent");
    s.store_field("query_id", query_id_);
    s.store_class_end();
  }
}

const std::int32_t updateBotMenuButton::ID;

object_ptr<Update> updateBotMenuButton::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotMenuButton>(p);
}

updateBotMenuButton::updateBotMenuButton(TlBufferParser &p)
  : bot_id_(TlFetchLong::parse(p))
  , button_(TlFetchObject<BotMenuButton>::parse(p))
{}

void updateBotMenuButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotMenuButton");
    s.store_field("bot_id", bot_id_);
    s.store_object_field("button", static_cast<const BaseObject *>(button_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateSavedRingtones::ID;

object_ptr<Update> updateSavedRingtones::fetch(TlBufferParser &p) {
  return make_tl_object<updateSavedRingtones>();
}

void updateSavedRingtones::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedRingtones");
    s.store_class_end();
  }
}

updateTranscribedAudio::updateTranscribedAudio()
  : flags_()
  , pending_()
  , peer_()
  , msg_id_()
  , transcription_id_()
  , text_()
{}

const std::int32_t updateTranscribedAudio::ID;

object_ptr<Update> updateTranscribedAudio::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateTranscribedAudio> res = make_tl_object<updateTranscribedAudio>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pending_ = (var0 & 1) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->msg_id_ = TlFetchInt::parse(p);
  res->transcription_id_ = TlFetchLong::parse(p);
  res->text_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateTranscribedAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateTranscribedAudio");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pending_ << 0)));
    if (var0 & 1) { s.store_field("pending", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("transcription_id", transcription_id_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

const std::int32_t updateReadFeaturedEmojiStickers::ID;

object_ptr<Update> updateReadFeaturedEmojiStickers::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadFeaturedEmojiStickers>();
}

void updateReadFeaturedEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadFeaturedEmojiStickers");
    s.store_class_end();
  }
}

const std::int32_t updateUserEmojiStatus::ID;

object_ptr<Update> updateUserEmojiStatus::fetch(TlBufferParser &p) {
  return make_tl_object<updateUserEmojiStatus>(p);
}

updateUserEmojiStatus::updateUserEmojiStatus(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , emoji_status_(TlFetchObject<EmojiStatus>::parse(p))
{}

void updateUserEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserEmojiStatus");
    s.store_field("user_id", user_id_);
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateRecentEmojiStatuses::ID;

object_ptr<Update> updateRecentEmojiStatuses::fetch(TlBufferParser &p) {
  return make_tl_object<updateRecentEmojiStatuses>();
}

void updateRecentEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateRecentEmojiStatuses");
    s.store_class_end();
  }
}

const std::int32_t updateRecentReactions::ID;

object_ptr<Update> updateRecentReactions::fetch(TlBufferParser &p) {
  return make_tl_object<updateRecentReactions>();
}

void updateRecentReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateRecentReactions");
    s.store_class_end();
  }
}

updateMoveStickerSetToTop::updateMoveStickerSetToTop()
  : flags_()
  , masks_()
  , emojis_()
  , stickerset_()
{}

const std::int32_t updateMoveStickerSetToTop::ID;

object_ptr<Update> updateMoveStickerSetToTop::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateMoveStickerSetToTop> res = make_tl_object<updateMoveStickerSetToTop>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->masks_ = (var0 & 1) != 0;
  res->emojis_ = (var0 & 2) != 0;
  res->stickerset_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateMoveStickerSetToTop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMoveStickerSetToTop");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (masks_ << 0) | (emojis_ << 1)));
    if (var0 & 1) { s.store_field("masks", true); }
    if (var0 & 2) { s.store_field("emojis", true); }
    s.store_field("stickerset", stickerset_);
    s.store_class_end();
  }
}

const std::int32_t updateMessageExtendedMedia::ID;

object_ptr<Update> updateMessageExtendedMedia::fetch(TlBufferParser &p) {
  return make_tl_object<updateMessageExtendedMedia>(p);
}

updateMessageExtendedMedia::updateMessageExtendedMedia(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , extended_media_(TlFetchBoxed<TlFetchVector<TlFetchObject<MessageExtendedMedia>>, 481674261>::parse(p))
{}

void updateMessageExtendedMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageExtendedMedia");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    { s.store_vector_begin("extended_media", extended_media_.size()); for (const auto &_value : extended_media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateUser::ID;

object_ptr<Update> updateUser::fetch(TlBufferParser &p) {
  return make_tl_object<updateUser>(p);
}

updateUser::updateUser(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
{}

void updateUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUser");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

const std::int32_t updateAutoSaveSettings::ID;

object_ptr<Update> updateAutoSaveSettings::fetch(TlBufferParser &p) {
  return make_tl_object<updateAutoSaveSettings>();
}

void updateAutoSaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAutoSaveSettings");
    s.store_class_end();
  }
}

const std::int32_t updateStory::ID;

object_ptr<Update> updateStory::fetch(TlBufferParser &p) {
  return make_tl_object<updateStory>(p);
}

updateStory::updateStory(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , story_(TlFetchObject<StoryItem>::parse(p))
{}

void updateStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateReadStories::ID;

object_ptr<Update> updateReadStories::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadStories>(p);
}

updateReadStories::updateReadStories(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , max_id_(TlFetchInt::parse(p))
{}

void updateReadStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadStories");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

const std::int32_t updateStoryID::ID;

object_ptr<Update> updateStoryID::fetch(TlBufferParser &p) {
  return make_tl_object<updateStoryID>(p);
}

updateStoryID::updateStoryID(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
  , random_id_(TlFetchLong::parse(p))
{}

void updateStoryID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStoryID");
    s.store_field("id", id_);
    s.store_field("random_id", random_id_);
    s.store_class_end();
  }
}

const std::int32_t updateStoriesStealthMode::ID;

object_ptr<Update> updateStoriesStealthMode::fetch(TlBufferParser &p) {
  return make_tl_object<updateStoriesStealthMode>(p);
}

updateStoriesStealthMode::updateStoriesStealthMode(TlBufferParser &p)
  : stealth_mode_(TlFetchBoxed<TlFetchObject<storiesStealthMode>, 1898850301>::parse(p))
{}

void updateStoriesStealthMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStoriesStealthMode");
    s.store_object_field("stealth_mode", static_cast<const BaseObject *>(stealth_mode_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateSentStoryReaction::ID;

object_ptr<Update> updateSentStoryReaction::fetch(TlBufferParser &p) {
  return make_tl_object<updateSentStoryReaction>(p);
}

updateSentStoryReaction::updateSentStoryReaction(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , story_id_(TlFetchInt::parse(p))
  , reaction_(TlFetchObject<Reaction>::parse(p))
{}

void updateSentStoryReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSentStoryReaction");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("story_id", story_id_);
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateBotChatBoost::ID;

object_ptr<Update> updateBotChatBoost::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotChatBoost>(p);
}

updateBotChatBoost::updateBotChatBoost(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , boost_(TlFetchBoxed<TlFetchObject<boost>, 1262359766>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotChatBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotChatBoost");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("boost", static_cast<const BaseObject *>(boost_.get()));
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

const std::int32_t updateChannelViewForumAsMessages::ID;

object_ptr<Update> updateChannelViewForumAsMessages::fetch(TlBufferParser &p) {
  return make_tl_object<updateChannelViewForumAsMessages>(p);
}

updateChannelViewForumAsMessages::updateChannelViewForumAsMessages(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , enabled_(TlFetchBool::parse(p))
{}

void updateChannelViewForumAsMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChannelViewForumAsMessages");
    s.store_field("channel_id", channel_id_);
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

updatePeerWallpaper::updatePeerWallpaper()
  : flags_()
  , wallpaper_overridden_()
  , peer_()
  , wallpaper_()
{}

const std::int32_t updatePeerWallpaper::ID;

object_ptr<Update> updatePeerWallpaper::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePeerWallpaper> res = make_tl_object<updatePeerWallpaper>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->wallpaper_overridden_ = (var0 & 2) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 1) { res->wallpaper_ = TlFetchObject<WallPaper>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePeerWallpaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePeerWallpaper");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (wallpaper_overridden_ << 1)));
    if (var0 & 2) { s.store_field("wallpaper_overridden", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get())); }
    s.store_class_end();
  }
}

const std::int32_t updateBotMessageReaction::ID;

object_ptr<Update> updateBotMessageReaction::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotMessageReaction>(p);
}

updateBotMessageReaction::updateBotMessageReaction(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
  , actor_(TlFetchObject<Peer>::parse(p))
  , old_reactions_(TlFetchBoxed<TlFetchVector<TlFetchObject<Reaction>>, 481674261>::parse(p))
  , new_reactions_(TlFetchBoxed<TlFetchVector<TlFetchObject<Reaction>>, 481674261>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotMessageReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotMessageReaction");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("date", date_);
    s.store_object_field("actor", static_cast<const BaseObject *>(actor_.get()));
    { s.store_vector_begin("old_reactions", old_reactions_.size()); for (const auto &_value : old_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("new_reactions", new_reactions_.size()); for (const auto &_value : new_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

const std::int32_t updateBotMessageReactions::ID;

object_ptr<Update> updateBotMessageReactions::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotMessageReactions>(p);
}

updateBotMessageReactions::updateBotMessageReactions(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
  , reactions_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<reactionCount>, -1546531968>>, 481674261>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotMessageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotMessageReactions");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("date", date_);
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

updateSavedDialogPinned::updateSavedDialogPinned()
  : flags_()
  , pinned_()
  , peer_()
{}

const std::int32_t updateSavedDialogPinned::ID;

object_ptr<Update> updateSavedDialogPinned::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateSavedDialogPinned> res = make_tl_object<updateSavedDialogPinned>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 1) != 0;
  res->peer_ = TlFetchObject<DialogPeer>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateSavedDialogPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedDialogPinned");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 0)));
    if (var0 & 1) { s.store_field("pinned", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

updatePinnedSavedDialogs::updatePinnedSavedDialogs()
  : flags_()
  , order_()
{}

const std::int32_t updatePinnedSavedDialogs::ID;

object_ptr<Update> updatePinnedSavedDialogs::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePinnedSavedDialogs> res = make_tl_object<updatePinnedSavedDialogs>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->order_ = TlFetchBoxed<TlFetchVector<TlFetchObject<DialogPeer>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePinnedSavedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePinnedSavedDialogs");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

const std::int32_t updateSavedReactionTags::ID;

object_ptr<Update> updateSavedReactionTags::fetch(TlBufferParser &p) {
  return make_tl_object<updateSavedReactionTags>();
}

void updateSavedReactionTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedReactionTags");
    s.store_class_end();
  }
}

const std::int32_t updateSmsJob::ID;

object_ptr<Update> updateSmsJob::fetch(TlBufferParser &p) {
  return make_tl_object<updateSmsJob>(p);
}

updateSmsJob::updateSmsJob(TlBufferParser &p)
  : job_id_(TlFetchString<string>::parse(p))
{}

void updateSmsJob::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSmsJob");
    s.store_field("job_id", job_id_);
    s.store_class_end();
  }
}

const std::int32_t updateQuickReplies::ID;

object_ptr<Update> updateQuickReplies::fetch(TlBufferParser &p) {
  return make_tl_object<updateQuickReplies>(p);
}

updateQuickReplies::updateQuickReplies(TlBufferParser &p)
  : quick_replies_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<quickReply>, 110563371>>, 481674261>::parse(p))
{}

void updateQuickReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateQuickReplies");
    { s.store_vector_begin("quick_replies", quick_replies_.size()); for (const auto &_value : quick_replies_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateNewQuickReply::ID;

object_ptr<Update> updateNewQuickReply::fetch(TlBufferParser &p) {
  return make_tl_object<updateNewQuickReply>(p);
}

updateNewQuickReply::updateNewQuickReply(TlBufferParser &p)
  : quick_reply_(TlFetchBoxed<TlFetchObject<quickReply>, 110563371>::parse(p))
{}

void updateNewQuickReply::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewQuickReply");
    s.store_object_field("quick_reply", static_cast<const BaseObject *>(quick_reply_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateDeleteQuickReply::ID;

object_ptr<Update> updateDeleteQuickReply::fetch(TlBufferParser &p) {
  return make_tl_object<updateDeleteQuickReply>(p);
}

updateDeleteQuickReply::updateDeleteQuickReply(TlBufferParser &p)
  : shortcut_id_(TlFetchInt::parse(p))
{}

void updateDeleteQuickReply::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDeleteQuickReply");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_class_end();
  }
}

const std::int32_t updateQuickReplyMessage::ID;

object_ptr<Update> updateQuickReplyMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateQuickReplyMessage>(p);
}

updateQuickReplyMessage::updateQuickReplyMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void updateQuickReplyMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateQuickReplyMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateDeleteQuickReplyMessages::ID;

object_ptr<Update> updateDeleteQuickReplyMessages::fetch(TlBufferParser &p) {
  return make_tl_object<updateDeleteQuickReplyMessages>(p);
}

updateDeleteQuickReplyMessages::updateDeleteQuickReplyMessages(TlBufferParser &p)
  : shortcut_id_(TlFetchInt::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void updateDeleteQuickReplyMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDeleteQuickReplyMessages");
    s.store_field("shortcut_id", shortcut_id_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updateBotBusinessConnect::ID;

object_ptr<Update> updateBotBusinessConnect::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotBusinessConnect>(p);
}

updateBotBusinessConnect::updateBotBusinessConnect(TlBufferParser &p)
  : connection_(TlFetchBoxed<TlFetchObject<botBusinessConnection>, -1892371723>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotBusinessConnect::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotBusinessConnect");
    s.store_object_field("connection", static_cast<const BaseObject *>(connection_.get()));
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

updateBotNewBusinessMessage::updateBotNewBusinessMessage()
  : flags_()
  , connection_id_()
  , message_()
  , reply_to_message_()
  , qts_()
{}

const std::int32_t updateBotNewBusinessMessage::ID;

object_ptr<Update> updateBotNewBusinessMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateBotNewBusinessMessage> res = make_tl_object<updateBotNewBusinessMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->connection_id_ = TlFetchString<string>::parse(p);
  res->message_ = TlFetchObject<Message>::parse(p);
  if (var0 & 1) { res->reply_to_message_ = TlFetchObject<Message>::parse(p); }
  res->qts_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateBotNewBusinessMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotNewBusinessMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("connection_id", connection_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    if (var0 & 1) { s.store_object_field("reply_to_message", static_cast<const BaseObject *>(reply_to_message_.get())); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

updateBotEditBusinessMessage::updateBotEditBusinessMessage()
  : flags_()
  , connection_id_()
  , message_()
  , reply_to_message_()
  , qts_()
{}

const std::int32_t updateBotEditBusinessMessage::ID;

object_ptr<Update> updateBotEditBusinessMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateBotEditBusinessMessage> res = make_tl_object<updateBotEditBusinessMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->connection_id_ = TlFetchString<string>::parse(p);
  res->message_ = TlFetchObject<Message>::parse(p);
  if (var0 & 1) { res->reply_to_message_ = TlFetchObject<Message>::parse(p); }
  res->qts_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateBotEditBusinessMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotEditBusinessMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("connection_id", connection_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    if (var0 & 1) { s.store_object_field("reply_to_message", static_cast<const BaseObject *>(reply_to_message_.get())); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

const std::int32_t updateBotDeleteBusinessMessage::ID;

object_ptr<Update> updateBotDeleteBusinessMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotDeleteBusinessMessage>(p);
}

updateBotDeleteBusinessMessage::updateBotDeleteBusinessMessage(TlBufferParser &p)
  : connection_id_(TlFetchString<string>::parse(p))
  , peer_(TlFetchObject<Peer>::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotDeleteBusinessMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotDeleteBusinessMessage");
    s.store_field("connection_id", connection_id_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

const std::int32_t updateNewStoryReaction::ID;

object_ptr<Update> updateNewStoryReaction::fetch(TlBufferParser &p) {
  return make_tl_object<updateNewStoryReaction>(p);
}

updateNewStoryReaction::updateNewStoryReaction(TlBufferParser &p)
  : story_id_(TlFetchInt::parse(p))
  , peer_(TlFetchObject<Peer>::parse(p))
  , reaction_(TlFetchObject<Reaction>::parse(p))
{}

void updateNewStoryReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewStoryReaction");
    s.store_field("story_id", story_id_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateStarsBalance::ID;

object_ptr<Update> updateStarsBalance::fetch(TlBufferParser &p) {
  return make_tl_object<updateStarsBalance>(p);
}

updateStarsBalance::updateStarsBalance(TlBufferParser &p)
  : balance_(TlFetchObject<StarsAmount>::parse(p))
{}

void updateStarsBalance::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStarsBalance");
    s.store_object_field("balance", static_cast<const BaseObject *>(balance_.get()));
    s.store_class_end();
  }
}

updateBusinessBotCallbackQuery::updateBusinessBotCallbackQuery()
  : flags_()
  , query_id_()
  , user_id_()
  , connection_id_()
  , message_()
  , reply_to_message_()
  , chat_instance_()
  , data_()
{}

const std::int32_t updateBusinessBotCallbackQuery::ID;

object_ptr<Update> updateBusinessBotCallbackQuery::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateBusinessBotCallbackQuery> res = make_tl_object<updateBusinessBotCallbackQuery>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->query_id_ = TlFetchLong::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  res->connection_id_ = TlFetchString<string>::parse(p);
  res->message_ = TlFetchObject<Message>::parse(p);
  if (var0 & 4) { res->reply_to_message_ = TlFetchObject<Message>::parse(p); }
  res->chat_instance_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->data_ = TlFetchBytes<bytes>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateBusinessBotCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBusinessBotCallbackQuery");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("query_id", query_id_);
    s.store_field("user_id", user_id_);
    s.store_field("connection_id", connection_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    if (var0 & 4) { s.store_object_field("reply_to_message", static_cast<const BaseObject *>(reply_to_message_.get())); }
    s.store_field("chat_instance", chat_instance_);
    if (var0 & 1) { s.store_bytes_field("data", data_); }
    s.store_class_end();
  }
}

const std::int32_t updateStarsRevenueStatus::ID;

object_ptr<Update> updateStarsRevenueStatus::fetch(TlBufferParser &p) {
  return make_tl_object<updateStarsRevenueStatus>(p);
}

updateStarsRevenueStatus::updateStarsRevenueStatus(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , status_(TlFetchBoxed<TlFetchObject<starsRevenueStatus>, -21080943>::parse(p))
{}

void updateStarsRevenueStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStarsRevenueStatus");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateBotPurchasedPaidMedia::ID;

object_ptr<Update> updateBotPurchasedPaidMedia::fetch(TlBufferParser &p) {
  return make_tl_object<updateBotPurchasedPaidMedia>(p);
}

updateBotPurchasedPaidMedia::updateBotPurchasedPaidMedia(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , payload_(TlFetchString<string>::parse(p))
  , qts_(TlFetchInt::parse(p))
{}

void updateBotPurchasedPaidMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBotPurchasedPaidMedia");
    s.store_field("user_id", user_id_);
    s.store_field("payload", payload_);
    s.store_field("qts", qts_);
    s.store_class_end();
  }
}

const std::int32_t updatePaidReactionPrivacy::ID;

object_ptr<Update> updatePaidReactionPrivacy::fetch(TlBufferParser &p) {
  return make_tl_object<updatePaidReactionPrivacy>(p);
}

updatePaidReactionPrivacy::updatePaidReactionPrivacy(TlBufferParser &p)
  : private_(TlFetchObject<PaidReactionPrivacy>::parse(p))
{}

void updatePaidReactionPrivacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePaidReactionPrivacy");
    s.store_object_field("private", static_cast<const BaseObject *>(private_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateSentPhoneCode::ID;

object_ptr<Update> updateSentPhoneCode::fetch(TlBufferParser &p) {
  return make_tl_object<updateSentPhoneCode>(p);
}

updateSentPhoneCode::updateSentPhoneCode(TlBufferParser &p)
  : sent_code_(TlFetchObject<auth_SentCode>::parse(p))
{}

void updateSentPhoneCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSentPhoneCode");
    s.store_object_field("sent_code", static_cast<const BaseObject *>(sent_code_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateGroupCallChainBlocks::ID;

object_ptr<Update> updateGroupCallChainBlocks::fetch(TlBufferParser &p) {
  return make_tl_object<updateGroupCallChainBlocks>(p);
}

updateGroupCallChainBlocks::updateGroupCallChainBlocks(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
  , sub_chain_id_(TlFetchInt::parse(p))
  , blocks_(TlFetchBoxed<TlFetchVector<TlFetchBytes<bytes>>, 481674261>::parse(p))
  , next_offset_(TlFetchInt::parse(p))
{}

void updateGroupCallChainBlocks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallChainBlocks");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("sub_chain_id", sub_chain_id_);
    { s.store_vector_begin("blocks", blocks_.size()); for (const auto &_value : blocks_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

const std::int32_t updateReadMonoForumInbox::ID;

object_ptr<Update> updateReadMonoForumInbox::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadMonoForumInbox>(p);
}

updateReadMonoForumInbox::updateReadMonoForumInbox(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , saved_peer_id_(TlFetchObject<Peer>::parse(p))
  , read_max_id_(TlFetchInt::parse(p))
{}

void updateReadMonoForumInbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadMonoForumInbox");
    s.store_field("channel_id", channel_id_);
    s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get()));
    s.store_field("read_max_id", read_max_id_);
    s.store_class_end();
  }
}

const std::int32_t updateReadMonoForumOutbox::ID;

object_ptr<Update> updateReadMonoForumOutbox::fetch(TlBufferParser &p) {
  return make_tl_object<updateReadMonoForumOutbox>(p);
}

updateReadMonoForumOutbox::updateReadMonoForumOutbox(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , saved_peer_id_(TlFetchObject<Peer>::parse(p))
  , read_max_id_(TlFetchInt::parse(p))
{}

void updateReadMonoForumOutbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReadMonoForumOutbox");
    s.store_field("channel_id", channel_id_);
    s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get()));
    s.store_field("read_max_id", read_max_id_);
    s.store_class_end();
  }
}

updateMonoForumNoPaidException::updateMonoForumNoPaidException()
  : flags_()
  , exception_()
  , channel_id_()
  , saved_peer_id_()
{}

const std::int32_t updateMonoForumNoPaidException::ID;

object_ptr<Update> updateMonoForumNoPaidException::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateMonoForumNoPaidException> res = make_tl_object<updateMonoForumNoPaidException>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->exception_ = (var0 & 1) != 0;
  res->channel_id_ = TlFetchLong::parse(p);
  res->saved_peer_id_ = TlFetchObject<Peer>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateMonoForumNoPaidException::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMonoForumNoPaidException");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (exception_ << 0)));
    if (var0 & 1) { s.store_field("exception", true); }
    s.store_field("channel_id", channel_id_);
    s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateGroupCallMessage::ID;

object_ptr<Update> updateGroupCallMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateGroupCallMessage>(p);
}

updateGroupCallMessage::updateGroupCallMessage(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
  , from_id_(TlFetchObject<Peer>::parse(p))
  , random_id_(TlFetchLong::parse(p))
  , message_(TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p))
{}

void updateGroupCallMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallMessage");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get()));
    s.store_field("random_id", random_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t updateGroupCallEncryptedMessage::ID;

object_ptr<Update> updateGroupCallEncryptedMessage::fetch(TlBufferParser &p) {
  return make_tl_object<updateGroupCallEncryptedMessage>(p);
}

updateGroupCallEncryptedMessage::updateGroupCallEncryptedMessage(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
  , from_id_(TlFetchObject<Peer>::parse(p))
  , encrypted_message_(TlFetchBytes<bytes>::parse(p))
{}

void updateGroupCallEncryptedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallEncryptedMessage");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get()));
    s.store_bytes_field("encrypted_message", encrypted_message_);
    s.store_class_end();
  }
}

updatePinnedForumTopic::updatePinnedForumTopic()
  : flags_()
  , pinned_()
  , peer_()
  , topic_id_()
{}

const std::int32_t updatePinnedForumTopic::ID;

object_ptr<Update> updatePinnedForumTopic::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePinnedForumTopic> res = make_tl_object<updatePinnedForumTopic>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 1) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->topic_id_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePinnedForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePinnedForumTopic");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 0)));
    if (var0 & 1) { s.store_field("pinned", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("topic_id", topic_id_);
    s.store_class_end();
  }
}

updatePinnedForumTopics::updatePinnedForumTopics()
  : flags_()
  , peer_()
  , order_()
{}

const std::int32_t updatePinnedForumTopics::ID;

object_ptr<Update> updatePinnedForumTopics::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updatePinnedForumTopics> res = make_tl_object<updatePinnedForumTopics>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 1) { res->order_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updatePinnedForumTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePinnedForumTopics");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<User> User::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case userEmpty::ID:
      return userEmpty::fetch(p);
    case user::ID:
      return user::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t userEmpty::ID;

object_ptr<User> userEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<userEmpty>(p);
}

userEmpty::userEmpty(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void userEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userEmpty");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

user::user()
  : flags_()
  , self_()
  , contact_()
  , mutual_contact_()
  , deleted_()
  , bot_()
  , bot_chat_history_()
  , bot_nochats_()
  , verified_()
  , restricted_()
  , min_()
  , bot_inline_geo_()
  , support_()
  , scam_()
  , apply_min_photo_()
  , fake_()
  , bot_attach_menu_()
  , premium_()
  , attach_menu_enabled_()
  , flags2_()
  , bot_can_edit_()
  , close_friend_()
  , stories_hidden_()
  , stories_unavailable_()
  , contact_require_premium_()
  , bot_business_()
  , bot_has_main_app_()
  , bot_forum_view_()
  , id_()
  , access_hash_()
  , first_name_()
  , last_name_()
  , username_()
  , phone_()
  , photo_()
  , status_()
  , bot_info_version_()
  , restriction_reason_()
  , bot_inline_placeholder_()
  , lang_code_()
  , emoji_status_()
  , usernames_()
  , stories_max_id_()
  , color_()
  , profile_color_()
  , bot_active_users_()
  , bot_verification_icon_()
  , send_paid_messages_stars_()
{}

user::user(int32 flags_, bool self_, bool contact_, bool mutual_contact_, bool deleted_, bool bot_, bool bot_chat_history_, bool bot_nochats_, bool verified_, bool restricted_, bool min_, bool bot_inline_geo_, bool support_, bool scam_, bool apply_min_photo_, bool fake_, bool bot_attach_menu_, bool premium_, bool attach_menu_enabled_, int32 flags2_, bool bot_can_edit_, bool close_friend_, bool stories_hidden_, bool stories_unavailable_, bool contact_require_premium_, bool bot_business_, bool bot_has_main_app_, bool bot_forum_view_, int64 id_, int64 access_hash_, string const &first_name_, string const &last_name_, string const &username_, string const &phone_, object_ptr<UserProfilePhoto> &&photo_, object_ptr<UserStatus> &&status_, int32 bot_info_version_, array<object_ptr<restrictionReason>> &&restriction_reason_, string const &bot_inline_placeholder_, string const &lang_code_, object_ptr<EmojiStatus> &&emoji_status_, array<object_ptr<username>> &&usernames_, int32 stories_max_id_, object_ptr<PeerColor> &&color_, object_ptr<PeerColor> &&profile_color_, int32 bot_active_users_, int64 bot_verification_icon_, int64 send_paid_messages_stars_)
  : flags_(flags_)
  , self_(self_)
  , contact_(contact_)
  , mutual_contact_(mutual_contact_)
  , deleted_(deleted_)
  , bot_(bot_)
  , bot_chat_history_(bot_chat_history_)
  , bot_nochats_(bot_nochats_)
  , verified_(verified_)
  , restricted_(restricted_)
  , min_(min_)
  , bot_inline_geo_(bot_inline_geo_)
  , support_(support_)
  , scam_(scam_)
  , apply_min_photo_(apply_min_photo_)
  , fake_(fake_)
  , bot_attach_menu_(bot_attach_menu_)
  , premium_(premium_)
  , attach_menu_enabled_(attach_menu_enabled_)
  , flags2_(flags2_)
  , bot_can_edit_(bot_can_edit_)
  , close_friend_(close_friend_)
  , stories_hidden_(stories_hidden_)
  , stories_unavailable_(stories_unavailable_)
  , contact_require_premium_(contact_require_premium_)
  , bot_business_(bot_business_)
  , bot_has_main_app_(bot_has_main_app_)
  , bot_forum_view_(bot_forum_view_)
  , id_(id_)
  , access_hash_(access_hash_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , username_(username_)
  , phone_(phone_)
  , photo_(std::move(photo_))
  , status_(std::move(status_))
  , bot_info_version_(bot_info_version_)
  , restriction_reason_(std::move(restriction_reason_))
  , bot_inline_placeholder_(bot_inline_placeholder_)
  , lang_code_(lang_code_)
  , emoji_status_(std::move(emoji_status_))
  , usernames_(std::move(usernames_))
  , stories_max_id_(stories_max_id_)
  , color_(std::move(color_))
  , profile_color_(std::move(profile_color_))
  , bot_active_users_(bot_active_users_)
  , bot_verification_icon_(bot_verification_icon_)
  , send_paid_messages_stars_(send_paid_messages_stars_)
{}

const std::int32_t user::ID;

object_ptr<User> user::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<user> res = make_tl_object<user>();
  int32 var0;
  int32 var1;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->self_ = (var0 & 1024) != 0;
  res->contact_ = (var0 & 2048) != 0;
  res->mutual_contact_ = (var0 & 4096) != 0;
  res->deleted_ = (var0 & 8192) != 0;
  res->bot_ = (var0 & 16384) != 0;
  res->bot_chat_history_ = (var0 & 32768) != 0;
  res->bot_nochats_ = (var0 & 65536) != 0;
  res->verified_ = (var0 & 131072) != 0;
  res->restricted_ = (var0 & 262144) != 0;
  res->min_ = (var0 & 1048576) != 0;
  res->bot_inline_geo_ = (var0 & 2097152) != 0;
  res->support_ = (var0 & 8388608) != 0;
  res->scam_ = (var0 & 16777216) != 0;
  res->apply_min_photo_ = (var0 & 33554432) != 0;
  res->fake_ = (var0 & 67108864) != 0;
  res->bot_attach_menu_ = (var0 & 134217728) != 0;
  res->premium_ = (var0 & 268435456) != 0;
  res->attach_menu_enabled_ = (var0 & 536870912) != 0;
  if ((var1 = res->flags2_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->bot_can_edit_ = (var1 & 2) != 0;
  res->close_friend_ = (var1 & 4) != 0;
  res->stories_hidden_ = (var1 & 8) != 0;
  res->stories_unavailable_ = (var1 & 16) != 0;
  res->contact_require_premium_ = (var1 & 1024) != 0;
  res->bot_business_ = (var1 & 2048) != 0;
  res->bot_has_main_app_ = (var1 & 8192) != 0;
  res->bot_forum_view_ = (var1 & 65536) != 0;
  res->id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->access_hash_ = TlFetchLong::parse(p); }
  if (var0 & 2) { res->first_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->last_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->username_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->phone_ = TlFetchString<string>::parse(p); }
  if (var0 & 32) { res->photo_ = TlFetchObject<UserProfilePhoto>::parse(p); }
  if (var0 & 64) { res->status_ = TlFetchObject<UserStatus>::parse(p); }
  if (var0 & 16384) { res->bot_info_version_ = TlFetchInt::parse(p); }
  if (var0 & 262144) { res->restriction_reason_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<restrictionReason>, -797791052>>, 481674261>::parse(p); }
  if (var0 & 524288) { res->bot_inline_placeholder_ = TlFetchString<string>::parse(p); }
  if (var0 & 4194304) { res->lang_code_ = TlFetchString<string>::parse(p); }
  if (var0 & 1073741824) { res->emoji_status_ = TlFetchObject<EmojiStatus>::parse(p); }
  if (var1 & 1) { res->usernames_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<username>, -1274595769>>, 481674261>::parse(p); }
  if (var1 & 32) { res->stories_max_id_ = TlFetchInt::parse(p); }
  if (var1 & 256) { res->color_ = TlFetchObject<PeerColor>::parse(p); }
  if (var1 & 512) { res->profile_color_ = TlFetchObject<PeerColor>::parse(p); }
  if (var1 & 4096) { res->bot_active_users_ = TlFetchInt::parse(p); }
  if (var1 & 16384) { res->bot_verification_icon_ = TlFetchLong::parse(p); }
  if (var1 & 32768) { res->send_paid_messages_stars_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void user::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "user");
  int32 var0;
  int32 var1;
    s.store_field("flags", (var0 = flags_ | (self_ << 10) | (contact_ << 11) | (mutual_contact_ << 12) | (deleted_ << 13) | (bot_ << 14) | (bot_chat_history_ << 15) | (bot_nochats_ << 16) | (verified_ << 17) | (restricted_ << 18) | (min_ << 20) | (bot_inline_geo_ << 21) | (support_ << 23) | (scam_ << 24) | (apply_min_photo_ << 25) | (fake_ << 26) | (bot_attach_menu_ << 27) | (premium_ << 28) | (attach_menu_enabled_ << 29)));
    if (var0 & 1024) { s.store_field("self", true); }
    if (var0 & 2048) { s.store_field("contact", true); }
    if (var0 & 4096) { s.store_field("mutual_contact", true); }
    if (var0 & 8192) { s.store_field("deleted", true); }
    if (var0 & 16384) { s.store_field("bot", true); }
    if (var0 & 32768) { s.store_field("bot_chat_history", true); }
    if (var0 & 65536) { s.store_field("bot_nochats", true); }
    if (var0 & 131072) { s.store_field("verified", true); }
    if (var0 & 262144) { s.store_field("restricted", true); }
    if (var0 & 1048576) { s.store_field("min", true); }
    if (var0 & 2097152) { s.store_field("bot_inline_geo", true); }
    if (var0 & 8388608) { s.store_field("support", true); }
    if (var0 & 16777216) { s.store_field("scam", true); }
    if (var0 & 33554432) { s.store_field("apply_min_photo", true); }
    if (var0 & 67108864) { s.store_field("fake", true); }
    if (var0 & 134217728) { s.store_field("bot_attach_menu", true); }
    if (var0 & 268435456) { s.store_field("premium", true); }
    if (var0 & 536870912) { s.store_field("attach_menu_enabled", true); }
    s.store_field("flags2", (var1 = flags2_ | (bot_can_edit_ << 1) | (close_friend_ << 2) | (stories_hidden_ << 3) | (stories_unavailable_ << 4) | (contact_require_premium_ << 10) | (bot_business_ << 11) | (bot_has_main_app_ << 13) | (bot_forum_view_ << 16)));
    if (var1 & 2) { s.store_field("bot_can_edit", true); }
    if (var1 & 4) { s.store_field("close_friend", true); }
    if (var1 & 8) { s.store_field("stories_hidden", true); }
    if (var1 & 16) { s.store_field("stories_unavailable", true); }
    if (var1 & 1024) { s.store_field("contact_require_premium", true); }
    if (var1 & 2048) { s.store_field("bot_business", true); }
    if (var1 & 8192) { s.store_field("bot_has_main_app", true); }
    if (var1 & 65536) { s.store_field("bot_forum_view", true); }
    s.store_field("id", id_);
    if (var0 & 1) { s.store_field("access_hash", access_hash_); }
    if (var0 & 2) { s.store_field("first_name", first_name_); }
    if (var0 & 4) { s.store_field("last_name", last_name_); }
    if (var0 & 8) { s.store_field("username", username_); }
    if (var0 & 16) { s.store_field("phone", phone_); }
    if (var0 & 32) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 64) { s.store_object_field("status", static_cast<const BaseObject *>(status_.get())); }
    if (var0 & 16384) { s.store_field("bot_info_version", bot_info_version_); }
    if (var0 & 262144) { { s.store_vector_begin("restriction_reason", restriction_reason_.size()); for (const auto &_value : restriction_reason_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 524288) { s.store_field("bot_inline_placeholder", bot_inline_placeholder_); }
    if (var0 & 4194304) { s.store_field("lang_code", lang_code_); }
    if (var0 & 1073741824) { s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get())); }
    if (var1 & 1) { { s.store_vector_begin("usernames", usernames_.size()); for (const auto &_value : usernames_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var1 & 32) { s.store_field("stories_max_id", stories_max_id_); }
    if (var1 & 256) { s.store_object_field("color", static_cast<const BaseObject *>(color_.get())); }
    if (var1 & 512) { s.store_object_field("profile_color", static_cast<const BaseObject *>(profile_color_.get())); }
    if (var1 & 4096) { s.store_field("bot_active_users", bot_active_users_); }
    if (var1 & 16384) { s.store_field("bot_verification_icon", bot_verification_icon_); }
    if (var1 & 32768) { s.store_field("send_paid_messages_stars", send_paid_messages_stars_); }
    s.store_class_end();
  }
}

webViewResultUrl::webViewResultUrl()
  : flags_()
  , fullsize_()
  , fullscreen_()
  , query_id_()
  , url_()
{}

const std::int32_t webViewResultUrl::ID;

object_ptr<webViewResultUrl> webViewResultUrl::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webViewResultUrl> res = make_tl_object<webViewResultUrl>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->fullsize_ = (var0 & 2) != 0;
  res->fullscreen_ = (var0 & 4) != 0;
  if (var0 & 1) { res->query_id_ = TlFetchLong::parse(p); }
  res->url_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void webViewResultUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webViewResultUrl");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (fullsize_ << 1) | (fullscreen_ << 2)));
    if (var0 & 2) { s.store_field("fullsize", true); }
    if (var0 & 4) { s.store_field("fullscreen", true); }
    if (var0 & 1) { s.store_field("query_id", query_id_); }
    s.store_field("url", url_);
    s.store_class_end();
  }
}

object_ptr<account_ResetPasswordResult> account_ResetPasswordResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_resetPasswordFailedWait::ID:
      return account_resetPasswordFailedWait::fetch(p);
    case account_resetPasswordRequestedWait::ID:
      return account_resetPasswordRequestedWait::fetch(p);
    case account_resetPasswordOk::ID:
      return account_resetPasswordOk::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_resetPasswordFailedWait::ID;

object_ptr<account_ResetPasswordResult> account_resetPasswordFailedWait::fetch(TlBufferParser &p) {
  return make_tl_object<account_resetPasswordFailedWait>(p);
}

account_resetPasswordFailedWait::account_resetPasswordFailedWait(TlBufferParser &p)
  : retry_date_(TlFetchInt::parse(p))
{}

void account_resetPasswordFailedWait::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetPasswordFailedWait");
    s.store_field("retry_date", retry_date_);
    s.store_class_end();
  }
}

const std::int32_t account_resetPasswordRequestedWait::ID;

object_ptr<account_ResetPasswordResult> account_resetPasswordRequestedWait::fetch(TlBufferParser &p) {
  return make_tl_object<account_resetPasswordRequestedWait>(p);
}

account_resetPasswordRequestedWait::account_resetPasswordRequestedWait(TlBufferParser &p)
  : until_date_(TlFetchInt::parse(p))
{}

void account_resetPasswordRequestedWait::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetPasswordRequestedWait");
    s.store_field("until_date", until_date_);
    s.store_class_end();
  }
}

const std::int32_t account_resetPasswordOk::ID;

object_ptr<account_ResetPasswordResult> account_resetPasswordOk::fetch(TlBufferParser &p) {
  return make_tl_object<account_resetPasswordOk>();
}

void account_resetPasswordOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetPasswordOk");
    s.store_class_end();
  }
}

account_resolvedBusinessChatLinks::account_resolvedBusinessChatLinks()
  : flags_()
  , peer_()
  , message_()
  , entities_()
  , chats_()
  , users_()
{}

const std::int32_t account_resolvedBusinessChatLinks::ID;

object_ptr<account_resolvedBusinessChatLinks> account_resolvedBusinessChatLinks::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<account_resolvedBusinessChatLinks> res = make_tl_object<account_resolvedBusinessChatLinks>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void account_resolvedBusinessChatLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resolvedBusinessChatLinks");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("message", message_);
    if (var0 & 1) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<account_WallPapers> account_WallPapers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_wallPapersNotModified::ID:
      return account_wallPapersNotModified::fetch(p);
    case account_wallPapers::ID:
      return account_wallPapers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_wallPapersNotModified::ID;

object_ptr<account_WallPapers> account_wallPapersNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<account_wallPapersNotModified>();
}

void account_wallPapersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.wallPapersNotModified");
    s.store_class_end();
  }
}

const std::int32_t account_wallPapers::ID;

object_ptr<account_WallPapers> account_wallPapers::fetch(TlBufferParser &p) {
  return make_tl_object<account_wallPapers>(p);
}

account_wallPapers::account_wallPapers(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , wallpapers_(TlFetchBoxed<TlFetchVector<TlFetchObject<WallPaper>>, 481674261>::parse(p))
{}

void account_wallPapers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.wallPapers");
    s.store_field("hash", hash_);
    { s.store_vector_begin("wallpapers", wallpapers_.size()); for (const auto &_value : wallpapers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<auth_LoginToken> auth_LoginToken::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case auth_loginToken::ID:
      return auth_loginToken::fetch(p);
    case auth_loginTokenMigrateTo::ID:
      return auth_loginTokenMigrateTo::fetch(p);
    case auth_loginTokenSuccess::ID:
      return auth_loginTokenSuccess::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t auth_loginToken::ID;

object_ptr<auth_LoginToken> auth_loginToken::fetch(TlBufferParser &p) {
  return make_tl_object<auth_loginToken>(p);
}

auth_loginToken::auth_loginToken(TlBufferParser &p)
  : expires_(TlFetchInt::parse(p))
  , token_(TlFetchBytes<bytes>::parse(p))
{}

void auth_loginToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.loginToken");
    s.store_field("expires", expires_);
    s.store_bytes_field("token", token_);
    s.store_class_end();
  }
}

const std::int32_t auth_loginTokenMigrateTo::ID;

object_ptr<auth_LoginToken> auth_loginTokenMigrateTo::fetch(TlBufferParser &p) {
  return make_tl_object<auth_loginTokenMigrateTo>(p);
}

auth_loginTokenMigrateTo::auth_loginTokenMigrateTo(TlBufferParser &p)
  : dc_id_(TlFetchInt::parse(p))
  , token_(TlFetchBytes<bytes>::parse(p))
{}

void auth_loginTokenMigrateTo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.loginTokenMigrateTo");
    s.store_field("dc_id", dc_id_);
    s.store_bytes_field("token", token_);
    s.store_class_end();
  }
}

const std::int32_t auth_loginTokenSuccess::ID;

object_ptr<auth_LoginToken> auth_loginTokenSuccess::fetch(TlBufferParser &p) {
  return make_tl_object<auth_loginTokenSuccess>(p);
}

auth_loginTokenSuccess::auth_loginTokenSuccess(TlBufferParser &p)
  : authorization_(TlFetchObject<auth_Authorization>::parse(p))
{}

void auth_loginTokenSuccess::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.loginTokenSuccess");
    s.store_object_field("authorization", static_cast<const BaseObject *>(authorization_.get()));
    s.store_class_end();
  }
}

const std::int32_t auth_passwordRecovery::ID;

object_ptr<auth_passwordRecovery> auth_passwordRecovery::fetch(TlBufferParser &p) {
  return make_tl_object<auth_passwordRecovery>(p);
}

auth_passwordRecovery::auth_passwordRecovery(TlBufferParser &p)
  : email_pattern_(TlFetchString<string>::parse(p))
{}

void auth_passwordRecovery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.passwordRecovery");
    s.store_field("email_pattern", email_pattern_);
    s.store_class_end();
  }
}

const std::int32_t fragment_collectibleInfo::ID;

object_ptr<fragment_collectibleInfo> fragment_collectibleInfo::fetch(TlBufferParser &p) {
  return make_tl_object<fragment_collectibleInfo>(p);
}

fragment_collectibleInfo::fragment_collectibleInfo(TlBufferParser &p)
  : purchase_date_(TlFetchInt::parse(p))
  , currency_(TlFetchString<string>::parse(p))
  , amount_(TlFetchLong::parse(p))
  , crypto_currency_(TlFetchString<string>::parse(p))
  , crypto_amount_(TlFetchLong::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void fragment_collectibleInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fragment.collectibleInfo");
    s.store_field("purchase_date", purchase_date_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("crypto_currency", crypto_currency_);
    s.store_field("crypto_amount", crypto_amount_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

help_countryCode::help_countryCode()
  : flags_()
  , country_code_()
  , prefixes_()
  , patterns_()
{}

const std::int32_t help_countryCode::ID;

object_ptr<help_countryCode> help_countryCode::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<help_countryCode> res = make_tl_object<help_countryCode>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->country_code_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->prefixes_ = TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p); }
  if (var0 & 2) { res->patterns_ = TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void help_countryCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.countryCode");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("country_code", country_code_);
    if (var0 & 1) { { s.store_vector_begin("prefixes", prefixes_.size()); for (const auto &_value : prefixes_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 2) { { s.store_vector_begin("patterns", patterns_.size()); for (const auto &_value : patterns_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<help_PeerColorSet> help_PeerColorSet::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_peerColorSet::ID:
      return help_peerColorSet::fetch(p);
    case help_peerColorProfileSet::ID:
      return help_peerColorProfileSet::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_peerColorSet::ID;

object_ptr<help_PeerColorSet> help_peerColorSet::fetch(TlBufferParser &p) {
  return make_tl_object<help_peerColorSet>(p);
}

help_peerColorSet::help_peerColorSet(TlBufferParser &p)
  : colors_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void help_peerColorSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.peerColorSet");
    { s.store_vector_begin("colors", colors_.size()); for (const auto &_value : colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t help_peerColorProfileSet::ID;

object_ptr<help_PeerColorSet> help_peerColorProfileSet::fetch(TlBufferParser &p) {
  return make_tl_object<help_peerColorProfileSet>(p);
}

help_peerColorProfileSet::help_peerColorProfileSet(TlBufferParser &p)
  : palette_colors_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
  , bg_colors_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
  , story_colors_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void help_peerColorProfileSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.peerColorProfileSet");
    { s.store_vector_begin("palette_colors", palette_colors_.size()); for (const auto &_value : palette_colors_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("bg_colors", bg_colors_.size()); for (const auto &_value : bg_colors_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("story_colors", story_colors_.size()); for (const auto &_value : story_colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_botPreparedInlineMessage::ID;

object_ptr<messages_botPreparedInlineMessage> messages_botPreparedInlineMessage::fetch(TlBufferParser &p) {
  return make_tl_object<messages_botPreparedInlineMessage>(p);
}

messages_botPreparedInlineMessage::messages_botPreparedInlineMessage(TlBufferParser &p)
  : id_(TlFetchString<string>::parse(p))
  , expire_date_(TlFetchInt::parse(p))
{}

void messages_botPreparedInlineMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.botPreparedInlineMessage");
    s.store_field("id", id_);
    s.store_field("expire_date", expire_date_);
    s.store_class_end();
  }
}

object_ptr<messages_EmojiGroups> messages_EmojiGroups::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_emojiGroupsNotModified::ID:
      return messages_emojiGroupsNotModified::fetch(p);
    case messages_emojiGroups::ID:
      return messages_emojiGroups::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_emojiGroupsNotModified::ID;

object_ptr<messages_EmojiGroups> messages_emojiGroupsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_emojiGroupsNotModified>();
}

void messages_emojiGroupsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.emojiGroupsNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_emojiGroups::ID;

object_ptr<messages_EmojiGroups> messages_emojiGroups::fetch(TlBufferParser &p) {
  return make_tl_object<messages_emojiGroups>(p);
}

messages_emojiGroups::messages_emojiGroups(TlBufferParser &p)
  : hash_(TlFetchInt::parse(p))
  , groups_(TlFetchBoxed<TlFetchVector<TlFetchObject<EmojiGroup>>, 481674261>::parse(p))
{}

void messages_emojiGroups::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.emojiGroups");
    s.store_field("hash", hash_);
    { s.store_vector_begin("groups", groups_.size()); for (const auto &_value : groups_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_ExportedChatInvite> messages_ExportedChatInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_exportedChatInvite::ID:
      return messages_exportedChatInvite::fetch(p);
    case messages_exportedChatInviteReplaced::ID:
      return messages_exportedChatInviteReplaced::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_exportedChatInvite::ID;

object_ptr<messages_ExportedChatInvite> messages_exportedChatInvite::fetch(TlBufferParser &p) {
  return make_tl_object<messages_exportedChatInvite>(p);
}

messages_exportedChatInvite::messages_exportedChatInvite(TlBufferParser &p)
  : invite_(TlFetchObject<ExportedChatInvite>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_exportedChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.exportedChatInvite");
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_exportedChatInviteReplaced::ID;

object_ptr<messages_ExportedChatInvite> messages_exportedChatInviteReplaced::fetch(TlBufferParser &p) {
  return make_tl_object<messages_exportedChatInviteReplaced>(p);
}

messages_exportedChatInviteReplaced::messages_exportedChatInviteReplaced(TlBufferParser &p)
  : invite_(TlFetchObject<ExportedChatInvite>::parse(p))
  , new_invite_(TlFetchObject<ExportedChatInvite>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_exportedChatInviteReplaced::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.exportedChatInviteReplaced");
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    s.store_object_field("new_invite", static_cast<const BaseObject *>(new_invite_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_SavedDialogs> messages_SavedDialogs::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_savedDialogs::ID:
      return messages_savedDialogs::fetch(p);
    case messages_savedDialogsSlice::ID:
      return messages_savedDialogsSlice::fetch(p);
    case messages_savedDialogsNotModified::ID:
      return messages_savedDialogsNotModified::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_savedDialogs::ID;

object_ptr<messages_SavedDialogs> messages_savedDialogs::fetch(TlBufferParser &p) {
  return make_tl_object<messages_savedDialogs>(p);
}

messages_savedDialogs::messages_savedDialogs(TlBufferParser &p)
  : dialogs_(TlFetchBoxed<TlFetchVector<TlFetchObject<SavedDialog>>, 481674261>::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_savedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savedDialogs");
    { s.store_vector_begin("dialogs", dialogs_.size()); for (const auto &_value : dialogs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_savedDialogsSlice::ID;

object_ptr<messages_SavedDialogs> messages_savedDialogsSlice::fetch(TlBufferParser &p) {
  return make_tl_object<messages_savedDialogsSlice>(p);
}

messages_savedDialogsSlice::messages_savedDialogsSlice(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , dialogs_(TlFetchBoxed<TlFetchVector<TlFetchObject<SavedDialog>>, 481674261>::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_savedDialogsSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savedDialogsSlice");
    s.store_field("count", count_);
    { s.store_vector_begin("dialogs", dialogs_.size()); for (const auto &_value : dialogs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_savedDialogsNotModified::ID;

object_ptr<messages_SavedDialogs> messages_savedDialogsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_savedDialogsNotModified>(p);
}

messages_savedDialogsNotModified::messages_savedDialogsNotModified(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
{}

void messages_savedDialogsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savedDialogsNotModified");
    s.store_field("count", count_);
    s.store_class_end();
  }
}

object_ptr<payments_CheckCanSendGiftResult> payments_CheckCanSendGiftResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case payments_checkCanSendGiftResultOk::ID:
      return payments_checkCanSendGiftResultOk::fetch(p);
    case payments_checkCanSendGiftResultFail::ID:
      return payments_checkCanSendGiftResultFail::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t payments_checkCanSendGiftResultOk::ID;

object_ptr<payments_CheckCanSendGiftResult> payments_checkCanSendGiftResultOk::fetch(TlBufferParser &p) {
  return make_tl_object<payments_checkCanSendGiftResultOk>();
}

void payments_checkCanSendGiftResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.checkCanSendGiftResultOk");
    s.store_class_end();
  }
}

const std::int32_t payments_checkCanSendGiftResultFail::ID;

object_ptr<payments_CheckCanSendGiftResult> payments_checkCanSendGiftResultFail::fetch(TlBufferParser &p) {
  return make_tl_object<payments_checkCanSendGiftResultFail>(p);
}

payments_checkCanSendGiftResultFail::payments_checkCanSendGiftResultFail(TlBufferParser &p)
  : reason_(TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p))
{}

void payments_checkCanSendGiftResultFail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.checkCanSendGiftResultFail");
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_class_end();
  }
}

payments_resaleStarGifts::payments_resaleStarGifts()
  : flags_()
  , count_()
  , gifts_()
  , next_offset_()
  , attributes_()
  , attributes_hash_()
  , chats_()
  , counters_()
  , users_()
{}

const std::int32_t payments_resaleStarGifts::ID;

object_ptr<payments_resaleStarGifts> payments_resaleStarGifts::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_resaleStarGifts> res = make_tl_object<payments_resaleStarGifts>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->gifts_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StarGift>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->attributes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StarGiftAttribute>>, 481674261>::parse(p); }
  if (var0 & 2) { res->attributes_hash_ = TlFetchLong::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  if (var0 & 4) { res->counters_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starGiftAttributeCounter>, 783398488>>, 481674261>::parse(p); }
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_resaleStarGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.resaleStarGifts");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("gifts", gifts_.size()); for (const auto &_value : gifts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    if (var0 & 2) { { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { s.store_field("attributes_hash", attributes_hash_); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 4) { { s.store_vector_begin("counters", counters_.size()); for (const auto &_value : counters_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_starsStatus::payments_starsStatus()
  : flags_()
  , balance_()
  , subscriptions_()
  , subscriptions_next_offset_()
  , subscriptions_missing_balance_()
  , history_()
  , next_offset_()
  , chats_()
  , users_()
{}

const std::int32_t payments_starsStatus::ID;

object_ptr<payments_starsStatus> payments_starsStatus::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_starsStatus> res = make_tl_object<payments_starsStatus>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->balance_ = TlFetchObject<StarsAmount>::parse(p);
  if (var0 & 2) { res->subscriptions_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starsSubscription>, 779004698>>, 481674261>::parse(p); }
  if (var0 & 4) { res->subscriptions_next_offset_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->subscriptions_missing_balance_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->history_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starsTransaction>, 325426864>>, 481674261>::parse(p); }
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_starsStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starsStatus");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("balance", static_cast<const BaseObject *>(balance_.get()));
    if (var0 & 2) { { s.store_vector_begin("subscriptions", subscriptions_.size()); for (const auto &_value : subscriptions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { s.store_field("subscriptions_next_offset", subscriptions_next_offset_); }
    if (var0 & 16) { s.store_field("subscriptions_missing_balance", subscriptions_missing_balance_); }
    if (var0 & 8) { { s.store_vector_begin("history", history_.size()); for (const auto &_value : history_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_foundStories::stories_foundStories()
  : flags_()
  , count_()
  , stories_()
  , next_offset_()
  , chats_()
  , users_()
{}

const std::int32_t stories_foundStories::ID;

object_ptr<stories_foundStories> stories_foundStories::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stories_foundStories> res = make_tl_object<stories_foundStories>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->stories_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<foundStory>, -394605632>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void stories_foundStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.foundStories");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("stories", stories_.size()); for (const auto &_value : stories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_storyViewsList::stories_storyViewsList()
  : flags_()
  , count_()
  , views_count_()
  , forwards_count_()
  , reactions_count_()
  , views_()
  , chats_()
  , users_()
  , next_offset_()
{}

const std::int32_t stories_storyViewsList::ID;

object_ptr<stories_storyViewsList> stories_storyViewsList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stories_storyViewsList> res = make_tl_object<stories_storyViewsList>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->views_count_ = TlFetchInt::parse(p);
  res->forwards_count_ = TlFetchInt::parse(p);
  res->reactions_count_ = TlFetchInt::parse(p);
  res->views_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StoryView>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void stories_storyViewsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.storyViewsList");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    s.store_field("views_count", views_count_);
    s.store_field("forwards_count", forwards_count_);
    s.store_field("reactions_count", reactions_count_);
    { s.store_vector_begin("views", views_.size()); for (const auto &_value : views_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    s.store_class_end();
  }
}

const std::int32_t users_userFull::ID;

object_ptr<users_userFull> users_userFull::fetch(TlBufferParser &p) {
  return make_tl_object<users_userFull>(p);
}

users_userFull::users_userFull(TlBufferParser &p)
  : full_user_(TlFetchBoxed<TlFetchObject<userFull>, -1607745218>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void users_userFull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.userFull");
    s.store_object_field("full_user", static_cast<const BaseObject *>(full_user_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_cancelPasswordEmail::ID;

void account_cancelPasswordEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1043606090);
}

void account_cancelPasswordEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1043606090);
}

void account_cancelPasswordEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.cancelPasswordEmail");
    s.store_class_end();
  }
}

account_cancelPasswordEmail::ReturnType account_cancelPasswordEmail::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_deleteAutoSaveExceptions::ID;

void account_deleteAutoSaveExceptions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1404829728);
}

void account_deleteAutoSaveExceptions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1404829728);
}

void account_deleteAutoSaveExceptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.deleteAutoSaveExceptions");
    s.store_class_end();
  }
}

account_deleteAutoSaveExceptions::ReturnType account_deleteAutoSaveExceptions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_getChannelRestrictedStatusEmojis::account_getChannelRestrictedStatusEmojis(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getChannelRestrictedStatusEmojis::ID;

void account_getChannelRestrictedStatusEmojis::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(900325589);
  TlStoreBinary::store(hash_, s);
}

void account_getChannelRestrictedStatusEmojis::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(900325589);
  TlStoreBinary::store(hash_, s);
}

void account_getChannelRestrictedStatusEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getChannelRestrictedStatusEmojis");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getChannelRestrictedStatusEmojis::ReturnType account_getChannelRestrictedStatusEmojis::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EmojiList>::parse(p);
#undef FAIL
}

account_getNotifyExceptions::account_getNotifyExceptions(int32 flags_, bool compare_sound_, bool compare_stories_, object_ptr<InputNotifyPeer> &&peer_)
  : flags_(flags_)
  , compare_sound_(compare_sound_)
  , compare_stories_(compare_stories_)
  , peer_(std::move(peer_))
{}

const std::int32_t account_getNotifyExceptions::ID;

void account_getNotifyExceptions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1398240377);
  TlStoreBinary::store((var0 = flags_ | (compare_sound_ << 1) | (compare_stories_ << 2)), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
}

void account_getNotifyExceptions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1398240377);
  TlStoreBinary::store((var0 = flags_ | (compare_sound_ << 1) | (compare_stories_ << 2)), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
}

void account_getNotifyExceptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getNotifyExceptions");
    s.store_field("flags", (var0 = flags_ | (compare_sound_ << 1) | (compare_stories_ << 2)));
    if (var0 & 2) { s.store_field("compare_sound", true); }
    if (var0 & 4) { s.store_field("compare_stories", true); }
    if (var0 & 1) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    s.store_class_end();
  }
}

account_getNotifyExceptions::ReturnType account_getNotifyExceptions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

const std::int32_t account_getPassword::ID;

void account_getPassword::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1418342645);
}

void account_getPassword::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1418342645);
}

void account_getPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getPassword");
    s.store_class_end();
  }
}

account_getPassword::ReturnType account_getPassword::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_password>, -1787080453>::parse(p);
#undef FAIL
}

account_getTheme::account_getTheme(string const &format_, object_ptr<InputTheme> &&theme_)
  : format_(format_)
  , theme_(std::move(theme_))
{}

const std::int32_t account_getTheme::ID;

void account_getTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(978872812);
  TlStoreString::store(format_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
}

void account_getTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(978872812);
  TlStoreString::store(format_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
}

void account_getTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getTheme");
    s.store_field("format", format_);
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

account_getTheme::ReturnType account_getTheme::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<theme>, -1609668650>::parse(p);
#undef FAIL
}

account_invalidateSignInCodes::account_invalidateSignInCodes(array<string> &&codes_)
  : codes_(std::move(codes_))
{}

const std::int32_t account_invalidateSignInCodes::ID;

void account_invalidateSignInCodes::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-896866118);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(codes_, s);
}

void account_invalidateSignInCodes::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-896866118);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(codes_, s);
}

void account_invalidateSignInCodes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.invalidateSignInCodes");
    { s.store_vector_begin("codes", codes_.size()); for (const auto &_value : codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_invalidateSignInCodes::ReturnType account_invalidateSignInCodes::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_reportProfilePhoto::account_reportProfilePhoto(object_ptr<InputPeer> &&peer_, object_ptr<InputPhoto> &&photo_id_, object_ptr<ReportReason> &&reason_, string const &message_)
  : peer_(std::move(peer_))
  , photo_id_(std::move(photo_id_))
  , reason_(std::move(reason_))
  , message_(message_)
{}

const std::int32_t account_reportProfilePhoto::ID;

void account_reportProfilePhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-91437323);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reason_, s);
  TlStoreString::store(message_, s);
}

void account_reportProfilePhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-91437323);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reason_, s);
  TlStoreString::store(message_, s);
}

void account_reportProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.reportProfilePhoto");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("photo_id", static_cast<const BaseObject *>(photo_id_.get()));
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_field("message", message_);
    s.store_class_end();
  }
}

account_reportProfilePhoto::ReturnType account_reportProfilePhoto::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_resetWebAuthorizations::ID;

void account_resetWebAuthorizations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1747789204);
}

void account_resetWebAuthorizations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1747789204);
}

void account_resetWebAuthorizations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetWebAuthorizations");
    s.store_class_end();
  }
}

account_resetWebAuthorizations::ReturnType account_resetWebAuthorizations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_saveWallPaper::account_saveWallPaper(object_ptr<InputWallPaper> &&wallpaper_, bool unsave_, object_ptr<wallPaperSettings> &&settings_)
  : wallpaper_(std::move(wallpaper_))
  , unsave_(unsave_)
  , settings_(std::move(settings_))
{}

const std::int32_t account_saveWallPaper::ID;

void account_saveWallPaper::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1817860919);
  TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s);
  TlStoreBool::store(unsave_, s);
  TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s);
}

void account_saveWallPaper::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1817860919);
  TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s);
  TlStoreBool::store(unsave_, s);
  TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s);
}

void account_saveWallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.saveWallPaper");
    s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get()));
    s.store_field("unsave", unsave_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_saveWallPaper::ReturnType account_saveWallPaper::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateUsername::account_updateUsername(string const &username_)
  : username_(username_)
{}

const std::int32_t account_updateUsername::ID;

void account_updateUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1040964988);
  TlStoreString::store(username_, s);
}

void account_updateUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1040964988);
  TlStoreString::store(username_, s);
}

void account_updateUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateUsername");
    s.store_field("username", username_);
    s.store_class_end();
  }
}

account_updateUsername::ReturnType account_updateUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<User>::parse(p);
#undef FAIL
}

auth_signIn::auth_signIn(int32 flags_, string const &phone_number_, string const &phone_code_hash_, string const &phone_code_, object_ptr<EmailVerification> &&email_verification_)
  : flags_(flags_)
  , phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , phone_code_(phone_code_)
  , email_verification_(std::move(email_verification_))
{}

const std::int32_t auth_signIn::ID;

void auth_signIn::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1923962543);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  if (var0 & 1) { TlStoreString::store(phone_code_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(email_verification_, s); }
}

void auth_signIn::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1923962543);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  if (var0 & 1) { TlStoreString::store(phone_code_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(email_verification_, s); }
}

void auth_signIn::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.signIn");
    s.store_field("flags", (var0 = flags_));
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    if (var0 & 1) { s.store_field("phone_code", phone_code_); }
    if (var0 & 2) { s.store_object_field("email_verification", static_cast<const BaseObject *>(email_verification_.get())); }
    s.store_class_end();
  }
}

auth_signIn::ReturnType auth_signIn::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_Authorization>::parse(p);
#undef FAIL
}

auth_signUp::auth_signUp(int32 flags_, bool no_joined_notifications_, string const &phone_number_, string const &phone_code_hash_, string const &first_name_, string const &last_name_)
  : flags_(flags_)
  , no_joined_notifications_(no_joined_notifications_)
  , phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , first_name_(first_name_)
  , last_name_(last_name_)
{}

const std::int32_t auth_signUp::ID;

void auth_signUp::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1429752041);
  TlStoreBinary::store((var0 = flags_ | (no_joined_notifications_ << 0)), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
}

void auth_signUp::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1429752041);
  TlStoreBinary::store((var0 = flags_ | (no_joined_notifications_ << 0)), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
}

void auth_signUp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.signUp");
    s.store_field("flags", (var0 = flags_ | (no_joined_notifications_ << 0)));
    if (var0 & 1) { s.store_field("no_joined_notifications", true); }
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_class_end();
  }
}

auth_signUp::ReturnType auth_signUp::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_Authorization>::parse(p);
#undef FAIL
}

bots_allowSendMessage::bots_allowSendMessage(object_ptr<InputUser> &&bot_)
  : bot_(std::move(bot_))
{}

const std::int32_t bots_allowSendMessage::ID;

void bots_allowSendMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-248323089);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_allowSendMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-248323089);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_allowSendMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.allowSendMessage");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

bots_allowSendMessage::ReturnType bots_allowSendMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

bots_checkDownloadFileParams::bots_checkDownloadFileParams(object_ptr<InputUser> &&bot_, string const &file_name_, string const &url_)
  : bot_(std::move(bot_))
  , file_name_(file_name_)
  , url_(url_)
{}

const std::int32_t bots_checkDownloadFileParams::ID;

void bots_checkDownloadFileParams::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1342666121);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(file_name_, s);
  TlStoreString::store(url_, s);
}

void bots_checkDownloadFileParams::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1342666121);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(file_name_, s);
  TlStoreString::store(url_, s);
}

void bots_checkDownloadFileParams::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.checkDownloadFileParams");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("file_name", file_name_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

bots_checkDownloadFileParams::ReturnType bots_checkDownloadFileParams::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_setBotMenuButton::bots_setBotMenuButton(object_ptr<InputUser> &&user_id_, object_ptr<BotMenuButton> &&button_)
  : user_id_(std::move(user_id_))
  , button_(std::move(button_))
{}

const std::int32_t bots_setBotMenuButton::ID;

void bots_setBotMenuButton::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1157944655);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(button_, s);
}

void bots_setBotMenuButton::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1157944655);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(button_, s);
}

void bots_setBotMenuButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.setBotMenuButton");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_object_field("button", static_cast<const BaseObject *>(button_.get()));
    s.store_class_end();
  }
}

bots_setBotMenuButton::ReturnType bots_setBotMenuButton::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_setCustomVerification::bots_setCustomVerification(int32 flags_, bool enabled_, object_ptr<InputUser> &&bot_, object_ptr<InputPeer> &&peer_, string const &custom_description_)
  : flags_(flags_)
  , enabled_(enabled_)
  , bot_(std::move(bot_))
  , peer_(std::move(peer_))
  , custom_description_(custom_description_)
{}

const std::int32_t bots_setCustomVerification::ID;

void bots_setCustomVerification::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1953898563);
  TlStoreBinary::store((var0 = flags_ | (enabled_ << 1)), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreString::store(custom_description_, s); }
}

void bots_setCustomVerification::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1953898563);
  TlStoreBinary::store((var0 = flags_ | (enabled_ << 1)), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreString::store(custom_description_, s); }
}

void bots_setCustomVerification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.setCustomVerification");
    s.store_field("flags", (var0 = flags_ | (enabled_ << 1)));
    if (var0 & 2) { s.store_field("enabled", true); }
    if (var0 & 1) { s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get())); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 4) { s.store_field("custom_description", custom_description_); }
    s.store_class_end();
  }
}

bots_setCustomVerification::ReturnType bots_setCustomVerification::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_deactivateAllUsernames::channels_deactivateAllUsernames(object_ptr<InputChannel> &&channel_)
  : channel_(std::move(channel_))
{}

const std::int32_t channels_deactivateAllUsernames::ID;

void channels_deactivateAllUsernames::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(170155475);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_deactivateAllUsernames::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(170155475);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_deactivateAllUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.deactivateAllUsernames");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

channels_deactivateAllUsernames::ReturnType channels_deactivateAllUsernames::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_editTitle::channels_editTitle(object_ptr<InputChannel> &&channel_, string const &title_)
  : channel_(std::move(channel_))
  , title_(title_)
{}

const std::int32_t channels_editTitle::ID;

void channels_editTitle::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1450044624);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(title_, s);
}

void channels_editTitle::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1450044624);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(title_, s);
}

void channels_editTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.editTitle");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("title", title_);
    s.store_class_end();
  }
}

channels_editTitle::ReturnType channels_editTitle::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_getChannels::channels_getChannels(array<object_ptr<InputChannel>> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t channels_getChannels::ID;

void channels_getChannels::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(176122811);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void channels_getChannels::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(176122811);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void channels_getChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getChannels");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

channels_getChannels::ReturnType channels_getChannels::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

channels_getParticipants::channels_getParticipants(object_ptr<InputChannel> &&channel_, object_ptr<ChannelParticipantsFilter> &&filter_, int32 offset_, int32 limit_, int64 hash_)
  : channel_(std::move(channel_))
  , filter_(std::move(filter_))
  , offset_(offset_)
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t channels_getParticipants::ID;

void channels_getParticipants::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2010044880);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void channels_getParticipants::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2010044880);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void channels_getParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getParticipants");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

channels_getParticipants::ReturnType channels_getParticipants::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<channels_ChannelParticipants>::parse(p);
#undef FAIL
}

channels_toggleParticipantsHidden::channels_toggleParticipantsHidden(object_ptr<InputChannel> &&channel_, bool enabled_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
{}

const std::int32_t channels_toggleParticipantsHidden::ID;

void channels_toggleParticipantsHidden::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1785624660);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleParticipantsHidden::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1785624660);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleParticipantsHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleParticipantsHidden");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

channels_toggleParticipantsHidden::ReturnType channels_toggleParticipantsHidden::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_toggleSlowMode::channels_toggleSlowMode(object_ptr<InputChannel> &&channel_, int32 seconds_)
  : channel_(std::move(channel_))
  , seconds_(seconds_)
{}

const std::int32_t channels_toggleSlowMode::ID;

void channels_toggleSlowMode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-304832784);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(seconds_, s);
}

void channels_toggleSlowMode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-304832784);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(seconds_, s);
}

void channels_toggleSlowMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleSlowMode");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("seconds", seconds_);
    s.store_class_end();
  }
}

channels_toggleSlowMode::ReturnType channels_toggleSlowMode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_editCloseFriends::contacts_editCloseFriends(array<int64> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t contacts_editCloseFriends::ID;

void contacts_editCloseFriends::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1167653392);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void contacts_editCloseFriends::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1167653392);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void contacts_editCloseFriends::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.editCloseFriends");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

contacts_editCloseFriends::ReturnType contacts_editCloseFriends::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t contacts_exportContactToken::ID;

void contacts_exportContactToken::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-127582169);
}

void contacts_exportContactToken::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-127582169);
}

void contacts_exportContactToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.exportContactToken");
    s.store_class_end();
  }
}

contacts_exportContactToken::ReturnType contacts_exportContactToken::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<exportedContactToken>, 1103040667>::parse(p);
#undef FAIL
}

contacts_getContacts::contacts_getContacts(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t contacts_getContacts::ID;

void contacts_getContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1574346258);
  TlStoreBinary::store(hash_, s);
}

void contacts_getContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1574346258);
  TlStoreBinary::store(hash_, s);
}

void contacts_getContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getContacts");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

contacts_getContacts::ReturnType contacts_getContacts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<contacts_Contacts>::parse(p);
#undef FAIL
}

const std::int32_t contacts_getSaved::ID;

void contacts_getSaved::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2098076769);
}

void contacts_getSaved::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2098076769);
}

void contacts_getSaved::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getSaved");
    s.store_class_end();
  }
}

contacts_getSaved::ReturnType contacts_getSaved::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<savedPhoneContact>, 289586518>>, 481674261>::parse(p);
#undef FAIL
}

const std::int32_t help_getCdnConfig::ID;

void help_getCdnConfig::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1375900482);
}

void help_getCdnConfig::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1375900482);
}

void help_getCdnConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getCdnConfig");
    s.store_class_end();
  }
}

help_getCdnConfig::ReturnType help_getCdnConfig::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<cdnConfig>, 1462101002>::parse(p);
#undef FAIL
}

const std::int32_t help_getNearestDc::ID;

void help_getNearestDc::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(531836966);
}

void help_getNearestDc::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(531836966);
}

void help_getNearestDc::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getNearestDc");
    s.store_class_end();
  }
}

help_getNearestDc::ReturnType help_getNearestDc::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<nearestDc>, -1910892683>::parse(p);
#undef FAIL
}

help_getRecentMeUrls::help_getRecentMeUrls(string const &referer_)
  : referer_(referer_)
{}

const std::int32_t help_getRecentMeUrls::ID;

void help_getRecentMeUrls::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1036054804);
  TlStoreString::store(referer_, s);
}

void help_getRecentMeUrls::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1036054804);
  TlStoreString::store(referer_, s);
}

void help_getRecentMeUrls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getRecentMeUrls");
    s.store_field("referer", referer_);
    s.store_class_end();
  }
}

help_getRecentMeUrls::ReturnType help_getRecentMeUrls::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<help_recentMeUrls>, 235081943>::parse(p);
#undef FAIL
}

langpack_getDifference::langpack_getDifference(string const &lang_pack_, string const &lang_code_, int32 from_version_)
  : lang_pack_(lang_pack_)
  , lang_code_(lang_code_)
  , from_version_(from_version_)
{}

const std::int32_t langpack_getDifference::ID;

void langpack_getDifference::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-845657435);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBinary::store(from_version_, s);
}

void langpack_getDifference::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-845657435);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBinary::store(from_version_, s);
}

void langpack_getDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langpack.getDifference");
    s.store_field("lang_pack", lang_pack_);
    s.store_field("lang_code", lang_code_);
    s.store_field("from_version", from_version_);
    s.store_class_end();
  }
}

langpack_getDifference::ReturnType langpack_getDifference::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<langPackDifference>, -209337866>::parse(p);
#undef FAIL
}

messages_addChatUser::messages_addChatUser(int64 chat_id_, object_ptr<InputUser> &&user_id_, int32 fwd_limit_)
  : chat_id_(chat_id_)
  , user_id_(std::move(user_id_))
  , fwd_limit_(fwd_limit_)
{}

const std::int32_t messages_addChatUser::ID;

void messages_addChatUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-876162809);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(fwd_limit_, s);
}

void messages_addChatUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-876162809);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(fwd_limit_, s);
}

void messages_addChatUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.addChatUser");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("fwd_limit", fwd_limit_);
    s.store_class_end();
  }
}

messages_addChatUser::ReturnType messages_addChatUser::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_invitedUsers>, 2136862630>::parse(p);
#undef FAIL
}

messages_deleteChatUser::messages_deleteChatUser(int32 flags_, bool revoke_history_, int64 chat_id_, object_ptr<InputUser> &&user_id_)
  : flags_(flags_)
  , revoke_history_(revoke_history_)
  , chat_id_(chat_id_)
  , user_id_(std::move(user_id_))
{}

const std::int32_t messages_deleteChatUser::ID;

void messages_deleteChatUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1575461717);
  TlStoreBinary::store((var0 = flags_ | (revoke_history_ << 0)), s);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_deleteChatUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1575461717);
  TlStoreBinary::store((var0 = flags_ | (revoke_history_ << 0)), s);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_deleteChatUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteChatUser");
    s.store_field("flags", (var0 = flags_ | (revoke_history_ << 0)));
    if (var0 & 1) { s.store_field("revoke_history", true); }
    s.store_field("chat_id", chat_id_);
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

messages_deleteChatUser::ReturnType messages_deleteChatUser::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_deleteTopicHistory::messages_deleteTopicHistory(object_ptr<InputPeer> &&peer_, int32 top_msg_id_)
  : peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
{}

const std::int32_t messages_deleteTopicHistory::ID;

void messages_deleteTopicHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-763269360);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(top_msg_id_, s);
}

void messages_deleteTopicHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-763269360);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(top_msg_id_, s);
}

void messages_deleteTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteTopicHistory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("top_msg_id", top_msg_id_);
    s.store_class_end();
  }
}

messages_deleteTopicHistory::ReturnType messages_deleteTopicHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedHistory>, -1269012015>::parse(p);
#undef FAIL
}

messages_editChatPhoto::messages_editChatPhoto(int64 chat_id_, object_ptr<InputChatPhoto> &&photo_)
  : chat_id_(chat_id_)
  , photo_(std::move(photo_))
{}

const std::int32_t messages_editChatPhoto::ID;

void messages_editChatPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(903730804);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_, s);
}

void messages_editChatPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(903730804);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_, s);
}

void messages_editChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editChatPhoto");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

messages_editChatPhoto::ReturnType messages_editChatPhoto::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_editExportedChatInvite::messages_editExportedChatInvite(int32 flags_, bool revoked_, object_ptr<InputPeer> &&peer_, string const &link_, int32 expire_date_, int32 usage_limit_, bool request_needed_, string const &title_)
  : flags_(flags_)
  , revoked_(revoked_)
  , peer_(std::move(peer_))
  , link_(link_)
  , expire_date_(expire_date_)
  , usage_limit_(usage_limit_)
  , request_needed_(request_needed_)
  , title_(title_)
{}

const std::int32_t messages_editExportedChatInvite::ID;

void messages_editExportedChatInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1110823051);
  TlStoreBinary::store((var0 = flags_ | (revoked_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
  if (var0 & 1) { TlStoreBinary::store(expire_date_, s); }
  if (var0 & 2) { TlStoreBinary::store(usage_limit_, s); }
  if (var0 & 8) { TlStoreBool::store(request_needed_, s); }
  if (var0 & 16) { TlStoreString::store(title_, s); }
}

void messages_editExportedChatInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1110823051);
  TlStoreBinary::store((var0 = flags_ | (revoked_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
  if (var0 & 1) { TlStoreBinary::store(expire_date_, s); }
  if (var0 & 2) { TlStoreBinary::store(usage_limit_, s); }
  if (var0 & 8) { TlStoreBool::store(request_needed_, s); }
  if (var0 & 16) { TlStoreString::store(title_, s); }
}

void messages_editExportedChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editExportedChatInvite");
    s.store_field("flags", (var0 = flags_ | (revoked_ << 2)));
    if (var0 & 4) { s.store_field("revoked", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("link", link_);
    if (var0 & 1) { s.store_field("expire_date", expire_date_); }
    if (var0 & 2) { s.store_field("usage_limit", usage_limit_); }
    if (var0 & 8) { s.store_field("request_needed", request_needed_); }
    if (var0 & 16) { s.store_field("title", title_); }
    s.store_class_end();
  }
}

messages_editExportedChatInvite::ReturnType messages_editExportedChatInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_ExportedChatInvite>::parse(p);
#undef FAIL
}

messages_getArchivedStickers::messages_getArchivedStickers(int32 flags_, bool masks_, bool emojis_, int64 offset_id_, int32 limit_)
  : flags_(flags_)
  , masks_(masks_)
  , emojis_(emojis_)
  , offset_id_(offset_id_)
  , limit_(limit_)
{}

const std::int32_t messages_getArchivedStickers::ID;

void messages_getArchivedStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1475442322);
  TlStoreBinary::store((var0 = flags_ | (masks_ << 0) | (emojis_ << 1)), s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getArchivedStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1475442322);
  TlStoreBinary::store((var0 = flags_ | (masks_ << 0) | (emojis_ << 1)), s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getArchivedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getArchivedStickers");
    s.store_field("flags", (var0 = flags_ | (masks_ << 0) | (emojis_ << 1)));
    if (var0 & 1) { s.store_field("masks", true); }
    if (var0 & 2) { s.store_field("emojis", true); }
    s.store_field("offset_id", offset_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getArchivedStickers::ReturnType messages_getArchivedStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_archivedStickers>, 1338747336>::parse(p);
#undef FAIL
}

messages_getEmojiStatusGroups::messages_getEmojiStatusGroups(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getEmojiStatusGroups::ID;

void messages_getEmojiStatusGroups::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(785209037);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiStatusGroups::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(785209037);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiStatusGroups::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiStatusGroups");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getEmojiStatusGroups::ReturnType messages_getEmojiStatusGroups::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_EmojiGroups>::parse(p);
#undef FAIL
}

messages_getExportedChatInvite::messages_getExportedChatInvite(object_ptr<InputPeer> &&peer_, string const &link_)
  : peer_(std::move(peer_))
  , link_(link_)
{}

const std::int32_t messages_getExportedChatInvite::ID;

void messages_getExportedChatInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1937010524);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
}

void messages_getExportedChatInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1937010524);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
}

void messages_getExportedChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getExportedChatInvite");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("link", link_);
    s.store_class_end();
  }
}

messages_getExportedChatInvite::ReturnType messages_getExportedChatInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_ExportedChatInvite>::parse(p);
#undef FAIL
}

messages_getMessagesViews::messages_getMessagesViews(object_ptr<InputPeer> &&peer_, array<int32> &&id_, bool increment_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
  , increment_(increment_)
{}

const std::int32_t messages_getMessagesViews::ID;

void messages_getMessagesViews::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1468322785);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBool::store(increment_, s);
}

void messages_getMessagesViews::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1468322785);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBool::store(increment_, s);
}

void messages_getMessagesViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMessagesViews");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("increment", increment_);
    s.store_class_end();
  }
}

messages_getMessagesViews::ReturnType messages_getMessagesViews::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_messageViews>, -1228606141>::parse(p);
#undef FAIL
}

messages_getReplies::messages_getReplies(object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 offset_id_, int32 offset_date_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_, int64 hash_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , offset_id_(offset_id_)
  , offset_date_(offset_date_)
  , add_offset_(add_offset_)
  , limit_(limit_)
  , max_id_(max_id_)
  , min_id_(min_id_)
  , hash_(hash_)
{}

const std::int32_t messages_getReplies::ID;

void messages_getReplies::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(584962828);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getReplies::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(584962828);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getReplies");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("offset_id", offset_id_);
    s.store_field("offset_date", offset_date_);
    s.store_field("add_offset", add_offset_);
    s.store_field("limit", limit_);
    s.store_field("max_id", max_id_);
    s.store_field("min_id", min_id_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getReplies::ReturnType messages_getReplies::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_getSearchResultsPositions::messages_getSearchResultsPositions(int32 flags_, object_ptr<InputPeer> &&peer_, object_ptr<InputPeer> &&saved_peer_id_, object_ptr<MessagesFilter> &&filter_, int32 offset_id_, int32 limit_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , saved_peer_id_(std::move(saved_peer_id_))
  , filter_(std::move(filter_))
  , offset_id_(offset_id_)
  , limit_(limit_)
{}

const std::int32_t messages_getSearchResultsPositions::ID;

void messages_getSearchResultsPositions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1669386480);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getSearchResultsPositions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1669386480);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getSearchResultsPositions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSearchResultsPositions");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 4) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("offset_id", offset_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getSearchResultsPositions::ReturnType messages_getSearchResultsPositions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_searchResultsPositions>, 1404185519>::parse(p);
#undef FAIL
}

messages_getStickers::messages_getStickers(string const &emoticon_, int64 hash_)
  : emoticon_(emoticon_)
  , hash_(hash_)
{}

const std::int32_t messages_getStickers::ID;

void messages_getStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-710552671);
  TlStoreString::store(emoticon_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-710552671);
  TlStoreString::store(emoticon_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getStickers");
    s.store_field("emoticon", emoticon_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getStickers::ReturnType messages_getStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Stickers>::parse(p);
#undef FAIL
}

const std::int32_t messages_getSuggestedDialogFilters::ID;

void messages_getSuggestedDialogFilters::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1566780372);
}

void messages_getSuggestedDialogFilters::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1566780372);
}

void messages_getSuggestedDialogFilters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSuggestedDialogFilters");
    s.store_class_end();
  }
}

messages_getSuggestedDialogFilters::ReturnType messages_getSuggestedDialogFilters::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<dialogFilterSuggested>, 2004110666>>, 481674261>::parse(p);
#undef FAIL
}

messages_installStickerSet::messages_installStickerSet(object_ptr<InputStickerSet> &&stickerset_, bool archived_)
  : stickerset_(std::move(stickerset_))
  , archived_(archived_)
{}

const std::int32_t messages_installStickerSet::ID;

void messages_installStickerSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-946871200);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBool::store(archived_, s);
}

void messages_installStickerSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-946871200);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBool::store(archived_, s);
}

void messages_installStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.installStickerSet");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_field("archived", archived_);
    s.store_class_end();
  }
}

messages_installStickerSet::ReturnType messages_installStickerSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSetInstallResult>::parse(p);
#undef FAIL
}

messages_readDiscussion::messages_readDiscussion(object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 read_max_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , read_max_id_(read_max_id_)
{}

const std::int32_t messages_readDiscussion::ID;

void messages_readDiscussion::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-147740172);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(read_max_id_, s);
}

void messages_readDiscussion::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-147740172);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(read_max_id_, s);
}

void messages_readDiscussion::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readDiscussion");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("read_max_id", read_max_id_);
    s.store_class_end();
  }
}

messages_readDiscussion::ReturnType messages_readDiscussion::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_readFeaturedStickers::messages_readFeaturedStickers(array<int64> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t messages_readFeaturedStickers::ID;

void messages_readFeaturedStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1527873830);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_readFeaturedStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1527873830);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_readFeaturedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readFeaturedStickers");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_readFeaturedStickers::ReturnType messages_readFeaturedStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_requestUrlAuth::messages_requestUrlAuth(int32 flags_, object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 button_id_, string const &url_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , button_id_(button_id_)
  , url_(url_)
{}

const std::int32_t messages_requestUrlAuth::ID;

void messages_requestUrlAuth::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(428848198);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  if (var0 & 2) { TlStoreBinary::store(msg_id_, s); }
  if (var0 & 2) { TlStoreBinary::store(button_id_, s); }
  if (var0 & 4) { TlStoreString::store(url_, s); }
}

void messages_requestUrlAuth::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(428848198);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  if (var0 & 2) { TlStoreBinary::store(msg_id_, s); }
  if (var0 & 2) { TlStoreBinary::store(button_id_, s); }
  if (var0 & 4) { TlStoreString::store(url_, s); }
}

void messages_requestUrlAuth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.requestUrlAuth");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 2) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    if (var0 & 2) { s.store_field("msg_id", msg_id_); }
    if (var0 & 2) { s.store_field("button_id", button_id_); }
    if (var0 & 4) { s.store_field("url", url_); }
    s.store_class_end();
  }
}

messages_requestUrlAuth::ReturnType messages_requestUrlAuth::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<UrlAuthResult>::parse(p);
#undef FAIL
}

messages_searchStickerSets::messages_searchStickerSets(int32 flags_, bool exclude_featured_, string const &q_, int64 hash_)
  : flags_(flags_)
  , exclude_featured_(exclude_featured_)
  , q_(q_)
  , hash_(hash_)
{}

const std::int32_t messages_searchStickerSets::ID;

void messages_searchStickerSets::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(896555914);
  TlStoreBinary::store((var0 = flags_ | (exclude_featured_ << 0)), s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchStickerSets::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(896555914);
  TlStoreBinary::store((var0 = flags_ | (exclude_featured_ << 0)), s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchStickerSets");
    s.store_field("flags", (var0 = flags_ | (exclude_featured_ << 0)));
    if (var0 & 1) { s.store_field("exclude_featured", true); }
    s.store_field("q", q_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_searchStickerSets::ReturnType messages_searchStickerSets::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_FoundStickerSets>::parse(p);
#undef FAIL
}

messages_searchStickers::messages_searchStickers(int32 flags_, bool emojis_, string const &q_, string const &emoticon_, array<string> &&lang_code_, int32 offset_, int32 limit_, int64 hash_)
  : flags_(flags_)
  , emojis_(emojis_)
  , q_(q_)
  , emoticon_(emoticon_)
  , lang_code_(std::move(lang_code_))
  , offset_(offset_)
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t messages_searchStickers::ID;

void messages_searchStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(699516522);
  TlStoreBinary::store((var0 = flags_ | (emojis_ << 0)), s);
  TlStoreString::store(q_, s);
  TlStoreString::store(emoticon_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(lang_code_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(699516522);
  TlStoreBinary::store((var0 = flags_ | (emojis_ << 0)), s);
  TlStoreString::store(q_, s);
  TlStoreString::store(emoticon_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(lang_code_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchStickers");
    s.store_field("flags", (var0 = flags_ | (emojis_ << 0)));
    if (var0 & 1) { s.store_field("emojis", true); }
    s.store_field("q", q_);
    s.store_field("emoticon", emoticon_);
    { s.store_vector_begin("lang_code", lang_code_.size()); for (const auto &_value : lang_code_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_searchStickers::ReturnType messages_searchStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_FoundStickers>::parse(p);
#undef FAIL
}

messages_setBotPrecheckoutResults::messages_setBotPrecheckoutResults(int32 flags_, bool success_, int64 query_id_, string const &error_)
  : flags_(flags_)
  , success_(success_)
  , query_id_(query_id_)
  , error_(error_)
{}

const std::int32_t messages_setBotPrecheckoutResults::ID;

void messages_setBotPrecheckoutResults::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(163765653);
  TlStoreBinary::store((var0 = flags_ | (success_ << 1)), s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreString::store(error_, s); }
}

void messages_setBotPrecheckoutResults::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(163765653);
  TlStoreBinary::store((var0 = flags_ | (success_ << 1)), s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreString::store(error_, s); }
}

void messages_setBotPrecheckoutResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setBotPrecheckoutResults");
    s.store_field("flags", (var0 = flags_ | (success_ << 1)));
    if (var0 & 2) { s.store_field("success", true); }
    s.store_field("query_id", query_id_);
    if (var0 & 1) { s.store_field("error", error_); }
    s.store_class_end();
  }
}

messages_setBotPrecheckoutResults::ReturnType messages_setBotPrecheckoutResults::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_setHistoryTTL::messages_setHistoryTTL(object_ptr<InputPeer> &&peer_, int32 period_)
  : peer_(std::move(peer_))
  , period_(period_)
{}

const std::int32_t messages_setHistoryTTL::ID;

void messages_setHistoryTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1207017500);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(period_, s);
}

void messages_setHistoryTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1207017500);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(period_, s);
}

void messages_setHistoryTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setHistoryTTL");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("period", period_);
    s.store_class_end();
  }
}

messages_setHistoryTTL::ReturnType messages_setHistoryTTL::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_botCancelStarsSubscription::payments_botCancelStarsSubscription(int32 flags_, bool restore_, object_ptr<InputUser> &&user_id_, string const &charge_id_)
  : flags_(flags_)
  , restore_(restore_)
  , user_id_(std::move(user_id_))
  , charge_id_(charge_id_)
{}

const std::int32_t payments_botCancelStarsSubscription::ID;

void payments_botCancelStarsSubscription::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1845102114);
  TlStoreBinary::store((var0 = flags_ | (restore_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(charge_id_, s);
}

void payments_botCancelStarsSubscription::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1845102114);
  TlStoreBinary::store((var0 = flags_ | (restore_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(charge_id_, s);
}

void payments_botCancelStarsSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.botCancelStarsSubscription");
    s.store_field("flags", (var0 = flags_ | (restore_ << 0)));
    if (var0 & 1) { s.store_field("restore", true); }
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("charge_id", charge_id_);
    s.store_class_end();
  }
}

payments_botCancelStarsSubscription::ReturnType payments_botCancelStarsSubscription::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_exportInvoice::payments_exportInvoice(object_ptr<InputMedia> &&invoice_media_)
  : invoice_media_(std::move(invoice_media_))
{}

const std::int32_t payments_exportInvoice::ID;

void payments_exportInvoice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(261206117);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_media_, s);
}

void payments_exportInvoice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(261206117);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_media_, s);
}

void payments_exportInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.exportInvoice");
    s.store_object_field("invoice_media", static_cast<const BaseObject *>(invoice_media_.get()));
    s.store_class_end();
  }
}

payments_exportInvoice::ReturnType payments_exportInvoice::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_exportedInvoice>, -1362048039>::parse(p);
#undef FAIL
}

payments_getConnectedStarRefBot::payments_getConnectedStarRefBot(object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&bot_)
  : peer_(std::move(peer_))
  , bot_(std::move(bot_))
{}

const std::int32_t payments_getConnectedStarRefBot::ID;

void payments_getConnectedStarRefBot::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1210476304);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void payments_getConnectedStarRefBot::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1210476304);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void payments_getConnectedStarRefBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getConnectedStarRefBot");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

payments_getConnectedStarRefBot::ReturnType payments_getConnectedStarRefBot::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_connectedStarRefBots>, -1730811363>::parse(p);
#undef FAIL
}

payments_getStarGifts::payments_getStarGifts(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t payments_getStarGifts::ID;

void payments_getStarGifts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1000983152);
  TlStoreBinary::store(hash_, s);
}

void payments_getStarGifts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1000983152);
  TlStoreBinary::store(hash_, s);
}

void payments_getStarGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarGifts");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

payments_getStarGifts::ReturnType payments_getStarGifts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_StarGifts>::parse(p);
#undef FAIL
}

payments_getUniqueStarGift::payments_getUniqueStarGift(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t payments_getUniqueStarGift::ID;

void payments_getUniqueStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1583919758);
  TlStoreString::store(slug_, s);
}

void payments_getUniqueStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1583919758);
  TlStoreString::store(slug_, s);
}

void payments_getUniqueStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getUniqueStarGift");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

payments_getUniqueStarGift::ReturnType payments_getUniqueStarGift::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_uniqueStarGift>, 1097619176>::parse(p);
#undef FAIL
}

payments_refundStarsCharge::payments_refundStarsCharge(object_ptr<InputUser> &&user_id_, string const &charge_id_)
  : user_id_(std::move(user_id_))
  , charge_id_(charge_id_)
{}

const std::int32_t payments_refundStarsCharge::ID;

void payments_refundStarsCharge::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(632196938);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(charge_id_, s);
}

void payments_refundStarsCharge::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(632196938);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(charge_id_, s);
}

void payments_refundStarsCharge::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.refundStarsCharge");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("charge_id", charge_id_);
    s.store_class_end();
  }
}

payments_refundStarsCharge::ReturnType payments_refundStarsCharge::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

const std::int32_t phone_getCallConfig::ID;

void phone_getCallConfig::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1430593449);
}

void phone_getCallConfig::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1430593449);
}

void phone_getCallConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.getCallConfig");
    s.store_class_end();
  }
}

phone_getCallConfig::ReturnType phone_getCallConfig::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p);
#undef FAIL
}

stickers_changeStickerPosition::stickers_changeStickerPosition(object_ptr<InputDocument> &&sticker_, int32 position_)
  : sticker_(std::move(sticker_))
  , position_(position_)
{}

const std::int32_t stickers_changeStickerPosition::ID;

void stickers_changeStickerPosition::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-4795190);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
  TlStoreBinary::store(position_, s);
}

void stickers_changeStickerPosition::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-4795190);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
  TlStoreBinary::store(position_, s);
}

void stickers_changeStickerPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.changeStickerPosition");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("position", position_);
    s.store_class_end();
  }
}

stickers_changeStickerPosition::ReturnType stickers_changeStickerPosition::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stories_getStoriesByID::stories_getStoriesByID(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t stories_getStoriesByID::ID;

void stories_getStoriesByID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1467271796);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_getStoriesByID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1467271796);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_getStoriesByID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getStoriesByID");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_getStoriesByID::ReturnType stories_getStoriesByID::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_stories>, 1673780490>::parse(p);
#undef FAIL
}

stories_getStoriesViews::stories_getStoriesViews(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t stories_getStoriesViews::ID;

void stories_getStoriesViews::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(685862088);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_getStoriesViews::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(685862088);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_getStoriesViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getStoriesViews");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_getStoriesViews::ReturnType stories_getStoriesViews::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_storyViews>, -560009955>::parse(p);
#undef FAIL
}

stories_readStories::stories_readStories(object_ptr<InputPeer> &&peer_, int32 max_id_)
  : peer_(std::move(peer_))
  , max_id_(max_id_)
{}

const std::int32_t stories_readStories::ID;

void stories_readStories::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1521034552);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
}

void stories_readStories::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1521034552);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
}

void stories_readStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.readStories");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

stories_readStories::ReturnType stories_readStories::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
#undef FAIL
}

stories_togglePeerStoriesHidden::stories_togglePeerStoriesHidden(object_ptr<InputPeer> &&peer_, bool hidden_)
  : peer_(std::move(peer_))
  , hidden_(hidden_)
{}

const std::int32_t stories_togglePeerStoriesHidden::ID;

void stories_togglePeerStoriesHidden::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1123805756);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(hidden_, s);
}

void stories_togglePeerStoriesHidden::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1123805756);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(hidden_, s);
}

void stories_togglePeerStoriesHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.togglePeerStoriesHidden");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("hidden", hidden_);
    s.store_class_end();
  }
}

stories_togglePeerStoriesHidden::ReturnType stories_togglePeerStoriesHidden::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

users_getFullUser::users_getFullUser(object_ptr<InputUser> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t users_getFullUser::ID;

void users_getFullUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1240508136);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void users_getFullUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1240508136);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void users_getFullUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.getFullUser");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

users_getFullUser::ReturnType users_getFullUser::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<users_userFull>, 997004590>::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
