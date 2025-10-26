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
Result<int32> tl_constructor_from_string(td_api::BotCommandScope *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"botCommandScopeDefault", 795652779},
    {"botCommandScopeAllPrivateChats", -344889543},
    {"botCommandScopeAllGroupChats", -981088162},
    {"botCommandScopeAllChatAdministrators", 1998329169},
    {"botCommandScopeChat", -430234971},
    {"botCommandScopeChatAdministrators", 1119682126},
    {"botCommandScopeChatMember", -211380494}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ChatMembersFilter *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatMembersFilterContacts", 1774485671},
    {"chatMembersFilterAdministrators", -1266893796},
    {"chatMembersFilterMembers", 670504342},
    {"chatMembersFilterMention", 1932296772},
    {"chatMembersFilterRestricted", 1256282813},
    {"chatMembersFilterBanned", -1863102648},
    {"chatMembersFilterBots", -1422567288}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::GiftForResaleOrder *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"giftForResaleOrderPrice", 1371740258},
    {"giftForResaleOrderPriceChangeDate", -1694144054},
    {"giftForResaleOrderNumber", -1301157632}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputCredentials *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputCredentialsSaved", -2034385364},
    {"inputCredentialsNew", -829689558},
    {"inputCredentialsApplePay", -1246570799},
    {"inputCredentialsGooglePay", 844384100}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputPassportElementErrorSource *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputPassportElementErrorSourceUnspecified", 267230319},
    {"inputPassportElementErrorSourceDataField", -426795002},
    {"inputPassportElementErrorSourceFrontSide", 588023741},
    {"inputPassportElementErrorSourceReverseSide", 413072891},
    {"inputPassportElementErrorSourceSelfie", -773575528},
    {"inputPassportElementErrorSourceTranslationFile", 505842299},
    {"inputPassportElementErrorSourceTranslationFiles", -527254048},
    {"inputPassportElementErrorSourceFile", -298492469},
    {"inputPassportElementErrorSourceFiles", -2008541640}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::MessageSchedulingState *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"messageSchedulingStateSendAtDate", -1485570073},
    {"messageSchedulingStateSendWhenOnline", 2092947464},
    {"messageSchedulingStateSendWhenVideoProcessed", 2101578734}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PaidReactionType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"paidReactionTypeRegular", -1199187333},
    {"paidReactionTypeAnonymous", 47892621},
    {"paidReactionTypeChat", -675782044}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ProxyType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"proxyTypeSocks5", -890027341},
    {"proxyTypeHttp", -1547188361},
    {"proxyTypeMtproto", -1964826627}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::StickerFormat *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"stickerFormatWebp", -2123043040},
    {"stickerFormatTgs", 1614588662},
    {"stickerFormatWebm", -2070162097}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::TargetChat *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"targetChatCurrent", -416689904},
    {"targetChatChosen", -1392978522},
    {"targetChatInternalLink", -579301408}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::WebAppOpenMode *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"webAppOpenModeCompact", 1711603675},
    {"webAppOpenModeFullSize", 189320513},
    {"webAppOpenModeFullScreen", 1871315357}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::affiliateTypeBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillGradient &to, JsonObject &from) {
  TRY_STATUS(from_json(to.top_color_, from.extract_field("top_color")));
  TRY_STATUS(from_json(to.bottom_color_, from.extract_field("bottom_color")));
  TRY_STATUS(from_json(to.rotation_angle_, from.extract_field("rotation_angle")));
  return Status::OK();
}

Status from_json(td_api::botCommand &to, JsonObject &from) {
  TRY_STATUS(from_json(to.command_, from.extract_field("command")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageScheduleAlways &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureGreetingMessage &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::callProblemSilentRemote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionStartPlayingGame &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.icon_, from.extract_field("icon")));
  TRY_STATUS(from_json(to.color_id_, from.extract_field("color_id")));
  TRY_STATUS(from_json(to.is_shareable_, from.extract_field("is_shareable")));
  TRY_STATUS(from_json(to.pinned_chat_ids_, from.extract_field("pinned_chat_ids")));
  TRY_STATUS(from_json(to.included_chat_ids_, from.extract_field("included_chat_ids")));
  TRY_STATUS(from_json(to.excluded_chat_ids_, from.extract_field("excluded_chat_ids")));
  TRY_STATUS(from_json(to.exclude_muted_, from.extract_field("exclude_muted")));
  TRY_STATUS(from_json(to.exclude_read_, from.extract_field("exclude_read")));
  TRY_STATUS(from_json(to.exclude_archived_, from.extract_field("exclude_archived")));
  TRY_STATUS(from_json(to.include_contacts_, from.extract_field("include_contacts")));
  TRY_STATUS(from_json(to.include_non_contacts_, from.extract_field("include_non_contacts")));
  TRY_STATUS(from_json(to.include_bots_, from.extract_field("include_bots")));
  TRY_STATUS(from_json(to.include_groups_, from.extract_field("include_groups")));
  TRY_STATUS(from_json(to.include_channels_, from.extract_field("include_channels")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusCreator &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_title_, from.extract_field("custom_title")));
  TRY_STATUS(from_json(to.is_anonymous_, from.extract_field("is_anonymous")));
  TRY_STATUS(from_json(to.is_member_, from.extract_field("is_member")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterMention &to, JsonObject &from) {
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  return Status::OK();
}

Status from_json(td_api::collectibleItemTypeUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenMicrosoftPushVoIP &to, JsonObject &from) {
  TRY_STATUS(from_json(to.channel_uri_, from.extract_field("channel_uri")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationAppleId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::error &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  return Status::OK();
}

Status from_json(td_api::fileTypeSecret &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeUnknown &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::forumTopicIcon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.color_, from.extract_field("color")));
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::groupCallDataChannelScreenSharing &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButtonTypeStartBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameter_, from.extract_field("parameter")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, from.extract_field("animation")));
  TRY_STATUS(from_json(to.main_frame_timestamp_, from.extract_field("main_frame_timestamp")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsGooglePay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultArticle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, from.extract_field("thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, from.extract_field("thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.video_url_, from.extract_field("video_url")));
  TRY_STATUS(from_json(to.mime_type_, from.extract_field("mime_type")));
  TRY_STATUS(from_json(to.video_width_, from.extract_field("video_width")));
  TRY_STATUS(from_json(to.video_height_, from.extract_field("video_height")));
  TRY_STATUS(from_json(to.video_duration_, from.extract_field("video_duration")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputMessagePaidMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.paid_media_, from.extract_field("paid_media")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::inputMessageDice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  TRY_STATUS(from_json(to.clear_draft_, from.extract_field("clear_draft")));
  return Status::OK();
}

Status from_json(td_api::inputMessageReplyToStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceUnspecified &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.element_hash_, from.extract_field("element_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPersonalDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.files_, from.extract_field("files")));
  TRY_STATUS(from_json(to.translation_, from.extract_field("translation")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeWeather &to, JsonObject &from) {
  TRY_STATUS(from_json(to.temperature_, from.extract_field("temperature")));
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  TRY_STATUS(from_json(to.background_color_, from.extract_field("background_color")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeAttachmentMenuBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.target_chat_, from.extract_field("target_chat")));
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatAffiliateProgram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeGiftCollection &to, JsonObject &from) {
  TRY_STATUS(from_json(to.gift_owner_username_, from.extract_field("gift_owner_username")));
  TRY_STATUS(from_json(to.collection_id_, from.extract_field("collection_id")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMyStars &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePublicChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_username_, from.extract_field("chat_username")));
  TRY_STATUS(from_json(to.draft_text_, from.extract_field("draft_text")));
  TRY_STATUS(from_json(to.open_profile_, from.extract_field("open_profile")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUnknownDeepLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::jsonValueNull &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::languagePackStringValuePluralized &to, JsonObject &from) {
  TRY_STATUS(from_json(to.zero_value_, from.extract_field("zero_value")));
  TRY_STATUS(from_json(to.one_value_, from.extract_field("one_value")));
  TRY_STATUS(from_json(to.two_value_, from.extract_field("two_value")));
  TRY_STATUS(from_json(to.few_value_, from.extract_field("few_value")));
  TRY_STATUS(from_json(to.many_value_, from.extract_field("many_value")));
  TRY_STATUS(from_json(to.other_value_, from.extract_field("other_value")));
  return Status::OK();
}

Status from_json(td_api::maskPointEyes &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSelfDestructTypeTimer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.self_destruct_time_, from.extract_field("self_destruct_time")));
  return Status::OK();
}

Status from_json(td_api::messageSourceHistoryPreview &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageTopicDirectMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.direct_messages_chat_topic_id_, from.extract_field("direct_messages_chat_topic_id")));
  return Status::OK();
}

Status from_json(td_api::newChatPrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_new_chats_from_unknown_users_, from.extract_field("allow_new_chats_from_unknown_users")));
  TRY_STATUS(from_json(to.incoming_paid_message_star_count_, from.extract_field("incoming_paid_message_star_count")));
  return Status::OK();
}

Status from_json(td_api::paidReactionTypeRegular &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeUtilityBill &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::phoneNumberCodeTypeChange &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureDisabledAds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAppIcons &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureBusiness &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeChatFolderChosenChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeMonthlyPostedStoryCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumSourceSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::profileTabGifts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::proxyTypeMtproto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_, from.extract_field("secret")));
  return Status::OK();
}

Status from_json(td_api::reactionTypePaid &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonChildAbuse &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::scopeAutosaveSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.autosave_photos_, from.extract_field("autosave_photos")));
  TRY_STATUS(from_json(to.autosave_videos_, from.extract_field("autosave_videos")));
  TRY_STATUS(from_json(to.max_video_file_size_, from.extract_field("max_video_file_size")));
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterUnreadMention &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerTypeRegular &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposeGiftedStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsCloseFriends &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionRestorePremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedPostPriceTon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.toncoin_cent_count_, from.extract_field("toncoin_cent_count")));
  return Status::OK();
}

Status from_json(td_api::targetChatCurrent &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposeStarGiveaway &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.winner_count_, from.extract_field("winner_count")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeUrl &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textParseModeMarkdown &to, JsonObject &from) {
  TRY_STATUS(from_json(to.version_, from.extract_field("version")));
  return Status::OK();
}

Status from_json(td_api::topChatCategoryCalls &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowProfilePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::webAppOpenModeFullScreen &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::addChatToList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  return Status::OK();
}

Status from_json(td_api::addMessageReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.is_big_, from.extract_field("is_big")));
  TRY_STATUS(from_json(to.update_recent_reactions_, from.extract_field("update_recent_reactions")));
  return Status::OK();
}

Status from_json(td_api::addRecentSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::answerCustomQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_query_id_, from.extract_field("custom_query_id")));
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::banGroupCallParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::canTransferOwnership &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationEmailCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkLoginEmailAddressCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::clearAllDraftMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.exclude_secret_chats_, from.extract_field("exclude_secret_chats")));
  return Status::OK();
}

Status from_json(td_api::clickChatSponsoredMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_media_click_, from.extract_field("is_media_click")));
  TRY_STATUS(from_json(to.from_fullscreen_, from.extract_field("from_fullscreen")));
  return Status::OK();
}

Status from_json(td_api::confirmQrCodeAuthentication &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::createChatSubscriptionInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.subscription_pricing_, from.extract_field("subscription_pricing")));
  return Status::OK();
}

Status from_json(td_api::createPrivateChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::deleteAccount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::deleteChatBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.restore_previous_, from.extract_field("restore_previous")));
  return Status::OK();
}

Status from_json(td_api::deleteDirectMessagesChatTopicHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  return Status::OK();
}

Status from_json(td_api::deleteQuickReplyShortcut &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  return Status::OK();
}

Status from_json(td_api::deleteStoryAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_album_id_, from.extract_field("story_album_id")));
  return Status::OK();
}

Status from_json(td_api::dropGiftOriginalDetails &to, JsonObject &from) {
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::editBusinessStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  TRY_STATUS(from_json(to.areas_, from.extract_field("areas")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.privacy_settings_, from.extract_field("privacy_settings")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::endGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getAnimatedEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::getAutosaveSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBotInfoShortDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getBusinessConnectedBot &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getChatAvailableMessageSenders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLinkMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.only_with_expired_subscription_, from.extract_field("only_with_expired_subscription")));
  TRY_STATUS(from_json(to.offset_member_, from.extract_field("offset_member")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatNotificationSettingsExceptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.compare_sound_, from.extract_field("compare_sound")));
  return Status::OK();
}

Status from_json(td_api::getChatSparseMessagePositions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::getCollectibleItemInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getCreatedPublicChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getDefaultChatPhotoCustomEmojiStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getEmojiCategories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getFileMimeType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_name_, from.extract_field("file_name")));
  return Status::OK();
}

Status from_json(td_api::getGiftUpgradePreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.gift_id_, from.extract_field("gift_id")));
  return Status::OK();
}

Status from_json(td_api::getInlineGameHighScores &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::getLoginUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.button_id_, from.extract_field("button_id")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getMessageAuthor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageProperties &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getNewChatPrivacySettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPaymentForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, from.extract_field("input_invoice")));
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  return Status::OK();
}

Status from_json(td_api::getPremiumInfoSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.month_count_, from.extract_field("month_count")));
  return Status::OK();
}

Status from_json(td_api::getPushReceiverId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::getRecommendedChatFolders &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_sound_id_, from.extract_field("notification_sound_id")));
  return Status::OK();
}

Status from_json(td_api::getStarGiveawayPaymentOptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, from.extract_field("set_id")));
  return Status::OK();
}

Status from_json(td_api::getStoryNotificationSettingsExceptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSupergroupMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getTonRevenueStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.can_self_unmute_, from.extract_field("can_self_unmute")));
  return Status::OK();
}

Status from_json(td_api::giftPremiumWithStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.month_count_, from.extract_field("month_count")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::joinChatByInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::loadGroupCallParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::openMessageContent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::postStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  TRY_STATUS(from_json(to.areas_, from.extract_field("areas")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.privacy_settings_, from.extract_field("privacy_settings")));
  TRY_STATUS(from_json(to.album_ids_, from.extract_field("album_ids")));
  TRY_STATUS(from_json(to.active_period_, from.extract_field("active_period")));
  TRY_STATUS(from_json(to.from_story_full_id_, from.extract_field("from_story_full_id")));
  TRY_STATUS(from_json(to.is_posted_to_chat_page_, from.extract_field("is_posted_to_chat_page")));
  TRY_STATUS(from_json(to.protect_content_, from.extract_field("protect_content")));
  return Status::OK();
}

Status from_json(td_api::readAllDirectMessagesChatTopicReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  return Status::OK();
}

Status from_json(td_api::recoverPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  TRY_STATUS(from_json(to.new_password_, from.extract_field("new_password")));
  TRY_STATUS(from_json(to.new_hint_, from.extract_field("new_hint")));
  return Status::OK();
}

Status from_json(td_api::removeFileFromDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.delete_from_cache_, from.extract_field("delete_from_cache")));
  return Status::OK();
}

Status from_json(td_api::removeProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::removeTopChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.category_, from.extract_field("category")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::reorderStoryAlbumStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_album_id_, from.extract_field("story_album_id")));
  TRY_STATUS(from_json(to.story_ids_, from.extract_field("story_ids")));
  return Status::OK();
}

Status from_json(td_api::reportChatSponsoredMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json_bytes(to.option_id_, from.extract_field("option_id")));
  return Status::OK();
}

Status from_json(td_api::requestPasswordRecovery &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetAuthenticationEmailAddress &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchAffiliatePrograms &to, JsonObject &from) {
  TRY_STATUS(from_json(to.affiliate_, from.extract_field("affiliate")));
  TRY_STATUS(from_json(to.sort_order_, from.extract_field("sort_order")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchPublicChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::searchSecretMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  return Status::OK();
}

Status from_json(td_api::sendAuthenticationFirebaseSms &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::sendCustomRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.method_, from.extract_field("method")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::sendPhoneNumberCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::setAlarm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.seconds_, from.extract_field("seconds")));
  return Status::OK();
}

Status from_json(td_api::setBirthdate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.birthdate_, from.extract_field("birthdate")));
  return Status::OK();
}

Status from_json(td_api::setBusinessAccountProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  TRY_STATUS(from_json(to.is_public_, from.extract_field("is_public")));
  return Status::OK();
}

Status from_json(td_api::setChatAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.accent_color_id_, from.extract_field("accent_color_id")));
  TRY_STATUS(from_json(to.background_custom_emoji_id_, from.extract_field("background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setChatDraftMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.draft_message_, from.extract_field("draft_message")));
  return Status::OK();
}

Status from_json(td_api::setChatPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::setCustomLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.info_, from.extract_field("info")));
  TRY_STATUS(from_json(to.strings_, from.extract_field("strings")));
  return Status::OK();
}

Status from_json(td_api::setEmojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_status_, from.extract_field("emoji_status")));
  return Status::OK();
}

Status from_json(td_api::setInactiveSessionTtl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inactive_session_ttl_days_, from.extract_field("inactive_session_ttl_days")));
  return Status::OK();
}

Status from_json(td_api::setMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reaction_types_, from.extract_field("reaction_types")));
  TRY_STATUS(from_json(to.is_big_, from.extract_field("is_big")));
  return Status::OK();
}

Status from_json(td_api::setPassportElementErrors &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.errors_, from.extract_field("errors")));
  return Status::OK();
}

Status from_json(td_api::setProfileAudioPosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.after_file_id_, from.extract_field("after_file_id")));
  return Status::OK();
}

Status from_json(td_api::setStickerKeywords &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.keywords_, from.extract_field("keywords")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupMainProfileTab &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.main_profile_tab_, from.extract_field("main_profile_tab")));
  return Status::OK();
}

Status from_json(td_api::setUserPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.setting_, from.extract_field("setting")));
  TRY_STATUS(from_json(to.rules_, from.extract_field("rules")));
  return Status::OK();
}

Status from_json(td_api::startGroupCallScreenSharing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.audio_source_id_, from.extract_field("audio_source_id")));
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::testCallBytes &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, from.extract_field("server")));
  TRY_STATUS(from_json(to.port_, from.extract_field("port")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.dc_id_, from.extract_field("dc_id")));
  TRY_STATUS(from_json(to.timeout_, from.extract_field("timeout")));
  return Status::OK();
}

Status from_json(td_api::toggleChatDefaultDisableNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.default_disable_notification_, from.extract_field("default_disable_notification")));
  return Status::OK();
}

Status from_json(td_api::toggleDownloadIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.is_paused_, from.extract_field("is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallParticipantIsMuted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.is_muted_, from.extract_field("is_muted")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupHasAutomaticTranslation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.has_automatic_translation_, from.extract_field("has_automatic_translation")));
  return Status::OK();
}

Status from_json(td_api::toggleUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.is_active_, from.extract_field("is_active")));
  return Status::OK();
}

Status from_json(td_api::unpinAllDirectMessagesChatTopicMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  return Status::OK();
}

Status from_json(td_api::viewSponsoredChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sponsored_chat_unique_id_, from.extract_field("sponsored_chat_unique_id")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::addedReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "addedReactions");
  jo("total_count", object.total_count_);
  jo("reactions", ToJson(object.reactions_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::animatedEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "animatedEmoji");
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  jo("sticker_width", object.sticker_width_);
  jo("sticker_height", object.sticker_height_);
  jo("fitzpatrick_type", object.fitzpatrick_type_);
  if (object.sound_) {
    jo("sound", ToJson(*object.sound_));
  }
}

void to_json(JsonValueScope &jv, const td_api::AuthenticationCodeType &object) {
  td_api::downcast_call(const_cast<td_api::AuthenticationCodeType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeTelegramMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeTelegramMessage");
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeSms &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeSms");
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeSmsWord &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeSmsWord");
  jo("first_letter", object.first_letter_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeSmsPhrase &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeSmsPhrase");
  jo("first_word", object.first_word_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeCall");
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeFlashCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeFlashCall");
  jo("pattern", object.pattern_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeMissedCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeMissedCall");
  jo("phone_number_prefix", object.phone_number_prefix_);
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeFragment &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeFragment");
  jo("url", object.url_);
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeFirebaseAndroid &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeFirebaseAndroid");
  if (object.device_verification_parameters_) {
    jo("device_verification_parameters", ToJson(*object.device_verification_parameters_));
  }
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::authenticationCodeTypeFirebaseIos &object) {
  auto jo = jv.enter_object();
  jo("@type", "authenticationCodeTypeFirebaseIos");
  jo("receipt", object.receipt_);
  jo("push_timeout", object.push_timeout_);
  jo("length", object.length_);
}

void to_json(JsonValueScope &jv, const td_api::availableReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "availableReaction");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("needs_premium", JsonBool{object.needs_premium_});
}

void to_json(JsonValueScope &jv, const td_api::basicGroupFullInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "basicGroupFullInfo");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("description", object.description_);
  jo("creator_user_id", object.creator_user_id_);
  jo("members", ToJson(object.members_));
  jo("can_hide_members", JsonBool{object.can_hide_members_});
  jo("can_toggle_aggressive_anti_spam", JsonBool{object.can_toggle_aggressive_anti_spam_});
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
  jo("bot_commands", ToJson(object.bot_commands_));
}

void to_json(JsonValueScope &jv, const td_api::botMenuButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMenuButton");
  jo("text", object.text_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::businessChatLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessChatLink");
  jo("link", object.link_);
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("title", object.title_);
  jo("view_count", object.view_count_);
}

void to_json(JsonValueScope &jv, const td_api::businessInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessInfo");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  if (object.opening_hours_) {
    jo("opening_hours", ToJson(*object.opening_hours_));
  }
  if (object.local_opening_hours_) {
    jo("local_opening_hours", ToJson(*object.local_opening_hours_));
  }
  jo("next_open_in", object.next_open_in_);
  jo("next_close_in", object.next_close_in_);
  if (object.greeting_message_settings_) {
    jo("greeting_message_settings", ToJson(*object.greeting_message_settings_));
  }
  if (object.away_message_settings_) {
    jo("away_message_settings", ToJson(*object.away_message_settings_));
  }
  if (object.start_page_) {
    jo("start_page", ToJson(*object.start_page_));
  }
}

void to_json(JsonValueScope &jv, const td_api::CallDiscardReason &object) {
  td_api::downcast_call(const_cast<td_api::CallDiscardReason &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonEmpty &object) {
  auto jo = jv.enter_object();
  jo("@type", "callDiscardReasonEmpty");
}

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonMissed &object) {
  auto jo = jv.enter_object();
  jo("@type", "callDiscardReasonMissed");
}

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonDeclined &object) {
  auto jo = jv.enter_object();
  jo("@type", "callDiscardReasonDeclined");
}

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonDisconnected &object) {
  auto jo = jv.enter_object();
  jo("@type", "callDiscardReasonDisconnected");
}

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonHungUp &object) {
  auto jo = jv.enter_object();
  jo("@type", "callDiscardReasonHungUp");
}

void to_json(JsonValueScope &jv, const td_api::callDiscardReasonUpgradeToGroupCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "callDiscardReasonUpgradeToGroupCall");
  jo("invite_link", object.invite_link_);
}

void to_json(JsonValueScope &jv, const td_api::CanSendGiftResult &object) {
  td_api::downcast_call(const_cast<td_api::CanSendGiftResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::canSendGiftResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendGiftResultOk");
}

void to_json(JsonValueScope &jv, const td_api::canSendGiftResultFail &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendGiftResultFail");
  if (object.reason_) {
    jo("reason", ToJson(*object.reason_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatAdministrators &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAdministrators");
  jo("administrators", ToJson(object.administrators_));
}

void to_json(JsonValueScope &jv, const td_api::chatBoostSlots &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostSlots");
  jo("slots", ToJson(object.slots_));
}

void to_json(JsonValueScope &jv, const td_api::chatFolderInviteLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderInviteLink");
  jo("invite_link", object.invite_link_);
  jo("name", object.name_);
  jo("chat_ids", ToJson(object.chat_ids_));
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkMembers");
  jo("total_count", object.total_count_);
  jo("members", ToJson(object.members_));
}

void to_json(JsonValueScope &jv, const td_api::chatMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMember");
  if (object.member_id_) {
    jo("member_id", ToJson(*object.member_id_));
  }
  jo("inviter_user_id", object.inviter_user_id_);
  jo("joined_chat_date", object.joined_chat_date_);
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatPhotoSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPhotoSticker");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  if (object.background_fill_) {
    jo("background_fill", ToJson(*object.background_fill_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ChatSource &object) {
  td_api::downcast_call(const_cast<td_api::ChatSource &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatSourceMtprotoProxy &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatSourceMtprotoProxy");
}

void to_json(JsonValueScope &jv, const td_api::chatSourcePublicServiceAnnouncement &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatSourcePublicServiceAnnouncement");
  jo("type", object.type_);
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::chats &object) {
  auto jo = jv.enter_object();
  jo("@type", "chats");
  jo("total_count", object.total_count_);
  jo("chat_ids", ToJson(object.chat_ids_));
}

void to_json(JsonValueScope &jv, const td_api::connectedAffiliatePrograms &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectedAffiliatePrograms");
  jo("total_count", object.total_count_);
  jo("programs", ToJson(object.programs_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::currentWeather &object) {
  auto jo = jv.enter_object();
  jo("@type", "currentWeather");
  jo("temperature", object.temperature_);
  jo("emoji", object.emoji_);
}

void to_json(JsonValueScope &jv, const td_api::directMessagesChatTopic &object) {
  auto jo = jv.enter_object();
  jo("@type", "directMessagesChatTopic");
  jo("chat_id", object.chat_id_);
  jo("id", object.id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("order", ToJson(JsonInt64{object.order_}));
  jo("can_send_unpaid_messages", JsonBool{object.can_send_unpaid_messages_});
  jo("is_marked_as_unread", JsonBool{object.is_marked_as_unread_});
  jo("unread_count", object.unread_count_);
  jo("last_read_inbox_message_id", object.last_read_inbox_message_id_);
  jo("last_read_outbox_message_id", object.last_read_outbox_message_id_);
  jo("unread_reaction_count", object.unread_reaction_count_);
  if (object.last_message_) {
    jo("last_message", ToJson(*object.last_message_));
  }
  if (object.draft_message_) {
    jo("draft_message", ToJson(*object.draft_message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::emojiChatTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiChatTheme");
  jo("name", object.name_);
  if (object.light_settings_) {
    jo("light_settings", ToJson(*object.light_settings_));
  }
  if (object.dark_settings_) {
    jo("dark_settings", ToJson(*object.dark_settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::encryptedCredentials &object) {
  auto jo = jv.enter_object();
  jo("@type", "encryptedCredentials");
  jo("data", base64_encode(object.data_));
  jo("hash", base64_encode(object.hash_));
  jo("secret", base64_encode(object.secret_));
}

void to_json(JsonValueScope &jv, const td_api::FileType &object) {
  td_api::downcast_call(const_cast<td_api::FileType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::fileTypeNone &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeNone");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeAnimation");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeAudio");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeDocument");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeNotificationSound &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeNotificationSound");
}

void to_json(JsonValueScope &jv, const td_api::fileTypePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypePhoto");
}

void to_json(JsonValueScope &jv, const td_api::fileTypePhotoStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypePhotoStory");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeProfilePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeProfilePhoto");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSecret &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSecret");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSecretThumbnail &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSecretThumbnail");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSecure &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSecure");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSelfDestructingPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSelfDestructingPhoto");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSelfDestructingVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSelfDestructingVideo");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSelfDestructingVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSelfDestructingVideoNote");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSelfDestructingVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSelfDestructingVoiceNote");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeSticker");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeThumbnail &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeThumbnail");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeUnknown &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeUnknown");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeVideo");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeVideoNote");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeVideoStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeVideoStory");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeVoiceNote");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeWallpaper &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeWallpaper");
}

void to_json(JsonValueScope &jv, const td_api::foundAffiliatePrograms &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundAffiliatePrograms");
  jo("total_count", object.total_count_);
  jo("programs", ToJson(object.programs_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::foundUsers &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundUsers");
  jo("user_ids", ToJson(object.user_ids_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::giftCollections &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftCollections");
  jo("collections", ToJson(object.collections_));
}

void to_json(JsonValueScope &jv, const td_api::giftsForResale &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftsForResale");
  jo("total_count", object.total_count_);
  jo("gifts", ToJson(object.gifts_));
  jo("models", ToJson(object.models_));
  jo("symbols", ToJson(object.symbols_));
  jo("backdrops", ToJson(object.backdrops_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::groupCallParticipantVideoInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallParticipantVideoInfo");
  jo("source_groups", ToJson(object.source_groups_));
  jo("endpoint_id", object.endpoint_id_);
  jo("is_paused", JsonBool{object.is_paused_});
}

void to_json(JsonValueScope &jv, const td_api::InlineKeyboardButtonType &object) {
  td_api::downcast_call(const_cast<td_api::InlineKeyboardButtonType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeUrl");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeLoginUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeLoginUrl");
  jo("url", object.url_);
  jo("id", object.id_);
  jo("forward_text", object.forward_text_);
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeWebApp");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeCallback &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeCallback");
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeCallbackWithPassword &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeCallbackWithPassword");
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeCallbackGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeCallbackGame");
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeSwitchInline &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeSwitchInline");
  jo("query", object.query_);
  if (object.target_chat_) {
    jo("target_chat", ToJson(*object.target_chat_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeBuy &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeBuy");
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeUser");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButtonTypeCopyText &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButtonTypeCopyText");
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::InputMessageReplyTo &object) {
  td_api::downcast_call(const_cast<td_api::InputMessageReplyTo &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inputMessageReplyToMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageReplyToMessage");
  jo("message_id", object.message_id_);
  if (object.quote_) {
    jo("quote", ToJson(*object.quote_));
  }
  jo("checklist_task_id", object.checklist_task_id_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessageReplyToExternalMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageReplyToExternalMessage");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.quote_) {
    jo("quote", ToJson(*object.quote_));
  }
  jo("checklist_task_id", object.checklist_task_id_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessageReplyToStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageReplyToStory");
  jo("story_poster_chat_id", object.story_poster_chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::invoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "invoice");
  jo("currency", object.currency_);
  jo("price_parts", ToJson(object.price_parts_));
  jo("subscription_period", object.subscription_period_);
  jo("max_tip_amount", object.max_tip_amount_);
  jo("suggested_tip_amounts", ToJson(object.suggested_tip_amounts_));
  jo("recurring_payment_terms_of_service_url", object.recurring_payment_terms_of_service_url_);
  jo("terms_of_service_url", object.terms_of_service_url_);
  jo("is_test", JsonBool{object.is_test_});
  jo("need_name", JsonBool{object.need_name_});
  jo("need_phone_number", JsonBool{object.need_phone_number_});
  jo("need_email_address", JsonBool{object.need_email_address_});
  jo("need_shipping_address", JsonBool{object.need_shipping_address_});
  jo("send_phone_number_to_provider", JsonBool{object.send_phone_number_to_provider_});
  jo("send_email_address_to_provider", JsonBool{object.send_email_address_to_provider_});
  jo("is_flexible", JsonBool{object.is_flexible_});
}

void to_json(JsonValueScope &jv, const td_api::languagePackStrings &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackStrings");
  jo("strings", ToJson(object.strings_));
}

void to_json(JsonValueScope &jv, const td_api::LogStream &object) {
  td_api::downcast_call(const_cast<td_api::LogStream &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::logStreamDefault &object) {
  auto jo = jv.enter_object();
  jo("@type", "logStreamDefault");
}

void to_json(JsonValueScope &jv, const td_api::logStreamFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "logStreamFile");
  jo("path", object.path_);
  jo("max_file_size", object.max_file_size_);
  jo("redirect_stderr", JsonBool{object.redirect_stderr_});
}

void to_json(JsonValueScope &jv, const td_api::logStreamEmpty &object) {
  auto jo = jv.enter_object();
  jo("@type", "logStreamEmpty");
}

void to_json(JsonValueScope &jv, const td_api::messageCalendar &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageCalendar");
  jo("total_count", object.total_count_);
  jo("days", ToJson(object.days_));
}

void to_json(JsonValueScope &jv, const td_api::messageInteractionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageInteractionInfo");
  jo("view_count", object.view_count_);
  jo("forward_count", object.forward_count_);
  if (object.reply_info_) {
    jo("reply_info", ToJson(*object.reply_info_));
  }
  if (object.reactions_) {
    jo("reactions", ToJson(*object.reactions_));
  }
}

void to_json(JsonValueScope &jv, const td_api::MessageReadDate &object) {
  td_api::downcast_call(const_cast<td_api::MessageReadDate &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageReadDateRead &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReadDateRead");
  jo("read_date", object.read_date_);
}

void to_json(JsonValueScope &jv, const td_api::messageReadDateUnread &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReadDateUnread");
}

void to_json(JsonValueScope &jv, const td_api::messageReadDateTooOld &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReadDateTooOld");
}

void to_json(JsonValueScope &jv, const td_api::messageReadDateUserPrivacyRestricted &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReadDateUserPrivacyRestricted");
}

void to_json(JsonValueScope &jv, const td_api::messageReadDateMyPrivacyRestricted &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReadDateMyPrivacyRestricted");
}

void to_json(JsonValueScope &jv, const td_api::messageThreadInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageThreadInfo");
  jo("chat_id", object.chat_id_);
  jo("message_thread_id", object.message_thread_id_);
  if (object.reply_info_) {
    jo("reply_info", ToJson(*object.reply_info_));
  }
  jo("unread_message_count", object.unread_message_count_);
  jo("messages", ToJson(object.messages_));
  if (object.draft_message_) {
    jo("draft_message", ToJson(*object.draft_message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::newChatPrivacySettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "newChatPrivacySettings");
  jo("allow_new_chats_from_unknown_users", JsonBool{object.allow_new_chats_from_unknown_users_});
  jo("incoming_paid_message_star_count", object.incoming_paid_message_star_count_);
}

void to_json(JsonValueScope &jv, const td_api::OptionValue &object) {
  td_api::downcast_call(const_cast<td_api::OptionValue &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::optionValueBoolean &object) {
  auto jo = jv.enter_object();
  jo("@type", "optionValueBoolean");
  jo("value", JsonBool{object.value_});
}

void to_json(JsonValueScope &jv, const td_api::optionValueEmpty &object) {
  auto jo = jv.enter_object();
  jo("@type", "optionValueEmpty");
}

void to_json(JsonValueScope &jv, const td_api::optionValueInteger &object) {
  auto jo = jv.enter_object();
  jo("@type", "optionValueInteger");
  jo("value", ToJson(JsonInt64{object.value_}));
}

void to_json(JsonValueScope &jv, const td_api::optionValueString &object) {
  auto jo = jv.enter_object();
  jo("@type", "optionValueString");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::PageBlockVerticalAlignment &object) {
  td_api::downcast_call(const_cast<td_api::PageBlockVerticalAlignment &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::pageBlockVerticalAlignmentTop &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockVerticalAlignmentTop");
}

void to_json(JsonValueScope &jv, const td_api::pageBlockVerticalAlignmentMiddle &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockVerticalAlignmentMiddle");
}

void to_json(JsonValueScope &jv, const td_api::pageBlockVerticalAlignmentBottom &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockVerticalAlignmentBottom");
}

void to_json(JsonValueScope &jv, const td_api::passportElements &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElements");
  jo("elements", ToJson(object.elements_));
}

void to_json(JsonValueScope &jv, const td_api::paymentReceipt &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentReceipt");
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
  jo("date", object.date_);
  jo("seller_bot_user_id", object.seller_bot_user_id_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::poll &object) {
  auto jo = jv.enter_object();
  jo("@type", "poll");
  jo("id", ToJson(JsonInt64{object.id_}));
  if (object.question_) {
    jo("question", ToJson(*object.question_));
  }
  jo("options", ToJson(object.options_));
  jo("total_voter_count", object.total_voter_count_);
  jo("recent_voter_ids", ToJson(object.recent_voter_ids_));
  jo("is_anonymous", JsonBool{object.is_anonymous_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("open_period", object.open_period_);
  jo("close_date", object.close_date_);
  jo("is_closed", JsonBool{object.is_closed_});
}

void to_json(JsonValueScope &jv, const td_api::premiumGiveawayPaymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumGiveawayPaymentOption");
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("winner_count", object.winner_count_);
  jo("month_count", object.month_count_);
  jo("store_product_id", object.store_product_id_);
  jo("store_product_quantity", object.store_product_quantity_);
}

void to_json(JsonValueScope &jv, const td_api::preparedInlineMessageId &object) {
  auto jo = jv.enter_object();
  jo("@type", "preparedInlineMessageId");
  jo("id", object.id_);
  jo("expiration_date", object.expiration_date_);
}

void to_json(JsonValueScope &jv, const td_api::PublicForward &object) {
  td_api::downcast_call(const_cast<td_api::PublicForward &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::publicForwardMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "publicForwardMessage");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::publicForwardStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "publicForwardStory");
  if (object.story_) {
    jo("story", ToJson(*object.story_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ReactionNotificationSource &object) {
  td_api::downcast_call(const_cast<td_api::ReactionNotificationSource &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::reactionNotificationSourceNone &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionNotificationSourceNone");
}

void to_json(JsonValueScope &jv, const td_api::reactionNotificationSourceContacts &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionNotificationSourceContacts");
}

void to_json(JsonValueScope &jv, const td_api::reactionNotificationSourceAll &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionNotificationSourceAll");
}

void to_json(JsonValueScope &jv, const td_api::remoteFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "remoteFile");
  jo("id", object.id_);
  jo("unique_id", object.unique_id_);
  jo("is_uploading_active", JsonBool{object.is_uploading_active_});
  jo("is_uploading_completed", JsonBool{object.is_uploading_completed_});
  jo("uploaded_size", object.uploaded_size_);
}

void to_json(JsonValueScope &jv, const td_api::RichText &object) {
  td_api::downcast_call(const_cast<td_api::RichText &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::richTextPlain &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextPlain");
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::richTextBold &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextBold");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextItalic &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextItalic");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextUnderline &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextUnderline");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextStrikethrough &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextStrikethrough");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextFixed &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextFixed");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextUrl");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("url", object.url_);
  jo("is_cached", JsonBool{object.is_cached_});
}

void to_json(JsonValueScope &jv, const td_api::richTextEmailAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextEmailAddress");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("email_address", object.email_address_);
}

void to_json(JsonValueScope &jv, const td_api::richTextSubscript &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextSubscript");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextSuperscript &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextSuperscript");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextMarked &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextMarked");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::richTextPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextPhoneNumber");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("phone_number", object.phone_number_);
}

void to_json(JsonValueScope &jv, const td_api::richTextIcon &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextIcon");
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  jo("width", object.width_);
  jo("height", object.height_);
}

void to_json(JsonValueScope &jv, const td_api::richTextReference &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextReference");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("anchor_name", object.anchor_name_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::richTextAnchor &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextAnchor");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::richTextAnchorLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTextAnchorLink");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("anchor_name", object.anchor_name_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::richTexts &object) {
  auto jo = jv.enter_object();
  jo("@type", "richTexts");
  jo("texts", ToJson(object.texts_));
}

void to_json(JsonValueScope &jv, const td_api::seconds &object) {
  auto jo = jv.enter_object();
  jo("@type", "seconds");
  jo("seconds", object.seconds_);
}

void to_json(JsonValueScope &jv, const td_api::sharedUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "sharedUser");
  jo("user_id", object.user_id_);
  jo("first_name", object.first_name_);
  jo("last_name", object.last_name_);
  jo("username", object.username_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starGiveawayPaymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "starGiveawayPaymentOption");
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("star_count", object.star_count_);
  jo("store_product_id", object.store_product_id_);
  jo("yearly_boost_count", object.yearly_boost_count_);
  jo("winner_options", ToJson(object.winner_options_));
  jo("is_default", JsonBool{object.is_default_});
  jo("is_additional", JsonBool{object.is_additional_});
}

void to_json(JsonValueScope &jv, const td_api::StarSubscriptionType &object) {
  td_api::downcast_call(const_cast<td_api::StarSubscriptionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::starSubscriptionTypeChannel &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscriptionTypeChannel");
  jo("can_reuse", JsonBool{object.can_reuse_});
  jo("invite_link", object.invite_link_);
}

void to_json(JsonValueScope &jv, const td_api::starSubscriptionTypeBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscriptionTypeBot");
  jo("is_canceled_by_bot", JsonBool{object.is_canceled_by_bot_});
  jo("title", object.title_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("invoice_link", object.invoice_link_);
}

void to_json(JsonValueScope &jv, const td_api::StickerFullType &object) {
  td_api::downcast_call(const_cast<td_api::StickerFullType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::stickerFullTypeRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerFullTypeRegular");
  if (object.premium_animation_) {
    jo("premium_animation", ToJson(*object.premium_animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::stickerFullTypeMask &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerFullTypeMask");
  if (object.mask_position_) {
    jo("mask_position", ToJson(*object.mask_position_));
  }
}

void to_json(JsonValueScope &jv, const td_api::stickerFullTypeCustomEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerFullTypeCustomEmoji");
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
  jo("needs_repainting", JsonBool{object.needs_repainting_});
}

void to_json(JsonValueScope &jv, const td_api::storageStatisticsFast &object) {
  auto jo = jv.enter_object();
  jo("@type", "storageStatisticsFast");
  jo("files_size", object.files_size_);
  jo("file_count", object.file_count_);
  jo("database_size", object.database_size_);
  jo("language_pack_database_size", object.language_pack_database_size_);
  jo("log_size", object.log_size_);
}

void to_json(JsonValueScope &jv, const td_api::storyInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInfo");
  jo("story_id", object.story_id_);
  jo("date", object.date_);
  jo("is_for_close_friends", JsonBool{object.is_for_close_friends_});
}

void to_json(JsonValueScope &jv, const td_api::storyStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyStatistics");
  if (object.story_interaction_graph_) {
    jo("story_interaction_graph", ToJson(*object.story_interaction_graph_));
  }
  if (object.story_reaction_graph_) {
    jo("story_reaction_graph", ToJson(*object.story_reaction_graph_));
  }
}

void to_json(JsonValueScope &jv, const td_api::tMeUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrl");
  jo("url", object.url_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::testString &object) {
  auto jo = jv.enter_object();
  jo("@type", "testString");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::textQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "textQuote");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("position", object.position_);
  jo("is_manual", JsonBool{object.is_manual_});
}

void to_json(JsonValueScope &jv, const td_api::TonTransactionType &object) {
  td_api::downcast_call(const_cast<td_api::TonTransactionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::tonTransactionTypeFragmentDeposit &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonTransactionTypeFragmentDeposit");
  jo("is_gift", JsonBool{object.is_gift_});
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::tonTransactionTypeSuggestedPostPayment &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonTransactionTypeSuggestedPostPayment");
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::tonTransactionTypeUpgradedGiftPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonTransactionTypeUpgradedGiftPurchase");
  jo("user_id", object.user_id_);
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::tonTransactionTypeUpgradedGiftSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonTransactionTypeUpgradedGiftSale");
  jo("user_id", object.user_id_);
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  jo("commission_per_mille", object.commission_per_mille_);
  jo("commission_toncoin_amount", object.commission_toncoin_amount_);
}

void to_json(JsonValueScope &jv, const td_api::tonTransactionTypeUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "tonTransactionTypeUnsupported");
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftBackdrop &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftBackdrop");
  jo("id", object.id_);
  jo("name", object.name_);
  if (object.colors_) {
    jo("colors", ToJson(*object.colors_));
  }
  jo("rarity_per_mille", object.rarity_per_mille_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftSymbolCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftSymbolCount");
  if (object.symbol_) {
    jo("symbol", ToJson(*object.symbol_));
  }
  jo("total_count", object.total_count_);
}

void to_json(JsonValueScope &jv, const td_api::UserStatus &object) {
  td_api::downcast_call(const_cast<td_api::UserStatus &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::userStatusEmpty &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusEmpty");
}

void to_json(JsonValueScope &jv, const td_api::userStatusOnline &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusOnline");
  jo("expires", object.expires_);
}

void to_json(JsonValueScope &jv, const td_api::userStatusOffline &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusOffline");
  jo("was_online", object.was_online_);
}

void to_json(JsonValueScope &jv, const td_api::userStatusRecently &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusRecently");
  jo("by_my_privacy_settings", JsonBool{object.by_my_privacy_settings_});
}

void to_json(JsonValueScope &jv, const td_api::userStatusLastWeek &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusLastWeek");
  jo("by_my_privacy_settings", JsonBool{object.by_my_privacy_settings_});
}

void to_json(JsonValueScope &jv, const td_api::userStatusLastMonth &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusLastMonth");
  jo("by_my_privacy_settings", JsonBool{object.by_my_privacy_settings_});
}

void to_json(JsonValueScope &jv, const td_api::video &object) {
  auto jo = jv.enter_object();
  jo("@type", "video");
  jo("duration", object.duration_);
  jo("width", object.width_);
  jo("height", object.height_);
  jo("file_name", object.file_name_);
  jo("mime_type", object.mime_type_);
  jo("has_stickers", JsonBool{object.has_stickers_});
  jo("supports_streaming", JsonBool{object.supports_streaming_});
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
}

void to_json(JsonValueScope &jv, const td_api::webApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "webApp");
  jo("short_name", object.short_name_);
  jo("title", object.title_);
  jo("description", object.description_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
}

}  // namespace td_api
}  // namespace td
