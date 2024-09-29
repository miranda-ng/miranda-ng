#pragma once

/**
 * \file
 * Contains downcast_call methods for calling a function object on downcasted to
 * the most derived class TDLib API object.
 */
#include "td_api.h"

namespace td {
namespace td_api {

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Object &obj, const T &func) {
  switch (obj.get_id()) {
    case accentColor::ID:
      func(static_cast<accentColor &>(obj));
      return true;
    case accountTtl::ID:
      func(static_cast<accountTtl &>(obj));
      return true;
    case addedReaction::ID:
      func(static_cast<addedReaction &>(obj));
      return true;
    case addedReactions::ID:
      func(static_cast<addedReactions &>(obj));
      return true;
    case address::ID:
      func(static_cast<address &>(obj));
      return true;
    case animatedChatPhoto::ID:
      func(static_cast<animatedChatPhoto &>(obj));
      return true;
    case animatedEmoji::ID:
      func(static_cast<animatedEmoji &>(obj));
      return true;
    case animation::ID:
      func(static_cast<animation &>(obj));
      return true;
    case animations::ID:
      func(static_cast<animations &>(obj));
      return true;
    case archiveChatListSettings::ID:
      func(static_cast<archiveChatListSettings &>(obj));
      return true;
    case attachmentMenuBot::ID:
      func(static_cast<attachmentMenuBot &>(obj));
      return true;
    case attachmentMenuBotColor::ID:
      func(static_cast<attachmentMenuBotColor &>(obj));
      return true;
    case audio::ID:
      func(static_cast<audio &>(obj));
      return true;
    case authenticationCodeInfo::ID:
      func(static_cast<authenticationCodeInfo &>(obj));
      return true;
    case authenticationCodeTypeTelegramMessage::ID:
      func(static_cast<authenticationCodeTypeTelegramMessage &>(obj));
      return true;
    case authenticationCodeTypeSms::ID:
      func(static_cast<authenticationCodeTypeSms &>(obj));
      return true;
    case authenticationCodeTypeSmsWord::ID:
      func(static_cast<authenticationCodeTypeSmsWord &>(obj));
      return true;
    case authenticationCodeTypeSmsPhrase::ID:
      func(static_cast<authenticationCodeTypeSmsPhrase &>(obj));
      return true;
    case authenticationCodeTypeCall::ID:
      func(static_cast<authenticationCodeTypeCall &>(obj));
      return true;
    case authenticationCodeTypeFlashCall::ID:
      func(static_cast<authenticationCodeTypeFlashCall &>(obj));
      return true;
    case authenticationCodeTypeMissedCall::ID:
      func(static_cast<authenticationCodeTypeMissedCall &>(obj));
      return true;
    case authenticationCodeTypeFragment::ID:
      func(static_cast<authenticationCodeTypeFragment &>(obj));
      return true;
    case authenticationCodeTypeFirebaseAndroid::ID:
      func(static_cast<authenticationCodeTypeFirebaseAndroid &>(obj));
      return true;
    case authenticationCodeTypeFirebaseIos::ID:
      func(static_cast<authenticationCodeTypeFirebaseIos &>(obj));
      return true;
    case authorizationStateWaitTdlibParameters::ID:
      func(static_cast<authorizationStateWaitTdlibParameters &>(obj));
      return true;
    case authorizationStateWaitPhoneNumber::ID:
      func(static_cast<authorizationStateWaitPhoneNumber &>(obj));
      return true;
    case authorizationStateWaitEmailAddress::ID:
      func(static_cast<authorizationStateWaitEmailAddress &>(obj));
      return true;
    case authorizationStateWaitEmailCode::ID:
      func(static_cast<authorizationStateWaitEmailCode &>(obj));
      return true;
    case authorizationStateWaitCode::ID:
      func(static_cast<authorizationStateWaitCode &>(obj));
      return true;
    case authorizationStateWaitOtherDeviceConfirmation::ID:
      func(static_cast<authorizationStateWaitOtherDeviceConfirmation &>(obj));
      return true;
    case authorizationStateWaitRegistration::ID:
      func(static_cast<authorizationStateWaitRegistration &>(obj));
      return true;
    case authorizationStateWaitPassword::ID:
      func(static_cast<authorizationStateWaitPassword &>(obj));
      return true;
    case authorizationStateReady::ID:
      func(static_cast<authorizationStateReady &>(obj));
      return true;
    case authorizationStateLoggingOut::ID:
      func(static_cast<authorizationStateLoggingOut &>(obj));
      return true;
    case authorizationStateClosing::ID:
      func(static_cast<authorizationStateClosing &>(obj));
      return true;
    case authorizationStateClosed::ID:
      func(static_cast<authorizationStateClosed &>(obj));
      return true;
    case autoDownloadSettings::ID:
      func(static_cast<autoDownloadSettings &>(obj));
      return true;
    case autoDownloadSettingsPresets::ID:
      func(static_cast<autoDownloadSettingsPresets &>(obj));
      return true;
    case autosaveSettings::ID:
      func(static_cast<autosaveSettings &>(obj));
      return true;
    case autosaveSettingsException::ID:
      func(static_cast<autosaveSettingsException &>(obj));
      return true;
    case autosaveSettingsScopePrivateChats::ID:
      func(static_cast<autosaveSettingsScopePrivateChats &>(obj));
      return true;
    case autosaveSettingsScopeGroupChats::ID:
      func(static_cast<autosaveSettingsScopeGroupChats &>(obj));
      return true;
    case autosaveSettingsScopeChannelChats::ID:
      func(static_cast<autosaveSettingsScopeChannelChats &>(obj));
      return true;
    case autosaveSettingsScopeChat::ID:
      func(static_cast<autosaveSettingsScopeChat &>(obj));
      return true;
    case availableReaction::ID:
      func(static_cast<availableReaction &>(obj));
      return true;
    case availableReactions::ID:
      func(static_cast<availableReactions &>(obj));
      return true;
    case background::ID:
      func(static_cast<background &>(obj));
      return true;
    case backgroundFillSolid::ID:
      func(static_cast<backgroundFillSolid &>(obj));
      return true;
    case backgroundFillGradient::ID:
      func(static_cast<backgroundFillGradient &>(obj));
      return true;
    case backgroundFillFreeformGradient::ID:
      func(static_cast<backgroundFillFreeformGradient &>(obj));
      return true;
    case backgroundTypeWallpaper::ID:
      func(static_cast<backgroundTypeWallpaper &>(obj));
      return true;
    case backgroundTypePattern::ID:
      func(static_cast<backgroundTypePattern &>(obj));
      return true;
    case backgroundTypeFill::ID:
      func(static_cast<backgroundTypeFill &>(obj));
      return true;
    case backgroundTypeChatTheme::ID:
      func(static_cast<backgroundTypeChatTheme &>(obj));
      return true;
    case backgrounds::ID:
      func(static_cast<backgrounds &>(obj));
      return true;
    case bankCardActionOpenUrl::ID:
      func(static_cast<bankCardActionOpenUrl &>(obj));
      return true;
    case bankCardInfo::ID:
      func(static_cast<bankCardInfo &>(obj));
      return true;
    case basicGroup::ID:
      func(static_cast<basicGroup &>(obj));
      return true;
    case basicGroupFullInfo::ID:
      func(static_cast<basicGroupFullInfo &>(obj));
      return true;
    case birthdate::ID:
      func(static_cast<birthdate &>(obj));
      return true;
    case blockListMain::ID:
      func(static_cast<blockListMain &>(obj));
      return true;
    case blockListStories::ID:
      func(static_cast<blockListStories &>(obj));
      return true;
    case botCommand::ID:
      func(static_cast<botCommand &>(obj));
      return true;
    case botCommandScopeDefault::ID:
      func(static_cast<botCommandScopeDefault &>(obj));
      return true;
    case botCommandScopeAllPrivateChats::ID:
      func(static_cast<botCommandScopeAllPrivateChats &>(obj));
      return true;
    case botCommandScopeAllGroupChats::ID:
      func(static_cast<botCommandScopeAllGroupChats &>(obj));
      return true;
    case botCommandScopeAllChatAdministrators::ID:
      func(static_cast<botCommandScopeAllChatAdministrators &>(obj));
      return true;
    case botCommandScopeChat::ID:
      func(static_cast<botCommandScopeChat &>(obj));
      return true;
    case botCommandScopeChatAdministrators::ID:
      func(static_cast<botCommandScopeChatAdministrators &>(obj));
      return true;
    case botCommandScopeChatMember::ID:
      func(static_cast<botCommandScopeChatMember &>(obj));
      return true;
    case botCommands::ID:
      func(static_cast<botCommands &>(obj));
      return true;
    case botInfo::ID:
      func(static_cast<botInfo &>(obj));
      return true;
    case botMediaPreview::ID:
      func(static_cast<botMediaPreview &>(obj));
      return true;
    case botMediaPreviewInfo::ID:
      func(static_cast<botMediaPreviewInfo &>(obj));
      return true;
    case botMediaPreviews::ID:
      func(static_cast<botMediaPreviews &>(obj));
      return true;
    case botMenuButton::ID:
      func(static_cast<botMenuButton &>(obj));
      return true;
    case botTransactionPurposePaidMedia::ID:
      func(static_cast<botTransactionPurposePaidMedia &>(obj));
      return true;
    case botTransactionPurposeInvoicePayment::ID:
      func(static_cast<botTransactionPurposeInvoicePayment &>(obj));
      return true;
    case botWriteAccessAllowReasonConnectedWebsite::ID:
      func(static_cast<botWriteAccessAllowReasonConnectedWebsite &>(obj));
      return true;
    case botWriteAccessAllowReasonAddedToAttachmentMenu::ID:
      func(static_cast<botWriteAccessAllowReasonAddedToAttachmentMenu &>(obj));
      return true;
    case botWriteAccessAllowReasonLaunchedWebApp::ID:
      func(static_cast<botWriteAccessAllowReasonLaunchedWebApp &>(obj));
      return true;
    case botWriteAccessAllowReasonAcceptedRequest::ID:
      func(static_cast<botWriteAccessAllowReasonAcceptedRequest &>(obj));
      return true;
    case businessAwayMessageScheduleAlways::ID:
      func(static_cast<businessAwayMessageScheduleAlways &>(obj));
      return true;
    case businessAwayMessageScheduleOutsideOfOpeningHours::ID:
      func(static_cast<businessAwayMessageScheduleOutsideOfOpeningHours &>(obj));
      return true;
    case businessAwayMessageScheduleCustom::ID:
      func(static_cast<businessAwayMessageScheduleCustom &>(obj));
      return true;
    case businessAwayMessageSettings::ID:
      func(static_cast<businessAwayMessageSettings &>(obj));
      return true;
    case businessBotManageBar::ID:
      func(static_cast<businessBotManageBar &>(obj));
      return true;
    case businessChatLink::ID:
      func(static_cast<businessChatLink &>(obj));
      return true;
    case businessChatLinkInfo::ID:
      func(static_cast<businessChatLinkInfo &>(obj));
      return true;
    case businessChatLinks::ID:
      func(static_cast<businessChatLinks &>(obj));
      return true;
    case businessConnectedBot::ID:
      func(static_cast<businessConnectedBot &>(obj));
      return true;
    case businessConnection::ID:
      func(static_cast<businessConnection &>(obj));
      return true;
    case businessFeatureLocation::ID:
      func(static_cast<businessFeatureLocation &>(obj));
      return true;
    case businessFeatureOpeningHours::ID:
      func(static_cast<businessFeatureOpeningHours &>(obj));
      return true;
    case businessFeatureQuickReplies::ID:
      func(static_cast<businessFeatureQuickReplies &>(obj));
      return true;
    case businessFeatureGreetingMessage::ID:
      func(static_cast<businessFeatureGreetingMessage &>(obj));
      return true;
    case businessFeatureAwayMessage::ID:
      func(static_cast<businessFeatureAwayMessage &>(obj));
      return true;
    case businessFeatureAccountLinks::ID:
      func(static_cast<businessFeatureAccountLinks &>(obj));
      return true;
    case businessFeatureStartPage::ID:
      func(static_cast<businessFeatureStartPage &>(obj));
      return true;
    case businessFeatureBots::ID:
      func(static_cast<businessFeatureBots &>(obj));
      return true;
    case businessFeatureEmojiStatus::ID:
      func(static_cast<businessFeatureEmojiStatus &>(obj));
      return true;
    case businessFeatureChatFolderTags::ID:
      func(static_cast<businessFeatureChatFolderTags &>(obj));
      return true;
    case businessFeatureUpgradedStories::ID:
      func(static_cast<businessFeatureUpgradedStories &>(obj));
      return true;
    case businessFeaturePromotionAnimation::ID:
      func(static_cast<businessFeaturePromotionAnimation &>(obj));
      return true;
    case businessFeatures::ID:
      func(static_cast<businessFeatures &>(obj));
      return true;
    case businessGreetingMessageSettings::ID:
      func(static_cast<businessGreetingMessageSettings &>(obj));
      return true;
    case businessInfo::ID:
      func(static_cast<businessInfo &>(obj));
      return true;
    case businessLocation::ID:
      func(static_cast<businessLocation &>(obj));
      return true;
    case businessMessage::ID:
      func(static_cast<businessMessage &>(obj));
      return true;
    case businessMessages::ID:
      func(static_cast<businessMessages &>(obj));
      return true;
    case businessOpeningHours::ID:
      func(static_cast<businessOpeningHours &>(obj));
      return true;
    case businessOpeningHoursInterval::ID:
      func(static_cast<businessOpeningHoursInterval &>(obj));
      return true;
    case businessRecipients::ID:
      func(static_cast<businessRecipients &>(obj));
      return true;
    case businessStartPage::ID:
      func(static_cast<businessStartPage &>(obj));
      return true;
    case call::ID:
      func(static_cast<call &>(obj));
      return true;
    case callDiscardReasonEmpty::ID:
      func(static_cast<callDiscardReasonEmpty &>(obj));
      return true;
    case callDiscardReasonMissed::ID:
      func(static_cast<callDiscardReasonMissed &>(obj));
      return true;
    case callDiscardReasonDeclined::ID:
      func(static_cast<callDiscardReasonDeclined &>(obj));
      return true;
    case callDiscardReasonDisconnected::ID:
      func(static_cast<callDiscardReasonDisconnected &>(obj));
      return true;
    case callDiscardReasonHungUp::ID:
      func(static_cast<callDiscardReasonHungUp &>(obj));
      return true;
    case callId::ID:
      func(static_cast<callId &>(obj));
      return true;
    case callProblemEcho::ID:
      func(static_cast<callProblemEcho &>(obj));
      return true;
    case callProblemNoise::ID:
      func(static_cast<callProblemNoise &>(obj));
      return true;
    case callProblemInterruptions::ID:
      func(static_cast<callProblemInterruptions &>(obj));
      return true;
    case callProblemDistortedSpeech::ID:
      func(static_cast<callProblemDistortedSpeech &>(obj));
      return true;
    case callProblemSilentLocal::ID:
      func(static_cast<callProblemSilentLocal &>(obj));
      return true;
    case callProblemSilentRemote::ID:
      func(static_cast<callProblemSilentRemote &>(obj));
      return true;
    case callProblemDropped::ID:
      func(static_cast<callProblemDropped &>(obj));
      return true;
    case callProblemDistortedVideo::ID:
      func(static_cast<callProblemDistortedVideo &>(obj));
      return true;
    case callProblemPixelatedVideo::ID:
      func(static_cast<callProblemPixelatedVideo &>(obj));
      return true;
    case callProtocol::ID:
      func(static_cast<callProtocol &>(obj));
      return true;
    case callServer::ID:
      func(static_cast<callServer &>(obj));
      return true;
    case callServerTypeTelegramReflector::ID:
      func(static_cast<callServerTypeTelegramReflector &>(obj));
      return true;
    case callServerTypeWebrtc::ID:
      func(static_cast<callServerTypeWebrtc &>(obj));
      return true;
    case callStatePending::ID:
      func(static_cast<callStatePending &>(obj));
      return true;
    case callStateExchangingKeys::ID:
      func(static_cast<callStateExchangingKeys &>(obj));
      return true;
    case callStateReady::ID:
      func(static_cast<callStateReady &>(obj));
      return true;
    case callStateHangingUp::ID:
      func(static_cast<callStateHangingUp &>(obj));
      return true;
    case callStateDiscarded::ID:
      func(static_cast<callStateDiscarded &>(obj));
      return true;
    case callStateError::ID:
      func(static_cast<callStateError &>(obj));
      return true;
    case callbackQueryAnswer::ID:
      func(static_cast<callbackQueryAnswer &>(obj));
      return true;
    case callbackQueryPayloadData::ID:
      func(static_cast<callbackQueryPayloadData &>(obj));
      return true;
    case callbackQueryPayloadDataWithPassword::ID:
      func(static_cast<callbackQueryPayloadDataWithPassword &>(obj));
      return true;
    case callbackQueryPayloadGame::ID:
      func(static_cast<callbackQueryPayloadGame &>(obj));
      return true;
    case canSendMessageToUserResultOk::ID:
      func(static_cast<canSendMessageToUserResultOk &>(obj));
      return true;
    case canSendMessageToUserResultUserIsDeleted::ID:
      func(static_cast<canSendMessageToUserResultUserIsDeleted &>(obj));
      return true;
    case canSendMessageToUserResultUserRestrictsNewChats::ID:
      func(static_cast<canSendMessageToUserResultUserRestrictsNewChats &>(obj));
      return true;
    case canSendStoryResultOk::ID:
      func(static_cast<canSendStoryResultOk &>(obj));
      return true;
    case canSendStoryResultPremiumNeeded::ID:
      func(static_cast<canSendStoryResultPremiumNeeded &>(obj));
      return true;
    case canSendStoryResultBoostNeeded::ID:
      func(static_cast<canSendStoryResultBoostNeeded &>(obj));
      return true;
    case canSendStoryResultActiveStoryLimitExceeded::ID:
      func(static_cast<canSendStoryResultActiveStoryLimitExceeded &>(obj));
      return true;
    case canSendStoryResultWeeklyLimitExceeded::ID:
      func(static_cast<canSendStoryResultWeeklyLimitExceeded &>(obj));
      return true;
    case canSendStoryResultMonthlyLimitExceeded::ID:
      func(static_cast<canSendStoryResultMonthlyLimitExceeded &>(obj));
      return true;
    case canTransferOwnershipResultOk::ID:
      func(static_cast<canTransferOwnershipResultOk &>(obj));
      return true;
    case canTransferOwnershipResultPasswordNeeded::ID:
      func(static_cast<canTransferOwnershipResultPasswordNeeded &>(obj));
      return true;
    case canTransferOwnershipResultPasswordTooFresh::ID:
      func(static_cast<canTransferOwnershipResultPasswordTooFresh &>(obj));
      return true;
    case canTransferOwnershipResultSessionTooFresh::ID:
      func(static_cast<canTransferOwnershipResultSessionTooFresh &>(obj));
      return true;
    case chat::ID:
      func(static_cast<chat &>(obj));
      return true;
    case chatActionTyping::ID:
      func(static_cast<chatActionTyping &>(obj));
      return true;
    case chatActionRecordingVideo::ID:
      func(static_cast<chatActionRecordingVideo &>(obj));
      return true;
    case chatActionUploadingVideo::ID:
      func(static_cast<chatActionUploadingVideo &>(obj));
      return true;
    case chatActionRecordingVoiceNote::ID:
      func(static_cast<chatActionRecordingVoiceNote &>(obj));
      return true;
    case chatActionUploadingVoiceNote::ID:
      func(static_cast<chatActionUploadingVoiceNote &>(obj));
      return true;
    case chatActionUploadingPhoto::ID:
      func(static_cast<chatActionUploadingPhoto &>(obj));
      return true;
    case chatActionUploadingDocument::ID:
      func(static_cast<chatActionUploadingDocument &>(obj));
      return true;
    case chatActionChoosingSticker::ID:
      func(static_cast<chatActionChoosingSticker &>(obj));
      return true;
    case chatActionChoosingLocation::ID:
      func(static_cast<chatActionChoosingLocation &>(obj));
      return true;
    case chatActionChoosingContact::ID:
      func(static_cast<chatActionChoosingContact &>(obj));
      return true;
    case chatActionStartPlayingGame::ID:
      func(static_cast<chatActionStartPlayingGame &>(obj));
      return true;
    case chatActionRecordingVideoNote::ID:
      func(static_cast<chatActionRecordingVideoNote &>(obj));
      return true;
    case chatActionUploadingVideoNote::ID:
      func(static_cast<chatActionUploadingVideoNote &>(obj));
      return true;
    case chatActionWatchingAnimations::ID:
      func(static_cast<chatActionWatchingAnimations &>(obj));
      return true;
    case chatActionCancel::ID:
      func(static_cast<chatActionCancel &>(obj));
      return true;
    case chatActionBarReportSpam::ID:
      func(static_cast<chatActionBarReportSpam &>(obj));
      return true;
    case chatActionBarReportUnrelatedLocation::ID:
      func(static_cast<chatActionBarReportUnrelatedLocation &>(obj));
      return true;
    case chatActionBarInviteMembers::ID:
      func(static_cast<chatActionBarInviteMembers &>(obj));
      return true;
    case chatActionBarReportAddBlock::ID:
      func(static_cast<chatActionBarReportAddBlock &>(obj));
      return true;
    case chatActionBarAddContact::ID:
      func(static_cast<chatActionBarAddContact &>(obj));
      return true;
    case chatActionBarSharePhoneNumber::ID:
      func(static_cast<chatActionBarSharePhoneNumber &>(obj));
      return true;
    case chatActionBarJoinRequest::ID:
      func(static_cast<chatActionBarJoinRequest &>(obj));
      return true;
    case chatActiveStories::ID:
      func(static_cast<chatActiveStories &>(obj));
      return true;
    case chatAdministrator::ID:
      func(static_cast<chatAdministrator &>(obj));
      return true;
    case chatAdministratorRights::ID:
      func(static_cast<chatAdministratorRights &>(obj));
      return true;
    case chatAdministrators::ID:
      func(static_cast<chatAdministrators &>(obj));
      return true;
    case chatAvailableReactionsAll::ID:
      func(static_cast<chatAvailableReactionsAll &>(obj));
      return true;
    case chatAvailableReactionsSome::ID:
      func(static_cast<chatAvailableReactionsSome &>(obj));
      return true;
    case chatBackground::ID:
      func(static_cast<chatBackground &>(obj));
      return true;
    case chatBoost::ID:
      func(static_cast<chatBoost &>(obj));
      return true;
    case chatBoostFeatures::ID:
      func(static_cast<chatBoostFeatures &>(obj));
      return true;
    case chatBoostLevelFeatures::ID:
      func(static_cast<chatBoostLevelFeatures &>(obj));
      return true;
    case chatBoostLink::ID:
      func(static_cast<chatBoostLink &>(obj));
      return true;
    case chatBoostLinkInfo::ID:
      func(static_cast<chatBoostLinkInfo &>(obj));
      return true;
    case chatBoostSlot::ID:
      func(static_cast<chatBoostSlot &>(obj));
      return true;
    case chatBoostSlots::ID:
      func(static_cast<chatBoostSlots &>(obj));
      return true;
    case chatBoostSourceGiftCode::ID:
      func(static_cast<chatBoostSourceGiftCode &>(obj));
      return true;
    case chatBoostSourceGiveaway::ID:
      func(static_cast<chatBoostSourceGiveaway &>(obj));
      return true;
    case chatBoostSourcePremium::ID:
      func(static_cast<chatBoostSourcePremium &>(obj));
      return true;
    case chatBoostStatus::ID:
      func(static_cast<chatBoostStatus &>(obj));
      return true;
    case chatEvent::ID:
      func(static_cast<chatEvent &>(obj));
      return true;
    case chatEventMessageEdited::ID:
      func(static_cast<chatEventMessageEdited &>(obj));
      return true;
    case chatEventMessageDeleted::ID:
      func(static_cast<chatEventMessageDeleted &>(obj));
      return true;
    case chatEventMessagePinned::ID:
      func(static_cast<chatEventMessagePinned &>(obj));
      return true;
    case chatEventMessageUnpinned::ID:
      func(static_cast<chatEventMessageUnpinned &>(obj));
      return true;
    case chatEventPollStopped::ID:
      func(static_cast<chatEventPollStopped &>(obj));
      return true;
    case chatEventMemberJoined::ID:
      func(static_cast<chatEventMemberJoined &>(obj));
      return true;
    case chatEventMemberJoinedByInviteLink::ID:
      func(static_cast<chatEventMemberJoinedByInviteLink &>(obj));
      return true;
    case chatEventMemberJoinedByRequest::ID:
      func(static_cast<chatEventMemberJoinedByRequest &>(obj));
      return true;
    case chatEventMemberInvited::ID:
      func(static_cast<chatEventMemberInvited &>(obj));
      return true;
    case chatEventMemberLeft::ID:
      func(static_cast<chatEventMemberLeft &>(obj));
      return true;
    case chatEventMemberPromoted::ID:
      func(static_cast<chatEventMemberPromoted &>(obj));
      return true;
    case chatEventMemberRestricted::ID:
      func(static_cast<chatEventMemberRestricted &>(obj));
      return true;
    case chatEventMemberSubscriptionExtended::ID:
      func(static_cast<chatEventMemberSubscriptionExtended &>(obj));
      return true;
    case chatEventAvailableReactionsChanged::ID:
      func(static_cast<chatEventAvailableReactionsChanged &>(obj));
      return true;
    case chatEventBackgroundChanged::ID:
      func(static_cast<chatEventBackgroundChanged &>(obj));
      return true;
    case chatEventDescriptionChanged::ID:
      func(static_cast<chatEventDescriptionChanged &>(obj));
      return true;
    case chatEventEmojiStatusChanged::ID:
      func(static_cast<chatEventEmojiStatusChanged &>(obj));
      return true;
    case chatEventLinkedChatChanged::ID:
      func(static_cast<chatEventLinkedChatChanged &>(obj));
      return true;
    case chatEventLocationChanged::ID:
      func(static_cast<chatEventLocationChanged &>(obj));
      return true;
    case chatEventMessageAutoDeleteTimeChanged::ID:
      func(static_cast<chatEventMessageAutoDeleteTimeChanged &>(obj));
      return true;
    case chatEventPermissionsChanged::ID:
      func(static_cast<chatEventPermissionsChanged &>(obj));
      return true;
    case chatEventPhotoChanged::ID:
      func(static_cast<chatEventPhotoChanged &>(obj));
      return true;
    case chatEventSlowModeDelayChanged::ID:
      func(static_cast<chatEventSlowModeDelayChanged &>(obj));
      return true;
    case chatEventStickerSetChanged::ID:
      func(static_cast<chatEventStickerSetChanged &>(obj));
      return true;
    case chatEventCustomEmojiStickerSetChanged::ID:
      func(static_cast<chatEventCustomEmojiStickerSetChanged &>(obj));
      return true;
    case chatEventTitleChanged::ID:
      func(static_cast<chatEventTitleChanged &>(obj));
      return true;
    case chatEventUsernameChanged::ID:
      func(static_cast<chatEventUsernameChanged &>(obj));
      return true;
    case chatEventActiveUsernamesChanged::ID:
      func(static_cast<chatEventActiveUsernamesChanged &>(obj));
      return true;
    case chatEventAccentColorChanged::ID:
      func(static_cast<chatEventAccentColorChanged &>(obj));
      return true;
    case chatEventProfileAccentColorChanged::ID:
      func(static_cast<chatEventProfileAccentColorChanged &>(obj));
      return true;
    case chatEventHasProtectedContentToggled::ID:
      func(static_cast<chatEventHasProtectedContentToggled &>(obj));
      return true;
    case chatEventInvitesToggled::ID:
      func(static_cast<chatEventInvitesToggled &>(obj));
      return true;
    case chatEventIsAllHistoryAvailableToggled::ID:
      func(static_cast<chatEventIsAllHistoryAvailableToggled &>(obj));
      return true;
    case chatEventHasAggressiveAntiSpamEnabledToggled::ID:
      func(static_cast<chatEventHasAggressiveAntiSpamEnabledToggled &>(obj));
      return true;
    case chatEventSignMessagesToggled::ID:
      func(static_cast<chatEventSignMessagesToggled &>(obj));
      return true;
    case chatEventShowMessageSenderToggled::ID:
      func(static_cast<chatEventShowMessageSenderToggled &>(obj));
      return true;
    case chatEventInviteLinkEdited::ID:
      func(static_cast<chatEventInviteLinkEdited &>(obj));
      return true;
    case chatEventInviteLinkRevoked::ID:
      func(static_cast<chatEventInviteLinkRevoked &>(obj));
      return true;
    case chatEventInviteLinkDeleted::ID:
      func(static_cast<chatEventInviteLinkDeleted &>(obj));
      return true;
    case chatEventVideoChatCreated::ID:
      func(static_cast<chatEventVideoChatCreated &>(obj));
      return true;
    case chatEventVideoChatEnded::ID:
      func(static_cast<chatEventVideoChatEnded &>(obj));
      return true;
    case chatEventVideoChatMuteNewParticipantsToggled::ID:
      func(static_cast<chatEventVideoChatMuteNewParticipantsToggled &>(obj));
      return true;
    case chatEventVideoChatParticipantIsMutedToggled::ID:
      func(static_cast<chatEventVideoChatParticipantIsMutedToggled &>(obj));
      return true;
    case chatEventVideoChatParticipantVolumeLevelChanged::ID:
      func(static_cast<chatEventVideoChatParticipantVolumeLevelChanged &>(obj));
      return true;
    case chatEventIsForumToggled::ID:
      func(static_cast<chatEventIsForumToggled &>(obj));
      return true;
    case chatEventForumTopicCreated::ID:
      func(static_cast<chatEventForumTopicCreated &>(obj));
      return true;
    case chatEventForumTopicEdited::ID:
      func(static_cast<chatEventForumTopicEdited &>(obj));
      return true;
    case chatEventForumTopicToggleIsClosed::ID:
      func(static_cast<chatEventForumTopicToggleIsClosed &>(obj));
      return true;
    case chatEventForumTopicToggleIsHidden::ID:
      func(static_cast<chatEventForumTopicToggleIsHidden &>(obj));
      return true;
    case chatEventForumTopicDeleted::ID:
      func(static_cast<chatEventForumTopicDeleted &>(obj));
      return true;
    case chatEventForumTopicPinned::ID:
      func(static_cast<chatEventForumTopicPinned &>(obj));
      return true;
    case chatEventLogFilters::ID:
      func(static_cast<chatEventLogFilters &>(obj));
      return true;
    case chatEvents::ID:
      func(static_cast<chatEvents &>(obj));
      return true;
    case chatFolder::ID:
      func(static_cast<chatFolder &>(obj));
      return true;
    case chatFolderIcon::ID:
      func(static_cast<chatFolderIcon &>(obj));
      return true;
    case chatFolderInfo::ID:
      func(static_cast<chatFolderInfo &>(obj));
      return true;
    case chatFolderInviteLink::ID:
      func(static_cast<chatFolderInviteLink &>(obj));
      return true;
    case chatFolderInviteLinkInfo::ID:
      func(static_cast<chatFolderInviteLinkInfo &>(obj));
      return true;
    case chatFolderInviteLinks::ID:
      func(static_cast<chatFolderInviteLinks &>(obj));
      return true;
    case chatInviteLink::ID:
      func(static_cast<chatInviteLink &>(obj));
      return true;
    case chatInviteLinkCount::ID:
      func(static_cast<chatInviteLinkCount &>(obj));
      return true;
    case chatInviteLinkCounts::ID:
      func(static_cast<chatInviteLinkCounts &>(obj));
      return true;
    case chatInviteLinkInfo::ID:
      func(static_cast<chatInviteLinkInfo &>(obj));
      return true;
    case chatInviteLinkMember::ID:
      func(static_cast<chatInviteLinkMember &>(obj));
      return true;
    case chatInviteLinkMembers::ID:
      func(static_cast<chatInviteLinkMembers &>(obj));
      return true;
    case chatInviteLinkSubscriptionInfo::ID:
      func(static_cast<chatInviteLinkSubscriptionInfo &>(obj));
      return true;
    case chatInviteLinks::ID:
      func(static_cast<chatInviteLinks &>(obj));
      return true;
    case chatJoinRequest::ID:
      func(static_cast<chatJoinRequest &>(obj));
      return true;
    case chatJoinRequests::ID:
      func(static_cast<chatJoinRequests &>(obj));
      return true;
    case chatJoinRequestsInfo::ID:
      func(static_cast<chatJoinRequestsInfo &>(obj));
      return true;
    case chatListMain::ID:
      func(static_cast<chatListMain &>(obj));
      return true;
    case chatListArchive::ID:
      func(static_cast<chatListArchive &>(obj));
      return true;
    case chatListFolder::ID:
      func(static_cast<chatListFolder &>(obj));
      return true;
    case chatLists::ID:
      func(static_cast<chatLists &>(obj));
      return true;
    case chatLocation::ID:
      func(static_cast<chatLocation &>(obj));
      return true;
    case chatMember::ID:
      func(static_cast<chatMember &>(obj));
      return true;
    case chatMemberStatusCreator::ID:
      func(static_cast<chatMemberStatusCreator &>(obj));
      return true;
    case chatMemberStatusAdministrator::ID:
      func(static_cast<chatMemberStatusAdministrator &>(obj));
      return true;
    case chatMemberStatusMember::ID:
      func(static_cast<chatMemberStatusMember &>(obj));
      return true;
    case chatMemberStatusRestricted::ID:
      func(static_cast<chatMemberStatusRestricted &>(obj));
      return true;
    case chatMemberStatusLeft::ID:
      func(static_cast<chatMemberStatusLeft &>(obj));
      return true;
    case chatMemberStatusBanned::ID:
      func(static_cast<chatMemberStatusBanned &>(obj));
      return true;
    case chatMembers::ID:
      func(static_cast<chatMembers &>(obj));
      return true;
    case chatMembersFilterContacts::ID:
      func(static_cast<chatMembersFilterContacts &>(obj));
      return true;
    case chatMembersFilterAdministrators::ID:
      func(static_cast<chatMembersFilterAdministrators &>(obj));
      return true;
    case chatMembersFilterMembers::ID:
      func(static_cast<chatMembersFilterMembers &>(obj));
      return true;
    case chatMembersFilterMention::ID:
      func(static_cast<chatMembersFilterMention &>(obj));
      return true;
    case chatMembersFilterRestricted::ID:
      func(static_cast<chatMembersFilterRestricted &>(obj));
      return true;
    case chatMembersFilterBanned::ID:
      func(static_cast<chatMembersFilterBanned &>(obj));
      return true;
    case chatMembersFilterBots::ID:
      func(static_cast<chatMembersFilterBots &>(obj));
      return true;
    case chatMessageSender::ID:
      func(static_cast<chatMessageSender &>(obj));
      return true;
    case chatMessageSenders::ID:
      func(static_cast<chatMessageSenders &>(obj));
      return true;
    case chatNearby::ID:
      func(static_cast<chatNearby &>(obj));
      return true;
    case chatNotificationSettings::ID:
      func(static_cast<chatNotificationSettings &>(obj));
      return true;
    case chatPermissions::ID:
      func(static_cast<chatPermissions &>(obj));
      return true;
    case chatPhoto::ID:
      func(static_cast<chatPhoto &>(obj));
      return true;
    case chatPhotoInfo::ID:
      func(static_cast<chatPhotoInfo &>(obj));
      return true;
    case chatPhotoSticker::ID:
      func(static_cast<chatPhotoSticker &>(obj));
      return true;
    case chatPhotoStickerTypeRegularOrMask::ID:
      func(static_cast<chatPhotoStickerTypeRegularOrMask &>(obj));
      return true;
    case chatPhotoStickerTypeCustomEmoji::ID:
      func(static_cast<chatPhotoStickerTypeCustomEmoji &>(obj));
      return true;
    case chatPhotos::ID:
      func(static_cast<chatPhotos &>(obj));
      return true;
    case chatPosition::ID:
      func(static_cast<chatPosition &>(obj));
      return true;
    case chatRevenueAmount::ID:
      func(static_cast<chatRevenueAmount &>(obj));
      return true;
    case chatRevenueStatistics::ID:
      func(static_cast<chatRevenueStatistics &>(obj));
      return true;
    case chatRevenueTransaction::ID:
      func(static_cast<chatRevenueTransaction &>(obj));
      return true;
    case chatRevenueTransactionTypeEarnings::ID:
      func(static_cast<chatRevenueTransactionTypeEarnings &>(obj));
      return true;
    case chatRevenueTransactionTypeWithdrawal::ID:
      func(static_cast<chatRevenueTransactionTypeWithdrawal &>(obj));
      return true;
    case chatRevenueTransactionTypeRefund::ID:
      func(static_cast<chatRevenueTransactionTypeRefund &>(obj));
      return true;
    case chatRevenueTransactions::ID:
      func(static_cast<chatRevenueTransactions &>(obj));
      return true;
    case chatSourceMtprotoProxy::ID:
      func(static_cast<chatSourceMtprotoProxy &>(obj));
      return true;
    case chatSourcePublicServiceAnnouncement::ID:
      func(static_cast<chatSourcePublicServiceAnnouncement &>(obj));
      return true;
    case chatStatisticsSupergroup::ID:
      func(static_cast<chatStatisticsSupergroup &>(obj));
      return true;
    case chatStatisticsChannel::ID:
      func(static_cast<chatStatisticsChannel &>(obj));
      return true;
    case chatStatisticsAdministratorActionsInfo::ID:
      func(static_cast<chatStatisticsAdministratorActionsInfo &>(obj));
      return true;
    case chatStatisticsInteractionInfo::ID:
      func(static_cast<chatStatisticsInteractionInfo &>(obj));
      return true;
    case chatStatisticsInviterInfo::ID:
      func(static_cast<chatStatisticsInviterInfo &>(obj));
      return true;
    case chatStatisticsMessageSenderInfo::ID:
      func(static_cast<chatStatisticsMessageSenderInfo &>(obj));
      return true;
    case chatStatisticsObjectTypeMessage::ID:
      func(static_cast<chatStatisticsObjectTypeMessage &>(obj));
      return true;
    case chatStatisticsObjectTypeStory::ID:
      func(static_cast<chatStatisticsObjectTypeStory &>(obj));
      return true;
    case chatTheme::ID:
      func(static_cast<chatTheme &>(obj));
      return true;
    case chatTransactionPurposePaidMedia::ID:
      func(static_cast<chatTransactionPurposePaidMedia &>(obj));
      return true;
    case chatTransactionPurposeJoin::ID:
      func(static_cast<chatTransactionPurposeJoin &>(obj));
      return true;
    case chatTransactionPurposeReaction::ID:
      func(static_cast<chatTransactionPurposeReaction &>(obj));
      return true;
    case chatTransactionPurposeGiveaway::ID:
      func(static_cast<chatTransactionPurposeGiveaway &>(obj));
      return true;
    case chatTypePrivate::ID:
      func(static_cast<chatTypePrivate &>(obj));
      return true;
    case chatTypeBasicGroup::ID:
      func(static_cast<chatTypeBasicGroup &>(obj));
      return true;
    case chatTypeSupergroup::ID:
      func(static_cast<chatTypeSupergroup &>(obj));
      return true;
    case chatTypeSecret::ID:
      func(static_cast<chatTypeSecret &>(obj));
      return true;
    case chats::ID:
      func(static_cast<chats &>(obj));
      return true;
    case chatsNearby::ID:
      func(static_cast<chatsNearby &>(obj));
      return true;
    case checkChatUsernameResultOk::ID:
      func(static_cast<checkChatUsernameResultOk &>(obj));
      return true;
    case checkChatUsernameResultUsernameInvalid::ID:
      func(static_cast<checkChatUsernameResultUsernameInvalid &>(obj));
      return true;
    case checkChatUsernameResultUsernameOccupied::ID:
      func(static_cast<checkChatUsernameResultUsernameOccupied &>(obj));
      return true;
    case checkChatUsernameResultUsernamePurchasable::ID:
      func(static_cast<checkChatUsernameResultUsernamePurchasable &>(obj));
      return true;
    case checkChatUsernameResultPublicChatsTooMany::ID:
      func(static_cast<checkChatUsernameResultPublicChatsTooMany &>(obj));
      return true;
    case checkChatUsernameResultPublicGroupsUnavailable::ID:
      func(static_cast<checkChatUsernameResultPublicGroupsUnavailable &>(obj));
      return true;
    case checkStickerSetNameResultOk::ID:
      func(static_cast<checkStickerSetNameResultOk &>(obj));
      return true;
    case checkStickerSetNameResultNameInvalid::ID:
      func(static_cast<checkStickerSetNameResultNameInvalid &>(obj));
      return true;
    case checkStickerSetNameResultNameOccupied::ID:
      func(static_cast<checkStickerSetNameResultNameOccupied &>(obj));
      return true;
    case closeBirthdayUser::ID:
      func(static_cast<closeBirthdayUser &>(obj));
      return true;
    case closedVectorPath::ID:
      func(static_cast<closedVectorPath &>(obj));
      return true;
    case collectibleItemInfo::ID:
      func(static_cast<collectibleItemInfo &>(obj));
      return true;
    case collectibleItemTypeUsername::ID:
      func(static_cast<collectibleItemTypeUsername &>(obj));
      return true;
    case collectibleItemTypePhoneNumber::ID:
      func(static_cast<collectibleItemTypePhoneNumber &>(obj));
      return true;
    case connectedWebsite::ID:
      func(static_cast<connectedWebsite &>(obj));
      return true;
    case connectedWebsites::ID:
      func(static_cast<connectedWebsites &>(obj));
      return true;
    case connectionStateWaitingForNetwork::ID:
      func(static_cast<connectionStateWaitingForNetwork &>(obj));
      return true;
    case connectionStateConnectingToProxy::ID:
      func(static_cast<connectionStateConnectingToProxy &>(obj));
      return true;
    case connectionStateConnecting::ID:
      func(static_cast<connectionStateConnecting &>(obj));
      return true;
    case connectionStateUpdating::ID:
      func(static_cast<connectionStateUpdating &>(obj));
      return true;
    case connectionStateReady::ID:
      func(static_cast<connectionStateReady &>(obj));
      return true;
    case contact::ID:
      func(static_cast<contact &>(obj));
      return true;
    case count::ID:
      func(static_cast<count &>(obj));
      return true;
    case countries::ID:
      func(static_cast<countries &>(obj));
      return true;
    case countryInfo::ID:
      func(static_cast<countryInfo &>(obj));
      return true;
    case createdBasicGroupChat::ID:
      func(static_cast<createdBasicGroupChat &>(obj));
      return true;
    case currentWeather::ID:
      func(static_cast<currentWeather &>(obj));
      return true;
    case customRequestResult::ID:
      func(static_cast<customRequestResult &>(obj));
      return true;
    case databaseStatistics::ID:
      func(static_cast<databaseStatistics &>(obj));
      return true;
    case date::ID:
      func(static_cast<date &>(obj));
      return true;
    case dateRange::ID:
      func(static_cast<dateRange &>(obj));
      return true;
    case datedFile::ID:
      func(static_cast<datedFile &>(obj));
      return true;
    case deepLinkInfo::ID:
      func(static_cast<deepLinkInfo &>(obj));
      return true;
    case deviceTokenFirebaseCloudMessaging::ID:
      func(static_cast<deviceTokenFirebaseCloudMessaging &>(obj));
      return true;
    case deviceTokenApplePush::ID:
      func(static_cast<deviceTokenApplePush &>(obj));
      return true;
    case deviceTokenApplePushVoIP::ID:
      func(static_cast<deviceTokenApplePushVoIP &>(obj));
      return true;
    case deviceTokenWindowsPush::ID:
      func(static_cast<deviceTokenWindowsPush &>(obj));
      return true;
    case deviceTokenMicrosoftPush::ID:
      func(static_cast<deviceTokenMicrosoftPush &>(obj));
      return true;
    case deviceTokenMicrosoftPushVoIP::ID:
      func(static_cast<deviceTokenMicrosoftPushVoIP &>(obj));
      return true;
    case deviceTokenWebPush::ID:
      func(static_cast<deviceTokenWebPush &>(obj));
      return true;
    case deviceTokenSimplePush::ID:
      func(static_cast<deviceTokenSimplePush &>(obj));
      return true;
    case deviceTokenUbuntuPush::ID:
      func(static_cast<deviceTokenUbuntuPush &>(obj));
      return true;
    case deviceTokenBlackBerryPush::ID:
      func(static_cast<deviceTokenBlackBerryPush &>(obj));
      return true;
    case deviceTokenTizenPush::ID:
      func(static_cast<deviceTokenTizenPush &>(obj));
      return true;
    case deviceTokenHuaweiPush::ID:
      func(static_cast<deviceTokenHuaweiPush &>(obj));
      return true;
    case diceStickersRegular::ID:
      func(static_cast<diceStickersRegular &>(obj));
      return true;
    case diceStickersSlotMachine::ID:
      func(static_cast<diceStickersSlotMachine &>(obj));
      return true;
    case document::ID:
      func(static_cast<document &>(obj));
      return true;
    case downloadedFileCounts::ID:
      func(static_cast<downloadedFileCounts &>(obj));
      return true;
    case draftMessage::ID:
      func(static_cast<draftMessage &>(obj));
      return true;
    case emailAddressAuthenticationCode::ID:
      func(static_cast<emailAddressAuthenticationCode &>(obj));
      return true;
    case emailAddressAuthenticationAppleId::ID:
      func(static_cast<emailAddressAuthenticationAppleId &>(obj));
      return true;
    case emailAddressAuthenticationGoogleId::ID:
      func(static_cast<emailAddressAuthenticationGoogleId &>(obj));
      return true;
    case emailAddressAuthenticationCodeInfo::ID:
      func(static_cast<emailAddressAuthenticationCodeInfo &>(obj));
      return true;
    case emailAddressResetStateAvailable::ID:
      func(static_cast<emailAddressResetStateAvailable &>(obj));
      return true;
    case emailAddressResetStatePending::ID:
      func(static_cast<emailAddressResetStatePending &>(obj));
      return true;
    case emojiCategories::ID:
      func(static_cast<emojiCategories &>(obj));
      return true;
    case emojiCategory::ID:
      func(static_cast<emojiCategory &>(obj));
      return true;
    case emojiCategorySourceSearch::ID:
      func(static_cast<emojiCategorySourceSearch &>(obj));
      return true;
    case emojiCategorySourcePremium::ID:
      func(static_cast<emojiCategorySourcePremium &>(obj));
      return true;
    case emojiCategoryTypeDefault::ID:
      func(static_cast<emojiCategoryTypeDefault &>(obj));
      return true;
    case emojiCategoryTypeRegularStickers::ID:
      func(static_cast<emojiCategoryTypeRegularStickers &>(obj));
      return true;
    case emojiCategoryTypeEmojiStatus::ID:
      func(static_cast<emojiCategoryTypeEmojiStatus &>(obj));
      return true;
    case emojiCategoryTypeChatPhoto::ID:
      func(static_cast<emojiCategoryTypeChatPhoto &>(obj));
      return true;
    case emojiKeyword::ID:
      func(static_cast<emojiKeyword &>(obj));
      return true;
    case emojiKeywords::ID:
      func(static_cast<emojiKeywords &>(obj));
      return true;
    case emojiReaction::ID:
      func(static_cast<emojiReaction &>(obj));
      return true;
    case emojiStatus::ID:
      func(static_cast<emojiStatus &>(obj));
      return true;
    case emojiStatuses::ID:
      func(static_cast<emojiStatuses &>(obj));
      return true;
    case emojis::ID:
      func(static_cast<emojis &>(obj));
      return true;
    case encryptedCredentials::ID:
      func(static_cast<encryptedCredentials &>(obj));
      return true;
    case encryptedPassportElement::ID:
      func(static_cast<encryptedPassportElement &>(obj));
      return true;
    case error::ID:
      func(static_cast<error &>(obj));
      return true;
    case factCheck::ID:
      func(static_cast<factCheck &>(obj));
      return true;
    case failedToAddMember::ID:
      func(static_cast<failedToAddMember &>(obj));
      return true;
    case failedToAddMembers::ID:
      func(static_cast<failedToAddMembers &>(obj));
      return true;
    case file::ID:
      func(static_cast<file &>(obj));
      return true;
    case fileDownload::ID:
      func(static_cast<fileDownload &>(obj));
      return true;
    case fileDownloadedPrefixSize::ID:
      func(static_cast<fileDownloadedPrefixSize &>(obj));
      return true;
    case filePart::ID:
      func(static_cast<filePart &>(obj));
      return true;
    case fileTypeNone::ID:
      func(static_cast<fileTypeNone &>(obj));
      return true;
    case fileTypeAnimation::ID:
      func(static_cast<fileTypeAnimation &>(obj));
      return true;
    case fileTypeAudio::ID:
      func(static_cast<fileTypeAudio &>(obj));
      return true;
    case fileTypeDocument::ID:
      func(static_cast<fileTypeDocument &>(obj));
      return true;
    case fileTypeNotificationSound::ID:
      func(static_cast<fileTypeNotificationSound &>(obj));
      return true;
    case fileTypePhoto::ID:
      func(static_cast<fileTypePhoto &>(obj));
      return true;
    case fileTypePhotoStory::ID:
      func(static_cast<fileTypePhotoStory &>(obj));
      return true;
    case fileTypeProfilePhoto::ID:
      func(static_cast<fileTypeProfilePhoto &>(obj));
      return true;
    case fileTypeSecret::ID:
      func(static_cast<fileTypeSecret &>(obj));
      return true;
    case fileTypeSecretThumbnail::ID:
      func(static_cast<fileTypeSecretThumbnail &>(obj));
      return true;
    case fileTypeSecure::ID:
      func(static_cast<fileTypeSecure &>(obj));
      return true;
    case fileTypeSticker::ID:
      func(static_cast<fileTypeSticker &>(obj));
      return true;
    case fileTypeThumbnail::ID:
      func(static_cast<fileTypeThumbnail &>(obj));
      return true;
    case fileTypeUnknown::ID:
      func(static_cast<fileTypeUnknown &>(obj));
      return true;
    case fileTypeVideo::ID:
      func(static_cast<fileTypeVideo &>(obj));
      return true;
    case fileTypeVideoNote::ID:
      func(static_cast<fileTypeVideoNote &>(obj));
      return true;
    case fileTypeVideoStory::ID:
      func(static_cast<fileTypeVideoStory &>(obj));
      return true;
    case fileTypeVoiceNote::ID:
      func(static_cast<fileTypeVoiceNote &>(obj));
      return true;
    case fileTypeWallpaper::ID:
      func(static_cast<fileTypeWallpaper &>(obj));
      return true;
    case firebaseAuthenticationSettingsAndroid::ID:
      func(static_cast<firebaseAuthenticationSettingsAndroid &>(obj));
      return true;
    case firebaseAuthenticationSettingsIos::ID:
      func(static_cast<firebaseAuthenticationSettingsIos &>(obj));
      return true;
    case firebaseDeviceVerificationParametersSafetyNet::ID:
      func(static_cast<firebaseDeviceVerificationParametersSafetyNet &>(obj));
      return true;
    case firebaseDeviceVerificationParametersPlayIntegrity::ID:
      func(static_cast<firebaseDeviceVerificationParametersPlayIntegrity &>(obj));
      return true;
    case formattedText::ID:
      func(static_cast<formattedText &>(obj));
      return true;
    case forumTopic::ID:
      func(static_cast<forumTopic &>(obj));
      return true;
    case forumTopicIcon::ID:
      func(static_cast<forumTopicIcon &>(obj));
      return true;
    case forumTopicInfo::ID:
      func(static_cast<forumTopicInfo &>(obj));
      return true;
    case forumTopics::ID:
      func(static_cast<forumTopics &>(obj));
      return true;
    case forwardSource::ID:
      func(static_cast<forwardSource &>(obj));
      return true;
    case foundChatBoosts::ID:
      func(static_cast<foundChatBoosts &>(obj));
      return true;
    case foundChatMessages::ID:
      func(static_cast<foundChatMessages &>(obj));
      return true;
    case foundFileDownloads::ID:
      func(static_cast<foundFileDownloads &>(obj));
      return true;
    case foundMessages::ID:
      func(static_cast<foundMessages &>(obj));
      return true;
    case foundPosition::ID:
      func(static_cast<foundPosition &>(obj));
      return true;
    case foundPositions::ID:
      func(static_cast<foundPositions &>(obj));
      return true;
    case foundStories::ID:
      func(static_cast<foundStories &>(obj));
      return true;
    case foundUsers::ID:
      func(static_cast<foundUsers &>(obj));
      return true;
    case foundWebApp::ID:
      func(static_cast<foundWebApp &>(obj));
      return true;
    case game::ID:
      func(static_cast<game &>(obj));
      return true;
    case gameHighScore::ID:
      func(static_cast<gameHighScore &>(obj));
      return true;
    case gameHighScores::ID:
      func(static_cast<gameHighScores &>(obj));
      return true;
    case giveawayInfoOngoing::ID:
      func(static_cast<giveawayInfoOngoing &>(obj));
      return true;
    case giveawayInfoCompleted::ID:
      func(static_cast<giveawayInfoCompleted &>(obj));
      return true;
    case giveawayParameters::ID:
      func(static_cast<giveawayParameters &>(obj));
      return true;
    case giveawayParticipantStatusEligible::ID:
      func(static_cast<giveawayParticipantStatusEligible &>(obj));
      return true;
    case giveawayParticipantStatusParticipating::ID:
      func(static_cast<giveawayParticipantStatusParticipating &>(obj));
      return true;
    case giveawayParticipantStatusAlreadyWasMember::ID:
      func(static_cast<giveawayParticipantStatusAlreadyWasMember &>(obj));
      return true;
    case giveawayParticipantStatusAdministrator::ID:
      func(static_cast<giveawayParticipantStatusAdministrator &>(obj));
      return true;
    case giveawayParticipantStatusDisallowedCountry::ID:
      func(static_cast<giveawayParticipantStatusDisallowedCountry &>(obj));
      return true;
    case giveawayPrizePremium::ID:
      func(static_cast<giveawayPrizePremium &>(obj));
      return true;
    case giveawayPrizeStars::ID:
      func(static_cast<giveawayPrizeStars &>(obj));
      return true;
    case groupCall::ID:
      func(static_cast<groupCall &>(obj));
      return true;
    case groupCallId::ID:
      func(static_cast<groupCallId &>(obj));
      return true;
    case groupCallParticipant::ID:
      func(static_cast<groupCallParticipant &>(obj));
      return true;
    case groupCallParticipantVideoInfo::ID:
      func(static_cast<groupCallParticipantVideoInfo &>(obj));
      return true;
    case groupCallRecentSpeaker::ID:
      func(static_cast<groupCallRecentSpeaker &>(obj));
      return true;
    case groupCallStream::ID:
      func(static_cast<groupCallStream &>(obj));
      return true;
    case groupCallStreams::ID:
      func(static_cast<groupCallStreams &>(obj));
      return true;
    case groupCallVideoQualityThumbnail::ID:
      func(static_cast<groupCallVideoQualityThumbnail &>(obj));
      return true;
    case groupCallVideoQualityMedium::ID:
      func(static_cast<groupCallVideoQualityMedium &>(obj));
      return true;
    case groupCallVideoQualityFull::ID:
      func(static_cast<groupCallVideoQualityFull &>(obj));
      return true;
    case groupCallVideoSourceGroup::ID:
      func(static_cast<groupCallVideoSourceGroup &>(obj));
      return true;
    case hashtags::ID:
      func(static_cast<hashtags &>(obj));
      return true;
    case httpUrl::ID:
      func(static_cast<httpUrl &>(obj));
      return true;
    case identityDocument::ID:
      func(static_cast<identityDocument &>(obj));
      return true;
    case importedContacts::ID:
      func(static_cast<importedContacts &>(obj));
      return true;
    case inlineKeyboardButton::ID:
      func(static_cast<inlineKeyboardButton &>(obj));
      return true;
    case inlineKeyboardButtonTypeUrl::ID:
      func(static_cast<inlineKeyboardButtonTypeUrl &>(obj));
      return true;
    case inlineKeyboardButtonTypeLoginUrl::ID:
      func(static_cast<inlineKeyboardButtonTypeLoginUrl &>(obj));
      return true;
    case inlineKeyboardButtonTypeWebApp::ID:
      func(static_cast<inlineKeyboardButtonTypeWebApp &>(obj));
      return true;
    case inlineKeyboardButtonTypeCallback::ID:
      func(static_cast<inlineKeyboardButtonTypeCallback &>(obj));
      return true;
    case inlineKeyboardButtonTypeCallbackWithPassword::ID:
      func(static_cast<inlineKeyboardButtonTypeCallbackWithPassword &>(obj));
      return true;
    case inlineKeyboardButtonTypeCallbackGame::ID:
      func(static_cast<inlineKeyboardButtonTypeCallbackGame &>(obj));
      return true;
    case inlineKeyboardButtonTypeSwitchInline::ID:
      func(static_cast<inlineKeyboardButtonTypeSwitchInline &>(obj));
      return true;
    case inlineKeyboardButtonTypeBuy::ID:
      func(static_cast<inlineKeyboardButtonTypeBuy &>(obj));
      return true;
    case inlineKeyboardButtonTypeUser::ID:
      func(static_cast<inlineKeyboardButtonTypeUser &>(obj));
      return true;
    case inlineQueryResultArticle::ID:
      func(static_cast<inlineQueryResultArticle &>(obj));
      return true;
    case inlineQueryResultContact::ID:
      func(static_cast<inlineQueryResultContact &>(obj));
      return true;
    case inlineQueryResultLocation::ID:
      func(static_cast<inlineQueryResultLocation &>(obj));
      return true;
    case inlineQueryResultVenue::ID:
      func(static_cast<inlineQueryResultVenue &>(obj));
      return true;
    case inlineQueryResultGame::ID:
      func(static_cast<inlineQueryResultGame &>(obj));
      return true;
    case inlineQueryResultAnimation::ID:
      func(static_cast<inlineQueryResultAnimation &>(obj));
      return true;
    case inlineQueryResultAudio::ID:
      func(static_cast<inlineQueryResultAudio &>(obj));
      return true;
    case inlineQueryResultDocument::ID:
      func(static_cast<inlineQueryResultDocument &>(obj));
      return true;
    case inlineQueryResultPhoto::ID:
      func(static_cast<inlineQueryResultPhoto &>(obj));
      return true;
    case inlineQueryResultSticker::ID:
      func(static_cast<inlineQueryResultSticker &>(obj));
      return true;
    case inlineQueryResultVideo::ID:
      func(static_cast<inlineQueryResultVideo &>(obj));
      return true;
    case inlineQueryResultVoiceNote::ID:
      func(static_cast<inlineQueryResultVoiceNote &>(obj));
      return true;
    case inlineQueryResults::ID:
      func(static_cast<inlineQueryResults &>(obj));
      return true;
    case inlineQueryResultsButton::ID:
      func(static_cast<inlineQueryResultsButton &>(obj));
      return true;
    case inlineQueryResultsButtonTypeStartBot::ID:
      func(static_cast<inlineQueryResultsButtonTypeStartBot &>(obj));
      return true;
    case inlineQueryResultsButtonTypeWebApp::ID:
      func(static_cast<inlineQueryResultsButtonTypeWebApp &>(obj));
      return true;
    case inputBackgroundLocal::ID:
      func(static_cast<inputBackgroundLocal &>(obj));
      return true;
    case inputBackgroundRemote::ID:
      func(static_cast<inputBackgroundRemote &>(obj));
      return true;
    case inputBackgroundPrevious::ID:
      func(static_cast<inputBackgroundPrevious &>(obj));
      return true;
    case inputBusinessChatLink::ID:
      func(static_cast<inputBusinessChatLink &>(obj));
      return true;
    case inputBusinessStartPage::ID:
      func(static_cast<inputBusinessStartPage &>(obj));
      return true;
    case inputChatPhotoPrevious::ID:
      func(static_cast<inputChatPhotoPrevious &>(obj));
      return true;
    case inputChatPhotoStatic::ID:
      func(static_cast<inputChatPhotoStatic &>(obj));
      return true;
    case inputChatPhotoAnimation::ID:
      func(static_cast<inputChatPhotoAnimation &>(obj));
      return true;
    case inputChatPhotoSticker::ID:
      func(static_cast<inputChatPhotoSticker &>(obj));
      return true;
    case inputCredentialsSaved::ID:
      func(static_cast<inputCredentialsSaved &>(obj));
      return true;
    case inputCredentialsNew::ID:
      func(static_cast<inputCredentialsNew &>(obj));
      return true;
    case inputCredentialsApplePay::ID:
      func(static_cast<inputCredentialsApplePay &>(obj));
      return true;
    case inputCredentialsGooglePay::ID:
      func(static_cast<inputCredentialsGooglePay &>(obj));
      return true;
    case inputFileId::ID:
      func(static_cast<inputFileId &>(obj));
      return true;
    case inputFileRemote::ID:
      func(static_cast<inputFileRemote &>(obj));
      return true;
    case inputFileLocal::ID:
      func(static_cast<inputFileLocal &>(obj));
      return true;
    case inputFileGenerated::ID:
      func(static_cast<inputFileGenerated &>(obj));
      return true;
    case inputIdentityDocument::ID:
      func(static_cast<inputIdentityDocument &>(obj));
      return true;
    case inputInlineQueryResultAnimation::ID:
      func(static_cast<inputInlineQueryResultAnimation &>(obj));
      return true;
    case inputInlineQueryResultArticle::ID:
      func(static_cast<inputInlineQueryResultArticle &>(obj));
      return true;
    case inputInlineQueryResultAudio::ID:
      func(static_cast<inputInlineQueryResultAudio &>(obj));
      return true;
    case inputInlineQueryResultContact::ID:
      func(static_cast<inputInlineQueryResultContact &>(obj));
      return true;
    case inputInlineQueryResultDocument::ID:
      func(static_cast<inputInlineQueryResultDocument &>(obj));
      return true;
    case inputInlineQueryResultGame::ID:
      func(static_cast<inputInlineQueryResultGame &>(obj));
      return true;
    case inputInlineQueryResultLocation::ID:
      func(static_cast<inputInlineQueryResultLocation &>(obj));
      return true;
    case inputInlineQueryResultPhoto::ID:
      func(static_cast<inputInlineQueryResultPhoto &>(obj));
      return true;
    case inputInlineQueryResultSticker::ID:
      func(static_cast<inputInlineQueryResultSticker &>(obj));
      return true;
    case inputInlineQueryResultVenue::ID:
      func(static_cast<inputInlineQueryResultVenue &>(obj));
      return true;
    case inputInlineQueryResultVideo::ID:
      func(static_cast<inputInlineQueryResultVideo &>(obj));
      return true;
    case inputInlineQueryResultVoiceNote::ID:
      func(static_cast<inputInlineQueryResultVoiceNote &>(obj));
      return true;
    case inputInvoiceMessage::ID:
      func(static_cast<inputInvoiceMessage &>(obj));
      return true;
    case inputInvoiceName::ID:
      func(static_cast<inputInvoiceName &>(obj));
      return true;
    case inputInvoiceTelegram::ID:
      func(static_cast<inputInvoiceTelegram &>(obj));
      return true;
    case inputMessageText::ID:
      func(static_cast<inputMessageText &>(obj));
      return true;
    case inputMessageAnimation::ID:
      func(static_cast<inputMessageAnimation &>(obj));
      return true;
    case inputMessageAudio::ID:
      func(static_cast<inputMessageAudio &>(obj));
      return true;
    case inputMessageDocument::ID:
      func(static_cast<inputMessageDocument &>(obj));
      return true;
    case inputMessagePaidMedia::ID:
      func(static_cast<inputMessagePaidMedia &>(obj));
      return true;
    case inputMessagePhoto::ID:
      func(static_cast<inputMessagePhoto &>(obj));
      return true;
    case inputMessageSticker::ID:
      func(static_cast<inputMessageSticker &>(obj));
      return true;
    case inputMessageVideo::ID:
      func(static_cast<inputMessageVideo &>(obj));
      return true;
    case inputMessageVideoNote::ID:
      func(static_cast<inputMessageVideoNote &>(obj));
      return true;
    case inputMessageVoiceNote::ID:
      func(static_cast<inputMessageVoiceNote &>(obj));
      return true;
    case inputMessageLocation::ID:
      func(static_cast<inputMessageLocation &>(obj));
      return true;
    case inputMessageVenue::ID:
      func(static_cast<inputMessageVenue &>(obj));
      return true;
    case inputMessageContact::ID:
      func(static_cast<inputMessageContact &>(obj));
      return true;
    case inputMessageDice::ID:
      func(static_cast<inputMessageDice &>(obj));
      return true;
    case inputMessageGame::ID:
      func(static_cast<inputMessageGame &>(obj));
      return true;
    case inputMessageInvoice::ID:
      func(static_cast<inputMessageInvoice &>(obj));
      return true;
    case inputMessagePoll::ID:
      func(static_cast<inputMessagePoll &>(obj));
      return true;
    case inputMessageStory::ID:
      func(static_cast<inputMessageStory &>(obj));
      return true;
    case inputMessageForwarded::ID:
      func(static_cast<inputMessageForwarded &>(obj));
      return true;
    case inputMessageReplyToMessage::ID:
      func(static_cast<inputMessageReplyToMessage &>(obj));
      return true;
    case inputMessageReplyToExternalMessage::ID:
      func(static_cast<inputMessageReplyToExternalMessage &>(obj));
      return true;
    case inputMessageReplyToStory::ID:
      func(static_cast<inputMessageReplyToStory &>(obj));
      return true;
    case inputPaidMedia::ID:
      func(static_cast<inputPaidMedia &>(obj));
      return true;
    case inputPaidMediaTypePhoto::ID:
      func(static_cast<inputPaidMediaTypePhoto &>(obj));
      return true;
    case inputPaidMediaTypeVideo::ID:
      func(static_cast<inputPaidMediaTypeVideo &>(obj));
      return true;
    case inputPassportElementPersonalDetails::ID:
      func(static_cast<inputPassportElementPersonalDetails &>(obj));
      return true;
    case inputPassportElementPassport::ID:
      func(static_cast<inputPassportElementPassport &>(obj));
      return true;
    case inputPassportElementDriverLicense::ID:
      func(static_cast<inputPassportElementDriverLicense &>(obj));
      return true;
    case inputPassportElementIdentityCard::ID:
      func(static_cast<inputPassportElementIdentityCard &>(obj));
      return true;
    case inputPassportElementInternalPassport::ID:
      func(static_cast<inputPassportElementInternalPassport &>(obj));
      return true;
    case inputPassportElementAddress::ID:
      func(static_cast<inputPassportElementAddress &>(obj));
      return true;
    case inputPassportElementUtilityBill::ID:
      func(static_cast<inputPassportElementUtilityBill &>(obj));
      return true;
    case inputPassportElementBankStatement::ID:
      func(static_cast<inputPassportElementBankStatement &>(obj));
      return true;
    case inputPassportElementRentalAgreement::ID:
      func(static_cast<inputPassportElementRentalAgreement &>(obj));
      return true;
    case inputPassportElementPassportRegistration::ID:
      func(static_cast<inputPassportElementPassportRegistration &>(obj));
      return true;
    case inputPassportElementTemporaryRegistration::ID:
      func(static_cast<inputPassportElementTemporaryRegistration &>(obj));
      return true;
    case inputPassportElementPhoneNumber::ID:
      func(static_cast<inputPassportElementPhoneNumber &>(obj));
      return true;
    case inputPassportElementEmailAddress::ID:
      func(static_cast<inputPassportElementEmailAddress &>(obj));
      return true;
    case inputPassportElementError::ID:
      func(static_cast<inputPassportElementError &>(obj));
      return true;
    case inputPassportElementErrorSourceUnspecified::ID:
      func(static_cast<inputPassportElementErrorSourceUnspecified &>(obj));
      return true;
    case inputPassportElementErrorSourceDataField::ID:
      func(static_cast<inputPassportElementErrorSourceDataField &>(obj));
      return true;
    case inputPassportElementErrorSourceFrontSide::ID:
      func(static_cast<inputPassportElementErrorSourceFrontSide &>(obj));
      return true;
    case inputPassportElementErrorSourceReverseSide::ID:
      func(static_cast<inputPassportElementErrorSourceReverseSide &>(obj));
      return true;
    case inputPassportElementErrorSourceSelfie::ID:
      func(static_cast<inputPassportElementErrorSourceSelfie &>(obj));
      return true;
    case inputPassportElementErrorSourceTranslationFile::ID:
      func(static_cast<inputPassportElementErrorSourceTranslationFile &>(obj));
      return true;
    case inputPassportElementErrorSourceTranslationFiles::ID:
      func(static_cast<inputPassportElementErrorSourceTranslationFiles &>(obj));
      return true;
    case inputPassportElementErrorSourceFile::ID:
      func(static_cast<inputPassportElementErrorSourceFile &>(obj));
      return true;
    case inputPassportElementErrorSourceFiles::ID:
      func(static_cast<inputPassportElementErrorSourceFiles &>(obj));
      return true;
    case inputPersonalDocument::ID:
      func(static_cast<inputPersonalDocument &>(obj));
      return true;
    case inputSticker::ID:
      func(static_cast<inputSticker &>(obj));
      return true;
    case inputStoryArea::ID:
      func(static_cast<inputStoryArea &>(obj));
      return true;
    case inputStoryAreaTypeLocation::ID:
      func(static_cast<inputStoryAreaTypeLocation &>(obj));
      return true;
    case inputStoryAreaTypeFoundVenue::ID:
      func(static_cast<inputStoryAreaTypeFoundVenue &>(obj));
      return true;
    case inputStoryAreaTypePreviousVenue::ID:
      func(static_cast<inputStoryAreaTypePreviousVenue &>(obj));
      return true;
    case inputStoryAreaTypeSuggestedReaction::ID:
      func(static_cast<inputStoryAreaTypeSuggestedReaction &>(obj));
      return true;
    case inputStoryAreaTypeMessage::ID:
      func(static_cast<inputStoryAreaTypeMessage &>(obj));
      return true;
    case inputStoryAreaTypeLink::ID:
      func(static_cast<inputStoryAreaTypeLink &>(obj));
      return true;
    case inputStoryAreaTypeWeather::ID:
      func(static_cast<inputStoryAreaTypeWeather &>(obj));
      return true;
    case inputStoryAreas::ID:
      func(static_cast<inputStoryAreas &>(obj));
      return true;
    case inputStoryContentPhoto::ID:
      func(static_cast<inputStoryContentPhoto &>(obj));
      return true;
    case inputStoryContentVideo::ID:
      func(static_cast<inputStoryContentVideo &>(obj));
      return true;
    case inputTextQuote::ID:
      func(static_cast<inputTextQuote &>(obj));
      return true;
    case inputThumbnail::ID:
      func(static_cast<inputThumbnail &>(obj));
      return true;
    case internalLinkTypeActiveSessions::ID:
      func(static_cast<internalLinkTypeActiveSessions &>(obj));
      return true;
    case internalLinkTypeAttachmentMenuBot::ID:
      func(static_cast<internalLinkTypeAttachmentMenuBot &>(obj));
      return true;
    case internalLinkTypeAuthenticationCode::ID:
      func(static_cast<internalLinkTypeAuthenticationCode &>(obj));
      return true;
    case internalLinkTypeBackground::ID:
      func(static_cast<internalLinkTypeBackground &>(obj));
      return true;
    case internalLinkTypeBotAddToChannel::ID:
      func(static_cast<internalLinkTypeBotAddToChannel &>(obj));
      return true;
    case internalLinkTypeBotStart::ID:
      func(static_cast<internalLinkTypeBotStart &>(obj));
      return true;
    case internalLinkTypeBotStartInGroup::ID:
      func(static_cast<internalLinkTypeBotStartInGroup &>(obj));
      return true;
    case internalLinkTypeBusinessChat::ID:
      func(static_cast<internalLinkTypeBusinessChat &>(obj));
      return true;
    case internalLinkTypeBuyStars::ID:
      func(static_cast<internalLinkTypeBuyStars &>(obj));
      return true;
    case internalLinkTypeChangePhoneNumber::ID:
      func(static_cast<internalLinkTypeChangePhoneNumber &>(obj));
      return true;
    case internalLinkTypeChatBoost::ID:
      func(static_cast<internalLinkTypeChatBoost &>(obj));
      return true;
    case internalLinkTypeChatFolderInvite::ID:
      func(static_cast<internalLinkTypeChatFolderInvite &>(obj));
      return true;
    case internalLinkTypeChatFolderSettings::ID:
      func(static_cast<internalLinkTypeChatFolderSettings &>(obj));
      return true;
    case internalLinkTypeChatInvite::ID:
      func(static_cast<internalLinkTypeChatInvite &>(obj));
      return true;
    case internalLinkTypeDefaultMessageAutoDeleteTimerSettings::ID:
      func(static_cast<internalLinkTypeDefaultMessageAutoDeleteTimerSettings &>(obj));
      return true;
    case internalLinkTypeEditProfileSettings::ID:
      func(static_cast<internalLinkTypeEditProfileSettings &>(obj));
      return true;
    case internalLinkTypeGame::ID:
      func(static_cast<internalLinkTypeGame &>(obj));
      return true;
    case internalLinkTypeInstantView::ID:
      func(static_cast<internalLinkTypeInstantView &>(obj));
      return true;
    case internalLinkTypeInvoice::ID:
      func(static_cast<internalLinkTypeInvoice &>(obj));
      return true;
    case internalLinkTypeLanguagePack::ID:
      func(static_cast<internalLinkTypeLanguagePack &>(obj));
      return true;
    case internalLinkTypeLanguageSettings::ID:
      func(static_cast<internalLinkTypeLanguageSettings &>(obj));
      return true;
    case internalLinkTypeMainWebApp::ID:
      func(static_cast<internalLinkTypeMainWebApp &>(obj));
      return true;
    case internalLinkTypeMessage::ID:
      func(static_cast<internalLinkTypeMessage &>(obj));
      return true;
    case internalLinkTypeMessageDraft::ID:
      func(static_cast<internalLinkTypeMessageDraft &>(obj));
      return true;
    case internalLinkTypePassportDataRequest::ID:
      func(static_cast<internalLinkTypePassportDataRequest &>(obj));
      return true;
    case internalLinkTypePhoneNumberConfirmation::ID:
      func(static_cast<internalLinkTypePhoneNumberConfirmation &>(obj));
      return true;
    case internalLinkTypePremiumFeatures::ID:
      func(static_cast<internalLinkTypePremiumFeatures &>(obj));
      return true;
    case internalLinkTypePremiumGift::ID:
      func(static_cast<internalLinkTypePremiumGift &>(obj));
      return true;
    case internalLinkTypePremiumGiftCode::ID:
      func(static_cast<internalLinkTypePremiumGiftCode &>(obj));
      return true;
    case internalLinkTypePrivacyAndSecuritySettings::ID:
      func(static_cast<internalLinkTypePrivacyAndSecuritySettings &>(obj));
      return true;
    case internalLinkTypeProxy::ID:
      func(static_cast<internalLinkTypeProxy &>(obj));
      return true;
    case internalLinkTypePublicChat::ID:
      func(static_cast<internalLinkTypePublicChat &>(obj));
      return true;
    case internalLinkTypeQrCodeAuthentication::ID:
      func(static_cast<internalLinkTypeQrCodeAuthentication &>(obj));
      return true;
    case internalLinkTypeRestorePurchases::ID:
      func(static_cast<internalLinkTypeRestorePurchases &>(obj));
      return true;
    case internalLinkTypeSettings::ID:
      func(static_cast<internalLinkTypeSettings &>(obj));
      return true;
    case internalLinkTypeStickerSet::ID:
      func(static_cast<internalLinkTypeStickerSet &>(obj));
      return true;
    case internalLinkTypeStory::ID:
      func(static_cast<internalLinkTypeStory &>(obj));
      return true;
    case internalLinkTypeTheme::ID:
      func(static_cast<internalLinkTypeTheme &>(obj));
      return true;
    case internalLinkTypeThemeSettings::ID:
      func(static_cast<internalLinkTypeThemeSettings &>(obj));
      return true;
    case internalLinkTypeUnknownDeepLink::ID:
      func(static_cast<internalLinkTypeUnknownDeepLink &>(obj));
      return true;
    case internalLinkTypeUnsupportedProxy::ID:
      func(static_cast<internalLinkTypeUnsupportedProxy &>(obj));
      return true;
    case internalLinkTypeUserPhoneNumber::ID:
      func(static_cast<internalLinkTypeUserPhoneNumber &>(obj));
      return true;
    case internalLinkTypeUserToken::ID:
      func(static_cast<internalLinkTypeUserToken &>(obj));
      return true;
    case internalLinkTypeVideoChat::ID:
      func(static_cast<internalLinkTypeVideoChat &>(obj));
      return true;
    case internalLinkTypeWebApp::ID:
      func(static_cast<internalLinkTypeWebApp &>(obj));
      return true;
    case inviteLinkChatTypeBasicGroup::ID:
      func(static_cast<inviteLinkChatTypeBasicGroup &>(obj));
      return true;
    case inviteLinkChatTypeSupergroup::ID:
      func(static_cast<inviteLinkChatTypeSupergroup &>(obj));
      return true;
    case inviteLinkChatTypeChannel::ID:
      func(static_cast<inviteLinkChatTypeChannel &>(obj));
      return true;
    case invoice::ID:
      func(static_cast<invoice &>(obj));
      return true;
    case jsonObjectMember::ID:
      func(static_cast<jsonObjectMember &>(obj));
      return true;
    case jsonValueNull::ID:
      func(static_cast<jsonValueNull &>(obj));
      return true;
    case jsonValueBoolean::ID:
      func(static_cast<jsonValueBoolean &>(obj));
      return true;
    case jsonValueNumber::ID:
      func(static_cast<jsonValueNumber &>(obj));
      return true;
    case jsonValueString::ID:
      func(static_cast<jsonValueString &>(obj));
      return true;
    case jsonValueArray::ID:
      func(static_cast<jsonValueArray &>(obj));
      return true;
    case jsonValueObject::ID:
      func(static_cast<jsonValueObject &>(obj));
      return true;
    case keyboardButton::ID:
      func(static_cast<keyboardButton &>(obj));
      return true;
    case keyboardButtonTypeText::ID:
      func(static_cast<keyboardButtonTypeText &>(obj));
      return true;
    case keyboardButtonTypeRequestPhoneNumber::ID:
      func(static_cast<keyboardButtonTypeRequestPhoneNumber &>(obj));
      return true;
    case keyboardButtonTypeRequestLocation::ID:
      func(static_cast<keyboardButtonTypeRequestLocation &>(obj));
      return true;
    case keyboardButtonTypeRequestPoll::ID:
      func(static_cast<keyboardButtonTypeRequestPoll &>(obj));
      return true;
    case keyboardButtonTypeRequestUsers::ID:
      func(static_cast<keyboardButtonTypeRequestUsers &>(obj));
      return true;
    case keyboardButtonTypeRequestChat::ID:
      func(static_cast<keyboardButtonTypeRequestChat &>(obj));
      return true;
    case keyboardButtonTypeWebApp::ID:
      func(static_cast<keyboardButtonTypeWebApp &>(obj));
      return true;
    case labeledPricePart::ID:
      func(static_cast<labeledPricePart &>(obj));
      return true;
    case languagePackInfo::ID:
      func(static_cast<languagePackInfo &>(obj));
      return true;
    case languagePackString::ID:
      func(static_cast<languagePackString &>(obj));
      return true;
    case languagePackStringValueOrdinary::ID:
      func(static_cast<languagePackStringValueOrdinary &>(obj));
      return true;
    case languagePackStringValuePluralized::ID:
      func(static_cast<languagePackStringValuePluralized &>(obj));
      return true;
    case languagePackStringValueDeleted::ID:
      func(static_cast<languagePackStringValueDeleted &>(obj));
      return true;
    case languagePackStrings::ID:
      func(static_cast<languagePackStrings &>(obj));
      return true;
    case linkPreview::ID:
      func(static_cast<linkPreview &>(obj));
      return true;
    case linkPreviewAlbumMediaPhoto::ID:
      func(static_cast<linkPreviewAlbumMediaPhoto &>(obj));
      return true;
    case linkPreviewAlbumMediaVideo::ID:
      func(static_cast<linkPreviewAlbumMediaVideo &>(obj));
      return true;
    case linkPreviewOptions::ID:
      func(static_cast<linkPreviewOptions &>(obj));
      return true;
    case linkPreviewTypeAlbum::ID:
      func(static_cast<linkPreviewTypeAlbum &>(obj));
      return true;
    case linkPreviewTypeAnimation::ID:
      func(static_cast<linkPreviewTypeAnimation &>(obj));
      return true;
    case linkPreviewTypeApp::ID:
      func(static_cast<linkPreviewTypeApp &>(obj));
      return true;
    case linkPreviewTypeArticle::ID:
      func(static_cast<linkPreviewTypeArticle &>(obj));
      return true;
    case linkPreviewTypeAudio::ID:
      func(static_cast<linkPreviewTypeAudio &>(obj));
      return true;
    case linkPreviewTypeBackground::ID:
      func(static_cast<linkPreviewTypeBackground &>(obj));
      return true;
    case linkPreviewTypeChannelBoost::ID:
      func(static_cast<linkPreviewTypeChannelBoost &>(obj));
      return true;
    case linkPreviewTypeChat::ID:
      func(static_cast<linkPreviewTypeChat &>(obj));
      return true;
    case linkPreviewTypeDocument::ID:
      func(static_cast<linkPreviewTypeDocument &>(obj));
      return true;
    case linkPreviewTypeEmbeddedAnimationPlayer::ID:
      func(static_cast<linkPreviewTypeEmbeddedAnimationPlayer &>(obj));
      return true;
    case linkPreviewTypeEmbeddedAudioPlayer::ID:
      func(static_cast<linkPreviewTypeEmbeddedAudioPlayer &>(obj));
      return true;
    case linkPreviewTypeEmbeddedVideoPlayer::ID:
      func(static_cast<linkPreviewTypeEmbeddedVideoPlayer &>(obj));
      return true;
    case linkPreviewTypeExternalAudio::ID:
      func(static_cast<linkPreviewTypeExternalAudio &>(obj));
      return true;
    case linkPreviewTypeExternalVideo::ID:
      func(static_cast<linkPreviewTypeExternalVideo &>(obj));
      return true;
    case linkPreviewTypeInvoice::ID:
      func(static_cast<linkPreviewTypeInvoice &>(obj));
      return true;
    case linkPreviewTypeMessage::ID:
      func(static_cast<linkPreviewTypeMessage &>(obj));
      return true;
    case linkPreviewTypePhoto::ID:
      func(static_cast<linkPreviewTypePhoto &>(obj));
      return true;
    case linkPreviewTypePremiumGiftCode::ID:
      func(static_cast<linkPreviewTypePremiumGiftCode &>(obj));
      return true;
    case linkPreviewTypeShareableChatFolder::ID:
      func(static_cast<linkPreviewTypeShareableChatFolder &>(obj));
      return true;
    case linkPreviewTypeSticker::ID:
      func(static_cast<linkPreviewTypeSticker &>(obj));
      return true;
    case linkPreviewTypeStickerSet::ID:
      func(static_cast<linkPreviewTypeStickerSet &>(obj));
      return true;
    case linkPreviewTypeStory::ID:
      func(static_cast<linkPreviewTypeStory &>(obj));
      return true;
    case linkPreviewTypeSupergroupBoost::ID:
      func(static_cast<linkPreviewTypeSupergroupBoost &>(obj));
      return true;
    case linkPreviewTypeTheme::ID:
      func(static_cast<linkPreviewTypeTheme &>(obj));
      return true;
    case linkPreviewTypeUnsupported::ID:
      func(static_cast<linkPreviewTypeUnsupported &>(obj));
      return true;
    case linkPreviewTypeUser::ID:
      func(static_cast<linkPreviewTypeUser &>(obj));
      return true;
    case linkPreviewTypeVideo::ID:
      func(static_cast<linkPreviewTypeVideo &>(obj));
      return true;
    case linkPreviewTypeVideoChat::ID:
      func(static_cast<linkPreviewTypeVideoChat &>(obj));
      return true;
    case linkPreviewTypeVideoNote::ID:
      func(static_cast<linkPreviewTypeVideoNote &>(obj));
      return true;
    case linkPreviewTypeVoiceNote::ID:
      func(static_cast<linkPreviewTypeVoiceNote &>(obj));
      return true;
    case linkPreviewTypeWebApp::ID:
      func(static_cast<linkPreviewTypeWebApp &>(obj));
      return true;
    case localFile::ID:
      func(static_cast<localFile &>(obj));
      return true;
    case localizationTargetInfo::ID:
      func(static_cast<localizationTargetInfo &>(obj));
      return true;
    case location::ID:
      func(static_cast<location &>(obj));
      return true;
    case locationAddress::ID:
      func(static_cast<locationAddress &>(obj));
      return true;
    case logStreamDefault::ID:
      func(static_cast<logStreamDefault &>(obj));
      return true;
    case logStreamFile::ID:
      func(static_cast<logStreamFile &>(obj));
      return true;
    case logStreamEmpty::ID:
      func(static_cast<logStreamEmpty &>(obj));
      return true;
    case logTags::ID:
      func(static_cast<logTags &>(obj));
      return true;
    case logVerbosityLevel::ID:
      func(static_cast<logVerbosityLevel &>(obj));
      return true;
    case loginUrlInfoOpen::ID:
      func(static_cast<loginUrlInfoOpen &>(obj));
      return true;
    case loginUrlInfoRequestConfirmation::ID:
      func(static_cast<loginUrlInfoRequestConfirmation &>(obj));
      return true;
    case mainWebApp::ID:
      func(static_cast<mainWebApp &>(obj));
      return true;
    case maskPointForehead::ID:
      func(static_cast<maskPointForehead &>(obj));
      return true;
    case maskPointEyes::ID:
      func(static_cast<maskPointEyes &>(obj));
      return true;
    case maskPointMouth::ID:
      func(static_cast<maskPointMouth &>(obj));
      return true;
    case maskPointChin::ID:
      func(static_cast<maskPointChin &>(obj));
      return true;
    case maskPosition::ID:
      func(static_cast<maskPosition &>(obj));
      return true;
    case message::ID:
      func(static_cast<message &>(obj));
      return true;
    case messageAutoDeleteTime::ID:
      func(static_cast<messageAutoDeleteTime &>(obj));
      return true;
    case messageCalendar::ID:
      func(static_cast<messageCalendar &>(obj));
      return true;
    case messageCalendarDay::ID:
      func(static_cast<messageCalendarDay &>(obj));
      return true;
    case messageText::ID:
      func(static_cast<messageText &>(obj));
      return true;
    case messageAnimation::ID:
      func(static_cast<messageAnimation &>(obj));
      return true;
    case messageAudio::ID:
      func(static_cast<messageAudio &>(obj));
      return true;
    case messageDocument::ID:
      func(static_cast<messageDocument &>(obj));
      return true;
    case messagePaidMedia::ID:
      func(static_cast<messagePaidMedia &>(obj));
      return true;
    case messagePhoto::ID:
      func(static_cast<messagePhoto &>(obj));
      return true;
    case messageSticker::ID:
      func(static_cast<messageSticker &>(obj));
      return true;
    case messageVideo::ID:
      func(static_cast<messageVideo &>(obj));
      return true;
    case messageVideoNote::ID:
      func(static_cast<messageVideoNote &>(obj));
      return true;
    case messageVoiceNote::ID:
      func(static_cast<messageVoiceNote &>(obj));
      return true;
    case messageExpiredPhoto::ID:
      func(static_cast<messageExpiredPhoto &>(obj));
      return true;
    case messageExpiredVideo::ID:
      func(static_cast<messageExpiredVideo &>(obj));
      return true;
    case messageExpiredVideoNote::ID:
      func(static_cast<messageExpiredVideoNote &>(obj));
      return true;
    case messageExpiredVoiceNote::ID:
      func(static_cast<messageExpiredVoiceNote &>(obj));
      return true;
    case messageLocation::ID:
      func(static_cast<messageLocation &>(obj));
      return true;
    case messageVenue::ID:
      func(static_cast<messageVenue &>(obj));
      return true;
    case messageContact::ID:
      func(static_cast<messageContact &>(obj));
      return true;
    case messageAnimatedEmoji::ID:
      func(static_cast<messageAnimatedEmoji &>(obj));
      return true;
    case messageDice::ID:
      func(static_cast<messageDice &>(obj));
      return true;
    case messageGame::ID:
      func(static_cast<messageGame &>(obj));
      return true;
    case messagePoll::ID:
      func(static_cast<messagePoll &>(obj));
      return true;
    case messageStory::ID:
      func(static_cast<messageStory &>(obj));
      return true;
    case messageInvoice::ID:
      func(static_cast<messageInvoice &>(obj));
      return true;
    case messageCall::ID:
      func(static_cast<messageCall &>(obj));
      return true;
    case messageVideoChatScheduled::ID:
      func(static_cast<messageVideoChatScheduled &>(obj));
      return true;
    case messageVideoChatStarted::ID:
      func(static_cast<messageVideoChatStarted &>(obj));
      return true;
    case messageVideoChatEnded::ID:
      func(static_cast<messageVideoChatEnded &>(obj));
      return true;
    case messageInviteVideoChatParticipants::ID:
      func(static_cast<messageInviteVideoChatParticipants &>(obj));
      return true;
    case messageBasicGroupChatCreate::ID:
      func(static_cast<messageBasicGroupChatCreate &>(obj));
      return true;
    case messageSupergroupChatCreate::ID:
      func(static_cast<messageSupergroupChatCreate &>(obj));
      return true;
    case messageChatChangeTitle::ID:
      func(static_cast<messageChatChangeTitle &>(obj));
      return true;
    case messageChatChangePhoto::ID:
      func(static_cast<messageChatChangePhoto &>(obj));
      return true;
    case messageChatDeletePhoto::ID:
      func(static_cast<messageChatDeletePhoto &>(obj));
      return true;
    case messageChatAddMembers::ID:
      func(static_cast<messageChatAddMembers &>(obj));
      return true;
    case messageChatJoinByLink::ID:
      func(static_cast<messageChatJoinByLink &>(obj));
      return true;
    case messageChatJoinByRequest::ID:
      func(static_cast<messageChatJoinByRequest &>(obj));
      return true;
    case messageChatDeleteMember::ID:
      func(static_cast<messageChatDeleteMember &>(obj));
      return true;
    case messageChatUpgradeTo::ID:
      func(static_cast<messageChatUpgradeTo &>(obj));
      return true;
    case messageChatUpgradeFrom::ID:
      func(static_cast<messageChatUpgradeFrom &>(obj));
      return true;
    case messagePinMessage::ID:
      func(static_cast<messagePinMessage &>(obj));
      return true;
    case messageScreenshotTaken::ID:
      func(static_cast<messageScreenshotTaken &>(obj));
      return true;
    case messageChatSetBackground::ID:
      func(static_cast<messageChatSetBackground &>(obj));
      return true;
    case messageChatSetTheme::ID:
      func(static_cast<messageChatSetTheme &>(obj));
      return true;
    case messageChatSetMessageAutoDeleteTime::ID:
      func(static_cast<messageChatSetMessageAutoDeleteTime &>(obj));
      return true;
    case messageChatBoost::ID:
      func(static_cast<messageChatBoost &>(obj));
      return true;
    case messageForumTopicCreated::ID:
      func(static_cast<messageForumTopicCreated &>(obj));
      return true;
    case messageForumTopicEdited::ID:
      func(static_cast<messageForumTopicEdited &>(obj));
      return true;
    case messageForumTopicIsClosedToggled::ID:
      func(static_cast<messageForumTopicIsClosedToggled &>(obj));
      return true;
    case messageForumTopicIsHiddenToggled::ID:
      func(static_cast<messageForumTopicIsHiddenToggled &>(obj));
      return true;
    case messageSuggestProfilePhoto::ID:
      func(static_cast<messageSuggestProfilePhoto &>(obj));
      return true;
    case messageCustomServiceAction::ID:
      func(static_cast<messageCustomServiceAction &>(obj));
      return true;
    case messageGameScore::ID:
      func(static_cast<messageGameScore &>(obj));
      return true;
    case messagePaymentSuccessful::ID:
      func(static_cast<messagePaymentSuccessful &>(obj));
      return true;
    case messagePaymentSuccessfulBot::ID:
      func(static_cast<messagePaymentSuccessfulBot &>(obj));
      return true;
    case messagePaymentRefunded::ID:
      func(static_cast<messagePaymentRefunded &>(obj));
      return true;
    case messageGiftedPremium::ID:
      func(static_cast<messageGiftedPremium &>(obj));
      return true;
    case messagePremiumGiftCode::ID:
      func(static_cast<messagePremiumGiftCode &>(obj));
      return true;
    case messageGiveawayCreated::ID:
      func(static_cast<messageGiveawayCreated &>(obj));
      return true;
    case messageGiveaway::ID:
      func(static_cast<messageGiveaway &>(obj));
      return true;
    case messageGiveawayCompleted::ID:
      func(static_cast<messageGiveawayCompleted &>(obj));
      return true;
    case messageGiveawayWinners::ID:
      func(static_cast<messageGiveawayWinners &>(obj));
      return true;
    case messageGiftedStars::ID:
      func(static_cast<messageGiftedStars &>(obj));
      return true;
    case messageGiveawayPrizeStars::ID:
      func(static_cast<messageGiveawayPrizeStars &>(obj));
      return true;
    case messageContactRegistered::ID:
      func(static_cast<messageContactRegistered &>(obj));
      return true;
    case messageUsersShared::ID:
      func(static_cast<messageUsersShared &>(obj));
      return true;
    case messageChatShared::ID:
      func(static_cast<messageChatShared &>(obj));
      return true;
    case messageBotWriteAccessAllowed::ID:
      func(static_cast<messageBotWriteAccessAllowed &>(obj));
      return true;
    case messageWebAppDataSent::ID:
      func(static_cast<messageWebAppDataSent &>(obj));
      return true;
    case messageWebAppDataReceived::ID:
      func(static_cast<messageWebAppDataReceived &>(obj));
      return true;
    case messagePassportDataSent::ID:
      func(static_cast<messagePassportDataSent &>(obj));
      return true;
    case messagePassportDataReceived::ID:
      func(static_cast<messagePassportDataReceived &>(obj));
      return true;
    case messageProximityAlertTriggered::ID:
      func(static_cast<messageProximityAlertTriggered &>(obj));
      return true;
    case messageUnsupported::ID:
      func(static_cast<messageUnsupported &>(obj));
      return true;
    case messageCopyOptions::ID:
      func(static_cast<messageCopyOptions &>(obj));
      return true;
    case messageEffect::ID:
      func(static_cast<messageEffect &>(obj));
      return true;
    case messageEffectTypeEmojiReaction::ID:
      func(static_cast<messageEffectTypeEmojiReaction &>(obj));
      return true;
    case messageEffectTypePremiumSticker::ID:
      func(static_cast<messageEffectTypePremiumSticker &>(obj));
      return true;
    case messageFileTypePrivate::ID:
      func(static_cast<messageFileTypePrivate &>(obj));
      return true;
    case messageFileTypeGroup::ID:
      func(static_cast<messageFileTypeGroup &>(obj));
      return true;
    case messageFileTypeUnknown::ID:
      func(static_cast<messageFileTypeUnknown &>(obj));
      return true;
    case messageForwardInfo::ID:
      func(static_cast<messageForwardInfo &>(obj));
      return true;
    case messageImportInfo::ID:
      func(static_cast<messageImportInfo &>(obj));
      return true;
    case messageInteractionInfo::ID:
      func(static_cast<messageInteractionInfo &>(obj));
      return true;
    case messageLink::ID:
      func(static_cast<messageLink &>(obj));
      return true;
    case messageLinkInfo::ID:
      func(static_cast<messageLinkInfo &>(obj));
      return true;
    case messageOriginUser::ID:
      func(static_cast<messageOriginUser &>(obj));
      return true;
    case messageOriginHiddenUser::ID:
      func(static_cast<messageOriginHiddenUser &>(obj));
      return true;
    case messageOriginChat::ID:
      func(static_cast<messageOriginChat &>(obj));
      return true;
    case messageOriginChannel::ID:
      func(static_cast<messageOriginChannel &>(obj));
      return true;
    case messagePosition::ID:
      func(static_cast<messagePosition &>(obj));
      return true;
    case messagePositions::ID:
      func(static_cast<messagePositions &>(obj));
      return true;
    case messageProperties::ID:
      func(static_cast<messageProperties &>(obj));
      return true;
    case messageReaction::ID:
      func(static_cast<messageReaction &>(obj));
      return true;
    case messageReactions::ID:
      func(static_cast<messageReactions &>(obj));
      return true;
    case messageReadDateRead::ID:
      func(static_cast<messageReadDateRead &>(obj));
      return true;
    case messageReadDateUnread::ID:
      func(static_cast<messageReadDateUnread &>(obj));
      return true;
    case messageReadDateTooOld::ID:
      func(static_cast<messageReadDateTooOld &>(obj));
      return true;
    case messageReadDateUserPrivacyRestricted::ID:
      func(static_cast<messageReadDateUserPrivacyRestricted &>(obj));
      return true;
    case messageReadDateMyPrivacyRestricted::ID:
      func(static_cast<messageReadDateMyPrivacyRestricted &>(obj));
      return true;
    case messageReplyInfo::ID:
      func(static_cast<messageReplyInfo &>(obj));
      return true;
    case messageReplyToMessage::ID:
      func(static_cast<messageReplyToMessage &>(obj));
      return true;
    case messageReplyToStory::ID:
      func(static_cast<messageReplyToStory &>(obj));
      return true;
    case messageSchedulingStateSendAtDate::ID:
      func(static_cast<messageSchedulingStateSendAtDate &>(obj));
      return true;
    case messageSchedulingStateSendWhenOnline::ID:
      func(static_cast<messageSchedulingStateSendWhenOnline &>(obj));
      return true;
    case messageSelfDestructTypeTimer::ID:
      func(static_cast<messageSelfDestructTypeTimer &>(obj));
      return true;
    case messageSelfDestructTypeImmediately::ID:
      func(static_cast<messageSelfDestructTypeImmediately &>(obj));
      return true;
    case messageSendOptions::ID:
      func(static_cast<messageSendOptions &>(obj));
      return true;
    case messageSenderUser::ID:
      func(static_cast<messageSenderUser &>(obj));
      return true;
    case messageSenderChat::ID:
      func(static_cast<messageSenderChat &>(obj));
      return true;
    case messageSenders::ID:
      func(static_cast<messageSenders &>(obj));
      return true;
    case messageSendingStatePending::ID:
      func(static_cast<messageSendingStatePending &>(obj));
      return true;
    case messageSendingStateFailed::ID:
      func(static_cast<messageSendingStateFailed &>(obj));
      return true;
    case messageSourceChatHistory::ID:
      func(static_cast<messageSourceChatHistory &>(obj));
      return true;
    case messageSourceMessageThreadHistory::ID:
      func(static_cast<messageSourceMessageThreadHistory &>(obj));
      return true;
    case messageSourceForumTopicHistory::ID:
      func(static_cast<messageSourceForumTopicHistory &>(obj));
      return true;
    case messageSourceHistoryPreview::ID:
      func(static_cast<messageSourceHistoryPreview &>(obj));
      return true;
    case messageSourceChatList::ID:
      func(static_cast<messageSourceChatList &>(obj));
      return true;
    case messageSourceSearch::ID:
      func(static_cast<messageSourceSearch &>(obj));
      return true;
    case messageSourceChatEventLog::ID:
      func(static_cast<messageSourceChatEventLog &>(obj));
      return true;
    case messageSourceNotification::ID:
      func(static_cast<messageSourceNotification &>(obj));
      return true;
    case messageSourceScreenshot::ID:
      func(static_cast<messageSourceScreenshot &>(obj));
      return true;
    case messageSourceOther::ID:
      func(static_cast<messageSourceOther &>(obj));
      return true;
    case messageSponsor::ID:
      func(static_cast<messageSponsor &>(obj));
      return true;
    case messageStatistics::ID:
      func(static_cast<messageStatistics &>(obj));
      return true;
    case messageThreadInfo::ID:
      func(static_cast<messageThreadInfo &>(obj));
      return true;
    case messageViewer::ID:
      func(static_cast<messageViewer &>(obj));
      return true;
    case messageViewers::ID:
      func(static_cast<messageViewers &>(obj));
      return true;
    case messages::ID:
      func(static_cast<messages &>(obj));
      return true;
    case minithumbnail::ID:
      func(static_cast<minithumbnail &>(obj));
      return true;
    case networkStatistics::ID:
      func(static_cast<networkStatistics &>(obj));
      return true;
    case networkStatisticsEntryFile::ID:
      func(static_cast<networkStatisticsEntryFile &>(obj));
      return true;
    case networkStatisticsEntryCall::ID:
      func(static_cast<networkStatisticsEntryCall &>(obj));
      return true;
    case networkTypeNone::ID:
      func(static_cast<networkTypeNone &>(obj));
      return true;
    case networkTypeMobile::ID:
      func(static_cast<networkTypeMobile &>(obj));
      return true;
    case networkTypeMobileRoaming::ID:
      func(static_cast<networkTypeMobileRoaming &>(obj));
      return true;
    case networkTypeWiFi::ID:
      func(static_cast<networkTypeWiFi &>(obj));
      return true;
    case networkTypeOther::ID:
      func(static_cast<networkTypeOther &>(obj));
      return true;
    case newChatPrivacySettings::ID:
      func(static_cast<newChatPrivacySettings &>(obj));
      return true;
    case notification::ID:
      func(static_cast<notification &>(obj));
      return true;
    case notificationGroup::ID:
      func(static_cast<notificationGroup &>(obj));
      return true;
    case notificationGroupTypeMessages::ID:
      func(static_cast<notificationGroupTypeMessages &>(obj));
      return true;
    case notificationGroupTypeMentions::ID:
      func(static_cast<notificationGroupTypeMentions &>(obj));
      return true;
    case notificationGroupTypeSecretChat::ID:
      func(static_cast<notificationGroupTypeSecretChat &>(obj));
      return true;
    case notificationGroupTypeCalls::ID:
      func(static_cast<notificationGroupTypeCalls &>(obj));
      return true;
    case notificationSettingsScopePrivateChats::ID:
      func(static_cast<notificationSettingsScopePrivateChats &>(obj));
      return true;
    case notificationSettingsScopeGroupChats::ID:
      func(static_cast<notificationSettingsScopeGroupChats &>(obj));
      return true;
    case notificationSettingsScopeChannelChats::ID:
      func(static_cast<notificationSettingsScopeChannelChats &>(obj));
      return true;
    case notificationSound::ID:
      func(static_cast<notificationSound &>(obj));
      return true;
    case notificationSounds::ID:
      func(static_cast<notificationSounds &>(obj));
      return true;
    case notificationTypeNewMessage::ID:
      func(static_cast<notificationTypeNewMessage &>(obj));
      return true;
    case notificationTypeNewSecretChat::ID:
      func(static_cast<notificationTypeNewSecretChat &>(obj));
      return true;
    case notificationTypeNewCall::ID:
      func(static_cast<notificationTypeNewCall &>(obj));
      return true;
    case notificationTypeNewPushMessage::ID:
      func(static_cast<notificationTypeNewPushMessage &>(obj));
      return true;
    case ok::ID:
      func(static_cast<ok &>(obj));
      return true;
    case optionValueBoolean::ID:
      func(static_cast<optionValueBoolean &>(obj));
      return true;
    case optionValueEmpty::ID:
      func(static_cast<optionValueEmpty &>(obj));
      return true;
    case optionValueInteger::ID:
      func(static_cast<optionValueInteger &>(obj));
      return true;
    case optionValueString::ID:
      func(static_cast<optionValueString &>(obj));
      return true;
    case orderInfo::ID:
      func(static_cast<orderInfo &>(obj));
      return true;
    case pageBlockTitle::ID:
      func(static_cast<pageBlockTitle &>(obj));
      return true;
    case pageBlockSubtitle::ID:
      func(static_cast<pageBlockSubtitle &>(obj));
      return true;
    case pageBlockAuthorDate::ID:
      func(static_cast<pageBlockAuthorDate &>(obj));
      return true;
    case pageBlockHeader::ID:
      func(static_cast<pageBlockHeader &>(obj));
      return true;
    case pageBlockSubheader::ID:
      func(static_cast<pageBlockSubheader &>(obj));
      return true;
    case pageBlockKicker::ID:
      func(static_cast<pageBlockKicker &>(obj));
      return true;
    case pageBlockParagraph::ID:
      func(static_cast<pageBlockParagraph &>(obj));
      return true;
    case pageBlockPreformatted::ID:
      func(static_cast<pageBlockPreformatted &>(obj));
      return true;
    case pageBlockFooter::ID:
      func(static_cast<pageBlockFooter &>(obj));
      return true;
    case pageBlockDivider::ID:
      func(static_cast<pageBlockDivider &>(obj));
      return true;
    case pageBlockAnchor::ID:
      func(static_cast<pageBlockAnchor &>(obj));
      return true;
    case pageBlockList::ID:
      func(static_cast<pageBlockList &>(obj));
      return true;
    case pageBlockBlockQuote::ID:
      func(static_cast<pageBlockBlockQuote &>(obj));
      return true;
    case pageBlockPullQuote::ID:
      func(static_cast<pageBlockPullQuote &>(obj));
      return true;
    case pageBlockAnimation::ID:
      func(static_cast<pageBlockAnimation &>(obj));
      return true;
    case pageBlockAudio::ID:
      func(static_cast<pageBlockAudio &>(obj));
      return true;
    case pageBlockPhoto::ID:
      func(static_cast<pageBlockPhoto &>(obj));
      return true;
    case pageBlockVideo::ID:
      func(static_cast<pageBlockVideo &>(obj));
      return true;
    case pageBlockVoiceNote::ID:
      func(static_cast<pageBlockVoiceNote &>(obj));
      return true;
    case pageBlockCover::ID:
      func(static_cast<pageBlockCover &>(obj));
      return true;
    case pageBlockEmbedded::ID:
      func(static_cast<pageBlockEmbedded &>(obj));
      return true;
    case pageBlockEmbeddedPost::ID:
      func(static_cast<pageBlockEmbeddedPost &>(obj));
      return true;
    case pageBlockCollage::ID:
      func(static_cast<pageBlockCollage &>(obj));
      return true;
    case pageBlockSlideshow::ID:
      func(static_cast<pageBlockSlideshow &>(obj));
      return true;
    case pageBlockChatLink::ID:
      func(static_cast<pageBlockChatLink &>(obj));
      return true;
    case pageBlockTable::ID:
      func(static_cast<pageBlockTable &>(obj));
      return true;
    case pageBlockDetails::ID:
      func(static_cast<pageBlockDetails &>(obj));
      return true;
    case pageBlockRelatedArticles::ID:
      func(static_cast<pageBlockRelatedArticles &>(obj));
      return true;
    case pageBlockMap::ID:
      func(static_cast<pageBlockMap &>(obj));
      return true;
    case pageBlockCaption::ID:
      func(static_cast<pageBlockCaption &>(obj));
      return true;
    case pageBlockHorizontalAlignmentLeft::ID:
      func(static_cast<pageBlockHorizontalAlignmentLeft &>(obj));
      return true;
    case pageBlockHorizontalAlignmentCenter::ID:
      func(static_cast<pageBlockHorizontalAlignmentCenter &>(obj));
      return true;
    case pageBlockHorizontalAlignmentRight::ID:
      func(static_cast<pageBlockHorizontalAlignmentRight &>(obj));
      return true;
    case pageBlockListItem::ID:
      func(static_cast<pageBlockListItem &>(obj));
      return true;
    case pageBlockRelatedArticle::ID:
      func(static_cast<pageBlockRelatedArticle &>(obj));
      return true;
    case pageBlockTableCell::ID:
      func(static_cast<pageBlockTableCell &>(obj));
      return true;
    case pageBlockVerticalAlignmentTop::ID:
      func(static_cast<pageBlockVerticalAlignmentTop &>(obj));
      return true;
    case pageBlockVerticalAlignmentMiddle::ID:
      func(static_cast<pageBlockVerticalAlignmentMiddle &>(obj));
      return true;
    case pageBlockVerticalAlignmentBottom::ID:
      func(static_cast<pageBlockVerticalAlignmentBottom &>(obj));
      return true;
    case paidMediaPreview::ID:
      func(static_cast<paidMediaPreview &>(obj));
      return true;
    case paidMediaPhoto::ID:
      func(static_cast<paidMediaPhoto &>(obj));
      return true;
    case paidMediaVideo::ID:
      func(static_cast<paidMediaVideo &>(obj));
      return true;
    case paidMediaUnsupported::ID:
      func(static_cast<paidMediaUnsupported &>(obj));
      return true;
    case paidReactor::ID:
      func(static_cast<paidReactor &>(obj));
      return true;
    case passportAuthorizationForm::ID:
      func(static_cast<passportAuthorizationForm &>(obj));
      return true;
    case passportElementPersonalDetails::ID:
      func(static_cast<passportElementPersonalDetails &>(obj));
      return true;
    case passportElementPassport::ID:
      func(static_cast<passportElementPassport &>(obj));
      return true;
    case passportElementDriverLicense::ID:
      func(static_cast<passportElementDriverLicense &>(obj));
      return true;
    case passportElementIdentityCard::ID:
      func(static_cast<passportElementIdentityCard &>(obj));
      return true;
    case passportElementInternalPassport::ID:
      func(static_cast<passportElementInternalPassport &>(obj));
      return true;
    case passportElementAddress::ID:
      func(static_cast<passportElementAddress &>(obj));
      return true;
    case passportElementUtilityBill::ID:
      func(static_cast<passportElementUtilityBill &>(obj));
      return true;
    case passportElementBankStatement::ID:
      func(static_cast<passportElementBankStatement &>(obj));
      return true;
    case passportElementRentalAgreement::ID:
      func(static_cast<passportElementRentalAgreement &>(obj));
      return true;
    case passportElementPassportRegistration::ID:
      func(static_cast<passportElementPassportRegistration &>(obj));
      return true;
    case passportElementTemporaryRegistration::ID:
      func(static_cast<passportElementTemporaryRegistration &>(obj));
      return true;
    case passportElementPhoneNumber::ID:
      func(static_cast<passportElementPhoneNumber &>(obj));
      return true;
    case passportElementEmailAddress::ID:
      func(static_cast<passportElementEmailAddress &>(obj));
      return true;
    case passportElementError::ID:
      func(static_cast<passportElementError &>(obj));
      return true;
    case passportElementErrorSourceUnspecified::ID:
      func(static_cast<passportElementErrorSourceUnspecified &>(obj));
      return true;
    case passportElementErrorSourceDataField::ID:
      func(static_cast<passportElementErrorSourceDataField &>(obj));
      return true;
    case passportElementErrorSourceFrontSide::ID:
      func(static_cast<passportElementErrorSourceFrontSide &>(obj));
      return true;
    case passportElementErrorSourceReverseSide::ID:
      func(static_cast<passportElementErrorSourceReverseSide &>(obj));
      return true;
    case passportElementErrorSourceSelfie::ID:
      func(static_cast<passportElementErrorSourceSelfie &>(obj));
      return true;
    case passportElementErrorSourceTranslationFile::ID:
      func(static_cast<passportElementErrorSourceTranslationFile &>(obj));
      return true;
    case passportElementErrorSourceTranslationFiles::ID:
      func(static_cast<passportElementErrorSourceTranslationFiles &>(obj));
      return true;
    case passportElementErrorSourceFile::ID:
      func(static_cast<passportElementErrorSourceFile &>(obj));
      return true;
    case passportElementErrorSourceFiles::ID:
      func(static_cast<passportElementErrorSourceFiles &>(obj));
      return true;
    case passportElementTypePersonalDetails::ID:
      func(static_cast<passportElementTypePersonalDetails &>(obj));
      return true;
    case passportElementTypePassport::ID:
      func(static_cast<passportElementTypePassport &>(obj));
      return true;
    case passportElementTypeDriverLicense::ID:
      func(static_cast<passportElementTypeDriverLicense &>(obj));
      return true;
    case passportElementTypeIdentityCard::ID:
      func(static_cast<passportElementTypeIdentityCard &>(obj));
      return true;
    case passportElementTypeInternalPassport::ID:
      func(static_cast<passportElementTypeInternalPassport &>(obj));
      return true;
    case passportElementTypeAddress::ID:
      func(static_cast<passportElementTypeAddress &>(obj));
      return true;
    case passportElementTypeUtilityBill::ID:
      func(static_cast<passportElementTypeUtilityBill &>(obj));
      return true;
    case passportElementTypeBankStatement::ID:
      func(static_cast<passportElementTypeBankStatement &>(obj));
      return true;
    case passportElementTypeRentalAgreement::ID:
      func(static_cast<passportElementTypeRentalAgreement &>(obj));
      return true;
    case passportElementTypePassportRegistration::ID:
      func(static_cast<passportElementTypePassportRegistration &>(obj));
      return true;
    case passportElementTypeTemporaryRegistration::ID:
      func(static_cast<passportElementTypeTemporaryRegistration &>(obj));
      return true;
    case passportElementTypePhoneNumber::ID:
      func(static_cast<passportElementTypePhoneNumber &>(obj));
      return true;
    case passportElementTypeEmailAddress::ID:
      func(static_cast<passportElementTypeEmailAddress &>(obj));
      return true;
    case passportElements::ID:
      func(static_cast<passportElements &>(obj));
      return true;
    case passportElementsWithErrors::ID:
      func(static_cast<passportElementsWithErrors &>(obj));
      return true;
    case passportRequiredElement::ID:
      func(static_cast<passportRequiredElement &>(obj));
      return true;
    case passportSuitableElement::ID:
      func(static_cast<passportSuitableElement &>(obj));
      return true;
    case passwordState::ID:
      func(static_cast<passwordState &>(obj));
      return true;
    case paymentForm::ID:
      func(static_cast<paymentForm &>(obj));
      return true;
    case paymentFormTypeRegular::ID:
      func(static_cast<paymentFormTypeRegular &>(obj));
      return true;
    case paymentFormTypeStars::ID:
      func(static_cast<paymentFormTypeStars &>(obj));
      return true;
    case paymentOption::ID:
      func(static_cast<paymentOption &>(obj));
      return true;
    case paymentProviderSmartGlocal::ID:
      func(static_cast<paymentProviderSmartGlocal &>(obj));
      return true;
    case paymentProviderStripe::ID:
      func(static_cast<paymentProviderStripe &>(obj));
      return true;
    case paymentProviderOther::ID:
      func(static_cast<paymentProviderOther &>(obj));
      return true;
    case paymentReceipt::ID:
      func(static_cast<paymentReceipt &>(obj));
      return true;
    case paymentReceiptTypeRegular::ID:
      func(static_cast<paymentReceiptTypeRegular &>(obj));
      return true;
    case paymentReceiptTypeStars::ID:
      func(static_cast<paymentReceiptTypeStars &>(obj));
      return true;
    case paymentResult::ID:
      func(static_cast<paymentResult &>(obj));
      return true;
    case personalDetails::ID:
      func(static_cast<personalDetails &>(obj));
      return true;
    case personalDocument::ID:
      func(static_cast<personalDocument &>(obj));
      return true;
    case phoneNumberAuthenticationSettings::ID:
      func(static_cast<phoneNumberAuthenticationSettings &>(obj));
      return true;
    case phoneNumberCodeTypeChange::ID:
      func(static_cast<phoneNumberCodeTypeChange &>(obj));
      return true;
    case phoneNumberCodeTypeVerify::ID:
      func(static_cast<phoneNumberCodeTypeVerify &>(obj));
      return true;
    case phoneNumberCodeTypeConfirmOwnership::ID:
      func(static_cast<phoneNumberCodeTypeConfirmOwnership &>(obj));
      return true;
    case phoneNumberInfo::ID:
      func(static_cast<phoneNumberInfo &>(obj));
      return true;
    case photo::ID:
      func(static_cast<photo &>(obj));
      return true;
    case photoSize::ID:
      func(static_cast<photoSize &>(obj));
      return true;
    case point::ID:
      func(static_cast<point &>(obj));
      return true;
    case poll::ID:
      func(static_cast<poll &>(obj));
      return true;
    case pollOption::ID:
      func(static_cast<pollOption &>(obj));
      return true;
    case pollTypeRegular::ID:
      func(static_cast<pollTypeRegular &>(obj));
      return true;
    case pollTypeQuiz::ID:
      func(static_cast<pollTypeQuiz &>(obj));
      return true;
    case premiumFeatureIncreasedLimits::ID:
      func(static_cast<premiumFeatureIncreasedLimits &>(obj));
      return true;
    case premiumFeatureIncreasedUploadFileSize::ID:
      func(static_cast<premiumFeatureIncreasedUploadFileSize &>(obj));
      return true;
    case premiumFeatureImprovedDownloadSpeed::ID:
      func(static_cast<premiumFeatureImprovedDownloadSpeed &>(obj));
      return true;
    case premiumFeatureVoiceRecognition::ID:
      func(static_cast<premiumFeatureVoiceRecognition &>(obj));
      return true;
    case premiumFeatureDisabledAds::ID:
      func(static_cast<premiumFeatureDisabledAds &>(obj));
      return true;
    case premiumFeatureUniqueReactions::ID:
      func(static_cast<premiumFeatureUniqueReactions &>(obj));
      return true;
    case premiumFeatureUniqueStickers::ID:
      func(static_cast<premiumFeatureUniqueStickers &>(obj));
      return true;
    case premiumFeatureCustomEmoji::ID:
      func(static_cast<premiumFeatureCustomEmoji &>(obj));
      return true;
    case premiumFeatureAdvancedChatManagement::ID:
      func(static_cast<premiumFeatureAdvancedChatManagement &>(obj));
      return true;
    case premiumFeatureProfileBadge::ID:
      func(static_cast<premiumFeatureProfileBadge &>(obj));
      return true;
    case premiumFeatureEmojiStatus::ID:
      func(static_cast<premiumFeatureEmojiStatus &>(obj));
      return true;
    case premiumFeatureAnimatedProfilePhoto::ID:
      func(static_cast<premiumFeatureAnimatedProfilePhoto &>(obj));
      return true;
    case premiumFeatureForumTopicIcon::ID:
      func(static_cast<premiumFeatureForumTopicIcon &>(obj));
      return true;
    case premiumFeatureAppIcons::ID:
      func(static_cast<premiumFeatureAppIcons &>(obj));
      return true;
    case premiumFeatureRealTimeChatTranslation::ID:
      func(static_cast<premiumFeatureRealTimeChatTranslation &>(obj));
      return true;
    case premiumFeatureUpgradedStories::ID:
      func(static_cast<premiumFeatureUpgradedStories &>(obj));
      return true;
    case premiumFeatureChatBoost::ID:
      func(static_cast<premiumFeatureChatBoost &>(obj));
      return true;
    case premiumFeatureAccentColor::ID:
      func(static_cast<premiumFeatureAccentColor &>(obj));
      return true;
    case premiumFeatureBackgroundForBoth::ID:
      func(static_cast<premiumFeatureBackgroundForBoth &>(obj));
      return true;
    case premiumFeatureSavedMessagesTags::ID:
      func(static_cast<premiumFeatureSavedMessagesTags &>(obj));
      return true;
    case premiumFeatureMessagePrivacy::ID:
      func(static_cast<premiumFeatureMessagePrivacy &>(obj));
      return true;
    case premiumFeatureLastSeenTimes::ID:
      func(static_cast<premiumFeatureLastSeenTimes &>(obj));
      return true;
    case premiumFeatureBusiness::ID:
      func(static_cast<premiumFeatureBusiness &>(obj));
      return true;
    case premiumFeatureMessageEffects::ID:
      func(static_cast<premiumFeatureMessageEffects &>(obj));
      return true;
    case premiumFeaturePromotionAnimation::ID:
      func(static_cast<premiumFeaturePromotionAnimation &>(obj));
      return true;
    case premiumFeatures::ID:
      func(static_cast<premiumFeatures &>(obj));
      return true;
    case premiumGiftCodeInfo::ID:
      func(static_cast<premiumGiftCodeInfo &>(obj));
      return true;
    case premiumGiftCodePaymentOption::ID:
      func(static_cast<premiumGiftCodePaymentOption &>(obj));
      return true;
    case premiumGiftCodePaymentOptions::ID:
      func(static_cast<premiumGiftCodePaymentOptions &>(obj));
      return true;
    case premiumLimit::ID:
      func(static_cast<premiumLimit &>(obj));
      return true;
    case premiumLimitTypeSupergroupCount::ID:
      func(static_cast<premiumLimitTypeSupergroupCount &>(obj));
      return true;
    case premiumLimitTypePinnedChatCount::ID:
      func(static_cast<premiumLimitTypePinnedChatCount &>(obj));
      return true;
    case premiumLimitTypeCreatedPublicChatCount::ID:
      func(static_cast<premiumLimitTypeCreatedPublicChatCount &>(obj));
      return true;
    case premiumLimitTypeSavedAnimationCount::ID:
      func(static_cast<premiumLimitTypeSavedAnimationCount &>(obj));
      return true;
    case premiumLimitTypeFavoriteStickerCount::ID:
      func(static_cast<premiumLimitTypeFavoriteStickerCount &>(obj));
      return true;
    case premiumLimitTypeChatFolderCount::ID:
      func(static_cast<premiumLimitTypeChatFolderCount &>(obj));
      return true;
    case premiumLimitTypeChatFolderChosenChatCount::ID:
      func(static_cast<premiumLimitTypeChatFolderChosenChatCount &>(obj));
      return true;
    case premiumLimitTypePinnedArchivedChatCount::ID:
      func(static_cast<premiumLimitTypePinnedArchivedChatCount &>(obj));
      return true;
    case premiumLimitTypePinnedSavedMessagesTopicCount::ID:
      func(static_cast<premiumLimitTypePinnedSavedMessagesTopicCount &>(obj));
      return true;
    case premiumLimitTypeCaptionLength::ID:
      func(static_cast<premiumLimitTypeCaptionLength &>(obj));
      return true;
    case premiumLimitTypeBioLength::ID:
      func(static_cast<premiumLimitTypeBioLength &>(obj));
      return true;
    case premiumLimitTypeChatFolderInviteLinkCount::ID:
      func(static_cast<premiumLimitTypeChatFolderInviteLinkCount &>(obj));
      return true;
    case premiumLimitTypeShareableChatFolderCount::ID:
      func(static_cast<premiumLimitTypeShareableChatFolderCount &>(obj));
      return true;
    case premiumLimitTypeActiveStoryCount::ID:
      func(static_cast<premiumLimitTypeActiveStoryCount &>(obj));
      return true;
    case premiumLimitTypeWeeklySentStoryCount::ID:
      func(static_cast<premiumLimitTypeWeeklySentStoryCount &>(obj));
      return true;
    case premiumLimitTypeMonthlySentStoryCount::ID:
      func(static_cast<premiumLimitTypeMonthlySentStoryCount &>(obj));
      return true;
    case premiumLimitTypeStoryCaptionLength::ID:
      func(static_cast<premiumLimitTypeStoryCaptionLength &>(obj));
      return true;
    case premiumLimitTypeStorySuggestedReactionAreaCount::ID:
      func(static_cast<premiumLimitTypeStorySuggestedReactionAreaCount &>(obj));
      return true;
    case premiumLimitTypeSimilarChatCount::ID:
      func(static_cast<premiumLimitTypeSimilarChatCount &>(obj));
      return true;
    case premiumPaymentOption::ID:
      func(static_cast<premiumPaymentOption &>(obj));
      return true;
    case premiumSourceLimitExceeded::ID:
      func(static_cast<premiumSourceLimitExceeded &>(obj));
      return true;
    case premiumSourceFeature::ID:
      func(static_cast<premiumSourceFeature &>(obj));
      return true;
    case premiumSourceBusinessFeature::ID:
      func(static_cast<premiumSourceBusinessFeature &>(obj));
      return true;
    case premiumSourceStoryFeature::ID:
      func(static_cast<premiumSourceStoryFeature &>(obj));
      return true;
    case premiumSourceLink::ID:
      func(static_cast<premiumSourceLink &>(obj));
      return true;
    case premiumSourceSettings::ID:
      func(static_cast<premiumSourceSettings &>(obj));
      return true;
    case premiumState::ID:
      func(static_cast<premiumState &>(obj));
      return true;
    case premiumStatePaymentOption::ID:
      func(static_cast<premiumStatePaymentOption &>(obj));
      return true;
    case premiumStoryFeaturePriorityOrder::ID:
      func(static_cast<premiumStoryFeaturePriorityOrder &>(obj));
      return true;
    case premiumStoryFeatureStealthMode::ID:
      func(static_cast<premiumStoryFeatureStealthMode &>(obj));
      return true;
    case premiumStoryFeaturePermanentViewsHistory::ID:
      func(static_cast<premiumStoryFeaturePermanentViewsHistory &>(obj));
      return true;
    case premiumStoryFeatureCustomExpirationDuration::ID:
      func(static_cast<premiumStoryFeatureCustomExpirationDuration &>(obj));
      return true;
    case premiumStoryFeatureSaveStories::ID:
      func(static_cast<premiumStoryFeatureSaveStories &>(obj));
      return true;
    case premiumStoryFeatureLinksAndFormatting::ID:
      func(static_cast<premiumStoryFeatureLinksAndFormatting &>(obj));
      return true;
    case premiumStoryFeatureVideoQuality::ID:
      func(static_cast<premiumStoryFeatureVideoQuality &>(obj));
      return true;
    case prepaidGiveaway::ID:
      func(static_cast<prepaidGiveaway &>(obj));
      return true;
    case productInfo::ID:
      func(static_cast<productInfo &>(obj));
      return true;
    case profileAccentColor::ID:
      func(static_cast<profileAccentColor &>(obj));
      return true;
    case profileAccentColors::ID:
      func(static_cast<profileAccentColors &>(obj));
      return true;
    case profilePhoto::ID:
      func(static_cast<profilePhoto &>(obj));
      return true;
    case proxies::ID:
      func(static_cast<proxies &>(obj));
      return true;
    case proxy::ID:
      func(static_cast<proxy &>(obj));
      return true;
    case proxyTypeSocks5::ID:
      func(static_cast<proxyTypeSocks5 &>(obj));
      return true;
    case proxyTypeHttp::ID:
      func(static_cast<proxyTypeHttp &>(obj));
      return true;
    case proxyTypeMtproto::ID:
      func(static_cast<proxyTypeMtproto &>(obj));
      return true;
    case publicChatTypeHasUsername::ID:
      func(static_cast<publicChatTypeHasUsername &>(obj));
      return true;
    case publicChatTypeIsLocationBased::ID:
      func(static_cast<publicChatTypeIsLocationBased &>(obj));
      return true;
    case publicForwardMessage::ID:
      func(static_cast<publicForwardMessage &>(obj));
      return true;
    case publicForwardStory::ID:
      func(static_cast<publicForwardStory &>(obj));
      return true;
    case publicForwards::ID:
      func(static_cast<publicForwards &>(obj));
      return true;
    case pushMessageContentHidden::ID:
      func(static_cast<pushMessageContentHidden &>(obj));
      return true;
    case pushMessageContentAnimation::ID:
      func(static_cast<pushMessageContentAnimation &>(obj));
      return true;
    case pushMessageContentAudio::ID:
      func(static_cast<pushMessageContentAudio &>(obj));
      return true;
    case pushMessageContentContact::ID:
      func(static_cast<pushMessageContentContact &>(obj));
      return true;
    case pushMessageContentContactRegistered::ID:
      func(static_cast<pushMessageContentContactRegistered &>(obj));
      return true;
    case pushMessageContentDocument::ID:
      func(static_cast<pushMessageContentDocument &>(obj));
      return true;
    case pushMessageContentGame::ID:
      func(static_cast<pushMessageContentGame &>(obj));
      return true;
    case pushMessageContentGameScore::ID:
      func(static_cast<pushMessageContentGameScore &>(obj));
      return true;
    case pushMessageContentInvoice::ID:
      func(static_cast<pushMessageContentInvoice &>(obj));
      return true;
    case pushMessageContentLocation::ID:
      func(static_cast<pushMessageContentLocation &>(obj));
      return true;
    case pushMessageContentPaidMedia::ID:
      func(static_cast<pushMessageContentPaidMedia &>(obj));
      return true;
    case pushMessageContentPhoto::ID:
      func(static_cast<pushMessageContentPhoto &>(obj));
      return true;
    case pushMessageContentPoll::ID:
      func(static_cast<pushMessageContentPoll &>(obj));
      return true;
    case pushMessageContentPremiumGiftCode::ID:
      func(static_cast<pushMessageContentPremiumGiftCode &>(obj));
      return true;
    case pushMessageContentGiveaway::ID:
      func(static_cast<pushMessageContentGiveaway &>(obj));
      return true;
    case pushMessageContentScreenshotTaken::ID:
      func(static_cast<pushMessageContentScreenshotTaken &>(obj));
      return true;
    case pushMessageContentSticker::ID:
      func(static_cast<pushMessageContentSticker &>(obj));
      return true;
    case pushMessageContentStory::ID:
      func(static_cast<pushMessageContentStory &>(obj));
      return true;
    case pushMessageContentText::ID:
      func(static_cast<pushMessageContentText &>(obj));
      return true;
    case pushMessageContentVideo::ID:
      func(static_cast<pushMessageContentVideo &>(obj));
      return true;
    case pushMessageContentVideoNote::ID:
      func(static_cast<pushMessageContentVideoNote &>(obj));
      return true;
    case pushMessageContentVoiceNote::ID:
      func(static_cast<pushMessageContentVoiceNote &>(obj));
      return true;
    case pushMessageContentBasicGroupChatCreate::ID:
      func(static_cast<pushMessageContentBasicGroupChatCreate &>(obj));
      return true;
    case pushMessageContentChatAddMembers::ID:
      func(static_cast<pushMessageContentChatAddMembers &>(obj));
      return true;
    case pushMessageContentChatChangePhoto::ID:
      func(static_cast<pushMessageContentChatChangePhoto &>(obj));
      return true;
    case pushMessageContentChatChangeTitle::ID:
      func(static_cast<pushMessageContentChatChangeTitle &>(obj));
      return true;
    case pushMessageContentChatSetBackground::ID:
      func(static_cast<pushMessageContentChatSetBackground &>(obj));
      return true;
    case pushMessageContentChatSetTheme::ID:
      func(static_cast<pushMessageContentChatSetTheme &>(obj));
      return true;
    case pushMessageContentChatDeleteMember::ID:
      func(static_cast<pushMessageContentChatDeleteMember &>(obj));
      return true;
    case pushMessageContentChatJoinByLink::ID:
      func(static_cast<pushMessageContentChatJoinByLink &>(obj));
      return true;
    case pushMessageContentChatJoinByRequest::ID:
      func(static_cast<pushMessageContentChatJoinByRequest &>(obj));
      return true;
    case pushMessageContentRecurringPayment::ID:
      func(static_cast<pushMessageContentRecurringPayment &>(obj));
      return true;
    case pushMessageContentSuggestProfilePhoto::ID:
      func(static_cast<pushMessageContentSuggestProfilePhoto &>(obj));
      return true;
    case pushMessageContentMessageForwards::ID:
      func(static_cast<pushMessageContentMessageForwards &>(obj));
      return true;
    case pushMessageContentMediaAlbum::ID:
      func(static_cast<pushMessageContentMediaAlbum &>(obj));
      return true;
    case pushReceiverId::ID:
      func(static_cast<pushReceiverId &>(obj));
      return true;
    case quickReplyMessage::ID:
      func(static_cast<quickReplyMessage &>(obj));
      return true;
    case quickReplyMessages::ID:
      func(static_cast<quickReplyMessages &>(obj));
      return true;
    case quickReplyShortcut::ID:
      func(static_cast<quickReplyShortcut &>(obj));
      return true;
    case reactionNotificationSettings::ID:
      func(static_cast<reactionNotificationSettings &>(obj));
      return true;
    case reactionNotificationSourceNone::ID:
      func(static_cast<reactionNotificationSourceNone &>(obj));
      return true;
    case reactionNotificationSourceContacts::ID:
      func(static_cast<reactionNotificationSourceContacts &>(obj));
      return true;
    case reactionNotificationSourceAll::ID:
      func(static_cast<reactionNotificationSourceAll &>(obj));
      return true;
    case reactionTypeEmoji::ID:
      func(static_cast<reactionTypeEmoji &>(obj));
      return true;
    case reactionTypeCustomEmoji::ID:
      func(static_cast<reactionTypeCustomEmoji &>(obj));
      return true;
    case reactionTypePaid::ID:
      func(static_cast<reactionTypePaid &>(obj));
      return true;
    case reactionUnavailabilityReasonAnonymousAdministrator::ID:
      func(static_cast<reactionUnavailabilityReasonAnonymousAdministrator &>(obj));
      return true;
    case reactionUnavailabilityReasonGuest::ID:
      func(static_cast<reactionUnavailabilityReasonGuest &>(obj));
      return true;
    case readDatePrivacySettings::ID:
      func(static_cast<readDatePrivacySettings &>(obj));
      return true;
    case recommendedChatFolder::ID:
      func(static_cast<recommendedChatFolder &>(obj));
      return true;
    case recommendedChatFolders::ID:
      func(static_cast<recommendedChatFolders &>(obj));
      return true;
    case recoveryEmailAddress::ID:
      func(static_cast<recoveryEmailAddress &>(obj));
      return true;
    case remoteFile::ID:
      func(static_cast<remoteFile &>(obj));
      return true;
    case replyMarkupRemoveKeyboard::ID:
      func(static_cast<replyMarkupRemoveKeyboard &>(obj));
      return true;
    case replyMarkupForceReply::ID:
      func(static_cast<replyMarkupForceReply &>(obj));
      return true;
    case replyMarkupShowKeyboard::ID:
      func(static_cast<replyMarkupShowKeyboard &>(obj));
      return true;
    case replyMarkupInlineKeyboard::ID:
      func(static_cast<replyMarkupInlineKeyboard &>(obj));
      return true;
    case reportChatSponsoredMessageOption::ID:
      func(static_cast<reportChatSponsoredMessageOption &>(obj));
      return true;
    case reportChatSponsoredMessageResultOk::ID:
      func(static_cast<reportChatSponsoredMessageResultOk &>(obj));
      return true;
    case reportChatSponsoredMessageResultFailed::ID:
      func(static_cast<reportChatSponsoredMessageResultFailed &>(obj));
      return true;
    case reportChatSponsoredMessageResultOptionRequired::ID:
      func(static_cast<reportChatSponsoredMessageResultOptionRequired &>(obj));
      return true;
    case reportChatSponsoredMessageResultAdsHidden::ID:
      func(static_cast<reportChatSponsoredMessageResultAdsHidden &>(obj));
      return true;
    case reportChatSponsoredMessageResultPremiumRequired::ID:
      func(static_cast<reportChatSponsoredMessageResultPremiumRequired &>(obj));
      return true;
    case reportReasonSpam::ID:
      func(static_cast<reportReasonSpam &>(obj));
      return true;
    case reportReasonViolence::ID:
      func(static_cast<reportReasonViolence &>(obj));
      return true;
    case reportReasonPornography::ID:
      func(static_cast<reportReasonPornography &>(obj));
      return true;
    case reportReasonChildAbuse::ID:
      func(static_cast<reportReasonChildAbuse &>(obj));
      return true;
    case reportReasonCopyright::ID:
      func(static_cast<reportReasonCopyright &>(obj));
      return true;
    case reportReasonUnrelatedLocation::ID:
      func(static_cast<reportReasonUnrelatedLocation &>(obj));
      return true;
    case reportReasonFake::ID:
      func(static_cast<reportReasonFake &>(obj));
      return true;
    case reportReasonIllegalDrugs::ID:
      func(static_cast<reportReasonIllegalDrugs &>(obj));
      return true;
    case reportReasonPersonalDetails::ID:
      func(static_cast<reportReasonPersonalDetails &>(obj));
      return true;
    case reportReasonCustom::ID:
      func(static_cast<reportReasonCustom &>(obj));
      return true;
    case resendCodeReasonUserRequest::ID:
      func(static_cast<resendCodeReasonUserRequest &>(obj));
      return true;
    case resendCodeReasonVerificationFailed::ID:
      func(static_cast<resendCodeReasonVerificationFailed &>(obj));
      return true;
    case resetPasswordResultOk::ID:
      func(static_cast<resetPasswordResultOk &>(obj));
      return true;
    case resetPasswordResultPending::ID:
      func(static_cast<resetPasswordResultPending &>(obj));
      return true;
    case resetPasswordResultDeclined::ID:
      func(static_cast<resetPasswordResultDeclined &>(obj));
      return true;
    case revenueWithdrawalStatePending::ID:
      func(static_cast<revenueWithdrawalStatePending &>(obj));
      return true;
    case revenueWithdrawalStateSucceeded::ID:
      func(static_cast<revenueWithdrawalStateSucceeded &>(obj));
      return true;
    case revenueWithdrawalStateFailed::ID:
      func(static_cast<revenueWithdrawalStateFailed &>(obj));
      return true;
    case richTextPlain::ID:
      func(static_cast<richTextPlain &>(obj));
      return true;
    case richTextBold::ID:
      func(static_cast<richTextBold &>(obj));
      return true;
    case richTextItalic::ID:
      func(static_cast<richTextItalic &>(obj));
      return true;
    case richTextUnderline::ID:
      func(static_cast<richTextUnderline &>(obj));
      return true;
    case richTextStrikethrough::ID:
      func(static_cast<richTextStrikethrough &>(obj));
      return true;
    case richTextFixed::ID:
      func(static_cast<richTextFixed &>(obj));
      return true;
    case richTextUrl::ID:
      func(static_cast<richTextUrl &>(obj));
      return true;
    case richTextEmailAddress::ID:
      func(static_cast<richTextEmailAddress &>(obj));
      return true;
    case richTextSubscript::ID:
      func(static_cast<richTextSubscript &>(obj));
      return true;
    case richTextSuperscript::ID:
      func(static_cast<richTextSuperscript &>(obj));
      return true;
    case richTextMarked::ID:
      func(static_cast<richTextMarked &>(obj));
      return true;
    case richTextPhoneNumber::ID:
      func(static_cast<richTextPhoneNumber &>(obj));
      return true;
    case richTextIcon::ID:
      func(static_cast<richTextIcon &>(obj));
      return true;
    case richTextReference::ID:
      func(static_cast<richTextReference &>(obj));
      return true;
    case richTextAnchor::ID:
      func(static_cast<richTextAnchor &>(obj));
      return true;
    case richTextAnchorLink::ID:
      func(static_cast<richTextAnchorLink &>(obj));
      return true;
    case richTexts::ID:
      func(static_cast<richTexts &>(obj));
      return true;
    case rtmpUrl::ID:
      func(static_cast<rtmpUrl &>(obj));
      return true;
    case savedCredentials::ID:
      func(static_cast<savedCredentials &>(obj));
      return true;
    case savedMessagesTag::ID:
      func(static_cast<savedMessagesTag &>(obj));
      return true;
    case savedMessagesTags::ID:
      func(static_cast<savedMessagesTags &>(obj));
      return true;
    case savedMessagesTopic::ID:
      func(static_cast<savedMessagesTopic &>(obj));
      return true;
    case savedMessagesTopicTypeMyNotes::ID:
      func(static_cast<savedMessagesTopicTypeMyNotes &>(obj));
      return true;
    case savedMessagesTopicTypeAuthorHidden::ID:
      func(static_cast<savedMessagesTopicTypeAuthorHidden &>(obj));
      return true;
    case savedMessagesTopicTypeSavedFromChat::ID:
      func(static_cast<savedMessagesTopicTypeSavedFromChat &>(obj));
      return true;
    case scopeAutosaveSettings::ID:
      func(static_cast<scopeAutosaveSettings &>(obj));
      return true;
    case scopeNotificationSettings::ID:
      func(static_cast<scopeNotificationSettings &>(obj));
      return true;
    case searchMessagesFilterEmpty::ID:
      func(static_cast<searchMessagesFilterEmpty &>(obj));
      return true;
    case searchMessagesFilterAnimation::ID:
      func(static_cast<searchMessagesFilterAnimation &>(obj));
      return true;
    case searchMessagesFilterAudio::ID:
      func(static_cast<searchMessagesFilterAudio &>(obj));
      return true;
    case searchMessagesFilterDocument::ID:
      func(static_cast<searchMessagesFilterDocument &>(obj));
      return true;
    case searchMessagesFilterPhoto::ID:
      func(static_cast<searchMessagesFilterPhoto &>(obj));
      return true;
    case searchMessagesFilterVideo::ID:
      func(static_cast<searchMessagesFilterVideo &>(obj));
      return true;
    case searchMessagesFilterVoiceNote::ID:
      func(static_cast<searchMessagesFilterVoiceNote &>(obj));
      return true;
    case searchMessagesFilterPhotoAndVideo::ID:
      func(static_cast<searchMessagesFilterPhotoAndVideo &>(obj));
      return true;
    case searchMessagesFilterUrl::ID:
      func(static_cast<searchMessagesFilterUrl &>(obj));
      return true;
    case searchMessagesFilterChatPhoto::ID:
      func(static_cast<searchMessagesFilterChatPhoto &>(obj));
      return true;
    case searchMessagesFilterVideoNote::ID:
      func(static_cast<searchMessagesFilterVideoNote &>(obj));
      return true;
    case searchMessagesFilterVoiceAndVideoNote::ID:
      func(static_cast<searchMessagesFilterVoiceAndVideoNote &>(obj));
      return true;
    case searchMessagesFilterMention::ID:
      func(static_cast<searchMessagesFilterMention &>(obj));
      return true;
    case searchMessagesFilterUnreadMention::ID:
      func(static_cast<searchMessagesFilterUnreadMention &>(obj));
      return true;
    case searchMessagesFilterUnreadReaction::ID:
      func(static_cast<searchMessagesFilterUnreadReaction &>(obj));
      return true;
    case searchMessagesFilterFailedToSend::ID:
      func(static_cast<searchMessagesFilterFailedToSend &>(obj));
      return true;
    case searchMessagesFilterPinned::ID:
      func(static_cast<searchMessagesFilterPinned &>(obj));
      return true;
    case seconds::ID:
      func(static_cast<seconds &>(obj));
      return true;
    case secretChat::ID:
      func(static_cast<secretChat &>(obj));
      return true;
    case secretChatStatePending::ID:
      func(static_cast<secretChatStatePending &>(obj));
      return true;
    case secretChatStateReady::ID:
      func(static_cast<secretChatStateReady &>(obj));
      return true;
    case secretChatStateClosed::ID:
      func(static_cast<secretChatStateClosed &>(obj));
      return true;
    case sentWebAppMessage::ID:
      func(static_cast<sentWebAppMessage &>(obj));
      return true;
    case session::ID:
      func(static_cast<session &>(obj));
      return true;
    case sessionTypeAndroid::ID:
      func(static_cast<sessionTypeAndroid &>(obj));
      return true;
    case sessionTypeApple::ID:
      func(static_cast<sessionTypeApple &>(obj));
      return true;
    case sessionTypeBrave::ID:
      func(static_cast<sessionTypeBrave &>(obj));
      return true;
    case sessionTypeChrome::ID:
      func(static_cast<sessionTypeChrome &>(obj));
      return true;
    case sessionTypeEdge::ID:
      func(static_cast<sessionTypeEdge &>(obj));
      return true;
    case sessionTypeFirefox::ID:
      func(static_cast<sessionTypeFirefox &>(obj));
      return true;
    case sessionTypeIpad::ID:
      func(static_cast<sessionTypeIpad &>(obj));
      return true;
    case sessionTypeIphone::ID:
      func(static_cast<sessionTypeIphone &>(obj));
      return true;
    case sessionTypeLinux::ID:
      func(static_cast<sessionTypeLinux &>(obj));
      return true;
    case sessionTypeMac::ID:
      func(static_cast<sessionTypeMac &>(obj));
      return true;
    case sessionTypeOpera::ID:
      func(static_cast<sessionTypeOpera &>(obj));
      return true;
    case sessionTypeSafari::ID:
      func(static_cast<sessionTypeSafari &>(obj));
      return true;
    case sessionTypeUbuntu::ID:
      func(static_cast<sessionTypeUbuntu &>(obj));
      return true;
    case sessionTypeUnknown::ID:
      func(static_cast<sessionTypeUnknown &>(obj));
      return true;
    case sessionTypeVivaldi::ID:
      func(static_cast<sessionTypeVivaldi &>(obj));
      return true;
    case sessionTypeWindows::ID:
      func(static_cast<sessionTypeWindows &>(obj));
      return true;
    case sessionTypeXbox::ID:
      func(static_cast<sessionTypeXbox &>(obj));
      return true;
    case sessions::ID:
      func(static_cast<sessions &>(obj));
      return true;
    case sharedChat::ID:
      func(static_cast<sharedChat &>(obj));
      return true;
    case sharedUser::ID:
      func(static_cast<sharedUser &>(obj));
      return true;
    case shippingOption::ID:
      func(static_cast<shippingOption &>(obj));
      return true;
    case speechRecognitionResultPending::ID:
      func(static_cast<speechRecognitionResultPending &>(obj));
      return true;
    case speechRecognitionResultText::ID:
      func(static_cast<speechRecognitionResultText &>(obj));
      return true;
    case speechRecognitionResultError::ID:
      func(static_cast<speechRecognitionResultError &>(obj));
      return true;
    case sponsoredMessage::ID:
      func(static_cast<sponsoredMessage &>(obj));
      return true;
    case sponsoredMessages::ID:
      func(static_cast<sponsoredMessages &>(obj));
      return true;
    case starGiveawayPaymentOption::ID:
      func(static_cast<starGiveawayPaymentOption &>(obj));
      return true;
    case starGiveawayPaymentOptions::ID:
      func(static_cast<starGiveawayPaymentOptions &>(obj));
      return true;
    case starGiveawayWinnerOption::ID:
      func(static_cast<starGiveawayWinnerOption &>(obj));
      return true;
    case starPaymentOption::ID:
      func(static_cast<starPaymentOption &>(obj));
      return true;
    case starPaymentOptions::ID:
      func(static_cast<starPaymentOptions &>(obj));
      return true;
    case starRevenueStatistics::ID:
      func(static_cast<starRevenueStatistics &>(obj));
      return true;
    case starRevenueStatus::ID:
      func(static_cast<starRevenueStatus &>(obj));
      return true;
    case starSubscription::ID:
      func(static_cast<starSubscription &>(obj));
      return true;
    case starSubscriptionPricing::ID:
      func(static_cast<starSubscriptionPricing &>(obj));
      return true;
    case starSubscriptions::ID:
      func(static_cast<starSubscriptions &>(obj));
      return true;
    case starTransaction::ID:
      func(static_cast<starTransaction &>(obj));
      return true;
    case starTransactionDirectionIncoming::ID:
      func(static_cast<starTransactionDirectionIncoming &>(obj));
      return true;
    case starTransactionDirectionOutgoing::ID:
      func(static_cast<starTransactionDirectionOutgoing &>(obj));
      return true;
    case starTransactionPartnerTelegram::ID:
      func(static_cast<starTransactionPartnerTelegram &>(obj));
      return true;
    case starTransactionPartnerAppStore::ID:
      func(static_cast<starTransactionPartnerAppStore &>(obj));
      return true;
    case starTransactionPartnerGooglePlay::ID:
      func(static_cast<starTransactionPartnerGooglePlay &>(obj));
      return true;
    case starTransactionPartnerFragment::ID:
      func(static_cast<starTransactionPartnerFragment &>(obj));
      return true;
    case starTransactionPartnerTelegramAds::ID:
      func(static_cast<starTransactionPartnerTelegramAds &>(obj));
      return true;
    case starTransactionPartnerBot::ID:
      func(static_cast<starTransactionPartnerBot &>(obj));
      return true;
    case starTransactionPartnerBusiness::ID:
      func(static_cast<starTransactionPartnerBusiness &>(obj));
      return true;
    case starTransactionPartnerChat::ID:
      func(static_cast<starTransactionPartnerChat &>(obj));
      return true;
    case starTransactionPartnerUser::ID:
      func(static_cast<starTransactionPartnerUser &>(obj));
      return true;
    case starTransactionPartnerUnsupported::ID:
      func(static_cast<starTransactionPartnerUnsupported &>(obj));
      return true;
    case starTransactions::ID:
      func(static_cast<starTransactions &>(obj));
      return true;
    case statisticalGraphData::ID:
      func(static_cast<statisticalGraphData &>(obj));
      return true;
    case statisticalGraphAsync::ID:
      func(static_cast<statisticalGraphAsync &>(obj));
      return true;
    case statisticalGraphError::ID:
      func(static_cast<statisticalGraphError &>(obj));
      return true;
    case statisticalValue::ID:
      func(static_cast<statisticalValue &>(obj));
      return true;
    case sticker::ID:
      func(static_cast<sticker &>(obj));
      return true;
    case stickerFormatWebp::ID:
      func(static_cast<stickerFormatWebp &>(obj));
      return true;
    case stickerFormatTgs::ID:
      func(static_cast<stickerFormatTgs &>(obj));
      return true;
    case stickerFormatWebm::ID:
      func(static_cast<stickerFormatWebm &>(obj));
      return true;
    case stickerFullTypeRegular::ID:
      func(static_cast<stickerFullTypeRegular &>(obj));
      return true;
    case stickerFullTypeMask::ID:
      func(static_cast<stickerFullTypeMask &>(obj));
      return true;
    case stickerFullTypeCustomEmoji::ID:
      func(static_cast<stickerFullTypeCustomEmoji &>(obj));
      return true;
    case stickerSet::ID:
      func(static_cast<stickerSet &>(obj));
      return true;
    case stickerSetInfo::ID:
      func(static_cast<stickerSetInfo &>(obj));
      return true;
    case stickerSets::ID:
      func(static_cast<stickerSets &>(obj));
      return true;
    case stickerTypeRegular::ID:
      func(static_cast<stickerTypeRegular &>(obj));
      return true;
    case stickerTypeMask::ID:
      func(static_cast<stickerTypeMask &>(obj));
      return true;
    case stickerTypeCustomEmoji::ID:
      func(static_cast<stickerTypeCustomEmoji &>(obj));
      return true;
    case stickers::ID:
      func(static_cast<stickers &>(obj));
      return true;
    case storageStatistics::ID:
      func(static_cast<storageStatistics &>(obj));
      return true;
    case storageStatisticsByChat::ID:
      func(static_cast<storageStatisticsByChat &>(obj));
      return true;
    case storageStatisticsByFileType::ID:
      func(static_cast<storageStatisticsByFileType &>(obj));
      return true;
    case storageStatisticsFast::ID:
      func(static_cast<storageStatisticsFast &>(obj));
      return true;
    case storePaymentPurposePremiumSubscription::ID:
      func(static_cast<storePaymentPurposePremiumSubscription &>(obj));
      return true;
    case storePaymentPurposeGiftedPremium::ID:
      func(static_cast<storePaymentPurposeGiftedPremium &>(obj));
      return true;
    case storePaymentPurposePremiumGiftCodes::ID:
      func(static_cast<storePaymentPurposePremiumGiftCodes &>(obj));
      return true;
    case storePaymentPurposePremiumGiveaway::ID:
      func(static_cast<storePaymentPurposePremiumGiveaway &>(obj));
      return true;
    case storePaymentPurposeStarGiveaway::ID:
      func(static_cast<storePaymentPurposeStarGiveaway &>(obj));
      return true;
    case storePaymentPurposeStars::ID:
      func(static_cast<storePaymentPurposeStars &>(obj));
      return true;
    case storePaymentPurposeGiftedStars::ID:
      func(static_cast<storePaymentPurposeGiftedStars &>(obj));
      return true;
    case stories::ID:
      func(static_cast<stories &>(obj));
      return true;
    case story::ID:
      func(static_cast<story &>(obj));
      return true;
    case storyArea::ID:
      func(static_cast<storyArea &>(obj));
      return true;
    case storyAreaPosition::ID:
      func(static_cast<storyAreaPosition &>(obj));
      return true;
    case storyAreaTypeLocation::ID:
      func(static_cast<storyAreaTypeLocation &>(obj));
      return true;
    case storyAreaTypeVenue::ID:
      func(static_cast<storyAreaTypeVenue &>(obj));
      return true;
    case storyAreaTypeSuggestedReaction::ID:
      func(static_cast<storyAreaTypeSuggestedReaction &>(obj));
      return true;
    case storyAreaTypeMessage::ID:
      func(static_cast<storyAreaTypeMessage &>(obj));
      return true;
    case storyAreaTypeLink::ID:
      func(static_cast<storyAreaTypeLink &>(obj));
      return true;
    case storyAreaTypeWeather::ID:
      func(static_cast<storyAreaTypeWeather &>(obj));
      return true;
    case storyContentPhoto::ID:
      func(static_cast<storyContentPhoto &>(obj));
      return true;
    case storyContentVideo::ID:
      func(static_cast<storyContentVideo &>(obj));
      return true;
    case storyContentUnsupported::ID:
      func(static_cast<storyContentUnsupported &>(obj));
      return true;
    case storyFullId::ID:
      func(static_cast<storyFullId &>(obj));
      return true;
    case storyInfo::ID:
      func(static_cast<storyInfo &>(obj));
      return true;
    case storyInteraction::ID:
      func(static_cast<storyInteraction &>(obj));
      return true;
    case storyInteractionInfo::ID:
      func(static_cast<storyInteractionInfo &>(obj));
      return true;
    case storyInteractionTypeView::ID:
      func(static_cast<storyInteractionTypeView &>(obj));
      return true;
    case storyInteractionTypeForward::ID:
      func(static_cast<storyInteractionTypeForward &>(obj));
      return true;
    case storyInteractionTypeRepost::ID:
      func(static_cast<storyInteractionTypeRepost &>(obj));
      return true;
    case storyInteractions::ID:
      func(static_cast<storyInteractions &>(obj));
      return true;
    case storyListMain::ID:
      func(static_cast<storyListMain &>(obj));
      return true;
    case storyListArchive::ID:
      func(static_cast<storyListArchive &>(obj));
      return true;
    case storyOriginPublicStory::ID:
      func(static_cast<storyOriginPublicStory &>(obj));
      return true;
    case storyOriginHiddenUser::ID:
      func(static_cast<storyOriginHiddenUser &>(obj));
      return true;
    case storyPrivacySettingsEveryone::ID:
      func(static_cast<storyPrivacySettingsEveryone &>(obj));
      return true;
    case storyPrivacySettingsContacts::ID:
      func(static_cast<storyPrivacySettingsContacts &>(obj));
      return true;
    case storyPrivacySettingsCloseFriends::ID:
      func(static_cast<storyPrivacySettingsCloseFriends &>(obj));
      return true;
    case storyPrivacySettingsSelectedUsers::ID:
      func(static_cast<storyPrivacySettingsSelectedUsers &>(obj));
      return true;
    case storyRepostInfo::ID:
      func(static_cast<storyRepostInfo &>(obj));
      return true;
    case storyStatistics::ID:
      func(static_cast<storyStatistics &>(obj));
      return true;
    case storyVideo::ID:
      func(static_cast<storyVideo &>(obj));
      return true;
    case suggestedActionEnableArchiveAndMuteNewChats::ID:
      func(static_cast<suggestedActionEnableArchiveAndMuteNewChats &>(obj));
      return true;
    case suggestedActionCheckPassword::ID:
      func(static_cast<suggestedActionCheckPassword &>(obj));
      return true;
    case suggestedActionCheckPhoneNumber::ID:
      func(static_cast<suggestedActionCheckPhoneNumber &>(obj));
      return true;
    case suggestedActionViewChecksHint::ID:
      func(static_cast<suggestedActionViewChecksHint &>(obj));
      return true;
    case suggestedActionConvertToBroadcastGroup::ID:
      func(static_cast<suggestedActionConvertToBroadcastGroup &>(obj));
      return true;
    case suggestedActionSetPassword::ID:
      func(static_cast<suggestedActionSetPassword &>(obj));
      return true;
    case suggestedActionUpgradePremium::ID:
      func(static_cast<suggestedActionUpgradePremium &>(obj));
      return true;
    case suggestedActionRestorePremium::ID:
      func(static_cast<suggestedActionRestorePremium &>(obj));
      return true;
    case suggestedActionSubscribeToAnnualPremium::ID:
      func(static_cast<suggestedActionSubscribeToAnnualPremium &>(obj));
      return true;
    case suggestedActionGiftPremiumForChristmas::ID:
      func(static_cast<suggestedActionGiftPremiumForChristmas &>(obj));
      return true;
    case suggestedActionSetBirthdate::ID:
      func(static_cast<suggestedActionSetBirthdate &>(obj));
      return true;
    case suggestedActionExtendPremium::ID:
      func(static_cast<suggestedActionExtendPremium &>(obj));
      return true;
    case suggestedActionExtendStarSubscriptions::ID:
      func(static_cast<suggestedActionExtendStarSubscriptions &>(obj));
      return true;
    case supergroup::ID:
      func(static_cast<supergroup &>(obj));
      return true;
    case supergroupFullInfo::ID:
      func(static_cast<supergroupFullInfo &>(obj));
      return true;
    case supergroupMembersFilterRecent::ID:
      func(static_cast<supergroupMembersFilterRecent &>(obj));
      return true;
    case supergroupMembersFilterContacts::ID:
      func(static_cast<supergroupMembersFilterContacts &>(obj));
      return true;
    case supergroupMembersFilterAdministrators::ID:
      func(static_cast<supergroupMembersFilterAdministrators &>(obj));
      return true;
    case supergroupMembersFilterSearch::ID:
      func(static_cast<supergroupMembersFilterSearch &>(obj));
      return true;
    case supergroupMembersFilterRestricted::ID:
      func(static_cast<supergroupMembersFilterRestricted &>(obj));
      return true;
    case supergroupMembersFilterBanned::ID:
      func(static_cast<supergroupMembersFilterBanned &>(obj));
      return true;
    case supergroupMembersFilterMention::ID:
      func(static_cast<supergroupMembersFilterMention &>(obj));
      return true;
    case supergroupMembersFilterBots::ID:
      func(static_cast<supergroupMembersFilterBots &>(obj));
      return true;
    case tMeUrl::ID:
      func(static_cast<tMeUrl &>(obj));
      return true;
    case tMeUrlTypeUser::ID:
      func(static_cast<tMeUrlTypeUser &>(obj));
      return true;
    case tMeUrlTypeSupergroup::ID:
      func(static_cast<tMeUrlTypeSupergroup &>(obj));
      return true;
    case tMeUrlTypeChatInvite::ID:
      func(static_cast<tMeUrlTypeChatInvite &>(obj));
      return true;
    case tMeUrlTypeStickerSet::ID:
      func(static_cast<tMeUrlTypeStickerSet &>(obj));
      return true;
    case tMeUrls::ID:
      func(static_cast<tMeUrls &>(obj));
      return true;
    case targetChatCurrent::ID:
      func(static_cast<targetChatCurrent &>(obj));
      return true;
    case targetChatChosen::ID:
      func(static_cast<targetChatChosen &>(obj));
      return true;
    case targetChatInternalLink::ID:
      func(static_cast<targetChatInternalLink &>(obj));
      return true;
    case telegramPaymentPurposePremiumGiftCodes::ID:
      func(static_cast<telegramPaymentPurposePremiumGiftCodes &>(obj));
      return true;
    case telegramPaymentPurposePremiumGiveaway::ID:
      func(static_cast<telegramPaymentPurposePremiumGiveaway &>(obj));
      return true;
    case telegramPaymentPurposeStars::ID:
      func(static_cast<telegramPaymentPurposeStars &>(obj));
      return true;
    case telegramPaymentPurposeGiftedStars::ID:
      func(static_cast<telegramPaymentPurposeGiftedStars &>(obj));
      return true;
    case telegramPaymentPurposeStarGiveaway::ID:
      func(static_cast<telegramPaymentPurposeStarGiveaway &>(obj));
      return true;
    case telegramPaymentPurposeJoinChat::ID:
      func(static_cast<telegramPaymentPurposeJoinChat &>(obj));
      return true;
    case temporaryPasswordState::ID:
      func(static_cast<temporaryPasswordState &>(obj));
      return true;
    case termsOfService::ID:
      func(static_cast<termsOfService &>(obj));
      return true;
    case testBytes::ID:
      func(static_cast<testBytes &>(obj));
      return true;
    case testInt::ID:
      func(static_cast<testInt &>(obj));
      return true;
    case testString::ID:
      func(static_cast<testString &>(obj));
      return true;
    case testVectorInt::ID:
      func(static_cast<testVectorInt &>(obj));
      return true;
    case testVectorIntObject::ID:
      func(static_cast<testVectorIntObject &>(obj));
      return true;
    case testVectorString::ID:
      func(static_cast<testVectorString &>(obj));
      return true;
    case testVectorStringObject::ID:
      func(static_cast<testVectorStringObject &>(obj));
      return true;
    case text::ID:
      func(static_cast<text &>(obj));
      return true;
    case textEntities::ID:
      func(static_cast<textEntities &>(obj));
      return true;
    case textEntity::ID:
      func(static_cast<textEntity &>(obj));
      return true;
    case textEntityTypeMention::ID:
      func(static_cast<textEntityTypeMention &>(obj));
      return true;
    case textEntityTypeHashtag::ID:
      func(static_cast<textEntityTypeHashtag &>(obj));
      return true;
    case textEntityTypeCashtag::ID:
      func(static_cast<textEntityTypeCashtag &>(obj));
      return true;
    case textEntityTypeBotCommand::ID:
      func(static_cast<textEntityTypeBotCommand &>(obj));
      return true;
    case textEntityTypeUrl::ID:
      func(static_cast<textEntityTypeUrl &>(obj));
      return true;
    case textEntityTypeEmailAddress::ID:
      func(static_cast<textEntityTypeEmailAddress &>(obj));
      return true;
    case textEntityTypePhoneNumber::ID:
      func(static_cast<textEntityTypePhoneNumber &>(obj));
      return true;
    case textEntityTypeBankCardNumber::ID:
      func(static_cast<textEntityTypeBankCardNumber &>(obj));
      return true;
    case textEntityTypeBold::ID:
      func(static_cast<textEntityTypeBold &>(obj));
      return true;
    case textEntityTypeItalic::ID:
      func(static_cast<textEntityTypeItalic &>(obj));
      return true;
    case textEntityTypeUnderline::ID:
      func(static_cast<textEntityTypeUnderline &>(obj));
      return true;
    case textEntityTypeStrikethrough::ID:
      func(static_cast<textEntityTypeStrikethrough &>(obj));
      return true;
    case textEntityTypeSpoiler::ID:
      func(static_cast<textEntityTypeSpoiler &>(obj));
      return true;
    case textEntityTypeCode::ID:
      func(static_cast<textEntityTypeCode &>(obj));
      return true;
    case textEntityTypePre::ID:
      func(static_cast<textEntityTypePre &>(obj));
      return true;
    case textEntityTypePreCode::ID:
      func(static_cast<textEntityTypePreCode &>(obj));
      return true;
    case textEntityTypeBlockQuote::ID:
      func(static_cast<textEntityTypeBlockQuote &>(obj));
      return true;
    case textEntityTypeExpandableBlockQuote::ID:
      func(static_cast<textEntityTypeExpandableBlockQuote &>(obj));
      return true;
    case textEntityTypeTextUrl::ID:
      func(static_cast<textEntityTypeTextUrl &>(obj));
      return true;
    case textEntityTypeMentionName::ID:
      func(static_cast<textEntityTypeMentionName &>(obj));
      return true;
    case textEntityTypeCustomEmoji::ID:
      func(static_cast<textEntityTypeCustomEmoji &>(obj));
      return true;
    case textEntityTypeMediaTimestamp::ID:
      func(static_cast<textEntityTypeMediaTimestamp &>(obj));
      return true;
    case textParseModeMarkdown::ID:
      func(static_cast<textParseModeMarkdown &>(obj));
      return true;
    case textParseModeHTML::ID:
      func(static_cast<textParseModeHTML &>(obj));
      return true;
    case textQuote::ID:
      func(static_cast<textQuote &>(obj));
      return true;
    case themeParameters::ID:
      func(static_cast<themeParameters &>(obj));
      return true;
    case themeSettings::ID:
      func(static_cast<themeSettings &>(obj));
      return true;
    case thumbnail::ID:
      func(static_cast<thumbnail &>(obj));
      return true;
    case thumbnailFormatJpeg::ID:
      func(static_cast<thumbnailFormatJpeg &>(obj));
      return true;
    case thumbnailFormatGif::ID:
      func(static_cast<thumbnailFormatGif &>(obj));
      return true;
    case thumbnailFormatMpeg4::ID:
      func(static_cast<thumbnailFormatMpeg4 &>(obj));
      return true;
    case thumbnailFormatPng::ID:
      func(static_cast<thumbnailFormatPng &>(obj));
      return true;
    case thumbnailFormatTgs::ID:
      func(static_cast<thumbnailFormatTgs &>(obj));
      return true;
    case thumbnailFormatWebm::ID:
      func(static_cast<thumbnailFormatWebm &>(obj));
      return true;
    case thumbnailFormatWebp::ID:
      func(static_cast<thumbnailFormatWebp &>(obj));
      return true;
    case timeZone::ID:
      func(static_cast<timeZone &>(obj));
      return true;
    case timeZones::ID:
      func(static_cast<timeZones &>(obj));
      return true;
    case topChatCategoryUsers::ID:
      func(static_cast<topChatCategoryUsers &>(obj));
      return true;
    case topChatCategoryBots::ID:
      func(static_cast<topChatCategoryBots &>(obj));
      return true;
    case topChatCategoryGroups::ID:
      func(static_cast<topChatCategoryGroups &>(obj));
      return true;
    case topChatCategoryChannels::ID:
      func(static_cast<topChatCategoryChannels &>(obj));
      return true;
    case topChatCategoryInlineBots::ID:
      func(static_cast<topChatCategoryInlineBots &>(obj));
      return true;
    case topChatCategoryWebAppBots::ID:
      func(static_cast<topChatCategoryWebAppBots &>(obj));
      return true;
    case topChatCategoryCalls::ID:
      func(static_cast<topChatCategoryCalls &>(obj));
      return true;
    case topChatCategoryForwardChats::ID:
      func(static_cast<topChatCategoryForwardChats &>(obj));
      return true;
    case trendingStickerSets::ID:
      func(static_cast<trendingStickerSets &>(obj));
      return true;
    case unconfirmedSession::ID:
      func(static_cast<unconfirmedSession &>(obj));
      return true;
    case unreadReaction::ID:
      func(static_cast<unreadReaction &>(obj));
      return true;
    case updateAuthorizationState::ID:
      func(static_cast<updateAuthorizationState &>(obj));
      return true;
    case updateNewMessage::ID:
      func(static_cast<updateNewMessage &>(obj));
      return true;
    case updateMessageSendAcknowledged::ID:
      func(static_cast<updateMessageSendAcknowledged &>(obj));
      return true;
    case updateMessageSendSucceeded::ID:
      func(static_cast<updateMessageSendSucceeded &>(obj));
      return true;
    case updateMessageSendFailed::ID:
      func(static_cast<updateMessageSendFailed &>(obj));
      return true;
    case updateMessageContent::ID:
      func(static_cast<updateMessageContent &>(obj));
      return true;
    case updateMessageEdited::ID:
      func(static_cast<updateMessageEdited &>(obj));
      return true;
    case updateMessageIsPinned::ID:
      func(static_cast<updateMessageIsPinned &>(obj));
      return true;
    case updateMessageInteractionInfo::ID:
      func(static_cast<updateMessageInteractionInfo &>(obj));
      return true;
    case updateMessageContentOpened::ID:
      func(static_cast<updateMessageContentOpened &>(obj));
      return true;
    case updateMessageMentionRead::ID:
      func(static_cast<updateMessageMentionRead &>(obj));
      return true;
    case updateMessageUnreadReactions::ID:
      func(static_cast<updateMessageUnreadReactions &>(obj));
      return true;
    case updateMessageFactCheck::ID:
      func(static_cast<updateMessageFactCheck &>(obj));
      return true;
    case updateMessageLiveLocationViewed::ID:
      func(static_cast<updateMessageLiveLocationViewed &>(obj));
      return true;
    case updateNewChat::ID:
      func(static_cast<updateNewChat &>(obj));
      return true;
    case updateChatTitle::ID:
      func(static_cast<updateChatTitle &>(obj));
      return true;
    case updateChatPhoto::ID:
      func(static_cast<updateChatPhoto &>(obj));
      return true;
    case updateChatAccentColors::ID:
      func(static_cast<updateChatAccentColors &>(obj));
      return true;
    case updateChatPermissions::ID:
      func(static_cast<updateChatPermissions &>(obj));
      return true;
    case updateChatLastMessage::ID:
      func(static_cast<updateChatLastMessage &>(obj));
      return true;
    case updateChatPosition::ID:
      func(static_cast<updateChatPosition &>(obj));
      return true;
    case updateChatAddedToList::ID:
      func(static_cast<updateChatAddedToList &>(obj));
      return true;
    case updateChatRemovedFromList::ID:
      func(static_cast<updateChatRemovedFromList &>(obj));
      return true;
    case updateChatReadInbox::ID:
      func(static_cast<updateChatReadInbox &>(obj));
      return true;
    case updateChatReadOutbox::ID:
      func(static_cast<updateChatReadOutbox &>(obj));
      return true;
    case updateChatActionBar::ID:
      func(static_cast<updateChatActionBar &>(obj));
      return true;
    case updateChatBusinessBotManageBar::ID:
      func(static_cast<updateChatBusinessBotManageBar &>(obj));
      return true;
    case updateChatAvailableReactions::ID:
      func(static_cast<updateChatAvailableReactions &>(obj));
      return true;
    case updateChatDraftMessage::ID:
      func(static_cast<updateChatDraftMessage &>(obj));
      return true;
    case updateChatEmojiStatus::ID:
      func(static_cast<updateChatEmojiStatus &>(obj));
      return true;
    case updateChatMessageSender::ID:
      func(static_cast<updateChatMessageSender &>(obj));
      return true;
    case updateChatMessageAutoDeleteTime::ID:
      func(static_cast<updateChatMessageAutoDeleteTime &>(obj));
      return true;
    case updateChatNotificationSettings::ID:
      func(static_cast<updateChatNotificationSettings &>(obj));
      return true;
    case updateChatPendingJoinRequests::ID:
      func(static_cast<updateChatPendingJoinRequests &>(obj));
      return true;
    case updateChatReplyMarkup::ID:
      func(static_cast<updateChatReplyMarkup &>(obj));
      return true;
    case updateChatBackground::ID:
      func(static_cast<updateChatBackground &>(obj));
      return true;
    case updateChatTheme::ID:
      func(static_cast<updateChatTheme &>(obj));
      return true;
    case updateChatUnreadMentionCount::ID:
      func(static_cast<updateChatUnreadMentionCount &>(obj));
      return true;
    case updateChatUnreadReactionCount::ID:
      func(static_cast<updateChatUnreadReactionCount &>(obj));
      return true;
    case updateChatVideoChat::ID:
      func(static_cast<updateChatVideoChat &>(obj));
      return true;
    case updateChatDefaultDisableNotification::ID:
      func(static_cast<updateChatDefaultDisableNotification &>(obj));
      return true;
    case updateChatHasProtectedContent::ID:
      func(static_cast<updateChatHasProtectedContent &>(obj));
      return true;
    case updateChatIsTranslatable::ID:
      func(static_cast<updateChatIsTranslatable &>(obj));
      return true;
    case updateChatIsMarkedAsUnread::ID:
      func(static_cast<updateChatIsMarkedAsUnread &>(obj));
      return true;
    case updateChatViewAsTopics::ID:
      func(static_cast<updateChatViewAsTopics &>(obj));
      return true;
    case updateChatBlockList::ID:
      func(static_cast<updateChatBlockList &>(obj));
      return true;
    case updateChatHasScheduledMessages::ID:
      func(static_cast<updateChatHasScheduledMessages &>(obj));
      return true;
    case updateChatFolders::ID:
      func(static_cast<updateChatFolders &>(obj));
      return true;
    case updateChatOnlineMemberCount::ID:
      func(static_cast<updateChatOnlineMemberCount &>(obj));
      return true;
    case updateSavedMessagesTopic::ID:
      func(static_cast<updateSavedMessagesTopic &>(obj));
      return true;
    case updateSavedMessagesTopicCount::ID:
      func(static_cast<updateSavedMessagesTopicCount &>(obj));
      return true;
    case updateQuickReplyShortcut::ID:
      func(static_cast<updateQuickReplyShortcut &>(obj));
      return true;
    case updateQuickReplyShortcutDeleted::ID:
      func(static_cast<updateQuickReplyShortcutDeleted &>(obj));
      return true;
    case updateQuickReplyShortcuts::ID:
      func(static_cast<updateQuickReplyShortcuts &>(obj));
      return true;
    case updateQuickReplyShortcutMessages::ID:
      func(static_cast<updateQuickReplyShortcutMessages &>(obj));
      return true;
    case updateForumTopicInfo::ID:
      func(static_cast<updateForumTopicInfo &>(obj));
      return true;
    case updateScopeNotificationSettings::ID:
      func(static_cast<updateScopeNotificationSettings &>(obj));
      return true;
    case updateReactionNotificationSettings::ID:
      func(static_cast<updateReactionNotificationSettings &>(obj));
      return true;
    case updateNotification::ID:
      func(static_cast<updateNotification &>(obj));
      return true;
    case updateNotificationGroup::ID:
      func(static_cast<updateNotificationGroup &>(obj));
      return true;
    case updateActiveNotifications::ID:
      func(static_cast<updateActiveNotifications &>(obj));
      return true;
    case updateHavePendingNotifications::ID:
      func(static_cast<updateHavePendingNotifications &>(obj));
      return true;
    case updateDeleteMessages::ID:
      func(static_cast<updateDeleteMessages &>(obj));
      return true;
    case updateChatAction::ID:
      func(static_cast<updateChatAction &>(obj));
      return true;
    case updateUserStatus::ID:
      func(static_cast<updateUserStatus &>(obj));
      return true;
    case updateUser::ID:
      func(static_cast<updateUser &>(obj));
      return true;
    case updateBasicGroup::ID:
      func(static_cast<updateBasicGroup &>(obj));
      return true;
    case updateSupergroup::ID:
      func(static_cast<updateSupergroup &>(obj));
      return true;
    case updateSecretChat::ID:
      func(static_cast<updateSecretChat &>(obj));
      return true;
    case updateUserFullInfo::ID:
      func(static_cast<updateUserFullInfo &>(obj));
      return true;
    case updateBasicGroupFullInfo::ID:
      func(static_cast<updateBasicGroupFullInfo &>(obj));
      return true;
    case updateSupergroupFullInfo::ID:
      func(static_cast<updateSupergroupFullInfo &>(obj));
      return true;
    case updateServiceNotification::ID:
      func(static_cast<updateServiceNotification &>(obj));
      return true;
    case updateFile::ID:
      func(static_cast<updateFile &>(obj));
      return true;
    case updateFileGenerationStart::ID:
      func(static_cast<updateFileGenerationStart &>(obj));
      return true;
    case updateFileGenerationStop::ID:
      func(static_cast<updateFileGenerationStop &>(obj));
      return true;
    case updateFileDownloads::ID:
      func(static_cast<updateFileDownloads &>(obj));
      return true;
    case updateFileAddedToDownloads::ID:
      func(static_cast<updateFileAddedToDownloads &>(obj));
      return true;
    case updateFileDownload::ID:
      func(static_cast<updateFileDownload &>(obj));
      return true;
    case updateFileRemovedFromDownloads::ID:
      func(static_cast<updateFileRemovedFromDownloads &>(obj));
      return true;
    case updateApplicationVerificationRequired::ID:
      func(static_cast<updateApplicationVerificationRequired &>(obj));
      return true;
    case updateCall::ID:
      func(static_cast<updateCall &>(obj));
      return true;
    case updateGroupCall::ID:
      func(static_cast<updateGroupCall &>(obj));
      return true;
    case updateGroupCallParticipant::ID:
      func(static_cast<updateGroupCallParticipant &>(obj));
      return true;
    case updateNewCallSignalingData::ID:
      func(static_cast<updateNewCallSignalingData &>(obj));
      return true;
    case updateUserPrivacySettingRules::ID:
      func(static_cast<updateUserPrivacySettingRules &>(obj));
      return true;
    case updateUnreadMessageCount::ID:
      func(static_cast<updateUnreadMessageCount &>(obj));
      return true;
    case updateUnreadChatCount::ID:
      func(static_cast<updateUnreadChatCount &>(obj));
      return true;
    case updateStory::ID:
      func(static_cast<updateStory &>(obj));
      return true;
    case updateStoryDeleted::ID:
      func(static_cast<updateStoryDeleted &>(obj));
      return true;
    case updateStorySendSucceeded::ID:
      func(static_cast<updateStorySendSucceeded &>(obj));
      return true;
    case updateStorySendFailed::ID:
      func(static_cast<updateStorySendFailed &>(obj));
      return true;
    case updateChatActiveStories::ID:
      func(static_cast<updateChatActiveStories &>(obj));
      return true;
    case updateStoryListChatCount::ID:
      func(static_cast<updateStoryListChatCount &>(obj));
      return true;
    case updateStoryStealthMode::ID:
      func(static_cast<updateStoryStealthMode &>(obj));
      return true;
    case updateOption::ID:
      func(static_cast<updateOption &>(obj));
      return true;
    case updateStickerSet::ID:
      func(static_cast<updateStickerSet &>(obj));
      return true;
    case updateInstalledStickerSets::ID:
      func(static_cast<updateInstalledStickerSets &>(obj));
      return true;
    case updateTrendingStickerSets::ID:
      func(static_cast<updateTrendingStickerSets &>(obj));
      return true;
    case updateRecentStickers::ID:
      func(static_cast<updateRecentStickers &>(obj));
      return true;
    case updateFavoriteStickers::ID:
      func(static_cast<updateFavoriteStickers &>(obj));
      return true;
    case updateSavedAnimations::ID:
      func(static_cast<updateSavedAnimations &>(obj));
      return true;
    case updateSavedNotificationSounds::ID:
      func(static_cast<updateSavedNotificationSounds &>(obj));
      return true;
    case updateDefaultBackground::ID:
      func(static_cast<updateDefaultBackground &>(obj));
      return true;
    case updateChatThemes::ID:
      func(static_cast<updateChatThemes &>(obj));
      return true;
    case updateAccentColors::ID:
      func(static_cast<updateAccentColors &>(obj));
      return true;
    case updateProfileAccentColors::ID:
      func(static_cast<updateProfileAccentColors &>(obj));
      return true;
    case updateLanguagePackStrings::ID:
      func(static_cast<updateLanguagePackStrings &>(obj));
      return true;
    case updateConnectionState::ID:
      func(static_cast<updateConnectionState &>(obj));
      return true;
    case updateTermsOfService::ID:
      func(static_cast<updateTermsOfService &>(obj));
      return true;
    case updateUsersNearby::ID:
      func(static_cast<updateUsersNearby &>(obj));
      return true;
    case updateUnconfirmedSession::ID:
      func(static_cast<updateUnconfirmedSession &>(obj));
      return true;
    case updateAttachmentMenuBots::ID:
      func(static_cast<updateAttachmentMenuBots &>(obj));
      return true;
    case updateWebAppMessageSent::ID:
      func(static_cast<updateWebAppMessageSent &>(obj));
      return true;
    case updateActiveEmojiReactions::ID:
      func(static_cast<updateActiveEmojiReactions &>(obj));
      return true;
    case updateAvailableMessageEffects::ID:
      func(static_cast<updateAvailableMessageEffects &>(obj));
      return true;
    case updateDefaultReactionType::ID:
      func(static_cast<updateDefaultReactionType &>(obj));
      return true;
    case updateSavedMessagesTags::ID:
      func(static_cast<updateSavedMessagesTags &>(obj));
      return true;
    case updateActiveLiveLocationMessages::ID:
      func(static_cast<updateActiveLiveLocationMessages &>(obj));
      return true;
    case updateOwnedStarCount::ID:
      func(static_cast<updateOwnedStarCount &>(obj));
      return true;
    case updateChatRevenueAmount::ID:
      func(static_cast<updateChatRevenueAmount &>(obj));
      return true;
    case updateStarRevenueStatus::ID:
      func(static_cast<updateStarRevenueStatus &>(obj));
      return true;
    case updateSpeechRecognitionTrial::ID:
      func(static_cast<updateSpeechRecognitionTrial &>(obj));
      return true;
    case updateDiceEmojis::ID:
      func(static_cast<updateDiceEmojis &>(obj));
      return true;
    case updateAnimatedEmojiMessageClicked::ID:
      func(static_cast<updateAnimatedEmojiMessageClicked &>(obj));
      return true;
    case updateAnimationSearchParameters::ID:
      func(static_cast<updateAnimationSearchParameters &>(obj));
      return true;
    case updateSuggestedActions::ID:
      func(static_cast<updateSuggestedActions &>(obj));
      return true;
    case updateSpeedLimitNotification::ID:
      func(static_cast<updateSpeedLimitNotification &>(obj));
      return true;
    case updateContactCloseBirthdays::ID:
      func(static_cast<updateContactCloseBirthdays &>(obj));
      return true;
    case updateAutosaveSettings::ID:
      func(static_cast<updateAutosaveSettings &>(obj));
      return true;
    case updateBusinessConnection::ID:
      func(static_cast<updateBusinessConnection &>(obj));
      return true;
    case updateNewBusinessMessage::ID:
      func(static_cast<updateNewBusinessMessage &>(obj));
      return true;
    case updateBusinessMessageEdited::ID:
      func(static_cast<updateBusinessMessageEdited &>(obj));
      return true;
    case updateBusinessMessagesDeleted::ID:
      func(static_cast<updateBusinessMessagesDeleted &>(obj));
      return true;
    case updateNewInlineQuery::ID:
      func(static_cast<updateNewInlineQuery &>(obj));
      return true;
    case updateNewChosenInlineResult::ID:
      func(static_cast<updateNewChosenInlineResult &>(obj));
      return true;
    case updateNewCallbackQuery::ID:
      func(static_cast<updateNewCallbackQuery &>(obj));
      return true;
    case updateNewInlineCallbackQuery::ID:
      func(static_cast<updateNewInlineCallbackQuery &>(obj));
      return true;
    case updateNewBusinessCallbackQuery::ID:
      func(static_cast<updateNewBusinessCallbackQuery &>(obj));
      return true;
    case updateNewShippingQuery::ID:
      func(static_cast<updateNewShippingQuery &>(obj));
      return true;
    case updateNewPreCheckoutQuery::ID:
      func(static_cast<updateNewPreCheckoutQuery &>(obj));
      return true;
    case updateNewCustomEvent::ID:
      func(static_cast<updateNewCustomEvent &>(obj));
      return true;
    case updateNewCustomQuery::ID:
      func(static_cast<updateNewCustomQuery &>(obj));
      return true;
    case updatePoll::ID:
      func(static_cast<updatePoll &>(obj));
      return true;
    case updatePollAnswer::ID:
      func(static_cast<updatePollAnswer &>(obj));
      return true;
    case updateChatMember::ID:
      func(static_cast<updateChatMember &>(obj));
      return true;
    case updateNewChatJoinRequest::ID:
      func(static_cast<updateNewChatJoinRequest &>(obj));
      return true;
    case updateChatBoost::ID:
      func(static_cast<updateChatBoost &>(obj));
      return true;
    case updateMessageReaction::ID:
      func(static_cast<updateMessageReaction &>(obj));
      return true;
    case updateMessageReactions::ID:
      func(static_cast<updateMessageReactions &>(obj));
      return true;
    case updatePaidMediaPurchased::ID:
      func(static_cast<updatePaidMediaPurchased &>(obj));
      return true;
    case updates::ID:
      func(static_cast<updates &>(obj));
      return true;
    case user::ID:
      func(static_cast<user &>(obj));
      return true;
    case userFullInfo::ID:
      func(static_cast<userFullInfo &>(obj));
      return true;
    case userLink::ID:
      func(static_cast<userLink &>(obj));
      return true;
    case userPrivacySettingShowStatus::ID:
      func(static_cast<userPrivacySettingShowStatus &>(obj));
      return true;
    case userPrivacySettingShowProfilePhoto::ID:
      func(static_cast<userPrivacySettingShowProfilePhoto &>(obj));
      return true;
    case userPrivacySettingShowLinkInForwardedMessages::ID:
      func(static_cast<userPrivacySettingShowLinkInForwardedMessages &>(obj));
      return true;
    case userPrivacySettingShowPhoneNumber::ID:
      func(static_cast<userPrivacySettingShowPhoneNumber &>(obj));
      return true;
    case userPrivacySettingShowBio::ID:
      func(static_cast<userPrivacySettingShowBio &>(obj));
      return true;
    case userPrivacySettingShowBirthdate::ID:
      func(static_cast<userPrivacySettingShowBirthdate &>(obj));
      return true;
    case userPrivacySettingAllowChatInvites::ID:
      func(static_cast<userPrivacySettingAllowChatInvites &>(obj));
      return true;
    case userPrivacySettingAllowCalls::ID:
      func(static_cast<userPrivacySettingAllowCalls &>(obj));
      return true;
    case userPrivacySettingAllowPeerToPeerCalls::ID:
      func(static_cast<userPrivacySettingAllowPeerToPeerCalls &>(obj));
      return true;
    case userPrivacySettingAllowFindingByPhoneNumber::ID:
      func(static_cast<userPrivacySettingAllowFindingByPhoneNumber &>(obj));
      return true;
    case userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages::ID:
      func(static_cast<userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &>(obj));
      return true;
    case userPrivacySettingRuleAllowAll::ID:
      func(static_cast<userPrivacySettingRuleAllowAll &>(obj));
      return true;
    case userPrivacySettingRuleAllowContacts::ID:
      func(static_cast<userPrivacySettingRuleAllowContacts &>(obj));
      return true;
    case userPrivacySettingRuleAllowPremiumUsers::ID:
      func(static_cast<userPrivacySettingRuleAllowPremiumUsers &>(obj));
      return true;
    case userPrivacySettingRuleAllowUsers::ID:
      func(static_cast<userPrivacySettingRuleAllowUsers &>(obj));
      return true;
    case userPrivacySettingRuleAllowChatMembers::ID:
      func(static_cast<userPrivacySettingRuleAllowChatMembers &>(obj));
      return true;
    case userPrivacySettingRuleRestrictAll::ID:
      func(static_cast<userPrivacySettingRuleRestrictAll &>(obj));
      return true;
    case userPrivacySettingRuleRestrictContacts::ID:
      func(static_cast<userPrivacySettingRuleRestrictContacts &>(obj));
      return true;
    case userPrivacySettingRuleRestrictUsers::ID:
      func(static_cast<userPrivacySettingRuleRestrictUsers &>(obj));
      return true;
    case userPrivacySettingRuleRestrictChatMembers::ID:
      func(static_cast<userPrivacySettingRuleRestrictChatMembers &>(obj));
      return true;
    case userPrivacySettingRules::ID:
      func(static_cast<userPrivacySettingRules &>(obj));
      return true;
    case userStatusEmpty::ID:
      func(static_cast<userStatusEmpty &>(obj));
      return true;
    case userStatusOnline::ID:
      func(static_cast<userStatusOnline &>(obj));
      return true;
    case userStatusOffline::ID:
      func(static_cast<userStatusOffline &>(obj));
      return true;
    case userStatusRecently::ID:
      func(static_cast<userStatusRecently &>(obj));
      return true;
    case userStatusLastWeek::ID:
      func(static_cast<userStatusLastWeek &>(obj));
      return true;
    case userStatusLastMonth::ID:
      func(static_cast<userStatusLastMonth &>(obj));
      return true;
    case userSupportInfo::ID:
      func(static_cast<userSupportInfo &>(obj));
      return true;
    case userTypeRegular::ID:
      func(static_cast<userTypeRegular &>(obj));
      return true;
    case userTypeDeleted::ID:
      func(static_cast<userTypeDeleted &>(obj));
      return true;
    case userTypeBot::ID:
      func(static_cast<userTypeBot &>(obj));
      return true;
    case userTypeUnknown::ID:
      func(static_cast<userTypeUnknown &>(obj));
      return true;
    case usernames::ID:
      func(static_cast<usernames &>(obj));
      return true;
    case users::ID:
      func(static_cast<users &>(obj));
      return true;
    case validatedOrderInfo::ID:
      func(static_cast<validatedOrderInfo &>(obj));
      return true;
    case vectorPathCommandLine::ID:
      func(static_cast<vectorPathCommandLine &>(obj));
      return true;
    case vectorPathCommandCubicBezierCurve::ID:
      func(static_cast<vectorPathCommandCubicBezierCurve &>(obj));
      return true;
    case venue::ID:
      func(static_cast<venue &>(obj));
      return true;
    case video::ID:
      func(static_cast<video &>(obj));
      return true;
    case videoChat::ID:
      func(static_cast<videoChat &>(obj));
      return true;
    case videoNote::ID:
      func(static_cast<videoNote &>(obj));
      return true;
    case voiceNote::ID:
      func(static_cast<voiceNote &>(obj));
      return true;
    case webApp::ID:
      func(static_cast<webApp &>(obj));
      return true;
    case webAppInfo::ID:
      func(static_cast<webAppInfo &>(obj));
      return true;
    case webPageInstantView::ID:
      func(static_cast<webPageInstantView &>(obj));
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
    case acceptCall::ID:
      func(static_cast<acceptCall &>(obj));
      return true;
    case acceptTermsOfService::ID:
      func(static_cast<acceptTermsOfService &>(obj));
      return true;
    case activateStoryStealthMode::ID:
      func(static_cast<activateStoryStealthMode &>(obj));
      return true;
    case addBotMediaPreview::ID:
      func(static_cast<addBotMediaPreview &>(obj));
      return true;
    case addChatFolderByInviteLink::ID:
      func(static_cast<addChatFolderByInviteLink &>(obj));
      return true;
    case addChatMember::ID:
      func(static_cast<addChatMember &>(obj));
      return true;
    case addChatMembers::ID:
      func(static_cast<addChatMembers &>(obj));
      return true;
    case addChatToList::ID:
      func(static_cast<addChatToList &>(obj));
      return true;
    case addContact::ID:
      func(static_cast<addContact &>(obj));
      return true;
    case addCustomServerLanguagePack::ID:
      func(static_cast<addCustomServerLanguagePack &>(obj));
      return true;
    case addFavoriteSticker::ID:
      func(static_cast<addFavoriteSticker &>(obj));
      return true;
    case addFileToDownloads::ID:
      func(static_cast<addFileToDownloads &>(obj));
      return true;
    case addLocalMessage::ID:
      func(static_cast<addLocalMessage &>(obj));
      return true;
    case addLogMessage::ID:
      func(static_cast<addLogMessage &>(obj));
      return true;
    case addMessageReaction::ID:
      func(static_cast<addMessageReaction &>(obj));
      return true;
    case addNetworkStatistics::ID:
      func(static_cast<addNetworkStatistics &>(obj));
      return true;
    case addPendingPaidMessageReaction::ID:
      func(static_cast<addPendingPaidMessageReaction &>(obj));
      return true;
    case addProxy::ID:
      func(static_cast<addProxy &>(obj));
      return true;
    case addQuickReplyShortcutInlineQueryResultMessage::ID:
      func(static_cast<addQuickReplyShortcutInlineQueryResultMessage &>(obj));
      return true;
    case addQuickReplyShortcutMessage::ID:
      func(static_cast<addQuickReplyShortcutMessage &>(obj));
      return true;
    case addQuickReplyShortcutMessageAlbum::ID:
      func(static_cast<addQuickReplyShortcutMessageAlbum &>(obj));
      return true;
    case addRecentSticker::ID:
      func(static_cast<addRecentSticker &>(obj));
      return true;
    case addRecentlyFoundChat::ID:
      func(static_cast<addRecentlyFoundChat &>(obj));
      return true;
    case addSavedAnimation::ID:
      func(static_cast<addSavedAnimation &>(obj));
      return true;
    case addSavedNotificationSound::ID:
      func(static_cast<addSavedNotificationSound &>(obj));
      return true;
    case addStickerToSet::ID:
      func(static_cast<addStickerToSet &>(obj));
      return true;
    case allowBotToSendMessages::ID:
      func(static_cast<allowBotToSendMessages &>(obj));
      return true;
    case answerCallbackQuery::ID:
      func(static_cast<answerCallbackQuery &>(obj));
      return true;
    case answerCustomQuery::ID:
      func(static_cast<answerCustomQuery &>(obj));
      return true;
    case answerInlineQuery::ID:
      func(static_cast<answerInlineQuery &>(obj));
      return true;
    case answerPreCheckoutQuery::ID:
      func(static_cast<answerPreCheckoutQuery &>(obj));
      return true;
    case answerShippingQuery::ID:
      func(static_cast<answerShippingQuery &>(obj));
      return true;
    case answerWebAppQuery::ID:
      func(static_cast<answerWebAppQuery &>(obj));
      return true;
    case applyPremiumGiftCode::ID:
      func(static_cast<applyPremiumGiftCode &>(obj));
      return true;
    case assignAppStoreTransaction::ID:
      func(static_cast<assignAppStoreTransaction &>(obj));
      return true;
    case assignGooglePlayTransaction::ID:
      func(static_cast<assignGooglePlayTransaction &>(obj));
      return true;
    case banChatMember::ID:
      func(static_cast<banChatMember &>(obj));
      return true;
    case blockMessageSenderFromReplies::ID:
      func(static_cast<blockMessageSenderFromReplies &>(obj));
      return true;
    case boostChat::ID:
      func(static_cast<boostChat &>(obj));
      return true;
    case canBotSendMessages::ID:
      func(static_cast<canBotSendMessages &>(obj));
      return true;
    case canPurchaseFromStore::ID:
      func(static_cast<canPurchaseFromStore &>(obj));
      return true;
    case canSendMessageToUser::ID:
      func(static_cast<canSendMessageToUser &>(obj));
      return true;
    case canSendStory::ID:
      func(static_cast<canSendStory &>(obj));
      return true;
    case canTransferOwnership::ID:
      func(static_cast<canTransferOwnership &>(obj));
      return true;
    case cancelDownloadFile::ID:
      func(static_cast<cancelDownloadFile &>(obj));
      return true;
    case cancelPasswordReset::ID:
      func(static_cast<cancelPasswordReset &>(obj));
      return true;
    case cancelPreliminaryUploadFile::ID:
      func(static_cast<cancelPreliminaryUploadFile &>(obj));
      return true;
    case cancelRecoveryEmailAddressVerification::ID:
      func(static_cast<cancelRecoveryEmailAddressVerification &>(obj));
      return true;
    case changeImportedContacts::ID:
      func(static_cast<changeImportedContacts &>(obj));
      return true;
    case changeStickerSet::ID:
      func(static_cast<changeStickerSet &>(obj));
      return true;
    case checkAuthenticationBotToken::ID:
      func(static_cast<checkAuthenticationBotToken &>(obj));
      return true;
    case checkAuthenticationCode::ID:
      func(static_cast<checkAuthenticationCode &>(obj));
      return true;
    case checkAuthenticationEmailCode::ID:
      func(static_cast<checkAuthenticationEmailCode &>(obj));
      return true;
    case checkAuthenticationPassword::ID:
      func(static_cast<checkAuthenticationPassword &>(obj));
      return true;
    case checkAuthenticationPasswordRecoveryCode::ID:
      func(static_cast<checkAuthenticationPasswordRecoveryCode &>(obj));
      return true;
    case checkChatFolderInviteLink::ID:
      func(static_cast<checkChatFolderInviteLink &>(obj));
      return true;
    case checkChatInviteLink::ID:
      func(static_cast<checkChatInviteLink &>(obj));
      return true;
    case checkChatUsername::ID:
      func(static_cast<checkChatUsername &>(obj));
      return true;
    case checkCreatedPublicChatsLimit::ID:
      func(static_cast<checkCreatedPublicChatsLimit &>(obj));
      return true;
    case checkEmailAddressVerificationCode::ID:
      func(static_cast<checkEmailAddressVerificationCode &>(obj));
      return true;
    case checkLoginEmailAddressCode::ID:
      func(static_cast<checkLoginEmailAddressCode &>(obj));
      return true;
    case checkPasswordRecoveryCode::ID:
      func(static_cast<checkPasswordRecoveryCode &>(obj));
      return true;
    case checkPhoneNumberCode::ID:
      func(static_cast<checkPhoneNumberCode &>(obj));
      return true;
    case checkPremiumGiftCode::ID:
      func(static_cast<checkPremiumGiftCode &>(obj));
      return true;
    case checkQuickReplyShortcutName::ID:
      func(static_cast<checkQuickReplyShortcutName &>(obj));
      return true;
    case checkRecoveryEmailAddressCode::ID:
      func(static_cast<checkRecoveryEmailAddressCode &>(obj));
      return true;
    case checkStickerSetName::ID:
      func(static_cast<checkStickerSetName &>(obj));
      return true;
    case cleanFileName::ID:
      func(static_cast<cleanFileName &>(obj));
      return true;
    case clearAllDraftMessages::ID:
      func(static_cast<clearAllDraftMessages &>(obj));
      return true;
    case clearAutosaveSettingsExceptions::ID:
      func(static_cast<clearAutosaveSettingsExceptions &>(obj));
      return true;
    case clearImportedContacts::ID:
      func(static_cast<clearImportedContacts &>(obj));
      return true;
    case clearRecentEmojiStatuses::ID:
      func(static_cast<clearRecentEmojiStatuses &>(obj));
      return true;
    case clearRecentReactions::ID:
      func(static_cast<clearRecentReactions &>(obj));
      return true;
    case clearRecentStickers::ID:
      func(static_cast<clearRecentStickers &>(obj));
      return true;
    case clearRecentlyFoundChats::ID:
      func(static_cast<clearRecentlyFoundChats &>(obj));
      return true;
    case clearSearchedForTags::ID:
      func(static_cast<clearSearchedForTags &>(obj));
      return true;
    case clickAnimatedEmojiMessage::ID:
      func(static_cast<clickAnimatedEmojiMessage &>(obj));
      return true;
    case clickChatSponsoredMessage::ID:
      func(static_cast<clickChatSponsoredMessage &>(obj));
      return true;
    case clickPremiumSubscriptionButton::ID:
      func(static_cast<clickPremiumSubscriptionButton &>(obj));
      return true;
    case close::ID:
      func(static_cast<close &>(obj));
      return true;
    case closeChat::ID:
      func(static_cast<closeChat &>(obj));
      return true;
    case closeSecretChat::ID:
      func(static_cast<closeSecretChat &>(obj));
      return true;
    case closeStory::ID:
      func(static_cast<closeStory &>(obj));
      return true;
    case closeWebApp::ID:
      func(static_cast<closeWebApp &>(obj));
      return true;
    case commitPendingPaidMessageReactions::ID:
      func(static_cast<commitPendingPaidMessageReactions &>(obj));
      return true;
    case confirmQrCodeAuthentication::ID:
      func(static_cast<confirmQrCodeAuthentication &>(obj));
      return true;
    case confirmSession::ID:
      func(static_cast<confirmSession &>(obj));
      return true;
    case createBasicGroupChat::ID:
      func(static_cast<createBasicGroupChat &>(obj));
      return true;
    case createBusinessChatLink::ID:
      func(static_cast<createBusinessChatLink &>(obj));
      return true;
    case createCall::ID:
      func(static_cast<createCall &>(obj));
      return true;
    case createChatFolder::ID:
      func(static_cast<createChatFolder &>(obj));
      return true;
    case createChatFolderInviteLink::ID:
      func(static_cast<createChatFolderInviteLink &>(obj));
      return true;
    case createChatInviteLink::ID:
      func(static_cast<createChatInviteLink &>(obj));
      return true;
    case createChatSubscriptionInviteLink::ID:
      func(static_cast<createChatSubscriptionInviteLink &>(obj));
      return true;
    case createForumTopic::ID:
      func(static_cast<createForumTopic &>(obj));
      return true;
    case createInvoiceLink::ID:
      func(static_cast<createInvoiceLink &>(obj));
      return true;
    case createNewBasicGroupChat::ID:
      func(static_cast<createNewBasicGroupChat &>(obj));
      return true;
    case createNewSecretChat::ID:
      func(static_cast<createNewSecretChat &>(obj));
      return true;
    case createNewStickerSet::ID:
      func(static_cast<createNewStickerSet &>(obj));
      return true;
    case createNewSupergroupChat::ID:
      func(static_cast<createNewSupergroupChat &>(obj));
      return true;
    case createPrivateChat::ID:
      func(static_cast<createPrivateChat &>(obj));
      return true;
    case createSecretChat::ID:
      func(static_cast<createSecretChat &>(obj));
      return true;
    case createSupergroupChat::ID:
      func(static_cast<createSupergroupChat &>(obj));
      return true;
    case createTemporaryPassword::ID:
      func(static_cast<createTemporaryPassword &>(obj));
      return true;
    case createVideoChat::ID:
      func(static_cast<createVideoChat &>(obj));
      return true;
    case deleteAccount::ID:
      func(static_cast<deleteAccount &>(obj));
      return true;
    case deleteAllCallMessages::ID:
      func(static_cast<deleteAllCallMessages &>(obj));
      return true;
    case deleteAllRevokedChatInviteLinks::ID:
      func(static_cast<deleteAllRevokedChatInviteLinks &>(obj));
      return true;
    case deleteBotMediaPreviews::ID:
      func(static_cast<deleteBotMediaPreviews &>(obj));
      return true;
    case deleteBusinessChatLink::ID:
      func(static_cast<deleteBusinessChatLink &>(obj));
      return true;
    case deleteBusinessConnectedBot::ID:
      func(static_cast<deleteBusinessConnectedBot &>(obj));
      return true;
    case deleteChat::ID:
      func(static_cast<deleteChat &>(obj));
      return true;
    case deleteChatBackground::ID:
      func(static_cast<deleteChatBackground &>(obj));
      return true;
    case deleteChatFolder::ID:
      func(static_cast<deleteChatFolder &>(obj));
      return true;
    case deleteChatFolderInviteLink::ID:
      func(static_cast<deleteChatFolderInviteLink &>(obj));
      return true;
    case deleteChatHistory::ID:
      func(static_cast<deleteChatHistory &>(obj));
      return true;
    case deleteChatMessagesByDate::ID:
      func(static_cast<deleteChatMessagesByDate &>(obj));
      return true;
    case deleteChatMessagesBySender::ID:
      func(static_cast<deleteChatMessagesBySender &>(obj));
      return true;
    case deleteChatReplyMarkup::ID:
      func(static_cast<deleteChatReplyMarkup &>(obj));
      return true;
    case deleteCommands::ID:
      func(static_cast<deleteCommands &>(obj));
      return true;
    case deleteDefaultBackground::ID:
      func(static_cast<deleteDefaultBackground &>(obj));
      return true;
    case deleteFile::ID:
      func(static_cast<deleteFile &>(obj));
      return true;
    case deleteForumTopic::ID:
      func(static_cast<deleteForumTopic &>(obj));
      return true;
    case deleteLanguagePack::ID:
      func(static_cast<deleteLanguagePack &>(obj));
      return true;
    case deleteMessages::ID:
      func(static_cast<deleteMessages &>(obj));
      return true;
    case deletePassportElement::ID:
      func(static_cast<deletePassportElement &>(obj));
      return true;
    case deleteProfilePhoto::ID:
      func(static_cast<deleteProfilePhoto &>(obj));
      return true;
    case deleteQuickReplyShortcut::ID:
      func(static_cast<deleteQuickReplyShortcut &>(obj));
      return true;
    case deleteQuickReplyShortcutMessages::ID:
      func(static_cast<deleteQuickReplyShortcutMessages &>(obj));
      return true;
    case deleteRevokedChatInviteLink::ID:
      func(static_cast<deleteRevokedChatInviteLink &>(obj));
      return true;
    case deleteSavedCredentials::ID:
      func(static_cast<deleteSavedCredentials &>(obj));
      return true;
    case deleteSavedMessagesTopicHistory::ID:
      func(static_cast<deleteSavedMessagesTopicHistory &>(obj));
      return true;
    case deleteSavedMessagesTopicMessagesByDate::ID:
      func(static_cast<deleteSavedMessagesTopicMessagesByDate &>(obj));
      return true;
    case deleteSavedOrderInfo::ID:
      func(static_cast<deleteSavedOrderInfo &>(obj));
      return true;
    case deleteStickerSet::ID:
      func(static_cast<deleteStickerSet &>(obj));
      return true;
    case deleteStory::ID:
      func(static_cast<deleteStory &>(obj));
      return true;
    case destroy::ID:
      func(static_cast<destroy &>(obj));
      return true;
    case disableAllSupergroupUsernames::ID:
      func(static_cast<disableAllSupergroupUsernames &>(obj));
      return true;
    case disableProxy::ID:
      func(static_cast<disableProxy &>(obj));
      return true;
    case discardCall::ID:
      func(static_cast<discardCall &>(obj));
      return true;
    case disconnectAllWebsites::ID:
      func(static_cast<disconnectAllWebsites &>(obj));
      return true;
    case disconnectWebsite::ID:
      func(static_cast<disconnectWebsite &>(obj));
      return true;
    case downloadFile::ID:
      func(static_cast<downloadFile &>(obj));
      return true;
    case editBotMediaPreview::ID:
      func(static_cast<editBotMediaPreview &>(obj));
      return true;
    case editBusinessChatLink::ID:
      func(static_cast<editBusinessChatLink &>(obj));
      return true;
    case editBusinessMessageCaption::ID:
      func(static_cast<editBusinessMessageCaption &>(obj));
      return true;
    case editBusinessMessageLiveLocation::ID:
      func(static_cast<editBusinessMessageLiveLocation &>(obj));
      return true;
    case editBusinessMessageMedia::ID:
      func(static_cast<editBusinessMessageMedia &>(obj));
      return true;
    case editBusinessMessageReplyMarkup::ID:
      func(static_cast<editBusinessMessageReplyMarkup &>(obj));
      return true;
    case editBusinessMessageText::ID:
      func(static_cast<editBusinessMessageText &>(obj));
      return true;
    case editChatFolder::ID:
      func(static_cast<editChatFolder &>(obj));
      return true;
    case editChatFolderInviteLink::ID:
      func(static_cast<editChatFolderInviteLink &>(obj));
      return true;
    case editChatInviteLink::ID:
      func(static_cast<editChatInviteLink &>(obj));
      return true;
    case editChatSubscriptionInviteLink::ID:
      func(static_cast<editChatSubscriptionInviteLink &>(obj));
      return true;
    case editCustomLanguagePackInfo::ID:
      func(static_cast<editCustomLanguagePackInfo &>(obj));
      return true;
    case editForumTopic::ID:
      func(static_cast<editForumTopic &>(obj));
      return true;
    case editInlineMessageCaption::ID:
      func(static_cast<editInlineMessageCaption &>(obj));
      return true;
    case editInlineMessageLiveLocation::ID:
      func(static_cast<editInlineMessageLiveLocation &>(obj));
      return true;
    case editInlineMessageMedia::ID:
      func(static_cast<editInlineMessageMedia &>(obj));
      return true;
    case editInlineMessageReplyMarkup::ID:
      func(static_cast<editInlineMessageReplyMarkup &>(obj));
      return true;
    case editInlineMessageText::ID:
      func(static_cast<editInlineMessageText &>(obj));
      return true;
    case editMessageCaption::ID:
      func(static_cast<editMessageCaption &>(obj));
      return true;
    case editMessageLiveLocation::ID:
      func(static_cast<editMessageLiveLocation &>(obj));
      return true;
    case editMessageMedia::ID:
      func(static_cast<editMessageMedia &>(obj));
      return true;
    case editMessageReplyMarkup::ID:
      func(static_cast<editMessageReplyMarkup &>(obj));
      return true;
    case editMessageSchedulingState::ID:
      func(static_cast<editMessageSchedulingState &>(obj));
      return true;
    case editMessageText::ID:
      func(static_cast<editMessageText &>(obj));
      return true;
    case editProxy::ID:
      func(static_cast<editProxy &>(obj));
      return true;
    case editQuickReplyMessage::ID:
      func(static_cast<editQuickReplyMessage &>(obj));
      return true;
    case editStarSubscription::ID:
      func(static_cast<editStarSubscription &>(obj));
      return true;
    case editStory::ID:
      func(static_cast<editStory &>(obj));
      return true;
    case editStoryCover::ID:
      func(static_cast<editStoryCover &>(obj));
      return true;
    case enableProxy::ID:
      func(static_cast<enableProxy &>(obj));
      return true;
    case endGroupCall::ID:
      func(static_cast<endGroupCall &>(obj));
      return true;
    case endGroupCallRecording::ID:
      func(static_cast<endGroupCallRecording &>(obj));
      return true;
    case endGroupCallScreenSharing::ID:
      func(static_cast<endGroupCallScreenSharing &>(obj));
      return true;
    case finishFileGeneration::ID:
      func(static_cast<finishFileGeneration &>(obj));
      return true;
    case forwardMessages::ID:
      func(static_cast<forwardMessages &>(obj));
      return true;
    case getAccountTtl::ID:
      func(static_cast<getAccountTtl &>(obj));
      return true;
    case getActiveSessions::ID:
      func(static_cast<getActiveSessions &>(obj));
      return true;
    case getAllPassportElements::ID:
      func(static_cast<getAllPassportElements &>(obj));
      return true;
    case getAllStickerEmojis::ID:
      func(static_cast<getAllStickerEmojis &>(obj));
      return true;
    case getAnimatedEmoji::ID:
      func(static_cast<getAnimatedEmoji &>(obj));
      return true;
    case getApplicationConfig::ID:
      func(static_cast<getApplicationConfig &>(obj));
      return true;
    case getApplicationDownloadLink::ID:
      func(static_cast<getApplicationDownloadLink &>(obj));
      return true;
    case getArchiveChatListSettings::ID:
      func(static_cast<getArchiveChatListSettings &>(obj));
      return true;
    case getArchivedStickerSets::ID:
      func(static_cast<getArchivedStickerSets &>(obj));
      return true;
    case getAttachedStickerSets::ID:
      func(static_cast<getAttachedStickerSets &>(obj));
      return true;
    case getAttachmentMenuBot::ID:
      func(static_cast<getAttachmentMenuBot &>(obj));
      return true;
    case getAuthorizationState::ID:
      func(static_cast<getAuthorizationState &>(obj));
      return true;
    case getAutoDownloadSettingsPresets::ID:
      func(static_cast<getAutoDownloadSettingsPresets &>(obj));
      return true;
    case getAutosaveSettings::ID:
      func(static_cast<getAutosaveSettings &>(obj));
      return true;
    case getAvailableChatBoostSlots::ID:
      func(static_cast<getAvailableChatBoostSlots &>(obj));
      return true;
    case getBackgroundUrl::ID:
      func(static_cast<getBackgroundUrl &>(obj));
      return true;
    case getBankCardInfo::ID:
      func(static_cast<getBankCardInfo &>(obj));
      return true;
    case getBasicGroup::ID:
      func(static_cast<getBasicGroup &>(obj));
      return true;
    case getBasicGroupFullInfo::ID:
      func(static_cast<getBasicGroupFullInfo &>(obj));
      return true;
    case getBlockedMessageSenders::ID:
      func(static_cast<getBlockedMessageSenders &>(obj));
      return true;
    case getBotInfoDescription::ID:
      func(static_cast<getBotInfoDescription &>(obj));
      return true;
    case getBotInfoShortDescription::ID:
      func(static_cast<getBotInfoShortDescription &>(obj));
      return true;
    case getBotMediaPreviewInfo::ID:
      func(static_cast<getBotMediaPreviewInfo &>(obj));
      return true;
    case getBotMediaPreviews::ID:
      func(static_cast<getBotMediaPreviews &>(obj));
      return true;
    case getBotName::ID:
      func(static_cast<getBotName &>(obj));
      return true;
    case getBusinessChatLinkInfo::ID:
      func(static_cast<getBusinessChatLinkInfo &>(obj));
      return true;
    case getBusinessChatLinks::ID:
      func(static_cast<getBusinessChatLinks &>(obj));
      return true;
    case getBusinessConnectedBot::ID:
      func(static_cast<getBusinessConnectedBot &>(obj));
      return true;
    case getBusinessConnection::ID:
      func(static_cast<getBusinessConnection &>(obj));
      return true;
    case getBusinessFeatures::ID:
      func(static_cast<getBusinessFeatures &>(obj));
      return true;
    case getCallbackQueryAnswer::ID:
      func(static_cast<getCallbackQueryAnswer &>(obj));
      return true;
    case getCallbackQueryMessage::ID:
      func(static_cast<getCallbackQueryMessage &>(obj));
      return true;
    case getChat::ID:
      func(static_cast<getChat &>(obj));
      return true;
    case getChatActiveStories::ID:
      func(static_cast<getChatActiveStories &>(obj));
      return true;
    case getChatAdministrators::ID:
      func(static_cast<getChatAdministrators &>(obj));
      return true;
    case getChatArchivedStories::ID:
      func(static_cast<getChatArchivedStories &>(obj));
      return true;
    case getChatAvailableMessageSenders::ID:
      func(static_cast<getChatAvailableMessageSenders &>(obj));
      return true;
    case getChatBoostFeatures::ID:
      func(static_cast<getChatBoostFeatures &>(obj));
      return true;
    case getChatBoostLevelFeatures::ID:
      func(static_cast<getChatBoostLevelFeatures &>(obj));
      return true;
    case getChatBoostLink::ID:
      func(static_cast<getChatBoostLink &>(obj));
      return true;
    case getChatBoostLinkInfo::ID:
      func(static_cast<getChatBoostLinkInfo &>(obj));
      return true;
    case getChatBoostStatus::ID:
      func(static_cast<getChatBoostStatus &>(obj));
      return true;
    case getChatBoosts::ID:
      func(static_cast<getChatBoosts &>(obj));
      return true;
    case getChatEventLog::ID:
      func(static_cast<getChatEventLog &>(obj));
      return true;
    case getChatFolder::ID:
      func(static_cast<getChatFolder &>(obj));
      return true;
    case getChatFolderChatCount::ID:
      func(static_cast<getChatFolderChatCount &>(obj));
      return true;
    case getChatFolderChatsToLeave::ID:
      func(static_cast<getChatFolderChatsToLeave &>(obj));
      return true;
    case getChatFolderDefaultIconName::ID:
      func(static_cast<getChatFolderDefaultIconName &>(obj));
      return true;
    case getChatFolderInviteLinks::ID:
      func(static_cast<getChatFolderInviteLinks &>(obj));
      return true;
    case getChatFolderNewChats::ID:
      func(static_cast<getChatFolderNewChats &>(obj));
      return true;
    case getChatHistory::ID:
      func(static_cast<getChatHistory &>(obj));
      return true;
    case getChatInviteLink::ID:
      func(static_cast<getChatInviteLink &>(obj));
      return true;
    case getChatInviteLinkCounts::ID:
      func(static_cast<getChatInviteLinkCounts &>(obj));
      return true;
    case getChatInviteLinkMembers::ID:
      func(static_cast<getChatInviteLinkMembers &>(obj));
      return true;
    case getChatInviteLinks::ID:
      func(static_cast<getChatInviteLinks &>(obj));
      return true;
    case getChatJoinRequests::ID:
      func(static_cast<getChatJoinRequests &>(obj));
      return true;
    case getChatListsToAddChat::ID:
      func(static_cast<getChatListsToAddChat &>(obj));
      return true;
    case getChatMember::ID:
      func(static_cast<getChatMember &>(obj));
      return true;
    case getChatMessageByDate::ID:
      func(static_cast<getChatMessageByDate &>(obj));
      return true;
    case getChatMessageCalendar::ID:
      func(static_cast<getChatMessageCalendar &>(obj));
      return true;
    case getChatMessageCount::ID:
      func(static_cast<getChatMessageCount &>(obj));
      return true;
    case getChatMessagePosition::ID:
      func(static_cast<getChatMessagePosition &>(obj));
      return true;
    case getChatNotificationSettingsExceptions::ID:
      func(static_cast<getChatNotificationSettingsExceptions &>(obj));
      return true;
    case getChatPinnedMessage::ID:
      func(static_cast<getChatPinnedMessage &>(obj));
      return true;
    case getChatPostedToChatPageStories::ID:
      func(static_cast<getChatPostedToChatPageStories &>(obj));
      return true;
    case getChatRevenueStatistics::ID:
      func(static_cast<getChatRevenueStatistics &>(obj));
      return true;
    case getChatRevenueTransactions::ID:
      func(static_cast<getChatRevenueTransactions &>(obj));
      return true;
    case getChatRevenueWithdrawalUrl::ID:
      func(static_cast<getChatRevenueWithdrawalUrl &>(obj));
      return true;
    case getChatScheduledMessages::ID:
      func(static_cast<getChatScheduledMessages &>(obj));
      return true;
    case getChatSimilarChatCount::ID:
      func(static_cast<getChatSimilarChatCount &>(obj));
      return true;
    case getChatSimilarChats::ID:
      func(static_cast<getChatSimilarChats &>(obj));
      return true;
    case getChatSparseMessagePositions::ID:
      func(static_cast<getChatSparseMessagePositions &>(obj));
      return true;
    case getChatSponsoredMessages::ID:
      func(static_cast<getChatSponsoredMessages &>(obj));
      return true;
    case getChatStatistics::ID:
      func(static_cast<getChatStatistics &>(obj));
      return true;
    case getChatStoryInteractions::ID:
      func(static_cast<getChatStoryInteractions &>(obj));
      return true;
    case getChats::ID:
      func(static_cast<getChats &>(obj));
      return true;
    case getChatsForChatFolderInviteLink::ID:
      func(static_cast<getChatsForChatFolderInviteLink &>(obj));
      return true;
    case getChatsToSendStories::ID:
      func(static_cast<getChatsToSendStories &>(obj));
      return true;
    case getCloseFriends::ID:
      func(static_cast<getCloseFriends &>(obj));
      return true;
    case getCollectibleItemInfo::ID:
      func(static_cast<getCollectibleItemInfo &>(obj));
      return true;
    case getCommands::ID:
      func(static_cast<getCommands &>(obj));
      return true;
    case getConnectedWebsites::ID:
      func(static_cast<getConnectedWebsites &>(obj));
      return true;
    case getContacts::ID:
      func(static_cast<getContacts &>(obj));
      return true;
    case getCountries::ID:
      func(static_cast<getCountries &>(obj));
      return true;
    case getCountryCode::ID:
      func(static_cast<getCountryCode &>(obj));
      return true;
    case getCountryFlagEmoji::ID:
      func(static_cast<getCountryFlagEmoji &>(obj));
      return true;
    case getCreatedPublicChats::ID:
      func(static_cast<getCreatedPublicChats &>(obj));
      return true;
    case getCurrentState::ID:
      func(static_cast<getCurrentState &>(obj));
      return true;
    case getCurrentWeather::ID:
      func(static_cast<getCurrentWeather &>(obj));
      return true;
    case getCustomEmojiReactionAnimations::ID:
      func(static_cast<getCustomEmojiReactionAnimations &>(obj));
      return true;
    case getCustomEmojiStickers::ID:
      func(static_cast<getCustomEmojiStickers &>(obj));
      return true;
    case getDatabaseStatistics::ID:
      func(static_cast<getDatabaseStatistics &>(obj));
      return true;
    case getDeepLinkInfo::ID:
      func(static_cast<getDeepLinkInfo &>(obj));
      return true;
    case getDefaultBackgroundCustomEmojiStickers::ID:
      func(static_cast<getDefaultBackgroundCustomEmojiStickers &>(obj));
      return true;
    case getDefaultChatEmojiStatuses::ID:
      func(static_cast<getDefaultChatEmojiStatuses &>(obj));
      return true;
    case getDefaultChatPhotoCustomEmojiStickers::ID:
      func(static_cast<getDefaultChatPhotoCustomEmojiStickers &>(obj));
      return true;
    case getDefaultEmojiStatuses::ID:
      func(static_cast<getDefaultEmojiStatuses &>(obj));
      return true;
    case getDefaultMessageAutoDeleteTime::ID:
      func(static_cast<getDefaultMessageAutoDeleteTime &>(obj));
      return true;
    case getDefaultProfilePhotoCustomEmojiStickers::ID:
      func(static_cast<getDefaultProfilePhotoCustomEmojiStickers &>(obj));
      return true;
    case getDisallowedChatEmojiStatuses::ID:
      func(static_cast<getDisallowedChatEmojiStatuses &>(obj));
      return true;
    case getEmojiCategories::ID:
      func(static_cast<getEmojiCategories &>(obj));
      return true;
    case getEmojiReaction::ID:
      func(static_cast<getEmojiReaction &>(obj));
      return true;
    case getEmojiSuggestionsUrl::ID:
      func(static_cast<getEmojiSuggestionsUrl &>(obj));
      return true;
    case getExternalLink::ID:
      func(static_cast<getExternalLink &>(obj));
      return true;
    case getExternalLinkInfo::ID:
      func(static_cast<getExternalLinkInfo &>(obj));
      return true;
    case getFavoriteStickers::ID:
      func(static_cast<getFavoriteStickers &>(obj));
      return true;
    case getFile::ID:
      func(static_cast<getFile &>(obj));
      return true;
    case getFileDownloadedPrefixSize::ID:
      func(static_cast<getFileDownloadedPrefixSize &>(obj));
      return true;
    case getFileExtension::ID:
      func(static_cast<getFileExtension &>(obj));
      return true;
    case getFileMimeType::ID:
      func(static_cast<getFileMimeType &>(obj));
      return true;
    case getForumTopic::ID:
      func(static_cast<getForumTopic &>(obj));
      return true;
    case getForumTopicDefaultIcons::ID:
      func(static_cast<getForumTopicDefaultIcons &>(obj));
      return true;
    case getForumTopicLink::ID:
      func(static_cast<getForumTopicLink &>(obj));
      return true;
    case getForumTopics::ID:
      func(static_cast<getForumTopics &>(obj));
      return true;
    case getGameHighScores::ID:
      func(static_cast<getGameHighScores &>(obj));
      return true;
    case getGiveawayInfo::ID:
      func(static_cast<getGiveawayInfo &>(obj));
      return true;
    case getGreetingStickers::ID:
      func(static_cast<getGreetingStickers &>(obj));
      return true;
    case getGrossingWebAppBots::ID:
      func(static_cast<getGrossingWebAppBots &>(obj));
      return true;
    case getGroupCall::ID:
      func(static_cast<getGroupCall &>(obj));
      return true;
    case getGroupCallInviteLink::ID:
      func(static_cast<getGroupCallInviteLink &>(obj));
      return true;
    case getGroupCallStreamSegment::ID:
      func(static_cast<getGroupCallStreamSegment &>(obj));
      return true;
    case getGroupCallStreams::ID:
      func(static_cast<getGroupCallStreams &>(obj));
      return true;
    case getGroupsInCommon::ID:
      func(static_cast<getGroupsInCommon &>(obj));
      return true;
    case getImportedContactCount::ID:
      func(static_cast<getImportedContactCount &>(obj));
      return true;
    case getInactiveSupergroupChats::ID:
      func(static_cast<getInactiveSupergroupChats &>(obj));
      return true;
    case getInlineGameHighScores::ID:
      func(static_cast<getInlineGameHighScores &>(obj));
      return true;
    case getInlineQueryResults::ID:
      func(static_cast<getInlineQueryResults &>(obj));
      return true;
    case getInstalledBackgrounds::ID:
      func(static_cast<getInstalledBackgrounds &>(obj));
      return true;
    case getInstalledStickerSets::ID:
      func(static_cast<getInstalledStickerSets &>(obj));
      return true;
    case getInternalLink::ID:
      func(static_cast<getInternalLink &>(obj));
      return true;
    case getInternalLinkType::ID:
      func(static_cast<getInternalLinkType &>(obj));
      return true;
    case getJsonString::ID:
      func(static_cast<getJsonString &>(obj));
      return true;
    case getJsonValue::ID:
      func(static_cast<getJsonValue &>(obj));
      return true;
    case getKeywordEmojis::ID:
      func(static_cast<getKeywordEmojis &>(obj));
      return true;
    case getLanguagePackInfo::ID:
      func(static_cast<getLanguagePackInfo &>(obj));
      return true;
    case getLanguagePackString::ID:
      func(static_cast<getLanguagePackString &>(obj));
      return true;
    case getLanguagePackStrings::ID:
      func(static_cast<getLanguagePackStrings &>(obj));
      return true;
    case getLinkPreview::ID:
      func(static_cast<getLinkPreview &>(obj));
      return true;
    case getLocalizationTargetInfo::ID:
      func(static_cast<getLocalizationTargetInfo &>(obj));
      return true;
    case getLogStream::ID:
      func(static_cast<getLogStream &>(obj));
      return true;
    case getLogTagVerbosityLevel::ID:
      func(static_cast<getLogTagVerbosityLevel &>(obj));
      return true;
    case getLogTags::ID:
      func(static_cast<getLogTags &>(obj));
      return true;
    case getLogVerbosityLevel::ID:
      func(static_cast<getLogVerbosityLevel &>(obj));
      return true;
    case getLoginUrl::ID:
      func(static_cast<getLoginUrl &>(obj));
      return true;
    case getLoginUrlInfo::ID:
      func(static_cast<getLoginUrlInfo &>(obj));
      return true;
    case getMainWebApp::ID:
      func(static_cast<getMainWebApp &>(obj));
      return true;
    case getMapThumbnailFile::ID:
      func(static_cast<getMapThumbnailFile &>(obj));
      return true;
    case getMarkdownText::ID:
      func(static_cast<getMarkdownText &>(obj));
      return true;
    case getMe::ID:
      func(static_cast<getMe &>(obj));
      return true;
    case getMenuButton::ID:
      func(static_cast<getMenuButton &>(obj));
      return true;
    case getMessage::ID:
      func(static_cast<getMessage &>(obj));
      return true;
    case getMessageAddedReactions::ID:
      func(static_cast<getMessageAddedReactions &>(obj));
      return true;
    case getMessageAvailableReactions::ID:
      func(static_cast<getMessageAvailableReactions &>(obj));
      return true;
    case getMessageEffect::ID:
      func(static_cast<getMessageEffect &>(obj));
      return true;
    case getMessageEmbeddingCode::ID:
      func(static_cast<getMessageEmbeddingCode &>(obj));
      return true;
    case getMessageFileType::ID:
      func(static_cast<getMessageFileType &>(obj));
      return true;
    case getMessageImportConfirmationText::ID:
      func(static_cast<getMessageImportConfirmationText &>(obj));
      return true;
    case getMessageLink::ID:
      func(static_cast<getMessageLink &>(obj));
      return true;
    case getMessageLinkInfo::ID:
      func(static_cast<getMessageLinkInfo &>(obj));
      return true;
    case getMessageLocally::ID:
      func(static_cast<getMessageLocally &>(obj));
      return true;
    case getMessageProperties::ID:
      func(static_cast<getMessageProperties &>(obj));
      return true;
    case getMessagePublicForwards::ID:
      func(static_cast<getMessagePublicForwards &>(obj));
      return true;
    case getMessageReadDate::ID:
      func(static_cast<getMessageReadDate &>(obj));
      return true;
    case getMessageStatistics::ID:
      func(static_cast<getMessageStatistics &>(obj));
      return true;
    case getMessageThread::ID:
      func(static_cast<getMessageThread &>(obj));
      return true;
    case getMessageThreadHistory::ID:
      func(static_cast<getMessageThreadHistory &>(obj));
      return true;
    case getMessageViewers::ID:
      func(static_cast<getMessageViewers &>(obj));
      return true;
    case getMessages::ID:
      func(static_cast<getMessages &>(obj));
      return true;
    case getNetworkStatistics::ID:
      func(static_cast<getNetworkStatistics &>(obj));
      return true;
    case getNewChatPrivacySettings::ID:
      func(static_cast<getNewChatPrivacySettings &>(obj));
      return true;
    case getOption::ID:
      func(static_cast<getOption &>(obj));
      return true;
    case getOwnedStickerSets::ID:
      func(static_cast<getOwnedStickerSets &>(obj));
      return true;
    case getPassportAuthorizationForm::ID:
      func(static_cast<getPassportAuthorizationForm &>(obj));
      return true;
    case getPassportAuthorizationFormAvailableElements::ID:
      func(static_cast<getPassportAuthorizationFormAvailableElements &>(obj));
      return true;
    case getPassportElement::ID:
      func(static_cast<getPassportElement &>(obj));
      return true;
    case getPasswordState::ID:
      func(static_cast<getPasswordState &>(obj));
      return true;
    case getPaymentForm::ID:
      func(static_cast<getPaymentForm &>(obj));
      return true;
    case getPaymentReceipt::ID:
      func(static_cast<getPaymentReceipt &>(obj));
      return true;
    case getPhoneNumberInfo::ID:
      func(static_cast<getPhoneNumberInfo &>(obj));
      return true;
    case getPhoneNumberInfoSync::ID:
      func(static_cast<getPhoneNumberInfoSync &>(obj));
      return true;
    case getPollVoters::ID:
      func(static_cast<getPollVoters &>(obj));
      return true;
    case getPreferredCountryLanguage::ID:
      func(static_cast<getPreferredCountryLanguage &>(obj));
      return true;
    case getPremiumFeatures::ID:
      func(static_cast<getPremiumFeatures &>(obj));
      return true;
    case getPremiumGiftCodePaymentOptions::ID:
      func(static_cast<getPremiumGiftCodePaymentOptions &>(obj));
      return true;
    case getPremiumLimit::ID:
      func(static_cast<getPremiumLimit &>(obj));
      return true;
    case getPremiumState::ID:
      func(static_cast<getPremiumState &>(obj));
      return true;
    case getPremiumStickerExamples::ID:
      func(static_cast<getPremiumStickerExamples &>(obj));
      return true;
    case getPremiumStickers::ID:
      func(static_cast<getPremiumStickers &>(obj));
      return true;
    case getProxies::ID:
      func(static_cast<getProxies &>(obj));
      return true;
    case getProxyLink::ID:
      func(static_cast<getProxyLink &>(obj));
      return true;
    case getPushReceiverId::ID:
      func(static_cast<getPushReceiverId &>(obj));
      return true;
    case getReadDatePrivacySettings::ID:
      func(static_cast<getReadDatePrivacySettings &>(obj));
      return true;
    case getRecentEmojiStatuses::ID:
      func(static_cast<getRecentEmojiStatuses &>(obj));
      return true;
    case getRecentInlineBots::ID:
      func(static_cast<getRecentInlineBots &>(obj));
      return true;
    case getRecentStickers::ID:
      func(static_cast<getRecentStickers &>(obj));
      return true;
    case getRecentlyOpenedChats::ID:
      func(static_cast<getRecentlyOpenedChats &>(obj));
      return true;
    case getRecentlyVisitedTMeUrls::ID:
      func(static_cast<getRecentlyVisitedTMeUrls &>(obj));
      return true;
    case getRecommendedChatFolders::ID:
      func(static_cast<getRecommendedChatFolders &>(obj));
      return true;
    case getRecommendedChats::ID:
      func(static_cast<getRecommendedChats &>(obj));
      return true;
    case getRecoveryEmailAddress::ID:
      func(static_cast<getRecoveryEmailAddress &>(obj));
      return true;
    case getRemoteFile::ID:
      func(static_cast<getRemoteFile &>(obj));
      return true;
    case getRepliedMessage::ID:
      func(static_cast<getRepliedMessage &>(obj));
      return true;
    case getSavedAnimations::ID:
      func(static_cast<getSavedAnimations &>(obj));
      return true;
    case getSavedMessagesTags::ID:
      func(static_cast<getSavedMessagesTags &>(obj));
      return true;
    case getSavedMessagesTopicHistory::ID:
      func(static_cast<getSavedMessagesTopicHistory &>(obj));
      return true;
    case getSavedMessagesTopicMessageByDate::ID:
      func(static_cast<getSavedMessagesTopicMessageByDate &>(obj));
      return true;
    case getSavedNotificationSound::ID:
      func(static_cast<getSavedNotificationSound &>(obj));
      return true;
    case getSavedNotificationSounds::ID:
      func(static_cast<getSavedNotificationSounds &>(obj));
      return true;
    case getSavedOrderInfo::ID:
      func(static_cast<getSavedOrderInfo &>(obj));
      return true;
    case getScopeNotificationSettings::ID:
      func(static_cast<getScopeNotificationSettings &>(obj));
      return true;
    case getSearchedForTags::ID:
      func(static_cast<getSearchedForTags &>(obj));
      return true;
    case getSecretChat::ID:
      func(static_cast<getSecretChat &>(obj));
      return true;
    case getStarAdAccountUrl::ID:
      func(static_cast<getStarAdAccountUrl &>(obj));
      return true;
    case getStarGiftPaymentOptions::ID:
      func(static_cast<getStarGiftPaymentOptions &>(obj));
      return true;
    case getStarGiveawayPaymentOptions::ID:
      func(static_cast<getStarGiveawayPaymentOptions &>(obj));
      return true;
    case getStarPaymentOptions::ID:
      func(static_cast<getStarPaymentOptions &>(obj));
      return true;
    case getStarRevenueStatistics::ID:
      func(static_cast<getStarRevenueStatistics &>(obj));
      return true;
    case getStarSubscriptions::ID:
      func(static_cast<getStarSubscriptions &>(obj));
      return true;
    case getStarTransactions::ID:
      func(static_cast<getStarTransactions &>(obj));
      return true;
    case getStarWithdrawalUrl::ID:
      func(static_cast<getStarWithdrawalUrl &>(obj));
      return true;
    case getStatisticalGraph::ID:
      func(static_cast<getStatisticalGraph &>(obj));
      return true;
    case getStickerEmojis::ID:
      func(static_cast<getStickerEmojis &>(obj));
      return true;
    case getStickerSet::ID:
      func(static_cast<getStickerSet &>(obj));
      return true;
    case getStickerSetName::ID:
      func(static_cast<getStickerSetName &>(obj));
      return true;
    case getStickers::ID:
      func(static_cast<getStickers &>(obj));
      return true;
    case getStorageStatistics::ID:
      func(static_cast<getStorageStatistics &>(obj));
      return true;
    case getStorageStatisticsFast::ID:
      func(static_cast<getStorageStatisticsFast &>(obj));
      return true;
    case getStory::ID:
      func(static_cast<getStory &>(obj));
      return true;
    case getStoryAvailableReactions::ID:
      func(static_cast<getStoryAvailableReactions &>(obj));
      return true;
    case getStoryInteractions::ID:
      func(static_cast<getStoryInteractions &>(obj));
      return true;
    case getStoryNotificationSettingsExceptions::ID:
      func(static_cast<getStoryNotificationSettingsExceptions &>(obj));
      return true;
    case getStoryPublicForwards::ID:
      func(static_cast<getStoryPublicForwards &>(obj));
      return true;
    case getStoryStatistics::ID:
      func(static_cast<getStoryStatistics &>(obj));
      return true;
    case getSuggestedFileName::ID:
      func(static_cast<getSuggestedFileName &>(obj));
      return true;
    case getSuggestedStickerSetName::ID:
      func(static_cast<getSuggestedStickerSetName &>(obj));
      return true;
    case getSuitableDiscussionChats::ID:
      func(static_cast<getSuitableDiscussionChats &>(obj));
      return true;
    case getSuitablePersonalChats::ID:
      func(static_cast<getSuitablePersonalChats &>(obj));
      return true;
    case getSupergroup::ID:
      func(static_cast<getSupergroup &>(obj));
      return true;
    case getSupergroupFullInfo::ID:
      func(static_cast<getSupergroupFullInfo &>(obj));
      return true;
    case getSupergroupMembers::ID:
      func(static_cast<getSupergroupMembers &>(obj));
      return true;
    case getSupportName::ID:
      func(static_cast<getSupportName &>(obj));
      return true;
    case getSupportUser::ID:
      func(static_cast<getSupportUser &>(obj));
      return true;
    case getTemporaryPasswordState::ID:
      func(static_cast<getTemporaryPasswordState &>(obj));
      return true;
    case getTextEntities::ID:
      func(static_cast<getTextEntities &>(obj));
      return true;
    case getThemeParametersJsonString::ID:
      func(static_cast<getThemeParametersJsonString &>(obj));
      return true;
    case getThemedChatEmojiStatuses::ID:
      func(static_cast<getThemedChatEmojiStatuses &>(obj));
      return true;
    case getThemedEmojiStatuses::ID:
      func(static_cast<getThemedEmojiStatuses &>(obj));
      return true;
    case getTimeZones::ID:
      func(static_cast<getTimeZones &>(obj));
      return true;
    case getTopChats::ID:
      func(static_cast<getTopChats &>(obj));
      return true;
    case getTrendingStickerSets::ID:
      func(static_cast<getTrendingStickerSets &>(obj));
      return true;
    case getUser::ID:
      func(static_cast<getUser &>(obj));
      return true;
    case getUserChatBoosts::ID:
      func(static_cast<getUserChatBoosts &>(obj));
      return true;
    case getUserFullInfo::ID:
      func(static_cast<getUserFullInfo &>(obj));
      return true;
    case getUserLink::ID:
      func(static_cast<getUserLink &>(obj));
      return true;
    case getUserPrivacySettingRules::ID:
      func(static_cast<getUserPrivacySettingRules &>(obj));
      return true;
    case getUserProfilePhotos::ID:
      func(static_cast<getUserProfilePhotos &>(obj));
      return true;
    case getUserSupportInfo::ID:
      func(static_cast<getUserSupportInfo &>(obj));
      return true;
    case getVideoChatAvailableParticipants::ID:
      func(static_cast<getVideoChatAvailableParticipants &>(obj));
      return true;
    case getVideoChatRtmpUrl::ID:
      func(static_cast<getVideoChatRtmpUrl &>(obj));
      return true;
    case getWebAppLinkUrl::ID:
      func(static_cast<getWebAppLinkUrl &>(obj));
      return true;
    case getWebAppUrl::ID:
      func(static_cast<getWebAppUrl &>(obj));
      return true;
    case getWebPageInstantView::ID:
      func(static_cast<getWebPageInstantView &>(obj));
      return true;
    case hideContactCloseBirthdays::ID:
      func(static_cast<hideContactCloseBirthdays &>(obj));
      return true;
    case hideSuggestedAction::ID:
      func(static_cast<hideSuggestedAction &>(obj));
      return true;
    case importContacts::ID:
      func(static_cast<importContacts &>(obj));
      return true;
    case importMessages::ID:
      func(static_cast<importMessages &>(obj));
      return true;
    case inviteGroupCallParticipants::ID:
      func(static_cast<inviteGroupCallParticipants &>(obj));
      return true;
    case joinChat::ID:
      func(static_cast<joinChat &>(obj));
      return true;
    case joinChatByInviteLink::ID:
      func(static_cast<joinChatByInviteLink &>(obj));
      return true;
    case joinGroupCall::ID:
      func(static_cast<joinGroupCall &>(obj));
      return true;
    case launchPrepaidGiveaway::ID:
      func(static_cast<launchPrepaidGiveaway &>(obj));
      return true;
    case leaveChat::ID:
      func(static_cast<leaveChat &>(obj));
      return true;
    case leaveGroupCall::ID:
      func(static_cast<leaveGroupCall &>(obj));
      return true;
    case loadActiveStories::ID:
      func(static_cast<loadActiveStories &>(obj));
      return true;
    case loadChats::ID:
      func(static_cast<loadChats &>(obj));
      return true;
    case loadGroupCallParticipants::ID:
      func(static_cast<loadGroupCallParticipants &>(obj));
      return true;
    case loadQuickReplyShortcutMessages::ID:
      func(static_cast<loadQuickReplyShortcutMessages &>(obj));
      return true;
    case loadQuickReplyShortcuts::ID:
      func(static_cast<loadQuickReplyShortcuts &>(obj));
      return true;
    case loadSavedMessagesTopics::ID:
      func(static_cast<loadSavedMessagesTopics &>(obj));
      return true;
    case logOut::ID:
      func(static_cast<logOut &>(obj));
      return true;
    case openChat::ID:
      func(static_cast<openChat &>(obj));
      return true;
    case openChatSimilarChat::ID:
      func(static_cast<openChatSimilarChat &>(obj));
      return true;
    case openMessageContent::ID:
      func(static_cast<openMessageContent &>(obj));
      return true;
    case openStory::ID:
      func(static_cast<openStory &>(obj));
      return true;
    case openWebApp::ID:
      func(static_cast<openWebApp &>(obj));
      return true;
    case optimizeStorage::ID:
      func(static_cast<optimizeStorage &>(obj));
      return true;
    case parseMarkdown::ID:
      func(static_cast<parseMarkdown &>(obj));
      return true;
    case parseTextEntities::ID:
      func(static_cast<parseTextEntities &>(obj));
      return true;
    case pinChatMessage::ID:
      func(static_cast<pinChatMessage &>(obj));
      return true;
    case pingProxy::ID:
      func(static_cast<pingProxy &>(obj));
      return true;
    case preliminaryUploadFile::ID:
      func(static_cast<preliminaryUploadFile &>(obj));
      return true;
    case processChatFolderNewChats::ID:
      func(static_cast<processChatFolderNewChats &>(obj));
      return true;
    case processChatJoinRequest::ID:
      func(static_cast<processChatJoinRequest &>(obj));
      return true;
    case processChatJoinRequests::ID:
      func(static_cast<processChatJoinRequests &>(obj));
      return true;
    case processPushNotification::ID:
      func(static_cast<processPushNotification &>(obj));
      return true;
    case rateSpeechRecognition::ID:
      func(static_cast<rateSpeechRecognition &>(obj));
      return true;
    case readAllChatMentions::ID:
      func(static_cast<readAllChatMentions &>(obj));
      return true;
    case readAllChatReactions::ID:
      func(static_cast<readAllChatReactions &>(obj));
      return true;
    case readAllMessageThreadMentions::ID:
      func(static_cast<readAllMessageThreadMentions &>(obj));
      return true;
    case readAllMessageThreadReactions::ID:
      func(static_cast<readAllMessageThreadReactions &>(obj));
      return true;
    case readChatList::ID:
      func(static_cast<readChatList &>(obj));
      return true;
    case readFilePart::ID:
      func(static_cast<readFilePart &>(obj));
      return true;
    case readdQuickReplyShortcutMessages::ID:
      func(static_cast<readdQuickReplyShortcutMessages &>(obj));
      return true;
    case recognizeSpeech::ID:
      func(static_cast<recognizeSpeech &>(obj));
      return true;
    case recoverAuthenticationPassword::ID:
      func(static_cast<recoverAuthenticationPassword &>(obj));
      return true;
    case recoverPassword::ID:
      func(static_cast<recoverPassword &>(obj));
      return true;
    case refundStarPayment::ID:
      func(static_cast<refundStarPayment &>(obj));
      return true;
    case registerDevice::ID:
      func(static_cast<registerDevice &>(obj));
      return true;
    case registerUser::ID:
      func(static_cast<registerUser &>(obj));
      return true;
    case removeAllFilesFromDownloads::ID:
      func(static_cast<removeAllFilesFromDownloads &>(obj));
      return true;
    case removeBusinessConnectedBotFromChat::ID:
      func(static_cast<removeBusinessConnectedBotFromChat &>(obj));
      return true;
    case removeChatActionBar::ID:
      func(static_cast<removeChatActionBar &>(obj));
      return true;
    case removeContacts::ID:
      func(static_cast<removeContacts &>(obj));
      return true;
    case removeFavoriteSticker::ID:
      func(static_cast<removeFavoriteSticker &>(obj));
      return true;
    case removeFileFromDownloads::ID:
      func(static_cast<removeFileFromDownloads &>(obj));
      return true;
    case removeInstalledBackground::ID:
      func(static_cast<removeInstalledBackground &>(obj));
      return true;
    case removeMessageReaction::ID:
      func(static_cast<removeMessageReaction &>(obj));
      return true;
    case removeNotification::ID:
      func(static_cast<removeNotification &>(obj));
      return true;
    case removeNotificationGroup::ID:
      func(static_cast<removeNotificationGroup &>(obj));
      return true;
    case removePendingPaidMessageReactions::ID:
      func(static_cast<removePendingPaidMessageReactions &>(obj));
      return true;
    case removeProxy::ID:
      func(static_cast<removeProxy &>(obj));
      return true;
    case removeRecentHashtag::ID:
      func(static_cast<removeRecentHashtag &>(obj));
      return true;
    case removeRecentSticker::ID:
      func(static_cast<removeRecentSticker &>(obj));
      return true;
    case removeRecentlyFoundChat::ID:
      func(static_cast<removeRecentlyFoundChat &>(obj));
      return true;
    case removeSavedAnimation::ID:
      func(static_cast<removeSavedAnimation &>(obj));
      return true;
    case removeSavedNotificationSound::ID:
      func(static_cast<removeSavedNotificationSound &>(obj));
      return true;
    case removeSearchedForTag::ID:
      func(static_cast<removeSearchedForTag &>(obj));
      return true;
    case removeStickerFromSet::ID:
      func(static_cast<removeStickerFromSet &>(obj));
      return true;
    case removeTopChat::ID:
      func(static_cast<removeTopChat &>(obj));
      return true;
    case reorderActiveUsernames::ID:
      func(static_cast<reorderActiveUsernames &>(obj));
      return true;
    case reorderBotActiveUsernames::ID:
      func(static_cast<reorderBotActiveUsernames &>(obj));
      return true;
    case reorderBotMediaPreviews::ID:
      func(static_cast<reorderBotMediaPreviews &>(obj));
      return true;
    case reorderChatFolders::ID:
      func(static_cast<reorderChatFolders &>(obj));
      return true;
    case reorderInstalledStickerSets::ID:
      func(static_cast<reorderInstalledStickerSets &>(obj));
      return true;
    case reorderQuickReplyShortcuts::ID:
      func(static_cast<reorderQuickReplyShortcuts &>(obj));
      return true;
    case reorderSupergroupActiveUsernames::ID:
      func(static_cast<reorderSupergroupActiveUsernames &>(obj));
      return true;
    case replacePrimaryChatInviteLink::ID:
      func(static_cast<replacePrimaryChatInviteLink &>(obj));
      return true;
    case replaceStickerInSet::ID:
      func(static_cast<replaceStickerInSet &>(obj));
      return true;
    case replaceVideoChatRtmpUrl::ID:
      func(static_cast<replaceVideoChatRtmpUrl &>(obj));
      return true;
    case reportAuthenticationCodeMissing::ID:
      func(static_cast<reportAuthenticationCodeMissing &>(obj));
      return true;
    case reportChat::ID:
      func(static_cast<reportChat &>(obj));
      return true;
    case reportChatPhoto::ID:
      func(static_cast<reportChatPhoto &>(obj));
      return true;
    case reportChatSponsoredMessage::ID:
      func(static_cast<reportChatSponsoredMessage &>(obj));
      return true;
    case reportMessageReactions::ID:
      func(static_cast<reportMessageReactions &>(obj));
      return true;
    case reportPhoneNumberCodeMissing::ID:
      func(static_cast<reportPhoneNumberCodeMissing &>(obj));
      return true;
    case reportStory::ID:
      func(static_cast<reportStory &>(obj));
      return true;
    case reportSupergroupAntiSpamFalsePositive::ID:
      func(static_cast<reportSupergroupAntiSpamFalsePositive &>(obj));
      return true;
    case reportSupergroupSpam::ID:
      func(static_cast<reportSupergroupSpam &>(obj));
      return true;
    case requestAuthenticationPasswordRecovery::ID:
      func(static_cast<requestAuthenticationPasswordRecovery &>(obj));
      return true;
    case requestPasswordRecovery::ID:
      func(static_cast<requestPasswordRecovery &>(obj));
      return true;
    case requestQrCodeAuthentication::ID:
      func(static_cast<requestQrCodeAuthentication &>(obj));
      return true;
    case resendAuthenticationCode::ID:
      func(static_cast<resendAuthenticationCode &>(obj));
      return true;
    case resendEmailAddressVerificationCode::ID:
      func(static_cast<resendEmailAddressVerificationCode &>(obj));
      return true;
    case resendLoginEmailAddressCode::ID:
      func(static_cast<resendLoginEmailAddressCode &>(obj));
      return true;
    case resendMessages::ID:
      func(static_cast<resendMessages &>(obj));
      return true;
    case resendPhoneNumberCode::ID:
      func(static_cast<resendPhoneNumberCode &>(obj));
      return true;
    case resendRecoveryEmailAddressCode::ID:
      func(static_cast<resendRecoveryEmailAddressCode &>(obj));
      return true;
    case resetAllNotificationSettings::ID:
      func(static_cast<resetAllNotificationSettings &>(obj));
      return true;
    case resetAuthenticationEmailAddress::ID:
      func(static_cast<resetAuthenticationEmailAddress &>(obj));
      return true;
    case resetInstalledBackgrounds::ID:
      func(static_cast<resetInstalledBackgrounds &>(obj));
      return true;
    case resetNetworkStatistics::ID:
      func(static_cast<resetNetworkStatistics &>(obj));
      return true;
    case resetPassword::ID:
      func(static_cast<resetPassword &>(obj));
      return true;
    case reuseStarSubscription::ID:
      func(static_cast<reuseStarSubscription &>(obj));
      return true;
    case revokeChatInviteLink::ID:
      func(static_cast<revokeChatInviteLink &>(obj));
      return true;
    case revokeGroupCallInviteLink::ID:
      func(static_cast<revokeGroupCallInviteLink &>(obj));
      return true;
    case saveApplicationLogEvent::ID:
      func(static_cast<saveApplicationLogEvent &>(obj));
      return true;
    case searchBackground::ID:
      func(static_cast<searchBackground &>(obj));
      return true;
    case searchCallMessages::ID:
      func(static_cast<searchCallMessages &>(obj));
      return true;
    case searchChatMembers::ID:
      func(static_cast<searchChatMembers &>(obj));
      return true;
    case searchChatMessages::ID:
      func(static_cast<searchChatMessages &>(obj));
      return true;
    case searchChatRecentLocationMessages::ID:
      func(static_cast<searchChatRecentLocationMessages &>(obj));
      return true;
    case searchChats::ID:
      func(static_cast<searchChats &>(obj));
      return true;
    case searchChatsNearby::ID:
      func(static_cast<searchChatsNearby &>(obj));
      return true;
    case searchChatsOnServer::ID:
      func(static_cast<searchChatsOnServer &>(obj));
      return true;
    case searchContacts::ID:
      func(static_cast<searchContacts &>(obj));
      return true;
    case searchEmojis::ID:
      func(static_cast<searchEmojis &>(obj));
      return true;
    case searchFileDownloads::ID:
      func(static_cast<searchFileDownloads &>(obj));
      return true;
    case searchHashtags::ID:
      func(static_cast<searchHashtags &>(obj));
      return true;
    case searchInstalledStickerSets::ID:
      func(static_cast<searchInstalledStickerSets &>(obj));
      return true;
    case searchMessages::ID:
      func(static_cast<searchMessages &>(obj));
      return true;
    case searchOutgoingDocumentMessages::ID:
      func(static_cast<searchOutgoingDocumentMessages &>(obj));
      return true;
    case searchPublicChat::ID:
      func(static_cast<searchPublicChat &>(obj));
      return true;
    case searchPublicChats::ID:
      func(static_cast<searchPublicChats &>(obj));
      return true;
    case searchPublicMessagesByTag::ID:
      func(static_cast<searchPublicMessagesByTag &>(obj));
      return true;
    case searchPublicStoriesByLocation::ID:
      func(static_cast<searchPublicStoriesByLocation &>(obj));
      return true;
    case searchPublicStoriesByTag::ID:
      func(static_cast<searchPublicStoriesByTag &>(obj));
      return true;
    case searchPublicStoriesByVenue::ID:
      func(static_cast<searchPublicStoriesByVenue &>(obj));
      return true;
    case searchQuote::ID:
      func(static_cast<searchQuote &>(obj));
      return true;
    case searchRecentlyFoundChats::ID:
      func(static_cast<searchRecentlyFoundChats &>(obj));
      return true;
    case searchSavedMessages::ID:
      func(static_cast<searchSavedMessages &>(obj));
      return true;
    case searchSecretMessages::ID:
      func(static_cast<searchSecretMessages &>(obj));
      return true;
    case searchStickerSet::ID:
      func(static_cast<searchStickerSet &>(obj));
      return true;
    case searchStickerSets::ID:
      func(static_cast<searchStickerSets &>(obj));
      return true;
    case searchStickers::ID:
      func(static_cast<searchStickers &>(obj));
      return true;
    case searchStringsByPrefix::ID:
      func(static_cast<searchStringsByPrefix &>(obj));
      return true;
    case searchUserByPhoneNumber::ID:
      func(static_cast<searchUserByPhoneNumber &>(obj));
      return true;
    case searchUserByToken::ID:
      func(static_cast<searchUserByToken &>(obj));
      return true;
    case searchWebApp::ID:
      func(static_cast<searchWebApp &>(obj));
      return true;
    case sendAuthenticationFirebaseSms::ID:
      func(static_cast<sendAuthenticationFirebaseSms &>(obj));
      return true;
    case sendBotStartMessage::ID:
      func(static_cast<sendBotStartMessage &>(obj));
      return true;
    case sendBusinessMessage::ID:
      func(static_cast<sendBusinessMessage &>(obj));
      return true;
    case sendBusinessMessageAlbum::ID:
      func(static_cast<sendBusinessMessageAlbum &>(obj));
      return true;
    case sendCallDebugInformation::ID:
      func(static_cast<sendCallDebugInformation &>(obj));
      return true;
    case sendCallLog::ID:
      func(static_cast<sendCallLog &>(obj));
      return true;
    case sendCallRating::ID:
      func(static_cast<sendCallRating &>(obj));
      return true;
    case sendCallSignalingData::ID:
      func(static_cast<sendCallSignalingData &>(obj));
      return true;
    case sendChatAction::ID:
      func(static_cast<sendChatAction &>(obj));
      return true;
    case sendCustomRequest::ID:
      func(static_cast<sendCustomRequest &>(obj));
      return true;
    case sendEmailAddressVerificationCode::ID:
      func(static_cast<sendEmailAddressVerificationCode &>(obj));
      return true;
    case sendInlineQueryResultMessage::ID:
      func(static_cast<sendInlineQueryResultMessage &>(obj));
      return true;
    case sendMessage::ID:
      func(static_cast<sendMessage &>(obj));
      return true;
    case sendMessageAlbum::ID:
      func(static_cast<sendMessageAlbum &>(obj));
      return true;
    case sendPassportAuthorizationForm::ID:
      func(static_cast<sendPassportAuthorizationForm &>(obj));
      return true;
    case sendPaymentForm::ID:
      func(static_cast<sendPaymentForm &>(obj));
      return true;
    case sendPhoneNumberCode::ID:
      func(static_cast<sendPhoneNumberCode &>(obj));
      return true;
    case sendPhoneNumberFirebaseSms::ID:
      func(static_cast<sendPhoneNumberFirebaseSms &>(obj));
      return true;
    case sendQuickReplyShortcutMessages::ID:
      func(static_cast<sendQuickReplyShortcutMessages &>(obj));
      return true;
    case sendStory::ID:
      func(static_cast<sendStory &>(obj));
      return true;
    case sendWebAppCustomRequest::ID:
      func(static_cast<sendWebAppCustomRequest &>(obj));
      return true;
    case sendWebAppData::ID:
      func(static_cast<sendWebAppData &>(obj));
      return true;
    case setAccentColor::ID:
      func(static_cast<setAccentColor &>(obj));
      return true;
    case setAccountTtl::ID:
      func(static_cast<setAccountTtl &>(obj));
      return true;
    case setAlarm::ID:
      func(static_cast<setAlarm &>(obj));
      return true;
    case setApplicationVerificationToken::ID:
      func(static_cast<setApplicationVerificationToken &>(obj));
      return true;
    case setArchiveChatListSettings::ID:
      func(static_cast<setArchiveChatListSettings &>(obj));
      return true;
    case setAuthenticationEmailAddress::ID:
      func(static_cast<setAuthenticationEmailAddress &>(obj));
      return true;
    case setAuthenticationPhoneNumber::ID:
      func(static_cast<setAuthenticationPhoneNumber &>(obj));
      return true;
    case setAutoDownloadSettings::ID:
      func(static_cast<setAutoDownloadSettings &>(obj));
      return true;
    case setAutosaveSettings::ID:
      func(static_cast<setAutosaveSettings &>(obj));
      return true;
    case setBio::ID:
      func(static_cast<setBio &>(obj));
      return true;
    case setBirthdate::ID:
      func(static_cast<setBirthdate &>(obj));
      return true;
    case setBotInfoDescription::ID:
      func(static_cast<setBotInfoDescription &>(obj));
      return true;
    case setBotInfoShortDescription::ID:
      func(static_cast<setBotInfoShortDescription &>(obj));
      return true;
    case setBotName::ID:
      func(static_cast<setBotName &>(obj));
      return true;
    case setBotProfilePhoto::ID:
      func(static_cast<setBotProfilePhoto &>(obj));
      return true;
    case setBotUpdatesStatus::ID:
      func(static_cast<setBotUpdatesStatus &>(obj));
      return true;
    case setBusinessAwayMessageSettings::ID:
      func(static_cast<setBusinessAwayMessageSettings &>(obj));
      return true;
    case setBusinessConnectedBot::ID:
      func(static_cast<setBusinessConnectedBot &>(obj));
      return true;
    case setBusinessGreetingMessageSettings::ID:
      func(static_cast<setBusinessGreetingMessageSettings &>(obj));
      return true;
    case setBusinessLocation::ID:
      func(static_cast<setBusinessLocation &>(obj));
      return true;
    case setBusinessMessageIsPinned::ID:
      func(static_cast<setBusinessMessageIsPinned &>(obj));
      return true;
    case setBusinessOpeningHours::ID:
      func(static_cast<setBusinessOpeningHours &>(obj));
      return true;
    case setBusinessStartPage::ID:
      func(static_cast<setBusinessStartPage &>(obj));
      return true;
    case setChatAccentColor::ID:
      func(static_cast<setChatAccentColor &>(obj));
      return true;
    case setChatActiveStoriesList::ID:
      func(static_cast<setChatActiveStoriesList &>(obj));
      return true;
    case setChatAvailableReactions::ID:
      func(static_cast<setChatAvailableReactions &>(obj));
      return true;
    case setChatBackground::ID:
      func(static_cast<setChatBackground &>(obj));
      return true;
    case setChatClientData::ID:
      func(static_cast<setChatClientData &>(obj));
      return true;
    case setChatDescription::ID:
      func(static_cast<setChatDescription &>(obj));
      return true;
    case setChatDiscussionGroup::ID:
      func(static_cast<setChatDiscussionGroup &>(obj));
      return true;
    case setChatDraftMessage::ID:
      func(static_cast<setChatDraftMessage &>(obj));
      return true;
    case setChatEmojiStatus::ID:
      func(static_cast<setChatEmojiStatus &>(obj));
      return true;
    case setChatLocation::ID:
      func(static_cast<setChatLocation &>(obj));
      return true;
    case setChatMemberStatus::ID:
      func(static_cast<setChatMemberStatus &>(obj));
      return true;
    case setChatMessageAutoDeleteTime::ID:
      func(static_cast<setChatMessageAutoDeleteTime &>(obj));
      return true;
    case setChatMessageSender::ID:
      func(static_cast<setChatMessageSender &>(obj));
      return true;
    case setChatNotificationSettings::ID:
      func(static_cast<setChatNotificationSettings &>(obj));
      return true;
    case setChatPermissions::ID:
      func(static_cast<setChatPermissions &>(obj));
      return true;
    case setChatPhoto::ID:
      func(static_cast<setChatPhoto &>(obj));
      return true;
    case setChatPinnedStories::ID:
      func(static_cast<setChatPinnedStories &>(obj));
      return true;
    case setChatProfileAccentColor::ID:
      func(static_cast<setChatProfileAccentColor &>(obj));
      return true;
    case setChatSlowModeDelay::ID:
      func(static_cast<setChatSlowModeDelay &>(obj));
      return true;
    case setChatTheme::ID:
      func(static_cast<setChatTheme &>(obj));
      return true;
    case setChatTitle::ID:
      func(static_cast<setChatTitle &>(obj));
      return true;
    case setCloseFriends::ID:
      func(static_cast<setCloseFriends &>(obj));
      return true;
    case setCommands::ID:
      func(static_cast<setCommands &>(obj));
      return true;
    case setCustomEmojiStickerSetThumbnail::ID:
      func(static_cast<setCustomEmojiStickerSetThumbnail &>(obj));
      return true;
    case setCustomLanguagePack::ID:
      func(static_cast<setCustomLanguagePack &>(obj));
      return true;
    case setCustomLanguagePackString::ID:
      func(static_cast<setCustomLanguagePackString &>(obj));
      return true;
    case setDatabaseEncryptionKey::ID:
      func(static_cast<setDatabaseEncryptionKey &>(obj));
      return true;
    case setDefaultBackground::ID:
      func(static_cast<setDefaultBackground &>(obj));
      return true;
    case setDefaultChannelAdministratorRights::ID:
      func(static_cast<setDefaultChannelAdministratorRights &>(obj));
      return true;
    case setDefaultGroupAdministratorRights::ID:
      func(static_cast<setDefaultGroupAdministratorRights &>(obj));
      return true;
    case setDefaultMessageAutoDeleteTime::ID:
      func(static_cast<setDefaultMessageAutoDeleteTime &>(obj));
      return true;
    case setDefaultReactionType::ID:
      func(static_cast<setDefaultReactionType &>(obj));
      return true;
    case setEmojiStatus::ID:
      func(static_cast<setEmojiStatus &>(obj));
      return true;
    case setFileGenerationProgress::ID:
      func(static_cast<setFileGenerationProgress &>(obj));
      return true;
    case setForumTopicNotificationSettings::ID:
      func(static_cast<setForumTopicNotificationSettings &>(obj));
      return true;
    case setGameScore::ID:
      func(static_cast<setGameScore &>(obj));
      return true;
    case setGroupCallParticipantIsSpeaking::ID:
      func(static_cast<setGroupCallParticipantIsSpeaking &>(obj));
      return true;
    case setGroupCallParticipantVolumeLevel::ID:
      func(static_cast<setGroupCallParticipantVolumeLevel &>(obj));
      return true;
    case setGroupCallTitle::ID:
      func(static_cast<setGroupCallTitle &>(obj));
      return true;
    case setInactiveSessionTtl::ID:
      func(static_cast<setInactiveSessionTtl &>(obj));
      return true;
    case setInlineGameScore::ID:
      func(static_cast<setInlineGameScore &>(obj));
      return true;
    case setLocation::ID:
      func(static_cast<setLocation &>(obj));
      return true;
    case setLogStream::ID:
      func(static_cast<setLogStream &>(obj));
      return true;
    case setLogTagVerbosityLevel::ID:
      func(static_cast<setLogTagVerbosityLevel &>(obj));
      return true;
    case setLogVerbosityLevel::ID:
      func(static_cast<setLogVerbosityLevel &>(obj));
      return true;
    case setLoginEmailAddress::ID:
      func(static_cast<setLoginEmailAddress &>(obj));
      return true;
    case setMenuButton::ID:
      func(static_cast<setMenuButton &>(obj));
      return true;
    case setMessageFactCheck::ID:
      func(static_cast<setMessageFactCheck &>(obj));
      return true;
    case setMessageReactions::ID:
      func(static_cast<setMessageReactions &>(obj));
      return true;
    case setMessageSenderBlockList::ID:
      func(static_cast<setMessageSenderBlockList &>(obj));
      return true;
    case setName::ID:
      func(static_cast<setName &>(obj));
      return true;
    case setNetworkType::ID:
      func(static_cast<setNetworkType &>(obj));
      return true;
    case setNewChatPrivacySettings::ID:
      func(static_cast<setNewChatPrivacySettings &>(obj));
      return true;
    case setOption::ID:
      func(static_cast<setOption &>(obj));
      return true;
    case setPassportElement::ID:
      func(static_cast<setPassportElement &>(obj));
      return true;
    case setPassportElementErrors::ID:
      func(static_cast<setPassportElementErrors &>(obj));
      return true;
    case setPassword::ID:
      func(static_cast<setPassword &>(obj));
      return true;
    case setPersonalChat::ID:
      func(static_cast<setPersonalChat &>(obj));
      return true;
    case setPinnedChats::ID:
      func(static_cast<setPinnedChats &>(obj));
      return true;
    case setPinnedForumTopics::ID:
      func(static_cast<setPinnedForumTopics &>(obj));
      return true;
    case setPinnedSavedMessagesTopics::ID:
      func(static_cast<setPinnedSavedMessagesTopics &>(obj));
      return true;
    case setPollAnswer::ID:
      func(static_cast<setPollAnswer &>(obj));
      return true;
    case setProfileAccentColor::ID:
      func(static_cast<setProfileAccentColor &>(obj));
      return true;
    case setProfilePhoto::ID:
      func(static_cast<setProfilePhoto &>(obj));
      return true;
    case setQuickReplyShortcutName::ID:
      func(static_cast<setQuickReplyShortcutName &>(obj));
      return true;
    case setReactionNotificationSettings::ID:
      func(static_cast<setReactionNotificationSettings &>(obj));
      return true;
    case setReadDatePrivacySettings::ID:
      func(static_cast<setReadDatePrivacySettings &>(obj));
      return true;
    case setRecoveryEmailAddress::ID:
      func(static_cast<setRecoveryEmailAddress &>(obj));
      return true;
    case setSavedMessagesTagLabel::ID:
      func(static_cast<setSavedMessagesTagLabel &>(obj));
      return true;
    case setScopeNotificationSettings::ID:
      func(static_cast<setScopeNotificationSettings &>(obj));
      return true;
    case setStickerEmojis::ID:
      func(static_cast<setStickerEmojis &>(obj));
      return true;
    case setStickerKeywords::ID:
      func(static_cast<setStickerKeywords &>(obj));
      return true;
    case setStickerMaskPosition::ID:
      func(static_cast<setStickerMaskPosition &>(obj));
      return true;
    case setStickerPositionInSet::ID:
      func(static_cast<setStickerPositionInSet &>(obj));
      return true;
    case setStickerSetThumbnail::ID:
      func(static_cast<setStickerSetThumbnail &>(obj));
      return true;
    case setStickerSetTitle::ID:
      func(static_cast<setStickerSetTitle &>(obj));
      return true;
    case setStoryPrivacySettings::ID:
      func(static_cast<setStoryPrivacySettings &>(obj));
      return true;
    case setStoryReaction::ID:
      func(static_cast<setStoryReaction &>(obj));
      return true;
    case setSupergroupCustomEmojiStickerSet::ID:
      func(static_cast<setSupergroupCustomEmojiStickerSet &>(obj));
      return true;
    case setSupergroupStickerSet::ID:
      func(static_cast<setSupergroupStickerSet &>(obj));
      return true;
    case setSupergroupUnrestrictBoostCount::ID:
      func(static_cast<setSupergroupUnrestrictBoostCount &>(obj));
      return true;
    case setSupergroupUsername::ID:
      func(static_cast<setSupergroupUsername &>(obj));
      return true;
    case setTdlibParameters::ID:
      func(static_cast<setTdlibParameters &>(obj));
      return true;
    case setUserPersonalProfilePhoto::ID:
      func(static_cast<setUserPersonalProfilePhoto &>(obj));
      return true;
    case setUserPrivacySettingRules::ID:
      func(static_cast<setUserPrivacySettingRules &>(obj));
      return true;
    case setUserSupportInfo::ID:
      func(static_cast<setUserSupportInfo &>(obj));
      return true;
    case setUsername::ID:
      func(static_cast<setUsername &>(obj));
      return true;
    case setVideoChatDefaultParticipant::ID:
      func(static_cast<setVideoChatDefaultParticipant &>(obj));
      return true;
    case shareChatWithBot::ID:
      func(static_cast<shareChatWithBot &>(obj));
      return true;
    case sharePhoneNumber::ID:
      func(static_cast<sharePhoneNumber &>(obj));
      return true;
    case shareUsersWithBot::ID:
      func(static_cast<shareUsersWithBot &>(obj));
      return true;
    case startGroupCallRecording::ID:
      func(static_cast<startGroupCallRecording &>(obj));
      return true;
    case startGroupCallScreenSharing::ID:
      func(static_cast<startGroupCallScreenSharing &>(obj));
      return true;
    case startScheduledGroupCall::ID:
      func(static_cast<startScheduledGroupCall &>(obj));
      return true;
    case stopBusinessPoll::ID:
      func(static_cast<stopBusinessPoll &>(obj));
      return true;
    case stopPoll::ID:
      func(static_cast<stopPoll &>(obj));
      return true;
    case suggestUserProfilePhoto::ID:
      func(static_cast<suggestUserProfilePhoto &>(obj));
      return true;
    case synchronizeLanguagePack::ID:
      func(static_cast<synchronizeLanguagePack &>(obj));
      return true;
    case terminateAllOtherSessions::ID:
      func(static_cast<terminateAllOtherSessions &>(obj));
      return true;
    case terminateSession::ID:
      func(static_cast<terminateSession &>(obj));
      return true;
    case testCallBytes::ID:
      func(static_cast<testCallBytes &>(obj));
      return true;
    case testCallEmpty::ID:
      func(static_cast<testCallEmpty &>(obj));
      return true;
    case testCallString::ID:
      func(static_cast<testCallString &>(obj));
      return true;
    case testCallVectorInt::ID:
      func(static_cast<testCallVectorInt &>(obj));
      return true;
    case testCallVectorIntObject::ID:
      func(static_cast<testCallVectorIntObject &>(obj));
      return true;
    case testCallVectorString::ID:
      func(static_cast<testCallVectorString &>(obj));
      return true;
    case testCallVectorStringObject::ID:
      func(static_cast<testCallVectorStringObject &>(obj));
      return true;
    case testGetDifference::ID:
      func(static_cast<testGetDifference &>(obj));
      return true;
    case testNetwork::ID:
      func(static_cast<testNetwork &>(obj));
      return true;
    case testProxy::ID:
      func(static_cast<testProxy &>(obj));
      return true;
    case testReturnError::ID:
      func(static_cast<testReturnError &>(obj));
      return true;
    case testSquareInt::ID:
      func(static_cast<testSquareInt &>(obj));
      return true;
    case testUseUpdate::ID:
      func(static_cast<testUseUpdate &>(obj));
      return true;
    case toggleAllDownloadsArePaused::ID:
      func(static_cast<toggleAllDownloadsArePaused &>(obj));
      return true;
    case toggleBotIsAddedToAttachmentMenu::ID:
      func(static_cast<toggleBotIsAddedToAttachmentMenu &>(obj));
      return true;
    case toggleBotUsernameIsActive::ID:
      func(static_cast<toggleBotUsernameIsActive &>(obj));
      return true;
    case toggleBusinessConnectedBotChatIsPaused::ID:
      func(static_cast<toggleBusinessConnectedBotChatIsPaused &>(obj));
      return true;
    case toggleChatDefaultDisableNotification::ID:
      func(static_cast<toggleChatDefaultDisableNotification &>(obj));
      return true;
    case toggleChatFolderTags::ID:
      func(static_cast<toggleChatFolderTags &>(obj));
      return true;
    case toggleChatHasProtectedContent::ID:
      func(static_cast<toggleChatHasProtectedContent &>(obj));
      return true;
    case toggleChatIsMarkedAsUnread::ID:
      func(static_cast<toggleChatIsMarkedAsUnread &>(obj));
      return true;
    case toggleChatIsPinned::ID:
      func(static_cast<toggleChatIsPinned &>(obj));
      return true;
    case toggleChatIsTranslatable::ID:
      func(static_cast<toggleChatIsTranslatable &>(obj));
      return true;
    case toggleChatViewAsTopics::ID:
      func(static_cast<toggleChatViewAsTopics &>(obj));
      return true;
    case toggleDownloadIsPaused::ID:
      func(static_cast<toggleDownloadIsPaused &>(obj));
      return true;
    case toggleForumTopicIsClosed::ID:
      func(static_cast<toggleForumTopicIsClosed &>(obj));
      return true;
    case toggleForumTopicIsPinned::ID:
      func(static_cast<toggleForumTopicIsPinned &>(obj));
      return true;
    case toggleGeneralForumTopicIsHidden::ID:
      func(static_cast<toggleGeneralForumTopicIsHidden &>(obj));
      return true;
    case toggleGroupCallEnabledStartNotification::ID:
      func(static_cast<toggleGroupCallEnabledStartNotification &>(obj));
      return true;
    case toggleGroupCallIsMyVideoEnabled::ID:
      func(static_cast<toggleGroupCallIsMyVideoEnabled &>(obj));
      return true;
    case toggleGroupCallIsMyVideoPaused::ID:
      func(static_cast<toggleGroupCallIsMyVideoPaused &>(obj));
      return true;
    case toggleGroupCallMuteNewParticipants::ID:
      func(static_cast<toggleGroupCallMuteNewParticipants &>(obj));
      return true;
    case toggleGroupCallParticipantIsHandRaised::ID:
      func(static_cast<toggleGroupCallParticipantIsHandRaised &>(obj));
      return true;
    case toggleGroupCallParticipantIsMuted::ID:
      func(static_cast<toggleGroupCallParticipantIsMuted &>(obj));
      return true;
    case toggleGroupCallScreenSharingIsPaused::ID:
      func(static_cast<toggleGroupCallScreenSharingIsPaused &>(obj));
      return true;
    case toggleHasSponsoredMessagesEnabled::ID:
      func(static_cast<toggleHasSponsoredMessagesEnabled &>(obj));
      return true;
    case togglePaidMessageReactionIsAnonymous::ID:
      func(static_cast<togglePaidMessageReactionIsAnonymous &>(obj));
      return true;
    case toggleSavedMessagesTopicIsPinned::ID:
      func(static_cast<toggleSavedMessagesTopicIsPinned &>(obj));
      return true;
    case toggleSessionCanAcceptCalls::ID:
      func(static_cast<toggleSessionCanAcceptCalls &>(obj));
      return true;
    case toggleSessionCanAcceptSecretChats::ID:
      func(static_cast<toggleSessionCanAcceptSecretChats &>(obj));
      return true;
    case toggleStoryIsPostedToChatPage::ID:
      func(static_cast<toggleStoryIsPostedToChatPage &>(obj));
      return true;
    case toggleSupergroupCanHaveSponsoredMessages::ID:
      func(static_cast<toggleSupergroupCanHaveSponsoredMessages &>(obj));
      return true;
    case toggleSupergroupHasAggressiveAntiSpamEnabled::ID:
      func(static_cast<toggleSupergroupHasAggressiveAntiSpamEnabled &>(obj));
      return true;
    case toggleSupergroupHasHiddenMembers::ID:
      func(static_cast<toggleSupergroupHasHiddenMembers &>(obj));
      return true;
    case toggleSupergroupIsAllHistoryAvailable::ID:
      func(static_cast<toggleSupergroupIsAllHistoryAvailable &>(obj));
      return true;
    case toggleSupergroupIsBroadcastGroup::ID:
      func(static_cast<toggleSupergroupIsBroadcastGroup &>(obj));
      return true;
    case toggleSupergroupIsForum::ID:
      func(static_cast<toggleSupergroupIsForum &>(obj));
      return true;
    case toggleSupergroupJoinByRequest::ID:
      func(static_cast<toggleSupergroupJoinByRequest &>(obj));
      return true;
    case toggleSupergroupJoinToSendMessages::ID:
      func(static_cast<toggleSupergroupJoinToSendMessages &>(obj));
      return true;
    case toggleSupergroupSignMessages::ID:
      func(static_cast<toggleSupergroupSignMessages &>(obj));
      return true;
    case toggleSupergroupUsernameIsActive::ID:
      func(static_cast<toggleSupergroupUsernameIsActive &>(obj));
      return true;
    case toggleUsernameIsActive::ID:
      func(static_cast<toggleUsernameIsActive &>(obj));
      return true;
    case transferChatOwnership::ID:
      func(static_cast<transferChatOwnership &>(obj));
      return true;
    case translateMessageText::ID:
      func(static_cast<translateMessageText &>(obj));
      return true;
    case translateText::ID:
      func(static_cast<translateText &>(obj));
      return true;
    case unpinAllChatMessages::ID:
      func(static_cast<unpinAllChatMessages &>(obj));
      return true;
    case unpinAllMessageThreadMessages::ID:
      func(static_cast<unpinAllMessageThreadMessages &>(obj));
      return true;
    case unpinChatMessage::ID:
      func(static_cast<unpinChatMessage &>(obj));
      return true;
    case upgradeBasicGroupChatToSupergroupChat::ID:
      func(static_cast<upgradeBasicGroupChatToSupergroupChat &>(obj));
      return true;
    case uploadStickerFile::ID:
      func(static_cast<uploadStickerFile &>(obj));
      return true;
    case validateOrderInfo::ID:
      func(static_cast<validateOrderInfo &>(obj));
      return true;
    case viewMessages::ID:
      func(static_cast<viewMessages &>(obj));
      return true;
    case viewPremiumFeature::ID:
      func(static_cast<viewPremiumFeature &>(obj));
      return true;
    case viewTrendingStickerSets::ID:
      func(static_cast<viewTrendingStickerSets &>(obj));
      return true;
    case writeGeneratedFilePart::ID:
      func(static_cast<writeGeneratedFilePart &>(obj));
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
bool downcast_call(AuthenticationCodeType &obj, const T &func) {
  switch (obj.get_id()) {
    case authenticationCodeTypeTelegramMessage::ID:
      func(static_cast<authenticationCodeTypeTelegramMessage &>(obj));
      return true;
    case authenticationCodeTypeSms::ID:
      func(static_cast<authenticationCodeTypeSms &>(obj));
      return true;
    case authenticationCodeTypeSmsWord::ID:
      func(static_cast<authenticationCodeTypeSmsWord &>(obj));
      return true;
    case authenticationCodeTypeSmsPhrase::ID:
      func(static_cast<authenticationCodeTypeSmsPhrase &>(obj));
      return true;
    case authenticationCodeTypeCall::ID:
      func(static_cast<authenticationCodeTypeCall &>(obj));
      return true;
    case authenticationCodeTypeFlashCall::ID:
      func(static_cast<authenticationCodeTypeFlashCall &>(obj));
      return true;
    case authenticationCodeTypeMissedCall::ID:
      func(static_cast<authenticationCodeTypeMissedCall &>(obj));
      return true;
    case authenticationCodeTypeFragment::ID:
      func(static_cast<authenticationCodeTypeFragment &>(obj));
      return true;
    case authenticationCodeTypeFirebaseAndroid::ID:
      func(static_cast<authenticationCodeTypeFirebaseAndroid &>(obj));
      return true;
    case authenticationCodeTypeFirebaseIos::ID:
      func(static_cast<authenticationCodeTypeFirebaseIos &>(obj));
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
bool downcast_call(AuthorizationState &obj, const T &func) {
  switch (obj.get_id()) {
    case authorizationStateWaitTdlibParameters::ID:
      func(static_cast<authorizationStateWaitTdlibParameters &>(obj));
      return true;
    case authorizationStateWaitPhoneNumber::ID:
      func(static_cast<authorizationStateWaitPhoneNumber &>(obj));
      return true;
    case authorizationStateWaitEmailAddress::ID:
      func(static_cast<authorizationStateWaitEmailAddress &>(obj));
      return true;
    case authorizationStateWaitEmailCode::ID:
      func(static_cast<authorizationStateWaitEmailCode &>(obj));
      return true;
    case authorizationStateWaitCode::ID:
      func(static_cast<authorizationStateWaitCode &>(obj));
      return true;
    case authorizationStateWaitOtherDeviceConfirmation::ID:
      func(static_cast<authorizationStateWaitOtherDeviceConfirmation &>(obj));
      return true;
    case authorizationStateWaitRegistration::ID:
      func(static_cast<authorizationStateWaitRegistration &>(obj));
      return true;
    case authorizationStateWaitPassword::ID:
      func(static_cast<authorizationStateWaitPassword &>(obj));
      return true;
    case authorizationStateReady::ID:
      func(static_cast<authorizationStateReady &>(obj));
      return true;
    case authorizationStateLoggingOut::ID:
      func(static_cast<authorizationStateLoggingOut &>(obj));
      return true;
    case authorizationStateClosing::ID:
      func(static_cast<authorizationStateClosing &>(obj));
      return true;
    case authorizationStateClosed::ID:
      func(static_cast<authorizationStateClosed &>(obj));
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
bool downcast_call(AutosaveSettingsScope &obj, const T &func) {
  switch (obj.get_id()) {
    case autosaveSettingsScopePrivateChats::ID:
      func(static_cast<autosaveSettingsScopePrivateChats &>(obj));
      return true;
    case autosaveSettingsScopeGroupChats::ID:
      func(static_cast<autosaveSettingsScopeGroupChats &>(obj));
      return true;
    case autosaveSettingsScopeChannelChats::ID:
      func(static_cast<autosaveSettingsScopeChannelChats &>(obj));
      return true;
    case autosaveSettingsScopeChat::ID:
      func(static_cast<autosaveSettingsScopeChat &>(obj));
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
bool downcast_call(BackgroundFill &obj, const T &func) {
  switch (obj.get_id()) {
    case backgroundFillSolid::ID:
      func(static_cast<backgroundFillSolid &>(obj));
      return true;
    case backgroundFillGradient::ID:
      func(static_cast<backgroundFillGradient &>(obj));
      return true;
    case backgroundFillFreeformGradient::ID:
      func(static_cast<backgroundFillFreeformGradient &>(obj));
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
bool downcast_call(BackgroundType &obj, const T &func) {
  switch (obj.get_id()) {
    case backgroundTypeWallpaper::ID:
      func(static_cast<backgroundTypeWallpaper &>(obj));
      return true;
    case backgroundTypePattern::ID:
      func(static_cast<backgroundTypePattern &>(obj));
      return true;
    case backgroundTypeFill::ID:
      func(static_cast<backgroundTypeFill &>(obj));
      return true;
    case backgroundTypeChatTheme::ID:
      func(static_cast<backgroundTypeChatTheme &>(obj));
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
bool downcast_call(BlockList &obj, const T &func) {
  switch (obj.get_id()) {
    case blockListMain::ID:
      func(static_cast<blockListMain &>(obj));
      return true;
    case blockListStories::ID:
      func(static_cast<blockListStories &>(obj));
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
bool downcast_call(BotCommandScope &obj, const T &func) {
  switch (obj.get_id()) {
    case botCommandScopeDefault::ID:
      func(static_cast<botCommandScopeDefault &>(obj));
      return true;
    case botCommandScopeAllPrivateChats::ID:
      func(static_cast<botCommandScopeAllPrivateChats &>(obj));
      return true;
    case botCommandScopeAllGroupChats::ID:
      func(static_cast<botCommandScopeAllGroupChats &>(obj));
      return true;
    case botCommandScopeAllChatAdministrators::ID:
      func(static_cast<botCommandScopeAllChatAdministrators &>(obj));
      return true;
    case botCommandScopeChat::ID:
      func(static_cast<botCommandScopeChat &>(obj));
      return true;
    case botCommandScopeChatAdministrators::ID:
      func(static_cast<botCommandScopeChatAdministrators &>(obj));
      return true;
    case botCommandScopeChatMember::ID:
      func(static_cast<botCommandScopeChatMember &>(obj));
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
bool downcast_call(BotTransactionPurpose &obj, const T &func) {
  switch (obj.get_id()) {
    case botTransactionPurposePaidMedia::ID:
      func(static_cast<botTransactionPurposePaidMedia &>(obj));
      return true;
    case botTransactionPurposeInvoicePayment::ID:
      func(static_cast<botTransactionPurposeInvoicePayment &>(obj));
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
bool downcast_call(BotWriteAccessAllowReason &obj, const T &func) {
  switch (obj.get_id()) {
    case botWriteAccessAllowReasonConnectedWebsite::ID:
      func(static_cast<botWriteAccessAllowReasonConnectedWebsite &>(obj));
      return true;
    case botWriteAccessAllowReasonAddedToAttachmentMenu::ID:
      func(static_cast<botWriteAccessAllowReasonAddedToAttachmentMenu &>(obj));
      return true;
    case botWriteAccessAllowReasonLaunchedWebApp::ID:
      func(static_cast<botWriteAccessAllowReasonLaunchedWebApp &>(obj));
      return true;
    case botWriteAccessAllowReasonAcceptedRequest::ID:
      func(static_cast<botWriteAccessAllowReasonAcceptedRequest &>(obj));
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
bool downcast_call(BusinessAwayMessageSchedule &obj, const T &func) {
  switch (obj.get_id()) {
    case businessAwayMessageScheduleAlways::ID:
      func(static_cast<businessAwayMessageScheduleAlways &>(obj));
      return true;
    case businessAwayMessageScheduleOutsideOfOpeningHours::ID:
      func(static_cast<businessAwayMessageScheduleOutsideOfOpeningHours &>(obj));
      return true;
    case businessAwayMessageScheduleCustom::ID:
      func(static_cast<businessAwayMessageScheduleCustom &>(obj));
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
bool downcast_call(BusinessFeature &obj, const T &func) {
  switch (obj.get_id()) {
    case businessFeatureLocation::ID:
      func(static_cast<businessFeatureLocation &>(obj));
      return true;
    case businessFeatureOpeningHours::ID:
      func(static_cast<businessFeatureOpeningHours &>(obj));
      return true;
    case businessFeatureQuickReplies::ID:
      func(static_cast<businessFeatureQuickReplies &>(obj));
      return true;
    case businessFeatureGreetingMessage::ID:
      func(static_cast<businessFeatureGreetingMessage &>(obj));
      return true;
    case businessFeatureAwayMessage::ID:
      func(static_cast<businessFeatureAwayMessage &>(obj));
      return true;
    case businessFeatureAccountLinks::ID:
      func(static_cast<businessFeatureAccountLinks &>(obj));
      return true;
    case businessFeatureStartPage::ID:
      func(static_cast<businessFeatureStartPage &>(obj));
      return true;
    case businessFeatureBots::ID:
      func(static_cast<businessFeatureBots &>(obj));
      return true;
    case businessFeatureEmojiStatus::ID:
      func(static_cast<businessFeatureEmojiStatus &>(obj));
      return true;
    case businessFeatureChatFolderTags::ID:
      func(static_cast<businessFeatureChatFolderTags &>(obj));
      return true;
    case businessFeatureUpgradedStories::ID:
      func(static_cast<businessFeatureUpgradedStories &>(obj));
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
bool downcast_call(CallDiscardReason &obj, const T &func) {
  switch (obj.get_id()) {
    case callDiscardReasonEmpty::ID:
      func(static_cast<callDiscardReasonEmpty &>(obj));
      return true;
    case callDiscardReasonMissed::ID:
      func(static_cast<callDiscardReasonMissed &>(obj));
      return true;
    case callDiscardReasonDeclined::ID:
      func(static_cast<callDiscardReasonDeclined &>(obj));
      return true;
    case callDiscardReasonDisconnected::ID:
      func(static_cast<callDiscardReasonDisconnected &>(obj));
      return true;
    case callDiscardReasonHungUp::ID:
      func(static_cast<callDiscardReasonHungUp &>(obj));
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
bool downcast_call(CallProblem &obj, const T &func) {
  switch (obj.get_id()) {
    case callProblemEcho::ID:
      func(static_cast<callProblemEcho &>(obj));
      return true;
    case callProblemNoise::ID:
      func(static_cast<callProblemNoise &>(obj));
      return true;
    case callProblemInterruptions::ID:
      func(static_cast<callProblemInterruptions &>(obj));
      return true;
    case callProblemDistortedSpeech::ID:
      func(static_cast<callProblemDistortedSpeech &>(obj));
      return true;
    case callProblemSilentLocal::ID:
      func(static_cast<callProblemSilentLocal &>(obj));
      return true;
    case callProblemSilentRemote::ID:
      func(static_cast<callProblemSilentRemote &>(obj));
      return true;
    case callProblemDropped::ID:
      func(static_cast<callProblemDropped &>(obj));
      return true;
    case callProblemDistortedVideo::ID:
      func(static_cast<callProblemDistortedVideo &>(obj));
      return true;
    case callProblemPixelatedVideo::ID:
      func(static_cast<callProblemPixelatedVideo &>(obj));
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
bool downcast_call(CallServerType &obj, const T &func) {
  switch (obj.get_id()) {
    case callServerTypeTelegramReflector::ID:
      func(static_cast<callServerTypeTelegramReflector &>(obj));
      return true;
    case callServerTypeWebrtc::ID:
      func(static_cast<callServerTypeWebrtc &>(obj));
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
bool downcast_call(CallState &obj, const T &func) {
  switch (obj.get_id()) {
    case callStatePending::ID:
      func(static_cast<callStatePending &>(obj));
      return true;
    case callStateExchangingKeys::ID:
      func(static_cast<callStateExchangingKeys &>(obj));
      return true;
    case callStateReady::ID:
      func(static_cast<callStateReady &>(obj));
      return true;
    case callStateHangingUp::ID:
      func(static_cast<callStateHangingUp &>(obj));
      return true;
    case callStateDiscarded::ID:
      func(static_cast<callStateDiscarded &>(obj));
      return true;
    case callStateError::ID:
      func(static_cast<callStateError &>(obj));
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
bool downcast_call(CallbackQueryPayload &obj, const T &func) {
  switch (obj.get_id()) {
    case callbackQueryPayloadData::ID:
      func(static_cast<callbackQueryPayloadData &>(obj));
      return true;
    case callbackQueryPayloadDataWithPassword::ID:
      func(static_cast<callbackQueryPayloadDataWithPassword &>(obj));
      return true;
    case callbackQueryPayloadGame::ID:
      func(static_cast<callbackQueryPayloadGame &>(obj));
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
bool downcast_call(CanSendMessageToUserResult &obj, const T &func) {
  switch (obj.get_id()) {
    case canSendMessageToUserResultOk::ID:
      func(static_cast<canSendMessageToUserResultOk &>(obj));
      return true;
    case canSendMessageToUserResultUserIsDeleted::ID:
      func(static_cast<canSendMessageToUserResultUserIsDeleted &>(obj));
      return true;
    case canSendMessageToUserResultUserRestrictsNewChats::ID:
      func(static_cast<canSendMessageToUserResultUserRestrictsNewChats &>(obj));
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
bool downcast_call(CanSendStoryResult &obj, const T &func) {
  switch (obj.get_id()) {
    case canSendStoryResultOk::ID:
      func(static_cast<canSendStoryResultOk &>(obj));
      return true;
    case canSendStoryResultPremiumNeeded::ID:
      func(static_cast<canSendStoryResultPremiumNeeded &>(obj));
      return true;
    case canSendStoryResultBoostNeeded::ID:
      func(static_cast<canSendStoryResultBoostNeeded &>(obj));
      return true;
    case canSendStoryResultActiveStoryLimitExceeded::ID:
      func(static_cast<canSendStoryResultActiveStoryLimitExceeded &>(obj));
      return true;
    case canSendStoryResultWeeklyLimitExceeded::ID:
      func(static_cast<canSendStoryResultWeeklyLimitExceeded &>(obj));
      return true;
    case canSendStoryResultMonthlyLimitExceeded::ID:
      func(static_cast<canSendStoryResultMonthlyLimitExceeded &>(obj));
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
bool downcast_call(CanTransferOwnershipResult &obj, const T &func) {
  switch (obj.get_id()) {
    case canTransferOwnershipResultOk::ID:
      func(static_cast<canTransferOwnershipResultOk &>(obj));
      return true;
    case canTransferOwnershipResultPasswordNeeded::ID:
      func(static_cast<canTransferOwnershipResultPasswordNeeded &>(obj));
      return true;
    case canTransferOwnershipResultPasswordTooFresh::ID:
      func(static_cast<canTransferOwnershipResultPasswordTooFresh &>(obj));
      return true;
    case canTransferOwnershipResultSessionTooFresh::ID:
      func(static_cast<canTransferOwnershipResultSessionTooFresh &>(obj));
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
bool downcast_call(ChatAction &obj, const T &func) {
  switch (obj.get_id()) {
    case chatActionTyping::ID:
      func(static_cast<chatActionTyping &>(obj));
      return true;
    case chatActionRecordingVideo::ID:
      func(static_cast<chatActionRecordingVideo &>(obj));
      return true;
    case chatActionUploadingVideo::ID:
      func(static_cast<chatActionUploadingVideo &>(obj));
      return true;
    case chatActionRecordingVoiceNote::ID:
      func(static_cast<chatActionRecordingVoiceNote &>(obj));
      return true;
    case chatActionUploadingVoiceNote::ID:
      func(static_cast<chatActionUploadingVoiceNote &>(obj));
      return true;
    case chatActionUploadingPhoto::ID:
      func(static_cast<chatActionUploadingPhoto &>(obj));
      return true;
    case chatActionUploadingDocument::ID:
      func(static_cast<chatActionUploadingDocument &>(obj));
      return true;
    case chatActionChoosingSticker::ID:
      func(static_cast<chatActionChoosingSticker &>(obj));
      return true;
    case chatActionChoosingLocation::ID:
      func(static_cast<chatActionChoosingLocation &>(obj));
      return true;
    case chatActionChoosingContact::ID:
      func(static_cast<chatActionChoosingContact &>(obj));
      return true;
    case chatActionStartPlayingGame::ID:
      func(static_cast<chatActionStartPlayingGame &>(obj));
      return true;
    case chatActionRecordingVideoNote::ID:
      func(static_cast<chatActionRecordingVideoNote &>(obj));
      return true;
    case chatActionUploadingVideoNote::ID:
      func(static_cast<chatActionUploadingVideoNote &>(obj));
      return true;
    case chatActionWatchingAnimations::ID:
      func(static_cast<chatActionWatchingAnimations &>(obj));
      return true;
    case chatActionCancel::ID:
      func(static_cast<chatActionCancel &>(obj));
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
bool downcast_call(ChatActionBar &obj, const T &func) {
  switch (obj.get_id()) {
    case chatActionBarReportSpam::ID:
      func(static_cast<chatActionBarReportSpam &>(obj));
      return true;
    case chatActionBarReportUnrelatedLocation::ID:
      func(static_cast<chatActionBarReportUnrelatedLocation &>(obj));
      return true;
    case chatActionBarInviteMembers::ID:
      func(static_cast<chatActionBarInviteMembers &>(obj));
      return true;
    case chatActionBarReportAddBlock::ID:
      func(static_cast<chatActionBarReportAddBlock &>(obj));
      return true;
    case chatActionBarAddContact::ID:
      func(static_cast<chatActionBarAddContact &>(obj));
      return true;
    case chatActionBarSharePhoneNumber::ID:
      func(static_cast<chatActionBarSharePhoneNumber &>(obj));
      return true;
    case chatActionBarJoinRequest::ID:
      func(static_cast<chatActionBarJoinRequest &>(obj));
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
bool downcast_call(ChatAvailableReactions &obj, const T &func) {
  switch (obj.get_id()) {
    case chatAvailableReactionsAll::ID:
      func(static_cast<chatAvailableReactionsAll &>(obj));
      return true;
    case chatAvailableReactionsSome::ID:
      func(static_cast<chatAvailableReactionsSome &>(obj));
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
bool downcast_call(ChatBoostSource &obj, const T &func) {
  switch (obj.get_id()) {
    case chatBoostSourceGiftCode::ID:
      func(static_cast<chatBoostSourceGiftCode &>(obj));
      return true;
    case chatBoostSourceGiveaway::ID:
      func(static_cast<chatBoostSourceGiveaway &>(obj));
      return true;
    case chatBoostSourcePremium::ID:
      func(static_cast<chatBoostSourcePremium &>(obj));
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
bool downcast_call(ChatEventAction &obj, const T &func) {
  switch (obj.get_id()) {
    case chatEventMessageEdited::ID:
      func(static_cast<chatEventMessageEdited &>(obj));
      return true;
    case chatEventMessageDeleted::ID:
      func(static_cast<chatEventMessageDeleted &>(obj));
      return true;
    case chatEventMessagePinned::ID:
      func(static_cast<chatEventMessagePinned &>(obj));
      return true;
    case chatEventMessageUnpinned::ID:
      func(static_cast<chatEventMessageUnpinned &>(obj));
      return true;
    case chatEventPollStopped::ID:
      func(static_cast<chatEventPollStopped &>(obj));
      return true;
    case chatEventMemberJoined::ID:
      func(static_cast<chatEventMemberJoined &>(obj));
      return true;
    case chatEventMemberJoinedByInviteLink::ID:
      func(static_cast<chatEventMemberJoinedByInviteLink &>(obj));
      return true;
    case chatEventMemberJoinedByRequest::ID:
      func(static_cast<chatEventMemberJoinedByRequest &>(obj));
      return true;
    case chatEventMemberInvited::ID:
      func(static_cast<chatEventMemberInvited &>(obj));
      return true;
    case chatEventMemberLeft::ID:
      func(static_cast<chatEventMemberLeft &>(obj));
      return true;
    case chatEventMemberPromoted::ID:
      func(static_cast<chatEventMemberPromoted &>(obj));
      return true;
    case chatEventMemberRestricted::ID:
      func(static_cast<chatEventMemberRestricted &>(obj));
      return true;
    case chatEventMemberSubscriptionExtended::ID:
      func(static_cast<chatEventMemberSubscriptionExtended &>(obj));
      return true;
    case chatEventAvailableReactionsChanged::ID:
      func(static_cast<chatEventAvailableReactionsChanged &>(obj));
      return true;
    case chatEventBackgroundChanged::ID:
      func(static_cast<chatEventBackgroundChanged &>(obj));
      return true;
    case chatEventDescriptionChanged::ID:
      func(static_cast<chatEventDescriptionChanged &>(obj));
      return true;
    case chatEventEmojiStatusChanged::ID:
      func(static_cast<chatEventEmojiStatusChanged &>(obj));
      return true;
    case chatEventLinkedChatChanged::ID:
      func(static_cast<chatEventLinkedChatChanged &>(obj));
      return true;
    case chatEventLocationChanged::ID:
      func(static_cast<chatEventLocationChanged &>(obj));
      return true;
    case chatEventMessageAutoDeleteTimeChanged::ID:
      func(static_cast<chatEventMessageAutoDeleteTimeChanged &>(obj));
      return true;
    case chatEventPermissionsChanged::ID:
      func(static_cast<chatEventPermissionsChanged &>(obj));
      return true;
    case chatEventPhotoChanged::ID:
      func(static_cast<chatEventPhotoChanged &>(obj));
      return true;
    case chatEventSlowModeDelayChanged::ID:
      func(static_cast<chatEventSlowModeDelayChanged &>(obj));
      return true;
    case chatEventStickerSetChanged::ID:
      func(static_cast<chatEventStickerSetChanged &>(obj));
      return true;
    case chatEventCustomEmojiStickerSetChanged::ID:
      func(static_cast<chatEventCustomEmojiStickerSetChanged &>(obj));
      return true;
    case chatEventTitleChanged::ID:
      func(static_cast<chatEventTitleChanged &>(obj));
      return true;
    case chatEventUsernameChanged::ID:
      func(static_cast<chatEventUsernameChanged &>(obj));
      return true;
    case chatEventActiveUsernamesChanged::ID:
      func(static_cast<chatEventActiveUsernamesChanged &>(obj));
      return true;
    case chatEventAccentColorChanged::ID:
      func(static_cast<chatEventAccentColorChanged &>(obj));
      return true;
    case chatEventProfileAccentColorChanged::ID:
      func(static_cast<chatEventProfileAccentColorChanged &>(obj));
      return true;
    case chatEventHasProtectedContentToggled::ID:
      func(static_cast<chatEventHasProtectedContentToggled &>(obj));
      return true;
    case chatEventInvitesToggled::ID:
      func(static_cast<chatEventInvitesToggled &>(obj));
      return true;
    case chatEventIsAllHistoryAvailableToggled::ID:
      func(static_cast<chatEventIsAllHistoryAvailableToggled &>(obj));
      return true;
    case chatEventHasAggressiveAntiSpamEnabledToggled::ID:
      func(static_cast<chatEventHasAggressiveAntiSpamEnabledToggled &>(obj));
      return true;
    case chatEventSignMessagesToggled::ID:
      func(static_cast<chatEventSignMessagesToggled &>(obj));
      return true;
    case chatEventShowMessageSenderToggled::ID:
      func(static_cast<chatEventShowMessageSenderToggled &>(obj));
      return true;
    case chatEventInviteLinkEdited::ID:
      func(static_cast<chatEventInviteLinkEdited &>(obj));
      return true;
    case chatEventInviteLinkRevoked::ID:
      func(static_cast<chatEventInviteLinkRevoked &>(obj));
      return true;
    case chatEventInviteLinkDeleted::ID:
      func(static_cast<chatEventInviteLinkDeleted &>(obj));
      return true;
    case chatEventVideoChatCreated::ID:
      func(static_cast<chatEventVideoChatCreated &>(obj));
      return true;
    case chatEventVideoChatEnded::ID:
      func(static_cast<chatEventVideoChatEnded &>(obj));
      return true;
    case chatEventVideoChatMuteNewParticipantsToggled::ID:
      func(static_cast<chatEventVideoChatMuteNewParticipantsToggled &>(obj));
      return true;
    case chatEventVideoChatParticipantIsMutedToggled::ID:
      func(static_cast<chatEventVideoChatParticipantIsMutedToggled &>(obj));
      return true;
    case chatEventVideoChatParticipantVolumeLevelChanged::ID:
      func(static_cast<chatEventVideoChatParticipantVolumeLevelChanged &>(obj));
      return true;
    case chatEventIsForumToggled::ID:
      func(static_cast<chatEventIsForumToggled &>(obj));
      return true;
    case chatEventForumTopicCreated::ID:
      func(static_cast<chatEventForumTopicCreated &>(obj));
      return true;
    case chatEventForumTopicEdited::ID:
      func(static_cast<chatEventForumTopicEdited &>(obj));
      return true;
    case chatEventForumTopicToggleIsClosed::ID:
      func(static_cast<chatEventForumTopicToggleIsClosed &>(obj));
      return true;
    case chatEventForumTopicToggleIsHidden::ID:
      func(static_cast<chatEventForumTopicToggleIsHidden &>(obj));
      return true;
    case chatEventForumTopicDeleted::ID:
      func(static_cast<chatEventForumTopicDeleted &>(obj));
      return true;
    case chatEventForumTopicPinned::ID:
      func(static_cast<chatEventForumTopicPinned &>(obj));
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
bool downcast_call(ChatList &obj, const T &func) {
  switch (obj.get_id()) {
    case chatListMain::ID:
      func(static_cast<chatListMain &>(obj));
      return true;
    case chatListArchive::ID:
      func(static_cast<chatListArchive &>(obj));
      return true;
    case chatListFolder::ID:
      func(static_cast<chatListFolder &>(obj));
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
bool downcast_call(ChatMemberStatus &obj, const T &func) {
  switch (obj.get_id()) {
    case chatMemberStatusCreator::ID:
      func(static_cast<chatMemberStatusCreator &>(obj));
      return true;
    case chatMemberStatusAdministrator::ID:
      func(static_cast<chatMemberStatusAdministrator &>(obj));
      return true;
    case chatMemberStatusMember::ID:
      func(static_cast<chatMemberStatusMember &>(obj));
      return true;
    case chatMemberStatusRestricted::ID:
      func(static_cast<chatMemberStatusRestricted &>(obj));
      return true;
    case chatMemberStatusLeft::ID:
      func(static_cast<chatMemberStatusLeft &>(obj));
      return true;
    case chatMemberStatusBanned::ID:
      func(static_cast<chatMemberStatusBanned &>(obj));
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
bool downcast_call(ChatMembersFilter &obj, const T &func) {
  switch (obj.get_id()) {
    case chatMembersFilterContacts::ID:
      func(static_cast<chatMembersFilterContacts &>(obj));
      return true;
    case chatMembersFilterAdministrators::ID:
      func(static_cast<chatMembersFilterAdministrators &>(obj));
      return true;
    case chatMembersFilterMembers::ID:
      func(static_cast<chatMembersFilterMembers &>(obj));
      return true;
    case chatMembersFilterMention::ID:
      func(static_cast<chatMembersFilterMention &>(obj));
      return true;
    case chatMembersFilterRestricted::ID:
      func(static_cast<chatMembersFilterRestricted &>(obj));
      return true;
    case chatMembersFilterBanned::ID:
      func(static_cast<chatMembersFilterBanned &>(obj));
      return true;
    case chatMembersFilterBots::ID:
      func(static_cast<chatMembersFilterBots &>(obj));
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
bool downcast_call(ChatPhotoStickerType &obj, const T &func) {
  switch (obj.get_id()) {
    case chatPhotoStickerTypeRegularOrMask::ID:
      func(static_cast<chatPhotoStickerTypeRegularOrMask &>(obj));
      return true;
    case chatPhotoStickerTypeCustomEmoji::ID:
      func(static_cast<chatPhotoStickerTypeCustomEmoji &>(obj));
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
bool downcast_call(ChatRevenueTransactionType &obj, const T &func) {
  switch (obj.get_id()) {
    case chatRevenueTransactionTypeEarnings::ID:
      func(static_cast<chatRevenueTransactionTypeEarnings &>(obj));
      return true;
    case chatRevenueTransactionTypeWithdrawal::ID:
      func(static_cast<chatRevenueTransactionTypeWithdrawal &>(obj));
      return true;
    case chatRevenueTransactionTypeRefund::ID:
      func(static_cast<chatRevenueTransactionTypeRefund &>(obj));
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
bool downcast_call(ChatSource &obj, const T &func) {
  switch (obj.get_id()) {
    case chatSourceMtprotoProxy::ID:
      func(static_cast<chatSourceMtprotoProxy &>(obj));
      return true;
    case chatSourcePublicServiceAnnouncement::ID:
      func(static_cast<chatSourcePublicServiceAnnouncement &>(obj));
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
bool downcast_call(ChatStatistics &obj, const T &func) {
  switch (obj.get_id()) {
    case chatStatisticsSupergroup::ID:
      func(static_cast<chatStatisticsSupergroup &>(obj));
      return true;
    case chatStatisticsChannel::ID:
      func(static_cast<chatStatisticsChannel &>(obj));
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
bool downcast_call(ChatStatisticsObjectType &obj, const T &func) {
  switch (obj.get_id()) {
    case chatStatisticsObjectTypeMessage::ID:
      func(static_cast<chatStatisticsObjectTypeMessage &>(obj));
      return true;
    case chatStatisticsObjectTypeStory::ID:
      func(static_cast<chatStatisticsObjectTypeStory &>(obj));
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
bool downcast_call(ChatTransactionPurpose &obj, const T &func) {
  switch (obj.get_id()) {
    case chatTransactionPurposePaidMedia::ID:
      func(static_cast<chatTransactionPurposePaidMedia &>(obj));
      return true;
    case chatTransactionPurposeJoin::ID:
      func(static_cast<chatTransactionPurposeJoin &>(obj));
      return true;
    case chatTransactionPurposeReaction::ID:
      func(static_cast<chatTransactionPurposeReaction &>(obj));
      return true;
    case chatTransactionPurposeGiveaway::ID:
      func(static_cast<chatTransactionPurposeGiveaway &>(obj));
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
bool downcast_call(ChatType &obj, const T &func) {
  switch (obj.get_id()) {
    case chatTypePrivate::ID:
      func(static_cast<chatTypePrivate &>(obj));
      return true;
    case chatTypeBasicGroup::ID:
      func(static_cast<chatTypeBasicGroup &>(obj));
      return true;
    case chatTypeSupergroup::ID:
      func(static_cast<chatTypeSupergroup &>(obj));
      return true;
    case chatTypeSecret::ID:
      func(static_cast<chatTypeSecret &>(obj));
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
bool downcast_call(CheckChatUsernameResult &obj, const T &func) {
  switch (obj.get_id()) {
    case checkChatUsernameResultOk::ID:
      func(static_cast<checkChatUsernameResultOk &>(obj));
      return true;
    case checkChatUsernameResultUsernameInvalid::ID:
      func(static_cast<checkChatUsernameResultUsernameInvalid &>(obj));
      return true;
    case checkChatUsernameResultUsernameOccupied::ID:
      func(static_cast<checkChatUsernameResultUsernameOccupied &>(obj));
      return true;
    case checkChatUsernameResultUsernamePurchasable::ID:
      func(static_cast<checkChatUsernameResultUsernamePurchasable &>(obj));
      return true;
    case checkChatUsernameResultPublicChatsTooMany::ID:
      func(static_cast<checkChatUsernameResultPublicChatsTooMany &>(obj));
      return true;
    case checkChatUsernameResultPublicGroupsUnavailable::ID:
      func(static_cast<checkChatUsernameResultPublicGroupsUnavailable &>(obj));
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
bool downcast_call(CheckStickerSetNameResult &obj, const T &func) {
  switch (obj.get_id()) {
    case checkStickerSetNameResultOk::ID:
      func(static_cast<checkStickerSetNameResultOk &>(obj));
      return true;
    case checkStickerSetNameResultNameInvalid::ID:
      func(static_cast<checkStickerSetNameResultNameInvalid &>(obj));
      return true;
    case checkStickerSetNameResultNameOccupied::ID:
      func(static_cast<checkStickerSetNameResultNameOccupied &>(obj));
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
bool downcast_call(CollectibleItemType &obj, const T &func) {
  switch (obj.get_id()) {
    case collectibleItemTypeUsername::ID:
      func(static_cast<collectibleItemTypeUsername &>(obj));
      return true;
    case collectibleItemTypePhoneNumber::ID:
      func(static_cast<collectibleItemTypePhoneNumber &>(obj));
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
bool downcast_call(ConnectionState &obj, const T &func) {
  switch (obj.get_id()) {
    case connectionStateWaitingForNetwork::ID:
      func(static_cast<connectionStateWaitingForNetwork &>(obj));
      return true;
    case connectionStateConnectingToProxy::ID:
      func(static_cast<connectionStateConnectingToProxy &>(obj));
      return true;
    case connectionStateConnecting::ID:
      func(static_cast<connectionStateConnecting &>(obj));
      return true;
    case connectionStateUpdating::ID:
      func(static_cast<connectionStateUpdating &>(obj));
      return true;
    case connectionStateReady::ID:
      func(static_cast<connectionStateReady &>(obj));
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
bool downcast_call(DeviceToken &obj, const T &func) {
  switch (obj.get_id()) {
    case deviceTokenFirebaseCloudMessaging::ID:
      func(static_cast<deviceTokenFirebaseCloudMessaging &>(obj));
      return true;
    case deviceTokenApplePush::ID:
      func(static_cast<deviceTokenApplePush &>(obj));
      return true;
    case deviceTokenApplePushVoIP::ID:
      func(static_cast<deviceTokenApplePushVoIP &>(obj));
      return true;
    case deviceTokenWindowsPush::ID:
      func(static_cast<deviceTokenWindowsPush &>(obj));
      return true;
    case deviceTokenMicrosoftPush::ID:
      func(static_cast<deviceTokenMicrosoftPush &>(obj));
      return true;
    case deviceTokenMicrosoftPushVoIP::ID:
      func(static_cast<deviceTokenMicrosoftPushVoIP &>(obj));
      return true;
    case deviceTokenWebPush::ID:
      func(static_cast<deviceTokenWebPush &>(obj));
      return true;
    case deviceTokenSimplePush::ID:
      func(static_cast<deviceTokenSimplePush &>(obj));
      return true;
    case deviceTokenUbuntuPush::ID:
      func(static_cast<deviceTokenUbuntuPush &>(obj));
      return true;
    case deviceTokenBlackBerryPush::ID:
      func(static_cast<deviceTokenBlackBerryPush &>(obj));
      return true;
    case deviceTokenTizenPush::ID:
      func(static_cast<deviceTokenTizenPush &>(obj));
      return true;
    case deviceTokenHuaweiPush::ID:
      func(static_cast<deviceTokenHuaweiPush &>(obj));
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
bool downcast_call(DiceStickers &obj, const T &func) {
  switch (obj.get_id()) {
    case diceStickersRegular::ID:
      func(static_cast<diceStickersRegular &>(obj));
      return true;
    case diceStickersSlotMachine::ID:
      func(static_cast<diceStickersSlotMachine &>(obj));
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
bool downcast_call(EmailAddressAuthentication &obj, const T &func) {
  switch (obj.get_id()) {
    case emailAddressAuthenticationCode::ID:
      func(static_cast<emailAddressAuthenticationCode &>(obj));
      return true;
    case emailAddressAuthenticationAppleId::ID:
      func(static_cast<emailAddressAuthenticationAppleId &>(obj));
      return true;
    case emailAddressAuthenticationGoogleId::ID:
      func(static_cast<emailAddressAuthenticationGoogleId &>(obj));
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
bool downcast_call(EmailAddressResetState &obj, const T &func) {
  switch (obj.get_id()) {
    case emailAddressResetStateAvailable::ID:
      func(static_cast<emailAddressResetStateAvailable &>(obj));
      return true;
    case emailAddressResetStatePending::ID:
      func(static_cast<emailAddressResetStatePending &>(obj));
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
bool downcast_call(EmojiCategorySource &obj, const T &func) {
  switch (obj.get_id()) {
    case emojiCategorySourceSearch::ID:
      func(static_cast<emojiCategorySourceSearch &>(obj));
      return true;
    case emojiCategorySourcePremium::ID:
      func(static_cast<emojiCategorySourcePremium &>(obj));
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
bool downcast_call(EmojiCategoryType &obj, const T &func) {
  switch (obj.get_id()) {
    case emojiCategoryTypeDefault::ID:
      func(static_cast<emojiCategoryTypeDefault &>(obj));
      return true;
    case emojiCategoryTypeRegularStickers::ID:
      func(static_cast<emojiCategoryTypeRegularStickers &>(obj));
      return true;
    case emojiCategoryTypeEmojiStatus::ID:
      func(static_cast<emojiCategoryTypeEmojiStatus &>(obj));
      return true;
    case emojiCategoryTypeChatPhoto::ID:
      func(static_cast<emojiCategoryTypeChatPhoto &>(obj));
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
bool downcast_call(FileType &obj, const T &func) {
  switch (obj.get_id()) {
    case fileTypeNone::ID:
      func(static_cast<fileTypeNone &>(obj));
      return true;
    case fileTypeAnimation::ID:
      func(static_cast<fileTypeAnimation &>(obj));
      return true;
    case fileTypeAudio::ID:
      func(static_cast<fileTypeAudio &>(obj));
      return true;
    case fileTypeDocument::ID:
      func(static_cast<fileTypeDocument &>(obj));
      return true;
    case fileTypeNotificationSound::ID:
      func(static_cast<fileTypeNotificationSound &>(obj));
      return true;
    case fileTypePhoto::ID:
      func(static_cast<fileTypePhoto &>(obj));
      return true;
    case fileTypePhotoStory::ID:
      func(static_cast<fileTypePhotoStory &>(obj));
      return true;
    case fileTypeProfilePhoto::ID:
      func(static_cast<fileTypeProfilePhoto &>(obj));
      return true;
    case fileTypeSecret::ID:
      func(static_cast<fileTypeSecret &>(obj));
      return true;
    case fileTypeSecretThumbnail::ID:
      func(static_cast<fileTypeSecretThumbnail &>(obj));
      return true;
    case fileTypeSecure::ID:
      func(static_cast<fileTypeSecure &>(obj));
      return true;
    case fileTypeSticker::ID:
      func(static_cast<fileTypeSticker &>(obj));
      return true;
    case fileTypeThumbnail::ID:
      func(static_cast<fileTypeThumbnail &>(obj));
      return true;
    case fileTypeUnknown::ID:
      func(static_cast<fileTypeUnknown &>(obj));
      return true;
    case fileTypeVideo::ID:
      func(static_cast<fileTypeVideo &>(obj));
      return true;
    case fileTypeVideoNote::ID:
      func(static_cast<fileTypeVideoNote &>(obj));
      return true;
    case fileTypeVideoStory::ID:
      func(static_cast<fileTypeVideoStory &>(obj));
      return true;
    case fileTypeVoiceNote::ID:
      func(static_cast<fileTypeVoiceNote &>(obj));
      return true;
    case fileTypeWallpaper::ID:
      func(static_cast<fileTypeWallpaper &>(obj));
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
bool downcast_call(FirebaseAuthenticationSettings &obj, const T &func) {
  switch (obj.get_id()) {
    case firebaseAuthenticationSettingsAndroid::ID:
      func(static_cast<firebaseAuthenticationSettingsAndroid &>(obj));
      return true;
    case firebaseAuthenticationSettingsIos::ID:
      func(static_cast<firebaseAuthenticationSettingsIos &>(obj));
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
bool downcast_call(FirebaseDeviceVerificationParameters &obj, const T &func) {
  switch (obj.get_id()) {
    case firebaseDeviceVerificationParametersSafetyNet::ID:
      func(static_cast<firebaseDeviceVerificationParametersSafetyNet &>(obj));
      return true;
    case firebaseDeviceVerificationParametersPlayIntegrity::ID:
      func(static_cast<firebaseDeviceVerificationParametersPlayIntegrity &>(obj));
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
bool downcast_call(GiveawayInfo &obj, const T &func) {
  switch (obj.get_id()) {
    case giveawayInfoOngoing::ID:
      func(static_cast<giveawayInfoOngoing &>(obj));
      return true;
    case giveawayInfoCompleted::ID:
      func(static_cast<giveawayInfoCompleted &>(obj));
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
bool downcast_call(GiveawayParticipantStatus &obj, const T &func) {
  switch (obj.get_id()) {
    case giveawayParticipantStatusEligible::ID:
      func(static_cast<giveawayParticipantStatusEligible &>(obj));
      return true;
    case giveawayParticipantStatusParticipating::ID:
      func(static_cast<giveawayParticipantStatusParticipating &>(obj));
      return true;
    case giveawayParticipantStatusAlreadyWasMember::ID:
      func(static_cast<giveawayParticipantStatusAlreadyWasMember &>(obj));
      return true;
    case giveawayParticipantStatusAdministrator::ID:
      func(static_cast<giveawayParticipantStatusAdministrator &>(obj));
      return true;
    case giveawayParticipantStatusDisallowedCountry::ID:
      func(static_cast<giveawayParticipantStatusDisallowedCountry &>(obj));
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
bool downcast_call(GiveawayPrize &obj, const T &func) {
  switch (obj.get_id()) {
    case giveawayPrizePremium::ID:
      func(static_cast<giveawayPrizePremium &>(obj));
      return true;
    case giveawayPrizeStars::ID:
      func(static_cast<giveawayPrizeStars &>(obj));
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
bool downcast_call(GroupCallVideoQuality &obj, const T &func) {
  switch (obj.get_id()) {
    case groupCallVideoQualityThumbnail::ID:
      func(static_cast<groupCallVideoQualityThumbnail &>(obj));
      return true;
    case groupCallVideoQualityMedium::ID:
      func(static_cast<groupCallVideoQualityMedium &>(obj));
      return true;
    case groupCallVideoQualityFull::ID:
      func(static_cast<groupCallVideoQualityFull &>(obj));
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
bool downcast_call(InlineKeyboardButtonType &obj, const T &func) {
  switch (obj.get_id()) {
    case inlineKeyboardButtonTypeUrl::ID:
      func(static_cast<inlineKeyboardButtonTypeUrl &>(obj));
      return true;
    case inlineKeyboardButtonTypeLoginUrl::ID:
      func(static_cast<inlineKeyboardButtonTypeLoginUrl &>(obj));
      return true;
    case inlineKeyboardButtonTypeWebApp::ID:
      func(static_cast<inlineKeyboardButtonTypeWebApp &>(obj));
      return true;
    case inlineKeyboardButtonTypeCallback::ID:
      func(static_cast<inlineKeyboardButtonTypeCallback &>(obj));
      return true;
    case inlineKeyboardButtonTypeCallbackWithPassword::ID:
      func(static_cast<inlineKeyboardButtonTypeCallbackWithPassword &>(obj));
      return true;
    case inlineKeyboardButtonTypeCallbackGame::ID:
      func(static_cast<inlineKeyboardButtonTypeCallbackGame &>(obj));
      return true;
    case inlineKeyboardButtonTypeSwitchInline::ID:
      func(static_cast<inlineKeyboardButtonTypeSwitchInline &>(obj));
      return true;
    case inlineKeyboardButtonTypeBuy::ID:
      func(static_cast<inlineKeyboardButtonTypeBuy &>(obj));
      return true;
    case inlineKeyboardButtonTypeUser::ID:
      func(static_cast<inlineKeyboardButtonTypeUser &>(obj));
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
bool downcast_call(InlineQueryResult &obj, const T &func) {
  switch (obj.get_id()) {
    case inlineQueryResultArticle::ID:
      func(static_cast<inlineQueryResultArticle &>(obj));
      return true;
    case inlineQueryResultContact::ID:
      func(static_cast<inlineQueryResultContact &>(obj));
      return true;
    case inlineQueryResultLocation::ID:
      func(static_cast<inlineQueryResultLocation &>(obj));
      return true;
    case inlineQueryResultVenue::ID:
      func(static_cast<inlineQueryResultVenue &>(obj));
      return true;
    case inlineQueryResultGame::ID:
      func(static_cast<inlineQueryResultGame &>(obj));
      return true;
    case inlineQueryResultAnimation::ID:
      func(static_cast<inlineQueryResultAnimation &>(obj));
      return true;
    case inlineQueryResultAudio::ID:
      func(static_cast<inlineQueryResultAudio &>(obj));
      return true;
    case inlineQueryResultDocument::ID:
      func(static_cast<inlineQueryResultDocument &>(obj));
      return true;
    case inlineQueryResultPhoto::ID:
      func(static_cast<inlineQueryResultPhoto &>(obj));
      return true;
    case inlineQueryResultSticker::ID:
      func(static_cast<inlineQueryResultSticker &>(obj));
      return true;
    case inlineQueryResultVideo::ID:
      func(static_cast<inlineQueryResultVideo &>(obj));
      return true;
    case inlineQueryResultVoiceNote::ID:
      func(static_cast<inlineQueryResultVoiceNote &>(obj));
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
bool downcast_call(InlineQueryResultsButtonType &obj, const T &func) {
  switch (obj.get_id()) {
    case inlineQueryResultsButtonTypeStartBot::ID:
      func(static_cast<inlineQueryResultsButtonTypeStartBot &>(obj));
      return true;
    case inlineQueryResultsButtonTypeWebApp::ID:
      func(static_cast<inlineQueryResultsButtonTypeWebApp &>(obj));
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
bool downcast_call(InputBackground &obj, const T &func) {
  switch (obj.get_id()) {
    case inputBackgroundLocal::ID:
      func(static_cast<inputBackgroundLocal &>(obj));
      return true;
    case inputBackgroundRemote::ID:
      func(static_cast<inputBackgroundRemote &>(obj));
      return true;
    case inputBackgroundPrevious::ID:
      func(static_cast<inputBackgroundPrevious &>(obj));
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
bool downcast_call(InputChatPhoto &obj, const T &func) {
  switch (obj.get_id()) {
    case inputChatPhotoPrevious::ID:
      func(static_cast<inputChatPhotoPrevious &>(obj));
      return true;
    case inputChatPhotoStatic::ID:
      func(static_cast<inputChatPhotoStatic &>(obj));
      return true;
    case inputChatPhotoAnimation::ID:
      func(static_cast<inputChatPhotoAnimation &>(obj));
      return true;
    case inputChatPhotoSticker::ID:
      func(static_cast<inputChatPhotoSticker &>(obj));
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
bool downcast_call(InputCredentials &obj, const T &func) {
  switch (obj.get_id()) {
    case inputCredentialsSaved::ID:
      func(static_cast<inputCredentialsSaved &>(obj));
      return true;
    case inputCredentialsNew::ID:
      func(static_cast<inputCredentialsNew &>(obj));
      return true;
    case inputCredentialsApplePay::ID:
      func(static_cast<inputCredentialsApplePay &>(obj));
      return true;
    case inputCredentialsGooglePay::ID:
      func(static_cast<inputCredentialsGooglePay &>(obj));
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
bool downcast_call(InputFile &obj, const T &func) {
  switch (obj.get_id()) {
    case inputFileId::ID:
      func(static_cast<inputFileId &>(obj));
      return true;
    case inputFileRemote::ID:
      func(static_cast<inputFileRemote &>(obj));
      return true;
    case inputFileLocal::ID:
      func(static_cast<inputFileLocal &>(obj));
      return true;
    case inputFileGenerated::ID:
      func(static_cast<inputFileGenerated &>(obj));
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
bool downcast_call(InputInlineQueryResult &obj, const T &func) {
  switch (obj.get_id()) {
    case inputInlineQueryResultAnimation::ID:
      func(static_cast<inputInlineQueryResultAnimation &>(obj));
      return true;
    case inputInlineQueryResultArticle::ID:
      func(static_cast<inputInlineQueryResultArticle &>(obj));
      return true;
    case inputInlineQueryResultAudio::ID:
      func(static_cast<inputInlineQueryResultAudio &>(obj));
      return true;
    case inputInlineQueryResultContact::ID:
      func(static_cast<inputInlineQueryResultContact &>(obj));
      return true;
    case inputInlineQueryResultDocument::ID:
      func(static_cast<inputInlineQueryResultDocument &>(obj));
      return true;
    case inputInlineQueryResultGame::ID:
      func(static_cast<inputInlineQueryResultGame &>(obj));
      return true;
    case inputInlineQueryResultLocation::ID:
      func(static_cast<inputInlineQueryResultLocation &>(obj));
      return true;
    case inputInlineQueryResultPhoto::ID:
      func(static_cast<inputInlineQueryResultPhoto &>(obj));
      return true;
    case inputInlineQueryResultSticker::ID:
      func(static_cast<inputInlineQueryResultSticker &>(obj));
      return true;
    case inputInlineQueryResultVenue::ID:
      func(static_cast<inputInlineQueryResultVenue &>(obj));
      return true;
    case inputInlineQueryResultVideo::ID:
      func(static_cast<inputInlineQueryResultVideo &>(obj));
      return true;
    case inputInlineQueryResultVoiceNote::ID:
      func(static_cast<inputInlineQueryResultVoiceNote &>(obj));
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
bool downcast_call(InputInvoice &obj, const T &func) {
  switch (obj.get_id()) {
    case inputInvoiceMessage::ID:
      func(static_cast<inputInvoiceMessage &>(obj));
      return true;
    case inputInvoiceName::ID:
      func(static_cast<inputInvoiceName &>(obj));
      return true;
    case inputInvoiceTelegram::ID:
      func(static_cast<inputInvoiceTelegram &>(obj));
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
bool downcast_call(InputMessageContent &obj, const T &func) {
  switch (obj.get_id()) {
    case inputMessageText::ID:
      func(static_cast<inputMessageText &>(obj));
      return true;
    case inputMessageAnimation::ID:
      func(static_cast<inputMessageAnimation &>(obj));
      return true;
    case inputMessageAudio::ID:
      func(static_cast<inputMessageAudio &>(obj));
      return true;
    case inputMessageDocument::ID:
      func(static_cast<inputMessageDocument &>(obj));
      return true;
    case inputMessagePaidMedia::ID:
      func(static_cast<inputMessagePaidMedia &>(obj));
      return true;
    case inputMessagePhoto::ID:
      func(static_cast<inputMessagePhoto &>(obj));
      return true;
    case inputMessageSticker::ID:
      func(static_cast<inputMessageSticker &>(obj));
      return true;
    case inputMessageVideo::ID:
      func(static_cast<inputMessageVideo &>(obj));
      return true;
    case inputMessageVideoNote::ID:
      func(static_cast<inputMessageVideoNote &>(obj));
      return true;
    case inputMessageVoiceNote::ID:
      func(static_cast<inputMessageVoiceNote &>(obj));
      return true;
    case inputMessageLocation::ID:
      func(static_cast<inputMessageLocation &>(obj));
      return true;
    case inputMessageVenue::ID:
      func(static_cast<inputMessageVenue &>(obj));
      return true;
    case inputMessageContact::ID:
      func(static_cast<inputMessageContact &>(obj));
      return true;
    case inputMessageDice::ID:
      func(static_cast<inputMessageDice &>(obj));
      return true;
    case inputMessageGame::ID:
      func(static_cast<inputMessageGame &>(obj));
      return true;
    case inputMessageInvoice::ID:
      func(static_cast<inputMessageInvoice &>(obj));
      return true;
    case inputMessagePoll::ID:
      func(static_cast<inputMessagePoll &>(obj));
      return true;
    case inputMessageStory::ID:
      func(static_cast<inputMessageStory &>(obj));
      return true;
    case inputMessageForwarded::ID:
      func(static_cast<inputMessageForwarded &>(obj));
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
bool downcast_call(InputMessageReplyTo &obj, const T &func) {
  switch (obj.get_id()) {
    case inputMessageReplyToMessage::ID:
      func(static_cast<inputMessageReplyToMessage &>(obj));
      return true;
    case inputMessageReplyToExternalMessage::ID:
      func(static_cast<inputMessageReplyToExternalMessage &>(obj));
      return true;
    case inputMessageReplyToStory::ID:
      func(static_cast<inputMessageReplyToStory &>(obj));
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
bool downcast_call(InputPaidMediaType &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPaidMediaTypePhoto::ID:
      func(static_cast<inputPaidMediaTypePhoto &>(obj));
      return true;
    case inputPaidMediaTypeVideo::ID:
      func(static_cast<inputPaidMediaTypeVideo &>(obj));
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
bool downcast_call(InputPassportElement &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPassportElementPersonalDetails::ID:
      func(static_cast<inputPassportElementPersonalDetails &>(obj));
      return true;
    case inputPassportElementPassport::ID:
      func(static_cast<inputPassportElementPassport &>(obj));
      return true;
    case inputPassportElementDriverLicense::ID:
      func(static_cast<inputPassportElementDriverLicense &>(obj));
      return true;
    case inputPassportElementIdentityCard::ID:
      func(static_cast<inputPassportElementIdentityCard &>(obj));
      return true;
    case inputPassportElementInternalPassport::ID:
      func(static_cast<inputPassportElementInternalPassport &>(obj));
      return true;
    case inputPassportElementAddress::ID:
      func(static_cast<inputPassportElementAddress &>(obj));
      return true;
    case inputPassportElementUtilityBill::ID:
      func(static_cast<inputPassportElementUtilityBill &>(obj));
      return true;
    case inputPassportElementBankStatement::ID:
      func(static_cast<inputPassportElementBankStatement &>(obj));
      return true;
    case inputPassportElementRentalAgreement::ID:
      func(static_cast<inputPassportElementRentalAgreement &>(obj));
      return true;
    case inputPassportElementPassportRegistration::ID:
      func(static_cast<inputPassportElementPassportRegistration &>(obj));
      return true;
    case inputPassportElementTemporaryRegistration::ID:
      func(static_cast<inputPassportElementTemporaryRegistration &>(obj));
      return true;
    case inputPassportElementPhoneNumber::ID:
      func(static_cast<inputPassportElementPhoneNumber &>(obj));
      return true;
    case inputPassportElementEmailAddress::ID:
      func(static_cast<inputPassportElementEmailAddress &>(obj));
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
bool downcast_call(InputPassportElementErrorSource &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPassportElementErrorSourceUnspecified::ID:
      func(static_cast<inputPassportElementErrorSourceUnspecified &>(obj));
      return true;
    case inputPassportElementErrorSourceDataField::ID:
      func(static_cast<inputPassportElementErrorSourceDataField &>(obj));
      return true;
    case inputPassportElementErrorSourceFrontSide::ID:
      func(static_cast<inputPassportElementErrorSourceFrontSide &>(obj));
      return true;
    case inputPassportElementErrorSourceReverseSide::ID:
      func(static_cast<inputPassportElementErrorSourceReverseSide &>(obj));
      return true;
    case inputPassportElementErrorSourceSelfie::ID:
      func(static_cast<inputPassportElementErrorSourceSelfie &>(obj));
      return true;
    case inputPassportElementErrorSourceTranslationFile::ID:
      func(static_cast<inputPassportElementErrorSourceTranslationFile &>(obj));
      return true;
    case inputPassportElementErrorSourceTranslationFiles::ID:
      func(static_cast<inputPassportElementErrorSourceTranslationFiles &>(obj));
      return true;
    case inputPassportElementErrorSourceFile::ID:
      func(static_cast<inputPassportElementErrorSourceFile &>(obj));
      return true;
    case inputPassportElementErrorSourceFiles::ID:
      func(static_cast<inputPassportElementErrorSourceFiles &>(obj));
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
bool downcast_call(InputStoryAreaType &obj, const T &func) {
  switch (obj.get_id()) {
    case inputStoryAreaTypeLocation::ID:
      func(static_cast<inputStoryAreaTypeLocation &>(obj));
      return true;
    case inputStoryAreaTypeFoundVenue::ID:
      func(static_cast<inputStoryAreaTypeFoundVenue &>(obj));
      return true;
    case inputStoryAreaTypePreviousVenue::ID:
      func(static_cast<inputStoryAreaTypePreviousVenue &>(obj));
      return true;
    case inputStoryAreaTypeSuggestedReaction::ID:
      func(static_cast<inputStoryAreaTypeSuggestedReaction &>(obj));
      return true;
    case inputStoryAreaTypeMessage::ID:
      func(static_cast<inputStoryAreaTypeMessage &>(obj));
      return true;
    case inputStoryAreaTypeLink::ID:
      func(static_cast<inputStoryAreaTypeLink &>(obj));
      return true;
    case inputStoryAreaTypeWeather::ID:
      func(static_cast<inputStoryAreaTypeWeather &>(obj));
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
bool downcast_call(InputStoryContent &obj, const T &func) {
  switch (obj.get_id()) {
    case inputStoryContentPhoto::ID:
      func(static_cast<inputStoryContentPhoto &>(obj));
      return true;
    case inputStoryContentVideo::ID:
      func(static_cast<inputStoryContentVideo &>(obj));
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
bool downcast_call(InternalLinkType &obj, const T &func) {
  switch (obj.get_id()) {
    case internalLinkTypeActiveSessions::ID:
      func(static_cast<internalLinkTypeActiveSessions &>(obj));
      return true;
    case internalLinkTypeAttachmentMenuBot::ID:
      func(static_cast<internalLinkTypeAttachmentMenuBot &>(obj));
      return true;
    case internalLinkTypeAuthenticationCode::ID:
      func(static_cast<internalLinkTypeAuthenticationCode &>(obj));
      return true;
    case internalLinkTypeBackground::ID:
      func(static_cast<internalLinkTypeBackground &>(obj));
      return true;
    case internalLinkTypeBotAddToChannel::ID:
      func(static_cast<internalLinkTypeBotAddToChannel &>(obj));
      return true;
    case internalLinkTypeBotStart::ID:
      func(static_cast<internalLinkTypeBotStart &>(obj));
      return true;
    case internalLinkTypeBotStartInGroup::ID:
      func(static_cast<internalLinkTypeBotStartInGroup &>(obj));
      return true;
    case internalLinkTypeBusinessChat::ID:
      func(static_cast<internalLinkTypeBusinessChat &>(obj));
      return true;
    case internalLinkTypeBuyStars::ID:
      func(static_cast<internalLinkTypeBuyStars &>(obj));
      return true;
    case internalLinkTypeChangePhoneNumber::ID:
      func(static_cast<internalLinkTypeChangePhoneNumber &>(obj));
      return true;
    case internalLinkTypeChatBoost::ID:
      func(static_cast<internalLinkTypeChatBoost &>(obj));
      return true;
    case internalLinkTypeChatFolderInvite::ID:
      func(static_cast<internalLinkTypeChatFolderInvite &>(obj));
      return true;
    case internalLinkTypeChatFolderSettings::ID:
      func(static_cast<internalLinkTypeChatFolderSettings &>(obj));
      return true;
    case internalLinkTypeChatInvite::ID:
      func(static_cast<internalLinkTypeChatInvite &>(obj));
      return true;
    case internalLinkTypeDefaultMessageAutoDeleteTimerSettings::ID:
      func(static_cast<internalLinkTypeDefaultMessageAutoDeleteTimerSettings &>(obj));
      return true;
    case internalLinkTypeEditProfileSettings::ID:
      func(static_cast<internalLinkTypeEditProfileSettings &>(obj));
      return true;
    case internalLinkTypeGame::ID:
      func(static_cast<internalLinkTypeGame &>(obj));
      return true;
    case internalLinkTypeInstantView::ID:
      func(static_cast<internalLinkTypeInstantView &>(obj));
      return true;
    case internalLinkTypeInvoice::ID:
      func(static_cast<internalLinkTypeInvoice &>(obj));
      return true;
    case internalLinkTypeLanguagePack::ID:
      func(static_cast<internalLinkTypeLanguagePack &>(obj));
      return true;
    case internalLinkTypeLanguageSettings::ID:
      func(static_cast<internalLinkTypeLanguageSettings &>(obj));
      return true;
    case internalLinkTypeMainWebApp::ID:
      func(static_cast<internalLinkTypeMainWebApp &>(obj));
      return true;
    case internalLinkTypeMessage::ID:
      func(static_cast<internalLinkTypeMessage &>(obj));
      return true;
    case internalLinkTypeMessageDraft::ID:
      func(static_cast<internalLinkTypeMessageDraft &>(obj));
      return true;
    case internalLinkTypePassportDataRequest::ID:
      func(static_cast<internalLinkTypePassportDataRequest &>(obj));
      return true;
    case internalLinkTypePhoneNumberConfirmation::ID:
      func(static_cast<internalLinkTypePhoneNumberConfirmation &>(obj));
      return true;
    case internalLinkTypePremiumFeatures::ID:
      func(static_cast<internalLinkTypePremiumFeatures &>(obj));
      return true;
    case internalLinkTypePremiumGift::ID:
      func(static_cast<internalLinkTypePremiumGift &>(obj));
      return true;
    case internalLinkTypePremiumGiftCode::ID:
      func(static_cast<internalLinkTypePremiumGiftCode &>(obj));
      return true;
    case internalLinkTypePrivacyAndSecuritySettings::ID:
      func(static_cast<internalLinkTypePrivacyAndSecuritySettings &>(obj));
      return true;
    case internalLinkTypeProxy::ID:
      func(static_cast<internalLinkTypeProxy &>(obj));
      return true;
    case internalLinkTypePublicChat::ID:
      func(static_cast<internalLinkTypePublicChat &>(obj));
      return true;
    case internalLinkTypeQrCodeAuthentication::ID:
      func(static_cast<internalLinkTypeQrCodeAuthentication &>(obj));
      return true;
    case internalLinkTypeRestorePurchases::ID:
      func(static_cast<internalLinkTypeRestorePurchases &>(obj));
      return true;
    case internalLinkTypeSettings::ID:
      func(static_cast<internalLinkTypeSettings &>(obj));
      return true;
    case internalLinkTypeStickerSet::ID:
      func(static_cast<internalLinkTypeStickerSet &>(obj));
      return true;
    case internalLinkTypeStory::ID:
      func(static_cast<internalLinkTypeStory &>(obj));
      return true;
    case internalLinkTypeTheme::ID:
      func(static_cast<internalLinkTypeTheme &>(obj));
      return true;
    case internalLinkTypeThemeSettings::ID:
      func(static_cast<internalLinkTypeThemeSettings &>(obj));
      return true;
    case internalLinkTypeUnknownDeepLink::ID:
      func(static_cast<internalLinkTypeUnknownDeepLink &>(obj));
      return true;
    case internalLinkTypeUnsupportedProxy::ID:
      func(static_cast<internalLinkTypeUnsupportedProxy &>(obj));
      return true;
    case internalLinkTypeUserPhoneNumber::ID:
      func(static_cast<internalLinkTypeUserPhoneNumber &>(obj));
      return true;
    case internalLinkTypeUserToken::ID:
      func(static_cast<internalLinkTypeUserToken &>(obj));
      return true;
    case internalLinkTypeVideoChat::ID:
      func(static_cast<internalLinkTypeVideoChat &>(obj));
      return true;
    case internalLinkTypeWebApp::ID:
      func(static_cast<internalLinkTypeWebApp &>(obj));
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
bool downcast_call(InviteLinkChatType &obj, const T &func) {
  switch (obj.get_id()) {
    case inviteLinkChatTypeBasicGroup::ID:
      func(static_cast<inviteLinkChatTypeBasicGroup &>(obj));
      return true;
    case inviteLinkChatTypeSupergroup::ID:
      func(static_cast<inviteLinkChatTypeSupergroup &>(obj));
      return true;
    case inviteLinkChatTypeChannel::ID:
      func(static_cast<inviteLinkChatTypeChannel &>(obj));
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
bool downcast_call(JsonValue &obj, const T &func) {
  switch (obj.get_id()) {
    case jsonValueNull::ID:
      func(static_cast<jsonValueNull &>(obj));
      return true;
    case jsonValueBoolean::ID:
      func(static_cast<jsonValueBoolean &>(obj));
      return true;
    case jsonValueNumber::ID:
      func(static_cast<jsonValueNumber &>(obj));
      return true;
    case jsonValueString::ID:
      func(static_cast<jsonValueString &>(obj));
      return true;
    case jsonValueArray::ID:
      func(static_cast<jsonValueArray &>(obj));
      return true;
    case jsonValueObject::ID:
      func(static_cast<jsonValueObject &>(obj));
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
bool downcast_call(KeyboardButtonType &obj, const T &func) {
  switch (obj.get_id()) {
    case keyboardButtonTypeText::ID:
      func(static_cast<keyboardButtonTypeText &>(obj));
      return true;
    case keyboardButtonTypeRequestPhoneNumber::ID:
      func(static_cast<keyboardButtonTypeRequestPhoneNumber &>(obj));
      return true;
    case keyboardButtonTypeRequestLocation::ID:
      func(static_cast<keyboardButtonTypeRequestLocation &>(obj));
      return true;
    case keyboardButtonTypeRequestPoll::ID:
      func(static_cast<keyboardButtonTypeRequestPoll &>(obj));
      return true;
    case keyboardButtonTypeRequestUsers::ID:
      func(static_cast<keyboardButtonTypeRequestUsers &>(obj));
      return true;
    case keyboardButtonTypeRequestChat::ID:
      func(static_cast<keyboardButtonTypeRequestChat &>(obj));
      return true;
    case keyboardButtonTypeWebApp::ID:
      func(static_cast<keyboardButtonTypeWebApp &>(obj));
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
bool downcast_call(LanguagePackStringValue &obj, const T &func) {
  switch (obj.get_id()) {
    case languagePackStringValueOrdinary::ID:
      func(static_cast<languagePackStringValueOrdinary &>(obj));
      return true;
    case languagePackStringValuePluralized::ID:
      func(static_cast<languagePackStringValuePluralized &>(obj));
      return true;
    case languagePackStringValueDeleted::ID:
      func(static_cast<languagePackStringValueDeleted &>(obj));
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
bool downcast_call(LinkPreviewAlbumMedia &obj, const T &func) {
  switch (obj.get_id()) {
    case linkPreviewAlbumMediaPhoto::ID:
      func(static_cast<linkPreviewAlbumMediaPhoto &>(obj));
      return true;
    case linkPreviewAlbumMediaVideo::ID:
      func(static_cast<linkPreviewAlbumMediaVideo &>(obj));
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
bool downcast_call(LinkPreviewType &obj, const T &func) {
  switch (obj.get_id()) {
    case linkPreviewTypeAlbum::ID:
      func(static_cast<linkPreviewTypeAlbum &>(obj));
      return true;
    case linkPreviewTypeAnimation::ID:
      func(static_cast<linkPreviewTypeAnimation &>(obj));
      return true;
    case linkPreviewTypeApp::ID:
      func(static_cast<linkPreviewTypeApp &>(obj));
      return true;
    case linkPreviewTypeArticle::ID:
      func(static_cast<linkPreviewTypeArticle &>(obj));
      return true;
    case linkPreviewTypeAudio::ID:
      func(static_cast<linkPreviewTypeAudio &>(obj));
      return true;
    case linkPreviewTypeBackground::ID:
      func(static_cast<linkPreviewTypeBackground &>(obj));
      return true;
    case linkPreviewTypeChannelBoost::ID:
      func(static_cast<linkPreviewTypeChannelBoost &>(obj));
      return true;
    case linkPreviewTypeChat::ID:
      func(static_cast<linkPreviewTypeChat &>(obj));
      return true;
    case linkPreviewTypeDocument::ID:
      func(static_cast<linkPreviewTypeDocument &>(obj));
      return true;
    case linkPreviewTypeEmbeddedAnimationPlayer::ID:
      func(static_cast<linkPreviewTypeEmbeddedAnimationPlayer &>(obj));
      return true;
    case linkPreviewTypeEmbeddedAudioPlayer::ID:
      func(static_cast<linkPreviewTypeEmbeddedAudioPlayer &>(obj));
      return true;
    case linkPreviewTypeEmbeddedVideoPlayer::ID:
      func(static_cast<linkPreviewTypeEmbeddedVideoPlayer &>(obj));
      return true;
    case linkPreviewTypeExternalAudio::ID:
      func(static_cast<linkPreviewTypeExternalAudio &>(obj));
      return true;
    case linkPreviewTypeExternalVideo::ID:
      func(static_cast<linkPreviewTypeExternalVideo &>(obj));
      return true;
    case linkPreviewTypeInvoice::ID:
      func(static_cast<linkPreviewTypeInvoice &>(obj));
      return true;
    case linkPreviewTypeMessage::ID:
      func(static_cast<linkPreviewTypeMessage &>(obj));
      return true;
    case linkPreviewTypePhoto::ID:
      func(static_cast<linkPreviewTypePhoto &>(obj));
      return true;
    case linkPreviewTypePremiumGiftCode::ID:
      func(static_cast<linkPreviewTypePremiumGiftCode &>(obj));
      return true;
    case linkPreviewTypeShareableChatFolder::ID:
      func(static_cast<linkPreviewTypeShareableChatFolder &>(obj));
      return true;
    case linkPreviewTypeSticker::ID:
      func(static_cast<linkPreviewTypeSticker &>(obj));
      return true;
    case linkPreviewTypeStickerSet::ID:
      func(static_cast<linkPreviewTypeStickerSet &>(obj));
      return true;
    case linkPreviewTypeStory::ID:
      func(static_cast<linkPreviewTypeStory &>(obj));
      return true;
    case linkPreviewTypeSupergroupBoost::ID:
      func(static_cast<linkPreviewTypeSupergroupBoost &>(obj));
      return true;
    case linkPreviewTypeTheme::ID:
      func(static_cast<linkPreviewTypeTheme &>(obj));
      return true;
    case linkPreviewTypeUnsupported::ID:
      func(static_cast<linkPreviewTypeUnsupported &>(obj));
      return true;
    case linkPreviewTypeUser::ID:
      func(static_cast<linkPreviewTypeUser &>(obj));
      return true;
    case linkPreviewTypeVideo::ID:
      func(static_cast<linkPreviewTypeVideo &>(obj));
      return true;
    case linkPreviewTypeVideoChat::ID:
      func(static_cast<linkPreviewTypeVideoChat &>(obj));
      return true;
    case linkPreviewTypeVideoNote::ID:
      func(static_cast<linkPreviewTypeVideoNote &>(obj));
      return true;
    case linkPreviewTypeVoiceNote::ID:
      func(static_cast<linkPreviewTypeVoiceNote &>(obj));
      return true;
    case linkPreviewTypeWebApp::ID:
      func(static_cast<linkPreviewTypeWebApp &>(obj));
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
bool downcast_call(LogStream &obj, const T &func) {
  switch (obj.get_id()) {
    case logStreamDefault::ID:
      func(static_cast<logStreamDefault &>(obj));
      return true;
    case logStreamFile::ID:
      func(static_cast<logStreamFile &>(obj));
      return true;
    case logStreamEmpty::ID:
      func(static_cast<logStreamEmpty &>(obj));
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
bool downcast_call(LoginUrlInfo &obj, const T &func) {
  switch (obj.get_id()) {
    case loginUrlInfoOpen::ID:
      func(static_cast<loginUrlInfoOpen &>(obj));
      return true;
    case loginUrlInfoRequestConfirmation::ID:
      func(static_cast<loginUrlInfoRequestConfirmation &>(obj));
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
bool downcast_call(MaskPoint &obj, const T &func) {
  switch (obj.get_id()) {
    case maskPointForehead::ID:
      func(static_cast<maskPointForehead &>(obj));
      return true;
    case maskPointEyes::ID:
      func(static_cast<maskPointEyes &>(obj));
      return true;
    case maskPointMouth::ID:
      func(static_cast<maskPointMouth &>(obj));
      return true;
    case maskPointChin::ID:
      func(static_cast<maskPointChin &>(obj));
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
bool downcast_call(MessageContent &obj, const T &func) {
  switch (obj.get_id()) {
    case messageText::ID:
      func(static_cast<messageText &>(obj));
      return true;
    case messageAnimation::ID:
      func(static_cast<messageAnimation &>(obj));
      return true;
    case messageAudio::ID:
      func(static_cast<messageAudio &>(obj));
      return true;
    case messageDocument::ID:
      func(static_cast<messageDocument &>(obj));
      return true;
    case messagePaidMedia::ID:
      func(static_cast<messagePaidMedia &>(obj));
      return true;
    case messagePhoto::ID:
      func(static_cast<messagePhoto &>(obj));
      return true;
    case messageSticker::ID:
      func(static_cast<messageSticker &>(obj));
      return true;
    case messageVideo::ID:
      func(static_cast<messageVideo &>(obj));
      return true;
    case messageVideoNote::ID:
      func(static_cast<messageVideoNote &>(obj));
      return true;
    case messageVoiceNote::ID:
      func(static_cast<messageVoiceNote &>(obj));
      return true;
    case messageExpiredPhoto::ID:
      func(static_cast<messageExpiredPhoto &>(obj));
      return true;
    case messageExpiredVideo::ID:
      func(static_cast<messageExpiredVideo &>(obj));
      return true;
    case messageExpiredVideoNote::ID:
      func(static_cast<messageExpiredVideoNote &>(obj));
      return true;
    case messageExpiredVoiceNote::ID:
      func(static_cast<messageExpiredVoiceNote &>(obj));
      return true;
    case messageLocation::ID:
      func(static_cast<messageLocation &>(obj));
      return true;
    case messageVenue::ID:
      func(static_cast<messageVenue &>(obj));
      return true;
    case messageContact::ID:
      func(static_cast<messageContact &>(obj));
      return true;
    case messageAnimatedEmoji::ID:
      func(static_cast<messageAnimatedEmoji &>(obj));
      return true;
    case messageDice::ID:
      func(static_cast<messageDice &>(obj));
      return true;
    case messageGame::ID:
      func(static_cast<messageGame &>(obj));
      return true;
    case messagePoll::ID:
      func(static_cast<messagePoll &>(obj));
      return true;
    case messageStory::ID:
      func(static_cast<messageStory &>(obj));
      return true;
    case messageInvoice::ID:
      func(static_cast<messageInvoice &>(obj));
      return true;
    case messageCall::ID:
      func(static_cast<messageCall &>(obj));
      return true;
    case messageVideoChatScheduled::ID:
      func(static_cast<messageVideoChatScheduled &>(obj));
      return true;
    case messageVideoChatStarted::ID:
      func(static_cast<messageVideoChatStarted &>(obj));
      return true;
    case messageVideoChatEnded::ID:
      func(static_cast<messageVideoChatEnded &>(obj));
      return true;
    case messageInviteVideoChatParticipants::ID:
      func(static_cast<messageInviteVideoChatParticipants &>(obj));
      return true;
    case messageBasicGroupChatCreate::ID:
      func(static_cast<messageBasicGroupChatCreate &>(obj));
      return true;
    case messageSupergroupChatCreate::ID:
      func(static_cast<messageSupergroupChatCreate &>(obj));
      return true;
    case messageChatChangeTitle::ID:
      func(static_cast<messageChatChangeTitle &>(obj));
      return true;
    case messageChatChangePhoto::ID:
      func(static_cast<messageChatChangePhoto &>(obj));
      return true;
    case messageChatDeletePhoto::ID:
      func(static_cast<messageChatDeletePhoto &>(obj));
      return true;
    case messageChatAddMembers::ID:
      func(static_cast<messageChatAddMembers &>(obj));
      return true;
    case messageChatJoinByLink::ID:
      func(static_cast<messageChatJoinByLink &>(obj));
      return true;
    case messageChatJoinByRequest::ID:
      func(static_cast<messageChatJoinByRequest &>(obj));
      return true;
    case messageChatDeleteMember::ID:
      func(static_cast<messageChatDeleteMember &>(obj));
      return true;
    case messageChatUpgradeTo::ID:
      func(static_cast<messageChatUpgradeTo &>(obj));
      return true;
    case messageChatUpgradeFrom::ID:
      func(static_cast<messageChatUpgradeFrom &>(obj));
      return true;
    case messagePinMessage::ID:
      func(static_cast<messagePinMessage &>(obj));
      return true;
    case messageScreenshotTaken::ID:
      func(static_cast<messageScreenshotTaken &>(obj));
      return true;
    case messageChatSetBackground::ID:
      func(static_cast<messageChatSetBackground &>(obj));
      return true;
    case messageChatSetTheme::ID:
      func(static_cast<messageChatSetTheme &>(obj));
      return true;
    case messageChatSetMessageAutoDeleteTime::ID:
      func(static_cast<messageChatSetMessageAutoDeleteTime &>(obj));
      return true;
    case messageChatBoost::ID:
      func(static_cast<messageChatBoost &>(obj));
      return true;
    case messageForumTopicCreated::ID:
      func(static_cast<messageForumTopicCreated &>(obj));
      return true;
    case messageForumTopicEdited::ID:
      func(static_cast<messageForumTopicEdited &>(obj));
      return true;
    case messageForumTopicIsClosedToggled::ID:
      func(static_cast<messageForumTopicIsClosedToggled &>(obj));
      return true;
    case messageForumTopicIsHiddenToggled::ID:
      func(static_cast<messageForumTopicIsHiddenToggled &>(obj));
      return true;
    case messageSuggestProfilePhoto::ID:
      func(static_cast<messageSuggestProfilePhoto &>(obj));
      return true;
    case messageCustomServiceAction::ID:
      func(static_cast<messageCustomServiceAction &>(obj));
      return true;
    case messageGameScore::ID:
      func(static_cast<messageGameScore &>(obj));
      return true;
    case messagePaymentSuccessful::ID:
      func(static_cast<messagePaymentSuccessful &>(obj));
      return true;
    case messagePaymentSuccessfulBot::ID:
      func(static_cast<messagePaymentSuccessfulBot &>(obj));
      return true;
    case messagePaymentRefunded::ID:
      func(static_cast<messagePaymentRefunded &>(obj));
      return true;
    case messageGiftedPremium::ID:
      func(static_cast<messageGiftedPremium &>(obj));
      return true;
    case messagePremiumGiftCode::ID:
      func(static_cast<messagePremiumGiftCode &>(obj));
      return true;
    case messageGiveawayCreated::ID:
      func(static_cast<messageGiveawayCreated &>(obj));
      return true;
    case messageGiveaway::ID:
      func(static_cast<messageGiveaway &>(obj));
      return true;
    case messageGiveawayCompleted::ID:
      func(static_cast<messageGiveawayCompleted &>(obj));
      return true;
    case messageGiveawayWinners::ID:
      func(static_cast<messageGiveawayWinners &>(obj));
      return true;
    case messageGiftedStars::ID:
      func(static_cast<messageGiftedStars &>(obj));
      return true;
    case messageGiveawayPrizeStars::ID:
      func(static_cast<messageGiveawayPrizeStars &>(obj));
      return true;
    case messageContactRegistered::ID:
      func(static_cast<messageContactRegistered &>(obj));
      return true;
    case messageUsersShared::ID:
      func(static_cast<messageUsersShared &>(obj));
      return true;
    case messageChatShared::ID:
      func(static_cast<messageChatShared &>(obj));
      return true;
    case messageBotWriteAccessAllowed::ID:
      func(static_cast<messageBotWriteAccessAllowed &>(obj));
      return true;
    case messageWebAppDataSent::ID:
      func(static_cast<messageWebAppDataSent &>(obj));
      return true;
    case messageWebAppDataReceived::ID:
      func(static_cast<messageWebAppDataReceived &>(obj));
      return true;
    case messagePassportDataSent::ID:
      func(static_cast<messagePassportDataSent &>(obj));
      return true;
    case messagePassportDataReceived::ID:
      func(static_cast<messagePassportDataReceived &>(obj));
      return true;
    case messageProximityAlertTriggered::ID:
      func(static_cast<messageProximityAlertTriggered &>(obj));
      return true;
    case messageUnsupported::ID:
      func(static_cast<messageUnsupported &>(obj));
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
bool downcast_call(MessageEffectType &obj, const T &func) {
  switch (obj.get_id()) {
    case messageEffectTypeEmojiReaction::ID:
      func(static_cast<messageEffectTypeEmojiReaction &>(obj));
      return true;
    case messageEffectTypePremiumSticker::ID:
      func(static_cast<messageEffectTypePremiumSticker &>(obj));
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
bool downcast_call(MessageFileType &obj, const T &func) {
  switch (obj.get_id()) {
    case messageFileTypePrivate::ID:
      func(static_cast<messageFileTypePrivate &>(obj));
      return true;
    case messageFileTypeGroup::ID:
      func(static_cast<messageFileTypeGroup &>(obj));
      return true;
    case messageFileTypeUnknown::ID:
      func(static_cast<messageFileTypeUnknown &>(obj));
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
bool downcast_call(MessageOrigin &obj, const T &func) {
  switch (obj.get_id()) {
    case messageOriginUser::ID:
      func(static_cast<messageOriginUser &>(obj));
      return true;
    case messageOriginHiddenUser::ID:
      func(static_cast<messageOriginHiddenUser &>(obj));
      return true;
    case messageOriginChat::ID:
      func(static_cast<messageOriginChat &>(obj));
      return true;
    case messageOriginChannel::ID:
      func(static_cast<messageOriginChannel &>(obj));
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
bool downcast_call(MessageReadDate &obj, const T &func) {
  switch (obj.get_id()) {
    case messageReadDateRead::ID:
      func(static_cast<messageReadDateRead &>(obj));
      return true;
    case messageReadDateUnread::ID:
      func(static_cast<messageReadDateUnread &>(obj));
      return true;
    case messageReadDateTooOld::ID:
      func(static_cast<messageReadDateTooOld &>(obj));
      return true;
    case messageReadDateUserPrivacyRestricted::ID:
      func(static_cast<messageReadDateUserPrivacyRestricted &>(obj));
      return true;
    case messageReadDateMyPrivacyRestricted::ID:
      func(static_cast<messageReadDateMyPrivacyRestricted &>(obj));
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
bool downcast_call(MessageReplyTo &obj, const T &func) {
  switch (obj.get_id()) {
    case messageReplyToMessage::ID:
      func(static_cast<messageReplyToMessage &>(obj));
      return true;
    case messageReplyToStory::ID:
      func(static_cast<messageReplyToStory &>(obj));
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
bool downcast_call(MessageSchedulingState &obj, const T &func) {
  switch (obj.get_id()) {
    case messageSchedulingStateSendAtDate::ID:
      func(static_cast<messageSchedulingStateSendAtDate &>(obj));
      return true;
    case messageSchedulingStateSendWhenOnline::ID:
      func(static_cast<messageSchedulingStateSendWhenOnline &>(obj));
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
bool downcast_call(MessageSelfDestructType &obj, const T &func) {
  switch (obj.get_id()) {
    case messageSelfDestructTypeTimer::ID:
      func(static_cast<messageSelfDestructTypeTimer &>(obj));
      return true;
    case messageSelfDestructTypeImmediately::ID:
      func(static_cast<messageSelfDestructTypeImmediately &>(obj));
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
bool downcast_call(MessageSender &obj, const T &func) {
  switch (obj.get_id()) {
    case messageSenderUser::ID:
      func(static_cast<messageSenderUser &>(obj));
      return true;
    case messageSenderChat::ID:
      func(static_cast<messageSenderChat &>(obj));
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
bool downcast_call(MessageSendingState &obj, const T &func) {
  switch (obj.get_id()) {
    case messageSendingStatePending::ID:
      func(static_cast<messageSendingStatePending &>(obj));
      return true;
    case messageSendingStateFailed::ID:
      func(static_cast<messageSendingStateFailed &>(obj));
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
bool downcast_call(MessageSource &obj, const T &func) {
  switch (obj.get_id()) {
    case messageSourceChatHistory::ID:
      func(static_cast<messageSourceChatHistory &>(obj));
      return true;
    case messageSourceMessageThreadHistory::ID:
      func(static_cast<messageSourceMessageThreadHistory &>(obj));
      return true;
    case messageSourceForumTopicHistory::ID:
      func(static_cast<messageSourceForumTopicHistory &>(obj));
      return true;
    case messageSourceHistoryPreview::ID:
      func(static_cast<messageSourceHistoryPreview &>(obj));
      return true;
    case messageSourceChatList::ID:
      func(static_cast<messageSourceChatList &>(obj));
      return true;
    case messageSourceSearch::ID:
      func(static_cast<messageSourceSearch &>(obj));
      return true;
    case messageSourceChatEventLog::ID:
      func(static_cast<messageSourceChatEventLog &>(obj));
      return true;
    case messageSourceNotification::ID:
      func(static_cast<messageSourceNotification &>(obj));
      return true;
    case messageSourceScreenshot::ID:
      func(static_cast<messageSourceScreenshot &>(obj));
      return true;
    case messageSourceOther::ID:
      func(static_cast<messageSourceOther &>(obj));
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
bool downcast_call(NetworkStatisticsEntry &obj, const T &func) {
  switch (obj.get_id()) {
    case networkStatisticsEntryFile::ID:
      func(static_cast<networkStatisticsEntryFile &>(obj));
      return true;
    case networkStatisticsEntryCall::ID:
      func(static_cast<networkStatisticsEntryCall &>(obj));
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
bool downcast_call(NetworkType &obj, const T &func) {
  switch (obj.get_id()) {
    case networkTypeNone::ID:
      func(static_cast<networkTypeNone &>(obj));
      return true;
    case networkTypeMobile::ID:
      func(static_cast<networkTypeMobile &>(obj));
      return true;
    case networkTypeMobileRoaming::ID:
      func(static_cast<networkTypeMobileRoaming &>(obj));
      return true;
    case networkTypeWiFi::ID:
      func(static_cast<networkTypeWiFi &>(obj));
      return true;
    case networkTypeOther::ID:
      func(static_cast<networkTypeOther &>(obj));
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
bool downcast_call(NotificationGroupType &obj, const T &func) {
  switch (obj.get_id()) {
    case notificationGroupTypeMessages::ID:
      func(static_cast<notificationGroupTypeMessages &>(obj));
      return true;
    case notificationGroupTypeMentions::ID:
      func(static_cast<notificationGroupTypeMentions &>(obj));
      return true;
    case notificationGroupTypeSecretChat::ID:
      func(static_cast<notificationGroupTypeSecretChat &>(obj));
      return true;
    case notificationGroupTypeCalls::ID:
      func(static_cast<notificationGroupTypeCalls &>(obj));
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
bool downcast_call(NotificationSettingsScope &obj, const T &func) {
  switch (obj.get_id()) {
    case notificationSettingsScopePrivateChats::ID:
      func(static_cast<notificationSettingsScopePrivateChats &>(obj));
      return true;
    case notificationSettingsScopeGroupChats::ID:
      func(static_cast<notificationSettingsScopeGroupChats &>(obj));
      return true;
    case notificationSettingsScopeChannelChats::ID:
      func(static_cast<notificationSettingsScopeChannelChats &>(obj));
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
bool downcast_call(NotificationType &obj, const T &func) {
  switch (obj.get_id()) {
    case notificationTypeNewMessage::ID:
      func(static_cast<notificationTypeNewMessage &>(obj));
      return true;
    case notificationTypeNewSecretChat::ID:
      func(static_cast<notificationTypeNewSecretChat &>(obj));
      return true;
    case notificationTypeNewCall::ID:
      func(static_cast<notificationTypeNewCall &>(obj));
      return true;
    case notificationTypeNewPushMessage::ID:
      func(static_cast<notificationTypeNewPushMessage &>(obj));
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
bool downcast_call(OptionValue &obj, const T &func) {
  switch (obj.get_id()) {
    case optionValueBoolean::ID:
      func(static_cast<optionValueBoolean &>(obj));
      return true;
    case optionValueEmpty::ID:
      func(static_cast<optionValueEmpty &>(obj));
      return true;
    case optionValueInteger::ID:
      func(static_cast<optionValueInteger &>(obj));
      return true;
    case optionValueString::ID:
      func(static_cast<optionValueString &>(obj));
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
bool downcast_call(PageBlock &obj, const T &func) {
  switch (obj.get_id()) {
    case pageBlockTitle::ID:
      func(static_cast<pageBlockTitle &>(obj));
      return true;
    case pageBlockSubtitle::ID:
      func(static_cast<pageBlockSubtitle &>(obj));
      return true;
    case pageBlockAuthorDate::ID:
      func(static_cast<pageBlockAuthorDate &>(obj));
      return true;
    case pageBlockHeader::ID:
      func(static_cast<pageBlockHeader &>(obj));
      return true;
    case pageBlockSubheader::ID:
      func(static_cast<pageBlockSubheader &>(obj));
      return true;
    case pageBlockKicker::ID:
      func(static_cast<pageBlockKicker &>(obj));
      return true;
    case pageBlockParagraph::ID:
      func(static_cast<pageBlockParagraph &>(obj));
      return true;
    case pageBlockPreformatted::ID:
      func(static_cast<pageBlockPreformatted &>(obj));
      return true;
    case pageBlockFooter::ID:
      func(static_cast<pageBlockFooter &>(obj));
      return true;
    case pageBlockDivider::ID:
      func(static_cast<pageBlockDivider &>(obj));
      return true;
    case pageBlockAnchor::ID:
      func(static_cast<pageBlockAnchor &>(obj));
      return true;
    case pageBlockList::ID:
      func(static_cast<pageBlockList &>(obj));
      return true;
    case pageBlockBlockQuote::ID:
      func(static_cast<pageBlockBlockQuote &>(obj));
      return true;
    case pageBlockPullQuote::ID:
      func(static_cast<pageBlockPullQuote &>(obj));
      return true;
    case pageBlockAnimation::ID:
      func(static_cast<pageBlockAnimation &>(obj));
      return true;
    case pageBlockAudio::ID:
      func(static_cast<pageBlockAudio &>(obj));
      return true;
    case pageBlockPhoto::ID:
      func(static_cast<pageBlockPhoto &>(obj));
      return true;
    case pageBlockVideo::ID:
      func(static_cast<pageBlockVideo &>(obj));
      return true;
    case pageBlockVoiceNote::ID:
      func(static_cast<pageBlockVoiceNote &>(obj));
      return true;
    case pageBlockCover::ID:
      func(static_cast<pageBlockCover &>(obj));
      return true;
    case pageBlockEmbedded::ID:
      func(static_cast<pageBlockEmbedded &>(obj));
      return true;
    case pageBlockEmbeddedPost::ID:
      func(static_cast<pageBlockEmbeddedPost &>(obj));
      return true;
    case pageBlockCollage::ID:
      func(static_cast<pageBlockCollage &>(obj));
      return true;
    case pageBlockSlideshow::ID:
      func(static_cast<pageBlockSlideshow &>(obj));
      return true;
    case pageBlockChatLink::ID:
      func(static_cast<pageBlockChatLink &>(obj));
      return true;
    case pageBlockTable::ID:
      func(static_cast<pageBlockTable &>(obj));
      return true;
    case pageBlockDetails::ID:
      func(static_cast<pageBlockDetails &>(obj));
      return true;
    case pageBlockRelatedArticles::ID:
      func(static_cast<pageBlockRelatedArticles &>(obj));
      return true;
    case pageBlockMap::ID:
      func(static_cast<pageBlockMap &>(obj));
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
bool downcast_call(PageBlockHorizontalAlignment &obj, const T &func) {
  switch (obj.get_id()) {
    case pageBlockHorizontalAlignmentLeft::ID:
      func(static_cast<pageBlockHorizontalAlignmentLeft &>(obj));
      return true;
    case pageBlockHorizontalAlignmentCenter::ID:
      func(static_cast<pageBlockHorizontalAlignmentCenter &>(obj));
      return true;
    case pageBlockHorizontalAlignmentRight::ID:
      func(static_cast<pageBlockHorizontalAlignmentRight &>(obj));
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
bool downcast_call(PageBlockVerticalAlignment &obj, const T &func) {
  switch (obj.get_id()) {
    case pageBlockVerticalAlignmentTop::ID:
      func(static_cast<pageBlockVerticalAlignmentTop &>(obj));
      return true;
    case pageBlockVerticalAlignmentMiddle::ID:
      func(static_cast<pageBlockVerticalAlignmentMiddle &>(obj));
      return true;
    case pageBlockVerticalAlignmentBottom::ID:
      func(static_cast<pageBlockVerticalAlignmentBottom &>(obj));
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
bool downcast_call(PaidMedia &obj, const T &func) {
  switch (obj.get_id()) {
    case paidMediaPreview::ID:
      func(static_cast<paidMediaPreview &>(obj));
      return true;
    case paidMediaPhoto::ID:
      func(static_cast<paidMediaPhoto &>(obj));
      return true;
    case paidMediaVideo::ID:
      func(static_cast<paidMediaVideo &>(obj));
      return true;
    case paidMediaUnsupported::ID:
      func(static_cast<paidMediaUnsupported &>(obj));
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
bool downcast_call(PassportElement &obj, const T &func) {
  switch (obj.get_id()) {
    case passportElementPersonalDetails::ID:
      func(static_cast<passportElementPersonalDetails &>(obj));
      return true;
    case passportElementPassport::ID:
      func(static_cast<passportElementPassport &>(obj));
      return true;
    case passportElementDriverLicense::ID:
      func(static_cast<passportElementDriverLicense &>(obj));
      return true;
    case passportElementIdentityCard::ID:
      func(static_cast<passportElementIdentityCard &>(obj));
      return true;
    case passportElementInternalPassport::ID:
      func(static_cast<passportElementInternalPassport &>(obj));
      return true;
    case passportElementAddress::ID:
      func(static_cast<passportElementAddress &>(obj));
      return true;
    case passportElementUtilityBill::ID:
      func(static_cast<passportElementUtilityBill &>(obj));
      return true;
    case passportElementBankStatement::ID:
      func(static_cast<passportElementBankStatement &>(obj));
      return true;
    case passportElementRentalAgreement::ID:
      func(static_cast<passportElementRentalAgreement &>(obj));
      return true;
    case passportElementPassportRegistration::ID:
      func(static_cast<passportElementPassportRegistration &>(obj));
      return true;
    case passportElementTemporaryRegistration::ID:
      func(static_cast<passportElementTemporaryRegistration &>(obj));
      return true;
    case passportElementPhoneNumber::ID:
      func(static_cast<passportElementPhoneNumber &>(obj));
      return true;
    case passportElementEmailAddress::ID:
      func(static_cast<passportElementEmailAddress &>(obj));
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
bool downcast_call(PassportElementErrorSource &obj, const T &func) {
  switch (obj.get_id()) {
    case passportElementErrorSourceUnspecified::ID:
      func(static_cast<passportElementErrorSourceUnspecified &>(obj));
      return true;
    case passportElementErrorSourceDataField::ID:
      func(static_cast<passportElementErrorSourceDataField &>(obj));
      return true;
    case passportElementErrorSourceFrontSide::ID:
      func(static_cast<passportElementErrorSourceFrontSide &>(obj));
      return true;
    case passportElementErrorSourceReverseSide::ID:
      func(static_cast<passportElementErrorSourceReverseSide &>(obj));
      return true;
    case passportElementErrorSourceSelfie::ID:
      func(static_cast<passportElementErrorSourceSelfie &>(obj));
      return true;
    case passportElementErrorSourceTranslationFile::ID:
      func(static_cast<passportElementErrorSourceTranslationFile &>(obj));
      return true;
    case passportElementErrorSourceTranslationFiles::ID:
      func(static_cast<passportElementErrorSourceTranslationFiles &>(obj));
      return true;
    case passportElementErrorSourceFile::ID:
      func(static_cast<passportElementErrorSourceFile &>(obj));
      return true;
    case passportElementErrorSourceFiles::ID:
      func(static_cast<passportElementErrorSourceFiles &>(obj));
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
bool downcast_call(PassportElementType &obj, const T &func) {
  switch (obj.get_id()) {
    case passportElementTypePersonalDetails::ID:
      func(static_cast<passportElementTypePersonalDetails &>(obj));
      return true;
    case passportElementTypePassport::ID:
      func(static_cast<passportElementTypePassport &>(obj));
      return true;
    case passportElementTypeDriverLicense::ID:
      func(static_cast<passportElementTypeDriverLicense &>(obj));
      return true;
    case passportElementTypeIdentityCard::ID:
      func(static_cast<passportElementTypeIdentityCard &>(obj));
      return true;
    case passportElementTypeInternalPassport::ID:
      func(static_cast<passportElementTypeInternalPassport &>(obj));
      return true;
    case passportElementTypeAddress::ID:
      func(static_cast<passportElementTypeAddress &>(obj));
      return true;
    case passportElementTypeUtilityBill::ID:
      func(static_cast<passportElementTypeUtilityBill &>(obj));
      return true;
    case passportElementTypeBankStatement::ID:
      func(static_cast<passportElementTypeBankStatement &>(obj));
      return true;
    case passportElementTypeRentalAgreement::ID:
      func(static_cast<passportElementTypeRentalAgreement &>(obj));
      return true;
    case passportElementTypePassportRegistration::ID:
      func(static_cast<passportElementTypePassportRegistration &>(obj));
      return true;
    case passportElementTypeTemporaryRegistration::ID:
      func(static_cast<passportElementTypeTemporaryRegistration &>(obj));
      return true;
    case passportElementTypePhoneNumber::ID:
      func(static_cast<passportElementTypePhoneNumber &>(obj));
      return true;
    case passportElementTypeEmailAddress::ID:
      func(static_cast<passportElementTypeEmailAddress &>(obj));
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
bool downcast_call(PaymentFormType &obj, const T &func) {
  switch (obj.get_id()) {
    case paymentFormTypeRegular::ID:
      func(static_cast<paymentFormTypeRegular &>(obj));
      return true;
    case paymentFormTypeStars::ID:
      func(static_cast<paymentFormTypeStars &>(obj));
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
bool downcast_call(PaymentProvider &obj, const T &func) {
  switch (obj.get_id()) {
    case paymentProviderSmartGlocal::ID:
      func(static_cast<paymentProviderSmartGlocal &>(obj));
      return true;
    case paymentProviderStripe::ID:
      func(static_cast<paymentProviderStripe &>(obj));
      return true;
    case paymentProviderOther::ID:
      func(static_cast<paymentProviderOther &>(obj));
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
bool downcast_call(PaymentReceiptType &obj, const T &func) {
  switch (obj.get_id()) {
    case paymentReceiptTypeRegular::ID:
      func(static_cast<paymentReceiptTypeRegular &>(obj));
      return true;
    case paymentReceiptTypeStars::ID:
      func(static_cast<paymentReceiptTypeStars &>(obj));
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
bool downcast_call(PhoneNumberCodeType &obj, const T &func) {
  switch (obj.get_id()) {
    case phoneNumberCodeTypeChange::ID:
      func(static_cast<phoneNumberCodeTypeChange &>(obj));
      return true;
    case phoneNumberCodeTypeVerify::ID:
      func(static_cast<phoneNumberCodeTypeVerify &>(obj));
      return true;
    case phoneNumberCodeTypeConfirmOwnership::ID:
      func(static_cast<phoneNumberCodeTypeConfirmOwnership &>(obj));
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
bool downcast_call(PollType &obj, const T &func) {
  switch (obj.get_id()) {
    case pollTypeRegular::ID:
      func(static_cast<pollTypeRegular &>(obj));
      return true;
    case pollTypeQuiz::ID:
      func(static_cast<pollTypeQuiz &>(obj));
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
bool downcast_call(PremiumFeature &obj, const T &func) {
  switch (obj.get_id()) {
    case premiumFeatureIncreasedLimits::ID:
      func(static_cast<premiumFeatureIncreasedLimits &>(obj));
      return true;
    case premiumFeatureIncreasedUploadFileSize::ID:
      func(static_cast<premiumFeatureIncreasedUploadFileSize &>(obj));
      return true;
    case premiumFeatureImprovedDownloadSpeed::ID:
      func(static_cast<premiumFeatureImprovedDownloadSpeed &>(obj));
      return true;
    case premiumFeatureVoiceRecognition::ID:
      func(static_cast<premiumFeatureVoiceRecognition &>(obj));
      return true;
    case premiumFeatureDisabledAds::ID:
      func(static_cast<premiumFeatureDisabledAds &>(obj));
      return true;
    case premiumFeatureUniqueReactions::ID:
      func(static_cast<premiumFeatureUniqueReactions &>(obj));
      return true;
    case premiumFeatureUniqueStickers::ID:
      func(static_cast<premiumFeatureUniqueStickers &>(obj));
      return true;
    case premiumFeatureCustomEmoji::ID:
      func(static_cast<premiumFeatureCustomEmoji &>(obj));
      return true;
    case premiumFeatureAdvancedChatManagement::ID:
      func(static_cast<premiumFeatureAdvancedChatManagement &>(obj));
      return true;
    case premiumFeatureProfileBadge::ID:
      func(static_cast<premiumFeatureProfileBadge &>(obj));
      return true;
    case premiumFeatureEmojiStatus::ID:
      func(static_cast<premiumFeatureEmojiStatus &>(obj));
      return true;
    case premiumFeatureAnimatedProfilePhoto::ID:
      func(static_cast<premiumFeatureAnimatedProfilePhoto &>(obj));
      return true;
    case premiumFeatureForumTopicIcon::ID:
      func(static_cast<premiumFeatureForumTopicIcon &>(obj));
      return true;
    case premiumFeatureAppIcons::ID:
      func(static_cast<premiumFeatureAppIcons &>(obj));
      return true;
    case premiumFeatureRealTimeChatTranslation::ID:
      func(static_cast<premiumFeatureRealTimeChatTranslation &>(obj));
      return true;
    case premiumFeatureUpgradedStories::ID:
      func(static_cast<premiumFeatureUpgradedStories &>(obj));
      return true;
    case premiumFeatureChatBoost::ID:
      func(static_cast<premiumFeatureChatBoost &>(obj));
      return true;
    case premiumFeatureAccentColor::ID:
      func(static_cast<premiumFeatureAccentColor &>(obj));
      return true;
    case premiumFeatureBackgroundForBoth::ID:
      func(static_cast<premiumFeatureBackgroundForBoth &>(obj));
      return true;
    case premiumFeatureSavedMessagesTags::ID:
      func(static_cast<premiumFeatureSavedMessagesTags &>(obj));
      return true;
    case premiumFeatureMessagePrivacy::ID:
      func(static_cast<premiumFeatureMessagePrivacy &>(obj));
      return true;
    case premiumFeatureLastSeenTimes::ID:
      func(static_cast<premiumFeatureLastSeenTimes &>(obj));
      return true;
    case premiumFeatureBusiness::ID:
      func(static_cast<premiumFeatureBusiness &>(obj));
      return true;
    case premiumFeatureMessageEffects::ID:
      func(static_cast<premiumFeatureMessageEffects &>(obj));
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
bool downcast_call(PremiumLimitType &obj, const T &func) {
  switch (obj.get_id()) {
    case premiumLimitTypeSupergroupCount::ID:
      func(static_cast<premiumLimitTypeSupergroupCount &>(obj));
      return true;
    case premiumLimitTypePinnedChatCount::ID:
      func(static_cast<premiumLimitTypePinnedChatCount &>(obj));
      return true;
    case premiumLimitTypeCreatedPublicChatCount::ID:
      func(static_cast<premiumLimitTypeCreatedPublicChatCount &>(obj));
      return true;
    case premiumLimitTypeSavedAnimationCount::ID:
      func(static_cast<premiumLimitTypeSavedAnimationCount &>(obj));
      return true;
    case premiumLimitTypeFavoriteStickerCount::ID:
      func(static_cast<premiumLimitTypeFavoriteStickerCount &>(obj));
      return true;
    case premiumLimitTypeChatFolderCount::ID:
      func(static_cast<premiumLimitTypeChatFolderCount &>(obj));
      return true;
    case premiumLimitTypeChatFolderChosenChatCount::ID:
      func(static_cast<premiumLimitTypeChatFolderChosenChatCount &>(obj));
      return true;
    case premiumLimitTypePinnedArchivedChatCount::ID:
      func(static_cast<premiumLimitTypePinnedArchivedChatCount &>(obj));
      return true;
    case premiumLimitTypePinnedSavedMessagesTopicCount::ID:
      func(static_cast<premiumLimitTypePinnedSavedMessagesTopicCount &>(obj));
      return true;
    case premiumLimitTypeCaptionLength::ID:
      func(static_cast<premiumLimitTypeCaptionLength &>(obj));
      return true;
    case premiumLimitTypeBioLength::ID:
      func(static_cast<premiumLimitTypeBioLength &>(obj));
      return true;
    case premiumLimitTypeChatFolderInviteLinkCount::ID:
      func(static_cast<premiumLimitTypeChatFolderInviteLinkCount &>(obj));
      return true;
    case premiumLimitTypeShareableChatFolderCount::ID:
      func(static_cast<premiumLimitTypeShareableChatFolderCount &>(obj));
      return true;
    case premiumLimitTypeActiveStoryCount::ID:
      func(static_cast<premiumLimitTypeActiveStoryCount &>(obj));
      return true;
    case premiumLimitTypeWeeklySentStoryCount::ID:
      func(static_cast<premiumLimitTypeWeeklySentStoryCount &>(obj));
      return true;
    case premiumLimitTypeMonthlySentStoryCount::ID:
      func(static_cast<premiumLimitTypeMonthlySentStoryCount &>(obj));
      return true;
    case premiumLimitTypeStoryCaptionLength::ID:
      func(static_cast<premiumLimitTypeStoryCaptionLength &>(obj));
      return true;
    case premiumLimitTypeStorySuggestedReactionAreaCount::ID:
      func(static_cast<premiumLimitTypeStorySuggestedReactionAreaCount &>(obj));
      return true;
    case premiumLimitTypeSimilarChatCount::ID:
      func(static_cast<premiumLimitTypeSimilarChatCount &>(obj));
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
bool downcast_call(PremiumSource &obj, const T &func) {
  switch (obj.get_id()) {
    case premiumSourceLimitExceeded::ID:
      func(static_cast<premiumSourceLimitExceeded &>(obj));
      return true;
    case premiumSourceFeature::ID:
      func(static_cast<premiumSourceFeature &>(obj));
      return true;
    case premiumSourceBusinessFeature::ID:
      func(static_cast<premiumSourceBusinessFeature &>(obj));
      return true;
    case premiumSourceStoryFeature::ID:
      func(static_cast<premiumSourceStoryFeature &>(obj));
      return true;
    case premiumSourceLink::ID:
      func(static_cast<premiumSourceLink &>(obj));
      return true;
    case premiumSourceSettings::ID:
      func(static_cast<premiumSourceSettings &>(obj));
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
bool downcast_call(PremiumStoryFeature &obj, const T &func) {
  switch (obj.get_id()) {
    case premiumStoryFeaturePriorityOrder::ID:
      func(static_cast<premiumStoryFeaturePriorityOrder &>(obj));
      return true;
    case premiumStoryFeatureStealthMode::ID:
      func(static_cast<premiumStoryFeatureStealthMode &>(obj));
      return true;
    case premiumStoryFeaturePermanentViewsHistory::ID:
      func(static_cast<premiumStoryFeaturePermanentViewsHistory &>(obj));
      return true;
    case premiumStoryFeatureCustomExpirationDuration::ID:
      func(static_cast<premiumStoryFeatureCustomExpirationDuration &>(obj));
      return true;
    case premiumStoryFeatureSaveStories::ID:
      func(static_cast<premiumStoryFeatureSaveStories &>(obj));
      return true;
    case premiumStoryFeatureLinksAndFormatting::ID:
      func(static_cast<premiumStoryFeatureLinksAndFormatting &>(obj));
      return true;
    case premiumStoryFeatureVideoQuality::ID:
      func(static_cast<premiumStoryFeatureVideoQuality &>(obj));
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
bool downcast_call(ProxyType &obj, const T &func) {
  switch (obj.get_id()) {
    case proxyTypeSocks5::ID:
      func(static_cast<proxyTypeSocks5 &>(obj));
      return true;
    case proxyTypeHttp::ID:
      func(static_cast<proxyTypeHttp &>(obj));
      return true;
    case proxyTypeMtproto::ID:
      func(static_cast<proxyTypeMtproto &>(obj));
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
bool downcast_call(PublicChatType &obj, const T &func) {
  switch (obj.get_id()) {
    case publicChatTypeHasUsername::ID:
      func(static_cast<publicChatTypeHasUsername &>(obj));
      return true;
    case publicChatTypeIsLocationBased::ID:
      func(static_cast<publicChatTypeIsLocationBased &>(obj));
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
bool downcast_call(PublicForward &obj, const T &func) {
  switch (obj.get_id()) {
    case publicForwardMessage::ID:
      func(static_cast<publicForwardMessage &>(obj));
      return true;
    case publicForwardStory::ID:
      func(static_cast<publicForwardStory &>(obj));
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
bool downcast_call(PushMessageContent &obj, const T &func) {
  switch (obj.get_id()) {
    case pushMessageContentHidden::ID:
      func(static_cast<pushMessageContentHidden &>(obj));
      return true;
    case pushMessageContentAnimation::ID:
      func(static_cast<pushMessageContentAnimation &>(obj));
      return true;
    case pushMessageContentAudio::ID:
      func(static_cast<pushMessageContentAudio &>(obj));
      return true;
    case pushMessageContentContact::ID:
      func(static_cast<pushMessageContentContact &>(obj));
      return true;
    case pushMessageContentContactRegistered::ID:
      func(static_cast<pushMessageContentContactRegistered &>(obj));
      return true;
    case pushMessageContentDocument::ID:
      func(static_cast<pushMessageContentDocument &>(obj));
      return true;
    case pushMessageContentGame::ID:
      func(static_cast<pushMessageContentGame &>(obj));
      return true;
    case pushMessageContentGameScore::ID:
      func(static_cast<pushMessageContentGameScore &>(obj));
      return true;
    case pushMessageContentInvoice::ID:
      func(static_cast<pushMessageContentInvoice &>(obj));
      return true;
    case pushMessageContentLocation::ID:
      func(static_cast<pushMessageContentLocation &>(obj));
      return true;
    case pushMessageContentPaidMedia::ID:
      func(static_cast<pushMessageContentPaidMedia &>(obj));
      return true;
    case pushMessageContentPhoto::ID:
      func(static_cast<pushMessageContentPhoto &>(obj));
      return true;
    case pushMessageContentPoll::ID:
      func(static_cast<pushMessageContentPoll &>(obj));
      return true;
    case pushMessageContentPremiumGiftCode::ID:
      func(static_cast<pushMessageContentPremiumGiftCode &>(obj));
      return true;
    case pushMessageContentGiveaway::ID:
      func(static_cast<pushMessageContentGiveaway &>(obj));
      return true;
    case pushMessageContentScreenshotTaken::ID:
      func(static_cast<pushMessageContentScreenshotTaken &>(obj));
      return true;
    case pushMessageContentSticker::ID:
      func(static_cast<pushMessageContentSticker &>(obj));
      return true;
    case pushMessageContentStory::ID:
      func(static_cast<pushMessageContentStory &>(obj));
      return true;
    case pushMessageContentText::ID:
      func(static_cast<pushMessageContentText &>(obj));
      return true;
    case pushMessageContentVideo::ID:
      func(static_cast<pushMessageContentVideo &>(obj));
      return true;
    case pushMessageContentVideoNote::ID:
      func(static_cast<pushMessageContentVideoNote &>(obj));
      return true;
    case pushMessageContentVoiceNote::ID:
      func(static_cast<pushMessageContentVoiceNote &>(obj));
      return true;
    case pushMessageContentBasicGroupChatCreate::ID:
      func(static_cast<pushMessageContentBasicGroupChatCreate &>(obj));
      return true;
    case pushMessageContentChatAddMembers::ID:
      func(static_cast<pushMessageContentChatAddMembers &>(obj));
      return true;
    case pushMessageContentChatChangePhoto::ID:
      func(static_cast<pushMessageContentChatChangePhoto &>(obj));
      return true;
    case pushMessageContentChatChangeTitle::ID:
      func(static_cast<pushMessageContentChatChangeTitle &>(obj));
      return true;
    case pushMessageContentChatSetBackground::ID:
      func(static_cast<pushMessageContentChatSetBackground &>(obj));
      return true;
    case pushMessageContentChatSetTheme::ID:
      func(static_cast<pushMessageContentChatSetTheme &>(obj));
      return true;
    case pushMessageContentChatDeleteMember::ID:
      func(static_cast<pushMessageContentChatDeleteMember &>(obj));
      return true;
    case pushMessageContentChatJoinByLink::ID:
      func(static_cast<pushMessageContentChatJoinByLink &>(obj));
      return true;
    case pushMessageContentChatJoinByRequest::ID:
      func(static_cast<pushMessageContentChatJoinByRequest &>(obj));
      return true;
    case pushMessageContentRecurringPayment::ID:
      func(static_cast<pushMessageContentRecurringPayment &>(obj));
      return true;
    case pushMessageContentSuggestProfilePhoto::ID:
      func(static_cast<pushMessageContentSuggestProfilePhoto &>(obj));
      return true;
    case pushMessageContentMessageForwards::ID:
      func(static_cast<pushMessageContentMessageForwards &>(obj));
      return true;
    case pushMessageContentMediaAlbum::ID:
      func(static_cast<pushMessageContentMediaAlbum &>(obj));
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
bool downcast_call(ReactionNotificationSource &obj, const T &func) {
  switch (obj.get_id()) {
    case reactionNotificationSourceNone::ID:
      func(static_cast<reactionNotificationSourceNone &>(obj));
      return true;
    case reactionNotificationSourceContacts::ID:
      func(static_cast<reactionNotificationSourceContacts &>(obj));
      return true;
    case reactionNotificationSourceAll::ID:
      func(static_cast<reactionNotificationSourceAll &>(obj));
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
bool downcast_call(ReactionType &obj, const T &func) {
  switch (obj.get_id()) {
    case reactionTypeEmoji::ID:
      func(static_cast<reactionTypeEmoji &>(obj));
      return true;
    case reactionTypeCustomEmoji::ID:
      func(static_cast<reactionTypeCustomEmoji &>(obj));
      return true;
    case reactionTypePaid::ID:
      func(static_cast<reactionTypePaid &>(obj));
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
bool downcast_call(ReactionUnavailabilityReason &obj, const T &func) {
  switch (obj.get_id()) {
    case reactionUnavailabilityReasonAnonymousAdministrator::ID:
      func(static_cast<reactionUnavailabilityReasonAnonymousAdministrator &>(obj));
      return true;
    case reactionUnavailabilityReasonGuest::ID:
      func(static_cast<reactionUnavailabilityReasonGuest &>(obj));
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
bool downcast_call(ReplyMarkup &obj, const T &func) {
  switch (obj.get_id()) {
    case replyMarkupRemoveKeyboard::ID:
      func(static_cast<replyMarkupRemoveKeyboard &>(obj));
      return true;
    case replyMarkupForceReply::ID:
      func(static_cast<replyMarkupForceReply &>(obj));
      return true;
    case replyMarkupShowKeyboard::ID:
      func(static_cast<replyMarkupShowKeyboard &>(obj));
      return true;
    case replyMarkupInlineKeyboard::ID:
      func(static_cast<replyMarkupInlineKeyboard &>(obj));
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
bool downcast_call(ReportChatSponsoredMessageResult &obj, const T &func) {
  switch (obj.get_id()) {
    case reportChatSponsoredMessageResultOk::ID:
      func(static_cast<reportChatSponsoredMessageResultOk &>(obj));
      return true;
    case reportChatSponsoredMessageResultFailed::ID:
      func(static_cast<reportChatSponsoredMessageResultFailed &>(obj));
      return true;
    case reportChatSponsoredMessageResultOptionRequired::ID:
      func(static_cast<reportChatSponsoredMessageResultOptionRequired &>(obj));
      return true;
    case reportChatSponsoredMessageResultAdsHidden::ID:
      func(static_cast<reportChatSponsoredMessageResultAdsHidden &>(obj));
      return true;
    case reportChatSponsoredMessageResultPremiumRequired::ID:
      func(static_cast<reportChatSponsoredMessageResultPremiumRequired &>(obj));
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
bool downcast_call(ReportReason &obj, const T &func) {
  switch (obj.get_id()) {
    case reportReasonSpam::ID:
      func(static_cast<reportReasonSpam &>(obj));
      return true;
    case reportReasonViolence::ID:
      func(static_cast<reportReasonViolence &>(obj));
      return true;
    case reportReasonPornography::ID:
      func(static_cast<reportReasonPornography &>(obj));
      return true;
    case reportReasonChildAbuse::ID:
      func(static_cast<reportReasonChildAbuse &>(obj));
      return true;
    case reportReasonCopyright::ID:
      func(static_cast<reportReasonCopyright &>(obj));
      return true;
    case reportReasonUnrelatedLocation::ID:
      func(static_cast<reportReasonUnrelatedLocation &>(obj));
      return true;
    case reportReasonFake::ID:
      func(static_cast<reportReasonFake &>(obj));
      return true;
    case reportReasonIllegalDrugs::ID:
      func(static_cast<reportReasonIllegalDrugs &>(obj));
      return true;
    case reportReasonPersonalDetails::ID:
      func(static_cast<reportReasonPersonalDetails &>(obj));
      return true;
    case reportReasonCustom::ID:
      func(static_cast<reportReasonCustom &>(obj));
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
bool downcast_call(ResendCodeReason &obj, const T &func) {
  switch (obj.get_id()) {
    case resendCodeReasonUserRequest::ID:
      func(static_cast<resendCodeReasonUserRequest &>(obj));
      return true;
    case resendCodeReasonVerificationFailed::ID:
      func(static_cast<resendCodeReasonVerificationFailed &>(obj));
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
bool downcast_call(ResetPasswordResult &obj, const T &func) {
  switch (obj.get_id()) {
    case resetPasswordResultOk::ID:
      func(static_cast<resetPasswordResultOk &>(obj));
      return true;
    case resetPasswordResultPending::ID:
      func(static_cast<resetPasswordResultPending &>(obj));
      return true;
    case resetPasswordResultDeclined::ID:
      func(static_cast<resetPasswordResultDeclined &>(obj));
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
bool downcast_call(RevenueWithdrawalState &obj, const T &func) {
  switch (obj.get_id()) {
    case revenueWithdrawalStatePending::ID:
      func(static_cast<revenueWithdrawalStatePending &>(obj));
      return true;
    case revenueWithdrawalStateSucceeded::ID:
      func(static_cast<revenueWithdrawalStateSucceeded &>(obj));
      return true;
    case revenueWithdrawalStateFailed::ID:
      func(static_cast<revenueWithdrawalStateFailed &>(obj));
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
bool downcast_call(RichText &obj, const T &func) {
  switch (obj.get_id()) {
    case richTextPlain::ID:
      func(static_cast<richTextPlain &>(obj));
      return true;
    case richTextBold::ID:
      func(static_cast<richTextBold &>(obj));
      return true;
    case richTextItalic::ID:
      func(static_cast<richTextItalic &>(obj));
      return true;
    case richTextUnderline::ID:
      func(static_cast<richTextUnderline &>(obj));
      return true;
    case richTextStrikethrough::ID:
      func(static_cast<richTextStrikethrough &>(obj));
      return true;
    case richTextFixed::ID:
      func(static_cast<richTextFixed &>(obj));
      return true;
    case richTextUrl::ID:
      func(static_cast<richTextUrl &>(obj));
      return true;
    case richTextEmailAddress::ID:
      func(static_cast<richTextEmailAddress &>(obj));
      return true;
    case richTextSubscript::ID:
      func(static_cast<richTextSubscript &>(obj));
      return true;
    case richTextSuperscript::ID:
      func(static_cast<richTextSuperscript &>(obj));
      return true;
    case richTextMarked::ID:
      func(static_cast<richTextMarked &>(obj));
      return true;
    case richTextPhoneNumber::ID:
      func(static_cast<richTextPhoneNumber &>(obj));
      return true;
    case richTextIcon::ID:
      func(static_cast<richTextIcon &>(obj));
      return true;
    case richTextReference::ID:
      func(static_cast<richTextReference &>(obj));
      return true;
    case richTextAnchor::ID:
      func(static_cast<richTextAnchor &>(obj));
      return true;
    case richTextAnchorLink::ID:
      func(static_cast<richTextAnchorLink &>(obj));
      return true;
    case richTexts::ID:
      func(static_cast<richTexts &>(obj));
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
bool downcast_call(SavedMessagesTopicType &obj, const T &func) {
  switch (obj.get_id()) {
    case savedMessagesTopicTypeMyNotes::ID:
      func(static_cast<savedMessagesTopicTypeMyNotes &>(obj));
      return true;
    case savedMessagesTopicTypeAuthorHidden::ID:
      func(static_cast<savedMessagesTopicTypeAuthorHidden &>(obj));
      return true;
    case savedMessagesTopicTypeSavedFromChat::ID:
      func(static_cast<savedMessagesTopicTypeSavedFromChat &>(obj));
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
bool downcast_call(SearchMessagesFilter &obj, const T &func) {
  switch (obj.get_id()) {
    case searchMessagesFilterEmpty::ID:
      func(static_cast<searchMessagesFilterEmpty &>(obj));
      return true;
    case searchMessagesFilterAnimation::ID:
      func(static_cast<searchMessagesFilterAnimation &>(obj));
      return true;
    case searchMessagesFilterAudio::ID:
      func(static_cast<searchMessagesFilterAudio &>(obj));
      return true;
    case searchMessagesFilterDocument::ID:
      func(static_cast<searchMessagesFilterDocument &>(obj));
      return true;
    case searchMessagesFilterPhoto::ID:
      func(static_cast<searchMessagesFilterPhoto &>(obj));
      return true;
    case searchMessagesFilterVideo::ID:
      func(static_cast<searchMessagesFilterVideo &>(obj));
      return true;
    case searchMessagesFilterVoiceNote::ID:
      func(static_cast<searchMessagesFilterVoiceNote &>(obj));
      return true;
    case searchMessagesFilterPhotoAndVideo::ID:
      func(static_cast<searchMessagesFilterPhotoAndVideo &>(obj));
      return true;
    case searchMessagesFilterUrl::ID:
      func(static_cast<searchMessagesFilterUrl &>(obj));
      return true;
    case searchMessagesFilterChatPhoto::ID:
      func(static_cast<searchMessagesFilterChatPhoto &>(obj));
      return true;
    case searchMessagesFilterVideoNote::ID:
      func(static_cast<searchMessagesFilterVideoNote &>(obj));
      return true;
    case searchMessagesFilterVoiceAndVideoNote::ID:
      func(static_cast<searchMessagesFilterVoiceAndVideoNote &>(obj));
      return true;
    case searchMessagesFilterMention::ID:
      func(static_cast<searchMessagesFilterMention &>(obj));
      return true;
    case searchMessagesFilterUnreadMention::ID:
      func(static_cast<searchMessagesFilterUnreadMention &>(obj));
      return true;
    case searchMessagesFilterUnreadReaction::ID:
      func(static_cast<searchMessagesFilterUnreadReaction &>(obj));
      return true;
    case searchMessagesFilterFailedToSend::ID:
      func(static_cast<searchMessagesFilterFailedToSend &>(obj));
      return true;
    case searchMessagesFilterPinned::ID:
      func(static_cast<searchMessagesFilterPinned &>(obj));
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
bool downcast_call(SecretChatState &obj, const T &func) {
  switch (obj.get_id()) {
    case secretChatStatePending::ID:
      func(static_cast<secretChatStatePending &>(obj));
      return true;
    case secretChatStateReady::ID:
      func(static_cast<secretChatStateReady &>(obj));
      return true;
    case secretChatStateClosed::ID:
      func(static_cast<secretChatStateClosed &>(obj));
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
bool downcast_call(SessionType &obj, const T &func) {
  switch (obj.get_id()) {
    case sessionTypeAndroid::ID:
      func(static_cast<sessionTypeAndroid &>(obj));
      return true;
    case sessionTypeApple::ID:
      func(static_cast<sessionTypeApple &>(obj));
      return true;
    case sessionTypeBrave::ID:
      func(static_cast<sessionTypeBrave &>(obj));
      return true;
    case sessionTypeChrome::ID:
      func(static_cast<sessionTypeChrome &>(obj));
      return true;
    case sessionTypeEdge::ID:
      func(static_cast<sessionTypeEdge &>(obj));
      return true;
    case sessionTypeFirefox::ID:
      func(static_cast<sessionTypeFirefox &>(obj));
      return true;
    case sessionTypeIpad::ID:
      func(static_cast<sessionTypeIpad &>(obj));
      return true;
    case sessionTypeIphone::ID:
      func(static_cast<sessionTypeIphone &>(obj));
      return true;
    case sessionTypeLinux::ID:
      func(static_cast<sessionTypeLinux &>(obj));
      return true;
    case sessionTypeMac::ID:
      func(static_cast<sessionTypeMac &>(obj));
      return true;
    case sessionTypeOpera::ID:
      func(static_cast<sessionTypeOpera &>(obj));
      return true;
    case sessionTypeSafari::ID:
      func(static_cast<sessionTypeSafari &>(obj));
      return true;
    case sessionTypeUbuntu::ID:
      func(static_cast<sessionTypeUbuntu &>(obj));
      return true;
    case sessionTypeUnknown::ID:
      func(static_cast<sessionTypeUnknown &>(obj));
      return true;
    case sessionTypeVivaldi::ID:
      func(static_cast<sessionTypeVivaldi &>(obj));
      return true;
    case sessionTypeWindows::ID:
      func(static_cast<sessionTypeWindows &>(obj));
      return true;
    case sessionTypeXbox::ID:
      func(static_cast<sessionTypeXbox &>(obj));
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
bool downcast_call(SpeechRecognitionResult &obj, const T &func) {
  switch (obj.get_id()) {
    case speechRecognitionResultPending::ID:
      func(static_cast<speechRecognitionResultPending &>(obj));
      return true;
    case speechRecognitionResultText::ID:
      func(static_cast<speechRecognitionResultText &>(obj));
      return true;
    case speechRecognitionResultError::ID:
      func(static_cast<speechRecognitionResultError &>(obj));
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
bool downcast_call(StarTransactionDirection &obj, const T &func) {
  switch (obj.get_id()) {
    case starTransactionDirectionIncoming::ID:
      func(static_cast<starTransactionDirectionIncoming &>(obj));
      return true;
    case starTransactionDirectionOutgoing::ID:
      func(static_cast<starTransactionDirectionOutgoing &>(obj));
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
bool downcast_call(StarTransactionPartner &obj, const T &func) {
  switch (obj.get_id()) {
    case starTransactionPartnerTelegram::ID:
      func(static_cast<starTransactionPartnerTelegram &>(obj));
      return true;
    case starTransactionPartnerAppStore::ID:
      func(static_cast<starTransactionPartnerAppStore &>(obj));
      return true;
    case starTransactionPartnerGooglePlay::ID:
      func(static_cast<starTransactionPartnerGooglePlay &>(obj));
      return true;
    case starTransactionPartnerFragment::ID:
      func(static_cast<starTransactionPartnerFragment &>(obj));
      return true;
    case starTransactionPartnerTelegramAds::ID:
      func(static_cast<starTransactionPartnerTelegramAds &>(obj));
      return true;
    case starTransactionPartnerBot::ID:
      func(static_cast<starTransactionPartnerBot &>(obj));
      return true;
    case starTransactionPartnerBusiness::ID:
      func(static_cast<starTransactionPartnerBusiness &>(obj));
      return true;
    case starTransactionPartnerChat::ID:
      func(static_cast<starTransactionPartnerChat &>(obj));
      return true;
    case starTransactionPartnerUser::ID:
      func(static_cast<starTransactionPartnerUser &>(obj));
      return true;
    case starTransactionPartnerUnsupported::ID:
      func(static_cast<starTransactionPartnerUnsupported &>(obj));
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
bool downcast_call(StatisticalGraph &obj, const T &func) {
  switch (obj.get_id()) {
    case statisticalGraphData::ID:
      func(static_cast<statisticalGraphData &>(obj));
      return true;
    case statisticalGraphAsync::ID:
      func(static_cast<statisticalGraphAsync &>(obj));
      return true;
    case statisticalGraphError::ID:
      func(static_cast<statisticalGraphError &>(obj));
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
bool downcast_call(StickerFormat &obj, const T &func) {
  switch (obj.get_id()) {
    case stickerFormatWebp::ID:
      func(static_cast<stickerFormatWebp &>(obj));
      return true;
    case stickerFormatTgs::ID:
      func(static_cast<stickerFormatTgs &>(obj));
      return true;
    case stickerFormatWebm::ID:
      func(static_cast<stickerFormatWebm &>(obj));
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
bool downcast_call(StickerFullType &obj, const T &func) {
  switch (obj.get_id()) {
    case stickerFullTypeRegular::ID:
      func(static_cast<stickerFullTypeRegular &>(obj));
      return true;
    case stickerFullTypeMask::ID:
      func(static_cast<stickerFullTypeMask &>(obj));
      return true;
    case stickerFullTypeCustomEmoji::ID:
      func(static_cast<stickerFullTypeCustomEmoji &>(obj));
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
bool downcast_call(StickerType &obj, const T &func) {
  switch (obj.get_id()) {
    case stickerTypeRegular::ID:
      func(static_cast<stickerTypeRegular &>(obj));
      return true;
    case stickerTypeMask::ID:
      func(static_cast<stickerTypeMask &>(obj));
      return true;
    case stickerTypeCustomEmoji::ID:
      func(static_cast<stickerTypeCustomEmoji &>(obj));
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
bool downcast_call(StorePaymentPurpose &obj, const T &func) {
  switch (obj.get_id()) {
    case storePaymentPurposePremiumSubscription::ID:
      func(static_cast<storePaymentPurposePremiumSubscription &>(obj));
      return true;
    case storePaymentPurposeGiftedPremium::ID:
      func(static_cast<storePaymentPurposeGiftedPremium &>(obj));
      return true;
    case storePaymentPurposePremiumGiftCodes::ID:
      func(static_cast<storePaymentPurposePremiumGiftCodes &>(obj));
      return true;
    case storePaymentPurposePremiumGiveaway::ID:
      func(static_cast<storePaymentPurposePremiumGiveaway &>(obj));
      return true;
    case storePaymentPurposeStarGiveaway::ID:
      func(static_cast<storePaymentPurposeStarGiveaway &>(obj));
      return true;
    case storePaymentPurposeStars::ID:
      func(static_cast<storePaymentPurposeStars &>(obj));
      return true;
    case storePaymentPurposeGiftedStars::ID:
      func(static_cast<storePaymentPurposeGiftedStars &>(obj));
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
bool downcast_call(StoryAreaType &obj, const T &func) {
  switch (obj.get_id()) {
    case storyAreaTypeLocation::ID:
      func(static_cast<storyAreaTypeLocation &>(obj));
      return true;
    case storyAreaTypeVenue::ID:
      func(static_cast<storyAreaTypeVenue &>(obj));
      return true;
    case storyAreaTypeSuggestedReaction::ID:
      func(static_cast<storyAreaTypeSuggestedReaction &>(obj));
      return true;
    case storyAreaTypeMessage::ID:
      func(static_cast<storyAreaTypeMessage &>(obj));
      return true;
    case storyAreaTypeLink::ID:
      func(static_cast<storyAreaTypeLink &>(obj));
      return true;
    case storyAreaTypeWeather::ID:
      func(static_cast<storyAreaTypeWeather &>(obj));
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
bool downcast_call(StoryContent &obj, const T &func) {
  switch (obj.get_id()) {
    case storyContentPhoto::ID:
      func(static_cast<storyContentPhoto &>(obj));
      return true;
    case storyContentVideo::ID:
      func(static_cast<storyContentVideo &>(obj));
      return true;
    case storyContentUnsupported::ID:
      func(static_cast<storyContentUnsupported &>(obj));
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
bool downcast_call(StoryInteractionType &obj, const T &func) {
  switch (obj.get_id()) {
    case storyInteractionTypeView::ID:
      func(static_cast<storyInteractionTypeView &>(obj));
      return true;
    case storyInteractionTypeForward::ID:
      func(static_cast<storyInteractionTypeForward &>(obj));
      return true;
    case storyInteractionTypeRepost::ID:
      func(static_cast<storyInteractionTypeRepost &>(obj));
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
bool downcast_call(StoryList &obj, const T &func) {
  switch (obj.get_id()) {
    case storyListMain::ID:
      func(static_cast<storyListMain &>(obj));
      return true;
    case storyListArchive::ID:
      func(static_cast<storyListArchive &>(obj));
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
bool downcast_call(StoryOrigin &obj, const T &func) {
  switch (obj.get_id()) {
    case storyOriginPublicStory::ID:
      func(static_cast<storyOriginPublicStory &>(obj));
      return true;
    case storyOriginHiddenUser::ID:
      func(static_cast<storyOriginHiddenUser &>(obj));
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
bool downcast_call(StoryPrivacySettings &obj, const T &func) {
  switch (obj.get_id()) {
    case storyPrivacySettingsEveryone::ID:
      func(static_cast<storyPrivacySettingsEveryone &>(obj));
      return true;
    case storyPrivacySettingsContacts::ID:
      func(static_cast<storyPrivacySettingsContacts &>(obj));
      return true;
    case storyPrivacySettingsCloseFriends::ID:
      func(static_cast<storyPrivacySettingsCloseFriends &>(obj));
      return true;
    case storyPrivacySettingsSelectedUsers::ID:
      func(static_cast<storyPrivacySettingsSelectedUsers &>(obj));
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
bool downcast_call(SuggestedAction &obj, const T &func) {
  switch (obj.get_id()) {
    case suggestedActionEnableArchiveAndMuteNewChats::ID:
      func(static_cast<suggestedActionEnableArchiveAndMuteNewChats &>(obj));
      return true;
    case suggestedActionCheckPassword::ID:
      func(static_cast<suggestedActionCheckPassword &>(obj));
      return true;
    case suggestedActionCheckPhoneNumber::ID:
      func(static_cast<suggestedActionCheckPhoneNumber &>(obj));
      return true;
    case suggestedActionViewChecksHint::ID:
      func(static_cast<suggestedActionViewChecksHint &>(obj));
      return true;
    case suggestedActionConvertToBroadcastGroup::ID:
      func(static_cast<suggestedActionConvertToBroadcastGroup &>(obj));
      return true;
    case suggestedActionSetPassword::ID:
      func(static_cast<suggestedActionSetPassword &>(obj));
      return true;
    case suggestedActionUpgradePremium::ID:
      func(static_cast<suggestedActionUpgradePremium &>(obj));
      return true;
    case suggestedActionRestorePremium::ID:
      func(static_cast<suggestedActionRestorePremium &>(obj));
      return true;
    case suggestedActionSubscribeToAnnualPremium::ID:
      func(static_cast<suggestedActionSubscribeToAnnualPremium &>(obj));
      return true;
    case suggestedActionGiftPremiumForChristmas::ID:
      func(static_cast<suggestedActionGiftPremiumForChristmas &>(obj));
      return true;
    case suggestedActionSetBirthdate::ID:
      func(static_cast<suggestedActionSetBirthdate &>(obj));
      return true;
    case suggestedActionExtendPremium::ID:
      func(static_cast<suggestedActionExtendPremium &>(obj));
      return true;
    case suggestedActionExtendStarSubscriptions::ID:
      func(static_cast<suggestedActionExtendStarSubscriptions &>(obj));
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
bool downcast_call(SupergroupMembersFilter &obj, const T &func) {
  switch (obj.get_id()) {
    case supergroupMembersFilterRecent::ID:
      func(static_cast<supergroupMembersFilterRecent &>(obj));
      return true;
    case supergroupMembersFilterContacts::ID:
      func(static_cast<supergroupMembersFilterContacts &>(obj));
      return true;
    case supergroupMembersFilterAdministrators::ID:
      func(static_cast<supergroupMembersFilterAdministrators &>(obj));
      return true;
    case supergroupMembersFilterSearch::ID:
      func(static_cast<supergroupMembersFilterSearch &>(obj));
      return true;
    case supergroupMembersFilterRestricted::ID:
      func(static_cast<supergroupMembersFilterRestricted &>(obj));
      return true;
    case supergroupMembersFilterBanned::ID:
      func(static_cast<supergroupMembersFilterBanned &>(obj));
      return true;
    case supergroupMembersFilterMention::ID:
      func(static_cast<supergroupMembersFilterMention &>(obj));
      return true;
    case supergroupMembersFilterBots::ID:
      func(static_cast<supergroupMembersFilterBots &>(obj));
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
bool downcast_call(TMeUrlType &obj, const T &func) {
  switch (obj.get_id()) {
    case tMeUrlTypeUser::ID:
      func(static_cast<tMeUrlTypeUser &>(obj));
      return true;
    case tMeUrlTypeSupergroup::ID:
      func(static_cast<tMeUrlTypeSupergroup &>(obj));
      return true;
    case tMeUrlTypeChatInvite::ID:
      func(static_cast<tMeUrlTypeChatInvite &>(obj));
      return true;
    case tMeUrlTypeStickerSet::ID:
      func(static_cast<tMeUrlTypeStickerSet &>(obj));
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
bool downcast_call(TargetChat &obj, const T &func) {
  switch (obj.get_id()) {
    case targetChatCurrent::ID:
      func(static_cast<targetChatCurrent &>(obj));
      return true;
    case targetChatChosen::ID:
      func(static_cast<targetChatChosen &>(obj));
      return true;
    case targetChatInternalLink::ID:
      func(static_cast<targetChatInternalLink &>(obj));
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
bool downcast_call(TelegramPaymentPurpose &obj, const T &func) {
  switch (obj.get_id()) {
    case telegramPaymentPurposePremiumGiftCodes::ID:
      func(static_cast<telegramPaymentPurposePremiumGiftCodes &>(obj));
      return true;
    case telegramPaymentPurposePremiumGiveaway::ID:
      func(static_cast<telegramPaymentPurposePremiumGiveaway &>(obj));
      return true;
    case telegramPaymentPurposeStars::ID:
      func(static_cast<telegramPaymentPurposeStars &>(obj));
      return true;
    case telegramPaymentPurposeGiftedStars::ID:
      func(static_cast<telegramPaymentPurposeGiftedStars &>(obj));
      return true;
    case telegramPaymentPurposeStarGiveaway::ID:
      func(static_cast<telegramPaymentPurposeStarGiveaway &>(obj));
      return true;
    case telegramPaymentPurposeJoinChat::ID:
      func(static_cast<telegramPaymentPurposeJoinChat &>(obj));
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
bool downcast_call(TextEntityType &obj, const T &func) {
  switch (obj.get_id()) {
    case textEntityTypeMention::ID:
      func(static_cast<textEntityTypeMention &>(obj));
      return true;
    case textEntityTypeHashtag::ID:
      func(static_cast<textEntityTypeHashtag &>(obj));
      return true;
    case textEntityTypeCashtag::ID:
      func(static_cast<textEntityTypeCashtag &>(obj));
      return true;
    case textEntityTypeBotCommand::ID:
      func(static_cast<textEntityTypeBotCommand &>(obj));
      return true;
    case textEntityTypeUrl::ID:
      func(static_cast<textEntityTypeUrl &>(obj));
      return true;
    case textEntityTypeEmailAddress::ID:
      func(static_cast<textEntityTypeEmailAddress &>(obj));
      return true;
    case textEntityTypePhoneNumber::ID:
      func(static_cast<textEntityTypePhoneNumber &>(obj));
      return true;
    case textEntityTypeBankCardNumber::ID:
      func(static_cast<textEntityTypeBankCardNumber &>(obj));
      return true;
    case textEntityTypeBold::ID:
      func(static_cast<textEntityTypeBold &>(obj));
      return true;
    case textEntityTypeItalic::ID:
      func(static_cast<textEntityTypeItalic &>(obj));
      return true;
    case textEntityTypeUnderline::ID:
      func(static_cast<textEntityTypeUnderline &>(obj));
      return true;
    case textEntityTypeStrikethrough::ID:
      func(static_cast<textEntityTypeStrikethrough &>(obj));
      return true;
    case textEntityTypeSpoiler::ID:
      func(static_cast<textEntityTypeSpoiler &>(obj));
      return true;
    case textEntityTypeCode::ID:
      func(static_cast<textEntityTypeCode &>(obj));
      return true;
    case textEntityTypePre::ID:
      func(static_cast<textEntityTypePre &>(obj));
      return true;
    case textEntityTypePreCode::ID:
      func(static_cast<textEntityTypePreCode &>(obj));
      return true;
    case textEntityTypeBlockQuote::ID:
      func(static_cast<textEntityTypeBlockQuote &>(obj));
      return true;
    case textEntityTypeExpandableBlockQuote::ID:
      func(static_cast<textEntityTypeExpandableBlockQuote &>(obj));
      return true;
    case textEntityTypeTextUrl::ID:
      func(static_cast<textEntityTypeTextUrl &>(obj));
      return true;
    case textEntityTypeMentionName::ID:
      func(static_cast<textEntityTypeMentionName &>(obj));
      return true;
    case textEntityTypeCustomEmoji::ID:
      func(static_cast<textEntityTypeCustomEmoji &>(obj));
      return true;
    case textEntityTypeMediaTimestamp::ID:
      func(static_cast<textEntityTypeMediaTimestamp &>(obj));
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
bool downcast_call(TextParseMode &obj, const T &func) {
  switch (obj.get_id()) {
    case textParseModeMarkdown::ID:
      func(static_cast<textParseModeMarkdown &>(obj));
      return true;
    case textParseModeHTML::ID:
      func(static_cast<textParseModeHTML &>(obj));
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
bool downcast_call(ThumbnailFormat &obj, const T &func) {
  switch (obj.get_id()) {
    case thumbnailFormatJpeg::ID:
      func(static_cast<thumbnailFormatJpeg &>(obj));
      return true;
    case thumbnailFormatGif::ID:
      func(static_cast<thumbnailFormatGif &>(obj));
      return true;
    case thumbnailFormatMpeg4::ID:
      func(static_cast<thumbnailFormatMpeg4 &>(obj));
      return true;
    case thumbnailFormatPng::ID:
      func(static_cast<thumbnailFormatPng &>(obj));
      return true;
    case thumbnailFormatTgs::ID:
      func(static_cast<thumbnailFormatTgs &>(obj));
      return true;
    case thumbnailFormatWebm::ID:
      func(static_cast<thumbnailFormatWebm &>(obj));
      return true;
    case thumbnailFormatWebp::ID:
      func(static_cast<thumbnailFormatWebp &>(obj));
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
bool downcast_call(TopChatCategory &obj, const T &func) {
  switch (obj.get_id()) {
    case topChatCategoryUsers::ID:
      func(static_cast<topChatCategoryUsers &>(obj));
      return true;
    case topChatCategoryBots::ID:
      func(static_cast<topChatCategoryBots &>(obj));
      return true;
    case topChatCategoryGroups::ID:
      func(static_cast<topChatCategoryGroups &>(obj));
      return true;
    case topChatCategoryChannels::ID:
      func(static_cast<topChatCategoryChannels &>(obj));
      return true;
    case topChatCategoryInlineBots::ID:
      func(static_cast<topChatCategoryInlineBots &>(obj));
      return true;
    case topChatCategoryWebAppBots::ID:
      func(static_cast<topChatCategoryWebAppBots &>(obj));
      return true;
    case topChatCategoryCalls::ID:
      func(static_cast<topChatCategoryCalls &>(obj));
      return true;
    case topChatCategoryForwardChats::ID:
      func(static_cast<topChatCategoryForwardChats &>(obj));
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
bool downcast_call(Update &obj, const T &func) {
  switch (obj.get_id()) {
    case updateAuthorizationState::ID:
      func(static_cast<updateAuthorizationState &>(obj));
      return true;
    case updateNewMessage::ID:
      func(static_cast<updateNewMessage &>(obj));
      return true;
    case updateMessageSendAcknowledged::ID:
      func(static_cast<updateMessageSendAcknowledged &>(obj));
      return true;
    case updateMessageSendSucceeded::ID:
      func(static_cast<updateMessageSendSucceeded &>(obj));
      return true;
    case updateMessageSendFailed::ID:
      func(static_cast<updateMessageSendFailed &>(obj));
      return true;
    case updateMessageContent::ID:
      func(static_cast<updateMessageContent &>(obj));
      return true;
    case updateMessageEdited::ID:
      func(static_cast<updateMessageEdited &>(obj));
      return true;
    case updateMessageIsPinned::ID:
      func(static_cast<updateMessageIsPinned &>(obj));
      return true;
    case updateMessageInteractionInfo::ID:
      func(static_cast<updateMessageInteractionInfo &>(obj));
      return true;
    case updateMessageContentOpened::ID:
      func(static_cast<updateMessageContentOpened &>(obj));
      return true;
    case updateMessageMentionRead::ID:
      func(static_cast<updateMessageMentionRead &>(obj));
      return true;
    case updateMessageUnreadReactions::ID:
      func(static_cast<updateMessageUnreadReactions &>(obj));
      return true;
    case updateMessageFactCheck::ID:
      func(static_cast<updateMessageFactCheck &>(obj));
      return true;
    case updateMessageLiveLocationViewed::ID:
      func(static_cast<updateMessageLiveLocationViewed &>(obj));
      return true;
    case updateNewChat::ID:
      func(static_cast<updateNewChat &>(obj));
      return true;
    case updateChatTitle::ID:
      func(static_cast<updateChatTitle &>(obj));
      return true;
    case updateChatPhoto::ID:
      func(static_cast<updateChatPhoto &>(obj));
      return true;
    case updateChatAccentColors::ID:
      func(static_cast<updateChatAccentColors &>(obj));
      return true;
    case updateChatPermissions::ID:
      func(static_cast<updateChatPermissions &>(obj));
      return true;
    case updateChatLastMessage::ID:
      func(static_cast<updateChatLastMessage &>(obj));
      return true;
    case updateChatPosition::ID:
      func(static_cast<updateChatPosition &>(obj));
      return true;
    case updateChatAddedToList::ID:
      func(static_cast<updateChatAddedToList &>(obj));
      return true;
    case updateChatRemovedFromList::ID:
      func(static_cast<updateChatRemovedFromList &>(obj));
      return true;
    case updateChatReadInbox::ID:
      func(static_cast<updateChatReadInbox &>(obj));
      return true;
    case updateChatReadOutbox::ID:
      func(static_cast<updateChatReadOutbox &>(obj));
      return true;
    case updateChatActionBar::ID:
      func(static_cast<updateChatActionBar &>(obj));
      return true;
    case updateChatBusinessBotManageBar::ID:
      func(static_cast<updateChatBusinessBotManageBar &>(obj));
      return true;
    case updateChatAvailableReactions::ID:
      func(static_cast<updateChatAvailableReactions &>(obj));
      return true;
    case updateChatDraftMessage::ID:
      func(static_cast<updateChatDraftMessage &>(obj));
      return true;
    case updateChatEmojiStatus::ID:
      func(static_cast<updateChatEmojiStatus &>(obj));
      return true;
    case updateChatMessageSender::ID:
      func(static_cast<updateChatMessageSender &>(obj));
      return true;
    case updateChatMessageAutoDeleteTime::ID:
      func(static_cast<updateChatMessageAutoDeleteTime &>(obj));
      return true;
    case updateChatNotificationSettings::ID:
      func(static_cast<updateChatNotificationSettings &>(obj));
      return true;
    case updateChatPendingJoinRequests::ID:
      func(static_cast<updateChatPendingJoinRequests &>(obj));
      return true;
    case updateChatReplyMarkup::ID:
      func(static_cast<updateChatReplyMarkup &>(obj));
      return true;
    case updateChatBackground::ID:
      func(static_cast<updateChatBackground &>(obj));
      return true;
    case updateChatTheme::ID:
      func(static_cast<updateChatTheme &>(obj));
      return true;
    case updateChatUnreadMentionCount::ID:
      func(static_cast<updateChatUnreadMentionCount &>(obj));
      return true;
    case updateChatUnreadReactionCount::ID:
      func(static_cast<updateChatUnreadReactionCount &>(obj));
      return true;
    case updateChatVideoChat::ID:
      func(static_cast<updateChatVideoChat &>(obj));
      return true;
    case updateChatDefaultDisableNotification::ID:
      func(static_cast<updateChatDefaultDisableNotification &>(obj));
      return true;
    case updateChatHasProtectedContent::ID:
      func(static_cast<updateChatHasProtectedContent &>(obj));
      return true;
    case updateChatIsTranslatable::ID:
      func(static_cast<updateChatIsTranslatable &>(obj));
      return true;
    case updateChatIsMarkedAsUnread::ID:
      func(static_cast<updateChatIsMarkedAsUnread &>(obj));
      return true;
    case updateChatViewAsTopics::ID:
      func(static_cast<updateChatViewAsTopics &>(obj));
      return true;
    case updateChatBlockList::ID:
      func(static_cast<updateChatBlockList &>(obj));
      return true;
    case updateChatHasScheduledMessages::ID:
      func(static_cast<updateChatHasScheduledMessages &>(obj));
      return true;
    case updateChatFolders::ID:
      func(static_cast<updateChatFolders &>(obj));
      return true;
    case updateChatOnlineMemberCount::ID:
      func(static_cast<updateChatOnlineMemberCount &>(obj));
      return true;
    case updateSavedMessagesTopic::ID:
      func(static_cast<updateSavedMessagesTopic &>(obj));
      return true;
    case updateSavedMessagesTopicCount::ID:
      func(static_cast<updateSavedMessagesTopicCount &>(obj));
      return true;
    case updateQuickReplyShortcut::ID:
      func(static_cast<updateQuickReplyShortcut &>(obj));
      return true;
    case updateQuickReplyShortcutDeleted::ID:
      func(static_cast<updateQuickReplyShortcutDeleted &>(obj));
      return true;
    case updateQuickReplyShortcuts::ID:
      func(static_cast<updateQuickReplyShortcuts &>(obj));
      return true;
    case updateQuickReplyShortcutMessages::ID:
      func(static_cast<updateQuickReplyShortcutMessages &>(obj));
      return true;
    case updateForumTopicInfo::ID:
      func(static_cast<updateForumTopicInfo &>(obj));
      return true;
    case updateScopeNotificationSettings::ID:
      func(static_cast<updateScopeNotificationSettings &>(obj));
      return true;
    case updateReactionNotificationSettings::ID:
      func(static_cast<updateReactionNotificationSettings &>(obj));
      return true;
    case updateNotification::ID:
      func(static_cast<updateNotification &>(obj));
      return true;
    case updateNotificationGroup::ID:
      func(static_cast<updateNotificationGroup &>(obj));
      return true;
    case updateActiveNotifications::ID:
      func(static_cast<updateActiveNotifications &>(obj));
      return true;
    case updateHavePendingNotifications::ID:
      func(static_cast<updateHavePendingNotifications &>(obj));
      return true;
    case updateDeleteMessages::ID:
      func(static_cast<updateDeleteMessages &>(obj));
      return true;
    case updateChatAction::ID:
      func(static_cast<updateChatAction &>(obj));
      return true;
    case updateUserStatus::ID:
      func(static_cast<updateUserStatus &>(obj));
      return true;
    case updateUser::ID:
      func(static_cast<updateUser &>(obj));
      return true;
    case updateBasicGroup::ID:
      func(static_cast<updateBasicGroup &>(obj));
      return true;
    case updateSupergroup::ID:
      func(static_cast<updateSupergroup &>(obj));
      return true;
    case updateSecretChat::ID:
      func(static_cast<updateSecretChat &>(obj));
      return true;
    case updateUserFullInfo::ID:
      func(static_cast<updateUserFullInfo &>(obj));
      return true;
    case updateBasicGroupFullInfo::ID:
      func(static_cast<updateBasicGroupFullInfo &>(obj));
      return true;
    case updateSupergroupFullInfo::ID:
      func(static_cast<updateSupergroupFullInfo &>(obj));
      return true;
    case updateServiceNotification::ID:
      func(static_cast<updateServiceNotification &>(obj));
      return true;
    case updateFile::ID:
      func(static_cast<updateFile &>(obj));
      return true;
    case updateFileGenerationStart::ID:
      func(static_cast<updateFileGenerationStart &>(obj));
      return true;
    case updateFileGenerationStop::ID:
      func(static_cast<updateFileGenerationStop &>(obj));
      return true;
    case updateFileDownloads::ID:
      func(static_cast<updateFileDownloads &>(obj));
      return true;
    case updateFileAddedToDownloads::ID:
      func(static_cast<updateFileAddedToDownloads &>(obj));
      return true;
    case updateFileDownload::ID:
      func(static_cast<updateFileDownload &>(obj));
      return true;
    case updateFileRemovedFromDownloads::ID:
      func(static_cast<updateFileRemovedFromDownloads &>(obj));
      return true;
    case updateApplicationVerificationRequired::ID:
      func(static_cast<updateApplicationVerificationRequired &>(obj));
      return true;
    case updateCall::ID:
      func(static_cast<updateCall &>(obj));
      return true;
    case updateGroupCall::ID:
      func(static_cast<updateGroupCall &>(obj));
      return true;
    case updateGroupCallParticipant::ID:
      func(static_cast<updateGroupCallParticipant &>(obj));
      return true;
    case updateNewCallSignalingData::ID:
      func(static_cast<updateNewCallSignalingData &>(obj));
      return true;
    case updateUserPrivacySettingRules::ID:
      func(static_cast<updateUserPrivacySettingRules &>(obj));
      return true;
    case updateUnreadMessageCount::ID:
      func(static_cast<updateUnreadMessageCount &>(obj));
      return true;
    case updateUnreadChatCount::ID:
      func(static_cast<updateUnreadChatCount &>(obj));
      return true;
    case updateStory::ID:
      func(static_cast<updateStory &>(obj));
      return true;
    case updateStoryDeleted::ID:
      func(static_cast<updateStoryDeleted &>(obj));
      return true;
    case updateStorySendSucceeded::ID:
      func(static_cast<updateStorySendSucceeded &>(obj));
      return true;
    case updateStorySendFailed::ID:
      func(static_cast<updateStorySendFailed &>(obj));
      return true;
    case updateChatActiveStories::ID:
      func(static_cast<updateChatActiveStories &>(obj));
      return true;
    case updateStoryListChatCount::ID:
      func(static_cast<updateStoryListChatCount &>(obj));
      return true;
    case updateStoryStealthMode::ID:
      func(static_cast<updateStoryStealthMode &>(obj));
      return true;
    case updateOption::ID:
      func(static_cast<updateOption &>(obj));
      return true;
    case updateStickerSet::ID:
      func(static_cast<updateStickerSet &>(obj));
      return true;
    case updateInstalledStickerSets::ID:
      func(static_cast<updateInstalledStickerSets &>(obj));
      return true;
    case updateTrendingStickerSets::ID:
      func(static_cast<updateTrendingStickerSets &>(obj));
      return true;
    case updateRecentStickers::ID:
      func(static_cast<updateRecentStickers &>(obj));
      return true;
    case updateFavoriteStickers::ID:
      func(static_cast<updateFavoriteStickers &>(obj));
      return true;
    case updateSavedAnimations::ID:
      func(static_cast<updateSavedAnimations &>(obj));
      return true;
    case updateSavedNotificationSounds::ID:
      func(static_cast<updateSavedNotificationSounds &>(obj));
      return true;
    case updateDefaultBackground::ID:
      func(static_cast<updateDefaultBackground &>(obj));
      return true;
    case updateChatThemes::ID:
      func(static_cast<updateChatThemes &>(obj));
      return true;
    case updateAccentColors::ID:
      func(static_cast<updateAccentColors &>(obj));
      return true;
    case updateProfileAccentColors::ID:
      func(static_cast<updateProfileAccentColors &>(obj));
      return true;
    case updateLanguagePackStrings::ID:
      func(static_cast<updateLanguagePackStrings &>(obj));
      return true;
    case updateConnectionState::ID:
      func(static_cast<updateConnectionState &>(obj));
      return true;
    case updateTermsOfService::ID:
      func(static_cast<updateTermsOfService &>(obj));
      return true;
    case updateUsersNearby::ID:
      func(static_cast<updateUsersNearby &>(obj));
      return true;
    case updateUnconfirmedSession::ID:
      func(static_cast<updateUnconfirmedSession &>(obj));
      return true;
    case updateAttachmentMenuBots::ID:
      func(static_cast<updateAttachmentMenuBots &>(obj));
      return true;
    case updateWebAppMessageSent::ID:
      func(static_cast<updateWebAppMessageSent &>(obj));
      return true;
    case updateActiveEmojiReactions::ID:
      func(static_cast<updateActiveEmojiReactions &>(obj));
      return true;
    case updateAvailableMessageEffects::ID:
      func(static_cast<updateAvailableMessageEffects &>(obj));
      return true;
    case updateDefaultReactionType::ID:
      func(static_cast<updateDefaultReactionType &>(obj));
      return true;
    case updateSavedMessagesTags::ID:
      func(static_cast<updateSavedMessagesTags &>(obj));
      return true;
    case updateActiveLiveLocationMessages::ID:
      func(static_cast<updateActiveLiveLocationMessages &>(obj));
      return true;
    case updateOwnedStarCount::ID:
      func(static_cast<updateOwnedStarCount &>(obj));
      return true;
    case updateChatRevenueAmount::ID:
      func(static_cast<updateChatRevenueAmount &>(obj));
      return true;
    case updateStarRevenueStatus::ID:
      func(static_cast<updateStarRevenueStatus &>(obj));
      return true;
    case updateSpeechRecognitionTrial::ID:
      func(static_cast<updateSpeechRecognitionTrial &>(obj));
      return true;
    case updateDiceEmojis::ID:
      func(static_cast<updateDiceEmojis &>(obj));
      return true;
    case updateAnimatedEmojiMessageClicked::ID:
      func(static_cast<updateAnimatedEmojiMessageClicked &>(obj));
      return true;
    case updateAnimationSearchParameters::ID:
      func(static_cast<updateAnimationSearchParameters &>(obj));
      return true;
    case updateSuggestedActions::ID:
      func(static_cast<updateSuggestedActions &>(obj));
      return true;
    case updateSpeedLimitNotification::ID:
      func(static_cast<updateSpeedLimitNotification &>(obj));
      return true;
    case updateContactCloseBirthdays::ID:
      func(static_cast<updateContactCloseBirthdays &>(obj));
      return true;
    case updateAutosaveSettings::ID:
      func(static_cast<updateAutosaveSettings &>(obj));
      return true;
    case updateBusinessConnection::ID:
      func(static_cast<updateBusinessConnection &>(obj));
      return true;
    case updateNewBusinessMessage::ID:
      func(static_cast<updateNewBusinessMessage &>(obj));
      return true;
    case updateBusinessMessageEdited::ID:
      func(static_cast<updateBusinessMessageEdited &>(obj));
      return true;
    case updateBusinessMessagesDeleted::ID:
      func(static_cast<updateBusinessMessagesDeleted &>(obj));
      return true;
    case updateNewInlineQuery::ID:
      func(static_cast<updateNewInlineQuery &>(obj));
      return true;
    case updateNewChosenInlineResult::ID:
      func(static_cast<updateNewChosenInlineResult &>(obj));
      return true;
    case updateNewCallbackQuery::ID:
      func(static_cast<updateNewCallbackQuery &>(obj));
      return true;
    case updateNewInlineCallbackQuery::ID:
      func(static_cast<updateNewInlineCallbackQuery &>(obj));
      return true;
    case updateNewBusinessCallbackQuery::ID:
      func(static_cast<updateNewBusinessCallbackQuery &>(obj));
      return true;
    case updateNewShippingQuery::ID:
      func(static_cast<updateNewShippingQuery &>(obj));
      return true;
    case updateNewPreCheckoutQuery::ID:
      func(static_cast<updateNewPreCheckoutQuery &>(obj));
      return true;
    case updateNewCustomEvent::ID:
      func(static_cast<updateNewCustomEvent &>(obj));
      return true;
    case updateNewCustomQuery::ID:
      func(static_cast<updateNewCustomQuery &>(obj));
      return true;
    case updatePoll::ID:
      func(static_cast<updatePoll &>(obj));
      return true;
    case updatePollAnswer::ID:
      func(static_cast<updatePollAnswer &>(obj));
      return true;
    case updateChatMember::ID:
      func(static_cast<updateChatMember &>(obj));
      return true;
    case updateNewChatJoinRequest::ID:
      func(static_cast<updateNewChatJoinRequest &>(obj));
      return true;
    case updateChatBoost::ID:
      func(static_cast<updateChatBoost &>(obj));
      return true;
    case updateMessageReaction::ID:
      func(static_cast<updateMessageReaction &>(obj));
      return true;
    case updateMessageReactions::ID:
      func(static_cast<updateMessageReactions &>(obj));
      return true;
    case updatePaidMediaPurchased::ID:
      func(static_cast<updatePaidMediaPurchased &>(obj));
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
bool downcast_call(UserPrivacySetting &obj, const T &func) {
  switch (obj.get_id()) {
    case userPrivacySettingShowStatus::ID:
      func(static_cast<userPrivacySettingShowStatus &>(obj));
      return true;
    case userPrivacySettingShowProfilePhoto::ID:
      func(static_cast<userPrivacySettingShowProfilePhoto &>(obj));
      return true;
    case userPrivacySettingShowLinkInForwardedMessages::ID:
      func(static_cast<userPrivacySettingShowLinkInForwardedMessages &>(obj));
      return true;
    case userPrivacySettingShowPhoneNumber::ID:
      func(static_cast<userPrivacySettingShowPhoneNumber &>(obj));
      return true;
    case userPrivacySettingShowBio::ID:
      func(static_cast<userPrivacySettingShowBio &>(obj));
      return true;
    case userPrivacySettingShowBirthdate::ID:
      func(static_cast<userPrivacySettingShowBirthdate &>(obj));
      return true;
    case userPrivacySettingAllowChatInvites::ID:
      func(static_cast<userPrivacySettingAllowChatInvites &>(obj));
      return true;
    case userPrivacySettingAllowCalls::ID:
      func(static_cast<userPrivacySettingAllowCalls &>(obj));
      return true;
    case userPrivacySettingAllowPeerToPeerCalls::ID:
      func(static_cast<userPrivacySettingAllowPeerToPeerCalls &>(obj));
      return true;
    case userPrivacySettingAllowFindingByPhoneNumber::ID:
      func(static_cast<userPrivacySettingAllowFindingByPhoneNumber &>(obj));
      return true;
    case userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages::ID:
      func(static_cast<userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &>(obj));
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
bool downcast_call(UserPrivacySettingRule &obj, const T &func) {
  switch (obj.get_id()) {
    case userPrivacySettingRuleAllowAll::ID:
      func(static_cast<userPrivacySettingRuleAllowAll &>(obj));
      return true;
    case userPrivacySettingRuleAllowContacts::ID:
      func(static_cast<userPrivacySettingRuleAllowContacts &>(obj));
      return true;
    case userPrivacySettingRuleAllowPremiumUsers::ID:
      func(static_cast<userPrivacySettingRuleAllowPremiumUsers &>(obj));
      return true;
    case userPrivacySettingRuleAllowUsers::ID:
      func(static_cast<userPrivacySettingRuleAllowUsers &>(obj));
      return true;
    case userPrivacySettingRuleAllowChatMembers::ID:
      func(static_cast<userPrivacySettingRuleAllowChatMembers &>(obj));
      return true;
    case userPrivacySettingRuleRestrictAll::ID:
      func(static_cast<userPrivacySettingRuleRestrictAll &>(obj));
      return true;
    case userPrivacySettingRuleRestrictContacts::ID:
      func(static_cast<userPrivacySettingRuleRestrictContacts &>(obj));
      return true;
    case userPrivacySettingRuleRestrictUsers::ID:
      func(static_cast<userPrivacySettingRuleRestrictUsers &>(obj));
      return true;
    case userPrivacySettingRuleRestrictChatMembers::ID:
      func(static_cast<userPrivacySettingRuleRestrictChatMembers &>(obj));
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
bool downcast_call(UserStatus &obj, const T &func) {
  switch (obj.get_id()) {
    case userStatusEmpty::ID:
      func(static_cast<userStatusEmpty &>(obj));
      return true;
    case userStatusOnline::ID:
      func(static_cast<userStatusOnline &>(obj));
      return true;
    case userStatusOffline::ID:
      func(static_cast<userStatusOffline &>(obj));
      return true;
    case userStatusRecently::ID:
      func(static_cast<userStatusRecently &>(obj));
      return true;
    case userStatusLastWeek::ID:
      func(static_cast<userStatusLastWeek &>(obj));
      return true;
    case userStatusLastMonth::ID:
      func(static_cast<userStatusLastMonth &>(obj));
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
bool downcast_call(UserType &obj, const T &func) {
  switch (obj.get_id()) {
    case userTypeRegular::ID:
      func(static_cast<userTypeRegular &>(obj));
      return true;
    case userTypeDeleted::ID:
      func(static_cast<userTypeDeleted &>(obj));
      return true;
    case userTypeBot::ID:
      func(static_cast<userTypeBot &>(obj));
      return true;
    case userTypeUnknown::ID:
      func(static_cast<userTypeUnknown &>(obj));
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
bool downcast_call(VectorPathCommand &obj, const T &func) {
  switch (obj.get_id()) {
    case vectorPathCommandLine::ID:
      func(static_cast<vectorPathCommandLine &>(obj));
      return true;
    case vectorPathCommandCubicBezierCurve::ID:
      func(static_cast<vectorPathCommandCubicBezierCurve &>(obj));
      return true;
    default:
      return false;
  }
}

}  // namespace td_api
}  // namespace td
