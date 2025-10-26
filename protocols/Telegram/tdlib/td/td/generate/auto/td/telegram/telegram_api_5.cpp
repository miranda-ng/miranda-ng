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


object_ptr<AttachMenuPeerType> AttachMenuPeerType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case attachMenuPeerTypeSameBotPM::ID:
      return attachMenuPeerTypeSameBotPM::fetch(p);
    case attachMenuPeerTypeBotPM::ID:
      return attachMenuPeerTypeBotPM::fetch(p);
    case attachMenuPeerTypePM::ID:
      return attachMenuPeerTypePM::fetch(p);
    case attachMenuPeerTypeChat::ID:
      return attachMenuPeerTypeChat::fetch(p);
    case attachMenuPeerTypeBroadcast::ID:
      return attachMenuPeerTypeBroadcast::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t attachMenuPeerTypeSameBotPM::ID;

object_ptr<AttachMenuPeerType> attachMenuPeerTypeSameBotPM::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuPeerTypeSameBotPM>();
}

void attachMenuPeerTypeSameBotPM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuPeerTypeSameBotPM");
    s.store_class_end();
  }
}

const std::int32_t attachMenuPeerTypeBotPM::ID;

object_ptr<AttachMenuPeerType> attachMenuPeerTypeBotPM::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuPeerTypeBotPM>();
}

void attachMenuPeerTypeBotPM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuPeerTypeBotPM");
    s.store_class_end();
  }
}

const std::int32_t attachMenuPeerTypePM::ID;

object_ptr<AttachMenuPeerType> attachMenuPeerTypePM::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuPeerTypePM>();
}

void attachMenuPeerTypePM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuPeerTypePM");
    s.store_class_end();
  }
}

const std::int32_t attachMenuPeerTypeChat::ID;

object_ptr<AttachMenuPeerType> attachMenuPeerTypeChat::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuPeerTypeChat>();
}

void attachMenuPeerTypeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuPeerTypeChat");
    s.store_class_end();
  }
}

const std::int32_t attachMenuPeerTypeBroadcast::ID;

object_ptr<AttachMenuPeerType> attachMenuPeerTypeBroadcast::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuPeerTypeBroadcast>();
}

void attachMenuPeerTypeBroadcast::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuPeerTypeBroadcast");
    s.store_class_end();
  }
}

autoDownloadSettings::autoDownloadSettings()
  : flags_()
  , disabled_()
  , video_preload_large_()
  , audio_preload_next_()
  , phonecalls_less_data_()
  , stories_preload_()
  , photo_size_max_()
  , video_size_max_()
  , file_size_max_()
  , video_upload_maxbitrate_()
  , small_queue_active_operations_max_()
  , large_queue_active_operations_max_()
{}

autoDownloadSettings::autoDownloadSettings(int32 flags_, bool disabled_, bool video_preload_large_, bool audio_preload_next_, bool phonecalls_less_data_, bool stories_preload_, int32 photo_size_max_, int64 video_size_max_, int64 file_size_max_, int32 video_upload_maxbitrate_, int32 small_queue_active_operations_max_, int32 large_queue_active_operations_max_)
  : flags_(flags_)
  , disabled_(disabled_)
  , video_preload_large_(video_preload_large_)
  , audio_preload_next_(audio_preload_next_)
  , phonecalls_less_data_(phonecalls_less_data_)
  , stories_preload_(stories_preload_)
  , photo_size_max_(photo_size_max_)
  , video_size_max_(video_size_max_)
  , file_size_max_(file_size_max_)
  , video_upload_maxbitrate_(video_upload_maxbitrate_)
  , small_queue_active_operations_max_(small_queue_active_operations_max_)
  , large_queue_active_operations_max_(large_queue_active_operations_max_)
{}

const std::int32_t autoDownloadSettings::ID;

object_ptr<autoDownloadSettings> autoDownloadSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<autoDownloadSettings> res = make_tl_object<autoDownloadSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->disabled_ = (var0 & 1) != 0;
  res->video_preload_large_ = (var0 & 2) != 0;
  res->audio_preload_next_ = (var0 & 4) != 0;
  res->phonecalls_less_data_ = (var0 & 8) != 0;
  res->stories_preload_ = (var0 & 16) != 0;
  res->photo_size_max_ = TlFetchInt::parse(p);
  res->video_size_max_ = TlFetchLong::parse(p);
  res->file_size_max_ = TlFetchLong::parse(p);
  res->video_upload_maxbitrate_ = TlFetchInt::parse(p);
  res->small_queue_active_operations_max_ = TlFetchInt::parse(p);
  res->large_queue_active_operations_max_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void autoDownloadSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (disabled_ << 0) | (video_preload_large_ << 1) | (audio_preload_next_ << 2) | (phonecalls_less_data_ << 3) | (stories_preload_ << 4)), s);
  TlStoreBinary::store(photo_size_max_, s);
  TlStoreBinary::store(video_size_max_, s);
  TlStoreBinary::store(file_size_max_, s);
  TlStoreBinary::store(video_upload_maxbitrate_, s);
  TlStoreBinary::store(small_queue_active_operations_max_, s);
  TlStoreBinary::store(large_queue_active_operations_max_, s);
}

void autoDownloadSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (disabled_ << 0) | (video_preload_large_ << 1) | (audio_preload_next_ << 2) | (phonecalls_less_data_ << 3) | (stories_preload_ << 4)), s);
  TlStoreBinary::store(photo_size_max_, s);
  TlStoreBinary::store(video_size_max_, s);
  TlStoreBinary::store(file_size_max_, s);
  TlStoreBinary::store(video_upload_maxbitrate_, s);
  TlStoreBinary::store(small_queue_active_operations_max_, s);
  TlStoreBinary::store(large_queue_active_operations_max_, s);
}

void autoDownloadSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autoDownloadSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (disabled_ << 0) | (video_preload_large_ << 1) | (audio_preload_next_ << 2) | (phonecalls_less_data_ << 3) | (stories_preload_ << 4)));
    if (var0 & 1) { s.store_field("disabled", true); }
    if (var0 & 2) { s.store_field("video_preload_large", true); }
    if (var0 & 4) { s.store_field("audio_preload_next", true); }
    if (var0 & 8) { s.store_field("phonecalls_less_data", true); }
    if (var0 & 16) { s.store_field("stories_preload", true); }
    s.store_field("photo_size_max", photo_size_max_);
    s.store_field("video_size_max", video_size_max_);
    s.store_field("file_size_max", file_size_max_);
    s.store_field("video_upload_maxbitrate", video_upload_maxbitrate_);
    s.store_field("small_queue_active_operations_max", small_queue_active_operations_max_);
    s.store_field("large_queue_active_operations_max", large_queue_active_operations_max_);
    s.store_class_end();
  }
}

object_ptr<BotMenuButton> BotMenuButton::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case botMenuButtonDefault::ID:
      return botMenuButtonDefault::fetch(p);
    case botMenuButtonCommands::ID:
      return botMenuButtonCommands::fetch(p);
    case botMenuButton::ID:
      return botMenuButton::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t botMenuButtonDefault::ID;

object_ptr<BotMenuButton> botMenuButtonDefault::fetch(TlBufferParser &p) {
  return make_tl_object<botMenuButtonDefault>();
}

void botMenuButtonDefault::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void botMenuButtonDefault::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void botMenuButtonDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botMenuButtonDefault");
    s.store_class_end();
  }
}

const std::int32_t botMenuButtonCommands::ID;

object_ptr<BotMenuButton> botMenuButtonCommands::fetch(TlBufferParser &p) {
  return make_tl_object<botMenuButtonCommands>();
}

void botMenuButtonCommands::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void botMenuButtonCommands::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void botMenuButtonCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botMenuButtonCommands");
    s.store_class_end();
  }
}

botMenuButton::botMenuButton(string const &text_, string const &url_)
  : text_(text_)
  , url_(url_)
{}

const std::int32_t botMenuButton::ID;

object_ptr<BotMenuButton> botMenuButton::fetch(TlBufferParser &p) {
  return make_tl_object<botMenuButton>(p);
}

botMenuButton::botMenuButton(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void botMenuButton::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void botMenuButton::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void botMenuButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botMenuButton");
    s.store_field("text", text_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

const std::int32_t botVerification::ID;

object_ptr<botVerification> botVerification::fetch(TlBufferParser &p) {
  return make_tl_object<botVerification>(p);
}

botVerification::botVerification(TlBufferParser &p)
  : bot_id_(TlFetchLong::parse(p))
  , icon_(TlFetchLong::parse(p))
  , description_(TlFetchString<string>::parse(p))
{}

void botVerification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botVerification");
    s.store_field("bot_id", bot_id_);
    s.store_field("icon", icon_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

businessIntro::businessIntro()
  : flags_()
  , title_()
  , description_()
  , sticker_()
{}

const std::int32_t businessIntro::ID;

object_ptr<businessIntro> businessIntro::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessIntro> res = make_tl_object<businessIntro>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->sticker_ = TlFetchObject<Document>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessIntro::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessIntro");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 1) { s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get())); }
    s.store_class_end();
  }
}

chatAdminRights::chatAdminRights()
  : flags_()
  , change_info_()
  , post_messages_()
  , edit_messages_()
  , delete_messages_()
  , ban_users_()
  , invite_users_()
  , pin_messages_()
  , add_admins_()
  , anonymous_()
  , manage_call_()
  , other_()
  , manage_topics_()
  , post_stories_()
  , edit_stories_()
  , delete_stories_()
  , manage_direct_messages_()
{}

chatAdminRights::chatAdminRights(int32 flags_, bool change_info_, bool post_messages_, bool edit_messages_, bool delete_messages_, bool ban_users_, bool invite_users_, bool pin_messages_, bool add_admins_, bool anonymous_, bool manage_call_, bool other_, bool manage_topics_, bool post_stories_, bool edit_stories_, bool delete_stories_, bool manage_direct_messages_)
  : flags_(flags_)
  , change_info_(change_info_)
  , post_messages_(post_messages_)
  , edit_messages_(edit_messages_)
  , delete_messages_(delete_messages_)
  , ban_users_(ban_users_)
  , invite_users_(invite_users_)
  , pin_messages_(pin_messages_)
  , add_admins_(add_admins_)
  , anonymous_(anonymous_)
  , manage_call_(manage_call_)
  , other_(other_)
  , manage_topics_(manage_topics_)
  , post_stories_(post_stories_)
  , edit_stories_(edit_stories_)
  , delete_stories_(delete_stories_)
  , manage_direct_messages_(manage_direct_messages_)
{}

const std::int32_t chatAdminRights::ID;

object_ptr<chatAdminRights> chatAdminRights::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatAdminRights> res = make_tl_object<chatAdminRights>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->change_info_ = (var0 & 1) != 0;
  res->post_messages_ = (var0 & 2) != 0;
  res->edit_messages_ = (var0 & 4) != 0;
  res->delete_messages_ = (var0 & 8) != 0;
  res->ban_users_ = (var0 & 16) != 0;
  res->invite_users_ = (var0 & 32) != 0;
  res->pin_messages_ = (var0 & 128) != 0;
  res->add_admins_ = (var0 & 512) != 0;
  res->anonymous_ = (var0 & 1024) != 0;
  res->manage_call_ = (var0 & 2048) != 0;
  res->other_ = (var0 & 4096) != 0;
  res->manage_topics_ = (var0 & 8192) != 0;
  res->post_stories_ = (var0 & 16384) != 0;
  res->edit_stories_ = (var0 & 32768) != 0;
  res->delete_stories_ = (var0 & 65536) != 0;
  res->manage_direct_messages_ = (var0 & 131072) != 0;
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void chatAdminRights::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (change_info_ << 0) | (post_messages_ << 1) | (edit_messages_ << 2) | (delete_messages_ << 3) | (ban_users_ << 4) | (invite_users_ << 5) | (pin_messages_ << 7) | (add_admins_ << 9) | (anonymous_ << 10) | (manage_call_ << 11) | (other_ << 12) | (manage_topics_ << 13) | (post_stories_ << 14) | (edit_stories_ << 15) | (delete_stories_ << 16) | (manage_direct_messages_ << 17)), s);
}

void chatAdminRights::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (change_info_ << 0) | (post_messages_ << 1) | (edit_messages_ << 2) | (delete_messages_ << 3) | (ban_users_ << 4) | (invite_users_ << 5) | (pin_messages_ << 7) | (add_admins_ << 9) | (anonymous_ << 10) | (manage_call_ << 11) | (other_ << 12) | (manage_topics_ << 13) | (post_stories_ << 14) | (edit_stories_ << 15) | (delete_stories_ << 16) | (manage_direct_messages_ << 17)), s);
}

void chatAdminRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatAdminRights");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (change_info_ << 0) | (post_messages_ << 1) | (edit_messages_ << 2) | (delete_messages_ << 3) | (ban_users_ << 4) | (invite_users_ << 5) | (pin_messages_ << 7) | (add_admins_ << 9) | (anonymous_ << 10) | (manage_call_ << 11) | (other_ << 12) | (manage_topics_ << 13) | (post_stories_ << 14) | (edit_stories_ << 15) | (delete_stories_ << 16) | (manage_direct_messages_ << 17)));
    if (var0 & 1) { s.store_field("change_info", true); }
    if (var0 & 2) { s.store_field("post_messages", true); }
    if (var0 & 4) { s.store_field("edit_messages", true); }
    if (var0 & 8) { s.store_field("delete_messages", true); }
    if (var0 & 16) { s.store_field("ban_users", true); }
    if (var0 & 32) { s.store_field("invite_users", true); }
    if (var0 & 128) { s.store_field("pin_messages", true); }
    if (var0 & 512) { s.store_field("add_admins", true); }
    if (var0 & 1024) { s.store_field("anonymous", true); }
    if (var0 & 2048) { s.store_field("manage_call", true); }
    if (var0 & 4096) { s.store_field("other", true); }
    if (var0 & 8192) { s.store_field("manage_topics", true); }
    if (var0 & 16384) { s.store_field("post_stories", true); }
    if (var0 & 32768) { s.store_field("edit_stories", true); }
    if (var0 & 65536) { s.store_field("delete_stories", true); }
    if (var0 & 131072) { s.store_field("manage_direct_messages", true); }
    s.store_class_end();
  }
}

const std::int32_t chatOnlines::ID;

object_ptr<chatOnlines> chatOnlines::fetch(TlBufferParser &p) {
  return make_tl_object<chatOnlines>(p);
}

chatOnlines::chatOnlines(TlBufferParser &p)
  : onlines_(TlFetchInt::parse(p))
{}

void chatOnlines::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatOnlines");
    s.store_field("onlines", onlines_);
    s.store_class_end();
  }
}

config::config()
  : flags_()
  , default_p2p_contacts_()
  , preload_featured_stickers_()
  , revoke_pm_inbox_()
  , blocked_mode_()
  , force_try_ipv6_()
  , date_()
  , expires_()
  , test_mode_()
  , this_dc_()
  , dc_options_()
  , dc_txt_domain_name_()
  , chat_size_max_()
  , megagroup_size_max_()
  , forwarded_count_max_()
  , online_update_period_ms_()
  , offline_blur_timeout_ms_()
  , offline_idle_timeout_ms_()
  , online_cloud_timeout_ms_()
  , notify_cloud_delay_ms_()
  , notify_default_delay_ms_()
  , push_chat_period_ms_()
  , push_chat_limit_()
  , edit_time_limit_()
  , revoke_time_limit_()
  , revoke_pm_time_limit_()
  , rating_e_decay_()
  , stickers_recent_limit_()
  , channels_read_media_period_()
  , tmp_sessions_()
  , call_receive_timeout_ms_()
  , call_ring_timeout_ms_()
  , call_connect_timeout_ms_()
  , call_packet_timeout_ms_()
  , me_url_prefix_()
  , autoupdate_url_prefix_()
  , gif_search_username_()
  , venue_search_username_()
  , img_search_username_()
  , static_maps_provider_()
  , caption_length_max_()
  , message_length_max_()
  , webfile_dc_id_()
  , suggested_lang_code_()
  , lang_pack_version_()
  , base_lang_pack_version_()
  , reactions_default_()
  , autologin_token_()
{}

const std::int32_t config::ID;

object_ptr<config> config::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<config> res = make_tl_object<config>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->default_p2p_contacts_ = (var0 & 8) != 0;
  res->preload_featured_stickers_ = (var0 & 16) != 0;
  res->revoke_pm_inbox_ = (var0 & 64) != 0;
  res->blocked_mode_ = (var0 & 256) != 0;
  res->force_try_ipv6_ = (var0 & 16384) != 0;
  res->date_ = TlFetchInt::parse(p);
  res->expires_ = TlFetchInt::parse(p);
  res->test_mode_ = TlFetchBool::parse(p);
  res->this_dc_ = TlFetchInt::parse(p);
  res->dc_options_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<dcOption>, 414687501>>, 481674261>::parse(p);
  res->dc_txt_domain_name_ = TlFetchString<string>::parse(p);
  res->chat_size_max_ = TlFetchInt::parse(p);
  res->megagroup_size_max_ = TlFetchInt::parse(p);
  res->forwarded_count_max_ = TlFetchInt::parse(p);
  res->online_update_period_ms_ = TlFetchInt::parse(p);
  res->offline_blur_timeout_ms_ = TlFetchInt::parse(p);
  res->offline_idle_timeout_ms_ = TlFetchInt::parse(p);
  res->online_cloud_timeout_ms_ = TlFetchInt::parse(p);
  res->notify_cloud_delay_ms_ = TlFetchInt::parse(p);
  res->notify_default_delay_ms_ = TlFetchInt::parse(p);
  res->push_chat_period_ms_ = TlFetchInt::parse(p);
  res->push_chat_limit_ = TlFetchInt::parse(p);
  res->edit_time_limit_ = TlFetchInt::parse(p);
  res->revoke_time_limit_ = TlFetchInt::parse(p);
  res->revoke_pm_time_limit_ = TlFetchInt::parse(p);
  res->rating_e_decay_ = TlFetchInt::parse(p);
  res->stickers_recent_limit_ = TlFetchInt::parse(p);
  res->channels_read_media_period_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->tmp_sessions_ = TlFetchInt::parse(p); }
  res->call_receive_timeout_ms_ = TlFetchInt::parse(p);
  res->call_ring_timeout_ms_ = TlFetchInt::parse(p);
  res->call_connect_timeout_ms_ = TlFetchInt::parse(p);
  res->call_packet_timeout_ms_ = TlFetchInt::parse(p);
  res->me_url_prefix_ = TlFetchString<string>::parse(p);
  if (var0 & 128) { res->autoupdate_url_prefix_ = TlFetchString<string>::parse(p); }
  if (var0 & 512) { res->gif_search_username_ = TlFetchString<string>::parse(p); }
  if (var0 & 1024) { res->venue_search_username_ = TlFetchString<string>::parse(p); }
  if (var0 & 2048) { res->img_search_username_ = TlFetchString<string>::parse(p); }
  if (var0 & 4096) { res->static_maps_provider_ = TlFetchString<string>::parse(p); }
  res->caption_length_max_ = TlFetchInt::parse(p);
  res->message_length_max_ = TlFetchInt::parse(p);
  res->webfile_dc_id_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->suggested_lang_code_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->lang_pack_version_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->base_lang_pack_version_ = TlFetchInt::parse(p); }
  if (var0 & 32768) { res->reactions_default_ = TlFetchObject<Reaction>::parse(p); }
  if (var0 & 65536) { res->autologin_token_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void config::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "config");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (default_p2p_contacts_ << 3) | (preload_featured_stickers_ << 4) | (revoke_pm_inbox_ << 6) | (blocked_mode_ << 8) | (force_try_ipv6_ << 14)));
    if (var0 & 8) { s.store_field("default_p2p_contacts", true); }
    if (var0 & 16) { s.store_field("preload_featured_stickers", true); }
    if (var0 & 64) { s.store_field("revoke_pm_inbox", true); }
    if (var0 & 256) { s.store_field("blocked_mode", true); }
    if (var0 & 16384) { s.store_field("force_try_ipv6", true); }
    s.store_field("date", date_);
    s.store_field("expires", expires_);
    s.store_field("test_mode", test_mode_);
    s.store_field("this_dc", this_dc_);
    { s.store_vector_begin("dc_options", dc_options_.size()); for (const auto &_value : dc_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("dc_txt_domain_name", dc_txt_domain_name_);
    s.store_field("chat_size_max", chat_size_max_);
    s.store_field("megagroup_size_max", megagroup_size_max_);
    s.store_field("forwarded_count_max", forwarded_count_max_);
    s.store_field("online_update_period_ms", online_update_period_ms_);
    s.store_field("offline_blur_timeout_ms", offline_blur_timeout_ms_);
    s.store_field("offline_idle_timeout_ms", offline_idle_timeout_ms_);
    s.store_field("online_cloud_timeout_ms", online_cloud_timeout_ms_);
    s.store_field("notify_cloud_delay_ms", notify_cloud_delay_ms_);
    s.store_field("notify_default_delay_ms", notify_default_delay_ms_);
    s.store_field("push_chat_period_ms", push_chat_period_ms_);
    s.store_field("push_chat_limit", push_chat_limit_);
    s.store_field("edit_time_limit", edit_time_limit_);
    s.store_field("revoke_time_limit", revoke_time_limit_);
    s.store_field("revoke_pm_time_limit", revoke_pm_time_limit_);
    s.store_field("rating_e_decay", rating_e_decay_);
    s.store_field("stickers_recent_limit", stickers_recent_limit_);
    s.store_field("channels_read_media_period", channels_read_media_period_);
    if (var0 & 1) { s.store_field("tmp_sessions", tmp_sessions_); }
    s.store_field("call_receive_timeout_ms", call_receive_timeout_ms_);
    s.store_field("call_ring_timeout_ms", call_ring_timeout_ms_);
    s.store_field("call_connect_timeout_ms", call_connect_timeout_ms_);
    s.store_field("call_packet_timeout_ms", call_packet_timeout_ms_);
    s.store_field("me_url_prefix", me_url_prefix_);
    if (var0 & 128) { s.store_field("autoupdate_url_prefix", autoupdate_url_prefix_); }
    if (var0 & 512) { s.store_field("gif_search_username", gif_search_username_); }
    if (var0 & 1024) { s.store_field("venue_search_username", venue_search_username_); }
    if (var0 & 2048) { s.store_field("img_search_username", img_search_username_); }
    if (var0 & 4096) { s.store_field("static_maps_provider", static_maps_provider_); }
    s.store_field("caption_length_max", caption_length_max_);
    s.store_field("message_length_max", message_length_max_);
    s.store_field("webfile_dc_id", webfile_dc_id_);
    if (var0 & 4) { s.store_field("suggested_lang_code", suggested_lang_code_); }
    if (var0 & 4) { s.store_field("lang_pack_version", lang_pack_version_); }
    if (var0 & 4) { s.store_field("base_lang_pack_version", base_lang_pack_version_); }
    if (var0 & 32768) { s.store_object_field("reactions_default", static_cast<const BaseObject *>(reactions_default_.get())); }
    if (var0 & 65536) { s.store_field("autologin_token", autologin_token_); }
    s.store_class_end();
  }
}

connectedBot::connectedBot()
  : flags_()
  , bot_id_()
  , recipients_()
  , rights_()
{}

const std::int32_t connectedBot::ID;

object_ptr<connectedBot> connectedBot::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<connectedBot> res = make_tl_object<connectedBot>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->bot_id_ = TlFetchLong::parse(p);
  res->recipients_ = TlFetchBoxed<TlFetchObject<businessBotRecipients>, -1198722189>::parse(p);
  res->rights_ = TlFetchBoxed<TlFetchObject<businessBotRights>, -1604170505>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void connectedBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectedBot");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("bot_id", bot_id_);
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_object_field("rights", static_cast<const BaseObject *>(rights_.get()));
    s.store_class_end();
  }
}

const std::int32_t contact::ID;

object_ptr<contact> contact::fetch(TlBufferParser &p) {
  return make_tl_object<contact>(p);
}

contact::contact(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , mutual_(TlFetchBool::parse(p))
{}

void contact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contact");
    s.store_field("user_id", user_id_);
    s.store_field("mutual", mutual_);
    s.store_class_end();
  }
}

dataJSON::dataJSON(string const &data_)
  : data_(data_)
{}

const std::int32_t dataJSON::ID;

object_ptr<dataJSON> dataJSON::fetch(TlBufferParser &p) {
  return make_tl_object<dataJSON>(p);
}

dataJSON::dataJSON(TlBufferParser &p)
  : data_(TlFetchString<string>::parse(p))
{}

void dataJSON::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(data_, s);
}

void dataJSON::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(data_, s);
}

void dataJSON::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dataJSON");
    s.store_field("data", data_);
    s.store_class_end();
  }
}

object_ptr<Dialog> Dialog::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case dialog::ID:
      return dialog::fetch(p);
    case dialogFolder::ID:
      return dialogFolder::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

dialog::dialog()
  : flags_()
  , pinned_()
  , unread_mark_()
  , view_forum_as_messages_()
  , peer_()
  , top_message_()
  , read_inbox_max_id_()
  , read_outbox_max_id_()
  , unread_count_()
  , unread_mentions_count_()
  , unread_reactions_count_()
  , notify_settings_()
  , pts_()
  , draft_()
  , folder_id_()
  , ttl_period_()
{}

const std::int32_t dialog::ID;

object_ptr<Dialog> dialog::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<dialog> res = make_tl_object<dialog>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 4) != 0;
  res->unread_mark_ = (var0 & 8) != 0;
  res->view_forum_as_messages_ = (var0 & 64) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->top_message_ = TlFetchInt::parse(p);
  res->read_inbox_max_id_ = TlFetchInt::parse(p);
  res->read_outbox_max_id_ = TlFetchInt::parse(p);
  res->unread_count_ = TlFetchInt::parse(p);
  res->unread_mentions_count_ = TlFetchInt::parse(p);
  res->unread_reactions_count_ = TlFetchInt::parse(p);
  res->notify_settings_ = TlFetchBoxed<TlFetchObject<peerNotifySettings>, -1721619444>::parse(p);
  if (var0 & 1) { res->pts_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->draft_ = TlFetchObject<DraftMessage>::parse(p); }
  if (var0 & 16) { res->folder_id_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void dialog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialog");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 2) | (unread_mark_ << 3) | (view_forum_as_messages_ << 6)));
    if (var0 & 4) { s.store_field("pinned", true); }
    if (var0 & 8) { s.store_field("unread_mark", true); }
    if (var0 & 64) { s.store_field("view_forum_as_messages", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("top_message", top_message_);
    s.store_field("read_inbox_max_id", read_inbox_max_id_);
    s.store_field("read_outbox_max_id", read_outbox_max_id_);
    s.store_field("unread_count", unread_count_);
    s.store_field("unread_mentions_count", unread_mentions_count_);
    s.store_field("unread_reactions_count", unread_reactions_count_);
    s.store_object_field("notify_settings", static_cast<const BaseObject *>(notify_settings_.get()));
    if (var0 & 1) { s.store_field("pts", pts_); }
    if (var0 & 2) { s.store_object_field("draft", static_cast<const BaseObject *>(draft_.get())); }
    if (var0 & 16) { s.store_field("folder_id", folder_id_); }
    if (var0 & 32) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

dialogFolder::dialogFolder()
  : flags_()
  , pinned_()
  , folder_()
  , peer_()
  , top_message_()
  , unread_muted_peers_count_()
  , unread_unmuted_peers_count_()
  , unread_muted_messages_count_()
  , unread_unmuted_messages_count_()
{}

const std::int32_t dialogFolder::ID;

object_ptr<Dialog> dialogFolder::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<dialogFolder> res = make_tl_object<dialogFolder>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 4) != 0;
  res->folder_ = TlFetchBoxed<TlFetchObject<folder>, -11252123>::parse(p);
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->top_message_ = TlFetchInt::parse(p);
  res->unread_muted_peers_count_ = TlFetchInt::parse(p);
  res->unread_unmuted_peers_count_ = TlFetchInt::parse(p);
  res->unread_muted_messages_count_ = TlFetchInt::parse(p);
  res->unread_unmuted_messages_count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void dialogFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialogFolder");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 2)));
    if (var0 & 4) { s.store_field("pinned", true); }
    s.store_object_field("folder", static_cast<const BaseObject *>(folder_.get()));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("top_message", top_message_);
    s.store_field("unread_muted_peers_count", unread_muted_peers_count_);
    s.store_field("unread_unmuted_peers_count", unread_unmuted_peers_count_);
    s.store_field("unread_muted_messages_count", unread_muted_messages_count_);
    s.store_field("unread_unmuted_messages_count", unread_unmuted_messages_count_);
    s.store_class_end();
  }
}

object_ptr<EmojiList> EmojiList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case emojiListNotModified::ID:
      return emojiListNotModified::fetch(p);
    case emojiList::ID:
      return emojiList::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t emojiListNotModified::ID;

object_ptr<EmojiList> emojiListNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<emojiListNotModified>();
}

void emojiListNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiListNotModified");
    s.store_class_end();
  }
}

const std::int32_t emojiList::ID;

object_ptr<EmojiList> emojiList::fetch(TlBufferParser &p) {
  return make_tl_object<emojiList>(p);
}

emojiList::emojiList(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , document_id_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void emojiList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiList");
    s.store_field("hash", hash_);
    { s.store_vector_begin("document_id", document_id_.size()); for (const auto &_value : document_id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<EncryptedFile> EncryptedFile::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case encryptedFileEmpty::ID:
      return encryptedFileEmpty::fetch(p);
    case encryptedFile::ID:
      return encryptedFile::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t encryptedFileEmpty::ID;

object_ptr<EncryptedFile> encryptedFileEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<encryptedFileEmpty>();
}

void encryptedFileEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedFileEmpty");
    s.store_class_end();
  }
}

const std::int32_t encryptedFile::ID;

object_ptr<EncryptedFile> encryptedFile::fetch(TlBufferParser &p) {
  return make_tl_object<encryptedFile>(p);
}

encryptedFile::encryptedFile(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , size_(TlFetchLong::parse(p))
  , dc_id_(TlFetchInt::parse(p))
  , key_fingerprint_(TlFetchInt::parse(p))
{}

void encryptedFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedFile");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("size", size_);
    s.store_field("dc_id", dc_id_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_class_end();
  }
}

const std::int32_t exportedStoryLink::ID;

object_ptr<exportedStoryLink> exportedStoryLink::fetch(TlBufferParser &p) {
  return make_tl_object<exportedStoryLink>(p);
}

exportedStoryLink::exportedStoryLink(TlBufferParser &p)
  : link_(TlFetchString<string>::parse(p))
{}

void exportedStoryLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "exportedStoryLink");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

object_ptr<InputBotInlineMessageID> InputBotInlineMessageID::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputBotInlineMessageID::ID:
      return inputBotInlineMessageID::fetch(p);
    case inputBotInlineMessageID64::ID:
      return inputBotInlineMessageID64::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

inputBotInlineMessageID::inputBotInlineMessageID(int32 dc_id_, int64 id_, int64 access_hash_)
  : dc_id_(dc_id_)
  , id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputBotInlineMessageID::ID;

object_ptr<InputBotInlineMessageID> inputBotInlineMessageID::fetch(TlBufferParser &p) {
  return make_tl_object<inputBotInlineMessageID>(p);
}

inputBotInlineMessageID::inputBotInlineMessageID(TlBufferParser &p)
  : dc_id_(TlFetchInt::parse(p))
  , id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputBotInlineMessageID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputBotInlineMessageID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputBotInlineMessageID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageID");
    s.store_field("dc_id", dc_id_);
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputBotInlineMessageID64::inputBotInlineMessageID64(int32 dc_id_, int64 owner_id_, int32 id_, int64 access_hash_)
  : dc_id_(dc_id_)
  , owner_id_(owner_id_)
  , id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputBotInlineMessageID64::ID;

object_ptr<InputBotInlineMessageID> inputBotInlineMessageID64::fetch(TlBufferParser &p) {
  return make_tl_object<inputBotInlineMessageID64>(p);
}

inputBotInlineMessageID64::inputBotInlineMessageID64(TlBufferParser &p)
  : dc_id_(TlFetchInt::parse(p))
  , owner_id_(TlFetchLong::parse(p))
  , id_(TlFetchInt::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputBotInlineMessageID64::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBinary::store(owner_id_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputBotInlineMessageID64::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBinary::store(owner_id_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputBotInlineMessageID64::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageID64");
    s.store_field("dc_id", dc_id_);
    s.store_field("owner_id", owner_id_);
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputBusinessChatLink::inputBusinessChatLink(int32 flags_, string const &message_, array<object_ptr<MessageEntity>> &&entities_, string const &title_)
  : flags_(flags_)
  , message_(message_)
  , entities_(std::move(entities_))
  , title_(title_)
{}

const std::int32_t inputBusinessChatLink::ID;

void inputBusinessChatLink::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(message_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 2) { TlStoreString::store(title_, s); }
}

void inputBusinessChatLink::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(message_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 2) { TlStoreString::store(title_, s); }
}

void inputBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessChatLink");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("message", message_);
    if (var0 & 1) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { s.store_field("title", title_); }
    s.store_class_end();
  }
}

inputBusinessIntro::inputBusinessIntro(int32 flags_, string const &title_, string const &description_, object_ptr<InputDocument> &&sticker_)
  : flags_(flags_)
  , title_(title_)
  , description_(description_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t inputBusinessIntro::ID;

void inputBusinessIntro::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(description_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s); }
}

void inputBusinessIntro::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(description_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s); }
}

void inputBusinessIntro::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessIntro");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 1) { s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get())); }
    s.store_class_end();
  }
}

const std::int32_t inputEncryptedFileEmpty::ID;

void inputEncryptedFileEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputEncryptedFileEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputEncryptedFileEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputEncryptedFileEmpty");
    s.store_class_end();
  }
}

inputEncryptedFileUploaded::inputEncryptedFileUploaded(int64 id_, int32 parts_, string const &md5_checksum_, int32 key_fingerprint_)
  : id_(id_)
  , parts_(parts_)
  , md5_checksum_(md5_checksum_)
  , key_fingerprint_(key_fingerprint_)
{}

const std::int32_t inputEncryptedFileUploaded::ID;

void inputEncryptedFileUploaded::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(md5_checksum_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void inputEncryptedFileUploaded::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(md5_checksum_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void inputEncryptedFileUploaded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputEncryptedFileUploaded");
    s.store_field("id", id_);
    s.store_field("parts", parts_);
    s.store_field("md5_checksum", md5_checksum_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_class_end();
  }
}

inputEncryptedFile::inputEncryptedFile(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputEncryptedFile::ID;

void inputEncryptedFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputEncryptedFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputEncryptedFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputEncryptedFile");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputEncryptedFileBigUploaded::inputEncryptedFileBigUploaded(int64 id_, int32 parts_, int32 key_fingerprint_)
  : id_(id_)
  , parts_(parts_)
  , key_fingerprint_(key_fingerprint_)
{}

const std::int32_t inputEncryptedFileBigUploaded::ID;

void inputEncryptedFileBigUploaded::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void inputEncryptedFileBigUploaded::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void inputEncryptedFileBigUploaded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputEncryptedFileBigUploaded");
    s.store_field("id", id_);
    s.store_field("parts", parts_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_class_end();
  }
}

inputThemeSettings::inputThemeSettings(int32 flags_, bool message_colors_animated_, object_ptr<BaseTheme> &&base_theme_, int32 accent_color_, int32 outbox_accent_color_, array<int32> &&message_colors_, object_ptr<InputWallPaper> &&wallpaper_, object_ptr<wallPaperSettings> &&wallpaper_settings_)
  : flags_(flags_)
  , message_colors_animated_(message_colors_animated_)
  , base_theme_(std::move(base_theme_))
  , accent_color_(accent_color_)
  , outbox_accent_color_(outbox_accent_color_)
  , message_colors_(std::move(message_colors_))
  , wallpaper_(std::move(wallpaper_))
  , wallpaper_settings_(std::move(wallpaper_settings_))
{}

const std::int32_t inputThemeSettings::ID;

void inputThemeSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (message_colors_animated_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(base_theme_, s);
  TlStoreBinary::store(accent_color_, s);
  if (var0 & 8) { TlStoreBinary::store(outbox_accent_color_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(message_colors_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 925826256>::store(wallpaper_settings_, s); }
}

void inputThemeSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (message_colors_animated_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(base_theme_, s);
  TlStoreBinary::store(accent_color_, s);
  if (var0 & 8) { TlStoreBinary::store(outbox_accent_color_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(message_colors_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 925826256>::store(wallpaper_settings_, s); }
}

void inputThemeSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputThemeSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (message_colors_animated_ << 2)));
    if (var0 & 4) { s.store_field("message_colors_animated", true); }
    s.store_object_field("base_theme", static_cast<const BaseObject *>(base_theme_.get()));
    s.store_field("accent_color", accent_color_);
    if (var0 & 8) { s.store_field("outbox_accent_color", outbox_accent_color_); }
    if (var0 & 1) { { s.store_vector_begin("message_colors", message_colors_.size()); for (const auto &_value : message_colors_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 2) { s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get())); }
    if (var0 & 2) { s.store_object_field("wallpaper_settings", static_cast<const BaseObject *>(wallpaper_settings_.get())); }
    s.store_class_end();
  }
}

maskCoords::maskCoords(int32 n_, double x_, double y_, double zoom_)
  : n_(n_)
  , x_(x_)
  , y_(y_)
  , zoom_(zoom_)
{}

const std::int32_t maskCoords::ID;

object_ptr<maskCoords> maskCoords::fetch(TlBufferParser &p) {
  return make_tl_object<maskCoords>(p);
}

maskCoords::maskCoords(TlBufferParser &p)
  : n_(TlFetchInt::parse(p))
  , x_(TlFetchDouble::parse(p))
  , y_(TlFetchDouble::parse(p))
  , zoom_(TlFetchDouble::parse(p))
{}

void maskCoords::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(n_, s);
  TlStoreBinary::store(x_, s);
  TlStoreBinary::store(y_, s);
  TlStoreBinary::store(zoom_, s);
}

void maskCoords::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(n_, s);
  TlStoreBinary::store(x_, s);
  TlStoreBinary::store(y_, s);
  TlStoreBinary::store(zoom_, s);
}

void maskCoords::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "maskCoords");
    s.store_field("n", n_);
    s.store_field("x", x_);
    s.store_field("y", y_);
    s.store_field("zoom", zoom_);
    s.store_class_end();
  }
}

mediaAreaCoordinates::mediaAreaCoordinates()
  : flags_()
  , x_()
  , y_()
  , w_()
  , h_()
  , rotation_()
  , radius_()
{}

mediaAreaCoordinates::mediaAreaCoordinates(int32 flags_, double x_, double y_, double w_, double h_, double rotation_, double radius_)
  : flags_(flags_)
  , x_(x_)
  , y_(y_)
  , w_(w_)
  , h_(h_)
  , rotation_(rotation_)
  , radius_(radius_)
{}

const std::int32_t mediaAreaCoordinates::ID;

object_ptr<mediaAreaCoordinates> mediaAreaCoordinates::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<mediaAreaCoordinates> res = make_tl_object<mediaAreaCoordinates>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->x_ = TlFetchDouble::parse(p);
  res->y_ = TlFetchDouble::parse(p);
  res->w_ = TlFetchDouble::parse(p);
  res->h_ = TlFetchDouble::parse(p);
  res->rotation_ = TlFetchDouble::parse(p);
  if (var0 & 1) { res->radius_ = TlFetchDouble::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void mediaAreaCoordinates::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(x_, s);
  TlStoreBinary::store(y_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(rotation_, s);
  if (var0 & 1) { TlStoreBinary::store(radius_, s); }
}

void mediaAreaCoordinates::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(x_, s);
  TlStoreBinary::store(y_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(rotation_, s);
  if (var0 & 1) { TlStoreBinary::store(radius_, s); }
}

void mediaAreaCoordinates::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mediaAreaCoordinates");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("x", x_);
    s.store_field("y", y_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("rotation", rotation_);
    if (var0 & 1) { s.store_field("radius", radius_); }
    s.store_class_end();
  }
}

missingInvitee::missingInvitee()
  : flags_()
  , premium_would_allow_invite_()
  , premium_required_for_pm_()
  , user_id_()
{}

const std::int32_t missingInvitee::ID;

object_ptr<missingInvitee> missingInvitee::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<missingInvitee> res = make_tl_object<missingInvitee>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->premium_would_allow_invite_ = (var0 & 1) != 0;
  res->premium_required_for_pm_ = (var0 & 2) != 0;
  res->user_id_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void missingInvitee::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "missingInvitee");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (premium_would_allow_invite_ << 0) | (premium_required_for_pm_ << 1)));
    if (var0 & 1) { s.store_field("premium_would_allow_invite", true); }
    if (var0 & 2) { s.store_field("premium_required_for_pm", true); }
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

object_ptr<PageBlock> PageBlock::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case pageBlockUnsupported::ID:
      return pageBlockUnsupported::fetch(p);
    case pageBlockTitle::ID:
      return pageBlockTitle::fetch(p);
    case pageBlockSubtitle::ID:
      return pageBlockSubtitle::fetch(p);
    case pageBlockAuthorDate::ID:
      return pageBlockAuthorDate::fetch(p);
    case pageBlockHeader::ID:
      return pageBlockHeader::fetch(p);
    case pageBlockSubheader::ID:
      return pageBlockSubheader::fetch(p);
    case pageBlockParagraph::ID:
      return pageBlockParagraph::fetch(p);
    case pageBlockPreformatted::ID:
      return pageBlockPreformatted::fetch(p);
    case pageBlockFooter::ID:
      return pageBlockFooter::fetch(p);
    case pageBlockDivider::ID:
      return pageBlockDivider::fetch(p);
    case pageBlockAnchor::ID:
      return pageBlockAnchor::fetch(p);
    case pageBlockList::ID:
      return pageBlockList::fetch(p);
    case pageBlockBlockquote::ID:
      return pageBlockBlockquote::fetch(p);
    case pageBlockPullquote::ID:
      return pageBlockPullquote::fetch(p);
    case pageBlockPhoto::ID:
      return pageBlockPhoto::fetch(p);
    case pageBlockVideo::ID:
      return pageBlockVideo::fetch(p);
    case pageBlockCover::ID:
      return pageBlockCover::fetch(p);
    case pageBlockEmbed::ID:
      return pageBlockEmbed::fetch(p);
    case pageBlockEmbedPost::ID:
      return pageBlockEmbedPost::fetch(p);
    case pageBlockCollage::ID:
      return pageBlockCollage::fetch(p);
    case pageBlockSlideshow::ID:
      return pageBlockSlideshow::fetch(p);
    case pageBlockChannel::ID:
      return pageBlockChannel::fetch(p);
    case pageBlockAudio::ID:
      return pageBlockAudio::fetch(p);
    case pageBlockKicker::ID:
      return pageBlockKicker::fetch(p);
    case pageBlockTable::ID:
      return pageBlockTable::fetch(p);
    case pageBlockOrderedList::ID:
      return pageBlockOrderedList::fetch(p);
    case pageBlockDetails::ID:
      return pageBlockDetails::fetch(p);
    case pageBlockRelatedArticles::ID:
      return pageBlockRelatedArticles::fetch(p);
    case pageBlockMap::ID:
      return pageBlockMap::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t pageBlockUnsupported::ID;

object_ptr<PageBlock> pageBlockUnsupported::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockUnsupported>();
}

void pageBlockUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockUnsupported");
    s.store_class_end();
  }
}

const std::int32_t pageBlockTitle::ID;

object_ptr<PageBlock> pageBlockTitle::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockTitle>(p);
}

pageBlockTitle::pageBlockTitle(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockTitle");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockSubtitle::ID;

object_ptr<PageBlock> pageBlockSubtitle::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockSubtitle>(p);
}

pageBlockSubtitle::pageBlockSubtitle(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockSubtitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockSubtitle");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockAuthorDate::ID;

object_ptr<PageBlock> pageBlockAuthorDate::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockAuthorDate>(p);
}

pageBlockAuthorDate::pageBlockAuthorDate(TlBufferParser &p)
  : author_(TlFetchObject<RichText>::parse(p))
  , published_date_(TlFetchInt::parse(p))
{}

void pageBlockAuthorDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockAuthorDate");
    s.store_object_field("author", static_cast<const BaseObject *>(author_.get()));
    s.store_field("published_date", published_date_);
    s.store_class_end();
  }
}

const std::int32_t pageBlockHeader::ID;

object_ptr<PageBlock> pageBlockHeader::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockHeader>(p);
}

pageBlockHeader::pageBlockHeader(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockHeader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockHeader");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockSubheader::ID;

object_ptr<PageBlock> pageBlockSubheader::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockSubheader>(p);
}

pageBlockSubheader::pageBlockSubheader(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockSubheader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockSubheader");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockParagraph::ID;

object_ptr<PageBlock> pageBlockParagraph::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockParagraph>(p);
}

pageBlockParagraph::pageBlockParagraph(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockParagraph::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockParagraph");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockPreformatted::ID;

object_ptr<PageBlock> pageBlockPreformatted::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockPreformatted>(p);
}

pageBlockPreformatted::pageBlockPreformatted(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , language_(TlFetchString<string>::parse(p))
{}

void pageBlockPreformatted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockPreformatted");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("language", language_);
    s.store_class_end();
  }
}

const std::int32_t pageBlockFooter::ID;

object_ptr<PageBlock> pageBlockFooter::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockFooter>(p);
}

pageBlockFooter::pageBlockFooter(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockFooter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockFooter");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockDivider::ID;

object_ptr<PageBlock> pageBlockDivider::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockDivider>();
}

void pageBlockDivider::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockDivider");
    s.store_class_end();
  }
}

const std::int32_t pageBlockAnchor::ID;

object_ptr<PageBlock> pageBlockAnchor::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockAnchor>(p);
}

pageBlockAnchor::pageBlockAnchor(TlBufferParser &p)
  : name_(TlFetchString<string>::parse(p))
{}

void pageBlockAnchor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockAnchor");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

const std::int32_t pageBlockList::ID;

object_ptr<PageBlock> pageBlockList::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockList>(p);
}

pageBlockList::pageBlockList(TlBufferParser &p)
  : items_(TlFetchBoxed<TlFetchVector<TlFetchObject<PageListItem>>, 481674261>::parse(p))
{}

void pageBlockList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockList");
    { s.store_vector_begin("items", items_.size()); for (const auto &_value : items_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t pageBlockBlockquote::ID;

object_ptr<PageBlock> pageBlockBlockquote::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockBlockquote>(p);
}

pageBlockBlockquote::pageBlockBlockquote(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , caption_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockBlockquote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockBlockquote");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockPullquote::ID;

object_ptr<PageBlock> pageBlockPullquote::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockPullquote>(p);
}

pageBlockPullquote::pageBlockPullquote(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
  , caption_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockPullquote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockPullquote");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

pageBlockPhoto::pageBlockPhoto()
  : flags_()
  , photo_id_()
  , caption_()
  , url_()
  , webpage_id_()
{}

const std::int32_t pageBlockPhoto::ID;

object_ptr<PageBlock> pageBlockPhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pageBlockPhoto> res = make_tl_object<pageBlockPhoto>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->photo_id_ = TlFetchLong::parse(p);
  res->caption_ = TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p);
  if (var0 & 1) { res->url_ = TlFetchString<string>::parse(p); }
  if (var0 & 1) { res->webpage_id_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void pageBlockPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockPhoto");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("photo_id", photo_id_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    if (var0 & 1) { s.store_field("url", url_); }
    if (var0 & 1) { s.store_field("webpage_id", webpage_id_); }
    s.store_class_end();
  }
}

pageBlockVideo::pageBlockVideo()
  : flags_()
  , autoplay_()
  , loop_()
  , video_id_()
  , caption_()
{}

const std::int32_t pageBlockVideo::ID;

object_ptr<PageBlock> pageBlockVideo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pageBlockVideo> res = make_tl_object<pageBlockVideo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->autoplay_ = (var0 & 1) != 0;
  res->loop_ = (var0 & 2) != 0;
  res->video_id_ = TlFetchLong::parse(p);
  res->caption_ = TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void pageBlockVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockVideo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (autoplay_ << 0) | (loop_ << 1)));
    if (var0 & 1) { s.store_field("autoplay", true); }
    if (var0 & 2) { s.store_field("loop", true); }
    s.store_field("video_id", video_id_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockCover::ID;

object_ptr<PageBlock> pageBlockCover::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockCover>(p);
}

pageBlockCover::pageBlockCover(TlBufferParser &p)
  : cover_(TlFetchObject<PageBlock>::parse(p))
{}

void pageBlockCover::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockCover");
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_class_end();
  }
}

pageBlockEmbed::pageBlockEmbed()
  : flags_()
  , full_width_()
  , allow_scrolling_()
  , url_()
  , html_()
  , poster_photo_id_()
  , w_()
  , h_()
  , caption_()
{}

const std::int32_t pageBlockEmbed::ID;

object_ptr<PageBlock> pageBlockEmbed::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pageBlockEmbed> res = make_tl_object<pageBlockEmbed>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->full_width_ = (var0 & 1) != 0;
  res->allow_scrolling_ = (var0 & 8) != 0;
  if (var0 & 2) { res->url_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->html_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->poster_photo_id_ = TlFetchLong::parse(p); }
  if (var0 & 32) { res->w_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->h_ = TlFetchInt::parse(p); }
  res->caption_ = TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void pageBlockEmbed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockEmbed");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (full_width_ << 0) | (allow_scrolling_ << 3)));
    if (var0 & 1) { s.store_field("full_width", true); }
    if (var0 & 8) { s.store_field("allow_scrolling", true); }
    if (var0 & 2) { s.store_field("url", url_); }
    if (var0 & 4) { s.store_field("html", html_); }
    if (var0 & 16) { s.store_field("poster_photo_id", poster_photo_id_); }
    if (var0 & 32) { s.store_field("w", w_); }
    if (var0 & 32) { s.store_field("h", h_); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockEmbedPost::ID;

object_ptr<PageBlock> pageBlockEmbedPost::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockEmbedPost>(p);
}

pageBlockEmbedPost::pageBlockEmbedPost(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , webpage_id_(TlFetchLong::parse(p))
  , author_photo_id_(TlFetchLong::parse(p))
  , author_(TlFetchString<string>::parse(p))
  , date_(TlFetchInt::parse(p))
  , blocks_(TlFetchBoxed<TlFetchVector<TlFetchObject<PageBlock>>, 481674261>::parse(p))
  , caption_(TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p))
{}

void pageBlockEmbedPost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockEmbedPost");
    s.store_field("url", url_);
    s.store_field("webpage_id", webpage_id_);
    s.store_field("author_photo_id", author_photo_id_);
    s.store_field("author", author_);
    s.store_field("date", date_);
    { s.store_vector_begin("blocks", blocks_.size()); for (const auto &_value : blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockCollage::ID;

object_ptr<PageBlock> pageBlockCollage::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockCollage>(p);
}

pageBlockCollage::pageBlockCollage(TlBufferParser &p)
  : items_(TlFetchBoxed<TlFetchVector<TlFetchObject<PageBlock>>, 481674261>::parse(p))
  , caption_(TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p))
{}

void pageBlockCollage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockCollage");
    { s.store_vector_begin("items", items_.size()); for (const auto &_value : items_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockSlideshow::ID;

object_ptr<PageBlock> pageBlockSlideshow::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockSlideshow>(p);
}

pageBlockSlideshow::pageBlockSlideshow(TlBufferParser &p)
  : items_(TlFetchBoxed<TlFetchVector<TlFetchObject<PageBlock>>, 481674261>::parse(p))
  , caption_(TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p))
{}

void pageBlockSlideshow::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockSlideshow");
    { s.store_vector_begin("items", items_.size()); for (const auto &_value : items_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockChannel::ID;

object_ptr<PageBlock> pageBlockChannel::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockChannel>(p);
}

pageBlockChannel::pageBlockChannel(TlBufferParser &p)
  : channel_(TlFetchObject<Chat>::parse(p))
{}

void pageBlockChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockChannel");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockAudio::ID;

object_ptr<PageBlock> pageBlockAudio::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockAudio>(p);
}

pageBlockAudio::pageBlockAudio(TlBufferParser &p)
  : audio_id_(TlFetchLong::parse(p))
  , caption_(TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p))
{}

void pageBlockAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockAudio");
    s.store_field("audio_id", audio_id_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockKicker::ID;

object_ptr<PageBlock> pageBlockKicker::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockKicker>(p);
}

pageBlockKicker::pageBlockKicker(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageBlockKicker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockKicker");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

pageBlockTable::pageBlockTable()
  : flags_()
  , bordered_()
  , striped_()
  , title_()
  , rows_()
{}

const std::int32_t pageBlockTable::ID;

object_ptr<PageBlock> pageBlockTable::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pageBlockTable> res = make_tl_object<pageBlockTable>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->bordered_ = (var0 & 1) != 0;
  res->striped_ = (var0 & 2) != 0;
  res->title_ = TlFetchObject<RichText>::parse(p);
  res->rows_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<pageTableRow>, -524237339>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void pageBlockTable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockTable");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (bordered_ << 0) | (striped_ << 1)));
    if (var0 & 1) { s.store_field("bordered", true); }
    if (var0 & 2) { s.store_field("striped", true); }
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    { s.store_vector_begin("rows", rows_.size()); for (const auto &_value : rows_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t pageBlockOrderedList::ID;

object_ptr<PageBlock> pageBlockOrderedList::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockOrderedList>(p);
}

pageBlockOrderedList::pageBlockOrderedList(TlBufferParser &p)
  : items_(TlFetchBoxed<TlFetchVector<TlFetchObject<PageListOrderedItem>>, 481674261>::parse(p))
{}

void pageBlockOrderedList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockOrderedList");
    { s.store_vector_begin("items", items_.size()); for (const auto &_value : items_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pageBlockDetails::pageBlockDetails()
  : flags_()
  , open_()
  , blocks_()
  , title_()
{}

const std::int32_t pageBlockDetails::ID;

object_ptr<PageBlock> pageBlockDetails::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pageBlockDetails> res = make_tl_object<pageBlockDetails>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->open_ = (var0 & 1) != 0;
  res->blocks_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PageBlock>>, 481674261>::parse(p);
  res->title_ = TlFetchObject<RichText>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void pageBlockDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockDetails");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (open_ << 0)));
    if (var0 & 1) { s.store_field("open", true); }
    { s.store_vector_begin("blocks", blocks_.size()); for (const auto &_value : blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageBlockRelatedArticles::ID;

object_ptr<PageBlock> pageBlockRelatedArticles::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockRelatedArticles>(p);
}

pageBlockRelatedArticles::pageBlockRelatedArticles(TlBufferParser &p)
  : title_(TlFetchObject<RichText>::parse(p))
  , articles_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<pageRelatedArticle>, -1282352120>>, 481674261>::parse(p))
{}

void pageBlockRelatedArticles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockRelatedArticles");
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    { s.store_vector_begin("articles", articles_.size()); for (const auto &_value : articles_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t pageBlockMap::ID;

object_ptr<PageBlock> pageBlockMap::fetch(TlBufferParser &p) {
  return make_tl_object<pageBlockMap>(p);
}

pageBlockMap::pageBlockMap(TlBufferParser &p)
  : geo_(TlFetchObject<GeoPoint>::parse(p))
  , zoom_(TlFetchInt::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , caption_(TlFetchBoxed<TlFetchObject<pageCaption>, 1869903447>::parse(p))
{}

void pageBlockMap::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockMap");
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    s.store_field("zoom", zoom_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonSpam::ID;

void inputReportReasonSpam::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonSpam::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonSpam");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonViolence::ID;

void inputReportReasonViolence::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonViolence::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonViolence::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonViolence");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonPornography::ID;

void inputReportReasonPornography::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonPornography::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonPornography::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonPornography");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonChildAbuse::ID;

void inputReportReasonChildAbuse::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonChildAbuse::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonChildAbuse::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonChildAbuse");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonOther::ID;

void inputReportReasonOther::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonOther::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonOther::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonOther");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonCopyright::ID;

void inputReportReasonCopyright::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonCopyright::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonCopyright::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonCopyright");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonGeoIrrelevant::ID;

void inputReportReasonGeoIrrelevant::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonGeoIrrelevant::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonGeoIrrelevant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonGeoIrrelevant");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonFake::ID;

void inputReportReasonFake::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonFake::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonFake::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonFake");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonIllegalDrugs::ID;

void inputReportReasonIllegalDrugs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonIllegalDrugs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonIllegalDrugs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonIllegalDrugs");
    s.store_class_end();
  }
}

const std::int32_t inputReportReasonPersonalDetails::ID;

void inputReportReasonPersonalDetails::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputReportReasonPersonalDetails::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputReportReasonPersonalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReportReasonPersonalDetails");
    s.store_class_end();
  }
}

savedReactionTag::savedReactionTag()
  : flags_()
  , reaction_()
  , title_()
  , count_()
{}

const std::int32_t savedReactionTag::ID;

object_ptr<savedReactionTag> savedReactionTag::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<savedReactionTag> res = make_tl_object<savedReactionTag>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->reaction_ = TlFetchObject<Reaction>::parse(p);
  if (var0 & 1) { res->title_ = TlFetchString<string>::parse(p); }
  res->count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void savedReactionTag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedReactionTag");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    if (var0 & 1) { s.store_field("title", title_); }
    s.store_field("count", count_);
    s.store_class_end();
  }
}

secureValue::secureValue()
  : flags_()
  , type_()
  , data_()
  , front_side_()
  , reverse_side_()
  , selfie_()
  , translation_()
  , files_()
  , plain_data_()
  , hash_()
{}

const std::int32_t secureValue::ID;

object_ptr<secureValue> secureValue::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<secureValue> res = make_tl_object<secureValue>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->type_ = TlFetchObject<SecureValueType>::parse(p);
  if (var0 & 1) { res->data_ = TlFetchBoxed<TlFetchObject<secureData>, -1964327229>::parse(p); }
  if (var0 & 2) { res->front_side_ = TlFetchObject<SecureFile>::parse(p); }
  if (var0 & 4) { res->reverse_side_ = TlFetchObject<SecureFile>::parse(p); }
  if (var0 & 8) { res->selfie_ = TlFetchObject<SecureFile>::parse(p); }
  if (var0 & 64) { res->translation_ = TlFetchBoxed<TlFetchVector<TlFetchObject<SecureFile>>, 481674261>::parse(p); }
  if (var0 & 16) { res->files_ = TlFetchBoxed<TlFetchVector<TlFetchObject<SecureFile>>, 481674261>::parse(p); }
  if (var0 & 32) { res->plain_data_ = TlFetchObject<SecurePlainData>::parse(p); }
  res->hash_ = TlFetchBytes<bytes>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void secureValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValue");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    if (var0 & 1) { s.store_object_field("data", static_cast<const BaseObject *>(data_.get())); }
    if (var0 & 2) { s.store_object_field("front_side", static_cast<const BaseObject *>(front_side_.get())); }
    if (var0 & 4) { s.store_object_field("reverse_side", static_cast<const BaseObject *>(reverse_side_.get())); }
    if (var0 & 8) { s.store_object_field("selfie", static_cast<const BaseObject *>(selfie_.get())); }
    if (var0 & 64) { { s.store_vector_begin("translation", translation_.size()); for (const auto &_value : translation_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 16) { { s.store_vector_begin("files", files_.size()); for (const auto &_value : files_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 32) { s.store_object_field("plain_data", static_cast<const BaseObject *>(plain_data_.get())); }
    s.store_bytes_field("hash", hash_);
    s.store_class_end();
  }
}

secureValueHash::secureValueHash(object_ptr<SecureValueType> &&type_, bytes &&hash_)
  : type_(std::move(type_))
  , hash_(std::move(hash_))
{}

const std::int32_t secureValueHash::ID;

void secureValueHash::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(hash_, s);
}

void secureValueHash::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(hash_, s);
}

void secureValueHash::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueHash");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("hash", hash_);
    s.store_class_end();
  }
}

object_ptr<StarGift> StarGift::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case starGift::ID:
      return starGift::fetch(p);
    case starGiftUnique::ID:
      return starGiftUnique::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

starGift::starGift()
  : flags_()
  , limited_()
  , sold_out_()
  , birthday_()
  , require_premium_()
  , limited_per_user_()
  , peer_color_available_()
  , id_()
  , sticker_()
  , stars_()
  , availability_remains_()
  , availability_total_()
  , availability_resale_()
  , convert_stars_()
  , first_sale_date_()
  , last_sale_date_()
  , upgrade_stars_()
  , resell_min_stars_()
  , title_()
  , released_by_()
  , per_user_total_()
  , per_user_remains_()
  , locked_until_date_()
{}

const std::int32_t starGift::ID;

object_ptr<StarGift> starGift::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starGift> res = make_tl_object<starGift>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->limited_ = (var0 & 1) != 0;
  res->sold_out_ = (var0 & 2) != 0;
  res->birthday_ = (var0 & 4) != 0;
  res->require_premium_ = (var0 & 128) != 0;
  res->limited_per_user_ = (var0 & 256) != 0;
  res->peer_color_available_ = (var0 & 1024) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->sticker_ = TlFetchObject<Document>::parse(p);
  res->stars_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->availability_remains_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->availability_total_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->availability_resale_ = TlFetchLong::parse(p); }
  res->convert_stars_ = TlFetchLong::parse(p);
  if (var0 & 2) { res->first_sale_date_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->last_sale_date_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->upgrade_stars_ = TlFetchLong::parse(p); }
  if (var0 & 16) { res->resell_min_stars_ = TlFetchLong::parse(p); }
  if (var0 & 32) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 64) { res->released_by_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 256) { res->per_user_total_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->per_user_remains_ = TlFetchInt::parse(p); }
  if (var0 & 512) { res->locked_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void starGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGift");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (limited_ << 0) | (sold_out_ << 1) | (birthday_ << 2) | (require_premium_ << 7) | (limited_per_user_ << 8) | (peer_color_available_ << 10)));
    if (var0 & 1) { s.store_field("limited", true); }
    if (var0 & 2) { s.store_field("sold_out", true); }
    if (var0 & 4) { s.store_field("birthday", true); }
    if (var0 & 128) { s.store_field("require_premium", true); }
    if (var0 & 256) { s.store_field("limited_per_user", true); }
    if (var0 & 1024) { s.store_field("peer_color_available", true); }
    s.store_field("id", id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("stars", stars_);
    if (var0 & 1) { s.store_field("availability_remains", availability_remains_); }
    if (var0 & 1) { s.store_field("availability_total", availability_total_); }
    if (var0 & 16) { s.store_field("availability_resale", availability_resale_); }
    s.store_field("convert_stars", convert_stars_);
    if (var0 & 2) { s.store_field("first_sale_date", first_sale_date_); }
    if (var0 & 2) { s.store_field("last_sale_date", last_sale_date_); }
    if (var0 & 8) { s.store_field("upgrade_stars", upgrade_stars_); }
    if (var0 & 16) { s.store_field("resell_min_stars", resell_min_stars_); }
    if (var0 & 32) { s.store_field("title", title_); }
    if (var0 & 64) { s.store_object_field("released_by", static_cast<const BaseObject *>(released_by_.get())); }
    if (var0 & 256) { s.store_field("per_user_total", per_user_total_); }
    if (var0 & 256) { s.store_field("per_user_remains", per_user_remains_); }
    if (var0 & 512) { s.store_field("locked_until_date", locked_until_date_); }
    s.store_class_end();
  }
}

starGiftUnique::starGiftUnique()
  : flags_()
  , require_premium_()
  , resale_ton_only_()
  , theme_available_()
  , id_()
  , gift_id_()
  , title_()
  , slug_()
  , num_()
  , owner_id_()
  , owner_name_()
  , owner_address_()
  , attributes_()
  , availability_issued_()
  , availability_total_()
  , gift_address_()
  , resell_amount_()
  , released_by_()
  , value_amount_()
  , value_currency_()
  , theme_peer_()
  , peer_color_()
  , host_id_()
{}

const std::int32_t starGiftUnique::ID;

object_ptr<StarGift> starGiftUnique::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starGiftUnique> res = make_tl_object<starGiftUnique>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->require_premium_ = (var0 & 64) != 0;
  res->resale_ton_only_ = (var0 & 128) != 0;
  res->theme_available_ = (var0 & 512) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->gift_id_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->slug_ = TlFetchString<string>::parse(p);
  res->num_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->owner_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 2) { res->owner_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->owner_address_ = TlFetchString<string>::parse(p); }
  res->attributes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StarGiftAttribute>>, 481674261>::parse(p);
  res->availability_issued_ = TlFetchInt::parse(p);
  res->availability_total_ = TlFetchInt::parse(p);
  if (var0 & 8) { res->gift_address_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->resell_amount_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StarsAmount>>, 481674261>::parse(p); }
  if (var0 & 32) { res->released_by_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 256) { res->value_amount_ = TlFetchLong::parse(p); }
  if (var0 & 256) { res->value_currency_ = TlFetchString<string>::parse(p); }
  if (var0 & 1024) { res->theme_peer_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 2048) { res->peer_color_ = TlFetchObject<PeerColor>::parse(p); }
  if (var0 & 4096) { res->host_id_ = TlFetchObject<Peer>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void starGiftUnique::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftUnique");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (require_premium_ << 6) | (resale_ton_only_ << 7) | (theme_available_ << 9)));
    if (var0 & 64) { s.store_field("require_premium", true); }
    if (var0 & 128) { s.store_field("resale_ton_only", true); }
    if (var0 & 512) { s.store_field("theme_available", true); }
    s.store_field("id", id_);
    s.store_field("gift_id", gift_id_);
    s.store_field("title", title_);
    s.store_field("slug", slug_);
    s.store_field("num", num_);
    if (var0 & 1) { s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get())); }
    if (var0 & 2) { s.store_field("owner_name", owner_name_); }
    if (var0 & 4) { s.store_field("owner_address", owner_address_); }
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("availability_issued", availability_issued_);
    s.store_field("availability_total", availability_total_);
    if (var0 & 8) { s.store_field("gift_address", gift_address_); }
    if (var0 & 16) { { s.store_vector_begin("resell_amount", resell_amount_.size()); for (const auto &_value : resell_amount_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 32) { s.store_object_field("released_by", static_cast<const BaseObject *>(released_by_.get())); }
    if (var0 & 256) { s.store_field("value_amount", value_amount_); }
    if (var0 & 256) { s.store_field("value_currency", value_currency_); }
    if (var0 & 1024) { s.store_object_field("theme_peer", static_cast<const BaseObject *>(theme_peer_.get())); }
    if (var0 & 2048) { s.store_object_field("peer_color", static_cast<const BaseObject *>(peer_color_.get())); }
    if (var0 & 4096) { s.store_object_field("host_id", static_cast<const BaseObject *>(host_id_.get())); }
    s.store_class_end();
  }
}

const std::int32_t starGiftAttributeCounter::ID;

object_ptr<starGiftAttributeCounter> starGiftAttributeCounter::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftAttributeCounter>(p);
}

starGiftAttributeCounter::starGiftAttributeCounter(TlBufferParser &p)
  : attribute_(TlFetchObject<StarGiftAttributeId>::parse(p))
  , count_(TlFetchInt::parse(p))
{}

void starGiftAttributeCounter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributeCounter");
    s.store_object_field("attribute", static_cast<const BaseObject *>(attribute_.get()));
    s.store_field("count", count_);
    s.store_class_end();
  }
}

const std::int32_t stickerKeyword::ID;

object_ptr<stickerKeyword> stickerKeyword::fetch(TlBufferParser &p) {
  return make_tl_object<stickerKeyword>(p);
}

stickerKeyword::stickerKeyword(TlBufferParser &p)
  : document_id_(TlFetchLong::parse(p))
  , keyword_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
{}

void stickerKeyword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerKeyword");
    s.store_field("document_id", document_id_);
    { s.store_vector_begin("keyword", keyword_.size()); for (const auto &_value : keyword_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

themeSettings::themeSettings()
  : flags_()
  , message_colors_animated_()
  , base_theme_()
  , accent_color_()
  , outbox_accent_color_()
  , message_colors_()
  , wallpaper_()
{}

const std::int32_t themeSettings::ID;

object_ptr<themeSettings> themeSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<themeSettings> res = make_tl_object<themeSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->message_colors_animated_ = (var0 & 4) != 0;
  res->base_theme_ = TlFetchObject<BaseTheme>::parse(p);
  res->accent_color_ = TlFetchInt::parse(p);
  if (var0 & 8) { res->outbox_accent_color_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->message_colors_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  if (var0 & 2) { res->wallpaper_ = TlFetchObject<WallPaper>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void themeSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "themeSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (message_colors_animated_ << 2)));
    if (var0 & 4) { s.store_field("message_colors_animated", true); }
    s.store_object_field("base_theme", static_cast<const BaseObject *>(base_theme_.get()));
    s.store_field("accent_color", accent_color_);
    if (var0 & 8) { s.store_field("outbox_accent_color", outbox_accent_color_); }
    if (var0 & 1) { { s.store_vector_begin("message_colors", message_colors_.size()); for (const auto &_value : message_colors_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 2) { s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get())); }
    s.store_class_end();
  }
}

const std::int32_t account_authorizations::ID;

object_ptr<account_authorizations> account_authorizations::fetch(TlBufferParser &p) {
  return make_tl_object<account_authorizations>(p);
}

account_authorizations::account_authorizations(TlBufferParser &p)
  : authorization_ttl_days_(TlFetchInt::parse(p))
  , authorizations_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<authorization>, -1392388579>>, 481674261>::parse(p))
{}

void account_authorizations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.authorizations");
    s.store_field("authorization_ttl_days", authorization_ttl_days_);
    { s.store_vector_begin("authorizations", authorizations_.size()); for (const auto &_value : authorizations_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<account_ChatThemes> account_ChatThemes::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_chatThemesNotModified::ID:
      return account_chatThemesNotModified::fetch(p);
    case account_chatThemes::ID:
      return account_chatThemes::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_chatThemesNotModified::ID;

object_ptr<account_ChatThemes> account_chatThemesNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<account_chatThemesNotModified>();
}

void account_chatThemesNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.chatThemesNotModified");
    s.store_class_end();
  }
}

account_chatThemes::account_chatThemes()
  : flags_()
  , hash_()
  , themes_()
  , chats_()
  , users_()
  , next_offset_()
{}

const std::int32_t account_chatThemes::ID;

object_ptr<account_ChatThemes> account_chatThemes::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<account_chatThemes> res = make_tl_object<account_chatThemes>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->hash_ = TlFetchLong::parse(p);
  res->themes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<ChatTheme>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void account_chatThemes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.chatThemes");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("hash", hash_);
    { s.store_vector_begin("themes", themes_.size()); for (const auto &_value : themes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    s.store_class_end();
  }
}

object_ptr<auth_CodeType> auth_CodeType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case auth_codeTypeSms::ID:
      return auth_codeTypeSms::fetch(p);
    case auth_codeTypeCall::ID:
      return auth_codeTypeCall::fetch(p);
    case auth_codeTypeFlashCall::ID:
      return auth_codeTypeFlashCall::fetch(p);
    case auth_codeTypeMissedCall::ID:
      return auth_codeTypeMissedCall::fetch(p);
    case auth_codeTypeFragmentSms::ID:
      return auth_codeTypeFragmentSms::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t auth_codeTypeSms::ID;

object_ptr<auth_CodeType> auth_codeTypeSms::fetch(TlBufferParser &p) {
  return make_tl_object<auth_codeTypeSms>();
}

void auth_codeTypeSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.codeTypeSms");
    s.store_class_end();
  }
}

const std::int32_t auth_codeTypeCall::ID;

object_ptr<auth_CodeType> auth_codeTypeCall::fetch(TlBufferParser &p) {
  return make_tl_object<auth_codeTypeCall>();
}

void auth_codeTypeCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.codeTypeCall");
    s.store_class_end();
  }
}

const std::int32_t auth_codeTypeFlashCall::ID;

object_ptr<auth_CodeType> auth_codeTypeFlashCall::fetch(TlBufferParser &p) {
  return make_tl_object<auth_codeTypeFlashCall>();
}

void auth_codeTypeFlashCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.codeTypeFlashCall");
    s.store_class_end();
  }
}

const std::int32_t auth_codeTypeMissedCall::ID;

object_ptr<auth_CodeType> auth_codeTypeMissedCall::fetch(TlBufferParser &p) {
  return make_tl_object<auth_codeTypeMissedCall>();
}

void auth_codeTypeMissedCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.codeTypeMissedCall");
    s.store_class_end();
  }
}

const std::int32_t auth_codeTypeFragmentSms::ID;

object_ptr<auth_CodeType> auth_codeTypeFragmentSms::fetch(TlBufferParser &p) {
  return make_tl_object<auth_codeTypeFragmentSms>();
}

void auth_codeTypeFragmentSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.codeTypeFragmentSms");
    s.store_class_end();
  }
}

const std::int32_t channels_adminLogResults::ID;

object_ptr<channels_adminLogResults> channels_adminLogResults::fetch(TlBufferParser &p) {
  return make_tl_object<channels_adminLogResults>(p);
}

channels_adminLogResults::channels_adminLogResults(TlBufferParser &p)
  : events_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<channelAdminLogEvent>, 531458253>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void channels_adminLogResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.adminLogResults");
    { s.store_vector_begin("events", events_.size()); for (const auto &_value : events_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<channels_ChannelParticipants> channels_ChannelParticipants::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case channels_channelParticipants::ID:
      return channels_channelParticipants::fetch(p);
    case channels_channelParticipantsNotModified::ID:
      return channels_channelParticipantsNotModified::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t channels_channelParticipants::ID;

object_ptr<channels_ChannelParticipants> channels_channelParticipants::fetch(TlBufferParser &p) {
  return make_tl_object<channels_channelParticipants>(p);
}

channels_channelParticipants::channels_channelParticipants(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , participants_(TlFetchBoxed<TlFetchVector<TlFetchObject<ChannelParticipant>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void channels_channelParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.channelParticipants");
    s.store_field("count", count_);
    { s.store_vector_begin("participants", participants_.size()); for (const auto &_value : participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t channels_channelParticipantsNotModified::ID;

object_ptr<channels_ChannelParticipants> channels_channelParticipantsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<channels_channelParticipantsNotModified>();
}

void channels_channelParticipantsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.channelParticipantsNotModified");
    s.store_class_end();
  }
}

object_ptr<contacts_Blocked> contacts_Blocked::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case contacts_blocked::ID:
      return contacts_blocked::fetch(p);
    case contacts_blockedSlice::ID:
      return contacts_blockedSlice::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t contacts_blocked::ID;

object_ptr<contacts_Blocked> contacts_blocked::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_blocked>(p);
}

contacts_blocked::contacts_blocked(TlBufferParser &p)
  : blocked_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<peerBlocked>, -386039788>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_blocked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.blocked");
    { s.store_vector_begin("blocked", blocked_.size()); for (const auto &_value : blocked_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t contacts_blockedSlice::ID;

object_ptr<contacts_Blocked> contacts_blockedSlice::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_blockedSlice>(p);
}

contacts_blockedSlice::contacts_blockedSlice(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , blocked_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<peerBlocked>, -386039788>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_blockedSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.blockedSlice");
    s.store_field("count", count_);
    { s.store_vector_begin("blocked", blocked_.size()); for (const auto &_value : blocked_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

contacts_contactBirthdays::contacts_contactBirthdays(array<object_ptr<contactBirthday>> &&contacts_, array<object_ptr<User>> &&users_)
  : contacts_(std::move(contacts_))
  , users_(std::move(users_))
{}

const std::int32_t contacts_contactBirthdays::ID;

object_ptr<contacts_contactBirthdays> contacts_contactBirthdays::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_contactBirthdays>(p);
}

contacts_contactBirthdays::contacts_contactBirthdays(TlBufferParser &p)
  : contacts_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<contactBirthday>, 496600883>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_contactBirthdays::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.contactBirthdays");
    { s.store_vector_begin("contacts", contacts_.size()); for (const auto &_value : contacts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t contacts_resolvedPeer::ID;

object_ptr<contacts_resolvedPeer> contacts_resolvedPeer::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_resolvedPeer>(p);
}

contacts_resolvedPeer::contacts_resolvedPeer(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_resolvedPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.resolvedPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<help_DeepLinkInfo> help_DeepLinkInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_deepLinkInfoEmpty::ID:
      return help_deepLinkInfoEmpty::fetch(p);
    case help_deepLinkInfo::ID:
      return help_deepLinkInfo::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_deepLinkInfoEmpty::ID;

object_ptr<help_DeepLinkInfo> help_deepLinkInfoEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<help_deepLinkInfoEmpty>();
}

void help_deepLinkInfoEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.deepLinkInfoEmpty");
    s.store_class_end();
  }
}

help_deepLinkInfo::help_deepLinkInfo()
  : flags_()
  , update_app_()
  , message_()
  , entities_()
{}

const std::int32_t help_deepLinkInfo::ID;

object_ptr<help_DeepLinkInfo> help_deepLinkInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<help_deepLinkInfo> res = make_tl_object<help_deepLinkInfo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->update_app_ = (var0 & 1) != 0;
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void help_deepLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.deepLinkInfo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (update_app_ << 0)));
    if (var0 & 1) { s.store_field("update_app", true); }
    s.store_field("message", message_);
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

const std::int32_t messages_chatFull::ID;

object_ptr<messages_chatFull> messages_chatFull::fetch(TlBufferParser &p) {
  return make_tl_object<messages_chatFull>(p);
}

messages_chatFull::messages_chatFull(TlBufferParser &p)
  : full_chat_(TlFetchObject<ChatFull>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_chatFull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.chatFull");
    s.store_object_field("full_chat", static_cast<const BaseObject *>(full_chat_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_historyImport::ID;

object_ptr<messages_historyImport> messages_historyImport::fetch(TlBufferParser &p) {
  return make_tl_object<messages_historyImport>(p);
}

messages_historyImport::messages_historyImport(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void messages_historyImport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.historyImport");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

const std::int32_t messages_messageViews::ID;

object_ptr<messages_messageViews> messages_messageViews::fetch(TlBufferParser &p) {
  return make_tl_object<messages_messageViews>(p);
}

messages_messageViews::messages_messageViews(TlBufferParser &p)
  : views_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<messageViews>, 1163625789>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_messageViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.messageViews");
    { s.store_vector_begin("views", views_.size()); for (const auto &_value : views_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_preparedInlineMessage::ID;

object_ptr<messages_preparedInlineMessage> messages_preparedInlineMessage::fetch(TlBufferParser &p) {
  return make_tl_object<messages_preparedInlineMessage>(p);
}

messages_preparedInlineMessage::messages_preparedInlineMessage(TlBufferParser &p)
  : query_id_(TlFetchLong::parse(p))
  , result_(TlFetchObject<BotInlineResult>::parse(p))
  , peer_types_(TlFetchBoxed<TlFetchVector<TlFetchObject<InlineQueryPeerType>>, 481674261>::parse(p))
  , cache_time_(TlFetchInt::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_preparedInlineMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.preparedInlineMessage");
    s.store_field("query_id", query_id_);
    s.store_object_field("result", static_cast<const BaseObject *>(result_.get()));
    { s.store_vector_begin("peer_types", peer_types_.size()); for (const auto &_value : peer_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("cache_time", cache_time_);
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_SavedReactionTags> messages_SavedReactionTags::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_savedReactionTagsNotModified::ID:
      return messages_savedReactionTagsNotModified::fetch(p);
    case messages_savedReactionTags::ID:
      return messages_savedReactionTags::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_savedReactionTagsNotModified::ID;

object_ptr<messages_SavedReactionTags> messages_savedReactionTagsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_savedReactionTagsNotModified>();
}

void messages_savedReactionTagsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savedReactionTagsNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_savedReactionTags::ID;

object_ptr<messages_SavedReactionTags> messages_savedReactionTags::fetch(TlBufferParser &p) {
  return make_tl_object<messages_savedReactionTags>(p);
}

messages_savedReactionTags::messages_savedReactionTags(TlBufferParser &p)
  : tags_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<savedReactionTag>, -881854424>>, 481674261>::parse(p))
  , hash_(TlFetchLong::parse(p))
{}

void messages_savedReactionTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savedReactionTags");
    { s.store_vector_begin("tags", tags_.size()); for (const auto &_value : tags_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

const std::int32_t messages_webPage::ID;

object_ptr<messages_webPage> messages_webPage::fetch(TlBufferParser &p) {
  return make_tl_object<messages_webPage>(p);
}

messages_webPage::messages_webPage(TlBufferParser &p)
  : webpage_(TlFetchObject<WebPage>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_webPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.webPage");
    s.store_object_field("webpage", static_cast<const BaseObject *>(webpage_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_suggestedStarRefBots::payments_suggestedStarRefBots()
  : flags_()
  , count_()
  , suggested_bots_()
  , users_()
  , next_offset_()
{}

const std::int32_t payments_suggestedStarRefBots::ID;

object_ptr<payments_suggestedStarRefBots> payments_suggestedStarRefBots::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_suggestedStarRefBots> res = make_tl_object<payments_suggestedStarRefBots>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->suggested_bots_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starRefProgram>, -586389774>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_suggestedStarRefBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.suggestedStarRefBots");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("suggested_bots", suggested_bots_.size()); for (const auto &_value : suggested_bots_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    s.store_class_end();
  }
}

const std::int32_t phone_groupCall::ID;

object_ptr<phone_groupCall> phone_groupCall::fetch(TlBufferParser &p) {
  return make_tl_object<phone_groupCall>(p);
}

phone_groupCall::phone_groupCall(TlBufferParser &p)
  : call_(TlFetchObject<GroupCall>::parse(p))
  , participants_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<groupCallParticipant>, -341428482>>, 481674261>::parse(p))
  , participants_next_offset_(TlFetchString<string>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void phone_groupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.groupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    { s.store_vector_begin("participants", participants_.size()); for (const auto &_value : participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("participants_next_offset", participants_next_offset_);
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

premium_boostsList::premium_boostsList()
  : flags_()
  , count_()
  , boosts_()
  , next_offset_()
  , users_()
{}

const std::int32_t premium_boostsList::ID;

object_ptr<premium_boostsList> premium_boostsList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<premium_boostsList> res = make_tl_object<premium_boostsList>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->boosts_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<boost>, 1262359766>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void premium_boostsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.boostsList");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("boosts", boosts_.size()); for (const auto &_value : boosts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<storage_FileType> storage_FileType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case storage_fileUnknown::ID:
      return storage_fileUnknown::fetch(p);
    case storage_filePartial::ID:
      return storage_filePartial::fetch(p);
    case storage_fileJpeg::ID:
      return storage_fileJpeg::fetch(p);
    case storage_fileGif::ID:
      return storage_fileGif::fetch(p);
    case storage_filePng::ID:
      return storage_filePng::fetch(p);
    case storage_filePdf::ID:
      return storage_filePdf::fetch(p);
    case storage_fileMp3::ID:
      return storage_fileMp3::fetch(p);
    case storage_fileMov::ID:
      return storage_fileMov::fetch(p);
    case storage_fileMp4::ID:
      return storage_fileMp4::fetch(p);
    case storage_fileWebp::ID:
      return storage_fileWebp::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t storage_fileUnknown::ID;

object_ptr<storage_FileType> storage_fileUnknown::fetch(TlBufferParser &p) {
  return make_tl_object<storage_fileUnknown>();
}

void storage_fileUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.fileUnknown");
    s.store_class_end();
  }
}

const std::int32_t storage_filePartial::ID;

object_ptr<storage_FileType> storage_filePartial::fetch(TlBufferParser &p) {
  return make_tl_object<storage_filePartial>();
}

void storage_filePartial::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.filePartial");
    s.store_class_end();
  }
}

const std::int32_t storage_fileJpeg::ID;

object_ptr<storage_FileType> storage_fileJpeg::fetch(TlBufferParser &p) {
  return make_tl_object<storage_fileJpeg>();
}

void storage_fileJpeg::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.fileJpeg");
    s.store_class_end();
  }
}

const std::int32_t storage_fileGif::ID;

object_ptr<storage_FileType> storage_fileGif::fetch(TlBufferParser &p) {
  return make_tl_object<storage_fileGif>();
}

void storage_fileGif::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.fileGif");
    s.store_class_end();
  }
}

const std::int32_t storage_filePng::ID;

object_ptr<storage_FileType> storage_filePng::fetch(TlBufferParser &p) {
  return make_tl_object<storage_filePng>();
}

void storage_filePng::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.filePng");
    s.store_class_end();
  }
}

const std::int32_t storage_filePdf::ID;

object_ptr<storage_FileType> storage_filePdf::fetch(TlBufferParser &p) {
  return make_tl_object<storage_filePdf>();
}

void storage_filePdf::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.filePdf");
    s.store_class_end();
  }
}

const std::int32_t storage_fileMp3::ID;

object_ptr<storage_FileType> storage_fileMp3::fetch(TlBufferParser &p) {
  return make_tl_object<storage_fileMp3>();
}

void storage_fileMp3::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.fileMp3");
    s.store_class_end();
  }
}

const std::int32_t storage_fileMov::ID;

object_ptr<storage_FileType> storage_fileMov::fetch(TlBufferParser &p) {
  return make_tl_object<storage_fileMov>();
}

void storage_fileMov::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.fileMov");
    s.store_class_end();
  }
}

const std::int32_t storage_fileMp4::ID;

object_ptr<storage_FileType> storage_fileMp4::fetch(TlBufferParser &p) {
  return make_tl_object<storage_fileMp4>();
}

void storage_fileMp4::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.fileMp4");
    s.store_class_end();
  }
}

const std::int32_t storage_fileWebp::ID;

object_ptr<storage_FileType> storage_fileWebp::fetch(TlBufferParser &p) {
  return make_tl_object<storage_fileWebp>();
}

void storage_fileWebp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storage.fileWebp");
    s.store_class_end();
  }
}

object_ptr<stories_AllStories> stories_AllStories::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case stories_allStoriesNotModified::ID:
      return stories_allStoriesNotModified::fetch(p);
    case stories_allStories::ID:
      return stories_allStories::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

stories_allStoriesNotModified::stories_allStoriesNotModified()
  : flags_()
  , state_()
  , stealth_mode_()
{}

const std::int32_t stories_allStoriesNotModified::ID;

object_ptr<stories_AllStories> stories_allStoriesNotModified::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stories_allStoriesNotModified> res = make_tl_object<stories_allStoriesNotModified>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->state_ = TlFetchString<string>::parse(p);
  res->stealth_mode_ = TlFetchBoxed<TlFetchObject<storiesStealthMode>, 1898850301>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void stories_allStoriesNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.allStoriesNotModified");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("state", state_);
    s.store_object_field("stealth_mode", static_cast<const BaseObject *>(stealth_mode_.get()));
    s.store_class_end();
  }
}

stories_allStories::stories_allStories()
  : flags_()
  , has_more_()
  , count_()
  , state_()
  , peer_stories_()
  , chats_()
  , users_()
  , stealth_mode_()
{}

const std::int32_t stories_allStories::ID;

object_ptr<stories_AllStories> stories_allStories::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stories_allStories> res = make_tl_object<stories_allStories>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_more_ = (var0 & 1) != 0;
  res->count_ = TlFetchInt::parse(p);
  res->state_ = TlFetchString<string>::parse(p);
  res->peer_stories_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<peerStories>, -1707742823>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  res->stealth_mode_ = TlFetchBoxed<TlFetchObject<storiesStealthMode>, 1898850301>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void stories_allStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.allStories");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_more_ << 0)));
    if (var0 & 1) { s.store_field("has_more", true); }
    s.store_field("count", count_);
    s.store_field("state", state_);
    { s.store_vector_begin("peer_stories", peer_stories_.size()); for (const auto &_value : peer_stories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("stealth_mode", static_cast<const BaseObject *>(stealth_mode_.get()));
    s.store_class_end();
  }
}

const std::int32_t account_declinePasswordReset::ID;

void account_declinePasswordReset::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1284770294);
}

void account_declinePasswordReset::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1284770294);
}

void account_declinePasswordReset::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.declinePasswordReset");
    s.store_class_end();
  }
}

account_declinePasswordReset::ReturnType account_declinePasswordReset::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_getAllSecureValues::ID;

void account_getAllSecureValues::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1299661699);
}

void account_getAllSecureValues::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1299661699);
}

void account_getAllSecureValues::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getAllSecureValues");
    s.store_class_end();
  }
}

account_getAllSecureValues::ReturnType account_getAllSecureValues::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<secureValue>, 411017418>>, 481674261>::parse(p);
#undef FAIL
}

account_getChannelDefaultEmojiStatuses::account_getChannelDefaultEmojiStatuses(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getChannelDefaultEmojiStatuses::ID;

void account_getChannelDefaultEmojiStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1999087573);
  TlStoreBinary::store(hash_, s);
}

void account_getChannelDefaultEmojiStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1999087573);
  TlStoreBinary::store(hash_, s);
}

void account_getChannelDefaultEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getChannelDefaultEmojiStatuses");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getChannelDefaultEmojiStatuses::ReturnType account_getChannelDefaultEmojiStatuses::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_EmojiStatuses>::parse(p);
#undef FAIL
}

account_getThemes::account_getThemes(string const &format_, int64 hash_)
  : format_(format_)
  , hash_(hash_)
{}

const std::int32_t account_getThemes::ID;

void account_getThemes::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1913054296);
  TlStoreString::store(format_, s);
  TlStoreBinary::store(hash_, s);
}

void account_getThemes::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1913054296);
  TlStoreString::store(format_, s);
  TlStoreBinary::store(hash_, s);
}

void account_getThemes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getThemes");
    s.store_field("format", format_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getThemes::ReturnType account_getThemes::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_Themes>::parse(p);
#undef FAIL
}

account_getTmpPassword::account_getTmpPassword(object_ptr<InputCheckPasswordSRP> &&password_, int32 period_)
  : password_(std::move(password_))
  , period_(period_)
{}

const std::int32_t account_getTmpPassword::ID;

void account_getTmpPassword::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1151208273);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
  TlStoreBinary::store(period_, s);
}

void account_getTmpPassword::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1151208273);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
  TlStoreBinary::store(period_, s);
}

void account_getTmpPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getTmpPassword");
    s.store_object_field("password", static_cast<const BaseObject *>(password_.get()));
    s.store_field("period", period_);
    s.store_class_end();
  }
}

account_getTmpPassword::ReturnType account_getTmpPassword::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_tmpPassword>, -614138572>::parse(p);
#undef FAIL
}

account_getWallPaper::account_getWallPaper(object_ptr<InputWallPaper> &&wallpaper_)
  : wallpaper_(std::move(wallpaper_))
{}

const std::int32_t account_getWallPaper::ID;

void account_getWallPaper::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-57811990);
  TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s);
}

void account_getWallPaper::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-57811990);
  TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s);
}

void account_getWallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getWallPaper");
    s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get()));
    s.store_class_end();
  }
}

account_getWallPaper::ReturnType account_getWallPaper::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<WallPaper>::parse(p);
#undef FAIL
}

account_installTheme::account_installTheme(int32 flags_, bool dark_, object_ptr<InputTheme> &&theme_, string const &format_, object_ptr<BaseTheme> &&base_theme_)
  : flags_(flags_)
  , dark_(dark_)
  , theme_(std::move(theme_))
  , format_(format_)
  , base_theme_(std::move(base_theme_))
{}

const std::int32_t account_installTheme::ID;

void account_installTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-953697477);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s); }
  if (var0 & 4) { TlStoreString::store(format_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(base_theme_, s); }
}

void account_installTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-953697477);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s); }
  if (var0 & 4) { TlStoreString::store(format_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(base_theme_, s); }
}

void account_installTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.installTheme");
    s.store_field("flags", (var0 = flags_ | (dark_ << 0)));
    if (var0 & 1) { s.store_field("dark", true); }
    if (var0 & 2) { s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get())); }
    if (var0 & 4) { s.store_field("format", format_); }
    if (var0 & 8) { s.store_object_field("base_theme", static_cast<const BaseObject *>(base_theme_.get())); }
    s.store_class_end();
  }
}

account_installTheme::ReturnType account_installTheme::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_resetPassword::ID;

void account_resetPassword::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1828139493);
}

void account_resetPassword::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1828139493);
}

void account_resetPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetPassword");
    s.store_class_end();
  }
}

account_resetPassword::ReturnType account_resetPassword::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_ResetPasswordResult>::parse(p);
#undef FAIL
}

account_saveTheme::account_saveTheme(object_ptr<InputTheme> &&theme_, bool unsave_)
  : theme_(std::move(theme_))
  , unsave_(unsave_)
{}

const std::int32_t account_saveTheme::ID;

void account_saveTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-229175188);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
  TlStoreBool::store(unsave_, s);
}

void account_saveTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-229175188);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
  TlStoreBool::store(unsave_, s);
}

void account_saveTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.saveTheme");
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_field("unsave", unsave_);
    s.store_class_end();
  }
}

account_saveTheme::ReturnType account_saveTheme::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_sendVerifyPhoneCode::account_sendVerifyPhoneCode(string const &phone_number_, object_ptr<codeSettings> &&settings_)
  : phone_number_(phone_number_)
  , settings_(std::move(settings_))
{}

const std::int32_t account_sendVerifyPhoneCode::ID;

void account_sendVerifyPhoneCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1516022023);
  TlStoreString::store(phone_number_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void account_sendVerifyPhoneCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1516022023);
  TlStoreString::store(phone_number_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void account_sendVerifyPhoneCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.sendVerifyPhoneCode");
    s.store_field("phone_number", phone_number_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_sendVerifyPhoneCode::ReturnType account_sendVerifyPhoneCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_SentCode>::parse(p);
#undef FAIL
}

account_setContentSettings::account_setContentSettings(int32 flags_, bool sensitive_enabled_)
  : flags_(flags_)
  , sensitive_enabled_(sensitive_enabled_)
{}

const std::int32_t account_setContentSettings::ID;

void account_setContentSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1250643605);
  TlStoreBinary::store((var0 = flags_ | (sensitive_enabled_ << 0)), s);
}

void account_setContentSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1250643605);
  TlStoreBinary::store((var0 = flags_ | (sensitive_enabled_ << 0)), s);
}

void account_setContentSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setContentSettings");
    s.store_field("flags", (var0 = flags_ | (sensitive_enabled_ << 0)));
    if (var0 & 1) { s.store_field("sensitive_enabled", true); }
    s.store_class_end();
  }
}

account_setContentSettings::ReturnType account_setContentSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_setReactionsNotifySettings::account_setReactionsNotifySettings(object_ptr<reactionsNotifySettings> &&settings_)
  : settings_(std::move(settings_))
{}

const std::int32_t account_setReactionsNotifySettings::ID;

void account_setReactionsNotifySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(829220168);
  TlStoreBoxed<TlStoreObject, 1457736048>::store(settings_, s);
}

void account_setReactionsNotifySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(829220168);
  TlStoreBoxed<TlStoreObject, 1457736048>::store(settings_, s);
}

void account_setReactionsNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setReactionsNotifySettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_setReactionsNotifySettings::ReturnType account_setReactionsNotifySettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<reactionsNotifySettings>, 1457736048>::parse(p);
#undef FAIL
}

account_updateBusinessLocation::account_updateBusinessLocation(int32 flags_, object_ptr<InputGeoPoint> &&geo_point_, string const &address_)
  : flags_(flags_)
  , geo_point_(std::move(geo_point_))
  , address_(address_)
{}

const std::int32_t account_updateBusinessLocation::ID;

void account_updateBusinessLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1637149926);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s); }
  if (var0 & 1) { TlStoreString::store(address_, s); }
}

void account_updateBusinessLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1637149926);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s); }
  if (var0 & 1) { TlStoreString::store(address_, s); }
}

void account_updateBusinessLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateBusinessLocation");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 2) { s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get())); }
    if (var0 & 1) { s.store_field("address", address_); }
    s.store_class_end();
  }
}

account_updateBusinessLocation::ReturnType account_updateBusinessLocation::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_dropTempAuthKeys::auth_dropTempAuthKeys(array<int64> &&except_auth_keys_)
  : except_auth_keys_(std::move(except_auth_keys_))
{}

const std::int32_t auth_dropTempAuthKeys::ID;

void auth_dropTempAuthKeys::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1907842680);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(except_auth_keys_, s);
}

void auth_dropTempAuthKeys::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1907842680);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(except_auth_keys_, s);
}

void auth_dropTempAuthKeys::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.dropTempAuthKeys");
    { s.store_vector_begin("except_auth_keys", except_auth_keys_.size()); for (const auto &_value : except_auth_keys_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

auth_dropTempAuthKeys::ReturnType auth_dropTempAuthKeys::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_importBotAuthorization::auth_importBotAuthorization(int32 flags_, int32 api_id_, string const &api_hash_, string const &bot_auth_token_)
  : flags_(flags_)
  , api_id_(api_id_)
  , api_hash_(api_hash_)
  , bot_auth_token_(bot_auth_token_)
{}

const std::int32_t auth_importBotAuthorization::ID;

void auth_importBotAuthorization::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1738800940);
  TlStoreBinary::store(flags_, s);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreString::store(bot_auth_token_, s);
}

void auth_importBotAuthorization::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1738800940);
  TlStoreBinary::store(flags_, s);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreString::store(bot_auth_token_, s);
}

void auth_importBotAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.importBotAuthorization");
    s.store_field("flags", flags_);
    s.store_field("api_id", api_id_);
    s.store_field("api_hash", api_hash_);
    s.store_field("bot_auth_token", bot_auth_token_);
    s.store_class_end();
  }
}

auth_importBotAuthorization::ReturnType auth_importBotAuthorization::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_Authorization>::parse(p);
#undef FAIL
}

auth_recoverPassword::auth_recoverPassword(int32 flags_, string const &code_, object_ptr<account_passwordInputSettings> &&new_settings_)
  : flags_(flags_)
  , code_(code_)
  , new_settings_(std::move(new_settings_))
{}

const std::int32_t auth_recoverPassword::ID;

void auth_recoverPassword::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(923364464);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(code_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1036572727>::store(new_settings_, s); }
}

void auth_recoverPassword::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(923364464);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(code_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1036572727>::store(new_settings_, s); }
}

void auth_recoverPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.recoverPassword");
    s.store_field("flags", (var0 = flags_));
    s.store_field("code", code_);
    if (var0 & 1) { s.store_object_field("new_settings", static_cast<const BaseObject *>(new_settings_.get())); }
    s.store_class_end();
  }
}

auth_recoverPassword::ReturnType auth_recoverPassword::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_Authorization>::parse(p);
#undef FAIL
}

const std::int32_t auth_requestPasswordRecovery::ID;

void auth_requestPasswordRecovery::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-661144474);
}

void auth_requestPasswordRecovery::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-661144474);
}

void auth_requestPasswordRecovery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.requestPasswordRecovery");
    s.store_class_end();
  }
}

auth_requestPasswordRecovery::ReturnType auth_requestPasswordRecovery::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<auth_passwordRecovery>, 326715557>::parse(p);
#undef FAIL
}

bots_getBotRecommendations::bots_getBotRecommendations(object_ptr<InputUser> &&bot_)
  : bot_(std::move(bot_))
{}

const std::int32_t bots_getBotRecommendations::ID;

void bots_getBotRecommendations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1581840363);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_getBotRecommendations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1581840363);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_getBotRecommendations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getBotRecommendations");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

bots_getBotRecommendations::ReturnType bots_getBotRecommendations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<users_Users>::parse(p);
#undef FAIL
}

bots_setBotBroadcastDefaultAdminRights::bots_setBotBroadcastDefaultAdminRights(object_ptr<chatAdminRights> &&admin_rights_)
  : admin_rights_(std::move(admin_rights_))
{}

const std::int32_t bots_setBotBroadcastDefaultAdminRights::ID;

void bots_setBotBroadcastDefaultAdminRights::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2021942497);
  TlStoreBoxed<TlStoreObject, 1605510357>::store(admin_rights_, s);
}

void bots_setBotBroadcastDefaultAdminRights::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2021942497);
  TlStoreBoxed<TlStoreObject, 1605510357>::store(admin_rights_, s);
}

void bots_setBotBroadcastDefaultAdminRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.setBotBroadcastDefaultAdminRights");
    s.store_object_field("admin_rights", static_cast<const BaseObject *>(admin_rights_.get()));
    s.store_class_end();
  }
}

bots_setBotBroadcastDefaultAdminRights::ReturnType bots_setBotBroadcastDefaultAdminRights::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_setBotCommands::bots_setBotCommands(object_ptr<BotCommandScope> &&scope_, string const &lang_code_, array<object_ptr<botCommand>> &&commands_)
  : scope_(std::move(scope_))
  , lang_code_(lang_code_)
  , commands_(std::move(commands_))
{}

const std::int32_t bots_setBotCommands::ID;

void bots_setBotCommands::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(85399130);
  TlStoreBoxedUnknown<TlStoreObject>::store(scope_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1032140601>>, 481674261>::store(commands_, s);
}

void bots_setBotCommands::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(85399130);
  TlStoreBoxedUnknown<TlStoreObject>::store(scope_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1032140601>>, 481674261>::store(commands_, s);
}

void bots_setBotCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.setBotCommands");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_field("lang_code", lang_code_);
    { s.store_vector_begin("commands", commands_.size()); for (const auto &_value : commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

bots_setBotCommands::ReturnType bots_setBotCommands::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_getLeftChannels::channels_getLeftChannels(int32 offset_)
  : offset_(offset_)
{}

const std::int32_t channels_getLeftChannels::ID;

void channels_getLeftChannels::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2092831552);
  TlStoreBinary::store(offset_, s);
}

void channels_getLeftChannels::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2092831552);
  TlStoreBinary::store(offset_, s);
}

void channels_getLeftChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getLeftChannels");
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

channels_getLeftChannels::ReturnType channels_getLeftChannels::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

channels_joinChannel::channels_joinChannel(object_ptr<InputChannel> &&channel_)
  : channel_(std::move(channel_))
{}

const std::int32_t channels_joinChannel::ID;

void channels_joinChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(615851205);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_joinChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(615851205);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_joinChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.joinChannel");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

channels_joinChannel::ReturnType channels_joinChannel::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_restrictSponsoredMessages::channels_restrictSponsoredMessages(object_ptr<InputChannel> &&channel_, bool restricted_)
  : channel_(std::move(channel_))
  , restricted_(restricted_)
{}

const std::int32_t channels_restrictSponsoredMessages::ID;

void channels_restrictSponsoredMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1696000743);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(restricted_, s);
}

void channels_restrictSponsoredMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1696000743);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(restricted_, s);
}

void channels_restrictSponsoredMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.restrictSponsoredMessages");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("restricted", restricted_);
    s.store_class_end();
  }
}

channels_restrictSponsoredMessages::ReturnType channels_restrictSponsoredMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_searchPosts::channels_searchPosts(int32 flags_, string const &hashtag_, string const &query_, int32 offset_rate_, object_ptr<InputPeer> &&offset_peer_, int32 offset_id_, int32 limit_, int64 allow_paid_stars_)
  : flags_(flags_)
  , hashtag_(hashtag_)
  , query_(query_)
  , offset_rate_(offset_rate_)
  , offset_peer_(std::move(offset_peer_))
  , offset_id_(offset_id_)
  , limit_(limit_)
  , allow_paid_stars_(allow_paid_stars_)
{}

const std::int32_t channels_searchPosts::ID;

void channels_searchPosts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-221973939);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(hashtag_, s); }
  if (var0 & 2) { TlStoreString::store(query_, s); }
  TlStoreBinary::store(offset_rate_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
  if (var0 & 4) { TlStoreBinary::store(allow_paid_stars_, s); }
}

void channels_searchPosts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-221973939);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(hashtag_, s); }
  if (var0 & 2) { TlStoreString::store(query_, s); }
  TlStoreBinary::store(offset_rate_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
  if (var0 & 4) { TlStoreBinary::store(allow_paid_stars_, s); }
}

void channels_searchPosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.searchPosts");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("hashtag", hashtag_); }
    if (var0 & 2) { s.store_field("query", query_); }
    s.store_field("offset_rate", offset_rate_);
    s.store_object_field("offset_peer", static_cast<const BaseObject *>(offset_peer_.get()));
    s.store_field("offset_id", offset_id_);
    s.store_field("limit", limit_);
    if (var0 & 4) { s.store_field("allow_paid_stars", allow_paid_stars_); }
    s.store_class_end();
  }
}

channels_searchPosts::ReturnType channels_searchPosts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

channels_togglePreHistoryHidden::channels_togglePreHistoryHidden(object_ptr<InputChannel> &&channel_, bool enabled_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
{}

const std::int32_t channels_togglePreHistoryHidden::ID;

void channels_togglePreHistoryHidden::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-356796084);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_togglePreHistoryHidden::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-356796084);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_togglePreHistoryHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.togglePreHistoryHidden");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

channels_togglePreHistoryHidden::ReturnType channels_togglePreHistoryHidden::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

chatlists_editExportedInvite::chatlists_editExportedInvite(int32 flags_, object_ptr<inputChatlistDialogFilter> &&chatlist_, string const &slug_, string const &title_, array<object_ptr<InputPeer>> &&peers_)
  : flags_(flags_)
  , chatlist_(std::move(chatlist_))
  , slug_(slug_)
  , title_(title_)
  , peers_(std::move(peers_))
{}

const std::int32_t chatlists_editExportedInvite::ID;

void chatlists_editExportedInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1698543165);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreString::store(slug_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s); }
}

void chatlists_editExportedInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1698543165);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreString::store(slug_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s); }
}

void chatlists_editExportedInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.editExportedInvite");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    s.store_field("slug", slug_);
    if (var0 & 2) { s.store_field("title", title_); }
    if (var0 & 4) { { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

chatlists_editExportedInvite::ReturnType chatlists_editExportedInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<exportedChatlistInvite>, 206668204>::parse(p);
#undef FAIL
}

const std::int32_t contacts_getBirthdays::ID;

void contacts_getBirthdays::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-621959068);
}

void contacts_getBirthdays::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-621959068);
}

void contacts_getBirthdays::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getBirthdays");
    s.store_class_end();
  }
}

contacts_getBirthdays::ReturnType contacts_getBirthdays::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<contacts_contactBirthdays>, 290452237>::parse(p);
#undef FAIL
}

contacts_resolveUsername::contacts_resolveUsername(int32 flags_, string const &username_, string const &referer_)
  : flags_(flags_)
  , username_(username_)
  , referer_(referer_)
{}

const std::int32_t contacts_resolveUsername::ID;

void contacts_resolveUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1918565308);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(username_, s);
  if (var0 & 1) { TlStoreString::store(referer_, s); }
}

void contacts_resolveUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1918565308);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(username_, s);
  if (var0 & 1) { TlStoreString::store(referer_, s); }
}

void contacts_resolveUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.resolveUsername");
    s.store_field("flags", (var0 = flags_));
    s.store_field("username", username_);
    if (var0 & 1) { s.store_field("referer", referer_); }
    s.store_class_end();
  }
}

contacts_resolveUsername::ReturnType contacts_resolveUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<contacts_resolvedPeer>, 2131196633>::parse(p);
#undef FAIL
}

contacts_setBlocked::contacts_setBlocked(int32 flags_, bool my_stories_from_, array<object_ptr<InputPeer>> &&id_, int32 limit_)
  : flags_(flags_)
  , my_stories_from_(my_stories_from_)
  , id_(std::move(id_))
  , limit_(limit_)
{}

const std::int32_t contacts_setBlocked::ID;

void contacts_setBlocked::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1798939530);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
  TlStoreBinary::store(limit_, s);
}

void contacts_setBlocked::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1798939530);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
  TlStoreBinary::store(limit_, s);
}

void contacts_setBlocked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.setBlocked");
    s.store_field("flags", (var0 = flags_ | (my_stories_from_ << 0)));
    if (var0 & 1) { s.store_field("my_stories_from", true); }
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

contacts_setBlocked::ReturnType contacts_setBlocked::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

help_getAppUpdate::help_getAppUpdate(string const &source_)
  : source_(source_)
{}

const std::int32_t help_getAppUpdate::ID;

void help_getAppUpdate::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1378703997);
  TlStoreString::store(source_, s);
}

void help_getAppUpdate::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1378703997);
  TlStoreString::store(source_, s);
}

void help_getAppUpdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getAppUpdate");
    s.store_field("source", source_);
    s.store_class_end();
  }
}

help_getAppUpdate::ReturnType help_getAppUpdate::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_AppUpdate>::parse(p);
#undef FAIL
}

help_getPeerColors::help_getPeerColors(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t help_getPeerColors::ID;

void help_getPeerColors::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-629083089);
  TlStoreBinary::store(hash_, s);
}

void help_getPeerColors::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-629083089);
  TlStoreBinary::store(hash_, s);
}

void help_getPeerColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getPeerColors");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

help_getPeerColors::ReturnType help_getPeerColors::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_PeerColors>::parse(p);
#undef FAIL
}

const std::int32_t help_getPremiumPromo::ID;

void help_getPremiumPromo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1206152236);
}

void help_getPremiumPromo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1206152236);
}

void help_getPremiumPromo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getPremiumPromo");
    s.store_class_end();
  }
}

help_getPremiumPromo::ReturnType help_getPremiumPromo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<help_premiumPromo>, 1395946908>::parse(p);
#undef FAIL
}

const std::int32_t help_getSupportName::ID;

void help_getSupportName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-748624084);
}

void help_getSupportName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-748624084);
}

void help_getSupportName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getSupportName");
    s.store_class_end();
  }
}

help_getSupportName::ReturnType help_getSupportName::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<help_supportName>, -1945767479>::parse(p);
#undef FAIL
}

langpack_getLanguage::langpack_getLanguage(string const &lang_pack_, string const &lang_code_)
  : lang_pack_(lang_pack_)
  , lang_code_(lang_code_)
{}

const std::int32_t langpack_getLanguage::ID;

void langpack_getLanguage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1784243458);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
}

void langpack_getLanguage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1784243458);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
}

void langpack_getLanguage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langpack.getLanguage");
    s.store_field("lang_pack", lang_pack_);
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

langpack_getLanguage::ReturnType langpack_getLanguage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<langPackLanguage>, -288727837>::parse(p);
#undef FAIL
}

messages_acceptEncryption::messages_acceptEncryption(object_ptr<inputEncryptedChat> &&peer_, bytes &&g_b_, int64 key_fingerprint_)
  : peer_(std::move(peer_))
  , g_b_(std::move(g_b_))
  , key_fingerprint_(key_fingerprint_)
{}

const std::int32_t messages_acceptEncryption::ID;

void messages_acceptEncryption::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1035731989);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreString::store(g_b_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void messages_acceptEncryption::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1035731989);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreString::store(g_b_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void messages_acceptEncryption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.acceptEncryption");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_bytes_field("g_b", g_b_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_class_end();
  }
}

messages_acceptEncryption::ReturnType messages_acceptEncryption::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EncryptedChat>::parse(p);
#undef FAIL
}

messages_deleteExportedChatInvite::messages_deleteExportedChatInvite(object_ptr<InputPeer> &&peer_, string const &link_)
  : peer_(std::move(peer_))
  , link_(link_)
{}

const std::int32_t messages_deleteExportedChatInvite::ID;

void messages_deleteExportedChatInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-731601877);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
}

void messages_deleteExportedChatInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-731601877);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(link_, s);
}

void messages_deleteExportedChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteExportedChatInvite");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("link", link_);
    s.store_class_end();
  }
}

messages_deleteExportedChatInvite::ReturnType messages_deleteExportedChatInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_editChatAdmin::messages_editChatAdmin(int64 chat_id_, object_ptr<InputUser> &&user_id_, bool is_admin_)
  : chat_id_(chat_id_)
  , user_id_(std::move(user_id_))
  , is_admin_(is_admin_)
{}

const std::int32_t messages_editChatAdmin::ID;

void messages_editChatAdmin::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1470377534);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBool::store(is_admin_, s);
}

void messages_editChatAdmin::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1470377534);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBool::store(is_admin_, s);
}

void messages_editChatAdmin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editChatAdmin");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("is_admin", is_admin_);
    s.store_class_end();
  }
}

messages_editChatAdmin::ReturnType messages_editChatAdmin::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_editInlineBotMessage::messages_editInlineBotMessage(int32 flags_, bool no_webpage_, bool invert_media_, object_ptr<InputBotInlineMessageID> &&id_, string const &message_, object_ptr<InputMedia> &&media_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_)
  : flags_(flags_)
  , no_webpage_(no_webpage_)
  , invert_media_(invert_media_)
  , id_(std::move(id_))
  , message_(message_)
  , media_(std::move(media_))
  , reply_markup_(std::move(reply_markup_))
  , entities_(std::move(entities_))
{}

const std::int32_t messages_editInlineBotMessage::ID;

void messages_editInlineBotMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2091549254);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 16)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 2048) { TlStoreString::store(message_, s); }
  if (var0 & 16384) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
}

void messages_editInlineBotMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2091549254);
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 16)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 2048) { TlStoreString::store(message_, s); }
  if (var0 & 16384) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
}

void messages_editInlineBotMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editInlineBotMessage");
    s.store_field("flags", (var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 16)));
    if (var0 & 2) { s.store_field("no_webpage", true); }
    if (var0 & 65536) { s.store_field("invert_media", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    if (var0 & 2048) { s.store_field("message", message_); }
    if (var0 & 16384) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    if (var0 & 8) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

messages_editInlineBotMessage::ReturnType messages_editInlineBotMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_faveSticker::messages_faveSticker(object_ptr<InputDocument> &&id_, bool unfave_)
  : id_(std::move(id_))
  , unfave_(unfave_)
{}

const std::int32_t messages_faveSticker::ID;

void messages_faveSticker::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1174420133);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unfave_, s);
}

void messages_faveSticker::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1174420133);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unfave_, s);
}

void messages_faveSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.faveSticker");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_field("unfave", unfave_);
    s.store_class_end();
  }
}

messages_faveSticker::ReturnType messages_faveSticker::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_getAvailableEffects::messages_getAvailableEffects(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getAvailableEffects::ID;

void messages_getAvailableEffects::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-559805895);
  TlStoreBinary::store(hash_, s);
}

void messages_getAvailableEffects::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-559805895);
  TlStoreBinary::store(hash_, s);
}

void messages_getAvailableEffects::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAvailableEffects");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getAvailableEffects::ReturnType messages_getAvailableEffects::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_AvailableEffects>::parse(p);
#undef FAIL
}

messages_getChats::messages_getChats(array<int64> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t messages_getChats::ID;

void messages_getChats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1240027791);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getChats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1240027791);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getChats");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getChats::ReturnType messages_getChats::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

messages_getDefaultTagReactions::messages_getDefaultTagReactions(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getDefaultTagReactions::ID;

void messages_getDefaultTagReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1107741656);
  TlStoreBinary::store(hash_, s);
}

void messages_getDefaultTagReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1107741656);
  TlStoreBinary::store(hash_, s);
}

void messages_getDefaultTagReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDefaultTagReactions");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getDefaultTagReactions::ReturnType messages_getDefaultTagReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Reactions>::parse(p);
#undef FAIL
}

messages_getDialogUnreadMarks::messages_getDialogUnreadMarks(int32 flags_, object_ptr<InputPeer> &&parent_peer_)
  : flags_(flags_)
  , parent_peer_(std::move(parent_peer_))
{}

const std::int32_t messages_getDialogUnreadMarks::ID;

void messages_getDialogUnreadMarks::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(555754018);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
}

void messages_getDialogUnreadMarks::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(555754018);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
}

void messages_getDialogUnreadMarks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDialogUnreadMarks");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
    s.store_class_end();
  }
}

messages_getDialogUnreadMarks::ReturnType messages_getDialogUnreadMarks::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<DialogPeer>>, 481674261>::parse(p);
#undef FAIL
}

messages_getDialogs::messages_getDialogs(int32 flags_, bool exclude_pinned_, int32 folder_id_, int32 offset_date_, int32 offset_id_, object_ptr<InputPeer> &&offset_peer_, int32 limit_, int64 hash_)
  : flags_(flags_)
  , exclude_pinned_(exclude_pinned_)
  , folder_id_(folder_id_)
  , offset_date_(offset_date_)
  , offset_id_(offset_id_)
  , offset_peer_(std::move(offset_peer_))
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t messages_getDialogs::ID;

void messages_getDialogs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1594569905);
  TlStoreBinary::store((var0 = flags_ | (exclude_pinned_ << 0)), s);
  if (var0 & 2) { TlStoreBinary::store(folder_id_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getDialogs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1594569905);
  TlStoreBinary::store((var0 = flags_ | (exclude_pinned_ << 0)), s);
  if (var0 & 2) { TlStoreBinary::store(folder_id_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDialogs");
    s.store_field("flags", (var0 = flags_ | (exclude_pinned_ << 0)));
    if (var0 & 1) { s.store_field("exclude_pinned", true); }
    if (var0 & 2) { s.store_field("folder_id", folder_id_); }
    s.store_field("offset_date", offset_date_);
    s.store_field("offset_id", offset_id_);
    s.store_object_field("offset_peer", static_cast<const BaseObject *>(offset_peer_.get()));
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getDialogs::ReturnType messages_getDialogs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Dialogs>::parse(p);
#undef FAIL
}

messages_getDiscussionMessage::messages_getDiscussionMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t messages_getDiscussionMessage::ID;

void messages_getDiscussionMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1147761405);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getDiscussionMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1147761405);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getDiscussionMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDiscussionMessage");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

messages_getDiscussionMessage::ReturnType messages_getDiscussionMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_discussionMessage>, -1506535550>::parse(p);
#undef FAIL
}

messages_getEmojiStickerGroups::messages_getEmojiStickerGroups(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getEmojiStickerGroups::ID;

void messages_getEmojiStickerGroups::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(500711669);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiStickerGroups::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(500711669);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiStickerGroups::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiStickerGroups");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getEmojiStickerGroups::ReturnType messages_getEmojiStickerGroups::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_EmojiGroups>::parse(p);
#undef FAIL
}

messages_getEmojiURL::messages_getEmojiURL(string const &lang_code_)
  : lang_code_(lang_code_)
{}

const std::int32_t messages_getEmojiURL::ID;

void messages_getEmojiURL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-709817306);
  TlStoreString::store(lang_code_, s);
}

void messages_getEmojiURL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-709817306);
  TlStoreString::store(lang_code_, s);
}

void messages_getEmojiURL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiURL");
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

messages_getEmojiURL::ReturnType messages_getEmojiURL::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<emojiURL>, -1519029347>::parse(p);
#undef FAIL
}

messages_getMessageReactionsList::messages_getMessageReactionsList(int32 flags_, object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<Reaction> &&reaction_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , id_(id_)
  , reaction_(std::move(reaction_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t messages_getMessageReactionsList::ID;

void messages_getMessageReactionsList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1176190792);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s); }
  if (var0 & 2) { TlStoreString::store(offset_, s); }
  TlStoreBinary::store(limit_, s);
}

void messages_getMessageReactionsList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1176190792);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s); }
  if (var0 & 2) { TlStoreString::store(offset_, s); }
  TlStoreBinary::store(limit_, s);
}

void messages_getMessageReactionsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMessageReactionsList");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get())); }
    if (var0 & 2) { s.store_field("offset", offset_); }
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getMessageReactionsList::ReturnType messages_getMessageReactionsList::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_messageReactionsList>, 834488621>::parse(p);
#undef FAIL
}

messages_getScheduledMessages::messages_getScheduledMessages(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t messages_getScheduledMessages::ID;

void messages_getScheduledMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1111817116);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getScheduledMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1111817116);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_getScheduledMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getScheduledMessages");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getScheduledMessages::ReturnType messages_getScheduledMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_getSearchCounters::messages_getSearchCounters(int32 flags_, object_ptr<InputPeer> &&peer_, object_ptr<InputPeer> &&saved_peer_id_, int32 top_msg_id_, array<object_ptr<MessagesFilter>> &&filters_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , saved_peer_id_(std::move(saved_peer_id_))
  , top_msg_id_(top_msg_id_)
  , filters_(std::move(filters_))
{}

const std::int32_t messages_getSearchCounters::ID;

void messages_getSearchCounters::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(465367808);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(filters_, s);
}

void messages_getSearchCounters::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(465367808);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(filters_, s);
}

void messages_getSearchCounters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSearchCounters");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 4) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    { s.store_vector_begin("filters", filters_.size()); for (const auto &_value : filters_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getSearchCounters::ReturnType messages_getSearchCounters::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<messages_searchCounter>, -398136321>>, 481674261>::parse(p);
#undef FAIL
}

messages_getTopReactions::messages_getTopReactions(int32 limit_, int64 hash_)
  : limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t messages_getTopReactions::ID;

void messages_getTopReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1149164102);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getTopReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1149164102);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getTopReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getTopReactions");
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getTopReactions::ReturnType messages_getTopReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Reactions>::parse(p);
#undef FAIL
}

messages_getWebPage::messages_getWebPage(string const &url_, int32 hash_)
  : url_(url_)
  , hash_(hash_)
{}

const std::int32_t messages_getWebPage::ID;

void messages_getWebPage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1919511901);
  TlStoreString::store(url_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getWebPage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1919511901);
  TlStoreString::store(url_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getWebPage");
    s.store_field("url", url_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getWebPage::ReturnType messages_getWebPage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_webPage>, -44166467>::parse(p);
#undef FAIL
}

messages_reorderPinnedSavedDialogs::messages_reorderPinnedSavedDialogs(int32 flags_, bool force_, array<object_ptr<InputDialogPeer>> &&order_)
  : flags_(flags_)
  , force_(force_)
  , order_(std::move(order_))
{}

const std::int32_t messages_reorderPinnedSavedDialogs::ID;

void messages_reorderPinnedSavedDialogs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1955502713);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s);
}

void messages_reorderPinnedSavedDialogs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1955502713);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s);
}

void messages_reorderPinnedSavedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reorderPinnedSavedDialogs");
    s.store_field("flags", (var0 = flags_ | (force_ << 0)));
    if (var0 & 1) { s.store_field("force", true); }
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_reorderPinnedSavedDialogs::ReturnType messages_reorderPinnedSavedDialogs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_requestMainWebView::messages_requestMainWebView(int32 flags_, bool compact_, bool fullscreen_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&bot_, string const &start_param_, object_ptr<dataJSON> &&theme_params_, string const &platform_)
  : flags_(flags_)
  , compact_(compact_)
  , fullscreen_(fullscreen_)
  , peer_(std::move(peer_))
  , bot_(std::move(bot_))
  , start_param_(start_param_)
  , theme_params_(std::move(theme_params_))
  , platform_(platform_)
{}

const std::int32_t messages_requestMainWebView::ID;

void messages_requestMainWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-908059013);
  TlStoreBinary::store((var0 = flags_ | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  if (var0 & 2) { TlStoreString::store(start_param_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
}

void messages_requestMainWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-908059013);
  TlStoreBinary::store((var0 = flags_ | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  if (var0 & 2) { TlStoreString::store(start_param_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
}

void messages_requestMainWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.requestMainWebView");
    s.store_field("flags", (var0 = flags_ | (compact_ << 7) | (fullscreen_ << 8)));
    if (var0 & 128) { s.store_field("compact", true); }
    if (var0 & 256) { s.store_field("fullscreen", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    if (var0 & 2) { s.store_field("start_param", start_param_); }
    if (var0 & 1) { s.store_object_field("theme_params", static_cast<const BaseObject *>(theme_params_.get())); }
    s.store_field("platform", platform_);
    s.store_class_end();
  }
}

messages_requestMainWebView::ReturnType messages_requestMainWebView::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<webViewResultUrl>, 1294139288>::parse(p);
#undef FAIL
}

messages_sendInlineBotResult::messages_sendInlineBotResult(int32 flags_, bool silent_, bool background_, bool clear_draft_, bool hide_via_, object_ptr<InputPeer> &&peer_, object_ptr<InputReplyTo> &&reply_to_, int64 random_id_, int64 query_id_, string const &id_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_, object_ptr<InputQuickReplyShortcut> &&quick_reply_shortcut_, int64 allow_paid_stars_)
  : flags_(flags_)
  , silent_(silent_)
  , background_(background_)
  , clear_draft_(clear_draft_)
  , hide_via_(hide_via_)
  , peer_(std::move(peer_))
  , reply_to_(std::move(reply_to_))
  , random_id_(random_id_)
  , query_id_(query_id_)
  , id_(id_)
  , schedule_date_(schedule_date_)
  , send_as_(std::move(send_as_))
  , quick_reply_shortcut_(std::move(quick_reply_shortcut_))
  , allow_paid_stars_(allow_paid_stars_)
{}

const std::int32_t messages_sendInlineBotResult::ID;

void messages_sendInlineBotResult::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1060145594);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (hide_via_ << 11)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(query_id_, s);
  TlStoreString::store(id_, s);
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
}

void messages_sendInlineBotResult::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1060145594);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (hide_via_ << 11)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(query_id_, s);
  TlStoreString::store(id_, s);
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
}

void messages_sendInlineBotResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendInlineBotResult");
    s.store_field("flags", (var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (hide_via_ << 11)));
    if (var0 & 32) { s.store_field("silent", true); }
    if (var0 & 64) { s.store_field("background", true); }
    if (var0 & 128) { s.store_field("clear_draft", true); }
    if (var0 & 2048) { s.store_field("hide_via", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    s.store_field("random_id", random_id_);
    s.store_field("query_id", query_id_);
    s.store_field("id", id_);
    if (var0 & 1024) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 8192) { s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get())); }
    if (var0 & 131072) { s.store_object_field("quick_reply_shortcut", static_cast<const BaseObject *>(quick_reply_shortcut_.get())); }
    if (var0 & 2097152) { s.store_field("allow_paid_stars", allow_paid_stars_); }
    s.store_class_end();
  }
}

messages_sendInlineBotResult::ReturnType messages_sendInlineBotResult::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_setInlineGameScore::messages_setInlineGameScore(int32 flags_, bool edit_message_, bool force_, object_ptr<InputBotInlineMessageID> &&id_, object_ptr<InputUser> &&user_id_, int32 score_)
  : flags_(flags_)
  , edit_message_(edit_message_)
  , force_(force_)
  , id_(std::move(id_))
  , user_id_(std::move(user_id_))
  , score_(score_)
{}

const std::int32_t messages_setInlineGameScore::ID;

void messages_setInlineGameScore::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(363700068);
  TlStoreBinary::store((var0 = flags_ | (edit_message_ << 0) | (force_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(score_, s);
}

void messages_setInlineGameScore::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(363700068);
  TlStoreBinary::store((var0 = flags_ | (edit_message_ << 0) | (force_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(score_, s);
}

void messages_setInlineGameScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setInlineGameScore");
    s.store_field("flags", (var0 = flags_ | (edit_message_ << 0) | (force_ << 1)));
    if (var0 & 1) { s.store_field("edit_message", true); }
    if (var0 & 2) { s.store_field("force", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("score", score_);
    s.store_class_end();
  }
}

messages_setInlineGameScore::ReturnType messages_setInlineGameScore::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_startBot::messages_startBot(object_ptr<InputUser> &&bot_, object_ptr<InputPeer> &&peer_, int64 random_id_, string const &start_param_)
  : bot_(std::move(bot_))
  , peer_(std::move(peer_))
  , random_id_(random_id_)
  , start_param_(start_param_)
{}

const std::int32_t messages_startBot::ID;

void messages_startBot::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-421563528);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(start_param_, s);
}

void messages_startBot::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-421563528);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(start_param_, s);
}

void messages_startBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.startBot");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("random_id", random_id_);
    s.store_field("start_param", start_param_);
    s.store_class_end();
  }
}

messages_startBot::ReturnType messages_startBot::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_togglePaidReactionPrivacy::messages_togglePaidReactionPrivacy(object_ptr<InputPeer> &&peer_, int32 msg_id_, object_ptr<PaidReactionPrivacy> &&private_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , private_(std::move(private_))
{}

const std::int32_t messages_togglePaidReactionPrivacy::ID;

void messages_togglePaidReactionPrivacy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1129874869);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(private_, s);
}

void messages_togglePaidReactionPrivacy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1129874869);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(private_, s);
}

void messages_togglePaidReactionPrivacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.togglePaidReactionPrivacy");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_object_field("private", static_cast<const BaseObject *>(private_.get()));
    s.store_class_end();
  }
}

messages_togglePaidReactionPrivacy::ReturnType messages_togglePaidReactionPrivacy::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_toggleTodoCompleted::messages_toggleTodoCompleted(object_ptr<InputPeer> &&peer_, int32 msg_id_, array<int32> &&completed_, array<int32> &&incompleted_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , completed_(std::move(completed_))
  , incompleted_(std::move(incompleted_))
{}

const std::int32_t messages_toggleTodoCompleted::ID;

void messages_toggleTodoCompleted::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-740282076);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(completed_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(incompleted_, s);
}

void messages_toggleTodoCompleted::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-740282076);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(completed_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(incompleted_, s);
}

void messages_toggleTodoCompleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleTodoCompleted");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    { s.store_vector_begin("completed", completed_.size()); for (const auto &_value : completed_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("incompleted", incompleted_.size()); for (const auto &_value : incompleted_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_toggleTodoCompleted::ReturnType messages_toggleTodoCompleted::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_translateText::messages_translateText(int32 flags_, object_ptr<InputPeer> &&peer_, array<int32> &&id_, array<object_ptr<textWithEntities>> &&text_, string const &to_lang_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , id_(std::move(id_))
  , text_(std::move(text_))
  , to_lang_(to_lang_)
{}

const std::int32_t messages_translateText::ID;

void messages_translateText::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1662529584);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 1964978502>>, 481674261>::store(text_, s); }
  TlStoreString::store(to_lang_, s);
}

void messages_translateText::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1662529584);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 1964978502>>, 481674261>::store(text_, s); }
  TlStoreString::store(to_lang_, s);
}

void messages_translateText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.translateText");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    if (var0 & 1) { { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 2) { { s.store_vector_begin("text", text_.size()); for (const auto &_value : text_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("to_lang", to_lang_);
    s.store_class_end();
  }
}

messages_translateText::ReturnType messages_translateText::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_translateResult>, 870003448>::parse(p);
#undef FAIL
}

messages_unpinAllMessages::messages_unpinAllMessages(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, object_ptr<InputPeer> &&saved_peer_id_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
  , saved_peer_id_(std::move(saved_peer_id_))
{}

const std::int32_t messages_unpinAllMessages::ID;

void messages_unpinAllMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(103667527);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
}

void messages_unpinAllMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(103667527);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
}

void messages_unpinAllMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.unpinAllMessages");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 2) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_class_end();
  }
}

messages_unpinAllMessages::ReturnType messages_unpinAllMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedHistory>, -1269012015>::parse(p);
#undef FAIL
}

payments_changeStarsSubscription::payments_changeStarsSubscription(int32 flags_, object_ptr<InputPeer> &&peer_, string const &subscription_id_, bool canceled_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , subscription_id_(subscription_id_)
  , canceled_(canceled_)
{}

const std::int32_t payments_changeStarsSubscription::ID;

void payments_changeStarsSubscription::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-948500360);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(subscription_id_, s);
  if (var0 & 1) { TlStoreBool::store(canceled_, s); }
}

void payments_changeStarsSubscription::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-948500360);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(subscription_id_, s);
  if (var0 & 1) { TlStoreBool::store(canceled_, s); }
}

void payments_changeStarsSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.changeStarsSubscription");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("subscription_id", subscription_id_);
    if (var0 & 1) { s.store_field("canceled", canceled_); }
    s.store_class_end();
  }
}

payments_changeStarsSubscription::ReturnType payments_changeStarsSubscription::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_connectStarRefBot::payments_connectStarRefBot(object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&bot_)
  : peer_(std::move(peer_))
  , bot_(std::move(bot_))
{}

const std::int32_t payments_connectStarRefBot::ID;

void payments_connectStarRefBot::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2127901834);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void payments_connectStarRefBot::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2127901834);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void payments_connectStarRefBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.connectStarRefBot");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

payments_connectStarRefBot::ReturnType payments_connectStarRefBot::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_connectedStarRefBots>, -1730811363>::parse(p);
#undef FAIL
}

payments_createStarGiftCollection::payments_createStarGiftCollection(object_ptr<InputPeer> &&peer_, string const &title_, array<object_ptr<InputSavedStarGift>> &&stargift_)
  : peer_(std::move(peer_))
  , title_(title_)
  , stargift_(std::move(stargift_))
{}

const std::int32_t payments_createStarGiftCollection::ID;

void payments_createStarGiftCollection::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(524947079);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stargift_, s);
}

void payments_createStarGiftCollection::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(524947079);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stargift_, s);
}

void payments_createStarGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.createStarGiftCollection");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("title", title_);
    { s.store_vector_begin("stargift", stargift_.size()); for (const auto &_value : stargift_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_createStarGiftCollection::ReturnType payments_createStarGiftCollection::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<starGiftCollection>, -1653926992>::parse(p);
#undef FAIL
}

payments_getConnectedStarRefBots::payments_getConnectedStarRefBots(int32 flags_, object_ptr<InputPeer> &&peer_, int32 offset_date_, string const &offset_link_, int32 limit_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , offset_date_(offset_date_)
  , offset_link_(offset_link_)
  , limit_(limit_)
{}

const std::int32_t payments_getConnectedStarRefBots::ID;

void payments_getConnectedStarRefBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1483318611);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBinary::store(offset_date_, s); }
  if (var0 & 4) { TlStoreString::store(offset_link_, s); }
  TlStoreBinary::store(limit_, s);
}

void payments_getConnectedStarRefBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1483318611);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBinary::store(offset_date_, s); }
  if (var0 & 4) { TlStoreString::store(offset_link_, s); }
  TlStoreBinary::store(limit_, s);
}

void payments_getConnectedStarRefBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getConnectedStarRefBots");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 4) { s.store_field("offset_date", offset_date_); }
    if (var0 & 4) { s.store_field("offset_link", offset_link_); }
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

payments_getConnectedStarRefBots::ReturnType payments_getConnectedStarRefBots::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_connectedStarRefBots>, -1730811363>::parse(p);
#undef FAIL
}

payments_getStarsTransactions::payments_getStarsTransactions(int32 flags_, bool inbound_, bool outbound_, bool ascending_, bool ton_, string const &subscription_id_, object_ptr<InputPeer> &&peer_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , inbound_(inbound_)
  , outbound_(outbound_)
  , ascending_(ascending_)
  , ton_(ton_)
  , subscription_id_(subscription_id_)
  , peer_(std::move(peer_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t payments_getStarsTransactions::ID;

void payments_getStarsTransactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1775912279);
  TlStoreBinary::store((var0 = flags_ | (inbound_ << 0) | (outbound_ << 1) | (ascending_ << 2) | (ton_ << 4)), s);
  if (var0 & 8) { TlStoreString::store(subscription_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getStarsTransactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1775912279);
  TlStoreBinary::store((var0 = flags_ | (inbound_ << 0) | (outbound_ << 1) | (ascending_ << 2) | (ton_ << 4)), s);
  if (var0 & 8) { TlStoreString::store(subscription_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getStarsTransactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsTransactions");
    s.store_field("flags", (var0 = flags_ | (inbound_ << 0) | (outbound_ << 1) | (ascending_ << 2) | (ton_ << 4)));
    if (var0 & 1) { s.store_field("inbound", true); }
    if (var0 & 2) { s.store_field("outbound", true); }
    if (var0 & 4) { s.store_field("ascending", true); }
    if (var0 & 16) { s.store_field("ton", true); }
    if (var0 & 8) { s.store_field("subscription_id", subscription_id_); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

payments_getStarsTransactions::ReturnType payments_getStarsTransactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starsStatus>, 1822222573>::parse(p);
#undef FAIL
}

payments_getStarsTransactionsByID::payments_getStarsTransactionsByID(int32 flags_, bool ton_, object_ptr<InputPeer> &&peer_, array<object_ptr<inputStarsTransaction>> &&id_)
  : flags_(flags_)
  , ton_(ton_)
  , peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t payments_getStarsTransactionsByID::ID;

void payments_getStarsTransactionsByID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(768218808);
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 543876817>>, 481674261>::store(id_, s);
}

void payments_getStarsTransactionsByID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(768218808);
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 543876817>>, 481674261>::store(id_, s);
}

void payments_getStarsTransactionsByID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsTransactionsByID");
    s.store_field("flags", (var0 = flags_ | (ton_ << 0)));
    if (var0 & 1) { s.store_field("ton", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_getStarsTransactionsByID::ReturnType payments_getStarsTransactionsByID::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starsStatus>, 1822222573>::parse(p);
#undef FAIL
}

payments_getUniqueStarGiftValueInfo::payments_getUniqueStarGiftValueInfo(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t payments_getUniqueStarGiftValueInfo::ID;

void payments_getUniqueStarGiftValueInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1130737515);
  TlStoreString::store(slug_, s);
}

void payments_getUniqueStarGiftValueInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1130737515);
  TlStoreString::store(slug_, s);
}

void payments_getUniqueStarGiftValueInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getUniqueStarGiftValueInfo");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

payments_getUniqueStarGiftValueInfo::ReturnType payments_getUniqueStarGiftValueInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_uniqueStarGiftValueInfo>, 1362093126>::parse(p);
#undef FAIL
}

payments_toggleChatStarGiftNotifications::payments_toggleChatStarGiftNotifications(int32 flags_, bool enabled_, object_ptr<InputPeer> &&peer_)
  : flags_(flags_)
  , enabled_(enabled_)
  , peer_(std::move(peer_))
{}

const std::int32_t payments_toggleChatStarGiftNotifications::ID;

void payments_toggleChatStarGiftNotifications::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1626009505);
  TlStoreBinary::store((var0 = flags_ | (enabled_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_toggleChatStarGiftNotifications::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1626009505);
  TlStoreBinary::store((var0 = flags_ | (enabled_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_toggleChatStarGiftNotifications::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.toggleChatStarGiftNotifications");
    s.store_field("flags", (var0 = flags_ | (enabled_ << 0)));
    if (var0 & 1) { s.store_field("enabled", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

payments_toggleChatStarGiftNotifications::ReturnType payments_toggleChatStarGiftNotifications::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_validateRequestedInfo::payments_validateRequestedInfo(int32 flags_, bool save_, object_ptr<InputInvoice> &&invoice_, object_ptr<paymentRequestedInfo> &&info_)
  : flags_(flags_)
  , save_(save_)
  , invoice_(std::move(invoice_))
  , info_(std::move(info_))
{}

const std::int32_t payments_validateRequestedInfo::ID;

void payments_validateRequestedInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1228345045);
  TlStoreBinary::store((var0 = flags_ | (save_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
  TlStoreBoxed<TlStoreObject, -1868808300>::store(info_, s);
}

void payments_validateRequestedInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1228345045);
  TlStoreBinary::store((var0 = flags_ | (save_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
  TlStoreBoxed<TlStoreObject, -1868808300>::store(info_, s);
}

void payments_validateRequestedInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.validateRequestedInfo");
    s.store_field("flags", (var0 = flags_ | (save_ << 0)));
    if (var0 & 1) { s.store_field("save", true); }
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_object_field("info", static_cast<const BaseObject *>(info_.get()));
    s.store_class_end();
  }
}

payments_validateRequestedInfo::ReturnType payments_validateRequestedInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_validatedRequestedInfo>, -784000893>::parse(p);
#undef FAIL
}

phone_deleteConferenceCallParticipants::phone_deleteConferenceCallParticipants(int32 flags_, bool only_left_, bool kick_, object_ptr<InputGroupCall> &&call_, array<int64> &&ids_, bytes &&block_)
  : flags_(flags_)
  , only_left_(only_left_)
  , kick_(kick_)
  , call_(std::move(call_))
  , ids_(std::move(ids_))
  , block_(std::move(block_))
{}

const std::int32_t phone_deleteConferenceCallParticipants::ID;

void phone_deleteConferenceCallParticipants::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1935276763);
  TlStoreBinary::store((var0 = flags_ | (only_left_ << 0) | (kick_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(ids_, s);
  TlStoreString::store(block_, s);
}

void phone_deleteConferenceCallParticipants::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1935276763);
  TlStoreBinary::store((var0 = flags_ | (only_left_ << 0) | (kick_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(ids_, s);
  TlStoreString::store(block_, s);
}

void phone_deleteConferenceCallParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.deleteConferenceCallParticipants");
    s.store_field("flags", (var0 = flags_ | (only_left_ << 0) | (kick_ << 1)));
    if (var0 & 1) { s.store_field("only_left", true); }
    if (var0 & 2) { s.store_field("kick", true); }
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    { s.store_vector_begin("ids", ids_.size()); for (const auto &_value : ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_bytes_field("block", block_);
    s.store_class_end();
  }
}

phone_deleteConferenceCallParticipants::ReturnType phone_deleteConferenceCallParticipants::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_getGroupCall::phone_getGroupCall(object_ptr<InputGroupCall> &&call_, int32 limit_)
  : call_(std::move(call_))
  , limit_(limit_)
{}

const std::int32_t phone_getGroupCall::ID;

void phone_getGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(68699611);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(limit_, s);
}

void phone_getGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(68699611);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(limit_, s);
}

void phone_getGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.getGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

phone_getGroupCall::ReturnType phone_getGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_groupCall>, -1636664659>::parse(p);
#undef FAIL
}

phone_getGroupCallStreamRtmpUrl::phone_getGroupCallStreamRtmpUrl(object_ptr<InputPeer> &&peer_, bool revoke_)
  : peer_(std::move(peer_))
  , revoke_(revoke_)
{}

const std::int32_t phone_getGroupCallStreamRtmpUrl::ID;

void phone_getGroupCallStreamRtmpUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-558650433);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(revoke_, s);
}

void phone_getGroupCallStreamRtmpUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-558650433);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(revoke_, s);
}

void phone_getGroupCallStreamRtmpUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.getGroupCallStreamRtmpUrl");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("revoke", revoke_);
    s.store_class_end();
  }
}

phone_getGroupCallStreamRtmpUrl::ReturnType phone_getGroupCallStreamRtmpUrl::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_groupCallStreamRtmpUrl>, 767505458>::parse(p);
#undef FAIL
}

phone_receivedCall::phone_receivedCall(object_ptr<inputPhoneCall> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t phone_receivedCall::ID;

void phone_receivedCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(399855457);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
}

void phone_receivedCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(399855457);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
}

void phone_receivedCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.receivedCall");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

phone_receivedCall::ReturnType phone_receivedCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

phone_saveDefaultGroupCallJoinAs::phone_saveDefaultGroupCallJoinAs(object_ptr<InputPeer> &&peer_, object_ptr<InputPeer> &&join_as_)
  : peer_(std::move(peer_))
  , join_as_(std::move(join_as_))
{}

const std::int32_t phone_saveDefaultGroupCallJoinAs::ID;

void phone_saveDefaultGroupCallJoinAs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1465786252);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(join_as_, s);
}

void phone_saveDefaultGroupCallJoinAs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1465786252);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(join_as_, s);
}

void phone_saveDefaultGroupCallJoinAs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.saveDefaultGroupCallJoinAs");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("join_as", static_cast<const BaseObject *>(join_as_.get()));
    s.store_class_end();
  }
}

phone_saveDefaultGroupCallJoinAs::ReturnType phone_saveDefaultGroupCallJoinAs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

photos_getUserPhotos::photos_getUserPhotos(object_ptr<InputUser> &&user_id_, int32 offset_, int64 max_id_, int32 limit_)
  : user_id_(std::move(user_id_))
  , offset_(offset_)
  , max_id_(max_id_)
  , limit_(limit_)
{}

const std::int32_t photos_getUserPhotos::ID;

void photos_getUserPhotos::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1848823128);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(limit_, s);
}

void photos_getUserPhotos::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1848823128);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(limit_, s);
}

void photos_getUserPhotos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.getUserPhotos");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("offset", offset_);
    s.store_field("max_id", max_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

photos_getUserPhotos::ReturnType photos_getUserPhotos::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<photos_Photos>::parse(p);
#undef FAIL
}

const std::int32_t premium_getMyBoosts::ID;

void premium_getMyBoosts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(199719754);
}

void premium_getMyBoosts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(199719754);
}

void premium_getMyBoosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.getMyBoosts");
    s.store_class_end();
  }
}

premium_getMyBoosts::ReturnType premium_getMyBoosts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<premium_myBoosts>, -1696454430>::parse(p);
#undef FAIL
}

stats_getMegagroupStats::stats_getMegagroupStats(int32 flags_, bool dark_, object_ptr<InputChannel> &&channel_)
  : flags_(flags_)
  , dark_(dark_)
  , channel_(std::move(channel_))
{}

const std::int32_t stats_getMegagroupStats::ID;

void stats_getMegagroupStats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-589330937);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void stats_getMegagroupStats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-589330937);
  TlStoreBinary::store((var0 = flags_ | (dark_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void stats_getMegagroupStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.getMegagroupStats");
    s.store_field("flags", (var0 = flags_ | (dark_ << 0)));
    if (var0 & 1) { s.store_field("dark", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

stats_getMegagroupStats::ReturnType stats_getMegagroupStats::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stats_megagroupStats>, -276825834>::parse(p);
#undef FAIL
}

stickers_changeSticker::stickers_changeSticker(int32 flags_, object_ptr<InputDocument> &&sticker_, string const &emoji_, object_ptr<maskCoords> &&mask_coords_, string const &keywords_)
  : flags_(flags_)
  , sticker_(std::move(sticker_))
  , emoji_(emoji_)
  , mask_coords_(std::move(mask_coords_))
  , keywords_(keywords_)
{}

const std::int32_t stickers_changeSticker::ID;

void stickers_changeSticker::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-179077444);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
  if (var0 & 1) { TlStoreString::store(emoji_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, -1361650766>::store(mask_coords_, s); }
  if (var0 & 4) { TlStoreString::store(keywords_, s); }
}

void stickers_changeSticker::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-179077444);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
  if (var0 & 1) { TlStoreString::store(emoji_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, -1361650766>::store(mask_coords_, s); }
  if (var0 & 4) { TlStoreString::store(keywords_, s); }
}

void stickers_changeSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.changeSticker");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    if (var0 & 1) { s.store_field("emoji", emoji_); }
    if (var0 & 2) { s.store_object_field("mask_coords", static_cast<const BaseObject *>(mask_coords_.get())); }
    if (var0 & 4) { s.store_field("keywords", keywords_); }
    s.store_class_end();
  }
}

stickers_changeSticker::ReturnType stickers_changeSticker::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stickers_removeStickerFromSet::stickers_removeStickerFromSet(object_ptr<InputDocument> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t stickers_removeStickerFromSet::ID;

void stickers_removeStickerFromSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-143257775);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
}

void stickers_removeStickerFromSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-143257775);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
}

void stickers_removeStickerFromSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.removeStickerFromSet");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

stickers_removeStickerFromSet::ReturnType stickers_removeStickerFromSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stickers_renameStickerSet::stickers_renameStickerSet(object_ptr<InputStickerSet> &&stickerset_, string const &title_)
  : stickerset_(std::move(stickerset_))
  , title_(title_)
{}

const std::int32_t stickers_renameStickerSet::ID;

void stickers_renameStickerSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(306912256);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreString::store(title_, s);
}

void stickers_renameStickerSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(306912256);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreString::store(title_, s);
}

void stickers_renameStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.renameStickerSet");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_field("title", title_);
    s.store_class_end();
  }
}

stickers_renameStickerSet::ReturnType stickers_renameStickerSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stories_toggleAllStoriesHidden::stories_toggleAllStoriesHidden(bool hidden_)
  : hidden_(hidden_)
{}

const std::int32_t stories_toggleAllStoriesHidden::ID;

void stories_toggleAllStoriesHidden::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2082822084);
  TlStoreBool::store(hidden_, s);
}

void stories_toggleAllStoriesHidden::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2082822084);
  TlStoreBool::store(hidden_, s);
}

void stories_toggleAllStoriesHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.toggleAllStoriesHidden");
    s.store_field("hidden", hidden_);
    s.store_class_end();
  }
}

stories_toggleAllStoriesHidden::ReturnType stories_toggleAllStoriesHidden::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stories_togglePinned::stories_togglePinned(object_ptr<InputPeer> &&peer_, array<int32> &&id_, bool pinned_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
  , pinned_(pinned_)
{}

const std::int32_t stories_togglePinned::ID;

void stories_togglePinned::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1703566865);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBool::store(pinned_, s);
}

void stories_togglePinned::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1703566865);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBool::store(pinned_, s);
}

void stories_togglePinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.togglePinned");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("pinned", pinned_);
    s.store_class_end();
  }
}

stories_togglePinned::ReturnType stories_togglePinned::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
#undef FAIL
}

const std::int32_t test_useConfigSimple::ID;

void test_useConfigSimple::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-105401795);
}

void test_useConfigSimple::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-105401795);
}

void test_useConfigSimple::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "test.useConfigSimple");
    s.store_class_end();
  }
}

test_useConfigSimple::ReturnType test_useConfigSimple::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<help_configSimple>, 1515793004>::parse(p);
#undef FAIL
}

const std::int32_t updates_getState::ID;

void updates_getState::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-304838614);
}

void updates_getState::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-304838614);
}

void updates_getState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.getState");
    s.store_class_end();
  }
}

updates_getState::ReturnType updates_getState::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<updates_state>, -1519637954>::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
