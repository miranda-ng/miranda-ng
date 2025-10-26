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


object_ptr<AttachMenuBots> AttachMenuBots::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case attachMenuBotsNotModified::ID:
      return attachMenuBotsNotModified::fetch(p);
    case attachMenuBots::ID:
      return attachMenuBots::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t attachMenuBotsNotModified::ID;

object_ptr<AttachMenuBots> attachMenuBotsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuBotsNotModified>();
}

void attachMenuBotsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuBotsNotModified");
    s.store_class_end();
  }
}

const std::int32_t attachMenuBots::ID;

object_ptr<AttachMenuBots> attachMenuBots::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuBots>(p);
}

attachMenuBots::attachMenuBots(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , bots_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<attachMenuBot>, -653423106>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void attachMenuBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuBots");
    s.store_field("hash", hash_);
    { s.store_vector_begin("bots", bots_.size()); for (const auto &_value : bots_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

availableEffect::availableEffect()
  : flags_()
  , premium_required_()
  , id_()
  , emoticon_()
  , static_icon_id_()
  , effect_sticker_id_()
  , effect_animation_id_()
{}

const std::int32_t availableEffect::ID;

object_ptr<availableEffect> availableEffect::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<availableEffect> res = make_tl_object<availableEffect>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->premium_required_ = (var0 & 4) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->emoticon_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->static_icon_id_ = TlFetchLong::parse(p); }
  res->effect_sticker_id_ = TlFetchLong::parse(p);
  if (var0 & 2) { res->effect_animation_id_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void availableEffect::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "availableEffect");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (premium_required_ << 2)));
    if (var0 & 4) { s.store_field("premium_required", true); }
    s.store_field("id", id_);
    s.store_field("emoticon", emoticon_);
    if (var0 & 1) { s.store_field("static_icon_id", static_icon_id_); }
    s.store_field("effect_sticker_id", effect_sticker_id_);
    if (var0 & 2) { s.store_field("effect_animation_id", effect_animation_id_); }
    s.store_class_end();
  }
}

object_ptr<BaseTheme> BaseTheme::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case baseThemeClassic::ID:
      return baseThemeClassic::fetch(p);
    case baseThemeDay::ID:
      return baseThemeDay::fetch(p);
    case baseThemeNight::ID:
      return baseThemeNight::fetch(p);
    case baseThemeTinted::ID:
      return baseThemeTinted::fetch(p);
    case baseThemeArctic::ID:
      return baseThemeArctic::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t baseThemeClassic::ID;

object_ptr<BaseTheme> baseThemeClassic::fetch(TlBufferParser &p) {
  return make_tl_object<baseThemeClassic>();
}

void baseThemeClassic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void baseThemeClassic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void baseThemeClassic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "baseThemeClassic");
    s.store_class_end();
  }
}

const std::int32_t baseThemeDay::ID;

object_ptr<BaseTheme> baseThemeDay::fetch(TlBufferParser &p) {
  return make_tl_object<baseThemeDay>();
}

void baseThemeDay::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void baseThemeDay::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void baseThemeDay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "baseThemeDay");
    s.store_class_end();
  }
}

const std::int32_t baseThemeNight::ID;

object_ptr<BaseTheme> baseThemeNight::fetch(TlBufferParser &p) {
  return make_tl_object<baseThemeNight>();
}

void baseThemeNight::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void baseThemeNight::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void baseThemeNight::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "baseThemeNight");
    s.store_class_end();
  }
}

const std::int32_t baseThemeTinted::ID;

object_ptr<BaseTheme> baseThemeTinted::fetch(TlBufferParser &p) {
  return make_tl_object<baseThemeTinted>();
}

void baseThemeTinted::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void baseThemeTinted::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void baseThemeTinted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "baseThemeTinted");
    s.store_class_end();
  }
}

const std::int32_t baseThemeArctic::ID;

object_ptr<BaseTheme> baseThemeArctic::fetch(TlBufferParser &p) {
  return make_tl_object<baseThemeArctic>();
}

void baseThemeArctic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void baseThemeArctic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void baseThemeArctic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "baseThemeArctic");
    s.store_class_end();
  }
}

businessBotRights::businessBotRights()
  : flags_()
  , reply_()
  , read_messages_()
  , delete_sent_messages_()
  , delete_received_messages_()
  , edit_name_()
  , edit_bio_()
  , edit_profile_photo_()
  , edit_username_()
  , view_gifts_()
  , sell_gifts_()
  , change_gift_settings_()
  , transfer_and_upgrade_gifts_()
  , transfer_stars_()
  , manage_stories_()
{}

businessBotRights::businessBotRights(int32 flags_, bool reply_, bool read_messages_, bool delete_sent_messages_, bool delete_received_messages_, bool edit_name_, bool edit_bio_, bool edit_profile_photo_, bool edit_username_, bool view_gifts_, bool sell_gifts_, bool change_gift_settings_, bool transfer_and_upgrade_gifts_, bool transfer_stars_, bool manage_stories_)
  : flags_(flags_)
  , reply_(reply_)
  , read_messages_(read_messages_)
  , delete_sent_messages_(delete_sent_messages_)
  , delete_received_messages_(delete_received_messages_)
  , edit_name_(edit_name_)
  , edit_bio_(edit_bio_)
  , edit_profile_photo_(edit_profile_photo_)
  , edit_username_(edit_username_)
  , view_gifts_(view_gifts_)
  , sell_gifts_(sell_gifts_)
  , change_gift_settings_(change_gift_settings_)
  , transfer_and_upgrade_gifts_(transfer_and_upgrade_gifts_)
  , transfer_stars_(transfer_stars_)
  , manage_stories_(manage_stories_)
{}

const std::int32_t businessBotRights::ID;

object_ptr<businessBotRights> businessBotRights::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessBotRights> res = make_tl_object<businessBotRights>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->reply_ = (var0 & 1) != 0;
  res->read_messages_ = (var0 & 2) != 0;
  res->delete_sent_messages_ = (var0 & 4) != 0;
  res->delete_received_messages_ = (var0 & 8) != 0;
  res->edit_name_ = (var0 & 16) != 0;
  res->edit_bio_ = (var0 & 32) != 0;
  res->edit_profile_photo_ = (var0 & 64) != 0;
  res->edit_username_ = (var0 & 128) != 0;
  res->view_gifts_ = (var0 & 256) != 0;
  res->sell_gifts_ = (var0 & 512) != 0;
  res->change_gift_settings_ = (var0 & 1024) != 0;
  res->transfer_and_upgrade_gifts_ = (var0 & 2048) != 0;
  res->transfer_stars_ = (var0 & 4096) != 0;
  res->manage_stories_ = (var0 & 8192) != 0;
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessBotRights::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (reply_ << 0) | (read_messages_ << 1) | (delete_sent_messages_ << 2) | (delete_received_messages_ << 3) | (edit_name_ << 4) | (edit_bio_ << 5) | (edit_profile_photo_ << 6) | (edit_username_ << 7) | (view_gifts_ << 8) | (sell_gifts_ << 9) | (change_gift_settings_ << 10) | (transfer_and_upgrade_gifts_ << 11) | (transfer_stars_ << 12) | (manage_stories_ << 13)), s);
}

void businessBotRights::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (reply_ << 0) | (read_messages_ << 1) | (delete_sent_messages_ << 2) | (delete_received_messages_ << 3) | (edit_name_ << 4) | (edit_bio_ << 5) | (edit_profile_photo_ << 6) | (edit_username_ << 7) | (view_gifts_ << 8) | (sell_gifts_ << 9) | (change_gift_settings_ << 10) | (transfer_and_upgrade_gifts_ << 11) | (transfer_stars_ << 12) | (manage_stories_ << 13)), s);
}

void businessBotRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessBotRights");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (reply_ << 0) | (read_messages_ << 1) | (delete_sent_messages_ << 2) | (delete_received_messages_ << 3) | (edit_name_ << 4) | (edit_bio_ << 5) | (edit_profile_photo_ << 6) | (edit_username_ << 7) | (view_gifts_ << 8) | (sell_gifts_ << 9) | (change_gift_settings_ << 10) | (transfer_and_upgrade_gifts_ << 11) | (transfer_stars_ << 12) | (manage_stories_ << 13)));
    if (var0 & 1) { s.store_field("reply", true); }
    if (var0 & 2) { s.store_field("read_messages", true); }
    if (var0 & 4) { s.store_field("delete_sent_messages", true); }
    if (var0 & 8) { s.store_field("delete_received_messages", true); }
    if (var0 & 16) { s.store_field("edit_name", true); }
    if (var0 & 32) { s.store_field("edit_bio", true); }
    if (var0 & 64) { s.store_field("edit_profile_photo", true); }
    if (var0 & 128) { s.store_field("edit_username", true); }
    if (var0 & 256) { s.store_field("view_gifts", true); }
    if (var0 & 512) { s.store_field("sell_gifts", true); }
    if (var0 & 1024) { s.store_field("change_gift_settings", true); }
    if (var0 & 2048) { s.store_field("transfer_and_upgrade_gifts", true); }
    if (var0 & 4096) { s.store_field("transfer_stars", true); }
    if (var0 & 8192) { s.store_field("manage_stories", true); }
    s.store_class_end();
  }
}

businessWorkHours::businessWorkHours()
  : flags_()
  , open_now_()
  , timezone_id_()
  , weekly_open_()
{}

businessWorkHours::businessWorkHours(int32 flags_, bool open_now_, string const &timezone_id_, array<object_ptr<businessWeeklyOpen>> &&weekly_open_)
  : flags_(flags_)
  , open_now_(open_now_)
  , timezone_id_(timezone_id_)
  , weekly_open_(std::move(weekly_open_))
{}

const std::int32_t businessWorkHours::ID;

object_ptr<businessWorkHours> businessWorkHours::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessWorkHours> res = make_tl_object<businessWorkHours>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->open_now_ = (var0 & 1) != 0;
  res->timezone_id_ = TlFetchString<string>::parse(p);
  res->weekly_open_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<businessWeeklyOpen>, 302717625>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessWorkHours::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (open_now_ << 0)), s);
  TlStoreString::store(timezone_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 302717625>>, 481674261>::store(weekly_open_, s);
}

void businessWorkHours::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (open_now_ << 0)), s);
  TlStoreString::store(timezone_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 302717625>>, 481674261>::store(weekly_open_, s);
}

void businessWorkHours::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessWorkHours");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (open_now_ << 0)));
    if (var0 & 1) { s.store_field("open_now", true); }
    s.store_field("timezone_id", timezone_id_);
    { s.store_vector_begin("weekly_open", weekly_open_.size()); for (const auto &_value : weekly_open_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<DialogPeer> DialogPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case dialogPeer::ID:
      return dialogPeer::fetch(p);
    case dialogPeerFolder::ID:
      return dialogPeerFolder::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t dialogPeer::ID;

object_ptr<DialogPeer> dialogPeer::fetch(TlBufferParser &p) {
  return make_tl_object<dialogPeer>(p);
}

dialogPeer::dialogPeer(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
{}

void dialogPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialogPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

const std::int32_t dialogPeerFolder::ID;

object_ptr<DialogPeer> dialogPeerFolder::fetch(TlBufferParser &p) {
  return make_tl_object<dialogPeerFolder>(p);
}

dialogPeerFolder::dialogPeerFolder(TlBufferParser &p)
  : folder_id_(TlFetchInt::parse(p))
{}

void dialogPeerFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialogPeerFolder");
    s.store_field("folder_id", folder_id_);
    s.store_class_end();
  }
}

const std::int32_t error::ID;

object_ptr<error> error::fetch(TlBufferParser &p) {
  return make_tl_object<error>(p);
}

error::error(TlBufferParser &p)
  : code_(TlFetchInt::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void error::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "error");
    s.store_field("code", code_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

object_ptr<ExportedChatInvite> ExportedChatInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatInviteExported::ID:
      return chatInviteExported::fetch(p);
    case chatInvitePublicJoinRequests::ID:
      return chatInvitePublicJoinRequests::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

chatInviteExported::chatInviteExported()
  : flags_()
  , revoked_()
  , permanent_()
  , request_needed_()
  , link_()
  , admin_id_()
  , date_()
  , start_date_()
  , expire_date_()
  , usage_limit_()
  , usage_()
  , requested_()
  , subscription_expired_()
  , title_()
  , subscription_pricing_()
{}

const std::int32_t chatInviteExported::ID;

object_ptr<ExportedChatInvite> chatInviteExported::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatInviteExported> res = make_tl_object<chatInviteExported>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->revoked_ = (var0 & 1) != 0;
  res->permanent_ = (var0 & 32) != 0;
  res->request_needed_ = (var0 & 64) != 0;
  res->link_ = TlFetchString<string>::parse(p);
  res->admin_id_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 16) { res->start_date_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->expire_date_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->usage_limit_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->usage_ = TlFetchInt::parse(p); }
  if (var0 & 128) { res->requested_ = TlFetchInt::parse(p); }
  if (var0 & 1024) { res->subscription_expired_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 512) { res->subscription_pricing_ = TlFetchBoxed<TlFetchObject<starsSubscriptionPricing>, 88173912>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chatInviteExported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteExported");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (revoked_ << 0) | (permanent_ << 5) | (request_needed_ << 6)));
    if (var0 & 1) { s.store_field("revoked", true); }
    if (var0 & 32) { s.store_field("permanent", true); }
    if (var0 & 64) { s.store_field("request_needed", true); }
    s.store_field("link", link_);
    s.store_field("admin_id", admin_id_);
    s.store_field("date", date_);
    if (var0 & 16) { s.store_field("start_date", start_date_); }
    if (var0 & 2) { s.store_field("expire_date", expire_date_); }
    if (var0 & 4) { s.store_field("usage_limit", usage_limit_); }
    if (var0 & 8) { s.store_field("usage", usage_); }
    if (var0 & 128) { s.store_field("requested", requested_); }
    if (var0 & 1024) { s.store_field("subscription_expired", subscription_expired_); }
    if (var0 & 256) { s.store_field("title", title_); }
    if (var0 & 512) { s.store_object_field("subscription_pricing", static_cast<const BaseObject *>(subscription_pricing_.get())); }
    s.store_class_end();
  }
}

const std::int32_t chatInvitePublicJoinRequests::ID;

object_ptr<ExportedChatInvite> chatInvitePublicJoinRequests::fetch(TlBufferParser &p) {
  return make_tl_object<chatInvitePublicJoinRequests>();
}

void chatInvitePublicJoinRequests::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInvitePublicJoinRequests");
    s.store_class_end();
  }
}

const std::int32_t exportedContactToken::ID;

object_ptr<exportedContactToken> exportedContactToken::fetch(TlBufferParser &p) {
  return make_tl_object<exportedContactToken>(p);
}

exportedContactToken::exportedContactToken(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , expires_(TlFetchInt::parse(p))
{}

void exportedContactToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "exportedContactToken");
    s.store_field("url", url_);
    s.store_field("expires", expires_);
    s.store_class_end();
  }
}

const std::int32_t exportedMessageLink::ID;

object_ptr<exportedMessageLink> exportedMessageLink::fetch(TlBufferParser &p) {
  return make_tl_object<exportedMessageLink>(p);
}

exportedMessageLink::exportedMessageLink(TlBufferParser &p)
  : link_(TlFetchString<string>::parse(p))
  , html_(TlFetchString<string>::parse(p))
{}

void exportedMessageLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "exportedMessageLink");
    s.store_field("link", link_);
    s.store_field("html", html_);
    s.store_class_end();
  }
}

const std::int32_t fileHash::ID;

object_ptr<fileHash> fileHash::fetch(TlBufferParser &p) {
  return make_tl_object<fileHash>(p);
}

fileHash::fileHash(TlBufferParser &p)
  : offset_(TlFetchLong::parse(p))
  , limit_(TlFetchInt::parse(p))
  , hash_(TlFetchBytes<bytes>::parse(p))
{}

void fileHash::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileHash");
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_bytes_field("hash", hash_);
    s.store_class_end();
  }
}

object_ptr<ForumTopic> ForumTopic::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case forumTopicDeleted::ID:
      return forumTopicDeleted::fetch(p);
    case forumTopic::ID:
      return forumTopic::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t forumTopicDeleted::ID;

object_ptr<ForumTopic> forumTopicDeleted::fetch(TlBufferParser &p) {
  return make_tl_object<forumTopicDeleted>(p);
}

forumTopicDeleted::forumTopicDeleted(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
{}

void forumTopicDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forumTopicDeleted");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

forumTopic::forumTopic()
  : flags_()
  , my_()
  , closed_()
  , pinned_()
  , short_()
  , hidden_()
  , title_missing_()
  , id_()
  , date_()
  , peer_()
  , title_()
  , icon_color_()
  , icon_emoji_id_()
  , top_message_()
  , read_inbox_max_id_()
  , read_outbox_max_id_()
  , unread_count_()
  , unread_mentions_count_()
  , unread_reactions_count_()
  , from_id_()
  , notify_settings_()
  , draft_()
{}

const std::int32_t forumTopic::ID;

object_ptr<ForumTopic> forumTopic::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<forumTopic> res = make_tl_object<forumTopic>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->my_ = (var0 & 2) != 0;
  res->closed_ = (var0 & 4) != 0;
  res->pinned_ = (var0 & 8) != 0;
  res->short_ = (var0 & 32) != 0;
  res->hidden_ = (var0 & 64) != 0;
  res->title_missing_ = (var0 & 128) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->icon_color_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->icon_emoji_id_ = TlFetchLong::parse(p); }
  res->top_message_ = TlFetchInt::parse(p);
  res->read_inbox_max_id_ = TlFetchInt::parse(p);
  res->read_outbox_max_id_ = TlFetchInt::parse(p);
  res->unread_count_ = TlFetchInt::parse(p);
  res->unread_mentions_count_ = TlFetchInt::parse(p);
  res->unread_reactions_count_ = TlFetchInt::parse(p);
  res->from_id_ = TlFetchObject<Peer>::parse(p);
  res->notify_settings_ = TlFetchBoxed<TlFetchObject<peerNotifySettings>, -1721619444>::parse(p);
  if (var0 & 16) { res->draft_ = TlFetchObject<DraftMessage>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void forumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forumTopic");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (my_ << 1) | (closed_ << 2) | (pinned_ << 3) | (short_ << 5) | (hidden_ << 6) | (title_missing_ << 7)));
    if (var0 & 2) { s.store_field("my", true); }
    if (var0 & 4) { s.store_field("closed", true); }
    if (var0 & 8) { s.store_field("pinned", true); }
    if (var0 & 32) { s.store_field("short", true); }
    if (var0 & 64) { s.store_field("hidden", true); }
    if (var0 & 128) { s.store_field("title_missing", true); }
    s.store_field("id", id_);
    s.store_field("date", date_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("title", title_);
    s.store_field("icon_color", icon_color_);
    if (var0 & 1) { s.store_field("icon_emoji_id", icon_emoji_id_); }
    s.store_field("top_message", top_message_);
    s.store_field("read_inbox_max_id", read_inbox_max_id_);
    s.store_field("read_outbox_max_id", read_outbox_max_id_);
    s.store_field("unread_count", unread_count_);
    s.store_field("unread_mentions_count", unread_mentions_count_);
    s.store_field("unread_reactions_count", unread_reactions_count_);
    s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get()));
    s.store_object_field("notify_settings", static_cast<const BaseObject *>(notify_settings_.get()));
    if (var0 & 16) { s.store_object_field("draft", static_cast<const BaseObject *>(draft_.get())); }
    s.store_class_end();
  }
}

game::game()
  : flags_()
  , id_()
  , access_hash_()
  , short_name_()
  , title_()
  , description_()
  , photo_()
  , document_()
{}

const std::int32_t game::ID;

object_ptr<game> game::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<game> res = make_tl_object<game>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->short_name_ = TlFetchString<string>::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  res->photo_ = TlFetchObject<Photo>::parse(p);
  if (var0 & 1) { res->document_ = TlFetchObject<Document>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void game::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "game");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("short_name", short_name_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    if (var0 & 1) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    s.store_class_end();
  }
}

const std::int32_t groupCallStreamChannel::ID;

object_ptr<groupCallStreamChannel> groupCallStreamChannel::fetch(TlBufferParser &p) {
  return make_tl_object<groupCallStreamChannel>(p);
}

groupCallStreamChannel::groupCallStreamChannel(TlBufferParser &p)
  : channel_(TlFetchInt::parse(p))
  , scale_(TlFetchInt::parse(p))
  , last_timestamp_ms_(TlFetchLong::parse(p))
{}

void groupCallStreamChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallStreamChannel");
    s.store_field("channel", channel_);
    s.store_field("scale", scale_);
    s.store_field("last_timestamp_ms", last_timestamp_ms_);
    s.store_class_end();
  }
}

inputBotAppID::inputBotAppID(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputBotAppID::ID;

void inputBotAppID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputBotAppID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputBotAppID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotAppID");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputBotAppShortName::inputBotAppShortName(object_ptr<InputUser> &&bot_id_, string const &short_name_)
  : bot_id_(std::move(bot_id_))
  , short_name_(short_name_)
{}

const std::int32_t inputBotAppShortName::ID;

void inputBotAppShortName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_id_, s);
  TlStoreString::store(short_name_, s);
}

void inputBotAppShortName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_id_, s);
  TlStoreString::store(short_name_, s);
}

void inputBotAppShortName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotAppShortName");
    s.store_object_field("bot_id", static_cast<const BaseObject *>(bot_id_.get()));
    s.store_field("short_name", short_name_);
    s.store_class_end();
  }
}

inputBusinessRecipients::inputBusinessRecipients(int32 flags_, bool existing_chats_, bool new_chats_, bool contacts_, bool non_contacts_, bool exclude_selected_, array<object_ptr<InputUser>> &&users_)
  : flags_(flags_)
  , existing_chats_(existing_chats_)
  , new_chats_(new_chats_)
  , contacts_(contacts_)
  , non_contacts_(non_contacts_)
  , exclude_selected_(exclude_selected_)
  , users_(std::move(users_))
{}

const std::int32_t inputBusinessRecipients::ID;

void inputBusinessRecipients::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)), s);
  if (var0 & 16) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s); }
}

void inputBusinessRecipients::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)), s);
  if (var0 & 16) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s); }
}

void inputBusinessRecipients::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessRecipients");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)));
    if (var0 & 1) { s.store_field("existing_chats", true); }
    if (var0 & 2) { s.store_field("new_chats", true); }
    if (var0 & 4) { s.store_field("contacts", true); }
    if (var0 & 8) { s.store_field("non_contacts", true); }
    if (var0 & 32) { s.store_field("exclude_selected", true); }
    if (var0 & 16) { { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

const std::int32_t inputCheckPasswordEmpty::ID;

void inputCheckPasswordEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputCheckPasswordEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputCheckPasswordEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCheckPasswordEmpty");
    s.store_class_end();
  }
}

inputCheckPasswordSRP::inputCheckPasswordSRP(int64 srp_id_, bytes &&A_, bytes &&M1_)
  : srp_id_(srp_id_)
  , A_(std::move(A_))
  , M1_(std::move(M1_))
{}

const std::int32_t inputCheckPasswordSRP::ID;

void inputCheckPasswordSRP::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(srp_id_, s);
  TlStoreString::store(A_, s);
  TlStoreString::store(M1_, s);
}

void inputCheckPasswordSRP::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(srp_id_, s);
  TlStoreString::store(A_, s);
  TlStoreString::store(M1_, s);
}

void inputCheckPasswordSRP::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCheckPasswordSRP");
    s.store_field("srp_id", srp_id_);
    s.store_bytes_field("A", A_);
    s.store_bytes_field("M1", M1_);
    s.store_class_end();
  }
}

inputPhoneContact::inputPhoneContact(int32 flags_, int64 client_id_, string const &phone_, string const &first_name_, string const &last_name_, object_ptr<textWithEntities> &&note_)
  : flags_(flags_)
  , client_id_(client_id_)
  , phone_(phone_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , note_(std::move(note_))
{}

const std::int32_t inputPhoneContact::ID;

void inputPhoneContact::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(client_id_, s);
  TlStoreString::store(phone_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 1964978502>::store(note_, s); }
}

void inputPhoneContact::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(client_id_, s);
  TlStoreString::store(phone_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 1964978502>::store(note_, s); }
}

void inputPhoneContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPhoneContact");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("client_id", client_id_);
    s.store_field("phone", phone_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    if (var0 & 1) { s.store_object_field("note", static_cast<const BaseObject *>(note_.get())); }
    s.store_class_end();
  }
}

inputWallPaper::inputWallPaper(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputWallPaper::ID;

void inputWallPaper::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputWallPaper::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputWallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputWallPaper");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputWallPaperSlug::inputWallPaperSlug(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t inputWallPaperSlug::ID;

void inputWallPaperSlug::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputWallPaperSlug::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputWallPaperSlug::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputWallPaperSlug");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

inputWallPaperNoFile::inputWallPaperNoFile(int64 id_)
  : id_(id_)
{}

const std::int32_t inputWallPaperNoFile::ID;

void inputWallPaperNoFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void inputWallPaperNoFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void inputWallPaperNoFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputWallPaperNoFile");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

jsonObjectValue::jsonObjectValue(string const &key_, object_ptr<JSONValue> &&value_)
  : key_(key_)
  , value_(std::move(value_))
{}

const std::int32_t jsonObjectValue::ID;

object_ptr<jsonObjectValue> jsonObjectValue::fetch(TlBufferParser &p) {
  return make_tl_object<jsonObjectValue>(p);
}

jsonObjectValue::jsonObjectValue(TlBufferParser &p)
  : key_(TlFetchString<string>::parse(p))
  , value_(TlFetchObject<JSONValue>::parse(p))
{}

void jsonObjectValue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(key_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(value_, s);
}

void jsonObjectValue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(key_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(value_, s);
}

void jsonObjectValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonObjectValue");
    s.store_field("key", key_);
    s.store_object_field("value", static_cast<const BaseObject *>(value_.get()));
    s.store_class_end();
  }
}

labeledPrice::labeledPrice(string const &label_, int64 amount_)
  : label_(label_)
  , amount_(amount_)
{}

const std::int32_t labeledPrice::ID;

object_ptr<labeledPrice> labeledPrice::fetch(TlBufferParser &p) {
  return make_tl_object<labeledPrice>(p);
}

labeledPrice::labeledPrice(TlBufferParser &p)
  : label_(TlFetchString<string>::parse(p))
  , amount_(TlFetchLong::parse(p))
{}

void labeledPrice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(label_, s);
  TlStoreBinary::store(amount_, s);
}

void labeledPrice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(label_, s);
  TlStoreBinary::store(amount_, s);
}

void labeledPrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "labeledPrice");
    s.store_field("label", label_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

object_ptr<LangPackString> LangPackString::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case langPackString::ID:
      return langPackString::fetch(p);
    case langPackStringPluralized::ID:
      return langPackStringPluralized::fetch(p);
    case langPackStringDeleted::ID:
      return langPackStringDeleted::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

langPackString::langPackString(string const &key_, string const &value_)
  : key_(key_)
  , value_(value_)
{}

const std::int32_t langPackString::ID;

object_ptr<LangPackString> langPackString::fetch(TlBufferParser &p) {
  return make_tl_object<langPackString>(p);
}

langPackString::langPackString(TlBufferParser &p)
  : key_(TlFetchString<string>::parse(p))
  , value_(TlFetchString<string>::parse(p))
{}

void langPackString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langPackString");
    s.store_field("key", key_);
    s.store_field("value", value_);
    s.store_class_end();
  }
}

langPackStringPluralized::langPackStringPluralized()
  : flags_()
  , key_()
  , zero_value_()
  , one_value_()
  , two_value_()
  , few_value_()
  , many_value_()
  , other_value_()
{}

langPackStringPluralized::langPackStringPluralized(int32 flags_, string const &key_, string const &zero_value_, string const &one_value_, string const &two_value_, string const &few_value_, string const &many_value_, string const &other_value_)
  : flags_(flags_)
  , key_(key_)
  , zero_value_(zero_value_)
  , one_value_(one_value_)
  , two_value_(two_value_)
  , few_value_(few_value_)
  , many_value_(many_value_)
  , other_value_(other_value_)
{}

const std::int32_t langPackStringPluralized::ID;

object_ptr<LangPackString> langPackStringPluralized::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<langPackStringPluralized> res = make_tl_object<langPackStringPluralized>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->key_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->zero_value_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->one_value_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->two_value_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->few_value_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->many_value_ = TlFetchString<string>::parse(p); }
  res->other_value_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void langPackStringPluralized::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langPackStringPluralized");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("key", key_);
    if (var0 & 1) { s.store_field("zero_value", zero_value_); }
    if (var0 & 2) { s.store_field("one_value", one_value_); }
    if (var0 & 4) { s.store_field("two_value", two_value_); }
    if (var0 & 8) { s.store_field("few_value", few_value_); }
    if (var0 & 16) { s.store_field("many_value", many_value_); }
    s.store_field("other_value", other_value_);
    s.store_class_end();
  }
}

langPackStringDeleted::langPackStringDeleted(string const &key_)
  : key_(key_)
{}

const std::int32_t langPackStringDeleted::ID;

object_ptr<LangPackString> langPackStringDeleted::fetch(TlBufferParser &p) {
  return make_tl_object<langPackStringDeleted>(p);
}

langPackStringDeleted::langPackStringDeleted(TlBufferParser &p)
  : key_(TlFetchString<string>::parse(p))
{}

void langPackStringDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langPackStringDeleted");
    s.store_field("key", key_);
    s.store_class_end();
  }
}

object_ptr<PeerLocated> PeerLocated::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case peerLocated::ID:
      return peerLocated::fetch(p);
    case peerSelfLocated::ID:
      return peerSelfLocated::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t peerLocated::ID;

object_ptr<PeerLocated> peerLocated::fetch(TlBufferParser &p) {
  return make_tl_object<peerLocated>(p);
}

peerLocated::peerLocated(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , expires_(TlFetchInt::parse(p))
  , distance_(TlFetchInt::parse(p))
{}

void peerLocated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerLocated");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("expires", expires_);
    s.store_field("distance", distance_);
    s.store_class_end();
  }
}

const std::int32_t peerSelfLocated::ID;

object_ptr<PeerLocated> peerSelfLocated::fetch(TlBufferParser &p) {
  return make_tl_object<peerSelfLocated>(p);
}

peerSelfLocated::peerSelfLocated(TlBufferParser &p)
  : expires_(TlFetchInt::parse(p))
{}

void peerSelfLocated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerSelfLocated");
    s.store_field("expires", expires_);
    s.store_class_end();
  }
}

object_ptr<PhoneCall> PhoneCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case phoneCallEmpty::ID:
      return phoneCallEmpty::fetch(p);
    case phoneCallWaiting::ID:
      return phoneCallWaiting::fetch(p);
    case phoneCallRequested::ID:
      return phoneCallRequested::fetch(p);
    case phoneCallAccepted::ID:
      return phoneCallAccepted::fetch(p);
    case phoneCall::ID:
      return phoneCall::fetch(p);
    case phoneCallDiscarded::ID:
      return phoneCallDiscarded::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t phoneCallEmpty::ID;

object_ptr<PhoneCall> phoneCallEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<phoneCallEmpty>(p);
}

phoneCallEmpty::phoneCallEmpty(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void phoneCallEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallEmpty");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

phoneCallWaiting::phoneCallWaiting()
  : flags_()
  , video_()
  , id_()
  , access_hash_()
  , date_()
  , admin_id_()
  , participant_id_()
  , protocol_()
  , receive_date_()
{}

const std::int32_t phoneCallWaiting::ID;

object_ptr<PhoneCall> phoneCallWaiting::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneCallWaiting> res = make_tl_object<phoneCallWaiting>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->video_ = (var0 & 64) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->admin_id_ = TlFetchLong::parse(p);
  res->participant_id_ = TlFetchLong::parse(p);
  res->protocol_ = TlFetchBoxed<TlFetchObject<phoneCallProtocol>, -58224696>::parse(p);
  if (var0 & 1) { res->receive_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void phoneCallWaiting::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallWaiting");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (video_ << 6)));
    if (var0 & 64) { s.store_field("video", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("admin_id", admin_id_);
    s.store_field("participant_id", participant_id_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    if (var0 & 1) { s.store_field("receive_date", receive_date_); }
    s.store_class_end();
  }
}

phoneCallRequested::phoneCallRequested()
  : flags_()
  , video_()
  , id_()
  , access_hash_()
  , date_()
  , admin_id_()
  , participant_id_()
  , g_a_hash_()
  , protocol_()
{}

const std::int32_t phoneCallRequested::ID;

object_ptr<PhoneCall> phoneCallRequested::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneCallRequested> res = make_tl_object<phoneCallRequested>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->video_ = (var0 & 64) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->admin_id_ = TlFetchLong::parse(p);
  res->participant_id_ = TlFetchLong::parse(p);
  res->g_a_hash_ = TlFetchBytes<bytes>::parse(p);
  res->protocol_ = TlFetchBoxed<TlFetchObject<phoneCallProtocol>, -58224696>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void phoneCallRequested::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallRequested");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (video_ << 6)));
    if (var0 & 64) { s.store_field("video", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("admin_id", admin_id_);
    s.store_field("participant_id", participant_id_);
    s.store_bytes_field("g_a_hash", g_a_hash_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    s.store_class_end();
  }
}

phoneCallAccepted::phoneCallAccepted()
  : flags_()
  , video_()
  , id_()
  , access_hash_()
  , date_()
  , admin_id_()
  , participant_id_()
  , g_b_()
  , protocol_()
{}

const std::int32_t phoneCallAccepted::ID;

object_ptr<PhoneCall> phoneCallAccepted::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneCallAccepted> res = make_tl_object<phoneCallAccepted>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->video_ = (var0 & 64) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->admin_id_ = TlFetchLong::parse(p);
  res->participant_id_ = TlFetchLong::parse(p);
  res->g_b_ = TlFetchBytes<bytes>::parse(p);
  res->protocol_ = TlFetchBoxed<TlFetchObject<phoneCallProtocol>, -58224696>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void phoneCallAccepted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallAccepted");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (video_ << 6)));
    if (var0 & 64) { s.store_field("video", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("admin_id", admin_id_);
    s.store_field("participant_id", participant_id_);
    s.store_bytes_field("g_b", g_b_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    s.store_class_end();
  }
}

phoneCall::phoneCall()
  : flags_()
  , p2p_allowed_()
  , video_()
  , conference_supported_()
  , id_()
  , access_hash_()
  , date_()
  , admin_id_()
  , participant_id_()
  , g_a_or_b_()
  , key_fingerprint_()
  , protocol_()
  , connections_()
  , start_date_()
  , custom_parameters_()
{}

const std::int32_t phoneCall::ID;

object_ptr<PhoneCall> phoneCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneCall> res = make_tl_object<phoneCall>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->p2p_allowed_ = (var0 & 32) != 0;
  res->video_ = (var0 & 64) != 0;
  res->conference_supported_ = (var0 & 256) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->admin_id_ = TlFetchLong::parse(p);
  res->participant_id_ = TlFetchLong::parse(p);
  res->g_a_or_b_ = TlFetchBytes<bytes>::parse(p);
  res->key_fingerprint_ = TlFetchLong::parse(p);
  res->protocol_ = TlFetchBoxed<TlFetchObject<phoneCallProtocol>, -58224696>::parse(p);
  res->connections_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PhoneConnection>>, 481674261>::parse(p);
  res->start_date_ = TlFetchInt::parse(p);
  if (var0 & 128) { res->custom_parameters_ = TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void phoneCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCall");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (p2p_allowed_ << 5) | (video_ << 6) | (conference_supported_ << 8)));
    if (var0 & 32) { s.store_field("p2p_allowed", true); }
    if (var0 & 64) { s.store_field("video", true); }
    if (var0 & 256) { s.store_field("conference_supported", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("admin_id", admin_id_);
    s.store_field("participant_id", participant_id_);
    s.store_bytes_field("g_a_or_b", g_a_or_b_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    { s.store_vector_begin("connections", connections_.size()); for (const auto &_value : connections_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("start_date", start_date_);
    if (var0 & 128) { s.store_object_field("custom_parameters", static_cast<const BaseObject *>(custom_parameters_.get())); }
    s.store_class_end();
  }
}

phoneCallDiscarded::phoneCallDiscarded()
  : flags_()
  , need_rating_()
  , need_debug_()
  , video_()
  , id_()
  , reason_()
  , duration_()
{}

const std::int32_t phoneCallDiscarded::ID;

object_ptr<PhoneCall> phoneCallDiscarded::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<phoneCallDiscarded> res = make_tl_object<phoneCallDiscarded>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->need_rating_ = (var0 & 4) != 0;
  res->need_debug_ = (var0 & 8) != 0;
  res->video_ = (var0 & 64) != 0;
  res->id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->reason_ = TlFetchObject<PhoneCallDiscardReason>::parse(p); }
  if (var0 & 2) { res->duration_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void phoneCallDiscarded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallDiscarded");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (need_rating_ << 2) | (need_debug_ << 3) | (video_ << 6)));
    if (var0 & 4) { s.store_field("need_rating", true); }
    if (var0 & 8) { s.store_field("need_debug", true); }
    if (var0 & 64) { s.store_field("video", true); }
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get())); }
    if (var0 & 2) { s.store_field("duration", duration_); }
    s.store_class_end();
  }
}

object_ptr<PublicForward> PublicForward::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case publicForwardMessage::ID:
      return publicForwardMessage::fetch(p);
    case publicForwardStory::ID:
      return publicForwardStory::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t publicForwardMessage::ID;

object_ptr<PublicForward> publicForwardMessage::fetch(TlBufferParser &p) {
  return make_tl_object<publicForwardMessage>(p);
}

publicForwardMessage::publicForwardMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void publicForwardMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicForwardMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t publicForwardStory::ID;

object_ptr<PublicForward> publicForwardStory::fetch(TlBufferParser &p) {
  return make_tl_object<publicForwardStory>(p);
}

publicForwardStory::publicForwardStory(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , story_(TlFetchObject<StoryItem>::parse(p))
{}

void publicForwardStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicForwardStory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

reactionCount::reactionCount()
  : flags_()
  , chosen_order_()
  , reaction_()
  , count_()
{}

const std::int32_t reactionCount::ID;

object_ptr<reactionCount> reactionCount::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<reactionCount> res = make_tl_object<reactionCount>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->chosen_order_ = TlFetchInt::parse(p); }
  res->reaction_ = TlFetchObject<Reaction>::parse(p);
  res->count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void reactionCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionCount");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("chosen_order", chosen_order_); }
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_field("count", count_);
    s.store_class_end();
  }
}

object_ptr<RecentMeUrl> RecentMeUrl::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case recentMeUrlUnknown::ID:
      return recentMeUrlUnknown::fetch(p);
    case recentMeUrlUser::ID:
      return recentMeUrlUser::fetch(p);
    case recentMeUrlChat::ID:
      return recentMeUrlChat::fetch(p);
    case recentMeUrlChatInvite::ID:
      return recentMeUrlChatInvite::fetch(p);
    case recentMeUrlStickerSet::ID:
      return recentMeUrlStickerSet::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t recentMeUrlUnknown::ID;

object_ptr<RecentMeUrl> recentMeUrlUnknown::fetch(TlBufferParser &p) {
  return make_tl_object<recentMeUrlUnknown>(p);
}

recentMeUrlUnknown::recentMeUrlUnknown(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void recentMeUrlUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recentMeUrlUnknown");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

const std::int32_t recentMeUrlUser::ID;

object_ptr<RecentMeUrl> recentMeUrlUser::fetch(TlBufferParser &p) {
  return make_tl_object<recentMeUrlUser>(p);
}

recentMeUrlUser::recentMeUrlUser(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , user_id_(TlFetchLong::parse(p))
{}

void recentMeUrlUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recentMeUrlUser");
    s.store_field("url", url_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

const std::int32_t recentMeUrlChat::ID;

object_ptr<RecentMeUrl> recentMeUrlChat::fetch(TlBufferParser &p) {
  return make_tl_object<recentMeUrlChat>(p);
}

recentMeUrlChat::recentMeUrlChat(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , chat_id_(TlFetchLong::parse(p))
{}

void recentMeUrlChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recentMeUrlChat");
    s.store_field("url", url_);
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

const std::int32_t recentMeUrlChatInvite::ID;

object_ptr<RecentMeUrl> recentMeUrlChatInvite::fetch(TlBufferParser &p) {
  return make_tl_object<recentMeUrlChatInvite>(p);
}

recentMeUrlChatInvite::recentMeUrlChatInvite(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , chat_invite_(TlFetchObject<ChatInvite>::parse(p))
{}

void recentMeUrlChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recentMeUrlChatInvite");
    s.store_field("url", url_);
    s.store_object_field("chat_invite", static_cast<const BaseObject *>(chat_invite_.get()));
    s.store_class_end();
  }
}

const std::int32_t recentMeUrlStickerSet::ID;

object_ptr<RecentMeUrl> recentMeUrlStickerSet::fetch(TlBufferParser &p) {
  return make_tl_object<recentMeUrlStickerSet>(p);
}

recentMeUrlStickerSet::recentMeUrlStickerSet(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , set_(TlFetchObject<StickerSetCovered>::parse(p))
{}

void recentMeUrlStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recentMeUrlStickerSet");
    s.store_field("url", url_);
    s.store_object_field("set", static_cast<const BaseObject *>(set_.get()));
    s.store_class_end();
  }
}

const std::int32_t smsJob::ID;

object_ptr<smsJob> smsJob::fetch(TlBufferParser &p) {
  return make_tl_object<smsJob>(p);
}

smsJob::smsJob(TlBufferParser &p)
  : job_id_(TlFetchString<string>::parse(p))
  , phone_number_(TlFetchString<string>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void smsJob::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsJob");
    s.store_field("job_id", job_id_);
    s.store_field("phone_number", phone_number_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

starsGiftOption::starsGiftOption()
  : flags_()
  , extended_()
  , stars_()
  , store_product_()
  , currency_()
  , amount_()
{}

const std::int32_t starsGiftOption::ID;

object_ptr<starsGiftOption> starsGiftOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsGiftOption> res = make_tl_object<starsGiftOption>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->extended_ = (var0 & 2) != 0;
  res->stars_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->store_product_ = TlFetchString<string>::parse(p); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starsGiftOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsGiftOption");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (extended_ << 1)));
    if (var0 & 2) { s.store_field("extended", true); }
    s.store_field("stars", stars_);
    if (var0 & 1) { s.store_field("store_product", store_product_); }
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryPeers::ID;

object_ptr<topPeerCategoryPeers> topPeerCategoryPeers::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryPeers>(p);
}

topPeerCategoryPeers::topPeerCategoryPeers(TlBufferParser &p)
  : category_(TlFetchObject<TopPeerCategory>::parse(p))
  , count_(TlFetchInt::parse(p))
  , peers_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<topPeer>, -305282981>>, 481674261>::parse(p))
{}

void topPeerCategoryPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryPeers");
    s.store_object_field("category", static_cast<const BaseObject *>(category_.get()));
    s.store_field("count", count_);
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

userFull::userFull()
  : flags_()
  , blocked_()
  , phone_calls_available_()
  , phone_calls_private_()
  , can_pin_message_()
  , has_scheduled_()
  , video_calls_available_()
  , voice_messages_forbidden_()
  , translations_disabled_()
  , stories_pinned_available_()
  , blocked_my_stories_from_()
  , wallpaper_overridden_()
  , contact_require_premium_()
  , read_dates_private_()
  , flags2_()
  , sponsored_enabled_()
  , can_view_revenue_()
  , bot_can_manage_emoji_status_()
  , display_gifts_button_()
  , id_()
  , about_()
  , settings_()
  , personal_photo_()
  , profile_photo_()
  , fallback_photo_()
  , notify_settings_()
  , bot_info_()
  , pinned_msg_id_()
  , common_chats_count_()
  , folder_id_()
  , ttl_period_()
  , theme_()
  , private_forward_name_()
  , bot_group_admin_rights_()
  , bot_broadcast_admin_rights_()
  , wallpaper_()
  , stories_()
  , business_work_hours_()
  , business_location_()
  , business_greeting_message_()
  , business_away_message_()
  , business_intro_()
  , birthday_()
  , personal_channel_id_()
  , personal_channel_message_()
  , stargifts_count_()
  , starref_program_()
  , bot_verification_()
  , send_paid_messages_stars_()
  , disallowed_gifts_()
  , stars_rating_()
  , stars_my_pending_rating_()
  , stars_my_pending_rating_date_()
  , main_tab_()
  , saved_music_()
  , note_()
{}

const std::int32_t userFull::ID;

object_ptr<userFull> userFull::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<userFull> res = make_tl_object<userFull>();
  int32 var0;
  int32 var1;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->blocked_ = (var0 & 1) != 0;
  res->phone_calls_available_ = (var0 & 16) != 0;
  res->phone_calls_private_ = (var0 & 32) != 0;
  res->can_pin_message_ = (var0 & 128) != 0;
  res->has_scheduled_ = (var0 & 4096) != 0;
  res->video_calls_available_ = (var0 & 8192) != 0;
  res->voice_messages_forbidden_ = (var0 & 1048576) != 0;
  res->translations_disabled_ = (var0 & 8388608) != 0;
  res->stories_pinned_available_ = (var0 & 67108864) != 0;
  res->blocked_my_stories_from_ = (var0 & 134217728) != 0;
  res->wallpaper_overridden_ = (var0 & 268435456) != 0;
  res->contact_require_premium_ = (var0 & 536870912) != 0;
  res->read_dates_private_ = (var0 & 1073741824) != 0;
  if ((var1 = res->flags2_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->sponsored_enabled_ = (var1 & 128) != 0;
  res->can_view_revenue_ = (var1 & 512) != 0;
  res->bot_can_manage_emoji_status_ = (var1 & 1024) != 0;
  res->display_gifts_button_ = (var1 & 65536) != 0;
  res->id_ = TlFetchLong::parse(p);
  if (var0 & 2) { res->about_ = TlFetchString<string>::parse(p); }
  res->settings_ = TlFetchBoxed<TlFetchObject<peerSettings>, -193510921>::parse(p);
  if (var0 & 2097152) { res->personal_photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 4) { res->profile_photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 4194304) { res->fallback_photo_ = TlFetchObject<Photo>::parse(p); }
  res->notify_settings_ = TlFetchBoxed<TlFetchObject<peerNotifySettings>, -1721619444>::parse(p);
  if (var0 & 8) { res->bot_info_ = TlFetchBoxed<TlFetchObject<botInfo>, 1300890265>::parse(p); }
  if (var0 & 64) { res->pinned_msg_id_ = TlFetchInt::parse(p); }
  res->common_chats_count_ = TlFetchInt::parse(p);
  if (var0 & 2048) { res->folder_id_ = TlFetchInt::parse(p); }
  if (var0 & 16384) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (var0 & 32768) { res->theme_ = TlFetchObject<ChatTheme>::parse(p); }
  if (var0 & 65536) { res->private_forward_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 131072) { res->bot_group_admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (var0 & 262144) { res->bot_broadcast_admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (var0 & 16777216) { res->wallpaper_ = TlFetchObject<WallPaper>::parse(p); }
  if (var0 & 33554432) { res->stories_ = TlFetchBoxed<TlFetchObject<peerStories>, -1707742823>::parse(p); }
  if (var1 & 1) { res->business_work_hours_ = TlFetchBoxed<TlFetchObject<businessWorkHours>, -1936543592>::parse(p); }
  if (var1 & 2) { res->business_location_ = TlFetchBoxed<TlFetchObject<businessLocation>, -1403249929>::parse(p); }
  if (var1 & 4) { res->business_greeting_message_ = TlFetchBoxed<TlFetchObject<businessGreetingMessage>, -451302485>::parse(p); }
  if (var1 & 8) { res->business_away_message_ = TlFetchBoxed<TlFetchObject<businessAwayMessage>, -283809188>::parse(p); }
  if (var1 & 16) { res->business_intro_ = TlFetchBoxed<TlFetchObject<businessIntro>, 1510606445>::parse(p); }
  if (var1 & 32) { res->birthday_ = TlFetchBoxed<TlFetchObject<birthday>, 1821253126>::parse(p); }
  if (var1 & 64) { res->personal_channel_id_ = TlFetchLong::parse(p); }
  if (var1 & 64) { res->personal_channel_message_ = TlFetchInt::parse(p); }
  if (var1 & 256) { res->stargifts_count_ = TlFetchInt::parse(p); }
  if (var1 & 2048) { res->starref_program_ = TlFetchBoxed<TlFetchObject<starRefProgram>, -586389774>::parse(p); }
  if (var1 & 4096) { res->bot_verification_ = TlFetchBoxed<TlFetchObject<botVerification>, -113453988>::parse(p); }
  if (var1 & 16384) { res->send_paid_messages_stars_ = TlFetchLong::parse(p); }
  if (var1 & 32768) { res->disallowed_gifts_ = TlFetchBoxed<TlFetchObject<disallowedGiftsSettings>, 1911715524>::parse(p); }
  if (var1 & 131072) { res->stars_rating_ = TlFetchBoxed<TlFetchObject<starsRating>, 453922567>::parse(p); }
  if (var1 & 262144) { res->stars_my_pending_rating_ = TlFetchBoxed<TlFetchObject<starsRating>, 453922567>::parse(p); }
  if (var1 & 262144) { res->stars_my_pending_rating_date_ = TlFetchInt::parse(p); }
  if (var1 & 1048576) { res->main_tab_ = TlFetchObject<ProfileTab>::parse(p); }
  if (var1 & 2097152) { res->saved_music_ = TlFetchObject<Document>::parse(p); }
  if (var1 & 4194304) { res->note_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void userFull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userFull");
  int32 var0;
  int32 var1;
    s.store_field("flags", (var0 = flags_ | (blocked_ << 0) | (phone_calls_available_ << 4) | (phone_calls_private_ << 5) | (can_pin_message_ << 7) | (has_scheduled_ << 12) | (video_calls_available_ << 13) | (voice_messages_forbidden_ << 20) | (translations_disabled_ << 23) | (stories_pinned_available_ << 26) | (blocked_my_stories_from_ << 27) | (wallpaper_overridden_ << 28) | (contact_require_premium_ << 29) | (read_dates_private_ << 30)));
    if (var0 & 1) { s.store_field("blocked", true); }
    if (var0 & 16) { s.store_field("phone_calls_available", true); }
    if (var0 & 32) { s.store_field("phone_calls_private", true); }
    if (var0 & 128) { s.store_field("can_pin_message", true); }
    if (var0 & 4096) { s.store_field("has_scheduled", true); }
    if (var0 & 8192) { s.store_field("video_calls_available", true); }
    if (var0 & 1048576) { s.store_field("voice_messages_forbidden", true); }
    if (var0 & 8388608) { s.store_field("translations_disabled", true); }
    if (var0 & 67108864) { s.store_field("stories_pinned_available", true); }
    if (var0 & 134217728) { s.store_field("blocked_my_stories_from", true); }
    if (var0 & 268435456) { s.store_field("wallpaper_overridden", true); }
    if (var0 & 536870912) { s.store_field("contact_require_premium", true); }
    if (var0 & 1073741824) { s.store_field("read_dates_private", true); }
    s.store_field("flags2", (var1 = flags2_ | (sponsored_enabled_ << 7) | (can_view_revenue_ << 9) | (bot_can_manage_emoji_status_ << 10) | (display_gifts_button_ << 16)));
    if (var1 & 128) { s.store_field("sponsored_enabled", true); }
    if (var1 & 512) { s.store_field("can_view_revenue", true); }
    if (var1 & 1024) { s.store_field("bot_can_manage_emoji_status", true); }
    if (var1 & 65536) { s.store_field("display_gifts_button", true); }
    s.store_field("id", id_);
    if (var0 & 2) { s.store_field("about", about_); }
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    if (var0 & 2097152) { s.store_object_field("personal_photo", static_cast<const BaseObject *>(personal_photo_.get())); }
    if (var0 & 4) { s.store_object_field("profile_photo", static_cast<const BaseObject *>(profile_photo_.get())); }
    if (var0 & 4194304) { s.store_object_field("fallback_photo", static_cast<const BaseObject *>(fallback_photo_.get())); }
    s.store_object_field("notify_settings", static_cast<const BaseObject *>(notify_settings_.get()));
    if (var0 & 8) { s.store_object_field("bot_info", static_cast<const BaseObject *>(bot_info_.get())); }
    if (var0 & 64) { s.store_field("pinned_msg_id", pinned_msg_id_); }
    s.store_field("common_chats_count", common_chats_count_);
    if (var0 & 2048) { s.store_field("folder_id", folder_id_); }
    if (var0 & 16384) { s.store_field("ttl_period", ttl_period_); }
    if (var0 & 32768) { s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get())); }
    if (var0 & 65536) { s.store_field("private_forward_name", private_forward_name_); }
    if (var0 & 131072) { s.store_object_field("bot_group_admin_rights", static_cast<const BaseObject *>(bot_group_admin_rights_.get())); }
    if (var0 & 262144) { s.store_object_field("bot_broadcast_admin_rights", static_cast<const BaseObject *>(bot_broadcast_admin_rights_.get())); }
    if (var0 & 16777216) { s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get())); }
    if (var0 & 33554432) { s.store_object_field("stories", static_cast<const BaseObject *>(stories_.get())); }
    if (var1 & 1) { s.store_object_field("business_work_hours", static_cast<const BaseObject *>(business_work_hours_.get())); }
    if (var1 & 2) { s.store_object_field("business_location", static_cast<const BaseObject *>(business_location_.get())); }
    if (var1 & 4) { s.store_object_field("business_greeting_message", static_cast<const BaseObject *>(business_greeting_message_.get())); }
    if (var1 & 8) { s.store_object_field("business_away_message", static_cast<const BaseObject *>(business_away_message_.get())); }
    if (var1 & 16) { s.store_object_field("business_intro", static_cast<const BaseObject *>(business_intro_.get())); }
    if (var1 & 32) { s.store_object_field("birthday", static_cast<const BaseObject *>(birthday_.get())); }
    if (var1 & 64) { s.store_field("personal_channel_id", personal_channel_id_); }
    if (var1 & 64) { s.store_field("personal_channel_message", personal_channel_message_); }
    if (var1 & 256) { s.store_field("stargifts_count", stargifts_count_); }
    if (var1 & 2048) { s.store_object_field("starref_program", static_cast<const BaseObject *>(starref_program_.get())); }
    if (var1 & 4096) { s.store_object_field("bot_verification", static_cast<const BaseObject *>(bot_verification_.get())); }
    if (var1 & 16384) { s.store_field("send_paid_messages_stars", send_paid_messages_stars_); }
    if (var1 & 32768) { s.store_object_field("disallowed_gifts", static_cast<const BaseObject *>(disallowed_gifts_.get())); }
    if (var1 & 131072) { s.store_object_field("stars_rating", static_cast<const BaseObject *>(stars_rating_.get())); }
    if (var1 & 262144) { s.store_object_field("stars_my_pending_rating", static_cast<const BaseObject *>(stars_my_pending_rating_.get())); }
    if (var1 & 262144) { s.store_field("stars_my_pending_rating_date", stars_my_pending_rating_date_); }
    if (var1 & 1048576) { s.store_object_field("main_tab", static_cast<const BaseObject *>(main_tab_.get())); }
    if (var1 & 2097152) { s.store_object_field("saved_music", static_cast<const BaseObject *>(saved_music_.get())); }
    if (var1 & 4194304) { s.store_object_field("note", static_cast<const BaseObject *>(note_.get())); }
    s.store_class_end();
  }
}

username::username()
  : flags_()
  , editable_()
  , active_()
  , username_()
{}

const std::int32_t username::ID;

object_ptr<username> username::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<username> res = make_tl_object<username>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->editable_ = (var0 & 1) != 0;
  res->active_ = (var0 & 2) != 0;
  res->username_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void username::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "username");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (editable_ << 0) | (active_ << 1)));
    if (var0 & 1) { s.store_field("editable", true); }
    if (var0 & 2) { s.store_field("active", true); }
    s.store_field("username", username_);
    s.store_class_end();
  }
}

object_ptr<WallPaper> WallPaper::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case wallPaper::ID:
      return wallPaper::fetch(p);
    case wallPaperNoFile::ID:
      return wallPaperNoFile::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

wallPaper::wallPaper()
  : id_()
  , flags_()
  , creator_()
  , default_()
  , pattern_()
  , dark_()
  , access_hash_()
  , slug_()
  , document_()
  , settings_()
{}

const std::int32_t wallPaper::ID;

object_ptr<WallPaper> wallPaper::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<wallPaper> res = make_tl_object<wallPaper>();
  int32 var0;
  res->id_ = TlFetchLong::parse(p);
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->creator_ = (var0 & 1) != 0;
  res->default_ = (var0 & 2) != 0;
  res->pattern_ = (var0 & 8) != 0;
  res->dark_ = (var0 & 16) != 0;
  res->access_hash_ = TlFetchLong::parse(p);
  res->slug_ = TlFetchString<string>::parse(p);
  res->document_ = TlFetchObject<Document>::parse(p);
  if (var0 & 4) { res->settings_ = TlFetchBoxed<TlFetchObject<wallPaperSettings>, 925826256>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void wallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "wallPaper");
  int32 var0;
    s.store_field("id", id_);
    s.store_field("flags", (var0 = flags_ | (creator_ << 0) | (default_ << 1) | (pattern_ << 3) | (dark_ << 4)));
    if (var0 & 1) { s.store_field("creator", true); }
    if (var0 & 2) { s.store_field("default", true); }
    if (var0 & 8) { s.store_field("pattern", true); }
    if (var0 & 16) { s.store_field("dark", true); }
    s.store_field("access_hash", access_hash_);
    s.store_field("slug", slug_);
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    if (var0 & 4) { s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get())); }
    s.store_class_end();
  }
}

wallPaperNoFile::wallPaperNoFile()
  : id_()
  , flags_()
  , default_()
  , dark_()
  , settings_()
{}

const std::int32_t wallPaperNoFile::ID;

object_ptr<WallPaper> wallPaperNoFile::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<wallPaperNoFile> res = make_tl_object<wallPaperNoFile>();
  int32 var0;
  res->id_ = TlFetchLong::parse(p);
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->default_ = (var0 & 2) != 0;
  res->dark_ = (var0 & 16) != 0;
  if (var0 & 4) { res->settings_ = TlFetchBoxed<TlFetchObject<wallPaperSettings>, 925826256>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void wallPaperNoFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "wallPaperNoFile");
  int32 var0;
    s.store_field("id", id_);
    s.store_field("flags", (var0 = flags_ | (default_ << 1) | (dark_ << 4)));
    if (var0 & 2) { s.store_field("default", true); }
    if (var0 & 16) { s.store_field("dark", true); }
    if (var0 & 4) { s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get())); }
    s.store_class_end();
  }
}

object_ptr<account_EmailVerified> account_EmailVerified::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_emailVerified::ID:
      return account_emailVerified::fetch(p);
    case account_emailVerifiedLogin::ID:
      return account_emailVerifiedLogin::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_emailVerified::ID;

object_ptr<account_EmailVerified> account_emailVerified::fetch(TlBufferParser &p) {
  return make_tl_object<account_emailVerified>(p);
}

account_emailVerified::account_emailVerified(TlBufferParser &p)
  : email_(TlFetchString<string>::parse(p))
{}

void account_emailVerified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.emailVerified");
    s.store_field("email", email_);
    s.store_class_end();
  }
}

const std::int32_t account_emailVerifiedLogin::ID;

object_ptr<account_EmailVerified> account_emailVerifiedLogin::fetch(TlBufferParser &p) {
  return make_tl_object<account_emailVerifiedLogin>(p);
}

account_emailVerifiedLogin::account_emailVerifiedLogin(TlBufferParser &p)
  : email_(TlFetchString<string>::parse(p))
  , sent_code_(TlFetchObject<auth_SentCode>::parse(p))
{}

void account_emailVerifiedLogin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.emailVerifiedLogin");
    s.store_field("email", email_);
    s.store_object_field("sent_code", static_cast<const BaseObject *>(sent_code_.get()));
    s.store_class_end();
  }
}

object_ptr<account_EmojiStatuses> account_EmojiStatuses::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_emojiStatusesNotModified::ID:
      return account_emojiStatusesNotModified::fetch(p);
    case account_emojiStatuses::ID:
      return account_emojiStatuses::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_emojiStatusesNotModified::ID;

object_ptr<account_EmojiStatuses> account_emojiStatusesNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<account_emojiStatusesNotModified>();
}

void account_emojiStatusesNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.emojiStatusesNotModified");
    s.store_class_end();
  }
}

const std::int32_t account_emojiStatuses::ID;

object_ptr<account_EmojiStatuses> account_emojiStatuses::fetch(TlBufferParser &p) {
  return make_tl_object<account_emojiStatuses>(p);
}

account_emojiStatuses::account_emojiStatuses(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , statuses_(TlFetchBoxed<TlFetchVector<TlFetchObject<EmojiStatus>>, 481674261>::parse(p))
{}

void account_emojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.emojiStatuses");
    s.store_field("hash", hash_);
    { s.store_vector_begin("statuses", statuses_.size()); for (const auto &_value : statuses_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_paidMessagesRevenue::ID;

object_ptr<account_paidMessagesRevenue> account_paidMessagesRevenue::fetch(TlBufferParser &p) {
  return make_tl_object<account_paidMessagesRevenue>(p);
}

account_paidMessagesRevenue::account_paidMessagesRevenue(TlBufferParser &p)
  : stars_amount_(TlFetchLong::parse(p))
{}

void account_paidMessagesRevenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.paidMessagesRevenue");
    s.store_field("stars_amount", stars_amount_);
    s.store_class_end();
  }
}

account_passwordInputSettings::account_passwordInputSettings(int32 flags_, object_ptr<PasswordKdfAlgo> &&new_algo_, bytes &&new_password_hash_, string const &hint_, string const &email_, object_ptr<secureSecretSettings> &&new_secure_settings_)
  : flags_(flags_)
  , new_algo_(std::move(new_algo_))
  , new_password_hash_(std::move(new_password_hash_))
  , hint_(hint_)
  , email_(email_)
  , new_secure_settings_(std::move(new_secure_settings_))
{}

const std::int32_t account_passwordInputSettings::ID;

void account_passwordInputSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(new_algo_, s); }
  if (var0 & 1) { TlStoreString::store(new_password_hash_, s); }
  if (var0 & 1) { TlStoreString::store(hint_, s); }
  if (var0 & 2) { TlStoreString::store(email_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 354925740>::store(new_secure_settings_, s); }
}

void account_passwordInputSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(new_algo_, s); }
  if (var0 & 1) { TlStoreString::store(new_password_hash_, s); }
  if (var0 & 1) { TlStoreString::store(hint_, s); }
  if (var0 & 2) { TlStoreString::store(email_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 354925740>::store(new_secure_settings_, s); }
}

void account_passwordInputSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.passwordInputSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("new_algo", static_cast<const BaseObject *>(new_algo_.get())); }
    if (var0 & 1) { s.store_bytes_field("new_password_hash", new_password_hash_); }
    if (var0 & 1) { s.store_field("hint", hint_); }
    if (var0 & 2) { s.store_field("email", email_); }
    if (var0 & 4) { s.store_object_field("new_secure_settings", static_cast<const BaseObject *>(new_secure_settings_.get())); }
    s.store_class_end();
  }
}

object_ptr<account_Themes> account_Themes::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_themesNotModified::ID:
      return account_themesNotModified::fetch(p);
    case account_themes::ID:
      return account_themes::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_themesNotModified::ID;

object_ptr<account_Themes> account_themesNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<account_themesNotModified>();
}

void account_themesNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.themesNotModified");
    s.store_class_end();
  }
}

const std::int32_t account_themes::ID;

object_ptr<account_Themes> account_themes::fetch(TlBufferParser &p) {
  return make_tl_object<account_themes>(p);
}

account_themes::account_themes(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , themes_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<theme>, -1609668650>>, 481674261>::parse(p))
{}

void account_themes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.themes");
    s.store_field("hash", hash_);
    { s.store_vector_begin("themes", themes_.size()); for (const auto &_value : themes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_webAuthorizations::ID;

object_ptr<account_webAuthorizations> account_webAuthorizations::fetch(TlBufferParser &p) {
  return make_tl_object<account_webAuthorizations>(p);
}

account_webAuthorizations::account_webAuthorizations(TlBufferParser &p)
  : authorizations_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<webAuthorization>, -1493633966>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void account_webAuthorizations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.webAuthorizations");
    { s.store_vector_begin("authorizations", authorizations_.size()); for (const auto &_value : authorizations_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<auth_SentCodeType> auth_SentCodeType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case auth_sentCodeTypeApp::ID:
      return auth_sentCodeTypeApp::fetch(p);
    case auth_sentCodeTypeSms::ID:
      return auth_sentCodeTypeSms::fetch(p);
    case auth_sentCodeTypeCall::ID:
      return auth_sentCodeTypeCall::fetch(p);
    case auth_sentCodeTypeFlashCall::ID:
      return auth_sentCodeTypeFlashCall::fetch(p);
    case auth_sentCodeTypeMissedCall::ID:
      return auth_sentCodeTypeMissedCall::fetch(p);
    case auth_sentCodeTypeEmailCode::ID:
      return auth_sentCodeTypeEmailCode::fetch(p);
    case auth_sentCodeTypeSetUpEmailRequired::ID:
      return auth_sentCodeTypeSetUpEmailRequired::fetch(p);
    case auth_sentCodeTypeFragmentSms::ID:
      return auth_sentCodeTypeFragmentSms::fetch(p);
    case auth_sentCodeTypeFirebaseSms::ID:
      return auth_sentCodeTypeFirebaseSms::fetch(p);
    case auth_sentCodeTypeSmsWord::ID:
      return auth_sentCodeTypeSmsWord::fetch(p);
    case auth_sentCodeTypeSmsPhrase::ID:
      return auth_sentCodeTypeSmsPhrase::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t auth_sentCodeTypeApp::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeApp::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodeTypeApp>(p);
}

auth_sentCodeTypeApp::auth_sentCodeTypeApp(TlBufferParser &p)
  : length_(TlFetchInt::parse(p))
{}

void auth_sentCodeTypeApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeApp");
    s.store_field("length", length_);
    s.store_class_end();
  }
}

const std::int32_t auth_sentCodeTypeSms::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeSms::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodeTypeSms>(p);
}

auth_sentCodeTypeSms::auth_sentCodeTypeSms(TlBufferParser &p)
  : length_(TlFetchInt::parse(p))
{}

void auth_sentCodeTypeSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeSms");
    s.store_field("length", length_);
    s.store_class_end();
  }
}

const std::int32_t auth_sentCodeTypeCall::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeCall::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodeTypeCall>(p);
}

auth_sentCodeTypeCall::auth_sentCodeTypeCall(TlBufferParser &p)
  : length_(TlFetchInt::parse(p))
{}

void auth_sentCodeTypeCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeCall");
    s.store_field("length", length_);
    s.store_class_end();
  }
}

const std::int32_t auth_sentCodeTypeFlashCall::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeFlashCall::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodeTypeFlashCall>(p);
}

auth_sentCodeTypeFlashCall::auth_sentCodeTypeFlashCall(TlBufferParser &p)
  : pattern_(TlFetchString<string>::parse(p))
{}

void auth_sentCodeTypeFlashCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeFlashCall");
    s.store_field("pattern", pattern_);
    s.store_class_end();
  }
}

const std::int32_t auth_sentCodeTypeMissedCall::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeMissedCall::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodeTypeMissedCall>(p);
}

auth_sentCodeTypeMissedCall::auth_sentCodeTypeMissedCall(TlBufferParser &p)
  : prefix_(TlFetchString<string>::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void auth_sentCodeTypeMissedCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeMissedCall");
    s.store_field("prefix", prefix_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

auth_sentCodeTypeEmailCode::auth_sentCodeTypeEmailCode()
  : flags_()
  , apple_signin_allowed_()
  , google_signin_allowed_()
  , email_pattern_()
  , length_()
  , reset_available_period_()
  , reset_pending_date_()
{}

const std::int32_t auth_sentCodeTypeEmailCode::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeEmailCode::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_sentCodeTypeEmailCode> res = make_tl_object<auth_sentCodeTypeEmailCode>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->apple_signin_allowed_ = (var0 & 1) != 0;
  res->google_signin_allowed_ = (var0 & 2) != 0;
  res->email_pattern_ = TlFetchString<string>::parse(p);
  res->length_ = TlFetchInt::parse(p);
  if (var0 & 8) { res->reset_available_period_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->reset_pending_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_sentCodeTypeEmailCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeEmailCode");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (apple_signin_allowed_ << 0) | (google_signin_allowed_ << 1)));
    if (var0 & 1) { s.store_field("apple_signin_allowed", true); }
    if (var0 & 2) { s.store_field("google_signin_allowed", true); }
    s.store_field("email_pattern", email_pattern_);
    s.store_field("length", length_);
    if (var0 & 8) { s.store_field("reset_available_period", reset_available_period_); }
    if (var0 & 16) { s.store_field("reset_pending_date", reset_pending_date_); }
    s.store_class_end();
  }
}

auth_sentCodeTypeSetUpEmailRequired::auth_sentCodeTypeSetUpEmailRequired()
  : flags_()
  , apple_signin_allowed_()
  , google_signin_allowed_()
{}

const std::int32_t auth_sentCodeTypeSetUpEmailRequired::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeSetUpEmailRequired::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_sentCodeTypeSetUpEmailRequired> res = make_tl_object<auth_sentCodeTypeSetUpEmailRequired>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->apple_signin_allowed_ = (var0 & 1) != 0;
  res->google_signin_allowed_ = (var0 & 2) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_sentCodeTypeSetUpEmailRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeSetUpEmailRequired");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (apple_signin_allowed_ << 0) | (google_signin_allowed_ << 1)));
    if (var0 & 1) { s.store_field("apple_signin_allowed", true); }
    if (var0 & 2) { s.store_field("google_signin_allowed", true); }
    s.store_class_end();
  }
}

const std::int32_t auth_sentCodeTypeFragmentSms::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeFragmentSms::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodeTypeFragmentSms>(p);
}

auth_sentCodeTypeFragmentSms::auth_sentCodeTypeFragmentSms(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void auth_sentCodeTypeFragmentSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeFragmentSms");
    s.store_field("url", url_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

auth_sentCodeTypeFirebaseSms::auth_sentCodeTypeFirebaseSms()
  : flags_()
  , nonce_()
  , play_integrity_project_id_()
  , play_integrity_nonce_()
  , receipt_()
  , push_timeout_()
  , length_()
{}

const std::int32_t auth_sentCodeTypeFirebaseSms::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeFirebaseSms::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_sentCodeTypeFirebaseSms> res = make_tl_object<auth_sentCodeTypeFirebaseSms>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->nonce_ = TlFetchBytes<bytes>::parse(p); }
  if (var0 & 4) { res->play_integrity_project_id_ = TlFetchLong::parse(p); }
  if (var0 & 4) { res->play_integrity_nonce_ = TlFetchBytes<bytes>::parse(p); }
  if (var0 & 2) { res->receipt_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->push_timeout_ = TlFetchInt::parse(p); }
  res->length_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_sentCodeTypeFirebaseSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeFirebaseSms");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_bytes_field("nonce", nonce_); }
    if (var0 & 4) { s.store_field("play_integrity_project_id", play_integrity_project_id_); }
    if (var0 & 4) { s.store_bytes_field("play_integrity_nonce", play_integrity_nonce_); }
    if (var0 & 2) { s.store_field("receipt", receipt_); }
    if (var0 & 2) { s.store_field("push_timeout", push_timeout_); }
    s.store_field("length", length_);
    s.store_class_end();
  }
}

auth_sentCodeTypeSmsWord::auth_sentCodeTypeSmsWord()
  : flags_()
  , beginning_()
{}

const std::int32_t auth_sentCodeTypeSmsWord::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeSmsWord::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_sentCodeTypeSmsWord> res = make_tl_object<auth_sentCodeTypeSmsWord>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->beginning_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_sentCodeTypeSmsWord::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeSmsWord");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("beginning", beginning_); }
    s.store_class_end();
  }
}

auth_sentCodeTypeSmsPhrase::auth_sentCodeTypeSmsPhrase()
  : flags_()
  , beginning_()
{}

const std::int32_t auth_sentCodeTypeSmsPhrase::ID;

object_ptr<auth_SentCodeType> auth_sentCodeTypeSmsPhrase::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_sentCodeTypeSmsPhrase> res = make_tl_object<auth_sentCodeTypeSmsPhrase>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->beginning_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_sentCodeTypeSmsPhrase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeTypeSmsPhrase");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("beginning", beginning_); }
    s.store_class_end();
  }
}

object_ptr<channels_SponsoredMessageReportResult> channels_SponsoredMessageReportResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case channels_sponsoredMessageReportResultChooseOption::ID:
      return channels_sponsoredMessageReportResultChooseOption::fetch(p);
    case channels_sponsoredMessageReportResultAdsHidden::ID:
      return channels_sponsoredMessageReportResultAdsHidden::fetch(p);
    case channels_sponsoredMessageReportResultReported::ID:
      return channels_sponsoredMessageReportResultReported::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t channels_sponsoredMessageReportResultChooseOption::ID;

object_ptr<channels_SponsoredMessageReportResult> channels_sponsoredMessageReportResultChooseOption::fetch(TlBufferParser &p) {
  return make_tl_object<channels_sponsoredMessageReportResultChooseOption>(p);
}

channels_sponsoredMessageReportResultChooseOption::channels_sponsoredMessageReportResultChooseOption(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , options_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<sponsoredMessageReportOption>, 1124938064>>, 481674261>::parse(p))
{}

void channels_sponsoredMessageReportResultChooseOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.sponsoredMessageReportResultChooseOption");
    s.store_field("title", title_);
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t channels_sponsoredMessageReportResultAdsHidden::ID;

object_ptr<channels_SponsoredMessageReportResult> channels_sponsoredMessageReportResultAdsHidden::fetch(TlBufferParser &p) {
  return make_tl_object<channels_sponsoredMessageReportResultAdsHidden>();
}

void channels_sponsoredMessageReportResultAdsHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.sponsoredMessageReportResultAdsHidden");
    s.store_class_end();
  }
}

const std::int32_t channels_sponsoredMessageReportResultReported::ID;

object_ptr<channels_SponsoredMessageReportResult> channels_sponsoredMessageReportResultReported::fetch(TlBufferParser &p) {
  return make_tl_object<channels_sponsoredMessageReportResultReported>();
}

void channels_sponsoredMessageReportResultReported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.sponsoredMessageReportResultReported");
    s.store_class_end();
  }
}

object_ptr<contacts_SponsoredPeers> contacts_SponsoredPeers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case contacts_sponsoredPeersEmpty::ID:
      return contacts_sponsoredPeersEmpty::fetch(p);
    case contacts_sponsoredPeers::ID:
      return contacts_sponsoredPeers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t contacts_sponsoredPeersEmpty::ID;

object_ptr<contacts_SponsoredPeers> contacts_sponsoredPeersEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_sponsoredPeersEmpty>();
}

void contacts_sponsoredPeersEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.sponsoredPeersEmpty");
    s.store_class_end();
  }
}

const std::int32_t contacts_sponsoredPeers::ID;

object_ptr<contacts_SponsoredPeers> contacts_sponsoredPeers::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_sponsoredPeers>(p);
}

contacts_sponsoredPeers::contacts_sponsoredPeers(TlBufferParser &p)
  : peers_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<sponsoredPeer>, -963180333>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_sponsoredPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.sponsoredPeers");
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

help_peerColorOption::help_peerColorOption()
  : flags_()
  , hidden_()
  , color_id_()
  , colors_()
  , dark_colors_()
  , channel_min_level_()
  , group_min_level_()
{}

const std::int32_t help_peerColorOption::ID;

object_ptr<help_peerColorOption> help_peerColorOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<help_peerColorOption> res = make_tl_object<help_peerColorOption>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->hidden_ = (var0 & 1) != 0;
  res->color_id_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->colors_ = TlFetchObject<help_PeerColorSet>::parse(p); }
  if (var0 & 4) { res->dark_colors_ = TlFetchObject<help_PeerColorSet>::parse(p); }
  if (var0 & 8) { res->channel_min_level_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->group_min_level_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void help_peerColorOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.peerColorOption");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (hidden_ << 0)));
    if (var0 & 1) { s.store_field("hidden", true); }
    s.store_field("color_id", color_id_);
    if (var0 & 2) { s.store_object_field("colors", static_cast<const BaseObject *>(colors_.get())); }
    if (var0 & 4) { s.store_object_field("dark_colors", static_cast<const BaseObject *>(dark_colors_.get())); }
    if (var0 & 8) { s.store_field("channel_min_level", channel_min_level_); }
    if (var0 & 16) { s.store_field("group_min_level", group_min_level_); }
    s.store_class_end();
  }
}

object_ptr<help_TermsOfServiceUpdate> help_TermsOfServiceUpdate::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_termsOfServiceUpdateEmpty::ID:
      return help_termsOfServiceUpdateEmpty::fetch(p);
    case help_termsOfServiceUpdate::ID:
      return help_termsOfServiceUpdate::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_termsOfServiceUpdateEmpty::ID;

object_ptr<help_TermsOfServiceUpdate> help_termsOfServiceUpdateEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<help_termsOfServiceUpdateEmpty>(p);
}

help_termsOfServiceUpdateEmpty::help_termsOfServiceUpdateEmpty(TlBufferParser &p)
  : expires_(TlFetchInt::parse(p))
{}

void help_termsOfServiceUpdateEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.termsOfServiceUpdateEmpty");
    s.store_field("expires", expires_);
    s.store_class_end();
  }
}

const std::int32_t help_termsOfServiceUpdate::ID;

object_ptr<help_TermsOfServiceUpdate> help_termsOfServiceUpdate::fetch(TlBufferParser &p) {
  return make_tl_object<help_termsOfServiceUpdate>(p);
}

help_termsOfServiceUpdate::help_termsOfServiceUpdate(TlBufferParser &p)
  : expires_(TlFetchInt::parse(p))
  , terms_of_service_(TlFetchBoxed<TlFetchObject<help_termsOfService>, 2013922064>::parse(p))
{}

void help_termsOfServiceUpdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.termsOfServiceUpdate");
    s.store_field("expires", expires_);
    s.store_object_field("terms_of_service", static_cast<const BaseObject *>(terms_of_service_.get()));
    s.store_class_end();
  }
}

object_ptr<help_TimezonesList> help_TimezonesList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_timezonesListNotModified::ID:
      return help_timezonesListNotModified::fetch(p);
    case help_timezonesList::ID:
      return help_timezonesList::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_timezonesListNotModified::ID;

object_ptr<help_TimezonesList> help_timezonesListNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<help_timezonesListNotModified>();
}

void help_timezonesListNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.timezonesListNotModified");
    s.store_class_end();
  }
}

const std::int32_t help_timezonesList::ID;

object_ptr<help_TimezonesList> help_timezonesList::fetch(TlBufferParser &p) {
  return make_tl_object<help_timezonesList>(p);
}

help_timezonesList::help_timezonesList(TlBufferParser &p)
  : timezones_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<timezone>, -7173643>>, 481674261>::parse(p))
  , hash_(TlFetchInt::parse(p))
{}

void help_timezonesList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.timezonesList");
    { s.store_vector_begin("timezones", timezones_.size()); for (const auto &_value : timezones_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

object_ptr<messages_AvailableEffects> messages_AvailableEffects::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_availableEffectsNotModified::ID:
      return messages_availableEffectsNotModified::fetch(p);
    case messages_availableEffects::ID:
      return messages_availableEffects::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_availableEffectsNotModified::ID;

object_ptr<messages_AvailableEffects> messages_availableEffectsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_availableEffectsNotModified>();
}

void messages_availableEffectsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.availableEffectsNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_availableEffects::ID;

object_ptr<messages_AvailableEffects> messages_availableEffects::fetch(TlBufferParser &p) {
  return make_tl_object<messages_availableEffects>(p);
}

messages_availableEffects::messages_availableEffects(TlBufferParser &p)
  : hash_(TlFetchInt::parse(p))
  , effects_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<availableEffect>, -1815879042>>, 481674261>::parse(p))
  , documents_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void messages_availableEffects::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.availableEffects");
    s.store_field("hash", hash_);
    { s.store_vector_begin("effects", effects_.size()); for (const auto &_value : effects_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_discussionMessage::messages_discussionMessage()
  : flags_()
  , messages_()
  , max_id_()
  , read_inbox_max_id_()
  , read_outbox_max_id_()
  , unread_count_()
  , chats_()
  , users_()
{}

const std::int32_t messages_discussionMessage::ID;

object_ptr<messages_discussionMessage> messages_discussionMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_discussionMessage> res = make_tl_object<messages_discussionMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p);
  if (var0 & 1) { res->max_id_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->read_inbox_max_id_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->read_outbox_max_id_ = TlFetchInt::parse(p); }
  res->unread_count_ = TlFetchInt::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_discussionMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.discussionMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("max_id", max_id_); }
    if (var0 & 2) { s.store_field("read_inbox_max_id", read_inbox_max_id_); }
    if (var0 & 4) { s.store_field("read_outbox_max_id", read_outbox_max_id_); }
    s.store_field("unread_count", unread_count_);
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_forumTopics::messages_forumTopics()
  : flags_()
  , order_by_create_date_()
  , count_()
  , topics_()
  , messages_()
  , chats_()
  , users_()
  , pts_()
{}

const std::int32_t messages_forumTopics::ID;

object_ptr<messages_forumTopics> messages_forumTopics::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_forumTopics> res = make_tl_object<messages_forumTopics>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->order_by_create_date_ = (var0 & 1) != 0;
  res->count_ = TlFetchInt::parse(p);
  res->topics_ = TlFetchBoxed<TlFetchVector<TlFetchObject<ForumTopic>>, 481674261>::parse(p);
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_forumTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.forumTopics");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (order_by_create_date_ << 0)));
    if (var0 & 1) { s.store_field("order_by_create_date", true); }
    s.store_field("count", count_);
    { s.store_vector_begin("topics", topics_.size()); for (const auto &_value : topics_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("pts", pts_);
    s.store_class_end();
  }
}

const std::int32_t messages_highScores::ID;

object_ptr<messages_highScores> messages_highScores::fetch(TlBufferParser &p) {
  return make_tl_object<messages_highScores>(p);
}

messages_highScores::messages_highScores(TlBufferParser &p)
  : scores_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<highScore>, 1940093419>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_highScores::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.highScores");
    { s.store_vector_begin("scores", scores_.size()); for (const auto &_value : scores_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_messageReactionsList::messages_messageReactionsList()
  : flags_()
  , count_()
  , reactions_()
  , chats_()
  , users_()
  , next_offset_()
{}

const std::int32_t messages_messageReactionsList::ID;

object_ptr<messages_messageReactionsList> messages_messageReactionsList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_messageReactionsList> res = make_tl_object<messages_messageReactionsList>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->reactions_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<messagePeerReaction>, -1938180548>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_messageReactionsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.messageReactionsList");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    s.store_class_end();
  }
}

object_ptr<messages_RecentStickers> messages_RecentStickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_recentStickersNotModified::ID:
      return messages_recentStickersNotModified::fetch(p);
    case messages_recentStickers::ID:
      return messages_recentStickers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_recentStickersNotModified::ID;

object_ptr<messages_RecentStickers> messages_recentStickersNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_recentStickersNotModified>();
}

void messages_recentStickersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.recentStickersNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_recentStickers::ID;

object_ptr<messages_RecentStickers> messages_recentStickers::fetch(TlBufferParser &p) {
  return make_tl_object<messages_recentStickers>(p);
}

messages_recentStickers::messages_recentStickers(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , packs_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<stickerPack>, 313694676>>, 481674261>::parse(p))
  , stickers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
  , dates_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void messages_recentStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.recentStickers");
    s.store_field("hash", hash_);
    { s.store_vector_begin("packs", packs_.size()); for (const auto &_value : packs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("dates", dates_.size()); for (const auto &_value : dates_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_StickerSetInstallResult> messages_StickerSetInstallResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_stickerSetInstallResultSuccess::ID:
      return messages_stickerSetInstallResultSuccess::fetch(p);
    case messages_stickerSetInstallResultArchive::ID:
      return messages_stickerSetInstallResultArchive::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_stickerSetInstallResultSuccess::ID;

object_ptr<messages_StickerSetInstallResult> messages_stickerSetInstallResultSuccess::fetch(TlBufferParser &p) {
  return make_tl_object<messages_stickerSetInstallResultSuccess>();
}

void messages_stickerSetInstallResultSuccess::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.stickerSetInstallResultSuccess");
    s.store_class_end();
  }
}

const std::int32_t messages_stickerSetInstallResultArchive::ID;

object_ptr<messages_StickerSetInstallResult> messages_stickerSetInstallResultArchive::fetch(TlBufferParser &p) {
  return make_tl_object<messages_stickerSetInstallResultArchive>(p);
}

messages_stickerSetInstallResultArchive::messages_stickerSetInstallResultArchive(TlBufferParser &p)
  : sets_(TlFetchBoxed<TlFetchVector<TlFetchObject<StickerSetCovered>>, 481674261>::parse(p))
{}

void messages_stickerSetInstallResultArchive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.stickerSetInstallResultArchive");
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t payments_connectedStarRefBots::ID;

object_ptr<payments_connectedStarRefBots> payments_connectedStarRefBots::fetch(TlBufferParser &p) {
  return make_tl_object<payments_connectedStarRefBots>(p);
}

payments_connectedStarRefBots::payments_connectedStarRefBots(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , connected_bots_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<connectedBotStarRef>, 429997937>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void payments_connectedStarRefBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.connectedStarRefBots");
    s.store_field("count", count_);
    { s.store_vector_begin("connected_bots", connected_bots_.size()); for (const auto &_value : connected_bots_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<payments_PaymentForm> payments_PaymentForm::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case payments_paymentForm::ID:
      return payments_paymentForm::fetch(p);
    case payments_paymentFormStars::ID:
      return payments_paymentFormStars::fetch(p);
    case payments_paymentFormStarGift::ID:
      return payments_paymentFormStarGift::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

payments_paymentForm::payments_paymentForm()
  : flags_()
  , can_save_credentials_()
  , password_missing_()
  , form_id_()
  , bot_id_()
  , title_()
  , description_()
  , photo_()
  , invoice_()
  , provider_id_()
  , url_()
  , native_provider_()
  , native_params_()
  , additional_methods_()
  , saved_info_()
  , saved_credentials_()
  , users_()
{}

const std::int32_t payments_paymentForm::ID;

object_ptr<payments_PaymentForm> payments_paymentForm::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_paymentForm> res = make_tl_object<payments_paymentForm>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->can_save_credentials_ = (var0 & 4) != 0;
  res->password_missing_ = (var0 & 8) != 0;
  res->form_id_ = TlFetchLong::parse(p);
  res->bot_id_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 32) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  res->invoice_ = TlFetchBoxed<TlFetchObject<invoice>, 77522308>::parse(p);
  res->provider_id_ = TlFetchLong::parse(p);
  res->url_ = TlFetchString<string>::parse(p);
  if (var0 & 16) { res->native_provider_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->native_params_ = TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p); }
  if (var0 & 64) { res->additional_methods_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<paymentFormMethod>, -1996951013>>, 481674261>::parse(p); }
  if (var0 & 1) { res->saved_info_ = TlFetchBoxed<TlFetchObject<paymentRequestedInfo>, -1868808300>::parse(p); }
  if (var0 & 2) { res->saved_credentials_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<paymentSavedCredentialsCard>, -842892769>>, 481674261>::parse(p); }
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void payments_paymentForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.paymentForm");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (can_save_credentials_ << 2) | (password_missing_ << 3)));
    if (var0 & 4) { s.store_field("can_save_credentials", true); }
    if (var0 & 8) { s.store_field("password_missing", true); }
    s.store_field("form_id", form_id_);
    s.store_field("bot_id", bot_id_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 32) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_field("provider_id", provider_id_);
    s.store_field("url", url_);
    if (var0 & 16) { s.store_field("native_provider", native_provider_); }
    if (var0 & 16) { s.store_object_field("native_params", static_cast<const BaseObject *>(native_params_.get())); }
    if (var0 & 64) { { s.store_vector_begin("additional_methods", additional_methods_.size()); for (const auto &_value : additional_methods_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 1) { s.store_object_field("saved_info", static_cast<const BaseObject *>(saved_info_.get())); }
    if (var0 & 2) { { s.store_vector_begin("saved_credentials", saved_credentials_.size()); for (const auto &_value : saved_credentials_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_paymentFormStars::payments_paymentFormStars()
  : flags_()
  , form_id_()
  , bot_id_()
  , title_()
  , description_()
  , photo_()
  , invoice_()
  , users_()
{}

const std::int32_t payments_paymentFormStars::ID;

object_ptr<payments_PaymentForm> payments_paymentFormStars::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_paymentFormStars> res = make_tl_object<payments_paymentFormStars>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->form_id_ = TlFetchLong::parse(p);
  res->bot_id_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 32) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  res->invoice_ = TlFetchBoxed<TlFetchObject<invoice>, 77522308>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void payments_paymentFormStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.paymentFormStars");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("form_id", form_id_);
    s.store_field("bot_id", bot_id_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 32) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t payments_paymentFormStarGift::ID;

object_ptr<payments_PaymentForm> payments_paymentFormStarGift::fetch(TlBufferParser &p) {
  return make_tl_object<payments_paymentFormStarGift>(p);
}

payments_paymentFormStarGift::payments_paymentFormStarGift(TlBufferParser &p)
  : form_id_(TlFetchLong::parse(p))
  , invoice_(TlFetchBoxed<TlFetchObject<invoice>, 77522308>::parse(p))
{}

void payments_paymentFormStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.paymentFormStarGift");
    s.store_field("form_id", form_id_);
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_class_end();
  }
}

payments_uniqueStarGiftValueInfo::payments_uniqueStarGiftValueInfo()
  : flags_()
  , last_sale_on_fragment_()
  , value_is_average_()
  , currency_()
  , value_()
  , initial_sale_date_()
  , initial_sale_stars_()
  , initial_sale_price_()
  , last_sale_date_()
  , last_sale_price_()
  , floor_price_()
  , average_price_()
  , listed_count_()
  , fragment_listed_count_()
  , fragment_listed_url_()
{}

const std::int32_t payments_uniqueStarGiftValueInfo::ID;

object_ptr<payments_uniqueStarGiftValueInfo> payments_uniqueStarGiftValueInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_uniqueStarGiftValueInfo> res = make_tl_object<payments_uniqueStarGiftValueInfo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->last_sale_on_fragment_ = (var0 & 2) != 0;
  res->value_is_average_ = (var0 & 64) != 0;
  res->currency_ = TlFetchString<string>::parse(p);
  res->value_ = TlFetchLong::parse(p);
  res->initial_sale_date_ = TlFetchInt::parse(p);
  res->initial_sale_stars_ = TlFetchLong::parse(p);
  res->initial_sale_price_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->last_sale_date_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->last_sale_price_ = TlFetchLong::parse(p); }
  if (var0 & 4) { res->floor_price_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->average_price_ = TlFetchLong::parse(p); }
  if (var0 & 16) { res->listed_count_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->fragment_listed_count_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->fragment_listed_url_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_uniqueStarGiftValueInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.uniqueStarGiftValueInfo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (last_sale_on_fragment_ << 1) | (value_is_average_ << 6)));
    if (var0 & 2) { s.store_field("last_sale_on_fragment", true); }
    if (var0 & 64) { s.store_field("value_is_average", true); }
    s.store_field("currency", currency_);
    s.store_field("value", value_);
    s.store_field("initial_sale_date", initial_sale_date_);
    s.store_field("initial_sale_stars", initial_sale_stars_);
    s.store_field("initial_sale_price", initial_sale_price_);
    if (var0 & 1) { s.store_field("last_sale_date", last_sale_date_); }
    if (var0 & 1) { s.store_field("last_sale_price", last_sale_price_); }
    if (var0 & 4) { s.store_field("floor_price", floor_price_); }
    if (var0 & 8) { s.store_field("average_price", average_price_); }
    if (var0 & 16) { s.store_field("listed_count", listed_count_); }
    if (var0 & 32) { s.store_field("fragment_listed_count", fragment_listed_count_); }
    if (var0 & 32) { s.store_field("fragment_listed_url", fragment_listed_url_); }
    s.store_class_end();
  }
}

payments_validatedRequestedInfo::payments_validatedRequestedInfo()
  : flags_()
  , id_()
  , shipping_options_()
{}

const std::int32_t payments_validatedRequestedInfo::ID;

object_ptr<payments_validatedRequestedInfo> payments_validatedRequestedInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_validatedRequestedInfo> res = make_tl_object<payments_validatedRequestedInfo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->id_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->shipping_options_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<shippingOption>, -1239335713>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_validatedRequestedInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.validatedRequestedInfo");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("id", id_); }
    if (var0 & 2) { { s.store_vector_begin("shipping_options", shipping_options_.size()); for (const auto &_value : shipping_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

phone_groupParticipants::phone_groupParticipants(int32 count_, array<object_ptr<groupCallParticipant>> &&participants_, string const &next_offset_, array<object_ptr<Chat>> &&chats_, array<object_ptr<User>> &&users_, int32 version_)
  : count_(count_)
  , participants_(std::move(participants_))
  , next_offset_(next_offset_)
  , chats_(std::move(chats_))
  , users_(std::move(users_))
  , version_(version_)
{}

const std::int32_t phone_groupParticipants::ID;

object_ptr<phone_groupParticipants> phone_groupParticipants::fetch(TlBufferParser &p) {
  return make_tl_object<phone_groupParticipants>(p);
}

phone_groupParticipants::phone_groupParticipants(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , participants_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<groupCallParticipant>, -341428482>>, 481674261>::parse(p))
  , next_offset_(TlFetchString<string>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
  , version_(TlFetchInt::parse(p))
{}

void phone_groupParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.groupParticipants");
    s.store_field("count", count_);
    { s.store_vector_begin("participants", participants_.size()); for (const auto &_value : participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("version", version_);
    s.store_class_end();
  }
}

const std::int32_t stats_megagroupStats::ID;

object_ptr<stats_megagroupStats> stats_megagroupStats::fetch(TlBufferParser &p) {
  return make_tl_object<stats_megagroupStats>(p);
}

stats_megagroupStats::stats_megagroupStats(TlBufferParser &p)
  : period_(TlFetchBoxed<TlFetchObject<statsDateRangeDays>, -1237848657>::parse(p))
  , members_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , messages_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , viewers_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , posters_(TlFetchBoxed<TlFetchObject<statsAbsValueAndPrev>, -884757282>::parse(p))
  , growth_graph_(TlFetchObject<StatsGraph>::parse(p))
  , members_graph_(TlFetchObject<StatsGraph>::parse(p))
  , new_members_by_source_graph_(TlFetchObject<StatsGraph>::parse(p))
  , languages_graph_(TlFetchObject<StatsGraph>::parse(p))
  , messages_graph_(TlFetchObject<StatsGraph>::parse(p))
  , actions_graph_(TlFetchObject<StatsGraph>::parse(p))
  , top_hours_graph_(TlFetchObject<StatsGraph>::parse(p))
  , weekdays_graph_(TlFetchObject<StatsGraph>::parse(p))
  , top_posters_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<statsGroupTopPoster>, -1660637285>>, 481674261>::parse(p))
  , top_admins_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<statsGroupTopAdmin>, -682079097>>, 481674261>::parse(p))
  , top_inviters_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<statsGroupTopInviter>, 1398765469>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void stats_megagroupStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.megagroupStats");
    s.store_object_field("period", static_cast<const BaseObject *>(period_.get()));
    s.store_object_field("members", static_cast<const BaseObject *>(members_.get()));
    s.store_object_field("messages", static_cast<const BaseObject *>(messages_.get()));
    s.store_object_field("viewers", static_cast<const BaseObject *>(viewers_.get()));
    s.store_object_field("posters", static_cast<const BaseObject *>(posters_.get()));
    s.store_object_field("growth_graph", static_cast<const BaseObject *>(growth_graph_.get()));
    s.store_object_field("members_graph", static_cast<const BaseObject *>(members_graph_.get()));
    s.store_object_field("new_members_by_source_graph", static_cast<const BaseObject *>(new_members_by_source_graph_.get()));
    s.store_object_field("languages_graph", static_cast<const BaseObject *>(languages_graph_.get()));
    s.store_object_field("messages_graph", static_cast<const BaseObject *>(messages_graph_.get()));
    s.store_object_field("actions_graph", static_cast<const BaseObject *>(actions_graph_.get()));
    s.store_object_field("top_hours_graph", static_cast<const BaseObject *>(top_hours_graph_.get()));
    s.store_object_field("weekdays_graph", static_cast<const BaseObject *>(weekdays_graph_.get()));
    { s.store_vector_begin("top_posters", top_posters_.size()); for (const auto &_value : top_posters_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("top_admins", top_admins_.size()); for (const auto &_value : top_admins_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("top_inviters", top_inviters_.size()); for (const auto &_value : top_inviters_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t stats_storyStats::ID;

object_ptr<stats_storyStats> stats_storyStats::fetch(TlBufferParser &p) {
  return make_tl_object<stats_storyStats>(p);
}

stats_storyStats::stats_storyStats(TlBufferParser &p)
  : views_graph_(TlFetchObject<StatsGraph>::parse(p))
  , reactions_by_emotion_graph_(TlFetchObject<StatsGraph>::parse(p))
{}

void stats_storyStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.storyStats");
    s.store_object_field("views_graph", static_cast<const BaseObject *>(views_graph_.get()));
    s.store_object_field("reactions_by_emotion_graph", static_cast<const BaseObject *>(reactions_by_emotion_graph_.get()));
    s.store_class_end();
  }
}

account_changeAuthorizationSettings::account_changeAuthorizationSettings(int32 flags_, bool confirmed_, int64 hash_, bool encrypted_requests_disabled_, bool call_requests_disabled_)
  : flags_(flags_)
  , confirmed_(confirmed_)
  , hash_(hash_)
  , encrypted_requests_disabled_(encrypted_requests_disabled_)
  , call_requests_disabled_(call_requests_disabled_)
{}

const std::int32_t account_changeAuthorizationSettings::ID;

void account_changeAuthorizationSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1089766498);
  TlStoreBinary::store((var0 = flags_ | (confirmed_ << 3)), s);
  TlStoreBinary::store(hash_, s);
  if (var0 & 1) { TlStoreBool::store(encrypted_requests_disabled_, s); }
  if (var0 & 2) { TlStoreBool::store(call_requests_disabled_, s); }
}

void account_changeAuthorizationSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1089766498);
  TlStoreBinary::store((var0 = flags_ | (confirmed_ << 3)), s);
  TlStoreBinary::store(hash_, s);
  if (var0 & 1) { TlStoreBool::store(encrypted_requests_disabled_, s); }
  if (var0 & 2) { TlStoreBool::store(call_requests_disabled_, s); }
}

void account_changeAuthorizationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.changeAuthorizationSettings");
    s.store_field("flags", (var0 = flags_ | (confirmed_ << 3)));
    if (var0 & 8) { s.store_field("confirmed", true); }
    s.store_field("hash", hash_);
    if (var0 & 1) { s.store_field("encrypted_requests_disabled", encrypted_requests_disabled_); }
    if (var0 & 2) { s.store_field("call_requests_disabled", call_requests_disabled_); }
    s.store_class_end();
  }
}

account_changeAuthorizationSettings::ReturnType account_changeAuthorizationSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_checkUsername::account_checkUsername(string const &username_)
  : username_(username_)
{}

const std::int32_t account_checkUsername::ID;

void account_checkUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(655677548);
  TlStoreString::store(username_, s);
}

void account_checkUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(655677548);
  TlStoreString::store(username_, s);
}

void account_checkUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.checkUsername");
    s.store_field("username", username_);
    s.store_class_end();
  }
}

account_checkUsername::ReturnType account_checkUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_finishTakeoutSession::account_finishTakeoutSession(int32 flags_, bool success_)
  : flags_(flags_)
  , success_(success_)
{}

const std::int32_t account_finishTakeoutSession::ID;

void account_finishTakeoutSession::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(489050862);
  TlStoreBinary::store((var0 = flags_ | (success_ << 0)), s);
}

void account_finishTakeoutSession::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(489050862);
  TlStoreBinary::store((var0 = flags_ | (success_ << 0)), s);
}

void account_finishTakeoutSession::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.finishTakeoutSession");
    s.store_field("flags", (var0 = flags_ | (success_ << 0)));
    if (var0 & 1) { s.store_field("success", true); }
    s.store_class_end();
  }
}

account_finishTakeoutSession::ReturnType account_finishTakeoutSession::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_getChatThemes::account_getChatThemes(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getChatThemes::ID;

void account_getChatThemes::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-700916087);
  TlStoreBinary::store(hash_, s);
}

void account_getChatThemes::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-700916087);
  TlStoreBinary::store(hash_, s);
}

void account_getChatThemes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getChatThemes");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getChatThemes::ReturnType account_getChatThemes::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_Themes>::parse(p);
#undef FAIL
}

const std::int32_t account_getWebAuthorizations::ID;

void account_getWebAuthorizations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(405695855);
}

void account_getWebAuthorizations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(405695855);
}

void account_getWebAuthorizations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getWebAuthorizations");
    s.store_class_end();
  }
}

account_getWebAuthorizations::ReturnType account_getWebAuthorizations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_webAuthorizations>, -313079300>::parse(p);
#undef FAIL
}

account_resetAuthorization::account_resetAuthorization(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_resetAuthorization::ID;

void account_resetAuthorization::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-545786948);
  TlStoreBinary::store(hash_, s);
}

void account_resetAuthorization::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-545786948);
  TlStoreBinary::store(hash_, s);
}

void account_resetAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetAuthorization");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_resetAuthorization::ReturnType account_resetAuthorization::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_resetWallPapers::ID;

void account_resetWallPapers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1153722364);
}

void account_resetWallPapers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1153722364);
}

void account_resetWallPapers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetWallPapers");
    s.store_class_end();
  }
}

account_resetWallPapers::ReturnType account_resetWallPapers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_sendVerifyEmailCode::account_sendVerifyEmailCode(object_ptr<EmailVerifyPurpose> &&purpose_, string const &email_)
  : purpose_(std::move(purpose_))
  , email_(email_)
{}

const std::int32_t account_sendVerifyEmailCode::ID;

void account_sendVerifyEmailCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1730136133);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
  TlStoreString::store(email_, s);
}

void account_sendVerifyEmailCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1730136133);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
  TlStoreString::store(email_, s);
}

void account_sendVerifyEmailCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.sendVerifyEmailCode");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_field("email", email_);
    s.store_class_end();
  }
}

account_sendVerifyEmailCode::ReturnType account_sendVerifyEmailCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_sentEmailCode>, -2128640689>::parse(p);
#undef FAIL
}

account_updatePersonalChannel::account_updatePersonalChannel(object_ptr<InputChannel> &&channel_)
  : channel_(std::move(channel_))
{}

const std::int32_t account_updatePersonalChannel::ID;

void account_updatePersonalChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-649919008);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void account_updatePersonalChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-649919008);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void account_updatePersonalChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updatePersonalChannel");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

account_updatePersonalChannel::ReturnType account_updatePersonalChannel::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_cancelCode::auth_cancelCode(string const &phone_number_, string const &phone_code_hash_)
  : phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
{}

const std::int32_t auth_cancelCode::ID;

void auth_cancelCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(520357240);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
}

void auth_cancelCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(520357240);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
}

void auth_cancelCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.cancelCode");
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_class_end();
  }
}

auth_cancelCode::ReturnType auth_cancelCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_answerWebhookJSONQuery::bots_answerWebhookJSONQuery(int64 query_id_, object_ptr<dataJSON> &&data_)
  : query_id_(query_id_)
  , data_(std::move(data_))
{}

const std::int32_t bots_answerWebhookJSONQuery::ID;

void bots_answerWebhookJSONQuery::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-434028723);
  TlStoreBinary::store(query_id_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(data_, s);
}

void bots_answerWebhookJSONQuery::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-434028723);
  TlStoreBinary::store(query_id_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(data_, s);
}

void bots_answerWebhookJSONQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.answerWebhookJSONQuery");
    s.store_field("query_id", query_id_);
    s.store_object_field("data", static_cast<const BaseObject *>(data_.get()));
    s.store_class_end();
  }
}

bots_answerWebhookJSONQuery::ReturnType bots_answerWebhookJSONQuery::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_checkSearchPostsFlood::channels_checkSearchPostsFlood(int32 flags_, string const &query_)
  : flags_(flags_)
  , query_(query_)
{}

const std::int32_t channels_checkSearchPostsFlood::ID;

void channels_checkSearchPostsFlood::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(576090389);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(query_, s); }
}

void channels_checkSearchPostsFlood::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(576090389);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(query_, s); }
}

void channels_checkSearchPostsFlood::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.checkSearchPostsFlood");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("query", query_); }
    s.store_class_end();
  }
}

channels_checkSearchPostsFlood::ReturnType channels_checkSearchPostsFlood::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<searchPostsFlood>, 1040931690>::parse(p);
#undef FAIL
}

channels_editAdmin::channels_editAdmin(object_ptr<InputChannel> &&channel_, object_ptr<InputUser> &&user_id_, object_ptr<chatAdminRights> &&admin_rights_, string const &rank_)
  : channel_(std::move(channel_))
  , user_id_(std::move(user_id_))
  , admin_rights_(std::move(admin_rights_))
  , rank_(rank_)
{}

const std::int32_t channels_editAdmin::ID;

void channels_editAdmin::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-751007486);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxed<TlStoreObject, 1605510357>::store(admin_rights_, s);
  TlStoreString::store(rank_, s);
}

void channels_editAdmin::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-751007486);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxed<TlStoreObject, 1605510357>::store(admin_rights_, s);
  TlStoreString::store(rank_, s);
}

void channels_editAdmin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.editAdmin");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_object_field("admin_rights", static_cast<const BaseObject *>(admin_rights_.get()));
    s.store_field("rank", rank_);
    s.store_class_end();
  }
}

channels_editAdmin::ReturnType channels_editAdmin::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_getAdminLog::channels_getAdminLog(int32 flags_, object_ptr<InputChannel> &&channel_, string const &q_, object_ptr<channelAdminLogEventsFilter> &&events_filter_, array<object_ptr<InputUser>> &&admins_, int64 max_id_, int64 min_id_, int32 limit_)
  : flags_(flags_)
  , channel_(std::move(channel_))
  , q_(q_)
  , events_filter_(std::move(events_filter_))
  , admins_(std::move(admins_))
  , max_id_(max_id_)
  , min_id_(min_id_)
  , limit_(limit_)
{}

const std::int32_t channels_getAdminLog::ID;

void channels_getAdminLog::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(870184064);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(q_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -368018716>::store(events_filter_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(admins_, s); }
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(limit_, s);
}

void channels_getAdminLog::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(870184064);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(q_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -368018716>::store(events_filter_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(admins_, s); }
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(limit_, s);
}

void channels_getAdminLog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getAdminLog");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("q", q_);
    if (var0 & 1) { s.store_object_field("events_filter", static_cast<const BaseObject *>(events_filter_.get())); }
    if (var0 & 2) { { s.store_vector_begin("admins", admins_.size()); for (const auto &_value : admins_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("max_id", max_id_);
    s.store_field("min_id", min_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

channels_getAdminLog::ReturnType channels_getAdminLog::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<channels_adminLogResults>, -309659827>::parse(p);
#undef FAIL
}

const std::int32_t channels_getGroupsForDiscussion::ID;

void channels_getGroupsForDiscussion::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-170208392);
}

void channels_getGroupsForDiscussion::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-170208392);
}

void channels_getGroupsForDiscussion::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getGroupsForDiscussion");
    s.store_class_end();
  }
}

channels_getGroupsForDiscussion::ReturnType channels_getGroupsForDiscussion::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

const std::int32_t channels_getInactiveChannels::ID;

void channels_getInactiveChannels::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(300429806);
}

void channels_getInactiveChannels::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(300429806);
}

void channels_getInactiveChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getInactiveChannels");
    s.store_class_end();
  }
}

channels_getInactiveChannels::ReturnType channels_getInactiveChannels::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_inactiveChats>, -1456996667>::parse(p);
#undef FAIL
}

channels_readMessageContents::channels_readMessageContents(object_ptr<InputChannel> &&channel_, array<int32> &&id_)
  : channel_(std::move(channel_))
  , id_(std::move(id_))
{}

const std::int32_t channels_readMessageContents::ID;

void channels_readMessageContents::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-357180360);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void channels_readMessageContents::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-357180360);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void channels_readMessageContents::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.readMessageContents");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

channels_readMessageContents::ReturnType channels_readMessageContents::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_reportAntiSpamFalsePositive::channels_reportAntiSpamFalsePositive(object_ptr<InputChannel> &&channel_, int32 msg_id_)
  : channel_(std::move(channel_))
  , msg_id_(msg_id_)
{}

const std::int32_t channels_reportAntiSpamFalsePositive::ID;

void channels_reportAntiSpamFalsePositive::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1471109485);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
}

void channels_reportAntiSpamFalsePositive::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1471109485);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
}

void channels_reportAntiSpamFalsePositive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.reportAntiSpamFalsePositive");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

channels_reportAntiSpamFalsePositive::ReturnType channels_reportAntiSpamFalsePositive::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_reportSpam::channels_reportSpam(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_, array<int32> &&id_)
  : channel_(std::move(channel_))
  , participant_(std::move(participant_))
  , id_(std::move(id_))
{}

const std::int32_t channels_reportSpam::ID;

void channels_reportSpam::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-196443371);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void channels_reportSpam::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-196443371);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void channels_reportSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.reportSpam");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

channels_reportSpam::ReturnType channels_reportSpam::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_toggleForum::channels_toggleForum(object_ptr<InputChannel> &&channel_, bool enabled_, bool tabs_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
  , tabs_(tabs_)
{}

const std::int32_t channels_toggleForum::ID;

void channels_toggleForum::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1073174324);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
  TlStoreBool::store(tabs_, s);
}

void channels_toggleForum::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1073174324);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
  TlStoreBool::store(tabs_, s);
}

void channels_toggleForum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleForum");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_field("tabs", tabs_);
    s.store_class_end();
  }
}

channels_toggleForum::ReturnType channels_toggleForum::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

chatlists_getChatlistUpdates::chatlists_getChatlistUpdates(object_ptr<inputChatlistDialogFilter> &&chatlist_)
  : chatlist_(std::move(chatlist_))
{}

const std::int32_t chatlists_getChatlistUpdates::ID;

void chatlists_getChatlistUpdates::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1992190687);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_getChatlistUpdates::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1992190687);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_getChatlistUpdates::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.getChatlistUpdates");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    s.store_class_end();
  }
}

chatlists_getChatlistUpdates::ReturnType chatlists_getChatlistUpdates::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<chatlists_chatlistUpdates>, -1816295539>::parse(p);
#undef FAIL
}

chatlists_getLeaveChatlistSuggestions::chatlists_getLeaveChatlistSuggestions(object_ptr<inputChatlistDialogFilter> &&chatlist_)
  : chatlist_(std::move(chatlist_))
{}

const std::int32_t chatlists_getLeaveChatlistSuggestions::ID;

void chatlists_getLeaveChatlistSuggestions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-37955820);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_getLeaveChatlistSuggestions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-37955820);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_getLeaveChatlistSuggestions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.getLeaveChatlistSuggestions");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    s.store_class_end();
  }
}

chatlists_getLeaveChatlistSuggestions::ReturnType chatlists_getLeaveChatlistSuggestions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p);
#undef FAIL
}

chatlists_joinChatlistUpdates::chatlists_joinChatlistUpdates(object_ptr<inputChatlistDialogFilter> &&chatlist_, array<object_ptr<InputPeer>> &&peers_)
  : chatlist_(std::move(chatlist_))
  , peers_(std::move(peers_))
{}

const std::int32_t chatlists_joinChatlistUpdates::ID;

void chatlists_joinChatlistUpdates::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-527828747);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_joinChatlistUpdates::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-527828747);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_joinChatlistUpdates::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.joinChatlistUpdates");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatlists_joinChatlistUpdates::ReturnType chatlists_joinChatlistUpdates::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_toggleTopPeers::contacts_toggleTopPeers(bool enabled_)
  : enabled_(enabled_)
{}

const std::int32_t contacts_toggleTopPeers::ID;

void contacts_toggleTopPeers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2062238246);
  TlStoreBool::store(enabled_, s);
}

void contacts_toggleTopPeers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2062238246);
  TlStoreBool::store(enabled_, s);
}

void contacts_toggleTopPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.toggleTopPeers");
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

contacts_toggleTopPeers::ReturnType contacts_toggleTopPeers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

help_getCountriesList::help_getCountriesList(string const &lang_code_, int32 hash_)
  : lang_code_(lang_code_)
  , hash_(hash_)
{}

const std::int32_t help_getCountriesList::ID;

void help_getCountriesList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1935116200);
  TlStoreString::store(lang_code_, s);
  TlStoreBinary::store(hash_, s);
}

void help_getCountriesList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1935116200);
  TlStoreString::store(lang_code_, s);
  TlStoreBinary::store(hash_, s);
}

void help_getCountriesList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getCountriesList");
    s.store_field("lang_code", lang_code_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

help_getCountriesList::ReturnType help_getCountriesList::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_CountriesList>::parse(p);
#undef FAIL
}

help_saveAppLog::help_saveAppLog(array<object_ptr<inputAppEvent>> &&events_)
  : events_(std::move(events_))
{}

const std::int32_t help_saveAppLog::ID;

void help_saveAppLog::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1862465352);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 488313413>>, 481674261>::store(events_, s);
}

void help_saveAppLog::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1862465352);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 488313413>>, 481674261>::store(events_, s);
}

void help_saveAppLog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.saveAppLog");
    { s.store_vector_begin("events", events_.size()); for (const auto &_value : events_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

help_saveAppLog::ReturnType help_saveAppLog::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_getCustomEmojiDocuments::messages_getCustomEmojiDocuments(array<int64> &&document_id_)
  : document_id_(std::move(document_id_))
{}

const std::int32_t messages_getCustomEmojiDocuments::ID;

void messages_getCustomEmojiDocuments::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-643100844);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(document_id_, s);
}

void messages_getCustomEmojiDocuments::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-643100844);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(document_id_, s);
}

void messages_getCustomEmojiDocuments::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getCustomEmojiDocuments");
    { s.store_vector_begin("document_id", document_id_.size()); for (const auto &_value : document_id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getCustomEmojiDocuments::ReturnType messages_getCustomEmojiDocuments::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p);
#undef FAIL
}

const std::int32_t messages_getDefaultHistoryTTL::ID;

void messages_getDefaultHistoryTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1703637384);
}

void messages_getDefaultHistoryTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1703637384);
}

void messages_getDefaultHistoryTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDefaultHistoryTTL");
    s.store_class_end();
  }
}

messages_getDefaultHistoryTTL::ReturnType messages_getDefaultHistoryTTL::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<defaultHistoryTTL>, 1135897376>::parse(p);
#undef FAIL
}

const std::int32_t messages_getDialogFilters::ID;

void messages_getDialogFilters::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-271283063);
}

void messages_getDialogFilters::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-271283063);
}

void messages_getDialogFilters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDialogFilters");
    s.store_class_end();
  }
}

messages_getDialogFilters::ReturnType messages_getDialogFilters::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_dialogFilters>, 718878489>::parse(p);
#undef FAIL
}

messages_getEmojiKeywordsLanguages::messages_getEmojiKeywordsLanguages(array<string> &&lang_codes_)
  : lang_codes_(std::move(lang_codes_))
{}

const std::int32_t messages_getEmojiKeywordsLanguages::ID;

void messages_getEmojiKeywordsLanguages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1318675378);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(lang_codes_, s);
}

void messages_getEmojiKeywordsLanguages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1318675378);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(lang_codes_, s);
}

void messages_getEmojiKeywordsLanguages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiKeywordsLanguages");
    { s.store_vector_begin("lang_codes", lang_codes_.size()); for (const auto &_value : lang_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getEmojiKeywordsLanguages::ReturnType messages_getEmojiKeywordsLanguages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<emojiLanguage>, -1275374751>>, 481674261>::parse(p);
#undef FAIL
}

messages_getEmojiProfilePhotoGroups::messages_getEmojiProfilePhotoGroups(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getEmojiProfilePhotoGroups::ID;

void messages_getEmojiProfilePhotoGroups::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(564480243);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiProfilePhotoGroups::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(564480243);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiProfilePhotoGroups::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiProfilePhotoGroups");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getEmojiProfilePhotoGroups::ReturnType messages_getEmojiProfilePhotoGroups::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_EmojiGroups>::parse(p);
#undef FAIL
}

messages_getExtendedMedia::messages_getExtendedMedia(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t messages_getExtendedMedia::ID;

void messages_getExtendedMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2064119788);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getExtendedMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2064119788);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getExtendedMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getExtendedMedia");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getExtendedMedia::ReturnType messages_getExtendedMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getGameHighScores::messages_getGameHighScores(object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<InputUser> &&user_id_)
  : peer_(std::move(peer_))
  , id_(id_)
  , user_id_(std::move(user_id_))
{}

const std::int32_t messages_getGameHighScores::ID;

void messages_getGameHighScores::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-400399203);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_getGameHighScores::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-400399203);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_getGameHighScores::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getGameHighScores");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

messages_getGameHighScores::ReturnType messages_getGameHighScores::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_highScores>, -1707344487>::parse(p);
#undef FAIL
}

messages_getMessageReadParticipants::messages_getMessageReadParticipants(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t messages_getMessageReadParticipants::ID;

void messages_getMessageReadParticipants::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(834782287);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getMessageReadParticipants::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(834782287);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getMessageReadParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMessageReadParticipants");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

messages_getMessageReadParticipants::ReturnType messages_getMessageReadParticipants::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<readParticipantDate>, 1246753138>>, 481674261>::parse(p);
#undef FAIL
}

messages_getPeerDialogs::messages_getPeerDialogs(array<object_ptr<InputDialogPeer>> &&peers_)
  : peers_(std::move(peers_))
{}

const std::int32_t messages_getPeerDialogs::ID;

void messages_getPeerDialogs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-462373635);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void messages_getPeerDialogs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-462373635);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void messages_getPeerDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPeerDialogs");
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getPeerDialogs::ReturnType messages_getPeerDialogs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_peerDialogs>, 863093588>::parse(p);
#undef FAIL
}

messages_getPeerSettings::messages_getPeerSettings(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t messages_getPeerSettings::ID;

void messages_getPeerSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-270948702);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_getPeerSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-270948702);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_getPeerSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPeerSettings");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_getPeerSettings::ReturnType messages_getPeerSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_peerSettings>, 1753266509>::parse(p);
#undef FAIL
}

messages_getQuickReplyMessages::messages_getQuickReplyMessages(int32 flags_, int32 shortcut_id_, array<int32> &&id_, int64 hash_)
  : flags_(flags_)
  , shortcut_id_(shortcut_id_)
  , id_(std::move(id_))
  , hash_(hash_)
{}

const std::int32_t messages_getQuickReplyMessages::ID;

void messages_getQuickReplyMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1801153085);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(shortcut_id_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s); }
  TlStoreBinary::store(hash_, s);
}

void messages_getQuickReplyMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1801153085);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(shortcut_id_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s); }
  TlStoreBinary::store(hash_, s);
}

void messages_getQuickReplyMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getQuickReplyMessages");
    s.store_field("flags", (var0 = flags_));
    s.store_field("shortcut_id", shortcut_id_);
    if (var0 & 1) { { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getQuickReplyMessages::ReturnType messages_getQuickReplyMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_getSavedReactionTags::messages_getSavedReactionTags(int32 flags_, object_ptr<InputPeer> &&peer_, int64 hash_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , hash_(hash_)
{}

const std::int32_t messages_getSavedReactionTags::ID;

void messages_getSavedReactionTags::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(909631579);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedReactionTags::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(909631579);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedReactionTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSavedReactionTags");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getSavedReactionTags::ReturnType messages_getSavedReactionTags::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SavedReactionTags>::parse(p);
#undef FAIL
}

messages_getSponsoredMessages::messages_getSponsoredMessages(int32 flags_, object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t messages_getSponsoredMessages::ID;

void messages_getSponsoredMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1030547536);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(msg_id_, s); }
}

void messages_getSponsoredMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1030547536);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(msg_id_, s); }
}

void messages_getSponsoredMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSponsoredMessages");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("msg_id", msg_id_); }
    s.store_class_end();
  }
}

messages_getSponsoredMessages::ReturnType messages_getSponsoredMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SponsoredMessages>::parse(p);
#undef FAIL
}

messages_initHistoryImport::messages_initHistoryImport(object_ptr<InputPeer> &&peer_, object_ptr<InputFile> &&file_, int32 media_count_)
  : peer_(std::move(peer_))
  , file_(std::move(file_))
  , media_count_(media_count_)
{}

const std::int32_t messages_initHistoryImport::ID;

void messages_initHistoryImport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(873008187);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  TlStoreBinary::store(media_count_, s);
}

void messages_initHistoryImport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(873008187);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  TlStoreBinary::store(media_count_, s);
}

void messages_initHistoryImport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.initHistoryImport");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_field("media_count", media_count_);
    s.store_class_end();
  }
}

messages_initHistoryImport::ReturnType messages_initHistoryImport::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_historyImport>, 375566091>::parse(p);
#undef FAIL
}

messages_receivedMessages::messages_receivedMessages(int32 max_id_)
  : max_id_(max_id_)
{}

const std::int32_t messages_receivedMessages::ID;

void messages_receivedMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(94983360);
  TlStoreBinary::store(max_id_, s);
}

void messages_receivedMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(94983360);
  TlStoreBinary::store(max_id_, s);
}

void messages_receivedMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.receivedMessages");
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

messages_receivedMessages::ReturnType messages_receivedMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<receivedNotifyMessage>, -1551583367>>, 481674261>::parse(p);
#undef FAIL
}

messages_sendEncryptedService::messages_sendEncryptedService(object_ptr<inputEncryptedChat> &&peer_, int64 random_id_, bytes &&data_)
  : peer_(std::move(peer_))
  , random_id_(random_id_)
  , data_(std::move(data_))
{}

const std::int32_t messages_sendEncryptedService::ID;

void messages_sendEncryptedService::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(852769188);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(data_, s);
}

void messages_sendEncryptedService::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(852769188);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(data_, s);
}

void messages_sendEncryptedService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendEncryptedService");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("random_id", random_id_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

messages_sendEncryptedService::ReturnType messages_sendEncryptedService::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SentEncryptedMessage>::parse(p);
#undef FAIL
}

messages_sendScreenshotNotification::messages_sendScreenshotNotification(object_ptr<InputPeer> &&peer_, object_ptr<InputReplyTo> &&reply_to_, int64 random_id_)
  : peer_(std::move(peer_))
  , reply_to_(std::move(reply_to_))
  , random_id_(random_id_)
{}

const std::int32_t messages_sendScreenshotNotification::ID;

void messages_sendScreenshotNotification::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1589618665);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s);
  TlStoreBinary::store(random_id_, s);
}

void messages_sendScreenshotNotification::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1589618665);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s);
  TlStoreBinary::store(random_id_, s);
}

void messages_sendScreenshotNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendScreenshotNotification");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_field("random_id", random_id_);
    s.store_class_end();
  }
}

messages_sendScreenshotNotification::ReturnType messages_sendScreenshotNotification::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_startHistoryImport::messages_startHistoryImport(object_ptr<InputPeer> &&peer_, int64 import_id_)
  : peer_(std::move(peer_))
  , import_id_(import_id_)
{}

const std::int32_t messages_startHistoryImport::ID;

void messages_startHistoryImport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1271008444);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(import_id_, s);
}

void messages_startHistoryImport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1271008444);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(import_id_, s);
}

void messages_startHistoryImport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.startHistoryImport");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("import_id", import_id_);
    s.store_class_end();
  }
}

messages_startHistoryImport::ReturnType messages_startHistoryImport::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_updateDialogFilter::messages_updateDialogFilter(int32 flags_, int32 id_, object_ptr<DialogFilter> &&filter_)
  : flags_(flags_)
  , id_(id_)
  , filter_(std::move(filter_))
{}

const std::int32_t messages_updateDialogFilter::ID;

void messages_updateDialogFilter::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(450142282);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s); }
}

void messages_updateDialogFilter::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(450142282);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s); }
}

void messages_updateDialogFilter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.updateDialogFilter");
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get())); }
    s.store_class_end();
  }
}

messages_updateDialogFilter::ReturnType messages_updateDialogFilter::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_updateSavedReactionTag::messages_updateSavedReactionTag(int32 flags_, object_ptr<Reaction> &&reaction_, string const &title_)
  : flags_(flags_)
  , reaction_(std::move(reaction_))
  , title_(title_)
{}

const std::int32_t messages_updateSavedReactionTag::ID;

void messages_updateSavedReactionTag::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1613331948);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
}

void messages_updateSavedReactionTag::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1613331948);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
}

void messages_updateSavedReactionTag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.updateSavedReactionTag");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    if (var0 & 1) { s.store_field("title", title_); }
    s.store_class_end();
  }
}

messages_updateSavedReactionTag::ReturnType messages_updateSavedReactionTag::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_uploadEncryptedFile::messages_uploadEncryptedFile(object_ptr<inputEncryptedChat> &&peer_, object_ptr<InputEncryptedFile> &&file_)
  : peer_(std::move(peer_))
  , file_(std::move(file_))
{}

const std::int32_t messages_uploadEncryptedFile::ID;

void messages_uploadEncryptedFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1347929239);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
}

void messages_uploadEncryptedFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1347929239);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
}

void messages_uploadEncryptedFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.uploadEncryptedFile");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_class_end();
  }
}

messages_uploadEncryptedFile::ReturnType messages_uploadEncryptedFile::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EncryptedFile>::parse(p);
#undef FAIL
}

messages_viewSponsoredMessage::messages_viewSponsoredMessage(bytes &&random_id_)
  : random_id_(std::move(random_id_))
{}

const std::int32_t messages_viewSponsoredMessage::ID;

void messages_viewSponsoredMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(647902787);
  TlStoreString::store(random_id_, s);
}

void messages_viewSponsoredMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(647902787);
  TlStoreString::store(random_id_, s);
}

void messages_viewSponsoredMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.viewSponsoredMessage");
    s.store_bytes_field("random_id", random_id_);
    s.store_class_end();
  }
}

messages_viewSponsoredMessage::ReturnType messages_viewSponsoredMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_fulfillStarsSubscription::payments_fulfillStarsSubscription(object_ptr<InputPeer> &&peer_, string const &subscription_id_)
  : peer_(std::move(peer_))
  , subscription_id_(subscription_id_)
{}

const std::int32_t payments_fulfillStarsSubscription::ID;

void payments_fulfillStarsSubscription::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-866391117);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(subscription_id_, s);
}

void payments_fulfillStarsSubscription::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-866391117);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(subscription_id_, s);
}

void payments_fulfillStarsSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.fulfillStarsSubscription");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("subscription_id", subscription_id_);
    s.store_class_end();
  }
}

payments_fulfillStarsSubscription::ReturnType payments_fulfillStarsSubscription::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_getBankCardData::payments_getBankCardData(string const &number_)
  : number_(number_)
{}

const std::int32_t payments_getBankCardData::ID;

void payments_getBankCardData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(779736953);
  TlStoreString::store(number_, s);
}

void payments_getBankCardData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(779736953);
  TlStoreString::store(number_, s);
}

void payments_getBankCardData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getBankCardData");
    s.store_field("number", number_);
    s.store_class_end();
  }
}

payments_getBankCardData::ReturnType payments_getBankCardData::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_bankCardData>, 1042605427>::parse(p);
#undef FAIL
}

payments_getPremiumGiftCodeOptions::payments_getPremiumGiftCodeOptions(int32 flags_, object_ptr<InputPeer> &&boost_peer_)
  : flags_(flags_)
  , boost_peer_(std::move(boost_peer_))
{}

const std::int32_t payments_getPremiumGiftCodeOptions::ID;

void payments_getPremiumGiftCodeOptions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(660060756);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s); }
}

void payments_getPremiumGiftCodeOptions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(660060756);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s); }
}

void payments_getPremiumGiftCodeOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getPremiumGiftCodeOptions");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("boost_peer", static_cast<const BaseObject *>(boost_peer_.get())); }
    s.store_class_end();
  }
}

payments_getPremiumGiftCodeOptions::ReturnType payments_getPremiumGiftCodeOptions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<premiumGiftCodeOption>, 629052971>>, 481674261>::parse(p);
#undef FAIL
}

payments_getStarGiftUpgradePreview::payments_getStarGiftUpgradePreview(int64 gift_id_)
  : gift_id_(gift_id_)
{}

const std::int32_t payments_getStarGiftUpgradePreview::ID;

void payments_getStarGiftUpgradePreview::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1667580751);
  TlStoreBinary::store(gift_id_, s);
}

void payments_getStarGiftUpgradePreview::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1667580751);
  TlStoreBinary::store(gift_id_, s);
}

void payments_getStarGiftUpgradePreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarGiftUpgradePreview");
    s.store_field("gift_id", gift_id_);
    s.store_class_end();
  }
}

payments_getStarGiftUpgradePreview::ReturnType payments_getStarGiftUpgradePreview::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starGiftUpgradePreview>, 1038213101>::parse(p);
#undef FAIL
}

payments_getSuggestedStarRefBots::payments_getSuggestedStarRefBots(int32 flags_, bool order_by_revenue_, bool order_by_date_, object_ptr<InputPeer> &&peer_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , order_by_revenue_(order_by_revenue_)
  , order_by_date_(order_by_date_)
  , peer_(std::move(peer_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t payments_getSuggestedStarRefBots::ID;

void payments_getSuggestedStarRefBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(225134839);
  TlStoreBinary::store((var0 = flags_ | (order_by_revenue_ << 0) | (order_by_date_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getSuggestedStarRefBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(225134839);
  TlStoreBinary::store((var0 = flags_ | (order_by_revenue_ << 0) | (order_by_date_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getSuggestedStarRefBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getSuggestedStarRefBots");
    s.store_field("flags", (var0 = flags_ | (order_by_revenue_ << 0) | (order_by_date_ << 1)));
    if (var0 & 1) { s.store_field("order_by_revenue", true); }
    if (var0 & 2) { s.store_field("order_by_date", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

payments_getSuggestedStarRefBots::ReturnType payments_getSuggestedStarRefBots::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_suggestedStarRefBots>, -1261053863>::parse(p);
#undef FAIL
}

payments_updateStarGiftPrice::payments_updateStarGiftPrice(object_ptr<InputSavedStarGift> &&stargift_, object_ptr<StarsAmount> &&resell_amount_)
  : stargift_(std::move(stargift_))
  , resell_amount_(std::move(resell_amount_))
{}

const std::int32_t payments_updateStarGiftPrice::ID;

void payments_updateStarGiftPrice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-306287413);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(resell_amount_, s);
}

void payments_updateStarGiftPrice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-306287413);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(resell_amount_, s);
}

void payments_updateStarGiftPrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.updateStarGiftPrice");
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_object_field("resell_amount", static_cast<const BaseObject *>(resell_amount_.get()));
    s.store_class_end();
  }
}

payments_updateStarGiftPrice::ReturnType payments_updateStarGiftPrice::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_declineConferenceCallInvite::phone_declineConferenceCallInvite(int32 msg_id_)
  : msg_id_(msg_id_)
{}

const std::int32_t phone_declineConferenceCallInvite::ID;

void phone_declineConferenceCallInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1011325297);
  TlStoreBinary::store(msg_id_, s);
}

void phone_declineConferenceCallInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1011325297);
  TlStoreBinary::store(msg_id_, s);
}

void phone_declineConferenceCallInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.declineConferenceCallInvite");
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

phone_declineConferenceCallInvite::ReturnType phone_declineConferenceCallInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_getGroupCallJoinAs::phone_getGroupCallJoinAs(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t phone_getGroupCallJoinAs::ID;

void phone_getGroupCallJoinAs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-277077702);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void phone_getGroupCallJoinAs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-277077702);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void phone_getGroupCallJoinAs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.getGroupCallJoinAs");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

phone_getGroupCallJoinAs::ReturnType phone_getGroupCallJoinAs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_joinAsPeers>, -1343921601>::parse(p);
#undef FAIL
}

phone_joinGroupCall::phone_joinGroupCall(int32 flags_, bool muted_, bool video_stopped_, object_ptr<InputGroupCall> &&call_, object_ptr<InputPeer> &&join_as_, string const &invite_hash_, UInt256 const &public_key_, bytes &&block_, object_ptr<dataJSON> &&params_)
  : flags_(flags_)
  , muted_(muted_)
  , video_stopped_(video_stopped_)
  , call_(std::move(call_))
  , join_as_(std::move(join_as_))
  , invite_hash_(invite_hash_)
  , public_key_(public_key_)
  , block_(std::move(block_))
  , params_(std::move(params_))
{}

const std::int32_t phone_joinGroupCall::ID;

void phone_joinGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1883951017);
  TlStoreBinary::store((var0 = flags_ | (muted_ << 0) | (video_stopped_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(join_as_, s);
  if (var0 & 2) { TlStoreString::store(invite_hash_, s); }
  if (var0 & 8) { TlStoreBinary::store(public_key_, s); }
  if (var0 & 8) { TlStoreString::store(block_, s); }
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void phone_joinGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1883951017);
  TlStoreBinary::store((var0 = flags_ | (muted_ << 0) | (video_stopped_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(join_as_, s);
  if (var0 & 2) { TlStoreString::store(invite_hash_, s); }
  if (var0 & 8) { TlStoreBinary::store(public_key_, s); }
  if (var0 & 8) { TlStoreString::store(block_, s); }
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void phone_joinGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.joinGroupCall");
    s.store_field("flags", (var0 = flags_ | (muted_ << 0) | (video_stopped_ << 2)));
    if (var0 & 1) { s.store_field("muted", true); }
    if (var0 & 4) { s.store_field("video_stopped", true); }
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_object_field("join_as", static_cast<const BaseObject *>(join_as_.get()));
    if (var0 & 2) { s.store_field("invite_hash", invite_hash_); }
    if (var0 & 8) { s.store_field("public_key", public_key_); }
    if (var0 & 8) { s.store_bytes_field("block", block_); }
    s.store_object_field("params", static_cast<const BaseObject *>(params_.get()));
    s.store_class_end();
  }
}

phone_joinGroupCall::ReturnType phone_joinGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_saveCallLog::phone_saveCallLog(object_ptr<inputPhoneCall> &&peer_, object_ptr<InputFile> &&file_)
  : peer_(std::move(peer_))
  , file_(std::move(file_))
{}

const std::int32_t phone_saveCallLog::ID;

void phone_saveCallLog::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1092913030);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
}

void phone_saveCallLog::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1092913030);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
}

void phone_saveCallLog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.saveCallLog");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_class_end();
  }
}

phone_saveCallLog::ReturnType phone_saveCallLog::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

photos_deletePhotos::photos_deletePhotos(array<object_ptr<InputPhoto>> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t photos_deletePhotos::ID;

void photos_deletePhotos::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2016444625);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void photos_deletePhotos::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2016444625);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void photos_deletePhotos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.deletePhotos");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

photos_deletePhotos::ReturnType photos_deletePhotos::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p);
#undef FAIL
}

photos_uploadProfilePhoto::photos_uploadProfilePhoto(int32 flags_, bool fallback_, object_ptr<InputUser> &&bot_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&video_, double video_start_ts_, object_ptr<VideoSize> &&video_emoji_markup_)
  : flags_(flags_)
  , fallback_(fallback_)
  , bot_(std::move(bot_))
  , file_(std::move(file_))
  , video_(std::move(video_))
  , video_start_ts_(video_start_ts_)
  , video_emoji_markup_(std::move(video_emoji_markup_))
{}

const std::int32_t photos_uploadProfilePhoto::ID;

void photos_uploadProfilePhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(59286453);
  TlStoreBinary::store((var0 = flags_ | (fallback_ << 3)), s);
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(file_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(video_, s); }
  if (var0 & 4) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 16) { TlStoreBoxedUnknown<TlStoreObject>::store(video_emoji_markup_, s); }
}

void photos_uploadProfilePhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(59286453);
  TlStoreBinary::store((var0 = flags_ | (fallback_ << 3)), s);
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(file_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(video_, s); }
  if (var0 & 4) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 16) { TlStoreBoxedUnknown<TlStoreObject>::store(video_emoji_markup_, s); }
}

void photos_uploadProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.uploadProfilePhoto");
    s.store_field("flags", (var0 = flags_ | (fallback_ << 3)));
    if (var0 & 8) { s.store_field("fallback", true); }
    if (var0 & 32) { s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get())); }
    if (var0 & 1) { s.store_object_field("file", static_cast<const BaseObject *>(file_.get())); }
    if (var0 & 2) { s.store_object_field("video", static_cast<const BaseObject *>(video_.get())); }
    if (var0 & 4) { s.store_field("video_start_ts", video_start_ts_); }
    if (var0 & 16) { s.store_object_field("video_emoji_markup", static_cast<const BaseObject *>(video_emoji_markup_.get())); }
    s.store_class_end();
  }
}

photos_uploadProfilePhoto::ReturnType photos_uploadProfilePhoto::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<photos_photo>, 539045032>::parse(p);
#undef FAIL
}

premium_getBoostsStatus::premium_getBoostsStatus(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t premium_getBoostsStatus::ID;

void premium_getBoostsStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(70197089);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void premium_getBoostsStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(70197089);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void premium_getBoostsStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.getBoostsStatus");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

premium_getBoostsStatus::ReturnType premium_getBoostsStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<premium_boostsStatus>, 1230586490>::parse(p);
#undef FAIL
}

smsjobs_updateSettings::smsjobs_updateSettings(int32 flags_, bool allow_international_)
  : flags_(flags_)
  , allow_international_(allow_international_)
{}

const std::int32_t smsjobs_updateSettings::ID;

void smsjobs_updateSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(155164863);
  TlStoreBinary::store((var0 = flags_ | (allow_international_ << 0)), s);
}

void smsjobs_updateSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(155164863);
  TlStoreBinary::store((var0 = flags_ | (allow_international_ << 0)), s);
}

void smsjobs_updateSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.updateSettings");
    s.store_field("flags", (var0 = flags_ | (allow_international_ << 0)));
    if (var0 & 1) { s.store_field("allow_international", true); }
    s.store_class_end();
  }
}

smsjobs_updateSettings::ReturnType smsjobs_updateSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stats_getMessagePublicForwards::stats_getMessagePublicForwards(object_ptr<InputChannel> &&channel_, int32 msg_id_, string const &offset_, int32 limit_)
  : channel_(std::move(channel_))
  , msg_id_(msg_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t stats_getMessagePublicForwards::ID;

void stats_getMessagePublicForwards::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1595212100);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stats_getMessagePublicForwards::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1595212100);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stats_getMessagePublicForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.getMessagePublicForwards");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stats_getMessagePublicForwards::ReturnType stats_getMessagePublicForwards::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stats_publicForwards>, -1828487648>::parse(p);
#undef FAIL
}

stats_getMessageStats::stats_getMessageStats(int32 flags_, bool dark_, object_ptr<InputChannel> &&channel_, int32 msg_id_)
  : flags_(flags_)
  , dark_(dark_)
  , channel_(std::move(channel_))
  , msg_id_(msg_id_)
{}

const std::int32_t stats_getMessageStats::ID;

void stats_getMessageStats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1226791947);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
}

void stats_getMessageStats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1226791947);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(msg_id_, s);
}

void stats_getMessageStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.getMessageStats");
    s.store_field("flags", (var0 = flags_ | (dark_ << 0)));
    if (var0 & 1) { s.store_field("dark", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

stats_getMessageStats::ReturnType stats_getMessageStats::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stats_messageStats>, 2145983508>::parse(p);
#undef FAIL
}

stats_loadAsyncGraph::stats_loadAsyncGraph(int32 flags_, string const &token_, int64 x_)
  : flags_(flags_)
  , token_(token_)
  , x_(x_)
{}

const std::int32_t stats_loadAsyncGraph::ID;

void stats_loadAsyncGraph::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1646092192);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(token_, s);
  if (var0 & 1) { TlStoreBinary::store(x_, s); }
}

void stats_loadAsyncGraph::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1646092192);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(token_, s);
  if (var0 & 1) { TlStoreBinary::store(x_, s); }
}

void stats_loadAsyncGraph::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.loadAsyncGraph");
    s.store_field("flags", (var0 = flags_));
    s.store_field("token", token_);
    if (var0 & 1) { s.store_field("x", x_); }
    s.store_class_end();
  }
}

stats_loadAsyncGraph::ReturnType stats_loadAsyncGraph::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<StatsGraph>::parse(p);
#undef FAIL
}

stickers_createStickerSet::stickers_createStickerSet(int32 flags_, bool masks_, bool emojis_, bool text_color_, object_ptr<InputUser> &&user_id_, string const &title_, string const &short_name_, object_ptr<InputDocument> &&thumb_, array<object_ptr<inputStickerSetItem>> &&stickers_, string const &software_)
  : flags_(flags_)
  , masks_(masks_)
  , emojis_(emojis_)
  , text_color_(text_color_)
  , user_id_(std::move(user_id_))
  , title_(title_)
  , short_name_(short_name_)
  , thumb_(std::move(thumb_))
  , stickers_(std::move(stickers_))
  , software_(software_)
{}

const std::int32_t stickers_createStickerSet::ID;

void stickers_createStickerSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1876841625);
  TlStoreBinary::store((var0 = flags_ | (masks_ << 0) | (emojis_ << 5) | (text_color_ << 6)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(short_name_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 853188252>>, 481674261>::store(stickers_, s);
  if (var0 & 8) { TlStoreString::store(software_, s); }
}

void stickers_createStickerSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1876841625);
  TlStoreBinary::store((var0 = flags_ | (masks_ << 0) | (emojis_ << 5) | (text_color_ << 6)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(short_name_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 853188252>>, 481674261>::store(stickers_, s);
  if (var0 & 8) { TlStoreString::store(software_, s); }
}

void stickers_createStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.createStickerSet");
    s.store_field("flags", (var0 = flags_ | (masks_ << 0) | (emojis_ << 5) | (text_color_ << 6)));
    if (var0 & 1) { s.store_field("masks", true); }
    if (var0 & 32) { s.store_field("emojis", true); }
    if (var0 & 64) { s.store_field("text_color", true); }
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("title", title_);
    s.store_field("short_name", short_name_);
    if (var0 & 4) { s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get())); }
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 8) { s.store_field("software", software_); }
    s.store_class_end();
  }
}

stickers_createStickerSet::ReturnType stickers_createStickerSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stories_deleteAlbum::stories_deleteAlbum(object_ptr<InputPeer> &&peer_, int32 album_id_)
  : peer_(std::move(peer_))
  , album_id_(album_id_)
{}

const std::int32_t stories_deleteAlbum::ID;

void stories_deleteAlbum::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1925949744);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(album_id_, s);
}

void stories_deleteAlbum::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1925949744);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(album_id_, s);
}

void stories_deleteAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.deleteAlbum");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("album_id", album_id_);
    s.store_class_end();
  }
}

stories_deleteAlbum::ReturnType stories_deleteAlbum::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stories_getAlbumStories::stories_getAlbumStories(object_ptr<InputPeer> &&peer_, int32 album_id_, int32 offset_, int32 limit_)
  : peer_(std::move(peer_))
  , album_id_(album_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t stories_getAlbumStories::ID;

void stories_getAlbumStories::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1400869535);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(album_id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getAlbumStories::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1400869535);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(album_id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getAlbumStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getAlbumStories");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("album_id", album_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stories_getAlbumStories::ReturnType stories_getAlbumStories::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_stories>, 1673780490>::parse(p);
#undef FAIL
}

stories_report::stories_report(object_ptr<InputPeer> &&peer_, array<int32> &&id_, bytes &&option_, string const &message_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
  , option_(std::move(option_))
  , message_(message_)
{}

const std::int32_t stories_report::ID;

void stories_report::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(433646405);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreString::store(option_, s);
  TlStoreString::store(message_, s);
}

void stories_report::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(433646405);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreString::store(option_, s);
  TlStoreString::store(message_, s);
}

void stories_report::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.report");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_bytes_field("option", option_);
    s.store_field("message", message_);
    s.store_class_end();
  }
}

stories_report::ReturnType stories_report::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<ReportResult>::parse(p);
#undef FAIL
}

stories_updateAlbum::stories_updateAlbum(int32 flags_, object_ptr<InputPeer> &&peer_, int32 album_id_, string const &title_, array<int32> &&delete_stories_, array<int32> &&add_stories_, array<int32> &&order_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , album_id_(album_id_)
  , title_(title_)
  , delete_stories_(std::move(delete_stories_))
  , add_stories_(std::move(add_stories_))
  , order_(std::move(order_))
{}

const std::int32_t stories_updateAlbum::ID;

void stories_updateAlbum::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1582455222);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(album_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(delete_stories_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(add_stories_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s); }
}

void stories_updateAlbum::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1582455222);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(album_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(delete_stories_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(add_stories_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s); }
}

void stories_updateAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.updateAlbum");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("album_id", album_id_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { { s.store_vector_begin("delete_stories", delete_stories_.size()); for (const auto &_value : delete_stories_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 4) { { s.store_vector_begin("add_stories", add_stories_.size()); for (const auto &_value : add_stories_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 8) { { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

stories_updateAlbum::ReturnType stories_updateAlbum::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<storyAlbum>, -1826262950>::parse(p);
#undef FAIL
}

upload_saveBigFilePart::upload_saveBigFilePart(int64 file_id_, int32 file_part_, int32 file_total_parts_, bytes &&bytes_)
  : file_id_(file_id_)
  , file_part_(file_part_)
  , file_total_parts_(file_total_parts_)
  , bytes_(std::move(bytes_))
{}

const std::int32_t upload_saveBigFilePart::ID;

void upload_saveBigFilePart::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-562337987);
  TlStoreBinary::store(file_id_, s);
  TlStoreBinary::store(file_part_, s);
  TlStoreBinary::store(file_total_parts_, s);
  TlStoreString::store(bytes_, s);
}

void upload_saveBigFilePart::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-562337987);
  TlStoreBinary::store(file_id_, s);
  TlStoreBinary::store(file_part_, s);
  TlStoreBinary::store(file_total_parts_, s);
  TlStoreString::store(bytes_, s);
}

void upload_saveBigFilePart::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.saveBigFilePart");
    s.store_field("file_id", file_id_);
    s.store_field("file_part", file_part_);
    s.store_field("file_total_parts", file_total_parts_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

upload_saveBigFilePart::ReturnType upload_saveBigFilePart::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

users_getRequirementsToContact::users_getRequirementsToContact(array<object_ptr<InputUser>> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t users_getRequirementsToContact::ID;

void users_getRequirementsToContact::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-660962397);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void users_getRequirementsToContact::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-660962397);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void users_getRequirementsToContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.getRequirementsToContact");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

users_getRequirementsToContact::ReturnType users_getRequirementsToContact::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<RequirementToContact>>, 481674261>::parse(p);
#undef FAIL
}

users_setSecureValueErrors::users_setSecureValueErrors(object_ptr<InputUser> &&id_, array<object_ptr<SecureValueError>> &&errors_)
  : id_(std::move(id_))
  , errors_(std::move(errors_))
{}

const std::int32_t users_setSecureValueErrors::ID;

void users_setSecureValueErrors::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1865902923);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(errors_, s);
}

void users_setSecureValueErrors::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1865902923);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(errors_, s);
}

void users_setSecureValueErrors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.setSecureValueErrors");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    { s.store_vector_begin("errors", errors_.size()); for (const auto &_value : errors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

users_setSecureValueErrors::ReturnType users_setSecureValueErrors::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
