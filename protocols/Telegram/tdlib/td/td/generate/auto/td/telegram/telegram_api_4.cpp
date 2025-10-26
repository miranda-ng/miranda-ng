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


const std::int32_t bankCardOpenUrl::ID;

object_ptr<bankCardOpenUrl> bankCardOpenUrl::fetch(TlBufferParser &p) {
  return make_tl_object<bankCardOpenUrl>(p);
}

bankCardOpenUrl::bankCardOpenUrl(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , name_(TlFetchString<string>::parse(p))
{}

void bankCardOpenUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bankCardOpenUrl");
    s.store_field("url", url_);
    s.store_field("name", name_);
    s.store_class_end();
  }
}

botAppSettings::botAppSettings()
  : flags_()
  , placeholder_path_()
  , background_color_()
  , background_dark_color_()
  , header_color_()
  , header_dark_color_()
{}

const std::int32_t botAppSettings::ID;

object_ptr<botAppSettings> botAppSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botAppSettings> res = make_tl_object<botAppSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->placeholder_path_ = TlFetchBytes<bytes>::parse(p); }
  if (var0 & 2) { res->background_color_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->background_dark_color_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->header_color_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->header_dark_color_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void botAppSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botAppSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_bytes_field("placeholder_path", placeholder_path_); }
    if (var0 & 2) { s.store_field("background_color", background_color_); }
    if (var0 & 4) { s.store_field("background_dark_color", background_dark_color_); }
    if (var0 & 8) { s.store_field("header_color", header_color_); }
    if (var0 & 16) { s.store_field("header_dark_color", header_dark_color_); }
    s.store_class_end();
  }
}

businessWeeklyOpen::businessWeeklyOpen(int32 start_minute_, int32 end_minute_)
  : start_minute_(start_minute_)
  , end_minute_(end_minute_)
{}

const std::int32_t businessWeeklyOpen::ID;

object_ptr<businessWeeklyOpen> businessWeeklyOpen::fetch(TlBufferParser &p) {
  return make_tl_object<businessWeeklyOpen>(p);
}

businessWeeklyOpen::businessWeeklyOpen(TlBufferParser &p)
  : start_minute_(TlFetchInt::parse(p))
  , end_minute_(TlFetchInt::parse(p))
{}

void businessWeeklyOpen::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(start_minute_, s);
  TlStoreBinary::store(end_minute_, s);
}

void businessWeeklyOpen::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(start_minute_, s);
  TlStoreBinary::store(end_minute_, s);
}

void businessWeeklyOpen::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessWeeklyOpen");
    s.store_field("start_minute", start_minute_);
    s.store_field("end_minute", end_minute_);
    s.store_class_end();
  }
}

const std::int32_t cdnPublicKey::ID;

object_ptr<cdnPublicKey> cdnPublicKey::fetch(TlBufferParser &p) {
  return make_tl_object<cdnPublicKey>(p);
}

cdnPublicKey::cdnPublicKey(TlBufferParser &p)
  : dc_id_(TlFetchInt::parse(p))
  , public_key_(TlFetchString<string>::parse(p))
{}

void cdnPublicKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "cdnPublicKey");
    s.store_field("dc_id", dc_id_);
    s.store_field("public_key", public_key_);
    s.store_class_end();
  }
}

const std::int32_t channelParticipantsRecent::ID;

void channelParticipantsRecent::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void channelParticipantsRecent::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void channelParticipantsRecent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsRecent");
    s.store_class_end();
  }
}

const std::int32_t channelParticipantsAdmins::ID;

void channelParticipantsAdmins::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void channelParticipantsAdmins::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void channelParticipantsAdmins::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsAdmins");
    s.store_class_end();
  }
}

channelParticipantsKicked::channelParticipantsKicked(string const &q_)
  : q_(q_)
{}

const std::int32_t channelParticipantsKicked::ID;

void channelParticipantsKicked::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsKicked::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsKicked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsKicked");
    s.store_field("q", q_);
    s.store_class_end();
  }
}

const std::int32_t channelParticipantsBots::ID;

void channelParticipantsBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void channelParticipantsBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void channelParticipantsBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsBots");
    s.store_class_end();
  }
}

channelParticipantsBanned::channelParticipantsBanned(string const &q_)
  : q_(q_)
{}

const std::int32_t channelParticipantsBanned::ID;

void channelParticipantsBanned::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsBanned::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsBanned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsBanned");
    s.store_field("q", q_);
    s.store_class_end();
  }
}

channelParticipantsSearch::channelParticipantsSearch(string const &q_)
  : q_(q_)
{}

const std::int32_t channelParticipantsSearch::ID;

void channelParticipantsSearch::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsSearch::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsSearch::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsSearch");
    s.store_field("q", q_);
    s.store_class_end();
  }
}

channelParticipantsContacts::channelParticipantsContacts(string const &q_)
  : q_(q_)
{}

const std::int32_t channelParticipantsContacts::ID;

void channelParticipantsContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(q_, s);
}

void channelParticipantsContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsContacts");
    s.store_field("q", q_);
    s.store_class_end();
  }
}

channelParticipantsMentions::channelParticipantsMentions(int32 flags_, string const &q_, int32 top_msg_id_)
  : flags_(flags_)
  , q_(q_)
  , top_msg_id_(top_msg_id_)
{}

const std::int32_t channelParticipantsMentions::ID;

void channelParticipantsMentions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(q_, s); }
  if (var0 & 2) { TlStoreBinary::store(top_msg_id_, s); }
}

void channelParticipantsMentions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(q_, s); }
  if (var0 & 2) { TlStoreBinary::store(top_msg_id_, s); }
}

void channelParticipantsMentions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantsMentions");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("q", q_); }
    if (var0 & 2) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_class_end();
  }
}

chatInviteImporter::chatInviteImporter()
  : flags_()
  , requested_()
  , via_chatlist_()
  , user_id_()
  , date_()
  , about_()
  , approved_by_()
{}

const std::int32_t chatInviteImporter::ID;

object_ptr<chatInviteImporter> chatInviteImporter::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatInviteImporter> res = make_tl_object<chatInviteImporter>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->requested_ = (var0 & 1) != 0;
  res->via_chatlist_ = (var0 & 8) != 0;
  res->user_id_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->about_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->approved_by_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void chatInviteImporter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteImporter");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (requested_ << 0) | (via_chatlist_ << 3)));
    if (var0 & 1) { s.store_field("requested", true); }
    if (var0 & 8) { s.store_field("via_chatlist", true); }
    s.store_field("user_id", user_id_);
    s.store_field("date", date_);
    if (var0 & 4) { s.store_field("about", about_); }
    if (var0 & 2) { s.store_field("approved_by", approved_by_); }
    s.store_class_end();
  }
}

emailVerificationCode::emailVerificationCode(string const &code_)
  : code_(code_)
{}

const std::int32_t emailVerificationCode::ID;

void emailVerificationCode::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(code_, s);
}

void emailVerificationCode::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(code_, s);
}

void emailVerificationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailVerificationCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

emailVerificationGoogle::emailVerificationGoogle(string const &token_)
  : token_(token_)
{}

const std::int32_t emailVerificationGoogle::ID;

void emailVerificationGoogle::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(token_, s);
}

void emailVerificationGoogle::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(token_, s);
}

void emailVerificationGoogle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailVerificationGoogle");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

emailVerificationApple::emailVerificationApple(string const &token_)
  : token_(token_)
{}

const std::int32_t emailVerificationApple::ID;

void emailVerificationApple::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(token_, s);
}

void emailVerificationApple::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(token_, s);
}

void emailVerificationApple::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailVerificationApple");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

object_ptr<EmojiGroup> EmojiGroup::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case emojiGroup::ID:
      return emojiGroup::fetch(p);
    case emojiGroupGreeting::ID:
      return emojiGroupGreeting::fetch(p);
    case emojiGroupPremium::ID:
      return emojiGroupPremium::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t emojiGroup::ID;

object_ptr<EmojiGroup> emojiGroup::fetch(TlBufferParser &p) {
  return make_tl_object<emojiGroup>(p);
}

emojiGroup::emojiGroup(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , icon_emoji_id_(TlFetchLong::parse(p))
  , emoticons_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
{}

void emojiGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiGroup");
    s.store_field("title", title_);
    s.store_field("icon_emoji_id", icon_emoji_id_);
    { s.store_vector_begin("emoticons", emoticons_.size()); for (const auto &_value : emoticons_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t emojiGroupGreeting::ID;

object_ptr<EmojiGroup> emojiGroupGreeting::fetch(TlBufferParser &p) {
  return make_tl_object<emojiGroupGreeting>(p);
}

emojiGroupGreeting::emojiGroupGreeting(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , icon_emoji_id_(TlFetchLong::parse(p))
  , emoticons_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
{}

void emojiGroupGreeting::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiGroupGreeting");
    s.store_field("title", title_);
    s.store_field("icon_emoji_id", icon_emoji_id_);
    { s.store_vector_begin("emoticons", emoticons_.size()); for (const auto &_value : emoticons_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t emojiGroupPremium::ID;

object_ptr<EmojiGroup> emojiGroupPremium::fetch(TlBufferParser &p) {
  return make_tl_object<emojiGroupPremium>(p);
}

emojiGroupPremium::emojiGroupPremium(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , icon_emoji_id_(TlFetchLong::parse(p))
{}

void emojiGroupPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiGroupPremium");
    s.store_field("title", title_);
    s.store_field("icon_emoji_id", icon_emoji_id_);
    s.store_class_end();
  }
}

geoPointAddress::geoPointAddress()
  : flags_()
  , country_iso2_()
  , state_()
  , city_()
  , street_()
{}

geoPointAddress::geoPointAddress(int32 flags_, string const &country_iso2_, string const &state_, string const &city_, string const &street_)
  : flags_(flags_)
  , country_iso2_(country_iso2_)
  , state_(state_)
  , city_(city_)
  , street_(street_)
{}

const std::int32_t geoPointAddress::ID;

object_ptr<geoPointAddress> geoPointAddress::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<geoPointAddress> res = make_tl_object<geoPointAddress>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->country_iso2_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->state_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->city_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->street_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void geoPointAddress::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(country_iso2_, s);
  if (var0 & 1) { TlStoreString::store(state_, s); }
  if (var0 & 2) { TlStoreString::store(city_, s); }
  if (var0 & 4) { TlStoreString::store(street_, s); }
}

void geoPointAddress::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(country_iso2_, s);
  if (var0 & 1) { TlStoreString::store(state_, s); }
  if (var0 & 2) { TlStoreString::store(city_, s); }
  if (var0 & 4) { TlStoreString::store(street_, s); }
}

void geoPointAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "geoPointAddress");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("country_iso2", country_iso2_);
    if (var0 & 1) { s.store_field("state", state_); }
    if (var0 & 2) { s.store_field("city", city_); }
    if (var0 & 4) { s.store_field("street", street_); }
    s.store_class_end();
  }
}

inputBotInlineMessageMediaAuto::inputBotInlineMessageMediaAuto(int32 flags_, bool invert_media_, string const &message_, array<object_ptr<MessageEntity>> &&entities_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , invert_media_(invert_media_)
  , message_(message_)
  , entities_(std::move(entities_))
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageMediaAuto::ID;

void inputBotInlineMessageMediaAuto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (invert_media_ << 3)), s);
  TlStoreString::store(message_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaAuto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (invert_media_ << 3)), s);
  TlStoreString::store(message_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaAuto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageMediaAuto");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (invert_media_ << 3)));
    if (var0 & 8) { s.store_field("invert_media", true); }
    s.store_field("message", message_);
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

inputBotInlineMessageText::inputBotInlineMessageText(int32 flags_, bool no_webpage_, bool invert_media_, string const &message_, array<object_ptr<MessageEntity>> &&entities_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , no_webpage_(no_webpage_)
  , invert_media_(invert_media_)
  , message_(message_)
  , entities_(std::move(entities_))
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageText::ID;

void inputBotInlineMessageText::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 0) | (invert_media_ << 3)), s);
  TlStoreString::store(message_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageText::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (no_webpage_ << 0) | (invert_media_ << 3)), s);
  TlStoreString::store(message_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageText");
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

inputBotInlineMessageMediaGeo::inputBotInlineMessageMediaGeo(int32 flags_, object_ptr<InputGeoPoint> &&geo_point_, int32 heading_, int32 period_, int32 proximity_notification_radius_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , geo_point_(std::move(geo_point_))
  , heading_(heading_)
  , period_(period_)
  , proximity_notification_radius_(proximity_notification_radius_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageMediaGeo::ID;

void inputBotInlineMessageMediaGeo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  if (var0 & 1) { TlStoreBinary::store(heading_, s); }
  if (var0 & 2) { TlStoreBinary::store(period_, s); }
  if (var0 & 8) { TlStoreBinary::store(proximity_notification_radius_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaGeo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  if (var0 & 1) { TlStoreBinary::store(heading_, s); }
  if (var0 & 2) { TlStoreBinary::store(period_, s); }
  if (var0 & 8) { TlStoreBinary::store(proximity_notification_radius_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaGeo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageMediaGeo");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    if (var0 & 1) { s.store_field("heading", heading_); }
    if (var0 & 2) { s.store_field("period", period_); }
    if (var0 & 8) { s.store_field("proximity_notification_radius", proximity_notification_radius_); }
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

inputBotInlineMessageMediaVenue::inputBotInlineMessageMediaVenue(int32 flags_, object_ptr<InputGeoPoint> &&geo_point_, string const &title_, string const &address_, string const &provider_, string const &venue_id_, string const &venue_type_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , geo_point_(std::move(geo_point_))
  , title_(title_)
  , address_(address_)
  , provider_(provider_)
  , venue_id_(venue_id_)
  , venue_type_(venue_type_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageMediaVenue::ID;

void inputBotInlineMessageMediaVenue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
  TlStoreString::store(venue_type_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaVenue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
  TlStoreString::store(venue_type_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageMediaVenue");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    s.store_field("title", title_);
    s.store_field("address", address_);
    s.store_field("provider", provider_);
    s.store_field("venue_id", venue_id_);
    s.store_field("venue_type", venue_type_);
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

inputBotInlineMessageMediaContact::inputBotInlineMessageMediaContact(int32 flags_, string const &phone_number_, string const &first_name_, string const &last_name_, string const &vcard_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , phone_number_(phone_number_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , vcard_(vcard_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageMediaContact::ID;

void inputBotInlineMessageMediaContact::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreString::store(vcard_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaContact::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreString::store(vcard_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageMediaContact");
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

inputBotInlineMessageGame::inputBotInlineMessageGame(int32 flags_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageGame::ID;

void inputBotInlineMessageGame::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageGame::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageGame");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

inputBotInlineMessageMediaInvoice::inputBotInlineMessageMediaInvoice(int32 flags_, string const &title_, string const &description_, object_ptr<inputWebDocument> &&photo_, object_ptr<invoice> &&invoice_, bytes &&payload_, string const &provider_, object_ptr<dataJSON> &&provider_data_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , title_(title_)
  , description_(description_)
  , photo_(std::move(photo_))
  , invoice_(std::move(invoice_))
  , payload_(std::move(payload_))
  , provider_(provider_)
  , provider_data_(std::move(provider_data_))
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageMediaInvoice::ID;

void inputBotInlineMessageMediaInvoice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(description_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1678949555>::store(photo_, s); }
  TlStoreBoxed<TlStoreObject, 77522308>::store(invoice_, s);
  TlStoreString::store(payload_, s);
  TlStoreString::store(provider_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(provider_data_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaInvoice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(description_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1678949555>::store(photo_, s); }
  TlStoreBoxed<TlStoreObject, 77522308>::store(invoice_, s);
  TlStoreString::store(payload_, s);
  TlStoreString::store(provider_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(provider_data_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageMediaInvoice");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 1) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_bytes_field("payload", payload_);
    s.store_field("provider", provider_);
    s.store_object_field("provider_data", static_cast<const BaseObject *>(provider_data_.get()));
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

inputBotInlineMessageMediaWebPage::inputBotInlineMessageMediaWebPage(int32 flags_, bool invert_media_, bool force_large_media_, bool force_small_media_, bool optional_, string const &message_, array<object_ptr<MessageEntity>> &&entities_, string const &url_, object_ptr<ReplyMarkup> &&reply_markup_)
  : flags_(flags_)
  , invert_media_(invert_media_)
  , force_large_media_(force_large_media_)
  , force_small_media_(force_small_media_)
  , optional_(optional_)
  , message_(message_)
  , entities_(std::move(entities_))
  , url_(url_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputBotInlineMessageMediaWebPage::ID;

void inputBotInlineMessageMediaWebPage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (invert_media_ << 3) | (force_large_media_ << 4) | (force_small_media_ << 5) | (optional_ << 6)), s);
  TlStoreString::store(message_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  TlStoreString::store(url_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaWebPage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (invert_media_ << 3) | (force_large_media_ << 4) | (force_small_media_ << 5) | (optional_ << 6)), s);
  TlStoreString::store(message_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  TlStoreString::store(url_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_markup_, s); }
}

void inputBotInlineMessageMediaWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBotInlineMessageMediaWebPage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (invert_media_ << 3) | (force_large_media_ << 4) | (force_small_media_ << 5) | (optional_ << 6)));
    if (var0 & 8) { s.store_field("invert_media", true); }
    if (var0 & 16) { s.store_field("force_large_media", true); }
    if (var0 & 32) { s.store_field("force_small_media", true); }
    if (var0 & 64) { s.store_field("optional", true); }
    s.store_field("message", message_);
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("url", url_);
    if (var0 & 4) { s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get())); }
    s.store_class_end();
  }
}

inputBusinessBotRecipients::inputBusinessBotRecipients(int32 flags_, bool existing_chats_, bool new_chats_, bool contacts_, bool non_contacts_, bool exclude_selected_, array<object_ptr<InputUser>> &&users_, array<object_ptr<InputUser>> &&exclude_users_)
  : flags_(flags_)
  , existing_chats_(existing_chats_)
  , new_chats_(new_chats_)
  , contacts_(contacts_)
  , non_contacts_(non_contacts_)
  , exclude_selected_(exclude_selected_)
  , users_(std::move(users_))
  , exclude_users_(std::move(exclude_users_))
{}

const std::int32_t inputBusinessBotRecipients::ID;

void inputBusinessBotRecipients::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)), s);
  if (var0 & 16) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s); }
  if (var0 & 64) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(exclude_users_, s); }
}

void inputBusinessBotRecipients::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)), s);
  if (var0 & 16) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s); }
  if (var0 & 64) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(exclude_users_, s); }
}

void inputBusinessBotRecipients::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessBotRecipients");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)));
    if (var0 & 1) { s.store_field("existing_chats", true); }
    if (var0 & 2) { s.store_field("new_chats", true); }
    if (var0 & 4) { s.store_field("contacts", true); }
    if (var0 & 8) { s.store_field("non_contacts", true); }
    if (var0 & 32) { s.store_field("exclude_selected", true); }
    if (var0 & 16) { { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 64) { { s.store_vector_begin("exclude_users", exclude_users_.size()); for (const auto &_value : exclude_users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<InputReplyTo> InputReplyTo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputReplyToMessage::ID:
      return inputReplyToMessage::fetch(p);
    case inputReplyToStory::ID:
      return inputReplyToStory::fetch(p);
    case inputReplyToMonoForum::ID:
      return inputReplyToMonoForum::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

inputReplyToMessage::inputReplyToMessage()
  : flags_()
  , reply_to_msg_id_()
  , top_msg_id_()
  , reply_to_peer_id_()
  , quote_text_()
  , quote_entities_()
  , quote_offset_()
  , monoforum_peer_id_()
  , todo_item_id_()
{}

inputReplyToMessage::inputReplyToMessage(int32 flags_, int32 reply_to_msg_id_, int32 top_msg_id_, object_ptr<InputPeer> &&reply_to_peer_id_, string const &quote_text_, array<object_ptr<MessageEntity>> &&quote_entities_, int32 quote_offset_, object_ptr<InputPeer> &&monoforum_peer_id_, int32 todo_item_id_)
  : flags_(flags_)
  , reply_to_msg_id_(reply_to_msg_id_)
  , top_msg_id_(top_msg_id_)
  , reply_to_peer_id_(std::move(reply_to_peer_id_))
  , quote_text_(quote_text_)
  , quote_entities_(std::move(quote_entities_))
  , quote_offset_(quote_offset_)
  , monoforum_peer_id_(std::move(monoforum_peer_id_))
  , todo_item_id_(todo_item_id_)
{}

const std::int32_t inputReplyToMessage::ID;

object_ptr<InputReplyTo> inputReplyToMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputReplyToMessage> res = make_tl_object<inputReplyToMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->reply_to_msg_id_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->top_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->reply_to_peer_id_ = TlFetchObject<InputPeer>::parse(p); }
  if (var0 & 4) { res->quote_text_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->quote_entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 16) { res->quote_offset_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->monoforum_peer_id_ = TlFetchObject<InputPeer>::parse(p); }
  if (var0 & 64) { res->todo_item_id_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputReplyToMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(reply_to_msg_id_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_peer_id_, s); }
  if (var0 & 4) { TlStoreString::store(quote_text_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(quote_entities_, s); }
  if (var0 & 16) { TlStoreBinary::store(quote_offset_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(monoforum_peer_id_, s); }
  if (var0 & 64) { TlStoreBinary::store(todo_item_id_, s); }
}

void inputReplyToMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(reply_to_msg_id_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_peer_id_, s); }
  if (var0 & 4) { TlStoreString::store(quote_text_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(quote_entities_, s); }
  if (var0 & 16) { TlStoreBinary::store(quote_offset_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(monoforum_peer_id_, s); }
  if (var0 & 64) { TlStoreBinary::store(todo_item_id_, s); }
}

void inputReplyToMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReplyToMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("reply_to_msg_id", reply_to_msg_id_);
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 2) { s.store_object_field("reply_to_peer_id", static_cast<const BaseObject *>(reply_to_peer_id_.get())); }
    if (var0 & 4) { s.store_field("quote_text", quote_text_); }
    if (var0 & 8) { { s.store_vector_begin("quote_entities", quote_entities_.size()); for (const auto &_value : quote_entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 16) { s.store_field("quote_offset", quote_offset_); }
    if (var0 & 32) { s.store_object_field("monoforum_peer_id", static_cast<const BaseObject *>(monoforum_peer_id_.get())); }
    if (var0 & 64) { s.store_field("todo_item_id", todo_item_id_); }
    s.store_class_end();
  }
}

inputReplyToStory::inputReplyToStory(object_ptr<InputPeer> &&peer_, int32 story_id_)
  : peer_(std::move(peer_))
  , story_id_(story_id_)
{}

const std::int32_t inputReplyToStory::ID;

object_ptr<InputReplyTo> inputReplyToStory::fetch(TlBufferParser &p) {
  return make_tl_object<inputReplyToStory>(p);
}

inputReplyToStory::inputReplyToStory(TlBufferParser &p)
  : peer_(TlFetchObject<InputPeer>::parse(p))
  , story_id_(TlFetchInt::parse(p))
{}

void inputReplyToStory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(story_id_, s);
}

void inputReplyToStory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(story_id_, s);
}

void inputReplyToStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReplyToStory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

inputReplyToMonoForum::inputReplyToMonoForum(object_ptr<InputPeer> &&monoforum_peer_id_)
  : monoforum_peer_id_(std::move(monoforum_peer_id_))
{}

const std::int32_t inputReplyToMonoForum::ID;

object_ptr<InputReplyTo> inputReplyToMonoForum::fetch(TlBufferParser &p) {
  return make_tl_object<inputReplyToMonoForum>(p);
}

inputReplyToMonoForum::inputReplyToMonoForum(TlBufferParser &p)
  : monoforum_peer_id_(TlFetchObject<InputPeer>::parse(p))
{}

void inputReplyToMonoForum::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(monoforum_peer_id_, s);
}

void inputReplyToMonoForum::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(monoforum_peer_id_, s);
}

void inputReplyToMonoForum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputReplyToMonoForum");
    s.store_object_field("monoforum_peer_id", static_cast<const BaseObject *>(monoforum_peer_id_.get()));
    s.store_class_end();
  }
}

object_ptr<KeyboardButton> KeyboardButton::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

keyboardButton::keyboardButton(string const &text_)
  : text_(text_)
{}

const std::int32_t keyboardButton::ID;

object_ptr<KeyboardButton> keyboardButton::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButton>(p);
}

keyboardButton::keyboardButton(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
{}

void keyboardButton::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButton::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButton");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

keyboardButtonUrl::keyboardButtonUrl(string const &text_, string const &url_)
  : text_(text_)
  , url_(url_)
{}

const std::int32_t keyboardButtonUrl::ID;

object_ptr<KeyboardButton> keyboardButtonUrl::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonUrl>(p);
}

keyboardButtonUrl::keyboardButtonUrl(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void keyboardButtonUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void keyboardButtonUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void keyboardButtonUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonUrl");
    s.store_field("text", text_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

keyboardButtonCallback::keyboardButtonCallback()
  : flags_()
  , requires_password_()
  , text_()
  , data_()
{}

keyboardButtonCallback::keyboardButtonCallback(int32 flags_, bool requires_password_, string const &text_, bytes &&data_)
  : flags_(flags_)
  , requires_password_(requires_password_)
  , text_(text_)
  , data_(std::move(data_))
{}

const std::int32_t keyboardButtonCallback::ID;

object_ptr<KeyboardButton> keyboardButtonCallback::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<keyboardButtonCallback> res = make_tl_object<keyboardButtonCallback>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->requires_password_ = (var0 & 1) != 0;
  res->text_ = TlFetchString<string>::parse(p);
  res->data_ = TlFetchBytes<bytes>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void keyboardButtonCallback::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (requires_password_ << 0)), s);
  TlStoreString::store(text_, s);
  TlStoreString::store(data_, s);
}

void keyboardButtonCallback::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (requires_password_ << 0)), s);
  TlStoreString::store(text_, s);
  TlStoreString::store(data_, s);
}

void keyboardButtonCallback::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonCallback");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (requires_password_ << 0)));
    if (var0 & 1) { s.store_field("requires_password", true); }
    s.store_field("text", text_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

keyboardButtonRequestPhone::keyboardButtonRequestPhone(string const &text_)
  : text_(text_)
{}

const std::int32_t keyboardButtonRequestPhone::ID;

object_ptr<KeyboardButton> keyboardButtonRequestPhone::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonRequestPhone>(p);
}

keyboardButtonRequestPhone::keyboardButtonRequestPhone(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
{}

void keyboardButtonRequestPhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonRequestPhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonRequestPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonRequestPhone");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

keyboardButtonRequestGeoLocation::keyboardButtonRequestGeoLocation(string const &text_)
  : text_(text_)
{}

const std::int32_t keyboardButtonRequestGeoLocation::ID;

object_ptr<KeyboardButton> keyboardButtonRequestGeoLocation::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonRequestGeoLocation>(p);
}

keyboardButtonRequestGeoLocation::keyboardButtonRequestGeoLocation(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
{}

void keyboardButtonRequestGeoLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonRequestGeoLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonRequestGeoLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonRequestGeoLocation");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

keyboardButtonSwitchInline::keyboardButtonSwitchInline()
  : flags_()
  , same_peer_()
  , text_()
  , query_()
  , peer_types_()
{}

keyboardButtonSwitchInline::keyboardButtonSwitchInline(int32 flags_, bool same_peer_, string const &text_, string const &query_, array<object_ptr<InlineQueryPeerType>> &&peer_types_)
  : flags_(flags_)
  , same_peer_(same_peer_)
  , text_(text_)
  , query_(query_)
  , peer_types_(std::move(peer_types_))
{}

const std::int32_t keyboardButtonSwitchInline::ID;

object_ptr<KeyboardButton> keyboardButtonSwitchInline::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<keyboardButtonSwitchInline> res = make_tl_object<keyboardButtonSwitchInline>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->same_peer_ = (var0 & 1) != 0;
  res->text_ = TlFetchString<string>::parse(p);
  res->query_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->peer_types_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InlineQueryPeerType>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void keyboardButtonSwitchInline::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (same_peer_ << 0)), s);
  TlStoreString::store(text_, s);
  TlStoreString::store(query_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peer_types_, s); }
}

void keyboardButtonSwitchInline::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (same_peer_ << 0)), s);
  TlStoreString::store(text_, s);
  TlStoreString::store(query_, s);
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(peer_types_, s); }
}

void keyboardButtonSwitchInline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonSwitchInline");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (same_peer_ << 0)));
    if (var0 & 1) { s.store_field("same_peer", true); }
    s.store_field("text", text_);
    s.store_field("query", query_);
    if (var0 & 2) { { s.store_vector_begin("peer_types", peer_types_.size()); for (const auto &_value : peer_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

keyboardButtonGame::keyboardButtonGame(string const &text_)
  : text_(text_)
{}

const std::int32_t keyboardButtonGame::ID;

object_ptr<KeyboardButton> keyboardButtonGame::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonGame>(p);
}

keyboardButtonGame::keyboardButtonGame(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
{}

void keyboardButtonGame::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonGame::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonGame");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

keyboardButtonBuy::keyboardButtonBuy(string const &text_)
  : text_(text_)
{}

const std::int32_t keyboardButtonBuy::ID;

object_ptr<KeyboardButton> keyboardButtonBuy::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonBuy>(p);
}

keyboardButtonBuy::keyboardButtonBuy(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
{}

void keyboardButtonBuy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonBuy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
}

void keyboardButtonBuy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonBuy");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

keyboardButtonUrlAuth::keyboardButtonUrlAuth()
  : flags_()
  , text_()
  , fwd_text_()
  , url_()
  , button_id_()
{}

keyboardButtonUrlAuth::keyboardButtonUrlAuth(int32 flags_, string const &text_, string const &fwd_text_, string const &url_, int32 button_id_)
  : flags_(flags_)
  , text_(text_)
  , fwd_text_(fwd_text_)
  , url_(url_)
  , button_id_(button_id_)
{}

const std::int32_t keyboardButtonUrlAuth::ID;

object_ptr<KeyboardButton> keyboardButtonUrlAuth::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<keyboardButtonUrlAuth> res = make_tl_object<keyboardButtonUrlAuth>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->text_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->fwd_text_ = TlFetchString<string>::parse(p); }
  res->url_ = TlFetchString<string>::parse(p);
  res->button_id_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void keyboardButtonUrlAuth::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(text_, s);
  if (var0 & 1) { TlStoreString::store(fwd_text_, s); }
  TlStoreString::store(url_, s);
  TlStoreBinary::store(button_id_, s);
}

void keyboardButtonUrlAuth::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(text_, s);
  if (var0 & 1) { TlStoreString::store(fwd_text_, s); }
  TlStoreString::store(url_, s);
  TlStoreBinary::store(button_id_, s);
}

void keyboardButtonUrlAuth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonUrlAuth");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("text", text_);
    if (var0 & 1) { s.store_field("fwd_text", fwd_text_); }
    s.store_field("url", url_);
    s.store_field("button_id", button_id_);
    s.store_class_end();
  }
}

inputKeyboardButtonUrlAuth::inputKeyboardButtonUrlAuth()
  : flags_()
  , request_write_access_()
  , text_()
  , fwd_text_()
  , url_()
  , bot_()
{}

inputKeyboardButtonUrlAuth::inputKeyboardButtonUrlAuth(int32 flags_, bool request_write_access_, string const &text_, string const &fwd_text_, string const &url_, object_ptr<InputUser> &&bot_)
  : flags_(flags_)
  , request_write_access_(request_write_access_)
  , text_(text_)
  , fwd_text_(fwd_text_)
  , url_(url_)
  , bot_(std::move(bot_))
{}

const std::int32_t inputKeyboardButtonUrlAuth::ID;

object_ptr<KeyboardButton> inputKeyboardButtonUrlAuth::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputKeyboardButtonUrlAuth> res = make_tl_object<inputKeyboardButtonUrlAuth>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->request_write_access_ = (var0 & 1) != 0;
  res->text_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->fwd_text_ = TlFetchString<string>::parse(p); }
  res->url_ = TlFetchString<string>::parse(p);
  res->bot_ = TlFetchObject<InputUser>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputKeyboardButtonUrlAuth::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (request_write_access_ << 0)), s);
  TlStoreString::store(text_, s);
  if (var0 & 2) { TlStoreString::store(fwd_text_, s); }
  TlStoreString::store(url_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void inputKeyboardButtonUrlAuth::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (request_write_access_ << 0)), s);
  TlStoreString::store(text_, s);
  if (var0 & 2) { TlStoreString::store(fwd_text_, s); }
  TlStoreString::store(url_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void inputKeyboardButtonUrlAuth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputKeyboardButtonUrlAuth");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (request_write_access_ << 0)));
    if (var0 & 1) { s.store_field("request_write_access", true); }
    s.store_field("text", text_);
    if (var0 & 2) { s.store_field("fwd_text", fwd_text_); }
    s.store_field("url", url_);
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

keyboardButtonRequestPoll::keyboardButtonRequestPoll()
  : flags_()
  , quiz_()
  , text_()
{}

keyboardButtonRequestPoll::keyboardButtonRequestPoll(int32 flags_, bool quiz_, string const &text_)
  : flags_(flags_)
  , quiz_(quiz_)
  , text_(text_)
{}

const std::int32_t keyboardButtonRequestPoll::ID;

object_ptr<KeyboardButton> keyboardButtonRequestPoll::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<keyboardButtonRequestPoll> res = make_tl_object<keyboardButtonRequestPoll>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->quiz_ = TlFetchBool::parse(p); }
  res->text_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void keyboardButtonRequestPoll::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBool::store(quiz_, s); }
  TlStoreString::store(text_, s);
}

void keyboardButtonRequestPoll::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBool::store(quiz_, s); }
  TlStoreString::store(text_, s);
}

void keyboardButtonRequestPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonRequestPoll");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("quiz", quiz_); }
    s.store_field("text", text_);
    s.store_class_end();
  }
}

inputKeyboardButtonUserProfile::inputKeyboardButtonUserProfile(string const &text_, object_ptr<InputUser> &&user_id_)
  : text_(text_)
  , user_id_(std::move(user_id_))
{}

const std::int32_t inputKeyboardButtonUserProfile::ID;

object_ptr<KeyboardButton> inputKeyboardButtonUserProfile::fetch(TlBufferParser &p) {
  return make_tl_object<inputKeyboardButtonUserProfile>(p);
}

inputKeyboardButtonUserProfile::inputKeyboardButtonUserProfile(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , user_id_(TlFetchObject<InputUser>::parse(p))
{}

void inputKeyboardButtonUserProfile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void inputKeyboardButtonUserProfile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void inputKeyboardButtonUserProfile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputKeyboardButtonUserProfile");
    s.store_field("text", text_);
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

keyboardButtonUserProfile::keyboardButtonUserProfile(string const &text_, int64 user_id_)
  : text_(text_)
  , user_id_(user_id_)
{}

const std::int32_t keyboardButtonUserProfile::ID;

object_ptr<KeyboardButton> keyboardButtonUserProfile::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonUserProfile>(p);
}

keyboardButtonUserProfile::keyboardButtonUserProfile(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , user_id_(TlFetchLong::parse(p))
{}

void keyboardButtonUserProfile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBinary::store(user_id_, s);
}

void keyboardButtonUserProfile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBinary::store(user_id_, s);
}

void keyboardButtonUserProfile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonUserProfile");
    s.store_field("text", text_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

keyboardButtonWebView::keyboardButtonWebView(string const &text_, string const &url_)
  : text_(text_)
  , url_(url_)
{}

const std::int32_t keyboardButtonWebView::ID;

object_ptr<KeyboardButton> keyboardButtonWebView::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonWebView>(p);
}

keyboardButtonWebView::keyboardButtonWebView(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void keyboardButtonWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void keyboardButtonWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void keyboardButtonWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonWebView");
    s.store_field("text", text_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

keyboardButtonSimpleWebView::keyboardButtonSimpleWebView(string const &text_, string const &url_)
  : text_(text_)
  , url_(url_)
{}

const std::int32_t keyboardButtonSimpleWebView::ID;

object_ptr<KeyboardButton> keyboardButtonSimpleWebView::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonSimpleWebView>(p);
}

keyboardButtonSimpleWebView::keyboardButtonSimpleWebView(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , url_(TlFetchString<string>::parse(p))
{}

void keyboardButtonSimpleWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void keyboardButtonSimpleWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(url_, s);
}

void keyboardButtonSimpleWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonSimpleWebView");
    s.store_field("text", text_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

keyboardButtonRequestPeer::keyboardButtonRequestPeer(string const &text_, int32 button_id_, object_ptr<RequestPeerType> &&peer_type_, int32 max_quantity_)
  : text_(text_)
  , button_id_(button_id_)
  , peer_type_(std::move(peer_type_))
  , max_quantity_(max_quantity_)
{}

const std::int32_t keyboardButtonRequestPeer::ID;

object_ptr<KeyboardButton> keyboardButtonRequestPeer::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonRequestPeer>(p);
}

keyboardButtonRequestPeer::keyboardButtonRequestPeer(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , button_id_(TlFetchInt::parse(p))
  , peer_type_(TlFetchObject<RequestPeerType>::parse(p))
  , max_quantity_(TlFetchInt::parse(p))
{}

void keyboardButtonRequestPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBinary::store(button_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_type_, s);
  TlStoreBinary::store(max_quantity_, s);
}

void keyboardButtonRequestPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBinary::store(button_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_type_, s);
  TlStoreBinary::store(max_quantity_, s);
}

void keyboardButtonRequestPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonRequestPeer");
    s.store_field("text", text_);
    s.store_field("button_id", button_id_);
    s.store_object_field("peer_type", static_cast<const BaseObject *>(peer_type_.get()));
    s.store_field("max_quantity", max_quantity_);
    s.store_class_end();
  }
}

inputKeyboardButtonRequestPeer::inputKeyboardButtonRequestPeer()
  : flags_()
  , name_requested_()
  , username_requested_()
  , photo_requested_()
  , text_()
  , button_id_()
  , peer_type_()
  , max_quantity_()
{}

inputKeyboardButtonRequestPeer::inputKeyboardButtonRequestPeer(int32 flags_, bool name_requested_, bool username_requested_, bool photo_requested_, string const &text_, int32 button_id_, object_ptr<RequestPeerType> &&peer_type_, int32 max_quantity_)
  : flags_(flags_)
  , name_requested_(name_requested_)
  , username_requested_(username_requested_)
  , photo_requested_(photo_requested_)
  , text_(text_)
  , button_id_(button_id_)
  , peer_type_(std::move(peer_type_))
  , max_quantity_(max_quantity_)
{}

const std::int32_t inputKeyboardButtonRequestPeer::ID;

object_ptr<KeyboardButton> inputKeyboardButtonRequestPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputKeyboardButtonRequestPeer> res = make_tl_object<inputKeyboardButtonRequestPeer>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->name_requested_ = (var0 & 1) != 0;
  res->username_requested_ = (var0 & 2) != 0;
  res->photo_requested_ = (var0 & 4) != 0;
  res->text_ = TlFetchString<string>::parse(p);
  res->button_id_ = TlFetchInt::parse(p);
  res->peer_type_ = TlFetchObject<RequestPeerType>::parse(p);
  res->max_quantity_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputKeyboardButtonRequestPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (name_requested_ << 0) | (username_requested_ << 1) | (photo_requested_ << 2)), s);
  TlStoreString::store(text_, s);
  TlStoreBinary::store(button_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_type_, s);
  TlStoreBinary::store(max_quantity_, s);
}

void inputKeyboardButtonRequestPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (name_requested_ << 0) | (username_requested_ << 1) | (photo_requested_ << 2)), s);
  TlStoreString::store(text_, s);
  TlStoreBinary::store(button_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_type_, s);
  TlStoreBinary::store(max_quantity_, s);
}

void inputKeyboardButtonRequestPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputKeyboardButtonRequestPeer");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (name_requested_ << 0) | (username_requested_ << 1) | (photo_requested_ << 2)));
    if (var0 & 1) { s.store_field("name_requested", true); }
    if (var0 & 2) { s.store_field("username_requested", true); }
    if (var0 & 4) { s.store_field("photo_requested", true); }
    s.store_field("text", text_);
    s.store_field("button_id", button_id_);
    s.store_object_field("peer_type", static_cast<const BaseObject *>(peer_type_.get()));
    s.store_field("max_quantity", max_quantity_);
    s.store_class_end();
  }
}

keyboardButtonCopy::keyboardButtonCopy(string const &text_, string const &copy_text_)
  : text_(text_)
  , copy_text_(copy_text_)
{}

const std::int32_t keyboardButtonCopy::ID;

object_ptr<KeyboardButton> keyboardButtonCopy::fetch(TlBufferParser &p) {
  return make_tl_object<keyboardButtonCopy>(p);
}

keyboardButtonCopy::keyboardButtonCopy(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , copy_text_(TlFetchString<string>::parse(p))
{}

void keyboardButtonCopy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(copy_text_, s);
}

void keyboardButtonCopy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreString::store(copy_text_, s);
}

void keyboardButtonCopy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonCopy");
    s.store_field("text", text_);
    s.store_field("copy_text", copy_text_);
    s.store_class_end();
  }
}

object_ptr<MessageAction> MessageAction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messageActionEmpty::ID;

object_ptr<MessageAction> messageActionEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionEmpty>();
}

void messageActionEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionEmpty");
    s.store_class_end();
  }
}

const std::int32_t messageActionChatCreate::ID;

object_ptr<MessageAction> messageActionChatCreate::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatCreate>(p);
}

messageActionChatCreate::messageActionChatCreate(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void messageActionChatCreate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatCreate");
    s.store_field("title", title_);
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messageActionChatEditTitle::ID;

object_ptr<MessageAction> messageActionChatEditTitle::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatEditTitle>(p);
}

messageActionChatEditTitle::messageActionChatEditTitle(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
{}

void messageActionChatEditTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatEditTitle");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

const std::int32_t messageActionChatEditPhoto::ID;

object_ptr<MessageAction> messageActionChatEditPhoto::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatEditPhoto>(p);
}

messageActionChatEditPhoto::messageActionChatEditPhoto(TlBufferParser &p)
  : photo_(TlFetchObject<Photo>::parse(p))
{}

void messageActionChatEditPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatEditPhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

const std::int32_t messageActionChatDeletePhoto::ID;

object_ptr<MessageAction> messageActionChatDeletePhoto::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatDeletePhoto>();
}

void messageActionChatDeletePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatDeletePhoto");
    s.store_class_end();
  }
}

const std::int32_t messageActionChatAddUser::ID;

object_ptr<MessageAction> messageActionChatAddUser::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatAddUser>(p);
}

messageActionChatAddUser::messageActionChatAddUser(TlBufferParser &p)
  : users_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void messageActionChatAddUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatAddUser");
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messageActionChatDeleteUser::ID;

object_ptr<MessageAction> messageActionChatDeleteUser::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatDeleteUser>(p);
}

messageActionChatDeleteUser::messageActionChatDeleteUser(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
{}

void messageActionChatDeleteUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatDeleteUser");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

const std::int32_t messageActionChatJoinedByLink::ID;

object_ptr<MessageAction> messageActionChatJoinedByLink::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatJoinedByLink>(p);
}

messageActionChatJoinedByLink::messageActionChatJoinedByLink(TlBufferParser &p)
  : inviter_id_(TlFetchLong::parse(p))
{}

void messageActionChatJoinedByLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatJoinedByLink");
    s.store_field("inviter_id", inviter_id_);
    s.store_class_end();
  }
}

const std::int32_t messageActionChannelCreate::ID;

object_ptr<MessageAction> messageActionChannelCreate::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChannelCreate>(p);
}

messageActionChannelCreate::messageActionChannelCreate(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
{}

void messageActionChannelCreate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChannelCreate");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

const std::int32_t messageActionChatMigrateTo::ID;

object_ptr<MessageAction> messageActionChatMigrateTo::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatMigrateTo>(p);
}

messageActionChatMigrateTo::messageActionChatMigrateTo(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
{}

void messageActionChatMigrateTo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatMigrateTo");
    s.store_field("channel_id", channel_id_);
    s.store_class_end();
  }
}

const std::int32_t messageActionChannelMigrateFrom::ID;

object_ptr<MessageAction> messageActionChannelMigrateFrom::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChannelMigrateFrom>(p);
}

messageActionChannelMigrateFrom::messageActionChannelMigrateFrom(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , chat_id_(TlFetchLong::parse(p))
{}

void messageActionChannelMigrateFrom::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChannelMigrateFrom");
    s.store_field("title", title_);
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

const std::int32_t messageActionPinMessage::ID;

object_ptr<MessageAction> messageActionPinMessage::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionPinMessage>();
}

void messageActionPinMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPinMessage");
    s.store_class_end();
  }
}

const std::int32_t messageActionHistoryClear::ID;

object_ptr<MessageAction> messageActionHistoryClear::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionHistoryClear>();
}

void messageActionHistoryClear::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionHistoryClear");
    s.store_class_end();
  }
}

const std::int32_t messageActionGameScore::ID;

object_ptr<MessageAction> messageActionGameScore::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionGameScore>(p);
}

messageActionGameScore::messageActionGameScore(TlBufferParser &p)
  : game_id_(TlFetchLong::parse(p))
  , score_(TlFetchInt::parse(p))
{}

void messageActionGameScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGameScore");
    s.store_field("game_id", game_id_);
    s.store_field("score", score_);
    s.store_class_end();
  }
}

messageActionPaymentSentMe::messageActionPaymentSentMe()
  : flags_()
  , recurring_init_()
  , recurring_used_()
  , currency_()
  , total_amount_()
  , payload_()
  , info_()
  , shipping_option_id_()
  , charge_()
  , subscription_until_date_()
{}

const std::int32_t messageActionPaymentSentMe::ID;

object_ptr<MessageAction> messageActionPaymentSentMe::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionPaymentSentMe> res = make_tl_object<messageActionPaymentSentMe>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->recurring_init_ = (var0 & 4) != 0;
  res->recurring_used_ = (var0 & 8) != 0;
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  res->payload_ = TlFetchBytes<bytes>::parse(p);
  if (var0 & 1) { res->info_ = TlFetchBoxed<TlFetchObject<paymentRequestedInfo>, -1868808300>::parse(p); }
  if (var0 & 2) { res->shipping_option_id_ = TlFetchString<string>::parse(p); }
  res->charge_ = TlFetchBoxed<TlFetchObject<paymentCharge>, -368917890>::parse(p);
  if (var0 & 16) { res->subscription_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionPaymentSentMe::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPaymentSentMe");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (recurring_init_ << 2) | (recurring_used_ << 3)));
    if (var0 & 4) { s.store_field("recurring_init", true); }
    if (var0 & 8) { s.store_field("recurring_used", true); }
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_bytes_field("payload", payload_);
    if (var0 & 1) { s.store_object_field("info", static_cast<const BaseObject *>(info_.get())); }
    if (var0 & 2) { s.store_field("shipping_option_id", shipping_option_id_); }
    s.store_object_field("charge", static_cast<const BaseObject *>(charge_.get()));
    if (var0 & 16) { s.store_field("subscription_until_date", subscription_until_date_); }
    s.store_class_end();
  }
}

messageActionPaymentSent::messageActionPaymentSent()
  : flags_()
  , recurring_init_()
  , recurring_used_()
  , currency_()
  , total_amount_()
  , invoice_slug_()
  , subscription_until_date_()
{}

const std::int32_t messageActionPaymentSent::ID;

object_ptr<MessageAction> messageActionPaymentSent::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionPaymentSent> res = make_tl_object<messageActionPaymentSent>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->recurring_init_ = (var0 & 4) != 0;
  res->recurring_used_ = (var0 & 8) != 0;
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->invoice_slug_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->subscription_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionPaymentSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPaymentSent");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (recurring_init_ << 2) | (recurring_used_ << 3)));
    if (var0 & 4) { s.store_field("recurring_init", true); }
    if (var0 & 8) { s.store_field("recurring_used", true); }
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    if (var0 & 1) { s.store_field("invoice_slug", invoice_slug_); }
    if (var0 & 16) { s.store_field("subscription_until_date", subscription_until_date_); }
    s.store_class_end();
  }
}

messageActionPhoneCall::messageActionPhoneCall()
  : flags_()
  , video_()
  , call_id_()
  , reason_()
  , duration_()
{}

const std::int32_t messageActionPhoneCall::ID;

object_ptr<MessageAction> messageActionPhoneCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionPhoneCall> res = make_tl_object<messageActionPhoneCall>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->video_ = (var0 & 4) != 0;
  res->call_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->reason_ = TlFetchObject<PhoneCallDiscardReason>::parse(p); }
  if (var0 & 2) { res->duration_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionPhoneCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPhoneCall");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (video_ << 2)));
    if (var0 & 4) { s.store_field("video", true); }
    s.store_field("call_id", call_id_);
    if (var0 & 1) { s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get())); }
    if (var0 & 2) { s.store_field("duration", duration_); }
    s.store_class_end();
  }
}

const std::int32_t messageActionScreenshotTaken::ID;

object_ptr<MessageAction> messageActionScreenshotTaken::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionScreenshotTaken>();
}

void messageActionScreenshotTaken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionScreenshotTaken");
    s.store_class_end();
  }
}

const std::int32_t messageActionCustomAction::ID;

object_ptr<MessageAction> messageActionCustomAction::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionCustomAction>(p);
}

messageActionCustomAction::messageActionCustomAction(TlBufferParser &p)
  : message_(TlFetchString<string>::parse(p))
{}

void messageActionCustomAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionCustomAction");
    s.store_field("message", message_);
    s.store_class_end();
  }
}

messageActionBotAllowed::messageActionBotAllowed()
  : flags_()
  , attach_menu_()
  , from_request_()
  , domain_()
  , app_()
{}

const std::int32_t messageActionBotAllowed::ID;

object_ptr<MessageAction> messageActionBotAllowed::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionBotAllowed> res = make_tl_object<messageActionBotAllowed>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->attach_menu_ = (var0 & 2) != 0;
  res->from_request_ = (var0 & 8) != 0;
  if (var0 & 1) { res->domain_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->app_ = TlFetchObject<BotApp>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionBotAllowed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionBotAllowed");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (attach_menu_ << 1) | (from_request_ << 3)));
    if (var0 & 2) { s.store_field("attach_menu", true); }
    if (var0 & 8) { s.store_field("from_request", true); }
    if (var0 & 1) { s.store_field("domain", domain_); }
    if (var0 & 4) { s.store_object_field("app", static_cast<const BaseObject *>(app_.get())); }
    s.store_class_end();
  }
}

const std::int32_t messageActionSecureValuesSentMe::ID;

object_ptr<MessageAction> messageActionSecureValuesSentMe::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionSecureValuesSentMe>(p);
}

messageActionSecureValuesSentMe::messageActionSecureValuesSentMe(TlBufferParser &p)
  : values_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<secureValue>, 411017418>>, 481674261>::parse(p))
  , credentials_(TlFetchBoxed<TlFetchObject<secureCredentialsEncrypted>, 871426631>::parse(p))
{}

void messageActionSecureValuesSentMe::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSecureValuesSentMe");
    { s.store_vector_begin("values", values_.size()); for (const auto &_value : values_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("credentials", static_cast<const BaseObject *>(credentials_.get()));
    s.store_class_end();
  }
}

const std::int32_t messageActionSecureValuesSent::ID;

object_ptr<MessageAction> messageActionSecureValuesSent::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionSecureValuesSent>(p);
}

messageActionSecureValuesSent::messageActionSecureValuesSent(TlBufferParser &p)
  : types_(TlFetchBoxed<TlFetchVector<TlFetchObject<SecureValueType>>, 481674261>::parse(p))
{}

void messageActionSecureValuesSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSecureValuesSent");
    { s.store_vector_begin("types", types_.size()); for (const auto &_value : types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messageActionContactSignUp::ID;

object_ptr<MessageAction> messageActionContactSignUp::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionContactSignUp>();
}

void messageActionContactSignUp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionContactSignUp");
    s.store_class_end();
  }
}

const std::int32_t messageActionGeoProximityReached::ID;

object_ptr<MessageAction> messageActionGeoProximityReached::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionGeoProximityReached>(p);
}

messageActionGeoProximityReached::messageActionGeoProximityReached(TlBufferParser &p)
  : from_id_(TlFetchObject<Peer>::parse(p))
  , to_id_(TlFetchObject<Peer>::parse(p))
  , distance_(TlFetchInt::parse(p))
{}

void messageActionGeoProximityReached::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGeoProximityReached");
    s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get()));
    s.store_object_field("to_id", static_cast<const BaseObject *>(to_id_.get()));
    s.store_field("distance", distance_);
    s.store_class_end();
  }
}

messageActionGroupCall::messageActionGroupCall()
  : flags_()
  , call_()
  , duration_()
{}

const std::int32_t messageActionGroupCall::ID;

object_ptr<MessageAction> messageActionGroupCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionGroupCall> res = make_tl_object<messageActionGroupCall>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->call_ = TlFetchObject<InputGroupCall>::parse(p);
  if (var0 & 1) { res->duration_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGroupCall");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    if (var0 & 1) { s.store_field("duration", duration_); }
    s.store_class_end();
  }
}

const std::int32_t messageActionInviteToGroupCall::ID;

object_ptr<MessageAction> messageActionInviteToGroupCall::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionInviteToGroupCall>(p);
}

messageActionInviteToGroupCall::messageActionInviteToGroupCall(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void messageActionInviteToGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionInviteToGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageActionSetMessagesTTL::messageActionSetMessagesTTL()
  : flags_()
  , period_()
  , auto_setting_from_()
{}

const std::int32_t messageActionSetMessagesTTL::ID;

object_ptr<MessageAction> messageActionSetMessagesTTL::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionSetMessagesTTL> res = make_tl_object<messageActionSetMessagesTTL>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->period_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->auto_setting_from_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionSetMessagesTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSetMessagesTTL");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("period", period_);
    if (var0 & 1) { s.store_field("auto_setting_from", auto_setting_from_); }
    s.store_class_end();
  }
}

const std::int32_t messageActionGroupCallScheduled::ID;

object_ptr<MessageAction> messageActionGroupCallScheduled::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionGroupCallScheduled>(p);
}

messageActionGroupCallScheduled::messageActionGroupCallScheduled(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
  , schedule_date_(TlFetchInt::parse(p))
{}

void messageActionGroupCallScheduled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGroupCallScheduled");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("schedule_date", schedule_date_);
    s.store_class_end();
  }
}

const std::int32_t messageActionSetChatTheme::ID;

object_ptr<MessageAction> messageActionSetChatTheme::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionSetChatTheme>(p);
}

messageActionSetChatTheme::messageActionSetChatTheme(TlBufferParser &p)
  : theme_(TlFetchObject<ChatTheme>::parse(p))
{}

void messageActionSetChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSetChatTheme");
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

const std::int32_t messageActionChatJoinedByRequest::ID;

object_ptr<MessageAction> messageActionChatJoinedByRequest::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionChatJoinedByRequest>();
}

void messageActionChatJoinedByRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionChatJoinedByRequest");
    s.store_class_end();
  }
}

const std::int32_t messageActionWebViewDataSentMe::ID;

object_ptr<MessageAction> messageActionWebViewDataSentMe::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionWebViewDataSentMe>(p);
}

messageActionWebViewDataSentMe::messageActionWebViewDataSentMe(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , data_(TlFetchString<string>::parse(p))
{}

void messageActionWebViewDataSentMe::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionWebViewDataSentMe");
    s.store_field("text", text_);
    s.store_field("data", data_);
    s.store_class_end();
  }
}

const std::int32_t messageActionWebViewDataSent::ID;

object_ptr<MessageAction> messageActionWebViewDataSent::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionWebViewDataSent>(p);
}

messageActionWebViewDataSent::messageActionWebViewDataSent(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
{}

void messageActionWebViewDataSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionWebViewDataSent");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

messageActionGiftPremium::messageActionGiftPremium()
  : flags_()
  , currency_()
  , amount_()
  , months_()
  , crypto_currency_()
  , crypto_amount_()
  , message_()
{}

const std::int32_t messageActionGiftPremium::ID;

object_ptr<MessageAction> messageActionGiftPremium::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionGiftPremium> res = make_tl_object<messageActionGiftPremium>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchLong::parse(p);
  res->months_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->crypto_currency_ = TlFetchString<string>::parse(p); }
  if (var0 & 1) { res->crypto_amount_ = TlFetchLong::parse(p); }
  if (var0 & 2) { res->message_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionGiftPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGiftPremium");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("months", months_);
    if (var0 & 1) { s.store_field("crypto_currency", crypto_currency_); }
    if (var0 & 1) { s.store_field("crypto_amount", crypto_amount_); }
    if (var0 & 2) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

messageActionTopicCreate::messageActionTopicCreate()
  : flags_()
  , title_missing_()
  , title_()
  , icon_color_()
  , icon_emoji_id_()
{}

const std::int32_t messageActionTopicCreate::ID;

object_ptr<MessageAction> messageActionTopicCreate::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionTopicCreate> res = make_tl_object<messageActionTopicCreate>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->title_missing_ = (var0 & 2) != 0;
  res->title_ = TlFetchString<string>::parse(p);
  res->icon_color_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->icon_emoji_id_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionTopicCreate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionTopicCreate");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (title_missing_ << 1)));
    if (var0 & 2) { s.store_field("title_missing", true); }
    s.store_field("title", title_);
    s.store_field("icon_color", icon_color_);
    if (var0 & 1) { s.store_field("icon_emoji_id", icon_emoji_id_); }
    s.store_class_end();
  }
}

messageActionTopicEdit::messageActionTopicEdit()
  : flags_()
  , title_()
  , icon_emoji_id_()
  , closed_()
  , hidden_()
{}

const std::int32_t messageActionTopicEdit::ID;

object_ptr<MessageAction> messageActionTopicEdit::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionTopicEdit> res = make_tl_object<messageActionTopicEdit>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->icon_emoji_id_ = TlFetchLong::parse(p); }
  if (var0 & 4) { res->closed_ = TlFetchBool::parse(p); }
  if (var0 & 8) { res->hidden_ = TlFetchBool::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionTopicEdit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionTopicEdit");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("icon_emoji_id", icon_emoji_id_); }
    if (var0 & 4) { s.store_field("closed", closed_); }
    if (var0 & 8) { s.store_field("hidden", hidden_); }
    s.store_class_end();
  }
}

const std::int32_t messageActionSuggestProfilePhoto::ID;

object_ptr<MessageAction> messageActionSuggestProfilePhoto::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionSuggestProfilePhoto>(p);
}

messageActionSuggestProfilePhoto::messageActionSuggestProfilePhoto(TlBufferParser &p)
  : photo_(TlFetchObject<Photo>::parse(p))
{}

void messageActionSuggestProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSuggestProfilePhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

const std::int32_t messageActionRequestedPeer::ID;

object_ptr<MessageAction> messageActionRequestedPeer::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionRequestedPeer>(p);
}

messageActionRequestedPeer::messageActionRequestedPeer(TlBufferParser &p)
  : button_id_(TlFetchInt::parse(p))
  , peers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p))
{}

void messageActionRequestedPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionRequestedPeer");
    s.store_field("button_id", button_id_);
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageActionSetChatWallPaper::messageActionSetChatWallPaper()
  : flags_()
  , same_()
  , for_both_()
  , wallpaper_()
{}

const std::int32_t messageActionSetChatWallPaper::ID;

object_ptr<MessageAction> messageActionSetChatWallPaper::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionSetChatWallPaper> res = make_tl_object<messageActionSetChatWallPaper>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->same_ = (var0 & 1) != 0;
  res->for_both_ = (var0 & 2) != 0;
  res->wallpaper_ = TlFetchObject<WallPaper>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionSetChatWallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSetChatWallPaper");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (same_ << 0) | (for_both_ << 1)));
    if (var0 & 1) { s.store_field("same", true); }
    if (var0 & 2) { s.store_field("for_both", true); }
    s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get()));
    s.store_class_end();
  }
}

messageActionGiftCode::messageActionGiftCode()
  : flags_()
  , via_giveaway_()
  , unclaimed_()
  , boost_peer_()
  , months_()
  , slug_()
  , currency_()
  , amount_()
  , crypto_currency_()
  , crypto_amount_()
  , message_()
{}

const std::int32_t messageActionGiftCode::ID;

object_ptr<MessageAction> messageActionGiftCode::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionGiftCode> res = make_tl_object<messageActionGiftCode>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->via_giveaway_ = (var0 & 1) != 0;
  res->unclaimed_ = (var0 & 32) != 0;
  if (var0 & 2) { res->boost_peer_ = TlFetchObject<Peer>::parse(p); }
  res->months_ = TlFetchInt::parse(p);
  res->slug_ = TlFetchString<string>::parse(p);
  if (var0 & 4) { res->currency_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->amount_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->crypto_currency_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->crypto_amount_ = TlFetchLong::parse(p); }
  if (var0 & 16) { res->message_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGiftCode");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (via_giveaway_ << 0) | (unclaimed_ << 5)));
    if (var0 & 1) { s.store_field("via_giveaway", true); }
    if (var0 & 32) { s.store_field("unclaimed", true); }
    if (var0 & 2) { s.store_object_field("boost_peer", static_cast<const BaseObject *>(boost_peer_.get())); }
    s.store_field("months", months_);
    s.store_field("slug", slug_);
    if (var0 & 4) { s.store_field("currency", currency_); }
    if (var0 & 4) { s.store_field("amount", amount_); }
    if (var0 & 8) { s.store_field("crypto_currency", crypto_currency_); }
    if (var0 & 8) { s.store_field("crypto_amount", crypto_amount_); }
    if (var0 & 16) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

messageActionGiveawayLaunch::messageActionGiveawayLaunch()
  : flags_()
  , stars_()
{}

const std::int32_t messageActionGiveawayLaunch::ID;

object_ptr<MessageAction> messageActionGiveawayLaunch::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionGiveawayLaunch> res = make_tl_object<messageActionGiveawayLaunch>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->stars_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionGiveawayLaunch::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGiveawayLaunch");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("stars", stars_); }
    s.store_class_end();
  }
}

messageActionGiveawayResults::messageActionGiveawayResults()
  : flags_()
  , stars_()
  , winners_count_()
  , unclaimed_count_()
{}

const std::int32_t messageActionGiveawayResults::ID;

object_ptr<MessageAction> messageActionGiveawayResults::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionGiveawayResults> res = make_tl_object<messageActionGiveawayResults>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->stars_ = (var0 & 1) != 0;
  res->winners_count_ = TlFetchInt::parse(p);
  res->unclaimed_count_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionGiveawayResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGiveawayResults");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (stars_ << 0)));
    if (var0 & 1) { s.store_field("stars", true); }
    s.store_field("winners_count", winners_count_);
    s.store_field("unclaimed_count", unclaimed_count_);
    s.store_class_end();
  }
}

const std::int32_t messageActionBoostApply::ID;

object_ptr<MessageAction> messageActionBoostApply::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionBoostApply>(p);
}

messageActionBoostApply::messageActionBoostApply(TlBufferParser &p)
  : boosts_(TlFetchInt::parse(p))
{}

void messageActionBoostApply::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionBoostApply");
    s.store_field("boosts", boosts_);
    s.store_class_end();
  }
}

const std::int32_t messageActionRequestedPeerSentMe::ID;

object_ptr<MessageAction> messageActionRequestedPeerSentMe::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionRequestedPeerSentMe>(p);
}

messageActionRequestedPeerSentMe::messageActionRequestedPeerSentMe(TlBufferParser &p)
  : button_id_(TlFetchInt::parse(p))
  , peers_(TlFetchBoxed<TlFetchVector<TlFetchObject<RequestedPeer>>, 481674261>::parse(p))
{}

void messageActionRequestedPeerSentMe::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionRequestedPeerSentMe");
    s.store_field("button_id", button_id_);
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageActionPaymentRefunded::messageActionPaymentRefunded()
  : flags_()
  , peer_()
  , currency_()
  , total_amount_()
  , payload_()
  , charge_()
{}

const std::int32_t messageActionPaymentRefunded::ID;

object_ptr<MessageAction> messageActionPaymentRefunded::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionPaymentRefunded> res = make_tl_object<messageActionPaymentRefunded>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->payload_ = TlFetchBytes<bytes>::parse(p); }
  res->charge_ = TlFetchBoxed<TlFetchObject<paymentCharge>, -368917890>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionPaymentRefunded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPaymentRefunded");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    if (var0 & 1) { s.store_bytes_field("payload", payload_); }
    s.store_object_field("charge", static_cast<const BaseObject *>(charge_.get()));
    s.store_class_end();
  }
}

messageActionGiftStars::messageActionGiftStars()
  : flags_()
  , currency_()
  , amount_()
  , stars_()
  , crypto_currency_()
  , crypto_amount_()
  , transaction_id_()
{}

const std::int32_t messageActionGiftStars::ID;

object_ptr<MessageAction> messageActionGiftStars::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionGiftStars> res = make_tl_object<messageActionGiftStars>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchLong::parse(p);
  res->stars_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->crypto_currency_ = TlFetchString<string>::parse(p); }
  if (var0 & 1) { res->crypto_amount_ = TlFetchLong::parse(p); }
  if (var0 & 2) { res->transaction_id_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionGiftStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGiftStars");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("stars", stars_);
    if (var0 & 1) { s.store_field("crypto_currency", crypto_currency_); }
    if (var0 & 1) { s.store_field("crypto_amount", crypto_amount_); }
    if (var0 & 2) { s.store_field("transaction_id", transaction_id_); }
    s.store_class_end();
  }
}

messageActionPrizeStars::messageActionPrizeStars()
  : flags_()
  , unclaimed_()
  , stars_()
  , transaction_id_()
  , boost_peer_()
  , giveaway_msg_id_()
{}

const std::int32_t messageActionPrizeStars::ID;

object_ptr<MessageAction> messageActionPrizeStars::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionPrizeStars> res = make_tl_object<messageActionPrizeStars>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->unclaimed_ = (var0 & 1) != 0;
  res->stars_ = TlFetchLong::parse(p);
  res->transaction_id_ = TlFetchString<string>::parse(p);
  res->boost_peer_ = TlFetchObject<Peer>::parse(p);
  res->giveaway_msg_id_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionPrizeStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPrizeStars");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (unclaimed_ << 0)));
    if (var0 & 1) { s.store_field("unclaimed", true); }
    s.store_field("stars", stars_);
    s.store_field("transaction_id", transaction_id_);
    s.store_object_field("boost_peer", static_cast<const BaseObject *>(boost_peer_.get()));
    s.store_field("giveaway_msg_id", giveaway_msg_id_);
    s.store_class_end();
  }
}

messageActionStarGift::messageActionStarGift()
  : flags_()
  , name_hidden_()
  , saved_()
  , converted_()
  , upgraded_()
  , refunded_()
  , can_upgrade_()
  , prepaid_upgrade_()
  , upgrade_separate_()
  , gift_()
  , message_()
  , convert_stars_()
  , upgrade_msg_id_()
  , upgrade_stars_()
  , from_id_()
  , peer_()
  , saved_id_()
  , prepaid_upgrade_hash_()
  , gift_msg_id_()
{}

const std::int32_t messageActionStarGift::ID;

object_ptr<MessageAction> messageActionStarGift::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionStarGift> res = make_tl_object<messageActionStarGift>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->name_hidden_ = (var0 & 1) != 0;
  res->saved_ = (var0 & 4) != 0;
  res->converted_ = (var0 & 8) != 0;
  res->upgraded_ = (var0 & 32) != 0;
  res->refunded_ = (var0 & 512) != 0;
  res->can_upgrade_ = (var0 & 1024) != 0;
  res->prepaid_upgrade_ = (var0 & 8192) != 0;
  res->upgrade_separate_ = (var0 & 65536) != 0;
  res->gift_ = TlFetchObject<StarGift>::parse(p);
  if (var0 & 2) { res->message_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p); }
  if (var0 & 16) { res->convert_stars_ = TlFetchLong::parse(p); }
  if (var0 & 32) { res->upgrade_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->upgrade_stars_ = TlFetchLong::parse(p); }
  if (var0 & 2048) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 4096) { res->peer_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 4096) { res->saved_id_ = TlFetchLong::parse(p); }
  if (var0 & 16384) { res->prepaid_upgrade_hash_ = TlFetchString<string>::parse(p); }
  if (var0 & 32768) { res->gift_msg_id_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionStarGift");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (name_hidden_ << 0) | (saved_ << 2) | (converted_ << 3) | (upgraded_ << 5) | (refunded_ << 9) | (can_upgrade_ << 10) | (prepaid_upgrade_ << 13) | (upgrade_separate_ << 16)));
    if (var0 & 1) { s.store_field("name_hidden", true); }
    if (var0 & 4) { s.store_field("saved", true); }
    if (var0 & 8) { s.store_field("converted", true); }
    if (var0 & 32) { s.store_field("upgraded", true); }
    if (var0 & 512) { s.store_field("refunded", true); }
    if (var0 & 1024) { s.store_field("can_upgrade", true); }
    if (var0 & 8192) { s.store_field("prepaid_upgrade", true); }
    if (var0 & 65536) { s.store_field("upgrade_separate", true); }
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    if (var0 & 2) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    if (var0 & 16) { s.store_field("convert_stars", convert_stars_); }
    if (var0 & 32) { s.store_field("upgrade_msg_id", upgrade_msg_id_); }
    if (var0 & 256) { s.store_field("upgrade_stars", upgrade_stars_); }
    if (var0 & 2048) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    if (var0 & 4096) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    if (var0 & 4096) { s.store_field("saved_id", saved_id_); }
    if (var0 & 16384) { s.store_field("prepaid_upgrade_hash", prepaid_upgrade_hash_); }
    if (var0 & 32768) { s.store_field("gift_msg_id", gift_msg_id_); }
    s.store_class_end();
  }
}

messageActionStarGiftUnique::messageActionStarGiftUnique()
  : flags_()
  , upgrade_()
  , transferred_()
  , saved_()
  , refunded_()
  , prepaid_upgrade_()
  , assigned_()
  , gift_()
  , can_export_at_()
  , transfer_stars_()
  , from_id_()
  , peer_()
  , saved_id_()
  , resale_amount_()
  , can_transfer_at_()
  , can_resell_at_()
  , drop_original_details_stars_()
{}

const std::int32_t messageActionStarGiftUnique::ID;

object_ptr<MessageAction> messageActionStarGiftUnique::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionStarGiftUnique> res = make_tl_object<messageActionStarGiftUnique>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->upgrade_ = (var0 & 1) != 0;
  res->transferred_ = (var0 & 2) != 0;
  res->saved_ = (var0 & 4) != 0;
  res->refunded_ = (var0 & 32) != 0;
  res->prepaid_upgrade_ = (var0 & 2048) != 0;
  res->assigned_ = (var0 & 8192) != 0;
  res->gift_ = TlFetchObject<StarGift>::parse(p);
  if (var0 & 8) { res->can_export_at_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->transfer_stars_ = TlFetchLong::parse(p); }
  if (var0 & 64) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 128) { res->peer_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 128) { res->saved_id_ = TlFetchLong::parse(p); }
  if (var0 & 256) { res->resale_amount_ = TlFetchObject<StarsAmount>::parse(p); }
  if (var0 & 512) { res->can_transfer_at_ = TlFetchInt::parse(p); }
  if (var0 & 1024) { res->can_resell_at_ = TlFetchInt::parse(p); }
  if (var0 & 4096) { res->drop_original_details_stars_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionStarGiftUnique::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionStarGiftUnique");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (upgrade_ << 0) | (transferred_ << 1) | (saved_ << 2) | (refunded_ << 5) | (prepaid_upgrade_ << 11) | (assigned_ << 13)));
    if (var0 & 1) { s.store_field("upgrade", true); }
    if (var0 & 2) { s.store_field("transferred", true); }
    if (var0 & 4) { s.store_field("saved", true); }
    if (var0 & 32) { s.store_field("refunded", true); }
    if (var0 & 2048) { s.store_field("prepaid_upgrade", true); }
    if (var0 & 8192) { s.store_field("assigned", true); }
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    if (var0 & 8) { s.store_field("can_export_at", can_export_at_); }
    if (var0 & 16) { s.store_field("transfer_stars", transfer_stars_); }
    if (var0 & 64) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    if (var0 & 128) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    if (var0 & 128) { s.store_field("saved_id", saved_id_); }
    if (var0 & 256) { s.store_object_field("resale_amount", static_cast<const BaseObject *>(resale_amount_.get())); }
    if (var0 & 512) { s.store_field("can_transfer_at", can_transfer_at_); }
    if (var0 & 1024) { s.store_field("can_resell_at", can_resell_at_); }
    if (var0 & 4096) { s.store_field("drop_original_details_stars", drop_original_details_stars_); }
    s.store_class_end();
  }
}

const std::int32_t messageActionPaidMessagesRefunded::ID;

object_ptr<MessageAction> messageActionPaidMessagesRefunded::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionPaidMessagesRefunded>(p);
}

messageActionPaidMessagesRefunded::messageActionPaidMessagesRefunded(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , stars_(TlFetchLong::parse(p))
{}

void messageActionPaidMessagesRefunded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPaidMessagesRefunded");
    s.store_field("count", count_);
    s.store_field("stars", stars_);
    s.store_class_end();
  }
}

messageActionPaidMessagesPrice::messageActionPaidMessagesPrice()
  : flags_()
  , broadcast_messages_allowed_()
  , stars_()
{}

const std::int32_t messageActionPaidMessagesPrice::ID;

object_ptr<MessageAction> messageActionPaidMessagesPrice::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionPaidMessagesPrice> res = make_tl_object<messageActionPaidMessagesPrice>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->broadcast_messages_allowed_ = (var0 & 1) != 0;
  res->stars_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionPaidMessagesPrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionPaidMessagesPrice");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (broadcast_messages_allowed_ << 0)));
    if (var0 & 1) { s.store_field("broadcast_messages_allowed", true); }
    s.store_field("stars", stars_);
    s.store_class_end();
  }
}

messageActionConferenceCall::messageActionConferenceCall()
  : flags_()
  , missed_()
  , active_()
  , video_()
  , call_id_()
  , duration_()
  , other_participants_()
{}

const std::int32_t messageActionConferenceCall::ID;

object_ptr<MessageAction> messageActionConferenceCall::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionConferenceCall> res = make_tl_object<messageActionConferenceCall>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->missed_ = (var0 & 1) != 0;
  res->active_ = (var0 & 2) != 0;
  res->video_ = (var0 & 16) != 0;
  res->call_id_ = TlFetchLong::parse(p);
  if (var0 & 4) { res->duration_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->other_participants_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionConferenceCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionConferenceCall");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (missed_ << 0) | (active_ << 1) | (video_ << 4)));
    if (var0 & 1) { s.store_field("missed", true); }
    if (var0 & 2) { s.store_field("active", true); }
    if (var0 & 16) { s.store_field("video", true); }
    s.store_field("call_id", call_id_);
    if (var0 & 4) { s.store_field("duration", duration_); }
    if (var0 & 8) { { s.store_vector_begin("other_participants", other_participants_.size()); for (const auto &_value : other_participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

const std::int32_t messageActionTodoCompletions::ID;

object_ptr<MessageAction> messageActionTodoCompletions::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionTodoCompletions>(p);
}

messageActionTodoCompletions::messageActionTodoCompletions(TlBufferParser &p)
  : completed_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
  , incompleted_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void messageActionTodoCompletions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionTodoCompletions");
    { s.store_vector_begin("completed", completed_.size()); for (const auto &_value : completed_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("incompleted", incompleted_.size()); for (const auto &_value : incompleted_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messageActionTodoAppendTasks::ID;

object_ptr<MessageAction> messageActionTodoAppendTasks::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionTodoAppendTasks>(p);
}

messageActionTodoAppendTasks::messageActionTodoAppendTasks(TlBufferParser &p)
  : list_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<todoItem>, -878074577>>, 481674261>::parse(p))
{}

void messageActionTodoAppendTasks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionTodoAppendTasks");
    { s.store_vector_begin("list", list_.size()); for (const auto &_value : list_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageActionSuggestedPostApproval::messageActionSuggestedPostApproval()
  : flags_()
  , rejected_()
  , balance_too_low_()
  , reject_comment_()
  , schedule_date_()
  , price_()
{}

const std::int32_t messageActionSuggestedPostApproval::ID;

object_ptr<MessageAction> messageActionSuggestedPostApproval::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionSuggestedPostApproval> res = make_tl_object<messageActionSuggestedPostApproval>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->rejected_ = (var0 & 1) != 0;
  res->balance_too_low_ = (var0 & 2) != 0;
  if (var0 & 4) { res->reject_comment_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->schedule_date_ = TlFetchInt::parse(p); }
  if (var0 & 16) { res->price_ = TlFetchObject<StarsAmount>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionSuggestedPostApproval::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSuggestedPostApproval");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (rejected_ << 0) | (balance_too_low_ << 1)));
    if (var0 & 1) { s.store_field("rejected", true); }
    if (var0 & 2) { s.store_field("balance_too_low", true); }
    if (var0 & 4) { s.store_field("reject_comment", reject_comment_); }
    if (var0 & 8) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 16) { s.store_object_field("price", static_cast<const BaseObject *>(price_.get())); }
    s.store_class_end();
  }
}

const std::int32_t messageActionSuggestedPostSuccess::ID;

object_ptr<MessageAction> messageActionSuggestedPostSuccess::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionSuggestedPostSuccess>(p);
}

messageActionSuggestedPostSuccess::messageActionSuggestedPostSuccess(TlBufferParser &p)
  : price_(TlFetchObject<StarsAmount>::parse(p))
{}

void messageActionSuggestedPostSuccess::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSuggestedPostSuccess");
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_class_end();
  }
}

messageActionSuggestedPostRefund::messageActionSuggestedPostRefund()
  : flags_()
  , payer_initiated_()
{}

const std::int32_t messageActionSuggestedPostRefund::ID;

object_ptr<MessageAction> messageActionSuggestedPostRefund::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionSuggestedPostRefund> res = make_tl_object<messageActionSuggestedPostRefund>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->payer_initiated_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionSuggestedPostRefund::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSuggestedPostRefund");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (payer_initiated_ << 0)));
    if (var0 & 1) { s.store_field("payer_initiated", true); }
    s.store_class_end();
  }
}

messageActionGiftTon::messageActionGiftTon()
  : flags_()
  , currency_()
  , amount_()
  , crypto_currency_()
  , crypto_amount_()
  , transaction_id_()
{}

const std::int32_t messageActionGiftTon::ID;

object_ptr<MessageAction> messageActionGiftTon::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageActionGiftTon> res = make_tl_object<messageActionGiftTon>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchLong::parse(p);
  res->crypto_currency_ = TlFetchString<string>::parse(p);
  res->crypto_amount_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->transaction_id_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageActionGiftTon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionGiftTon");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("crypto_currency", crypto_currency_);
    s.store_field("crypto_amount", crypto_amount_);
    if (var0 & 1) { s.store_field("transaction_id", transaction_id_); }
    s.store_class_end();
  }
}

const std::int32_t messageActionSuggestBirthday::ID;

object_ptr<MessageAction> messageActionSuggestBirthday::fetch(TlBufferParser &p) {
  return make_tl_object<messageActionSuggestBirthday>(p);
}

messageActionSuggestBirthday::messageActionSuggestBirthday(TlBufferParser &p)
  : birthday_(TlFetchBoxed<TlFetchObject<birthday>, 1821253126>::parse(p))
{}

void messageActionSuggestBirthday::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageActionSuggestBirthday");
    s.store_object_field("birthday", static_cast<const BaseObject *>(birthday_.get()));
    s.store_class_end();
  }
}

messageFwdHeader::messageFwdHeader()
  : flags_()
  , imported_()
  , saved_out_()
  , from_id_()
  , from_name_()
  , date_()
  , channel_post_()
  , post_author_()
  , saved_from_peer_()
  , saved_from_msg_id_()
  , saved_from_id_()
  , saved_from_name_()
  , saved_date_()
  , psa_type_()
{}

const std::int32_t messageFwdHeader::ID;

object_ptr<messageFwdHeader> messageFwdHeader::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageFwdHeader> res = make_tl_object<messageFwdHeader>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->imported_ = (var0 & 128) != 0;
  res->saved_out_ = (var0 & 2048) != 0;
  if (var0 & 1) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 32) { res->from_name_ = TlFetchString<string>::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->channel_post_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->post_author_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->saved_from_peer_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 16) { res->saved_from_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 256) { res->saved_from_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 512) { res->saved_from_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 1024) { res->saved_date_ = TlFetchInt::parse(p); }
  if (var0 & 64) { res->psa_type_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messageFwdHeader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageFwdHeader");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (imported_ << 7) | (saved_out_ << 11)));
    if (var0 & 128) { s.store_field("imported", true); }
    if (var0 & 2048) { s.store_field("saved_out", true); }
    if (var0 & 1) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    if (var0 & 32) { s.store_field("from_name", from_name_); }
    s.store_field("date", date_);
    if (var0 & 4) { s.store_field("channel_post", channel_post_); }
    if (var0 & 8) { s.store_field("post_author", post_author_); }
    if (var0 & 16) { s.store_object_field("saved_from_peer", static_cast<const BaseObject *>(saved_from_peer_.get())); }
    if (var0 & 16) { s.store_field("saved_from_msg_id", saved_from_msg_id_); }
    if (var0 & 256) { s.store_object_field("saved_from_id", static_cast<const BaseObject *>(saved_from_id_.get())); }
    if (var0 & 512) { s.store_field("saved_from_name", saved_from_name_); }
    if (var0 & 1024) { s.store_field("saved_date", saved_date_); }
    if (var0 & 64) { s.store_field("psa_type", psa_type_); }
    s.store_class_end();
  }
}

object_ptr<MessageMedia> MessageMedia::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messageMediaEmpty::ID;

object_ptr<MessageMedia> messageMediaEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaEmpty>();
}

void messageMediaEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaEmpty");
    s.store_class_end();
  }
}

messageMediaPhoto::messageMediaPhoto()
  : flags_()
  , spoiler_()
  , photo_()
  , ttl_seconds_()
{}

const std::int32_t messageMediaPhoto::ID;

object_ptr<MessageMedia> messageMediaPhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaPhoto> res = make_tl_object<messageMediaPhoto>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->spoiler_ = (var0 & 8) != 0;
  if (var0 & 1) { res->photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 4) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaPhoto");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (spoiler_ << 3)));
    if (var0 & 8) { s.store_field("spoiler", true); }
    if (var0 & 1) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 4) { s.store_field("ttl_seconds", ttl_seconds_); }
    s.store_class_end();
  }
}

const std::int32_t messageMediaGeo::ID;

object_ptr<MessageMedia> messageMediaGeo::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaGeo>(p);
}

messageMediaGeo::messageMediaGeo(TlBufferParser &p)
  : geo_(TlFetchObject<GeoPoint>::parse(p))
{}

void messageMediaGeo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaGeo");
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    s.store_class_end();
  }
}

const std::int32_t messageMediaContact::ID;

object_ptr<MessageMedia> messageMediaContact::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaContact>(p);
}

messageMediaContact::messageMediaContact(TlBufferParser &p)
  : phone_number_(TlFetchString<string>::parse(p))
  , first_name_(TlFetchString<string>::parse(p))
  , last_name_(TlFetchString<string>::parse(p))
  , vcard_(TlFetchString<string>::parse(p))
  , user_id_(TlFetchLong::parse(p))
{}

void messageMediaContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaContact");
    s.store_field("phone_number", phone_number_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("vcard", vcard_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

const std::int32_t messageMediaUnsupported::ID;

object_ptr<MessageMedia> messageMediaUnsupported::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaUnsupported>();
}

void messageMediaUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaUnsupported");
    s.store_class_end();
  }
}

messageMediaDocument::messageMediaDocument()
  : flags_()
  , nopremium_()
  , spoiler_()
  , video_()
  , round_()
  , voice_()
  , document_()
  , alt_documents_()
  , video_cover_()
  , video_timestamp_()
  , ttl_seconds_()
{}

const std::int32_t messageMediaDocument::ID;

object_ptr<MessageMedia> messageMediaDocument::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaDocument> res = make_tl_object<messageMediaDocument>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->nopremium_ = (var0 & 8) != 0;
  res->spoiler_ = (var0 & 16) != 0;
  res->video_ = (var0 & 64) != 0;
  res->round_ = (var0 & 128) != 0;
  res->voice_ = (var0 & 256) != 0;
  if (var0 & 1) { res->document_ = TlFetchObject<Document>::parse(p); }
  if (var0 & 32) { res->alt_documents_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p); }
  if (var0 & 512) { res->video_cover_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 1024) { res->video_timestamp_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaDocument");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (nopremium_ << 3) | (spoiler_ << 4) | (video_ << 6) | (round_ << 7) | (voice_ << 8)));
    if (var0 & 8) { s.store_field("nopremium", true); }
    if (var0 & 16) { s.store_field("spoiler", true); }
    if (var0 & 64) { s.store_field("video", true); }
    if (var0 & 128) { s.store_field("round", true); }
    if (var0 & 256) { s.store_field("voice", true); }
    if (var0 & 1) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 32) { { s.store_vector_begin("alt_documents", alt_documents_.size()); for (const auto &_value : alt_documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 512) { s.store_object_field("video_cover", static_cast<const BaseObject *>(video_cover_.get())); }
    if (var0 & 1024) { s.store_field("video_timestamp", video_timestamp_); }
    if (var0 & 4) { s.store_field("ttl_seconds", ttl_seconds_); }
    s.store_class_end();
  }
}

messageMediaWebPage::messageMediaWebPage()
  : flags_()
  , force_large_media_()
  , force_small_media_()
  , manual_()
  , safe_()
  , webpage_()
{}

const std::int32_t messageMediaWebPage::ID;

object_ptr<MessageMedia> messageMediaWebPage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaWebPage> res = make_tl_object<messageMediaWebPage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->force_large_media_ = (var0 & 1) != 0;
  res->force_small_media_ = (var0 & 2) != 0;
  res->manual_ = (var0 & 8) != 0;
  res->safe_ = (var0 & 16) != 0;
  res->webpage_ = TlFetchObject<WebPage>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaWebPage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (force_large_media_ << 0) | (force_small_media_ << 1) | (manual_ << 3) | (safe_ << 4)));
    if (var0 & 1) { s.store_field("force_large_media", true); }
    if (var0 & 2) { s.store_field("force_small_media", true); }
    if (var0 & 8) { s.store_field("manual", true); }
    if (var0 & 16) { s.store_field("safe", true); }
    s.store_object_field("webpage", static_cast<const BaseObject *>(webpage_.get()));
    s.store_class_end();
  }
}

const std::int32_t messageMediaVenue::ID;

object_ptr<MessageMedia> messageMediaVenue::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaVenue>(p);
}

messageMediaVenue::messageMediaVenue(TlBufferParser &p)
  : geo_(TlFetchObject<GeoPoint>::parse(p))
  , title_(TlFetchString<string>::parse(p))
  , address_(TlFetchString<string>::parse(p))
  , provider_(TlFetchString<string>::parse(p))
  , venue_id_(TlFetchString<string>::parse(p))
  , venue_type_(TlFetchString<string>::parse(p))
{}

void messageMediaVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaVenue");
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    s.store_field("title", title_);
    s.store_field("address", address_);
    s.store_field("provider", provider_);
    s.store_field("venue_id", venue_id_);
    s.store_field("venue_type", venue_type_);
    s.store_class_end();
  }
}

const std::int32_t messageMediaGame::ID;

object_ptr<MessageMedia> messageMediaGame::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaGame>(p);
}

messageMediaGame::messageMediaGame(TlBufferParser &p)
  : game_(TlFetchBoxed<TlFetchObject<game>, -1107729093>::parse(p))
{}

void messageMediaGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaGame");
    s.store_object_field("game", static_cast<const BaseObject *>(game_.get()));
    s.store_class_end();
  }
}

messageMediaInvoice::messageMediaInvoice()
  : flags_()
  , shipping_address_requested_()
  , test_()
  , title_()
  , description_()
  , photo_()
  , receipt_msg_id_()
  , currency_()
  , total_amount_()
  , start_param_()
  , extended_media_()
{}

const std::int32_t messageMediaInvoice::ID;

object_ptr<MessageMedia> messageMediaInvoice::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaInvoice> res = make_tl_object<messageMediaInvoice>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->shipping_address_requested_ = (var0 & 2) != 0;
  res->test_ = (var0 & 8) != 0;
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  if (var0 & 4) { res->receipt_msg_id_ = TlFetchInt::parse(p); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  res->start_param_ = TlFetchString<string>::parse(p);
  if (var0 & 16) { res->extended_media_ = TlFetchObject<MessageExtendedMedia>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaInvoice");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (shipping_address_requested_ << 1) | (test_ << 3)));
    if (var0 & 2) { s.store_field("shipping_address_requested", true); }
    if (var0 & 8) { s.store_field("test", true); }
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 1) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 4) { s.store_field("receipt_msg_id", receipt_msg_id_); }
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_field("start_param", start_param_);
    if (var0 & 16) { s.store_object_field("extended_media", static_cast<const BaseObject *>(extended_media_.get())); }
    s.store_class_end();
  }
}

messageMediaGeoLive::messageMediaGeoLive()
  : flags_()
  , geo_()
  , heading_()
  , period_()
  , proximity_notification_radius_()
{}

const std::int32_t messageMediaGeoLive::ID;

object_ptr<MessageMedia> messageMediaGeoLive::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaGeoLive> res = make_tl_object<messageMediaGeoLive>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->geo_ = TlFetchObject<GeoPoint>::parse(p);
  if (var0 & 1) { res->heading_ = TlFetchInt::parse(p); }
  res->period_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->proximity_notification_radius_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaGeoLive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaGeoLive");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("geo", static_cast<const BaseObject *>(geo_.get()));
    if (var0 & 1) { s.store_field("heading", heading_); }
    s.store_field("period", period_);
    if (var0 & 2) { s.store_field("proximity_notification_radius", proximity_notification_radius_); }
    s.store_class_end();
  }
}

const std::int32_t messageMediaPoll::ID;

object_ptr<MessageMedia> messageMediaPoll::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaPoll>(p);
}

messageMediaPoll::messageMediaPoll(TlBufferParser &p)
  : poll_(TlFetchBoxed<TlFetchObject<poll>, 1484026161>::parse(p))
  , results_(TlFetchBoxed<TlFetchObject<pollResults>, 2061444128>::parse(p))
{}

void messageMediaPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaPoll");
    s.store_object_field("poll", static_cast<const BaseObject *>(poll_.get()));
    s.store_object_field("results", static_cast<const BaseObject *>(results_.get()));
    s.store_class_end();
  }
}

const std::int32_t messageMediaDice::ID;

object_ptr<MessageMedia> messageMediaDice::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaDice>(p);
}

messageMediaDice::messageMediaDice(TlBufferParser &p)
  : value_(TlFetchInt::parse(p))
  , emoticon_(TlFetchString<string>::parse(p))
{}

void messageMediaDice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaDice");
    s.store_field("value", value_);
    s.store_field("emoticon", emoticon_);
    s.store_class_end();
  }
}

messageMediaStory::messageMediaStory()
  : flags_()
  , via_mention_()
  , peer_()
  , id_()
  , story_()
{}

const std::int32_t messageMediaStory::ID;

object_ptr<MessageMedia> messageMediaStory::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaStory> res = make_tl_object<messageMediaStory>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->via_mention_ = (var0 & 2) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->id_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->story_ = TlFetchObject<StoryItem>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaStory");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (via_mention_ << 1)));
    if (var0 & 2) { s.store_field("via_mention", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("story", static_cast<const BaseObject *>(story_.get())); }
    s.store_class_end();
  }
}

messageMediaGiveaway::messageMediaGiveaway()
  : flags_()
  , only_new_subscribers_()
  , winners_are_visible_()
  , channels_()
  , countries_iso2_()
  , prize_description_()
  , quantity_()
  , months_()
  , stars_()
  , until_date_()
{}

const std::int32_t messageMediaGiveaway::ID;

object_ptr<MessageMedia> messageMediaGiveaway::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaGiveaway> res = make_tl_object<messageMediaGiveaway>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->only_new_subscribers_ = (var0 & 1) != 0;
  res->winners_are_visible_ = (var0 & 4) != 0;
  res->channels_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p);
  if (var0 & 2) { res->countries_iso2_ = TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p); }
  if (var0 & 8) { res->prize_description_ = TlFetchString<string>::parse(p); }
  res->quantity_ = TlFetchInt::parse(p);
  if (var0 & 16) { res->months_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->stars_ = TlFetchLong::parse(p); }
  res->until_date_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaGiveaway");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (only_new_subscribers_ << 0) | (winners_are_visible_ << 2)));
    if (var0 & 1) { s.store_field("only_new_subscribers", true); }
    if (var0 & 4) { s.store_field("winners_are_visible", true); }
    { s.store_vector_begin("channels", channels_.size()); for (const auto &_value : channels_) { s.store_field("", _value); } s.store_class_end(); }
    if (var0 & 2) { { s.store_vector_begin("countries_iso2", countries_iso2_.size()); for (const auto &_value : countries_iso2_) { s.store_field("", _value); } s.store_class_end(); } }
    if (var0 & 8) { s.store_field("prize_description", prize_description_); }
    s.store_field("quantity", quantity_);
    if (var0 & 16) { s.store_field("months", months_); }
    if (var0 & 32) { s.store_field("stars", stars_); }
    s.store_field("until_date", until_date_);
    s.store_class_end();
  }
}

messageMediaGiveawayResults::messageMediaGiveawayResults()
  : flags_()
  , only_new_subscribers_()
  , refunded_()
  , channel_id_()
  , additional_peers_count_()
  , launch_msg_id_()
  , winners_count_()
  , unclaimed_count_()
  , winners_()
  , months_()
  , stars_()
  , prize_description_()
  , until_date_()
{}

const std::int32_t messageMediaGiveawayResults::ID;

object_ptr<MessageMedia> messageMediaGiveawayResults::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaGiveawayResults> res = make_tl_object<messageMediaGiveawayResults>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->only_new_subscribers_ = (var0 & 1) != 0;
  res->refunded_ = (var0 & 4) != 0;
  res->channel_id_ = TlFetchLong::parse(p);
  if (var0 & 8) { res->additional_peers_count_ = TlFetchInt::parse(p); }
  res->launch_msg_id_ = TlFetchInt::parse(p);
  res->winners_count_ = TlFetchInt::parse(p);
  res->unclaimed_count_ = TlFetchInt::parse(p);
  res->winners_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p);
  if (var0 & 16) { res->months_ = TlFetchInt::parse(p); }
  if (var0 & 32) { res->stars_ = TlFetchLong::parse(p); }
  if (var0 & 2) { res->prize_description_ = TlFetchString<string>::parse(p); }
  res->until_date_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaGiveawayResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaGiveawayResults");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (only_new_subscribers_ << 0) | (refunded_ << 2)));
    if (var0 & 1) { s.store_field("only_new_subscribers", true); }
    if (var0 & 4) { s.store_field("refunded", true); }
    s.store_field("channel_id", channel_id_);
    if (var0 & 8) { s.store_field("additional_peers_count", additional_peers_count_); }
    s.store_field("launch_msg_id", launch_msg_id_);
    s.store_field("winners_count", winners_count_);
    s.store_field("unclaimed_count", unclaimed_count_);
    { s.store_vector_begin("winners", winners_.size()); for (const auto &_value : winners_) { s.store_field("", _value); } s.store_class_end(); }
    if (var0 & 16) { s.store_field("months", months_); }
    if (var0 & 32) { s.store_field("stars", stars_); }
    if (var0 & 2) { s.store_field("prize_description", prize_description_); }
    s.store_field("until_date", until_date_);
    s.store_class_end();
  }
}

const std::int32_t messageMediaPaidMedia::ID;

object_ptr<MessageMedia> messageMediaPaidMedia::fetch(TlBufferParser &p) {
  return make_tl_object<messageMediaPaidMedia>(p);
}

messageMediaPaidMedia::messageMediaPaidMedia(TlBufferParser &p)
  : stars_amount_(TlFetchLong::parse(p))
  , extended_media_(TlFetchBoxed<TlFetchVector<TlFetchObject<MessageExtendedMedia>>, 481674261>::parse(p))
{}

void messageMediaPaidMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaPaidMedia");
    s.store_field("stars_amount", stars_amount_);
    { s.store_vector_begin("extended_media", extended_media_.size()); for (const auto &_value : extended_media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageMediaToDo::messageMediaToDo()
  : flags_()
  , todo_()
  , completions_()
{}

const std::int32_t messageMediaToDo::ID;

object_ptr<MessageMedia> messageMediaToDo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageMediaToDo> res = make_tl_object<messageMediaToDo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->todo_ = TlFetchBoxed<TlFetchObject<todoList>, 1236871718>::parse(p);
  if (var0 & 1) { res->completions_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<todoCompletion>, 1287725239>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageMediaToDo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageMediaToDo");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("todo", static_cast<const BaseObject *>(todo_.get()));
    if (var0 & 1) { { s.store_vector_begin("completions", completions_.size()); for (const auto &_value : completions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

messageViews::messageViews()
  : flags_()
  , views_()
  , forwards_()
  , replies_()
{}

const std::int32_t messageViews::ID;

object_ptr<messageViews> messageViews::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageViews> res = make_tl_object<messageViews>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->views_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->forwards_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->replies_ = TlFetchBoxed<TlFetchObject<messageReplies>, -2083123262>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messageViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageViews");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("views", views_); }
    if (var0 & 2) { s.store_field("forwards", forwards_); }
    if (var0 & 4) { s.store_object_field("replies", static_cast<const BaseObject *>(replies_.get())); }
    s.store_class_end();
  }
}

myBoost::myBoost()
  : flags_()
  , slot_()
  , peer_()
  , date_()
  , expires_()
  , cooldown_until_date_()
{}

const std::int32_t myBoost::ID;

object_ptr<myBoost> myBoost::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<myBoost> res = make_tl_object<myBoost>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->slot_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->peer_ = TlFetchObject<Peer>::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  res->expires_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->cooldown_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void myBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "myBoost");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("slot", slot_);
    if (var0 & 1) { s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get())); }
    s.store_field("date", date_);
    s.store_field("expires", expires_);
    if (var0 & 2) { s.store_field("cooldown_until_date", cooldown_until_date_); }
    s.store_class_end();
  }
}

object_ptr<NotifyPeer> NotifyPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t notifyPeer::ID;

object_ptr<NotifyPeer> notifyPeer::fetch(TlBufferParser &p) {
  return make_tl_object<notifyPeer>(p);
}

notifyPeer::notifyPeer(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
{}

void notifyPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notifyPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

const std::int32_t notifyUsers::ID;

object_ptr<NotifyPeer> notifyUsers::fetch(TlBufferParser &p) {
  return make_tl_object<notifyUsers>();
}

void notifyUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notifyUsers");
    s.store_class_end();
  }
}

const std::int32_t notifyChats::ID;

object_ptr<NotifyPeer> notifyChats::fetch(TlBufferParser &p) {
  return make_tl_object<notifyChats>();
}

void notifyChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notifyChats");
    s.store_class_end();
  }
}

const std::int32_t notifyBroadcasts::ID;

object_ptr<NotifyPeer> notifyBroadcasts::fetch(TlBufferParser &p) {
  return make_tl_object<notifyBroadcasts>();
}

void notifyBroadcasts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notifyBroadcasts");
    s.store_class_end();
  }
}

const std::int32_t notifyForumTopic::ID;

object_ptr<NotifyPeer> notifyForumTopic::fetch(TlBufferParser &p) {
  return make_tl_object<notifyForumTopic>(p);
}

notifyForumTopic::notifyForumTopic(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , top_msg_id_(TlFetchInt::parse(p))
{}

void notifyForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notifyForumTopic");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("top_msg_id", top_msg_id_);
    s.store_class_end();
  }
}

const std::int32_t outboxReadDate::ID;

object_ptr<outboxReadDate> outboxReadDate::fetch(TlBufferParser &p) {
  return make_tl_object<outboxReadDate>(p);
}

outboxReadDate::outboxReadDate(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
{}

void outboxReadDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "outboxReadDate");
    s.store_field("date", date_);
    s.store_class_end();
  }
}

object_ptr<PeerColor> PeerColor::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case peerColor::ID:
      return peerColor::fetch(p);
    case peerColorCollectible::ID:
      return peerColorCollectible::fetch(p);
    case inputPeerColorCollectible::ID:
      return inputPeerColorCollectible::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

peerColor::peerColor()
  : flags_()
  , color_()
  , background_emoji_id_()
{}

peerColor::peerColor(int32 flags_, int32 color_, int64 background_emoji_id_)
  : flags_(flags_)
  , color_(color_)
  , background_emoji_id_(background_emoji_id_)
{}

const std::int32_t peerColor::ID;

object_ptr<PeerColor> peerColor::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<peerColor> res = make_tl_object<peerColor>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->color_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->background_emoji_id_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void peerColor::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBinary::store(color_, s); }
  if (var0 & 2) { TlStoreBinary::store(background_emoji_id_, s); }
}

void peerColor::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBinary::store(color_, s); }
  if (var0 & 2) { TlStoreBinary::store(background_emoji_id_, s); }
}

void peerColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerColor");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("color", color_); }
    if (var0 & 2) { s.store_field("background_emoji_id", background_emoji_id_); }
    s.store_class_end();
  }
}

peerColorCollectible::peerColorCollectible()
  : flags_()
  , collectible_id_()
  , gift_emoji_id_()
  , background_emoji_id_()
  , accent_color_()
  , colors_()
  , dark_accent_color_()
  , dark_colors_()
{}

peerColorCollectible::peerColorCollectible(int32 flags_, int64 collectible_id_, int64 gift_emoji_id_, int64 background_emoji_id_, int32 accent_color_, array<int32> &&colors_, int32 dark_accent_color_, array<int32> &&dark_colors_)
  : flags_(flags_)
  , collectible_id_(collectible_id_)
  , gift_emoji_id_(gift_emoji_id_)
  , background_emoji_id_(background_emoji_id_)
  , accent_color_(accent_color_)
  , colors_(std::move(colors_))
  , dark_accent_color_(dark_accent_color_)
  , dark_colors_(std::move(dark_colors_))
{}

const std::int32_t peerColorCollectible::ID;

object_ptr<PeerColor> peerColorCollectible::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<peerColorCollectible> res = make_tl_object<peerColorCollectible>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->collectible_id_ = TlFetchLong::parse(p);
  res->gift_emoji_id_ = TlFetchLong::parse(p);
  res->background_emoji_id_ = TlFetchLong::parse(p);
  res->accent_color_ = TlFetchInt::parse(p);
  res->colors_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
  if (var0 & 1) { res->dark_accent_color_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->dark_colors_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void peerColorCollectible::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(collectible_id_, s);
  TlStoreBinary::store(gift_emoji_id_, s);
  TlStoreBinary::store(background_emoji_id_, s);
  TlStoreBinary::store(accent_color_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(colors_, s);
  if (var0 & 1) { TlStoreBinary::store(dark_accent_color_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(dark_colors_, s); }
}

void peerColorCollectible::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(collectible_id_, s);
  TlStoreBinary::store(gift_emoji_id_, s);
  TlStoreBinary::store(background_emoji_id_, s);
  TlStoreBinary::store(accent_color_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(colors_, s);
  if (var0 & 1) { TlStoreBinary::store(dark_accent_color_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(dark_colors_, s); }
}

void peerColorCollectible::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerColorCollectible");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("collectible_id", collectible_id_);
    s.store_field("gift_emoji_id", gift_emoji_id_);
    s.store_field("background_emoji_id", background_emoji_id_);
    s.store_field("accent_color", accent_color_);
    { s.store_vector_begin("colors", colors_.size()); for (const auto &_value : colors_) { s.store_field("", _value); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("dark_accent_color", dark_accent_color_); }
    if (var0 & 2) { { s.store_vector_begin("dark_colors", dark_colors_.size()); for (const auto &_value : dark_colors_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

inputPeerColorCollectible::inputPeerColorCollectible(int64 collectible_id_)
  : collectible_id_(collectible_id_)
{}

const std::int32_t inputPeerColorCollectible::ID;

object_ptr<PeerColor> inputPeerColorCollectible::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerColorCollectible>(p);
}

inputPeerColorCollectible::inputPeerColorCollectible(TlBufferParser &p)
  : collectible_id_(TlFetchLong::parse(p))
{}

void inputPeerColorCollectible::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(collectible_id_, s);
}

void inputPeerColorCollectible::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(collectible_id_, s);
}

void inputPeerColorCollectible::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerColorCollectible");
    s.store_field("collectible_id", collectible_id_);
    s.store_class_end();
  }
}

object_ptr<PhotoSize> PhotoSize::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

photoSizeEmpty::photoSizeEmpty(string const &type_)
  : type_(type_)
{}

const std::int32_t photoSizeEmpty::ID;

object_ptr<PhotoSize> photoSizeEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<photoSizeEmpty>(p);
}

photoSizeEmpty::photoSizeEmpty(TlBufferParser &p)
  : type_(TlFetchString<string>::parse(p))
{}

void photoSizeEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoSizeEmpty");
    s.store_field("type", type_);
    s.store_class_end();
  }
}

photoSize::photoSize(string const &type_, int32 w_, int32 h_, int32 size_)
  : type_(type_)
  , w_(w_)
  , h_(h_)
  , size_(size_)
{}

const std::int32_t photoSize::ID;

object_ptr<PhotoSize> photoSize::fetch(TlBufferParser &p) {
  return make_tl_object<photoSize>(p);
}

photoSize::photoSize(TlBufferParser &p)
  : type_(TlFetchString<string>::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
{}

void photoSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoSize");
    s.store_field("type", type_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    s.store_class_end();
  }
}

photoCachedSize::photoCachedSize(string const &type_, int32 w_, int32 h_, bytes &&bytes_)
  : type_(type_)
  , w_(w_)
  , h_(h_)
  , bytes_(std::move(bytes_))
{}

const std::int32_t photoCachedSize::ID;

object_ptr<PhotoSize> photoCachedSize::fetch(TlBufferParser &p) {
  return make_tl_object<photoCachedSize>(p);
}

photoCachedSize::photoCachedSize(TlBufferParser &p)
  : type_(TlFetchString<string>::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
{}

void photoCachedSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoCachedSize");
    s.store_field("type", type_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

const std::int32_t photoStrippedSize::ID;

object_ptr<PhotoSize> photoStrippedSize::fetch(TlBufferParser &p) {
  return make_tl_object<photoStrippedSize>(p);
}

photoStrippedSize::photoStrippedSize(TlBufferParser &p)
  : type_(TlFetchString<string>::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
{}

void photoStrippedSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoStrippedSize");
    s.store_field("type", type_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

const std::int32_t photoSizeProgressive::ID;

object_ptr<PhotoSize> photoSizeProgressive::fetch(TlBufferParser &p) {
  return make_tl_object<photoSizeProgressive>(p);
}

photoSizeProgressive::photoSizeProgressive(TlBufferParser &p)
  : type_(TlFetchString<string>::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , sizes_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void photoSizeProgressive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoSizeProgressive");
    s.store_field("type", type_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    { s.store_vector_begin("sizes", sizes_.size()); for (const auto &_value : sizes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t photoPathSize::ID;

object_ptr<PhotoSize> photoPathSize::fetch(TlBufferParser &p) {
  return make_tl_object<photoPathSize>(p);
}

photoPathSize::photoPathSize(TlBufferParser &p)
  : type_(TlFetchString<string>::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
{}

void photoPathSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoPathSize");
    s.store_field("type", type_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

object_ptr<PostInteractionCounters> PostInteractionCounters::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case postInteractionCountersMessage::ID:
      return postInteractionCountersMessage::fetch(p);
    case postInteractionCountersStory::ID:
      return postInteractionCountersStory::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t postInteractionCountersMessage::ID;

object_ptr<PostInteractionCounters> postInteractionCountersMessage::fetch(TlBufferParser &p) {
  return make_tl_object<postInteractionCountersMessage>(p);
}

postInteractionCountersMessage::postInteractionCountersMessage(TlBufferParser &p)
  : msg_id_(TlFetchInt::parse(p))
  , views_(TlFetchInt::parse(p))
  , forwards_(TlFetchInt::parse(p))
  , reactions_(TlFetchInt::parse(p))
{}

void postInteractionCountersMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "postInteractionCountersMessage");
    s.store_field("msg_id", msg_id_);
    s.store_field("views", views_);
    s.store_field("forwards", forwards_);
    s.store_field("reactions", reactions_);
    s.store_class_end();
  }
}

const std::int32_t postInteractionCountersStory::ID;

object_ptr<PostInteractionCounters> postInteractionCountersStory::fetch(TlBufferParser &p) {
  return make_tl_object<postInteractionCountersStory>(p);
}

postInteractionCountersStory::postInteractionCountersStory(TlBufferParser &p)
  : story_id_(TlFetchInt::parse(p))
  , views_(TlFetchInt::parse(p))
  , forwards_(TlFetchInt::parse(p))
  , reactions_(TlFetchInt::parse(p))
{}

void postInteractionCountersStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "postInteractionCountersStory");
    s.store_field("story_id", story_id_);
    s.store_field("views", views_);
    s.store_field("forwards", forwards_);
    s.store_field("reactions", reactions_);
    s.store_class_end();
  }
}

premiumSubscriptionOption::premiumSubscriptionOption()
  : flags_()
  , current_()
  , can_purchase_upgrade_()
  , transaction_()
  , months_()
  , currency_()
  , amount_()
  , bot_url_()
  , store_product_()
{}

const std::int32_t premiumSubscriptionOption::ID;

object_ptr<premiumSubscriptionOption> premiumSubscriptionOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<premiumSubscriptionOption> res = make_tl_object<premiumSubscriptionOption>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->current_ = (var0 & 2) != 0;
  res->can_purchase_upgrade_ = (var0 & 4) != 0;
  if (var0 & 8) { res->transaction_ = TlFetchString<string>::parse(p); }
  res->months_ = TlFetchInt::parse(p);
  res->currency_ = TlFetchString<string>::parse(p);
  res->amount_ = TlFetchLong::parse(p);
  res->bot_url_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->store_product_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void premiumSubscriptionOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumSubscriptionOption");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (current_ << 1) | (can_purchase_upgrade_ << 2)));
    if (var0 & 2) { s.store_field("current", true); }
    if (var0 & 4) { s.store_field("can_purchase_upgrade", true); }
    if (var0 & 8) { s.store_field("transaction", transaction_); }
    s.store_field("months", months_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("bot_url", bot_url_);
    if (var0 & 1) { s.store_field("store_product", store_product_); }
    s.store_class_end();
  }
}

object_ptr<PrivacyKey> PrivacyKey::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t privacyKeyStatusTimestamp::ID;

object_ptr<PrivacyKey> privacyKeyStatusTimestamp::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyStatusTimestamp>();
}

void privacyKeyStatusTimestamp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyStatusTimestamp");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyChatInvite::ID;

object_ptr<PrivacyKey> privacyKeyChatInvite::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyChatInvite>();
}

void privacyKeyChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyChatInvite");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyPhoneCall::ID;

object_ptr<PrivacyKey> privacyKeyPhoneCall::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyPhoneCall>();
}

void privacyKeyPhoneCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyPhoneCall");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyPhoneP2P::ID;

object_ptr<PrivacyKey> privacyKeyPhoneP2P::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyPhoneP2P>();
}

void privacyKeyPhoneP2P::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyPhoneP2P");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyForwards::ID;

object_ptr<PrivacyKey> privacyKeyForwards::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyForwards>();
}

void privacyKeyForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyForwards");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyProfilePhoto::ID;

object_ptr<PrivacyKey> privacyKeyProfilePhoto::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyProfilePhoto>();
}

void privacyKeyProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyProfilePhoto");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyPhoneNumber::ID;

object_ptr<PrivacyKey> privacyKeyPhoneNumber::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyPhoneNumber>();
}

void privacyKeyPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyPhoneNumber");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyAddedByPhone::ID;

object_ptr<PrivacyKey> privacyKeyAddedByPhone::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyAddedByPhone>();
}

void privacyKeyAddedByPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyAddedByPhone");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyVoiceMessages::ID;

object_ptr<PrivacyKey> privacyKeyVoiceMessages::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyVoiceMessages>();
}

void privacyKeyVoiceMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyVoiceMessages");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyAbout::ID;

object_ptr<PrivacyKey> privacyKeyAbout::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyAbout>();
}

void privacyKeyAbout::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyAbout");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyBirthday::ID;

object_ptr<PrivacyKey> privacyKeyBirthday::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyBirthday>();
}

void privacyKeyBirthday::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyBirthday");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyStarGiftsAutoSave::ID;

object_ptr<PrivacyKey> privacyKeyStarGiftsAutoSave::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyStarGiftsAutoSave>();
}

void privacyKeyStarGiftsAutoSave::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyStarGiftsAutoSave");
    s.store_class_end();
  }
}

const std::int32_t privacyKeyNoPaidMessages::ID;

object_ptr<PrivacyKey> privacyKeyNoPaidMessages::fetch(TlBufferParser &p) {
  return make_tl_object<privacyKeyNoPaidMessages>();
}

void privacyKeyNoPaidMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyKeyNoPaidMessages");
    s.store_class_end();
  }
}

const std::int32_t readParticipantDate::ID;

object_ptr<readParticipantDate> readParticipantDate::fetch(TlBufferParser &p) {
  return make_tl_object<readParticipantDate>(p);
}

readParticipantDate::readParticipantDate(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void readParticipantDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readParticipantDate");
    s.store_field("user_id", user_id_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t receivedNotifyMessage::ID;

object_ptr<receivedNotifyMessage> receivedNotifyMessage::fetch(TlBufferParser &p) {
  return make_tl_object<receivedNotifyMessage>(p);
}

receivedNotifyMessage::receivedNotifyMessage(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
  , flags_(TlFetchInt::parse(p))
{}

void receivedNotifyMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "receivedNotifyMessage");
    s.store_field("id", id_);
    s.store_field("flags", flags_);
    s.store_class_end();
  }
}

object_ptr<RequestPeerType> RequestPeerType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case requestPeerTypeUser::ID:
      return requestPeerTypeUser::fetch(p);
    case requestPeerTypeChat::ID:
      return requestPeerTypeChat::fetch(p);
    case requestPeerTypeBroadcast::ID:
      return requestPeerTypeBroadcast::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

requestPeerTypeUser::requestPeerTypeUser()
  : flags_()
  , bot_()
  , premium_()
{}

requestPeerTypeUser::requestPeerTypeUser(int32 flags_, bool bot_, bool premium_)
  : flags_(flags_)
  , bot_(bot_)
  , premium_(premium_)
{}

const std::int32_t requestPeerTypeUser::ID;

object_ptr<RequestPeerType> requestPeerTypeUser::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<requestPeerTypeUser> res = make_tl_object<requestPeerTypeUser>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->bot_ = TlFetchBool::parse(p); }
  if (var0 & 2) { res->premium_ = TlFetchBool::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void requestPeerTypeUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBool::store(bot_, s); }
  if (var0 & 2) { TlStoreBool::store(premium_, s); }
}

void requestPeerTypeUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBool::store(bot_, s); }
  if (var0 & 2) { TlStoreBool::store(premium_, s); }
}

void requestPeerTypeUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestPeerTypeUser");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("bot", bot_); }
    if (var0 & 2) { s.store_field("premium", premium_); }
    s.store_class_end();
  }
}

requestPeerTypeChat::requestPeerTypeChat()
  : flags_()
  , creator_()
  , bot_participant_()
  , has_username_()
  , forum_()
  , user_admin_rights_()
  , bot_admin_rights_()
{}

requestPeerTypeChat::requestPeerTypeChat(int32 flags_, bool creator_, bool bot_participant_, bool has_username_, bool forum_, object_ptr<chatAdminRights> &&user_admin_rights_, object_ptr<chatAdminRights> &&bot_admin_rights_)
  : flags_(flags_)
  , creator_(creator_)
  , bot_participant_(bot_participant_)
  , has_username_(has_username_)
  , forum_(forum_)
  , user_admin_rights_(std::move(user_admin_rights_))
  , bot_admin_rights_(std::move(bot_admin_rights_))
{}

const std::int32_t requestPeerTypeChat::ID;

object_ptr<RequestPeerType> requestPeerTypeChat::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<requestPeerTypeChat> res = make_tl_object<requestPeerTypeChat>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->creator_ = (var0 & 1) != 0;
  res->bot_participant_ = (var0 & 32) != 0;
  if (var0 & 8) { res->has_username_ = TlFetchBool::parse(p); }
  if (var0 & 16) { res->forum_ = TlFetchBool::parse(p); }
  if (var0 & 2) { res->user_admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (var0 & 4) { res->bot_admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void requestPeerTypeChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (creator_ << 0) | (bot_participant_ << 5)), s);
  if (var0 & 8) { TlStoreBool::store(has_username_, s); }
  if (var0 & 16) { TlStoreBool::store(forum_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1605510357>::store(user_admin_rights_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 1605510357>::store(bot_admin_rights_, s); }
}

void requestPeerTypeChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (creator_ << 0) | (bot_participant_ << 5)), s);
  if (var0 & 8) { TlStoreBool::store(has_username_, s); }
  if (var0 & 16) { TlStoreBool::store(forum_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1605510357>::store(user_admin_rights_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 1605510357>::store(bot_admin_rights_, s); }
}

void requestPeerTypeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestPeerTypeChat");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (creator_ << 0) | (bot_participant_ << 5)));
    if (var0 & 1) { s.store_field("creator", true); }
    if (var0 & 32) { s.store_field("bot_participant", true); }
    if (var0 & 8) { s.store_field("has_username", has_username_); }
    if (var0 & 16) { s.store_field("forum", forum_); }
    if (var0 & 2) { s.store_object_field("user_admin_rights", static_cast<const BaseObject *>(user_admin_rights_.get())); }
    if (var0 & 4) { s.store_object_field("bot_admin_rights", static_cast<const BaseObject *>(bot_admin_rights_.get())); }
    s.store_class_end();
  }
}

requestPeerTypeBroadcast::requestPeerTypeBroadcast()
  : flags_()
  , creator_()
  , has_username_()
  , user_admin_rights_()
  , bot_admin_rights_()
{}

requestPeerTypeBroadcast::requestPeerTypeBroadcast(int32 flags_, bool creator_, bool has_username_, object_ptr<chatAdminRights> &&user_admin_rights_, object_ptr<chatAdminRights> &&bot_admin_rights_)
  : flags_(flags_)
  , creator_(creator_)
  , has_username_(has_username_)
  , user_admin_rights_(std::move(user_admin_rights_))
  , bot_admin_rights_(std::move(bot_admin_rights_))
{}

const std::int32_t requestPeerTypeBroadcast::ID;

object_ptr<RequestPeerType> requestPeerTypeBroadcast::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<requestPeerTypeBroadcast> res = make_tl_object<requestPeerTypeBroadcast>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->creator_ = (var0 & 1) != 0;
  if (var0 & 8) { res->has_username_ = TlFetchBool::parse(p); }
  if (var0 & 2) { res->user_admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (var0 & 4) { res->bot_admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void requestPeerTypeBroadcast::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (creator_ << 0)), s);
  if (var0 & 8) { TlStoreBool::store(has_username_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1605510357>::store(user_admin_rights_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 1605510357>::store(bot_admin_rights_, s); }
}

void requestPeerTypeBroadcast::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (creator_ << 0)), s);
  if (var0 & 8) { TlStoreBool::store(has_username_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreObject, 1605510357>::store(user_admin_rights_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 1605510357>::store(bot_admin_rights_, s); }
}

void requestPeerTypeBroadcast::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestPeerTypeBroadcast");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (creator_ << 0)));
    if (var0 & 1) { s.store_field("creator", true); }
    if (var0 & 8) { s.store_field("has_username", has_username_); }
    if (var0 & 2) { s.store_object_field("user_admin_rights", static_cast<const BaseObject *>(user_admin_rights_.get())); }
    if (var0 & 4) { s.store_object_field("bot_admin_rights", static_cast<const BaseObject *>(bot_admin_rights_.get())); }
    s.store_class_end();
  }
}

const std::int32_t sponsoredMessageReportOption::ID;

object_ptr<sponsoredMessageReportOption> sponsoredMessageReportOption::fetch(TlBufferParser &p) {
  return make_tl_object<sponsoredMessageReportOption>(p);
}

sponsoredMessageReportOption::sponsoredMessageReportOption(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , option_(TlFetchBytes<bytes>::parse(p))
{}

void sponsoredMessageReportOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sponsoredMessageReportOption");
    s.store_field("text", text_);
    s.store_bytes_field("option", option_);
    s.store_class_end();
  }
}

sponsoredPeer::sponsoredPeer()
  : flags_()
  , random_id_()
  , peer_()
  , sponsor_info_()
  , additional_info_()
{}

const std::int32_t sponsoredPeer::ID;

object_ptr<sponsoredPeer> sponsoredPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<sponsoredPeer> res = make_tl_object<sponsoredPeer>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->random_id_ = TlFetchBytes<bytes>::parse(p);
  res->peer_ = TlFetchObject<Peer>::parse(p);
  if (var0 & 1) { res->sponsor_info_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->additional_info_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void sponsoredPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sponsoredPeer");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_bytes_field("random_id", random_id_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("sponsor_info", sponsor_info_); }
    if (var0 & 2) { s.store_field("additional_info", additional_info_); }
    s.store_class_end();
  }
}

object_ptr<StarGiftAttributeId> StarGiftAttributeId::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case starGiftAttributeIdModel::ID:
      return starGiftAttributeIdModel::fetch(p);
    case starGiftAttributeIdPattern::ID:
      return starGiftAttributeIdPattern::fetch(p);
    case starGiftAttributeIdBackdrop::ID:
      return starGiftAttributeIdBackdrop::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

starGiftAttributeIdModel::starGiftAttributeIdModel(int64 document_id_)
  : document_id_(document_id_)
{}

const std::int32_t starGiftAttributeIdModel::ID;

object_ptr<StarGiftAttributeId> starGiftAttributeIdModel::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftAttributeIdModel>(p);
}

starGiftAttributeIdModel::starGiftAttributeIdModel(TlBufferParser &p)
  : document_id_(TlFetchLong::parse(p))
{}

void starGiftAttributeIdModel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(document_id_, s);
}

void starGiftAttributeIdModel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(document_id_, s);
}

void starGiftAttributeIdModel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributeIdModel");
    s.store_field("document_id", document_id_);
    s.store_class_end();
  }
}

starGiftAttributeIdPattern::starGiftAttributeIdPattern(int64 document_id_)
  : document_id_(document_id_)
{}

const std::int32_t starGiftAttributeIdPattern::ID;

object_ptr<StarGiftAttributeId> starGiftAttributeIdPattern::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftAttributeIdPattern>(p);
}

starGiftAttributeIdPattern::starGiftAttributeIdPattern(TlBufferParser &p)
  : document_id_(TlFetchLong::parse(p))
{}

void starGiftAttributeIdPattern::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(document_id_, s);
}

void starGiftAttributeIdPattern::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(document_id_, s);
}

void starGiftAttributeIdPattern::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributeIdPattern");
    s.store_field("document_id", document_id_);
    s.store_class_end();
  }
}

starGiftAttributeIdBackdrop::starGiftAttributeIdBackdrop(int32 backdrop_id_)
  : backdrop_id_(backdrop_id_)
{}

const std::int32_t starGiftAttributeIdBackdrop::ID;

object_ptr<StarGiftAttributeId> starGiftAttributeIdBackdrop::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftAttributeIdBackdrop>(p);
}

starGiftAttributeIdBackdrop::starGiftAttributeIdBackdrop(TlBufferParser &p)
  : backdrop_id_(TlFetchInt::parse(p))
{}

void starGiftAttributeIdBackdrop::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(backdrop_id_, s);
}

void starGiftAttributeIdBackdrop::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(backdrop_id_, s);
}

void starGiftAttributeIdBackdrop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributeIdBackdrop");
    s.store_field("backdrop_id", backdrop_id_);
    s.store_class_end();
  }
}

const std::int32_t starGiftUpgradePrice::ID;

object_ptr<starGiftUpgradePrice> starGiftUpgradePrice::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftUpgradePrice>(p);
}

starGiftUpgradePrice::starGiftUpgradePrice(TlBufferParser &p)
  : date_(TlFetchInt::parse(p))
  , upgrade_stars_(TlFetchLong::parse(p))
{}

void starGiftUpgradePrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftUpgradePrice");
    s.store_field("date", date_);
    s.store_field("upgrade_stars", upgrade_stars_);
    s.store_class_end();
  }
}

starRefProgram::starRefProgram()
  : flags_()
  , bot_id_()
  , commission_permille_()
  , duration_months_()
  , end_date_()
  , daily_revenue_per_user_()
{}

const std::int32_t starRefProgram::ID;

object_ptr<starRefProgram> starRefProgram::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starRefProgram> res = make_tl_object<starRefProgram>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->bot_id_ = TlFetchLong::parse(p);
  res->commission_permille_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->duration_months_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->end_date_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->daily_revenue_per_user_ = TlFetchObject<StarsAmount>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starRefProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starRefProgram");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("bot_id", bot_id_);
    s.store_field("commission_permille", commission_permille_);
    if (var0 & 1) { s.store_field("duration_months", duration_months_); }
    if (var0 & 2) { s.store_field("end_date", end_date_); }
    if (var0 & 4) { s.store_object_field("daily_revenue_per_user", static_cast<const BaseObject *>(daily_revenue_per_user_.get())); }
    s.store_class_end();
  }
}

object_ptr<StarsAmount> StarsAmount::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case starsAmount::ID:
      return starsAmount::fetch(p);
    case starsTonAmount::ID:
      return starsTonAmount::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

starsAmount::starsAmount(int64 amount_, int32 nanos_)
  : amount_(amount_)
  , nanos_(nanos_)
{}

const std::int32_t starsAmount::ID;

object_ptr<StarsAmount> starsAmount::fetch(TlBufferParser &p) {
  return make_tl_object<starsAmount>(p);
}

starsAmount::starsAmount(TlBufferParser &p)
  : amount_(TlFetchLong::parse(p))
  , nanos_(TlFetchInt::parse(p))
{}

void starsAmount::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(amount_, s);
  TlStoreBinary::store(nanos_, s);
}

void starsAmount::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(amount_, s);
  TlStoreBinary::store(nanos_, s);
}

void starsAmount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsAmount");
    s.store_field("amount", amount_);
    s.store_field("nanos", nanos_);
    s.store_class_end();
  }
}

starsTonAmount::starsTonAmount(int64 amount_)
  : amount_(amount_)
{}

const std::int32_t starsTonAmount::ID;

object_ptr<StarsAmount> starsTonAmount::fetch(TlBufferParser &p) {
  return make_tl_object<starsTonAmount>(p);
}

starsTonAmount::starsTonAmount(TlBufferParser &p)
  : amount_(TlFetchLong::parse(p))
{}

void starsTonAmount::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(amount_, s);
}

void starsTonAmount::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(amount_, s);
}

void starsTonAmount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTonAmount");
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

const std::int32_t statsDateRangeDays::ID;

object_ptr<statsDateRangeDays> statsDateRangeDays::fetch(TlBufferParser &p) {
  return make_tl_object<statsDateRangeDays>(p);
}

statsDateRangeDays::statsDateRangeDays(TlBufferParser &p)
  : min_date_(TlFetchInt::parse(p))
  , max_date_(TlFetchInt::parse(p))
{}

void statsDateRangeDays::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsDateRangeDays");
    s.store_field("min_date", min_date_);
    s.store_field("max_date", max_date_);
    s.store_class_end();
  }
}

object_ptr<StickerSetCovered> StickerSetCovered::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case stickerSetCovered::ID:
      return stickerSetCovered::fetch(p);
    case stickerSetMultiCovered::ID:
      return stickerSetMultiCovered::fetch(p);
    case stickerSetFullCovered::ID:
      return stickerSetFullCovered::fetch(p);
    case stickerSetNoCovered::ID:
      return stickerSetNoCovered::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t stickerSetCovered::ID;

object_ptr<StickerSetCovered> stickerSetCovered::fetch(TlBufferParser &p) {
  return make_tl_object<stickerSetCovered>(p);
}

stickerSetCovered::stickerSetCovered(TlBufferParser &p)
  : set_(TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>::parse(p))
  , cover_(TlFetchObject<Document>::parse(p))
{}

void stickerSetCovered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSetCovered");
    s.store_object_field("set", static_cast<const BaseObject *>(set_.get()));
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_class_end();
  }
}

const std::int32_t stickerSetMultiCovered::ID;

object_ptr<StickerSetCovered> stickerSetMultiCovered::fetch(TlBufferParser &p) {
  return make_tl_object<stickerSetMultiCovered>(p);
}

stickerSetMultiCovered::stickerSetMultiCovered(TlBufferParser &p)
  : set_(TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>::parse(p))
  , covers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void stickerSetMultiCovered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSetMultiCovered");
    s.store_object_field("set", static_cast<const BaseObject *>(set_.get()));
    { s.store_vector_begin("covers", covers_.size()); for (const auto &_value : covers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t stickerSetFullCovered::ID;

object_ptr<StickerSetCovered> stickerSetFullCovered::fetch(TlBufferParser &p) {
  return make_tl_object<stickerSetFullCovered>(p);
}

stickerSetFullCovered::stickerSetFullCovered(TlBufferParser &p)
  : set_(TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>::parse(p))
  , packs_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<stickerPack>, 313694676>>, 481674261>::parse(p))
  , keywords_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<stickerKeyword>, -50416996>>, 481674261>::parse(p))
  , documents_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void stickerSetFullCovered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSetFullCovered");
    s.store_object_field("set", static_cast<const BaseObject *>(set_.get()));
    { s.store_vector_begin("packs", packs_.size()); for (const auto &_value : packs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("keywords", keywords_.size()); for (const auto &_value : keywords_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t stickerSetNoCovered::ID;

object_ptr<StickerSetCovered> stickerSetNoCovered::fetch(TlBufferParser &p) {
  return make_tl_object<stickerSetNoCovered>(p);
}

stickerSetNoCovered::stickerSetNoCovered(TlBufferParser &p)
  : set_(TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>::parse(p))
{}

void stickerSetNoCovered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSetNoCovered");
    s.store_object_field("set", static_cast<const BaseObject *>(set_.get()));
    s.store_class_end();
  }
}

storiesStealthMode::storiesStealthMode()
  : flags_()
  , active_until_date_()
  , cooldown_until_date_()
{}

const std::int32_t storiesStealthMode::ID;

object_ptr<storiesStealthMode> storiesStealthMode::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storiesStealthMode> res = make_tl_object<storiesStealthMode>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->active_until_date_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->cooldown_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void storiesStealthMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storiesStealthMode");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("active_until_date", active_until_date_); }
    if (var0 & 2) { s.store_field("cooldown_until_date", cooldown_until_date_); }
    s.store_class_end();
  }
}

storyViews::storyViews()
  : flags_()
  , has_viewers_()
  , views_count_()
  , forwards_count_()
  , reactions_()
  , reactions_count_()
  , recent_viewers_()
{}

const std::int32_t storyViews::ID;

object_ptr<storyViews> storyViews::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyViews> res = make_tl_object<storyViews>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_viewers_ = (var0 & 2) != 0;
  res->views_count_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->forwards_count_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->reactions_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<reactionCount>, -1546531968>>, 481674261>::parse(p); }
  if (var0 & 16) { res->reactions_count_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->recent_viewers_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void storyViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyViews");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_viewers_ << 1)));
    if (var0 & 2) { s.store_field("has_viewers", true); }
    s.store_field("views_count", views_count_);
    if (var0 & 4) { s.store_field("forwards_count", forwards_count_); }
    if (var0 & 8) { { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 16) { s.store_field("reactions_count", reactions_count_); }
    if (var0 & 1) { { s.store_vector_begin("recent_viewers", recent_viewers_.size()); for (const auto &_value : recent_viewers_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

suggestedPost::suggestedPost()
  : flags_()
  , accepted_()
  , rejected_()
  , price_()
  , schedule_date_()
{}

suggestedPost::suggestedPost(int32 flags_, bool accepted_, bool rejected_, object_ptr<StarsAmount> &&price_, int32 schedule_date_)
  : flags_(flags_)
  , accepted_(accepted_)
  , rejected_(rejected_)
  , price_(std::move(price_))
  , schedule_date_(schedule_date_)
{}

const std::int32_t suggestedPost::ID;

object_ptr<suggestedPost> suggestedPost::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<suggestedPost> res = make_tl_object<suggestedPost>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->accepted_ = (var0 & 2) != 0;
  res->rejected_ = (var0 & 4) != 0;
  if (var0 & 8) { res->price_ = TlFetchObject<StarsAmount>::parse(p); }
  if (var0 & 1) { res->schedule_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void suggestedPost::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (accepted_ << 1) | (rejected_ << 2)), s);
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(price_, s); }
  if (var0 & 1) { TlStoreBinary::store(schedule_date_, s); }
}

void suggestedPost::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (accepted_ << 1) | (rejected_ << 2)), s);
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(price_, s); }
  if (var0 & 1) { TlStoreBinary::store(schedule_date_, s); }
}

void suggestedPost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPost");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (accepted_ << 1) | (rejected_ << 2)));
    if (var0 & 2) { s.store_field("accepted", true); }
    if (var0 & 4) { s.store_field("rejected", true); }
    if (var0 & 8) { s.store_object_field("price", static_cast<const BaseObject *>(price_.get())); }
    if (var0 & 1) { s.store_field("schedule_date", schedule_date_); }
    s.store_class_end();
  }
}

object_ptr<account_SavedRingtone> account_SavedRingtone::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_savedRingtone::ID:
      return account_savedRingtone::fetch(p);
    case account_savedRingtoneConverted::ID:
      return account_savedRingtoneConverted::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_savedRingtone::ID;

object_ptr<account_SavedRingtone> account_savedRingtone::fetch(TlBufferParser &p) {
  return make_tl_object<account_savedRingtone>();
}

void account_savedRingtone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.savedRingtone");
    s.store_class_end();
  }
}

const std::int32_t account_savedRingtoneConverted::ID;

object_ptr<account_SavedRingtone> account_savedRingtoneConverted::fetch(TlBufferParser &p) {
  return make_tl_object<account_savedRingtoneConverted>(p);
}

account_savedRingtoneConverted::account_savedRingtoneConverted(TlBufferParser &p)
  : document_(TlFetchObject<Document>::parse(p))
{}

void account_savedRingtoneConverted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.savedRingtoneConverted");
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_class_end();
  }
}

auth_loggedOut::auth_loggedOut()
  : flags_()
  , future_auth_token_()
{}

const std::int32_t auth_loggedOut::ID;

object_ptr<auth_loggedOut> auth_loggedOut::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_loggedOut> res = make_tl_object<auth_loggedOut>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->future_auth_token_ = TlFetchBytes<bytes>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void auth_loggedOut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.loggedOut");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_bytes_field("future_auth_token", future_auth_token_); }
    s.store_class_end();
  }
}

const std::int32_t channels_channelParticipant::ID;

object_ptr<channels_channelParticipant> channels_channelParticipant::fetch(TlBufferParser &p) {
  return make_tl_object<channels_channelParticipant>(p);
}

channels_channelParticipant::channels_channelParticipant(TlBufferParser &p)
  : participant_(TlFetchObject<ChannelParticipant>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void channels_channelParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.channelParticipant");
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t chatlists_exportedInvites::ID;

object_ptr<chatlists_exportedInvites> chatlists_exportedInvites::fetch(TlBufferParser &p) {
  return make_tl_object<chatlists_exportedInvites>(p);
}

chatlists_exportedInvites::chatlists_exportedInvites(TlBufferParser &p)
  : invites_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<exportedChatlistInvite>, 206668204>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void chatlists_exportedInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.exportedInvites");
    { s.store_vector_begin("invites", invites_.size()); for (const auto &_value : invites_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<help_UserInfo> help_UserInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_userInfoEmpty::ID:
      return help_userInfoEmpty::fetch(p);
    case help_userInfo::ID:
      return help_userInfo::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_userInfoEmpty::ID;

object_ptr<help_UserInfo> help_userInfoEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<help_userInfoEmpty>();
}

void help_userInfoEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.userInfoEmpty");
    s.store_class_end();
  }
}

const std::int32_t help_userInfo::ID;

object_ptr<help_UserInfo> help_userInfo::fetch(TlBufferParser &p) {
  return make_tl_object<help_userInfo>(p);
}

help_userInfo::help_userInfo(TlBufferParser &p)
  : message_(TlFetchString<string>::parse(p))
  , entities_(TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p))
  , author_(TlFetchString<string>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void help_userInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.userInfo");
    s.store_field("message", message_);
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("author", author_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

object_ptr<messages_FeaturedStickers> messages_FeaturedStickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_featuredStickersNotModified::ID:
      return messages_featuredStickersNotModified::fetch(p);
    case messages_featuredStickers::ID:
      return messages_featuredStickers::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_featuredStickersNotModified::ID;

object_ptr<messages_FeaturedStickers> messages_featuredStickersNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_featuredStickersNotModified>(p);
}

messages_featuredStickersNotModified::messages_featuredStickersNotModified(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
{}

void messages_featuredStickersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.featuredStickersNotModified");
    s.store_field("count", count_);
    s.store_class_end();
  }
}

messages_featuredStickers::messages_featuredStickers()
  : flags_()
  , premium_()
  , hash_()
  , count_()
  , sets_()
  , unread_()
{}

const std::int32_t messages_featuredStickers::ID;

object_ptr<messages_FeaturedStickers> messages_featuredStickers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_featuredStickers> res = make_tl_object<messages_featuredStickers>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->premium_ = (var0 & 1) != 0;
  res->hash_ = TlFetchLong::parse(p);
  res->count_ = TlFetchInt::parse(p);
  res->sets_ = TlFetchBoxed<TlFetchVector<TlFetchObject<StickerSetCovered>>, 481674261>::parse(p);
  res->unread_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messages_featuredStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.featuredStickers");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (premium_ << 0)));
    if (var0 & 1) { s.store_field("premium", true); }
    s.store_field("hash", hash_);
    s.store_field("count", count_);
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("unread", unread_.size()); for (const auto &_value : unread_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_FoundStickerSets> messages_FoundStickerSets::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_foundStickerSetsNotModified::ID:
      return messages_foundStickerSetsNotModified::fetch(p);
    case messages_foundStickerSets::ID:
      return messages_foundStickerSets::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_foundStickerSetsNotModified::ID;

object_ptr<messages_FoundStickerSets> messages_foundStickerSetsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_foundStickerSetsNotModified>();
}

void messages_foundStickerSetsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.foundStickerSetsNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_foundStickerSets::ID;

object_ptr<messages_FoundStickerSets> messages_foundStickerSets::fetch(TlBufferParser &p) {
  return make_tl_object<messages_foundStickerSets>(p);
}

messages_foundStickerSets::messages_foundStickerSets(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , sets_(TlFetchBoxed<TlFetchVector<TlFetchObject<StickerSetCovered>>, 481674261>::parse(p))
{}

void messages_foundStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.foundStickerSets");
    s.store_field("hash", hash_);
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_historyImportParsed::messages_historyImportParsed()
  : flags_()
  , pm_()
  , group_()
  , title_()
{}

const std::int32_t messages_historyImportParsed::ID;

object_ptr<messages_historyImportParsed> messages_historyImportParsed::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_historyImportParsed> res = make_tl_object<messages_historyImportParsed>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pm_ = (var0 & 1) != 0;
  res->group_ = (var0 & 2) != 0;
  if (var0 & 4) { res->title_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_historyImportParsed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.historyImportParsed");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pm_ << 0) | (group_ << 1)));
    if (var0 & 1) { s.store_field("pm", true); }
    if (var0 & 2) { s.store_field("group", true); }
    if (var0 & 4) { s.store_field("title", title_); }
    s.store_class_end();
  }
}

const std::int32_t messages_inactiveChats::ID;

object_ptr<messages_inactiveChats> messages_inactiveChats::fetch(TlBufferParser &p) {
  return make_tl_object<messages_inactiveChats>(p);
}

messages_inactiveChats::messages_inactiveChats(TlBufferParser &p)
  : dates_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_inactiveChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.inactiveChats");
    { s.store_vector_begin("dates", dates_.size()); for (const auto &_value : dates_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_Messages> messages_Messages::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_messages::ID:
      return messages_messages::fetch(p);
    case messages_messagesSlice::ID:
      return messages_messagesSlice::fetch(p);
    case messages_channelMessages::ID:
      return messages_channelMessages::fetch(p);
    case messages_messagesNotModified::ID:
      return messages_messagesNotModified::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_messages::ID;

object_ptr<messages_Messages> messages_messages::fetch(TlBufferParser &p) {
  return make_tl_object<messages_messages>(p);
}

messages_messages::messages_messages(TlBufferParser &p)
  : messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , topics_(TlFetchBoxed<TlFetchVector<TlFetchObject<ForumTopic>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_messages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.messages");
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("topics", topics_.size()); for (const auto &_value : topics_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_messagesSlice::messages_messagesSlice()
  : flags_()
  , inexact_()
  , count_()
  , next_rate_()
  , offset_id_offset_()
  , search_flood_()
  , messages_()
  , topics_()
  , chats_()
  , users_()
{}

const std::int32_t messages_messagesSlice::ID;

object_ptr<messages_Messages> messages_messagesSlice::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_messagesSlice> res = make_tl_object<messages_messagesSlice>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->inexact_ = (var0 & 2) != 0;
  res->count_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->next_rate_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->offset_id_offset_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->search_flood_ = TlFetchBoxed<TlFetchObject<searchPostsFlood>, 1040931690>::parse(p); }
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p);
  res->topics_ = TlFetchBoxed<TlFetchVector<TlFetchObject<ForumTopic>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messages_messagesSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.messagesSlice");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (inexact_ << 1)));
    if (var0 & 2) { s.store_field("inexact", true); }
    s.store_field("count", count_);
    if (var0 & 1) { s.store_field("next_rate", next_rate_); }
    if (var0 & 4) { s.store_field("offset_id_offset", offset_id_offset_); }
    if (var0 & 8) { s.store_object_field("search_flood", static_cast<const BaseObject *>(search_flood_.get())); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("topics", topics_.size()); for (const auto &_value : topics_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_channelMessages::messages_channelMessages()
  : flags_()
  , inexact_()
  , pts_()
  , count_()
  , offset_id_offset_()
  , messages_()
  , topics_()
  , chats_()
  , users_()
{}

const std::int32_t messages_channelMessages::ID;

object_ptr<messages_Messages> messages_channelMessages::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_channelMessages> res = make_tl_object<messages_channelMessages>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->inexact_ = (var0 & 2) != 0;
  res->pts_ = TlFetchInt::parse(p);
  res->count_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->offset_id_offset_ = TlFetchInt::parse(p); }
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p);
  res->topics_ = TlFetchBoxed<TlFetchVector<TlFetchObject<ForumTopic>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messages_channelMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.channelMessages");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (inexact_ << 1)));
    if (var0 & 2) { s.store_field("inexact", true); }
    s.store_field("pts", pts_);
    s.store_field("count", count_);
    if (var0 & 4) { s.store_field("offset_id_offset", offset_id_offset_); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("topics", topics_.size()); for (const auto &_value : topics_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_messagesNotModified::ID;

object_ptr<messages_Messages> messages_messagesNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_messagesNotModified>(p);
}

messages_messagesNotModified::messages_messagesNotModified(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
{}

void messages_messagesNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.messagesNotModified");
    s.store_field("count", count_);
    s.store_class_end();
  }
}

const std::int32_t messages_myStickers::ID;

object_ptr<messages_myStickers> messages_myStickers::fetch(TlBufferParser &p) {
  return make_tl_object<messages_myStickers>(p);
}

messages_myStickers::messages_myStickers(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , sets_(TlFetchBoxed<TlFetchVector<TlFetchObject<StickerSetCovered>>, 481674261>::parse(p))
{}

void messages_myStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.myStickers");
    s.store_field("count", count_);
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_Reactions> messages_Reactions::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_reactionsNotModified::ID:
      return messages_reactionsNotModified::fetch(p);
    case messages_reactions::ID:
      return messages_reactions::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_reactionsNotModified::ID;

object_ptr<messages_Reactions> messages_reactionsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_reactionsNotModified>();
}

void messages_reactionsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reactionsNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_reactions::ID;

object_ptr<messages_Reactions> messages_reactions::fetch(TlBufferParser &p) {
  return make_tl_object<messages_reactions>(p);
}

messages_reactions::messages_reactions(TlBufferParser &p)
  : hash_(TlFetchLong::parse(p))
  , reactions_(TlFetchBoxed<TlFetchVector<TlFetchObject<Reaction>>, 481674261>::parse(p))
{}

void messages_reactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reactions");
    s.store_field("hash", hash_);
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_searchResultsCalendar::messages_searchResultsCalendar()
  : flags_()
  , inexact_()
  , count_()
  , min_date_()
  , min_msg_id_()
  , offset_id_offset_()
  , periods_()
  , messages_()
  , chats_()
  , users_()
{}

const std::int32_t messages_searchResultsCalendar::ID;

object_ptr<messages_searchResultsCalendar> messages_searchResultsCalendar::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_searchResultsCalendar> res = make_tl_object<messages_searchResultsCalendar>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->inexact_ = (var0 & 1) != 0;
  res->count_ = TlFetchInt::parse(p);
  res->min_date_ = TlFetchInt::parse(p);
  res->min_msg_id_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->offset_id_offset_ = TlFetchInt::parse(p); }
  res->periods_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<searchResultsCalendarPeriod>, -911191137>>, 481674261>::parse(p);
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_searchResultsCalendar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchResultsCalendar");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (inexact_ << 0)));
    if (var0 & 1) { s.store_field("inexact", true); }
    s.store_field("count", count_);
    s.store_field("min_date", min_date_);
    s.store_field("min_msg_id", min_msg_id_);
    if (var0 & 2) { s.store_field("offset_id_offset", offset_id_offset_); }
    { s.store_vector_begin("periods", periods_.size()); for (const auto &_value : periods_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<payments_PaymentResult> payments_PaymentResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case payments_paymentResult::ID:
      return payments_paymentResult::fetch(p);
    case payments_paymentVerificationNeeded::ID:
      return payments_paymentVerificationNeeded::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t payments_paymentResult::ID;

object_ptr<payments_PaymentResult> payments_paymentResult::fetch(TlBufferParser &p) {
  return make_tl_object<payments_paymentResult>(p);
}

payments_paymentResult::payments_paymentResult(TlBufferParser &p)
  : updates_(TlFetchObject<Updates>::parse(p))
{}

void payments_paymentResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.paymentResult");
    s.store_object_field("updates", static_cast<const BaseObject *>(updates_.get()));
    s.store_class_end();
  }
}

const std::int32_t payments_paymentVerificationNeeded::ID;

object_ptr<payments_PaymentResult> payments_paymentVerificationNeeded::fetch(TlBufferParser &p) {
  return make_tl_object<payments_paymentVerificationNeeded>(p);
}

payments_paymentVerificationNeeded::payments_paymentVerificationNeeded(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void payments_paymentVerificationNeeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.paymentVerificationNeeded");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

const std::int32_t phone_phoneCall::ID;

object_ptr<phone_phoneCall> phone_phoneCall::fetch(TlBufferParser &p) {
  return make_tl_object<phone_phoneCall>(p);
}

phone_phoneCall::phone_phoneCall(TlBufferParser &p)
  : phone_call_(TlFetchObject<PhoneCall>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void phone_phoneCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.phoneCall");
    s.store_object_field("phone_call", static_cast<const BaseObject *>(phone_call_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t stories_storyViews::ID;

object_ptr<stories_storyViews> stories_storyViews::fetch(TlBufferParser &p) {
  return make_tl_object<stories_storyViews>(p);
}

stories_storyViews::stories_storyViews(TlBufferParser &p)
  : views_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<storyViews>, -1923523370>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void stories_storyViews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.storyViews");
    { s.store_vector_begin("views", views_.size()); for (const auto &_value : views_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<users_SavedMusic> users_SavedMusic::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case users_savedMusicNotModified::ID:
      return users_savedMusicNotModified::fetch(p);
    case users_savedMusic::ID:
      return users_savedMusic::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t users_savedMusicNotModified::ID;

object_ptr<users_SavedMusic> users_savedMusicNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<users_savedMusicNotModified>(p);
}

users_savedMusicNotModified::users_savedMusicNotModified(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
{}

void users_savedMusicNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.savedMusicNotModified");
    s.store_field("count", count_);
    s.store_class_end();
  }
}

const std::int32_t users_savedMusic::ID;

object_ptr<users_SavedMusic> users_savedMusic::fetch(TlBufferParser &p) {
  return make_tl_object<users_savedMusic>(p);
}

users_savedMusic::users_savedMusic(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , documents_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void users_savedMusic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.savedMusic");
    s.store_field("count", count_);
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_acceptAuthorization::account_acceptAuthorization(int64 bot_id_, string const &scope_, string const &public_key_, array<object_ptr<secureValueHash>> &&value_hashes_, object_ptr<secureCredentialsEncrypted> &&credentials_)
  : bot_id_(bot_id_)
  , scope_(scope_)
  , public_key_(public_key_)
  , value_hashes_(std::move(value_hashes_))
  , credentials_(std::move(credentials_))
{}

const std::int32_t account_acceptAuthorization::ID;

void account_acceptAuthorization::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-202552205);
  TlStoreBinary::store(bot_id_, s);
  TlStoreString::store(scope_, s);
  TlStoreString::store(public_key_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -316748368>>, 481674261>::store(value_hashes_, s);
  TlStoreBoxed<TlStoreObject, 871426631>::store(credentials_, s);
}

void account_acceptAuthorization::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-202552205);
  TlStoreBinary::store(bot_id_, s);
  TlStoreString::store(scope_, s);
  TlStoreString::store(public_key_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -316748368>>, 481674261>::store(value_hashes_, s);
  TlStoreBoxed<TlStoreObject, 871426631>::store(credentials_, s);
}

void account_acceptAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.acceptAuthorization");
    s.store_field("bot_id", bot_id_);
    s.store_field("scope", scope_);
    s.store_field("public_key", public_key_);
    { s.store_vector_begin("value_hashes", value_hashes_.size()); for (const auto &_value : value_hashes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("credentials", static_cast<const BaseObject *>(credentials_.get()));
    s.store_class_end();
  }
}

account_acceptAuthorization::ReturnType account_acceptAuthorization::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_confirmPasswordEmail::account_confirmPasswordEmail(string const &code_)
  : code_(code_)
{}

const std::int32_t account_confirmPasswordEmail::ID;

void account_confirmPasswordEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1881204448);
  TlStoreString::store(code_, s);
}

void account_confirmPasswordEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1881204448);
  TlStoreString::store(code_, s);
}

void account_confirmPasswordEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.confirmPasswordEmail");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

account_confirmPasswordEmail::ReturnType account_confirmPasswordEmail::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_createTheme::account_createTheme(int32 flags_, string const &slug_, string const &title_, object_ptr<InputDocument> &&document_, array<object_ptr<inputThemeSettings>> &&settings_)
  : flags_(flags_)
  , slug_(slug_)
  , title_(title_)
  , document_(std::move(document_))
  , settings_(std::move(settings_))
{}

const std::int32_t account_createTheme::ID;

void account_createTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1697530880);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(slug_, s);
  TlStoreString::store(title_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(document_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1881255857>>, 481674261>::store(settings_, s); }
}

void account_createTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1697530880);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(slug_, s);
  TlStoreString::store(title_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(document_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, -1881255857>>, 481674261>::store(settings_, s); }
}

void account_createTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.createTheme");
    s.store_field("flags", (var0 = flags_));
    s.store_field("slug", slug_);
    s.store_field("title", title_);
    if (var0 & 4) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 8) { { s.store_vector_begin("settings", settings_.size()); for (const auto &_value : settings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

account_createTheme::ReturnType account_createTheme::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<theme>, -1609668650>::parse(p);
#undef FAIL
}

const std::int32_t account_getAutoSaveSettings::ID;

void account_getAutoSaveSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1379156774);
}

void account_getAutoSaveSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1379156774);
}

void account_getAutoSaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getAutoSaveSettings");
    s.store_class_end();
  }
}

account_getAutoSaveSettings::ReturnType account_getAutoSaveSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_autoSaveSettings>, 1279133341>::parse(p);
#undef FAIL
}

const std::int32_t account_getContactSignUpNotification::ID;

void account_getContactSignUpNotification::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1626880216);
}

void account_getContactSignUpNotification::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1626880216);
}

void account_getContactSignUpNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getContactSignUpNotification");
    s.store_class_end();
  }
}

account_getContactSignUpNotification::ReturnType account_getContactSignUpNotification::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_getWallPapers::account_getWallPapers(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getWallPapers::ID;

void account_getWallPapers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(127302966);
  TlStoreBinary::store(hash_, s);
}

void account_getWallPapers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(127302966);
  TlStoreBinary::store(hash_, s);
}

void account_getWallPapers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getWallPapers");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getWallPapers::ReturnType account_getWallPapers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_WallPapers>::parse(p);
#undef FAIL
}

account_initTakeoutSession::account_initTakeoutSession(int32 flags_, bool contacts_, bool message_users_, bool message_chats_, bool message_megagroups_, bool message_channels_, bool files_, int64 file_max_size_)
  : flags_(flags_)
  , contacts_(contacts_)
  , message_users_(message_users_)
  , message_chats_(message_chats_)
  , message_megagroups_(message_megagroups_)
  , message_channels_(message_channels_)
  , files_(files_)
  , file_max_size_(file_max_size_)
{}

const std::int32_t account_initTakeoutSession::ID;

void account_initTakeoutSession::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1896617296);
  TlStoreBinary::store((var0 = flags_ | (contacts_ << 0) | (message_users_ << 1) | (message_chats_ << 2) | (message_megagroups_ << 3) | (message_channels_ << 4) | (files_ << 5)), s);
  if (var0 & 32) { TlStoreBinary::store(file_max_size_, s); }
}

void account_initTakeoutSession::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1896617296);
  TlStoreBinary::store((var0 = flags_ | (contacts_ << 0) | (message_users_ << 1) | (message_chats_ << 2) | (message_megagroups_ << 3) | (message_channels_ << 4) | (files_ << 5)), s);
  if (var0 & 32) { TlStoreBinary::store(file_max_size_, s); }
}

void account_initTakeoutSession::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.initTakeoutSession");
    s.store_field("flags", (var0 = flags_ | (contacts_ << 0) | (message_users_ << 1) | (message_chats_ << 2) | (message_megagroups_ << 3) | (message_channels_ << 4) | (files_ << 5)));
    if (var0 & 1) { s.store_field("contacts", true); }
    if (var0 & 2) { s.store_field("message_users", true); }
    if (var0 & 4) { s.store_field("message_chats", true); }
    if (var0 & 8) { s.store_field("message_megagroups", true); }
    if (var0 & 16) { s.store_field("message_channels", true); }
    if (var0 & 32) { s.store_field("files", true); }
    if (var0 & 32) { s.store_field("file_max_size", file_max_size_); }
    s.store_class_end();
  }
}

account_initTakeoutSession::ReturnType account_initTakeoutSession::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_takeout>, 1304052993>::parse(p);
#undef FAIL
}

account_installWallPaper::account_installWallPaper(object_ptr<InputWallPaper> &&wallpaper_, object_ptr<wallPaperSettings> &&settings_)
  : wallpaper_(std::move(wallpaper_))
  , settings_(std::move(settings_))
{}

const std::int32_t account_installWallPaper::ID;

void account_installWallPaper::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-18000023);
  TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s);
  TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s);
}

void account_installWallPaper::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-18000023);
  TlStoreBoxedUnknown<TlStoreObject>::store(wallpaper_, s);
  TlStoreBoxed<TlStoreObject, 925826256>::store(settings_, s);
}

void account_installWallPaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.installWallPaper");
    s.store_object_field("wallpaper", static_cast<const BaseObject *>(wallpaper_.get()));
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_installWallPaper::ReturnType account_installWallPaper::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_registerDevice::account_registerDevice(int32 flags_, bool no_muted_, int32 token_type_, string const &token_, bool app_sandbox_, bytes &&secret_, array<int64> &&other_uids_)
  : flags_(flags_)
  , no_muted_(no_muted_)
  , token_type_(token_type_)
  , token_(token_)
  , app_sandbox_(app_sandbox_)
  , secret_(std::move(secret_))
  , other_uids_(std::move(other_uids_))
{}

const std::int32_t account_registerDevice::ID;

void account_registerDevice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-326762118);
  TlStoreBinary::store((var0 = flags_ | (no_muted_ << 0)), s);
  TlStoreBinary::store(token_type_, s);
  TlStoreString::store(token_, s);
  TlStoreBool::store(app_sandbox_, s);
  TlStoreString::store(secret_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(other_uids_, s);
}

void account_registerDevice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-326762118);
  TlStoreBinary::store((var0 = flags_ | (no_muted_ << 0)), s);
  TlStoreBinary::store(token_type_, s);
  TlStoreString::store(token_, s);
  TlStoreBool::store(app_sandbox_, s);
  TlStoreString::store(secret_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(other_uids_, s);
}

void account_registerDevice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.registerDevice");
    s.store_field("flags", (var0 = flags_ | (no_muted_ << 0)));
    if (var0 & 1) { s.store_field("no_muted", true); }
    s.store_field("token_type", token_type_);
    s.store_field("token", token_);
    s.store_field("app_sandbox", app_sandbox_);
    s.store_bytes_field("secret", secret_);
    { s.store_vector_begin("other_uids", other_uids_.size()); for (const auto &_value : other_uids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_registerDevice::ReturnType account_registerDevice::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_resetNotifySettings::ID;

void account_resetNotifySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-612493497);
}

void account_resetNotifySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-612493497);
}

void account_resetNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetNotifySettings");
    s.store_class_end();
  }
}

account_resetNotifySettings::ReturnType account_resetNotifySettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_saveAutoDownloadSettings::account_saveAutoDownloadSettings(int32 flags_, bool low_, bool high_, object_ptr<autoDownloadSettings> &&settings_)
  : flags_(flags_)
  , low_(low_)
  , high_(high_)
  , settings_(std::move(settings_))
{}

const std::int32_t account_saveAutoDownloadSettings::ID;

void account_saveAutoDownloadSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1995661875);
  TlStoreBinary::store((var0 = flags_ | (low_ << 0) | (high_ << 1)), s);
  TlStoreBoxed<TlStoreObject, -1163561432>::store(settings_, s);
}

void account_saveAutoDownloadSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1995661875);
  TlStoreBinary::store((var0 = flags_ | (low_ << 0) | (high_ << 1)), s);
  TlStoreBoxed<TlStoreObject, -1163561432>::store(settings_, s);
}

void account_saveAutoDownloadSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.saveAutoDownloadSettings");
    s.store_field("flags", (var0 = flags_ | (low_ << 0) | (high_ << 1)));
    if (var0 & 1) { s.store_field("low", true); }
    if (var0 & 2) { s.store_field("high", true); }
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_saveAutoDownloadSettings::ReturnType account_saveAutoDownloadSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_saveRingtone::account_saveRingtone(object_ptr<InputDocument> &&id_, bool unsave_)
  : id_(std::move(id_))
  , unsave_(unsave_)
{}

const std::int32_t account_saveRingtone::ID;

void account_saveRingtone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1038768899);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unsave_, s);
}

void account_saveRingtone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1038768899);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unsave_, s);
}

void account_saveRingtone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.saveRingtone");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_field("unsave", unsave_);
    s.store_class_end();
  }
}

account_saveRingtone::ReturnType account_saveRingtone::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_SavedRingtone>::parse(p);
#undef FAIL
}

account_setAuthorizationTTL::account_setAuthorizationTTL(int32 authorization_ttl_days_)
  : authorization_ttl_days_(authorization_ttl_days_)
{}

const std::int32_t account_setAuthorizationTTL::ID;

void account_setAuthorizationTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1081501024);
  TlStoreBinary::store(authorization_ttl_days_, s);
}

void account_setAuthorizationTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1081501024);
  TlStoreBinary::store(authorization_ttl_days_, s);
}

void account_setAuthorizationTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setAuthorizationTTL");
    s.store_field("authorization_ttl_days", authorization_ttl_days_);
    s.store_class_end();
  }
}

account_setAuthorizationTTL::ReturnType account_setAuthorizationTTL::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_setContactSignUpNotification::account_setContactSignUpNotification(bool silent_)
  : silent_(silent_)
{}

const std::int32_t account_setContactSignUpNotification::ID;

void account_setContactSignUpNotification::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-806076575);
  TlStoreBool::store(silent_, s);
}

void account_setContactSignUpNotification::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-806076575);
  TlStoreBool::store(silent_, s);
}

void account_setContactSignUpNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setContactSignUpNotification");
    s.store_field("silent", silent_);
    s.store_class_end();
  }
}

account_setContactSignUpNotification::ReturnType account_setContactSignUpNotification::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_toggleSponsoredMessages::account_toggleSponsoredMessages(bool enabled_)
  : enabled_(enabled_)
{}

const std::int32_t account_toggleSponsoredMessages::ID;

void account_toggleSponsoredMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1176919155);
  TlStoreBool::store(enabled_, s);
}

void account_toggleSponsoredMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1176919155);
  TlStoreBool::store(enabled_, s);
}

void account_toggleSponsoredMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.toggleSponsoredMessages");
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

account_toggleSponsoredMessages::ReturnType account_toggleSponsoredMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateBusinessGreetingMessage::account_updateBusinessGreetingMessage(int32 flags_, object_ptr<inputBusinessGreetingMessage> &&message_)
  : flags_(flags_)
  , message_(std::move(message_))
{}

const std::int32_t account_updateBusinessGreetingMessage::ID;

void account_updateBusinessGreetingMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1724755908);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 26528571>::store(message_, s); }
}

void account_updateBusinessGreetingMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1724755908);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 26528571>::store(message_, s); }
}

void account_updateBusinessGreetingMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateBusinessGreetingMessage");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

account_updateBusinessGreetingMessage::ReturnType account_updateBusinessGreetingMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateBusinessWorkHours::account_updateBusinessWorkHours(int32 flags_, object_ptr<businessWorkHours> &&business_work_hours_)
  : flags_(flags_)
  , business_work_hours_(std::move(business_work_hours_))
{}

const std::int32_t account_updateBusinessWorkHours::ID;

void account_updateBusinessWorkHours::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1258348646);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1936543592>::store(business_work_hours_, s); }
}

void account_updateBusinessWorkHours::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1258348646);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1936543592>::store(business_work_hours_, s); }
}

void account_updateBusinessWorkHours::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateBusinessWorkHours");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("business_work_hours", static_cast<const BaseObject *>(business_work_hours_.get())); }
    s.store_class_end();
  }
}

account_updateBusinessWorkHours::ReturnType account_updateBusinessWorkHours::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_uploadRingtone::account_uploadRingtone(object_ptr<InputFile> &&file_, string const &file_name_, string const &mime_type_)
  : file_(std::move(file_))
  , file_name_(file_name_)
  , mime_type_(mime_type_)
{}

const std::int32_t account_uploadRingtone::ID;

void account_uploadRingtone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2095414366);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  TlStoreString::store(file_name_, s);
  TlStoreString::store(mime_type_, s);
}

void account_uploadRingtone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2095414366);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  TlStoreString::store(file_name_, s);
  TlStoreString::store(mime_type_, s);
}

void account_uploadRingtone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.uploadRingtone");
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_field("file_name", file_name_);
    s.store_field("mime_type", mime_type_);
    s.store_class_end();
  }
}

account_uploadRingtone::ReturnType account_uploadRingtone::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Document>::parse(p);
#undef FAIL
}

account_verifyEmail::account_verifyEmail(object_ptr<EmailVerifyPurpose> &&purpose_, object_ptr<EmailVerification> &&verification_)
  : purpose_(std::move(purpose_))
  , verification_(std::move(verification_))
{}

const std::int32_t account_verifyEmail::ID;

void account_verifyEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(53322959);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(verification_, s);
}

void account_verifyEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(53322959);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(verification_, s);
}

void account_verifyEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.verifyEmail");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_object_field("verification", static_cast<const BaseObject *>(verification_.get()));
    s.store_class_end();
  }
}

account_verifyEmail::ReturnType account_verifyEmail::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_EmailVerified>::parse(p);
#undef FAIL
}

bots_editPreviewMedia::bots_editPreviewMedia(object_ptr<InputUser> &&bot_, string const &lang_code_, object_ptr<InputMedia> &&media_, object_ptr<InputMedia> &&new_media_)
  : bot_(std::move(bot_))
  , lang_code_(lang_code_)
  , media_(std::move(media_))
  , new_media_(std::move(new_media_))
{}

const std::int32_t bots_editPreviewMedia::ID;

void bots_editPreviewMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2061148049);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(new_media_, s);
}

void bots_editPreviewMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2061148049);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(new_media_, s);
}

void bots_editPreviewMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.editPreviewMedia");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("lang_code", lang_code_);
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_object_field("new_media", static_cast<const BaseObject *>(new_media_.get()));
    s.store_class_end();
  }
}

bots_editPreviewMedia::ReturnType bots_editPreviewMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<botPreviewMedia>, 602479523>::parse(p);
#undef FAIL
}

bots_getPreviewInfo::bots_getPreviewInfo(object_ptr<InputUser> &&bot_, string const &lang_code_)
  : bot_(std::move(bot_))
  , lang_code_(lang_code_)
{}

const std::int32_t bots_getPreviewInfo::ID;

void bots_getPreviewInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1111143341);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
}

void bots_getPreviewInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1111143341);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
}

void bots_getPreviewInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getPreviewInfo");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

bots_getPreviewInfo::ReturnType bots_getPreviewInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<bots_previewInfo>, 212278628>::parse(p);
#undef FAIL
}

bots_reorderUsernames::bots_reorderUsernames(object_ptr<InputUser> &&bot_, array<string> &&order_)
  : bot_(std::move(bot_))
  , order_(std::move(order_))
{}

const std::int32_t bots_reorderUsernames::ID;

void bots_reorderUsernames::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1760972350);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(order_, s);
}

void bots_reorderUsernames::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1760972350);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(order_, s);
}

void bots_reorderUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.reorderUsernames");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

bots_reorderUsernames::ReturnType bots_reorderUsernames::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_resetBotCommands::bots_resetBotCommands(object_ptr<BotCommandScope> &&scope_, string const &lang_code_)
  : scope_(std::move(scope_))
  , lang_code_(lang_code_)
{}

const std::int32_t bots_resetBotCommands::ID;

void bots_resetBotCommands::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1032708345);
  TlStoreBoxedUnknown<TlStoreObject>::store(scope_, s);
  TlStoreString::store(lang_code_, s);
}

void bots_resetBotCommands::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1032708345);
  TlStoreBoxedUnknown<TlStoreObject>::store(scope_, s);
  TlStoreString::store(lang_code_, s);
}

void bots_resetBotCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.resetBotCommands");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

bots_resetBotCommands::ReturnType bots_resetBotCommands::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_toggleUserEmojiStatusPermission::bots_toggleUserEmojiStatusPermission(object_ptr<InputUser> &&bot_, bool enabled_)
  : bot_(std::move(bot_))
  , enabled_(enabled_)
{}

const std::int32_t bots_toggleUserEmojiStatusPermission::ID;

void bots_toggleUserEmojiStatusPermission::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(115237778);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBool::store(enabled_, s);
}

void bots_toggleUserEmojiStatusPermission::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(115237778);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreBool::store(enabled_, s);
}

void bots_toggleUserEmojiStatusPermission::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.toggleUserEmojiStatusPermission");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

bots_toggleUserEmojiStatusPermission::ReturnType bots_toggleUserEmojiStatusPermission::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_updateUserEmojiStatus::bots_updateUserEmojiStatus(object_ptr<InputUser> &&user_id_, object_ptr<EmojiStatus> &&emoji_status_)
  : user_id_(std::move(user_id_))
  , emoji_status_(std::move(emoji_status_))
{}

const std::int32_t bots_updateUserEmojiStatus::ID;

void bots_updateUserEmojiStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-308334395);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(emoji_status_, s);
}

void bots_updateUserEmojiStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-308334395);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(emoji_status_, s);
}

void bots_updateUserEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.updateUserEmojiStatus");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

bots_updateUserEmojiStatus::ReturnType bots_updateUserEmojiStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_checkUsername::channels_checkUsername(object_ptr<InputChannel> &&channel_, string const &username_)
  : channel_(std::move(channel_))
  , username_(username_)
{}

const std::int32_t channels_checkUsername::ID;

void channels_checkUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(283557164);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(username_, s);
}

void channels_checkUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(283557164);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(username_, s);
}

void channels_checkUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.checkUsername");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("username", username_);
    s.store_class_end();
  }
}

channels_checkUsername::ReturnType channels_checkUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_deleteChannel::channels_deleteChannel(object_ptr<InputChannel> &&channel_)
  : channel_(std::move(channel_))
{}

const std::int32_t channels_deleteChannel::ID;

void channels_deleteChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1072619549);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_deleteChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1072619549);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_deleteChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.deleteChannel");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

channels_deleteChannel::ReturnType channels_deleteChannel::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_getSendAs::channels_getSendAs(int32 flags_, bool for_paid_reactions_, object_ptr<InputPeer> &&peer_)
  : flags_(flags_)
  , for_paid_reactions_(for_paid_reactions_)
  , peer_(std::move(peer_))
{}

const std::int32_t channels_getSendAs::ID;

void channels_getSendAs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-410672065);
  TlStoreBinary::store((var0 = flags_ | (for_paid_reactions_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void channels_getSendAs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-410672065);
  TlStoreBinary::store((var0 = flags_ | (for_paid_reactions_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void channels_getSendAs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getSendAs");
    s.store_field("flags", (var0 = flags_ | (for_paid_reactions_ << 0)));
    if (var0 & 1) { s.store_field("for_paid_reactions", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

channels_getSendAs::ReturnType channels_getSendAs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<channels_sendAsPeers>, -191450938>::parse(p);
#undef FAIL
}

channels_leaveChannel::channels_leaveChannel(object_ptr<InputChannel> &&channel_)
  : channel_(std::move(channel_))
{}

const std::int32_t channels_leaveChannel::ID;

void channels_leaveChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-130635115);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_leaveChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-130635115);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_leaveChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.leaveChannel");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

channels_leaveChannel::ReturnType channels_leaveChannel::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_setBoostsToUnblockRestrictions::channels_setBoostsToUnblockRestrictions(object_ptr<InputChannel> &&channel_, int32 boosts_)
  : channel_(std::move(channel_))
  , boosts_(boosts_)
{}

const std::int32_t channels_setBoostsToUnblockRestrictions::ID;

void channels_setBoostsToUnblockRestrictions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1388733202);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(boosts_, s);
}

void channels_setBoostsToUnblockRestrictions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1388733202);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(boosts_, s);
}

void channels_setBoostsToUnblockRestrictions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.setBoostsToUnblockRestrictions");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("boosts", boosts_);
    s.store_class_end();
  }
}

channels_setBoostsToUnblockRestrictions::ReturnType channels_setBoostsToUnblockRestrictions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_block::contacts_block(int32 flags_, bool my_stories_from_, object_ptr<InputPeer> &&id_)
  : flags_(flags_)
  , my_stories_from_(my_stories_from_)
  , id_(std::move(id_))
{}

const std::int32_t contacts_block::ID;

void contacts_block::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(774801204);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void contacts_block::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(774801204);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void contacts_block::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.block");
    s.store_field("flags", (var0 = flags_ | (my_stories_from_ << 0)));
    if (var0 & 1) { s.store_field("my_stories_from", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

contacts_block::ReturnType contacts_block::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

contacts_deleteByPhones::contacts_deleteByPhones(array<string> &&phones_)
  : phones_(std::move(phones_))
{}

const std::int32_t contacts_deleteByPhones::ID;

void contacts_deleteByPhones::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(269745566);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(phones_, s);
}

void contacts_deleteByPhones::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(269745566);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(phones_, s);
}

void contacts_deleteByPhones::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.deleteByPhones");
    { s.store_vector_begin("phones", phones_.size()); for (const auto &_value : phones_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

contacts_deleteByPhones::ReturnType contacts_deleteByPhones::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

langpack_getLanguages::langpack_getLanguages(string const &lang_pack_)
  : lang_pack_(lang_pack_)
{}

const std::int32_t langpack_getLanguages::ID;

void langpack_getLanguages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1120311183);
  TlStoreString::store(lang_pack_, s);
}

void langpack_getLanguages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1120311183);
  TlStoreString::store(lang_pack_, s);
}

void langpack_getLanguages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langpack.getLanguages");
    s.store_field("lang_pack", lang_pack_);
    s.store_class_end();
  }
}

langpack_getLanguages::ReturnType langpack_getLanguages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<langPackLanguage>, -288727837>>, 481674261>::parse(p);
#undef FAIL
}

const std::int32_t messages_clearAllDrafts::ID;

void messages_clearAllDrafts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2119757468);
}

void messages_clearAllDrafts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2119757468);
}

void messages_clearAllDrafts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.clearAllDrafts");
    s.store_class_end();
  }
}

messages_clearAllDrafts::ReturnType messages_clearAllDrafts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_deleteQuickReplyMessages::messages_deleteQuickReplyMessages(int32 shortcut_id_, array<int32> &&id_)
  : shortcut_id_(shortcut_id_)
  , id_(std::move(id_))
{}

const std::int32_t messages_deleteQuickReplyMessages::ID;

void messages_deleteQuickReplyMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-519706352);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_deleteQuickReplyMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-519706352);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_deleteQuickReplyMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteQuickReplyMessages");
    s.store_field("shortcut_id", shortcut_id_);
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_deleteQuickReplyMessages::ReturnType messages_deleteQuickReplyMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getFactCheck::messages_getFactCheck(object_ptr<InputPeer> &&peer_, array<int32> &&msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(std::move(msg_id_))
{}

const std::int32_t messages_getFactCheck::ID;

void messages_getFactCheck::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1177696786);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_id_, s);
}

void messages_getFactCheck::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1177696786);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(msg_id_, s);
}

void messages_getFactCheck::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getFactCheck");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("msg_id", msg_id_.size()); for (const auto &_value : msg_id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getFactCheck::ReturnType messages_getFactCheck::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<factCheck>, -1197736753>>, 481674261>::parse(p);
#undef FAIL
}

messages_getFeaturedStickers::messages_getFeaturedStickers(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getFeaturedStickers::ID;

void messages_getFeaturedStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1685588756);
  TlStoreBinary::store(hash_, s);
}

void messages_getFeaturedStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1685588756);
  TlStoreBinary::store(hash_, s);
}

void messages_getFeaturedStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getFeaturedStickers");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getFeaturedStickers::ReturnType messages_getFeaturedStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_FeaturedStickers>::parse(p);
#undef FAIL
}

messages_getMessages::messages_getMessages(array<object_ptr<InputMessage>> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t messages_getMessages::ID;

void messages_getMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1673946374);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void messages_getMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1673946374);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void messages_getMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMessages");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getMessages::ReturnType messages_getMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

const std::int32_t messages_getPinnedSavedDialogs::ID;

void messages_getPinnedSavedDialogs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-700607264);
}

void messages_getPinnedSavedDialogs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-700607264);
}

void messages_getPinnedSavedDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPinnedSavedDialogs");
    s.store_class_end();
  }
}

messages_getPinnedSavedDialogs::ReturnType messages_getPinnedSavedDialogs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SavedDialogs>::parse(p);
#undef FAIL
}

messages_getPollVotes::messages_getPollVotes(int32 flags_, object_ptr<InputPeer> &&peer_, int32 id_, bytes &&option_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , id_(id_)
  , option_(std::move(option_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t messages_getPollVotes::ID;

void messages_getPollVotes::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1200736242);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreString::store(option_, s); }
  if (var0 & 2) { TlStoreString::store(offset_, s); }
  TlStoreBinary::store(limit_, s);
}

void messages_getPollVotes::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1200736242);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreString::store(option_, s); }
  if (var0 & 2) { TlStoreString::store(offset_, s); }
  TlStoreBinary::store(limit_, s);
}

void messages_getPollVotes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPollVotes");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_bytes_field("option", option_); }
    if (var0 & 2) { s.store_field("offset", offset_); }
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getPollVotes::ReturnType messages_getPollVotes::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_votesList>, 1218005070>::parse(p);
#undef FAIL
}

messages_getSavedDialogsByID::messages_getSavedDialogsByID(int32 flags_, object_ptr<InputPeer> &&parent_peer_, array<object_ptr<InputPeer>> &&ids_)
  : flags_(flags_)
  , parent_peer_(std::move(parent_peer_))
  , ids_(std::move(ids_))
{}

const std::int32_t messages_getSavedDialogsByID::ID;

void messages_getSavedDialogsByID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1869585558);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(ids_, s);
}

void messages_getSavedDialogsByID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1869585558);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(ids_, s);
}

void messages_getSavedDialogsByID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSavedDialogsByID");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 2) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
    { s.store_vector_begin("ids", ids_.size()); for (const auto &_value : ids_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_getSavedDialogsByID::ReturnType messages_getSavedDialogsByID::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_SavedDialogs>::parse(p);
#undef FAIL
}

messages_getSearchResultsCalendar::messages_getSearchResultsCalendar(int32 flags_, object_ptr<InputPeer> &&peer_, object_ptr<InputPeer> &&saved_peer_id_, object_ptr<MessagesFilter> &&filter_, int32 offset_id_, int32 offset_date_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , saved_peer_id_(std::move(saved_peer_id_))
  , filter_(std::move(filter_))
  , offset_id_(offset_id_)
  , offset_date_(offset_date_)
{}

const std::int32_t messages_getSearchResultsCalendar::ID;

void messages_getSearchResultsCalendar::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1789130429);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
}

void messages_getSearchResultsCalendar::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1789130429);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(saved_peer_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
}

void messages_getSearchResultsCalendar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSearchResultsCalendar");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 4) { s.store_object_field("saved_peer_id", static_cast<const BaseObject *>(saved_peer_id_.get())); }
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("offset_id", offset_id_);
    s.store_field("offset_date", offset_date_);
    s.store_class_end();
  }
}

messages_getSearchResultsCalendar::ReturnType messages_getSearchResultsCalendar::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_searchResultsCalendar>, 343859772>::parse(p);
#undef FAIL
}

const std::int32_t messages_getSplitRanges::ID;

void messages_getSplitRanges::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(486505992);
}

void messages_getSplitRanges::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(486505992);
}

void messages_getSplitRanges::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSplitRanges");
    s.store_class_end();
  }
}

messages_getSplitRanges::ReturnType messages_getSplitRanges::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<messageRange>, 182649427>>, 481674261>::parse(p);
#undef FAIL
}

messages_hidePeerSettingsBar::messages_hidePeerSettingsBar(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t messages_hidePeerSettingsBar::ID;

void messages_hidePeerSettingsBar::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1336717624);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_hidePeerSettingsBar::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1336717624);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_hidePeerSettingsBar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.hidePeerSettingsBar");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_hidePeerSettingsBar::ReturnType messages_hidePeerSettingsBar::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_readEncryptedHistory::messages_readEncryptedHistory(object_ptr<inputEncryptedChat> &&peer_, int32 max_date_)
  : peer_(std::move(peer_))
  , max_date_(max_date_)
{}

const std::int32_t messages_readEncryptedHistory::ID;

void messages_readEncryptedHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2135648522);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(max_date_, s);
}

void messages_readEncryptedHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2135648522);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBinary::store(max_date_, s);
}

void messages_readEncryptedHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readEncryptedHistory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_date", max_date_);
    s.store_class_end();
  }
}

messages_readEncryptedHistory::ReturnType messages_readEncryptedHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_reportSponsoredMessage::messages_reportSponsoredMessage(bytes &&random_id_, bytes &&option_)
  : random_id_(std::move(random_id_))
  , option_(std::move(option_))
{}

const std::int32_t messages_reportSponsoredMessage::ID;

void messages_reportSponsoredMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(315355332);
  TlStoreString::store(random_id_, s);
  TlStoreString::store(option_, s);
}

void messages_reportSponsoredMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(315355332);
  TlStoreString::store(random_id_, s);
  TlStoreString::store(option_, s);
}

void messages_reportSponsoredMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reportSponsoredMessage");
    s.store_bytes_field("random_id", random_id_);
    s.store_bytes_field("option", option_);
    s.store_class_end();
  }
}

messages_reportSponsoredMessage::ReturnType messages_reportSponsoredMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<channels_SponsoredMessageReportResult>::parse(p);
#undef FAIL
}

messages_requestEncryption::messages_requestEncryption(object_ptr<InputUser> &&user_id_, int32 random_id_, bytes &&g_a_)
  : user_id_(std::move(user_id_))
  , random_id_(random_id_)
  , g_a_(std::move(g_a_))
{}

const std::int32_t messages_requestEncryption::ID;

void messages_requestEncryption::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-162681021);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(g_a_, s);
}

void messages_requestEncryption::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-162681021);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(g_a_, s);
}

void messages_requestEncryption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.requestEncryption");
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("random_id", random_id_);
    s.store_bytes_field("g_a", g_a_);
    s.store_class_end();
  }
}

messages_requestEncryption::ReturnType messages_requestEncryption::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EncryptedChat>::parse(p);
#undef FAIL
}

messages_requestSimpleWebView::messages_requestSimpleWebView(int32 flags_, bool from_switch_webview_, bool from_side_menu_, bool compact_, bool fullscreen_, object_ptr<InputUser> &&bot_, string const &url_, string const &start_param_, object_ptr<dataJSON> &&theme_params_, string const &platform_)
  : flags_(flags_)
  , from_switch_webview_(from_switch_webview_)
  , from_side_menu_(from_side_menu_)
  , compact_(compact_)
  , fullscreen_(fullscreen_)
  , bot_(std::move(bot_))
  , url_(url_)
  , start_param_(start_param_)
  , theme_params_(std::move(theme_params_))
  , platform_(platform_)
{}

const std::int32_t messages_requestSimpleWebView::ID;

void messages_requestSimpleWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1094336115);
  TlStoreBinary::store((var0 = flags_ | (from_switch_webview_ << 1) | (from_side_menu_ << 2) | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  if (var0 & 8) { TlStoreString::store(url_, s); }
  if (var0 & 16) { TlStoreString::store(start_param_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
}

void messages_requestSimpleWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1094336115);
  TlStoreBinary::store((var0 = flags_ | (from_switch_webview_ << 1) | (from_side_menu_ << 2) | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  if (var0 & 8) { TlStoreString::store(url_, s); }
  if (var0 & 16) { TlStoreString::store(start_param_, s); }
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
}

void messages_requestSimpleWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.requestSimpleWebView");
    s.store_field("flags", (var0 = flags_ | (from_switch_webview_ << 1) | (from_side_menu_ << 2) | (compact_ << 7) | (fullscreen_ << 8)));
    if (var0 & 2) { s.store_field("from_switch_webview", true); }
    if (var0 & 4) { s.store_field("from_side_menu", true); }
    if (var0 & 128) { s.store_field("compact", true); }
    if (var0 & 256) { s.store_field("fullscreen", true); }
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    if (var0 & 8) { s.store_field("url", url_); }
    if (var0 & 16) { s.store_field("start_param", start_param_); }
    if (var0 & 1) { s.store_object_field("theme_params", static_cast<const BaseObject *>(theme_params_.get())); }
    s.store_field("platform", platform_);
    s.store_class_end();
  }
}

messages_requestSimpleWebView::ReturnType messages_requestSimpleWebView::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<webViewResultUrl>, 1294139288>::parse(p);
#undef FAIL
}

messages_requestWebView::messages_requestWebView(int32 flags_, bool from_bot_menu_, bool silent_, bool compact_, bool fullscreen_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&bot_, string const &url_, string const &start_param_, object_ptr<dataJSON> &&theme_params_, string const &platform_, object_ptr<InputReplyTo> &&reply_to_, object_ptr<InputPeer> &&send_as_)
  : flags_(flags_)
  , from_bot_menu_(from_bot_menu_)
  , silent_(silent_)
  , compact_(compact_)
  , fullscreen_(fullscreen_)
  , peer_(std::move(peer_))
  , bot_(std::move(bot_))
  , url_(url_)
  , start_param_(start_param_)
  , theme_params_(std::move(theme_params_))
  , platform_(platform_)
  , reply_to_(std::move(reply_to_))
  , send_as_(std::move(send_as_))
{}

const std::int32_t messages_requestWebView::ID;

void messages_requestWebView::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(647873217);
  TlStoreBinary::store((var0 = flags_ | (from_bot_menu_ << 4) | (silent_ << 5) | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  if (var0 & 2) { TlStoreString::store(url_, s); }
  if (var0 & 8) { TlStoreString::store(start_param_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
}

void messages_requestWebView::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(647873217);
  TlStoreBinary::store((var0 = flags_ | (from_bot_menu_ << 4) | (silent_ << 5) | (compact_ << 7) | (fullscreen_ << 8)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  if (var0 & 2) { TlStoreString::store(url_, s); }
  if (var0 & 8) { TlStoreString::store(start_param_, s); }
  if (var0 & 4) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
  TlStoreString::store(platform_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
}

void messages_requestWebView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.requestWebView");
    s.store_field("flags", (var0 = flags_ | (from_bot_menu_ << 4) | (silent_ << 5) | (compact_ << 7) | (fullscreen_ << 8)));
    if (var0 & 16) { s.store_field("from_bot_menu", true); }
    if (var0 & 32) { s.store_field("silent", true); }
    if (var0 & 128) { s.store_field("compact", true); }
    if (var0 & 256) { s.store_field("fullscreen", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    if (var0 & 2) { s.store_field("url", url_); }
    if (var0 & 8) { s.store_field("start_param", start_param_); }
    if (var0 & 4) { s.store_object_field("theme_params", static_cast<const BaseObject *>(theme_params_.get())); }
    s.store_field("platform", platform_);
    if (var0 & 1) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    if (var0 & 8192) { s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get())); }
    s.store_class_end();
  }
}

messages_requestWebView::ReturnType messages_requestWebView::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<webViewResultUrl>, 1294139288>::parse(p);
#undef FAIL
}

payments_getGiveawayInfo::payments_getGiveawayInfo(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t payments_getGiveawayInfo::ID;

void payments_getGiveawayInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-198994907);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void payments_getGiveawayInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-198994907);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void payments_getGiveawayInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getGiveawayInfo");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

payments_getGiveawayInfo::ReturnType payments_getGiveawayInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_GiveawayInfo>::parse(p);
#undef FAIL
}

payments_getStarsStatus::payments_getStarsStatus(int32 flags_, bool ton_, object_ptr<InputPeer> &&peer_)
  : flags_(flags_)
  , ton_(ton_)
  , peer_(std::move(peer_))
{}

const std::int32_t payments_getStarsStatus::ID;

void payments_getStarsStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1319744447);
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_getStarsStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1319744447);
  TlStoreBinary::store((var0 = flags_ | (ton_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void payments_getStarsStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsStatus");
    s.store_field("flags", (var0 = flags_ | (ton_ << 0)));
    if (var0 & 1) { s.store_field("ton", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

payments_getStarsStatus::ReturnType payments_getStarsStatus::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starsStatus>, 1822222573>::parse(p);
#undef FAIL
}

payments_getStarsSubscriptions::payments_getStarsSubscriptions(int32 flags_, bool missing_balance_, object_ptr<InputPeer> &&peer_, string const &offset_)
  : flags_(flags_)
  , missing_balance_(missing_balance_)
  , peer_(std::move(peer_))
  , offset_(offset_)
{}

const std::int32_t payments_getStarsSubscriptions::ID;

void payments_getStarsSubscriptions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(52761285);
  TlStoreBinary::store((var0 = flags_ | (missing_balance_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
}

void payments_getStarsSubscriptions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(52761285);
  TlStoreBinary::store((var0 = flags_ | (missing_balance_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
}

void payments_getStarsSubscriptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsSubscriptions");
    s.store_field("flags", (var0 = flags_ | (missing_balance_ << 0)));
    if (var0 & 1) { s.store_field("missing_balance", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

payments_getStarsSubscriptions::ReturnType payments_getStarsSubscriptions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starsStatus>, 1822222573>::parse(p);
#undef FAIL
}

phone_exportGroupCallInvite::phone_exportGroupCallInvite(int32 flags_, bool can_self_unmute_, object_ptr<InputGroupCall> &&call_)
  : flags_(flags_)
  , can_self_unmute_(can_self_unmute_)
  , call_(std::move(call_))
{}

const std::int32_t phone_exportGroupCallInvite::ID;

void phone_exportGroupCallInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-425040769);
  TlStoreBinary::store((var0 = flags_ | (can_self_unmute_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_exportGroupCallInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-425040769);
  TlStoreBinary::store((var0 = flags_ | (can_self_unmute_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_exportGroupCallInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.exportGroupCallInvite");
    s.store_field("flags", (var0 = flags_ | (can_self_unmute_ << 0)));
    if (var0 & 1) { s.store_field("can_self_unmute", true); }
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

phone_exportGroupCallInvite::ReturnType phone_exportGroupCallInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_exportedGroupCallInvite>, 541839704>::parse(p);
#undef FAIL
}

phone_saveCallDebug::phone_saveCallDebug(object_ptr<inputPhoneCall> &&peer_, object_ptr<dataJSON> &&debug_)
  : peer_(std::move(peer_))
  , debug_(std::move(debug_))
{}

const std::int32_t phone_saveCallDebug::ID;

void phone_saveCallDebug::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(662363518);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(debug_, s);
}

void phone_saveCallDebug::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(662363518);
  TlStoreBoxed<TlStoreObject, 506920429>::store(peer_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(debug_, s);
}

void phone_saveCallDebug::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.saveCallDebug");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("debug", static_cast<const BaseObject *>(debug_.get()));
    s.store_class_end();
  }
}

phone_saveCallDebug::ReturnType phone_saveCallDebug::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

phone_sendGroupCallEncryptedMessage::phone_sendGroupCallEncryptedMessage(object_ptr<InputGroupCall> &&call_, bytes &&encrypted_message_)
  : call_(std::move(call_))
  , encrypted_message_(std::move(encrypted_message_))
{}

const std::int32_t phone_sendGroupCallEncryptedMessage::ID;

void phone_sendGroupCallEncryptedMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-441473683);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreString::store(encrypted_message_, s);
}

void phone_sendGroupCallEncryptedMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-441473683);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreString::store(encrypted_message_, s);
}

void phone_sendGroupCallEncryptedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.sendGroupCallEncryptedMessage");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_bytes_field("encrypted_message", encrypted_message_);
    s.store_class_end();
  }
}

phone_sendGroupCallEncryptedMessage::ReturnType phone_sendGroupCallEncryptedMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

phone_sendGroupCallMessage::phone_sendGroupCallMessage(object_ptr<InputGroupCall> &&call_, int64 random_id_, object_ptr<textWithEntities> &&message_)
  : call_(std::move(call_))
  , random_id_(random_id_)
  , message_(std::move(message_))
{}

const std::int32_t phone_sendGroupCallMessage::ID;

void phone_sendGroupCallMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2021052396);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s);
}

void phone_sendGroupCallMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2021052396);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(message_, s);
}

void phone_sendGroupCallMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.sendGroupCallMessage");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_field("random_id", random_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

phone_sendGroupCallMessage::ReturnType phone_sendGroupCallMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

premium_getUserBoosts::premium_getUserBoosts(object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&user_id_)
  : peer_(std::move(peer_))
  , user_id_(std::move(user_id_))
{}

const std::int32_t premium_getUserBoosts::ID;

void premium_getUserBoosts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(965037343);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void premium_getUserBoosts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(965037343);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void premium_getUserBoosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.getUserBoosts");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

premium_getUserBoosts::ReturnType premium_getUserBoosts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<premium_boostsList>, -2030542532>::parse(p);
#undef FAIL
}

const std::int32_t smsjobs_leave::ID;

void smsjobs_leave::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1734824589);
}

void smsjobs_leave::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1734824589);
}

void smsjobs_leave::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.leave");
    s.store_class_end();
  }
}

smsjobs_leave::ReturnType smsjobs_leave::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stories_getStoryReactionsList::stories_getStoryReactionsList(int32 flags_, bool forwards_first_, object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<Reaction> &&reaction_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , forwards_first_(forwards_first_)
  , peer_(std::move(peer_))
  , id_(id_)
  , reaction_(std::move(reaction_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t stories_getStoryReactionsList::ID;

void stories_getStoryReactionsList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1179482081);
  TlStoreBinary::store((var0 = flags_ | (forwards_first_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s); }
  if (var0 & 2) { TlStoreString::store(offset_, s); }
  TlStoreBinary::store(limit_, s);
}

void stories_getStoryReactionsList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1179482081);
  TlStoreBinary::store((var0 = flags_ | (forwards_first_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s); }
  if (var0 & 2) { TlStoreString::store(offset_, s); }
  TlStoreBinary::store(limit_, s);
}

void stories_getStoryReactionsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getStoryReactionsList");
    s.store_field("flags", (var0 = flags_ | (forwards_first_ << 2)));
    if (var0 & 4) { s.store_field("forwards_first", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    if (var0 & 1) { s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get())); }
    if (var0 & 2) { s.store_field("offset", offset_); }
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stories_getStoryReactionsList::ReturnType stories_getStoryReactionsList::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stories_storyReactionsList>, -1436583780>::parse(p);
#undef FAIL
}

stories_sendReaction::stories_sendReaction(int32 flags_, bool add_to_recent_, object_ptr<InputPeer> &&peer_, int32 story_id_, object_ptr<Reaction> &&reaction_)
  : flags_(flags_)
  , add_to_recent_(add_to_recent_)
  , peer_(std::move(peer_))
  , story_id_(story_id_)
  , reaction_(std::move(reaction_))
{}

const std::int32_t stories_sendReaction::ID;

void stories_sendReaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2144810674);
  TlStoreBinary::store((var0 = flags_ | (add_to_recent_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(story_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
}

void stories_sendReaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2144810674);
  TlStoreBinary::store((var0 = flags_ | (add_to_recent_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(story_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reaction_, s);
}

void stories_sendReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.sendReaction");
    s.store_field("flags", (var0 = flags_ | (add_to_recent_ << 0)));
    if (var0 & 1) { s.store_field("add_to_recent", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("story_id", story_id_);
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_class_end();
  }
}

stories_sendReaction::ReturnType stories_sendReaction::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

users_getSavedMusicByID::users_getSavedMusicByID(object_ptr<InputUser> &&id_, array<object_ptr<InputDocument>> &&documents_)
  : id_(std::move(id_))
  , documents_(std::move(documents_))
{}

const std::int32_t users_getSavedMusicByID::ID;

void users_getSavedMusicByID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1970513129);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(documents_, s);
}

void users_getSavedMusicByID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1970513129);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(documents_, s);
}

void users_getSavedMusicByID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.getSavedMusicByID");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

users_getSavedMusicByID::ReturnType users_getSavedMusicByID::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<users_SavedMusic>::parse(p);
#undef FAIL
}

users_getUsers::users_getUsers(array<object_ptr<InputUser>> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t users_getUsers::ID;

void users_getUsers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(227648840);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void users_getUsers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(227648840);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void users_getUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.getUsers");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

users_getUsers::ReturnType users_getUsers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
