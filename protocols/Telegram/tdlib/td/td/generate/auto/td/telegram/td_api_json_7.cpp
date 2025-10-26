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
Result<int32> tl_constructor_from_string(td_api::BlockList *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"blockListMain", 1352930172},
    {"blockListStories", 103323228}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ChatMemberStatus *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatMemberStatusCreator", -160019714},
    {"chatMemberStatusAdministrator", -70024163},
    {"chatMemberStatusMember", -32707562},
    {"chatMemberStatusRestricted", 1661432998},
    {"chatMemberStatusLeft", -5815259},
    {"chatMemberStatusBanned", -1653518666}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::FirebaseAuthenticationSettings *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"firebaseAuthenticationSettingsAndroid", -1771112932},
    {"firebaseAuthenticationSettingsIos", 222930116}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputChatTheme *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputChatThemeEmoji", -1461787199},
    {"inputChatThemeGift", 2026976301}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputPassportElement *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputPassportElementPersonalDetails", 164791359},
    {"inputPassportElementPassport", -497011356},
    {"inputPassportElementDriverLicense", 304813264},
    {"inputPassportElementIdentityCard", -9963390},
    {"inputPassportElementInternalPassport", 715360043},
    {"inputPassportElementAddress", 461630480},
    {"inputPassportElementUtilityBill", 1389203841},
    {"inputPassportElementBankStatement", -26585208},
    {"inputPassportElementRentalAgreement", 1736154155},
    {"inputPassportElementPassportRegistration", 1314562128},
    {"inputPassportElementTemporaryRegistration", -1913238047},
    {"inputPassportElementPhoneNumber", 1319357497},
    {"inputPassportElementEmailAddress", -248605659}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::MaskPoint *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"maskPointForehead", 1027512005},
    {"maskPointEyes", 1748310861},
    {"maskPointMouth", 411773406},
    {"maskPointChin", 534995335}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::OptionValue *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"optionValueBoolean", 63135518},
    {"optionValueEmpty", 918955155},
    {"optionValueInteger", -186858780},
    {"optionValueString", 756248212}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ProfileTab *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"profileTabPosts", -1181952362},
    {"profileTabGifts", 1296815210},
    {"profileTabMedia", 1925597525},
    {"profileTabFiles", -1422681088},
    {"profileTabLinks", -748329831},
    {"profileTabMusic", -1624780178},
    {"profileTabVoice", -461960914},
    {"profileTabGifs", -1564412267}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::SearchMessagesFilter *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"searchMessagesFilterEmpty", -869395657},
    {"searchMessagesFilterAnimation", -155713339},
    {"searchMessagesFilterAudio", 867505275},
    {"searchMessagesFilterDocument", 1526331215},
    {"searchMessagesFilterPhoto", 925932293},
    {"searchMessagesFilterVideo", 115538222},
    {"searchMessagesFilterVoiceNote", 1841439357},
    {"searchMessagesFilterPhotoAndVideo", 1352130963},
    {"searchMessagesFilterUrl", -1828724341},
    {"searchMessagesFilterChatPhoto", -1247751329},
    {"searchMessagesFilterVideoNote", 564323321},
    {"searchMessagesFilterVoiceAndVideoNote", 664174819},
    {"searchMessagesFilterMention", 2001258652},
    {"searchMessagesFilterUnreadMention", -95769149},
    {"searchMessagesFilterUnreadReaction", -1379651328},
    {"searchMessagesFilterFailedToSend", -596322564},
    {"searchMessagesFilterPinned", 371805512}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::SupergroupMembersFilter *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"supergroupMembersFilterRecent", 1178199509},
    {"supergroupMembersFilterContacts", -1282910856},
    {"supergroupMembersFilterAdministrators", -2097380265},
    {"supergroupMembersFilterSearch", -1696358469},
    {"supergroupMembersFilterRestricted", -1107800034},
    {"supergroupMembersFilterBanned", -1210621683},
    {"supergroupMembersFilterMention", 1151301973},
    {"supergroupMembersFilterBots", 492138918}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::UserPrivacySettingRule *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"userPrivacySettingRuleAllowAll", -1967186881},
    {"userPrivacySettingRuleAllowContacts", -1892733680},
    {"userPrivacySettingRuleAllowBots", 1404208925},
    {"userPrivacySettingRuleAllowPremiumUsers", 1624147265},
    {"userPrivacySettingRuleAllowUsers", 1110988334},
    {"userPrivacySettingRuleAllowChatMembers", -2048749863},
    {"userPrivacySettingRuleRestrictAll", -1406495408},
    {"userPrivacySettingRuleRestrictContacts", 1008389378},
    {"userPrivacySettingRuleRestrictBots", -1902547363},
    {"userPrivacySettingRuleRestrictUsers", 622796522},
    {"userPrivacySettingRuleRestrictChatMembers", 392530897}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::affiliateTypeCurrentUser &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::backgroundFillSolid &to, JsonObject &from) {
  TRY_STATUS(from_json(to.color_, from.extract_field("color")));
  return Status::OK();
}

Status from_json(td_api::blockListStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::botMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureQuickReplies &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessGreetingMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.recipients_, from.extract_field("recipients")));
  TRY_STATUS(from_json(to.inactivity_days_, from.extract_field("inactivity_days")));
  return Status::OK();
}

Status from_json(td_api::callProblemSilentLocal &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionTyping &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionChoosingContact &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatEventLogFilters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_edits_, from.extract_field("message_edits")));
  TRY_STATUS(from_json(to.message_deletions_, from.extract_field("message_deletions")));
  TRY_STATUS(from_json(to.message_pins_, from.extract_field("message_pins")));
  TRY_STATUS(from_json(to.member_joins_, from.extract_field("member_joins")));
  TRY_STATUS(from_json(to.member_leaves_, from.extract_field("member_leaves")));
  TRY_STATUS(from_json(to.member_invites_, from.extract_field("member_invites")));
  TRY_STATUS(from_json(to.member_promotions_, from.extract_field("member_promotions")));
  TRY_STATUS(from_json(to.member_restrictions_, from.extract_field("member_restrictions")));
  TRY_STATUS(from_json(to.info_changes_, from.extract_field("info_changes")));
  TRY_STATUS(from_json(to.setting_changes_, from.extract_field("setting_changes")));
  TRY_STATUS(from_json(to.invite_link_changes_, from.extract_field("invite_link_changes")));
  TRY_STATUS(from_json(to.video_chat_changes_, from.extract_field("video_chat_changes")));
  TRY_STATUS(from_json(to.forum_changes_, from.extract_field("forum_changes")));
  TRY_STATUS(from_json(to.subscription_extensions_, from.extract_field("subscription_extensions")));
  return Status::OK();
}

Status from_json(td_api::chatLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterMembers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatPhotoStickerTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenMicrosoftPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.channel_uri_, from.extract_field("channel_uri")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::emojiStatusTypeUpgradedGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.upgraded_gift_id_, from.extract_field("upgraded_gift_id")));
  TRY_STATUS(from_json(to.gift_title_, from.extract_field("gift_title")));
  TRY_STATUS(from_json(to.gift_name_, from.extract_field("gift_name")));
  TRY_STATUS(from_json(to.model_custom_emoji_id_, from.extract_field("model_custom_emoji_id")));
  TRY_STATUS(from_json(to.symbol_custom_emoji_id_, from.extract_field("symbol_custom_emoji_id")));
  TRY_STATUS(from_json(to.backdrop_colors_, from.extract_field("backdrop_colors")));
  return Status::OK();
}

Status from_json(td_api::fileTypeProfilePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeThumbnail &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::formattedText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.entities_, from.extract_field("entities")));
  return Status::OK();
}

Status from_json(td_api::groupCallDataChannelMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeLoginUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.forward_text_, from.extract_field("forward_text")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoStatic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsApplePay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_mime_type_, from.extract_field("thumbnail_mime_type")));
  TRY_STATUS(from_json(to.video_url_, from.extract_field("video_url")));
  TRY_STATUS(from_json(to.video_mime_type_, from.extract_field("video_mime_type")));
  TRY_STATUS(from_json(to.video_duration_, from.extract_field("video_duration")));
  TRY_STATUS(from_json(to.video_width_, from.extract_field("video_width")));
  TRY_STATUS(from_json(to.video_height_, from.extract_field("video_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.venue_, from.extract_field("venue")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, from.extract_field("thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, from.extract_field("thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputMessageDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.document_, from.extract_field("document")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.disable_content_type_detection_, from.extract_field("disable_content_type_detection")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  return Status::OK();
}

Status from_json(td_api::inputMessageContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contact_, from.extract_field("contact")));
  return Status::OK();
}

Status from_json(td_api::inputMessageReplyToExternalMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  TRY_STATUS(from_json(to.checklist_task_id_, from.extract_field("checklist_task_id")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementInternalPassport &to, JsonObject &from) {
  TRY_STATUS(from_json(to.internal_passport_, from.extract_field("internal_passport")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementError &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFiles &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hashes_, from.extract_field("file_hashes")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeActiveSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChangePhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMessageDraft &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.contains_link_, from.extract_field("contains_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, from.extract_field("server")));
  TRY_STATUS(from_json(to.port_, from.extract_field("port")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeThemeSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::jsonObjectMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.key_, from.extract_field("key")));
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::languagePackStringValueOrdinary &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::maskPointForehead &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSchedulingStateSendWhenVideoProcessed &to, JsonObject &from) {
  TRY_STATUS(from_json(to.send_date_, from.extract_field("send_date")));
  return Status::OK();
}

Status from_json(td_api::messageSourceDirectMessagesChatTopicHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageTopicForum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  return Status::OK();
}

Status from_json(td_api::networkTypeOther &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::orderInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  TRY_STATUS(from_json(to.shipping_address_, from.extract_field("shipping_address")));
  return Status::OK();
}

Status from_json(td_api::passportElementTypeAddress &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::phoneNumberAuthenticationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_flash_call_, from.extract_field("allow_flash_call")));
  TRY_STATUS(from_json(to.allow_missed_call_, from.extract_field("allow_missed_call")));
  TRY_STATUS(from_json(to.is_current_phone_number_, from.extract_field("is_current_phone_number")));
  TRY_STATUS(from_json(to.has_unknown_phone_number_, from.extract_field("has_unknown_phone_number")));
  TRY_STATUS(from_json(to.allow_sms_retriever_api_, from.extract_field("allow_sms_retriever_api")));
  TRY_STATUS(from_json(to.firebase_authentication_settings_, from.extract_field("firebase_authentication_settings")));
  TRY_STATUS(from_json(to.authentication_tokens_, from.extract_field("authentication_tokens")));
  return Status::OK();
}

Status from_json(td_api::premiumFeatureVoiceRecognition &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureForumTopicIcon &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureLastSeenTimes &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeChatFolderCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeWeeklyPostedStoryCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumSourceLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::profileTabPosts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::proxyTypeHttp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json(to.http_only_, from.extract_field("http_only")));
  return Status::OK();
}

Status from_json(td_api::reactionTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::reportReasonPornography &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendCodeReasonVerificationFailed &to, JsonObject &from) {
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterDocument &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterMention &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerFormatWebm &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposeStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.except_user_ids_, from.extract_field("except_user_ids")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionUpgradePremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedPostPriceStar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposeGiftedStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBotCommand &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeSpoiler &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMediaTimestamp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.media_timestamp_, from.extract_field("media_timestamp")));
  return Status::OK();
}

Status from_json(td_api::topChatCategoryWebAppBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowFindingByPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::webAppOpenModeFullSize &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::addChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::addLogMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.verbosity_level_, from.extract_field("verbosity_level")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::addQuickReplyShortcutMessageAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_name_, from.extract_field("shortcut_name")));
  TRY_STATUS(from_json(to.reply_to_message_id_, from.extract_field("reply_to_message_id")));
  TRY_STATUS(from_json(to.input_message_contents_, from.extract_field("input_message_contents")));
  return Status::OK();
}

Status from_json(td_api::answerCallbackQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.callback_query_id_, from.extract_field("callback_query_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.show_alert_, from.extract_field("show_alert")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.cache_time_, from.extract_field("cache_time")));
  return Status::OK();
}

Status from_json(td_api::banChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.member_id_, from.extract_field("member_id")));
  TRY_STATUS(from_json(to.banned_until_date_, from.extract_field("banned_until_date")));
  TRY_STATUS(from_json(to.revoke_messages_, from.extract_field("revoke_messages")));
  return Status::OK();
}

Status from_json(td_api::canSendMessageToUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkEmailAddressVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::cleanFileName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_name_, from.extract_field("file_name")));
  return Status::OK();
}

Status from_json(td_api::clickAnimatedEmojiMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::commitPendingPaidMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::createChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.expiration_date_, from.extract_field("expiration_date")));
  TRY_STATUS(from_json(to.member_limit_, from.extract_field("member_limit")));
  TRY_STATUS(from_json(to.creates_join_request_, from.extract_field("creates_join_request")));
  return Status::OK();
}

Status from_json(td_api::createNewSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.is_forum_, from.extract_field("is_forum")));
  TRY_STATUS(from_json(to.is_channel_, from.extract_field("is_channel")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, from.extract_field("message_auto_delete_time")));
  TRY_STATUS(from_json(to.for_import_, from.extract_field("for_import")));
  return Status::OK();
}

Status from_json(td_api::decryptGroupCallData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.data_channel_, from.extract_field("data_channel")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::deleteChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::deleteDefaultBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.for_dark_theme_, from.extract_field("for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::deleteProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.profile_photo_id_, from.extract_field("profile_photo_id")));
  return Status::OK();
}

Status from_json(td_api::deleteStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::downloadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.priority_, from.extract_field("priority")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.synchronous_, from.extract_field("synchronous")));
  return Status::OK();
}

Status from_json(td_api::editBusinessMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageLiveLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.live_period_, from.extract_field("live_period")));
  TRY_STATUS(from_json(to.heading_, from.extract_field("heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, from.extract_field("proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::editMessageSchedulingState &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.scheduling_state_, from.extract_field("scheduling_state")));
  return Status::OK();
}

Status from_json(td_api::encryptGroupCallData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.data_channel_, from.extract_field("data_channel")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  TRY_STATUS(from_json(to.unencrypted_prefix_size_, from.extract_field("unencrypted_prefix_size")));
  return Status::OK();
}

Status from_json(td_api::getAllStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.return_only_main_emoji_, from.extract_field("return_only_main_emoji")));
  return Status::OK();
}

Status from_json(td_api::getAutoDownloadSettingsPresets &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBotInfoDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getBusinessChatLinks &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getChatArchivedStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.from_story_id_, from.extract_field("from_story_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatEventLog &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.from_event_id_, from.extract_field("from_event_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filters_, from.extract_field("filters")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLinkCounts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMessagePosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getChatSimilarChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getCloseFriends &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCountryFlagEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  return Status::OK();
}

Status from_json(td_api::getDefaultChatEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDisallowedChatEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getFileExtension &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mime_type_, from.extract_field("mime_type")));
  return Status::OK();
}

Status from_json(td_api::getGiftCollections &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  return Status::OK();
}

Status from_json(td_api::getInactiveSupergroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getKeywordEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.input_language_codes_, from.extract_field("input_language_codes")));
  return Status::OK();
}

Status from_json(td_api::getLogVerbosityLevel &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getMessageAddedReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getMessageLocally &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getNetworkStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_current_, from.extract_field("only_current")));
  return Status::OK();
}

Status from_json(td_api::getPasswordState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPremiumGiveawayPaymentOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.boosted_chat_id_, from.extract_field("boosted_chat_id")));
  return Status::OK();
}

Status from_json(td_api::getPublicPostSearchLimits &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::getRecentlyVisitedTMeUrls &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::getSavedMessagesTopicMessageByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  return Status::OK();
}

Status from_json(td_api::getStarGiftPaymentOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getStickerOutline &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_file_id_, from.extract_field("sticker_file_id")));
  TRY_STATUS(from_json(to.for_animated_emoji_, from.extract_field("for_animated_emoji")));
  TRY_STATUS(from_json(to.for_clicked_animated_emoji_message_, from.extract_field("for_clicked_animated_emoji_message")));
  return Status::OK();
}

Status from_json(td_api::getStoryInteractions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.only_contacts_, from.extract_field("only_contacts")));
  TRY_STATUS(from_json(to.prefer_forwards_, from.extract_field("prefer_forwards")));
  TRY_STATUS(from_json(to.prefer_with_reaction_, from.extract_field("prefer_with_reaction")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getSupergroupFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::getTimeZones &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getUpgradedGiftWithdrawalUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatAvailableParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getWebPageInstantView &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::joinChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::loadDirectMessagesChatTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::openChatSimilarChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.opened_chat_id_, from.extract_field("opened_chat_id")));
  return Status::OK();
}

Status from_json(td_api::pingProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::readAllChatReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::recoverAuthenticationPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  TRY_STATUS(from_json(to.new_password_, from.extract_field("new_password")));
  TRY_STATUS(from_json(to.new_hint_, from.extract_field("new_hint")));
  return Status::OK();
}

Status from_json(td_api::removeFavoriteSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::removeProfileAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::removeStoryAlbumStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_album_id_, from.extract_field("story_album_id")));
  TRY_STATUS(from_json(to.story_ids_, from.extract_field("story_ids")));
  return Status::OK();
}

Status from_json(td_api::reorderQuickReplyShortcuts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_ids_, from.extract_field("shortcut_ids")));
  return Status::OK();
}

Status from_json(td_api::reportChatPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::requestAuthenticationPasswordRecovery &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetAllNotificationSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::savePreparedInlineMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.result_, from.extract_field("result")));
  TRY_STATUS(from_json(to.chat_types_, from.extract_field("chat_types")));
  return Status::OK();
}

Status from_json(td_api::searchChatsOnServer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchPublicChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::searchSavedMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::sellGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  return Status::OK();
}

Status from_json(td_api::sendChatAction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.action_, from.extract_field("action")));
  return Status::OK();
}

Status from_json(td_api::sendPaymentForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, from.extract_field("input_invoice")));
  TRY_STATUS(from_json(to.payment_form_id_, from.extract_field("payment_form_id")));
  TRY_STATUS(from_json(to.order_info_id_, from.extract_field("order_info_id")));
  TRY_STATUS(from_json(to.shipping_option_id_, from.extract_field("shipping_option_id")));
  TRY_STATUS(from_json(to.credentials_, from.extract_field("credentials")));
  TRY_STATUS(from_json(to.tip_amount_, from.extract_field("tip_amount")));
  return Status::OK();
}

Status from_json(td_api::setAccountTtl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.ttl_, from.extract_field("ttl")));
  return Status::OK();
}

Status from_json(td_api::setBio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bio_, from.extract_field("bio")));
  return Status::OK();
}

Status from_json(td_api::setBusinessAccountName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  return Status::OK();
}

Status from_json(td_api::setBusinessStartPage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.start_page_, from.extract_field("start_page")));
  return Status::OK();
}

Status from_json(td_api::setChatDiscussionGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.discussion_chat_id_, from.extract_field("discussion_chat_id")));
  return Status::OK();
}

Status from_json(td_api::setChatPermissions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.permissions_, from.extract_field("permissions")));
  return Status::OK();
}

Status from_json(td_api::setCustomEmojiStickerSetThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setDirectMessagesChatTopicIsMarkedAsUnread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.is_marked_as_unread_, from.extract_field("is_marked_as_unread")));
  return Status::OK();
}

Status from_json(td_api::setGroupCallParticipantVolumeLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.volume_level_, from.extract_field("volume_level")));
  return Status::OK();
}

Status from_json(td_api::setMessageFactCheck &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::setPassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.element_, from.extract_field("element")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::setProfileAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.profile_accent_color_id_, from.extract_field("profile_accent_color_id")));
  TRY_STATUS(from_json(to.profile_background_custom_emoji_id_, from.extract_field("profile_background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.emojis_, from.extract_field("emojis")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupCustomEmojiStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.custom_emoji_sticker_set_id_, from.extract_field("custom_emoji_sticker_set_id")));
  return Status::OK();
}

Status from_json(td_api::setUserPersonalProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::startGroupCallRecording &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.record_video_, from.extract_field("record_video")));
  TRY_STATUS(from_json(to.use_portrait_orientation_, from.extract_field("use_portrait_orientation")));
  return Status::OK();
}

Status from_json(td_api::terminateSession &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  return Status::OK();
}

Status from_json(td_api::testNetwork &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::toggleBusinessConnectedBotChatIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_paused_, from.extract_field("is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleDirectMessagesChatTopicCanSendUnpaidMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.can_send_unpaid_messages_, from.extract_field("can_send_unpaid_messages")));
  TRY_STATUS(from_json(to.refund_payments_, from.extract_field("refund_payments")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallParticipantIsHandRaised &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.is_hand_raised_, from.extract_field("is_hand_raised")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupHasAggressiveAntiSpamEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.has_aggressive_anti_spam_enabled_, from.extract_field("has_aggressive_anti_spam_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.is_active_, from.extract_field("is_active")));
  return Status::OK();
}

Status from_json(td_api::unpinAllChatMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::viewPremiumFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::addedReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "addedReaction");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("is_outgoing", JsonBool{object.is_outgoing_});
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::animatedChatPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "animatedChatPhoto");
  jo("length", object.length_);
  if (object.file_) {
    jo("file", ToJson(*object.file_));
  }
  jo("main_frame_timestamp", object.main_frame_timestamp_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeInfo");
  jo("phone_number", object.phone_number_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  if (object.next_type_) {
    jo("next_type", ToJson(*object.next_type_));
  }
  jo("timeout", object.timeout_);
}

void to_json(JsonValueScope &jv, const td_api::availableGifts &object) {
  auto jo = jv.enter_object();
  jo("@type", "availableGifts");
  jo("gifts", ToJson(object.gifts_));
}

void to_json(JsonValueScope &jv, const td_api::basicGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "basicGroup");
  jo("id", object.id_);
  jo("member_count", object.member_count_);
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
  jo("is_active", JsonBool{object.is_active_});
  jo("upgraded_to_supergroup_id", object.upgraded_to_supergroup_id_);
}

void to_json(JsonValueScope &jv, const td_api::botMediaPreviews &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMediaPreviews");
  jo("previews", ToJson(object.previews_));
}

void to_json(JsonValueScope &jv, const td_api::businessBotRights &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessBotRights");
  jo("can_reply", JsonBool{object.can_reply_});
  jo("can_read_messages", JsonBool{object.can_read_messages_});
  jo("can_delete_sent_messages", JsonBool{object.can_delete_sent_messages_});
  jo("can_delete_all_messages", JsonBool{object.can_delete_all_messages_});
  jo("can_edit_name", JsonBool{object.can_edit_name_});
  jo("can_edit_bio", JsonBool{object.can_edit_bio_});
  jo("can_edit_profile_photo", JsonBool{object.can_edit_profile_photo_});
  jo("can_edit_username", JsonBool{object.can_edit_username_});
  jo("can_view_gifts_and_stars", JsonBool{object.can_view_gifts_and_stars_});
  jo("can_sell_gifts", JsonBool{object.can_sell_gifts_});
  jo("can_change_gift_settings", JsonBool{object.can_change_gift_settings_});
  jo("can_transfer_and_upgrade_gifts", JsonBool{object.can_transfer_and_upgrade_gifts_});
  jo("can_transfer_stars", JsonBool{object.can_transfer_stars_});
  jo("can_manage_stories", JsonBool{object.can_manage_stories_});
}

void to_json(JsonValueScope &jv, const td_api::businessGreetingMessageSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessGreetingMessageSettings");
  jo("shortcut_id", object.shortcut_id_);
  if (object.recipients_) {
    jo("recipients", ToJson(*object.recipients_));
  }
  jo("inactivity_days", object.inactivity_days_);
}

void to_json(JsonValueScope &jv, const td_api::call &object) {
  auto jo = jv.enter_object();
  jo("@type", "call");
  jo("id", object.id_);
  jo("user_id", object.user_id_);
  jo("is_outgoing", JsonBool{object.is_outgoing_});
  jo("is_video", JsonBool{object.is_video_});
  if (object.state_) {
    jo("state", ToJson(*object.state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::CanPostStoryResult &object) {
  td_api::downcast_call(const_cast<td_api::CanPostStoryResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::canPostStoryResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "canPostStoryResultOk");
  jo("story_count", object.story_count_);
}

void to_json(JsonValueScope &jv, const td_api::canPostStoryResultPremiumNeeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canPostStoryResultPremiumNeeded");
}

void to_json(JsonValueScope &jv, const td_api::canPostStoryResultBoostNeeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canPostStoryResultBoostNeeded");
}

void to_json(JsonValueScope &jv, const td_api::canPostStoryResultActiveStoryLimitExceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canPostStoryResultActiveStoryLimitExceeded");
}

void to_json(JsonValueScope &jv, const td_api::canPostStoryResultWeeklyLimitExceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canPostStoryResultWeeklyLimitExceeded");
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::canPostStoryResultMonthlyLimitExceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canPostStoryResultMonthlyLimitExceeded");
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::chatAdministratorRights &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAdministratorRights");
  jo("can_manage_chat", JsonBool{object.can_manage_chat_});
  jo("can_change_info", JsonBool{object.can_change_info_});
  jo("can_post_messages", JsonBool{object.can_post_messages_});
  jo("can_edit_messages", JsonBool{object.can_edit_messages_});
  jo("can_delete_messages", JsonBool{object.can_delete_messages_});
  jo("can_invite_users", JsonBool{object.can_invite_users_});
  jo("can_restrict_members", JsonBool{object.can_restrict_members_});
  jo("can_pin_messages", JsonBool{object.can_pin_messages_});
  jo("can_manage_topics", JsonBool{object.can_manage_topics_});
  jo("can_promote_members", JsonBool{object.can_promote_members_});
  jo("can_manage_video_chats", JsonBool{object.can_manage_video_chats_});
  jo("can_post_stories", JsonBool{object.can_post_stories_});
  jo("can_edit_stories", JsonBool{object.can_edit_stories_});
  jo("can_delete_stories", JsonBool{object.can_delete_stories_});
  jo("can_manage_direct_messages", JsonBool{object.can_manage_direct_messages_});
  jo("is_anonymous", JsonBool{object.is_anonymous_});
}

void to_json(JsonValueScope &jv, const td_api::chatBoostSlot &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostSlot");
  jo("slot_id", object.slot_id_);
  jo("currently_boosted_chat_id", object.currently_boosted_chat_id_);
  jo("start_date", object.start_date_);
  jo("expiration_date", object.expiration_date_);
  jo("cooldown_until_date", object.cooldown_until_date_);
}

void to_json(JsonValueScope &jv, const td_api::chatFolderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderInfo");
  jo("id", object.id_);
  if (object.name_) {
    jo("name", ToJson(*object.name_));
  }
  if (object.icon_) {
    jo("icon", ToJson(*object.icon_));
  }
  jo("color_id", object.color_id_);
  jo("is_shareable", JsonBool{object.is_shareable_});
  jo("has_my_invite_links", JsonBool{object.has_my_invite_links_});
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkMember");
  jo("user_id", object.user_id_);
  jo("joined_chat_date", object.joined_chat_date_);
  jo("via_chat_folder_invite_link", JsonBool{object.via_chat_folder_invite_link_});
  jo("approver_user_id", object.approver_user_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatLocation");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("address", object.address_);
}

void to_json(JsonValueScope &jv, const td_api::chatPhotoInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPhotoInfo");
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

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactions");
  jo("ton_amount", object.ton_amount_);
  jo("transactions", ToJson(object.transactions_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::ChatType &object) {
  td_api::downcast_call(const_cast<td_api::ChatType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatTypePrivate &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTypePrivate");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatTypeBasicGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTypeBasicGroup");
  jo("basic_group_id", object.basic_group_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatTypeSupergroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTypeSupergroup");
  jo("supergroup_id", object.supergroup_id_);
  jo("is_channel", JsonBool{object.is_channel_});
}

void to_json(JsonValueScope &jv, const td_api::chatTypeSecret &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTypeSecret");
  jo("secret_chat_id", object.secret_chat_id_);
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::connectedAffiliateProgram &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectedAffiliateProgram");
  jo("url", object.url_);
  jo("bot_user_id", object.bot_user_id_);
  if (object.parameters_) {
    jo("parameters", ToJson(*object.parameters_));
  }
  jo("connection_date", object.connection_date_);
  jo("is_disconnected", JsonBool{object.is_disconnected_});
  jo("user_count", ToJson(JsonInt64{object.user_count_}));
  jo("revenue_star_count", ToJson(JsonInt64{object.revenue_star_count_}));
}

void to_json(JsonValueScope &jv, const td_api::createdBasicGroupChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "createdBasicGroupChat");
  jo("chat_id", object.chat_id_);
  if (object.failed_to_add_members_) {
    jo("failed_to_add_members", ToJson(*object.failed_to_add_members_));
  }
}

void to_json(JsonValueScope &jv, const td_api::DiceStickers &object) {
  td_api::downcast_call(const_cast<td_api::DiceStickers &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::diceStickersRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "diceStickersRegular");
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::diceStickersSlotMachine &object) {
  auto jo = jv.enter_object();
  jo("@type", "diceStickersSlotMachine");
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  if (object.lever_) {
    jo("lever", ToJson(*object.lever_));
  }
  if (object.left_reel_) {
    jo("left_reel", ToJson(*object.left_reel_));
  }
  if (object.center_reel_) {
    jo("center_reel", ToJson(*object.center_reel_));
  }
  if (object.right_reel_) {
    jo("right_reel", ToJson(*object.right_reel_));
  }
}

void to_json(JsonValueScope &jv, const td_api::EmojiCategorySource &object) {
  td_api::downcast_call(const_cast<td_api::EmojiCategorySource &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::emojiCategorySourceSearch &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiCategorySourceSearch");
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::emojiCategorySourcePremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiCategorySourcePremium");
}

void to_json(JsonValueScope &jv, const td_api::emojis &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojis");
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::fileDownloadedPrefixSize &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileDownloadedPrefixSize");
  jo("size", object.size_);
}

void to_json(JsonValueScope &jv, const td_api::foundAffiliateProgram &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundAffiliateProgram");
  jo("bot_user_id", object.bot_user_id_);
  if (object.info_) {
    jo("info", ToJson(*object.info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::foundStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundStories");
  jo("total_count", object.total_count_);
  jo("stories", ToJson(object.stories_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::giftCollection &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftCollection");
  jo("id", object.id_);
  jo("name", object.name_);
  if (object.icon_) {
    jo("icon", ToJson(*object.icon_));
  }
  jo("gift_count", object.gift_count_);
}

void to_json(JsonValueScope &jv, const td_api::giftUpgradePrice &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftUpgradePrice");
  jo("date", object.date_);
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::groupCallParticipant &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallParticipant");
  if (object.participant_id_) {
    jo("participant_id", ToJson(*object.participant_id_));
  }
  jo("audio_source_id", object.audio_source_id_);
  jo("screen_sharing_audio_source_id", object.screen_sharing_audio_source_id_);
  if (object.video_info_) {
    jo("video_info", ToJson(*object.video_info_));
  }
  if (object.screen_sharing_video_info_) {
    jo("screen_sharing_video_info", ToJson(*object.screen_sharing_video_info_));
  }
  jo("bio", object.bio_);
  jo("is_current_user", JsonBool{object.is_current_user_});
  jo("is_speaking", JsonBool{object.is_speaking_});
  jo("is_hand_raised", JsonBool{object.is_hand_raised_});
  jo("can_be_muted_for_all_users", JsonBool{object.can_be_muted_for_all_users_});
  jo("can_be_unmuted_for_all_users", JsonBool{object.can_be_unmuted_for_all_users_});
  jo("can_be_muted_for_current_user", JsonBool{object.can_be_muted_for_current_user_});
  jo("can_be_unmuted_for_current_user", JsonBool{object.can_be_unmuted_for_current_user_});
  jo("is_muted_for_all_users", JsonBool{object.is_muted_for_all_users_});
  jo("is_muted_for_current_user", JsonBool{object.is_muted_for_current_user_});
  jo("can_unmute_self", JsonBool{object.can_unmute_self_});
  jo("volume_level", object.volume_level_);
  jo("order", object.order_);
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButton");
  jo("text", object.text_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::InputMessageContent &object) {
  td_api::downcast_call(const_cast<td_api::InputMessageContent &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inputMessageText &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageText");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  if (object.link_preview_options_) {
    jo("link_preview_options", ToJson(*object.link_preview_options_));
  }
  jo("clear_draft", JsonBool{object.clear_draft_});
}

void to_json(JsonValueScope &jv, const td_api::inputMessageAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageAnimation");
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("added_sticker_file_ids", ToJson(object.added_sticker_file_ids_));
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
  jo("has_spoiler", JsonBool{object.has_spoiler_});
}

void to_json(JsonValueScope &jv, const td_api::inputMessageAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageAudio");
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
  if (object.album_cover_thumbnail_) {
    jo("album_cover_thumbnail", ToJson(*object.album_cover_thumbnail_));
  }
  jo("duration", object.duration_);
  jo("title", object.title_);
  jo("performer", object.performer_);
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessageDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageDocument");
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("disable_content_type_detection", JsonBool{object.disable_content_type_detection_});
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessagePaidMedia &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessagePaidMedia");
  jo("star_count", object.star_count_);
  jo("paid_media", ToJson(object.paid_media_));
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
  jo("payload", object.payload_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessagePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessagePhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("added_sticker_file_ids", ToJson(object.added_sticker_file_ids_));
  jo("width", object.width_);
  jo("height", object.height_);
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
  if (object.self_destruct_type_) {
    jo("self_destruct_type", ToJson(*object.self_destruct_type_));
  }
  jo("has_spoiler", JsonBool{object.has_spoiler_});
}

void to_json(JsonValueScope &jv, const td_api::inputMessageSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageSticker");
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("width", object.width_);
  jo("height", object.height_);
  jo("emoji", object.emoji_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessageVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.cover_) {
    jo("cover", ToJson(*object.cover_));
  }
  jo("start_timestamp", object.start_timestamp_);
  jo("added_sticker_file_ids", ToJson(object.added_sticker_file_ids_));
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
  jo("supports_streaming", JsonBool{object.supports_streaming_});
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
  if (object.self_destruct_type_) {
    jo("self_destruct_type", ToJson(*object.self_destruct_type_));
  }
  jo("has_spoiler", JsonBool{object.has_spoiler_});
}

void to_json(JsonValueScope &jv, const td_api::inputMessageVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageVideoNote");
  if (object.video_note_) {
    jo("video_note", ToJson(*object.video_note_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("duration", object.duration_);
  jo("length", object.length_);
  if (object.self_destruct_type_) {
    jo("self_destruct_type", ToJson(*object.self_destruct_type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessageVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageVoiceNote");
  if (object.voice_note_) {
    jo("voice_note", ToJson(*object.voice_note_));
  }
  jo("duration", object.duration_);
  jo("waveform", base64_encode(object.waveform_));
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  if (object.self_destruct_type_) {
    jo("self_destruct_type", ToJson(*object.self_destruct_type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessageLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageLocation");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("live_period", object.live_period_);
  jo("heading", object.heading_);
  jo("proximity_alert_radius", object.proximity_alert_radius_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessageVenue &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageVenue");
  if (object.venue_) {
    jo("venue", ToJson(*object.venue_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessageContact &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageContact");
  if (object.contact_) {
    jo("contact", ToJson(*object.contact_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessageDice &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageDice");
  jo("emoji", object.emoji_);
  jo("clear_draft", JsonBool{object.clear_draft_});
}

void to_json(JsonValueScope &jv, const td_api::inputMessageGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageGame");
  jo("bot_user_id", object.bot_user_id_);
  jo("game_short_name", object.game_short_name_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessageInvoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageInvoice");
  if (object.invoice_) {
    jo("invoice", ToJson(*object.invoice_));
  }
  jo("title", object.title_);
  jo("description", object.description_);
  jo("photo_url", object.photo_url_);
  jo("photo_size", object.photo_size_);
  jo("photo_width", object.photo_width_);
  jo("photo_height", object.photo_height_);
  jo("payload", base64_encode(object.payload_));
  jo("provider_token", object.provider_token_);
  jo("provider_data", object.provider_data_);
  jo("start_parameter", object.start_parameter_);
  if (object.paid_media_) {
    jo("paid_media", ToJson(*object.paid_media_));
  }
  if (object.paid_media_caption_) {
    jo("paid_media_caption", ToJson(*object.paid_media_caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessagePoll &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessagePoll");
  if (object.question_) {
    jo("question", ToJson(*object.question_));
  }
  jo("options", ToJson(object.options_));
  jo("is_anonymous", JsonBool{object.is_anonymous_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("open_period", object.open_period_);
  jo("close_date", object.close_date_);
  jo("is_closed", JsonBool{object.is_closed_});
}

void to_json(JsonValueScope &jv, const td_api::inputMessageStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageStory");
  jo("story_poster_chat_id", object.story_poster_chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessageChecklist &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageChecklist");
  if (object.checklist_) {
    jo("checklist", ToJson(*object.checklist_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessageForwarded &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageForwarded");
  jo("from_chat_id", object.from_chat_id_);
  jo("message_id", object.message_id_);
  jo("in_game_share", JsonBool{object.in_game_share_});
  jo("replace_video_start_timestamp", JsonBool{object.replace_video_start_timestamp_});
  jo("new_video_start_timestamp", object.new_video_start_timestamp_);
  if (object.copy_options_) {
    jo("copy_options", ToJson(*object.copy_options_));
  }
}

void to_json(JsonValueScope &jv, const td_api::InviteLinkChatType &object) {
  td_api::downcast_call(const_cast<td_api::InviteLinkChatType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inviteLinkChatTypeBasicGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "inviteLinkChatTypeBasicGroup");
}

void to_json(JsonValueScope &jv, const td_api::inviteLinkChatTypeSupergroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "inviteLinkChatTypeSupergroup");
}

void to_json(JsonValueScope &jv, const td_api::inviteLinkChatTypeChannel &object) {
  auto jo = jv.enter_object();
  jo("@type", "inviteLinkChatTypeChannel");
}

void to_json(JsonValueScope &jv, const td_api::LanguagePackStringValue &object) {
  td_api::downcast_call(const_cast<td_api::LanguagePackStringValue &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::languagePackStringValueOrdinary &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackStringValueOrdinary");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::languagePackStringValuePluralized &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackStringValuePluralized");
  jo("zero_value", object.zero_value_);
  jo("one_value", object.one_value_);
  jo("two_value", object.two_value_);
  jo("few_value", object.few_value_);
  jo("many_value", object.many_value_);
  jo("other_value", object.other_value_);
}

void to_json(JsonValueScope &jv, const td_api::languagePackStringValueDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackStringValueDeleted");
}

void to_json(JsonValueScope &jv, const td_api::locationAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "locationAddress");
  jo("country_code", object.country_code_);
  jo("state", object.state_);
  jo("city", object.city_);
  jo("street", object.street_);
}

void to_json(JsonValueScope &jv, const td_api::messageAutoDeleteTime &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageAutoDeleteTime");
  jo("time", object.time_);
}

void to_json(JsonValueScope &jv, const td_api::messageImportInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageImportInfo");
  jo("sender_name", object.sender_name_);
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::messageReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReactions");
  jo("reactions", ToJson(object.reactions_));
  jo("are_tags", JsonBool{object.are_tags_});
  jo("paid_reactors", ToJson(object.paid_reactors_));
  jo("can_get_added_reactions", JsonBool{object.can_get_added_reactions_});
}

void to_json(JsonValueScope &jv, const td_api::messageStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageStatistics");
  if (object.message_interaction_graph_) {
    jo("message_interaction_graph", ToJson(*object.message_interaction_graph_));
  }
  if (object.message_reaction_graph_) {
    jo("message_reaction_graph", ToJson(*object.message_reaction_graph_));
  }
}

void to_json(JsonValueScope &jv, const td_api::NetworkType &object) {
  td_api::downcast_call(const_cast<td_api::NetworkType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::networkTypeNone &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkTypeNone");
}

void to_json(JsonValueScope &jv, const td_api::networkTypeMobile &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkTypeMobile");
}

void to_json(JsonValueScope &jv, const td_api::networkTypeMobileRoaming &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkTypeMobileRoaming");
}

void to_json(JsonValueScope &jv, const td_api::networkTypeWiFi &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkTypeWiFi");
}

void to_json(JsonValueScope &jv, const td_api::networkTypeOther &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkTypeOther");
}

void to_json(JsonValueScope &jv, const td_api::ok &object) {
  auto jo = jv.enter_object();
  jo("@type", "ok");
}

void to_json(JsonValueScope &jv, const td_api::pageBlockTableCell &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockTableCell");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("is_header", JsonBool{object.is_header_});
  jo("colspan", object.colspan_);
  jo("rowspan", object.rowspan_);
  if (object.align_) {
    jo("align", ToJson(*object.align_));
  }
  if (object.valign_) {
    jo("valign", ToJson(*object.valign_));
  }
}

void to_json(JsonValueScope &jv, const td_api::PassportElementType &object) {
  td_api::downcast_call(const_cast<td_api::PassportElementType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypePersonalDetails &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypePersonalDetails");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypePassport &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypePassport");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeDriverLicense &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeDriverLicense");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeIdentityCard &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeIdentityCard");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeInternalPassport &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeInternalPassport");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeAddress");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeUtilityBill &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeUtilityBill");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeBankStatement &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeBankStatement");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeRentalAgreement &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeRentalAgreement");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypePassportRegistration &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypePassportRegistration");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeTemporaryRegistration &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeTemporaryRegistration");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypePhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypePhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::passportElementTypeEmailAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTypeEmailAddress");
}

void to_json(JsonValueScope &jv, const td_api::PaymentProvider &object) {
  td_api::downcast_call(const_cast<td_api::PaymentProvider &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::paymentProviderSmartGlocal &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentProviderSmartGlocal");
  jo("public_token", object.public_token_);
  jo("tokenize_url", object.tokenize_url_);
}

void to_json(JsonValueScope &jv, const td_api::paymentProviderStripe &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentProviderStripe");
  jo("publishable_key", object.publishable_key_);
  jo("need_country", JsonBool{object.need_country_});
  jo("need_postal_code", JsonBool{object.need_postal_code_});
  jo("need_cardholder_name", JsonBool{object.need_cardholder_name_});
}

void to_json(JsonValueScope &jv, const td_api::paymentProviderOther &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentProviderOther");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::point &object) {
  auto jo = jv.enter_object();
  jo("@type", "point");
  jo("x", object.x_);
  jo("y", object.y_);
}

void to_json(JsonValueScope &jv, const td_api::premiumGiftPaymentOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumGiftPaymentOptions");
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::preparedInlineMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "preparedInlineMessage");
  jo("inline_query_id", ToJson(JsonInt64{object.inline_query_id_}));
  if (object.result_) {
    jo("result", ToJson(*object.result_));
  }
  if (object.chat_types_) {
    jo("chat_types", ToJson(*object.chat_types_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ProxyType &object) {
  td_api::downcast_call(const_cast<td_api::ProxyType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::proxyTypeSocks5 &object) {
  auto jo = jv.enter_object();
  jo("@type", "proxyTypeSocks5");
  jo("username", object.username_);
  jo("password", object.password_);
}

void to_json(JsonValueScope &jv, const td_api::proxyTypeHttp &object) {
  auto jo = jv.enter_object();
  jo("@type", "proxyTypeHttp");
  jo("username", object.username_);
  jo("password", object.password_);
  jo("http_only", JsonBool{object.http_only_});
}

void to_json(JsonValueScope &jv, const td_api::proxyTypeMtproto &object) {
  auto jo = jv.enter_object();
  jo("@type", "proxyTypeMtproto");
  jo("secret", object.secret_);
}

void to_json(JsonValueScope &jv, const td_api::reactionNotificationSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionNotificationSettings");
  if (object.message_reaction_source_) {
    jo("message_reaction_source", ToJson(*object.message_reaction_source_));
  }
  if (object.story_reaction_source_) {
    jo("story_reaction_source", ToJson(*object.story_reaction_source_));
  }
  jo("sound_id", ToJson(JsonInt64{object.sound_id_}));
  jo("show_preview", JsonBool{object.show_preview_});
}

void to_json(JsonValueScope &jv, const td_api::recoveryEmailAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "recoveryEmailAddress");
  jo("recovery_email_address", object.recovery_email_address_);
}

void to_json(JsonValueScope &jv, const td_api::RevenueWithdrawalState &object) {
  td_api::downcast_call(const_cast<td_api::RevenueWithdrawalState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::revenueWithdrawalStatePending &object) {
  auto jo = jv.enter_object();
  jo("@type", "revenueWithdrawalStatePending");
}

void to_json(JsonValueScope &jv, const td_api::revenueWithdrawalStateSucceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "revenueWithdrawalStateSucceeded");
  jo("date", object.date_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::revenueWithdrawalStateFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "revenueWithdrawalStateFailed");
}

void to_json(JsonValueScope &jv, const td_api::scopeNotificationSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "scopeNotificationSettings");
  jo("mute_for", object.mute_for_);
  jo("sound_id", ToJson(JsonInt64{object.sound_id_}));
  jo("show_preview", JsonBool{object.show_preview_});
  jo("use_default_mute_stories", JsonBool{object.use_default_mute_stories_});
  jo("mute_stories", JsonBool{object.mute_stories_});
  jo("story_sound_id", ToJson(JsonInt64{object.story_sound_id_}));
  jo("show_story_poster", JsonBool{object.show_story_poster_});
  jo("disable_pinned_message_notifications", JsonBool{object.disable_pinned_message_notifications_});
  jo("disable_mention_notifications", JsonBool{object.disable_mention_notifications_});
}

void to_json(JsonValueScope &jv, const td_api::sharedChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "sharedChat");
  jo("chat_id", object.chat_id_);
  jo("title", object.title_);
  jo("username", object.username_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "starCount");
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::starSubscriptionPricing &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscriptionPricing");
  jo("period", object.period_);
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::StickerFormat &object) {
  td_api::downcast_call(const_cast<td_api::StickerFormat &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::stickerFormatWebp &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerFormatWebp");
}

void to_json(JsonValueScope &jv, const td_api::stickerFormatTgs &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerFormatTgs");
}

void to_json(JsonValueScope &jv, const td_api::stickerFormatWebm &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerFormatWebm");
}

void to_json(JsonValueScope &jv, const td_api::storageStatisticsByFileType &object) {
  auto jo = jv.enter_object();
  jo("@type", "storageStatisticsByFileType");
  if (object.file_type_) {
    jo("file_type", ToJson(*object.file_type_));
  }
  jo("size", object.size_);
  jo("count", object.count_);
}

void to_json(JsonValueScope &jv, const td_api::StoryContent &object) {
  td_api::downcast_call(const_cast<td_api::StoryContent &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::storyContentPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyContentPhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storyContentVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyContentVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.alternative_video_) {
    jo("alternative_video", ToJson(*object.alternative_video_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storyContentUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyContentUnsupported");
}

void to_json(JsonValueScope &jv, const td_api::storyRepostInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyRepostInfo");
  if (object.origin_) {
    jo("origin", ToJson(*object.origin_));
  }
  jo("is_content_modified", JsonBool{object.is_content_modified_});
}

void to_json(JsonValueScope &jv, const td_api::supergroupFullInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "supergroupFullInfo");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("description", object.description_);
  jo("member_count", object.member_count_);
  jo("administrator_count", object.administrator_count_);
  jo("restricted_count", object.restricted_count_);
  jo("banned_count", object.banned_count_);
  jo("linked_chat_id", object.linked_chat_id_);
  jo("direct_messages_chat_id", object.direct_messages_chat_id_);
  jo("slow_mode_delay", object.slow_mode_delay_);
  jo("slow_mode_delay_expires_in", object.slow_mode_delay_expires_in_);
  jo("can_enable_paid_messages", JsonBool{object.can_enable_paid_messages_});
  jo("can_enable_paid_reaction", JsonBool{object.can_enable_paid_reaction_});
  jo("can_get_members", JsonBool{object.can_get_members_});
  jo("has_hidden_members", JsonBool{object.has_hidden_members_});
  jo("can_hide_members", JsonBool{object.can_hide_members_});
  jo("can_set_sticker_set", JsonBool{object.can_set_sticker_set_});
  jo("can_set_location", JsonBool{object.can_set_location_});
  jo("can_get_statistics", JsonBool{object.can_get_statistics_});
  jo("can_get_revenue_statistics", JsonBool{object.can_get_revenue_statistics_});
  jo("can_get_star_revenue_statistics", JsonBool{object.can_get_star_revenue_statistics_});
  jo("can_send_gift", JsonBool{object.can_send_gift_});
  jo("can_toggle_aggressive_anti_spam", JsonBool{object.can_toggle_aggressive_anti_spam_});
  jo("is_all_history_available", JsonBool{object.is_all_history_available_});
  jo("can_have_sponsored_messages", JsonBool{object.can_have_sponsored_messages_});
  jo("has_aggressive_anti_spam_enabled", JsonBool{object.has_aggressive_anti_spam_enabled_});
  jo("has_paid_media_allowed", JsonBool{object.has_paid_media_allowed_});
  jo("has_pinned_stories", JsonBool{object.has_pinned_stories_});
  jo("gift_count", object.gift_count_);
  jo("my_boost_count", object.my_boost_count_);
  jo("unrestrict_boost_count", object.unrestrict_boost_count_);
  jo("outgoing_paid_message_star_count", object.outgoing_paid_message_star_count_);
  jo("sticker_set_id", ToJson(JsonInt64{object.sticker_set_id_}));
  jo("custom_emoji_sticker_set_id", ToJson(JsonInt64{object.custom_emoji_sticker_set_id_}));
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
  jo("bot_commands", ToJson(object.bot_commands_));
  if (object.bot_verification_) {
    jo("bot_verification", ToJson(*object.bot_verification_));
  }
  if (object.main_profile_tab_) {
    jo("main_profile_tab", ToJson(*object.main_profile_tab_));
  }
  jo("upgraded_from_basic_group_id", object.upgraded_from_basic_group_id_);
  jo("upgraded_from_max_message_id", object.upgraded_from_max_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::testInt &object) {
  auto jo = jv.enter_object();
  jo("@type", "testInt");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::TextEntityType &object) {
  td_api::downcast_call(const_cast<td_api::TextEntityType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeMention &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeMention");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeHashtag &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeHashtag");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeCashtag &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeCashtag");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeBotCommand &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeBotCommand");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeUrl");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeEmailAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeEmailAddress");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypePhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypePhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeBankCardNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeBankCardNumber");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeBold &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeBold");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeItalic &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeItalic");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeUnderline &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeUnderline");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeStrikethrough &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeStrikethrough");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeSpoiler &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeSpoiler");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeCode");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypePre &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypePre");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypePreCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypePreCode");
  jo("language", object.language_);
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeBlockQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeBlockQuote");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeExpandableBlockQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeExpandableBlockQuote");
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeTextUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeTextUrl");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeMentionName &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeMentionName");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeCustomEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeCustomEmoji");
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::textEntityTypeMediaTimestamp &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntityTypeMediaTimestamp");
  jo("media_timestamp", object.media_timestamp_);
}

void to_json(JsonValueScope &jv, const td_api::tonTransaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonTransaction");
  jo("id", object.id_);
  jo("ton_amount", object.ton_amount_);
  jo("is_refund", JsonBool{object.is_refund_});
  jo("date", object.date_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::upgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGift");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("regular_gift_id", ToJson(JsonInt64{object.regular_gift_id_}));
  jo("publisher_chat_id", object.publisher_chat_id_);
  jo("title", object.title_);
  jo("name", object.name_);
  jo("number", object.number_);
  jo("total_upgraded_count", object.total_upgraded_count_);
  jo("max_upgraded_count", object.max_upgraded_count_);
  jo("is_premium", JsonBool{object.is_premium_});
  jo("is_theme_available", JsonBool{object.is_theme_available_});
  jo("used_theme_chat_id", object.used_theme_chat_id_);
  if (object.host_id_) {
    jo("host_id", ToJson(*object.host_id_));
  }
  if (object.owner_id_) {
    jo("owner_id", ToJson(*object.owner_id_));
  }
  jo("owner_address", object.owner_address_);
  jo("owner_name", object.owner_name_);
  jo("gift_address", object.gift_address_);
  if (object.model_) {
    jo("model", ToJson(*object.model_));
  }
  if (object.symbol_) {
    jo("symbol", ToJson(*object.symbol_));
  }
  if (object.backdrop_) {
    jo("backdrop", ToJson(*object.backdrop_));
  }
  if (object.original_details_) {
    jo("original_details", ToJson(*object.original_details_));
  }
  if (object.colors_) {
    jo("colors", ToJson(*object.colors_));
  }
  if (object.resale_parameters_) {
    jo("resale_parameters", ToJson(*object.resale_parameters_));
  }
  jo("value_currency", object.value_currency_);
  jo("value_amount", object.value_amount_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftSymbol &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftSymbol");
  jo("name", object.name_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  jo("rarity_per_mille", object.rarity_per_mille_);
}

void to_json(JsonValueScope &jv, const td_api::userRating &object) {
  auto jo = jv.enter_object();
  jo("@type", "userRating");
  jo("level", object.level_);
  jo("is_maximum_level_reached", JsonBool{object.is_maximum_level_reached_});
  jo("rating", object.rating_);
  jo("current_level_rating", object.current_level_rating_);
  jo("next_level_rating", object.next_level_rating_);
}

void to_json(JsonValueScope &jv, const td_api::verificationStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "verificationStatus");
  jo("is_verified", JsonBool{object.is_verified_});
  jo("is_scam", JsonBool{object.is_scam_});
  jo("is_fake", JsonBool{object.is_fake_});
  jo("bot_verification_icon_custom_emoji_id", ToJson(JsonInt64{object.bot_verification_icon_custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::voiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "voiceNote");
  jo("duration", object.duration_);
  jo("waveform", base64_encode(object.waveform_));
  jo("mime_type", object.mime_type_);
  if (object.speech_recognition_result_) {
    jo("speech_recognition_result", ToJson(*object.speech_recognition_result_));
  }
  if (object.voice_) {
    jo("voice", ToJson(*object.voice_));
  }
}

}  // namespace td_api
}  // namespace td
