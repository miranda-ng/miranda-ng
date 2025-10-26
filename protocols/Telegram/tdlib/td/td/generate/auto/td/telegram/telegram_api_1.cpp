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


const std::int32_t accessPointRule::ID;

object_ptr<accessPointRule> accessPointRule::fetch(TlBufferParser &p) {
  return make_tl_object<accessPointRule>(p);
}

accessPointRule::accessPointRule(TlBufferParser &p)
  : phone_prefix_rules_(TlFetchString<string>::parse(p))
  , dc_id_(TlFetchInt::parse(p))
  , ips_(TlFetchVector<TlFetchObject<IpPort>>::parse(p))
{}

void accessPointRule::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "accessPointRule");
    s.store_field("phone_prefix_rules", phone_prefix_rules_);
    s.store_field("dc_id", dc_id_);
    { s.store_vector_begin("ips", ips_.size()); for (const auto &_value : ips_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

accountDaysTTL::accountDaysTTL(int32 days_)
  : days_(days_)
{}

const std::int32_t accountDaysTTL::ID;

object_ptr<accountDaysTTL> accountDaysTTL::fetch(TlBufferParser &p) {
  return make_tl_object<accountDaysTTL>(p);
}

accountDaysTTL::accountDaysTTL(TlBufferParser &p)
  : days_(TlFetchInt::parse(p))
{}

void accountDaysTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(days_, s);
}

void accountDaysTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(days_, s);
}

void accountDaysTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "accountDaysTTL");
    s.store_field("days", days_);
    s.store_class_end();
  }
}

attachMenuBotIcon::attachMenuBotIcon()
  : flags_()
  , name_()
  , icon_()
  , colors_()
{}

const std::int32_t attachMenuBotIcon::ID;

object_ptr<attachMenuBotIcon> attachMenuBotIcon::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<attachMenuBotIcon> res = make_tl_object<attachMenuBotIcon>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->name_ = TlFetchString<string>::parse(p);
  res->icon_ = TlFetchObject<Document>::parse(p);
  if (var0 & 1) { res->colors_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<attachMenuBotIconColor>, 1165423600>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void attachMenuBotIcon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachMenuBotIcon");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("name", name_);
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    if (var0 & 1) { { s.store_vector_begin("colors", colors_.size()); for (const auto &_value : colors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<BusinessAwayMessageSchedule> BusinessAwayMessageSchedule::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case businessAwayMessageScheduleAlways::ID:
      return businessAwayMessageScheduleAlways::fetch(p);
    case businessAwayMessageScheduleOutsideWorkHours::ID:
      return businessAwayMessageScheduleOutsideWorkHours::fetch(p);
    case businessAwayMessageScheduleCustom::ID:
      return businessAwayMessageScheduleCustom::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t businessAwayMessageScheduleAlways::ID;

object_ptr<BusinessAwayMessageSchedule> businessAwayMessageScheduleAlways::fetch(TlBufferParser &p) {
  return make_tl_object<businessAwayMessageScheduleAlways>();
}

void businessAwayMessageScheduleAlways::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void businessAwayMessageScheduleAlways::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void businessAwayMessageScheduleAlways::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessageScheduleAlways");
    s.store_class_end();
  }
}

const std::int32_t businessAwayMessageScheduleOutsideWorkHours::ID;

object_ptr<BusinessAwayMessageSchedule> businessAwayMessageScheduleOutsideWorkHours::fetch(TlBufferParser &p) {
  return make_tl_object<businessAwayMessageScheduleOutsideWorkHours>();
}

void businessAwayMessageScheduleOutsideWorkHours::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void businessAwayMessageScheduleOutsideWorkHours::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void businessAwayMessageScheduleOutsideWorkHours::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessageScheduleOutsideWorkHours");
    s.store_class_end();
  }
}

businessAwayMessageScheduleCustom::businessAwayMessageScheduleCustom(int32 start_date_, int32 end_date_)
  : start_date_(start_date_)
  , end_date_(end_date_)
{}

const std::int32_t businessAwayMessageScheduleCustom::ID;

object_ptr<BusinessAwayMessageSchedule> businessAwayMessageScheduleCustom::fetch(TlBufferParser &p) {
  return make_tl_object<businessAwayMessageScheduleCustom>(p);
}

businessAwayMessageScheduleCustom::businessAwayMessageScheduleCustom(TlBufferParser &p)
  : start_date_(TlFetchInt::parse(p))
  , end_date_(TlFetchInt::parse(p))
{}

void businessAwayMessageScheduleCustom::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(start_date_, s);
  TlStoreBinary::store(end_date_, s);
}

void businessAwayMessageScheduleCustom::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(start_date_, s);
  TlStoreBinary::store(end_date_, s);
}

void businessAwayMessageScheduleCustom::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessageScheduleCustom");
    s.store_field("start_date", start_date_);
    s.store_field("end_date", end_date_);
    s.store_class_end();
  }
}

object_ptr<ChannelParticipant> ChannelParticipant::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

channelParticipant::channelParticipant()
  : flags_()
  , user_id_()
  , date_()
  , subscription_until_date_()
{}

const std::int32_t channelParticipant::ID;

object_ptr<ChannelParticipant> channelParticipant::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelParticipant> res = make_tl_object<channelParticipant>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->user_id_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->subscription_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipant");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("user_id", user_id_);
    s.store_field("date", date_);
    if (var0 & 1) { s.store_field("subscription_until_date", subscription_until_date_); }
    s.store_class_end();
  }
}

channelParticipantSelf::channelParticipantSelf()
  : flags_()
  , via_request_()
  , user_id_()
  , inviter_id_()
  , date_()
  , subscription_until_date_()
{}

const std::int32_t channelParticipantSelf::ID;

object_ptr<ChannelParticipant> channelParticipantSelf::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelParticipantSelf> res = make_tl_object<channelParticipantSelf>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->via_request_ = (var0 & 1) != 0;
  res->user_id_ = TlFetchLong::parse(p);
  res->inviter_id_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->subscription_until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelParticipantSelf::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantSelf");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (via_request_ << 0)));
    if (var0 & 1) { s.store_field("via_request", true); }
    s.store_field("user_id", user_id_);
    s.store_field("inviter_id", inviter_id_);
    s.store_field("date", date_);
    if (var0 & 2) { s.store_field("subscription_until_date", subscription_until_date_); }
    s.store_class_end();
  }
}

channelParticipantCreator::channelParticipantCreator()
  : flags_()
  , user_id_()
  , admin_rights_()
  , rank_()
{}

const std::int32_t channelParticipantCreator::ID;

object_ptr<ChannelParticipant> channelParticipantCreator::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelParticipantCreator> res = make_tl_object<channelParticipantCreator>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->user_id_ = TlFetchLong::parse(p);
  res->admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p);
  if (var0 & 1) { res->rank_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelParticipantCreator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantCreator");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("user_id", user_id_);
    s.store_object_field("admin_rights", static_cast<const BaseObject *>(admin_rights_.get()));
    if (var0 & 1) { s.store_field("rank", rank_); }
    s.store_class_end();
  }
}

channelParticipantAdmin::channelParticipantAdmin()
  : flags_()
  , can_edit_()
  , self_()
  , user_id_()
  , inviter_id_()
  , promoted_by_()
  , date_()
  , admin_rights_()
  , rank_()
{}

const std::int32_t channelParticipantAdmin::ID;

object_ptr<ChannelParticipant> channelParticipantAdmin::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelParticipantAdmin> res = make_tl_object<channelParticipantAdmin>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->can_edit_ = (var0 & 1) != 0;
  res->self_ = (var0 & 2) != 0;
  res->user_id_ = TlFetchLong::parse(p);
  if (var0 & 2) { res->inviter_id_ = TlFetchLong::parse(p); }
  res->promoted_by_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p);
  if (var0 & 4) { res->rank_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelParticipantAdmin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantAdmin");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (can_edit_ << 0) | (self_ << 1)));
    if (var0 & 1) { s.store_field("can_edit", true); }
    if (var0 & 2) { s.store_field("self", true); }
    s.store_field("user_id", user_id_);
    if (var0 & 2) { s.store_field("inviter_id", inviter_id_); }
    s.store_field("promoted_by", promoted_by_);
    s.store_field("date", date_);
    s.store_object_field("admin_rights", static_cast<const BaseObject *>(admin_rights_.get()));
    if (var0 & 4) { s.store_field("rank", rank_); }
    s.store_class_end();
  }
}

channelParticipantBanned::channelParticipantBanned()
  : flags_()
  , left_()
  , peer_()
  , kicked_by_()
  , date_()
  , banned_rights_()
{}

const std::int32_t channelParticipantBanned::ID;

object_ptr<ChannelParticipant> channelParticipantBanned::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelParticipantBanned> res = make_tl_object<channelParticipantBanned>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->left_ = (var0 & 1) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->kicked_by_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->banned_rights_ = TlFetchBoxed<TlFetchObject<chatBannedRights>, -1626209256>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelParticipantBanned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantBanned");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (left_ << 0)));
    if (var0 & 1) { s.store_field("left", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("kicked_by", kicked_by_);
    s.store_field("date", date_);
    s.store_object_field("banned_rights", static_cast<const BaseObject *>(banned_rights_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelParticipantLeft::ID;

object_ptr<ChannelParticipant> channelParticipantLeft::fetch(TlBufferParser &p) {
  return make_tl_object<channelParticipantLeft>(p);
}

channelParticipantLeft::channelParticipantLeft(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
{}

void channelParticipantLeft::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelParticipantLeft");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

object_ptr<ChatParticipants> ChatParticipants::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatParticipantsForbidden::ID:
      return chatParticipantsForbidden::fetch(p);
    case chatParticipants::ID:
      return chatParticipants::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

chatParticipantsForbidden::chatParticipantsForbidden()
  : flags_()
  , chat_id_()
  , self_participant_()
{}

const std::int32_t chatParticipantsForbidden::ID;

object_ptr<ChatParticipants> chatParticipantsForbidden::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatParticipantsForbidden> res = make_tl_object<chatParticipantsForbidden>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->chat_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->self_participant_ = TlFetchObject<ChatParticipant>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chatParticipantsForbidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatParticipantsForbidden");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("chat_id", chat_id_);
    if (var0 & 1) { s.store_object_field("self_participant", static_cast<const BaseObject *>(self_participant_.get())); }
    s.store_class_end();
  }
}

const std::int32_t chatParticipants::ID;

object_ptr<ChatParticipants> chatParticipants::fetch(TlBufferParser &p) {
  return make_tl_object<chatParticipants>(p);
}

chatParticipants::chatParticipants(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
  , participants_(TlFetchBoxed<TlFetchVector<TlFetchObject<ChatParticipant>>, 481674261>::parse(p))
  , version_(TlFetchInt::parse(p))
{}

void chatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatParticipants");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("participants", participants_.size()); for (const auto &_value : participants_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("version", version_);
    s.store_class_end();
  }
}

object_ptr<ChatPhoto> ChatPhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case chatPhotoEmpty::ID:
      return chatPhotoEmpty::fetch(p);
    case chatPhoto::ID:
      return chatPhoto::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t chatPhotoEmpty::ID;

object_ptr<ChatPhoto> chatPhotoEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<chatPhotoEmpty>();
}

void chatPhotoEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhotoEmpty");
    s.store_class_end();
  }
}

chatPhoto::chatPhoto()
  : flags_()
  , has_video_()
  , photo_id_()
  , stripped_thumb_()
  , dc_id_()
{}

const std::int32_t chatPhoto::ID;

object_ptr<ChatPhoto> chatPhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chatPhoto> res = make_tl_object<chatPhoto>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_video_ = (var0 & 1) != 0;
  res->photo_id_ = TlFetchLong::parse(p);
  if (var0 & 2) { res->stripped_thumb_ = TlFetchBytes<bytes>::parse(p); }
  res->dc_id_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhoto");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_video_ << 0)));
    if (var0 & 1) { s.store_field("has_video", true); }
    s.store_field("photo_id", photo_id_);
    if (var0 & 2) { s.store_bytes_field("stripped_thumb", stripped_thumb_); }
    s.store_field("dc_id", dc_id_);
    s.store_class_end();
  }
}

object_ptr<DraftMessage> DraftMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case draftMessageEmpty::ID:
      return draftMessageEmpty::fetch(p);
    case draftMessage::ID:
      return draftMessage::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

draftMessageEmpty::draftMessageEmpty()
  : flags_()
  , date_()
{}

const std::int32_t draftMessageEmpty::ID;

object_ptr<DraftMessage> draftMessageEmpty::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<draftMessageEmpty> res = make_tl_object<draftMessageEmpty>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void draftMessageEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "draftMessageEmpty");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("date", date_); }
    s.store_class_end();
  }
}

draftMessage::draftMessage()
  : flags_()
  , no_webpage_()
  , invert_media_()
  , reply_to_()
  , message_()
  , entities_()
  , media_()
  , date_()
  , effect_()
  , suggested_post_()
{}

const std::int32_t draftMessage::ID;

object_ptr<DraftMessage> draftMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<draftMessage> res = make_tl_object<draftMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->no_webpage_ = (var0 & 2) != 0;
  res->invert_media_ = (var0 & 64) != 0;
  if (var0 & 16) { res->reply_to_ = TlFetchObject<InputReplyTo>::parse(p); }
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 8) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 32) { res->media_ = TlFetchObject<InputMedia>::parse(p); }
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 128) { res->effect_ = TlFetchLong::parse(p); }
  if (var0 & 256) { res->suggested_post_ = TlFetchBoxed<TlFetchObject<suggestedPost>, 244201445>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void draftMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "draftMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (no_webpage_ << 1) | (invert_media_ << 6)));
    if (var0 & 2) { s.store_field("no_webpage", true); }
    if (var0 & 64) { s.store_field("invert_media", true); }
    if (var0 & 16) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    s.store_field("message", message_);
    if (var0 & 8) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 32) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    s.store_field("date", date_);
    if (var0 & 128) { s.store_field("effect", effect_); }
    if (var0 & 256) { s.store_object_field("suggested_post", static_cast<const BaseObject *>(suggested_post_.get())); }
    s.store_class_end();
  }
}

const std::int32_t emojiLanguage::ID;

object_ptr<emojiLanguage> emojiLanguage::fetch(TlBufferParser &p) {
  return make_tl_object<emojiLanguage>(p);
}

emojiLanguage::emojiLanguage(TlBufferParser &p)
  : lang_code_(TlFetchString<string>::parse(p))
{}

void emojiLanguage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiLanguage");
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

object_ptr<EmojiStatus> EmojiStatus::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case emojiStatusEmpty::ID:
      return emojiStatusEmpty::fetch(p);
    case emojiStatus::ID:
      return emojiStatus::fetch(p);
    case emojiStatusCollectible::ID:
      return emojiStatusCollectible::fetch(p);
    case inputEmojiStatusCollectible::ID:
      return inputEmojiStatusCollectible::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t emojiStatusEmpty::ID;

object_ptr<EmojiStatus> emojiStatusEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<emojiStatusEmpty>();
}

void emojiStatusEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void emojiStatusEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void emojiStatusEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatusEmpty");
    s.store_class_end();
  }
}

emojiStatus::emojiStatus()
  : flags_()
  , document_id_()
  , until_()
{}

emojiStatus::emojiStatus(int32 flags_, int64 document_id_, int32 until_)
  : flags_(flags_)
  , document_id_(document_id_)
  , until_(until_)
{}

const std::int32_t emojiStatus::ID;

object_ptr<EmojiStatus> emojiStatus::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<emojiStatus> res = make_tl_object<emojiStatus>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->document_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->until_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void emojiStatus::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(document_id_, s);
  if (var0 & 1) { TlStoreBinary::store(until_, s); }
}

void emojiStatus::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(document_id_, s);
  if (var0 & 1) { TlStoreBinary::store(until_, s); }
}

void emojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatus");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("document_id", document_id_);
    if (var0 & 1) { s.store_field("until", until_); }
    s.store_class_end();
  }
}

emojiStatusCollectible::emojiStatusCollectible()
  : flags_()
  , collectible_id_()
  , document_id_()
  , title_()
  , slug_()
  , pattern_document_id_()
  , center_color_()
  , edge_color_()
  , pattern_color_()
  , text_color_()
  , until_()
{}

emojiStatusCollectible::emojiStatusCollectible(int32 flags_, int64 collectible_id_, int64 document_id_, string const &title_, string const &slug_, int64 pattern_document_id_, int32 center_color_, int32 edge_color_, int32 pattern_color_, int32 text_color_, int32 until_)
  : flags_(flags_)
  , collectible_id_(collectible_id_)
  , document_id_(document_id_)
  , title_(title_)
  , slug_(slug_)
  , pattern_document_id_(pattern_document_id_)
  , center_color_(center_color_)
  , edge_color_(edge_color_)
  , pattern_color_(pattern_color_)
  , text_color_(text_color_)
  , until_(until_)
{}

const std::int32_t emojiStatusCollectible::ID;

object_ptr<EmojiStatus> emojiStatusCollectible::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<emojiStatusCollectible> res = make_tl_object<emojiStatusCollectible>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->collectible_id_ = TlFetchLong::parse(p);
  res->document_id_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->slug_ = TlFetchString<string>::parse(p);
  res->pattern_document_id_ = TlFetchLong::parse(p);
  res->center_color_ = TlFetchInt::parse(p);
  res->edge_color_ = TlFetchInt::parse(p);
  res->pattern_color_ = TlFetchInt::parse(p);
  res->text_color_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->until_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void emojiStatusCollectible::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(collectible_id_, s);
  TlStoreBinary::store(document_id_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(slug_, s);
  TlStoreBinary::store(pattern_document_id_, s);
  TlStoreBinary::store(center_color_, s);
  TlStoreBinary::store(edge_color_, s);
  TlStoreBinary::store(pattern_color_, s);
  TlStoreBinary::store(text_color_, s);
  if (var0 & 1) { TlStoreBinary::store(until_, s); }
}

void emojiStatusCollectible::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(collectible_id_, s);
  TlStoreBinary::store(document_id_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(slug_, s);
  TlStoreBinary::store(pattern_document_id_, s);
  TlStoreBinary::store(center_color_, s);
  TlStoreBinary::store(edge_color_, s);
  TlStoreBinary::store(pattern_color_, s);
  TlStoreBinary::store(text_color_, s);
  if (var0 & 1) { TlStoreBinary::store(until_, s); }
}

void emojiStatusCollectible::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatusCollectible");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("collectible_id", collectible_id_);
    s.store_field("document_id", document_id_);
    s.store_field("title", title_);
    s.store_field("slug", slug_);
    s.store_field("pattern_document_id", pattern_document_id_);
    s.store_field("center_color", center_color_);
    s.store_field("edge_color", edge_color_);
    s.store_field("pattern_color", pattern_color_);
    s.store_field("text_color", text_color_);
    if (var0 & 1) { s.store_field("until", until_); }
    s.store_class_end();
  }
}

inputEmojiStatusCollectible::inputEmojiStatusCollectible()
  : flags_()
  , collectible_id_()
  , until_()
{}

inputEmojiStatusCollectible::inputEmojiStatusCollectible(int32 flags_, int64 collectible_id_, int32 until_)
  : flags_(flags_)
  , collectible_id_(collectible_id_)
  , until_(until_)
{}

const std::int32_t inputEmojiStatusCollectible::ID;

object_ptr<EmojiStatus> inputEmojiStatusCollectible::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputEmojiStatusCollectible> res = make_tl_object<inputEmojiStatusCollectible>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->collectible_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->until_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputEmojiStatusCollectible::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(collectible_id_, s);
  if (var0 & 1) { TlStoreBinary::store(until_, s); }
}

void inputEmojiStatusCollectible::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(collectible_id_, s);
  if (var0 & 1) { TlStoreBinary::store(until_, s); }
}

void inputEmojiStatusCollectible::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputEmojiStatusCollectible");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("collectible_id", collectible_id_);
    if (var0 & 1) { s.store_field("until", until_); }
    s.store_class_end();
  }
}

folder::folder()
  : flags_()
  , autofill_new_broadcasts_()
  , autofill_public_groups_()
  , autofill_new_correspondents_()
  , id_()
  , title_()
  , photo_()
{}

const std::int32_t folder::ID;

object_ptr<folder> folder::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<folder> res = make_tl_object<folder>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->autofill_new_broadcasts_ = (var0 & 1) != 0;
  res->autofill_public_groups_ = (var0 & 2) != 0;
  res->autofill_new_correspondents_ = (var0 & 4) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  if (var0 & 8) { res->photo_ = TlFetchObject<ChatPhoto>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void folder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "folder");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (autofill_new_broadcasts_ << 0) | (autofill_public_groups_ << 1) | (autofill_new_correspondents_ << 2)));
    if (var0 & 1) { s.store_field("autofill_new_broadcasts", true); }
    if (var0 & 2) { s.store_field("autofill_public_groups", true); }
    if (var0 & 4) { s.store_field("autofill_new_correspondents", true); }
    s.store_field("id", id_);
    s.store_field("title", title_);
    if (var0 & 8) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_class_end();
  }
}

inputBusinessGreetingMessage::inputBusinessGreetingMessage(int32 shortcut_id_, object_ptr<inputBusinessRecipients> &&recipients_, int32 no_activity_days_)
  : shortcut_id_(shortcut_id_)
  , recipients_(std::move(recipients_))
  , no_activity_days_(no_activity_days_)
{}

const std::int32_t inputBusinessGreetingMessage::ID;

void inputBusinessGreetingMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxed<TlStoreObject, 1871393450>::store(recipients_, s);
  TlStoreBinary::store(no_activity_days_, s);
}

void inputBusinessGreetingMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxed<TlStoreObject, 1871393450>::store(recipients_, s);
  TlStoreBinary::store(no_activity_days_, s);
}

void inputBusinessGreetingMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessGreetingMessage");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_field("no_activity_days", no_activity_days_);
    s.store_class_end();
  }
}

inputChatlistDialogFilter::inputChatlistDialogFilter(int32 filter_id_)
  : filter_id_(filter_id_)
{}

const std::int32_t inputChatlistDialogFilter::ID;

void inputChatlistDialogFilter::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(filter_id_, s);
}

void inputChatlistDialogFilter::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(filter_id_, s);
}

void inputChatlistDialogFilter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatlistDialogFilter");
    s.store_field("filter_id", filter_id_);
    s.store_class_end();
  }
}

inputFolderPeer::inputFolderPeer(object_ptr<InputPeer> &&peer_, int32 folder_id_)
  : peer_(std::move(peer_))
  , folder_id_(folder_id_)
{}

const std::int32_t inputFolderPeer::ID;

void inputFolderPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(folder_id_, s);
}

void inputFolderPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(folder_id_, s);
}

void inputFolderPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFolderPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("folder_id", folder_id_);
    s.store_class_end();
  }
}

object_ptr<InputMedia> InputMedia::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputMediaEmpty::ID;

object_ptr<InputMedia> inputMediaEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaEmpty>();
}

void inputMediaEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputMediaEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputMediaEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaEmpty");
    s.store_class_end();
  }
}

inputMediaUploadedPhoto::inputMediaUploadedPhoto()
  : flags_()
  , spoiler_()
  , file_()
  , stickers_()
  , ttl_seconds_()
{}

inputMediaUploadedPhoto::inputMediaUploadedPhoto(int32 flags_, bool spoiler_, object_ptr<InputFile> &&file_, array<object_ptr<InputDocument>> &&stickers_, int32 ttl_seconds_)
  : flags_(flags_)
  , spoiler_(spoiler_)
  , file_(std::move(file_))
  , stickers_(std::move(stickers_))
  , ttl_seconds_(ttl_seconds_)
{}

const std::int32_t inputMediaUploadedPhoto::ID;

object_ptr<InputMedia> inputMediaUploadedPhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaUploadedPhoto> res = make_tl_object<inputMediaUploadedPhoto>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->spoiler_ = (var0 & 4) != 0;
  res->file_ = TlFetchObject<InputFile>::parse(p);
  if (var0 & 1) { res->stickers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputDocument>>, 481674261>::parse(p); }
  if (var0 & 2) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaUploadedPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stickers_, s); }
  if (var0 & 2) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaUploadedPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stickers_, s); }
  if (var0 & 2) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaUploadedPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaUploadedPhoto");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (spoiler_ << 2)));
    if (var0 & 4) { s.store_field("spoiler", true); }
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    if (var0 & 1) { { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2) { s.store_field("ttl_seconds", ttl_seconds_); }
    s.store_class_end();
  }
}

inputMediaPhoto::inputMediaPhoto()
  : flags_()
  , spoiler_()
  , id_()
  , ttl_seconds_()
{}

inputMediaPhoto::inputMediaPhoto(int32 flags_, bool spoiler_, object_ptr<InputPhoto> &&id_, int32 ttl_seconds_)
  : flags_(flags_)
  , spoiler_(spoiler_)
  , id_(std::move(id_))
  , ttl_seconds_(ttl_seconds_)
{}

const std::int32_t inputMediaPhoto::ID;

object_ptr<InputMedia> inputMediaPhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaPhoto> res = make_tl_object<inputMediaPhoto>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->spoiler_ = (var0 & 2) != 0;
  res->id_ = TlFetchObject<InputPhoto>::parse(p);
  if (var0 & 1) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaPhoto");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (spoiler_ << 1)));
    if (var0 & 2) { s.store_field("spoiler", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    if (var0 & 1) { s.store_field("ttl_seconds", ttl_seconds_); }
    s.store_class_end();
  }
}

inputMediaGeoPoint::inputMediaGeoPoint(object_ptr<InputGeoPoint> &&geo_point_)
  : geo_point_(std::move(geo_point_))
{}

const std::int32_t inputMediaGeoPoint::ID;

object_ptr<InputMedia> inputMediaGeoPoint::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaGeoPoint>(p);
}

inputMediaGeoPoint::inputMediaGeoPoint(TlBufferParser &p)
  : geo_point_(TlFetchObject<InputGeoPoint>::parse(p))
{}

void inputMediaGeoPoint::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
}

void inputMediaGeoPoint::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
}

void inputMediaGeoPoint::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaGeoPoint");
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    s.store_class_end();
  }
}

inputMediaContact::inputMediaContact(string const &phone_number_, string const &first_name_, string const &last_name_, string const &vcard_)
  : phone_number_(phone_number_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , vcard_(vcard_)
{}

const std::int32_t inputMediaContact::ID;

object_ptr<InputMedia> inputMediaContact::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaContact>(p);
}

inputMediaContact::inputMediaContact(TlBufferParser &p)
  : phone_number_(TlFetchString<string>::parse(p))
  , first_name_(TlFetchString<string>::parse(p))
  , last_name_(TlFetchString<string>::parse(p))
  , vcard_(TlFetchString<string>::parse(p))
{}

void inputMediaContact::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreString::store(vcard_, s);
}

void inputMediaContact::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreString::store(vcard_, s);
}

void inputMediaContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaContact");
    s.store_field("phone_number", phone_number_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("vcard", vcard_);
    s.store_class_end();
  }
}

inputMediaUploadedDocument::inputMediaUploadedDocument()
  : flags_()
  , nosound_video_()
  , force_file_()
  , spoiler_()
  , file_()
  , thumb_()
  , mime_type_()
  , attributes_()
  , stickers_()
  , video_cover_()
  , video_timestamp_()
  , ttl_seconds_()
{}

inputMediaUploadedDocument::inputMediaUploadedDocument(int32 flags_, bool nosound_video_, bool force_file_, bool spoiler_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&thumb_, string const &mime_type_, array<object_ptr<DocumentAttribute>> &&attributes_, array<object_ptr<InputDocument>> &&stickers_, object_ptr<InputPhoto> &&video_cover_, int32 video_timestamp_, int32 ttl_seconds_)
  : flags_(flags_)
  , nosound_video_(nosound_video_)
  , force_file_(force_file_)
  , spoiler_(spoiler_)
  , file_(std::move(file_))
  , thumb_(std::move(thumb_))
  , mime_type_(mime_type_)
  , attributes_(std::move(attributes_))
  , stickers_(std::move(stickers_))
  , video_cover_(std::move(video_cover_))
  , video_timestamp_(video_timestamp_)
  , ttl_seconds_(ttl_seconds_)
{}

const std::int32_t inputMediaUploadedDocument::ID;

object_ptr<InputMedia> inputMediaUploadedDocument::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaUploadedDocument> res = make_tl_object<inputMediaUploadedDocument>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->nosound_video_ = (var0 & 8) != 0;
  res->force_file_ = (var0 & 16) != 0;
  res->spoiler_ = (var0 & 32) != 0;
  res->file_ = TlFetchObject<InputFile>::parse(p);
  if (var0 & 4) { res->thumb_ = TlFetchObject<InputFile>::parse(p); }
  res->mime_type_ = TlFetchString<string>::parse(p);
  res->attributes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p);
  if (var0 & 1) { res->stickers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputDocument>>, 481674261>::parse(p); }
  if (var0 & 64) { res->video_cover_ = TlFetchObject<InputPhoto>::parse(p); }
  if (var0 & 128) { res->video_timestamp_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaUploadedDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (nosound_video_ << 3) | (force_file_ << 4) | (spoiler_ << 5)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  TlStoreString::store(mime_type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stickers_, s); }
  if (var0 & 64) { TlStoreBoxedUnknown<TlStoreObject>::store(video_cover_, s); }
  if (var0 & 128) { TlStoreBinary::store(video_timestamp_, s); }
  if (var0 & 2) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaUploadedDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (nosound_video_ << 3) | (force_file_ << 4) | (spoiler_ << 5)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  TlStoreString::store(mime_type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(stickers_, s); }
  if (var0 & 64) { TlStoreBoxedUnknown<TlStoreObject>::store(video_cover_, s); }
  if (var0 & 128) { TlStoreBinary::store(video_timestamp_, s); }
  if (var0 & 2) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaUploadedDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaUploadedDocument");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (nosound_video_ << 3) | (force_file_ << 4) | (spoiler_ << 5)));
    if (var0 & 8) { s.store_field("nosound_video", true); }
    if (var0 & 16) { s.store_field("force_file", true); }
    if (var0 & 32) { s.store_field("spoiler", true); }
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    if (var0 & 4) { s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get())); }
    s.store_field("mime_type", mime_type_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 64) { s.store_object_field("video_cover", static_cast<const BaseObject *>(video_cover_.get())); }
    if (var0 & 128) { s.store_field("video_timestamp", video_timestamp_); }
    if (var0 & 2) { s.store_field("ttl_seconds", ttl_seconds_); }
    s.store_class_end();
  }
}

inputMediaDocument::inputMediaDocument()
  : flags_()
  , spoiler_()
  , id_()
  , video_cover_()
  , video_timestamp_()
  , ttl_seconds_()
  , query_()
{}

inputMediaDocument::inputMediaDocument(int32 flags_, bool spoiler_, object_ptr<InputDocument> &&id_, object_ptr<InputPhoto> &&video_cover_, int32 video_timestamp_, int32 ttl_seconds_, string const &query_)
  : flags_(flags_)
  , spoiler_(spoiler_)
  , id_(std::move(id_))
  , video_cover_(std::move(video_cover_))
  , video_timestamp_(video_timestamp_)
  , ttl_seconds_(ttl_seconds_)
  , query_(query_)
{}

const std::int32_t inputMediaDocument::ID;

object_ptr<InputMedia> inputMediaDocument::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaDocument> res = make_tl_object<inputMediaDocument>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->spoiler_ = (var0 & 4) != 0;
  res->id_ = TlFetchObject<InputDocument>::parse(p);
  if (var0 & 8) { res->video_cover_ = TlFetchObject<InputPhoto>::parse(p); }
  if (var0 & 16) { res->video_timestamp_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->query_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(video_cover_, s); }
  if (var0 & 16) { TlStoreBinary::store(video_timestamp_, s); }
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
  if (var0 & 2) { TlStoreString::store(query_, s); }
}

void inputMediaDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(video_cover_, s); }
  if (var0 & 16) { TlStoreBinary::store(video_timestamp_, s); }
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
  if (var0 & 2) { TlStoreString::store(query_, s); }
}

void inputMediaDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaDocument");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (spoiler_ << 2)));
    if (var0 & 4) { s.store_field("spoiler", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    if (var0 & 8) { s.store_object_field("video_cover", static_cast<const BaseObject *>(video_cover_.get())); }
    if (var0 & 16) { s.store_field("video_timestamp", video_timestamp_); }
    if (var0 & 1) { s.store_field("ttl_seconds", ttl_seconds_); }
    if (var0 & 2) { s.store_field("query", query_); }
    s.store_class_end();
  }
}

inputMediaVenue::inputMediaVenue(object_ptr<InputGeoPoint> &&geo_point_, string const &title_, string const &address_, string const &provider_, string const &venue_id_, string const &venue_type_)
  : geo_point_(std::move(geo_point_))
  , title_(title_)
  , address_(address_)
  , provider_(provider_)
  , venue_id_(venue_id_)
  , venue_type_(venue_type_)
{}

const std::int32_t inputMediaVenue::ID;

object_ptr<InputMedia> inputMediaVenue::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaVenue>(p);
}

inputMediaVenue::inputMediaVenue(TlBufferParser &p)
  : geo_point_(TlFetchObject<InputGeoPoint>::parse(p))
  , title_(TlFetchString<string>::parse(p))
  , address_(TlFetchString<string>::parse(p))
  , provider_(TlFetchString<string>::parse(p))
  , venue_id_(TlFetchString<string>::parse(p))
  , venue_type_(TlFetchString<string>::parse(p))
{}

void inputMediaVenue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
  TlStoreString::store(venue_type_, s);
}

void inputMediaVenue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
  TlStoreString::store(venue_type_, s);
}

void inputMediaVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaVenue");
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    s.store_field("title", title_);
    s.store_field("address", address_);
    s.store_field("provider", provider_);
    s.store_field("venue_id", venue_id_);
    s.store_field("venue_type", venue_type_);
    s.store_class_end();
  }
}

inputMediaPhotoExternal::inputMediaPhotoExternal()
  : flags_()
  , spoiler_()
  , url_()
  , ttl_seconds_()
{}

inputMediaPhotoExternal::inputMediaPhotoExternal(int32 flags_, bool spoiler_, string const &url_, int32 ttl_seconds_)
  : flags_(flags_)
  , spoiler_(spoiler_)
  , url_(url_)
  , ttl_seconds_(ttl_seconds_)
{}

const std::int32_t inputMediaPhotoExternal::ID;

object_ptr<InputMedia> inputMediaPhotoExternal::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaPhotoExternal> res = make_tl_object<inputMediaPhotoExternal>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->spoiler_ = (var0 & 2) != 0;
  res->url_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaPhotoExternal::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 1)), s);
  TlStoreString::store(url_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaPhotoExternal::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 1)), s);
  TlStoreString::store(url_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
}

void inputMediaPhotoExternal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaPhotoExternal");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (spoiler_ << 1)));
    if (var0 & 2) { s.store_field("spoiler", true); }
    s.store_field("url", url_);
    if (var0 & 1) { s.store_field("ttl_seconds", ttl_seconds_); }
    s.store_class_end();
  }
}

inputMediaDocumentExternal::inputMediaDocumentExternal()
  : flags_()
  , spoiler_()
  , url_()
  , ttl_seconds_()
  , video_cover_()
  , video_timestamp_()
{}

inputMediaDocumentExternal::inputMediaDocumentExternal(int32 flags_, bool spoiler_, string const &url_, int32 ttl_seconds_, object_ptr<InputPhoto> &&video_cover_, int32 video_timestamp_)
  : flags_(flags_)
  , spoiler_(spoiler_)
  , url_(url_)
  , ttl_seconds_(ttl_seconds_)
  , video_cover_(std::move(video_cover_))
  , video_timestamp_(video_timestamp_)
{}

const std::int32_t inputMediaDocumentExternal::ID;

object_ptr<InputMedia> inputMediaDocumentExternal::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaDocumentExternal> res = make_tl_object<inputMediaDocumentExternal>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->spoiler_ = (var0 & 2) != 0;
  res->url_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->ttl_seconds_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->video_cover_ = TlFetchObject<InputPhoto>::parse(p); }
  if (var0 & 8) { res->video_timestamp_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaDocumentExternal::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 1)), s);
  TlStoreString::store(url_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(video_cover_, s); }
  if (var0 & 8) { TlStoreBinary::store(video_timestamp_, s); }
}

void inputMediaDocumentExternal::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (spoiler_ << 1)), s);
  TlStoreString::store(url_, s);
  if (var0 & 1) { TlStoreBinary::store(ttl_seconds_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(video_cover_, s); }
  if (var0 & 8) { TlStoreBinary::store(video_timestamp_, s); }
}

void inputMediaDocumentExternal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaDocumentExternal");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (spoiler_ << 1)));
    if (var0 & 2) { s.store_field("spoiler", true); }
    s.store_field("url", url_);
    if (var0 & 1) { s.store_field("ttl_seconds", ttl_seconds_); }
    if (var0 & 4) { s.store_object_field("video_cover", static_cast<const BaseObject *>(video_cover_.get())); }
    if (var0 & 8) { s.store_field("video_timestamp", video_timestamp_); }
    s.store_class_end();
  }
}

inputMediaGame::inputMediaGame(object_ptr<InputGame> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t inputMediaGame::ID;

object_ptr<InputMedia> inputMediaGame::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaGame>(p);
}

inputMediaGame::inputMediaGame(TlBufferParser &p)
  : id_(TlFetchObject<InputGame>::parse(p))
{}

void inputMediaGame::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputMediaGame::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputMediaGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaGame");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

inputMediaInvoice::inputMediaInvoice()
  : flags_()
  , title_()
  , description_()
  , photo_()
  , invoice_()
  , payload_()
  , provider_()
  , provider_data_()
  , start_param_()
  , extended_media_()
{}

inputMediaInvoice::inputMediaInvoice(int32 flags_, string const &title_, string const &description_, object_ptr<inputWebDocument> &&photo_, object_ptr<invoice> &&invoice_, bytes &&payload_, string const &provider_, object_ptr<dataJSON> &&provider_data_, string const &start_param_, object_ptr<InputMedia> &&extended_media_)
  : flags_(flags_)
  , title_(title_)
  , description_(description_)
  , photo_(std::move(photo_))
  , invoice_(std::move(invoice_))
  , payload_(std::move(payload_))
  , provider_(provider_)
  , provider_data_(std::move(provider_data_))
  , start_param_(start_param_)
  , extended_media_(std::move(extended_media_))
{}

const std::int32_t inputMediaInvoice::ID;

object_ptr<InputMedia> inputMediaInvoice::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaInvoice> res = make_tl_object<inputMediaInvoice>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->photo_ = TlFetchBoxed<TlFetchObject<inputWebDocument>, -1678949555>::parse(p); }
  res->invoice_ = TlFetchBoxed<TlFetchObject<invoice>, 77522308>::parse(p);
  res->payload_ = TlFetchBytes<bytes>::parse(p);
  if (var0 & 8) { res->provider_ = TlFetchString<string>::parse(p); }
  res->provider_data_ = TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p);
  if (var0 & 2) { res->start_param_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->extended_media_ = TlFetchObject<InputMedia>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaInvoice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(description_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1678949555>::store(photo_, s); }
  TlStoreBoxed<TlStoreObject, 77522308>::store(invoice_, s);
  TlStoreString::store(payload_, s);
  if (var0 & 8) { TlStoreString::store(provider_, s); }
  TlStoreBoxed<TlStoreObject, 2104790276>::store(provider_data_, s);
  if (var0 & 2) { TlStoreString::store(start_param_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(extended_media_, s); }
}

void inputMediaInvoice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(title_, s);
  TlStoreString::store(description_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1678949555>::store(photo_, s); }
  TlStoreBoxed<TlStoreObject, 77522308>::store(invoice_, s);
  TlStoreString::store(payload_, s);
  if (var0 & 8) { TlStoreString::store(provider_, s); }
  TlStoreBoxed<TlStoreObject, 2104790276>::store(provider_data_, s);
  if (var0 & 2) { TlStoreString::store(start_param_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(extended_media_, s); }
}

void inputMediaInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaInvoice");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 1) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_bytes_field("payload", payload_);
    if (var0 & 8) { s.store_field("provider", provider_); }
    s.store_object_field("provider_data", static_cast<const BaseObject *>(provider_data_.get()));
    if (var0 & 2) { s.store_field("start_param", start_param_); }
    if (var0 & 4) { s.store_object_field("extended_media", static_cast<const BaseObject *>(extended_media_.get())); }
    s.store_class_end();
  }
}

inputMediaGeoLive::inputMediaGeoLive()
  : flags_()
  , stopped_()
  , geo_point_()
  , heading_()
  , period_()
  , proximity_notification_radius_()
{}

inputMediaGeoLive::inputMediaGeoLive(int32 flags_, bool stopped_, object_ptr<InputGeoPoint> &&geo_point_, int32 heading_, int32 period_, int32 proximity_notification_radius_)
  : flags_(flags_)
  , stopped_(stopped_)
  , geo_point_(std::move(geo_point_))
  , heading_(heading_)
  , period_(period_)
  , proximity_notification_radius_(proximity_notification_radius_)
{}

const std::int32_t inputMediaGeoLive::ID;

object_ptr<InputMedia> inputMediaGeoLive::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaGeoLive> res = make_tl_object<inputMediaGeoLive>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->stopped_ = (var0 & 1) != 0;
  res->geo_point_ = TlFetchObject<InputGeoPoint>::parse(p);
  if (var0 & 4) { res->heading_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->period_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->proximity_notification_radius_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaGeoLive::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (stopped_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  if (var0 & 4) { TlStoreBinary::store(heading_, s); }
  if (var0 & 2) { TlStoreBinary::store(period_, s); }
  if (var0 & 8) { TlStoreBinary::store(proximity_notification_radius_, s); }
}

void inputMediaGeoLive::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (stopped_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  if (var0 & 4) { TlStoreBinary::store(heading_, s); }
  if (var0 & 2) { TlStoreBinary::store(period_, s); }
  if (var0 & 8) { TlStoreBinary::store(proximity_notification_radius_, s); }
}

void inputMediaGeoLive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaGeoLive");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (stopped_ << 0)));
    if (var0 & 1) { s.store_field("stopped", true); }
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    if (var0 & 4) { s.store_field("heading", heading_); }
    if (var0 & 2) { s.store_field("period", period_); }
    if (var0 & 8) { s.store_field("proximity_notification_radius", proximity_notification_radius_); }
    s.store_class_end();
  }
}

inputMediaPoll::inputMediaPoll()
  : flags_()
  , poll_()
  , correct_answers_()
  , solution_()
  , solution_entities_()
{}

inputMediaPoll::inputMediaPoll(int32 flags_, object_ptr<poll> &&poll_, array<bytes> &&correct_answers_, string const &solution_, array<object_ptr<MessageEntity>> &&solution_entities_)
  : flags_(flags_)
  , poll_(std::move(poll_))
  , correct_answers_(std::move(correct_answers_))
  , solution_(solution_)
  , solution_entities_(std::move(solution_entities_))
{}

const std::int32_t inputMediaPoll::ID;

object_ptr<InputMedia> inputMediaPoll::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaPoll> res = make_tl_object<inputMediaPoll>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->poll_ = TlFetchBoxed<TlFetchObject<poll>, 1484026161>::parse(p);
  if (var0 & 1) { res->correct_answers_ = TlFetchBoxed<TlFetchVector<TlFetchBytes<bytes>>, 481674261>::parse(p); }
  if (var0 & 2) { res->solution_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->solution_entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaPoll::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreObject, 1484026161>::store(poll_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(correct_answers_, s); }
  if (var0 & 2) { TlStoreString::store(solution_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(solution_entities_, s); }
}

void inputMediaPoll::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxed<TlStoreObject, 1484026161>::store(poll_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(correct_answers_, s); }
  if (var0 & 2) { TlStoreString::store(solution_, s); }
  if (var0 & 2) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(solution_entities_, s); }
}

void inputMediaPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaPoll");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("poll", static_cast<const BaseObject *>(poll_.get()));
    if (var0 & 1) { { s.store_vector_begin("correct_answers", correct_answers_.size()); for (const auto &_value : correct_answers_) { s.store_bytes_field("", _value); } s.store_class_end(); } }
    if (var0 & 2) { s.store_field("solution", solution_); }
    if (var0 & 2) { { s.store_vector_begin("solution_entities", solution_entities_.size()); for (const auto &_value : solution_entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

inputMediaDice::inputMediaDice(string const &emoticon_)
  : emoticon_(emoticon_)
{}

const std::int32_t inputMediaDice::ID;

object_ptr<InputMedia> inputMediaDice::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaDice>(p);
}

inputMediaDice::inputMediaDice(TlBufferParser &p)
  : emoticon_(TlFetchString<string>::parse(p))
{}

void inputMediaDice::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void inputMediaDice::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(emoticon_, s);
}

void inputMediaDice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaDice");
    s.store_field("emoticon", emoticon_);
    s.store_class_end();
  }
}

inputMediaStory::inputMediaStory(object_ptr<InputPeer> &&peer_, int32 id_)
  : peer_(std::move(peer_))
  , id_(id_)
{}

const std::int32_t inputMediaStory::ID;

object_ptr<InputMedia> inputMediaStory::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaStory>(p);
}

inputMediaStory::inputMediaStory(TlBufferParser &p)
  : peer_(TlFetchObject<InputPeer>::parse(p))
  , id_(TlFetchInt::parse(p))
{}

void inputMediaStory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void inputMediaStory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void inputMediaStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaStory");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

inputMediaWebPage::inputMediaWebPage()
  : flags_()
  , force_large_media_()
  , force_small_media_()
  , optional_()
  , url_()
{}

inputMediaWebPage::inputMediaWebPage(int32 flags_, bool force_large_media_, bool force_small_media_, bool optional_, string const &url_)
  : flags_(flags_)
  , force_large_media_(force_large_media_)
  , force_small_media_(force_small_media_)
  , optional_(optional_)
  , url_(url_)
{}

const std::int32_t inputMediaWebPage::ID;

object_ptr<InputMedia> inputMediaWebPage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaWebPage> res = make_tl_object<inputMediaWebPage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->force_large_media_ = (var0 & 1) != 0;
  res->force_small_media_ = (var0 & 2) != 0;
  res->optional_ = (var0 & 4) != 0;
  res->url_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaWebPage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (force_large_media_ << 0) | (force_small_media_ << 1) | (optional_ << 2)), s);
  TlStoreString::store(url_, s);
}

void inputMediaWebPage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (force_large_media_ << 0) | (force_small_media_ << 1) | (optional_ << 2)), s);
  TlStoreString::store(url_, s);
}

void inputMediaWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaWebPage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (force_large_media_ << 0) | (force_small_media_ << 1) | (optional_ << 2)));
    if (var0 & 1) { s.store_field("force_large_media", true); }
    if (var0 & 2) { s.store_field("force_small_media", true); }
    if (var0 & 4) { s.store_field("optional", true); }
    s.store_field("url", url_);
    s.store_class_end();
  }
}

inputMediaPaidMedia::inputMediaPaidMedia()
  : flags_()
  , stars_amount_()
  , extended_media_()
  , payload_()
{}

inputMediaPaidMedia::inputMediaPaidMedia(int32 flags_, int64 stars_amount_, array<object_ptr<InputMedia>> &&extended_media_, string const &payload_)
  : flags_(flags_)
  , stars_amount_(stars_amount_)
  , extended_media_(std::move(extended_media_))
  , payload_(payload_)
{}

const std::int32_t inputMediaPaidMedia::ID;

object_ptr<InputMedia> inputMediaPaidMedia::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<inputMediaPaidMedia> res = make_tl_object<inputMediaPaidMedia>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->stars_amount_ = TlFetchLong::parse(p);
  res->extended_media_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputMedia>>, 481674261>::parse(p);
  if (var0 & 1) { res->payload_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void inputMediaPaidMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(stars_amount_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(extended_media_, s);
  if (var0 & 1) { TlStoreString::store(payload_, s); }
}

void inputMediaPaidMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(stars_amount_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(extended_media_, s);
  if (var0 & 1) { TlStoreString::store(payload_, s); }
}

void inputMediaPaidMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaPaidMedia");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("stars_amount", stars_amount_);
    { s.store_vector_begin("extended_media", extended_media_.size()); for (const auto &_value : extended_media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("payload", payload_); }
    s.store_class_end();
  }
}

inputMediaTodo::inputMediaTodo(object_ptr<todoList> &&todo_)
  : todo_(std::move(todo_))
{}

const std::int32_t inputMediaTodo::ID;

object_ptr<InputMedia> inputMediaTodo::fetch(TlBufferParser &p) {
  return make_tl_object<inputMediaTodo>(p);
}

inputMediaTodo::inputMediaTodo(TlBufferParser &p)
  : todo_(TlFetchBoxed<TlFetchObject<todoList>, 1236871718>::parse(p))
{}

void inputMediaTodo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 1236871718>::store(todo_, s);
}

void inputMediaTodo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreObject, 1236871718>::store(todo_, s);
}

void inputMediaTodo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMediaTodo");
    s.store_object_field("todo", static_cast<const BaseObject *>(todo_.get()));
    s.store_class_end();
  }
}

object_ptr<InputPhoto> InputPhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputPhotoEmpty::ID:
      return inputPhotoEmpty::fetch(p);
    case inputPhoto::ID:
      return inputPhoto::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputPhotoEmpty::ID;

object_ptr<InputPhoto> inputPhotoEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputPhotoEmpty>();
}

void inputPhotoEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPhotoEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPhotoEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPhotoEmpty");
    s.store_class_end();
  }
}

inputPhoto::inputPhoto(int64 id_, int64 access_hash_, bytes &&file_reference_)
  : id_(id_)
  , access_hash_(access_hash_)
  , file_reference_(std::move(file_reference_))
{}

const std::int32_t inputPhoto::ID;

object_ptr<InputPhoto> inputPhoto::fetch(TlBufferParser &p) {
  return make_tl_object<inputPhoto>(p);
}

inputPhoto::inputPhoto(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , file_reference_(TlFetchBytes<bytes>::parse(p))
{}

void inputPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
}

void inputPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreString::store(file_reference_, s);
}

void inputPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPhoto");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueAllowContacts::ID;

void inputPrivacyValueAllowContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueAllowContacts");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueAllowAll::ID;

void inputPrivacyValueAllowAll::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowAll::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueAllowAll");
    s.store_class_end();
  }
}

inputPrivacyValueAllowUsers::inputPrivacyValueAllowUsers(array<object_ptr<InputUser>> &&users_)
  : users_(std::move(users_))
{}

const std::int32_t inputPrivacyValueAllowUsers::ID;

void inputPrivacyValueAllowUsers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void inputPrivacyValueAllowUsers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void inputPrivacyValueAllowUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueAllowUsers");
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueDisallowContacts::ID;

void inputPrivacyValueDisallowContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueDisallowContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueDisallowContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueDisallowContacts");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueDisallowAll::ID;

void inputPrivacyValueDisallowAll::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueDisallowAll::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueDisallowAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueDisallowAll");
    s.store_class_end();
  }
}

inputPrivacyValueDisallowUsers::inputPrivacyValueDisallowUsers(array<object_ptr<InputUser>> &&users_)
  : users_(std::move(users_))
{}

const std::int32_t inputPrivacyValueDisallowUsers::ID;

void inputPrivacyValueDisallowUsers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void inputPrivacyValueDisallowUsers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void inputPrivacyValueDisallowUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueDisallowUsers");
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputPrivacyValueAllowChatParticipants::inputPrivacyValueAllowChatParticipants(array<int64> &&chats_)
  : chats_(std::move(chats_))
{}

const std::int32_t inputPrivacyValueAllowChatParticipants::ID;

void inputPrivacyValueAllowChatParticipants::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(chats_, s);
}

void inputPrivacyValueAllowChatParticipants::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(chats_, s);
}

void inputPrivacyValueAllowChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueAllowChatParticipants");
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputPrivacyValueDisallowChatParticipants::inputPrivacyValueDisallowChatParticipants(array<int64> &&chats_)
  : chats_(std::move(chats_))
{}

const std::int32_t inputPrivacyValueDisallowChatParticipants::ID;

void inputPrivacyValueDisallowChatParticipants::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(chats_, s);
}

void inputPrivacyValueDisallowChatParticipants::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(chats_, s);
}

void inputPrivacyValueDisallowChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueDisallowChatParticipants");
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueAllowCloseFriends::ID;

void inputPrivacyValueAllowCloseFriends::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowCloseFriends::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowCloseFriends::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueAllowCloseFriends");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueAllowPremium::ID;

void inputPrivacyValueAllowPremium::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowPremium::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueAllowPremium");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueAllowBots::ID;

void inputPrivacyValueAllowBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueAllowBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueAllowBots");
    s.store_class_end();
  }
}

const std::int32_t inputPrivacyValueDisallowBots::ID;

void inputPrivacyValueDisallowBots::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueDisallowBots::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPrivacyValueDisallowBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPrivacyValueDisallowBots");
    s.store_class_end();
  }
}

inputStickerSetItem::inputStickerSetItem(int32 flags_, object_ptr<InputDocument> &&document_, string const &emoji_, object_ptr<maskCoords> &&mask_coords_, string const &keywords_)
  : flags_(flags_)
  , document_(std::move(document_))
  , emoji_(emoji_)
  , mask_coords_(std::move(mask_coords_))
  , keywords_(keywords_)
{}

const std::int32_t inputStickerSetItem::ID;

void inputStickerSetItem::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(document_, s);
  TlStoreString::store(emoji_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1361650766>::store(mask_coords_, s); }
  if (var0 & 2) { TlStoreString::store(keywords_, s); }
}

void inputStickerSetItem::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(document_, s);
  TlStoreString::store(emoji_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1361650766>::store(mask_coords_, s); }
  if (var0 & 2) { TlStoreString::store(keywords_, s); }
}

void inputStickerSetItem::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStickerSetItem");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_field("emoji", emoji_);
    if (var0 & 1) { s.store_object_field("mask_coords", static_cast<const BaseObject *>(mask_coords_.get())); }
    if (var0 & 2) { s.store_field("keywords", keywords_); }
    s.store_class_end();
  }
}

messageRange::messageRange(int32 min_id_, int32 max_id_)
  : min_id_(min_id_)
  , max_id_(max_id_)
{}

const std::int32_t messageRange::ID;

object_ptr<messageRange> messageRange::fetch(TlBufferParser &p) {
  return make_tl_object<messageRange>(p);
}

messageRange::messageRange(TlBufferParser &p)
  : min_id_(TlFetchInt::parse(p))
  , max_id_(TlFetchInt::parse(p))
{}

void messageRange::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(max_id_, s);
}

void messageRange::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(max_id_, s);
}

void messageRange::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageRange");
    s.store_field("min_id", min_id_);
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

object_ptr<NotificationSound> NotificationSound::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case notificationSoundDefault::ID:
      return notificationSoundDefault::fetch(p);
    case notificationSoundNone::ID:
      return notificationSoundNone::fetch(p);
    case notificationSoundLocal::ID:
      return notificationSoundLocal::fetch(p);
    case notificationSoundRingtone::ID:
      return notificationSoundRingtone::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t notificationSoundDefault::ID;

object_ptr<NotificationSound> notificationSoundDefault::fetch(TlBufferParser &p) {
  return make_tl_object<notificationSoundDefault>();
}

void notificationSoundDefault::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void notificationSoundDefault::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void notificationSoundDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSoundDefault");
    s.store_class_end();
  }
}

const std::int32_t notificationSoundNone::ID;

object_ptr<NotificationSound> notificationSoundNone::fetch(TlBufferParser &p) {
  return make_tl_object<notificationSoundNone>();
}

void notificationSoundNone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void notificationSoundNone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void notificationSoundNone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSoundNone");
    s.store_class_end();
  }
}

notificationSoundLocal::notificationSoundLocal(string const &title_, string const &data_)
  : title_(title_)
  , data_(data_)
{}

const std::int32_t notificationSoundLocal::ID;

object_ptr<NotificationSound> notificationSoundLocal::fetch(TlBufferParser &p) {
  return make_tl_object<notificationSoundLocal>(p);
}

notificationSoundLocal::notificationSoundLocal(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , data_(TlFetchString<string>::parse(p))
{}

void notificationSoundLocal::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(title_, s);
  TlStoreString::store(data_, s);
}

void notificationSoundLocal::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(title_, s);
  TlStoreString::store(data_, s);
}

void notificationSoundLocal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSoundLocal");
    s.store_field("title", title_);
    s.store_field("data", data_);
    s.store_class_end();
  }
}

notificationSoundRingtone::notificationSoundRingtone(int64 id_)
  : id_(id_)
{}

const std::int32_t notificationSoundRingtone::ID;

object_ptr<NotificationSound> notificationSoundRingtone::fetch(TlBufferParser &p) {
  return make_tl_object<notificationSoundRingtone>(p);
}

notificationSoundRingtone::notificationSoundRingtone(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void notificationSoundRingtone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void notificationSoundRingtone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
}

void notificationSoundRingtone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSoundRingtone");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

object_ptr<PageListItem> PageListItem::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case pageListItemText::ID:
      return pageListItemText::fetch(p);
    case pageListItemBlocks::ID:
      return pageListItemBlocks::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t pageListItemText::ID;

object_ptr<PageListItem> pageListItemText::fetch(TlBufferParser &p) {
  return make_tl_object<pageListItemText>(p);
}

pageListItemText::pageListItemText(TlBufferParser &p)
  : text_(TlFetchObject<RichText>::parse(p))
{}

void pageListItemText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageListItemText");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

const std::int32_t pageListItemBlocks::ID;

object_ptr<PageListItem> pageListItemBlocks::fetch(TlBufferParser &p) {
  return make_tl_object<pageListItemBlocks>(p);
}

pageListItemBlocks::pageListItemBlocks(TlBufferParser &p)
  : blocks_(TlFetchBoxed<TlFetchVector<TlFetchObject<PageBlock>>, 481674261>::parse(p))
{}

void pageListItemBlocks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageListItemBlocks");
    { s.store_vector_begin("blocks", blocks_.size()); for (const auto &_value : blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pageRelatedArticle::pageRelatedArticle()
  : flags_()
  , url_()
  , webpage_id_()
  , title_()
  , description_()
  , photo_id_()
  , author_()
  , published_date_()
{}

const std::int32_t pageRelatedArticle::ID;

object_ptr<pageRelatedArticle> pageRelatedArticle::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pageRelatedArticle> res = make_tl_object<pageRelatedArticle>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->url_ = TlFetchString<string>::parse(p);
  res->webpage_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->description_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->photo_id_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->author_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->published_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void pageRelatedArticle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageRelatedArticle");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("url", url_);
    s.store_field("webpage_id", webpage_id_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("description", description_); }
    if (var0 & 4) { s.store_field("photo_id", photo_id_); }
    if (var0 & 8) { s.store_field("author", author_); }
    if (var0 & 16) { s.store_field("published_date", published_date_); }
    s.store_class_end();
  }
}

object_ptr<PhoneCallDiscardReason> PhoneCallDiscardReason::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t phoneCallDiscardReasonMissed::ID;

object_ptr<PhoneCallDiscardReason> phoneCallDiscardReasonMissed::fetch(TlBufferParser &p) {
  return make_tl_object<phoneCallDiscardReasonMissed>();
}

void phoneCallDiscardReasonMissed::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonMissed::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonMissed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallDiscardReasonMissed");
    s.store_class_end();
  }
}

const std::int32_t phoneCallDiscardReasonDisconnect::ID;

object_ptr<PhoneCallDiscardReason> phoneCallDiscardReasonDisconnect::fetch(TlBufferParser &p) {
  return make_tl_object<phoneCallDiscardReasonDisconnect>();
}

void phoneCallDiscardReasonDisconnect::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonDisconnect::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonDisconnect::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallDiscardReasonDisconnect");
    s.store_class_end();
  }
}

const std::int32_t phoneCallDiscardReasonHangup::ID;

object_ptr<PhoneCallDiscardReason> phoneCallDiscardReasonHangup::fetch(TlBufferParser &p) {
  return make_tl_object<phoneCallDiscardReasonHangup>();
}

void phoneCallDiscardReasonHangup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonHangup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonHangup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallDiscardReasonHangup");
    s.store_class_end();
  }
}

const std::int32_t phoneCallDiscardReasonBusy::ID;

object_ptr<PhoneCallDiscardReason> phoneCallDiscardReasonBusy::fetch(TlBufferParser &p) {
  return make_tl_object<phoneCallDiscardReasonBusy>();
}

void phoneCallDiscardReasonBusy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonBusy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void phoneCallDiscardReasonBusy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallDiscardReasonBusy");
    s.store_class_end();
  }
}

phoneCallDiscardReasonMigrateConferenceCall::phoneCallDiscardReasonMigrateConferenceCall(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t phoneCallDiscardReasonMigrateConferenceCall::ID;

object_ptr<PhoneCallDiscardReason> phoneCallDiscardReasonMigrateConferenceCall::fetch(TlBufferParser &p) {
  return make_tl_object<phoneCallDiscardReasonMigrateConferenceCall>(p);
}

phoneCallDiscardReasonMigrateConferenceCall::phoneCallDiscardReasonMigrateConferenceCall(TlBufferParser &p)
  : slug_(TlFetchString<string>::parse(p))
{}

void phoneCallDiscardReasonMigrateConferenceCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void phoneCallDiscardReasonMigrateConferenceCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(slug_, s);
}

void phoneCallDiscardReasonMigrateConferenceCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneCallDiscardReasonMigrateConferenceCall");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

object_ptr<Photo> Photo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case photoEmpty::ID:
      return photoEmpty::fetch(p);
    case photo::ID:
      return photo::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t photoEmpty::ID;

object_ptr<Photo> photoEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<photoEmpty>(p);
}

photoEmpty::photoEmpty(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void photoEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoEmpty");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

photo::photo()
  : flags_()
  , has_stickers_()
  , id_()
  , access_hash_()
  , file_reference_()
  , date_()
  , sizes_()
  , video_sizes_()
  , dc_id_()
{}

const std::int32_t photo::ID;

object_ptr<Photo> photo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<photo> res = make_tl_object<photo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_stickers_ = (var0 & 1) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->file_reference_ = TlFetchBytes<bytes>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->sizes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PhotoSize>>, 481674261>::parse(p);
  if (var0 & 2) { res->video_sizes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<VideoSize>>, 481674261>::parse(p); }
  res->dc_id_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void photo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photo");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_stickers_ << 0)));
    if (var0 & 1) { s.store_field("has_stickers", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_bytes_field("file_reference", file_reference_);
    s.store_field("date", date_);
    { s.store_vector_begin("sizes", sizes_.size()); for (const auto &_value : sizes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 2) { { s.store_vector_begin("video_sizes", video_sizes_.size()); for (const auto &_value : video_sizes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_field("dc_id", dc_id_);
    s.store_class_end();
  }
}

postAddress::postAddress(string const &street_line1_, string const &street_line2_, string const &city_, string const &state_, string const &country_iso2_, string const &post_code_)
  : street_line1_(street_line1_)
  , street_line2_(street_line2_)
  , city_(city_)
  , state_(state_)
  , country_iso2_(country_iso2_)
  , post_code_(post_code_)
{}

const std::int32_t postAddress::ID;

object_ptr<postAddress> postAddress::fetch(TlBufferParser &p) {
  return make_tl_object<postAddress>(p);
}

postAddress::postAddress(TlBufferParser &p)
  : street_line1_(TlFetchString<string>::parse(p))
  , street_line2_(TlFetchString<string>::parse(p))
  , city_(TlFetchString<string>::parse(p))
  , state_(TlFetchString<string>::parse(p))
  , country_iso2_(TlFetchString<string>::parse(p))
  , post_code_(TlFetchString<string>::parse(p))
{}

void postAddress::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(street_line1_, s);
  TlStoreString::store(street_line2_, s);
  TlStoreString::store(city_, s);
  TlStoreString::store(state_, s);
  TlStoreString::store(country_iso2_, s);
  TlStoreString::store(post_code_, s);
}

void postAddress::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(street_line1_, s);
  TlStoreString::store(street_line2_, s);
  TlStoreString::store(city_, s);
  TlStoreString::store(state_, s);
  TlStoreString::store(country_iso2_, s);
  TlStoreString::store(post_code_, s);
}

void postAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "postAddress");
    s.store_field("street_line1", street_line1_);
    s.store_field("street_line2", street_line2_);
    s.store_field("city", city_);
    s.store_field("state", state_);
    s.store_field("country_iso2", country_iso2_);
    s.store_field("post_code", post_code_);
    s.store_class_end();
  }
}

object_ptr<PrivacyRule> PrivacyRule::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t privacyValueAllowContacts::ID;

object_ptr<PrivacyRule> privacyValueAllowContacts::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueAllowContacts>();
}

void privacyValueAllowContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueAllowContacts");
    s.store_class_end();
  }
}

const std::int32_t privacyValueAllowAll::ID;

object_ptr<PrivacyRule> privacyValueAllowAll::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueAllowAll>();
}

void privacyValueAllowAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueAllowAll");
    s.store_class_end();
  }
}

const std::int32_t privacyValueAllowUsers::ID;

object_ptr<PrivacyRule> privacyValueAllowUsers::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueAllowUsers>(p);
}

privacyValueAllowUsers::privacyValueAllowUsers(TlBufferParser &p)
  : users_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void privacyValueAllowUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueAllowUsers");
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t privacyValueDisallowContacts::ID;

object_ptr<PrivacyRule> privacyValueDisallowContacts::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueDisallowContacts>();
}

void privacyValueDisallowContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueDisallowContacts");
    s.store_class_end();
  }
}

const std::int32_t privacyValueDisallowAll::ID;

object_ptr<PrivacyRule> privacyValueDisallowAll::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueDisallowAll>();
}

void privacyValueDisallowAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueDisallowAll");
    s.store_class_end();
  }
}

const std::int32_t privacyValueDisallowUsers::ID;

object_ptr<PrivacyRule> privacyValueDisallowUsers::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueDisallowUsers>(p);
}

privacyValueDisallowUsers::privacyValueDisallowUsers(TlBufferParser &p)
  : users_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void privacyValueDisallowUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueDisallowUsers");
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t privacyValueAllowChatParticipants::ID;

object_ptr<PrivacyRule> privacyValueAllowChatParticipants::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueAllowChatParticipants>(p);
}

privacyValueAllowChatParticipants::privacyValueAllowChatParticipants(TlBufferParser &p)
  : chats_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void privacyValueAllowChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueAllowChatParticipants");
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t privacyValueDisallowChatParticipants::ID;

object_ptr<PrivacyRule> privacyValueDisallowChatParticipants::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueDisallowChatParticipants>(p);
}

privacyValueDisallowChatParticipants::privacyValueDisallowChatParticipants(TlBufferParser &p)
  : chats_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void privacyValueDisallowChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueDisallowChatParticipants");
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t privacyValueAllowCloseFriends::ID;

object_ptr<PrivacyRule> privacyValueAllowCloseFriends::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueAllowCloseFriends>();
}

void privacyValueAllowCloseFriends::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueAllowCloseFriends");
    s.store_class_end();
  }
}

const std::int32_t privacyValueAllowPremium::ID;

object_ptr<PrivacyRule> privacyValueAllowPremium::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueAllowPremium>();
}

void privacyValueAllowPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueAllowPremium");
    s.store_class_end();
  }
}

const std::int32_t privacyValueAllowBots::ID;

object_ptr<PrivacyRule> privacyValueAllowBots::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueAllowBots>();
}

void privacyValueAllowBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueAllowBots");
    s.store_class_end();
  }
}

const std::int32_t privacyValueDisallowBots::ID;

object_ptr<PrivacyRule> privacyValueDisallowBots::fetch(TlBufferParser &p) {
  return make_tl_object<privacyValueDisallowBots>();
}

void privacyValueDisallowBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "privacyValueDisallowBots");
    s.store_class_end();
  }
}

reactionsNotifySettings::reactionsNotifySettings()
  : flags_()
  , messages_notify_from_()
  , stories_notify_from_()
  , sound_()
  , show_previews_()
{}

reactionsNotifySettings::reactionsNotifySettings(int32 flags_, object_ptr<ReactionNotificationsFrom> &&messages_notify_from_, object_ptr<ReactionNotificationsFrom> &&stories_notify_from_, object_ptr<NotificationSound> &&sound_, bool show_previews_)
  : flags_(flags_)
  , messages_notify_from_(std::move(messages_notify_from_))
  , stories_notify_from_(std::move(stories_notify_from_))
  , sound_(std::move(sound_))
  , show_previews_(show_previews_)
{}

const std::int32_t reactionsNotifySettings::ID;

object_ptr<reactionsNotifySettings> reactionsNotifySettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<reactionsNotifySettings> res = make_tl_object<reactionsNotifySettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->messages_notify_from_ = TlFetchObject<ReactionNotificationsFrom>::parse(p); }
  if (var0 & 2) { res->stories_notify_from_ = TlFetchObject<ReactionNotificationsFrom>::parse(p); }
  res->sound_ = TlFetchObject<NotificationSound>::parse(p);
  res->show_previews_ = TlFetchBool::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void reactionsNotifySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(messages_notify_from_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(stories_notify_from_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(sound_, s);
  TlStoreBool::store(show_previews_, s);
}

void reactionsNotifySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(messages_notify_from_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(stories_notify_from_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(sound_, s);
  TlStoreBool::store(show_previews_, s);
}

void reactionsNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionsNotifySettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("messages_notify_from", static_cast<const BaseObject *>(messages_notify_from_.get())); }
    if (var0 & 2) { s.store_object_field("stories_notify_from", static_cast<const BaseObject *>(stories_notify_from_.get())); }
    s.store_object_field("sound", static_cast<const BaseObject *>(sound_.get()));
    s.store_field("show_previews", show_previews_);
    s.store_class_end();
  }
}

object_ptr<RequirementToContact> RequirementToContact::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case requirementToContactEmpty::ID:
      return requirementToContactEmpty::fetch(p);
    case requirementToContactPremium::ID:
      return requirementToContactPremium::fetch(p);
    case requirementToContactPaidMessages::ID:
      return requirementToContactPaidMessages::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t requirementToContactEmpty::ID;

object_ptr<RequirementToContact> requirementToContactEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<requirementToContactEmpty>();
}

void requirementToContactEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requirementToContactEmpty");
    s.store_class_end();
  }
}

const std::int32_t requirementToContactPremium::ID;

object_ptr<RequirementToContact> requirementToContactPremium::fetch(TlBufferParser &p) {
  return make_tl_object<requirementToContactPremium>();
}

void requirementToContactPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requirementToContactPremium");
    s.store_class_end();
  }
}

const std::int32_t requirementToContactPaidMessages::ID;

object_ptr<RequirementToContact> requirementToContactPaidMessages::fetch(TlBufferParser &p) {
  return make_tl_object<requirementToContactPaidMessages>(p);
}

requirementToContactPaidMessages::requirementToContactPaidMessages(TlBufferParser &p)
  : stars_amount_(TlFetchLong::parse(p))
{}

void requirementToContactPaidMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requirementToContactPaidMessages");
    s.store_field("stars_amount", stars_amount_);
    s.store_class_end();
  }
}

const std::int32_t restrictionReason::ID;

object_ptr<restrictionReason> restrictionReason::fetch(TlBufferParser &p) {
  return make_tl_object<restrictionReason>(p);
}

restrictionReason::restrictionReason(TlBufferParser &p)
  : platform_(TlFetchString<string>::parse(p))
  , reason_(TlFetchString<string>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void restrictionReason::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "restrictionReason");
    s.store_field("platform", platform_);
    s.store_field("reason", reason_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

object_ptr<SecureValueError> SecureValueError::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

secureValueErrorData::secureValueErrorData(object_ptr<SecureValueType> &&type_, bytes &&data_hash_, string const &field_, string const &text_)
  : type_(std::move(type_))
  , data_hash_(std::move(data_hash_))
  , field_(field_)
  , text_(text_)
{}

const std::int32_t secureValueErrorData::ID;

object_ptr<SecureValueError> secureValueErrorData::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorData>(p);
}

secureValueErrorData::secureValueErrorData(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , data_hash_(TlFetchBytes<bytes>::parse(p))
  , field_(TlFetchString<string>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(data_hash_, s);
  TlStoreString::store(field_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(data_hash_, s);
  TlStoreString::store(field_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorData");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("data_hash", data_hash_);
    s.store_field("field", field_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueErrorFrontSide::secureValueErrorFrontSide(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_)
  : type_(std::move(type_))
  , file_hash_(std::move(file_hash_))
  , text_(text_)
{}

const std::int32_t secureValueErrorFrontSide::ID;

object_ptr<SecureValueError> secureValueErrorFrontSide::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorFrontSide>(p);
}

secureValueErrorFrontSide::secureValueErrorFrontSide(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , file_hash_(TlFetchBytes<bytes>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorFrontSide::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorFrontSide::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorFrontSide::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorFrontSide");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("file_hash", file_hash_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueErrorReverseSide::secureValueErrorReverseSide(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_)
  : type_(std::move(type_))
  , file_hash_(std::move(file_hash_))
  , text_(text_)
{}

const std::int32_t secureValueErrorReverseSide::ID;

object_ptr<SecureValueError> secureValueErrorReverseSide::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorReverseSide>(p);
}

secureValueErrorReverseSide::secureValueErrorReverseSide(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , file_hash_(TlFetchBytes<bytes>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorReverseSide::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorReverseSide::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorReverseSide::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorReverseSide");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("file_hash", file_hash_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueErrorSelfie::secureValueErrorSelfie(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_)
  : type_(std::move(type_))
  , file_hash_(std::move(file_hash_))
  , text_(text_)
{}

const std::int32_t secureValueErrorSelfie::ID;

object_ptr<SecureValueError> secureValueErrorSelfie::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorSelfie>(p);
}

secureValueErrorSelfie::secureValueErrorSelfie(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , file_hash_(TlFetchBytes<bytes>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorSelfie::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorSelfie::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorSelfie::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorSelfie");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("file_hash", file_hash_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueErrorFile::secureValueErrorFile(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_)
  : type_(std::move(type_))
  , file_hash_(std::move(file_hash_))
  , text_(text_)
{}

const std::int32_t secureValueErrorFile::ID;

object_ptr<SecureValueError> secureValueErrorFile::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorFile>(p);
}

secureValueErrorFile::secureValueErrorFile(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , file_hash_(TlFetchBytes<bytes>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorFile");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("file_hash", file_hash_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueErrorFiles::secureValueErrorFiles(object_ptr<SecureValueType> &&type_, array<bytes> &&file_hash_, string const &text_)
  : type_(std::move(type_))
  , file_hash_(std::move(file_hash_))
  , text_(text_)
{}

const std::int32_t secureValueErrorFiles::ID;

object_ptr<SecureValueError> secureValueErrorFiles::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorFiles>(p);
}

secureValueErrorFiles::secureValueErrorFiles(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , file_hash_(TlFetchBoxed<TlFetchVector<TlFetchBytes<bytes>>, 481674261>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorFiles::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorFiles::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorFiles");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    { s.store_vector_begin("file_hash", file_hash_.size()); for (const auto &_value : file_hash_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueError::secureValueError(object_ptr<SecureValueType> &&type_, bytes &&hash_, string const &text_)
  : type_(std::move(type_))
  , hash_(std::move(hash_))
  , text_(text_)
{}

const std::int32_t secureValueError::ID;

object_ptr<SecureValueError> secureValueError::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueError>(p);
}

secureValueError::secureValueError(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , hash_(TlFetchBytes<bytes>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueError::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueError::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueError");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("hash", hash_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueErrorTranslationFile::secureValueErrorTranslationFile(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_)
  : type_(std::move(type_))
  , file_hash_(std::move(file_hash_))
  , text_(text_)
{}

const std::int32_t secureValueErrorTranslationFile::ID;

object_ptr<SecureValueError> secureValueErrorTranslationFile::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorTranslationFile>(p);
}

secureValueErrorTranslationFile::secureValueErrorTranslationFile(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , file_hash_(TlFetchBytes<bytes>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorTranslationFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorTranslationFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreString::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorTranslationFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorTranslationFile");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("file_hash", file_hash_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

secureValueErrorTranslationFiles::secureValueErrorTranslationFiles(object_ptr<SecureValueType> &&type_, array<bytes> &&file_hash_, string const &text_)
  : type_(std::move(type_))
  , file_hash_(std::move(file_hash_))
  , text_(text_)
{}

const std::int32_t secureValueErrorTranslationFiles::ID;

object_ptr<SecureValueError> secureValueErrorTranslationFiles::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueErrorTranslationFiles>(p);
}

secureValueErrorTranslationFiles::secureValueErrorTranslationFiles(TlBufferParser &p)
  : type_(TlFetchObject<SecureValueType>::parse(p))
  , file_hash_(TlFetchBoxed<TlFetchVector<TlFetchBytes<bytes>>, 481674261>::parse(p))
  , text_(TlFetchString<string>::parse(p))
{}

void secureValueErrorTranslationFiles::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorTranslationFiles::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(file_hash_, s);
  TlStoreString::store(text_, s);
}

void secureValueErrorTranslationFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueErrorTranslationFiles");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    { s.store_vector_begin("file_hash", file_hash_.size()); for (const auto &_value : file_hash_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_field("text", text_);
    s.store_class_end();
  }
}

object_ptr<SecureValueType> SecureValueType::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t secureValueTypePersonalDetails::ID;

object_ptr<SecureValueType> secureValueTypePersonalDetails::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypePersonalDetails>();
}

void secureValueTypePersonalDetails::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypePersonalDetails::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypePersonalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypePersonalDetails");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypePassport::ID;

object_ptr<SecureValueType> secureValueTypePassport::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypePassport>();
}

void secureValueTypePassport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypePassport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypePassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypePassport");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeDriverLicense::ID;

object_ptr<SecureValueType> secureValueTypeDriverLicense::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeDriverLicense>();
}

void secureValueTypeDriverLicense::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeDriverLicense::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeDriverLicense::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeDriverLicense");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeIdentityCard::ID;

object_ptr<SecureValueType> secureValueTypeIdentityCard::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeIdentityCard>();
}

void secureValueTypeIdentityCard::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeIdentityCard::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeIdentityCard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeIdentityCard");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeInternalPassport::ID;

object_ptr<SecureValueType> secureValueTypeInternalPassport::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeInternalPassport>();
}

void secureValueTypeInternalPassport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeInternalPassport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeInternalPassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeInternalPassport");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeAddress::ID;

object_ptr<SecureValueType> secureValueTypeAddress::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeAddress>();
}

void secureValueTypeAddress::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeAddress::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeAddress");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeUtilityBill::ID;

object_ptr<SecureValueType> secureValueTypeUtilityBill::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeUtilityBill>();
}

void secureValueTypeUtilityBill::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeUtilityBill::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeUtilityBill::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeUtilityBill");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeBankStatement::ID;

object_ptr<SecureValueType> secureValueTypeBankStatement::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeBankStatement>();
}

void secureValueTypeBankStatement::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeBankStatement::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeBankStatement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeBankStatement");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeRentalAgreement::ID;

object_ptr<SecureValueType> secureValueTypeRentalAgreement::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeRentalAgreement>();
}

void secureValueTypeRentalAgreement::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeRentalAgreement::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeRentalAgreement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeRentalAgreement");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypePassportRegistration::ID;

object_ptr<SecureValueType> secureValueTypePassportRegistration::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypePassportRegistration>();
}

void secureValueTypePassportRegistration::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypePassportRegistration::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypePassportRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypePassportRegistration");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeTemporaryRegistration::ID;

object_ptr<SecureValueType> secureValueTypeTemporaryRegistration::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeTemporaryRegistration>();
}

void secureValueTypeTemporaryRegistration::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeTemporaryRegistration::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeTemporaryRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeTemporaryRegistration");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypePhone::ID;

object_ptr<SecureValueType> secureValueTypePhone::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypePhone>();
}

void secureValueTypePhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypePhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypePhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypePhone");
    s.store_class_end();
  }
}

const std::int32_t secureValueTypeEmail::ID;

object_ptr<SecureValueType> secureValueTypeEmail::fetch(TlBufferParser &p) {
  return make_tl_object<secureValueTypeEmail>();
}

void secureValueTypeEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void secureValueTypeEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void secureValueTypeEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secureValueTypeEmail");
    s.store_class_end();
  }
}

starGiftCollection::starGiftCollection()
  : flags_()
  , collection_id_()
  , title_()
  , icon_()
  , gifts_count_()
  , hash_()
{}

const std::int32_t starGiftCollection::ID;

object_ptr<starGiftCollection> starGiftCollection::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starGiftCollection> res = make_tl_object<starGiftCollection>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->collection_id_ = TlFetchInt::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->icon_ = TlFetchObject<Document>::parse(p); }
  res->gifts_count_ = TlFetchInt::parse(p);
  res->hash_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftCollection");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("collection_id", collection_id_);
    s.store_field("title", title_);
    if (var0 & 1) { s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get())); }
    s.store_field("gifts_count", gifts_count_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

starsRating::starsRating()
  : flags_()
  , level_()
  , current_level_stars_()
  , stars_()
  , next_level_stars_()
{}

const std::int32_t starsRating::ID;

object_ptr<starsRating> starsRating::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsRating> res = make_tl_object<starsRating>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->level_ = TlFetchInt::parse(p);
  res->current_level_stars_ = TlFetchLong::parse(p);
  res->stars_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->next_level_stars_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starsRating::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsRating");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("level", level_);
    s.store_field("current_level_stars", current_level_stars_);
    s.store_field("stars", stars_);
    if (var0 & 1) { s.store_field("next_level_stars", next_level_stars_); }
    s.store_class_end();
  }
}

starsSubscription::starsSubscription()
  : flags_()
  , canceled_()
  , can_refulfill_()
  , missing_balance_()
  , bot_canceled_()
  , id_()
  , peer_()
  , until_date_()
  , pricing_()
  , chat_invite_hash_()
  , title_()
  , photo_()
  , invoice_slug_()
{}

const std::int32_t starsSubscription::ID;

object_ptr<starsSubscription> starsSubscription::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsSubscription> res = make_tl_object<starsSubscription>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->canceled_ = (var0 & 1) != 0;
  res->can_refulfill_ = (var0 & 2) != 0;
  res->missing_balance_ = (var0 & 4) != 0;
  res->bot_canceled_ = (var0 & 128) != 0;
  res->id_ = TlFetchString<string>::parse(p);
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->until_date_ = TlFetchInt::parse(p);
  res->pricing_ = TlFetchBoxed<TlFetchObject<starsSubscriptionPricing>, 88173912>::parse(p);
  if (var0 & 8) { res->chat_invite_hash_ = TlFetchString<string>::parse(p); }
  if (var0 & 16) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 32) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  if (var0 & 64) { res->invoice_slug_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void starsSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsSubscription");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (canceled_ << 0) | (can_refulfill_ << 1) | (missing_balance_ << 2) | (bot_canceled_ << 7)));
    if (var0 & 1) { s.store_field("canceled", true); }
    if (var0 & 2) { s.store_field("can_refulfill", true); }
    if (var0 & 4) { s.store_field("missing_balance", true); }
    if (var0 & 128) { s.store_field("bot_canceled", true); }
    s.store_field("id", id_);
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("until_date", until_date_);
    s.store_object_field("pricing", static_cast<const BaseObject *>(pricing_.get()));
    if (var0 & 8) { s.store_field("chat_invite_hash", chat_invite_hash_); }
    if (var0 & 16) { s.store_field("title", title_); }
    if (var0 & 32) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    if (var0 & 64) { s.store_field("invoice_slug", invoice_slug_); }
    s.store_class_end();
  }
}

const std::int32_t statsAbsValueAndPrev::ID;

object_ptr<statsAbsValueAndPrev> statsAbsValueAndPrev::fetch(TlBufferParser &p) {
  return make_tl_object<statsAbsValueAndPrev>(p);
}

statsAbsValueAndPrev::statsAbsValueAndPrev(TlBufferParser &p)
  : current_(TlFetchDouble::parse(p))
  , previous_(TlFetchDouble::parse(p))
{}

void statsAbsValueAndPrev::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsAbsValueAndPrev");
    s.store_field("current", current_);
    s.store_field("previous", previous_);
    s.store_class_end();
  }
}

const std::int32_t statsGroupTopInviter::ID;

object_ptr<statsGroupTopInviter> statsGroupTopInviter::fetch(TlBufferParser &p) {
  return make_tl_object<statsGroupTopInviter>(p);
}

statsGroupTopInviter::statsGroupTopInviter(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , invitations_(TlFetchInt::parse(p))
{}

void statsGroupTopInviter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsGroupTopInviter");
    s.store_field("user_id", user_id_);
    s.store_field("invitations", invitations_);
    s.store_class_end();
  }
}

const std::int32_t statsGroupTopPoster::ID;

object_ptr<statsGroupTopPoster> statsGroupTopPoster::fetch(TlBufferParser &p) {
  return make_tl_object<statsGroupTopPoster>(p);
}

statsGroupTopPoster::statsGroupTopPoster(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , messages_(TlFetchInt::parse(p))
  , avg_chars_(TlFetchInt::parse(p))
{}

void statsGroupTopPoster::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsGroupTopPoster");
    s.store_field("user_id", user_id_);
    s.store_field("messages", messages_);
    s.store_field("avg_chars", avg_chars_);
    s.store_class_end();
  }
}

storyAlbum::storyAlbum()
  : flags_()
  , album_id_()
  , title_()
  , icon_photo_()
  , icon_video_()
{}

const std::int32_t storyAlbum::ID;

object_ptr<storyAlbum> storyAlbum::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyAlbum> res = make_tl_object<storyAlbum>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->album_id_ = TlFetchInt::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  if (var0 & 1) { res->icon_photo_ = TlFetchObject<Photo>::parse(p); }
  if (var0 & 2) { res->icon_video_ = TlFetchObject<Document>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void storyAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAlbum");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("album_id", album_id_);
    s.store_field("title", title_);
    if (var0 & 1) { s.store_object_field("icon_photo", static_cast<const BaseObject *>(icon_photo_.get())); }
    if (var0 & 2) { s.store_object_field("icon_video", static_cast<const BaseObject *>(icon_video_.get())); }
    s.store_class_end();
  }
}

object_ptr<StoryReaction> StoryReaction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case storyReaction::ID:
      return storyReaction::fetch(p);
    case storyReactionPublicForward::ID:
      return storyReactionPublicForward::fetch(p);
    case storyReactionPublicRepost::ID:
      return storyReactionPublicRepost::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t storyReaction::ID;

object_ptr<StoryReaction> storyReaction::fetch(TlBufferParser &p) {
  return make_tl_object<storyReaction>(p);
}

storyReaction::storyReaction(TlBufferParser &p)
  : peer_id_(TlFetchObject<Peer>::parse(p))
  , date_(TlFetchInt::parse(p))
  , reaction_(TlFetchObject<Reaction>::parse(p))
{}

void storyReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyReaction");
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    s.store_field("date", date_);
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_class_end();
  }
}

const std::int32_t storyReactionPublicForward::ID;

object_ptr<StoryReaction> storyReactionPublicForward::fetch(TlBufferParser &p) {
  return make_tl_object<storyReactionPublicForward>(p);
}

storyReactionPublicForward::storyReactionPublicForward(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void storyReactionPublicForward::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyReactionPublicForward");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t storyReactionPublicRepost::ID;

object_ptr<StoryReaction> storyReactionPublicRepost::fetch(TlBufferParser &p) {
  return make_tl_object<storyReactionPublicRepost>(p);
}

storyReactionPublicRepost::storyReactionPublicRepost(TlBufferParser &p)
  : peer_id_(TlFetchObject<Peer>::parse(p))
  , story_(TlFetchObject<StoryItem>::parse(p))
{}

void storyReactionPublicRepost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyReactionPublicRepost");
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

textWithEntities::textWithEntities(string const &text_, array<object_ptr<MessageEntity>> &&entities_)
  : text_(text_)
  , entities_(std::move(entities_))
{}

const std::int32_t textWithEntities::ID;

object_ptr<textWithEntities> textWithEntities::fetch(TlBufferParser &p) {
  return make_tl_object<textWithEntities>(p);
}

textWithEntities::textWithEntities(TlBufferParser &p)
  : text_(TlFetchString<string>::parse(p))
  , entities_(TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p))
{}

void textWithEntities::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s);
}

void textWithEntities::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(text_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s);
}

void textWithEntities::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textWithEntities");
    s.store_field("text", text_);
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t todoCompletion::ID;

object_ptr<todoCompletion> todoCompletion::fetch(TlBufferParser &p) {
  return make_tl_object<todoCompletion>(p);
}

todoCompletion::todoCompletion(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
  , completed_by_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void todoCompletion::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "todoCompletion");
    s.store_field("id", id_);
    s.store_field("completed_by", completed_by_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

object_ptr<Updates> Updates::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t updatesTooLong::ID;

object_ptr<Updates> updatesTooLong::fetch(TlBufferParser &p) {
  return make_tl_object<updatesTooLong>();
}

void updatesTooLong::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatesTooLong");
    s.store_class_end();
  }
}

updateShortMessage::updateShortMessage()
  : flags_()
  , out_()
  , mentioned_()
  , media_unread_()
  , silent_()
  , id_()
  , user_id_()
  , message_()
  , pts_()
  , pts_count_()
  , date_()
  , fwd_from_()
  , via_bot_id_()
  , reply_to_()
  , entities_()
  , ttl_period_()
{}

const std::int32_t updateShortMessage::ID;

object_ptr<Updates> updateShortMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateShortMessage> res = make_tl_object<updateShortMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->out_ = (var0 & 2) != 0;
  res->mentioned_ = (var0 & 16) != 0;
  res->media_unread_ = (var0 & 32) != 0;
  res->silent_ = (var0 & 8192) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->user_id_ = TlFetchLong::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  res->pts_count_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->fwd_from_ = TlFetchBoxed<TlFetchObject<messageFwdHeader>, 1313731771>::parse(p); }
  if (var0 & 2048) { res->via_bot_id_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->reply_to_ = TlFetchObject<MessageReplyHeader>::parse(p); }
  if (var0 & 128) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 33554432) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateShortMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateShortMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (out_ << 1) | (mentioned_ << 4) | (media_unread_ << 5) | (silent_ << 13)));
    if (var0 & 2) { s.store_field("out", true); }
    if (var0 & 16) { s.store_field("mentioned", true); }
    if (var0 & 32) { s.store_field("media_unread", true); }
    if (var0 & 8192) { s.store_field("silent", true); }
    s.store_field("id", id_);
    s.store_field("user_id", user_id_);
    s.store_field("message", message_);
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_field("date", date_);
    if (var0 & 4) { s.store_object_field("fwd_from", static_cast<const BaseObject *>(fwd_from_.get())); }
    if (var0 & 2048) { s.store_field("via_bot_id", via_bot_id_); }
    if (var0 & 8) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    if (var0 & 128) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 33554432) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

updateShortChatMessage::updateShortChatMessage()
  : flags_()
  , out_()
  , mentioned_()
  , media_unread_()
  , silent_()
  , id_()
  , from_id_()
  , chat_id_()
  , message_()
  , pts_()
  , pts_count_()
  , date_()
  , fwd_from_()
  , via_bot_id_()
  , reply_to_()
  , entities_()
  , ttl_period_()
{}

const std::int32_t updateShortChatMessage::ID;

object_ptr<Updates> updateShortChatMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateShortChatMessage> res = make_tl_object<updateShortChatMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->out_ = (var0 & 2) != 0;
  res->mentioned_ = (var0 & 16) != 0;
  res->media_unread_ = (var0 & 32) != 0;
  res->silent_ = (var0 & 8192) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->from_id_ = TlFetchLong::parse(p);
  res->chat_id_ = TlFetchLong::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  res->pts_count_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->fwd_from_ = TlFetchBoxed<TlFetchObject<messageFwdHeader>, 1313731771>::parse(p); }
  if (var0 & 2048) { res->via_bot_id_ = TlFetchLong::parse(p); }
  if (var0 & 8) { res->reply_to_ = TlFetchObject<MessageReplyHeader>::parse(p); }
  if (var0 & 128) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 33554432) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateShortChatMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateShortChatMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (out_ << 1) | (mentioned_ << 4) | (media_unread_ << 5) | (silent_ << 13)));
    if (var0 & 2) { s.store_field("out", true); }
    if (var0 & 16) { s.store_field("mentioned", true); }
    if (var0 & 32) { s.store_field("media_unread", true); }
    if (var0 & 8192) { s.store_field("silent", true); }
    s.store_field("id", id_);
    s.store_field("from_id", from_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message", message_);
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_field("date", date_);
    if (var0 & 4) { s.store_object_field("fwd_from", static_cast<const BaseObject *>(fwd_from_.get())); }
    if (var0 & 2048) { s.store_field("via_bot_id", via_bot_id_); }
    if (var0 & 8) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    if (var0 & 128) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 33554432) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

const std::int32_t updateShort::ID;

object_ptr<Updates> updateShort::fetch(TlBufferParser &p) {
  return make_tl_object<updateShort>(p);
}

updateShort::updateShort(TlBufferParser &p)
  : update_(TlFetchObject<Update>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void updateShort::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateShort");
    s.store_object_field("update", static_cast<const BaseObject *>(update_.get()));
    s.store_field("date", date_);
    s.store_class_end();
  }
}

const std::int32_t updatesCombined::ID;

object_ptr<Updates> updatesCombined::fetch(TlBufferParser &p) {
  return make_tl_object<updatesCombined>(p);
}

updatesCombined::updatesCombined(TlBufferParser &p)
  : updates_(TlFetchBoxed<TlFetchVector<TlFetchObject<Update>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , date_(TlFetchInt::parse(p))
  , seq_start_(TlFetchInt::parse(p))
  , seq_(TlFetchInt::parse(p))
{}

void updatesCombined::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatesCombined");
    { s.store_vector_begin("updates", updates_.size()); for (const auto &_value : updates_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("date", date_);
    s.store_field("seq_start", seq_start_);
    s.store_field("seq", seq_);
    s.store_class_end();
  }
}

updates::updates()
  : updates_()
  , users_()
  , chats_()
  , date_()
  , seq_()
{}

const std::int32_t updates::ID;

object_ptr<Updates> updates::fetch(TlBufferParser &p) {
  return make_tl_object<updates>(p);
}

updates::updates(TlBufferParser &p)
  : updates_(TlFetchBoxed<TlFetchVector<TlFetchObject<Update>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , date_(TlFetchInt::parse(p))
  , seq_(TlFetchInt::parse(p))
{}

void updates::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates");
    { s.store_vector_begin("updates", updates_.size()); for (const auto &_value : updates_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("date", date_);
    s.store_field("seq", seq_);
    s.store_class_end();
  }
}

updateShortSentMessage::updateShortSentMessage()
  : flags_()
  , out_()
  , id_()
  , pts_()
  , pts_count_()
  , date_()
  , media_()
  , entities_()
  , ttl_period_()
{}

const std::int32_t updateShortSentMessage::ID;

object_ptr<Updates> updateShortSentMessage::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<updateShortSentMessage> res = make_tl_object<updateShortSentMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->out_ = (var0 & 2) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->pts_ = TlFetchInt::parse(p);
  res->pts_count_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 512) { res->media_ = TlFetchObject<MessageMedia>::parse(p); }
  if (var0 & 128) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 33554432) { res->ttl_period_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void updateShortSentMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateShortSentMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (out_ << 1)));
    if (var0 & 2) { s.store_field("out", true); }
    s.store_field("id", id_);
    s.store_field("pts", pts_);
    s.store_field("pts_count", pts_count_);
    s.store_field("date", date_);
    if (var0 & 512) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 128) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 33554432) { s.store_field("ttl_period", ttl_period_); }
    s.store_class_end();
  }
}

object_ptr<UrlAuthResult> UrlAuthResult::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case urlAuthResultRequest::ID:
      return urlAuthResultRequest::fetch(p);
    case urlAuthResultAccepted::ID:
      return urlAuthResultAccepted::fetch(p);
    case urlAuthResultDefault::ID:
      return urlAuthResultDefault::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

urlAuthResultRequest::urlAuthResultRequest()
  : flags_()
  , request_write_access_()
  , bot_()
  , domain_()
{}

const std::int32_t urlAuthResultRequest::ID;

object_ptr<UrlAuthResult> urlAuthResultRequest::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<urlAuthResultRequest> res = make_tl_object<urlAuthResultRequest>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->request_write_access_ = (var0 & 1) != 0;
  res->bot_ = TlFetchObject<User>::parse(p);
  res->domain_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void urlAuthResultRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "urlAuthResultRequest");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (request_write_access_ << 0)));
    if (var0 & 1) { s.store_field("request_write_access", true); }
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("domain", domain_);
    s.store_class_end();
  }
}

const std::int32_t urlAuthResultAccepted::ID;

object_ptr<UrlAuthResult> urlAuthResultAccepted::fetch(TlBufferParser &p) {
  return make_tl_object<urlAuthResultAccepted>(p);
}

urlAuthResultAccepted::urlAuthResultAccepted(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void urlAuthResultAccepted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "urlAuthResultAccepted");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

const std::int32_t urlAuthResultDefault::ID;

object_ptr<UrlAuthResult> urlAuthResultDefault::fetch(TlBufferParser &p) {
  return make_tl_object<urlAuthResultDefault>();
}

void urlAuthResultDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "urlAuthResultDefault");
    s.store_class_end();
  }
}

object_ptr<UserProfilePhoto> UserProfilePhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case userProfilePhotoEmpty::ID:
      return userProfilePhotoEmpty::fetch(p);
    case userProfilePhoto::ID:
      return userProfilePhoto::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t userProfilePhotoEmpty::ID;

object_ptr<UserProfilePhoto> userProfilePhotoEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<userProfilePhotoEmpty>();
}

void userProfilePhotoEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userProfilePhotoEmpty");
    s.store_class_end();
  }
}

userProfilePhoto::userProfilePhoto()
  : flags_()
  , has_video_()
  , personal_()
  , photo_id_()
  , stripped_thumb_()
  , dc_id_()
{}

userProfilePhoto::userProfilePhoto(int32 flags_, bool has_video_, bool personal_, int64 photo_id_, bytes &&stripped_thumb_, int32 dc_id_)
  : flags_(flags_)
  , has_video_(has_video_)
  , personal_(personal_)
  , photo_id_(photo_id_)
  , stripped_thumb_(std::move(stripped_thumb_))
  , dc_id_(dc_id_)
{}

const std::int32_t userProfilePhoto::ID;

object_ptr<UserProfilePhoto> userProfilePhoto::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<userProfilePhoto> res = make_tl_object<userProfilePhoto>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_video_ = (var0 & 1) != 0;
  res->personal_ = (var0 & 4) != 0;
  res->photo_id_ = TlFetchLong::parse(p);
  if (var0 & 2) { res->stripped_thumb_ = TlFetchBytes<bytes>::parse(p); }
  res->dc_id_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void userProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userProfilePhoto");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_video_ << 0) | (personal_ << 2)));
    if (var0 & 1) { s.store_field("has_video", true); }
    if (var0 & 4) { s.store_field("personal", true); }
    s.store_field("photo_id", photo_id_);
    if (var0 & 2) { s.store_bytes_field("stripped_thumb", stripped_thumb_); }
    s.store_field("dc_id", dc_id_);
    s.store_class_end();
  }
}

const std::int32_t account_autoDownloadSettings::ID;

object_ptr<account_autoDownloadSettings> account_autoDownloadSettings::fetch(TlBufferParser &p) {
  return make_tl_object<account_autoDownloadSettings>(p);
}

account_autoDownloadSettings::account_autoDownloadSettings(TlBufferParser &p)
  : low_(TlFetchBoxed<TlFetchObject<autoDownloadSettings>, -1163561432>::parse(p))
  , medium_(TlFetchBoxed<TlFetchObject<autoDownloadSettings>, -1163561432>::parse(p))
  , high_(TlFetchBoxed<TlFetchObject<autoDownloadSettings>, -1163561432>::parse(p))
{}

void account_autoDownloadSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.autoDownloadSettings");
    s.store_object_field("low", static_cast<const BaseObject *>(low_.get()));
    s.store_object_field("medium", static_cast<const BaseObject *>(medium_.get()));
    s.store_object_field("high", static_cast<const BaseObject *>(high_.get()));
    s.store_class_end();
  }
}

object_ptr<auth_Authorization> auth_Authorization::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case auth_authorization::ID:
      return auth_authorization::fetch(p);
    case auth_authorizationSignUpRequired::ID:
      return auth_authorizationSignUpRequired::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

auth_authorization::auth_authorization()
  : flags_()
  , setup_password_required_()
  , otherwise_relogin_days_()
  , tmp_sessions_()
  , future_auth_token_()
  , user_()
{}

const std::int32_t auth_authorization::ID;

object_ptr<auth_Authorization> auth_authorization::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_authorization> res = make_tl_object<auth_authorization>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->setup_password_required_ = (var0 & 2) != 0;
  if (var0 & 2) { res->otherwise_relogin_days_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->tmp_sessions_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->future_auth_token_ = TlFetchBytes<bytes>::parse(p); }
  res->user_ = TlFetchObject<User>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_authorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.authorization");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (setup_password_required_ << 1)));
    if (var0 & 2) { s.store_field("setup_password_required", true); }
    if (var0 & 2) { s.store_field("otherwise_relogin_days", otherwise_relogin_days_); }
    if (var0 & 1) { s.store_field("tmp_sessions", tmp_sessions_); }
    if (var0 & 4) { s.store_bytes_field("future_auth_token", future_auth_token_); }
    s.store_object_field("user", static_cast<const BaseObject *>(user_.get()));
    s.store_class_end();
  }
}

auth_authorizationSignUpRequired::auth_authorizationSignUpRequired()
  : flags_()
  , terms_of_service_()
{}

const std::int32_t auth_authorizationSignUpRequired::ID;

object_ptr<auth_Authorization> auth_authorizationSignUpRequired::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_authorizationSignUpRequired> res = make_tl_object<auth_authorizationSignUpRequired>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->terms_of_service_ = TlFetchBoxed<TlFetchObject<help_termsOfService>, 2013922064>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_authorizationSignUpRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.authorizationSignUpRequired");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("terms_of_service", static_cast<const BaseObject *>(terms_of_service_.get())); }
    s.store_class_end();
  }
}

help_country::help_country()
  : flags_()
  , hidden_()
  , iso2_()
  , default_name_()
  , name_()
  , country_codes_()
{}

const std::int32_t help_country::ID;

object_ptr<help_country> help_country::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<help_country> res = make_tl_object<help_country>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->hidden_ = (var0 & 1) != 0;
  res->iso2_ = TlFetchString<string>::parse(p);
  res->default_name_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->name_ = TlFetchString<string>::parse(p); }
  res->country_codes_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<help_countryCode>, 1107543535>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void help_country::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.country");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (hidden_ << 0)));
    if (var0 & 1) { s.store_field("hidden", true); }
    s.store_field("iso2", iso2_);
    s.store_field("default_name", default_name_);
    if (var0 & 2) { s.store_field("name", name_); }
    { s.store_vector_begin("country_codes", country_codes_.size()); for (const auto &_value : country_codes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_dialogFilters::messages_dialogFilters()
  : flags_()
  , tags_enabled_()
  , filters_()
{}

const std::int32_t messages_dialogFilters::ID;

object_ptr<messages_dialogFilters> messages_dialogFilters::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_dialogFilters> res = make_tl_object<messages_dialogFilters>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->tags_enabled_ = (var0 & 1) != 0;
  res->filters_ = TlFetchBoxed<TlFetchVector<TlFetchObject<DialogFilter>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_dialogFilters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.dialogFilters");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (tags_enabled_ << 0)));
    if (var0 & 1) { s.store_field("tags_enabled", true); }
    { s.store_vector_begin("filters", filters_.size()); for (const auto &_value : filters_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_SponsoredMessages> messages_SponsoredMessages::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_sponsoredMessages::ID:
      return messages_sponsoredMessages::fetch(p);
    case messages_sponsoredMessagesEmpty::ID:
      return messages_sponsoredMessagesEmpty::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

messages_sponsoredMessages::messages_sponsoredMessages()
  : flags_()
  , posts_between_()
  , start_delay_()
  , between_delay_()
  , messages_()
  , chats_()
  , users_()
{}

const std::int32_t messages_sponsoredMessages::ID;

object_ptr<messages_SponsoredMessages> messages_sponsoredMessages::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_sponsoredMessages> res = make_tl_object<messages_sponsoredMessages>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->posts_between_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->start_delay_ = TlFetchInt::parse(p); }
  if (var0 & 4) { res->between_delay_ = TlFetchInt::parse(p); }
  res->messages_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<sponsoredMessage>, 2109703795>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messages_sponsoredMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sponsoredMessages");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("posts_between", posts_between_); }
    if (var0 & 2) { s.store_field("start_delay", start_delay_); }
    if (var0 & 4) { s.store_field("between_delay", between_delay_); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_sponsoredMessagesEmpty::ID;

object_ptr<messages_SponsoredMessages> messages_sponsoredMessagesEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<messages_sponsoredMessagesEmpty>();
}

void messages_sponsoredMessagesEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sponsoredMessagesEmpty");
    s.store_class_end();
  }
}

object_ptr<messages_StickerSet> messages_StickerSet::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_stickerSet::ID:
      return messages_stickerSet::fetch(p);
    case messages_stickerSetNotModified::ID:
      return messages_stickerSetNotModified::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

messages_stickerSet::messages_stickerSet(object_ptr<stickerSet> &&set_, array<object_ptr<stickerPack>> &&packs_, array<object_ptr<stickerKeyword>> &&keywords_, array<object_ptr<Document>> &&documents_)
  : set_(std::move(set_))
  , packs_(std::move(packs_))
  , keywords_(std::move(keywords_))
  , documents_(std::move(documents_))
{}

const std::int32_t messages_stickerSet::ID;

object_ptr<messages_StickerSet> messages_stickerSet::fetch(TlBufferParser &p) {
  return make_tl_object<messages_stickerSet>(p);
}

messages_stickerSet::messages_stickerSet(TlBufferParser &p)
  : set_(TlFetchBoxed<TlFetchObject<stickerSet>, 768691932>::parse(p))
  , packs_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<stickerPack>, 313694676>>, 481674261>::parse(p))
  , keywords_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<stickerKeyword>, -50416996>>, 481674261>::parse(p))
  , documents_(TlFetchBoxed<TlFetchVector<TlFetchObject<Document>>, 481674261>::parse(p))
{}

void messages_stickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.stickerSet");
    s.store_object_field("set", static_cast<const BaseObject *>(set_.get()));
    { s.store_vector_begin("packs", packs_.size()); for (const auto &_value : packs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("keywords", keywords_.size()); for (const auto &_value : keywords_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_stickerSetNotModified::ID;

object_ptr<messages_StickerSet> messages_stickerSetNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_stickerSetNotModified>();
}

void messages_stickerSetNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.stickerSetNotModified");
    s.store_class_end();
  }
}

const std::int32_t messages_translateResult::ID;

object_ptr<messages_translateResult> messages_translateResult::fetch(TlBufferParser &p) {
  return make_tl_object<messages_translateResult>(p);
}

messages_translateResult::messages_translateResult(TlBufferParser &p)
  : result_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>>, 481674261>::parse(p))
{}

void messages_translateResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.translateResult");
    { s.store_vector_begin("result", result_.size()); for (const auto &_value : result_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_votesList::messages_votesList()
  : flags_()
  , count_()
  , votes_()
  , chats_()
  , users_()
  , next_offset_()
{}

const std::int32_t messages_votesList::ID;

object_ptr<messages_votesList> messages_votesList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_votesList> res = make_tl_object<messages_votesList>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->count_ = TlFetchInt::parse(p);
  res->votes_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessagePeerVote>>, 481674261>::parse(p);
  res->chats_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (var0 & 1) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_votesList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.votesList");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("count", count_);
    { s.store_vector_begin("votes", votes_.size()); for (const auto &_value : votes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1) { s.store_field("next_offset", next_offset_); }
    s.store_class_end();
  }
}

const std::int32_t messages_webPagePreview::ID;

object_ptr<messages_webPagePreview> messages_webPagePreview::fetch(TlBufferParser &p) {
  return make_tl_object<messages_webPagePreview>(p);
}

messages_webPagePreview::messages_webPagePreview(TlBufferParser &p)
  : media_(TlFetchObject<MessageMedia>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_webPagePreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.webPagePreview");
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<payments_PaymentReceipt> payments_PaymentReceipt::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case payments_paymentReceipt::ID:
      return payments_paymentReceipt::fetch(p);
    case payments_paymentReceiptStars::ID:
      return payments_paymentReceiptStars::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

payments_paymentReceipt::payments_paymentReceipt()
  : flags_()
  , date_()
  , bot_id_()
  , provider_id_()
  , title_()
  , description_()
  , photo_()
  , invoice_()
  , info_()
  , shipping_()
  , tip_amount_()
  , currency_()
  , total_amount_()
  , credentials_title_()
  , users_()
{}

const std::int32_t payments_paymentReceipt::ID;

object_ptr<payments_PaymentReceipt> payments_paymentReceipt::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_paymentReceipt> res = make_tl_object<payments_paymentReceipt>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->date_ = TlFetchInt::parse(p);
  res->bot_id_ = TlFetchLong::parse(p);
  res->provider_id_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 4) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  res->invoice_ = TlFetchBoxed<TlFetchObject<invoice>, 77522308>::parse(p);
  if (var0 & 1) { res->info_ = TlFetchBoxed<TlFetchObject<paymentRequestedInfo>, -1868808300>::parse(p); }
  if (var0 & 2) { res->shipping_ = TlFetchBoxed<TlFetchObject<shippingOption>, -1239335713>::parse(p); }
  if (var0 & 8) { res->tip_amount_ = TlFetchLong::parse(p); }
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  res->credentials_title_ = TlFetchString<string>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void payments_paymentReceipt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.paymentReceipt");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("date", date_);
    s.store_field("bot_id", bot_id_);
    s.store_field("provider_id", provider_id_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 4) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    if (var0 & 1) { s.store_object_field("info", static_cast<const BaseObject *>(info_.get())); }
    if (var0 & 2) { s.store_object_field("shipping", static_cast<const BaseObject *>(shipping_.get())); }
    if (var0 & 8) { s.store_field("tip_amount", tip_amount_); }
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_field("credentials_title", credentials_title_);
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_paymentReceiptStars::payments_paymentReceiptStars()
  : flags_()
  , date_()
  , bot_id_()
  , title_()
  , description_()
  , photo_()
  , invoice_()
  , currency_()
  , total_amount_()
  , transaction_id_()
  , users_()
{}

const std::int32_t payments_paymentReceiptStars::ID;

object_ptr<payments_PaymentReceipt> payments_paymentReceiptStars::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<payments_paymentReceiptStars> res = make_tl_object<payments_paymentReceiptStars>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->date_ = TlFetchInt::parse(p);
  res->bot_id_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  if (var0 & 4) { res->photo_ = TlFetchObject<WebDocument>::parse(p); }
  res->invoice_ = TlFetchBoxed<TlFetchObject<invoice>, 77522308>::parse(p);
  res->currency_ = TlFetchString<string>::parse(p);
  res->total_amount_ = TlFetchLong::parse(p);
  res->transaction_id_ = TlFetchString<string>::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void payments_paymentReceiptStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.paymentReceiptStars");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("date", date_);
    s.store_field("bot_id", bot_id_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    if (var0 & 4) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_field("transaction_id", transaction_id_);
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t payments_starGiftWithdrawalUrl::ID;

object_ptr<payments_starGiftWithdrawalUrl> payments_starGiftWithdrawalUrl::fetch(TlBufferParser &p) {
  return make_tl_object<payments_starGiftWithdrawalUrl>(p);
}

payments_starGiftWithdrawalUrl::payments_starGiftWithdrawalUrl(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
{}

void payments_starGiftWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.starGiftWithdrawalUrl");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

const std::int32_t phone_groupCallStreamChannels::ID;

object_ptr<phone_groupCallStreamChannels> phone_groupCallStreamChannels::fetch(TlBufferParser &p) {
  return make_tl_object<phone_groupCallStreamChannels>(p);
}

phone_groupCallStreamChannels::phone_groupCallStreamChannels(TlBufferParser &p)
  : channels_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<groupCallStreamChannel>, -2132064081>>, 481674261>::parse(p))
{}

void phone_groupCallStreamChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.groupCallStreamChannels");
    { s.store_vector_begin("channels", channels_.size()); for (const auto &_value : channels_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t photos_photo::ID;

object_ptr<photos_photo> photos_photo::fetch(TlBufferParser &p) {
  return make_tl_object<photos_photo>(p);
}

photos_photo::photos_photo(TlBufferParser &p)
  : photo_(TlFetchObject<Photo>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void photos_photo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.photo");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

smsjobs_status::smsjobs_status()
  : flags_()
  , allow_international_()
  , recent_sent_()
  , recent_since_()
  , recent_remains_()
  , total_sent_()
  , total_since_()
  , last_gift_slug_()
  , terms_url_()
{}

const std::int32_t smsjobs_status::ID;

object_ptr<smsjobs_status> smsjobs_status::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<smsjobs_status> res = make_tl_object<smsjobs_status>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->allow_international_ = (var0 & 1) != 0;
  res->recent_sent_ = TlFetchInt::parse(p);
  res->recent_since_ = TlFetchInt::parse(p);
  res->recent_remains_ = TlFetchInt::parse(p);
  res->total_sent_ = TlFetchInt::parse(p);
  res->total_since_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->last_gift_slug_ = TlFetchString<string>::parse(p); }
  res->terms_url_ = TlFetchString<string>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void smsjobs_status::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.status");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (allow_international_ << 0)));
    if (var0 & 1) { s.store_field("allow_international", true); }
    s.store_field("recent_sent", recent_sent_);
    s.store_field("recent_since", recent_since_);
    s.store_field("recent_remains", recent_remains_);
    s.store_field("total_sent", total_sent_);
    s.store_field("total_since", total_since_);
    if (var0 & 2) { s.store_field("last_gift_slug", last_gift_slug_); }
    s.store_field("terms_url", terms_url_);
    s.store_class_end();
  }
}

const std::int32_t stats_messageStats::ID;

object_ptr<stats_messageStats> stats_messageStats::fetch(TlBufferParser &p) {
  return make_tl_object<stats_messageStats>(p);
}

stats_messageStats::stats_messageStats(TlBufferParser &p)
  : views_graph_(TlFetchObject<StatsGraph>::parse(p))
  , reactions_by_emotion_graph_(TlFetchObject<StatsGraph>::parse(p))
{}

void stats_messageStats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.messageStats");
    s.store_object_field("views_graph", static_cast<const BaseObject *>(views_graph_.get()));
    s.store_object_field("reactions_by_emotion_graph", static_cast<const BaseObject *>(reactions_by_emotion_graph_.get()));
    s.store_class_end();
  }
}

const std::int32_t upload_webFile::ID;

object_ptr<upload_webFile> upload_webFile::fetch(TlBufferParser &p) {
  return make_tl_object<upload_webFile>(p);
}

upload_webFile::upload_webFile(TlBufferParser &p)
  : size_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , file_type_(TlFetchObject<storage_FileType>::parse(p))
  , mtime_(TlFetchInt::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
{}

void upload_webFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.webFile");
    s.store_field("size", size_);
    s.store_field("mime_type", mime_type_);
    s.store_object_field("file_type", static_cast<const BaseObject *>(file_type_.get()));
    s.store_field("mtime", mtime_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

const std::int32_t account_clearRecentEmojiStatuses::ID;

void account_clearRecentEmojiStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(404757166);
}

void account_clearRecentEmojiStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(404757166);
}

void account_clearRecentEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.clearRecentEmojiStatuses");
    s.store_class_end();
  }
}

account_clearRecentEmojiStatuses::ReturnType account_clearRecentEmojiStatuses::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_deleteAccount::account_deleteAccount(int32 flags_, string const &reason_, object_ptr<InputCheckPasswordSRP> &&password_)
  : flags_(flags_)
  , reason_(reason_)
  , password_(std::move(password_))
{}

const std::int32_t account_deleteAccount::ID;

void account_deleteAccount::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1564422284);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(reason_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(password_, s); }
}

void account_deleteAccount::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1564422284);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(reason_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(password_, s); }
}

void account_deleteAccount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.deleteAccount");
    s.store_field("flags", (var0 = flags_));
    s.store_field("reason", reason_);
    if (var0 & 1) { s.store_object_field("password", static_cast<const BaseObject *>(password_.get())); }
    s.store_class_end();
  }
}

account_deleteAccount::ReturnType account_deleteAccount::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_getAutoDownloadSettings::ID;

void account_getAutoDownloadSettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1457130303);
}

void account_getAutoDownloadSettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1457130303);
}

void account_getAutoDownloadSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getAutoDownloadSettings");
    s.store_class_end();
  }
}

account_getAutoDownloadSettings::ReturnType account_getAutoDownloadSettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_autoDownloadSettings>, 1674235686>::parse(p);
#undef FAIL
}

account_getNotifySettings::account_getNotifySettings(object_ptr<InputNotifyPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t account_getNotifySettings::ID;

void account_getNotifySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(313765169);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void account_getNotifySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(313765169);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void account_getNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getNotifySettings");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

account_getNotifySettings::ReturnType account_getNotifySettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<peerNotifySettings>, -1721619444>::parse(p);
#undef FAIL
}

account_getPaidMessagesRevenue::account_getPaidMessagesRevenue(int32 flags_, object_ptr<InputPeer> &&parent_peer_, object_ptr<InputUser> &&user_id_)
  : flags_(flags_)
  , parent_peer_(std::move(parent_peer_))
  , user_id_(std::move(user_id_))
{}

const std::int32_t account_getPaidMessagesRevenue::ID;

void account_getPaidMessagesRevenue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(431639143);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void account_getPaidMessagesRevenue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(431639143);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void account_getPaidMessagesRevenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getPaidMessagesRevenue");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

account_getPaidMessagesRevenue::ReturnType account_getPaidMessagesRevenue::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_paidMessagesRevenue>, 504403720>::parse(p);
#undef FAIL
}

const std::int32_t account_getReactionsNotifySettings::ID;

void account_getReactionsNotifySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(115172684);
}

void account_getReactionsNotifySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(115172684);
}

void account_getReactionsNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getReactionsNotifySettings");
    s.store_class_end();
  }
}

account_getReactionsNotifySettings::ReturnType account_getReactionsNotifySettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<reactionsNotifySettings>, 1457736048>::parse(p);
#undef FAIL
}

account_getSavedRingtones::account_getSavedRingtones(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getSavedRingtones::ID;

void account_getSavedRingtones::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-510647672);
  TlStoreBinary::store(hash_, s);
}

void account_getSavedRingtones::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-510647672);
  TlStoreBinary::store(hash_, s);
}

void account_getSavedRingtones::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getSavedRingtones");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getSavedRingtones::ReturnType account_getSavedRingtones::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<account_SavedRingtones>::parse(p);
#undef FAIL
}

account_reportPeer::account_reportPeer(object_ptr<InputPeer> &&peer_, object_ptr<ReportReason> &&reason_, string const &message_)
  : peer_(std::move(peer_))
  , reason_(std::move(reason_))
  , message_(message_)
{}

const std::int32_t account_reportPeer::ID;

void account_reportPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-977650298);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reason_, s);
  TlStoreString::store(message_, s);
}

void account_reportPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-977650298);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(reason_, s);
  TlStoreString::store(message_, s);
}

void account_reportPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.reportPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_field("message", message_);
    s.store_class_end();
  }
}

account_reportPeer::ReturnType account_reportPeer::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t account_resendPasswordEmail::ID;

void account_resendPasswordEmail::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2055154197);
}

void account_resendPasswordEmail::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2055154197);
}

void account_resendPasswordEmail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resendPasswordEmail");
    s.store_class_end();
  }
}

account_resendPasswordEmail::ReturnType account_resendPasswordEmail::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_setGlobalPrivacySettings::account_setGlobalPrivacySettings(object_ptr<globalPrivacySettings> &&settings_)
  : settings_(std::move(settings_))
{}

const std::int32_t account_setGlobalPrivacySettings::ID;

void account_setGlobalPrivacySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(517647042);
  TlStoreBoxed<TlStoreObject, -29248689>::store(settings_, s);
}

void account_setGlobalPrivacySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(517647042);
  TlStoreBoxed<TlStoreObject, -29248689>::store(settings_, s);
}

void account_setGlobalPrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setGlobalPrivacySettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

account_setGlobalPrivacySettings::ReturnType account_setGlobalPrivacySettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<globalPrivacySettings>, -29248689>::parse(p);
#undef FAIL
}

account_setPrivacy::account_setPrivacy(object_ptr<InputPrivacyKey> &&key_, array<object_ptr<InputPrivacyRule>> &&rules_)
  : key_(std::move(key_))
  , rules_(std::move(rules_))
{}

const std::int32_t account_setPrivacy::ID;

void account_setPrivacy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-906486552);
  TlStoreBoxedUnknown<TlStoreObject>::store(key_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(rules_, s);
}

void account_setPrivacy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-906486552);
  TlStoreBoxedUnknown<TlStoreObject>::store(key_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(rules_, s);
}

void account_setPrivacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.setPrivacy");
    s.store_object_field("key", static_cast<const BaseObject *>(key_.get()));
    { s.store_vector_begin("rules", rules_.size()); for (const auto &_value : rules_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_setPrivacy::ReturnType account_setPrivacy::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_privacyRules>, 1352683077>::parse(p);
#undef FAIL
}

account_toggleNoPaidMessagesException::account_toggleNoPaidMessagesException(int32 flags_, bool refund_charged_, bool require_payment_, object_ptr<InputPeer> &&parent_peer_, object_ptr<InputUser> &&user_id_)
  : flags_(flags_)
  , refund_charged_(refund_charged_)
  , require_payment_(require_payment_)
  , parent_peer_(std::move(parent_peer_))
  , user_id_(std::move(user_id_))
{}

const std::int32_t account_toggleNoPaidMessagesException::ID;

void account_toggleNoPaidMessagesException::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-30483850);
  TlStoreBinary::store((var0 = flags_ | (refund_charged_ << 0) | (require_payment_ << 2)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void account_toggleNoPaidMessagesException::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-30483850);
  TlStoreBinary::store((var0 = flags_ | (refund_charged_ << 0) | (require_payment_ << 2)), s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void account_toggleNoPaidMessagesException::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.toggleNoPaidMessagesException");
    s.store_field("flags", (var0 = flags_ | (refund_charged_ << 0) | (require_payment_ << 2)));
    if (var0 & 1) { s.store_field("refund_charged", true); }
    if (var0 & 4) { s.store_field("require_payment", true); }
    if (var0 & 2) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

account_toggleNoPaidMessagesException::ReturnType account_toggleNoPaidMessagesException::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_updateProfile::account_updateProfile(int32 flags_, string const &first_name_, string const &last_name_, string const &about_)
  : flags_(flags_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , about_(about_)
{}

const std::int32_t account_updateProfile::ID;

void account_updateProfile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2018596725);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(first_name_, s); }
  if (var0 & 2) { TlStoreString::store(last_name_, s); }
  if (var0 & 4) { TlStoreString::store(about_, s); }
}

void account_updateProfile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2018596725);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(first_name_, s); }
  if (var0 & 2) { TlStoreString::store(last_name_, s); }
  if (var0 & 4) { TlStoreString::store(about_, s); }
}

void account_updateProfile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.updateProfile");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("first_name", first_name_); }
    if (var0 & 2) { s.store_field("last_name", last_name_); }
    if (var0 & 4) { s.store_field("about", about_); }
    s.store_class_end();
  }
}

account_updateProfile::ReturnType account_updateProfile::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<User>::parse(p);
#undef FAIL
}

auth_checkPassword::auth_checkPassword(object_ptr<InputCheckPasswordSRP> &&password_)
  : password_(std::move(password_))
{}

const std::int32_t auth_checkPassword::ID;

void auth_checkPassword::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-779399914);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void auth_checkPassword::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-779399914);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void auth_checkPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.checkPassword");
    s.store_object_field("password", static_cast<const BaseObject *>(password_.get()));
    s.store_class_end();
  }
}

auth_checkPassword::ReturnType auth_checkPassword::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_Authorization>::parse(p);
#undef FAIL
}

auth_exportAuthorization::auth_exportAuthorization(int32 dc_id_)
  : dc_id_(dc_id_)
{}

const std::int32_t auth_exportAuthorization::ID;

void auth_exportAuthorization::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-440401971);
  TlStoreBinary::store(dc_id_, s);
}

void auth_exportAuthorization::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-440401971);
  TlStoreBinary::store(dc_id_, s);
}

void auth_exportAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.exportAuthorization");
    s.store_field("dc_id", dc_id_);
    s.store_class_end();
  }
}

auth_exportAuthorization::ReturnType auth_exportAuthorization::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<auth_exportedAuthorization>, -1271602504>::parse(p);
#undef FAIL
}

auth_importLoginToken::auth_importLoginToken(bytes &&token_)
  : token_(std::move(token_))
{}

const std::int32_t auth_importLoginToken::ID;

void auth_importLoginToken::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1783866140);
  TlStoreString::store(token_, s);
}

void auth_importLoginToken::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1783866140);
  TlStoreString::store(token_, s);
}

void auth_importLoginToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.importLoginToken");
    s.store_bytes_field("token", token_);
    s.store_class_end();
  }
}

auth_importLoginToken::ReturnType auth_importLoginToken::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_LoginToken>::parse(p);
#undef FAIL
}

const std::int32_t auth_resetAuthorizations::ID;

void auth_resetAuthorizations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1616179942);
}

void auth_resetAuthorizations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1616179942);
}

void auth_resetAuthorizations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.resetAuthorizations");
    s.store_class_end();
  }
}

auth_resetAuthorizations::ReturnType auth_resetAuthorizations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_addPreviewMedia::bots_addPreviewMedia(object_ptr<InputUser> &&bot_, string const &lang_code_, object_ptr<InputMedia> &&media_)
  : bot_(std::move(bot_))
  , lang_code_(lang_code_)
  , media_(std::move(media_))
{}

const std::int32_t bots_addPreviewMedia::ID;

void bots_addPreviewMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(397326170);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void bots_addPreviewMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(397326170);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void bots_addPreviewMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.addPreviewMedia");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("lang_code", lang_code_);
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

bots_addPreviewMedia::ReturnType bots_addPreviewMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<botPreviewMedia>, 602479523>::parse(p);
#undef FAIL
}

bots_deletePreviewMedia::bots_deletePreviewMedia(object_ptr<InputUser> &&bot_, string const &lang_code_, array<object_ptr<InputMedia>> &&media_)
  : bot_(std::move(bot_))
  , lang_code_(lang_code_)
  , media_(std::move(media_))
{}

const std::int32_t bots_deletePreviewMedia::ID;

void bots_deletePreviewMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(755054003);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(media_, s);
}

void bots_deletePreviewMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(755054003);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(media_, s);
}

void bots_deletePreviewMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.deletePreviewMedia");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("lang_code", lang_code_);
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

bots_deletePreviewMedia::ReturnType bots_deletePreviewMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_invokeWebViewCustomMethod::bots_invokeWebViewCustomMethod(object_ptr<InputUser> &&bot_, string const &custom_method_, object_ptr<dataJSON> &&params_)
  : bot_(std::move(bot_))
  , custom_method_(custom_method_)
  , params_(std::move(params_))
{}

const std::int32_t bots_invokeWebViewCustomMethod::ID;

void bots_invokeWebViewCustomMethod::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(142591463);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(custom_method_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void bots_invokeWebViewCustomMethod::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(142591463);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
  TlStoreString::store(custom_method_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void bots_invokeWebViewCustomMethod::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.invokeWebViewCustomMethod");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_field("custom_method", custom_method_);
    s.store_object_field("params", static_cast<const BaseObject *>(params_.get()));
    s.store_class_end();
  }
}

bots_invokeWebViewCustomMethod::ReturnType bots_invokeWebViewCustomMethod::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p);
#undef FAIL
}

bots_setBotInfo::bots_setBotInfo(int32 flags_, object_ptr<InputUser> &&bot_, string const &lang_code_, string const &name_, string const &about_, string const &description_)
  : flags_(flags_)
  , bot_(std::move(bot_))
  , lang_code_(lang_code_)
  , name_(name_)
  , about_(about_)
  , description_(description_)
{}

const std::int32_t bots_setBotInfo::ID;

void bots_setBotInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(282013987);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreString::store(lang_code_, s);
  if (var0 & 8) { TlStoreString::store(name_, s); }
  if (var0 & 1) { TlStoreString::store(about_, s); }
  if (var0 & 2) { TlStoreString::store(description_, s); }
}

void bots_setBotInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(282013987);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreString::store(lang_code_, s);
  if (var0 & 8) { TlStoreString::store(name_, s); }
  if (var0 & 1) { TlStoreString::store(about_, s); }
  if (var0 & 2) { TlStoreString::store(description_, s); }
}

void bots_setBotInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.setBotInfo");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 4) { s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get())); }
    s.store_field("lang_code", lang_code_);
    if (var0 & 8) { s.store_field("name", name_); }
    if (var0 & 1) { s.store_field("about", about_); }
    if (var0 & 2) { s.store_field("description", description_); }
    s.store_class_end();
  }
}

bots_setBotInfo::ReturnType bots_setBotInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_convertToGigagroup::channels_convertToGigagroup(object_ptr<InputChannel> &&channel_)
  : channel_(std::move(channel_))
{}

const std::int32_t channels_convertToGigagroup::ID;

void channels_convertToGigagroup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(187239529);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_convertToGigagroup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(187239529);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_convertToGigagroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.convertToGigagroup");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

channels_convertToGigagroup::ReturnType channels_convertToGigagroup::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_editLocation::channels_editLocation(object_ptr<InputChannel> &&channel_, object_ptr<InputGeoPoint> &&geo_point_, string const &address_)
  : channel_(std::move(channel_))
  , geo_point_(std::move(geo_point_))
  , address_(address_)
{}

const std::int32_t channels_editLocation::ID;

void channels_editLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1491484525);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreString::store(address_, s);
}

void channels_editLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1491484525);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  TlStoreString::store(address_, s);
}

void channels_editLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.editLocation");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    s.store_field("address", address_);
    s.store_class_end();
  }
}

channels_editLocation::ReturnType channels_editLocation::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_inviteToChannel::channels_inviteToChannel(object_ptr<InputChannel> &&channel_, array<object_ptr<InputUser>> &&users_)
  : channel_(std::move(channel_))
  , users_(std::move(users_))
{}

const std::int32_t channels_inviteToChannel::ID;

void channels_inviteToChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-907854508);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void channels_inviteToChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-907854508);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void channels_inviteToChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.inviteToChannel");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

channels_inviteToChannel::ReturnType channels_inviteToChannel::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_invitedUsers>, 2136862630>::parse(p);
#undef FAIL
}

channels_setDiscussionGroup::channels_setDiscussionGroup(object_ptr<InputChannel> &&broadcast_, object_ptr<InputChannel> &&group_)
  : broadcast_(std::move(broadcast_))
  , group_(std::move(group_))
{}

const std::int32_t channels_setDiscussionGroup::ID;

void channels_setDiscussionGroup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1079520178);
  TlStoreBoxedUnknown<TlStoreObject>::store(broadcast_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(group_, s);
}

void channels_setDiscussionGroup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1079520178);
  TlStoreBoxedUnknown<TlStoreObject>::store(broadcast_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(group_, s);
}

void channels_setDiscussionGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.setDiscussionGroup");
    s.store_object_field("broadcast", static_cast<const BaseObject *>(broadcast_.get()));
    s.store_object_field("group", static_cast<const BaseObject *>(group_.get()));
    s.store_class_end();
  }
}

channels_setDiscussionGroup::ReturnType channels_setDiscussionGroup::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_setMainProfileTab::channels_setMainProfileTab(object_ptr<InputChannel> &&channel_, object_ptr<ProfileTab> &&tab_)
  : channel_(std::move(channel_))
  , tab_(std::move(tab_))
{}

const std::int32_t channels_setMainProfileTab::ID;

void channels_setMainProfileTab::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(897842353);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(tab_, s);
}

void channels_setMainProfileTab::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(897842353);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(tab_, s);
}

void channels_setMainProfileTab::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.setMainProfileTab");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("tab", static_cast<const BaseObject *>(tab_.get()));
    s.store_class_end();
  }
}

channels_setMainProfileTab::ReturnType channels_setMainProfileTab::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

channels_updateUsername::channels_updateUsername(object_ptr<InputChannel> &&channel_, string const &username_)
  : channel_(std::move(channel_))
  , username_(username_)
{}

const std::int32_t channels_updateUsername::ID;

void channels_updateUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(890549214);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(username_, s);
}

void channels_updateUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(890549214);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreString::store(username_, s);
}

void channels_updateUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.updateUsername");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("username", username_);
    s.store_class_end();
  }
}

channels_updateUsername::ReturnType channels_updateUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

contacts_getSponsoredPeers::contacts_getSponsoredPeers(string const &q_)
  : q_(q_)
{}

const std::int32_t contacts_getSponsoredPeers::ID;

void contacts_getSponsoredPeers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1228356717);
  TlStoreString::store(q_, s);
}

void contacts_getSponsoredPeers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1228356717);
  TlStoreString::store(q_, s);
}

void contacts_getSponsoredPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getSponsoredPeers");
    s.store_field("q", q_);
    s.store_class_end();
  }
}

contacts_getSponsoredPeers::ReturnType contacts_getSponsoredPeers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<contacts_SponsoredPeers>::parse(p);
#undef FAIL
}

contacts_getTopPeers::contacts_getTopPeers(int32 flags_, bool correspondents_, bool bots_pm_, bool bots_inline_, bool phone_calls_, bool forward_users_, bool forward_chats_, bool groups_, bool channels_, bool bots_app_, int32 offset_, int32 limit_, int64 hash_)
  : flags_(flags_)
  , correspondents_(correspondents_)
  , bots_pm_(bots_pm_)
  , bots_inline_(bots_inline_)
  , phone_calls_(phone_calls_)
  , forward_users_(forward_users_)
  , forward_chats_(forward_chats_)
  , groups_(groups_)
  , channels_(channels_)
  , bots_app_(bots_app_)
  , offset_(offset_)
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t contacts_getTopPeers::ID;

void contacts_getTopPeers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1758168906);
  TlStoreBinary::store((var0 = flags_ | (correspondents_ << 0) | (bots_pm_ << 1) | (bots_inline_ << 2) | (phone_calls_ << 3) | (forward_users_ << 4) | (forward_chats_ << 5) | (groups_ << 10) | (channels_ << 15) | (bots_app_ << 16)), s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void contacts_getTopPeers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1758168906);
  TlStoreBinary::store((var0 = flags_ | (correspondents_ << 0) | (bots_pm_ << 1) | (bots_inline_ << 2) | (phone_calls_ << 3) | (forward_users_ << 4) | (forward_chats_ << 5) | (groups_ << 10) | (channels_ << 15) | (bots_app_ << 16)), s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void contacts_getTopPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getTopPeers");
    s.store_field("flags", (var0 = flags_ | (correspondents_ << 0) | (bots_pm_ << 1) | (bots_inline_ << 2) | (phone_calls_ << 3) | (forward_users_ << 4) | (forward_chats_ << 5) | (groups_ << 10) | (channels_ << 15) | (bots_app_ << 16)));
    if (var0 & 1) { s.store_field("correspondents", true); }
    if (var0 & 2) { s.store_field("bots_pm", true); }
    if (var0 & 4) { s.store_field("bots_inline", true); }
    if (var0 & 8) { s.store_field("phone_calls", true); }
    if (var0 & 16) { s.store_field("forward_users", true); }
    if (var0 & 32) { s.store_field("forward_chats", true); }
    if (var0 & 1024) { s.store_field("groups", true); }
    if (var0 & 32768) { s.store_field("channels", true); }
    if (var0 & 65536) { s.store_field("bots_app", true); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

contacts_getTopPeers::ReturnType contacts_getTopPeers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<contacts_TopPeers>::parse(p);
#undef FAIL
}

contacts_importContacts::contacts_importContacts(array<object_ptr<inputPhoneContact>> &&contacts_)
  : contacts_(std::move(contacts_))
{}

const std::int32_t contacts_importContacts::ID;

void contacts_importContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(746589157);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 1780335806>>, 481674261>::store(contacts_, s);
}

void contacts_importContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(746589157);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 1780335806>>, 481674261>::store(contacts_, s);
}

void contacts_importContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.importContacts");
    { s.store_vector_begin("contacts", contacts_.size()); for (const auto &_value : contacts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

contacts_importContacts::ReturnType contacts_importContacts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<contacts_importedContacts>, 2010127419>::parse(p);
#undef FAIL
}

const std::int32_t contacts_resetSaved::ID;

void contacts_resetSaved::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2020263951);
}

void contacts_resetSaved::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2020263951);
}

void contacts_resetSaved::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.resetSaved");
    s.store_class_end();
  }
}

contacts_resetSaved::ReturnType contacts_resetSaved::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

contacts_unblock::contacts_unblock(int32 flags_, bool my_stories_from_, object_ptr<InputPeer> &&id_)
  : flags_(flags_)
  , my_stories_from_(my_stories_from_)
  , id_(std::move(id_))
{}

const std::int32_t contacts_unblock::ID;

void contacts_unblock::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1252994264);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void contacts_unblock::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1252994264);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void contacts_unblock::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.unblock");
    s.store_field("flags", (var0 = flags_ | (my_stories_from_ << 0)));
    if (var0 & 1) { s.store_field("my_stories_from", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

contacts_unblock::ReturnType contacts_unblock::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

help_dismissSuggestion::help_dismissSuggestion(object_ptr<InputPeer> &&peer_, string const &suggestion_)
  : peer_(std::move(peer_))
  , suggestion_(suggestion_)
{}

const std::int32_t help_dismissSuggestion::ID;

void help_dismissSuggestion::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-183649631);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(suggestion_, s);
}

void help_dismissSuggestion::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-183649631);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(suggestion_, s);
}

void help_dismissSuggestion::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.dismissSuggestion");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("suggestion", suggestion_);
    s.store_class_end();
  }
}

help_dismissSuggestion::ReturnType help_dismissSuggestion::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

const std::int32_t help_getConfig::ID;

void help_getConfig::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-990308245);
}

void help_getConfig::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-990308245);
}

void help_getConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getConfig");
    s.store_class_end();
  }
}

help_getConfig::ReturnType help_getConfig::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<config>, -870702050>::parse(p);
#undef FAIL
}

help_getDeepLinkInfo::help_getDeepLinkInfo(string const &path_)
  : path_(path_)
{}

const std::int32_t help_getDeepLinkInfo::ID;

void help_getDeepLinkInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1072547679);
  TlStoreString::store(path_, s);
}

void help_getDeepLinkInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1072547679);
  TlStoreString::store(path_, s);
}

void help_getDeepLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getDeepLinkInfo");
    s.store_field("path", path_);
    s.store_class_end();
  }
}

help_getDeepLinkInfo::ReturnType help_getDeepLinkInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_DeepLinkInfo>::parse(p);
#undef FAIL
}

help_getTimezonesList::help_getTimezonesList(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t help_getTimezonesList::ID;

void help_getTimezonesList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1236468288);
  TlStoreBinary::store(hash_, s);
}

void help_getTimezonesList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1236468288);
  TlStoreBinary::store(hash_, s);
}

void help_getTimezonesList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getTimezonesList");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

help_getTimezonesList::ReturnType help_getTimezonesList::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_TimezonesList>::parse(p);
#undef FAIL
}

langpack_getStrings::langpack_getStrings(string const &lang_pack_, string const &lang_code_, array<string> &&keys_)
  : lang_pack_(lang_pack_)
  , lang_code_(lang_code_)
  , keys_(std::move(keys_))
{}

const std::int32_t langpack_getStrings::ID;

void langpack_getStrings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-269862909);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(keys_, s);
}

void langpack_getStrings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-269862909);
  TlStoreString::store(lang_pack_, s);
  TlStoreString::store(lang_code_, s);
  TlStoreBoxed<TlStoreVector<TlStoreString>, 481674261>::store(keys_, s);
}

void langpack_getStrings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "langpack.getStrings");
    s.store_field("lang_pack", lang_pack_);
    s.store_field("lang_code", lang_code_);
    { s.store_vector_begin("keys", keys_.size()); for (const auto &_value : keys_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

langpack_getStrings::ReturnType langpack_getStrings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchObject<LangPackString>>, 481674261>::parse(p);
#undef FAIL
}

messages_checkHistoryImport::messages_checkHistoryImport(string const &import_head_)
  : import_head_(import_head_)
{}

const std::int32_t messages_checkHistoryImport::ID;

void messages_checkHistoryImport::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1140726259);
  TlStoreString::store(import_head_, s);
}

void messages_checkHistoryImport::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1140726259);
  TlStoreString::store(import_head_, s);
}

void messages_checkHistoryImport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.checkHistoryImport");
    s.store_field("import_head", import_head_);
    s.store_class_end();
  }
}

messages_checkHistoryImport::ReturnType messages_checkHistoryImport::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_historyImportParsed>, 1578088377>::parse(p);
#undef FAIL
}

messages_checkQuickReplyShortcut::messages_checkQuickReplyShortcut(string const &shortcut_)
  : shortcut_(shortcut_)
{}

const std::int32_t messages_checkQuickReplyShortcut::ID;

void messages_checkQuickReplyShortcut::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-237962285);
  TlStoreString::store(shortcut_, s);
}

void messages_checkQuickReplyShortcut::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-237962285);
  TlStoreString::store(shortcut_, s);
}

void messages_checkQuickReplyShortcut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.checkQuickReplyShortcut");
    s.store_field("shortcut", shortcut_);
    s.store_class_end();
  }
}

messages_checkQuickReplyShortcut::ReturnType messages_checkQuickReplyShortcut::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_discardEncryption::messages_discardEncryption(int32 flags_, bool delete_history_, int32 chat_id_)
  : flags_(flags_)
  , delete_history_(delete_history_)
  , chat_id_(chat_id_)
{}

const std::int32_t messages_discardEncryption::ID;

void messages_discardEncryption::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-208425312);
  TlStoreBinary::store((var0 = flags_ | (delete_history_ << 0)), s);
  TlStoreBinary::store(chat_id_, s);
}

void messages_discardEncryption::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-208425312);
  TlStoreBinary::store((var0 = flags_ | (delete_history_ << 0)), s);
  TlStoreBinary::store(chat_id_, s);
}

void messages_discardEncryption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.discardEncryption");
    s.store_field("flags", (var0 = flags_ | (delete_history_ << 0)));
    if (var0 & 1) { s.store_field("delete_history", true); }
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

messages_discardEncryption::ReturnType messages_discardEncryption::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_editChatAbout::messages_editChatAbout(object_ptr<InputPeer> &&peer_, string const &about_)
  : peer_(std::move(peer_))
  , about_(about_)
{}

const std::int32_t messages_editChatAbout::ID;

void messages_editChatAbout::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-554301545);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(about_, s);
}

void messages_editChatAbout::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-554301545);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(about_, s);
}

void messages_editChatAbout::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.editChatAbout");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("about", about_);
    s.store_class_end();
  }
}

messages_editChatAbout::ReturnType messages_editChatAbout::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_forwardMessages::messages_forwardMessages(int32 flags_, bool silent_, bool background_, bool with_my_score_, bool drop_author_, bool drop_media_captions_, bool noforwards_, bool allow_paid_floodskip_, object_ptr<InputPeer> &&from_peer_, array<int32> &&id_, array<int64> &&random_id_, object_ptr<InputPeer> &&to_peer_, int32 top_msg_id_, object_ptr<InputReplyTo> &&reply_to_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_, object_ptr<InputQuickReplyShortcut> &&quick_reply_shortcut_, int32 video_timestamp_, int64 allow_paid_stars_, object_ptr<suggestedPost> &&suggested_post_)
  : flags_(flags_)
  , silent_(silent_)
  , background_(background_)
  , with_my_score_(with_my_score_)
  , drop_author_(drop_author_)
  , drop_media_captions_(drop_media_captions_)
  , noforwards_(noforwards_)
  , allow_paid_floodskip_(allow_paid_floodskip_)
  , from_peer_(std::move(from_peer_))
  , id_(std::move(id_))
  , random_id_(std::move(random_id_))
  , to_peer_(std::move(to_peer_))
  , top_msg_id_(top_msg_id_)
  , reply_to_(std::move(reply_to_))
  , schedule_date_(schedule_date_)
  , send_as_(std::move(send_as_))
  , quick_reply_shortcut_(std::move(quick_reply_shortcut_))
  , video_timestamp_(video_timestamp_)
  , allow_paid_stars_(allow_paid_stars_)
  , suggested_post_(std::move(suggested_post_))
{}

const std::int32_t messages_forwardMessages::ID;

void messages_forwardMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1752618806);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (with_my_score_ << 8) | (drop_author_ << 11) | (drop_media_captions_ << 12) | (noforwards_ << 14) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(from_peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_peer_, s);
  if (var0 & 512) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 4194304) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 1048576) { TlStoreBinary::store(video_timestamp_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
  if (var0 & 8388608) { TlStoreBoxed<TlStoreObject, 244201445>::store(suggested_post_, s); }
}

void messages_forwardMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1752618806);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (with_my_score_ << 8) | (drop_author_ << 11) | (drop_media_captions_ << 12) | (noforwards_ << 14) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(from_peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(to_peer_, s);
  if (var0 & 512) { TlStoreBinary::store(top_msg_id_, s); }
  if (var0 & 4194304) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 1048576) { TlStoreBinary::store(video_timestamp_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
  if (var0 & 8388608) { TlStoreBoxed<TlStoreObject, 244201445>::store(suggested_post_, s); }
}

void messages_forwardMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.forwardMessages");
    s.store_field("flags", (var0 = flags_ | (silent_ << 5) | (background_ << 6) | (with_my_score_ << 8) | (drop_author_ << 11) | (drop_media_captions_ << 12) | (noforwards_ << 14) | (allow_paid_floodskip_ << 19)));
    if (var0 & 32) { s.store_field("silent", true); }
    if (var0 & 64) { s.store_field("background", true); }
    if (var0 & 256) { s.store_field("with_my_score", true); }
    if (var0 & 2048) { s.store_field("drop_author", true); }
    if (var0 & 4096) { s.store_field("drop_media_captions", true); }
    if (var0 & 16384) { s.store_field("noforwards", true); }
    if (var0 & 524288) { s.store_field("allow_paid_floodskip", true); }
    s.store_object_field("from_peer", static_cast<const BaseObject *>(from_peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("random_id", random_id_.size()); for (const auto &_value : random_id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_object_field("to_peer", static_cast<const BaseObject *>(to_peer_.get()));
    if (var0 & 512) { s.store_field("top_msg_id", top_msg_id_); }
    if (var0 & 4194304) { s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get())); }
    if (var0 & 1024) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 8192) { s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get())); }
    if (var0 & 131072) { s.store_object_field("quick_reply_shortcut", static_cast<const BaseObject *>(quick_reply_shortcut_.get())); }
    if (var0 & 1048576) { s.store_field("video_timestamp", video_timestamp_); }
    if (var0 & 2097152) { s.store_field("allow_paid_stars", allow_paid_stars_); }
    if (var0 & 8388608) { s.store_object_field("suggested_post", static_cast<const BaseObject *>(suggested_post_.get())); }
    s.store_class_end();
  }
}

messages_forwardMessages::ReturnType messages_forwardMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getAllStickers::messages_getAllStickers(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t messages_getAllStickers::ID;

void messages_getAllStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1197432408);
  TlStoreBinary::store(hash_, s);
}

void messages_getAllStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1197432408);
  TlStoreBinary::store(hash_, s);
}

void messages_getAllStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAllStickers");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getAllStickers::ReturnType messages_getAllStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_AllStickers>::parse(p);
#undef FAIL
}

messages_getAttachMenuBot::messages_getAttachMenuBot(object_ptr<InputUser> &&bot_)
  : bot_(std::move(bot_))
{}

const std::int32_t messages_getAttachMenuBot::ID;

void messages_getAttachMenuBot::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1998676370);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void messages_getAttachMenuBot::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1998676370);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void messages_getAttachMenuBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getAttachMenuBot");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

messages_getAttachMenuBot::ReturnType messages_getAttachMenuBot::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<attachMenuBotsBot>, -1816172929>::parse(p);
#undef FAIL
}

messages_getEmojiKeywordsDifference::messages_getEmojiKeywordsDifference(string const &lang_code_, int32 from_version_)
  : lang_code_(lang_code_)
  , from_version_(from_version_)
{}

const std::int32_t messages_getEmojiKeywordsDifference::ID;

void messages_getEmojiKeywordsDifference::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(352892591);
  TlStoreString::store(lang_code_, s);
  TlStoreBinary::store(from_version_, s);
}

void messages_getEmojiKeywordsDifference::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(352892591);
  TlStoreString::store(lang_code_, s);
  TlStoreBinary::store(from_version_, s);
}

void messages_getEmojiKeywordsDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getEmojiKeywordsDifference");
    s.store_field("lang_code", lang_code_);
    s.store_field("from_version", from_version_);
    s.store_class_end();
  }
}

messages_getEmojiKeywordsDifference::ReturnType messages_getEmojiKeywordsDifference::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<emojiKeywordsDifference>, 1556570557>::parse(p);
#undef FAIL
}

messages_getExportedChatInvites::messages_getExportedChatInvites(int32 flags_, bool revoked_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&admin_id_, int32 offset_date_, string const &offset_link_, int32 limit_)
  : flags_(flags_)
  , revoked_(revoked_)
  , peer_(std::move(peer_))
  , admin_id_(std::move(admin_id_))
  , offset_date_(offset_date_)
  , offset_link_(offset_link_)
  , limit_(limit_)
{}

const std::int32_t messages_getExportedChatInvites::ID;

void messages_getExportedChatInvites::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1565154314);
  TlStoreBinary::store((var0 = flags_ | (revoked_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(admin_id_, s);
  if (var0 & 4) { TlStoreBinary::store(offset_date_, s); }
  if (var0 & 4) { TlStoreString::store(offset_link_, s); }
  TlStoreBinary::store(limit_, s);
}

void messages_getExportedChatInvites::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1565154314);
  TlStoreBinary::store((var0 = flags_ | (revoked_ << 3)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(admin_id_, s);
  if (var0 & 4) { TlStoreBinary::store(offset_date_, s); }
  if (var0 & 4) { TlStoreString::store(offset_link_, s); }
  TlStoreBinary::store(limit_, s);
}

void messages_getExportedChatInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getExportedChatInvites");
    s.store_field("flags", (var0 = flags_ | (revoked_ << 3)));
    if (var0 & 8) { s.store_field("revoked", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("admin_id", static_cast<const BaseObject *>(admin_id_.get()));
    if (var0 & 4) { s.store_field("offset_date", offset_date_); }
    if (var0 & 4) { s.store_field("offset_link", offset_link_); }
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getExportedChatInvites::ReturnType messages_getExportedChatInvites::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_exportedChatInvites>, -1111085620>::parse(p);
#undef FAIL
}

messages_getFullChat::messages_getFullChat(int64 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t messages_getFullChat::ID;

void messages_getFullChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1364194508);
  TlStoreBinary::store(chat_id_, s);
}

void messages_getFullChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1364194508);
  TlStoreBinary::store(chat_id_, s);
}

void messages_getFullChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getFullChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

messages_getFullChat::ReturnType messages_getFullChat::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_chatFull>, -438840932>::parse(p);
#undef FAIL
}

messages_getMessageEditData::messages_getMessageEditData(object_ptr<InputPeer> &&peer_, int32 id_)
  : peer_(std::move(peer_))
  , id_(id_)
{}

const std::int32_t messages_getMessageEditData::ID;

void messages_getMessageEditData::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-39416522);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void messages_getMessageEditData::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-39416522);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
}

void messages_getMessageEditData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getMessageEditData");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

messages_getMessageEditData::ReturnType messages_getMessageEditData::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_messageEditData>, 649453030>::parse(p);
#undef FAIL
}

messages_getOutboxReadDate::messages_getOutboxReadDate(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t messages_getOutboxReadDate::ID;

void messages_getOutboxReadDate::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1941176739);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getOutboxReadDate::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1941176739);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getOutboxReadDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getOutboxReadDate");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

messages_getOutboxReadDate::ReturnType messages_getOutboxReadDate::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<outboxReadDate>, 1001931436>::parse(p);
#undef FAIL
}

const std::int32_t messages_getPaidReactionPrivacy::ID;

void messages_getPaidReactionPrivacy::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1193563562);
}

void messages_getPaidReactionPrivacy::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1193563562);
}

void messages_getPaidReactionPrivacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPaidReactionPrivacy");
    s.store_class_end();
  }
}

messages_getPaidReactionPrivacy::ReturnType messages_getPaidReactionPrivacy::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getRecentLocations::messages_getRecentLocations(object_ptr<InputPeer> &&peer_, int32 limit_, int64 hash_)
  : peer_(std::move(peer_))
  , limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t messages_getRecentLocations::ID;

void messages_getRecentLocations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1881817312);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getRecentLocations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1881817312);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getRecentLocations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getRecentLocations");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getRecentLocations::ReturnType messages_getRecentLocations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_getRecentReactions::messages_getRecentReactions(int32 limit_, int64 hash_)
  : limit_(limit_)
  , hash_(hash_)
{}

const std::int32_t messages_getRecentReactions::ID;

void messages_getRecentReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(960896434);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getRecentReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(960896434);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getRecentReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getRecentReactions");
    s.store_field("limit", limit_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getRecentReactions::ReturnType messages_getRecentReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Reactions>::parse(p);
#undef FAIL
}

messages_getRecentStickers::messages_getRecentStickers(int32 flags_, bool attached_, int64 hash_)
  : flags_(flags_)
  , attached_(attached_)
  , hash_(hash_)
{}

const std::int32_t messages_getRecentStickers::ID;

void messages_getRecentStickers::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1649852357);
  TlStoreBinary::store((var0 = flags_ | (attached_ << 0)), s);
  TlStoreBinary::store(hash_, s);
}

void messages_getRecentStickers::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1649852357);
  TlStoreBinary::store((var0 = flags_ | (attached_ << 0)), s);
  TlStoreBinary::store(hash_, s);
}

void messages_getRecentStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getRecentStickers");
    s.store_field("flags", (var0 = flags_ | (attached_ << 0)));
    if (var0 & 1) { s.store_field("attached", true); }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getRecentStickers::ReturnType messages_getRecentStickers::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_RecentStickers>::parse(p);
#undef FAIL
}

messages_getWebPagePreview::messages_getWebPagePreview(int32 flags_, string const &message_, array<object_ptr<MessageEntity>> &&entities_)
  : flags_(flags_)
  , message_(message_)
  , entities_(std::move(entities_))
{}

const std::int32_t messages_getWebPagePreview::ID;

void messages_getWebPagePreview::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1460498287);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(message_, s);
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
}

void messages_getWebPagePreview::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1460498287);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(message_, s);
  if (var0 & 8) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
}

void messages_getWebPagePreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getWebPagePreview");
    s.store_field("flags", (var0 = flags_));
    s.store_field("message", message_);
    if (var0 & 8) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

messages_getWebPagePreview::ReturnType messages_getWebPagePreview::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_webPagePreview>, -1936029524>::parse(p);
#undef FAIL
}

messages_hideAllChatJoinRequests::messages_hideAllChatJoinRequests(int32 flags_, bool approved_, object_ptr<InputPeer> &&peer_, string const &link_)
  : flags_(flags_)
  , approved_(approved_)
  , peer_(std::move(peer_))
  , link_(link_)
{}

const std::int32_t messages_hideAllChatJoinRequests::ID;

void messages_hideAllChatJoinRequests::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-528091926);
  TlStoreBinary::store((var0 = flags_ | (approved_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreString::store(link_, s); }
}

void messages_hideAllChatJoinRequests::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-528091926);
  TlStoreBinary::store((var0 = flags_ | (approved_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 2) { TlStoreString::store(link_, s); }
}

void messages_hideAllChatJoinRequests::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.hideAllChatJoinRequests");
    s.store_field("flags", (var0 = flags_ | (approved_ << 0)));
    if (var0 & 1) { s.store_field("approved", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 2) { s.store_field("link", link_); }
    s.store_class_end();
  }
}

messages_hideAllChatJoinRequests::ReturnType messages_hideAllChatJoinRequests::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_hideChatJoinRequest::messages_hideChatJoinRequest(int32 flags_, bool approved_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&user_id_)
  : flags_(flags_)
  , approved_(approved_)
  , peer_(std::move(peer_))
  , user_id_(std::move(user_id_))
{}

const std::int32_t messages_hideChatJoinRequest::ID;

void messages_hideChatJoinRequest::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2145904661);
  TlStoreBinary::store((var0 = flags_ | (approved_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_hideChatJoinRequest::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2145904661);
  TlStoreBinary::store((var0 = flags_ | (approved_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
}

void messages_hideChatJoinRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.hideChatJoinRequest");
    s.store_field("flags", (var0 = flags_ | (approved_ << 0)));
    if (var0 & 1) { s.store_field("approved", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_class_end();
  }
}

messages_hideChatJoinRequest::ReturnType messages_hideChatJoinRequest::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_rateTranscribedAudio::messages_rateTranscribedAudio(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 transcription_id_, bool good_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , transcription_id_(transcription_id_)
  , good_(good_)
{}

const std::int32_t messages_rateTranscribedAudio::ID;

void messages_rateTranscribedAudio::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2132608815);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(transcription_id_, s);
  TlStoreBool::store(good_, s);
}

void messages_rateTranscribedAudio::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2132608815);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(transcription_id_, s);
  TlStoreBool::store(good_, s);
}

void messages_rateTranscribedAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.rateTranscribedAudio");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("transcription_id", transcription_id_);
    s.store_field("good", good_);
    s.store_class_end();
  }
}

messages_rateTranscribedAudio::ReturnType messages_rateTranscribedAudio::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_saveGif::messages_saveGif(object_ptr<InputDocument> &&id_, bool unsave_)
  : id_(std::move(id_))
  , unsave_(unsave_)
{}

const std::int32_t messages_saveGif::ID;

void messages_saveGif::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(846868683);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unsave_, s);
}

void messages_saveGif::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(846868683);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBool::store(unsave_, s);
}

void messages_saveGif::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.saveGif");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_field("unsave", unsave_);
    s.store_class_end();
  }
}

messages_saveGif::ReturnType messages_saveGif::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_searchGlobal::messages_searchGlobal(int32 flags_, bool broadcasts_only_, bool groups_only_, bool users_only_, int32 folder_id_, string const &q_, object_ptr<MessagesFilter> &&filter_, int32 min_date_, int32 max_date_, int32 offset_rate_, object_ptr<InputPeer> &&offset_peer_, int32 offset_id_, int32 limit_)
  : flags_(flags_)
  , broadcasts_only_(broadcasts_only_)
  , groups_only_(groups_only_)
  , users_only_(users_only_)
  , folder_id_(folder_id_)
  , q_(q_)
  , filter_(std::move(filter_))
  , min_date_(min_date_)
  , max_date_(max_date_)
  , offset_rate_(offset_rate_)
  , offset_peer_(std::move(offset_peer_))
  , offset_id_(offset_id_)
  , limit_(limit_)
{}

const std::int32_t messages_searchGlobal::ID;

void messages_searchGlobal::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1271290010);
  TlStoreBinary::store((var0 = flags_ | (broadcasts_only_ << 1) | (groups_only_ << 2) | (users_only_ << 3)), s);
  if (var0 & 1) { TlStoreBinary::store(folder_id_, s); }
  TlStoreString::store(q_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(min_date_, s);
  TlStoreBinary::store(max_date_, s);
  TlStoreBinary::store(offset_rate_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_searchGlobal::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1271290010);
  TlStoreBinary::store((var0 = flags_ | (broadcasts_only_ << 1) | (groups_only_ << 2) | (users_only_ << 3)), s);
  if (var0 & 1) { TlStoreBinary::store(folder_id_, s); }
  TlStoreString::store(q_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(filter_, s);
  TlStoreBinary::store(min_date_, s);
  TlStoreBinary::store(max_date_, s);
  TlStoreBinary::store(offset_rate_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(offset_peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_searchGlobal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchGlobal");
    s.store_field("flags", (var0 = flags_ | (broadcasts_only_ << 1) | (groups_only_ << 2) | (users_only_ << 3)));
    if (var0 & 2) { s.store_field("broadcasts_only", true); }
    if (var0 & 4) { s.store_field("groups_only", true); }
    if (var0 & 8) { s.store_field("users_only", true); }
    if (var0 & 1) { s.store_field("folder_id", folder_id_); }
    s.store_field("q", q_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("min_date", min_date_);
    s.store_field("max_date", max_date_);
    s.store_field("offset_rate", offset_rate_);
    s.store_object_field("offset_peer", static_cast<const BaseObject *>(offset_peer_.get()));
    s.store_field("offset_id", offset_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_searchGlobal::ReturnType messages_searchGlobal::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_sendMultiMedia::messages_sendMultiMedia(int32 flags_, bool silent_, bool background_, bool clear_draft_, bool noforwards_, bool update_stickersets_order_, bool invert_media_, bool allow_paid_floodskip_, object_ptr<InputPeer> &&peer_, object_ptr<InputReplyTo> &&reply_to_, array<object_ptr<inputSingleMedia>> &&multi_media_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_, object_ptr<InputQuickReplyShortcut> &&quick_reply_shortcut_, int64 effect_, int64 allow_paid_stars_)
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
  , multi_media_(std::move(multi_media_))
  , schedule_date_(schedule_date_)
  , send_as_(std::move(send_as_))
  , quick_reply_shortcut_(std::move(quick_reply_shortcut_))
  , effect_(effect_)
  , allow_paid_stars_(allow_paid_stars_)
{}

const std::int32_t messages_sendMultiMedia::ID;

void messages_sendMultiMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(469278068);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 482797855>>, 481674261>::store(multi_media_, s);
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 262144) { TlStoreBinary::store(effect_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
}

void messages_sendMultiMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(469278068);
  TlStoreBinary::store((var0 = flags_ | (silent_ << 5) | (background_ << 6) | (clear_draft_ << 7) | (noforwards_ << 14) | (update_stickersets_order_ << 15) | (invert_media_ << 16) | (allow_paid_floodskip_ << 19)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(reply_to_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 482797855>>, 481674261>::store(multi_media_, s);
  if (var0 & 1024) { TlStoreBinary::store(schedule_date_, s); }
  if (var0 & 8192) { TlStoreBoxedUnknown<TlStoreObject>::store(send_as_, s); }
  if (var0 & 131072) { TlStoreBoxedUnknown<TlStoreObject>::store(quick_reply_shortcut_, s); }
  if (var0 & 262144) { TlStoreBinary::store(effect_, s); }
  if (var0 & 2097152) { TlStoreBinary::store(allow_paid_stars_, s); }
}

void messages_sendMultiMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendMultiMedia");
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
    { s.store_vector_begin("multi_media", multi_media_.size()); for (const auto &_value : multi_media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 1024) { s.store_field("schedule_date", schedule_date_); }
    if (var0 & 8192) { s.store_object_field("send_as", static_cast<const BaseObject *>(send_as_.get())); }
    if (var0 & 131072) { s.store_object_field("quick_reply_shortcut", static_cast<const BaseObject *>(quick_reply_shortcut_.get())); }
    if (var0 & 262144) { s.store_field("effect", effect_); }
    if (var0 & 2097152) { s.store_field("allow_paid_stars", allow_paid_stars_); }
    s.store_class_end();
  }
}

messages_sendMultiMedia::ReturnType messages_sendMultiMedia::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_sendQuickReplyMessages::messages_sendQuickReplyMessages(object_ptr<InputPeer> &&peer_, int32 shortcut_id_, array<int32> &&id_, array<int64> &&random_id_)
  : peer_(std::move(peer_))
  , shortcut_id_(shortcut_id_)
  , id_(std::move(id_))
  , random_id_(std::move(random_id_))
{}

const std::int32_t messages_sendQuickReplyMessages::ID;

void messages_sendQuickReplyMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1819610593);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_id_, s);
}

void messages_sendQuickReplyMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1819610593);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(shortcut_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_id_, s);
}

void messages_sendQuickReplyMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendQuickReplyMessages");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("shortcut_id", shortcut_id_);
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("random_id", random_id_.size()); for (const auto &_value : random_id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_sendQuickReplyMessages::ReturnType messages_sendQuickReplyMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_sendReaction::messages_sendReaction(int32 flags_, bool big_, bool add_to_recent_, object_ptr<InputPeer> &&peer_, int32 msg_id_, array<object_ptr<Reaction>> &&reaction_)
  : flags_(flags_)
  , big_(big_)
  , add_to_recent_(add_to_recent_)
  , peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , reaction_(std::move(reaction_))
{}

const std::int32_t messages_sendReaction::ID;

void messages_sendReaction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-754091820);
  TlStoreBinary::store((var0 = flags_ | (big_ << 1) | (add_to_recent_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(reaction_, s); }
}

void messages_sendReaction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-754091820);
  TlStoreBinary::store((var0 = flags_ | (big_ << 1) | (add_to_recent_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(reaction_, s); }
}

void messages_sendReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendReaction");
    s.store_field("flags", (var0 = flags_ | (big_ << 1) | (add_to_recent_ << 2)));
    if (var0 & 2) { s.store_field("big", true); }
    if (var0 & 4) { s.store_field("add_to_recent", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    if (var0 & 1) { { s.store_vector_begin("reaction", reaction_.size()); for (const auto &_value : reaction_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

messages_sendReaction::ReturnType messages_sendReaction::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_sendWebViewResultMessage::messages_sendWebViewResultMessage(string const &bot_query_id_, object_ptr<InputBotInlineResult> &&result_)
  : bot_query_id_(bot_query_id_)
  , result_(std::move(result_))
{}

const std::int32_t messages_sendWebViewResultMessage::ID;

void messages_sendWebViewResultMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(172168437);
  TlStoreString::store(bot_query_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(result_, s);
}

void messages_sendWebViewResultMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(172168437);
  TlStoreString::store(bot_query_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(result_, s);
}

void messages_sendWebViewResultMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendWebViewResultMessage");
    s.store_field("bot_query_id", bot_query_id_);
    s.store_object_field("result", static_cast<const BaseObject *>(result_.get()));
    s.store_class_end();
  }
}

messages_sendWebViewResultMessage::ReturnType messages_sendWebViewResultMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<webViewMessageSent>, 211046684>::parse(p);
#undef FAIL
}

messages_setChatAvailableReactions::messages_setChatAvailableReactions(int32 flags_, object_ptr<InputPeer> &&peer_, object_ptr<ChatReactions> &&available_reactions_, int32 reactions_limit_, bool paid_enabled_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , available_reactions_(std::move(available_reactions_))
  , reactions_limit_(reactions_limit_)
  , paid_enabled_(paid_enabled_)
{}

const std::int32_t messages_setChatAvailableReactions::ID;

void messages_setChatAvailableReactions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2041895551);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(available_reactions_, s);
  if (var0 & 1) { TlStoreBinary::store(reactions_limit_, s); }
  if (var0 & 2) { TlStoreBool::store(paid_enabled_, s); }
}

void messages_setChatAvailableReactions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2041895551);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(available_reactions_, s);
  if (var0 & 1) { TlStoreBinary::store(reactions_limit_, s); }
  if (var0 & 2) { TlStoreBool::store(paid_enabled_, s); }
}

void messages_setChatAvailableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setChatAvailableReactions");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("available_reactions", static_cast<const BaseObject *>(available_reactions_.get()));
    if (var0 & 1) { s.store_field("reactions_limit", reactions_limit_); }
    if (var0 & 2) { s.store_field("paid_enabled", paid_enabled_); }
    s.store_class_end();
  }
}

messages_setChatAvailableReactions::ReturnType messages_setChatAvailableReactions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_setChatTheme::messages_setChatTheme(object_ptr<InputPeer> &&peer_, object_ptr<InputChatTheme> &&theme_)
  : peer_(std::move(peer_))
  , theme_(std::move(theme_))
{}

const std::int32_t messages_setChatTheme::ID;

void messages_setChatTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(135398089);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
}

void messages_setChatTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(135398089);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(theme_, s);
}

void messages_setChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setChatTheme");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

messages_setChatTheme::ReturnType messages_setChatTheme::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_toggleNoForwards::messages_toggleNoForwards(object_ptr<InputPeer> &&peer_, bool enabled_)
  : peer_(std::move(peer_))
  , enabled_(enabled_)
{}

const std::int32_t messages_toggleNoForwards::ID;

void messages_toggleNoForwards::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1323389022);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(enabled_, s);
}

void messages_toggleNoForwards::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1323389022);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(enabled_, s);
}

void messages_toggleNoForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleNoForwards");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

messages_toggleNoForwards::ReturnType messages_toggleNoForwards::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_uninstallStickerSet::messages_uninstallStickerSet(object_ptr<InputStickerSet> &&stickerset_)
  : stickerset_(std::move(stickerset_))
{}

const std::int32_t messages_uninstallStickerSet::ID;

void messages_uninstallStickerSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-110209570);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void messages_uninstallStickerSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-110209570);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void messages_uninstallStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.uninstallStickerSet");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_class_end();
  }
}

messages_uninstallStickerSet::ReturnType messages_uninstallStickerSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_clearSavedInfo::payments_clearSavedInfo(int32 flags_, bool credentials_, bool info_)
  : flags_(flags_)
  , credentials_(credentials_)
  , info_(info_)
{}

const std::int32_t payments_clearSavedInfo::ID;

void payments_clearSavedInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-667062079);
  TlStoreBinary::store((var0 = flags_ | (credentials_ << 0) | (info_ << 1)), s);
}

void payments_clearSavedInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-667062079);
  TlStoreBinary::store((var0 = flags_ | (credentials_ << 0) | (info_ << 1)), s);
}

void payments_clearSavedInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.clearSavedInfo");
    s.store_field("flags", (var0 = flags_ | (credentials_ << 0) | (info_ << 1)));
    if (var0 & 1) { s.store_field("credentials", true); }
    if (var0 & 2) { s.store_field("info", true); }
    s.store_class_end();
  }
}

payments_clearSavedInfo::ReturnType payments_clearSavedInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_getPaymentForm::payments_getPaymentForm(int32 flags_, object_ptr<InputInvoice> &&invoice_, object_ptr<dataJSON> &&theme_params_)
  : flags_(flags_)
  , invoice_(std::move(invoice_))
  , theme_params_(std::move(theme_params_))
{}

const std::int32_t payments_getPaymentForm::ID;

void payments_getPaymentForm::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(924093883);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
}

void payments_getPaymentForm::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(924093883);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, 2104790276>::store(theme_params_, s); }
}

void payments_getPaymentForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getPaymentForm");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    if (var0 & 1) { s.store_object_field("theme_params", static_cast<const BaseObject *>(theme_params_.get())); }
    s.store_class_end();
  }
}

payments_getPaymentForm::ReturnType payments_getPaymentForm::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_PaymentForm>::parse(p);
#undef FAIL
}

payments_getSavedStarGifts::payments_getSavedStarGifts(int32 flags_, bool exclude_unsaved_, bool exclude_saved_, bool exclude_unlimited_, bool exclude_unique_, bool sort_by_value_, bool exclude_upgradable_, bool exclude_unupgradable_, bool peer_color_available_, bool exclude_hosted_, object_ptr<InputPeer> &&peer_, int32 collection_id_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , exclude_unsaved_(exclude_unsaved_)
  , exclude_saved_(exclude_saved_)
  , exclude_unlimited_(exclude_unlimited_)
  , exclude_unique_(exclude_unique_)
  , sort_by_value_(sort_by_value_)
  , exclude_upgradable_(exclude_upgradable_)
  , exclude_unupgradable_(exclude_unupgradable_)
  , peer_color_available_(peer_color_available_)
  , exclude_hosted_(exclude_hosted_)
  , peer_(std::move(peer_))
  , collection_id_(collection_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t payments_getSavedStarGifts::ID;

void payments_getSavedStarGifts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1558583959);
  TlStoreBinary::store((var0 = flags_ | (exclude_unsaved_ << 0) | (exclude_saved_ << 1) | (exclude_unlimited_ << 2) | (exclude_unique_ << 4) | (sort_by_value_ << 5) | (exclude_upgradable_ << 7) | (exclude_unupgradable_ << 8) | (peer_color_available_ << 9) | (exclude_hosted_ << 10)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 64) { TlStoreBinary::store(collection_id_, s); }
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getSavedStarGifts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1558583959);
  TlStoreBinary::store((var0 = flags_ | (exclude_unsaved_ << 0) | (exclude_saved_ << 1) | (exclude_unlimited_ << 2) | (exclude_unique_ << 4) | (sort_by_value_ << 5) | (exclude_upgradable_ << 7) | (exclude_unupgradable_ << 8) | (peer_color_available_ << 9) | (exclude_hosted_ << 10)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 64) { TlStoreBinary::store(collection_id_, s); }
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void payments_getSavedStarGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getSavedStarGifts");
    s.store_field("flags", (var0 = flags_ | (exclude_unsaved_ << 0) | (exclude_saved_ << 1) | (exclude_unlimited_ << 2) | (exclude_unique_ << 4) | (sort_by_value_ << 5) | (exclude_upgradable_ << 7) | (exclude_unupgradable_ << 8) | (peer_color_available_ << 9) | (exclude_hosted_ << 10)));
    if (var0 & 1) { s.store_field("exclude_unsaved", true); }
    if (var0 & 2) { s.store_field("exclude_saved", true); }
    if (var0 & 4) { s.store_field("exclude_unlimited", true); }
    if (var0 & 16) { s.store_field("exclude_unique", true); }
    if (var0 & 32) { s.store_field("sort_by_value", true); }
    if (var0 & 128) { s.store_field("exclude_upgradable", true); }
    if (var0 & 256) { s.store_field("exclude_unupgradable", true); }
    if (var0 & 512) { s.store_field("peer_color_available", true); }
    if (var0 & 1024) { s.store_field("exclude_hosted", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 64) { s.store_field("collection_id", collection_id_); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

payments_getSavedStarGifts::ReturnType payments_getSavedStarGifts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_savedStarGifts>, -1779201615>::parse(p);
#undef FAIL
}

payments_sendPaymentForm::payments_sendPaymentForm(int32 flags_, int64 form_id_, object_ptr<InputInvoice> &&invoice_, string const &requested_info_id_, string const &shipping_option_id_, object_ptr<InputPaymentCredentials> &&credentials_, int64 tip_amount_)
  : flags_(flags_)
  , form_id_(form_id_)
  , invoice_(std::move(invoice_))
  , requested_info_id_(requested_info_id_)
  , shipping_option_id_(shipping_option_id_)
  , credentials_(std::move(credentials_))
  , tip_amount_(tip_amount_)
{}

const std::int32_t payments_sendPaymentForm::ID;

void payments_sendPaymentForm::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(755192367);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(form_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
  if (var0 & 1) { TlStoreString::store(requested_info_id_, s); }
  if (var0 & 2) { TlStoreString::store(shipping_option_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(credentials_, s);
  if (var0 & 4) { TlStoreBinary::store(tip_amount_, s); }
}

void payments_sendPaymentForm::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(755192367);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(form_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(invoice_, s);
  if (var0 & 1) { TlStoreString::store(requested_info_id_, s); }
  if (var0 & 2) { TlStoreString::store(shipping_option_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(credentials_, s);
  if (var0 & 4) { TlStoreBinary::store(tip_amount_, s); }
}

void payments_sendPaymentForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.sendPaymentForm");
    s.store_field("flags", (var0 = flags_));
    s.store_field("form_id", form_id_);
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    if (var0 & 1) { s.store_field("requested_info_id", requested_info_id_); }
    if (var0 & 2) { s.store_field("shipping_option_id", shipping_option_id_); }
    s.store_object_field("credentials", static_cast<const BaseObject *>(credentials_.get()));
    if (var0 & 4) { s.store_field("tip_amount", tip_amount_); }
    s.store_class_end();
  }
}

payments_sendPaymentForm::ReturnType payments_sendPaymentForm::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<payments_PaymentResult>::parse(p);
#undef FAIL
}

payments_upgradeStarGift::payments_upgradeStarGift(int32 flags_, bool keep_original_details_, object_ptr<InputSavedStarGift> &&stargift_)
  : flags_(flags_)
  , keep_original_details_(keep_original_details_)
  , stargift_(std::move(stargift_))
{}

const std::int32_t payments_upgradeStarGift::ID;

void payments_upgradeStarGift::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1361648395);
  TlStoreBinary::store((var0 = flags_ | (keep_original_details_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void payments_upgradeStarGift::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1361648395);
  TlStoreBinary::store((var0 = flags_ | (keep_original_details_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
}

void payments_upgradeStarGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.upgradeStarGift");
    s.store_field("flags", (var0 = flags_ | (keep_original_details_ << 0)));
    if (var0 & 1) { s.store_field("keep_original_details", true); }
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_class_end();
  }
}

payments_upgradeStarGift::ReturnType payments_upgradeStarGift::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_checkGroupCall::phone_checkGroupCall(object_ptr<InputGroupCall> &&call_, array<int32> &&sources_)
  : call_(std::move(call_))
  , sources_(std::move(sources_))
{}

const std::int32_t phone_checkGroupCall::ID;

void phone_checkGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1248003721);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(sources_, s);
}

void phone_checkGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1248003721);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(sources_, s);
}

void phone_checkGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.checkGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    { s.store_vector_begin("sources", sources_.size()); for (const auto &_value : sources_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

phone_checkGroupCall::ReturnType phone_checkGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
#undef FAIL
}

phone_createGroupCall::phone_createGroupCall(int32 flags_, bool rtmp_stream_, object_ptr<InputPeer> &&peer_, int32 random_id_, string const &title_, int32 schedule_date_)
  : flags_(flags_)
  , rtmp_stream_(rtmp_stream_)
  , peer_(std::move(peer_))
  , random_id_(random_id_)
  , title_(title_)
  , schedule_date_(schedule_date_)
{}

const std::int32_t phone_createGroupCall::ID;

void phone_createGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1221445336);
  TlStoreBinary::store((var0 = flags_ | (rtmp_stream_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBinary::store(schedule_date_, s); }
}

void phone_createGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1221445336);
  TlStoreBinary::store((var0 = flags_ | (rtmp_stream_ << 2)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(random_id_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreBinary::store(schedule_date_, s); }
}

void phone_createGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.createGroupCall");
    s.store_field("flags", (var0 = flags_ | (rtmp_stream_ << 2)));
    if (var0 & 4) { s.store_field("rtmp_stream", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("random_id", random_id_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("schedule_date", schedule_date_); }
    s.store_class_end();
  }
}

phone_createGroupCall::ReturnType phone_createGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_joinGroupCallPresentation::phone_joinGroupCallPresentation(object_ptr<InputGroupCall> &&call_, object_ptr<dataJSON> &&params_)
  : call_(std::move(call_))
  , params_(std::move(params_))
{}

const std::int32_t phone_joinGroupCallPresentation::ID;

void phone_joinGroupCallPresentation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-873829436);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void phone_joinGroupCallPresentation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-873829436);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreObject, 2104790276>::store(params_, s);
}

void phone_joinGroupCallPresentation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.joinGroupCallPresentation");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_object_field("params", static_cast<const BaseObject *>(params_.get()));
    s.store_class_end();
  }
}

phone_joinGroupCallPresentation::ReturnType phone_joinGroupCallPresentation::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

phone_requestCall::phone_requestCall(int32 flags_, bool video_, object_ptr<InputUser> &&user_id_, int32 random_id_, bytes &&g_a_hash_, object_ptr<phoneCallProtocol> &&protocol_)
  : flags_(flags_)
  , video_(video_)
  , user_id_(std::move(user_id_))
  , random_id_(random_id_)
  , g_a_hash_(std::move(g_a_hash_))
  , protocol_(std::move(protocol_))
{}

const std::int32_t phone_requestCall::ID;

void phone_requestCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1124046573);
  TlStoreBinary::store((var0 = flags_ | (video_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(g_a_hash_, s);
  TlStoreBoxed<TlStoreObject, -58224696>::store(protocol_, s);
}

void phone_requestCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1124046573);
  TlStoreBinary::store((var0 = flags_ | (video_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(g_a_hash_, s);
  TlStoreBoxed<TlStoreObject, -58224696>::store(protocol_, s);
}

void phone_requestCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.requestCall");
    s.store_field("flags", (var0 = flags_ | (video_ << 0)));
    if (var0 & 1) { s.store_field("video", true); }
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    s.store_field("random_id", random_id_);
    s.store_bytes_field("g_a_hash", g_a_hash_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    s.store_class_end();
  }
}

phone_requestCall::ReturnType phone_requestCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_phoneCall>, -326966976>::parse(p);
#undef FAIL
}

photos_uploadContactProfilePhoto::photos_uploadContactProfilePhoto(int32 flags_, bool suggest_, bool save_, object_ptr<InputUser> &&user_id_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&video_, double video_start_ts_, object_ptr<VideoSize> &&video_emoji_markup_)
  : flags_(flags_)
  , suggest_(suggest_)
  , save_(save_)
  , user_id_(std::move(user_id_))
  , file_(std::move(file_))
  , video_(std::move(video_))
  , video_start_ts_(video_start_ts_)
  , video_emoji_markup_(std::move(video_emoji_markup_))
{}

const std::int32_t photos_uploadContactProfilePhoto::ID;

void photos_uploadContactProfilePhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-515093903);
  TlStoreBinary::store((var0 = flags_ | (suggest_ << 3) | (save_ << 4)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(file_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(video_, s); }
  if (var0 & 4) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(video_emoji_markup_, s); }
}

void photos_uploadContactProfilePhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-515093903);
  TlStoreBinary::store((var0 = flags_ | (suggest_ << 3) | (save_ << 4)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(user_id_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(file_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(video_, s); }
  if (var0 & 4) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(video_emoji_markup_, s); }
}

void photos_uploadContactProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photos.uploadContactProfilePhoto");
    s.store_field("flags", (var0 = flags_ | (suggest_ << 3) | (save_ << 4)));
    if (var0 & 8) { s.store_field("suggest", true); }
    if (var0 & 16) { s.store_field("save", true); }
    s.store_object_field("user_id", static_cast<const BaseObject *>(user_id_.get()));
    if (var0 & 1) { s.store_object_field("file", static_cast<const BaseObject *>(file_.get())); }
    if (var0 & 2) { s.store_object_field("video", static_cast<const BaseObject *>(video_.get())); }
    if (var0 & 4) { s.store_field("video_start_ts", video_start_ts_); }
    if (var0 & 32) { s.store_object_field("video_emoji_markup", static_cast<const BaseObject *>(video_emoji_markup_.get())); }
    s.store_class_end();
  }
}

photos_uploadContactProfilePhoto::ReturnType photos_uploadContactProfilePhoto::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<photos_photo>, 539045032>::parse(p);
#undef FAIL
}

premium_getBoostsList::premium_getBoostsList(int32 flags_, bool gifts_, object_ptr<InputPeer> &&peer_, string const &offset_, int32 limit_)
  : flags_(flags_)
  , gifts_(gifts_)
  , peer_(std::move(peer_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t premium_getBoostsList::ID;

void premium_getBoostsList::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1626764896);
  TlStoreBinary::store((var0 = flags_ | (gifts_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void premium_getBoostsList::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1626764896);
  TlStoreBinary::store((var0 = flags_ | (gifts_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void premium_getBoostsList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.getBoostsList");
    s.store_field("flags", (var0 = flags_ | (gifts_ << 0)));
    if (var0 & 1) { s.store_field("gifts", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

premium_getBoostsList::ReturnType premium_getBoostsList::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<premium_boostsList>, -2030542532>::parse(p);
#undef FAIL
}

smsjobs_finishJob::smsjobs_finishJob(int32 flags_, string const &job_id_, string const &error_)
  : flags_(flags_)
  , job_id_(job_id_)
  , error_(error_)
{}

const std::int32_t smsjobs_finishJob::ID;

void smsjobs_finishJob::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1327415076);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(job_id_, s);
  if (var0 & 1) { TlStoreString::store(error_, s); }
}

void smsjobs_finishJob::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1327415076);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(job_id_, s);
  if (var0 & 1) { TlStoreString::store(error_, s); }
}

void smsjobs_finishJob::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.finishJob");
    s.store_field("flags", (var0 = flags_));
    s.store_field("job_id", job_id_);
    if (var0 & 1) { s.store_field("error", error_); }
    s.store_class_end();
  }
}

smsjobs_finishJob::ReturnType smsjobs_finishJob::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

smsjobs_getSmsJob::smsjobs_getSmsJob(string const &job_id_)
  : job_id_(job_id_)
{}

const std::int32_t smsjobs_getSmsJob::ID;

void smsjobs_getSmsJob::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2005766191);
  TlStoreString::store(job_id_, s);
}

void smsjobs_getSmsJob::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2005766191);
  TlStoreString::store(job_id_, s);
}

void smsjobs_getSmsJob::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.getSmsJob");
    s.store_field("job_id", job_id_);
    s.store_class_end();
  }
}

smsjobs_getSmsJob::ReturnType smsjobs_getSmsJob::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<smsJob>, -425595208>::parse(p);
#undef FAIL
}

stickers_suggestShortName::stickers_suggestShortName(string const &title_)
  : title_(title_)
{}

const std::int32_t stickers_suggestShortName::ID;

void stickers_suggestShortName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1303364867);
  TlStoreString::store(title_, s);
}

void stickers_suggestShortName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1303364867);
  TlStoreString::store(title_, s);
}

void stickers_suggestShortName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.suggestShortName");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

stickers_suggestShortName::ReturnType stickers_suggestShortName::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stickers_suggestedShortName>, -2046910401>::parse(p);
#undef FAIL
}

const std::int32_t stories_getAllReadPeerStories::ID;

void stories_getAllReadPeerStories::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1688541191);
}

void stories_getAllReadPeerStories::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1688541191);
}

void stories_getAllReadPeerStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getAllReadPeerStories");
    s.store_class_end();
  }
}

stories_getAllReadPeerStories::ReturnType stories_getAllReadPeerStories::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

upload_getFile::upload_getFile(int32 flags_, bool precise_, bool cdn_supported_, object_ptr<InputFileLocation> &&location_, int64 offset_, int32 limit_)
  : flags_(flags_)
  , precise_(precise_)
  , cdn_supported_(cdn_supported_)
  , location_(std::move(location_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t upload_getFile::ID;

void upload_getFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1101843010);
  TlStoreBinary::store((var0 = flags_ | (precise_ << 0) | (cdn_supported_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void upload_getFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1101843010);
  TlStoreBinary::store((var0 = flags_ | (precise_ << 0) | (cdn_supported_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void upload_getFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.getFile");
    s.store_field("flags", (var0 = flags_ | (precise_ << 0) | (cdn_supported_ << 1)));
    if (var0 & 1) { s.store_field("precise", true); }
    if (var0 & 2) { s.store_field("cdn_supported", true); }
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

upload_getFile::ReturnType upload_getFile::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<upload_File>::parse(p);
#undef FAIL
}

upload_getWebFile::upload_getWebFile(object_ptr<InputWebFileLocation> &&location_, int32 offset_, int32 limit_)
  : location_(std::move(location_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t upload_getWebFile::ID;

void upload_getWebFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(619086221);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void upload_getWebFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(619086221);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void upload_getWebFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.getWebFile");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

upload_getWebFile::ReturnType upload_getWebFile::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<upload_webFile>, 568808380>::parse(p);
#undef FAIL
}

upload_saveFilePart::upload_saveFilePart(int64 file_id_, int32 file_part_, bytes &&bytes_)
  : file_id_(file_id_)
  , file_part_(file_part_)
  , bytes_(std::move(bytes_))
{}

const std::int32_t upload_saveFilePart::ID;

void upload_saveFilePart::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1291540959);
  TlStoreBinary::store(file_id_, s);
  TlStoreBinary::store(file_part_, s);
  TlStoreString::store(bytes_, s);
}

void upload_saveFilePart::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1291540959);
  TlStoreBinary::store(file_id_, s);
  TlStoreBinary::store(file_part_, s);
  TlStoreString::store(bytes_, s);
}

void upload_saveFilePart::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.saveFilePart");
    s.store_field("file_id", file_id_);
    s.store_field("file_part", file_part_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

upload_saveFilePart::ReturnType upload_saveFilePart::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

users_suggestBirthday::users_suggestBirthday(object_ptr<InputUser> &&id_, object_ptr<birthday> &&birthday_)
  : id_(std::move(id_))
  , birthday_(std::move(birthday_))
{}

const std::int32_t users_suggestBirthday::ID;

void users_suggestBirthday::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-61656206);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1821253126>::store(birthday_, s);
}

void users_suggestBirthday::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-61656206);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1821253126>::store(birthday_, s);
}

void users_suggestBirthday::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users.suggestBirthday");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_object_field("birthday", static_cast<const BaseObject *>(birthday_.get()));
    s.store_class_end();
  }
}

users_suggestBirthday::ReturnType users_suggestBirthday::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
