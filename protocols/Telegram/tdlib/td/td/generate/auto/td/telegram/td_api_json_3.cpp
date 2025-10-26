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
Result<int32> tl_constructor_from_string(td_api::AffiliateType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"affiliateTypeCurrentUser", 1453785589},
    {"affiliateTypeBot", -1032587200},
    {"affiliateTypeChannel", -683939735}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::CallbackQueryPayload *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"callbackQueryPayloadData", -1977729946},
    {"callbackQueryPayloadDataWithPassword", 1340266738},
    {"callbackQueryPayloadGame", 1303571512}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::EmailAddressAuthentication *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"emailAddressAuthenticationCode", -993257022},
    {"emailAddressAuthenticationAppleId", 633948265},
    {"emailAddressAuthenticationGoogleId", -19142846}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InlineKeyboardButtonType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inlineKeyboardButtonTypeUrl", 1130741420},
    {"inlineKeyboardButtonTypeLoginUrl", -1203413081},
    {"inlineKeyboardButtonTypeWebApp", -1767471672},
    {"inlineKeyboardButtonTypeCallback", -1127515139},
    {"inlineKeyboardButtonTypeCallbackWithPassword", 908018248},
    {"inlineKeyboardButtonTypeCallbackGame", -383429528},
    {"inlineKeyboardButtonTypeSwitchInline", 544906485},
    {"inlineKeyboardButtonTypeBuy", 1360739440},
    {"inlineKeyboardButtonTypeUser", 1836574114},
    {"inlineKeyboardButtonTypeCopyText", 68883206}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputInvoice *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputInvoiceMessage", 1490872848},
    {"inputInvoiceName", -1312155917},
    {"inputInvoiceTelegram", -1762853139}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::JsonValue *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"jsonValueNull", -92872499},
    {"jsonValueBoolean", -2142186576},
    {"jsonValueNumber", -1010822033},
    {"jsonValueString", 1597947313},
    {"jsonValueArray", -183913546},
    {"jsonValueObject", 520252026}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::MessageTopic *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"messageTopicThread", 1360920071},
    {"messageTopicForum", 2119440112},
    {"messageTopicDirectMessages", -1285378599},
    {"messageTopicSavedMessages", 588026991}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PremiumFeature *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"premiumFeatureIncreasedLimits", 1785455031},
    {"premiumFeatureIncreasedUploadFileSize", 1825367155},
    {"premiumFeatureImprovedDownloadSpeed", -267695554},
    {"premiumFeatureVoiceRecognition", 1288216542},
    {"premiumFeatureDisabledAds", -2008587702},
    {"premiumFeatureUniqueReactions", 766750743},
    {"premiumFeatureUniqueStickers", -2101773312},
    {"premiumFeatureCustomEmoji", 1332599628},
    {"premiumFeatureAdvancedChatManagement", 796347674},
    {"premiumFeatureProfileBadge", 233648322},
    {"premiumFeatureEmojiStatus", -36516639},
    {"premiumFeatureAnimatedProfilePhoto", -100741914},
    {"premiumFeatureForumTopicIcon", -823172286},
    {"premiumFeatureAppIcons", 1585050761},
    {"premiumFeatureRealTimeChatTranslation", -1143471488},
    {"premiumFeatureUpgradedStories", -1878522597},
    {"premiumFeatureChatBoost", 1576574747},
    {"premiumFeatureAccentColor", 907724190},
    {"premiumFeatureBackgroundForBoth", 575074042},
    {"premiumFeatureSavedMessagesTags", 1003219334},
    {"premiumFeatureMessagePrivacy", 802322678},
    {"premiumFeatureLastSeenTimes", -762230129},
    {"premiumFeatureBusiness", -1503619324},
    {"premiumFeatureMessageEffects", -723300255},
    {"premiumFeatureChecklists", -1128709251}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ReplyMarkup *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"replyMarkupRemoveKeyboard", -691252879},
    {"replyMarkupForceReply", 1101461919},
    {"replyMarkupShowKeyboard", -791495984},
    {"replyMarkupInlineKeyboard", -619317658}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::StoryList *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"storyListMain", -672222209},
    {"storyListArchive", -41900223}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::TopChatCategory *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"topChatCategoryUsers", 1026706816},
    {"topChatCategoryBots", -1577129195},
    {"topChatCategoryGroups", 1530056846},
    {"topChatCategoryChannels", -500825885},
    {"topChatCategoryInlineBots", 377023356},
    {"topChatCategoryWebAppBots", 100062973},
    {"topChatCategoryCalls", 356208861},
    {"topChatCategoryForwardChats", 1695922133}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::affiliateProgramParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.commission_per_mille_, from.extract_field("commission_per_mille")));
  TRY_STATUS(from_json(to.month_count_, from.extract_field("month_count")));
  return Status::OK();
}

Status from_json(td_api::autosaveSettingsScopePrivateChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::backgroundTypeFill &to, JsonObject &from) {
  TRY_STATUS(from_json(to.fill_, from.extract_field("fill")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeAllChatAdministrators &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessBotRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.can_reply_, from.extract_field("can_reply")));
  TRY_STATUS(from_json(to.can_read_messages_, from.extract_field("can_read_messages")));
  TRY_STATUS(from_json(to.can_delete_sent_messages_, from.extract_field("can_delete_sent_messages")));
  TRY_STATUS(from_json(to.can_delete_all_messages_, from.extract_field("can_delete_all_messages")));
  TRY_STATUS(from_json(to.can_edit_name_, from.extract_field("can_edit_name")));
  TRY_STATUS(from_json(to.can_edit_bio_, from.extract_field("can_edit_bio")));
  TRY_STATUS(from_json(to.can_edit_profile_photo_, from.extract_field("can_edit_profile_photo")));
  TRY_STATUS(from_json(to.can_edit_username_, from.extract_field("can_edit_username")));
  TRY_STATUS(from_json(to.can_view_gifts_and_stars_, from.extract_field("can_view_gifts_and_stars")));
  TRY_STATUS(from_json(to.can_sell_gifts_, from.extract_field("can_sell_gifts")));
  TRY_STATUS(from_json(to.can_change_gift_settings_, from.extract_field("can_change_gift_settings")));
  TRY_STATUS(from_json(to.can_transfer_and_upgrade_gifts_, from.extract_field("can_transfer_and_upgrade_gifts")));
  TRY_STATUS(from_json(to.can_transfer_stars_, from.extract_field("can_transfer_stars")));
  TRY_STATUS(from_json(to.can_manage_stories_, from.extract_field("can_manage_stories")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemEcho &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProtocol &to, JsonObject &from) {
  TRY_STATUS(from_json(to.udp_p2p_, from.extract_field("udp_p2p")));
  TRY_STATUS(from_json(to.udp_reflector_, from.extract_field("udp_reflector")));
  TRY_STATUS(from_json(to.min_layer_, from.extract_field("min_layer")));
  TRY_STATUS(from_json(to.max_layer_, from.extract_field("max_layer")));
  TRY_STATUS(from_json(to.library_versions_, from.extract_field("library_versions")));
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionCancel &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatJoinRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  TRY_STATUS(from_json(to.bio_, from.extract_field("bio")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusLeft &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.use_default_mute_for_, from.extract_field("use_default_mute_for")));
  TRY_STATUS(from_json(to.mute_for_, from.extract_field("mute_for")));
  TRY_STATUS(from_json(to.use_default_sound_, from.extract_field("use_default_sound")));
  TRY_STATUS(from_json(to.sound_id_, from.extract_field("sound_id")));
  TRY_STATUS(from_json(to.use_default_show_preview_, from.extract_field("use_default_show_preview")));
  TRY_STATUS(from_json(to.show_preview_, from.extract_field("show_preview")));
  TRY_STATUS(from_json(to.use_default_mute_stories_, from.extract_field("use_default_mute_stories")));
  TRY_STATUS(from_json(to.mute_stories_, from.extract_field("mute_stories")));
  TRY_STATUS(from_json(to.use_default_story_sound_, from.extract_field("use_default_story_sound")));
  TRY_STATUS(from_json(to.story_sound_id_, from.extract_field("story_sound_id")));
  TRY_STATUS(from_json(to.use_default_show_story_poster_, from.extract_field("use_default_show_story_poster")));
  TRY_STATUS(from_json(to.show_story_poster_, from.extract_field("show_story_poster")));
  TRY_STATUS(from_json(to.use_default_disable_pinned_message_notifications_, from.extract_field("use_default_disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.disable_pinned_message_notifications_, from.extract_field("disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.use_default_disable_mention_notifications_, from.extract_field("use_default_disable_mention_notifications")));
  TRY_STATUS(from_json(to.disable_mention_notifications_, from.extract_field("disable_mention_notifications")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenFirebaseCloudMessaging &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  TRY_STATUS(from_json(to.encrypt_, from.extract_field("encrypt")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenBlackBerryPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeEmojiStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeDocument &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSelfDestructingVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::giftResalePriceStar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::groupCallVideoQualityFull &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeSwitchInline &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.target_chat_, from.extract_field("target_chat")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundPrevious &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::inputChecklist &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.tasks_, from.extract_field("tasks")));
  TRY_STATUS(from_json(to.others_can_add_tasks_, from.extract_field("others_can_add_tasks")));
  TRY_STATUS(from_json(to.others_can_mark_tasks_as_done_, from.extract_field("others_can_mark_tasks_as_done")));
  return Status::OK();
}

Status from_json(td_api::inputFileGenerated &to, JsonObject &from) {
  TRY_STATUS(from_json(to.original_path_, from.extract_field("original_path")));
  TRY_STATUS(from_json(to.conversion_, from.extract_field("conversion")));
  TRY_STATUS(from_json(to.expected_size_, from.extract_field("expected_size")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::inputInvoiceTelegram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVideoNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.video_note_, from.extract_field("video_note")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.length_, from.extract_field("length")));
  TRY_STATUS(from_json(to.self_destruct_type_, from.extract_field("self_destruct_type")));
  return Status::OK();
}

Status from_json(td_api::inputMessageStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPersonalDetails &to, JsonObject &from) {
  TRY_STATUS(from_json(to.personal_details_, from.extract_field("personal_details")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPassportRegistration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.passport_registration_, from.extract_field("passport_registration")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceSelfie &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeFoundVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_id_, from.extract_field("query_id")));
  TRY_STATUS(from_json(to.result_id_, from.extract_field("result_id")));
  return Status::OK();
}

Status from_json(td_api::inputStoryContentVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.video_, from.extract_field("video")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, from.extract_field("added_sticker_file_ids")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.cover_frame_timestamp_, from.extract_field("cover_frame_timestamp")));
  TRY_STATUS(from_json(to.is_animation_, from.extract_field("is_animation")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotStart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.autostart_, from.extract_field("autostart")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatInvite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePremiumFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_name_, from.extract_field("sticker_set_name")));
  TRY_STATUS(from_json(to.expect_custom_emoji_, from.extract_field("expect_custom_emoji")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUserToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::jsonValueArray &to, JsonObject &from) {
  TRY_STATUS(from_json(to.values_, from.extract_field("values")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::locationAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  TRY_STATUS(from_json(to.state_, from.extract_field("state")));
  TRY_STATUS(from_json(to.city_, from.extract_field("city")));
  TRY_STATUS(from_json(to.street_, from.extract_field("street")));
  return Status::OK();
}

Status from_json(td_api::messageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.time_, from.extract_field("time")));
  return Status::OK();
}

Status from_json(td_api::messageSenderChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::messageSourceNotification &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkTypeNone &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optionValueBoolean &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::passportElementTypePassport &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeTemporaryRegistration &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::pollTypeQuiz &to, JsonObject &from) {
  TRY_STATUS(from_json(to.correct_option_id_, from.extract_field("correct_option_id")));
  TRY_STATUS(from_json(to.explanation_, from.extract_field("explanation")));
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAdvancedChatManagement &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAccentColor &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypePinnedChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeBioLength &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumSourceLimitExceeded &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_type_, from.extract_field("limit_type")));
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureCustomExpirationDuration &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::profileTabMusic &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSourceNone &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::replyMarkupShowKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rows_, from.extract_field("rows")));
  TRY_STATUS(from_json(to.is_persistent_, from.extract_field("is_persistent")));
  TRY_STATUS(from_json(to.resize_keyboard_, from.extract_field("resize_keyboard")));
  TRY_STATUS(from_json(to.one_time_, from.extract_field("one_time")));
  TRY_STATUS(from_json(to.is_personal_, from.extract_field("is_personal")));
  TRY_STATUS(from_json(to.input_field_placeholder_, from.extract_field("input_field_placeholder")));
  return Status::OK();
}

Status from_json(td_api::reportReasonIllegalDrugs &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesChatTypeFilterChannel &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterUrl &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::shippingOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.price_parts_, from.extract_field("price_parts")));
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposePremiumGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::storyFullId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.poster_chat_id_, from.extract_field("poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionCheckPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionSetProfilePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterSearch &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposePremiumGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.month_count_, from.extract_field("month_count")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::textEntity &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.length_, from.extract_field("length")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBold &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeExpandableBlockQuote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::upgradedGiftAttributeIdModel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_id_, from.extract_field("sticker_id")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowBirthdate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::activateStoryStealthMode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::addFavoriteSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::addProfileAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::addStickerToSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::answerWebAppQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.web_app_query_id_, from.extract_field("web_app_query_id")));
  TRY_STATUS(from_json(to.result_, from.extract_field("result")));
  return Status::OK();
}

Status from_json(td_api::canBotSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::cancelRecoveryEmailAddressVerification &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::checkChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::checkQuickReplyShortcutName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::clearRecentReactions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::closeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::createBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_info_, from.extract_field("link_info")));
  return Status::OK();
}

Status from_json(td_api::createInvoiceLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.invoice_, from.extract_field("invoice")));
  return Status::OK();
}

Status from_json(td_api::createTemporaryPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json(to.valid_for_, from.extract_field("valid_for")));
  return Status::OK();
}

Status from_json(td_api::deleteBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::deleteChatMessagesByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.min_date_, from.extract_field("min_date")));
  TRY_STATUS(from_json(to.max_date_, from.extract_field("max_date")));
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteGiftCollection &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.collection_id_, from.extract_field("collection_id")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedMessagesTopicHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::discardCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.is_disconnected_, from.extract_field("is_disconnected")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.is_video_, from.extract_field("is_video")));
  TRY_STATUS(from_json(to.connection_id_, from.extract_field("connection_id")));
  return Status::OK();
}

Status from_json(td_api::editBusinessMessageChecklist &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.checklist_, from.extract_field("checklist")));
  return Status::OK();
}

Status from_json(td_api::editChatSubscriptionInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::editMessageChecklist &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.checklist_, from.extract_field("checklist")));
  return Status::OK();
}

Status from_json(td_api::editStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  TRY_STATUS(from_json(to.areas_, from.extract_field("areas")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  return Status::OK();
}

Status from_json(td_api::forwardMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.from_chat_id_, from.extract_field("from_chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.send_copy_, from.extract_field("send_copy")));
  TRY_STATUS(from_json(to.remove_caption_, from.extract_field("remove_caption")));
  return Status::OK();
}

Status from_json(td_api::getArchivedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.offset_sticker_set_id_, from.extract_field("offset_sticker_set_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getBankCardInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bank_card_number_, from.extract_field("bank_card_number")));
  return Status::OK();
}

Status from_json(td_api::getBotSimilarBotCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.return_local_, from.extract_field("return_local")));
  return Status::OK();
}

Status from_json(td_api::getCallbackQueryMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.callback_query_id_, from.extract_field("callback_query_id")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.member_id_, from.extract_field("member_id")));
  return Status::OK();
}

Status from_json(td_api::getChatRevenueTransactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatStoryInteractions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.prefer_forwards_, from.extract_field("prefer_forwards")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getConnectedWebsites &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCustomEmojiStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_ids_, from.extract_field("custom_emoji_ids")));
  return Status::OK();
}

Status from_json(td_api::getDirectMessagesChatTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  return Status::OK();
}

Status from_json(td_api::getExternalLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::getForumTopicLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  return Status::OK();
}

Status from_json(td_api::getGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getInternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.is_http_, from.extract_field("is_http")));
  return Status::OK();
}

Status from_json(td_api::getLocalizationTargetInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::getMarkdownText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::getMessageFileType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_file_head_, from.extract_field("message_file_head")));
  return Status::OK();
}

Status from_json(td_api::getMessageThread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getPaidMessageRevenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getPollVoters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.option_id_, from.extract_field("option_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getPremiumStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getRecentEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRepliedMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getSearchSponsoredChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::getStarTransactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.subscription_id_, from.extract_field("subscription_id")));
  TRY_STATUS(from_json(to.direction_, from.extract_field("direction")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getStorageStatisticsFast &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSuggestedStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::getTextEntities &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::getTrendingStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getUserPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.setting_, from.extract_field("setting")));
  return Status::OK();
}

Status from_json(td_api::getVideoMessageAdvertisements &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::importMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_file_, from.extract_field("message_file")));
  TRY_STATUS(from_json(to.attached_files_, from.extract_field("attached_files")));
  return Status::OK();
}

Status from_json(td_api::leaveChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::logOut &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optimizeStorage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.size_, from.extract_field("size")));
  TRY_STATUS(from_json(to.ttl_, from.extract_field("ttl")));
  TRY_STATUS(from_json(to.count_, from.extract_field("count")));
  TRY_STATUS(from_json(to.immunity_delay_, from.extract_field("immunity_delay")));
  TRY_STATUS(from_json(to.file_types_, from.extract_field("file_types")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  TRY_STATUS(from_json(to.exclude_chat_ids_, from.extract_field("exclude_chat_ids")));
  TRY_STATUS(from_json(to.return_deleted_file_statistics_, from.extract_field("return_deleted_file_statistics")));
  TRY_STATUS(from_json(to.chat_limit_, from.extract_field("chat_limit")));
  return Status::OK();
}

Status from_json(td_api::processChatJoinRequests &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.approve_, from.extract_field("approve")));
  return Status::OK();
}

Status from_json(td_api::readChatList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  return Status::OK();
}

Status from_json(td_api::removeAllFilesFromDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_active_, from.extract_field("only_active")));
  TRY_STATUS(from_json(to.only_completed_, from.extract_field("only_completed")));
  TRY_STATUS(from_json(to.delete_from_cache_, from.extract_field("delete_from_cache")));
  return Status::OK();
}

Status from_json(td_api::removeMessageSenderBotVerification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.verified_id_, from.extract_field("verified_id")));
  return Status::OK();
}

Status from_json(td_api::removeSavedAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, from.extract_field("animation")));
  return Status::OK();
}

Status from_json(td_api::reorderChatFolders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_ids_, from.extract_field("chat_folder_ids")));
  TRY_STATUS(from_json(to.main_chat_list_position_, from.extract_field("main_chat_list_position")));
  return Status::OK();
}

Status from_json(td_api::replaceStickerInSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.old_sticker_, from.extract_field("old_sticker")));
  TRY_STATUS(from_json(to.new_sticker_, from.extract_field("new_sticker")));
  return Status::OK();
}

Status from_json(td_api::reportStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json_bytes(to.option_id_, from.extract_field("option_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::resendLoginEmailAddressCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reuseStarSubscription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.subscription_id_, from.extract_field("subscription_id")));
  return Status::OK();
}

Status from_json(td_api::searchChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  return Status::OK();
}

Status from_json(td_api::searchHashtags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.prefix_, from.extract_field("prefix")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchPublicStoriesByTag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchStringsByPrefix &to, JsonObject &from) {
  TRY_STATUS(from_json(to.strings_, from.extract_field("strings")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.return_none_for_empty_query_, from.extract_field("return_none_for_empty_query")));
  return Status::OK();
}

Status from_json(td_api::sendCallDebugInformation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.debug_information_, from.extract_field("debug_information")));
  return Status::OK();
}

Status from_json(td_api::sendInlineQueryResultMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.query_id_, from.extract_field("query_id")));
  TRY_STATUS(from_json(to.result_id_, from.extract_field("result_id")));
  TRY_STATUS(from_json(to.hide_via_bot_, from.extract_field("hide_via_bot")));
  return Status::OK();
}

Status from_json(td_api::sendTextMessageDraft &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  TRY_STATUS(from_json(to.draft_id_, from.extract_field("draft_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::setAuthenticationPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setBotProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::setBusinessGreetingMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.greeting_message_settings_, from.extract_field("greeting_message_settings")));
  return Status::OK();
}

Status from_json(td_api::setChatBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.background_, from.extract_field("background")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.dark_theme_dimming_, from.extract_field("dark_theme_dimming")));
  TRY_STATUS(from_json(to.only_for_self_, from.extract_field("only_for_self")));
  return Status::OK();
}

Status from_json(td_api::setChatMessageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, from.extract_field("message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::setChatTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  return Status::OK();
}

Status from_json(td_api::setDefaultChannelAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.default_channel_administrator_rights_, from.extract_field("default_channel_administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::setGiftCollectionName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.collection_id_, from.extract_field("collection_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::setLogVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.new_verbosity_level_, from.extract_field("new_verbosity_level")));
  return Status::OK();
}

Status from_json(td_api::setNetworkType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::setPinnedForumTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_ids_, from.extract_field("forum_topic_ids")));
  return Status::OK();
}

Status from_json(td_api::setReadDatePrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setStickerSetTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::setTdlibParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.use_test_dc_, from.extract_field("use_test_dc")));
  TRY_STATUS(from_json(to.database_directory_, from.extract_field("database_directory")));
  TRY_STATUS(from_json(to.files_directory_, from.extract_field("files_directory")));
  TRY_STATUS(from_json_bytes(to.database_encryption_key_, from.extract_field("database_encryption_key")));
  TRY_STATUS(from_json(to.use_file_database_, from.extract_field("use_file_database")));
  TRY_STATUS(from_json(to.use_chat_info_database_, from.extract_field("use_chat_info_database")));
  TRY_STATUS(from_json(to.use_message_database_, from.extract_field("use_message_database")));
  TRY_STATUS(from_json(to.use_secret_chats_, from.extract_field("use_secret_chats")));
  TRY_STATUS(from_json(to.api_id_, from.extract_field("api_id")));
  TRY_STATUS(from_json(to.api_hash_, from.extract_field("api_hash")));
  TRY_STATUS(from_json(to.system_language_code_, from.extract_field("system_language_code")));
  TRY_STATUS(from_json(to.device_model_, from.extract_field("device_model")));
  TRY_STATUS(from_json(to.system_version_, from.extract_field("system_version")));
  TRY_STATUS(from_json(to.application_version_, from.extract_field("application_version")));
  return Status::OK();
}

Status from_json(td_api::setVideoChatTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::suggestUserBirthdate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.birthdate_, from.extract_field("birthdate")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorIntObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::toggleAllDownloadsArePaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.are_paused_, from.extract_field("are_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsMarkedAsUnread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_marked_as_unread_, from.extract_field("is_marked_as_unread")));
  return Status::OK();
}

Status from_json(td_api::toggleGiftIsSaved &to, JsonObject &from) {
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  TRY_STATUS(from_json(to.is_saved_, from.extract_field("is_saved")));
  return Status::OK();
}

Status from_json(td_api::toggleSessionCanAcceptCalls &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  TRY_STATUS(from_json(to.can_accept_calls_, from.extract_field("can_accept_calls")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsForum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.is_forum_, from.extract_field("is_forum")));
  TRY_STATUS(from_json(to.has_forum_tabs_, from.extract_field("has_forum_tabs")));
  return Status::OK();
}

Status from_json(td_api::transferChatOwnership &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::upgradeGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  TRY_STATUS(from_json(to.keep_original_details_, from.extract_field("keep_original_details")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::accentColor &object) {
  auto jo = jv.enter_object();
  jo("@type", "accentColor");
  jo("id", object.id_);
  jo("built_in_accent_color_id", object.built_in_accent_color_id_);
  jo("light_theme_colors", ToJson(object.light_theme_colors_));
  jo("dark_theme_colors", ToJson(object.dark_theme_colors_));
  jo("min_channel_chat_boost_level", object.min_channel_chat_boost_level_);
}

void to_json(JsonValueScope &jv, const td_api::affiliateProgramInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "affiliateProgramInfo");
  if (object.parameters_) {
    jo("parameters", ToJson(*object.parameters_));
  }
  jo("end_date", object.end_date_);
  if (object.daily_revenue_per_user_amount_) {
    jo("daily_revenue_per_user_amount", ToJson(*object.daily_revenue_per_user_amount_));
  }
}

void to_json(JsonValueScope &jv, const td_api::attachmentMenuBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "attachmentMenuBot");
  jo("bot_user_id", object.bot_user_id_);
  jo("supports_self_chat", JsonBool{object.supports_self_chat_});
  jo("supports_user_chats", JsonBool{object.supports_user_chats_});
  jo("supports_bot_chats", JsonBool{object.supports_bot_chats_});
  jo("supports_group_chats", JsonBool{object.supports_group_chats_});
  jo("supports_channel_chats", JsonBool{object.supports_channel_chats_});
  jo("request_write_access", JsonBool{object.request_write_access_});
  jo("is_added", JsonBool{object.is_added_});
  jo("show_in_attachment_menu", JsonBool{object.show_in_attachment_menu_});
  jo("show_in_side_menu", JsonBool{object.show_in_side_menu_});
  jo("show_disclaimer_in_side_menu", JsonBool{object.show_disclaimer_in_side_menu_});
  jo("name", object.name_);
  if (object.name_color_) {
    jo("name_color", ToJson(*object.name_color_));
  }
  if (object.default_icon_) {
    jo("default_icon", ToJson(*object.default_icon_));
  }
  if (object.ios_static_icon_) {
    jo("ios_static_icon", ToJson(*object.ios_static_icon_));
  }
  if (object.ios_animated_icon_) {
    jo("ios_animated_icon", ToJson(*object.ios_animated_icon_));
  }
  if (object.ios_side_menu_icon_) {
    jo("ios_side_menu_icon", ToJson(*object.ios_side_menu_icon_));
  }
  if (object.android_icon_) {
    jo("android_icon", ToJson(*object.android_icon_));
  }
  if (object.android_side_menu_icon_) {
    jo("android_side_menu_icon", ToJson(*object.android_side_menu_icon_));
  }
  if (object.macos_icon_) {
    jo("macos_icon", ToJson(*object.macos_icon_));
  }
  if (object.macos_side_menu_icon_) {
    jo("macos_side_menu_icon", ToJson(*object.macos_side_menu_icon_));
  }
  if (object.icon_color_) {
    jo("icon_color", ToJson(*object.icon_color_));
  }
  if (object.web_app_placeholder_) {
    jo("web_app_placeholder", ToJson(*object.web_app_placeholder_));
  }
}

void to_json(JsonValueScope &jv, const td_api::autosaveSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "autosaveSettings");
  if (object.private_chat_settings_) {
    jo("private_chat_settings", ToJson(*object.private_chat_settings_));
  }
  if (object.group_settings_) {
    jo("group_settings", ToJson(*object.group_settings_));
  }
  if (object.channel_settings_) {
    jo("channel_settings", ToJson(*object.channel_settings_));
  }
  jo("exceptions", ToJson(object.exceptions_));
}

void to_json(JsonValueScope &jv, const td_api::BackgroundType &object) {
  td_api::downcast_call(const_cast<td_api::BackgroundType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::backgroundTypeWallpaper &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgroundTypeWallpaper");
  jo("is_blurred", JsonBool{object.is_blurred_});
  jo("is_moving", JsonBool{object.is_moving_});
}

void to_json(JsonValueScope &jv, const td_api::backgroundTypePattern &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgroundTypePattern");
  if (object.fill_) {
    jo("fill", ToJson(*object.fill_));
  }
  jo("intensity", object.intensity_);
  jo("is_inverted", JsonBool{object.is_inverted_});
  jo("is_moving", JsonBool{object.is_moving_});
}

void to_json(JsonValueScope &jv, const td_api::backgroundTypeFill &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgroundTypeFill");
  if (object.fill_) {
    jo("fill", ToJson(*object.fill_));
  }
}

void to_json(JsonValueScope &jv, const td_api::backgroundTypeChatTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgroundTypeChatTheme");
  jo("theme_name", object.theme_name_);
}

void to_json(JsonValueScope &jv, const td_api::botCommands &object) {
  auto jo = jv.enter_object();
  jo("@type", "botCommands");
  jo("bot_user_id", object.bot_user_id_);
  jo("commands", ToJson(object.commands_));
}

void to_json(JsonValueScope &jv, const td_api::BuiltInTheme &object) {
  td_api::downcast_call(const_cast<td_api::BuiltInTheme &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::builtInThemeClassic &object) {
  auto jo = jv.enter_object();
  jo("@type", "builtInThemeClassic");
}

void to_json(JsonValueScope &jv, const td_api::builtInThemeDay &object) {
  auto jo = jv.enter_object();
  jo("@type", "builtInThemeDay");
}

void to_json(JsonValueScope &jv, const td_api::builtInThemeNight &object) {
  auto jo = jv.enter_object();
  jo("@type", "builtInThemeNight");
}

void to_json(JsonValueScope &jv, const td_api::builtInThemeTinted &object) {
  auto jo = jv.enter_object();
  jo("@type", "builtInThemeTinted");
}

void to_json(JsonValueScope &jv, const td_api::builtInThemeArctic &object) {
  auto jo = jv.enter_object();
  jo("@type", "builtInThemeArctic");
}

void to_json(JsonValueScope &jv, const td_api::businessConnection &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessConnection");
  jo("id", object.id_);
  jo("user_id", object.user_id_);
  jo("user_chat_id", object.user_chat_id_);
  jo("date", object.date_);
  if (object.rights_) {
    jo("rights", ToJson(*object.rights_));
  }
  jo("is_enabled", JsonBool{object.is_enabled_});
}

void to_json(JsonValueScope &jv, const td_api::businessOpeningHours &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessOpeningHours");
  jo("time_zone_id", object.time_zone_id_);
  jo("opening_hours", ToJson(object.opening_hours_));
}

void to_json(JsonValueScope &jv, const td_api::CallServerType &object) {
  td_api::downcast_call(const_cast<td_api::CallServerType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::callServerTypeTelegramReflector &object) {
  auto jo = jv.enter_object();
  jo("@type", "callServerTypeTelegramReflector");
  jo("peer_tag", base64_encode(object.peer_tag_));
  jo("is_tcp", JsonBool{object.is_tcp_});
}

void to_json(JsonValueScope &jv, const td_api::callServerTypeWebrtc &object) {
  auto jo = jv.enter_object();
  jo("@type", "callServerTypeWebrtc");
  jo("username", object.username_);
  jo("password", object.password_);
  jo("supports_turn", JsonBool{object.supports_turn_});
  jo("supports_stun", JsonBool{object.supports_stun_});
}

void to_json(JsonValueScope &jv, const td_api::ChatAction &object) {
  td_api::downcast_call(const_cast<td_api::ChatAction &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatActionTyping &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionTyping");
}

void to_json(JsonValueScope &jv, const td_api::chatActionRecordingVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionRecordingVideo");
}

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionUploadingVideo");
  jo("progress", object.progress_);
}

void to_json(JsonValueScope &jv, const td_api::chatActionRecordingVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionRecordingVoiceNote");
}

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionUploadingVoiceNote");
  jo("progress", object.progress_);
}

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionUploadingPhoto");
  jo("progress", object.progress_);
}

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionUploadingDocument");
  jo("progress", object.progress_);
}

void to_json(JsonValueScope &jv, const td_api::chatActionChoosingSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionChoosingSticker");
}

void to_json(JsonValueScope &jv, const td_api::chatActionChoosingLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionChoosingLocation");
}

void to_json(JsonValueScope &jv, const td_api::chatActionChoosingContact &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionChoosingContact");
}

void to_json(JsonValueScope &jv, const td_api::chatActionStartPlayingGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionStartPlayingGame");
}

void to_json(JsonValueScope &jv, const td_api::chatActionRecordingVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionRecordingVideoNote");
}

void to_json(JsonValueScope &jv, const td_api::chatActionUploadingVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionUploadingVideoNote");
  jo("progress", object.progress_);
}

void to_json(JsonValueScope &jv, const td_api::chatActionWatchingAnimations &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionWatchingAnimations");
  jo("emoji", object.emoji_);
}

void to_json(JsonValueScope &jv, const td_api::chatActionCancel &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionCancel");
}

void to_json(JsonValueScope &jv, const td_api::chatBoostFeatures &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostFeatures");
  jo("features", ToJson(object.features_));
  jo("min_profile_background_custom_emoji_boost_level", object.min_profile_background_custom_emoji_boost_level_);
  jo("min_background_custom_emoji_boost_level", object.min_background_custom_emoji_boost_level_);
  jo("min_emoji_status_boost_level", object.min_emoji_status_boost_level_);
  jo("min_chat_theme_background_boost_level", object.min_chat_theme_background_boost_level_);
  jo("min_custom_background_boost_level", object.min_custom_background_boost_level_);
  jo("min_custom_emoji_sticker_set_boost_level", object.min_custom_emoji_sticker_set_boost_level_);
  jo("min_automatic_translation_boost_level", object.min_automatic_translation_boost_level_);
  jo("min_speech_recognition_boost_level", object.min_speech_recognition_boost_level_);
  jo("min_sponsored_message_disable_boost_level", object.min_sponsored_message_disable_boost_level_);
}

void to_json(JsonValueScope &jv, const td_api::ChatEventAction &object) {
  td_api::downcast_call(const_cast<td_api::ChatEventAction &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatEventMessageEdited &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMessageEdited");
  if (object.old_message_) {
    jo("old_message", ToJson(*object.old_message_));
  }
  if (object.new_message_) {
    jo("new_message", ToJson(*object.new_message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMessageDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMessageDeleted");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("can_report_anti_spam_false_positive", JsonBool{object.can_report_anti_spam_false_positive_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventMessagePinned &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMessagePinned");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMessageUnpinned &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMessageUnpinned");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventPollStopped &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventPollStopped");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberJoined &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberJoined");
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberJoinedByInviteLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberJoinedByInviteLink");
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
  jo("via_chat_folder_invite_link", JsonBool{object.via_chat_folder_invite_link_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberJoinedByRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberJoinedByRequest");
  jo("approver_user_id", object.approver_user_id_);
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberInvited &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberInvited");
  jo("user_id", object.user_id_);
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberLeft &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberLeft");
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberPromoted &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberPromoted");
  jo("user_id", object.user_id_);
  if (object.old_status_) {
    jo("old_status", ToJson(*object.old_status_));
  }
  if (object.new_status_) {
    jo("new_status", ToJson(*object.new_status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberRestricted &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberRestricted");
  if (object.member_id_) {
    jo("member_id", ToJson(*object.member_id_));
  }
  if (object.old_status_) {
    jo("old_status", ToJson(*object.old_status_));
  }
  if (object.new_status_) {
    jo("new_status", ToJson(*object.new_status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMemberSubscriptionExtended &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMemberSubscriptionExtended");
  jo("user_id", object.user_id_);
  if (object.old_status_) {
    jo("old_status", ToJson(*object.old_status_));
  }
  if (object.new_status_) {
    jo("new_status", ToJson(*object.new_status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventAvailableReactionsChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventAvailableReactionsChanged");
  if (object.old_available_reactions_) {
    jo("old_available_reactions", ToJson(*object.old_available_reactions_));
  }
  if (object.new_available_reactions_) {
    jo("new_available_reactions", ToJson(*object.new_available_reactions_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventBackgroundChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventBackgroundChanged");
  if (object.old_background_) {
    jo("old_background", ToJson(*object.old_background_));
  }
  if (object.new_background_) {
    jo("new_background", ToJson(*object.new_background_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventDescriptionChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventDescriptionChanged");
  jo("old_description", object.old_description_);
  jo("new_description", object.new_description_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventEmojiStatusChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventEmojiStatusChanged");
  if (object.old_emoji_status_) {
    jo("old_emoji_status", ToJson(*object.old_emoji_status_));
  }
  if (object.new_emoji_status_) {
    jo("new_emoji_status", ToJson(*object.new_emoji_status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventLinkedChatChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventLinkedChatChanged");
  jo("old_linked_chat_id", object.old_linked_chat_id_);
  jo("new_linked_chat_id", object.new_linked_chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventLocationChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventLocationChanged");
  if (object.old_location_) {
    jo("old_location", ToJson(*object.old_location_));
  }
  if (object.new_location_) {
    jo("new_location", ToJson(*object.new_location_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventMessageAutoDeleteTimeChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventMessageAutoDeleteTimeChanged");
  jo("old_message_auto_delete_time", object.old_message_auto_delete_time_);
  jo("new_message_auto_delete_time", object.new_message_auto_delete_time_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventPermissionsChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventPermissionsChanged");
  if (object.old_permissions_) {
    jo("old_permissions", ToJson(*object.old_permissions_));
  }
  if (object.new_permissions_) {
    jo("new_permissions", ToJson(*object.new_permissions_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventPhotoChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventPhotoChanged");
  if (object.old_photo_) {
    jo("old_photo", ToJson(*object.old_photo_));
  }
  if (object.new_photo_) {
    jo("new_photo", ToJson(*object.new_photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventSlowModeDelayChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventSlowModeDelayChanged");
  jo("old_slow_mode_delay", object.old_slow_mode_delay_);
  jo("new_slow_mode_delay", object.new_slow_mode_delay_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventStickerSetChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventStickerSetChanged");
  jo("old_sticker_set_id", ToJson(JsonInt64{object.old_sticker_set_id_}));
  jo("new_sticker_set_id", ToJson(JsonInt64{object.new_sticker_set_id_}));
}

void to_json(JsonValueScope &jv, const td_api::chatEventCustomEmojiStickerSetChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventCustomEmojiStickerSetChanged");
  jo("old_sticker_set_id", ToJson(JsonInt64{object.old_sticker_set_id_}));
  jo("new_sticker_set_id", ToJson(JsonInt64{object.new_sticker_set_id_}));
}

void to_json(JsonValueScope &jv, const td_api::chatEventTitleChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventTitleChanged");
  jo("old_title", object.old_title_);
  jo("new_title", object.new_title_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventUsernameChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventUsernameChanged");
  jo("old_username", object.old_username_);
  jo("new_username", object.new_username_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventActiveUsernamesChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventActiveUsernamesChanged");
  jo("old_usernames", ToJson(object.old_usernames_));
  jo("new_usernames", ToJson(object.new_usernames_));
}

void to_json(JsonValueScope &jv, const td_api::chatEventAccentColorChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventAccentColorChanged");
  jo("old_accent_color_id", object.old_accent_color_id_);
  jo("old_background_custom_emoji_id", ToJson(JsonInt64{object.old_background_custom_emoji_id_}));
  jo("new_accent_color_id", object.new_accent_color_id_);
  jo("new_background_custom_emoji_id", ToJson(JsonInt64{object.new_background_custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::chatEventProfileAccentColorChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventProfileAccentColorChanged");
  jo("old_profile_accent_color_id", object.old_profile_accent_color_id_);
  jo("old_profile_background_custom_emoji_id", ToJson(JsonInt64{object.old_profile_background_custom_emoji_id_}));
  jo("new_profile_accent_color_id", object.new_profile_accent_color_id_);
  jo("new_profile_background_custom_emoji_id", ToJson(JsonInt64{object.new_profile_background_custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::chatEventHasProtectedContentToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventHasProtectedContentToggled");
  jo("has_protected_content", JsonBool{object.has_protected_content_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventInvitesToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventInvitesToggled");
  jo("can_invite_users", JsonBool{object.can_invite_users_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventIsAllHistoryAvailableToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventIsAllHistoryAvailableToggled");
  jo("is_all_history_available", JsonBool{object.is_all_history_available_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventHasAggressiveAntiSpamEnabledToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventHasAggressiveAntiSpamEnabledToggled");
  jo("has_aggressive_anti_spam_enabled", JsonBool{object.has_aggressive_anti_spam_enabled_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventSignMessagesToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventSignMessagesToggled");
  jo("sign_messages", JsonBool{object.sign_messages_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventShowMessageSenderToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventShowMessageSenderToggled");
  jo("show_message_sender", JsonBool{object.show_message_sender_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventAutomaticTranslationToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventAutomaticTranslationToggled");
  jo("has_automatic_translation", JsonBool{object.has_automatic_translation_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventInviteLinkEdited &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventInviteLinkEdited");
  if (object.old_invite_link_) {
    jo("old_invite_link", ToJson(*object.old_invite_link_));
  }
  if (object.new_invite_link_) {
    jo("new_invite_link", ToJson(*object.new_invite_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventInviteLinkRevoked &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventInviteLinkRevoked");
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventInviteLinkDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventInviteLinkDeleted");
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatCreated &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventVideoChatCreated");
  jo("group_call_id", object.group_call_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatEnded &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventVideoChatEnded");
  jo("group_call_id", object.group_call_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatMuteNewParticipantsToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventVideoChatMuteNewParticipantsToggled");
  jo("mute_new_participants", JsonBool{object.mute_new_participants_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatParticipantIsMutedToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventVideoChatParticipantIsMutedToggled");
  if (object.participant_id_) {
    jo("participant_id", ToJson(*object.participant_id_));
  }
  jo("is_muted", JsonBool{object.is_muted_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventVideoChatParticipantVolumeLevelChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventVideoChatParticipantVolumeLevelChanged");
  if (object.participant_id_) {
    jo("participant_id", ToJson(*object.participant_id_));
  }
  jo("volume_level", object.volume_level_);
}

void to_json(JsonValueScope &jv, const td_api::chatEventIsForumToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventIsForumToggled");
  jo("is_forum", JsonBool{object.is_forum_});
}

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicCreated &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventForumTopicCreated");
  if (object.topic_info_) {
    jo("topic_info", ToJson(*object.topic_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicEdited &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventForumTopicEdited");
  if (object.old_topic_info_) {
    jo("old_topic_info", ToJson(*object.old_topic_info_));
  }
  if (object.new_topic_info_) {
    jo("new_topic_info", ToJson(*object.new_topic_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicToggleIsClosed &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventForumTopicToggleIsClosed");
  if (object.topic_info_) {
    jo("topic_info", ToJson(*object.topic_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicToggleIsHidden &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventForumTopicToggleIsHidden");
  if (object.topic_info_) {
    jo("topic_info", ToJson(*object.topic_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventForumTopicDeleted");
  if (object.topic_info_) {
    jo("topic_info", ToJson(*object.topic_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatEventForumTopicPinned &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventForumTopicPinned");
  if (object.old_topic_info_) {
    jo("old_topic_info", ToJson(*object.old_topic_info_));
  }
  if (object.new_topic_info_) {
    jo("new_topic_info", ToJson(*object.new_topic_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLink");
  jo("invite_link", object.invite_link_);
  jo("name", object.name_);
  jo("creator_user_id", object.creator_user_id_);
  jo("date", object.date_);
  jo("edit_date", object.edit_date_);
  jo("expiration_date", object.expiration_date_);
  if (object.subscription_pricing_) {
    jo("subscription_pricing", ToJson(*object.subscription_pricing_));
  }
  jo("member_limit", object.member_limit_);
  jo("member_count", object.member_count_);
  jo("expired_member_count", object.expired_member_count_);
  jo("pending_join_request_count", object.pending_join_request_count_);
  jo("creates_join_request", JsonBool{object.creates_join_request_});
  jo("is_primary", JsonBool{object.is_primary_});
  jo("is_revoked", JsonBool{object.is_revoked_});
}

void to_json(JsonValueScope &jv, const td_api::chatJoinRequests &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatJoinRequests");
  jo("total_count", object.total_count_);
  jo("requests", ToJson(object.requests_));
}

void to_json(JsonValueScope &jv, const td_api::chatMessageSenders &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMessageSenders");
  jo("senders", ToJson(object.senders_));
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueAmount &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueAmount");
  jo("cryptocurrency", object.cryptocurrency_);
  jo("total_amount", ToJson(JsonInt64{object.total_amount_}));
  jo("balance_amount", ToJson(JsonInt64{object.balance_amount_}));
  jo("available_amount", ToJson(JsonInt64{object.available_amount_}));
  jo("withdrawal_enabled", JsonBool{object.withdrawal_enabled_});
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsInviterInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsInviterInfo");
  jo("user_id", object.user_id_);
  jo("added_member_count", object.added_member_count_);
}

void to_json(JsonValueScope &jv, const td_api::checklistTask &object) {
  auto jo = jv.enter_object();
  jo("@type", "checklistTask");
  jo("id", object.id_);
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("completed_by_user_id", object.completed_by_user_id_);
  jo("completion_date", object.completion_date_);
}

void to_json(JsonValueScope &jv, const td_api::contact &object) {
  auto jo = jv.enter_object();
  jo("@type", "contact");
  jo("phone_number", object.phone_number_);
  jo("first_name", object.first_name_);
  jo("last_name", object.last_name_);
  jo("vcard", object.vcard_);
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::date &object) {
  auto jo = jv.enter_object();
  jo("@type", "date");
  jo("day", object.day_);
  jo("month", object.month_);
  jo("year", object.year_);
}

void to_json(JsonValueScope &jv, const td_api::emailAddressAuthenticationCodeInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "emailAddressAuthenticationCodeInfo");
  jo("email_address_pattern", object.email_address_pattern_);
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::emojiStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatus");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("expiration_date", object.expiration_date_);
}

void to_json(JsonValueScope &jv, const td_api::failedToAddMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "failedToAddMember");
  jo("user_id", object.user_id_);
  jo("premium_would_allow_invite", JsonBool{object.premium_would_allow_invite_});
  jo("premium_required_to_send_messages", JsonBool{object.premium_required_to_send_messages_});
}

void to_json(JsonValueScope &jv, const td_api::forumTopicIcon &object) {
  auto jo = jv.enter_object();
  jo("@type", "forumTopicIcon");
  jo("color", object.color_);
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::foundMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundMessages");
  jo("total_count", object.total_count_);
  jo("messages", ToJson(object.messages_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::gameHighScores &object) {
  auto jo = jv.enter_object();
  jo("@type", "gameHighScores");
  jo("scores", ToJson(object.scores_));
}

void to_json(JsonValueScope &jv, const td_api::GiftResalePrice &object) {
  td_api::downcast_call(const_cast<td_api::GiftResalePrice &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::giftResalePriceStar &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftResalePriceStar");
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::giftResalePriceTon &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftResalePriceTon");
  jo("toncoin_cent_count", object.toncoin_cent_count_);
}

void to_json(JsonValueScope &jv, const td_api::GiveawayPrize &object) {
  td_api::downcast_call(const_cast<td_api::GiveawayPrize &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::giveawayPrizePremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayPrizePremium");
  jo("month_count", object.month_count_);
}

void to_json(JsonValueScope &jv, const td_api::giveawayPrizeStars &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayPrizeStars");
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::hashtags &object) {
  auto jo = jv.enter_object();
  jo("@type", "hashtags");
  jo("hashtags", ToJson(object.hashtags_));
}

void to_json(JsonValueScope &jv, const td_api::InlineQueryResultsButtonType &object) {
  td_api::downcast_call(const_cast<td_api::InlineQueryResultsButtonType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultsButtonTypeStartBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultsButtonTypeStartBot");
  jo("parameter", object.parameter_);
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultsButtonTypeWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultsButtonTypeWebApp");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::inputTextQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputTextQuote");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("position", object.position_);
}

void to_json(JsonValueScope &jv, const td_api::KeyboardButtonType &object) {
  td_api::downcast_call(const_cast<td_api::KeyboardButtonType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeText &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeText");
}

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeRequestPhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeRequestLocation");
}

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestPoll &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeRequestPoll");
  jo("force_regular", JsonBool{object.force_regular_});
  jo("force_quiz", JsonBool{object.force_quiz_});
}

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestUsers &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeRequestUsers");
  jo("id", object.id_);
  jo("restrict_user_is_bot", JsonBool{object.restrict_user_is_bot_});
  jo("user_is_bot", JsonBool{object.user_is_bot_});
  jo("restrict_user_is_premium", JsonBool{object.restrict_user_is_premium_});
  jo("user_is_premium", JsonBool{object.user_is_premium_});
  jo("max_quantity", object.max_quantity_);
  jo("request_name", JsonBool{object.request_name_});
  jo("request_username", JsonBool{object.request_username_});
  jo("request_photo", JsonBool{object.request_photo_});
}

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeRequestChat");
  jo("id", object.id_);
  jo("chat_is_channel", JsonBool{object.chat_is_channel_});
  jo("restrict_chat_is_forum", JsonBool{object.restrict_chat_is_forum_});
  jo("chat_is_forum", JsonBool{object.chat_is_forum_});
  jo("restrict_chat_has_username", JsonBool{object.restrict_chat_has_username_});
  jo("chat_has_username", JsonBool{object.chat_has_username_});
  jo("chat_is_created", JsonBool{object.chat_is_created_});
  if (object.user_administrator_rights_) {
    jo("user_administrator_rights", ToJson(*object.user_administrator_rights_));
  }
  if (object.bot_administrator_rights_) {
    jo("bot_administrator_rights", ToJson(*object.bot_administrator_rights_));
  }
  jo("bot_is_member", JsonBool{object.bot_is_member_});
  jo("request_title", JsonBool{object.request_title_});
  jo("request_username", JsonBool{object.request_username_});
  jo("request_photo", JsonBool{object.request_photo_});
}

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeWebApp");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::LinkPreviewType &object) {
  td_api::downcast_call(const_cast<td_api::LinkPreviewType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeAlbum &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeAlbum");
  jo("media", ToJson(object.media_));
  jo("caption", object.caption_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeAnimation");
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeApp");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeArticle &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeArticle");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeAudio");
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeBackground");
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  if (object.background_type_) {
    jo("background_type", ToJson(*object.background_type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeChannelBoost &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeChannelBoost");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeChat");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("creates_join_request", JsonBool{object.creates_join_request_});
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeDirectMessagesChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeDirectMessagesChat");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeDocument");
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeEmbeddedAnimationPlayer &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeEmbeddedAnimationPlayer");
  jo("url", object.url_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeEmbeddedAudioPlayer &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeEmbeddedAudioPlayer");
  jo("url", object.url_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeEmbeddedVideoPlayer &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeEmbeddedVideoPlayer");
  jo("url", object.url_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeExternalAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeExternalAudio");
  jo("url", object.url_);
  jo("mime_type", object.mime_type_);
  jo("duration", object.duration_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeExternalVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeExternalVideo");
  jo("url", object.url_);
  jo("mime_type", object.mime_type_);
  jo("width", object.width_);
  jo("height", object.height_);
  jo("duration", object.duration_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeGiftCollection &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeGiftCollection");
  jo("icons", ToJson(object.icons_));
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeGroupCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeGroupCall");
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeInvoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeInvoice");
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeMessage");
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypePhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypePremiumGiftCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypePremiumGiftCode");
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeShareableChatFolder &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeShareableChatFolder");
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeSticker");
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeStickerSet &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeStickerSet");
  jo("stickers", ToJson(object.stickers_));
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeStory");
  jo("story_poster_chat_id", object.story_poster_chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeStoryAlbum &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeStoryAlbum");
  if (object.photo_icon_) {
    jo("photo_icon", ToJson(*object.photo_icon_));
  }
  if (object.video_icon_) {
    jo("video_icon", ToJson(*object.video_icon_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeSupergroupBoost &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeSupergroupBoost");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeTheme");
  jo("documents", ToJson(object.documents_));
  if (object.settings_) {
    jo("settings", ToJson(*object.settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeUnsupported");
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeUpgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeUpgradedGift");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeUser");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("is_bot", JsonBool{object.is_bot_});
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.cover_) {
    jo("cover", ToJson(*object.cover_));
  }
  jo("start_timestamp", object.start_timestamp_);
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeVideoChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeVideoChat");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("is_live_stream", JsonBool{object.is_live_stream_});
  jo("joins_as_speaker", JsonBool{object.joins_as_speaker_});
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeVideoNote");
  if (object.video_note_) {
    jo("video_note", ToJson(*object.video_note_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeVoiceNote");
  if (object.voice_note_) {
    jo("voice_note", ToJson(*object.voice_note_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeWebApp");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::mainWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "mainWebApp");
  jo("url", object.url_);
  if (object.mode_) {
    jo("mode", ToJson(*object.mode_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageEffect &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageEffect");
  jo("id", ToJson(JsonInt64{object.id_}));
  if (object.static_icon_) {
    jo("static_icon", ToJson(*object.static_icon_));
  }
  jo("emoji", object.emoji_);
  jo("is_premium", JsonBool{object.is_premium_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messagePosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePosition");
  jo("position", object.position_);
  jo("message_id", object.message_id_);
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::MessageSelfDestructType &object) {
  td_api::downcast_call(const_cast<td_api::MessageSelfDestructType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageSelfDestructTypeTimer &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSelfDestructTypeTimer");
  jo("self_destruct_time", object.self_destruct_time_);
}

void to_json(JsonValueScope &jv, const td_api::messageSelfDestructTypeImmediately &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSelfDestructTypeImmediately");
}

void to_json(JsonValueScope &jv, const td_api::messages &object) {
  auto jo = jv.enter_object();
  jo("@type", "messages");
  jo("total_count", object.total_count_);
  jo("messages", ToJson(object.messages_));
}

void to_json(JsonValueScope &jv, const td_api::NotificationSettingsScope &object) {
  td_api::downcast_call(const_cast<td_api::NotificationSettingsScope &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::notificationSettingsScopePrivateChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationSettingsScopePrivateChats");
}

void to_json(JsonValueScope &jv, const td_api::notificationSettingsScopeGroupChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationSettingsScopeGroupChats");
}

void to_json(JsonValueScope &jv, const td_api::notificationSettingsScopeChannelChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationSettingsScopeChannelChats");
}

void to_json(JsonValueScope &jv, const td_api::pageBlockCaption &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockCaption");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  if (object.credit_) {
    jo("credit", ToJson(*object.credit_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportAuthorizationForm &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportAuthorizationForm");
  jo("id", object.id_);
  jo("required_elements", ToJson(object.required_elements_));
  jo("privacy_policy_url", object.privacy_policy_url_);
}

void to_json(JsonValueScope &jv, const td_api::passwordState &object) {
  auto jo = jv.enter_object();
  jo("@type", "passwordState");
  jo("has_password", JsonBool{object.has_password_});
  jo("password_hint", object.password_hint_);
  jo("has_recovery_email_address", JsonBool{object.has_recovery_email_address_});
  jo("has_passport_data", JsonBool{object.has_passport_data_});
  if (object.recovery_email_address_code_info_) {
    jo("recovery_email_address_code_info", ToJson(*object.recovery_email_address_code_info_));
  }
  jo("login_email_address_pattern", object.login_email_address_pattern_);
  jo("pending_reset_date", object.pending_reset_date_);
}

void to_json(JsonValueScope &jv, const td_api::personalDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "personalDocument");
  jo("files", ToJson(object.files_));
  jo("translation", ToJson(object.translation_));
}

void to_json(JsonValueScope &jv, const td_api::premiumFeaturePromotionAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeaturePromotionAnimation");
  if (object.feature_) {
    jo("feature", ToJson(*object.feature_));
  }
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::premiumPaymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumPaymentOption");
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("discount_percentage", object.discount_percentage_);
  jo("month_count", object.month_count_);
  jo("store_product_id", object.store_product_id_);
  if (object.payment_link_) {
    jo("payment_link", ToJson(*object.payment_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::profilePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "profilePhoto");
  jo("id", ToJson(JsonInt64{object.id_}));
  if (object.small_) {
    jo("small", ToJson(*object.small_));
  }
  if (object.big_) {
    jo("big", ToJson(*object.big_));
  }
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  jo("has_animation", JsonBool{object.has_animation_});
  jo("is_personal", JsonBool{object.is_personal_});
}

void to_json(JsonValueScope &jv, const td_api::pushReceiverId &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushReceiverId");
  jo("id", ToJson(JsonInt64{object.id_}));
}

void to_json(JsonValueScope &jv, const td_api::receivedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "receivedGift");
  jo("received_gift_id", object.received_gift_id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("is_private", JsonBool{object.is_private_});
  jo("is_saved", JsonBool{object.is_saved_});
  jo("is_pinned", JsonBool{object.is_pinned_});
  jo("can_be_upgraded", JsonBool{object.can_be_upgraded_});
  jo("can_be_transferred", JsonBool{object.can_be_transferred_});
  jo("was_refunded", JsonBool{object.was_refunded_});
  jo("date", object.date_);
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  jo("collection_ids", ToJson(object.collection_ids_));
  jo("sell_star_count", object.sell_star_count_);
  jo("prepaid_upgrade_star_count", object.prepaid_upgrade_star_count_);
  jo("is_upgrade_separate", JsonBool{object.is_upgrade_separate_});
  jo("transfer_star_count", object.transfer_star_count_);
  jo("drop_original_details_star_count", object.drop_original_details_star_count_);
  jo("next_transfer_date", object.next_transfer_date_);
  jo("next_resale_date", object.next_resale_date_);
  jo("export_date", object.export_date_);
  jo("prepaid_upgrade_hash", object.prepaid_upgrade_hash_);
}

void to_json(JsonValueScope &jv, const td_api::ReportSponsoredResult &object) {
  td_api::downcast_call(const_cast<td_api::ReportSponsoredResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::reportSponsoredResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportSponsoredResultOk");
}

void to_json(JsonValueScope &jv, const td_api::reportSponsoredResultFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportSponsoredResultFailed");
}

void to_json(JsonValueScope &jv, const td_api::reportSponsoredResultOptionRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportSponsoredResultOptionRequired");
  jo("title", object.title_);
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::reportSponsoredResultAdsHidden &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportSponsoredResultAdsHidden");
}

void to_json(JsonValueScope &jv, const td_api::reportSponsoredResultPremiumRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportSponsoredResultPremiumRequired");
}

void to_json(JsonValueScope &jv, const td_api::savedMessagesTags &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedMessagesTags");
  jo("tags", ToJson(object.tags_));
}

void to_json(JsonValueScope &jv, const td_api::sentWebAppMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "sentWebAppMessage");
  jo("inline_message_id", object.inline_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::sponsoredChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "sponsoredChats");
  jo("chats", ToJson(object.chats_));
}

void to_json(JsonValueScope &jv, const td_api::starPaymentOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starPaymentOptions");
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::starTransactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactions");
  if (object.star_amount_) {
    jo("star_amount", ToJson(*object.star_amount_));
  }
  jo("transactions", ToJson(object.transactions_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::StickerType &object) {
  td_api::downcast_call(const_cast<td_api::StickerType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::stickerTypeRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerTypeRegular");
}

void to_json(JsonValueScope &jv, const td_api::stickerTypeMask &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerTypeMask");
}

void to_json(JsonValueScope &jv, const td_api::stickerTypeCustomEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerTypeCustomEmoji");
}

void to_json(JsonValueScope &jv, const td_api::storyAlbums &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAlbums");
  jo("albums", ToJson(object.albums_));
}

void to_json(JsonValueScope &jv, const td_api::storyInteractions &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteractions");
  jo("total_count", object.total_count_);
  jo("total_forward_count", object.total_forward_count_);
  jo("total_reaction_count", object.total_reaction_count_);
  jo("interactions", ToJson(object.interactions_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::SuggestedPostPrice &object) {
  td_api::downcast_call(const_cast<td_api::SuggestedPostPrice &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostPriceStar &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostPriceStar");
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostPriceTon &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostPriceTon");
  jo("toncoin_cent_count", object.toncoin_cent_count_);
}

void to_json(JsonValueScope &jv, const td_api::targetChatTypes &object) {
  auto jo = jv.enter_object();
  jo("@type", "targetChatTypes");
  jo("allow_user_chats", JsonBool{object.allow_user_chats_});
  jo("allow_bot_chats", JsonBool{object.allow_bot_chats_});
  jo("allow_group_chats", JsonBool{object.allow_group_chats_});
  jo("allow_channel_chats", JsonBool{object.allow_channel_chats_});
}

void to_json(JsonValueScope &jv, const td_api::testVectorStringObject &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorStringObject");
  jo("value", ToJson(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::timeZone &object) {
  auto jo = jv.enter_object();
  jo("@type", "timeZone");
  jo("id", object.id_);
  jo("name", object.name_);
  jo("utc_time_offset", object.utc_time_offset_);
}

void to_json(JsonValueScope &jv, const td_api::unreadReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "unreadReaction");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("is_big", JsonBool{object.is_big_});
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftModel &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftModel");
  jo("name", object.name_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  jo("rarity_per_mille", object.rarity_per_mille_);
}

void to_json(JsonValueScope &jv, const td_api::userLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "userLink");
  jo("url", object.url_);
  jo("expires_in", object.expires_in_);
}

void to_json(JsonValueScope &jv, const td_api::users &object) {
  auto jo = jv.enter_object();
  jo("@type", "users");
  jo("total_count", object.total_count_);
  jo("user_ids", ToJson(object.user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::videoMessageAdvertisement &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoMessageAdvertisement");
  jo("unique_id", object.unique_id_);
  jo("text", object.text_);
  jo("min_display_duration", object.min_display_duration_);
  jo("max_display_duration", object.max_display_duration_);
  jo("can_be_reported", JsonBool{object.can_be_reported_});
  if (object.sponsor_) {
    jo("sponsor", ToJson(*object.sponsor_));
  }
  jo("title", object.title_);
  jo("additional_info", object.additional_info_);
}

}  // namespace td_api
}  // namespace td
