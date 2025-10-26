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
Result<int32> tl_constructor_from_string(td_api::BusinessAwayMessageSchedule *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"businessAwayMessageScheduleAlways", -910564679},
    {"businessAwayMessageScheduleOutsideOfOpeningHours", -968630506},
    {"businessAwayMessageScheduleCustom", -1967108654}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ChatPhotoStickerType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatPhotoStickerTypeRegularOrMask", -415147620},
    {"chatPhotoStickerTypeCustomEmoji", -266224943}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::GiftResalePrice *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"giftResalePriceStar", 1184402054},
    {"giftResalePriceTon", -415435950}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputFile *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputFileId", 1788906253},
    {"inputFileRemote", -107574466},
    {"inputFileLocal", 2056030919},
    {"inputFileGenerated", -1333385216}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputStoryAreaType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputStoryAreaTypeLocation", -1433714887},
    {"inputStoryAreaTypeFoundVenue", -1395809130},
    {"inputStoryAreaTypePreviousVenue", 1846693388},
    {"inputStoryAreaTypeSuggestedReaction", 2101826003},
    {"inputStoryAreaTypeMessage", -266607529},
    {"inputStoryAreaTypeLink", 1408441160},
    {"inputStoryAreaTypeWeather", -1212686691},
    {"inputStoryAreaTypeUpgradedGift", 793059694}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::MessageSelfDestructType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"messageSelfDestructTypeTimer", 1351440333},
    {"messageSelfDestructTypeImmediately", -1036218363}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PassportElementType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"passportElementTypePersonalDetails", -1032136365},
    {"passportElementTypePassport", -436360376},
    {"passportElementTypeDriverLicense", 1827298379},
    {"passportElementTypeIdentityCard", -502356132},
    {"passportElementTypeInternalPassport", -793781959},
    {"passportElementTypeAddress", 496327874},
    {"passportElementTypeUtilityBill", 627084906},
    {"passportElementTypeBankStatement", 574095667},
    {"passportElementTypeRentalAgreement", -2060583280},
    {"passportElementTypePassportRegistration", -159478209},
    {"passportElementTypeTemporaryRegistration", 1092498527},
    {"passportElementTypePhoneNumber", -995361172},
    {"passportElementTypeEmailAddress", -79321405}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PublicChatType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"publicChatTypeHasUsername", 350789758},
    {"publicChatTypeIsLocationBased", 1183735952}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::StickerType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"stickerTypeRegular", 56345973},
    {"stickerTypeMask", -1765394796},
    {"stickerTypeCustomEmoji", -120752249}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::TelegramPaymentPurpose *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"telegramPaymentPurposePremiumGift", -1600286150},
    {"telegramPaymentPurposePremiumGiftCodes", -1863495348},
    {"telegramPaymentPurposePremiumGiveaway", -760757441},
    {"telegramPaymentPurposeStars", 1204968037},
    {"telegramPaymentPurposeGiftedStars", -1850308042},
    {"telegramPaymentPurposeStarGiveaway", 1014604689},
    {"telegramPaymentPurposeJoinChat", -1914869880}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::acceptedGiftTypes &to, JsonObject &from) {
  TRY_STATUS(from_json(to.unlimited_gifts_, from.extract_field("unlimited_gifts")));
  TRY_STATUS(from_json(to.limited_gifts_, from.extract_field("limited_gifts")));
  TRY_STATUS(from_json(to.upgraded_gifts_, from.extract_field("upgraded_gifts")));
  TRY_STATUS(from_json(to.premium_subscription_, from.extract_field("premium_subscription")));
  return Status::OK();
}

Status from_json(td_api::affiliateTypeChannel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillFreeformGradient &to, JsonObject &from) {
  TRY_STATUS(from_json(to.colors_, from.extract_field("colors")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageScheduleOutsideOfOpeningHours &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureAwayMessage &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessOpeningHours &to, JsonObject &from) {
  TRY_STATUS(from_json(to.time_zone_id_, from.extract_field("time_zone_id")));
  TRY_STATUS(from_json(to.opening_hours_, from.extract_field("opening_hours")));
  return Status::OK();
}

Status from_json(td_api::callProblemDropped &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatFolderIcon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusAdministrator &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_title_, from.extract_field("custom_title")));
  TRY_STATUS(from_json(to.can_be_edited_, from.extract_field("can_be_edited")));
  TRY_STATUS(from_json(to.rights_, from.extract_field("rights")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterRestricted &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::collectibleItemTypePhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenWebPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.endpoint_, from.extract_field("endpoint")));
  TRY_STATUS(from_json(to.p256dh_base64url_, from.extract_field("p256dh_base64url")));
  TRY_STATUS(from_json(to.auth_base64url_, from.extract_field("auth_base64url")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationGoogleId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::fileTypeNone &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSecretThumbnail &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::giftForResaleOrderPrice &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::groupCallJoinParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.audio_source_id_, from.extract_field("audio_source_id")));
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  TRY_STATUS(from_json(to.is_muted_, from.extract_field("is_muted")));
  TRY_STATUS(from_json(to.is_my_video_enabled_, from.extract_field("is_my_video_enabled")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallback &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::inputFileId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.performer_, from.extract_field("performer")));
  TRY_STATUS(from_json(to.audio_url_, from.extract_field("audio_url")));
  TRY_STATUS(from_json(to.audio_duration_, from.extract_field("audio_duration")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.voice_note_url_, from.extract_field("voice_note_url")));
  TRY_STATUS(from_json(to.voice_note_duration_, from.extract_field("voice_note_duration")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputMessagePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, from.extract_field("added_sticker_file_ids")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
  TRY_STATUS(from_json(to.self_destruct_type_, from.extract_field("self_destruct_type")));
  TRY_STATUS(from_json(to.has_spoiler_, from.extract_field("has_spoiler")));
  return Status::OK();
}

Status from_json(td_api::inputMessageGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  return Status::OK();
}

Status from_json(td_api::inputPaidMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.media_, from.extract_field("media")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, from.extract_field("added_sticker_file_ids")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementUtilityBill &to, JsonObject &from) {
  TRY_STATUS(from_json(to.utility_bill_, from.extract_field("utility_bill")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceDataField &to, JsonObject &from) {
  TRY_STATUS(from_json(to.field_name_, from.extract_field("field_name")));
  TRY_STATUS(from_json_bytes(to.data_hash_, from.extract_field("data_hash")));
  return Status::OK();
}

Status from_json(td_api::inputSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.format_, from.extract_field("format")));
  TRY_STATUS(from_json(to.emojis_, from.extract_field("emojis")));
  TRY_STATUS(from_json(to.mask_position_, from.extract_field("mask_position")));
  TRY_STATUS(from_json(to.keywords_, from.extract_field("keywords")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeUpgradedGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.gift_name_, from.extract_field("gift_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatBoost &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMyToncoins &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeQrCodeAuthentication &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUnsupportedProxy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::jsonValueBoolean &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestPoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.force_regular_, from.extract_field("force_regular")));
  TRY_STATUS(from_json(to.force_quiz_, from.extract_field("force_quiz")));
  return Status::OK();
}

Status from_json(td_api::languagePackStringValueDeleted &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::maskPointMouth &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSelfDestructTypeImmediately &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceChatList &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageTopicSavedMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::notificationSettingsScopePrivateChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::paidReactionTypeAnonymous &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeBankStatement &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::phoneNumberCodeTypeVerify &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureUniqueReactions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureRealTimeChatTranslation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureMessageEffects &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypePinnedArchivedChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeStoryCaptionLength &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeaturePriorityOrder &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::profileTabMedia &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::publicChatTypeHasUsername &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::readDatePrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.show_read_date_, from.extract_field("show_read_date")));
  return Status::OK();
}

Status from_json(td_api::reportReasonCopyright &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::scopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mute_for_, from.extract_field("mute_for")));
  TRY_STATUS(from_json(to.sound_id_, from.extract_field("sound_id")));
  TRY_STATUS(from_json(to.show_preview_, from.extract_field("show_preview")));
  TRY_STATUS(from_json(to.use_default_mute_stories_, from.extract_field("use_default_mute_stories")));
  TRY_STATUS(from_json(to.mute_stories_, from.extract_field("mute_stories")));
  TRY_STATUS(from_json(to.story_sound_id_, from.extract_field("story_sound_id")));
  TRY_STATUS(from_json(to.show_story_poster_, from.extract_field("show_story_poster")));
  TRY_STATUS(from_json(to.disable_pinned_message_notifications_, from.extract_field("disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.disable_mention_notifications_, from.extract_field("disable_mention_notifications")));
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterUnreadReaction &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerTypeMask &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storeTransactionAppStore &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.receipt_, from.extract_field("receipt")));
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsSelectedUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionSubscribeToAnnualPremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterRecent &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::targetChatChosen &to, JsonObject &from) {
  TRY_STATUS(from_json(to.types_, from.extract_field("types")));
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposeJoinChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeEmailAddress &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypePre &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textParseModeHTML &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryForwardChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowLinkInForwardedMessages &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAutosaveGifts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::webAppOpenParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  TRY_STATUS(from_json(to.application_name_, from.extract_field("application_name")));
  TRY_STATUS(from_json(to.mode_, from.extract_field("mode")));
  return Status::OK();
}

Status from_json(td_api::addChecklistTasks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.tasks_, from.extract_field("tasks")));
  return Status::OK();
}

Status from_json(td_api::addNetworkStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.entry_, from.extract_field("entry")));
  return Status::OK();
}

Status from_json(td_api::addRecentlyFoundChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::answerInlineQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_query_id_, from.extract_field("inline_query_id")));
  TRY_STATUS(from_json(to.is_personal_, from.extract_field("is_personal")));
  TRY_STATUS(from_json(to.button_, from.extract_field("button")));
  TRY_STATUS(from_json(to.results_, from.extract_field("results")));
  TRY_STATUS(from_json(to.cache_time_, from.extract_field("cache_time")));
  TRY_STATUS(from_json(to.next_offset_, from.extract_field("next_offset")));
  return Status::OK();
}

Status from_json(td_api::blockMessageSenderFromReplies &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.delete_message_, from.extract_field("delete_message")));
  TRY_STATUS(from_json(to.delete_all_messages_, from.extract_field("delete_all_messages")));
  TRY_STATUS(from_json(to.report_spam_, from.extract_field("report_spam")));
  return Status::OK();
}

Status from_json(td_api::cancelDownloadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.only_if_pending_, from.extract_field("only_if_pending")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::checkPasswordRecoveryCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  return Status::OK();
}

Status from_json(td_api::clearAutosaveSettingsExceptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clickPremiumSubscriptionButton &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::confirmSession &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  return Status::OK();
}

Status from_json(td_api::createForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.is_name_implicit_, from.extract_field("is_name_implicit")));
  TRY_STATUS(from_json(to.icon_, from.extract_field("icon")));
  return Status::OK();
}

Status from_json(td_api::createSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, from.extract_field("secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::deleteAllCallMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.leave_chat_ids_, from.extract_field("leave_chat_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteDirectMessagesChatTopicMessagesByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.min_date_, from.extract_field("min_date")));
  TRY_STATUS(from_json(to.max_date_, from.extract_field("max_date")));
  return Status::OK();
}

Status from_json(td_api::deleteQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::destroy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::editBotMediaPreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  return Status::OK();
}

Status from_json(td_api::editChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::editProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  TRY_STATUS(from_json(to.server_, from.extract_field("server")));
  TRY_STATUS(from_json(to.port_, from.extract_field("port")));
  TRY_STATUS(from_json(to.enable_, from.extract_field("enable")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::endGroupCallRecording &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getApplicationConfig &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAvailableChatBoostSlots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBotMediaPreviewInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getBusinessConnection &to, JsonObject &from) {
  TRY_STATUS(from_json(to.connection_id_, from.extract_field("connection_id")));
  return Status::OK();
}

Status from_json(td_api::getChatAvailablePaidMessageReactionSenders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderChatCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.creator_user_id_, from.extract_field("creator_user_id")));
  TRY_STATUS(from_json(to.is_revoked_, from.extract_field("is_revoked")));
  TRY_STATUS(from_json(to.offset_date_, from.extract_field("offset_date")));
  TRY_STATUS(from_json(to.offset_invite_link_, from.extract_field("offset_invite_link")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatPinnedMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatSponsoredMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getCurrentState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getEmojiReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::getForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  return Status::OK();
}

Status from_json(td_api::getGiveawayInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getInlineQueryResults &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_location_, from.extract_field("user_location")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_database_path_, from.extract_field("language_pack_database_path")));
  TRY_STATUS(from_json(to.localization_target_, from.extract_field("localization_target")));
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  TRY_STATUS(from_json(to.key_, from.extract_field("key")));
  return Status::OK();
}

Status from_json(td_api::getLoginUrlInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.button_id_, from.extract_field("button_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.row_size_, from.extract_field("row_size")));
  return Status::OK();
}

Status from_json(td_api::getMessagePublicForwards &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::getPaymentReceipt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getPremiumLimit &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_type_, from.extract_field("limit_type")));
  return Status::OK();
}

Status from_json(td_api::getReadDatePrivacySettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecommendedChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSavedNotificationSounds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStarPaymentOptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, from.extract_field("set_id")));
  return Status::OK();
}

Status from_json(td_api::getStoryPublicForwards &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getSupportName &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTonTransactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.direction_, from.extract_field("direction")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getUserChatBoosts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatRtmpUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::hideContactCloseBirthdays &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::joinGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_group_call_, from.extract_field("input_group_call")));
  TRY_STATUS(from_json(to.join_parameters_, from.extract_field("join_parameters")));
  return Status::OK();
}

Status from_json(td_api::loadQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  return Status::OK();
}

Status from_json(td_api::openSponsoredChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sponsored_chat_unique_id_, from.extract_field("sponsored_chat_unique_id")));
  return Status::OK();
}

Status from_json(td_api::preliminaryUploadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_, from.extract_field("file")));
  TRY_STATUS(from_json(to.file_type_, from.extract_field("file_type")));
  TRY_STATUS(from_json(to.priority_, from.extract_field("priority")));
  return Status::OK();
}

Status from_json(td_api::readAllForumTopicMentions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  return Status::OK();
}

Status from_json(td_api::refundStarPayment &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.telegram_payment_charge_id_, from.extract_field("telegram_payment_charge_id")));
  return Status::OK();
}

Status from_json(td_api::removeGiftCollectionGifts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.collection_id_, from.extract_field("collection_id")));
  TRY_STATUS(from_json(to.received_gift_ids_, from.extract_field("received_gift_ids")));
  return Status::OK();
}

Status from_json(td_api::removeRecentHashtag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hashtag_, from.extract_field("hashtag")));
  return Status::OK();
}

Status from_json(td_api::reorderActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.usernames_, from.extract_field("usernames")));
  return Status::OK();
}

Status from_json(td_api::reorderStoryAlbums &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_album_ids_, from.extract_field("story_album_ids")));
  return Status::OK();
}

Status from_json(td_api::reportMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  return Status::OK();
}

Status from_json(td_api::requestQrCodeAuthentication &to, JsonObject &from) {
  TRY_STATUS(from_json(to.other_user_ids_, from.extract_field("other_user_ids")));
  return Status::OK();
}

Status from_json(td_api::resetInstalledBackgrounds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::searchEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.input_language_codes_, from.extract_field("input_language_codes")));
  return Status::OK();
}

Status from_json(td_api::searchPublicMessagesByTag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.ignore_cache_, from.extract_field("ignore_cache")));
  return Status::OK();
}

Status from_json(td_api::sendBotStartMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.parameter_, from.extract_field("parameter")));
  return Status::OK();
}

Status from_json(td_api::sendEmailAddressVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  return Status::OK();
}

Status from_json(td_api::sendPhoneNumberFirebaseSms &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::setApplicationVerificationToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.verification_id_, from.extract_field("verification_id")));
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::setBotInfoDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  return Status::OK();
}

Status from_json(td_api::setBusinessAccountUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::setChatActiveStoriesList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_list_, from.extract_field("story_list")));
  return Status::OK();
}

Status from_json(td_api::setChatEmojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.emoji_status_, from.extract_field("emoji_status")));
  return Status::OK();
}

Status from_json(td_api::setChatPinnedStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_ids_, from.extract_field("story_ids")));
  return Status::OK();
}

Status from_json(td_api::setCustomLanguagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  TRY_STATUS(from_json(to.new_string_, from.extract_field("new_string")));
  return Status::OK();
}

Status from_json(td_api::setFileGenerationProgress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, from.extract_field("generation_id")));
  TRY_STATUS(from_json(to.expected_size_, from.extract_field("expected_size")));
  TRY_STATUS(from_json(to.local_prefix_size_, from.extract_field("local_prefix_size")));
  return Status::OK();
}

Status from_json(td_api::setInlineGameScore &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.edit_message_, from.extract_field("edit_message")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.score_, from.extract_field("score")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::setMessageSenderBlockList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  TRY_STATUS(from_json(to.block_list_, from.extract_field("block_list")));
  return Status::OK();
}

Status from_json(td_api::setPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.old_password_, from.extract_field("old_password")));
  TRY_STATUS(from_json(to.new_password_, from.extract_field("new_password")));
  TRY_STATUS(from_json(to.new_hint_, from.extract_field("new_hint")));
  TRY_STATUS(from_json(to.set_recovery_email_address_, from.extract_field("set_recovery_email_address")));
  TRY_STATUS(from_json(to.new_recovery_email_address_, from.extract_field("new_recovery_email_address")));
  return Status::OK();
}

Status from_json(td_api::setProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  TRY_STATUS(from_json(to.is_public_, from.extract_field("is_public")));
  return Status::OK();
}

Status from_json(td_api::setStickerMaskPosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.mask_position_, from.extract_field("mask_position")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.sticker_set_id_, from.extract_field("sticker_set_id")));
  return Status::OK();
}

Status from_json(td_api::setUserSupportInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  return Status::OK();
}

Status from_json(td_api::startScheduledVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::testCallEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::testReturnError &to, JsonObject &from) {
  TRY_STATUS(from_json(to.error_, from.extract_field("error")));
  return Status::OK();
}

Status from_json(td_api::toggleChatFolderTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.are_tags_enabled_, from.extract_field("are_tags_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleForumTopicIsClosed &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  TRY_STATUS(from_json(to.is_closed_, from.extract_field("is_closed")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallScreenSharingIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.is_paused_, from.extract_field("is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupHasHiddenMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.has_hidden_members_, from.extract_field("has_hidden_members")));
  return Status::OK();
}

Status from_json(td_api::toggleVideoChatEnabledStartNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.enabled_start_notification_, from.extract_field("enabled_start_notification")));
  return Status::OK();
}

Status from_json(td_api::unpinAllForumTopicMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  return Status::OK();
}

Status from_json(td_api::viewTrendingStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_ids_, from.extract_field("sticker_set_ids")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::address &object) {
  auto jo = jv.enter_object();
  jo("@type", "address");
  jo("country_code", object.country_code_);
  jo("state", object.state_);
  jo("city", object.city_);
  jo("street_line1", object.street_line1_);
  jo("street_line2", object.street_line2_);
  jo("postal_code", object.postal_code_);
}

void to_json(JsonValueScope &jv, const td_api::animation &object) {
  auto jo = jv.enter_object();
  jo("@type", "animation");
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
  jo("file_name", object.file_name_);
  jo("mime_type", object.mime_type_);
  jo("has_stickers", JsonBool{object.has_stickers_});
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::AuthorizationState &object) {
  td_api::downcast_call(const_cast<td_api::AuthorizationState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitTdlibParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitTdlibParameters");
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitPhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitPremiumPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitPremiumPurchase");
  jo("store_product_id", object.store_product_id_);
  jo("support_email_address", object.support_email_address_);
  jo("support_email_subject", object.support_email_subject_);
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitEmailAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitEmailAddress");
  jo("allow_apple_id", JsonBool{object.allow_apple_id_});
  jo("allow_google_id", JsonBool{object.allow_google_id_});
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitEmailCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitEmailCode");
  jo("allow_apple_id", JsonBool{object.allow_apple_id_});
  jo("allow_google_id", JsonBool{object.allow_google_id_});
  if (object.code_info_) {
    jo("code_info", ToJson(*object.code_info_));
  }
  if (object.email_address_reset_state_) {
    jo("email_address_reset_state", ToJson(*object.email_address_reset_state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitCode");
  if (object.code_info_) {
    jo("code_info", ToJson(*object.code_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitOtherDeviceConfirmation &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitOtherDeviceConfirmation");
  jo("link", object.link_);
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitRegistration &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitRegistration");
  if (object.terms_of_service_) {
    jo("terms_of_service", ToJson(*object.terms_of_service_));
  }
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateWaitPassword &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateWaitPassword");
  jo("password_hint", object.password_hint_);
  jo("has_recovery_email_address", JsonBool{object.has_recovery_email_address_});
  jo("has_passport_data", JsonBool{object.has_passport_data_});
  jo("recovery_email_address_pattern", object.recovery_email_address_pattern_);
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateReady &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateReady");
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateLoggingOut &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateLoggingOut");
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateClosing &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateClosing");
}

void to_json(JsonValueScope &jv, const td_api::authorizationStateClosed &object) {
  auto jo = jv.enter_object();
  jo("@type", "authorizationStateClosed");
}

void to_json(JsonValueScope &jv, const td_api::availableReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "availableReactions");
  jo("top_reactions", ToJson(object.top_reactions_));
  jo("recent_reactions", ToJson(object.recent_reactions_));
  jo("popular_reactions", ToJson(object.popular_reactions_));
  jo("allow_custom_emoji", JsonBool{object.allow_custom_emoji_});
  jo("are_tags", JsonBool{object.are_tags_});
  if (object.unavailability_reason_) {
    jo("unavailability_reason", ToJson(*object.unavailability_reason_));
  }
}

void to_json(JsonValueScope &jv, const td_api::birthdate &object) {
  auto jo = jv.enter_object();
  jo("@type", "birthdate");
  jo("day", object.day_);
  jo("month", object.month_);
  jo("year", object.year_);
}

void to_json(JsonValueScope &jv, const td_api::botVerification &object) {
  auto jo = jv.enter_object();
  jo("@type", "botVerification");
  jo("bot_user_id", object.bot_user_id_);
  jo("icon_custom_emoji_id", ToJson(JsonInt64{object.icon_custom_emoji_id_}));
  if (object.custom_description_) {
    jo("custom_description", ToJson(*object.custom_description_));
  }
}

void to_json(JsonValueScope &jv, const td_api::businessChatLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessChatLinkInfo");
  jo("chat_id", object.chat_id_);
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::businessLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessLocation");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("address", object.address_);
}

void to_json(JsonValueScope &jv, const td_api::callId &object) {
  auto jo = jv.enter_object();
  jo("@type", "callId");
  jo("id", object.id_);
}

void to_json(JsonValueScope &jv, const td_api::CanSendMessageToUserResult &object) {
  td_api::downcast_call(const_cast<td_api::CanSendMessageToUserResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::canSendMessageToUserResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendMessageToUserResultOk");
}

void to_json(JsonValueScope &jv, const td_api::canSendMessageToUserResultUserHasPaidMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendMessageToUserResultUserHasPaidMessages");
  jo("outgoing_paid_message_star_count", object.outgoing_paid_message_star_count_);
}

void to_json(JsonValueScope &jv, const td_api::canSendMessageToUserResultUserIsDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendMessageToUserResultUserIsDeleted");
}

void to_json(JsonValueScope &jv, const td_api::canSendMessageToUserResultUserRestrictsNewChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendMessageToUserResultUserRestrictsNewChats");
}

void to_json(JsonValueScope &jv, const td_api::ChatAvailableReactions &object) {
  td_api::downcast_call(const_cast<td_api::ChatAvailableReactions &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatAvailableReactionsAll &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAvailableReactionsAll");
  jo("max_reaction_count", object.max_reaction_count_);
}

void to_json(JsonValueScope &jv, const td_api::chatAvailableReactionsSome &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAvailableReactionsSome");
  jo("reactions", ToJson(object.reactions_));
  jo("max_reaction_count", object.max_reaction_count_);
}

void to_json(JsonValueScope &jv, const td_api::ChatBoostSource &object) {
  td_api::downcast_call(const_cast<td_api::ChatBoostSource &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatBoostSourceGiftCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostSourceGiftCode");
  jo("user_id", object.user_id_);
  jo("gift_code", object.gift_code_);
}

void to_json(JsonValueScope &jv, const td_api::chatBoostSourceGiveaway &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostSourceGiveaway");
  jo("user_id", object.user_id_);
  jo("gift_code", object.gift_code_);
  jo("star_count", object.star_count_);
  jo("giveaway_message_id", object.giveaway_message_id_);
  jo("is_unclaimed", JsonBool{object.is_unclaimed_});
}

void to_json(JsonValueScope &jv, const td_api::chatBoostSourcePremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostSourcePremium");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatFolderInviteLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderInviteLinkInfo");
  if (object.chat_folder_info_) {
    jo("chat_folder_info", ToJson(*object.chat_folder_info_));
  }
  jo("missing_chat_ids", ToJson(object.missing_chat_ids_));
  jo("added_chat_ids", ToJson(object.added_chat_ids_));
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkSubscriptionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkSubscriptionInfo");
  if (object.pricing_) {
    jo("pricing", ToJson(*object.pricing_));
  }
  jo("can_reuse", JsonBool{object.can_reuse_});
  jo("form_id", ToJson(JsonInt64{object.form_id_}));
}

void to_json(JsonValueScope &jv, const td_api::ChatMemberStatus &object) {
  td_api::downcast_call(const_cast<td_api::ChatMemberStatus &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusCreator &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMemberStatusCreator");
  jo("custom_title", object.custom_title_);
  jo("is_anonymous", JsonBool{object.is_anonymous_});
  jo("is_member", JsonBool{object.is_member_});
}

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusAdministrator &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMemberStatusAdministrator");
  jo("custom_title", object.custom_title_);
  jo("can_be_edited", JsonBool{object.can_be_edited_});
  if (object.rights_) {
    jo("rights", ToJson(*object.rights_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMemberStatusMember");
  jo("member_until_date", object.member_until_date_);
}

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusRestricted &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMemberStatusRestricted");
  jo("is_member", JsonBool{object.is_member_});
  jo("restricted_until_date", object.restricted_until_date_);
  if (object.permissions_) {
    jo("permissions", ToJson(*object.permissions_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusLeft &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMemberStatusLeft");
}

void to_json(JsonValueScope &jv, const td_api::chatMemberStatusBanned &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMemberStatusBanned");
  jo("banned_until_date", object.banned_until_date_);
}

void to_json(JsonValueScope &jv, const td_api::ChatPhotoStickerType &object) {
  td_api::downcast_call(const_cast<td_api::ChatPhotoStickerType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatPhotoStickerTypeRegularOrMask &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPhotoStickerTypeRegularOrMask");
  jo("sticker_set_id", ToJson(JsonInt64{object.sticker_set_id_}));
  jo("sticker_id", ToJson(JsonInt64{object.sticker_id_}));
}

void to_json(JsonValueScope &jv, const td_api::chatPhotoStickerTypeCustomEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPhotoStickerTypeCustomEmoji");
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::ChatStatistics &object) {
  td_api::downcast_call(const_cast<td_api::ChatStatistics &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsSupergroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsSupergroup");
  if (object.period_) {
    jo("period", ToJson(*object.period_));
  }
  if (object.member_count_) {
    jo("member_count", ToJson(*object.member_count_));
  }
  if (object.message_count_) {
    jo("message_count", ToJson(*object.message_count_));
  }
  if (object.viewer_count_) {
    jo("viewer_count", ToJson(*object.viewer_count_));
  }
  if (object.sender_count_) {
    jo("sender_count", ToJson(*object.sender_count_));
  }
  if (object.member_count_graph_) {
    jo("member_count_graph", ToJson(*object.member_count_graph_));
  }
  if (object.join_graph_) {
    jo("join_graph", ToJson(*object.join_graph_));
  }
  if (object.join_by_source_graph_) {
    jo("join_by_source_graph", ToJson(*object.join_by_source_graph_));
  }
  if (object.language_graph_) {
    jo("language_graph", ToJson(*object.language_graph_));
  }
  if (object.message_content_graph_) {
    jo("message_content_graph", ToJson(*object.message_content_graph_));
  }
  if (object.action_graph_) {
    jo("action_graph", ToJson(*object.action_graph_));
  }
  if (object.day_graph_) {
    jo("day_graph", ToJson(*object.day_graph_));
  }
  if (object.week_graph_) {
    jo("week_graph", ToJson(*object.week_graph_));
  }
  jo("top_senders", ToJson(object.top_senders_));
  jo("top_administrators", ToJson(object.top_administrators_));
  jo("top_inviters", ToJson(object.top_inviters_));
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsChannel &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsChannel");
  if (object.period_) {
    jo("period", ToJson(*object.period_));
  }
  if (object.member_count_) {
    jo("member_count", ToJson(*object.member_count_));
  }
  if (object.mean_message_view_count_) {
    jo("mean_message_view_count", ToJson(*object.mean_message_view_count_));
  }
  if (object.mean_message_share_count_) {
    jo("mean_message_share_count", ToJson(*object.mean_message_share_count_));
  }
  if (object.mean_message_reaction_count_) {
    jo("mean_message_reaction_count", ToJson(*object.mean_message_reaction_count_));
  }
  if (object.mean_story_view_count_) {
    jo("mean_story_view_count", ToJson(*object.mean_story_view_count_));
  }
  if (object.mean_story_share_count_) {
    jo("mean_story_share_count", ToJson(*object.mean_story_share_count_));
  }
  if (object.mean_story_reaction_count_) {
    jo("mean_story_reaction_count", ToJson(*object.mean_story_reaction_count_));
  }
  jo("enabled_notifications_percentage", object.enabled_notifications_percentage_);
  if (object.member_count_graph_) {
    jo("member_count_graph", ToJson(*object.member_count_graph_));
  }
  if (object.join_graph_) {
    jo("join_graph", ToJson(*object.join_graph_));
  }
  if (object.mute_graph_) {
    jo("mute_graph", ToJson(*object.mute_graph_));
  }
  if (object.view_count_by_hour_graph_) {
    jo("view_count_by_hour_graph", ToJson(*object.view_count_by_hour_graph_));
  }
  if (object.view_count_by_source_graph_) {
    jo("view_count_by_source_graph", ToJson(*object.view_count_by_source_graph_));
  }
  if (object.join_by_source_graph_) {
    jo("join_by_source_graph", ToJson(*object.join_by_source_graph_));
  }
  if (object.language_graph_) {
    jo("language_graph", ToJson(*object.language_graph_));
  }
  if (object.message_interaction_graph_) {
    jo("message_interaction_graph", ToJson(*object.message_interaction_graph_));
  }
  if (object.message_reaction_graph_) {
    jo("message_reaction_graph", ToJson(*object.message_reaction_graph_));
  }
  if (object.story_interaction_graph_) {
    jo("story_interaction_graph", ToJson(*object.story_interaction_graph_));
  }
  if (object.story_reaction_graph_) {
    jo("story_reaction_graph", ToJson(*object.story_reaction_graph_));
  }
  if (object.instant_view_interaction_graph_) {
    jo("instant_view_interaction_graph", ToJson(*object.instant_view_interaction_graph_));
  }
  jo("recent_interactions", ToJson(object.recent_interactions_));
}

void to_json(JsonValueScope &jv, const td_api::CheckChatUsernameResult &object) {
  td_api::downcast_call(const_cast<td_api::CheckChatUsernameResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkChatUsernameResultOk");
}

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultUsernameInvalid &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkChatUsernameResultUsernameInvalid");
}

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultUsernameOccupied &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkChatUsernameResultUsernameOccupied");
}

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultUsernamePurchasable &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkChatUsernameResultUsernamePurchasable");
}

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultPublicChatsTooMany &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkChatUsernameResultPublicChatsTooMany");
}

void to_json(JsonValueScope &jv, const td_api::checkChatUsernameResultPublicGroupsUnavailable &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkChatUsernameResultPublicGroupsUnavailable");
}

void to_json(JsonValueScope &jv, const td_api::connectedWebsite &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectedWebsite");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("domain_name", object.domain_name_);
  jo("bot_user_id", object.bot_user_id_);
  jo("browser", object.browser_);
  jo("platform", object.platform_);
  jo("log_in_date", object.log_in_date_);
  jo("last_active_date", object.last_active_date_);
  jo("ip_address", object.ip_address_);
  jo("location", object.location_);
}

void to_json(JsonValueScope &jv, const td_api::customRequestResult &object) {
  auto jo = jv.enter_object();
  jo("@type", "customRequestResult");
  jo("result", object.result_);
}

void to_json(JsonValueScope &jv, const td_api::document &object) {
  auto jo = jv.enter_object();
  jo("@type", "document");
  jo("file_name", object.file_name_);
  jo("mime_type", object.mime_type_);
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
}

void to_json(JsonValueScope &jv, const td_api::emojiKeyword &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiKeyword");
  jo("emoji", object.emoji_);
  jo("keyword", object.keyword_);
}

void to_json(JsonValueScope &jv, const td_api::encryptedPassportElement &object) {
  auto jo = jv.enter_object();
  jo("@type", "encryptedPassportElement");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("data", base64_encode(object.data_));
  if (object.front_side_) {
    jo("front_side", ToJson(*object.front_side_));
  }
  if (object.reverse_side_) {
    jo("reverse_side", ToJson(*object.reverse_side_));
  }
  if (object.selfie_) {
    jo("selfie", ToJson(*object.selfie_));
  }
  jo("translation", ToJson(object.translation_));
  jo("files", ToJson(object.files_));
  jo("value", object.value_);
  jo("hash", object.hash_);
}

void to_json(JsonValueScope &jv, const td_api::FirebaseDeviceVerificationParameters &object) {
  td_api::downcast_call(const_cast<td_api::FirebaseDeviceVerificationParameters &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::firebaseDeviceVerificationParametersSafetyNet &object) {
  auto jo = jv.enter_object();
  jo("@type", "firebaseDeviceVerificationParametersSafetyNet");
  jo("nonce", base64_encode(object.nonce_));
}

void to_json(JsonValueScope &jv, const td_api::firebaseDeviceVerificationParametersPlayIntegrity &object) {
  auto jo = jv.enter_object();
  jo("@type", "firebaseDeviceVerificationParametersPlayIntegrity");
  jo("nonce", object.nonce_);
  jo("cloud_project_number", ToJson(JsonInt64{object.cloud_project_number_}));
}

void to_json(JsonValueScope &jv, const td_api::foundChatBoosts &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundChatBoosts");
  jo("total_count", object.total_count_);
  jo("boosts", ToJson(object.boosts_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::foundWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundWebApp");
  if (object.web_app_) {
    jo("web_app", ToJson(*object.web_app_));
  }
  jo("request_write_access", JsonBool{object.request_write_access_});
  jo("skip_confirmation", JsonBool{object.skip_confirmation_});
}

void to_json(JsonValueScope &jv, const td_api::giftForResale &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftForResale");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  jo("received_gift_id", object.received_gift_id_);
}

void to_json(JsonValueScope &jv, const td_api::GiveawayInfo &object) {
  td_api::downcast_call(const_cast<td_api::GiveawayInfo &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::giveawayInfoOngoing &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayInfoOngoing");
  jo("creation_date", object.creation_date_);
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
  jo("is_ended", JsonBool{object.is_ended_});
}

void to_json(JsonValueScope &jv, const td_api::giveawayInfoCompleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayInfoCompleted");
  jo("creation_date", object.creation_date_);
  jo("actual_winners_selection_date", object.actual_winners_selection_date_);
  jo("was_refunded", JsonBool{object.was_refunded_});
  jo("is_winner", JsonBool{object.is_winner_});
  jo("winner_count", object.winner_count_);
  jo("activation_count", object.activation_count_);
  jo("gift_code", object.gift_code_);
  jo("won_star_count", object.won_star_count_);
}

void to_json(JsonValueScope &jv, const td_api::groupCallParticipants &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallParticipants");
  jo("total_count", object.total_count_);
  jo("participant_ids", ToJson(object.participant_ids_));
}

void to_json(JsonValueScope &jv, const td_api::InlineQueryResult &object) {
  td_api::downcast_call(const_cast<td_api::InlineQueryResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultArticle &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultArticle");
  jo("id", object.id_);
  jo("url", object.url_);
  jo("title", object.title_);
  jo("description", object.description_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultContact &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultContact");
  jo("id", object.id_);
  if (object.contact_) {
    jo("contact", ToJson(*object.contact_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultLocation");
  jo("id", object.id_);
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("title", object.title_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultVenue &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultVenue");
  jo("id", object.id_);
  if (object.venue_) {
    jo("venue", ToJson(*object.venue_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultGame");
  jo("id", object.id_);
  if (object.game_) {
    jo("game", ToJson(*object.game_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultAnimation");
  jo("id", object.id_);
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultAudio");
  jo("id", object.id_);
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultDocument");
  jo("id", object.id_);
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  jo("title", object.title_);
  jo("description", object.description_);
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultPhoto");
  jo("id", object.id_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("title", object.title_);
  jo("description", object.description_);
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultSticker");
  jo("id", object.id_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultVideo");
  jo("id", object.id_);
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  jo("title", object.title_);
  jo("description", object.description_);
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultVoiceNote");
  jo("id", object.id_);
  if (object.voice_note_) {
    jo("voice_note", ToJson(*object.voice_note_));
  }
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::inputPaidMedia &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputPaidMedia");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  if (object.media_) {
    jo("media", ToJson(*object.media_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("added_sticker_file_ids", ToJson(object.added_sticker_file_ids_));
  jo("width", object.width_);
  jo("height", object.height_);
}

void to_json(JsonValueScope &jv, const td_api::jsonObjectMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonObjectMember");
  jo("key", object.key_);
  if (object.value_) {
    jo("value", ToJson(*object.value_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreview &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreview");
  jo("url", object.url_);
  jo("display_url", object.display_url_);
  jo("site_name", object.site_name_);
  jo("title", object.title_);
  if (object.description_) {
    jo("description", ToJson(*object.description_));
  }
  jo("author", object.author_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("has_large_media", JsonBool{object.has_large_media_});
  jo("show_large_media", JsonBool{object.show_large_media_});
  jo("show_media_above_description", JsonBool{object.show_media_above_description_});
  jo("skip_confirmation", JsonBool{object.skip_confirmation_});
  jo("show_above_text", JsonBool{object.show_above_text_});
  jo("instant_view_version", object.instant_view_version_);
}

void to_json(JsonValueScope &jv, const td_api::logTags &object) {
  auto jo = jv.enter_object();
  jo("@type", "logTags");
  jo("tags", ToJson(object.tags_));
}

void to_json(JsonValueScope &jv, const td_api::messageCalendarDay &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageCalendarDay");
  jo("total_count", object.total_count_);
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageLink");
  jo("link", object.link_);
  jo("is_public", JsonBool{object.is_public_});
}

void to_json(JsonValueScope &jv, const td_api::messageReplyInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReplyInfo");
  jo("reply_count", object.reply_count_);
  jo("recent_replier_ids", ToJson(object.recent_replier_ids_));
  jo("last_read_inbox_message_id", object.last_read_inbox_message_id_);
  jo("last_read_outbox_message_id", object.last_read_outbox_message_id_);
  jo("last_message_id", object.last_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::MessageTopic &object) {
  td_api::downcast_call(const_cast<td_api::MessageTopic &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageTopicThread &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageTopicThread");
  jo("message_thread_id", object.message_thread_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageTopicForum &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageTopicForum");
  jo("forum_topic_id", object.forum_topic_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageTopicDirectMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageTopicDirectMessages");
  jo("direct_messages_chat_topic_id", object.direct_messages_chat_topic_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageTopicSavedMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageTopicSavedMessages");
  jo("saved_messages_topic_id", object.saved_messages_topic_id_);
}

void to_json(JsonValueScope &jv, const td_api::notification &object) {
  auto jo = jv.enter_object();
  jo("@type", "notification");
  jo("id", object.id_);
  jo("date", object.date_);
  jo("is_silent", JsonBool{object.is_silent_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::orderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "orderInfo");
  jo("name", object.name_);
  jo("phone_number", object.phone_number_);
  jo("email_address", object.email_address_);
  if (object.shipping_address_) {
    jo("shipping_address", ToJson(*object.shipping_address_));
  }
}

void to_json(JsonValueScope &jv, const td_api::PaidMedia &object) {
  td_api::downcast_call(const_cast<td_api::PaidMedia &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::paidMediaPreview &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidMediaPreview");
  jo("width", object.width_);
  jo("height", object.height_);
  jo("duration", object.duration_);
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
}

void to_json(JsonValueScope &jv, const td_api::paidMediaPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidMediaPhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::paidMediaVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidMediaVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.cover_) {
    jo("cover", ToJson(*object.cover_));
  }
  jo("start_timestamp", object.start_timestamp_);
}

void to_json(JsonValueScope &jv, const td_api::paidMediaUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidMediaUnsupported");
}

void to_json(JsonValueScope &jv, const td_api::passportElementsWithErrors &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementsWithErrors");
  jo("elements", ToJson(object.elements_));
  jo("errors", ToJson(object.errors_));
}

void to_json(JsonValueScope &jv, const td_api::PaymentReceiptType &object) {
  td_api::downcast_call(const_cast<td_api::PaymentReceiptType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::paymentReceiptTypeRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentReceiptTypeRegular");
  jo("payment_provider_user_id", object.payment_provider_user_id_);
  if (object.invoice_) {
    jo("invoice", ToJson(*object.invoice_));
  }
  if (object.order_info_) {
    jo("order_info", ToJson(*object.order_info_));
  }
  if (object.shipping_option_) {
    jo("shipping_option", ToJson(*object.shipping_option_));
  }
  jo("credentials_title", object.credentials_title_);
  jo("tip_amount", object.tip_amount_);
}

void to_json(JsonValueScope &jv, const td_api::paymentReceiptTypeStars &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentReceiptTypeStars");
  jo("star_count", object.star_count_);
  jo("transaction_id", object.transaction_id_);
}

void to_json(JsonValueScope &jv, const td_api::pollOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "pollOption");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("voter_count", object.voter_count_);
  jo("vote_percentage", object.vote_percentage_);
  jo("is_chosen", JsonBool{object.is_chosen_});
  jo("is_being_chosen", JsonBool{object.is_being_chosen_});
}

void to_json(JsonValueScope &jv, const td_api::premiumGiveawayPaymentOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumGiveawayPaymentOptions");
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::productInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "productInfo");
  jo("title", object.title_);
  if (object.description_) {
    jo("description", ToJson(*object.description_));
  }
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::publicForwards &object) {
  auto jo = jv.enter_object();
  jo("@type", "publicForwards");
  jo("total_count", object.total_count_);
  jo("forwards", ToJson(object.forwards_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::ReactionType &object) {
  td_api::downcast_call(const_cast<td_api::ReactionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::reactionTypeEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionTypeEmoji");
  jo("emoji", object.emoji_);
}

void to_json(JsonValueScope &jv, const td_api::reactionTypeCustomEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionTypeCustomEmoji");
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::reactionTypePaid &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionTypePaid");
}

void to_json(JsonValueScope &jv, const td_api::ReplyMarkup &object) {
  td_api::downcast_call(const_cast<td_api::ReplyMarkup &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::replyMarkupRemoveKeyboard &object) {
  auto jo = jv.enter_object();
  jo("@type", "replyMarkupRemoveKeyboard");
  jo("is_personal", JsonBool{object.is_personal_});
}

void to_json(JsonValueScope &jv, const td_api::replyMarkupForceReply &object) {
  auto jo = jv.enter_object();
  jo("@type", "replyMarkupForceReply");
  jo("is_personal", JsonBool{object.is_personal_});
  jo("input_field_placeholder", object.input_field_placeholder_);
}

void to_json(JsonValueScope &jv, const td_api::replyMarkupShowKeyboard &object) {
  auto jo = jv.enter_object();
  jo("@type", "replyMarkupShowKeyboard");
  jo("rows", ToJson(object.rows_));
  jo("is_persistent", JsonBool{object.is_persistent_});
  jo("resize_keyboard", JsonBool{object.resize_keyboard_});
  jo("one_time", JsonBool{object.one_time_});
  jo("is_personal", JsonBool{object.is_personal_});
  jo("input_field_placeholder", object.input_field_placeholder_);
}

void to_json(JsonValueScope &jv, const td_api::replyMarkupInlineKeyboard &object) {
  auto jo = jv.enter_object();
  jo("@type", "replyMarkupInlineKeyboard");
  jo("rows", ToJson(object.rows_));
}

void to_json(JsonValueScope &jv, const td_api::rtmpUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "rtmpUrl");
  jo("url", object.url_);
  jo("stream_key", object.stream_key_);
}

void to_json(JsonValueScope &jv, const td_api::secretChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "secretChat");
  jo("id", object.id_);
  jo("user_id", object.user_id_);
  if (object.state_) {
    jo("state", ToJson(*object.state_));
  }
  jo("is_outbound", JsonBool{object.is_outbound_});
  jo("key_hash", base64_encode(object.key_hash_));
  jo("layer", object.layer_);
}

void to_json(JsonValueScope &jv, const td_api::shippingOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "shippingOption");
  jo("id", object.id_);
  jo("title", object.title_);
  jo("price_parts", ToJson(object.price_parts_));
}

void to_json(JsonValueScope &jv, const td_api::starGiveawayPaymentOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starGiveawayPaymentOptions");
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::starSubscriptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscriptions");
  if (object.star_amount_) {
    jo("star_amount", ToJson(*object.star_amount_));
  }
  jo("subscriptions", ToJson(object.subscriptions_));
  jo("required_star_count", object.required_star_count_);
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::stickerSet &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerSet");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("title", object.title_);
  jo("name", object.name_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.thumbnail_outline_) {
    jo("thumbnail_outline", ToJson(*object.thumbnail_outline_));
  }
  jo("is_owned", JsonBool{object.is_owned_});
  jo("is_installed", JsonBool{object.is_installed_});
  jo("is_archived", JsonBool{object.is_archived_});
  jo("is_official", JsonBool{object.is_official_});
  if (object.sticker_type_) {
    jo("sticker_type", ToJson(*object.sticker_type_));
  }
  jo("needs_repainting", JsonBool{object.needs_repainting_});
  jo("is_allowed_as_chat_emoji_status", JsonBool{object.is_allowed_as_chat_emoji_status_});
  jo("is_viewed", JsonBool{object.is_viewed_});
  jo("stickers", ToJson(object.stickers_));
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::stories &object) {
  auto jo = jv.enter_object();
  jo("@type", "stories");
  jo("total_count", object.total_count_);
  jo("stories", ToJson(object.stories_));
  jo("pinned_story_ids", ToJson(object.pinned_story_ids_));
}

void to_json(JsonValueScope &jv, const td_api::storyInteraction &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteraction");
  if (object.actor_id_) {
    jo("actor_id", ToJson(*object.actor_id_));
  }
  jo("interaction_date", object.interaction_date_);
  if (object.block_list_) {
    jo("block_list", ToJson(*object.block_list_));
  }
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storyVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyVideo");
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
  jo("has_stickers", JsonBool{object.has_stickers_});
  jo("is_animation", JsonBool{object.is_animation_});
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("preload_prefix_size", object.preload_prefix_size_);
  jo("cover_frame_timestamp", object.cover_frame_timestamp_);
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
}

void to_json(JsonValueScope &jv, const td_api::TMeUrlType &object) {
  td_api::downcast_call(const_cast<td_api::TMeUrlType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrlTypeUser");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeSupergroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrlTypeSupergroup");
  jo("supergroup_id", object.supergroup_id_);
}

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeChatInvite &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrlTypeChatInvite");
  if (object.info_) {
    jo("info", ToJson(*object.info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::tMeUrlTypeStickerSet &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrlTypeStickerSet");
  jo("sticker_set_id", ToJson(JsonInt64{object.sticker_set_id_}));
}

void to_json(JsonValueScope &jv, const td_api::testVectorInt &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorInt");
  jo("value", ToJson(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::themeSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "themeSettings");
  if (object.base_theme_) {
    jo("base_theme", ToJson(*object.base_theme_));
  }
  jo("accent_color", object.accent_color_);
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  if (object.outgoing_message_fill_) {
    jo("outgoing_message_fill", ToJson(*object.outgoing_message_fill_));
  }
  jo("animate_outgoing_message_fill", JsonBool{object.animate_outgoing_message_fill_});
  jo("outgoing_message_accent_color", object.outgoing_message_accent_color_);
}

void to_json(JsonValueScope &jv, const td_api::tonTransactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonTransactions");
  jo("ton_amount", object.ton_amount_);
  jo("transactions", ToJson(object.transactions_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftBackdropColors &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftBackdropColors");
  jo("center_color", object.center_color_);
  jo("edge_color", object.edge_color_);
  jo("symbol_color", object.symbol_color_);
  jo("text_color", object.text_color_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftValueInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftValueInfo");
  jo("currency", object.currency_);
  jo("value", object.value_);
  jo("is_value_average", JsonBool{object.is_value_average_});
  jo("initial_sale_date", object.initial_sale_date_);
  jo("initial_sale_star_count", object.initial_sale_star_count_);
  jo("initial_sale_price", object.initial_sale_price_);
  jo("last_sale_date", object.last_sale_date_);
  jo("last_sale_price", object.last_sale_price_);
  jo("is_last_sale_on_fragment", JsonBool{object.is_last_sale_on_fragment_});
  jo("minimum_price", object.minimum_price_);
  jo("average_sale_price", object.average_sale_price_);
  jo("telegram_listed_gift_count", object.telegram_listed_gift_count_);
  jo("fragment_listed_gift_count", object.fragment_listed_gift_count_);
  jo("fragment_url", object.fragment_url_);
}

void to_json(JsonValueScope &jv, const td_api::userSupportInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "userSupportInfo");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("author", object.author_);
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::videoChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoChat");
  jo("group_call_id", object.group_call_id_);
  jo("has_participants", JsonBool{object.has_participants_});
  if (object.default_participant_id_) {
    jo("default_participant_id", ToJson(*object.default_participant_id_));
  }
}

void to_json(JsonValueScope &jv, const td_api::webAppInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "webAppInfo");
  jo("launch_id", ToJson(JsonInt64{object.launch_id_}));
  jo("url", object.url_);
}

}  // namespace td_api
}  // namespace td
