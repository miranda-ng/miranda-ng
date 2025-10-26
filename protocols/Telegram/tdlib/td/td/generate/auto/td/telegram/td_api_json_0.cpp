#include "td/telegram/td_api_json.h"

#include "td/telegram/td_api.h"
#include "td/telegram/td_api.hpp"

#include "td/tl/tl_json.h"

#include "td/utils/base64.h"
#include "td/utils/common.h"
#include "td/utils/FlatHashMap.h"
#include "td/utils/Slice.h"

namespace td {
namespace td_api {

void to_json(JsonValueScope &jv, const td_api::object_ptr<Object> &value) {
  td::to_json(jv, value);
}

Status from_json(td_api::object_ptr<Function> &to, td::JsonValue from) {
  return td::from_json(to, std::move(from));
}

void to_json(JsonValueScope &jv, const Object &object) {
  switch (object.get_id()) {
    case td_api::accentColor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::accentColor &)>(td_api::to_json)(jv, static_cast<const td_api::accentColor &>(object));
    case td_api::acceptedGiftTypes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::acceptedGiftTypes &)>(td_api::to_json)(jv, static_cast<const td_api::acceptedGiftTypes &>(object));
    case td_api::accountInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::accountInfo &)>(td_api::to_json)(jv, static_cast<const td_api::accountInfo &>(object));
    case td_api::accountTtl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::accountTtl &)>(td_api::to_json)(jv, static_cast<const td_api::accountTtl &>(object));
    case td_api::addedReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::addedReaction &)>(td_api::to_json)(jv, static_cast<const td_api::addedReaction &>(object));
    case td_api::addedReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::addedReactions &)>(td_api::to_json)(jv, static_cast<const td_api::addedReactions &>(object));
    case td_api::address::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::address &)>(td_api::to_json)(jv, static_cast<const td_api::address &>(object));
    case td_api::advertisementSponsor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::advertisementSponsor &)>(td_api::to_json)(jv, static_cast<const td_api::advertisementSponsor &>(object));
    case td_api::affiliateInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::affiliateInfo &)>(td_api::to_json)(jv, static_cast<const td_api::affiliateInfo &>(object));
    case td_api::affiliateProgramInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::affiliateProgramInfo &)>(td_api::to_json)(jv, static_cast<const td_api::affiliateProgramInfo &>(object));
    case td_api::affiliateProgramParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::affiliateProgramParameters &)>(td_api::to_json)(jv, static_cast<const td_api::affiliateProgramParameters &>(object));
    case td_api::ageVerificationParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::ageVerificationParameters &)>(td_api::to_json)(jv, static_cast<const td_api::ageVerificationParameters &>(object));
    case td_api::alternativeVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::alternativeVideo &)>(td_api::to_json)(jv, static_cast<const td_api::alternativeVideo &>(object));
    case td_api::animatedChatPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::animatedChatPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::animatedChatPhoto &>(object));
    case td_api::animatedEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::animatedEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::animatedEmoji &>(object));
    case td_api::animation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::animation &)>(td_api::to_json)(jv, static_cast<const td_api::animation &>(object));
    case td_api::animations::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::animations &)>(td_api::to_json)(jv, static_cast<const td_api::animations &>(object));
    case td_api::archiveChatListSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::archiveChatListSettings &)>(td_api::to_json)(jv, static_cast<const td_api::archiveChatListSettings &>(object));
    case td_api::attachmentMenuBot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::attachmentMenuBot &)>(td_api::to_json)(jv, static_cast<const td_api::attachmentMenuBot &>(object));
    case td_api::attachmentMenuBotColor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::attachmentMenuBotColor &)>(td_api::to_json)(jv, static_cast<const td_api::attachmentMenuBotColor &>(object));
    case td_api::audio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::audio &)>(td_api::to_json)(jv, static_cast<const td_api::audio &>(object));
    case td_api::audios::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::audios &)>(td_api::to_json)(jv, static_cast<const td_api::audios &>(object));
    case td_api::authenticationCodeInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeInfo &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeInfo &>(object));
    case td_api::authenticationCodeTypeCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeCall &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeCall &>(object));
    case td_api::authenticationCodeTypeFirebaseAndroid::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeFirebaseAndroid &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeFirebaseAndroid &>(object));
    case td_api::authenticationCodeTypeFirebaseIos::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeFirebaseIos &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeFirebaseIos &>(object));
    case td_api::authenticationCodeTypeFlashCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeFlashCall &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeFlashCall &>(object));
    case td_api::authenticationCodeTypeFragment::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeFragment &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeFragment &>(object));
    case td_api::authenticationCodeTypeMissedCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeMissedCall &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeMissedCall &>(object));
    case td_api::authenticationCodeTypeSms::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeSms &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeSms &>(object));
    case td_api::authenticationCodeTypeSmsPhrase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeSmsPhrase &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeSmsPhrase &>(object));
    case td_api::authenticationCodeTypeSmsWord::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeSmsWord &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeSmsWord &>(object));
    case td_api::authenticationCodeTypeTelegramMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authenticationCodeTypeTelegramMessage &)>(td_api::to_json)(jv, static_cast<const td_api::authenticationCodeTypeTelegramMessage &>(object));
    case td_api::authorizationStateClosed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateClosed &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateClosed &>(object));
    case td_api::authorizationStateClosing::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateClosing &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateClosing &>(object));
    case td_api::authorizationStateLoggingOut::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateLoggingOut &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateLoggingOut &>(object));
    case td_api::authorizationStateReady::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateReady &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateReady &>(object));
    case td_api::authorizationStateWaitCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitCode &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitCode &>(object));
    case td_api::authorizationStateWaitEmailAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitEmailAddress &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitEmailAddress &>(object));
    case td_api::authorizationStateWaitEmailCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitEmailCode &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitEmailCode &>(object));
    case td_api::authorizationStateWaitOtherDeviceConfirmation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitOtherDeviceConfirmation &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitOtherDeviceConfirmation &>(object));
    case td_api::authorizationStateWaitPassword::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitPassword &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitPassword &>(object));
    case td_api::authorizationStateWaitPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitPhoneNumber &>(object));
    case td_api::authorizationStateWaitPremiumPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitPremiumPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitPremiumPurchase &>(object));
    case td_api::authorizationStateWaitRegistration::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitRegistration &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitRegistration &>(object));
    case td_api::authorizationStateWaitTdlibParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::authorizationStateWaitTdlibParameters &)>(td_api::to_json)(jv, static_cast<const td_api::authorizationStateWaitTdlibParameters &>(object));
    case td_api::autoDownloadSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autoDownloadSettings &)>(td_api::to_json)(jv, static_cast<const td_api::autoDownloadSettings &>(object));
    case td_api::autoDownloadSettingsPresets::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autoDownloadSettingsPresets &)>(td_api::to_json)(jv, static_cast<const td_api::autoDownloadSettingsPresets &>(object));
    case td_api::autosaveSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autosaveSettings &)>(td_api::to_json)(jv, static_cast<const td_api::autosaveSettings &>(object));
    case td_api::autosaveSettingsException::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autosaveSettingsException &)>(td_api::to_json)(jv, static_cast<const td_api::autosaveSettingsException &>(object));
    case td_api::autosaveSettingsScopeChannelChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autosaveSettingsScopeChannelChats &)>(td_api::to_json)(jv, static_cast<const td_api::autosaveSettingsScopeChannelChats &>(object));
    case td_api::autosaveSettingsScopeChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autosaveSettingsScopeChat &)>(td_api::to_json)(jv, static_cast<const td_api::autosaveSettingsScopeChat &>(object));
    case td_api::autosaveSettingsScopeGroupChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autosaveSettingsScopeGroupChats &)>(td_api::to_json)(jv, static_cast<const td_api::autosaveSettingsScopeGroupChats &>(object));
    case td_api::autosaveSettingsScopePrivateChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::autosaveSettingsScopePrivateChats &)>(td_api::to_json)(jv, static_cast<const td_api::autosaveSettingsScopePrivateChats &>(object));
    case td_api::availableGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::availableGift &)>(td_api::to_json)(jv, static_cast<const td_api::availableGift &>(object));
    case td_api::availableGifts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::availableGifts &)>(td_api::to_json)(jv, static_cast<const td_api::availableGifts &>(object));
    case td_api::availableReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::availableReaction &)>(td_api::to_json)(jv, static_cast<const td_api::availableReaction &>(object));
    case td_api::availableReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::availableReactions &)>(td_api::to_json)(jv, static_cast<const td_api::availableReactions &>(object));
    case td_api::background::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::background &)>(td_api::to_json)(jv, static_cast<const td_api::background &>(object));
    case td_api::backgroundFillFreeformGradient::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgroundFillFreeformGradient &)>(td_api::to_json)(jv, static_cast<const td_api::backgroundFillFreeformGradient &>(object));
    case td_api::backgroundFillGradient::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgroundFillGradient &)>(td_api::to_json)(jv, static_cast<const td_api::backgroundFillGradient &>(object));
    case td_api::backgroundFillSolid::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgroundFillSolid &)>(td_api::to_json)(jv, static_cast<const td_api::backgroundFillSolid &>(object));
    case td_api::backgroundTypeChatTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgroundTypeChatTheme &)>(td_api::to_json)(jv, static_cast<const td_api::backgroundTypeChatTheme &>(object));
    case td_api::backgroundTypeFill::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgroundTypeFill &)>(td_api::to_json)(jv, static_cast<const td_api::backgroundTypeFill &>(object));
    case td_api::backgroundTypePattern::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgroundTypePattern &)>(td_api::to_json)(jv, static_cast<const td_api::backgroundTypePattern &>(object));
    case td_api::backgroundTypeWallpaper::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgroundTypeWallpaper &)>(td_api::to_json)(jv, static_cast<const td_api::backgroundTypeWallpaper &>(object));
    case td_api::backgrounds::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::backgrounds &)>(td_api::to_json)(jv, static_cast<const td_api::backgrounds &>(object));
    case td_api::bankCardActionOpenUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::bankCardActionOpenUrl &)>(td_api::to_json)(jv, static_cast<const td_api::bankCardActionOpenUrl &>(object));
    case td_api::bankCardInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::bankCardInfo &)>(td_api::to_json)(jv, static_cast<const td_api::bankCardInfo &>(object));
    case td_api::basicGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::basicGroup &)>(td_api::to_json)(jv, static_cast<const td_api::basicGroup &>(object));
    case td_api::basicGroupFullInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::basicGroupFullInfo &)>(td_api::to_json)(jv, static_cast<const td_api::basicGroupFullInfo &>(object));
    case td_api::birthdate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::birthdate &)>(td_api::to_json)(jv, static_cast<const td_api::birthdate &>(object));
    case td_api::blockListMain::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::blockListMain &)>(td_api::to_json)(jv, static_cast<const td_api::blockListMain &>(object));
    case td_api::blockListStories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::blockListStories &)>(td_api::to_json)(jv, static_cast<const td_api::blockListStories &>(object));
    case td_api::botCommand::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botCommand &)>(td_api::to_json)(jv, static_cast<const td_api::botCommand &>(object));
    case td_api::botCommands::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botCommands &)>(td_api::to_json)(jv, static_cast<const td_api::botCommands &>(object));
    case td_api::botInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botInfo &)>(td_api::to_json)(jv, static_cast<const td_api::botInfo &>(object));
    case td_api::botMediaPreview::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botMediaPreview &)>(td_api::to_json)(jv, static_cast<const td_api::botMediaPreview &>(object));
    case td_api::botMediaPreviewInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botMediaPreviewInfo &)>(td_api::to_json)(jv, static_cast<const td_api::botMediaPreviewInfo &>(object));
    case td_api::botMediaPreviews::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botMediaPreviews &)>(td_api::to_json)(jv, static_cast<const td_api::botMediaPreviews &>(object));
    case td_api::botMenuButton::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botMenuButton &)>(td_api::to_json)(jv, static_cast<const td_api::botMenuButton &>(object));
    case td_api::botVerification::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botVerification &)>(td_api::to_json)(jv, static_cast<const td_api::botVerification &>(object));
    case td_api::botVerificationParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botVerificationParameters &)>(td_api::to_json)(jv, static_cast<const td_api::botVerificationParameters &>(object));
    case td_api::botWriteAccessAllowReasonAcceptedRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botWriteAccessAllowReasonAcceptedRequest &)>(td_api::to_json)(jv, static_cast<const td_api::botWriteAccessAllowReasonAcceptedRequest &>(object));
    case td_api::botWriteAccessAllowReasonAddedToAttachmentMenu::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botWriteAccessAllowReasonAddedToAttachmentMenu &)>(td_api::to_json)(jv, static_cast<const td_api::botWriteAccessAllowReasonAddedToAttachmentMenu &>(object));
    case td_api::botWriteAccessAllowReasonConnectedWebsite::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botWriteAccessAllowReasonConnectedWebsite &)>(td_api::to_json)(jv, static_cast<const td_api::botWriteAccessAllowReasonConnectedWebsite &>(object));
    case td_api::botWriteAccessAllowReasonLaunchedWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::botWriteAccessAllowReasonLaunchedWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::botWriteAccessAllowReasonLaunchedWebApp &>(object));
    case td_api::builtInThemeArctic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::builtInThemeArctic &)>(td_api::to_json)(jv, static_cast<const td_api::builtInThemeArctic &>(object));
    case td_api::builtInThemeClassic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::builtInThemeClassic &)>(td_api::to_json)(jv, static_cast<const td_api::builtInThemeClassic &>(object));
    case td_api::builtInThemeDay::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::builtInThemeDay &)>(td_api::to_json)(jv, static_cast<const td_api::builtInThemeDay &>(object));
    case td_api::builtInThemeNight::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::builtInThemeNight &)>(td_api::to_json)(jv, static_cast<const td_api::builtInThemeNight &>(object));
    case td_api::builtInThemeTinted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::builtInThemeTinted &)>(td_api::to_json)(jv, static_cast<const td_api::builtInThemeTinted &>(object));
    case td_api::businessAwayMessageScheduleAlways::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessAwayMessageScheduleAlways &)>(td_api::to_json)(jv, static_cast<const td_api::businessAwayMessageScheduleAlways &>(object));
    case td_api::businessAwayMessageScheduleCustom::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessAwayMessageScheduleCustom &)>(td_api::to_json)(jv, static_cast<const td_api::businessAwayMessageScheduleCustom &>(object));
    case td_api::businessAwayMessageScheduleOutsideOfOpeningHours::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessAwayMessageScheduleOutsideOfOpeningHours &)>(td_api::to_json)(jv, static_cast<const td_api::businessAwayMessageScheduleOutsideOfOpeningHours &>(object));
    case td_api::businessAwayMessageSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessAwayMessageSettings &)>(td_api::to_json)(jv, static_cast<const td_api::businessAwayMessageSettings &>(object));
    case td_api::businessBotManageBar::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessBotManageBar &)>(td_api::to_json)(jv, static_cast<const td_api::businessBotManageBar &>(object));
    case td_api::businessBotRights::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessBotRights &)>(td_api::to_json)(jv, static_cast<const td_api::businessBotRights &>(object));
    case td_api::businessChatLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessChatLink &)>(td_api::to_json)(jv, static_cast<const td_api::businessChatLink &>(object));
    case td_api::businessChatLinkInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessChatLinkInfo &)>(td_api::to_json)(jv, static_cast<const td_api::businessChatLinkInfo &>(object));
    case td_api::businessChatLinks::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessChatLinks &)>(td_api::to_json)(jv, static_cast<const td_api::businessChatLinks &>(object));
    case td_api::businessConnectedBot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessConnectedBot &)>(td_api::to_json)(jv, static_cast<const td_api::businessConnectedBot &>(object));
    case td_api::businessConnection::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessConnection &)>(td_api::to_json)(jv, static_cast<const td_api::businessConnection &>(object));
    case td_api::businessFeatureAccountLinks::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureAccountLinks &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureAccountLinks &>(object));
    case td_api::businessFeatureAwayMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureAwayMessage &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureAwayMessage &>(object));
    case td_api::businessFeatureBots::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureBots &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureBots &>(object));
    case td_api::businessFeatureChatFolderTags::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureChatFolderTags &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureChatFolderTags &>(object));
    case td_api::businessFeatureEmojiStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureEmojiStatus &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureEmojiStatus &>(object));
    case td_api::businessFeatureGreetingMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureGreetingMessage &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureGreetingMessage &>(object));
    case td_api::businessFeatureLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureLocation &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureLocation &>(object));
    case td_api::businessFeatureOpeningHours::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureOpeningHours &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureOpeningHours &>(object));
    case td_api::businessFeaturePromotionAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeaturePromotionAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeaturePromotionAnimation &>(object));
    case td_api::businessFeatureQuickReplies::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureQuickReplies &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureQuickReplies &>(object));
    case td_api::businessFeatureStartPage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureStartPage &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureStartPage &>(object));
    case td_api::businessFeatureUpgradedStories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatureUpgradedStories &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatureUpgradedStories &>(object));
    case td_api::businessFeatures::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessFeatures &)>(td_api::to_json)(jv, static_cast<const td_api::businessFeatures &>(object));
    case td_api::businessGreetingMessageSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessGreetingMessageSettings &)>(td_api::to_json)(jv, static_cast<const td_api::businessGreetingMessageSettings &>(object));
    case td_api::businessInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessInfo &)>(td_api::to_json)(jv, static_cast<const td_api::businessInfo &>(object));
    case td_api::businessLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessLocation &)>(td_api::to_json)(jv, static_cast<const td_api::businessLocation &>(object));
    case td_api::businessMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessMessage &)>(td_api::to_json)(jv, static_cast<const td_api::businessMessage &>(object));
    case td_api::businessMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessMessages &)>(td_api::to_json)(jv, static_cast<const td_api::businessMessages &>(object));
    case td_api::businessOpeningHours::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessOpeningHours &)>(td_api::to_json)(jv, static_cast<const td_api::businessOpeningHours &>(object));
    case td_api::businessOpeningHoursInterval::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessOpeningHoursInterval &)>(td_api::to_json)(jv, static_cast<const td_api::businessOpeningHoursInterval &>(object));
    case td_api::businessRecipients::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessRecipients &)>(td_api::to_json)(jv, static_cast<const td_api::businessRecipients &>(object));
    case td_api::businessStartPage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::businessStartPage &)>(td_api::to_json)(jv, static_cast<const td_api::businessStartPage &>(object));
    case td_api::call::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::call &)>(td_api::to_json)(jv, static_cast<const td_api::call &>(object));
    case td_api::callDiscardReasonDeclined::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callDiscardReasonDeclined &)>(td_api::to_json)(jv, static_cast<const td_api::callDiscardReasonDeclined &>(object));
    case td_api::callDiscardReasonDisconnected::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callDiscardReasonDisconnected &)>(td_api::to_json)(jv, static_cast<const td_api::callDiscardReasonDisconnected &>(object));
    case td_api::callDiscardReasonEmpty::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callDiscardReasonEmpty &)>(td_api::to_json)(jv, static_cast<const td_api::callDiscardReasonEmpty &>(object));
    case td_api::callDiscardReasonHungUp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callDiscardReasonHungUp &)>(td_api::to_json)(jv, static_cast<const td_api::callDiscardReasonHungUp &>(object));
    case td_api::callDiscardReasonMissed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callDiscardReasonMissed &)>(td_api::to_json)(jv, static_cast<const td_api::callDiscardReasonMissed &>(object));
    case td_api::callDiscardReasonUpgradeToGroupCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callDiscardReasonUpgradeToGroupCall &)>(td_api::to_json)(jv, static_cast<const td_api::callDiscardReasonUpgradeToGroupCall &>(object));
    case td_api::callId::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callId &)>(td_api::to_json)(jv, static_cast<const td_api::callId &>(object));
    case td_api::callProtocol::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callProtocol &)>(td_api::to_json)(jv, static_cast<const td_api::callProtocol &>(object));
    case td_api::callServer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callServer &)>(td_api::to_json)(jv, static_cast<const td_api::callServer &>(object));
    case td_api::callServerTypeTelegramReflector::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callServerTypeTelegramReflector &)>(td_api::to_json)(jv, static_cast<const td_api::callServerTypeTelegramReflector &>(object));
    case td_api::callServerTypeWebrtc::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callServerTypeWebrtc &)>(td_api::to_json)(jv, static_cast<const td_api::callServerTypeWebrtc &>(object));
    case td_api::callStateDiscarded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callStateDiscarded &)>(td_api::to_json)(jv, static_cast<const td_api::callStateDiscarded &>(object));
    case td_api::callStateError::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callStateError &)>(td_api::to_json)(jv, static_cast<const td_api::callStateError &>(object));
    case td_api::callStateExchangingKeys::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callStateExchangingKeys &)>(td_api::to_json)(jv, static_cast<const td_api::callStateExchangingKeys &>(object));
    case td_api::callStateHangingUp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callStateHangingUp &)>(td_api::to_json)(jv, static_cast<const td_api::callStateHangingUp &>(object));
    case td_api::callStatePending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callStatePending &)>(td_api::to_json)(jv, static_cast<const td_api::callStatePending &>(object));
    case td_api::callStateReady::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callStateReady &)>(td_api::to_json)(jv, static_cast<const td_api::callStateReady &>(object));
    case td_api::callbackQueryAnswer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callbackQueryAnswer &)>(td_api::to_json)(jv, static_cast<const td_api::callbackQueryAnswer &>(object));
    case td_api::callbackQueryPayloadData::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callbackQueryPayloadData &)>(td_api::to_json)(jv, static_cast<const td_api::callbackQueryPayloadData &>(object));
    case td_api::callbackQueryPayloadDataWithPassword::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callbackQueryPayloadDataWithPassword &)>(td_api::to_json)(jv, static_cast<const td_api::callbackQueryPayloadDataWithPassword &>(object));
    case td_api::callbackQueryPayloadGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::callbackQueryPayloadGame &)>(td_api::to_json)(jv, static_cast<const td_api::callbackQueryPayloadGame &>(object));
    case td_api::canPostStoryResultActiveStoryLimitExceeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canPostStoryResultActiveStoryLimitExceeded &)>(td_api::to_json)(jv, static_cast<const td_api::canPostStoryResultActiveStoryLimitExceeded &>(object));
    case td_api::canPostStoryResultBoostNeeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canPostStoryResultBoostNeeded &)>(td_api::to_json)(jv, static_cast<const td_api::canPostStoryResultBoostNeeded &>(object));
    case td_api::canPostStoryResultMonthlyLimitExceeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canPostStoryResultMonthlyLimitExceeded &)>(td_api::to_json)(jv, static_cast<const td_api::canPostStoryResultMonthlyLimitExceeded &>(object));
    case td_api::canPostStoryResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canPostStoryResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::canPostStoryResultOk &>(object));
    case td_api::canPostStoryResultPremiumNeeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canPostStoryResultPremiumNeeded &)>(td_api::to_json)(jv, static_cast<const td_api::canPostStoryResultPremiumNeeded &>(object));
    case td_api::canPostStoryResultWeeklyLimitExceeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canPostStoryResultWeeklyLimitExceeded &)>(td_api::to_json)(jv, static_cast<const td_api::canPostStoryResultWeeklyLimitExceeded &>(object));
    case td_api::canSendGiftResultFail::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canSendGiftResultFail &)>(td_api::to_json)(jv, static_cast<const td_api::canSendGiftResultFail &>(object));
    case td_api::canSendGiftResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canSendGiftResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::canSendGiftResultOk &>(object));
    case td_api::canSendMessageToUserResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canSendMessageToUserResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::canSendMessageToUserResultOk &>(object));
    case td_api::canSendMessageToUserResultUserHasPaidMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canSendMessageToUserResultUserHasPaidMessages &)>(td_api::to_json)(jv, static_cast<const td_api::canSendMessageToUserResultUserHasPaidMessages &>(object));
    case td_api::canSendMessageToUserResultUserIsDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canSendMessageToUserResultUserIsDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::canSendMessageToUserResultUserIsDeleted &>(object));
    case td_api::canSendMessageToUserResultUserRestrictsNewChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canSendMessageToUserResultUserRestrictsNewChats &)>(td_api::to_json)(jv, static_cast<const td_api::canSendMessageToUserResultUserRestrictsNewChats &>(object));
    case td_api::canTransferOwnershipResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canTransferOwnershipResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::canTransferOwnershipResultOk &>(object));
    case td_api::canTransferOwnershipResultPasswordNeeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canTransferOwnershipResultPasswordNeeded &)>(td_api::to_json)(jv, static_cast<const td_api::canTransferOwnershipResultPasswordNeeded &>(object));
    case td_api::canTransferOwnershipResultPasswordTooFresh::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canTransferOwnershipResultPasswordTooFresh &)>(td_api::to_json)(jv, static_cast<const td_api::canTransferOwnershipResultPasswordTooFresh &>(object));
    case td_api::canTransferOwnershipResultSessionTooFresh::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::canTransferOwnershipResultSessionTooFresh &)>(td_api::to_json)(jv, static_cast<const td_api::canTransferOwnershipResultSessionTooFresh &>(object));
    case td_api::chat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chat &)>(td_api::to_json)(jv, static_cast<const td_api::chat &>(object));
    case td_api::chatActionBarAddContact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionBarAddContact &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionBarAddContact &>(object));
    case td_api::chatActionBarInviteMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionBarInviteMembers &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionBarInviteMembers &>(object));
    case td_api::chatActionBarJoinRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionBarJoinRequest &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionBarJoinRequest &>(object));
    case td_api::chatActionBarReportAddBlock::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionBarReportAddBlock &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionBarReportAddBlock &>(object));
    case td_api::chatActionBarReportSpam::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionBarReportSpam &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionBarReportSpam &>(object));
    case td_api::chatActionBarSharePhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionBarSharePhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionBarSharePhoneNumber &>(object));
    case td_api::chatActionCancel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionCancel &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionCancel &>(object));
    case td_api::chatActionChoosingContact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionChoosingContact &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionChoosingContact &>(object));
    case td_api::chatActionChoosingLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionChoosingLocation &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionChoosingLocation &>(object));
    case td_api::chatActionChoosingSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionChoosingSticker &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionChoosingSticker &>(object));
    case td_api::chatActionRecordingVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionRecordingVideo &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionRecordingVideo &>(object));
    case td_api::chatActionRecordingVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionRecordingVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionRecordingVideoNote &>(object));
    case td_api::chatActionRecordingVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionRecordingVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionRecordingVoiceNote &>(object));
    case td_api::chatActionStartPlayingGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionStartPlayingGame &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionStartPlayingGame &>(object));
    case td_api::chatActionTyping::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionTyping &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionTyping &>(object));
    case td_api::chatActionUploadingDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionUploadingDocument &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionUploadingDocument &>(object));
    case td_api::chatActionUploadingPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionUploadingPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionUploadingPhoto &>(object));
    case td_api::chatActionUploadingVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionUploadingVideo &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionUploadingVideo &>(object));
    case td_api::chatActionUploadingVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionUploadingVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionUploadingVideoNote &>(object));
    case td_api::chatActionUploadingVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionUploadingVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionUploadingVoiceNote &>(object));
    case td_api::chatActionWatchingAnimations::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActionWatchingAnimations &)>(td_api::to_json)(jv, static_cast<const td_api::chatActionWatchingAnimations &>(object));
    case td_api::chatActiveStories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatActiveStories &)>(td_api::to_json)(jv, static_cast<const td_api::chatActiveStories &>(object));
    case td_api::chatAdministrator::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatAdministrator &)>(td_api::to_json)(jv, static_cast<const td_api::chatAdministrator &>(object));
    case td_api::chatAdministratorRights::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatAdministratorRights &)>(td_api::to_json)(jv, static_cast<const td_api::chatAdministratorRights &>(object));
    case td_api::chatAdministrators::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatAdministrators &)>(td_api::to_json)(jv, static_cast<const td_api::chatAdministrators &>(object));
    case td_api::chatAvailableReactionsAll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatAvailableReactionsAll &)>(td_api::to_json)(jv, static_cast<const td_api::chatAvailableReactionsAll &>(object));
    case td_api::chatAvailableReactionsSome::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatAvailableReactionsSome &)>(td_api::to_json)(jv, static_cast<const td_api::chatAvailableReactionsSome &>(object));
    case td_api::chatBackground::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBackground &)>(td_api::to_json)(jv, static_cast<const td_api::chatBackground &>(object));
    case td_api::chatBoost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoost &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoost &>(object));
    case td_api::chatBoostFeatures::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostFeatures &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostFeatures &>(object));
    case td_api::chatBoostLevelFeatures::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostLevelFeatures &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostLevelFeatures &>(object));
    case td_api::chatBoostLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostLink &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostLink &>(object));
    case td_api::chatBoostLinkInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostLinkInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostLinkInfo &>(object));
    case td_api::chatBoostSlot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostSlot &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostSlot &>(object));
    case td_api::chatBoostSlots::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostSlots &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostSlots &>(object));
    case td_api::chatBoostSourceGiftCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostSourceGiftCode &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostSourceGiftCode &>(object));
    case td_api::chatBoostSourceGiveaway::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostSourceGiveaway &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostSourceGiveaway &>(object));
    case td_api::chatBoostSourcePremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostSourcePremium &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostSourcePremium &>(object));
    case td_api::chatBoostStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatBoostStatus &)>(td_api::to_json)(jv, static_cast<const td_api::chatBoostStatus &>(object));
    case td_api::chatEvent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEvent &)>(td_api::to_json)(jv, static_cast<const td_api::chatEvent &>(object));
    case td_api::chatEventAccentColorChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventAccentColorChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventAccentColorChanged &>(object));
    case td_api::chatEventActiveUsernamesChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventActiveUsernamesChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventActiveUsernamesChanged &>(object));
    case td_api::chatEventAutomaticTranslationToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventAutomaticTranslationToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventAutomaticTranslationToggled &>(object));
    case td_api::chatEventAvailableReactionsChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventAvailableReactionsChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventAvailableReactionsChanged &>(object));
    case td_api::chatEventBackgroundChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventBackgroundChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventBackgroundChanged &>(object));
    case td_api::chatEventCustomEmojiStickerSetChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventCustomEmojiStickerSetChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventCustomEmojiStickerSetChanged &>(object));
    case td_api::chatEventDescriptionChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventDescriptionChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventDescriptionChanged &>(object));
    case td_api::chatEventEmojiStatusChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventEmojiStatusChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventEmojiStatusChanged &>(object));
    case td_api::chatEventForumTopicCreated::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventForumTopicCreated &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventForumTopicCreated &>(object));
    case td_api::chatEventForumTopicDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventForumTopicDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventForumTopicDeleted &>(object));
    case td_api::chatEventForumTopicEdited::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventForumTopicEdited &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventForumTopicEdited &>(object));
    case td_api::chatEventForumTopicPinned::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventForumTopicPinned &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventForumTopicPinned &>(object));
    case td_api::chatEventForumTopicToggleIsClosed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventForumTopicToggleIsClosed &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventForumTopicToggleIsClosed &>(object));
    case td_api::chatEventForumTopicToggleIsHidden::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventForumTopicToggleIsHidden &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventForumTopicToggleIsHidden &>(object));
    case td_api::chatEventHasAggressiveAntiSpamEnabledToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventHasAggressiveAntiSpamEnabledToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventHasAggressiveAntiSpamEnabledToggled &>(object));
    case td_api::chatEventHasProtectedContentToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventHasProtectedContentToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventHasProtectedContentToggled &>(object));
    case td_api::chatEventInviteLinkDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventInviteLinkDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventInviteLinkDeleted &>(object));
    case td_api::chatEventInviteLinkEdited::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventInviteLinkEdited &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventInviteLinkEdited &>(object));
    case td_api::chatEventInviteLinkRevoked::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventInviteLinkRevoked &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventInviteLinkRevoked &>(object));
    case td_api::chatEventInvitesToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventInvitesToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventInvitesToggled &>(object));
    case td_api::chatEventIsAllHistoryAvailableToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventIsAllHistoryAvailableToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventIsAllHistoryAvailableToggled &>(object));
    case td_api::chatEventIsForumToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventIsForumToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventIsForumToggled &>(object));
    case td_api::chatEventLinkedChatChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventLinkedChatChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventLinkedChatChanged &>(object));
    case td_api::chatEventLocationChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventLocationChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventLocationChanged &>(object));
    case td_api::chatEventMemberInvited::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberInvited &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberInvited &>(object));
    case td_api::chatEventMemberJoined::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberJoined &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberJoined &>(object));
    case td_api::chatEventMemberJoinedByInviteLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberJoinedByInviteLink &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberJoinedByInviteLink &>(object));
    case td_api::chatEventMemberJoinedByRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberJoinedByRequest &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberJoinedByRequest &>(object));
    case td_api::chatEventMemberLeft::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberLeft &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberLeft &>(object));
    case td_api::chatEventMemberPromoted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberPromoted &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberPromoted &>(object));
    case td_api::chatEventMemberRestricted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberRestricted &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberRestricted &>(object));
    case td_api::chatEventMemberSubscriptionExtended::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMemberSubscriptionExtended &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMemberSubscriptionExtended &>(object));
    case td_api::chatEventMessageAutoDeleteTimeChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMessageAutoDeleteTimeChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMessageAutoDeleteTimeChanged &>(object));
    case td_api::chatEventMessageDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMessageDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMessageDeleted &>(object));
    case td_api::chatEventMessageEdited::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMessageEdited &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMessageEdited &>(object));
    case td_api::chatEventMessagePinned::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMessagePinned &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMessagePinned &>(object));
    case td_api::chatEventMessageUnpinned::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventMessageUnpinned &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventMessageUnpinned &>(object));
    case td_api::chatEventPermissionsChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventPermissionsChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventPermissionsChanged &>(object));
    case td_api::chatEventPhotoChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventPhotoChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventPhotoChanged &>(object));
    case td_api::chatEventPollStopped::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventPollStopped &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventPollStopped &>(object));
    case td_api::chatEventProfileAccentColorChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventProfileAccentColorChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventProfileAccentColorChanged &>(object));
    case td_api::chatEventShowMessageSenderToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventShowMessageSenderToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventShowMessageSenderToggled &>(object));
    case td_api::chatEventSignMessagesToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventSignMessagesToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventSignMessagesToggled &>(object));
    case td_api::chatEventSlowModeDelayChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventSlowModeDelayChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventSlowModeDelayChanged &>(object));
    case td_api::chatEventStickerSetChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventStickerSetChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventStickerSetChanged &>(object));
    case td_api::chatEventTitleChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventTitleChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventTitleChanged &>(object));
    case td_api::chatEventUsernameChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventUsernameChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventUsernameChanged &>(object));
    case td_api::chatEventVideoChatCreated::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventVideoChatCreated &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventVideoChatCreated &>(object));
    case td_api::chatEventVideoChatEnded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventVideoChatEnded &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventVideoChatEnded &>(object));
    case td_api::chatEventVideoChatMuteNewParticipantsToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventVideoChatMuteNewParticipantsToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventVideoChatMuteNewParticipantsToggled &>(object));
    case td_api::chatEventVideoChatParticipantIsMutedToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventVideoChatParticipantIsMutedToggled &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventVideoChatParticipantIsMutedToggled &>(object));
    case td_api::chatEventVideoChatParticipantVolumeLevelChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEventVideoChatParticipantVolumeLevelChanged &)>(td_api::to_json)(jv, static_cast<const td_api::chatEventVideoChatParticipantVolumeLevelChanged &>(object));
    case td_api::chatEvents::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatEvents &)>(td_api::to_json)(jv, static_cast<const td_api::chatEvents &>(object));
    case td_api::chatFolder::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatFolder &)>(td_api::to_json)(jv, static_cast<const td_api::chatFolder &>(object));
    case td_api::chatFolderIcon::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatFolderIcon &)>(td_api::to_json)(jv, static_cast<const td_api::chatFolderIcon &>(object));
    case td_api::chatFolderInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatFolderInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatFolderInfo &>(object));
    case td_api::chatFolderInviteLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatFolderInviteLink &)>(td_api::to_json)(jv, static_cast<const td_api::chatFolderInviteLink &>(object));
    case td_api::chatFolderInviteLinkInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatFolderInviteLinkInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatFolderInviteLinkInfo &>(object));
    case td_api::chatFolderInviteLinks::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatFolderInviteLinks &)>(td_api::to_json)(jv, static_cast<const td_api::chatFolderInviteLinks &>(object));
    case td_api::chatFolderName::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatFolderName &)>(td_api::to_json)(jv, static_cast<const td_api::chatFolderName &>(object));
    case td_api::chatInviteLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLink &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLink &>(object));
    case td_api::chatInviteLinkCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLinkCount &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLinkCount &>(object));
    case td_api::chatInviteLinkCounts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLinkCounts &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLinkCounts &>(object));
    case td_api::chatInviteLinkInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLinkInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLinkInfo &>(object));
    case td_api::chatInviteLinkMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLinkMember &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLinkMember &>(object));
    case td_api::chatInviteLinkMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLinkMembers &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLinkMembers &>(object));
    case td_api::chatInviteLinkSubscriptionInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLinkSubscriptionInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLinkSubscriptionInfo &>(object));
    case td_api::chatInviteLinks::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatInviteLinks &)>(td_api::to_json)(jv, static_cast<const td_api::chatInviteLinks &>(object));
    case td_api::chatJoinRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatJoinRequest &)>(td_api::to_json)(jv, static_cast<const td_api::chatJoinRequest &>(object));
    case td_api::chatJoinRequests::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatJoinRequests &)>(td_api::to_json)(jv, static_cast<const td_api::chatJoinRequests &>(object));
    case td_api::chatJoinRequestsInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatJoinRequestsInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatJoinRequestsInfo &>(object));
    case td_api::chatListArchive::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatListArchive &)>(td_api::to_json)(jv, static_cast<const td_api::chatListArchive &>(object));
    case td_api::chatListFolder::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatListFolder &)>(td_api::to_json)(jv, static_cast<const td_api::chatListFolder &>(object));
    case td_api::chatListMain::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatListMain &)>(td_api::to_json)(jv, static_cast<const td_api::chatListMain &>(object));
    case td_api::chatLists::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatLists &)>(td_api::to_json)(jv, static_cast<const td_api::chatLists &>(object));
    case td_api::chatLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatLocation &)>(td_api::to_json)(jv, static_cast<const td_api::chatLocation &>(object));
    case td_api::chatMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMember &)>(td_api::to_json)(jv, static_cast<const td_api::chatMember &>(object));
    case td_api::chatMemberStatusAdministrator::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMemberStatusAdministrator &)>(td_api::to_json)(jv, static_cast<const td_api::chatMemberStatusAdministrator &>(object));
    case td_api::chatMemberStatusBanned::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMemberStatusBanned &)>(td_api::to_json)(jv, static_cast<const td_api::chatMemberStatusBanned &>(object));
    case td_api::chatMemberStatusCreator::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMemberStatusCreator &)>(td_api::to_json)(jv, static_cast<const td_api::chatMemberStatusCreator &>(object));
    case td_api::chatMemberStatusLeft::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMemberStatusLeft &)>(td_api::to_json)(jv, static_cast<const td_api::chatMemberStatusLeft &>(object));
    case td_api::chatMemberStatusMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMemberStatusMember &)>(td_api::to_json)(jv, static_cast<const td_api::chatMemberStatusMember &>(object));
    case td_api::chatMemberStatusRestricted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMemberStatusRestricted &)>(td_api::to_json)(jv, static_cast<const td_api::chatMemberStatusRestricted &>(object));
    case td_api::chatMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMembers &)>(td_api::to_json)(jv, static_cast<const td_api::chatMembers &>(object));
    case td_api::chatMessageSender::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMessageSender &)>(td_api::to_json)(jv, static_cast<const td_api::chatMessageSender &>(object));
    case td_api::chatMessageSenders::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatMessageSenders &)>(td_api::to_json)(jv, static_cast<const td_api::chatMessageSenders &>(object));
    case td_api::chatNotificationSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatNotificationSettings &)>(td_api::to_json)(jv, static_cast<const td_api::chatNotificationSettings &>(object));
    case td_api::chatPermissions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPermissions &)>(td_api::to_json)(jv, static_cast<const td_api::chatPermissions &>(object));
    case td_api::chatPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::chatPhoto &>(object));
    case td_api::chatPhotoInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPhotoInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatPhotoInfo &>(object));
    case td_api::chatPhotoSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPhotoSticker &)>(td_api::to_json)(jv, static_cast<const td_api::chatPhotoSticker &>(object));
    case td_api::chatPhotoStickerTypeCustomEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPhotoStickerTypeCustomEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::chatPhotoStickerTypeCustomEmoji &>(object));
    case td_api::chatPhotoStickerTypeRegularOrMask::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPhotoStickerTypeRegularOrMask &)>(td_api::to_json)(jv, static_cast<const td_api::chatPhotoStickerTypeRegularOrMask &>(object));
    case td_api::chatPhotos::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPhotos &)>(td_api::to_json)(jv, static_cast<const td_api::chatPhotos &>(object));
    case td_api::chatPosition::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatPosition &)>(td_api::to_json)(jv, static_cast<const td_api::chatPosition &>(object));
    case td_api::chatRevenueAmount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueAmount &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueAmount &>(object));
    case td_api::chatRevenueStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueStatistics &>(object));
    case td_api::chatRevenueTransaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueTransaction &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueTransaction &>(object));
    case td_api::chatRevenueTransactionTypeFragmentRefund::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueTransactionTypeFragmentRefund &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueTransactionTypeFragmentRefund &>(object));
    case td_api::chatRevenueTransactionTypeFragmentWithdrawal::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueTransactionTypeFragmentWithdrawal &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueTransactionTypeFragmentWithdrawal &>(object));
    case td_api::chatRevenueTransactionTypeSponsoredMessageEarnings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueTransactionTypeSponsoredMessageEarnings &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueTransactionTypeSponsoredMessageEarnings &>(object));
    case td_api::chatRevenueTransactionTypeSuggestedPostEarnings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueTransactionTypeSuggestedPostEarnings &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueTransactionTypeSuggestedPostEarnings &>(object));
    case td_api::chatRevenueTransactionTypeUnsupported::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueTransactionTypeUnsupported &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueTransactionTypeUnsupported &>(object));
    case td_api::chatRevenueTransactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatRevenueTransactions &)>(td_api::to_json)(jv, static_cast<const td_api::chatRevenueTransactions &>(object));
    case td_api::chatSourceMtprotoProxy::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatSourceMtprotoProxy &)>(td_api::to_json)(jv, static_cast<const td_api::chatSourceMtprotoProxy &>(object));
    case td_api::chatSourcePublicServiceAnnouncement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatSourcePublicServiceAnnouncement &)>(td_api::to_json)(jv, static_cast<const td_api::chatSourcePublicServiceAnnouncement &>(object));
    case td_api::chatStatisticsAdministratorActionsInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsAdministratorActionsInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsAdministratorActionsInfo &>(object));
    case td_api::chatStatisticsChannel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsChannel &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsChannel &>(object));
    case td_api::chatStatisticsInteractionInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsInteractionInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsInteractionInfo &>(object));
    case td_api::chatStatisticsInviterInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsInviterInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsInviterInfo &>(object));
    case td_api::chatStatisticsMessageSenderInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsMessageSenderInfo &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsMessageSenderInfo &>(object));
    case td_api::chatStatisticsObjectTypeMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsObjectTypeMessage &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsObjectTypeMessage &>(object));
    case td_api::chatStatisticsObjectTypeStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsObjectTypeStory &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsObjectTypeStory &>(object));
    case td_api::chatStatisticsSupergroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatStatisticsSupergroup &)>(td_api::to_json)(jv, static_cast<const td_api::chatStatisticsSupergroup &>(object));
    case td_api::chatThemeEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatThemeEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::chatThemeEmoji &>(object));
    case td_api::chatThemeGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatThemeGift &)>(td_api::to_json)(jv, static_cast<const td_api::chatThemeGift &>(object));
    case td_api::chatTypeBasicGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatTypeBasicGroup &)>(td_api::to_json)(jv, static_cast<const td_api::chatTypeBasicGroup &>(object));
    case td_api::chatTypePrivate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatTypePrivate &)>(td_api::to_json)(jv, static_cast<const td_api::chatTypePrivate &>(object));
    case td_api::chatTypeSecret::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatTypeSecret &)>(td_api::to_json)(jv, static_cast<const td_api::chatTypeSecret &>(object));
    case td_api::chatTypeSupergroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chatTypeSupergroup &)>(td_api::to_json)(jv, static_cast<const td_api::chatTypeSupergroup &>(object));
    case td_api::chats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::chats &)>(td_api::to_json)(jv, static_cast<const td_api::chats &>(object));
    case td_api::checkChatUsernameResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkChatUsernameResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::checkChatUsernameResultOk &>(object));
    case td_api::checkChatUsernameResultPublicChatsTooMany::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkChatUsernameResultPublicChatsTooMany &)>(td_api::to_json)(jv, static_cast<const td_api::checkChatUsernameResultPublicChatsTooMany &>(object));
    case td_api::checkChatUsernameResultPublicGroupsUnavailable::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkChatUsernameResultPublicGroupsUnavailable &)>(td_api::to_json)(jv, static_cast<const td_api::checkChatUsernameResultPublicGroupsUnavailable &>(object));
    case td_api::checkChatUsernameResultUsernameInvalid::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkChatUsernameResultUsernameInvalid &)>(td_api::to_json)(jv, static_cast<const td_api::checkChatUsernameResultUsernameInvalid &>(object));
    case td_api::checkChatUsernameResultUsernameOccupied::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkChatUsernameResultUsernameOccupied &)>(td_api::to_json)(jv, static_cast<const td_api::checkChatUsernameResultUsernameOccupied &>(object));
    case td_api::checkChatUsernameResultUsernamePurchasable::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkChatUsernameResultUsernamePurchasable &)>(td_api::to_json)(jv, static_cast<const td_api::checkChatUsernameResultUsernamePurchasable &>(object));
    case td_api::checkStickerSetNameResultNameInvalid::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkStickerSetNameResultNameInvalid &)>(td_api::to_json)(jv, static_cast<const td_api::checkStickerSetNameResultNameInvalid &>(object));
    case td_api::checkStickerSetNameResultNameOccupied::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkStickerSetNameResultNameOccupied &)>(td_api::to_json)(jv, static_cast<const td_api::checkStickerSetNameResultNameOccupied &>(object));
    case td_api::checkStickerSetNameResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checkStickerSetNameResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::checkStickerSetNameResultOk &>(object));
    case td_api::checklist::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checklist &)>(td_api::to_json)(jv, static_cast<const td_api::checklist &>(object));
    case td_api::checklistTask::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::checklistTask &)>(td_api::to_json)(jv, static_cast<const td_api::checklistTask &>(object));
    case td_api::closeBirthdayUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::closeBirthdayUser &)>(td_api::to_json)(jv, static_cast<const td_api::closeBirthdayUser &>(object));
    case td_api::closedVectorPath::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::closedVectorPath &)>(td_api::to_json)(jv, static_cast<const td_api::closedVectorPath &>(object));
    case td_api::collectibleItemInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::collectibleItemInfo &)>(td_api::to_json)(jv, static_cast<const td_api::collectibleItemInfo &>(object));
    case td_api::connectedAffiliateProgram::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectedAffiliateProgram &)>(td_api::to_json)(jv, static_cast<const td_api::connectedAffiliateProgram &>(object));
    case td_api::connectedAffiliatePrograms::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectedAffiliatePrograms &)>(td_api::to_json)(jv, static_cast<const td_api::connectedAffiliatePrograms &>(object));
    case td_api::connectedWebsite::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectedWebsite &)>(td_api::to_json)(jv, static_cast<const td_api::connectedWebsite &>(object));
    case td_api::connectedWebsites::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectedWebsites &)>(td_api::to_json)(jv, static_cast<const td_api::connectedWebsites &>(object));
    case td_api::connectionStateConnecting::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectionStateConnecting &)>(td_api::to_json)(jv, static_cast<const td_api::connectionStateConnecting &>(object));
    case td_api::connectionStateConnectingToProxy::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectionStateConnectingToProxy &)>(td_api::to_json)(jv, static_cast<const td_api::connectionStateConnectingToProxy &>(object));
    case td_api::connectionStateReady::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectionStateReady &)>(td_api::to_json)(jv, static_cast<const td_api::connectionStateReady &>(object));
    case td_api::connectionStateUpdating::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectionStateUpdating &)>(td_api::to_json)(jv, static_cast<const td_api::connectionStateUpdating &>(object));
    case td_api::connectionStateWaitingForNetwork::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::connectionStateWaitingForNetwork &)>(td_api::to_json)(jv, static_cast<const td_api::connectionStateWaitingForNetwork &>(object));
    case td_api::contact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::contact &)>(td_api::to_json)(jv, static_cast<const td_api::contact &>(object));
    case td_api::count::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::count &)>(td_api::to_json)(jv, static_cast<const td_api::count &>(object));
    case td_api::countries::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::countries &)>(td_api::to_json)(jv, static_cast<const td_api::countries &>(object));
    case td_api::countryInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::countryInfo &)>(td_api::to_json)(jv, static_cast<const td_api::countryInfo &>(object));
    case td_api::createdBasicGroupChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::createdBasicGroupChat &)>(td_api::to_json)(jv, static_cast<const td_api::createdBasicGroupChat &>(object));
    case td_api::currentWeather::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::currentWeather &)>(td_api::to_json)(jv, static_cast<const td_api::currentWeather &>(object));
    case td_api::customRequestResult::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::customRequestResult &)>(td_api::to_json)(jv, static_cast<const td_api::customRequestResult &>(object));
    case td_api::data::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::data &)>(td_api::to_json)(jv, static_cast<const td_api::data &>(object));
    case td_api::databaseStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::databaseStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::databaseStatistics &>(object));
    case td_api::date::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::date &)>(td_api::to_json)(jv, static_cast<const td_api::date &>(object));
    case td_api::dateRange::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::dateRange &)>(td_api::to_json)(jv, static_cast<const td_api::dateRange &>(object));
    case td_api::datedFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::datedFile &)>(td_api::to_json)(jv, static_cast<const td_api::datedFile &>(object));
    case td_api::deepLinkInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::deepLinkInfo &)>(td_api::to_json)(jv, static_cast<const td_api::deepLinkInfo &>(object));
    case td_api::diceStickersRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::diceStickersRegular &)>(td_api::to_json)(jv, static_cast<const td_api::diceStickersRegular &>(object));
    case td_api::diceStickersSlotMachine::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::diceStickersSlotMachine &)>(td_api::to_json)(jv, static_cast<const td_api::diceStickersSlotMachine &>(object));
    case td_api::directMessagesChatTopic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::directMessagesChatTopic &)>(td_api::to_json)(jv, static_cast<const td_api::directMessagesChatTopic &>(object));
    case td_api::document::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::document &)>(td_api::to_json)(jv, static_cast<const td_api::document &>(object));
    case td_api::downloadedFileCounts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::downloadedFileCounts &)>(td_api::to_json)(jv, static_cast<const td_api::downloadedFileCounts &>(object));
    case td_api::draftMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::draftMessage &)>(td_api::to_json)(jv, static_cast<const td_api::draftMessage &>(object));
    case td_api::emailAddressAuthenticationCodeInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emailAddressAuthenticationCodeInfo &)>(td_api::to_json)(jv, static_cast<const td_api::emailAddressAuthenticationCodeInfo &>(object));
    case td_api::emailAddressResetStateAvailable::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emailAddressResetStateAvailable &)>(td_api::to_json)(jv, static_cast<const td_api::emailAddressResetStateAvailable &>(object));
    case td_api::emailAddressResetStatePending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emailAddressResetStatePending &)>(td_api::to_json)(jv, static_cast<const td_api::emailAddressResetStatePending &>(object));
    case td_api::emojiCategories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiCategories &)>(td_api::to_json)(jv, static_cast<const td_api::emojiCategories &>(object));
    case td_api::emojiCategory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiCategory &)>(td_api::to_json)(jv, static_cast<const td_api::emojiCategory &>(object));
    case td_api::emojiCategorySourcePremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiCategorySourcePremium &)>(td_api::to_json)(jv, static_cast<const td_api::emojiCategorySourcePremium &>(object));
    case td_api::emojiCategorySourceSearch::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiCategorySourceSearch &)>(td_api::to_json)(jv, static_cast<const td_api::emojiCategorySourceSearch &>(object));
    case td_api::emojiChatTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiChatTheme &)>(td_api::to_json)(jv, static_cast<const td_api::emojiChatTheme &>(object));
    case td_api::emojiKeyword::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiKeyword &)>(td_api::to_json)(jv, static_cast<const td_api::emojiKeyword &>(object));
    case td_api::emojiKeywords::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiKeywords &)>(td_api::to_json)(jv, static_cast<const td_api::emojiKeywords &>(object));
    case td_api::emojiReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiReaction &)>(td_api::to_json)(jv, static_cast<const td_api::emojiReaction &>(object));
    case td_api::emojiStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiStatus &)>(td_api::to_json)(jv, static_cast<const td_api::emojiStatus &>(object));
    case td_api::emojiStatusCustomEmojis::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiStatusCustomEmojis &)>(td_api::to_json)(jv, static_cast<const td_api::emojiStatusCustomEmojis &>(object));
    case td_api::emojiStatusTypeCustomEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiStatusTypeCustomEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::emojiStatusTypeCustomEmoji &>(object));
    case td_api::emojiStatusTypeUpgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiStatusTypeUpgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::emojiStatusTypeUpgradedGift &>(object));
    case td_api::emojiStatuses::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojiStatuses &)>(td_api::to_json)(jv, static_cast<const td_api::emojiStatuses &>(object));
    case td_api::emojis::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::emojis &)>(td_api::to_json)(jv, static_cast<const td_api::emojis &>(object));
    case td_api::encryptedCredentials::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::encryptedCredentials &)>(td_api::to_json)(jv, static_cast<const td_api::encryptedCredentials &>(object));
    case td_api::encryptedPassportElement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::encryptedPassportElement &)>(td_api::to_json)(jv, static_cast<const td_api::encryptedPassportElement &>(object));
    case td_api::error::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::error &)>(td_api::to_json)(jv, static_cast<const td_api::error &>(object));
    case td_api::factCheck::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::factCheck &)>(td_api::to_json)(jv, static_cast<const td_api::factCheck &>(object));
    case td_api::failedToAddMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::failedToAddMember &)>(td_api::to_json)(jv, static_cast<const td_api::failedToAddMember &>(object));
    case td_api::failedToAddMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::failedToAddMembers &)>(td_api::to_json)(jv, static_cast<const td_api::failedToAddMembers &>(object));
    case td_api::file::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::file &)>(td_api::to_json)(jv, static_cast<const td_api::file &>(object));
    case td_api::fileDownload::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileDownload &)>(td_api::to_json)(jv, static_cast<const td_api::fileDownload &>(object));
    case td_api::fileDownloadedPrefixSize::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileDownloadedPrefixSize &)>(td_api::to_json)(jv, static_cast<const td_api::fileDownloadedPrefixSize &>(object));
    case td_api::fileTypeAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeAnimation &>(object));
    case td_api::fileTypeAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeAudio &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeAudio &>(object));
    case td_api::fileTypeDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeDocument &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeDocument &>(object));
    case td_api::fileTypeNone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeNone &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeNone &>(object));
    case td_api::fileTypeNotificationSound::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeNotificationSound &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeNotificationSound &>(object));
    case td_api::fileTypePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypePhoto &>(object));
    case td_api::fileTypePhotoStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypePhotoStory &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypePhotoStory &>(object));
    case td_api::fileTypeProfilePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeProfilePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeProfilePhoto &>(object));
    case td_api::fileTypeSecret::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSecret &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSecret &>(object));
    case td_api::fileTypeSecretThumbnail::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSecretThumbnail &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSecretThumbnail &>(object));
    case td_api::fileTypeSecure::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSecure &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSecure &>(object));
    case td_api::fileTypeSelfDestructingPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSelfDestructingPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSelfDestructingPhoto &>(object));
    case td_api::fileTypeSelfDestructingVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSelfDestructingVideo &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSelfDestructingVideo &>(object));
    case td_api::fileTypeSelfDestructingVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSelfDestructingVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSelfDestructingVideoNote &>(object));
    case td_api::fileTypeSelfDestructingVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSelfDestructingVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSelfDestructingVoiceNote &>(object));
    case td_api::fileTypeSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeSticker &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeSticker &>(object));
    case td_api::fileTypeThumbnail::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeThumbnail &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeThumbnail &>(object));
    case td_api::fileTypeUnknown::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeUnknown &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeUnknown &>(object));
    case td_api::fileTypeVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeVideo &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeVideo &>(object));
    case td_api::fileTypeVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeVideoNote &>(object));
    case td_api::fileTypeVideoStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeVideoStory &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeVideoStory &>(object));
    case td_api::fileTypeVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeVoiceNote &>(object));
    case td_api::fileTypeWallpaper::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::fileTypeWallpaper &)>(td_api::to_json)(jv, static_cast<const td_api::fileTypeWallpaper &>(object));
    case td_api::firebaseDeviceVerificationParametersPlayIntegrity::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::firebaseDeviceVerificationParametersPlayIntegrity &)>(td_api::to_json)(jv, static_cast<const td_api::firebaseDeviceVerificationParametersPlayIntegrity &>(object));
    case td_api::firebaseDeviceVerificationParametersSafetyNet::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::firebaseDeviceVerificationParametersSafetyNet &)>(td_api::to_json)(jv, static_cast<const td_api::firebaseDeviceVerificationParametersSafetyNet &>(object));
    case td_api::formattedText::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::formattedText &)>(td_api::to_json)(jv, static_cast<const td_api::formattedText &>(object));
    case td_api::forumTopic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::forumTopic &)>(td_api::to_json)(jv, static_cast<const td_api::forumTopic &>(object));
    case td_api::forumTopicIcon::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::forumTopicIcon &)>(td_api::to_json)(jv, static_cast<const td_api::forumTopicIcon &>(object));
    case td_api::forumTopicInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::forumTopicInfo &)>(td_api::to_json)(jv, static_cast<const td_api::forumTopicInfo &>(object));
    case td_api::forumTopics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::forumTopics &)>(td_api::to_json)(jv, static_cast<const td_api::forumTopics &>(object));
    case td_api::forwardSource::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::forwardSource &)>(td_api::to_json)(jv, static_cast<const td_api::forwardSource &>(object));
    case td_api::foundAffiliateProgram::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundAffiliateProgram &)>(td_api::to_json)(jv, static_cast<const td_api::foundAffiliateProgram &>(object));
    case td_api::foundAffiliatePrograms::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundAffiliatePrograms &)>(td_api::to_json)(jv, static_cast<const td_api::foundAffiliatePrograms &>(object));
    case td_api::foundChatBoosts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundChatBoosts &)>(td_api::to_json)(jv, static_cast<const td_api::foundChatBoosts &>(object));
    case td_api::foundChatMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundChatMessages &)>(td_api::to_json)(jv, static_cast<const td_api::foundChatMessages &>(object));
    case td_api::foundFileDownloads::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundFileDownloads &)>(td_api::to_json)(jv, static_cast<const td_api::foundFileDownloads &>(object));
    case td_api::foundMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundMessages &)>(td_api::to_json)(jv, static_cast<const td_api::foundMessages &>(object));
    case td_api::foundPosition::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundPosition &)>(td_api::to_json)(jv, static_cast<const td_api::foundPosition &>(object));
    case td_api::foundPositions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundPositions &)>(td_api::to_json)(jv, static_cast<const td_api::foundPositions &>(object));
    case td_api::foundPublicPosts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundPublicPosts &)>(td_api::to_json)(jv, static_cast<const td_api::foundPublicPosts &>(object));
    case td_api::foundStories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundStories &)>(td_api::to_json)(jv, static_cast<const td_api::foundStories &>(object));
    case td_api::foundUsers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundUsers &)>(td_api::to_json)(jv, static_cast<const td_api::foundUsers &>(object));
    case td_api::foundWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::foundWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::foundWebApp &>(object));
    case td_api::game::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::game &)>(td_api::to_json)(jv, static_cast<const td_api::game &>(object));
    case td_api::gameHighScore::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::gameHighScore &)>(td_api::to_json)(jv, static_cast<const td_api::gameHighScore &>(object));
    case td_api::gameHighScores::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::gameHighScores &)>(td_api::to_json)(jv, static_cast<const td_api::gameHighScores &>(object));
    case td_api::gift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::gift &)>(td_api::to_json)(jv, static_cast<const td_api::gift &>(object));
    case td_api::giftChatTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftChatTheme &)>(td_api::to_json)(jv, static_cast<const td_api::giftChatTheme &>(object));
    case td_api::giftChatThemes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftChatThemes &)>(td_api::to_json)(jv, static_cast<const td_api::giftChatThemes &>(object));
    case td_api::giftCollection::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftCollection &)>(td_api::to_json)(jv, static_cast<const td_api::giftCollection &>(object));
    case td_api::giftCollections::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftCollections &)>(td_api::to_json)(jv, static_cast<const td_api::giftCollections &>(object));
    case td_api::giftForResale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftForResale &)>(td_api::to_json)(jv, static_cast<const td_api::giftForResale &>(object));
    case td_api::giftPurchaseLimits::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftPurchaseLimits &)>(td_api::to_json)(jv, static_cast<const td_api::giftPurchaseLimits &>(object));
    case td_api::giftResaleParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftResaleParameters &)>(td_api::to_json)(jv, static_cast<const td_api::giftResaleParameters &>(object));
    case td_api::giftResalePriceStar::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftResalePriceStar &)>(td_api::to_json)(jv, static_cast<const td_api::giftResalePriceStar &>(object));
    case td_api::giftResalePriceTon::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftResalePriceTon &)>(td_api::to_json)(jv, static_cast<const td_api::giftResalePriceTon &>(object));
    case td_api::giftResaleResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftResaleResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::giftResaleResultOk &>(object));
    case td_api::giftResaleResultPriceIncreased::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftResaleResultPriceIncreased &)>(td_api::to_json)(jv, static_cast<const td_api::giftResaleResultPriceIncreased &>(object));
    case td_api::giftSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftSettings &)>(td_api::to_json)(jv, static_cast<const td_api::giftSettings &>(object));
    case td_api::giftUpgradePreview::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftUpgradePreview &)>(td_api::to_json)(jv, static_cast<const td_api::giftUpgradePreview &>(object));
    case td_api::giftUpgradePrice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftUpgradePrice &)>(td_api::to_json)(jv, static_cast<const td_api::giftUpgradePrice &>(object));
    case td_api::giftsForResale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giftsForResale &)>(td_api::to_json)(jv, static_cast<const td_api::giftsForResale &>(object));
    case td_api::giveawayInfoCompleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayInfoCompleted &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayInfoCompleted &>(object));
    case td_api::giveawayInfoOngoing::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayInfoOngoing &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayInfoOngoing &>(object));
    case td_api::giveawayParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayParameters &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayParameters &>(object));
    case td_api::giveawayParticipantStatusAdministrator::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayParticipantStatusAdministrator &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayParticipantStatusAdministrator &>(object));
    case td_api::giveawayParticipantStatusAlreadyWasMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayParticipantStatusAlreadyWasMember &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayParticipantStatusAlreadyWasMember &>(object));
    case td_api::giveawayParticipantStatusDisallowedCountry::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayParticipantStatusDisallowedCountry &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayParticipantStatusDisallowedCountry &>(object));
    case td_api::giveawayParticipantStatusEligible::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayParticipantStatusEligible &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayParticipantStatusEligible &>(object));
    case td_api::giveawayParticipantStatusParticipating::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayParticipantStatusParticipating &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayParticipantStatusParticipating &>(object));
    case td_api::giveawayPrizePremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayPrizePremium &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayPrizePremium &>(object));
    case td_api::giveawayPrizeStars::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::giveawayPrizeStars &)>(td_api::to_json)(jv, static_cast<const td_api::giveawayPrizeStars &>(object));
    case td_api::groupCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCall &)>(td_api::to_json)(jv, static_cast<const td_api::groupCall &>(object));
    case td_api::groupCallId::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCallId &)>(td_api::to_json)(jv, static_cast<const td_api::groupCallId &>(object));
    case td_api::groupCallInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCallInfo &)>(td_api::to_json)(jv, static_cast<const td_api::groupCallInfo &>(object));
    case td_api::groupCallParticipant::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCallParticipant &)>(td_api::to_json)(jv, static_cast<const td_api::groupCallParticipant &>(object));
    case td_api::groupCallParticipantVideoInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCallParticipantVideoInfo &)>(td_api::to_json)(jv, static_cast<const td_api::groupCallParticipantVideoInfo &>(object));
    case td_api::groupCallParticipants::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCallParticipants &)>(td_api::to_json)(jv, static_cast<const td_api::groupCallParticipants &>(object));
    case td_api::groupCallRecentSpeaker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCallRecentSpeaker &)>(td_api::to_json)(jv, static_cast<const td_api::groupCallRecentSpeaker &>(object));
    case td_api::groupCallVideoSourceGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::groupCallVideoSourceGroup &)>(td_api::to_json)(jv, static_cast<const td_api::groupCallVideoSourceGroup &>(object));
    case td_api::hashtags::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::hashtags &)>(td_api::to_json)(jv, static_cast<const td_api::hashtags &>(object));
    case td_api::httpUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::httpUrl &)>(td_api::to_json)(jv, static_cast<const td_api::httpUrl &>(object));
    case td_api::identityDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::identityDocument &)>(td_api::to_json)(jv, static_cast<const td_api::identityDocument &>(object));
    case td_api::importedContacts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::importedContacts &)>(td_api::to_json)(jv, static_cast<const td_api::importedContacts &>(object));
    case td_api::inlineKeyboardButton::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButton &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButton &>(object));
    case td_api::inlineKeyboardButtonTypeBuy::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeBuy &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeBuy &>(object));
    case td_api::inlineKeyboardButtonTypeCallback::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeCallback &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeCallback &>(object));
    case td_api::inlineKeyboardButtonTypeCallbackGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeCallbackGame &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeCallbackGame &>(object));
    case td_api::inlineKeyboardButtonTypeCallbackWithPassword::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeCallbackWithPassword &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeCallbackWithPassword &>(object));
    case td_api::inlineKeyboardButtonTypeCopyText::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeCopyText &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeCopyText &>(object));
    case td_api::inlineKeyboardButtonTypeLoginUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeLoginUrl &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeLoginUrl &>(object));
    case td_api::inlineKeyboardButtonTypeSwitchInline::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeSwitchInline &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeSwitchInline &>(object));
    case td_api::inlineKeyboardButtonTypeUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeUrl &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeUrl &>(object));
    case td_api::inlineKeyboardButtonTypeUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeUser &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeUser &>(object));
    case td_api::inlineKeyboardButtonTypeWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineKeyboardButtonTypeWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::inlineKeyboardButtonTypeWebApp &>(object));
    case td_api::inlineQueryResultAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultAnimation &>(object));
    case td_api::inlineQueryResultArticle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultArticle &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultArticle &>(object));
    case td_api::inlineQueryResultAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultAudio &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultAudio &>(object));
    case td_api::inlineQueryResultContact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultContact &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultContact &>(object));
    case td_api::inlineQueryResultDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultDocument &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultDocument &>(object));
    case td_api::inlineQueryResultGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultGame &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultGame &>(object));
    case td_api::inlineQueryResultLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultLocation &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultLocation &>(object));
    case td_api::inlineQueryResultPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultPhoto &>(object));
    case td_api::inlineQueryResultSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultSticker &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultSticker &>(object));
    case td_api::inlineQueryResultVenue::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultVenue &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultVenue &>(object));
    case td_api::inlineQueryResultVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultVideo &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultVideo &>(object));
    case td_api::inlineQueryResultVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultVoiceNote &>(object));
    case td_api::inlineQueryResults::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResults &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResults &>(object));
    case td_api::inlineQueryResultsButton::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultsButton &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultsButton &>(object));
    case td_api::inlineQueryResultsButtonTypeStartBot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultsButtonTypeStartBot &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultsButtonTypeStartBot &>(object));
    case td_api::inlineQueryResultsButtonTypeWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inlineQueryResultsButtonTypeWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::inlineQueryResultsButtonTypeWebApp &>(object));
    case td_api::inputChecklist::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputChecklist &)>(td_api::to_json)(jv, static_cast<const td_api::inputChecklist &>(object));
    case td_api::inputChecklistTask::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputChecklistTask &)>(td_api::to_json)(jv, static_cast<const td_api::inputChecklistTask &>(object));
    case td_api::inputFileGenerated::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputFileGenerated &)>(td_api::to_json)(jv, static_cast<const td_api::inputFileGenerated &>(object));
    case td_api::inputFileId::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputFileId &)>(td_api::to_json)(jv, static_cast<const td_api::inputFileId &>(object));
    case td_api::inputFileLocal::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputFileLocal &)>(td_api::to_json)(jv, static_cast<const td_api::inputFileLocal &>(object));
    case td_api::inputFileRemote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputFileRemote &)>(td_api::to_json)(jv, static_cast<const td_api::inputFileRemote &>(object));
    case td_api::inputMessageAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageAnimation &>(object));
    case td_api::inputMessageAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageAudio &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageAudio &>(object));
    case td_api::inputMessageChecklist::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageChecklist &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageChecklist &>(object));
    case td_api::inputMessageContact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageContact &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageContact &>(object));
    case td_api::inputMessageDice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageDice &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageDice &>(object));
    case td_api::inputMessageDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageDocument &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageDocument &>(object));
    case td_api::inputMessageForwarded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageForwarded &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageForwarded &>(object));
    case td_api::inputMessageGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageGame &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageGame &>(object));
    case td_api::inputMessageInvoice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageInvoice &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageInvoice &>(object));
    case td_api::inputMessageLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageLocation &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageLocation &>(object));
    case td_api::inputMessagePaidMedia::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessagePaidMedia &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessagePaidMedia &>(object));
    case td_api::inputMessagePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessagePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessagePhoto &>(object));
    case td_api::inputMessagePoll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessagePoll &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessagePoll &>(object));
    case td_api::inputMessageReplyToExternalMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageReplyToExternalMessage &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageReplyToExternalMessage &>(object));
    case td_api::inputMessageReplyToMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageReplyToMessage &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageReplyToMessage &>(object));
    case td_api::inputMessageReplyToStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageReplyToStory &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageReplyToStory &>(object));
    case td_api::inputMessageSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageSticker &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageSticker &>(object));
    case td_api::inputMessageStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageStory &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageStory &>(object));
    case td_api::inputMessageText::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageText &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageText &>(object));
    case td_api::inputMessageVenue::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageVenue &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageVenue &>(object));
    case td_api::inputMessageVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageVideo &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageVideo &>(object));
    case td_api::inputMessageVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageVideoNote &>(object));
    case td_api::inputMessageVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputMessageVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::inputMessageVoiceNote &>(object));
    case td_api::inputPaidMedia::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputPaidMedia &)>(td_api::to_json)(jv, static_cast<const td_api::inputPaidMedia &>(object));
    case td_api::inputPaidMediaTypePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputPaidMediaTypePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::inputPaidMediaTypePhoto &>(object));
    case td_api::inputPaidMediaTypeVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputPaidMediaTypeVideo &)>(td_api::to_json)(jv, static_cast<const td_api::inputPaidMediaTypeVideo &>(object));
    case td_api::inputSuggestedPostInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputSuggestedPostInfo &)>(td_api::to_json)(jv, static_cast<const td_api::inputSuggestedPostInfo &>(object));
    case td_api::inputTextQuote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputTextQuote &)>(td_api::to_json)(jv, static_cast<const td_api::inputTextQuote &>(object));
    case td_api::inputThumbnail::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inputThumbnail &)>(td_api::to_json)(jv, static_cast<const td_api::inputThumbnail &>(object));
    case td_api::internalLinkTypeActiveSessions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeActiveSessions &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeActiveSessions &>(object));
    case td_api::internalLinkTypeAttachmentMenuBot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeAttachmentMenuBot &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeAttachmentMenuBot &>(object));
    case td_api::internalLinkTypeAuthenticationCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeAuthenticationCode &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeAuthenticationCode &>(object));
    case td_api::internalLinkTypeBackground::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeBackground &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeBackground &>(object));
    case td_api::internalLinkTypeBotAddToChannel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeBotAddToChannel &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeBotAddToChannel &>(object));
    case td_api::internalLinkTypeBotStart::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeBotStart &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeBotStart &>(object));
    case td_api::internalLinkTypeBotStartInGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeBotStartInGroup &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeBotStartInGroup &>(object));
    case td_api::internalLinkTypeBusinessChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeBusinessChat &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeBusinessChat &>(object));
    case td_api::internalLinkTypeBuyStars::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeBuyStars &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeBuyStars &>(object));
    case td_api::internalLinkTypeChangePhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeChangePhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeChangePhoneNumber &>(object));
    case td_api::internalLinkTypeChatAffiliateProgram::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeChatAffiliateProgram &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeChatAffiliateProgram &>(object));
    case td_api::internalLinkTypeChatBoost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeChatBoost &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeChatBoost &>(object));
    case td_api::internalLinkTypeChatFolderInvite::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeChatFolderInvite &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeChatFolderInvite &>(object));
    case td_api::internalLinkTypeChatFolderSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeChatFolderSettings &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeChatFolderSettings &>(object));
    case td_api::internalLinkTypeChatInvite::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeChatInvite &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeChatInvite &>(object));
    case td_api::internalLinkTypeDefaultMessageAutoDeleteTimerSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeDefaultMessageAutoDeleteTimerSettings &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeDefaultMessageAutoDeleteTimerSettings &>(object));
    case td_api::internalLinkTypeDirectMessagesChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeDirectMessagesChat &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeDirectMessagesChat &>(object));
    case td_api::internalLinkTypeEditProfileSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeEditProfileSettings &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeEditProfileSettings &>(object));
    case td_api::internalLinkTypeGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeGame &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeGame &>(object));
    case td_api::internalLinkTypeGiftCollection::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeGiftCollection &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeGiftCollection &>(object));
    case td_api::internalLinkTypeGroupCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeGroupCall &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeGroupCall &>(object));
    case td_api::internalLinkTypeInstantView::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeInstantView &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeInstantView &>(object));
    case td_api::internalLinkTypeInvoice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeInvoice &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeInvoice &>(object));
    case td_api::internalLinkTypeLanguagePack::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeLanguagePack &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeLanguagePack &>(object));
    case td_api::internalLinkTypeLanguageSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeLanguageSettings &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeLanguageSettings &>(object));
    case td_api::internalLinkTypeMainWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeMainWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeMainWebApp &>(object));
    case td_api::internalLinkTypeMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeMessage &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeMessage &>(object));
    case td_api::internalLinkTypeMessageDraft::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeMessageDraft &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeMessageDraft &>(object));
    case td_api::internalLinkTypeMyStars::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeMyStars &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeMyStars &>(object));
    case td_api::internalLinkTypeMyToncoins::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeMyToncoins &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeMyToncoins &>(object));
    case td_api::internalLinkTypePassportDataRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypePassportDataRequest &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypePassportDataRequest &>(object));
    case td_api::internalLinkTypePhoneNumberConfirmation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypePhoneNumberConfirmation &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypePhoneNumberConfirmation &>(object));
    case td_api::internalLinkTypePremiumFeatures::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypePremiumFeatures &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypePremiumFeatures &>(object));
    case td_api::internalLinkTypePremiumGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypePremiumGift &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypePremiumGift &>(object));
    case td_api::internalLinkTypePremiumGiftCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypePremiumGiftCode &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypePremiumGiftCode &>(object));
    case td_api::internalLinkTypePrivacyAndSecuritySettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypePrivacyAndSecuritySettings &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypePrivacyAndSecuritySettings &>(object));
    case td_api::internalLinkTypeProxy::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeProxy &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeProxy &>(object));
    case td_api::internalLinkTypePublicChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypePublicChat &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypePublicChat &>(object));
    case td_api::internalLinkTypeQrCodeAuthentication::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeQrCodeAuthentication &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeQrCodeAuthentication &>(object));
    case td_api::internalLinkTypeRestorePurchases::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeRestorePurchases &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeRestorePurchases &>(object));
    case td_api::internalLinkTypeSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeSettings &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeSettings &>(object));
    case td_api::internalLinkTypeStickerSet::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeStickerSet &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeStickerSet &>(object));
    case td_api::internalLinkTypeStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeStory &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeStory &>(object));
    case td_api::internalLinkTypeStoryAlbum::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeStoryAlbum &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeStoryAlbum &>(object));
    case td_api::internalLinkTypeTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeTheme &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeTheme &>(object));
    case td_api::internalLinkTypeThemeSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeThemeSettings &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeThemeSettings &>(object));
    case td_api::internalLinkTypeUnknownDeepLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeUnknownDeepLink &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeUnknownDeepLink &>(object));
    case td_api::internalLinkTypeUnsupportedProxy::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeUnsupportedProxy &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeUnsupportedProxy &>(object));
    case td_api::internalLinkTypeUpgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeUpgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeUpgradedGift &>(object));
    case td_api::internalLinkTypeUserPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeUserPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeUserPhoneNumber &>(object));
    case td_api::internalLinkTypeUserToken::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeUserToken &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeUserToken &>(object));
    case td_api::internalLinkTypeVideoChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeVideoChat &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeVideoChat &>(object));
    case td_api::internalLinkTypeWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::internalLinkTypeWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::internalLinkTypeWebApp &>(object));
    case td_api::inviteGroupCallParticipantResultSuccess::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inviteGroupCallParticipantResultSuccess &)>(td_api::to_json)(jv, static_cast<const td_api::inviteGroupCallParticipantResultSuccess &>(object));
    case td_api::inviteGroupCallParticipantResultUserAlreadyParticipant::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inviteGroupCallParticipantResultUserAlreadyParticipant &)>(td_api::to_json)(jv, static_cast<const td_api::inviteGroupCallParticipantResultUserAlreadyParticipant &>(object));
    case td_api::inviteGroupCallParticipantResultUserPrivacyRestricted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inviteGroupCallParticipantResultUserPrivacyRestricted &)>(td_api::to_json)(jv, static_cast<const td_api::inviteGroupCallParticipantResultUserPrivacyRestricted &>(object));
    case td_api::inviteGroupCallParticipantResultUserWasBanned::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inviteGroupCallParticipantResultUserWasBanned &)>(td_api::to_json)(jv, static_cast<const td_api::inviteGroupCallParticipantResultUserWasBanned &>(object));
    case td_api::inviteLinkChatTypeBasicGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inviteLinkChatTypeBasicGroup &)>(td_api::to_json)(jv, static_cast<const td_api::inviteLinkChatTypeBasicGroup &>(object));
    case td_api::inviteLinkChatTypeChannel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inviteLinkChatTypeChannel &)>(td_api::to_json)(jv, static_cast<const td_api::inviteLinkChatTypeChannel &>(object));
    case td_api::inviteLinkChatTypeSupergroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::inviteLinkChatTypeSupergroup &)>(td_api::to_json)(jv, static_cast<const td_api::inviteLinkChatTypeSupergroup &>(object));
    case td_api::invoice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::invoice &)>(td_api::to_json)(jv, static_cast<const td_api::invoice &>(object));
    case td_api::jsonObjectMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::jsonObjectMember &)>(td_api::to_json)(jv, static_cast<const td_api::jsonObjectMember &>(object));
    case td_api::jsonValueArray::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::jsonValueArray &)>(td_api::to_json)(jv, static_cast<const td_api::jsonValueArray &>(object));
    case td_api::jsonValueBoolean::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::jsonValueBoolean &)>(td_api::to_json)(jv, static_cast<const td_api::jsonValueBoolean &>(object));
    case td_api::jsonValueNull::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::jsonValueNull &)>(td_api::to_json)(jv, static_cast<const td_api::jsonValueNull &>(object));
    case td_api::jsonValueNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::jsonValueNumber &)>(td_api::to_json)(jv, static_cast<const td_api::jsonValueNumber &>(object));
    case td_api::jsonValueObject::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::jsonValueObject &)>(td_api::to_json)(jv, static_cast<const td_api::jsonValueObject &>(object));
    case td_api::jsonValueString::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::jsonValueString &)>(td_api::to_json)(jv, static_cast<const td_api::jsonValueString &>(object));
    case td_api::keyboardButton::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButton &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButton &>(object));
    case td_api::keyboardButtonTypeRequestChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButtonTypeRequestChat &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButtonTypeRequestChat &>(object));
    case td_api::keyboardButtonTypeRequestLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButtonTypeRequestLocation &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButtonTypeRequestLocation &>(object));
    case td_api::keyboardButtonTypeRequestPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButtonTypeRequestPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButtonTypeRequestPhoneNumber &>(object));
    case td_api::keyboardButtonTypeRequestPoll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButtonTypeRequestPoll &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButtonTypeRequestPoll &>(object));
    case td_api::keyboardButtonTypeRequestUsers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButtonTypeRequestUsers &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButtonTypeRequestUsers &>(object));
    case td_api::keyboardButtonTypeText::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButtonTypeText &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButtonTypeText &>(object));
    case td_api::keyboardButtonTypeWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::keyboardButtonTypeWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::keyboardButtonTypeWebApp &>(object));
    case td_api::labeledPricePart::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::labeledPricePart &)>(td_api::to_json)(jv, static_cast<const td_api::labeledPricePart &>(object));
    case td_api::languagePackInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::languagePackInfo &)>(td_api::to_json)(jv, static_cast<const td_api::languagePackInfo &>(object));
    case td_api::languagePackString::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::languagePackString &)>(td_api::to_json)(jv, static_cast<const td_api::languagePackString &>(object));
    case td_api::languagePackStringValueDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::languagePackStringValueDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::languagePackStringValueDeleted &>(object));
    case td_api::languagePackStringValueOrdinary::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::languagePackStringValueOrdinary &)>(td_api::to_json)(jv, static_cast<const td_api::languagePackStringValueOrdinary &>(object));
    case td_api::languagePackStringValuePluralized::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::languagePackStringValuePluralized &)>(td_api::to_json)(jv, static_cast<const td_api::languagePackStringValuePluralized &>(object));
    case td_api::languagePackStrings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::languagePackStrings &)>(td_api::to_json)(jv, static_cast<const td_api::languagePackStrings &>(object));
    case td_api::linkPreview::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreview &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreview &>(object));
    case td_api::linkPreviewAlbumMediaPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewAlbumMediaPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewAlbumMediaPhoto &>(object));
    case td_api::linkPreviewAlbumMediaVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewAlbumMediaVideo &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewAlbumMediaVideo &>(object));
    case td_api::linkPreviewOptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewOptions &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewOptions &>(object));
    case td_api::linkPreviewTypeAlbum::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeAlbum &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeAlbum &>(object));
    case td_api::linkPreviewTypeAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeAnimation &>(object));
    case td_api::linkPreviewTypeApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeApp &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeApp &>(object));
    case td_api::linkPreviewTypeArticle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeArticle &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeArticle &>(object));
    case td_api::linkPreviewTypeAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeAudio &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeAudio &>(object));
    case td_api::linkPreviewTypeBackground::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeBackground &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeBackground &>(object));
    case td_api::linkPreviewTypeChannelBoost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeChannelBoost &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeChannelBoost &>(object));
    case td_api::linkPreviewTypeChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeChat &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeChat &>(object));
    case td_api::linkPreviewTypeDirectMessagesChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeDirectMessagesChat &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeDirectMessagesChat &>(object));
    case td_api::linkPreviewTypeDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeDocument &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeDocument &>(object));
    case td_api::linkPreviewTypeEmbeddedAnimationPlayer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeEmbeddedAnimationPlayer &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeEmbeddedAnimationPlayer &>(object));
    case td_api::linkPreviewTypeEmbeddedAudioPlayer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeEmbeddedAudioPlayer &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeEmbeddedAudioPlayer &>(object));
    case td_api::linkPreviewTypeEmbeddedVideoPlayer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeEmbeddedVideoPlayer &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeEmbeddedVideoPlayer &>(object));
    case td_api::linkPreviewTypeExternalAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeExternalAudio &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeExternalAudio &>(object));
    case td_api::linkPreviewTypeExternalVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeExternalVideo &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeExternalVideo &>(object));
    case td_api::linkPreviewTypeGiftCollection::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeGiftCollection &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeGiftCollection &>(object));
    case td_api::linkPreviewTypeGroupCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeGroupCall &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeGroupCall &>(object));
    case td_api::linkPreviewTypeInvoice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeInvoice &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeInvoice &>(object));
    case td_api::linkPreviewTypeMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeMessage &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeMessage &>(object));
    case td_api::linkPreviewTypePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypePhoto &>(object));
    case td_api::linkPreviewTypePremiumGiftCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypePremiumGiftCode &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypePremiumGiftCode &>(object));
    case td_api::linkPreviewTypeShareableChatFolder::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeShareableChatFolder &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeShareableChatFolder &>(object));
    case td_api::linkPreviewTypeSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeSticker &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeSticker &>(object));
    case td_api::linkPreviewTypeStickerSet::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeStickerSet &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeStickerSet &>(object));
    case td_api::linkPreviewTypeStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeStory &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeStory &>(object));
    case td_api::linkPreviewTypeStoryAlbum::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeStoryAlbum &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeStoryAlbum &>(object));
    case td_api::linkPreviewTypeSupergroupBoost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeSupergroupBoost &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeSupergroupBoost &>(object));
    case td_api::linkPreviewTypeTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeTheme &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeTheme &>(object));
    case td_api::linkPreviewTypeUnsupported::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeUnsupported &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeUnsupported &>(object));
    case td_api::linkPreviewTypeUpgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeUpgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeUpgradedGift &>(object));
    case td_api::linkPreviewTypeUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeUser &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeUser &>(object));
    case td_api::linkPreviewTypeVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeVideo &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeVideo &>(object));
    case td_api::linkPreviewTypeVideoChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeVideoChat &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeVideoChat &>(object));
    case td_api::linkPreviewTypeVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeVideoNote &>(object));
    case td_api::linkPreviewTypeVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeVoiceNote &>(object));
    case td_api::linkPreviewTypeWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::linkPreviewTypeWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::linkPreviewTypeWebApp &>(object));
    case td_api::localFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::localFile &)>(td_api::to_json)(jv, static_cast<const td_api::localFile &>(object));
    case td_api::localizationTargetInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::localizationTargetInfo &)>(td_api::to_json)(jv, static_cast<const td_api::localizationTargetInfo &>(object));
    case td_api::location::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::location &)>(td_api::to_json)(jv, static_cast<const td_api::location &>(object));
    case td_api::locationAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::locationAddress &)>(td_api::to_json)(jv, static_cast<const td_api::locationAddress &>(object));
    case td_api::logStreamDefault::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::logStreamDefault &)>(td_api::to_json)(jv, static_cast<const td_api::logStreamDefault &>(object));
    case td_api::logStreamEmpty::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::logStreamEmpty &)>(td_api::to_json)(jv, static_cast<const td_api::logStreamEmpty &>(object));
    case td_api::logStreamFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::logStreamFile &)>(td_api::to_json)(jv, static_cast<const td_api::logStreamFile &>(object));
    case td_api::logTags::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::logTags &)>(td_api::to_json)(jv, static_cast<const td_api::logTags &>(object));
    case td_api::logVerbosityLevel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::logVerbosityLevel &)>(td_api::to_json)(jv, static_cast<const td_api::logVerbosityLevel &>(object));
    case td_api::loginUrlInfoOpen::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::loginUrlInfoOpen &)>(td_api::to_json)(jv, static_cast<const td_api::loginUrlInfoOpen &>(object));
    case td_api::loginUrlInfoRequestConfirmation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::loginUrlInfoRequestConfirmation &)>(td_api::to_json)(jv, static_cast<const td_api::loginUrlInfoRequestConfirmation &>(object));
    case td_api::mainWebApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::mainWebApp &)>(td_api::to_json)(jv, static_cast<const td_api::mainWebApp &>(object));
    case td_api::maskPointChin::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::maskPointChin &)>(td_api::to_json)(jv, static_cast<const td_api::maskPointChin &>(object));
    case td_api::maskPointEyes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::maskPointEyes &)>(td_api::to_json)(jv, static_cast<const td_api::maskPointEyes &>(object));
    case td_api::maskPointForehead::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::maskPointForehead &)>(td_api::to_json)(jv, static_cast<const td_api::maskPointForehead &>(object));
    case td_api::maskPointMouth::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::maskPointMouth &)>(td_api::to_json)(jv, static_cast<const td_api::maskPointMouth &>(object));
    case td_api::maskPosition::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::maskPosition &)>(td_api::to_json)(jv, static_cast<const td_api::maskPosition &>(object));
    case td_api::message::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::message &)>(td_api::to_json)(jv, static_cast<const td_api::message &>(object));
    case td_api::messageAnimatedEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageAnimatedEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::messageAnimatedEmoji &>(object));
    case td_api::messageAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::messageAnimation &>(object));
    case td_api::messageAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageAudio &)>(td_api::to_json)(jv, static_cast<const td_api::messageAudio &>(object));
    case td_api::messageAutoDeleteTime::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageAutoDeleteTime &)>(td_api::to_json)(jv, static_cast<const td_api::messageAutoDeleteTime &>(object));
    case td_api::messageBasicGroupChatCreate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageBasicGroupChatCreate &)>(td_api::to_json)(jv, static_cast<const td_api::messageBasicGroupChatCreate &>(object));
    case td_api::messageBotWriteAccessAllowed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageBotWriteAccessAllowed &)>(td_api::to_json)(jv, static_cast<const td_api::messageBotWriteAccessAllowed &>(object));
    case td_api::messageCalendar::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageCalendar &)>(td_api::to_json)(jv, static_cast<const td_api::messageCalendar &>(object));
    case td_api::messageCalendarDay::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageCalendarDay &)>(td_api::to_json)(jv, static_cast<const td_api::messageCalendarDay &>(object));
    case td_api::messageCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageCall &)>(td_api::to_json)(jv, static_cast<const td_api::messageCall &>(object));
    case td_api::messageChatAddMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatAddMembers &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatAddMembers &>(object));
    case td_api::messageChatBoost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatBoost &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatBoost &>(object));
    case td_api::messageChatChangePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatChangePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatChangePhoto &>(object));
    case td_api::messageChatChangeTitle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatChangeTitle &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatChangeTitle &>(object));
    case td_api::messageChatDeleteMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatDeleteMember &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatDeleteMember &>(object));
    case td_api::messageChatDeletePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatDeletePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatDeletePhoto &>(object));
    case td_api::messageChatJoinByLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatJoinByLink &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatJoinByLink &>(object));
    case td_api::messageChatJoinByRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatJoinByRequest &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatJoinByRequest &>(object));
    case td_api::messageChatSetBackground::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatSetBackground &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatSetBackground &>(object));
    case td_api::messageChatSetMessageAutoDeleteTime::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatSetMessageAutoDeleteTime &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatSetMessageAutoDeleteTime &>(object));
    case td_api::messageChatSetTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatSetTheme &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatSetTheme &>(object));
    case td_api::messageChatShared::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatShared &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatShared &>(object));
    case td_api::messageChatUpgradeFrom::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatUpgradeFrom &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatUpgradeFrom &>(object));
    case td_api::messageChatUpgradeTo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChatUpgradeTo &)>(td_api::to_json)(jv, static_cast<const td_api::messageChatUpgradeTo &>(object));
    case td_api::messageChecklist::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChecklist &)>(td_api::to_json)(jv, static_cast<const td_api::messageChecklist &>(object));
    case td_api::messageChecklistTasksAdded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChecklistTasksAdded &)>(td_api::to_json)(jv, static_cast<const td_api::messageChecklistTasksAdded &>(object));
    case td_api::messageChecklistTasksDone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageChecklistTasksDone &)>(td_api::to_json)(jv, static_cast<const td_api::messageChecklistTasksDone &>(object));
    case td_api::messageContact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageContact &)>(td_api::to_json)(jv, static_cast<const td_api::messageContact &>(object));
    case td_api::messageContactRegistered::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageContactRegistered &)>(td_api::to_json)(jv, static_cast<const td_api::messageContactRegistered &>(object));
    case td_api::messageCopyOptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageCopyOptions &)>(td_api::to_json)(jv, static_cast<const td_api::messageCopyOptions &>(object));
    case td_api::messageCustomServiceAction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageCustomServiceAction &)>(td_api::to_json)(jv, static_cast<const td_api::messageCustomServiceAction &>(object));
    case td_api::messageDice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageDice &)>(td_api::to_json)(jv, static_cast<const td_api::messageDice &>(object));
    case td_api::messageDirectMessagePriceChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageDirectMessagePriceChanged &)>(td_api::to_json)(jv, static_cast<const td_api::messageDirectMessagePriceChanged &>(object));
    case td_api::messageDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageDocument &)>(td_api::to_json)(jv, static_cast<const td_api::messageDocument &>(object));
    case td_api::messageEffect::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageEffect &)>(td_api::to_json)(jv, static_cast<const td_api::messageEffect &>(object));
    case td_api::messageEffectTypeEmojiReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageEffectTypeEmojiReaction &)>(td_api::to_json)(jv, static_cast<const td_api::messageEffectTypeEmojiReaction &>(object));
    case td_api::messageEffectTypePremiumSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageEffectTypePremiumSticker &)>(td_api::to_json)(jv, static_cast<const td_api::messageEffectTypePremiumSticker &>(object));
    case td_api::messageExpiredPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageExpiredPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::messageExpiredPhoto &>(object));
    case td_api::messageExpiredVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageExpiredVideo &)>(td_api::to_json)(jv, static_cast<const td_api::messageExpiredVideo &>(object));
    case td_api::messageExpiredVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageExpiredVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::messageExpiredVideoNote &>(object));
    case td_api::messageExpiredVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageExpiredVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::messageExpiredVoiceNote &>(object));
    case td_api::messageFileTypeGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageFileTypeGroup &)>(td_api::to_json)(jv, static_cast<const td_api::messageFileTypeGroup &>(object));
    case td_api::messageFileTypePrivate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageFileTypePrivate &)>(td_api::to_json)(jv, static_cast<const td_api::messageFileTypePrivate &>(object));
    case td_api::messageFileTypeUnknown::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageFileTypeUnknown &)>(td_api::to_json)(jv, static_cast<const td_api::messageFileTypeUnknown &>(object));
    case td_api::messageForumTopicCreated::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageForumTopicCreated &)>(td_api::to_json)(jv, static_cast<const td_api::messageForumTopicCreated &>(object));
    case td_api::messageForumTopicEdited::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageForumTopicEdited &)>(td_api::to_json)(jv, static_cast<const td_api::messageForumTopicEdited &>(object));
    case td_api::messageForumTopicIsClosedToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageForumTopicIsClosedToggled &)>(td_api::to_json)(jv, static_cast<const td_api::messageForumTopicIsClosedToggled &>(object));
    case td_api::messageForumTopicIsHiddenToggled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageForumTopicIsHiddenToggled &)>(td_api::to_json)(jv, static_cast<const td_api::messageForumTopicIsHiddenToggled &>(object));
    case td_api::messageForwardInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageForwardInfo &)>(td_api::to_json)(jv, static_cast<const td_api::messageForwardInfo &>(object));
    case td_api::messageGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGame &)>(td_api::to_json)(jv, static_cast<const td_api::messageGame &>(object));
    case td_api::messageGameScore::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGameScore &)>(td_api::to_json)(jv, static_cast<const td_api::messageGameScore &>(object));
    case td_api::messageGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGift &)>(td_api::to_json)(jv, static_cast<const td_api::messageGift &>(object));
    case td_api::messageGiftedPremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiftedPremium &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiftedPremium &>(object));
    case td_api::messageGiftedStars::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiftedStars &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiftedStars &>(object));
    case td_api::messageGiftedTon::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiftedTon &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiftedTon &>(object));
    case td_api::messageGiveaway::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiveaway &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiveaway &>(object));
    case td_api::messageGiveawayCompleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiveawayCompleted &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiveawayCompleted &>(object));
    case td_api::messageGiveawayCreated::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiveawayCreated &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiveawayCreated &>(object));
    case td_api::messageGiveawayPrizeStars::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiveawayPrizeStars &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiveawayPrizeStars &>(object));
    case td_api::messageGiveawayWinners::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGiveawayWinners &)>(td_api::to_json)(jv, static_cast<const td_api::messageGiveawayWinners &>(object));
    case td_api::messageGroupCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageGroupCall &)>(td_api::to_json)(jv, static_cast<const td_api::messageGroupCall &>(object));
    case td_api::messageImportInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageImportInfo &)>(td_api::to_json)(jv, static_cast<const td_api::messageImportInfo &>(object));
    case td_api::messageInteractionInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageInteractionInfo &)>(td_api::to_json)(jv, static_cast<const td_api::messageInteractionInfo &>(object));
    case td_api::messageInviteVideoChatParticipants::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageInviteVideoChatParticipants &)>(td_api::to_json)(jv, static_cast<const td_api::messageInviteVideoChatParticipants &>(object));
    case td_api::messageInvoice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageInvoice &)>(td_api::to_json)(jv, static_cast<const td_api::messageInvoice &>(object));
    case td_api::messageLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageLink &)>(td_api::to_json)(jv, static_cast<const td_api::messageLink &>(object));
    case td_api::messageLinkInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageLinkInfo &)>(td_api::to_json)(jv, static_cast<const td_api::messageLinkInfo &>(object));
    case td_api::messageLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageLocation &)>(td_api::to_json)(jv, static_cast<const td_api::messageLocation &>(object));
    case td_api::messageOriginChannel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageOriginChannel &)>(td_api::to_json)(jv, static_cast<const td_api::messageOriginChannel &>(object));
    case td_api::messageOriginChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageOriginChat &)>(td_api::to_json)(jv, static_cast<const td_api::messageOriginChat &>(object));
    case td_api::messageOriginHiddenUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageOriginHiddenUser &)>(td_api::to_json)(jv, static_cast<const td_api::messageOriginHiddenUser &>(object));
    case td_api::messageOriginUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageOriginUser &)>(td_api::to_json)(jv, static_cast<const td_api::messageOriginUser &>(object));
    case td_api::messagePaidMedia::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePaidMedia &)>(td_api::to_json)(jv, static_cast<const td_api::messagePaidMedia &>(object));
    case td_api::messagePaidMessagePriceChanged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePaidMessagePriceChanged &)>(td_api::to_json)(jv, static_cast<const td_api::messagePaidMessagePriceChanged &>(object));
    case td_api::messagePaidMessagesRefunded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePaidMessagesRefunded &)>(td_api::to_json)(jv, static_cast<const td_api::messagePaidMessagesRefunded &>(object));
    case td_api::messagePassportDataReceived::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePassportDataReceived &)>(td_api::to_json)(jv, static_cast<const td_api::messagePassportDataReceived &>(object));
    case td_api::messagePassportDataSent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePassportDataSent &)>(td_api::to_json)(jv, static_cast<const td_api::messagePassportDataSent &>(object));
    case td_api::messagePaymentRefunded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePaymentRefunded &)>(td_api::to_json)(jv, static_cast<const td_api::messagePaymentRefunded &>(object));
    case td_api::messagePaymentSuccessful::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePaymentSuccessful &)>(td_api::to_json)(jv, static_cast<const td_api::messagePaymentSuccessful &>(object));
    case td_api::messagePaymentSuccessfulBot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePaymentSuccessfulBot &)>(td_api::to_json)(jv, static_cast<const td_api::messagePaymentSuccessfulBot &>(object));
    case td_api::messagePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::messagePhoto &>(object));
    case td_api::messagePinMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePinMessage &)>(td_api::to_json)(jv, static_cast<const td_api::messagePinMessage &>(object));
    case td_api::messagePoll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePoll &)>(td_api::to_json)(jv, static_cast<const td_api::messagePoll &>(object));
    case td_api::messagePosition::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePosition &)>(td_api::to_json)(jv, static_cast<const td_api::messagePosition &>(object));
    case td_api::messagePositions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePositions &)>(td_api::to_json)(jv, static_cast<const td_api::messagePositions &>(object));
    case td_api::messagePremiumGiftCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messagePremiumGiftCode &)>(td_api::to_json)(jv, static_cast<const td_api::messagePremiumGiftCode &>(object));
    case td_api::messageProperties::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageProperties &)>(td_api::to_json)(jv, static_cast<const td_api::messageProperties &>(object));
    case td_api::messageProximityAlertTriggered::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageProximityAlertTriggered &)>(td_api::to_json)(jv, static_cast<const td_api::messageProximityAlertTriggered &>(object));
    case td_api::messageReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReaction &)>(td_api::to_json)(jv, static_cast<const td_api::messageReaction &>(object));
    case td_api::messageReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReactions &)>(td_api::to_json)(jv, static_cast<const td_api::messageReactions &>(object));
    case td_api::messageReadDateMyPrivacyRestricted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReadDateMyPrivacyRestricted &)>(td_api::to_json)(jv, static_cast<const td_api::messageReadDateMyPrivacyRestricted &>(object));
    case td_api::messageReadDateRead::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReadDateRead &)>(td_api::to_json)(jv, static_cast<const td_api::messageReadDateRead &>(object));
    case td_api::messageReadDateTooOld::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReadDateTooOld &)>(td_api::to_json)(jv, static_cast<const td_api::messageReadDateTooOld &>(object));
    case td_api::messageReadDateUnread::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReadDateUnread &)>(td_api::to_json)(jv, static_cast<const td_api::messageReadDateUnread &>(object));
    case td_api::messageReadDateUserPrivacyRestricted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReadDateUserPrivacyRestricted &)>(td_api::to_json)(jv, static_cast<const td_api::messageReadDateUserPrivacyRestricted &>(object));
    case td_api::messageRefundedUpgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageRefundedUpgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::messageRefundedUpgradedGift &>(object));
    case td_api::messageReplyInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReplyInfo &)>(td_api::to_json)(jv, static_cast<const td_api::messageReplyInfo &>(object));
    case td_api::messageReplyToMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReplyToMessage &)>(td_api::to_json)(jv, static_cast<const td_api::messageReplyToMessage &>(object));
    case td_api::messageReplyToStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageReplyToStory &)>(td_api::to_json)(jv, static_cast<const td_api::messageReplyToStory &>(object));
    case td_api::messageSchedulingStateSendAtDate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSchedulingStateSendAtDate &)>(td_api::to_json)(jv, static_cast<const td_api::messageSchedulingStateSendAtDate &>(object));
    case td_api::messageSchedulingStateSendWhenOnline::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSchedulingStateSendWhenOnline &)>(td_api::to_json)(jv, static_cast<const td_api::messageSchedulingStateSendWhenOnline &>(object));
    case td_api::messageSchedulingStateSendWhenVideoProcessed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSchedulingStateSendWhenVideoProcessed &)>(td_api::to_json)(jv, static_cast<const td_api::messageSchedulingStateSendWhenVideoProcessed &>(object));
    case td_api::messageScreenshotTaken::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageScreenshotTaken &)>(td_api::to_json)(jv, static_cast<const td_api::messageScreenshotTaken &>(object));
    case td_api::messageSelfDestructTypeImmediately::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSelfDestructTypeImmediately &)>(td_api::to_json)(jv, static_cast<const td_api::messageSelfDestructTypeImmediately &>(object));
    case td_api::messageSelfDestructTypeTimer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSelfDestructTypeTimer &)>(td_api::to_json)(jv, static_cast<const td_api::messageSelfDestructTypeTimer &>(object));
    case td_api::messageSenderChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSenderChat &)>(td_api::to_json)(jv, static_cast<const td_api::messageSenderChat &>(object));
    case td_api::messageSenderUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSenderUser &)>(td_api::to_json)(jv, static_cast<const td_api::messageSenderUser &>(object));
    case td_api::messageSenders::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSenders &)>(td_api::to_json)(jv, static_cast<const td_api::messageSenders &>(object));
    case td_api::messageSendingStateFailed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSendingStateFailed &)>(td_api::to_json)(jv, static_cast<const td_api::messageSendingStateFailed &>(object));
    case td_api::messageSendingStatePending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSendingStatePending &)>(td_api::to_json)(jv, static_cast<const td_api::messageSendingStatePending &>(object));
    case td_api::messageStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::messageStatistics &>(object));
    case td_api::messageSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSticker &)>(td_api::to_json)(jv, static_cast<const td_api::messageSticker &>(object));
    case td_api::messageStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageStory &)>(td_api::to_json)(jv, static_cast<const td_api::messageStory &>(object));
    case td_api::messageSuggestBirthdate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSuggestBirthdate &)>(td_api::to_json)(jv, static_cast<const td_api::messageSuggestBirthdate &>(object));
    case td_api::messageSuggestProfilePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSuggestProfilePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::messageSuggestProfilePhoto &>(object));
    case td_api::messageSuggestedPostApprovalFailed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSuggestedPostApprovalFailed &)>(td_api::to_json)(jv, static_cast<const td_api::messageSuggestedPostApprovalFailed &>(object));
    case td_api::messageSuggestedPostApproved::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSuggestedPostApproved &)>(td_api::to_json)(jv, static_cast<const td_api::messageSuggestedPostApproved &>(object));
    case td_api::messageSuggestedPostDeclined::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSuggestedPostDeclined &)>(td_api::to_json)(jv, static_cast<const td_api::messageSuggestedPostDeclined &>(object));
    case td_api::messageSuggestedPostPaid::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSuggestedPostPaid &)>(td_api::to_json)(jv, static_cast<const td_api::messageSuggestedPostPaid &>(object));
    case td_api::messageSuggestedPostRefunded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSuggestedPostRefunded &)>(td_api::to_json)(jv, static_cast<const td_api::messageSuggestedPostRefunded &>(object));
    case td_api::messageSupergroupChatCreate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageSupergroupChatCreate &)>(td_api::to_json)(jv, static_cast<const td_api::messageSupergroupChatCreate &>(object));
    case td_api::messageText::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageText &)>(td_api::to_json)(jv, static_cast<const td_api::messageText &>(object));
    case td_api::messageThreadInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageThreadInfo &)>(td_api::to_json)(jv, static_cast<const td_api::messageThreadInfo &>(object));
    case td_api::messageTopicDirectMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageTopicDirectMessages &)>(td_api::to_json)(jv, static_cast<const td_api::messageTopicDirectMessages &>(object));
    case td_api::messageTopicForum::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageTopicForum &)>(td_api::to_json)(jv, static_cast<const td_api::messageTopicForum &>(object));
    case td_api::messageTopicSavedMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageTopicSavedMessages &)>(td_api::to_json)(jv, static_cast<const td_api::messageTopicSavedMessages &>(object));
    case td_api::messageTopicThread::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageTopicThread &)>(td_api::to_json)(jv, static_cast<const td_api::messageTopicThread &>(object));
    case td_api::messageUnsupported::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageUnsupported &)>(td_api::to_json)(jv, static_cast<const td_api::messageUnsupported &>(object));
    case td_api::messageUpgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageUpgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::messageUpgradedGift &>(object));
    case td_api::messageUsersShared::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageUsersShared &)>(td_api::to_json)(jv, static_cast<const td_api::messageUsersShared &>(object));
    case td_api::messageVenue::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageVenue &)>(td_api::to_json)(jv, static_cast<const td_api::messageVenue &>(object));
    case td_api::messageVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageVideo &)>(td_api::to_json)(jv, static_cast<const td_api::messageVideo &>(object));
    case td_api::messageVideoChatEnded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageVideoChatEnded &)>(td_api::to_json)(jv, static_cast<const td_api::messageVideoChatEnded &>(object));
    case td_api::messageVideoChatScheduled::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageVideoChatScheduled &)>(td_api::to_json)(jv, static_cast<const td_api::messageVideoChatScheduled &>(object));
    case td_api::messageVideoChatStarted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageVideoChatStarted &)>(td_api::to_json)(jv, static_cast<const td_api::messageVideoChatStarted &>(object));
    case td_api::messageVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::messageVideoNote &>(object));
    case td_api::messageViewer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageViewer &)>(td_api::to_json)(jv, static_cast<const td_api::messageViewer &>(object));
    case td_api::messageViewers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageViewers &)>(td_api::to_json)(jv, static_cast<const td_api::messageViewers &>(object));
    case td_api::messageVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::messageVoiceNote &>(object));
    case td_api::messageWebAppDataReceived::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageWebAppDataReceived &)>(td_api::to_json)(jv, static_cast<const td_api::messageWebAppDataReceived &>(object));
    case td_api::messageWebAppDataSent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messageWebAppDataSent &)>(td_api::to_json)(jv, static_cast<const td_api::messageWebAppDataSent &>(object));
    case td_api::messages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::messages &)>(td_api::to_json)(jv, static_cast<const td_api::messages &>(object));
    case td_api::minithumbnail::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::minithumbnail &)>(td_api::to_json)(jv, static_cast<const td_api::minithumbnail &>(object));
    case td_api::networkStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::networkStatistics &>(object));
    case td_api::networkStatisticsEntryCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkStatisticsEntryCall &)>(td_api::to_json)(jv, static_cast<const td_api::networkStatisticsEntryCall &>(object));
    case td_api::networkStatisticsEntryFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkStatisticsEntryFile &)>(td_api::to_json)(jv, static_cast<const td_api::networkStatisticsEntryFile &>(object));
    case td_api::networkTypeMobile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkTypeMobile &)>(td_api::to_json)(jv, static_cast<const td_api::networkTypeMobile &>(object));
    case td_api::networkTypeMobileRoaming::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkTypeMobileRoaming &)>(td_api::to_json)(jv, static_cast<const td_api::networkTypeMobileRoaming &>(object));
    case td_api::networkTypeNone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkTypeNone &)>(td_api::to_json)(jv, static_cast<const td_api::networkTypeNone &>(object));
    case td_api::networkTypeOther::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkTypeOther &)>(td_api::to_json)(jv, static_cast<const td_api::networkTypeOther &>(object));
    case td_api::networkTypeWiFi::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::networkTypeWiFi &)>(td_api::to_json)(jv, static_cast<const td_api::networkTypeWiFi &>(object));
    case td_api::newChatPrivacySettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::newChatPrivacySettings &)>(td_api::to_json)(jv, static_cast<const td_api::newChatPrivacySettings &>(object));
    case td_api::notification::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notification &)>(td_api::to_json)(jv, static_cast<const td_api::notification &>(object));
    case td_api::notificationGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationGroup &)>(td_api::to_json)(jv, static_cast<const td_api::notificationGroup &>(object));
    case td_api::notificationGroupTypeCalls::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationGroupTypeCalls &)>(td_api::to_json)(jv, static_cast<const td_api::notificationGroupTypeCalls &>(object));
    case td_api::notificationGroupTypeMentions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationGroupTypeMentions &)>(td_api::to_json)(jv, static_cast<const td_api::notificationGroupTypeMentions &>(object));
    case td_api::notificationGroupTypeMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationGroupTypeMessages &)>(td_api::to_json)(jv, static_cast<const td_api::notificationGroupTypeMessages &>(object));
    case td_api::notificationGroupTypeSecretChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationGroupTypeSecretChat &)>(td_api::to_json)(jv, static_cast<const td_api::notificationGroupTypeSecretChat &>(object));
    case td_api::notificationSettingsScopeChannelChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationSettingsScopeChannelChats &)>(td_api::to_json)(jv, static_cast<const td_api::notificationSettingsScopeChannelChats &>(object));
    case td_api::notificationSettingsScopeGroupChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationSettingsScopeGroupChats &)>(td_api::to_json)(jv, static_cast<const td_api::notificationSettingsScopeGroupChats &>(object));
    case td_api::notificationSettingsScopePrivateChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationSettingsScopePrivateChats &)>(td_api::to_json)(jv, static_cast<const td_api::notificationSettingsScopePrivateChats &>(object));
    case td_api::notificationSound::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationSound &)>(td_api::to_json)(jv, static_cast<const td_api::notificationSound &>(object));
    case td_api::notificationSounds::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationSounds &)>(td_api::to_json)(jv, static_cast<const td_api::notificationSounds &>(object));
    case td_api::notificationTypeNewCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationTypeNewCall &)>(td_api::to_json)(jv, static_cast<const td_api::notificationTypeNewCall &>(object));
    case td_api::notificationTypeNewMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationTypeNewMessage &)>(td_api::to_json)(jv, static_cast<const td_api::notificationTypeNewMessage &>(object));
    case td_api::notificationTypeNewPushMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationTypeNewPushMessage &)>(td_api::to_json)(jv, static_cast<const td_api::notificationTypeNewPushMessage &>(object));
    case td_api::notificationTypeNewSecretChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::notificationTypeNewSecretChat &)>(td_api::to_json)(jv, static_cast<const td_api::notificationTypeNewSecretChat &>(object));
    case td_api::ok::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::ok &)>(td_api::to_json)(jv, static_cast<const td_api::ok &>(object));
    case td_api::optionValueBoolean::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::optionValueBoolean &)>(td_api::to_json)(jv, static_cast<const td_api::optionValueBoolean &>(object));
    case td_api::optionValueEmpty::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::optionValueEmpty &)>(td_api::to_json)(jv, static_cast<const td_api::optionValueEmpty &>(object));
    case td_api::optionValueInteger::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::optionValueInteger &)>(td_api::to_json)(jv, static_cast<const td_api::optionValueInteger &>(object));
    case td_api::optionValueString::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::optionValueString &)>(td_api::to_json)(jv, static_cast<const td_api::optionValueString &>(object));
    case td_api::orderInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::orderInfo &)>(td_api::to_json)(jv, static_cast<const td_api::orderInfo &>(object));
    case td_api::outline::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::outline &)>(td_api::to_json)(jv, static_cast<const td_api::outline &>(object));
    case td_api::pageBlockAnchor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockAnchor &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockAnchor &>(object));
    case td_api::pageBlockAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockAnimation &>(object));
    case td_api::pageBlockAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockAudio &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockAudio &>(object));
    case td_api::pageBlockAuthorDate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockAuthorDate &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockAuthorDate &>(object));
    case td_api::pageBlockBlockQuote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockBlockQuote &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockBlockQuote &>(object));
    case td_api::pageBlockCaption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockCaption &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockCaption &>(object));
    case td_api::pageBlockChatLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockChatLink &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockChatLink &>(object));
    case td_api::pageBlockCollage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockCollage &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockCollage &>(object));
    case td_api::pageBlockCover::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockCover &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockCover &>(object));
    case td_api::pageBlockDetails::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockDetails &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockDetails &>(object));
    case td_api::pageBlockDivider::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockDivider &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockDivider &>(object));
    case td_api::pageBlockEmbedded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockEmbedded &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockEmbedded &>(object));
    case td_api::pageBlockEmbeddedPost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockEmbeddedPost &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockEmbeddedPost &>(object));
    case td_api::pageBlockFooter::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockFooter &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockFooter &>(object));
    case td_api::pageBlockHeader::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockHeader &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockHeader &>(object));
    case td_api::pageBlockHorizontalAlignmentCenter::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockHorizontalAlignmentCenter &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockHorizontalAlignmentCenter &>(object));
    case td_api::pageBlockHorizontalAlignmentLeft::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockHorizontalAlignmentLeft &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockHorizontalAlignmentLeft &>(object));
    case td_api::pageBlockHorizontalAlignmentRight::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockHorizontalAlignmentRight &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockHorizontalAlignmentRight &>(object));
    case td_api::pageBlockKicker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockKicker &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockKicker &>(object));
    case td_api::pageBlockList::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockList &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockList &>(object));
    case td_api::pageBlockListItem::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockListItem &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockListItem &>(object));
    case td_api::pageBlockMap::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockMap &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockMap &>(object));
    case td_api::pageBlockParagraph::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockParagraph &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockParagraph &>(object));
    case td_api::pageBlockPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockPhoto &>(object));
    case td_api::pageBlockPreformatted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockPreformatted &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockPreformatted &>(object));
    case td_api::pageBlockPullQuote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockPullQuote &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockPullQuote &>(object));
    case td_api::pageBlockRelatedArticle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockRelatedArticle &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockRelatedArticle &>(object));
    case td_api::pageBlockRelatedArticles::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockRelatedArticles &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockRelatedArticles &>(object));
    case td_api::pageBlockSlideshow::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockSlideshow &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockSlideshow &>(object));
    case td_api::pageBlockSubheader::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockSubheader &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockSubheader &>(object));
    case td_api::pageBlockSubtitle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockSubtitle &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockSubtitle &>(object));
    case td_api::pageBlockTable::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockTable &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockTable &>(object));
    case td_api::pageBlockTableCell::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockTableCell &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockTableCell &>(object));
    case td_api::pageBlockTitle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockTitle &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockTitle &>(object));
    case td_api::pageBlockVerticalAlignmentBottom::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockVerticalAlignmentBottom &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockVerticalAlignmentBottom &>(object));
    case td_api::pageBlockVerticalAlignmentMiddle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockVerticalAlignmentMiddle &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockVerticalAlignmentMiddle &>(object));
    case td_api::pageBlockVerticalAlignmentTop::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockVerticalAlignmentTop &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockVerticalAlignmentTop &>(object));
    case td_api::pageBlockVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockVideo &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockVideo &>(object));
    case td_api::pageBlockVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pageBlockVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::pageBlockVoiceNote &>(object));
    case td_api::paidMediaPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidMediaPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::paidMediaPhoto &>(object));
    case td_api::paidMediaPreview::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidMediaPreview &)>(td_api::to_json)(jv, static_cast<const td_api::paidMediaPreview &>(object));
    case td_api::paidMediaUnsupported::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidMediaUnsupported &)>(td_api::to_json)(jv, static_cast<const td_api::paidMediaUnsupported &>(object));
    case td_api::paidMediaVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidMediaVideo &)>(td_api::to_json)(jv, static_cast<const td_api::paidMediaVideo &>(object));
    case td_api::paidReactionTypeAnonymous::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidReactionTypeAnonymous &)>(td_api::to_json)(jv, static_cast<const td_api::paidReactionTypeAnonymous &>(object));
    case td_api::paidReactionTypeChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidReactionTypeChat &)>(td_api::to_json)(jv, static_cast<const td_api::paidReactionTypeChat &>(object));
    case td_api::paidReactionTypeRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidReactionTypeRegular &)>(td_api::to_json)(jv, static_cast<const td_api::paidReactionTypeRegular &>(object));
    case td_api::paidReactor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paidReactor &)>(td_api::to_json)(jv, static_cast<const td_api::paidReactor &>(object));
    case td_api::passportAuthorizationForm::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportAuthorizationForm &)>(td_api::to_json)(jv, static_cast<const td_api::passportAuthorizationForm &>(object));
    case td_api::passportElementAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementAddress &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementAddress &>(object));
    case td_api::passportElementBankStatement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementBankStatement &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementBankStatement &>(object));
    case td_api::passportElementDriverLicense::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementDriverLicense &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementDriverLicense &>(object));
    case td_api::passportElementEmailAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementEmailAddress &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementEmailAddress &>(object));
    case td_api::passportElementError::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementError &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementError &>(object));
    case td_api::passportElementErrorSourceDataField::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceDataField &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceDataField &>(object));
    case td_api::passportElementErrorSourceFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceFile &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceFile &>(object));
    case td_api::passportElementErrorSourceFiles::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceFiles &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceFiles &>(object));
    case td_api::passportElementErrorSourceFrontSide::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceFrontSide &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceFrontSide &>(object));
    case td_api::passportElementErrorSourceReverseSide::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceReverseSide &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceReverseSide &>(object));
    case td_api::passportElementErrorSourceSelfie::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceSelfie &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceSelfie &>(object));
    case td_api::passportElementErrorSourceTranslationFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceTranslationFile &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceTranslationFile &>(object));
    case td_api::passportElementErrorSourceTranslationFiles::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceTranslationFiles &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceTranslationFiles &>(object));
    case td_api::passportElementErrorSourceUnspecified::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementErrorSourceUnspecified &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementErrorSourceUnspecified &>(object));
    case td_api::passportElementIdentityCard::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementIdentityCard &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementIdentityCard &>(object));
    case td_api::passportElementInternalPassport::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementInternalPassport &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementInternalPassport &>(object));
    case td_api::passportElementPassport::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementPassport &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementPassport &>(object));
    case td_api::passportElementPassportRegistration::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementPassportRegistration &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementPassportRegistration &>(object));
    case td_api::passportElementPersonalDetails::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementPersonalDetails &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementPersonalDetails &>(object));
    case td_api::passportElementPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementPhoneNumber &>(object));
    case td_api::passportElementRentalAgreement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementRentalAgreement &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementRentalAgreement &>(object));
    case td_api::passportElementTemporaryRegistration::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTemporaryRegistration &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTemporaryRegistration &>(object));
    case td_api::passportElementTypeAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeAddress &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeAddress &>(object));
    case td_api::passportElementTypeBankStatement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeBankStatement &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeBankStatement &>(object));
    case td_api::passportElementTypeDriverLicense::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeDriverLicense &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeDriverLicense &>(object));
    case td_api::passportElementTypeEmailAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeEmailAddress &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeEmailAddress &>(object));
    case td_api::passportElementTypeIdentityCard::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeIdentityCard &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeIdentityCard &>(object));
    case td_api::passportElementTypeInternalPassport::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeInternalPassport &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeInternalPassport &>(object));
    case td_api::passportElementTypePassport::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypePassport &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypePassport &>(object));
    case td_api::passportElementTypePassportRegistration::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypePassportRegistration &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypePassportRegistration &>(object));
    case td_api::passportElementTypePersonalDetails::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypePersonalDetails &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypePersonalDetails &>(object));
    case td_api::passportElementTypePhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypePhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypePhoneNumber &>(object));
    case td_api::passportElementTypeRentalAgreement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeRentalAgreement &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeRentalAgreement &>(object));
    case td_api::passportElementTypeTemporaryRegistration::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeTemporaryRegistration &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeTemporaryRegistration &>(object));
    case td_api::passportElementTypeUtilityBill::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementTypeUtilityBill &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementTypeUtilityBill &>(object));
    case td_api::passportElementUtilityBill::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementUtilityBill &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementUtilityBill &>(object));
    case td_api::passportElements::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElements &)>(td_api::to_json)(jv, static_cast<const td_api::passportElements &>(object));
    case td_api::passportElementsWithErrors::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportElementsWithErrors &)>(td_api::to_json)(jv, static_cast<const td_api::passportElementsWithErrors &>(object));
    case td_api::passportRequiredElement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportRequiredElement &)>(td_api::to_json)(jv, static_cast<const td_api::passportRequiredElement &>(object));
    case td_api::passportSuitableElement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passportSuitableElement &)>(td_api::to_json)(jv, static_cast<const td_api::passportSuitableElement &>(object));
    case td_api::passwordState::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::passwordState &)>(td_api::to_json)(jv, static_cast<const td_api::passwordState &>(object));
    case td_api::paymentForm::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentForm &)>(td_api::to_json)(jv, static_cast<const td_api::paymentForm &>(object));
    case td_api::paymentFormTypeRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentFormTypeRegular &)>(td_api::to_json)(jv, static_cast<const td_api::paymentFormTypeRegular &>(object));
    case td_api::paymentFormTypeStarSubscription::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentFormTypeStarSubscription &)>(td_api::to_json)(jv, static_cast<const td_api::paymentFormTypeStarSubscription &>(object));
    case td_api::paymentFormTypeStars::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentFormTypeStars &)>(td_api::to_json)(jv, static_cast<const td_api::paymentFormTypeStars &>(object));
    case td_api::paymentOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentOption &)>(td_api::to_json)(jv, static_cast<const td_api::paymentOption &>(object));
    case td_api::paymentProviderOther::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentProviderOther &)>(td_api::to_json)(jv, static_cast<const td_api::paymentProviderOther &>(object));
    case td_api::paymentProviderSmartGlocal::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentProviderSmartGlocal &)>(td_api::to_json)(jv, static_cast<const td_api::paymentProviderSmartGlocal &>(object));
    case td_api::paymentProviderStripe::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentProviderStripe &)>(td_api::to_json)(jv, static_cast<const td_api::paymentProviderStripe &>(object));
    case td_api::paymentReceipt::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentReceipt &)>(td_api::to_json)(jv, static_cast<const td_api::paymentReceipt &>(object));
    case td_api::paymentReceiptTypeRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentReceiptTypeRegular &)>(td_api::to_json)(jv, static_cast<const td_api::paymentReceiptTypeRegular &>(object));
    case td_api::paymentReceiptTypeStars::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentReceiptTypeStars &)>(td_api::to_json)(jv, static_cast<const td_api::paymentReceiptTypeStars &>(object));
    case td_api::paymentResult::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::paymentResult &)>(td_api::to_json)(jv, static_cast<const td_api::paymentResult &>(object));
    case td_api::personalDetails::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::personalDetails &)>(td_api::to_json)(jv, static_cast<const td_api::personalDetails &>(object));
    case td_api::personalDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::personalDocument &)>(td_api::to_json)(jv, static_cast<const td_api::personalDocument &>(object));
    case td_api::phoneNumberInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::phoneNumberInfo &)>(td_api::to_json)(jv, static_cast<const td_api::phoneNumberInfo &>(object));
    case td_api::photo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::photo &)>(td_api::to_json)(jv, static_cast<const td_api::photo &>(object));
    case td_api::photoSize::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::photoSize &)>(td_api::to_json)(jv, static_cast<const td_api::photoSize &>(object));
    case td_api::point::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::point &)>(td_api::to_json)(jv, static_cast<const td_api::point &>(object));
    case td_api::poll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::poll &)>(td_api::to_json)(jv, static_cast<const td_api::poll &>(object));
    case td_api::pollOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pollOption &)>(td_api::to_json)(jv, static_cast<const td_api::pollOption &>(object));
    case td_api::pollTypeQuiz::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pollTypeQuiz &)>(td_api::to_json)(jv, static_cast<const td_api::pollTypeQuiz &>(object));
    case td_api::pollTypeRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pollTypeRegular &)>(td_api::to_json)(jv, static_cast<const td_api::pollTypeRegular &>(object));
    case td_api::premiumFeatureAccentColor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureAccentColor &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureAccentColor &>(object));
    case td_api::premiumFeatureAdvancedChatManagement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureAdvancedChatManagement &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureAdvancedChatManagement &>(object));
    case td_api::premiumFeatureAnimatedProfilePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureAnimatedProfilePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureAnimatedProfilePhoto &>(object));
    case td_api::premiumFeatureAppIcons::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureAppIcons &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureAppIcons &>(object));
    case td_api::premiumFeatureBackgroundForBoth::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureBackgroundForBoth &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureBackgroundForBoth &>(object));
    case td_api::premiumFeatureBusiness::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureBusiness &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureBusiness &>(object));
    case td_api::premiumFeatureChatBoost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureChatBoost &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureChatBoost &>(object));
    case td_api::premiumFeatureChecklists::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureChecklists &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureChecklists &>(object));
    case td_api::premiumFeatureCustomEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureCustomEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureCustomEmoji &>(object));
    case td_api::premiumFeatureDisabledAds::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureDisabledAds &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureDisabledAds &>(object));
    case td_api::premiumFeatureEmojiStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureEmojiStatus &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureEmojiStatus &>(object));
    case td_api::premiumFeatureForumTopicIcon::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureForumTopicIcon &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureForumTopicIcon &>(object));
    case td_api::premiumFeatureImprovedDownloadSpeed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureImprovedDownloadSpeed &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureImprovedDownloadSpeed &>(object));
    case td_api::premiumFeatureIncreasedLimits::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureIncreasedLimits &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureIncreasedLimits &>(object));
    case td_api::premiumFeatureIncreasedUploadFileSize::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureIncreasedUploadFileSize &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureIncreasedUploadFileSize &>(object));
    case td_api::premiumFeatureLastSeenTimes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureLastSeenTimes &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureLastSeenTimes &>(object));
    case td_api::premiumFeatureMessageEffects::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureMessageEffects &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureMessageEffects &>(object));
    case td_api::premiumFeatureMessagePrivacy::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureMessagePrivacy &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureMessagePrivacy &>(object));
    case td_api::premiumFeatureProfileBadge::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureProfileBadge &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureProfileBadge &>(object));
    case td_api::premiumFeaturePromotionAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeaturePromotionAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeaturePromotionAnimation &>(object));
    case td_api::premiumFeatureRealTimeChatTranslation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureRealTimeChatTranslation &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureRealTimeChatTranslation &>(object));
    case td_api::premiumFeatureSavedMessagesTags::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureSavedMessagesTags &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureSavedMessagesTags &>(object));
    case td_api::premiumFeatureUniqueReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureUniqueReactions &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureUniqueReactions &>(object));
    case td_api::premiumFeatureUniqueStickers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureUniqueStickers &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureUniqueStickers &>(object));
    case td_api::premiumFeatureUpgradedStories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureUpgradedStories &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureUpgradedStories &>(object));
    case td_api::premiumFeatureVoiceRecognition::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatureVoiceRecognition &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatureVoiceRecognition &>(object));
    case td_api::premiumFeatures::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumFeatures &)>(td_api::to_json)(jv, static_cast<const td_api::premiumFeatures &>(object));
    case td_api::premiumGiftCodeInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumGiftCodeInfo &)>(td_api::to_json)(jv, static_cast<const td_api::premiumGiftCodeInfo &>(object));
    case td_api::premiumGiftPaymentOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumGiftPaymentOption &)>(td_api::to_json)(jv, static_cast<const td_api::premiumGiftPaymentOption &>(object));
    case td_api::premiumGiftPaymentOptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumGiftPaymentOptions &)>(td_api::to_json)(jv, static_cast<const td_api::premiumGiftPaymentOptions &>(object));
    case td_api::premiumGiveawayPaymentOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumGiveawayPaymentOption &)>(td_api::to_json)(jv, static_cast<const td_api::premiumGiveawayPaymentOption &>(object));
    case td_api::premiumGiveawayPaymentOptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumGiveawayPaymentOptions &)>(td_api::to_json)(jv, static_cast<const td_api::premiumGiveawayPaymentOptions &>(object));
    case td_api::premiumLimit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimit &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimit &>(object));
    case td_api::premiumLimitTypeActiveStoryCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeActiveStoryCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeActiveStoryCount &>(object));
    case td_api::premiumLimitTypeBioLength::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeBioLength &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeBioLength &>(object));
    case td_api::premiumLimitTypeCaptionLength::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeCaptionLength &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeCaptionLength &>(object));
    case td_api::premiumLimitTypeChatFolderChosenChatCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeChatFolderChosenChatCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeChatFolderChosenChatCount &>(object));
    case td_api::premiumLimitTypeChatFolderCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeChatFolderCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeChatFolderCount &>(object));
    case td_api::premiumLimitTypeChatFolderInviteLinkCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeChatFolderInviteLinkCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeChatFolderInviteLinkCount &>(object));
    case td_api::premiumLimitTypeCreatedPublicChatCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeCreatedPublicChatCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeCreatedPublicChatCount &>(object));
    case td_api::premiumLimitTypeFavoriteStickerCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeFavoriteStickerCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeFavoriteStickerCount &>(object));
    case td_api::premiumLimitTypeMonthlyPostedStoryCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeMonthlyPostedStoryCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeMonthlyPostedStoryCount &>(object));
    case td_api::premiumLimitTypePinnedArchivedChatCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypePinnedArchivedChatCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypePinnedArchivedChatCount &>(object));
    case td_api::premiumLimitTypePinnedChatCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypePinnedChatCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypePinnedChatCount &>(object));
    case td_api::premiumLimitTypePinnedSavedMessagesTopicCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypePinnedSavedMessagesTopicCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypePinnedSavedMessagesTopicCount &>(object));
    case td_api::premiumLimitTypeSavedAnimationCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeSavedAnimationCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeSavedAnimationCount &>(object));
    case td_api::premiumLimitTypeShareableChatFolderCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeShareableChatFolderCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeShareableChatFolderCount &>(object));
    case td_api::premiumLimitTypeSimilarChatCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeSimilarChatCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeSimilarChatCount &>(object));
    case td_api::premiumLimitTypeStoryCaptionLength::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeStoryCaptionLength &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeStoryCaptionLength &>(object));
    case td_api::premiumLimitTypeStorySuggestedReactionAreaCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeStorySuggestedReactionAreaCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeStorySuggestedReactionAreaCount &>(object));
    case td_api::premiumLimitTypeSupergroupCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeSupergroupCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeSupergroupCount &>(object));
    case td_api::premiumLimitTypeWeeklyPostedStoryCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumLimitTypeWeeklyPostedStoryCount &)>(td_api::to_json)(jv, static_cast<const td_api::premiumLimitTypeWeeklyPostedStoryCount &>(object));
    case td_api::premiumPaymentOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumPaymentOption &)>(td_api::to_json)(jv, static_cast<const td_api::premiumPaymentOption &>(object));
    case td_api::premiumState::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumState &)>(td_api::to_json)(jv, static_cast<const td_api::premiumState &>(object));
    case td_api::premiumStatePaymentOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::premiumStatePaymentOption &)>(td_api::to_json)(jv, static_cast<const td_api::premiumStatePaymentOption &>(object));
    case td_api::prepaidGiveaway::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::prepaidGiveaway &)>(td_api::to_json)(jv, static_cast<const td_api::prepaidGiveaway &>(object));
    case td_api::preparedInlineMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::preparedInlineMessage &)>(td_api::to_json)(jv, static_cast<const td_api::preparedInlineMessage &>(object));
    case td_api::preparedInlineMessageId::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::preparedInlineMessageId &)>(td_api::to_json)(jv, static_cast<const td_api::preparedInlineMessageId &>(object));
    case td_api::productInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::productInfo &)>(td_api::to_json)(jv, static_cast<const td_api::productInfo &>(object));
    case td_api::profileAccentColor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileAccentColor &)>(td_api::to_json)(jv, static_cast<const td_api::profileAccentColor &>(object));
    case td_api::profileAccentColors::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileAccentColors &)>(td_api::to_json)(jv, static_cast<const td_api::profileAccentColors &>(object));
    case td_api::profilePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profilePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::profilePhoto &>(object));
    case td_api::profileTabFiles::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabFiles &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabFiles &>(object));
    case td_api::profileTabGifs::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabGifs &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabGifs &>(object));
    case td_api::profileTabGifts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabGifts &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabGifts &>(object));
    case td_api::profileTabLinks::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabLinks &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabLinks &>(object));
    case td_api::profileTabMedia::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabMedia &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabMedia &>(object));
    case td_api::profileTabMusic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabMusic &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabMusic &>(object));
    case td_api::profileTabPosts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabPosts &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabPosts &>(object));
    case td_api::profileTabVoice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::profileTabVoice &)>(td_api::to_json)(jv, static_cast<const td_api::profileTabVoice &>(object));
    case td_api::proxies::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::proxies &)>(td_api::to_json)(jv, static_cast<const td_api::proxies &>(object));
    case td_api::proxy::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::proxy &)>(td_api::to_json)(jv, static_cast<const td_api::proxy &>(object));
    case td_api::proxyTypeHttp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::proxyTypeHttp &)>(td_api::to_json)(jv, static_cast<const td_api::proxyTypeHttp &>(object));
    case td_api::proxyTypeMtproto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::proxyTypeMtproto &)>(td_api::to_json)(jv, static_cast<const td_api::proxyTypeMtproto &>(object));
    case td_api::proxyTypeSocks5::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::proxyTypeSocks5 &)>(td_api::to_json)(jv, static_cast<const td_api::proxyTypeSocks5 &>(object));
    case td_api::publicForwardMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::publicForwardMessage &)>(td_api::to_json)(jv, static_cast<const td_api::publicForwardMessage &>(object));
    case td_api::publicForwardStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::publicForwardStory &)>(td_api::to_json)(jv, static_cast<const td_api::publicForwardStory &>(object));
    case td_api::publicForwards::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::publicForwards &)>(td_api::to_json)(jv, static_cast<const td_api::publicForwards &>(object));
    case td_api::publicPostSearchLimits::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::publicPostSearchLimits &)>(td_api::to_json)(jv, static_cast<const td_api::publicPostSearchLimits &>(object));
    case td_api::pushMessageContentAnimation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentAnimation &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentAnimation &>(object));
    case td_api::pushMessageContentAudio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentAudio &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentAudio &>(object));
    case td_api::pushMessageContentBasicGroupChatCreate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentBasicGroupChatCreate &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentBasicGroupChatCreate &>(object));
    case td_api::pushMessageContentChatAddMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatAddMembers &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatAddMembers &>(object));
    case td_api::pushMessageContentChatChangePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatChangePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatChangePhoto &>(object));
    case td_api::pushMessageContentChatChangeTitle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatChangeTitle &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatChangeTitle &>(object));
    case td_api::pushMessageContentChatDeleteMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatDeleteMember &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatDeleteMember &>(object));
    case td_api::pushMessageContentChatJoinByLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatJoinByLink &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatJoinByLink &>(object));
    case td_api::pushMessageContentChatJoinByRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatJoinByRequest &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatJoinByRequest &>(object));
    case td_api::pushMessageContentChatSetBackground::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatSetBackground &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatSetBackground &>(object));
    case td_api::pushMessageContentChatSetTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChatSetTheme &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChatSetTheme &>(object));
    case td_api::pushMessageContentChecklist::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChecklist &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChecklist &>(object));
    case td_api::pushMessageContentChecklistTasksAdded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChecklistTasksAdded &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChecklistTasksAdded &>(object));
    case td_api::pushMessageContentChecklistTasksDone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentChecklistTasksDone &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentChecklistTasksDone &>(object));
    case td_api::pushMessageContentContact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentContact &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentContact &>(object));
    case td_api::pushMessageContentContactRegistered::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentContactRegistered &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentContactRegistered &>(object));
    case td_api::pushMessageContentDocument::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentDocument &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentDocument &>(object));
    case td_api::pushMessageContentGame::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentGame &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentGame &>(object));
    case td_api::pushMessageContentGameScore::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentGameScore &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentGameScore &>(object));
    case td_api::pushMessageContentGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentGift &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentGift &>(object));
    case td_api::pushMessageContentGiveaway::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentGiveaway &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentGiveaway &>(object));
    case td_api::pushMessageContentHidden::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentHidden &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentHidden &>(object));
    case td_api::pushMessageContentInviteVideoChatParticipants::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentInviteVideoChatParticipants &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentInviteVideoChatParticipants &>(object));
    case td_api::pushMessageContentInvoice::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentInvoice &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentInvoice &>(object));
    case td_api::pushMessageContentLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentLocation &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentLocation &>(object));
    case td_api::pushMessageContentMediaAlbum::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentMediaAlbum &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentMediaAlbum &>(object));
    case td_api::pushMessageContentMessageForwards::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentMessageForwards &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentMessageForwards &>(object));
    case td_api::pushMessageContentPaidMedia::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentPaidMedia &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentPaidMedia &>(object));
    case td_api::pushMessageContentPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentPhoto &>(object));
    case td_api::pushMessageContentPoll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentPoll &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentPoll &>(object));
    case td_api::pushMessageContentPremiumGiftCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentPremiumGiftCode &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentPremiumGiftCode &>(object));
    case td_api::pushMessageContentProximityAlertTriggered::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentProximityAlertTriggered &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentProximityAlertTriggered &>(object));
    case td_api::pushMessageContentRecurringPayment::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentRecurringPayment &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentRecurringPayment &>(object));
    case td_api::pushMessageContentScreenshotTaken::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentScreenshotTaken &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentScreenshotTaken &>(object));
    case td_api::pushMessageContentSticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentSticker &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentSticker &>(object));
    case td_api::pushMessageContentStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentStory &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentStory &>(object));
    case td_api::pushMessageContentSuggestBirthdate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentSuggestBirthdate &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentSuggestBirthdate &>(object));
    case td_api::pushMessageContentSuggestProfilePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentSuggestProfilePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentSuggestProfilePhoto &>(object));
    case td_api::pushMessageContentText::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentText &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentText &>(object));
    case td_api::pushMessageContentUpgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentUpgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentUpgradedGift &>(object));
    case td_api::pushMessageContentVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentVideo &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentVideo &>(object));
    case td_api::pushMessageContentVideoChatEnded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentVideoChatEnded &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentVideoChatEnded &>(object));
    case td_api::pushMessageContentVideoChatStarted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentVideoChatStarted &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentVideoChatStarted &>(object));
    case td_api::pushMessageContentVideoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentVideoNote &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentVideoNote &>(object));
    case td_api::pushMessageContentVoiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushMessageContentVoiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::pushMessageContentVoiceNote &>(object));
    case td_api::pushReceiverId::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::pushReceiverId &)>(td_api::to_json)(jv, static_cast<const td_api::pushReceiverId &>(object));
    case td_api::quickReplyMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::quickReplyMessage &)>(td_api::to_json)(jv, static_cast<const td_api::quickReplyMessage &>(object));
    case td_api::quickReplyMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::quickReplyMessages &)>(td_api::to_json)(jv, static_cast<const td_api::quickReplyMessages &>(object));
    case td_api::quickReplyShortcut::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::quickReplyShortcut &)>(td_api::to_json)(jv, static_cast<const td_api::quickReplyShortcut &>(object));
    case td_api::reactionNotificationSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionNotificationSettings &)>(td_api::to_json)(jv, static_cast<const td_api::reactionNotificationSettings &>(object));
    case td_api::reactionNotificationSourceAll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionNotificationSourceAll &)>(td_api::to_json)(jv, static_cast<const td_api::reactionNotificationSourceAll &>(object));
    case td_api::reactionNotificationSourceContacts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionNotificationSourceContacts &)>(td_api::to_json)(jv, static_cast<const td_api::reactionNotificationSourceContacts &>(object));
    case td_api::reactionNotificationSourceNone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionNotificationSourceNone &)>(td_api::to_json)(jv, static_cast<const td_api::reactionNotificationSourceNone &>(object));
    case td_api::reactionTypeCustomEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionTypeCustomEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::reactionTypeCustomEmoji &>(object));
    case td_api::reactionTypeEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionTypeEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::reactionTypeEmoji &>(object));
    case td_api::reactionTypePaid::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionTypePaid &)>(td_api::to_json)(jv, static_cast<const td_api::reactionTypePaid &>(object));
    case td_api::reactionUnavailabilityReasonAnonymousAdministrator::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionUnavailabilityReasonAnonymousAdministrator &)>(td_api::to_json)(jv, static_cast<const td_api::reactionUnavailabilityReasonAnonymousAdministrator &>(object));
    case td_api::reactionUnavailabilityReasonGuest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reactionUnavailabilityReasonGuest &)>(td_api::to_json)(jv, static_cast<const td_api::reactionUnavailabilityReasonGuest &>(object));
    case td_api::readDatePrivacySettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::readDatePrivacySettings &)>(td_api::to_json)(jv, static_cast<const td_api::readDatePrivacySettings &>(object));
    case td_api::receivedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::receivedGift &)>(td_api::to_json)(jv, static_cast<const td_api::receivedGift &>(object));
    case td_api::receivedGifts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::receivedGifts &)>(td_api::to_json)(jv, static_cast<const td_api::receivedGifts &>(object));
    case td_api::recommendedChatFolder::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::recommendedChatFolder &)>(td_api::to_json)(jv, static_cast<const td_api::recommendedChatFolder &>(object));
    case td_api::recommendedChatFolders::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::recommendedChatFolders &)>(td_api::to_json)(jv, static_cast<const td_api::recommendedChatFolders &>(object));
    case td_api::recoveryEmailAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::recoveryEmailAddress &)>(td_api::to_json)(jv, static_cast<const td_api::recoveryEmailAddress &>(object));
    case td_api::remoteFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::remoteFile &)>(td_api::to_json)(jv, static_cast<const td_api::remoteFile &>(object));
    case td_api::replyMarkupForceReply::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::replyMarkupForceReply &)>(td_api::to_json)(jv, static_cast<const td_api::replyMarkupForceReply &>(object));
    case td_api::replyMarkupInlineKeyboard::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::replyMarkupInlineKeyboard &)>(td_api::to_json)(jv, static_cast<const td_api::replyMarkupInlineKeyboard &>(object));
    case td_api::replyMarkupRemoveKeyboard::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::replyMarkupRemoveKeyboard &)>(td_api::to_json)(jv, static_cast<const td_api::replyMarkupRemoveKeyboard &>(object));
    case td_api::replyMarkupShowKeyboard::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::replyMarkupShowKeyboard &)>(td_api::to_json)(jv, static_cast<const td_api::replyMarkupShowKeyboard &>(object));
    case td_api::reportChatResultMessagesRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportChatResultMessagesRequired &)>(td_api::to_json)(jv, static_cast<const td_api::reportChatResultMessagesRequired &>(object));
    case td_api::reportChatResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportChatResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::reportChatResultOk &>(object));
    case td_api::reportChatResultOptionRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportChatResultOptionRequired &)>(td_api::to_json)(jv, static_cast<const td_api::reportChatResultOptionRequired &>(object));
    case td_api::reportChatResultTextRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportChatResultTextRequired &)>(td_api::to_json)(jv, static_cast<const td_api::reportChatResultTextRequired &>(object));
    case td_api::reportOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportOption &)>(td_api::to_json)(jv, static_cast<const td_api::reportOption &>(object));
    case td_api::reportSponsoredResultAdsHidden::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportSponsoredResultAdsHidden &)>(td_api::to_json)(jv, static_cast<const td_api::reportSponsoredResultAdsHidden &>(object));
    case td_api::reportSponsoredResultFailed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportSponsoredResultFailed &)>(td_api::to_json)(jv, static_cast<const td_api::reportSponsoredResultFailed &>(object));
    case td_api::reportSponsoredResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportSponsoredResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::reportSponsoredResultOk &>(object));
    case td_api::reportSponsoredResultOptionRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportSponsoredResultOptionRequired &)>(td_api::to_json)(jv, static_cast<const td_api::reportSponsoredResultOptionRequired &>(object));
    case td_api::reportSponsoredResultPremiumRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportSponsoredResultPremiumRequired &)>(td_api::to_json)(jv, static_cast<const td_api::reportSponsoredResultPremiumRequired &>(object));
    case td_api::reportStoryResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportStoryResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::reportStoryResultOk &>(object));
    case td_api::reportStoryResultOptionRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportStoryResultOptionRequired &)>(td_api::to_json)(jv, static_cast<const td_api::reportStoryResultOptionRequired &>(object));
    case td_api::reportStoryResultTextRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::reportStoryResultTextRequired &)>(td_api::to_json)(jv, static_cast<const td_api::reportStoryResultTextRequired &>(object));
    case td_api::resetPasswordResultDeclined::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::resetPasswordResultDeclined &)>(td_api::to_json)(jv, static_cast<const td_api::resetPasswordResultDeclined &>(object));
    case td_api::resetPasswordResultOk::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::resetPasswordResultOk &)>(td_api::to_json)(jv, static_cast<const td_api::resetPasswordResultOk &>(object));
    case td_api::resetPasswordResultPending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::resetPasswordResultPending &)>(td_api::to_json)(jv, static_cast<const td_api::resetPasswordResultPending &>(object));
    case td_api::restrictionInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::restrictionInfo &)>(td_api::to_json)(jv, static_cast<const td_api::restrictionInfo &>(object));
    case td_api::revenueWithdrawalStateFailed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::revenueWithdrawalStateFailed &)>(td_api::to_json)(jv, static_cast<const td_api::revenueWithdrawalStateFailed &>(object));
    case td_api::revenueWithdrawalStatePending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::revenueWithdrawalStatePending &)>(td_api::to_json)(jv, static_cast<const td_api::revenueWithdrawalStatePending &>(object));
    case td_api::revenueWithdrawalStateSucceeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::revenueWithdrawalStateSucceeded &)>(td_api::to_json)(jv, static_cast<const td_api::revenueWithdrawalStateSucceeded &>(object));
    case td_api::richTextAnchor::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextAnchor &)>(td_api::to_json)(jv, static_cast<const td_api::richTextAnchor &>(object));
    case td_api::richTextAnchorLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextAnchorLink &)>(td_api::to_json)(jv, static_cast<const td_api::richTextAnchorLink &>(object));
    case td_api::richTextBold::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextBold &)>(td_api::to_json)(jv, static_cast<const td_api::richTextBold &>(object));
    case td_api::richTextEmailAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextEmailAddress &)>(td_api::to_json)(jv, static_cast<const td_api::richTextEmailAddress &>(object));
    case td_api::richTextFixed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextFixed &)>(td_api::to_json)(jv, static_cast<const td_api::richTextFixed &>(object));
    case td_api::richTextIcon::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextIcon &)>(td_api::to_json)(jv, static_cast<const td_api::richTextIcon &>(object));
    case td_api::richTextItalic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextItalic &)>(td_api::to_json)(jv, static_cast<const td_api::richTextItalic &>(object));
    case td_api::richTextMarked::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextMarked &)>(td_api::to_json)(jv, static_cast<const td_api::richTextMarked &>(object));
    case td_api::richTextPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::richTextPhoneNumber &>(object));
    case td_api::richTextPlain::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextPlain &)>(td_api::to_json)(jv, static_cast<const td_api::richTextPlain &>(object));
    case td_api::richTextReference::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextReference &)>(td_api::to_json)(jv, static_cast<const td_api::richTextReference &>(object));
    case td_api::richTextStrikethrough::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextStrikethrough &)>(td_api::to_json)(jv, static_cast<const td_api::richTextStrikethrough &>(object));
    case td_api::richTextSubscript::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextSubscript &)>(td_api::to_json)(jv, static_cast<const td_api::richTextSubscript &>(object));
    case td_api::richTextSuperscript::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextSuperscript &)>(td_api::to_json)(jv, static_cast<const td_api::richTextSuperscript &>(object));
    case td_api::richTextUnderline::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextUnderline &)>(td_api::to_json)(jv, static_cast<const td_api::richTextUnderline &>(object));
    case td_api::richTextUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTextUrl &)>(td_api::to_json)(jv, static_cast<const td_api::richTextUrl &>(object));
    case td_api::richTexts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::richTexts &)>(td_api::to_json)(jv, static_cast<const td_api::richTexts &>(object));
    case td_api::rtmpUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::rtmpUrl &)>(td_api::to_json)(jv, static_cast<const td_api::rtmpUrl &>(object));
    case td_api::savedCredentials::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::savedCredentials &)>(td_api::to_json)(jv, static_cast<const td_api::savedCredentials &>(object));
    case td_api::savedMessagesTag::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::savedMessagesTag &)>(td_api::to_json)(jv, static_cast<const td_api::savedMessagesTag &>(object));
    case td_api::savedMessagesTags::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::savedMessagesTags &)>(td_api::to_json)(jv, static_cast<const td_api::savedMessagesTags &>(object));
    case td_api::savedMessagesTopic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::savedMessagesTopic &)>(td_api::to_json)(jv, static_cast<const td_api::savedMessagesTopic &>(object));
    case td_api::savedMessagesTopicTypeAuthorHidden::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::savedMessagesTopicTypeAuthorHidden &)>(td_api::to_json)(jv, static_cast<const td_api::savedMessagesTopicTypeAuthorHidden &>(object));
    case td_api::savedMessagesTopicTypeMyNotes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::savedMessagesTopicTypeMyNotes &)>(td_api::to_json)(jv, static_cast<const td_api::savedMessagesTopicTypeMyNotes &>(object));
    case td_api::savedMessagesTopicTypeSavedFromChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::savedMessagesTopicTypeSavedFromChat &)>(td_api::to_json)(jv, static_cast<const td_api::savedMessagesTopicTypeSavedFromChat &>(object));
    case td_api::scopeAutosaveSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::scopeAutosaveSettings &)>(td_api::to_json)(jv, static_cast<const td_api::scopeAutosaveSettings &>(object));
    case td_api::scopeNotificationSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::scopeNotificationSettings &)>(td_api::to_json)(jv, static_cast<const td_api::scopeNotificationSettings &>(object));
    case td_api::seconds::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::seconds &)>(td_api::to_json)(jv, static_cast<const td_api::seconds &>(object));
    case td_api::secretChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::secretChat &)>(td_api::to_json)(jv, static_cast<const td_api::secretChat &>(object));
    case td_api::secretChatStateClosed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::secretChatStateClosed &)>(td_api::to_json)(jv, static_cast<const td_api::secretChatStateClosed &>(object));
    case td_api::secretChatStatePending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::secretChatStatePending &)>(td_api::to_json)(jv, static_cast<const td_api::secretChatStatePending &>(object));
    case td_api::secretChatStateReady::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::secretChatStateReady &)>(td_api::to_json)(jv, static_cast<const td_api::secretChatStateReady &>(object));
    case td_api::sentGiftRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sentGiftRegular &)>(td_api::to_json)(jv, static_cast<const td_api::sentGiftRegular &>(object));
    case td_api::sentGiftUpgraded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sentGiftUpgraded &)>(td_api::to_json)(jv, static_cast<const td_api::sentGiftUpgraded &>(object));
    case td_api::sentWebAppMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sentWebAppMessage &)>(td_api::to_json)(jv, static_cast<const td_api::sentWebAppMessage &>(object));
    case td_api::session::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::session &)>(td_api::to_json)(jv, static_cast<const td_api::session &>(object));
    case td_api::sessionTypeAndroid::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeAndroid &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeAndroid &>(object));
    case td_api::sessionTypeApple::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeApple &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeApple &>(object));
    case td_api::sessionTypeBrave::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeBrave &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeBrave &>(object));
    case td_api::sessionTypeChrome::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeChrome &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeChrome &>(object));
    case td_api::sessionTypeEdge::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeEdge &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeEdge &>(object));
    case td_api::sessionTypeFirefox::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeFirefox &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeFirefox &>(object));
    case td_api::sessionTypeIpad::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeIpad &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeIpad &>(object));
    case td_api::sessionTypeIphone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeIphone &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeIphone &>(object));
    case td_api::sessionTypeLinux::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeLinux &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeLinux &>(object));
    case td_api::sessionTypeMac::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeMac &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeMac &>(object));
    case td_api::sessionTypeOpera::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeOpera &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeOpera &>(object));
    case td_api::sessionTypeSafari::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeSafari &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeSafari &>(object));
    case td_api::sessionTypeUbuntu::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeUbuntu &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeUbuntu &>(object));
    case td_api::sessionTypeUnknown::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeUnknown &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeUnknown &>(object));
    case td_api::sessionTypeVivaldi::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeVivaldi &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeVivaldi &>(object));
    case td_api::sessionTypeWindows::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeWindows &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeWindows &>(object));
    case td_api::sessionTypeXbox::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessionTypeXbox &)>(td_api::to_json)(jv, static_cast<const td_api::sessionTypeXbox &>(object));
    case td_api::sessions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sessions &)>(td_api::to_json)(jv, static_cast<const td_api::sessions &>(object));
    case td_api::sharedChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sharedChat &)>(td_api::to_json)(jv, static_cast<const td_api::sharedChat &>(object));
    case td_api::sharedUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sharedUser &)>(td_api::to_json)(jv, static_cast<const td_api::sharedUser &>(object));
    case td_api::shippingOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::shippingOption &)>(td_api::to_json)(jv, static_cast<const td_api::shippingOption &>(object));
    case td_api::speechRecognitionResultError::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::speechRecognitionResultError &)>(td_api::to_json)(jv, static_cast<const td_api::speechRecognitionResultError &>(object));
    case td_api::speechRecognitionResultPending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::speechRecognitionResultPending &)>(td_api::to_json)(jv, static_cast<const td_api::speechRecognitionResultPending &>(object));
    case td_api::speechRecognitionResultText::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::speechRecognitionResultText &)>(td_api::to_json)(jv, static_cast<const td_api::speechRecognitionResultText &>(object));
    case td_api::sponsoredChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sponsoredChat &)>(td_api::to_json)(jv, static_cast<const td_api::sponsoredChat &>(object));
    case td_api::sponsoredChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sponsoredChats &)>(td_api::to_json)(jv, static_cast<const td_api::sponsoredChats &>(object));
    case td_api::sponsoredMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sponsoredMessage &)>(td_api::to_json)(jv, static_cast<const td_api::sponsoredMessage &>(object));
    case td_api::sponsoredMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sponsoredMessages &)>(td_api::to_json)(jv, static_cast<const td_api::sponsoredMessages &>(object));
    case td_api::starAmount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starAmount &)>(td_api::to_json)(jv, static_cast<const td_api::starAmount &>(object));
    case td_api::starCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starCount &)>(td_api::to_json)(jv, static_cast<const td_api::starCount &>(object));
    case td_api::starGiveawayPaymentOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starGiveawayPaymentOption &)>(td_api::to_json)(jv, static_cast<const td_api::starGiveawayPaymentOption &>(object));
    case td_api::starGiveawayPaymentOptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starGiveawayPaymentOptions &)>(td_api::to_json)(jv, static_cast<const td_api::starGiveawayPaymentOptions &>(object));
    case td_api::starGiveawayWinnerOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starGiveawayWinnerOption &)>(td_api::to_json)(jv, static_cast<const td_api::starGiveawayWinnerOption &>(object));
    case td_api::starPaymentOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starPaymentOption &)>(td_api::to_json)(jv, static_cast<const td_api::starPaymentOption &>(object));
    case td_api::starPaymentOptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starPaymentOptions &)>(td_api::to_json)(jv, static_cast<const td_api::starPaymentOptions &>(object));
    case td_api::starRevenueStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starRevenueStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::starRevenueStatistics &>(object));
    case td_api::starRevenueStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starRevenueStatus &)>(td_api::to_json)(jv, static_cast<const td_api::starRevenueStatus &>(object));
    case td_api::starSubscription::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starSubscription &)>(td_api::to_json)(jv, static_cast<const td_api::starSubscription &>(object));
    case td_api::starSubscriptionPricing::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starSubscriptionPricing &)>(td_api::to_json)(jv, static_cast<const td_api::starSubscriptionPricing &>(object));
    case td_api::starSubscriptionTypeBot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starSubscriptionTypeBot &)>(td_api::to_json)(jv, static_cast<const td_api::starSubscriptionTypeBot &>(object));
    case td_api::starSubscriptionTypeChannel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starSubscriptionTypeChannel &)>(td_api::to_json)(jv, static_cast<const td_api::starSubscriptionTypeChannel &>(object));
    case td_api::starSubscriptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starSubscriptions &)>(td_api::to_json)(jv, static_cast<const td_api::starSubscriptions &>(object));
    case td_api::starTransaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransaction &)>(td_api::to_json)(jv, static_cast<const td_api::starTransaction &>(object));
    case td_api::starTransactionTypeAffiliateProgramCommission::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeAffiliateProgramCommission &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeAffiliateProgramCommission &>(object));
    case td_api::starTransactionTypeAppStoreDeposit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeAppStoreDeposit &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeAppStoreDeposit &>(object));
    case td_api::starTransactionTypeBotInvoicePurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBotInvoicePurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBotInvoicePurchase &>(object));
    case td_api::starTransactionTypeBotInvoiceSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBotInvoiceSale &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBotInvoiceSale &>(object));
    case td_api::starTransactionTypeBotPaidMediaPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBotPaidMediaPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBotPaidMediaPurchase &>(object));
    case td_api::starTransactionTypeBotPaidMediaSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBotPaidMediaSale &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBotPaidMediaSale &>(object));
    case td_api::starTransactionTypeBotSubscriptionPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBotSubscriptionPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBotSubscriptionPurchase &>(object));
    case td_api::starTransactionTypeBotSubscriptionSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBotSubscriptionSale &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBotSubscriptionSale &>(object));
    case td_api::starTransactionTypeBusinessBotTransferReceive::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBusinessBotTransferReceive &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBusinessBotTransferReceive &>(object));
    case td_api::starTransactionTypeBusinessBotTransferSend::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeBusinessBotTransferSend &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeBusinessBotTransferSend &>(object));
    case td_api::starTransactionTypeChannelPaidMediaPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeChannelPaidMediaPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeChannelPaidMediaPurchase &>(object));
    case td_api::starTransactionTypeChannelPaidMediaSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeChannelPaidMediaSale &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeChannelPaidMediaSale &>(object));
    case td_api::starTransactionTypeChannelPaidReactionReceive::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeChannelPaidReactionReceive &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeChannelPaidReactionReceive &>(object));
    case td_api::starTransactionTypeChannelPaidReactionSend::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeChannelPaidReactionSend &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeChannelPaidReactionSend &>(object));
    case td_api::starTransactionTypeChannelSubscriptionPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeChannelSubscriptionPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeChannelSubscriptionPurchase &>(object));
    case td_api::starTransactionTypeChannelSubscriptionSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeChannelSubscriptionSale &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeChannelSubscriptionSale &>(object));
    case td_api::starTransactionTypeFragmentDeposit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeFragmentDeposit &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeFragmentDeposit &>(object));
    case td_api::starTransactionTypeFragmentWithdrawal::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeFragmentWithdrawal &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeFragmentWithdrawal &>(object));
    case td_api::starTransactionTypeGiftOriginalDetailsDrop::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGiftOriginalDetailsDrop &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGiftOriginalDetailsDrop &>(object));
    case td_api::starTransactionTypeGiftPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGiftPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGiftPurchase &>(object));
    case td_api::starTransactionTypeGiftSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGiftSale &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGiftSale &>(object));
    case td_api::starTransactionTypeGiftTransfer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGiftTransfer &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGiftTransfer &>(object));
    case td_api::starTransactionTypeGiftUpgrade::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGiftUpgrade &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGiftUpgrade &>(object));
    case td_api::starTransactionTypeGiftUpgradePurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGiftUpgradePurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGiftUpgradePurchase &>(object));
    case td_api::starTransactionTypeGiveawayDeposit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGiveawayDeposit &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGiveawayDeposit &>(object));
    case td_api::starTransactionTypeGooglePlayDeposit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeGooglePlayDeposit &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeGooglePlayDeposit &>(object));
    case td_api::starTransactionTypePaidMessageReceive::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypePaidMessageReceive &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypePaidMessageReceive &>(object));
    case td_api::starTransactionTypePaidMessageSend::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypePaidMessageSend &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypePaidMessageSend &>(object));
    case td_api::starTransactionTypePremiumBotDeposit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypePremiumBotDeposit &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypePremiumBotDeposit &>(object));
    case td_api::starTransactionTypePremiumPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypePremiumPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypePremiumPurchase &>(object));
    case td_api::starTransactionTypePublicPostSearch::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypePublicPostSearch &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypePublicPostSearch &>(object));
    case td_api::starTransactionTypeSuggestedPostPaymentReceive::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeSuggestedPostPaymentReceive &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeSuggestedPostPaymentReceive &>(object));
    case td_api::starTransactionTypeSuggestedPostPaymentSend::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeSuggestedPostPaymentSend &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeSuggestedPostPaymentSend &>(object));
    case td_api::starTransactionTypeTelegramAdsWithdrawal::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeTelegramAdsWithdrawal &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeTelegramAdsWithdrawal &>(object));
    case td_api::starTransactionTypeTelegramApiUsage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeTelegramApiUsage &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeTelegramApiUsage &>(object));
    case td_api::starTransactionTypeUnsupported::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeUnsupported &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeUnsupported &>(object));
    case td_api::starTransactionTypeUpgradedGiftPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeUpgradedGiftPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeUpgradedGiftPurchase &>(object));
    case td_api::starTransactionTypeUpgradedGiftSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeUpgradedGiftSale &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeUpgradedGiftSale &>(object));
    case td_api::starTransactionTypeUserDeposit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactionTypeUserDeposit &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactionTypeUserDeposit &>(object));
    case td_api::starTransactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::starTransactions &)>(td_api::to_json)(jv, static_cast<const td_api::starTransactions &>(object));
    case td_api::statisticalGraphAsync::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::statisticalGraphAsync &)>(td_api::to_json)(jv, static_cast<const td_api::statisticalGraphAsync &>(object));
    case td_api::statisticalGraphData::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::statisticalGraphData &)>(td_api::to_json)(jv, static_cast<const td_api::statisticalGraphData &>(object));
    case td_api::statisticalGraphError::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::statisticalGraphError &)>(td_api::to_json)(jv, static_cast<const td_api::statisticalGraphError &>(object));
    case td_api::statisticalValue::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::statisticalValue &)>(td_api::to_json)(jv, static_cast<const td_api::statisticalValue &>(object));
    case td_api::sticker::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::sticker &)>(td_api::to_json)(jv, static_cast<const td_api::sticker &>(object));
    case td_api::stickerFormatTgs::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerFormatTgs &)>(td_api::to_json)(jv, static_cast<const td_api::stickerFormatTgs &>(object));
    case td_api::stickerFormatWebm::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerFormatWebm &)>(td_api::to_json)(jv, static_cast<const td_api::stickerFormatWebm &>(object));
    case td_api::stickerFormatWebp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerFormatWebp &)>(td_api::to_json)(jv, static_cast<const td_api::stickerFormatWebp &>(object));
    case td_api::stickerFullTypeCustomEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerFullTypeCustomEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::stickerFullTypeCustomEmoji &>(object));
    case td_api::stickerFullTypeMask::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerFullTypeMask &)>(td_api::to_json)(jv, static_cast<const td_api::stickerFullTypeMask &>(object));
    case td_api::stickerFullTypeRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerFullTypeRegular &)>(td_api::to_json)(jv, static_cast<const td_api::stickerFullTypeRegular &>(object));
    case td_api::stickerSet::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerSet &)>(td_api::to_json)(jv, static_cast<const td_api::stickerSet &>(object));
    case td_api::stickerSetInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerSetInfo &)>(td_api::to_json)(jv, static_cast<const td_api::stickerSetInfo &>(object));
    case td_api::stickerSets::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerSets &)>(td_api::to_json)(jv, static_cast<const td_api::stickerSets &>(object));
    case td_api::stickerTypeCustomEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerTypeCustomEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::stickerTypeCustomEmoji &>(object));
    case td_api::stickerTypeMask::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerTypeMask &)>(td_api::to_json)(jv, static_cast<const td_api::stickerTypeMask &>(object));
    case td_api::stickerTypeRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickerTypeRegular &)>(td_api::to_json)(jv, static_cast<const td_api::stickerTypeRegular &>(object));
    case td_api::stickers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stickers &)>(td_api::to_json)(jv, static_cast<const td_api::stickers &>(object));
    case td_api::storageStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storageStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::storageStatistics &>(object));
    case td_api::storageStatisticsByChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storageStatisticsByChat &)>(td_api::to_json)(jv, static_cast<const td_api::storageStatisticsByChat &>(object));
    case td_api::storageStatisticsByFileType::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storageStatisticsByFileType &)>(td_api::to_json)(jv, static_cast<const td_api::storageStatisticsByFileType &>(object));
    case td_api::storageStatisticsFast::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storageStatisticsFast &)>(td_api::to_json)(jv, static_cast<const td_api::storageStatisticsFast &>(object));
    case td_api::stories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::stories &)>(td_api::to_json)(jv, static_cast<const td_api::stories &>(object));
    case td_api::story::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::story &)>(td_api::to_json)(jv, static_cast<const td_api::story &>(object));
    case td_api::storyAlbum::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAlbum &)>(td_api::to_json)(jv, static_cast<const td_api::storyAlbum &>(object));
    case td_api::storyAlbums::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAlbums &)>(td_api::to_json)(jv, static_cast<const td_api::storyAlbums &>(object));
    case td_api::storyArea::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyArea &)>(td_api::to_json)(jv, static_cast<const td_api::storyArea &>(object));
    case td_api::storyAreaPosition::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaPosition &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaPosition &>(object));
    case td_api::storyAreaTypeLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaTypeLink &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaTypeLink &>(object));
    case td_api::storyAreaTypeLocation::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaTypeLocation &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaTypeLocation &>(object));
    case td_api::storyAreaTypeMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaTypeMessage &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaTypeMessage &>(object));
    case td_api::storyAreaTypeSuggestedReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaTypeSuggestedReaction &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaTypeSuggestedReaction &>(object));
    case td_api::storyAreaTypeUpgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaTypeUpgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaTypeUpgradedGift &>(object));
    case td_api::storyAreaTypeVenue::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaTypeVenue &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaTypeVenue &>(object));
    case td_api::storyAreaTypeWeather::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyAreaTypeWeather &)>(td_api::to_json)(jv, static_cast<const td_api::storyAreaTypeWeather &>(object));
    case td_api::storyContentPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyContentPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::storyContentPhoto &>(object));
    case td_api::storyContentUnsupported::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyContentUnsupported &)>(td_api::to_json)(jv, static_cast<const td_api::storyContentUnsupported &>(object));
    case td_api::storyContentVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyContentVideo &)>(td_api::to_json)(jv, static_cast<const td_api::storyContentVideo &>(object));
    case td_api::storyInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyInfo &)>(td_api::to_json)(jv, static_cast<const td_api::storyInfo &>(object));
    case td_api::storyInteraction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyInteraction &)>(td_api::to_json)(jv, static_cast<const td_api::storyInteraction &>(object));
    case td_api::storyInteractionInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyInteractionInfo &)>(td_api::to_json)(jv, static_cast<const td_api::storyInteractionInfo &>(object));
    case td_api::storyInteractionTypeForward::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyInteractionTypeForward &)>(td_api::to_json)(jv, static_cast<const td_api::storyInteractionTypeForward &>(object));
    case td_api::storyInteractionTypeRepost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyInteractionTypeRepost &)>(td_api::to_json)(jv, static_cast<const td_api::storyInteractionTypeRepost &>(object));
    case td_api::storyInteractionTypeView::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyInteractionTypeView &)>(td_api::to_json)(jv, static_cast<const td_api::storyInteractionTypeView &>(object));
    case td_api::storyInteractions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyInteractions &)>(td_api::to_json)(jv, static_cast<const td_api::storyInteractions &>(object));
    case td_api::storyListArchive::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyListArchive &)>(td_api::to_json)(jv, static_cast<const td_api::storyListArchive &>(object));
    case td_api::storyListMain::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyListMain &)>(td_api::to_json)(jv, static_cast<const td_api::storyListMain &>(object));
    case td_api::storyOriginHiddenUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyOriginHiddenUser &)>(td_api::to_json)(jv, static_cast<const td_api::storyOriginHiddenUser &>(object));
    case td_api::storyOriginPublicStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyOriginPublicStory &)>(td_api::to_json)(jv, static_cast<const td_api::storyOriginPublicStory &>(object));
    case td_api::storyPrivacySettingsCloseFriends::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyPrivacySettingsCloseFriends &)>(td_api::to_json)(jv, static_cast<const td_api::storyPrivacySettingsCloseFriends &>(object));
    case td_api::storyPrivacySettingsContacts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyPrivacySettingsContacts &)>(td_api::to_json)(jv, static_cast<const td_api::storyPrivacySettingsContacts &>(object));
    case td_api::storyPrivacySettingsEveryone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyPrivacySettingsEveryone &)>(td_api::to_json)(jv, static_cast<const td_api::storyPrivacySettingsEveryone &>(object));
    case td_api::storyPrivacySettingsSelectedUsers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyPrivacySettingsSelectedUsers &)>(td_api::to_json)(jv, static_cast<const td_api::storyPrivacySettingsSelectedUsers &>(object));
    case td_api::storyRepostInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyRepostInfo &)>(td_api::to_json)(jv, static_cast<const td_api::storyRepostInfo &>(object));
    case td_api::storyStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::storyStatistics &>(object));
    case td_api::storyVideo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::storyVideo &)>(td_api::to_json)(jv, static_cast<const td_api::storyVideo &>(object));
    case td_api::suggestedActionCheckPassword::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionCheckPassword &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionCheckPassword &>(object));
    case td_api::suggestedActionCheckPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionCheckPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionCheckPhoneNumber &>(object));
    case td_api::suggestedActionConvertToBroadcastGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionConvertToBroadcastGroup &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionConvertToBroadcastGroup &>(object));
    case td_api::suggestedActionCustom::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionCustom &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionCustom &>(object));
    case td_api::suggestedActionEnableArchiveAndMuteNewChats::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionEnableArchiveAndMuteNewChats &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionEnableArchiveAndMuteNewChats &>(object));
    case td_api::suggestedActionExtendPremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionExtendPremium &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionExtendPremium &>(object));
    case td_api::suggestedActionExtendStarSubscriptions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionExtendStarSubscriptions &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionExtendStarSubscriptions &>(object));
    case td_api::suggestedActionGiftPremiumForChristmas::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionGiftPremiumForChristmas &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionGiftPremiumForChristmas &>(object));
    case td_api::suggestedActionRestorePremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionRestorePremium &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionRestorePremium &>(object));
    case td_api::suggestedActionSetBirthdate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionSetBirthdate &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionSetBirthdate &>(object));
    case td_api::suggestedActionSetPassword::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionSetPassword &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionSetPassword &>(object));
    case td_api::suggestedActionSetProfilePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionSetProfilePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionSetProfilePhoto &>(object));
    case td_api::suggestedActionSubscribeToAnnualPremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionSubscribeToAnnualPremium &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionSubscribeToAnnualPremium &>(object));
    case td_api::suggestedActionUpgradePremium::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionUpgradePremium &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionUpgradePremium &>(object));
    case td_api::suggestedActionViewChecksHint::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedActionViewChecksHint &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedActionViewChecksHint &>(object));
    case td_api::suggestedPostInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostInfo &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostInfo &>(object));
    case td_api::suggestedPostPriceStar::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostPriceStar &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostPriceStar &>(object));
    case td_api::suggestedPostPriceTon::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostPriceTon &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostPriceTon &>(object));
    case td_api::suggestedPostRefundReasonPaymentRefunded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostRefundReasonPaymentRefunded &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostRefundReasonPaymentRefunded &>(object));
    case td_api::suggestedPostRefundReasonPostDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostRefundReasonPostDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostRefundReasonPostDeleted &>(object));
    case td_api::suggestedPostStateApproved::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostStateApproved &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostStateApproved &>(object));
    case td_api::suggestedPostStateDeclined::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostStateDeclined &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostStateDeclined &>(object));
    case td_api::suggestedPostStatePending::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::suggestedPostStatePending &)>(td_api::to_json)(jv, static_cast<const td_api::suggestedPostStatePending &>(object));
    case td_api::supergroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::supergroup &)>(td_api::to_json)(jv, static_cast<const td_api::supergroup &>(object));
    case td_api::supergroupFullInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::supergroupFullInfo &)>(td_api::to_json)(jv, static_cast<const td_api::supergroupFullInfo &>(object));
    case td_api::tMeUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tMeUrl &)>(td_api::to_json)(jv, static_cast<const td_api::tMeUrl &>(object));
    case td_api::tMeUrlTypeChatInvite::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tMeUrlTypeChatInvite &)>(td_api::to_json)(jv, static_cast<const td_api::tMeUrlTypeChatInvite &>(object));
    case td_api::tMeUrlTypeStickerSet::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tMeUrlTypeStickerSet &)>(td_api::to_json)(jv, static_cast<const td_api::tMeUrlTypeStickerSet &>(object));
    case td_api::tMeUrlTypeSupergroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tMeUrlTypeSupergroup &)>(td_api::to_json)(jv, static_cast<const td_api::tMeUrlTypeSupergroup &>(object));
    case td_api::tMeUrlTypeUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tMeUrlTypeUser &)>(td_api::to_json)(jv, static_cast<const td_api::tMeUrlTypeUser &>(object));
    case td_api::tMeUrls::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tMeUrls &)>(td_api::to_json)(jv, static_cast<const td_api::tMeUrls &>(object));
    case td_api::targetChatChosen::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::targetChatChosen &)>(td_api::to_json)(jv, static_cast<const td_api::targetChatChosen &>(object));
    case td_api::targetChatCurrent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::targetChatCurrent &)>(td_api::to_json)(jv, static_cast<const td_api::targetChatCurrent &>(object));
    case td_api::targetChatInternalLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::targetChatInternalLink &)>(td_api::to_json)(jv, static_cast<const td_api::targetChatInternalLink &>(object));
    case td_api::targetChatTypes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::targetChatTypes &)>(td_api::to_json)(jv, static_cast<const td_api::targetChatTypes &>(object));
    case td_api::temporaryPasswordState::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::temporaryPasswordState &)>(td_api::to_json)(jv, static_cast<const td_api::temporaryPasswordState &>(object));
    case td_api::termsOfService::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::termsOfService &)>(td_api::to_json)(jv, static_cast<const td_api::termsOfService &>(object));
    case td_api::testBytes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::testBytes &)>(td_api::to_json)(jv, static_cast<const td_api::testBytes &>(object));
    case td_api::testInt::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::testInt &)>(td_api::to_json)(jv, static_cast<const td_api::testInt &>(object));
    case td_api::testString::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::testString &)>(td_api::to_json)(jv, static_cast<const td_api::testString &>(object));
    case td_api::testVectorInt::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::testVectorInt &)>(td_api::to_json)(jv, static_cast<const td_api::testVectorInt &>(object));
    case td_api::testVectorIntObject::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::testVectorIntObject &)>(td_api::to_json)(jv, static_cast<const td_api::testVectorIntObject &>(object));
    case td_api::testVectorString::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::testVectorString &)>(td_api::to_json)(jv, static_cast<const td_api::testVectorString &>(object));
    case td_api::testVectorStringObject::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::testVectorStringObject &)>(td_api::to_json)(jv, static_cast<const td_api::testVectorStringObject &>(object));
    case td_api::text::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::text &)>(td_api::to_json)(jv, static_cast<const td_api::text &>(object));
    case td_api::textEntities::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntities &)>(td_api::to_json)(jv, static_cast<const td_api::textEntities &>(object));
    case td_api::textEntity::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntity &)>(td_api::to_json)(jv, static_cast<const td_api::textEntity &>(object));
    case td_api::textEntityTypeBankCardNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeBankCardNumber &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeBankCardNumber &>(object));
    case td_api::textEntityTypeBlockQuote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeBlockQuote &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeBlockQuote &>(object));
    case td_api::textEntityTypeBold::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeBold &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeBold &>(object));
    case td_api::textEntityTypeBotCommand::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeBotCommand &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeBotCommand &>(object));
    case td_api::textEntityTypeCashtag::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeCashtag &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeCashtag &>(object));
    case td_api::textEntityTypeCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeCode &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeCode &>(object));
    case td_api::textEntityTypeCustomEmoji::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeCustomEmoji &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeCustomEmoji &>(object));
    case td_api::textEntityTypeEmailAddress::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeEmailAddress &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeEmailAddress &>(object));
    case td_api::textEntityTypeExpandableBlockQuote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeExpandableBlockQuote &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeExpandableBlockQuote &>(object));
    case td_api::textEntityTypeHashtag::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeHashtag &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeHashtag &>(object));
    case td_api::textEntityTypeItalic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeItalic &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeItalic &>(object));
    case td_api::textEntityTypeMediaTimestamp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeMediaTimestamp &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeMediaTimestamp &>(object));
    case td_api::textEntityTypeMention::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeMention &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeMention &>(object));
    case td_api::textEntityTypeMentionName::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeMentionName &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeMentionName &>(object));
    case td_api::textEntityTypePhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypePhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypePhoneNumber &>(object));
    case td_api::textEntityTypePre::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypePre &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypePre &>(object));
    case td_api::textEntityTypePreCode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypePreCode &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypePreCode &>(object));
    case td_api::textEntityTypeSpoiler::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeSpoiler &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeSpoiler &>(object));
    case td_api::textEntityTypeStrikethrough::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeStrikethrough &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeStrikethrough &>(object));
    case td_api::textEntityTypeTextUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeTextUrl &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeTextUrl &>(object));
    case td_api::textEntityTypeUnderline::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeUnderline &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeUnderline &>(object));
    case td_api::textEntityTypeUrl::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textEntityTypeUrl &)>(td_api::to_json)(jv, static_cast<const td_api::textEntityTypeUrl &>(object));
    case td_api::textQuote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::textQuote &)>(td_api::to_json)(jv, static_cast<const td_api::textQuote &>(object));
    case td_api::themeSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::themeSettings &)>(td_api::to_json)(jv, static_cast<const td_api::themeSettings &>(object));
    case td_api::thumbnail::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnail &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnail &>(object));
    case td_api::thumbnailFormatGif::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnailFormatGif &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnailFormatGif &>(object));
    case td_api::thumbnailFormatJpeg::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnailFormatJpeg &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnailFormatJpeg &>(object));
    case td_api::thumbnailFormatMpeg4::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnailFormatMpeg4 &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnailFormatMpeg4 &>(object));
    case td_api::thumbnailFormatPng::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnailFormatPng &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnailFormatPng &>(object));
    case td_api::thumbnailFormatTgs::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnailFormatTgs &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnailFormatTgs &>(object));
    case td_api::thumbnailFormatWebm::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnailFormatWebm &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnailFormatWebm &>(object));
    case td_api::thumbnailFormatWebp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::thumbnailFormatWebp &)>(td_api::to_json)(jv, static_cast<const td_api::thumbnailFormatWebp &>(object));
    case td_api::timeZone::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::timeZone &)>(td_api::to_json)(jv, static_cast<const td_api::timeZone &>(object));
    case td_api::timeZones::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::timeZones &)>(td_api::to_json)(jv, static_cast<const td_api::timeZones &>(object));
    case td_api::tonRevenueStatistics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonRevenueStatistics &)>(td_api::to_json)(jv, static_cast<const td_api::tonRevenueStatistics &>(object));
    case td_api::tonRevenueStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonRevenueStatus &)>(td_api::to_json)(jv, static_cast<const td_api::tonRevenueStatus &>(object));
    case td_api::tonTransaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonTransaction &)>(td_api::to_json)(jv, static_cast<const td_api::tonTransaction &>(object));
    case td_api::tonTransactionTypeFragmentDeposit::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonTransactionTypeFragmentDeposit &)>(td_api::to_json)(jv, static_cast<const td_api::tonTransactionTypeFragmentDeposit &>(object));
    case td_api::tonTransactionTypeSuggestedPostPayment::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonTransactionTypeSuggestedPostPayment &)>(td_api::to_json)(jv, static_cast<const td_api::tonTransactionTypeSuggestedPostPayment &>(object));
    case td_api::tonTransactionTypeUnsupported::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonTransactionTypeUnsupported &)>(td_api::to_json)(jv, static_cast<const td_api::tonTransactionTypeUnsupported &>(object));
    case td_api::tonTransactionTypeUpgradedGiftPurchase::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonTransactionTypeUpgradedGiftPurchase &)>(td_api::to_json)(jv, static_cast<const td_api::tonTransactionTypeUpgradedGiftPurchase &>(object));
    case td_api::tonTransactionTypeUpgradedGiftSale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonTransactionTypeUpgradedGiftSale &)>(td_api::to_json)(jv, static_cast<const td_api::tonTransactionTypeUpgradedGiftSale &>(object));
    case td_api::tonTransactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::tonTransactions &)>(td_api::to_json)(jv, static_cast<const td_api::tonTransactions &>(object));
    case td_api::trendingStickerSets::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::trendingStickerSets &)>(td_api::to_json)(jv, static_cast<const td_api::trendingStickerSets &>(object));
    case td_api::unconfirmedSession::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::unconfirmedSession &)>(td_api::to_json)(jv, static_cast<const td_api::unconfirmedSession &>(object));
    case td_api::unreadReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::unreadReaction &)>(td_api::to_json)(jv, static_cast<const td_api::unreadReaction &>(object));
    case td_api::updateAccentColors::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAccentColors &)>(td_api::to_json)(jv, static_cast<const td_api::updateAccentColors &>(object));
    case td_api::updateActiveEmojiReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateActiveEmojiReactions &)>(td_api::to_json)(jv, static_cast<const td_api::updateActiveEmojiReactions &>(object));
    case td_api::updateActiveLiveLocationMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateActiveLiveLocationMessages &)>(td_api::to_json)(jv, static_cast<const td_api::updateActiveLiveLocationMessages &>(object));
    case td_api::updateActiveNotifications::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateActiveNotifications &)>(td_api::to_json)(jv, static_cast<const td_api::updateActiveNotifications &>(object));
    case td_api::updateAgeVerificationParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAgeVerificationParameters &)>(td_api::to_json)(jv, static_cast<const td_api::updateAgeVerificationParameters &>(object));
    case td_api::updateAnimatedEmojiMessageClicked::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAnimatedEmojiMessageClicked &)>(td_api::to_json)(jv, static_cast<const td_api::updateAnimatedEmojiMessageClicked &>(object));
    case td_api::updateAnimationSearchParameters::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAnimationSearchParameters &)>(td_api::to_json)(jv, static_cast<const td_api::updateAnimationSearchParameters &>(object));
    case td_api::updateApplicationRecaptchaVerificationRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateApplicationRecaptchaVerificationRequired &)>(td_api::to_json)(jv, static_cast<const td_api::updateApplicationRecaptchaVerificationRequired &>(object));
    case td_api::updateApplicationVerificationRequired::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateApplicationVerificationRequired &)>(td_api::to_json)(jv, static_cast<const td_api::updateApplicationVerificationRequired &>(object));
    case td_api::updateAttachmentMenuBots::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAttachmentMenuBots &)>(td_api::to_json)(jv, static_cast<const td_api::updateAttachmentMenuBots &>(object));
    case td_api::updateAuthorizationState::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAuthorizationState &)>(td_api::to_json)(jv, static_cast<const td_api::updateAuthorizationState &>(object));
    case td_api::updateAutosaveSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAutosaveSettings &)>(td_api::to_json)(jv, static_cast<const td_api::updateAutosaveSettings &>(object));
    case td_api::updateAvailableMessageEffects::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateAvailableMessageEffects &)>(td_api::to_json)(jv, static_cast<const td_api::updateAvailableMessageEffects &>(object));
    case td_api::updateBasicGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateBasicGroup &)>(td_api::to_json)(jv, static_cast<const td_api::updateBasicGroup &>(object));
    case td_api::updateBasicGroupFullInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateBasicGroupFullInfo &)>(td_api::to_json)(jv, static_cast<const td_api::updateBasicGroupFullInfo &>(object));
    case td_api::updateBusinessConnection::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateBusinessConnection &)>(td_api::to_json)(jv, static_cast<const td_api::updateBusinessConnection &>(object));
    case td_api::updateBusinessMessageEdited::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateBusinessMessageEdited &)>(td_api::to_json)(jv, static_cast<const td_api::updateBusinessMessageEdited &>(object));
    case td_api::updateBusinessMessagesDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateBusinessMessagesDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::updateBusinessMessagesDeleted &>(object));
    case td_api::updateCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateCall &)>(td_api::to_json)(jv, static_cast<const td_api::updateCall &>(object));
    case td_api::updateChatAccentColors::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatAccentColors &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatAccentColors &>(object));
    case td_api::updateChatAction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatAction &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatAction &>(object));
    case td_api::updateChatActionBar::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatActionBar &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatActionBar &>(object));
    case td_api::updateChatActiveStories::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatActiveStories &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatActiveStories &>(object));
    case td_api::updateChatAddedToList::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatAddedToList &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatAddedToList &>(object));
    case td_api::updateChatAvailableReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatAvailableReactions &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatAvailableReactions &>(object));
    case td_api::updateChatBackground::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatBackground &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatBackground &>(object));
    case td_api::updateChatBlockList::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatBlockList &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatBlockList &>(object));
    case td_api::updateChatBoost::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatBoost &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatBoost &>(object));
    case td_api::updateChatBusinessBotManageBar::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatBusinessBotManageBar &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatBusinessBotManageBar &>(object));
    case td_api::updateChatDefaultDisableNotification::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatDefaultDisableNotification &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatDefaultDisableNotification &>(object));
    case td_api::updateChatDraftMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatDraftMessage &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatDraftMessage &>(object));
    case td_api::updateChatEmojiStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatEmojiStatus &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatEmojiStatus &>(object));
    case td_api::updateChatFolders::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatFolders &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatFolders &>(object));
    case td_api::updateChatHasProtectedContent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatHasProtectedContent &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatHasProtectedContent &>(object));
    case td_api::updateChatHasScheduledMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatHasScheduledMessages &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatHasScheduledMessages &>(object));
    case td_api::updateChatIsMarkedAsUnread::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatIsMarkedAsUnread &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatIsMarkedAsUnread &>(object));
    case td_api::updateChatIsTranslatable::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatIsTranslatable &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatIsTranslatable &>(object));
    case td_api::updateChatLastMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatLastMessage &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatLastMessage &>(object));
    case td_api::updateChatMember::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatMember &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatMember &>(object));
    case td_api::updateChatMessageAutoDeleteTime::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatMessageAutoDeleteTime &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatMessageAutoDeleteTime &>(object));
    case td_api::updateChatMessageSender::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatMessageSender &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatMessageSender &>(object));
    case td_api::updateChatNotificationSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatNotificationSettings &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatNotificationSettings &>(object));
    case td_api::updateChatOnlineMemberCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatOnlineMemberCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatOnlineMemberCount &>(object));
    case td_api::updateChatPendingJoinRequests::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatPendingJoinRequests &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatPendingJoinRequests &>(object));
    case td_api::updateChatPermissions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatPermissions &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatPermissions &>(object));
    case td_api::updateChatPhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatPhoto &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatPhoto &>(object));
    case td_api::updateChatPosition::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatPosition &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatPosition &>(object));
    case td_api::updateChatReadInbox::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatReadInbox &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatReadInbox &>(object));
    case td_api::updateChatReadOutbox::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatReadOutbox &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatReadOutbox &>(object));
    case td_api::updateChatRemovedFromList::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatRemovedFromList &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatRemovedFromList &>(object));
    case td_api::updateChatReplyMarkup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatReplyMarkup &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatReplyMarkup &>(object));
    case td_api::updateChatRevenueAmount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatRevenueAmount &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatRevenueAmount &>(object));
    case td_api::updateChatTheme::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatTheme &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatTheme &>(object));
    case td_api::updateChatTitle::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatTitle &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatTitle &>(object));
    case td_api::updateChatUnreadMentionCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatUnreadMentionCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatUnreadMentionCount &>(object));
    case td_api::updateChatUnreadReactionCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatUnreadReactionCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatUnreadReactionCount &>(object));
    case td_api::updateChatVideoChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatVideoChat &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatVideoChat &>(object));
    case td_api::updateChatViewAsTopics::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateChatViewAsTopics &)>(td_api::to_json)(jv, static_cast<const td_api::updateChatViewAsTopics &>(object));
    case td_api::updateConnectionState::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateConnectionState &)>(td_api::to_json)(jv, static_cast<const td_api::updateConnectionState &>(object));
    case td_api::updateContactCloseBirthdays::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateContactCloseBirthdays &)>(td_api::to_json)(jv, static_cast<const td_api::updateContactCloseBirthdays &>(object));
    case td_api::updateDefaultBackground::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateDefaultBackground &)>(td_api::to_json)(jv, static_cast<const td_api::updateDefaultBackground &>(object));
    case td_api::updateDefaultPaidReactionType::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateDefaultPaidReactionType &)>(td_api::to_json)(jv, static_cast<const td_api::updateDefaultPaidReactionType &>(object));
    case td_api::updateDefaultReactionType::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateDefaultReactionType &)>(td_api::to_json)(jv, static_cast<const td_api::updateDefaultReactionType &>(object));
    case td_api::updateDeleteMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateDeleteMessages &)>(td_api::to_json)(jv, static_cast<const td_api::updateDeleteMessages &>(object));
    case td_api::updateDiceEmojis::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateDiceEmojis &)>(td_api::to_json)(jv, static_cast<const td_api::updateDiceEmojis &>(object));
    case td_api::updateDirectMessagesChatTopic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateDirectMessagesChatTopic &)>(td_api::to_json)(jv, static_cast<const td_api::updateDirectMessagesChatTopic &>(object));
    case td_api::updateEmojiChatThemes::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateEmojiChatThemes &)>(td_api::to_json)(jv, static_cast<const td_api::updateEmojiChatThemes &>(object));
    case td_api::updateFavoriteStickers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFavoriteStickers &)>(td_api::to_json)(jv, static_cast<const td_api::updateFavoriteStickers &>(object));
    case td_api::updateFile::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFile &)>(td_api::to_json)(jv, static_cast<const td_api::updateFile &>(object));
    case td_api::updateFileAddedToDownloads::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFileAddedToDownloads &)>(td_api::to_json)(jv, static_cast<const td_api::updateFileAddedToDownloads &>(object));
    case td_api::updateFileDownload::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFileDownload &)>(td_api::to_json)(jv, static_cast<const td_api::updateFileDownload &>(object));
    case td_api::updateFileDownloads::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFileDownloads &)>(td_api::to_json)(jv, static_cast<const td_api::updateFileDownloads &>(object));
    case td_api::updateFileGenerationStart::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFileGenerationStart &)>(td_api::to_json)(jv, static_cast<const td_api::updateFileGenerationStart &>(object));
    case td_api::updateFileGenerationStop::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFileGenerationStop &)>(td_api::to_json)(jv, static_cast<const td_api::updateFileGenerationStop &>(object));
    case td_api::updateFileRemovedFromDownloads::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFileRemovedFromDownloads &)>(td_api::to_json)(jv, static_cast<const td_api::updateFileRemovedFromDownloads &>(object));
    case td_api::updateForumTopic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateForumTopic &)>(td_api::to_json)(jv, static_cast<const td_api::updateForumTopic &>(object));
    case td_api::updateForumTopicInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateForumTopicInfo &)>(td_api::to_json)(jv, static_cast<const td_api::updateForumTopicInfo &>(object));
    case td_api::updateFreezeState::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateFreezeState &)>(td_api::to_json)(jv, static_cast<const td_api::updateFreezeState &>(object));
    case td_api::updateGroupCall::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateGroupCall &)>(td_api::to_json)(jv, static_cast<const td_api::updateGroupCall &>(object));
    case td_api::updateGroupCallNewMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateGroupCallNewMessage &)>(td_api::to_json)(jv, static_cast<const td_api::updateGroupCallNewMessage &>(object));
    case td_api::updateGroupCallParticipant::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateGroupCallParticipant &)>(td_api::to_json)(jv, static_cast<const td_api::updateGroupCallParticipant &>(object));
    case td_api::updateGroupCallParticipants::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateGroupCallParticipants &)>(td_api::to_json)(jv, static_cast<const td_api::updateGroupCallParticipants &>(object));
    case td_api::updateGroupCallVerificationState::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateGroupCallVerificationState &)>(td_api::to_json)(jv, static_cast<const td_api::updateGroupCallVerificationState &>(object));
    case td_api::updateHavePendingNotifications::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateHavePendingNotifications &)>(td_api::to_json)(jv, static_cast<const td_api::updateHavePendingNotifications &>(object));
    case td_api::updateInstalledStickerSets::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateInstalledStickerSets &)>(td_api::to_json)(jv, static_cast<const td_api::updateInstalledStickerSets &>(object));
    case td_api::updateLanguagePackStrings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateLanguagePackStrings &)>(td_api::to_json)(jv, static_cast<const td_api::updateLanguagePackStrings &>(object));
    case td_api::updateMessageContent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageContent &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageContent &>(object));
    case td_api::updateMessageContentOpened::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageContentOpened &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageContentOpened &>(object));
    case td_api::updateMessageEdited::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageEdited &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageEdited &>(object));
    case td_api::updateMessageFactCheck::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageFactCheck &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageFactCheck &>(object));
    case td_api::updateMessageInteractionInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageInteractionInfo &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageInteractionInfo &>(object));
    case td_api::updateMessageIsPinned::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageIsPinned &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageIsPinned &>(object));
    case td_api::updateMessageLiveLocationViewed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageLiveLocationViewed &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageLiveLocationViewed &>(object));
    case td_api::updateMessageMentionRead::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageMentionRead &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageMentionRead &>(object));
    case td_api::updateMessageReaction::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageReaction &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageReaction &>(object));
    case td_api::updateMessageReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageReactions &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageReactions &>(object));
    case td_api::updateMessageSendAcknowledged::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageSendAcknowledged &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageSendAcknowledged &>(object));
    case td_api::updateMessageSendFailed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageSendFailed &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageSendFailed &>(object));
    case td_api::updateMessageSendSucceeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageSendSucceeded &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageSendSucceeded &>(object));
    case td_api::updateMessageSuggestedPostInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageSuggestedPostInfo &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageSuggestedPostInfo &>(object));
    case td_api::updateMessageUnreadReactions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateMessageUnreadReactions &)>(td_api::to_json)(jv, static_cast<const td_api::updateMessageUnreadReactions &>(object));
    case td_api::updateNewBusinessCallbackQuery::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewBusinessCallbackQuery &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewBusinessCallbackQuery &>(object));
    case td_api::updateNewBusinessMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewBusinessMessage &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewBusinessMessage &>(object));
    case td_api::updateNewCallSignalingData::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewCallSignalingData &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewCallSignalingData &>(object));
    case td_api::updateNewCallbackQuery::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewCallbackQuery &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewCallbackQuery &>(object));
    case td_api::updateNewChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewChat &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewChat &>(object));
    case td_api::updateNewChatJoinRequest::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewChatJoinRequest &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewChatJoinRequest &>(object));
    case td_api::updateNewChosenInlineResult::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewChosenInlineResult &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewChosenInlineResult &>(object));
    case td_api::updateNewCustomEvent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewCustomEvent &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewCustomEvent &>(object));
    case td_api::updateNewCustomQuery::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewCustomQuery &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewCustomQuery &>(object));
    case td_api::updateNewInlineCallbackQuery::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewInlineCallbackQuery &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewInlineCallbackQuery &>(object));
    case td_api::updateNewInlineQuery::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewInlineQuery &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewInlineQuery &>(object));
    case td_api::updateNewMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewMessage &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewMessage &>(object));
    case td_api::updateNewPreCheckoutQuery::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewPreCheckoutQuery &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewPreCheckoutQuery &>(object));
    case td_api::updateNewShippingQuery::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNewShippingQuery &)>(td_api::to_json)(jv, static_cast<const td_api::updateNewShippingQuery &>(object));
    case td_api::updateNotification::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNotification &)>(td_api::to_json)(jv, static_cast<const td_api::updateNotification &>(object));
    case td_api::updateNotificationGroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateNotificationGroup &)>(td_api::to_json)(jv, static_cast<const td_api::updateNotificationGroup &>(object));
    case td_api::updateOption::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateOption &)>(td_api::to_json)(jv, static_cast<const td_api::updateOption &>(object));
    case td_api::updateOwnedStarCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateOwnedStarCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateOwnedStarCount &>(object));
    case td_api::updateOwnedTonCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateOwnedTonCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateOwnedTonCount &>(object));
    case td_api::updatePaidMediaPurchased::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updatePaidMediaPurchased &)>(td_api::to_json)(jv, static_cast<const td_api::updatePaidMediaPurchased &>(object));
    case td_api::updatePendingTextMessage::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updatePendingTextMessage &)>(td_api::to_json)(jv, static_cast<const td_api::updatePendingTextMessage &>(object));
    case td_api::updatePoll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updatePoll &)>(td_api::to_json)(jv, static_cast<const td_api::updatePoll &>(object));
    case td_api::updatePollAnswer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updatePollAnswer &)>(td_api::to_json)(jv, static_cast<const td_api::updatePollAnswer &>(object));
    case td_api::updateProfileAccentColors::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateProfileAccentColors &)>(td_api::to_json)(jv, static_cast<const td_api::updateProfileAccentColors &>(object));
    case td_api::updateQuickReplyShortcut::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateQuickReplyShortcut &)>(td_api::to_json)(jv, static_cast<const td_api::updateQuickReplyShortcut &>(object));
    case td_api::updateQuickReplyShortcutDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateQuickReplyShortcutDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::updateQuickReplyShortcutDeleted &>(object));
    case td_api::updateQuickReplyShortcutMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateQuickReplyShortcutMessages &)>(td_api::to_json)(jv, static_cast<const td_api::updateQuickReplyShortcutMessages &>(object));
    case td_api::updateQuickReplyShortcuts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateQuickReplyShortcuts &)>(td_api::to_json)(jv, static_cast<const td_api::updateQuickReplyShortcuts &>(object));
    case td_api::updateReactionNotificationSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateReactionNotificationSettings &)>(td_api::to_json)(jv, static_cast<const td_api::updateReactionNotificationSettings &>(object));
    case td_api::updateRecentStickers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateRecentStickers &)>(td_api::to_json)(jv, static_cast<const td_api::updateRecentStickers &>(object));
    case td_api::updateSavedAnimations::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSavedAnimations &)>(td_api::to_json)(jv, static_cast<const td_api::updateSavedAnimations &>(object));
    case td_api::updateSavedMessagesTags::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSavedMessagesTags &)>(td_api::to_json)(jv, static_cast<const td_api::updateSavedMessagesTags &>(object));
    case td_api::updateSavedMessagesTopic::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSavedMessagesTopic &)>(td_api::to_json)(jv, static_cast<const td_api::updateSavedMessagesTopic &>(object));
    case td_api::updateSavedMessagesTopicCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSavedMessagesTopicCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateSavedMessagesTopicCount &>(object));
    case td_api::updateSavedNotificationSounds::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSavedNotificationSounds &)>(td_api::to_json)(jv, static_cast<const td_api::updateSavedNotificationSounds &>(object));
    case td_api::updateScopeNotificationSettings::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateScopeNotificationSettings &)>(td_api::to_json)(jv, static_cast<const td_api::updateScopeNotificationSettings &>(object));
    case td_api::updateSecretChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSecretChat &)>(td_api::to_json)(jv, static_cast<const td_api::updateSecretChat &>(object));
    case td_api::updateServiceNotification::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateServiceNotification &)>(td_api::to_json)(jv, static_cast<const td_api::updateServiceNotification &>(object));
    case td_api::updateSpeechRecognitionTrial::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSpeechRecognitionTrial &)>(td_api::to_json)(jv, static_cast<const td_api::updateSpeechRecognitionTrial &>(object));
    case td_api::updateSpeedLimitNotification::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSpeedLimitNotification &)>(td_api::to_json)(jv, static_cast<const td_api::updateSpeedLimitNotification &>(object));
    case td_api::updateStarRevenueStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStarRevenueStatus &)>(td_api::to_json)(jv, static_cast<const td_api::updateStarRevenueStatus &>(object));
    case td_api::updateStickerSet::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStickerSet &)>(td_api::to_json)(jv, static_cast<const td_api::updateStickerSet &>(object));
    case td_api::updateStory::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStory &)>(td_api::to_json)(jv, static_cast<const td_api::updateStory &>(object));
    case td_api::updateStoryDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStoryDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::updateStoryDeleted &>(object));
    case td_api::updateStoryListChatCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStoryListChatCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateStoryListChatCount &>(object));
    case td_api::updateStoryPostFailed::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStoryPostFailed &)>(td_api::to_json)(jv, static_cast<const td_api::updateStoryPostFailed &>(object));
    case td_api::updateStoryPostSucceeded::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStoryPostSucceeded &)>(td_api::to_json)(jv, static_cast<const td_api::updateStoryPostSucceeded &>(object));
    case td_api::updateStoryStealthMode::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateStoryStealthMode &)>(td_api::to_json)(jv, static_cast<const td_api::updateStoryStealthMode &>(object));
    case td_api::updateSuggestedActions::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSuggestedActions &)>(td_api::to_json)(jv, static_cast<const td_api::updateSuggestedActions &>(object));
    case td_api::updateSupergroup::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSupergroup &)>(td_api::to_json)(jv, static_cast<const td_api::updateSupergroup &>(object));
    case td_api::updateSupergroupFullInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateSupergroupFullInfo &)>(td_api::to_json)(jv, static_cast<const td_api::updateSupergroupFullInfo &>(object));
    case td_api::updateTermsOfService::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateTermsOfService &)>(td_api::to_json)(jv, static_cast<const td_api::updateTermsOfService &>(object));
    case td_api::updateTonRevenueStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateTonRevenueStatus &)>(td_api::to_json)(jv, static_cast<const td_api::updateTonRevenueStatus &>(object));
    case td_api::updateTopicMessageCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateTopicMessageCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateTopicMessageCount &>(object));
    case td_api::updateTrendingStickerSets::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateTrendingStickerSets &)>(td_api::to_json)(jv, static_cast<const td_api::updateTrendingStickerSets &>(object));
    case td_api::updateUnconfirmedSession::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateUnconfirmedSession &)>(td_api::to_json)(jv, static_cast<const td_api::updateUnconfirmedSession &>(object));
    case td_api::updateUnreadChatCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateUnreadChatCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateUnreadChatCount &>(object));
    case td_api::updateUnreadMessageCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateUnreadMessageCount &)>(td_api::to_json)(jv, static_cast<const td_api::updateUnreadMessageCount &>(object));
    case td_api::updateUser::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateUser &)>(td_api::to_json)(jv, static_cast<const td_api::updateUser &>(object));
    case td_api::updateUserFullInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateUserFullInfo &)>(td_api::to_json)(jv, static_cast<const td_api::updateUserFullInfo &>(object));
    case td_api::updateUserPrivacySettingRules::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateUserPrivacySettingRules &)>(td_api::to_json)(jv, static_cast<const td_api::updateUserPrivacySettingRules &>(object));
    case td_api::updateUserStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateUserStatus &)>(td_api::to_json)(jv, static_cast<const td_api::updateUserStatus &>(object));
    case td_api::updateVideoPublished::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateVideoPublished &)>(td_api::to_json)(jv, static_cast<const td_api::updateVideoPublished &>(object));
    case td_api::updateWebAppMessageSent::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updateWebAppMessageSent &)>(td_api::to_json)(jv, static_cast<const td_api::updateWebAppMessageSent &>(object));
    case td_api::updates::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::updates &)>(td_api::to_json)(jv, static_cast<const td_api::updates &>(object));
    case td_api::upgradeGiftResult::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradeGiftResult &)>(td_api::to_json)(jv, static_cast<const td_api::upgradeGiftResult &>(object));
    case td_api::upgradedGift::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGift &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGift &>(object));
    case td_api::upgradedGiftBackdrop::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftBackdrop &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftBackdrop &>(object));
    case td_api::upgradedGiftBackdropColors::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftBackdropColors &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftBackdropColors &>(object));
    case td_api::upgradedGiftBackdropCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftBackdropCount &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftBackdropCount &>(object));
    case td_api::upgradedGiftColors::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftColors &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftColors &>(object));
    case td_api::upgradedGiftModel::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftModel &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftModel &>(object));
    case td_api::upgradedGiftModelCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftModelCount &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftModelCount &>(object));
    case td_api::upgradedGiftOriginBlockchain::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftOriginBlockchain &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftOriginBlockchain &>(object));
    case td_api::upgradedGiftOriginPrepaidUpgrade::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftOriginPrepaidUpgrade &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftOriginPrepaidUpgrade &>(object));
    case td_api::upgradedGiftOriginResale::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftOriginResale &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftOriginResale &>(object));
    case td_api::upgradedGiftOriginTransfer::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftOriginTransfer &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftOriginTransfer &>(object));
    case td_api::upgradedGiftOriginUpgrade::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftOriginUpgrade &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftOriginUpgrade &>(object));
    case td_api::upgradedGiftOriginalDetails::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftOriginalDetails &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftOriginalDetails &>(object));
    case td_api::upgradedGiftSymbol::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftSymbol &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftSymbol &>(object));
    case td_api::upgradedGiftSymbolCount::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftSymbolCount &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftSymbolCount &>(object));
    case td_api::upgradedGiftValueInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::upgradedGiftValueInfo &)>(td_api::to_json)(jv, static_cast<const td_api::upgradedGiftValueInfo &>(object));
    case td_api::user::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::user &)>(td_api::to_json)(jv, static_cast<const td_api::user &>(object));
    case td_api::userFullInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userFullInfo &)>(td_api::to_json)(jv, static_cast<const td_api::userFullInfo &>(object));
    case td_api::userLink::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userLink &)>(td_api::to_json)(jv, static_cast<const td_api::userLink &>(object));
    case td_api::userPrivacySettingAllowCalls::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingAllowCalls &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingAllowCalls &>(object));
    case td_api::userPrivacySettingAllowChatInvites::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingAllowChatInvites &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingAllowChatInvites &>(object));
    case td_api::userPrivacySettingAllowFindingByPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingAllowFindingByPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingAllowFindingByPhoneNumber &>(object));
    case td_api::userPrivacySettingAllowPeerToPeerCalls::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingAllowPeerToPeerCalls &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingAllowPeerToPeerCalls &>(object));
    case td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &>(object));
    case td_api::userPrivacySettingAllowUnpaidMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingAllowUnpaidMessages &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingAllowUnpaidMessages &>(object));
    case td_api::userPrivacySettingAutosaveGifts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingAutosaveGifts &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingAutosaveGifts &>(object));
    case td_api::userPrivacySettingRuleAllowAll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleAllowAll &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleAllowAll &>(object));
    case td_api::userPrivacySettingRuleAllowBots::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleAllowBots &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleAllowBots &>(object));
    case td_api::userPrivacySettingRuleAllowChatMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleAllowChatMembers &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleAllowChatMembers &>(object));
    case td_api::userPrivacySettingRuleAllowContacts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleAllowContacts &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleAllowContacts &>(object));
    case td_api::userPrivacySettingRuleAllowPremiumUsers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleAllowPremiumUsers &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleAllowPremiumUsers &>(object));
    case td_api::userPrivacySettingRuleAllowUsers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleAllowUsers &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleAllowUsers &>(object));
    case td_api::userPrivacySettingRuleRestrictAll::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleRestrictAll &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleRestrictAll &>(object));
    case td_api::userPrivacySettingRuleRestrictBots::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleRestrictBots &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleRestrictBots &>(object));
    case td_api::userPrivacySettingRuleRestrictChatMembers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleRestrictChatMembers &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleRestrictChatMembers &>(object));
    case td_api::userPrivacySettingRuleRestrictContacts::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleRestrictContacts &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleRestrictContacts &>(object));
    case td_api::userPrivacySettingRuleRestrictUsers::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRuleRestrictUsers &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRuleRestrictUsers &>(object));
    case td_api::userPrivacySettingRules::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingRules &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingRules &>(object));
    case td_api::userPrivacySettingShowBio::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingShowBio &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingShowBio &>(object));
    case td_api::userPrivacySettingShowBirthdate::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingShowBirthdate &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingShowBirthdate &>(object));
    case td_api::userPrivacySettingShowLinkInForwardedMessages::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingShowLinkInForwardedMessages &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingShowLinkInForwardedMessages &>(object));
    case td_api::userPrivacySettingShowPhoneNumber::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingShowPhoneNumber &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingShowPhoneNumber &>(object));
    case td_api::userPrivacySettingShowProfilePhoto::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingShowProfilePhoto &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingShowProfilePhoto &>(object));
    case td_api::userPrivacySettingShowStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userPrivacySettingShowStatus &)>(td_api::to_json)(jv, static_cast<const td_api::userPrivacySettingShowStatus &>(object));
    case td_api::userRating::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userRating &)>(td_api::to_json)(jv, static_cast<const td_api::userRating &>(object));
    case td_api::userStatusEmpty::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userStatusEmpty &)>(td_api::to_json)(jv, static_cast<const td_api::userStatusEmpty &>(object));
    case td_api::userStatusLastMonth::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userStatusLastMonth &)>(td_api::to_json)(jv, static_cast<const td_api::userStatusLastMonth &>(object));
    case td_api::userStatusLastWeek::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userStatusLastWeek &)>(td_api::to_json)(jv, static_cast<const td_api::userStatusLastWeek &>(object));
    case td_api::userStatusOffline::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userStatusOffline &)>(td_api::to_json)(jv, static_cast<const td_api::userStatusOffline &>(object));
    case td_api::userStatusOnline::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userStatusOnline &)>(td_api::to_json)(jv, static_cast<const td_api::userStatusOnline &>(object));
    case td_api::userStatusRecently::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userStatusRecently &)>(td_api::to_json)(jv, static_cast<const td_api::userStatusRecently &>(object));
    case td_api::userSupportInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userSupportInfo &)>(td_api::to_json)(jv, static_cast<const td_api::userSupportInfo &>(object));
    case td_api::userTypeBot::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userTypeBot &)>(td_api::to_json)(jv, static_cast<const td_api::userTypeBot &>(object));
    case td_api::userTypeDeleted::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userTypeDeleted &)>(td_api::to_json)(jv, static_cast<const td_api::userTypeDeleted &>(object));
    case td_api::userTypeRegular::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userTypeRegular &)>(td_api::to_json)(jv, static_cast<const td_api::userTypeRegular &>(object));
    case td_api::userTypeUnknown::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::userTypeUnknown &)>(td_api::to_json)(jv, static_cast<const td_api::userTypeUnknown &>(object));
    case td_api::usernames::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::usernames &)>(td_api::to_json)(jv, static_cast<const td_api::usernames &>(object));
    case td_api::users::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::users &)>(td_api::to_json)(jv, static_cast<const td_api::users &>(object));
    case td_api::validatedOrderInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::validatedOrderInfo &)>(td_api::to_json)(jv, static_cast<const td_api::validatedOrderInfo &>(object));
    case td_api::vectorPathCommandCubicBezierCurve::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::vectorPathCommandCubicBezierCurve &)>(td_api::to_json)(jv, static_cast<const td_api::vectorPathCommandCubicBezierCurve &>(object));
    case td_api::vectorPathCommandLine::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::vectorPathCommandLine &)>(td_api::to_json)(jv, static_cast<const td_api::vectorPathCommandLine &>(object));
    case td_api::venue::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::venue &)>(td_api::to_json)(jv, static_cast<const td_api::venue &>(object));
    case td_api::verificationStatus::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::verificationStatus &)>(td_api::to_json)(jv, static_cast<const td_api::verificationStatus &>(object));
    case td_api::video::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::video &)>(td_api::to_json)(jv, static_cast<const td_api::video &>(object));
    case td_api::videoChat::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::videoChat &)>(td_api::to_json)(jv, static_cast<const td_api::videoChat &>(object));
    case td_api::videoChatStream::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::videoChatStream &)>(td_api::to_json)(jv, static_cast<const td_api::videoChatStream &>(object));
    case td_api::videoChatStreams::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::videoChatStreams &)>(td_api::to_json)(jv, static_cast<const td_api::videoChatStreams &>(object));
    case td_api::videoMessageAdvertisement::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::videoMessageAdvertisement &)>(td_api::to_json)(jv, static_cast<const td_api::videoMessageAdvertisement &>(object));
    case td_api::videoMessageAdvertisements::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::videoMessageAdvertisements &)>(td_api::to_json)(jv, static_cast<const td_api::videoMessageAdvertisements &>(object));
    case td_api::videoNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::videoNote &)>(td_api::to_json)(jv, static_cast<const td_api::videoNote &>(object));
    case td_api::videoStoryboard::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::videoStoryboard &)>(td_api::to_json)(jv, static_cast<const td_api::videoStoryboard &>(object));
    case td_api::voiceNote::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::voiceNote &)>(td_api::to_json)(jv, static_cast<const td_api::voiceNote &>(object));
    case td_api::webApp::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::webApp &)>(td_api::to_json)(jv, static_cast<const td_api::webApp &>(object));
    case td_api::webAppInfo::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::webAppInfo &)>(td_api::to_json)(jv, static_cast<const td_api::webAppInfo &>(object));
    case td_api::webAppOpenModeCompact::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::webAppOpenModeCompact &)>(td_api::to_json)(jv, static_cast<const td_api::webAppOpenModeCompact &>(object));
    case td_api::webAppOpenModeFullScreen::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::webAppOpenModeFullScreen &)>(td_api::to_json)(jv, static_cast<const td_api::webAppOpenModeFullScreen &>(object));
    case td_api::webAppOpenModeFullSize::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::webAppOpenModeFullSize &)>(td_api::to_json)(jv, static_cast<const td_api::webAppOpenModeFullSize &>(object));
    case td_api::webPageInstantView::ID:
      return static_cast<void(*)(JsonValueScope &, const td_api::webPageInstantView &)>(td_api::to_json)(jv, static_cast<const td_api::webPageInstantView &>(object));
    default:
      UNREACHABLE();
  }
}

}  // namespace td_api
}  // namespace td
