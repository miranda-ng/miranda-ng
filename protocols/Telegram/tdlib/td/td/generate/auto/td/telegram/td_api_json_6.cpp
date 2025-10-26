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
Result<int32> tl_constructor_from_string(td_api::BackgroundType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"backgroundTypeWallpaper", 1972128891},
    {"backgroundTypePattern", 1290213117},
    {"backgroundTypeFill", 993008684},
    {"backgroundTypeChatTheme", 1299879762}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ChatList *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatListMain", -400991316},
    {"chatListArchive", 362770115},
    {"chatListFolder", 385760856}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::FileType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"fileTypeNone", 2003009189},
    {"fileTypeAnimation", -290816582},
    {"fileTypeAudio", -709112160},
    {"fileTypeDocument", -564722929},
    {"fileTypeNotificationSound", -1020289271},
    {"fileTypePhoto", -1718914651},
    {"fileTypePhotoStory", 2018995956},
    {"fileTypeProfilePhoto", 1795089315},
    {"fileTypeSecret", -1871899401},
    {"fileTypeSecretThumbnail", -1401326026},
    {"fileTypeSecure", -1419133146},
    {"fileTypeSelfDestructingPhoto", 2077176475},
    {"fileTypeSelfDestructingVideo", -1223900123},
    {"fileTypeSelfDestructingVideoNote", 1495274177},
    {"fileTypeSelfDestructingVoiceNote", 1691409181},
    {"fileTypeSticker", 475233385},
    {"fileTypeThumbnail", -12443298},
    {"fileTypeUnknown", -2011566768},
    {"fileTypeVideo", 1430816539},
    {"fileTypeVideoNote", -518412385},
    {"fileTypeVideoStory", -2146754143},
    {"fileTypeVoiceNote", -588681661},
    {"fileTypeWallpaper", 1854930076}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputChatPhoto *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputChatPhotoPrevious", 23128529},
    {"inputChatPhotoStatic", 1979179699},
    {"inputChatPhotoAnimation", 90846242},
    {"inputChatPhotoSticker", 1315861341}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputPaidMediaType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputPaidMediaTypePhoto", -761660134},
    {"inputPaidMediaTypeVideo", 1793741625}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::LogStream *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"logStreamDefault", 1390581436},
    {"logStreamFile", 1532136933},
    {"logStreamEmpty", -499912244}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::NotificationSettingsScope *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"notificationSettingsScopePrivateChats", 937446759},
    {"notificationSettingsScopeGroupChats", 1212142067},
    {"notificationSettingsScopeChannelChats", 548013448}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PremiumStoryFeature *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"premiumStoryFeaturePriorityOrder", -1880001849},
    {"premiumStoryFeatureStealthMode", 1194605988},
    {"premiumStoryFeaturePermanentViewsHistory", -1029683296},
    {"premiumStoryFeatureCustomExpirationDuration", -593229162},
    {"premiumStoryFeatureSaveStories", -1501286467},
    {"premiumStoryFeatureLinksAndFormatting", -622623753},
    {"premiumStoryFeatureVideoQuality", -1162887511}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::SearchMessagesChatTypeFilter *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"searchMessagesChatTypeFilterPrivate", 1169248975},
    {"searchMessagesChatTypeFilterGroup", -2059426022},
    {"searchMessagesChatTypeFilterChannel", -773540139}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::SuggestedPostPrice *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"suggestedPostPriceStar", 216488903},
    {"suggestedPostPriceTon", -1095222334}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::UserPrivacySetting *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"userPrivacySettingShowStatus", 1862829310},
    {"userPrivacySettingShowProfilePhoto", 1408485877},
    {"userPrivacySettingShowLinkInForwardedMessages", 592688870},
    {"userPrivacySettingShowPhoneNumber", -791567831},
    {"userPrivacySettingShowBio", 959981409},
    {"userPrivacySettingShowBirthdate", 1167504607},
    {"userPrivacySettingAllowChatInvites", 1271668007},
    {"userPrivacySettingAllowCalls", -906967291},
    {"userPrivacySettingAllowPeerToPeerCalls", 352500032},
    {"userPrivacySettingAllowFindingByPhoneNumber", -1846645423},
    {"userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages", 338112060},
    {"userPrivacySettingAutosaveGifts", 1889167821},
    {"userPrivacySettingAllowUnpaidMessages", 1430051047}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::affiliateProgramSortOrderRevenue &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::autosaveSettingsScopeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::blockListMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureOpeningHours &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureUpgradedStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemDistortedSpeech &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  return Status::OK();
}

Status from_json(td_api::chatActionChoosingLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatAvailableReactionsSome &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reactions_, from.extract_field("reactions")));
  TRY_STATUS(from_json(to.max_reaction_count_, from.extract_field("max_reaction_count")));
  return Status::OK();
}

Status from_json(td_api::chatListFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterAdministrators &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatPhotoStickerTypeRegularOrMask &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_id_, from.extract_field("sticker_set_id")));
  TRY_STATUS(from_json(to.sticker_id_, from.extract_field("sticker_id")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenWindowsPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.access_token_, from.extract_field("access_token")));
  return Status::OK();
}

Status from_json(td_api::draftMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  TRY_STATUS(from_json(to.input_message_text_, from.extract_field("input_message_text")));
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  TRY_STATUS(from_json(to.suggested_post_info_, from.extract_field("suggested_post_info")));
  return Status::OK();
}

Status from_json(td_api::emojiStatusTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::fileTypePhotoStory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSticker &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::firebaseAuthenticationSettingsIos &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, from.extract_field("is_app_sandbox")));
  return Status::OK();
}

Status from_json(td_api::giveawayParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.boosted_chat_id_, from.extract_field("boosted_chat_id")));
  TRY_STATUS(from_json(to.additional_chat_ids_, from.extract_field("additional_chat_ids")));
  TRY_STATUS(from_json(to.winners_selection_date_, from.extract_field("winners_selection_date")));
  TRY_STATUS(from_json(to.only_new_members_, from.extract_field("only_new_members")));
  TRY_STATUS(from_json(to.has_public_winners_, from.extract_field("has_public_winners")));
  TRY_STATUS(from_json(to.country_codes_, from.extract_field("country_codes")));
  TRY_STATUS(from_json(to.prize_description_, from.extract_field("prize_description")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCopyText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoPrevious &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_photo_id_, from.extract_field("chat_photo_id")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsNew &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  TRY_STATUS(from_json(to.allow_save_, from.extract_field("allow_save")));
  return Status::OK();
}

Status from_json(td_api::inputIdentityDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.number_, from.extract_field("number")));
  TRY_STATUS(from_json(to.expiration_date_, from.extract_field("expiration_date")));
  TRY_STATUS(from_json(to.front_side_, from.extract_field("front_side")));
  TRY_STATUS(from_json(to.reverse_side_, from.extract_field("reverse_side")));
  TRY_STATUS(from_json(to.selfie_, from.extract_field("selfie")));
  TRY_STATUS(from_json(to.translation_, from.extract_field("translation")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.sticker_url_, from.extract_field("sticker_url")));
  TRY_STATUS(from_json(to.sticker_width_, from.extract_field("sticker_width")));
  TRY_STATUS(from_json(to.sticker_height_, from.extract_field("sticker_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputMessageAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.audio_, from.extract_field("audio")));
  TRY_STATUS(from_json(to.album_cover_thumbnail_, from.extract_field("album_cover_thumbnail")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.performer_, from.extract_field("performer")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.venue_, from.extract_field("venue")));
  return Status::OK();
}

Status from_json(td_api::inputMessageReplyToMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  TRY_STATUS(from_json(to.checklist_task_id_, from.extract_field("checklist_task_id")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementIdentityCard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.identity_card_, from.extract_field("identity_card")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFile &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::inputThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBuyStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeEditProfileSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePrivacyAndSecuritySettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_name_, from.extract_field("theme_name")));
  return Status::OK();
}

Status from_json(td_api::invoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.price_parts_, from.extract_field("price_parts")));
  TRY_STATUS(from_json(to.subscription_period_, from.extract_field("subscription_period")));
  TRY_STATUS(from_json(to.max_tip_amount_, from.extract_field("max_tip_amount")));
  TRY_STATUS(from_json(to.suggested_tip_amounts_, from.extract_field("suggested_tip_amounts")));
  TRY_STATUS(from_json(to.recurring_payment_terms_of_service_url_, from.extract_field("recurring_payment_terms_of_service_url")));
  TRY_STATUS(from_json(to.terms_of_service_url_, from.extract_field("terms_of_service_url")));
  TRY_STATUS(from_json(to.is_test_, from.extract_field("is_test")));
  TRY_STATUS(from_json(to.need_name_, from.extract_field("need_name")));
  TRY_STATUS(from_json(to.need_phone_number_, from.extract_field("need_phone_number")));
  TRY_STATUS(from_json(to.need_email_address_, from.extract_field("need_email_address")));
  TRY_STATUS(from_json(to.need_shipping_address_, from.extract_field("need_shipping_address")));
  TRY_STATUS(from_json(to.send_phone_number_to_provider_, from.extract_field("send_phone_number_to_provider")));
  TRY_STATUS(from_json(to.send_email_address_to_provider_, from.extract_field("send_email_address_to_provider")));
  TRY_STATUS(from_json(to.is_flexible_, from.extract_field("is_flexible")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeText &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::languagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.key_, from.extract_field("key")));
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::logStreamEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSchedulingStateSendWhenOnline &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceForumTopicHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageTopicThread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::networkTypeWiFi &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optionValueString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::passportElementTypeInternalPassport &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::personalDetails &to, JsonObject &from) {
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.middle_name_, from.extract_field("middle_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  TRY_STATUS(from_json(to.native_first_name_, from.extract_field("native_first_name")));
  TRY_STATUS(from_json(to.native_middle_name_, from.extract_field("native_middle_name")));
  TRY_STATUS(from_json(to.native_last_name_, from.extract_field("native_last_name")));
  TRY_STATUS(from_json(to.birthdate_, from.extract_field("birthdate")));
  TRY_STATUS(from_json(to.gender_, from.extract_field("gender")));
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  TRY_STATUS(from_json(to.residence_country_code_, from.extract_field("residence_country_code")));
  return Status::OK();
}

Status from_json(td_api::premiumFeatureImprovedDownloadSpeed &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAnimatedProfilePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureMessagePrivacy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeFavoriteStickerCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeActiveStoryCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumSourceStoryFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureVideoQuality &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::proxyTypeSocks5 &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::reactionTypeEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::reportReasonViolence &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendCodeReasonUserRequest &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterAudio &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVoiceAndVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerFormatTgs &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposeStarGiveaway &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.winner_count_, from.extract_field("winner_count")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsEveryone &to, JsonObject &from) {
  TRY_STATUS(from_json(to.except_user_ids_, from.extract_field("except_user_ids")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionSetPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_delay_, from.extract_field("authorization_delay")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionCustom &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterMention &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposeStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeCashtag &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeStrikethrough &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::topChatCategoryInlineBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::upgradedGiftBackdropColors &to, JsonObject &from) {
  TRY_STATUS(from_json(to.center_color_, from.extract_field("center_color")));
  TRY_STATUS(from_json(to.edge_color_, from.extract_field("edge_color")));
  TRY_STATUS(from_json(to.symbol_color_, from.extract_field("symbol_color")));
  TRY_STATUS(from_json(to.text_color_, from.extract_field("text_color")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowPeerToPeerCalls &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::webAppOpenModeCompact &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::addChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.forward_limit_, from.extract_field("forward_limit")));
  return Status::OK();
}

Status from_json(td_api::addLocalMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::addQuickReplyShortcutMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_name_, from.extract_field("shortcut_name")));
  TRY_STATUS(from_json(to.reply_to_message_id_, from.extract_field("reply_to_message_id")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::allowUnpaidMessagesFromUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.refund_payments_, from.extract_field("refund_payments")));
  return Status::OK();
}

Status from_json(td_api::assignStoreTransaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.transaction_, from.extract_field("transaction")));
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::canSendGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.gift_id_, from.extract_field("gift_id")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationBotToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::checkCreatedPublicChatsLimit &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::checkWebAppFileDownload &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.file_name_, from.extract_field("file_name")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::clearSearchedForTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.clear_cashtags_, from.extract_field("clear_cashtags")));
  return Status::OK();
}

Status from_json(td_api::closeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.web_app_launch_id_, from.extract_field("web_app_launch_id")));
  return Status::OK();
}

Status from_json(td_api::createChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::createNewStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.needs_repainting_, from.extract_field("needs_repainting")));
  TRY_STATUS(from_json(to.stickers_, from.extract_field("stickers")));
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  return Status::OK();
}

Status from_json(td_api::declineSuggestedPost &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.comment_, from.extract_field("comment")));
  return Status::OK();
}

Status from_json(td_api::deleteBusinessStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::deleteCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::deletePassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::deleteStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::disconnectWebsite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.website_id_, from.extract_field("website_id")));
  return Status::OK();
}

Status from_json(td_api::editBusinessMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageCaption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
  return Status::OK();
}

Status from_json(td_api::editMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::enableProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::getAllPassportElements &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getAuthorizationState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBlockedMessageSenders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.block_list_, from.extract_field("block_list")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getBusinessChatLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_name_, from.extract_field("link_name")));
  return Status::OK();
}

Status from_json(td_api::getChatAdministrators &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatBoosts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.only_gift_codes_, from.extract_field("only_gift_codes")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.return_local_, from.extract_field("return_local")));
  return Status::OK();
}

Status from_json(td_api::getChatSimilarChatCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.return_local_, from.extract_field("return_local")));
  return Status::OK();
}

Status from_json(td_api::getChatsToPostStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCountryCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultBackgroundCustomEmojiStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDirectMessagesChatTopicRevenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  return Status::OK();
}

Status from_json(td_api::getFileDownloadedPrefixSize &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  return Status::OK();
}

Status from_json(td_api::getGiftChatThemes &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getImportedContactCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getJsonValue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.json_, from.extract_field("json")));
  return Status::OK();
}

Status from_json(td_api::getLogTags &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::getMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::getPassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getPremiumGiftPaymentOptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getProxyLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::getRecentlyOpenedChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getSavedMessagesTopicHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getStarAdAccountUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  return Status::OK();
}

Status from_json(td_api::getStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::getStoryAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.row_size_, from.extract_field("row_size")));
  return Status::OK();
}

Status from_json(td_api::getSupergroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::getThemedEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getUpgradedGiftValueInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::getUserSupportInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getWebAppUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::isProfileAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::loadChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::openChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::pinChatMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  TRY_STATUS(from_json(to.only_for_self_, from.extract_field("only_for_self")));
  return Status::OK();
}

Status from_json(td_api::readAllChatMentions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::recognizeSpeech &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::removeContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::removePendingPaidMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::removeStickerFromSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::reorderInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.sticker_set_ids_, from.extract_field("sticker_set_ids")));
  return Status::OK();
}

Status from_json(td_api::reportChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json_bytes(to.option_id_, from.extract_field("option_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::reportVideoMessageAdvertisement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.advertisement_unique_id_, from.extract_field("advertisement_unique_id")));
  TRY_STATUS(from_json_bytes(to.option_id_, from.extract_field("option_id")));
  return Status::OK();
}

Status from_json(td_api::resendRecoveryEmailAddressCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::saveApplicationLogEvent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::searchChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchOutgoingDocumentMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchRecentlyFoundChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.web_app_short_name_, from.extract_field("web_app_short_name")));
  return Status::OK();
}

Status from_json(td_api::sendCallSignalingData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::sendPassportAuthorizationForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_form_id_, from.extract_field("authorization_form_id")));
  TRY_STATUS(from_json(to.types_, from.extract_field("types")));
  return Status::OK();
}

Status from_json(td_api::setAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.accent_color_id_, from.extract_field("accent_color_id")));
  TRY_STATUS(from_json(to.background_custom_emoji_id_, from.extract_field("background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setAutosaveSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setBusinessAccountGiftSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setBusinessOpeningHours &to, JsonObject &from) {
  TRY_STATUS(from_json(to.opening_hours_, from.extract_field("opening_hours")));
  return Status::OK();
}

Status from_json(td_api::setChatDirectMessagesGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_enabled_, from.extract_field("is_enabled")));
  TRY_STATUS(from_json(to.paid_message_star_count_, from.extract_field("paid_message_star_count")));
  return Status::OK();
}

Status from_json(td_api::setChatPaidMessageStarCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.paid_message_star_count_, from.extract_field("paid_message_star_count")));
  return Status::OK();
}

Status from_json(td_api::setCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.commands_, from.extract_field("commands")));
  return Status::OK();
}

Status from_json(td_api::setDefaultReactionType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  return Status::OK();
}

Status from_json(td_api::setGroupCallParticipantIsSpeaking &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.audio_source_, from.extract_field("audio_source")));
  TRY_STATUS(from_json(to.is_speaking_, from.extract_field("is_speaking")));
  return Status::OK();
}

Status from_json(td_api::setMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.menu_button_, from.extract_field("menu_button")));
  return Status::OK();
}

Status from_json(td_api::setPaidMessageReactionType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::setPollAnswer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.option_ids_, from.extract_field("option_ids")));
  return Status::OK();
}

Status from_json(td_api::setScopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setStoryReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.update_recent_reactions_, from.extract_field("update_recent_reactions")));
  return Status::OK();
}

Status from_json(td_api::setUserNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.note_, from.extract_field("note")));
  return Status::OK();
}

Status from_json(td_api::shareUsersWithBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.button_id_, from.extract_field("button_id")));
  TRY_STATUS(from_json(to.shared_user_ids_, from.extract_field("shared_user_ids")));
  TRY_STATUS(from_json(to.only_check_, from.extract_field("only_check")));
  return Status::OK();
}

Status from_json(td_api::terminateAllOtherSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::testGetDifference &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::toggleBotUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.is_active_, from.extract_field("is_active")));
  return Status::OK();
}

Status from_json(td_api::toggleChatViewAsTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.view_as_topics_, from.extract_field("view_as_topics")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallIsMyVideoPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.is_my_video_paused_, from.extract_field("is_my_video_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupCanHaveSponsoredMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.can_have_sponsored_messages_, from.extract_field("can_have_sponsored_messages")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupSignMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.sign_messages_, from.extract_field("sign_messages")));
  TRY_STATUS(from_json(to.show_message_sender_, from.extract_field("show_message_sender")));
  return Status::OK();
}

Status from_json(td_api::translateText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.to_language_code_, from.extract_field("to_language_code")));
  return Status::OK();
}

Status from_json(td_api::viewMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  TRY_STATUS(from_json(to.force_read_, from.extract_field("force_read")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::accountTtl &object) {
  auto jo = jv.enter_object();
  jo("@type", "accountTtl");
  jo("days", object.days_);
}

void to_json(JsonValueScope &jv, const td_api::alternativeVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "alternativeVideo");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("width", object.width_);
  jo("height", object.height_);
  jo("codec", object.codec_);
  if (object.hls_file_) {
    jo("hls_file", ToJson(*object.hls_file_));
  }
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
}

void to_json(JsonValueScope &jv, const td_api::audios &object) {
  auto jo = jv.enter_object();
  jo("@type", "audios");
  jo("total_count", object.total_count_);
  jo("audios", ToJson(object.audios_));
}

void to_json(JsonValueScope &jv, const td_api::availableGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "availableGift");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  jo("resale_count", object.resale_count_);
  jo("min_resale_star_count", object.min_resale_star_count_);
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::bankCardInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "bankCardInfo");
  jo("title", object.title_);
  jo("actions", ToJson(object.actions_));
}

void to_json(JsonValueScope &jv, const td_api::botMediaPreviewInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMediaPreviewInfo");
  jo("previews", ToJson(object.previews_));
  jo("language_codes", ToJson(object.language_codes_));
}

void to_json(JsonValueScope &jv, const td_api::businessBotManageBar &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessBotManageBar");
  jo("bot_user_id", object.bot_user_id_);
  jo("manage_url", object.manage_url_);
  jo("is_bot_paused", JsonBool{object.is_bot_paused_});
  jo("can_bot_reply", JsonBool{object.can_bot_reply_});
}

void to_json(JsonValueScope &jv, const td_api::businessFeatures &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatures");
  jo("features", ToJson(object.features_));
}

void to_json(JsonValueScope &jv, const td_api::businessStartPage &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessStartPage");
  jo("title", object.title_);
  jo("message", object.message_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::CallbackQueryPayload &object) {
  td_api::downcast_call(const_cast<td_api::CallbackQueryPayload &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::callbackQueryPayloadData &object) {
  auto jo = jv.enter_object();
  jo("@type", "callbackQueryPayloadData");
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::callbackQueryPayloadDataWithPassword &object) {
  auto jo = jv.enter_object();
  jo("@type", "callbackQueryPayloadDataWithPassword");
  jo("password", object.password_);
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::callbackQueryPayloadGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "callbackQueryPayloadGame");
  jo("game_short_name", object.game_short_name_);
}

void to_json(JsonValueScope &jv, const td_api::chatAdministrator &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAdministrator");
  jo("user_id", object.user_id_);
  jo("custom_title", object.custom_title_);
  jo("is_owner", JsonBool{object.is_owner_});
}

void to_json(JsonValueScope &jv, const td_api::chatBoostLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostLinkInfo");
  jo("is_public", JsonBool{object.is_public_});
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatFolderIcon &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderIcon");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkInfo");
  jo("chat_id", object.chat_id_);
  jo("accessible_for", object.accessible_for_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("title", object.title_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("accent_color_id", object.accent_color_id_);
  jo("description", object.description_);
  jo("member_count", object.member_count_);
  jo("member_user_ids", ToJson(object.member_user_ids_));
  if (object.subscription_info_) {
    jo("subscription_info", ToJson(*object.subscription_info_));
  }
  jo("creates_join_request", JsonBool{object.creates_join_request_});
  jo("is_public", JsonBool{object.is_public_});
  if (object.verification_status_) {
    jo("verification_status", ToJson(*object.verification_status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatLists &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatLists");
  jo("chat_lists", ToJson(object.chat_lists_));
}

void to_json(JsonValueScope &jv, const td_api::chatPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPhoto");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("added_date", object.added_date_);
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  jo("sizes", ToJson(object.sizes_));
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  if (object.small_animation_) {
    jo("small_animation", ToJson(*object.small_animation_));
  }
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ChatRevenueTransactionType &object) {
  td_api::downcast_call(const_cast<td_api::ChatRevenueTransactionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeUnsupported");
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeSponsoredMessageEarnings &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeSponsoredMessageEarnings");
  jo("start_date", object.start_date_);
  jo("end_date", object.end_date_);
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeSuggestedPostEarnings &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeSuggestedPostEarnings");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeFragmentWithdrawal &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeFragmentWithdrawal");
  jo("withdrawal_date", object.withdrawal_date_);
  if (object.state_) {
    jo("state", ToJson(*object.state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeFragmentRefund &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeFragmentRefund");
  jo("refund_date", object.refund_date_);
}

void to_json(JsonValueScope &jv, const td_api::ChatTheme &object) {
  td_api::downcast_call(const_cast<td_api::ChatTheme &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatThemeEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatThemeEmoji");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::chatThemeGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatThemeGift");
  if (object.gift_theme_) {
    jo("gift_theme", ToJson(*object.gift_theme_));
  }
}

void to_json(JsonValueScope &jv, const td_api::collectibleItemInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "collectibleItemInfo");
  jo("purchase_date", object.purchase_date_);
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("cryptocurrency", object.cryptocurrency_);
  jo("cryptocurrency_amount", ToJson(JsonInt64{object.cryptocurrency_amount_}));
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::countryInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "countryInfo");
  jo("country_code", object.country_code_);
  jo("name", object.name_);
  jo("english_name", object.english_name_);
  jo("is_hidden", JsonBool{object.is_hidden_});
  jo("calling_codes", ToJson(object.calling_codes_));
}

void to_json(JsonValueScope &jv, const td_api::deepLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "deepLinkInfo");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("need_update_application", JsonBool{object.need_update_application_});
}

void to_json(JsonValueScope &jv, const td_api::emojiCategory &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiCategory");
  jo("name", object.name_);
  if (object.icon_) {
    jo("icon", ToJson(*object.icon_));
  }
  if (object.source_) {
    jo("source", ToJson(*object.source_));
  }
  jo("is_greeting", JsonBool{object.is_greeting_});
}

void to_json(JsonValueScope &jv, const td_api::emojiStatuses &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatuses");
  jo("emoji_statuses", ToJson(object.emoji_statuses_));
}

void to_json(JsonValueScope &jv, const td_api::fileDownload &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileDownload");
  jo("file_id", object.file_id_);
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("add_date", object.add_date_);
  jo("complete_date", object.complete_date_);
  jo("is_paused", JsonBool{object.is_paused_});
}

void to_json(JsonValueScope &jv, const td_api::forwardSource &object) {
  auto jo = jv.enter_object();
  jo("@type", "forwardSource");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("sender_name", object.sender_name_);
  jo("date", object.date_);
  jo("is_outgoing", JsonBool{object.is_outgoing_});
}

void to_json(JsonValueScope &jv, const td_api::foundPublicPosts &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundPublicPosts");
  jo("messages", ToJson(object.messages_));
  jo("next_offset", object.next_offset_);
  if (object.search_limits_) {
    jo("search_limits", ToJson(*object.search_limits_));
  }
  jo("are_limits_exceeded", JsonBool{object.are_limits_exceeded_});
}

void to_json(JsonValueScope &jv, const td_api::giftChatThemes &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftChatThemes");
  jo("themes", ToJson(object.themes_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::giftUpgradePreview &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftUpgradePreview");
  jo("models", ToJson(object.models_));
  jo("symbols", ToJson(object.symbols_));
  jo("backdrops", ToJson(object.backdrops_));
  jo("prices", ToJson(object.prices_));
  jo("next_prices", ToJson(object.next_prices_));
}

void to_json(JsonValueScope &jv, const td_api::groupCallInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallInfo");
  jo("group_call_id", object.group_call_id_);
  jo("join_payload", object.join_payload_);
}

void to_json(JsonValueScope &jv, const td_api::importedContacts &object) {
  auto jo = jv.enter_object();
  jo("@type", "importedContacts");
  jo("user_ids", ToJson(object.user_ids_));
  jo("importer_count", ToJson(object.importer_count_));
}

void to_json(JsonValueScope &jv, const td_api::InputFile &object) {
  td_api::downcast_call(const_cast<td_api::InputFile &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inputFileId &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputFileId");
  jo("id", object.id_);
}

void to_json(JsonValueScope &jv, const td_api::inputFileRemote &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputFileRemote");
  jo("id", object.id_);
}

void to_json(JsonValueScope &jv, const td_api::inputFileLocal &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputFileLocal");
  jo("path", object.path_);
}

void to_json(JsonValueScope &jv, const td_api::inputFileGenerated &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputFileGenerated");
  jo("original_path", object.original_path_);
  jo("conversion", object.conversion_);
  jo("expected_size", object.expected_size_);
}

void to_json(JsonValueScope &jv, const td_api::InviteGroupCallParticipantResult &object) {
  td_api::downcast_call(const_cast<td_api::InviteGroupCallParticipantResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inviteGroupCallParticipantResultUserPrivacyRestricted &object) {
  auto jo = jv.enter_object();
  jo("@type", "inviteGroupCallParticipantResultUserPrivacyRestricted");
}

void to_json(JsonValueScope &jv, const td_api::inviteGroupCallParticipantResultUserAlreadyParticipant &object) {
  auto jo = jv.enter_object();
  jo("@type", "inviteGroupCallParticipantResultUserAlreadyParticipant");
}

void to_json(JsonValueScope &jv, const td_api::inviteGroupCallParticipantResultUserWasBanned &object) {
  auto jo = jv.enter_object();
  jo("@type", "inviteGroupCallParticipantResultUserWasBanned");
}

void to_json(JsonValueScope &jv, const td_api::inviteGroupCallParticipantResultSuccess &object) {
  auto jo = jv.enter_object();
  jo("@type", "inviteGroupCallParticipantResultSuccess");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::languagePackString &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackString");
  jo("key", object.key_);
  if (object.value_) {
    jo("value", ToJson(*object.value_));
  }
}

void to_json(JsonValueScope &jv, const td_api::location &object) {
  auto jo = jv.enter_object();
  jo("@type", "location");
  jo("latitude", object.latitude_);
  jo("longitude", object.longitude_);
  jo("horizontal_accuracy", object.horizontal_accuracy_);
}

void to_json(JsonValueScope &jv, const td_api::message &object) {
  auto jo = jv.enter_object();
  jo("@type", "message");
  jo("id", object.id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("chat_id", object.chat_id_);
  if (object.sending_state_) {
    jo("sending_state", ToJson(*object.sending_state_));
  }
  if (object.scheduling_state_) {
    jo("scheduling_state", ToJson(*object.scheduling_state_));
  }
  jo("is_outgoing", JsonBool{object.is_outgoing_});
  jo("is_pinned", JsonBool{object.is_pinned_});
  jo("is_from_offline", JsonBool{object.is_from_offline_});
  jo("can_be_saved", JsonBool{object.can_be_saved_});
  jo("has_timestamped_media", JsonBool{object.has_timestamped_media_});
  jo("is_channel_post", JsonBool{object.is_channel_post_});
  jo("is_paid_star_suggested_post", JsonBool{object.is_paid_star_suggested_post_});
  jo("is_paid_ton_suggested_post", JsonBool{object.is_paid_ton_suggested_post_});
  jo("contains_unread_mention", JsonBool{object.contains_unread_mention_});
  jo("date", object.date_);
  jo("edit_date", object.edit_date_);
  if (object.forward_info_) {
    jo("forward_info", ToJson(*object.forward_info_));
  }
  if (object.import_info_) {
    jo("import_info", ToJson(*object.import_info_));
  }
  if (object.interaction_info_) {
    jo("interaction_info", ToJson(*object.interaction_info_));
  }
  jo("unread_reactions", ToJson(object.unread_reactions_));
  if (object.fact_check_) {
    jo("fact_check", ToJson(*object.fact_check_));
  }
  if (object.suggested_post_info_) {
    jo("suggested_post_info", ToJson(*object.suggested_post_info_));
  }
  if (object.reply_to_) {
    jo("reply_to", ToJson(*object.reply_to_));
  }
  if (object.topic_id_) {
    jo("topic_id", ToJson(*object.topic_id_));
  }
  if (object.self_destruct_type_) {
    jo("self_destruct_type", ToJson(*object.self_destruct_type_));
  }
  jo("self_destruct_in", object.self_destruct_in_);
  jo("auto_delete_in", object.auto_delete_in_);
  jo("via_bot_user_id", object.via_bot_user_id_);
  jo("sender_business_bot_user_id", object.sender_business_bot_user_id_);
  jo("sender_boost_count", object.sender_boost_count_);
  jo("paid_message_star_count", object.paid_message_star_count_);
  jo("author_signature", object.author_signature_);
  jo("media_album_id", ToJson(JsonInt64{object.media_album_id_}));
  jo("effect_id", ToJson(JsonInt64{object.effect_id_}));
  if (object.restriction_info_) {
    jo("restriction_info", ToJson(*object.restriction_info_));
  }
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
  if (object.reply_markup_) {
    jo("reply_markup", ToJson(*object.reply_markup_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageForwardInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForwardInfo");
  if (object.origin_) {
    jo("origin", ToJson(*object.origin_));
  }
  jo("date", object.date_);
  if (object.source_) {
    jo("source", ToJson(*object.source_));
  }
  jo("public_service_announcement_type", object.public_service_announcement_type_);
}

void to_json(JsonValueScope &jv, const td_api::messageReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReaction");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("total_count", object.total_count_);
  jo("is_chosen", JsonBool{object.is_chosen_});
  if (object.used_sender_id_) {
    jo("used_sender_id", ToJson(*object.used_sender_id_));
  }
  jo("recent_sender_ids", ToJson(object.recent_sender_ids_));
}

void to_json(JsonValueScope &jv, const td_api::MessageSendingState &object) {
  td_api::downcast_call(const_cast<td_api::MessageSendingState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageSendingStatePending &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSendingStatePending");
  jo("sending_id", object.sending_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageSendingStateFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSendingStateFailed");
  if (object.error_) {
    jo("error", ToJson(*object.error_));
  }
  jo("can_retry", JsonBool{object.can_retry_});
  jo("need_another_sender", JsonBool{object.need_another_sender_});
  jo("need_another_reply_quote", JsonBool{object.need_another_reply_quote_});
  jo("need_drop_reply", JsonBool{object.need_drop_reply_});
  jo("required_paid_message_star_count", object.required_paid_message_star_count_);
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::NetworkStatisticsEntry &object) {
  td_api::downcast_call(const_cast<td_api::NetworkStatisticsEntry &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::networkStatisticsEntryFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkStatisticsEntryFile");
  if (object.file_type_) {
    jo("file_type", ToJson(*object.file_type_));
  }
  if (object.network_type_) {
    jo("network_type", ToJson(*object.network_type_));
  }
  jo("sent_bytes", object.sent_bytes_);
  jo("received_bytes", object.received_bytes_);
}

void to_json(JsonValueScope &jv, const td_api::networkStatisticsEntryCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkStatisticsEntryCall");
  if (object.network_type_) {
    jo("network_type", ToJson(*object.network_type_));
  }
  jo("sent_bytes", object.sent_bytes_);
  jo("received_bytes", object.received_bytes_);
  jo("duration", object.duration_);
}

void to_json(JsonValueScope &jv, const td_api::NotificationType &object) {
  td_api::downcast_call(const_cast<td_api::NotificationType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationTypeNewMessage");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("show_preview", JsonBool{object.show_preview_});
}

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewSecretChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationTypeNewSecretChat");
}

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationTypeNewCall");
  jo("call_id", object.call_id_);
}

void to_json(JsonValueScope &jv, const td_api::notificationTypeNewPushMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationTypeNewPushMessage");
  jo("message_id", object.message_id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("sender_name", object.sender_name_);
  jo("is_outgoing", JsonBool{object.is_outgoing_});
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockRelatedArticle &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockRelatedArticle");
  jo("url", object.url_);
  jo("title", object.title_);
  jo("description", object.description_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("author", object.author_);
  jo("publish_date", object.publish_date_);
}

void to_json(JsonValueScope &jv, const td_api::PassportElementErrorSource &object) {
  td_api::downcast_call(const_cast<td_api::PassportElementErrorSource &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceUnspecified &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceUnspecified");
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceDataField &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceDataField");
  jo("field_name", object.field_name_);
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceFrontSide &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceFrontSide");
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceReverseSide &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceReverseSide");
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceSelfie &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceSelfie");
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceTranslationFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceTranslationFile");
  jo("file_index", object.file_index_);
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceTranslationFiles &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceTranslationFiles");
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceFile");
  jo("file_index", object.file_index_);
}

void to_json(JsonValueScope &jv, const td_api::passportElementErrorSourceFiles &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementErrorSourceFiles");
}

void to_json(JsonValueScope &jv, const td_api::paymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentOption");
  jo("title", object.title_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::photoSize &object) {
  auto jo = jv.enter_object();
  jo("@type", "photoSize");
  jo("type", object.type_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("width", object.width_);
  jo("height", object.height_);
  jo("progressive_sizes", ToJson(object.progressive_sizes_));
}

void to_json(JsonValueScope &jv, const td_api::premiumGiftPaymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumGiftPaymentOption");
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("star_count", object.star_count_);
  jo("discount_percentage", object.discount_percentage_);
  jo("month_count", object.month_count_);
  jo("store_product_id", object.store_product_id_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::prepaidGiveaway &object) {
  auto jo = jv.enter_object();
  jo("@type", "prepaidGiveaway");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("winner_count", object.winner_count_);
  if (object.prize_) {
    jo("prize", ToJson(*object.prize_));
  }
  jo("boost_count", object.boost_count_);
  jo("payment_date", object.payment_date_);
}

void to_json(JsonValueScope &jv, const td_api::proxy &object) {
  auto jo = jv.enter_object();
  jo("@type", "proxy");
  jo("id", object.id_);
  jo("server", object.server_);
  jo("port", object.port_);
  jo("last_used_date", object.last_used_date_);
  jo("is_enabled", JsonBool{object.is_enabled_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::quickReplyShortcut &object) {
  auto jo = jv.enter_object();
  jo("@type", "quickReplyShortcut");
  jo("id", object.id_);
  jo("name", object.name_);
  if (object.first_message_) {
    jo("first_message", ToJson(*object.first_message_));
  }
  jo("message_count", object.message_count_);
}

void to_json(JsonValueScope &jv, const td_api::recommendedChatFolders &object) {
  auto jo = jv.enter_object();
  jo("@type", "recommendedChatFolders");
  jo("chat_folders", ToJson(object.chat_folders_));
}

void to_json(JsonValueScope &jv, const td_api::restrictionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "restrictionInfo");
  jo("restriction_reason", object.restriction_reason_);
  jo("has_sensitive_content", JsonBool{object.has_sensitive_content_});
}

void to_json(JsonValueScope &jv, const td_api::scopeAutosaveSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "scopeAutosaveSettings");
  jo("autosave_photos", JsonBool{object.autosave_photos_});
  jo("autosave_videos", JsonBool{object.autosave_videos_});
  jo("max_video_file_size", object.max_video_file_size_);
}

void to_json(JsonValueScope &jv, const td_api::sessions &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessions");
  jo("sessions", ToJson(object.sessions_));
  jo("inactive_session_ttl_days", object.inactive_session_ttl_days_);
}

void to_json(JsonValueScope &jv, const td_api::starAmount &object) {
  auto jo = jv.enter_object();
  jo("@type", "starAmount");
  jo("star_count", object.star_count_);
  jo("nanostar_count", object.nanostar_count_);
}

void to_json(JsonValueScope &jv, const td_api::starSubscription &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscription");
  jo("id", object.id_);
  jo("chat_id", object.chat_id_);
  jo("expiration_date", object.expiration_date_);
  jo("is_canceled", JsonBool{object.is_canceled_});
  jo("is_expiring", JsonBool{object.is_expiring_});
  if (object.pricing_) {
    jo("pricing", ToJson(*object.pricing_));
  }
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::sticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "sticker");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("set_id", ToJson(JsonInt64{object.set_id_}));
  jo("width", object.width_);
  jo("height", object.height_);
  jo("emoji", object.emoji_);
  if (object.format_) {
    jo("format", ToJson(*object.format_));
  }
  if (object.full_type_) {
    jo("full_type", ToJson(*object.full_type_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storageStatisticsByChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "storageStatisticsByChat");
  jo("chat_id", object.chat_id_);
  jo("size", object.size_);
  jo("count", object.count_);
  jo("by_file_type", ToJson(object.by_file_type_));
}

void to_json(JsonValueScope &jv, const td_api::StoryAreaType &object) {
  td_api::downcast_call(const_cast<td_api::StoryAreaType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::storyAreaTypeLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaTypeLocation");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  if (object.address_) {
    jo("address", ToJson(*object.address_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storyAreaTypeVenue &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaTypeVenue");
  if (object.venue_) {
    jo("venue", ToJson(*object.venue_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storyAreaTypeSuggestedReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaTypeSuggestedReaction");
  if (object.reaction_type_) {
    jo("reaction_type", ToJson(*object.reaction_type_));
  }
  jo("total_count", object.total_count_);
  jo("is_dark", JsonBool{object.is_dark_});
  jo("is_flipped", JsonBool{object.is_flipped_});
}

void to_json(JsonValueScope &jv, const td_api::storyAreaTypeMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaTypeMessage");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::storyAreaTypeLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaTypeLink");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::storyAreaTypeWeather &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaTypeWeather");
  jo("temperature", object.temperature_);
  jo("emoji", object.emoji_);
  jo("background_color", object.background_color_);
}

void to_json(JsonValueScope &jv, const td_api::storyAreaTypeUpgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAreaTypeUpgradedGift");
  jo("gift_name", object.gift_name_);
}

void to_json(JsonValueScope &jv, const td_api::StoryPrivacySettings &object) {
  td_api::downcast_call(const_cast<td_api::StoryPrivacySettings &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::storyPrivacySettingsEveryone &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyPrivacySettingsEveryone");
  jo("except_user_ids", ToJson(object.except_user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::storyPrivacySettingsContacts &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyPrivacySettingsContacts");
  jo("except_user_ids", ToJson(object.except_user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::storyPrivacySettingsCloseFriends &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyPrivacySettingsCloseFriends");
}

void to_json(JsonValueScope &jv, const td_api::storyPrivacySettingsSelectedUsers &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyPrivacySettingsSelectedUsers");
  jo("user_ids", ToJson(object.user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::supergroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "supergroup");
  jo("id", object.id_);
  if (object.usernames_) {
    jo("usernames", ToJson(*object.usernames_));
  }
  jo("date", object.date_);
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
  jo("member_count", object.member_count_);
  jo("boost_level", object.boost_level_);
  jo("has_automatic_translation", JsonBool{object.has_automatic_translation_});
  jo("has_linked_chat", JsonBool{object.has_linked_chat_});
  jo("has_location", JsonBool{object.has_location_});
  jo("sign_messages", JsonBool{object.sign_messages_});
  jo("show_message_sender", JsonBool{object.show_message_sender_});
  jo("join_to_send_messages", JsonBool{object.join_to_send_messages_});
  jo("join_by_request", JsonBool{object.join_by_request_});
  jo("is_slow_mode_enabled", JsonBool{object.is_slow_mode_enabled_});
  jo("is_channel", JsonBool{object.is_channel_});
  jo("is_broadcast_group", JsonBool{object.is_broadcast_group_});
  jo("is_forum", JsonBool{object.is_forum_});
  jo("is_direct_messages_group", JsonBool{object.is_direct_messages_group_});
  jo("is_administered_direct_messages_group", JsonBool{object.is_administered_direct_messages_group_});
  if (object.verification_status_) {
    jo("verification_status", ToJson(*object.verification_status_));
  }
  jo("has_direct_messages_group", JsonBool{object.has_direct_messages_group_});
  jo("has_forum_tabs", JsonBool{object.has_forum_tabs_});
  if (object.restriction_info_) {
    jo("restriction_info", ToJson(*object.restriction_info_));
  }
  jo("paid_message_star_count", object.paid_message_star_count_);
  jo("has_active_stories", JsonBool{object.has_active_stories_});
  jo("has_unread_active_stories", JsonBool{object.has_unread_active_stories_});
}

void to_json(JsonValueScope &jv, const td_api::testBytes &object) {
  auto jo = jv.enter_object();
  jo("@type", "testBytes");
  jo("value", base64_encode(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::textEntity &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntity");
  jo("offset", object.offset_);
  jo("length", object.length_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::tonRevenueStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonRevenueStatus");
  jo("total_amount", ToJson(JsonInt64{object.total_amount_}));
  jo("balance_amount", ToJson(JsonInt64{object.balance_amount_}));
  jo("available_amount", ToJson(JsonInt64{object.available_amount_}));
  jo("withdrawal_enabled", JsonBool{object.withdrawal_enabled_});
}

void to_json(JsonValueScope &jv, const td_api::upgradeGiftResult &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradeGiftResult");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  jo("received_gift_id", object.received_gift_id_);
  jo("is_saved", JsonBool{object.is_saved_});
  jo("can_be_transferred", JsonBool{object.can_be_transferred_});
  jo("transfer_star_count", object.transfer_star_count_);
  jo("drop_original_details_star_count", object.drop_original_details_star_count_);
  jo("next_transfer_date", object.next_transfer_date_);
  jo("next_resale_date", object.next_resale_date_);
  jo("export_date", object.export_date_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftOriginalDetails &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftOriginalDetails");
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.receiver_id_) {
    jo("receiver_id", ToJson(*object.receiver_id_));
  }
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRules &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRules");
  jo("rules", ToJson(object.rules_));
}

void to_json(JsonValueScope &jv, const td_api::venue &object) {
  auto jo = jv.enter_object();
  jo("@type", "venue");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("title", object.title_);
  jo("address", object.address_);
  jo("provider", object.provider_);
  jo("id", object.id_);
  jo("type", object.type_);
}

void to_json(JsonValueScope &jv, const td_api::videoStoryboard &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoStoryboard");
  if (object.storyboard_file_) {
    jo("storyboard_file", ToJson(*object.storyboard_file_));
  }
  jo("width", object.width_);
  jo("height", object.height_);
  if (object.map_file_) {
    jo("map_file", ToJson(*object.map_file_));
  }
}

}  // namespace td_api
}  // namespace td
