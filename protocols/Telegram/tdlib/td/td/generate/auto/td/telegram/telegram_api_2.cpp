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


autoSaveSettings::autoSaveSettings()
  : flags_()
  , photos_()
  , videos_()
  , video_max_size_()
{}

autoSaveSettings::autoSaveSettings(int32 flags_, bool photos_, bool videos_, int64 video_max_size_)
  : flags_(flags_)
  , photos_(photos_)
  , videos_(videos_)
  , video_max_size_(video_max_size_)
{}

const std::int32_t autoSaveSettings::ID;

object_ptr<autoSaveSettings> autoSaveSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<autoSaveSettings> res = make_tl_object<autoSaveSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->photos_ = (var0 & 1) != 0;
  res->videos_ = (var0 & 2) != 0;
  if (var0 & 4) { res->video_max_size_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void autoSaveSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (photos_ << 0) | (videos_ << 1)), s);
  if (var0 & 4) { TlStoreBinary::store(video_max_size_, s); }
}

void autoSaveSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (photos_ << 0) | (videos_ << 1)), s);
  if (var0 & 4) { TlStoreBinary::store(video_max_size_, s); }
}

void autoSaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autoSaveSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (photos_ << 0) | (videos_ << 1)));
    if (var0 & 1) { s.store_field("photos", true); }
    if (var0 & 2) { s.store_field("videos", true); }
    if (var0 & 4) { s.store_field("video_max_size", video_max_size_); }
    s.store_class_end();
  }
}

const std::int32_t cdnConfig::ID;

object_ptr<cdnConfig> cdnConfig::fetch(TlBufferParser &p) {
  return make_tl_object<cdnConfig>(p);
}

cdnConfig::cdnConfig(TlBufferParser &p)
  : public_keys_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<cdnPublicKey>, -914167110>>, 481674261>::parse(p))
{}

void cdnConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "cdnConfig");
    { s.store_vector_begin("public_keys", public_keys_.size()); for (const auto &_value : public_keys_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<ChatFull> ChatFull::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatFull::ID:
      return chatFull::fetch(p);
    case channelFull::ID:
      return channelFull::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

chatFull::chatFull()
  : flags_()
  , can_set_username_()
  , has_scheduled_()
  , translations_disabled_()
  , id_()
  , about_()
  , participants_()
  , chat_photo_()
  , notify_settings_()
  , exported_invite_()
  , bot_info_()
  , pinned_msg_id_()
  , folder_id_()
  , call_()
  , ttl_period_()
  , groupcall_default_join_as_()
  , theme_emoticon_()
  , requests_pending_()
  , recent_requesters_()
  , available_reactions_()
  , reactions_limit_()
{}

const std::int32_t chatFull::ID;

object_ptr<ChatFull> chatFull::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatFull> res = make_tl_object<chatFull>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->can_set_username_ = (var0 & 128) != 0;
  res->has_scheduled_ = (var0 & 256) != 0;
  res->translations_disabled_ = (var0 & 524288) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->about_ = TlFetchString<string>::parse(p);
  res->participants_ = TlFetchObject<ChatParticipants>::parse(p);
  if (var0 & 4) { res->chat_photo_ = TlFetchObject<Photo>::parse(p); }
  res->notify_settings_ = TlFetchBoxed<TlFetchObject<peerNotifySettings>, -1721619444>::parse(p);
  if (var0 & 8192) { res->exported_invite_ = TlFetchObject<ExportedChatInvite>::parse(p); }
  if (var0 & 8) { res->bot_info_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<botInfo>, 1300890265>>, 481674261>::parse(p); }
  if (var0 & 64) { res->pinned_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 2048) { res->folder_id_ = TlFetchInt::parse(p); }
  if (var0 & 4096) { res->call_ = TlFetchObject<InputGroupCall>::parse(p); }
  if (var0 & 16384) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (var0 & 32768) { res->groupcall_default_join_as_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 65536) { res->theme_emoticon_ = TlFetchString<string>::parse(p); }
  if (var0 & 131072) { res->requests_pending_ = TlFetchInt::parse(p); }
  if (var0 & 131072) { res->recent_requesters_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p); }
  if (var0 & 262144) { res->available_reactions_ = TlFetchObject<ChatReactions>::parse(p); }
  if (var0 & 1048576) { res->reactions_limit_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chatFull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFull");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (can_set_username_ << 7) | (has_scheduled_ << 8) | (translations_disabled_ << 19)));
    if (var0 & 128) { s.store_field("can_set_username", true); }
    if (var0 & 256) { s.store_field("has_scheduled", true); }
    if (var0 & 524288) { s.store_field("translations_disabled", true); }
    s.store_field("id", id_);
    s.store_field("about", about_);
    s.store_object_field("participants", static_cast<const BaseObject *>(participants_.get()));
    if (var0 & 4) { s.store_object_field("chat_photo", static_cast<const BaseObject *>(chat_photo_.get())); }
    s.store_object_field("notify_settings", static_cast<const BaseObject *>(notify_settings_.get()));
    if (var0 & 8192) { s.store_object_field("exported_invite", static_cast<const BaseObject *>(exported_invite_.get())); }
    if (var0 & 8) { { s.store_vector_begin("bot_info", bot_info_.size()); for (const auto &_value : bot_info_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 64) { s.store_field("pinned_msg_id", pinned_msg_id_); }
    if (var0 & 2048) { s.store_field("folder_id", folder_id_); }
    if (var0 & 4096) { s.store_object_field("call", static_cast<const BaseObject *>(call_.get())); }
    if (var0 & 16384) { s.store_field("ttl_period", ttl_period_); }
    if (var0 & 32768) { s.store_object_field("groupcall_default_join_as", static_cast<const BaseObject *>(groupcall_default_join_as_.get())); }
    if (var0 & 65536) { s.store_field("theme_emoticon", theme_emoticon_); }
    if (var0 & 131072) { s.store_field("requests_pending", requests_pending_); }
    if (var0 & 131072) { { s.store_vector_begin("recent_requesters", recent_requesters_.size()); for (const auto &_value : recent_requesters_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 262144) { s.store_object_field("available_reactions", static_cast<const BaseObject *>(available_reactions_.get())); }
    if (var0 & 1048576) { s.store_field("reactions_limit", reactions_limit_); }
    s.store_class_end();
  }
}

channelFull::channelFull()
  : flags_()
  , can_view_participants_()
  , can_set_username_()
  , can_set_stickers_()
  , hidden_prehistory_()
  , can_set_location_()
  , has_scheduled_()
  , can_view_stats_()
  , blocked_()
  , flags2_()
  , can_delete_channel_()
  , antispam_()
  , participants_hidden_()
  , translations_disabled_()
  , stories_pinned_available_()
  , view_forum_as_messages_()
  , restricted_sponsored_()
  , can_view_revenue_()
  , paid_media_allowed_()
  , can_view_stars_revenue_()
  , paid_reactions_available_()
  , stargifts_available_()
  , paid_messages_available_()
  , id_()
  , about_()
  , participants_count_()
  , admins_count_()
  , kicked_count_()
  , banned_count_()
  , online_count_()
  , read_inbox_max_id_()
  , read_outbox_max_id_()
  , unread_count_()
  , chat_photo_()
  , notify_settings_()
  , exported_invite_()
  , bot_info_()
  , migrated_from_chat_id_()
  , migrated_from_max_id_()
  , pinned_msg_id_()
  , stickerset_()
  , available_min_id_()
  , folder_id_()
  , linked_chat_id_()
  , location_()
  , slowmode_seconds_()
  , slowmode_next_send_date_()
  , stats_dc_()
  , pts_()
  , call_()
  , ttl_period_()
  , pending_suggestions_()
  , groupcall_default_join_as_()
  , theme_emoticon_()
  , requests_pending_()
  , recent_requesters_()
  , default_send_as_()
  , available_reactions_()
  , reactions_limit_()
  , stories_()
  , wallpaper_()
  , boosts_applied_()
  , boosts_unrestrict_()
  , emojiset_()
  , bot_verification_()
  , stargifts_count_()
  , send_paid_messages_stars_()
  , main_tab_()
{}

const std::int32_t channelFull::ID;

object_ptr<ChatFull> channelFull::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelFull> res = make_tl_object<channelFull>();
  int32 var0;
  int32 var1;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->can_view_participants_ = (var0 & 8) != 0;
  res->can_set_username_ = (var0 & 64) != 0;
  res->can_set_stickers_ = (var0 & 128) != 0;
  res->hidden_prehistory_ = (var0 & 1024) != 0;
  res->can_set_location_ = (var0 & 65536) != 0;
  res->has_scheduled_ = (var0 & 524288) != 0;
  res->can_view_stats_ = (var0 & 1048576) != 0;
  res->blocked_ = (var0 & 4194304) != 0;
  if ((var1 = res->flags2_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->can_delete_channel_ = (var1 & 1) != 0;
  res->antispam_ = (var1 & 2) != 0;
  res->participants_hidden_ = (var1 & 4) != 0;
  res->translations_disabled_ = (var1 & 8) != 0;
  res->stories_pinned_available_ = (var1 & 32) != 0;
  res->view_forum_as_messages_ = (var1 & 64) != 0;
  res->restricted_sponsored_ = (var1 & 2048) != 0;
  res->can_view_revenue_ = (var1 & 4096) != 0;
  res->paid_media_allowed_ = (var1 & 16384) != 0;
  res->can_view_stars_revenue_ = (var1 & 32768) != 0;
  res->paid_reactions_available_ = (var1 & 65536) != 0;
  res->stargifts_available_ = (var1 & 524288) != 0;
  res->paid_messages_available_ = (var1 & 1048576) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->about_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->participants_count_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->admins_count_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->kicked_count_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->banned_count_ = TlFetchInt::parse(p); }
  if (var0 & 8192) { res->online_count_ = TlFetchInt::parse(p); }
  res->read_inbox_max_id_ = TlFetchInt::parse(p);
  res->read_outbox_max_id_ = TlFetchInt::parse(p);
  res->unread_count_ = TlFetchInt::parse(p);
  res->chat_photo_ = TlFetchObject<Photo>::parse(p);
  res->notify_settings_ = TlFetchBoxed<TlFetchObject<peerNotifySettings>, -1721619444>::parse(p);
  if (var0 & 8388608) { res->exported_invite_ = TlFetchObject<ExportedChatInvite>::parse(p); }
  res->bot_info_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<botInfo>, 1300890265>>, 481674261>::parse(p);
  if (var0 & 16) { res->migrated_from_chat_id_ = TlFetchLong::parse(p); }
  if (var0 & 16) { res->migrated_from_max_id_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->pinned_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->stickerset_ = TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>::parse(p); }
  if (var0 & 512) { res->available_min_id_ = TlFetchInt::parse(p); }
  if (var0 & 2048) { res->folder_id_ = TlFetchInt::parse(p); }
  if (var0 & 16384) { res->linked_chat_id_ = TlFetchLong::parse(p); }
  if (var0 & 32768) { res->location_ = TlFetchObject<ChannelLocation>::parse(p); }
  if (var0 & 131072) { res->slowmode_seconds_ = TlFetchInt::parse(p); }
  if (var0 & 262144) { res->slowmode_next_send_date_ = TlFetchInt::parse(p); }
  if (var0 & 4096) { res->stats_dc_ = TlFetchInt::parse(p); }
  res->pts_ = TlFetchInt::parse(p);
  if (var0 & 2097152) { res->call_ = TlFetchObject<InputGroupCall>::parse(p); }
  if (var0 & 16777216) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (var0 & 33554432) { res->pending_suggestions_ = TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p); }
  if (var0 & 67108864) { res->groupcall_default_join_as_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 134217728) { res->theme_emoticon_ = TlFetchString<string>::parse(p); }
  if (var0 & 268435456) { res->requests_pending_ = TlFetchInt::parse(p); }
  if (var0 & 268435456) { res->recent_requesters_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p); }
  if (var0 & 536870912) { res->default_send_as_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 1073741824) { res->available_reactions_ = TlFetchObject<ChatReactions>::parse(p); }
  if (var1 & 8192) { res->reactions_limit_ = TlFetchInt::parse(p); }
  if (var1 & 16) { res->stories_ = TlFetchBoxed<TlFetchObject<peerStories>, -1707742823>::parse(p); }
  if (var1 & 128) { res->wallpaper_ = TlFetchObject<WallPaper>::parse(p); }
  if (var1 & 256) { res->boosts_applied_ = TlFetchInt::parse(p); }
  if (var1 & 512) { res->boosts_unrestrict_ = TlFetchInt::parse(p); }
  if (var1 & 1024) { res->emojiset_ = TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>::parse(p); }
  if (var1 & 131072) { res->bot_verification_ = TlFetchBoxed<TlFetchObject<botVerification>, -113453988>::parse(p); }
  if (var1 & 262144) { res->stargifts_count_ = TlFetchInt::parse(p); }
  if (var1 & 2097152) { res->send_paid_messages_stars_ = TlFetchLong::parse(p); }
  if (var1 & 4194304) { res->main_tab_ = TlFetchObject<ProfileTab>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelFull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelFull");
  int32 var0;
  int32 var1;
    s.store_field("flags", (var0 = flags_ | (can_view_participants_ << 3) | (can_set_username_ << 6) | (can_set_stickers_ << 7) | (hidden_prehistory_ << 10) | (can_set_location_ << 16) | (has_scheduled_ << 19) | (can_view_stats_ << 20) | (blocked_ << 22)));
    if (var0 & 8) { s.store_field("can_view_participants", true); }
    if (var0 & 64) { s.store_field("can_set_username", true); }
    if (var0 & 128) { s.store_field("can_set_stickers", true); }
    if (var0 & 1024) { s.store_field("hidden_prehistory", true); }
    if (var0 & 65536) { s.store_field("can_set_location", true); }
    if (var0 & 524288) { s.store_field("has_scheduled", true); }
    if (var0 & 1048576) { s.store_field("can_view_stats", true); }
    if (var0 & 4194304) { s.store_field("blocked", true); }
    s.store_field("flags2", (var1 = flags2_ | (can_delete_channel_ << 0) | (antispam_ << 1) | (participants_hidden_ << 2) | (translations_disabled_ << 3) | (stories_pinned_available_ << 5) | (view_forum_as_messages_ << 6) | (restricted_sponsored_ << 11) | (can_view_revenue_ << 12) | (paid_media_allowed_ << 14) | (can_view_stars_revenue_ << 15) | (paid_reactions_available_ << 16) | (stargifts_available_ << 19) | (paid_messages_available_ << 20)));
    if (var1 & 1) { s.store_field("can_delete_channel", true); }
    if (var1 & 2) { s.store_field("antispam", true); }
    if (var1 & 4) { s.store_field("participants_hidden", true); }
    if (var1 & 8) { s.store_field("translations_disabled", true); }
    if (var1 & 32) { s.store_field("stories_pinned_available", true); }
    if (var1 & 64) { s.store_field("view_forum_as_messages", true); }
    if (var1 & 2048) { s.store_field("restricted_sponsored", true); }
    if (var1 & 4096) { s.store_field("can_view_revenue", true); }
    if (var1 & 16384) { s.store_field("paid_media_allowed", true); }
    if (var1 & 32768) { s.store_field("can_view_stars_revenue", true); }
    if (var1 & 65536) { s.store_field("paid_reactions_available", true); }
    if (var1 & 524288) { s.store_field("stargifts_available", true); }
    if (var1 & 1048576) { s.store_field("paid_messages_available", true); }
    s.store_field("id", id_);
    s.store_field("about", about_);
    if (var0 & 1) { s.store_field("participants_count", participants_count_); }
    if (var0 & 2) { s.store_field("admins_count", admins_count_); }
    if (var0 & 4) { s.store_field("kicked_count", kicked_count_); }
    if (var0 & 4) { s.store_field("banned_count", banned_count_); }
    if (var0 & 8192) { s.store_field("online_count", online_count_); }
    s.store_field("read_inbox_max_id", read_inbox_max_id_);
    s.store_field("read_outbox_max_id", read_outbox_max_id_);
    s.store_field("unread_count", unread_count_);
    s.store_object_field("chat_photo", static_cast<const BaseObject *>(chat_photo_.get()));
    s.store_object_field("notify_settings", static_cast<const BaseObject *>(notify_settings_.get()));
    if (var0 & 8388608) { s.store_object_field("exported_invite", static_cast<const BaseObject *>(exported_invite_.get())); }
    { s.store_vector_begin("bot_info", bot_info_.size()); for (const auto &_value : bot_info_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 16) { s.store_field("migrated_from_chat_id", migrated_from_chat_id_); }
    if (var0 & 16) { s.store_field("migrated_from_max_id", migrated_from_max_id_); }
    if (var0 & 32) { s.store_field("pinned_msg_id", pinned_msg_id_); }
    if (var0 & 256) { s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get())); }
    if (var0 & 512) { s.store_field("available_min_id", available_min_id_); }
    if (var0 & 2048) { s.store_field("folder_id", folder_id_); }
    if (var0 & 16384) { s.store_field("linked_chat_id", linked_chat_id_); }
    if (var0 & 32768) { s.store_object_field("location", static_cast<const BaseObject *>(location_.get())); }
    if (var0 & 131072) { s.store_field("slowmode_seconds", slowmode_seconds_); }
    if (var0 & 262144) { s.store_field("slowmode_next_send_date", slowmode_next_send_date_); }
    if (var0 & 4096) { s.store_field("stats_dc", stats_dc_); }
    s.store_field("pts", pts_);
    if (var0 & 2097152) { s.store_object_field("call", static_cast<const BaseObject *>(call_.get())); }
    if (var0 & 16777216) { s.store_field("ttl_period", ttl_period_); }
    if (var0 & 33554432) { { s.store_vector_begin("pending_suggestions", pending_suggestions_.size()); for (const auto &_value : pending_suggestions_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 67108864) { s.store_object_field("groupcall_default_join_as", static_cast<const BaseObject *>(groupcall_default_join_as_.get())); }
    if (var0 & 134217728) { s.store_field("theme_emoticon", theme_emoticon_); }
    if (var0 & 268435456) { s.store_field("requests_pending", requests_pending_); }
    if (var0 & 268435456) { { s.store_vector_begin("recent_requesters", recent_requesters_.size()); for (const auto &_value : recent_requesters_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 536870912) { s.store_object_field("default_send_as", static_cast<const BaseObject *>(default_send_as_.get())); }
    if (var0 & 1073741824) { s.store_object_field("available_reactions", static_cast<const BaseObject *>(available_reactions_.get())); }
    if (var1 & 8192) { s.store_field("reactions_limit", reactions_limit_); }
    if (var1 & 16) { s.store_object_field("stories", static_cast<const BaseObject *>(stories_.get())); }
    if (var1 & 128) { s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get())); }
    if (var1 & 256) { s.store_field("boosts_applied", boosts_applied_); }
    if (var1 & 512) { s.store_field("boosts_unrestrict", boosts_unrestrict_); }
    if (var1 & 1024) { s.store_object_field("emojiset", static_cast<const BaseObject *>(emojiset_.get())); }
    if (var1 & 131072) { s.store_object_field("bot_verification", static_cast<const BaseObject *>(bot_verification_.get())); }
    if (var1 & 262144) { s.store_field("stargifts_count", stargifts_count_); }
    if (var1 & 2097152) { s.store_field("send_paid_messages_stars", send_paid_messages_stars_); }
    if (var1 & 4194304) { s.store_object_field("main_tab", static_cast<const BaseObject *>(main_tab_.get())); }
    s.store_class_end();
  }
}

object_ptr<ChatInvite> ChatInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatInviteAlready::ID:
      return chatInviteAlready::fetch(p);
    case chatInvite::ID:
      return chatInvite::fetch(p);
    case chatInvitePeek::ID:
      return chatInvitePeek::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t chatInviteAlready::ID;

object_ptr<ChatInvite> chatInviteAlready::fetch(TlBufferParser &p) {
  return make_tl_object<chatInviteAlready>(p);
}

chatInviteAlready::chatInviteAlready(TlBufferParser &p)
  : chat_(TlFetchObject<Chat>::parse(p))
{}

void chatInviteAlready::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteAlready");
    s.store_object_field("chat", static_cast<const BaseObject *>(chat_.get()));
    s.store_class_end();
  }
}

chatInvite::chatInvite()
  : flags_()
  , channel_()
  , broadcast_()
  , public_()
  , megagroup_()
  , request_needed_()
  , verified_()
  , scam_()
  , fake_()
  , can_refulfill_subscription_()
  , title_()
  , about_()
  , photo_()
  , participants_count_()
  , participants_()
  , color_()
  , subscription_pricing_()
  , subscription_form_id_()
  , bot_verification_()
{}

const std::int32_t chatInvite::ID;

object_ptr<ChatInvite> chatInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatInvite> res = make_tl_object<chatInvite>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->channel_ = (var0 & 1) != 0;
  res->broadcast_ = (var0 & 2) != 0;
  res->public_ = (var0 & 4) != 0;
  res->megagroup_ = (var0 & 8) != 0;
  res->request_needed_ = (var0 & 64) != 0;
  res->verified_ = (var0 & 128) != 0;
  res->scam_ = (var0 & 256) != 0;
  res->fake_ = (var0 & 512) != 0;
  res->can_refulfill_subscription_ = (var0 & 2048) != 0;
  res->title_ = TlFetchString<string>::parse(p);
  if (var0 & 32) { res->about_ = TlFetchString<string>::parse(p); }
  res->photo_ = TlFetchObject<Photo>::parse(p);
  res->participants_count_ = TlFetchInt::parse(p);
  if (var0 & 16) { res->participants_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p); }
  res->color_ = TlFetchInt::parse(p);
  if (var0 & 1024) { res->subscription_pricing_ = TlFetchBoxed<TlFetchObject<starsSubscriptionPricing>, 88173912>::parse(p); }
  if (var0 & 4096) { res->subscription_form_id_ = TlFetchLong::parse(p); }
  if (var0 & 8192) { res->bot_verification_ = TlFetchBoxed<TlFetchObject<botVerification>, -113453988>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInvite");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (channel_ << 0) | (broadcast_ << 1) | (public_ << 2) | (megagroup_ << 3) | (request_needed_ << 6) | (verified_ << 7) | (scam_ << 8) | (fake_ << 9) | (can_refulfill_subscription_ << 11)));
    if (var0 & 1) { s.store_field("channel", true); }
    if (var0 & 2) { s.store_field("broadcast", true); }
    if (var0 & 4) { s.store_field("public", true); }
    if (var0 & 8) { s.store_field("megagroup", true); }
    if (var0 & 64) { s.store_field("request_needed", true); }
    if (var0 & 128) { s.store_field("verified", true); }
    if (var0 & 256) { s.store_field("scam", true); }
    if (var0 & 512) { s.store_field("fake", true); }
    if (var0 & 2048) { s.store_field("can_refulfill_subscription", true); }
    s.store_field("title", title_);
    if (var0 & 32) { s.store_field("about", about_); }
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("participants_count", participants_count_);
    if (var0 & 16) { { s.store_vector_begin("participants", participants_.size()); for (const auto &_value : participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("color", color_);
    if (var0 & 1024) { s.store_object_field("subscription_pricing", static_cast<const BaseObject *>(subscription_pricing_.get())); }
    if (var0 & 4096) { s.store_field("subscription_form_id", subscription_form_id_); }
    if (var0 & 8192) { s.store_object_field("bot_verification", static_cast<const BaseObject *>(bot_verification_.get())); }
    s.store_class_end();
  }
}

const std::int32_t chatInvitePeek::ID;

object_ptr<ChatInvite> chatInvitePeek::fetch(TlBufferParser &p) {
  return make_tl_object<chatInvitePeek>(p);
}

chatInvitePeek::chatInvitePeek(TlBufferParser &p)
  : chat_(TlFetchObject<Chat>::parse(p))
  , expires_(TlFetchInt::parse(p))
{}

void chatInvitePeek::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInvitePeek");
    s.store_object_field("chat", static_cast<const BaseObject *>(chat_.get()));
    s.store_field("expires", expires_);
    s.store_class_end();
  }
}

const std::int32_t contactBirthday::ID;

object_ptr<contactBirthday> contactBirthday::fetch(TlBufferParser &p) {
  return make_tl_object<contactBirthday>(p);
}

contactBirthday::contactBirthday(TlBufferParser &p)
  : contact_id_(TlFetchLong::parse(p))
  , birthday_(TlFetchBoxed<TlFetchObject<birthday>, 1821253126>::parse(p))
{}

void contactBirthday::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contactBirthday");
    s.store_field("contact_id", contact_id_);
    s.store_object_field("birthday", static_cast<const BaseObject *>(birthday_.get()));
    s.store_class_end();
  }
}

object_ptr<DocumentAttribute> DocumentAttribute::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case documentAttributeImageSize::ID:
      return documentAttributeImageSize::fetch(p);
    case documentAttributeAnimated::ID:
      return documentAttributeAnimated::fetch(p);
    case documentAttributeSticker::ID:
      return documentAttributeSticker::fetch(p);
    case documentAttributeVideo::ID:
      return documentAttributeVideo::fetch(p);
    case documentAttributeAudio::ID:
      return documentAttributeAudio::fetch(p);
    case documentAttributeFilename::ID:
      return documentAttributeFilename::fetch(p);
    case documentAttributeHasStickers::ID:
      return documentAttributeHasStickers::fetch(p);
    case documentAttributeCustomEmoji::ID:
      return documentAttributeCustomEmoji::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

documentAttributeImageSize::documentAttributeImageSize(int32 w_, int32 h_)
  : w_(w_)
  , h_(h_)
{}

const std::int32_t documentAttributeImageSize::ID;

object_ptr<DocumentAttribute> documentAttributeImageSize::fetch(TlBufferParser &p) {
  return make_tl_object<documentAttributeImageSize>(p);
}

documentAttributeImageSize::documentAttributeImageSize(TlBufferParser &p)
  : w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
{}

void documentAttributeImageSize::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
}

void documentAttributeImageSize::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
}

void documentAttributeImageSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeImageSize");
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_class_end();
  }
}

const std::int32_t documentAttributeAnimated::ID;

object_ptr<DocumentAttribute> documentAttributeAnimated::fetch(TlBufferParser &p) {
  return make_tl_object<documentAttributeAnimated>();
}

void documentAttributeAnimated::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void documentAttributeAnimated::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void documentAttributeAnimated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeAnimated");
    s.store_class_end();
  }
}

documentAttributeSticker::documentAttributeSticker()
  : flags_()
  , mask_()
  , alt_()
  , stickerset_()
  , mask_coords_()
{}

documentAttributeSticker::documentAttributeSticker(int32 flags_, bool mask_, string const &alt_, object_ptr<InputStickerSet> &&stickerset_, object_ptr<maskCoords> &&mask_coords_)
  : flags_(flags_)
  , mask_(mask_)
  , alt_(alt_)
  , stickerset_(std::move(stickerset_))
  , mask_coords_(std::move(mask_coords_))
{}

const std::int32_t documentAttributeSticker::ID;

object_ptr<DocumentAttribute> documentAttributeSticker::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<documentAttributeSticker> res = make_tl_object<documentAttributeSticker>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->mask_ = (var0 & 2) != 0;
  res->alt_ = TlFetchString<string>::parse(p);
  res->stickerset_ = TlFetchObject<InputStickerSet>::parse(p);
  if (var0 & 1) { res->mask_coords_ = TlFetchBoxed<TlFetchObject<maskCoords>, -1361650766>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void documentAttributeSticker::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (mask_ << 1)), s);
  TlStoreString::store(alt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1361650766>::store(mask_coords_, s); }
}

void documentAttributeSticker::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (mask_ << 1)), s);
  TlStoreString::store(alt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1361650766>::store(mask_coords_, s); }
}

void documentAttributeSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeSticker");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (mask_ << 1)));
    if (var0 & 2) { s.store_field("mask", true); }
    s.store_field("alt", alt_);
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    if (var0 & 1) { s.store_object_field("mask_coords", static_cast<const BaseObject *>(mask_coords_.get())); }
    s.store_class_end();
  }
}

documentAttributeVideo::documentAttributeVideo()
  : flags_()
  , round_message_()
  , supports_streaming_()
  , nosound_()
  , duration_()
  , w_()
  , h_()
  , preload_prefix_size_()
  , video_start_ts_()
  , video_codec_()
{}

documentAttributeVideo::documentAttributeVideo(int32 flags_, bool round_message_, bool supports_streaming_, bool nosound_, double duration_, int32 w_, int32 h_, int32 preload_prefix_size_, double video_start_ts_, string const &video_codec_)
  : flags_(flags_)
  , round_message_(round_message_)
  , supports_streaming_(supports_streaming_)
  , nosound_(nosound_)
  , duration_(duration_)
  , w_(w_)
  , h_(h_)
  , preload_prefix_size_(preload_prefix_size_)
  , video_start_ts_(video_start_ts_)
  , video_codec_(video_codec_)
{}

const std::int32_t documentAttributeVideo::ID;

object_ptr<DocumentAttribute> documentAttributeVideo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<documentAttributeVideo> res = make_tl_object<documentAttributeVideo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->round_message_ = (var0 & 1) != 0;
  res->supports_streaming_ = (var0 & 2) != 0;
  res->nosound_ = (var0 & 8) != 0;
  res->duration_ = TlFetchDouble::parse(p);
  res->w_ = TlFetchInt::parse(p);
  res->h_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->preload_prefix_size_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->video_start_ts_ = TlFetchDouble::parse(p); }
  if (var0 & 32) { res->video_codec_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void documentAttributeVideo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (round_message_ << 0) | (supports_streaming_ << 1) | (nosound_ << 3)), s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  if (var0 & 4) { TlStoreBinary::store(preload_prefix_size_, s); }
  if (var0 & 16) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 32) { TlStoreString::store(video_codec_, s); }
}

void documentAttributeVideo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (round_message_ << 0) | (supports_streaming_ << 1) | (nosound_ << 3)), s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  if (var0 & 4) { TlStoreBinary::store(preload_prefix_size_, s); }
  if (var0 & 16) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 32) { TlStoreString::store(video_codec_, s); }
}

void documentAttributeVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeVideo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (round_message_ << 0) | (supports_streaming_ << 1) | (nosound_ << 3)));
    if (var0 & 1) { s.store_field("round_message", true); }
    if (var0 & 2) { s.store_field("supports_streaming", true); }
    if (var0 & 8) { s.store_field("nosound", true); }
    s.store_field("duration", duration_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    if (var0 & 4) { s.store_field("preload_prefix_size", preload_prefix_size_); }
    if (var0 & 16) { s.store_field("video_start_ts", video_start_ts_); }
    if (var0 & 32) { s.store_field("video_codec", video_codec_); }
    s.store_class_end();
  }
}

documentAttributeAudio::documentAttributeAudio()
  : flags_()
  , voice_()
  , duration_()
  , title_()
  , performer_()
  , waveform_()
{}

documentAttributeAudio::documentAttributeAudio(int32 flags_, bool voice_, int32 duration_, string const &title_, string const &performer_, bytes &&waveform_)
  : flags_(flags_)
  , voice_(voice_)
  , duration_(duration_)
  , title_(title_)
  , performer_(performer_)
  , waveform_(std::move(waveform_))
{}

const std::int32_t documentAttributeAudio::ID;

object_ptr<DocumentAttribute> documentAttributeAudio::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<documentAttributeAudio> res = make_tl_object<documentAttributeAudio>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->voice_ = (var0 & 1024) != 0;
  res->duration_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->performer_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->waveform_ = TlFetchBytes<bytes>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void documentAttributeAudio::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (voice_ << 10)), s);
  TlStoreBinary::store(duration_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreString::store(performer_, s); }
  if (var0 & 4) { TlStoreString::store(waveform_, s); }
}

void documentAttributeAudio::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (voice_ << 10)), s);
  TlStoreBinary::store(duration_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreString::store(performer_, s); }
  if (var0 & 4) { TlStoreString::store(waveform_, s); }
}

void documentAttributeAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeAudio");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (voice_ << 10)));
    if (var0 & 1024) { s.store_field("voice", true); }
    s.store_field("duration", duration_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("performer", performer_); }
    if (var0 & 4) { s.store_bytes_field("waveform", waveform_); }
    s.store_class_end();
  }
}

documentAttributeFilename::documentAttributeFilename(string const &file_name_)
  : file_name_(file_name_)
{}

const std::int32_t documentAttributeFilename::ID;

object_ptr<DocumentAttribute> documentAttributeFilename::fetch(TlBufferParser &p) {
  return make_tl_object<documentAttributeFilename>(p);
}

documentAttributeFilename::documentAttributeFilename(TlBufferParser &p)
  : file_name_(TlFetchString<string>::parse(p))
{}

void documentAttributeFilename::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(file_name_, s);
}

void documentAttributeFilename::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(file_name_, s);
}

void documentAttributeFilename::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeFilename");
    s.store_field("file_name", file_name_);
    s.store_class_end();
  }
}

const std::int32_t documentAttributeHasStickers::ID;

object_ptr<DocumentAttribute> documentAttributeHasStickers::fetch(TlBufferParser &p) {
  return make_tl_object<documentAttributeHasStickers>();
}

void documentAttributeHasStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void documentAttributeHasStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void documentAttributeHasStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeHasStickers");
    s.store_class_end();
  }
}

documentAttributeCustomEmoji::documentAttributeCustomEmoji()
  : flags_()
  , free_()
  , text_color_()
  , alt_()
  , stickerset_()
{}

documentAttributeCustomEmoji::documentAttributeCustomEmoji(int32 flags_, bool free_, bool text_color_, string const &alt_, object_ptr<InputStickerSet> &&stickerset_)
  : flags_(flags_)
  , free_(free_)
  , text_color_(text_color_)
  , alt_(alt_)
  , stickerset_(std::move(stickerset_))
{}

const std::int32_t documentAttributeCustomEmoji::ID;

object_ptr<DocumentAttribute> documentAttributeCustomEmoji::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<documentAttributeCustomEmoji> res = make_tl_object<documentAttributeCustomEmoji>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->free_ = (var0 & 1) != 0;
  res->text_color_ = (var0 & 2) != 0;
  res->alt_ = TlFetchString<string>::parse(p);
  res->stickerset_ = TlFetchObject<InputStickerSet>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void documentAttributeCustomEmoji::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (free_ << 0) | (text_color_ << 1)), s);
  TlStoreString::store(alt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void documentAttributeCustomEmoji::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (free_ << 0) | (text_color_ << 1)), s);
  TlStoreString::store(alt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void documentAttributeCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeCustomEmoji");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (free_ << 0) | (text_color_ << 1)));
    if (var0 & 1) { s.store_field("free", true); }
    if (var0 & 2) { s.store_field("text_color", true); }
    s.store_field("alt", alt_);
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_class_end();
  }
}

object_ptr<GroupCall> GroupCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case groupCallDiscarded::ID:
      return groupCallDiscarded::fetch(p);
    case groupCall::ID:
      return groupCall::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t groupCallDiscarded::ID;

object_ptr<GroupCall> groupCallDiscarded::fetch(TlBufferParser &p) {
  return make_tl_object<groupCallDiscarded>(p);
}

groupCallDiscarded::groupCallDiscarded(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , duration_(TlFetchInt::parse(p))
{}

void groupCallDiscarded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallDiscarded");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

groupCall::groupCall()
  : flags_()
  , join_muted_()
  , can_change_join_muted_()
  , join_date_asc_()
  , schedule_start_subscribed_()
  , can_start_video_()
  , record_video_active_()
  , rtmp_stream_()
  , listeners_hidden_()
  , conference_()
  , creator_()
  , messages_enabled_()
  , can_change_messages_enabled_()
  , min_()
  , id_()
  , access_hash_()
  , participants_count_()
  , title_()
  , stream_dc_id_()
  , record_start_date_()
  , schedule_date_()
  , unmuted_video_count_()
  , unmuted_video_limit_()
  , version_()
  , invite_link_()
{}

const std::int32_t groupCall::ID;

object_ptr<GroupCall> groupCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<groupCall> res = make_tl_object<groupCall>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->join_muted_ = (var0 & 2) != 0;
  res->can_change_join_muted_ = (var0 & 4) != 0;
  res->join_date_asc_ = (var0 & 64) != 0;
  res->schedule_start_subscribed_ = (var0 & 256) != 0;
  res->can_start_video_ = (var0 & 512) != 0;
  res->record_video_active_ = (var0 & 2048) != 0;
  res->rtmp_stream_ = (var0 & 4096) != 0;
  res->listeners_hidden_ = (var0 & 8192) != 0;
  res->conference_ = (var0 & 16384) != 0;
  res->creator_ = (var0 & 32768) != 0;
  res->messages_enabled_ = (var0 & 131072) != 0;
  res->can_change_messages_enabled_ = (var0 & 262144) != 0;
  res->min_ = (var0 & 524288) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->participants_count_ = TlFetchInt::parse(p);
  if (var0 & 8) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->stream_dc_id_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->record_start_date_ = TlFetchInt::parse(p); }
  if (var0 & 128) { res->schedule_date_ = TlFetchInt::parse(p); }
  if (var0 & 1024) { res->unmuted_video_count_ = TlFetchInt::parse(p); }
  res->unmuted_video_limit_ = TlFetchInt::parse(p);
  res->version_ = TlFetchInt::parse(p);
  if (var0 & 65536) { res->invite_link_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void groupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCall");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (join_muted_ << 1) | (can_change_join_muted_ << 2) | (join_date_asc_ << 6) | (schedule_start_subscribed_ << 8) | (can_start_video_ << 9) | (record_video_active_ << 11) | (rtmp_stream_ << 12) | (listeners_hidden_ << 13) | (conference_ << 14) | (creator_ << 15) | (messages_enabled_ << 17) | (can_change_messages_enabled_ << 18) | (min_ << 19)));
    if (var0 & 2) { s.store_field("join_muted", true); }
    if (var0 & 4) { s.store_field("can_change_join_muted", true); }
    if (var0 & 64) { s.store_field("join_date_asc", true); }
    if (var0 & 256) { s.store_field("schedule_start_subscribed", true); }
    if (var0 & 512) { s.store_field("can_start_video", true); }
    if (var0 & 2048) { s.store_field("record_video_active", true); }
    if (var0 & 4096) { s.store_field("rtmp_stream", true); }
    if (var0 & 8192) { s.store_field("listeners_hidden", true); }
    if (var0 & 16384) { s.store_field("conference", true); }
    if (var0 & 32768) { s.store_field("creator", true); }
    if (var0 & 131072) { s.store_field("messages_enabled", true); }
    if (var0 & 262144) { s.store_field("can_change_messages_enabled", true); }
    if (var0 & 524288) { s.store_field("min", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("participants_count", participants_count_);
    if (var0 & 8) { s.store_field("title", title_); }
    if (var0 & 16) { s.store_field("stream_dc_id", stream_dc_id_); }
    if (var0 & 32) { s.store_field("record_start_date", record_start_date_); }
    if (var0 & 128) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 1024) { s.store_field("unmuted_video_count", unmuted_video_count_); }
    s.store_field("unmuted_video_limit", unmuted_video_limit_);
    s.store_field("version", version_);
    if (var0 & 65536) { s.store_field("invite_link", invite_link_); }
    s.store_class_end();
  }
}

groupCallParticipantVideo::groupCallParticipantVideo()
  : flags_()
  , paused_()
  , endpoint_()
  , source_groups_()
  , audio_source_()
{}

const std::int32_t groupCallParticipantVideo::ID;

object_ptr<groupCallParticipantVideo> groupCallParticipantVideo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<groupCallParticipantVideo> res = make_tl_object<groupCallParticipantVideo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->paused_ = (var0 & 1) != 0;
  res->endpoint_ = TlFetchString<string>::parse(p);
  res->source_groups_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<groupCallParticipantVideoSourceGroup>, -592373577>>, 481674261>::parse(p);
  if (var0 & 2) { res->audio_source_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void groupCallParticipantVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallParticipantVideo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (paused_ << 0)));
    if (var0 & 1) { s.store_field("paused", true); }
    s.store_field("endpoint", endpoint_);
    { s.store_vector_begin("source_groups", source_groups_.size()); for (const auto &_value : source_groups_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 2) { s.store_field("audio_source", audio_source_); }
    s.store_class_end();
  }
}

const std::int32_t groupCallParticipantVideoSourceGroup::ID;

object_ptr<groupCallParticipantVideoSourceGroup> groupCallParticipantVideoSourceGroup::fetch(TlBufferParser &p) {
  return make_tl_object<groupCallParticipantVideoSourceGroup>(p);
}

groupCallParticipantVideoSourceGroup::groupCallParticipantVideoSourceGroup(TlBufferParser &p)
  : semantics_(TlFetchString<string>::parse(p))
  , sources_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void groupCallParticipantVideoSourceGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallParticipantVideoSourceGroup");
    s.store_field("semantics", semantics_);
    { s.store_vector_begin("sources", sources_.size()); for (const auto &_value : sources_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<InputGeoPoint> InputGeoPoint::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputGeoPointEmpty::ID:
      return inputGeoPointEmpty::fetch(p);
    case inputGeoPoint::ID:
      return inputGeoPoint::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputGeoPointEmpty::ID;

object_ptr<InputGeoPoint> inputGeoPointEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputGeoPointEmpty>();
}

void inputGeoPointEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputGeoPointEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputGeoPointEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGeoPointEmpty");
    s.store_class_end();
  }
}

inputGeoPoint::inputGeoPoint()
  : flags_()
  , lat_()
  , long_()
  , accuracy_radius_()
{}

inputGeoPoint::inputGeoPoint(int32 flags_, double lat_, double long_, int32 accuracy_radius_)
  : flags_(flags_)
  , lat_(lat_)
  , long_(long_)
  , accuracy_radius_(accuracy_radius_)
{}

const std::int32_t inputGeoPoint::ID;

object_ptr<InputGeoPoint> inputGeoPoint::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputGeoPoint> res = make_tl_object<inputGeoPoint>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->lat_ = TlFetchDouble::parse(p);
  res->long_ = TlFetchDouble::parse(p);
  if (var0 & 1) { res->accuracy_radius_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputGeoPoint::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(long_, s);
  if (var0 & 1) { TlStoreBinary::store(accuracy_radius_, s); }
}

void inputGeoPoint::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(long_, s);
  if (var0 & 1) { TlStoreBinary::store(accuracy_radius_, s); }
}

void inputGeoPoint::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGeoPoint");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("lat", lat_);
    s.store_field("long", long_);
    if (var0 & 1) { s.store_field("accuracy_radius", accuracy_radius_); }
    s.store_class_end();
  }
}

inputInvoiceMessage::inputInvoiceMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t inputInvoiceMessage::ID;

void inputInvoiceMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void inputInvoiceMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void inputInvoiceMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceMessage");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

inputInvoiceSlug::inputInvoiceSlug(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t inputInvoiceSlug::ID;

void inputInvoiceSlug::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputInvoiceSlug::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputInvoiceSlug::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceSlug");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

inputInvoicePremiumGiftCode::inputInvoicePremiumGiftCode(object_ptr<InputStorePaymentPurpose> &&purpose_, object_ptr<premiumGiftCodeOption> &&option_)
  : purpose_(std::move(purpose_))
  , option_(std::move(option_))
{}

const std::int32_t inputInvoicePremiumGiftCode::ID;

void inputInvoicePremiumGiftCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
  TlStoreBoxed<TlStoreObject, 629052971>::store(option_, s);
}

void inputInvoicePremiumGiftCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
  TlStoreBoxed<TlStoreObject, 629052971>::store(option_, s);
}

void inputInvoicePremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoicePremiumGiftCode");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_object_field("option", static_cast<const BaseObject *>(option_.get()));
    s.store_class_end();
  }
}

inputInvoiceStars::inputInvoiceStars(object_ptr<InputStorePaymentPurpose> &&purpose_)
  : purpose_(std::move(purpose_))
{}

const std::int32_t inputInvoiceStars::ID;

void inputInvoiceStars::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void inputInvoiceStars::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void inputInvoiceStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceStars");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

inputInvoiceChatInviteSubscription::inputInvoiceChatInviteSubscription(string const &hash_)
  : hash_(hash_)
{}

const std::int32_t inputInvoiceChatInviteSubscription::ID;

void inputInvoiceChatInviteSubscription::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(hash_, s);
}

void inputInvoiceChatInviteSubscription::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(hash_, s);
}

void inputInvoiceChatInviteSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceChatInviteSubscription");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

inputInvoiceStarGift::inputInvoiceStarGift(int32 flags_, bool hide_name_, bool include_upgrade_, object_ptr<InputPeer> &&peer_, int64 gift_id_, object_ptr<textWithEntities> &&message_)
  : flags_(flags_)
  , hide_name_(hide_name_)
  , include_upgrade_(include_upgrade_)
  , peer_(std::move(peer_))
  , gift_id_(gift_id_)
  , message_(std::move(message_))
{}

const std::int32_t inputInvoiceStarGift::ID;

void inputInvoiceStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (hide_name_ << 0) | (include_upgrade_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(gift_id_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s); }
}

void inputInvoiceStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (hide_name_ << 0) | (include_upgrade_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(gift_id_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s); }
}

void inputInvoiceStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceStarGift");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (hide_name_ << 0) | (include_upgrade_ << 2)));
    if (var0 & 1) { s.store_field("hide_name", true); }
    if (var0 & 4) { s.store_field("include_upgrade", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("gift_id", gift_id_);
    if (var0 & 2) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

inputInvoiceStarGiftUpgrade::inputInvoiceStarGiftUpgrade(int32 flags_, bool keep_original_details_, object_ptr<InputSavedStarGift> &&stargift_)
  : flags_(flags_)
  , keep_original_details_(keep_original_details_)
  , stargift_(std::move(stargift_))
{}

const std::int32_t inputInvoiceStarGiftUpgrade::ID;

void inputInvoiceStarGiftUpgrade::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (keep_original_details_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void inputInvoiceStarGiftUpgrade::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (keep_original_details_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void inputInvoiceStarGiftUpgrade::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceStarGiftUpgrade");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (keep_original_details_ << 0)));
    if (var0 & 1) { s.store_field("keep_original_details", true); }
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_class_end();
  }
}

inputInvoiceStarGiftTransfer::inputInvoiceStarGiftTransfer(object_ptr<InputSavedStarGift> &&stargift_, object_ptr<InputPeer> &&to_id_)
  : stargift_(std::move(stargift_))
  , to_id_(std::move(to_id_))
{}

const std::int32_t inputInvoiceStarGiftTransfer::ID;

void inputInvoiceStarGiftTransfer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_id_, s);
}

void inputInvoiceStarGiftTransfer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_id_, s);
}

void inputInvoiceStarGiftTransfer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceStarGiftTransfer");
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_object_field("to_id", static_cast<const BaseObject *>(to_id_.get()));
    s.store_class_end();
  }
}

inputInvoicePremiumGiftStars::inputInvoicePremiumGiftStars(int32 flags_, object_ptr<InputUser> &&user_id_, int32 months_, object_ptr<textWithEntities> &&message_)
  : flags_(flags_)
  , user_id_(std::move(user_id_))
  , months_(months_)
  , message_(std::move(message_))
{}

const std::int32_t inputInvoicePremiumGiftStars::ID;

void inputInvoicePremiumGiftStars::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(months_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s); }
}

void inputInvoicePremiumGiftStars::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(months_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s); }
}

void inputInvoicePremiumGiftStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoicePremiumGiftStars");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("months", months_);
    if (var0 & 1) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

inputInvoiceBusinessBotTransferStars::inputInvoiceBusinessBotTransferStars(object_ptr<InputUser> &&bot_, int64 stars_)
  : bot_(std::move(bot_))
  , stars_(stars_)
{}

const std::int32_t inputInvoiceBusinessBotTransferStars::ID;

void inputInvoiceBusinessBotTransferStars::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(stars_, s);
}

void inputInvoiceBusinessBotTransferStars::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(stars_, s);
}

void inputInvoiceBusinessBotTransferStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceBusinessBotTransferStars");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("stars", stars_);
    s.store_class_end();
  }
}

inputInvoiceStarGiftResale::inputInvoiceStarGiftResale(int32 flags_, bool ton_, string const &slug_, object_ptr<InputPeer> &&to_id_)
  : flags_(flags_)
  , ton_(ton_)
  , slug_(slug_)
  , to_id_(std::move(to_id_))
{}

const std::int32_t inputInvoiceStarGiftResale::ID;

void inputInvoiceStarGiftResale::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreString::store(slug_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_id_, s);
}

void inputInvoiceStarGiftResale::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreString::store(slug_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_id_, s);
}

void inputInvoiceStarGiftResale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceStarGiftResale");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (ton_ << 0)));
    if (var0 & 1) { s.store_field("ton", true); }
    s.store_field("slug", slug_);
    s.store_object_field("to_id", static_cast<const BaseObject *>(to_id_.get()));
    s.store_class_end();
  }
}

inputInvoiceStarGiftPrepaidUpgrade::inputInvoiceStarGiftPrepaidUpgrade(object_ptr<InputPeer> &&peer_, string const &hash_)
  : peer_(std::move(peer_))
  , hash_(hash_)
{}

const std::int32_t inputInvoiceStarGiftPrepaidUpgrade::ID;

void inputInvoiceStarGiftPrepaidUpgrade::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(hash_, s);
}

void inputInvoiceStarGiftPrepaidUpgrade::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(hash_, s);
}

void inputInvoiceStarGiftPrepaidUpgrade::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceStarGiftPrepaidUpgrade");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

inputInvoicePremiumAuthCode::inputInvoicePremiumAuthCode(object_ptr<InputStorePaymentPurpose> &&purpose_)
  : purpose_(std::move(purpose_))
{}

const std::int32_t inputInvoicePremiumAuthCode::ID;

void inputInvoicePremiumAuthCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void inputInvoicePremiumAuthCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void inputInvoicePremiumAuthCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoicePremiumAuthCode");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

inputInvoiceStarGiftDropOriginalDetails::inputInvoiceStarGiftDropOriginalDetails(object_ptr<InputSavedStarGift> &&stargift_)
  : stargift_(std::move(stargift_))
{}

const std::int32_t inputInvoiceStarGiftDropOriginalDetails::ID;

void inputInvoiceStarGiftDropOriginalDetails::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void inputInvoiceStarGiftDropOriginalDetails::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void inputInvoiceStarGiftDropOriginalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceStarGiftDropOriginalDetails");
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyStatusTimestamp::ID;

void inputPrivacyKeyStatusTimestamp::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyStatusTimestamp::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyStatusTimestamp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyStatusTimestamp");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyChatInvite::ID;

void inputPrivacyKeyChatInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyChatInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyChatInvite");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyPhoneCall::ID;

void inputPrivacyKeyPhoneCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyPhoneCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyPhoneCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyPhoneCall");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyPhoneP2P::ID;

void inputPrivacyKeyPhoneP2P::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyPhoneP2P::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyPhoneP2P::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyPhoneP2P");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyForwards::ID;

void inputPrivacyKeyForwards::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyForwards::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyForwards");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyProfilePhoto::ID;

void inputPrivacyKeyProfilePhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyProfilePhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyProfilePhoto");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyPhoneNumber::ID;

void inputPrivacyKeyPhoneNumber::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyPhoneNumber::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyPhoneNumber");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyAddedByPhone::ID;

void inputPrivacyKeyAddedByPhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyAddedByPhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyAddedByPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyAddedByPhone");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyVoiceMessages::ID;

void inputPrivacyKeyVoiceMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyVoiceMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyVoiceMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyVoiceMessages");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyAbout::ID;

void inputPrivacyKeyAbout::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyAbout::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyAbout::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyAbout");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyBirthday::ID;

void inputPrivacyKeyBirthday::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyBirthday::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyBirthday::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyBirthday");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyStarGiftsAutoSave::ID;

void inputPrivacyKeyStarGiftsAutoSave::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyStarGiftsAutoSave::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyStarGiftsAutoSave::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyStarGiftsAutoSave");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyKeyNoPaidMessages::ID;

void inputPrivacyKeyNoPaidMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyNoPaidMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyKeyNoPaidMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyKeyNoPaidMessages");
    s.store_class_end();
  }
}

inputQuickReplyShortcut::inputQuickReplyShortcut(string const &shortcut_)
  : shortcut_(shortcut_)
{}

const std::int32_t inputQuickReplyShortcut::ID;

void inputQuickReplyShortcut::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(shortcut_, s);
}

void inputQuickReplyShortcut::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(shortcut_, s);
}

void inputQuickReplyShortcut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputQuickReplyShortcut");
    s.store_field("shortcut", shortcut_);
    s.store_class_end();
  }
}

inputQuickReplyShortcutId::inputQuickReplyShortcutId(int32 shortcut_id_)
  : shortcut_id_(shortcut_id_)
{}

const std::int32_t inputQuickReplyShortcutId::ID;

void inputQuickReplyShortcutId::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(shortcut_id_, s);
}

void inputQuickReplyShortcutId::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(shortcut_id_, s);
}

void inputQuickReplyShortcutId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputQuickReplyShortcutId");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_class_end();
  }
}

object_ptr<InputStickerSet> InputStickerSet::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputStickerSetEmpty::ID:
      return inputStickerSetEmpty::fetch(p);
    case inputStickerSetID::ID:
      return inputStickerSetID::fetch(p);
    case inputStickerSetShortName::ID:
      return inputStickerSetShortName::fetch(p);
    case inputStickerSetAnimatedEmoji::ID:
      return inputStickerSetAnimatedEmoji::fetch(p);
    case inputStickerSetDice::ID:
      return inputStickerSetDice::fetch(p);
    case inputStickerSetAnimatedEmojiAnimations::ID:
      return inputStickerSetAnimatedEmojiAnimations::fetch(p);
    case inputStickerSetPremiumGifts::ID:
      return inputStickerSetPremiumGifts::fetch(p);
    case inputStickerSetEmojiGenericAnimations::ID:
      return inputStickerSetEmojiGenericAnimations::fetch(p);
    case inputStickerSetEmojiDefaultStatuses::ID:
      return inputStickerSetEmojiDefaultStatuses::fetch(p);
    case inputStickerSetEmojiDefaultTopicIcons::ID:
      return inputStickerSetEmojiDefaultTopicIcons::fetch(p);
    case inputStickerSetEmojiChannelDefaultStatuses::ID:
      return inputStickerSetEmojiChannelDefaultStatuses::fetch(p);
    case inputStickerSetTonGifts::ID:
      return inputStickerSetTonGifts::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputStickerSetEmpty::ID;

object_ptr<InputStickerSet> inputStickerSetEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetEmpty>();
}

void inputStickerSetEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetEmpty");
    s.store_class_end();
  }
}

inputStickerSetID::inputStickerSetID(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputStickerSetID::ID;

object_ptr<InputStickerSet> inputStickerSetID::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetID>(p);
}

inputStickerSetID::inputStickerSetID(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputStickerSetID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputStickerSetID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputStickerSetID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetID");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputStickerSetShortName::inputStickerSetShortName(string const &short_name_)
  : short_name_(short_name_)
{}

const std::int32_t inputStickerSetShortName::ID;

object_ptr<InputStickerSet> inputStickerSetShortName::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetShortName>(p);
}

inputStickerSetShortName::inputStickerSetShortName(TlBufferParser &p)
  : short_name_(TlFetchString<string>::parse(p))
{}

void inputStickerSetShortName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(short_name_, s);
}

void inputStickerSetShortName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(short_name_, s);
}

void inputStickerSetShortName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetShortName");
    s.store_field("short_name", short_name_);
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetAnimatedEmoji::ID;

object_ptr<InputStickerSet> inputStickerSetAnimatedEmoji::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetAnimatedEmoji>();
}

void inputStickerSetAnimatedEmoji::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetAnimatedEmoji::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetAnimatedEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetAnimatedEmoji");
    s.store_class_end();
  }
}

inputStickerSetDice::inputStickerSetDice(string const &emoticon_)
  : emoticon_(emoticon_)
{}

const std::int32_t inputStickerSetDice::ID;

object_ptr<InputStickerSet> inputStickerSetDice::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetDice>(p);
}

inputStickerSetDice::inputStickerSetDice(TlBufferParser &p)
  : emoticon_(TlFetchString<string>::parse(p))
{}

void inputStickerSetDice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void inputStickerSetDice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void inputStickerSetDice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetDice");
    s.store_field("emoticon", emoticon_);
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetAnimatedEmojiAnimations::ID;

object_ptr<InputStickerSet> inputStickerSetAnimatedEmojiAnimations::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetAnimatedEmojiAnimations>();
}

void inputStickerSetAnimatedEmojiAnimations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetAnimatedEmojiAnimations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetAnimatedEmojiAnimations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetAnimatedEmojiAnimations");
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetPremiumGifts::ID;

object_ptr<InputStickerSet> inputStickerSetPremiumGifts::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetPremiumGifts>();
}

void inputStickerSetPremiumGifts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetPremiumGifts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetPremiumGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetPremiumGifts");
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetEmojiGenericAnimations::ID;

object_ptr<InputStickerSet> inputStickerSetEmojiGenericAnimations::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetEmojiGenericAnimations>();
}

void inputStickerSetEmojiGenericAnimations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiGenericAnimations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiGenericAnimations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetEmojiGenericAnimations");
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetEmojiDefaultStatuses::ID;

object_ptr<InputStickerSet> inputStickerSetEmojiDefaultStatuses::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetEmojiDefaultStatuses>();
}

void inputStickerSetEmojiDefaultStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiDefaultStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiDefaultStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetEmojiDefaultStatuses");
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetEmojiDefaultTopicIcons::ID;

object_ptr<InputStickerSet> inputStickerSetEmojiDefaultTopicIcons::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetEmojiDefaultTopicIcons>();
}

void inputStickerSetEmojiDefaultTopicIcons::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiDefaultTopicIcons::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiDefaultTopicIcons::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetEmojiDefaultTopicIcons");
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetEmojiChannelDefaultStatuses::ID;

object_ptr<InputStickerSet> inputStickerSetEmojiChannelDefaultStatuses::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetEmojiChannelDefaultStatuses>();
}

void inputStickerSetEmojiChannelDefaultStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiChannelDefaultStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetEmojiChannelDefaultStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetEmojiChannelDefaultStatuses");
    s.store_class_end();
  }
}

const std::int32_t inputStickerSetTonGifts::ID;

object_ptr<InputStickerSet> inputStickerSetTonGifts::fetch(TlBufferParser &p) {
  return make_tl_object<inputStickerSetTonGifts>();
}

void inputStickerSetTonGifts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputStickerSetTonGifts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputStickerSetTonGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetTonGifts");
    s.store_class_end();
  }
}

keyboardButtonRow::keyboardButtonRow(array<object_ptr<KeyboardButton>> &&buttons_)
  : buttons_(std::move(buttons_))
{}

const std::int32_t keyboardButtonRow::ID;

object_ptr<keyboardButtonRow> keyboardButtonRow::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonRow>(p);
}

keyboardButtonRow::keyboardButtonRow(TlBufferParser &p)
  : buttons_(TlFetchBoxed<TlFetchVector<TlFetchObject<KeyboardButton>>, 481674261>::parse(p))
{}

void keyboardButtonRow::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(buttons_, s);
}

void keyboardButtonRow::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(buttons_, s);
}

void keyboardButtonRow::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonRow");
    { s.store_vector_begin("buttons", buttons_.size()); for (const auto &_value : buttons_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<MessagePeerVote> MessagePeerVote::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messagePeerVote::ID:
      return messagePeerVote::fetch(p);
    case messagePeerVoteInputOption::ID:
      return messagePeerVoteInputOption::fetch(p);
    case messagePeerVoteMultiple::ID:
      return messagePeerVoteMultiple::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messagePeerVote::ID;

object_ptr<MessagePeerVote> messagePeerVote::fetch(TlBufferParser &p) {
  return make_tl_object<messagePeerVote>(p);
}

messagePeerVote::messagePeerVote(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , option_(TlFetchBytes<bytes>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void messagePeerVote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePeerVote");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_bytes_field("option", option_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t messagePeerVoteInputOption::ID;

object_ptr<MessagePeerVote> messagePeerVoteInputOption::fetch(TlBufferParser &p) {
  return make_tl_object<messagePeerVoteInputOption>(p);
}

messagePeerVoteInputOption::messagePeerVoteInputOption(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void messagePeerVoteInputOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePeerVoteInputOption");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t messagePeerVoteMultiple::ID;

object_ptr<MessagePeerVote> messagePeerVoteMultiple::fetch(TlBufferParser &p) {
  return make_tl_object<messagePeerVoteMultiple>(p);
}

messagePeerVoteMultiple::messagePeerVoteMultiple(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , options_(TlFetchBoxed<TlFetchVector<TlFetchBytes<bytes>>, 481674261>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void messagePeerVoteMultiple::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePeerVoteMultiple");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_field("date", date_);
    s.store_class_end();
  }
}

messageReplies::messageReplies()
  : flags_()
  , comments_()
  , replies_()
  , replies_pts_()
  , recent_repliers_()
  , channel_id_()
  , max_id_()
  , read_max_id_()
{}

const std::int32_t messageReplies::ID;

object_ptr<messageReplies> messageReplies::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageReplies> res = make_tl_object<messageReplies>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->comments_ = (var0 & 1) != 0;
  res->replies_ = TlFetchInt::parse(p);
  res->replies_pts_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->recent_repliers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p); }
  if (var0 & 1) { res->channel_id_ = TlFetchLong::parse(p); }
  if (var0 & 4) { res->max_id_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->read_max_id_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messageReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReplies");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (comments_ << 0)));
    if (var0 & 1) { s.store_field("comments", true); }
    s.store_field("replies", replies_);
    s.store_field("replies_pts", replies_pts_);
    if (var0 & 2) { { s.store_vector_begin("recent_repliers", recent_repliers_.size()); for (const auto &_value : recent_repliers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 1) { s.store_field("channel_id", channel_id_); }
    if (var0 & 4) { s.store_field("max_id", max_id_); }
    if (var0 & 8) { s.store_field("read_max_id", read_max_id_); }
    s.store_class_end();
  }
}

const std::int32_t pageCaption::ID;

object_ptr<pageCaption> pageCaption::fetch(TlBufferParser &p) {
  return make_tl_object<pageCaption>(p);
}

pageCaption::pageCaption(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , credit_(TlFetchObject<RichText>::parse(p))
{}

void pageCaption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageCaption");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("credit", static_cast<const BaseObject *>(credit_.get()));
    s.store_class_end();
  }
}

object_ptr<PageListOrderedItem> PageListOrderedItem::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case pageListOrderedItemText::ID:
      return pageListOrderedItemText::fetch(p);
    case pageListOrderedItemBlocks::ID:
      return pageListOrderedItemBlocks::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t pageListOrderedItemText::ID;

object_ptr<PageListOrderedItem> pageListOrderedItemText::fetch(TlBufferParser &p) {
  return make_tl_object<pageListOrderedItemText>(p);
}

pageListOrderedItemText::pageListOrderedItemText(TlBufferParser &p)
  : num_(TlFetchString<string>::parse(p))
  , text_(TlFetchObject<RichText>::parse(p))
{}

void pageListOrderedItemText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageListOrderedItemText");
    s.store_field("num", num_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageListOrderedItemBlocks::ID;

object_ptr<PageListOrderedItem> pageListOrderedItemBlocks::fetch(TlBufferParser &p) {
  return make_tl_object<pageListOrderedItemBlocks>(p);
}

pageListOrderedItemBlocks::pageListOrderedItemBlocks(TlBufferParser &p)
  : num_(TlFetchString<string>::parse(p))
  , blocks_(TlFetchBoxed<TlFetchVector<TlFetchObject<PageBlock>>, 481674261>::parse(p))
{}

void pageListOrderedItemBlocks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageListOrderedItemBlocks");
    s.store_field("num", num_);
    { s.store_vector_begin("blocks", blocks_.size()); for (const auto &_value : blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pageTableCell::pageTableCell()
  : flags_()
  , header_()
  , align_center_()
  , align_right_()
  , valign_middle_()
  , valign_bottom_()
  , text_()
  , colspan_()
  , rowspan_()
{}

const std::int32_t pageTableCell::ID;

object_ptr<pageTableCell> pageTableCell::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pageTableCell> res = make_tl_object<pageTableCell>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->header_ = (var0 & 1) != 0;
  res->align_center_ = (var0 & 8) != 0;
  res->align_right_ = (var0 & 16) != 0;
  res->valign_middle_ = (var0 & 32) != 0;
  res->valign_bottom_ = (var0 & 64) != 0;
  if (var0 & 128) { res->text_ = TlFetchObject<RichText>::parse(p); }
  if (var0 & 2) { res->colspan_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->rowspan_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void pageTableCell::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageTableCell");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (header_ << 0) | (align_center_ << 3) | (align_right_ << 4) | (valign_middle_ << 5) | (valign_bottom_ << 6)));
    if (var0 & 1) { s.store_field("header", true); }
    if (var0 & 8) { s.store_field("align_center", true); }
    if (var0 & 16) { s.store_field("align_right", true); }
    if (var0 & 32) { s.store_field("valign_middle", true); }
    if (var0 & 64) { s.store_field("valign_bottom", true); }
    if (var0 & 128) { s.store_object_field("text", static_cast<const BaseObject *>(text_.get())); }
    if (var0 & 2) { s.store_field("colspan", colspan_); }
    if (var0 & 4) { s.store_field("rowspan", rowspan_); }
    s.store_class_end();
  }
}

const std::int32_t pageTableRow::ID;

object_ptr<pageTableRow> pageTableRow::fetch(TlBufferParser &p) {
  return make_tl_object<pageTableRow>(p);
}

pageTableRow::pageTableRow(TlBufferParser &p)
  : cells_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<pageTableCell>, 878078826>>, 481674261>::parse(p))
{}

void pageTableRow::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageTableRow");
    { s.store_vector_begin("cells", cells_.size()); for (const auto &_value : cells_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<PaidReactionPrivacy> PaidReactionPrivacy::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case paidReactionPrivacyDefault::ID:
      return paidReactionPrivacyDefault::fetch(p);
    case paidReactionPrivacyAnonymous::ID:
      return paidReactionPrivacyAnonymous::fetch(p);
    case paidReactionPrivacyPeer::ID:
      return paidReactionPrivacyPeer::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t paidReactionPrivacyDefault::ID;

object_ptr<PaidReactionPrivacy> paidReactionPrivacyDefault::fetch(TlBufferParser &p) {
  return make_tl_object<paidReactionPrivacyDefault>();
}

void paidReactionPrivacyDefault::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void paidReactionPrivacyDefault::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void paidReactionPrivacyDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidReactionPrivacyDefault");
    s.store_class_end();
  }
}

const std::int32_t paidReactionPrivacyAnonymous::ID;

object_ptr<PaidReactionPrivacy> paidReactionPrivacyAnonymous::fetch(TlBufferParser &p) {
  return make_tl_object<paidReactionPrivacyAnonymous>();
}

void paidReactionPrivacyAnonymous::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void paidReactionPrivacyAnonymous::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void paidReactionPrivacyAnonymous::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidReactionPrivacyAnonymous");
    s.store_class_end();
  }
}

paidReactionPrivacyPeer::paidReactionPrivacyPeer(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t paidReactionPrivacyPeer::ID;

object_ptr<PaidReactionPrivacy> paidReactionPrivacyPeer::fetch(TlBufferParser &p) {
  return make_tl_object<paidReactionPrivacyPeer>(p);
}

paidReactionPrivacyPeer::paidReactionPrivacyPeer(TlBufferParser &p)
  : peer_(TlFetchObject<InputPeer>::parse(p))
{}

void paidReactionPrivacyPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void paidReactionPrivacyPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void paidReactionPrivacyPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidReactionPrivacyPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

object_ptr<PasswordKdfAlgo> PasswordKdfAlgo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case passwordKdfAlgoUnknown::ID:
      return passwordKdfAlgoUnknown::fetch(p);
    case passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::ID:
      return passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t passwordKdfAlgoUnknown::ID;

object_ptr<PasswordKdfAlgo> passwordKdfAlgoUnknown::fetch(TlBufferParser &p) {
  return make_tl_object<passwordKdfAlgoUnknown>();
}

void passwordKdfAlgoUnknown::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void passwordKdfAlgoUnknown::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void passwordKdfAlgoUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passwordKdfAlgoUnknown");
    s.store_class_end();
  }
}

passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow(bytes &&salt1_, bytes &&salt2_, int32 g_, bytes &&p_)
  : salt1_(std::move(salt1_))
  , salt2_(std::move(salt2_))
  , g_(g_)
  , p_(std::move(p_))
{}

const std::int32_t passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::ID;

object_ptr<PasswordKdfAlgo> passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::fetch(TlBufferParser &p) {
  return make_tl_object<passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow>(p);
}

passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow(TlBufferParser &p)
  : salt1_(TlFetchBytes<bytes>::parse(p))
  , salt2_(TlFetchBytes<bytes>::parse(p))
  , g_(TlFetchInt::parse(p))
  , p_(TlFetchBytes<bytes>::parse(p))
{}

void passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(salt1_, s);
  TlStoreString::store(salt2_, s);
  TlStoreBinary::store(g_, s);
  TlStoreString::store(p_, s);
}

void passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(salt1_, s);
  TlStoreString::store(salt2_, s);
  TlStoreBinary::store(g_, s);
  TlStoreString::store(p_, s);
}

void passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow");
    s.store_bytes_field("salt1", salt1_);
    s.store_bytes_field("salt2", salt2_);
    s.store_field("g", g_);
    s.store_bytes_field("p", p_);
    s.store_class_end();
  }
}

const std::int32_t quickReply::ID;

object_ptr<quickReply> quickReply::fetch(TlBufferParser &p) {
  return make_tl_object<quickReply>(p);
}

quickReply::quickReply(TlBufferParser &p)
  : shortcut_id_(TlFetchInt::parse(p))
  , shortcut_(TlFetchString<string>::parse(p))
  , top_message_(TlFetchInt::parse(p))
  , count_(TlFetchInt::parse(p))
{}

void quickReply::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "quickReply");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_field("shortcut", shortcut_);
    s.store_field("top_message", top_message_);
    s.store_field("count", count_);
    s.store_class_end();
  }
}

const std::int32_t savedPhoneContact::ID;

object_ptr<savedPhoneContact> savedPhoneContact::fetch(TlBufferParser &p) {
  return make_tl_object<savedPhoneContact>(p);
}

savedPhoneContact::savedPhoneContact(TlBufferParser &p)
  : phone_(TlFetchString<string>::parse(p))
  , first_name_(TlFetchString<string>::parse(p))
  , last_name_(TlFetchString<string>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void savedPhoneContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedPhoneContact");
    s.store_field("phone", phone_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

secureCredentialsEncrypted::secureCredentialsEncrypted(bytes &&data_, bytes &&hash_, bytes &&secret_)
  : data_(std::move(data_))
  , hash_(std::move(hash_))
  , secret_(std::move(secret_))
{}

const std::int32_t secureCredentialsEncrypted::ID;

object_ptr<secureCredentialsEncrypted> secureCredentialsEncrypted::fetch(TlBufferParser &p) {
  return make_tl_object<secureCredentialsEncrypted>(p);
}

secureCredentialsEncrypted::secureCredentialsEncrypted(TlBufferParser &p)
  : data_(TlFetchBytes<bytes>::parse(p))
  , hash_(TlFetchBytes<bytes>::parse(p))
  , secret_(TlFetchBytes<bytes>::parse(p))
{}

void secureCredentialsEncrypted::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(data_, s);
  TlStoreString::store(hash_, s);
  TlStoreString::store(secret_, s);
}

void secureCredentialsEncrypted::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(data_, s);
  TlStoreString::store(hash_, s);
  TlStoreString::store(secret_, s);
}

void secureCredentialsEncrypted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureCredentialsEncrypted");
    s.store_bytes_field("data", data_);
    s.store_bytes_field("hash", hash_);
    s.store_bytes_field("secret", secret_);
    s.store_class_end();
  }
}

object_ptr<SecureFile> SecureFile::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case secureFileEmpty::ID:
      return secureFileEmpty::fetch(p);
    case secureFile::ID:
      return secureFile::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t secureFileEmpty::ID;

object_ptr<SecureFile> secureFileEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<secureFileEmpty>();
}

void secureFileEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureFileEmpty");
    s.store_class_end();
  }
}

const std::int32_t secureFile::ID;

object_ptr<SecureFile> secureFile::fetch(TlBufferParser &p) {
  return make_tl_object<secureFile>(p);
}

secureFile::secureFile(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , size_(TlFetchLong::parse(p))
  , dc_id_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
  , file_hash_(TlFetchBytes<bytes>::parse(p))
  , secret_(TlFetchBytes<bytes>::parse(p))
{}

void secureFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureFile");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("size", size_);
    s.store_field("dc_id", dc_id_);
    s.store_field("date", date_);
    s.store_bytes_field("file_hash", file_hash_);
    s.store_bytes_field("secret", secret_);
    s.store_class_end();
  }
}

shippingOption::shippingOption(string const &id_, string const &title_, array<object_ptr<labeledPrice>> &&prices_)
  : id_(id_)
  , title_(title_)
  , prices_(std::move(prices_))
{}

const std::int32_t shippingOption::ID;

object_ptr<shippingOption> shippingOption::fetch(TlBufferParser &p) {
  return make_tl_object<shippingOption>(p);
}

shippingOption::shippingOption(TlBufferParser &p)
  : id_(TlFetchString<string>::parse(p))
  , title_(TlFetchString<string>::parse(p))
  , prices_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<labeledPrice>, -886477832>>, 481674261>::parse(p))
{}

void shippingOption::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -886477832>>, 481674261>::store(prices_, s);
}

void shippingOption::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(id_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -886477832>>, 481674261>::store(prices_, s);
}

void shippingOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "shippingOption");
    s.store_field("id", id_);
    s.store_field("title", title_);
    { s.store_vector_begin("prices", prices_.size()); for (const auto &_value : prices_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starsRevenueStatus::starsRevenueStatus()
  : flags_()
  , withdrawal_enabled_()
  , current_balance_()
  , available_balance_()
  , overall_revenue_()
  , next_withdrawal_at_()
{}

const std::int32_t starsRevenueStatus::ID;

object_ptr<starsRevenueStatus> starsRevenueStatus::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsRevenueStatus> res = make_tl_object<starsRevenueStatus>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->withdrawal_enabled_ = (var0 & 1) != 0;
  res->current_balance_ = TlFetchObject<StarsAmount>::parse(p);
  res->available_balance_ = TlFetchObject<StarsAmount>::parse(p);
  res->overall_revenue_ = TlFetchObject<StarsAmount>::parse(p);
  if (var0 & 2) { res->next_withdrawal_at_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starsRevenueStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsRevenueStatus");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (withdrawal_enabled_ << 0)));
    if (var0 & 1) { s.store_field("withdrawal_enabled", true); }
    s.store_object_field("current_balance", static_cast<const BaseObject *>(current_balance_.get()));
    s.store_object_field("available_balance", static_cast<const BaseObject *>(available_balance_.get()));
    s.store_object_field("overall_revenue", static_cast<const BaseObject *>(overall_revenue_.get()));
    if (var0 & 2) { s.store_field("next_withdrawal_at", next_withdrawal_at_); }
    s.store_class_end();
  }
}

object_ptr<StatsGraph> StatsGraph::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case statsGraphAsync::ID:
      return statsGraphAsync::fetch(p);
    case statsGraphError::ID:
      return statsGraphError::fetch(p);
    case statsGraph::ID:
      return statsGraph::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t statsGraphAsync::ID;

object_ptr<StatsGraph> statsGraphAsync::fetch(TlBufferParser &p) {
  return make_tl_object<statsGraphAsync>(p);
}

statsGraphAsync::statsGraphAsync(TlBufferParser &p)
  : token_(TlFetchString<string>::parse(p))
{}

void statsGraphAsync::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsGraphAsync");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

statsGraphError::statsGraphError(string const &error_)
  : error_(error_)
{}

const std::int32_t statsGraphError::ID;

object_ptr<StatsGraph> statsGraphError::fetch(TlBufferParser &p) {
  return make_tl_object<statsGraphError>(p);
}

statsGraphError::statsGraphError(TlBufferParser &p)
  : error_(TlFetchString<string>::parse(p))
{}

void statsGraphError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsGraphError");
    s.store_field("error", error_);
    s.store_class_end();
  }
}

statsGraph::statsGraph()
  : flags_()
  , json_()
  , zoom_token_()
{}

const std::int32_t statsGraph::ID;

object_ptr<StatsGraph> statsGraph::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<statsGraph> res = make_tl_object<statsGraph>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->json_ = TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p);
  if (var0 & 1) { res->zoom_token_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void statsGraph::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsGraph");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("json", static_cast<const BaseObject *>(json_.get()));
    if (var0 & 1) { s.store_field("zoom_token", zoom_token_); }
    s.store_class_end();
  }
}

const std::int32_t timezone::ID;

object_ptr<timezone> timezone::fetch(TlBufferParser &p) {
  return make_tl_object<timezone>(p);
}

timezone::timezone(TlBufferParser &p)
  : id_(TlFetchString<string>::parse(p))
  , name_(TlFetchString<string>::parse(p))
  , utc_offset_(TlFetchInt::parse(p))
{}

void timezone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "timezone");
    s.store_field("id", id_);
    s.store_field("name", name_);
    s.store_field("utc_offset", utc_offset_);
    s.store_class_end();
  }
}

object_ptr<UserStatus> UserStatus::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case userStatusEmpty::ID:
      return userStatusEmpty::fetch(p);
    case userStatusOnline::ID:
      return userStatusOnline::fetch(p);
    case userStatusOffline::ID:
      return userStatusOffline::fetch(p);
    case userStatusRecently::ID:
      return userStatusRecently::fetch(p);
    case userStatusLastWeek::ID:
      return userStatusLastWeek::fetch(p);
    case userStatusLastMonth::ID:
      return userStatusLastMonth::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t userStatusEmpty::ID;

object_ptr<UserStatus> userStatusEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<userStatusEmpty>();
}

void userStatusEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusEmpty");
    s.store_class_end();
  }
}

const std::int32_t userStatusOnline::ID;

object_ptr<UserStatus> userStatusOnline::fetch(TlBufferParser &p) {
  return make_tl_object<userStatusOnline>(p);
}

userStatusOnline::userStatusOnline(TlBufferParser &p)
  : expires_(TlFetchInt::parse(p))
{}

void userStatusOnline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusOnline");
    s.store_field("expires", expires_);
    s.store_class_end();
  }
}

const std::int32_t userStatusOffline::ID;

object_ptr<UserStatus> userStatusOffline::fetch(TlBufferParser &p) {
  return make_tl_object<userStatusOffline>(p);
}

userStatusOffline::userStatusOffline(TlBufferParser &p)
  : was_online_(TlFetchInt::parse(p))
{}

void userStatusOffline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusOffline");
    s.store_field("was_online", was_online_);
    s.store_class_end();
  }
}

userStatusRecently::userStatusRecently()
  : flags_()
  , by_me_()
{}

const std::int32_t userStatusRecently::ID;

object_ptr<UserStatus> userStatusRecently::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<userStatusRecently> res = make_tl_object<userStatusRecently>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->by_me_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void userStatusRecently::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusRecently");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (by_me_ << 0)));
    if (var0 & 1) { s.store_field("by_me", true); }
    s.store_class_end();
  }
}

userStatusLastWeek::userStatusLastWeek()
  : flags_()
  , by_me_()
{}

const std::int32_t userStatusLastWeek::ID;

object_ptr<UserStatus> userStatusLastWeek::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<userStatusLastWeek> res = make_tl_object<userStatusLastWeek>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->by_me_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void userStatusLastWeek::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusLastWeek");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (by_me_ << 0)));
    if (var0 & 1) { s.store_field("by_me", true); }
    s.store_class_end();
  }
}

userStatusLastMonth::userStatusLastMonth()
  : flags_()
  , by_me_()
{}

const std::int32_t userStatusLastMonth::ID;

object_ptr<UserStatus> userStatusLastMonth::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<userStatusLastMonth> res = make_tl_object<userStatusLastMonth>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->by_me_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void userStatusLastMonth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusLastMonth");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (by_me_ << 0)));
    if (var0 & 1) { s.store_field("by_me", true); }
    s.store_class_end();
  }
}

const std::int32_t webAuthorization::ID;

object_ptr<webAuthorization> webAuthorization::fetch(TlBufferParser &p) {
  return make_tl_object<webAuthorization>(p);
}

webAuthorization::webAuthorization(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , bot_id_(TlFetchLong::parse(p))
  , domain_(TlFetchString<string>::parse(p))
  , browser_(TlFetchString<string>::parse(p))
  , platform_(TlFetchString<string>::parse(p))
  , date_created_(TlFetchInt::parse(p))
  , date_active_(TlFetchInt::parse(p))
  , ip_(TlFetchString<string>::parse(p))
  , region_(TlFetchString<string>::parse(p))
{}

void webAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webAuthorization");
    s.store_field("hash", hash_);
    s.store_field("bot_id", bot_id_);
    s.store_field("domain", domain_);
    s.store_field("browser", browser_);
    s.store_field("platform", platform_);
    s.store_field("date_created", date_created_);
    s.store_field("date_active", date_active_);
    s.store_field("ip", ip_);
    s.store_field("region", region_);
    s.store_class_end();
  }
}

object_ptr<WebPageAttribute> WebPageAttribute::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case webPageAttributeTheme::ID:
      return webPageAttributeTheme::fetch(p);
    case webPageAttributeStory::ID:
      return webPageAttributeStory::fetch(p);
    case webPageAttributeStickerSet::ID:
      return webPageAttributeStickerSet::fetch(p);
    case webPageAttributeUniqueStarGift::ID:
      return webPageAttributeUniqueStarGift::fetch(p);
    case webPageAttributeStarGiftCollection::ID:
      return webPageAttributeStarGiftCollection::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

webPageAttributeTheme::webPageAttributeTheme()
  : flags_()
  , documents_()
  , settings_()
{}

const std::int32_t webPageAttributeTheme::ID;

object_ptr<WebPageAttribute> webPageAttributeTheme::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webPageAttributeTheme> res = make_tl_object<webPageAttributeTheme>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->documents_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p); }
  if (var0 & 2) { res->settings_ = TlFetchBoxed<TlFetchObject<themeSettings>, -94849324>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void webPageAttributeTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageAttributeTheme");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get())); }
    s.store_class_end();
  }
}

webPageAttributeStory::webPageAttributeStory()
  : flags_()
  , peer_()
  , id_()
  , story_()
{}

const std::int32_t webPageAttributeStory::ID;

object_ptr<WebPageAttribute> webPageAttributeStory::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webPageAttributeStory> res = make_tl_object<webPageAttributeStory>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->id_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->story_ = TlFetchObject<StoryItem>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void webPageAttributeStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageAttributeStory");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("story", static_cast<const BaseObject *>(story_.get())); }
    s.store_class_end();
  }
}

webPageAttributeStickerSet::webPageAttributeStickerSet()
  : flags_()
  , emojis_()
  , text_color_()
  , stickers_()
{}

const std::int32_t webPageAttributeStickerSet::ID;

object_ptr<WebPageAttribute> webPageAttributeStickerSet::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webPageAttributeStickerSet> res = make_tl_object<webPageAttributeStickerSet>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->emojis_ = (var0 & 1) != 0;
  res->text_color_ = (var0 & 2) != 0;
  res->stickers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void webPageAttributeStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageAttributeStickerSet");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (emojis_ << 0) | (text_color_ << 1)));
    if (var0 & 1) { s.store_field("emojis", true); }
    if (var0 & 2) { s.store_field("text_color", true); }
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t webPageAttributeUniqueStarGift::ID;

object_ptr<WebPageAttribute> webPageAttributeUniqueStarGift::fetch(TlBufferParser &p) {
  return make_tl_object<webPageAttributeUniqueStarGift>(p);
}

webPageAttributeUniqueStarGift::webPageAttributeUniqueStarGift(TlBufferParser &p)
  : gift_(TlFetchObject<StarGift>::parse(p))
{}

void webPageAttributeUniqueStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageAttributeUniqueStarGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

const std::int32_t webPageAttributeStarGiftCollection::ID;

object_ptr<WebPageAttribute> webPageAttributeStarGiftCollection::fetch(TlBufferParser &p) {
  return make_tl_object<webPageAttributeStarGiftCollection>(p);
}

webPageAttributeStarGiftCollection::webPageAttributeStarGiftCollection(TlBufferParser &p)
  : icons_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void webPageAttributeStarGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageAttributeStarGiftCollection");
    { s.store_vector_begin("icons", icons_.size()); for (const auto &_value : icons_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_authorizationForm::account_authorizationForm()
  : flags_()
  , required_types_()
  , values_()
  , errors_()
  , users_()
  , privacy_policy_url_()
{}

const std::int32_t account_authorizationForm::ID;

object_ptr<account_authorizationForm> account_authorizationForm::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<account_authorizationForm> res = make_tl_object<account_authorizationForm>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->required_types_ = TlFetchBoxed<TlFetchVector<TlFetchObject<SecureRequiredType>>, 481674261>::parse(p);
  res->values_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<secureValue>, 411017418>>, 481674261>::parse(p);
  res->errors_ = TlFetchBoxed<TlFetchVector<TlFetchObject<SecureValueError>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (var0 & 1) { res->privacy_policy_url_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void account_authorizationForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.authorizationForm");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    { s.store_vector_begin("required_types", required_types_.size()); for (const auto &_value : required_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("values", values_.size()); for (const auto &_value : values_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("errors", errors_.size()); for (const auto &_value : errors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("privacy_policy_url", privacy_policy_url_); }
    s.store_class_end();
  }
}

const std::int32_t account_privacyRules::ID;

object_ptr<account_privacyRules> account_privacyRules::fetch(TlBufferParser &p) {
  return make_tl_object<account_privacyRules>(p);
}

account_privacyRules::account_privacyRules(TlBufferParser &p)
  : rules_(TlFetchBoxed<TlFetchVector<TlFetchObject<PrivacyRule>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void account_privacyRules::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.privacyRules");
    { s.store_vector_begin("rules", rules_.size()); for (const auto &_value : rules_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_sentEmailCode::ID;

object_ptr<account_sentEmailCode> account_sentEmailCode::fetch(TlBufferParser &p) {
  return make_tl_object<account_sentEmailCode>(p);
}

account_sentEmailCode::account_sentEmailCode(TlBufferParser &p)
  : email_pattern_(TlFetchString<string>::parse(p))
  , length_(TlFetchInt::parse(p))
{}

void account_sentEmailCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.sentEmailCode");
    s.store_field("email_pattern", email_pattern_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

bots_popularAppBots::bots_popularAppBots()
  : flags_()
  , next_offset_()
  , users_()
{}

const std::int32_t bots_popularAppBots::ID;

object_ptr<bots_popularAppBots> bots_popularAppBots::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<bots_popularAppBots> res = make_tl_object<bots_popularAppBots>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void bots_popularAppBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.popularAppBots");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t contacts_importedContacts::ID;

object_ptr<contacts_importedContacts> contacts_importedContacts::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_importedContacts>(p);
}

contacts_importedContacts::contacts_importedContacts(TlBufferParser &p)
  : imported_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<importedContact>, -1052885936>>, 481674261>::parse(p))
  , popular_invites_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<popularContact>, 1558266229>>, 481674261>::parse(p))
  , retry_contacts_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_importedContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.importedContacts");
    { s.store_vector_begin("imported", imported_.size()); for (const auto &_value : imported_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("popular_invites", popular_invites_.size()); for (const auto &_value : popular_invites_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("retry_contacts", retry_contacts_.size()); for (const auto &_value : retry_contacts_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

help_termsOfService::help_termsOfService()
  : flags_()
  , popup_()
  , id_()
  , text_()
  , entities_()
  , min_age_confirm_()
{}

const std::int32_t help_termsOfService::ID;

object_ptr<help_termsOfService> help_termsOfService::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<help_termsOfService> res = make_tl_object<help_termsOfService>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->popup_ = (var0 & 1) != 0;
  res->id_ = TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p);
  res->text_ = TlFetchString<string>::parse(p);
  res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p);
  if (var0 & 2) { res->min_age_confirm_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void help_termsOfService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.termsOfService");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (popup_ << 0)));
    if (var0 & 1) { s.store_field("popup", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_field("text", text_);
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 2) { s.store_field("min_age_confirm", min_age_confirm_); }
    s.store_class_end();
  }
}

const std::int32_t messages_affectedHistory::ID;

object_ptr<messages_affectedHistory> messages_affectedHistory::fetch(TlBufferParser &p) {
  return make_tl_object<messages_affectedHistory>(p);
}

messages_affectedHistory::messages_affectedHistory(TlBufferParser &p)
  : pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
  , offset_(TlFetchInt::parse(p))
{}

void messages_affectedHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.affectedHistory");
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

messages_botCallbackAnswer::messages_botCallbackAnswer()
  : flags_()
  , alert_()
  , has_url_()
  , native_ui_()
  , message_()
  , url_()
  , cache_time_()
{}

const std::int32_t messages_botCallbackAnswer::ID;

object_ptr<messages_botCallbackAnswer> messages_botCallbackAnswer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_botCallbackAnswer> res = make_tl_object<messages_botCallbackAnswer>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->alert_ = (var0 & 2) != 0;
  res->has_url_ = (var0 & 8) != 0;
  res->native_ui_ = (var0 & 16) != 0;
  if (var0 & 1) { res->message_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->url_ = TlFetchString<string>::parse(p); }
  res->cache_time_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_botCallbackAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.botCallbackAnswer");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (alert_ << 1) | (has_url_ << 3) | (native_ui_ << 4)));
    if (var0 & 2) { s.store_field("alert", true); }
    if (var0 & 8) { s.store_field("has_url", true); }
    if (var0 & 16) { s.store_field("native_ui", true); }
    if (var0 & 1) { s.store_field("message", message_); }
    if (var0 & 4) { s.store_field("url", url_); }
    s.store_field("cache_time", cache_time_);
    s.store_class_end();
  }
}

const std::int32_t messages_chatInviteImporters::ID;

object_ptr<messages_chatInviteImporters> messages_chatInviteImporters::fetch(TlBufferParser &p) {
  return make_tl_object<messages_chatInviteImporters>(p);
}

messages_chatInviteImporters::messages_chatInviteImporters(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , importers_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<chatInviteImporter>, -1940201511>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_chatInviteImporters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.chatInviteImporters");
    s.store_field("count", count_);
    { s.store_vector_begin("importers", importers_.size()); for (const auto &_value : importers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_DhConfig> messages_DhConfig::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_dhConfigNotModified::ID:
      return messages_dhConfigNotModified::fetch(p);
    case messages_dhConfig::ID:
      return messages_dhConfig::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_dhConfigNotModified::ID;

object_ptr<messages_DhConfig> messages_dhConfigNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_dhConfigNotModified>(p);
}

messages_dhConfigNotModified::messages_dhConfigNotModified(TlBufferParser &p)
  : random_(TlFetchBytes<bytes>::parse(p))
{}

void messages_dhConfigNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.dhConfigNotModified");
    s.store_bytes_field("random", random_);
    s.store_class_end();
  }
}

const std::int32_t messages_dhConfig::ID;

object_ptr<messages_DhConfig> messages_dhConfig::fetch(TlBufferParser &p) {
  return make_tl_object<messages_dhConfig>(p);
}

messages_dhConfig::messages_dhConfig(TlBufferParser &p)
  : g_(TlFetchInt::parse(p))
  , p_(TlFetchBytes<bytes>::parse(p))
  , version_(TlFetchInt::parse(p))
  , random_(TlFetchBytes<bytes>::parse(p))
{}

void messages_dhConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.dhConfig");
    s.store_field("g", g_);
    s.store_bytes_field("p", p_);
    s.store_field("version", version_);
    s.store_bytes_field("random", random_);
    s.store_class_end();
  }
}

object_ptr<messages_FavedStickers> messages_FavedStickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_favedStickersNotModified::ID:
      return messages_favedStickersNotModified::fetch(p);
    case messages_favedStickers::ID:
      return messages_favedStickers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_favedStickersNotModified::ID;

object_ptr<messages_FavedStickers> messages_favedStickersNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_favedStickersNotModified>();
}

void messages_favedStickersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.favedStickersNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_favedStickers::ID;

object_ptr<messages_FavedStickers> messages_favedStickers::fetch(TlBufferParser &p) {
  return make_tl_object<messages_favedStickers>(p);
}

messages_favedStickers::messages_favedStickers(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , packs_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<stickerPack>, 313694676>>, 481674261>::parse(p))
  , stickers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void messages_favedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.favedStickers");
    s.store_field("hash", hash_);
    { s.store_vector_begin("packs", packs_.size()); for (const auto &_value : packs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_SavedGifs> messages_SavedGifs::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_savedGifsNotModified::ID:
      return messages_savedGifsNotModified::fetch(p);
    case messages_savedGifs::ID:
      return messages_savedGifs::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_savedGifsNotModified::ID;

object_ptr<messages_SavedGifs> messages_savedGifsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_savedGifsNotModified>();
}

void messages_savedGifsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savedGifsNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_savedGifs::ID;

object_ptr<messages_SavedGifs> messages_savedGifs::fetch(TlBufferParser &p) {
  return make_tl_object<messages_savedGifs>(p);
}

messages_savedGifs::messages_savedGifs(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , gifs_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void messages_savedGifs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savedGifs");
    s.store_field("hash", hash_);
    { s.store_vector_begin("gifs", gifs_.size()); for (const auto &_value : gifs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_searchResultsPositions::ID;

object_ptr<messages_searchResultsPositions> messages_searchResultsPositions::fetch(TlBufferParser &p) {
  return make_tl_object<messages_searchResultsPositions>(p);
}

messages_searchResultsPositions::messages_searchResultsPositions(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , positions_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<searchResultPosition>, 2137295719>>, 481674261>::parse(p))
{}

void messages_searchResultsPositions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchResultsPositions");
    s.store_field("count", count_);
    { s.store_vector_begin("positions", positions_.size()); for (const auto &_value : positions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_Stickers> messages_Stickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_stickersNotModified::ID:
      return messages_stickersNotModified::fetch(p);
    case messages_stickers::ID:
      return messages_stickers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_stickersNotModified::ID;

object_ptr<messages_Stickers> messages_stickersNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_stickersNotModified>();
}

void messages_stickersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.stickersNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_stickers::ID;

object_ptr<messages_Stickers> messages_stickers::fetch(TlBufferParser &p) {
  return make_tl_object<messages_stickers>(p);
}

messages_stickers::messages_stickers(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , stickers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void messages_stickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.stickers");
    s.store_field("hash", hash_);
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<payments_GiveawayInfo> payments_GiveawayInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case payments_giveawayInfo::ID:
      return payments_giveawayInfo::fetch(p);
    case payments_giveawayInfoResults::ID:
      return payments_giveawayInfoResults::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

payments_giveawayInfo::payments_giveawayInfo()
  : flags_()
  , participating_()
  , preparing_results_()
  , start_date_()
  , joined_too_early_date_()
  , admin_disallowed_chat_id_()
  , disallowed_country_()
{}

const std::int32_t payments_giveawayInfo::ID;

object_ptr<payments_GiveawayInfo> payments_giveawayInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_giveawayInfo> res = make_tl_object<payments_giveawayInfo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->participating_ = (var0 & 1) != 0;
  res->preparing_results_ = (var0 & 8) != 0;
  res->start_date_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->joined_too_early_date_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->admin_disallowed_chat_id_ = TlFetchLong::parse(p); }
  if (var0 & 16) { res->disallowed_country_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void payments_giveawayInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.giveawayInfo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (participating_ << 0) | (preparing_results_ << 3)));
    if (var0 & 1) { s.store_field("participating", true); }
    if (var0 & 8) { s.store_field("preparing_results", true); }
    s.store_field("start_date", start_date_);
    if (var0 & 2) { s.store_field("joined_too_early_date", joined_too_early_date_); }
    if (var0 & 4) { s.store_field("admin_disallowed_chat_id", admin_disallowed_chat_id_); }
    if (var0 & 16) { s.store_field("disallowed_country", disallowed_country_); }
    s.store_class_end();
  }
}

payments_giveawayInfoResults::payments_giveawayInfoResults()
  : flags_()
  , winner_()
  , refunded_()
  , start_date_()
  , gift_code_slug_()
  , stars_prize_()
  , finish_date_()
  , winners_count_()
  , activated_count_()
{}

const std::int32_t payments_giveawayInfoResults::ID;

object_ptr<payments_GiveawayInfo> payments_giveawayInfoResults::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_giveawayInfoResults> res = make_tl_object<payments_giveawayInfoResults>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->winner_ = (var0 & 1) != 0;
  res->refunded_ = (var0 & 2) != 0;
  res->start_date_ = TlFetchInt::parse(p);
  if (var0 & 8) { res->gift_code_slug_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->stars_prize_ = TlFetchLong::parse(p); }
  res->finish_date_ = TlFetchInt::parse(p);
  res->winners_count_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->activated_count_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void payments_giveawayInfoResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.giveawayInfoResults");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (winner_ << 0) | (refunded_ << 1)));
    if (var0 & 1) { s.store_field("winner", true); }
    if (var0 & 2) { s.store_field("refunded", true); }
    s.store_field("start_date", start_date_);
    if (var0 & 8) { s.store_field("gift_code_slug", gift_code_slug_); }
    if (var0 & 16) { s.store_field("stars_prize", stars_prize_); }
    s.store_field("finish_date", finish_date_);
    s.store_field("winners_count", winners_count_);
    if (var0 & 4) { s.store_field("activated_count", activated_count_); }
    s.store_class_end();
  }
}

object_ptr<payments_StarGiftCollections> payments_StarGiftCollections::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case payments_starGiftCollectionsNotModified::ID:
      return payments_starGiftCollectionsNotModified::fetch(p);
    case payments_starGiftCollections::ID:
      return payments_starGiftCollections::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t payments_starGiftCollectionsNotModified::ID;

object_ptr<payments_StarGiftCollections> payments_starGiftCollectionsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starGiftCollectionsNotModified>();
}

void payments_starGiftCollectionsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starGiftCollectionsNotModified");
    s.store_class_end();
  }
}

const std::int32_t payments_starGiftCollections::ID;

object_ptr<payments_StarGiftCollections> payments_starGiftCollections::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starGiftCollections>(p);
}

payments_starGiftCollections::payments_starGiftCollections(TlBufferParser &p)
  : collections_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starGiftCollection>, -1653926992>>, 481674261>::parse(p))
{}

void payments_starGiftCollections::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starGiftCollections");
    { s.store_vector_begin("collections", collections_.size()); for (const auto &_value : collections_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<payments_StarGifts> payments_StarGifts::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case payments_starGiftsNotModified::ID:
      return payments_starGiftsNotModified::fetch(p);
    case payments_starGifts::ID:
      return payments_starGifts::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t payments_starGiftsNotModified::ID;

object_ptr<payments_StarGifts> payments_starGiftsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starGiftsNotModified>();
}

void payments_starGiftsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starGiftsNotModified");
    s.store_class_end();
  }
}

const std::int32_t payments_starGifts::ID;

object_ptr<payments_StarGifts> payments_starGifts::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starGifts>(p);
}

payments_starGifts::payments_starGifts(TlBufferParser &p)
  : hash_(TlFetchInt::parse(p))
  , gifts_(TlFetchBoxed<TlFetchVector<TlFetchObject<StarGift>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void payments_starGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starGifts");
    s.store_field("hash", hash_);
    { s.store_vector_begin("gifts", gifts_.size()); for (const auto &_value : gifts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t payments_uniqueStarGift::ID;

object_ptr<payments_uniqueStarGift> payments_uniqueStarGift::fetch(TlBufferParser &p) {
  return make_tl_object<payments_uniqueStarGift>(p);
}

payments_uniqueStarGift::payments_uniqueStarGift(TlBufferParser &p)
  : gift_(TlFetchObject<StarGift>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void payments_uniqueStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.uniqueStarGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_storyReactionsList::stories_storyReactionsList()
  : flags_()
  , count_()
  , reactions_()
  , chats_()
  , users_()
  , next_offset_()
{}

const std::int32_t stories_storyReactionsList::ID;

object_ptr<stories_storyReactionsList> stories_storyReactionsList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stories_storyReactionsList> res = make_tl_object<stories_storyReactionsList>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->reactions_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StoryReaction>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void stories_storyReactionsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.storyReactionsList");
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

const std::int32_t account_getBusinessChatLinks::ID;

void account_getBusinessChatLinks::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1869667809);
}

void account_getBusinessChatLinks::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1869667809);
}

void account_getBusinessChatLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getBusinessChatLinks");
    s.store_class_end();
  }
}

account_getBusinessChatLinks::ReturnType account_getBusinessChatLinks::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_businessChatLinks>, -331111727>::parse(p);
#undef FAIL
}

const std::int32_t account_getConnectedBots::ID;

void account_getConnectedBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1319421967);
}

void account_getConnectedBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1319421967);
}

void account_getConnectedBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getConnectedBots");
    s.store_class_end();
  }
}

account_getConnectedBots::ReturnType account_getConnectedBots::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_connectedBots>, 400029819>::parse(p);
#undef FAIL
}

account_getDefaultEmojiStatuses::account_getDefaultEmojiStatuses(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getDefaultEmojiStatuses::ID;

void account_getDefaultEmojiStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-696962170);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultEmojiStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-696962170);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getDefaultEmojiStatuses");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getDefaultEmojiStatuses::ReturnType account_getDefaultEmojiStatuses::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_EmojiStatuses>::parse(p);
#undef FAIL
}

account_getRecentEmojiStatuses::account_getRecentEmojiStatuses(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getRecentEmojiStatuses::ID;

void account_getRecentEmojiStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(257392901);
  TlStoreBinary::store(hash_, s);
}

void account_getRecentEmojiStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(257392901);
  TlStoreBinary::store(hash_, s);
}

void account_getRecentEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getRecentEmojiStatuses");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getRecentEmojiStatuses::ReturnType account_getRecentEmojiStatuses::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_EmojiStatuses>::parse(p);
#undef FAIL
}

account_getUniqueGiftChatThemes::account_getUniqueGiftChatThemes(string const &offset_, int32 limit_, int64 hash_)
  : offset_(offset_)
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t account_getUniqueGiftChatThemes::ID;

void account_getUniqueGiftChatThemes::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-466818615);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void account_getUniqueGiftChatThemes::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-466818615);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void account_getUniqueGiftChatThemes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getUniqueGiftChatThemes");
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getUniqueGiftChatThemes::ReturnType account_getUniqueGiftChatThemes::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_ChatThemes>::parse(p);
#undef FAIL
}

account_setMainProfileTab::account_setMainProfileTab(object_ptr<ProfileTab> &&tab_)
  : tab_(std::move(tab_))
{}

const std::int32_t account_setMainProfileTab::ID;

void account_setMainProfileTab::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1575909552);
  TlStoreBoxedUnknown<TlStoreObject>::store(tab_, s);
}

void account_setMainProfileTab::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1575909552);
  TlStoreBoxedUnknown<TlStoreObject>::store(tab_, s);
}

void account_setMainProfileTab::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setMainProfileTab");
    s.store_object_field("tab", static_cast<const BaseObject *>(tab_.get()));
    s.store_class_end();
  }
}

account_setMainProfileTab::ReturnType account_setMainProfileTab::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_uploadWallPaper::account_uploadWallPaper(int32 flags_, bool for_chat_, object_ptr<InputFile> &&file_, string const &mime_type_, object_ptr<wallPaperSettings> &&settings_)
  : flags_(flags_)
  , for_chat_(for_chat_)
  , file_(std::move(file_))
  , mime_type_(mime_type_)
  , settings_(std::move(settings_))
{}

const std::int32_t account_uploadWallPaper::ID;

void account_uploadWallPaper::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-476410109);
  TlStoreBinary::store((var0 = flags_ | (for_chat_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s);
}

void account_uploadWallPaper::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-476410109);
  TlStoreBinary::store((var0 = flags_ | (for_chat_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s);
}

void account_uploadWallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.uploadWallPaper");
    s.store_field("flags", (var0 = flags_ | (for_chat_ << 0)));
    if (var0 & 1) { s.store_field("for_chat", true); }
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_field("mime_type", mime_type_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_uploadWallPaper::ReturnType account_uploadWallPaper::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<WallPaper>::parse(p);
#undef FAIL
}

account_verifyPhone::account_verifyPhone(string const &phone_number_, string const &phone_code_hash_, string const &phone_code_)
  : phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , phone_code_(phone_code_)
{}

const std::int32_t account_verifyPhone::ID;

void account_verifyPhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1305716726);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(phone_code_, s);
}

void account_verifyPhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1305716726);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(phone_code_, s);
}

void account_verifyPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.verifyPhone");
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("phone_code", phone_code_);
    s.store_class_end();
  }
}

account_verifyPhone::ReturnType account_verifyPhone::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_checkPaidAuth::auth_checkPaidAuth(string const &phone_number_, string const &phone_code_hash_, int64 form_id_)
  : phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , form_id_(form_id_)
{}

const std::int32_t auth_checkPaidAuth::ID;

void auth_checkPaidAuth::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1457889180);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreBinary::store(form_id_, s);
}

void auth_checkPaidAuth::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1457889180);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreBinary::store(form_id_, s);
}

void auth_checkPaidAuth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.checkPaidAuth");
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("form_id", form_id_);
    s.store_class_end();
  }
}

auth_checkPaidAuth::ReturnType auth_checkPaidAuth::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_SentCode>::parse(p);
#undef FAIL
}

auth_importAuthorization::auth_importAuthorization(int64 id_, bytes &&bytes_)
  : id_(id_)
  , bytes_(std::move(bytes_))
{}

const std::int32_t auth_importAuthorization::ID;

void auth_importAuthorization::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1518699091);
  TlStoreBinary::store(id_, s);
  TlStoreString::store(bytes_, s);
}

void auth_importAuthorization::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1518699091);
  TlStoreBinary::store(id_, s);
  TlStoreString::store(bytes_, s);
}

void auth_importAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.importAuthorization");
    s.store_field("id", id_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

auth_importAuthorization::ReturnType auth_importAuthorization::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_Authorization>::parse(p);
#undef FAIL
}

const std::int32_t auth_logOut::ID;

void auth_logOut::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1047706137);
}

void auth_logOut::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1047706137);
}

void auth_logOut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.logOut");
    s.store_class_end();
  }
}

auth_logOut::ReturnType auth_logOut::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<auth_loggedOut>, -1012759713>::parse(p);
#undef FAIL
}

auth_resetLoginEmail::auth_resetLoginEmail(string const &phone_number_, string const &phone_code_hash_)
  : phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
{}

const std::int32_t auth_resetLoginEmail::ID;

void auth_resetLoginEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2123760019);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
}

void auth_resetLoginEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2123760019);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
}

void auth_resetLoginEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.resetLoginEmail");
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_class_end();
  }
}

auth_resetLoginEmail::ReturnType auth_resetLoginEmail::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_SentCode>::parse(p);
#undef FAIL
}

const std::int32_t bots_getAdminedBots::ID;

void bots_getAdminedBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1334764157);
}

void bots_getAdminedBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1334764157);
}

void bots_getAdminedBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getAdminedBots");
    s.store_class_end();
  }
}

bots_getAdminedBots::ReturnType bots_getAdminedBots::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
#undef FAIL
}

bots_getBotMenuButton::bots_getBotMenuButton(object_ptr<InputUser> &&user_id_)
  : user_id_(std::move(user_id_))
{}

const std::int32_t bots_getBotMenuButton::ID;

void bots_getBotMenuButton::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1671369944);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void bots_getBotMenuButton::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1671369944);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void bots_getBotMenuButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getBotMenuButton");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

bots_getBotMenuButton::ReturnType bots_getBotMenuButton::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<BotMenuButton>::parse(p);
#undef FAIL
}

channels_editCreator::channels_editCreator(object_ptr<InputChannel> &&channel_, object_ptr<InputUser> &&user_id_, object_ptr<InputCheckPasswordSRP> &&password_)
  : channel_(std::move(channel_))
  , user_id_(std::move(user_id_))
  , password_(std::move(password_))
{}

const std::int32_t channels_editCreator::ID;

void channels_editCreator::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1892102881);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void channels_editCreator::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1892102881);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void channels_editCreator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.editCreator");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_object_field("password", static_cast<const BaseObject *>(password_.get()));
    s.store_class_end();
  }
}

channels_editCreator::ReturnType channels_editCreator::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_editPhoto::channels_editPhoto(object_ptr<InputChannel> &&channel_, object_ptr<InputChatPhoto> &&photo_)
  : channel_(std::move(channel_))
  , photo_(std::move(photo_))
{}

const std::int32_t channels_editPhoto::ID;

void channels_editPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-248621111);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_, s);
}

void channels_editPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-248621111);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(photo_, s);
}

void channels_editPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.editPhoto");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

channels_editPhoto::ReturnType channels_editPhoto::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_getFullChannel::channels_getFullChannel(object_ptr<InputChannel> &&channel_)
  : channel_(std::move(channel_))
{}

const std::int32_t channels_getFullChannel::ID;

void channels_getFullChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(141781513);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_getFullChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(141781513);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_getFullChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getFullChannel");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

channels_getFullChannel::ReturnType channels_getFullChannel::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_chatFull>, -438840932>::parse(p);
#undef FAIL
}

channels_getMessageAuthor::channels_getMessageAuthor(object_ptr<InputChannel> &&channel_, int32 id_)
  : channel_(std::move(channel_))
  , id_(id_)
{}

const std::int32_t channels_getMessageAuthor::ID;

void channels_getMessageAuthor::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-320691994);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(id_, s);
}

void channels_getMessageAuthor::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-320691994);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(id_, s);
}

void channels_getMessageAuthor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getMessageAuthor");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

channels_getMessageAuthor::ReturnType channels_getMessageAuthor::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<User>::parse(p);
#undef FAIL
}

channels_toggleUsername::channels_toggleUsername(object_ptr<InputChannel> &&channel_, string const &username_, bool active_)
  : channel_(std::move(channel_))
  , username_(username_)
  , active_(active_)
{}

const std::int32_t channels_toggleUsername::ID;

void channels_toggleUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1358053637);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(username_, s);
  TlStoreBool::store(active_, s);
}

void channels_toggleUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1358053637);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(username_, s);
  TlStoreBool::store(active_, s);
}

void channels_toggleUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleUsername");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("username", username_);
    s.store_field("active", active_);
    s.store_class_end();
  }
}

channels_toggleUsername::ReturnType channels_toggleUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_toggleViewForumAsMessages::channels_toggleViewForumAsMessages(object_ptr<InputChannel> &&channel_, bool enabled_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
{}

const std::int32_t channels_toggleViewForumAsMessages::ID;

void channels_toggleViewForumAsMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1757889771);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleViewForumAsMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1757889771);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleViewForumAsMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleViewForumAsMessages");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

channels_toggleViewForumAsMessages::ReturnType channels_toggleViewForumAsMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

chatlists_checkChatlistInvite::chatlists_checkChatlistInvite(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t chatlists_checkChatlistInvite::ID;

void chatlists_checkChatlistInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1103171583);
  TlStoreString::store(slug_, s);
}

void chatlists_checkChatlistInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1103171583);
  TlStoreString::store(slug_, s);
}

void chatlists_checkChatlistInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.checkChatlistInvite");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

chatlists_checkChatlistInvite::ReturnType chatlists_checkChatlistInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<chatlists_ChatlistInvite>::parse(p);
#undef FAIL
}

chatlists_deleteExportedInvite::chatlists_deleteExportedInvite(object_ptr<inputChatlistDialogFilter> &&chatlist_, string const &slug_)
  : chatlist_(std::move(chatlist_))
  , slug_(slug_)
{}

const std::int32_t chatlists_deleteExportedInvite::ID;

void chatlists_deleteExportedInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1906072670);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreString::store(slug_, s);
}

void chatlists_deleteExportedInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1906072670);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreString::store(slug_, s);
}

void chatlists_deleteExportedInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.deleteExportedInvite");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

chatlists_deleteExportedInvite::ReturnType chatlists_deleteExportedInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

chatlists_leaveChatlist::chatlists_leaveChatlist(object_ptr<inputChatlistDialogFilter> &&chatlist_, array<object_ptr<InputPeer>> &&peers_)
  : chatlist_(std::move(chatlist_))
  , peers_(std::move(peers_))
{}

const std::int32_t chatlists_leaveChatlist::ID;

void chatlists_leaveChatlist::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1962598714);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_leaveChatlist::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1962598714);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_leaveChatlist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.leaveChatlist");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatlists_leaveChatlist::ReturnType chatlists_leaveChatlist::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_updateContactNote::contacts_updateContactNote(object_ptr<InputUser> &&id_, object_ptr<textWithEntities> &&note_)
  : id_(std::move(id_))
  , note_(std::move(note_))
{}

const std::int32_t contacts_updateContactNote::ID;

void contacts_updateContactNote::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(329212923);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(note_, s);
}

void contacts_updateContactNote::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(329212923);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(note_, s);
}

void contacts_updateContactNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.updateContactNote");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_object_field("note", static_cast<const BaseObject *>(note_.get()));
    s.store_class_end();
  }
}

contacts_updateContactNote::ReturnType contacts_updateContactNote::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

help_acceptTermsOfService::help_acceptTermsOfService(object_ptr<dataJSON> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t help_acceptTermsOfService::ID;

void help_acceptTermsOfService::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-294455398);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(id_, s);
}

void help_acceptTermsOfService::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-294455398);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(id_, s);
}

void help_acceptTermsOfService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.acceptTermsOfService");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

help_acceptTermsOfService::ReturnType help_acceptTermsOfService::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

help_getAppConfig::help_getAppConfig(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t help_getAppConfig::ID;

void help_getAppConfig::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1642330196);
  TlStoreBinary::store(hash_, s);
}

void help_getAppConfig::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1642330196);
  TlStoreBinary::store(hash_, s);
}

void help_getAppConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getAppConfig");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

help_getAppConfig::ReturnType help_getAppConfig::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_AppConfig>::parse(p);
#undef FAIL
}

const std::int32_t help_getSupport::ID;

void help_getSupport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1663104819);
}

void help_getSupport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1663104819);
}

void help_getSupport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getSupport");
    s.store_class_end();
  }
}

help_getSupport::ReturnType help_getSupport::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<help_support>, 398898678>::parse(p);
#undef FAIL
}

help_hidePromoData::help_hidePromoData(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t help_hidePromoData::ID;

void help_hidePromoData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(505748629);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void help_hidePromoData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(505748629);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void help_hidePromoData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.hidePromoData");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

help_hidePromoData::ReturnType help_hidePromoData::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

langpack_getLangPack::langpack_getLangPack(string const &lang_pack_, string const &lang_code_)
  : lang_pack_(lang_pack_)
  , lang_code_(lang_code_)
{}

const std::int32_t langpack_getLangPack::ID;

void langpack_getLangPack::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-219008246);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
}

void langpack_getLangPack::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-219008246);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
}

void langpack_getLangPack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langpack.getLangPack");
    s.store_field("lang_pack", lang_pack_);
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

langpack_getLangPack::ReturnType langpack_getLangPack::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<langPackDifference>, -209337866>::parse(p);
#undef FAIL
}

messages_appendTodoList::messages_appendTodoList(object_ptr<InputPeer> &&peer_, int32 msg_id_, array<object_ptr<todoItem>> &&list_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , list_(std::move(list_))
{}

const std::int32_t messages_appendTodoList::ID;

void messages_appendTodoList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(564531287);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -878074577>>, 481674261>::store(list_, s);
}

void messages_appendTodoList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(564531287);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -878074577>>, 481674261>::store(list_, s);
}

void messages_appendTodoList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.appendTodoList");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    { s.store_vector_begin("list", list_.size()); for (const auto &_value : list_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_appendTodoList::ReturnType messages_appendTodoList::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_createForumTopic::messages_createForumTopic(int32 flags_, bool title_missing_, object_ptr<InputPeer> &&peer_, string const &title_, int32 icon_color_, int64 icon_emoji_id_, int64 random_id_, object_ptr<InputPeer> &&send_as_)
  : flags_(flags_)
  , title_missing_(title_missing_)
  , peer_(std::move(peer_))
  , title_(title_)
  , icon_color_(icon_color_)
  , icon_emoji_id_(icon_emoji_id_)
  , random_id_(random_id_)
  , send_as_(std::move(send_as_))
{}

const std::int32_t messages_createForumTopic::ID;

void messages_createForumTopic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(798540757);
  TlStoreBinary::store((var0 = flags_ | (title_missing_ << 4)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(title_, s);
  if (var0 & 1) { TlStoreBinary::store(icon_color_, s); }
  if (var0 & 8) { TlStoreBinary::store(icon_emoji_id_, s); }
  TlStoreBinary::store(random_id_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
}

void messages_createForumTopic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(798540757);
  TlStoreBinary::store((var0 = flags_ | (title_missing_ << 4)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(title_, s);
  if (var0 & 1) { TlStoreBinary::store(icon_color_, s); }
  if (var0 & 8) { TlStoreBinary::store(icon_emoji_id_, s); }
  TlStoreBinary::store(random_id_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
}

void messages_createForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.createForumTopic");
    s.store_field("flags", (var0 = flags_ | (title_missing_ << 4)));
    if (var0 & 16) { s.store_field("title_missing", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("title", title_);
    if (var0 & 1) { s.store_field("icon_color", icon_color_); }
    if (var0 & 8) { s.store_field("icon_emoji_id", icon_emoji_id_); }
    s.store_field("random_id", random_id_);
    if (var0 & 4) { s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get())); }
    s.store_class_end();
  }
}

messages_createForumTopic::ReturnType messages_createForumTopic::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_deleteQuickReplyShortcut::messages_deleteQuickReplyShortcut(int32 shortcut_id_)
  : shortcut_id_(shortcut_id_)
{}

const std::int32_t messages_deleteQuickReplyShortcut::ID;

void messages_deleteQuickReplyShortcut::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1019234112);
  TlStoreBinary::store(shortcut_id_, s);
}

void messages_deleteQuickReplyShortcut::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1019234112);
  TlStoreBinary::store(shortcut_id_, s);
}

void messages_deleteQuickReplyShortcut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteQuickReplyShortcut");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_class_end();
  }
}

messages_deleteQuickReplyShortcut::ReturnType messages_deleteQuickReplyShortcut::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_deleteRevokedExportedChatInvites::messages_deleteRevokedExportedChatInvites(object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&admin_id_)
  : peer_(std::move(peer_))
  , admin_id_(std::move(admin_id_))
{}

const std::int32_t messages_deleteRevokedExportedChatInvites::ID;

void messages_deleteRevokedExportedChatInvites::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1452833749);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(admin_id_, s);
}

void messages_deleteRevokedExportedChatInvites::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1452833749);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(admin_id_, s);
}

void messages_deleteRevokedExportedChatInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteRevokedExportedChatInvites");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("admin_id", static_cast<const BaseObject *>(admin_id_.get()));
    s.store_class_end();
  }
}

messages_deleteRevokedExportedChatInvites::ReturnType messages_deleteRevokedExportedChatInvites::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_deleteScheduledMessages::messages_deleteScheduledMessages(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t messages_deleteScheduledMessages::ID;

void messages_deleteScheduledMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1504586518);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_deleteScheduledMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1504586518);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_deleteScheduledMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteScheduledMessages");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_deleteScheduledMessages::ReturnType messages_deleteScheduledMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_editChatTitle::messages_editChatTitle(int64 chat_id_, string const &title_)
  : chat_id_(chat_id_)
  , title_(title_)
{}

const std::int32_t messages_editChatTitle::ID;

void messages_editChatTitle::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1937260541);
  TlStoreBinary::store(chat_id_, s);
  TlStoreString::store(title_, s);
}

void messages_editChatTitle::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1937260541);
  TlStoreBinary::store(chat_id_, s);
  TlStoreString::store(title_, s);
}

void messages_editChatTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editChatTitle");
    s.store_field("chat_id", chat_id_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

messages_editChatTitle::ReturnType messages_editChatTitle::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getAdminsWithInvites::messages_getAdminsWithInvites(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t messages_getAdminsWithInvites::ID;

void messages_getAdminsWithInvites::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(958457583);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_getAdminsWithInvites::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(958457583);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_getAdminsWithInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAdminsWithInvites");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_getAdminsWithInvites::ReturnType messages_getAdminsWithInvites::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_chatAdminsWithInvites>, -1231326505>::parse(p);
#undef FAIL
}

messages_getBotCallbackAnswer::messages_getBotCallbackAnswer(int32 flags_, bool game_, object_ptr<InputPeer> &&peer_, int32 msg_id_, bytes &&data_, object_ptr<InputCheckPasswordSRP> &&password_)
  : flags_(flags_)
  , game_(game_)
  , peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , data_(std::move(data_))
  , password_(std::move(password_))
{}

const std::int32_t messages_getBotCallbackAnswer::ID;

void messages_getBotCallbackAnswer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1824339449);
  TlStoreBinary::store((var0 = flags_ | (game_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  if (var0 & 1) { TlStoreString::store(data_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(password_, s); }
}

void messages_getBotCallbackAnswer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1824339449);
  TlStoreBinary::store((var0 = flags_ | (game_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  if (var0 & 1) { TlStoreString::store(data_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(password_, s); }
}

void messages_getBotCallbackAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getBotCallbackAnswer");
    s.store_field("flags", (var0 = flags_ | (game_ << 1)));
    if (var0 & 2) { s.store_field("game", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    if (var0 & 1) { s.store_bytes_field("data", data_); }
    if (var0 & 4) { s.store_object_field("password", static_cast<const BaseObject *>(password_.get())); }
    s.store_class_end();
  }
}

messages_getBotCallbackAnswer::ReturnType messages_getBotCallbackAnswer::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_botCallbackAnswer>, 911761060>::parse(p);
#undef FAIL
}

messages_getChatInviteImporters::messages_getChatInviteImporters(int32 flags_, bool requested_, bool subscription_expired_, object_ptr<InputPeer> &&peer_, string const &link_, string const &q_, int32 offset_date_, object_ptr<InputUser> &&offset_user_, int32 limit_)
  : flags_(flags_)
  , requested_(requested_)
  , subscription_expired_(subscription_expired_)
  , peer_(std::move(peer_))
  , link_(link_)
  , q_(q_)
  , offset_date_(offset_date_)
  , offset_user_(std::move(offset_user_))
  , limit_(limit_)
{}

const std::int32_t messages_getChatInviteImporters::ID;

void messages_getChatInviteImporters::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-553329330);
  TlStoreBinary::store((var0 = flags_ | (requested_ << 0) | (subscription_expired_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreString::store(link_, s); }
  if (var0 & 4) { TlStoreString::store(q_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_user_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getChatInviteImporters::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-553329330);
  TlStoreBinary::store((var0 = flags_ | (requested_ << 0) | (subscription_expired_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreString::store(link_, s); }
  if (var0 & 4) { TlStoreString::store(q_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_user_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getChatInviteImporters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getChatInviteImporters");
    s.store_field("flags", (var0 = flags_ | (requested_ << 0) | (subscription_expired_ << 3)));
    if (var0 & 1) { s.store_field("requested", true); }
    if (var0 & 8) { s.store_field("subscription_expired", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 2) { s.store_field("link", link_); }
    if (var0 & 4) { s.store_field("q", q_); }
    s.store_field("offset_date", offset_date_);
    s.store_object_field("offset_user", static_cast<const BaseObject *>(offset_user_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getChatInviteImporters::ReturnType messages_getChatInviteImporters::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_chatInviteImporters>, -2118733814>::parse(p);
#undef FAIL
}

messages_getCommonChats::messages_getCommonChats(object_ptr<InputUser> &&user_id_, int64 max_id_, int32 limit_)
  : user_id_(std::move(user_id_))
  , max_id_(max_id_)
  , limit_(limit_)
{}

const std::int32_t messages_getCommonChats::ID;

void messages_getCommonChats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-468934396);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getCommonChats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-468934396);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getCommonChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getCommonChats");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("max_id", max_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getCommonChats::ReturnType messages_getCommonChats::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

messages_getEmojiStickers::messages_getEmojiStickers(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getEmojiStickers::ID;

void messages_getEmojiStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-67329649);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-67329649);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiStickers");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getEmojiStickers::ReturnType messages_getEmojiStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_AllStickers>::parse(p);
#undef FAIL
}

messages_getFeaturedEmojiStickers::messages_getFeaturedEmojiStickers(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getFeaturedEmojiStickers::ID;

void messages_getFeaturedEmojiStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(248473398);
  TlStoreBinary::store(hash_, s);
}

void messages_getFeaturedEmojiStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(248473398);
  TlStoreBinary::store(hash_, s);
}

void messages_getFeaturedEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getFeaturedEmojiStickers");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getFeaturedEmojiStickers::ReturnType messages_getFeaturedEmojiStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_FeaturedStickers>::parse(p);
#undef FAIL
}

messages_getMessagesReactions::messages_getMessagesReactions(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t messages_getMessagesReactions::ID;

void messages_getMessagesReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1950707482);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getMessagesReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1950707482);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getMessagesReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMessagesReactions");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getMessagesReactions::ReturnType messages_getMessagesReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getMyStickers::messages_getMyStickers(int64 offset_id_, int32 limit_)
  : offset_id_(offset_id_)
  , limit_(limit_)
{}

const std::int32_t messages_getMyStickers::ID;

void messages_getMyStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-793386500);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getMyStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-793386500);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getMyStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMyStickers");
    s.store_field("offset_id", offset_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getMyStickers::ReturnType messages_getMyStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_myStickers>, -83926371>::parse(p);
#undef FAIL
}

messages_readReactions::messages_readReactions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, object_ptr<InputPeer> &&saved_peer_id_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
  , saved_peer_id_(std::move(saved_peer_id_))
{}

const std::int32_t messages_readReactions::ID;

void messages_readReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1631301741);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
}

void messages_readReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1631301741);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
}

void messages_readReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readReactions");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 2) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_class_end();
  }
}

messages_readReactions::ReturnType messages_readReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedHistory>, -1269012015>::parse(p);
#undef FAIL
}

messages_receivedQueue::messages_receivedQueue(int32 max_qts_)
  : max_qts_(max_qts_)
{}

const std::int32_t messages_receivedQueue::ID;

void messages_receivedQueue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1436924774);
  TlStoreBinary::store(max_qts_, s);
}

void messages_receivedQueue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1436924774);
  TlStoreBinary::store(max_qts_, s);
}

void messages_receivedQueue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.receivedQueue");
    s.store_field("max_qts", max_qts_);
    s.store_class_end();
  }
}

messages_receivedQueue::ReturnType messages_receivedQueue::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p);
#undef FAIL
}

messages_reorderQuickReplies::messages_reorderQuickReplies(array<int32> &&order_)
  : order_(std::move(order_))
{}

const std::int32_t messages_reorderQuickReplies::ID;

void messages_reorderQuickReplies::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1613961479);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_reorderQuickReplies::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1613961479);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_reorderQuickReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reorderQuickReplies");
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_reorderQuickReplies::ReturnType messages_reorderQuickReplies::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_report::messages_report(object_ptr<InputPeer> &&peer_, array<int32> &&id_, bytes &&option_, string const &message_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
  , option_(std::move(option_))
  , message_(message_)
{}

const std::int32_t messages_report::ID;

void messages_report::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-59199589);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreString::store(option_, s);
  TlStoreString::store(message_, s);
}

void messages_report::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-59199589);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreString::store(option_, s);
  TlStoreString::store(message_, s);
}

void messages_report::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.report");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_bytes_field("option", option_);
    s.store_field("message", message_);
    s.store_class_end();
  }
}

messages_report::ReturnType messages_report::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<ReportResult>::parse(p);
#undef FAIL
}

messages_reportMessagesDelivery::messages_reportMessagesDelivery(int32 flags_, bool push_, object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : flags_(flags_)
  , push_(push_)
  , peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t messages_reportMessagesDelivery::ID;

void messages_reportMessagesDelivery::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1517122453);
  TlStoreBinary::store((var0 = flags_ | (push_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_reportMessagesDelivery::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1517122453);
  TlStoreBinary::store((var0 = flags_ | (push_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_reportMessagesDelivery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reportMessagesDelivery");
    s.store_field("flags", (var0 = flags_ | (push_ << 0)));
    if (var0 & 1) { s.store_field("push", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_reportMessagesDelivery::ReturnType messages_reportMessagesDelivery::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_saveDefaultSendAs::messages_saveDefaultSendAs(object_ptr<InputPeer> &&peer_, object_ptr<InputPeer> &&send_as_)
  : peer_(std::move(peer_))
  , send_as_(std::move(send_as_))
{}

const std::int32_t messages_saveDefaultSendAs::ID;

void messages_saveDefaultSendAs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-855777386);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s);
}

void messages_saveDefaultSendAs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-855777386);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s);
}

void messages_saveDefaultSendAs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.saveDefaultSendAs");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get()));
    s.store_class_end();
  }
}

messages_saveDefaultSendAs::ReturnType messages_saveDefaultSendAs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_saveDraft::messages_saveDraft(int32 flags_, bool no_webpage_, bool invert_media_, object_ptr<InputReplyTo> &&reply_to_, object_ptr<InputPeer> &&peer_, string const &message_, array<object_ptr<MessageEntity>> &&entities_, object_ptr<InputMedia> &&media_, int64 effect_, object_ptr<suggestedPost> &&suggested_post_)
  : flags_(flags_)
  , no_webpage_(no_webpage_)
  , invert_media_(invert_media_)
  , reply_to_(std::move(reply_to_))
  , peer_(std::move(peer_))
  , message_(message_)
  , entities_(std::move(entities_))
  , media_(std::move(media_))
  , effect_(effect_)
  , suggested_post_(std::move(suggested_post_))
{}

const std::int32_t messages_saveDraft::ID;

void messages_saveDraft::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1420701838);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 6)), s);
  if (var0 & 16) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(message_, s);
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 128) { TlStoreBinary::store(effect_, s); }
  if (var0 & 256) { TlStoreBoxed<TlStoreObject, 244201445>::store(suggested_post_, s); }
}

void messages_saveDraft::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1420701838);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 6)), s);
  if (var0 & 16) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(message_, s);
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 128) { TlStoreBinary::store(effect_, s); }
  if (var0 & 256) { TlStoreBoxed<TlStoreObject, 244201445>::store(suggested_post_, s); }
}

void messages_saveDraft::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.saveDraft");
    s.store_field("flags", (var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 6)));
    if (var0 & 2) { s.store_field("no_webpage", true); }
    if (var0 & 64) { s.store_field("invert_media", true); }
    if (var0 & 16) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("message", message_);
    if (var0 & 8) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 32) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 128) { s.store_field("effect", effect_); }
    if (var0 & 256) { s.store_object_field("suggested_post", static_cast<const BaseObject *>(suggested_post_.get())); }
    s.store_class_end();
  }
}

messages_saveDraft::ReturnType messages_saveDraft::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_search::messages_search(int32 flags_, object_ptr<InputPeer> &&peer_, string const &q_, object_ptr<InputPeer> &&from_id_, object_ptr<InputPeer> &&saved_peer_id_, array<object_ptr<Reaction>> &&saved_reaction_, int32 top_msg_id_, object_ptr<MessagesFilter> &&filter_, int32 min_date_, int32 max_date_, int32 offset_id_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_, int64 hash_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , q_(q_)
  , from_id_(std::move(from_id_))
  , saved_peer_id_(std::move(saved_peer_id_))
  , saved_reaction_(std::move(saved_reaction_))
  , top_msg_id_(top_msg_id_)
  , filter_(std::move(filter_))
  , min_date_(min_date_)
  , max_date_(max_date_)
  , offset_id_(offset_id_)
  , add_offset_(add_offset_)
  , limit_(limit_)
  , max_id_(max_id_)
  , min_id_(min_id_)
  , hash_(hash_)
{}

const std::int32_t messages_search::ID;

void messages_search::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(703497338);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(q_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(from_id_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(saved_reaction_, s); }
  if (var0 & 2) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(min_date_, s);
  TlStoreBinary::store(max_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_search::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(703497338);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(q_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(from_id_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(saved_reaction_, s); }
  if (var0 & 2) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(min_date_, s);
  TlStoreBinary::store(max_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_search::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.search");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("q", q_);
    if (var0 & 1) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    if (var0 & 4) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    if (var0 & 8) { { s.store_vector_begin("saved_reaction", saved_reaction_.size()); for (const auto &_value : saved_reaction_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("min_date", min_date_);
    s.store_field("max_date", max_date_);
    s.store_field("offset_id", offset_id_);
    s.store_field("add_offset", add_offset_);
    s.store_field("limit", limit_);
    s.store_field("max_id", max_id_);
    s.store_field("min_id", min_id_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_search::ReturnType messages_search::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_setInlineBotResults::messages_setInlineBotResults(int32 flags_, bool gallery_, bool private_, int64 query_id_, array<object_ptr<InputBotInlineResult>> &&results_, int32 cache_time_, string const &next_offset_, object_ptr<inlineBotSwitchPM> &&switch_pm_, object_ptr<inlineBotWebView> &&switch_webview_)
  : flags_(flags_)
  , gallery_(gallery_)
  , private_(private_)
  , query_id_(query_id_)
  , results_(std::move(results_))
  , cache_time_(cache_time_)
  , next_offset_(next_offset_)
  , switch_pm_(std::move(switch_pm_))
  , switch_webview_(std::move(switch_webview_))
{}

const std::int32_t messages_setInlineBotResults::ID;

void messages_setInlineBotResults::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1156406247);
  TlStoreBinary::store((var0 = flags_ | (gallery_ << 0) | (private_ << 1)), s);
  TlStoreBinary::store(query_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(results_, s);
  TlStoreBinary::store(cache_time_, s);
  if (var0 & 4) { TlStoreString::store(next_offset_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreObject, 1008755359>::store(switch_pm_, s); }
  if (var0 & 16) { TlStoreBoxed<TlStoreObject, -1250781739>::store(switch_webview_, s); }
}

void messages_setInlineBotResults::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1156406247);
  TlStoreBinary::store((var0 = flags_ | (gallery_ << 0) | (private_ << 1)), s);
  TlStoreBinary::store(query_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(results_, s);
  TlStoreBinary::store(cache_time_, s);
  if (var0 & 4) { TlStoreString::store(next_offset_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreObject, 1008755359>::store(switch_pm_, s); }
  if (var0 & 16) { TlStoreBoxed<TlStoreObject, -1250781739>::store(switch_webview_, s); }
}

void messages_setInlineBotResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setInlineBotResults");
    s.store_field("flags", (var0 = flags_ | (gallery_ << 0) | (private_ << 1)));
    if (var0 & 1) { s.store_field("gallery", true); }
    if (var0 & 2) { s.store_field("private", true); }
    s.store_field("query_id", query_id_);
    { s.store_vector_begin("results", results_.size()); for (const auto &_value : results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("cache_time", cache_time_);
    if (var0 & 4) { s.store_field("next_offset", next_offset_); }
    if (var0 & 8) { s.store_object_field("switch_pm", static_cast<const BaseObject *>(switch_pm_.get())); }
    if (var0 & 16) { s.store_object_field("switch_webview", static_cast<const BaseObject *>(switch_webview_.get())); }
    s.store_class_end();
  }
}

messages_setInlineBotResults::ReturnType messages_setInlineBotResults::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_toggleBotInAttachMenu::messages_toggleBotInAttachMenu(int32 flags_, bool write_allowed_, object_ptr<InputUser> &&bot_, bool enabled_)
  : flags_(flags_)
  , write_allowed_(write_allowed_)
  , bot_(std::move(bot_))
  , enabled_(enabled_)
{}

const std::int32_t messages_toggleBotInAttachMenu::ID;

void messages_toggleBotInAttachMenu::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1777704297);
  TlStoreBinary::store((var0 = flags_ | (write_allowed_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBool::store(enabled_, s);
}

void messages_toggleBotInAttachMenu::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1777704297);
  TlStoreBinary::store((var0 = flags_ | (write_allowed_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBool::store(enabled_, s);
}

void messages_toggleBotInAttachMenu::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleBotInAttachMenu");
    s.store_field("flags", (var0 = flags_ | (write_allowed_ << 0)));
    if (var0 & 1) { s.store_field("write_allowed", true); }
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

messages_toggleBotInAttachMenu::ReturnType messages_toggleBotInAttachMenu::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_togglePeerTranslations::messages_togglePeerTranslations(int32 flags_, bool disabled_, object_ptr<InputPeer> &&peer_)
  : flags_(flags_)
  , disabled_(disabled_)
  , peer_(std::move(peer_))
{}

const std::int32_t messages_togglePeerTranslations::ID;

void messages_togglePeerTranslations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-461589127);
  TlStoreBinary::store((var0 = flags_ | (disabled_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_togglePeerTranslations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-461589127);
  TlStoreBinary::store((var0 = flags_ | (disabled_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_togglePeerTranslations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.togglePeerTranslations");
    s.store_field("flags", (var0 = flags_ | (disabled_ << 0)));
    if (var0 & 1) { s.store_field("disabled", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_togglePeerTranslations::ReturnType messages_togglePeerTranslations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_toggleStickerSets::messages_toggleStickerSets(int32 flags_, bool uninstall_, bool archive_, bool unarchive_, array<object_ptr<InputStickerSet>> &&stickersets_)
  : flags_(flags_)
  , uninstall_(uninstall_)
  , archive_(archive_)
  , unarchive_(unarchive_)
  , stickersets_(std::move(stickersets_))
{}

const std::int32_t messages_toggleStickerSets::ID;

void messages_toggleStickerSets::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1257951254);
  TlStoreBinary::store((var0 = flags_ | (uninstall_ << 0) | (archive_ << 1) | (unarchive_ << 2)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stickersets_, s);
}

void messages_toggleStickerSets::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1257951254);
  TlStoreBinary::store((var0 = flags_ | (uninstall_ << 0) | (archive_ << 1) | (unarchive_ << 2)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stickersets_, s);
}

void messages_toggleStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleStickerSets");
    s.store_field("flags", (var0 = flags_ | (uninstall_ << 0) | (archive_ << 1) | (unarchive_ << 2)));
    if (var0 & 1) { s.store_field("uninstall", true); }
    if (var0 & 2) { s.store_field("archive", true); }
    if (var0 & 4) { s.store_field("unarchive", true); }
    { s.store_vector_begin("stickersets", stickersets_.size()); for (const auto &_value : stickersets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_toggleStickerSets::ReturnType messages_toggleStickerSets::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_toggleSuggestedPostApproval::messages_toggleSuggestedPostApproval(int32 flags_, bool reject_, object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 schedule_date_, string const &reject_comment_)
  : flags_(flags_)
  , reject_(reject_)
  , peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , schedule_date_(schedule_date_)
  , reject_comment_(reject_comment_)
{}

const std::int32_t messages_toggleSuggestedPostApproval::ID;

void messages_toggleSuggestedPostApproval::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2130229924);
  TlStoreBinary::store((var0 = flags_ | (reject_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  if (var0 & 1) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 4) { TlStoreString::store(reject_comment_, s); }
}

void messages_toggleSuggestedPostApproval::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2130229924);
  TlStoreBinary::store((var0 = flags_ | (reject_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  if (var0 & 1) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 4) { TlStoreString::store(reject_comment_, s); }
}

void messages_toggleSuggestedPostApproval::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleSuggestedPostApproval");
    s.store_field("flags", (var0 = flags_ | (reject_ << 1)));
    if (var0 & 2) { s.store_field("reject", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    if (var0 & 1) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 4) { s.store_field("reject_comment", reject_comment_); }
    s.store_class_end();
  }
}

messages_toggleSuggestedPostApproval::ReturnType messages_toggleSuggestedPostApproval::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_updateDialogFiltersOrder::messages_updateDialogFiltersOrder(array<int32> &&order_)
  : order_(std::move(order_))
{}

const std::int32_t messages_updateDialogFiltersOrder::ID;

void messages_updateDialogFiltersOrder::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-983318044);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_updateDialogFiltersOrder::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-983318044);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_updateDialogFiltersOrder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.updateDialogFiltersOrder");
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_updateDialogFiltersOrder::ReturnType messages_updateDialogFiltersOrder::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_updatePinnedMessage::messages_updatePinnedMessage(int32 flags_, bool silent_, bool unpin_, bool pm_oneside_, object_ptr<InputPeer> &&peer_, int32 id_)
  : flags_(flags_)
  , silent_(silent_)
  , unpin_(unpin_)
  , pm_oneside_(pm_oneside_)
  , peer_(std::move(peer_))
  , id_(id_)
{}

const std::int32_t messages_updatePinnedMessage::ID;

void messages_updatePinnedMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-760547348);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 0) | (unpin_ << 1) | (pm_oneside_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void messages_updatePinnedMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-760547348);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 0) | (unpin_ << 1) | (pm_oneside_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void messages_updatePinnedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.updatePinnedMessage");
    s.store_field("flags", (var0 = flags_ | (silent_ << 0) | (unpin_ << 1) | (pm_oneside_ << 2)));
    if (var0 & 1) { s.store_field("silent", true); }
    if (var0 & 2) { s.store_field("unpin", true); }
    if (var0 & 4) { s.store_field("pm_oneside", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

messages_updatePinnedMessage::ReturnType messages_updatePinnedMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_checkGiftCode::payments_checkGiftCode(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t payments_checkGiftCode::ID;

void payments_checkGiftCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1907247935);
  TlStoreString::store(slug_, s);
}

void payments_checkGiftCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1907247935);
  TlStoreString::store(slug_, s);
}

void payments_checkGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.checkGiftCode");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

payments_checkGiftCode::ReturnType payments_checkGiftCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_checkedGiftCode>, 675942550>::parse(p);
#undef FAIL
}

payments_convertStarGift::payments_convertStarGift(object_ptr<InputSavedStarGift> &&stargift_)
  : stargift_(std::move(stargift_))
{}

const std::int32_t payments_convertStarGift::ID;

void payments_convertStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1958676331);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void payments_convertStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1958676331);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void payments_convertStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.convertStarGift");
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_class_end();
  }
}

payments_convertStarGift::ReturnType payments_convertStarGift::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_editConnectedStarRefBot::payments_editConnectedStarRefBot(int32 flags_, bool revoked_, object_ptr<InputPeer> &&peer_, string const &link_)
  : flags_(flags_)
  , revoked_(revoked_)
  , peer_(std::move(peer_))
  , link_(link_)
{}

const std::int32_t payments_editConnectedStarRefBot::ID;

void payments_editConnectedStarRefBot::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-453204829);
  TlStoreBinary::store((var0 = flags_ | (revoked_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
}

void payments_editConnectedStarRefBot::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-453204829);
  TlStoreBinary::store((var0 = flags_ | (revoked_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
}

void payments_editConnectedStarRefBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.editConnectedStarRefBot");
    s.store_field("flags", (var0 = flags_ | (revoked_ << 0)));
    if (var0 & 1) { s.store_field("revoked", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("link", link_);
    s.store_class_end();
  }
}

payments_editConnectedStarRefBot::ReturnType payments_editConnectedStarRefBot::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_connectedStarRefBots>, -1730811363>::parse(p);
#undef FAIL
}

payments_getResaleStarGifts::payments_getResaleStarGifts(int32 flags_, bool sort_by_price_, bool sort_by_num_, int64 attributes_hash_, int64 gift_id_, array<object_ptr<StarGiftAttributeId>> &&attributes_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , sort_by_price_(sort_by_price_)
  , sort_by_num_(sort_by_num_)
  , attributes_hash_(attributes_hash_)
  , gift_id_(gift_id_)
  , attributes_(std::move(attributes_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t payments_getResaleStarGifts::ID;

void payments_getResaleStarGifts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2053087798);
  TlStoreBinary::store((var0 = flags_ | (sort_by_price_ << 1) | (sort_by_num_ << 2)), s);
  if (var0 & 1) { TlStoreBinary::store(attributes_hash_, s); }
  TlStoreBinary::store(gift_id_, s);
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s); }
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getResaleStarGifts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2053087798);
  TlStoreBinary::store((var0 = flags_ | (sort_by_price_ << 1) | (sort_by_num_ << 2)), s);
  if (var0 & 1) { TlStoreBinary::store(attributes_hash_, s); }
  TlStoreBinary::store(gift_id_, s);
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s); }
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getResaleStarGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getResaleStarGifts");
    s.store_field("flags", (var0 = flags_ | (sort_by_price_ << 1) | (sort_by_num_ << 2)));
    if (var0 & 2) { s.store_field("sort_by_price", true); }
    if (var0 & 4) { s.store_field("sort_by_num", true); }
    if (var0 & 1) { s.store_field("attributes_hash", attributes_hash_); }
    s.store_field("gift_id", gift_id_);
    if (var0 & 8) { { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

payments_getResaleStarGifts::ReturnType payments_getResaleStarGifts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_resaleStarGifts>, -1803939105>::parse(p);
#undef FAIL
}

payments_getStarsGiftOptions::payments_getStarsGiftOptions(int32 flags_, object_ptr<InputUser> &&user_id_)
  : flags_(flags_)
  , user_id_(std::move(user_id_))
{}

const std::int32_t payments_getStarsGiftOptions::ID;

void payments_getStarsGiftOptions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-741774392);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s); }
}

void payments_getStarsGiftOptions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-741774392);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s); }
}

void payments_getStarsGiftOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsGiftOptions");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get())); }
    s.store_class_end();
  }
}

payments_getStarsGiftOptions::ReturnType payments_getStarsGiftOptions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starsGiftOption>, 1577421297>>, 481674261>::parse(p);
#undef FAIL
}

payments_getStarsRevenueAdsAccountUrl::payments_getStarsRevenueAdsAccountUrl(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t payments_getStarsRevenueAdsAccountUrl::ID;

void payments_getStarsRevenueAdsAccountUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-774377531);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_getStarsRevenueAdsAccountUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-774377531);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_getStarsRevenueAdsAccountUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsRevenueAdsAccountUrl");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

payments_getStarsRevenueAdsAccountUrl::ReturnType payments_getStarsRevenueAdsAccountUrl::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starsRevenueAdsAccountUrl>, 961445665>::parse(p);
#undef FAIL
}

payments_sendStarsForm::payments_sendStarsForm(int64 form_id_, object_ptr<InputInvoice> &&invoice_)
  : form_id_(form_id_)
  , invoice_(std::move(invoice_))
{}

const std::int32_t payments_sendStarsForm::ID;

void payments_sendStarsForm::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2040056084);
  TlStoreBinary::store(form_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
}

void payments_sendStarsForm::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2040056084);
  TlStoreBinary::store(form_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
}

void payments_sendStarsForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.sendStarsForm");
    s.store_field("form_id", form_id_);
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_class_end();
  }
}

payments_sendStarsForm::ReturnType payments_sendStarsForm::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_PaymentResult>::parse(p);
#undef FAIL
}

payments_transferStarGift::payments_transferStarGift(object_ptr<InputSavedStarGift> &&stargift_, object_ptr<InputPeer> &&to_id_)
  : stargift_(std::move(stargift_))
  , to_id_(std::move(to_id_))
{}

const std::int32_t payments_transferStarGift::ID;

void payments_transferStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2132285290);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_id_, s);
}

void payments_transferStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2132285290);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_id_, s);
}

void payments_transferStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.transferStarGift");
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_object_field("to_id", static_cast<const BaseObject *>(to_id_.get()));
    s.store_class_end();
  }
}

payments_transferStarGift::ReturnType payments_transferStarGift::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_createConferenceCall::phone_createConferenceCall(int32 flags_, bool muted_, bool video_stopped_, bool join_, int32 random_id_, UInt256 const &public_key_, bytes &&block_, object_ptr<dataJSON> &&params_)
  : flags_(flags_)
  , muted_(muted_)
  , video_stopped_(video_stopped_)
  , join_(join_)
  , random_id_(random_id_)
  , public_key_(public_key_)
  , block_(std::move(block_))
  , params_(std::move(params_))
{}

const std::int32_t phone_createConferenceCall::ID;

void phone_createConferenceCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2097431739);
  TlStoreBinary::store((var0 = flags_ | (muted_ << 0) | (video_stopped_ << 2) | (join_ << 3)), s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 8) { TlStoreBinary::store(public_key_, s); }
  if (var0 & 8) { TlStoreString::store(block_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s); }
}

void phone_createConferenceCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2097431739);
  TlStoreBinary::store((var0 = flags_ | (muted_ << 0) | (video_stopped_ << 2) | (join_ << 3)), s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 8) { TlStoreBinary::store(public_key_, s); }
  if (var0 & 8) { TlStoreString::store(block_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s); }
}

void phone_createConferenceCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.createConferenceCall");
    s.store_field("flags", (var0 = flags_ | (muted_ << 0) | (video_stopped_ << 2) | (join_ << 3)));
    if (var0 & 1) { s.store_field("muted", true); }
    if (var0 & 4) { s.store_field("video_stopped", true); }
    if (var0 & 8) { s.store_field("join", true); }
    s.store_field("random_id", random_id_);
    if (var0 & 8) { s.store_field("public_key", public_key_); }
    if (var0 & 8) { s.store_bytes_field("block", block_); }
    if (var0 & 8) { s.store_object_field("params", static_cast<const BaseObject *>(params_.get())); }
    s.store_class_end();
  }
}

phone_createConferenceCall::ReturnType phone_createConferenceCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_getGroupCallChainBlocks::phone_getGroupCallChainBlocks(object_ptr<InputGroupCall> &&call_, int32 sub_chain_id_, int32 offset_, int32 limit_)
  : call_(std::move(call_))
  , sub_chain_id_(sub_chain_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t phone_getGroupCallChainBlocks::ID;

void phone_getGroupCallChainBlocks::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-291534682);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(sub_chain_id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void phone_getGroupCallChainBlocks::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-291534682);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(sub_chain_id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void phone_getGroupCallChainBlocks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.getGroupCallChainBlocks");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("sub_chain_id", sub_chain_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

phone_getGroupCallChainBlocks::ReturnType phone_getGroupCallChainBlocks::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_startScheduledGroupCall::phone_startScheduledGroupCall(object_ptr<InputGroupCall> &&call_)
  : call_(std::move(call_))
{}

const std::int32_t phone_startScheduledGroupCall::ID;

void phone_startScheduledGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1451287362);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_startScheduledGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1451287362);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_startScheduledGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.startScheduledGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

phone_startScheduledGroupCall::ReturnType phone_startScheduledGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

stories_activateStealthMode::stories_activateStealthMode(int32 flags_, bool past_, bool future_)
  : flags_(flags_)
  , past_(past_)
  , future_(future_)
{}

const std::int32_t stories_activateStealthMode::ID;

void stories_activateStealthMode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1471926630);
  TlStoreBinary::store((var0 = flags_ | (past_ << 0) | (future_ << 1)), s);
}

void stories_activateStealthMode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1471926630);
  TlStoreBinary::store((var0 = flags_ | (past_ << 0) | (future_ << 1)), s);
}

void stories_activateStealthMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.activateStealthMode");
    s.store_field("flags", (var0 = flags_ | (past_ << 0) | (future_ << 1)));
    if (var0 & 1) { s.store_field("past", true); }
    if (var0 & 2) { s.store_field("future", true); }
    s.store_class_end();
  }
}

stories_activateStealthMode::ReturnType stories_activateStealthMode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

stories_canSendStory::stories_canSendStory(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t stories_canSendStory::ID;

void stories_canSendStory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(820732912);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void stories_canSendStory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(820732912);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void stories_canSendStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.canSendStory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

stories_canSendStory::ReturnType stories_canSendStory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_canSendStoryCount>, -1014513586>::parse(p);
#undef FAIL
}

stories_editStory::stories_editStory(int32 flags_, object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<InputMedia> &&media_, array<object_ptr<MediaArea>> &&media_areas_, string const &caption_, array<object_ptr<MessageEntity>> &&entities_, array<object_ptr<InputPrivacyRule>> &&privacy_rules_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , id_(id_)
  , media_(std::move(media_))
  , media_areas_(std::move(media_areas_))
  , caption_(caption_)
  , entities_(std::move(entities_))
  , privacy_rules_(std::move(privacy_rules_))
{}

const std::int32_t stories_editStory::ID;

void stories_editStory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1249658298);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(media_areas_, s); }
  if (var0 & 2) { TlStoreString::store(caption_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(privacy_rules_, s); }
}

void stories_editStory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1249658298);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(media_areas_, s); }
  if (var0 & 2) { TlStoreString::store(caption_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(privacy_rules_, s); }
}

void stories_editStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.editStory");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 8) { { s.store_vector_begin("media_areas", media_areas_.size()); for (const auto &_value : media_areas_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { s.store_field("caption", caption_); }
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { { s.store_vector_begin("privacy_rules", privacy_rules_.size()); for (const auto &_value : privacy_rules_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

stories_editStory::ReturnType stories_editStory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

stories_getPinnedStories::stories_getPinnedStories(object_ptr<InputPeer> &&peer_, int32 offset_id_, int32 limit_)
  : peer_(std::move(peer_))
  , offset_id_(offset_id_)
  , limit_(limit_)
{}

const std::int32_t stories_getPinnedStories::ID;

void stories_getPinnedStories::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1478600156);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getPinnedStories::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1478600156);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getPinnedStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getPinnedStories");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("offset_id", offset_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stories_getPinnedStories::ReturnType stories_getPinnedStories::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_stories>, 1673780490>::parse(p);
#undef FAIL
}

stories_getStoriesArchive::stories_getStoriesArchive(object_ptr<InputPeer> &&peer_, int32 offset_id_, int32 limit_)
  : peer_(std::move(peer_))
  , offset_id_(offset_id_)
  , limit_(limit_)
{}

const std::int32_t stories_getStoriesArchive::ID;

void stories_getStoriesArchive::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1271586794);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getStoriesArchive::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1271586794);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getStoriesArchive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getStoriesArchive");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("offset_id", offset_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stories_getStoriesArchive::ReturnType stories_getStoriesArchive::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_stories>, 1673780490>::parse(p);
#undef FAIL
}

users_getSavedMusic::users_getSavedMusic(object_ptr<InputUser> &&id_, int32 offset_, int32 limit_, int64 hash_)
  : id_(std::move(id_))
  , offset_(offset_)
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t users_getSavedMusic::ID;

void users_getSavedMusic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2022539235);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void users_getSavedMusic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2022539235);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void users_getSavedMusic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.getSavedMusic");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

users_getSavedMusic::ReturnType users_getSavedMusic::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<users_SavedMusic>::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
