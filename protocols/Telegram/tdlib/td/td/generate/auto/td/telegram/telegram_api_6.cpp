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


birthday::birthday()
  : flags_()
  , day_()
  , month_()
  , year_()
{}

birthday::birthday(int32 flags_, int32 day_, int32 month_, int32 year_)
  : flags_(flags_)
  , day_(day_)
  , month_(month_)
  , year_(year_)
{}

const std::int32_t birthday::ID;

object_ptr<birthday> birthday::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<birthday> res = make_tl_object<birthday>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->day_ = TlFetchInt::parse(p);
  res->month_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->year_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void birthday::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(day_, s);
  TlStoreBinary::store(month_, s);
  if (var0 & 1) { TlStoreBinary::store(year_, s); }
}

void birthday::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(day_, s);
  TlStoreBinary::store(month_, s);
  if (var0 & 1) { TlStoreBinary::store(year_, s); }
}

void birthday::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "birthday");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("day", day_);
    s.store_field("month", month_);
    if (var0 & 1) { s.store_field("year", year_); }
    s.store_class_end();
  }
}

object_ptr<BotInlineMessage> BotInlineMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

botInlineMessageMediaAuto::botInlineMessageMediaAuto()
  : flags_()
  , invert_media_()
  , message_()
  , entities_()
  , reply_markup_()
{}

const std::int32_t botInlineMessageMediaAuto::ID;

object_ptr<BotInlineMessage> botInlineMessageMediaAuto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMessageMediaAuto> res = make_tl_object<botInlineMessageMediaAuto>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->invert_media_ = (var0 & 8) != 0;
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 4) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMessageMediaAuto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMessageMediaAuto");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (invert_media_ << 3)));
    if (var0 & 8) { s.store_field("invert_media", true); }
    s.store_field("message", message_);
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

botInlineMessageText::botInlineMessageText()
  : flags_()
  , no_webpage_()
  , invert_media_()
  , message_()
  , entities_()
  , reply_markup_()
{}

const std::int32_t botInlineMessageText::ID;

object_ptr<BotInlineMessage> botInlineMessageText::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMessageText> res = make_tl_object<botInlineMessageText>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->no_webpage_ = (var0 & 1) != 0;
  res->invert_media_ = (var0 & 8) != 0;
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 4) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMessageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMessageText");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (no_webpage_ << 0) | (invert_media_ << 3)));
    if (var0 & 1) { s.store_field("no_webpage", true); }
    if (var0 & 8) { s.store_field("invert_media", true); }
    s.store_field("message", message_);
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

botInlineMessageMediaGeo::botInlineMessageMediaGeo()
  : flags_()
  , geo_()
  , heading_()
  , period_()
  , proximity_notification_radius_()
  , reply_markup_()
{}

const std::int32_t botInlineMessageMediaGeo::ID;

object_ptr<BotInlineMessage> botInlineMessageMediaGeo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMessageMediaGeo> res = make_tl_object<botInlineMessageMediaGeo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->geo_ = TlFetchObject<GeoPoint>::parse(p);
  if (var0 & 1) { res->heading_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->period_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->proximity_notification_radius_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMessageMediaGeo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMessageMediaGeo");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    if (var0 & 1) { s.store_field("heading", heading_); }
    if (var0 & 2) { s.store_field("period", period_); }
    if (var0 & 8) { s.store_field("proximity_notification_radius", proximity_notification_radius_); }
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

botInlineMessageMediaVenue::botInlineMessageMediaVenue()
  : flags_()
  , geo_()
  , title_()
  , address_()
  , provider_()
  , venue_id_()
  , venue_type_()
  , reply_markup_()
{}

const std::int32_t botInlineMessageMediaVenue::ID;

object_ptr<BotInlineMessage> botInlineMessageMediaVenue::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMessageMediaVenue> res = make_tl_object<botInlineMessageMediaVenue>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->geo_ = TlFetchObject<GeoPoint>::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->address_ = TlFetchString<string>::parse(p);
  res->provider_ = TlFetchString<string>::parse(p);
  res->venue_id_ = TlFetchString<string>::parse(p);
  res->venue_type_ = TlFetchString<string>::parse(p);
  if (var0 & 4) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMessageMediaVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMessageMediaVenue");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    s.store_field("title", title_);
    s.store_field("address", address_);
    s.store_field("provider", provider_);
    s.store_field("venue_id", venue_id_);
    s.store_field("venue_type", venue_type_);
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

botInlineMessageMediaContact::botInlineMessageMediaContact()
  : flags_()
  , phone_number_()
  , first_name_()
  , last_name_()
  , vcard_()
  , reply_markup_()
{}

const std::int32_t botInlineMessageMediaContact::ID;

object_ptr<BotInlineMessage> botInlineMessageMediaContact::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMessageMediaContact> res = make_tl_object<botInlineMessageMediaContact>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->phone_number_ = TlFetchString<string>::parse(p);
  res->first_name_ = TlFetchString<string>::parse(p);
  res->last_name_ = TlFetchString<string>::parse(p);
  res->vcard_ = TlFetchString<string>::parse(p);
  if (var0 & 4) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMessageMediaContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMessageMediaContact");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("phone_number", phone_number_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("vcard", vcard_);
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

botInlineMessageMediaInvoice::botInlineMessageMediaInvoice()
  : flags_()
  , shipping_address_requested_()
  , test_()
  , title_()
  , description_()
  , photo_()
  , currency_()
  , total_amount_()
  , reply_markup_()
{}

const std::int32_t botInlineMessageMediaInvoice::ID;

object_ptr<BotInlineMessage> botInlineMessageMediaInvoice::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMessageMediaInvoice> res = make_tl_object<botInlineMessageMediaInvoice>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->shipping_address_requested_ = (var0 & 2) != 0;
  res->test_ = (var0 & 8) != 0;
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  if (var0 & 4) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMessageMediaInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMessageMediaInvoice");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (shipping_address_requested_ << 1) | (test_ << 3)));
    if (var0 & 2) { s.store_field("shipping_address_requested", true); }
    if (var0 & 8) { s.store_field("test", true); }
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 1) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

botInlineMessageMediaWebPage::botInlineMessageMediaWebPage()
  : flags_()
  , invert_media_()
  , force_large_media_()
  , force_small_media_()
  , manual_()
  , safe_()
  , message_()
  , entities_()
  , url_()
  , reply_markup_()
{}

const std::int32_t botInlineMessageMediaWebPage::ID;

object_ptr<BotInlineMessage> botInlineMessageMediaWebPage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botInlineMessageMediaWebPage> res = make_tl_object<botInlineMessageMediaWebPage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->invert_media_ = (var0 & 8) != 0;
  res->force_large_media_ = (var0 & 16) != 0;
  res->force_small_media_ = (var0 & 32) != 0;
  res->manual_ = (var0 & 128) != 0;
  res->safe_ = (var0 & 256) != 0;
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  res->url_ = TlFetchString<string>::parse(p);
  if (var0 & 4) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botInlineMessageMediaWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInlineMessageMediaWebPage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (invert_media_ << 3) | (force_large_media_ << 4) | (force_small_media_ << 5) | (manual_ << 7) | (safe_ << 8)));
    if (var0 & 8) { s.store_field("invert_media", true); }
    if (var0 & 16) { s.store_field("force_large_media", true); }
    if (var0 & 32) { s.store_field("force_small_media", true); }
    if (var0 & 128) { s.store_field("manual", true); }
    if (var0 & 256) { s.store_field("safe", true); }
    s.store_field("message", message_);
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("url", url_);
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

businessBotRecipients::businessBotRecipients()
  : flags_()
  , existing_chats_()
  , new_chats_()
  , contacts_()
  , non_contacts_()
  , exclude_selected_()
  , users_()
  , exclude_users_()
{}

const std::int32_t businessBotRecipients::ID;

object_ptr<businessBotRecipients> businessBotRecipients::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessBotRecipients> res = make_tl_object<businessBotRecipients>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->existing_chats_ = (var0 & 1) != 0;
  res->new_chats_ = (var0 & 2) != 0;
  res->contacts_ = (var0 & 4) != 0;
  res->non_contacts_ = (var0 & 8) != 0;
  res->exclude_selected_ = (var0 & 32) != 0;
  if (var0 & 16) { res->users_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p); }
  if (var0 & 64) { res->exclude_users_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessBotRecipients::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessBotRecipients");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)));
    if (var0 & 1) { s.store_field("existing_chats", true); }
    if (var0 & 2) { s.store_field("new_chats", true); }
    if (var0 & 4) { s.store_field("contacts", true); }
    if (var0 & 8) { s.store_field("non_contacts", true); }
    if (var0 & 32) { s.store_field("exclude_selected", true); }
    if (var0 & 16) { { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 64) { { s.store_vector_begin("exclude_users", exclude_users_.size()); for (const auto &_value : exclude_users_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

businessLocation::businessLocation()
  : flags_()
  , geo_point_()
  , address_()
{}

const std::int32_t businessLocation::ID;

object_ptr<businessLocation> businessLocation::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessLocation> res = make_tl_object<businessLocation>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->geo_point_ = TlFetchObject<GeoPoint>::parse(p); }
  res->address_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessLocation");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get())); }
    s.store_field("address", address_);
    s.store_class_end();
  }
}

chatBannedRights::chatBannedRights()
  : flags_()
  , view_messages_()
  , send_messages_()
  , send_media_()
  , send_stickers_()
  , send_gifs_()
  , send_games_()
  , send_inline_()
  , embed_links_()
  , send_polls_()
  , change_info_()
  , invite_users_()
  , pin_messages_()
  , manage_topics_()
  , send_photos_()
  , send_videos_()
  , send_roundvideos_()
  , send_audios_()
  , send_voices_()
  , send_docs_()
  , send_plain_()
  , until_date_()
{}

chatBannedRights::chatBannedRights(int32 flags_, bool view_messages_, bool send_messages_, bool send_media_, bool send_stickers_, bool send_gifs_, bool send_games_, bool send_inline_, bool embed_links_, bool send_polls_, bool change_info_, bool invite_users_, bool pin_messages_, bool manage_topics_, bool send_photos_, bool send_videos_, bool send_roundvideos_, bool send_audios_, bool send_voices_, bool send_docs_, bool send_plain_, int32 until_date_)
  : flags_(flags_)
  , view_messages_(view_messages_)
  , send_messages_(send_messages_)
  , send_media_(send_media_)
  , send_stickers_(send_stickers_)
  , send_gifs_(send_gifs_)
  , send_games_(send_games_)
  , send_inline_(send_inline_)
  , embed_links_(embed_links_)
  , send_polls_(send_polls_)
  , change_info_(change_info_)
  , invite_users_(invite_users_)
  , pin_messages_(pin_messages_)
  , manage_topics_(manage_topics_)
  , send_photos_(send_photos_)
  , send_videos_(send_videos_)
  , send_roundvideos_(send_roundvideos_)
  , send_audios_(send_audios_)
  , send_voices_(send_voices_)
  , send_docs_(send_docs_)
  , send_plain_(send_plain_)
  , until_date_(until_date_)
{}

const std::int32_t chatBannedRights::ID;

object_ptr<chatBannedRights> chatBannedRights::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatBannedRights> res = make_tl_object<chatBannedRights>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->view_messages_ = (var0 & 1) != 0;
  res->send_messages_ = (var0 & 2) != 0;
  res->send_media_ = (var0 & 4) != 0;
  res->send_stickers_ = (var0 & 8) != 0;
  res->send_gifs_ = (var0 & 16) != 0;
  res->send_games_ = (var0 & 32) != 0;
  res->send_inline_ = (var0 & 64) != 0;
  res->embed_links_ = (var0 & 128) != 0;
  res->send_polls_ = (var0 & 256) != 0;
  res->change_info_ = (var0 & 1024) != 0;
  res->invite_users_ = (var0 & 32768) != 0;
  res->pin_messages_ = (var0 & 131072) != 0;
  res->manage_topics_ = (var0 & 262144) != 0;
  res->send_photos_ = (var0 & 524288) != 0;
  res->send_videos_ = (var0 & 1048576) != 0;
  res->send_roundvideos_ = (var0 & 2097152) != 0;
  res->send_audios_ = (var0 & 4194304) != 0;
  res->send_voices_ = (var0 & 8388608) != 0;
  res->send_docs_ = (var0 & 16777216) != 0;
  res->send_plain_ = (var0 & 33554432) != 0;
  res->until_date_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void chatBannedRights::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (view_messages_ << 0) | (send_messages_ << 1) | (send_media_ << 2) | (send_stickers_ << 3) | (send_gifs_ << 4) | (send_games_ << 5) | (send_inline_ << 6) | (embed_links_ << 7) | (send_polls_ << 8) | (change_info_ << 10) | (invite_users_ << 15) | (pin_messages_ << 17) | (manage_topics_ << 18) | (send_photos_ << 19) | (send_videos_ << 20) | (send_roundvideos_ << 21) | (send_audios_ << 22) | (send_voices_ << 23) | (send_docs_ << 24) | (send_plain_ << 25)), s);
  TlStoreBinary::store(until_date_, s);
}

void chatBannedRights::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (view_messages_ << 0) | (send_messages_ << 1) | (send_media_ << 2) | (send_stickers_ << 3) | (send_gifs_ << 4) | (send_games_ << 5) | (send_inline_ << 6) | (embed_links_ << 7) | (send_polls_ << 8) | (change_info_ << 10) | (invite_users_ << 15) | (pin_messages_ << 17) | (manage_topics_ << 18) | (send_photos_ << 19) | (send_videos_ << 20) | (send_roundvideos_ << 21) | (send_audios_ << 22) | (send_voices_ << 23) | (send_docs_ << 24) | (send_plain_ << 25)), s);
  TlStoreBinary::store(until_date_, s);
}

void chatBannedRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBannedRights");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (view_messages_ << 0) | (send_messages_ << 1) | (send_media_ << 2) | (send_stickers_ << 3) | (send_gifs_ << 4) | (send_games_ << 5) | (send_inline_ << 6) | (embed_links_ << 7) | (send_polls_ << 8) | (change_info_ << 10) | (invite_users_ << 15) | (pin_messages_ << 17) | (manage_topics_ << 18) | (send_photos_ << 19) | (send_videos_ << 20) | (send_roundvideos_ << 21) | (send_audios_ << 22) | (send_voices_ << 23) | (send_docs_ << 24) | (send_plain_ << 25)));
    if (var0 & 1) { s.store_field("view_messages", true); }
    if (var0 & 2) { s.store_field("send_messages", true); }
    if (var0 & 4) { s.store_field("send_media", true); }
    if (var0 & 8) { s.store_field("send_stickers", true); }
    if (var0 & 16) { s.store_field("send_gifs", true); }
    if (var0 & 32) { s.store_field("send_games", true); }
    if (var0 & 64) { s.store_field("send_inline", true); }
    if (var0 & 128) { s.store_field("embed_links", true); }
    if (var0 & 256) { s.store_field("send_polls", true); }
    if (var0 & 1024) { s.store_field("change_info", true); }
    if (var0 & 32768) { s.store_field("invite_users", true); }
    if (var0 & 131072) { s.store_field("pin_messages", true); }
    if (var0 & 262144) { s.store_field("manage_topics", true); }
    if (var0 & 524288) { s.store_field("send_photos", true); }
    if (var0 & 1048576) { s.store_field("send_videos", true); }
    if (var0 & 2097152) { s.store_field("send_roundvideos", true); }
    if (var0 & 4194304) { s.store_field("send_audios", true); }
    if (var0 & 8388608) { s.store_field("send_voices", true); }
    if (var0 & 16777216) { s.store_field("send_docs", true); }
    if (var0 & 33554432) { s.store_field("send_plain", true); }
    s.store_field("until_date", until_date_);
    s.store_class_end();
  }
}

object_ptr<ChatParticipant> ChatParticipant::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatParticipant::ID:
      return chatParticipant::fetch(p);
    case chatParticipantCreator::ID:
      return chatParticipantCreator::fetch(p);
    case chatParticipantAdmin::ID:
      return chatParticipantAdmin::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t chatParticipant::ID;

object_ptr<ChatParticipant> chatParticipant::fetch(TlBufferParser &p) {
  return make_tl_object<chatParticipant>(p);
}

chatParticipant::chatParticipant(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , inviter_id_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void chatParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatParticipant");
    s.store_field("user_id", user_id_);
    s.store_field("inviter_id", inviter_id_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t chatParticipantCreator::ID;

object_ptr<ChatParticipant> chatParticipantCreator::fetch(TlBufferParser &p) {
  return make_tl_object<chatParticipantCreator>(p);
}

chatParticipantCreator::chatParticipantCreator(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
{}

void chatParticipantCreator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatParticipantCreator");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

const std::int32_t chatParticipantAdmin::ID;

object_ptr<ChatParticipant> chatParticipantAdmin::fetch(TlBufferParser &p) {
  return make_tl_object<chatParticipantAdmin>(p);
}

chatParticipantAdmin::chatParticipantAdmin(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , inviter_id_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void chatParticipantAdmin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatParticipantAdmin");
    s.store_field("user_id", user_id_);
    s.store_field("inviter_id", inviter_id_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

object_ptr<ChatTheme> ChatTheme::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatTheme::ID:
      return chatTheme::fetch(p);
    case chatThemeUniqueGift::ID:
      return chatThemeUniqueGift::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t chatTheme::ID;

object_ptr<ChatTheme> chatTheme::fetch(TlBufferParser &p) {
  return make_tl_object<chatTheme>(p);
}

chatTheme::chatTheme(TlBufferParser &p)
  : emoticon_(TlFetchString<string>::parse(p))
{}

void chatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatTheme");
    s.store_field("emoticon", emoticon_);
    s.store_class_end();
  }
}

const std::int32_t chatThemeUniqueGift::ID;

object_ptr<ChatTheme> chatThemeUniqueGift::fetch(TlBufferParser &p) {
  return make_tl_object<chatThemeUniqueGift>(p);
}

chatThemeUniqueGift::chatThemeUniqueGift(TlBufferParser &p)
  : gift_(TlFetchObject<StarGift>::parse(p))
  , theme_settings_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<themeSettings>, -94849324>>, 481674261>::parse(p))
{}

void chatThemeUniqueGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatThemeUniqueGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    { s.store_vector_begin("theme_settings", theme_settings_.size()); for (const auto &_value : theme_settings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<Document> Document::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case documentEmpty::ID:
      return documentEmpty::fetch(p);
    case document::ID:
      return document::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t documentEmpty::ID;

object_ptr<Document> documentEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<documentEmpty>(p);
}

documentEmpty::documentEmpty(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void documentEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentEmpty");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

document::document()
  : flags_()
  , id_()
  , access_hash_()
  , file_reference_()
  , date_()
  , mime_type_()
  , size_()
  , thumbs_()
  , video_thumbs_()
  , dc_id_()
  , attributes_()
{}

document::document(int32 flags_, int64 id_, int64 access_hash_, bytes &&file_reference_, int32 date_, string const &mime_type_, int64 size_, array<object_ptr<PhotoSize>> &&thumbs_, array<object_ptr<VideoSize>> &&video_thumbs_, int32 dc_id_, array<object_ptr<DocumentAttribute>> &&attributes_)
  : flags_(flags_)
  , id_(id_)
  , access_hash_(access_hash_)
  , file_reference_(std::move(file_reference_))
  , date_(date_)
  , mime_type_(mime_type_)
  , size_(size_)
  , thumbs_(std::move(thumbs_))
  , video_thumbs_(std::move(video_thumbs_))
  , dc_id_(dc_id_)
  , attributes_(std::move(attributes_))
{}

const std::int32_t document::ID;

object_ptr<Document> document::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<document> res = make_tl_object<document>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->file_reference_ = TlFetchBytes<bytes>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->mime_type_ = TlFetchString<string>::parse(p);
  res->size_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->thumbs_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PhotoSize>>, 481674261>::parse(p); }
  if (var0 & 2) { res->video_thumbs_ = TlFetchBoxed<TlFetchVector<TlFetchObject<VideoSize>>, 481674261>::parse(p); }
  res->dc_id_ = TlFetchInt::parse(p);
  res->attributes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void document::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "document");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_field("date", date_);
    s.store_field("mime_type", mime_type_);
    s.store_field("size", size_);
    if (var0 & 1) { { s.store_vector_begin("thumbs", thumbs_.size()); for (const auto &_value : thumbs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { { s.store_vector_begin("video_thumbs", video_thumbs_.size()); for (const auto &_value : video_thumbs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("dc_id", dc_id_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

emailVerifyPurposeLoginSetup::emailVerifyPurposeLoginSetup(string const &phone_number_, string const &phone_code_hash_)
  : phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
{}

const std::int32_t emailVerifyPurposeLoginSetup::ID;

void emailVerifyPurposeLoginSetup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
}

void emailVerifyPurposeLoginSetup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
}

void emailVerifyPurposeLoginSetup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailVerifyPurposeLoginSetup");
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_class_end();
  }
}

const std::int32_t emailVerifyPurposeLoginChange::ID;

void emailVerifyPurposeLoginChange::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void emailVerifyPurposeLoginChange::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void emailVerifyPurposeLoginChange::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailVerifyPurposeLoginChange");
    s.store_class_end();
  }
}

const std::int32_t emailVerifyPurposePassport::ID;

void emailVerifyPurposePassport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void emailVerifyPurposePassport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void emailVerifyPurposePassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailVerifyPurposePassport");
    s.store_class_end();
  }
}

object_ptr<EmojiKeyword> EmojiKeyword::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case emojiKeyword::ID:
      return emojiKeyword::fetch(p);
    case emojiKeywordDeleted::ID:
      return emojiKeywordDeleted::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t emojiKeyword::ID;

object_ptr<EmojiKeyword> emojiKeyword::fetch(TlBufferParser &p) {
  return make_tl_object<emojiKeyword>(p);
}

emojiKeyword::emojiKeyword(TlBufferParser &p)
  : keyword_(TlFetchString<string>::parse(p))
  , emoticons_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
{}

void emojiKeyword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiKeyword");
    s.store_field("keyword", keyword_);
    { s.store_vector_begin("emoticons", emoticons_.size()); for (const auto &_value : emoticons_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t emojiKeywordDeleted::ID;

object_ptr<EmojiKeyword> emojiKeywordDeleted::fetch(TlBufferParser &p) {
  return make_tl_object<emojiKeywordDeleted>(p);
}

emojiKeywordDeleted::emojiKeywordDeleted(TlBufferParser &p)
  : keyword_(TlFetchString<string>::parse(p))
  , emoticons_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
{}

void emojiKeywordDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiKeywordDeleted");
    s.store_field("keyword", keyword_);
    { s.store_vector_begin("emoticons", emoticons_.size()); for (const auto &_value : emoticons_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t emojiKeywordsDifference::ID;

object_ptr<emojiKeywordsDifference> emojiKeywordsDifference::fetch(TlBufferParser &p) {
  return make_tl_object<emojiKeywordsDifference>(p);
}

emojiKeywordsDifference::emojiKeywordsDifference(TlBufferParser &p)
  : lang_code_(TlFetchString<string>::parse(p))
  , from_version_(TlFetchInt::parse(p))
  , version_(TlFetchInt::parse(p))
  , keywords_(TlFetchBoxed<TlFetchVector<TlFetchObject<EmojiKeyword>>, 481674261>::parse(p))
{}

void emojiKeywordsDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiKeywordsDifference");
    s.store_field("lang_code", lang_code_);
    s.store_field("from_version", from_version_);
    s.store_field("version", version_);
    { s.store_vector_begin("keywords", keywords_.size()); for (const auto &_value : keywords_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t foundStory::ID;

object_ptr<foundStory> foundStory::fetch(TlBufferParser &p) {
  return make_tl_object<foundStory>(p);
}

foundStory::foundStory(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , story_(TlFetchObject<StoryItem>::parse(p))
{}

void foundStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundStory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

inlineBotSwitchPM::inlineBotSwitchPM(string const &text_, string const &start_param_)
  : text_(text_)
  , start_param_(start_param_)
{}

const std::int32_t inlineBotSwitchPM::ID;

object_ptr<inlineBotSwitchPM> inlineBotSwitchPM::fetch(TlBufferParser &p) {
  return make_tl_object<inlineBotSwitchPM>(p);
}

inlineBotSwitchPM::inlineBotSwitchPM(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , start_param_(TlFetchString<string>::parse(p))
{}

void inlineBotSwitchPM::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(start_param_, s);
}

void inlineBotSwitchPM::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(start_param_, s);
}

void inlineBotSwitchPM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineBotSwitchPM");
    s.store_field("text", text_);
    s.store_field("start_param", start_param_);
    s.store_class_end();
  }
}

object_ptr<InlineQueryPeerType> InlineQueryPeerType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inlineQueryPeerTypeSameBotPM::ID;

object_ptr<InlineQueryPeerType> inlineQueryPeerTypeSameBotPM::fetch(TlBufferParser &p) {
  return make_tl_object<inlineQueryPeerTypeSameBotPM>();
}

void inlineQueryPeerTypeSameBotPM::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeSameBotPM::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeSameBotPM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryPeerTypeSameBotPM");
    s.store_class_end();
  }
}

const std::int32_t inlineQueryPeerTypePM::ID;

object_ptr<InlineQueryPeerType> inlineQueryPeerTypePM::fetch(TlBufferParser &p) {
  return make_tl_object<inlineQueryPeerTypePM>();
}

void inlineQueryPeerTypePM::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypePM::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypePM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryPeerTypePM");
    s.store_class_end();
  }
}

const std::int32_t inlineQueryPeerTypeChat::ID;

object_ptr<InlineQueryPeerType> inlineQueryPeerTypeChat::fetch(TlBufferParser &p) {
  return make_tl_object<inlineQueryPeerTypeChat>();
}

void inlineQueryPeerTypeChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryPeerTypeChat");
    s.store_class_end();
  }
}

const std::int32_t inlineQueryPeerTypeMegagroup::ID;

object_ptr<InlineQueryPeerType> inlineQueryPeerTypeMegagroup::fetch(TlBufferParser &p) {
  return make_tl_object<inlineQueryPeerTypeMegagroup>();
}

void inlineQueryPeerTypeMegagroup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeMegagroup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeMegagroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryPeerTypeMegagroup");
    s.store_class_end();
  }
}

const std::int32_t inlineQueryPeerTypeBroadcast::ID;

object_ptr<InlineQueryPeerType> inlineQueryPeerTypeBroadcast::fetch(TlBufferParser &p) {
  return make_tl_object<inlineQueryPeerTypeBroadcast>();
}

void inlineQueryPeerTypeBroadcast::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeBroadcast::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeBroadcast::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryPeerTypeBroadcast");
    s.store_class_end();
  }
}

const std::int32_t inlineQueryPeerTypeBotPM::ID;

object_ptr<InlineQueryPeerType> inlineQueryPeerTypeBotPM::fetch(TlBufferParser &p) {
  return make_tl_object<inlineQueryPeerTypeBotPM>();
}

void inlineQueryPeerTypeBotPM::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeBotPM::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inlineQueryPeerTypeBotPM::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryPeerTypeBotPM");
    s.store_class_end();
  }
}

object_ptr<InputChannel> InputChannel::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputChannelEmpty::ID:
      return inputChannelEmpty::fetch(p);
    case inputChannel::ID:
      return inputChannel::fetch(p);
    case inputChannelFromMessage::ID:
      return inputChannelFromMessage::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputChannelEmpty::ID;

object_ptr<InputChannel> inputChannelEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputChannelEmpty>();
}

void inputChannelEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputChannelEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputChannelEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChannelEmpty");
    s.store_class_end();
  }
}

inputChannel::inputChannel(int64 channel_id_, int64 access_hash_)
  : channel_id_(channel_id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputChannel::ID;

object_ptr<InputChannel> inputChannel::fetch(TlBufferParser &p) {
  return make_tl_object<inputChannel>(p);
}

inputChannel::inputChannel(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(channel_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(channel_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChannel");
    s.store_field("channel_id", channel_id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputChannelFromMessage::inputChannelFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 channel_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , channel_id_(channel_id_)
{}

const std::int32_t inputChannelFromMessage::ID;

object_ptr<InputChannel> inputChannelFromMessage::fetch(TlBufferParser &p) {
  return make_tl_object<inputChannelFromMessage>(p);
}

inputChannelFromMessage::inputChannelFromMessage(TlBufferParser &p)
  : peer_(TlFetchObject<InputPeer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , channel_id_(TlFetchLong::parse(p))
{}

void inputChannelFromMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(channel_id_, s);
}

void inputChannelFromMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(channel_id_, s);
}

void inputChannelFromMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChannelFromMessage");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("channel_id", channel_id_);
    s.store_class_end();
  }
}

const std::int32_t inputChatThemeEmpty::ID;

void inputChatThemeEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputChatThemeEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputChatThemeEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatThemeEmpty");
    s.store_class_end();
  }
}

inputChatTheme::inputChatTheme(string const &emoticon_)
  : emoticon_(emoticon_)
{}

const std::int32_t inputChatTheme::ID;

void inputChatTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void inputChatTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void inputChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatTheme");
    s.store_field("emoticon", emoticon_);
    s.store_class_end();
  }
}

inputChatThemeUniqueGift::inputChatThemeUniqueGift(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t inputChatThemeUniqueGift::ID;

void inputChatThemeUniqueGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputChatThemeUniqueGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void inputChatThemeUniqueGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatThemeUniqueGift");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

const std::int32_t inputClientProxy::ID;

void inputClientProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputClientProxy");
    s.store_field("address", address_);
    s.store_field("port", port_);
    s.store_class_end();
  }
}

inputMessageID::inputMessageID(int32 id_)
  : id_(id_)
{}

const std::int32_t inputMessageID::ID;

void inputMessageID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void inputMessageID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void inputMessageID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageID");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

inputMessageReplyTo::inputMessageReplyTo(int32 id_)
  : id_(id_)
{}

const std::int32_t inputMessageReplyTo::ID;

void inputMessageReplyTo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void inputMessageReplyTo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void inputMessageReplyTo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageReplyTo");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

const std::int32_t inputMessagePinned::ID;

void inputMessagePinned::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMessagePinned::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMessagePinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagePinned");
    s.store_class_end();
  }
}

inputMessageCallbackQuery::inputMessageCallbackQuery(int32 id_, int64 query_id_)
  : id_(id_)
  , query_id_(query_id_)
{}

const std::int32_t inputMessageCallbackQuery::ID;

void inputMessageCallbackQuery::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(query_id_, s);
}

void inputMessageCallbackQuery::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(query_id_, s);
}

void inputMessageCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageCallbackQuery");
    s.store_field("id", id_);
    s.store_field("query_id", query_id_);
    s.store_class_end();
  }
}

inputStarsTransaction::inputStarsTransaction(int32 flags_, bool refund_, string const &id_)
  : flags_(flags_)
  , refund_(refund_)
  , id_(id_)
{}

const std::int32_t inputStarsTransaction::ID;

void inputStarsTransaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (refund_ << 0)), s);
  TlStoreString::store(id_, s);
}

void inputStarsTransaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (refund_ << 0)), s);
  TlStoreString::store(id_, s);
}

void inputStarsTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStarsTransaction");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (refund_ << 0)));
    if (var0 & 1) { s.store_field("refund", true); }
    s.store_field("id", id_);
    s.store_class_end();
  }
}

object_ptr<InputUser> InputUser::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputUserEmpty::ID:
      return inputUserEmpty::fetch(p);
    case inputUserSelf::ID:
      return inputUserSelf::fetch(p);
    case inputUser::ID:
      return inputUser::fetch(p);
    case inputUserFromMessage::ID:
      return inputUserFromMessage::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputUserEmpty::ID;

object_ptr<InputUser> inputUserEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputUserEmpty>();
}

void inputUserEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputUserEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputUserEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputUserEmpty");
    s.store_class_end();
  }
}

const std::int32_t inputUserSelf::ID;

object_ptr<InputUser> inputUserSelf::fetch(TlBufferParser &p) {
  return make_tl_object<inputUserSelf>();
}

void inputUserSelf::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputUserSelf::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputUserSelf::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputUserSelf");
    s.store_class_end();
  }
}

inputUser::inputUser(int64 user_id_, int64 access_hash_)
  : user_id_(user_id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputUser::ID;

object_ptr<InputUser> inputUser::fetch(TlBufferParser &p) {
  return make_tl_object<inputUser>(p);
}

inputUser::inputUser(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputUser");
    s.store_field("user_id", user_id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputUserFromMessage::inputUserFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 user_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , user_id_(user_id_)
{}

const std::int32_t inputUserFromMessage::ID;

object_ptr<InputUser> inputUserFromMessage::fetch(TlBufferParser &p) {
  return make_tl_object<inputUserFromMessage>(p);
}

inputUserFromMessage::inputUserFromMessage(TlBufferParser &p)
  : peer_(TlFetchObject<InputPeer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , user_id_(TlFetchLong::parse(p))
{}

void inputUserFromMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(user_id_, s);
}

void inputUserFromMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(user_id_, s);
}

void inputUserFromMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputUserFromMessage");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

inputWebFileLocation::inputWebFileLocation(string const &url_, int64 access_hash_)
  : url_(url_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputWebFileLocation::ID;

void inputWebFileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(url_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputWebFileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(url_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputWebFileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputWebFileLocation");
    s.store_field("url", url_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputWebFileGeoPointLocation::inputWebFileGeoPointLocation(object_ptr<InputGeoPoint> &&geo_point_, int64 access_hash_, int32 w_, int32 h_, int32 zoom_, int32 scale_)
  : geo_point_(std::move(geo_point_))
  , access_hash_(access_hash_)
  , w_(w_)
  , h_(h_)
  , zoom_(zoom_)
  , scale_(scale_)
{}

const std::int32_t inputWebFileGeoPointLocation::ID;

void inputWebFileGeoPointLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(zoom_, s);
  TlStoreBinary::store(scale_, s);
}

void inputWebFileGeoPointLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(zoom_, s);
  TlStoreBinary::store(scale_, s);
}

void inputWebFileGeoPointLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputWebFileGeoPointLocation");
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    s.store_field("access_hash", access_hash_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("zoom", zoom_);
    s.store_field("scale", scale_);
    s.store_class_end();
  }
}

inputWebFileAudioAlbumThumbLocation::inputWebFileAudioAlbumThumbLocation(int32 flags_, bool small_, object_ptr<InputDocument> &&document_, string const &title_, string const &performer_)
  : flags_(flags_)
  , small_(small_)
  , document_(std::move(document_))
  , title_(title_)
  , performer_(performer_)
{}

const std::int32_t inputWebFileAudioAlbumThumbLocation::ID;

void inputWebFileAudioAlbumThumbLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (small_ << 2)), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(document_, s); }
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreString::store(performer_, s); }
}

void inputWebFileAudioAlbumThumbLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (small_ << 2)), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(document_, s); }
  if (var0 & 2) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreString::store(performer_, s); }
}

void inputWebFileAudioAlbumThumbLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputWebFileAudioAlbumThumbLocation");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (small_ << 2)));
    if (var0 & 4) { s.store_field("small", true); }
    if (var0 & 1) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 2) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("performer", performer_); }
    s.store_class_end();
  }
}

invoice::invoice()
  : flags_()
  , test_()
  , name_requested_()
  , phone_requested_()
  , email_requested_()
  , shipping_address_requested_()
  , flexible_()
  , phone_to_provider_()
  , email_to_provider_()
  , recurring_()
  , currency_()
  , prices_()
  , max_tip_amount_()
  , suggested_tip_amounts_()
  , terms_url_()
  , subscription_period_()
{}

invoice::invoice(int32 flags_, bool test_, bool name_requested_, bool phone_requested_, bool email_requested_, bool shipping_address_requested_, bool flexible_, bool phone_to_provider_, bool email_to_provider_, bool recurring_, string const &currency_, array<object_ptr<labeledPrice>> &&prices_, int64 max_tip_amount_, array<int64> &&suggested_tip_amounts_, string const &terms_url_, int32 subscription_period_)
  : flags_(flags_)
  , test_(test_)
  , name_requested_(name_requested_)
  , phone_requested_(phone_requested_)
  , email_requested_(email_requested_)
  , shipping_address_requested_(shipping_address_requested_)
  , flexible_(flexible_)
  , phone_to_provider_(phone_to_provider_)
  , email_to_provider_(email_to_provider_)
  , recurring_(recurring_)
  , currency_(currency_)
  , prices_(std::move(prices_))
  , max_tip_amount_(max_tip_amount_)
  , suggested_tip_amounts_(std::move(suggested_tip_amounts_))
  , terms_url_(terms_url_)
  , subscription_period_(subscription_period_)
{}

const std::int32_t invoice::ID;

object_ptr<invoice> invoice::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<invoice> res = make_tl_object<invoice>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->test_ = (var0 & 1) != 0;
  res->name_requested_ = (var0 & 2) != 0;
  res->phone_requested_ = (var0 & 4) != 0;
  res->email_requested_ = (var0 & 8) != 0;
  res->shipping_address_requested_ = (var0 & 16) != 0;
  res->flexible_ = (var0 & 32) != 0;
  res->phone_to_provider_ = (var0 & 64) != 0;
  res->email_to_provider_ = (var0 & 128) != 0;
  res->recurring_ = (var0 & 512) != 0;
  res->currency_ = TlFetchString<string>::parse(p);
  res->prices_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<labeledPrice>, -886477832>>, 481674261>::parse(p);
  if (var0 & 256) { res->max_tip_amount_ = TlFetchLong::parse(p); }
  if (var0 & 256) { res->suggested_tip_amounts_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p); }
  if (var0 & 1024) { res->terms_url_ = TlFetchString<string>::parse(p); }
  if (var0 & 2048) { res->subscription_period_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void invoice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (test_ << 0) | (name_requested_ << 1) | (phone_requested_ << 2) | (email_requested_ << 3) | (shipping_address_requested_ << 4) | (flexible_ << 5) | (phone_to_provider_ << 6) | (email_to_provider_ << 7) | (recurring_ << 9)), s);
  TlStoreString::store(currency_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -886477832>>, 481674261>::store(prices_, s);
  if (var0 & 256) { TlStoreBinary::store(max_tip_amount_, s); }
  if (var0 & 256) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(suggested_tip_amounts_, s); }
  if (var0 & 1024) { TlStoreString::store(terms_url_, s); }
  if (var0 & 2048) { TlStoreBinary::store(subscription_period_, s); }
}

void invoice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (test_ << 0) | (name_requested_ << 1) | (phone_requested_ << 2) | (email_requested_ << 3) | (shipping_address_requested_ << 4) | (flexible_ << 5) | (phone_to_provider_ << 6) | (email_to_provider_ << 7) | (recurring_ << 9)), s);
  TlStoreString::store(currency_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -886477832>>, 481674261>::store(prices_, s);
  if (var0 & 256) { TlStoreBinary::store(max_tip_amount_, s); }
  if (var0 & 256) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(suggested_tip_amounts_, s); }
  if (var0 & 1024) { TlStoreString::store(terms_url_, s); }
  if (var0 & 2048) { TlStoreBinary::store(subscription_period_, s); }
}

void invoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "invoice");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (test_ << 0) | (name_requested_ << 1) | (phone_requested_ << 2) | (email_requested_ << 3) | (shipping_address_requested_ << 4) | (flexible_ << 5) | (phone_to_provider_ << 6) | (email_to_provider_ << 7) | (recurring_ << 9)));
    if (var0 & 1) { s.store_field("test", true); }
    if (var0 & 2) { s.store_field("name_requested", true); }
    if (var0 & 4) { s.store_field("phone_requested", true); }
    if (var0 & 8) { s.store_field("email_requested", true); }
    if (var0 & 16) { s.store_field("shipping_address_requested", true); }
    if (var0 & 32) { s.store_field("flexible", true); }
    if (var0 & 64) { s.store_field("phone_to_provider", true); }
    if (var0 & 128) { s.store_field("email_to_provider", true); }
    if (var0 & 512) { s.store_field("recurring", true); }
    s.store_field("currency", currency_);
    { s.store_vector_begin("prices", prices_.size()); for (const auto &_value : prices_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 256) { s.store_field("max_tip_amount", max_tip_amount_); }
    if (var0 & 256) { { s.store_vector_begin("suggested_tip_amounts", suggested_tip_amounts_.size()); for (const auto &_value : suggested_tip_amounts_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 1024) { s.store_field("terms_url", terms_url_); }
    if (var0 & 2048) { s.store_field("subscription_period", subscription_period_); }
    s.store_class_end();
  }
}

langPackLanguage::langPackLanguage()
  : flags_()
  , official_()
  , rtl_()
  , beta_()
  , name_()
  , native_name_()
  , lang_code_()
  , base_lang_code_()
  , plural_code_()
  , strings_count_()
  , translated_count_()
  , translations_url_()
{}

const std::int32_t langPackLanguage::ID;

object_ptr<langPackLanguage> langPackLanguage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<langPackLanguage> res = make_tl_object<langPackLanguage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->official_ = (var0 & 1) != 0;
  res->rtl_ = (var0 & 4) != 0;
  res->beta_ = (var0 & 8) != 0;
  res->name_ = TlFetchString<string>::parse(p);
  res->native_name_ = TlFetchString<string>::parse(p);
  res->lang_code_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->base_lang_code_ = TlFetchString<string>::parse(p); }
  res->plural_code_ = TlFetchString<string>::parse(p);
  res->strings_count_ = TlFetchInt::parse(p);
  res->translated_count_ = TlFetchInt::parse(p);
  res->translations_url_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void langPackLanguage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langPackLanguage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (official_ << 0) | (rtl_ << 2) | (beta_ << 3)));
    if (var0 & 1) { s.store_field("official", true); }
    if (var0 & 4) { s.store_field("rtl", true); }
    if (var0 & 8) { s.store_field("beta", true); }
    s.store_field("name", name_);
    s.store_field("native_name", native_name_);
    s.store_field("lang_code", lang_code_);
    if (var0 & 2) { s.store_field("base_lang_code", base_lang_code_); }
    s.store_field("plural_code", plural_code_);
    s.store_field("strings_count", strings_count_);
    s.store_field("translated_count", translated_count_);
    s.store_field("translations_url", translations_url_);
    s.store_class_end();
  }
}

object_ptr<Message> Message::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messageEmpty::ID:
      return messageEmpty::fetch(p);
    case message::ID:
      return message::fetch(p);
    case messageService::ID:
      return messageService::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

messageEmpty::messageEmpty()
  : flags_()
  , id_()
  , peer_id_()
{}

const std::int32_t messageEmpty::ID;

object_ptr<Message> messageEmpty::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageEmpty> res = make_tl_object<messageEmpty>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->peer_id_ = TlFetchObject<Peer>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEmpty");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get())); }
    s.store_class_end();
  }
}

message::message()
  : flags_()
  , out_()
  , mentioned_()
  , media_unread_()
  , silent_()
  , post_()
  , from_scheduled_()
  , legacy_()
  , edit_hide_()
  , pinned_()
  , noforwards_()
  , invert_media_()
  , flags2_()
  , offline_()
  , video_processing_pending_()
  , paid_suggested_post_stars_()
  , paid_suggested_post_ton_()
  , id_()
  , from_id_()
  , from_boosts_applied_()
  , peer_id_()
  , saved_peer_id_()
  , fwd_from_()
  , via_bot_id_()
  , via_business_bot_id_()
  , reply_to_()
  , date_()
  , message_()
  , media_()
  , reply_markup_()
  , entities_()
  , views_()
  , forwards_()
  , replies_()
  , edit_date_()
  , post_author_()
  , grouped_id_()
  , reactions_()
  , restriction_reason_()
  , ttl_period_()
  , quick_reply_shortcut_id_()
  , effect_()
  , factcheck_()
  , report_delivery_until_date_()
  , paid_message_stars_()
  , suggested_post_()
{}

message::message(int32 flags_, bool out_, bool mentioned_, bool media_unread_, bool silent_, bool post_, bool from_scheduled_, bool legacy_, bool edit_hide_, bool pinned_, bool noforwards_, bool invert_media_, int32 flags2_, bool offline_, bool video_processing_pending_, bool paid_suggested_post_stars_, bool paid_suggested_post_ton_, int32 id_, object_ptr<Peer> &&from_id_, int32 from_boosts_applied_, object_ptr<Peer> &&peer_id_, object_ptr<Peer> &&saved_peer_id_, object_ptr<messageFwdHeader> &&fwd_from_, int64 via_bot_id_, int64 via_business_bot_id_, object_ptr<MessageReplyHeader> &&reply_to_, int32 date_, string const &message_, object_ptr<MessageMedia> &&media_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 views_, int32 forwards_, object_ptr<messageReplies> &&replies_, int32 edit_date_, string const &post_author_, int64 grouped_id_, object_ptr<messageReactions> &&reactions_, array<object_ptr<restrictionReason>> &&restriction_reason_, int32 ttl_period_, int32 quick_reply_shortcut_id_, int64 effect_, object_ptr<factCheck> &&factcheck_, int32 report_delivery_until_date_, int64 paid_message_stars_, object_ptr<suggestedPost> &&suggested_post_)
  : flags_(flags_)
  , out_(out_)
  , mentioned_(mentioned_)
  , media_unread_(media_unread_)
  , silent_(silent_)
  , post_(post_)
  , from_scheduled_(from_scheduled_)
  , legacy_(legacy_)
  , edit_hide_(edit_hide_)
  , pinned_(pinned_)
  , noforwards_(noforwards_)
  , invert_media_(invert_media_)
  , flags2_(flags2_)
  , offline_(offline_)
  , video_processing_pending_(video_processing_pending_)
  , paid_suggested_post_stars_(paid_suggested_post_stars_)
  , paid_suggested_post_ton_(paid_suggested_post_ton_)
  , id_(id_)
  , from_id_(std::move(from_id_))
  , from_boosts_applied_(from_boosts_applied_)
  , peer_id_(std::move(peer_id_))
  , saved_peer_id_(std::move(saved_peer_id_))
  , fwd_from_(std::move(fwd_from_))
  , via_bot_id_(via_bot_id_)
  , via_business_bot_id_(via_business_bot_id_)
  , reply_to_(std::move(reply_to_))
  , date_(date_)
  , message_(message_)
  , media_(std::move(media_))
  , reply_markup_(std::move(reply_markup_))
  , entities_(std::move(entities_))
  , views_(views_)
  , forwards_(forwards_)
  , replies_(std::move(replies_))
  , edit_date_(edit_date_)
  , post_author_(post_author_)
  , grouped_id_(grouped_id_)
  , reactions_(std::move(reactions_))
  , restriction_reason_(std::move(restriction_reason_))
  , ttl_period_(ttl_period_)
  , quick_reply_shortcut_id_(quick_reply_shortcut_id_)
  , effect_(effect_)
  , factcheck_(std::move(factcheck_))
  , report_delivery_until_date_(report_delivery_until_date_)
  , paid_message_stars_(paid_message_stars_)
  , suggested_post_(std::move(suggested_post_))
{}

const std::int32_t message::ID;

object_ptr<Message> message::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<message> res = make_tl_object<message>();
  int32 var0;
  int32 var1;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->out_ = (var0 & 2) != 0;
  res->mentioned_ = (var0 & 16) != 0;
  res->media_unread_ = (var0 & 32) != 0;
  res->silent_ = (var0 & 8192) != 0;
  res->post_ = (var0 & 16384) != 0;
  res->from_scheduled_ = (var0 & 262144) != 0;
  res->legacy_ = (var0 & 524288) != 0;
  res->edit_hide_ = (var0 & 2097152) != 0;
  res->pinned_ = (var0 & 16777216) != 0;
  res->noforwards_ = (var0 & 67108864) != 0;
  res->invert_media_ = (var0 & 134217728) != 0;
  if ((var1 = res->flags2_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->offline_ = (var1 & 2) != 0;
  res->video_processing_pending_ = (var1 & 16) != 0;
  res->paid_suggested_post_stars_ = (var1 & 256) != 0;
  res->paid_suggested_post_ton_ = (var1 & 512) != 0;
  res->id_ = TlFetchInt::parse(p);
  if (var0 & 256) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 536870912) { res->from_boosts_applied_ = TlFetchInt::parse(p); }
  res->peer_id_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 268435456) { res->saved_peer_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 4) { res->fwd_from_ = TlFetchBoxed<TlFetchObject<messageFwdHeader>, 1313731771>::parse(p); }
  if (var0 & 2048) { res->via_bot_id_ = TlFetchLong::parse(p); }
  if (var1 & 1) { res->via_business_bot_id_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->reply_to_ = TlFetchObject<MessageReplyHeader>::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 512) { res->media_ = TlFetchObject<MessageMedia>::parse(p); }
  if (var0 & 64) { res->reply_markup_ = TlFetchObject<ReplyMarkup>::parse(p); }
  if (var0 & 128) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 1024) { res->views_ = TlFetchInt::parse(p); }
  if (var0 & 1024) { res->forwards_ = TlFetchInt::parse(p); }
  if (var0 & 8388608) { res->replies_ = TlFetchBoxed<TlFetchObject<messageReplies>, -2083123262>::parse(p); }
  if (var0 & 32768) { res->edit_date_ = TlFetchInt::parse(p); }
  if (var0 & 65536) { res->post_author_ = TlFetchString<string>::parse(p); }
  if (var0 & 131072) { res->grouped_id_ = TlFetchLong::parse(p); }
  if (var0 & 1048576) { res->reactions_ = TlFetchBoxed<TlFetchObject<messageReactions>, 171155211>::parse(p); }
  if (var0 & 4194304) { res->restriction_reason_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<restrictionReason>, -797791052>>, 481674261>::parse(p); }
  if (var0 & 33554432) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (var0 & 1073741824) { res->quick_reply_shortcut_id_ = TlFetchInt::parse(p); }
  if (var1 & 4) { res->effect_ = TlFetchLong::parse(p); }
  if (var1 & 8) { res->factcheck_ = TlFetchBoxed<TlFetchObject<factCheck>, -1197736753>::parse(p); }
  if (var1 & 32) { res->report_delivery_until_date_ = TlFetchInt::parse(p); }
  if (var1 & 64) { res->paid_message_stars_ = TlFetchLong::parse(p); }
  if (var1 & 128) { res->suggested_post_ = TlFetchBoxed<TlFetchObject<suggestedPost>, 244201445>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void message::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "message");
  int32 var0;
  int32 var1;
    s.store_field("flags", (var0 = flags_ | (out_ << 1) | (mentioned_ << 4) | (media_unread_ << 5) | (silent_ << 13) | (post_ << 14) | (from_scheduled_ << 18) | (legacy_ << 19) | (edit_hide_ << 21) | (pinned_ << 24) | (noforwards_ << 26) | (invert_media_ << 27)));
    if (var0 & 2) { s.store_field("out", true); }
    if (var0 & 16) { s.store_field("mentioned", true); }
    if (var0 & 32) { s.store_field("media_unread", true); }
    if (var0 & 8192) { s.store_field("silent", true); }
    if (var0 & 16384) { s.store_field("post", true); }
    if (var0 & 262144) { s.store_field("from_scheduled", true); }
    if (var0 & 524288) { s.store_field("legacy", true); }
    if (var0 & 2097152) { s.store_field("edit_hide", true); }
    if (var0 & 16777216) { s.store_field("pinned", true); }
    if (var0 & 67108864) { s.store_field("noforwards", true); }
    if (var0 & 134217728) { s.store_field("invert_media", true); }
    s.store_field("flags2", (var1 = flags2_ | (offline_ << 1) | (video_processing_pending_ << 4) | (paid_suggested_post_stars_ << 8) | (paid_suggested_post_ton_ << 9)));
    if (var1 & 2) { s.store_field("offline", true); }
    if (var1 & 16) { s.store_field("video_processing_pending", true); }
    if (var1 & 256) { s.store_field("paid_suggested_post_stars", true); }
    if (var1 & 512) { s.store_field("paid_suggested_post_ton", true); }
    s.store_field("id", id_);
    if (var0 & 256) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    if (var0 & 536870912) { s.store_field("from_boosts_applied", from_boosts_applied_); }
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    if (var0 & 268435456) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    if (var0 & 4) { s.store_object_field("fwd_from", static_cast<const BaseObject *>(fwd_from_.get())); }
    if (var0 & 2048) { s.store_field("via_bot_id", via_bot_id_); }
    if (var1 & 1) { s.store_field("via_business_bot_id", via_business_bot_id_); }
    if (var0 & 8) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    s.store_field("date", date_);
    s.store_field("message", message_);
    if (var0 & 512) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 64) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    if (var0 & 128) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 1024) { s.store_field("views", views_); }
    if (var0 & 1024) { s.store_field("forwards", forwards_); }
    if (var0 & 8388608) { s.store_object_field("replies", static_cast<const BaseObject *>(replies_.get())); }
    if (var0 & 32768) { s.store_field("edit_date", edit_date_); }
    if (var0 & 65536) { s.store_field("post_author", post_author_); }
    if (var0 & 131072) { s.store_field("grouped_id", grouped_id_); }
    if (var0 & 1048576) { s.store_object_field("reactions", static_cast<const BaseObject *>(reactions_.get())); }
    if (var0 & 4194304) { { s.store_vector_begin("restriction_reason", restriction_reason_.size()); for (const auto &_value : restriction_reason_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 33554432) { s.store_field("ttl_period", ttl_period_); }
    if (var0 & 1073741824) { s.store_field("quick_reply_shortcut_id", quick_reply_shortcut_id_); }
    if (var1 & 4) { s.store_field("effect", effect_); }
    if (var1 & 8) { s.store_object_field("factcheck", static_cast<const BaseObject *>(factcheck_.get())); }
    if (var1 & 32) { s.store_field("report_delivery_until_date", report_delivery_until_date_); }
    if (var1 & 64) { s.store_field("paid_message_stars", paid_message_stars_); }
    if (var1 & 128) { s.store_object_field("suggested_post", static_cast<const BaseObject *>(suggested_post_.get())); }
    s.store_class_end();
  }
}

messageService::messageService()
  : flags_()
  , out_()
  , mentioned_()
  , media_unread_()
  , reactions_are_possible_()
  , silent_()
  , post_()
  , legacy_()
  , id_()
  , from_id_()
  , peer_id_()
  , saved_peer_id_()
  , reply_to_()
  , date_()
  , action_()
  , reactions_()
  , ttl_period_()
{}

const std::int32_t messageService::ID;

object_ptr<Message> messageService::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageService> res = make_tl_object<messageService>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->out_ = (var0 & 2) != 0;
  res->mentioned_ = (var0 & 16) != 0;
  res->media_unread_ = (var0 & 32) != 0;
  res->reactions_are_possible_ = (var0 & 512) != 0;
  res->silent_ = (var0 & 8192) != 0;
  res->post_ = (var0 & 16384) != 0;
  res->legacy_ = (var0 & 524288) != 0;
  res->id_ = TlFetchInt::parse(p);
  if (var0 & 256) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  res->peer_id_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 268435456) { res->saved_peer_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 8) { res->reply_to_ = TlFetchObject<MessageReplyHeader>::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  res->action_ = TlFetchObject<MessageAction>::parse(p);
  if (var0 & 1048576) { res->reactions_ = TlFetchBoxed<TlFetchObject<messageReactions>, 171155211>::parse(p); }
  if (var0 & 33554432) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageService");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (out_ << 1) | (mentioned_ << 4) | (media_unread_ << 5) | (reactions_are_possible_ << 9) | (silent_ << 13) | (post_ << 14) | (legacy_ << 19)));
    if (var0 & 2) { s.store_field("out", true); }
    if (var0 & 16) { s.store_field("mentioned", true); }
    if (var0 & 32) { s.store_field("media_unread", true); }
    if (var0 & 512) { s.store_field("reactions_are_possible", true); }
    if (var0 & 8192) { s.store_field("silent", true); }
    if (var0 & 16384) { s.store_field("post", true); }
    if (var0 & 524288) { s.store_field("legacy", true); }
    s.store_field("id", id_);
    if (var0 & 256) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    if (var0 & 268435456) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    if (var0 & 8) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    s.store_field("date", date_);
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    if (var0 & 1048576) { s.store_object_field("reactions", static_cast<const BaseObject *>(reactions_.get())); }
    if (var0 & 33554432) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

messageReactions::messageReactions()
  : flags_()
  , min_()
  , can_see_list_()
  , reactions_as_tags_()
  , results_()
  , recent_reactions_()
  , top_reactors_()
{}

const std::int32_t messageReactions::ID;

object_ptr<messageReactions> messageReactions::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageReactions> res = make_tl_object<messageReactions>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->min_ = (var0 & 1) != 0;
  res->can_see_list_ = (var0 & 4) != 0;
  res->reactions_as_tags_ = (var0 & 8) != 0;
  res->results_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<reactionCount>, -1546531968>>, 481674261>::parse(p);
  if (var0 & 2) { res->recent_reactions_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<messagePeerReaction>, -1938180548>>, 481674261>::parse(p); }
  if (var0 & 16) { res->top_reactors_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<messageReactor>, 1269016922>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReactions");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (min_ << 0) | (can_see_list_ << 2) | (reactions_as_tags_ << 3)));
    if (var0 & 1) { s.store_field("min", true); }
    if (var0 & 4) { s.store_field("can_see_list", true); }
    if (var0 & 8) { s.store_field("reactions_as_tags", true); }
    { s.store_vector_begin("results", results_.size()); for (const auto &_value : results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 2) { { s.store_vector_begin("recent_reactions", recent_reactions_.size()); for (const auto &_value : recent_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 16) { { s.store_vector_begin("top_reactors", top_reactors_.size()); for (const auto &_value : top_reactors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

page::page()
  : flags_()
  , part_()
  , rtl_()
  , v2_()
  , url_()
  , blocks_()
  , photos_()
  , documents_()
  , views_()
{}

const std::int32_t page::ID;

object_ptr<page> page::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<page> res = make_tl_object<page>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->part_ = (var0 & 1) != 0;
  res->rtl_ = (var0 & 2) != 0;
  res->v2_ = (var0 & 4) != 0;
  res->url_ = TlFetchString<string>::parse(p);
  res->blocks_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PageBlock>>, 481674261>::parse(p);
  res->photos_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Photo>>, 481674261>::parse(p);
  res->documents_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p);
  if (var0 & 8) { res->views_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void page::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "page");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (part_ << 0) | (rtl_ << 1) | (v2_ << 2)));
    if (var0 & 1) { s.store_field("part", true); }
    if (var0 & 2) { s.store_field("rtl", true); }
    if (var0 & 4) { s.store_field("v2", true); }
    s.store_field("url", url_);
    { s.store_vector_begin("blocks", blocks_.size()); for (const auto &_value : blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("photos", photos_.size()); for (const auto &_value : photos_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 8) { s.store_field("views", views_); }
    s.store_class_end();
  }
}

const std::int32_t paymentSavedCredentialsCard::ID;

object_ptr<paymentSavedCredentialsCard> paymentSavedCredentialsCard::fetch(TlBufferParser &p) {
  return make_tl_object<paymentSavedCredentialsCard>(p);
}

paymentSavedCredentialsCard::paymentSavedCredentialsCard(TlBufferParser &p)
  : id_(TlFetchString<string>::parse(p))
  , title_(TlFetchString<string>::parse(p))
{}

void paymentSavedCredentialsCard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentSavedCredentialsCard");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

peerStories::peerStories()
  : flags_()
  , peer_()
  , max_read_id_()
  , stories_()
{}

const std::int32_t peerStories::ID;

object_ptr<peerStories> peerStories::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<peerStories> res = make_tl_object<peerStories>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 1) { res->max_read_id_ = TlFetchInt::parse(p); }
  res->stories_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StoryItem>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void peerStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerStories");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("max_read_id", max_read_id_); }
    { s.store_vector_begin("stories", stories_.size()); for (const auto &_value : stories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pollAnswer::pollAnswer(object_ptr<textWithEntities> &&text_, bytes &&option_)
  : text_(std::move(text_))
  , option_(std::move(option_))
{}

const std::int32_t pollAnswer::ID;

object_ptr<pollAnswer> pollAnswer::fetch(TlBufferParser &p) {
  return make_tl_object<pollAnswer>(p);
}

pollAnswer::pollAnswer(TlBufferParser &p)
  : text_(TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p))
  , option_(TlFetchBytes<bytes>::parse(p))
{}

void pollAnswer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(text_, s);
  TlStoreString::store(option_, s);
}

void pollAnswer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(text_, s);
  TlStoreString::store(option_, s);
}

void pollAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pollAnswer");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_bytes_field("option", option_);
    s.store_class_end();
  }
}

const std::int32_t popularContact::ID;

object_ptr<popularContact> popularContact::fetch(TlBufferParser &p) {
  return make_tl_object<popularContact>(p);
}

popularContact::popularContact(TlBufferParser &p)
  : client_id_(TlFetchLong::parse(p))
  , importers_(TlFetchInt::parse(p))
{}

void popularContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "popularContact");
    s.store_field("client_id", client_id_);
    s.store_field("importers", importers_);
    s.store_class_end();
  }
}

const std::int32_t searchResultsCalendarPeriod::ID;

object_ptr<searchResultsCalendarPeriod> searchResultsCalendarPeriod::fetch(TlBufferParser &p) {
  return make_tl_object<searchResultsCalendarPeriod>(p);
}

searchResultsCalendarPeriod::searchResultsCalendarPeriod(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
  , min_msg_id_(TlFetchInt::parse(p))
  , max_msg_id_(TlFetchInt::parse(p))
  , count_(TlFetchInt::parse(p))
{}

void searchResultsCalendarPeriod::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchResultsCalendarPeriod");
    s.store_field("date", date_);
    s.store_field("min_msg_id", min_msg_id_);
    s.store_field("max_msg_id", max_msg_id_);
    s.store_field("count", count_);
    s.store_class_end();
  }
}

object_ptr<SecurePlainData> SecurePlainData::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case securePlainPhone::ID:
      return securePlainPhone::fetch(p);
    case securePlainEmail::ID:
      return securePlainEmail::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

securePlainPhone::securePlainPhone(string const &phone_)
  : phone_(phone_)
{}

const std::int32_t securePlainPhone::ID;

object_ptr<SecurePlainData> securePlainPhone::fetch(TlBufferParser &p) {
  return make_tl_object<securePlainPhone>(p);
}

securePlainPhone::securePlainPhone(TlBufferParser &p)
  : phone_(TlFetchString<string>::parse(p))
{}

void securePlainPhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_, s);
}

void securePlainPhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_, s);
}

void securePlainPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "securePlainPhone");
    s.store_field("phone", phone_);
    s.store_class_end();
  }
}

securePlainEmail::securePlainEmail(string const &email_)
  : email_(email_)
{}

const std::int32_t securePlainEmail::ID;

object_ptr<SecurePlainData> securePlainEmail::fetch(TlBufferParser &p) {
  return make_tl_object<securePlainEmail>(p);
}

securePlainEmail::securePlainEmail(TlBufferParser &p)
  : email_(TlFetchString<string>::parse(p))
{}

void securePlainEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(email_, s);
}

void securePlainEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(email_, s);
}

void securePlainEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "securePlainEmail");
    s.store_field("email", email_);
    s.store_class_end();
  }
}

object_ptr<SendMessageAction> SendMessageAction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t sendMessageTypingAction::ID;

object_ptr<SendMessageAction> sendMessageTypingAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageTypingAction>();
}

void sendMessageTypingAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageTypingAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageTypingAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageTypingAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageCancelAction::ID;

object_ptr<SendMessageAction> sendMessageCancelAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageCancelAction>();
}

void sendMessageCancelAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageCancelAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageCancelAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageCancelAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageRecordVideoAction::ID;

object_ptr<SendMessageAction> sendMessageRecordVideoAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageRecordVideoAction>();
}

void sendMessageRecordVideoAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageRecordVideoAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageRecordVideoAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageRecordVideoAction");
    s.store_class_end();
  }
}

sendMessageUploadVideoAction::sendMessageUploadVideoAction(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t sendMessageUploadVideoAction::ID;

object_ptr<SendMessageAction> sendMessageUploadVideoAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageUploadVideoAction>(p);
}

sendMessageUploadVideoAction::sendMessageUploadVideoAction(TlBufferParser &p)
  : progress_(TlFetchInt::parse(p))
{}

void sendMessageUploadVideoAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadVideoAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadVideoAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadVideoAction");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

const std::int32_t sendMessageRecordAudioAction::ID;

object_ptr<SendMessageAction> sendMessageRecordAudioAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageRecordAudioAction>();
}

void sendMessageRecordAudioAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageRecordAudioAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageRecordAudioAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageRecordAudioAction");
    s.store_class_end();
  }
}

sendMessageUploadAudioAction::sendMessageUploadAudioAction(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t sendMessageUploadAudioAction::ID;

object_ptr<SendMessageAction> sendMessageUploadAudioAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageUploadAudioAction>(p);
}

sendMessageUploadAudioAction::sendMessageUploadAudioAction(TlBufferParser &p)
  : progress_(TlFetchInt::parse(p))
{}

void sendMessageUploadAudioAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadAudioAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadAudioAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadAudioAction");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

sendMessageUploadPhotoAction::sendMessageUploadPhotoAction(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t sendMessageUploadPhotoAction::ID;

object_ptr<SendMessageAction> sendMessageUploadPhotoAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageUploadPhotoAction>(p);
}

sendMessageUploadPhotoAction::sendMessageUploadPhotoAction(TlBufferParser &p)
  : progress_(TlFetchInt::parse(p))
{}

void sendMessageUploadPhotoAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadPhotoAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadPhotoAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadPhotoAction");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

sendMessageUploadDocumentAction::sendMessageUploadDocumentAction(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t sendMessageUploadDocumentAction::ID;

object_ptr<SendMessageAction> sendMessageUploadDocumentAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageUploadDocumentAction>(p);
}

sendMessageUploadDocumentAction::sendMessageUploadDocumentAction(TlBufferParser &p)
  : progress_(TlFetchInt::parse(p))
{}

void sendMessageUploadDocumentAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadDocumentAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadDocumentAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadDocumentAction");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

const std::int32_t sendMessageGeoLocationAction::ID;

object_ptr<SendMessageAction> sendMessageGeoLocationAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageGeoLocationAction>();
}

void sendMessageGeoLocationAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageGeoLocationAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageGeoLocationAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageGeoLocationAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageChooseContactAction::ID;

object_ptr<SendMessageAction> sendMessageChooseContactAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageChooseContactAction>();
}

void sendMessageChooseContactAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageChooseContactAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageChooseContactAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageChooseContactAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageGamePlayAction::ID;

object_ptr<SendMessageAction> sendMessageGamePlayAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageGamePlayAction>();
}

void sendMessageGamePlayAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageGamePlayAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageGamePlayAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageGamePlayAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageRecordRoundAction::ID;

object_ptr<SendMessageAction> sendMessageRecordRoundAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageRecordRoundAction>();
}

void sendMessageRecordRoundAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageRecordRoundAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageRecordRoundAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageRecordRoundAction");
    s.store_class_end();
  }
}

sendMessageUploadRoundAction::sendMessageUploadRoundAction(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t sendMessageUploadRoundAction::ID;

object_ptr<SendMessageAction> sendMessageUploadRoundAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageUploadRoundAction>(p);
}

sendMessageUploadRoundAction::sendMessageUploadRoundAction(TlBufferParser &p)
  : progress_(TlFetchInt::parse(p))
{}

void sendMessageUploadRoundAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadRoundAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageUploadRoundAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadRoundAction");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

const std::int32_t speakingInGroupCallAction::ID;

object_ptr<SendMessageAction> speakingInGroupCallAction::fetch(TlBufferParser &p) {
  return make_tl_object<speakingInGroupCallAction>();
}

void speakingInGroupCallAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void speakingInGroupCallAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void speakingInGroupCallAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "speakingInGroupCallAction");
    s.store_class_end();
  }
}

sendMessageHistoryImportAction::sendMessageHistoryImportAction(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t sendMessageHistoryImportAction::ID;

object_ptr<SendMessageAction> sendMessageHistoryImportAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageHistoryImportAction>(p);
}

sendMessageHistoryImportAction::sendMessageHistoryImportAction(TlBufferParser &p)
  : progress_(TlFetchInt::parse(p))
{}

void sendMessageHistoryImportAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageHistoryImportAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(progress_, s);
}

void sendMessageHistoryImportAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageHistoryImportAction");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

const std::int32_t sendMessageChooseStickerAction::ID;

object_ptr<SendMessageAction> sendMessageChooseStickerAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageChooseStickerAction>();
}

void sendMessageChooseStickerAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageChooseStickerAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageChooseStickerAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageChooseStickerAction");
    s.store_class_end();
  }
}

sendMessageEmojiInteraction::sendMessageEmojiInteraction(string const &emoticon_, int32 msg_id_, object_ptr<dataJSON> &&interaction_)
  : emoticon_(emoticon_)
  , msg_id_(msg_id_)
  , interaction_(std::move(interaction_))
{}

const std::int32_t sendMessageEmojiInteraction::ID;

object_ptr<SendMessageAction> sendMessageEmojiInteraction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageEmojiInteraction>(p);
}

sendMessageEmojiInteraction::sendMessageEmojiInteraction(TlBufferParser &p)
  : emoticon_(TlFetchString<string>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , interaction_(TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p))
{}

void sendMessageEmojiInteraction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(interaction_, s);
}

void sendMessageEmojiInteraction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(interaction_, s);
}

void sendMessageEmojiInteraction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageEmojiInteraction");
    s.store_field("emoticon", emoticon_);
    s.store_field("msg_id", msg_id_);
    s.store_object_field("interaction", static_cast<const BaseObject *>(interaction_.get()));
    s.store_class_end();
  }
}

sendMessageEmojiInteractionSeen::sendMessageEmojiInteractionSeen(string const &emoticon_)
  : emoticon_(emoticon_)
{}

const std::int32_t sendMessageEmojiInteractionSeen::ID;

object_ptr<SendMessageAction> sendMessageEmojiInteractionSeen::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageEmojiInteractionSeen>(p);
}

sendMessageEmojiInteractionSeen::sendMessageEmojiInteractionSeen(TlBufferParser &p)
  : emoticon_(TlFetchString<string>::parse(p))
{}

void sendMessageEmojiInteractionSeen::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void sendMessageEmojiInteractionSeen::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void sendMessageEmojiInteractionSeen::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageEmojiInteractionSeen");
    s.store_field("emoticon", emoticon_);
    s.store_class_end();
  }
}

sendMessageTextDraftAction::sendMessageTextDraftAction(int64 random_id_, object_ptr<textWithEntities> &&text_)
  : random_id_(random_id_)
  , text_(std::move(text_))
{}

const std::int32_t sendMessageTextDraftAction::ID;

object_ptr<SendMessageAction> sendMessageTextDraftAction::fetch(TlBufferParser &p) {
  return make_tl_object<sendMessageTextDraftAction>(p);
}

sendMessageTextDraftAction::sendMessageTextDraftAction(TlBufferParser &p)
  : random_id_(TlFetchLong::parse(p))
  , text_(TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p))
{}

void sendMessageTextDraftAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(text_, s);
}

void sendMessageTextDraftAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(text_, s);
}

void sendMessageTextDraftAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageTextDraftAction");
    s.store_field("random_id", random_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

stickerSet::stickerSet()
  : flags_()
  , archived_()
  , official_()
  , masks_()
  , emojis_()
  , text_color_()
  , channel_emoji_status_()
  , creator_()
  , installed_date_()
  , id_()
  , access_hash_()
  , title_()
  , short_name_()
  , thumbs_()
  , thumb_dc_id_()
  , thumb_version_()
  , thumb_document_id_()
  , count_()
  , hash_()
{}

const std::int32_t stickerSet::ID;

object_ptr<stickerSet> stickerSet::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stickerSet> res = make_tl_object<stickerSet>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->archived_ = (var0 & 2) != 0;
  res->official_ = (var0 & 4) != 0;
  res->masks_ = (var0 & 8) != 0;
  res->emojis_ = (var0 & 128) != 0;
  res->text_color_ = (var0 & 512) != 0;
  res->channel_emoji_status_ = (var0 & 1024) != 0;
  res->creator_ = (var0 & 2048) != 0;
  if (var0 & 1) { res->installed_date_ = TlFetchInt::parse(p); }
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->short_name_ = TlFetchString<string>::parse(p);
  if (var0 & 16) { res->thumbs_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PhotoSize>>, 481674261>::parse(p); }
  if (var0 & 16) { res->thumb_dc_id_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->thumb_version_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->thumb_document_id_ = TlFetchLong::parse(p); }
  res->count_ = TlFetchInt::parse(p);
  res->hash_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void stickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSet");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (archived_ << 1) | (official_ << 2) | (masks_ << 3) | (emojis_ << 7) | (text_color_ << 9) | (channel_emoji_status_ << 10) | (creator_ << 11)));
    if (var0 & 2) { s.store_field("archived", true); }
    if (var0 & 4) { s.store_field("official", true); }
    if (var0 & 8) { s.store_field("masks", true); }
    if (var0 & 128) { s.store_field("emojis", true); }
    if (var0 & 512) { s.store_field("text_color", true); }
    if (var0 & 1024) { s.store_field("channel_emoji_status", true); }
    if (var0 & 2048) { s.store_field("creator", true); }
    if (var0 & 1) { s.store_field("installed_date", installed_date_); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("title", title_);
    s.store_field("short_name", short_name_);
    if (var0 & 16) { { s.store_vector_begin("thumbs", thumbs_.size()); for (const auto &_value : thumbs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 16) { s.store_field("thumb_dc_id", thumb_dc_id_); }
    if (var0 & 16) { s.store_field("thumb_version", thumb_version_); }
    if (var0 & 256) { s.store_field("thumb_document_id", thumb_document_id_); }
    s.store_field("count", count_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

todoItem::todoItem(int32 id_, object_ptr<textWithEntities> &&title_)
  : id_(id_)
  , title_(std::move(title_))
{}

const std::int32_t todoItem::ID;

object_ptr<todoItem> todoItem::fetch(TlBufferParser &p) {
  return make_tl_object<todoItem>(p);
}

todoItem::todoItem(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
  , title_(TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p))
{}

void todoItem::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
}

void todoItem::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
}

void todoItem::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "todoItem");
    s.store_field("id", id_);
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    s.store_class_end();
  }
}

wallPaperSettings::wallPaperSettings()
  : flags_()
  , blur_()
  , motion_()
  , background_color_()
  , second_background_color_()
  , third_background_color_()
  , fourth_background_color_()
  , intensity_()
  , rotation_()
  , emoticon_()
{}

wallPaperSettings::wallPaperSettings(int32 flags_, bool blur_, bool motion_, int32 background_color_, int32 second_background_color_, int32 third_background_color_, int32 fourth_background_color_, int32 intensity_, int32 rotation_, string const &emoticon_)
  : flags_(flags_)
  , blur_(blur_)
  , motion_(motion_)
  , background_color_(background_color_)
  , second_background_color_(second_background_color_)
  , third_background_color_(third_background_color_)
  , fourth_background_color_(fourth_background_color_)
  , intensity_(intensity_)
  , rotation_(rotation_)
  , emoticon_(emoticon_)
{}

const std::int32_t wallPaperSettings::ID;

object_ptr<wallPaperSettings> wallPaperSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<wallPaperSettings> res = make_tl_object<wallPaperSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->blur_ = (var0 & 2) != 0;
  res->motion_ = (var0 & 4) != 0;
  if (var0 & 1) { res->background_color_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->second_background_color_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->third_background_color_ = TlFetchInt::parse(p); }
  if (var0 & 64) { res->fourth_background_color_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->intensity_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->rotation_ = TlFetchInt::parse(p); }
  if (var0 & 128) { res->emoticon_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void wallPaperSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (blur_ << 1) | (motion_ << 2)), s);
  if (var0 & 1) { TlStoreBinary::store(background_color_, s); }
  if (var0 & 16) { TlStoreBinary::store(second_background_color_, s); }
  if (var0 & 32) { TlStoreBinary::store(third_background_color_, s); }
  if (var0 & 64) { TlStoreBinary::store(fourth_background_color_, s); }
  if (var0 & 8) { TlStoreBinary::store(intensity_, s); }
  if (var0 & 16) { TlStoreBinary::store(rotation_, s); }
  if (var0 & 128) { TlStoreString::store(emoticon_, s); }
}

void wallPaperSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (blur_ << 1) | (motion_ << 2)), s);
  if (var0 & 1) { TlStoreBinary::store(background_color_, s); }
  if (var0 & 16) { TlStoreBinary::store(second_background_color_, s); }
  if (var0 & 32) { TlStoreBinary::store(third_background_color_, s); }
  if (var0 & 64) { TlStoreBinary::store(fourth_background_color_, s); }
  if (var0 & 8) { TlStoreBinary::store(intensity_, s); }
  if (var0 & 16) { TlStoreBinary::store(rotation_, s); }
  if (var0 & 128) { TlStoreString::store(emoticon_, s); }
}

void wallPaperSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "wallPaperSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (blur_ << 1) | (motion_ << 2)));
    if (var0 & 2) { s.store_field("blur", true); }
    if (var0 & 4) { s.store_field("motion", true); }
    if (var0 & 1) { s.store_field("background_color", background_color_); }
    if (var0 & 16) { s.store_field("second_background_color", second_background_color_); }
    if (var0 & 32) { s.store_field("third_background_color", third_background_color_); }
    if (var0 & 64) { s.store_field("fourth_background_color", fourth_background_color_); }
    if (var0 & 8) { s.store_field("intensity", intensity_); }
    if (var0 & 16) { s.store_field("rotation", rotation_); }
    if (var0 & 128) { s.store_field("emoticon", emoticon_); }
    s.store_class_end();
  }
}

object_ptr<WebDocument> WebDocument::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case webDocument::ID:
      return webDocument::fetch(p);
    case webDocumentNoProxy::ID:
      return webDocumentNoProxy::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t webDocument::ID;

object_ptr<WebDocument> webDocument::fetch(TlBufferParser &p) {
  return make_tl_object<webDocument>(p);
}

webDocument::webDocument(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , size_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , attributes_(TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p))
{}

void webDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webDocument");
    s.store_field("url", url_);
    s.store_field("access_hash", access_hash_);
    s.store_field("size", size_);
    s.store_field("mime_type", mime_type_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t webDocumentNoProxy::ID;

object_ptr<WebDocument> webDocumentNoProxy::fetch(TlBufferParser &p) {
  return make_tl_object<webDocumentNoProxy>(p);
}

webDocumentNoProxy::webDocumentNoProxy(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , size_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , attributes_(TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p))
{}

void webDocumentNoProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webDocumentNoProxy");
    s.store_field("url", url_);
    s.store_field("size", size_);
    s.store_field("mime_type", mime_type_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<WebPage> WebPage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case webPageEmpty::ID:
      return webPageEmpty::fetch(p);
    case webPagePending::ID:
      return webPagePending::fetch(p);
    case webPage::ID:
      return webPage::fetch(p);
    case webPageNotModified::ID:
      return webPageNotModified::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

webPageEmpty::webPageEmpty()
  : flags_()
  , id_()
  , url_()
{}

const std::int32_t webPageEmpty::ID;

object_ptr<WebPage> webPageEmpty::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webPageEmpty> res = make_tl_object<webPageEmpty>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->url_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void webPageEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageEmpty");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_field("url", url_); }
    s.store_class_end();
  }
}

webPagePending::webPagePending()
  : flags_()
  , id_()
  , url_()
  , date_()
{}

const std::int32_t webPagePending::ID;

object_ptr<WebPage> webPagePending::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webPagePending> res = make_tl_object<webPagePending>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->url_ = TlFetchString<string>::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void webPagePending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPagePending");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_field("url", url_); }
    s.store_field("date", date_);
    s.store_class_end();
  }
}

webPage::webPage()
  : flags_()
  , has_large_media_()
  , video_cover_photo_()
  , id_()
  , url_()
  , display_url_()
  , hash_()
  , type_()
  , site_name_()
  , title_()
  , description_()
  , photo_()
  , embed_url_()
  , embed_type_()
  , embed_width_()
  , embed_height_()
  , duration_()
  , author_()
  , document_()
  , cached_page_()
  , attributes_()
{}

const std::int32_t webPage::ID;

object_ptr<WebPage> webPage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webPage> res = make_tl_object<webPage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_large_media_ = (var0 & 8192) != 0;
  res->video_cover_photo_ = (var0 & 16384) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->url_ = TlFetchString<string>::parse(p);
  res->display_url_ = TlFetchString<string>::parse(p);
  res->hash_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->type_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->site_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->description_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 32) { res->embed_url_ = TlFetchString<string>::parse(p); }
  if (var0 & 32) { res->embed_type_ = TlFetchString<string>::parse(p); }
  if (var0 & 64) { res->embed_width_ = TlFetchInt::parse(p); }
  if (var0 & 64) { res->embed_height_ = TlFetchInt::parse(p); }
  if (var0 & 128) { res->duration_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->author_ = TlFetchString<string>::parse(p); }
  if (var0 & 512) { res->document_ = TlFetchObject<Document>::parse(p); }
  if (var0 & 1024) { res->cached_page_ = TlFetchBoxed<TlFetchObject<page>, -1738178803>::parse(p); }
  if (var0 & 4096) { res->attributes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<WebPageAttribute>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void webPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_large_media_ << 13) | (video_cover_photo_ << 14)));
    if (var0 & 8192) { s.store_field("has_large_media", true); }
    if (var0 & 16384) { s.store_field("video_cover_photo", true); }
    s.store_field("id", id_);
    s.store_field("url", url_);
    s.store_field("display_url", display_url_);
    s.store_field("hash", hash_);
    if (var0 & 1) { s.store_field("type", type_); }
    if (var0 & 2) { s.store_field("site_name", site_name_); }
    if (var0 & 4) { s.store_field("title", title_); }
    if (var0 & 8) { s.store_field("description", description_); }
    if (var0 & 16) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 32) { s.store_field("embed_url", embed_url_); }
    if (var0 & 32) { s.store_field("embed_type", embed_type_); }
    if (var0 & 64) { s.store_field("embed_width", embed_width_); }
    if (var0 & 64) { s.store_field("embed_height", embed_height_); }
    if (var0 & 128) { s.store_field("duration", duration_); }
    if (var0 & 256) { s.store_field("author", author_); }
    if (var0 & 512) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 1024) { s.store_object_field("cached_page", static_cast<const BaseObject *>(cached_page_.get())); }
    if (var0 & 4096) { { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

webPageNotModified::webPageNotModified()
  : flags_()
  , cached_page_views_()
{}

const std::int32_t webPageNotModified::ID;

object_ptr<WebPage> webPageNotModified::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webPageNotModified> res = make_tl_object<webPageNotModified>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->cached_page_views_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void webPageNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageNotModified");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("cached_page_views", cached_page_views_); }
    s.store_class_end();
  }
}

const std::int32_t account_businessChatLinks::ID;

object_ptr<account_businessChatLinks> account_businessChatLinks::fetch(TlBufferParser &p) {
  return make_tl_object<account_businessChatLinks>(p);
}

account_businessChatLinks::account_businessChatLinks(TlBufferParser &p)
  : links_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<businessChatLink>, -1263638929>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void account_businessChatLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.businessChatLinks");
    { s.store_vector_begin("links", links_.size()); for (const auto &_value : links_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_connectedBots::ID;

object_ptr<account_connectedBots> account_connectedBots::fetch(TlBufferParser &p) {
  return make_tl_object<account_connectedBots>(p);
}

account_connectedBots::account_connectedBots(TlBufferParser &p)
  : connected_bots_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<connectedBot>, -849058964>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void account_connectedBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.connectedBots");
    { s.store_vector_begin("connected_bots", connected_bots_.size()); for (const auto &_value : connected_bots_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_password::account_password()
  : flags_()
  , has_recovery_()
  , has_secure_values_()
  , has_password_()
  , current_algo_()
  , srp_B_()
  , srp_id_()
  , hint_()
  , email_unconfirmed_pattern_()
  , new_algo_()
  , new_secure_algo_()
  , secure_random_()
  , pending_reset_date_()
  , login_email_pattern_()
{}

const std::int32_t account_password::ID;

object_ptr<account_password> account_password::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<account_password> res = make_tl_object<account_password>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_recovery_ = (var0 & 1) != 0;
  res->has_secure_values_ = (var0 & 2) != 0;
  res->has_password_ = (var0 & 4) != 0;
  if (var0 & 4) { res->current_algo_ = TlFetchObject<PasswordKdfAlgo>::parse(p); }
  if (var0 & 4) { res->srp_B_ = TlFetchBytes<bytes>::parse(p); }
  if (var0 & 4) { res->srp_id_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->hint_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->email_unconfirmed_pattern_ = TlFetchString<string>::parse(p); }
  res->new_algo_ = TlFetchObject<PasswordKdfAlgo>::parse(p);
  res->new_secure_algo_ = TlFetchObject<SecurePasswordKdfAlgo>::parse(p);
  res->secure_random_ = TlFetchBytes<bytes>::parse(p);
  if (var0 & 32) { res->pending_reset_date_ = TlFetchInt::parse(p); }
  if (var0 & 64) { res->login_email_pattern_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void account_password::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.password");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_recovery_ << 0) | (has_secure_values_ << 1) | (has_password_ << 2)));
    if (var0 & 1) { s.store_field("has_recovery", true); }
    if (var0 & 2) { s.store_field("has_secure_values", true); }
    if (var0 & 4) { s.store_field("has_password", true); }
    if (var0 & 4) { s.store_object_field("current_algo", static_cast<const BaseObject *>(current_algo_.get())); }
    if (var0 & 4) { s.store_bytes_field("srp_B", srp_B_); }
    if (var0 & 4) { s.store_field("srp_id", srp_id_); }
    if (var0 & 8) { s.store_field("hint", hint_); }
    if (var0 & 16) { s.store_field("email_unconfirmed_pattern", email_unconfirmed_pattern_); }
    s.store_object_field("new_algo", static_cast<const BaseObject *>(new_algo_.get()));
    s.store_object_field("new_secure_algo", static_cast<const BaseObject *>(new_secure_algo_.get()));
    s.store_bytes_field("secure_random", secure_random_);
    if (var0 & 32) { s.store_field("pending_reset_date", pending_reset_date_); }
    if (var0 & 64) { s.store_field("login_email_pattern", login_email_pattern_); }
    s.store_class_end();
  }
}

const std::int32_t bots_previewInfo::ID;

object_ptr<bots_previewInfo> bots_previewInfo::fetch(TlBufferParser &p) {
  return make_tl_object<bots_previewInfo>(p);
}

bots_previewInfo::bots_previewInfo(TlBufferParser &p)
  : media_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<botPreviewMedia>, 602479523>>, 481674261>::parse(p))
  , lang_codes_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
{}

void bots_previewInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.previewInfo");
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("lang_codes", lang_codes_.size()); for (const auto &_value : lang_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<chatlists_ChatlistInvite> chatlists_ChatlistInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatlists_chatlistInviteAlready::ID:
      return chatlists_chatlistInviteAlready::fetch(p);
    case chatlists_chatlistInvite::ID:
      return chatlists_chatlistInvite::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t chatlists_chatlistInviteAlready::ID;

object_ptr<chatlists_ChatlistInvite> chatlists_chatlistInviteAlready::fetch(TlBufferParser &p) {
  return make_tl_object<chatlists_chatlistInviteAlready>(p);
}

chatlists_chatlistInviteAlready::chatlists_chatlistInviteAlready(TlBufferParser &p)
  : filter_id_(TlFetchInt::parse(p))
  , missing_peers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p))
  , already_peers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void chatlists_chatlistInviteAlready::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.chatlistInviteAlready");
    s.store_field("filter_id", filter_id_);
    { s.store_vector_begin("missing_peers", missing_peers_.size()); for (const auto &_value : missing_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("already_peers", already_peers_.size()); for (const auto &_value : already_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatlists_chatlistInvite::chatlists_chatlistInvite()
  : flags_()
  , title_noanimate_()
  , title_()
  , emoticon_()
  , peers_()
  , chats_()
  , users_()
{}

const std::int32_t chatlists_chatlistInvite::ID;

object_ptr<chatlists_ChatlistInvite> chatlists_chatlistInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatlists_chatlistInvite> res = make_tl_object<chatlists_chatlistInvite>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->title_noanimate_ = (var0 & 2) != 0;
  res->title_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p);
  if (var0 & 1) { res->emoticon_ = TlFetchString<string>::parse(p); }
  res->peers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chatlists_chatlistInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.chatlistInvite");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (title_noanimate_ << 1)));
    if (var0 & 2) { s.store_field("title_noanimate", true); }
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    if (var0 & 1) { s.store_field("emoticon", emoticon_); }
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<contacts_Contacts> contacts_Contacts::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case contacts_contactsNotModified::ID:
      return contacts_contactsNotModified::fetch(p);
    case contacts_contacts::ID:
      return contacts_contacts::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t contacts_contactsNotModified::ID;

object_ptr<contacts_Contacts> contacts_contactsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_contactsNotModified>();
}

void contacts_contactsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.contactsNotModified");
    s.store_class_end();
  }
}

const std::int32_t contacts_contacts::ID;

object_ptr<contacts_Contacts> contacts_contacts::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_contacts>(p);
}

contacts_contacts::contacts_contacts(TlBufferParser &p)
  : contacts_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<contact>, 341499403>>, 481674261>::parse(p))
  , saved_count_(TlFetchInt::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_contacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.contacts");
    { s.store_vector_begin("contacts", contacts_.size()); for (const auto &_value : contacts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("saved_count", saved_count_);
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<help_AppConfig> help_AppConfig::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_appConfigNotModified::ID:
      return help_appConfigNotModified::fetch(p);
    case help_appConfig::ID:
      return help_appConfig::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_appConfigNotModified::ID;

object_ptr<help_AppConfig> help_appConfigNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<help_appConfigNotModified>();
}

void help_appConfigNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.appConfigNotModified");
    s.store_class_end();
  }
}

const std::int32_t help_appConfig::ID;

object_ptr<help_AppConfig> help_appConfig::fetch(TlBufferParser &p) {
  return make_tl_object<help_appConfig>(p);
}

help_appConfig::help_appConfig(TlBufferParser &p)
  : hash_(TlFetchInt::parse(p))
  , config_(TlFetchObject<JSONValue>::parse(p))
{}

void help_appConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.appConfig");
    s.store_field("hash", hash_);
    s.store_object_field("config", static_cast<const BaseObject *>(config_.get()));
    s.store_class_end();
  }
}

const std::int32_t messages_affectedFoundMessages::ID;

object_ptr<messages_affectedFoundMessages> messages_affectedFoundMessages::fetch(TlBufferParser &p) {
  return make_tl_object<messages_affectedFoundMessages>(p);
}

messages_affectedFoundMessages::messages_affectedFoundMessages(TlBufferParser &p)
  : pts_(TlFetchInt::parse(p))
  , pts_count_(TlFetchInt::parse(p))
  , offset_(TlFetchInt::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void messages_affectedFoundMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.affectedFoundMessages");
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_field("offset", offset_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_archivedStickers::ID;

object_ptr<messages_archivedStickers> messages_archivedStickers::fetch(TlBufferParser &p) {
  return make_tl_object<messages_archivedStickers>(p);
}

messages_archivedStickers::messages_archivedStickers(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , sets_(TlFetchBoxed<TlFetchVector<TlFetchObject<StickerSetCovered>>, 481674261>::parse(p))
{}

void messages_archivedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.archivedStickers");
    s.store_field("count", count_);
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_botApp::messages_botApp()
  : flags_()
  , inactive_()
  , request_write_access_()
  , has_settings_()
  , app_()
{}

const std::int32_t messages_botApp::ID;

object_ptr<messages_botApp> messages_botApp::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_botApp> res = make_tl_object<messages_botApp>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->inactive_ = (var0 & 1) != 0;
  res->request_write_access_ = (var0 & 2) != 0;
  res->has_settings_ = (var0 & 4) != 0;
  res->app_ = TlFetchObject<BotApp>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_botApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.botApp");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (inactive_ << 0) | (request_write_access_ << 1) | (has_settings_ << 2)));
    if (var0 & 1) { s.store_field("inactive", true); }
    if (var0 & 2) { s.store_field("request_write_access", true); }
    if (var0 & 4) { s.store_field("has_settings", true); }
    s.store_object_field("app", static_cast<const BaseObject *>(app_.get()));
    s.store_class_end();
  }
}

const std::int32_t messages_checkedHistoryImportPeer::ID;

object_ptr<messages_checkedHistoryImportPeer> messages_checkedHistoryImportPeer::fetch(TlBufferParser &p) {
  return make_tl_object<messages_checkedHistoryImportPeer>(p);
}

messages_checkedHistoryImportPeer::messages_checkedHistoryImportPeer(TlBufferParser &p)
  : confirm_text_(TlFetchString<string>::parse(p))
{}

void messages_checkedHistoryImportPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.checkedHistoryImportPeer");
    s.store_field("confirm_text", confirm_text_);
    s.store_class_end();
  }
}

object_ptr<messages_FoundStickers> messages_FoundStickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_foundStickersNotModified::ID:
      return messages_foundStickersNotModified::fetch(p);
    case messages_foundStickers::ID:
      return messages_foundStickers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

messages_foundStickersNotModified::messages_foundStickersNotModified()
  : flags_()
  , next_offset_()
{}

const std::int32_t messages_foundStickersNotModified::ID;

object_ptr<messages_FoundStickers> messages_foundStickersNotModified::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_foundStickersNotModified> res = make_tl_object<messages_foundStickersNotModified>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->next_offset_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messages_foundStickersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.foundStickersNotModified");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    s.store_class_end();
  }
}

messages_foundStickers::messages_foundStickers()
  : flags_()
  , next_offset_()
  , hash_()
  , stickers_()
{}

const std::int32_t messages_foundStickers::ID;

object_ptr<messages_FoundStickers> messages_foundStickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_foundStickers> res = make_tl_object<messages_foundStickers>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->next_offset_ = TlFetchInt::parse(p); }
  res->hash_ = TlFetchLong::parse(p);
  res->stickers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messages_foundStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.foundStickers");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    s.store_field("hash", hash_);
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_searchCounter::messages_searchCounter()
  : flags_()
  , inexact_()
  , filter_()
  , count_()
{}

const std::int32_t messages_searchCounter::ID;

object_ptr<messages_searchCounter> messages_searchCounter::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_searchCounter> res = make_tl_object<messages_searchCounter>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->inexact_ = (var0 & 2) != 0;
  res->filter_ = TlFetchObject<MessagesFilter>::parse(p);
  res->count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_searchCounter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchCounter");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (inexact_ << 1)));
    if (var0 & 2) { s.store_field("inexact", true); }
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("count", count_);
    s.store_class_end();
  }
}

stats_publicForwards::stats_publicForwards()
  : flags_()
  , count_()
  , forwards_()
  , next_offset_()
  , chats_()
  , users_()
{}

const std::int32_t stats_publicForwards::ID;

object_ptr<stats_publicForwards> stats_publicForwards::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<stats_publicForwards> res = make_tl_object<stats_publicForwards>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->forwards_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PublicForward>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void stats_publicForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.publicForwards");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("forwards", forwards_.size()); for (const auto &_value : forwards_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t stories_peerStories::ID;

object_ptr<stories_peerStories> stories_peerStories::fetch(TlBufferParser &p) {
  return make_tl_object<stories_peerStories>(p);
}

stories_peerStories::stories_peerStories(TlBufferParser &p)
  : stories_(TlFetchBoxed<TlFetchObject<peerStories>, -1707742823>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void stories_peerStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.peerStories");
    s.store_object_field("stories", static_cast<const BaseObject *>(stories_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<updates_ChannelDifference> updates_ChannelDifference::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case updates_channelDifferenceEmpty::ID:
      return updates_channelDifferenceEmpty::fetch(p);
    case updates_channelDifferenceTooLong::ID:
      return updates_channelDifferenceTooLong::fetch(p);
    case updates_channelDifference::ID:
      return updates_channelDifference::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

updates_channelDifferenceEmpty::updates_channelDifferenceEmpty()
  : flags_()
  , final_()
  , pts_()
  , timeout_()
{}

const std::int32_t updates_channelDifferenceEmpty::ID;

object_ptr<updates_ChannelDifference> updates_channelDifferenceEmpty::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updates_channelDifferenceEmpty> res = make_tl_object<updates_channelDifferenceEmpty>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->final_ = (var0 & 1) != 0;
  res->pts_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->timeout_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updates_channelDifferenceEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.channelDifferenceEmpty");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (final_ << 0)));
    if (var0 & 1) { s.store_field("final", true); }
    s.store_field("pts", pts_);
    if (var0 & 2) { s.store_field("timeout", timeout_); }
    s.store_class_end();
  }
}

updates_channelDifferenceTooLong::updates_channelDifferenceTooLong()
  : flags_()
  , final_()
  , timeout_()
  , dialog_()
  , messages_()
  , chats_()
  , users_()
{}

const std::int32_t updates_channelDifferenceTooLong::ID;

object_ptr<updates_ChannelDifference> updates_channelDifferenceTooLong::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updates_channelDifferenceTooLong> res = make_tl_object<updates_channelDifferenceTooLong>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->final_ = (var0 & 1) != 0;
  if (var0 & 2) { res->timeout_ = TlFetchInt::parse(p); }
  res->dialog_ = TlFetchObject<Dialog>::parse(p);
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updates_channelDifferenceTooLong::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.channelDifferenceTooLong");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (final_ << 0)));
    if (var0 & 1) { s.store_field("final", true); }
    if (var0 & 2) { s.store_field("timeout", timeout_); }
    s.store_object_field("dialog", static_cast<const BaseObject *>(dialog_.get()));
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updates_channelDifference::updates_channelDifference()
  : flags_()
  , final_()
  , pts_()
  , timeout_()
  , new_messages_()
  , other_updates_()
  , chats_()
  , users_()
{}

const std::int32_t updates_channelDifference::ID;

object_ptr<updates_ChannelDifference> updates_channelDifference::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updates_channelDifference> res = make_tl_object<updates_channelDifference>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->final_ = (var0 & 1) != 0;
  res->pts_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->timeout_ = TlFetchInt::parse(p); }
  res->new_messages_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p);
  res->other_updates_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Update>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updates_channelDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.channelDifference");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (final_ << 0)));
    if (var0 & 1) { s.store_field("final", true); }
    s.store_field("pts", pts_);
    if (var0 & 2) { s.store_field("timeout", timeout_); }
    { s.store_vector_begin("new_messages", new_messages_.size()); for (const auto &_value : new_messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("other_updates", other_updates_.size()); for (const auto &_value : other_updates_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_deleteBusinessChatLink::account_deleteBusinessChatLink(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t account_deleteBusinessChatLink::ID;

void account_deleteBusinessChatLink::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1611085428);
  TlStoreString::store(slug_, s);
}

void account_deleteBusinessChatLink::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1611085428);
  TlStoreString::store(slug_, s);
}

void account_deleteBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.deleteBusinessChatLink");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

account_deleteBusinessChatLink::ReturnType account_deleteBusinessChatLink::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_deleteSecureValue::account_deleteSecureValue(array<object_ptr<SecureValueType>> &&types_)
  : types_(std::move(types_))
{}

const std::int32_t account_deleteSecureValue::ID;

void account_deleteSecureValue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1199522741);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(types_, s);
}

void account_deleteSecureValue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1199522741);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(types_, s);
}

void account_deleteSecureValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.deleteSecureValue");
    { s.store_vector_begin("types", types_.size()); for (const auto &_value : types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_deleteSecureValue::ReturnType account_deleteSecureValue::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_getBotBusinessConnection::account_getBotBusinessConnection(string const &connection_id_)
  : connection_id_(connection_id_)
{}

const std::int32_t account_getBotBusinessConnection::ID;

void account_getBotBusinessConnection::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1990746736);
  TlStoreString::store(connection_id_, s);
}

void account_getBotBusinessConnection::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1990746736);
  TlStoreString::store(connection_id_, s);
}

void account_getBotBusinessConnection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getBotBusinessConnection");
    s.store_field("connection_id", connection_id_);
    s.store_class_end();
  }
}

account_getBotBusinessConnection::ReturnType account_getBotBusinessConnection::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

account_getDefaultGroupPhotoEmojis::account_getDefaultGroupPhotoEmojis(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getDefaultGroupPhotoEmojis::ID;

void account_getDefaultGroupPhotoEmojis::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1856479058);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultGroupPhotoEmojis::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1856479058);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultGroupPhotoEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getDefaultGroupPhotoEmojis");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getDefaultGroupPhotoEmojis::ReturnType account_getDefaultGroupPhotoEmojis::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EmojiList>::parse(p);
#undef FAIL
}

account_getDefaultProfilePhotoEmojis::account_getDefaultProfilePhotoEmojis(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getDefaultProfilePhotoEmojis::ID;

void account_getDefaultProfilePhotoEmojis::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-495647960);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultProfilePhotoEmojis::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-495647960);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultProfilePhotoEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getDefaultProfilePhotoEmojis");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getDefaultProfilePhotoEmojis::ReturnType account_getDefaultProfilePhotoEmojis::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EmojiList>::parse(p);
#undef FAIL
}

account_saveSecureValue::account_saveSecureValue(object_ptr<inputSecureValue> &&value_, int64 secure_secret_id_)
  : value_(std::move(value_))
  , secure_secret_id_(secure_secret_id_)
{}

const std::int32_t account_saveSecureValue::ID;

void account_saveSecureValue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1986010339);
  TlStoreBoxed<TlStoreObject, -618540889>::store(value_, s);
  TlStoreBinary::store(secure_secret_id_, s);
}

void account_saveSecureValue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1986010339);
  TlStoreBoxed<TlStoreObject, -618540889>::store(value_, s);
  TlStoreBinary::store(secure_secret_id_, s);
}

void account_saveSecureValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.saveSecureValue");
    s.store_object_field("value", static_cast<const BaseObject *>(value_.get()));
    s.store_field("secure_secret_id", secure_secret_id_);
    s.store_class_end();
  }
}

account_saveSecureValue::ReturnType account_saveSecureValue::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<secureValue>, 411017418>::parse(p);
#undef FAIL
}

account_sendChangePhoneCode::account_sendChangePhoneCode(string const &phone_number_, object_ptr<codeSettings> &&settings_)
  : phone_number_(phone_number_)
  , settings_(std::move(settings_))
{}

const std::int32_t account_sendChangePhoneCode::ID;

void account_sendChangePhoneCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2108208411);
  TlStoreString::store(phone_number_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void account_sendChangePhoneCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2108208411);
  TlStoreString::store(phone_number_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void account_sendChangePhoneCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.sendChangePhoneCode");
    s.store_field("phone_number", phone_number_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_sendChangePhoneCode::ReturnType account_sendChangePhoneCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_SentCode>::parse(p);
#undef FAIL
}

account_sendConfirmPhoneCode::account_sendConfirmPhoneCode(string const &hash_, object_ptr<codeSettings> &&settings_)
  : hash_(hash_)
  , settings_(std::move(settings_))
{}

const std::int32_t account_sendConfirmPhoneCode::ID;

void account_sendConfirmPhoneCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(457157256);
  TlStoreString::store(hash_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void account_sendConfirmPhoneCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(457157256);
  TlStoreString::store(hash_, s);
  TlStoreBoxed<TlStoreObject, -1390068360>::store(settings_, s);
}

void account_sendConfirmPhoneCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.sendConfirmPhoneCode");
    s.store_field("hash", hash_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_sendConfirmPhoneCode::ReturnType account_sendConfirmPhoneCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_SentCode>::parse(p);
#undef FAIL
}

account_setAccountTTL::account_setAccountTTL(object_ptr<accountDaysTTL> &&ttl_)
  : ttl_(std::move(ttl_))
{}

const std::int32_t account_setAccountTTL::ID;

void account_setAccountTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(608323678);
  TlStoreBoxed<TlStoreObject, -1194283041>::store(ttl_, s);
}

void account_setAccountTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(608323678);
  TlStoreBoxed<TlStoreObject, -1194283041>::store(ttl_, s);
}

void account_setAccountTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setAccountTTL");
    s.store_object_field("ttl", static_cast<const BaseObject *>(ttl_.get()));
    s.store_class_end();
  }
}

account_setAccountTTL::ReturnType account_setAccountTTL::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateConnectedBot::account_updateConnectedBot(int32 flags_, bool deleted_, object_ptr<businessBotRights> &&rights_, object_ptr<InputUser> &&bot_, object_ptr<inputBusinessBotRecipients> &&recipients_)
  : flags_(flags_)
  , deleted_(deleted_)
  , rights_(std::move(rights_))
  , bot_(std::move(bot_))
  , recipients_(std::move(recipients_))
{}

const std::int32_t account_updateConnectedBot::ID;

void account_updateConnectedBot::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1721797758);
  TlStoreBinary::store((var0 = flags_ | (deleted_ << 1)), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1604170505>::store(rights_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxed<TlStoreObject, -991587810>::store(recipients_, s);
}

void account_updateConnectedBot::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1721797758);
  TlStoreBinary::store((var0 = flags_ | (deleted_ << 1)), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1604170505>::store(rights_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxed<TlStoreObject, -991587810>::store(recipients_, s);
}

void account_updateConnectedBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateConnectedBot");
    s.store_field("flags", (var0 = flags_ | (deleted_ << 1)));
    if (var0 & 2) { s.store_field("deleted", true); }
    if (var0 & 1) { s.store_object_field("rights", static_cast<const BaseObject *>(rights_.get())); }
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_class_end();
  }
}

account_updateConnectedBot::ReturnType account_updateConnectedBot::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

account_updateDeviceLocked::account_updateDeviceLocked(int32 period_)
  : period_(period_)
{}

const std::int32_t account_updateDeviceLocked::ID;

void account_updateDeviceLocked::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(954152242);
  TlStoreBinary::store(period_, s);
}

void account_updateDeviceLocked::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(954152242);
  TlStoreBinary::store(period_, s);
}

void account_updateDeviceLocked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateDeviceLocked");
    s.store_field("period", period_);
    s.store_class_end();
  }
}

account_updateDeviceLocked::ReturnType account_updateDeviceLocked::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateEmojiStatus::account_updateEmojiStatus(object_ptr<EmojiStatus> &&emoji_status_)
  : emoji_status_(std::move(emoji_status_))
{}

const std::int32_t account_updateEmojiStatus::ID;

void account_updateEmojiStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-70001045);
  TlStoreBoxedUnknown<TlStoreObject>::store(emoji_status_, s);
}

void account_updateEmojiStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-70001045);
  TlStoreBoxedUnknown<TlStoreObject>::store(emoji_status_, s);
}

void account_updateEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateEmojiStatus");
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

account_updateEmojiStatus::ReturnType account_updateEmojiStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updatePasswordSettings::account_updatePasswordSettings(object_ptr<InputCheckPasswordSRP> &&password_, object_ptr<account_passwordInputSettings> &&new_settings_)
  : password_(std::move(password_))
  , new_settings_(std::move(new_settings_))
{}

const std::int32_t account_updatePasswordSettings::ID;

void account_updatePasswordSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1516564433);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
  TlStoreBoxed<TlStoreObject, -1036572727>::store(new_settings_, s);
}

void account_updatePasswordSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1516564433);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
  TlStoreBoxed<TlStoreObject, -1036572727>::store(new_settings_, s);
}

void account_updatePasswordSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updatePasswordSettings");
    s.store_object_field("password", static_cast<const BaseObject *>(password_.get()));
    s.store_object_field("new_settings", static_cast<const BaseObject *>(new_settings_.get()));
    s.store_class_end();
  }
}

account_updatePasswordSettings::ReturnType account_updatePasswordSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_acceptLoginToken::auth_acceptLoginToken(bytes &&token_)
  : token_(std::move(token_))
{}

const std::int32_t auth_acceptLoginToken::ID;

void auth_acceptLoginToken::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-392909491);
  TlStoreString::store(token_, s);
}

void auth_acceptLoginToken::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-392909491);
  TlStoreString::store(token_, s);
}

void auth_acceptLoginToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.acceptLoginToken");
    s.store_bytes_field("token", token_);
    s.store_class_end();
  }
}

auth_acceptLoginToken::ReturnType auth_acceptLoginToken::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<authorization>, -1392388579>::parse(p);
#undef FAIL
}

auth_bindTempAuthKey::auth_bindTempAuthKey(int64 perm_auth_key_id_, int64 nonce_, int32 expires_at_, bytes &&encrypted_message_)
  : perm_auth_key_id_(perm_auth_key_id_)
  , nonce_(nonce_)
  , expires_at_(expires_at_)
  , encrypted_message_(std::move(encrypted_message_))
{}

const std::int32_t auth_bindTempAuthKey::ID;

void auth_bindTempAuthKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-841733627);
  TlStoreBinary::store(perm_auth_key_id_, s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(expires_at_, s);
  TlStoreString::store(encrypted_message_, s);
}

void auth_bindTempAuthKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-841733627);
  TlStoreBinary::store(perm_auth_key_id_, s);
  TlStoreBinary::store(nonce_, s);
  TlStoreBinary::store(expires_at_, s);
  TlStoreString::store(encrypted_message_, s);
}

void auth_bindTempAuthKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.bindTempAuthKey");
    s.store_field("perm_auth_key_id", perm_auth_key_id_);
    s.store_field("nonce", nonce_);
    s.store_field("expires_at", expires_at_);
    s.store_bytes_field("encrypted_message", encrypted_message_);
    s.store_class_end();
  }
}

auth_bindTempAuthKey::ReturnType auth_bindTempAuthKey::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

auth_exportLoginToken::auth_exportLoginToken(int32 api_id_, string const &api_hash_, array<int64> &&except_ids_)
  : api_id_(api_id_)
  , api_hash_(api_hash_)
  , except_ids_(std::move(except_ids_))
{}

const std::int32_t auth_exportLoginToken::ID;

void auth_exportLoginToken::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1210022402);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(except_ids_, s);
}

void auth_exportLoginToken::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1210022402);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(except_ids_, s);
}

void auth_exportLoginToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.exportLoginToken");
    s.store_field("api_id", api_id_);
    s.store_field("api_hash", api_hash_);
    { s.store_vector_begin("except_ids", except_ids_.size()); for (const auto &_value : except_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

auth_exportLoginToken::ReturnType auth_exportLoginToken::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_LoginToken>::parse(p);
#undef FAIL
}

auth_resendCode::auth_resendCode(int32 flags_, string const &phone_number_, string const &phone_code_hash_, string const &reason_)
  : flags_(flags_)
  , phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , reason_(reason_)
{}

const std::int32_t auth_resendCode::ID;

void auth_resendCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-890997469);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  if (var0 & 1) { TlStoreString::store(reason_, s); }
}

void auth_resendCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-890997469);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  if (var0 & 1) { TlStoreString::store(reason_, s); }
}

void auth_resendCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.resendCode");
    s.store_field("flags", (var0 = flags_));
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    if (var0 & 1) { s.store_field("reason", reason_); }
    s.store_class_end();
  }
}

auth_resendCode::ReturnType auth_resendCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_SentCode>::parse(p);
#undef FAIL
}

bots_getPopularAppBots::bots_getPopularAppBots(string const &offset_, int32 limit_)
  : offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t bots_getPopularAppBots::ID;

void bots_getPopularAppBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1034878574);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void bots_getPopularAppBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1034878574);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void bots_getPopularAppBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getPopularAppBots");
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

bots_getPopularAppBots::ReturnType bots_getPopularAppBots::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<bots_popularAppBots>, 428978491>::parse(p);
#undef FAIL
}

bots_sendCustomRequest::bots_sendCustomRequest(string const &custom_method_, object_ptr<dataJSON> &&params_)
  : custom_method_(custom_method_)
  , params_(std::move(params_))
{}

const std::int32_t bots_sendCustomRequest::ID;

void bots_sendCustomRequest::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1440257555);
  TlStoreString::store(custom_method_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void bots_sendCustomRequest::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1440257555);
  TlStoreString::store(custom_method_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void bots_sendCustomRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.sendCustomRequest");
    s.store_field("custom_method", custom_method_);
    s.store_object_field("params", static_cast<const BaseObject *>(params_.get()));
    s.store_class_end();
  }
}

bots_sendCustomRequest::ReturnType bots_sendCustomRequest::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p);
#undef FAIL
}

channels_deleteParticipantHistory::channels_deleteParticipantHistory(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_)
  : channel_(std::move(channel_))
  , participant_(std::move(participant_))
{}

const std::int32_t channels_deleteParticipantHistory::ID;

void channels_deleteParticipantHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(913655003);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
}

void channels_deleteParticipantHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(913655003);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
}

void channels_deleteParticipantHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.deleteParticipantHistory");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_class_end();
  }
}

channels_deleteParticipantHistory::ReturnType channels_deleteParticipantHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedHistory>, -1269012015>::parse(p);
#undef FAIL
}

channels_editBanned::channels_editBanned(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_, object_ptr<chatBannedRights> &&banned_rights_)
  : channel_(std::move(channel_))
  , participant_(std::move(participant_))
  , banned_rights_(std::move(banned_rights_))
{}

const std::int32_t channels_editBanned::ID;

void channels_editBanned::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1763259007);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
  TlStoreBoxed<TlStoreObject, -1626209256>::store(banned_rights_, s);
}

void channels_editBanned::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1763259007);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
  TlStoreBoxed<TlStoreObject, -1626209256>::store(banned_rights_, s);
}

void channels_editBanned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.editBanned");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_object_field("banned_rights", static_cast<const BaseObject *>(banned_rights_.get()));
    s.store_class_end();
  }
}

channels_editBanned::ReturnType channels_editBanned::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_readHistory::channels_readHistory(object_ptr<InputChannel> &&channel_, int32 max_id_)
  : channel_(std::move(channel_))
  , max_id_(max_id_)
{}

const std::int32_t channels_readHistory::ID;

void channels_readHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-871347913);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(max_id_, s);
}

void channels_readHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-871347913);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(max_id_, s);
}

void channels_readHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.readHistory");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

channels_readHistory::ReturnType channels_readHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_reorderUsernames::channels_reorderUsernames(object_ptr<InputChannel> &&channel_, array<string> &&order_)
  : channel_(std::move(channel_))
  , order_(std::move(order_))
{}

const std::int32_t channels_reorderUsernames::ID;

void channels_reorderUsernames::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1268978403);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(order_, s);
}

void channels_reorderUsernames::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1268978403);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(order_, s);
}

void channels_reorderUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.reorderUsernames");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

channels_reorderUsernames::ReturnType channels_reorderUsernames::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_updatePaidMessagesPrice::channels_updatePaidMessagesPrice(int32 flags_, bool broadcast_messages_allowed_, object_ptr<InputChannel> &&channel_, int64 send_paid_messages_stars_)
  : flags_(flags_)
  , broadcast_messages_allowed_(broadcast_messages_allowed_)
  , channel_(std::move(channel_))
  , send_paid_messages_stars_(send_paid_messages_stars_)
{}

const std::int32_t channels_updatePaidMessagesPrice::ID;

void channels_updatePaidMessagesPrice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1259483771);
  TlStoreBinary::store((var0 = flags_ | (broadcast_messages_allowed_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(send_paid_messages_stars_, s);
}

void channels_updatePaidMessagesPrice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1259483771);
  TlStoreBinary::store((var0 = flags_ | (broadcast_messages_allowed_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(send_paid_messages_stars_, s);
}

void channels_updatePaidMessagesPrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.updatePaidMessagesPrice");
    s.store_field("flags", (var0 = flags_ | (broadcast_messages_allowed_ << 0)));
    if (var0 & 1) { s.store_field("broadcast_messages_allowed", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("send_paid_messages_stars", send_paid_messages_stars_);
    s.store_class_end();
  }
}

channels_updatePaidMessagesPrice::ReturnType channels_updatePaidMessagesPrice::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

chatlists_exportChatlistInvite::chatlists_exportChatlistInvite(object_ptr<inputChatlistDialogFilter> &&chatlist_, string const &title_, array<object_ptr<InputPeer>> &&peers_)
  : chatlist_(std::move(chatlist_))
  , title_(title_)
  , peers_(std::move(peers_))
{}

const std::int32_t chatlists_exportChatlistInvite::ID;

void chatlists_exportChatlistInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2072885362);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_exportChatlistInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2072885362);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
  TlStoreString::store(title_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peers_, s);
}

void chatlists_exportChatlistInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.exportChatlistInvite");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    s.store_field("title", title_);
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatlists_exportChatlistInvite::ReturnType chatlists_exportChatlistInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<chatlists_exportedChatlistInvite>, 283567014>::parse(p);
#undef FAIL
}

chatlists_getExportedInvites::chatlists_getExportedInvites(object_ptr<inputChatlistDialogFilter> &&chatlist_)
  : chatlist_(std::move(chatlist_))
{}

const std::int32_t chatlists_getExportedInvites::ID;

void chatlists_getExportedInvites::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-838608253);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_getExportedInvites::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-838608253);
  TlStoreBoxed<TlStoreObject, -203367885>::store(chatlist_, s);
}

void chatlists_getExportedInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.getExportedInvites");
    s.store_object_field("chatlist", static_cast<const BaseObject *>(chatlist_.get()));
    s.store_class_end();
  }
}

chatlists_getExportedInvites::ReturnType chatlists_getExportedInvites::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<chatlists_exportedInvites>, 279670215>::parse(p);
#undef FAIL
}

contacts_addContact::contacts_addContact(int32 flags_, bool add_phone_privacy_exception_, object_ptr<InputUser> &&id_, string const &first_name_, string const &last_name_, string const &phone_, object_ptr<textWithEntities> &&note_)
  : flags_(flags_)
  , add_phone_privacy_exception_(add_phone_privacy_exception_)
  , id_(std::move(id_))
  , first_name_(first_name_)
  , last_name_(last_name_)
  , phone_(phone_)
  , note_(std::move(note_))
{}

const std::int32_t contacts_addContact::ID;

void contacts_addContact::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-642109868);
  TlStoreBinary::store((var0 = flags_ | (add_phone_privacy_exception_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreString::store(phone_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1964978502>::store(note_, s); }
}

void contacts_addContact::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-642109868);
  TlStoreBinary::store((var0 = flags_ | (add_phone_privacy_exception_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreString::store(phone_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1964978502>::store(note_, s); }
}

void contacts_addContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.addContact");
    s.store_field("flags", (var0 = flags_ | (add_phone_privacy_exception_ << 0)));
    if (var0 & 1) { s.store_field("add_phone_privacy_exception", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("phone", phone_);
    if (var0 & 2) { s.store_object_field("note", static_cast<const BaseObject *>(note_.get())); }
    s.store_class_end();
  }
}

contacts_addContact::ReturnType contacts_addContact::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_blockFromReplies::contacts_blockFromReplies(int32 flags_, bool delete_message_, bool delete_history_, bool report_spam_, int32 msg_id_)
  : flags_(flags_)
  , delete_message_(delete_message_)
  , delete_history_(delete_history_)
  , report_spam_(report_spam_)
  , msg_id_(msg_id_)
{}

const std::int32_t contacts_blockFromReplies::ID;

void contacts_blockFromReplies::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(698914348);
  TlStoreBinary::store((var0 = flags_ | (delete_message_ << 0) | (delete_history_ << 1) | (report_spam_ << 2)), s);
  TlStoreBinary::store(msg_id_, s);
}

void contacts_blockFromReplies::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(698914348);
  TlStoreBinary::store((var0 = flags_ | (delete_message_ << 0) | (delete_history_ << 1) | (report_spam_ << 2)), s);
  TlStoreBinary::store(msg_id_, s);
}

void contacts_blockFromReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.blockFromReplies");
    s.store_field("flags", (var0 = flags_ | (delete_message_ << 0) | (delete_history_ << 1) | (report_spam_ << 2)));
    if (var0 & 1) { s.store_field("delete_message", true); }
    if (var0 & 2) { s.store_field("delete_history", true); }
    if (var0 & 4) { s.store_field("report_spam", true); }
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

contacts_blockFromReplies::ReturnType contacts_blockFromReplies::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_resetTopPeerRating::contacts_resetTopPeerRating(object_ptr<TopPeerCategory> &&category_, object_ptr<InputPeer> &&peer_)
  : category_(std::move(category_))
  , peer_(std::move(peer_))
{}

const std::int32_t contacts_resetTopPeerRating::ID;

void contacts_resetTopPeerRating::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(451113900);
  TlStoreBoxedUnknown<TlStoreObject>::store(category_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void contacts_resetTopPeerRating::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(451113900);
  TlStoreBoxedUnknown<TlStoreObject>::store(category_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void contacts_resetTopPeerRating::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.resetTopPeerRating");
    s.store_object_field("category", static_cast<const BaseObject *>(category_.get()));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

contacts_resetTopPeerRating::ReturnType contacts_resetTopPeerRating::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

folders_editPeerFolders::folders_editPeerFolders(array<object_ptr<inputFolderPeer>> &&folder_peers_)
  : folder_peers_(std::move(folder_peers_))
{}

const std::int32_t folders_editPeerFolders::ID;

void folders_editPeerFolders::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1749536939);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -70073706>>, 481674261>::store(folder_peers_, s);
}

void folders_editPeerFolders::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1749536939);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -70073706>>, 481674261>::store(folder_peers_, s);
}

void folders_editPeerFolders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "folders.editPeerFolders");
    { s.store_vector_begin("folder_peers", folder_peers_.size()); for (const auto &_value : folder_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

folders_editPeerFolders::ReturnType folders_editPeerFolders::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

fragment_getCollectibleInfo::fragment_getCollectibleInfo(object_ptr<InputCollectible> &&collectible_)
  : collectible_(std::move(collectible_))
{}

const std::int32_t fragment_getCollectibleInfo::ID;

void fragment_getCollectibleInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1105295942);
  TlStoreBoxedUnknown<TlStoreObject>::store(collectible_, s);
}

void fragment_getCollectibleInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1105295942);
  TlStoreBoxedUnknown<TlStoreObject>::store(collectible_, s);
}

void fragment_getCollectibleInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fragment.getCollectibleInfo");
    s.store_object_field("collectible", static_cast<const BaseObject *>(collectible_.get()));
    s.store_class_end();
  }
}

fragment_getCollectibleInfo::ReturnType fragment_getCollectibleInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<fragment_collectibleInfo>, 1857945489>::parse(p);
#undef FAIL
}

help_editUserInfo::help_editUserInfo(object_ptr<InputUser> &&user_id_, string const &message_, array<object_ptr<MessageEntity>> &&entities_)
  : user_id_(std::move(user_id_))
  , message_(message_)
  , entities_(std::move(entities_))
{}

const std::int32_t help_editUserInfo::ID;

void help_editUserInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1723407216);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(message_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s);
}

void help_editUserInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1723407216);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreString::store(message_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s);
}

void help_editUserInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.editUserInfo");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("message", message_);
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

help_editUserInfo::ReturnType help_editUserInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_UserInfo>::parse(p);
#undef FAIL
}

help_getPeerProfileColors::help_getPeerProfileColors(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t help_getPeerProfileColors::ID;

void help_getPeerProfileColors::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1412453891);
  TlStoreBinary::store(hash_, s);
}

void help_getPeerProfileColors::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1412453891);
  TlStoreBinary::store(hash_, s);
}

void help_getPeerProfileColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getPeerProfileColors");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

help_getPeerProfileColors::ReturnType help_getPeerProfileColors::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_PeerColors>::parse(p);
#undef FAIL
}

const std::int32_t help_getPromoData::ID;

void help_getPromoData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1063816159);
}

void help_getPromoData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1063816159);
}

void help_getPromoData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getPromoData");
    s.store_class_end();
  }
}

help_getPromoData::ReturnType help_getPromoData::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_PromoData>::parse(p);
#undef FAIL
}

help_getUserInfo::help_getUserInfo(object_ptr<InputUser> &&user_id_)
  : user_id_(std::move(user_id_))
{}

const std::int32_t help_getUserInfo::ID;

void help_getUserInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(59377875);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void help_getUserInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(59377875);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void help_getUserInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getUserInfo");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

help_getUserInfo::ReturnType help_getUserInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_UserInfo>::parse(p);
#undef FAIL
}

messages_clearRecentStickers::messages_clearRecentStickers(int32 flags_, bool attached_)
  : flags_(flags_)
  , attached_(attached_)
{}

const std::int32_t messages_clearRecentStickers::ID;

void messages_clearRecentStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1986437075);
  TlStoreBinary::store((var0 = flags_ | (attached_ << 0)), s);
}

void messages_clearRecentStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1986437075);
  TlStoreBinary::store((var0 = flags_ | (attached_ << 0)), s);
}

void messages_clearRecentStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.clearRecentStickers");
    s.store_field("flags", (var0 = flags_ | (attached_ << 0)));
    if (var0 & 1) { s.store_field("attached", true); }
    s.store_class_end();
  }
}

messages_clearRecentStickers::ReturnType messages_clearRecentStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_clickSponsoredMessage::messages_clickSponsoredMessage(int32 flags_, bool media_, bool fullscreen_, bytes &&random_id_)
  : flags_(flags_)
  , media_(media_)
  , fullscreen_(fullscreen_)
  , random_id_(std::move(random_id_))
{}

const std::int32_t messages_clickSponsoredMessage::ID;

void messages_clickSponsoredMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2110454402);
  TlStoreBinary::store((var0 = flags_ | (media_ << 0) | (fullscreen_ << 1)), s);
  TlStoreString::store(random_id_, s);
}

void messages_clickSponsoredMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2110454402);
  TlStoreBinary::store((var0 = flags_ | (media_ << 0) | (fullscreen_ << 1)), s);
  TlStoreString::store(random_id_, s);
}

void messages_clickSponsoredMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.clickSponsoredMessage");
    s.store_field("flags", (var0 = flags_ | (media_ << 0) | (fullscreen_ << 1)));
    if (var0 & 1) { s.store_field("media", true); }
    if (var0 & 2) { s.store_field("fullscreen", true); }
    s.store_bytes_field("random_id", random_id_);
    s.store_class_end();
  }
}

messages_clickSponsoredMessage::ReturnType messages_clickSponsoredMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_deleteSavedHistory::messages_deleteSavedHistory(int32 flags_, object_ptr<InputPeer> &&parent_peer_, object_ptr<InputPeer> &&peer_, int32 max_id_, int32 min_date_, int32 max_date_)
  : flags_(flags_)
  , parent_peer_(std::move(parent_peer_))
  , peer_(std::move(peer_))
  , max_id_(max_id_)
  , min_date_(min_date_)
  , max_date_(max_date_)
{}

const std::int32_t messages_deleteSavedHistory::ID;

void messages_deleteSavedHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1304758367);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
  if (var0 & 4) { TlStoreBinary::store(min_date_, s); }
  if (var0 & 8) { TlStoreBinary::store(max_date_, s); }
}

void messages_deleteSavedHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1304758367);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
  if (var0 & 4) { TlStoreBinary::store(min_date_, s); }
  if (var0 & 8) { TlStoreBinary::store(max_date_, s); }
}

void messages_deleteSavedHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteSavedHistory");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_id", max_id_);
    if (var0 & 4) { s.store_field("min_date", min_date_); }
    if (var0 & 8) { s.store_field("max_date", max_date_); }
    s.store_class_end();
  }
}

messages_deleteSavedHistory::ReturnType messages_deleteSavedHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedHistory>, -1269012015>::parse(p);
#undef FAIL
}

messages_editFactCheck::messages_editFactCheck(object_ptr<InputPeer> &&peer_, int32 msg_id_, object_ptr<textWithEntities> &&text_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , text_(std::move(text_))
{}

const std::int32_t messages_editFactCheck::ID;

void messages_editFactCheck::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(92925557);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(text_, s);
}

void messages_editFactCheck::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(92925557);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(text_, s);
}

void messages_editFactCheck::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editFactCheck");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

messages_editFactCheck::ReturnType messages_editFactCheck::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getBotApp::messages_getBotApp(object_ptr<InputBotApp> &&app_, int64 hash_)
  : app_(std::move(app_))
  , hash_(hash_)
{}

const std::int32_t messages_getBotApp::ID;

void messages_getBotApp::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(889046467);
  TlStoreBoxedUnknown<TlStoreObject>::store(app_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getBotApp::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(889046467);
  TlStoreBoxedUnknown<TlStoreObject>::store(app_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getBotApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getBotApp");
    s.store_object_field("app", static_cast<const BaseObject *>(app_.get()));
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getBotApp::ReturnType messages_getBotApp::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_botApp>, -347034123>::parse(p);
#undef FAIL
}

messages_getDhConfig::messages_getDhConfig(int32 version_, int32 random_length_)
  : version_(version_)
  , random_length_(random_length_)
{}

const std::int32_t messages_getDhConfig::ID;

void messages_getDhConfig::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(651135312);
  TlStoreBinary::store(version_, s);
  TlStoreBinary::store(random_length_, s);
}

void messages_getDhConfig::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(651135312);
  TlStoreBinary::store(version_, s);
  TlStoreBinary::store(random_length_, s);
}

void messages_getDhConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDhConfig");
    s.store_field("version", version_);
    s.store_field("random_length", random_length_);
    s.store_class_end();
  }
}

messages_getDhConfig::ReturnType messages_getDhConfig::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_DhConfig>::parse(p);
#undef FAIL
}

messages_getDocumentByHash::messages_getDocumentByHash(bytes &&sha256_, int64 size_, string const &mime_type_)
  : sha256_(std::move(sha256_))
  , size_(size_)
  , mime_type_(mime_type_)
{}

const std::int32_t messages_getDocumentByHash::ID;

void messages_getDocumentByHash::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1309538785);
  TlStoreString::store(sha256_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(mime_type_, s);
}

void messages_getDocumentByHash::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1309538785);
  TlStoreString::store(sha256_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(mime_type_, s);
}

void messages_getDocumentByHash::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getDocumentByHash");
    s.store_bytes_field("sha256", sha256_);
    s.store_field("size", size_);
    s.store_field("mime_type", mime_type_);
    s.store_class_end();
  }
}

messages_getDocumentByHash::ReturnType messages_getDocumentByHash::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Document>::parse(p);
#undef FAIL
}

messages_getFavedStickers::messages_getFavedStickers(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getFavedStickers::ID;

void messages_getFavedStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(82946729);
  TlStoreBinary::store(hash_, s);
}

void messages_getFavedStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(82946729);
  TlStoreBinary::store(hash_, s);
}

void messages_getFavedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getFavedStickers");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getFavedStickers::ReturnType messages_getFavedStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_FavedStickers>::parse(p);
#undef FAIL
}

messages_getHistory::messages_getHistory(object_ptr<InputPeer> &&peer_, int32 offset_id_, int32 offset_date_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_, int64 hash_)
  : peer_(std::move(peer_))
  , offset_id_(offset_id_)
  , offset_date_(offset_date_)
  , add_offset_(add_offset_)
  , limit_(limit_)
  , max_id_(max_id_)
  , min_id_(min_id_)
  , hash_(hash_)
{}

const std::int32_t messages_getHistory::ID;

void messages_getHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1143203525);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1143203525);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getHistory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
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

messages_getHistory::ReturnType messages_getHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_getInlineGameHighScores::messages_getInlineGameHighScores(object_ptr<InputBotInlineMessageID> &&id_, object_ptr<InputUser> &&user_id_)
  : id_(std::move(id_))
  , user_id_(std::move(user_id_))
{}

const std::int32_t messages_getInlineGameHighScores::ID;

void messages_getInlineGameHighScores::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(258170395);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_getInlineGameHighScores::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(258170395);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_getInlineGameHighScores::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getInlineGameHighScores");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

messages_getInlineGameHighScores::ReturnType messages_getInlineGameHighScores::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_highScores>, -1707344487>::parse(p);
#undef FAIL
}

messages_getMaskStickers::messages_getMaskStickers(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getMaskStickers::ID;

void messages_getMaskStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1678738104);
  TlStoreBinary::store(hash_, s);
}

void messages_getMaskStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1678738104);
  TlStoreBinary::store(hash_, s);
}

void messages_getMaskStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMaskStickers");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getMaskStickers::ReturnType messages_getMaskStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_AllStickers>::parse(p);
#undef FAIL
}

messages_getOnlines::messages_getOnlines(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t messages_getOnlines::ID;

void messages_getOnlines::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1848369232);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_getOnlines::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1848369232);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_getOnlines::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getOnlines");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_getOnlines::ReturnType messages_getOnlines::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<chatOnlines>, -264117680>::parse(p);
#undef FAIL
}

messages_getSavedGifs::messages_getSavedGifs(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getSavedGifs::ID;

void messages_getSavedGifs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1559270965);
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedGifs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1559270965);
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedGifs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSavedGifs");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getSavedGifs::ReturnType messages_getSavedGifs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SavedGifs>::parse(p);
#undef FAIL
}

messages_getUnreadMentions::messages_getUnreadMentions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, int32 offset_id_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
  , offset_id_(offset_id_)
  , add_offset_(add_offset_)
  , limit_(limit_)
  , max_id_(max_id_)
  , min_id_(min_id_)
{}

const std::int32_t messages_getUnreadMentions::ID;

void messages_getUnreadMentions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-251140208);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
}

void messages_getUnreadMentions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-251140208);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
}

void messages_getUnreadMentions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getUnreadMentions");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_field("offset_id", offset_id_);
    s.store_field("add_offset", add_offset_);
    s.store_field("limit", limit_);
    s.store_field("max_id", max_id_);
    s.store_field("min_id", min_id_);
    s.store_class_end();
  }
}

messages_getUnreadMentions::ReturnType messages_getUnreadMentions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_importChatInvite::messages_importChatInvite(string const &hash_)
  : hash_(hash_)
{}

const std::int32_t messages_importChatInvite::ID;

void messages_importChatInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1817183516);
  TlStoreString::store(hash_, s);
}

void messages_importChatInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1817183516);
  TlStoreString::store(hash_, s);
}

void messages_importChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.importChatInvite");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_importChatInvite::ReturnType messages_importChatInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_markDialogUnread::messages_markDialogUnread(int32 flags_, bool unread_, object_ptr<InputPeer> &&parent_peer_, object_ptr<InputDialogPeer> &&peer_)
  : flags_(flags_)
  , unread_(unread_)
  , parent_peer_(std::move(parent_peer_))
  , peer_(std::move(peer_))
{}

const std::int32_t messages_markDialogUnread::ID;

void messages_markDialogUnread::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1940912392);
  TlStoreBinary::store((var0 = flags_ | (unread_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_markDialogUnread::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1940912392);
  TlStoreBinary::store((var0 = flags_ | (unread_ << 0)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_markDialogUnread::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.markDialogUnread");
    s.store_field("flags", (var0 = flags_ | (unread_ << 0)));
    if (var0 & 1) { s.store_field("unread", true); }
    if (var0 & 2) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_markDialogUnread::ReturnType messages_markDialogUnread::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_reorderPinnedDialogs::messages_reorderPinnedDialogs(int32 flags_, bool force_, int32 folder_id_, array<object_ptr<InputDialogPeer>> &&order_)
  : flags_(flags_)
  , force_(force_)
  , folder_id_(folder_id_)
  , order_(std::move(order_))
{}

const std::int32_t messages_reorderPinnedDialogs::ID;

void messages_reorderPinnedDialogs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(991616823);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBinary::store(folder_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s);
}

void messages_reorderPinnedDialogs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(991616823);
  TlStoreBinary::store((var0 = flags_ | (force_ << 0)), s);
  TlStoreBinary::store(folder_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s);
}

void messages_reorderPinnedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reorderPinnedDialogs");
    s.store_field("flags", (var0 = flags_ | (force_ << 0)));
    if (var0 & 1) { s.store_field("force", true); }
    s.store_field("folder_id", folder_id_);
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_reorderPinnedDialogs::ReturnType messages_reorderPinnedDialogs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_sendEncryptedFile::messages_sendEncryptedFile(int32 flags_, bool silent_, object_ptr<inputEncryptedChat> &&peer_, int64 random_id_, bytes &&data_, object_ptr<InputEncryptedFile> &&file_)
  : flags_(flags_)
  , silent_(silent_)
  , peer_(std::move(peer_))
  , random_id_(random_id_)
  , data_(std::move(data_))
  , file_(std::move(file_))
{}

const std::int32_t messages_sendEncryptedFile::ID;

void messages_sendEncryptedFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1431914525);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 0)), s);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(data_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
}

void messages_sendEncryptedFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1431914525);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 0)), s);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(data_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
}

void messages_sendEncryptedFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendEncryptedFile");
    s.store_field("flags", (var0 = flags_ | (silent_ << 0)));
    if (var0 & 1) { s.store_field("silent", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("random_id", random_id_);
    s.store_bytes_field("data", data_);
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_class_end();
  }
}

messages_sendEncryptedFile::ReturnType messages_sendEncryptedFile::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SentEncryptedMessage>::parse(p);
#undef FAIL
}

messages_sendMedia::messages_sendMedia(int32 flags_, bool silent_, bool background_, bool clear_draft_, bool noforwards_, bool update_stickersets_order_, bool invert_media_, bool allow_paid_floodskip_, object_ptr<InputPeer> &&peer_, object_ptr<InputReplyTo> &&reply_to_, object_ptr<InputMedia> &&media_, string const &message_, int64 random_id_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_, object_ptr<InputQuickReplyShortcut> &&quick_reply_shortcut_, int64 effect_, int64 allow_paid_stars_, object_ptr<suggestedPost> &&suggested_post_)
  : flags_(flags_)
  , silent_(silent_)
  , background_(background_)
  , clear_draft_(clear_draft_)
  , noforwards_(noforwards_)
  , update_stickersets_order_(update_stickersets_order_)
  , invert_media_(invert_media_)
  , allow_paid_floodskip_(allow_paid_floodskip_)
  , peer_(std::move(peer_))
  , reply_to_(std::move(reply_to_))
  , media_(std::move(media_))
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

const std::int32_t messages_sendMedia::ID;

void messages_sendMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1403659839);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
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

void messages_sendMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1403659839);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
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

void messages_sendMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendMedia");
    s.store_field("flags", (var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)));
    if (var0 & 32) { s.store_field("silent", true); }
    if (var0 & 64) { s.store_field("background", true); }
    if (var0 & 128) { s.store_field("clear_draft", true); }
    if (var0 & 16384) { s.store_field("noforwards", true); }
    if (var0 & 32768) { s.store_field("update_stickersets_order", true); }
    if (var0 & 65536) { s.store_field("invert_media", true); }
    if (var0 & 524288) { s.store_field("allow_paid_floodskip", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
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

messages_sendMedia::ReturnType messages_sendMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_setDefaultReaction::messages_setDefaultReaction(object_ptr<Reaction> &&reaction_)
  : reaction_(std::move(reaction_))
{}

const std::int32_t messages_setDefaultReaction::ID;

void messages_setDefaultReaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1330094102);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
}

void messages_setDefaultReaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1330094102);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
}

void messages_setDefaultReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setDefaultReaction");
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_class_end();
  }
}

messages_setDefaultReaction::ReturnType messages_setDefaultReaction::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_setGameScore::messages_setGameScore(int32 flags_, bool edit_message_, bool force_, object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<InputUser> &&user_id_, int32 score_)
  : flags_(flags_)
  , edit_message_(edit_message_)
  , force_(force_)
  , peer_(std::move(peer_))
  , id_(id_)
  , user_id_(std::move(user_id_))
  , score_(score_)
{}

const std::int32_t messages_setGameScore::ID;

void messages_setGameScore::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1896289088);
  TlStoreBinary::store((var0 = flags_ | (edit_message_ << 0) | (force_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(score_, s);
}

void messages_setGameScore::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1896289088);
  TlStoreBinary::store((var0 = flags_ | (edit_message_ << 0) | (force_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(score_, s);
}

void messages_setGameScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setGameScore");
    s.store_field("flags", (var0 = flags_ | (edit_message_ << 0) | (force_ << 1)));
    if (var0 & 1) { s.store_field("edit_message", true); }
    if (var0 & 2) { s.store_field("force", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("score", score_);
    s.store_class_end();
  }
}

messages_setGameScore::ReturnType messages_setGameScore::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_toggleSavedDialogPin::messages_toggleSavedDialogPin(int32 flags_, bool pinned_, object_ptr<InputDialogPeer> &&peer_)
  : flags_(flags_)
  , pinned_(pinned_)
  , peer_(std::move(peer_))
{}

const std::int32_t messages_toggleSavedDialogPin::ID;

void messages_toggleSavedDialogPin::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1400783906);
  TlStoreBinary::store((var0 = flags_ | (pinned_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_toggleSavedDialogPin::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1400783906);
  TlStoreBinary::store((var0 = flags_ | (pinned_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_toggleSavedDialogPin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleSavedDialogPin");
    s.store_field("flags", (var0 = flags_ | (pinned_ << 0)));
    if (var0 & 1) { s.store_field("pinned", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_toggleSavedDialogPin::ReturnType messages_toggleSavedDialogPin::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_applyGiftCode::payments_applyGiftCode(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t payments_applyGiftCode::ID;

void payments_applyGiftCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-152934316);
  TlStoreString::store(slug_, s);
}

void payments_applyGiftCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-152934316);
  TlStoreString::store(slug_, s);
}

void payments_applyGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.applyGiftCode");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

payments_applyGiftCode::ReturnType payments_applyGiftCode::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_getStarGiftCollections::payments_getStarGiftCollections(object_ptr<InputPeer> &&peer_, int64 hash_)
  : peer_(std::move(peer_))
  , hash_(hash_)
{}

const std::int32_t payments_getStarGiftCollections::ID;

void payments_getStarGiftCollections::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1743023651);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(hash_, s);
}

void payments_getStarGiftCollections::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1743023651);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(hash_, s);
}

void payments_getStarGiftCollections::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarGiftCollections");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

payments_getStarGiftCollections::ReturnType payments_getStarGiftCollections::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_StarGiftCollections>::parse(p);
#undef FAIL
}

payments_getStarsRevenueWithdrawalUrl::payments_getStarsRevenueWithdrawalUrl(int32 flags_, bool ton_, object_ptr<InputPeer> &&peer_, int64 amount_, object_ptr<InputCheckPasswordSRP> &&password_)
  : flags_(flags_)
  , ton_(ton_)
  , peer_(std::move(peer_))
  , amount_(amount_)
  , password_(std::move(password_))
{}

const std::int32_t payments_getStarsRevenueWithdrawalUrl::ID;

void payments_getStarsRevenueWithdrawalUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(607378578);
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreBinary::store(amount_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void payments_getStarsRevenueWithdrawalUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(607378578);
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreBinary::store(amount_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void payments_getStarsRevenueWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsRevenueWithdrawalUrl");
    s.store_field("flags", (var0 = flags_ | (ton_ << 0)));
    if (var0 & 1) { s.store_field("ton", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 2) { s.store_field("amount", amount_); }
    s.store_object_field("password", static_cast<const BaseObject *>(password_.get()));
    s.store_class_end();
  }
}

payments_getStarsRevenueWithdrawalUrl::ReturnType payments_getStarsRevenueWithdrawalUrl::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starsRevenueWithdrawalUrl>, 497778871>::parse(p);
#undef FAIL
}

payments_toggleStarGiftsPinnedToTop::payments_toggleStarGiftsPinnedToTop(object_ptr<InputPeer> &&peer_, array<object_ptr<InputSavedStarGift>> &&stargift_)
  : peer_(std::move(peer_))
  , stargift_(std::move(stargift_))
{}

const std::int32_t payments_toggleStarGiftsPinnedToTop::ID;

void payments_toggleStarGiftsPinnedToTop::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(353626032);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stargift_, s);
}

void payments_toggleStarGiftsPinnedToTop::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(353626032);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stargift_, s);
}

void payments_toggleStarGiftsPinnedToTop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.toggleStarGiftsPinnedToTop");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("stargift", stargift_.size()); for (const auto &_value : stargift_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_toggleStarGiftsPinnedToTop::ReturnType payments_toggleStarGiftsPinnedToTop::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_updateStarGiftCollection::payments_updateStarGiftCollection(int32 flags_, object_ptr<InputPeer> &&peer_, int32 collection_id_, string const &title_, array<object_ptr<InputSavedStarGift>> &&delete_stargift_, array<object_ptr<InputSavedStarGift>> &&add_stargift_, array<object_ptr<InputSavedStarGift>> &&order_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , collection_id_(collection_id_)
  , title_(title_)
  , delete_stargift_(std::move(delete_stargift_))
  , add_stargift_(std::move(add_stargift_))
  , order_(std::move(order_))
{}

const std::int32_t payments_updateStarGiftCollection::ID;

void payments_updateStarGiftCollection::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1339932391);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(collection_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(delete_stargift_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(add_stargift_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s); }
}

void payments_updateStarGiftCollection::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1339932391);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(collection_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(delete_stargift_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(add_stargift_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(order_, s); }
}

void payments_updateStarGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.updateStarGiftCollection");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("collection_id", collection_id_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { { s.store_vector_begin("delete_stargift", delete_stargift_.size()); for (const auto &_value : delete_stargift_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { { s.store_vector_begin("add_stargift", add_stargift_.size()); for (const auto &_value : add_stargift_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 8) { { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

payments_updateStarGiftCollection::ReturnType payments_updateStarGiftCollection::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<starGiftCollection>, -1653926992>::parse(p);
#undef FAIL
}

phone_acceptCall::phone_acceptCall(object_ptr<inputPhoneCall> &&peer_, bytes &&g_b_, object_ptr<phoneCallProtocol> &&protocol_)
  : peer_(std::move(peer_))
  , g_b_(std::move(g_b_))
  , protocol_(std::move(protocol_))
{}

const std::int32_t phone_acceptCall::ID;

void phone_acceptCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1003664544);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreString::store(g_b_, s);
  TlStoreBoxed<TlStoreObject, -58224696>::store(protocol_, s);
}

void phone_acceptCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1003664544);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreString::store(g_b_, s);
  TlStoreBoxed<TlStoreObject, -58224696>::store(protocol_, s);
}

void phone_acceptCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.acceptCall");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_bytes_field("g_b", g_b_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    s.store_class_end();
  }
}

phone_acceptCall::ReturnType phone_acceptCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_phoneCall>, -326966976>::parse(p);
#undef FAIL
}

phone_discardCall::phone_discardCall(int32 flags_, bool video_, object_ptr<inputPhoneCall> &&peer_, int32 duration_, object_ptr<PhoneCallDiscardReason> &&reason_, int64 connection_id_)
  : flags_(flags_)
  , video_(video_)
  , peer_(std::move(peer_))
  , duration_(duration_)
  , reason_(std::move(reason_))
  , connection_id_(connection_id_)
{}

const std::int32_t phone_discardCall::ID;

void phone_discardCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1295269440);
  TlStoreBinary::store((var0 = flags_ | (video_ << 0)), s);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reason_, s);
  TlStoreBinary::store(connection_id_, s);
}

void phone_discardCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1295269440);
  TlStoreBinary::store((var0 = flags_ | (video_ << 0)), s);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reason_, s);
  TlStoreBinary::store(connection_id_, s);
}

void phone_discardCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.discardCall");
    s.store_field("flags", (var0 = flags_ | (video_ << 0)));
    if (var0 & 1) { s.store_field("video", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("duration", duration_);
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_field("connection_id", connection_id_);
    s.store_class_end();
  }
}

phone_discardCall::ReturnType phone_discardCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_inviteConferenceCallParticipant::phone_inviteConferenceCallParticipant(int32 flags_, bool video_, object_ptr<InputGroupCall> &&call_, object_ptr<InputUser> &&user_id_)
  : flags_(flags_)
  , video_(video_)
  , call_(std::move(call_))
  , user_id_(std::move(user_id_))
{}

const std::int32_t phone_inviteConferenceCallParticipant::ID;

void phone_inviteConferenceCallParticipant::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1124981115);
  TlStoreBinary::store((var0 = flags_ | (video_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void phone_inviteConferenceCallParticipant::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1124981115);
  TlStoreBinary::store((var0 = flags_ | (video_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void phone_inviteConferenceCallParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.inviteConferenceCallParticipant");
    s.store_field("flags", (var0 = flags_ | (video_ << 0)));
    if (var0 & 1) { s.store_field("video", true); }
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

phone_inviteConferenceCallParticipant::ReturnType phone_inviteConferenceCallParticipant::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_leaveGroupCall::phone_leaveGroupCall(object_ptr<InputGroupCall> &&call_, int32 source_)
  : call_(std::move(call_))
  , source_(source_)
{}

const std::int32_t phone_leaveGroupCall::ID;

void phone_leaveGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1342404601);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(source_, s);
}

void phone_leaveGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1342404601);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(source_, s);
}

void phone_leaveGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.leaveGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("source", source_);
    s.store_class_end();
  }
}

phone_leaveGroupCall::ReturnType phone_leaveGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_toggleGroupCallSettings::phone_toggleGroupCallSettings(int32 flags_, bool reset_invite_hash_, object_ptr<InputGroupCall> &&call_, bool join_muted_, bool messages_enabled_)
  : flags_(flags_)
  , reset_invite_hash_(reset_invite_hash_)
  , call_(std::move(call_))
  , join_muted_(join_muted_)
  , messages_enabled_(messages_enabled_)
{}

const std::int32_t phone_toggleGroupCallSettings::ID;

void phone_toggleGroupCallSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-378390524);
  TlStoreBinary::store((var0 = flags_ | (reset_invite_hash_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  if (var0 & 1) { TlStoreBool::store(join_muted_, s); }
  if (var0 & 4) { TlStoreBool::store(messages_enabled_, s); }
}

void phone_toggleGroupCallSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-378390524);
  TlStoreBinary::store((var0 = flags_ | (reset_invite_hash_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  if (var0 & 1) { TlStoreBool::store(join_muted_, s); }
  if (var0 & 4) { TlStoreBool::store(messages_enabled_, s); }
}

void phone_toggleGroupCallSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.toggleGroupCallSettings");
    s.store_field("flags", (var0 = flags_ | (reset_invite_hash_ << 1)));
    if (var0 & 2) { s.store_field("reset_invite_hash", true); }
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    if (var0 & 1) { s.store_field("join_muted", join_muted_); }
    if (var0 & 4) { s.store_field("messages_enabled", messages_enabled_); }
    s.store_class_end();
  }
}

phone_toggleGroupCallSettings::ReturnType phone_toggleGroupCallSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_toggleGroupCallStartSubscription::phone_toggleGroupCallStartSubscription(object_ptr<InputGroupCall> &&call_, bool subscribed_)
  : call_(std::move(call_))
  , subscribed_(subscribed_)
{}

const std::int32_t phone_toggleGroupCallStartSubscription::ID;

void phone_toggleGroupCallStartSubscription::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(563885286);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBool::store(subscribed_, s);
}

void phone_toggleGroupCallStartSubscription::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(563885286);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBool::store(subscribed_, s);
}

void phone_toggleGroupCallStartSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.toggleGroupCallStartSubscription");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("subscribed", subscribed_);
    s.store_class_end();
  }
}

phone_toggleGroupCallStartSubscription::ReturnType phone_toggleGroupCallStartSubscription::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

premium_applyBoost::premium_applyBoost(int32 flags_, array<int32> &&slots_, object_ptr<InputPeer> &&peer_)
  : flags_(flags_)
  , slots_(std::move(slots_))
  , peer_(std::move(peer_))
{}

const std::int32_t premium_applyBoost::ID;

void premium_applyBoost::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1803396934);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(slots_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void premium_applyBoost::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1803396934);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(slots_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void premium_applyBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.applyBoost");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { { s.store_vector_begin("slots", slots_.size()); for (const auto &_value : slots_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

premium_applyBoost::ReturnType premium_applyBoost::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<premium_myBoosts>, -1696454430>::parse(p);
#undef FAIL
}

stickers_deleteStickerSet::stickers_deleteStickerSet(object_ptr<InputStickerSet> &&stickerset_)
  : stickerset_(std::move(stickerset_))
{}

const std::int32_t stickers_deleteStickerSet::ID;

void stickers_deleteStickerSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2022685804);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void stickers_deleteStickerSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2022685804);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void stickers_deleteStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.deleteStickerSet");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_class_end();
  }
}

stickers_deleteStickerSet::ReturnType stickers_deleteStickerSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stories_getPeerStories::stories_getPeerStories(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t stories_getPeerStories::ID;

void stories_getPeerStories::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(743103056);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void stories_getPeerStories::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(743103056);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void stories_getPeerStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getPeerStories");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

stories_getPeerStories::ReturnType stories_getPeerStories::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_peerStories>, -890861720>::parse(p);
#undef FAIL
}

stories_searchPosts::stories_searchPosts(int32 flags_, string const &hashtag_, object_ptr<MediaArea> &&area_, object_ptr<InputPeer> &&peer_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , hashtag_(hashtag_)
  , area_(std::move(area_))
  , peer_(std::move(peer_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t stories_searchPosts::ID;

void stories_searchPosts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-780072697);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(hashtag_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(area_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_searchPosts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-780072697);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(hashtag_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(area_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s); }
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stories_searchPosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.searchPosts");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("hashtag", hashtag_); }
    if (var0 & 2) { s.store_object_field("area", static_cast<const BaseObject *>(area_.get())); }
    if (var0 & 4) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stories_searchPosts::ReturnType stories_searchPosts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_foundStories>, -488736969>::parse(p);
#undef FAIL
}

const std::int32_t test_parseInputAppEvent::ID;

void test_parseInputAppEvent::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1156741135);
}

void test_parseInputAppEvent::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1156741135);
}

void test_parseInputAppEvent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "test.parseInputAppEvent");
    s.store_class_end();
  }
}

test_parseInputAppEvent::ReturnType test_parseInputAppEvent::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<inputAppEvent>, 488313413>::parse(p);
#undef FAIL
}

upload_getCdnFile::upload_getCdnFile(bytes &&file_token_, int64 offset_, int32 limit_)
  : file_token_(std::move(file_token_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t upload_getCdnFile::ID;

void upload_getCdnFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(962554330);
  TlStoreString::store(file_token_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void upload_getCdnFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(962554330);
  TlStoreString::store(file_token_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void upload_getCdnFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.getCdnFile");
    s.store_bytes_field("file_token", file_token_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

upload_getCdnFile::ReturnType upload_getCdnFile::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<upload_CdnFile>::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
