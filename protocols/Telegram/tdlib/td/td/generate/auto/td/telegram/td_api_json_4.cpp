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
Result<int32> tl_constructor_from_string(td_api::AutosaveSettingsScope *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"autosaveSettingsScopePrivateChats", 1395227007},
    {"autosaveSettingsScopeGroupChats", 853544526},
    {"autosaveSettingsScopeChannelChats", -499572783},
    {"autosaveSettingsScopeChat", -1632255255}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ChatAction *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatActionTyping", 380122167},
    {"chatActionRecordingVideo", 216553362},
    {"chatActionUploadingVideo", 1234185270},
    {"chatActionRecordingVoiceNote", -808850058},
    {"chatActionUploadingVoiceNote", -613643666},
    {"chatActionUploadingPhoto", 654240583},
    {"chatActionUploadingDocument", 167884362},
    {"chatActionChoosingSticker", 372753697},
    {"chatActionChoosingLocation", -2017893596},
    {"chatActionChoosingContact", -1222507496},
    {"chatActionStartPlayingGame", -865884164},
    {"chatActionRecordingVideoNote", 16523393},
    {"chatActionUploadingVideoNote", 1172364918},
    {"chatActionWatchingAnimations", 2052990641},
    {"chatActionCancel", 1160523958}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::EmojiCategoryType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"emojiCategoryTypeDefault", 1188782699},
    {"emojiCategoryTypeRegularStickers", -1337484846},
    {"emojiCategoryTypeEmojiStatus", 1381282631},
    {"emojiCategoryTypeChatPhoto", 1059063081}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InlineQueryResultsButtonType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inlineQueryResultsButtonTypeStartBot", -23400235},
    {"inlineQueryResultsButtonTypeWebApp", -1197382814}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputMessageContent *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputMessageText", -212805484},
    {"inputMessageAnimation", -210404059},
    {"inputMessageAudio", -626786126},
    {"inputMessageDocument", 1633383097},
    {"inputMessagePaidMedia", -1274819374},
    {"inputMessagePhoto", -810129442},
    {"inputMessageSticker", 1072805625},
    {"inputMessageVideo", -605958271},
    {"inputMessageVideoNote", -714598691},
    {"inputMessageVoiceNote", 1461977004},
    {"inputMessageLocation", 648735088},
    {"inputMessageVenue", 1447926269},
    {"inputMessageContact", -982446849},
    {"inputMessageDice", 841574313},
    {"inputMessageGame", 1252944610},
    {"inputMessageInvoice", -1162047631},
    {"inputMessagePoll", -263337164},
    {"inputMessageStory", -370732053},
    {"inputMessageChecklist", -1722965261},
    {"inputMessageForwarded", -1076506316}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::KeyboardButtonType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"keyboardButtonTypeText", -1773037256},
    {"keyboardButtonTypeRequestPhoneNumber", -1529235527},
    {"keyboardButtonTypeRequestLocation", -125661955},
    {"keyboardButtonTypeRequestPoll", 1902435512},
    {"keyboardButtonTypeRequestUsers", -1738765315},
    {"keyboardButtonTypeRequestChat", 1511138485},
    {"keyboardButtonTypeWebApp", 1892220770}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::NetworkStatisticsEntry *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"networkStatisticsEntryFile", 188452706},
    {"networkStatisticsEntryCall", 737000365}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PremiumLimitType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"premiumLimitTypeSupergroupCount", -247467131},
    {"premiumLimitTypePinnedChatCount", -998947871},
    {"premiumLimitTypeCreatedPublicChatCount", 446086841},
    {"premiumLimitTypeSavedAnimationCount", -19759735},
    {"premiumLimitTypeFavoriteStickerCount", 639754787},
    {"premiumLimitTypeChatFolderCount", 377489774},
    {"premiumLimitTypeChatFolderChosenChatCount", 1691435861},
    {"premiumLimitTypePinnedArchivedChatCount", 1485515276},
    {"premiumLimitTypePinnedSavedMessagesTopicCount", -1544854305},
    {"premiumLimitTypeCaptionLength", 293984314},
    {"premiumLimitTypeBioLength", -1146976765},
    {"premiumLimitTypeChatFolderInviteLinkCount", -128702950},
    {"premiumLimitTypeShareableChatFolderCount", 1612625095},
    {"premiumLimitTypeActiveStoryCount", -1926486372},
    {"premiumLimitTypeWeeklyPostedStoryCount", -506354313},
    {"premiumLimitTypeMonthlyPostedStoryCount", 26329490},
    {"premiumLimitTypeStoryCaptionLength", -1093324030},
    {"premiumLimitTypeStorySuggestedReactionAreaCount", -1170032633},
    {"premiumLimitTypeSimilarChatCount", -1563549935}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ReportReason *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"reportReasonSpam", -1207032897},
    {"reportReasonViolence", 2038679353},
    {"reportReasonPornography", 1306467575},
    {"reportReasonChildAbuse", 761086718},
    {"reportReasonCopyright", 1474441135},
    {"reportReasonUnrelatedLocation", 87562288},
    {"reportReasonFake", 352862176},
    {"reportReasonIllegalDrugs", -61599200},
    {"reportReasonPersonalDetails", -1588882414},
    {"reportReasonCustom", -1380459917}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::StoryPrivacySettings *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"storyPrivacySettingsEveryone", 890847843},
    {"storyPrivacySettingsContacts", 50285309},
    {"storyPrivacySettingsCloseFriends", 2097122144},
    {"storyPrivacySettingsSelectedUsers", -1885772602}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::TransactionDirection *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"transactionDirectionIncoming", -271074103},
    {"transactionDirectionOutgoing", 1638241254}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::affiliateProgramSortOrderProfitability &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::autosaveSettingsScopeGroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::backgroundTypeChatTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_name_, from.extract_field("theme_name")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::businessConnectedBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.recipients_, from.extract_field("recipients")));
  TRY_STATUS(from_json(to.rights_, from.extract_field("rights")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureEmojiStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemNoise &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadData &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.can_manage_chat_, from.extract_field("can_manage_chat")));
  TRY_STATUS(from_json(to.can_change_info_, from.extract_field("can_change_info")));
  TRY_STATUS(from_json(to.can_post_messages_, from.extract_field("can_post_messages")));
  TRY_STATUS(from_json(to.can_edit_messages_, from.extract_field("can_edit_messages")));
  TRY_STATUS(from_json(to.can_delete_messages_, from.extract_field("can_delete_messages")));
  TRY_STATUS(from_json(to.can_invite_users_, from.extract_field("can_invite_users")));
  TRY_STATUS(from_json(to.can_restrict_members_, from.extract_field("can_restrict_members")));
  TRY_STATUS(from_json(to.can_pin_messages_, from.extract_field("can_pin_messages")));
  TRY_STATUS(from_json(to.can_manage_topics_, from.extract_field("can_manage_topics")));
  TRY_STATUS(from_json(to.can_promote_members_, from.extract_field("can_promote_members")));
  TRY_STATUS(from_json(to.can_manage_video_chats_, from.extract_field("can_manage_video_chats")));
  TRY_STATUS(from_json(to.can_post_stories_, from.extract_field("can_post_stories")));
  TRY_STATUS(from_json(to.can_edit_stories_, from.extract_field("can_edit_stories")));
  TRY_STATUS(from_json(to.can_delete_stories_, from.extract_field("can_delete_stories")));
  TRY_STATUS(from_json(to.can_manage_direct_messages_, from.extract_field("can_manage_direct_messages")));
  TRY_STATUS(from_json(to.is_anonymous_, from.extract_field("is_anonymous")));
  return Status::OK();
}

Status from_json(td_api::chatListMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusBanned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.banned_until_date_, from.extract_field("banned_until_date")));
  return Status::OK();
}

Status from_json(td_api::chatPermissions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.can_send_basic_messages_, from.extract_field("can_send_basic_messages")));
  TRY_STATUS(from_json(to.can_send_audios_, from.extract_field("can_send_audios")));
  TRY_STATUS(from_json(to.can_send_documents_, from.extract_field("can_send_documents")));
  TRY_STATUS(from_json(to.can_send_photos_, from.extract_field("can_send_photos")));
  TRY_STATUS(from_json(to.can_send_videos_, from.extract_field("can_send_videos")));
  TRY_STATUS(from_json(to.can_send_video_notes_, from.extract_field("can_send_video_notes")));
  TRY_STATUS(from_json(to.can_send_voice_notes_, from.extract_field("can_send_voice_notes")));
  TRY_STATUS(from_json(to.can_send_polls_, from.extract_field("can_send_polls")));
  TRY_STATUS(from_json(to.can_send_other_messages_, from.extract_field("can_send_other_messages")));
  TRY_STATUS(from_json(to.can_add_link_previews_, from.extract_field("can_add_link_previews")));
  TRY_STATUS(from_json(to.can_change_info_, from.extract_field("can_change_info")));
  TRY_STATUS(from_json(to.can_invite_users_, from.extract_field("can_invite_users")));
  TRY_STATUS(from_json(to.can_pin_messages_, from.extract_field("can_pin_messages")));
  TRY_STATUS(from_json(to.can_create_topics_, from.extract_field("can_create_topics")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenApplePush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, from.extract_field("is_app_sandbox")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenTizenPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reg_id_, from.extract_field("reg_id")));
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeChatPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeNotificationSound &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSelfDestructingVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeWallpaper &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::giftResalePriceTon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.toncoin_cent_count_, from.extract_field("toncoin_cent_count")));
  return Status::OK();
}

Status from_json(td_api::importedContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  TRY_STATUS(from_json(to.note_, from.extract_field("note")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeBuy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inputBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::inputChecklistTask &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::inputGroupCallLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.live_period_, from.extract_field("live_period")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, from.extract_field("thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, from.extract_field("thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.link_preview_options_, from.extract_field("link_preview_options")));
  TRY_STATUS(from_json(to.clear_draft_, from.extract_field("clear_draft")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.voice_note_, from.extract_field("voice_note")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json_bytes(to.waveform_, from.extract_field("waveform")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.self_destruct_type_, from.extract_field("self_destruct_type")));
  return Status::OK();
}

Status from_json(td_api::inputMessageChecklist &to, JsonObject &from) {
  TRY_STATUS(from_json(to.checklist_, from.extract_field("checklist")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPassport &to, JsonObject &from) {
  TRY_STATUS(from_json(to.passport_, from.extract_field("passport")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementTemporaryRegistration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.temporary_registration_, from.extract_field("temporary_registration")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceTranslationFile &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypePreviousVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.venue_provider_, from.extract_field("venue_provider")));
  TRY_STATUS(from_json(to.venue_id_, from.extract_field("venue_id")));
  return Status::OK();
}

Status from_json(td_api::inputSuggestedPostInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.price_, from.extract_field("price")));
  TRY_STATUS(from_json(to.send_date_, from.extract_field("send_date")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotStartInGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.administrator_rights_, from.extract_field("administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeDefaultMessageAutoDeleteTimerSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeLanguageSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePremiumGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_username_, from.extract_field("story_poster_username")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_username_, from.extract_field("chat_username")));
  TRY_STATUS(from_json(to.invite_hash_, from.extract_field("invite_hash")));
  TRY_STATUS(from_json(to.is_live_stream_, from.extract_field("is_live_stream")));
  return Status::OK();
}

Status from_json(td_api::jsonValueObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.members_, from.extract_field("members")));
  return Status::OK();
}

Status from_json(td_api::labeledPricePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.label_, from.extract_field("label")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  return Status::OK();
}

Status from_json(td_api::logStreamDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageCopyOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.send_copy_, from.extract_field("send_copy")));
  TRY_STATUS(from_json(to.replace_caption_, from.extract_field("replace_caption")));
  TRY_STATUS(from_json(to.new_caption_, from.extract_field("new_caption")));
  TRY_STATUS(from_json(to.new_show_caption_above_media_, from.extract_field("new_show_caption_above_media")));
  return Status::OK();
}

Status from_json(td_api::messageSourceChatHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceScreenshot &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkTypeMobile &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optionValueEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeDriverLicense &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypePhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureIncreasedLimits &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureProfileBadge &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureBackgroundForBoth &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeCreatedPublicChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeChatFolderInviteLinkCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumSourceFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureSaveStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::profileTabVoice &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSourceContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::replyMarkupInlineKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rows_, from.extract_field("rows")));
  return Status::OK();
}

Status from_json(td_api::reportReasonPersonalDetails &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterChatPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::starSubscriptionPricing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.period_, from.extract_field("period")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposePremiumGiftCodes &to, JsonObject &from) {
  TRY_STATUS(from_json(to.boosted_chat_id_, from.extract_field("boosted_chat_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::storyListMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionViewChecksHint &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionExtendPremium &to, JsonObject &from) {
  TRY_STATUS(from_json(to.manage_premium_subscription_url_, from.extract_field("manage_premium_subscription_url")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterRestricted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposePremiumGiftCodes &to, JsonObject &from) {
  TRY_STATUS(from_json(to.boosted_chat_id_, from.extract_field("boosted_chat_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  TRY_STATUS(from_json(to.month_count_, from.extract_field("month_count")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMention &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeItalic &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeTextUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::topChatCategoryGroups &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::upgradedGiftAttributeIdSymbol &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_id_, from.extract_field("sticker_id")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowChatInvites &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rules_, from.extract_field("rules")));
  return Status::OK();
}

Status from_json(td_api::addBotMediaPreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  return Status::OK();
}

Status from_json(td_api::addFileToDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.priority_, from.extract_field("priority")));
  return Status::OK();
}

Status from_json(td_api::addProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, from.extract_field("server")));
  TRY_STATUS(from_json(to.port_, from.extract_field("port")));
  TRY_STATUS(from_json(to.enable_, from.extract_field("enable")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::addStoryAlbumStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_album_id_, from.extract_field("story_album_id")));
  TRY_STATUS(from_json(to.story_ids_, from.extract_field("story_ids")));
  return Status::OK();
}

Status from_json(td_api::applyPremiumGiftCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::canPostStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::changeImportedContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contacts_, from.extract_field("contacts")));
  return Status::OK();
}

Status from_json(td_api::checkChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::checkRecoveryEmailAddressCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::clearRecentStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  return Status::OK();
}

Status from_json(td_api::closeSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, from.extract_field("secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::createCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.protocol_, from.extract_field("protocol")));
  TRY_STATUS(from_json(to.is_video_, from.extract_field("is_video")));
  return Status::OK();
}

Status from_json(td_api::createNewBasicGroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, from.extract_field("message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::createVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.start_date_, from.extract_field("start_date")));
  TRY_STATUS(from_json(to.is_rtmp_stream_, from.extract_field("is_rtmp_stream")));
  return Status::OK();
}

Status from_json(td_api::deleteBusinessConnectedBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChatMessagesBySender &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  return Status::OK();
}

Status from_json(td_api::deleteLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedMessagesTopicMessagesByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.min_date_, from.extract_field("min_date")));
  TRY_STATUS(from_json(to.max_date_, from.extract_field("max_date")));
  return Status::OK();
}

Status from_json(td_api::disconnectAffiliateProgram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.affiliate_, from.extract_field("affiliate")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::editBusinessMessageLiveLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.live_period_, from.extract_field("live_period")));
  TRY_STATUS(from_json(to.heading_, from.extract_field("heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, from.extract_field("proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::editCustomLanguagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.info_, from.extract_field("info")));
  return Status::OK();
}

Status from_json(td_api::editMessageLiveLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.live_period_, from.extract_field("live_period")));
  TRY_STATUS(from_json(to.heading_, from.extract_field("heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, from.extract_field("proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::editStoryCover &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.cover_frame_timestamp_, from.extract_field("cover_frame_timestamp")));
  return Status::OK();
}

Status from_json(td_api::getAccountTtl &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAttachedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::getBasicGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, from.extract_field("basic_group_id")));
  return Status::OK();
}

Status from_json(td_api::getBotSimilarBots &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::getChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderNewChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  return Status::OK();
}

Status from_json(td_api::getChatRevenueWithdrawalUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDatabaseStatistics &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDirectMessagesChatTopicHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getFavoriteStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getForumTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_date_, from.extract_field("offset_date")));
  TRY_STATUS(from_json(to.offset_message_id_, from.extract_field("offset_message_id")));
  TRY_STATUS(from_json(to.offset_forum_topic_id_, from.extract_field("offset_forum_topic_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getGroupCallParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_group_call_, from.extract_field("input_group_call")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getInternalLinkType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::getLogStream &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getMe &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getMessageImportConfirmationText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageThreadHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getPassportAuthorizationForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.public_key_, from.extract_field("public_key")));
  TRY_STATUS(from_json(to.nonce_, from.extract_field("nonce")));
  return Status::OK();
}

Status from_json(td_api::getPreferredCountryLanguage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  return Status::OK();
}

Status from_json(td_api::getPreparedInlineMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.prepared_message_id_, from.extract_field("prepared_message_id")));
  return Status::OK();
}

Status from_json(td_api::getRecentInlineBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSavedAnimations &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSearchedForTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_prefix_, from.extract_field("tag_prefix")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getStarWithdrawalUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::getSuitableDiscussionChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getThemeParametersJsonString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  return Status::OK();
}

Status from_json(td_api::getUpgradedGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::getUserProfileAudios &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getWebAppLinkUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.web_app_short_name_, from.extract_field("web_app_short_name")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::inviteGroupCallParticipant &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.is_video_, from.extract_field("is_video")));
  return Status::OK();
}

Status from_json(td_api::leaveGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::markChecklistTasksAsDone &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.marked_as_done_task_ids_, from.extract_field("marked_as_done_task_ids")));
  TRY_STATUS(from_json(to.marked_as_not_done_task_ids_, from.extract_field("marked_as_not_done_task_ids")));
  return Status::OK();
}

Status from_json(td_api::parseMarkdown &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::processPushNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::readFilePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.count_, from.extract_field("count")));
  return Status::OK();
}

Status from_json(td_api::removeBusinessConnectedBotFromChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::removeNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_group_id_, from.extract_field("notification_group_id")));
  TRY_STATUS(from_json(to.notification_id_, from.extract_field("notification_id")));
  return Status::OK();
}

Status from_json(td_api::removeSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_sound_id_, from.extract_field("notification_sound_id")));
  return Status::OK();
}

Status from_json(td_api::reorderGiftCollectionGifts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.collection_id_, from.extract_field("collection_id")));
  TRY_STATUS(from_json(to.received_gift_ids_, from.extract_field("received_gift_ids")));
  return Status::OK();
}

Status from_json(td_api::replaceVideoChatRtmpUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::reportSupergroupAntiSpamFalsePositive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::resendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  TRY_STATUS(from_json(to.paid_message_star_count_, from.extract_field("paid_message_star_count")));
  return Status::OK();
}

Status from_json(td_api::revokeChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::searchChatMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  return Status::OK();
}

Status from_json(td_api::searchInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchPublicStoriesByVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.venue_provider_, from.extract_field("venue_provider")));
  TRY_STATUS(from_json(to.venue_id_, from.extract_field("venue_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchUserByPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::sendCallLog &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.log_file_, from.extract_field("log_file")));
  return Status::OK();
}

Status from_json(td_api::sendMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::sendWebAppCustomRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.method_, from.extract_field("method")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::setAuthenticationPremiumPurchaseTransaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.transaction_, from.extract_field("transaction")));
  TRY_STATUS(from_json(to.is_restore_, from.extract_field("is_restore")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  return Status::OK();
}

Status from_json(td_api::setBotUpdatesStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.pending_update_count_, from.extract_field("pending_update_count")));
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::setBusinessLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::setChatClientData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.client_data_, from.extract_field("client_data")));
  return Status::OK();
}

Status from_json(td_api::setChatMessageSender &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_sender_id_, from.extract_field("message_sender_id")));
  return Status::OK();
}

Status from_json(td_api::setChatTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::setDefaultGroupAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.default_group_administrator_rights_, from.extract_field("default_group_administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::setGiftResalePrice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  TRY_STATUS(from_json(to.price_, from.extract_field("price")));
  return Status::OK();
}

Status from_json(td_api::setLoginEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.new_login_email_address_, from.extract_field("new_login_email_address")));
  return Status::OK();
}

Status from_json(td_api::setNewChatPrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setPinnedGifts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.received_gift_ids_, from.extract_field("received_gift_ids")));
  return Status::OK();
}

Status from_json(td_api::setRecoveryEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json(to.new_recovery_email_address_, from.extract_field("new_recovery_email_address")));
  return Status::OK();
}

Status from_json(td_api::setStoryAlbumName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_album_id_, from.extract_field("story_album_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::setUpgradedGiftColors &to, JsonObject &from) {
  TRY_STATUS(from_json(to.upgraded_gift_colors_id_, from.extract_field("upgraded_gift_colors_id")));
  return Status::OK();
}

Status from_json(td_api::shareChatWithBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.button_id_, from.extract_field("button_id")));
  TRY_STATUS(from_json(to.shared_chat_id_, from.extract_field("shared_chat_id")));
  TRY_STATUS(from_json(to.only_check_, from.extract_field("only_check")));
  return Status::OK();
}

Status from_json(td_api::suggestUserProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::toggleBotCanManageEmojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.can_manage_emoji_status_, from.extract_field("can_manage_emoji_status")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallCanSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.can_send_messages_, from.extract_field("can_send_messages")));
  return Status::OK();
}

Status from_json(td_api::toggleSessionCanAcceptSecretChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  TRY_STATUS(from_json(to.can_accept_secret_chats_, from.extract_field("can_accept_secret_chats")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupJoinByRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.join_by_request_, from.extract_field("join_by_request")));
  return Status::OK();
}

Status from_json(td_api::transferGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  TRY_STATUS(from_json(to.new_owner_id_, from.extract_field("new_owner_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::uploadStickerFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.sticker_format_, from.extract_field("sticker_format")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::acceptedGiftTypes &object) {
  auto jo = jv.enter_object();
  jo("@type", "acceptedGiftTypes");
  jo("unlimited_gifts", JsonBool{object.unlimited_gifts_});
  jo("limited_gifts", JsonBool{object.limited_gifts_});
  jo("upgraded_gifts", JsonBool{object.upgraded_gifts_});
  jo("premium_subscription", JsonBool{object.premium_subscription_});
}

void to_json(JsonValueScope &jv, const td_api::affiliateProgramParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "affiliateProgramParameters");
  jo("commission_per_mille", object.commission_per_mille_);
  jo("month_count", object.month_count_);
}

void to_json(JsonValueScope &jv, const td_api::attachmentMenuBotColor &object) {
  auto jo = jv.enter_object();
  jo("@type", "attachmentMenuBotColor");
  jo("light_color", object.light_color_);
  jo("dark_color", object.dark_color_);
}

void to_json(JsonValueScope &jv, const td_api::autosaveSettingsException &object) {
  auto jo = jv.enter_object();
  jo("@type", "autosaveSettingsException");
  jo("chat_id", object.chat_id_);
  if (object.settings_) {
    jo("settings", ToJson(*object.settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::backgrounds &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgrounds");
  jo("backgrounds", ToJson(object.backgrounds_));
}

void to_json(JsonValueScope &jv, const td_api::botInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "botInfo");
  jo("short_description", object.short_description_);
  jo("description", object.description_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  if (object.menu_button_) {
    jo("menu_button", ToJson(*object.menu_button_));
  }
  jo("commands", ToJson(object.commands_));
  jo("privacy_policy_url", object.privacy_policy_url_);
  if (object.default_group_administrator_rights_) {
    jo("default_group_administrator_rights", ToJson(*object.default_group_administrator_rights_));
  }
  if (object.default_channel_administrator_rights_) {
    jo("default_channel_administrator_rights", ToJson(*object.default_channel_administrator_rights_));
  }
  if (object.affiliate_program_) {
    jo("affiliate_program", ToJson(*object.affiliate_program_));
  }
  jo("web_app_background_light_color", object.web_app_background_light_color_);
  jo("web_app_background_dark_color", object.web_app_background_dark_color_);
  jo("web_app_header_light_color", object.web_app_header_light_color_);
  jo("web_app_header_dark_color", object.web_app_header_dark_color_);
  if (object.verification_parameters_) {
    jo("verification_parameters", ToJson(*object.verification_parameters_));
  }
  jo("can_get_revenue_statistics", JsonBool{object.can_get_revenue_statistics_});
  jo("can_manage_emoji_status", JsonBool{object.can_manage_emoji_status_});
  jo("has_media_previews", JsonBool{object.has_media_previews_});
  if (object.edit_commands_link_) {
    jo("edit_commands_link", ToJson(*object.edit_commands_link_));
  }
  if (object.edit_description_link_) {
    jo("edit_description_link", ToJson(*object.edit_description_link_));
  }
  if (object.edit_description_media_link_) {
    jo("edit_description_media_link", ToJson(*object.edit_description_media_link_));
  }
  if (object.edit_settings_link_) {
    jo("edit_settings_link", ToJson(*object.edit_settings_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::BusinessAwayMessageSchedule &object) {
  td_api::downcast_call(const_cast<td_api::BusinessAwayMessageSchedule &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::businessAwayMessageScheduleAlways &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessAwayMessageScheduleAlways");
}

void to_json(JsonValueScope &jv, const td_api::businessAwayMessageScheduleOutsideOfOpeningHours &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessAwayMessageScheduleOutsideOfOpeningHours");
}

void to_json(JsonValueScope &jv, const td_api::businessAwayMessageScheduleCustom &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessAwayMessageScheduleCustom");
  jo("start_date", object.start_date_);
  jo("end_date", object.end_date_);
}

void to_json(JsonValueScope &jv, const td_api::BusinessFeature &object) {
  td_api::downcast_call(const_cast<td_api::BusinessFeature &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureLocation");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureOpeningHours &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureOpeningHours");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureQuickReplies &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureQuickReplies");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureGreetingMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureGreetingMessage");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureAwayMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureAwayMessage");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureAccountLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureAccountLinks");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureStartPage &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureStartPage");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureBots &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureBots");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureEmojiStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureEmojiStatus");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureChatFolderTags &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureChatFolderTags");
}

void to_json(JsonValueScope &jv, const td_api::businessFeatureUpgradedStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatureUpgradedStories");
}

void to_json(JsonValueScope &jv, const td_api::businessOpeningHoursInterval &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessOpeningHoursInterval");
  jo("start_minute", object.start_minute_);
  jo("end_minute", object.end_minute_);
}

void to_json(JsonValueScope &jv, const td_api::CallState &object) {
  td_api::downcast_call(const_cast<td_api::CallState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::callStatePending &object) {
  auto jo = jv.enter_object();
  jo("@type", "callStatePending");
  jo("is_created", JsonBool{object.is_created_});
  jo("is_received", JsonBool{object.is_received_});
}

void to_json(JsonValueScope &jv, const td_api::callStateExchangingKeys &object) {
  auto jo = jv.enter_object();
  jo("@type", "callStateExchangingKeys");
}

void to_json(JsonValueScope &jv, const td_api::callStateReady &object) {
  auto jo = jv.enter_object();
  jo("@type", "callStateReady");
  if (object.protocol_) {
    jo("protocol", ToJson(*object.protocol_));
  }
  jo("servers", ToJson(object.servers_));
  jo("config", object.config_);
  jo("encryption_key", base64_encode(object.encryption_key_));
  jo("emojis", ToJson(object.emojis_));
  jo("allow_p2p", JsonBool{object.allow_p2p_});
  jo("is_group_call_supported", JsonBool{object.is_group_call_supported_});
  jo("custom_parameters", object.custom_parameters_);
}

void to_json(JsonValueScope &jv, const td_api::callStateHangingUp &object) {
  auto jo = jv.enter_object();
  jo("@type", "callStateHangingUp");
}

void to_json(JsonValueScope &jv, const td_api::callStateDiscarded &object) {
  auto jo = jv.enter_object();
  jo("@type", "callStateDiscarded");
  if (object.reason_) {
    jo("reason", ToJson(*object.reason_));
  }
  jo("need_rating", JsonBool{object.need_rating_});
  jo("need_debug_information", JsonBool{object.need_debug_information_});
  jo("need_log", JsonBool{object.need_log_});
}

void to_json(JsonValueScope &jv, const td_api::callStateError &object) {
  auto jo = jv.enter_object();
  jo("@type", "callStateError");
  if (object.error_) {
    jo("error", ToJson(*object.error_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ChatActionBar &object) {
  td_api::downcast_call(const_cast<td_api::ChatActionBar &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportSpam &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarReportSpam");
  jo("can_unarchive", JsonBool{object.can_unarchive_});
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarInviteMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarInviteMembers");
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportAddBlock &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarReportAddBlock");
  jo("can_unarchive", JsonBool{object.can_unarchive_});
  if (object.account_info_) {
    jo("account_info", ToJson(*object.account_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarAddContact &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarAddContact");
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarSharePhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarSharePhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarJoinRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarJoinRequest");
  jo("title", object.title_);
  jo("is_channel", JsonBool{object.is_channel_});
  jo("request_date", object.request_date_);
}

void to_json(JsonValueScope &jv, const td_api::chatBoostLevelFeatures &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostLevelFeatures");
  jo("level", object.level_);
  jo("story_per_day_count", object.story_per_day_count_);
  jo("custom_emoji_reaction_count", object.custom_emoji_reaction_count_);
  jo("title_color_count", object.title_color_count_);
  jo("profile_accent_color_count", object.profile_accent_color_count_);
  jo("can_set_profile_background_custom_emoji", JsonBool{object.can_set_profile_background_custom_emoji_});
  jo("accent_color_count", object.accent_color_count_);
  jo("can_set_background_custom_emoji", JsonBool{object.can_set_background_custom_emoji_});
  jo("can_set_emoji_status", JsonBool{object.can_set_emoji_status_});
  jo("chat_theme_background_count", object.chat_theme_background_count_);
  jo("can_set_custom_background", JsonBool{object.can_set_custom_background_});
  jo("can_set_custom_emoji_sticker_set", JsonBool{object.can_set_custom_emoji_sticker_set_});
  jo("can_enable_automatic_translation", JsonBool{object.can_enable_automatic_translation_});
  jo("can_recognize_speech", JsonBool{object.can_recognize_speech_});
  jo("can_disable_sponsored_messages", JsonBool{object.can_disable_sponsored_messages_});
}

void to_json(JsonValueScope &jv, const td_api::chatEvents &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEvents");
  jo("events", ToJson(object.events_));
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkCount");
  jo("user_id", object.user_id_);
  jo("invite_link_count", object.invite_link_count_);
  jo("revoked_invite_link_count", object.revoked_invite_link_count_);
}

void to_json(JsonValueScope &jv, const td_api::chatJoinRequestsInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatJoinRequestsInfo");
  jo("total_count", object.total_count_);
  jo("user_ids", ToJson(object.user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::chatNotificationSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatNotificationSettings");
  jo("use_default_mute_for", JsonBool{object.use_default_mute_for_});
  jo("mute_for", object.mute_for_);
  jo("use_default_sound", JsonBool{object.use_default_sound_});
  jo("sound_id", ToJson(JsonInt64{object.sound_id_}));
  jo("use_default_show_preview", JsonBool{object.use_default_show_preview_});
  jo("show_preview", JsonBool{object.show_preview_});
  jo("use_default_mute_stories", JsonBool{object.use_default_mute_stories_});
  jo("mute_stories", JsonBool{object.mute_stories_});
  jo("use_default_story_sound", JsonBool{object.use_default_story_sound_});
  jo("story_sound_id", ToJson(JsonInt64{object.story_sound_id_}));
  jo("use_default_show_story_poster", JsonBool{object.use_default_show_story_poster_});
  jo("show_story_poster", JsonBool{object.show_story_poster_});
  jo("use_default_disable_pinned_message_notifications", JsonBool{object.use_default_disable_pinned_message_notifications_});
  jo("disable_pinned_message_notifications", JsonBool{object.disable_pinned_message_notifications_});
  jo("use_default_disable_mention_notifications", JsonBool{object.use_default_disable_mention_notifications_});
  jo("disable_mention_notifications", JsonBool{object.disable_mention_notifications_});
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueStatistics");
  if (object.revenue_by_hour_graph_) {
    jo("revenue_by_hour_graph", ToJson(*object.revenue_by_hour_graph_));
  }
  if (object.revenue_graph_) {
    jo("revenue_graph", ToJson(*object.revenue_graph_));
  }
  if (object.revenue_amount_) {
    jo("revenue_amount", ToJson(*object.revenue_amount_));
  }
  jo("usd_rate", object.usd_rate_);
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsMessageSenderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsMessageSenderInfo");
  jo("user_id", object.user_id_);
  jo("sent_message_count", object.sent_message_count_);
  jo("average_character_count", object.average_character_count_);
}

void to_json(JsonValueScope &jv, const td_api::closeBirthdayUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "closeBirthdayUser");
  jo("user_id", object.user_id_);
  if (object.birthdate_) {
    jo("birthdate", ToJson(*object.birthdate_));
  }
}

void to_json(JsonValueScope &jv, const td_api::count &object) {
  auto jo = jv.enter_object();
  jo("@type", "count");
  jo("count", object.count_);
}

void to_json(JsonValueScope &jv, const td_api::dateRange &object) {
  auto jo = jv.enter_object();
  jo("@type", "dateRange");
  jo("start_date", object.start_date_);
  jo("end_date", object.end_date_);
}

void to_json(JsonValueScope &jv, const td_api::EmailAddressResetState &object) {
  td_api::downcast_call(const_cast<td_api::EmailAddressResetState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::emailAddressResetStateAvailable &object) {
  auto jo = jv.enter_object();
  jo("@type", "emailAddressResetStateAvailable");
  jo("wait_period", object.wait_period_);
}

void to_json(JsonValueScope &jv, const td_api::emailAddressResetStatePending &object) {
  auto jo = jv.enter_object();
  jo("@type", "emailAddressResetStatePending");
  jo("reset_in", object.reset_in_);
}

void to_json(JsonValueScope &jv, const td_api::emojiStatusCustomEmojis &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatusCustomEmojis");
  jo("custom_emoji_ids", ToJson(JsonVectorInt64{object.custom_emoji_ids_}));
}

void to_json(JsonValueScope &jv, const td_api::failedToAddMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "failedToAddMembers");
  jo("failed_to_add_members", ToJson(object.failed_to_add_members_));
}

void to_json(JsonValueScope &jv, const td_api::forumTopicInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "forumTopicInfo");
  jo("chat_id", object.chat_id_);
  jo("forum_topic_id", object.forum_topic_id_);
  jo("name", object.name_);
  if (object.icon_) {
    jo("icon", ToJson(*object.icon_));
  }
  jo("creation_date", object.creation_date_);
  if (object.creator_id_) {
    jo("creator_id", ToJson(*object.creator_id_));
  }
  jo("is_general", JsonBool{object.is_general_});
  jo("is_outgoing", JsonBool{object.is_outgoing_});
  jo("is_closed", JsonBool{object.is_closed_});
  jo("is_hidden", JsonBool{object.is_hidden_});
  jo("is_name_implicit", JsonBool{object.is_name_implicit_});
}

void to_json(JsonValueScope &jv, const td_api::foundPosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundPosition");
  jo("position", object.position_);
}

void to_json(JsonValueScope &jv, const td_api::gift &object) {
  auto jo = jv.enter_object();
  jo("@type", "gift");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("publisher_chat_id", object.publisher_chat_id_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  jo("star_count", object.star_count_);
  jo("default_sell_star_count", object.default_sell_star_count_);
  jo("upgrade_star_count", object.upgrade_star_count_);
  jo("has_colors", JsonBool{object.has_colors_});
  jo("is_for_birthday", JsonBool{object.is_for_birthday_});
  jo("is_premium", JsonBool{object.is_premium_});
  jo("next_send_date", object.next_send_date_);
  if (object.user_limits_) {
    jo("user_limits", ToJson(*object.user_limits_));
  }
  if (object.overall_limits_) {
    jo("overall_limits", ToJson(*object.overall_limits_));
  }
  jo("first_send_date", object.first_send_date_);
  jo("last_send_date", object.last_send_date_);
}

void to_json(JsonValueScope &jv, const td_api::GiftResaleResult &object) {
  td_api::downcast_call(const_cast<td_api::GiftResaleResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::giftResaleResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftResaleResultOk");
}

void to_json(JsonValueScope &jv, const td_api::giftResaleResultPriceIncreased &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftResaleResultPriceIncreased");
  if (object.price_) {
    jo("price", ToJson(*object.price_));
  }
}

void to_json(JsonValueScope &jv, const td_api::groupCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCall");
  jo("id", object.id_);
  jo("title", object.title_);
  jo("invite_link", object.invite_link_);
  jo("scheduled_start_date", object.scheduled_start_date_);
  jo("enabled_start_notification", JsonBool{object.enabled_start_notification_});
  jo("is_active", JsonBool{object.is_active_});
  jo("is_video_chat", JsonBool{object.is_video_chat_});
  jo("is_rtmp_stream", JsonBool{object.is_rtmp_stream_});
  jo("is_joined", JsonBool{object.is_joined_});
  jo("need_rejoin", JsonBool{object.need_rejoin_});
  jo("is_owned", JsonBool{object.is_owned_});
  jo("can_be_managed", JsonBool{object.can_be_managed_});
  jo("participant_count", object.participant_count_);
  jo("has_hidden_listeners", JsonBool{object.has_hidden_listeners_});
  jo("loaded_all_participants", JsonBool{object.loaded_all_participants_});
  jo("recent_speakers", ToJson(object.recent_speakers_));
  jo("is_my_video_enabled", JsonBool{object.is_my_video_enabled_});
  jo("is_my_video_paused", JsonBool{object.is_my_video_paused_});
  jo("can_enable_video", JsonBool{object.can_enable_video_});
  jo("mute_new_participants", JsonBool{object.mute_new_participants_});
  jo("can_toggle_mute_new_participants", JsonBool{object.can_toggle_mute_new_participants_});
  jo("can_send_messages", JsonBool{object.can_send_messages_});
  jo("can_toggle_can_send_messages", JsonBool{object.can_toggle_can_send_messages_});
  jo("record_duration", object.record_duration_);
  jo("is_video_recorded", JsonBool{object.is_video_recorded_});
  jo("duration", object.duration_);
}

void to_json(JsonValueScope &jv, const td_api::httpUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "httpUrl");
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::inputChecklist &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputChecklist");
  if (object.title_) {
    jo("title", ToJson(*object.title_));
  }
  jo("tasks", ToJson(object.tasks_));
  jo("others_can_add_tasks", JsonBool{object.others_can_add_tasks_});
  jo("others_can_mark_tasks_as_done", JsonBool{object.others_can_mark_tasks_as_done_});
}

void to_json(JsonValueScope &jv, const td_api::inputThumbnail &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputThumbnail");
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("width", object.width_);
  jo("height", object.height_);
}

void to_json(JsonValueScope &jv, const td_api::labeledPricePart &object) {
  auto jo = jv.enter_object();
  jo("@type", "labeledPricePart");
  jo("label", object.label_);
  jo("amount", object.amount_);
}

void to_json(JsonValueScope &jv, const td_api::localFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "localFile");
  jo("path", object.path_);
  jo("can_be_downloaded", JsonBool{object.can_be_downloaded_});
  jo("can_be_deleted", JsonBool{object.can_be_deleted_});
  jo("is_downloading_active", JsonBool{object.is_downloading_active_});
  jo("is_downloading_completed", JsonBool{object.is_downloading_completed_});
  jo("download_offset", object.download_offset_);
  jo("downloaded_prefix_size", object.downloaded_prefix_size_);
  jo("downloaded_size", object.downloaded_size_);
}

void to_json(JsonValueScope &jv, const td_api::MaskPoint &object) {
  td_api::downcast_call(const_cast<td_api::MaskPoint &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::maskPointForehead &object) {
  auto jo = jv.enter_object();
  jo("@type", "maskPointForehead");
}

void to_json(JsonValueScope &jv, const td_api::maskPointEyes &object) {
  auto jo = jv.enter_object();
  jo("@type", "maskPointEyes");
}

void to_json(JsonValueScope &jv, const td_api::maskPointMouth &object) {
  auto jo = jv.enter_object();
  jo("@type", "maskPointMouth");
}

void to_json(JsonValueScope &jv, const td_api::maskPointChin &object) {
  auto jo = jv.enter_object();
  jo("@type", "maskPointChin");
}

void to_json(JsonValueScope &jv, const td_api::MessageEffectType &object) {
  td_api::downcast_call(const_cast<td_api::MessageEffectType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageEffectTypeEmojiReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageEffectTypeEmojiReaction");
  if (object.select_animation_) {
    jo("select_animation", ToJson(*object.select_animation_));
  }
  if (object.effect_animation_) {
    jo("effect_animation", ToJson(*object.effect_animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageEffectTypePremiumSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageEffectTypePremiumSticker");
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messagePositions &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePositions");
  jo("total_count", object.total_count_);
  jo("positions", ToJson(object.positions_));
}

void to_json(JsonValueScope &jv, const td_api::MessageSender &object) {
  td_api::downcast_call(const_cast<td_api::MessageSender &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageSenderUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSenderUser");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageSenderChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSenderChat");
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::minithumbnail &object) {
  auto jo = jv.enter_object();
  jo("@type", "minithumbnail");
  jo("width", object.width_);
  jo("height", object.height_);
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::notificationSound &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationSound");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("duration", object.duration_);
  jo("date", object.date_);
  jo("title", object.title_);
  jo("data", object.data_);
  if (object.sound_) {
    jo("sound", ToJson(*object.sound_));
  }
}

void to_json(JsonValueScope &jv, const td_api::PageBlockHorizontalAlignment &object) {
  td_api::downcast_call(const_cast<td_api::PageBlockHorizontalAlignment &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::pageBlockHorizontalAlignmentLeft &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockHorizontalAlignmentLeft");
}

void to_json(JsonValueScope &jv, const td_api::pageBlockHorizontalAlignmentCenter &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockHorizontalAlignmentCenter");
}

void to_json(JsonValueScope &jv, const td_api::pageBlockHorizontalAlignmentRight &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockHorizontalAlignmentRight");
}

void to_json(JsonValueScope &jv, const td_api::PassportElement &object) {
  td_api::downcast_call(const_cast<td_api::PassportElement &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::passportElementPersonalDetails &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementPersonalDetails");
  if (object.personal_details_) {
    jo("personal_details", ToJson(*object.personal_details_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementPassport &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementPassport");
  if (object.passport_) {
    jo("passport", ToJson(*object.passport_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementDriverLicense &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementDriverLicense");
  if (object.driver_license_) {
    jo("driver_license", ToJson(*object.driver_license_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementIdentityCard &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementIdentityCard");
  if (object.identity_card_) {
    jo("identity_card", ToJson(*object.identity_card_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementInternalPassport &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementInternalPassport");
  if (object.internal_passport_) {
    jo("internal_passport", ToJson(*object.internal_passport_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementAddress");
  if (object.address_) {
    jo("address", ToJson(*object.address_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementUtilityBill &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementUtilityBill");
  if (object.utility_bill_) {
    jo("utility_bill", ToJson(*object.utility_bill_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementBankStatement &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementBankStatement");
  if (object.bank_statement_) {
    jo("bank_statement", ToJson(*object.bank_statement_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementRentalAgreement &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementRentalAgreement");
  if (object.rental_agreement_) {
    jo("rental_agreement", ToJson(*object.rental_agreement_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementPassportRegistration &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementPassportRegistration");
  if (object.passport_registration_) {
    jo("passport_registration", ToJson(*object.passport_registration_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementTemporaryRegistration &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementTemporaryRegistration");
  if (object.temporary_registration_) {
    jo("temporary_registration", ToJson(*object.temporary_registration_));
  }
}

void to_json(JsonValueScope &jv, const td_api::passportElementPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementPhoneNumber");
  jo("phone_number", object.phone_number_);
}

void to_json(JsonValueScope &jv, const td_api::passportElementEmailAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementEmailAddress");
  jo("email_address", object.email_address_);
}

void to_json(JsonValueScope &jv, const td_api::paymentForm &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentForm");
  jo("id", ToJson(JsonInt64{object.id_}));
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("seller_bot_user_id", object.seller_bot_user_id_);
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::phoneNumberInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "phoneNumberInfo");
  if (object.country_) {
    jo("country", ToJson(*object.country_));
  }
  jo("country_calling_code", object.country_calling_code_);
  jo("formatted_phone_number", object.formatted_phone_number_);
  jo("is_anonymous", JsonBool{object.is_anonymous_});
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatures &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatures");
  jo("features", ToJson(object.features_));
  jo("limits", ToJson(object.limits_));
  if (object.payment_link_) {
    jo("payment_link", ToJson(*object.payment_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::premiumState &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumState");
  if (object.state_) {
    jo("state", ToJson(*object.state_));
  }
  jo("payment_options", ToJson(object.payment_options_));
  jo("animations", ToJson(object.animations_));
  jo("business_animations", ToJson(object.business_animations_));
}

void to_json(JsonValueScope &jv, const td_api::ProfileTab &object) {
  td_api::downcast_call(const_cast<td_api::ProfileTab &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::profileTabPosts &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabPosts");
}

void to_json(JsonValueScope &jv, const td_api::profileTabGifts &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabGifts");
}

void to_json(JsonValueScope &jv, const td_api::profileTabMedia &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabMedia");
}

void to_json(JsonValueScope &jv, const td_api::profileTabFiles &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabFiles");
}

void to_json(JsonValueScope &jv, const td_api::profileTabLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabLinks");
}

void to_json(JsonValueScope &jv, const td_api::profileTabMusic &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabMusic");
}

void to_json(JsonValueScope &jv, const td_api::profileTabVoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabVoice");
}

void to_json(JsonValueScope &jv, const td_api::profileTabGifs &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileTabGifs");
}

void to_json(JsonValueScope &jv, const td_api::quickReplyMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "quickReplyMessage");
  jo("id", object.id_);
  if (object.sending_state_) {
    jo("sending_state", ToJson(*object.sending_state_));
  }
  jo("can_be_edited", JsonBool{object.can_be_edited_});
  jo("reply_to_message_id", object.reply_to_message_id_);
  jo("via_bot_user_id", object.via_bot_user_id_);
  jo("media_album_id", ToJson(JsonInt64{object.media_album_id_}));
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
  if (object.reply_markup_) {
    jo("reply_markup", ToJson(*object.reply_markup_));
  }
}

void to_json(JsonValueScope &jv, const td_api::receivedGifts &object) {
  auto jo = jv.enter_object();
  jo("@type", "receivedGifts");
  jo("total_count", object.total_count_);
  jo("gifts", ToJson(object.gifts_));
  jo("are_notifications_enabled", JsonBool{object.are_notifications_enabled_});
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::ReportStoryResult &object) {
  td_api::downcast_call(const_cast<td_api::ReportStoryResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::reportStoryResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportStoryResultOk");
}

void to_json(JsonValueScope &jv, const td_api::reportStoryResultOptionRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportStoryResultOptionRequired");
  jo("title", object.title_);
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::reportStoryResultTextRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportStoryResultTextRequired");
  jo("option_id", base64_encode(object.option_id_));
  jo("is_optional", JsonBool{object.is_optional_});
}

void to_json(JsonValueScope &jv, const td_api::savedMessagesTopic &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedMessagesTopic");
  jo("id", object.id_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("is_pinned", JsonBool{object.is_pinned_});
  jo("order", ToJson(JsonInt64{object.order_}));
  if (object.last_message_) {
    jo("last_message", ToJson(*object.last_message_));
  }
  if (object.draft_message_) {
    jo("draft_message", ToJson(*object.draft_message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::session &object) {
  auto jo = jv.enter_object();
  jo("@type", "session");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("is_current", JsonBool{object.is_current_});
  jo("is_password_pending", JsonBool{object.is_password_pending_});
  jo("is_unconfirmed", JsonBool{object.is_unconfirmed_});
  jo("can_accept_secret_chats", JsonBool{object.can_accept_secret_chats_});
  jo("can_accept_calls", JsonBool{object.can_accept_calls_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("api_id", object.api_id_);
  jo("application_name", object.application_name_);
  jo("application_version", object.application_version_);
  jo("is_official_application", JsonBool{object.is_official_application_});
  jo("device_model", object.device_model_);
  jo("platform", object.platform_);
  jo("system_version", object.system_version_);
  jo("log_in_date", object.log_in_date_);
  jo("last_active_date", object.last_active_date_);
  jo("ip_address", object.ip_address_);
  jo("location", object.location_);
}

void to_json(JsonValueScope &jv, const td_api::sponsoredMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "sponsoredMessage");
  jo("message_id", object.message_id_);
  jo("is_recommended", JsonBool{object.is_recommended_});
  jo("can_be_reported", JsonBool{object.can_be_reported_});
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
  if (object.sponsor_) {
    jo("sponsor", ToJson(*object.sponsor_));
  }
  jo("title", object.title_);
  jo("button_text", object.button_text_);
  jo("accent_color_id", object.accent_color_id_);
  jo("background_custom_emoji_id", ToJson(JsonInt64{object.background_custom_emoji_id_}));
  jo("additional_info", object.additional_info_);
}

void to_json(JsonValueScope &jv, const td_api::starRevenueStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "starRevenueStatistics");
  if (object.revenue_by_day_graph_) {
    jo("revenue_by_day_graph", ToJson(*object.revenue_by_day_graph_));
  }
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
  jo("usd_rate", object.usd_rate_);
}

void to_json(JsonValueScope &jv, const td_api::StatisticalGraph &object) {
  td_api::downcast_call(const_cast<td_api::StatisticalGraph &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::statisticalGraphData &object) {
  auto jo = jv.enter_object();
  jo("@type", "statisticalGraphData");
  jo("json_data", object.json_data_);
  jo("zoom_token", object.zoom_token_);
}

void to_json(JsonValueScope &jv, const td_api::statisticalGraphAsync &object) {
  auto jo = jv.enter_object();
  jo("@type", "statisticalGraphAsync");
  jo("token", object.token_);
}

void to_json(JsonValueScope &jv, const td_api::statisticalGraphError &object) {
  auto jo = jv.enter_object();
  jo("@type", "statisticalGraphError");
  jo("error_message", object.error_message_);
}

void to_json(JsonValueScope &jv, const td_api::stickers &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickers");
  jo("stickers", ToJson(object.stickers_));
}

void to_json(JsonValueScope &jv, const td_api::storyArea &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyArea");
  if (object.position_) {
    jo("position", ToJson(*object.position_));
  }
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::StoryList &object) {
  td_api::downcast_call(const_cast<td_api::StoryList &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::storyListMain &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyListMain");
}

void to_json(JsonValueScope &jv, const td_api::storyListArchive &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyListArchive");
}

void to_json(JsonValueScope &jv, const td_api::SuggestedPostRefundReason &object) {
  td_api::downcast_call(const_cast<td_api::SuggestedPostRefundReason &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostRefundReasonPostDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostRefundReasonPostDeleted");
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostRefundReasonPaymentRefunded &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostRefundReasonPaymentRefunded");
}

void to_json(JsonValueScope &jv, const td_api::temporaryPasswordState &object) {
  auto jo = jv.enter_object();
  jo("@type", "temporaryPasswordState");
  jo("has_password", JsonBool{object.has_password_});
  jo("valid_for", object.valid_for_);
}

void to_json(JsonValueScope &jv, const td_api::text &object) {
  auto jo = jv.enter_object();
  jo("@type", "text");
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::timeZones &object) {
  auto jo = jv.enter_object();
  jo("@type", "timeZones");
  jo("time_zones", ToJson(object.time_zones_));
}

void to_json(JsonValueScope &jv, const td_api::Update &object) {
  td_api::downcast_call(const_cast<td_api::Update &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::updateAuthorizationState &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAuthorizationState");
  if (object.authorization_state_) {
    jo("authorization_state", ToJson(*object.authorization_state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewMessage");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageSendAcknowledged &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageSendAcknowledged");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateMessageSendSucceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageSendSucceeded");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("old_message_id", object.old_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateMessageSendFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageSendFailed");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("old_message_id", object.old_message_id_);
  if (object.error_) {
    jo("error", ToJson(*object.error_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageContent &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageContent");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.new_content_) {
    jo("new_content", ToJson(*object.new_content_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageEdited &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageEdited");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("edit_date", object.edit_date_);
  if (object.reply_markup_) {
    jo("reply_markup", ToJson(*object.reply_markup_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageIsPinned &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageIsPinned");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::updateMessageInteractionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageInteractionInfo");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.interaction_info_) {
    jo("interaction_info", ToJson(*object.interaction_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageContentOpened &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageContentOpened");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateMessageMentionRead &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageMentionRead");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("unread_mention_count", object.unread_mention_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateMessageUnreadReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageUnreadReactions");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("unread_reactions", ToJson(object.unread_reactions_));
  jo("unread_reaction_count", object.unread_reaction_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateMessageFactCheck &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageFactCheck");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.fact_check_) {
    jo("fact_check", ToJson(*object.fact_check_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageSuggestedPostInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageSuggestedPostInfo");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.suggested_post_info_) {
    jo("suggested_post_info", ToJson(*object.suggested_post_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageLiveLocationViewed &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageLiveLocationViewed");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateVideoPublished &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateVideoPublished");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateNewChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewChat");
  if (object.chat_) {
    jo("chat", ToJson(*object.chat_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatTitle &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatTitle");
  jo("chat_id", object.chat_id_);
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatPhoto");
  jo("chat_id", object.chat_id_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatAccentColors &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatAccentColors");
  jo("chat_id", object.chat_id_);
  jo("accent_color_id", object.accent_color_id_);
  jo("background_custom_emoji_id", ToJson(JsonInt64{object.background_custom_emoji_id_}));
  if (object.upgraded_gift_colors_) {
    jo("upgraded_gift_colors", ToJson(*object.upgraded_gift_colors_));
  }
  jo("profile_accent_color_id", object.profile_accent_color_id_);
  jo("profile_background_custom_emoji_id", ToJson(JsonInt64{object.profile_background_custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::updateChatPermissions &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatPermissions");
  jo("chat_id", object.chat_id_);
  if (object.permissions_) {
    jo("permissions", ToJson(*object.permissions_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatLastMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatLastMessage");
  jo("chat_id", object.chat_id_);
  if (object.last_message_) {
    jo("last_message", ToJson(*object.last_message_));
  }
  jo("positions", ToJson(object.positions_));
}

void to_json(JsonValueScope &jv, const td_api::updateChatPosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatPosition");
  jo("chat_id", object.chat_id_);
  if (object.position_) {
    jo("position", ToJson(*object.position_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatAddedToList &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatAddedToList");
  jo("chat_id", object.chat_id_);
  if (object.chat_list_) {
    jo("chat_list", ToJson(*object.chat_list_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatRemovedFromList &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatRemovedFromList");
  jo("chat_id", object.chat_id_);
  if (object.chat_list_) {
    jo("chat_list", ToJson(*object.chat_list_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatReadInbox &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatReadInbox");
  jo("chat_id", object.chat_id_);
  jo("last_read_inbox_message_id", object.last_read_inbox_message_id_);
  jo("unread_count", object.unread_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatReadOutbox &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatReadOutbox");
  jo("chat_id", object.chat_id_);
  jo("last_read_outbox_message_id", object.last_read_outbox_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatActionBar &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatActionBar");
  jo("chat_id", object.chat_id_);
  if (object.action_bar_) {
    jo("action_bar", ToJson(*object.action_bar_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatBusinessBotManageBar &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatBusinessBotManageBar");
  jo("chat_id", object.chat_id_);
  if (object.business_bot_manage_bar_) {
    jo("business_bot_manage_bar", ToJson(*object.business_bot_manage_bar_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatAvailableReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatAvailableReactions");
  jo("chat_id", object.chat_id_);
  if (object.available_reactions_) {
    jo("available_reactions", ToJson(*object.available_reactions_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatDraftMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatDraftMessage");
  jo("chat_id", object.chat_id_);
  if (object.draft_message_) {
    jo("draft_message", ToJson(*object.draft_message_));
  }
  jo("positions", ToJson(object.positions_));
}

void to_json(JsonValueScope &jv, const td_api::updateChatEmojiStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatEmojiStatus");
  jo("chat_id", object.chat_id_);
  if (object.emoji_status_) {
    jo("emoji_status", ToJson(*object.emoji_status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatMessageSender &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatMessageSender");
  jo("chat_id", object.chat_id_);
  if (object.message_sender_id_) {
    jo("message_sender_id", ToJson(*object.message_sender_id_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatMessageAutoDeleteTime &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatMessageAutoDeleteTime");
  jo("chat_id", object.chat_id_);
  jo("message_auto_delete_time", object.message_auto_delete_time_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatNotificationSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatNotificationSettings");
  jo("chat_id", object.chat_id_);
  if (object.notification_settings_) {
    jo("notification_settings", ToJson(*object.notification_settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatPendingJoinRequests &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatPendingJoinRequests");
  jo("chat_id", object.chat_id_);
  if (object.pending_join_requests_) {
    jo("pending_join_requests", ToJson(*object.pending_join_requests_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatReplyMarkup &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatReplyMarkup");
  jo("chat_id", object.chat_id_);
  jo("reply_markup_message_id", object.reply_markup_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatBackground");
  jo("chat_id", object.chat_id_);
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatTheme");
  jo("chat_id", object.chat_id_);
  if (object.theme_) {
    jo("theme", ToJson(*object.theme_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatUnreadMentionCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatUnreadMentionCount");
  jo("chat_id", object.chat_id_);
  jo("unread_mention_count", object.unread_mention_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatUnreadReactionCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatUnreadReactionCount");
  jo("chat_id", object.chat_id_);
  jo("unread_reaction_count", object.unread_reaction_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatVideoChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatVideoChat");
  jo("chat_id", object.chat_id_);
  if (object.video_chat_) {
    jo("video_chat", ToJson(*object.video_chat_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatDefaultDisableNotification &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatDefaultDisableNotification");
  jo("chat_id", object.chat_id_);
  jo("default_disable_notification", JsonBool{object.default_disable_notification_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatHasProtectedContent &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatHasProtectedContent");
  jo("chat_id", object.chat_id_);
  jo("has_protected_content", JsonBool{object.has_protected_content_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatIsTranslatable &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatIsTranslatable");
  jo("chat_id", object.chat_id_);
  jo("is_translatable", JsonBool{object.is_translatable_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatIsMarkedAsUnread &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatIsMarkedAsUnread");
  jo("chat_id", object.chat_id_);
  jo("is_marked_as_unread", JsonBool{object.is_marked_as_unread_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatViewAsTopics &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatViewAsTopics");
  jo("chat_id", object.chat_id_);
  jo("view_as_topics", JsonBool{object.view_as_topics_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatBlockList &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatBlockList");
  jo("chat_id", object.chat_id_);
  if (object.block_list_) {
    jo("block_list", ToJson(*object.block_list_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatHasScheduledMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatHasScheduledMessages");
  jo("chat_id", object.chat_id_);
  jo("has_scheduled_messages", JsonBool{object.has_scheduled_messages_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatFolders &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatFolders");
  jo("chat_folders", ToJson(object.chat_folders_));
  jo("main_chat_list_position", object.main_chat_list_position_);
  jo("are_tags_enabled", JsonBool{object.are_tags_enabled_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatOnlineMemberCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatOnlineMemberCount");
  jo("chat_id", object.chat_id_);
  jo("online_member_count", object.online_member_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateSavedMessagesTopic &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSavedMessagesTopic");
  if (object.topic_) {
    jo("topic", ToJson(*object.topic_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateSavedMessagesTopicCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSavedMessagesTopicCount");
  jo("topic_count", object.topic_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateDirectMessagesChatTopic &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateDirectMessagesChatTopic");
  if (object.topic_) {
    jo("topic", ToJson(*object.topic_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateTopicMessageCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateTopicMessageCount");
  jo("chat_id", object.chat_id_);
  if (object.topic_id_) {
    jo("topic_id", ToJson(*object.topic_id_));
  }
  jo("message_count", object.message_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateQuickReplyShortcut &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateQuickReplyShortcut");
  if (object.shortcut_) {
    jo("shortcut", ToJson(*object.shortcut_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateQuickReplyShortcutDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateQuickReplyShortcutDeleted");
  jo("shortcut_id", object.shortcut_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateQuickReplyShortcuts &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateQuickReplyShortcuts");
  jo("shortcut_ids", ToJson(object.shortcut_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateQuickReplyShortcutMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateQuickReplyShortcutMessages");
  jo("shortcut_id", object.shortcut_id_);
  jo("messages", ToJson(object.messages_));
}

void to_json(JsonValueScope &jv, const td_api::updateForumTopicInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateForumTopicInfo");
  if (object.info_) {
    jo("info", ToJson(*object.info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateForumTopic &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateForumTopic");
  jo("chat_id", object.chat_id_);
  jo("forum_topic_id", object.forum_topic_id_);
  jo("is_pinned", JsonBool{object.is_pinned_});
  jo("last_read_inbox_message_id", object.last_read_inbox_message_id_);
  jo("last_read_outbox_message_id", object.last_read_outbox_message_id_);
  jo("unread_mention_count", object.unread_mention_count_);
  jo("unread_reaction_count", object.unread_reaction_count_);
  if (object.notification_settings_) {
    jo("notification_settings", ToJson(*object.notification_settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateScopeNotificationSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateScopeNotificationSettings");
  if (object.scope_) {
    jo("scope", ToJson(*object.scope_));
  }
  if (object.notification_settings_) {
    jo("notification_settings", ToJson(*object.notification_settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateReactionNotificationSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateReactionNotificationSettings");
  if (object.notification_settings_) {
    jo("notification_settings", ToJson(*object.notification_settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNotification &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNotification");
  jo("notification_group_id", object.notification_group_id_);
  if (object.notification_) {
    jo("notification", ToJson(*object.notification_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNotificationGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNotificationGroup");
  jo("notification_group_id", object.notification_group_id_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("chat_id", object.chat_id_);
  jo("notification_settings_chat_id", object.notification_settings_chat_id_);
  jo("notification_sound_id", ToJson(JsonInt64{object.notification_sound_id_}));
  jo("total_count", object.total_count_);
  jo("added_notifications", ToJson(object.added_notifications_));
  jo("removed_notification_ids", ToJson(object.removed_notification_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateActiveNotifications &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateActiveNotifications");
  jo("groups", ToJson(object.groups_));
}

void to_json(JsonValueScope &jv, const td_api::updateHavePendingNotifications &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateHavePendingNotifications");
  jo("have_delayed_notifications", JsonBool{object.have_delayed_notifications_});
  jo("have_unreceived_notifications", JsonBool{object.have_unreceived_notifications_});
}

void to_json(JsonValueScope &jv, const td_api::updateDeleteMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateDeleteMessages");
  jo("chat_id", object.chat_id_);
  jo("message_ids", ToJson(object.message_ids_));
  jo("is_permanent", JsonBool{object.is_permanent_});
  jo("from_cache", JsonBool{object.from_cache_});
}

void to_json(JsonValueScope &jv, const td_api::updateChatAction &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatAction");
  jo("chat_id", object.chat_id_);
  if (object.topic_id_) {
    jo("topic_id", ToJson(*object.topic_id_));
  }
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.action_) {
    jo("action", ToJson(*object.action_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updatePendingTextMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "updatePendingTextMessage");
  jo("chat_id", object.chat_id_);
  jo("forum_topic_id", object.forum_topic_id_);
  jo("draft_id", ToJson(JsonInt64{object.draft_id_}));
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateUserStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUserStatus");
  jo("user_id", object.user_id_);
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUser");
  if (object.user_) {
    jo("user", ToJson(*object.user_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateBasicGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateBasicGroup");
  if (object.basic_group_) {
    jo("basic_group", ToJson(*object.basic_group_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateSupergroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSupergroup");
  if (object.supergroup_) {
    jo("supergroup", ToJson(*object.supergroup_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateSecretChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSecretChat");
  if (object.secret_chat_) {
    jo("secret_chat", ToJson(*object.secret_chat_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateUserFullInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUserFullInfo");
  jo("user_id", object.user_id_);
  if (object.user_full_info_) {
    jo("user_full_info", ToJson(*object.user_full_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateBasicGroupFullInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateBasicGroupFullInfo");
  jo("basic_group_id", object.basic_group_id_);
  if (object.basic_group_full_info_) {
    jo("basic_group_full_info", ToJson(*object.basic_group_full_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateSupergroupFullInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSupergroupFullInfo");
  jo("supergroup_id", object.supergroup_id_);
  if (object.supergroup_full_info_) {
    jo("supergroup_full_info", ToJson(*object.supergroup_full_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateServiceNotification &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateServiceNotification");
  jo("type", object.type_);
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFile");
  if (object.file_) {
    jo("file", ToJson(*object.file_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateFileGenerationStart &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFileGenerationStart");
  jo("generation_id", ToJson(JsonInt64{object.generation_id_}));
  jo("original_path", object.original_path_);
  jo("destination_path", object.destination_path_);
  jo("conversion", object.conversion_);
}

void to_json(JsonValueScope &jv, const td_api::updateFileGenerationStop &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFileGenerationStop");
  jo("generation_id", ToJson(JsonInt64{object.generation_id_}));
}

void to_json(JsonValueScope &jv, const td_api::updateFileDownloads &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFileDownloads");
  jo("total_size", object.total_size_);
  jo("total_count", object.total_count_);
  jo("downloaded_size", object.downloaded_size_);
}

void to_json(JsonValueScope &jv, const td_api::updateFileAddedToDownloads &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFileAddedToDownloads");
  if (object.file_download_) {
    jo("file_download", ToJson(*object.file_download_));
  }
  if (object.counts_) {
    jo("counts", ToJson(*object.counts_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateFileDownload &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFileDownload");
  jo("file_id", object.file_id_);
  jo("complete_date", object.complete_date_);
  jo("is_paused", JsonBool{object.is_paused_});
  if (object.counts_) {
    jo("counts", ToJson(*object.counts_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateFileRemovedFromDownloads &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFileRemovedFromDownloads");
  jo("file_id", object.file_id_);
  if (object.counts_) {
    jo("counts", ToJson(*object.counts_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateApplicationVerificationRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateApplicationVerificationRequired");
  jo("verification_id", object.verification_id_);
  jo("nonce", object.nonce_);
  jo("cloud_project_number", ToJson(JsonInt64{object.cloud_project_number_}));
}

void to_json(JsonValueScope &jv, const td_api::updateApplicationRecaptchaVerificationRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateApplicationRecaptchaVerificationRequired");
  jo("verification_id", object.verification_id_);
  jo("action", object.action_);
  jo("recaptcha_key_id", object.recaptcha_key_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateCall");
  if (object.call_) {
    jo("call", ToJson(*object.call_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateGroupCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateGroupCall");
  if (object.group_call_) {
    jo("group_call", ToJson(*object.group_call_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateGroupCallParticipant &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateGroupCallParticipant");
  jo("group_call_id", object.group_call_id_);
  if (object.participant_) {
    jo("participant", ToJson(*object.participant_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateGroupCallParticipants &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateGroupCallParticipants");
  jo("group_call_id", object.group_call_id_);
  jo("participant_user_ids", ToJson(JsonVectorInt64{object.participant_user_ids_}));
}

void to_json(JsonValueScope &jv, const td_api::updateGroupCallVerificationState &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateGroupCallVerificationState");
  jo("group_call_id", object.group_call_id_);
  jo("generation", object.generation_);
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::updateGroupCallNewMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateGroupCallNewMessage");
  jo("group_call_id", object.group_call_id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewCallSignalingData &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewCallSignalingData");
  jo("call_id", object.call_id_);
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::updateUserPrivacySettingRules &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUserPrivacySettingRules");
  if (object.setting_) {
    jo("setting", ToJson(*object.setting_));
  }
  if (object.rules_) {
    jo("rules", ToJson(*object.rules_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateUnreadMessageCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUnreadMessageCount");
  if (object.chat_list_) {
    jo("chat_list", ToJson(*object.chat_list_));
  }
  jo("unread_count", object.unread_count_);
  jo("unread_unmuted_count", object.unread_unmuted_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateUnreadChatCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUnreadChatCount");
  if (object.chat_list_) {
    jo("chat_list", ToJson(*object.chat_list_));
  }
  jo("total_count", object.total_count_);
  jo("unread_count", object.unread_count_);
  jo("unread_unmuted_count", object.unread_unmuted_count_);
  jo("marked_as_unread_count", object.marked_as_unread_count_);
  jo("marked_as_unread_unmuted_count", object.marked_as_unread_unmuted_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStory");
  if (object.story_) {
    jo("story", ToJson(*object.story_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateStoryDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStoryDeleted");
  jo("story_poster_chat_id", object.story_poster_chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateStoryPostSucceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStoryPostSucceeded");
  if (object.story_) {
    jo("story", ToJson(*object.story_));
  }
  jo("old_story_id", object.old_story_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateStoryPostFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStoryPostFailed");
  if (object.story_) {
    jo("story", ToJson(*object.story_));
  }
  if (object.error_) {
    jo("error", ToJson(*object.error_));
  }
  if (object.error_type_) {
    jo("error_type", ToJson(*object.error_type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatActiveStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatActiveStories");
  if (object.active_stories_) {
    jo("active_stories", ToJson(*object.active_stories_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateStoryListChatCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStoryListChatCount");
  if (object.story_list_) {
    jo("story_list", ToJson(*object.story_list_));
  }
  jo("chat_count", object.chat_count_);
}

void to_json(JsonValueScope &jv, const td_api::updateStoryStealthMode &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStoryStealthMode");
  jo("active_until_date", object.active_until_date_);
  jo("cooldown_until_date", object.cooldown_until_date_);
}

void to_json(JsonValueScope &jv, const td_api::updateOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateOption");
  jo("name", object.name_);
  if (object.value_) {
    jo("value", ToJson(*object.value_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateStickerSet &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStickerSet");
  if (object.sticker_set_) {
    jo("sticker_set", ToJson(*object.sticker_set_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateInstalledStickerSets &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateInstalledStickerSets");
  if (object.sticker_type_) {
    jo("sticker_type", ToJson(*object.sticker_type_));
  }
  jo("sticker_set_ids", ToJson(JsonVectorInt64{object.sticker_set_ids_}));
}

void to_json(JsonValueScope &jv, const td_api::updateTrendingStickerSets &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateTrendingStickerSets");
  if (object.sticker_type_) {
    jo("sticker_type", ToJson(*object.sticker_type_));
  }
  if (object.sticker_sets_) {
    jo("sticker_sets", ToJson(*object.sticker_sets_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateRecentStickers &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateRecentStickers");
  jo("is_attached", JsonBool{object.is_attached_});
  jo("sticker_ids", ToJson(object.sticker_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateFavoriteStickers &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFavoriteStickers");
  jo("sticker_ids", ToJson(object.sticker_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateSavedAnimations &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSavedAnimations");
  jo("animation_ids", ToJson(object.animation_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateSavedNotificationSounds &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSavedNotificationSounds");
  jo("notification_sound_ids", ToJson(JsonVectorInt64{object.notification_sound_ids_}));
}

void to_json(JsonValueScope &jv, const td_api::updateDefaultBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateDefaultBackground");
  jo("for_dark_theme", JsonBool{object.for_dark_theme_});
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateEmojiChatThemes &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateEmojiChatThemes");
  jo("chat_themes", ToJson(object.chat_themes_));
}

void to_json(JsonValueScope &jv, const td_api::updateAccentColors &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAccentColors");
  jo("colors", ToJson(object.colors_));
  jo("available_accent_color_ids", ToJson(object.available_accent_color_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateProfileAccentColors &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateProfileAccentColors");
  jo("colors", ToJson(object.colors_));
  jo("available_accent_color_ids", ToJson(object.available_accent_color_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateLanguagePackStrings &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateLanguagePackStrings");
  jo("localization_target", object.localization_target_);
  jo("language_pack_id", object.language_pack_id_);
  jo("strings", ToJson(object.strings_));
}

void to_json(JsonValueScope &jv, const td_api::updateConnectionState &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateConnectionState");
  if (object.state_) {
    jo("state", ToJson(*object.state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateFreezeState &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateFreezeState");
  jo("is_frozen", JsonBool{object.is_frozen_});
  jo("freezing_date", object.freezing_date_);
  jo("deletion_date", object.deletion_date_);
  jo("appeal_link", object.appeal_link_);
}

void to_json(JsonValueScope &jv, const td_api::updateAgeVerificationParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAgeVerificationParameters");
  if (object.parameters_) {
    jo("parameters", ToJson(*object.parameters_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateTermsOfService &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateTermsOfService");
  jo("terms_of_service_id", object.terms_of_service_id_);
  if (object.terms_of_service_) {
    jo("terms_of_service", ToJson(*object.terms_of_service_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateUnconfirmedSession &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUnconfirmedSession");
  if (object.session_) {
    jo("session", ToJson(*object.session_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateAttachmentMenuBots &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAttachmentMenuBots");
  jo("bots", ToJson(object.bots_));
}

void to_json(JsonValueScope &jv, const td_api::updateWebAppMessageSent &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateWebAppMessageSent");
  jo("web_app_launch_id", ToJson(JsonInt64{object.web_app_launch_id_}));
}

void to_json(JsonValueScope &jv, const td_api::updateActiveEmojiReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateActiveEmojiReactions");
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::updateAvailableMessageEffects &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAvailableMessageEffects");
  jo("reaction_effect_ids", ToJson(JsonVectorInt64{object.reaction_effect_ids_}));
  jo("sticker_effect_ids", ToJson(JsonVectorInt64{object.sticker_effect_ids_}));
}

void to_json(JsonValueScope &jv, const td_api::updateDefaultReactionType &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateDefaultReactionType");
  if (object.reaction_type_) {
    jo("reaction_type", ToJson(*object.reaction_type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateDefaultPaidReactionType &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateDefaultPaidReactionType");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateSavedMessagesTags &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSavedMessagesTags");
  jo("saved_messages_topic_id", object.saved_messages_topic_id_);
  if (object.tags_) {
    jo("tags", ToJson(*object.tags_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateActiveLiveLocationMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateActiveLiveLocationMessages");
  jo("messages", ToJson(object.messages_));
}

void to_json(JsonValueScope &jv, const td_api::updateOwnedStarCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateOwnedStarCount");
  if (object.star_amount_) {
    jo("star_amount", ToJson(*object.star_amount_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateOwnedTonCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateOwnedTonCount");
  jo("ton_amount", object.ton_amount_);
}

void to_json(JsonValueScope &jv, const td_api::updateChatRevenueAmount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatRevenueAmount");
  jo("chat_id", object.chat_id_);
  if (object.revenue_amount_) {
    jo("revenue_amount", ToJson(*object.revenue_amount_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateStarRevenueStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStarRevenueStatus");
  if (object.owner_id_) {
    jo("owner_id", ToJson(*object.owner_id_));
  }
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateTonRevenueStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateTonRevenueStatus");
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateSpeechRecognitionTrial &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSpeechRecognitionTrial");
  jo("max_media_duration", object.max_media_duration_);
  jo("weekly_count", object.weekly_count_);
  jo("left_count", object.left_count_);
  jo("next_reset_date", object.next_reset_date_);
}

void to_json(JsonValueScope &jv, const td_api::updateDiceEmojis &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateDiceEmojis");
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::updateAnimatedEmojiMessageClicked &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAnimatedEmojiMessageClicked");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateAnimationSearchParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAnimationSearchParameters");
  jo("provider", object.provider_);
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::updateSuggestedActions &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSuggestedActions");
  jo("added_actions", ToJson(object.added_actions_));
  jo("removed_actions", ToJson(object.removed_actions_));
}

void to_json(JsonValueScope &jv, const td_api::updateSpeedLimitNotification &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSpeedLimitNotification");
  jo("is_upload", JsonBool{object.is_upload_});
}

void to_json(JsonValueScope &jv, const td_api::updateContactCloseBirthdays &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateContactCloseBirthdays");
  jo("close_birthday_users", ToJson(object.close_birthday_users_));
}

void to_json(JsonValueScope &jv, const td_api::updateAutosaveSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAutosaveSettings");
  if (object.scope_) {
    jo("scope", ToJson(*object.scope_));
  }
  if (object.settings_) {
    jo("settings", ToJson(*object.settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateBusinessConnection &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateBusinessConnection");
  if (object.connection_) {
    jo("connection", ToJson(*object.connection_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewBusinessMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewBusinessMessage");
  jo("connection_id", object.connection_id_);
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateBusinessMessageEdited &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateBusinessMessageEdited");
  jo("connection_id", object.connection_id_);
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateBusinessMessagesDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateBusinessMessagesDeleted");
  jo("connection_id", object.connection_id_);
  jo("chat_id", object.chat_id_);
  jo("message_ids", ToJson(object.message_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateNewInlineQuery &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewInlineQuery");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("sender_user_id", object.sender_user_id_);
  if (object.user_location_) {
    jo("user_location", ToJson(*object.user_location_));
  }
  if (object.chat_type_) {
    jo("chat_type", ToJson(*object.chat_type_));
  }
  jo("query", object.query_);
  jo("offset", object.offset_);
}

void to_json(JsonValueScope &jv, const td_api::updateNewChosenInlineResult &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewChosenInlineResult");
  jo("sender_user_id", object.sender_user_id_);
  if (object.user_location_) {
    jo("user_location", ToJson(*object.user_location_));
  }
  jo("query", object.query_);
  jo("result_id", object.result_id_);
  jo("inline_message_id", object.inline_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateNewCallbackQuery &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewCallbackQuery");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("sender_user_id", object.sender_user_id_);
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("chat_instance", ToJson(JsonInt64{object.chat_instance_}));
  if (object.payload_) {
    jo("payload", ToJson(*object.payload_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewInlineCallbackQuery &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewInlineCallbackQuery");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("sender_user_id", object.sender_user_id_);
  jo("inline_message_id", object.inline_message_id_);
  jo("chat_instance", ToJson(JsonInt64{object.chat_instance_}));
  if (object.payload_) {
    jo("payload", ToJson(*object.payload_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewBusinessCallbackQuery &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewBusinessCallbackQuery");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("sender_user_id", object.sender_user_id_);
  jo("connection_id", object.connection_id_);
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("chat_instance", ToJson(JsonInt64{object.chat_instance_}));
  if (object.payload_) {
    jo("payload", ToJson(*object.payload_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewShippingQuery &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewShippingQuery");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("sender_user_id", object.sender_user_id_);
  jo("invoice_payload", object.invoice_payload_);
  if (object.shipping_address_) {
    jo("shipping_address", ToJson(*object.shipping_address_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewPreCheckoutQuery &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewPreCheckoutQuery");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("sender_user_id", object.sender_user_id_);
  jo("currency", object.currency_);
  jo("total_amount", object.total_amount_);
  jo("invoice_payload", base64_encode(object.invoice_payload_));
  jo("shipping_option_id", object.shipping_option_id_);
  if (object.order_info_) {
    jo("order_info", ToJson(*object.order_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewCustomEvent &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewCustomEvent");
  jo("event", object.event_);
}

void to_json(JsonValueScope &jv, const td_api::updateNewCustomQuery &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewCustomQuery");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("data", object.data_);
  jo("timeout", object.timeout_);
}

void to_json(JsonValueScope &jv, const td_api::updatePoll &object) {
  auto jo = jv.enter_object();
  jo("@type", "updatePoll");
  if (object.poll_) {
    jo("poll", ToJson(*object.poll_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updatePollAnswer &object) {
  auto jo = jv.enter_object();
  jo("@type", "updatePollAnswer");
  jo("poll_id", ToJson(JsonInt64{object.poll_id_}));
  if (object.voter_id_) {
    jo("voter_id", ToJson(*object.voter_id_));
  }
  jo("option_ids", ToJson(object.option_ids_));
}

void to_json(JsonValueScope &jv, const td_api::updateChatMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatMember");
  jo("chat_id", object.chat_id_);
  jo("actor_user_id", object.actor_user_id_);
  jo("date", object.date_);
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
  jo("via_join_request", JsonBool{object.via_join_request_});
  jo("via_chat_folder_invite_link", JsonBool{object.via_chat_folder_invite_link_});
  if (object.old_chat_member_) {
    jo("old_chat_member", ToJson(*object.old_chat_member_));
  }
  if (object.new_chat_member_) {
    jo("new_chat_member", ToJson(*object.new_chat_member_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateNewChatJoinRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateNewChatJoinRequest");
  jo("chat_id", object.chat_id_);
  if (object.request_) {
    jo("request", ToJson(*object.request_));
  }
  jo("user_chat_id", object.user_chat_id_);
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateChatBoost &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatBoost");
  jo("chat_id", object.chat_id_);
  if (object.boost_) {
    jo("boost", ToJson(*object.boost_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateMessageReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageReaction");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.actor_id_) {
    jo("actor_id", ToJson(*object.actor_id_));
  }
  jo("date", object.date_);
  jo("old_reaction_types", ToJson(object.old_reaction_types_));
  jo("new_reaction_types", ToJson(object.new_reaction_types_));
}

void to_json(JsonValueScope &jv, const td_api::updateMessageReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageReactions");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("date", object.date_);
  jo("reactions", ToJson(object.reactions_));
}

void to_json(JsonValueScope &jv, const td_api::updatePaidMediaPurchased &object) {
  auto jo = jv.enter_object();
  jo("@type", "updatePaidMediaPurchased");
  jo("user_id", object.user_id_);
  jo("payload", object.payload_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftModelCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftModelCount");
  if (object.model_) {
    jo("model", ToJson(*object.model_));
  }
  jo("total_count", object.total_count_);
}

void to_json(JsonValueScope &jv, const td_api::UserPrivacySetting &object) {
  td_api::downcast_call(const_cast<td_api::UserPrivacySetting &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingShowStatus");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowProfilePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingShowProfilePhoto");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowLinkInForwardedMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingShowLinkInForwardedMessages");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingShowPhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowBio &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingShowBio");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingShowBirthdate &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingShowBirthdate");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowChatInvites &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingAllowChatInvites");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowCalls &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingAllowCalls");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowPeerToPeerCalls &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingAllowPeerToPeerCalls");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowFindingByPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingAllowFindingByPhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAutosaveGifts &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingAutosaveGifts");
}

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingAllowUnpaidMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingAllowUnpaidMessages");
}

void to_json(JsonValueScope &jv, const td_api::validatedOrderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "validatedOrderInfo");
  jo("order_info_id", object.order_info_id_);
  jo("shipping_options", ToJson(object.shipping_options_));
}

void to_json(JsonValueScope &jv, const td_api::videoMessageAdvertisements &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoMessageAdvertisements");
  jo("advertisements", ToJson(object.advertisements_));
  jo("start_delay", object.start_delay_);
  jo("between_delay", object.between_delay_);
}

}  // namespace td_api
}  // namespace td
