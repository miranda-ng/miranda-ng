#pragma once

/**
 * \file
 * Contains downcast_call methods for calling a function object on downcasted to
 * the most derived class TDLib API object.
 */
#include "secret_api.h"

namespace td {
namespace secret_api {

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Object &obj, const T &func) {
  switch (obj.get_id()) {
    case decryptedMessage8::ID:
      func(static_cast<decryptedMessage8 &>(obj));
      return true;
    case decryptedMessageService8::ID:
      func(static_cast<decryptedMessageService8 &>(obj));
      return true;
    case decryptedMessage23::ID:
      func(static_cast<decryptedMessage23 &>(obj));
      return true;
    case decryptedMessageService::ID:
      func(static_cast<decryptedMessageService &>(obj));
      return true;
    case decryptedMessage46::ID:
      func(static_cast<decryptedMessage46 &>(obj));
      return true;
    case decryptedMessage::ID:
      func(static_cast<decryptedMessage &>(obj));
      return true;
    case decryptedMessageActionSetMessageTTL::ID:
      func(static_cast<decryptedMessageActionSetMessageTTL &>(obj));
      return true;
    case decryptedMessageActionReadMessages::ID:
      func(static_cast<decryptedMessageActionReadMessages &>(obj));
      return true;
    case decryptedMessageActionDeleteMessages::ID:
      func(static_cast<decryptedMessageActionDeleteMessages &>(obj));
      return true;
    case decryptedMessageActionScreenshotMessages::ID:
      func(static_cast<decryptedMessageActionScreenshotMessages &>(obj));
      return true;
    case decryptedMessageActionFlushHistory::ID:
      func(static_cast<decryptedMessageActionFlushHistory &>(obj));
      return true;
    case decryptedMessageActionResend::ID:
      func(static_cast<decryptedMessageActionResend &>(obj));
      return true;
    case decryptedMessageActionNotifyLayer::ID:
      func(static_cast<decryptedMessageActionNotifyLayer &>(obj));
      return true;
    case decryptedMessageActionTyping::ID:
      func(static_cast<decryptedMessageActionTyping &>(obj));
      return true;
    case decryptedMessageActionRequestKey::ID:
      func(static_cast<decryptedMessageActionRequestKey &>(obj));
      return true;
    case decryptedMessageActionAcceptKey::ID:
      func(static_cast<decryptedMessageActionAcceptKey &>(obj));
      return true;
    case decryptedMessageActionAbortKey::ID:
      func(static_cast<decryptedMessageActionAbortKey &>(obj));
      return true;
    case decryptedMessageActionCommitKey::ID:
      func(static_cast<decryptedMessageActionCommitKey &>(obj));
      return true;
    case decryptedMessageActionNoop::ID:
      func(static_cast<decryptedMessageActionNoop &>(obj));
      return true;
    case decryptedMessageLayer::ID:
      func(static_cast<decryptedMessageLayer &>(obj));
      return true;
    case decryptedMessageMediaEmpty::ID:
      func(static_cast<decryptedMessageMediaEmpty &>(obj));
      return true;
    case decryptedMessageMediaPhoto8::ID:
      func(static_cast<decryptedMessageMediaPhoto8 &>(obj));
      return true;
    case decryptedMessageMediaVideo8::ID:
      func(static_cast<decryptedMessageMediaVideo8 &>(obj));
      return true;
    case decryptedMessageMediaGeoPoint::ID:
      func(static_cast<decryptedMessageMediaGeoPoint &>(obj));
      return true;
    case decryptedMessageMediaContact::ID:
      func(static_cast<decryptedMessageMediaContact &>(obj));
      return true;
    case decryptedMessageMediaDocument8::ID:
      func(static_cast<decryptedMessageMediaDocument8 &>(obj));
      return true;
    case decryptedMessageMediaAudio8::ID:
      func(static_cast<decryptedMessageMediaAudio8 &>(obj));
      return true;
    case decryptedMessageMediaVideo23::ID:
      func(static_cast<decryptedMessageMediaVideo23 &>(obj));
      return true;
    case decryptedMessageMediaAudio::ID:
      func(static_cast<decryptedMessageMediaAudio &>(obj));
      return true;
    case decryptedMessageMediaExternalDocument::ID:
      func(static_cast<decryptedMessageMediaExternalDocument &>(obj));
      return true;
    case decryptedMessageMediaPhoto::ID:
      func(static_cast<decryptedMessageMediaPhoto &>(obj));
      return true;
    case decryptedMessageMediaVideo::ID:
      func(static_cast<decryptedMessageMediaVideo &>(obj));
      return true;
    case decryptedMessageMediaDocument46::ID:
      func(static_cast<decryptedMessageMediaDocument46 &>(obj));
      return true;
    case decryptedMessageMediaVenue::ID:
      func(static_cast<decryptedMessageMediaVenue &>(obj));
      return true;
    case decryptedMessageMediaWebPage::ID:
      func(static_cast<decryptedMessageMediaWebPage &>(obj));
      return true;
    case decryptedMessageMediaDocument::ID:
      func(static_cast<decryptedMessageMediaDocument &>(obj));
      return true;
    case documentAttributeImageSize::ID:
      func(static_cast<documentAttributeImageSize &>(obj));
      return true;
    case documentAttributeAnimated::ID:
      func(static_cast<documentAttributeAnimated &>(obj));
      return true;
    case documentAttributeSticker23::ID:
      func(static_cast<documentAttributeSticker23 &>(obj));
      return true;
    case documentAttributeVideo23::ID:
      func(static_cast<documentAttributeVideo23 &>(obj));
      return true;
    case documentAttributeAudio23::ID:
      func(static_cast<documentAttributeAudio23 &>(obj));
      return true;
    case documentAttributeFilename::ID:
      func(static_cast<documentAttributeFilename &>(obj));
      return true;
    case documentAttributeAudio45::ID:
      func(static_cast<documentAttributeAudio45 &>(obj));
      return true;
    case documentAttributeSticker::ID:
      func(static_cast<documentAttributeSticker &>(obj));
      return true;
    case documentAttributeAudio::ID:
      func(static_cast<documentAttributeAudio &>(obj));
      return true;
    case documentAttributeVideo::ID:
      func(static_cast<documentAttributeVideo &>(obj));
      return true;
    case fileLocationUnavailable::ID:
      func(static_cast<fileLocationUnavailable &>(obj));
      return true;
    case fileLocation::ID:
      func(static_cast<fileLocation &>(obj));
      return true;
    case inputStickerSetShortName::ID:
      func(static_cast<inputStickerSetShortName &>(obj));
      return true;
    case inputStickerSetEmpty::ID:
      func(static_cast<inputStickerSetEmpty &>(obj));
      return true;
    case messageEntityUnknown::ID:
      func(static_cast<messageEntityUnknown &>(obj));
      return true;
    case messageEntityMention::ID:
      func(static_cast<messageEntityMention &>(obj));
      return true;
    case messageEntityHashtag::ID:
      func(static_cast<messageEntityHashtag &>(obj));
      return true;
    case messageEntityBotCommand::ID:
      func(static_cast<messageEntityBotCommand &>(obj));
      return true;
    case messageEntityUrl::ID:
      func(static_cast<messageEntityUrl &>(obj));
      return true;
    case messageEntityEmail::ID:
      func(static_cast<messageEntityEmail &>(obj));
      return true;
    case messageEntityBold::ID:
      func(static_cast<messageEntityBold &>(obj));
      return true;
    case messageEntityItalic::ID:
      func(static_cast<messageEntityItalic &>(obj));
      return true;
    case messageEntityCode::ID:
      func(static_cast<messageEntityCode &>(obj));
      return true;
    case messageEntityPre::ID:
      func(static_cast<messageEntityPre &>(obj));
      return true;
    case messageEntityTextUrl::ID:
      func(static_cast<messageEntityTextUrl &>(obj));
      return true;
    case messageEntityMentionName::ID:
      func(static_cast<messageEntityMentionName &>(obj));
      return true;
    case messageEntityPhone::ID:
      func(static_cast<messageEntityPhone &>(obj));
      return true;
    case messageEntityCashtag::ID:
      func(static_cast<messageEntityCashtag &>(obj));
      return true;
    case messageEntityBankCard::ID:
      func(static_cast<messageEntityBankCard &>(obj));
      return true;
    case messageEntityUnderline::ID:
      func(static_cast<messageEntityUnderline &>(obj));
      return true;
    case messageEntityStrike::ID:
      func(static_cast<messageEntityStrike &>(obj));
      return true;
    case messageEntityBlockquote::ID:
      func(static_cast<messageEntityBlockquote &>(obj));
      return true;
    case messageEntitySpoiler::ID:
      func(static_cast<messageEntitySpoiler &>(obj));
      return true;
    case messageEntityCustomEmoji::ID:
      func(static_cast<messageEntityCustomEmoji &>(obj));
      return true;
    case photoSizeEmpty::ID:
      func(static_cast<photoSizeEmpty &>(obj));
      return true;
    case photoSize::ID:
      func(static_cast<photoSize &>(obj));
      return true;
    case photoCachedSize::ID:
      func(static_cast<photoCachedSize &>(obj));
      return true;
    case sendMessageTypingAction::ID:
      func(static_cast<sendMessageTypingAction &>(obj));
      return true;
    case sendMessageCancelAction::ID:
      func(static_cast<sendMessageCancelAction &>(obj));
      return true;
    case sendMessageRecordVideoAction::ID:
      func(static_cast<sendMessageRecordVideoAction &>(obj));
      return true;
    case sendMessageUploadVideoAction::ID:
      func(static_cast<sendMessageUploadVideoAction &>(obj));
      return true;
    case sendMessageRecordAudioAction::ID:
      func(static_cast<sendMessageRecordAudioAction &>(obj));
      return true;
    case sendMessageUploadAudioAction::ID:
      func(static_cast<sendMessageUploadAudioAction &>(obj));
      return true;
    case sendMessageUploadPhotoAction::ID:
      func(static_cast<sendMessageUploadPhotoAction &>(obj));
      return true;
    case sendMessageUploadDocumentAction::ID:
      func(static_cast<sendMessageUploadDocumentAction &>(obj));
      return true;
    case sendMessageGeoLocationAction::ID:
      func(static_cast<sendMessageGeoLocationAction &>(obj));
      return true;
    case sendMessageChooseContactAction::ID:
      func(static_cast<sendMessageChooseContactAction &>(obj));
      return true;
    case sendMessageRecordRoundAction::ID:
      func(static_cast<sendMessageRecordRoundAction &>(obj));
      return true;
    case sendMessageUploadRoundAction::ID:
      func(static_cast<sendMessageUploadRoundAction &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Function &obj, const T &func) {
  switch (obj.get_id()) {
    case test_dummyFunction::ID:
      func(static_cast<test_dummyFunction &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DecryptedMessage &obj, const T &func) {
  switch (obj.get_id()) {
    case decryptedMessage8::ID:
      func(static_cast<decryptedMessage8 &>(obj));
      return true;
    case decryptedMessageService8::ID:
      func(static_cast<decryptedMessageService8 &>(obj));
      return true;
    case decryptedMessage23::ID:
      func(static_cast<decryptedMessage23 &>(obj));
      return true;
    case decryptedMessageService::ID:
      func(static_cast<decryptedMessageService &>(obj));
      return true;
    case decryptedMessage46::ID:
      func(static_cast<decryptedMessage46 &>(obj));
      return true;
    case decryptedMessage::ID:
      func(static_cast<decryptedMessage &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DecryptedMessageAction &obj, const T &func) {
  switch (obj.get_id()) {
    case decryptedMessageActionSetMessageTTL::ID:
      func(static_cast<decryptedMessageActionSetMessageTTL &>(obj));
      return true;
    case decryptedMessageActionReadMessages::ID:
      func(static_cast<decryptedMessageActionReadMessages &>(obj));
      return true;
    case decryptedMessageActionDeleteMessages::ID:
      func(static_cast<decryptedMessageActionDeleteMessages &>(obj));
      return true;
    case decryptedMessageActionScreenshotMessages::ID:
      func(static_cast<decryptedMessageActionScreenshotMessages &>(obj));
      return true;
    case decryptedMessageActionFlushHistory::ID:
      func(static_cast<decryptedMessageActionFlushHistory &>(obj));
      return true;
    case decryptedMessageActionResend::ID:
      func(static_cast<decryptedMessageActionResend &>(obj));
      return true;
    case decryptedMessageActionNotifyLayer::ID:
      func(static_cast<decryptedMessageActionNotifyLayer &>(obj));
      return true;
    case decryptedMessageActionTyping::ID:
      func(static_cast<decryptedMessageActionTyping &>(obj));
      return true;
    case decryptedMessageActionRequestKey::ID:
      func(static_cast<decryptedMessageActionRequestKey &>(obj));
      return true;
    case decryptedMessageActionAcceptKey::ID:
      func(static_cast<decryptedMessageActionAcceptKey &>(obj));
      return true;
    case decryptedMessageActionAbortKey::ID:
      func(static_cast<decryptedMessageActionAbortKey &>(obj));
      return true;
    case decryptedMessageActionCommitKey::ID:
      func(static_cast<decryptedMessageActionCommitKey &>(obj));
      return true;
    case decryptedMessageActionNoop::ID:
      func(static_cast<decryptedMessageActionNoop &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DecryptedMessageMedia &obj, const T &func) {
  switch (obj.get_id()) {
    case decryptedMessageMediaEmpty::ID:
      func(static_cast<decryptedMessageMediaEmpty &>(obj));
      return true;
    case decryptedMessageMediaPhoto8::ID:
      func(static_cast<decryptedMessageMediaPhoto8 &>(obj));
      return true;
    case decryptedMessageMediaVideo8::ID:
      func(static_cast<decryptedMessageMediaVideo8 &>(obj));
      return true;
    case decryptedMessageMediaGeoPoint::ID:
      func(static_cast<decryptedMessageMediaGeoPoint &>(obj));
      return true;
    case decryptedMessageMediaContact::ID:
      func(static_cast<decryptedMessageMediaContact &>(obj));
      return true;
    case decryptedMessageMediaDocument8::ID:
      func(static_cast<decryptedMessageMediaDocument8 &>(obj));
      return true;
    case decryptedMessageMediaAudio8::ID:
      func(static_cast<decryptedMessageMediaAudio8 &>(obj));
      return true;
    case decryptedMessageMediaVideo23::ID:
      func(static_cast<decryptedMessageMediaVideo23 &>(obj));
      return true;
    case decryptedMessageMediaAudio::ID:
      func(static_cast<decryptedMessageMediaAudio &>(obj));
      return true;
    case decryptedMessageMediaExternalDocument::ID:
      func(static_cast<decryptedMessageMediaExternalDocument &>(obj));
      return true;
    case decryptedMessageMediaPhoto::ID:
      func(static_cast<decryptedMessageMediaPhoto &>(obj));
      return true;
    case decryptedMessageMediaVideo::ID:
      func(static_cast<decryptedMessageMediaVideo &>(obj));
      return true;
    case decryptedMessageMediaDocument46::ID:
      func(static_cast<decryptedMessageMediaDocument46 &>(obj));
      return true;
    case decryptedMessageMediaVenue::ID:
      func(static_cast<decryptedMessageMediaVenue &>(obj));
      return true;
    case decryptedMessageMediaWebPage::ID:
      func(static_cast<decryptedMessageMediaWebPage &>(obj));
      return true;
    case decryptedMessageMediaDocument::ID:
      func(static_cast<decryptedMessageMediaDocument &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DocumentAttribute &obj, const T &func) {
  switch (obj.get_id()) {
    case documentAttributeImageSize::ID:
      func(static_cast<documentAttributeImageSize &>(obj));
      return true;
    case documentAttributeAnimated::ID:
      func(static_cast<documentAttributeAnimated &>(obj));
      return true;
    case documentAttributeSticker23::ID:
      func(static_cast<documentAttributeSticker23 &>(obj));
      return true;
    case documentAttributeVideo23::ID:
      func(static_cast<documentAttributeVideo23 &>(obj));
      return true;
    case documentAttributeAudio23::ID:
      func(static_cast<documentAttributeAudio23 &>(obj));
      return true;
    case documentAttributeFilename::ID:
      func(static_cast<documentAttributeFilename &>(obj));
      return true;
    case documentAttributeAudio45::ID:
      func(static_cast<documentAttributeAudio45 &>(obj));
      return true;
    case documentAttributeSticker::ID:
      func(static_cast<documentAttributeSticker &>(obj));
      return true;
    case documentAttributeAudio::ID:
      func(static_cast<documentAttributeAudio &>(obj));
      return true;
    case documentAttributeVideo::ID:
      func(static_cast<documentAttributeVideo &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(FileLocation &obj, const T &func) {
  switch (obj.get_id()) {
    case fileLocationUnavailable::ID:
      func(static_cast<fileLocationUnavailable &>(obj));
      return true;
    case fileLocation::ID:
      func(static_cast<fileLocation &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputStickerSet &obj, const T &func) {
  switch (obj.get_id()) {
    case inputStickerSetShortName::ID:
      func(static_cast<inputStickerSetShortName &>(obj));
      return true;
    case inputStickerSetEmpty::ID:
      func(static_cast<inputStickerSetEmpty &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(MessageEntity &obj, const T &func) {
  switch (obj.get_id()) {
    case messageEntityUnknown::ID:
      func(static_cast<messageEntityUnknown &>(obj));
      return true;
    case messageEntityMention::ID:
      func(static_cast<messageEntityMention &>(obj));
      return true;
    case messageEntityHashtag::ID:
      func(static_cast<messageEntityHashtag &>(obj));
      return true;
    case messageEntityBotCommand::ID:
      func(static_cast<messageEntityBotCommand &>(obj));
      return true;
    case messageEntityUrl::ID:
      func(static_cast<messageEntityUrl &>(obj));
      return true;
    case messageEntityEmail::ID:
      func(static_cast<messageEntityEmail &>(obj));
      return true;
    case messageEntityBold::ID:
      func(static_cast<messageEntityBold &>(obj));
      return true;
    case messageEntityItalic::ID:
      func(static_cast<messageEntityItalic &>(obj));
      return true;
    case messageEntityCode::ID:
      func(static_cast<messageEntityCode &>(obj));
      return true;
    case messageEntityPre::ID:
      func(static_cast<messageEntityPre &>(obj));
      return true;
    case messageEntityTextUrl::ID:
      func(static_cast<messageEntityTextUrl &>(obj));
      return true;
    case messageEntityMentionName::ID:
      func(static_cast<messageEntityMentionName &>(obj));
      return true;
    case messageEntityPhone::ID:
      func(static_cast<messageEntityPhone &>(obj));
      return true;
    case messageEntityCashtag::ID:
      func(static_cast<messageEntityCashtag &>(obj));
      return true;
    case messageEntityBankCard::ID:
      func(static_cast<messageEntityBankCard &>(obj));
      return true;
    case messageEntityUnderline::ID:
      func(static_cast<messageEntityUnderline &>(obj));
      return true;
    case messageEntityStrike::ID:
      func(static_cast<messageEntityStrike &>(obj));
      return true;
    case messageEntityBlockquote::ID:
      func(static_cast<messageEntityBlockquote &>(obj));
      return true;
    case messageEntitySpoiler::ID:
      func(static_cast<messageEntitySpoiler &>(obj));
      return true;
    case messageEntityCustomEmoji::ID:
      func(static_cast<messageEntityCustomEmoji &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PhotoSize &obj, const T &func) {
  switch (obj.get_id()) {
    case photoSizeEmpty::ID:
      func(static_cast<photoSizeEmpty &>(obj));
      return true;
    case photoSize::ID:
      func(static_cast<photoSize &>(obj));
      return true;
    case photoCachedSize::ID:
      func(static_cast<photoCachedSize &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SendMessageAction &obj, const T &func) {
  switch (obj.get_id()) {
    case sendMessageTypingAction::ID:
      func(static_cast<sendMessageTypingAction &>(obj));
      return true;
    case sendMessageCancelAction::ID:
      func(static_cast<sendMessageCancelAction &>(obj));
      return true;
    case sendMessageRecordVideoAction::ID:
      func(static_cast<sendMessageRecordVideoAction &>(obj));
      return true;
    case sendMessageUploadVideoAction::ID:
      func(static_cast<sendMessageUploadVideoAction &>(obj));
      return true;
    case sendMessageRecordAudioAction::ID:
      func(static_cast<sendMessageRecordAudioAction &>(obj));
      return true;
    case sendMessageUploadAudioAction::ID:
      func(static_cast<sendMessageUploadAudioAction &>(obj));
      return true;
    case sendMessageUploadPhotoAction::ID:
      func(static_cast<sendMessageUploadPhotoAction &>(obj));
      return true;
    case sendMessageUploadDocumentAction::ID:
      func(static_cast<sendMessageUploadDocumentAction &>(obj));
      return true;
    case sendMessageGeoLocationAction::ID:
      func(static_cast<sendMessageGeoLocationAction &>(obj));
      return true;
    case sendMessageChooseContactAction::ID:
      func(static_cast<sendMessageChooseContactAction &>(obj));
      return true;
    case sendMessageRecordRoundAction::ID:
      func(static_cast<sendMessageRecordRoundAction &>(obj));
      return true;
    case sendMessageUploadRoundAction::ID:
      func(static_cast<sendMessageUploadRoundAction &>(obj));
      return true;
    default:
      return false;
  }
}

}  // namespace secret_api
}  // namespace td
