#pragma once

#include "td/telegram/td_api.h"

#include "td/utils/JsonBuilder.h"
#include "td/utils/Status.h"

namespace td {
namespace td_api {

void to_json(JsonValueScope &jv, const tl_object_ptr<Object> &value);

Status from_json(tl_object_ptr<Function> &to, td::JsonValue from);

void to_json(JsonValueScope &jv, const Object &object);

void to_json(JsonValueScope &jv, const Function &object);

Result<int32> tl_constructor_from_string(td_api::BackgroundFill *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::BackgroundType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::BotCommandScope *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::CallProblem *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::CallbackQueryPayload *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ChatAction *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ChatAvailableReactions *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ChatList *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ChatMemberStatus *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ChatMembersFilter *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ChatReportReason *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::DeviceToken *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::EmailAddressAuthentication *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::FileType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::GroupCallVideoQuality *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InlineKeyboardButtonType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputBackground *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputChatPhoto *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputCredentials *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputFile *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputInlineQueryResult *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputInvoice *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputMessageContent *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputPassportElement *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::InputPassportElementErrorSource *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::JsonValue *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::KeyboardButtonType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::LanguagePackStringValue *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::LogStream *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::MaskPoint *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::MessageSchedulingState *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::MessageSender *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::NetworkStatisticsEntry *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::NetworkType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::NotificationSettingsScope *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::OptionValue *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::PassportElementType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::PollType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::PremiumFeature *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::PremiumLimitType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::PremiumSource *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ProxyType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::PublicChatType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ReactionType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::ReplyMarkup *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::SearchMessagesFilter *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::StickerFormat *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::StickerType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::StorePaymentPurpose *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::SuggestedAction *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::SupergroupMembersFilter *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::TextEntityType *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::TextParseMode *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::TopChatCategory *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::UserPrivacySetting *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::UserPrivacySettingRule *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::Object *object, const std::string &str);

Result<int32> tl_constructor_from_string(td_api::Function *object, const std::string &str);

Status from_json(td_api::accountTtl &to, JsonObject &from);

Status from_json(td_api::address &to, JsonObject &from);

Status from_json(td_api::autoDownloadSettings &to, JsonObject &from);

Status from_json(td_api::backgroundFillSolid &to, JsonObject &from);

Status from_json(td_api::backgroundFillGradient &to, JsonObject &from);

Status from_json(td_api::backgroundFillFreeformGradient &to, JsonObject &from);

Status from_json(td_api::backgroundTypeWallpaper &to, JsonObject &from);

Status from_json(td_api::backgroundTypePattern &to, JsonObject &from);

Status from_json(td_api::backgroundTypeFill &to, JsonObject &from);

Status from_json(td_api::botCommand &to, JsonObject &from);

Status from_json(td_api::botCommandScopeDefault &to, JsonObject &from);

Status from_json(td_api::botCommandScopeAllPrivateChats &to, JsonObject &from);

Status from_json(td_api::botCommandScopeAllGroupChats &to, JsonObject &from);

Status from_json(td_api::botCommandScopeAllChatAdministrators &to, JsonObject &from);

Status from_json(td_api::botCommandScopeChat &to, JsonObject &from);

Status from_json(td_api::botCommandScopeChatAdministrators &to, JsonObject &from);

Status from_json(td_api::botCommandScopeChatMember &to, JsonObject &from);

Status from_json(td_api::botMenuButton &to, JsonObject &from);

Status from_json(td_api::callProblemEcho &to, JsonObject &from);

Status from_json(td_api::callProblemNoise &to, JsonObject &from);

Status from_json(td_api::callProblemInterruptions &to, JsonObject &from);

Status from_json(td_api::callProblemDistortedSpeech &to, JsonObject &from);

Status from_json(td_api::callProblemSilentLocal &to, JsonObject &from);

Status from_json(td_api::callProblemSilentRemote &to, JsonObject &from);

Status from_json(td_api::callProblemDropped &to, JsonObject &from);

Status from_json(td_api::callProblemDistortedVideo &to, JsonObject &from);

Status from_json(td_api::callProblemPixelatedVideo &to, JsonObject &from);

Status from_json(td_api::callProtocol &to, JsonObject &from);

Status from_json(td_api::callbackQueryPayloadData &to, JsonObject &from);

Status from_json(td_api::callbackQueryPayloadDataWithPassword &to, JsonObject &from);

Status from_json(td_api::callbackQueryPayloadGame &to, JsonObject &from);

Status from_json(td_api::chatActionTyping &to, JsonObject &from);

Status from_json(td_api::chatActionRecordingVideo &to, JsonObject &from);

Status from_json(td_api::chatActionUploadingVideo &to, JsonObject &from);

Status from_json(td_api::chatActionRecordingVoiceNote &to, JsonObject &from);

Status from_json(td_api::chatActionUploadingVoiceNote &to, JsonObject &from);

Status from_json(td_api::chatActionUploadingPhoto &to, JsonObject &from);

Status from_json(td_api::chatActionUploadingDocument &to, JsonObject &from);

Status from_json(td_api::chatActionChoosingSticker &to, JsonObject &from);

Status from_json(td_api::chatActionChoosingLocation &to, JsonObject &from);

Status from_json(td_api::chatActionChoosingContact &to, JsonObject &from);

Status from_json(td_api::chatActionStartPlayingGame &to, JsonObject &from);

Status from_json(td_api::chatActionRecordingVideoNote &to, JsonObject &from);

Status from_json(td_api::chatActionUploadingVideoNote &to, JsonObject &from);

Status from_json(td_api::chatActionWatchingAnimations &to, JsonObject &from);

Status from_json(td_api::chatActionCancel &to, JsonObject &from);

Status from_json(td_api::chatAdministratorRights &to, JsonObject &from);

Status from_json(td_api::chatAvailableReactionsAll &to, JsonObject &from);

Status from_json(td_api::chatAvailableReactionsSome &to, JsonObject &from);

Status from_json(td_api::chatEventLogFilters &to, JsonObject &from);

Status from_json(td_api::chatFilter &to, JsonObject &from);

Status from_json(td_api::chatInviteLinkMember &to, JsonObject &from);

Status from_json(td_api::chatJoinRequest &to, JsonObject &from);

Status from_json(td_api::chatListMain &to, JsonObject &from);

Status from_json(td_api::chatListArchive &to, JsonObject &from);

Status from_json(td_api::chatListFilter &to, JsonObject &from);

Status from_json(td_api::chatLocation &to, JsonObject &from);

Status from_json(td_api::chatMemberStatusCreator &to, JsonObject &from);

Status from_json(td_api::chatMemberStatusAdministrator &to, JsonObject &from);

Status from_json(td_api::chatMemberStatusMember &to, JsonObject &from);

Status from_json(td_api::chatMemberStatusRestricted &to, JsonObject &from);

Status from_json(td_api::chatMemberStatusLeft &to, JsonObject &from);

Status from_json(td_api::chatMemberStatusBanned &to, JsonObject &from);

Status from_json(td_api::chatMembersFilterContacts &to, JsonObject &from);

Status from_json(td_api::chatMembersFilterAdministrators &to, JsonObject &from);

Status from_json(td_api::chatMembersFilterMembers &to, JsonObject &from);

Status from_json(td_api::chatMembersFilterMention &to, JsonObject &from);

Status from_json(td_api::chatMembersFilterRestricted &to, JsonObject &from);

Status from_json(td_api::chatMembersFilterBanned &to, JsonObject &from);

Status from_json(td_api::chatMembersFilterBots &to, JsonObject &from);

Status from_json(td_api::chatNotificationSettings &to, JsonObject &from);

Status from_json(td_api::chatPermissions &to, JsonObject &from);

Status from_json(td_api::chatReportReasonSpam &to, JsonObject &from);

Status from_json(td_api::chatReportReasonViolence &to, JsonObject &from);

Status from_json(td_api::chatReportReasonPornography &to, JsonObject &from);

Status from_json(td_api::chatReportReasonChildAbuse &to, JsonObject &from);

Status from_json(td_api::chatReportReasonCopyright &to, JsonObject &from);

Status from_json(td_api::chatReportReasonUnrelatedLocation &to, JsonObject &from);

Status from_json(td_api::chatReportReasonFake &to, JsonObject &from);

Status from_json(td_api::chatReportReasonIllegalDrugs &to, JsonObject &from);

Status from_json(td_api::chatReportReasonPersonalDetails &to, JsonObject &from);

Status from_json(td_api::chatReportReasonCustom &to, JsonObject &from);

Status from_json(td_api::contact &to, JsonObject &from);

Status from_json(td_api::date &to, JsonObject &from);

Status from_json(td_api::deviceTokenFirebaseCloudMessaging &to, JsonObject &from);

Status from_json(td_api::deviceTokenApplePush &to, JsonObject &from);

Status from_json(td_api::deviceTokenApplePushVoIP &to, JsonObject &from);

Status from_json(td_api::deviceTokenWindowsPush &to, JsonObject &from);

Status from_json(td_api::deviceTokenMicrosoftPush &to, JsonObject &from);

Status from_json(td_api::deviceTokenMicrosoftPushVoIP &to, JsonObject &from);

Status from_json(td_api::deviceTokenWebPush &to, JsonObject &from);

Status from_json(td_api::deviceTokenSimplePush &to, JsonObject &from);

Status from_json(td_api::deviceTokenUbuntuPush &to, JsonObject &from);

Status from_json(td_api::deviceTokenBlackBerryPush &to, JsonObject &from);

Status from_json(td_api::deviceTokenTizenPush &to, JsonObject &from);

Status from_json(td_api::draftMessage &to, JsonObject &from);

Status from_json(td_api::emailAddressAuthenticationCode &to, JsonObject &from);

Status from_json(td_api::emailAddressAuthenticationAppleId &to, JsonObject &from);

Status from_json(td_api::emailAddressAuthenticationGoogleId &to, JsonObject &from);

Status from_json(td_api::emojiStatus &to, JsonObject &from);

Status from_json(td_api::error &to, JsonObject &from);

Status from_json(td_api::fileTypeNone &to, JsonObject &from);

Status from_json(td_api::fileTypeAnimation &to, JsonObject &from);

Status from_json(td_api::fileTypeAudio &to, JsonObject &from);

Status from_json(td_api::fileTypeDocument &to, JsonObject &from);

Status from_json(td_api::fileTypeNotificationSound &to, JsonObject &from);

Status from_json(td_api::fileTypePhoto &to, JsonObject &from);

Status from_json(td_api::fileTypeProfilePhoto &to, JsonObject &from);

Status from_json(td_api::fileTypeSecret &to, JsonObject &from);

Status from_json(td_api::fileTypeSecretThumbnail &to, JsonObject &from);

Status from_json(td_api::fileTypeSecure &to, JsonObject &from);

Status from_json(td_api::fileTypeSticker &to, JsonObject &from);

Status from_json(td_api::fileTypeThumbnail &to, JsonObject &from);

Status from_json(td_api::fileTypeUnknown &to, JsonObject &from);

Status from_json(td_api::fileTypeVideo &to, JsonObject &from);

Status from_json(td_api::fileTypeVideoNote &to, JsonObject &from);

Status from_json(td_api::fileTypeVoiceNote &to, JsonObject &from);

Status from_json(td_api::fileTypeWallpaper &to, JsonObject &from);

Status from_json(td_api::formattedText &to, JsonObject &from);

Status from_json(td_api::forumTopicIcon &to, JsonObject &from);

Status from_json(td_api::groupCallVideoQualityThumbnail &to, JsonObject &from);

Status from_json(td_api::groupCallVideoQualityMedium &to, JsonObject &from);

Status from_json(td_api::groupCallVideoQualityFull &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButton &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeUrl &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeLoginUrl &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeWebApp &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeCallback &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeCallbackWithPassword &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeCallbackGame &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeSwitchInline &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeBuy &to, JsonObject &from);

Status from_json(td_api::inlineKeyboardButtonTypeUser &to, JsonObject &from);

Status from_json(td_api::inputBackgroundLocal &to, JsonObject &from);

Status from_json(td_api::inputBackgroundRemote &to, JsonObject &from);

Status from_json(td_api::inputChatPhotoPrevious &to, JsonObject &from);

Status from_json(td_api::inputChatPhotoStatic &to, JsonObject &from);

Status from_json(td_api::inputChatPhotoAnimation &to, JsonObject &from);

Status from_json(td_api::inputCredentialsSaved &to, JsonObject &from);

Status from_json(td_api::inputCredentialsNew &to, JsonObject &from);

Status from_json(td_api::inputCredentialsApplePay &to, JsonObject &from);

Status from_json(td_api::inputCredentialsGooglePay &to, JsonObject &from);

Status from_json(td_api::inputFileId &to, JsonObject &from);

Status from_json(td_api::inputFileRemote &to, JsonObject &from);

Status from_json(td_api::inputFileLocal &to, JsonObject &from);

Status from_json(td_api::inputFileGenerated &to, JsonObject &from);

Status from_json(td_api::inputIdentityDocument &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultAnimation &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultArticle &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultAudio &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultContact &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultDocument &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultGame &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultLocation &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultPhoto &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultSticker &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultVenue &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultVideo &to, JsonObject &from);

Status from_json(td_api::inputInlineQueryResultVoiceNote &to, JsonObject &from);

Status from_json(td_api::inputInvoiceMessage &to, JsonObject &from);

Status from_json(td_api::inputInvoiceName &to, JsonObject &from);

Status from_json(td_api::inputMessageText &to, JsonObject &from);

Status from_json(td_api::inputMessageAnimation &to, JsonObject &from);

Status from_json(td_api::inputMessageAudio &to, JsonObject &from);

Status from_json(td_api::inputMessageDocument &to, JsonObject &from);

Status from_json(td_api::inputMessagePhoto &to, JsonObject &from);

Status from_json(td_api::inputMessageSticker &to, JsonObject &from);

Status from_json(td_api::inputMessageVideo &to, JsonObject &from);

Status from_json(td_api::inputMessageVideoNote &to, JsonObject &from);

Status from_json(td_api::inputMessageVoiceNote &to, JsonObject &from);

Status from_json(td_api::inputMessageLocation &to, JsonObject &from);

Status from_json(td_api::inputMessageVenue &to, JsonObject &from);

Status from_json(td_api::inputMessageContact &to, JsonObject &from);

Status from_json(td_api::inputMessageDice &to, JsonObject &from);

Status from_json(td_api::inputMessageGame &to, JsonObject &from);

Status from_json(td_api::inputMessageInvoice &to, JsonObject &from);

Status from_json(td_api::inputMessagePoll &to, JsonObject &from);

Status from_json(td_api::inputMessageForwarded &to, JsonObject &from);

Status from_json(td_api::inputPassportElementPersonalDetails &to, JsonObject &from);

Status from_json(td_api::inputPassportElementPassport &to, JsonObject &from);

Status from_json(td_api::inputPassportElementDriverLicense &to, JsonObject &from);

Status from_json(td_api::inputPassportElementIdentityCard &to, JsonObject &from);

Status from_json(td_api::inputPassportElementInternalPassport &to, JsonObject &from);

Status from_json(td_api::inputPassportElementAddress &to, JsonObject &from);

Status from_json(td_api::inputPassportElementUtilityBill &to, JsonObject &from);

Status from_json(td_api::inputPassportElementBankStatement &to, JsonObject &from);

Status from_json(td_api::inputPassportElementRentalAgreement &to, JsonObject &from);

Status from_json(td_api::inputPassportElementPassportRegistration &to, JsonObject &from);

Status from_json(td_api::inputPassportElementTemporaryRegistration &to, JsonObject &from);

Status from_json(td_api::inputPassportElementPhoneNumber &to, JsonObject &from);

Status from_json(td_api::inputPassportElementEmailAddress &to, JsonObject &from);

Status from_json(td_api::inputPassportElementError &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceUnspecified &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceDataField &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceFrontSide &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceReverseSide &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceSelfie &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceTranslationFile &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceTranslationFiles &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceFile &to, JsonObject &from);

Status from_json(td_api::inputPassportElementErrorSourceFiles &to, JsonObject &from);

Status from_json(td_api::inputPersonalDocument &to, JsonObject &from);

Status from_json(td_api::inputSticker &to, JsonObject &from);

Status from_json(td_api::inputThumbnail &to, JsonObject &from);

Status from_json(td_api::invoice &to, JsonObject &from);

Status from_json(td_api::jsonObjectMember &to, JsonObject &from);

Status from_json(td_api::jsonValueNull &to, JsonObject &from);

Status from_json(td_api::jsonValueBoolean &to, JsonObject &from);

Status from_json(td_api::jsonValueNumber &to, JsonObject &from);

Status from_json(td_api::jsonValueString &to, JsonObject &from);

Status from_json(td_api::jsonValueArray &to, JsonObject &from);

Status from_json(td_api::jsonValueObject &to, JsonObject &from);

Status from_json(td_api::keyboardButton &to, JsonObject &from);

Status from_json(td_api::keyboardButtonTypeText &to, JsonObject &from);

Status from_json(td_api::keyboardButtonTypeRequestPhoneNumber &to, JsonObject &from);

Status from_json(td_api::keyboardButtonTypeRequestLocation &to, JsonObject &from);

Status from_json(td_api::keyboardButtonTypeRequestPoll &to, JsonObject &from);

Status from_json(td_api::keyboardButtonTypeWebApp &to, JsonObject &from);

Status from_json(td_api::labeledPricePart &to, JsonObject &from);

Status from_json(td_api::languagePackInfo &to, JsonObject &from);

Status from_json(td_api::languagePackString &to, JsonObject &from);

Status from_json(td_api::languagePackStringValueOrdinary &to, JsonObject &from);

Status from_json(td_api::languagePackStringValuePluralized &to, JsonObject &from);

Status from_json(td_api::languagePackStringValueDeleted &to, JsonObject &from);

Status from_json(td_api::location &to, JsonObject &from);

Status from_json(td_api::logStreamDefault &to, JsonObject &from);

Status from_json(td_api::logStreamFile &to, JsonObject &from);

Status from_json(td_api::logStreamEmpty &to, JsonObject &from);

Status from_json(td_api::maskPointForehead &to, JsonObject &from);

Status from_json(td_api::maskPointEyes &to, JsonObject &from);

Status from_json(td_api::maskPointMouth &to, JsonObject &from);

Status from_json(td_api::maskPointChin &to, JsonObject &from);

Status from_json(td_api::maskPosition &to, JsonObject &from);

Status from_json(td_api::messageCopyOptions &to, JsonObject &from);

Status from_json(td_api::messageSchedulingStateSendAtDate &to, JsonObject &from);

Status from_json(td_api::messageSchedulingStateSendWhenOnline &to, JsonObject &from);

Status from_json(td_api::messageSendOptions &to, JsonObject &from);

Status from_json(td_api::messageSenderUser &to, JsonObject &from);

Status from_json(td_api::messageSenderChat &to, JsonObject &from);

Status from_json(td_api::networkStatisticsEntryFile &to, JsonObject &from);

Status from_json(td_api::networkStatisticsEntryCall &to, JsonObject &from);

Status from_json(td_api::networkTypeNone &to, JsonObject &from);

Status from_json(td_api::networkTypeMobile &to, JsonObject &from);

Status from_json(td_api::networkTypeMobileRoaming &to, JsonObject &from);

Status from_json(td_api::networkTypeWiFi &to, JsonObject &from);

Status from_json(td_api::networkTypeOther &to, JsonObject &from);

Status from_json(td_api::notificationSettingsScopePrivateChats &to, JsonObject &from);

Status from_json(td_api::notificationSettingsScopeGroupChats &to, JsonObject &from);

Status from_json(td_api::notificationSettingsScopeChannelChats &to, JsonObject &from);

Status from_json(td_api::optionValueBoolean &to, JsonObject &from);

Status from_json(td_api::optionValueEmpty &to, JsonObject &from);

Status from_json(td_api::optionValueInteger &to, JsonObject &from);

Status from_json(td_api::optionValueString &to, JsonObject &from);

Status from_json(td_api::orderInfo &to, JsonObject &from);

Status from_json(td_api::passportElementTypePersonalDetails &to, JsonObject &from);

Status from_json(td_api::passportElementTypePassport &to, JsonObject &from);

Status from_json(td_api::passportElementTypeDriverLicense &to, JsonObject &from);

Status from_json(td_api::passportElementTypeIdentityCard &to, JsonObject &from);

Status from_json(td_api::passportElementTypeInternalPassport &to, JsonObject &from);

Status from_json(td_api::passportElementTypeAddress &to, JsonObject &from);

Status from_json(td_api::passportElementTypeUtilityBill &to, JsonObject &from);

Status from_json(td_api::passportElementTypeBankStatement &to, JsonObject &from);

Status from_json(td_api::passportElementTypeRentalAgreement &to, JsonObject &from);

Status from_json(td_api::passportElementTypePassportRegistration &to, JsonObject &from);

Status from_json(td_api::passportElementTypeTemporaryRegistration &to, JsonObject &from);

Status from_json(td_api::passportElementTypePhoneNumber &to, JsonObject &from);

Status from_json(td_api::passportElementTypeEmailAddress &to, JsonObject &from);

Status from_json(td_api::personalDetails &to, JsonObject &from);

Status from_json(td_api::phoneNumberAuthenticationSettings &to, JsonObject &from);

Status from_json(td_api::pollTypeRegular &to, JsonObject &from);

Status from_json(td_api::pollTypeQuiz &to, JsonObject &from);

Status from_json(td_api::premiumFeatureIncreasedLimits &to, JsonObject &from);

Status from_json(td_api::premiumFeatureIncreasedUploadFileSize &to, JsonObject &from);

Status from_json(td_api::premiumFeatureImprovedDownloadSpeed &to, JsonObject &from);

Status from_json(td_api::premiumFeatureVoiceRecognition &to, JsonObject &from);

Status from_json(td_api::premiumFeatureDisabledAds &to, JsonObject &from);

Status from_json(td_api::premiumFeatureUniqueReactions &to, JsonObject &from);

Status from_json(td_api::premiumFeatureUniqueStickers &to, JsonObject &from);

Status from_json(td_api::premiumFeatureCustomEmoji &to, JsonObject &from);

Status from_json(td_api::premiumFeatureAdvancedChatManagement &to, JsonObject &from);

Status from_json(td_api::premiumFeatureProfileBadge &to, JsonObject &from);

Status from_json(td_api::premiumFeatureEmojiStatus &to, JsonObject &from);

Status from_json(td_api::premiumFeatureAnimatedProfilePhoto &to, JsonObject &from);

Status from_json(td_api::premiumFeatureForumTopicIcon &to, JsonObject &from);

Status from_json(td_api::premiumFeatureAppIcons &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeSupergroupCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypePinnedChatCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeCreatedPublicChatCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeSavedAnimationCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeFavoriteStickerCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeChatFilterCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeChatFilterChosenChatCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypePinnedArchivedChatCount &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeCaptionLength &to, JsonObject &from);

Status from_json(td_api::premiumLimitTypeBioLength &to, JsonObject &from);

Status from_json(td_api::premiumSourceLimitExceeded &to, JsonObject &from);

Status from_json(td_api::premiumSourceFeature &to, JsonObject &from);

Status from_json(td_api::premiumSourceLink &to, JsonObject &from);

Status from_json(td_api::premiumSourceSettings &to, JsonObject &from);

Status from_json(td_api::proxyTypeSocks5 &to, JsonObject &from);

Status from_json(td_api::proxyTypeHttp &to, JsonObject &from);

Status from_json(td_api::proxyTypeMtproto &to, JsonObject &from);

Status from_json(td_api::publicChatTypeHasUsername &to, JsonObject &from);

Status from_json(td_api::publicChatTypeIsLocationBased &to, JsonObject &from);

Status from_json(td_api::reactionTypeEmoji &to, JsonObject &from);

Status from_json(td_api::reactionTypeCustomEmoji &to, JsonObject &from);

Status from_json(td_api::replyMarkupRemoveKeyboard &to, JsonObject &from);

Status from_json(td_api::replyMarkupForceReply &to, JsonObject &from);

Status from_json(td_api::replyMarkupShowKeyboard &to, JsonObject &from);

Status from_json(td_api::replyMarkupInlineKeyboard &to, JsonObject &from);

Status from_json(td_api::scopeNotificationSettings &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterEmpty &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterAnimation &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterAudio &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterDocument &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterPhoto &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterVideo &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterVoiceNote &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterPhotoAndVideo &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterUrl &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterChatPhoto &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterVideoNote &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterVoiceAndVideoNote &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterMention &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterUnreadMention &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterUnreadReaction &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterFailedToSend &to, JsonObject &from);

Status from_json(td_api::searchMessagesFilterPinned &to, JsonObject &from);

Status from_json(td_api::shippingOption &to, JsonObject &from);

Status from_json(td_api::stickerFormatWebp &to, JsonObject &from);

Status from_json(td_api::stickerFormatTgs &to, JsonObject &from);

Status from_json(td_api::stickerFormatWebm &to, JsonObject &from);

Status from_json(td_api::stickerTypeRegular &to, JsonObject &from);

Status from_json(td_api::stickerTypeMask &to, JsonObject &from);

Status from_json(td_api::stickerTypeCustomEmoji &to, JsonObject &from);

Status from_json(td_api::storePaymentPurposePremiumSubscription &to, JsonObject &from);

Status from_json(td_api::storePaymentPurposeGiftedPremium &to, JsonObject &from);

Status from_json(td_api::suggestedActionEnableArchiveAndMuteNewChats &to, JsonObject &from);

Status from_json(td_api::suggestedActionCheckPassword &to, JsonObject &from);

Status from_json(td_api::suggestedActionCheckPhoneNumber &to, JsonObject &from);

Status from_json(td_api::suggestedActionViewChecksHint &to, JsonObject &from);

Status from_json(td_api::suggestedActionConvertToBroadcastGroup &to, JsonObject &from);

Status from_json(td_api::suggestedActionSetPassword &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterRecent &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterContacts &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterAdministrators &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterSearch &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterRestricted &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterBanned &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterMention &to, JsonObject &from);

Status from_json(td_api::supergroupMembersFilterBots &to, JsonObject &from);

Status from_json(td_api::testInt &to, JsonObject &from);

Status from_json(td_api::testString &to, JsonObject &from);

Status from_json(td_api::textEntity &to, JsonObject &from);

Status from_json(td_api::textEntityTypeMention &to, JsonObject &from);

Status from_json(td_api::textEntityTypeHashtag &to, JsonObject &from);

Status from_json(td_api::textEntityTypeCashtag &to, JsonObject &from);

Status from_json(td_api::textEntityTypeBotCommand &to, JsonObject &from);

Status from_json(td_api::textEntityTypeUrl &to, JsonObject &from);

Status from_json(td_api::textEntityTypeEmailAddress &to, JsonObject &from);

Status from_json(td_api::textEntityTypePhoneNumber &to, JsonObject &from);

Status from_json(td_api::textEntityTypeBankCardNumber &to, JsonObject &from);

Status from_json(td_api::textEntityTypeBold &to, JsonObject &from);

Status from_json(td_api::textEntityTypeItalic &to, JsonObject &from);

Status from_json(td_api::textEntityTypeUnderline &to, JsonObject &from);

Status from_json(td_api::textEntityTypeStrikethrough &to, JsonObject &from);

Status from_json(td_api::textEntityTypeSpoiler &to, JsonObject &from);

Status from_json(td_api::textEntityTypeCode &to, JsonObject &from);

Status from_json(td_api::textEntityTypePre &to, JsonObject &from);

Status from_json(td_api::textEntityTypePreCode &to, JsonObject &from);

Status from_json(td_api::textEntityTypeTextUrl &to, JsonObject &from);

Status from_json(td_api::textEntityTypeMentionName &to, JsonObject &from);

Status from_json(td_api::textEntityTypeCustomEmoji &to, JsonObject &from);

Status from_json(td_api::textEntityTypeMediaTimestamp &to, JsonObject &from);

Status from_json(td_api::textParseModeMarkdown &to, JsonObject &from);

Status from_json(td_api::textParseModeHTML &to, JsonObject &from);

Status from_json(td_api::themeParameters &to, JsonObject &from);

Status from_json(td_api::topChatCategoryUsers &to, JsonObject &from);

Status from_json(td_api::topChatCategoryBots &to, JsonObject &from);

Status from_json(td_api::topChatCategoryGroups &to, JsonObject &from);

Status from_json(td_api::topChatCategoryChannels &to, JsonObject &from);

Status from_json(td_api::topChatCategoryInlineBots &to, JsonObject &from);

Status from_json(td_api::topChatCategoryCalls &to, JsonObject &from);

Status from_json(td_api::topChatCategoryForwardChats &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingShowStatus &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingShowProfilePhoto &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingShowLinkInForwardedMessages &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingShowPhoneNumber &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingAllowChatInvites &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingAllowCalls &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingAllowPeerToPeerCalls &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingAllowFindingByPhoneNumber &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleAllowAll &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleAllowContacts &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleAllowUsers &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleAllowChatMembers &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleRestrictAll &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleRestrictContacts &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleRestrictUsers &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRuleRestrictChatMembers &to, JsonObject &from);

Status from_json(td_api::userPrivacySettingRules &to, JsonObject &from);

Status from_json(td_api::venue &to, JsonObject &from);

Status from_json(td_api::acceptCall &to, JsonObject &from);

Status from_json(td_api::acceptTermsOfService &to, JsonObject &from);

Status from_json(td_api::addChatMember &to, JsonObject &from);

Status from_json(td_api::addChatMembers &to, JsonObject &from);

Status from_json(td_api::addChatToList &to, JsonObject &from);

Status from_json(td_api::addContact &to, JsonObject &from);

Status from_json(td_api::addCustomServerLanguagePack &to, JsonObject &from);

Status from_json(td_api::addFavoriteSticker &to, JsonObject &from);

Status from_json(td_api::addFileToDownloads &to, JsonObject &from);

Status from_json(td_api::addLocalMessage &to, JsonObject &from);

Status from_json(td_api::addLogMessage &to, JsonObject &from);

Status from_json(td_api::addMessageReaction &to, JsonObject &from);

Status from_json(td_api::addNetworkStatistics &to, JsonObject &from);

Status from_json(td_api::addProxy &to, JsonObject &from);

Status from_json(td_api::addRecentSticker &to, JsonObject &from);

Status from_json(td_api::addRecentlyFoundChat &to, JsonObject &from);

Status from_json(td_api::addSavedAnimation &to, JsonObject &from);

Status from_json(td_api::addSavedNotificationSound &to, JsonObject &from);

Status from_json(td_api::addStickerToSet &to, JsonObject &from);

Status from_json(td_api::answerCallbackQuery &to, JsonObject &from);

Status from_json(td_api::answerCustomQuery &to, JsonObject &from);

Status from_json(td_api::answerInlineQuery &to, JsonObject &from);

Status from_json(td_api::answerPreCheckoutQuery &to, JsonObject &from);

Status from_json(td_api::answerShippingQuery &to, JsonObject &from);

Status from_json(td_api::answerWebAppQuery &to, JsonObject &from);

Status from_json(td_api::assignAppStoreTransaction &to, JsonObject &from);

Status from_json(td_api::assignGooglePlayTransaction &to, JsonObject &from);

Status from_json(td_api::banChatMember &to, JsonObject &from);

Status from_json(td_api::blockMessageSenderFromReplies &to, JsonObject &from);

Status from_json(td_api::canPurchasePremium &to, JsonObject &from);

Status from_json(td_api::canTransferOwnership &to, JsonObject &from);

Status from_json(td_api::cancelDownloadFile &to, JsonObject &from);

Status from_json(td_api::cancelPasswordReset &to, JsonObject &from);

Status from_json(td_api::cancelPreliminaryUploadFile &to, JsonObject &from);

Status from_json(td_api::changeImportedContacts &to, JsonObject &from);

Status from_json(td_api::changePhoneNumber &to, JsonObject &from);

Status from_json(td_api::changeStickerSet &to, JsonObject &from);

Status from_json(td_api::checkAuthenticationBotToken &to, JsonObject &from);

Status from_json(td_api::checkAuthenticationCode &to, JsonObject &from);

Status from_json(td_api::checkAuthenticationEmailCode &to, JsonObject &from);

Status from_json(td_api::checkAuthenticationPassword &to, JsonObject &from);

Status from_json(td_api::checkAuthenticationPasswordRecoveryCode &to, JsonObject &from);

Status from_json(td_api::checkChangePhoneNumberCode &to, JsonObject &from);

Status from_json(td_api::checkChatInviteLink &to, JsonObject &from);

Status from_json(td_api::checkChatUsername &to, JsonObject &from);

Status from_json(td_api::checkCreatedPublicChatsLimit &to, JsonObject &from);

Status from_json(td_api::checkEmailAddressVerificationCode &to, JsonObject &from);

Status from_json(td_api::checkLoginEmailAddressCode &to, JsonObject &from);

Status from_json(td_api::checkPasswordRecoveryCode &to, JsonObject &from);

Status from_json(td_api::checkPhoneNumberConfirmationCode &to, JsonObject &from);

Status from_json(td_api::checkPhoneNumberVerificationCode &to, JsonObject &from);

Status from_json(td_api::checkRecoveryEmailAddressCode &to, JsonObject &from);

Status from_json(td_api::checkStickerSetName &to, JsonObject &from);

Status from_json(td_api::cleanFileName &to, JsonObject &from);

Status from_json(td_api::clearAllDraftMessages &to, JsonObject &from);

Status from_json(td_api::clearImportedContacts &to, JsonObject &from);

Status from_json(td_api::clearRecentEmojiStatuses &to, JsonObject &from);

Status from_json(td_api::clearRecentReactions &to, JsonObject &from);

Status from_json(td_api::clearRecentStickers &to, JsonObject &from);

Status from_json(td_api::clearRecentlyFoundChats &to, JsonObject &from);

Status from_json(td_api::clickAnimatedEmojiMessage &to, JsonObject &from);

Status from_json(td_api::clickPremiumSubscriptionButton &to, JsonObject &from);

Status from_json(td_api::close &to, JsonObject &from);

Status from_json(td_api::closeChat &to, JsonObject &from);

Status from_json(td_api::closeSecretChat &to, JsonObject &from);

Status from_json(td_api::closeWebApp &to, JsonObject &from);

Status from_json(td_api::confirmQrCodeAuthentication &to, JsonObject &from);

Status from_json(td_api::createBasicGroupChat &to, JsonObject &from);

Status from_json(td_api::createCall &to, JsonObject &from);

Status from_json(td_api::createChatFilter &to, JsonObject &from);

Status from_json(td_api::createChatInviteLink &to, JsonObject &from);

Status from_json(td_api::createForumTopic &to, JsonObject &from);

Status from_json(td_api::createInvoiceLink &to, JsonObject &from);

Status from_json(td_api::createNewBasicGroupChat &to, JsonObject &from);

Status from_json(td_api::createNewSecretChat &to, JsonObject &from);

Status from_json(td_api::createNewStickerSet &to, JsonObject &from);

Status from_json(td_api::createNewSupergroupChat &to, JsonObject &from);

Status from_json(td_api::createPrivateChat &to, JsonObject &from);

Status from_json(td_api::createSecretChat &to, JsonObject &from);

Status from_json(td_api::createSupergroupChat &to, JsonObject &from);

Status from_json(td_api::createTemporaryPassword &to, JsonObject &from);

Status from_json(td_api::createVideoChat &to, JsonObject &from);

Status from_json(td_api::deleteAccount &to, JsonObject &from);

Status from_json(td_api::deleteAllCallMessages &to, JsonObject &from);

Status from_json(td_api::deleteAllRevokedChatInviteLinks &to, JsonObject &from);

Status from_json(td_api::deleteChat &to, JsonObject &from);

Status from_json(td_api::deleteChatFilter &to, JsonObject &from);

Status from_json(td_api::deleteChatHistory &to, JsonObject &from);

Status from_json(td_api::deleteChatMessagesByDate &to, JsonObject &from);

Status from_json(td_api::deleteChatMessagesBySender &to, JsonObject &from);

Status from_json(td_api::deleteChatReplyMarkup &to, JsonObject &from);

Status from_json(td_api::deleteCommands &to, JsonObject &from);

Status from_json(td_api::deleteFile &to, JsonObject &from);

Status from_json(td_api::deleteForumTopic &to, JsonObject &from);

Status from_json(td_api::deleteLanguagePack &to, JsonObject &from);

Status from_json(td_api::deleteMessages &to, JsonObject &from);

Status from_json(td_api::deletePassportElement &to, JsonObject &from);

Status from_json(td_api::deleteProfilePhoto &to, JsonObject &from);

Status from_json(td_api::deleteRevokedChatInviteLink &to, JsonObject &from);

Status from_json(td_api::deleteSavedCredentials &to, JsonObject &from);

Status from_json(td_api::deleteSavedOrderInfo &to, JsonObject &from);

Status from_json(td_api::destroy &to, JsonObject &from);

Status from_json(td_api::disableAllSupergroupUsernames &to, JsonObject &from);

Status from_json(td_api::disableProxy &to, JsonObject &from);

Status from_json(td_api::discardCall &to, JsonObject &from);

Status from_json(td_api::disconnectAllWebsites &to, JsonObject &from);

Status from_json(td_api::disconnectWebsite &to, JsonObject &from);

Status from_json(td_api::downloadFile &to, JsonObject &from);

Status from_json(td_api::editChatFilter &to, JsonObject &from);

Status from_json(td_api::editChatInviteLink &to, JsonObject &from);

Status from_json(td_api::editCustomLanguagePackInfo &to, JsonObject &from);

Status from_json(td_api::editForumTopic &to, JsonObject &from);

Status from_json(td_api::editInlineMessageCaption &to, JsonObject &from);

Status from_json(td_api::editInlineMessageLiveLocation &to, JsonObject &from);

Status from_json(td_api::editInlineMessageMedia &to, JsonObject &from);

Status from_json(td_api::editInlineMessageReplyMarkup &to, JsonObject &from);

Status from_json(td_api::editInlineMessageText &to, JsonObject &from);

Status from_json(td_api::editMessageCaption &to, JsonObject &from);

Status from_json(td_api::editMessageLiveLocation &to, JsonObject &from);

Status from_json(td_api::editMessageMedia &to, JsonObject &from);

Status from_json(td_api::editMessageReplyMarkup &to, JsonObject &from);

Status from_json(td_api::editMessageSchedulingState &to, JsonObject &from);

Status from_json(td_api::editMessageText &to, JsonObject &from);

Status from_json(td_api::editProxy &to, JsonObject &from);

Status from_json(td_api::enableProxy &to, JsonObject &from);

Status from_json(td_api::endGroupCall &to, JsonObject &from);

Status from_json(td_api::endGroupCallRecording &to, JsonObject &from);

Status from_json(td_api::endGroupCallScreenSharing &to, JsonObject &from);

Status from_json(td_api::finishFileGeneration &to, JsonObject &from);

Status from_json(td_api::forwardMessages &to, JsonObject &from);

Status from_json(td_api::getAccountTtl &to, JsonObject &from);

Status from_json(td_api::getActiveLiveLocationMessages &to, JsonObject &from);

Status from_json(td_api::getActiveSessions &to, JsonObject &from);

Status from_json(td_api::getAllPassportElements &to, JsonObject &from);

Status from_json(td_api::getAnimatedEmoji &to, JsonObject &from);

Status from_json(td_api::getApplicationConfig &to, JsonObject &from);

Status from_json(td_api::getApplicationDownloadLink &to, JsonObject &from);

Status from_json(td_api::getArchivedStickerSets &to, JsonObject &from);

Status from_json(td_api::getAttachedStickerSets &to, JsonObject &from);

Status from_json(td_api::getAttachmentMenuBot &to, JsonObject &from);

Status from_json(td_api::getAuthorizationState &to, JsonObject &from);

Status from_json(td_api::getAutoDownloadSettingsPresets &to, JsonObject &from);

Status from_json(td_api::getBackgroundUrl &to, JsonObject &from);

Status from_json(td_api::getBackgrounds &to, JsonObject &from);

Status from_json(td_api::getBankCardInfo &to, JsonObject &from);

Status from_json(td_api::getBasicGroup &to, JsonObject &from);

Status from_json(td_api::getBasicGroupFullInfo &to, JsonObject &from);

Status from_json(td_api::getBlockedMessageSenders &to, JsonObject &from);

Status from_json(td_api::getCallbackQueryAnswer &to, JsonObject &from);

Status from_json(td_api::getCallbackQueryMessage &to, JsonObject &from);

Status from_json(td_api::getChat &to, JsonObject &from);

Status from_json(td_api::getChatAdministrators &to, JsonObject &from);

Status from_json(td_api::getChatAvailableMessageSenders &to, JsonObject &from);

Status from_json(td_api::getChatEventLog &to, JsonObject &from);

Status from_json(td_api::getChatFilter &to, JsonObject &from);

Status from_json(td_api::getChatFilterDefaultIconName &to, JsonObject &from);

Status from_json(td_api::getChatHistory &to, JsonObject &from);

Status from_json(td_api::getChatInviteLink &to, JsonObject &from);

Status from_json(td_api::getChatInviteLinkCounts &to, JsonObject &from);

Status from_json(td_api::getChatInviteLinkMembers &to, JsonObject &from);

Status from_json(td_api::getChatInviteLinks &to, JsonObject &from);

Status from_json(td_api::getChatJoinRequests &to, JsonObject &from);

Status from_json(td_api::getChatListsToAddChat &to, JsonObject &from);

Status from_json(td_api::getChatMember &to, JsonObject &from);

Status from_json(td_api::getChatMessageByDate &to, JsonObject &from);

Status from_json(td_api::getChatMessageCalendar &to, JsonObject &from);

Status from_json(td_api::getChatMessageCount &to, JsonObject &from);

Status from_json(td_api::getChatMessagePosition &to, JsonObject &from);

Status from_json(td_api::getChatNotificationSettingsExceptions &to, JsonObject &from);

Status from_json(td_api::getChatPinnedMessage &to, JsonObject &from);

Status from_json(td_api::getChatScheduledMessages &to, JsonObject &from);

Status from_json(td_api::getChatSparseMessagePositions &to, JsonObject &from);

Status from_json(td_api::getChatSponsoredMessages &to, JsonObject &from);

Status from_json(td_api::getChatStatistics &to, JsonObject &from);

Status from_json(td_api::getChats &to, JsonObject &from);

Status from_json(td_api::getCommands &to, JsonObject &from);

Status from_json(td_api::getConnectedWebsites &to, JsonObject &from);

Status from_json(td_api::getContacts &to, JsonObject &from);

Status from_json(td_api::getCountries &to, JsonObject &from);

Status from_json(td_api::getCountryCode &to, JsonObject &from);

Status from_json(td_api::getCreatedPublicChats &to, JsonObject &from);

Status from_json(td_api::getCurrentState &to, JsonObject &from);

Status from_json(td_api::getCustomEmojiReactionAnimations &to, JsonObject &from);

Status from_json(td_api::getCustomEmojiStickers &to, JsonObject &from);

Status from_json(td_api::getDatabaseStatistics &to, JsonObject &from);

Status from_json(td_api::getDeepLinkInfo &to, JsonObject &from);

Status from_json(td_api::getDefaultEmojiStatuses &to, JsonObject &from);

Status from_json(td_api::getEmojiReaction &to, JsonObject &from);

Status from_json(td_api::getEmojiSuggestionsUrl &to, JsonObject &from);

Status from_json(td_api::getExternalLink &to, JsonObject &from);

Status from_json(td_api::getExternalLinkInfo &to, JsonObject &from);

Status from_json(td_api::getFavoriteStickers &to, JsonObject &from);

Status from_json(td_api::getFile &to, JsonObject &from);

Status from_json(td_api::getFileDownloadedPrefixSize &to, JsonObject &from);

Status from_json(td_api::getFileExtension &to, JsonObject &from);

Status from_json(td_api::getFileMimeType &to, JsonObject &from);

Status from_json(td_api::getForumTopicDefaultIcons &to, JsonObject &from);

Status from_json(td_api::getGameHighScores &to, JsonObject &from);

Status from_json(td_api::getGroupCall &to, JsonObject &from);

Status from_json(td_api::getGroupCallInviteLink &to, JsonObject &from);

Status from_json(td_api::getGroupCallStreamSegment &to, JsonObject &from);

Status from_json(td_api::getGroupCallStreams &to, JsonObject &from);

Status from_json(td_api::getGroupsInCommon &to, JsonObject &from);

Status from_json(td_api::getImportedContactCount &to, JsonObject &from);

Status from_json(td_api::getInactiveSupergroupChats &to, JsonObject &from);

Status from_json(td_api::getInlineGameHighScores &to, JsonObject &from);

Status from_json(td_api::getInlineQueryResults &to, JsonObject &from);

Status from_json(td_api::getInstalledStickerSets &to, JsonObject &from);

Status from_json(td_api::getInternalLinkType &to, JsonObject &from);

Status from_json(td_api::getJsonString &to, JsonObject &from);

Status from_json(td_api::getJsonValue &to, JsonObject &from);

Status from_json(td_api::getLanguagePackInfo &to, JsonObject &from);

Status from_json(td_api::getLanguagePackString &to, JsonObject &from);

Status from_json(td_api::getLanguagePackStrings &to, JsonObject &from);

Status from_json(td_api::getLocalizationTargetInfo &to, JsonObject &from);

Status from_json(td_api::getLogStream &to, JsonObject &from);

Status from_json(td_api::getLogTagVerbosityLevel &to, JsonObject &from);

Status from_json(td_api::getLogTags &to, JsonObject &from);

Status from_json(td_api::getLogVerbosityLevel &to, JsonObject &from);

Status from_json(td_api::getLoginUrl &to, JsonObject &from);

Status from_json(td_api::getLoginUrlInfo &to, JsonObject &from);

Status from_json(td_api::getMapThumbnailFile &to, JsonObject &from);

Status from_json(td_api::getMarkdownText &to, JsonObject &from);

Status from_json(td_api::getMe &to, JsonObject &from);

Status from_json(td_api::getMenuButton &to, JsonObject &from);

Status from_json(td_api::getMessage &to, JsonObject &from);

Status from_json(td_api::getMessageAddedReactions &to, JsonObject &from);

Status from_json(td_api::getMessageAvailableReactions &to, JsonObject &from);

Status from_json(td_api::getMessageEmbeddingCode &to, JsonObject &from);

Status from_json(td_api::getMessageFileType &to, JsonObject &from);

Status from_json(td_api::getMessageImportConfirmationText &to, JsonObject &from);

Status from_json(td_api::getMessageLink &to, JsonObject &from);

Status from_json(td_api::getMessageLinkInfo &to, JsonObject &from);

Status from_json(td_api::getMessageLocally &to, JsonObject &from);

Status from_json(td_api::getMessagePublicForwards &to, JsonObject &from);

Status from_json(td_api::getMessageStatistics &to, JsonObject &from);

Status from_json(td_api::getMessageThread &to, JsonObject &from);

Status from_json(td_api::getMessageThreadHistory &to, JsonObject &from);

Status from_json(td_api::getMessageViewers &to, JsonObject &from);

Status from_json(td_api::getMessages &to, JsonObject &from);

Status from_json(td_api::getNetworkStatistics &to, JsonObject &from);

Status from_json(td_api::getOption &to, JsonObject &from);

Status from_json(td_api::getPassportAuthorizationForm &to, JsonObject &from);

Status from_json(td_api::getPassportAuthorizationFormAvailableElements &to, JsonObject &from);

Status from_json(td_api::getPassportElement &to, JsonObject &from);

Status from_json(td_api::getPasswordState &to, JsonObject &from);

Status from_json(td_api::getPaymentForm &to, JsonObject &from);

Status from_json(td_api::getPaymentReceipt &to, JsonObject &from);

Status from_json(td_api::getPhoneNumberInfo &to, JsonObject &from);

Status from_json(td_api::getPhoneNumberInfoSync &to, JsonObject &from);

Status from_json(td_api::getPollVoters &to, JsonObject &from);

Status from_json(td_api::getPreferredCountryLanguage &to, JsonObject &from);

Status from_json(td_api::getPremiumFeatures &to, JsonObject &from);

Status from_json(td_api::getPremiumLimit &to, JsonObject &from);

Status from_json(td_api::getPremiumState &to, JsonObject &from);

Status from_json(td_api::getPremiumStickerExamples &to, JsonObject &from);

Status from_json(td_api::getPremiumStickers &to, JsonObject &from);

Status from_json(td_api::getProxies &to, JsonObject &from);

Status from_json(td_api::getProxyLink &to, JsonObject &from);

Status from_json(td_api::getPushReceiverId &to, JsonObject &from);

Status from_json(td_api::getRecentEmojiStatuses &to, JsonObject &from);

Status from_json(td_api::getRecentInlineBots &to, JsonObject &from);

Status from_json(td_api::getRecentStickers &to, JsonObject &from);

Status from_json(td_api::getRecentlyOpenedChats &to, JsonObject &from);

Status from_json(td_api::getRecentlyVisitedTMeUrls &to, JsonObject &from);

Status from_json(td_api::getRecommendedChatFilters &to, JsonObject &from);

Status from_json(td_api::getRecoveryEmailAddress &to, JsonObject &from);

Status from_json(td_api::getRemoteFile &to, JsonObject &from);

Status from_json(td_api::getRepliedMessage &to, JsonObject &from);

Status from_json(td_api::getSavedAnimations &to, JsonObject &from);

Status from_json(td_api::getSavedNotificationSound &to, JsonObject &from);

Status from_json(td_api::getSavedNotificationSounds &to, JsonObject &from);

Status from_json(td_api::getSavedOrderInfo &to, JsonObject &from);

Status from_json(td_api::getScopeNotificationSettings &to, JsonObject &from);

Status from_json(td_api::getSecretChat &to, JsonObject &from);

Status from_json(td_api::getStatisticalGraph &to, JsonObject &from);

Status from_json(td_api::getStickerEmojis &to, JsonObject &from);

Status from_json(td_api::getStickerSet &to, JsonObject &from);

Status from_json(td_api::getStickers &to, JsonObject &from);

Status from_json(td_api::getStorageStatistics &to, JsonObject &from);

Status from_json(td_api::getStorageStatisticsFast &to, JsonObject &from);

Status from_json(td_api::getSuggestedFileName &to, JsonObject &from);

Status from_json(td_api::getSuggestedStickerSetName &to, JsonObject &from);

Status from_json(td_api::getSuitableDiscussionChats &to, JsonObject &from);

Status from_json(td_api::getSupergroup &to, JsonObject &from);

Status from_json(td_api::getSupergroupFullInfo &to, JsonObject &from);

Status from_json(td_api::getSupergroupMembers &to, JsonObject &from);

Status from_json(td_api::getSupportUser &to, JsonObject &from);

Status from_json(td_api::getTemporaryPasswordState &to, JsonObject &from);

Status from_json(td_api::getTextEntities &to, JsonObject &from);

Status from_json(td_api::getThemeParametersJsonString &to, JsonObject &from);

Status from_json(td_api::getThemedEmojiStatuses &to, JsonObject &from);

Status from_json(td_api::getTopChats &to, JsonObject &from);

Status from_json(td_api::getTrendingStickerSets &to, JsonObject &from);

Status from_json(td_api::getUser &to, JsonObject &from);

Status from_json(td_api::getUserFullInfo &to, JsonObject &from);

Status from_json(td_api::getUserPrivacySettingRules &to, JsonObject &from);

Status from_json(td_api::getUserProfilePhotos &to, JsonObject &from);

Status from_json(td_api::getUserSupportInfo &to, JsonObject &from);

Status from_json(td_api::getVideoChatAvailableParticipants &to, JsonObject &from);

Status from_json(td_api::getVideoChatRtmpUrl &to, JsonObject &from);

Status from_json(td_api::getWebAppUrl &to, JsonObject &from);

Status from_json(td_api::getWebPageInstantView &to, JsonObject &from);

Status from_json(td_api::getWebPagePreview &to, JsonObject &from);

Status from_json(td_api::hideSuggestedAction &to, JsonObject &from);

Status from_json(td_api::importContacts &to, JsonObject &from);

Status from_json(td_api::importMessages &to, JsonObject &from);

Status from_json(td_api::inviteGroupCallParticipants &to, JsonObject &from);

Status from_json(td_api::joinChat &to, JsonObject &from);

Status from_json(td_api::joinChatByInviteLink &to, JsonObject &from);

Status from_json(td_api::joinGroupCall &to, JsonObject &from);

Status from_json(td_api::leaveChat &to, JsonObject &from);

Status from_json(td_api::leaveGroupCall &to, JsonObject &from);

Status from_json(td_api::loadChats &to, JsonObject &from);

Status from_json(td_api::loadGroupCallParticipants &to, JsonObject &from);

Status from_json(td_api::logOut &to, JsonObject &from);

Status from_json(td_api::openChat &to, JsonObject &from);

Status from_json(td_api::openMessageContent &to, JsonObject &from);

Status from_json(td_api::openWebApp &to, JsonObject &from);

Status from_json(td_api::optimizeStorage &to, JsonObject &from);

Status from_json(td_api::parseMarkdown &to, JsonObject &from);

Status from_json(td_api::parseTextEntities &to, JsonObject &from);

Status from_json(td_api::pinChatMessage &to, JsonObject &from);

Status from_json(td_api::pingProxy &to, JsonObject &from);

Status from_json(td_api::preliminaryUploadFile &to, JsonObject &from);

Status from_json(td_api::processChatJoinRequest &to, JsonObject &from);

Status from_json(td_api::processChatJoinRequests &to, JsonObject &from);

Status from_json(td_api::processPushNotification &to, JsonObject &from);

Status from_json(td_api::rateSpeechRecognition &to, JsonObject &from);

Status from_json(td_api::readAllChatMentions &to, JsonObject &from);

Status from_json(td_api::readAllChatReactions &to, JsonObject &from);

Status from_json(td_api::readAllMessageThreadMentions &to, JsonObject &from);

Status from_json(td_api::readAllMessageThreadReactions &to, JsonObject &from);

Status from_json(td_api::readFilePart &to, JsonObject &from);

Status from_json(td_api::recognizeSpeech &to, JsonObject &from);

Status from_json(td_api::recoverAuthenticationPassword &to, JsonObject &from);

Status from_json(td_api::recoverPassword &to, JsonObject &from);

Status from_json(td_api::registerDevice &to, JsonObject &from);

Status from_json(td_api::registerUser &to, JsonObject &from);

Status from_json(td_api::removeAllFilesFromDownloads &to, JsonObject &from);

Status from_json(td_api::removeBackground &to, JsonObject &from);

Status from_json(td_api::removeChatActionBar &to, JsonObject &from);

Status from_json(td_api::removeContacts &to, JsonObject &from);

Status from_json(td_api::removeFavoriteSticker &to, JsonObject &from);

Status from_json(td_api::removeFileFromDownloads &to, JsonObject &from);

Status from_json(td_api::removeMessageReaction &to, JsonObject &from);

Status from_json(td_api::removeNotification &to, JsonObject &from);

Status from_json(td_api::removeNotificationGroup &to, JsonObject &from);

Status from_json(td_api::removeProxy &to, JsonObject &from);

Status from_json(td_api::removeRecentHashtag &to, JsonObject &from);

Status from_json(td_api::removeRecentSticker &to, JsonObject &from);

Status from_json(td_api::removeRecentlyFoundChat &to, JsonObject &from);

Status from_json(td_api::removeSavedAnimation &to, JsonObject &from);

Status from_json(td_api::removeSavedNotificationSound &to, JsonObject &from);

Status from_json(td_api::removeStickerFromSet &to, JsonObject &from);

Status from_json(td_api::removeTopChat &to, JsonObject &from);

Status from_json(td_api::reorderActiveUsernames &to, JsonObject &from);

Status from_json(td_api::reorderChatFilters &to, JsonObject &from);

Status from_json(td_api::reorderInstalledStickerSets &to, JsonObject &from);

Status from_json(td_api::reorderSupergroupActiveUsernames &to, JsonObject &from);

Status from_json(td_api::replacePrimaryChatInviteLink &to, JsonObject &from);

Status from_json(td_api::replaceVideoChatRtmpUrl &to, JsonObject &from);

Status from_json(td_api::reportChat &to, JsonObject &from);

Status from_json(td_api::reportChatPhoto &to, JsonObject &from);

Status from_json(td_api::reportMessageReactions &to, JsonObject &from);

Status from_json(td_api::reportSupergroupSpam &to, JsonObject &from);

Status from_json(td_api::requestAuthenticationPasswordRecovery &to, JsonObject &from);

Status from_json(td_api::requestPasswordRecovery &to, JsonObject &from);

Status from_json(td_api::requestQrCodeAuthentication &to, JsonObject &from);

Status from_json(td_api::resendAuthenticationCode &to, JsonObject &from);

Status from_json(td_api::resendChangePhoneNumberCode &to, JsonObject &from);

Status from_json(td_api::resendEmailAddressVerificationCode &to, JsonObject &from);

Status from_json(td_api::resendLoginEmailAddressCode &to, JsonObject &from);

Status from_json(td_api::resendMessages &to, JsonObject &from);

Status from_json(td_api::resendPhoneNumberConfirmationCode &to, JsonObject &from);

Status from_json(td_api::resendPhoneNumberVerificationCode &to, JsonObject &from);

Status from_json(td_api::resendRecoveryEmailAddressCode &to, JsonObject &from);

Status from_json(td_api::resetAllNotificationSettings &to, JsonObject &from);

Status from_json(td_api::resetBackgrounds &to, JsonObject &from);

Status from_json(td_api::resetNetworkStatistics &to, JsonObject &from);

Status from_json(td_api::resetPassword &to, JsonObject &from);

Status from_json(td_api::revokeChatInviteLink &to, JsonObject &from);

Status from_json(td_api::revokeGroupCallInviteLink &to, JsonObject &from);

Status from_json(td_api::saveApplicationLogEvent &to, JsonObject &from);

Status from_json(td_api::searchBackground &to, JsonObject &from);

Status from_json(td_api::searchCallMessages &to, JsonObject &from);

Status from_json(td_api::searchChatMembers &to, JsonObject &from);

Status from_json(td_api::searchChatMessages &to, JsonObject &from);

Status from_json(td_api::searchChatRecentLocationMessages &to, JsonObject &from);

Status from_json(td_api::searchChats &to, JsonObject &from);

Status from_json(td_api::searchChatsNearby &to, JsonObject &from);

Status from_json(td_api::searchChatsOnServer &to, JsonObject &from);

Status from_json(td_api::searchContacts &to, JsonObject &from);

Status from_json(td_api::searchEmojis &to, JsonObject &from);

Status from_json(td_api::searchFileDownloads &to, JsonObject &from);

Status from_json(td_api::searchHashtags &to, JsonObject &from);

Status from_json(td_api::searchInstalledStickerSets &to, JsonObject &from);

Status from_json(td_api::searchMessages &to, JsonObject &from);

Status from_json(td_api::searchOutgoingDocumentMessages &to, JsonObject &from);

Status from_json(td_api::searchPublicChat &to, JsonObject &from);

Status from_json(td_api::searchPublicChats &to, JsonObject &from);

Status from_json(td_api::searchSecretMessages &to, JsonObject &from);

Status from_json(td_api::searchStickerSet &to, JsonObject &from);

Status from_json(td_api::searchStickerSets &to, JsonObject &from);

Status from_json(td_api::searchStickers &to, JsonObject &from);

Status from_json(td_api::searchUserByPhoneNumber &to, JsonObject &from);

Status from_json(td_api::sendBotStartMessage &to, JsonObject &from);

Status from_json(td_api::sendCallDebugInformation &to, JsonObject &from);

Status from_json(td_api::sendCallLog &to, JsonObject &from);

Status from_json(td_api::sendCallRating &to, JsonObject &from);

Status from_json(td_api::sendCallSignalingData &to, JsonObject &from);

Status from_json(td_api::sendChatAction &to, JsonObject &from);

Status from_json(td_api::sendChatScreenshotTakenNotification &to, JsonObject &from);

Status from_json(td_api::sendCustomRequest &to, JsonObject &from);

Status from_json(td_api::sendEmailAddressVerificationCode &to, JsonObject &from);

Status from_json(td_api::sendInlineQueryResultMessage &to, JsonObject &from);

Status from_json(td_api::sendMessage &to, JsonObject &from);

Status from_json(td_api::sendMessageAlbum &to, JsonObject &from);

Status from_json(td_api::sendPassportAuthorizationForm &to, JsonObject &from);

Status from_json(td_api::sendPaymentForm &to, JsonObject &from);

Status from_json(td_api::sendPhoneNumberConfirmationCode &to, JsonObject &from);

Status from_json(td_api::sendPhoneNumberVerificationCode &to, JsonObject &from);

Status from_json(td_api::sendWebAppData &to, JsonObject &from);

Status from_json(td_api::setAccountTtl &to, JsonObject &from);

Status from_json(td_api::setAlarm &to, JsonObject &from);

Status from_json(td_api::setAuthenticationEmailAddress &to, JsonObject &from);

Status from_json(td_api::setAuthenticationPhoneNumber &to, JsonObject &from);

Status from_json(td_api::setAutoDownloadSettings &to, JsonObject &from);

Status from_json(td_api::setBackground &to, JsonObject &from);

Status from_json(td_api::setBio &to, JsonObject &from);

Status from_json(td_api::setBotUpdatesStatus &to, JsonObject &from);

Status from_json(td_api::setChatAvailableReactions &to, JsonObject &from);

Status from_json(td_api::setChatClientData &to, JsonObject &from);

Status from_json(td_api::setChatDescription &to, JsonObject &from);

Status from_json(td_api::setChatDiscussionGroup &to, JsonObject &from);

Status from_json(td_api::setChatDraftMessage &to, JsonObject &from);

Status from_json(td_api::setChatLocation &to, JsonObject &from);

Status from_json(td_api::setChatMemberStatus &to, JsonObject &from);

Status from_json(td_api::setChatMessageSender &to, JsonObject &from);

Status from_json(td_api::setChatMessageTtl &to, JsonObject &from);

Status from_json(td_api::setChatNotificationSettings &to, JsonObject &from);

Status from_json(td_api::setChatPermissions &to, JsonObject &from);

Status from_json(td_api::setChatPhoto &to, JsonObject &from);

Status from_json(td_api::setChatSlowModeDelay &to, JsonObject &from);

Status from_json(td_api::setChatTheme &to, JsonObject &from);

Status from_json(td_api::setChatTitle &to, JsonObject &from);

Status from_json(td_api::setCommands &to, JsonObject &from);

Status from_json(td_api::setCustomLanguagePack &to, JsonObject &from);

Status from_json(td_api::setCustomLanguagePackString &to, JsonObject &from);

Status from_json(td_api::setDatabaseEncryptionKey &to, JsonObject &from);

Status from_json(td_api::setDefaultChannelAdministratorRights &to, JsonObject &from);

Status from_json(td_api::setDefaultGroupAdministratorRights &to, JsonObject &from);

Status from_json(td_api::setDefaultReactionType &to, JsonObject &from);

Status from_json(td_api::setEmojiStatus &to, JsonObject &from);

Status from_json(td_api::setFileGenerationProgress &to, JsonObject &from);

Status from_json(td_api::setGameScore &to, JsonObject &from);

Status from_json(td_api::setGroupCallParticipantIsSpeaking &to, JsonObject &from);

Status from_json(td_api::setGroupCallParticipantVolumeLevel &to, JsonObject &from);

Status from_json(td_api::setGroupCallTitle &to, JsonObject &from);

Status from_json(td_api::setInactiveSessionTtl &to, JsonObject &from);

Status from_json(td_api::setInlineGameScore &to, JsonObject &from);

Status from_json(td_api::setLocation &to, JsonObject &from);

Status from_json(td_api::setLogStream &to, JsonObject &from);

Status from_json(td_api::setLogTagVerbosityLevel &to, JsonObject &from);

Status from_json(td_api::setLogVerbosityLevel &to, JsonObject &from);

Status from_json(td_api::setLoginEmailAddress &to, JsonObject &from);

Status from_json(td_api::setMenuButton &to, JsonObject &from);

Status from_json(td_api::setName &to, JsonObject &from);

Status from_json(td_api::setNetworkType &to, JsonObject &from);

Status from_json(td_api::setOption &to, JsonObject &from);

Status from_json(td_api::setPassportElement &to, JsonObject &from);

Status from_json(td_api::setPassportElementErrors &to, JsonObject &from);

Status from_json(td_api::setPassword &to, JsonObject &from);

Status from_json(td_api::setPinnedChats &to, JsonObject &from);

Status from_json(td_api::setPollAnswer &to, JsonObject &from);

Status from_json(td_api::setProfilePhoto &to, JsonObject &from);

Status from_json(td_api::setRecoveryEmailAddress &to, JsonObject &from);

Status from_json(td_api::setScopeNotificationSettings &to, JsonObject &from);

Status from_json(td_api::setStickerPositionInSet &to, JsonObject &from);

Status from_json(td_api::setStickerSetThumbnail &to, JsonObject &from);

Status from_json(td_api::setSupergroupStickerSet &to, JsonObject &from);

Status from_json(td_api::setSupergroupUsername &to, JsonObject &from);

Status from_json(td_api::setTdlibParameters &to, JsonObject &from);

Status from_json(td_api::setUserPrivacySettingRules &to, JsonObject &from);

Status from_json(td_api::setUserSupportInfo &to, JsonObject &from);

Status from_json(td_api::setUsername &to, JsonObject &from);

Status from_json(td_api::setVideoChatDefaultParticipant &to, JsonObject &from);

Status from_json(td_api::sharePhoneNumber &to, JsonObject &from);

Status from_json(td_api::startGroupCallRecording &to, JsonObject &from);

Status from_json(td_api::startGroupCallScreenSharing &to, JsonObject &from);

Status from_json(td_api::startScheduledGroupCall &to, JsonObject &from);

Status from_json(td_api::stopPoll &to, JsonObject &from);

Status from_json(td_api::synchronizeLanguagePack &to, JsonObject &from);

Status from_json(td_api::terminateAllOtherSessions &to, JsonObject &from);

Status from_json(td_api::terminateSession &to, JsonObject &from);

Status from_json(td_api::testCallBytes &to, JsonObject &from);

Status from_json(td_api::testCallEmpty &to, JsonObject &from);

Status from_json(td_api::testCallString &to, JsonObject &from);

Status from_json(td_api::testCallVectorInt &to, JsonObject &from);

Status from_json(td_api::testCallVectorIntObject &to, JsonObject &from);

Status from_json(td_api::testCallVectorString &to, JsonObject &from);

Status from_json(td_api::testCallVectorStringObject &to, JsonObject &from);

Status from_json(td_api::testGetDifference &to, JsonObject &from);

Status from_json(td_api::testNetwork &to, JsonObject &from);

Status from_json(td_api::testProxy &to, JsonObject &from);

Status from_json(td_api::testReturnError &to, JsonObject &from);

Status from_json(td_api::testSquareInt &to, JsonObject &from);

Status from_json(td_api::testUseUpdate &to, JsonObject &from);

Status from_json(td_api::toggleAllDownloadsArePaused &to, JsonObject &from);

Status from_json(td_api::toggleBotIsAddedToAttachmentMenu &to, JsonObject &from);

Status from_json(td_api::toggleChatDefaultDisableNotification &to, JsonObject &from);

Status from_json(td_api::toggleChatHasProtectedContent &to, JsonObject &from);

Status from_json(td_api::toggleChatIsMarkedAsUnread &to, JsonObject &from);

Status from_json(td_api::toggleChatIsPinned &to, JsonObject &from);

Status from_json(td_api::toggleDownloadIsPaused &to, JsonObject &from);

Status from_json(td_api::toggleForumTopicIsClosed &to, JsonObject &from);

Status from_json(td_api::toggleGroupCallEnabledStartNotification &to, JsonObject &from);

Status from_json(td_api::toggleGroupCallIsMyVideoEnabled &to, JsonObject &from);

Status from_json(td_api::toggleGroupCallIsMyVideoPaused &to, JsonObject &from);

Status from_json(td_api::toggleGroupCallMuteNewParticipants &to, JsonObject &from);

Status from_json(td_api::toggleGroupCallParticipantIsHandRaised &to, JsonObject &from);

Status from_json(td_api::toggleGroupCallParticipantIsMuted &to, JsonObject &from);

Status from_json(td_api::toggleGroupCallScreenSharingIsPaused &to, JsonObject &from);

Status from_json(td_api::toggleMessageSenderIsBlocked &to, JsonObject &from);

Status from_json(td_api::toggleSessionCanAcceptCalls &to, JsonObject &from);

Status from_json(td_api::toggleSessionCanAcceptSecretChats &to, JsonObject &from);

Status from_json(td_api::toggleSupergroupIsAllHistoryAvailable &to, JsonObject &from);

Status from_json(td_api::toggleSupergroupIsBroadcastGroup &to, JsonObject &from);

Status from_json(td_api::toggleSupergroupIsForum &to, JsonObject &from);

Status from_json(td_api::toggleSupergroupJoinByRequest &to, JsonObject &from);

Status from_json(td_api::toggleSupergroupJoinToSendMessages &to, JsonObject &from);

Status from_json(td_api::toggleSupergroupSignMessages &to, JsonObject &from);

Status from_json(td_api::toggleSupergroupUsernameIsActive &to, JsonObject &from);

Status from_json(td_api::toggleUsernameIsActive &to, JsonObject &from);

Status from_json(td_api::transferChatOwnership &to, JsonObject &from);

Status from_json(td_api::translateText &to, JsonObject &from);

Status from_json(td_api::unpinAllChatMessages &to, JsonObject &from);

Status from_json(td_api::unpinAllMessageThreadMessages &to, JsonObject &from);

Status from_json(td_api::unpinChatMessage &to, JsonObject &from);

Status from_json(td_api::upgradeBasicGroupChatToSupergroupChat &to, JsonObject &from);

Status from_json(td_api::uploadStickerFile &to, JsonObject &from);

Status from_json(td_api::validateOrderInfo &to, JsonObject &from);

Status from_json(td_api::viewMessages &to, JsonObject &from);

Status from_json(td_api::viewPremiumFeature &to, JsonObject &from);

Status from_json(td_api::viewTrendingStickerSets &to, JsonObject &from);

Status from_json(td_api::writeGeneratedFilePart &to, JsonObject &from);

void to_json(JsonValueScope &jv, const td_api::accountTtl &object);

void to_json(JsonValueScope &jv, const td_api::addedReaction &object);

void to_json(JsonValueScope &jv, const td_api::addedReactions &object);

void to_json(JsonValueScope &jv, const td_api::address &object);

void to_json(JsonValueScope &jv, const td_api::animatedChatPhoto &object);

void to_json(JsonValueScope &jv, const td_api::animatedEmoji &object);

void to_json(JsonValueScope &jv, const td_api::animation &object);

void to_json(JsonValueScope &jv, const td_api::animations &object);

void to_json(JsonValueScope &jv, const td_api::attachmentMenuBot &object);

void to_json(JsonValueScope &jv, const td_api::attachmentMenuBotColor &object);

void to_json(JsonValueScope &jv, const td_api::audio &object);

void to_json(JsonValueScope &jv, const td_api::authenticationCodeInfo &object);

void to_json(JsonValueScope &jv, const td_api::AuthenticationCodeType &object);

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeTelegramMessage &object);

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeSms &object);

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeCall &object);

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeFlashCall &object);

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeMissedCall &object);

void to_json(JsonValueScope &jv, const td_api::AuthorizationState &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitTdlibParameters &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitEmailAddress &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitEmailCode &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitCode &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitOtherDeviceConfirmation &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitRegistration &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitPassword &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateReady &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateLoggingOut &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateClosing &object);

void to_json(JsonValueScope &jv, const td_api::authorizationStateClosed &object);

void to_json(JsonValueScope &jv, const td_api::autoDownloadSettings &object);

void to_json(JsonValueScope &jv, const td_api::autoDownloadSettingsPresets &object);

void to_json(JsonValueScope &jv, const td_api::availableReaction &object);

void to_json(JsonValueScope &jv, const td_api::availableReactions &object);

void to_json(JsonValueScope &jv, const td_api::background &object);

void to_json(JsonValueScope &jv, const td_api::BackgroundFill &object);

void to_json(JsonValueScope &jv, const td_api::backgroundFillSolid &object);

void to_json(JsonValueScope &jv, const td_api::backgroundFillGradient &object);

void to_json(JsonValueScope &jv, const td_api::backgroundFillFreeformGradient &object);

void to_json(JsonValueScope &jv, const td_api::BackgroundType &object);

void to_json(JsonValueScope &jv, const td_api::backgroundTypeWallpaper &object);

void to_json(JsonValueScope &jv, const td_api::backgroundTypePattern &object);

void to_json(JsonValueScope &jv, const td_api::backgroundTypeFill &object);

void to_json(JsonValueScope &jv, const td_api::backgrounds &object);

void to_json(JsonValueScope &jv, const td_api::bankCardActionOpenUrl &object);

void to_json(JsonValueScope &jv, const td_api::bankCardInfo &object);

void to_json(JsonValueScope &jv, const td_api::basicGroup &object);

void to_json(JsonValueScope &jv, const td_api::basicGroupFullInfo &object);

void to_json(JsonValueScope &jv, const td_api::botCommand &object);

void to_json(JsonValueScope &jv, const td_api::botCommands &object);

void to_json(JsonValueScope &jv, const td_api::botInfo &object);

void to_json(JsonValueScope &jv, const td_api::botMenuButton &object);

void to_json(JsonValueScope &jv, const td_api::call &object);

void to_json(JsonValueScope &jv, const td_api::CallDiscardReason &object);

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonEmpty &object);

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonMissed &object);

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonDeclined &object);

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonDisconnected &object);

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonHungUp &object);

void to_json(JsonValueScope &jv, const td_api::callId &object);

void to_json(JsonValueScope &jv, const td_api::callProtocol &object);

void to_json(JsonValueScope &jv, const td_api::callServer &object);

void to_json(JsonValueScope &jv, const td_api::CallServerType &object);

void to_json(JsonValueScope &jv, const td_api::callServerTypeTelegramReflector &object);

void to_json(JsonValueScope &jv, const td_api::callServerTypeWebrtc &object);

void to_json(JsonValueScope &jv, const td_api::CallState &object);

void to_json(JsonValueScope &jv, const td_api::callStatePending &object);

void to_json(JsonValueScope &jv, const td_api::callStateExchangingKeys &object);

void to_json(JsonValueScope &jv, const td_api::callStateReady &object);

void to_json(JsonValueScope &jv, const td_api::callStateHangingUp &object);

void to_json(JsonValueScope &jv, const td_api::callStateDiscarded &object);

void to_json(JsonValueScope &jv, const td_api::callStateError &object);

void to_json(JsonValueScope &jv, const td_api::callbackQueryAnswer &object);

void to_json(JsonValueScope &jv, const td_api::CallbackQueryPayload &object);

void to_json(JsonValueScope &jv, const td_api::callbackQueryPayloadData &object);

void to_json(JsonValueScope &jv, const td_api::callbackQueryPayloadDataWithPassword &object);

void to_json(JsonValueScope &jv, const td_api::callbackQueryPayloadGame &object);

void to_json(JsonValueScope &jv, const td_api::CanTransferOwnershipResult &object);

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultOk &object);

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultPasswordNeeded &object);

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultPasswordTooFresh &object);

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultSessionTooFresh &object);

void to_json(JsonValueScope &jv, const td_api::chat &object);

void to_json(JsonValueScope &jv, const td_api::ChatAction &object);

void to_json(JsonValueScope &jv, const td_api::chatActionTyping &object);

void to_json(JsonValueScope &jv, const td_api::chatActionRecordingVideo &object);

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingVideo &object);

void to_json(JsonValueScope &jv, const td_api::chatActionRecordingVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingPhoto &object);

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingDocument &object);

void to_json(JsonValueScope &jv, const td_api::chatActionChoosingSticker &object);

void to_json(JsonValueScope &jv, const td_api::chatActionChoosingLocation &object);

void to_json(JsonValueScope &jv, const td_api::chatActionChoosingContact &object);

void to_json(JsonValueScope &jv, const td_api::chatActionStartPlayingGame &object);

void to_json(JsonValueScope &jv, const td_api::chatActionRecordingVideoNote &object);

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingVideoNote &object);

void to_json(JsonValueScope &jv, const td_api::chatActionWatchingAnimations &object);

void to_json(JsonValueScope &jv, const td_api::chatActionCancel &object);

void to_json(JsonValueScope &jv, const td_api::ChatActionBar &object);

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportSpam &object);

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportUnrelatedLocation &object);

void to_json(JsonValueScope &jv, const td_api::chatActionBarInviteMembers &object);

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportAddBlock &object);

void to_json(JsonValueScope &jv, const td_api::chatActionBarAddContact &object);

void to_json(JsonValueScope &jv, const td_api::chatActionBarSharePhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::chatActionBarJoinRequest &object);

void to_json(JsonValueScope &jv, const td_api::chatAdministrator &object);

void to_json(JsonValueScope &jv, const td_api::chatAdministratorRights &object);

void to_json(JsonValueScope &jv, const td_api::chatAdministrators &object);

void to_json(JsonValueScope &jv, const td_api::ChatAvailableReactions &object);

void to_json(JsonValueScope &jv, const td_api::chatAvailableReactionsAll &object);

void to_json(JsonValueScope &jv, const td_api::chatAvailableReactionsSome &object);

void to_json(JsonValueScope &jv, const td_api::chatEvent &object);

void to_json(JsonValueScope &jv, const td_api::ChatEventAction &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMessageEdited &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMessageDeleted &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMessagePinned &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMessageUnpinned &object);

void to_json(JsonValueScope &jv, const td_api::chatEventPollStopped &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMemberJoined &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMemberJoinedByInviteLink &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMemberJoinedByRequest &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMemberInvited &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMemberLeft &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMemberPromoted &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMemberRestricted &object);

void to_json(JsonValueScope &jv, const td_api::chatEventAvailableReactionsChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventDescriptionChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventLinkedChatChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventLocationChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventMessageTtlChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventPermissionsChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventPhotoChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventSlowModeDelayChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventStickerSetChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventTitleChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventUsernameChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventActiveUsernamesChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventHasProtectedContentToggled &object);

void to_json(JsonValueScope &jv, const td_api::chatEventInvitesToggled &object);

void to_json(JsonValueScope &jv, const td_api::chatEventIsAllHistoryAvailableToggled &object);

void to_json(JsonValueScope &jv, const td_api::chatEventSignMessagesToggled &object);

void to_json(JsonValueScope &jv, const td_api::chatEventInviteLinkEdited &object);

void to_json(JsonValueScope &jv, const td_api::chatEventInviteLinkRevoked &object);

void to_json(JsonValueScope &jv, const td_api::chatEventInviteLinkDeleted &object);

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatCreated &object);

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatEnded &object);

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatMuteNewParticipantsToggled &object);

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatParticipantIsMutedToggled &object);

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatParticipantVolumeLevelChanged &object);

void to_json(JsonValueScope &jv, const td_api::chatEventIsForumToggled &object);

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicCreated &object);

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicEdited &object);

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicToggleIsClosed &object);

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicDeleted &object);

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicPinned &object);

void to_json(JsonValueScope &jv, const td_api::chatEvents &object);

void to_json(JsonValueScope &jv, const td_api::chatFilter &object);

void to_json(JsonValueScope &jv, const td_api::chatFilterInfo &object);

void to_json(JsonValueScope &jv, const td_api::chatInviteLink &object);

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkCount &object);

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkCounts &object);

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkInfo &object);

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkMember &object);

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkMembers &object);

void to_json(JsonValueScope &jv, const td_api::chatInviteLinks &object);

void to_json(JsonValueScope &jv, const td_api::chatJoinRequest &object);

void to_json(JsonValueScope &jv, const td_api::chatJoinRequests &object);

void to_json(JsonValueScope &jv, const td_api::chatJoinRequestsInfo &object);

void to_json(JsonValueScope &jv, const td_api::ChatList &object);

void to_json(JsonValueScope &jv, const td_api::chatListMain &object);

void to_json(JsonValueScope &jv, const td_api::chatListArchive &object);

void to_json(JsonValueScope &jv, const td_api::chatListFilter &object);

void to_json(JsonValueScope &jv, const td_api::chatLists &object);

void to_json(JsonValueScope &jv, const td_api::chatLocation &object);

void to_json(JsonValueScope &jv, const td_api::chatMember &object);

void to_json(JsonValueScope &jv, const td_api::ChatMemberStatus &object);

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusCreator &object);

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusAdministrator &object);

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusMember &object);

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusRestricted &object);

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusLeft &object);

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusBanned &object);

void to_json(JsonValueScope &jv, const td_api::chatMembers &object);

void to_json(JsonValueScope &jv, const td_api::chatMessageSender &object);

void to_json(JsonValueScope &jv, const td_api::chatMessageSenders &object);

void to_json(JsonValueScope &jv, const td_api::chatNearby &object);

void to_json(JsonValueScope &jv, const td_api::chatNotificationSettings &object);

void to_json(JsonValueScope &jv, const td_api::chatPermissions &object);

void to_json(JsonValueScope &jv, const td_api::chatPhoto &object);

void to_json(JsonValueScope &jv, const td_api::chatPhotoInfo &object);

void to_json(JsonValueScope &jv, const td_api::chatPhotos &object);

void to_json(JsonValueScope &jv, const td_api::chatPosition &object);

void to_json(JsonValueScope &jv, const td_api::ChatSource &object);

void to_json(JsonValueScope &jv, const td_api::chatSourceMtprotoProxy &object);

void to_json(JsonValueScope &jv, const td_api::chatSourcePublicServiceAnnouncement &object);

void to_json(JsonValueScope &jv, const td_api::ChatStatistics &object);

void to_json(JsonValueScope &jv, const td_api::chatStatisticsSupergroup &object);

void to_json(JsonValueScope &jv, const td_api::chatStatisticsChannel &object);

void to_json(JsonValueScope &jv, const td_api::chatStatisticsAdministratorActionsInfo &object);

void to_json(JsonValueScope &jv, const td_api::chatStatisticsInviterInfo &object);

void to_json(JsonValueScope &jv, const td_api::chatStatisticsMessageInteractionInfo &object);

void to_json(JsonValueScope &jv, const td_api::chatStatisticsMessageSenderInfo &object);

void to_json(JsonValueScope &jv, const td_api::chatTheme &object);

void to_json(JsonValueScope &jv, const td_api::ChatType &object);

void to_json(JsonValueScope &jv, const td_api::chatTypePrivate &object);

void to_json(JsonValueScope &jv, const td_api::chatTypeBasicGroup &object);

void to_json(JsonValueScope &jv, const td_api::chatTypeSupergroup &object);

void to_json(JsonValueScope &jv, const td_api::chatTypeSecret &object);

void to_json(JsonValueScope &jv, const td_api::chats &object);

void to_json(JsonValueScope &jv, const td_api::chatsNearby &object);

void to_json(JsonValueScope &jv, const td_api::CheckChatUsernameResult &object);

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultOk &object);

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultUsernameInvalid &object);

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultUsernameOccupied &object);

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultPublicChatsTooMuch &object);

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultPublicGroupsUnavailable &object);

void to_json(JsonValueScope &jv, const td_api::CheckStickerSetNameResult &object);

void to_json(JsonValueScope &jv, const td_api::checkStickerSetNameResultOk &object);

void to_json(JsonValueScope &jv, const td_api::checkStickerSetNameResultNameInvalid &object);

void to_json(JsonValueScope &jv, const td_api::checkStickerSetNameResultNameOccupied &object);

void to_json(JsonValueScope &jv, const td_api::closedVectorPath &object);

void to_json(JsonValueScope &jv, const td_api::connectedWebsite &object);

void to_json(JsonValueScope &jv, const td_api::connectedWebsites &object);

void to_json(JsonValueScope &jv, const td_api::ConnectionState &object);

void to_json(JsonValueScope &jv, const td_api::connectionStateWaitingForNetwork &object);

void to_json(JsonValueScope &jv, const td_api::connectionStateConnectingToProxy &object);

void to_json(JsonValueScope &jv, const td_api::connectionStateConnecting &object);

void to_json(JsonValueScope &jv, const td_api::connectionStateUpdating &object);

void to_json(JsonValueScope &jv, const td_api::connectionStateReady &object);

void to_json(JsonValueScope &jv, const td_api::contact &object);

void to_json(JsonValueScope &jv, const td_api::count &object);

void to_json(JsonValueScope &jv, const td_api::countries &object);

void to_json(JsonValueScope &jv, const td_api::countryInfo &object);

void to_json(JsonValueScope &jv, const td_api::customRequestResult &object);

void to_json(JsonValueScope &jv, const td_api::databaseStatistics &object);

void to_json(JsonValueScope &jv, const td_api::date &object);

void to_json(JsonValueScope &jv, const td_api::dateRange &object);

void to_json(JsonValueScope &jv, const td_api::datedFile &object);

void to_json(JsonValueScope &jv, const td_api::deepLinkInfo &object);

void to_json(JsonValueScope &jv, const td_api::DiceStickers &object);

void to_json(JsonValueScope &jv, const td_api::diceStickersRegular &object);

void to_json(JsonValueScope &jv, const td_api::diceStickersSlotMachine &object);

void to_json(JsonValueScope &jv, const td_api::document &object);

void to_json(JsonValueScope &jv, const td_api::downloadedFileCounts &object);

void to_json(JsonValueScope &jv, const td_api::draftMessage &object);

void to_json(JsonValueScope &jv, const td_api::emailAddressAuthenticationCodeInfo &object);

void to_json(JsonValueScope &jv, const td_api::emojiReaction &object);

void to_json(JsonValueScope &jv, const td_api::emojiStatus &object);

void to_json(JsonValueScope &jv, const td_api::emojiStatuses &object);

void to_json(JsonValueScope &jv, const td_api::emojis &object);

void to_json(JsonValueScope &jv, const td_api::encryptedCredentials &object);

void to_json(JsonValueScope &jv, const td_api::encryptedPassportElement &object);

void to_json(JsonValueScope &jv, const td_api::error &object);

void to_json(JsonValueScope &jv, const td_api::file &object);

void to_json(JsonValueScope &jv, const td_api::fileDownload &object);

void to_json(JsonValueScope &jv, const td_api::fileDownloadedPrefixSize &object);

void to_json(JsonValueScope &jv, const td_api::filePart &object);

void to_json(JsonValueScope &jv, const td_api::FileType &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeNone &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeAnimation &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeAudio &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeDocument &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeNotificationSound &object);

void to_json(JsonValueScope &jv, const td_api::fileTypePhoto &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeProfilePhoto &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeSecret &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeSecretThumbnail &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeSecure &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeSticker &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeThumbnail &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeUnknown &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeVideo &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeVideoNote &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::fileTypeWallpaper &object);

void to_json(JsonValueScope &jv, const td_api::formattedText &object);

void to_json(JsonValueScope &jv, const td_api::forumTopicIcon &object);

void to_json(JsonValueScope &jv, const td_api::forumTopicInfo &object);

void to_json(JsonValueScope &jv, const td_api::foundFileDownloads &object);

void to_json(JsonValueScope &jv, const td_api::foundMessages &object);

void to_json(JsonValueScope &jv, const td_api::game &object);

void to_json(JsonValueScope &jv, const td_api::gameHighScore &object);

void to_json(JsonValueScope &jv, const td_api::gameHighScores &object);

void to_json(JsonValueScope &jv, const td_api::groupCall &object);

void to_json(JsonValueScope &jv, const td_api::groupCallId &object);

void to_json(JsonValueScope &jv, const td_api::groupCallParticipant &object);

void to_json(JsonValueScope &jv, const td_api::groupCallParticipantVideoInfo &object);

void to_json(JsonValueScope &jv, const td_api::groupCallRecentSpeaker &object);

void to_json(JsonValueScope &jv, const td_api::groupCallStream &object);

void to_json(JsonValueScope &jv, const td_api::groupCallStreams &object);

void to_json(JsonValueScope &jv, const td_api::groupCallVideoSourceGroup &object);

void to_json(JsonValueScope &jv, const td_api::hashtags &object);

void to_json(JsonValueScope &jv, const td_api::httpUrl &object);

void to_json(JsonValueScope &jv, const td_api::identityDocument &object);

void to_json(JsonValueScope &jv, const td_api::importedContacts &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButton &object);

void to_json(JsonValueScope &jv, const td_api::InlineKeyboardButtonType &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeUrl &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeLoginUrl &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeWebApp &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeCallback &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeCallbackWithPassword &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeCallbackGame &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeSwitchInline &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeBuy &object);

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeUser &object);

void to_json(JsonValueScope &jv, const td_api::InlineQueryResult &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultArticle &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultContact &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultLocation &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultVenue &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultGame &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultAnimation &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultAudio &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultDocument &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultPhoto &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultSticker &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultVideo &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::inlineQueryResults &object);

void to_json(JsonValueScope &jv, const td_api::InputFile &object);

void to_json(JsonValueScope &jv, const td_api::inputFileId &object);

void to_json(JsonValueScope &jv, const td_api::inputFileRemote &object);

void to_json(JsonValueScope &jv, const td_api::inputFileLocal &object);

void to_json(JsonValueScope &jv, const td_api::inputFileGenerated &object);

void to_json(JsonValueScope &jv, const td_api::InputMessageContent &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageText &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageAnimation &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageAudio &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageDocument &object);

void to_json(JsonValueScope &jv, const td_api::inputMessagePhoto &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageSticker &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageVideo &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageVideoNote &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageLocation &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageVenue &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageContact &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageDice &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageGame &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageInvoice &object);

void to_json(JsonValueScope &jv, const td_api::inputMessagePoll &object);

void to_json(JsonValueScope &jv, const td_api::inputMessageForwarded &object);

void to_json(JsonValueScope &jv, const td_api::inputThumbnail &object);

void to_json(JsonValueScope &jv, const td_api::InternalLinkType &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeActiveSessions &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeAttachmentMenuBot &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeAuthenticationCode &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBackground &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBotStart &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBotStartInGroup &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBotAddToChannel &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChangePhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChatInvite &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeFilterSettings &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeGame &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeInstantView &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeInvoice &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeLanguagePack &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeLanguageSettings &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeMessage &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeMessageDraft &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePassportDataRequest &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePhoneNumberConfirmation &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePremiumFeatures &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePrivacyAndSecuritySettings &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeProxy &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePublicChat &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeQrCodeAuthentication &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeRestorePurchases &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeSettings &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeStickerSet &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeTheme &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeThemeSettings &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUnknownDeepLink &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUnsupportedProxy &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUserPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeVideoChat &object);

void to_json(JsonValueScope &jv, const td_api::invoice &object);

void to_json(JsonValueScope &jv, const td_api::jsonObjectMember &object);

void to_json(JsonValueScope &jv, const td_api::JsonValue &object);

void to_json(JsonValueScope &jv, const td_api::jsonValueNull &object);

void to_json(JsonValueScope &jv, const td_api::jsonValueBoolean &object);

void to_json(JsonValueScope &jv, const td_api::jsonValueNumber &object);

void to_json(JsonValueScope &jv, const td_api::jsonValueString &object);

void to_json(JsonValueScope &jv, const td_api::jsonValueArray &object);

void to_json(JsonValueScope &jv, const td_api::jsonValueObject &object);

void to_json(JsonValueScope &jv, const td_api::keyboardButton &object);

void to_json(JsonValueScope &jv, const td_api::KeyboardButtonType &object);

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeText &object);

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestLocation &object);

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestPoll &object);

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeWebApp &object);

void to_json(JsonValueScope &jv, const td_api::labeledPricePart &object);

void to_json(JsonValueScope &jv, const td_api::languagePackInfo &object);

void to_json(JsonValueScope &jv, const td_api::languagePackString &object);

void to_json(JsonValueScope &jv, const td_api::LanguagePackStringValue &object);

void to_json(JsonValueScope &jv, const td_api::languagePackStringValueOrdinary &object);

void to_json(JsonValueScope &jv, const td_api::languagePackStringValuePluralized &object);

void to_json(JsonValueScope &jv, const td_api::languagePackStringValueDeleted &object);

void to_json(JsonValueScope &jv, const td_api::languagePackStrings &object);

void to_json(JsonValueScope &jv, const td_api::localFile &object);

void to_json(JsonValueScope &jv, const td_api::localizationTargetInfo &object);

void to_json(JsonValueScope &jv, const td_api::location &object);

void to_json(JsonValueScope &jv, const td_api::LogStream &object);

void to_json(JsonValueScope &jv, const td_api::logStreamDefault &object);

void to_json(JsonValueScope &jv, const td_api::logStreamFile &object);

void to_json(JsonValueScope &jv, const td_api::logStreamEmpty &object);

void to_json(JsonValueScope &jv, const td_api::logTags &object);

void to_json(JsonValueScope &jv, const td_api::logVerbosityLevel &object);

void to_json(JsonValueScope &jv, const td_api::LoginUrlInfo &object);

void to_json(JsonValueScope &jv, const td_api::loginUrlInfoOpen &object);

void to_json(JsonValueScope &jv, const td_api::loginUrlInfoRequestConfirmation &object);

void to_json(JsonValueScope &jv, const td_api::MaskPoint &object);

void to_json(JsonValueScope &jv, const td_api::maskPointForehead &object);

void to_json(JsonValueScope &jv, const td_api::maskPointEyes &object);

void to_json(JsonValueScope &jv, const td_api::maskPointMouth &object);

void to_json(JsonValueScope &jv, const td_api::maskPointChin &object);

void to_json(JsonValueScope &jv, const td_api::maskPosition &object);

void to_json(JsonValueScope &jv, const td_api::message &object);

void to_json(JsonValueScope &jv, const td_api::messageCalendar &object);

void to_json(JsonValueScope &jv, const td_api::messageCalendarDay &object);

void to_json(JsonValueScope &jv, const td_api::MessageContent &object);

void to_json(JsonValueScope &jv, const td_api::messageText &object);

void to_json(JsonValueScope &jv, const td_api::messageAnimation &object);

void to_json(JsonValueScope &jv, const td_api::messageAudio &object);

void to_json(JsonValueScope &jv, const td_api::messageDocument &object);

void to_json(JsonValueScope &jv, const td_api::messagePhoto &object);

void to_json(JsonValueScope &jv, const td_api::messageExpiredPhoto &object);

void to_json(JsonValueScope &jv, const td_api::messageSticker &object);

void to_json(JsonValueScope &jv, const td_api::messageVideo &object);

void to_json(JsonValueScope &jv, const td_api::messageExpiredVideo &object);

void to_json(JsonValueScope &jv, const td_api::messageVideoNote &object);

void to_json(JsonValueScope &jv, const td_api::messageVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::messageLocation &object);

void to_json(JsonValueScope &jv, const td_api::messageVenue &object);

void to_json(JsonValueScope &jv, const td_api::messageContact &object);

void to_json(JsonValueScope &jv, const td_api::messageAnimatedEmoji &object);

void to_json(JsonValueScope &jv, const td_api::messageDice &object);

void to_json(JsonValueScope &jv, const td_api::messageGame &object);

void to_json(JsonValueScope &jv, const td_api::messagePoll &object);

void to_json(JsonValueScope &jv, const td_api::messageInvoice &object);

void to_json(JsonValueScope &jv, const td_api::messageCall &object);

void to_json(JsonValueScope &jv, const td_api::messageVideoChatScheduled &object);

void to_json(JsonValueScope &jv, const td_api::messageVideoChatStarted &object);

void to_json(JsonValueScope &jv, const td_api::messageVideoChatEnded &object);

void to_json(JsonValueScope &jv, const td_api::messageInviteVideoChatParticipants &object);

void to_json(JsonValueScope &jv, const td_api::messageBasicGroupChatCreate &object);

void to_json(JsonValueScope &jv, const td_api::messageSupergroupChatCreate &object);

void to_json(JsonValueScope &jv, const td_api::messageChatChangeTitle &object);

void to_json(JsonValueScope &jv, const td_api::messageChatChangePhoto &object);

void to_json(JsonValueScope &jv, const td_api::messageChatDeletePhoto &object);

void to_json(JsonValueScope &jv, const td_api::messageChatAddMembers &object);

void to_json(JsonValueScope &jv, const td_api::messageChatJoinByLink &object);

void to_json(JsonValueScope &jv, const td_api::messageChatJoinByRequest &object);

void to_json(JsonValueScope &jv, const td_api::messageChatDeleteMember &object);

void to_json(JsonValueScope &jv, const td_api::messageChatUpgradeTo &object);

void to_json(JsonValueScope &jv, const td_api::messageChatUpgradeFrom &object);

void to_json(JsonValueScope &jv, const td_api::messagePinMessage &object);

void to_json(JsonValueScope &jv, const td_api::messageScreenshotTaken &object);

void to_json(JsonValueScope &jv, const td_api::messageChatSetTheme &object);

void to_json(JsonValueScope &jv, const td_api::messageChatSetTtl &object);

void to_json(JsonValueScope &jv, const td_api::messageForumTopicCreated &object);

void to_json(JsonValueScope &jv, const td_api::messageForumTopicEdited &object);

void to_json(JsonValueScope &jv, const td_api::messageForumTopicIsClosedToggled &object);

void to_json(JsonValueScope &jv, const td_api::messageCustomServiceAction &object);

void to_json(JsonValueScope &jv, const td_api::messageGameScore &object);

void to_json(JsonValueScope &jv, const td_api::messagePaymentSuccessful &object);

void to_json(JsonValueScope &jv, const td_api::messagePaymentSuccessfulBot &object);

void to_json(JsonValueScope &jv, const td_api::messageGiftedPremium &object);

void to_json(JsonValueScope &jv, const td_api::messageContactRegistered &object);

void to_json(JsonValueScope &jv, const td_api::messageWebsiteConnected &object);

void to_json(JsonValueScope &jv, const td_api::messageWebAppDataSent &object);

void to_json(JsonValueScope &jv, const td_api::messageWebAppDataReceived &object);

void to_json(JsonValueScope &jv, const td_api::messagePassportDataSent &object);

void to_json(JsonValueScope &jv, const td_api::messagePassportDataReceived &object);

void to_json(JsonValueScope &jv, const td_api::messageProximityAlertTriggered &object);

void to_json(JsonValueScope &jv, const td_api::messageUnsupported &object);

void to_json(JsonValueScope &jv, const td_api::messageCopyOptions &object);

void to_json(JsonValueScope &jv, const td_api::MessageExtendedMedia &object);

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaPreview &object);

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaPhoto &object);

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaVideo &object);

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaUnsupported &object);

void to_json(JsonValueScope &jv, const td_api::MessageFileType &object);

void to_json(JsonValueScope &jv, const td_api::messageFileTypePrivate &object);

void to_json(JsonValueScope &jv, const td_api::messageFileTypeGroup &object);

void to_json(JsonValueScope &jv, const td_api::messageFileTypeUnknown &object);

void to_json(JsonValueScope &jv, const td_api::messageForwardInfo &object);

void to_json(JsonValueScope &jv, const td_api::MessageForwardOrigin &object);

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginUser &object);

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginChat &object);

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginHiddenUser &object);

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginChannel &object);

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginMessageImport &object);

void to_json(JsonValueScope &jv, const td_api::messageInteractionInfo &object);

void to_json(JsonValueScope &jv, const td_api::messageLink &object);

void to_json(JsonValueScope &jv, const td_api::messageLinkInfo &object);

void to_json(JsonValueScope &jv, const td_api::messagePosition &object);

void to_json(JsonValueScope &jv, const td_api::messagePositions &object);

void to_json(JsonValueScope &jv, const td_api::messageReaction &object);

void to_json(JsonValueScope &jv, const td_api::messageReplyInfo &object);

void to_json(JsonValueScope &jv, const td_api::MessageSchedulingState &object);

void to_json(JsonValueScope &jv, const td_api::messageSchedulingStateSendAtDate &object);

void to_json(JsonValueScope &jv, const td_api::messageSchedulingStateSendWhenOnline &object);

void to_json(JsonValueScope &jv, const td_api::MessageSender &object);

void to_json(JsonValueScope &jv, const td_api::messageSenderUser &object);

void to_json(JsonValueScope &jv, const td_api::messageSenderChat &object);

void to_json(JsonValueScope &jv, const td_api::messageSenders &object);

void to_json(JsonValueScope &jv, const td_api::MessageSendingState &object);

void to_json(JsonValueScope &jv, const td_api::messageSendingStatePending &object);

void to_json(JsonValueScope &jv, const td_api::messageSendingStateFailed &object);

void to_json(JsonValueScope &jv, const td_api::messageStatistics &object);

void to_json(JsonValueScope &jv, const td_api::messageThreadInfo &object);

void to_json(JsonValueScope &jv, const td_api::messages &object);

void to_json(JsonValueScope &jv, const td_api::minithumbnail &object);

void to_json(JsonValueScope &jv, const td_api::networkStatistics &object);

void to_json(JsonValueScope &jv, const td_api::NetworkStatisticsEntry &object);

void to_json(JsonValueScope &jv, const td_api::networkStatisticsEntryFile &object);

void to_json(JsonValueScope &jv, const td_api::networkStatisticsEntryCall &object);

void to_json(JsonValueScope &jv, const td_api::NetworkType &object);

void to_json(JsonValueScope &jv, const td_api::networkTypeNone &object);

void to_json(JsonValueScope &jv, const td_api::networkTypeMobile &object);

void to_json(JsonValueScope &jv, const td_api::networkTypeMobileRoaming &object);

void to_json(JsonValueScope &jv, const td_api::networkTypeWiFi &object);

void to_json(JsonValueScope &jv, const td_api::networkTypeOther &object);

void to_json(JsonValueScope &jv, const td_api::notification &object);

void to_json(JsonValueScope &jv, const td_api::notificationGroup &object);

void to_json(JsonValueScope &jv, const td_api::NotificationGroupType &object);

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeMessages &object);

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeMentions &object);

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeSecretChat &object);

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeCalls &object);

void to_json(JsonValueScope &jv, const td_api::NotificationSettingsScope &object);

void to_json(JsonValueScope &jv, const td_api::notificationSettingsScopePrivateChats &object);

void to_json(JsonValueScope &jv, const td_api::notificationSettingsScopeGroupChats &object);

void to_json(JsonValueScope &jv, const td_api::notificationSettingsScopeChannelChats &object);

void to_json(JsonValueScope &jv, const td_api::notificationSound &object);

void to_json(JsonValueScope &jv, const td_api::notificationSounds &object);

void to_json(JsonValueScope &jv, const td_api::NotificationType &object);

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewMessage &object);

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewSecretChat &object);

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewCall &object);

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewPushMessage &object);

void to_json(JsonValueScope &jv, const td_api::ok &object);

void to_json(JsonValueScope &jv, const td_api::OptionValue &object);

void to_json(JsonValueScope &jv, const td_api::optionValueBoolean &object);

void to_json(JsonValueScope &jv, const td_api::optionValueEmpty &object);

void to_json(JsonValueScope &jv, const td_api::optionValueInteger &object);

void to_json(JsonValueScope &jv, const td_api::optionValueString &object);

void to_json(JsonValueScope &jv, const td_api::orderInfo &object);

void to_json(JsonValueScope &jv, const td_api::PageBlock &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockTitle &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockSubtitle &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockAuthorDate &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockHeader &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockSubheader &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockKicker &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockParagraph &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockPreformatted &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockFooter &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockDivider &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockAnchor &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockList &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockBlockQuote &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockPullQuote &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockAnimation &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockAudio &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockPhoto &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockVideo &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockCover &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockEmbedded &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockEmbeddedPost &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockCollage &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockSlideshow &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockChatLink &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockTable &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockDetails &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockRelatedArticles &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockMap &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockCaption &object);

void to_json(JsonValueScope &jv, const td_api::PageBlockHorizontalAlignment &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockHorizontalAlignmentLeft &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockHorizontalAlignmentCenter &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockHorizontalAlignmentRight &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockListItem &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockRelatedArticle &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockTableCell &object);

void to_json(JsonValueScope &jv, const td_api::PageBlockVerticalAlignment &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockVerticalAlignmentTop &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockVerticalAlignmentMiddle &object);

void to_json(JsonValueScope &jv, const td_api::pageBlockVerticalAlignmentBottom &object);

void to_json(JsonValueScope &jv, const td_api::passportAuthorizationForm &object);

void to_json(JsonValueScope &jv, const td_api::PassportElement &object);

void to_json(JsonValueScope &jv, const td_api::passportElementPersonalDetails &object);

void to_json(JsonValueScope &jv, const td_api::passportElementPassport &object);

void to_json(JsonValueScope &jv, const td_api::passportElementDriverLicense &object);

void to_json(JsonValueScope &jv, const td_api::passportElementIdentityCard &object);

void to_json(JsonValueScope &jv, const td_api::passportElementInternalPassport &object);

void to_json(JsonValueScope &jv, const td_api::passportElementAddress &object);

void to_json(JsonValueScope &jv, const td_api::passportElementUtilityBill &object);

void to_json(JsonValueScope &jv, const td_api::passportElementBankStatement &object);

void to_json(JsonValueScope &jv, const td_api::passportElementRentalAgreement &object);

void to_json(JsonValueScope &jv, const td_api::passportElementPassportRegistration &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTemporaryRegistration &object);

void to_json(JsonValueScope &jv, const td_api::passportElementPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::passportElementEmailAddress &object);

void to_json(JsonValueScope &jv, const td_api::passportElementError &object);

void to_json(JsonValueScope &jv, const td_api::PassportElementErrorSource &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceUnspecified &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceDataField &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceFrontSide &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceReverseSide &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceSelfie &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceTranslationFile &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceTranslationFiles &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceFile &object);

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceFiles &object);

void to_json(JsonValueScope &jv, const td_api::PassportElementType &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypePersonalDetails &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypePassport &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeDriverLicense &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeIdentityCard &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeInternalPassport &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeAddress &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeUtilityBill &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeBankStatement &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeRentalAgreement &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypePassportRegistration &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeTemporaryRegistration &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypePhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::passportElementTypeEmailAddress &object);

void to_json(JsonValueScope &jv, const td_api::passportElements &object);

void to_json(JsonValueScope &jv, const td_api::passportElementsWithErrors &object);

void to_json(JsonValueScope &jv, const td_api::passportRequiredElement &object);

void to_json(JsonValueScope &jv, const td_api::passportSuitableElement &object);

void to_json(JsonValueScope &jv, const td_api::passwordState &object);

void to_json(JsonValueScope &jv, const td_api::paymentForm &object);

void to_json(JsonValueScope &jv, const td_api::paymentOption &object);

void to_json(JsonValueScope &jv, const td_api::PaymentProvider &object);

void to_json(JsonValueScope &jv, const td_api::paymentProviderSmartGlocal &object);

void to_json(JsonValueScope &jv, const td_api::paymentProviderStripe &object);

void to_json(JsonValueScope &jv, const td_api::paymentProviderOther &object);

void to_json(JsonValueScope &jv, const td_api::paymentReceipt &object);

void to_json(JsonValueScope &jv, const td_api::paymentResult &object);

void to_json(JsonValueScope &jv, const td_api::personalDetails &object);

void to_json(JsonValueScope &jv, const td_api::personalDocument &object);

void to_json(JsonValueScope &jv, const td_api::phoneNumberInfo &object);

void to_json(JsonValueScope &jv, const td_api::photo &object);

void to_json(JsonValueScope &jv, const td_api::photoSize &object);

void to_json(JsonValueScope &jv, const td_api::point &object);

void to_json(JsonValueScope &jv, const td_api::poll &object);

void to_json(JsonValueScope &jv, const td_api::pollOption &object);

void to_json(JsonValueScope &jv, const td_api::PollType &object);

void to_json(JsonValueScope &jv, const td_api::pollTypeRegular &object);

void to_json(JsonValueScope &jv, const td_api::pollTypeQuiz &object);

void to_json(JsonValueScope &jv, const td_api::PremiumFeature &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureIncreasedLimits &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureIncreasedUploadFileSize &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureImprovedDownloadSpeed &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureVoiceRecognition &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureDisabledAds &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureUniqueReactions &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureUniqueStickers &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureCustomEmoji &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureAdvancedChatManagement &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureProfileBadge &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureEmojiStatus &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureAnimatedProfilePhoto &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureForumTopicIcon &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatureAppIcons &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeaturePromotionAnimation &object);

void to_json(JsonValueScope &jv, const td_api::premiumFeatures &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimit &object);

void to_json(JsonValueScope &jv, const td_api::PremiumLimitType &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeSupergroupCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypePinnedChatCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeCreatedPublicChatCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeSavedAnimationCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeFavoriteStickerCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeChatFilterCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeChatFilterChosenChatCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypePinnedArchivedChatCount &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeCaptionLength &object);

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeBioLength &object);

void to_json(JsonValueScope &jv, const td_api::premiumPaymentOption &object);

void to_json(JsonValueScope &jv, const td_api::premiumState &object);

void to_json(JsonValueScope &jv, const td_api::profilePhoto &object);

void to_json(JsonValueScope &jv, const td_api::proxies &object);

void to_json(JsonValueScope &jv, const td_api::proxy &object);

void to_json(JsonValueScope &jv, const td_api::ProxyType &object);

void to_json(JsonValueScope &jv, const td_api::proxyTypeSocks5 &object);

void to_json(JsonValueScope &jv, const td_api::proxyTypeHttp &object);

void to_json(JsonValueScope &jv, const td_api::proxyTypeMtproto &object);

void to_json(JsonValueScope &jv, const td_api::PushMessageContent &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentHidden &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentAnimation &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentAudio &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentContact &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentContactRegistered &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentDocument &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentGame &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentGameScore &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentInvoice &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentLocation &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentPhoto &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentPoll &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentScreenshotTaken &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentSticker &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentText &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVideo &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVideoNote &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVoiceNote &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentBasicGroupChatCreate &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatAddMembers &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatChangePhoto &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatChangeTitle &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatSetTheme &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatDeleteMember &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatJoinByLink &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatJoinByRequest &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentRecurringPayment &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentMessageForwards &object);

void to_json(JsonValueScope &jv, const td_api::pushMessageContentMediaAlbum &object);

void to_json(JsonValueScope &jv, const td_api::pushReceiverId &object);

void to_json(JsonValueScope &jv, const td_api::ReactionType &object);

void to_json(JsonValueScope &jv, const td_api::reactionTypeEmoji &object);

void to_json(JsonValueScope &jv, const td_api::reactionTypeCustomEmoji &object);

void to_json(JsonValueScope &jv, const td_api::recommendedChatFilter &object);

void to_json(JsonValueScope &jv, const td_api::recommendedChatFilters &object);

void to_json(JsonValueScope &jv, const td_api::recoveryEmailAddress &object);

void to_json(JsonValueScope &jv, const td_api::remoteFile &object);

void to_json(JsonValueScope &jv, const td_api::ReplyMarkup &object);

void to_json(JsonValueScope &jv, const td_api::replyMarkupRemoveKeyboard &object);

void to_json(JsonValueScope &jv, const td_api::replyMarkupForceReply &object);

void to_json(JsonValueScope &jv, const td_api::replyMarkupShowKeyboard &object);

void to_json(JsonValueScope &jv, const td_api::replyMarkupInlineKeyboard &object);

void to_json(JsonValueScope &jv, const td_api::ResetPasswordResult &object);

void to_json(JsonValueScope &jv, const td_api::resetPasswordResultOk &object);

void to_json(JsonValueScope &jv, const td_api::resetPasswordResultPending &object);

void to_json(JsonValueScope &jv, const td_api::resetPasswordResultDeclined &object);

void to_json(JsonValueScope &jv, const td_api::RichText &object);

void to_json(JsonValueScope &jv, const td_api::richTextPlain &object);

void to_json(JsonValueScope &jv, const td_api::richTextBold &object);

void to_json(JsonValueScope &jv, const td_api::richTextItalic &object);

void to_json(JsonValueScope &jv, const td_api::richTextUnderline &object);

void to_json(JsonValueScope &jv, const td_api::richTextStrikethrough &object);

void to_json(JsonValueScope &jv, const td_api::richTextFixed &object);

void to_json(JsonValueScope &jv, const td_api::richTextUrl &object);

void to_json(JsonValueScope &jv, const td_api::richTextEmailAddress &object);

void to_json(JsonValueScope &jv, const td_api::richTextSubscript &object);

void to_json(JsonValueScope &jv, const td_api::richTextSuperscript &object);

void to_json(JsonValueScope &jv, const td_api::richTextMarked &object);

void to_json(JsonValueScope &jv, const td_api::richTextPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::richTextIcon &object);

void to_json(JsonValueScope &jv, const td_api::richTextReference &object);

void to_json(JsonValueScope &jv, const td_api::richTextAnchor &object);

void to_json(JsonValueScope &jv, const td_api::richTextAnchorLink &object);

void to_json(JsonValueScope &jv, const td_api::richTexts &object);

void to_json(JsonValueScope &jv, const td_api::rtmpUrl &object);

void to_json(JsonValueScope &jv, const td_api::savedCredentials &object);

void to_json(JsonValueScope &jv, const td_api::scopeNotificationSettings &object);

void to_json(JsonValueScope &jv, const td_api::seconds &object);

void to_json(JsonValueScope &jv, const td_api::secretChat &object);

void to_json(JsonValueScope &jv, const td_api::SecretChatState &object);

void to_json(JsonValueScope &jv, const td_api::secretChatStatePending &object);

void to_json(JsonValueScope &jv, const td_api::secretChatStateReady &object);

void to_json(JsonValueScope &jv, const td_api::secretChatStateClosed &object);

void to_json(JsonValueScope &jv, const td_api::sentWebAppMessage &object);

void to_json(JsonValueScope &jv, const td_api::session &object);

void to_json(JsonValueScope &jv, const td_api::SessionType &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeAndroid &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeApple &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeBrave &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeChrome &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeEdge &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeFirefox &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeIpad &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeIphone &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeLinux &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeMac &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeOpera &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeSafari &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeUbuntu &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeUnknown &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeVivaldi &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeWindows &object);

void to_json(JsonValueScope &jv, const td_api::sessionTypeXbox &object);

void to_json(JsonValueScope &jv, const td_api::sessions &object);

void to_json(JsonValueScope &jv, const td_api::shippingOption &object);

void to_json(JsonValueScope &jv, const td_api::SpeechRecognitionResult &object);

void to_json(JsonValueScope &jv, const td_api::speechRecognitionResultPending &object);

void to_json(JsonValueScope &jv, const td_api::speechRecognitionResultText &object);

void to_json(JsonValueScope &jv, const td_api::speechRecognitionResultError &object);

void to_json(JsonValueScope &jv, const td_api::sponsoredMessage &object);

void to_json(JsonValueScope &jv, const td_api::sponsoredMessages &object);

void to_json(JsonValueScope &jv, const td_api::StatisticalGraph &object);

void to_json(JsonValueScope &jv, const td_api::statisticalGraphData &object);

void to_json(JsonValueScope &jv, const td_api::statisticalGraphAsync &object);

void to_json(JsonValueScope &jv, const td_api::statisticalGraphError &object);

void to_json(JsonValueScope &jv, const td_api::statisticalValue &object);

void to_json(JsonValueScope &jv, const td_api::sticker &object);

void to_json(JsonValueScope &jv, const td_api::StickerFormat &object);

void to_json(JsonValueScope &jv, const td_api::stickerFormatWebp &object);

void to_json(JsonValueScope &jv, const td_api::stickerFormatTgs &object);

void to_json(JsonValueScope &jv, const td_api::stickerFormatWebm &object);

void to_json(JsonValueScope &jv, const td_api::stickerSet &object);

void to_json(JsonValueScope &jv, const td_api::stickerSetInfo &object);

void to_json(JsonValueScope &jv, const td_api::stickerSets &object);

void to_json(JsonValueScope &jv, const td_api::StickerType &object);

void to_json(JsonValueScope &jv, const td_api::stickerTypeRegular &object);

void to_json(JsonValueScope &jv, const td_api::stickerTypeMask &object);

void to_json(JsonValueScope &jv, const td_api::stickerTypeCustomEmoji &object);

void to_json(JsonValueScope &jv, const td_api::stickers &object);

void to_json(JsonValueScope &jv, const td_api::storageStatistics &object);

void to_json(JsonValueScope &jv, const td_api::storageStatisticsByChat &object);

void to_json(JsonValueScope &jv, const td_api::storageStatisticsByFileType &object);

void to_json(JsonValueScope &jv, const td_api::storageStatisticsFast &object);

void to_json(JsonValueScope &jv, const td_api::SuggestedAction &object);

void to_json(JsonValueScope &jv, const td_api::suggestedActionEnableArchiveAndMuteNewChats &object);

void to_json(JsonValueScope &jv, const td_api::suggestedActionCheckPassword &object);

void to_json(JsonValueScope &jv, const td_api::suggestedActionCheckPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::suggestedActionViewChecksHint &object);

void to_json(JsonValueScope &jv, const td_api::suggestedActionConvertToBroadcastGroup &object);

void to_json(JsonValueScope &jv, const td_api::suggestedActionSetPassword &object);

void to_json(JsonValueScope &jv, const td_api::supergroup &object);

void to_json(JsonValueScope &jv, const td_api::supergroupFullInfo &object);

void to_json(JsonValueScope &jv, const td_api::tMeUrl &object);

void to_json(JsonValueScope &jv, const td_api::TMeUrlType &object);

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeUser &object);

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeSupergroup &object);

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeChatInvite &object);

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeStickerSet &object);

void to_json(JsonValueScope &jv, const td_api::tMeUrls &object);

void to_json(JsonValueScope &jv, const td_api::TargetChat &object);

void to_json(JsonValueScope &jv, const td_api::targetChatCurrent &object);

void to_json(JsonValueScope &jv, const td_api::targetChatChosen &object);

void to_json(JsonValueScope &jv, const td_api::targetChatInternalLink &object);

void to_json(JsonValueScope &jv, const td_api::temporaryPasswordState &object);

void to_json(JsonValueScope &jv, const td_api::termsOfService &object);

void to_json(JsonValueScope &jv, const td_api::testBytes &object);

void to_json(JsonValueScope &jv, const td_api::testInt &object);

void to_json(JsonValueScope &jv, const td_api::testString &object);

void to_json(JsonValueScope &jv, const td_api::testVectorInt &object);

void to_json(JsonValueScope &jv, const td_api::testVectorIntObject &object);

void to_json(JsonValueScope &jv, const td_api::testVectorString &object);

void to_json(JsonValueScope &jv, const td_api::testVectorStringObject &object);

void to_json(JsonValueScope &jv, const td_api::text &object);

void to_json(JsonValueScope &jv, const td_api::textEntities &object);

void to_json(JsonValueScope &jv, const td_api::textEntity &object);

void to_json(JsonValueScope &jv, const td_api::TextEntityType &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeMention &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeHashtag &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeCashtag &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeBotCommand &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeUrl &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeEmailAddress &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypePhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeBankCardNumber &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeBold &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeItalic &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeUnderline &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeStrikethrough &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeSpoiler &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeCode &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypePre &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypePreCode &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeTextUrl &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeMentionName &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeCustomEmoji &object);

void to_json(JsonValueScope &jv, const td_api::textEntityTypeMediaTimestamp &object);

void to_json(JsonValueScope &jv, const td_api::themeSettings &object);

void to_json(JsonValueScope &jv, const td_api::thumbnail &object);

void to_json(JsonValueScope &jv, const td_api::ThumbnailFormat &object);

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatJpeg &object);

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatGif &object);

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatMpeg4 &object);

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatPng &object);

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatTgs &object);

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatWebm &object);

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatWebp &object);

void to_json(JsonValueScope &jv, const td_api::trendingStickerSets &object);

void to_json(JsonValueScope &jv, const td_api::unreadReaction &object);

void to_json(JsonValueScope &jv, const td_api::Update &object);

void to_json(JsonValueScope &jv, const td_api::updateAuthorizationState &object);

void to_json(JsonValueScope &jv, const td_api::updateNewMessage &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageSendAcknowledged &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageSendSucceeded &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageSendFailed &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageContent &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageEdited &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageIsPinned &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageInteractionInfo &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageContentOpened &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageMentionRead &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageUnreadReactions &object);

void to_json(JsonValueScope &jv, const td_api::updateMessageLiveLocationViewed &object);

void to_json(JsonValueScope &jv, const td_api::updateNewChat &object);

void to_json(JsonValueScope &jv, const td_api::updateChatTitle &object);

void to_json(JsonValueScope &jv, const td_api::updateChatPhoto &object);

void to_json(JsonValueScope &jv, const td_api::updateChatPermissions &object);

void to_json(JsonValueScope &jv, const td_api::updateChatLastMessage &object);

void to_json(JsonValueScope &jv, const td_api::updateChatPosition &object);

void to_json(JsonValueScope &jv, const td_api::updateChatReadInbox &object);

void to_json(JsonValueScope &jv, const td_api::updateChatReadOutbox &object);

void to_json(JsonValueScope &jv, const td_api::updateChatActionBar &object);

void to_json(JsonValueScope &jv, const td_api::updateChatAvailableReactions &object);

void to_json(JsonValueScope &jv, const td_api::updateChatDraftMessage &object);

void to_json(JsonValueScope &jv, const td_api::updateChatMessageSender &object);

void to_json(JsonValueScope &jv, const td_api::updateChatMessageTtl &object);

void to_json(JsonValueScope &jv, const td_api::updateChatNotificationSettings &object);

void to_json(JsonValueScope &jv, const td_api::updateChatPendingJoinRequests &object);

void to_json(JsonValueScope &jv, const td_api::updateChatReplyMarkup &object);

void to_json(JsonValueScope &jv, const td_api::updateChatTheme &object);

void to_json(JsonValueScope &jv, const td_api::updateChatUnreadMentionCount &object);

void to_json(JsonValueScope &jv, const td_api::updateChatUnreadReactionCount &object);

void to_json(JsonValueScope &jv, const td_api::updateChatVideoChat &object);

void to_json(JsonValueScope &jv, const td_api::updateChatDefaultDisableNotification &object);

void to_json(JsonValueScope &jv, const td_api::updateChatHasProtectedContent &object);

void to_json(JsonValueScope &jv, const td_api::updateChatHasScheduledMessages &object);

void to_json(JsonValueScope &jv, const td_api::updateChatIsBlocked &object);

void to_json(JsonValueScope &jv, const td_api::updateChatIsMarkedAsUnread &object);

void to_json(JsonValueScope &jv, const td_api::updateChatFilters &object);

void to_json(JsonValueScope &jv, const td_api::updateChatOnlineMemberCount &object);

void to_json(JsonValueScope &jv, const td_api::updateForumTopicInfo &object);

void to_json(JsonValueScope &jv, const td_api::updateScopeNotificationSettings &object);

void to_json(JsonValueScope &jv, const td_api::updateNotification &object);

void to_json(JsonValueScope &jv, const td_api::updateNotificationGroup &object);

void to_json(JsonValueScope &jv, const td_api::updateActiveNotifications &object);

void to_json(JsonValueScope &jv, const td_api::updateHavePendingNotifications &object);

void to_json(JsonValueScope &jv, const td_api::updateDeleteMessages &object);

void to_json(JsonValueScope &jv, const td_api::updateChatAction &object);

void to_json(JsonValueScope &jv, const td_api::updateUserStatus &object);

void to_json(JsonValueScope &jv, const td_api::updateUser &object);

void to_json(JsonValueScope &jv, const td_api::updateBasicGroup &object);

void to_json(JsonValueScope &jv, const td_api::updateSupergroup &object);

void to_json(JsonValueScope &jv, const td_api::updateSecretChat &object);

void to_json(JsonValueScope &jv, const td_api::updateUserFullInfo &object);

void to_json(JsonValueScope &jv, const td_api::updateBasicGroupFullInfo &object);

void to_json(JsonValueScope &jv, const td_api::updateSupergroupFullInfo &object);

void to_json(JsonValueScope &jv, const td_api::updateServiceNotification &object);

void to_json(JsonValueScope &jv, const td_api::updateFile &object);

void to_json(JsonValueScope &jv, const td_api::updateFileGenerationStart &object);

void to_json(JsonValueScope &jv, const td_api::updateFileGenerationStop &object);

void to_json(JsonValueScope &jv, const td_api::updateFileDownloads &object);

void to_json(JsonValueScope &jv, const td_api::updateFileAddedToDownloads &object);

void to_json(JsonValueScope &jv, const td_api::updateFileDownload &object);

void to_json(JsonValueScope &jv, const td_api::updateFileRemovedFromDownloads &object);

void to_json(JsonValueScope &jv, const td_api::updateCall &object);

void to_json(JsonValueScope &jv, const td_api::updateGroupCall &object);

void to_json(JsonValueScope &jv, const td_api::updateGroupCallParticipant &object);

void to_json(JsonValueScope &jv, const td_api::updateNewCallSignalingData &object);

void to_json(JsonValueScope &jv, const td_api::updateUserPrivacySettingRules &object);

void to_json(JsonValueScope &jv, const td_api::updateUnreadMessageCount &object);

void to_json(JsonValueScope &jv, const td_api::updateUnreadChatCount &object);

void to_json(JsonValueScope &jv, const td_api::updateOption &object);

void to_json(JsonValueScope &jv, const td_api::updateStickerSet &object);

void to_json(JsonValueScope &jv, const td_api::updateInstalledStickerSets &object);

void to_json(JsonValueScope &jv, const td_api::updateTrendingStickerSets &object);

void to_json(JsonValueScope &jv, const td_api::updateRecentStickers &object);

void to_json(JsonValueScope &jv, const td_api::updateFavoriteStickers &object);

void to_json(JsonValueScope &jv, const td_api::updateSavedAnimations &object);

void to_json(JsonValueScope &jv, const td_api::updateSavedNotificationSounds &object);

void to_json(JsonValueScope &jv, const td_api::updateSelectedBackground &object);

void to_json(JsonValueScope &jv, const td_api::updateChatThemes &object);

void to_json(JsonValueScope &jv, const td_api::updateLanguagePackStrings &object);

void to_json(JsonValueScope &jv, const td_api::updateConnectionState &object);

void to_json(JsonValueScope &jv, const td_api::updateTermsOfService &object);

void to_json(JsonValueScope &jv, const td_api::updateUsersNearby &object);

void to_json(JsonValueScope &jv, const td_api::updateAttachmentMenuBots &object);

void to_json(JsonValueScope &jv, const td_api::updateWebAppMessageSent &object);

void to_json(JsonValueScope &jv, const td_api::updateActiveEmojiReactions &object);

void to_json(JsonValueScope &jv, const td_api::updateDefaultReactionType &object);

void to_json(JsonValueScope &jv, const td_api::updateDiceEmojis &object);

void to_json(JsonValueScope &jv, const td_api::updateAnimatedEmojiMessageClicked &object);

void to_json(JsonValueScope &jv, const td_api::updateAnimationSearchParameters &object);

void to_json(JsonValueScope &jv, const td_api::updateSuggestedActions &object);

void to_json(JsonValueScope &jv, const td_api::updateNewInlineQuery &object);

void to_json(JsonValueScope &jv, const td_api::updateNewChosenInlineResult &object);

void to_json(JsonValueScope &jv, const td_api::updateNewCallbackQuery &object);

void to_json(JsonValueScope &jv, const td_api::updateNewInlineCallbackQuery &object);

void to_json(JsonValueScope &jv, const td_api::updateNewShippingQuery &object);

void to_json(JsonValueScope &jv, const td_api::updateNewPreCheckoutQuery &object);

void to_json(JsonValueScope &jv, const td_api::updateNewCustomEvent &object);

void to_json(JsonValueScope &jv, const td_api::updateNewCustomQuery &object);

void to_json(JsonValueScope &jv, const td_api::updatePoll &object);

void to_json(JsonValueScope &jv, const td_api::updatePollAnswer &object);

void to_json(JsonValueScope &jv, const td_api::updateChatMember &object);

void to_json(JsonValueScope &jv, const td_api::updateNewChatJoinRequest &object);

void to_json(JsonValueScope &jv, const td_api::updates &object);

void to_json(JsonValueScope &jv, const td_api::user &object);

void to_json(JsonValueScope &jv, const td_api::userFullInfo &object);

void to_json(JsonValueScope &jv, const td_api::UserPrivacySetting &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowStatus &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowProfilePhoto &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowLinkInForwardedMessages &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowChatInvites &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowCalls &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowPeerToPeerCalls &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowFindingByPhoneNumber &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &object);

void to_json(JsonValueScope &jv, const td_api::UserPrivacySettingRule &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowAll &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowContacts &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowUsers &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowChatMembers &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictAll &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictContacts &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictUsers &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictChatMembers &object);

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRules &object);

void to_json(JsonValueScope &jv, const td_api::UserStatus &object);

void to_json(JsonValueScope &jv, const td_api::userStatusEmpty &object);

void to_json(JsonValueScope &jv, const td_api::userStatusOnline &object);

void to_json(JsonValueScope &jv, const td_api::userStatusOffline &object);

void to_json(JsonValueScope &jv, const td_api::userStatusRecently &object);

void to_json(JsonValueScope &jv, const td_api::userStatusLastWeek &object);

void to_json(JsonValueScope &jv, const td_api::userStatusLastMonth &object);

void to_json(JsonValueScope &jv, const td_api::userSupportInfo &object);

void to_json(JsonValueScope &jv, const td_api::UserType &object);

void to_json(JsonValueScope &jv, const td_api::userTypeRegular &object);

void to_json(JsonValueScope &jv, const td_api::userTypeDeleted &object);

void to_json(JsonValueScope &jv, const td_api::userTypeBot &object);

void to_json(JsonValueScope &jv, const td_api::userTypeUnknown &object);

void to_json(JsonValueScope &jv, const td_api::usernames &object);

void to_json(JsonValueScope &jv, const td_api::users &object);

void to_json(JsonValueScope &jv, const td_api::validatedOrderInfo &object);

void to_json(JsonValueScope &jv, const td_api::VectorPathCommand &object);

void to_json(JsonValueScope &jv, const td_api::vectorPathCommandLine &object);

void to_json(JsonValueScope &jv, const td_api::vectorPathCommandCubicBezierCurve &object);

void to_json(JsonValueScope &jv, const td_api::venue &object);

void to_json(JsonValueScope &jv, const td_api::video &object);

void to_json(JsonValueScope &jv, const td_api::videoChat &object);

void to_json(JsonValueScope &jv, const td_api::videoNote &object);

void to_json(JsonValueScope &jv, const td_api::voiceNote &object);

void to_json(JsonValueScope &jv, const td_api::webAppInfo &object);

void to_json(JsonValueScope &jv, const td_api::webPage &object);

void to_json(JsonValueScope &jv, const td_api::webPageInstantView &object);

}  // namespace td_api
}  // namespace td
