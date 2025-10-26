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


object_ptr<BotApp> BotApp::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case botAppNotModified::ID:
      return botAppNotModified::fetch(p);
    case botApp::ID:
      return botApp::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t botAppNotModified::ID;

object_ptr<BotApp> botAppNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<botAppNotModified>();
}

void botAppNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botAppNotModified");
    s.store_class_end();
  }
}

botApp::botApp()
  : flags_()
  , id_()
  , access_hash_()
  , short_name_()
  , title_()
  , description_()
  , photo_()
  , document_()
  , hash_()
{}

const std::int32_t botApp::ID;

object_ptr<BotApp> botApp::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<botApp> res = make_tl_object<botApp>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->short_name_ = TlFetchString<string>::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->description_ = TlFetchString<string>::parse(p);
  res->photo_ = TlFetchObject<Photo>::parse(p);
  if (var0 & 1) { res->document_ = TlFetchObject<Document>::parse(p); }
  res->hash_ = TlFetchLong::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void botApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botApp");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("short_name", short_name_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    if (var0 & 1) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

const std::int32_t businessGreetingMessage::ID;

object_ptr<businessGreetingMessage> businessGreetingMessage::fetch(TlBufferParser &p) {
  return make_tl_object<businessGreetingMessage>(p);
}

businessGreetingMessage::businessGreetingMessage(TlBufferParser &p)
  : shortcut_id_(TlFetchInt::parse(p))
  , recipients_(TlFetchBoxed<TlFetchObject<businessRecipients>, 554733559>::parse(p))
  , no_activity_days_(TlFetchInt::parse(p))
{}

void businessGreetingMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessGreetingMessage");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_field("no_activity_days", no_activity_days_);
    s.store_class_end();
  }
}

businessRecipients::businessRecipients()
  : flags_()
  , existing_chats_()
  , new_chats_()
  , contacts_()
  , non_contacts_()
  , exclude_selected_()
  , users_()
{}

const std::int32_t businessRecipients::ID;

object_ptr<businessRecipients> businessRecipients::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<businessRecipients> res = make_tl_object<businessRecipients>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->existing_chats_ = (var0 & 1) != 0;
  res->new_chats_ = (var0 & 2) != 0;
  res->contacts_ = (var0 & 4) != 0;
  res->non_contacts_ = (var0 & 8) != 0;
  res->exclude_selected_ = (var0 & 32) != 0;
  if (var0 & 16) { res->users_ = TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void businessRecipients::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessRecipients");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (existing_chats_ << 0) | (new_chats_ << 1) | (contacts_ << 2) | (non_contacts_ << 3) | (exclude_selected_ << 5)));
    if (var0 & 1) { s.store_field("existing_chats", true); }
    if (var0 & 2) { s.store_field("new_chats", true); }
    if (var0 & 4) { s.store_field("contacts", true); }
    if (var0 & 8) { s.store_field("non_contacts", true); }
    if (var0 & 32) { s.store_field("exclude_selected", true); }
    if (var0 & 16) { { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<ChannelAdminLogEventAction> ChannelAdminLogEventAction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t channelAdminLogEventActionChangeTitle::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeTitle::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeTitle>(p);
}

channelAdminLogEventActionChangeTitle::channelAdminLogEventActionChangeTitle(TlBufferParser &p)
  : prev_value_(TlFetchString<string>::parse(p))
  , new_value_(TlFetchString<string>::parse(p))
{}

void channelAdminLogEventActionChangeTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeTitle");
    s.store_field("prev_value", prev_value_);
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeAbout::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeAbout::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeAbout>(p);
}

channelAdminLogEventActionChangeAbout::channelAdminLogEventActionChangeAbout(TlBufferParser &p)
  : prev_value_(TlFetchString<string>::parse(p))
  , new_value_(TlFetchString<string>::parse(p))
{}

void channelAdminLogEventActionChangeAbout::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeAbout");
    s.store_field("prev_value", prev_value_);
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeUsername::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeUsername::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeUsername>(p);
}

channelAdminLogEventActionChangeUsername::channelAdminLogEventActionChangeUsername(TlBufferParser &p)
  : prev_value_(TlFetchString<string>::parse(p))
  , new_value_(TlFetchString<string>::parse(p))
{}

void channelAdminLogEventActionChangeUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeUsername");
    s.store_field("prev_value", prev_value_);
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangePhoto::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangePhoto::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangePhoto>(p);
}

channelAdminLogEventActionChangePhoto::channelAdminLogEventActionChangePhoto(TlBufferParser &p)
  : prev_photo_(TlFetchObject<Photo>::parse(p))
  , new_photo_(TlFetchObject<Photo>::parse(p))
{}

void channelAdminLogEventActionChangePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangePhoto");
    s.store_object_field("prev_photo", static_cast<const BaseObject *>(prev_photo_.get()));
    s.store_object_field("new_photo", static_cast<const BaseObject *>(new_photo_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleInvites::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleInvites::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleInvites>(p);
}

channelAdminLogEventActionToggleInvites::channelAdminLogEventActionToggleInvites(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleInvites");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleSignatures::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleSignatures::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleSignatures>(p);
}

channelAdminLogEventActionToggleSignatures::channelAdminLogEventActionToggleSignatures(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleSignatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleSignatures");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionUpdatePinned::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionUpdatePinned::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionUpdatePinned>(p);
}

channelAdminLogEventActionUpdatePinned::channelAdminLogEventActionUpdatePinned(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void channelAdminLogEventActionUpdatePinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionUpdatePinned");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionEditMessage::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionEditMessage::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionEditMessage>(p);
}

channelAdminLogEventActionEditMessage::channelAdminLogEventActionEditMessage(TlBufferParser &p)
  : prev_message_(TlFetchObject<Message>::parse(p))
  , new_message_(TlFetchObject<Message>::parse(p))
{}

void channelAdminLogEventActionEditMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionEditMessage");
    s.store_object_field("prev_message", static_cast<const BaseObject *>(prev_message_.get()));
    s.store_object_field("new_message", static_cast<const BaseObject *>(new_message_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionDeleteMessage::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionDeleteMessage::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionDeleteMessage>(p);
}

channelAdminLogEventActionDeleteMessage::channelAdminLogEventActionDeleteMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void channelAdminLogEventActionDeleteMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionDeleteMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantJoin::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantJoin::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantJoin>();
}

void channelAdminLogEventActionParticipantJoin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantJoin");
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantLeave::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantLeave::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantLeave>();
}

void channelAdminLogEventActionParticipantLeave::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantLeave");
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantInvite::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantInvite::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantInvite>(p);
}

channelAdminLogEventActionParticipantInvite::channelAdminLogEventActionParticipantInvite(TlBufferParser &p)
  : participant_(TlFetchObject<ChannelParticipant>::parse(p))
{}

void channelAdminLogEventActionParticipantInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantInvite");
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantToggleBan::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantToggleBan::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantToggleBan>(p);
}

channelAdminLogEventActionParticipantToggleBan::channelAdminLogEventActionParticipantToggleBan(TlBufferParser &p)
  : prev_participant_(TlFetchObject<ChannelParticipant>::parse(p))
  , new_participant_(TlFetchObject<ChannelParticipant>::parse(p))
{}

void channelAdminLogEventActionParticipantToggleBan::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantToggleBan");
    s.store_object_field("prev_participant", static_cast<const BaseObject *>(prev_participant_.get()));
    s.store_object_field("new_participant", static_cast<const BaseObject *>(new_participant_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantToggleAdmin::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantToggleAdmin::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantToggleAdmin>(p);
}

channelAdminLogEventActionParticipantToggleAdmin::channelAdminLogEventActionParticipantToggleAdmin(TlBufferParser &p)
  : prev_participant_(TlFetchObject<ChannelParticipant>::parse(p))
  , new_participant_(TlFetchObject<ChannelParticipant>::parse(p))
{}

void channelAdminLogEventActionParticipantToggleAdmin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantToggleAdmin");
    s.store_object_field("prev_participant", static_cast<const BaseObject *>(prev_participant_.get()));
    s.store_object_field("new_participant", static_cast<const BaseObject *>(new_participant_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeStickerSet::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeStickerSet::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeStickerSet>(p);
}

channelAdminLogEventActionChangeStickerSet::channelAdminLogEventActionChangeStickerSet(TlBufferParser &p)
  : prev_stickerset_(TlFetchObject<InputStickerSet>::parse(p))
  , new_stickerset_(TlFetchObject<InputStickerSet>::parse(p))
{}

void channelAdminLogEventActionChangeStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeStickerSet");
    s.store_object_field("prev_stickerset", static_cast<const BaseObject *>(prev_stickerset_.get()));
    s.store_object_field("new_stickerset", static_cast<const BaseObject *>(new_stickerset_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionTogglePreHistoryHidden::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionTogglePreHistoryHidden::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionTogglePreHistoryHidden>(p);
}

channelAdminLogEventActionTogglePreHistoryHidden::channelAdminLogEventActionTogglePreHistoryHidden(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionTogglePreHistoryHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionTogglePreHistoryHidden");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionDefaultBannedRights::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionDefaultBannedRights::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionDefaultBannedRights>(p);
}

channelAdminLogEventActionDefaultBannedRights::channelAdminLogEventActionDefaultBannedRights(TlBufferParser &p)
  : prev_banned_rights_(TlFetchBoxed<TlFetchObject<chatBannedRights>, -1626209256>::parse(p))
  , new_banned_rights_(TlFetchBoxed<TlFetchObject<chatBannedRights>, -1626209256>::parse(p))
{}

void channelAdminLogEventActionDefaultBannedRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionDefaultBannedRights");
    s.store_object_field("prev_banned_rights", static_cast<const BaseObject *>(prev_banned_rights_.get()));
    s.store_object_field("new_banned_rights", static_cast<const BaseObject *>(new_banned_rights_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionStopPoll::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionStopPoll::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionStopPoll>(p);
}

channelAdminLogEventActionStopPoll::channelAdminLogEventActionStopPoll(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void channelAdminLogEventActionStopPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionStopPoll");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeLinkedChat::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeLinkedChat::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeLinkedChat>(p);
}

channelAdminLogEventActionChangeLinkedChat::channelAdminLogEventActionChangeLinkedChat(TlBufferParser &p)
  : prev_value_(TlFetchLong::parse(p))
  , new_value_(TlFetchLong::parse(p))
{}

void channelAdminLogEventActionChangeLinkedChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeLinkedChat");
    s.store_field("prev_value", prev_value_);
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeLocation::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeLocation::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeLocation>(p);
}

channelAdminLogEventActionChangeLocation::channelAdminLogEventActionChangeLocation(TlBufferParser &p)
  : prev_value_(TlFetchObject<ChannelLocation>::parse(p))
  , new_value_(TlFetchObject<ChannelLocation>::parse(p))
{}

void channelAdminLogEventActionChangeLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeLocation");
    s.store_object_field("prev_value", static_cast<const BaseObject *>(prev_value_.get()));
    s.store_object_field("new_value", static_cast<const BaseObject *>(new_value_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleSlowMode::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleSlowMode::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleSlowMode>(p);
}

channelAdminLogEventActionToggleSlowMode::channelAdminLogEventActionToggleSlowMode(TlBufferParser &p)
  : prev_value_(TlFetchInt::parse(p))
  , new_value_(TlFetchInt::parse(p))
{}

void channelAdminLogEventActionToggleSlowMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleSlowMode");
    s.store_field("prev_value", prev_value_);
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionStartGroupCall::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionStartGroupCall::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionStartGroupCall>(p);
}

channelAdminLogEventActionStartGroupCall::channelAdminLogEventActionStartGroupCall(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
{}

void channelAdminLogEventActionStartGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionStartGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionDiscardGroupCall::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionDiscardGroupCall::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionDiscardGroupCall>(p);
}

channelAdminLogEventActionDiscardGroupCall::channelAdminLogEventActionDiscardGroupCall(TlBufferParser &p)
  : call_(TlFetchObject<InputGroupCall>::parse(p))
{}

void channelAdminLogEventActionDiscardGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionDiscardGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantMute::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantMute::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantMute>(p);
}

channelAdminLogEventActionParticipantMute::channelAdminLogEventActionParticipantMute(TlBufferParser &p)
  : participant_(TlFetchBoxed<TlFetchObject<groupCallParticipant>, -341428482>::parse(p))
{}

void channelAdminLogEventActionParticipantMute::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantMute");
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantUnmute::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantUnmute::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantUnmute>(p);
}

channelAdminLogEventActionParticipantUnmute::channelAdminLogEventActionParticipantUnmute(TlBufferParser &p)
  : participant_(TlFetchBoxed<TlFetchObject<groupCallParticipant>, -341428482>::parse(p))
{}

void channelAdminLogEventActionParticipantUnmute::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantUnmute");
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleGroupCallSetting::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleGroupCallSetting::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleGroupCallSetting>(p);
}

channelAdminLogEventActionToggleGroupCallSetting::channelAdminLogEventActionToggleGroupCallSetting(TlBufferParser &p)
  : join_muted_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleGroupCallSetting::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleGroupCallSetting");
    s.store_field("join_muted", join_muted_);
    s.store_class_end();
  }
}

channelAdminLogEventActionParticipantJoinByInvite::channelAdminLogEventActionParticipantJoinByInvite()
  : flags_()
  , via_chatlist_()
  , invite_()
{}

const std::int32_t channelAdminLogEventActionParticipantJoinByInvite::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantJoinByInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelAdminLogEventActionParticipantJoinByInvite> res = make_tl_object<channelAdminLogEventActionParticipantJoinByInvite>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->via_chatlist_ = (var0 & 1) != 0;
  res->invite_ = TlFetchObject<ExportedChatInvite>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelAdminLogEventActionParticipantJoinByInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantJoinByInvite");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (via_chatlist_ << 0)));
    if (var0 & 1) { s.store_field("via_chatlist", true); }
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionExportedInviteDelete::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionExportedInviteDelete::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionExportedInviteDelete>(p);
}

channelAdminLogEventActionExportedInviteDelete::channelAdminLogEventActionExportedInviteDelete(TlBufferParser &p)
  : invite_(TlFetchObject<ExportedChatInvite>::parse(p))
{}

void channelAdminLogEventActionExportedInviteDelete::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionExportedInviteDelete");
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionExportedInviteRevoke::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionExportedInviteRevoke::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionExportedInviteRevoke>(p);
}

channelAdminLogEventActionExportedInviteRevoke::channelAdminLogEventActionExportedInviteRevoke(TlBufferParser &p)
  : invite_(TlFetchObject<ExportedChatInvite>::parse(p))
{}

void channelAdminLogEventActionExportedInviteRevoke::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionExportedInviteRevoke");
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionExportedInviteEdit::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionExportedInviteEdit::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionExportedInviteEdit>(p);
}

channelAdminLogEventActionExportedInviteEdit::channelAdminLogEventActionExportedInviteEdit(TlBufferParser &p)
  : prev_invite_(TlFetchObject<ExportedChatInvite>::parse(p))
  , new_invite_(TlFetchObject<ExportedChatInvite>::parse(p))
{}

void channelAdminLogEventActionExportedInviteEdit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionExportedInviteEdit");
    s.store_object_field("prev_invite", static_cast<const BaseObject *>(prev_invite_.get()));
    s.store_object_field("new_invite", static_cast<const BaseObject *>(new_invite_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantVolume::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantVolume::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantVolume>(p);
}

channelAdminLogEventActionParticipantVolume::channelAdminLogEventActionParticipantVolume(TlBufferParser &p)
  : participant_(TlFetchBoxed<TlFetchObject<groupCallParticipant>, -341428482>::parse(p))
{}

void channelAdminLogEventActionParticipantVolume::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantVolume");
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeHistoryTTL::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeHistoryTTL::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeHistoryTTL>(p);
}

channelAdminLogEventActionChangeHistoryTTL::channelAdminLogEventActionChangeHistoryTTL(TlBufferParser &p)
  : prev_value_(TlFetchInt::parse(p))
  , new_value_(TlFetchInt::parse(p))
{}

void channelAdminLogEventActionChangeHistoryTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeHistoryTTL");
    s.store_field("prev_value", prev_value_);
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantJoinByRequest::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantJoinByRequest::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantJoinByRequest>(p);
}

channelAdminLogEventActionParticipantJoinByRequest::channelAdminLogEventActionParticipantJoinByRequest(TlBufferParser &p)
  : invite_(TlFetchObject<ExportedChatInvite>::parse(p))
  , approved_by_(TlFetchLong::parse(p))
{}

void channelAdminLogEventActionParticipantJoinByRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantJoinByRequest");
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    s.store_field("approved_by", approved_by_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleNoForwards::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleNoForwards::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleNoForwards>(p);
}

channelAdminLogEventActionToggleNoForwards::channelAdminLogEventActionToggleNoForwards(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleNoForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleNoForwards");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionSendMessage::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionSendMessage::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionSendMessage>(p);
}

channelAdminLogEventActionSendMessage::channelAdminLogEventActionSendMessage(TlBufferParser &p)
  : message_(TlFetchObject<Message>::parse(p))
{}

void channelAdminLogEventActionSendMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionSendMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeAvailableReactions::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeAvailableReactions::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeAvailableReactions>(p);
}

channelAdminLogEventActionChangeAvailableReactions::channelAdminLogEventActionChangeAvailableReactions(TlBufferParser &p)
  : prev_value_(TlFetchObject<ChatReactions>::parse(p))
  , new_value_(TlFetchObject<ChatReactions>::parse(p))
{}

void channelAdminLogEventActionChangeAvailableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeAvailableReactions");
    s.store_object_field("prev_value", static_cast<const BaseObject *>(prev_value_.get()));
    s.store_object_field("new_value", static_cast<const BaseObject *>(new_value_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeUsernames::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeUsernames::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeUsernames>(p);
}

channelAdminLogEventActionChangeUsernames::channelAdminLogEventActionChangeUsernames(TlBufferParser &p)
  : prev_value_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
  , new_value_(TlFetchBoxed<TlFetchVector<TlFetchString<string>>, 481674261>::parse(p))
{}

void channelAdminLogEventActionChangeUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeUsernames");
    { s.store_vector_begin("prev_value", prev_value_.size()); for (const auto &_value : prev_value_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("new_value", new_value_.size()); for (const auto &_value : new_value_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleForum::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleForum::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleForum>(p);
}

channelAdminLogEventActionToggleForum::channelAdminLogEventActionToggleForum(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleForum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleForum");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionCreateTopic::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionCreateTopic::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionCreateTopic>(p);
}

channelAdminLogEventActionCreateTopic::channelAdminLogEventActionCreateTopic(TlBufferParser &p)
  : topic_(TlFetchObject<ForumTopic>::parse(p))
{}

void channelAdminLogEventActionCreateTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionCreateTopic");
    s.store_object_field("topic", static_cast<const BaseObject *>(topic_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionEditTopic::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionEditTopic::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionEditTopic>(p);
}

channelAdminLogEventActionEditTopic::channelAdminLogEventActionEditTopic(TlBufferParser &p)
  : prev_topic_(TlFetchObject<ForumTopic>::parse(p))
  , new_topic_(TlFetchObject<ForumTopic>::parse(p))
{}

void channelAdminLogEventActionEditTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionEditTopic");
    s.store_object_field("prev_topic", static_cast<const BaseObject *>(prev_topic_.get()));
    s.store_object_field("new_topic", static_cast<const BaseObject *>(new_topic_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionDeleteTopic::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionDeleteTopic::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionDeleteTopic>(p);
}

channelAdminLogEventActionDeleteTopic::channelAdminLogEventActionDeleteTopic(TlBufferParser &p)
  : topic_(TlFetchObject<ForumTopic>::parse(p))
{}

void channelAdminLogEventActionDeleteTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionDeleteTopic");
    s.store_object_field("topic", static_cast<const BaseObject *>(topic_.get()));
    s.store_class_end();
  }
}

channelAdminLogEventActionPinTopic::channelAdminLogEventActionPinTopic()
  : flags_()
  , prev_topic_()
  , new_topic_()
{}

const std::int32_t channelAdminLogEventActionPinTopic::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionPinTopic::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelAdminLogEventActionPinTopic> res = make_tl_object<channelAdminLogEventActionPinTopic>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->prev_topic_ = TlFetchObject<ForumTopic>::parse(p); }
  if (var0 & 2) { res->new_topic_ = TlFetchObject<ForumTopic>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelAdminLogEventActionPinTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionPinTopic");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("prev_topic", static_cast<const BaseObject *>(prev_topic_.get())); }
    if (var0 & 2) { s.store_object_field("new_topic", static_cast<const BaseObject *>(new_topic_.get())); }
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleAntiSpam::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleAntiSpam::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleAntiSpam>(p);
}

channelAdminLogEventActionToggleAntiSpam::channelAdminLogEventActionToggleAntiSpam(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleAntiSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleAntiSpam");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangePeerColor::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangePeerColor::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangePeerColor>(p);
}

channelAdminLogEventActionChangePeerColor::channelAdminLogEventActionChangePeerColor(TlBufferParser &p)
  : prev_value_(TlFetchObject<PeerColor>::parse(p))
  , new_value_(TlFetchObject<PeerColor>::parse(p))
{}

void channelAdminLogEventActionChangePeerColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangePeerColor");
    s.store_object_field("prev_value", static_cast<const BaseObject *>(prev_value_.get()));
    s.store_object_field("new_value", static_cast<const BaseObject *>(new_value_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeProfilePeerColor::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeProfilePeerColor::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeProfilePeerColor>(p);
}

channelAdminLogEventActionChangeProfilePeerColor::channelAdminLogEventActionChangeProfilePeerColor(TlBufferParser &p)
  : prev_value_(TlFetchObject<PeerColor>::parse(p))
  , new_value_(TlFetchObject<PeerColor>::parse(p))
{}

void channelAdminLogEventActionChangeProfilePeerColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeProfilePeerColor");
    s.store_object_field("prev_value", static_cast<const BaseObject *>(prev_value_.get()));
    s.store_object_field("new_value", static_cast<const BaseObject *>(new_value_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeWallpaper::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeWallpaper::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeWallpaper>(p);
}

channelAdminLogEventActionChangeWallpaper::channelAdminLogEventActionChangeWallpaper(TlBufferParser &p)
  : prev_value_(TlFetchObject<WallPaper>::parse(p))
  , new_value_(TlFetchObject<WallPaper>::parse(p))
{}

void channelAdminLogEventActionChangeWallpaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeWallpaper");
    s.store_object_field("prev_value", static_cast<const BaseObject *>(prev_value_.get()));
    s.store_object_field("new_value", static_cast<const BaseObject *>(new_value_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeEmojiStatus::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeEmojiStatus::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeEmojiStatus>(p);
}

channelAdminLogEventActionChangeEmojiStatus::channelAdminLogEventActionChangeEmojiStatus(TlBufferParser &p)
  : prev_value_(TlFetchObject<EmojiStatus>::parse(p))
  , new_value_(TlFetchObject<EmojiStatus>::parse(p))
{}

void channelAdminLogEventActionChangeEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeEmojiStatus");
    s.store_object_field("prev_value", static_cast<const BaseObject *>(prev_value_.get()));
    s.store_object_field("new_value", static_cast<const BaseObject *>(new_value_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionChangeEmojiStickerSet::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionChangeEmojiStickerSet::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionChangeEmojiStickerSet>(p);
}

channelAdminLogEventActionChangeEmojiStickerSet::channelAdminLogEventActionChangeEmojiStickerSet(TlBufferParser &p)
  : prev_stickerset_(TlFetchObject<InputStickerSet>::parse(p))
  , new_stickerset_(TlFetchObject<InputStickerSet>::parse(p))
{}

void channelAdminLogEventActionChangeEmojiStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionChangeEmojiStickerSet");
    s.store_object_field("prev_stickerset", static_cast<const BaseObject *>(prev_stickerset_.get()));
    s.store_object_field("new_stickerset", static_cast<const BaseObject *>(new_stickerset_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleSignatureProfiles::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleSignatureProfiles::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleSignatureProfiles>(p);
}

channelAdminLogEventActionToggleSignatureProfiles::channelAdminLogEventActionToggleSignatureProfiles(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleSignatureProfiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleSignatureProfiles");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionParticipantSubExtend::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionParticipantSubExtend::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionParticipantSubExtend>(p);
}

channelAdminLogEventActionParticipantSubExtend::channelAdminLogEventActionParticipantSubExtend(TlBufferParser &p)
  : prev_participant_(TlFetchObject<ChannelParticipant>::parse(p))
  , new_participant_(TlFetchObject<ChannelParticipant>::parse(p))
{}

void channelAdminLogEventActionParticipantSubExtend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionParticipantSubExtend");
    s.store_object_field("prev_participant", static_cast<const BaseObject *>(prev_participant_.get()));
    s.store_object_field("new_participant", static_cast<const BaseObject *>(new_participant_.get()));
    s.store_class_end();
  }
}

const std::int32_t channelAdminLogEventActionToggleAutotranslation::ID;

object_ptr<ChannelAdminLogEventAction> channelAdminLogEventActionToggleAutotranslation::fetch(TlBufferParser &p) {
  return make_tl_object<channelAdminLogEventActionToggleAutotranslation>(p);
}

channelAdminLogEventActionToggleAutotranslation::channelAdminLogEventActionToggleAutotranslation(TlBufferParser &p)
  : new_value_(TlFetchBool::parse(p))
{}

void channelAdminLogEventActionToggleAutotranslation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelAdminLogEventActionToggleAutotranslation");
    s.store_field("new_value", new_value_);
    s.store_class_end();
  }
}

object_ptr<Chat> Chat::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t chatEmpty::ID;

object_ptr<Chat> chatEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<chatEmpty>(p);
}

chatEmpty::chatEmpty(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
{}

void chatEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEmpty");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

chat::chat()
  : flags_()
  , creator_()
  , left_()
  , deactivated_()
  , call_active_()
  , call_not_empty_()
  , noforwards_()
  , id_()
  , title_()
  , photo_()
  , participants_count_()
  , date_()
  , version_()
  , migrated_to_()
  , admin_rights_()
  , default_banned_rights_()
{}

const std::int32_t chat::ID;

object_ptr<Chat> chat::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<chat> res = make_tl_object<chat>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->creator_ = (var0 & 1) != 0;
  res->left_ = (var0 & 4) != 0;
  res->deactivated_ = (var0 & 32) != 0;
  res->call_active_ = (var0 & 8388608) != 0;
  res->call_not_empty_ = (var0 & 16777216) != 0;
  res->noforwards_ = (var0 & 33554432) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  res->photo_ = TlFetchObject<ChatPhoto>::parse(p);
  res->participants_count_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->version_ = TlFetchInt::parse(p);
  if (var0 & 64) { res->migrated_to_ = TlFetchObject<InputChannel>::parse(p); }
  if (var0 & 16384) { res->admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (var0 & 262144) { res->default_banned_rights_ = TlFetchBoxed<TlFetchObject<chatBannedRights>, -1626209256>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void chat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chat");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (creator_ << 0) | (left_ << 2) | (deactivated_ << 5) | (call_active_ << 23) | (call_not_empty_ << 24) | (noforwards_ << 25)));
    if (var0 & 1) { s.store_field("creator", true); }
    if (var0 & 4) { s.store_field("left", true); }
    if (var0 & 32) { s.store_field("deactivated", true); }
    if (var0 & 8388608) { s.store_field("call_active", true); }
    if (var0 & 16777216) { s.store_field("call_not_empty", true); }
    if (var0 & 33554432) { s.store_field("noforwards", true); }
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("participants_count", participants_count_);
    s.store_field("date", date_);
    s.store_field("version", version_);
    if (var0 & 64) { s.store_object_field("migrated_to", static_cast<const BaseObject *>(migrated_to_.get())); }
    if (var0 & 16384) { s.store_object_field("admin_rights", static_cast<const BaseObject *>(admin_rights_.get())); }
    if (var0 & 262144) { s.store_object_field("default_banned_rights", static_cast<const BaseObject *>(default_banned_rights_.get())); }
    s.store_class_end();
  }
}

const std::int32_t chatForbidden::ID;

object_ptr<Chat> chatForbidden::fetch(TlBufferParser &p) {
  return make_tl_object<chatForbidden>(p);
}

chatForbidden::chatForbidden(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , title_(TlFetchString<string>::parse(p))
{}

void chatForbidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatForbidden");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

channel::channel()
  : flags_()
  , creator_()
  , left_()
  , broadcast_()
  , verified_()
  , megagroup_()
  , restricted_()
  , signatures_()
  , min_()
  , scam_()
  , has_link_()
  , has_geo_()
  , slowmode_enabled_()
  , call_active_()
  , call_not_empty_()
  , fake_()
  , gigagroup_()
  , noforwards_()
  , join_to_send_()
  , join_request_()
  , forum_()
  , flags2_()
  , stories_hidden_()
  , stories_hidden_min_()
  , stories_unavailable_()
  , signature_profiles_()
  , autotranslation_()
  , broadcast_messages_allowed_()
  , monoforum_()
  , forum_tabs_()
  , id_()
  , access_hash_()
  , title_()
  , username_()
  , photo_()
  , date_()
  , restriction_reason_()
  , admin_rights_()
  , banned_rights_()
  , default_banned_rights_()
  , participants_count_()
  , usernames_()
  , stories_max_id_()
  , color_()
  , profile_color_()
  , emoji_status_()
  , level_()
  , subscription_until_date_()
  , bot_verification_icon_()
  , send_paid_messages_stars_()
  , linked_monoforum_id_()
{}

const std::int32_t channel::ID;

object_ptr<Chat> channel::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channel> res = make_tl_object<channel>();
  int32 var0;
  int32 var1;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->creator_ = (var0 & 1) != 0;
  res->left_ = (var0 & 4) != 0;
  res->broadcast_ = (var0 & 32) != 0;
  res->verified_ = (var0 & 128) != 0;
  res->megagroup_ = (var0 & 256) != 0;
  res->restricted_ = (var0 & 512) != 0;
  res->signatures_ = (var0 & 2048) != 0;
  res->min_ = (var0 & 4096) != 0;
  res->scam_ = (var0 & 524288) != 0;
  res->has_link_ = (var0 & 1048576) != 0;
  res->has_geo_ = (var0 & 2097152) != 0;
  res->slowmode_enabled_ = (var0 & 4194304) != 0;
  res->call_active_ = (var0 & 8388608) != 0;
  res->call_not_empty_ = (var0 & 16777216) != 0;
  res->fake_ = (var0 & 33554432) != 0;
  res->gigagroup_ = (var0 & 67108864) != 0;
  res->noforwards_ = (var0 & 134217728) != 0;
  res->join_to_send_ = (var0 & 268435456) != 0;
  res->join_request_ = (var0 & 536870912) != 0;
  res->forum_ = (var0 & 1073741824) != 0;
  if ((var1 = res->flags2_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->stories_hidden_ = (var1 & 2) != 0;
  res->stories_hidden_min_ = (var1 & 4) != 0;
  res->stories_unavailable_ = (var1 & 8) != 0;
  res->signature_profiles_ = (var1 & 4096) != 0;
  res->autotranslation_ = (var1 & 32768) != 0;
  res->broadcast_messages_allowed_ = (var1 & 65536) != 0;
  res->monoforum_ = (var1 & 131072) != 0;
  res->forum_tabs_ = (var1 & 524288) != 0;
  res->id_ = TlFetchLong::parse(p);
  if (var0 & 8192) { res->access_hash_ = TlFetchLong::parse(p); }
  res->title_ = TlFetchString<string>::parse(p);
  if (var0 & 64) { res->username_ = TlFetchString<string>::parse(p); }
  res->photo_ = TlFetchObject<ChatPhoto>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 512) { res->restriction_reason_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<restrictionReason>, -797791052>>, 481674261>::parse(p); }
  if (var0 & 16384) { res->admin_rights_ = TlFetchBoxed<TlFetchObject<chatAdminRights>, 1605510357>::parse(p); }
  if (var0 & 32768) { res->banned_rights_ = TlFetchBoxed<TlFetchObject<chatBannedRights>, -1626209256>::parse(p); }
  if (var0 & 262144) { res->default_banned_rights_ = TlFetchBoxed<TlFetchObject<chatBannedRights>, -1626209256>::parse(p); }
  if (var0 & 131072) { res->participants_count_ = TlFetchInt::parse(p); }
  if (var1 & 1) { res->usernames_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<username>, -1274595769>>, 481674261>::parse(p); }
  if (var1 & 16) { res->stories_max_id_ = TlFetchInt::parse(p); }
  if (var1 & 128) { res->color_ = TlFetchObject<PeerColor>::parse(p); }
  if (var1 & 256) { res->profile_color_ = TlFetchObject<PeerColor>::parse(p); }
  if (var1 & 512) { res->emoji_status_ = TlFetchObject<EmojiStatus>::parse(p); }
  if (var1 & 1024) { res->level_ = TlFetchInt::parse(p); }
  if (var1 & 2048) { res->subscription_until_date_ = TlFetchInt::parse(p); }
  if (var1 & 8192) { res->bot_verification_icon_ = TlFetchLong::parse(p); }
  if (var1 & 16384) { res->send_paid_messages_stars_ = TlFetchLong::parse(p); }
  if (var1 & 262144) { res->linked_monoforum_id_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channel");
  int32 var0;
  int32 var1;
    s.store_field("flags", (var0 = flags_ | (creator_ << 0) | (left_ << 2) | (broadcast_ << 5) | (verified_ << 7) | (megagroup_ << 8) | (restricted_ << 9) | (signatures_ << 11) | (min_ << 12) | (scam_ << 19) | (has_link_ << 20) | (has_geo_ << 21) | (slowmode_enabled_ << 22) | (call_active_ << 23) | (call_not_empty_ << 24) | (fake_ << 25) | (gigagroup_ << 26) | (noforwards_ << 27) | (join_to_send_ << 28) | (join_request_ << 29) | (forum_ << 30)));
    if (var0 & 1) { s.store_field("creator", true); }
    if (var0 & 4) { s.store_field("left", true); }
    if (var0 & 32) { s.store_field("broadcast", true); }
    if (var0 & 128) { s.store_field("verified", true); }
    if (var0 & 256) { s.store_field("megagroup", true); }
    if (var0 & 512) { s.store_field("restricted", true); }
    if (var0 & 2048) { s.store_field("signatures", true); }
    if (var0 & 4096) { s.store_field("min", true); }
    if (var0 & 524288) { s.store_field("scam", true); }
    if (var0 & 1048576) { s.store_field("has_link", true); }
    if (var0 & 2097152) { s.store_field("has_geo", true); }
    if (var0 & 4194304) { s.store_field("slowmode_enabled", true); }
    if (var0 & 8388608) { s.store_field("call_active", true); }
    if (var0 & 16777216) { s.store_field("call_not_empty", true); }
    if (var0 & 33554432) { s.store_field("fake", true); }
    if (var0 & 67108864) { s.store_field("gigagroup", true); }
    if (var0 & 134217728) { s.store_field("noforwards", true); }
    if (var0 & 268435456) { s.store_field("join_to_send", true); }
    if (var0 & 536870912) { s.store_field("join_request", true); }
    if (var0 & 1073741824) { s.store_field("forum", true); }
    s.store_field("flags2", (var1 = flags2_ | (stories_hidden_ << 1) | (stories_hidden_min_ << 2) | (stories_unavailable_ << 3) | (signature_profiles_ << 12) | (autotranslation_ << 15) | (broadcast_messages_allowed_ << 16) | (monoforum_ << 17) | (forum_tabs_ << 19)));
    if (var1 & 2) { s.store_field("stories_hidden", true); }
    if (var1 & 4) { s.store_field("stories_hidden_min", true); }
    if (var1 & 8) { s.store_field("stories_unavailable", true); }
    if (var1 & 4096) { s.store_field("signature_profiles", true); }
    if (var1 & 32768) { s.store_field("autotranslation", true); }
    if (var1 & 65536) { s.store_field("broadcast_messages_allowed", true); }
    if (var1 & 131072) { s.store_field("monoforum", true); }
    if (var1 & 524288) { s.store_field("forum_tabs", true); }
    s.store_field("id", id_);
    if (var0 & 8192) { s.store_field("access_hash", access_hash_); }
    s.store_field("title", title_);
    if (var0 & 64) { s.store_field("username", username_); }
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("date", date_);
    if (var0 & 512) { { s.store_vector_begin("restriction_reason", restriction_reason_.size()); for (const auto &_value : restriction_reason_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 16384) { s.store_object_field("admin_rights", static_cast<const BaseObject *>(admin_rights_.get())); }
    if (var0 & 32768) { s.store_object_field("banned_rights", static_cast<const BaseObject *>(banned_rights_.get())); }
    if (var0 & 262144) { s.store_object_field("default_banned_rights", static_cast<const BaseObject *>(default_banned_rights_.get())); }
    if (var0 & 131072) { s.store_field("participants_count", participants_count_); }
    if (var1 & 1) { { s.store_vector_begin("usernames", usernames_.size()); for (const auto &_value : usernames_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var1 & 16) { s.store_field("stories_max_id", stories_max_id_); }
    if (var1 & 128) { s.store_object_field("color", static_cast<const BaseObject *>(color_.get())); }
    if (var1 & 256) { s.store_object_field("profile_color", static_cast<const BaseObject *>(profile_color_.get())); }
    if (var1 & 512) { s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get())); }
    if (var1 & 1024) { s.store_field("level", level_); }
    if (var1 & 2048) { s.store_field("subscription_until_date", subscription_until_date_); }
    if (var1 & 8192) { s.store_field("bot_verification_icon", bot_verification_icon_); }
    if (var1 & 16384) { s.store_field("send_paid_messages_stars", send_paid_messages_stars_); }
    if (var1 & 262144) { s.store_field("linked_monoforum_id", linked_monoforum_id_); }
    s.store_class_end();
  }
}

channelForbidden::channelForbidden()
  : flags_()
  , broadcast_()
  , megagroup_()
  , id_()
  , access_hash_()
  , title_()
  , until_date_()
{}

channelForbidden::channelForbidden(int32 flags_, bool broadcast_, bool megagroup_, int64 id_, int64 access_hash_, string const &title_, int32 until_date_)
  : flags_(flags_)
  , broadcast_(broadcast_)
  , megagroup_(megagroup_)
  , id_(id_)
  , access_hash_(access_hash_)
  , title_(title_)
  , until_date_(until_date_)
{}

const std::int32_t channelForbidden::ID;

object_ptr<Chat> channelForbidden::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<channelForbidden> res = make_tl_object<channelForbidden>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->broadcast_ = (var0 & 32) != 0;
  res->megagroup_ = (var0 & 256) != 0;
  res->id_ = TlFetchLong::parse(p);
  res->access_hash_ = TlFetchLong::parse(p);
  res->title_ = TlFetchString<string>::parse(p);
  if (var0 & 65536) { res->until_date_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void channelForbidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channelForbidden");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (broadcast_ << 5) | (megagroup_ << 8)));
    if (var0 & 32) { s.store_field("broadcast", true); }
    if (var0 & 256) { s.store_field("megagroup", true); }
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("title", title_);
    if (var0 & 65536) { s.store_field("until_date", until_date_); }
    s.store_class_end();
  }
}

dcOption::dcOption()
  : flags_()
  , ipv6_()
  , media_only_()
  , tcpo_only_()
  , cdn_()
  , static_()
  , this_port_only_()
  , id_()
  , ip_address_()
  , port_()
  , secret_()
{}

const std::int32_t dcOption::ID;

object_ptr<dcOption> dcOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<dcOption> res = make_tl_object<dcOption>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->ipv6_ = (var0 & 1) != 0;
  res->media_only_ = (var0 & 2) != 0;
  res->tcpo_only_ = (var0 & 4) != 0;
  res->cdn_ = (var0 & 8) != 0;
  res->static_ = (var0 & 16) != 0;
  res->this_port_only_ = (var0 & 32) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->ip_address_ = TlFetchString<string>::parse(p);
  res->port_ = TlFetchInt::parse(p);
  if (var0 & 1024) { res->secret_ = TlFetchBytes<bytes>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void dcOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dcOption");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (ipv6_ << 0) | (media_only_ << 1) | (tcpo_only_ << 2) | (cdn_ << 3) | (static_ << 4) | (this_port_only_ << 5)));
    if (var0 & 1) { s.store_field("ipv6", true); }
    if (var0 & 2) { s.store_field("media_only", true); }
    if (var0 & 4) { s.store_field("tcpo_only", true); }
    if (var0 & 8) { s.store_field("cdn", true); }
    if (var0 & 16) { s.store_field("static", true); }
    if (var0 & 32) { s.store_field("this_port_only", true); }
    s.store_field("id", id_);
    s.store_field("ip_address", ip_address_);
    s.store_field("port", port_);
    if (var0 & 1024) { s.store_bytes_field("secret", secret_); }
    s.store_class_end();
  }
}

object_ptr<DialogFilter> DialogFilter::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case dialogFilter::ID:
      return dialogFilter::fetch(p);
    case dialogFilterDefault::ID:
      return dialogFilterDefault::fetch(p);
    case dialogFilterChatlist::ID:
      return dialogFilterChatlist::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

dialogFilter::dialogFilter()
  : flags_()
  , contacts_()
  , non_contacts_()
  , groups_()
  , broadcasts_()
  , bots_()
  , exclude_muted_()
  , exclude_read_()
  , exclude_archived_()
  , title_noanimate_()
  , id_()
  , title_()
  , emoticon_()
  , color_()
  , pinned_peers_()
  , include_peers_()
  , exclude_peers_()
{}

dialogFilter::dialogFilter(int32 flags_, bool contacts_, bool non_contacts_, bool groups_, bool broadcasts_, bool bots_, bool exclude_muted_, bool exclude_read_, bool exclude_archived_, bool title_noanimate_, int32 id_, object_ptr<textWithEntities> &&title_, string const &emoticon_, int32 color_, array<object_ptr<InputPeer>> &&pinned_peers_, array<object_ptr<InputPeer>> &&include_peers_, array<object_ptr<InputPeer>> &&exclude_peers_)
  : flags_(flags_)
  , contacts_(contacts_)
  , non_contacts_(non_contacts_)
  , groups_(groups_)
  , broadcasts_(broadcasts_)
  , bots_(bots_)
  , exclude_muted_(exclude_muted_)
  , exclude_read_(exclude_read_)
  , exclude_archived_(exclude_archived_)
  , title_noanimate_(title_noanimate_)
  , id_(id_)
  , title_(std::move(title_))
  , emoticon_(emoticon_)
  , color_(color_)
  , pinned_peers_(std::move(pinned_peers_))
  , include_peers_(std::move(include_peers_))
  , exclude_peers_(std::move(exclude_peers_))
{}

const std::int32_t dialogFilter::ID;

object_ptr<DialogFilter> dialogFilter::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<dialogFilter> res = make_tl_object<dialogFilter>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->contacts_ = (var0 & 1) != 0;
  res->non_contacts_ = (var0 & 2) != 0;
  res->groups_ = (var0 & 4) != 0;
  res->broadcasts_ = (var0 & 8) != 0;
  res->bots_ = (var0 & 16) != 0;
  res->exclude_muted_ = (var0 & 2048) != 0;
  res->exclude_read_ = (var0 & 4096) != 0;
  res->exclude_archived_ = (var0 & 8192) != 0;
  res->title_noanimate_ = (var0 & 268435456) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->title_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p);
  if (var0 & 33554432) { res->emoticon_ = TlFetchString<string>::parse(p); }
  if (var0 & 134217728) { res->color_ = TlFetchInt::parse(p); }
  res->pinned_peers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputPeer>>, 481674261>::parse(p);
  res->include_peers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputPeer>>, 481674261>::parse(p);
  res->exclude_peers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputPeer>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void dialogFilter::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (contacts_ << 0) | (non_contacts_ << 1) | (groups_ << 2) | (broadcasts_ << 3) | (bots_ << 4) | (exclude_muted_ << 11) | (exclude_read_ << 12) | (exclude_archived_ << 13) | (title_noanimate_ << 28)), s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
  if (var0 & 33554432) { TlStoreString::store(emoticon_, s); }
  if (var0 & 134217728) { TlStoreBinary::store(color_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(pinned_peers_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(include_peers_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(exclude_peers_, s);
}

void dialogFilter::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (contacts_ << 0) | (non_contacts_ << 1) | (groups_ << 2) | (broadcasts_ << 3) | (bots_ << 4) | (exclude_muted_ << 11) | (exclude_read_ << 12) | (exclude_archived_ << 13) | (title_noanimate_ << 28)), s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
  if (var0 & 33554432) { TlStoreString::store(emoticon_, s); }
  if (var0 & 134217728) { TlStoreBinary::store(color_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(pinned_peers_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(include_peers_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(exclude_peers_, s);
}

void dialogFilter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialogFilter");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (contacts_ << 0) | (non_contacts_ << 1) | (groups_ << 2) | (broadcasts_ << 3) | (bots_ << 4) | (exclude_muted_ << 11) | (exclude_read_ << 12) | (exclude_archived_ << 13) | (title_noanimate_ << 28)));
    if (var0 & 1) { s.store_field("contacts", true); }
    if (var0 & 2) { s.store_field("non_contacts", true); }
    if (var0 & 4) { s.store_field("groups", true); }
    if (var0 & 8) { s.store_field("broadcasts", true); }
    if (var0 & 16) { s.store_field("bots", true); }
    if (var0 & 2048) { s.store_field("exclude_muted", true); }
    if (var0 & 4096) { s.store_field("exclude_read", true); }
    if (var0 & 8192) { s.store_field("exclude_archived", true); }
    if (var0 & 268435456) { s.store_field("title_noanimate", true); }
    s.store_field("id", id_);
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    if (var0 & 33554432) { s.store_field("emoticon", emoticon_); }
    if (var0 & 134217728) { s.store_field("color", color_); }
    { s.store_vector_begin("pinned_peers", pinned_peers_.size()); for (const auto &_value : pinned_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("include_peers", include_peers_.size()); for (const auto &_value : include_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("exclude_peers", exclude_peers_.size()); for (const auto &_value : exclude_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t dialogFilterDefault::ID;

object_ptr<DialogFilter> dialogFilterDefault::fetch(TlBufferParser &p) {
  return make_tl_object<dialogFilterDefault>();
}

void dialogFilterDefault::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void dialogFilterDefault::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void dialogFilterDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialogFilterDefault");
    s.store_class_end();
  }
}

dialogFilterChatlist::dialogFilterChatlist()
  : flags_()
  , has_my_invites_()
  , title_noanimate_()
  , id_()
  , title_()
  , emoticon_()
  , color_()
  , pinned_peers_()
  , include_peers_()
{}

dialogFilterChatlist::dialogFilterChatlist(int32 flags_, bool has_my_invites_, bool title_noanimate_, int32 id_, object_ptr<textWithEntities> &&title_, string const &emoticon_, int32 color_, array<object_ptr<InputPeer>> &&pinned_peers_, array<object_ptr<InputPeer>> &&include_peers_)
  : flags_(flags_)
  , has_my_invites_(has_my_invites_)
  , title_noanimate_(title_noanimate_)
  , id_(id_)
  , title_(std::move(title_))
  , emoticon_(emoticon_)
  , color_(color_)
  , pinned_peers_(std::move(pinned_peers_))
  , include_peers_(std::move(include_peers_))
{}

const std::int32_t dialogFilterChatlist::ID;

object_ptr<DialogFilter> dialogFilterChatlist::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<dialogFilterChatlist> res = make_tl_object<dialogFilterChatlist>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->has_my_invites_ = (var0 & 67108864) != 0;
  res->title_noanimate_ = (var0 & 268435456) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->title_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p);
  if (var0 & 33554432) { res->emoticon_ = TlFetchString<string>::parse(p); }
  if (var0 & 134217728) { res->color_ = TlFetchInt::parse(p); }
  res->pinned_peers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputPeer>>, 481674261>::parse(p);
  res->include_peers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<InputPeer>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void dialogFilterChatlist::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (has_my_invites_ << 26) | (title_noanimate_ << 28)), s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
  if (var0 & 33554432) { TlStoreString::store(emoticon_, s); }
  if (var0 & 134217728) { TlStoreBinary::store(color_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(pinned_peers_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(include_peers_, s);
}

void dialogFilterChatlist::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (has_my_invites_ << 26) | (title_noanimate_ << 28)), s);
  TlStoreBinary::store(id_, s);
  TlStoreBoxed<TlStoreObject, 1964978502>::store(title_, s);
  if (var0 & 33554432) { TlStoreString::store(emoticon_, s); }
  if (var0 & 134217728) { TlStoreBinary::store(color_, s); }
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(pinned_peers_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(include_peers_, s);
}

void dialogFilterChatlist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dialogFilterChatlist");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (has_my_invites_ << 26) | (title_noanimate_ << 28)));
    if (var0 & 67108864) { s.store_field("has_my_invites", true); }
    if (var0 & 268435456) { s.store_field("title_noanimate", true); }
    s.store_field("id", id_);
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    if (var0 & 33554432) { s.store_field("emoticon", emoticon_); }
    if (var0 & 134217728) { s.store_field("color", color_); }
    { s.store_vector_begin("pinned_peers", pinned_peers_.size()); for (const auto &_value : pinned_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("include_peers", include_peers_.size()); for (const auto &_value : include_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

exportedChatlistInvite::exportedChatlistInvite()
  : flags_()
  , title_()
  , url_()
  , peers_()
{}

const std::int32_t exportedChatlistInvite::ID;

object_ptr<exportedChatlistInvite> exportedChatlistInvite::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<exportedChatlistInvite> res = make_tl_object<exportedChatlistInvite>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->title_ = TlFetchString<string>::parse(p);
  res->url_ = TlFetchString<string>::parse(p);
  res->peers_ = TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void exportedChatlistInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "exportedChatlistInvite");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("title", title_);
    s.store_field("url", url_);
    { s.store_vector_begin("peers", peers_.size()); for (const auto &_value : peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

globalPrivacySettings::globalPrivacySettings()
  : flags_()
  , archive_and_mute_new_noncontact_peers_()
  , keep_archived_unmuted_()
  , keep_archived_folders_()
  , hide_read_marks_()
  , new_noncontact_peers_require_premium_()
  , display_gifts_button_()
  , noncontact_peers_paid_stars_()
  , disallowed_gifts_()
{}

globalPrivacySettings::globalPrivacySettings(int32 flags_, bool archive_and_mute_new_noncontact_peers_, bool keep_archived_unmuted_, bool keep_archived_folders_, bool hide_read_marks_, bool new_noncontact_peers_require_premium_, bool display_gifts_button_, int64 noncontact_peers_paid_stars_, object_ptr<disallowedGiftsSettings> &&disallowed_gifts_)
  : flags_(flags_)
  , archive_and_mute_new_noncontact_peers_(archive_and_mute_new_noncontact_peers_)
  , keep_archived_unmuted_(keep_archived_unmuted_)
  , keep_archived_folders_(keep_archived_folders_)
  , hide_read_marks_(hide_read_marks_)
  , new_noncontact_peers_require_premium_(new_noncontact_peers_require_premium_)
  , display_gifts_button_(display_gifts_button_)
  , noncontact_peers_paid_stars_(noncontact_peers_paid_stars_)
  , disallowed_gifts_(std::move(disallowed_gifts_))
{}

const std::int32_t globalPrivacySettings::ID;

object_ptr<globalPrivacySettings> globalPrivacySettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<globalPrivacySettings> res = make_tl_object<globalPrivacySettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->archive_and_mute_new_noncontact_peers_ = (var0 & 1) != 0;
  res->keep_archived_unmuted_ = (var0 & 2) != 0;
  res->keep_archived_folders_ = (var0 & 4) != 0;
  res->hide_read_marks_ = (var0 & 8) != 0;
  res->new_noncontact_peers_require_premium_ = (var0 & 16) != 0;
  res->display_gifts_button_ = (var0 & 128) != 0;
  if (var0 & 32) { res->noncontact_peers_paid_stars_ = TlFetchLong::parse(p); }
  if (var0 & 64) { res->disallowed_gifts_ = TlFetchBoxed<TlFetchObject<disallowedGiftsSettings>, 1911715524>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void globalPrivacySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (archive_and_mute_new_noncontact_peers_ << 0) | (keep_archived_unmuted_ << 1) | (keep_archived_folders_ << 2) | (hide_read_marks_ << 3) | (new_noncontact_peers_require_premium_ << 4) | (display_gifts_button_ << 7)), s);
  if (var0 & 32) { TlStoreBinary::store(noncontact_peers_paid_stars_, s); }
  if (var0 & 64) { TlStoreBoxed<TlStoreObject, 1911715524>::store(disallowed_gifts_, s); }
}

void globalPrivacySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (archive_and_mute_new_noncontact_peers_ << 0) | (keep_archived_unmuted_ << 1) | (keep_archived_folders_ << 2) | (hide_read_marks_ << 3) | (new_noncontact_peers_require_premium_ << 4) | (display_gifts_button_ << 7)), s);
  if (var0 & 32) { TlStoreBinary::store(noncontact_peers_paid_stars_, s); }
  if (var0 & 64) { TlStoreBoxed<TlStoreObject, 1911715524>::store(disallowed_gifts_, s); }
}

void globalPrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "globalPrivacySettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (archive_and_mute_new_noncontact_peers_ << 0) | (keep_archived_unmuted_ << 1) | (keep_archived_folders_ << 2) | (hide_read_marks_ << 3) | (new_noncontact_peers_require_premium_ << 4) | (display_gifts_button_ << 7)));
    if (var0 & 1) { s.store_field("archive_and_mute_new_noncontact_peers", true); }
    if (var0 & 2) { s.store_field("keep_archived_unmuted", true); }
    if (var0 & 4) { s.store_field("keep_archived_folders", true); }
    if (var0 & 8) { s.store_field("hide_read_marks", true); }
    if (var0 & 16) { s.store_field("new_noncontact_peers_require_premium", true); }
    if (var0 & 128) { s.store_field("display_gifts_button", true); }
    if (var0 & 32) { s.store_field("noncontact_peers_paid_stars", noncontact_peers_paid_stars_); }
    if (var0 & 64) { s.store_object_field("disallowed_gifts", static_cast<const BaseObject *>(disallowed_gifts_.get())); }
    s.store_class_end();
  }
}

groupCallParticipant::groupCallParticipant()
  : flags_()
  , muted_()
  , left_()
  , can_self_unmute_()
  , just_joined_()
  , versioned_()
  , min_()
  , muted_by_you_()
  , volume_by_admin_()
  , self_()
  , video_joined_()
  , peer_()
  , date_()
  , active_date_()
  , source_()
  , volume_()
  , about_()
  , raise_hand_rating_()
  , video_()
  , presentation_()
{}

const std::int32_t groupCallParticipant::ID;

object_ptr<groupCallParticipant> groupCallParticipant::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<groupCallParticipant> res = make_tl_object<groupCallParticipant>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->muted_ = (var0 & 1) != 0;
  res->left_ = (var0 & 2) != 0;
  res->can_self_unmute_ = (var0 & 4) != 0;
  res->just_joined_ = (var0 & 16) != 0;
  res->versioned_ = (var0 & 32) != 0;
  res->min_ = (var0 & 256) != 0;
  res->muted_by_you_ = (var0 & 512) != 0;
  res->volume_by_admin_ = (var0 & 1024) != 0;
  res->self_ = (var0 & 4096) != 0;
  res->video_joined_ = (var0 & 32768) != 0;
  res->peer_ = TlFetchObject<Peer>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 8) { res->active_date_ = TlFetchInt::parse(p); }
  res->source_ = TlFetchInt::parse(p);
  if (var0 & 128) { res->volume_ = TlFetchInt::parse(p); }
  if (var0 & 2048) { res->about_ = TlFetchString<string>::parse(p); }
  if (var0 & 8192) { res->raise_hand_rating_ = TlFetchLong::parse(p); }
  if (var0 & 64) { res->video_ = TlFetchBoxed<TlFetchObject<groupCallParticipantVideo>, 1735736008>::parse(p); }
  if (var0 & 16384) { res->presentation_ = TlFetchBoxed<TlFetchObject<groupCallParticipantVideo>, 1735736008>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void groupCallParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallParticipant");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (muted_ << 0) | (left_ << 1) | (can_self_unmute_ << 2) | (just_joined_ << 4) | (versioned_ << 5) | (min_ << 8) | (muted_by_you_ << 9) | (volume_by_admin_ << 10) | (self_ << 12) | (video_joined_ << 15)));
    if (var0 & 1) { s.store_field("muted", true); }
    if (var0 & 2) { s.store_field("left", true); }
    if (var0 & 4) { s.store_field("can_self_unmute", true); }
    if (var0 & 16) { s.store_field("just_joined", true); }
    if (var0 & 32) { s.store_field("versioned", true); }
    if (var0 & 256) { s.store_field("min", true); }
    if (var0 & 512) { s.store_field("muted_by_you", true); }
    if (var0 & 1024) { s.store_field("volume_by_admin", true); }
    if (var0 & 4096) { s.store_field("self", true); }
    if (var0 & 32768) { s.store_field("video_joined", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("date", date_);
    if (var0 & 8) { s.store_field("active_date", active_date_); }
    s.store_field("source", source_);
    if (var0 & 128) { s.store_field("volume", volume_); }
    if (var0 & 2048) { s.store_field("about", about_); }
    if (var0 & 8192) { s.store_field("raise_hand_rating", raise_hand_rating_); }
    if (var0 & 64) { s.store_object_field("video", static_cast<const BaseObject *>(video_.get())); }
    if (var0 & 16384) { s.store_object_field("presentation", static_cast<const BaseObject *>(presentation_.get())); }
    s.store_class_end();
  }
}

const std::int32_t importedContact::ID;

object_ptr<importedContact> importedContact::fetch(TlBufferParser &p) {
  return make_tl_object<importedContact>(p);
}

importedContact::importedContact(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , client_id_(TlFetchLong::parse(p))
{}

void importedContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "importedContact");
    s.store_field("user_id", user_id_);
    s.store_field("client_id", client_id_);
    s.store_class_end();
  }
}

const std::int32_t inputChatPhotoEmpty::ID;

void inputChatPhotoEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputChatPhotoEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputChatPhotoEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatPhotoEmpty");
    s.store_class_end();
  }
}

inputChatUploadedPhoto::inputChatUploadedPhoto(int32 flags_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&video_, double video_start_ts_, object_ptr<VideoSize> &&video_emoji_markup_)
  : flags_(flags_)
  , file_(std::move(file_))
  , video_(std::move(video_))
  , video_start_ts_(video_start_ts_)
  , video_emoji_markup_(std::move(video_emoji_markup_))
{}

const std::int32_t inputChatUploadedPhoto::ID;

void inputChatUploadedPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(file_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(video_, s); }
  if (var0 & 4) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(video_emoji_markup_, s); }
}

void inputChatUploadedPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(file_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(video_, s); }
  if (var0 & 4) { TlStoreBinary::store(video_start_ts_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(video_emoji_markup_, s); }
}

void inputChatUploadedPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatUploadedPhoto");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("file", static_cast<const BaseObject *>(file_.get())); }
    if (var0 & 2) { s.store_object_field("video", static_cast<const BaseObject *>(video_.get())); }
    if (var0 & 4) { s.store_field("video_start_ts", video_start_ts_); }
    if (var0 & 8) { s.store_object_field("video_emoji_markup", static_cast<const BaseObject *>(video_emoji_markup_.get())); }
    s.store_class_end();
  }
}

inputChatPhoto::inputChatPhoto(object_ptr<InputPhoto> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t inputChatPhoto::ID;

void inputChatPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputChatPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatPhoto");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

inputCollectibleUsername::inputCollectibleUsername(string const &username_)
  : username_(username_)
{}

const std::int32_t inputCollectibleUsername::ID;

void inputCollectibleUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(username_, s);
}

void inputCollectibleUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(username_, s);
}

void inputCollectibleUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCollectibleUsername");
    s.store_field("username", username_);
    s.store_class_end();
  }
}

inputCollectiblePhone::inputCollectiblePhone(string const &phone_)
  : phone_(phone_)
{}

const std::int32_t inputCollectiblePhone::ID;

void inputCollectiblePhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_, s);
}

void inputCollectiblePhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_, s);
}

void inputCollectiblePhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCollectiblePhone");
    s.store_field("phone", phone_);
    s.store_class_end();
  }
}

inputDialogPeer::inputDialogPeer(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t inputDialogPeer::ID;

void inputDialogPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void inputDialogPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void inputDialogPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputDialogPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

inputDialogPeerFolder::inputDialogPeerFolder(int32 folder_id_)
  : folder_id_(folder_id_)
{}

const std::int32_t inputDialogPeerFolder::ID;

void inputDialogPeerFolder::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(folder_id_, s);
}

void inputDialogPeerFolder::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(folder_id_, s);
}

void inputDialogPeerFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputDialogPeerFolder");
    s.store_field("folder_id", folder_id_);
    s.store_class_end();
  }
}

inputEncryptedChat::inputEncryptedChat(int32 chat_id_, int64 access_hash_)
  : chat_id_(chat_id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputEncryptedChat::ID;

void inputEncryptedChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputEncryptedChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(chat_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputEncryptedChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputEncryptedChat");
    s.store_field("chat_id", chat_id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

object_ptr<InputFile> InputFile::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputFile::ID:
      return inputFile::fetch(p);
    case inputFileBig::ID:
      return inputFileBig::fetch(p);
    case inputFileStoryDocument::ID:
      return inputFileStoryDocument::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

inputFile::inputFile(int64 id_, int32 parts_, string const &name_, string const &md5_checksum_)
  : id_(id_)
  , parts_(parts_)
  , name_(name_)
  , md5_checksum_(md5_checksum_)
{}

const std::int32_t inputFile::ID;

object_ptr<InputFile> inputFile::fetch(TlBufferParser &p) {
  return make_tl_object<inputFile>(p);
}

inputFile::inputFile(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , parts_(TlFetchInt::parse(p))
  , name_(TlFetchString<string>::parse(p))
  , md5_checksum_(TlFetchString<string>::parse(p))
{}

void inputFile::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(name_, s);
  TlStoreString::store(md5_checksum_, s);
}

void inputFile::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(name_, s);
  TlStoreString::store(md5_checksum_, s);
}

void inputFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFile");
    s.store_field("id", id_);
    s.store_field("parts", parts_);
    s.store_field("name", name_);
    s.store_field("md5_checksum", md5_checksum_);
    s.store_class_end();
  }
}

inputFileBig::inputFileBig(int64 id_, int32 parts_, string const &name_)
  : id_(id_)
  , parts_(parts_)
  , name_(name_)
{}

const std::int32_t inputFileBig::ID;

object_ptr<InputFile> inputFileBig::fetch(TlBufferParser &p) {
  return make_tl_object<inputFileBig>(p);
}

inputFileBig::inputFileBig(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , parts_(TlFetchInt::parse(p))
  , name_(TlFetchString<string>::parse(p))
{}

void inputFileBig::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(name_, s);
}

void inputFileBig::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(parts_, s);
  TlStoreString::store(name_, s);
}

void inputFileBig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFileBig");
    s.store_field("id", id_);
    s.store_field("parts", parts_);
    s.store_field("name", name_);
    s.store_class_end();
  }
}

inputFileStoryDocument::inputFileStoryDocument(object_ptr<InputDocument> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t inputFileStoryDocument::ID;

object_ptr<InputFile> inputFileStoryDocument::fetch(TlBufferParser &p) {
  return make_tl_object<inputFileStoryDocument>(p);
}

inputFileStoryDocument::inputFileStoryDocument(TlBufferParser &p)
  : id_(TlFetchObject<InputDocument>::parse(p))
{}

void inputFileStoryDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputFileStoryDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
}

void inputFileStoryDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFileStoryDocument");
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    s.store_class_end();
  }
}

object_ptr<InputGame> InputGame::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputGameID::ID:
      return inputGameID::fetch(p);
    case inputGameShortName::ID:
      return inputGameShortName::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

inputGameID::inputGameID(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputGameID::ID;

object_ptr<InputGame> inputGameID::fetch(TlBufferParser &p) {
  return make_tl_object<inputGameID>(p);
}

inputGameID::inputGameID(TlBufferParser &p)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputGameID::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputGameID::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputGameID::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGameID");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputGameShortName::inputGameShortName(object_ptr<InputUser> &&bot_id_, string const &short_name_)
  : bot_id_(std::move(bot_id_))
  , short_name_(short_name_)
{}

const std::int32_t inputGameShortName::ID;

object_ptr<InputGame> inputGameShortName::fetch(TlBufferParser &p) {
  return make_tl_object<inputGameShortName>(p);
}

inputGameShortName::inputGameShortName(TlBufferParser &p)
  : bot_id_(TlFetchObject<InputUser>::parse(p))
  , short_name_(TlFetchString<string>::parse(p))
{}

void inputGameShortName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_id_, s);
  TlStoreString::store(short_name_, s);
}

void inputGameShortName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_id_, s);
  TlStoreString::store(short_name_, s);
}

void inputGameShortName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGameShortName");
    s.store_object_field("bot_id", static_cast<const BaseObject *>(bot_id_.get()));
    s.store_field("short_name", short_name_);
    s.store_class_end();
  }
}

object_ptr<InputPeer> InputPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
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
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t inputPeerEmpty::ID;

object_ptr<InputPeer> inputPeerEmpty::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerEmpty>();
}

void inputPeerEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPeerEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPeerEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerEmpty");
    s.store_class_end();
  }
}

const std::int32_t inputPeerSelf::ID;

object_ptr<InputPeer> inputPeerSelf::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerSelf>();
}

void inputPeerSelf::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void inputPeerSelf::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void inputPeerSelf::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerSelf");
    s.store_class_end();
  }
}

inputPeerChat::inputPeerChat(int64 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t inputPeerChat::ID;

object_ptr<InputPeer> inputPeerChat::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerChat>(p);
}

inputPeerChat::inputPeerChat(TlBufferParser &p)
  : chat_id_(TlFetchLong::parse(p))
{}

void inputPeerChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(chat_id_, s);
}

void inputPeerChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(chat_id_, s);
}

void inputPeerChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

inputPeerUser::inputPeerUser(int64 user_id_, int64 access_hash_)
  : user_id_(user_id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputPeerUser::ID;

object_ptr<InputPeer> inputPeerUser::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerUser>(p);
}

inputPeerUser::inputPeerUser(TlBufferParser &p)
  : user_id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputPeerUser::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputPeerUser::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(user_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputPeerUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerUser");
    s.store_field("user_id", user_id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputPeerChannel::inputPeerChannel(int64 channel_id_, int64 access_hash_)
  : channel_id_(channel_id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputPeerChannel::ID;

object_ptr<InputPeer> inputPeerChannel::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerChannel>(p);
}

inputPeerChannel::inputPeerChannel(TlBufferParser &p)
  : channel_id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
{}

void inputPeerChannel::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(channel_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputPeerChannel::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(channel_id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputPeerChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerChannel");
    s.store_field("channel_id", channel_id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputPeerUserFromMessage::inputPeerUserFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 user_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , user_id_(user_id_)
{}

const std::int32_t inputPeerUserFromMessage::ID;

object_ptr<InputPeer> inputPeerUserFromMessage::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerUserFromMessage>(p);
}

inputPeerUserFromMessage::inputPeerUserFromMessage(TlBufferParser &p)
  : peer_(TlFetchObject<InputPeer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , user_id_(TlFetchLong::parse(p))
{}

void inputPeerUserFromMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(user_id_, s);
}

void inputPeerUserFromMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(user_id_, s);
}

void inputPeerUserFromMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerUserFromMessage");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

inputPeerChannelFromMessage::inputPeerChannelFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 channel_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
  , channel_id_(channel_id_)
{}

const std::int32_t inputPeerChannelFromMessage::ID;

object_ptr<InputPeer> inputPeerChannelFromMessage::fetch(TlBufferParser &p) {
  return make_tl_object<inputPeerChannelFromMessage>(p);
}

inputPeerChannelFromMessage::inputPeerChannelFromMessage(TlBufferParser &p)
  : peer_(TlFetchObject<InputPeer>::parse(p))
  , msg_id_(TlFetchInt::parse(p))
  , channel_id_(TlFetchLong::parse(p))
{}

void inputPeerChannelFromMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(channel_id_, s);
}

void inputPeerChannelFromMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
  TlStoreBinary::store(channel_id_, s);
}

void inputPeerChannelFromMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerChannelFromMessage");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_field("channel_id", channel_id_);
    s.store_class_end();
  }
}

inputPeerNotifySettings::inputPeerNotifySettings(int32 flags_, bool show_previews_, bool silent_, int32 mute_until_, object_ptr<NotificationSound> &&sound_, bool stories_muted_, bool stories_hide_sender_, object_ptr<NotificationSound> &&stories_sound_)
  : flags_(flags_)
  , show_previews_(show_previews_)
  , silent_(silent_)
  , mute_until_(mute_until_)
  , sound_(std::move(sound_))
  , stories_muted_(stories_muted_)
  , stories_hide_sender_(stories_hide_sender_)
  , stories_sound_(std::move(stories_sound_))
{}

const std::int32_t inputPeerNotifySettings::ID;

void inputPeerNotifySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBool::store(show_previews_, s); }
  if (var0 & 2) { TlStoreBool::store(silent_, s); }
  if (var0 & 4) { TlStoreBinary::store(mute_until_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(sound_, s); }
  if (var0 & 64) { TlStoreBool::store(stories_muted_, s); }
  if (var0 & 128) { TlStoreBool::store(stories_hide_sender_, s); }
  if (var0 & 256) { TlStoreBoxedUnknown<TlStoreObject>::store(stories_sound_, s); }
}

void inputPeerNotifySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBool::store(show_previews_, s); }
  if (var0 & 2) { TlStoreBool::store(silent_, s); }
  if (var0 & 4) { TlStoreBinary::store(mute_until_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(sound_, s); }
  if (var0 & 64) { TlStoreBool::store(stories_muted_, s); }
  if (var0 & 128) { TlStoreBool::store(stories_hide_sender_, s); }
  if (var0 & 256) { TlStoreBoxedUnknown<TlStoreObject>::store(stories_sound_, s); }
}

void inputPeerNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPeerNotifySettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("show_previews", show_previews_); }
    if (var0 & 2) { s.store_field("silent", silent_); }
    if (var0 & 4) { s.store_field("mute_until", mute_until_); }
    if (var0 & 8) { s.store_object_field("sound", static_cast<const BaseObject *>(sound_.get())); }
    if (var0 & 64) { s.store_field("stories_muted", stories_muted_); }
    if (var0 & 128) { s.store_field("stories_hide_sender", stories_hide_sender_); }
    if (var0 & 256) { s.store_object_field("stories_sound", static_cast<const BaseObject *>(stories_sound_.get())); }
    s.store_class_end();
  }
}

inputPhoneCall::inputPhoneCall(int64 id_, int64 access_hash_)
  : id_(id_)
  , access_hash_(access_hash_)
{}

const std::int32_t inputPhoneCall::ID;

void inputPhoneCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputPhoneCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
}

void inputPhoneCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPhoneCall");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_class_end();
  }
}

inputSecureValue::inputSecureValue(int32 flags_, object_ptr<SecureValueType> &&type_, object_ptr<secureData> &&data_, object_ptr<InputSecureFile> &&front_side_, object_ptr<InputSecureFile> &&reverse_side_, object_ptr<InputSecureFile> &&selfie_, array<object_ptr<InputSecureFile>> &&translation_, array<object_ptr<InputSecureFile>> &&files_, object_ptr<SecurePlainData> &&plain_data_)
  : flags_(flags_)
  , type_(std::move(type_))
  , data_(std::move(data_))
  , front_side_(std::move(front_side_))
  , reverse_side_(std::move(reverse_side_))
  , selfie_(std::move(selfie_))
  , translation_(std::move(translation_))
  , files_(std::move(files_))
  , plain_data_(std::move(plain_data_))
{}

const std::int32_t inputSecureValue::ID;

void inputSecureValue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1964327229>::store(data_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(front_side_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reverse_side_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(selfie_, s); }
  if (var0 & 64) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(translation_, s); }
  if (var0 & 16) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(files_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(plain_data_, s); }
}

void inputSecureValue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(type_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreObject, -1964327229>::store(data_, s); }
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(front_side_, s); }
  if (var0 & 4) { TlStoreBoxedUnknown<TlStoreObject>::store(reverse_side_, s); }
  if (var0 & 8) { TlStoreBoxedUnknown<TlStoreObject>::store(selfie_, s); }
  if (var0 & 64) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(translation_, s); }
  if (var0 & 16) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(files_, s); }
  if (var0 & 32) { TlStoreBoxedUnknown<TlStoreObject>::store(plain_data_, s); }
}

void inputSecureValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSecureValue");
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
    s.store_class_end();
  }
}

inputSingleMedia::inputSingleMedia(int32 flags_, object_ptr<InputMedia> &&media_, int64 random_id_, string const &message_, array<object_ptr<MessageEntity>> &&entities_)
  : flags_(flags_)
  , media_(std::move(media_))
  , random_id_(random_id_)
  , message_(message_)
  , entities_(std::move(entities_))
{}

const std::int32_t inputSingleMedia::ID;

void inputSingleMedia::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(message_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
}

void inputSingleMedia::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(message_, s);
  if (var0 & 1) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
}

void inputSingleMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSingleMedia");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_field("random_id", random_id_);
    s.store_field("message", message_);
    if (var0 & 1) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<IpPort> IpPort::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case ipPort::ID:
      return ipPort::fetch(p);
    case ipPortSecret::ID:
      return ipPortSecret::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t ipPort::ID;

object_ptr<IpPort> ipPort::fetch(TlBufferParser &p) {
  return make_tl_object<ipPort>(p);
}

ipPort::ipPort(TlBufferParser &p)
  : ipv4_(TlFetchInt::parse(p))
  , port_(TlFetchInt::parse(p))
{}

void ipPort::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "ipPort");
    s.store_field("ipv4", ipv4_);
    s.store_field("port", port_);
    s.store_class_end();
  }
}

const std::int32_t ipPortSecret::ID;

object_ptr<IpPort> ipPortSecret::fetch(TlBufferParser &p) {
  return make_tl_object<ipPortSecret>(p);
}

ipPortSecret::ipPortSecret(TlBufferParser &p)
  : ipv4_(TlFetchInt::parse(p))
  , port_(TlFetchInt::parse(p))
  , secret_(TlFetchBytes<bytes>::parse(p))
{}

void ipPortSecret::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "ipPortSecret");
    s.store_field("ipv4", ipv4_);
    s.store_field("port", port_);
    s.store_bytes_field("secret", secret_);
    s.store_class_end();
  }
}

object_ptr<MessageExtendedMedia> MessageExtendedMedia::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messageExtendedMediaPreview::ID:
      return messageExtendedMediaPreview::fetch(p);
    case messageExtendedMedia::ID:
      return messageExtendedMedia::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

messageExtendedMediaPreview::messageExtendedMediaPreview()
  : flags_()
  , w_()
  , h_()
  , thumb_()
  , video_duration_()
{}

const std::int32_t messageExtendedMediaPreview::ID;

object_ptr<MessageExtendedMedia> messageExtendedMediaPreview::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageExtendedMediaPreview> res = make_tl_object<messageExtendedMediaPreview>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->w_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->h_ = TlFetchInt::parse(p); }
  if (var0 & 2) { res->thumb_ = TlFetchObject<PhotoSize>::parse(p); }
  if (var0 & 4) { res->video_duration_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageExtendedMediaPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageExtendedMediaPreview");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("w", w_); }
    if (var0 & 1) { s.store_field("h", h_); }
    if (var0 & 2) { s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get())); }
    if (var0 & 4) { s.store_field("video_duration", video_duration_); }
    s.store_class_end();
  }
}

const std::int32_t messageExtendedMedia::ID;

object_ptr<MessageExtendedMedia> messageExtendedMedia::fetch(TlBufferParser &p) {
  return make_tl_object<messageExtendedMedia>(p);
}

messageExtendedMedia::messageExtendedMedia(TlBufferParser &p)
  : media_(TlFetchObject<MessageMedia>::parse(p))
{}

void messageExtendedMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageExtendedMedia");
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

messagePeerReaction::messagePeerReaction()
  : flags_()
  , big_()
  , unread_()
  , my_()
  , peer_id_()
  , date_()
  , reaction_()
{}

const std::int32_t messagePeerReaction::ID;

object_ptr<messagePeerReaction> messagePeerReaction::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messagePeerReaction> res = make_tl_object<messagePeerReaction>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->big_ = (var0 & 1) != 0;
  res->unread_ = (var0 & 2) != 0;
  res->my_ = (var0 & 4) != 0;
  res->peer_id_ = TlFetchObject<Peer>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->reaction_ = TlFetchObject<Reaction>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messagePeerReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePeerReaction");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (big_ << 0) | (unread_ << 1) | (my_ << 2)));
    if (var0 & 1) { s.store_field("big", true); }
    if (var0 & 2) { s.store_field("unread", true); }
    if (var0 & 4) { s.store_field("my", true); }
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    s.store_field("date", date_);
    s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get()));
    s.store_class_end();
  }
}

object_ptr<MessageReplyHeader> MessageReplyHeader::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messageReplyHeader::ID:
      return messageReplyHeader::fetch(p);
    case messageReplyStoryHeader::ID:
      return messageReplyStoryHeader::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

messageReplyHeader::messageReplyHeader()
  : flags_()
  , reply_to_scheduled_()
  , forum_topic_()
  , quote_()
  , reply_to_msg_id_()
  , reply_to_peer_id_()
  , reply_from_()
  , reply_media_()
  , reply_to_top_id_()
  , quote_text_()
  , quote_entities_()
  , quote_offset_()
  , todo_item_id_()
{}

const std::int32_t messageReplyHeader::ID;

object_ptr<MessageReplyHeader> messageReplyHeader::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messageReplyHeader> res = make_tl_object<messageReplyHeader>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->reply_to_scheduled_ = (var0 & 4) != 0;
  res->forum_topic_ = (var0 & 8) != 0;
  res->quote_ = (var0 & 512) != 0;
  if (var0 & 16) { res->reply_to_msg_id_ = TlFetchInt::parse(p); }
  if (var0 & 1) { res->reply_to_peer_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 32) { res->reply_from_ = TlFetchBoxed<TlFetchObject<messageFwdHeader>, 1313731771>::parse(p); }
  if (var0 & 256) { res->reply_media_ = TlFetchObject<MessageMedia>::parse(p); }
  if (var0 & 2) { res->reply_to_top_id_ = TlFetchInt::parse(p); }
  if (var0 & 64) { res->quote_text_ = TlFetchString<string>::parse(p); }
  if (var0 & 128) { res->quote_entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 1024) { res->quote_offset_ = TlFetchInt::parse(p); }
  if (var0 & 2048) { res->todo_item_id_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void messageReplyHeader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReplyHeader");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (reply_to_scheduled_ << 2) | (forum_topic_ << 3) | (quote_ << 9)));
    if (var0 & 4) { s.store_field("reply_to_scheduled", true); }
    if (var0 & 8) { s.store_field("forum_topic", true); }
    if (var0 & 512) { s.store_field("quote", true); }
    if (var0 & 16) { s.store_field("reply_to_msg_id", reply_to_msg_id_); }
    if (var0 & 1) { s.store_object_field("reply_to_peer_id", static_cast<const BaseObject *>(reply_to_peer_id_.get())); }
    if (var0 & 32) { s.store_object_field("reply_from", static_cast<const BaseObject *>(reply_from_.get())); }
    if (var0 & 256) { s.store_object_field("reply_media", static_cast<const BaseObject *>(reply_media_.get())); }
    if (var0 & 2) { s.store_field("reply_to_top_id", reply_to_top_id_); }
    if (var0 & 64) { s.store_field("quote_text", quote_text_); }
    if (var0 & 128) { { s.store_vector_begin("quote_entities", quote_entities_.size()); for (const auto &_value : quote_entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 1024) { s.store_field("quote_offset", quote_offset_); }
    if (var0 & 2048) { s.store_field("todo_item_id", todo_item_id_); }
    s.store_class_end();
  }
}

const std::int32_t messageReplyStoryHeader::ID;

object_ptr<MessageReplyHeader> messageReplyStoryHeader::fetch(TlBufferParser &p) {
  return make_tl_object<messageReplyStoryHeader>(p);
}

messageReplyStoryHeader::messageReplyStoryHeader(TlBufferParser &p)
  : peer_(TlFetchObject<Peer>::parse(p))
  , story_id_(TlFetchInt::parse(p))
{}

void messageReplyStoryHeader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReplyStoryHeader");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

paymentRequestedInfo::paymentRequestedInfo()
  : flags_()
  , name_()
  , phone_()
  , email_()
  , shipping_address_()
{}

paymentRequestedInfo::paymentRequestedInfo(int32 flags_, string const &name_, string const &phone_, string const &email_, object_ptr<postAddress> &&shipping_address_)
  : flags_(flags_)
  , name_(name_)
  , phone_(phone_)
  , email_(email_)
  , shipping_address_(std::move(shipping_address_))
{}

const std::int32_t paymentRequestedInfo::ID;

object_ptr<paymentRequestedInfo> paymentRequestedInfo::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<paymentRequestedInfo> res = make_tl_object<paymentRequestedInfo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->name_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->phone_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->email_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->shipping_address_ = TlFetchBoxed<TlFetchObject<postAddress>, 512535275>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void paymentRequestedInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(name_, s); }
  if (var0 & 2) { TlStoreString::store(phone_, s); }
  if (var0 & 4) { TlStoreString::store(email_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreObject, 512535275>::store(shipping_address_, s); }
}

void paymentRequestedInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreString::store(name_, s); }
  if (var0 & 2) { TlStoreString::store(phone_, s); }
  if (var0 & 4) { TlStoreString::store(email_, s); }
  if (var0 & 8) { TlStoreBoxed<TlStoreObject, 512535275>::store(shipping_address_, s); }
}

void paymentRequestedInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentRequestedInfo");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("name", name_); }
    if (var0 & 2) { s.store_field("phone", phone_); }
    if (var0 & 4) { s.store_field("email", email_); }
    if (var0 & 8) { s.store_object_field("shipping_address", static_cast<const BaseObject *>(shipping_address_.get())); }
    s.store_class_end();
  }
}

const std::int32_t peerBlocked::ID;

object_ptr<peerBlocked> peerBlocked::fetch(TlBufferParser &p) {
  return make_tl_object<peerBlocked>(p);
}

peerBlocked::peerBlocked(TlBufferParser &p)
  : peer_id_(TlFetchObject<Peer>::parse(p))
  , date_(TlFetchInt::parse(p))
{}

void peerBlocked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerBlocked");
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    s.store_field("date", date_);
    s.store_class_end();
  }
}

peerNotifySettings::peerNotifySettings()
  : flags_()
  , show_previews_()
  , silent_()
  , mute_until_()
  , ios_sound_()
  , android_sound_()
  , other_sound_()
  , stories_muted_()
  , stories_hide_sender_()
  , stories_ios_sound_()
  , stories_android_sound_()
  , stories_other_sound_()
{}

const std::int32_t peerNotifySettings::ID;

object_ptr<peerNotifySettings> peerNotifySettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<peerNotifySettings> res = make_tl_object<peerNotifySettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->show_previews_ = TlFetchBool::parse(p); }
  if (var0 & 2) { res->silent_ = TlFetchBool::parse(p); }
  if (var0 & 4) { res->mute_until_ = TlFetchInt::parse(p); }
  if (var0 & 8) { res->ios_sound_ = TlFetchObject<NotificationSound>::parse(p); }
  if (var0 & 16) { res->android_sound_ = TlFetchObject<NotificationSound>::parse(p); }
  if (var0 & 32) { res->other_sound_ = TlFetchObject<NotificationSound>::parse(p); }
  if (var0 & 64) { res->stories_muted_ = TlFetchBool::parse(p); }
  if (var0 & 128) { res->stories_hide_sender_ = TlFetchBool::parse(p); }
  if (var0 & 256) { res->stories_ios_sound_ = TlFetchObject<NotificationSound>::parse(p); }
  if (var0 & 512) { res->stories_android_sound_ = TlFetchObject<NotificationSound>::parse(p); }
  if (var0 & 1024) { res->stories_other_sound_ = TlFetchObject<NotificationSound>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void peerNotifySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "peerNotifySettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_field("show_previews", show_previews_); }
    if (var0 & 2) { s.store_field("silent", silent_); }
    if (var0 & 4) { s.store_field("mute_until", mute_until_); }
    if (var0 & 8) { s.store_object_field("ios_sound", static_cast<const BaseObject *>(ios_sound_.get())); }
    if (var0 & 16) { s.store_object_field("android_sound", static_cast<const BaseObject *>(android_sound_.get())); }
    if (var0 & 32) { s.store_object_field("other_sound", static_cast<const BaseObject *>(other_sound_.get())); }
    if (var0 & 64) { s.store_field("stories_muted", stories_muted_); }
    if (var0 & 128) { s.store_field("stories_hide_sender", stories_hide_sender_); }
    if (var0 & 256) { s.store_object_field("stories_ios_sound", static_cast<const BaseObject *>(stories_ios_sound_.get())); }
    if (var0 & 512) { s.store_object_field("stories_android_sound", static_cast<const BaseObject *>(stories_android_sound_.get())); }
    if (var0 & 1024) { s.store_object_field("stories_other_sound", static_cast<const BaseObject *>(stories_other_sound_.get())); }
    s.store_class_end();
  }
}

pollAnswerVoters::pollAnswerVoters()
  : flags_()
  , chosen_()
  , correct_()
  , option_()
  , voters_()
{}

const std::int32_t pollAnswerVoters::ID;

object_ptr<pollAnswerVoters> pollAnswerVoters::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<pollAnswerVoters> res = make_tl_object<pollAnswerVoters>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->chosen_ = (var0 & 1) != 0;
  res->correct_ = (var0 & 2) != 0;
  res->option_ = TlFetchBytes<bytes>::parse(p);
  res->voters_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void pollAnswerVoters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pollAnswerVoters");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (chosen_ << 0) | (correct_ << 1)));
    if (var0 & 1) { s.store_field("chosen", true); }
    if (var0 & 2) { s.store_field("correct", true); }
    s.store_bytes_field("option", option_);
    s.store_field("voters", voters_);
    s.store_class_end();
  }
}

object_ptr<ReplyMarkup> ReplyMarkup::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case replyKeyboardHide::ID:
      return replyKeyboardHide::fetch(p);
    case replyKeyboardForceReply::ID:
      return replyKeyboardForceReply::fetch(p);
    case replyKeyboardMarkup::ID:
      return replyKeyboardMarkup::fetch(p);
    case replyInlineMarkup::ID:
      return replyInlineMarkup::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

replyKeyboardHide::replyKeyboardHide()
  : flags_()
  , selective_()
{}

replyKeyboardHide::replyKeyboardHide(int32 flags_, bool selective_)
  : flags_(flags_)
  , selective_(selective_)
{}

const std::int32_t replyKeyboardHide::ID;

object_ptr<ReplyMarkup> replyKeyboardHide::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<replyKeyboardHide> res = make_tl_object<replyKeyboardHide>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->selective_ = (var0 & 4) != 0;
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void replyKeyboardHide::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (selective_ << 2)), s);
}

void replyKeyboardHide::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (selective_ << 2)), s);
}

void replyKeyboardHide::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyKeyboardHide");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (selective_ << 2)));
    if (var0 & 4) { s.store_field("selective", true); }
    s.store_class_end();
  }
}

replyKeyboardForceReply::replyKeyboardForceReply()
  : flags_()
  , single_use_()
  , selective_()
  , placeholder_()
{}

replyKeyboardForceReply::replyKeyboardForceReply(int32 flags_, bool single_use_, bool selective_, string const &placeholder_)
  : flags_(flags_)
  , single_use_(single_use_)
  , selective_(selective_)
  , placeholder_(placeholder_)
{}

const std::int32_t replyKeyboardForceReply::ID;

object_ptr<ReplyMarkup> replyKeyboardForceReply::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<replyKeyboardForceReply> res = make_tl_object<replyKeyboardForceReply>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->single_use_ = (var0 & 2) != 0;
  res->selective_ = (var0 & 4) != 0;
  if (var0 & 8) { res->placeholder_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void replyKeyboardForceReply::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (single_use_ << 1) | (selective_ << 2)), s);
  if (var0 & 8) { TlStoreString::store(placeholder_, s); }
}

void replyKeyboardForceReply::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (single_use_ << 1) | (selective_ << 2)), s);
  if (var0 & 8) { TlStoreString::store(placeholder_, s); }
}

void replyKeyboardForceReply::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyKeyboardForceReply");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (single_use_ << 1) | (selective_ << 2)));
    if (var0 & 2) { s.store_field("single_use", true); }
    if (var0 & 4) { s.store_field("selective", true); }
    if (var0 & 8) { s.store_field("placeholder", placeholder_); }
    s.store_class_end();
  }
}

replyKeyboardMarkup::replyKeyboardMarkup()
  : flags_()
  , resize_()
  , single_use_()
  , selective_()
  , persistent_()
  , rows_()
  , placeholder_()
{}

replyKeyboardMarkup::replyKeyboardMarkup(int32 flags_, bool resize_, bool single_use_, bool selective_, bool persistent_, array<object_ptr<keyboardButtonRow>> &&rows_, string const &placeholder_)
  : flags_(flags_)
  , resize_(resize_)
  , single_use_(single_use_)
  , selective_(selective_)
  , persistent_(persistent_)
  , rows_(std::move(rows_))
  , placeholder_(placeholder_)
{}

const std::int32_t replyKeyboardMarkup::ID;

object_ptr<ReplyMarkup> replyKeyboardMarkup::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<replyKeyboardMarkup> res = make_tl_object<replyKeyboardMarkup>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->resize_ = (var0 & 1) != 0;
  res->single_use_ = (var0 & 2) != 0;
  res->selective_ = (var0 & 4) != 0;
  res->persistent_ = (var0 & 16) != 0;
  res->rows_ = TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<keyboardButtonRow>, 2002815875>>, 481674261>::parse(p);
  if (var0 & 8) { res->placeholder_ = TlFetchString<string>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void replyKeyboardMarkup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (resize_ << 0) | (single_use_ << 1) | (selective_ << 2) | (persistent_ << 4)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 2002815875>>, 481674261>::store(rows_, s);
  if (var0 & 8) { TlStoreString::store(placeholder_, s); }
}

void replyKeyboardMarkup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_ | (resize_ << 0) | (single_use_ << 1) | (selective_ << 2) | (persistent_ << 4)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 2002815875>>, 481674261>::store(rows_, s);
  if (var0 & 8) { TlStoreString::store(placeholder_, s); }
}

void replyKeyboardMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyKeyboardMarkup");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (resize_ << 0) | (single_use_ << 1) | (selective_ << 2) | (persistent_ << 4)));
    if (var0 & 1) { s.store_field("resize", true); }
    if (var0 & 2) { s.store_field("single_use", true); }
    if (var0 & 4) { s.store_field("selective", true); }
    if (var0 & 16) { s.store_field("persistent", true); }
    { s.store_vector_begin("rows", rows_.size()); for (const auto &_value : rows_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 8) { s.store_field("placeholder", placeholder_); }
    s.store_class_end();
  }
}

replyInlineMarkup::replyInlineMarkup(array<object_ptr<keyboardButtonRow>> &&rows_)
  : rows_(std::move(rows_))
{}

const std::int32_t replyInlineMarkup::ID;

object_ptr<ReplyMarkup> replyInlineMarkup::fetch(TlBufferParser &p) {
  return make_tl_object<replyInlineMarkup>(p);
}

replyInlineMarkup::replyInlineMarkup(TlBufferParser &p)
  : rows_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<keyboardButtonRow>, 2002815875>>, 481674261>::parse(p))
{}

void replyInlineMarkup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 2002815875>>, 481674261>::store(rows_, s);
}

void replyInlineMarkup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxed<TlStoreObject, 2002815875>>, 481674261>::store(rows_, s);
}

void replyInlineMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyInlineMarkup");
    { s.store_vector_begin("rows", rows_.size()); for (const auto &_value : rows_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<RequestedPeer> RequestedPeer::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case requestedPeerUser::ID:
      return requestedPeerUser::fetch(p);
    case requestedPeerChat::ID:
      return requestedPeerChat::fetch(p);
    case requestedPeerChannel::ID:
      return requestedPeerChannel::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

requestedPeerUser::requestedPeerUser()
  : flags_()
  , user_id_()
  , first_name_()
  , last_name_()
  , username_()
  , photo_()
{}

const std::int32_t requestedPeerUser::ID;

object_ptr<RequestedPeer> requestedPeerUser::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<requestedPeerUser> res = make_tl_object<requestedPeerUser>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->user_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->first_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 1) { res->last_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->username_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->photo_ = TlFetchObject<Photo>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void requestedPeerUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestedPeerUser");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("user_id", user_id_);
    if (var0 & 1) { s.store_field("first_name", first_name_); }
    if (var0 & 1) { s.store_field("last_name", last_name_); }
    if (var0 & 2) { s.store_field("username", username_); }
    if (var0 & 4) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_class_end();
  }
}

requestedPeerChat::requestedPeerChat()
  : flags_()
  , chat_id_()
  , title_()
  , photo_()
{}

const std::int32_t requestedPeerChat::ID;

object_ptr<RequestedPeer> requestedPeerChat::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<requestedPeerChat> res = make_tl_object<requestedPeerChat>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->chat_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->photo_ = TlFetchObject<Photo>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void requestedPeerChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestedPeerChat");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("chat_id", chat_id_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 4) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_class_end();
  }
}

requestedPeerChannel::requestedPeerChannel()
  : flags_()
  , channel_id_()
  , title_()
  , username_()
  , photo_()
{}

const std::int32_t requestedPeerChannel::ID;

object_ptr<RequestedPeer> requestedPeerChannel::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<requestedPeerChannel> res = make_tl_object<requestedPeerChannel>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->channel_id_ = TlFetchLong::parse(p);
  if (var0 & 1) { res->title_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->username_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->photo_ = TlFetchObject<Photo>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void requestedPeerChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestedPeerChannel");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("channel_id", channel_id_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("username", username_); }
    if (var0 & 4) { s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get())); }
    s.store_class_end();
  }
}

const std::int32_t searchResultPosition::ID;

object_ptr<searchResultPosition> searchResultPosition::fetch(TlBufferParser &p) {
  return make_tl_object<searchResultPosition>(p);
}

searchResultPosition::searchResultPosition(TlBufferParser &p)
  : msg_id_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
  , offset_(TlFetchInt::parse(p))
{}

void searchResultPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchResultPosition");
    s.store_field("msg_id", msg_id_);
    s.store_field("date", date_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

object_ptr<StarGiftAttribute> StarGiftAttribute::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case starGiftAttributeModel::ID:
      return starGiftAttributeModel::fetch(p);
    case starGiftAttributePattern::ID:
      return starGiftAttributePattern::fetch(p);
    case starGiftAttributeBackdrop::ID:
      return starGiftAttributeBackdrop::fetch(p);
    case starGiftAttributeOriginalDetails::ID:
      return starGiftAttributeOriginalDetails::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t starGiftAttributeModel::ID;

object_ptr<StarGiftAttribute> starGiftAttributeModel::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftAttributeModel>(p);
}

starGiftAttributeModel::starGiftAttributeModel(TlBufferParser &p)
  : name_(TlFetchString<string>::parse(p))
  , document_(TlFetchObject<Document>::parse(p))
  , rarity_permille_(TlFetchInt::parse(p))
{}

void starGiftAttributeModel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributeModel");
    s.store_field("name", name_);
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_field("rarity_permille", rarity_permille_);
    s.store_class_end();
  }
}

const std::int32_t starGiftAttributePattern::ID;

object_ptr<StarGiftAttribute> starGiftAttributePattern::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftAttributePattern>(p);
}

starGiftAttributePattern::starGiftAttributePattern(TlBufferParser &p)
  : name_(TlFetchString<string>::parse(p))
  , document_(TlFetchObject<Document>::parse(p))
  , rarity_permille_(TlFetchInt::parse(p))
{}

void starGiftAttributePattern::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributePattern");
    s.store_field("name", name_);
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_field("rarity_permille", rarity_permille_);
    s.store_class_end();
  }
}

const std::int32_t starGiftAttributeBackdrop::ID;

object_ptr<StarGiftAttribute> starGiftAttributeBackdrop::fetch(TlBufferParser &p) {
  return make_tl_object<starGiftAttributeBackdrop>(p);
}

starGiftAttributeBackdrop::starGiftAttributeBackdrop(TlBufferParser &p)
  : name_(TlFetchString<string>::parse(p))
  , backdrop_id_(TlFetchInt::parse(p))
  , center_color_(TlFetchInt::parse(p))
  , edge_color_(TlFetchInt::parse(p))
  , pattern_color_(TlFetchInt::parse(p))
  , text_color_(TlFetchInt::parse(p))
  , rarity_permille_(TlFetchInt::parse(p))
{}

void starGiftAttributeBackdrop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributeBackdrop");
    s.store_field("name", name_);
    s.store_field("backdrop_id", backdrop_id_);
    s.store_field("center_color", center_color_);
    s.store_field("edge_color", edge_color_);
    s.store_field("pattern_color", pattern_color_);
    s.store_field("text_color", text_color_);
    s.store_field("rarity_permille", rarity_permille_);
    s.store_class_end();
  }
}

starGiftAttributeOriginalDetails::starGiftAttributeOriginalDetails()
  : flags_()
  , sender_id_()
  , recipient_id_()
  , date_()
  , message_()
{}

const std::int32_t starGiftAttributeOriginalDetails::ID;

object_ptr<StarGiftAttribute> starGiftAttributeOriginalDetails::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starGiftAttributeOriginalDetails> res = make_tl_object<starGiftAttributeOriginalDetails>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->sender_id_ = TlFetchObject<Peer>::parse(p); }
  res->recipient_id_ = TlFetchObject<Peer>::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 2) { res->message_ = TlFetchBoxed<TlFetchObject<textWithEntities>, 1964978502>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void starGiftAttributeOriginalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiftAttributeOriginalDetails");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get())); }
    s.store_object_field("recipient_id", static_cast<const BaseObject *>(recipient_id_.get()));
    s.store_field("date", date_);
    if (var0 & 2) { s.store_object_field("message", static_cast<const BaseObject *>(message_.get())); }
    s.store_class_end();
  }
}

starsTopupOption::starsTopupOption()
  : flags_()
  , extended_()
  , stars_()
  , store_product_()
  , currency_()
  , amount_()
{}

const std::int32_t starsTopupOption::ID;

object_ptr<starsTopupOption> starsTopupOption::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<starsTopupOption> res = make_tl_object<starsTopupOption>();
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

void starsTopupOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starsTopupOption");
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

const std::int32_t statsURL::ID;

void statsURL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statsURL");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

object_ptr<StoryItem> StoryItem::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case storyItemDeleted::ID:
      return storyItemDeleted::fetch(p);
    case storyItemSkipped::ID:
      return storyItemSkipped::fetch(p);
    case storyItem::ID:
      return storyItem::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t storyItemDeleted::ID;

object_ptr<StoryItem> storyItemDeleted::fetch(TlBufferParser &p) {
  return make_tl_object<storyItemDeleted>(p);
}

storyItemDeleted::storyItemDeleted(TlBufferParser &p)
  : id_(TlFetchInt::parse(p))
{}

void storyItemDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyItemDeleted");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

storyItemSkipped::storyItemSkipped()
  : flags_()
  , close_friends_()
  , id_()
  , date_()
  , expire_date_()
{}

const std::int32_t storyItemSkipped::ID;

object_ptr<StoryItem> storyItemSkipped::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyItemSkipped> res = make_tl_object<storyItemSkipped>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->close_friends_ = (var0 & 256) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  res->expire_date_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void storyItemSkipped::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyItemSkipped");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (close_friends_ << 8)));
    if (var0 & 256) { s.store_field("close_friends", true); }
    s.store_field("id", id_);
    s.store_field("date", date_);
    s.store_field("expire_date", expire_date_);
    s.store_class_end();
  }
}

storyItem::storyItem()
  : flags_()
  , pinned_()
  , public_()
  , close_friends_()
  , min_()
  , noforwards_()
  , edited_()
  , contacts_()
  , selected_contacts_()
  , out_()
  , id_()
  , date_()
  , from_id_()
  , fwd_from_()
  , expire_date_()
  , caption_()
  , entities_()
  , media_()
  , media_areas_()
  , privacy_()
  , views_()
  , sent_reaction_()
  , albums_()
{}

const std::int32_t storyItem::ID;

object_ptr<StoryItem> storyItem::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyItem> res = make_tl_object<storyItem>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->pinned_ = (var0 & 32) != 0;
  res->public_ = (var0 & 128) != 0;
  res->close_friends_ = (var0 & 256) != 0;
  res->min_ = (var0 & 512) != 0;
  res->noforwards_ = (var0 & 1024) != 0;
  res->edited_ = (var0 & 2048) != 0;
  res->contacts_ = (var0 & 4096) != 0;
  res->selected_contacts_ = (var0 & 8192) != 0;
  res->out_ = (var0 & 65536) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 262144) { res->from_id_ = TlFetchObject<Peer>::parse(p); }
  if (var0 & 131072) { res->fwd_from_ = TlFetchBoxed<TlFetchObject<storyFwdHeader>, -1205411504>::parse(p); }
  res->expire_date_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->caption_ = TlFetchString<string>::parse(p); }
  if (var0 & 2) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  res->media_ = TlFetchObject<MessageMedia>::parse(p);
  if (var0 & 16384) { res->media_areas_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MediaArea>>, 481674261>::parse(p); }
  if (var0 & 4) { res->privacy_ = TlFetchBoxed<TlFetchVector<TlFetchObject<PrivacyRule>>, 481674261>::parse(p); }
  if (var0 & 8) { res->views_ = TlFetchBoxed<TlFetchObject<storyViews>, -1923523370>::parse(p); }
  if (var0 & 32768) { res->sent_reaction_ = TlFetchObject<Reaction>::parse(p); }
  if (var0 & 524288) { res->albums_ = TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void storyItem::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyItem");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (pinned_ << 5) | (public_ << 7) | (close_friends_ << 8) | (min_ << 9) | (noforwards_ << 10) | (edited_ << 11) | (contacts_ << 12) | (selected_contacts_ << 13) | (out_ << 16)));
    if (var0 & 32) { s.store_field("pinned", true); }
    if (var0 & 128) { s.store_field("public", true); }
    if (var0 & 256) { s.store_field("close_friends", true); }
    if (var0 & 512) { s.store_field("min", true); }
    if (var0 & 1024) { s.store_field("noforwards", true); }
    if (var0 & 2048) { s.store_field("edited", true); }
    if (var0 & 4096) { s.store_field("contacts", true); }
    if (var0 & 8192) { s.store_field("selected_contacts", true); }
    if (var0 & 65536) { s.store_field("out", true); }
    s.store_field("id", id_);
    s.store_field("date", date_);
    if (var0 & 262144) { s.store_object_field("from_id", static_cast<const BaseObject *>(from_id_.get())); }
    if (var0 & 131072) { s.store_object_field("fwd_from", static_cast<const BaseObject *>(fwd_from_.get())); }
    s.store_field("expire_date", expire_date_);
    if (var0 & 1) { s.store_field("caption", caption_); }
    if (var0 & 2) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    if (var0 & 16384) { { s.store_vector_begin("media_areas", media_areas_.size()); for (const auto &_value : media_areas_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 4) { { s.store_vector_begin("privacy", privacy_.size()); for (const auto &_value : privacy_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 8) { s.store_object_field("views", static_cast<const BaseObject *>(views_.get())); }
    if (var0 & 32768) { s.store_object_field("sent_reaction", static_cast<const BaseObject *>(sent_reaction_.get())); }
    if (var0 & 524288) { { s.store_vector_begin("albums", albums_.size()); for (const auto &_value : albums_) { s.store_field("", _value); } s.store_class_end(); } }
    s.store_class_end();
  }
}

object_ptr<StoryView> StoryView::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case storyView::ID:
      return storyView::fetch(p);
    case storyViewPublicForward::ID:
      return storyViewPublicForward::fetch(p);
    case storyViewPublicRepost::ID:
      return storyViewPublicRepost::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

storyView::storyView()
  : flags_()
  , blocked_()
  , blocked_my_stories_from_()
  , user_id_()
  , date_()
  , reaction_()
{}

const std::int32_t storyView::ID;

object_ptr<StoryView> storyView::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyView> res = make_tl_object<storyView>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->blocked_ = (var0 & 1) != 0;
  res->blocked_my_stories_from_ = (var0 & 2) != 0;
  res->user_id_ = TlFetchLong::parse(p);
  res->date_ = TlFetchInt::parse(p);
  if (var0 & 4) { res->reaction_ = TlFetchObject<Reaction>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void storyView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyView");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (blocked_ << 0) | (blocked_my_stories_from_ << 1)));
    if (var0 & 1) { s.store_field("blocked", true); }
    if (var0 & 2) { s.store_field("blocked_my_stories_from", true); }
    s.store_field("user_id", user_id_);
    s.store_field("date", date_);
    if (var0 & 4) { s.store_object_field("reaction", static_cast<const BaseObject *>(reaction_.get())); }
    s.store_class_end();
  }
}

storyViewPublicForward::storyViewPublicForward()
  : flags_()
  , blocked_()
  , blocked_my_stories_from_()
  , message_()
{}

const std::int32_t storyViewPublicForward::ID;

object_ptr<StoryView> storyViewPublicForward::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyViewPublicForward> res = make_tl_object<storyViewPublicForward>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->blocked_ = (var0 & 1) != 0;
  res->blocked_my_stories_from_ = (var0 & 2) != 0;
  res->message_ = TlFetchObject<Message>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void storyViewPublicForward::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyViewPublicForward");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (blocked_ << 0) | (blocked_my_stories_from_ << 1)));
    if (var0 & 1) { s.store_field("blocked", true); }
    if (var0 & 2) { s.store_field("blocked_my_stories_from", true); }
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

storyViewPublicRepost::storyViewPublicRepost()
  : flags_()
  , blocked_()
  , blocked_my_stories_from_()
  , peer_id_()
  , story_()
{}

const std::int32_t storyViewPublicRepost::ID;

object_ptr<StoryView> storyViewPublicRepost::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<storyViewPublicRepost> res = make_tl_object<storyViewPublicRepost>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->blocked_ = (var0 & 1) != 0;
  res->blocked_my_stories_from_ = (var0 & 2) != 0;
  res->peer_id_ = TlFetchObject<Peer>::parse(p);
  res->story_ = TlFetchObject<StoryItem>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void storyViewPublicRepost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyViewPublicRepost");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (blocked_ << 0) | (blocked_my_stories_from_ << 1)));
    if (var0 & 1) { s.store_field("blocked", true); }
    if (var0 & 2) { s.store_field("blocked_my_stories_from", true); }
    s.store_object_field("peer_id", static_cast<const BaseObject *>(peer_id_.get()));
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

object_ptr<VideoSize> VideoSize::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case videoSize::ID:
      return videoSize::fetch(p);
    case videoSizeEmojiMarkup::ID:
      return videoSizeEmojiMarkup::fetch(p);
    case videoSizeStickerMarkup::ID:
      return videoSizeStickerMarkup::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

videoSize::videoSize()
  : flags_()
  , type_()
  , w_()
  , h_()
  , size_()
  , video_start_ts_()
{}

videoSize::videoSize(int32 flags_, string const &type_, int32 w_, int32 h_, int32 size_, double video_start_ts_)
  : flags_(flags_)
  , type_(type_)
  , w_(w_)
  , h_(h_)
  , size_(size_)
  , video_start_ts_(video_start_ts_)
{}

const std::int32_t videoSize::ID;

object_ptr<VideoSize> videoSize::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<videoSize> res = make_tl_object<videoSize>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->type_ = TlFetchString<string>::parse(p);
  res->w_ = TlFetchInt::parse(p);
  res->h_ = TlFetchInt::parse(p);
  res->size_ = TlFetchInt::parse(p);
  if (var0 & 1) { res->video_start_ts_ = TlFetchDouble::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void videoSize::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(type_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  if (var0 & 1) { TlStoreBinary::store(video_start_ts_, s); }
}

void videoSize::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(type_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  if (var0 & 1) { TlStoreBinary::store(video_start_ts_, s); }
}

void videoSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoSize");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("type", type_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    if (var0 & 1) { s.store_field("video_start_ts", video_start_ts_); }
    s.store_class_end();
  }
}

videoSizeEmojiMarkup::videoSizeEmojiMarkup(int64 emoji_id_, array<int32> &&background_colors_)
  : emoji_id_(emoji_id_)
  , background_colors_(std::move(background_colors_))
{}

const std::int32_t videoSizeEmojiMarkup::ID;

object_ptr<VideoSize> videoSizeEmojiMarkup::fetch(TlBufferParser &p) {
  return make_tl_object<videoSizeEmojiMarkup>(p);
}

videoSizeEmojiMarkup::videoSizeEmojiMarkup(TlBufferParser &p)
  : emoji_id_(TlFetchLong::parse(p))
  , background_colors_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void videoSizeEmojiMarkup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(emoji_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(background_colors_, s);
}

void videoSizeEmojiMarkup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(emoji_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(background_colors_, s);
}

void videoSizeEmojiMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoSizeEmojiMarkup");
    s.store_field("emoji_id", emoji_id_);
    { s.store_vector_begin("background_colors", background_colors_.size()); for (const auto &_value : background_colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

videoSizeStickerMarkup::videoSizeStickerMarkup(object_ptr<InputStickerSet> &&stickerset_, int64 sticker_id_, array<int32> &&background_colors_)
  : stickerset_(std::move(stickerset_))
  , sticker_id_(sticker_id_)
  , background_colors_(std::move(background_colors_))
{}

const std::int32_t videoSizeStickerMarkup::ID;

object_ptr<VideoSize> videoSizeStickerMarkup::fetch(TlBufferParser &p) {
  return make_tl_object<videoSizeStickerMarkup>(p);
}

videoSizeStickerMarkup::videoSizeStickerMarkup(TlBufferParser &p)
  : stickerset_(TlFetchObject<InputStickerSet>::parse(p))
  , sticker_id_(TlFetchLong::parse(p))
  , background_colors_(TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p))
{}

void videoSizeStickerMarkup::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(sticker_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(background_colors_, s);
}

void videoSizeStickerMarkup::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(sticker_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(background_colors_, s);
}

void videoSizeStickerMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoSizeStickerMarkup");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_field("sticker_id", sticker_id_);
    { s.store_vector_begin("background_colors", background_colors_.size()); for (const auto &_value : background_colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

webViewMessageSent::webViewMessageSent()
  : flags_()
  , msg_id_()
{}

const std::int32_t webViewMessageSent::ID;

object_ptr<webViewMessageSent> webViewMessageSent::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<webViewMessageSent> res = make_tl_object<webViewMessageSent>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->msg_id_ = TlFetchObject<InputBotInlineMessageID>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void webViewMessageSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webViewMessageSent");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("msg_id", static_cast<const BaseObject *>(msg_id_.get())); }
    s.store_class_end();
  }
}

account_contentSettings::account_contentSettings()
  : flags_()
  , sensitive_enabled_()
  , sensitive_can_change_()
{}

const std::int32_t account_contentSettings::ID;

object_ptr<account_contentSettings> account_contentSettings::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<account_contentSettings> res = make_tl_object<account_contentSettings>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->sensitive_enabled_ = (var0 & 1) != 0;
  res->sensitive_can_change_ = (var0 & 2) != 0;
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void account_contentSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.contentSettings");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (sensitive_enabled_ << 0) | (sensitive_can_change_ << 1)));
    if (var0 & 1) { s.store_field("sensitive_enabled", true); }
    if (var0 & 2) { s.store_field("sensitive_can_change", true); }
    s.store_class_end();
  }
}

object_ptr<account_SavedMusicIds> account_SavedMusicIds::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case account_savedMusicIdsNotModified::ID:
      return account_savedMusicIdsNotModified::fetch(p);
    case account_savedMusicIds::ID:
      return account_savedMusicIds::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t account_savedMusicIdsNotModified::ID;

object_ptr<account_SavedMusicIds> account_savedMusicIdsNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<account_savedMusicIdsNotModified>();
}

void account_savedMusicIdsNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.savedMusicIdsNotModified");
    s.store_class_end();
  }
}

const std::int32_t account_savedMusicIds::ID;

object_ptr<account_SavedMusicIds> account_savedMusicIds::fetch(TlBufferParser &p) {
  return make_tl_object<account_savedMusicIds>(p);
}

account_savedMusicIds::account_savedMusicIds(TlBufferParser &p)
  : ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
{}

void account_savedMusicIds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.savedMusicIds");
    { s.store_vector_begin("ids", ids_.size()); for (const auto &_value : ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t account_tmpPassword::ID;

object_ptr<account_tmpPassword> account_tmpPassword::fetch(TlBufferParser &p) {
  return make_tl_object<account_tmpPassword>(p);
}

account_tmpPassword::account_tmpPassword(TlBufferParser &p)
  : tmp_password_(TlFetchBytes<bytes>::parse(p))
  , valid_until_(TlFetchInt::parse(p))
{}

void account_tmpPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.tmpPassword");
    s.store_bytes_field("tmp_password", tmp_password_);
    s.store_field("valid_until", valid_until_);
    s.store_class_end();
  }
}

object_ptr<auth_SentCode> auth_SentCode::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case auth_sentCode::ID:
      return auth_sentCode::fetch(p);
    case auth_sentCodeSuccess::ID:
      return auth_sentCodeSuccess::fetch(p);
    case auth_sentCodePaymentRequired::ID:
      return auth_sentCodePaymentRequired::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

auth_sentCode::auth_sentCode()
  : flags_()
  , type_()
  , phone_code_hash_()
  , next_type_()
  , timeout_()
{}

const std::int32_t auth_sentCode::ID;

object_ptr<auth_SentCode> auth_sentCode::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<auth_sentCode> res = make_tl_object<auth_sentCode>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->type_ = TlFetchObject<auth_SentCodeType>::parse(p);
  res->phone_code_hash_ = TlFetchString<string>::parse(p);
  if (var0 & 2) { res->next_type_ = TlFetchObject<auth_CodeType>::parse(p); }
  if (var0 & 4) { res->timeout_ = TlFetchInt::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void auth_sentCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCode");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("phone_code_hash", phone_code_hash_);
    if (var0 & 2) { s.store_object_field("next_type", static_cast<const BaseObject *>(next_type_.get())); }
    if (var0 & 4) { s.store_field("timeout", timeout_); }
    s.store_class_end();
  }
}

const std::int32_t auth_sentCodeSuccess::ID;

object_ptr<auth_SentCode> auth_sentCodeSuccess::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodeSuccess>(p);
}

auth_sentCodeSuccess::auth_sentCodeSuccess(TlBufferParser &p)
  : authorization_(TlFetchObject<auth_Authorization>::parse(p))
{}

void auth_sentCodeSuccess::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodeSuccess");
    s.store_object_field("authorization", static_cast<const BaseObject *>(authorization_.get()));
    s.store_class_end();
  }
}

const std::int32_t auth_sentCodePaymentRequired::ID;

object_ptr<auth_SentCode> auth_sentCodePaymentRequired::fetch(TlBufferParser &p) {
  return make_tl_object<auth_sentCodePaymentRequired>(p);
}

auth_sentCodePaymentRequired::auth_sentCodePaymentRequired(TlBufferParser &p)
  : store_product_(TlFetchString<string>::parse(p))
  , phone_code_hash_(TlFetchString<string>::parse(p))
  , support_email_address_(TlFetchString<string>::parse(p))
  , support_email_subject_(TlFetchString<string>::parse(p))
  , currency_(TlFetchString<string>::parse(p))
  , amount_(TlFetchLong::parse(p))
{}

void auth_sentCodePaymentRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.sentCodePaymentRequired");
    s.store_field("store_product", store_product_);
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("support_email_address", support_email_address_);
    s.store_field("support_email_subject", support_email_subject_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

const std::int32_t bots_botInfo::ID;

object_ptr<bots_botInfo> bots_botInfo::fetch(TlBufferParser &p) {
  return make_tl_object<bots_botInfo>(p);
}

bots_botInfo::bots_botInfo(TlBufferParser &p)
  : name_(TlFetchString<string>::parse(p))
  , about_(TlFetchString<string>::parse(p))
  , description_(TlFetchString<string>::parse(p))
{}

void bots_botInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.botInfo");
    s.store_field("name", name_);
    s.store_field("about", about_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

const std::int32_t chatlists_chatlistUpdates::ID;

object_ptr<chatlists_chatlistUpdates> chatlists_chatlistUpdates::fetch(TlBufferParser &p) {
  return make_tl_object<chatlists_chatlistUpdates>(p);
}

chatlists_chatlistUpdates::chatlists_chatlistUpdates(TlBufferParser &p)
  : missing_peers_(TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void chatlists_chatlistUpdates::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.chatlistUpdates");
    { s.store_vector_begin("missing_peers", missing_peers_.size()); for (const auto &_value : missing_peers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t chatlists_exportedChatlistInvite::ID;

object_ptr<chatlists_exportedChatlistInvite> chatlists_exportedChatlistInvite::fetch(TlBufferParser &p) {
  return make_tl_object<chatlists_exportedChatlistInvite>(p);
}

chatlists_exportedChatlistInvite::chatlists_exportedChatlistInvite(TlBufferParser &p)
  : filter_(TlFetchObject<DialogFilter>::parse(p))
  , invite_(TlFetchBoxed<TlFetchObject<exportedChatlistInvite>, 206668204>::parse(p))
{}

void chatlists_exportedChatlistInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatlists.exportedChatlistInvite");
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_object_field("invite", static_cast<const BaseObject *>(invite_.get()));
    s.store_class_end();
  }
}

const std::int32_t contacts_found::ID;

object_ptr<contacts_found> contacts_found::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_found>(p);
}

contacts_found::contacts_found(TlBufferParser &p)
  : my_results_(TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p))
  , results_(TlFetchBoxed<TlFetchVector<TlFetchObject<Peer>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_found::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.found");
    { s.store_vector_begin("my_results", my_results_.size()); for (const auto &_value : my_results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("results", results_.size()); for (const auto &_value : results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<contacts_TopPeers> contacts_TopPeers::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case contacts_topPeersNotModified::ID:
      return contacts_topPeersNotModified::fetch(p);
    case contacts_topPeers::ID:
      return contacts_topPeers::fetch(p);
    case contacts_topPeersDisabled::ID:
      return contacts_topPeersDisabled::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t contacts_topPeersNotModified::ID;

object_ptr<contacts_TopPeers> contacts_topPeersNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_topPeersNotModified>();
}

void contacts_topPeersNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.topPeersNotModified");
    s.store_class_end();
  }
}

const std::int32_t contacts_topPeers::ID;

object_ptr<contacts_TopPeers> contacts_topPeers::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_topPeers>(p);
}

contacts_topPeers::contacts_topPeers(TlBufferParser &p)
  : categories_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<topPeerCategoryPeers>, -75283823>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void contacts_topPeers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.topPeers");
    { s.store_vector_begin("categories", categories_.size()); for (const auto &_value : categories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t contacts_topPeersDisabled::ID;

object_ptr<contacts_TopPeers> contacts_topPeersDisabled::fetch(TlBufferParser &p) {
  return make_tl_object<contacts_topPeersDisabled>();
}

void contacts_topPeersDisabled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.topPeersDisabled");
    s.store_class_end();
  }
}

object_ptr<help_AppUpdate> help_AppUpdate::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_appUpdate::ID:
      return help_appUpdate::fetch(p);
    case help_noAppUpdate::ID:
      return help_noAppUpdate::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

help_appUpdate::help_appUpdate()
  : flags_()
  , can_not_skip_()
  , id_()
  , version_()
  , text_()
  , entities_()
  , document_()
  , url_()
  , sticker_()
{}

const std::int32_t help_appUpdate::ID;

object_ptr<help_AppUpdate> help_appUpdate::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<help_appUpdate> res = make_tl_object<help_appUpdate>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->can_not_skip_ = (var0 & 1) != 0;
  res->id_ = TlFetchInt::parse(p);
  res->version_ = TlFetchString<string>::parse(p);
  res->text_ = TlFetchString<string>::parse(p);
  res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p);
  if (var0 & 2) { res->document_ = TlFetchObject<Document>::parse(p); }
  if (var0 & 4) { res->url_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->sticker_ = TlFetchObject<Document>::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void help_appUpdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.appUpdate");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (can_not_skip_ << 0)));
    if (var0 & 1) { s.store_field("can_not_skip", true); }
    s.store_field("id", id_);
    s.store_field("version", version_);
    s.store_field("text", text_);
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    if (var0 & 2) { s.store_object_field("document", static_cast<const BaseObject *>(document_.get())); }
    if (var0 & 4) { s.store_field("url", url_); }
    if (var0 & 8) { s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get())); }
    s.store_class_end();
  }
}

const std::int32_t help_noAppUpdate::ID;

object_ptr<help_AppUpdate> help_noAppUpdate::fetch(TlBufferParser &p) {
  return make_tl_object<help_noAppUpdate>();
}

void help_noAppUpdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.noAppUpdate");
    s.store_class_end();
  }
}

object_ptr<help_CountriesList> help_CountriesList::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_countriesListNotModified::ID:
      return help_countriesListNotModified::fetch(p);
    case help_countriesList::ID:
      return help_countriesList::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_countriesListNotModified::ID;

object_ptr<help_CountriesList> help_countriesListNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<help_countriesListNotModified>();
}

void help_countriesListNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.countriesListNotModified");
    s.store_class_end();
  }
}

const std::int32_t help_countriesList::ID;

object_ptr<help_CountriesList> help_countriesList::fetch(TlBufferParser &p) {
  return make_tl_object<help_countriesList>(p);
}

help_countriesList::help_countriesList(TlBufferParser &p)
  : countries_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<help_country>, -1014526429>>, 481674261>::parse(p))
  , hash_(TlFetchInt::parse(p))
{}

void help_countriesList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.countriesList");
    { s.store_vector_begin("countries", countries_.size()); for (const auto &_value : countries_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

object_ptr<help_PassportConfig> help_PassportConfig::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case help_passportConfigNotModified::ID:
      return help_passportConfigNotModified::fetch(p);
    case help_passportConfig::ID:
      return help_passportConfig::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t help_passportConfigNotModified::ID;

object_ptr<help_PassportConfig> help_passportConfigNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<help_passportConfigNotModified>();
}

void help_passportConfigNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.passportConfigNotModified");
    s.store_class_end();
  }
}

const std::int32_t help_passportConfig::ID;

object_ptr<help_PassportConfig> help_passportConfig::fetch(TlBufferParser &p) {
  return make_tl_object<help_passportConfig>(p);
}

help_passportConfig::help_passportConfig(TlBufferParser &p)
  : hash_(TlFetchInt::parse(p))
  , countries_langs_(TlFetchBoxed<TlFetchObject<dataJSON>, 2104790276>::parse(p))
{}

void help_passportConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.passportConfig");
    s.store_field("hash", hash_);
    s.store_object_field("countries_langs", static_cast<const BaseObject *>(countries_langs_.get()));
    s.store_class_end();
  }
}

messages_botResults::messages_botResults()
  : flags_()
  , gallery_()
  , query_id_()
  , next_offset_()
  , switch_pm_()
  , switch_webview_()
  , results_()
  , cache_time_()
  , users_()
{}

const std::int32_t messages_botResults::ID;

object_ptr<messages_botResults> messages_botResults::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_botResults> res = make_tl_object<messages_botResults>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->gallery_ = (var0 & 1) != 0;
  res->query_id_ = TlFetchLong::parse(p);
  if (var0 & 2) { res->next_offset_ = TlFetchString<string>::parse(p); }
  if (var0 & 4) { res->switch_pm_ = TlFetchBoxed<TlFetchObject<inlineBotSwitchPM>, 1008755359>::parse(p); }
  if (var0 & 8) { res->switch_webview_ = TlFetchBoxed<TlFetchObject<inlineBotWebView>, -1250781739>::parse(p); }
  res->results_ = TlFetchBoxed<TlFetchVector<TlFetchObject<BotInlineResult>>, 481674261>::parse(p);
  res->cache_time_ = TlFetchInt::parse(p);
  res->users_ = TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p);
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_botResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.botResults");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (gallery_ << 0)));
    if (var0 & 1) { s.store_field("gallery", true); }
    s.store_field("query_id", query_id_);
    if (var0 & 2) { s.store_field("next_offset", next_offset_); }
    if (var0 & 4) { s.store_object_field("switch_pm", static_cast<const BaseObject *>(switch_pm_.get())); }
    if (var0 & 8) { s.store_object_field("switch_webview", static_cast<const BaseObject *>(switch_webview_.get())); }
    { s.store_vector_begin("results", results_.size()); for (const auto &_value : results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("cache_time", cache_time_);
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

object_ptr<messages_Chats> messages_Chats::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_chats::ID:
      return messages_chats::fetch(p);
    case messages_chatsSlice::ID:
      return messages_chatsSlice::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_chats::ID;

object_ptr<messages_Chats> messages_chats::fetch(TlBufferParser &p) {
  return make_tl_object<messages_chats>(p);
}

messages_chats::messages_chats(TlBufferParser &p)
  : chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
{}

void messages_chats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.chats");
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_chatsSlice::ID;

object_ptr<messages_Chats> messages_chatsSlice::fetch(TlBufferParser &p) {
  return make_tl_object<messages_chatsSlice>(p);
}

messages_chatsSlice::messages_chatsSlice(TlBufferParser &p)
  : count_(TlFetchInt::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
{}

void messages_chatsSlice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.chatsSlice");
    s.store_field("count", count_);
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_invitedUsers::ID;

object_ptr<messages_invitedUsers> messages_invitedUsers::fetch(TlBufferParser &p) {
  return make_tl_object<messages_invitedUsers>(p);
}

messages_invitedUsers::messages_invitedUsers(TlBufferParser &p)
  : updates_(TlFetchObject<Updates>::parse(p))
  , missing_invitees_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<missingInvitee>, 1653379620>>, 481674261>::parse(p))
{}

void messages_invitedUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.invitedUsers");
    s.store_object_field("updates", static_cast<const BaseObject *>(updates_.get()));
    { s.store_vector_begin("missing_invitees", missing_invitees_.size()); for (const auto &_value : missing_invitees_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_messageEditData::messages_messageEditData()
  : flags_()
  , caption_()
{}

const std::int32_t messages_messageEditData::ID;

object_ptr<messages_messageEditData> messages_messageEditData::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<messages_messageEditData> res = make_tl_object<messages_messageEditData>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->caption_ = (var0 & 1) != 0;
  if (p.get_error()) { FAIL(""); }
  return res;
#undef FAIL
}

void messages_messageEditData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.messageEditData");
  int32 var0;
    s.store_field("flags", (var0 = flags_ | (caption_ << 0)));
    if (var0 & 1) { s.store_field("caption", true); }
    s.store_class_end();
  }
}

const std::int32_t messages_peerDialogs::ID;

object_ptr<messages_peerDialogs> messages_peerDialogs::fetch(TlBufferParser &p) {
  return make_tl_object<messages_peerDialogs>(p);
}

messages_peerDialogs::messages_peerDialogs(TlBufferParser &p)
  : dialogs_(TlFetchBoxed<TlFetchVector<TlFetchObject<Dialog>>, 481674261>::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
  , state_(TlFetchBoxed<TlFetchObject<updates_state>, -1519637954>::parse(p))
{}

void messages_peerDialogs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.peerDialogs");
    { s.store_vector_begin("dialogs", dialogs_.size()); for (const auto &_value : dialogs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    s.store_class_end();
  }
}

object_ptr<messages_QuickReplies> messages_QuickReplies::fetch(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case messages_quickReplies::ID:
      return messages_quickReplies::fetch(p);
    case messages_quickRepliesNotModified::ID:
      return messages_quickRepliesNotModified::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t messages_quickReplies::ID;

object_ptr<messages_QuickReplies> messages_quickReplies::fetch(TlBufferParser &p) {
  return make_tl_object<messages_quickReplies>(p);
}

messages_quickReplies::messages_quickReplies(TlBufferParser &p)
  : quick_replies_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<quickReply>, 110563371>>, 481674261>::parse(p))
  , messages_(TlFetchBoxed<TlFetchVector<TlFetchObject<Message>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void messages_quickReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.quickReplies");
    { s.store_vector_begin("quick_replies", quick_replies_.size()); for (const auto &_value : quick_replies_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t messages_quickRepliesNotModified::ID;

object_ptr<messages_QuickReplies> messages_quickRepliesNotModified::fetch(TlBufferParser &p) {
  return make_tl_object<messages_quickRepliesNotModified>();
}

void messages_quickRepliesNotModified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.quickRepliesNotModified");
    s.store_class_end();
  }
}

const std::int32_t payments_bankCardData::ID;

object_ptr<payments_bankCardData> payments_bankCardData::fetch(TlBufferParser &p) {
  return make_tl_object<payments_bankCardData>(p);
}

payments_bankCardData::payments_bankCardData(TlBufferParser &p)
  : title_(TlFetchString<string>::parse(p))
  , open_urls_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<bankCardOpenUrl>, -177732982>>, 481674261>::parse(p))
{}

void payments_bankCardData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.bankCardData");
    s.store_field("title", title_);
    { s.store_vector_begin("open_urls", open_urls_.size()); for (const auto &_value : open_urls_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t phone_groupCallStreamRtmpUrl::ID;

object_ptr<phone_groupCallStreamRtmpUrl> phone_groupCallStreamRtmpUrl::fetch(TlBufferParser &p) {
  return make_tl_object<phone_groupCallStreamRtmpUrl>(p);
}

phone_groupCallStreamRtmpUrl::phone_groupCallStreamRtmpUrl(TlBufferParser &p)
  : url_(TlFetchString<string>::parse(p))
  , key_(TlFetchString<string>::parse(p))
{}

void phone_groupCallStreamRtmpUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.groupCallStreamRtmpUrl");
    s.store_field("url", url_);
    s.store_field("key", key_);
    s.store_class_end();
  }
}

const std::int32_t premium_myBoosts::ID;

object_ptr<premium_myBoosts> premium_myBoosts::fetch(TlBufferParser &p) {
  return make_tl_object<premium_myBoosts>(p);
}

premium_myBoosts::premium_myBoosts(TlBufferParser &p)
  : my_boosts_(TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<myBoost>, -1001897636>>, 481674261>::parse(p))
  , chats_(TlFetchBoxed<TlFetchVector<TlFetchObject<Chat>>, 481674261>::parse(p))
  , users_(TlFetchBoxed<TlFetchVector<TlFetchObject<User>>, 481674261>::parse(p))
{}

void premium_myBoosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premium.myBoosts");
    { s.store_vector_begin("my_boosts", my_boosts_.size()); for (const auto &_value : my_boosts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t updates_state::ID;

object_ptr<updates_state> updates_state::fetch(TlBufferParser &p) {
  return make_tl_object<updates_state>(p);
}

updates_state::updates_state(TlBufferParser &p)
  : pts_(TlFetchInt::parse(p))
  , qts_(TlFetchInt::parse(p))
  , date_(TlFetchInt::parse(p))
  , seq_(TlFetchInt::parse(p))
  , unread_count_(TlFetchInt::parse(p))
{}

void updates_state::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.state");
    s.store_field("pts", pts_);
    s.store_field("qts", qts_);
    s.store_field("date", date_);
    s.store_field("seq", seq_);
    s.store_field("unread_count", unread_count_);
    s.store_class_end();
  }
}

account_confirmPhone::account_confirmPhone(string const &phone_code_hash_, string const &phone_code_)
  : phone_code_hash_(phone_code_hash_)
  , phone_code_(phone_code_)
{}

const std::int32_t account_confirmPhone::ID;

void account_confirmPhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1596029123);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(phone_code_, s);
}

void account_confirmPhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1596029123);
  TlStoreString::store(phone_code_hash_, s);
  TlStoreString::store(phone_code_, s);
}

void account_confirmPhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.confirmPhone");
    s.store_field("phone_code_hash", phone_code_hash_);
    s.store_field("phone_code", phone_code_);
    s.store_class_end();
  }
}

account_confirmPhone::ReturnType account_confirmPhone::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_createBusinessChatLink::account_createBusinessChatLink(object_ptr<inputBusinessChatLink> &&link_)
  : link_(std::move(link_))
{}

const std::int32_t account_createBusinessChatLink::ID;

void account_createBusinessChatLink::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2007898482);
  TlStoreBoxed<TlStoreObject, 292003751>::store(link_, s);
}

void account_createBusinessChatLink::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2007898482);
  TlStoreBoxed<TlStoreObject, 292003751>::store(link_, s);
}

void account_createBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.createBusinessChatLink");
    s.store_object_field("link", static_cast<const BaseObject *>(link_.get()));
    s.store_class_end();
  }
}

account_createBusinessChatLink::ReturnType account_createBusinessChatLink::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<businessChatLink>, -1263638929>::parse(p);
#undef FAIL
}

account_disablePeerConnectedBot::account_disablePeerConnectedBot(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t account_disablePeerConnectedBot::ID;

void account_disablePeerConnectedBot::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1581481689);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void account_disablePeerConnectedBot::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1581481689);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void account_disablePeerConnectedBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.disablePeerConnectedBot");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

account_disablePeerConnectedBot::ReturnType account_disablePeerConnectedBot::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_getDefaultBackgroundEmojis::account_getDefaultBackgroundEmojis(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_getDefaultBackgroundEmojis::ID;

void account_getDefaultBackgroundEmojis::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1509246514);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultBackgroundEmojis::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1509246514);
  TlStoreBinary::store(hash_, s);
}

void account_getDefaultBackgroundEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getDefaultBackgroundEmojis");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_getDefaultBackgroundEmojis::ReturnType account_getDefaultBackgroundEmojis::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EmojiList>::parse(p);
#undef FAIL
}

const std::int32_t account_getGlobalPrivacySettings::ID;

void account_getGlobalPrivacySettings::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-349483786);
}

void account_getGlobalPrivacySettings::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-349483786);
}

void account_getGlobalPrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getGlobalPrivacySettings");
    s.store_class_end();
  }
}

account_getGlobalPrivacySettings::ReturnType account_getGlobalPrivacySettings::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<globalPrivacySettings>, -29248689>::parse(p);
#undef FAIL
}

account_getSecureValue::account_getSecureValue(array<object_ptr<SecureValueType>> &&types_)
  : types_(std::move(types_))
{}

const std::int32_t account_getSecureValue::ID;

void account_getSecureValue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1936088002);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(types_, s);
}

void account_getSecureValue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1936088002);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(types_, s);
}

void account_getSecureValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.getSecureValue");
    { s.store_vector_begin("types", types_.size()); for (const auto &_value : types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

account_getSecureValue::ReturnType account_getSecureValue::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<secureValue>, 411017418>>, 481674261>::parse(p);
#undef FAIL
}

account_resetWebAuthorization::account_resetWebAuthorization(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t account_resetWebAuthorization::ID;

void account_resetWebAuthorization::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(755087855);
  TlStoreBinary::store(hash_, s);
}

void account_resetWebAuthorization::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(755087855);
  TlStoreBinary::store(hash_, s);
}

void account_resetWebAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resetWebAuthorization");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

account_resetWebAuthorization::ReturnType account_resetWebAuthorization::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_resolveBusinessChatLink::account_resolveBusinessChatLink(string const &slug_)
  : slug_(slug_)
{}

const std::int32_t account_resolveBusinessChatLink::ID;

void account_resolveBusinessChatLink::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1418913262);
  TlStoreString::store(slug_, s);
}

void account_resolveBusinessChatLink::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1418913262);
  TlStoreString::store(slug_, s);
}

void account_resolveBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.resolveBusinessChatLink");
    s.store_field("slug", slug_);
    s.store_class_end();
  }
}

account_resolveBusinessChatLink::ReturnType account_resolveBusinessChatLink::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<account_resolvedBusinessChatLinks>, -1708937439>::parse(p);
#undef FAIL
}

account_saveMusic::account_saveMusic(int32 flags_, bool unsave_, object_ptr<InputDocument> &&id_, object_ptr<InputDocument> &&after_id_)
  : flags_(flags_)
  , unsave_(unsave_)
  , id_(std::move(id_))
  , after_id_(std::move(after_id_))
{}

const std::int32_t account_saveMusic::ID;

void account_saveMusic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1301859671);
  TlStoreBinary::store((var0 = flags_ | (unsave_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(after_id_, s); }
}

void account_saveMusic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1301859671);
  TlStoreBinary::store((var0 = flags_ | (unsave_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(id_, s);
  if (var0 & 2) { TlStoreBoxedUnknown<TlStoreObject>::store(after_id_, s); }
}

void account_saveMusic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.saveMusic");
    s.store_field("flags", (var0 = flags_ | (unsave_ << 0)));
    if (var0 & 1) { s.store_field("unsave", true); }
    s.store_object_field("id", static_cast<const BaseObject *>(id_.get()));
    if (var0 & 2) { s.store_object_field("after_id", static_cast<const BaseObject *>(after_id_.get())); }
    s.store_class_end();
  }
}

account_saveMusic::ReturnType account_saveMusic::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_toggleConnectedBotPaused::account_toggleConnectedBotPaused(object_ptr<InputPeer> &&peer_, bool paused_)
  : peer_(std::move(peer_))
  , paused_(paused_)
{}

const std::int32_t account_toggleConnectedBotPaused::ID;

void account_toggleConnectedBotPaused::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1684934807);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(paused_, s);
}

void account_toggleConnectedBotPaused::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1684934807);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBool::store(paused_, s);
}

void account_toggleConnectedBotPaused::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.toggleConnectedBotPaused");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("paused", paused_);
    s.store_class_end();
  }
}

account_toggleConnectedBotPaused::ReturnType account_toggleConnectedBotPaused::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_toggleUsername::account_toggleUsername(string const &username_, bool active_)
  : username_(username_)
  , active_(active_)
{}

const std::int32_t account_toggleUsername::ID;

void account_toggleUsername::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1490465654);
  TlStoreString::store(username_, s);
  TlStoreBool::store(active_, s);
}

void account_toggleUsername::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1490465654);
  TlStoreString::store(username_, s);
  TlStoreBool::store(active_, s);
}

void account_toggleUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.toggleUsername");
    s.store_field("username", username_);
    s.store_field("active", active_);
    s.store_class_end();
  }
}

account_toggleUsername::ReturnType account_toggleUsername::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

account_uploadTheme::account_uploadTheme(int32 flags_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&thumb_, string const &file_name_, string const &mime_type_)
  : flags_(flags_)
  , file_(std::move(file_))
  , thumb_(std::move(thumb_))
  , file_name_(file_name_)
  , mime_type_(mime_type_)
{}

const std::int32_t account_uploadTheme::ID;

void account_uploadTheme::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(473805619);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  TlStoreString::store(file_name_, s);
  TlStoreString::store(mime_type_, s);
}

void account_uploadTheme::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(473805619);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(file_, s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s); }
  TlStoreString::store(file_name_, s);
  TlStoreString::store(mime_type_, s);
}

void account_uploadTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "account.uploadTheme");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    if (var0 & 1) { s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get())); }
    s.store_field("file_name", file_name_);
    s.store_field("mime_type", mime_type_);
    s.store_class_end();
  }
}

account_uploadTheme::ReturnType account_uploadTheme::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Document>::parse(p);
#undef FAIL
}

auth_importWebTokenAuthorization::auth_importWebTokenAuthorization(int32 api_id_, string const &api_hash_, string const &web_auth_token_)
  : api_id_(api_id_)
  , api_hash_(api_hash_)
  , web_auth_token_(web_auth_token_)
{}

const std::int32_t auth_importWebTokenAuthorization::ID;

void auth_importWebTokenAuthorization::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(767062953);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreString::store(web_auth_token_, s);
}

void auth_importWebTokenAuthorization::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(767062953);
  TlStoreBinary::store(api_id_, s);
  TlStoreString::store(api_hash_, s);
  TlStoreString::store(web_auth_token_, s);
}

void auth_importWebTokenAuthorization::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.importWebTokenAuthorization");
    s.store_field("api_id", api_id_);
    s.store_field("api_hash", api_hash_);
    s.store_field("web_auth_token", web_auth_token_);
    s.store_class_end();
  }
}

auth_importWebTokenAuthorization::ReturnType auth_importWebTokenAuthorization::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<auth_Authorization>::parse(p);
#undef FAIL
}

auth_requestFirebaseSms::auth_requestFirebaseSms(int32 flags_, string const &phone_number_, string const &phone_code_hash_, string const &safety_net_token_, string const &play_integrity_token_, string const &ios_push_secret_)
  : flags_(flags_)
  , phone_number_(phone_number_)
  , phone_code_hash_(phone_code_hash_)
  , safety_net_token_(safety_net_token_)
  , play_integrity_token_(play_integrity_token_)
  , ios_push_secret_(ios_push_secret_)
{}

const std::int32_t auth_requestFirebaseSms::ID;

void auth_requestFirebaseSms::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1908857314);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  if (var0 & 1) { TlStoreString::store(safety_net_token_, s); }
  if (var0 & 4) { TlStoreString::store(play_integrity_token_, s); }
  if (var0 & 2) { TlStoreString::store(ios_push_secret_, s); }
}

void auth_requestFirebaseSms::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1908857314);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(phone_code_hash_, s);
  if (var0 & 1) { TlStoreString::store(safety_net_token_, s); }
  if (var0 & 4) { TlStoreString::store(play_integrity_token_, s); }
  if (var0 & 2) { TlStoreString::store(ios_push_secret_, s); }
}

void auth_requestFirebaseSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "auth.requestFirebaseSms");
    s.store_field("flags", (var0 = flags_));
    s.store_field("phone_number", phone_number_);
    s.store_field("phone_code_hash", phone_code_hash_);
    if (var0 & 1) { s.store_field("safety_net_token", safety_net_token_); }
    if (var0 & 4) { s.store_field("play_integrity_token", play_integrity_token_); }
    if (var0 & 2) { s.store_field("ios_push_secret", ios_push_secret_); }
    s.store_class_end();
  }
}

auth_requestFirebaseSms::ReturnType auth_requestFirebaseSms::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_canSendMessage::bots_canSendMessage(object_ptr<InputUser> &&bot_)
  : bot_(std::move(bot_))
{}

const std::int32_t bots_canSendMessage::ID;

void bots_canSendMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(324662502);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_canSendMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(324662502);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_canSendMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.canSendMessage");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

bots_canSendMessage::ReturnType bots_canSendMessage::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

bots_getBotInfo::bots_getBotInfo(int32 flags_, object_ptr<InputUser> &&bot_, string const &lang_code_)
  : flags_(flags_)
  , bot_(std::move(bot_))
  , lang_code_(lang_code_)
{}

const std::int32_t bots_getBotInfo::ID;

void bots_getBotInfo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-589753091);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreString::store(lang_code_, s);
}

void bots_getBotInfo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-589753091);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s); }
  TlStoreString::store(lang_code_, s);
}

void bots_getBotInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getBotInfo");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get())); }
    s.store_field("lang_code", lang_code_);
    s.store_class_end();
  }
}

bots_getBotInfo::ReturnType bots_getBotInfo::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<bots_botInfo>, -391678544>::parse(p);
#undef FAIL
}

bots_getPreviewMedias::bots_getPreviewMedias(object_ptr<InputUser> &&bot_)
  : bot_(std::move(bot_))
{}

const std::int32_t bots_getPreviewMedias::ID;

void bots_getPreviewMedias::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1566222003);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_getPreviewMedias::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1566222003);
  TlStoreBoxedUnknown<TlStoreObject>::store(bot_, s);
}

void bots_getPreviewMedias::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bots.getPreviewMedias");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

bots_getPreviewMedias::ReturnType bots_getPreviewMedias::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<botPreviewMedia>, 602479523>>, 481674261>::parse(p);
#undef FAIL
}

channels_deleteHistory::channels_deleteHistory(int32 flags_, bool for_everyone_, object_ptr<InputChannel> &&channel_, int32 max_id_)
  : flags_(flags_)
  , for_everyone_(for_everyone_)
  , channel_(std::move(channel_))
  , max_id_(max_id_)
{}

const std::int32_t channels_deleteHistory::ID;

void channels_deleteHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1683319225);
  TlStoreBinary::store((var0 = flags_ | (for_everyone_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(max_id_, s);
}

void channels_deleteHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1683319225);
  TlStoreBinary::store((var0 = flags_ | (for_everyone_ << 0)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(max_id_, s);
}

void channels_deleteHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.deleteHistory");
    s.store_field("flags", (var0 = flags_ | (for_everyone_ << 0)));
    if (var0 & 1) { s.store_field("for_everyone", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

channels_deleteHistory::ReturnType channels_deleteHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_deleteMessages::channels_deleteMessages(object_ptr<InputChannel> &&channel_, array<int32> &&id_)
  : channel_(std::move(channel_))
  , id_(std::move(id_))
{}

const std::int32_t channels_deleteMessages::ID;

void channels_deleteMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2067661490);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void channels_deleteMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2067661490);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void channels_deleteMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.deleteMessages");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

channels_deleteMessages::ReturnType channels_deleteMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedMessages>, -2066640507>::parse(p);
#undef FAIL
}

channels_exportMessageLink::channels_exportMessageLink(int32 flags_, bool grouped_, bool thread_, object_ptr<InputChannel> &&channel_, int32 id_)
  : flags_(flags_)
  , grouped_(grouped_)
  , thread_(thread_)
  , channel_(std::move(channel_))
  , id_(id_)
{}

const std::int32_t channels_exportMessageLink::ID;

void channels_exportMessageLink::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-432034325);
  TlStoreBinary::store((var0 = flags_ | (grouped_ << 0) | (thread_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(id_, s);
}

void channels_exportMessageLink::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-432034325);
  TlStoreBinary::store((var0 = flags_ | (grouped_ << 0) | (thread_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBinary::store(id_, s);
}

void channels_exportMessageLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.exportMessageLink");
    s.store_field("flags", (var0 = flags_ | (grouped_ << 0) | (thread_ << 1)));
    if (var0 & 1) { s.store_field("grouped", true); }
    if (var0 & 2) { s.store_field("thread", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("id", id_);
    s.store_class_end();
  }
}

channels_exportMessageLink::ReturnType channels_exportMessageLink::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<exportedMessageLink>, 1571494644>::parse(p);
#undef FAIL
}

channels_getChannelRecommendations::channels_getChannelRecommendations(int32 flags_, object_ptr<InputChannel> &&channel_)
  : flags_(flags_)
  , channel_(std::move(channel_))
{}

const std::int32_t channels_getChannelRecommendations::ID;

void channels_getChannelRecommendations::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(631707458);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s); }
}

void channels_getChannelRecommendations::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(631707458);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s); }
}

void channels_getChannelRecommendations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getChannelRecommendations");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get())); }
    s.store_class_end();
  }
}

channels_getChannelRecommendations::ReturnType channels_getChannelRecommendations::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

channels_getParticipant::channels_getParticipant(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_)
  : channel_(std::move(channel_))
  , participant_(std::move(participant_))
{}

const std::int32_t channels_getParticipant::ID;

void channels_getParticipant::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1599378234);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
}

void channels_getParticipant::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1599378234);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(participant_, s);
}

void channels_getParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.getParticipant");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_class_end();
  }
}

channels_getParticipant::ReturnType channels_getParticipant::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<channels_channelParticipant>, -541588713>::parse(p);
#undef FAIL
}

channels_toggleAntiSpam::channels_toggleAntiSpam(object_ptr<InputChannel> &&channel_, bool enabled_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
{}

const std::int32_t channels_toggleAntiSpam::ID;

void channels_toggleAntiSpam::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1760814315);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleAntiSpam::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1760814315);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleAntiSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleAntiSpam");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

channels_toggleAntiSpam::ReturnType channels_toggleAntiSpam::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_toggleJoinRequest::channels_toggleJoinRequest(object_ptr<InputChannel> &&channel_, bool enabled_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
{}

const std::int32_t channels_toggleJoinRequest::ID;

void channels_toggleJoinRequest::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1277789622);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleJoinRequest::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1277789622);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleJoinRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleJoinRequest");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

channels_toggleJoinRequest::ReturnType channels_toggleJoinRequest::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_toggleJoinToSend::channels_toggleJoinToSend(object_ptr<InputChannel> &&channel_, bool enabled_)
  : channel_(std::move(channel_))
  , enabled_(enabled_)
{}

const std::int32_t channels_toggleJoinToSend::ID;

void channels_toggleJoinToSend::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-456419968);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleJoinToSend::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-456419968);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
  TlStoreBool::store(enabled_, s);
}

void channels_toggleJoinToSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleJoinToSend");
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

channels_toggleJoinToSend::ReturnType channels_toggleJoinToSend::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

channels_toggleSignatures::channels_toggleSignatures(int32 flags_, bool signatures_enabled_, bool profiles_enabled_, object_ptr<InputChannel> &&channel_)
  : flags_(flags_)
  , signatures_enabled_(signatures_enabled_)
  , profiles_enabled_(profiles_enabled_)
  , channel_(std::move(channel_))
{}

const std::int32_t channels_toggleSignatures::ID;

void channels_toggleSignatures::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1099781276);
  TlStoreBinary::store((var0 = flags_ | (signatures_enabled_ << 0) | (profiles_enabled_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_toggleSignatures::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1099781276);
  TlStoreBinary::store((var0 = flags_ | (signatures_enabled_ << 0) | (profiles_enabled_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(channel_, s);
}

void channels_toggleSignatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "channels.toggleSignatures");
    s.store_field("flags", (var0 = flags_ | (signatures_enabled_ << 0) | (profiles_enabled_ << 1)));
    if (var0 & 1) { s.store_field("signatures_enabled", true); }
    if (var0 & 2) { s.store_field("profiles_enabled", true); }
    s.store_object_field("channel", static_cast<const BaseObject *>(channel_.get()));
    s.store_class_end();
  }
}

channels_toggleSignatures::ReturnType channels_toggleSignatures::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_deleteContacts::contacts_deleteContacts(array<object_ptr<InputUser>> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t contacts_deleteContacts::ID;

void contacts_deleteContacts::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(157945344);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void contacts_deleteContacts::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(157945344);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void contacts_deleteContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.deleteContacts");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

contacts_deleteContacts::ReturnType contacts_deleteContacts::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

contacts_getBlocked::contacts_getBlocked(int32 flags_, bool my_stories_from_, int32 offset_, int32 limit_)
  : flags_(flags_)
  , my_stories_from_(my_stories_from_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t contacts_getBlocked::ID;

void contacts_getBlocked::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1702457472);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void contacts_getBlocked::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1702457472);
  TlStoreBinary::store((var0 = flags_ | (my_stories_from_ << 0)), s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void contacts_getBlocked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getBlocked");
    s.store_field("flags", (var0 = flags_ | (my_stories_from_ << 0)));
    if (var0 & 1) { s.store_field("my_stories_from", true); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

contacts_getBlocked::ReturnType contacts_getBlocked::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<contacts_Blocked>::parse(p);
#undef FAIL
}

contacts_getContactIDs::contacts_getContactIDs(int64 hash_)
  : hash_(hash_)
{}

const std::int32_t contacts_getContactIDs::ID;

void contacts_getContactIDs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2061264541);
  TlStoreBinary::store(hash_, s);
}

void contacts_getContactIDs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2061264541);
  TlStoreBinary::store(hash_, s);
}

void contacts_getContactIDs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getContactIDs");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

contacts_getContactIDs::ReturnType contacts_getContactIDs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
#undef FAIL
}

contacts_getLocated::contacts_getLocated(int32 flags_, bool background_, object_ptr<InputGeoPoint> &&geo_point_, int32 self_expires_)
  : flags_(flags_)
  , background_(background_)
  , geo_point_(std::move(geo_point_))
  , self_expires_(self_expires_)
{}

const std::int32_t contacts_getLocated::ID;

void contacts_getLocated::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-750207932);
  TlStoreBinary::store((var0 = flags_ | (background_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  if (var0 & 1) { TlStoreBinary::store(self_expires_, s); }
}

void contacts_getLocated::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-750207932);
  TlStoreBinary::store((var0 = flags_ | (background_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(geo_point_, s);
  if (var0 & 1) { TlStoreBinary::store(self_expires_, s); }
}

void contacts_getLocated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getLocated");
    s.store_field("flags", (var0 = flags_ | (background_ << 1)));
    if (var0 & 2) { s.store_field("background", true); }
    s.store_object_field("geo_point", static_cast<const BaseObject *>(geo_point_.get()));
    if (var0 & 1) { s.store_field("self_expires", self_expires_); }
    s.store_class_end();
  }
}

contacts_getLocated::ReturnType contacts_getLocated::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

const std::int32_t contacts_getStatuses::ID;

void contacts_getStatuses::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-995929106);
}

void contacts_getStatuses::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-995929106);
}

void contacts_getStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.getStatuses");
    s.store_class_end();
  }
}

contacts_getStatuses::ReturnType contacts_getStatuses::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<contactStatus>, 383348795>>, 481674261>::parse(p);
#undef FAIL
}

contacts_importContactToken::contacts_importContactToken(string const &token_)
  : token_(token_)
{}

const std::int32_t contacts_importContactToken::ID;

void contacts_importContactToken::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(318789512);
  TlStoreString::store(token_, s);
}

void contacts_importContactToken::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(318789512);
  TlStoreString::store(token_, s);
}

void contacts_importContactToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.importContactToken");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

contacts_importContactToken::ReturnType contacts_importContactToken::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<User>::parse(p);
#undef FAIL
}

contacts_resolvePhone::contacts_resolvePhone(string const &phone_)
  : phone_(phone_)
{}

const std::int32_t contacts_resolvePhone::ID;

void contacts_resolvePhone::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1963375804);
  TlStoreString::store(phone_, s);
}

void contacts_resolvePhone::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1963375804);
  TlStoreString::store(phone_, s);
}

void contacts_resolvePhone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contacts.resolvePhone");
    s.store_field("phone", phone_);
    s.store_class_end();
  }
}

contacts_resolvePhone::ReturnType contacts_resolvePhone::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<contacts_resolvedPeer>, 2131196633>::parse(p);
#undef FAIL
}

help_getPassportConfig::help_getPassportConfig(int32 hash_)
  : hash_(hash_)
{}

const std::int32_t help_getPassportConfig::ID;

void help_getPassportConfig::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-966677240);
  TlStoreBinary::store(hash_, s);
}

void help_getPassportConfig::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-966677240);
  TlStoreBinary::store(hash_, s);
}

void help_getPassportConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "help.getPassportConfig");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

help_getPassportConfig::ReturnType help_getPassportConfig::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<help_PassportConfig>::parse(p);
#undef FAIL
}

invokeWithBusinessConnectionPrefix::invokeWithBusinessConnectionPrefix(string const &connection_id_)
  : connection_id_(connection_id_)
{}

const std::int32_t invokeWithBusinessConnectionPrefix::ID;

void invokeWithBusinessConnectionPrefix::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-584540274);
  TlStoreString::store(connection_id_, s);
}

void invokeWithBusinessConnectionPrefix::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-584540274);
  TlStoreString::store(connection_id_, s);
}

void invokeWithBusinessConnectionPrefix::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "invokeWithBusinessConnectionPrefix");
    s.store_field("connection_id", connection_id_);
    s.store_class_end();
  }
}

invokeWithBusinessConnectionPrefix::ReturnType invokeWithBusinessConnectionPrefix::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<error>, -994444869>::parse(p);
#undef FAIL
}

invokeWithGooglePlayIntegrityPrefix::invokeWithGooglePlayIntegrityPrefix(string const &nonce_, string const &token_)
  : nonce_(nonce_)
  , token_(token_)
{}

const std::int32_t invokeWithGooglePlayIntegrityPrefix::ID;

void invokeWithGooglePlayIntegrityPrefix::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(502868356);
  TlStoreString::store(nonce_, s);
  TlStoreString::store(token_, s);
}

void invokeWithGooglePlayIntegrityPrefix::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(502868356);
  TlStoreString::store(nonce_, s);
  TlStoreString::store(token_, s);
}

void invokeWithGooglePlayIntegrityPrefix::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "invokeWithGooglePlayIntegrityPrefix");
    s.store_field("nonce", nonce_);
    s.store_field("token", token_);
    s.store_class_end();
  }
}

invokeWithGooglePlayIntegrityPrefix::ReturnType invokeWithGooglePlayIntegrityPrefix::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<error>, -994444869>::parse(p);
#undef FAIL
}

messages_checkChatInvite::messages_checkChatInvite(string const &hash_)
  : hash_(hash_)
{}

const std::int32_t messages_checkChatInvite::ID;

void messages_checkChatInvite::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1051570619);
  TlStoreString::store(hash_, s);
}

void messages_checkChatInvite::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1051570619);
  TlStoreString::store(hash_, s);
}

void messages_checkChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.checkChatInvite");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_checkChatInvite::ReturnType messages_checkChatInvite::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<ChatInvite>::parse(p);
#undef FAIL
}

messages_checkHistoryImportPeer::messages_checkHistoryImportPeer(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t messages_checkHistoryImportPeer::ID;

void messages_checkHistoryImportPeer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1573261059);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_checkHistoryImportPeer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1573261059);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_checkHistoryImportPeer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.checkHistoryImportPeer");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_checkHistoryImportPeer::ReturnType messages_checkHistoryImportPeer::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_checkedHistoryImportPeer>, -1571952873>::parse(p);
#undef FAIL
}

messages_deleteChat::messages_deleteChat(int64 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t messages_deleteChat::ID;

void messages_deleteChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1540419152);
  TlStoreBinary::store(chat_id_, s);
}

void messages_deleteChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1540419152);
  TlStoreBinary::store(chat_id_, s);
}

void messages_deleteChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

messages_deleteChat::ReturnType messages_deleteChat::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_deleteHistory::messages_deleteHistory(int32 flags_, bool just_clear_, bool revoke_, object_ptr<InputPeer> &&peer_, int32 max_id_, int32 min_date_, int32 max_date_)
  : flags_(flags_)
  , just_clear_(just_clear_)
  , revoke_(revoke_)
  , peer_(std::move(peer_))
  , max_id_(max_id_)
  , min_date_(min_date_)
  , max_date_(max_date_)
{}

const std::int32_t messages_deleteHistory::ID;

void messages_deleteHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1332768214);
  TlStoreBinary::store((var0 = flags_ | (just_clear_ << 0) | (revoke_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
  if (var0 & 4) { TlStoreBinary::store(min_date_, s); }
  if (var0 & 8) { TlStoreBinary::store(max_date_, s); }
}

void messages_deleteHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1332768214);
  TlStoreBinary::store((var0 = flags_ | (just_clear_ << 0) | (revoke_ << 1)), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
  if (var0 & 4) { TlStoreBinary::store(min_date_, s); }
  if (var0 & 8) { TlStoreBinary::store(max_date_, s); }
}

void messages_deleteHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteHistory");
    s.store_field("flags", (var0 = flags_ | (just_clear_ << 0) | (revoke_ << 1)));
    if (var0 & 1) { s.store_field("just_clear", true); }
    if (var0 & 2) { s.store_field("revoke", true); }
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_id", max_id_);
    if (var0 & 4) { s.store_field("min_date", min_date_); }
    if (var0 & 8) { s.store_field("max_date", max_date_); }
    s.store_class_end();
  }
}

messages_deleteHistory::ReturnType messages_deleteHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedHistory>, -1269012015>::parse(p);
#undef FAIL
}

messages_deleteMessages::messages_deleteMessages(int32 flags_, bool revoke_, array<int32> &&id_)
  : flags_(flags_)
  , revoke_(revoke_)
  , id_(std::move(id_))
{}

const std::int32_t messages_deleteMessages::ID;

void messages_deleteMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-443640366);
  TlStoreBinary::store((var0 = flags_ | (revoke_ << 0)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_deleteMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-443640366);
  TlStoreBinary::store((var0 = flags_ | (revoke_ << 0)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_deleteMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deleteMessages");
    s.store_field("flags", (var0 = flags_ | (revoke_ << 0)));
    if (var0 & 1) { s.store_field("revoke", true); }
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_deleteMessages::ReturnType messages_deleteMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedMessages>, -2066640507>::parse(p);
#undef FAIL
}

messages_deletePhoneCallHistory::messages_deletePhoneCallHistory(int32 flags_, bool revoke_)
  : flags_(flags_)
  , revoke_(revoke_)
{}

const std::int32_t messages_deletePhoneCallHistory::ID;

void messages_deletePhoneCallHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-104078327);
  TlStoreBinary::store((var0 = flags_ | (revoke_ << 0)), s);
}

void messages_deletePhoneCallHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-104078327);
  TlStoreBinary::store((var0 = flags_ | (revoke_ << 0)), s);
}

void messages_deletePhoneCallHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.deletePhoneCallHistory");
    s.store_field("flags", (var0 = flags_ | (revoke_ << 0)));
    if (var0 & 1) { s.store_field("revoke", true); }
    s.store_class_end();
  }
}

messages_deletePhoneCallHistory::ReturnType messages_deletePhoneCallHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedFoundMessages>, -275956116>::parse(p);
#undef FAIL
}

messages_getForumTopics::messages_getForumTopics(int32 flags_, object_ptr<InputPeer> &&peer_, string const &q_, int32 offset_date_, int32 offset_id_, int32 offset_topic_, int32 limit_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , q_(q_)
  , offset_date_(offset_date_)
  , offset_id_(offset_id_)
  , offset_topic_(offset_topic_)
  , limit_(limit_)
{}

const std::int32_t messages_getForumTopics::ID;

void messages_getForumTopics::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1000635391);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreString::store(q_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_topic_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getForumTopics::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1000635391);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreString::store(q_, s); }
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_topic_, s);
  TlStoreBinary::store(limit_, s);
}

void messages_getForumTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getForumTopics");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("q", q_); }
    s.store_field("offset_date", offset_date_);
    s.store_field("offset_id", offset_id_);
    s.store_field("offset_topic", offset_topic_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

messages_getForumTopics::ReturnType messages_getForumTopics::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_forumTopics>, 913709011>::parse(p);
#undef FAIL
}

messages_getPollResults::messages_getPollResults(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t messages_getPollResults::ID;

void messages_getPollResults::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1941660731);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getPollResults::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1941660731);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_getPollResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getPollResults");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

messages_getPollResults::ReturnType messages_getPollResults::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_getSavedHistory::messages_getSavedHistory(int32 flags_, object_ptr<InputPeer> &&parent_peer_, object_ptr<InputPeer> &&peer_, int32 offset_id_, int32 offset_date_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_, int64 hash_)
  : flags_(flags_)
  , parent_peer_(std::move(parent_peer_))
  , peer_(std::move(peer_))
  , offset_id_(offset_id_)
  , offset_date_(offset_date_)
  , add_offset_(add_offset_)
  , limit_(limit_)
  , max_id_(max_id_)
  , min_id_(min_id_)
  , hash_(hash_)
{}

const std::int32_t messages_getSavedHistory::ID;

void messages_getSavedHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1718964215);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1718964215);
  TlStoreBinary::store((var0 = flags_), s);
  if (var0 & 1) { TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(offset_id_, s);
  TlStoreBinary::store(offset_date_, s);
  TlStoreBinary::store(add_offset_, s);
  TlStoreBinary::store(limit_, s);
  TlStoreBinary::store(max_id_, s);
  TlStoreBinary::store(min_id_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getSavedHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getSavedHistory");
    s.store_field("flags", (var0 = flags_));
    if (var0 & 1) { s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get())); }
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

messages_getSavedHistory::ReturnType messages_getSavedHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Messages>::parse(p);
#undef FAIL
}

messages_getStickerSet::messages_getStickerSet(object_ptr<InputStickerSet> &&stickerset_, int32 hash_)
  : stickerset_(std::move(stickerset_))
  , hash_(hash_)
{}

const std::int32_t messages_getStickerSet::ID;

void messages_getStickerSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-928977804);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getStickerSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-928977804);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_getStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.getStickerSet");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_getStickerSet::ReturnType messages_getStickerSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

messages_migrateChat::messages_migrateChat(int64 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t messages_migrateChat::ID;

void messages_migrateChat::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1568189671);
  TlStoreBinary::store(chat_id_, s);
}

void messages_migrateChat::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1568189671);
  TlStoreBinary::store(chat_id_, s);
}

void messages_migrateChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.migrateChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

messages_migrateChat::ReturnType messages_migrateChat::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_readMessageContents::messages_readMessageContents(array<int32> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t messages_readMessageContents::ID;

void messages_readMessageContents::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(916930423);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_readMessageContents::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(916930423);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_readMessageContents::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readMessageContents");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_readMessageContents::ReturnType messages_readMessageContents::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_affectedMessages>, -2066640507>::parse(p);
#undef FAIL
}

messages_readSavedHistory::messages_readSavedHistory(object_ptr<InputPeer> &&parent_peer_, object_ptr<InputPeer> &&peer_, int32 max_id_)
  : parent_peer_(std::move(parent_peer_))
  , peer_(std::move(peer_))
  , max_id_(max_id_)
{}

const std::int32_t messages_readSavedHistory::ID;

void messages_readSavedHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1169540261);
  TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
}

void messages_readSavedHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1169540261);
  TlStoreBoxedUnknown<TlStoreObject>::store(parent_peer_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(max_id_, s);
}

void messages_readSavedHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.readSavedHistory");
    s.store_object_field("parent_peer", static_cast<const BaseObject *>(parent_peer_.get()));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("max_id", max_id_);
    s.store_class_end();
  }
}

messages_readSavedHistory::ReturnType messages_readSavedHistory::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_reorderStickerSets::messages_reorderStickerSets(int32 flags_, bool masks_, bool emojis_, array<int64> &&order_)
  : flags_(flags_)
  , masks_(masks_)
  , emojis_(emojis_)
  , order_(std::move(order_))
{}

const std::int32_t messages_reorderStickerSets::ID;

void messages_reorderStickerSets::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2016638777);
  TlStoreBinary::store((var0 = flags_ | (masks_ << 0) | (emojis_ << 1)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_reorderStickerSets::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2016638777);
  TlStoreBinary::store((var0 = flags_ | (masks_ << 0) | (emojis_ << 1)), s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void messages_reorderStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reorderStickerSets");
    s.store_field("flags", (var0 = flags_ | (masks_ << 0) | (emojis_ << 1)));
    if (var0 & 1) { s.store_field("masks", true); }
    if (var0 & 2) { s.store_field("emojis", true); }
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_reorderStickerSets::ReturnType messages_reorderStickerSets::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_reportSpam::messages_reportSpam(object_ptr<InputPeer> &&peer_)
  : peer_(std::move(peer_))
{}

const std::int32_t messages_reportSpam::ID;

void messages_reportSpam::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-820669733);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_reportSpam::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-820669733);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
}

void messages_reportSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.reportSpam");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_class_end();
  }
}

messages_reportSpam::ReturnType messages_reportSpam::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_searchCustomEmoji::messages_searchCustomEmoji(string const &emoticon_, int64 hash_)
  : emoticon_(emoticon_)
  , hash_(hash_)
{}

const std::int32_t messages_searchCustomEmoji::ID;

void messages_searchCustomEmoji::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(739360983);
  TlStoreString::store(emoticon_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchCustomEmoji::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(739360983);
  TlStoreString::store(emoticon_, s);
  TlStoreBinary::store(hash_, s);
}

void messages_searchCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.searchCustomEmoji");
    s.store_field("emoticon", emoticon_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

messages_searchCustomEmoji::ReturnType messages_searchCustomEmoji::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<EmojiList>::parse(p);
#undef FAIL
}

messages_sendScheduledMessages::messages_sendScheduledMessages(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t messages_sendScheduledMessages::ID;

void messages_sendScheduledMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1120369398);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_sendScheduledMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1120369398);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void messages_sendScheduledMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.sendScheduledMessages");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages_sendScheduledMessages::ReturnType messages_sendScheduledMessages::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

messages_setDefaultHistoryTTL::messages_setDefaultHistoryTTL(int32 period_)
  : period_(period_)
{}

const std::int32_t messages_setDefaultHistoryTTL::ID;

void messages_setDefaultHistoryTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1632299963);
  TlStoreBinary::store(period_, s);
}

void messages_setDefaultHistoryTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1632299963);
  TlStoreBinary::store(period_, s);
}

void messages_setDefaultHistoryTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setDefaultHistoryTTL");
    s.store_field("period", period_);
    s.store_class_end();
  }
}

messages_setDefaultHistoryTTL::ReturnType messages_setDefaultHistoryTTL::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_setEncryptedTyping::messages_setEncryptedTyping(object_ptr<inputEncryptedChat> &&peer_, bool typing_)
  : peer_(std::move(peer_))
  , typing_(typing_)
{}

const std::int32_t messages_setEncryptedTyping::ID;

void messages_setEncryptedTyping::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2031374829);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBool::store(typing_, s);
}

void messages_setEncryptedTyping::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2031374829);
  TlStoreBoxed<TlStoreObject, -247351839>::store(peer_, s);
  TlStoreBool::store(typing_, s);
}

void messages_setEncryptedTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setEncryptedTyping");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("typing", typing_);
    s.store_class_end();
  }
}

messages_setEncryptedTyping::ReturnType messages_setEncryptedTyping::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_setTyping::messages_setTyping(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, object_ptr<SendMessageAction> &&action_)
  : flags_(flags_)
  , peer_(std::move(peer_))
  , top_msg_id_(top_msg_id_)
  , action_(std::move(action_))
{}

const std::int32_t messages_setTyping::ID;

void messages_setTyping::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1486110434);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void messages_setTyping::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1486110434);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  if (var0 & 1) { TlStoreBinary::store(top_msg_id_, s); }
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void messages_setTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.setTyping");
    s.store_field("flags", (var0 = flags_));
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    if (var0 & 1) { s.store_field("top_msg_id", top_msg_id_); }
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

messages_setTyping::ReturnType messages_setTyping::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_toggleDialogFilterTags::messages_toggleDialogFilterTags(bool enabled_)
  : enabled_(enabled_)
{}

const std::int32_t messages_toggleDialogFilterTags::ID;

void messages_toggleDialogFilterTags::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-47326647);
  TlStoreBool::store(enabled_, s);
}

void messages_toggleDialogFilterTags::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-47326647);
  TlStoreBool::store(enabled_, s);
}

void messages_toggleDialogFilterTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.toggleDialogFilterTags");
    s.store_field("enabled", enabled_);
    s.store_class_end();
  }
}

messages_toggleDialogFilterTags::ReturnType messages_toggleDialogFilterTags::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

messages_transcribeAudio::messages_transcribeAudio(object_ptr<InputPeer> &&peer_, int32 msg_id_)
  : peer_(std::move(peer_))
  , msg_id_(msg_id_)
{}

const std::int32_t messages_transcribeAudio::ID;

void messages_transcribeAudio::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(647928393);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_transcribeAudio::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(647928393);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(msg_id_, s);
}

void messages_transcribeAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.transcribeAudio");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("msg_id", msg_id_);
    s.store_class_end();
  }
}

messages_transcribeAudio::ReturnType messages_transcribeAudio::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<messages_transcribedAudio>, -809903785>::parse(p);
#undef FAIL
}

messages_updatePinnedForumTopic::messages_updatePinnedForumTopic(object_ptr<InputPeer> &&peer_, int32 topic_id_, bool pinned_)
  : peer_(std::move(peer_))
  , topic_id_(topic_id_)
  , pinned_(pinned_)
{}

const std::int32_t messages_updatePinnedForumTopic::ID;

void messages_updatePinnedForumTopic::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(392032849);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(topic_id_, s);
  TlStoreBool::store(pinned_, s);
}

void messages_updatePinnedForumTopic::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(392032849);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(topic_id_, s);
  TlStoreBool::store(pinned_, s);
}

void messages_updatePinnedForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages.updatePinnedForumTopic");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("topic_id", topic_id_);
    s.store_field("pinned", pinned_);
    s.store_class_end();
  }
}

messages_updatePinnedForumTopic::ReturnType messages_updatePinnedForumTopic::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

payments_canPurchaseStore::payments_canPurchaseStore(object_ptr<InputStorePaymentPurpose> &&purpose_)
  : purpose_(std::move(purpose_))
{}

const std::int32_t payments_canPurchaseStore::ID;

void payments_canPurchaseStore::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1339842215);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_canPurchaseStore::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1339842215);
  TlStoreBoxedUnknown<TlStoreObject>::store(purpose_, s);
}

void payments_canPurchaseStore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.canPurchaseStore");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

payments_canPurchaseStore::ReturnType payments_canPurchaseStore::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

payments_getStarGiftWithdrawalUrl::payments_getStarGiftWithdrawalUrl(object_ptr<InputSavedStarGift> &&stargift_, object_ptr<InputCheckPasswordSRP> &&password_)
  : stargift_(std::move(stargift_))
  , password_(std::move(password_))
{}

const std::int32_t payments_getStarGiftWithdrawalUrl::ID;

void payments_getStarGiftWithdrawalUrl::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-798059608);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void payments_getStarGiftWithdrawalUrl::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-798059608);
  TlStoreBoxedUnknown<TlStoreObject>::store(stargift_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(password_, s);
}

void payments_getStarGiftWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarGiftWithdrawalUrl");
    s.store_object_field("stargift", static_cast<const BaseObject *>(stargift_.get()));
    s.store_object_field("password", static_cast<const BaseObject *>(password_.get()));
    s.store_class_end();
  }
}

payments_getStarGiftWithdrawalUrl::ReturnType payments_getStarGiftWithdrawalUrl::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<payments_starGiftWithdrawalUrl>, -2069218660>::parse(p);
#undef FAIL
}

const std::int32_t payments_getStarsGiveawayOptions::ID;

void payments_getStarsGiveawayOptions::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1122042562);
}

void payments_getStarsGiveawayOptions::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1122042562);
}

void payments_getStarsGiveawayOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.getStarsGiveawayOptions");
    s.store_class_end();
  }
}

payments_getStarsGiveawayOptions::ReturnType payments_getStarsGiveawayOptions::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<starsGiveawayOption>, -1798404822>>, 481674261>::parse(p);
#undef FAIL
}

payments_reorderStarGiftCollections::payments_reorderStarGiftCollections(object_ptr<InputPeer> &&peer_, array<int32> &&order_)
  : peer_(std::move(peer_))
  , order_(std::move(order_))
{}

const std::int32_t payments_reorderStarGiftCollections::ID;

void payments_reorderStarGiftCollections::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1020594996);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void payments_reorderStarGiftCollections::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1020594996);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(order_, s);
}

void payments_reorderStarGiftCollections::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "payments.reorderStarGiftCollections");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("order", order_.size()); for (const auto &_value : order_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

payments_reorderStarGiftCollections::ReturnType payments_reorderStarGiftCollections::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

phone_getGroupCallStreamChannels::phone_getGroupCallStreamChannels(object_ptr<InputGroupCall> &&call_)
  : call_(std::move(call_))
{}

const std::int32_t phone_getGroupCallStreamChannels::ID;

void phone_getGroupCallStreamChannels::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(447879488);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_getGroupCallStreamChannels::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(447879488);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
}

void phone_getGroupCallStreamChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.getGroupCallStreamChannels");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

phone_getGroupCallStreamChannels::ReturnType phone_getGroupCallStreamChannels::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<phone_groupCallStreamChannels>, -790330702>::parse(p);
#undef FAIL
}

phone_inviteToGroupCall::phone_inviteToGroupCall(object_ptr<InputGroupCall> &&call_, array<object_ptr<InputUser>> &&users_)
  : call_(std::move(call_))
  , users_(std::move(users_))
{}

const std::int32_t phone_inviteToGroupCall::ID;

void phone_inviteToGroupCall::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(2067345760);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void phone_inviteToGroupCall::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(2067345760);
  TlStoreBoxedUnknown<TlStoreObject>::store(call_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(users_, s);
}

void phone_inviteToGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phone.inviteToGroupCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

phone_inviteToGroupCall::ReturnType phone_inviteToGroupCall::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<Updates>::parse(p);
#undef FAIL
}

const std::int32_t smsjobs_isEligibleToJoin::ID;

void smsjobs_isEligibleToJoin::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(249313744);
}

void smsjobs_isEligibleToJoin::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(249313744);
}

void smsjobs_isEligibleToJoin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.isEligibleToJoin");
    s.store_class_end();
  }
}

smsjobs_isEligibleToJoin::ReturnType smsjobs_isEligibleToJoin::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<smsjobs_eligibleToJoin>, -594852657>::parse(p);
#undef FAIL
}

const std::int32_t smsjobs_join::ID;

void smsjobs_join::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1488007635);
}

void smsjobs_join::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1488007635);
}

void smsjobs_join::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "smsjobs.join");
    s.store_class_end();
  }
}

smsjobs_join::ReturnType smsjobs_join::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stats_getStoryPublicForwards::stats_getStoryPublicForwards(object_ptr<InputPeer> &&peer_, int32 id_, string const &offset_, int32 limit_)
  : peer_(std::move(peer_))
  , id_(id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t stats_getStoryPublicForwards::ID;

void stats_getStoryPublicForwards::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1505526026);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stats_getStoryPublicForwards::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1505526026);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(id_, s);
  TlStoreString::store(offset_, s);
  TlStoreBinary::store(limit_, s);
}

void stats_getStoryPublicForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stats.getStoryPublicForwards");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("id", id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

stats_getStoryPublicForwards::ReturnType stats_getStoryPublicForwards::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchObject<stats_publicForwards>, -1828487648>::parse(p);
#undef FAIL
}

stickers_addStickerToSet::stickers_addStickerToSet(object_ptr<InputStickerSet> &&stickerset_, object_ptr<inputStickerSetItem> &&sticker_)
  : stickerset_(std::move(stickerset_))
  , sticker_(std::move(sticker_))
{}

const std::int32_t stickers_addStickerToSet::ID;

void stickers_addStickerToSet::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-2041315650);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBoxed<TlStoreObject, 853188252>::store(sticker_, s);
}

void stickers_addStickerToSet::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-2041315650);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
  TlStoreBoxed<TlStoreObject, 853188252>::store(sticker_, s);
}

void stickers_addStickerToSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.addStickerToSet");
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

stickers_addStickerToSet::ReturnType stickers_addStickerToSet::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stickers_checkShortName::stickers_checkShortName(string const &short_name_)
  : short_name_(short_name_)
{}

const std::int32_t stickers_checkShortName::ID;

void stickers_checkShortName::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(676017721);
  TlStoreString::store(short_name_, s);
}

void stickers_checkShortName::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(676017721);
  TlStoreString::store(short_name_, s);
}

void stickers_checkShortName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.checkShortName");
    s.store_field("short_name", short_name_);
    s.store_class_end();
  }
}

stickers_checkShortName::ReturnType stickers_checkShortName::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

stickers_replaceSticker::stickers_replaceSticker(object_ptr<InputDocument> &&sticker_, object_ptr<inputStickerSetItem> &&new_sticker_)
  : sticker_(std::move(sticker_))
  , new_sticker_(std::move(new_sticker_))
{}

const std::int32_t stickers_replaceSticker::ID;

void stickers_replaceSticker::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1184253338);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
  TlStoreBoxed<TlStoreObject, 853188252>::store(new_sticker_, s);
}

void stickers_replaceSticker::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1184253338);
  TlStoreBoxedUnknown<TlStoreObject>::store(sticker_, s);
  TlStoreBoxed<TlStoreObject, 853188252>::store(new_sticker_, s);
}

void stickers_replaceSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers.replaceSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_object_field("new_sticker", static_cast<const BaseObject *>(new_sticker_.get()));
    s.store_class_end();
  }
}

stickers_replaceSticker::ReturnType stickers_replaceSticker::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_StickerSet>::parse(p);
#undef FAIL
}

stories_deleteStories::stories_deleteStories(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t stories_deleteStories::ID;

void stories_deleteStories::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1369842849);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_deleteStories::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1369842849);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_deleteStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.deleteStories");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_deleteStories::ReturnType stories_deleteStories::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
#undef FAIL
}

stories_getAlbums::stories_getAlbums(object_ptr<InputPeer> &&peer_, int64 hash_)
  : peer_(std::move(peer_))
  , hash_(hash_)
{}

const std::int32_t stories_getAlbums::ID;

void stories_getAlbums::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(632548039);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(hash_, s);
}

void stories_getAlbums::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(632548039);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBinary::store(hash_, s);
}

void stories_getAlbums::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getAlbums");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

stories_getAlbums::ReturnType stories_getAlbums::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<stories_Albums>::parse(p);
#undef FAIL
}

const std::int32_t stories_getChatsToSend::ID;

void stories_getChatsToSend::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1519744160);
}

void stories_getChatsToSend::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1519744160);
}

void stories_getChatsToSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getChatsToSend");
    s.store_class_end();
  }
}

stories_getChatsToSend::ReturnType stories_getChatsToSend::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<messages_Chats>::parse(p);
#undef FAIL
}

stories_getPeerMaxIDs::stories_getPeerMaxIDs(array<object_ptr<InputPeer>> &&id_)
  : id_(std::move(id_))
{}

const std::int32_t stories_getPeerMaxIDs::ID;

void stories_getPeerMaxIDs::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(1398375363);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void stories_getPeerMaxIDs::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(1398375363);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(id_, s);
}

void stories_getPeerMaxIDs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.getPeerMaxIDs");
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_getPeerMaxIDs::ReturnType stories_getPeerMaxIDs::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchInt>, 481674261>::parse(p);
#undef FAIL
}

stories_togglePinnedToTop::stories_togglePinnedToTop(object_ptr<InputPeer> &&peer_, array<int32> &&id_)
  : peer_(std::move(peer_))
  , id_(std::move(id_))
{}

const std::int32_t stories_togglePinnedToTop::ID;

void stories_togglePinnedToTop::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(187268763);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_togglePinnedToTop::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(187268763);
  TlStoreBoxedUnknown<TlStoreObject>::store(peer_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(id_, s);
}

void stories_togglePinnedToTop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories.togglePinnedToTop");
    s.store_object_field("peer", static_cast<const BaseObject *>(peer_.get()));
    { s.store_vector_begin("id", id_.size()); for (const auto &_value : id_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

stories_togglePinnedToTop::ReturnType stories_togglePinnedToTop::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}

updates_getDifference::updates_getDifference(int32 flags_, int32 pts_, int32 pts_limit_, int32 pts_total_limit_, int32 date_, int32 qts_, int32 qts_limit_)
  : flags_(flags_)
  , pts_(pts_)
  , pts_limit_(pts_limit_)
  , pts_total_limit_(pts_total_limit_)
  , date_(date_)
  , qts_(qts_)
  , qts_limit_(qts_limit_)
{}

const std::int32_t updates_getDifference::ID;

void updates_getDifference::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(432207715);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(pts_, s);
  if (var0 & 2) { TlStoreBinary::store(pts_limit_, s); }
  if (var0 & 1) { TlStoreBinary::store(pts_total_limit_, s); }
  TlStoreBinary::store(date_, s);
  TlStoreBinary::store(qts_, s);
  if (var0 & 4) { TlStoreBinary::store(qts_limit_, s); }
}

void updates_getDifference::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(432207715);
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(pts_, s);
  if (var0 & 2) { TlStoreBinary::store(pts_limit_, s); }
  if (var0 & 1) { TlStoreBinary::store(pts_total_limit_, s); }
  TlStoreBinary::store(date_, s);
  TlStoreBinary::store(qts_, s);
  if (var0 & 4) { TlStoreBinary::store(qts_limit_, s); }
}

void updates_getDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates.getDifference");
    s.store_field("flags", (var0 = flags_));
    s.store_field("pts", pts_);
    if (var0 & 2) { s.store_field("pts_limit", pts_limit_); }
    if (var0 & 1) { s.store_field("pts_total_limit", pts_total_limit_); }
    s.store_field("date", date_);
    s.store_field("qts", qts_);
    if (var0 & 4) { s.store_field("qts_limit", qts_limit_); }
    s.store_class_end();
  }
}

updates_getDifference::ReturnType updates_getDifference::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchObject<updates_Difference>::parse(p);
#undef FAIL
}

upload_getFileHashes::upload_getFileHashes(object_ptr<InputFileLocation> &&location_, int64 offset_)
  : location_(std::move(location_))
  , offset_(offset_)
{}

const std::int32_t upload_getFileHashes::ID;

void upload_getFileHashes::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-1856595926);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(offset_, s);
}

void upload_getFileHashes::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-1856595926);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(offset_, s);
}

void upload_getFileHashes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upload.getFileHashes");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

upload_getFileHashes::ReturnType upload_getFileHashes::fetch_result(TlBufferParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBoxed<TlFetchVector<TlFetchBoxed<TlFetchObject<fileHash>, -207944868>>, 481674261>::parse(p);
#undef FAIL
}
}  // namespace telegram_api
}  // namespace td
