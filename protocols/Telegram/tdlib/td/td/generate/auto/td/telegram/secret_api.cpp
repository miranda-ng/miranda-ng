#include "secret_api.h"

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
namespace secret_api {

std::string to_string(const BaseObject &value) {
  TlStorerToString storer;
  value.store(storer, "");
  return storer.move_as_string();
}

object_ptr<Object> Object::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case decryptedMessage8::ID:
      return decryptedMessage8::fetch(p);
    case decryptedMessageService8::ID:
      return decryptedMessageService8::fetch(p);
    case decryptedMessage23::ID:
      return decryptedMessage23::fetch(p);
    case decryptedMessageService::ID:
      return decryptedMessageService::fetch(p);
    case decryptedMessage46::ID:
      return decryptedMessage46::fetch(p);
    case decryptedMessage::ID:
      return decryptedMessage::fetch(p);
    case decryptedMessageActionSetMessageTTL::ID:
      return decryptedMessageActionSetMessageTTL::fetch(p);
    case decryptedMessageActionReadMessages::ID:
      return decryptedMessageActionReadMessages::fetch(p);
    case decryptedMessageActionDeleteMessages::ID:
      return decryptedMessageActionDeleteMessages::fetch(p);
    case decryptedMessageActionScreenshotMessages::ID:
      return decryptedMessageActionScreenshotMessages::fetch(p);
    case decryptedMessageActionFlushHistory::ID:
      return decryptedMessageActionFlushHistory::fetch(p);
    case decryptedMessageActionResend::ID:
      return decryptedMessageActionResend::fetch(p);
    case decryptedMessageActionNotifyLayer::ID:
      return decryptedMessageActionNotifyLayer::fetch(p);
    case decryptedMessageActionTyping::ID:
      return decryptedMessageActionTyping::fetch(p);
    case decryptedMessageActionRequestKey::ID:
      return decryptedMessageActionRequestKey::fetch(p);
    case decryptedMessageActionAcceptKey::ID:
      return decryptedMessageActionAcceptKey::fetch(p);
    case decryptedMessageActionAbortKey::ID:
      return decryptedMessageActionAbortKey::fetch(p);
    case decryptedMessageActionCommitKey::ID:
      return decryptedMessageActionCommitKey::fetch(p);
    case decryptedMessageActionNoop::ID:
      return decryptedMessageActionNoop::fetch(p);
    case decryptedMessageLayer::ID:
      return decryptedMessageLayer::fetch(p);
    case decryptedMessageMediaEmpty::ID:
      return decryptedMessageMediaEmpty::fetch(p);
    case decryptedMessageMediaPhoto8::ID:
      return decryptedMessageMediaPhoto8::fetch(p);
    case decryptedMessageMediaVideo8::ID:
      return decryptedMessageMediaVideo8::fetch(p);
    case decryptedMessageMediaGeoPoint::ID:
      return decryptedMessageMediaGeoPoint::fetch(p);
    case decryptedMessageMediaContact::ID:
      return decryptedMessageMediaContact::fetch(p);
    case decryptedMessageMediaDocument8::ID:
      return decryptedMessageMediaDocument8::fetch(p);
    case decryptedMessageMediaAudio8::ID:
      return decryptedMessageMediaAudio8::fetch(p);
    case decryptedMessageMediaVideo23::ID:
      return decryptedMessageMediaVideo23::fetch(p);
    case decryptedMessageMediaAudio::ID:
      return decryptedMessageMediaAudio::fetch(p);
    case decryptedMessageMediaExternalDocument::ID:
      return decryptedMessageMediaExternalDocument::fetch(p);
    case decryptedMessageMediaPhoto::ID:
      return decryptedMessageMediaPhoto::fetch(p);
    case decryptedMessageMediaVideo::ID:
      return decryptedMessageMediaVideo::fetch(p);
    case decryptedMessageMediaDocument46::ID:
      return decryptedMessageMediaDocument46::fetch(p);
    case decryptedMessageMediaVenue::ID:
      return decryptedMessageMediaVenue::fetch(p);
    case decryptedMessageMediaWebPage::ID:
      return decryptedMessageMediaWebPage::fetch(p);
    case decryptedMessageMediaDocument::ID:
      return decryptedMessageMediaDocument::fetch(p);
    case documentAttributeImageSize::ID:
      return documentAttributeImageSize::fetch(p);
    case documentAttributeAnimated::ID:
      return documentAttributeAnimated::fetch(p);
    case documentAttributeSticker23::ID:
      return documentAttributeSticker23::fetch(p);
    case documentAttributeVideo23::ID:
      return documentAttributeVideo23::fetch(p);
    case documentAttributeAudio23::ID:
      return documentAttributeAudio23::fetch(p);
    case documentAttributeFilename::ID:
      return documentAttributeFilename::fetch(p);
    case documentAttributeAudio45::ID:
      return documentAttributeAudio45::fetch(p);
    case documentAttributeSticker::ID:
      return documentAttributeSticker::fetch(p);
    case documentAttributeAudio::ID:
      return documentAttributeAudio::fetch(p);
    case documentAttributeVideo::ID:
      return documentAttributeVideo::fetch(p);
    case fileLocationUnavailable::ID:
      return fileLocationUnavailable::fetch(p);
    case fileLocation::ID:
      return fileLocation::fetch(p);
    case inputStickerSetShortName::ID:
      return inputStickerSetShortName::fetch(p);
    case inputStickerSetEmpty::ID:
      return inputStickerSetEmpty::fetch(p);
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
    case messageEntityPhone::ID:
      return messageEntityPhone::fetch(p);
    case messageEntityCashtag::ID:
      return messageEntityCashtag::fetch(p);
    case messageEntityBankCard::ID:
      return messageEntityBankCard::fetch(p);
    case messageEntityUnderline::ID:
      return messageEntityUnderline::fetch(p);
    case messageEntityStrike::ID:
      return messageEntityStrike::fetch(p);
    case messageEntityBlockquote::ID:
      return messageEntityBlockquote::fetch(p);
    case messageEntitySpoiler::ID:
      return messageEntitySpoiler::fetch(p);
    case messageEntityCustomEmoji::ID:
      return messageEntityCustomEmoji::fetch(p);
    case photoSizeEmpty::ID:
      return photoSizeEmpty::fetch(p);
    case photoSize::ID:
      return photoSize::fetch(p);
    case photoCachedSize::ID:
      return photoCachedSize::fetch(p);
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
    case sendMessageRecordRoundAction::ID:
      return sendMessageRecordRoundAction::fetch(p);
    case sendMessageUploadRoundAction::ID:
      return sendMessageUploadRoundAction::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

object_ptr<Function> Function::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case test_dummyFunction::ID:
      return test_dummyFunction::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

object_ptr<DecryptedMessage> DecryptedMessage::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case decryptedMessage8::ID:
      return decryptedMessage8::fetch(p);
    case decryptedMessageService8::ID:
      return decryptedMessageService8::fetch(p);
    case decryptedMessage23::ID:
      return decryptedMessage23::fetch(p);
    case decryptedMessageService::ID:
      return decryptedMessageService::fetch(p);
    case decryptedMessage46::ID:
      return decryptedMessage46::fetch(p);
    case decryptedMessage::ID:
      return decryptedMessage::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

decryptedMessage8::decryptedMessage8(int64 random_id_, bytes &&random_bytes_, string const &message_, object_ptr<DecryptedMessageMedia> &&media_)
  : random_id_(random_id_)
  , random_bytes_(std::move(random_bytes_))
  , message_(message_)
  , media_(std::move(media_))
{}

const std::int32_t decryptedMessage8::ID;

object_ptr<DecryptedMessage> decryptedMessage8::fetch(TlParser &p) {
  return make_tl_object<decryptedMessage8>(p);
}

decryptedMessage8::decryptedMessage8(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_id_(TlFetchLong::parse(p))
  , random_bytes_(TlFetchBytes<bytes>::parse(p))
  , message_(TlFetchString<string>::parse(p))
  , media_(TlFetchObject<DecryptedMessageMedia>::parse(p))
#undef FAIL
{}

void decryptedMessage8::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(random_bytes_, s);
  TlStoreString::store(message_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void decryptedMessage8::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(random_bytes_, s);
  TlStoreString::store(message_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void decryptedMessage8::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessage8");
    s.store_field("random_id", random_id_);
    s.store_bytes_field("random_bytes", random_bytes_);
    s.store_field("message", message_);
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

decryptedMessageService8::decryptedMessageService8(int64 random_id_, bytes &&random_bytes_, object_ptr<DecryptedMessageAction> &&action_)
  : random_id_(random_id_)
  , random_bytes_(std::move(random_bytes_))
  , action_(std::move(action_))
{}

const std::int32_t decryptedMessageService8::ID;

object_ptr<DecryptedMessage> decryptedMessageService8::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageService8>(p);
}

decryptedMessageService8::decryptedMessageService8(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_id_(TlFetchLong::parse(p))
  , random_bytes_(TlFetchBytes<bytes>::parse(p))
  , action_(TlFetchObject<DecryptedMessageAction>::parse(p))
#undef FAIL
{}

void decryptedMessageService8::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(random_bytes_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void decryptedMessageService8::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreString::store(random_bytes_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void decryptedMessageService8::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageService8");
    s.store_field("random_id", random_id_);
    s.store_bytes_field("random_bytes", random_bytes_);
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

decryptedMessage23::decryptedMessage23(int64 random_id_, int32 ttl_, string const &message_, object_ptr<DecryptedMessageMedia> &&media_)
  : random_id_(random_id_)
  , ttl_(ttl_)
  , message_(message_)
  , media_(std::move(media_))
{}

const std::int32_t decryptedMessage23::ID;

object_ptr<DecryptedMessage> decryptedMessage23::fetch(TlParser &p) {
  return make_tl_object<decryptedMessage23>(p);
}

decryptedMessage23::decryptedMessage23(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_id_(TlFetchLong::parse(p))
  , ttl_(TlFetchInt::parse(p))
  , message_(TlFetchString<string>::parse(p))
  , media_(TlFetchObject<DecryptedMessageMedia>::parse(p))
#undef FAIL
{}

void decryptedMessage23::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(ttl_, s);
  TlStoreString::store(message_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void decryptedMessage23::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(ttl_, s);
  TlStoreString::store(message_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(media_, s);
}

void decryptedMessage23::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessage23");
    s.store_field("random_id", random_id_);
    s.store_field("ttl", ttl_);
    s.store_field("message", message_);
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_class_end();
  }
}

decryptedMessageService::decryptedMessageService(int64 random_id_, object_ptr<DecryptedMessageAction> &&action_)
  : random_id_(random_id_)
  , action_(std::move(action_))
{}

const std::int32_t decryptedMessageService::ID;

object_ptr<DecryptedMessage> decryptedMessageService::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageService>(p);
}

decryptedMessageService::decryptedMessageService(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_id_(TlFetchLong::parse(p))
  , action_(TlFetchObject<DecryptedMessageAction>::parse(p))
#undef FAIL
{}

void decryptedMessageService::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void decryptedMessageService::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void decryptedMessageService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageService");
    s.store_field("random_id", random_id_);
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

decryptedMessage46::decryptedMessage46()
  : flags_()
  , random_id_()
  , ttl_()
  , message_()
  , media_()
  , entities_()
  , via_bot_name_()
  , reply_to_random_id_()
{}

decryptedMessage46::decryptedMessage46(int32 flags_, int64 random_id_, int32 ttl_, string const &message_, object_ptr<DecryptedMessageMedia> &&media_, array<object_ptr<MessageEntity>> &&entities_, string const &via_bot_name_, int64 reply_to_random_id_)
  : flags_(flags_)
  , random_id_(random_id_)
  , ttl_(ttl_)
  , message_(message_)
  , media_(std::move(media_))
  , entities_(std::move(entities_))
  , via_bot_name_(via_bot_name_)
  , reply_to_random_id_(reply_to_random_id_)
{}

const std::int32_t decryptedMessage46::ID;

object_ptr<DecryptedMessage> decryptedMessage46::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<decryptedMessage46> res = make_tl_object<decryptedMessage46>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  res->random_id_ = TlFetchLong::parse(p);
  res->ttl_ = TlFetchInt::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 512) { res->media_ = TlFetchObject<DecryptedMessageMedia>::parse(p); }
  if (var0 & 128) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 2048) { res->via_bot_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->reply_to_random_id_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void decryptedMessage46::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(ttl_, s);
  TlStoreString::store(message_, s);
  if (var0 & 512) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 128) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 2048) { TlStoreString::store(via_bot_name_, s); }
  if (var0 & 8) { TlStoreBinary::store(reply_to_random_id_, s); }
}

void decryptedMessage46::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(ttl_, s);
  TlStoreString::store(message_, s);
  if (var0 & 512) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 128) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 2048) { TlStoreString::store(via_bot_name_, s); }
  if (var0 & 8) { TlStoreBinary::store(reply_to_random_id_, s); }
}

void decryptedMessage46::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessage46");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("random_id", random_id_);
    s.store_field("ttl", ttl_);
    s.store_field("message", message_);
    if (var0 & 512) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 128) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2048) { s.store_field("via_bot_name", via_bot_name_); }
    if (var0 & 8) { s.store_field("reply_to_random_id", reply_to_random_id_); }
    s.store_class_end();
  }
}

decryptedMessage::decryptedMessage()
  : flags_()
  , silent_()
  , random_id_()
  , ttl_()
  , message_()
  , media_()
  , entities_()
  , via_bot_name_()
  , reply_to_random_id_()
  , grouped_id_()
{}

decryptedMessage::decryptedMessage(int32 flags_, bool silent_, int64 random_id_, int32 ttl_, string const &message_, object_ptr<DecryptedMessageMedia> &&media_, array<object_ptr<MessageEntity>> &&entities_, string const &via_bot_name_, int64 reply_to_random_id_, int64 grouped_id_)
  : flags_(flags_)
  , silent_(silent_)
  , random_id_(random_id_)
  , ttl_(ttl_)
  , message_(message_)
  , media_(std::move(media_))
  , entities_(std::move(entities_))
  , via_bot_name_(via_bot_name_)
  , reply_to_random_id_(reply_to_random_id_)
  , grouped_id_(grouped_id_)
{}

const std::int32_t decryptedMessage::ID;

object_ptr<DecryptedMessage> decryptedMessage::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<decryptedMessage> res = make_tl_object<decryptedMessage>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 32) { res->silent_ = TlFetchTrue::parse(p); }
  res->random_id_ = TlFetchLong::parse(p);
  res->ttl_ = TlFetchInt::parse(p);
  res->message_ = TlFetchString<string>::parse(p);
  if (var0 & 512) { res->media_ = TlFetchObject<DecryptedMessageMedia>::parse(p); }
  if (var0 & 128) { res->entities_ = TlFetchBoxed<TlFetchVector<TlFetchObject<MessageEntity>>, 481674261>::parse(p); }
  if (var0 & 2048) { res->via_bot_name_ = TlFetchString<string>::parse(p); }
  if (var0 & 8) { res->reply_to_random_id_ = TlFetchLong::parse(p); }
  if (var0 & 131072) { res->grouped_id_ = TlFetchLong::parse(p); }
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void decryptedMessage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(ttl_, s);
  TlStoreString::store(message_, s);
  if (var0 & 512) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 128) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 2048) { TlStoreString::store(via_bot_name_, s); }
  if (var0 & 8) { TlStoreBinary::store(reply_to_random_id_, s); }
  if (var0 & 131072) { TlStoreBinary::store(grouped_id_, s); }
}

void decryptedMessage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(random_id_, s);
  TlStoreBinary::store(ttl_, s);
  TlStoreString::store(message_, s);
  if (var0 & 512) { TlStoreBoxedUnknown<TlStoreObject>::store(media_, s); }
  if (var0 & 128) { TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(entities_, s); }
  if (var0 & 2048) { TlStoreString::store(via_bot_name_, s); }
  if (var0 & 8) { TlStoreBinary::store(reply_to_random_id_, s); }
  if (var0 & 131072) { TlStoreBinary::store(grouped_id_, s); }
}

void decryptedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessage");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("random_id", random_id_);
    s.store_field("ttl", ttl_);
    s.store_field("message", message_);
    if (var0 & 512) { s.store_object_field("media", static_cast<const BaseObject *>(media_.get())); }
    if (var0 & 128) { { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); } }
    if (var0 & 2048) { s.store_field("via_bot_name", via_bot_name_); }
    if (var0 & 8) { s.store_field("reply_to_random_id", reply_to_random_id_); }
    if (var0 & 131072) { s.store_field("grouped_id", grouped_id_); }
    s.store_class_end();
  }
}

object_ptr<DecryptedMessageAction> DecryptedMessageAction::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case decryptedMessageActionSetMessageTTL::ID:
      return decryptedMessageActionSetMessageTTL::fetch(p);
    case decryptedMessageActionReadMessages::ID:
      return decryptedMessageActionReadMessages::fetch(p);
    case decryptedMessageActionDeleteMessages::ID:
      return decryptedMessageActionDeleteMessages::fetch(p);
    case decryptedMessageActionScreenshotMessages::ID:
      return decryptedMessageActionScreenshotMessages::fetch(p);
    case decryptedMessageActionFlushHistory::ID:
      return decryptedMessageActionFlushHistory::fetch(p);
    case decryptedMessageActionResend::ID:
      return decryptedMessageActionResend::fetch(p);
    case decryptedMessageActionNotifyLayer::ID:
      return decryptedMessageActionNotifyLayer::fetch(p);
    case decryptedMessageActionTyping::ID:
      return decryptedMessageActionTyping::fetch(p);
    case decryptedMessageActionRequestKey::ID:
      return decryptedMessageActionRequestKey::fetch(p);
    case decryptedMessageActionAcceptKey::ID:
      return decryptedMessageActionAcceptKey::fetch(p);
    case decryptedMessageActionAbortKey::ID:
      return decryptedMessageActionAbortKey::fetch(p);
    case decryptedMessageActionCommitKey::ID:
      return decryptedMessageActionCommitKey::fetch(p);
    case decryptedMessageActionNoop::ID:
      return decryptedMessageActionNoop::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

decryptedMessageActionSetMessageTTL::decryptedMessageActionSetMessageTTL(int32 ttl_seconds_)
  : ttl_seconds_(ttl_seconds_)
{}

const std::int32_t decryptedMessageActionSetMessageTTL::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionSetMessageTTL::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionSetMessageTTL>(p);
}

decryptedMessageActionSetMessageTTL::decryptedMessageActionSetMessageTTL(TlParser &p)
#define FAIL(error) p.set_error(error)
  : ttl_seconds_(TlFetchInt::parse(p))
#undef FAIL
{}

void decryptedMessageActionSetMessageTTL::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(ttl_seconds_, s);
}

void decryptedMessageActionSetMessageTTL::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(ttl_seconds_, s);
}

void decryptedMessageActionSetMessageTTL::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionSetMessageTTL");
    s.store_field("ttl_seconds", ttl_seconds_);
    s.store_class_end();
  }
}

decryptedMessageActionReadMessages::decryptedMessageActionReadMessages(array<int64> &&random_ids_)
  : random_ids_(std::move(random_ids_))
{}

const std::int32_t decryptedMessageActionReadMessages::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionReadMessages::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionReadMessages>(p);
}

decryptedMessageActionReadMessages::decryptedMessageActionReadMessages(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
#undef FAIL
{}

void decryptedMessageActionReadMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_ids_, s);
}

void decryptedMessageActionReadMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_ids_, s);
}

void decryptedMessageActionReadMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionReadMessages");
    { s.store_vector_begin("random_ids", random_ids_.size()); for (const auto &_value : random_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

decryptedMessageActionDeleteMessages::decryptedMessageActionDeleteMessages(array<int64> &&random_ids_)
  : random_ids_(std::move(random_ids_))
{}

const std::int32_t decryptedMessageActionDeleteMessages::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionDeleteMessages::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionDeleteMessages>(p);
}

decryptedMessageActionDeleteMessages::decryptedMessageActionDeleteMessages(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
#undef FAIL
{}

void decryptedMessageActionDeleteMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_ids_, s);
}

void decryptedMessageActionDeleteMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_ids_, s);
}

void decryptedMessageActionDeleteMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionDeleteMessages");
    { s.store_vector_begin("random_ids", random_ids_.size()); for (const auto &_value : random_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

decryptedMessageActionScreenshotMessages::decryptedMessageActionScreenshotMessages(array<int64> &&random_ids_)
  : random_ids_(std::move(random_ids_))
{}

const std::int32_t decryptedMessageActionScreenshotMessages::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionScreenshotMessages::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionScreenshotMessages>(p);
}

decryptedMessageActionScreenshotMessages::decryptedMessageActionScreenshotMessages(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_ids_(TlFetchBoxed<TlFetchVector<TlFetchLong>, 481674261>::parse(p))
#undef FAIL
{}

void decryptedMessageActionScreenshotMessages::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_ids_, s);
}

void decryptedMessageActionScreenshotMessages::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxed<TlStoreVector<TlStoreBinary>, 481674261>::store(random_ids_, s);
}

void decryptedMessageActionScreenshotMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionScreenshotMessages");
    { s.store_vector_begin("random_ids", random_ids_.size()); for (const auto &_value : random_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

const std::int32_t decryptedMessageActionFlushHistory::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionFlushHistory::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionFlushHistory>();
}

void decryptedMessageActionFlushHistory::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void decryptedMessageActionFlushHistory::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void decryptedMessageActionFlushHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionFlushHistory");
    s.store_class_end();
  }
}

decryptedMessageActionResend::decryptedMessageActionResend(int32 start_seq_no_, int32 end_seq_no_)
  : start_seq_no_(start_seq_no_)
  , end_seq_no_(end_seq_no_)
{}

const std::int32_t decryptedMessageActionResend::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionResend::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionResend>(p);
}

decryptedMessageActionResend::decryptedMessageActionResend(TlParser &p)
#define FAIL(error) p.set_error(error)
  : start_seq_no_(TlFetchInt::parse(p))
  , end_seq_no_(TlFetchInt::parse(p))
#undef FAIL
{}

void decryptedMessageActionResend::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(start_seq_no_, s);
  TlStoreBinary::store(end_seq_no_, s);
}

void decryptedMessageActionResend::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(start_seq_no_, s);
  TlStoreBinary::store(end_seq_no_, s);
}

void decryptedMessageActionResend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionResend");
    s.store_field("start_seq_no", start_seq_no_);
    s.store_field("end_seq_no", end_seq_no_);
    s.store_class_end();
  }
}

decryptedMessageActionNotifyLayer::decryptedMessageActionNotifyLayer(int32 layer_)
  : layer_(layer_)
{}

const std::int32_t decryptedMessageActionNotifyLayer::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionNotifyLayer::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionNotifyLayer>(p);
}

decryptedMessageActionNotifyLayer::decryptedMessageActionNotifyLayer(TlParser &p)
#define FAIL(error) p.set_error(error)
  : layer_(TlFetchInt::parse(p))
#undef FAIL
{}

void decryptedMessageActionNotifyLayer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(layer_, s);
}

void decryptedMessageActionNotifyLayer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(layer_, s);
}

void decryptedMessageActionNotifyLayer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionNotifyLayer");
    s.store_field("layer", layer_);
    s.store_class_end();
  }
}

decryptedMessageActionTyping::decryptedMessageActionTyping(object_ptr<SendMessageAction> &&action_)
  : action_(std::move(action_))
{}

const std::int32_t decryptedMessageActionTyping::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionTyping::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionTyping>(p);
}

decryptedMessageActionTyping::decryptedMessageActionTyping(TlParser &p)
#define FAIL(error) p.set_error(error)
  : action_(TlFetchObject<SendMessageAction>::parse(p))
#undef FAIL
{}

void decryptedMessageActionTyping::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void decryptedMessageActionTyping::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBoxedUnknown<TlStoreObject>::store(action_, s);
}

void decryptedMessageActionTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionTyping");
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

decryptedMessageActionRequestKey::decryptedMessageActionRequestKey(int64 exchange_id_, bytes &&g_a_)
  : exchange_id_(exchange_id_)
  , g_a_(std::move(g_a_))
{}

const std::int32_t decryptedMessageActionRequestKey::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionRequestKey::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionRequestKey>(p);
}

decryptedMessageActionRequestKey::decryptedMessageActionRequestKey(TlParser &p)
#define FAIL(error) p.set_error(error)
  : exchange_id_(TlFetchLong::parse(p))
  , g_a_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void decryptedMessageActionRequestKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
  TlStoreString::store(g_a_, s);
}

void decryptedMessageActionRequestKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
  TlStoreString::store(g_a_, s);
}

void decryptedMessageActionRequestKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionRequestKey");
    s.store_field("exchange_id", exchange_id_);
    s.store_bytes_field("g_a", g_a_);
    s.store_class_end();
  }
}

decryptedMessageActionAcceptKey::decryptedMessageActionAcceptKey(int64 exchange_id_, bytes &&g_b_, int64 key_fingerprint_)
  : exchange_id_(exchange_id_)
  , g_b_(std::move(g_b_))
  , key_fingerprint_(key_fingerprint_)
{}

const std::int32_t decryptedMessageActionAcceptKey::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionAcceptKey::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionAcceptKey>(p);
}

decryptedMessageActionAcceptKey::decryptedMessageActionAcceptKey(TlParser &p)
#define FAIL(error) p.set_error(error)
  : exchange_id_(TlFetchLong::parse(p))
  , g_b_(TlFetchBytes<bytes>::parse(p))
  , key_fingerprint_(TlFetchLong::parse(p))
#undef FAIL
{}

void decryptedMessageActionAcceptKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
  TlStoreString::store(g_b_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void decryptedMessageActionAcceptKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
  TlStoreString::store(g_b_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void decryptedMessageActionAcceptKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionAcceptKey");
    s.store_field("exchange_id", exchange_id_);
    s.store_bytes_field("g_b", g_b_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_class_end();
  }
}

decryptedMessageActionAbortKey::decryptedMessageActionAbortKey(int64 exchange_id_)
  : exchange_id_(exchange_id_)
{}

const std::int32_t decryptedMessageActionAbortKey::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionAbortKey::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionAbortKey>(p);
}

decryptedMessageActionAbortKey::decryptedMessageActionAbortKey(TlParser &p)
#define FAIL(error) p.set_error(error)
  : exchange_id_(TlFetchLong::parse(p))
#undef FAIL
{}

void decryptedMessageActionAbortKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
}

void decryptedMessageActionAbortKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
}

void decryptedMessageActionAbortKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionAbortKey");
    s.store_field("exchange_id", exchange_id_);
    s.store_class_end();
  }
}

decryptedMessageActionCommitKey::decryptedMessageActionCommitKey(int64 exchange_id_, int64 key_fingerprint_)
  : exchange_id_(exchange_id_)
  , key_fingerprint_(key_fingerprint_)
{}

const std::int32_t decryptedMessageActionCommitKey::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionCommitKey::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionCommitKey>(p);
}

decryptedMessageActionCommitKey::decryptedMessageActionCommitKey(TlParser &p)
#define FAIL(error) p.set_error(error)
  : exchange_id_(TlFetchLong::parse(p))
  , key_fingerprint_(TlFetchLong::parse(p))
#undef FAIL
{}

void decryptedMessageActionCommitKey::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void decryptedMessageActionCommitKey::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(exchange_id_, s);
  TlStoreBinary::store(key_fingerprint_, s);
}

void decryptedMessageActionCommitKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionCommitKey");
    s.store_field("exchange_id", exchange_id_);
    s.store_field("key_fingerprint", key_fingerprint_);
    s.store_class_end();
  }
}

const std::int32_t decryptedMessageActionNoop::ID;

object_ptr<DecryptedMessageAction> decryptedMessageActionNoop::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageActionNoop>();
}

void decryptedMessageActionNoop::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void decryptedMessageActionNoop::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void decryptedMessageActionNoop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageActionNoop");
    s.store_class_end();
  }
}

decryptedMessageLayer::decryptedMessageLayer(bytes &&random_bytes_, int32 layer_, int32 in_seq_no_, int32 out_seq_no_, object_ptr<DecryptedMessage> &&message_)
  : random_bytes_(std::move(random_bytes_))
  , layer_(layer_)
  , in_seq_no_(in_seq_no_)
  , out_seq_no_(out_seq_no_)
  , message_(std::move(message_))
{}

const std::int32_t decryptedMessageLayer::ID;

object_ptr<decryptedMessageLayer> decryptedMessageLayer::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageLayer>(p);
}

decryptedMessageLayer::decryptedMessageLayer(TlParser &p)
#define FAIL(error) p.set_error(error)
  : random_bytes_(TlFetchBytes<bytes>::parse(p))
  , layer_(TlFetchInt::parse(p))
  , in_seq_no_(TlFetchInt::parse(p))
  , out_seq_no_(TlFetchInt::parse(p))
  , message_(TlFetchObject<DecryptedMessage>::parse(p))
#undef FAIL
{}

void decryptedMessageLayer::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(random_bytes_, s);
  TlStoreBinary::store(layer_, s);
  TlStoreBinary::store(in_seq_no_, s);
  TlStoreBinary::store(out_seq_no_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(message_, s);
}

void decryptedMessageLayer::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(random_bytes_, s);
  TlStoreBinary::store(layer_, s);
  TlStoreBinary::store(in_seq_no_, s);
  TlStoreBinary::store(out_seq_no_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(message_, s);
}

void decryptedMessageLayer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageLayer");
    s.store_bytes_field("random_bytes", random_bytes_);
    s.store_field("layer", layer_);
    s.store_field("in_seq_no", in_seq_no_);
    s.store_field("out_seq_no", out_seq_no_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

object_ptr<DecryptedMessageMedia> DecryptedMessageMedia::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case decryptedMessageMediaEmpty::ID:
      return decryptedMessageMediaEmpty::fetch(p);
    case decryptedMessageMediaPhoto8::ID:
      return decryptedMessageMediaPhoto8::fetch(p);
    case decryptedMessageMediaVideo8::ID:
      return decryptedMessageMediaVideo8::fetch(p);
    case decryptedMessageMediaGeoPoint::ID:
      return decryptedMessageMediaGeoPoint::fetch(p);
    case decryptedMessageMediaContact::ID:
      return decryptedMessageMediaContact::fetch(p);
    case decryptedMessageMediaDocument8::ID:
      return decryptedMessageMediaDocument8::fetch(p);
    case decryptedMessageMediaAudio8::ID:
      return decryptedMessageMediaAudio8::fetch(p);
    case decryptedMessageMediaVideo23::ID:
      return decryptedMessageMediaVideo23::fetch(p);
    case decryptedMessageMediaAudio::ID:
      return decryptedMessageMediaAudio::fetch(p);
    case decryptedMessageMediaExternalDocument::ID:
      return decryptedMessageMediaExternalDocument::fetch(p);
    case decryptedMessageMediaPhoto::ID:
      return decryptedMessageMediaPhoto::fetch(p);
    case decryptedMessageMediaVideo::ID:
      return decryptedMessageMediaVideo::fetch(p);
    case decryptedMessageMediaDocument46::ID:
      return decryptedMessageMediaDocument46::fetch(p);
    case decryptedMessageMediaVenue::ID:
      return decryptedMessageMediaVenue::fetch(p);
    case decryptedMessageMediaWebPage::ID:
      return decryptedMessageMediaWebPage::fetch(p);
    case decryptedMessageMediaDocument::ID:
      return decryptedMessageMediaDocument::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t decryptedMessageMediaEmpty::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaEmpty::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaEmpty>();
}

void decryptedMessageMediaEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void decryptedMessageMediaEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void decryptedMessageMediaEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaEmpty");
    s.store_class_end();
  }
}

decryptedMessageMediaPhoto8::decryptedMessageMediaPhoto8(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, int32 w_, int32 h_, int32 size_, bytes &&key_, bytes &&iv_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , w_(w_)
  , h_(h_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
{}

const std::int32_t decryptedMessageMediaPhoto8::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaPhoto8::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaPhoto8>(p);
}

decryptedMessageMediaPhoto8::decryptedMessageMediaPhoto8(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaPhoto8::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaPhoto8::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaPhoto8::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaPhoto8");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_class_end();
  }
}

decryptedMessageMediaVideo8::decryptedMessageMediaVideo8(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, int32 duration_, int32 w_, int32 h_, int32 size_, bytes &&key_, bytes &&iv_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , duration_(duration_)
  , w_(w_)
  , h_(h_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
{}

const std::int32_t decryptedMessageMediaVideo8::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaVideo8::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaVideo8>(p);
}

decryptedMessageMediaVideo8::decryptedMessageMediaVideo8(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , duration_(TlFetchInt::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaVideo8::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaVideo8::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaVideo8::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaVideo8");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("duration", duration_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_class_end();
  }
}

decryptedMessageMediaGeoPoint::decryptedMessageMediaGeoPoint(double lat_, double long_)
  : lat_(lat_)
  , long_(long_)
{}

const std::int32_t decryptedMessageMediaGeoPoint::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaGeoPoint::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaGeoPoint>(p);
}

decryptedMessageMediaGeoPoint::decryptedMessageMediaGeoPoint(TlParser &p)
#define FAIL(error) p.set_error(error)
  : lat_(TlFetchDouble::parse(p))
  , long_(TlFetchDouble::parse(p))
#undef FAIL
{}

void decryptedMessageMediaGeoPoint::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(long_, s);
}

void decryptedMessageMediaGeoPoint::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(long_, s);
}

void decryptedMessageMediaGeoPoint::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaGeoPoint");
    s.store_field("lat", lat_);
    s.store_field("long", long_);
    s.store_class_end();
  }
}

decryptedMessageMediaContact::decryptedMessageMediaContact(string const &phone_number_, string const &first_name_, string const &last_name_, int32 user_id_)
  : phone_number_(phone_number_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , user_id_(user_id_)
{}

const std::int32_t decryptedMessageMediaContact::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaContact::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaContact>(p);
}

decryptedMessageMediaContact::decryptedMessageMediaContact(TlParser &p)
#define FAIL(error) p.set_error(error)
  : phone_number_(TlFetchString<string>::parse(p))
  , first_name_(TlFetchString<string>::parse(p))
  , last_name_(TlFetchString<string>::parse(p))
  , user_id_(TlFetchInt::parse(p))
#undef FAIL
{}

void decryptedMessageMediaContact::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreBinary::store(user_id_, s);
}

void decryptedMessageMediaContact::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(phone_number_, s);
  TlStoreString::store(first_name_, s);
  TlStoreString::store(last_name_, s);
  TlStoreBinary::store(user_id_, s);
}

void decryptedMessageMediaContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaContact");
    s.store_field("phone_number", phone_number_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

decryptedMessageMediaDocument8::decryptedMessageMediaDocument8(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, string const &file_name_, string const &mime_type_, int32 size_, bytes &&key_, bytes &&iv_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , file_name_(file_name_)
  , mime_type_(mime_type_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
{}

const std::int32_t decryptedMessageMediaDocument8::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaDocument8::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaDocument8>(p);
}

decryptedMessageMediaDocument8::decryptedMessageMediaDocument8(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , file_name_(TlFetchString<string>::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaDocument8::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreString::store(file_name_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaDocument8::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreString::store(file_name_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaDocument8::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaDocument8");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("file_name", file_name_);
    s.store_field("mime_type", mime_type_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_class_end();
  }
}

decryptedMessageMediaAudio8::decryptedMessageMediaAudio8(int32 duration_, int32 size_, bytes &&key_, bytes &&iv_)
  : duration_(duration_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
{}

const std::int32_t decryptedMessageMediaAudio8::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaAudio8::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaAudio8>(p);
}

decryptedMessageMediaAudio8::decryptedMessageMediaAudio8(TlParser &p)
#define FAIL(error) p.set_error(error)
  : duration_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaAudio8::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaAudio8::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaAudio8::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaAudio8");
    s.store_field("duration", duration_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_class_end();
  }
}

decryptedMessageMediaVideo23::decryptedMessageMediaVideo23(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, int32 duration_, string const &mime_type_, int32 w_, int32 h_, int32 size_, bytes &&key_, bytes &&iv_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , duration_(duration_)
  , mime_type_(mime_type_)
  , w_(w_)
  , h_(h_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
{}

const std::int32_t decryptedMessageMediaVideo23::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaVideo23::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaVideo23>(p);
}

decryptedMessageMediaVideo23::decryptedMessageMediaVideo23(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , duration_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaVideo23::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaVideo23::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaVideo23::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaVideo23");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("duration", duration_);
    s.store_field("mime_type", mime_type_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_class_end();
  }
}

decryptedMessageMediaAudio::decryptedMessageMediaAudio(int32 duration_, string const &mime_type_, int32 size_, bytes &&key_, bytes &&iv_)
  : duration_(duration_)
  , mime_type_(mime_type_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
{}

const std::int32_t decryptedMessageMediaAudio::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaAudio::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaAudio>(p);
}

decryptedMessageMediaAudio::decryptedMessageMediaAudio(TlParser &p)
#define FAIL(error) p.set_error(error)
  : duration_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaAudio::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaAudio::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
}

void decryptedMessageMediaAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaAudio");
    s.store_field("duration", duration_);
    s.store_field("mime_type", mime_type_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_class_end();
  }
}

decryptedMessageMediaExternalDocument::decryptedMessageMediaExternalDocument(int64 id_, int64 access_hash_, int32 date_, string const &mime_type_, int32 size_, object_ptr<PhotoSize> &&thumb_, int32 dc_id_, array<object_ptr<DocumentAttribute>> &&attributes_)
  : id_(id_)
  , access_hash_(access_hash_)
  , date_(date_)
  , mime_type_(mime_type_)
  , size_(size_)
  , thumb_(std::move(thumb_))
  , dc_id_(dc_id_)
  , attributes_(std::move(attributes_))
{}

const std::int32_t decryptedMessageMediaExternalDocument::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaExternalDocument::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaExternalDocument>(p);
}

decryptedMessageMediaExternalDocument::decryptedMessageMediaExternalDocument(TlParser &p)
#define FAIL(error) p.set_error(error)
  : id_(TlFetchLong::parse(p))
  , access_hash_(TlFetchLong::parse(p))
  , date_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , size_(TlFetchInt::parse(p))
  , thumb_(TlFetchObject<PhotoSize>::parse(p))
  , dc_id_(TlFetchInt::parse(p))
  , attributes_(TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaExternalDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreBinary::store(date_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
}

void decryptedMessageMediaExternalDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(id_, s);
  TlStoreBinary::store(access_hash_, s);
  TlStoreBinary::store(date_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(thumb_, s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
}

void decryptedMessageMediaExternalDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaExternalDocument");
    s.store_field("id", id_);
    s.store_field("access_hash", access_hash_);
    s.store_field("date", date_);
    s.store_field("mime_type", mime_type_);
    s.store_field("size", size_);
    s.store_object_field("thumb", static_cast<const BaseObject *>(thumb_.get()));
    s.store_field("dc_id", dc_id_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

decryptedMessageMediaPhoto::decryptedMessageMediaPhoto(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, int32 w_, int32 h_, int32 size_, bytes &&key_, bytes &&iv_, string const &caption_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , w_(w_)
  , h_(h_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
  , caption_(caption_)
{}

const std::int32_t decryptedMessageMediaPhoto::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaPhoto::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaPhoto>(p);
}

decryptedMessageMediaPhoto::decryptedMessageMediaPhoto(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
  , caption_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaPhoto::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaPhoto::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaPhoto");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_field("caption", caption_);
    s.store_class_end();
  }
}

decryptedMessageMediaVideo::decryptedMessageMediaVideo(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, int32 duration_, string const &mime_type_, int32 w_, int32 h_, int32 size_, bytes &&key_, bytes &&iv_, string const &caption_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , duration_(duration_)
  , mime_type_(mime_type_)
  , w_(w_)
  , h_(h_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
  , caption_(caption_)
{}

const std::int32_t decryptedMessageMediaVideo::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaVideo::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaVideo>(p);
}

decryptedMessageMediaVideo::decryptedMessageMediaVideo(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , duration_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
  , caption_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaVideo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaVideo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaVideo");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("duration", duration_);
    s.store_field("mime_type", mime_type_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    s.store_field("caption", caption_);
    s.store_class_end();
  }
}

decryptedMessageMediaDocument46::decryptedMessageMediaDocument46(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, string const &mime_type_, int32 size_, bytes &&key_, bytes &&iv_, array<object_ptr<DocumentAttribute>> &&attributes_, string const &caption_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , mime_type_(mime_type_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
  , attributes_(std::move(attributes_))
  , caption_(caption_)
{}

const std::int32_t decryptedMessageMediaDocument46::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaDocument46::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaDocument46>(p);
}

decryptedMessageMediaDocument46::decryptedMessageMediaDocument46(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , size_(TlFetchInt::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
  , attributes_(TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p))
  , caption_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaDocument46::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaDocument46::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaDocument46::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaDocument46");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("mime_type", mime_type_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("caption", caption_);
    s.store_class_end();
  }
}

decryptedMessageMediaVenue::decryptedMessageMediaVenue(double lat_, double long_, string const &title_, string const &address_, string const &provider_, string const &venue_id_)
  : lat_(lat_)
  , long_(long_)
  , title_(title_)
  , address_(address_)
  , provider_(provider_)
  , venue_id_(venue_id_)
{}

const std::int32_t decryptedMessageMediaVenue::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaVenue::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaVenue>(p);
}

decryptedMessageMediaVenue::decryptedMessageMediaVenue(TlParser &p)
#define FAIL(error) p.set_error(error)
  : lat_(TlFetchDouble::parse(p))
  , long_(TlFetchDouble::parse(p))
  , title_(TlFetchString<string>::parse(p))
  , address_(TlFetchString<string>::parse(p))
  , provider_(TlFetchString<string>::parse(p))
  , venue_id_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaVenue::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(long_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
}

void decryptedMessageMediaVenue::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(lat_, s);
  TlStoreBinary::store(long_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(address_, s);
  TlStoreString::store(provider_, s);
  TlStoreString::store(venue_id_, s);
}

void decryptedMessageMediaVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaVenue");
    s.store_field("lat", lat_);
    s.store_field("long", long_);
    s.store_field("title", title_);
    s.store_field("address", address_);
    s.store_field("provider", provider_);
    s.store_field("venue_id", venue_id_);
    s.store_class_end();
  }
}

decryptedMessageMediaWebPage::decryptedMessageMediaWebPage(string const &url_)
  : url_(url_)
{}

const std::int32_t decryptedMessageMediaWebPage::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaWebPage::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaWebPage>(p);
}

decryptedMessageMediaWebPage::decryptedMessageMediaWebPage(TlParser &p)
#define FAIL(error) p.set_error(error)
  : url_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaWebPage::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(url_, s);
}

void decryptedMessageMediaWebPage::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(url_, s);
}

void decryptedMessageMediaWebPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaWebPage");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

decryptedMessageMediaDocument::decryptedMessageMediaDocument(bytes &&thumb_, int32 thumb_w_, int32 thumb_h_, string const &mime_type_, int64 size_, bytes &&key_, bytes &&iv_, array<object_ptr<DocumentAttribute>> &&attributes_, string const &caption_)
  : thumb_(std::move(thumb_))
  , thumb_w_(thumb_w_)
  , thumb_h_(thumb_h_)
  , mime_type_(mime_type_)
  , size_(size_)
  , key_(std::move(key_))
  , iv_(std::move(iv_))
  , attributes_(std::move(attributes_))
  , caption_(caption_)
{}

const std::int32_t decryptedMessageMediaDocument::ID;

object_ptr<DecryptedMessageMedia> decryptedMessageMediaDocument::fetch(TlParser &p) {
  return make_tl_object<decryptedMessageMediaDocument>(p);
}

decryptedMessageMediaDocument::decryptedMessageMediaDocument(TlParser &p)
#define FAIL(error) p.set_error(error)
  : thumb_(TlFetchBytes<bytes>::parse(p))
  , thumb_w_(TlFetchInt::parse(p))
  , thumb_h_(TlFetchInt::parse(p))
  , mime_type_(TlFetchString<string>::parse(p))
  , size_(TlFetchLong::parse(p))
  , key_(TlFetchBytes<bytes>::parse(p))
  , iv_(TlFetchBytes<bytes>::parse(p))
  , attributes_(TlFetchBoxed<TlFetchVector<TlFetchObject<DocumentAttribute>>, 481674261>::parse(p))
  , caption_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void decryptedMessageMediaDocument::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaDocument::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(thumb_, s);
  TlStoreBinary::store(thumb_w_, s);
  TlStoreBinary::store(thumb_h_, s);
  TlStoreString::store(mime_type_, s);
  TlStoreBinary::store(size_, s);
  TlStoreString::store(key_, s);
  TlStoreString::store(iv_, s);
  TlStoreBoxed<TlStoreVector<TlStoreBoxedUnknown<TlStoreObject>>, 481674261>::store(attributes_, s);
  TlStoreString::store(caption_, s);
}

void decryptedMessageMediaDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptedMessageMediaDocument");
    s.store_bytes_field("thumb", thumb_);
    s.store_field("thumb_w", thumb_w_);
    s.store_field("thumb_h", thumb_h_);
    s.store_field("mime_type", mime_type_);
    s.store_field("size", size_);
    s.store_bytes_field("key", key_);
    s.store_bytes_field("iv", iv_);
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("caption", caption_);
    s.store_class_end();
  }
}

object_ptr<DocumentAttribute> DocumentAttribute::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case documentAttributeImageSize::ID:
      return documentAttributeImageSize::fetch(p);
    case documentAttributeAnimated::ID:
      return documentAttributeAnimated::fetch(p);
    case documentAttributeSticker23::ID:
      return documentAttributeSticker23::fetch(p);
    case documentAttributeVideo23::ID:
      return documentAttributeVideo23::fetch(p);
    case documentAttributeAudio23::ID:
      return documentAttributeAudio23::fetch(p);
    case documentAttributeFilename::ID:
      return documentAttributeFilename::fetch(p);
    case documentAttributeAudio45::ID:
      return documentAttributeAudio45::fetch(p);
    case documentAttributeSticker::ID:
      return documentAttributeSticker::fetch(p);
    case documentAttributeAudio::ID:
      return documentAttributeAudio::fetch(p);
    case documentAttributeVideo::ID:
      return documentAttributeVideo::fetch(p);
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

object_ptr<DocumentAttribute> documentAttributeImageSize::fetch(TlParser &p) {
  return make_tl_object<documentAttributeImageSize>(p);
}

documentAttributeImageSize::documentAttributeImageSize(TlParser &p)
#define FAIL(error) p.set_error(error)
  : w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<DocumentAttribute> documentAttributeAnimated::fetch(TlParser &p) {
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

const std::int32_t documentAttributeSticker23::ID;

object_ptr<DocumentAttribute> documentAttributeSticker23::fetch(TlParser &p) {
  return make_tl_object<documentAttributeSticker23>();
}

void documentAttributeSticker23::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void documentAttributeSticker23::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void documentAttributeSticker23::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeSticker23");
    s.store_class_end();
  }
}

documentAttributeVideo23::documentAttributeVideo23(int32 duration_, int32 w_, int32 h_)
  : duration_(duration_)
  , w_(w_)
  , h_(h_)
{}

const std::int32_t documentAttributeVideo23::ID;

object_ptr<DocumentAttribute> documentAttributeVideo23::fetch(TlParser &p) {
  return make_tl_object<documentAttributeVideo23>(p);
}

documentAttributeVideo23::documentAttributeVideo23(TlParser &p)
#define FAIL(error) p.set_error(error)
  : duration_(TlFetchInt::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
#undef FAIL
{}

void documentAttributeVideo23::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
}

void documentAttributeVideo23::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
}

void documentAttributeVideo23::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeVideo23");
    s.store_field("duration", duration_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_class_end();
  }
}

documentAttributeAudio23::documentAttributeAudio23(int32 duration_)
  : duration_(duration_)
{}

const std::int32_t documentAttributeAudio23::ID;

object_ptr<DocumentAttribute> documentAttributeAudio23::fetch(TlParser &p) {
  return make_tl_object<documentAttributeAudio23>(p);
}

documentAttributeAudio23::documentAttributeAudio23(TlParser &p)
#define FAIL(error) p.set_error(error)
  : duration_(TlFetchInt::parse(p))
#undef FAIL
{}

void documentAttributeAudio23::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
}

void documentAttributeAudio23::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
}

void documentAttributeAudio23::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeAudio23");
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

documentAttributeFilename::documentAttributeFilename(string const &file_name_)
  : file_name_(file_name_)
{}

const std::int32_t documentAttributeFilename::ID;

object_ptr<DocumentAttribute> documentAttributeFilename::fetch(TlParser &p) {
  return make_tl_object<documentAttributeFilename>(p);
}

documentAttributeFilename::documentAttributeFilename(TlParser &p)
#define FAIL(error) p.set_error(error)
  : file_name_(TlFetchString<string>::parse(p))
#undef FAIL
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

documentAttributeAudio45::documentAttributeAudio45(int32 duration_, string const &title_, string const &performer_)
  : duration_(duration_)
  , title_(title_)
  , performer_(performer_)
{}

const std::int32_t documentAttributeAudio45::ID;

object_ptr<DocumentAttribute> documentAttributeAudio45::fetch(TlParser &p) {
  return make_tl_object<documentAttributeAudio45>(p);
}

documentAttributeAudio45::documentAttributeAudio45(TlParser &p)
#define FAIL(error) p.set_error(error)
  : duration_(TlFetchInt::parse(p))
  , title_(TlFetchString<string>::parse(p))
  , performer_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void documentAttributeAudio45::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(performer_, s);
}

void documentAttributeAudio45::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(duration_, s);
  TlStoreString::store(title_, s);
  TlStoreString::store(performer_, s);
}

void documentAttributeAudio45::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeAudio45");
    s.store_field("duration", duration_);
    s.store_field("title", title_);
    s.store_field("performer", performer_);
    s.store_class_end();
  }
}

documentAttributeSticker::documentAttributeSticker(string const &alt_, object_ptr<InputStickerSet> &&stickerset_)
  : alt_(alt_)
  , stickerset_(std::move(stickerset_))
{}

const std::int32_t documentAttributeSticker::ID;

object_ptr<DocumentAttribute> documentAttributeSticker::fetch(TlParser &p) {
  return make_tl_object<documentAttributeSticker>(p);
}

documentAttributeSticker::documentAttributeSticker(TlParser &p)
#define FAIL(error) p.set_error(error)
  : alt_(TlFetchString<string>::parse(p))
  , stickerset_(TlFetchObject<InputStickerSet>::parse(p))
#undef FAIL
{}

void documentAttributeSticker::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(alt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void documentAttributeSticker::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(alt_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(stickerset_, s);
}

void documentAttributeSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeSticker");
    s.store_field("alt", alt_);
    s.store_object_field("stickerset", static_cast<const BaseObject *>(stickerset_.get()));
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

object_ptr<DocumentAttribute> documentAttributeAudio::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<documentAttributeAudio> res = make_tl_object<documentAttributeAudio>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1024) { res->voice_ = TlFetchTrue::parse(p); }
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
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(duration_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreString::store(performer_, s); }
  if (var0 & 4) { TlStoreString::store(waveform_, s); }
}

void documentAttributeAudio::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(duration_, s);
  if (var0 & 1) { TlStoreString::store(title_, s); }
  if (var0 & 2) { TlStoreString::store(performer_, s); }
  if (var0 & 4) { TlStoreString::store(waveform_, s); }
}

void documentAttributeAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeAudio");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("duration", duration_);
    if (var0 & 1) { s.store_field("title", title_); }
    if (var0 & 2) { s.store_field("performer", performer_); }
    if (var0 & 4) { s.store_bytes_field("waveform", waveform_); }
    s.store_class_end();
  }
}

documentAttributeVideo::documentAttributeVideo()
  : flags_()
  , round_message_()
  , duration_()
  , w_()
  , h_()
{}

documentAttributeVideo::documentAttributeVideo(int32 flags_, bool round_message_, int32 duration_, int32 w_, int32 h_)
  : flags_(flags_)
  , round_message_(round_message_)
  , duration_(duration_)
  , w_(w_)
  , h_(h_)
{}

const std::int32_t documentAttributeVideo::ID;

object_ptr<DocumentAttribute> documentAttributeVideo::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  object_ptr<documentAttributeVideo> res = make_tl_object<documentAttributeVideo>();
  int32 var0;
  if ((var0 = res->flags_ = TlFetchInt::parse(p)) < 0) { FAIL("Variable of type # can't be negative"); }
  if (var0 & 1) { res->round_message_ = TlFetchTrue::parse(p); }
  res->duration_ = TlFetchInt::parse(p);
  res->w_ = TlFetchInt::parse(p);
  res->h_ = TlFetchInt::parse(p);
  if (p.get_error()) { FAIL(""); }
  return std::move(res);
#undef FAIL
}

void documentAttributeVideo::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
}

void documentAttributeVideo::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  int32 var0;
  TlStoreBinary::store((var0 = flags_), s);
  TlStoreBinary::store(duration_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
}

void documentAttributeVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "documentAttributeVideo");
  int32 var0;
    s.store_field("flags", (var0 = flags_));
    s.store_field("duration", duration_);
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_class_end();
  }
}

object_ptr<FileLocation> FileLocation::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case fileLocationUnavailable::ID:
      return fileLocationUnavailable::fetch(p);
    case fileLocation::ID:
      return fileLocation::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

fileLocationUnavailable::fileLocationUnavailable(int64 volume_id_, int32 local_id_, int64 secret_)
  : volume_id_(volume_id_)
  , local_id_(local_id_)
  , secret_(secret_)
{}

const std::int32_t fileLocationUnavailable::ID;

object_ptr<FileLocation> fileLocationUnavailable::fetch(TlParser &p) {
  return make_tl_object<fileLocationUnavailable>(p);
}

fileLocationUnavailable::fileLocationUnavailable(TlParser &p)
#define FAIL(error) p.set_error(error)
  : volume_id_(TlFetchLong::parse(p))
  , local_id_(TlFetchInt::parse(p))
  , secret_(TlFetchLong::parse(p))
#undef FAIL
{}

void fileLocationUnavailable::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
}

void fileLocationUnavailable::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
}

void fileLocationUnavailable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileLocationUnavailable");
    s.store_field("volume_id", volume_id_);
    s.store_field("local_id", local_id_);
    s.store_field("secret", secret_);
    s.store_class_end();
  }
}

fileLocation::fileLocation(int32 dc_id_, int64 volume_id_, int32 local_id_, int64 secret_)
  : dc_id_(dc_id_)
  , volume_id_(volume_id_)
  , local_id_(local_id_)
  , secret_(secret_)
{}

const std::int32_t fileLocation::ID;

object_ptr<FileLocation> fileLocation::fetch(TlParser &p) {
  return make_tl_object<fileLocation>(p);
}

fileLocation::fileLocation(TlParser &p)
#define FAIL(error) p.set_error(error)
  : dc_id_(TlFetchInt::parse(p))
  , volume_id_(TlFetchLong::parse(p))
  , local_id_(TlFetchInt::parse(p))
  , secret_(TlFetchLong::parse(p))
#undef FAIL
{}

void fileLocation::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
}

void fileLocation::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(dc_id_, s);
  TlStoreBinary::store(volume_id_, s);
  TlStoreBinary::store(local_id_, s);
  TlStoreBinary::store(secret_, s);
}

void fileLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileLocation");
    s.store_field("dc_id", dc_id_);
    s.store_field("volume_id", volume_id_);
    s.store_field("local_id", local_id_);
    s.store_field("secret", secret_);
    s.store_class_end();
  }
}

object_ptr<InputStickerSet> InputStickerSet::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case inputStickerSetShortName::ID:
      return inputStickerSetShortName::fetch(p);
    case inputStickerSetEmpty::ID:
      return inputStickerSetEmpty::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

inputStickerSetShortName::inputStickerSetShortName(string const &short_name_)
  : short_name_(short_name_)
{}

const std::int32_t inputStickerSetShortName::ID;

object_ptr<InputStickerSet> inputStickerSetShortName::fetch(TlParser &p) {
  return make_tl_object<inputStickerSetShortName>(p);
}

inputStickerSetShortName::inputStickerSetShortName(TlParser &p)
#define FAIL(error) p.set_error(error)
  : short_name_(TlFetchString<string>::parse(p))
#undef FAIL
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

const std::int32_t inputStickerSetEmpty::ID;

object_ptr<InputStickerSet> inputStickerSetEmpty::fetch(TlParser &p) {
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

object_ptr<MessageEntity> MessageEntity::fetch(TlParser &p) {
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
    case messageEntityPhone::ID:
      return messageEntityPhone::fetch(p);
    case messageEntityCashtag::ID:
      return messageEntityCashtag::fetch(p);
    case messageEntityBankCard::ID:
      return messageEntityBankCard::fetch(p);
    case messageEntityUnderline::ID:
      return messageEntityUnderline::fetch(p);
    case messageEntityStrike::ID:
      return messageEntityStrike::fetch(p);
    case messageEntityBlockquote::ID:
      return messageEntityBlockquote::fetch(p);
    case messageEntitySpoiler::ID:
      return messageEntitySpoiler::fetch(p);
    case messageEntityCustomEmoji::ID:
      return messageEntityCustomEmoji::fetch(p);
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

object_ptr<MessageEntity> messageEntityUnknown::fetch(TlParser &p) {
  return make_tl_object<messageEntityUnknown>(p);
}

messageEntityUnknown::messageEntityUnknown(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityMention::fetch(TlParser &p) {
  return make_tl_object<messageEntityMention>(p);
}

messageEntityMention::messageEntityMention(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityHashtag::fetch(TlParser &p) {
  return make_tl_object<messageEntityHashtag>(p);
}

messageEntityHashtag::messageEntityHashtag(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityBotCommand::fetch(TlParser &p) {
  return make_tl_object<messageEntityBotCommand>(p);
}

messageEntityBotCommand::messageEntityBotCommand(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityUrl::fetch(TlParser &p) {
  return make_tl_object<messageEntityUrl>(p);
}

messageEntityUrl::messageEntityUrl(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityEmail::fetch(TlParser &p) {
  return make_tl_object<messageEntityEmail>(p);
}

messageEntityEmail::messageEntityEmail(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityBold::fetch(TlParser &p) {
  return make_tl_object<messageEntityBold>(p);
}

messageEntityBold::messageEntityBold(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityItalic::fetch(TlParser &p) {
  return make_tl_object<messageEntityItalic>(p);
}

messageEntityItalic::messageEntityItalic(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityCode::fetch(TlParser &p) {
  return make_tl_object<messageEntityCode>(p);
}

messageEntityCode::messageEntityCode(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityPre::fetch(TlParser &p) {
  return make_tl_object<messageEntityPre>(p);
}

messageEntityPre::messageEntityPre(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , language_(TlFetchString<string>::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityTextUrl::fetch(TlParser &p) {
  return make_tl_object<messageEntityTextUrl>(p);
}

messageEntityTextUrl::messageEntityTextUrl(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , url_(TlFetchString<string>::parse(p))
#undef FAIL
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

messageEntityMentionName::messageEntityMentionName(int32 offset_, int32 length_, int32 user_id_)
  : offset_(offset_)
  , length_(length_)
  , user_id_(user_id_)
{}

const std::int32_t messageEntityMentionName::ID;

object_ptr<MessageEntity> messageEntityMentionName::fetch(TlParser &p) {
  return make_tl_object<messageEntityMentionName>(p);
}

messageEntityMentionName::messageEntityMentionName(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , user_id_(TlFetchInt::parse(p))
#undef FAIL
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

messageEntityPhone::messageEntityPhone(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityPhone::ID;

object_ptr<MessageEntity> messageEntityPhone::fetch(TlParser &p) {
  return make_tl_object<messageEntityPhone>(p);
}

messageEntityPhone::messageEntityPhone(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityCashtag::fetch(TlParser &p) {
  return make_tl_object<messageEntityCashtag>(p);
}

messageEntityCashtag::messageEntityCashtag(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

messageEntityBankCard::messageEntityBankCard(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityBankCard::ID;

object_ptr<MessageEntity> messageEntityBankCard::fetch(TlParser &p) {
  return make_tl_object<messageEntityBankCard>(p);
}

messageEntityBankCard::messageEntityBankCard(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

messageEntityUnderline::messageEntityUnderline(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityUnderline::ID;

object_ptr<MessageEntity> messageEntityUnderline::fetch(TlParser &p) {
  return make_tl_object<messageEntityUnderline>(p);
}

messageEntityUnderline::messageEntityUnderline(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityStrike::fetch(TlParser &p) {
  return make_tl_object<messageEntityStrike>(p);
}

messageEntityStrike::messageEntityStrike(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

messageEntityBlockquote::messageEntityBlockquote(int32 offset_, int32 length_)
  : offset_(offset_)
  , length_(length_)
{}

const std::int32_t messageEntityBlockquote::ID;

object_ptr<MessageEntity> messageEntityBlockquote::fetch(TlParser &p) {
  return make_tl_object<messageEntityBlockquote>(p);
}

messageEntityBlockquote::messageEntityBlockquote(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
{}

void messageEntityBlockquote::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBlockquote::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreBinary::store(offset_, s);
  TlStoreBinary::store(length_, s);
}

void messageEntityBlockquote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEntityBlockquote");
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

object_ptr<MessageEntity> messageEntitySpoiler::fetch(TlParser &p) {
  return make_tl_object<messageEntitySpoiler>(p);
}

messageEntitySpoiler::messageEntitySpoiler(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
#undef FAIL
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

object_ptr<MessageEntity> messageEntityCustomEmoji::fetch(TlParser &p) {
  return make_tl_object<messageEntityCustomEmoji>(p);
}

messageEntityCustomEmoji::messageEntityCustomEmoji(TlParser &p)
#define FAIL(error) p.set_error(error)
  : offset_(TlFetchInt::parse(p))
  , length_(TlFetchInt::parse(p))
  , document_id_(TlFetchLong::parse(p))
#undef FAIL
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

object_ptr<PhotoSize> PhotoSize::fetch(TlParser &p) {
#define FAIL(error) p.set_error(error); return nullptr;
  int constructor = p.fetch_int();
  switch (constructor) {
    case photoSizeEmpty::ID:
      return photoSizeEmpty::fetch(p);
    case photoSize::ID:
      return photoSize::fetch(p);
    case photoCachedSize::ID:
      return photoCachedSize::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

photoSizeEmpty::photoSizeEmpty(string const &type_)
  : type_(type_)
{}

const std::int32_t photoSizeEmpty::ID;

object_ptr<PhotoSize> photoSizeEmpty::fetch(TlParser &p) {
  return make_tl_object<photoSizeEmpty>(p);
}

photoSizeEmpty::photoSizeEmpty(TlParser &p)
#define FAIL(error) p.set_error(error)
  : type_(TlFetchString<string>::parse(p))
#undef FAIL
{}

void photoSizeEmpty::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(type_, s);
}

void photoSizeEmpty::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(type_, s);
}

void photoSizeEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoSizeEmpty");
    s.store_field("type", type_);
    s.store_class_end();
  }
}

photoSize::photoSize(string const &type_, object_ptr<FileLocation> &&location_, int32 w_, int32 h_, int32 size_)
  : type_(type_)
  , location_(std::move(location_))
  , w_(w_)
  , h_(h_)
  , size_(size_)
{}

const std::int32_t photoSize::ID;

object_ptr<PhotoSize> photoSize::fetch(TlParser &p) {
  return make_tl_object<photoSize>(p);
}

photoSize::photoSize(TlParser &p)
#define FAIL(error) p.set_error(error)
  : type_(TlFetchString<string>::parse(p))
  , location_(TlFetchObject<FileLocation>::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , size_(TlFetchInt::parse(p))
#undef FAIL
{}

void photoSize::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(type_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
}

void photoSize::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(type_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreBinary::store(size_, s);
}

void photoSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoSize");
    s.store_field("type", type_);
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_field("size", size_);
    s.store_class_end();
  }
}

photoCachedSize::photoCachedSize(string const &type_, object_ptr<FileLocation> &&location_, int32 w_, int32 h_, bytes &&bytes_)
  : type_(type_)
  , location_(std::move(location_))
  , w_(w_)
  , h_(h_)
  , bytes_(std::move(bytes_))
{}

const std::int32_t photoCachedSize::ID;

object_ptr<PhotoSize> photoCachedSize::fetch(TlParser &p) {
  return make_tl_object<photoCachedSize>(p);
}

photoCachedSize::photoCachedSize(TlParser &p)
#define FAIL(error) p.set_error(error)
  : type_(TlFetchString<string>::parse(p))
  , location_(TlFetchObject<FileLocation>::parse(p))
  , w_(TlFetchInt::parse(p))
  , h_(TlFetchInt::parse(p))
  , bytes_(TlFetchBytes<bytes>::parse(p))
#undef FAIL
{}

void photoCachedSize::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  TlStoreString::store(type_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreString::store(bytes_, s);
}

void photoCachedSize::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  TlStoreString::store(type_, s);
  TlStoreBoxedUnknown<TlStoreObject>::store(location_, s);
  TlStoreBinary::store(w_, s);
  TlStoreBinary::store(h_, s);
  TlStoreString::store(bytes_, s);
}

void photoCachedSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoCachedSize");
    s.store_field("type", type_);
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("w", w_);
    s.store_field("h", h_);
    s.store_bytes_field("bytes", bytes_);
    s.store_class_end();
  }
}

object_ptr<SendMessageAction> SendMessageAction::fetch(TlParser &p) {
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
    case sendMessageRecordRoundAction::ID:
      return sendMessageRecordRoundAction::fetch(p);
    case sendMessageUploadRoundAction::ID:
      return sendMessageUploadRoundAction::fetch(p);
    default:
      FAIL(PSTRING() << "Unknown constructor found " << format::as_hex(constructor));
  }
#undef FAIL
}

const std::int32_t sendMessageTypingAction::ID;

object_ptr<SendMessageAction> sendMessageTypingAction::fetch(TlParser &p) {
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

object_ptr<SendMessageAction> sendMessageCancelAction::fetch(TlParser &p) {
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

object_ptr<SendMessageAction> sendMessageRecordVideoAction::fetch(TlParser &p) {
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

const std::int32_t sendMessageUploadVideoAction::ID;

object_ptr<SendMessageAction> sendMessageUploadVideoAction::fetch(TlParser &p) {
  return make_tl_object<sendMessageUploadVideoAction>();
}

void sendMessageUploadVideoAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageUploadVideoAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageUploadVideoAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadVideoAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageRecordAudioAction::ID;

object_ptr<SendMessageAction> sendMessageRecordAudioAction::fetch(TlParser &p) {
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

const std::int32_t sendMessageUploadAudioAction::ID;

object_ptr<SendMessageAction> sendMessageUploadAudioAction::fetch(TlParser &p) {
  return make_tl_object<sendMessageUploadAudioAction>();
}

void sendMessageUploadAudioAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageUploadAudioAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageUploadAudioAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadAudioAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageUploadPhotoAction::ID;

object_ptr<SendMessageAction> sendMessageUploadPhotoAction::fetch(TlParser &p) {
  return make_tl_object<sendMessageUploadPhotoAction>();
}

void sendMessageUploadPhotoAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageUploadPhotoAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageUploadPhotoAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadPhotoAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageUploadDocumentAction::ID;

object_ptr<SendMessageAction> sendMessageUploadDocumentAction::fetch(TlParser &p) {
  return make_tl_object<sendMessageUploadDocumentAction>();
}

void sendMessageUploadDocumentAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageUploadDocumentAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageUploadDocumentAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadDocumentAction");
    s.store_class_end();
  }
}

const std::int32_t sendMessageGeoLocationAction::ID;

object_ptr<SendMessageAction> sendMessageGeoLocationAction::fetch(TlParser &p) {
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

object_ptr<SendMessageAction> sendMessageChooseContactAction::fetch(TlParser &p) {
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

const std::int32_t sendMessageRecordRoundAction::ID;

object_ptr<SendMessageAction> sendMessageRecordRoundAction::fetch(TlParser &p) {
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

const std::int32_t sendMessageUploadRoundAction::ID;

object_ptr<SendMessageAction> sendMessageUploadRoundAction::fetch(TlParser &p) {
  return make_tl_object<sendMessageUploadRoundAction>();
}

void sendMessageUploadRoundAction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
}

void sendMessageUploadRoundAction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
}

void sendMessageUploadRoundAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageUploadRoundAction");
    s.store_class_end();
  }
}

const std::int32_t test_dummyFunction::ID;

object_ptr<test_dummyFunction> test_dummyFunction::fetch(TlParser &p) {
  return make_tl_object<test_dummyFunction>();
}

void test_dummyFunction::store(TlStorerCalcLength &s) const {
  (void)sizeof(s);
  s.store_binary(-936020215);
}

void test_dummyFunction::store(TlStorerUnsafe &s) const {
  (void)sizeof(s);
  s.store_binary(-936020215);
}

void test_dummyFunction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "test.dummyFunction");
    s.store_class_end();
  }
}

test_dummyFunction::ReturnType test_dummyFunction::fetch_result(TlParser &p) {
#define FAIL(error) p.set_error(error); return ReturnType()
  return TlFetchBool::parse(p);
#undef FAIL
}
}  // namespace secret_api
}  // namespace td
