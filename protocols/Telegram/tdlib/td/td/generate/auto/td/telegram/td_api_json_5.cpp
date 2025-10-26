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
Result<int32> tl_constructor_from_string(td_api::BackgroundFill *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"backgroundFillSolid", 1010678813},
    {"backgroundFillGradient", -1839206017},
    {"backgroundFillFreeformGradient", -1145469255}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ChatAvailableReactions *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatAvailableReactionsAll", 694160279},
    {"chatAvailableReactionsSome", 152513153}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::EmojiStatusType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"emojiStatusTypeCustomEmoji", -1666780939},
    {"emojiStatusTypeUpgradedGift", -837921804}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputBackground *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputBackgroundLocal", -1747094364},
    {"inputBackgroundRemote", -274976231},
    {"inputBackgroundPrevious", -351905954}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputMessageReplyTo *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputMessageReplyToMessage", -782038760},
    {"inputMessageReplyToExternalMessage", -505276703},
    {"inputMessageReplyToStory", -1723842320}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::LanguagePackStringValue *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"languagePackStringValueOrdinary", -249256352},
    {"languagePackStringValuePluralized", 1906840261},
    {"languagePackStringValueDeleted", 1834792698}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::NetworkType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"networkTypeNone", -1971691759},
    {"networkTypeMobile", 819228239},
    {"networkTypeMobileRoaming", -1435199760},
    {"networkTypeWiFi", -633872070},
    {"networkTypeOther", 1942128539}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PremiumSource *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"premiumSourceLimitExceeded", -2052159742},
    {"premiumSourceFeature", 445813541},
    {"premiumSourceBusinessFeature", -1492946340},
    {"premiumSourceStoryFeature", -1030737556},
    {"premiumSourceLink", 2135071132},
    {"premiumSourceSettings", -285702859}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ResendCodeReason *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"resendCodeReasonUserRequest", -441923456},
    {"resendCodeReasonVerificationFailed", 529870273}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::SuggestedAction *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"suggestedActionEnableArchiveAndMuteNewChats", 2017586255},
    {"suggestedActionCheckPassword", 1910534839},
    {"suggestedActionCheckPhoneNumber", 648771563},
    {"suggestedActionViewChecksHint", 891303239},
    {"suggestedActionConvertToBroadcastGroup", -965071304},
    {"suggestedActionSetPassword", 1863613848},
    {"suggestedActionUpgradePremium", 1890220539},
    {"suggestedActionRestorePremium", -385229468},
    {"suggestedActionSubscribeToAnnualPremium", 373913787},
    {"suggestedActionGiftPremiumForChristmas", -1816924561},
    {"suggestedActionSetBirthdate", -356672766},
    {"suggestedActionSetProfilePhoto", -1612563093},
    {"suggestedActionExtendPremium", -566207286},
    {"suggestedActionExtendStarSubscriptions", -47000234},
    {"suggestedActionCustom", 2092876611}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::UpgradedGiftAttributeId *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"upgradedGiftAttributeIdModel", 1053287307},
    {"upgradedGiftAttributeIdSymbol", 1188205608},
    {"upgradedGiftAttributeIdBackdrop", 1461997935}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::affiliateProgramSortOrderCreationDate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::autosaveSettingsScopeChannelChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::birthdate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.day_, from.extract_field("day")));
  TRY_STATUS(from_json(to.month_, from.extract_field("month")));
  TRY_STATUS(from_json(to.year_, from.extract_field("year")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChatAdministrators &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureChatFolderTags &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemInterruptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadDataWithPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::chatActionChoosingSticker &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatAvailableReactionsAll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.max_reaction_count_, from.extract_field("max_reaction_count")));
  return Status::OK();
}

Status from_json(td_api::chatListArchive &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatPhotoSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.background_fill_, from.extract_field("background_fill")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenApplePushVoIP &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, from.extract_field("is_app_sandbox")));
  TRY_STATUS(from_json(to.encrypt_, from.extract_field("encrypt")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenHuaweiPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  TRY_STATUS(from_json(to.encrypt_, from.extract_field("encrypt")));
  return Status::OK();
}

Status from_json(td_api::emojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.expiration_date_, from.extract_field("expiration_date")));
  return Status::OK();
}

Status from_json(td_api::fileTypePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSelfDestructingVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::firebaseAuthenticationSettingsAndroid &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::giftSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.show_gift_button_, from.extract_field("show_gift_button")));
  TRY_STATUS(from_json(to.accepted_gift_types_, from.extract_field("accepted_gift_types")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::inputBusinessStartPage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsSaved &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_credentials_id_, from.extract_field("saved_credentials_id")));
  return Status::OK();
}

Status from_json(td_api::inputGroupCallMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.photo_url_, from.extract_field("photo_url")));
  TRY_STATUS(from_json(to.photo_width_, from.extract_field("photo_width")));
  TRY_STATUS(from_json(to.photo_height_, from.extract_field("photo_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputMessageAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, from.extract_field("animation")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, from.extract_field("added_sticker_file_ids")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
  TRY_STATUS(from_json(to.has_spoiler_, from.extract_field("has_spoiler")));
  return Status::OK();
}

Status from_json(td_api::inputMessageLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.live_period_, from.extract_field("live_period")));
  TRY_STATUS(from_json(to.heading_, from.extract_field("heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, from.extract_field("proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::inputMessageForwarded &to, JsonObject &from) {
  TRY_STATUS(from_json(to.from_chat_id_, from.extract_field("from_chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.in_game_share_, from.extract_field("in_game_share")));
  TRY_STATUS(from_json(to.replace_video_start_timestamp_, from.extract_field("replace_video_start_timestamp")));
  TRY_STATUS(from_json(to.new_video_start_timestamp_, from.extract_field("new_video_start_timestamp")));
  TRY_STATUS(from_json(to.copy_options_, from.extract_field("copy_options")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementDriverLicense &to, JsonObject &from) {
  TRY_STATUS(from_json(to.driver_license_, from.extract_field("driver_license")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceTranslationFiles &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hashes_, from.extract_field("file_hashes")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeSuggestedReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  TRY_STATUS(from_json(to.is_flipped_, from.extract_field("is_flipped")));
  return Status::OK();
}

Status from_json(td_api::inputTextQuote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.position_, from.extract_field("position")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBusinessChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_name_, from.extract_field("link_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeDirectMessagesChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.channel_username_, from.extract_field("channel_username")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMainWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.mode_, from.extract_field("mode")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePremiumGiftCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeStoryAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_album_owner_username_, from.extract_field("story_album_owner_username")));
  TRY_STATUS(from_json(to.story_album_id_, from.extract_field("story_album_id")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.web_app_short_name_, from.extract_field("web_app_short_name")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.mode_, from.extract_field("mode")));
  return Status::OK();
}

Status from_json(td_api::keyboardButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::languagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.base_language_pack_id_, from.extract_field("base_language_pack_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.native_name_, from.extract_field("native_name")));
  TRY_STATUS(from_json(to.plural_code_, from.extract_field("plural_code")));
  TRY_STATUS(from_json(to.is_official_, from.extract_field("is_official")));
  TRY_STATUS(from_json(to.is_rtl_, from.extract_field("is_rtl")));
  TRY_STATUS(from_json(to.is_beta_, from.extract_field("is_beta")));
  TRY_STATUS(from_json(to.is_installed_, from.extract_field("is_installed")));
  TRY_STATUS(from_json(to.total_string_count_, from.extract_field("total_string_count")));
  TRY_STATUS(from_json(to.translated_string_count_, from.extract_field("translated_string_count")));
  TRY_STATUS(from_json(to.local_string_count_, from.extract_field("local_string_count")));
  TRY_STATUS(from_json(to.translation_url_, from.extract_field("translation_url")));
  return Status::OK();
}

Status from_json(td_api::logStreamFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.path_, from.extract_field("path")));
  TRY_STATUS(from_json(to.max_file_size_, from.extract_field("max_file_size")));
  TRY_STATUS(from_json(to.redirect_stderr_, from.extract_field("redirect_stderr")));
  return Status::OK();
}

Status from_json(td_api::messageSchedulingStateSendAtDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.send_date_, from.extract_field("send_date")));
  return Status::OK();
}

Status from_json(td_api::messageSourceMessageThreadHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceOther &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkTypeMobileRoaming &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optionValueInteger &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::passportElementTypeIdentityCard &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeEmailAddress &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureIncreasedUploadFileSize &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureEmojiStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureSavedMessagesTags &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeSavedAnimationCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeShareableChatFolderCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumSourceBusinessFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureLinksAndFormatting &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::profileTabGifs &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSourceAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonSpam &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonCustom &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterAnimation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerFormatWebp &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposePremiumGiveaway &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  return Status::OK();
}

Status from_json(td_api::storyListArchive &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionConvertToBroadcastGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionExtendStarSubscriptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterBanned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposePremiumGiveaway &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.winner_count_, from.extract_field("winner_count")));
  TRY_STATUS(from_json(to.month_count_, from.extract_field("month_count")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeHashtag &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeUnderline &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMentionName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::topChatCategoryChannels &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::upgradedGiftAttributeIdBackdrop &to, JsonObject &from) {
  TRY_STATUS(from_json(to.backdrop_id_, from.extract_field("backdrop_id")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowCalls &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowPremiumUsers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::venue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  TRY_STATUS(from_json(to.provider_, from.extract_field("provider")));
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::addChatFolderByInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::addGiftCollectionGifts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.collection_id_, from.extract_field("collection_id")));
  TRY_STATUS(from_json(to.received_gift_ids_, from.extract_field("received_gift_ids")));
  return Status::OK();
}

Status from_json(td_api::addQuickReplyShortcutInlineQueryResultMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_name_, from.extract_field("shortcut_name")));
  TRY_STATUS(from_json(to.reply_to_message_id_, from.extract_field("reply_to_message_id")));
  TRY_STATUS(from_json(to.query_id_, from.extract_field("query_id")));
  TRY_STATUS(from_json(to.result_id_, from.extract_field("result_id")));
  TRY_STATUS(from_json(to.hide_via_bot_, from.extract_field("hide_via_bot")));
  return Status::OK();
}

Status from_json(td_api::allowBotToSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::approveSuggestedPost &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.send_date_, from.extract_field("send_date")));
  return Status::OK();
}

Status from_json(td_api::canPurchaseFromStore &to, JsonObject &from) {
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::changeStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, from.extract_field("set_id")));
  TRY_STATUS(from_json(to.is_installed_, from.extract_field("is_installed")));
  TRY_STATUS(from_json(to.is_archived_, from.extract_field("is_archived")));
  return Status::OK();
}

Status from_json(td_api::checkChatUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::checkStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::clearRecentlyFoundChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::closeStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::createChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::createNewSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::declineGroupCallInvitation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::deleteBusinessMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteChatReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::deleteMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedOrderInfo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::disconnectAllWebsites &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::editBusinessMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.edit_icon_custom_emoji_, from.extract_field("edit_icon_custom_emoji")));
  TRY_STATUS(from_json(to.icon_custom_emoji_id_, from.extract_field("icon_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::editMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editUserStarSubscription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.telegram_payment_charge_id_, from.extract_field("telegram_payment_charge_id")));
  TRY_STATUS(from_json(to.is_canceled_, from.extract_field("is_canceled")));
  return Status::OK();
}

Status from_json(td_api::getActiveSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAttachmentMenuBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::getBasicGroupFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, from.extract_field("basic_group_id")));
  return Status::OK();
}

Status from_json(td_api::getBusinessAccountStarAmount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  return Status::OK();
}

Status from_json(td_api::getChatActiveStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageCalendar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  return Status::OK();
}

Status from_json(td_api::getChatScheduledMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatsForChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getCountries &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDeepLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::getDirectMessagesChatTopicMessageByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  return Status::OK();
}

Status from_json(td_api::getFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::getGameHighScores &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getGroupsInCommon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.offset_chat_id_, from.extract_field("offset_chat_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getJsonString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.json_value_, from.extract_field("json_value")));
  return Status::OK();
}

Status from_json(td_api::getLogTagVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  return Status::OK();
}

Status from_json(td_api::getMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.media_timestamp_, from.extract_field("media_timestamp")));
  TRY_STATUS(from_json(to.for_album_, from.extract_field("for_album")));
  TRY_STATUS(from_json(to.in_message_thread_, from.extract_field("in_message_thread")));
  return Status::OK();
}

Status from_json(td_api::getMessageViewers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getPassportAuthorizationFormAvailableElements &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_form_id_, from.extract_field("authorization_form_id")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getPremiumFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  return Status::OK();
}

Status from_json(td_api::getProxies &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecentStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  return Status::OK();
}

Status from_json(td_api::getSavedMessagesTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::getSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, from.extract_field("secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::getStatisticalGraph &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::getStoryAlbumStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_album_id_, from.extract_field("story_album_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getSuitablePersonalChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getThemedChatEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getUpgradedGiftEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getUserProfilePhotos &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getWebAppPlaceholder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::inviteVideoChatParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::loadActiveStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_list_, from.extract_field("story_list")));
  return Status::OK();
}

Status from_json(td_api::openBotSimilarBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.opened_bot_user_id_, from.extract_field("opened_bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::parseTextEntities &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.parse_mode_, from.extract_field("parse_mode")));
  return Status::OK();
}

Status from_json(td_api::rateSpeechRecognition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_good_, from.extract_field("is_good")));
  return Status::OK();
}

Status from_json(td_api::readdQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_name_, from.extract_field("shortcut_name")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::removeChatActionBar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::removeNotificationGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_group_id_, from.extract_field("notification_group_id")));
  TRY_STATUS(from_json(to.max_notification_id_, from.extract_field("max_notification_id")));
  return Status::OK();
}

Status from_json(td_api::removeSearchedForTag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  return Status::OK();
}

Status from_json(td_api::reorderGiftCollections &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.collection_ids_, from.extract_field("collection_ids")));
  return Status::OK();
}

Status from_json(td_api::reportAuthenticationCodeMissing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mobile_network_code_, from.extract_field("mobile_network_code")));
  return Status::OK();
}

Status from_json(td_api::reportSupergroupSpam &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::resendPhoneNumberCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  return Status::OK();
}

Status from_json(td_api::revokeGroupCallInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::searchChatRecentLocationMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.chat_type_filter_, from.extract_field("chat_type_filter")));
  TRY_STATUS(from_json(to.min_date_, from.extract_field("min_date")));
  TRY_STATUS(from_json(to.max_date_, from.extract_field("max_date")));
  return Status::OK();
}

Status from_json(td_api::searchQuote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  TRY_STATUS(from_json(to.quote_position_, from.extract_field("quote_position")));
  return Status::OK();
}

Status from_json(td_api::searchUserByToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::sendCallRating &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.rating_, from.extract_field("rating")));
  TRY_STATUS(from_json(to.comment_, from.extract_field("comment")));
  TRY_STATUS(from_json(to.problems_, from.extract_field("problems")));
  return Status::OK();
}

Status from_json(td_api::sendMessageAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.input_message_contents_, from.extract_field("input_message_contents")));
  return Status::OK();
}

Status from_json(td_api::sendWebAppData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.button_text_, from.extract_field("button_text")));
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::setAutoDownloadSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::setBusinessAccountBio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.bio_, from.extract_field("bio")));
  return Status::OK();
}

Status from_json(td_api::setBusinessMessageIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::setChatDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  return Status::OK();
}

Status from_json(td_api::setChatNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setCloseFriends &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::setDefaultMessageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_auto_delete_time_, from.extract_field("message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::setGiftSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setMainProfileTab &to, JsonObject &from) {
  TRY_STATUS(from_json(to.main_profile_tab_, from.extract_field("main_profile_tab")));
  return Status::OK();
}

Status from_json(td_api::setOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::setPinnedSavedMessagesTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_ids_, from.extract_field("saved_messages_topic_ids")));
  return Status::OK();
}

Status from_json(td_api::setSavedMessagesTagLabel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.label_, from.extract_field("label")));
  return Status::OK();
}

Status from_json(td_api::setStoryPrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.privacy_settings_, from.extract_field("privacy_settings")));
  return Status::OK();
}

Status from_json(td_api::setUserEmojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.emoji_status_, from.extract_field("emoji_status")));
  return Status::OK();
}

Status from_json(td_api::sharePhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::synchronizeLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorStringObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::toggleBotIsAddedToAttachmentMenu &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.is_added_, from.extract_field("is_added")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsTranslatable &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_translatable_, from.extract_field("is_translatable")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallIsMyVideoEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.is_my_video_enabled_, from.extract_field("is_my_video_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleStoryIsPostedToChatPage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.is_posted_to_chat_page_, from.extract_field("is_posted_to_chat_page")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupJoinToSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.join_to_send_messages_, from.extract_field("join_to_send_messages")));
  return Status::OK();
}

Status from_json(td_api::translateMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.to_language_code_, from.extract_field("to_language_code")));
  return Status::OK();
}

Status from_json(td_api::validateOrderInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, from.extract_field("input_invoice")));
  TRY_STATUS(from_json(to.order_info_, from.extract_field("order_info")));
  TRY_STATUS(from_json(to.allow_save_, from.extract_field("allow_save")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::accountInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "accountInfo");
  jo("registration_month", object.registration_month_);
  jo("registration_year", object.registration_year_);
  jo("phone_number_country_code", object.phone_number_country_code_);
  jo("last_name_change_date", object.last_name_change_date_);
  jo("last_photo_change_date", object.last_photo_change_date_);
}

void to_json(JsonValueScope &jv, const td_api::ageVerificationParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "ageVerificationParameters");
  jo("min_age", object.min_age_);
  jo("verification_bot_username", object.verification_bot_username_);
  jo("country", object.country_);
}

void to_json(JsonValueScope &jv, const td_api::audio &object) {
  auto jo = jv.enter_object();
  jo("@type", "audio");
  jo("duration", object.duration_);
  jo("title", object.title_);
  jo("performer", object.performer_);
  jo("file_name", object.file_name_);
  jo("mime_type", object.mime_type_);
  if (object.album_cover_minithumbnail_) {
    jo("album_cover_minithumbnail", ToJson(*object.album_cover_minithumbnail_));
  }
  if (object.album_cover_thumbnail_) {
    jo("album_cover_thumbnail", ToJson(*object.album_cover_thumbnail_));
  }
  jo("external_album_covers", ToJson(object.external_album_covers_));
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
}

void to_json(JsonValueScope &jv, const td_api::AutosaveSettingsScope &object) {
  td_api::downcast_call(const_cast<td_api::AutosaveSettingsScope &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::autosaveSettingsScopePrivateChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "autosaveSettingsScopePrivateChats");
}

void to_json(JsonValueScope &jv, const td_api::autosaveSettingsScopeGroupChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "autosaveSettingsScopeGroupChats");
}

void to_json(JsonValueScope &jv, const td_api::autosaveSettingsScopeChannelChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "autosaveSettingsScopeChannelChats");
}

void to_json(JsonValueScope &jv, const td_api::autosaveSettingsScopeChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "autosaveSettingsScopeChat");
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::bankCardActionOpenUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "bankCardActionOpenUrl");
  jo("text", object.text_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::botMediaPreview &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMediaPreview");
  jo("date", object.date_);
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
}

void to_json(JsonValueScope &jv, const td_api::businessAwayMessageSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessAwayMessageSettings");
  jo("shortcut_id", object.shortcut_id_);
  if (object.recipients_) {
    jo("recipients", ToJson(*object.recipients_));
  }
  if (object.schedule_) {
    jo("schedule", ToJson(*object.schedule_));
  }
  jo("offline_only", JsonBool{object.offline_only_});
}

void to_json(JsonValueScope &jv, const td_api::businessFeaturePromotionAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeaturePromotionAnimation");
  if (object.feature_) {
    jo("feature", ToJson(*object.feature_));
  }
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::businessRecipients &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessRecipients");
  jo("chat_ids", ToJson(object.chat_ids_));
  jo("excluded_chat_ids", ToJson(object.excluded_chat_ids_));
  jo("select_existing_chats", JsonBool{object.select_existing_chats_});
  jo("select_new_chats", JsonBool{object.select_new_chats_});
  jo("select_contacts", JsonBool{object.select_contacts_});
  jo("select_non_contacts", JsonBool{object.select_non_contacts_});
  jo("exclude_selected", JsonBool{object.exclude_selected_});
}

void to_json(JsonValueScope &jv, const td_api::callbackQueryAnswer &object) {
  auto jo = jv.enter_object();
  jo("@type", "callbackQueryAnswer");
  jo("text", object.text_);
  jo("show_alert", JsonBool{object.show_alert_});
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::chatActiveStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActiveStories");
  jo("chat_id", object.chat_id_);
  if (object.list_) {
    jo("list", ToJson(*object.list_));
  }
  jo("order", object.order_);
  jo("can_be_archived", JsonBool{object.can_be_archived_});
  jo("max_read_story_id", object.max_read_story_id_);
  jo("stories", ToJson(object.stories_));
}

void to_json(JsonValueScope &jv, const td_api::chatBoostLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostLink");
  jo("link", object.link_);
  jo("is_public", JsonBool{object.is_public_});
}

void to_json(JsonValueScope &jv, const td_api::chatFolder &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolder");
  if (object.name_) {
    jo("name", ToJson(*object.name_));
  }
  if (object.icon_) {
    jo("icon", ToJson(*object.icon_));
  }
  jo("color_id", object.color_id_);
  jo("is_shareable", JsonBool{object.is_shareable_});
  jo("pinned_chat_ids", ToJson(object.pinned_chat_ids_));
  jo("included_chat_ids", ToJson(object.included_chat_ids_));
  jo("excluded_chat_ids", ToJson(object.excluded_chat_ids_));
  jo("exclude_muted", JsonBool{object.exclude_muted_});
  jo("exclude_read", JsonBool{object.exclude_read_});
  jo("exclude_archived", JsonBool{object.exclude_archived_});
  jo("include_contacts", JsonBool{object.include_contacts_});
  jo("include_non_contacts", JsonBool{object.include_non_contacts_});
  jo("include_bots", JsonBool{object.include_bots_});
  jo("include_groups", JsonBool{object.include_groups_});
  jo("include_channels", JsonBool{object.include_channels_});
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkCounts &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkCounts");
  jo("invite_link_counts", ToJson(object.invite_link_counts_));
}

void to_json(JsonValueScope &jv, const td_api::ChatList &object) {
  td_api::downcast_call(const_cast<td_api::ChatList &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatListMain &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatListMain");
}

void to_json(JsonValueScope &jv, const td_api::chatListArchive &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatListArchive");
}

void to_json(JsonValueScope &jv, const td_api::chatListFolder &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatListFolder");
  jo("chat_folder_id", object.chat_folder_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatPermissions &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPermissions");
  jo("can_send_basic_messages", JsonBool{object.can_send_basic_messages_});
  jo("can_send_audios", JsonBool{object.can_send_audios_});
  jo("can_send_documents", JsonBool{object.can_send_documents_});
  jo("can_send_photos", JsonBool{object.can_send_photos_});
  jo("can_send_videos", JsonBool{object.can_send_videos_});
  jo("can_send_video_notes", JsonBool{object.can_send_video_notes_});
  jo("can_send_voice_notes", JsonBool{object.can_send_voice_notes_});
  jo("can_send_polls", JsonBool{object.can_send_polls_});
  jo("can_send_other_messages", JsonBool{object.can_send_other_messages_});
  jo("can_add_link_previews", JsonBool{object.can_add_link_previews_});
  jo("can_change_info", JsonBool{object.can_change_info_});
  jo("can_invite_users", JsonBool{object.can_invite_users_});
  jo("can_pin_messages", JsonBool{object.can_pin_messages_});
  jo("can_create_topics", JsonBool{object.can_create_topics_});
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransaction");
  jo("cryptocurrency", object.cryptocurrency_);
  jo("cryptocurrency_amount", ToJson(JsonInt64{object.cryptocurrency_amount_}));
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ChatStatisticsObjectType &object) {
  td_api::downcast_call(const_cast<td_api::ChatStatisticsObjectType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsObjectTypeMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsObjectTypeMessage");
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsObjectTypeStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsObjectTypeStory");
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::closedVectorPath &object) {
  auto jo = jv.enter_object();
  jo("@type", "closedVectorPath");
  jo("commands", ToJson(object.commands_));
}

void to_json(JsonValueScope &jv, const td_api::countries &object) {
  auto jo = jv.enter_object();
  jo("@type", "countries");
  jo("countries", ToJson(object.countries_));
}

void to_json(JsonValueScope &jv, const td_api::datedFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "datedFile");
  if (object.file_) {
    jo("file", ToJson(*object.file_));
  }
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::emojiCategories &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiCategories");
  jo("categories", ToJson(object.categories_));
}

void to_json(JsonValueScope &jv, const td_api::EmojiStatusType &object) {
  td_api::downcast_call(const_cast<td_api::EmojiStatusType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::emojiStatusTypeCustomEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatusTypeCustomEmoji");
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::emojiStatusTypeUpgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatusTypeUpgradedGift");
  jo("upgraded_gift_id", ToJson(JsonInt64{object.upgraded_gift_id_}));
  jo("gift_title", object.gift_title_);
  jo("gift_name", object.gift_name_);
  jo("model_custom_emoji_id", ToJson(JsonInt64{object.model_custom_emoji_id_}));
  jo("symbol_custom_emoji_id", ToJson(JsonInt64{object.symbol_custom_emoji_id_}));
  if (object.backdrop_colors_) {
    jo("backdrop_colors", ToJson(*object.backdrop_colors_));
  }
}

void to_json(JsonValueScope &jv, const td_api::file &object) {
  auto jo = jv.enter_object();
  jo("@type", "file");
  jo("id", object.id_);
  jo("size", object.size_);
  jo("expected_size", object.expected_size_);
  if (object.local_) {
    jo("local", ToJson(*object.local_));
  }
  if (object.remote_) {
    jo("remote", ToJson(*object.remote_));
  }
}

void to_json(JsonValueScope &jv, const td_api::forumTopics &object) {
  auto jo = jv.enter_object();
  jo("@type", "forumTopics");
  jo("total_count", object.total_count_);
  jo("topics", ToJson(object.topics_));
  jo("next_offset_date", object.next_offset_date_);
  jo("next_offset_message_id", object.next_offset_message_id_);
  jo("next_offset_forum_topic_id", object.next_offset_forum_topic_id_);
}

void to_json(JsonValueScope &jv, const td_api::foundPositions &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundPositions");
  jo("total_count", object.total_count_);
  jo("positions", ToJson(object.positions_));
}

void to_json(JsonValueScope &jv, const td_api::giftChatTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftChatTheme");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  if (object.light_settings_) {
    jo("light_settings", ToJson(*object.light_settings_));
  }
  if (object.dark_settings_) {
    jo("dark_settings", ToJson(*object.dark_settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::giftSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftSettings");
  jo("show_gift_button", JsonBool{object.show_gift_button_});
  if (object.accepted_gift_types_) {
    jo("accepted_gift_types", ToJson(*object.accepted_gift_types_));
  }
}

void to_json(JsonValueScope &jv, const td_api::groupCallId &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallId");
  jo("id", object.id_);
}

void to_json(JsonValueScope &jv, const td_api::identityDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "identityDocument");
  jo("number", object.number_);
  if (object.expiration_date_) {
    jo("expiration_date", ToJson(*object.expiration_date_));
  }
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
}

void to_json(JsonValueScope &jv, const td_api::inputChecklistTask &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputChecklistTask");
  jo("id", object.id_);
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::InternalLinkType &object) {
  td_api::downcast_call(const_cast<td_api::InternalLinkType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeActiveSessions &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeActiveSessions");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeAttachmentMenuBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeAttachmentMenuBot");
  if (object.target_chat_) {
    jo("target_chat", ToJson(*object.target_chat_));
  }
  jo("bot_username", object.bot_username_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeAuthenticationCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeAuthenticationCode");
  jo("code", object.code_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeBackground");
  jo("background_name", object.background_name_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBotAddToChannel &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeBotAddToChannel");
  jo("bot_username", object.bot_username_);
  if (object.administrator_rights_) {
    jo("administrator_rights", ToJson(*object.administrator_rights_));
  }
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBotStart &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeBotStart");
  jo("bot_username", object.bot_username_);
  jo("start_parameter", object.start_parameter_);
  jo("autostart", JsonBool{object.autostart_});
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBotStartInGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeBotStartInGroup");
  jo("bot_username", object.bot_username_);
  jo("start_parameter", object.start_parameter_);
  if (object.administrator_rights_) {
    jo("administrator_rights", ToJson(*object.administrator_rights_));
  }
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBusinessChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeBusinessChat");
  jo("link_name", object.link_name_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeBuyStars &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeBuyStars");
  jo("star_count", object.star_count_);
  jo("purpose", object.purpose_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChangePhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeChangePhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChatAffiliateProgram &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeChatAffiliateProgram");
  jo("username", object.username_);
  jo("referrer", object.referrer_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChatBoost &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeChatBoost");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChatFolderInvite &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeChatFolderInvite");
  jo("invite_link", object.invite_link_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChatFolderSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeChatFolderSettings");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChatInvite &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeChatInvite");
  jo("invite_link", object.invite_link_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeDefaultMessageAutoDeleteTimerSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeDefaultMessageAutoDeleteTimerSettings");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeDirectMessagesChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeDirectMessagesChat");
  jo("channel_username", object.channel_username_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeEditProfileSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeEditProfileSettings");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeGame");
  jo("bot_username", object.bot_username_);
  jo("game_short_name", object.game_short_name_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeGiftCollection &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeGiftCollection");
  jo("gift_owner_username", object.gift_owner_username_);
  jo("collection_id", object.collection_id_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeGroupCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeGroupCall");
  jo("invite_link", object.invite_link_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeInstantView &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeInstantView");
  jo("url", object.url_);
  jo("fallback_url", object.fallback_url_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeInvoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeInvoice");
  jo("invoice_name", object.invoice_name_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeLanguagePack &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeLanguagePack");
  jo("language_pack_id", object.language_pack_id_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeLanguageSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeLanguageSettings");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeMainWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeMainWebApp");
  jo("bot_username", object.bot_username_);
  jo("start_parameter", object.start_parameter_);
  if (object.mode_) {
    jo("mode", ToJson(*object.mode_));
  }
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeMessage");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeMessageDraft &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeMessageDraft");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("contains_link", JsonBool{object.contains_link_});
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeMyStars &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeMyStars");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeMyToncoins &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeMyToncoins");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePassportDataRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypePassportDataRequest");
  jo("bot_user_id", object.bot_user_id_);
  jo("scope", object.scope_);
  jo("public_key", object.public_key_);
  jo("nonce", object.nonce_);
  jo("callback_url", object.callback_url_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePhoneNumberConfirmation &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypePhoneNumberConfirmation");
  jo("hash", object.hash_);
  jo("phone_number", object.phone_number_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePremiumFeatures &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypePremiumFeatures");
  jo("referrer", object.referrer_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePremiumGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypePremiumGift");
  jo("referrer", object.referrer_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePremiumGiftCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypePremiumGiftCode");
  jo("code", object.code_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePrivacyAndSecuritySettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypePrivacyAndSecuritySettings");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeProxy &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeProxy");
  jo("server", object.server_);
  jo("port", object.port_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypePublicChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypePublicChat");
  jo("chat_username", object.chat_username_);
  jo("draft_text", object.draft_text_);
  jo("open_profile", JsonBool{object.open_profile_});
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeQrCodeAuthentication &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeQrCodeAuthentication");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeRestorePurchases &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeRestorePurchases");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeSettings");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeStickerSet &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeStickerSet");
  jo("sticker_set_name", object.sticker_set_name_);
  jo("expect_custom_emoji", JsonBool{object.expect_custom_emoji_});
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeStory");
  jo("story_poster_username", object.story_poster_username_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeStoryAlbum &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeStoryAlbum");
  jo("story_album_owner_username", object.story_album_owner_username_);
  jo("story_album_id", object.story_album_id_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeTheme");
  jo("theme_name", object.theme_name_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeThemeSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeThemeSettings");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUnknownDeepLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeUnknownDeepLink");
  jo("link", object.link_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUnsupportedProxy &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeUnsupportedProxy");
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUpgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeUpgradedGift");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUserPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeUserPhoneNumber");
  jo("phone_number", object.phone_number_);
  jo("draft_text", object.draft_text_);
  jo("open_profile", JsonBool{object.open_profile_});
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeUserToken &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeUserToken");
  jo("token", object.token_);
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeVideoChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeVideoChat");
  jo("chat_username", object.chat_username_);
  jo("invite_hash", object.invite_hash_);
  jo("is_live_stream", JsonBool{object.is_live_stream_});
}

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeWebApp");
  jo("bot_username", object.bot_username_);
  jo("web_app_short_name", object.web_app_short_name_);
  jo("start_parameter", object.start_parameter_);
  if (object.mode_) {
    jo("mode", ToJson(*object.mode_));
  }
}

void to_json(JsonValueScope &jv, const td_api::languagePackInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackInfo");
  jo("id", object.id_);
  jo("base_language_pack_id", object.base_language_pack_id_);
  jo("name", object.name_);
  jo("native_name", object.native_name_);
  jo("plural_code", object.plural_code_);
  jo("is_official", JsonBool{object.is_official_});
  jo("is_rtl", JsonBool{object.is_rtl_});
  jo("is_beta", JsonBool{object.is_beta_});
  jo("is_installed", JsonBool{object.is_installed_});
  jo("total_string_count", object.total_string_count_);
  jo("translated_string_count", object.translated_string_count_);
  jo("local_string_count", object.local_string_count_);
  jo("translation_url", object.translation_url_);
}

void to_json(JsonValueScope &jv, const td_api::localizationTargetInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "localizationTargetInfo");
  jo("language_packs", ToJson(object.language_packs_));
}

void to_json(JsonValueScope &jv, const td_api::maskPosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "maskPosition");
  if (object.point_) {
    jo("point", ToJson(*object.point_));
  }
  jo("x_shift", object.x_shift_);
  jo("y_shift", object.y_shift_);
  jo("scale", object.scale_);
}

void to_json(JsonValueScope &jv, const td_api::MessageFileType &object) {
  td_api::downcast_call(const_cast<td_api::MessageFileType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageFileTypePrivate &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageFileTypePrivate");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::messageFileTypeGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageFileTypeGroup");
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::messageFileTypeUnknown &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageFileTypeUnknown");
}

void to_json(JsonValueScope &jv, const td_api::messageProperties &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageProperties");
  jo("can_add_offer", JsonBool{object.can_add_offer_});
  jo("can_add_tasks", JsonBool{object.can_add_tasks_});
  jo("can_be_approved", JsonBool{object.can_be_approved_});
  jo("can_be_copied", JsonBool{object.can_be_copied_});
  jo("can_be_copied_to_secret_chat", JsonBool{object.can_be_copied_to_secret_chat_});
  jo("can_be_declined", JsonBool{object.can_be_declined_});
  jo("can_be_deleted_only_for_self", JsonBool{object.can_be_deleted_only_for_self_});
  jo("can_be_deleted_for_all_users", JsonBool{object.can_be_deleted_for_all_users_});
  jo("can_be_edited", JsonBool{object.can_be_edited_});
  jo("can_be_forwarded", JsonBool{object.can_be_forwarded_});
  jo("can_be_paid", JsonBool{object.can_be_paid_});
  jo("can_be_pinned", JsonBool{object.can_be_pinned_});
  jo("can_be_replied", JsonBool{object.can_be_replied_});
  jo("can_be_replied_in_another_chat", JsonBool{object.can_be_replied_in_another_chat_});
  jo("can_be_saved", JsonBool{object.can_be_saved_});
  jo("can_be_shared_in_story", JsonBool{object.can_be_shared_in_story_});
  jo("can_edit_media", JsonBool{object.can_edit_media_});
  jo("can_edit_scheduling_state", JsonBool{object.can_edit_scheduling_state_});
  jo("can_edit_suggested_post_info", JsonBool{object.can_edit_suggested_post_info_});
  jo("can_get_author", JsonBool{object.can_get_author_});
  jo("can_get_embedding_code", JsonBool{object.can_get_embedding_code_});
  jo("can_get_link", JsonBool{object.can_get_link_});
  jo("can_get_media_timestamp_links", JsonBool{object.can_get_media_timestamp_links_});
  jo("can_get_message_thread", JsonBool{object.can_get_message_thread_});
  jo("can_get_read_date", JsonBool{object.can_get_read_date_});
  jo("can_get_statistics", JsonBool{object.can_get_statistics_});
  jo("can_get_video_advertisements", JsonBool{object.can_get_video_advertisements_});
  jo("can_get_viewers", JsonBool{object.can_get_viewers_});
  jo("can_mark_tasks_as_done", JsonBool{object.can_mark_tasks_as_done_});
  jo("can_recognize_speech", JsonBool{object.can_recognize_speech_});
  jo("can_report_chat", JsonBool{object.can_report_chat_});
  jo("can_report_reactions", JsonBool{object.can_report_reactions_});
  jo("can_report_supergroup_spam", JsonBool{object.can_report_supergroup_spam_});
  jo("can_set_fact_check", JsonBool{object.can_set_fact_check_});
  jo("need_show_statistics", JsonBool{object.need_show_statistics_});
}

void to_json(JsonValueScope &jv, const td_api::messageSenders &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSenders");
  jo("total_count", object.total_count_);
  jo("senders", ToJson(object.senders_));
}

void to_json(JsonValueScope &jv, const td_api::networkStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkStatistics");
  jo("since_date", object.since_date_);
  jo("entries", ToJson(object.entries_));
}

void to_json(JsonValueScope &jv, const td_api::notificationSounds &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationSounds");
  jo("notification_sounds", ToJson(object.notification_sounds_));
}

void to_json(JsonValueScope &jv, const td_api::pageBlockListItem &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockListItem");
  jo("label", object.label_);
  jo("page_blocks", ToJson(object.page_blocks_));
}

void to_json(JsonValueScope &jv, const td_api::passportElementError &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementError");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("message", object.message_);
  if (object.source_) {
    jo("source", ToJson(*object.source_));
  }
}

void to_json(JsonValueScope &jv, const td_api::PaymentFormType &object) {
  td_api::downcast_call(const_cast<td_api::PaymentFormType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::paymentFormTypeRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentFormTypeRegular");
  if (object.invoice_) {
    jo("invoice", ToJson(*object.invoice_));
  }
  jo("payment_provider_user_id", object.payment_provider_user_id_);
  if (object.payment_provider_) {
    jo("payment_provider", ToJson(*object.payment_provider_));
  }
  jo("additional_payment_options", ToJson(object.additional_payment_options_));
  if (object.saved_order_info_) {
    jo("saved_order_info", ToJson(*object.saved_order_info_));
  }
  jo("saved_credentials", ToJson(object.saved_credentials_));
  jo("can_save_credentials", JsonBool{object.can_save_credentials_});
  jo("need_password", JsonBool{object.need_password_});
}

void to_json(JsonValueScope &jv, const td_api::paymentFormTypeStars &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentFormTypeStars");
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::paymentFormTypeStarSubscription &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentFormTypeStarSubscription");
  if (object.pricing_) {
    jo("pricing", ToJson(*object.pricing_));
  }
}

void to_json(JsonValueScope &jv, const td_api::photo &object) {
  auto jo = jv.enter_object();
  jo("@type", "photo");
  jo("has_stickers", JsonBool{object.has_stickers_});
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  jo("sizes", ToJson(object.sizes_));
}

void to_json(JsonValueScope &jv, const td_api::premiumGiftCodeInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumGiftCodeInfo");
  if (object.creator_id_) {
    jo("creator_id", ToJson(*object.creator_id_));
  }
  jo("creation_date", object.creation_date_);
  jo("is_from_giveaway", JsonBool{object.is_from_giveaway_});
  jo("giveaway_message_id", object.giveaway_message_id_);
  jo("month_count", object.month_count_);
  jo("user_id", object.user_id_);
  jo("use_date", object.use_date_);
}

void to_json(JsonValueScope &jv, const td_api::premiumStatePaymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumStatePaymentOption");
  if (object.payment_option_) {
    jo("payment_option", ToJson(*object.payment_option_));
  }
  jo("is_current", JsonBool{object.is_current_});
  jo("is_upgrade", JsonBool{object.is_upgrade_});
  jo("last_transaction_id", object.last_transaction_id_);
}

void to_json(JsonValueScope &jv, const td_api::proxies &object) {
  auto jo = jv.enter_object();
  jo("@type", "proxies");
  jo("proxies", ToJson(object.proxies_));
}

void to_json(JsonValueScope &jv, const td_api::quickReplyMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "quickReplyMessages");
  jo("messages", ToJson(object.messages_));
}

void to_json(JsonValueScope &jv, const td_api::recommendedChatFolder &object) {
  auto jo = jv.enter_object();
  jo("@type", "recommendedChatFolder");
  if (object.folder_) {
    jo("folder", ToJson(*object.folder_));
  }
  jo("description", object.description_);
}

void to_json(JsonValueScope &jv, const td_api::ResetPasswordResult &object) {
  td_api::downcast_call(const_cast<td_api::ResetPasswordResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::resetPasswordResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "resetPasswordResultOk");
}

void to_json(JsonValueScope &jv, const td_api::resetPasswordResultPending &object) {
  auto jo = jv.enter_object();
  jo("@type", "resetPasswordResultPending");
  jo("pending_reset_date", object.pending_reset_date_);
}

void to_json(JsonValueScope &jv, const td_api::resetPasswordResultDeclined &object) {
  auto jo = jv.enter_object();
  jo("@type", "resetPasswordResultDeclined");
  jo("retry_date", object.retry_date_);
}

void to_json(JsonValueScope &jv, const td_api::SavedMessagesTopicType &object) {
  td_api::downcast_call(const_cast<td_api::SavedMessagesTopicType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::savedMessagesTopicTypeMyNotes &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedMessagesTopicTypeMyNotes");
}

void to_json(JsonValueScope &jv, const td_api::savedMessagesTopicTypeAuthorHidden &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedMessagesTopicTypeAuthorHidden");
}

void to_json(JsonValueScope &jv, const td_api::savedMessagesTopicTypeSavedFromChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedMessagesTopicTypeSavedFromChat");
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::SessionType &object) {
  td_api::downcast_call(const_cast<td_api::SessionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeAndroid &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeAndroid");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeApple &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeApple");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeBrave &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeBrave");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeChrome &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeChrome");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeEdge &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeEdge");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeFirefox &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeFirefox");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeIpad &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeIpad");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeIphone &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeIphone");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeLinux &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeLinux");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeMac &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeMac");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeOpera &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeOpera");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeSafari &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeSafari");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeUbuntu &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeUbuntu");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeUnknown &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeUnknown");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeVivaldi &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeVivaldi");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeWindows &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeWindows");
}

void to_json(JsonValueScope &jv, const td_api::sessionTypeXbox &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessionTypeXbox");
}

void to_json(JsonValueScope &jv, const td_api::sponsoredMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "sponsoredMessages");
  jo("messages", ToJson(object.messages_));
  jo("messages_between", object.messages_between_);
}

void to_json(JsonValueScope &jv, const td_api::starRevenueStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "starRevenueStatus");
  if (object.total_amount_) {
    jo("total_amount", ToJson(*object.total_amount_));
  }
  if (object.current_amount_) {
    jo("current_amount", ToJson(*object.current_amount_));
  }
  if (object.available_amount_) {
    jo("available_amount", ToJson(*object.available_amount_));
  }
  jo("withdrawal_enabled", JsonBool{object.withdrawal_enabled_});
  jo("next_withdrawal_in", object.next_withdrawal_in_);
}

void to_json(JsonValueScope &jv, const td_api::statisticalValue &object) {
  auto jo = jv.enter_object();
  jo("@type", "statisticalValue");
  jo("value", object.value_);
  jo("previous_value", object.previous_value_);
  jo("growth_rate_percentage", object.growth_rate_percentage_);
}

void to_json(JsonValueScope &jv, const td_api::storageStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "storageStatistics");
  jo("size", object.size_);
  jo("count", object.count_);
  jo("by_chat", ToJson(object.by_chat_));
}

void to_json(JsonValueScope &jv, const td_api::storyAreaPosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaPosition");
  jo("x_percentage", object.x_percentage_);
  jo("y_percentage", object.y_percentage_);
  jo("width_percentage", object.width_percentage_);
  jo("height_percentage", object.height_percentage_);
  jo("rotation_angle", object.rotation_angle_);
  jo("corner_radius_percentage", object.corner_radius_percentage_);
}

void to_json(JsonValueScope &jv, const td_api::StoryOrigin &object) {
  td_api::downcast_call(const_cast<td_api::StoryOrigin &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::storyOriginPublicStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyOriginPublicStory");
  jo("chat_id", object.chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::storyOriginHiddenUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyOriginHiddenUser");
  jo("poster_name", object.poster_name_);
}

void to_json(JsonValueScope &jv, const td_api::SuggestedPostState &object) {
  td_api::downcast_call(const_cast<td_api::SuggestedPostState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostStatePending &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostStatePending");
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostStateApproved &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostStateApproved");
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostStateDeclined &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostStateDeclined");
}

void to_json(JsonValueScope &jv, const td_api::termsOfService &object) {
  auto jo = jv.enter_object();
  jo("@type", "termsOfService");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("min_user_age", object.min_user_age_);
  jo("show_popup", JsonBool{object.show_popup_});
}

void to_json(JsonValueScope &jv, const td_api::textEntities &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntities");
  jo("entities", ToJson(object.entities_));
}

void to_json(JsonValueScope &jv, const td_api::tonRevenueStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonRevenueStatistics");
  if (object.revenue_by_day_graph_) {
    jo("revenue_by_day_graph", ToJson(*object.revenue_by_day_graph_));
  }
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
  jo("usd_rate", object.usd_rate_);
}

void to_json(JsonValueScope &jv, const td_api::updates &object) {
  auto jo = jv.enter_object();
  jo("@type", "updates");
  jo("updates", ToJson(object.updates_));
}

void to_json(JsonValueScope &jv, const td_api::UpgradedGiftOrigin &object) {
  td_api::downcast_call(const_cast<td_api::UpgradedGiftOrigin &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftOriginUpgrade &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftOriginUpgrade");
  jo("gift_message_id", object.gift_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftOriginTransfer &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftOriginTransfer");
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftOriginResale &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftOriginResale");
  if (object.price_) {
    jo("price", ToJson(*object.price_));
  }
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftOriginBlockchain &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftOriginBlockchain");
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftOriginPrepaidUpgrade &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftOriginPrepaidUpgrade");
}

void to_json(JsonValueScope &jv, const td_api::UserPrivacySettingRule &object) {
  td_api::downcast_call(const_cast<td_api::UserPrivacySettingRule &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowAll &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleAllowAll");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowContacts &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleAllowContacts");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowBots &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleAllowBots");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowPremiumUsers &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleAllowPremiumUsers");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowUsers &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleAllowUsers");
  jo("user_ids", ToJson(object.user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleAllowChatMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleAllowChatMembers");
  jo("chat_ids", ToJson(object.chat_ids_));
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictAll &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleRestrictAll");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictContacts &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleRestrictContacts");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictBots &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleRestrictBots");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictUsers &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleRestrictUsers");
  jo("user_ids", ToJson(object.user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRuleRestrictChatMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRuleRestrictChatMembers");
  jo("chat_ids", ToJson(object.chat_ids_));
}

void to_json(JsonValueScope &jv, const td_api::VectorPathCommand &object) {
  td_api::downcast_call(const_cast<td_api::VectorPathCommand &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::vectorPathCommandLine &object) {
  auto jo = jv.enter_object();
  jo("@type", "vectorPathCommandLine");
  if (object.end_point_) {
    jo("end_point", ToJson(*object.end_point_));
  }
}

void to_json(JsonValueScope &jv, const td_api::vectorPathCommandCubicBezierCurve &object) {
  auto jo = jv.enter_object();
  jo("@type", "vectorPathCommandCubicBezierCurve");
  if (object.start_control_point_) {
    jo("start_control_point", ToJson(*object.start_control_point_));
  }
  if (object.end_control_point_) {
    jo("end_control_point", ToJson(*object.end_control_point_));
  }
  if (object.end_point_) {
    jo("end_point", ToJson(*object.end_point_));
  }
}

void to_json(JsonValueScope &jv, const td_api::videoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoNote");
  jo("duration", object.duration_);
  jo("waveform", base64_encode(object.waveform_));
  jo("length", object.length_);
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.speech_recognition_result_) {
    jo("speech_recognition_result", ToJson(*object.speech_recognition_result_));
  }
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
}

}  // namespace td_api
}  // namespace td
