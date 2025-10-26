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


availableReaction::availableReaction()
  : flags_()
  , inactive_()
  , premium_()
  , reaction_()
  , title_()
  , static_icon_()
  , appear_animation_()
  , select_animation_()
  , activate_animation_()
  , effect_animation_()
  , around_animation_()
  , center_icon_()
{}

const std::int32_t availableReaction::ID;

object_ptr<availableReaction> availableReaction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<availableReaction> res = make_tl_object<availableReaction>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->inactive_ = (var0 & 1) != 0;
  res->premium_ = (var0 & 4) != 0;
  res->reaction_ = TlFetchString<string>::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->static_icon_ = TlFetchObject<Document>::parse(p);
  res->appear_animation_ = TlFetchObject<Document>::parse(p);
  res->select_animation_ = TlFetchObject<Document>::parse(p);
  res->activate_animation_ = TlFetchObject<Document>::parse(p);
  res->effect_animation_ = TlFetchObject<Document>::parse(p);
  if (var0 & 2) { res->around_animation_ = TlFetchObject<Document>::parse(p); }
  if (var0 & 2) { res->center_icon_ = TlFetchObject<Document>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void availableReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "availableReaction");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (inactive_ << 0) | (premium_ << 2)));
    if (var0 & 1) { s.store_field("inactive", true); }
    if (var0 & 4) { s.store_field("premium", true); }
    s.store_field("reaction", reaction_);
    s.store_field("title", title_);
    s.store_object_field("static_icon", static_cast<const BaseObject *>(static_icon_.get()));
    s.store_object_field("appear_animation", static_cast<const BaseObject *>(appear_animation_.get()));
    s.store_object_field("select_animation", static_cast<const BaseObject *>(select_animation_.get()));
    s.store_object_field("activate_animation", static_cast<const BaseObject *>(activate_animation_.get()));
    s.store_object_field("effect_animation", static_cast<const BaseObject *>(effect_animation_.get()));
    if (var0 & 2) { s.store_object_field("around_animation", static_cast<const BaseObject *>(around_animation_.get())); }
    if (var0 & 2) { s.store_object_field("center_icon", static_cast<const BaseObject *>(center_icon_.get())); }
    s.store_class_end();
  }
}

botCommand::botCommand(string const &command_, string const &description_)
  : command_(command_)
  , description_(description_)
{}

const std::int32_t botCommand::ID;

object_ptr<botCommand> botCommand::fetch(TlBufferParser &p) {
  return make_tl_object<botCommand>(p);
}

botCommand::botCommand(TlBufferParser &p)
  : command_(TlFetchString<string>::parse(p))
  , description_(TlFetchString<string>::parse(p))
{}

void botCommand::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(command_, s);
  TlStoreString::store(description_, s);
}

void botCommand::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(command_, s);
  TlStoreString::store(description_, s);
}

void botCommand::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommand");
    s.store_field("command", command_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

const std::int32_t botCommandScopeDefault::ID;

void botCommandScopeDefault::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void botCommandScopeDefault::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void botCommandScopeDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeDefault");
    s.store_class_end();
  }
}

const std::int32_t botCommandScopeUsers::ID;

void botCommandScopeUsers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void botCommandScopeUsers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void botCommandScopeUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeUsers");
    s.store_class_end();
  }
}

const std::int32_t botCommandScopeChats::ID;

void botCommandScopeChats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void botCommandScopeChats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void botCommandScopeChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeChats");
    s.store_class_end();
  }
}

const std::int32_t botCommandScopeChatAdmins::ID;

void botCommandScopeChatAdmins::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void botCommandScopeChatAdmins::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void botCommandScopeChatAdmins::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeChatAdmins");
    s.store_class_end();
  }
}

botCommandScopePeer::botCommandScopePeer(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t botCommandScopePeer::ID;

void botCommandScopePeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void botCommandScopePeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void botCommandScopePeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopePeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

botCommandScopePeerAdmins::botCommandScopePeerAdmins(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t botCommandScopePeerAdmins::ID;

void botCommandScopePeerAdmins::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void botCommandScopePeerAdmins::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void botCommandScopePeerAdmins::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopePeerAdmins");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

botCommandScopePeerUser::botCommandScopePeerUser(object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&user_id_)
  : peer_(std::move(peer_))
  , user_id_(std::move(user_id_))
{}

const std::int32_t botCommandScopePeerUser::ID;

void botCommandScopePeerUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void botCommandScopePeerUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void botCommandScopePeerUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopePeerUser");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

botVerifierSettings::botVerifierSettings()
  : flags_()
  , can_modify_custom_description_()
  , icon_()
  , company_()
  , custom_description_()
{}

const std::int32_t botVerifierSettings::ID;

object_ptr<botVerifierSettings> botVerifierSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botVerifierSettings> res = make_tl_object<botVerifierSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->can_modify_custom_description_ = (var0 & 2) != 0;
  res->icon_ = TlFetchLong::parse(p);
  res->company_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->custom_description_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void botVerifierSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botVerifierSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (can_modify_custom_description_ << 1)));
    if (var0 & 2) { s.store_field("can_modify_custom_description", true); }
    s.store_field("icon", icon_);
    s.store_field("company", company_);
    if (var0 & 1) { s.store_field("custom_description", custom_description_); }
    s.store_class_end();
  }
}

businessChatLink::businessChatLink()
  : flags_()
  , link_()
  , message_()
  , entities_()
  , title_()
  , views_()
{}

const std::int32_t businessChatLink::ID;

object_ptr<businessChatLink> businessChatLink::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessChatLink> res = make_tl_object<businessChatLink>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->link_ = TlFetchString<string>::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 2) { res->title_ = TlFetchString<string>::parse(p); }
  res->views_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessChatLink");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("link", link_);
    s.store_field("message", message_);
    if (var0 & 1) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { s.store_field("title", title_); }
    s.store_field("views", views_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEvent::ID;

object_ptr<channelAdminLogEvent> channelAdminLogEvent::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEvent>(p);
}

channelAdminLogEvent::channelAdminLogEvent(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , action_(TlFetchObject<ChannelAdminLogEventAction>::parse(p))
{}

void channelAdminLogEvent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEvent");
    s.store_field("id", id_);
    s.store_field("date", date_);
    s.store_field("user_id", user_id_);
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

object_ptr<ChannelLocation> ChannelLocation::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case channelLocationEmpty::ID:
      return channelLocationEmpty::fetch(p);
    case channelLocation::ID:
      return channelLocation::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t channelLocationEmpty::ID;

object_ptr<ChannelLocation> channelLocationEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<channelLocationEmpty>();
}

void channelLocationEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelLocationEmpty");
    s.store_class_end();
  }
}

const std::int32_t channelLocation::ID;

object_ptr<ChannelLocation> channelLocation::fetch(TlBufferParser &p) {
  return make_tl_object<channelLocation>(p);
}

channelLocation::channelLocation(TlBufferParser &p)
  : geo_point_(TlFetchObject<GeoPoint>::parse(p))
  , address_(TlFetchString<string>::parse(p))
{}

void channelLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelLocation");
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    s.store_field("address", address_);
    s.store_class_end();
  }
}

object_ptr<ChatReactions> ChatReactions::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatReactionsNone::ID:
      return chatReactionsNone::fetch(p);
    case chatReactionsAll::ID:
      return chatReactionsAll::fetch(p);
    case chatReactionsSome::ID:
      return chatReactionsSome::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t chatReactionsNone::ID;

object_ptr<ChatReactions> chatReactionsNone::fetch(TlBufferParser &p) {
  return make_tl_object<chatReactionsNone>();
}

void chatReactionsNone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void chatReactionsNone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void chatReactionsNone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatReactionsNone");
    s.store_class_end();
  }
}

chatReactionsAll::chatReactionsAll()
  : flags_()
  , allow_custom_()
{}

chatReactionsAll::chatReactionsAll(int32 flags_, bool allow_custom_)
  : flags_(flags_)
  , allow_custom_(allow_custom_)
{}

const std::int32_t chatReactionsAll::ID;

object_ptr<ChatReactions> chatReactionsAll::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatReactionsAll> res = make_tl_object<chatReactionsAll>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->allow_custom_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chatReactionsAll::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (allow_custom_ << 0)), s);
}

void chatReactionsAll::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (allow_custom_ << 0)), s);
}

void chatReactionsAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatReactionsAll");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (allow_custom_ << 0)));
    if (var0 & 1) { s.store_field("allow_custom", true); }
    s.store_class_end();
  }
}

chatReactionsSome::chatReactionsSome(array<object_ptr<Reaction>> &&reactions_)
  : reactions_(std::move(reactions_))
{}

const std::int32_t chatReactionsSome::ID;

object_ptr<ChatReactions> chatReactionsSome::fetch(TlBufferParser &p) {
  return make_tl_object<chatReactionsSome>(p);
}

chatReactionsSome::chatReactionsSome(TlBufferParser &p)
  : reactions_(TlFetchBoxed<TlFetchVector<TlFetchObject<Reaction>>, 481674261>::parse(p))
{}

void chatReactionsSome::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(reactions_, s);
}

void chatReactionsSome::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(reactions_, s);
}

void chatReactionsSome::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatReactionsSome");
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

codeSettings::codeSettings(int32 flags_, bool allow_flashcall_, bool current_number_, bool allow_app_hash_, bool allow_missed_call_, bool allow_firebase_, bool unknown_number_, array<bytes> &&logout_tokens_, string const &token_, bool app_sandbox_)
  : flags_(flags_)
  , allow_flashcall_(allow_flashcall_)
  , current_number_(current_number_)
  , allow_app_hash_(allow_app_hash_)
  , allow_missed_call_(allow_missed_call_)
  , allow_firebase_(allow_firebase_)
  , unknown_number_(unknown_number_)
  , logout_tokens_(std::move(logout_tokens_))
  , token_(token_)
  , app_sandbox_(app_sandbox_)
{}

const std::int32_t codeSettings::ID;

void codeSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (allow_flashcall_ << 0) | (current_number_ << 1) | (allow_app_hash_ << 4) | (allow_missed_call_ << 5) | (allow_firebase_ << 7) | (unknown_number_ << 9)), s);
  if (var0 & 64) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(logout_tokens_, s); }
  if (var0 & 256) { TlStoreString::store(token_, s); }
  if (var0 & 256) { TlStoreBool::store(app_sandbox_, s); }
}

void codeSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (allow_flashcall_ << 0) | (current_number_ << 1) | (allow_app_hash_ << 4) | (allow_missed_call_ << 5) | (allow_firebase_ << 7) | (unknown_number_ << 9)), s);
  if (var0 & 64) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(logout_tokens_, s); }
  if (var0 & 256) { TlStoreString::store(token_, s); }
  if (var0 & 256) { TlStoreBool::store(app_sandbox_, s); }
}

void codeSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "codeSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (allow_flashcall_ << 0) | (current_number_ << 1) | (allow_app_hash_ << 4) | (allow_missed_call_ << 5) | (allow_firebase_ << 7) | (unknown_number_ << 9)));
    if (var0 & 1) { s.store_field("allow_flashcall", true); }
    if (var0 & 2) { s.store_field("current_number", true); }
    if (var0 & 16) { s.store_field("allow_app_hash", true); }
    if (var0 & 32) { s.store_field("allow_missed_call", true); }
    if (var0 & 128) { s.store_field("allow_firebase", true); }
    if (var0 & 512) { s.store_field("unknown_number", true); }
    if (var0 & 64) { { s.store_vector_begin("logout_tokens", logout_tokens_.size()); for (const auto &_value : logout_tokens_) { s.store_bytes_field("", _value); } s.store_class_end(); } }
    if (var0 & 256) { s.store_field("token", token_); }
    if (var0 & 256) { s.store_field("app_sandbox", app_sandbox_); }
    s.store_class_end();
  }
}

const std::int32_t contactStatus::ID;

object_ptr<contactStatus> contactStatus::fetch(TlBufferParser &p) {
  return make_tl_object<contactStatus>(p);
}

contactStatus::contactStatus(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , status_(TlFetchObject<UserStatus>::parse(p))
{}

void contactStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contactStatus");
    s.store_field("user_id", user_id_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

disallowedGiftsSettings::disallowedGiftsSettings()
  : flags_()
  , disallow_unlimited_stargifts_()
  , disallow_limited_stargifts_()
  , disallow_unique_stargifts_()
  , disallow_premium_gifts_()
{}

disallowedGiftsSettings::disallowedGiftsSettings(int32 flags_, bool disallow_unlimited_stargifts_, bool disallow_limited_stargifts_, bool disallow_unique_stargifts_, bool disallow_premium_gifts_)
  : flags_(flags_)
  , disallow_unlimited_stargifts_(disallow_unlimited_stargifts_)
  , disallow_limited_stargifts_(disallow_limited_stargifts_)
  , disallow_unique_stargifts_(disallow_unique_stargifts_)
  , disallow_premium_gifts_(disallow_premium_gifts_)
{}

const std::int32_t disallowedGiftsSettings::ID;

object_ptr<disallowedGiftsSettings> disallowedGiftsSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<disallowedGiftsSettings> res = make_tl_object<disallowedGiftsSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->disallow_unlimited_stargifts_ = (var0 & 1) != 0;
  res->disallow_limited_stargifts_ = (var0 & 2) != 0;
  res->disallow_unique_stargifts_ = (var0 & 4) != 0;
  res->disallow_premium_gifts_ = (var0 & 8) != 0;
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void disallowedGiftsSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (disallow_unlimited_stargifts_ << 0) | (disallow_limited_stargifts_ << 1) | (disallow_unique_stargifts_ << 2) | (disallow_premium_gifts_ << 3)), s);
}

void disallowedGiftsSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (disallow_unlimited_stargifts_ << 0) | (disallow_limited_stargifts_ << 1) | (disallow_unique_stargifts_ << 2) | (disallow_premium_gifts_ << 3)), s);
}

void disallowedGiftsSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "disallowedGiftsSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (disallow_unlimited_stargifts_ << 0) | (disallow_limited_stargifts_ << 1) | (disallow_unique_stargifts_ << 2) | (disallow_premium_gifts_ << 3)));
    if (var0 & 1) { s.store_field("disallow_unlimited_stargifts", true); }
    if (var0 & 2) { s.store_field("disallow_limited_stargifts", true); }
    if (var0 & 4) { s.store_field("disallow_unique_stargifts", true); }
    if (var0 & 8) { s.store_field("disallow_premium_gifts", true); }
    s.store_class_end();
  }
}

object_ptr<EncryptedChat> EncryptedChat::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case encryptedChatEmpty::ID:
      return encryptedChatEmpty::fetch(p);
    case encryptedChatWaiting::ID:
      return encryptedChatWaiting::fetch(p);
    case encryptedChatRequested::ID:
      return encryptedChatRequested::fetch(p);
    case encryptedChat::ID:
      return encryptedChat::fetch(p);
    case encryptedChatDiscarded::ID:
      return encryptedChatDiscarded::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t encryptedChatEmpty::ID;

object_ptr<EncryptedChat> encryptedChatEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<encryptedChatEmpty>(p);
}

encryptedChatEmpty::encryptedChatEmpty(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
{}

void encryptedChatEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedChatEmpty");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

encryptedChatWaiting::encryptedChatWaiting(int32 id_, int64 access_hash_, int32 date_, int64 admin_id_, int64 participant_id_)
  : id_(id_)
  , access_hash_(access_hash_)
  , date_(date_)
  , admin_id_(admin_id_)
  , participant_id_(participant_id_)
{}

const std::int32_t encryptedChatWaiting::ID;

object_ptr<EncryptedChat> encryptedChatWaiting::fetch(TlBufferParser &p) {
  return make_tl_object<encryptedChatWaiting>(p);
}

encryptedChatWaiting::encryptedChatWaiting(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
  , admin_id_(TlFetchLong::parse(p))
  , participant_id_(TlFetchLong::parse(p))
{}

void encryptedChatWaiting::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedChatWaiting");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("admin_id", admin_id_);
    s.store_field("participant_id", participant_id_);
    s.store_class_end();
  }
}

encryptedChatRequested::encryptedChatRequested()
  : flags_()
  , folder_id_()
  , id_()
  , access_hash_()
  , date_()
  , admin_id_()
  , participant_id_()
  , g_a_()
{}

encryptedChatRequested::encryptedChatRequested(int32 flags_, int32 folder_id_, int32 id_, int64 access_hash_, int32 date_, int64 admin_id_, int64 participant_id_, bytes &&g_a_)
  : flags_(flags_)
  , folder_id_(folder_id_)
  , id_(id_)
  , access_hash_(access_hash_)
  , date_(date_)
  , admin_id_(admin_id_)
  , participant_id_(participant_id_)
  , g_a_(std::move(g_a_))
{}

const std::int32_t encryptedChatRequested::ID;

object_ptr<EncryptedChat> encryptedChatRequested::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<encryptedChatRequested> res = make_tl_object<encryptedChatRequested>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->folder_id_ = TlFetchInt::parse(p); }
  res->id_ = TlFetchInt::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->admin_id_ = TlFetchLong::parse(p);
  res->participant_id_ = TlFetchLong::parse(p);
  res->g_a_ = TlFetchBytes<bytes>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void encryptedChatRequested::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedChatRequested");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("folder_id", folder_id_); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("admin_id", admin_id_);
    s.store_field("participant_id", participant_id_);
    s.store_bytes_field("g_a", g_a_);
    s.store_class_end();
  }
}

encryptedChat::encryptedChat(int32 id_, int64 access_hash_, int32 date_, int64 admin_id_, int64 participant_id_, bytes &&g_a_or_b_, int64 key_fingerprint_)
  : id_(id_)
  , access_hash_(access_hash_)
  , date_(date_)
  , admin_id_(admin_id_)
  , participant_id_(participant_id_)
  , g_a_or_b_(std::move(g_a_or_b_))
  , key_fingerprint_(key_fingerprint_)
{}

const std::int32_t encryptedChat::ID;

object_ptr<EncryptedChat> encryptedChat::fetch(TlBufferParser &p) {
  return make_tl_object<encryptedChat>(p);
}

encryptedChat::encryptedChat(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
  , admin_id_(TlFetchLong::parse(p))
  , participant_id_(TlFetchLong::parse(p))
  , g_a_or_b_(TlFetchBytes<bytes>::parse(p))
  , key_fingerprint_(TlFetchLong::parse(p))
{}

void encryptedChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedChat");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("admin_id", admin_id_);
    s.store_field("participant_id", participant_id_);
    s.store_bytes_field("g_a_or_b", g_a_or_b_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_class_end();
  }
}

encryptedChatDiscarded::encryptedChatDiscarded()
  : flags_()
  , history_deleted_()
  , id_()
{}

const std::int32_t encryptedChatDiscarded::ID;

object_ptr<EncryptedChat> encryptedChatDiscarded::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<encryptedChatDiscarded> res = make_tl_object<encryptedChatDiscarded>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->history_deleted_ = (var0 & 1) != 0;
  res->id_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void encryptedChatDiscarded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedChatDiscarded");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (history_deleted_ << 0)));
    if (var0 & 1) { s.store_field("history_deleted", true); }
    s.store_field("id", id_);
    s.store_class_end();
  }
}

factCheck::factCheck()
  : flags_()
  , need_check_()
  , country_()
  , text_()
  , hash_()
{}

const std::int32_t factCheck::ID;

object_ptr<factCheck> factCheck::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<factCheck> res = make_tl_object<factCheck>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->need_check_ = (var0 & 1) != 0;
  if (var0 & 2) { res->country_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->text_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p); }
  res->hash_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void factCheck::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "factCheck");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (need_check_ << 0)));
    if (var0 & 1) { s.store_field("need_check", true); }
    if (var0 & 2) { s.store_field("country", country_); }
    if (var0 & 2) { s.store_object_field("text", static_cast<const BaseObject *>(text_.get())); }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

const std::int32_t folderPeer::ID;

object_ptr<folderPeer> folderPeer::fetch(TlBufferParser &p) {
  return make_tl_object<folderPeer>(p);
}

folderPeer::folderPeer(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , folder_id_(TlFetchInt::parse(p))
{}

void folderPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "folderPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("folder_id", folder_id_);
    s.store_class_end();
  }
}

const std::int32_t highScore::ID;

object_ptr<highScore> highScore::fetch(TlBufferParser &p) {
  return make_tl_object<highScore>(p);
}

highScore::highScore(TlBufferParser &p)
  : pos_(TlFetchInt::parse(p))
  , user_id_(TlFetchLong::parse(p))
  , score_(TlFetchInt::parse(p))
{}

void highScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "highScore");
    s.store_field("pos", pos_);
    s.store_field("user_id", user_id_);
    s.store_field("score", score_);
    s.store_class_end();
  }
}

inputPaymentCredentialsSaved::inputPaymentCredentialsSaved(string const &id_, bytes &&tmp_password_)
  : id_(id_)
  , tmp_password_(std::move(tmp_password_))
{}

const std::int32_t inputPaymentCredentialsSaved::ID;

void inputPaymentCredentialsSaved::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(tmp_password_, s);
}

void inputPaymentCredentialsSaved::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(tmp_password_, s);
}

void inputPaymentCredentialsSaved::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPaymentCredentialsSaved");
    s.store_field("id", id_);
    s.store_bytes_field("tmp_password", tmp_password_);
    s.store_class_end();
  }
}

inputPaymentCredentials::inputPaymentCredentials(int32 flags_, bool save_, object_ptr<dataJSON> &&data_)
  : flags_(flags_)
  , save_(save_)
  , data_(std::move(data_))
{}

const std::int32_t inputPaymentCredentials::ID;

void inputPaymentCredentials::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (save_ << 0)), s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(data_, s);
}

void inputPaymentCredentials::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (save_ << 0)), s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(data_, s);
}

void inputPaymentCredentials::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPaymentCredentials");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (save_ << 0)));
    if (var0 & 1) { s.store_field("save", true); }
    s.store_object_field("data", static_cast<const BaseObject *>(data_.get()));
    s.store_class_end();
  }
}

inputPaymentCredentialsApplePay::inputPaymentCredentialsApplePay(object_ptr<dataJSON> &&payment_data_)
  : payment_data_(std::move(payment_data_))
{}

const std::int32_t inputPaymentCredentialsApplePay::ID;

void inputPaymentCredentialsApplePay::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(payment_data_, s);
}

void inputPaymentCredentialsApplePay::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(payment_data_, s);
}

void inputPaymentCredentialsApplePay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPaymentCredentialsApplePay");
    s.store_object_field("payment_data", static_cast<const BaseObject *>(payment_data_.get()));
    s.store_class_end();
  }
}

inputPaymentCredentialsGooglePay::inputPaymentCredentialsGooglePay(object_ptr<dataJSON> &&payment_token_)
  : payment_token_(std::move(payment_token_))
{}

const std::int32_t inputPaymentCredentialsGooglePay::ID;

void inputPaymentCredentialsGooglePay::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(payment_token_, s);
}

void inputPaymentCredentialsGooglePay::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(payment_token_, s);
}

void inputPaymentCredentialsGooglePay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPaymentCredentialsGooglePay");
    s.store_object_field("payment_token", static_cast<const BaseObject *>(payment_token_.get()));
    s.store_class_end();
  }
}

inputStickeredMediaPhoto::inputStickeredMediaPhoto(object_ptr<InputPhoto> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t inputStickeredMediaPhoto::ID;

void inputStickeredMediaPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputStickeredMediaPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputStickeredMediaPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickeredMediaPhoto");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

inputStickeredMediaDocument::inputStickeredMediaDocument(object_ptr<InputDocument> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t inputStickeredMediaDocument::ID;

void inputStickeredMediaDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputStickeredMediaDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputStickeredMediaDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickeredMediaDocument");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

const std::int32_t langPackDifference::ID;

object_ptr<langPackDifference> langPackDifference::fetch(TlBufferParser &p) {
  return make_tl_object<langPackDifference>(p);
}

langPackDifference::langPackDifference(TlBufferParser &p)
  : lang_code_(TlFetchString<string>::parse(p))
  , from_version_(TlFetchInt::parse(p))
  , version_(TlFetchInt::parse(p))
  , strings_(TlFetchBoxed<TlFetchVector<TlFetchObject<LangPackString>>, 481674261>::parse(p))
{}

void langPackDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langPackDifference");
    s.store_field("lang_code", lang_code_);
    s.store_field("from_version", from_version_);
    s.store_field("version", version_);
    { s.store_vector_begin("strings", strings_.size()); for (const auto &_value : strings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<MessageEntity> MessageEntity::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messageEntityUnknown::ID:
      return messageEntityUnknown::fetch(p);
    case messageEntityMention::ID:
      return messageEntityMention::fetch(p);
    case messageEntityHashtag::ID:
      return messageEntityHashtag::fetch(p);
    case messageEntityBotCommand::ID:
      return messageEntityBotCommand::fetch(p);
    case messageEntityUrl::ID:
      return messageEntityUrl::fetch(p);
    case messageEntityEmail::ID:
      return messageEntityEmail::fetch(p);
    case messageEntityBold::ID:
      return messageEntityBold::fetch(p);
    case messageEntityItalic::ID:
      return messageEntityItalic::fetch(p);
    case messageEntityCode::ID:
      return messageEntityCode::fetch(p);
    case messageEntityPre::ID:
      return messageEntityPre::fetch(p);
    case messageEntityTextUrl::ID:
      return messageEntityTextUrl::fetch(p);
    case messageEntityMentionName::ID:
      return messageEntityMentionName::fetch(p);
    case inputMessageEntityMentionName::ID:
      return inputMessageEntityMentionName::fetch(p);
    case messageEntityPhone::ID:
      return messageEntityPhone::fetch(p);
    case messageEntityCashtag::ID:
      return messageEntityCashtag::fetch(p);
    case messageEntityUnderline::ID:
      return messageEntityUnderline::fetch(p);
    case messageEntityStrike::ID:
      return messageEntityStrike::fetch(p);
    case messageEntityBankCard::ID:
      return messageEntityBankCard::fetch(p);
    case messageEntitySpoiler::ID:
      return messageEntitySpoiler::fetch(p);
    case messageEntityCustomEmoji::ID:
      return messageEntityCustomEmoji::fetch(p);
    case messageEntityBlockquote::ID:
      return messageEntityBlockquote::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

messageEntityUnknown::messageEntityUnknown(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityUnknown::ID;

object_ptr<MessageEntity> messageEntityUnknown::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityUnknown>(p);
}

messageEntityUnknown::messageEntityUnknown(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityUnknown::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityUnknown::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityUnknown");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityMention::messageEntityMention(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityMention::ID;

object_ptr<MessageEntity> messageEntityMention::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityMention>(p);
}

messageEntityMention::messageEntityMention(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityMention::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityMention::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityMention::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityMention");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityHashtag::messageEntityHashtag(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityHashtag::ID;

object_ptr<MessageEntity> messageEntityHashtag::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityHashtag>(p);
}

messageEntityHashtag::messageEntityHashtag(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityHashtag::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityHashtag::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityHashtag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityHashtag");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityBotCommand::messageEntityBotCommand(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityBotCommand::ID;

object_ptr<MessageEntity> messageEntityBotCommand::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityBotCommand>(p);
}

messageEntityBotCommand::messageEntityBotCommand(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityBotCommand::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBotCommand::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBotCommand::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityBotCommand");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityUrl::messageEntityUrl(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityUrl::ID;

object_ptr<MessageEntity> messageEntityUrl::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityUrl>(p);
}

messageEntityUrl::messageEntityUrl(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityUrl");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityEmail::messageEntityEmail(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityEmail::ID;

object_ptr<MessageEntity> messageEntityEmail::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityEmail>(p);
}

messageEntityEmail::messageEntityEmail(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityEmail");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityBold::messageEntityBold(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityBold::ID;

object_ptr<MessageEntity> messageEntityBold::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityBold>(p);
}

messageEntityBold::messageEntityBold(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityBold::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBold::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBold::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityBold");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityItalic::messageEntityItalic(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityItalic::ID;

object_ptr<MessageEntity> messageEntityItalic::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityItalic>(p);
}

messageEntityItalic::messageEntityItalic(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityItalic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityItalic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityItalic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityItalic");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityCode::messageEntityCode(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityCode::ID;

object_ptr<MessageEntity> messageEntityCode::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityCode>(p);
}

messageEntityCode::messageEntityCode(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityCode");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityPre::messageEntityPre(int32 offset_, int32 length_, string const &language_)
  : offset_(offset_)
  , length_(length_)
  , language_(language_)
{}

const std::int32_t messageEntityPre::ID;

object_ptr<MessageEntity> messageEntityPre::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityPre>(p);
}

messageEntityPre::messageEntityPre(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , language_(TlFetchString<string>::parse(p))
{}

void messageEntityPre::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreString::store(language_, s);
}

void messageEntityPre::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreString::store(language_, s);
}

void messageEntityPre::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityPre");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_field("language", language_);
    s.store_class_end();
  }
}

messageEntityTextUrl::messageEntityTextUrl(int32 offset_, int32 length_, string const &url_)
  : offset_(offset_)
  , length_(length_)
  , url_(url_)
{}

const std::int32_t messageEntityTextUrl::ID;

object_ptr<MessageEntity> messageEntityTextUrl::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityTextUrl>(p);
}

messageEntityTextUrl::messageEntityTextUrl(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void messageEntityTextUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreString::store(url_, s);
}

void messageEntityTextUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreString::store(url_, s);
}

void messageEntityTextUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityTextUrl");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

messageEntityMentionName::messageEntityMentionName(int32 offset_, int32 length_, int64 user_id_)
  : offset_(offset_)
  , length_(length_)
  , user_id_(user_id_)
{}

const std::int32_t messageEntityMentionName::ID;

object_ptr<MessageEntity> messageEntityMentionName::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityMentionName>(p);
}

messageEntityMentionName::messageEntityMentionName(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , user_id_(TlFetchLong::parse(p))
{}

void messageEntityMentionName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreBinary::store(user_id_, s);
}

void messageEntityMentionName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreBinary::store(user_id_, s);
}

void messageEntityMentionName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityMentionName");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

inputMessageEntityMentionName::inputMessageEntityMentionName(int32 offset_, int32 length_, object_ptr<InputUser> &&user_id_)
  : offset_(offset_)
  , length_(length_)
  , user_id_(std::move(user_id_))
{}

const std::int32_t inputMessageEntityMentionName::ID;

object_ptr<MessageEntity> inputMessageEntityMentionName::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessageEntityMentionName>(p);
}

inputMessageEntityMentionName::inputMessageEntityMentionName(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , user_id_(TlFetchObject<InputUser>::parse(p))
{}

void inputMessageEntityMentionName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void inputMessageEntityMentionName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void inputMessageEntityMentionName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageEntityMentionName");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

messageEntityPhone::messageEntityPhone(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityPhone::ID;

object_ptr<MessageEntity> messageEntityPhone::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityPhone>(p);
}

messageEntityPhone::messageEntityPhone(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityPhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityPhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityPhone");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityCashtag::messageEntityCashtag(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityCashtag::ID;

object_ptr<MessageEntity> messageEntityCashtag::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityCashtag>(p);
}

messageEntityCashtag::messageEntityCashtag(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityCashtag::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityCashtag::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityCashtag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityCashtag");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityUnderline::messageEntityUnderline(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityUnderline::ID;

object_ptr<MessageEntity> messageEntityUnderline::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityUnderline>(p);
}

messageEntityUnderline::messageEntityUnderline(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityUnderline::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityUnderline::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityUnderline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityUnderline");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityStrike::messageEntityStrike(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityStrike::ID;

object_ptr<MessageEntity> messageEntityStrike::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityStrike>(p);
}

messageEntityStrike::messageEntityStrike(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityStrike::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityStrike::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityStrike::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityStrike");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityBankCard::messageEntityBankCard(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityBankCard::ID;

object_ptr<MessageEntity> messageEntityBankCard::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityBankCard>(p);
}

messageEntityBankCard::messageEntityBankCard(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntityBankCard::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBankCard::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBankCard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityBankCard");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntitySpoiler::messageEntitySpoiler(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntitySpoiler::ID;

object_ptr<MessageEntity> messageEntitySpoiler::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntitySpoiler>(p);
}

messageEntitySpoiler::messageEntitySpoiler(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void messageEntitySpoiler::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntitySpoiler::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntitySpoiler::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntitySpoiler");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

messageEntityCustomEmoji::messageEntityCustomEmoji(int32 offset_, int32 length_, int64 document_id_)
  : offset_(offset_)
  , length_(length_)
  , document_id_(document_id_)
{}

const std::int32_t messageEntityCustomEmoji::ID;

object_ptr<MessageEntity> messageEntityCustomEmoji::fetch(TlBufferParser &p) {
  return make_tl_object<messageEntityCustomEmoji>(p);
}

messageEntityCustomEmoji::messageEntityCustomEmoji(TlBufferParser &p)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , document_id_(TlFetchLong::parse(p))
{}

void messageEntityCustomEmoji::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreBinary::store(document_id_, s);
}

void messageEntityCustomEmoji::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
  TlStoreBinary::store(document_id_, s);
}

void messageEntityCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityCustomEmoji");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_field("document_id", document_id_);
    s.store_class_end();
  }
}

messageEntityBlockquote::messageEntityBlockquote()
  : flags_()
  , collapsed_()
  , offset_()
  , length_()
{}

messageEntityBlockquote::messageEntityBlockquote(int32 flags_, bool collapsed_, int32 offset_, int32 length_)
  : flags_(flags_)
  , collapsed_(collapsed_)
  , offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityBlockquote::ID;

object_ptr<MessageEntity> messageEntityBlockquote::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageEntityBlockquote> res = make_tl_object<messageEntityBlockquote>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->collapsed_ = (var0 & 1) != 0;
  res->offset_ = TlFetchInt::parse(p);
  res->length_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageEntityBlockquote::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (collapsed_ << 0)), s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBlockquote::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (collapsed_ << 0)), s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBlockquote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityBlockquote");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (collapsed_ << 0)));
    if (var0 & 1) { s.store_field("collapsed", true); }
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

const std::int32_t messageReportOption::ID;

object_ptr<messageReportOption> messageReportOption::fetch(TlBufferParser &p) {
  return make_tl_object<messageReportOption>(p);
}

messageReportOption::messageReportOption(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , option_(TlFetchBytes<bytes>::parse(p))
{}

void messageReportOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReportOption");
    s.store_field("text", text_);
    s.store_bytes_field("option", option_);
    s.store_class_end();
  }
}

object_ptr<MessagesFilter> MessagesFilter::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputMessagesFilterEmpty::ID:
      return inputMessagesFilterEmpty::fetch(p);
    case inputMessagesFilterPhotos::ID:
      return inputMessagesFilterPhotos::fetch(p);
    case inputMessagesFilterVideo::ID:
      return inputMessagesFilterVideo::fetch(p);
    case inputMessagesFilterPhotoVideo::ID:
      return inputMessagesFilterPhotoVideo::fetch(p);
    case inputMessagesFilterDocument::ID:
      return inputMessagesFilterDocument::fetch(p);
    case inputMessagesFilterUrl::ID:
      return inputMessagesFilterUrl::fetch(p);
    case inputMessagesFilterGif::ID:
      return inputMessagesFilterGif::fetch(p);
    case inputMessagesFilterVoice::ID:
      return inputMessagesFilterVoice::fetch(p);
    case inputMessagesFilterMusic::ID:
      return inputMessagesFilterMusic::fetch(p);
    case inputMessagesFilterChatPhotos::ID:
      return inputMessagesFilterChatPhotos::fetch(p);
    case inputMessagesFilterPhoneCalls::ID:
      return inputMessagesFilterPhoneCalls::fetch(p);
    case inputMessagesFilterRoundVoice::ID:
      return inputMessagesFilterRoundVoice::fetch(p);
    case inputMessagesFilterRoundVideo::ID:
      return inputMessagesFilterRoundVideo::fetch(p);
    case inputMessagesFilterMyMentions::ID:
      return inputMessagesFilterMyMentions::fetch(p);
    case inputMessagesFilterGeo::ID:
      return inputMessagesFilterGeo::fetch(p);
    case inputMessagesFilterContacts::ID:
      return inputMessagesFilterContacts::fetch(p);
    case inputMessagesFilterPinned::ID:
      return inputMessagesFilterPinned::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputMessagesFilterEmpty::ID;

object_ptr<MessagesFilter> inputMessagesFilterEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterEmpty>();
}

void inputMessagesFilterEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterEmpty");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterPhotos::ID;

object_ptr<MessagesFilter> inputMessagesFilterPhotos::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterPhotos>();
}

void inputMessagesFilterPhotos::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterPhotos::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterPhotos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterPhotos");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterVideo::ID;

object_ptr<MessagesFilter> inputMessagesFilterVideo::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterVideo>();
}

void inputMessagesFilterVideo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterVideo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterVideo");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterPhotoVideo::ID;

object_ptr<MessagesFilter> inputMessagesFilterPhotoVideo::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterPhotoVideo>();
}

void inputMessagesFilterPhotoVideo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterPhotoVideo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterPhotoVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterPhotoVideo");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterDocument::ID;

object_ptr<MessagesFilter> inputMessagesFilterDocument::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterDocument>();
}

void inputMessagesFilterDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterDocument");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterUrl::ID;

object_ptr<MessagesFilter> inputMessagesFilterUrl::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterUrl>();
}

void inputMessagesFilterUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterUrl");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterGif::ID;

object_ptr<MessagesFilter> inputMessagesFilterGif::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterGif>();
}

void inputMessagesFilterGif::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterGif::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterGif::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterGif");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterVoice::ID;

object_ptr<MessagesFilter> inputMessagesFilterVoice::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterVoice>();
}

void inputMessagesFilterVoice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterVoice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterVoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterVoice");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterMusic::ID;

object_ptr<MessagesFilter> inputMessagesFilterMusic::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterMusic>();
}

void inputMessagesFilterMusic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterMusic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterMusic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterMusic");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterChatPhotos::ID;

object_ptr<MessagesFilter> inputMessagesFilterChatPhotos::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterChatPhotos>();
}

void inputMessagesFilterChatPhotos::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterChatPhotos::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterChatPhotos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterChatPhotos");
    s.store_class_end();
  }
}

inputMessagesFilterPhoneCalls::inputMessagesFilterPhoneCalls()
  : flags_()
  , missed_()
{}

inputMessagesFilterPhoneCalls::inputMessagesFilterPhoneCalls(int32 flags_, bool missed_)
  : flags_(flags_)
  , missed_(missed_)
{}

const std::int32_t inputMessagesFilterPhoneCalls::ID;

object_ptr<MessagesFilter> inputMessagesFilterPhoneCalls::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMessagesFilterPhoneCalls> res = make_tl_object<inputMessagesFilterPhoneCalls>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->missed_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMessagesFilterPhoneCalls::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (missed_ << 0)), s);
}

void inputMessagesFilterPhoneCalls::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (missed_ << 0)), s);
}

void inputMessagesFilterPhoneCalls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterPhoneCalls");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (missed_ << 0)));
    if (var0 & 1) { s.store_field("missed", true); }
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterRoundVoice::ID;

object_ptr<MessagesFilter> inputMessagesFilterRoundVoice::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterRoundVoice>();
}

void inputMessagesFilterRoundVoice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterRoundVoice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterRoundVoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterRoundVoice");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterRoundVideo::ID;

object_ptr<MessagesFilter> inputMessagesFilterRoundVideo::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterRoundVideo>();
}

void inputMessagesFilterRoundVideo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterRoundVideo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterRoundVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterRoundVideo");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterMyMentions::ID;

object_ptr<MessagesFilter> inputMessagesFilterMyMentions::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterMyMentions>();
}

void inputMessagesFilterMyMentions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterMyMentions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterMyMentions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterMyMentions");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterGeo::ID;

object_ptr<MessagesFilter> inputMessagesFilterGeo::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterGeo>();
}

void inputMessagesFilterGeo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterGeo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterGeo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterGeo");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterContacts::ID;

object_ptr<MessagesFilter> inputMessagesFilterContacts::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterContacts>();
}

void inputMessagesFilterContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterContacts");
    s.store_class_end();
  }
}

const std::int32_t inputMessagesFilterPinned::ID;

object_ptr<MessagesFilter> inputMessagesFilterPinned::fetch(TlBufferParser &p) {
  return make_tl_object<inputMessagesFilterPinned>();
}

void inputMessagesFilterPinned::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterPinned::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagesFilterPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagesFilterPinned");
    s.store_class_end();
  }
}

const std::int32_t paymentCharge::ID;

object_ptr<paymentCharge> paymentCharge::fetch(TlBufferParser &p) {
  return make_tl_object<paymentCharge>(p);
}

paymentCharge::paymentCharge(TlBufferParser &p)
  : id_(TlFetchString<string>::parse(p))
  , provider_charge_id_(TlFetchString<string>::parse(p))
{}

void paymentCharge::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentCharge");
    s.store_field("id", id_);
    s.store_field("provider_charge_id", provider_charge_id_);
    s.store_class_end();
  }
}

object_ptr<Peer> Peer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case peerUser::ID:
      return peerUser::fetch(p);
    case peerChat::ID:
      return peerChat::fetch(p);
    case peerChannel::ID:
      return peerChannel::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

peerUser::peerUser(int64 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t peerUser::ID;

object_ptr<Peer> peerUser::fetch(TlBufferParser &p) {
  return make_tl_object<peerUser>(p);
}

peerUser::peerUser(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
{}

void peerUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerUser");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

peerChat::peerChat(int64 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t peerChat::ID;

object_ptr<Peer> peerChat::fetch(TlBufferParser &p) {
  return make_tl_object<peerChat>(p);
}

peerChat::peerChat(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
{}

void peerChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

const std::int32_t peerChannel::ID;

object_ptr<Peer> peerChannel::fetch(TlBufferParser &p) {
  return make_tl_object<peerChannel>(p);
}

peerChannel::peerChannel(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
{}

void peerChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerChannel");
    s.store_field("channel_id", channel_id_);
    s.store_class_end();
  }
}

phoneCallProtocol::phoneCallProtocol()
  : flags_()
  , udp_p2p_()
  , udp_reflector_()
  , min_layer_()
  , max_layer_()
  , library_versions_()
{}

phoneCallProtocol::phoneCallProtocol(int32 flags_, bool udp_p2p_, bool udp_reflector_, int32 min_layer_, int32 max_layer_, array<string> &&library_versions_)
  : flags_(flags_)
  , udp_p2p_(udp_p2p_)
  , udp_reflector_(udp_reflector_)
  , min_layer_(min_layer_)
  , max_layer_(max_layer_)
  , library_versions_(std::move(library_versions_))
{}

const std::int32_t phoneCallProtocol::ID;

object_ptr<phoneCallProtocol> phoneCallProtocol::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneCallProtocol> res = make_tl_object<phoneCallProtocol>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->udp_p2p_ = (var0 & 1) != 0;
  res->udp_reflector_ = (var0 & 2) != 0;
  res->min_layer_ = TlFetchInt::parse(p);
  res->max_layer_ = TlFetchInt::parse(p);
  res->library_versions_ = TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void phoneCallProtocol::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (udp_p2p_ << 0) | (udp_reflector_ << 1)), s);
  TlStoreBinary::store(min_layer_, s);
  TlStoreBinary::store(max_layer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(library_versions_, s);
}

void phoneCallProtocol::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (udp_p2p_ << 0) | (udp_reflector_ << 1)), s);
  TlStoreBinary::store(min_layer_, s);
  TlStoreBinary::store(max_layer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(library_versions_, s);
}

void phoneCallProtocol::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallProtocol");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (udp_p2p_ << 0) | (udp_reflector_ << 1)));
    if (var0 & 1) { s.store_field("udp_p2p", true); }
    if (var0 & 2) { s.store_field("udp_reflector", true); }
    s.store_field("min_layer", min_layer_);
    s.store_field("max_layer", max_layer_);
    { s.store_vector_begin("library_versions", library_versions_.size()); for (const auto &_value : library_versions_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<PhoneConnection> PhoneConnection::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case phoneConnection::ID:
      return phoneConnection::fetch(p);
    case phoneConnectionWebrtc::ID:
      return phoneConnectionWebrtc::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

phoneConnection::phoneConnection()
  : flags_()
  , tcp_()
  , id_()
  , ip_()
  , ipv6_()
  , port_()
  , peer_tag_()
{}

const std::int32_t phoneConnection::ID;

object_ptr<PhoneConnection> phoneConnection::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneConnection> res = make_tl_object<phoneConnection>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->tcp_ = (var0 & 1) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->ip_ = TlFetchString<string>::parse(p);
  res->ipv6_ = TlFetchString<string>::parse(p);
  res->port_ = TlFetchInt::parse(p);
  res->peer_tag_ = TlFetchBytes<bytes>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void phoneConnection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneConnection");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (tcp_ << 0)));
    if (var0 & 1) { s.store_field("tcp", true); }
    s.store_field("id", id_);
    s.store_field("ip", ip_);
    s.store_field("ipv6", ipv6_);
    s.store_field("port", port_);
    s.store_bytes_field("peer_tag", peer_tag_);
    s.store_class_end();
  }
}

phoneConnectionWebrtc::phoneConnectionWebrtc()
  : flags_()
  , turn_()
  , stun_()
  , id_()
  , ip_()
  , ipv6_()
  , port_()
  , username_()
  , password_()
{}

const std::int32_t phoneConnectionWebrtc::ID;

object_ptr<PhoneConnection> phoneConnectionWebrtc::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneConnectionWebrtc> res = make_tl_object<phoneConnectionWebrtc>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->turn_ = (var0 & 1) != 0;
  res->stun_ = (var0 & 2) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->ip_ = TlFetchString<string>::parse(p);
  res->ipv6_ = TlFetchString<string>::parse(p);
  res->port_ = TlFetchInt::parse(p);
  res->username_ = TlFetchString<string>::parse(p);
  res->password_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void phoneConnectionWebrtc::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneConnectionWebrtc");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (turn_ << 0) | (stun_ << 1)));
    if (var0 & 1) { s.store_field("turn", true); }
    if (var0 & 2) { s.store_field("stun", true); }
    s.store_field("id", id_);
    s.store_field("ip", ip_);
    s.store_field("ipv6", ipv6_);
    s.store_field("port", port_);
    s.store_field("username", username_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

object_ptr<PrepaidGiveaway> PrepaidGiveaway::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case prepaidGiveaway::ID:
      return prepaidGiveaway::fetch(p);
    case prepaidStarsGiveaway::ID:
      return prepaidStarsGiveaway::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t prepaidGiveaway::ID;

object_ptr<PrepaidGiveaway> prepaidGiveaway::fetch(TlBufferParser &p) {
  return make_tl_object<prepaidGiveaway>(p);
}

prepaidGiveaway::prepaidGiveaway(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , months_(TlFetchInt::parse(p))
  , quantity_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void prepaidGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "prepaidGiveaway");
    s.store_field("id", id_);
    s.store_field("months", months_);
    s.store_field("quantity", quantity_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t prepaidStarsGiveaway::ID;

object_ptr<PrepaidGiveaway> prepaidStarsGiveaway::fetch(TlBufferParser &p) {
  return make_tl_object<prepaidStarsGiveaway>(p);
}

prepaidStarsGiveaway::prepaidStarsGiveaway(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , stars_(TlFetchLong::parse(p))
  , quantity_(TlFetchInt::parse(p))
  , boosts_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void prepaidStarsGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "prepaidStarsGiveaway");
    s.store_field("id", id_);
    s.store_field("stars", stars_);
    s.store_field("quantity", quantity_);
    s.store_field("boosts", boosts_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

object_ptr<ProfileTab> ProfileTab::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case profileTabPosts::ID:
      return profileTabPosts::fetch(p);
    case profileTabGifts::ID:
      return profileTabGifts::fetch(p);
    case profileTabMedia::ID:
      return profileTabMedia::fetch(p);
    case profileTabFiles::ID:
      return profileTabFiles::fetch(p);
    case profileTabMusic::ID:
      return profileTabMusic::fetch(p);
    case profileTabVoice::ID:
      return profileTabVoice::fetch(p);
    case profileTabLinks::ID:
      return profileTabLinks::fetch(p);
    case profileTabGifs::ID:
      return profileTabGifs::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t profileTabPosts::ID;

object_ptr<ProfileTab> profileTabPosts::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabPosts>();
}

void profileTabPosts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabPosts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabPosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabPosts");
    s.store_class_end();
  }
}

const std::int32_t profileTabGifts::ID;

object_ptr<ProfileTab> profileTabGifts::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabGifts>();
}

void profileTabGifts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabGifts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabGifts");
    s.store_class_end();
  }
}

const std::int32_t profileTabMedia::ID;

object_ptr<ProfileTab> profileTabMedia::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabMedia>();
}

void profileTabMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabMedia");
    s.store_class_end();
  }
}

const std::int32_t profileTabFiles::ID;

object_ptr<ProfileTab> profileTabFiles::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabFiles>();
}

void profileTabFiles::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabFiles::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabFiles");
    s.store_class_end();
  }
}

const std::int32_t profileTabMusic::ID;

object_ptr<ProfileTab> profileTabMusic::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabMusic>();
}

void profileTabMusic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabMusic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabMusic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabMusic");
    s.store_class_end();
  }
}

const std::int32_t profileTabVoice::ID;

object_ptr<ProfileTab> profileTabVoice::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabVoice>();
}

void profileTabVoice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabVoice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabVoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabVoice");
    s.store_class_end();
  }
}

const std::int32_t profileTabLinks::ID;

object_ptr<ProfileTab> profileTabLinks::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabLinks>();
}

void profileTabLinks::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabLinks::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabLinks");
    s.store_class_end();
  }
}

const std::int32_t profileTabGifs::ID;

object_ptr<ProfileTab> profileTabGifs::fetch(TlBufferParser &p) {
  return make_tl_object<profileTabGifs>();
}

void profileTabGifs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void profileTabGifs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void profileTabGifs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabGifs");
    s.store_class_end();
  }
}

object_ptr<SecurePasswordKdfAlgo> SecurePasswordKdfAlgo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case securePasswordKdfAlgoUnknown::ID:
      return securePasswordKdfAlgoUnknown::fetch(p);
    case securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::ID:
      return securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::fetch(p);
    case securePasswordKdfAlgoSHA512::ID:
      return securePasswordKdfAlgoSHA512::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t securePasswordKdfAlgoUnknown::ID;

object_ptr<SecurePasswordKdfAlgo> securePasswordKdfAlgoUnknown::fetch(TlBufferParser &p) {
  return make_tl_object<securePasswordKdfAlgoUnknown>();
}

void securePasswordKdfAlgoUnknown::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void securePasswordKdfAlgoUnknown::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void securePasswordKdfAlgoUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "securePasswordKdfAlgoUnknown");
    s.store_class_end();
  }
}

securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::securePasswordKdfAlgoPBKDF2HMACSHA512iter100000(bytes &&salt_)
  : salt_(std::move(salt_))
{}

const std::int32_t securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::ID;

object_ptr<SecurePasswordKdfAlgo> securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::fetch(TlBufferParser &p) {
  return make_tl_object<securePasswordKdfAlgoPBKDF2HMACSHA512iter100000>(p);
}

securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::securePasswordKdfAlgoPBKDF2HMACSHA512iter100000(TlBufferParser &p)
  : salt_(TlFetchBytes<bytes>::parse(p))
{}

void securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(salt_, s);
}

void securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(salt_, s);
}

void securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "securePasswordKdfAlgoPBKDF2HMACSHA512iter100000");
    s.store_bytes_field("salt", salt_);
    s.store_class_end();
  }
}

securePasswordKdfAlgoSHA512::securePasswordKdfAlgoSHA512(bytes &&salt_)
  : salt_(std::move(salt_))
{}

const std::int32_t securePasswordKdfAlgoSHA512::ID;

object_ptr<SecurePasswordKdfAlgo> securePasswordKdfAlgoSHA512::fetch(TlBufferParser &p) {
  return make_tl_object<securePasswordKdfAlgoSHA512>(p);
}

securePasswordKdfAlgoSHA512::securePasswordKdfAlgoSHA512(TlBufferParser &p)
  : salt_(TlFetchBytes<bytes>::parse(p))
{}

void securePasswordKdfAlgoSHA512::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(salt_, s);
}

void securePasswordKdfAlgoSHA512::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(salt_, s);
}

void securePasswordKdfAlgoSHA512::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "securePasswordKdfAlgoSHA512");
    s.store_bytes_field("salt", salt_);
    s.store_class_end();
  }
}

object_ptr<SecureRequiredType> SecureRequiredType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case secureRequiredType::ID:
      return secureRequiredType::fetch(p);
    case secureRequiredTypeOneOf::ID:
      return secureRequiredTypeOneOf::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

secureRequiredType::secureRequiredType()
  : flags_()
  , native_names_()
  , selfie_required_()
  , translation_required_()
  , type_()
{}

const std::int32_t secureRequiredType::ID;

object_ptr<SecureRequiredType> secureRequiredType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<secureRequiredType> res = make_tl_object<secureRequiredType>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->native_names_ = (var0 & 1) != 0;
  res->selfie_required_ = (var0 & 2) != 0;
  res->translation_required_ = (var0 & 4) != 0;
  res->type_ = TlFetchObject<SecureValueType>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void secureRequiredType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureRequiredType");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (native_names_ << 0) | (selfie_required_ << 1) | (translation_required_ << 2)));
    if (var0 & 1) { s.store_field("native_names", true); }
    if (var0 & 2) { s.store_field("selfie_required", true); }
    if (var0 & 4) { s.store_field("translation_required", true); }
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

const std::int32_t secureRequiredTypeOneOf::ID;

object_ptr<SecureRequiredType> secureRequiredTypeOneOf::fetch(TlBufferParser &p) {
  return make_tl_object<secureRequiredTypeOneOf>(p);
}

secureRequiredTypeOneOf::secureRequiredTypeOneOf(TlBufferParser &p)
  : types_(TlFetchBoxed<TlFetchVector<TlFetchObject<SecureRequiredType>>, 481674261>::parse(p))
{}

void secureRequiredTypeOneOf::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureRequiredTypeOneOf");
    { s.store_vector_begin("types", types_.size()); for (const auto &_value : types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

secureSecretSettings::secureSecretSettings(object_ptr<SecurePasswordKdfAlgo> &&secure_algo_, bytes &&secure_secret_, int64 secure_secret_id_)
  : secure_algo_(std::move(secure_algo_))
  , secure_secret_(std::move(secure_secret_))
  , secure_secret_id_(secure_secret_id_)
{}

const std::int32_t secureSecretSettings::ID;

object_ptr<secureSecretSettings> secureSecretSettings::fetch(TlBufferParser &p) {
  return make_tl_object<secureSecretSettings>(p);
}

secureSecretSettings::secureSecretSettings(TlBufferParser &p)
  : secure_algo_(TlFetchObject<SecurePasswordKdfAlgo>::parse(p))
  , secure_secret_(TlFetchBytes<bytes>::parse(p))
  , secure_secret_id_(TlFetchLong::parse(p))
{}

void secureSecretSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(secure_algo_, s);
  TlStoreString::store(secure_secret_, s);
  TlStoreBinary::store(secure_secret_id_, s);
}

void secureSecretSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(secure_algo_, s);
  TlStoreString::store(secure_secret_, s);
  TlStoreBinary::store(secure_secret_id_, s);
}

void secureSecretSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureSecretSettings");
    s.store_object_field("secure_algo", static_cast<const BaseObject *>(secure_algo_.get()));
    s.store_bytes_field("secure_secret", secure_secret_);
    s.store_field("secure_secret_id", secure_secret_id_);
    s.store_class_end();
  }
}

sendAsPeer::sendAsPeer()
  : flags_()
  , premium_required_()
  , peer_()
{}

const std::int32_t sendAsPeer::ID;

object_ptr<sendAsPeer> sendAsPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<sendAsPeer> res = make_tl_object<sendAsPeer>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->premium_required_ = (var0 & 1) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void sendAsPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendAsPeer");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (premium_required_ << 0)));
    if (var0 & 1) { s.store_field("premium_required", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

sponsoredMessage::sponsoredMessage()
  : flags_()
  , recommended_()
  , can_report_()
  , random_id_()
  , url_()
  , title_()
  , message_()
  , entities_()
  , photo_()
  , media_()
  , color_()
  , button_text_()
  , sponsor_info_()
  , additional_info_()
  , min_display_duration_()
  , max_display_duration_()
{}

const std::int32_t sponsoredMessage::ID;

object_ptr<sponsoredMessage> sponsoredMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<sponsoredMessage> res = make_tl_object<sponsoredMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->recommended_ = (var0 & 32) != 0;
  res->can_report_ = (var0 & 4096) != 0;
  res->random_id_ = TlFetchBytes<bytes>::parse(p);
  res->url_ = TlFetchString<string>::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 64) { res->photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 16384) { res->media_ = TlFetchObject<MessageMedia>::parse(p); }
  if (var0 & 8192) { res->color_ = TlFetchObject<PeerColor>::parse(p); }
  res->button_text_ = TlFetchString<string>::parse(p);
  if (var0 & 128) { res->sponsor_info_ = TlFetchString<string>::parse(p); }
  if (var0 & 256) { res->additional_info_ = TlFetchString<string>::parse(p); }
  if (var0 & 32768) { res->min_display_duration_ = TlFetchInt::parse(p); }
  if (var0 & 32768) { res->max_display_duration_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void sponsoredMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sponsoredMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (recommended_ << 5) | (can_report_ << 12)));
    if (var0 & 32) { s.store_field("recommended", true); }
    if (var0 & 4096) { s.store_field("can_report", true); }
    s.store_bytes_field("random_id", random_id_);
    s.store_field("url", url_);
    s.store_field("title", title_);
    s.store_field("message", message_);
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 64) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 16384) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 8192) { s.store_object_field("color", static_cast<const BaseObject *>(color_.get())); }
    s.store_field("button_text", button_text_);
    if (var0 & 128) { s.store_field("sponsor_info", sponsor_info_); }
    if (var0 & 256) { s.store_field("additional_info", additional_info_); }
    if (var0 & 32768) { s.store_field("min_display_duration", min_display_duration_); }
    if (var0 & 32768) { s.store_field("max_display_duration", max_display_duration_); }
    s.store_class_end();
  }
}

starsSubscriptionPricing::starsSubscriptionPricing(int32 period_, int64 amount_)
  : period_(period_)
  , amount_(amount_)
{}

const std::int32_t starsSubscriptionPricing::ID;

object_ptr<starsSubscriptionPricing> starsSubscriptionPricing::fetch(TlBufferParser &p) {
  return make_tl_object<starsSubscriptionPricing>(p);
}

starsSubscriptionPricing::starsSubscriptionPricing(TlBufferParser &p)
  : period_(TlFetchInt::parse(p))
  , amount_(TlFetchLong::parse(p))
{}

void starsSubscriptionPricing::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(period_, s);
  TlStoreBinary::store(amount_, s);
}

void starsSubscriptionPricing::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(period_, s);
  TlStoreBinary::store(amount_, s);
}

void starsSubscriptionPricing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsSubscriptionPricing");
    s.store_field("period", period_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

const std::int32_t statsGroupTopAdmin::ID;

object_ptr<statsGroupTopAdmin> statsGroupTopAdmin::fetch(TlBufferParser &p) {
  return make_tl_object<statsGroupTopAdmin>(p);
}

statsGroupTopAdmin::statsGroupTopAdmin(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , deleted_(TlFetchInt::parse(p))
  , kicked_(TlFetchInt::parse(p))
  , banned_(TlFetchInt::parse(p))
{}

void statsGroupTopAdmin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsGroupTopAdmin");
    s.store_field("user_id", user_id_);
    s.store_field("deleted", deleted_);
    s.store_field("kicked", kicked_);
    s.store_field("banned", banned_);
    s.store_class_end();
  }
}

const std::int32_t stickerPack::ID;

object_ptr<stickerPack> stickerPack::fetch(TlBufferParser &p) {
  return make_tl_object<stickerPack>(p);
}

stickerPack::stickerPack(TlBufferParser &p)
  : emoticon_(TlFetchString<string>::parse(p))
  , documents_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void stickerPack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerPack");
    s.store_field("emoticon", emoticon_);
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

todoList::todoList()
  : flags_()
  , others_can_append_()
  , others_can_complete_()
  , title_()
  , list_()
{}

todoList::todoList(int32 flags_, bool others_can_append_, bool others_can_complete_, object_ptr<textWithEntities> &&title_, array<object_ptr<todoItem>> &&list_)
  : flags_(flags_)
  , others_can_append_(others_can_append_)
  , others_can_complete_(others_can_complete_)
  , title_(std::move(title_))
  , list_(std::move(list_))
{}

const std::int32_t todoList::ID;

object_ptr<todoList> todoList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<todoList> res = make_tl_object<todoList>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->others_can_append_ = (var0 & 1) != 0;
  res->others_can_complete_ = (var0 & 2) != 0;
  res->title_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p);
  res->list_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<todoItem>, -878074577>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void todoList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (others_can_append_ << 0) | (others_can_complete_ << 1)), s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -878074577>>, 481674261>::store(list_, s);
}

void todoList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (others_can_append_ << 0) | (others_can_complete_ << 1)), s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -878074577>>, 481674261>::store(list_, s);
}

void todoList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "todoList");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (others_can_append_ << 0) | (others_can_complete_ << 1)));
    if (var0 & 1) { s.store_field("others_can_append", true); }
    if (var0 & 2) { s.store_field("others_can_complete", true); }
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    { s.store_vector_begin("list", list_.size()); for (const auto &_value : list_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_passwordSettings::account_passwordSettings()
  : flags_()
  , email_()
  , secure_settings_()
{}

const std::int32_t account_passwordSettings::ID;

object_ptr<account_passwordSettings> account_passwordSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<account_passwordSettings> res = make_tl_object<account_passwordSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->email_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->secure_settings_ = TlFetchBoxed<TlFetchObject<secureSecretSettings>, 354925740>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void account_passwordSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.passwordSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("email", email_); }
    if (var0 & 2) { s.store_object_field("secure_settings", static_cast<const BaseObject *>(secure_settings_.get())); }
    s.store_class_end();
  }
}

object_ptr<account_SavedRingtones> account_SavedRingtones::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_savedRingtonesNotModified::ID:
      return account_savedRingtonesNotModified::fetch(p);
    case account_savedRingtones::ID:
      return account_savedRingtones::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_savedRingtonesNotModified::ID;

object_ptr<account_SavedRingtones> account_savedRingtonesNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<account_savedRingtonesNotModified>();
}

void account_savedRingtonesNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.savedRingtonesNotModified");
    s.store_class_end();
  }
}

const std::int32_t account_savedRingtones::ID;

object_ptr<account_SavedRingtones> account_savedRingtones::fetch(TlBufferParser &p) {
  return make_tl_object<account_savedRingtones>(p);
}

account_savedRingtones::account_savedRingtones(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , ringtones_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void account_savedRingtones::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.savedRingtones");
    s.store_field("hash", hash_);
    { s.store_vector_begin("ringtones", ringtones_.size()); for (const auto &_value : ringtones_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t channels_sendAsPeers::ID;

object_ptr<channels_sendAsPeers> channels_sendAsPeers::fetch(TlBufferParser &p) {
  return make_tl_object<channels_sendAsPeers>(p);
}

channels_sendAsPeers::channels_sendAsPeers(TlBufferParser &p)
  : peers_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<sendAsPeer>, -1206095820>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void channels_sendAsPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.sendAsPeers");
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t help_configSimple::ID;

object_ptr<help_configSimple> help_configSimple::fetch(TlBufferParser &p) {
  return make_tl_object<help_configSimple>(p);
}

help_configSimple::help_configSimple(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
  , expires_(TlFetchInt::parse(p))
  , rules_(TlFetchVector<TlFetchBoxed<TlFetchObject<accessPointRule>, 1182381663>>::parse(p))
{}

void help_configSimple::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.configSimple");
    s.store_field("date", date_);
    s.store_field("expires", expires_);
    { s.store_vector_begin("rules", rules_.size()); for (const auto &_value : rules_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t help_inviteText::ID;

object_ptr<help_inviteText> help_inviteText::fetch(TlBufferParser &p) {
  return make_tl_object<help_inviteText>(p);
}

help_inviteText::help_inviteText(TlBufferParser &p)
  : message_(TlFetchString<string>::parse(p))
{}

void help_inviteText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.inviteText");
    s.store_field("message", message_);
    s.store_class_end();
  }
}

object_ptr<help_PeerColors> help_PeerColors::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_peerColorsNotModified::ID:
      return help_peerColorsNotModified::fetch(p);
    case help_peerColors::ID:
      return help_peerColors::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_peerColorsNotModified::ID;

object_ptr<help_PeerColors> help_peerColorsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<help_peerColorsNotModified>();
}

void help_peerColorsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.peerColorsNotModified");
    s.store_class_end();
  }
}

const std::int32_t help_peerColors::ID;

object_ptr<help_PeerColors> help_peerColors::fetch(TlBufferParser &p) {
  return make_tl_object<help_peerColors>(p);
}

help_peerColors::help_peerColors(TlBufferParser &p)
  : hash_(TlFetchInt::parse(p))
  , colors_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<help_peerColorOption>, -1377014082>>, 481674261>::parse(p))
{}

void help_peerColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.peerColors");
    s.store_field("hash", hash_);
    { s.store_vector_begin("colors", colors_.size()); for (const auto &_value : colors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t help_premiumPromo::ID;

object_ptr<help_premiumPromo> help_premiumPromo::fetch(TlBufferParser &p) {
  return make_tl_object<help_premiumPromo>(p);
}

help_premiumPromo::help_premiumPromo(TlBufferParser &p)
  : status_text_(TlFetchString<string>::parse(p))
  , status_entities_(TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p))
  , video_sections_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
  , videos_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
  , period_options_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<premiumSubscriptionOption>, 1596792306>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void help_premiumPromo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.premiumPromo");
    s.store_field("status_text", status_text_);
    { s.store_vector_begin("status_entities", status_entities_.size()); for (const auto &_value : status_entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("video_sections", video_sections_.size()); for (const auto &_value : video_sections_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("videos", videos_.size()); for (const auto &_value : videos_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("period_options", period_options_.size()); for (const auto &_value : period_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t help_supportName::ID;

object_ptr<help_supportName> help_supportName::fetch(TlBufferParser &p) {
  return make_tl_object<help_supportName>(p);
}

help_supportName::help_supportName(TlBufferParser &p)
  : name_(TlFetchString<string>::parse(p))
{}

void help_supportName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.supportName");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

object_ptr<messages_AllStickers> messages_AllStickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_allStickersNotModified::ID:
      return messages_allStickersNotModified::fetch(p);
    case messages_allStickers::ID:
      return messages_allStickers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_allStickersNotModified::ID;

object_ptr<messages_AllStickers> messages_allStickersNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_allStickersNotModified>();
}

void messages_allStickersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.allStickersNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_allStickers::ID;

object_ptr<messages_AllStickers> messages_allStickers::fetch(TlBufferParser &p) {
  return make_tl_object<messages_allStickers>(p);
}

messages_allStickers::messages_allStickers(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , sets_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>>, 481674261>::parse(p))
{}

void messages_allStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.allStickers");
    s.store_field("hash", hash_);
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_AvailableReactions> messages_AvailableReactions::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_availableReactionsNotModified::ID:
      return messages_availableReactionsNotModified::fetch(p);
    case messages_availableReactions::ID:
      return messages_availableReactions::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_availableReactionsNotModified::ID;

object_ptr<messages_AvailableReactions> messages_availableReactionsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_availableReactionsNotModified>();
}

void messages_availableReactionsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.availableReactionsNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_availableReactions::ID;

object_ptr<messages_AvailableReactions> messages_availableReactions::fetch(TlBufferParser &p) {
  return make_tl_object<messages_availableReactions>(p);
}

messages_availableReactions::messages_availableReactions(TlBufferParser &p)
  : hash_(TlFetchInt::parse(p))
  , reactions_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<availableReaction>, -1065882623>>, 481674261>::parse(p))
{}

void messages_availableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.availableReactions");
    s.store_field("hash", hash_);
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_Dialogs> messages_Dialogs::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_dialogs::ID:
      return messages_dialogs::fetch(p);
    case messages_dialogsSlice::ID:
      return messages_dialogsSlice::fetch(p);
    case messages_dialogsNotModified::ID:
      return messages_dialogsNotModified::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_dialogs::ID;

object_ptr<messages_Dialogs> messages_dialogs::fetch(TlBufferParser &p) {
  return make_tl_object<messages_dialogs>(p);
}

messages_dialogs::messages_dialogs(TlBufferParser &p)
  : dialogs_(TlFetchBoxed<TlFetchVector<TlFetchObject<Dialog>>, 481674261>::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_dialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.dialogs");
    { s.store_vector_begin("dialogs", dialogs_.size()); for (const auto &_value : dialogs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_dialogsSlice::ID;

object_ptr<messages_Dialogs> messages_dialogsSlice::fetch(TlBufferParser &p) {
  return make_tl_object<messages_dialogsSlice>(p);
}

messages_dialogsSlice::messages_dialogsSlice(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , dialogs_(TlFetchBoxed<TlFetchVector<TlFetchObject<Dialog>>, 481674261>::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_dialogsSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.dialogsSlice");
    s.store_field("count", count_);
    { s.store_vector_begin("dialogs", dialogs_.size()); for (const auto &_value : dialogs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_dialogsNotModified::ID;

object_ptr<messages_Dialogs> messages_dialogsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_dialogsNotModified>(p);
}

messages_dialogsNotModified::messages_dialogsNotModified(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
{}

void messages_dialogsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.dialogsNotModified");
    s.store_field("count", count_);
    s.store_class_end();
  }
}

const std::int32_t messages_exportedChatInvites::ID;

object_ptr<messages_exportedChatInvites> messages_exportedChatInvites::fetch(TlBufferParser &p) {
  return make_tl_object<messages_exportedChatInvites>(p);
}

messages_exportedChatInvites::messages_exportedChatInvites(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , invites_(TlFetchBoxed<TlFetchVector<TlFetchObject<ExportedChatInvite>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_exportedChatInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.exportedChatInvites");
    s.store_field("count", count_);
    { s.store_vector_begin("invites", invites_.size()); for (const auto &_value : invites_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_peerSettings::ID;

object_ptr<messages_peerSettings> messages_peerSettings::fetch(TlBufferParser &p) {
  return make_tl_object<messages_peerSettings>(p);
}

messages_peerSettings::messages_peerSettings(TlBufferParser &p)
  : settings_(TlFetchBoxed<TlFetchObject<peerSettings>, -193510921>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_peerSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.peerSettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_SentEncryptedMessage> messages_SentEncryptedMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_sentEncryptedMessage::ID:
      return messages_sentEncryptedMessage::fetch(p);
    case messages_sentEncryptedFile::ID:
      return messages_sentEncryptedFile::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_sentEncryptedMessage::ID;

object_ptr<messages_SentEncryptedMessage> messages_sentEncryptedMessage::fetch(TlBufferParser &p) {
  return make_tl_object<messages_sentEncryptedMessage>(p);
}

messages_sentEncryptedMessage::messages_sentEncryptedMessage(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
{}

void messages_sentEncryptedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sentEncryptedMessage");
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t messages_sentEncryptedFile::ID;

object_ptr<messages_SentEncryptedMessage> messages_sentEncryptedFile::fetch(TlBufferParser &p) {
  return make_tl_object<messages_sentEncryptedFile>(p);
}

messages_sentEncryptedFile::messages_sentEncryptedFile(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
  , file_(TlFetchObject<EncryptedFile>::parse(p))
{}

void messages_sentEncryptedFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sentEncryptedFile");
    s.store_field("date", date_);
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_class_end();
  }
}

messages_transcribedAudio::messages_transcribedAudio()
  : flags_()
  , pending_()
  , transcription_id_()
  , text_()
  , trial_remains_num_()
  , trial_remains_until_date_()
{}

const std::int32_t messages_transcribedAudio::ID;

object_ptr<messages_transcribedAudio> messages_transcribedAudio::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_transcribedAudio> res = make_tl_object<messages_transcribedAudio>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pending_ = (var0 & 1) != 0;
  res->transcription_id_ = TlFetchLong::parse(p);
  res->text_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->trial_remains_num_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->trial_remains_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_transcribedAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.transcribedAudio");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pending_ << 0)));
    if (var0 & 1) { s.store_field("pending", true); }
    s.store_field("transcription_id", transcription_id_);
    s.store_field("text", text_);
    if (var0 & 2) { s.store_field("trial_remains_num", trial_remains_num_); }
    if (var0 & 2) { s.store_field("trial_remains_until_date", trial_remains_until_date_); }
    s.store_class_end();
  }
}

payments_checkedGiftCode::payments_checkedGiftCode()
  : flags_()
  , via_giveaway_()
  , from_id_()
  , giveaway_msg_id_()
  , to_id_()
  , date_()
  , months_()
  , used_date_()
  , chats_()
  , users_()
{}

const std::int32_t payments_checkedGiftCode::ID;

object_ptr<payments_checkedGiftCode> payments_checkedGiftCode::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_checkedGiftCode> res = make_tl_object<payments_checkedGiftCode>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->via_giveaway_ = (var0 & 4) != 0;
  if (var0 & 16) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 8) { res->giveaway_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->to_id_ = TlFetchLong::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  res->months_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->used_date_ = TlFetchInt::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_checkedGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.checkedGiftCode");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (via_giveaway_ << 2)));
    if (var0 & 4) { s.store_field("via_giveaway", true); }
    if (var0 & 16) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    if (var0 & 8) { s.store_field("giveaway_msg_id", giveaway_msg_id_); }
    if (var0 & 1) { s.store_field("to_id", to_id_); }
    s.store_field("date", date_);
    s.store_field("months", months_);
    if (var0 & 2) { s.store_field("used_date", used_date_); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t payments_exportedInvoice::ID;

object_ptr<payments_exportedInvoice> payments_exportedInvoice::fetch(TlBufferParser &p) {
  return make_tl_object<payments_exportedInvoice>(p);
}

payments_exportedInvoice::payments_exportedInvoice(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void payments_exportedInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.exportedInvoice");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

payments_savedStarGifts::payments_savedStarGifts()
  : flags_()
  , count_()
  , chat_notifications_enabled_()
  , gifts_()
  , next_offset_()
  , chats_()
  , users_()
{}

const std::int32_t payments_savedStarGifts::ID;

object_ptr<payments_savedStarGifts> payments_savedStarGifts::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_savedStarGifts> res = make_tl_object<payments_savedStarGifts>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->chat_notifications_enabled_ = TlFetchBool::parse(p); }
  res->gifts_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<savedStarGift>, -1987861422>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_savedStarGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.savedStarGifts");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    if (var0 & 2) { s.store_field("chat_notifications_enabled", chat_notifications_enabled_); }
    { s.store_vector_begin("gifts", gifts_.size()); for (const auto &_value : gifts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t payments_starGiftUpgradePreview::ID;

object_ptr<payments_starGiftUpgradePreview> payments_starGiftUpgradePreview::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starGiftUpgradePreview>(p);
}

payments_starGiftUpgradePreview::payments_starGiftUpgradePreview(TlBufferParser &p)
  : sample_attributes_(TlFetchBoxed<TlFetchVector<TlFetchObject<StarGiftAttribute>>, 481674261>::parse(p))
  , prices_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starGiftUpgradePrice>, -1712704739>>, 481674261>::parse(p))
  , next_prices_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starGiftUpgradePrice>, -1712704739>>, 481674261>::parse(p))
{}

void payments_starGiftUpgradePreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starGiftUpgradePreview");
    { s.store_vector_begin("sample_attributes", sample_attributes_.size()); for (const auto &_value : sample_attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("prices", prices_.size()); for (const auto &_value : prices_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("next_prices", next_prices_.size()); for (const auto &_value : next_prices_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_starsRevenueStats::payments_starsRevenueStats()
  : flags_()
  , top_hours_graph_()
  , revenue_graph_()
  , status_()
  , usd_rate_()
{}

const std::int32_t payments_starsRevenueStats::ID;

object_ptr<payments_starsRevenueStats> payments_starsRevenueStats::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_starsRevenueStats> res = make_tl_object<payments_starsRevenueStats>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->top_hours_graph_ = TlFetchObject<StatsGraph>::parse(p); }
  res->revenue_graph_ = TlFetchObject<StatsGraph>::parse(p);
  res->status_ = TlFetchBoxed<TlFetchObject<starsRevenueStatus>, -21080943>::parse(p);
  res->usd_rate_ = TlFetchDouble::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_starsRevenueStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starsRevenueStats");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("top_hours_graph", static_cast<const BaseObject *>(top_hours_graph_.get())); }
    s.store_object_field("revenue_graph", static_cast<const BaseObject *>(revenue_graph_.get()));
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_field("usd_rate", usd_rate_);
    s.store_class_end();
  }
}

const std::int32_t phone_exportedGroupCallInvite::ID;

object_ptr<phone_exportedGroupCallInvite> phone_exportedGroupCallInvite::fetch(TlBufferParser &p) {
  return make_tl_object<phone_exportedGroupCallInvite>(p);
}

phone_exportedGroupCallInvite::phone_exportedGroupCallInvite(TlBufferParser &p)
  : link_(TlFetchString<string>::parse(p))
{}

void phone_exportedGroupCallInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.exportedGroupCallInvite");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

const std::int32_t phone_joinAsPeers::ID;

object_ptr<phone_joinAsPeers> phone_joinAsPeers::fetch(TlBufferParser &p) {
  return make_tl_object<phone_joinAsPeers>(p);
}

phone_joinAsPeers::phone_joinAsPeers(TlBufferParser &p)
  : peers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void phone_joinAsPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.joinAsPeers");
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

premium_boostsStatus::premium_boostsStatus()
  : flags_()
  , my_boost_()
  , level_()
  , current_level_boosts_()
  , boosts_()
  , gift_boosts_()
  , next_level_boosts_()
  , premium_audience_()
  , boost_url_()
  , prepaid_giveaways_()
  , my_boost_slots_()
{}

const std::int32_t premium_boostsStatus::ID;

object_ptr<premium_boostsStatus> premium_boostsStatus::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<premium_boostsStatus> res = make_tl_object<premium_boostsStatus>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->my_boost_ = (var0 & 4) != 0;
  res->level_ = TlFetchInt::parse(p);
  res->current_level_boosts_ = TlFetchInt::parse(p);
  res->boosts_ = TlFetchInt::parse(p);
  if (var0 & 16) { res->gift_boosts_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->next_level_boosts_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->premium_audience_ = TlFetchBoxed<TlFetchObject<statsPercentValue>, -875679776>::parse(p); }
  res->boost_url_ = TlFetchString<string>::parse(p);
  if (var0 & 8) { res->prepaid_giveaways_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PrepaidGiveaway>>, 481674261>::parse(p); }
  if (var0 & 4) { res->my_boost_slots_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void premium_boostsStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.boostsStatus");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (my_boost_ << 2)));
    if (var0 & 4) { s.store_field("my_boost", true); }
    s.store_field("level", level_);
    s.store_field("current_level_boosts", current_level_boosts_);
    s.store_field("boosts", boosts_);
    if (var0 & 16) { s.store_field("gift_boosts", gift_boosts_); }
    if (var0 & 1) { s.store_field("next_level_boosts", next_level_boosts_); }
    if (var0 & 2) { s.store_object_field("premium_audience", static_cast<const BaseObject *>(premium_audience_.get())); }
    s.store_field("boost_url", boost_url_);
    if (var0 & 8) { { s.store_vector_begin("prepaid_giveaways", prepaid_giveaways_.size()); for (const auto &_value : prepaid_giveaways_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { { s.store_vector_begin("my_boost_slots", my_boost_slots_.size()); for (const auto &_value : my_boost_slots_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

const std::int32_t smsjobs_eligibleToJoin::ID;

object_ptr<smsjobs_eligibleToJoin> smsjobs_eligibleToJoin::fetch(TlBufferParser &p) {
  return make_tl_object<smsjobs_eligibleToJoin>(p);
}

smsjobs_eligibleToJoin::smsjobs_eligibleToJoin(TlBufferParser &p)
  : terms_url_(TlFetchString<string>::parse(p))
  , monthly_sent_sms_(TlFetchInt::parse(p))
{}

void smsjobs_eligibleToJoin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.eligibleToJoin");
    s.store_field("terms_url", terms_url_);
    s.store_field("monthly_sent_sms", monthly_sent_sms_);
    s.store_class_end();
  }
}

const std::int32_t stats_broadcastStats::ID;

object_ptr<stats_broadcastStats> stats_broadcastStats::fetch(TlBufferParser &p) {
  return make_tl_object<stats_broadcastStats>(p);
}

stats_broadcastStats::stats_broadcastStats(TlBufferParser &p)
  : period_(TlFetchBoxed<TlFetchObject<statsDateRangeDays>, -1237848657>::parse(p))
  , followers_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , views_per_post_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , shares_per_post_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , reactions_per_post_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , views_per_story_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , shares_per_story_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , reactions_per_story_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , enabled_notifications_(TlFetchBoxed<TlFetchObject<statsPercentValue>, -875679776>::parse(p))
  , growth_graph_(TlFetchObject<StatsGraph>::parse(p))
  , followers_graph_(TlFetchObject<StatsGraph>::parse(p))
  , mute_graph_(TlFetchObject<StatsGraph>::parse(p))
  , top_hours_graph_(TlFetchObject<StatsGraph>::parse(p))
  , interactions_graph_(TlFetchObject<StatsGraph>::parse(p))
  , iv_interactions_graph_(TlFetchObject<StatsGraph>::parse(p))
  , views_by_source_graph_(TlFetchObject<StatsGraph>::parse(p))
  , new_followers_by_source_graph_(TlFetchObject<StatsGraph>::parse(p))
  , languages_graph_(TlFetchObject<StatsGraph>::parse(p))
  , reactions_by_emotion_graph_(TlFetchObject<StatsGraph>::parse(p))
  , story_interactions_graph_(TlFetchObject<StatsGraph>::parse(p))
  , story_reactions_by_emotion_graph_(TlFetchObject<StatsGraph>::parse(p))
  , recent_posts_interactions_(TlFetchBoxed<TlFetchVector<TlFetchObject<PostInteractionCounters>>, 481674261>::parse(p))
{}

void stats_broadcastStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.broadcastStats");
    s.store_object_field("period", static_cast<const BaseObject *>(period_.get()));
    s.store_object_field("followers", static_cast<const BaseObject *>(followers_.get()));
    s.store_object_field("views_per_post", static_cast<const BaseObject *>(views_per_post_.get()));
    s.store_object_field("shares_per_post", static_cast<const BaseObject *>(shares_per_post_.get()));
    s.store_object_field("reactions_per_post", static_cast<const BaseObject *>(reactions_per_post_.get()));
    s.store_object_field("views_per_story", static_cast<const BaseObject *>(views_per_story_.get()));
    s.store_object_field("shares_per_story", static_cast<const BaseObject *>(shares_per_story_.get()));
    s.store_object_field("reactions_per_story", static_cast<const BaseObject *>(reactions_per_story_.get()));
    s.store_object_field("enabled_notifications", static_cast<const BaseObject *>(enabled_notifications_.get()));
    s.store_object_field("growth_graph", static_cast<const BaseObject *>(growth_graph_.get()));
    s.store_object_field("followers_graph", static_cast<const BaseObject *>(followers_graph_.get()));
    s.store_object_field("mute_graph", static_cast<const BaseObject *>(mute_graph_.get()));
    s.store_object_field("top_hours_graph", static_cast<const BaseObject *>(top_hours_graph_.get()));
    s.store_object_field("interactions_graph", static_cast<const BaseObject *>(interactions_graph_.get()));
    s.store_object_field("iv_interactions_graph", static_cast<const BaseObject *>(iv_interactions_graph_.get()));
    s.store_object_field("views_by_source_graph", static_cast<const BaseObject *>(views_by_source_graph_.get()));
    s.store_object_field("new_followers_by_source_graph", static_cast<const BaseObject *>(new_followers_by_source_graph_.get()));
    s.store_object_field("languages_graph", static_cast<const BaseObject *>(languages_graph_.get()));
    s.store_object_field("reactions_by_emotion_graph", static_cast<const BaseObject *>(reactions_by_emotion_graph_.get()));
    s.store_object_field("story_interactions_graph", static_cast<const BaseObject *>(story_interactions_graph_.get()));
    s.store_object_field("story_reactions_by_emotion_graph", static_cast<const BaseObject *>(story_reactions_by_emotion_graph_.get()));
    { s.store_vector_begin("recent_posts_interactions", recent_posts_interactions_.size()); for (const auto &_value : recent_posts_interactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<stories_Albums> stories_Albums::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case stories_albumsNotModified::ID:
      return stories_albumsNotModified::fetch(p);
    case stories_albums::ID:
      return stories_albums::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t stories_albumsNotModified::ID;

object_ptr<stories_Albums> stories_albumsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<stories_albumsNotModified>();
}

void stories_albumsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.albumsNotModified");
    s.store_class_end();
  }
}

const std::int32_t stories_albums::ID;

object_ptr<stories_Albums> stories_albums::fetch(TlBufferParser &p) {
  return make_tl_object<stories_albums>(p);
}

stories_albums::stories_albums(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , albums_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<storyAlbum>, -1826262950>>, 481674261>::parse(p))
{}

void stories_albums::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.albums");
    s.store_field("hash", hash_);
    { s.store_vector_begin("albums", albums_.size()); for (const auto &_value : albums_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t stories_canSendStoryCount::ID;

object_ptr<stories_canSendStoryCount> stories_canSendStoryCount::fetch(TlBufferParser &p) {
  return make_tl_object<stories_canSendStoryCount>(p);
}

stories_canSendStoryCount::stories_canSendStoryCount(TlBufferParser &p)
  : count_remains_(TlFetchInt::parse(p))
{}

void stories_canSendStoryCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.canSendStoryCount");
    s.store_field("count_remains", count_remains_);
    s.store_class_end();
  }
}

object_ptr<updates_Difference> updates_Difference::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case updates_differenceEmpty::ID:
      return updates_differenceEmpty::fetch(p);
    case updates_difference::ID:
      return updates_difference::fetch(p);
    case updates_differenceSlice::ID:
      return updates_differenceSlice::fetch(p);
    case updates_differenceTooLong::ID:
      return updates_differenceTooLong::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t updates_differenceEmpty::ID;

object_ptr<updates_Difference> updates_differenceEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<updates_differenceEmpty>(p);
}

updates_differenceEmpty::updates_differenceEmpty(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
  , seq_(TlFetchInt::parse(p))
{}

void updates_differenceEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.differenceEmpty");
    s.store_field("date", date_);
    s.store_field("seq", seq_);
    s.store_class_end();
  }
}

const std::int32_t updates_difference::ID;

object_ptr<updates_Difference> updates_difference::fetch(TlBufferParser &p) {
  return make_tl_object<updates_difference>(p);
}

updates_difference::updates_difference(TlBufferParser &p)
  : new_messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , new_encrypted_messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<EncryptedMessage>>, 481674261>::parse(p))
  , other_updates_(TlFetchBoxed<TlFetchVector<TlFetchObject<Update>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
  , state_(TlFetchBoxed<TlFetchObject<updates_state>, -1519637954>::parse(p))
{}

void updates_difference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.difference");
    { s.store_vector_begin("new_messages", new_messages_.size()); for (const auto &_value : new_messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("new_encrypted_messages", new_encrypted_messages_.size()); for (const auto &_value : new_encrypted_messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("other_updates", other_updates_.size()); for (const auto &_value : other_updates_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    s.store_class_end();
  }
}

updates_differenceSlice::updates_differenceSlice(array<object_ptr<Message>> &&new_messages_, array<object_ptr<EncryptedMessage>> &&new_encrypted_messages_, array<object_ptr<Update>> &&other_updates_, array<object_ptr<Chat>> &&chats_, array<object_ptr<User>> &&users_, object_ptr<updates_state> &&intermediate_state_)
  : new_messages_(std::move(new_messages_))
  , new_encrypted_messages_(std::move(new_encrypted_messages_))
  , other_updates_(std::move(other_updates_))
  , chats_(std::move(chats_))
  , users_(std::move(users_))
  , intermediate_state_(std::move(intermediate_state_))
{}

const std::int32_t updates_differenceSlice::ID;

object_ptr<updates_Difference> updates_differenceSlice::fetch(TlBufferParser &p) {
  return make_tl_object<updates_differenceSlice>(p);
}

updates_differenceSlice::updates_differenceSlice(TlBufferParser &p)
  : new_messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , new_encrypted_messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<EncryptedMessage>>, 481674261>::parse(p))
  , other_updates_(TlFetchBoxed<TlFetchVector<TlFetchObject<Update>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
  , intermediate_state_(TlFetchBoxed<TlFetchObject<updates_state>, -1519637954>::parse(p))
{}

void updates_differenceSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.differenceSlice");
    { s.store_vector_begin("new_messages", new_messages_.size()); for (const auto &_value : new_messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("new_encrypted_messages", new_encrypted_messages_.size()); for (const auto &_value : new_encrypted_messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("other_updates", other_updates_.size()); for (const auto &_value : other_updates_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("intermediate_state", static_cast<const BaseObject *>(intermediate_state_.get()));
    s.store_class_end();
  }
}

const std::int32_t updates_differenceTooLong::ID;

object_ptr<updates_Difference> updates_differenceTooLong::fetch(TlBufferParser &p) {
  return make_tl_object<updates_differenceTooLong>(p);
}

updates_differenceTooLong::updates_differenceTooLong(TlBufferParser &p)
  : pts_(TlFetchInt::parse(p))
{}

void updates_differenceTooLong::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.differenceTooLong");
    s.store_field("pts", pts_);
    s.store_class_end();
  }
}

object_ptr<upload_File> upload_File::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case upload_file::ID:
      return upload_file::fetch(p);
    case upload_fileCdnRedirect::ID:
      return upload_fileCdnRedirect::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t upload_file::ID;

object_ptr<upload_File> upload_file::fetch(TlBufferParser &p) {
  return make_tl_object<upload_file>(p);
}

upload_file::upload_file(TlBufferParser &p)
  : type_(TlFetchObject<storage_FileType>::parse(p))
  , mtime_(TlFetchInt::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
{}

void upload_file::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.file");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("mtime", mtime_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

const std::int32_t upload_fileCdnRedirect::ID;

object_ptr<upload_File> upload_fileCdnRedirect::fetch(TlBufferParser &p) {
  return make_tl_object<upload_fileCdnRedirect>(p);
}

upload_fileCdnRedirect::upload_fileCdnRedirect(TlBufferParser &p)
  : dc_id_(TlFetchInt::parse(p))
  , file_token_(TlFetchBytes<bytes>::parse(p))
  , encryption_key_(TlFetchBytes<bytes>::parse(p))
  , encryption_iv_(TlFetchBytes<bytes>::parse(p))
  , file_hashes_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<fileHash>, -207944868>>, 481674261>::parse(p))
{}

void upload_fileCdnRedirect::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.fileCdnRedirect");
    s.store_field("dc_id", dc_id_);
    s.store_bytes_field("file_token", file_token_);
    s.store_bytes_field("encryption_key", encryption_key_);
    s.store_bytes_field("encryption_iv", encryption_iv_);
    { s.store_vector_begin("file_hashes", file_hashes_.size()); for (const auto &_value : file_hashes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_changePhone::account_changePhone(string const &phone_number_, string const &phone_code_hash_, string const &phone_code_)
  : phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , phone_code_(phone_code_)
{}

const std::int32_t account_changePhone::ID;

void account_changePhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1891839707);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(phone_code_, s);
}

void account_changePhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1891839707);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(phone_code_, s);
}

void account_changePhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.changePhone");
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("phone_code", phone_code_);
    s.store_class_end();
  }
}

account_changePhone::ReturnType account_changePhone::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<User>::parse(p);
#undef FAIL
}

account_editBusinessChatLink::account_editBusinessChatLink(string const &slug_, object_ptr<inputBusinessChatLink> &&link_)
  : slug_(slug_)
  , link_(std::move(link_))
{}

const std::int32_t account_editBusinessChatLink::ID;

void account_editBusinessChatLink::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1942744913);
  TlStoreString::store(slug_, s);
  TlStoreBoxed<TlStoreObject, 292003751>::store(link_, s);
}

void account_editBusinessChatLink::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1942744913);
  TlStoreString::store(slug_, s);
  TlStoreBoxed<TlStoreObject, 292003751>::store(link_, s);
}

void account_editBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.editBusinessChatLink");
    s.store_field("slug", slug_);
    s.store_object_field("link", static_cast<const BaseObject *>(link_.get()));
    s.store_class_end();
  }
}

account_editBusinessChatLink::ReturnType account_editBusinessChatLink::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<businessChatLink>, -1263638929>::parse(p);
#undef FAIL
}

const std::int32_t account_getAuthorizations::ID;

void account_getAuthorizations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-484392616);
}

void account_getAuthorizations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-484392616);
}

void account_getAuthorizations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getAuthorizations");
    s.store_class_end();
  }
}

account_getAuthorizations::ReturnType account_getAuthorizations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_authorizations>, 1275039392>::parse(p);
#undef FAIL
}

account_getCollectibleEmojiStatuses::account_getCollectibleEmojiStatuses(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getCollectibleEmojiStatuses::ID;

void account_getCollectibleEmojiStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(779830595);
  TlStoreBinary::store(hash_, s);
}

void account_getCollectibleEmojiStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(779830595);
  TlStoreBinary::store(hash_, s);
}

void account_getCollectibleEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getCollectibleEmojiStatuses");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getCollectibleEmojiStatuses::ReturnType account_getCollectibleEmojiStatuses::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_EmojiStatuses>::parse(p);
#undef FAIL
}

const std::int32_t account_getContentSettings::ID;

void account_getContentSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1952756306);
}

void account_getContentSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1952756306);
}

void account_getContentSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getContentSettings");
    s.store_class_end();
  }
}

account_getContentSettings::ReturnType account_getContentSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_contentSettings>, 1474462241>::parse(p);
#undef FAIL
}

account_getMultiWallPapers::account_getMultiWallPapers(array<object_ptr<InputWallPaper>> &&wallpapers_)
  : wallpapers_(std::move(wallpapers_))
{}

const std::int32_t account_getMultiWallPapers::ID;

void account_getMultiWallPapers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1705865692);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(wallpapers_, s);
}

void account_getMultiWallPapers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1705865692);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(wallpapers_, s);
}

void account_getMultiWallPapers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getMultiWallPapers");
    { s.store_vector_begin("wallpapers", wallpapers_.size()); for (const auto &_value : wallpapers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_getMultiWallPapers::ReturnType account_getMultiWallPapers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<WallPaper>>, 481674261>::parse(p);
#undef FAIL
}

account_getPasswordSettings::account_getPasswordSettings(object_ptr<InputCheckPasswordSRP> &&password_)
  : password_(std::move(password_))
{}

const std::int32_t account_getPasswordSettings::ID;

void account_getPasswordSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1663767815);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void account_getPasswordSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1663767815);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void account_getPasswordSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getPasswordSettings");
    s.store_object_field("password", static_cast<const BaseObject *>(password_.get()));
    s.store_class_end();
  }
}

account_getPasswordSettings::ReturnType account_getPasswordSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_passwordSettings>, -1705233435>::parse(p);
#undef FAIL
}

account_getPrivacy::account_getPrivacy(object_ptr<InputPrivacyKey> &&key_)
  : key_(std::move(key_))
{}

const std::int32_t account_getPrivacy::ID;

void account_getPrivacy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-623130288);
  TlStoreBoxedUnknown<TlStoreObject>::store(key_, s);
}

void account_getPrivacy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-623130288);
  TlStoreBoxedUnknown<TlStoreObject>::store(key_, s);
}

void account_getPrivacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getPrivacy");
    s.store_object_field("key", static_cast<const BaseObject *>(key_.get()));
    s.store_class_end();
  }
}

account_getPrivacy::ReturnType account_getPrivacy::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_privacyRules>, 1352683077>::parse(p);
#undef FAIL
}

account_getSavedMusicIds::account_getSavedMusicIds(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getSavedMusicIds::ID;

void account_getSavedMusicIds::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-526557265);
  TlStoreBinary::store(hash_, s);
}

void account_getSavedMusicIds::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-526557265);
  TlStoreBinary::store(hash_, s);
}

void account_getSavedMusicIds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getSavedMusicIds");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getSavedMusicIds::ReturnType account_getSavedMusicIds::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_SavedMusicIds>::parse(p);
#undef FAIL
}

account_reorderUsernames::account_reorderUsernames(array<string> &&order_)
  : order_(std::move(order_))
{}

const std::int32_t account_reorderUsernames::ID;

void account_reorderUsernames::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-279966037);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(order_, s);
}

void account_reorderUsernames::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-279966037);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(order_, s);
}

void account_reorderUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.reorderUsernames");
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_reorderUsernames::ReturnType account_reorderUsernames::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_saveAutoSaveSettings::account_saveAutoSaveSettings(int32 flags_, bool users_, bool chats_, bool broadcasts_, object_ptr<InputPeer> &&peer_, object_ptr<autoSaveSettings> &&settings_)
  : flags_(flags_)
  , users_(users_)
  , chats_(chats_)
  , broadcasts_(broadcasts_)
  , peer_(std::move(peer_))
  , settings_(std::move(settings_))
{}

const std::int32_t account_saveAutoSaveSettings::ID;

void account_saveAutoSaveSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-694451359);
  TlStoreBinary::store((var0 = flags_ | (users_ << 0) | (chats_ << 1) | (broadcasts_ << 2)), s);
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  TlStoreBoxed<TlStoreObject, -934791986>::store(settings_, s);
}

void account_saveAutoSaveSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-694451359);
  TlStoreBinary::store((var0 = flags_ | (users_ << 0) | (chats_ << 1) | (broadcasts_ << 2)), s);
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  TlStoreBoxed<TlStoreObject, -934791986>::store(settings_, s);
}

void account_saveAutoSaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.saveAutoSaveSettings");
    s.store_field("flags", (var0 = flags_ | (users_ << 0) | (chats_ << 1) | (broadcasts_ << 2)));
    if (var0 & 1) { s.store_field("users", true); }
    if (var0 & 2) { s.store_field("chats", true); }
    if (var0 & 4) { s.store_field("broadcasts", true); }
    if (var0 & 8) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_saveAutoSaveSettings::ReturnType account_saveAutoSaveSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_unregisterDevice::account_unregisterDevice(int32 token_type_, string const &token_, array<int64> &&other_uids_)
  : token_type_(token_type_)
  , token_(token_)
  , other_uids_(std::move(other_uids_))
{}

const std::int32_t account_unregisterDevice::ID;

void account_unregisterDevice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1779249670);
  TlStoreBinary::store(token_type_, s);
  TlStoreString::store(token_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(other_uids_, s);
}

void account_unregisterDevice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1779249670);
  TlStoreBinary::store(token_type_, s);
  TlStoreString::store(token_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(other_uids_, s);
}

void account_unregisterDevice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.unregisterDevice");
    s.store_field("token_type", token_type_);
    s.store_field("token", token_);
    { s.store_vector_begin("other_uids", other_uids_.size()); for (const auto &_value : other_uids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_unregisterDevice::ReturnType account_unregisterDevice::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateBirthday::account_updateBirthday(int32 flags_, object_ptr<birthday> &&birthday_)
  : flags_(flags_)
  , birthday_(std::move(birthday_))
{}

const std::int32_t account_updateBirthday::ID;

void account_updateBirthday::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-865203183);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 1821253126>::store(birthday_, s); }
}

void account_updateBirthday::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-865203183);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 1821253126>::store(birthday_, s); }
}

void account_updateBirthday::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateBirthday");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("birthday", static_cast<const BaseObject *>(birthday_.get())); }
    s.store_class_end();
  }
}

account_updateBirthday::ReturnType account_updateBirthday::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateBusinessAwayMessage::account_updateBusinessAwayMessage(int32 flags_, object_ptr<inputBusinessAwayMessage> &&message_)
  : flags_(flags_)
  , message_(std::move(message_))
{}

const std::int32_t account_updateBusinessAwayMessage::ID;

void account_updateBusinessAwayMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1570078811);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -2094959136>::store(message_, s); }
}

void account_updateBusinessAwayMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1570078811);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -2094959136>::store(message_, s); }
}

void account_updateBusinessAwayMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateBusinessAwayMessage");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

account_updateBusinessAwayMessage::ReturnType account_updateBusinessAwayMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateColor::account_updateColor(int32 flags_, bool for_profile_, object_ptr<PeerColor> &&color_)
  : flags_(flags_)
  , for_profile_(for_profile_)
  , color_(std::move(color_))
{}

const std::int32_t account_updateColor::ID;

void account_updateColor::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1749885262);
  TlStoreBinary::store((var0 = flags_ | (for_profile_ << 1)), s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(color_, s); }
}

void account_updateColor::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1749885262);
  TlStoreBinary::store((var0 = flags_ | (for_profile_ << 1)), s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(color_, s); }
}

void account_updateColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateColor");
    s.store_field("flags", (var0 = flags_ | (for_profile_ << 1)));
    if (var0 & 2) { s.store_field("for_profile", true); }
    if (var0 & 4) { s.store_object_field("color", static_cast<const BaseObject *>(color_.get())); }
    s.store_class_end();
  }
}

account_updateColor::ReturnType account_updateColor::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateTheme::account_updateTheme(int32 flags_, string const &format_, object_ptr<InputTheme> &&theme_, string const &slug_, string const &title_, object_ptr<InputDocument> &&document_, array<object_ptr<inputThemeSettings>> &&settings_)
  : flags_(flags_)
  , format_(format_)
  , theme_(std::move(theme_))
  , slug_(slug_)
  , title_(title_)
  , document_(std::move(document_))
  , settings_(std::move(settings_))
{}

const std::int32_t account_updateTheme::ID;

void account_updateTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(737414348);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(format_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
  if (var0 & 1) { TlStoreString::store(slug_, s); }
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(document_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1881255857>>, 481674261>::store(settings_, s); }
}

void account_updateTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(737414348);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(format_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
  if (var0 & 1) { TlStoreString::store(slug_, s); }
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(document_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1881255857>>, 481674261>::store(settings_, s); }
}

void account_updateTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateTheme");
    s.store_field("flags", (var0 = flags_));
    s.store_field("format", format_);
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    if (var0 & 1) { s.store_field("slug", slug_); }
    if (var0 & 2) { s.store_field("title", title_); }
    if (var0 & 4) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 8) { { s.store_vector_begin("settings", settings_.size()); for (const auto &_value : settings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

account_updateTheme::ReturnType account_updateTheme::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<theme>, -1609668650>::parse(p);
#undef FAIL
}

auth_reportMissingCode::auth_reportMissingCode(string const &phone_number_, string const &phone_code_hash_, string const &mnc_)
  : phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , mnc_(mnc_)
{}

const std::int32_t auth_reportMissingCode::ID;

void auth_reportMissingCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-878841866);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(mnc_, s);
}

void auth_reportMissingCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-878841866);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(mnc_, s);
}

void auth_reportMissingCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.reportMissingCode");
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("mnc", mnc_);
    s.store_class_end();
  }
}

auth_reportMissingCode::ReturnType auth_reportMissingCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_sendCode::auth_sendCode(string const &phone_number_, int32 api_id_, string const &api_hash_, object_ptr<codeSettings> &&settings_)
  : phone_number_(phone_number_)
  , api_id_(api_id_)
  , api_hash_(api_hash_)
  , settings_(std::move(settings_))
{}

const std::int32_t auth_sendCode::ID;

void auth_sendCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1502141361);
  TlStoreString::store(phone_number_, s);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void auth_sendCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1502141361);
  TlStoreString::store(phone_number_, s);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void auth_sendCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sendCode");
    s.store_field("phone_number", phone_number_);
    s.store_field("api_id", api_id_);
    s.store_field("api_hash", api_hash_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

auth_sendCode::ReturnType auth_sendCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_SentCode>::parse(p);
#undef FAIL
}

bots_getBotCommands::bots_getBotCommands(object_ptr<BotCommandScope> &&scope_, string const &lang_code_)
  : scope_(std::move(scope_))
  , lang_code_(lang_code_)
{}

const std::int32_t bots_getBotCommands::ID;

void bots_getBotCommands::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-481554986);
  TlStoreBoxedUnknown<TlStoreObject>::store(scope_, s);
  TlStoreString::store(lang_code_, s);
}

void bots_getBotCommands::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-481554986);
  TlStoreBoxedUnknown<TlStoreObject>::store(scope_, s);
  TlStoreString::store(lang_code_, s);
}

void bots_getBotCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getBotCommands");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

bots_getBotCommands::ReturnType bots_getBotCommands::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<botCommand>, -1032140601>>, 481674261>::parse(p);
#undef FAIL
}

bots_setBotGroupDefaultAdminRights::bots_setBotGroupDefaultAdminRights(object_ptr<chatAdminRights> &&admin_rights_)
  : admin_rights_(std::move(admin_rights_))
{}

const std::int32_t bots_setBotGroupDefaultAdminRights::ID;

void bots_setBotGroupDefaultAdminRights::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1839281686);
  TlStoreBoxed<TlStoreObject, 1605510357>::store(admin_rights_, s);
}

void bots_setBotGroupDefaultAdminRights::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1839281686);
  TlStoreBoxed<TlStoreObject, 1605510357>::store(admin_rights_, s);
}

void bots_setBotGroupDefaultAdminRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.setBotGroupDefaultAdminRights");
    s.store_object_field("admin_rights", static_cast<const BaseObject *>(admin_rights_.get()));
    s.store_class_end();
  }
}

bots_setBotGroupDefaultAdminRights::ReturnType bots_setBotGroupDefaultAdminRights::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_createChannel::channels_createChannel(int32 flags_, bool broadcast_, bool megagroup_, bool for_import_, bool forum_, string const &title_, string const &about_, object_ptr<InputGeoPoint> &&geo_point_, string const &address_, int32 ttl_period_)
  : flags_(flags_)
  , broadcast_(broadcast_)
  , megagroup_(megagroup_)
  , for_import_(for_import_)
  , forum_(forum_)
  , title_(title_)
  , about_(about_)
  , geo_point_(std::move(geo_point_))
  , address_(address_)
  , ttl_period_(ttl_period_)
{}

const std::int32_t channels_createChannel::ID;

void channels_createChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1862244601);
  TlStoreBinary::store((var0 = flags_ | (broadcast_ << 0) | (megagroup_ << 1) | (for_import_ << 3) | (forum_ << 5)), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(about_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s); }
  if (var0 & 4) { TlStoreString::store(address_, s); }
  if (var0 & 16) { TlStoreBinary::store(ttl_period_, s); }
}

void channels_createChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1862244601);
  TlStoreBinary::store((var0 = flags_ | (broadcast_ << 0) | (megagroup_ << 1) | (for_import_ << 3) | (forum_ << 5)), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(about_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s); }
  if (var0 & 4) { TlStoreString::store(address_, s); }
  if (var0 & 16) { TlStoreBinary::store(ttl_period_, s); }
}

void channels_createChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.createChannel");
    s.store_field("flags", (var0 = flags_ | (broadcast_ << 0) | (megagroup_ << 1) | (for_import_ << 3) | (forum_ << 5)));
    if (var0 & 1) { s.store_field("broadcast", true); }
    if (var0 & 2) { s.store_field("megagroup", true); }
    if (var0 & 8) { s.store_field("for_import", true); }
    if (var0 & 32) { s.store_field("forum", true); }
    s.store_field("title", title_);
    s.store_field("about", about_);
    if (var0 & 4) { s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get())); }
    if (var0 & 4) { s.store_field("address", address_); }
    if (var0 & 16) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

channels_createChannel::ReturnType channels_createChannel::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_getAdminedPublicChannels::channels_getAdminedPublicChannels(int32 flags_, bool by_location_, bool check_limit_, bool for_personal_)
  : flags_(flags_)
  , by_location_(by_location_)
  , check_limit_(check_limit_)
  , for_personal_(for_personal_)
{}

const std::int32_t channels_getAdminedPublicChannels::ID;

void channels_getAdminedPublicChannels::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-122669393);
  TlStoreBinary::store((var0 = flags_ | (by_location_ << 0) | (check_limit_ << 1) | (for_personal_ << 2)), s);
}

void channels_getAdminedPublicChannels::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-122669393);
  TlStoreBinary::store((var0 = flags_ | (by_location_ << 0) | (check_limit_ << 1) | (for_personal_ << 2)), s);
}

void channels_getAdminedPublicChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getAdminedPublicChannels");
    s.store_field("flags", (var0 = flags_ | (by_location_ << 0) | (check_limit_ << 1) | (for_personal_ << 2)));
    if (var0 & 1) { s.store_field("by_location", true); }
    if (var0 & 2) { s.store_field("check_limit", true); }
    if (var0 & 4) { s.store_field("for_personal", true); }
    s.store_class_end();
  }
}

channels_getAdminedPublicChannels::ReturnType channels_getAdminedPublicChannels::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

channels_updateEmojiStatus::channels_updateEmojiStatus(object_ptr<InputChannel> &&channel_, object_ptr<EmojiStatus> &&emoji_status_)
  : channel_(std::move(channel_))
  , emoji_status_(std::move(emoji_status_))
{}

const std::int32_t channels_updateEmojiStatus::ID;

void channels_updateEmojiStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-254548312);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(emoji_status_, s);
}

void channels_updateEmojiStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-254548312);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(emoji_status_, s);
}

void channels_updateEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.updateEmojiStatus");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

channels_updateEmojiStatus::ReturnType channels_updateEmojiStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

chatlists_hideChatlistUpdates::chatlists_hideChatlistUpdates(object_ptr<inputChatlistDialogFilter> &&chatlist_)
  : chatlist_(std::move(chatlist_))
{}

const std::int32_t chatlists_hideChatlistUpdates::ID;

void chatlists_hideChatlistUpdates::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1726252795);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_hideChatlistUpdates::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1726252795);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_hideChatlistUpdates::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.hideChatlistUpdates");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    s.store_class_end();
  }
}

chatlists_hideChatlistUpdates::ReturnType chatlists_hideChatlistUpdates::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

contacts_acceptContact::contacts_acceptContact(object_ptr<InputUser> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t contacts_acceptContact::ID;

void contacts_acceptContact::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-130964977);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void contacts_acceptContact::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-130964977);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void contacts_acceptContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.acceptContact");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

contacts_acceptContact::ReturnType contacts_acceptContact::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_search::contacts_search(string const &q_, int32 limit_)
  : q_(q_)
  , limit_(limit_)
{}

const std::int32_t contacts_search::ID;

void contacts_search::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(301470424);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(limit_, s);
}

void contacts_search::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(301470424);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(limit_, s);
}

void contacts_search::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.search");
    s.store_field("q", q_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

contacts_search::ReturnType contacts_search::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<contacts_found>, -1290580579>::parse(p);
#undef FAIL
}

const std::int32_t help_getInviteText::ID;

void help_getInviteText::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1295590211);
}

void help_getInviteText::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1295590211);
}

void help_getInviteText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getInviteText");
    s.store_class_end();
  }
}

help_getInviteText::ReturnType help_getInviteText::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<help_inviteText>, 415997816>::parse(p);
#undef FAIL
}

const std::int32_t help_getTermsOfServiceUpdate::ID;

void help_getTermsOfServiceUpdate::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(749019089);
}

void help_getTermsOfServiceUpdate::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(749019089);
}

void help_getTermsOfServiceUpdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getTermsOfServiceUpdate");
    s.store_class_end();
  }
}

help_getTermsOfServiceUpdate::ReturnType help_getTermsOfServiceUpdate::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_TermsOfServiceUpdate>::parse(p);
#undef FAIL
}

invokeWithApnsSecretPrefix::invokeWithApnsSecretPrefix(string const &nonce_, string const &secret_)
  : nonce_(nonce_)
  , secret_(secret_)
{}

const std::int32_t invokeWithApnsSecretPrefix::ID;

void invokeWithApnsSecretPrefix::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(229528824);
  TlStoreString::store(nonce_, s);
  TlStoreString::store(secret_, s);
}

void invokeWithApnsSecretPrefix::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(229528824);
  TlStoreString::store(nonce_, s);
  TlStoreString::store(secret_, s);
}

void invokeWithApnsSecretPrefix::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "invokeWithApnsSecretPrefix");
    s.store_field("nonce", nonce_);
    s.store_field("secret", secret_);
    s.store_class_end();
  }
}

invokeWithApnsSecretPrefix::ReturnType invokeWithApnsSecretPrefix::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<error>, -994444869>::parse(p);
#undef FAIL
}

messages_acceptUrlAuth::messages_acceptUrlAuth(int32 flags_, bool write_allowed_, object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 button_id_, string const &url_)
  : flags_(flags_)
  , write_allowed_(write_allowed_)
  , peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , button_id_(button_id_)
  , url_(url_)
{}

const std::int32_t messages_acceptUrlAuth::ID;

void messages_acceptUrlAuth::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1322487515);
  TlStoreBinary::store((var0 = flags_ | (write_allowed_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  if (var0 & 2) { TlStoreBinary::store(msg_id_, s); }
  if (var0 & 2) { TlStoreBinary::store(button_id_, s); }
  if (var0 & 4) { TlStoreString::store(url_, s); }
}

void messages_acceptUrlAuth::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1322487515);
  TlStoreBinary::store((var0 = flags_ | (write_allowed_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  if (var0 & 2) { TlStoreBinary::store(msg_id_, s); }
  if (var0 & 2) { TlStoreBinary::store(button_id_, s); }
  if (var0 & 4) { TlStoreString::store(url_, s); }
}

void messages_acceptUrlAuth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.acceptUrlAuth");
    s.store_field("flags", (var0 = flags_ | (write_allowed_ << 0)));
    if (var0 & 1) { s.store_field("write_allowed", true); }
    if (var0 & 2) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    if (var0 & 2) { s.store_field("msg_id", msg_id_); }
    if (var0 & 2) { s.store_field("button_id", button_id_); }
    if (var0 & 4) { s.store_field("url", url_); }
    s.store_class_end();
  }
}

messages_acceptUrlAuth::ReturnType messages_acceptUrlAuth::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<UrlAuthResult>::parse(p);
#undef FAIL
}

messages_deleteFactCheck::messages_deleteFactCheck(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t messages_deleteFactCheck::ID;

void messages_deleteFactCheck::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-774204404);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_deleteFactCheck::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-774204404);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_deleteFactCheck::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteFactCheck");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

messages_deleteFactCheck::ReturnType messages_deleteFactCheck::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_editMessage::messages_editMessage(int32 flags_, bool no_webpage_, bool invert_media_, object_ptr<InputPeer> &&peer_, int32 id_, string const &message_, object_ptr<InputMedia> &&media_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 schedule_date_, int32 quick_reply_shortcut_id_)
  : flags_(flags_)
  , no_webpage_(no_webpage_)
  , invert_media_(invert_media_)
  , peer_(std::move(peer_))
  , id_(id_)
  , message_(message_)
  , media_(std::move(media_))
  , reply_markup_(std::move(reply_markup_))
  , entities_(std::move(entities_))
  , schedule_date_(schedule_date_)
  , quick_reply_shortcut_id_(quick_reply_shortcut_id_)
{}

const std::int32_t messages_editMessage::ID;

void messages_editMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-539934715);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 16)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 2048) { TlStoreString::store(message_, s); }
  if (var0 & 16384) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 32768) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 131072) { TlStoreBinary::store(quick_reply_shortcut_id_, s); }
}

void messages_editMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-539934715);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 16)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 2048) { TlStoreString::store(message_, s); }
  if (var0 & 16384) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 32768) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 131072) { TlStoreBinary::store(quick_reply_shortcut_id_, s); }
}

void messages_editMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editMessage");
    s.store_field("flags", (var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 16)));
    if (var0 & 2) { s.store_field("no_webpage", true); }
    if (var0 & 65536) { s.store_field("invert_media", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    if (var0 & 2048) { s.store_field("message", message_); }
    if (var0 & 16384) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    if (var0 & 8) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 32768) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 131072) { s.store_field("quick_reply_shortcut_id", quick_reply_shortcut_id_); }
    s.store_class_end();
  }
}

messages_editMessage::ReturnType messages_editMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_editQuickReplyShortcut::messages_editQuickReplyShortcut(int32 shortcut_id_, string const &shortcut_)
  : shortcut_id_(shortcut_id_)
  , shortcut_(shortcut_)
{}

const std::int32_t messages_editQuickReplyShortcut::ID;

void messages_editQuickReplyShortcut::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1543519471);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreString::store(shortcut_, s);
}

void messages_editQuickReplyShortcut::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1543519471);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreString::store(shortcut_, s);
}

void messages_editQuickReplyShortcut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editQuickReplyShortcut");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_field("shortcut", shortcut_);
    s.store_class_end();
  }
}

messages_editQuickReplyShortcut::ReturnType messages_editQuickReplyShortcut::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_exportChatInvite::messages_exportChatInvite(int32 flags_, bool legacy_revoke_permanent_, bool request_needed_, object_ptr<InputPeer> &&peer_, int32 expire_date_, int32 usage_limit_, string const &title_, object_ptr<starsSubscriptionPricing> &&subscription_pricing_)
  : flags_(flags_)
  , legacy_revoke_permanent_(legacy_revoke_permanent_)
  , request_needed_(request_needed_)
  , peer_(std::move(peer_))
  , expire_date_(expire_date_)
  , usage_limit_(usage_limit_)
  , title_(title_)
  , subscription_pricing_(std::move(subscription_pricing_))
{}

const std::int32_t messages_exportChatInvite::ID;

void messages_exportChatInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1537876336);
  TlStoreBinary::store((var0 = flags_ | (legacy_revoke_permanent_ << 2) | (request_needed_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(expire_date_, s); }
  if (var0 & 2) { TlStoreBinary::store(usage_limit_, s); }
  if (var0 & 16) { TlStoreString::store(title_, s); }
  if (var0 & 32) { TlStoreBoxed<TlStoreObject, 88173912>::store(subscription_pricing_, s); }
}

void messages_exportChatInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1537876336);
  TlStoreBinary::store((var0 = flags_ | (legacy_revoke_permanent_ << 2) | (request_needed_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(expire_date_, s); }
  if (var0 & 2) { TlStoreBinary::store(usage_limit_, s); }
  if (var0 & 16) { TlStoreString::store(title_, s); }
  if (var0 & 32) { TlStoreBoxed<TlStoreObject, 88173912>::store(subscription_pricing_, s); }
}

void messages_exportChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.exportChatInvite");
    s.store_field("flags", (var0 = flags_ | (legacy_revoke_permanent_ << 2) | (request_needed_ << 3)));
    if (var0 & 4) { s.store_field("legacy_revoke_permanent", true); }
    if (var0 & 8) { s.store_field("request_needed", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("expire_date", expire_date_); }
    if (var0 & 2) { s.store_field("usage_limit", usage_limit_); }
    if (var0 & 16) { s.store_field("title", title_); }
    if (var0 & 32) { s.store_object_field("subscription_pricing", static_cast<const BaseObject *>(subscription_pricing_.get())); }
    s.store_class_end();
  }
}

messages_exportChatInvite::ReturnType messages_exportChatInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<ExportedChatInvite>::parse(p);
#undef FAIL
}

const std::int32_t messages_getAllDrafts::ID;

void messages_getAllDrafts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1782549861);
}

void messages_getAllDrafts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1782549861);
}

void messages_getAllDrafts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAllDrafts");
    s.store_class_end();
  }
}

messages_getAllDrafts::ReturnType messages_getAllDrafts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getAvailableReactions::messages_getAvailableReactions(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getAvailableReactions::ID;

void messages_getAvailableReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(417243308);
  TlStoreBinary::store(hash_, s);
}

void messages_getAvailableReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(417243308);
  TlStoreBinary::store(hash_, s);
}

void messages_getAvailableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAvailableReactions");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getAvailableReactions::ReturnType messages_getAvailableReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_AvailableReactions>::parse(p);
#undef FAIL
}

messages_getForumTopicsByID::messages_getForumTopicsByID(object_ptr<InputPeer> &&peer_, array<int32> &&topics_)
  : peer_(std::move(peer_))
  , topics_(std::move(topics_))
{}

const std::int32_t messages_getForumTopicsByID::ID;

void messages_getForumTopicsByID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1358280184);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(topics_, s);
}

void messages_getForumTopicsByID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1358280184);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(topics_, s);
}

void messages_getForumTopicsByID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getForumTopicsByID");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("topics", topics_.size()); for (const auto &_value : topics_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getForumTopicsByID::ReturnType messages_getForumTopicsByID::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_forumTopics>, 913709011>::parse(p);
#undef FAIL
}

messages_getOldFeaturedStickers::messages_getOldFeaturedStickers(int32 offset_, int32 limit_, int64 hash_)
  : offset_(offset_)
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t messages_getOldFeaturedStickers::ID;

void messages_getOldFeaturedStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2127598753);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getOldFeaturedStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2127598753);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getOldFeaturedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getOldFeaturedStickers");
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getOldFeaturedStickers::ReturnType messages_getOldFeaturedStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_FeaturedStickers>::parse(p);
#undef FAIL
}

messages_getPinnedDialogs::messages_getPinnedDialogs(int32 folder_id_)
  : folder_id_(folder_id_)
{}

const std::int32_t messages_getPinnedDialogs::ID;

void messages_getPinnedDialogs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-692498958);
  TlStoreBinary::store(folder_id_, s);
}

void messages_getPinnedDialogs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-692498958);
  TlStoreBinary::store(folder_id_, s);
}

void messages_getPinnedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPinnedDialogs");
    s.store_field("folder_id", folder_id_);
    s.store_class_end();
  }
}

messages_getPinnedDialogs::ReturnType messages_getPinnedDialogs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_peerDialogs>, 863093588>::parse(p);
#undef FAIL
}

messages_getPreparedInlineMessage::messages_getPreparedInlineMessage(object_ptr<InputUser> &&bot_, string const &id_)
  : bot_(std::move(bot_))
  , id_(id_)
{}

const std::int32_t messages_getPreparedInlineMessage::ID;

void messages_getPreparedInlineMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2055291464);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(id_, s);
}

void messages_getPreparedInlineMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2055291464);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(id_, s);
}

void messages_getPreparedInlineMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPreparedInlineMessage");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

messages_getPreparedInlineMessage::ReturnType messages_getPreparedInlineMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_preparedInlineMessage>, -11046771>::parse(p);
#undef FAIL
}

messages_getQuickReplies::messages_getQuickReplies(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getQuickReplies::ID;

void messages_getQuickReplies::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-729550168);
  TlStoreBinary::store(hash_, s);
}

void messages_getQuickReplies::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-729550168);
  TlStoreBinary::store(hash_, s);
}

void messages_getQuickReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getQuickReplies");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getQuickReplies::ReturnType messages_getQuickReplies::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_QuickReplies>::parse(p);
#undef FAIL
}

messages_getScheduledHistory::messages_getScheduledHistory(object_ptr<InputPeer> &&peer_, int64 hash_)
  : peer_(std::move(peer_))
  , hash_(hash_)
{}

const std::int32_t messages_getScheduledHistory::ID;

void messages_getScheduledHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-183077365);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getScheduledHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-183077365);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getScheduledHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getScheduledHistory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getScheduledHistory::ReturnType messages_getScheduledHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_getUnreadReactions::messages_getUnreadReactions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, object_ptr<InputPeer> &&saved_peer_id_, int32 offset_id_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
  , saved_peer_id_(std::move(saved_peer_id_))
  , offset_id_(offset_id_)
  , add_offset_(add_offset_)
  , limit_(limit_)
  , max_id_(max_id_)
  , min_id_(min_id_)
{}

const std::int32_t messages_getUnreadReactions::ID;

void messages_getUnreadReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1115713364);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
}

void messages_getUnreadReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1115713364);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
}

void messages_getUnreadReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getUnreadReactions");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 2) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_field("offset_id", offset_id_);
    s.store_field("add_offset", add_offset_);
    s.store_field("limit", limit_);
    s.store_field("max_id", max_id_);
    s.store_field("min_id", min_id_);
    s.store_class_end();
  }
}

messages_getUnreadReactions::ReturnType messages_getUnreadReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_readMentions::messages_readMentions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
{}

const std::int32_t messages_readMentions::ID;

void messages_readMentions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(921026381);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
}

void messages_readMentions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(921026381);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
}

void messages_readMentions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readMentions");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_class_end();
  }
}

messages_readMentions::ReturnType messages_readMentions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedHistory>, -1269012015>::parse(p);
#undef FAIL
}

messages_reportEncryptedSpam::messages_reportEncryptedSpam(object_ptr<inputEncryptedChat> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t messages_reportEncryptedSpam::ID;

void messages_reportEncryptedSpam::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1259113487);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
}

void messages_reportEncryptedSpam::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1259113487);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
}

void messages_reportEncryptedSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reportEncryptedSpam");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_reportEncryptedSpam::ReturnType messages_reportEncryptedSpam::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_reportReaction::messages_reportReaction(object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<InputPeer> &&reaction_peer_)
  : peer_(std::move(peer_))
  , id_(id_)
  , reaction_peer_(std::move(reaction_peer_))
{}

const std::int32_t messages_reportReaction::ID;

void messages_reportReaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1063567478);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_peer_, s);
}

void messages_reportReaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1063567478);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_peer_, s);
}

void messages_reportReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reportReaction");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_object_field("reaction_peer", static_cast<const BaseObject *>(reaction_peer_.get()));
    s.store_class_end();
  }
}

messages_reportReaction::ReturnType messages_reportReaction::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_requestAppWebView::messages_requestAppWebView(int32 flags_, bool write_allowed_, bool compact_, bool fullscreen_, object_ptr<InputPeer> &&peer_, object_ptr<InputBotApp> &&app_, string const &start_param_, object_ptr<dataJSON> &&theme_params_, string const &platform_)
  : flags_(flags_)
  , write_allowed_(write_allowed_)
  , compact_(compact_)
  , fullscreen_(fullscreen_)
  , peer_(std::move(peer_))
  , app_(std::move(app_))
  , start_param_(start_param_)
  , theme_params_(std::move(theme_params_))
  , platform_(platform_)
{}

const std::int32_t messages_requestAppWebView::ID;

void messages_requestAppWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1398901710);
  TlStoreBinary::store((var0 = flags_ | (write_allowed_ << 0) | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(app_, s);
  if (var0 & 2) { TlStoreString::store(start_param_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
}

void messages_requestAppWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1398901710);
  TlStoreBinary::store((var0 = flags_ | (write_allowed_ << 0) | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(app_, s);
  if (var0 & 2) { TlStoreString::store(start_param_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
}

void messages_requestAppWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.requestAppWebView");
    s.store_field("flags", (var0 = flags_ | (write_allowed_ << 0) | (compact_ << 7) | (fullscreen_ << 8)));
    if (var0 & 1) { s.store_field("write_allowed", true); }
    if (var0 & 128) { s.store_field("compact", true); }
    if (var0 & 256) { s.store_field("fullscreen", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("app", static_cast<const BaseObject *>(app_.get()));
    if (var0 & 2) { s.store_field("start_param", start_param_); }
    if (var0 & 4) { s.store_object_field("theme_params", static_cast<const BaseObject *>(theme_params_.get())); }
    s.store_field("platform", platform_);
    s.store_class_end();
  }
}

messages_requestAppWebView::ReturnType messages_requestAppWebView::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<webViewResultUrl>, 1294139288>::parse(p);
#undef FAIL
}

messages_saveRecentSticker::messages_saveRecentSticker(int32 flags_, bool attached_, object_ptr<InputDocument> &&id_, bool unsave_)
  : flags_(flags_)
  , attached_(attached_)
  , id_(std::move(id_))
  , unsave_(unsave_)
{}

const std::int32_t messages_saveRecentSticker::ID;

void messages_saveRecentSticker::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(958863608);
  TlStoreBinary::store((var0 = flags_ | (attached_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unsave_, s);
}

void messages_saveRecentSticker::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(958863608);
  TlStoreBinary::store((var0 = flags_ | (attached_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unsave_, s);
}

void messages_saveRecentSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.saveRecentSticker");
    s.store_field("flags", (var0 = flags_ | (attached_ << 0)));
    if (var0 & 1) { s.store_field("attached", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_field("unsave", unsave_);
    s.store_class_end();
  }
}

messages_saveRecentSticker::ReturnType messages_saveRecentSticker::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_sendBotRequestedPeer::messages_sendBotRequestedPeer(object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 button_id_, array<object_ptr<InputPeer>> &&requested_peers_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , button_id_(button_id_)
  , requested_peers_(std::move(requested_peers_))
{}

const std::int32_t messages_sendBotRequestedPeer::ID;

void messages_sendBotRequestedPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1850552224);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(button_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(requested_peers_, s);
}

void messages_sendBotRequestedPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1850552224);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(button_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(requested_peers_, s);
}

void messages_sendBotRequestedPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendBotRequestedPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("button_id", button_id_);
    { s.store_vector_begin("requested_peers", requested_peers_.size()); for (const auto &_value : requested_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_sendBotRequestedPeer::ReturnType messages_sendBotRequestedPeer::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_sendMessage::messages_sendMessage(int32 flags_, bool no_webpage_, bool silent_, bool background_, bool clear_draft_, bool noforwards_, bool update_stickersets_order_, bool invert_media_, bool allow_paid_floodskip_, object_ptr<InputPeer> &&peer_, object_ptr<InputReplyTo> &&reply_to_, string const &message_, int64 random_id_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_, object_ptr<InputQuickReplyShortcut> &&quick_reply_shortcut_, int64 effect_, int64 allow_paid_stars_, object_ptr<suggestedPost> &&suggested_post_)
  : flags_(flags_)
  , no_webpage_(no_webpage_)
  , silent_(silent_)
  , background_(background_)
  , clear_draft_(clear_draft_)
  , noforwards_(noforwards_)
  , update_stickersets_order_(update_stickersets_order_)
  , invert_media_(invert_media_)
  , allow_paid_floodskip_(allow_paid_floodskip_)
  , peer_(std::move(peer_))
  , reply_to_(std::move(reply_to_))
  , message_(message_)
  , random_id_(random_id_)
  , reply_markup_(std::move(reply_markup_))
  , entities_(std::move(entities_))
  , schedule_date_(schedule_date_)
  , send_as_(std::move(send_as_))
  , quick_reply_shortcut_(std::move(quick_reply_shortcut_))
  , effect_(effect_)
  , allow_paid_stars_(allow_paid_stars_)
  , suggested_post_(std::move(suggested_post_))
{}

const std::int32_t messages_sendMessage::ID;

void messages_sendMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-33170278);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreString::store(message_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 262144) { TlStoreBinary::store(effect_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
  if (var0 & 4194304) { TlStoreBoxed<TlStoreObject, 244201445>::store(suggested_post_, s); }
}

void messages_sendMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-33170278);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreString::store(message_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 262144) { TlStoreBinary::store(effect_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
  if (var0 & 4194304) { TlStoreBoxed<TlStoreObject, 244201445>::store(suggested_post_, s); }
}

void messages_sendMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendMessage");
    s.store_field("flags", (var0 = flags_ | (no_webpage_ << 1) | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)));
    if (var0 & 2) { s.store_field("no_webpage", true); }
    if (var0 & 32) { s.store_field("silent", true); }
    if (var0 & 64) { s.store_field("background", true); }
    if (var0 & 128) { s.store_field("clear_draft", true); }
    if (var0 & 16384) { s.store_field("noforwards", true); }
    if (var0 & 32768) { s.store_field("update_stickersets_order", true); }
    if (var0 & 65536) { s.store_field("invert_media", true); }
    if (var0 & 524288) { s.store_field("allow_paid_floodskip", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    s.store_field("message", message_);
    s.store_field("random_id", random_id_);
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    if (var0 & 8) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 1024) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 8192) { s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get())); }
    if (var0 & 131072) { s.store_object_field("quick_reply_shortcut", static_cast<const BaseObject *>(quick_reply_shortcut_.get())); }
    if (var0 & 262144) { s.store_field("effect", effect_); }
    if (var0 & 2097152) { s.store_field("allow_paid_stars", allow_paid_stars_); }
    if (var0 & 4194304) { s.store_object_field("suggested_post", static_cast<const BaseObject *>(suggested_post_.get())); }
    s.store_class_end();
  }
}

messages_sendMessage::ReturnType messages_sendMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_sendPaidReaction::messages_sendPaidReaction(int32 flags_, object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 count_, int64 random_id_, object_ptr<PaidReactionPrivacy> &&private_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , count_(count_)
  , random_id_(random_id_)
  , private_(std::move(private_))
{}

const std::int32_t messages_sendPaidReaction::ID;

void messages_sendPaidReaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1488702288);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(count_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(private_, s); }
}

void messages_sendPaidReaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1488702288);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(count_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(private_, s); }
}

void messages_sendPaidReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendPaidReaction");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("count", count_);
    s.store_field("random_id", random_id_);
    if (var0 & 1) { s.store_object_field("private", static_cast<const BaseObject *>(private_.get())); }
    s.store_class_end();
  }
}

messages_sendPaidReaction::ReturnType messages_sendPaidReaction::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_sendWebViewData::messages_sendWebViewData(object_ptr<InputUser> &&bot_, int64 random_id_, string const &button_text_, string const &data_)
  : bot_(std::move(bot_))
  , random_id_(random_id_)
  , button_text_(button_text_)
  , data_(data_)
{}

const std::int32_t messages_sendWebViewData::ID;

void messages_sendWebViewData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-603831608);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(button_text_, s);
  TlStoreString::store(data_, s);
}

void messages_sendWebViewData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-603831608);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(button_text_, s);
  TlStoreString::store(data_, s);
}

void messages_sendWebViewData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendWebViewData");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("random_id", random_id_);
    s.store_field("button_text", button_text_);
    s.store_field("data", data_);
    s.store_class_end();
  }
}

messages_sendWebViewData::ReturnType messages_sendWebViewData::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_setBotShippingResults::messages_setBotShippingResults(int32 flags_, int64 query_id_, string const &error_, array<object_ptr<shippingOption>> &&shipping_options_)
  : flags_(flags_)
  , query_id_(query_id_)
  , error_(error_)
  , shipping_options_(std::move(shipping_options_))
{}

const std::int32_t messages_setBotShippingResults::ID;

void messages_setBotShippingResults::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-436833542);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreString::store(error_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1239335713>>, 481674261>::store(shipping_options_, s); }
}

void messages_setBotShippingResults::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-436833542);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreString::store(error_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1239335713>>, 481674261>::store(shipping_options_, s); }
}

void messages_setBotShippingResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setBotShippingResults");
    s.store_field("flags", (var0 = flags_));
    s.store_field("query_id", query_id_);
    if (var0 & 1) { s.store_field("error", error_); }
    if (var0 & 2) { { s.store_vector_begin("shipping_options", shipping_options_.size()); for (const auto &_value : shipping_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

messages_setBotShippingResults::ReturnType messages_setBotShippingResults::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_toggleDialogPin::messages_toggleDialogPin(int32 flags_, bool pinned_, object_ptr<InputDialogPeer> &&peer_)
  : flags_(flags_)
  , pinned_(pinned_)
  , peer_(std::move(peer_))
{}

const std::int32_t messages_toggleDialogPin::ID;

void messages_toggleDialogPin::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1489903017);
  TlStoreBinary::store((var0 = flags_ | (pinned_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_toggleDialogPin::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1489903017);
  TlStoreBinary::store((var0 = flags_ | (pinned_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_toggleDialogPin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleDialogPin");
    s.store_field("flags", (var0 = flags_ | (pinned_ << 0)));
    if (var0 & 1) { s.store_field("pinned", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_toggleDialogPin::ReturnType messages_toggleDialogPin::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_uploadImportedMedia::messages_uploadImportedMedia(object_ptr<InputPeer> &&peer_, int64 import_id_, string const &file_name_, object_ptr<InputMedia> &&media_)
  : peer_(std::move(peer_))
  , import_id_(import_id_)
  , file_name_(file_name_)
  , media_(std::move(media_))
{}

const std::int32_t messages_uploadImportedMedia::ID;

void messages_uploadImportedMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(713433234);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(import_id_, s);
  TlStoreString::store(file_name_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void messages_uploadImportedMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(713433234);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(import_id_, s);
  TlStoreString::store(file_name_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void messages_uploadImportedMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.uploadImportedMedia");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("import_id", import_id_);
    s.store_field("file_name", file_name_);
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

messages_uploadImportedMedia::ReturnType messages_uploadImportedMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<MessageMedia>::parse(p);
#undef FAIL
}

messages_uploadMedia::messages_uploadMedia(int32 flags_, string const &business_connection_id_, object_ptr<InputPeer> &&peer_, object_ptr<InputMedia> &&media_)
  : flags_(flags_)
  , business_connection_id_(business_connection_id_)
  , peer_(std::move(peer_))
  , media_(std::move(media_))
{}

const std::int32_t messages_uploadMedia::ID;

void messages_uploadMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(345405816);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(business_connection_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void messages_uploadMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(345405816);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(business_connection_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void messages_uploadMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.uploadMedia");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("business_connection_id", business_connection_id_); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

messages_uploadMedia::ReturnType messages_uploadMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<MessageMedia>::parse(p);
#undef FAIL
}

payments_checkCanSendGift::payments_checkCanSendGift(int64 gift_id_)
  : gift_id_(gift_id_)
{}

const std::int32_t payments_checkCanSendGift::ID;

void payments_checkCanSendGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1060835895);
  TlStoreBinary::store(gift_id_, s);
}

void payments_checkCanSendGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1060835895);
  TlStoreBinary::store(gift_id_, s);
}

void payments_checkCanSendGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.checkCanSendGift");
    s.store_field("gift_id", gift_id_);
    s.store_class_end();
  }
}

payments_checkCanSendGift::ReturnType payments_checkCanSendGift::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_CheckCanSendGiftResult>::parse(p);
#undef FAIL
}

payments_getPaymentReceipt::payments_getPaymentReceipt(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t payments_getPaymentReceipt::ID;

void payments_getPaymentReceipt::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(611897804);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void payments_getPaymentReceipt::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(611897804);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void payments_getPaymentReceipt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getPaymentReceipt");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

payments_getPaymentReceipt::ReturnType payments_getPaymentReceipt::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_PaymentReceipt>::parse(p);
#undef FAIL
}

payments_getStarsRevenueStats::payments_getStarsRevenueStats(int32 flags_, bool dark_, bool ton_, object_ptr<InputPeer> &&peer_)
  : flags_(flags_)
  , dark_(dark_)
  , ton_(ton_)
  , peer_(std::move(peer_))
{}

const std::int32_t payments_getStarsRevenueStats::ID;

void payments_getStarsRevenueStats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-652215594);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0) | (ton_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_getStarsRevenueStats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-652215594);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0) | (ton_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_getStarsRevenueStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsRevenueStats");
    s.store_field("flags", (var0 = flags_ | (dark_ << 0) | (ton_ << 1)));
    if (var0 & 1) { s.store_field("dark", true); }
    if (var0 & 2) { s.store_field("ton", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

payments_getStarsRevenueStats::ReturnType payments_getStarsRevenueStats::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starsRevenueStats>, 1814066038>::parse(p);
#undef FAIL
}

phone_discardGroupCall::phone_discardGroupCall(object_ptr<InputGroupCall> &&call_)
  : call_(std::move(call_))
{}

const std::int32_t phone_discardGroupCall::ID;

void phone_discardGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2054648117);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_discardGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2054648117);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_discardGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.discardGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

phone_discardGroupCall::ReturnType phone_discardGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_editGroupCallTitle::phone_editGroupCallTitle(object_ptr<InputGroupCall> &&call_, string const &title_)
  : call_(std::move(call_))
  , title_(title_)
{}

const std::int32_t phone_editGroupCallTitle::ID;

void phone_editGroupCallTitle::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(480685066);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreString::store(title_, s);
}

void phone_editGroupCallTitle::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(480685066);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreString::store(title_, s);
}

void phone_editGroupCallTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.editGroupCallTitle");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("title", title_);
    s.store_class_end();
  }
}

phone_editGroupCallTitle::ReturnType phone_editGroupCallTitle::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_getGroupParticipants::phone_getGroupParticipants(object_ptr<InputGroupCall> &&call_, array<object_ptr<InputPeer>> &&ids_, array<int32> &&sources_, string const &offset_, int32 limit_)
  : call_(std::move(call_))
  , ids_(std::move(ids_))
  , sources_(std::move(sources_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t phone_getGroupParticipants::ID;

void phone_getGroupParticipants::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-984033109);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(ids_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(sources_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void phone_getGroupParticipants::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-984033109);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(ids_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(sources_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void phone_getGroupParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.getGroupParticipants");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    { s.store_vector_begin("ids", ids_.size()); for (const auto &_value : ids_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("sources", sources_.size()); for (const auto &_value : sources_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

phone_getGroupParticipants::ReturnType phone_getGroupParticipants::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_groupParticipants>, -193506890>::parse(p);
#undef FAIL
}

phone_sendConferenceCallBroadcast::phone_sendConferenceCallBroadcast(object_ptr<InputGroupCall> &&call_, bytes &&block_)
  : call_(std::move(call_))
  , block_(std::move(block_))
{}

const std::int32_t phone_sendConferenceCallBroadcast::ID;

void phone_sendConferenceCallBroadcast::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-965732096);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreString::store(block_, s);
}

void phone_sendConferenceCallBroadcast::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-965732096);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreString::store(block_, s);
}

void phone_sendConferenceCallBroadcast::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.sendConferenceCallBroadcast");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_bytes_field("block", block_);
    s.store_class_end();
  }
}

phone_sendConferenceCallBroadcast::ReturnType phone_sendConferenceCallBroadcast::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_setCallRating::phone_setCallRating(int32 flags_, bool user_initiative_, object_ptr<inputPhoneCall> &&peer_, int32 rating_, string const &comment_)
  : flags_(flags_)
  , user_initiative_(user_initiative_)
  , peer_(std::move(peer_))
  , rating_(rating_)
  , comment_(comment_)
{}

const std::int32_t phone_setCallRating::ID;

void phone_setCallRating::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1508562471);
  TlStoreBinary::store((var0 = flags_ | (user_initiative_ << 0)), s);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBinary::store(rating_, s);
  TlStoreString::store(comment_, s);
}

void phone_setCallRating::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1508562471);
  TlStoreBinary::store((var0 = flags_ | (user_initiative_ << 0)), s);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBinary::store(rating_, s);
  TlStoreString::store(comment_, s);
}

void phone_setCallRating::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.setCallRating");
    s.store_field("flags", (var0 = flags_ | (user_initiative_ << 0)));
    if (var0 & 1) { s.store_field("user_initiative", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("rating", rating_);
    s.store_field("comment", comment_);
    s.store_class_end();
  }
}

phone_setCallRating::ReturnType phone_setCallRating::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

photos_updateProfilePhoto::photos_updateProfilePhoto(int32 flags_, bool fallback_, object_ptr<InputUser> &&bot_, object_ptr<InputPhoto> &&id_)
  : flags_(flags_)
  , fallback_(fallback_)
  , bot_(std::move(bot_))
  , id_(std::move(id_))
{}

const std::int32_t photos_updateProfilePhoto::ID;

void photos_updateProfilePhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(166207545);
  TlStoreBinary::store((var0 = flags_ | (fallback_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void photos_updateProfilePhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(166207545);
  TlStoreBinary::store((var0 = flags_ | (fallback_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void photos_updateProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.updateProfilePhoto");
    s.store_field("flags", (var0 = flags_ | (fallback_ << 0)));
    if (var0 & 1) { s.store_field("fallback", true); }
    if (var0 & 2) { s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get())); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

photos_updateProfilePhoto::ReturnType photos_updateProfilePhoto::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<photos_photo>, 539045032>::parse(p);
#undef FAIL
}

const std::int32_t smsjobs_getStatus::ID;

void smsjobs_getStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(279353576);
}

void smsjobs_getStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(279353576);
}

void smsjobs_getStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.getStatus");
    s.store_class_end();
  }
}

smsjobs_getStatus::ReturnType smsjobs_getStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<smsjobs_status>, 720277905>::parse(p);
#undef FAIL
}

stats_getBroadcastStats::stats_getBroadcastStats(int32 flags_, bool dark_, object_ptr<InputChannel> &&channel_)
  : flags_(flags_)
  , dark_(dark_)
  , channel_(std::move(channel_))
{}

const std::int32_t stats_getBroadcastStats::ID;

void stats_getBroadcastStats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1421720550);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void stats_getBroadcastStats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1421720550);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void stats_getBroadcastStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.getBroadcastStats");
    s.store_field("flags", (var0 = flags_ | (dark_ << 0)));
    if (var0 & 1) { s.store_field("dark", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

stats_getBroadcastStats::ReturnType stats_getBroadcastStats::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stats_broadcastStats>, 963421692>::parse(p);
#undef FAIL
}

stats_getStoryStats::stats_getStoryStats(int32 flags_, bool dark_, object_ptr<InputPeer> &&peer_, int32 id_)
  : flags_(flags_)
  , dark_(dark_)
  , peer_(std::move(peer_))
  , id_(id_)
{}

const std::int32_t stats_getStoryStats::ID;

void stats_getStoryStats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(927985472);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void stats_getStoryStats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(927985472);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void stats_getStoryStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.getStoryStats");
    s.store_field("flags", (var0 = flags_ | (dark_ << 0)));
    if (var0 & 1) { s.store_field("dark", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

stats_getStoryStats::ReturnType stats_getStoryStats::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stats_storyStats>, 1355613820>::parse(p);
#undef FAIL
}

stickers_setStickerSetThumb::stickers_setStickerSetThumb(int32 flags_, object_ptr<InputStickerSet> &&stickerset_, object_ptr<InputDocument> &&thumb_, int64 thumb_document_id_)
  : flags_(flags_)
  , stickerset_(std::move(stickerset_))
  , thumb_(std::move(thumb_))
  , thumb_document_id_(thumb_document_id_)
{}

const std::int32_t stickers_setStickerSetThumb::ID;

void stickers_setStickerSetThumb::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1486204014);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  if (var0 & 2) { TlStoreBinary::store(thumb_document_id_, s); }
}

void stickers_setStickerSetThumb::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1486204014);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  if (var0 & 2) { TlStoreBinary::store(thumb_document_id_, s); }
}

void stickers_setStickerSetThumb::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.setStickerSetThumb");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    if (var0 & 1) { s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get())); }
    if (var0 & 2) { s.store_field("thumb_document_id", thumb_document_id_); }
    s.store_class_end();
  }
}

stickers_setStickerSetThumb::ReturnType stickers_setStickerSetThumb::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stories_getAllStories::stories_getAllStories(int32 flags_, bool next_, bool hidden_, string const &state_)
  : flags_(flags_)
  , next_(next_)
  , hidden_(hidden_)
  , state_(state_)
{}

const std::int32_t stories_getAllStories::ID;

void stories_getAllStories::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-290400731);
  TlStoreBinary::store((var0 = flags_ | (next_ << 1) | (hidden_ << 2)), s);
  if (var0 & 1) { TlStoreString::store(state_, s); }
}

void stories_getAllStories::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-290400731);
  TlStoreBinary::store((var0 = flags_ | (next_ << 1) | (hidden_ << 2)), s);
  if (var0 & 1) { TlStoreString::store(state_, s); }
}

void stories_getAllStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getAllStories");
    s.store_field("flags", (var0 = flags_ | (next_ << 1) | (hidden_ << 2)));
    if (var0 & 2) { s.store_field("next", true); }
    if (var0 & 4) { s.store_field("hidden", true); }
    if (var0 & 1) { s.store_field("state", state_); }
    s.store_class_end();
  }
}

stories_getAllStories::ReturnType stories_getAllStories::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<stories_AllStories>::parse(p);
#undef FAIL
}

stories_incrementStoryViews::stories_incrementStoryViews(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t stories_incrementStoryViews::ID;

void stories_incrementStoryViews::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1308456197);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_incrementStoryViews::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1308456197);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_incrementStoryViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.incrementStoryViews");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_incrementStoryViews::ReturnType stories_incrementStoryViews::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
