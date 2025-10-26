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
Result<int32> tl_constructor_from_string(td_api::AffiliateProgramSortOrder *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"affiliateProgramSortOrderProfitability", -1963282585},
    {"affiliateProgramSortOrderCreationDate", -1558628083},
    {"affiliateProgramSortOrderRevenue", 1923269304}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::CallProblem *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"callProblemEcho", 801116548},
    {"callProblemNoise", 1053065359},
    {"callProblemInterruptions", 1119493218},
    {"callProblemDistortedSpeech", 379960581},
    {"callProblemSilentLocal", 253652790},
    {"callProblemSilentRemote", 573634714},
    {"callProblemDropped", -1207311487},
    {"callProblemDistortedVideo", 385245706},
    {"callProblemPixelatedVideo", 2115315411}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::DeviceToken *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"deviceTokenFirebaseCloudMessaging", -797881849},
    {"deviceTokenApplePush", 387541955},
    {"deviceTokenApplePushVoIP", 804275689},
    {"deviceTokenWindowsPush", -1410514289},
    {"deviceTokenMicrosoftPush", 1224269900},
    {"deviceTokenMicrosoftPushVoIP", -785603759},
    {"deviceTokenWebPush", -1694507273},
    {"deviceTokenSimplePush", 49584736},
    {"deviceTokenUbuntuPush", 1782320422},
    {"deviceTokenBlackBerryPush", 1559167234},
    {"deviceTokenTizenPush", -1359947213},
    {"deviceTokenHuaweiPush", 1989103142}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::GroupCallVideoQuality *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"groupCallVideoQualityThumbnail", -379186304},
    {"groupCallVideoQualityMedium", 394968234},
    {"groupCallVideoQualityFull", -2125916617}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputInlineQueryResult *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputInlineQueryResultAnimation", -1489808874},
    {"inputInlineQueryResultArticle", 1983218620},
    {"inputInlineQueryResultAudio", 1260139988},
    {"inputInlineQueryResultContact", 1846064594},
    {"inputInlineQueryResultDocument", 578801869},
    {"inputInlineQueryResultGame", 966074327},
    {"inputInlineQueryResultLocation", -1887650218},
    {"inputInlineQueryResultPhoto", -1123338721},
    {"inputInlineQueryResultSticker", 274007129},
    {"inputInlineQueryResultVenue", 541704509},
    {"inputInlineQueryResultVideo", 1724073191},
    {"inputInlineQueryResultVoiceNote", -1790072503}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InternalLinkType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"internalLinkTypeActiveSessions", 1886108589},
    {"internalLinkTypeAttachmentMenuBot", 1682719269},
    {"internalLinkTypeAuthenticationCode", -209235982},
    {"internalLinkTypeBackground", 185411848},
    {"internalLinkTypeBotAddToChannel", 1401602752},
    {"internalLinkTypeBotStart", 1066950637},
    {"internalLinkTypeBotStartInGroup", -905081650},
    {"internalLinkTypeBusinessChat", -1606751785},
    {"internalLinkTypeBuyStars", -1454587065},
    {"internalLinkTypeChangePhoneNumber", -265856255},
    {"internalLinkTypeChatAffiliateProgram", 632049700},
    {"internalLinkTypeChatBoost", -716571328},
    {"internalLinkTypeChatFolderInvite", -1984804546},
    {"internalLinkTypeChatFolderSettings", -1073805988},
    {"internalLinkTypeChatInvite", 428621017},
    {"internalLinkTypeDefaultMessageAutoDeleteTimerSettings", 732625201},
    {"internalLinkTypeDirectMessagesChat", 1795016752},
    {"internalLinkTypeEditProfileSettings", -1022472090},
    {"internalLinkTypeGame", -260788787},
    {"internalLinkTypeGiftCollection", -812480347},
    {"internalLinkTypeGroupCall", 1953084438},
    {"internalLinkTypeInstantView", 1776607039},
    {"internalLinkTypeInvoice", -213094996},
    {"internalLinkTypeLanguagePack", -1450766996},
    {"internalLinkTypeLanguageSettings", -1340479770},
    {"internalLinkTypeMainWebApp", 1574925033},
    {"internalLinkTypeMessage", 978541650},
    {"internalLinkTypeMessageDraft", 661633749},
    {"internalLinkTypeMyStars", 1613887070},
    {"internalLinkTypeMyToncoins", -1861488996},
    {"internalLinkTypePassportDataRequest", -988819839},
    {"internalLinkTypePhoneNumberConfirmation", 1757375254},
    {"internalLinkTypePremiumFeatures", 1216892745},
    {"internalLinkTypePremiumGift", 1523936577},
    {"internalLinkTypePremiumGiftCode", -564356974},
    {"internalLinkTypePrivacyAndSecuritySettings", -1386255665},
    {"internalLinkTypeProxy", -1313788694},
    {"internalLinkTypePublicChat", 1769614592},
    {"internalLinkTypeQrCodeAuthentication", -1089332956},
    {"internalLinkTypeRestorePurchases", 606090371},
    {"internalLinkTypeSettings", 393561524},
    {"internalLinkTypeStickerSet", -1589227614},
    {"internalLinkTypeStory", 1852042869},
    {"internalLinkTypeStoryAlbum", -332692184},
    {"internalLinkTypeTheme", -200935417},
    {"internalLinkTypeThemeSettings", -1051903722},
    {"internalLinkTypeUnknownDeepLink", 625596379},
    {"internalLinkTypeUnsupportedProxy", -566649079},
    {"internalLinkTypeUpgradedGift", -708405605},
    {"internalLinkTypeUserPhoneNumber", 273398536},
    {"internalLinkTypeUserToken", -1462248615},
    {"internalLinkTypeVideoChat", -2020149068},
    {"internalLinkTypeWebApp", 2062112045}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::MessageSource *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"messageSourceChatHistory", -1090386116},
    {"messageSourceMessageThreadHistory", 290427142},
    {"messageSourceForumTopicHistory", -1518064457},
    {"messageSourceDirectMessagesChatTopicHistory", 1869256503},
    {"messageSourceHistoryPreview", 1024254993},
    {"messageSourceChatList", -2047406102},
    {"messageSourceSearch", 1921333105},
    {"messageSourceChatEventLog", -1028777540},
    {"messageSourceNotification", -1046406163},
    {"messageSourceScreenshot", 469982474},
    {"messageSourceOther", 901818114}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::PollType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"pollTypeRegular", 641265698},
    {"pollTypeQuiz", 657013913}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::ReactionType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"reactionTypeEmoji", -1942084920},
    {"reactionTypeCustomEmoji", -989117709},
    {"reactionTypePaid", 436294381}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::StoreTransaction *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"storeTransactionAppStore", 1625562441},
    {"storeTransactionGooglePlay", 1094018617}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::TextParseMode *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"textParseModeMarkdown", 360073407},
    {"textParseModeHTML", 1660208627}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::address &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  TRY_STATUS(from_json(to.state_, from.extract_field("state")));
  TRY_STATUS(from_json(to.city_, from.extract_field("city")));
  TRY_STATUS(from_json(to.street_line1_, from.extract_field("street_line1")));
  TRY_STATUS(from_json(to.street_line2_, from.extract_field("street_line2")));
  TRY_STATUS(from_json(to.postal_code_, from.extract_field("postal_code")));
  return Status::OK();
}

Status from_json(td_api::autoDownloadSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_auto_download_enabled_, from.extract_field("is_auto_download_enabled")));
  TRY_STATUS(from_json(to.max_photo_file_size_, from.extract_field("max_photo_file_size")));
  TRY_STATUS(from_json(to.max_video_file_size_, from.extract_field("max_video_file_size")));
  TRY_STATUS(from_json(to.max_other_file_size_, from.extract_field("max_other_file_size")));
  TRY_STATUS(from_json(to.video_upload_bitrate_, from.extract_field("video_upload_bitrate")));
  TRY_STATUS(from_json(to.preload_large_videos_, from.extract_field("preload_large_videos")));
  TRY_STATUS(from_json(to.preload_next_audio_, from.extract_field("preload_next_audio")));
  TRY_STATUS(from_json(to.preload_stories_, from.extract_field("preload_stories")));
  TRY_STATUS(from_json(to.use_less_data_for_calls_, from.extract_field("use_less_data_for_calls")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypePattern &to, JsonObject &from) {
  TRY_STATUS(from_json(to.fill_, from.extract_field("fill")));
  TRY_STATUS(from_json(to.intensity_, from.extract_field("intensity")));
  TRY_STATUS(from_json(to.is_inverted_, from.extract_field("is_inverted")));
  TRY_STATUS(from_json(to.is_moving_, from.extract_field("is_moving")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeAllGroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.recipients_, from.extract_field("recipients")));
  TRY_STATUS(from_json(to.schedule_, from.extract_field("schedule")));
  TRY_STATUS(from_json(to.offline_only_, from.extract_field("offline_only")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureStartPage &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessRecipients &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  TRY_STATUS(from_json(to.excluded_chat_ids_, from.extract_field("excluded_chat_ids")));
  TRY_STATUS(from_json(to.select_existing_chats_, from.extract_field("select_existing_chats")));
  TRY_STATUS(from_json(to.select_new_chats_, from.extract_field("select_new_chats")));
  TRY_STATUS(from_json(to.select_contacts_, from.extract_field("select_contacts")));
  TRY_STATUS(from_json(to.select_non_contacts_, from.extract_field("select_non_contacts")));
  TRY_STATUS(from_json(to.exclude_selected_, from.extract_field("exclude_selected")));
  return Status::OK();
}

Status from_json(td_api::callProblemPixelatedVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionWatchingAnimations &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::chatInviteLinkMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.joined_chat_date_, from.extract_field("joined_chat_date")));
  TRY_STATUS(from_json(to.via_chat_folder_invite_link_, from.extract_field("via_chat_folder_invite_link")));
  TRY_STATUS(from_json(to.approver_user_id_, from.extract_field("approver_user_id")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusRestricted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_member_, from.extract_field("is_member")));
  TRY_STATUS(from_json(to.restricted_until_date_, from.extract_field("restricted_until_date")));
  TRY_STATUS(from_json(to.permissions_, from.extract_field("permissions")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::date &to, JsonObject &from) {
  TRY_STATUS(from_json(to.day_, from.extract_field("day")));
  TRY_STATUS(from_json(to.month_, from.extract_field("month")));
  TRY_STATUS(from_json(to.year_, from.extract_field("year")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenUbuntuPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeRegularStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeAudio &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSelfDestructingPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVideoStory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::giftForResaleOrderNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::groupCallVideoQualityMedium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallbackGame &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inputBackgroundRemote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_id_, from.extract_field("background_id")));
  return Status::OK();
}

Status from_json(td_api::inputChatThemeGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::inputFileLocal &to, JsonObject &from) {
  TRY_STATUS(from_json(to.path_, from.extract_field("path")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.document_url_, from.extract_field("document_url")));
  TRY_STATUS(from_json(to.mime_type_, from.extract_field("mime_type")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, from.extract_field("thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, from.extract_field("thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInvoiceName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.video_, from.extract_field("video")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.cover_, from.extract_field("cover")));
  TRY_STATUS(from_json(to.start_timestamp_, from.extract_field("start_timestamp")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, from.extract_field("added_sticker_file_ids")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  TRY_STATUS(from_json(to.supports_streaming_, from.extract_field("supports_streaming")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
  TRY_STATUS(from_json(to.self_destruct_type_, from.extract_field("self_destruct_type")));
  TRY_STATUS(from_json(to.has_spoiler_, from.extract_field("has_spoiler")));
  return Status::OK();
}

Status from_json(td_api::inputMessagePoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.question_, from.extract_field("question")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.is_anonymous_, from.extract_field("is_anonymous")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.open_period_, from.extract_field("open_period")));
  TRY_STATUS(from_json(to.close_date_, from.extract_field("close_date")));
  TRY_STATUS(from_json(to.is_closed_, from.extract_field("is_closed")));
  return Status::OK();
}

Status from_json(td_api::inputPaidMediaTypeVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.cover_, from.extract_field("cover")));
  TRY_STATUS(from_json(to.start_timestamp_, from.extract_field("start_timestamp")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.supports_streaming_, from.extract_field("supports_streaming")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementRentalAgreement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rental_agreement_, from.extract_field("rental_agreement")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceReverseSide &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::inputStoryContentPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, from.extract_field("added_sticker_file_ids")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotAddToChannel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.administrator_rights_, from.extract_field("administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatFolderSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeInvoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invoice_name_, from.extract_field("invoice_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePhoneNumberConfirmation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hash_, from.extract_field("hash")));
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUserPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.draft_text_, from.extract_field("draft_text")));
  TRY_STATUS(from_json(to.open_profile_, from.extract_field("open_profile")));
  return Status::OK();
}

Status from_json(td_api::jsonValueString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.chat_is_channel_, from.extract_field("chat_is_channel")));
  TRY_STATUS(from_json(to.restrict_chat_is_forum_, from.extract_field("restrict_chat_is_forum")));
  TRY_STATUS(from_json(to.chat_is_forum_, from.extract_field("chat_is_forum")));
  TRY_STATUS(from_json(to.restrict_chat_has_username_, from.extract_field("restrict_chat_has_username")));
  TRY_STATUS(from_json(to.chat_has_username_, from.extract_field("chat_has_username")));
  TRY_STATUS(from_json(to.chat_is_created_, from.extract_field("chat_is_created")));
  TRY_STATUS(from_json(to.user_administrator_rights_, from.extract_field("user_administrator_rights")));
  TRY_STATUS(from_json(to.bot_administrator_rights_, from.extract_field("bot_administrator_rights")));
  TRY_STATUS(from_json(to.bot_is_member_, from.extract_field("bot_is_member")));
  TRY_STATUS(from_json(to.request_title_, from.extract_field("request_title")));
  TRY_STATUS(from_json(to.request_username_, from.extract_field("request_username")));
  TRY_STATUS(from_json(to.request_photo_, from.extract_field("request_photo")));
  return Status::OK();
}

Status from_json(td_api::location &to, JsonObject &from) {
  TRY_STATUS(from_json(to.latitude_, from.extract_field("latitude")));
  TRY_STATUS(from_json(to.longitude_, from.extract_field("longitude")));
  TRY_STATUS(from_json(to.horizontal_accuracy_, from.extract_field("horizontal_accuracy")));
  return Status::OK();
}

Status from_json(td_api::maskPosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.point_, from.extract_field("point")));
  TRY_STATUS(from_json(to.x_shift_, from.extract_field("x_shift")));
  TRY_STATUS(from_json(to.y_shift_, from.extract_field("y_shift")));
  TRY_STATUS(from_json(to.scale_, from.extract_field("scale")));
  return Status::OK();
}

Status from_json(td_api::messageSenderUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::messageSourceChatEventLog &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkStatisticsEntryCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.network_type_, from.extract_field("network_type")));
  TRY_STATUS(from_json(to.sent_bytes_, from.extract_field("sent_bytes")));
  TRY_STATUS(from_json(to.received_bytes_, from.extract_field("received_bytes")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  return Status::OK();
}

Status from_json(td_api::notificationSettingsScopeChannelChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypePersonalDetails &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypePassportRegistration &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::pollTypeRegular &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_multiple_answers_, from.extract_field("allow_multiple_answers")));
  return Status::OK();
}

Status from_json(td_api::premiumFeatureCustomEmoji &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureChatBoost &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeSupergroupCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeCaptionLength &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeSimilarChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeaturePermanentViewsHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::profileTabLinks &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_reaction_source_, from.extract_field("message_reaction_source")));
  TRY_STATUS(from_json(to.story_reaction_source_, from.extract_field("story_reaction_source")));
  TRY_STATUS(from_json(to.sound_id_, from.extract_field("sound_id")));
  TRY_STATUS(from_json(to.show_preview_, from.extract_field("show_preview")));
  return Status::OK();
}

Status from_json(td_api::replyMarkupForceReply &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_personal_, from.extract_field("is_personal")));
  TRY_STATUS(from_json(to.input_field_placeholder_, from.extract_field("input_field_placeholder")));
  return Status::OK();
}

Status from_json(td_api::reportReasonFake &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesChatTypeFilterGroup &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterPhotoAndVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterPinned &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposePremiumSubscription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_restore_, from.extract_field("is_restore")));
  TRY_STATUS(from_json(to.is_upgrade_, from.extract_field("is_upgrade")));
  return Status::OK();
}

Status from_json(td_api::storyAreaPosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_percentage_, from.extract_field("x_percentage")));
  TRY_STATUS(from_json(to.y_percentage_, from.extract_field("y_percentage")));
  TRY_STATUS(from_json(to.width_percentage_, from.extract_field("width_percentage")));
  TRY_STATUS(from_json(to.height_percentage_, from.extract_field("height_percentage")));
  TRY_STATUS(from_json(to.rotation_angle_, from.extract_field("rotation_angle")));
  TRY_STATUS(from_json(to.corner_radius_percentage_, from.extract_field("corner_radius_percentage")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionCheckPassword &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionSetBirthdate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterAdministrators &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::targetChatTypes &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_user_chats_, from.extract_field("allow_user_chats")));
  TRY_STATUS(from_json(to.allow_bot_chats_, from.extract_field("allow_bot_chats")));
  TRY_STATUS(from_json(to.allow_group_chats_, from.extract_field("allow_group_chats")));
  TRY_STATUS(from_json(to.allow_channel_chats_, from.extract_field("allow_channel_chats")));
  return Status::OK();
}

Status from_json(td_api::testString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBankCardNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBlockQuote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryUsers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::transactionDirectionOutgoing &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowBio &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::acceptTermsOfService &to, JsonObject &from) {
  TRY_STATUS(from_json(to.terms_of_service_id_, from.extract_field("terms_of_service_id")));
  return Status::OK();
}

Status from_json(td_api::addCustomServerLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::addPendingPaidMessageReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::addSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sound_, from.extract_field("sound")));
  return Status::OK();
}

Status from_json(td_api::answerShippingQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shipping_query_id_, from.extract_field("shipping_query_id")));
  TRY_STATUS(from_json(to.shipping_options_, from.extract_field("shipping_options")));
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::buyGiftUpgrade &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.prepaid_upgrade_hash_, from.extract_field("prepaid_upgrade_hash")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::cancelPreliminaryUploadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationPremiumPurchase &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  return Status::OK();
}

Status from_json(td_api::checkPremiumGiftCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::clearRecentEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::close &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::createBasicGroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, from.extract_field("basic_group_id")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::createGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.join_parameters_, from.extract_field("join_parameters")));
  return Status::OK();
}

Status from_json(td_api::createSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::deleteBotMediaPreviews &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.file_ids_, from.extract_field("file_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteChatHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.remove_from_chat_list_, from.extract_field("remove_from_chat_list")));
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedCredentials &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::disableProxy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::editBusinessMessageCaption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
  return Status::OK();
}

Status from_json(td_api::editChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.expiration_date_, from.extract_field("expiration_date")));
  TRY_STATUS(from_json(to.member_limit_, from.extract_field("member_limit")));
  TRY_STATUS(from_json(to.creates_join_request_, from.extract_field("creates_join_request")));
  return Status::OK();
}

Status from_json(td_api::editMessageCaption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
  return Status::OK();
}

Status from_json(td_api::editStarSubscription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.subscription_id_, from.extract_field("subscription_id")));
  TRY_STATUS(from_json(to.is_canceled_, from.extract_field("is_canceled")));
  return Status::OK();
}

Status from_json(td_api::finishFileGeneration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, from.extract_field("generation_id")));
  TRY_STATUS(from_json(to.error_, from.extract_field("error")));
  return Status::OK();
}

Status from_json(td_api::getArchiveChatListSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBackgroundUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getBotName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getCallbackQueryAnswer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostLevelFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_channel_, from.extract_field("is_channel")));
  TRY_STATUS(from_json(to.level_, from.extract_field("level")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderDefaultIconName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::getChatListsToAddChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatRevenueStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getChatStoryAlbums &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getConnectedAffiliatePrograms &to, JsonObject &from) {
  TRY_STATUS(from_json(to.affiliate_, from.extract_field("affiliate")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getCustomEmojiReactionAnimations &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultProfilePhotoCustomEmojiStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getExternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getForumTopicHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getGrossingWebAppBots &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  return Status::OK();
}

Status from_json(td_api::getLinkPreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.link_preview_options_, from.extract_field("link_preview_options")));
  return Status::OK();
}

Status from_json(td_api::getMapThumbnailFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.zoom_, from.extract_field("zoom")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  TRY_STATUS(from_json(to.scale_, from.extract_field("scale")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageEmbeddingCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.for_album_, from.extract_field("for_album")));
  return Status::OK();
}

Status from_json(td_api::getMessageStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getOwnedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_sticker_set_id_, from.extract_field("offset_sticker_set_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getPhoneNumberInfoSync &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.phone_number_prefix_, from.extract_field("phone_number_prefix")));
  return Status::OK();
}

Status from_json(td_api::getPremiumStickerExamples &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getReceivedGifts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.collection_id_, from.extract_field("collection_id")));
  TRY_STATUS(from_json(to.exclude_unsaved_, from.extract_field("exclude_unsaved")));
  TRY_STATUS(from_json(to.exclude_saved_, from.extract_field("exclude_saved")));
  TRY_STATUS(from_json(to.exclude_unlimited_, from.extract_field("exclude_unlimited")));
  TRY_STATUS(from_json(to.exclude_upgradable_, from.extract_field("exclude_upgradable")));
  TRY_STATUS(from_json(to.exclude_non_upgradable_, from.extract_field("exclude_non_upgradable")));
  TRY_STATUS(from_json(to.exclude_upgraded_, from.extract_field("exclude_upgraded")));
  TRY_STATUS(from_json(to.exclude_without_colors_, from.extract_field("exclude_without_colors")));
  TRY_STATUS(from_json(to.exclude_hosted_, from.extract_field("exclude_hosted")));
  TRY_STATUS(from_json(to.sort_by_price_, from.extract_field("sort_by_price")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getRemoteFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.remote_file_id_, from.extract_field("remote_file_id")));
  TRY_STATUS(from_json(to.file_type_, from.extract_field("file_type")));
  return Status::OK();
}

Status from_json(td_api::getScopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  return Status::OK();
}

Status from_json(td_api::getStarSubscriptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_expiring_, from.extract_field("only_expiring")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  return Status::OK();
}

Status from_json(td_api::getStorageStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_limit_, from.extract_field("chat_limit")));
  return Status::OK();
}

Status from_json(td_api::getSuggestedFileName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.directory_, from.extract_field("directory")));
  return Status::OK();
}

Status from_json(td_api::getTemporaryPasswordState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTopChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.category_, from.extract_field("category")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getUserLink &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getVideoChatStreams &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::importContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contacts_, from.extract_field("contacts")));
  return Status::OK();
}

Status from_json(td_api::launchPrepaidGiveaway &to, JsonObject &from) {
  TRY_STATUS(from_json(to.giveaway_id_, from.extract_field("giveaway_id")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  TRY_STATUS(from_json(to.winner_count_, from.extract_field("winner_count")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::loadSavedMessagesTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::openWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.topic_id_, from.extract_field("topic_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::processChatJoinRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.approve_, from.extract_field("approve")));
  return Status::OK();
}

Status from_json(td_api::readBusinessMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::registerUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  return Status::OK();
}

Status from_json(td_api::removeMessageReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  return Status::OK();
}

Status from_json(td_api::removeRecentlyFoundChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::reorderBotMediaPreviews &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.file_ids_, from.extract_field("file_ids")));
  return Status::OK();
}

Status from_json(td_api::replacePrimaryChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::reportSponsoredChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sponsored_chat_unique_id_, from.extract_field("sponsored_chat_unique_id")));
  TRY_STATUS(from_json_bytes(to.option_id_, from.extract_field("option_id")));
  return Status::OK();
}

Status from_json(td_api::resendEmailAddressVerificationCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetPassword &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchChatAffiliateProgram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::searchGiftsForResale &to, JsonObject &from) {
  TRY_STATUS(from_json(to.gift_id_, from.extract_field("gift_id")));
  TRY_STATUS(from_json(to.order_, from.extract_field("order")));
  TRY_STATUS(from_json(to.attributes_, from.extract_field("attributes")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchPublicStoriesByLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.emojis_, from.extract_field("emojis")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.input_language_codes_, from.extract_field("input_language_codes")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::sendBusinessMessageAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  TRY_STATUS(from_json(to.protect_content_, from.extract_field("protect_content")));
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  TRY_STATUS(from_json(to.input_message_contents_, from.extract_field("input_message_contents")));
  return Status::OK();
}

Status from_json(td_api::sendGroupCallMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::sendResoldGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.gift_name_, from.extract_field("gift_name")));
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.price_, from.extract_field("price")));
  return Status::OK();
}

Status from_json(td_api::setAuthenticationEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  return Status::OK();
}

Status from_json(td_api::setBotName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::setBusinessConnectedBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_, from.extract_field("bot")));
  return Status::OK();
}

Status from_json(td_api::setChatAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.available_reactions_, from.extract_field("available_reactions")));
  return Status::OK();
}

Status from_json(td_api::setChatMemberStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.member_id_, from.extract_field("member_id")));
  TRY_STATUS(from_json(to.status_, from.extract_field("status")));
  return Status::OK();
}

Status from_json(td_api::setChatSlowModeDelay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.slow_mode_delay_, from.extract_field("slow_mode_delay")));
  return Status::OK();
}

Status from_json(td_api::setDefaultBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_, from.extract_field("background")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.for_dark_theme_, from.extract_field("for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::setGameScore &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.edit_message_, from.extract_field("edit_message")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.score_, from.extract_field("score")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::setLogTagVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.new_verbosity_level_, from.extract_field("new_verbosity_level")));
  return Status::OK();
}

Status from_json(td_api::setName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  return Status::OK();
}

Status from_json(td_api::setPinnedChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::setReactionNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setStickerSetThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.format_, from.extract_field("format")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::setVideoChatDefaultParticipant &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.default_participant_id_, from.extract_field("default_participant_id")));
  return Status::OK();
}

Status from_json(td_api::stopPoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testUseUpdate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::toggleChatHasProtectedContent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.has_protected_content_, from.extract_field("has_protected_content")));
  return Status::OK();
}

Status from_json(td_api::toggleGeneralForumTopicIsHidden &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_hidden_, from.extract_field("is_hidden")));
  return Status::OK();
}

Status from_json(td_api::toggleSavedMessagesTopicIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsBroadcastGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::transferBusinessAccountStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::upgradeBasicGroupChatToSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::writeGeneratedFilePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, from.extract_field("generation_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::affiliateInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "affiliateInfo");
  jo("commission_per_mille", object.commission_per_mille_);
  jo("affiliate_chat_id", object.affiliate_chat_id_);
  if (object.star_amount_) {
    jo("star_amount", ToJson(*object.star_amount_));
  }
}

void to_json(JsonValueScope &jv, const td_api::archiveChatListSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "archiveChatListSettings");
  jo("archive_and_mute_new_chats_from_unknown_users", JsonBool{object.archive_and_mute_new_chats_from_unknown_users_});
  jo("keep_unmuted_chats_archived", JsonBool{object.keep_unmuted_chats_archived_});
  jo("keep_chats_from_folders_archived", JsonBool{object.keep_chats_from_folders_archived_});
}

void to_json(JsonValueScope &jv, const td_api::autoDownloadSettingsPresets &object) {
  auto jo = jv.enter_object();
  jo("@type", "autoDownloadSettingsPresets");
  if (object.low_) {
    jo("low", ToJson(*object.low_));
  }
  if (object.medium_) {
    jo("medium", ToJson(*object.medium_));
  }
  if (object.high_) {
    jo("high", ToJson(*object.high_));
  }
}

void to_json(JsonValueScope &jv, const td_api::BackgroundFill &object) {
  td_api::downcast_call(const_cast<td_api::BackgroundFill &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::backgroundFillSolid &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgroundFillSolid");
  jo("color", object.color_);
}

void to_json(JsonValueScope &jv, const td_api::backgroundFillGradient &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgroundFillGradient");
  jo("top_color", object.top_color_);
  jo("bottom_color", object.bottom_color_);
  jo("rotation_angle", object.rotation_angle_);
}

void to_json(JsonValueScope &jv, const td_api::backgroundFillFreeformGradient &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgroundFillFreeformGradient");
  jo("colors", ToJson(object.colors_));
}

void to_json(JsonValueScope &jv, const td_api::botCommand &object) {
  auto jo = jv.enter_object();
  jo("@type", "botCommand");
  jo("command", object.command_);
  jo("description", object.description_);
}

void to_json(JsonValueScope &jv, const td_api::BotWriteAccessAllowReason &object) {
  td_api::downcast_call(const_cast<td_api::BotWriteAccessAllowReason &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::botWriteAccessAllowReasonConnectedWebsite &object) {
  auto jo = jv.enter_object();
  jo("@type", "botWriteAccessAllowReasonConnectedWebsite");
  jo("domain_name", object.domain_name_);
}

void to_json(JsonValueScope &jv, const td_api::botWriteAccessAllowReasonAddedToAttachmentMenu &object) {
  auto jo = jv.enter_object();
  jo("@type", "botWriteAccessAllowReasonAddedToAttachmentMenu");
}

void to_json(JsonValueScope &jv, const td_api::botWriteAccessAllowReasonLaunchedWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "botWriteAccessAllowReasonLaunchedWebApp");
  if (object.web_app_) {
    jo("web_app", ToJson(*object.web_app_));
  }
}

void to_json(JsonValueScope &jv, const td_api::botWriteAccessAllowReasonAcceptedRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "botWriteAccessAllowReasonAcceptedRequest");
}

void to_json(JsonValueScope &jv, const td_api::businessConnectedBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessConnectedBot");
  jo("bot_user_id", object.bot_user_id_);
  if (object.recipients_) {
    jo("recipients", ToJson(*object.recipients_));
  }
  if (object.rights_) {
    jo("rights", ToJson(*object.rights_));
  }
}

void to_json(JsonValueScope &jv, const td_api::businessMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessMessages");
  jo("messages", ToJson(object.messages_));
}

void to_json(JsonValueScope &jv, const td_api::callServer &object) {
  auto jo = jv.enter_object();
  jo("@type", "callServer");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("ip_address", object.ip_address_);
  jo("ipv6_address", object.ipv6_address_);
  jo("port", object.port_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chat &object) {
  auto jo = jv.enter_object();
  jo("@type", "chat");
  jo("id", object.id_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("title", object.title_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("accent_color_id", object.accent_color_id_);
  jo("background_custom_emoji_id", ToJson(JsonInt64{object.background_custom_emoji_id_}));
  if (object.upgraded_gift_colors_) {
    jo("upgraded_gift_colors", ToJson(*object.upgraded_gift_colors_));
  }
  jo("profile_accent_color_id", object.profile_accent_color_id_);
  jo("profile_background_custom_emoji_id", ToJson(JsonInt64{object.profile_background_custom_emoji_id_}));
  if (object.permissions_) {
    jo("permissions", ToJson(*object.permissions_));
  }
  if (object.last_message_) {
    jo("last_message", ToJson(*object.last_message_));
  }
  jo("positions", ToJson(object.positions_));
  jo("chat_lists", ToJson(object.chat_lists_));
  if (object.message_sender_id_) {
    jo("message_sender_id", ToJson(*object.message_sender_id_));
  }
  if (object.block_list_) {
    jo("block_list", ToJson(*object.block_list_));
  }
  jo("has_protected_content", JsonBool{object.has_protected_content_});
  jo("is_translatable", JsonBool{object.is_translatable_});
  jo("is_marked_as_unread", JsonBool{object.is_marked_as_unread_});
  jo("view_as_topics", JsonBool{object.view_as_topics_});
  jo("has_scheduled_messages", JsonBool{object.has_scheduled_messages_});
  jo("can_be_deleted_only_for_self", JsonBool{object.can_be_deleted_only_for_self_});
  jo("can_be_deleted_for_all_users", JsonBool{object.can_be_deleted_for_all_users_});
  jo("can_be_reported", JsonBool{object.can_be_reported_});
  jo("default_disable_notification", JsonBool{object.default_disable_notification_});
  jo("unread_count", object.unread_count_);
  jo("last_read_inbox_message_id", object.last_read_inbox_message_id_);
  jo("last_read_outbox_message_id", object.last_read_outbox_message_id_);
  jo("unread_mention_count", object.unread_mention_count_);
  jo("unread_reaction_count", object.unread_reaction_count_);
  if (object.notification_settings_) {
    jo("notification_settings", ToJson(*object.notification_settings_));
  }
  if (object.available_reactions_) {
    jo("available_reactions", ToJson(*object.available_reactions_));
  }
  jo("message_auto_delete_time", object.message_auto_delete_time_);
  if (object.emoji_status_) {
    jo("emoji_status", ToJson(*object.emoji_status_));
  }
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  if (object.theme_) {
    jo("theme", ToJson(*object.theme_));
  }
  if (object.action_bar_) {
    jo("action_bar", ToJson(*object.action_bar_));
  }
  if (object.business_bot_manage_bar_) {
    jo("business_bot_manage_bar", ToJson(*object.business_bot_manage_bar_));
  }
  if (object.video_chat_) {
    jo("video_chat", ToJson(*object.video_chat_));
  }
  if (object.pending_join_requests_) {
    jo("pending_join_requests", ToJson(*object.pending_join_requests_));
  }
  jo("reply_markup_message_id", object.reply_markup_message_id_);
  if (object.draft_message_) {
    jo("draft_message", ToJson(*object.draft_message_));
  }
  jo("client_data", object.client_data_);
}

void to_json(JsonValueScope &jv, const td_api::chatBoost &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoost");
  jo("id", object.id_);
  jo("count", object.count_);
  if (object.source_) {
    jo("source", ToJson(*object.source_));
  }
  jo("start_date", object.start_date_);
  jo("expiration_date", object.expiration_date_);
}

void to_json(JsonValueScope &jv, const td_api::chatEvent &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEvent");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("date", object.date_);
  if (object.member_id_) {
    jo("member_id", ToJson(*object.member_id_));
  }
  if (object.action_) {
    jo("action", ToJson(*object.action_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatFolderName &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderName");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("animate_custom_emoji", JsonBool{object.animate_custom_emoji_});
}

void to_json(JsonValueScope &jv, const td_api::chatJoinRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatJoinRequest");
  jo("user_id", object.user_id_);
  jo("date", object.date_);
  jo("bio", object.bio_);
}

void to_json(JsonValueScope &jv, const td_api::chatMessageSender &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMessageSender");
  if (object.sender_) {
    jo("sender", ToJson(*object.sender_));
  }
  jo("needs_premium", JsonBool{object.needs_premium_});
}

void to_json(JsonValueScope &jv, const td_api::chatPosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPosition");
  if (object.list_) {
    jo("list", ToJson(*object.list_));
  }
  jo("order", ToJson(JsonInt64{object.order_}));
  jo("is_pinned", JsonBool{object.is_pinned_});
  if (object.source_) {
    jo("source", ToJson(*object.source_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsInteractionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsInteractionInfo");
  if (object.object_type_) {
    jo("object_type", ToJson(*object.object_type_));
  }
  jo("view_count", object.view_count_);
  jo("forward_count", object.forward_count_);
  jo("reaction_count", object.reaction_count_);
}

void to_json(JsonValueScope &jv, const td_api::checklist &object) {
  auto jo = jv.enter_object();
  jo("@type", "checklist");
  if (object.title_) {
    jo("title", ToJson(*object.title_));
  }
  jo("tasks", ToJson(object.tasks_));
  jo("others_can_add_tasks", JsonBool{object.others_can_add_tasks_});
  jo("can_add_tasks", JsonBool{object.can_add_tasks_});
  jo("others_can_mark_tasks_as_done", JsonBool{object.others_can_mark_tasks_as_done_});
  jo("can_mark_tasks_as_done", JsonBool{object.can_mark_tasks_as_done_});
}

void to_json(JsonValueScope &jv, const td_api::ConnectionState &object) {
  td_api::downcast_call(const_cast<td_api::ConnectionState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::connectionStateWaitingForNetwork &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectionStateWaitingForNetwork");
}

void to_json(JsonValueScope &jv, const td_api::connectionStateConnectingToProxy &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectionStateConnectingToProxy");
}

void to_json(JsonValueScope &jv, const td_api::connectionStateConnecting &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectionStateConnecting");
}

void to_json(JsonValueScope &jv, const td_api::connectionStateUpdating &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectionStateUpdating");
}

void to_json(JsonValueScope &jv, const td_api::connectionStateReady &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectionStateReady");
}

void to_json(JsonValueScope &jv, const td_api::databaseStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "databaseStatistics");
  jo("statistics", object.statistics_);
}

void to_json(JsonValueScope &jv, const td_api::draftMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "draftMessage");
  if (object.reply_to_) {
    jo("reply_to", ToJson(*object.reply_to_));
  }
  jo("date", object.date_);
  if (object.input_message_text_) {
    jo("input_message_text", ToJson(*object.input_message_text_));
  }
  jo("effect_id", ToJson(JsonInt64{object.effect_id_}));
  if (object.suggested_post_info_) {
    jo("suggested_post_info", ToJson(*object.suggested_post_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::emojiReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiReaction");
  jo("emoji", object.emoji_);
  jo("title", object.title_);
  jo("is_active", JsonBool{object.is_active_});
  if (object.static_icon_) {
    jo("static_icon", ToJson(*object.static_icon_));
  }
  if (object.appear_animation_) {
    jo("appear_animation", ToJson(*object.appear_animation_));
  }
  if (object.select_animation_) {
    jo("select_animation", ToJson(*object.select_animation_));
  }
  if (object.activate_animation_) {
    jo("activate_animation", ToJson(*object.activate_animation_));
  }
  if (object.effect_animation_) {
    jo("effect_animation", ToJson(*object.effect_animation_));
  }
  if (object.around_animation_) {
    jo("around_animation", ToJson(*object.around_animation_));
  }
  if (object.center_animation_) {
    jo("center_animation", ToJson(*object.center_animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::factCheck &object) {
  auto jo = jv.enter_object();
  jo("@type", "factCheck");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("country_code", object.country_code_);
}

void to_json(JsonValueScope &jv, const td_api::forumTopic &object) {
  auto jo = jv.enter_object();
  jo("@type", "forumTopic");
  if (object.info_) {
    jo("info", ToJson(*object.info_));
  }
  if (object.last_message_) {
    jo("last_message", ToJson(*object.last_message_));
  }
  jo("order", ToJson(JsonInt64{object.order_}));
  jo("is_pinned", JsonBool{object.is_pinned_});
  jo("unread_count", object.unread_count_);
  jo("last_read_inbox_message_id", object.last_read_inbox_message_id_);
  jo("last_read_outbox_message_id", object.last_read_outbox_message_id_);
  jo("unread_mention_count", object.unread_mention_count_);
  jo("unread_reaction_count", object.unread_reaction_count_);
  if (object.notification_settings_) {
    jo("notification_settings", ToJson(*object.notification_settings_));
  }
  if (object.draft_message_) {
    jo("draft_message", ToJson(*object.draft_message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::foundFileDownloads &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundFileDownloads");
  if (object.total_counts_) {
    jo("total_counts", ToJson(*object.total_counts_));
  }
  jo("files", ToJson(object.files_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::gameHighScore &object) {
  auto jo = jv.enter_object();
  jo("@type", "gameHighScore");
  jo("position", object.position_);
  jo("user_id", object.user_id_);
  jo("score", object.score_);
}

void to_json(JsonValueScope &jv, const td_api::giftResaleParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftResaleParameters");
  jo("star_count", object.star_count_);
  jo("toncoin_cent_count", object.toncoin_cent_count_);
  jo("toncoin_only", JsonBool{object.toncoin_only_});
}

void to_json(JsonValueScope &jv, const td_api::GiveawayParticipantStatus &object) {
  td_api::downcast_call(const_cast<td_api::GiveawayParticipantStatus &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::giveawayParticipantStatusEligible &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayParticipantStatusEligible");
}

void to_json(JsonValueScope &jv, const td_api::giveawayParticipantStatusParticipating &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayParticipantStatusParticipating");
}

void to_json(JsonValueScope &jv, const td_api::giveawayParticipantStatusAlreadyWasMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayParticipantStatusAlreadyWasMember");
  jo("joined_chat_date", object.joined_chat_date_);
}

void to_json(JsonValueScope &jv, const td_api::giveawayParticipantStatusAdministrator &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayParticipantStatusAdministrator");
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::giveawayParticipantStatusDisallowedCountry &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayParticipantStatusDisallowedCountry");
  jo("user_country_code", object.user_country_code_);
}

void to_json(JsonValueScope &jv, const td_api::groupCallVideoSourceGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallVideoSourceGroup");
  jo("semantics", object.semantics_);
  jo("source_ids", ToJson(object.source_ids_));
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultsButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultsButton");
  jo("text", object.text_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputSuggestedPostInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputSuggestedPostInfo");
  if (object.price_) {
    jo("price", ToJson(*object.price_));
  }
  jo("send_date", object.send_date_);
}

void to_json(JsonValueScope &jv, const td_api::keyboardButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButton");
  jo("text", object.text_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewOptions");
  jo("is_disabled", JsonBool{object.is_disabled_});
  jo("url", object.url_);
  jo("force_small_media", JsonBool{object.force_small_media_});
  jo("force_large_media", JsonBool{object.force_large_media_});
  jo("show_above_text", JsonBool{object.show_above_text_});
}

void to_json(JsonValueScope &jv, const td_api::LoginUrlInfo &object) {
  td_api::downcast_call(const_cast<td_api::LoginUrlInfo &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::loginUrlInfoOpen &object) {
  auto jo = jv.enter_object();
  jo("@type", "loginUrlInfoOpen");
  jo("url", object.url_);
  jo("skip_confirmation", JsonBool{object.skip_confirmation_});
}

void to_json(JsonValueScope &jv, const td_api::loginUrlInfoRequestConfirmation &object) {
  auto jo = jv.enter_object();
  jo("@type", "loginUrlInfoRequestConfirmation");
  jo("url", object.url_);
  jo("domain", object.domain_);
  jo("bot_user_id", object.bot_user_id_);
  jo("request_write_access", JsonBool{object.request_write_access_});
}

void to_json(JsonValueScope &jv, const td_api::messageCopyOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageCopyOptions");
  jo("send_copy", JsonBool{object.send_copy_});
  jo("replace_caption", JsonBool{object.replace_caption_});
  if (object.new_caption_) {
    jo("new_caption", ToJson(*object.new_caption_));
  }
  jo("new_show_caption_above_media", JsonBool{object.new_show_caption_above_media_});
}

void to_json(JsonValueScope &jv, const td_api::MessageOrigin &object) {
  td_api::downcast_call(const_cast<td_api::MessageOrigin &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageOriginUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageOriginUser");
  jo("sender_user_id", object.sender_user_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageOriginHiddenUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageOriginHiddenUser");
  jo("sender_name", object.sender_name_);
}

void to_json(JsonValueScope &jv, const td_api::messageOriginChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageOriginChat");
  jo("sender_chat_id", object.sender_chat_id_);
  jo("author_signature", object.author_signature_);
}

void to_json(JsonValueScope &jv, const td_api::messageOriginChannel &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageOriginChannel");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("author_signature", object.author_signature_);
}

void to_json(JsonValueScope &jv, const td_api::MessageSchedulingState &object) {
  td_api::downcast_call(const_cast<td_api::MessageSchedulingState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageSchedulingStateSendAtDate &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSchedulingStateSendAtDate");
  jo("send_date", object.send_date_);
}

void to_json(JsonValueScope &jv, const td_api::messageSchedulingStateSendWhenOnline &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSchedulingStateSendWhenOnline");
}

void to_json(JsonValueScope &jv, const td_api::messageSchedulingStateSendWhenVideoProcessed &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSchedulingStateSendWhenVideoProcessed");
  jo("send_date", object.send_date_);
}

void to_json(JsonValueScope &jv, const td_api::messageViewers &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageViewers");
  jo("viewers", ToJson(object.viewers_));
}

void to_json(JsonValueScope &jv, const td_api::NotificationGroupType &object) {
  td_api::downcast_call(const_cast<td_api::NotificationGroupType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationGroupTypeMessages");
}

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeMentions &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationGroupTypeMentions");
}

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeSecretChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationGroupTypeSecretChat");
}

void to_json(JsonValueScope &jv, const td_api::notificationGroupTypeCalls &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationGroupTypeCalls");
}

void to_json(JsonValueScope &jv, const td_api::PageBlock &object) {
  td_api::downcast_call(const_cast<td_api::PageBlock &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::pageBlockTitle &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockTitle");
  if (object.title_) {
    jo("title", ToJson(*object.title_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockSubtitle &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockSubtitle");
  if (object.subtitle_) {
    jo("subtitle", ToJson(*object.subtitle_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockAuthorDate &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockAuthorDate");
  if (object.author_) {
    jo("author", ToJson(*object.author_));
  }
  jo("publish_date", object.publish_date_);
}

void to_json(JsonValueScope &jv, const td_api::pageBlockHeader &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockHeader");
  if (object.header_) {
    jo("header", ToJson(*object.header_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockSubheader &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockSubheader");
  if (object.subheader_) {
    jo("subheader", ToJson(*object.subheader_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockKicker &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockKicker");
  if (object.kicker_) {
    jo("kicker", ToJson(*object.kicker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockParagraph &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockParagraph");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockPreformatted &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockPreformatted");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("language", object.language_);
}

void to_json(JsonValueScope &jv, const td_api::pageBlockFooter &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockFooter");
  if (object.footer_) {
    jo("footer", ToJson(*object.footer_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockDivider &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockDivider");
}

void to_json(JsonValueScope &jv, const td_api::pageBlockAnchor &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockAnchor");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::pageBlockList &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockList");
  jo("items", ToJson(object.items_));
}

void to_json(JsonValueScope &jv, const td_api::pageBlockBlockQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockBlockQuote");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  if (object.credit_) {
    jo("credit", ToJson(*object.credit_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockPullQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockPullQuote");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  if (object.credit_) {
    jo("credit", ToJson(*object.credit_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockAnimation");
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("need_autoplay", JsonBool{object.need_autoplay_});
}

void to_json(JsonValueScope &jv, const td_api::pageBlockAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockAudio");
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockPhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::pageBlockVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("need_autoplay", JsonBool{object.need_autoplay_});
  jo("is_looped", JsonBool{object.is_looped_});
}

void to_json(JsonValueScope &jv, const td_api::pageBlockVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockVoiceNote");
  if (object.voice_note_) {
    jo("voice_note", ToJson(*object.voice_note_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockCover &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockCover");
  if (object.cover_) {
    jo("cover", ToJson(*object.cover_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockEmbedded &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockEmbedded");
  jo("url", object.url_);
  jo("html", object.html_);
  if (object.poster_photo_) {
    jo("poster_photo", ToJson(*object.poster_photo_));
  }
  jo("width", object.width_);
  jo("height", object.height_);
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("is_full_width", JsonBool{object.is_full_width_});
  jo("allow_scrolling", JsonBool{object.allow_scrolling_});
}

void to_json(JsonValueScope &jv, const td_api::pageBlockEmbeddedPost &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockEmbeddedPost");
  jo("url", object.url_);
  jo("author", object.author_);
  if (object.author_photo_) {
    jo("author_photo", ToJson(*object.author_photo_));
  }
  jo("date", object.date_);
  jo("page_blocks", ToJson(object.page_blocks_));
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockCollage &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockCollage");
  jo("page_blocks", ToJson(object.page_blocks_));
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockSlideshow &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockSlideshow");
  jo("page_blocks", ToJson(object.page_blocks_));
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::pageBlockChatLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockChatLink");
  jo("title", object.title_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("accent_color_id", object.accent_color_id_);
  jo("username", object.username_);
}

void to_json(JsonValueScope &jv, const td_api::pageBlockTable &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockTable");
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("cells", ToJson(object.cells_));
  jo("is_bordered", JsonBool{object.is_bordered_});
  jo("is_striped", JsonBool{object.is_striped_});
}

void to_json(JsonValueScope &jv, const td_api::pageBlockDetails &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockDetails");
  if (object.header_) {
    jo("header", ToJson(*object.header_));
  }
  jo("page_blocks", ToJson(object.page_blocks_));
  jo("is_open", JsonBool{object.is_open_});
}

void to_json(JsonValueScope &jv, const td_api::pageBlockRelatedArticles &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockRelatedArticles");
  if (object.header_) {
    jo("header", ToJson(*object.header_));
  }
  jo("articles", ToJson(object.articles_));
}

void to_json(JsonValueScope &jv, const td_api::pageBlockMap &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockMap");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("zoom", object.zoom_);
  jo("width", object.width_);
  jo("height", object.height_);
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::paidReactor &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidReactor");
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("star_count", object.star_count_);
  jo("is_top", JsonBool{object.is_top_});
  jo("is_me", JsonBool{object.is_me_});
  jo("is_anonymous", JsonBool{object.is_anonymous_});
}

void to_json(JsonValueScope &jv, const td_api::passportSuitableElement &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportSuitableElement");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("is_selfie_required", JsonBool{object.is_selfie_required_});
  jo("is_translation_required", JsonBool{object.is_translation_required_});
  jo("is_native_name_required", JsonBool{object.is_native_name_required_});
}

void to_json(JsonValueScope &jv, const td_api::personalDetails &object) {
  auto jo = jv.enter_object();
  jo("@type", "personalDetails");
  jo("first_name", object.first_name_);
  jo("middle_name", object.middle_name_);
  jo("last_name", object.last_name_);
  jo("native_first_name", object.native_first_name_);
  jo("native_middle_name", object.native_middle_name_);
  jo("native_last_name", object.native_last_name_);
  if (object.birthdate_) {
    jo("birthdate", ToJson(*object.birthdate_));
  }
  jo("gender", object.gender_);
  jo("country_code", object.country_code_);
  jo("residence_country_code", object.residence_country_code_);
}

void to_json(JsonValueScope &jv, const td_api::PremiumFeature &object) {
  td_api::downcast_call(const_cast<td_api::PremiumFeature &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureIncreasedLimits &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureIncreasedLimits");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureIncreasedUploadFileSize &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureIncreasedUploadFileSize");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureImprovedDownloadSpeed &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureImprovedDownloadSpeed");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureVoiceRecognition &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureVoiceRecognition");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureDisabledAds &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureDisabledAds");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureUniqueReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureUniqueReactions");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureUniqueStickers &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureUniqueStickers");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureCustomEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureCustomEmoji");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureAdvancedChatManagement &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureAdvancedChatManagement");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureProfileBadge &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureProfileBadge");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureEmojiStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureEmojiStatus");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureAnimatedProfilePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureAnimatedProfilePhoto");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureForumTopicIcon &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureForumTopicIcon");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureAppIcons &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureAppIcons");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureRealTimeChatTranslation &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureRealTimeChatTranslation");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureUpgradedStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureUpgradedStories");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureChatBoost &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureChatBoost");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureAccentColor &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureAccentColor");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureBackgroundForBoth &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureBackgroundForBoth");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureSavedMessagesTags &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureSavedMessagesTags");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureMessagePrivacy &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureMessagePrivacy");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureLastSeenTimes &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureLastSeenTimes");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureBusiness &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureBusiness");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureMessageEffects &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureMessageEffects");
}

void to_json(JsonValueScope &jv, const td_api::premiumFeatureChecklists &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatureChecklists");
}

void to_json(JsonValueScope &jv, const td_api::PremiumLimitType &object) {
  td_api::downcast_call(const_cast<td_api::PremiumLimitType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeSupergroupCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeSupergroupCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypePinnedChatCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypePinnedChatCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeCreatedPublicChatCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeCreatedPublicChatCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeSavedAnimationCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeSavedAnimationCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeFavoriteStickerCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeFavoriteStickerCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeChatFolderCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeChatFolderCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeChatFolderChosenChatCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeChatFolderChosenChatCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypePinnedArchivedChatCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypePinnedArchivedChatCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypePinnedSavedMessagesTopicCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypePinnedSavedMessagesTopicCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeCaptionLength &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeCaptionLength");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeBioLength &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeBioLength");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeChatFolderInviteLinkCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeChatFolderInviteLinkCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeShareableChatFolderCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeShareableChatFolderCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeActiveStoryCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeActiveStoryCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeWeeklyPostedStoryCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeWeeklyPostedStoryCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeMonthlyPostedStoryCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeMonthlyPostedStoryCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeStoryCaptionLength &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeStoryCaptionLength");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeStorySuggestedReactionAreaCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeStorySuggestedReactionAreaCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeSimilarChatCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeSimilarChatCount");
}

void to_json(JsonValueScope &jv, const td_api::profileAccentColors &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileAccentColors");
  jo("palette_colors", ToJson(object.palette_colors_));
  jo("background_colors", ToJson(object.background_colors_));
  jo("story_colors", ToJson(object.story_colors_));
}

void to_json(JsonValueScope &jv, const td_api::PushMessageContent &object) {
  td_api::downcast_call(const_cast<td_api::PushMessageContent &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentHidden &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentHidden");
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentAnimation");
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  jo("caption", object.caption_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentAudio");
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentContact &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentContact");
  jo("name", object.name_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentContactRegistered &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentContactRegistered");
  jo("as_premium_account", JsonBool{object.as_premium_account_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentDocument");
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentGame");
  jo("title", object.title_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentGameScore &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentGameScore");
  jo("title", object.title_);
  jo("score", object.score_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentInvoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentInvoice");
  jo("price", object.price_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentLocation");
  jo("is_live", JsonBool{object.is_live_});
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentPaidMedia &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentPaidMedia");
  jo("star_count", object.star_count_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentPhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("caption", object.caption_);
  jo("is_secret", JsonBool{object.is_secret_});
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentPoll &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentPoll");
  jo("question", object.question_);
  jo("is_regular", JsonBool{object.is_regular_});
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentPremiumGiftCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentPremiumGiftCode");
  jo("month_count", object.month_count_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentGiveaway &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentGiveaway");
  jo("winner_count", object.winner_count_);
  if (object.prize_) {
    jo("prize", ToJson(*object.prize_));
  }
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentGift");
  jo("star_count", object.star_count_);
  jo("is_prepaid_upgrade", JsonBool{object.is_prepaid_upgrade_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentUpgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentUpgradedGift");
  jo("is_upgrade", JsonBool{object.is_upgrade_});
  jo("is_prepaid_upgrade", JsonBool{object.is_prepaid_upgrade_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentScreenshotTaken &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentScreenshotTaken");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentSticker");
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  jo("emoji", object.emoji_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentStory");
  jo("is_mention", JsonBool{object.is_mention_});
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentText &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentText");
  jo("text", object.text_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChecklist &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChecklist");
  jo("title", object.title_);
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  jo("caption", object.caption_);
  jo("is_secret", JsonBool{object.is_secret_});
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentVideoNote");
  if (object.video_note_) {
    jo("video_note", ToJson(*object.video_note_));
  }
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentVoiceNote");
  if (object.voice_note_) {
    jo("voice_note", ToJson(*object.voice_note_));
  }
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentBasicGroupChatCreate &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentBasicGroupChatCreate");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVideoChatStarted &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentVideoChatStarted");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentVideoChatEnded &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentVideoChatEnded");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentInviteVideoChatParticipants &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentInviteVideoChatParticipants");
  jo("is_current_user", JsonBool{object.is_current_user_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatAddMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatAddMembers");
  jo("member_name", object.member_name_);
  jo("is_current_user", JsonBool{object.is_current_user_});
  jo("is_returned", JsonBool{object.is_returned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatChangePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatChangePhoto");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatChangeTitle &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatChangeTitle");
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatSetBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatSetBackground");
  jo("is_same", JsonBool{object.is_same_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatSetTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatSetTheme");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatDeleteMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatDeleteMember");
  jo("member_name", object.member_name_);
  jo("is_current_user", JsonBool{object.is_current_user_});
  jo("is_left", JsonBool{object.is_left_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatJoinByLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatJoinByLink");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChatJoinByRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChatJoinByRequest");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentRecurringPayment &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentRecurringPayment");
  jo("amount", object.amount_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentSuggestProfilePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentSuggestProfilePhoto");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentSuggestBirthdate &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentSuggestBirthdate");
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentProximityAlertTriggered &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentProximityAlertTriggered");
  jo("distance", object.distance_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChecklistTasksAdded &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChecklistTasksAdded");
  jo("task_count", object.task_count_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentChecklistTasksDone &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentChecklistTasksDone");
  jo("task_count", object.task_count_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentMessageForwards &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentMessageForwards");
  jo("total_count", object.total_count_);
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentMediaAlbum &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentMediaAlbum");
  jo("total_count", object.total_count_);
  jo("has_photos", JsonBool{object.has_photos_});
  jo("has_videos", JsonBool{object.has_videos_});
  jo("has_audios", JsonBool{object.has_audios_});
  jo("has_documents", JsonBool{object.has_documents_});
}

void to_json(JsonValueScope &jv, const td_api::readDatePrivacySettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "readDatePrivacySettings");
  jo("show_read_date", JsonBool{object.show_read_date_});
}

void to_json(JsonValueScope &jv, const td_api::reportOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportOption");
  jo("id", base64_encode(object.id_));
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::savedMessagesTag &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedMessagesTag");
  if (object.tag_) {
    jo("tag", ToJson(*object.tag_));
  }
  jo("label", object.label_);
  jo("count", object.count_);
}

void to_json(JsonValueScope &jv, const td_api::SentGift &object) {
  td_api::downcast_call(const_cast<td_api::SentGift &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::sentGiftRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "sentGiftRegular");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::sentGiftUpgraded &object) {
  auto jo = jv.enter_object();
  jo("@type", "sentGiftUpgraded");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::sponsoredChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "sponsoredChat");
  jo("unique_id", object.unique_id_);
  jo("chat_id", object.chat_id_);
  jo("sponsor_info", object.sponsor_info_);
  jo("additional_info", object.additional_info_);
}

void to_json(JsonValueScope &jv, const td_api::starPaymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "starPaymentOption");
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("star_count", object.star_count_);
  jo("store_product_id", object.store_product_id_);
  jo("is_additional", JsonBool{object.is_additional_});
}

void to_json(JsonValueScope &jv, const td_api::StarTransactionType &object) {
  td_api::downcast_call(const_cast<td_api::StarTransactionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypePremiumBotDeposit &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypePremiumBotDeposit");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeAppStoreDeposit &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeAppStoreDeposit");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGooglePlayDeposit &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGooglePlayDeposit");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeFragmentDeposit &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeFragmentDeposit");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeUserDeposit &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeUserDeposit");
  jo("user_id", object.user_id_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGiveawayDeposit &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGiveawayDeposit");
  jo("chat_id", object.chat_id_);
  jo("giveaway_message_id", object.giveaway_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeFragmentWithdrawal &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeFragmentWithdrawal");
  if (object.withdrawal_state_) {
    jo("withdrawal_state", ToJson(*object.withdrawal_state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeTelegramAdsWithdrawal &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeTelegramAdsWithdrawal");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeTelegramApiUsage &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeTelegramApiUsage");
  jo("request_count", object.request_count_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBotPaidMediaPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBotPaidMediaPurchase");
  jo("user_id", object.user_id_);
  jo("media", ToJson(object.media_));
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBotPaidMediaSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBotPaidMediaSale");
  jo("user_id", object.user_id_);
  jo("media", ToJson(object.media_));
  jo("payload", object.payload_);
  if (object.affiliate_) {
    jo("affiliate", ToJson(*object.affiliate_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeChannelPaidMediaPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeChannelPaidMediaPurchase");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("media", ToJson(object.media_));
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeChannelPaidMediaSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeChannelPaidMediaSale");
  jo("user_id", object.user_id_);
  jo("message_id", object.message_id_);
  jo("media", ToJson(object.media_));
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBotInvoicePurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBotInvoicePurchase");
  jo("user_id", object.user_id_);
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBotInvoiceSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBotInvoiceSale");
  jo("user_id", object.user_id_);
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
  jo("invoice_payload", base64_encode(object.invoice_payload_));
  if (object.affiliate_) {
    jo("affiliate", ToJson(*object.affiliate_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBotSubscriptionPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBotSubscriptionPurchase");
  jo("user_id", object.user_id_);
  jo("subscription_period", object.subscription_period_);
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBotSubscriptionSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBotSubscriptionSale");
  jo("user_id", object.user_id_);
  jo("subscription_period", object.subscription_period_);
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
  jo("invoice_payload", base64_encode(object.invoice_payload_));
  if (object.affiliate_) {
    jo("affiliate", ToJson(*object.affiliate_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeChannelSubscriptionPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeChannelSubscriptionPurchase");
  jo("chat_id", object.chat_id_);
  jo("subscription_period", object.subscription_period_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeChannelSubscriptionSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeChannelSubscriptionSale");
  jo("user_id", object.user_id_);
  jo("subscription_period", object.subscription_period_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGiftPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGiftPurchase");
  if (object.owner_id_) {
    jo("owner_id", ToJson(*object.owner_id_));
  }
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGiftTransfer &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGiftTransfer");
  if (object.owner_id_) {
    jo("owner_id", ToJson(*object.owner_id_));
  }
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGiftOriginalDetailsDrop &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGiftOriginalDetailsDrop");
  if (object.owner_id_) {
    jo("owner_id", ToJson(*object.owner_id_));
  }
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGiftSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGiftSale");
  jo("user_id", object.user_id_);
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGiftUpgrade &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGiftUpgrade");
  jo("user_id", object.user_id_);
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeGiftUpgradePurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeGiftUpgradePurchase");
  if (object.owner_id_) {
    jo("owner_id", ToJson(*object.owner_id_));
  }
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeUpgradedGiftPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeUpgradedGiftPurchase");
  jo("user_id", object.user_id_);
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeUpgradedGiftSale &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeUpgradedGiftSale");
  jo("user_id", object.user_id_);
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  jo("commission_per_mille", object.commission_per_mille_);
  if (object.commission_star_amount_) {
    jo("commission_star_amount", ToJson(*object.commission_star_amount_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeChannelPaidReactionSend &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeChannelPaidReactionSend");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeChannelPaidReactionReceive &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeChannelPaidReactionReceive");
  jo("user_id", object.user_id_);
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeAffiliateProgramCommission &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeAffiliateProgramCommission");
  jo("chat_id", object.chat_id_);
  jo("commission_per_mille", object.commission_per_mille_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypePaidMessageSend &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypePaidMessageSend");
  jo("chat_id", object.chat_id_);
  jo("message_count", object.message_count_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypePaidMessageReceive &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypePaidMessageReceive");
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("message_count", object.message_count_);
  jo("commission_per_mille", object.commission_per_mille_);
  if (object.commission_star_amount_) {
    jo("commission_star_amount", ToJson(*object.commission_star_amount_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeSuggestedPostPaymentSend &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeSuggestedPostPaymentSend");
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeSuggestedPostPaymentReceive &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeSuggestedPostPaymentReceive");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypePremiumPurchase &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypePremiumPurchase");
  jo("user_id", object.user_id_);
  jo("month_count", object.month_count_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBusinessBotTransferSend &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBusinessBotTransferSend");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeBusinessBotTransferReceive &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeBusinessBotTransferReceive");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypePublicPostSearch &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypePublicPostSearch");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionTypeUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionTypeUnsupported");
}

void to_json(JsonValueScope &jv, const td_api::stickerSets &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerSets");
  jo("total_count", object.total_count_);
  jo("sets", ToJson(object.sets_));
}

void to_json(JsonValueScope &jv, const td_api::storyAlbum &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyAlbum");
  jo("id", object.id_);
  jo("name", object.name_);
  if (object.photo_icon_) {
    jo("photo_icon", ToJson(*object.photo_icon_));
  }
  if (object.video_icon_) {
    jo("video_icon", ToJson(*object.video_icon_));
  }
}

void to_json(JsonValueScope &jv, const td_api::StoryInteractionType &object) {
  td_api::downcast_call(const_cast<td_api::StoryInteractionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::storyInteractionTypeView &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteractionTypeView");
  if (object.chosen_reaction_type_) {
    jo("chosen_reaction_type", ToJson(*object.chosen_reaction_type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storyInteractionTypeForward &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteractionTypeForward");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::storyInteractionTypeRepost &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteractionTypeRepost");
  if (object.story_) {
    jo("story", ToJson(*object.story_));
  }
}

void to_json(JsonValueScope &jv, const td_api::suggestedPostInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedPostInfo");
  if (object.price_) {
    jo("price", ToJson(*object.price_));
  }
  jo("send_date", object.send_date_);
  if (object.state_) {
    jo("state", ToJson(*object.state_));
  }
  jo("can_be_approved", JsonBool{object.can_be_approved_});
  jo("can_be_declined", JsonBool{object.can_be_declined_});
}

void to_json(JsonValueScope &jv, const td_api::TargetChat &object) {
  td_api::downcast_call(const_cast<td_api::TargetChat &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::targetChatCurrent &object) {
  auto jo = jv.enter_object();
  jo("@type", "targetChatCurrent");
}

void to_json(JsonValueScope &jv, const td_api::targetChatChosen &object) {
  auto jo = jv.enter_object();
  jo("@type", "targetChatChosen");
  if (object.types_) {
    jo("types", ToJson(*object.types_));
  }
}

void to_json(JsonValueScope &jv, const td_api::targetChatInternalLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "targetChatInternalLink");
  if (object.link_) {
    jo("link", ToJson(*object.link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::testVectorString &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorString");
  jo("value", ToJson(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::ThumbnailFormat &object) {
  td_api::downcast_call(const_cast<td_api::ThumbnailFormat &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatJpeg &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnailFormatJpeg");
}

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatGif &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnailFormatGif");
}

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatMpeg4 &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnailFormatMpeg4");
}

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatPng &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnailFormatPng");
}

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatTgs &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnailFormatTgs");
}

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatWebm &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnailFormatWebm");
}

void to_json(JsonValueScope &jv, const td_api::thumbnailFormatWebp &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnailFormatWebp");
}

void to_json(JsonValueScope &jv, const td_api::unconfirmedSession &object) {
  auto jo = jv.enter_object();
  jo("@type", "unconfirmedSession");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("log_in_date", object.log_in_date_);
  jo("device_model", object.device_model_);
  jo("location", object.location_);
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftColors &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftColors");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("model_custom_emoji_id", ToJson(JsonInt64{object.model_custom_emoji_id_}));
  jo("symbol_custom_emoji_id", ToJson(JsonInt64{object.symbol_custom_emoji_id_}));
  jo("light_theme_accent_color", object.light_theme_accent_color_);
  jo("light_theme_colors", ToJson(object.light_theme_colors_));
  jo("dark_theme_accent_color", object.dark_theme_accent_color_);
  jo("dark_theme_colors", ToJson(object.dark_theme_colors_));
}

void to_json(JsonValueScope &jv, const td_api::userFullInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "userFullInfo");
  if (object.personal_photo_) {
    jo("personal_photo", ToJson(*object.personal_photo_));
  }
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.public_photo_) {
    jo("public_photo", ToJson(*object.public_photo_));
  }
  if (object.block_list_) {
    jo("block_list", ToJson(*object.block_list_));
  }
  jo("can_be_called", JsonBool{object.can_be_called_});
  jo("supports_video_calls", JsonBool{object.supports_video_calls_});
  jo("has_private_calls", JsonBool{object.has_private_calls_});
  jo("has_private_forwards", JsonBool{object.has_private_forwards_});
  jo("has_restricted_voice_and_video_note_messages", JsonBool{object.has_restricted_voice_and_video_note_messages_});
  jo("has_posted_to_profile_stories", JsonBool{object.has_posted_to_profile_stories_});
  jo("has_sponsored_messages_enabled", JsonBool{object.has_sponsored_messages_enabled_});
  jo("need_phone_number_privacy_exception", JsonBool{object.need_phone_number_privacy_exception_});
  jo("set_chat_background", JsonBool{object.set_chat_background_});
  if (object.bio_) {
    jo("bio", ToJson(*object.bio_));
  }
  if (object.birthdate_) {
    jo("birthdate", ToJson(*object.birthdate_));
  }
  jo("personal_chat_id", object.personal_chat_id_);
  jo("gift_count", object.gift_count_);
  jo("group_in_common_count", object.group_in_common_count_);
  jo("incoming_paid_message_star_count", object.incoming_paid_message_star_count_);
  jo("outgoing_paid_message_star_count", object.outgoing_paid_message_star_count_);
  if (object.gift_settings_) {
    jo("gift_settings", ToJson(*object.gift_settings_));
  }
  if (object.bot_verification_) {
    jo("bot_verification", ToJson(*object.bot_verification_));
  }
  if (object.main_profile_tab_) {
    jo("main_profile_tab", ToJson(*object.main_profile_tab_));
  }
  if (object.first_profile_audio_) {
    jo("first_profile_audio", ToJson(*object.first_profile_audio_));
  }
  if (object.rating_) {
    jo("rating", ToJson(*object.rating_));
  }
  if (object.pending_rating_) {
    jo("pending_rating", ToJson(*object.pending_rating_));
  }
  jo("pending_rating_date", object.pending_rating_date_);
  if (object.note_) {
    jo("note", ToJson(*object.note_));
  }
  if (object.business_info_) {
    jo("business_info", ToJson(*object.business_info_));
  }
  if (object.bot_info_) {
    jo("bot_info", ToJson(*object.bot_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::usernames &object) {
  auto jo = jv.enter_object();
  jo("@type", "usernames");
  jo("active_usernames", ToJson(object.active_usernames_));
  jo("disabled_usernames", ToJson(object.disabled_usernames_));
  jo("editable_username", object.editable_username_);
}

void to_json(JsonValueScope &jv, const td_api::videoChatStreams &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoChatStreams");
  jo("streams", ToJson(object.streams_));
}

void to_json(JsonValueScope &jv, const td_api::webPageInstantView &object) {
  auto jo = jv.enter_object();
  jo("@type", "webPageInstantView");
  jo("page_blocks", ToJson(object.page_blocks_));
  jo("view_count", object.view_count_);
  jo("version", object.version_);
  jo("is_rtl", JsonBool{object.is_rtl_});
  jo("is_full", JsonBool{object.is_full_});
  if (object.feedback_link_) {
    jo("feedback_link", ToJson(*object.feedback_link_));
  }
}

}  // namespace td_api
}  // namespace td
