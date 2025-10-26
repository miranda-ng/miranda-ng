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

std::string to_string(const BaseObject &value) {
  TlStorerToString storer;
  value.store(storer, "");
  return storer.move_as_string();
}

attachMenuBot::attachMenuBot()
  : flags_()
  , inactive_()
  , has_settings_()
  , request_write_access_()
  , show_in_attach_menu_()
  , show_in_side_menu_()
  , side_menu_disclaimer_needed_()
  , bot_id_()
  , short_name_()
  , peer_types_()
  , icons_()
{}

const std::int32_t attachMenuBot::ID;

object_ptr<attachMenuBot> attachMenuBot::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<attachMenuBot> res = make_tl_object<attachMenuBot>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->inactive_ = (var0 & 1) != 0;
  res->has_settings_ = (var0 & 2) != 0;
  res->request_write_access_ = (var0 & 4) != 0;
  res->show_in_attach_menu_ = (var0 & 8) != 0;
  res->show_in_side_menu_ = (var0 & 16) != 0;
  res->side_menu_disclaimer_needed_ = (var0 & 32) != 0;
  res->bot_id_ = TlFetchLong::parse(p);
  res->short_name_ = TlFetchString<string>::parse(p);
  if (var0 & 8) { res->peer_types_ = TlFetchBoxed<TlFetchVector<TlFetchObject<AttachMenuPeerType>>, 481674261>::parse(p); }
  res->icons_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<attachMenuBotIcon>, -1297663893>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void attachMenuBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuBot");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (inactive_ << 0) | (has_settings_ << 1) | (request_write_access_ << 2) | (show_in_attach_menu_ << 3) | (show_in_side_menu_ << 4) | (side_menu_disclaimer_needed_ << 5)));
    if (var0 & 1) { s.store_field("inactive", true); }
    if (var0 & 2) { s.store_field("has_settings", true); }
    if (var0 & 4) { s.store_field("request_write_access", true); }
    if (var0 & 8) { s.store_field("show_in_attach_menu", true); }
    if (var0 & 16) { s.store_field("show_in_side_menu", true); }
    if (var0 & 32) { s.store_field("side_menu_disclaimer_needed", true); }
    s.store_field("bot_id", bot_id_);
    s.store_field("short_name", short_name_);
    if (var0 & 8) { { s.store_vector_begin("peer_types", peer_types_.size()); for (const auto &_value : peer_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    { s.store_vector_begin("icons", icons_.size()); for (const auto &_value : icons_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t attachMenuBotIconColor::ID;

object_ptr<attachMenuBotIconColor> attachMenuBotIconColor::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuBotIconColor>(p);
}

attachMenuBotIconColor::attachMenuBotIconColor(TlBufferParser &p)
  : name_(TlFetchString<string>::parse(p))
  , color_(TlFetchInt::parse(p))
{}

void attachMenuBotIconColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuBotIconColor");
    s.store_field("name", name_);
    s.store_field("color", color_);
    s.store_class_end();
  }
}

const std::int32_t attachMenuBotsBot::ID;

object_ptr<attachMenuBotsBot> attachMenuBotsBot::fetch(TlBufferParser &p) {
  return make_tl_object<attachMenuBotsBot>(p);
}

attachMenuBotsBot::attachMenuBotsBot(TlBufferParser &p)
  : bot_(TlFetchBoxed<TlFetchObject<attachMenuBot>, -653423106>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void attachMenuBotsBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuBotsBot");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

botBusinessConnection::botBusinessConnection()
  : flags_()
  , disabled_()
  , connection_id_()
  , user_id_()
  , dc_id_()
  , date_()
  , rights_()
{}

const std::int32_t botBusinessConnection::ID;

object_ptr<botBusinessConnection> botBusinessConnection::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botBusinessConnection> res = make_tl_object<botBusinessConnection>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->disabled_ = (var0 & 2) != 0;
  res->connection_id_ = TlFetchString<string>::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  res->dc_id_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->rights_ = TlFetchBoxed<TlFetchObject<businessBotRights>, -1604170505>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void botBusinessConnection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botBusinessConnection");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (disabled_ << 1)));
    if (var0 & 2) { s.store_field("disabled", true); }
    s.store_field("connection_id", connection_id_);
    s.store_field("user_id", user_id_);
    s.store_field("dc_id", dc_id_);
    s.store_field("date", date_);
    if (var0 & 4) { s.store_object_field("rights", static_cast<const BaseObject *>(rights_.get())); }
    s.store_class_end();
  }
}

botInfo::botInfo()
  : flags_()
  , has_preview_medias_()
  , user_id_()
  , description_()
  , description_photo_()
  , description_document_()
  , commands_()
  , menu_button_()
  , privacy_policy_url_()
  , app_settings_()
  , verifier_settings_()
{}

const std::int32_t botInfo::ID;

object_ptr<botInfo> botInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInfo> res = make_tl_object<botInfo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_preview_medias_ = (var0 & 64) != 0;
  if (var0 & 1) { res->user_id_ = TlFetchLong::parse(p); }
  if (var0 & 2) { res->description_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->description_photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 32) { res->description_document_ = TlFetchObject<Document>::parse(p); }
  if (var0 & 4) { res->commands_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<botCommand>, -1032140601>>, 481674261>::parse(p); }
  if (var0 & 8) { res->menu_button_ = TlFetchObject<BotMenuButton>::parse(p); }
  if (var0 & 128) { res->privacy_policy_url_ = TlFetchString<string>::parse(p); }
  if (var0 & 256) { res->app_settings_ = TlFetchBoxed<TlFetchObject<botAppSettings>, -912582320>::parse(p); }
  if (var0 & 512) { res->verifier_settings_ = TlFetchBoxed<TlFetchObject<botVerifierSettings>, -1328716265>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void botInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInfo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_preview_medias_ << 6)));
    if (var0 & 64) { s.store_field("has_preview_medias", true); }
    if (var0 & 1) { s.store_field("user_id", user_id_); }
    if (var0 & 2) { s.store_field("description", description_); }
    if (var0 & 16) { s.store_object_field("description_photo", static_cast<const BaseObject *>(description_photo_.get())); }
    if (var0 & 32) { s.store_object_field("description_document", static_cast<const BaseObject *>(description_document_.get())); }
    if (var0 & 4) { { s.store_vector_begin("commands", commands_.size()); for (const auto &_value : commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 8) { s.store_object_field("menu_button", static_cast<const BaseObject *>(menu_button_.get())); }
    if (var0 & 128) { s.store_field("privacy_policy_url", privacy_policy_url_); }
    if (var0 & 256) { s.store_object_field("app_settings", static_cast<const BaseObject *>(app_settings_.get())); }
    if (var0 & 512) { s.store_object_field("verifier_settings", static_cast<const BaseObject *>(verifier_settings_.get())); }
    s.store_class_end();
  }
}

const std::int32_t botPreviewMedia::ID;

object_ptr<botPreviewMedia> botPreviewMedia::fetch(TlBufferParser &p) {
  return make_tl_object<botPreviewMedia>(p);
}

botPreviewMedia::botPreviewMedia(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
  , media_(TlFetchObject<MessageMedia>::parse(p))
{}

void botPreviewMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botPreviewMedia");
    s.store_field("date", date_);
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

businessAwayMessage::businessAwayMessage()
  : flags_()
  , offline_only_()
  , shortcut_id_()
  , schedule_()
  , recipients_()
{}

const std::int32_t businessAwayMessage::ID;

object_ptr<businessAwayMessage> businessAwayMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessAwayMessage> res = make_tl_object<businessAwayMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->offline_only_ = (var0 & 1) != 0;
  res->shortcut_id_ = TlFetchInt::parse(p);
  res->schedule_ = TlFetchObject<BusinessAwayMessageSchedule>::parse(p);
  res->recipients_ = TlFetchBoxed<TlFetchObject<businessRecipients>, 554733559>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessAwayMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (offline_only_ << 0)));
    if (var0 & 1) { s.store_field("offline_only", true); }
    s.store_field("shortcut_id", shortcut_id_);
    s.store_object_field("schedule", static_cast<const BaseObject *>(schedule_.get()));
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_class_end();
  }
}

channelAdminLogEventsFilter::channelAdminLogEventsFilter(int32 flags_, bool join_, bool leave_, bool invite_, bool ban_, bool unban_, bool kick_, bool unkick_, bool promote_, bool demote_, bool info_, bool settings_, bool pinned_, bool edit_, bool delete_, bool group_call_, bool invites_, bool send_, bool forums_, bool sub_extend_)
  : flags_(flags_)
  , join_(join_)
  , leave_(leave_)
  , invite_(invite_)
  , ban_(ban_)
  , unban_(unban_)
  , kick_(kick_)
  , unkick_(unkick_)
  , promote_(promote_)
  , demote_(demote_)
  , info_(info_)
  , settings_(settings_)
  , pinned_(pinned_)
  , edit_(edit_)
  , delete_(delete_)
  , group_call_(group_call_)
  , invites_(invites_)
  , send_(send_)
  , forums_(forums_)
  , sub_extend_(sub_extend_)
{}

const std::int32_t channelAdminLogEventsFilter::ID;

void channelAdminLogEventsFilter::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (join_ << 0) | (leave_ << 1) | (invite_ << 2) | (ban_ << 3) | (unban_ << 4) | (kick_ << 5) | (unkick_ << 6) | (promote_ << 7) | (demote_ << 8) | (info_ << 9) | (settings_ << 10) | (pinned_ << 11) | (edit_ << 12) | (delete_ << 13) | (group_call_ << 14) | (invites_ << 15) | (send_ << 16) | (forums_ << 17) | (sub_extend_ << 18)), s);
}

void channelAdminLogEventsFilter::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (join_ << 0) | (leave_ << 1) | (invite_ << 2) | (ban_ << 3) | (unban_ << 4) | (kick_ << 5) | (unkick_ << 6) | (promote_ << 7) | (demote_ << 8) | (info_ << 9) | (settings_ << 10) | (pinned_ << 11) | (edit_ << 12) | (delete_ << 13) | (group_call_ << 14) | (invites_ << 15) | (send_ << 16) | (forums_ << 17) | (sub_extend_ << 18)), s);
}

void channelAdminLogEventsFilter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventsFilter");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (join_ << 0) | (leave_ << 1) | (invite_ << 2) | (ban_ << 3) | (unban_ << 4) | (kick_ << 5) | (unkick_ << 6) | (promote_ << 7) | (demote_ << 8) | (info_ << 9) | (settings_ << 10) | (pinned_ << 11) | (edit_ << 12) | (delete_ << 13) | (group_call_ << 14) | (invites_ << 15) | (send_ << 16) | (forums_ << 17) | (sub_extend_ << 18)));
    if (var0 & 1) { s.store_field("join", true); }
    if (var0 & 2) { s.store_field("leave", true); }
    if (var0 & 4) { s.store_field("invite", true); }
    if (var0 & 8) { s.store_field("ban", true); }
    if (var0 & 16) { s.store_field("unban", true); }
    if (var0 & 32) { s.store_field("kick", true); }
    if (var0 & 64) { s.store_field("unkick", true); }
    if (var0 & 128) { s.store_field("promote", true); }
    if (var0 & 256) { s.store_field("demote", true); }
    if (var0 & 512) { s.store_field("info", true); }
    if (var0 & 1024) { s.store_field("settings", true); }
    if (var0 & 2048) { s.store_field("pinned", true); }
    if (var0 & 4096) { s.store_field("edit", true); }
    if (var0 & 8192) { s.store_field("delete", true); }
    if (var0 & 16384) { s.store_field("group_call", true); }
    if (var0 & 32768) { s.store_field("invites", true); }
    if (var0 & 65536) { s.store_field("send", true); }
    if (var0 & 131072) { s.store_field("forums", true); }
    if (var0 & 262144) { s.store_field("sub_extend", true); }
    s.store_class_end();
  }
}

const std::int32_t defaultHistoryTTL::ID;

object_ptr<defaultHistoryTTL> defaultHistoryTTL::fetch(TlBufferParser &p) {
  return make_tl_object<defaultHistoryTTL>(p);
}

defaultHistoryTTL::defaultHistoryTTL(TlBufferParser &p)
  : period_(TlFetchInt::parse(p))
{}

void defaultHistoryTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "defaultHistoryTTL");
    s.store_field("period", period_);
    s.store_class_end();
  }
}

object_ptr<GeoPoint> GeoPoint::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case geoPointEmpty::ID:
      return geoPointEmpty::fetch(p);
    case geoPoint::ID:
      return geoPoint::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t geoPointEmpty::ID;

object_ptr<GeoPoint> geoPointEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<geoPointEmpty>();
}

void geoPointEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void geoPointEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void geoPointEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "geoPointEmpty");
    s.store_class_end();
  }
}

geoPoint::geoPoint()
  : flags_()
  , long_()
  , lat_()
  , access_hash_()
  , accuracy_radius_()
{}

geoPoint::geoPoint(int32 flags_, double long_, double lat_, int64 access_hash_, int32 accuracy_radius_)
  : flags_(flags_)
  , long_(long_)
  , lat_(lat_)
  , access_hash_(access_hash_)
  , accuracy_radius_(accuracy_radius_)
{}

const std::int32_t geoPoint::ID;

object_ptr<GeoPoint> geoPoint::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<geoPoint> res = make_tl_object<geoPoint>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->long_ = TlFetchDouble::parse(p);
  res->lat_ = TlFetchDouble::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->accuracy_radius_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void geoPoint::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(long_, s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(access_hash_, s);
  if (var0 & 1) { TlStoreBinary::store(accuracy_radius_, s); }
}

void geoPoint::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(long_, s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(access_hash_, s);
  if (var0 & 1) { TlStoreBinary::store(accuracy_radius_, s); }
}

void geoPoint::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "geoPoint");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("long", long_);
    s.store_field("lat", lat_);
    s.store_field("access_hash", access_hash_);
    if (var0 & 1) { s.store_field("accuracy_radius", accuracy_radius_); }
    s.store_class_end();
  }
}

inputAppEvent::inputAppEvent(double time_, string const &type_, int64 peer_, object_ptr<JSONValue> &&data_)
  : time_(time_)
  , type_(type_)
  , peer_(peer_)
  , data_(std::move(data_))
{}

const std::int32_t inputAppEvent::ID;

object_ptr<inputAppEvent> inputAppEvent::fetch(TlBufferParser &p) {
  return make_tl_object<inputAppEvent>(p);
}

inputAppEvent::inputAppEvent(TlBufferParser &p)
  : time_(TlFetchDouble::parse(p))
  , type_(TlFetchString<string>::parse(p))
  , peer_(TlFetchLong::parse(p))
  , data_(TlFetchObject<JSONValue>::parse(p))
{}

void inputAppEvent::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(time_, s);
  TlStoreString::store(type_, s);
  TlStoreBinary::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(data_, s);
}

void inputAppEvent::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(time_, s);
  TlStoreString::store(type_, s);
  TlStoreBinary::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(data_, s);
}

void inputAppEvent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputAppEvent");
    s.store_field("time", time_);
    s.store_field("type", type_);
    s.store_field("peer", peer_);
    s.store_object_field("data", static_cast<const BaseObject *>(data_.get()));
    s.store_class_end();
  }
}

inputBusinessAwayMessage::inputBusinessAwayMessage(int32 flags_, bool offline_only_, int32 shortcut_id_, object_ptr<BusinessAwayMessageSchedule> &&schedule_, object_ptr<inputBusinessRecipients> &&recipients_)
  : flags_(flags_)
  , offline_only_(offline_only_)
  , shortcut_id_(shortcut_id_)
  , schedule_(std::move(schedule_))
  , recipients_(std::move(recipients_))
{}

const std::int32_t inputBusinessAwayMessage::ID;

void inputBusinessAwayMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (offline_only_ << 0)), s);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(schedule_, s);
  TlStoreBoxed<TlStoreObject, 1871393450>::store(recipients_, s);
}

void inputBusinessAwayMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (offline_only_ << 0)), s);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(schedule_, s);
  TlStoreBoxed<TlStoreObject, 1871393450>::store(recipients_, s);
}

void inputBusinessAwayMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessAwayMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (offline_only_ << 0)));
    if (var0 & 1) { s.store_field("offline_only", true); }
    s.store_field("shortcut_id", shortcut_id_);
    s.store_object_field("schedule", static_cast<const BaseObject *>(schedule_.get()));
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_class_end();
  }
}

inputPeerPhotoFileLocationLegacy::inputPeerPhotoFileLocationLegacy(int32 flags_, bool big_, object_ptr<InputPeer> &&peer_, int64 volume_id_, int32 local_id_)
  : flags_(flags_)
  , big_(big_)
  , peer_(std::move(peer_))
  , volume_id_(volume_id_)
  , local_id_(local_id_)
{}

const std::int32_t inputPeerPhotoFileLocationLegacy::ID;

void inputPeerPhotoFileLocationLegacy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (big_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
}

void inputPeerPhotoFileLocationLegacy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (big_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
}

void inputPeerPhotoFileLocationLegacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerPhotoFileLocationLegacy");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (big_ << 0)));
    if (var0 & 1) { s.store_field("big", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("volume_id", volume_id_);
    s.store_field("local_id", local_id_);
    s.store_class_end();
  }
}

inputStickerSetThumbLegacy::inputStickerSetThumbLegacy(object_ptr<InputStickerSet> &&stickerset_, int64 volume_id_, int32 local_id_)
  : stickerset_(std::move(stickerset_))
  , volume_id_(volume_id_)
  , local_id_(local_id_)
{}

const std::int32_t inputStickerSetThumbLegacy::ID;

void inputStickerSetThumbLegacy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
}

void inputStickerSetThumbLegacy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
}

void inputStickerSetThumbLegacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetThumbLegacy");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_field("volume_id", volume_id_);
    s.store_field("local_id", local_id_);
    s.store_class_end();
  }
}

inputFileLocation::inputFileLocation(int64 volume_id_, int32 local_id_, int64 secret_, bytes &&file_reference_)
  : volume_id_(volume_id_)
  , local_id_(local_id_)
  , secret_(secret_)
  , file_reference_(std::move(file_reference_))
{}

const std::int32_t inputFileLocation::ID;

void inputFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
  TlStoreString::store(file_reference_, s);
}

void inputFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
  TlStoreString::store(file_reference_, s);
}

void inputFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFileLocation");
    s.store_field("volume_id", volume_id_);
    s.store_field("local_id", local_id_);
    s.store_field("secret", secret_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_class_end();
  }
}

inputEncryptedFileLocation::inputEncryptedFileLocation(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputEncryptedFileLocation::ID;

void inputEncryptedFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputEncryptedFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputEncryptedFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputEncryptedFileLocation");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputDocumentFileLocation::inputDocumentFileLocation(int64 id_, int64 access_hash_, bytes &&file_reference_, string const &thumb_size_)
  : id_(id_)
  , access_hash_(access_hash_)
  , file_reference_(std::move(file_reference_))
  , thumb_size_(thumb_size_)
{}

const std::int32_t inputDocumentFileLocation::ID;

void inputDocumentFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
  TlStoreString::store(thumb_size_, s);
}

void inputDocumentFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
  TlStoreString::store(thumb_size_, s);
}

void inputDocumentFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputDocumentFileLocation");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_field("thumb_size", thumb_size_);
    s.store_class_end();
  }
}

inputSecureFileLocation::inputSecureFileLocation(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputSecureFileLocation::ID;

void inputSecureFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputSecureFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputSecureFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSecureFileLocation");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

const std::int32_t inputTakeoutFileLocation::ID;

void inputTakeoutFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputTakeoutFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputTakeoutFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputTakeoutFileLocation");
    s.store_class_end();
  }
}

inputPhotoFileLocation::inputPhotoFileLocation(int64 id_, int64 access_hash_, bytes &&file_reference_, string const &thumb_size_)
  : id_(id_)
  , access_hash_(access_hash_)
  , file_reference_(std::move(file_reference_))
  , thumb_size_(thumb_size_)
{}

const std::int32_t inputPhotoFileLocation::ID;

void inputPhotoFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
  TlStoreString::store(thumb_size_, s);
}

void inputPhotoFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
  TlStoreString::store(thumb_size_, s);
}

void inputPhotoFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPhotoFileLocation");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_field("thumb_size", thumb_size_);
    s.store_class_end();
  }
}

inputPhotoLegacyFileLocation::inputPhotoLegacyFileLocation(int64 id_, int64 access_hash_, bytes &&file_reference_, int64 volume_id_, int32 local_id_, int64 secret_)
  : id_(id_)
  , access_hash_(access_hash_)
  , file_reference_(std::move(file_reference_))
  , volume_id_(volume_id_)
  , local_id_(local_id_)
  , secret_(secret_)
{}

const std::int32_t inputPhotoLegacyFileLocation::ID;

void inputPhotoLegacyFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
}

void inputPhotoLegacyFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
}

void inputPhotoLegacyFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPhotoLegacyFileLocation");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_field("volume_id", volume_id_);
    s.store_field("local_id", local_id_);
    s.store_field("secret", secret_);
    s.store_class_end();
  }
}

inputPeerPhotoFileLocation::inputPeerPhotoFileLocation(int32 flags_, bool big_, object_ptr<InputPeer> &&peer_, int64 photo_id_)
  : flags_(flags_)
  , big_(big_)
  , peer_(std::move(peer_))
  , photo_id_(photo_id_)
{}

const std::int32_t inputPeerPhotoFileLocation::ID;

void inputPeerPhotoFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (big_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(photo_id_, s);
}

void inputPeerPhotoFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (big_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(photo_id_, s);
}

void inputPeerPhotoFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerPhotoFileLocation");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (big_ << 0)));
    if (var0 & 1) { s.store_field("big", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("photo_id", photo_id_);
    s.store_class_end();
  }
}

inputStickerSetThumb::inputStickerSetThumb(object_ptr<InputStickerSet> &&stickerset_, int32 thumb_version_)
  : stickerset_(std::move(stickerset_))
  , thumb_version_(thumb_version_)
{}

const std::int32_t inputStickerSetThumb::ID;

void inputStickerSetThumb::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(thumb_version_, s);
}

void inputStickerSetThumb::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(thumb_version_, s);
}

void inputStickerSetThumb::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetThumb");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_field("thumb_version", thumb_version_);
    s.store_class_end();
  }
}

inputGroupCallStream::inputGroupCallStream(int32 flags_, object_ptr<InputGroupCall> &&call_, int64 time_ms_, int32 scale_, int32 video_channel_, int32 video_quality_)
  : flags_(flags_)
  , call_(std::move(call_))
  , time_ms_(time_ms_)
  , scale_(scale_)
  , video_channel_(video_channel_)
  , video_quality_(video_quality_)
{}

const std::int32_t inputGroupCallStream::ID;

void inputGroupCallStream::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(time_ms_, s);
  TlStoreBinary::store(scale_, s);
  if (var0 & 1) { TlStoreBinary::store(video_channel_, s); }
  if (var0 & 1) { TlStoreBinary::store(video_quality_, s); }
}

void inputGroupCallStream::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(time_ms_, s);
  TlStoreBinary::store(scale_, s);
  if (var0 & 1) { TlStoreBinary::store(video_channel_, s); }
  if (var0 & 1) { TlStoreBinary::store(video_quality_, s); }
}

void inputGroupCallStream::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGroupCallStream");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("time_ms", time_ms_);
    s.store_field("scale", scale_);
    if (var0 & 1) { s.store_field("video_channel", video_channel_); }
    if (var0 & 1) { s.store_field("video_quality", video_quality_); }
    s.store_class_end();
  }
}

inputSavedStarGiftUser::inputSavedStarGiftUser(int32 msg_id_)
  : msg_id_(msg_id_)
{}

const std::int32_t inputSavedStarGiftUser::ID;

void inputSavedStarGiftUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
}

void inputSavedStarGiftUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(msg_id_, s);
}

void inputSavedStarGiftUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSavedStarGiftUser");
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

inputSavedStarGiftChat::inputSavedStarGiftChat(object_ptr<InputPeer> &&peer_, int64 saved_id_)
  : peer_(std::move(peer_))
  , saved_id_(saved_id_)
{}

const std::int32_t inputSavedStarGiftChat::ID;

void inputSavedStarGiftChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(saved_id_, s);
}

void inputSavedStarGiftChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(saved_id_, s);
}

void inputSavedStarGiftChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSavedStarGiftChat");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("saved_id", saved_id_);
    s.store_class_end();
  }
}

inputSavedStarGiftSlug::inputSavedStarGiftSlug(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t inputSavedStarGiftSlug::ID;

void inputSavedStarGiftSlug::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputSavedStarGiftSlug::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputSavedStarGiftSlug::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSavedStarGiftSlug");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

inputStorePaymentPremiumSubscription::inputStorePaymentPremiumSubscription(int32 flags_, bool restore_, bool upgrade_)
  : flags_(flags_)
  , restore_(restore_)
  , upgrade_(upgrade_)
{}

const std::int32_t inputStorePaymentPremiumSubscription::ID;

void inputStorePaymentPremiumSubscription::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (restore_ << 0) | (upgrade_ << 1)), s);
}

void inputStorePaymentPremiumSubscription::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (restore_ << 0) | (upgrade_ << 1)), s);
}

void inputStorePaymentPremiumSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentPremiumSubscription");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (restore_ << 0) | (upgrade_ << 1)));
    if (var0 & 1) { s.store_field("restore", true); }
    if (var0 & 2) { s.store_field("upgrade", true); }
    s.store_class_end();
  }
}

inputStorePaymentGiftPremium::inputStorePaymentGiftPremium(object_ptr<InputUser> &&user_id_, string const &currency_, int64 amount_)
  : user_id_(std::move(user_id_))
  , currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t inputStorePaymentGiftPremium::ID;

void inputStorePaymentGiftPremium::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentGiftPremium::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentGiftPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentGiftPremium");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

inputStorePaymentPremiumGiftCode::inputStorePaymentPremiumGiftCode(int32 flags_, array<object_ptr<InputUser>> &&users_, object_ptr<InputPeer> &&boost_peer_, string const &currency_, int64 amount_, object_ptr<textWithEntities> &&message_)
  : flags_(flags_)
  , users_(std::move(users_))
  , boost_peer_(std::move(boost_peer_))
  , currency_(currency_)
  , amount_(amount_)
  , message_(std::move(message_))
{}

const std::int32_t inputStorePaymentPremiumGiftCode::ID;

void inputStorePaymentPremiumGiftCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s); }
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s); }
}

void inputStorePaymentPremiumGiftCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s); }
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s); }
}

void inputStorePaymentPremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentPremiumGiftCode");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_object_field("boost_peer", static_cast<const BaseObject *>(boost_peer_.get())); }
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    if (var0 & 2) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

inputStorePaymentPremiumGiveaway::inputStorePaymentPremiumGiveaway(int32 flags_, bool only_new_subscribers_, bool winners_are_visible_, object_ptr<InputPeer> &&boost_peer_, array<object_ptr<InputPeer>> &&additional_peers_, array<string> &&countries_iso2_, string const &prize_description_, int64 random_id_, int32 until_date_, string const &currency_, int64 amount_)
  : flags_(flags_)
  , only_new_subscribers_(only_new_subscribers_)
  , winners_are_visible_(winners_are_visible_)
  , boost_peer_(std::move(boost_peer_))
  , additional_peers_(std::move(additional_peers_))
  , countries_iso2_(std::move(countries_iso2_))
  , prize_description_(prize_description_)
  , random_id_(random_id_)
  , until_date_(until_date_)
  , currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t inputStorePaymentPremiumGiveaway::ID;

void inputStorePaymentPremiumGiveaway::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (only_new_subscribers_ << 0) | (winners_are_visible_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(additional_peers_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(countries_iso2_, s); }
  if (var0 & 16) { TlStoreString::store(prize_description_, s); }
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(until_date_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentPremiumGiveaway::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (only_new_subscribers_ << 0) | (winners_are_visible_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(additional_peers_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(countries_iso2_, s); }
  if (var0 & 16) { TlStoreString::store(prize_description_, s); }
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(until_date_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentPremiumGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentPremiumGiveaway");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (only_new_subscribers_ << 0) | (winners_are_visible_ << 3)));
    if (var0 & 1) { s.store_field("only_new_subscribers", true); }
    if (var0 & 8) { s.store_field("winners_are_visible", true); }
    s.store_object_field("boost_peer", static_cast<const BaseObject *>(boost_peer_.get()));
    if (var0 & 2) { { s.store_vector_begin("additional_peers", additional_peers_.size()); for (const auto &_value : additional_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { { s.store_vector_begin("countries_iso2", countries_iso2_.size()); for (const auto &_value : countries_iso2_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 16) { s.store_field("prize_description", prize_description_); }
    s.store_field("random_id", random_id_);
    s.store_field("until_date", until_date_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

inputStorePaymentStarsTopup::inputStorePaymentStarsTopup(int32 flags_, int64 stars_, string const &currency_, int64 amount_, object_ptr<InputPeer> &&spend_purpose_peer_)
  : flags_(flags_)
  , stars_(stars_)
  , currency_(currency_)
  , amount_(amount_)
  , spend_purpose_peer_(std::move(spend_purpose_peer_))
{}

const std::int32_t inputStorePaymentStarsTopup::ID;

void inputStorePaymentStarsTopup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(stars_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(spend_purpose_peer_, s); }
}

void inputStorePaymentStarsTopup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(stars_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(spend_purpose_peer_, s); }
}

void inputStorePaymentStarsTopup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentStarsTopup");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("stars", stars_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    if (var0 & 1) { s.store_object_field("spend_purpose_peer", static_cast<const BaseObject *>(spend_purpose_peer_.get())); }
    s.store_class_end();
  }
}

inputStorePaymentStarsGift::inputStorePaymentStarsGift(object_ptr<InputUser> &&user_id_, int64 stars_, string const &currency_, int64 amount_)
  : user_id_(std::move(user_id_))
  , stars_(stars_)
  , currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t inputStorePaymentStarsGift::ID;

void inputStorePaymentStarsGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(stars_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentStarsGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(stars_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentStarsGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentStarsGift");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("stars", stars_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

inputStorePaymentStarsGiveaway::inputStorePaymentStarsGiveaway(int32 flags_, bool only_new_subscribers_, bool winners_are_visible_, int64 stars_, object_ptr<InputPeer> &&boost_peer_, array<object_ptr<InputPeer>> &&additional_peers_, array<string> &&countries_iso2_, string const &prize_description_, int64 random_id_, int32 until_date_, string const &currency_, int64 amount_, int32 users_)
  : flags_(flags_)
  , only_new_subscribers_(only_new_subscribers_)
  , winners_are_visible_(winners_are_visible_)
  , stars_(stars_)
  , boost_peer_(std::move(boost_peer_))
  , additional_peers_(std::move(additional_peers_))
  , countries_iso2_(std::move(countries_iso2_))
  , prize_description_(prize_description_)
  , random_id_(random_id_)
  , until_date_(until_date_)
  , currency_(currency_)
  , amount_(amount_)
  , users_(users_)
{}

const std::int32_t inputStorePaymentStarsGiveaway::ID;

void inputStorePaymentStarsGiveaway::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (only_new_subscribers_ << 0) | (winners_are_visible_ << 3)), s);
  TlStoreBinary::store(stars_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(additional_peers_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(countries_iso2_, s); }
  if (var0 & 16) { TlStoreString::store(prize_description_, s); }
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(until_date_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
  TlStoreBinary::store(users_, s);
}

void inputStorePaymentStarsGiveaway::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (only_new_subscribers_ << 0) | (winners_are_visible_ << 3)), s);
  TlStoreBinary::store(stars_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(boost_peer_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(additional_peers_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(countries_iso2_, s); }
  if (var0 & 16) { TlStoreString::store(prize_description_, s); }
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(until_date_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
  TlStoreBinary::store(users_, s);
}

void inputStorePaymentStarsGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentStarsGiveaway");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (only_new_subscribers_ << 0) | (winners_are_visible_ << 3)));
    if (var0 & 1) { s.store_field("only_new_subscribers", true); }
    if (var0 & 8) { s.store_field("winners_are_visible", true); }
    s.store_field("stars", stars_);
    s.store_object_field("boost_peer", static_cast<const BaseObject *>(boost_peer_.get()));
    if (var0 & 2) { { s.store_vector_begin("additional_peers", additional_peers_.size()); for (const auto &_value : additional_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { { s.store_vector_begin("countries_iso2", countries_iso2_.size()); for (const auto &_value : countries_iso2_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 16) { s.store_field("prize_description", prize_description_); }
    s.store_field("random_id", random_id_);
    s.store_field("until_date", until_date_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("users", users_);
    s.store_class_end();
  }
}

inputStorePaymentAuthCode::inputStorePaymentAuthCode(int32 flags_, bool restore_, string const &phone_number_, string const &phone_code_hash_, string const &currency_, int64 amount_)
  : flags_(flags_)
  , restore_(restore_)
  , phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t inputStorePaymentAuthCode::ID;

void inputStorePaymentAuthCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (restore_ << 0)), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentAuthCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (restore_ << 0)), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(currency_, s);
  TlStoreBinary::store(amount_, s);
}

void inputStorePaymentAuthCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStorePaymentAuthCode");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (restore_ << 0)));
    if (var0 & 1) { s.store_field("restore", true); }
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

inputTheme::inputTheme(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputTheme::ID;

void inputTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputTheme");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputThemeSlug::inputThemeSlug(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t inputThemeSlug::ID;

void inputThemeSlug::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputThemeSlug::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputThemeSlug::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputThemeSlug");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

inputWebDocument::inputWebDocument(string const &url_, int32 size_, string const &mime_type_, array<object_ptr<DocumentAttribute>> &&attributes_)
  : url_(url_)
  , size_(size_)
  , mime_type_(mime_type_)
  , attributes_(std::move(attributes_))
{}

const std::int32_t inputWebDocument::ID;

object_ptr<inputWebDocument> inputWebDocument::fetch(TlBufferParser &p) {
  return make_tl_object<inputWebDocument>(p);
}

inputWebDocument::inputWebDocument(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , size_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , attributes_(TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p))
{}

void inputWebDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(url_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
}

void inputWebDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(url_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
}

void inputWebDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputWebDocument");
    s.store_field("url", url_);
    s.store_field("size", size_);
    s.store_field("mime_type", mime_type_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageReactor::messageReactor()
  : flags_()
  , top_()
  , my_()
  , anonymous_()
  , peer_id_()
  , count_()
{}

const std::int32_t messageReactor::ID;

object_ptr<messageReactor> messageReactor::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageReactor> res = make_tl_object<messageReactor>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->top_ = (var0 & 1) != 0;
  res->my_ = (var0 & 2) != 0;
  res->anonymous_ = (var0 & 4) != 0;
  if (var0 & 8) { res->peer_id_ = TlFetchObject<Peer>::parse(p); }
  res->count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messageReactor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReactor");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (top_ << 0) | (my_ << 1) | (anonymous_ << 2)));
    if (var0 & 1) { s.store_field("top", true); }
    if (var0 & 2) { s.store_field("my", true); }
    if (var0 & 4) { s.store_field("anonymous", true); }
    if (var0 & 8) { s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get())); }
    s.store_field("count", count_);
    s.store_class_end();
  }
}

const std::int32_t pendingSuggestion::ID;

object_ptr<pendingSuggestion> pendingSuggestion::fetch(TlBufferParser &p) {
  return make_tl_object<pendingSuggestion>(p);
}

pendingSuggestion::pendingSuggestion(TlBufferParser &p)
  : suggestion_(TlFetchString<string>::parse(p))
  , title_(TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p))
  , description_(TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void pendingSuggestion::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pendingSuggestion");
    s.store_field("suggestion", suggestion_);
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    s.store_object_field("description", static_cast<const BaseObject *>(description_.get()));
    s.store_field("url", url_);
    s.store_class_end();
  }
}

pollResults::pollResults()
  : flags_()
  , min_()
  , results_()
  , total_voters_()
  , recent_voters_()
  , solution_()
  , solution_entities_()
{}

const std::int32_t pollResults::ID;

object_ptr<pollResults> pollResults::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pollResults> res = make_tl_object<pollResults>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->min_ = (var0 & 1) != 0;
  if (var0 & 2) { res->results_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<pollAnswerVoters>, 997055186>>, 481674261>::parse(p); }
  if (var0 & 4) { res->total_voters_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->recent_voters_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p); }
  if (var0 & 16) { res->solution_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->solution_entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void pollResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pollResults");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (min_ << 0)));
    if (var0 & 1) { s.store_field("min", true); }
    if (var0 & 2) { { s.store_vector_begin("results", results_.size()); for (const auto &_value : results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { s.store_field("total_voters", total_voters_); }
    if (var0 & 8) { { s.store_vector_begin("recent_voters", recent_voters_.size()); for (const auto &_value : recent_voters_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 16) { s.store_field("solution", solution_); }
    if (var0 & 16) { { s.store_vector_begin("solution_entities", solution_entities_.size()); for (const auto &_value : solution_entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<ReportResult> ReportResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case reportResultChooseOption::ID:
      return reportResultChooseOption::fetch(p);
    case reportResultAddComment::ID:
      return reportResultAddComment::fetch(p);
    case reportResultReported::ID:
      return reportResultReported::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t reportResultChooseOption::ID;

object_ptr<ReportResult> reportResultChooseOption::fetch(TlBufferParser &p) {
  return make_tl_object<reportResultChooseOption>(p);
}

reportResultChooseOption::reportResultChooseOption(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , options_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<messageReportOption>, 2030298073>>, 481674261>::parse(p))
{}

void reportResultChooseOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportResultChooseOption");
    s.store_field("title", title_);
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportResultAddComment::reportResultAddComment()
  : flags_()
  , optional_()
  , option_()
{}

const std::int32_t reportResultAddComment::ID;

object_ptr<ReportResult> reportResultAddComment::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<reportResultAddComment> res = make_tl_object<reportResultAddComment>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->optional_ = (var0 & 1) != 0;
  res->option_ = TlFetchBytes<bytes>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void reportResultAddComment::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportResultAddComment");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (optional_ << 0)));
    if (var0 & 1) { s.store_field("optional", true); }
    s.store_bytes_field("option", option_);
    s.store_class_end();
  }
}

const std::int32_t reportResultReported::ID;

object_ptr<ReportResult> reportResultReported::fetch(TlBufferParser &p) {
  return make_tl_object<reportResultReported>();
}

void reportResultReported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportResultReported");
    s.store_class_end();
  }
}

object_ptr<SavedDialog> SavedDialog::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case savedDialog::ID:
      return savedDialog::fetch(p);
    case monoForumDialog::ID:
      return monoForumDialog::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

savedDialog::savedDialog()
  : flags_()
  , pinned_()
  , peer_()
  , top_message_()
{}

const std::int32_t savedDialog::ID;

object_ptr<SavedDialog> savedDialog::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<savedDialog> res = make_tl_object<savedDialog>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 4) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->top_message_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void savedDialog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedDialog");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 2)));
    if (var0 & 4) { s.store_field("pinned", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("top_message", top_message_);
    s.store_class_end();
  }
}

monoForumDialog::monoForumDialog()
  : flags_()
  , unread_mark_()
  , nopaid_messages_exception_()
  , peer_()
  , top_message_()
  , read_inbox_max_id_()
  , read_outbox_max_id_()
  , unread_count_()
  , unread_reactions_count_()
  , draft_()
{}

const std::int32_t monoForumDialog::ID;

object_ptr<SavedDialog> monoForumDialog::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<monoForumDialog> res = make_tl_object<monoForumDialog>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->unread_mark_ = (var0 & 8) != 0;
  res->nopaid_messages_exception_ = (var0 & 16) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->top_message_ = TlFetchInt::parse(p);
  res->read_inbox_max_id_ = TlFetchInt::parse(p);
  res->read_outbox_max_id_ = TlFetchInt::parse(p);
  res->unread_count_ = TlFetchInt::parse(p);
  res->unread_reactions_count_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->draft_ = TlFetchObject<DraftMessage>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void monoForumDialog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "monoForumDialog");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (unread_mark_ << 3) | (nopaid_messages_exception_ << 4)));
    if (var0 & 8) { s.store_field("unread_mark", true); }
    if (var0 & 16) { s.store_field("nopaid_messages_exception", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("top_message", top_message_);
    s.store_field("read_inbox_max_id", read_inbox_max_id_);
    s.store_field("read_outbox_max_id", read_outbox_max_id_);
    s.store_field("unread_count", unread_count_);
    s.store_field("unread_reactions_count", unread_reactions_count_);
    if (var0 & 2) { s.store_object_field("draft", static_cast<const BaseObject *>(draft_.get())); }
    s.store_class_end();
  }
}

savedStarGift::savedStarGift()
  : flags_()
  , name_hidden_()
  , unsaved_()
  , refunded_()
  , can_upgrade_()
  , pinned_to_top_()
  , upgrade_separate_()
  , from_id_()
  , date_()
  , gift_()
  , message_()
  , msg_id_()
  , saved_id_()
  , convert_stars_()
  , upgrade_stars_()
  , can_export_at_()
  , transfer_stars_()
  , can_transfer_at_()
  , can_resell_at_()
  , collection_id_()
  , prepaid_upgrade_hash_()
  , drop_original_details_stars_()
{}

const std::int32_t savedStarGift::ID;

object_ptr<savedStarGift> savedStarGift::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<savedStarGift> res = make_tl_object<savedStarGift>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->name_hidden_ = (var0 & 1) != 0;
  res->unsaved_ = (var0 & 32) != 0;
  res->refunded_ = (var0 & 512) != 0;
  res->can_upgrade_ = (var0 & 1024) != 0;
  res->pinned_to_top_ = (var0 & 4096) != 0;
  res->upgrade_separate_ = (var0 & 131072) != 0;
  if (var0 & 2) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  res->gift_ = TlFetchObject<StarGift>::parse(p);
  if (var0 & 4) { res->message_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p); }
  if (var0 & 8) { res->msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 2048) { res->saved_id_ = TlFetchLong::parse(p); }
  if (var0 & 16) { res->convert_stars_ = TlFetchLong::parse(p); }
  if (var0 & 64) { res->upgrade_stars_ = TlFetchLong::parse(p); }
  if (var0 & 128) { res->can_export_at_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->transfer_stars_ = TlFetchLong::parse(p); }
  if (var0 & 8192) { res->can_transfer_at_ = TlFetchInt::parse(p); }
  if (var0 & 16384) { res->can_resell_at_ = TlFetchInt::parse(p); }
  if (var0 & 32768) { res->collection_id_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  if (var0 & 65536) { res->prepaid_upgrade_hash_ = TlFetchString<string>::parse(p); }
  if (var0 & 262144) { res->drop_original_details_stars_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void savedStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedStarGift");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (name_hidden_ << 0) | (unsaved_ << 5) | (refunded_ << 9) | (can_upgrade_ << 10) | (pinned_to_top_ << 12) | (upgrade_separate_ << 17)));
    if (var0 & 1) { s.store_field("name_hidden", true); }
    if (var0 & 32) { s.store_field("unsaved", true); }
    if (var0 & 512) { s.store_field("refunded", true); }
    if (var0 & 1024) { s.store_field("can_upgrade", true); }
    if (var0 & 4096) { s.store_field("pinned_to_top", true); }
    if (var0 & 131072) { s.store_field("upgrade_separate", true); }
    if (var0 & 2) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    s.store_field("date", date_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    if (var0 & 4) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    if (var0 & 8) { s.store_field("msg_id", msg_id_); }
    if (var0 & 2048) { s.store_field("saved_id", saved_id_); }
    if (var0 & 16) { s.store_field("convert_stars", convert_stars_); }
    if (var0 & 64) { s.store_field("upgrade_stars", upgrade_stars_); }
    if (var0 & 128) { s.store_field("can_export_at", can_export_at_); }
    if (var0 & 256) { s.store_field("transfer_stars", transfer_stars_); }
    if (var0 & 8192) { s.store_field("can_transfer_at", can_transfer_at_); }
    if (var0 & 16384) { s.store_field("can_resell_at", can_resell_at_); }
    if (var0 & 32768) { { s.store_vector_begin("collection_id", collection_id_.size()); for (const auto &_value : collection_id_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 65536) { s.store_field("prepaid_upgrade_hash", prepaid_upgrade_hash_); }
    if (var0 & 262144) { s.store_field("drop_original_details_stars", drop_original_details_stars_); }
    s.store_class_end();
  }
}

secureData::secureData(bytes &&data_, bytes &&data_hash_, bytes &&secret_)
  : data_(std::move(data_))
  , data_hash_(std::move(data_hash_))
  , secret_(std::move(secret_))
{}

const std::int32_t secureData::ID;

object_ptr<secureData> secureData::fetch(TlBufferParser &p) {
  return make_tl_object<secureData>(p);
}

secureData::secureData(TlBufferParser &p)
  : data_(TlFetchBytes<bytes>::parse(p))
  , data_hash_(TlFetchBytes<bytes>::parse(p))
  , secret_(TlFetchBytes<bytes>::parse(p))
{}

void secureData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(data_, s);
  TlStoreString::store(data_hash_, s);
  TlStoreString::store(secret_, s);
}

void secureData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(data_, s);
  TlStoreString::store(data_hash_, s);
  TlStoreString::store(secret_, s);
}

void secureData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureData");
    s.store_bytes_field("data", data_);
    s.store_bytes_field("data_hash", data_hash_);
    s.store_bytes_field("secret", secret_);
    s.store_class_end();
  }
}

starsGiveawayOption::starsGiveawayOption()
  : flags_()
  , extended_()
  , default_()
  , stars_()
  , yearly_boosts_()
  , store_product_()
  , currency_()
  , amount_()
  , winners_()
{}

const std::int32_t starsGiveawayOption::ID;

object_ptr<starsGiveawayOption> starsGiveawayOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsGiveawayOption> res = make_tl_object<starsGiveawayOption>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->extended_ = (var0 & 1) != 0;
  res->default_ = (var0 & 2) != 0;
  res->stars_ = TlFetchLong::parse(p);
  res->yearly_boosts_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->store_product_ = TlFetchString<string>::parse(p); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchLong::parse(p);
  res->winners_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starsGiveawayWinnersOption>, 1411605001>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starsGiveawayOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsGiveawayOption");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (extended_ << 0) | (default_ << 1)));
    if (var0 & 1) { s.store_field("extended", true); }
    if (var0 & 2) { s.store_field("default", true); }
    s.store_field("stars", stars_);
    s.store_field("yearly_boosts", yearly_boosts_);
    if (var0 & 4) { s.store_field("store_product", store_product_); }
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    { s.store_vector_begin("winners", winners_.size()); for (const auto &_value : winners_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starsGiveawayWinnersOption::starsGiveawayWinnersOption()
  : flags_()
  , default_()
  , users_()
  , per_user_stars_()
{}

const std::int32_t starsGiveawayWinnersOption::ID;

object_ptr<starsGiveawayWinnersOption> starsGiveawayWinnersOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsGiveawayWinnersOption> res = make_tl_object<starsGiveawayWinnersOption>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->default_ = (var0 & 1) != 0;
  res->users_ = TlFetchInt::parse(p);
  res->per_user_stars_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starsGiveawayWinnersOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsGiveawayWinnersOption");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (default_ << 0)));
    if (var0 & 1) { s.store_field("default", true); }
    s.store_field("users", users_);
    s.store_field("per_user_stars", per_user_stars_);
    s.store_class_end();
  }
}

object_ptr<StarsTransactionPeer> StarsTransactionPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case starsTransactionPeerUnsupported::ID:
      return starsTransactionPeerUnsupported::fetch(p);
    case starsTransactionPeerAppStore::ID:
      return starsTransactionPeerAppStore::fetch(p);
    case starsTransactionPeerPlayMarket::ID:
      return starsTransactionPeerPlayMarket::fetch(p);
    case starsTransactionPeerPremiumBot::ID:
      return starsTransactionPeerPremiumBot::fetch(p);
    case starsTransactionPeerFragment::ID:
      return starsTransactionPeerFragment::fetch(p);
    case starsTransactionPeer::ID:
      return starsTransactionPeer::fetch(p);
    case starsTransactionPeerAds::ID:
      return starsTransactionPeerAds::fetch(p);
    case starsTransactionPeerAPI::ID:
      return starsTransactionPeerAPI::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t starsTransactionPeerUnsupported::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeerUnsupported::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeerUnsupported>();
}

void starsTransactionPeerUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeerUnsupported");
    s.store_class_end();
  }
}

const std::int32_t starsTransactionPeerAppStore::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeerAppStore::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeerAppStore>();
}

void starsTransactionPeerAppStore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeerAppStore");
    s.store_class_end();
  }
}

const std::int32_t starsTransactionPeerPlayMarket::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeerPlayMarket::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeerPlayMarket>();
}

void starsTransactionPeerPlayMarket::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeerPlayMarket");
    s.store_class_end();
  }
}

const std::int32_t starsTransactionPeerPremiumBot::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeerPremiumBot::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeerPremiumBot>();
}

void starsTransactionPeerPremiumBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeerPremiumBot");
    s.store_class_end();
  }
}

const std::int32_t starsTransactionPeerFragment::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeerFragment::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeerFragment>();
}

void starsTransactionPeerFragment::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeerFragment");
    s.store_class_end();
  }
}

const std::int32_t starsTransactionPeer::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeer::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeer>(p);
}

starsTransactionPeer::starsTransactionPeer(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
{}

void starsTransactionPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

const std::int32_t starsTransactionPeerAds::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeerAds::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeerAds>();
}

void starsTransactionPeerAds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeerAds");
    s.store_class_end();
  }
}

const std::int32_t starsTransactionPeerAPI::ID;

object_ptr<StarsTransactionPeer> starsTransactionPeerAPI::fetch(TlBufferParser &p) {
  return make_tl_object<starsTransactionPeerAPI>();
}

void starsTransactionPeerAPI::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTransactionPeerAPI");
    s.store_class_end();
  }
}

const std::int32_t statsPercentValue::ID;

object_ptr<statsPercentValue> statsPercentValue::fetch(TlBufferParser &p) {
  return make_tl_object<statsPercentValue>(p);
}

statsPercentValue::statsPercentValue(TlBufferParser &p)
  : part_(TlFetchDouble::parse(p))
  , total_(TlFetchDouble::parse(p))
{}

void statsPercentValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsPercentValue");
    s.store_field("part", part_);
    s.store_field("total", total_);
    s.store_class_end();
  }
}

storyFwdHeader::storyFwdHeader()
  : flags_()
  , modified_()
  , from_()
  , from_name_()
  , story_id_()
{}

const std::int32_t storyFwdHeader::ID;

object_ptr<storyFwdHeader> storyFwdHeader::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyFwdHeader> res = make_tl_object<storyFwdHeader>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->modified_ = (var0 & 8) != 0;
  if (var0 & 1) { res->from_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 2) { res->from_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->story_id_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void storyFwdHeader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyFwdHeader");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (modified_ << 3)));
    if (var0 & 8) { s.store_field("modified", true); }
    if (var0 & 1) { s.store_object_field("from", static_cast<const BaseObject *>(from_.get())); }
    if (var0 & 2) { s.store_field("from_name", from_name_); }
    if (var0 & 4) { s.store_field("story_id", story_id_); }
    s.store_class_end();
  }
}

object_ptr<TopPeerCategory> TopPeerCategory::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case topPeerCategoryBotsPM::ID:
      return topPeerCategoryBotsPM::fetch(p);
    case topPeerCategoryBotsInline::ID:
      return topPeerCategoryBotsInline::fetch(p);
    case topPeerCategoryCorrespondents::ID:
      return topPeerCategoryCorrespondents::fetch(p);
    case topPeerCategoryGroups::ID:
      return topPeerCategoryGroups::fetch(p);
    case topPeerCategoryChannels::ID:
      return topPeerCategoryChannels::fetch(p);
    case topPeerCategoryPhoneCalls::ID:
      return topPeerCategoryPhoneCalls::fetch(p);
    case topPeerCategoryForwardUsers::ID:
      return topPeerCategoryForwardUsers::fetch(p);
    case topPeerCategoryForwardChats::ID:
      return topPeerCategoryForwardChats::fetch(p);
    case topPeerCategoryBotsApp::ID:
      return topPeerCategoryBotsApp::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t topPeerCategoryBotsPM::ID;

object_ptr<TopPeerCategory> topPeerCategoryBotsPM::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryBotsPM>();
}

void topPeerCategoryBotsPM::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryBotsPM::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryBotsPM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryBotsPM");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryBotsInline::ID;

object_ptr<TopPeerCategory> topPeerCategoryBotsInline::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryBotsInline>();
}

void topPeerCategoryBotsInline::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryBotsInline::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryBotsInline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryBotsInline");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryCorrespondents::ID;

object_ptr<TopPeerCategory> topPeerCategoryCorrespondents::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryCorrespondents>();
}

void topPeerCategoryCorrespondents::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryCorrespondents::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryCorrespondents::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryCorrespondents");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryGroups::ID;

object_ptr<TopPeerCategory> topPeerCategoryGroups::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryGroups>();
}

void topPeerCategoryGroups::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryGroups::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryGroups::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryGroups");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryChannels::ID;

object_ptr<TopPeerCategory> topPeerCategoryChannels::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryChannels>();
}

void topPeerCategoryChannels::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryChannels::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryChannels");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryPhoneCalls::ID;

object_ptr<TopPeerCategory> topPeerCategoryPhoneCalls::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryPhoneCalls>();
}

void topPeerCategoryPhoneCalls::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryPhoneCalls::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryPhoneCalls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryPhoneCalls");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryForwardUsers::ID;

object_ptr<TopPeerCategory> topPeerCategoryForwardUsers::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryForwardUsers>();
}

void topPeerCategoryForwardUsers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryForwardUsers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryForwardUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryForwardUsers");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryForwardChats::ID;

object_ptr<TopPeerCategory> topPeerCategoryForwardChats::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryForwardChats>();
}

void topPeerCategoryForwardChats::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryForwardChats::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryForwardChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryForwardChats");
    s.store_class_end();
  }
}

const std::int32_t topPeerCategoryBotsApp::ID;

object_ptr<TopPeerCategory> topPeerCategoryBotsApp::fetch(TlBufferParser &p) {
  return make_tl_object<topPeerCategoryBotsApp>();
}

void topPeerCategoryBotsApp::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void topPeerCategoryBotsApp::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void topPeerCategoryBotsApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topPeerCategoryBotsApp");
    s.store_class_end();
  }
}

const std::int32_t account_autoSaveSettings::ID;

object_ptr<account_autoSaveSettings> account_autoSaveSettings::fetch(TlBufferParser &p) {
  return make_tl_object<account_autoSaveSettings>(p);
}

account_autoSaveSettings::account_autoSaveSettings(TlBufferParser &p)
  : users_settings_(TlFetchBoxed<TlFetchObject<autoSaveSettings>, -934791986>::parse(p))
  , chats_settings_(TlFetchBoxed<TlFetchObject<autoSaveSettings>, -934791986>::parse(p))
  , broadcasts_settings_(TlFetchBoxed<TlFetchObject<autoSaveSettings>, -934791986>::parse(p))
  , exceptions_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<autoSaveException>, -2124403385>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void account_autoSaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.autoSaveSettings");
    s.store_object_field("users_settings", static_cast<const BaseObject *>(users_settings_.get()));
    s.store_object_field("chats_settings", static_cast<const BaseObject *>(chats_settings_.get()));
    s.store_object_field("broadcasts_settings", static_cast<const BaseObject *>(broadcasts_settings_.get()));
    { s.store_vector_begin("exceptions", exceptions_.size()); for (const auto &_value : exceptions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_takeout::ID;

object_ptr<account_takeout> account_takeout::fetch(TlBufferParser &p) {
  return make_tl_object<account_takeout>(p);
}

account_takeout::account_takeout(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void account_takeout::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.takeout");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

const std::int32_t auth_exportedAuthorization::ID;

object_ptr<auth_exportedAuthorization> auth_exportedAuthorization::fetch(TlBufferParser &p) {
  return make_tl_object<auth_exportedAuthorization>(p);
}

auth_exportedAuthorization::auth_exportedAuthorization(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
{}

void auth_exportedAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.exportedAuthorization");
    s.store_field("id", id_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

object_ptr<help_PromoData> help_PromoData::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_promoDataEmpty::ID:
      return help_promoDataEmpty::fetch(p);
    case help_promoData::ID:
      return help_promoData::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_promoDataEmpty::ID;

object_ptr<help_PromoData> help_promoDataEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<help_promoDataEmpty>(p);
}

help_promoDataEmpty::help_promoDataEmpty(TlBufferParser &p)
  : expires_(TlFetchInt::parse(p))
{}

void help_promoDataEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.promoDataEmpty");
    s.store_field("expires", expires_);
    s.store_class_end();
  }
}

help_promoData::help_promoData()
  : flags_()
  , proxy_()
  , expires_()
  , peer_()
  , psa_type_()
  , psa_message_()
  , pending_suggestions_()
  , dismissed_suggestions_()
  , custom_pending_suggestion_()
  , chats_()
  , users_()
{}

const std::int32_t help_promoData::ID;

object_ptr<help_PromoData> help_promoData::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<help_promoData> res = make_tl_object<help_promoData>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->proxy_ = (var0 & 1) != 0;
  res->expires_ = TlFetchInt::parse(p);
  if (var0 & 8) { res->peer_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 2) { res->psa_type_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->psa_message_ = TlFetchString<string>::parse(p); }
  res->pending_suggestions_ = TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p);
  res->dismissed_suggestions_ = TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p);
  if (var0 & 16) { res->custom_pending_suggestion_ = TlFetchBoxed<TlFetchObject<pendingSuggestion>, -404214254>::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void help_promoData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.promoData");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (proxy_ << 0)));
    if (var0 & 1) { s.store_field("proxy", true); }
    s.store_field("expires", expires_);
    if (var0 & 8) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    if (var0 & 2) { s.store_field("psa_type", psa_type_); }
    if (var0 & 4) { s.store_field("psa_message", psa_message_); }
    { s.store_vector_begin("pending_suggestions", pending_suggestions_.size()); for (const auto &_value : pending_suggestions_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("dismissed_suggestions", dismissed_suggestions_.size()); for (const auto &_value : dismissed_suggestions_) { s.store_field("", _value); } s.store_class_end(); }
    if (var0 & 16) { s.store_object_field("custom_pending_suggestion", static_cast<const BaseObject *>(custom_pending_suggestion_.get())); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t help_recentMeUrls::ID;

object_ptr<help_recentMeUrls> help_recentMeUrls::fetch(TlBufferParser &p) {
  return make_tl_object<help_recentMeUrls>(p);
}

help_recentMeUrls::help_recentMeUrls(TlBufferParser &p)
  : urls_(TlFetchBoxed<TlFetchVector<TlFetchObject<RecentMeUrl>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void help_recentMeUrls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.recentMeUrls");
    { s.store_vector_begin("urls", urls_.size()); for (const auto &_value : urls_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t help_support::ID;

object_ptr<help_support> help_support::fetch(TlBufferParser &p) {
  return make_tl_object<help_support>(p);
}

help_support::help_support(TlBufferParser &p)
  : phone_number_(TlFetchString<string>::parse(p))
  , user_(TlFetchObject<User>::parse(p))
{}

void help_support::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.support");
    s.store_field("phone_number", phone_number_);
    s.store_object_field("user", static_cast<const BaseObject *>(user_.get()));
    s.store_class_end();
  }
}

const std::int32_t messages_affectedMessages::ID;

object_ptr<messages_affectedMessages> messages_affectedMessages::fetch(TlBufferParser &p) {
  return make_tl_object<messages_affectedMessages>(p);
}

messages_affectedMessages::messages_affectedMessages(TlBufferParser &p)
  : pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
{}

void messages_affectedMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.affectedMessages");
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_class_end();
  }
}

const std::int32_t messages_chatAdminsWithInvites::ID;

object_ptr<messages_chatAdminsWithInvites> messages_chatAdminsWithInvites::fetch(TlBufferParser &p) {
  return make_tl_object<messages_chatAdminsWithInvites>(p);
}

messages_chatAdminsWithInvites::messages_chatAdminsWithInvites(TlBufferParser &p)
  : admins_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<chatAdminWithInvites>, -219353309>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_chatAdminsWithInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.chatAdminsWithInvites");
    { s.store_vector_begin("admins", admins_.size()); for (const auto &_value : admins_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_savedInfo::payments_savedInfo()
  : flags_()
  , has_saved_credentials_()
  , saved_info_()
{}

const std::int32_t payments_savedInfo::ID;

object_ptr<payments_savedInfo> payments_savedInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_savedInfo> res = make_tl_object<payments_savedInfo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_saved_credentials_ = (var0 & 2) != 0;
  if (var0 & 1) { res->saved_info_ = TlFetchBoxed<TlFetchObject<paymentRequestedInfo>, -1868808300>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void payments_savedInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.savedInfo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_saved_credentials_ << 1)));
    if (var0 & 2) { s.store_field("has_saved_credentials", true); }
    if (var0 & 1) { s.store_object_field("saved_info", static_cast<const BaseObject *>(saved_info_.get())); }
    s.store_class_end();
  }
}

const std::int32_t payments_starsRevenueAdsAccountUrl::ID;

object_ptr<payments_starsRevenueAdsAccountUrl> payments_starsRevenueAdsAccountUrl::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starsRevenueAdsAccountUrl>(p);
}

payments_starsRevenueAdsAccountUrl::payments_starsRevenueAdsAccountUrl(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void payments_starsRevenueAdsAccountUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starsRevenueAdsAccountUrl");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

const std::int32_t payments_starsRevenueWithdrawalUrl::ID;

object_ptr<payments_starsRevenueWithdrawalUrl> payments_starsRevenueWithdrawalUrl::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starsRevenueWithdrawalUrl>(p);
}

payments_starsRevenueWithdrawalUrl::payments_starsRevenueWithdrawalUrl(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void payments_starsRevenueWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starsRevenueWithdrawalUrl");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

object_ptr<photos_Photos> photos_Photos::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case photos_photos::ID:
      return photos_photos::fetch(p);
    case photos_photosSlice::ID:
      return photos_photosSlice::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t photos_photos::ID;

object_ptr<photos_Photos> photos_photos::fetch(TlBufferParser &p) {
  return make_tl_object<photos_photos>(p);
}

photos_photos::photos_photos(TlBufferParser &p)
  : photos_(TlFetchBoxed<TlFetchVector<TlFetchObject<Photo>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void photos_photos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.photos");
    { s.store_vector_begin("photos", photos_.size()); for (const auto &_value : photos_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t photos_photosSlice::ID;

object_ptr<photos_Photos> photos_photosSlice::fetch(TlBufferParser &p) {
  return make_tl_object<photos_photosSlice>(p);
}

photos_photosSlice::photos_photosSlice(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , photos_(TlFetchBoxed<TlFetchVector<TlFetchObject<Photo>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void photos_photosSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.photosSlice");
    s.store_field("count", count_);
    { s.store_vector_begin("photos", photos_.size()); for (const auto &_value : photos_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t stickers_suggestedShortName::ID;

object_ptr<stickers_suggestedShortName> stickers_suggestedShortName::fetch(TlBufferParser &p) {
  return make_tl_object<stickers_suggestedShortName>(p);
}

stickers_suggestedShortName::stickers_suggestedShortName(TlBufferParser &p)
  : short_name_(TlFetchString<string>::parse(p))
{}

void stickers_suggestedShortName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.suggestedShortName");
    s.store_field("short_name", short_name_);
    s.store_class_end();
  }
}

stories_stories::stories_stories()
  : flags_()
  , count_()
  , stories_()
  , pinned_to_top_()
  , chats_()
  , users_()
{}

const std::int32_t stories_stories::ID;

object_ptr<stories_stories> stories_stories::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stories_stories> res = make_tl_object<stories_stories>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->stories_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StoryItem>>, 481674261>::parse(p);
  if (var0 & 1) { res->pinned_to_top_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void stories_stories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.stories");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("stories", stories_.size()); for (const auto &_value : stories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { { s.store_vector_begin("pinned_to_top", pinned_to_top_.size()); for (const auto &_value : pinned_to_top_) { s.store_field("", _value); } s.store_class_end(); } }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<upload_CdnFile> upload_CdnFile::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case upload_cdnFileReuploadNeeded::ID:
      return upload_cdnFileReuploadNeeded::fetch(p);
    case upload_cdnFile::ID:
      return upload_cdnFile::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t upload_cdnFileReuploadNeeded::ID;

object_ptr<upload_CdnFile> upload_cdnFileReuploadNeeded::fetch(TlBufferParser &p) {
  return make_tl_object<upload_cdnFileReuploadNeeded>(p);
}

upload_cdnFileReuploadNeeded::upload_cdnFileReuploadNeeded(TlBufferParser &p)
  : request_token_(TlFetchBytes<bytes>::parse(p))
{}

void upload_cdnFileReuploadNeeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.cdnFileReuploadNeeded");
    s.store_bytes_field("request_token", request_token_);
    s.store_class_end();
  }
}

const std::int32_t upload_cdnFile::ID;

object_ptr<upload_CdnFile> upload_cdnFile::fetch(TlBufferParser &p) {
  return make_tl_object<upload_cdnFile>(p);
}

upload_cdnFile::upload_cdnFile(TlBufferParser &p)
  : bytes_(TlFetchBytes<bytes>::parse(p))
{}

void upload_cdnFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.cdnFile");
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

object_ptr<users_Users> users_Users::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case users_users::ID:
      return users_users::fetch(p);
    case users_usersSlice::ID:
      return users_usersSlice::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t users_users::ID;

object_ptr<users_Users> users_users::fetch(TlBufferParser &p) {
  return make_tl_object<users_users>(p);
}

users_users::users_users(TlBufferParser &p)
  : users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void users_users::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.users");
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t users_usersSlice::ID;

object_ptr<users_Users> users_usersSlice::fetch(TlBufferParser &p) {
  return make_tl_object<users_usersSlice>(p);
}

users_usersSlice::users_usersSlice(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void users_usersSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.usersSlice");
    s.store_field("count", count_);
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_getAccountTTL::ID;

void account_getAccountTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(150761757);
}

void account_getAccountTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(150761757);
}

void account_getAccountTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getAccountTTL");
    s.store_class_end();
  }
}

account_getAccountTTL::ReturnType account_getAccountTTL::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<accountDaysTTL>, -1194283041>::parse(p);
#undef FAIL
}

account_getAuthorizationForm::account_getAuthorizationForm(int64 bot_id_, string const &scope_, string const &public_key_)
  : bot_id_(bot_id_)
  , scope_(scope_)
  , public_key_(public_key_)
{}

const std::int32_t account_getAuthorizationForm::ID;

void account_getAuthorizationForm::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1456907910);
  TlStoreBinary::store(bot_id_, s);
  TlStoreString::store(scope_, s);
  TlStoreString::store(public_key_, s);
}

void account_getAuthorizationForm::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1456907910);
  TlStoreBinary::store(bot_id_, s);
  TlStoreString::store(scope_, s);
  TlStoreString::store(public_key_, s);
}

void account_getAuthorizationForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getAuthorizationForm");
    s.store_field("bot_id", bot_id_);
    s.store_field("scope", scope_);
    s.store_field("public_key", public_key_);
    s.store_class_end();
  }
}

account_getAuthorizationForm::ReturnType account_getAuthorizationForm::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_authorizationForm>, -1389486888>::parse(p);
#undef FAIL
}

account_updateBusinessIntro::account_updateBusinessIntro(int32 flags_, object_ptr<inputBusinessIntro> &&intro_)
  : flags_(flags_)
  , intro_(std::move(intro_))
{}

const std::int32_t account_updateBusinessIntro::ID;

void account_updateBusinessIntro::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1508585420);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 163867085>::store(intro_, s); }
}

void account_updateBusinessIntro::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1508585420);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 163867085>::store(intro_, s); }
}

void account_updateBusinessIntro::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateBusinessIntro");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("intro", static_cast<const BaseObject *>(intro_.get())); }
    s.store_class_end();
  }
}

account_updateBusinessIntro::ReturnType account_updateBusinessIntro::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateNotifySettings::account_updateNotifySettings(object_ptr<InputNotifyPeer> &&peer_, object_ptr<inputPeerNotifySettings> &&settings_)
  : peer_(std::move(peer_))
  , settings_(std::move(settings_))
{}

const std::int32_t account_updateNotifySettings::ID;

void account_updateNotifySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2067899501);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreObject, -892638494>::store(settings_, s);
}

void account_updateNotifySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2067899501);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreObject, -892638494>::store(settings_, s);
}

void account_updateNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateNotifySettings");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_updateNotifySettings::ReturnType account_updateNotifySettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateStatus::account_updateStatus(bool offline_)
  : offline_(offline_)
{}

const std::int32_t account_updateStatus::ID;

void account_updateStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1713919532);
  TlStoreBool::store(offline_, s);
}

void account_updateStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1713919532);
  TlStoreBool::store(offline_, s);
}

void account_updateStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateStatus");
    s.store_field("offline", offline_);
    s.store_class_end();
  }
}

account_updateStatus::ReturnType account_updateStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_checkRecoveryPassword::auth_checkRecoveryPassword(string const &code_)
  : code_(code_)
{}

const std::int32_t auth_checkRecoveryPassword::ID;

void auth_checkRecoveryPassword::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(221691769);
  TlStoreString::store(code_, s);
}

void auth_checkRecoveryPassword::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(221691769);
  TlStoreString::store(code_, s);
}

void auth_checkRecoveryPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.checkRecoveryPassword");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

auth_checkRecoveryPassword::ReturnType auth_checkRecoveryPassword::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_reorderPreviewMedias::bots_reorderPreviewMedias(object_ptr<InputUser> &&bot_, string const &lang_code_, array<object_ptr<InputMedia>> &&order_)
  : bot_(std::move(bot_))
  , lang_code_(lang_code_)
  , order_(std::move(order_))
{}

const std::int32_t bots_reorderPreviewMedias::ID;

void bots_reorderPreviewMedias::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1238895702);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s);
}

void bots_reorderPreviewMedias::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1238895702);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s);
}

void bots_reorderPreviewMedias::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.reorderPreviewMedias");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("lang_code", lang_code_);
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

bots_reorderPreviewMedias::ReturnType bots_reorderPreviewMedias::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_toggleUsername::bots_toggleUsername(object_ptr<InputUser> &&bot_, string const &username_, bool active_)
  : bot_(std::move(bot_))
  , username_(username_)
  , active_(active_)
{}

const std::int32_t bots_toggleUsername::ID;

void bots_toggleUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(87861619);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(username_, s);
  TlStoreBool::store(active_, s);
}

void bots_toggleUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(87861619);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(username_, s);
  TlStoreBool::store(active_, s);
}

void bots_toggleUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.toggleUsername");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("username", username_);
    s.store_field("active", active_);
    s.store_class_end();
  }
}

bots_toggleUsername::ReturnType bots_toggleUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_updateStarRefProgram::bots_updateStarRefProgram(int32 flags_, object_ptr<InputUser> &&bot_, int32 commission_permille_, int32 duration_months_)
  : flags_(flags_)
  , bot_(std::move(bot_))
  , commission_permille_(commission_permille_)
  , duration_months_(duration_months_)
{}

const std::int32_t bots_updateStarRefProgram::ID;

void bots_updateStarRefProgram::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2005621427);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(commission_permille_, s);
  if (var0 & 1) { TlStoreBinary::store(duration_months_, s); }
}

void bots_updateStarRefProgram::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2005621427);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(commission_permille_, s);
  if (var0 & 1) { TlStoreBinary::store(duration_months_, s); }
}

void bots_updateStarRefProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.updateStarRefProgram");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("commission_permille", commission_permille_);
    if (var0 & 1) { s.store_field("duration_months", duration_months_); }
    s.store_class_end();
  }
}

bots_updateStarRefProgram::ReturnType bots_updateStarRefProgram::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<starRefProgram>, -586389774>::parse(p);
#undef FAIL
}

channels_getMessages::channels_getMessages(object_ptr<InputChannel> &&channel_, array<object_ptr<InputMessage>> &&id_)
  : channel_(std::move(channel_))
  , id_(std::move(id_))
{}

const std::int32_t channels_getMessages::ID;

void channels_getMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1383294429);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void channels_getMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1383294429);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void channels_getMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getMessages");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

channels_getMessages::ReturnType channels_getMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

channels_setEmojiStickers::channels_setEmojiStickers(object_ptr<InputChannel> &&channel_, object_ptr<InputStickerSet> &&stickerset_)
  : channel_(std::move(channel_))
  , stickerset_(std::move(stickerset_))
{}

const std::int32_t channels_setEmojiStickers::ID;

void channels_setEmojiStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1020866743);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void channels_setEmojiStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1020866743);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void channels_setEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.setEmojiStickers");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_class_end();
  }
}

channels_setEmojiStickers::ReturnType channels_setEmojiStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_setStickers::channels_setStickers(object_ptr<InputChannel> &&channel_, object_ptr<InputStickerSet> &&stickerset_)
  : channel_(std::move(channel_))
  , stickerset_(std::move(stickerset_))
{}

const std::int32_t channels_setStickers::ID;

void channels_setStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-359881479);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void channels_setStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-359881479);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void channels_setStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.setStickers");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_class_end();
  }
}

channels_setStickers::ReturnType channels_setStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_toggleAutotranslation::channels_toggleAutotranslation(object_ptr<InputChannel> &&channel_, bool enabled_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
{}

const std::int32_t channels_toggleAutotranslation::ID;

void channels_toggleAutotranslation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(377471137);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleAutotranslation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(377471137);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleAutotranslation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleAutotranslation");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

channels_toggleAutotranslation::ReturnType channels_toggleAutotranslation::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_updateColor::channels_updateColor(int32 flags_, bool for_profile_, object_ptr<InputChannel> &&channel_, int32 color_, int64 background_emoji_id_)
  : flags_(flags_)
  , for_profile_(for_profile_)
  , channel_(std::move(channel_))
  , color_(color_)
  , background_emoji_id_(background_emoji_id_)
{}

const std::int32_t channels_updateColor::ID;

void channels_updateColor::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-659933583);
  TlStoreBinary::store((var0 = flags_ | (for_profile_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  if (var0 & 4) { TlStoreBinary::store(color_, s); }
  if (var0 & 1) { TlStoreBinary::store(background_emoji_id_, s); }
}

void channels_updateColor::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-659933583);
  TlStoreBinary::store((var0 = flags_ | (for_profile_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  if (var0 & 4) { TlStoreBinary::store(color_, s); }
  if (var0 & 1) { TlStoreBinary::store(background_emoji_id_, s); }
}

void channels_updateColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.updateColor");
    s.store_field("flags", (var0 = flags_ | (for_profile_ << 1)));
    if (var0 & 2) { s.store_field("for_profile", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    if (var0 & 4) { s.store_field("color", color_); }
    if (var0 & 1) { s.store_field("background_emoji_id", background_emoji_id_); }
    s.store_class_end();
  }
}

channels_updateColor::ReturnType channels_updateColor::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

chatlists_joinChatlistInvite::chatlists_joinChatlistInvite(string const &slug_, array<object_ptr<InputPeer>> &&peers_)
  : slug_(slug_)
  , peers_(std::move(peers_))
{}

const std::int32_t chatlists_joinChatlistInvite::ID;

void chatlists_joinChatlistInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1498291302);
  TlStoreString::store(slug_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_joinChatlistInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1498291302);
  TlStoreString::store(slug_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_joinChatlistInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.joinChatlistInvite");
    s.store_field("slug", slug_);
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatlists_joinChatlistInvite::ReturnType chatlists_joinChatlistInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

help_setBotUpdatesStatus::help_setBotUpdatesStatus(int32 pending_updates_count_, string const &message_)
  : pending_updates_count_(pending_updates_count_)
  , message_(message_)
{}

const std::int32_t help_setBotUpdatesStatus::ID;

void help_setBotUpdatesStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-333262899);
  TlStoreBinary::store(pending_updates_count_, s);
  TlStoreString::store(message_, s);
}

void help_setBotUpdatesStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-333262899);
  TlStoreBinary::store(pending_updates_count_, s);
  TlStoreString::store(message_, s);
}

void help_setBotUpdatesStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.setBotUpdatesStatus");
    s.store_field("pending_updates_count", pending_updates_count_);
    s.store_field("message", message_);
    s.store_class_end();
  }
}

help_setBotUpdatesStatus::ReturnType help_setBotUpdatesStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

invokeWithReCaptchaPrefix::invokeWithReCaptchaPrefix(string const &token_)
  : token_(token_)
{}

const std::int32_t invokeWithReCaptchaPrefix::ID;

void invokeWithReCaptchaPrefix::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1380249708);
  TlStoreString::store(token_, s);
}

void invokeWithReCaptchaPrefix::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1380249708);
  TlStoreString::store(token_, s);
}

void invokeWithReCaptchaPrefix::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "invokeWithReCaptchaPrefix");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

invokeWithReCaptchaPrefix::ReturnType invokeWithReCaptchaPrefix::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<error>, -994444869>::parse(p);
#undef FAIL
}

const std::int32_t messages_clearRecentReactions::ID;

void messages_clearRecentReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1644236876);
}

void messages_clearRecentReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1644236876);
}

void messages_clearRecentReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.clearRecentReactions");
    s.store_class_end();
  }
}

messages_clearRecentReactions::ReturnType messages_clearRecentReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_createChat::messages_createChat(int32 flags_, array<object_ptr<InputUser>> &&users_, string const &title_, int32 ttl_period_)
  : flags_(flags_)
  , users_(std::move(users_))
  , title_(title_)
  , ttl_period_(ttl_period_)
{}

const std::int32_t messages_createChat::ID;

void messages_createChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1831936556);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
  TlStoreString::store(title_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_period_, s); }
}

void messages_createChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1831936556);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
  TlStoreString::store(title_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_period_, s); }
}

void messages_createChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.createChat");
    s.store_field("flags", (var0 = flags_));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("title", title_);
    if (var0 & 1) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

messages_createChat::ReturnType messages_createChat::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_invitedUsers>, 2136862630>::parse(p);
#undef FAIL
}

messages_editChatDefaultBannedRights::messages_editChatDefaultBannedRights(object_ptr<InputPeer> &&peer_, object_ptr<chatBannedRights> &&banned_rights_)
  : peer_(std::move(peer_))
  , banned_rights_(std::move(banned_rights_))
{}

const std::int32_t messages_editChatDefaultBannedRights::ID;

void messages_editChatDefaultBannedRights::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1517917375);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreObject, -1626209256>::store(banned_rights_, s);
}

void messages_editChatDefaultBannedRights::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1517917375);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreObject, -1626209256>::store(banned_rights_, s);
}

void messages_editChatDefaultBannedRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editChatDefaultBannedRights");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("banned_rights", static_cast<const BaseObject *>(banned_rights_.get()));
    s.store_class_end();
  }
}

messages_editChatDefaultBannedRights::ReturnType messages_editChatDefaultBannedRights::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_editForumTopic::messages_editForumTopic(int32 flags_, object_ptr<InputPeer> &&peer_, int32 topic_id_, string const &title_, int64 icon_emoji_id_, bool closed_, bool hidden_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , topic_id_(topic_id_)
  , title_(title_)
  , icon_emoji_id_(icon_emoji_id_)
  , closed_(closed_)
  , hidden_(hidden_)
{}

const std::int32_t messages_editForumTopic::ID;

void messages_editForumTopic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-825487052);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(topic_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBinary::store(icon_emoji_id_, s); }
  if (var0 & 4) { TlStoreBool::store(closed_, s); }
  if (var0 & 8) { TlStoreBool::store(hidden_, s); }
}

void messages_editForumTopic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-825487052);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(topic_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBinary::store(icon_emoji_id_, s); }
  if (var0 & 4) { TlStoreBool::store(closed_, s); }
  if (var0 & 8) { TlStoreBool::store(hidden_, s); }
}

void messages_editForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editForumTopic");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("topic_id", topic_id_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("icon_emoji_id", icon_emoji_id_); }
    if (var0 & 4) { s.store_field("closed", closed_); }
    if (var0 & 8) { s.store_field("hidden", hidden_); }
    s.store_class_end();
  }
}

messages_editForumTopic::ReturnType messages_editForumTopic::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getAttachMenuBots::messages_getAttachMenuBots(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getAttachMenuBots::ID;

void messages_getAttachMenuBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(385663691);
  TlStoreBinary::store(hash_, s);
}

void messages_getAttachMenuBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(385663691);
  TlStoreBinary::store(hash_, s);
}

void messages_getAttachMenuBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAttachMenuBots");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getAttachMenuBots::ReturnType messages_getAttachMenuBots::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<AttachMenuBots>::parse(p);
#undef FAIL
}

messages_getAttachedStickers::messages_getAttachedStickers(object_ptr<InputStickeredMedia> &&media_)
  : media_(std::move(media_))
{}

const std::int32_t messages_getAttachedStickers::ID;

void messages_getAttachedStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-866424884);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void messages_getAttachedStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-866424884);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void messages_getAttachedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAttachedStickers");
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

messages_getAttachedStickers::ReturnType messages_getAttachedStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<StickerSetCovered>>, 481674261>::parse(p);
#undef FAIL
}

messages_getEmojiGroups::messages_getEmojiGroups(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getEmojiGroups::ID;

void messages_getEmojiGroups::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1955122779);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiGroups::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1955122779);
  TlStoreBinary::store(hash_, s);
}

void messages_getEmojiGroups::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiGroups");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getEmojiGroups::ReturnType messages_getEmojiGroups::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_EmojiGroups>::parse(p);
#undef FAIL
}

messages_getEmojiKeywords::messages_getEmojiKeywords(string const &lang_code_)
  : lang_code_(lang_code_)
{}

const std::int32_t messages_getEmojiKeywords::ID;

void messages_getEmojiKeywords::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(899735650);
  TlStoreString::store(lang_code_, s);
}

void messages_getEmojiKeywords::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(899735650);
  TlStoreString::store(lang_code_, s);
}

void messages_getEmojiKeywords::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiKeywords");
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

messages_getEmojiKeywords::ReturnType messages_getEmojiKeywords::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<emojiKeywordsDifference>, 1556570557>::parse(p);
#undef FAIL
}

messages_getInlineBotResults::messages_getInlineBotResults(int32 flags_, object_ptr<InputUser> &&bot_, object_ptr<InputPeer> &&peer_, object_ptr<InputGeoPoint> &&geo_point_, string const &query_, string const &offset_)
  : flags_(flags_)
  , bot_(std::move(bot_))
  , peer_(std::move(peer_))
  , geo_point_(std::move(geo_point_))
  , query_(query_)
  , offset_(offset_)
{}

const std::int32_t messages_getInlineBotResults::ID;

void messages_getInlineBotResults::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1364105629);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s); }
  TlStoreString::store(query_, s);
  TlStoreString::store(offset_, s);
}

void messages_getInlineBotResults::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1364105629);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s); }
  TlStoreString::store(query_, s);
  TlStoreString::store(offset_, s);
}

void messages_getInlineBotResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getInlineBotResults");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get())); }
    s.store_field("query", query_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

messages_getInlineBotResults::ReturnType messages_getInlineBotResults::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_botResults>, -534646026>::parse(p);
#undef FAIL
}

messages_getSavedDialogs::messages_getSavedDialogs(int32 flags_, bool exclude_pinned_, object_ptr<InputPeer> &&parent_peer_, int32 offset_date_, int32 offset_id_, object_ptr<InputPeer> &&offset_peer_, int32 limit_, int64 hash_)
  : flags_(flags_)
  , exclude_pinned_(exclude_pinned_)
  , parent_peer_(std::move(parent_peer_))
  , offset_date_(offset_date_)
  , offset_id_(offset_id_)
  , offset_peer_(std::move(offset_peer_))
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t messages_getSavedDialogs::ID;

void messages_getSavedDialogs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(512883865);
  TlStoreBinary::store((var0 = flags_ | (exclude_pinned_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedDialogs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(512883865);
  TlStoreBinary::store((var0 = flags_ | (exclude_pinned_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSavedDialogs");
    s.store_field("flags", (var0 = flags_ | (exclude_pinned_ << 0)));
    if (var0 & 1) { s.store_field("exclude_pinned", true); }
    if (var0 & 2) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
    s.store_field("offset_date", offset_date_);
    s.store_field("offset_id", offset_id_);
    s.store_object_field("offset_peer", static_cast<const BaseObject *>(offset_peer_.get()));
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getSavedDialogs::ReturnType messages_getSavedDialogs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SavedDialogs>::parse(p);
#undef FAIL
}

messages_prolongWebView::messages_prolongWebView(int32 flags_, bool silent_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&bot_, int64 query_id_, object_ptr<InputReplyTo> &&reply_to_, object_ptr<InputPeer> &&send_as_)
  : flags_(flags_)
  , silent_(silent_)
  , peer_(std::move(peer_))
  , bot_(std::move(bot_))
  , query_id_(query_id_)
  , reply_to_(std::move(reply_to_))
  , send_as_(std::move(send_as_))
{}

const std::int32_t messages_prolongWebView::ID;

void messages_prolongWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1328014717);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
}

void messages_prolongWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1328014717);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
}

void messages_prolongWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.prolongWebView");
    s.store_field("flags", (var0 = flags_ | (silent_ << 5)));
    if (var0 & 32) { s.store_field("silent", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("query_id", query_id_);
    if (var0 & 1) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    if (var0 & 8192) { s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get())); }
    s.store_class_end();
  }
}

messages_prolongWebView::ReturnType messages_prolongWebView::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_readHistory::messages_readHistory(object_ptr<InputPeer> &&peer_, int32 max_id_)
  : peer_(std::move(peer_))
  , max_id_(max_id_)
{}

const std::int32_t messages_readHistory::ID;

void messages_readHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(238054714);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
}

void messages_readHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(238054714);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
}

void messages_readHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readHistory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

messages_readHistory::ReturnType messages_readHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedMessages>, -2066640507>::parse(p);
#undef FAIL
}

messages_reorderPinnedForumTopics::messages_reorderPinnedForumTopics(int32 flags_, bool force_, object_ptr<InputPeer> &&peer_, array<int32> &&order_)
  : flags_(flags_)
  , force_(force_)
  , peer_(std::move(peer_))
  , order_(std::move(order_))
{}

const std::int32_t messages_reorderPinnedForumTopics::ID;

void messages_reorderPinnedForumTopics::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(242762224);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_reorderPinnedForumTopics::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(242762224);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_reorderPinnedForumTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reorderPinnedForumTopics");
    s.store_field("flags", (var0 = flags_ | (force_ << 0)));
    if (var0 & 1) { s.store_field("force", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_reorderPinnedForumTopics::ReturnType messages_reorderPinnedForumTopics::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_savePreparedInlineMessage::messages_savePreparedInlineMessage(int32 flags_, object_ptr<InputBotInlineResult> &&result_, object_ptr<InputUser> &&user_id_, array<object_ptr<InlineQueryPeerType>> &&peer_types_)
  : flags_(flags_)
  , result_(std::move(result_))
  , user_id_(std::move(user_id_))
  , peer_types_(std::move(peer_types_))
{}

const std::int32_t messages_savePreparedInlineMessage::ID;

void messages_savePreparedInlineMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-232816849);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(result_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peer_types_, s); }
}

void messages_savePreparedInlineMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-232816849);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(result_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peer_types_, s); }
}

void messages_savePreparedInlineMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.savePreparedInlineMessage");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("result", static_cast<const BaseObject *>(result_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    if (var0 & 1) { { s.store_vector_begin("peer_types", peer_types_.size()); for (const auto &_value : peer_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

messages_savePreparedInlineMessage::ReturnType messages_savePreparedInlineMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_botPreparedInlineMessage>, -1899035375>::parse(p);
#undef FAIL
}

messages_searchEmojiStickerSets::messages_searchEmojiStickerSets(int32 flags_, bool exclude_featured_, string const &q_, int64 hash_)
  : flags_(flags_)
  , exclude_featured_(exclude_featured_)
  , q_(q_)
  , hash_(hash_)
{}

const std::int32_t messages_searchEmojiStickerSets::ID;

void messages_searchEmojiStickerSets::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1833678516);
  TlStoreBinary::store((var0 = flags_ | (exclude_featured_ << 0)), s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchEmojiStickerSets::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1833678516);
  TlStoreBinary::store((var0 = flags_ | (exclude_featured_ << 0)), s);
  TlStoreString::store(q_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchEmojiStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchEmojiStickerSets");
    s.store_field("flags", (var0 = flags_ | (exclude_featured_ << 0)));
    if (var0 & 1) { s.store_field("exclude_featured", true); }
    s.store_field("q", q_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_searchEmojiStickerSets::ReturnType messages_searchEmojiStickerSets::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_FoundStickerSets>::parse(p);
#undef FAIL
}

messages_searchSentMedia::messages_searchSentMedia(string const &q_, object_ptr<MessagesFilter> &&filter_, int32 limit_)
  : q_(q_)
  , filter_(std::move(filter_))
  , limit_(limit_)
{}

const std::int32_t messages_searchSentMedia::ID;

void messages_searchSentMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(276705696);
  TlStoreString::store(q_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_searchSentMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(276705696);
  TlStoreString::store(q_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_searchSentMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchSentMedia");
    s.store_field("q", q_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_searchSentMedia::ReturnType messages_searchSentMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_sendEncrypted::messages_sendEncrypted(int32 flags_, bool silent_, object_ptr<inputEncryptedChat> &&peer_, int64 random_id_, bytes &&data_)
  : flags_(flags_)
  , silent_(silent_)
  , peer_(std::move(peer_))
  , random_id_(random_id_)
  , data_(std::move(data_))
{}

const std::int32_t messages_sendEncrypted::ID;

void messages_sendEncrypted::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1157265941);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 0)), s);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(data_, s);
}

void messages_sendEncrypted::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1157265941);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 0)), s);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(data_, s);
}

void messages_sendEncrypted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendEncrypted");
    s.store_field("flags", (var0 = flags_ | (silent_ << 0)));
    if (var0 & 1) { s.store_field("silent", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("random_id", random_id_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

messages_sendEncrypted::ReturnType messages_sendEncrypted::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SentEncryptedMessage>::parse(p);
#undef FAIL
}

messages_sendVote::messages_sendVote(object_ptr<InputPeer> &&peer_, int32 msg_id_, array<bytes> &&options_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , options_(std::move(options_))
{}

const std::int32_t messages_sendVote::ID;

void messages_sendVote::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(283795844);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(options_, s);
}

void messages_sendVote::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(283795844);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(options_, s);
}

void messages_sendVote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendVote");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_sendVote::ReturnType messages_sendVote::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_setBotCallbackAnswer::messages_setBotCallbackAnswer(int32 flags_, bool alert_, int64 query_id_, string const &message_, string const &url_, int32 cache_time_)
  : flags_(flags_)
  , alert_(alert_)
  , query_id_(query_id_)
  , message_(message_)
  , url_(url_)
  , cache_time_(cache_time_)
{}

const std::int32_t messages_setBotCallbackAnswer::ID;

void messages_setBotCallbackAnswer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-712043766);
  TlStoreBinary::store((var0 = flags_ | (alert_ << 1)), s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreString::store(message_, s); }
  if (var0 & 4) { TlStoreString::store(url_, s); }
  TlStoreBinary::store(cache_time_, s);
}

void messages_setBotCallbackAnswer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-712043766);
  TlStoreBinary::store((var0 = flags_ | (alert_ << 1)), s);
  TlStoreBinary::store(query_id_, s);
  if (var0 & 1) { TlStoreString::store(message_, s); }
  if (var0 & 4) { TlStoreString::store(url_, s); }
  TlStoreBinary::store(cache_time_, s);
}

void messages_setBotCallbackAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setBotCallbackAnswer");
    s.store_field("flags", (var0 = flags_ | (alert_ << 1)));
    if (var0 & 2) { s.store_field("alert", true); }
    s.store_field("query_id", query_id_);
    if (var0 & 1) { s.store_field("message", message_); }
    if (var0 & 4) { s.store_field("url", url_); }
    s.store_field("cache_time", cache_time_);
    s.store_class_end();
  }
}

messages_setBotCallbackAnswer::ReturnType messages_setBotCallbackAnswer::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_setChatWallPaper::messages_setChatWallPaper(int32 flags_, bool for_both_, bool revert_, object_ptr<InputPeer> &&peer_, object_ptr<InputWallPaper> &&wallpaper_, object_ptr<wallPaperSettings> &&settings_, int32 id_)
  : flags_(flags_)
  , for_both_(for_both_)
  , revert_(revert_)
  , peer_(std::move(peer_))
  , wallpaper_(std::move(wallpaper_))
  , settings_(std::move(settings_))
  , id_(id_)
{}

const std::int32_t messages_setChatWallPaper::ID;

void messages_setChatWallPaper::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1879389471);
  TlStoreBinary::store((var0 = flags_ | (for_both_ << 3) | (revert_ << 4)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s); }
  if (var0 & 2) { TlStoreBinary::store(id_, s); }
}

void messages_setChatWallPaper::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1879389471);
  TlStoreBinary::store((var0 = flags_ | (for_both_ << 3) | (revert_ << 4)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s); }
  if (var0 & 2) { TlStoreBinary::store(id_, s); }
}

void messages_setChatWallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setChatWallPaper");
    s.store_field("flags", (var0 = flags_ | (for_both_ << 3) | (revert_ << 4)));
    if (var0 & 8) { s.store_field("for_both", true); }
    if (var0 & 16) { s.store_field("revert", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get())); }
    if (var0 & 4) { s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get())); }
    if (var0 & 2) { s.store_field("id", id_); }
    s.store_class_end();
  }
}

messages_setChatWallPaper::ReturnType messages_setChatWallPaper::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_assignAppStoreTransaction::payments_assignAppStoreTransaction(bytes &&receipt_, object_ptr<InputStorePaymentPurpose> &&purpose_)
  : receipt_(std::move(receipt_))
  , purpose_(std::move(purpose_))
{}

const std::int32_t payments_assignAppStoreTransaction::ID;

void payments_assignAppStoreTransaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2131921795);
  TlStoreString::store(receipt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_assignAppStoreTransaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2131921795);
  TlStoreString::store(receipt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_assignAppStoreTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.assignAppStoreTransaction");
    s.store_bytes_field("receipt", receipt_);
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

payments_assignAppStoreTransaction::ReturnType payments_assignAppStoreTransaction::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_assignPlayMarketTransaction::payments_assignPlayMarketTransaction(object_ptr<dataJSON> &&receipt_, object_ptr<InputStorePaymentPurpose> &&purpose_)
  : receipt_(std::move(receipt_))
  , purpose_(std::move(purpose_))
{}

const std::int32_t payments_assignPlayMarketTransaction::ID;

void payments_assignPlayMarketTransaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-537046829);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(receipt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_assignPlayMarketTransaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-537046829);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(receipt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_assignPlayMarketTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.assignPlayMarketTransaction");
    s.store_object_field("receipt", static_cast<const BaseObject *>(receipt_.get()));
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

payments_assignPlayMarketTransaction::ReturnType payments_assignPlayMarketTransaction::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_deleteStarGiftCollection::payments_deleteStarGiftCollection(object_ptr<InputPeer> &&peer_, int32 collection_id_)
  : peer_(std::move(peer_))
  , collection_id_(collection_id_)
{}

const std::int32_t payments_deleteStarGiftCollection::ID;

void payments_deleteStarGiftCollection::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1386854168);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(collection_id_, s);
}

void payments_deleteStarGiftCollection::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1386854168);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(collection_id_, s);
}

void payments_deleteStarGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.deleteStarGiftCollection");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("collection_id", collection_id_);
    s.store_class_end();
  }
}

payments_deleteStarGiftCollection::ReturnType payments_deleteStarGiftCollection::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t payments_getSavedInfo::ID;

void payments_getSavedInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(578650699);
}

void payments_getSavedInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(578650699);
}

void payments_getSavedInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getSavedInfo");
    s.store_class_end();
  }
}

payments_getSavedInfo::ReturnType payments_getSavedInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_savedInfo>, -74456004>::parse(p);
#undef FAIL
}

payments_getSavedStarGift::payments_getSavedStarGift(array<object_ptr<InputSavedStarGift>> &&stargift_)
  : stargift_(std::move(stargift_))
{}

const std::int32_t payments_getSavedStarGift::ID;

void payments_getSavedStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1269456634);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stargift_, s);
}

void payments_getSavedStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1269456634);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stargift_, s);
}

void payments_getSavedStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getSavedStarGift");
    { s.store_vector_begin("stargift", stargift_.size()); for (const auto &_value : stargift_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_getSavedStarGift::ReturnType payments_getSavedStarGift::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_savedStarGifts>, -1779201615>::parse(p);
#undef FAIL
}

const std::int32_t payments_getStarsTopupOptions::ID;

void payments_getStarsTopupOptions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1072773165);
}

void payments_getStarsTopupOptions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1072773165);
}

void payments_getStarsTopupOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsTopupOptions");
    s.store_class_end();
  }
}

payments_getStarsTopupOptions::ReturnType payments_getStarsTopupOptions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starsTopupOption>, 198776256>>, 481674261>::parse(p);
#undef FAIL
}

payments_launchPrepaidGiveaway::payments_launchPrepaidGiveaway(object_ptr<InputPeer> &&peer_, int64 giveaway_id_, object_ptr<InputStorePaymentPurpose> &&purpose_)
  : peer_(std::move(peer_))
  , giveaway_id_(giveaway_id_)
  , purpose_(std::move(purpose_))
{}

const std::int32_t payments_launchPrepaidGiveaway::ID;

void payments_launchPrepaidGiveaway::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1609928480);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(giveaway_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_launchPrepaidGiveaway::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1609928480);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(giveaway_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_launchPrepaidGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.launchPrepaidGiveaway");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("giveaway_id", giveaway_id_);
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

payments_launchPrepaidGiveaway::ReturnType payments_launchPrepaidGiveaway::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_saveStarGift::payments_saveStarGift(int32 flags_, bool unsave_, object_ptr<InputSavedStarGift> &&stargift_)
  : flags_(flags_)
  , unsave_(unsave_)
  , stargift_(std::move(stargift_))
{}

const std::int32_t payments_saveStarGift::ID;

void payments_saveStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(707422588);
  TlStoreBinary::store((var0 = flags_ | (unsave_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void payments_saveStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(707422588);
  TlStoreBinary::store((var0 = flags_ | (unsave_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void payments_saveStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.saveStarGift");
    s.store_field("flags", (var0 = flags_ | (unsave_ << 0)));
    if (var0 & 1) { s.store_field("unsave", true); }
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_class_end();
  }
}

payments_saveStarGift::ReturnType payments_saveStarGift::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

phone_confirmCall::phone_confirmCall(object_ptr<inputPhoneCall> &&peer_, bytes &&g_a_, int64 key_fingerprint_, object_ptr<phoneCallProtocol> &&protocol_)
  : peer_(std::move(peer_))
  , g_a_(std::move(g_a_))
  , key_fingerprint_(key_fingerprint_)
  , protocol_(std::move(protocol_))
{}

const std::int32_t phone_confirmCall::ID;

void phone_confirmCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(788404002);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreString::store(g_a_, s);
  TlStoreBinary::store(key_fingerprint_, s);
  TlStoreBoxed<TlStoreObject, -58224696>::store(protocol_, s);
}

void phone_confirmCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(788404002);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreString::store(g_a_, s);
  TlStoreBinary::store(key_fingerprint_, s);
  TlStoreBoxed<TlStoreObject, -58224696>::store(protocol_, s);
}

void phone_confirmCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.confirmCall");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_bytes_field("g_a", g_a_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    s.store_class_end();
  }
}

phone_confirmCall::ReturnType phone_confirmCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_phoneCall>, -326966976>::parse(p);
#undef FAIL
}

phone_editGroupCallParticipant::phone_editGroupCallParticipant(int32 flags_, object_ptr<InputGroupCall> &&call_, object_ptr<InputPeer> &&participant_, bool muted_, int32 volume_, bool raise_hand_, bool video_stopped_, bool video_paused_, bool presentation_paused_)
  : flags_(flags_)
  , call_(std::move(call_))
  , participant_(std::move(participant_))
  , muted_(muted_)
  , volume_(volume_)
  , raise_hand_(raise_hand_)
  , video_stopped_(video_stopped_)
  , video_paused_(video_paused_)
  , presentation_paused_(presentation_paused_)
{}

const std::int32_t phone_editGroupCallParticipant::ID;

void phone_editGroupCallParticipant::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1524155713);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
  if (var0 & 1) { TlStoreBool::store(muted_, s); }
  if (var0 & 2) { TlStoreBinary::store(volume_, s); }
  if (var0 & 4) { TlStoreBool::store(raise_hand_, s); }
  if (var0 & 8) { TlStoreBool::store(video_stopped_, s); }
  if (var0 & 16) { TlStoreBool::store(video_paused_, s); }
  if (var0 & 32) { TlStoreBool::store(presentation_paused_, s); }
}

void phone_editGroupCallParticipant::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1524155713);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
  if (var0 & 1) { TlStoreBool::store(muted_, s); }
  if (var0 & 2) { TlStoreBinary::store(volume_, s); }
  if (var0 & 4) { TlStoreBool::store(raise_hand_, s); }
  if (var0 & 8) { TlStoreBool::store(video_stopped_, s); }
  if (var0 & 16) { TlStoreBool::store(video_paused_, s); }
  if (var0 & 32) { TlStoreBool::store(presentation_paused_, s); }
}

void phone_editGroupCallParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.editGroupCallParticipant");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    if (var0 & 1) { s.store_field("muted", muted_); }
    if (var0 & 2) { s.store_field("volume", volume_); }
    if (var0 & 4) { s.store_field("raise_hand", raise_hand_); }
    if (var0 & 8) { s.store_field("video_stopped", video_stopped_); }
    if (var0 & 16) { s.store_field("video_paused", video_paused_); }
    if (var0 & 32) { s.store_field("presentation_paused", presentation_paused_); }
    s.store_class_end();
  }
}

phone_editGroupCallParticipant::ReturnType phone_editGroupCallParticipant::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_leaveGroupCallPresentation::phone_leaveGroupCallPresentation(object_ptr<InputGroupCall> &&call_)
  : call_(std::move(call_))
{}

const std::int32_t phone_leaveGroupCallPresentation::ID;

void phone_leaveGroupCallPresentation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(475058500);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_leaveGroupCallPresentation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(475058500);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_leaveGroupCallPresentation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.leaveGroupCallPresentation");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

phone_leaveGroupCallPresentation::ReturnType phone_leaveGroupCallPresentation::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_sendSignalingData::phone_sendSignalingData(object_ptr<inputPhoneCall> &&peer_, bytes &&data_)
  : peer_(std::move(peer_))
  , data_(std::move(data_))
{}

const std::int32_t phone_sendSignalingData::ID;

void phone_sendSignalingData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-8744061);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreString::store(data_, s);
}

void phone_sendSignalingData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-8744061);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreString::store(data_, s);
}

void phone_sendSignalingData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.sendSignalingData");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

phone_sendSignalingData::ReturnType phone_sendSignalingData::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

phone_toggleGroupCallRecord::phone_toggleGroupCallRecord(int32 flags_, bool start_, bool video_, object_ptr<InputGroupCall> &&call_, string const &title_, bool video_portrait_)
  : flags_(flags_)
  , start_(start_)
  , video_(video_)
  , call_(std::move(call_))
  , title_(title_)
  , video_portrait_(video_portrait_)
{}

const std::int32_t phone_toggleGroupCallRecord::ID;

void phone_toggleGroupCallRecord::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-248985848);
  TlStoreBinary::store((var0 = flags_ | (start_ << 0) | (video_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreBool::store(video_portrait_, s); }
}

void phone_toggleGroupCallRecord::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-248985848);
  TlStoreBinary::store((var0 = flags_ | (start_ << 0) | (video_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 4) { TlStoreBool::store(video_portrait_, s); }
}

void phone_toggleGroupCallRecord::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.toggleGroupCallRecord");
    s.store_field("flags", (var0 = flags_ | (start_ << 0) | (video_ << 2)));
    if (var0 & 1) { s.store_field("start", true); }
    if (var0 & 4) { s.store_field("video", true); }
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    if (var0 & 2) { s.store_field("title", title_); }
    if (var0 & 4) { s.store_field("video_portrait", video_portrait_); }
    s.store_class_end();
  }
}

phone_toggleGroupCallRecord::ReturnType phone_toggleGroupCallRecord::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

stories_createAlbum::stories_createAlbum(object_ptr<InputPeer> &&peer_, string const &title_, array<int32> &&stories_)
  : peer_(std::move(peer_))
  , title_(title_)
  , stories_(std::move(stories_))
{}

const std::int32_t stories_createAlbum::ID;

void stories_createAlbum::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1553754395);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(stories_, s);
}

void stories_createAlbum::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1553754395);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(stories_, s);
}

void stories_createAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.createAlbum");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("title", title_);
    { s.store_vector_begin("stories", stories_.size()); for (const auto &_value : stories_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_createAlbum::ReturnType stories_createAlbum::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<storyAlbum>, -1826262950>::parse(p);
#undef FAIL
}

stories_exportStoryLink::stories_exportStoryLink(object_ptr<InputPeer> &&peer_, int32 id_)
  : peer_(std::move(peer_))
  , id_(id_)
{}

const std::int32_t stories_exportStoryLink::ID;

void stories_exportStoryLink::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2072899360);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void stories_exportStoryLink::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2072899360);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void stories_exportStoryLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.exportStoryLink");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

stories_exportStoryLink::ReturnType stories_exportStoryLink::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<exportedStoryLink>, 1070138683>::parse(p);
#undef FAIL
}

stories_getStoryViewsList::stories_getStoryViewsList(int32 flags_, bool just_contacts_, bool reactions_first_, bool forwards_first_, object_ptr<InputPeer> &&peer_, string const &q_, int32 id_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , just_contacts_(just_contacts_)
  , reactions_first_(reactions_first_)
  , forwards_first_(forwards_first_)
  , peer_(std::move(peer_))
  , q_(q_)
  , id_(id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t stories_getStoryViewsList::ID;

void stories_getStoryViewsList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2127707223);
  TlStoreBinary::store((var0 = flags_ | (just_contacts_ << 0) | (reactions_first_ << 2) | (forwards_first_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreString::store(q_, s); }
  TlStoreBinary::store(id_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getStoryViewsList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2127707223);
  TlStoreBinary::store((var0 = flags_ | (just_contacts_ << 0) | (reactions_first_ << 2) | (forwards_first_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreString::store(q_, s); }
  TlStoreBinary::store(id_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_getStoryViewsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getStoryViewsList");
    s.store_field("flags", (var0 = flags_ | (just_contacts_ << 0) | (reactions_first_ << 2) | (forwards_first_ << 3)));
    if (var0 & 1) { s.store_field("just_contacts", true); }
    if (var0 & 4) { s.store_field("reactions_first", true); }
    if (var0 & 8) { s.store_field("forwards_first", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 2) { s.store_field("q", q_); }
    s.store_field("id", id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stories_getStoryViewsList::ReturnType stories_getStoryViewsList::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_storyViewsList>, 1507299269>::parse(p);
#undef FAIL
}

stories_reorderAlbums::stories_reorderAlbums(object_ptr<InputPeer> &&peer_, array<int32> &&order_)
  : peer_(std::move(peer_))
  , order_(std::move(order_))
{}

const std::int32_t stories_reorderAlbums::ID;

void stories_reorderAlbums::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2060059687);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void stories_reorderAlbums::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2060059687);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void stories_reorderAlbums::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.reorderAlbums");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_reorderAlbums::ReturnType stories_reorderAlbums::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stories_sendStory::stories_sendStory(int32 flags_, bool pinned_, bool noforwards_, bool fwd_modified_, object_ptr<InputPeer> &&peer_, object_ptr<InputMedia> &&media_, array<object_ptr<MediaArea>> &&media_areas_, string const &caption_, array<object_ptr<MessageEntity>> &&entities_, array<object_ptr<InputPrivacyRule>> &&privacy_rules_, int64 random_id_, int32 period_, object_ptr<InputPeer> &&fwd_from_id_, int32 fwd_from_story_, array<int32> &&albums_)
  : flags_(flags_)
  , pinned_(pinned_)
  , noforwards_(noforwards_)
  , fwd_modified_(fwd_modified_)
  , peer_(std::move(peer_))
  , media_(std::move(media_))
  , media_areas_(std::move(media_areas_))
  , caption_(caption_)
  , entities_(std::move(entities_))
  , privacy_rules_(std::move(privacy_rules_))
  , random_id_(random_id_)
  , period_(period_)
  , fwd_from_id_(std::move(fwd_from_id_))
  , fwd_from_story_(fwd_from_story_)
  , albums_(std::move(albums_))
{}

const std::int32_t stories_sendStory::ID;

void stories_sendStory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1937752812);
  TlStoreBinary::store((var0 = flags_ | (pinned_ << 2) | (noforwards_ << 4) | (fwd_modified_ << 7)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
  if (var0 & 32) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(media_areas_, s); }
  if (var0 & 1) { TlStoreString::store(caption_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(privacy_rules_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 8) { TlStoreBinary::store(period_, s); }
  if (var0 & 64) { TlStoreBoxedUnknown<TlStoreObject>::store(fwd_from_id_, s); }
  if (var0 & 64) { TlStoreBinary::store(fwd_from_story_, s); }
  if (var0 & 256) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(albums_, s); }
}

void stories_sendStory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1937752812);
  TlStoreBinary::store((var0 = flags_ | (pinned_ << 2) | (noforwards_ << 4) | (fwd_modified_ << 7)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
  if (var0 & 32) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(media_areas_, s); }
  if (var0 & 1) { TlStoreString::store(caption_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(privacy_rules_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 8) { TlStoreBinary::store(period_, s); }
  if (var0 & 64) { TlStoreBoxedUnknown<TlStoreObject>::store(fwd_from_id_, s); }
  if (var0 & 64) { TlStoreBinary::store(fwd_from_story_, s); }
  if (var0 & 256) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(albums_, s); }
}

void stories_sendStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.sendStory");
    s.store_field("flags", (var0 = flags_ | (pinned_ << 2) | (noforwards_ << 4) | (fwd_modified_ << 7)));
    if (var0 & 4) { s.store_field("pinned", true); }
    if (var0 & 16) { s.store_field("noforwards", true); }
    if (var0 & 128) { s.store_field("fwd_modified", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    if (var0 & 32) { { s.store_vector_begin("media_areas", media_areas_.size()); for (const auto &_value : media_areas_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 1) { s.store_field("caption", caption_); }
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    { s.store_vector_begin("privacy_rules", privacy_rules_.size()); for (const auto &_value : privacy_rules_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("random_id", random_id_);
    if (var0 & 8) { s.store_field("period", period_); }
    if (var0 & 64) { s.store_object_field("fwd_from_id", static_cast<const BaseObject *>(fwd_from_id_.get())); }
    if (var0 & 64) { s.store_field("fwd_from_story", fwd_from_story_); }
    if (var0 & 256) { { s.store_vector_begin("albums", albums_.size()); for (const auto &_value : albums_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

stories_sendStory::ReturnType stories_sendStory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

updates_getChannelDifference::updates_getChannelDifference(int32 flags_, bool force_, object_ptr<InputChannel> &&channel_, object_ptr<ChannelMessagesFilter> &&filter_, int32 pts_, int32 limit_)
  : flags_(flags_)
  , force_(force_)
  , channel_(std::move(channel_))
  , filter_(std::move(filter_))
  , pts_(pts_)
  , limit_(limit_)
{}

const std::int32_t updates_getChannelDifference::ID;

void updates_getChannelDifference::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(51854712);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(pts_, s);
  TlStoreBinary::store(limit_, s);
}

void updates_getChannelDifference::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(51854712);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(pts_, s);
  TlStoreBinary::store(limit_, s);
}

void updates_getChannelDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.getChannelDifference");
    s.store_field("flags", (var0 = flags_ | (force_ << 0)));
    if (var0 & 1) { s.store_field("force", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("pts", pts_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

updates_getChannelDifference::ReturnType updates_getChannelDifference::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<updates_ChannelDifference>::parse(p);
#undef FAIL
}

upload_getCdnFileHashes::upload_getCdnFileHashes(bytes &&file_token_, int64 offset_)
  : file_token_(std::move(file_token_))
  , offset_(offset_)
{}

const std::int32_t upload_getCdnFileHashes::ID;

void upload_getCdnFileHashes::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1847836879);
  TlStoreString::store(file_token_, s);
  TlStoreBinary::store(offset_, s);
}

void upload_getCdnFileHashes::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1847836879);
  TlStoreString::store(file_token_, s);
  TlStoreBinary::store(offset_, s);
}

void upload_getCdnFileHashes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.getCdnFileHashes");
    s.store_bytes_field("file_token", file_token_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

upload_getCdnFileHashes::ReturnType upload_getCdnFileHashes::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<fileHash>, -207944868>>, 481674261>::parse(p);
#undef FAIL
}

upload_reuploadCdnFile::upload_reuploadCdnFile(bytes &&file_token_, bytes &&request_token_)
  : file_token_(std::move(file_token_))
  , request_token_(std::move(request_token_))
{}

const std::int32_t upload_reuploadCdnFile::ID;

void upload_reuploadCdnFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1691921240);
  TlStoreString::store(file_token_, s);
  TlStoreString::store(request_token_, s);
}

void upload_reuploadCdnFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1691921240);
  TlStoreString::store(file_token_, s);
  TlStoreString::store(request_token_, s);
}

void upload_reuploadCdnFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.reuploadCdnFile");
    s.store_bytes_field("file_token", file_token_);
    s.store_bytes_field("request_token", request_token_);
    s.store_class_end();
  }
}

upload_reuploadCdnFile::ReturnType upload_reuploadCdnFile::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<fileHash>, -207944868>>, 481674261>::parse(p);
#undef FAIL
}

object_ptr<Object> Object::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case accessPointRule::ID:
      return accessPointRule::fetch(p);
    case accountDaysTTL::ID:
      return accountDaysTTL::fetch(p);
    case attachMenuBot::ID:
      return attachMenuBot::fetch(p);
    case attachMenuBotIcon::ID:
      return attachMenuBotIcon::fetch(p);
    case attachMenuBotIconColor::ID:
      return attachMenuBotIconColor::fetch(p);
    case attachMenuBotsNotModified::ID:
      return attachMenuBotsNotModified::fetch(p);
    case attachMenuBots::ID:
      return attachMenuBots::fetch(p);
    case attachMenuBotsBot::ID:
      return attachMenuBotsBot::fetch(p);
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
    case authorization::ID:
      return authorization::fetch(p);
    case autoDownloadSettings::ID:
      return autoDownloadSettings::fetch(p);
    case autoSaveException::ID:
      return autoSaveException::fetch(p);
    case autoSaveSettings::ID:
      return autoSaveSettings::fetch(p);
    case availableEffect::ID:
      return availableEffect::fetch(p);
    case availableReaction::ID:
      return availableReaction::fetch(p);
    case bankCardOpenUrl::ID:
      return bankCardOpenUrl::fetch(p);
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
    case birthday::ID:
      return birthday::fetch(p);
    case boost::ID:
      return boost::fetch(p);
    case botAppNotModified::ID:
      return botAppNotModified::fetch(p);
    case botApp::ID:
      return botApp::fetch(p);
    case botAppSettings::ID:
      return botAppSettings::fetch(p);
    case botBusinessConnection::ID:
      return botBusinessConnection::fetch(p);
    case botCommand::ID:
      return botCommand::fetch(p);
    case botInfo::ID:
      return botInfo::fetch(p);
    case botInlineMessageMediaAuto::ID:
      return botInlineMessageMediaAuto::fetch(p);
    case botInlineMessageText::ID:
      return botInlineMessageText::fetch(p);
    case botInlineMessageMediaGeo::ID:
      return botInlineMessageMediaGeo::fetch(p);
    case botInlineMessageMediaVenue::ID:
      return botInlineMessageMediaVenue::fetch(p);
    case botInlineMessageMediaContact::ID:
      return botInlineMessageMediaContact::fetch(p);
    case botInlineMessageMediaInvoice::ID:
      return botInlineMessageMediaInvoice::fetch(p);
    case botInlineMessageMediaWebPage::ID:
      return botInlineMessageMediaWebPage::fetch(p);
    case botInlineResult::ID:
      return botInlineResult::fetch(p);
    case botInlineMediaResult::ID:
      return botInlineMediaResult::fetch(p);
    case botMenuButtonDefault::ID:
      return botMenuButtonDefault::fetch(p);
    case botMenuButtonCommands::ID:
      return botMenuButtonCommands::fetch(p);
    case botMenuButton::ID:
      return botMenuButton::fetch(p);
    case botPreviewMedia::ID:
      return botPreviewMedia::fetch(p);
    case botVerification::ID:
      return botVerification::fetch(p);
    case botVerifierSettings::ID:
      return botVerifierSettings::fetch(p);
    case businessAwayMessage::ID:
      return businessAwayMessage::fetch(p);
    case businessAwayMessageScheduleAlways::ID:
      return businessAwayMessageScheduleAlways::fetch(p);
    case businessAwayMessageScheduleOutsideWorkHours::ID:
      return businessAwayMessageScheduleOutsideWorkHours::fetch(p);
    case businessAwayMessageScheduleCustom::ID:
      return businessAwayMessageScheduleCustom::fetch(p);
    case businessBotRecipients::ID:
      return businessBotRecipients::fetch(p);
    case businessBotRights::ID:
      return businessBotRights::fetch(p);
    case businessChatLink::ID:
      return businessChatLink::fetch(p);
    case businessGreetingMessage::ID:
      return businessGreetingMessage::fetch(p);
    case businessIntro::ID:
      return businessIntro::fetch(p);
    case businessLocation::ID:
      return businessLocation::fetch(p);
    case businessRecipients::ID:
      return businessRecipients::fetch(p);
    case businessWeeklyOpen::ID:
      return businessWeeklyOpen::fetch(p);
    case businessWorkHours::ID:
      return businessWorkHours::fetch(p);
    case cdnConfig::ID:
      return cdnConfig::fetch(p);
    case cdnPublicKey::ID:
      return cdnPublicKey::fetch(p);
    case channelAdminLogEvent::ID:
      return channelAdminLogEvent::fetch(p);
    case channelAdminLogEventActionChangeTitle::ID:
      return channelAdminLogEventActionChangeTitle::fetch(p);
    case channelAdminLogEventActionChangeAbout::ID:
      return channelAdminLogEventActionChangeAbout::fetch(p);
    case channelAdminLogEventActionChangeUsername::ID:
      return channelAdminLogEventActionChangeUsername::fetch(p);
    case channelAdminLogEventActionChangePhoto::ID:
      return channelAdminLogEventActionChangePhoto::fetch(p);
    case channelAdminLogEventActionToggleInvites::ID:
      return channelAdminLogEventActionToggleInvites::fetch(p);
    case channelAdminLogEventActionToggleSignatures::ID:
      return channelAdminLogEventActionToggleSignatures::fetch(p);
    case channelAdminLogEventActionUpdatePinned::ID:
      return channelAdminLogEventActionUpdatePinned::fetch(p);
    case channelAdminLogEventActionEditMessage::ID:
      return channelAdminLogEventActionEditMessage::fetch(p);
    case channelAdminLogEventActionDeleteMessage::ID:
      return channelAdminLogEventActionDeleteMessage::fetch(p);
    case channelAdminLogEventActionParticipantJoin::ID:
      return channelAdminLogEventActionParticipantJoin::fetch(p);
    case channelAdminLogEventActionParticipantLeave::ID:
      return channelAdminLogEventActionParticipantLeave::fetch(p);
    case channelAdminLogEventActionParticipantInvite::ID:
      return channelAdminLogEventActionParticipantInvite::fetch(p);
    case channelAdminLogEventActionParticipantToggleBan::ID:
      return channelAdminLogEventActionParticipantToggleBan::fetch(p);
    case channelAdminLogEventActionParticipantToggleAdmin::ID:
      return channelAdminLogEventActionParticipantToggleAdmin::fetch(p);
    case channelAdminLogEventActionChangeStickerSet::ID:
      return channelAdminLogEventActionChangeStickerSet::fetch(p);
    case channelAdminLogEventActionTogglePreHistoryHidden::ID:
      return channelAdminLogEventActionTogglePreHistoryHidden::fetch(p);
    case channelAdminLogEventActionDefaultBannedRights::ID:
      return channelAdminLogEventActionDefaultBannedRights::fetch(p);
    case channelAdminLogEventActionStopPoll::ID:
      return channelAdminLogEventActionStopPoll::fetch(p);
    case channelAdminLogEventActionChangeLinkedChat::ID:
      return channelAdminLogEventActionChangeLinkedChat::fetch(p);
    case channelAdminLogEventActionChangeLocation::ID:
      return channelAdminLogEventActionChangeLocation::fetch(p);
    case channelAdminLogEventActionToggleSlowMode::ID:
      return channelAdminLogEventActionToggleSlowMode::fetch(p);
    case channelAdminLogEventActionStartGroupCall::ID:
      return channelAdminLogEventActionStartGroupCall::fetch(p);
    case channelAdminLogEventActionDiscardGroupCall::ID:
      return channelAdminLogEventActionDiscardGroupCall::fetch(p);
    case channelAdminLogEventActionParticipantMute::ID:
      return channelAdminLogEventActionParticipantMute::fetch(p);
    case channelAdminLogEventActionParticipantUnmute::ID:
      return channelAdminLogEventActionParticipantUnmute::fetch(p);
    case channelAdminLogEventActionToggleGroupCallSetting::ID:
      return channelAdminLogEventActionToggleGroupCallSetting::fetch(p);
    case channelAdminLogEventActionParticipantJoinByInvite::ID:
      return channelAdminLogEventActionParticipantJoinByInvite::fetch(p);
    case channelAdminLogEventActionExportedInviteDelete::ID:
      return channelAdminLogEventActionExportedInviteDelete::fetch(p);
    case channelAdminLogEventActionExportedInviteRevoke::ID:
      return channelAdminLogEventActionExportedInviteRevoke::fetch(p);
    case channelAdminLogEventActionExportedInviteEdit::ID:
      return channelAdminLogEventActionExportedInviteEdit::fetch(p);
    case channelAdminLogEventActionParticipantVolume::ID:
      return channelAdminLogEventActionParticipantVolume::fetch(p);
    case channelAdminLogEventActionChangeHistoryTTL::ID:
      return channelAdminLogEventActionChangeHistoryTTL::fetch(p);
    case channelAdminLogEventActionParticipantJoinByRequest::ID:
      return channelAdminLogEventActionParticipantJoinByRequest::fetch(p);
    case channelAdminLogEventActionToggleNoForwards::ID:
      return channelAdminLogEventActionToggleNoForwards::fetch(p);
    case channelAdminLogEventActionSendMessage::ID:
      return channelAdminLogEventActionSendMessage::fetch(p);
    case channelAdminLogEventActionChangeAvailableReactions::ID:
      return channelAdminLogEventActionChangeAvailableReactions::fetch(p);
    case channelAdminLogEventActionChangeUsernames::ID:
      return channelAdminLogEventActionChangeUsernames::fetch(p);
    case channelAdminLogEventActionToggleForum::ID:
      return channelAdminLogEventActionToggleForum::fetch(p);
    case channelAdminLogEventActionCreateTopic::ID:
      return channelAdminLogEventActionCreateTopic::fetch(p);
    case channelAdminLogEventActionEditTopic::ID:
      return channelAdminLogEventActionEditTopic::fetch(p);
    case channelAdminLogEventActionDeleteTopic::ID:
      return channelAdminLogEventActionDeleteTopic::fetch(p);
    case channelAdminLogEventActionPinTopic::ID:
      return channelAdminLogEventActionPinTopic::fetch(p);
    case channelAdminLogEventActionToggleAntiSpam::ID:
      return channelAdminLogEventActionToggleAntiSpam::fetch(p);
    case channelAdminLogEventActionChangePeerColor::ID:
      return channelAdminLogEventActionChangePeerColor::fetch(p);
    case channelAdminLogEventActionChangeProfilePeerColor::ID:
      return channelAdminLogEventActionChangeProfilePeerColor::fetch(p);
    case channelAdminLogEventActionChangeWallpaper::ID:
      return channelAdminLogEventActionChangeWallpaper::fetch(p);
    case channelAdminLogEventActionChangeEmojiStatus::ID:
      return channelAdminLogEventActionChangeEmojiStatus::fetch(p);
    case channelAdminLogEventActionChangeEmojiStickerSet::ID:
      return channelAdminLogEventActionChangeEmojiStickerSet::fetch(p);
    case channelAdminLogEventActionToggleSignatureProfiles::ID:
      return channelAdminLogEventActionToggleSignatureProfiles::fetch(p);
    case channelAdminLogEventActionParticipantSubExtend::ID:
      return channelAdminLogEventActionParticipantSubExtend::fetch(p);
    case channelAdminLogEventActionToggleAutotranslation::ID:
      return channelAdminLogEventActionToggleAutotranslation::fetch(p);
    case channelLocationEmpty::ID:
      return channelLocationEmpty::fetch(p);
    case channelLocation::ID:
      return channelLocation::fetch(p);
    case channelParticipant::ID:
      return channelParticipant::fetch(p);
    case channelParticipantSelf::ID:
      return channelParticipantSelf::fetch(p);
    case channelParticipantCreator::ID:
      return channelParticipantCreator::fetch(p);
    case channelParticipantAdmin::ID:
      return channelParticipantAdmin::fetch(p);
    case channelParticipantBanned::ID:
      return channelParticipantBanned::fetch(p);
    case channelParticipantLeft::ID:
      return channelParticipantLeft::fetch(p);
    case chatEmpty::ID:
      return chatEmpty::fetch(p);
    case chat::ID:
      return chat::fetch(p);
    case chatForbidden::ID:
      return chatForbidden::fetch(p);
    case channel::ID:
      return channel::fetch(p);
    case channelForbidden::ID:
      return channelForbidden::fetch(p);
    case chatAdminRights::ID:
      return chatAdminRights::fetch(p);
    case chatAdminWithInvites::ID:
      return chatAdminWithInvites::fetch(p);
    case chatBannedRights::ID:
      return chatBannedRights::fetch(p);
    case chatFull::ID:
      return chatFull::fetch(p);
    case channelFull::ID:
      return channelFull::fetch(p);
    case chatInviteAlready::ID:
      return chatInviteAlready::fetch(p);
    case chatInvite::ID:
      return chatInvite::fetch(p);
    case chatInvitePeek::ID:
      return chatInvitePeek::fetch(p);
    case chatInviteImporter::ID:
      return chatInviteImporter::fetch(p);
    case chatOnlines::ID:
      return chatOnlines::fetch(p);
    case chatParticipant::ID:
      return chatParticipant::fetch(p);
    case chatParticipantCreator::ID:
      return chatParticipantCreator::fetch(p);
    case chatParticipantAdmin::ID:
      return chatParticipantAdmin::fetch(p);
    case chatParticipantsForbidden::ID:
      return chatParticipantsForbidden::fetch(p);
    case chatParticipants::ID:
      return chatParticipants::fetch(p);
    case chatPhotoEmpty::ID:
      return chatPhotoEmpty::fetch(p);
    case chatPhoto::ID:
      return chatPhoto::fetch(p);
    case chatReactionsNone::ID:
      return chatReactionsNone::fetch(p);
    case chatReactionsAll::ID:
      return chatReactionsAll::fetch(p);
    case chatReactionsSome::ID:
      return chatReactionsSome::fetch(p);
    case chatTheme::ID:
      return chatTheme::fetch(p);
    case chatThemeUniqueGift::ID:
      return chatThemeUniqueGift::fetch(p);
    case config::ID:
      return config::fetch(p);
    case connectedBot::ID:
      return connectedBot::fetch(p);
    case connectedBotStarRef::ID:
      return connectedBotStarRef::fetch(p);
    case contact::ID:
      return contact::fetch(p);
    case contactBirthday::ID:
      return contactBirthday::fetch(p);
    case contactStatus::ID:
      return contactStatus::fetch(p);
    case dataJSON::ID:
      return dataJSON::fetch(p);
    case dcOption::ID:
      return dcOption::fetch(p);
    case defaultHistoryTTL::ID:
      return defaultHistoryTTL::fetch(p);
    case dialog::ID:
      return dialog::fetch(p);
    case dialogFolder::ID:
      return dialogFolder::fetch(p);
    case dialogFilter::ID:
      return dialogFilter::fetch(p);
    case dialogFilterDefault::ID:
      return dialogFilterDefault::fetch(p);
    case dialogFilterChatlist::ID:
      return dialogFilterChatlist::fetch(p);
    case dialogFilterSuggested::ID:
      return dialogFilterSuggested::fetch(p);
    case dialogPeer::ID:
      return dialogPeer::fetch(p);
    case dialogPeerFolder::ID:
      return dialogPeerFolder::fetch(p);
    case disallowedGiftsSettings::ID:
      return disallowedGiftsSettings::fetch(p);
    case documentEmpty::ID:
      return documentEmpty::fetch(p);
    case document::ID:
      return document::fetch(p);
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
    case draftMessageEmpty::ID:
      return draftMessageEmpty::fetch(p);
    case draftMessage::ID:
      return draftMessage::fetch(p);
    case emojiGroup::ID:
      return emojiGroup::fetch(p);
    case emojiGroupGreeting::ID:
      return emojiGroupGreeting::fetch(p);
    case emojiGroupPremium::ID:
      return emojiGroupPremium::fetch(p);
    case emojiKeyword::ID:
      return emojiKeyword::fetch(p);
    case emojiKeywordDeleted::ID:
      return emojiKeywordDeleted::fetch(p);
    case emojiKeywordsDifference::ID:
      return emojiKeywordsDifference::fetch(p);
    case emojiLanguage::ID:
      return emojiLanguage::fetch(p);
    case emojiListNotModified::ID:
      return emojiListNotModified::fetch(p);
    case emojiList::ID:
      return emojiList::fetch(p);
    case emojiStatusEmpty::ID:
      return emojiStatusEmpty::fetch(p);
    case emojiStatus::ID:
      return emojiStatus::fetch(p);
    case emojiStatusCollectible::ID:
      return emojiStatusCollectible::fetch(p);
    case inputEmojiStatusCollectible::ID:
      return inputEmojiStatusCollectible::fetch(p);
    case emojiURL::ID:
      return emojiURL::fetch(p);
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
    case encryptedFileEmpty::ID:
      return encryptedFileEmpty::fetch(p);
    case encryptedFile::ID:
      return encryptedFile::fetch(p);
    case encryptedMessage::ID:
      return encryptedMessage::fetch(p);
    case encryptedMessageService::ID:
      return encryptedMessageService::fetch(p);
    case error::ID:
      return error::fetch(p);
    case chatInviteExported::ID:
      return chatInviteExported::fetch(p);
    case chatInvitePublicJoinRequests::ID:
      return chatInvitePublicJoinRequests::fetch(p);
    case exportedChatlistInvite::ID:
      return exportedChatlistInvite::fetch(p);
    case exportedContactToken::ID:
      return exportedContactToken::fetch(p);
    case exportedMessageLink::ID:
      return exportedMessageLink::fetch(p);
    case exportedStoryLink::ID:
      return exportedStoryLink::fetch(p);
    case factCheck::ID:
      return factCheck::fetch(p);
    case fileHash::ID:
      return fileHash::fetch(p);
    case folder::ID:
      return folder::fetch(p);
    case folderPeer::ID:
      return folderPeer::fetch(p);
    case forumTopicDeleted::ID:
      return forumTopicDeleted::fetch(p);
    case forumTopic::ID:
      return forumTopic::fetch(p);
    case foundStory::ID:
      return foundStory::fetch(p);
    case game::ID:
      return game::fetch(p);
    case geoPointEmpty::ID:
      return geoPointEmpty::fetch(p);
    case geoPoint::ID:
      return geoPoint::fetch(p);
    case geoPointAddress::ID:
      return geoPointAddress::fetch(p);
    case globalPrivacySettings::ID:
      return globalPrivacySettings::fetch(p);
    case groupCallDiscarded::ID:
      return groupCallDiscarded::fetch(p);
    case groupCall::ID:
      return groupCall::fetch(p);
    case groupCallParticipant::ID:
      return groupCallParticipant::fetch(p);
    case groupCallParticipantVideo::ID:
      return groupCallParticipantVideo::fetch(p);
    case groupCallParticipantVideoSourceGroup::ID:
      return groupCallParticipantVideoSourceGroup::fetch(p);
    case groupCallStreamChannel::ID:
      return groupCallStreamChannel::fetch(p);
    case highScore::ID:
      return highScore::fetch(p);
    case importedContact::ID:
      return importedContact::fetch(p);
    case inlineBotSwitchPM::ID:
      return inlineBotSwitchPM::fetch(p);
    case inlineBotWebView::ID:
      return inlineBotWebView::fetch(p);
    case inlineQueryPeerTypeSameBotPM::ID:
      return inlineQueryPeerTypeSameBotPM::fetch(p);
    case inlineQueryPeerTypePM::ID:
      return inlineQueryPeerTypePM::fetch(p);
    case inlineQueryPeerTypeChat::ID:
      return inlineQueryPeerTypeChat::fetch(p);
    case inlineQueryPeerTypeMegagroup::ID:
      return inlineQueryPeerTypeMegagroup::fetch(p);
    case inlineQueryPeerTypeBroadcast::ID:
      return inlineQueryPeerTypeBroadcast::fetch(p);
    case inlineQueryPeerTypeBotPM::ID:
      return inlineQueryPeerTypeBotPM::fetch(p);
    case inputAppEvent::ID:
      return inputAppEvent::fetch(p);
    case inputBotInlineMessageID::ID:
      return inputBotInlineMessageID::fetch(p);
    case inputBotInlineMessageID64::ID:
      return inputBotInlineMessageID64::fetch(p);
    case inputChannelEmpty::ID:
      return inputChannelEmpty::fetch(p);
    case inputChannel::ID:
      return inputChannel::fetch(p);
    case inputChannelFromMessage::ID:
      return inputChannelFromMessage::fetch(p);
    case inputDocumentEmpty::ID:
      return inputDocumentEmpty::fetch(p);
    case inputDocument::ID:
      return inputDocument::fetch(p);
    case inputFile::ID:
      return inputFile::fetch(p);
    case inputFileBig::ID:
      return inputFileBig::fetch(p);
    case inputFileStoryDocument::ID:
      return inputFileStoryDocument::fetch(p);
    case inputGameID::ID:
      return inputGameID::fetch(p);
    case inputGameShortName::ID:
      return inputGameShortName::fetch(p);
    case inputGeoPointEmpty::ID:
      return inputGeoPointEmpty::fetch(p);
    case inputGeoPoint::ID:
      return inputGeoPoint::fetch(p);
    case inputGroupCall::ID:
      return inputGroupCall::fetch(p);
    case inputGroupCallSlug::ID:
      return inputGroupCallSlug::fetch(p);
    case inputGroupCallInviteMessage::ID:
      return inputGroupCallInviteMessage::fetch(p);
    case inputMediaEmpty::ID:
      return inputMediaEmpty::fetch(p);
    case inputMediaUploadedPhoto::ID:
      return inputMediaUploadedPhoto::fetch(p);
    case inputMediaPhoto::ID:
      return inputMediaPhoto::fetch(p);
    case inputMediaGeoPoint::ID:
      return inputMediaGeoPoint::fetch(p);
    case inputMediaContact::ID:
      return inputMediaContact::fetch(p);
    case inputMediaUploadedDocument::ID:
      return inputMediaUploadedDocument::fetch(p);
    case inputMediaDocument::ID:
      return inputMediaDocument::fetch(p);
    case inputMediaVenue::ID:
      return inputMediaVenue::fetch(p);
    case inputMediaPhotoExternal::ID:
      return inputMediaPhotoExternal::fetch(p);
    case inputMediaDocumentExternal::ID:
      return inputMediaDocumentExternal::fetch(p);
    case inputMediaGame::ID:
      return inputMediaGame::fetch(p);
    case inputMediaInvoice::ID:
      return inputMediaInvoice::fetch(p);
    case inputMediaGeoLive::ID:
      return inputMediaGeoLive::fetch(p);
    case inputMediaPoll::ID:
      return inputMediaPoll::fetch(p);
    case inputMediaDice::ID:
      return inputMediaDice::fetch(p);
    case inputMediaStory::ID:
      return inputMediaStory::fetch(p);
    case inputMediaWebPage::ID:
      return inputMediaWebPage::fetch(p);
    case inputMediaPaidMedia::ID:
      return inputMediaPaidMedia::fetch(p);
    case inputMediaTodo::ID:
      return inputMediaTodo::fetch(p);
    case inputPeerEmpty::ID:
      return inputPeerEmpty::fetch(p);
    case inputPeerSelf::ID:
      return inputPeerSelf::fetch(p);
    case inputPeerChat::ID:
      return inputPeerChat::fetch(p);
    case inputPeerUser::ID:
      return inputPeerUser::fetch(p);
    case inputPeerChannel::ID:
      return inputPeerChannel::fetch(p);
    case inputPeerUserFromMessage::ID:
      return inputPeerUserFromMessage::fetch(p);
    case inputPeerChannelFromMessage::ID:
      return inputPeerChannelFromMessage::fetch(p);
    case inputPhotoEmpty::ID:
      return inputPhotoEmpty::fetch(p);
    case inputPhoto::ID:
      return inputPhoto::fetch(p);
    case inputReplyToMessage::ID:
      return inputReplyToMessage::fetch(p);
    case inputReplyToStory::ID:
      return inputReplyToStory::fetch(p);
    case inputReplyToMonoForum::ID:
      return inputReplyToMonoForum::fetch(p);
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
    case inputUserEmpty::ID:
      return inputUserEmpty::fetch(p);
    case inputUserSelf::ID:
      return inputUserSelf::fetch(p);
    case inputUser::ID:
      return inputUser::fetch(p);
    case inputUserFromMessage::ID:
      return inputUserFromMessage::fetch(p);
    case inputWebDocument::ID:
      return inputWebDocument::fetch(p);
    case invoice::ID:
      return invoice::fetch(p);
    case ipPort::ID:
      return ipPort::fetch(p);
    case ipPortSecret::ID:
      return ipPortSecret::fetch(p);
    case jsonObjectValue::ID:
      return jsonObjectValue::fetch(p);
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
    case keyboardButton::ID:
      return keyboardButton::fetch(p);
    case keyboardButtonUrl::ID:
      return keyboardButtonUrl::fetch(p);
    case keyboardButtonCallback::ID:
      return keyboardButtonCallback::fetch(p);
    case keyboardButtonRequestPhone::ID:
      return keyboardButtonRequestPhone::fetch(p);
    case keyboardButtonRequestGeoLocation::ID:
      return keyboardButtonRequestGeoLocation::fetch(p);
    case keyboardButtonSwitchInline::ID:
      return keyboardButtonSwitchInline::fetch(p);
    case keyboardButtonGame::ID:
      return keyboardButtonGame::fetch(p);
    case keyboardButtonBuy::ID:
      return keyboardButtonBuy::fetch(p);
    case keyboardButtonUrlAuth::ID:
      return keyboardButtonUrlAuth::fetch(p);
    case inputKeyboardButtonUrlAuth::ID:
      return inputKeyboardButtonUrlAuth::fetch(p);
    case keyboardButtonRequestPoll::ID:
      return keyboardButtonRequestPoll::fetch(p);
    case inputKeyboardButtonUserProfile::ID:
      return inputKeyboardButtonUserProfile::fetch(p);
    case keyboardButtonUserProfile::ID:
      return keyboardButtonUserProfile::fetch(p);
    case keyboardButtonWebView::ID:
      return keyboardButtonWebView::fetch(p);
    case keyboardButtonSimpleWebView::ID:
      return keyboardButtonSimpleWebView::fetch(p);
    case keyboardButtonRequestPeer::ID:
      return keyboardButtonRequestPeer::fetch(p);
    case inputKeyboardButtonRequestPeer::ID:
      return inputKeyboardButtonRequestPeer::fetch(p);
    case keyboardButtonCopy::ID:
      return keyboardButtonCopy::fetch(p);
    case keyboardButtonRow::ID:
      return keyboardButtonRow::fetch(p);
    case labeledPrice::ID:
      return labeledPrice::fetch(p);
    case langPackDifference::ID:
      return langPackDifference::fetch(p);
    case langPackLanguage::ID:
      return langPackLanguage::fetch(p);
    case langPackString::ID:
      return langPackString::fetch(p);
    case langPackStringPluralized::ID:
      return langPackStringPluralized::fetch(p);
    case langPackStringDeleted::ID:
      return langPackStringDeleted::fetch(p);
    case maskCoords::ID:
      return maskCoords::fetch(p);
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
    case mediaAreaCoordinates::ID:
      return mediaAreaCoordinates::fetch(p);
    case messageEmpty::ID:
      return messageEmpty::fetch(p);
    case message::ID:
      return message::fetch(p);
    case messageService::ID:
      return messageService::fetch(p);
    case messageActionEmpty::ID:
      return messageActionEmpty::fetch(p);
    case messageActionChatCreate::ID:
      return messageActionChatCreate::fetch(p);
    case messageActionChatEditTitle::ID:
      return messageActionChatEditTitle::fetch(p);
    case messageActionChatEditPhoto::ID:
      return messageActionChatEditPhoto::fetch(p);
    case messageActionChatDeletePhoto::ID:
      return messageActionChatDeletePhoto::fetch(p);
    case messageActionChatAddUser::ID:
      return messageActionChatAddUser::fetch(p);
    case messageActionChatDeleteUser::ID:
      return messageActionChatDeleteUser::fetch(p);
    case messageActionChatJoinedByLink::ID:
      return messageActionChatJoinedByLink::fetch(p);
    case messageActionChannelCreate::ID:
      return messageActionChannelCreate::fetch(p);
    case messageActionChatMigrateTo::ID:
      return messageActionChatMigrateTo::fetch(p);
    case messageActionChannelMigrateFrom::ID:
      return messageActionChannelMigrateFrom::fetch(p);
    case messageActionPinMessage::ID:
      return messageActionPinMessage::fetch(p);
    case messageActionHistoryClear::ID:
      return messageActionHistoryClear::fetch(p);
    case messageActionGameScore::ID:
      return messageActionGameScore::fetch(p);
    case messageActionPaymentSentMe::ID:
      return messageActionPaymentSentMe::fetch(p);
    case messageActionPaymentSent::ID:
      return messageActionPaymentSent::fetch(p);
    case messageActionPhoneCall::ID:
      return messageActionPhoneCall::fetch(p);
    case messageActionScreenshotTaken::ID:
      return messageActionScreenshotTaken::fetch(p);
    case messageActionCustomAction::ID:
      return messageActionCustomAction::fetch(p);
    case messageActionBotAllowed::ID:
      return messageActionBotAllowed::fetch(p);
    case messageActionSecureValuesSentMe::ID:
      return messageActionSecureValuesSentMe::fetch(p);
    case messageActionSecureValuesSent::ID:
      return messageActionSecureValuesSent::fetch(p);
    case messageActionContactSignUp::ID:
      return messageActionContactSignUp::fetch(p);
    case messageActionGeoProximityReached::ID:
      return messageActionGeoProximityReached::fetch(p);
    case messageActionGroupCall::ID:
      return messageActionGroupCall::fetch(p);
    case messageActionInviteToGroupCall::ID:
      return messageActionInviteToGroupCall::fetch(p);
    case messageActionSetMessagesTTL::ID:
      return messageActionSetMessagesTTL::fetch(p);
    case messageActionGroupCallScheduled::ID:
      return messageActionGroupCallScheduled::fetch(p);
    case messageActionSetChatTheme::ID:
      return messageActionSetChatTheme::fetch(p);
    case messageActionChatJoinedByRequest::ID:
      return messageActionChatJoinedByRequest::fetch(p);
    case messageActionWebViewDataSentMe::ID:
      return messageActionWebViewDataSentMe::fetch(p);
    case messageActionWebViewDataSent::ID:
      return messageActionWebViewDataSent::fetch(p);
    case messageActionGiftPremium::ID:
      return messageActionGiftPremium::fetch(p);
    case messageActionTopicCreate::ID:
      return messageActionTopicCreate::fetch(p);
    case messageActionTopicEdit::ID:
      return messageActionTopicEdit::fetch(p);
    case messageActionSuggestProfilePhoto::ID:
      return messageActionSuggestProfilePhoto::fetch(p);
    case messageActionRequestedPeer::ID:
      return messageActionRequestedPeer::fetch(p);
    case messageActionSetChatWallPaper::ID:
      return messageActionSetChatWallPaper::fetch(p);
    case messageActionGiftCode::ID:
      return messageActionGiftCode::fetch(p);
    case messageActionGiveawayLaunch::ID:
      return messageActionGiveawayLaunch::fetch(p);
    case messageActionGiveawayResults::ID:
      return messageActionGiveawayResults::fetch(p);
    case messageActionBoostApply::ID:
      return messageActionBoostApply::fetch(p);
    case messageActionRequestedPeerSentMe::ID:
      return messageActionRequestedPeerSentMe::fetch(p);
    case messageActionPaymentRefunded::ID:
      return messageActionPaymentRefunded::fetch(p);
    case messageActionGiftStars::ID:
      return messageActionGiftStars::fetch(p);
    case messageActionPrizeStars::ID:
      return messageActionPrizeStars::fetch(p);
    case messageActionStarGift::ID:
      return messageActionStarGift::fetch(p);
    case messageActionStarGiftUnique::ID:
      return messageActionStarGiftUnique::fetch(p);
    case messageActionPaidMessagesRefunded::ID:
      return messageActionPaidMessagesRefunded::fetch(p);
    case messageActionPaidMessagesPrice::ID:
      return messageActionPaidMessagesPrice::fetch(p);
    case messageActionConferenceCall::ID:
      return messageActionConferenceCall::fetch(p);
    case messageActionTodoCompletions::ID:
      return messageActionTodoCompletions::fetch(p);
    case messageActionTodoAppendTasks::ID:
      return messageActionTodoAppendTasks::fetch(p);
    case messageActionSuggestedPostApproval::ID:
      return messageActionSuggestedPostApproval::fetch(p);
    case messageActionSuggestedPostSuccess::ID:
      return messageActionSuggestedPostSuccess::fetch(p);
    case messageActionSuggestedPostRefund::ID:
      return messageActionSuggestedPostRefund::fetch(p);
    case messageActionGiftTon::ID:
      return messageActionGiftTon::fetch(p);
    case messageActionSuggestBirthday::ID:
      return messageActionSuggestBirthday::fetch(p);
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
    case messageExtendedMediaPreview::ID:
      return messageExtendedMediaPreview::fetch(p);
    case messageExtendedMedia::ID:
      return messageExtendedMedia::fetch(p);
    case messageFwdHeader::ID:
      return messageFwdHeader::fetch(p);
    case messageMediaEmpty::ID:
      return messageMediaEmpty::fetch(p);
    case messageMediaPhoto::ID:
      return messageMediaPhoto::fetch(p);
    case messageMediaGeo::ID:
      return messageMediaGeo::fetch(p);
    case messageMediaContact::ID:
      return messageMediaContact::fetch(p);
    case messageMediaUnsupported::ID:
      return messageMediaUnsupported::fetch(p);
    case messageMediaDocument::ID:
      return messageMediaDocument::fetch(p);
    case messageMediaWebPage::ID:
      return messageMediaWebPage::fetch(p);
    case messageMediaVenue::ID:
      return messageMediaVenue::fetch(p);
    case messageMediaGame::ID:
      return messageMediaGame::fetch(p);
    case messageMediaInvoice::ID:
      return messageMediaInvoice::fetch(p);
    case messageMediaGeoLive::ID:
      return messageMediaGeoLive::fetch(p);
    case messageMediaPoll::ID:
      return messageMediaPoll::fetch(p);
    case messageMediaDice::ID:
      return messageMediaDice::fetch(p);
    case messageMediaStory::ID:
      return messageMediaStory::fetch(p);
    case messageMediaGiveaway::ID:
      return messageMediaGiveaway::fetch(p);
    case messageMediaGiveawayResults::ID:
      return messageMediaGiveawayResults::fetch(p);
    case messageMediaPaidMedia::ID:
      return messageMediaPaidMedia::fetch(p);
    case messageMediaToDo::ID:
      return messageMediaToDo::fetch(p);
    case messagePeerReaction::ID:
      return messagePeerReaction::fetch(p);
    case messagePeerVote::ID:
      return messagePeerVote::fetch(p);
    case messagePeerVoteInputOption::ID:
      return messagePeerVoteInputOption::fetch(p);
    case messagePeerVoteMultiple::ID:
      return messagePeerVoteMultiple::fetch(p);
    case messageRange::ID:
      return messageRange::fetch(p);
    case messageReactions::ID:
      return messageReactions::fetch(p);
    case messageReactor::ID:
      return messageReactor::fetch(p);
    case messageReplies::ID:
      return messageReplies::fetch(p);
    case messageReplyHeader::ID:
      return messageReplyHeader::fetch(p);
    case messageReplyStoryHeader::ID:
      return messageReplyStoryHeader::fetch(p);
    case messageReportOption::ID:
      return messageReportOption::fetch(p);
    case messageViews::ID:
      return messageViews::fetch(p);
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
    case missingInvitee::ID:
      return missingInvitee::fetch(p);
    case myBoost::ID:
      return myBoost::fetch(p);
    case nearestDc::ID:
      return nearestDc::fetch(p);
    case notificationSoundDefault::ID:
      return notificationSoundDefault::fetch(p);
    case notificationSoundNone::ID:
      return notificationSoundNone::fetch(p);
    case notificationSoundLocal::ID:
      return notificationSoundLocal::fetch(p);
    case notificationSoundRingtone::ID:
      return notificationSoundRingtone::fetch(p);
    case notifyPeer::ID:
      return notifyPeer::fetch(p);
    case notifyUsers::ID:
      return notifyUsers::fetch(p);
    case notifyChats::ID:
      return notifyChats::fetch(p);
    case notifyBroadcasts::ID:
      return notifyBroadcasts::fetch(p);
    case notifyForumTopic::ID:
      return notifyForumTopic::fetch(p);
    case outboxReadDate::ID:
      return outboxReadDate::fetch(p);
    case page::ID:
      return page::fetch(p);
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
    case pageCaption::ID:
      return pageCaption::fetch(p);
    case pageListItemText::ID:
      return pageListItemText::fetch(p);
    case pageListItemBlocks::ID:
      return pageListItemBlocks::fetch(p);
    case pageListOrderedItemText::ID:
      return pageListOrderedItemText::fetch(p);
    case pageListOrderedItemBlocks::ID:
      return pageListOrderedItemBlocks::fetch(p);
    case pageRelatedArticle::ID:
      return pageRelatedArticle::fetch(p);
    case pageTableCell::ID:
      return pageTableCell::fetch(p);
    case pageTableRow::ID:
      return pageTableRow::fetch(p);
    case paidReactionPrivacyDefault::ID:
      return paidReactionPrivacyDefault::fetch(p);
    case paidReactionPrivacyAnonymous::ID:
      return paidReactionPrivacyAnonymous::fetch(p);
    case paidReactionPrivacyPeer::ID:
      return paidReactionPrivacyPeer::fetch(p);
    case passwordKdfAlgoUnknown::ID:
      return passwordKdfAlgoUnknown::fetch(p);
    case passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::ID:
      return passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::fetch(p);
    case paymentCharge::ID:
      return paymentCharge::fetch(p);
    case paymentFormMethod::ID:
      return paymentFormMethod::fetch(p);
    case paymentRequestedInfo::ID:
      return paymentRequestedInfo::fetch(p);
    case paymentSavedCredentialsCard::ID:
      return paymentSavedCredentialsCard::fetch(p);
    case peerUser::ID:
      return peerUser::fetch(p);
    case peerChat::ID:
      return peerChat::fetch(p);
    case peerChannel::ID:
      return peerChannel::fetch(p);
    case peerBlocked::ID:
      return peerBlocked::fetch(p);
    case peerColor::ID:
      return peerColor::fetch(p);
    case peerColorCollectible::ID:
      return peerColorCollectible::fetch(p);
    case inputPeerColorCollectible::ID:
      return inputPeerColorCollectible::fetch(p);
    case peerLocated::ID:
      return peerLocated::fetch(p);
    case peerSelfLocated::ID:
      return peerSelfLocated::fetch(p);
    case peerNotifySettings::ID:
      return peerNotifySettings::fetch(p);
    case peerSettings::ID:
      return peerSettings::fetch(p);
    case peerStories::ID:
      return peerStories::fetch(p);
    case pendingSuggestion::ID:
      return pendingSuggestion::fetch(p);
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
    case phoneCallDiscardReasonMissed::ID:
      return phoneCallDiscardReasonMissed::fetch(p);
    case phoneCallDiscardReasonDisconnect::ID:
      return phoneCallDiscardReasonDisconnect::fetch(p);
    case phoneCallDiscardReasonHangup::ID:
      return phoneCallDiscardReasonHangup::fetch(p);
    case phoneCallDiscardReasonBusy::ID:
      return phoneCallDiscardReasonBusy::fetch(p);
    case phoneCallDiscardReasonMigrateConferenceCall::ID:
      return phoneCallDiscardReasonMigrateConferenceCall::fetch(p);
    case phoneCallProtocol::ID:
      return phoneCallProtocol::fetch(p);
    case phoneConnection::ID:
      return phoneConnection::fetch(p);
    case phoneConnectionWebrtc::ID:
      return phoneConnectionWebrtc::fetch(p);
    case photoEmpty::ID:
      return photoEmpty::fetch(p);
    case photo::ID:
      return photo::fetch(p);
    case photoSizeEmpty::ID:
      return photoSizeEmpty::fetch(p);
    case photoSize::ID:
      return photoSize::fetch(p);
    case photoCachedSize::ID:
      return photoCachedSize::fetch(p);
    case photoStrippedSize::ID:
      return photoStrippedSize::fetch(p);
    case photoSizeProgressive::ID:
      return photoSizeProgressive::fetch(p);
    case photoPathSize::ID:
      return photoPathSize::fetch(p);
    case poll::ID:
      return poll::fetch(p);
    case pollAnswer::ID:
      return pollAnswer::fetch(p);
    case pollAnswerVoters::ID:
      return pollAnswerVoters::fetch(p);
    case pollResults::ID:
      return pollResults::fetch(p);
    case popularContact::ID:
      return popularContact::fetch(p);
    case postAddress::ID:
      return postAddress::fetch(p);
    case postInteractionCountersMessage::ID:
      return postInteractionCountersMessage::fetch(p);
    case postInteractionCountersStory::ID:
      return postInteractionCountersStory::fetch(p);
    case premiumGiftCodeOption::ID:
      return premiumGiftCodeOption::fetch(p);
    case premiumSubscriptionOption::ID:
      return premiumSubscriptionOption::fetch(p);
    case prepaidGiveaway::ID:
      return prepaidGiveaway::fetch(p);
    case prepaidStarsGiveaway::ID:
      return prepaidStarsGiveaway::fetch(p);
    case privacyKeyStatusTimestamp::ID:
      return privacyKeyStatusTimestamp::fetch(p);
    case privacyKeyChatInvite::ID:
      return privacyKeyChatInvite::fetch(p);
    case privacyKeyPhoneCall::ID:
      return privacyKeyPhoneCall::fetch(p);
    case privacyKeyPhoneP2P::ID:
      return privacyKeyPhoneP2P::fetch(p);
    case privacyKeyForwards::ID:
      return privacyKeyForwards::fetch(p);
    case privacyKeyProfilePhoto::ID:
      return privacyKeyProfilePhoto::fetch(p);
    case privacyKeyPhoneNumber::ID:
      return privacyKeyPhoneNumber::fetch(p);
    case privacyKeyAddedByPhone::ID:
      return privacyKeyAddedByPhone::fetch(p);
    case privacyKeyVoiceMessages::ID:
      return privacyKeyVoiceMessages::fetch(p);
    case privacyKeyAbout::ID:
      return privacyKeyAbout::fetch(p);
    case privacyKeyBirthday::ID:
      return privacyKeyBirthday::fetch(p);
    case privacyKeyStarGiftsAutoSave::ID:
      return privacyKeyStarGiftsAutoSave::fetch(p);
    case privacyKeyNoPaidMessages::ID:
      return privacyKeyNoPaidMessages::fetch(p);
    case privacyValueAllowContacts::ID:
      return privacyValueAllowContacts::fetch(p);
    case privacyValueAllowAll::ID:
      return privacyValueAllowAll::fetch(p);
    case privacyValueAllowUsers::ID:
      return privacyValueAllowUsers::fetch(p);
    case privacyValueDisallowContacts::ID:
      return privacyValueDisallowContacts::fetch(p);
    case privacyValueDisallowAll::ID:
      return privacyValueDisallowAll::fetch(p);
    case privacyValueDisallowUsers::ID:
      return privacyValueDisallowUsers::fetch(p);
    case privacyValueAllowChatParticipants::ID:
      return privacyValueAllowChatParticipants::fetch(p);
    case privacyValueDisallowChatParticipants::ID:
      return privacyValueDisallowChatParticipants::fetch(p);
    case privacyValueAllowCloseFriends::ID:
      return privacyValueAllowCloseFriends::fetch(p);
    case privacyValueAllowPremium::ID:
      return privacyValueAllowPremium::fetch(p);
    case privacyValueAllowBots::ID:
      return privacyValueAllowBots::fetch(p);
    case privacyValueDisallowBots::ID:
      return privacyValueDisallowBots::fetch(p);
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
    case publicForwardMessage::ID:
      return publicForwardMessage::fetch(p);
    case publicForwardStory::ID:
      return publicForwardStory::fetch(p);
    case quickReply::ID:
      return quickReply::fetch(p);
    case reactionEmpty::ID:
      return reactionEmpty::fetch(p);
    case reactionEmoji::ID:
      return reactionEmoji::fetch(p);
    case reactionCustomEmoji::ID:
      return reactionCustomEmoji::fetch(p);
    case reactionPaid::ID:
      return reactionPaid::fetch(p);
    case reactionCount::ID:
      return reactionCount::fetch(p);
    case reactionNotificationsFromContacts::ID:
      return reactionNotificationsFromContacts::fetch(p);
    case reactionNotificationsFromAll::ID:
      return reactionNotificationsFromAll::fetch(p);
    case reactionsNotifySettings::ID:
      return reactionsNotifySettings::fetch(p);
    case readParticipantDate::ID:
      return readParticipantDate::fetch(p);
    case receivedNotifyMessage::ID:
      return receivedNotifyMessage::fetch(p);
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
    case replyKeyboardHide::ID:
      return replyKeyboardHide::fetch(p);
    case replyKeyboardForceReply::ID:
      return replyKeyboardForceReply::fetch(p);
    case replyKeyboardMarkup::ID:
      return replyKeyboardMarkup::fetch(p);
    case replyInlineMarkup::ID:
      return replyInlineMarkup::fetch(p);
    case reportResultChooseOption::ID:
      return reportResultChooseOption::fetch(p);
    case reportResultAddComment::ID:
      return reportResultAddComment::fetch(p);
    case reportResultReported::ID:
      return reportResultReported::fetch(p);
    case requestPeerTypeUser::ID:
      return requestPeerTypeUser::fetch(p);
    case requestPeerTypeChat::ID:
      return requestPeerTypeChat::fetch(p);
    case requestPeerTypeBroadcast::ID:
      return requestPeerTypeBroadcast::fetch(p);
    case requestedPeerUser::ID:
      return requestedPeerUser::fetch(p);
    case requestedPeerChat::ID:
      return requestedPeerChat::fetch(p);
    case requestedPeerChannel::ID:
      return requestedPeerChannel::fetch(p);
    case requirementToContactEmpty::ID:
      return requirementToContactEmpty::fetch(p);
    case requirementToContactPremium::ID:
      return requirementToContactPremium::fetch(p);
    case requirementToContactPaidMessages::ID:
      return requirementToContactPaidMessages::fetch(p);
    case restrictionReason::ID:
      return restrictionReason::fetch(p);
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
    case savedPhoneContact::ID:
      return savedPhoneContact::fetch(p);
    case savedDialog::ID:
      return savedDialog::fetch(p);
    case monoForumDialog::ID:
      return monoForumDialog::fetch(p);
    case savedReactionTag::ID:
      return savedReactionTag::fetch(p);
    case savedStarGift::ID:
      return savedStarGift::fetch(p);
    case searchPostsFlood::ID:
      return searchPostsFlood::fetch(p);
    case searchResultsCalendarPeriod::ID:
      return searchResultsCalendarPeriod::fetch(p);
    case searchResultPosition::ID:
      return searchResultPosition::fetch(p);
    case secureCredentialsEncrypted::ID:
      return secureCredentialsEncrypted::fetch(p);
    case secureData::ID:
      return secureData::fetch(p);
    case secureFileEmpty::ID:
      return secureFileEmpty::fetch(p);
    case secureFile::ID:
      return secureFile::fetch(p);
    case securePasswordKdfAlgoUnknown::ID:
      return securePasswordKdfAlgoUnknown::fetch(p);
    case securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::ID:
      return securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::fetch(p);
    case securePasswordKdfAlgoSHA512::ID:
      return securePasswordKdfAlgoSHA512::fetch(p);
    case securePlainPhone::ID:
      return securePlainPhone::fetch(p);
    case securePlainEmail::ID:
      return securePlainEmail::fetch(p);
    case secureRequiredType::ID:
      return secureRequiredType::fetch(p);
    case secureRequiredTypeOneOf::ID:
      return secureRequiredTypeOneOf::fetch(p);
    case secureSecretSettings::ID:
      return secureSecretSettings::fetch(p);
    case secureValue::ID:
      return secureValue::fetch(p);
    case secureValueErrorData::ID:
      return secureValueErrorData::fetch(p);
    case secureValueErrorFrontSide::ID:
      return secureValueErrorFrontSide::fetch(p);
    case secureValueErrorReverseSide::ID:
      return secureValueErrorReverseSide::fetch(p);
    case secureValueErrorSelfie::ID:
      return secureValueErrorSelfie::fetch(p);
    case secureValueErrorFile::ID:
      return secureValueErrorFile::fetch(p);
    case secureValueErrorFiles::ID:
      return secureValueErrorFiles::fetch(p);
    case secureValueError::ID:
      return secureValueError::fetch(p);
    case secureValueErrorTranslationFile::ID:
      return secureValueErrorTranslationFile::fetch(p);
    case secureValueErrorTranslationFiles::ID:
      return secureValueErrorTranslationFiles::fetch(p);
    case secureValueTypePersonalDetails::ID:
      return secureValueTypePersonalDetails::fetch(p);
    case secureValueTypePassport::ID:
      return secureValueTypePassport::fetch(p);
    case secureValueTypeDriverLicense::ID:
      return secureValueTypeDriverLicense::fetch(p);
    case secureValueTypeIdentityCard::ID:
      return secureValueTypeIdentityCard::fetch(p);
    case secureValueTypeInternalPassport::ID:
      return secureValueTypeInternalPassport::fetch(p);
    case secureValueTypeAddress::ID:
      return secureValueTypeAddress::fetch(p);
    case secureValueTypeUtilityBill::ID:
      return secureValueTypeUtilityBill::fetch(p);
    case secureValueTypeBankStatement::ID:
      return secureValueTypeBankStatement::fetch(p);
    case secureValueTypeRentalAgreement::ID:
      return secureValueTypeRentalAgreement::fetch(p);
    case secureValueTypePassportRegistration::ID:
      return secureValueTypePassportRegistration::fetch(p);
    case secureValueTypeTemporaryRegistration::ID:
      return secureValueTypeTemporaryRegistration::fetch(p);
    case secureValueTypePhone::ID:
      return secureValueTypePhone::fetch(p);
    case secureValueTypeEmail::ID:
      return secureValueTypeEmail::fetch(p);
    case sendAsPeer::ID:
      return sendAsPeer::fetch(p);
    case sendMessageTypingAction::ID:
      return sendMessageTypingAction::fetch(p);
    case sendMessageCancelAction::ID:
      return sendMessageCancelAction::fetch(p);
    case sendMessageRecordVideoAction::ID:
      return sendMessageRecordVideoAction::fetch(p);
    case sendMessageUploadVideoAction::ID:
      return sendMessageUploadVideoAction::fetch(p);
    case sendMessageRecordAudioAction::ID:
      return sendMessageRecordAudioAction::fetch(p);
    case sendMessageUploadAudioAction::ID:
      return sendMessageUploadAudioAction::fetch(p);
    case sendMessageUploadPhotoAction::ID:
      return sendMessageUploadPhotoAction::fetch(p);
    case sendMessageUploadDocumentAction::ID:
      return sendMessageUploadDocumentAction::fetch(p);
    case sendMessageGeoLocationAction::ID:
      return sendMessageGeoLocationAction::fetch(p);
    case sendMessageChooseContactAction::ID:
      return sendMessageChooseContactAction::fetch(p);
    case sendMessageGamePlayAction::ID:
      return sendMessageGamePlayAction::fetch(p);
    case sendMessageRecordRoundAction::ID:
      return sendMessageRecordRoundAction::fetch(p);
    case sendMessageUploadRoundAction::ID:
      return sendMessageUploadRoundAction::fetch(p);
    case speakingInGroupCallAction::ID:
      return speakingInGroupCallAction::fetch(p);
    case sendMessageHistoryImportAction::ID:
      return sendMessageHistoryImportAction::fetch(p);
    case sendMessageChooseStickerAction::ID:
      return sendMessageChooseStickerAction::fetch(p);
    case sendMessageEmojiInteraction::ID:
      return sendMessageEmojiInteraction::fetch(p);
    case sendMessageEmojiInteractionSeen::ID:
      return sendMessageEmojiInteractionSeen::fetch(p);
    case sendMessageTextDraftAction::ID:
      return sendMessageTextDraftAction::fetch(p);
    case shippingOption::ID:
      return shippingOption::fetch(p);
    case smsJob::ID:
      return smsJob::fetch(p);
    case sponsoredMessage::ID:
      return sponsoredMessage::fetch(p);
    case sponsoredMessageReportOption::ID:
      return sponsoredMessageReportOption::fetch(p);
    case sponsoredPeer::ID:
      return sponsoredPeer::fetch(p);
    case starGift::ID:
      return starGift::fetch(p);
    case starGiftUnique::ID:
      return starGiftUnique::fetch(p);
    case starGiftAttributeModel::ID:
      return starGiftAttributeModel::fetch(p);
    case starGiftAttributePattern::ID:
      return starGiftAttributePattern::fetch(p);
    case starGiftAttributeBackdrop::ID:
      return starGiftAttributeBackdrop::fetch(p);
    case starGiftAttributeOriginalDetails::ID:
      return starGiftAttributeOriginalDetails::fetch(p);
    case starGiftAttributeCounter::ID:
      return starGiftAttributeCounter::fetch(p);
    case starGiftAttributeIdModel::ID:
      return starGiftAttributeIdModel::fetch(p);
    case starGiftAttributeIdPattern::ID:
      return starGiftAttributeIdPattern::fetch(p);
    case starGiftAttributeIdBackdrop::ID:
      return starGiftAttributeIdBackdrop::fetch(p);
    case starGiftCollection::ID:
      return starGiftCollection::fetch(p);
    case starGiftUpgradePrice::ID:
      return starGiftUpgradePrice::fetch(p);
    case starRefProgram::ID:
      return starRefProgram::fetch(p);
    case starsAmount::ID:
      return starsAmount::fetch(p);
    case starsTonAmount::ID:
      return starsTonAmount::fetch(p);
    case starsGiftOption::ID:
      return starsGiftOption::fetch(p);
    case starsGiveawayOption::ID:
      return starsGiveawayOption::fetch(p);
    case starsGiveawayWinnersOption::ID:
      return starsGiveawayWinnersOption::fetch(p);
    case starsRating::ID:
      return starsRating::fetch(p);
    case starsRevenueStatus::ID:
      return starsRevenueStatus::fetch(p);
    case starsSubscription::ID:
      return starsSubscription::fetch(p);
    case starsSubscriptionPricing::ID:
      return starsSubscriptionPricing::fetch(p);
    case starsTopupOption::ID:
      return starsTopupOption::fetch(p);
    case starsTransaction::ID:
      return starsTransaction::fetch(p);
    case starsTransactionPeerUnsupported::ID:
      return starsTransactionPeerUnsupported::fetch(p);
    case starsTransactionPeerAppStore::ID:
      return starsTransactionPeerAppStore::fetch(p);
    case starsTransactionPeerPlayMarket::ID:
      return starsTransactionPeerPlayMarket::fetch(p);
    case starsTransactionPeerPremiumBot::ID:
      return starsTransactionPeerPremiumBot::fetch(p);
    case starsTransactionPeerFragment::ID:
      return starsTransactionPeerFragment::fetch(p);
    case starsTransactionPeer::ID:
      return starsTransactionPeer::fetch(p);
    case starsTransactionPeerAds::ID:
      return starsTransactionPeerAds::fetch(p);
    case starsTransactionPeerAPI::ID:
      return starsTransactionPeerAPI::fetch(p);
    case statsAbsValueAndPrev::ID:
      return statsAbsValueAndPrev::fetch(p);
    case statsDateRangeDays::ID:
      return statsDateRangeDays::fetch(p);
    case statsGraphAsync::ID:
      return statsGraphAsync::fetch(p);
    case statsGraphError::ID:
      return statsGraphError::fetch(p);
    case statsGraph::ID:
      return statsGraph::fetch(p);
    case statsGroupTopAdmin::ID:
      return statsGroupTopAdmin::fetch(p);
    case statsGroupTopInviter::ID:
      return statsGroupTopInviter::fetch(p);
    case statsGroupTopPoster::ID:
      return statsGroupTopPoster::fetch(p);
    case statsPercentValue::ID:
      return statsPercentValue::fetch(p);
    case stickerKeyword::ID:
      return stickerKeyword::fetch(p);
    case stickerPack::ID:
      return stickerPack::fetch(p);
    case stickerSet::ID:
      return stickerSet::fetch(p);
    case stickerSetCovered::ID:
      return stickerSetCovered::fetch(p);
    case stickerSetMultiCovered::ID:
      return stickerSetMultiCovered::fetch(p);
    case stickerSetFullCovered::ID:
      return stickerSetFullCovered::fetch(p);
    case stickerSetNoCovered::ID:
      return stickerSetNoCovered::fetch(p);
    case storiesStealthMode::ID:
      return storiesStealthMode::fetch(p);
    case storyAlbum::ID:
      return storyAlbum::fetch(p);
    case storyFwdHeader::ID:
      return storyFwdHeader::fetch(p);
    case storyItemDeleted::ID:
      return storyItemDeleted::fetch(p);
    case storyItemSkipped::ID:
      return storyItemSkipped::fetch(p);
    case storyItem::ID:
      return storyItem::fetch(p);
    case storyReaction::ID:
      return storyReaction::fetch(p);
    case storyReactionPublicForward::ID:
      return storyReactionPublicForward::fetch(p);
    case storyReactionPublicRepost::ID:
      return storyReactionPublicRepost::fetch(p);
    case storyView::ID:
      return storyView::fetch(p);
    case storyViewPublicForward::ID:
      return storyViewPublicForward::fetch(p);
    case storyViewPublicRepost::ID:
      return storyViewPublicRepost::fetch(p);
    case storyViews::ID:
      return storyViews::fetch(p);
    case suggestedPost::ID:
      return suggestedPost::fetch(p);
    case textWithEntities::ID:
      return textWithEntities::fetch(p);
    case theme::ID:
      return theme::fetch(p);
    case themeSettings::ID:
      return themeSettings::fetch(p);
    case timezone::ID:
      return timezone::fetch(p);
    case todoCompletion::ID:
      return todoCompletion::fetch(p);
    case todoItem::ID:
      return todoItem::fetch(p);
    case todoList::ID:
      return todoList::fetch(p);
    case topPeer::ID:
      return topPeer::fetch(p);
    case topPeerCategoryBotsPM::ID:
      return topPeerCategoryBotsPM::fetch(p);
    case topPeerCategoryBotsInline::ID:
      return topPeerCategoryBotsInline::fetch(p);
    case topPeerCategoryCorrespondents::ID:
      return topPeerCategoryCorrespondents::fetch(p);
    case topPeerCategoryGroups::ID:
      return topPeerCategoryGroups::fetch(p);
    case topPeerCategoryChannels::ID:
      return topPeerCategoryChannels::fetch(p);
    case topPeerCategoryPhoneCalls::ID:
      return topPeerCategoryPhoneCalls::fetch(p);
    case topPeerCategoryForwardUsers::ID:
      return topPeerCategoryForwardUsers::fetch(p);
    case topPeerCategoryForwardChats::ID:
      return topPeerCategoryForwardChats::fetch(p);
    case topPeerCategoryBotsApp::ID:
      return topPeerCategoryBotsApp::fetch(p);
    case topPeerCategoryPeers::ID:
      return topPeerCategoryPeers::fetch(p);
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
    case updatesTooLong::ID:
      return updatesTooLong::fetch(p);
    case updateShortMessage::ID:
      return updateShortMessage::fetch(p);
    case updateShortChatMessage::ID:
      return updateShortChatMessage::fetch(p);
    case updateShort::ID:
      return updateShort::fetch(p);
    case updatesCombined::ID:
      return updatesCombined::fetch(p);
    case updates::ID:
      return updates::fetch(p);
    case updateShortSentMessage::ID:
      return updateShortSentMessage::fetch(p);
    case urlAuthResultRequest::ID:
      return urlAuthResultRequest::fetch(p);
    case urlAuthResultAccepted::ID:
      return urlAuthResultAccepted::fetch(p);
    case urlAuthResultDefault::ID:
      return urlAuthResultDefault::fetch(p);
    case userEmpty::ID:
      return userEmpty::fetch(p);
    case user::ID:
      return user::fetch(p);
    case userFull::ID:
      return userFull::fetch(p);
    case userProfilePhotoEmpty::ID:
      return userProfilePhotoEmpty::fetch(p);
    case userProfilePhoto::ID:
      return userProfilePhoto::fetch(p);
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
    case username::ID:
      return username::fetch(p);
    case videoSize::ID:
      return videoSize::fetch(p);
    case videoSizeEmojiMarkup::ID:
      return videoSizeEmojiMarkup::fetch(p);
    case videoSizeStickerMarkup::ID:
      return videoSizeStickerMarkup::fetch(p);
    case wallPaper::ID:
      return wallPaper::fetch(p);
    case wallPaperNoFile::ID:
      return wallPaperNoFile::fetch(p);
    case wallPaperSettings::ID:
      return wallPaperSettings::fetch(p);
    case webAuthorization::ID:
      return webAuthorization::fetch(p);
    case webDocument::ID:
      return webDocument::fetch(p);
    case webDocumentNoProxy::ID:
      return webDocumentNoProxy::fetch(p);
    case webPageEmpty::ID:
      return webPageEmpty::fetch(p);
    case webPagePending::ID:
      return webPagePending::fetch(p);
    case webPage::ID:
      return webPage::fetch(p);
    case webPageNotModified::ID:
      return webPageNotModified::fetch(p);
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
    case webViewMessageSent::ID:
      return webViewMessageSent::fetch(p);
    case webViewResultUrl::ID:
      return webViewResultUrl::fetch(p);
    case account_authorizationForm::ID:
      return account_authorizationForm::fetch(p);
    case account_authorizations::ID:
      return account_authorizations::fetch(p);
    case account_autoDownloadSettings::ID:
      return account_autoDownloadSettings::fetch(p);
    case account_autoSaveSettings::ID:
      return account_autoSaveSettings::fetch(p);
    case account_businessChatLinks::ID:
      return account_businessChatLinks::fetch(p);
    case account_chatThemesNotModified::ID:
      return account_chatThemesNotModified::fetch(p);
    case account_chatThemes::ID:
      return account_chatThemes::fetch(p);
    case account_connectedBots::ID:
      return account_connectedBots::fetch(p);
    case account_contentSettings::ID:
      return account_contentSettings::fetch(p);
    case account_emailVerified::ID:
      return account_emailVerified::fetch(p);
    case account_emailVerifiedLogin::ID:
      return account_emailVerifiedLogin::fetch(p);
    case account_emojiStatusesNotModified::ID:
      return account_emojiStatusesNotModified::fetch(p);
    case account_emojiStatuses::ID:
      return account_emojiStatuses::fetch(p);
    case account_paidMessagesRevenue::ID:
      return account_paidMessagesRevenue::fetch(p);
    case account_password::ID:
      return account_password::fetch(p);
    case account_passwordSettings::ID:
      return account_passwordSettings::fetch(p);
    case account_privacyRules::ID:
      return account_privacyRules::fetch(p);
    case account_resetPasswordFailedWait::ID:
      return account_resetPasswordFailedWait::fetch(p);
    case account_resetPasswordRequestedWait::ID:
      return account_resetPasswordRequestedWait::fetch(p);
    case account_resetPasswordOk::ID:
      return account_resetPasswordOk::fetch(p);
    case account_resolvedBusinessChatLinks::ID:
      return account_resolvedBusinessChatLinks::fetch(p);
    case account_savedMusicIdsNotModified::ID:
      return account_savedMusicIdsNotModified::fetch(p);
    case account_savedMusicIds::ID:
      return account_savedMusicIds::fetch(p);
    case account_savedRingtone::ID:
      return account_savedRingtone::fetch(p);
    case account_savedRingtoneConverted::ID:
      return account_savedRingtoneConverted::fetch(p);
    case account_savedRingtonesNotModified::ID:
      return account_savedRingtonesNotModified::fetch(p);
    case account_savedRingtones::ID:
      return account_savedRingtones::fetch(p);
    case account_sentEmailCode::ID:
      return account_sentEmailCode::fetch(p);
    case account_takeout::ID:
      return account_takeout::fetch(p);
    case account_themesNotModified::ID:
      return account_themesNotModified::fetch(p);
    case account_themes::ID:
      return account_themes::fetch(p);
    case account_tmpPassword::ID:
      return account_tmpPassword::fetch(p);
    case account_wallPapersNotModified::ID:
      return account_wallPapersNotModified::fetch(p);
    case account_wallPapers::ID:
      return account_wallPapers::fetch(p);
    case account_webAuthorizations::ID:
      return account_webAuthorizations::fetch(p);
    case auth_authorization::ID:
      return auth_authorization::fetch(p);
    case auth_authorizationSignUpRequired::ID:
      return auth_authorizationSignUpRequired::fetch(p);
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
    case auth_exportedAuthorization::ID:
      return auth_exportedAuthorization::fetch(p);
    case auth_loggedOut::ID:
      return auth_loggedOut::fetch(p);
    case auth_loginToken::ID:
      return auth_loginToken::fetch(p);
    case auth_loginTokenMigrateTo::ID:
      return auth_loginTokenMigrateTo::fetch(p);
    case auth_loginTokenSuccess::ID:
      return auth_loginTokenSuccess::fetch(p);
    case auth_passwordRecovery::ID:
      return auth_passwordRecovery::fetch(p);
    case auth_sentCode::ID:
      return auth_sentCode::fetch(p);
    case auth_sentCodeSuccess::ID:
      return auth_sentCodeSuccess::fetch(p);
    case auth_sentCodePaymentRequired::ID:
      return auth_sentCodePaymentRequired::fetch(p);
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
    case bots_botInfo::ID:
      return bots_botInfo::fetch(p);
    case bots_popularAppBots::ID:
      return bots_popularAppBots::fetch(p);
    case bots_previewInfo::ID:
      return bots_previewInfo::fetch(p);
    case channels_adminLogResults::ID:
      return channels_adminLogResults::fetch(p);
    case channels_channelParticipant::ID:
      return channels_channelParticipant::fetch(p);
    case channels_channelParticipants::ID:
      return channels_channelParticipants::fetch(p);
    case channels_channelParticipantsNotModified::ID:
      return channels_channelParticipantsNotModified::fetch(p);
    case channels_sendAsPeers::ID:
      return channels_sendAsPeers::fetch(p);
    case channels_sponsoredMessageReportResultChooseOption::ID:
      return channels_sponsoredMessageReportResultChooseOption::fetch(p);
    case channels_sponsoredMessageReportResultAdsHidden::ID:
      return channels_sponsoredMessageReportResultAdsHidden::fetch(p);
    case channels_sponsoredMessageReportResultReported::ID:
      return channels_sponsoredMessageReportResultReported::fetch(p);
    case chatlists_chatlistInviteAlready::ID:
      return chatlists_chatlistInviteAlready::fetch(p);
    case chatlists_chatlistInvite::ID:
      return chatlists_chatlistInvite::fetch(p);
    case chatlists_chatlistUpdates::ID:
      return chatlists_chatlistUpdates::fetch(p);
    case chatlists_exportedChatlistInvite::ID:
      return chatlists_exportedChatlistInvite::fetch(p);
    case chatlists_exportedInvites::ID:
      return chatlists_exportedInvites::fetch(p);
    case contacts_blocked::ID:
      return contacts_blocked::fetch(p);
    case contacts_blockedSlice::ID:
      return contacts_blockedSlice::fetch(p);
    case contacts_contactBirthdays::ID:
      return contacts_contactBirthdays::fetch(p);
    case contacts_contactsNotModified::ID:
      return contacts_contactsNotModified::fetch(p);
    case contacts_contacts::ID:
      return contacts_contacts::fetch(p);
    case contacts_found::ID:
      return contacts_found::fetch(p);
    case contacts_importedContacts::ID:
      return contacts_importedContacts::fetch(p);
    case contacts_resolvedPeer::ID:
      return contacts_resolvedPeer::fetch(p);
    case contacts_sponsoredPeersEmpty::ID:
      return contacts_sponsoredPeersEmpty::fetch(p);
    case contacts_sponsoredPeers::ID:
      return contacts_sponsoredPeers::fetch(p);
    case contacts_topPeersNotModified::ID:
      return contacts_topPeersNotModified::fetch(p);
    case contacts_topPeers::ID:
      return contacts_topPeers::fetch(p);
    case contacts_topPeersDisabled::ID:
      return contacts_topPeersDisabled::fetch(p);
    case fragment_collectibleInfo::ID:
      return fragment_collectibleInfo::fetch(p);
    case help_appConfigNotModified::ID:
      return help_appConfigNotModified::fetch(p);
    case help_appConfig::ID:
      return help_appConfig::fetch(p);
    case help_appUpdate::ID:
      return help_appUpdate::fetch(p);
    case help_noAppUpdate::ID:
      return help_noAppUpdate::fetch(p);
    case help_configSimple::ID:
      return help_configSimple::fetch(p);
    case help_countriesListNotModified::ID:
      return help_countriesListNotModified::fetch(p);
    case help_countriesList::ID:
      return help_countriesList::fetch(p);
    case help_country::ID:
      return help_country::fetch(p);
    case help_countryCode::ID:
      return help_countryCode::fetch(p);
    case help_deepLinkInfoEmpty::ID:
      return help_deepLinkInfoEmpty::fetch(p);
    case help_deepLinkInfo::ID:
      return help_deepLinkInfo::fetch(p);
    case help_inviteText::ID:
      return help_inviteText::fetch(p);
    case help_passportConfigNotModified::ID:
      return help_passportConfigNotModified::fetch(p);
    case help_passportConfig::ID:
      return help_passportConfig::fetch(p);
    case help_peerColorOption::ID:
      return help_peerColorOption::fetch(p);
    case help_peerColorSet::ID:
      return help_peerColorSet::fetch(p);
    case help_peerColorProfileSet::ID:
      return help_peerColorProfileSet::fetch(p);
    case help_peerColorsNotModified::ID:
      return help_peerColorsNotModified::fetch(p);
    case help_peerColors::ID:
      return help_peerColors::fetch(p);
    case help_premiumPromo::ID:
      return help_premiumPromo::fetch(p);
    case help_promoDataEmpty::ID:
      return help_promoDataEmpty::fetch(p);
    case help_promoData::ID:
      return help_promoData::fetch(p);
    case help_recentMeUrls::ID:
      return help_recentMeUrls::fetch(p);
    case help_support::ID:
      return help_support::fetch(p);
    case help_supportName::ID:
      return help_supportName::fetch(p);
    case help_termsOfService::ID:
      return help_termsOfService::fetch(p);
    case help_termsOfServiceUpdateEmpty::ID:
      return help_termsOfServiceUpdateEmpty::fetch(p);
    case help_termsOfServiceUpdate::ID:
      return help_termsOfServiceUpdate::fetch(p);
    case help_timezonesListNotModified::ID:
      return help_timezonesListNotModified::fetch(p);
    case help_timezonesList::ID:
      return help_timezonesList::fetch(p);
    case help_userInfoEmpty::ID:
      return help_userInfoEmpty::fetch(p);
    case help_userInfo::ID:
      return help_userInfo::fetch(p);
    case messages_affectedFoundMessages::ID:
      return messages_affectedFoundMessages::fetch(p);
    case messages_affectedHistory::ID:
      return messages_affectedHistory::fetch(p);
    case messages_affectedMessages::ID:
      return messages_affectedMessages::fetch(p);
    case messages_allStickersNotModified::ID:
      return messages_allStickersNotModified::fetch(p);
    case messages_allStickers::ID:
      return messages_allStickers::fetch(p);
    case messages_archivedStickers::ID:
      return messages_archivedStickers::fetch(p);
    case messages_availableEffectsNotModified::ID:
      return messages_availableEffectsNotModified::fetch(p);
    case messages_availableEffects::ID:
      return messages_availableEffects::fetch(p);
    case messages_availableReactionsNotModified::ID:
      return messages_availableReactionsNotModified::fetch(p);
    case messages_availableReactions::ID:
      return messages_availableReactions::fetch(p);
    case messages_botApp::ID:
      return messages_botApp::fetch(p);
    case messages_botCallbackAnswer::ID:
      return messages_botCallbackAnswer::fetch(p);
    case messages_botPreparedInlineMessage::ID:
      return messages_botPreparedInlineMessage::fetch(p);
    case messages_botResults::ID:
      return messages_botResults::fetch(p);
    case messages_chatAdminsWithInvites::ID:
      return messages_chatAdminsWithInvites::fetch(p);
    case messages_chatFull::ID:
      return messages_chatFull::fetch(p);
    case messages_chatInviteImporters::ID:
      return messages_chatInviteImporters::fetch(p);
    case messages_chats::ID:
      return messages_chats::fetch(p);
    case messages_chatsSlice::ID:
      return messages_chatsSlice::fetch(p);
    case messages_checkedHistoryImportPeer::ID:
      return messages_checkedHistoryImportPeer::fetch(p);
    case messages_dhConfigNotModified::ID:
      return messages_dhConfigNotModified::fetch(p);
    case messages_dhConfig::ID:
      return messages_dhConfig::fetch(p);
    case messages_dialogFilters::ID:
      return messages_dialogFilters::fetch(p);
    case messages_dialogs::ID:
      return messages_dialogs::fetch(p);
    case messages_dialogsSlice::ID:
      return messages_dialogsSlice::fetch(p);
    case messages_dialogsNotModified::ID:
      return messages_dialogsNotModified::fetch(p);
    case messages_discussionMessage::ID:
      return messages_discussionMessage::fetch(p);
    case messages_emojiGroupsNotModified::ID:
      return messages_emojiGroupsNotModified::fetch(p);
    case messages_emojiGroups::ID:
      return messages_emojiGroups::fetch(p);
    case messages_exportedChatInvite::ID:
      return messages_exportedChatInvite::fetch(p);
    case messages_exportedChatInviteReplaced::ID:
      return messages_exportedChatInviteReplaced::fetch(p);
    case messages_exportedChatInvites::ID:
      return messages_exportedChatInvites::fetch(p);
    case messages_favedStickersNotModified::ID:
      return messages_favedStickersNotModified::fetch(p);
    case messages_favedStickers::ID:
      return messages_favedStickers::fetch(p);
    case messages_featuredStickersNotModified::ID:
      return messages_featuredStickersNotModified::fetch(p);
    case messages_featuredStickers::ID:
      return messages_featuredStickers::fetch(p);
    case messages_forumTopics::ID:
      return messages_forumTopics::fetch(p);
    case messages_foundStickerSetsNotModified::ID:
      return messages_foundStickerSetsNotModified::fetch(p);
    case messages_foundStickerSets::ID:
      return messages_foundStickerSets::fetch(p);
    case messages_foundStickersNotModified::ID:
      return messages_foundStickersNotModified::fetch(p);
    case messages_foundStickers::ID:
      return messages_foundStickers::fetch(p);
    case messages_highScores::ID:
      return messages_highScores::fetch(p);
    case messages_historyImport::ID:
      return messages_historyImport::fetch(p);
    case messages_historyImportParsed::ID:
      return messages_historyImportParsed::fetch(p);
    case messages_inactiveChats::ID:
      return messages_inactiveChats::fetch(p);
    case messages_invitedUsers::ID:
      return messages_invitedUsers::fetch(p);
    case messages_messageEditData::ID:
      return messages_messageEditData::fetch(p);
    case messages_messageReactionsList::ID:
      return messages_messageReactionsList::fetch(p);
    case messages_messageViews::ID:
      return messages_messageViews::fetch(p);
    case messages_messages::ID:
      return messages_messages::fetch(p);
    case messages_messagesSlice::ID:
      return messages_messagesSlice::fetch(p);
    case messages_channelMessages::ID:
      return messages_channelMessages::fetch(p);
    case messages_messagesNotModified::ID:
      return messages_messagesNotModified::fetch(p);
    case messages_myStickers::ID:
      return messages_myStickers::fetch(p);
    case messages_peerDialogs::ID:
      return messages_peerDialogs::fetch(p);
    case messages_peerSettings::ID:
      return messages_peerSettings::fetch(p);
    case messages_preparedInlineMessage::ID:
      return messages_preparedInlineMessage::fetch(p);
    case messages_quickReplies::ID:
      return messages_quickReplies::fetch(p);
    case messages_quickRepliesNotModified::ID:
      return messages_quickRepliesNotModified::fetch(p);
    case messages_reactionsNotModified::ID:
      return messages_reactionsNotModified::fetch(p);
    case messages_reactions::ID:
      return messages_reactions::fetch(p);
    case messages_recentStickersNotModified::ID:
      return messages_recentStickersNotModified::fetch(p);
    case messages_recentStickers::ID:
      return messages_recentStickers::fetch(p);
    case messages_savedDialogs::ID:
      return messages_savedDialogs::fetch(p);
    case messages_savedDialogsSlice::ID:
      return messages_savedDialogsSlice::fetch(p);
    case messages_savedDialogsNotModified::ID:
      return messages_savedDialogsNotModified::fetch(p);
    case messages_savedGifsNotModified::ID:
      return messages_savedGifsNotModified::fetch(p);
    case messages_savedGifs::ID:
      return messages_savedGifs::fetch(p);
    case messages_savedReactionTagsNotModified::ID:
      return messages_savedReactionTagsNotModified::fetch(p);
    case messages_savedReactionTags::ID:
      return messages_savedReactionTags::fetch(p);
    case messages_searchCounter::ID:
      return messages_searchCounter::fetch(p);
    case messages_searchResultsCalendar::ID:
      return messages_searchResultsCalendar::fetch(p);
    case messages_searchResultsPositions::ID:
      return messages_searchResultsPositions::fetch(p);
    case messages_sentEncryptedMessage::ID:
      return messages_sentEncryptedMessage::fetch(p);
    case messages_sentEncryptedFile::ID:
      return messages_sentEncryptedFile::fetch(p);
    case messages_sponsoredMessages::ID:
      return messages_sponsoredMessages::fetch(p);
    case messages_sponsoredMessagesEmpty::ID:
      return messages_sponsoredMessagesEmpty::fetch(p);
    case messages_stickerSet::ID:
      return messages_stickerSet::fetch(p);
    case messages_stickerSetNotModified::ID:
      return messages_stickerSetNotModified::fetch(p);
    case messages_stickerSetInstallResultSuccess::ID:
      return messages_stickerSetInstallResultSuccess::fetch(p);
    case messages_stickerSetInstallResultArchive::ID:
      return messages_stickerSetInstallResultArchive::fetch(p);
    case messages_stickersNotModified::ID:
      return messages_stickersNotModified::fetch(p);
    case messages_stickers::ID:
      return messages_stickers::fetch(p);
    case messages_transcribedAudio::ID:
      return messages_transcribedAudio::fetch(p);
    case messages_translateResult::ID:
      return messages_translateResult::fetch(p);
    case messages_votesList::ID:
      return messages_votesList::fetch(p);
    case messages_webPage::ID:
      return messages_webPage::fetch(p);
    case messages_webPagePreview::ID:
      return messages_webPagePreview::fetch(p);
    case payments_bankCardData::ID:
      return payments_bankCardData::fetch(p);
    case payments_checkCanSendGiftResultOk::ID:
      return payments_checkCanSendGiftResultOk::fetch(p);
    case payments_checkCanSendGiftResultFail::ID:
      return payments_checkCanSendGiftResultFail::fetch(p);
    case payments_checkedGiftCode::ID:
      return payments_checkedGiftCode::fetch(p);
    case payments_connectedStarRefBots::ID:
      return payments_connectedStarRefBots::fetch(p);
    case payments_exportedInvoice::ID:
      return payments_exportedInvoice::fetch(p);
    case payments_giveawayInfo::ID:
      return payments_giveawayInfo::fetch(p);
    case payments_giveawayInfoResults::ID:
      return payments_giveawayInfoResults::fetch(p);
    case payments_paymentForm::ID:
      return payments_paymentForm::fetch(p);
    case payments_paymentFormStars::ID:
      return payments_paymentFormStars::fetch(p);
    case payments_paymentFormStarGift::ID:
      return payments_paymentFormStarGift::fetch(p);
    case payments_paymentReceipt::ID:
      return payments_paymentReceipt::fetch(p);
    case payments_paymentReceiptStars::ID:
      return payments_paymentReceiptStars::fetch(p);
    case payments_paymentResult::ID:
      return payments_paymentResult::fetch(p);
    case payments_paymentVerificationNeeded::ID:
      return payments_paymentVerificationNeeded::fetch(p);
    case payments_resaleStarGifts::ID:
      return payments_resaleStarGifts::fetch(p);
    case payments_savedInfo::ID:
      return payments_savedInfo::fetch(p);
    case payments_savedStarGifts::ID:
      return payments_savedStarGifts::fetch(p);
    case payments_starGiftCollectionsNotModified::ID:
      return payments_starGiftCollectionsNotModified::fetch(p);
    case payments_starGiftCollections::ID:
      return payments_starGiftCollections::fetch(p);
    case payments_starGiftUpgradePreview::ID:
      return payments_starGiftUpgradePreview::fetch(p);
    case payments_starGiftWithdrawalUrl::ID:
      return payments_starGiftWithdrawalUrl::fetch(p);
    case payments_starGiftsNotModified::ID:
      return payments_starGiftsNotModified::fetch(p);
    case payments_starGifts::ID:
      return payments_starGifts::fetch(p);
    case payments_starsRevenueAdsAccountUrl::ID:
      return payments_starsRevenueAdsAccountUrl::fetch(p);
    case payments_starsRevenueStats::ID:
      return payments_starsRevenueStats::fetch(p);
    case payments_starsRevenueWithdrawalUrl::ID:
      return payments_starsRevenueWithdrawalUrl::fetch(p);
    case payments_starsStatus::ID:
      return payments_starsStatus::fetch(p);
    case payments_suggestedStarRefBots::ID:
      return payments_suggestedStarRefBots::fetch(p);
    case payments_uniqueStarGift::ID:
      return payments_uniqueStarGift::fetch(p);
    case payments_uniqueStarGiftValueInfo::ID:
      return payments_uniqueStarGiftValueInfo::fetch(p);
    case payments_validatedRequestedInfo::ID:
      return payments_validatedRequestedInfo::fetch(p);
    case phone_exportedGroupCallInvite::ID:
      return phone_exportedGroupCallInvite::fetch(p);
    case phone_groupCall::ID:
      return phone_groupCall::fetch(p);
    case phone_groupCallStreamChannels::ID:
      return phone_groupCallStreamChannels::fetch(p);
    case phone_groupCallStreamRtmpUrl::ID:
      return phone_groupCallStreamRtmpUrl::fetch(p);
    case phone_groupParticipants::ID:
      return phone_groupParticipants::fetch(p);
    case phone_joinAsPeers::ID:
      return phone_joinAsPeers::fetch(p);
    case phone_phoneCall::ID:
      return phone_phoneCall::fetch(p);
    case photos_photo::ID:
      return photos_photo::fetch(p);
    case photos_photos::ID:
      return photos_photos::fetch(p);
    case photos_photosSlice::ID:
      return photos_photosSlice::fetch(p);
    case premium_boostsList::ID:
      return premium_boostsList::fetch(p);
    case premium_boostsStatus::ID:
      return premium_boostsStatus::fetch(p);
    case premium_myBoosts::ID:
      return premium_myBoosts::fetch(p);
    case smsjobs_eligibleToJoin::ID:
      return smsjobs_eligibleToJoin::fetch(p);
    case smsjobs_status::ID:
      return smsjobs_status::fetch(p);
    case stats_broadcastStats::ID:
      return stats_broadcastStats::fetch(p);
    case stats_megagroupStats::ID:
      return stats_megagroupStats::fetch(p);
    case stats_messageStats::ID:
      return stats_messageStats::fetch(p);
    case stats_publicForwards::ID:
      return stats_publicForwards::fetch(p);
    case stats_storyStats::ID:
      return stats_storyStats::fetch(p);
    case stickers_suggestedShortName::ID:
      return stickers_suggestedShortName::fetch(p);
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
    case stories_albumsNotModified::ID:
      return stories_albumsNotModified::fetch(p);
    case stories_albums::ID:
      return stories_albums::fetch(p);
    case stories_allStoriesNotModified::ID:
      return stories_allStoriesNotModified::fetch(p);
    case stories_allStories::ID:
      return stories_allStories::fetch(p);
    case stories_canSendStoryCount::ID:
      return stories_canSendStoryCount::fetch(p);
    case stories_foundStories::ID:
      return stories_foundStories::fetch(p);
    case stories_peerStories::ID:
      return stories_peerStories::fetch(p);
    case stories_stories::ID:
      return stories_stories::fetch(p);
    case stories_storyReactionsList::ID:
      return stories_storyReactionsList::fetch(p);
    case stories_storyViews::ID:
      return stories_storyViews::fetch(p);
    case stories_storyViewsList::ID:
      return stories_storyViewsList::fetch(p);
    case updates_channelDifferenceEmpty::ID:
      return updates_channelDifferenceEmpty::fetch(p);
    case updates_channelDifferenceTooLong::ID:
      return updates_channelDifferenceTooLong::fetch(p);
    case updates_channelDifference::ID:
      return updates_channelDifference::fetch(p);
    case updates_differenceEmpty::ID:
      return updates_differenceEmpty::fetch(p);
    case updates_difference::ID:
      return updates_difference::fetch(p);
    case updates_differenceSlice::ID:
      return updates_differenceSlice::fetch(p);
    case updates_differenceTooLong::ID:
      return updates_differenceTooLong::fetch(p);
    case updates_state::ID:
      return updates_state::fetch(p);
    case upload_cdnFileReuploadNeeded::ID:
      return upload_cdnFileReuploadNeeded::fetch(p);
    case upload_cdnFile::ID:
      return upload_cdnFile::fetch(p);
    case upload_file::ID:
      return upload_file::fetch(p);
    case upload_fileCdnRedirect::ID:
      return upload_fileCdnRedirect::fetch(p);
    case upload_webFile::ID:
      return upload_webFile::fetch(p);
    case users_savedMusicNotModified::ID:
      return users_savedMusicNotModified::fetch(p);
    case users_savedMusic::ID:
      return users_savedMusic::fetch(p);
    case users_userFull::ID:
      return users_userFull::fetch(p);
    case users_users::ID:
      return users_users::fetch(p);
    case users_usersSlice::ID:
      return users_usersSlice::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
