#include "td/telegram/td_api_json.h"

#include "td/telegram/td_api.h"
#include "td/telegram/td_api.hpp"

#include "td/tl/tl_json.h"

#include "td/utils/base64.h"
#include "td/utils/common.h"
#include "td/utils/FlatHashMap.h"
#include "td/utils/Slice.h"

#include <functional>

namespace td {
namespace td_api {

void to_json(JsonValueScope &jv, const tl_object_ptr<Object> &value) {
  td::to_json(jv, std::move(value));
}

Status from_json(tl_object_ptr<Function> &to, td::JsonValue from) {
  return td::from_json(to, std::move(from));
}

template <class T>
auto lazy_to_json(JsonValueScope &jv, const T &t) -> decltype(td_api::to_json(jv, t)) {
  return td_api::to_json(jv, t);
}

template <class T>
void lazy_to_json(std::reference_wrapper<JsonValueScope>, const T &t) {
  UNREACHABLE();
}

void to_json(JsonValueScope &jv, const Object &object) {
  downcast_call(const_cast<Object &>(object), [&jv](const auto &object) { lazy_to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const Function &object) {
  downcast_call(const_cast<Function &>(object), [&jv](const auto &object) { lazy_to_json(jv, object); });
}

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

Result<int32> tl_constructor_from_string(td_api::BusinessFeature *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"businessFeatureLocation", -1064304004},
    {"businessFeatureOpeningHours", 461054701},
    {"businessFeatureQuickReplies", -1674048894},
    {"businessFeatureGreetingMessage", 1789424756},
    {"businessFeatureAwayMessage", 1090119901},
    {"businessFeatureAccountLinks", 1878693646},
    {"businessFeatureStartPage", 401471457},
    {"businessFeatureBots", 275084773},
    {"businessFeatureEmojiStatus", -846282523},
    {"businessFeatureChatFolderTags", -543880918},
    {"businessFeatureUpgradedStories", -1812245550}
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

Result<int32> tl_constructor_from_string(td_api::ChatMembersFilter *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatMembersFilterContacts", 1774485671},
    {"chatMembersFilterAdministrators", -1266893796},
    {"chatMembersFilterMembers", 670504342},
    {"chatMembersFilterMention", 856419831},
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

Result<int32> tl_constructor_from_string(td_api::CollectibleItemType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"collectibleItemTypeUsername", 458680273},
    {"collectibleItemTypePhoneNumber", 1256251714}
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
    {"inlineKeyboardButtonTypeUser", 1836574114}
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

Result<int32> tl_constructor_from_string(td_api::InputInlineQueryResult *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputInlineQueryResultAnimation", -1489808874},
    {"inputInlineQueryResultArticle", 1973670156},
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

Result<int32> tl_constructor_from_string(td_api::InputMessageContent *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputMessageText", -212805484},
    {"inputMessageAnimation", -210404059},
    {"inputMessageAudio", -626786126},
    {"inputMessageDocument", 1633383097},
    {"inputMessagePaidMedia", -1274819374},
    {"inputMessagePhoto", -810129442},
    {"inputMessageSticker", 1072805625},
    {"inputMessageVideo", 615537686},
    {"inputMessageVideoNote", -714598691},
    {"inputMessageVoiceNote", 1461977004},
    {"inputMessageLocation", 648735088},
    {"inputMessageVenue", 1447926269},
    {"inputMessageContact", -982446849},
    {"inputMessageDice", 841574313},
    {"inputMessageGame", 1252944610},
    {"inputMessageInvoice", -1162047631},
    {"inputMessagePoll", -263337164},
    {"inputMessageStory", 554278243},
    {"inputMessageForwarded", 1696232440}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputMessageReplyTo *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputMessageReplyToMessage", -1033987837},
    {"inputMessageReplyToExternalMessage", -1993530582},
    {"inputMessageReplyToStory", 1370410616}
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
    {"inputPaidMediaTypeVideo", -1336673796}
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

Result<int32> tl_constructor_from_string(td_api::InputStoryAreaType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputStoryAreaTypeLocation", -1433714887},
    {"inputStoryAreaTypeFoundVenue", -1395809130},
    {"inputStoryAreaTypePreviousVenue", 1846693388},
    {"inputStoryAreaTypeSuggestedReaction", 2101826003},
    {"inputStoryAreaTypeMessage", -266607529},
    {"inputStoryAreaTypeLink", 1408441160},
    {"inputStoryAreaTypeWeather", -1212686691}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputStoryContent *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputStoryContentPhoto", -309196727},
    {"inputStoryContentVideo", 3809243}
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
    {"internalLinkTypeChatBoost", -716571328},
    {"internalLinkTypeChatFolderInvite", -1984804546},
    {"internalLinkTypeChatFolderSettings", -1073805988},
    {"internalLinkTypeChatInvite", 428621017},
    {"internalLinkTypeDefaultMessageAutoDeleteTimerSettings", 732625201},
    {"internalLinkTypeEditProfileSettings", -1022472090},
    {"internalLinkTypeGame", -260788787},
    {"internalLinkTypeInstantView", 1776607039},
    {"internalLinkTypeInvoice", -213094996},
    {"internalLinkTypeLanguagePack", -1450766996},
    {"internalLinkTypeLanguageSettings", -1340479770},
    {"internalLinkTypeMainWebApp", -1668012442},
    {"internalLinkTypeMessage", 978541650},
    {"internalLinkTypeMessageDraft", 661633749},
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
    {"internalLinkTypeStory", 1471997511},
    {"internalLinkTypeTheme", -200935417},
    {"internalLinkTypeThemeSettings", -1051903722},
    {"internalLinkTypeUnknownDeepLink", 625596379},
    {"internalLinkTypeUnsupportedProxy", -566649079},
    {"internalLinkTypeUserPhoneNumber", 273398536},
    {"internalLinkTypeUserToken", -1462248615},
    {"internalLinkTypeVideoChat", -2020149068},
    {"internalLinkTypeWebApp", 1281932045}
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

Result<int32> tl_constructor_from_string(td_api::MessageSchedulingState *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"messageSchedulingStateSendAtDate", -1485570073},
    {"messageSchedulingStateSendWhenOnline", 2092947464}
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

Result<int32> tl_constructor_from_string(td_api::MessageSender *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"messageSenderUser", -336109341},
    {"messageSenderChat", -239660751}
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

Result<int32> tl_constructor_from_string(td_api::PhoneNumberCodeType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"phoneNumberCodeTypeChange", 87144986},
    {"phoneNumberCodeTypeVerify", -1029402661},
    {"phoneNumberCodeTypeConfirmOwnership", -485404696}
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
    {"premiumFeatureMessageEffects", -723300255}
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
    {"premiumLimitTypeWeeklySentStoryCount", 40485707},
    {"premiumLimitTypeMonthlySentStoryCount", 819481475},
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

Result<int32> tl_constructor_from_string(td_api::ReactionNotificationSource *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"reactionNotificationSourceNone", 366374940},
    {"reactionNotificationSourceContacts", 555501621},
    {"reactionNotificationSourceAll", 1241689234}
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

Result<int32> tl_constructor_from_string(td_api::StarTransactionDirection *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"starTransactionDirectionIncoming", -1295335866},
    {"starTransactionDirectionOutgoing", 1854125472}
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

Result<int32> tl_constructor_from_string(td_api::StorePaymentPurpose *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"storePaymentPurposePremiumSubscription", 1263894804},
    {"storePaymentPurposeGiftedPremium", 1916846289},
    {"storePaymentPurposePremiumGiftCodes", -1527840798},
    {"storePaymentPurposePremiumGiveaway", 1302624938},
    {"storePaymentPurposeStarGiveaway", 211212441},
    {"storePaymentPurposeStars", -1803497708},
    {"storePaymentPurposeGiftedStars", 893691428}
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
    {"suggestedActionExtendPremium", -566207286},
    {"suggestedActionExtendStarSubscriptions", -47000234}
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
    {"supergroupMembersFilterMention", 947915036},
    {"supergroupMembersFilterBots", 492138918}
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
    {"targetChatChosen", -307442990},
    {"targetChatInternalLink", -579301408}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::TelegramPaymentPurpose *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"telegramPaymentPurposePremiumGiftCodes", -1637144394},
    {"telegramPaymentPurposePremiumGiveaway", -760757441},
    {"telegramPaymentPurposeStars", -495718830},
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

Result<int32> tl_constructor_from_string(td_api::TextEntityType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"textEntityTypeMention", 934535013},
    {"textEntityTypeHashtag", -1023958307},
    {"textEntityTypeCashtag", 1222915915},
    {"textEntityTypeBotCommand", -1150997581},
    {"textEntityTypeUrl", -1312762756},
    {"textEntityTypeEmailAddress", 1425545249},
    {"textEntityTypePhoneNumber", -1160140246},
    {"textEntityTypeBankCardNumber", 105986320},
    {"textEntityTypeBold", -1128210000},
    {"textEntityTypeItalic", -118253987},
    {"textEntityTypeUnderline", 792317842},
    {"textEntityTypeStrikethrough", 961529082},
    {"textEntityTypeSpoiler", 544019899},
    {"textEntityTypeCode", -974534326},
    {"textEntityTypePre", 1648958606},
    {"textEntityTypePreCode", -945325397},
    {"textEntityTypeBlockQuote", -1003999032},
    {"textEntityTypeExpandableBlockQuote", 36572261},
    {"textEntityTypeTextUrl", 445719651},
    {"textEntityTypeMentionName", -1570974289},
    {"textEntityTypeCustomEmoji", 1724820677},
    {"textEntityTypeMediaTimestamp", -1841898992}
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
    {"userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages", 338112060}
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
    {"userPrivacySettingRuleAllowPremiumUsers", 1624147265},
    {"userPrivacySettingRuleAllowUsers", 1110988334},
    {"userPrivacySettingRuleAllowChatMembers", -2048749863},
    {"userPrivacySettingRuleRestrictAll", -1406495408},
    {"userPrivacySettingRuleRestrictContacts", 1008389378},
    {"userPrivacySettingRuleRestrictUsers", 622796522},
    {"userPrivacySettingRuleRestrictChatMembers", 392530897}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::Object *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"accountTtl", 1324495492},
    {"address", -2043654342},
    {"archiveChatListSettings", 1058499236},
    {"autoDownloadSettings", 991433696},
    {"autosaveSettingsScopePrivateChats", 1395227007},
    {"autosaveSettingsScopeGroupChats", 853544526},
    {"autosaveSettingsScopeChannelChats", -499572783},
    {"autosaveSettingsScopeChat", -1632255255},
    {"backgroundFillSolid", 1010678813},
    {"backgroundFillGradient", -1839206017},
    {"backgroundFillFreeformGradient", -1145469255},
    {"backgroundTypeWallpaper", 1972128891},
    {"backgroundTypePattern", 1290213117},
    {"backgroundTypeFill", 993008684},
    {"backgroundTypeChatTheme", 1299879762},
    {"birthdate", 1644064030},
    {"blockListMain", 1352930172},
    {"blockListStories", 103323228},
    {"botCommand", -1032140601},
    {"botCommandScopeDefault", 795652779},
    {"botCommandScopeAllPrivateChats", -344889543},
    {"botCommandScopeAllGroupChats", -981088162},
    {"botCommandScopeAllChatAdministrators", 1998329169},
    {"botCommandScopeChat", -430234971},
    {"botCommandScopeChatAdministrators", 1119682126},
    {"botCommandScopeChatMember", -211380494},
    {"botMenuButton", -944407322},
    {"businessAwayMessageScheduleAlways", -910564679},
    {"businessAwayMessageScheduleOutsideOfOpeningHours", -968630506},
    {"businessAwayMessageScheduleCustom", -1967108654},
    {"businessAwayMessageSettings", 353084137},
    {"businessConnectedBot", -330241321},
    {"businessFeatureLocation", -1064304004},
    {"businessFeatureOpeningHours", 461054701},
    {"businessFeatureQuickReplies", -1674048894},
    {"businessFeatureGreetingMessage", 1789424756},
    {"businessFeatureAwayMessage", 1090119901},
    {"businessFeatureAccountLinks", 1878693646},
    {"businessFeatureStartPage", 401471457},
    {"businessFeatureBots", 275084773},
    {"businessFeatureEmojiStatus", -846282523},
    {"businessFeatureChatFolderTags", -543880918},
    {"businessFeatureUpgradedStories", -1812245550},
    {"businessGreetingMessageSettings", 1689140754},
    {"businessLocation", -1084969126},
    {"businessOpeningHours", 816603700},
    {"businessOpeningHoursInterval", -1108322732},
    {"businessRecipients", 868656909},
    {"callProblemEcho", 801116548},
    {"callProblemNoise", 1053065359},
    {"callProblemInterruptions", 1119493218},
    {"callProblemDistortedSpeech", 379960581},
    {"callProblemSilentLocal", 253652790},
    {"callProblemSilentRemote", 573634714},
    {"callProblemDropped", -1207311487},
    {"callProblemDistortedVideo", 385245706},
    {"callProblemPixelatedVideo", 2115315411},
    {"callProtocol", -1075562897},
    {"callbackQueryPayloadData", -1977729946},
    {"callbackQueryPayloadDataWithPassword", 1340266738},
    {"callbackQueryPayloadGame", 1303571512},
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
    {"chatActionCancel", 1160523958},
    {"chatAdministratorRights", 1599049796},
    {"chatAvailableReactionsAll", 694160279},
    {"chatAvailableReactionsSome", 152513153},
    {"chatEventLogFilters", -1032965711},
    {"chatFolder", -474905057},
    {"chatFolderIcon", -146104090},
    {"chatInviteLinkMember", 29156795},
    {"chatJoinRequest", 59341416},
    {"chatListMain", -400991316},
    {"chatListArchive", 362770115},
    {"chatListFolder", 385760856},
    {"chatLocation", -1566863583},
    {"chatMemberStatusCreator", -160019714},
    {"chatMemberStatusAdministrator", -70024163},
    {"chatMemberStatusMember", -32707562},
    {"chatMemberStatusRestricted", 1661432998},
    {"chatMemberStatusLeft", -5815259},
    {"chatMemberStatusBanned", -1653518666},
    {"chatMembersFilterContacts", 1774485671},
    {"chatMembersFilterAdministrators", -1266893796},
    {"chatMembersFilterMembers", 670504342},
    {"chatMembersFilterMention", 856419831},
    {"chatMembersFilterRestricted", 1256282813},
    {"chatMembersFilterBanned", -1863102648},
    {"chatMembersFilterBots", -1422567288},
    {"chatNotificationSettings", -572779825},
    {"chatPermissions", -118334855},
    {"chatPhotoSticker", -1459387485},
    {"chatPhotoStickerTypeRegularOrMask", -415147620},
    {"chatPhotoStickerTypeCustomEmoji", -266224943},
    {"collectibleItemTypeUsername", 458680273},
    {"collectibleItemTypePhoneNumber", 1256251714},
    {"contact", -1993844876},
    {"date", -277956960},
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
    {"deviceTokenHuaweiPush", 1989103142},
    {"draftMessage", 1125328749},
    {"emailAddressAuthenticationCode", -993257022},
    {"emailAddressAuthenticationAppleId", 633948265},
    {"emailAddressAuthenticationGoogleId", -19142846},
    {"emojiCategoryTypeDefault", 1188782699},
    {"emojiCategoryTypeRegularStickers", -1337484846},
    {"emojiCategoryTypeEmojiStatus", 1381282631},
    {"emojiCategoryTypeChatPhoto", 1059063081},
    {"emojiStatus", -606529994},
    {"error", -1679978726},
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
    {"fileTypeSticker", 475233385},
    {"fileTypeThumbnail", -12443298},
    {"fileTypeUnknown", -2011566768},
    {"fileTypeVideo", 1430816539},
    {"fileTypeVideoNote", -518412385},
    {"fileTypeVideoStory", -2146754143},
    {"fileTypeVoiceNote", -588681661},
    {"fileTypeWallpaper", 1854930076},
    {"firebaseAuthenticationSettingsAndroid", -1771112932},
    {"firebaseAuthenticationSettingsIos", 222930116},
    {"formattedText", -252624564},
    {"forumTopicIcon", -818765421},
    {"giveawayParameters", 1171549354},
    {"groupCallVideoQualityThumbnail", -379186304},
    {"groupCallVideoQualityMedium", 394968234},
    {"groupCallVideoQualityFull", -2125916617},
    {"inlineKeyboardButton", -372105704},
    {"inlineKeyboardButtonTypeUrl", 1130741420},
    {"inlineKeyboardButtonTypeLoginUrl", -1203413081},
    {"inlineKeyboardButtonTypeWebApp", -1767471672},
    {"inlineKeyboardButtonTypeCallback", -1127515139},
    {"inlineKeyboardButtonTypeCallbackWithPassword", 908018248},
    {"inlineKeyboardButtonTypeCallbackGame", -383429528},
    {"inlineKeyboardButtonTypeSwitchInline", 544906485},
    {"inlineKeyboardButtonTypeBuy", 1360739440},
    {"inlineKeyboardButtonTypeUser", 1836574114},
    {"inlineQueryResultsButton", -790689618},
    {"inlineQueryResultsButtonTypeStartBot", -23400235},
    {"inlineQueryResultsButtonTypeWebApp", -1197382814},
    {"inputBackgroundLocal", -1747094364},
    {"inputBackgroundRemote", -274976231},
    {"inputBackgroundPrevious", -351905954},
    {"inputBusinessChatLink", 237858296},
    {"inputBusinessStartPage", -327383072},
    {"inputChatPhotoPrevious", 23128529},
    {"inputChatPhotoStatic", 1979179699},
    {"inputChatPhotoAnimation", 90846242},
    {"inputChatPhotoSticker", 1315861341},
    {"inputCredentialsSaved", -2034385364},
    {"inputCredentialsNew", -829689558},
    {"inputCredentialsApplePay", -1246570799},
    {"inputCredentialsGooglePay", 844384100},
    {"inputFileId", 1788906253},
    {"inputFileRemote", -107574466},
    {"inputFileLocal", 2056030919},
    {"inputFileGenerated", -1333385216},
    {"inputIdentityDocument", 767353688},
    {"inputInlineQueryResultAnimation", -1489808874},
    {"inputInlineQueryResultArticle", 1973670156},
    {"inputInlineQueryResultAudio", 1260139988},
    {"inputInlineQueryResultContact", 1846064594},
    {"inputInlineQueryResultDocument", 578801869},
    {"inputInlineQueryResultGame", 966074327},
    {"inputInlineQueryResultLocation", -1887650218},
    {"inputInlineQueryResultPhoto", -1123338721},
    {"inputInlineQueryResultSticker", 274007129},
    {"inputInlineQueryResultVenue", 541704509},
    {"inputInlineQueryResultVideo", 1724073191},
    {"inputInlineQueryResultVoiceNote", -1790072503},
    {"inputInvoiceMessage", 1490872848},
    {"inputInvoiceName", -1312155917},
    {"inputInvoiceTelegram", -1762853139},
    {"inputMessageText", -212805484},
    {"inputMessageAnimation", -210404059},
    {"inputMessageAudio", -626786126},
    {"inputMessageDocument", 1633383097},
    {"inputMessagePaidMedia", -1274819374},
    {"inputMessagePhoto", -810129442},
    {"inputMessageSticker", 1072805625},
    {"inputMessageVideo", 615537686},
    {"inputMessageVideoNote", -714598691},
    {"inputMessageVoiceNote", 1461977004},
    {"inputMessageLocation", 648735088},
    {"inputMessageVenue", 1447926269},
    {"inputMessageContact", -982446849},
    {"inputMessageDice", 841574313},
    {"inputMessageGame", 1252944610},
    {"inputMessageInvoice", -1162047631},
    {"inputMessagePoll", -263337164},
    {"inputMessageStory", 554278243},
    {"inputMessageForwarded", 1696232440},
    {"inputMessageReplyToMessage", -1033987837},
    {"inputMessageReplyToExternalMessage", -1993530582},
    {"inputMessageReplyToStory", 1370410616},
    {"inputPaidMedia", 475844035},
    {"inputPaidMediaTypePhoto", -761660134},
    {"inputPaidMediaTypeVideo", -1336673796},
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
    {"inputPassportElementEmailAddress", -248605659},
    {"inputPassportElementError", 285756898},
    {"inputPassportElementErrorSourceUnspecified", 267230319},
    {"inputPassportElementErrorSourceDataField", -426795002},
    {"inputPassportElementErrorSourceFrontSide", 588023741},
    {"inputPassportElementErrorSourceReverseSide", 413072891},
    {"inputPassportElementErrorSourceSelfie", -773575528},
    {"inputPassportElementErrorSourceTranslationFile", 505842299},
    {"inputPassportElementErrorSourceTranslationFiles", -527254048},
    {"inputPassportElementErrorSourceFile", -298492469},
    {"inputPassportElementErrorSourceFiles", -2008541640},
    {"inputPersonalDocument", 1676966826},
    {"inputSticker", 1589392402},
    {"inputStoryArea", 122859135},
    {"inputStoryAreaTypeLocation", -1433714887},
    {"inputStoryAreaTypeFoundVenue", -1395809130},
    {"inputStoryAreaTypePreviousVenue", 1846693388},
    {"inputStoryAreaTypeSuggestedReaction", 2101826003},
    {"inputStoryAreaTypeMessage", -266607529},
    {"inputStoryAreaTypeLink", 1408441160},
    {"inputStoryAreaTypeWeather", -1212686691},
    {"inputStoryAreas", -883247088},
    {"inputStoryContentPhoto", -309196727},
    {"inputStoryContentVideo", 3809243},
    {"inputTextQuote", -1219859172},
    {"inputThumbnail", 1582387236},
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
    {"internalLinkTypeChatBoost", -716571328},
    {"internalLinkTypeChatFolderInvite", -1984804546},
    {"internalLinkTypeChatFolderSettings", -1073805988},
    {"internalLinkTypeChatInvite", 428621017},
    {"internalLinkTypeDefaultMessageAutoDeleteTimerSettings", 732625201},
    {"internalLinkTypeEditProfileSettings", -1022472090},
    {"internalLinkTypeGame", -260788787},
    {"internalLinkTypeInstantView", 1776607039},
    {"internalLinkTypeInvoice", -213094996},
    {"internalLinkTypeLanguagePack", -1450766996},
    {"internalLinkTypeLanguageSettings", -1340479770},
    {"internalLinkTypeMainWebApp", -1668012442},
    {"internalLinkTypeMessage", 978541650},
    {"internalLinkTypeMessageDraft", 661633749},
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
    {"internalLinkTypeStory", 1471997511},
    {"internalLinkTypeTheme", -200935417},
    {"internalLinkTypeThemeSettings", -1051903722},
    {"internalLinkTypeUnknownDeepLink", 625596379},
    {"internalLinkTypeUnsupportedProxy", -566649079},
    {"internalLinkTypeUserPhoneNumber", 273398536},
    {"internalLinkTypeUserToken", -1462248615},
    {"internalLinkTypeVideoChat", -2020149068},
    {"internalLinkTypeWebApp", 1281932045},
    {"invoice", 1039926674},
    {"jsonObjectMember", -1803309418},
    {"jsonValueNull", -92872499},
    {"jsonValueBoolean", -2142186576},
    {"jsonValueNumber", -1010822033},
    {"jsonValueString", 1597947313},
    {"jsonValueArray", -183913546},
    {"jsonValueObject", 520252026},
    {"keyboardButton", -2069836172},
    {"keyboardButtonTypeText", -1773037256},
    {"keyboardButtonTypeRequestPhoneNumber", -1529235527},
    {"keyboardButtonTypeRequestLocation", -125661955},
    {"keyboardButtonTypeRequestPoll", 1902435512},
    {"keyboardButtonTypeRequestUsers", -1738765315},
    {"keyboardButtonTypeRequestChat", 1511138485},
    {"keyboardButtonTypeWebApp", 1892220770},
    {"labeledPricePart", 552789798},
    {"languagePackInfo", 542199642},
    {"languagePackString", 1307632736},
    {"languagePackStringValueOrdinary", -249256352},
    {"languagePackStringValuePluralized", 1906840261},
    {"languagePackStringValueDeleted", 1834792698},
    {"linkPreviewOptions", 1046590451},
    {"location", -443392141},
    {"locationAddress", -1545940190},
    {"logStreamDefault", 1390581436},
    {"logStreamFile", 1532136933},
    {"logStreamEmpty", -499912244},
    {"maskPointForehead", 1027512005},
    {"maskPointEyes", 1748310861},
    {"maskPointMouth", 411773406},
    {"maskPointChin", 534995335},
    {"maskPosition", -2097433026},
    {"messageAutoDeleteTime", 1972045589},
    {"messageCopyOptions", 1079772090},
    {"messageSchedulingStateSendAtDate", -1485570073},
    {"messageSchedulingStateSendWhenOnline", 2092947464},
    {"messageSelfDestructTypeTimer", 1351440333},
    {"messageSelfDestructTypeImmediately", -1036218363},
    {"messageSendOptions", -2105243218},
    {"messageSenderUser", -336109341},
    {"messageSenderChat", -239660751},
    {"messageSourceChatHistory", -1090386116},
    {"messageSourceMessageThreadHistory", 290427142},
    {"messageSourceForumTopicHistory", -1518064457},
    {"messageSourceHistoryPreview", 1024254993},
    {"messageSourceChatList", -2047406102},
    {"messageSourceSearch", 1921333105},
    {"messageSourceChatEventLog", -1028777540},
    {"messageSourceNotification", -1046406163},
    {"messageSourceScreenshot", 469982474},
    {"messageSourceOther", 901818114},
    {"networkStatisticsEntryFile", 188452706},
    {"networkStatisticsEntryCall", 737000365},
    {"networkTypeNone", -1971691759},
    {"networkTypeMobile", 819228239},
    {"networkTypeMobileRoaming", -1435199760},
    {"networkTypeWiFi", -633872070},
    {"networkTypeOther", 1942128539},
    {"newChatPrivacySettings", 1528154694},
    {"notificationSettingsScopePrivateChats", 937446759},
    {"notificationSettingsScopeGroupChats", 1212142067},
    {"notificationSettingsScopeChannelChats", 548013448},
    {"optionValueBoolean", 63135518},
    {"optionValueEmpty", 918955155},
    {"optionValueInteger", -186858780},
    {"optionValueString", 756248212},
    {"orderInfo", 783997294},
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
    {"passportElementTypeEmailAddress", -79321405},
    {"personalDetails", -1061656137},
    {"phoneNumberAuthenticationSettings", 1881885547},
    {"phoneNumberCodeTypeChange", 87144986},
    {"phoneNumberCodeTypeVerify", -1029402661},
    {"phoneNumberCodeTypeConfirmOwnership", -485404696},
    {"pollTypeRegular", 641265698},
    {"pollTypeQuiz", 657013913},
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
    {"premiumLimitTypeWeeklySentStoryCount", 40485707},
    {"premiumLimitTypeMonthlySentStoryCount", 819481475},
    {"premiumLimitTypeStoryCaptionLength", -1093324030},
    {"premiumLimitTypeStorySuggestedReactionAreaCount", -1170032633},
    {"premiumLimitTypeSimilarChatCount", -1563549935},
    {"premiumSourceLimitExceeded", -2052159742},
    {"premiumSourceFeature", 445813541},
    {"premiumSourceBusinessFeature", -1492946340},
    {"premiumSourceStoryFeature", -1030737556},
    {"premiumSourceLink", 2135071132},
    {"premiumSourceSettings", -285702859},
    {"premiumStoryFeaturePriorityOrder", -1880001849},
    {"premiumStoryFeatureStealthMode", 1194605988},
    {"premiumStoryFeaturePermanentViewsHistory", -1029683296},
    {"premiumStoryFeatureCustomExpirationDuration", -593229162},
    {"premiumStoryFeatureSaveStories", -1501286467},
    {"premiumStoryFeatureLinksAndFormatting", -622623753},
    {"premiumStoryFeatureVideoQuality", -1162887511},
    {"proxyTypeSocks5", -890027341},
    {"proxyTypeHttp", -1547188361},
    {"proxyTypeMtproto", -1964826627},
    {"publicChatTypeHasUsername", 350789758},
    {"publicChatTypeIsLocationBased", 1183735952},
    {"reactionNotificationSettings", 733017684},
    {"reactionNotificationSourceNone", 366374940},
    {"reactionNotificationSourceContacts", 555501621},
    {"reactionNotificationSourceAll", 1241689234},
    {"reactionTypeEmoji", -1942084920},
    {"reactionTypeCustomEmoji", -989117709},
    {"reactionTypePaid", 436294381},
    {"readDatePrivacySettings", 1654842920},
    {"replyMarkupRemoveKeyboard", -691252879},
    {"replyMarkupForceReply", 1101461919},
    {"replyMarkupShowKeyboard", -791495984},
    {"replyMarkupInlineKeyboard", -619317658},
    {"reportReasonSpam", -1207032897},
    {"reportReasonViolence", 2038679353},
    {"reportReasonPornography", 1306467575},
    {"reportReasonChildAbuse", 761086718},
    {"reportReasonCopyright", 1474441135},
    {"reportReasonUnrelatedLocation", 87562288},
    {"reportReasonFake", 352862176},
    {"reportReasonIllegalDrugs", -61599200},
    {"reportReasonPersonalDetails", -1588882414},
    {"reportReasonCustom", -1380459917},
    {"resendCodeReasonUserRequest", -441923456},
    {"resendCodeReasonVerificationFailed", 529870273},
    {"scopeAutosaveSettings", 1546821427},
    {"scopeNotificationSettings", -599105185},
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
    {"searchMessagesFilterPinned", 371805512},
    {"shippingOption", 1425690001},
    {"starSubscriptionPricing", -1767733162},
    {"starTransactionDirectionIncoming", -1295335866},
    {"starTransactionDirectionOutgoing", 1854125472},
    {"stickerFormatWebp", -2123043040},
    {"stickerFormatTgs", 1614588662},
    {"stickerFormatWebm", -2070162097},
    {"stickerTypeRegular", 56345973},
    {"stickerTypeMask", -1765394796},
    {"stickerTypeCustomEmoji", -120752249},
    {"storePaymentPurposePremiumSubscription", 1263894804},
    {"storePaymentPurposeGiftedPremium", 1916846289},
    {"storePaymentPurposePremiumGiftCodes", -1527840798},
    {"storePaymentPurposePremiumGiveaway", 1302624938},
    {"storePaymentPurposeStarGiveaway", 211212441},
    {"storePaymentPurposeStars", -1803497708},
    {"storePaymentPurposeGiftedStars", 893691428},
    {"storyAreaPosition", -1533023124},
    {"storyFullId", 1880961525},
    {"storyListMain", -672222209},
    {"storyListArchive", -41900223},
    {"storyPrivacySettingsEveryone", 890847843},
    {"storyPrivacySettingsContacts", 50285309},
    {"storyPrivacySettingsCloseFriends", 2097122144},
    {"storyPrivacySettingsSelectedUsers", -1885772602},
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
    {"suggestedActionExtendPremium", -566207286},
    {"suggestedActionExtendStarSubscriptions", -47000234},
    {"supergroupMembersFilterRecent", 1178199509},
    {"supergroupMembersFilterContacts", -1282910856},
    {"supergroupMembersFilterAdministrators", -2097380265},
    {"supergroupMembersFilterSearch", -1696358469},
    {"supergroupMembersFilterRestricted", -1107800034},
    {"supergroupMembersFilterBanned", -1210621683},
    {"supergroupMembersFilterMention", 947915036},
    {"supergroupMembersFilterBots", 492138918},
    {"targetChatCurrent", -416689904},
    {"targetChatChosen", -307442990},
    {"targetChatInternalLink", -579301408},
    {"telegramPaymentPurposePremiumGiftCodes", -1637144394},
    {"telegramPaymentPurposePremiumGiveaway", -760757441},
    {"telegramPaymentPurposeStars", -495718830},
    {"telegramPaymentPurposeGiftedStars", -1850308042},
    {"telegramPaymentPurposeStarGiveaway", 1014604689},
    {"telegramPaymentPurposeJoinChat", -1914869880},
    {"testInt", -574804983},
    {"testString", -27891572},
    {"textEntity", -1951688280},
    {"textEntityTypeMention", 934535013},
    {"textEntityTypeHashtag", -1023958307},
    {"textEntityTypeCashtag", 1222915915},
    {"textEntityTypeBotCommand", -1150997581},
    {"textEntityTypeUrl", -1312762756},
    {"textEntityTypeEmailAddress", 1425545249},
    {"textEntityTypePhoneNumber", -1160140246},
    {"textEntityTypeBankCardNumber", 105986320},
    {"textEntityTypeBold", -1128210000},
    {"textEntityTypeItalic", -118253987},
    {"textEntityTypeUnderline", 792317842},
    {"textEntityTypeStrikethrough", 961529082},
    {"textEntityTypeSpoiler", 544019899},
    {"textEntityTypeCode", -974534326},
    {"textEntityTypePre", 1648958606},
    {"textEntityTypePreCode", -945325397},
    {"textEntityTypeBlockQuote", -1003999032},
    {"textEntityTypeExpandableBlockQuote", 36572261},
    {"textEntityTypeTextUrl", 445719651},
    {"textEntityTypeMentionName", -1570974289},
    {"textEntityTypeCustomEmoji", 1724820677},
    {"textEntityTypeMediaTimestamp", -1841898992},
    {"textParseModeMarkdown", 360073407},
    {"textParseModeHTML", 1660208627},
    {"themeParameters", -276589137},
    {"topChatCategoryUsers", 1026706816},
    {"topChatCategoryBots", -1577129195},
    {"topChatCategoryGroups", 1530056846},
    {"topChatCategoryChannels", -500825885},
    {"topChatCategoryInlineBots", 377023356},
    {"topChatCategoryWebAppBots", 100062973},
    {"topChatCategoryCalls", 356208861},
    {"topChatCategoryForwardChats", 1695922133},
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
    {"userPrivacySettingRuleAllowAll", -1967186881},
    {"userPrivacySettingRuleAllowContacts", -1892733680},
    {"userPrivacySettingRuleAllowPremiumUsers", 1624147265},
    {"userPrivacySettingRuleAllowUsers", 1110988334},
    {"userPrivacySettingRuleAllowChatMembers", -2048749863},
    {"userPrivacySettingRuleRestrictAll", -1406495408},
    {"userPrivacySettingRuleRestrictContacts", 1008389378},
    {"userPrivacySettingRuleRestrictUsers", 622796522},
    {"userPrivacySettingRuleRestrictChatMembers", 392530897},
    {"userPrivacySettingRules", 322477541},
    {"venue", 1070406393}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::Function *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"acceptCall", -646618416},
    {"acceptTermsOfService", 2130576356},
    {"activateStoryStealthMode", -1009023855},
    {"addBotMediaPreview", 1347126571},
    {"addChatFolderByInviteLink", -858593816},
    {"addChatMember", 1720144407},
    {"addChatMembers", -1675991329},
    {"addChatToList", -80523595},
    {"addContact", 1869640000},
    {"addCustomServerLanguagePack", 4492771},
    {"addFavoriteSticker", 324504799},
    {"addFileToDownloads", 867533751},
    {"addLocalMessage", -166217823},
    {"addLogMessage", 1597427692},
    {"addMessageReaction", 1419269613},
    {"addNetworkStatistics", 1264825305},
    {"addPendingPaidMessageReaction", 1716816153},
    {"addProxy", 331529432},
    {"addQuickReplyShortcutInlineQueryResultMessage", -2017449468},
    {"addQuickReplyShortcutMessage", 1058573098},
    {"addQuickReplyShortcutMessageAlbum", 1348436244},
    {"addRecentSticker", -1478109026},
    {"addRecentlyFoundChat", -1746396787},
    {"addSavedAnimation", -1538525088},
    {"addSavedNotificationSound", 1043956975},
    {"addStickerToSet", 1457266235},
    {"allowBotToSendMessages", 1776928142},
    {"answerCallbackQuery", -1153028490},
    {"answerCustomQuery", -1293603521},
    {"answerInlineQuery", 1343853844},
    {"answerPreCheckoutQuery", -1486789653},
    {"answerShippingQuery", -434601324},
    {"answerWebAppQuery", -1598776079},
    {"applyPremiumGiftCode", -1347138530},
    {"assignAppStoreTransaction", -2030892112},
    {"assignGooglePlayTransaction", -1992704860},
    {"banChatMember", -888111748},
    {"blockMessageSenderFromReplies", -1214384757},
    {"boostChat", 1945750252},
    {"canBotSendMessages", 544052364},
    {"canPurchaseFromStore", 1017811816},
    {"canSendMessageToUser", 1529489462},
    {"canSendStory", -1226825365},
    {"canTransferOwnership", 634602508},
    {"cancelDownloadFile", -1954524450},
    {"cancelPasswordReset", 940733538},
    {"cancelPreliminaryUploadFile", 823412414},
    {"cancelRecoveryEmailAddressVerification", -1516728691},
    {"changeImportedContacts", 1968207955},
    {"changeStickerSet", 449357293},
    {"checkAuthenticationBotToken", 639321206},
    {"checkAuthenticationCode", -302103382},
    {"checkAuthenticationEmailCode", -582827361},
    {"checkAuthenticationPassword", -2025698400},
    {"checkAuthenticationPasswordRecoveryCode", -603309083},
    {"checkChatFolderInviteLink", 522557851},
    {"checkChatInviteLink", -496940997},
    {"checkChatUsername", -119119344},
    {"checkCreatedPublicChatsLimit", -445546591},
    {"checkEmailAddressVerificationCode", -426386685},
    {"checkLoginEmailAddressCode", -1454244766},
    {"checkPasswordRecoveryCode", -200794600},
    {"checkPhoneNumberCode", -603626079},
    {"checkPremiumGiftCode", -1786063260},
    {"checkQuickReplyShortcutName", 2101203241},
    {"checkRecoveryEmailAddressCode", -1997039589},
    {"checkStickerSetName", -1789392642},
    {"cleanFileName", 967964667},
    {"clearAllDraftMessages", -46369573},
    {"clearAutosaveSettingsExceptions", 1475109874},
    {"clearImportedContacts", 869503298},
    {"clearRecentEmojiStatuses", -428749986},
    {"clearRecentReactions", 1298253650},
    {"clearRecentStickers", -321242684},
    {"clearRecentlyFoundChats", -285582542},
    {"clearSearchedForTags", 512017238},
    {"clickAnimatedEmojiMessage", 196179554},
    {"clickChatSponsoredMessage", -641687573},
    {"clickPremiumSubscriptionButton", -369319162},
    {"close", -1187782273},
    {"closeChat", 39749353},
    {"closeSecretChat", -471006133},
    {"closeStory", 1144852309},
    {"closeWebApp", 1755391174},
    {"commitPendingPaidMessageReactions", -171354618},
    {"confirmQrCodeAuthentication", -376199379},
    {"confirmSession", -674647009},
    {"createBasicGroupChat", 1972024548},
    {"createBusinessChatLink", -1861018304},
    {"createCall", -1104663024},
    {"createChatFolder", 1015399680},
    {"createChatFolderInviteLink", -2037911099},
    {"createChatInviteLink", 287744833},
    {"createChatSubscriptionInviteLink", 2255717},
    {"createForumTopic", -1040570140},
    {"createInvoiceLink", 216787233},
    {"createNewBasicGroupChat", 1806454709},
    {"createNewSecretChat", -620682651},
    {"createNewStickerSet", -481065727},
    {"createNewSupergroupChat", 804058822},
    {"createPrivateChat", -947758327},
    {"createSecretChat", 1930285615},
    {"createSupergroupChat", 1187475691},
    {"createTemporaryPassword", -1626509434},
    {"createVideoChat", 2124715405},
    {"deleteAccount", 1395816134},
    {"deleteAllCallMessages", -1466445325},
    {"deleteAllRevokedChatInviteLinks", 1112020698},
    {"deleteBotMediaPreviews", -1397512722},
    {"deleteBusinessChatLink", -1101895865},
    {"deleteBusinessConnectedBot", -1633976747},
    {"deleteChat", -171253666},
    {"deleteChatBackground", 320267896},
    {"deleteChatFolder", -1956364551},
    {"deleteChatFolderInviteLink", -930057858},
    {"deleteChatHistory", -1472081761},
    {"deleteChatMessagesByDate", -1639653185},
    {"deleteChatMessagesBySender", -1164235161},
    {"deleteChatReplyMarkup", 100637531},
    {"deleteCommands", 1002732586},
    {"deleteDefaultBackground", -1297814210},
    {"deleteFile", 1807653676},
    {"deleteForumTopic", 1864916152},
    {"deleteLanguagePack", -2108761026},
    {"deleteMessages", 1130090173},
    {"deletePassportElement", -1719555468},
    {"deleteProfilePhoto", 1319794625},
    {"deleteQuickReplyShortcut", -246911978},
    {"deleteQuickReplyShortcutMessages", -40522947},
    {"deleteRevokedChatInviteLink", -1859711873},
    {"deleteSavedCredentials", 826300114},
    {"deleteSavedMessagesTopicHistory", 1776237930},
    {"deleteSavedMessagesTopicMessagesByDate", 1444389},
    {"deleteSavedOrderInfo", 1629058164},
    {"deleteStickerSet", 1577745325},
    {"deleteStory", -1623871722},
    {"destroy", 685331274},
    {"disableAllSupergroupUsernames", 843511216},
    {"disableProxy", -2100095102},
    {"discardCall", -1784044162},
    {"disconnectAllWebsites", -1082985981},
    {"disconnectWebsite", -778767395},
    {"downloadFile", 1059402292},
    {"editBotMediaPreview", -2037031582},
    {"editBusinessChatLink", 1594947110},
    {"editBusinessMessageCaption", -1071562045},
    {"editBusinessMessageLiveLocation", 494972447},
    {"editBusinessMessageMedia", -60733576},
    {"editBusinessMessageReplyMarkup", 701787159},
    {"editBusinessMessageText", -1149169252},
    {"editChatFolder", 53672754},
    {"editChatFolderInviteLink", -2141872095},
    {"editChatInviteLink", 1320303996},
    {"editChatSubscriptionInviteLink", -951826989},
    {"editCustomLanguagePackInfo", 1320751257},
    {"editForumTopic", -1485402016},
    {"editInlineMessageCaption", 1409762552},
    {"editInlineMessageLiveLocation", 2134352044},
    {"editInlineMessageMedia", 23553921},
    {"editInlineMessageReplyMarkup", -67565858},
    {"editInlineMessageText", -855457307},
    {"editMessageCaption", -2020117951},
    {"editMessageLiveLocation", -1890511980},
    {"editMessageMedia", -1152678125},
    {"editMessageReplyMarkup", 332127881},
    {"editMessageSchedulingState", -1372976192},
    {"editMessageText", 196272567},
    {"editProxy", -1605883821},
    {"editQuickReplyMessage", 80517006},
    {"editStarSubscription", 2048538904},
    {"editStory", 1584013745},
    {"editStoryCover", -1423307701},
    {"enableProxy", 1494450838},
    {"endGroupCall", 573131959},
    {"endGroupCallRecording", -75799927},
    {"endGroupCallScreenSharing", -2047599540},
    {"finishFileGeneration", -1055060835},
    {"forwardMessages", 966156347},
    {"getAccountTtl", -443905161},
    {"getActiveSessions", 1119710526},
    {"getAllPassportElements", -2038945045},
    {"getAllStickerEmojis", 296562224},
    {"getAnimatedEmoji", 1065635702},
    {"getApplicationConfig", -1823144318},
    {"getApplicationDownloadLink", 112013252},
    {"getArchiveChatListSettings", -2087874976},
    {"getArchivedStickerSets", 1001931341},
    {"getAttachedStickerSets", 1302172429},
    {"getAttachmentMenuBot", 1034248699},
    {"getAuthorizationState", 1949154877},
    {"getAutoDownloadSettingsPresets", -1721088201},
    {"getAutosaveSettings", 2136207914},
    {"getAvailableChatBoostSlots", 1929898965},
    {"getBackgroundUrl", 733769682},
    {"getBankCardInfo", -1310515792},
    {"getBasicGroup", -1635174828},
    {"getBasicGroupFullInfo", -1822039253},
    {"getBlockedMessageSenders", -1931137258},
    {"getBotInfoDescription", -762841035},
    {"getBotInfoShortDescription", 1243358740},
    {"getBotMediaPreviewInfo", 1358299446},
    {"getBotMediaPreviews", 577131608},
    {"getBotName", -1707118036},
    {"getBusinessChatLinkInfo", 797670986},
    {"getBusinessChatLinks", 710287703},
    {"getBusinessConnectedBot", 911058883},
    {"getBusinessConnection", -2114706400},
    {"getBusinessFeatures", -997171199},
    {"getCallbackQueryAnswer", 116357727},
    {"getCallbackQueryMessage", -1121939086},
    {"getChat", 1866601536},
    {"getChatActiveStories", 776993781},
    {"getChatAdministrators", 1544468155},
    {"getChatArchivedStories", -1356950392},
    {"getChatAvailableMessageSenders", 1158670635},
    {"getChatBoostFeatures", -389994336},
    {"getChatBoostLevelFeatures", 1172717195},
    {"getChatBoostLink", 1458662533},
    {"getChatBoostLinkInfo", 654068572},
    {"getChatBoostStatus", -810775857},
    {"getChatBoosts", -1419859400},
    {"getChatEventLog", -1281344669},
    {"getChatFolder", 92809880},
    {"getChatFolderChatCount", 2111097790},
    {"getChatFolderChatsToLeave", -1916672337},
    {"getChatFolderDefaultIconName", 754425959},
    {"getChatFolderInviteLinks", 329079776},
    {"getChatFolderNewChats", 2123181260},
    {"getChatHistory", -799960451},
    {"getChatInviteLink", -479575555},
    {"getChatInviteLinkCounts", 890299025},
    {"getChatInviteLinkMembers", 1728376124},
    {"getChatInviteLinks", 883252396},
    {"getChatJoinRequests", -388428126},
    {"getChatListsToAddChat", 654956193},
    {"getChatMember", -792636814},
    {"getChatMessageByDate", 1062564150},
    {"getChatMessageCalendar", -2119225929},
    {"getChatMessageCount", 955746569},
    {"getChatMessagePosition", 136051911},
    {"getChatNotificationSettingsExceptions", 201199121},
    {"getChatPinnedMessage", 359865008},
    {"getChatPostedToChatPageStories", -46414037},
    {"getChatRevenueStatistics", 701995836},
    {"getChatRevenueTransactions", 1194264341},
    {"getChatRevenueWithdrawalUrl", 506595104},
    {"getChatScheduledMessages", -549638149},
    {"getChatSimilarChatCount", 1178506894},
    {"getChatSimilarChats", -1152348285},
    {"getChatSparseMessagePositions", 994389757},
    {"getChatSponsoredMessages", 1353203864},
    {"getChatStatistics", 327057816},
    {"getChatStoryInteractions", -974359690},
    {"getChats", -972768574},
    {"getChatsForChatFolderInviteLink", 1873561929},
    {"getChatsToSendStories", 586802084},
    {"getCloseFriends", -1445628722},
    {"getCollectibleItemInfo", -217797238},
    {"getCommands", 1488621559},
    {"getConnectedWebsites", -170536110},
    {"getContacts", -1417722768},
    {"getCountries", -51902050},
    {"getCountryCode", 1540593906},
    {"getCountryFlagEmoji", 981871098},
    {"getCreatedPublicChats", 710354415},
    {"getCurrentState", -1191417719},
    {"getCurrentWeather", -1965384759},
    {"getCustomEmojiReactionAnimations", 1232375250},
    {"getCustomEmojiStickers", -2127427955},
    {"getDatabaseStatistics", -1942760263},
    {"getDeepLinkInfo", 680673150},
    {"getDefaultBackgroundCustomEmojiStickers", 485910542},
    {"getDefaultChatEmojiStatuses", -1481996570},
    {"getDefaultChatPhotoCustomEmojiStickers", -376342683},
    {"getDefaultEmojiStatuses", 618946243},
    {"getDefaultMessageAutoDeleteTime", -450857574},
    {"getDefaultProfilePhotoCustomEmojiStickers", 1280041655},
    {"getDisallowedChatEmojiStatuses", -770421344},
    {"getEmojiCategories", 2139537774},
    {"getEmojiReaction", -449572388},
    {"getEmojiSuggestionsUrl", -1404101841},
    {"getExternalLink", 1586688235},
    {"getExternalLinkInfo", 1175288383},
    {"getFavoriteStickers", -338964672},
    {"getFile", 1553923406},
    {"getFileDownloadedPrefixSize", 855948589},
    {"getFileExtension", -106055372},
    {"getFileMimeType", -2073879671},
    {"getForumTopic", -442761663},
    {"getForumTopicDefaultIcons", 1479898332},
    {"getForumTopicLink", -914650933},
    {"getForumTopics", -72647334},
    {"getGameHighScores", 15746459},
    {"getGiveawayInfo", -1215852357},
    {"getGreetingStickers", 374873372},
    {"getGrossingWebAppBots", 1696779802},
    {"getGroupCall", 1468491406},
    {"getGroupCallInviteLink", 719407396},
    {"getGroupCallStreamSegment", -2077959515},
    {"getGroupCallStreams", -1619226268},
    {"getGroupsInCommon", 381539178},
    {"getImportedContactCount", -656336346},
    {"getInactiveSupergroupChats", -657720907},
    {"getInlineGameHighScores", -533107798},
    {"getInlineQueryResults", 2044524652},
    {"getInstalledBackgrounds", -1051406241},
    {"getInstalledStickerSets", 1630467830},
    {"getInternalLink", 962654640},
    {"getInternalLinkType", -1948428535},
    {"getJsonString", 663458849},
    {"getJsonValue", -1829086715},
    {"getKeywordEmojis", -1969795990},
    {"getLanguagePackInfo", 2077809320},
    {"getLanguagePackString", 150789747},
    {"getLanguagePackStrings", 1246259088},
    {"getLinkPreview", -1039572191},
    {"getLocalizationTargetInfo", 1849499526},
    {"getLogStream", 1167608667},
    {"getLogTagVerbosityLevel", 951004547},
    {"getLogTags", -254449190},
    {"getLogVerbosityLevel", 594057956},
    {"getLoginUrl", 791844305},
    {"getLoginUrlInfo", -859202125},
    {"getMainWebApp", -1098632550},
    {"getMapThumbnailFile", -152660070},
    {"getMarkdownText", 164524584},
    {"getMe", -191516033},
    {"getMenuButton", -437324736},
    {"getMessage", -1821196160},
    {"getMessageAddedReactions", 2110172754},
    {"getMessageAvailableReactions", 1994098354},
    {"getMessageEffect", -1638843116},
    {"getMessageEmbeddingCode", 1654967561},
    {"getMessageFileType", -490270764},
    {"getMessageImportConfirmationText", 390627752},
    {"getMessageLink", -984158342},
    {"getMessageLinkInfo", -700533672},
    {"getMessageLocally", -603575444},
    {"getMessageProperties", 773382571},
    {"getMessagePublicForwards", 1369285812},
    {"getMessageReadDate", -1484455101},
    {"getMessageStatistics", 1270194648},
    {"getMessageThread", 2062695998},
    {"getMessageThreadHistory", -1808411608},
    {"getMessageViewers", -1584457010},
    {"getMessages", 425299338},
    {"getNetworkStatistics", -986228706},
    {"getNewChatPrivacySettings", -1295299657},
    {"getOption", -1572495746},
    {"getOwnedStickerSets", 1493074208},
    {"getPassportAuthorizationForm", 1636107398},
    {"getPassportAuthorizationFormAvailableElements", 1068700924},
    {"getPassportElement", -1882398342},
    {"getPasswordState", -174752904},
    {"getPaymentForm", -1924172076},
    {"getPaymentReceipt", 1013758294},
    {"getPhoneNumberInfo", -1608344583},
    {"getPhoneNumberInfoSync", 547061048},
    {"getPollVoters", -1000625748},
    {"getPreferredCountryLanguage", -933049386},
    {"getPremiumFeatures", -1260640695},
    {"getPremiumGiftCodePaymentOptions", -1991099860},
    {"getPremiumLimit", 1075313898},
    {"getPremiumState", 663632610},
    {"getPremiumStickerExamples", 1399442328},
    {"getPremiumStickers", -280950192},
    {"getProxies", -95026381},
    {"getProxyLink", -1054495112},
    {"getPushReceiverId", -286505294},
    {"getReadDatePrivacySettings", 451435451},
    {"getRecentEmojiStatuses", -1371914967},
    {"getRecentInlineBots", 1437823548},
    {"getRecentStickers", -579622241},
    {"getRecentlyOpenedChats", -1924156893},
    {"getRecentlyVisitedTMeUrls", 806754961},
    {"getRecommendedChatFolders", -145540217},
    {"getRecommendedChats", -649884303},
    {"getRecoveryEmailAddress", -1594770947},
    {"getRemoteFile", 2137204530},
    {"getRepliedMessage", -641918531},
    {"getSavedAnimations", 7051032},
    {"getSavedMessagesTags", -1932105815},
    {"getSavedMessagesTopicHistory", 2011552360},
    {"getSavedMessagesTopicMessageByDate", -1050786176},
    {"getSavedNotificationSound", 459569431},
    {"getSavedNotificationSounds", -1070305368},
    {"getSavedOrderInfo", -1152016675},
    {"getScopeNotificationSettings", -995613361},
    {"getSearchedForTags", -1692716851},
    {"getSecretChat", 40599169},
    {"getStarAdAccountUrl", 1940473181},
    {"getStarGiftPaymentOptions", -500735773},
    {"getStarGiveawayPaymentOptions", -883172578},
    {"getStarPaymentOptions", 1838351940},
    {"getStarRevenueStatistics", -260356841},
    {"getStarSubscriptions", -641223956},
    {"getStarTransactions", -258541327},
    {"getStarWithdrawalUrl", -1445841134},
    {"getStatisticalGraph", 1100975515},
    {"getStickerEmojis", -1895508665},
    {"getStickerSet", 1052318659},
    {"getStickerSetName", 1039849089},
    {"getStickers", 1158058819},
    {"getStorageStatistics", -853193929},
    {"getStorageStatisticsFast", 61368066},
    {"getStory", 1903893624},
    {"getStoryAvailableReactions", 595938619},
    {"getStoryInteractions", 483475469},
    {"getStoryNotificationSettingsExceptions", 627715760},
    {"getStoryPublicForwards", 1761074363},
    {"getStoryStatistics", 982926146},
    {"getSuggestedFileName", -2049399674},
    {"getSuggestedStickerSetName", -1340995520},
    {"getSuitableDiscussionChats", 49044982},
    {"getSuitablePersonalChats", -1870357515},
    {"getSupergroup", 989663458},
    {"getSupergroupFullInfo", 1099776056},
    {"getSupergroupMembers", -570940984},
    {"getSupportName", 1302205794},
    {"getSupportUser", -1733497700},
    {"getTemporaryPasswordState", -12670830},
    {"getTextEntities", -341490693},
    {"getThemeParametersJsonString", -1850145288},
    {"getThemedChatEmojiStatuses", -76325707},
    {"getThemedEmojiStatuses", 1791346882},
    {"getTimeZones", 1340268632},
    {"getTopChats", -388410847},
    {"getTrendingStickerSets", -531085986},
    {"getUser", 1117363211},
    {"getUserChatBoosts", -1190205543},
    {"getUserFullInfo", -776823720},
    {"getUserLink", 1226839270},
    {"getUserPrivacySettingRules", -2077223311},
    {"getUserProfilePhotos", -908132798},
    {"getUserSupportInfo", 1957008133},
    {"getVideoChatAvailableParticipants", -1000496379},
    {"getVideoChatRtmpUrl", 1210784543},
    {"getWebAppLinkUrl", 1326379980},
    {"getWebAppUrl", 1690578110},
    {"getWebPageInstantView", -1962649975},
    {"hideContactCloseBirthdays", -1163065221},
    {"hideSuggestedAction", -1561384065},
    {"importContacts", -215132767},
    {"importMessages", -1864116784},
    {"inviteGroupCallParticipants", 1867097679},
    {"joinChat", 326769313},
    {"joinChatByInviteLink", -1049973882},
    {"joinGroupCall", -1043773467},
    {"launchPrepaidGiveaway", 639465530},
    {"leaveChat", -1825080735},
    {"leaveGroupCall", 980152233},
    {"loadActiveStories", 2106390328},
    {"loadChats", -1885635205},
    {"loadGroupCallParticipants", 938720974},
    {"loadQuickReplyShortcutMessages", -46092588},
    {"loadQuickReplyShortcuts", -1016614243},
    {"loadSavedMessagesTopics", 289855160},
    {"logOut", -1581923301},
    {"openChat", -323371509},
    {"openChatSimilarChat", -1884883949},
    {"openMessageContent", -739088005},
    {"openStory", -824542083},
    {"openWebApp", -1339660624},
    {"optimizeStorage", 853186759},
    {"parseMarkdown", 756366063},
    {"parseTextEntities", -1709194593},
    {"pinChatMessage", 2034719663},
    {"pingProxy", -979681103},
    {"preliminaryUploadFile", 1894239129},
    {"processChatFolderNewChats", 1498280672},
    {"processChatJoinRequest", 1004876963},
    {"processChatJoinRequests", 1048722894},
    {"processPushNotification", 786679952},
    {"rateSpeechRecognition", -287521867},
    {"readAllChatMentions", 1357558453},
    {"readAllChatReactions", 1421973357},
    {"readAllMessageThreadMentions", 1323136341},
    {"readAllMessageThreadReactions", -792975554},
    {"readChatList", -1117480790},
    {"readFilePart", 906798861},
    {"readdQuickReplyShortcutMessages", 387399566},
    {"recognizeSpeech", 1741947577},
    {"recoverAuthenticationPassword", -131001053},
    {"recoverPassword", -1524262541},
    {"refundStarPayment", -1804165035},
    {"registerDevice", 366088823},
    {"registerUser", -1012247828},
    {"removeAllFilesFromDownloads", -1186433402},
    {"removeBusinessConnectedBotFromChat", 2020766707},
    {"removeChatActionBar", -1650968070},
    {"removeContacts", 1943858054},
    {"removeFavoriteSticker", 1152945264},
    {"removeFileFromDownloads", 1460060142},
    {"removeInstalledBackground", 1346446652},
    {"removeMessageReaction", -1756934789},
    {"removeNotification", 862630734},
    {"removeNotificationGroup", 1713005454},
    {"removePendingPaidMessageReactions", 1100258555},
    {"removeProxy", 1369219847},
    {"removeRecentHashtag", -1013735260},
    {"removeRecentSticker", 1246577677},
    {"removeRecentlyFoundChat", 717340444},
    {"removeSavedAnimation", -495605479},
    {"removeSavedNotificationSound", -480032946},
    {"removeSearchedForTag", 891382730},
    {"removeStickerFromSet", 1642196644},
    {"removeTopChat", -1907876267},
    {"reorderActiveUsernames", -455399375},
    {"reorderBotActiveUsernames", -1602301664},
    {"reorderBotMediaPreviews", 630851043},
    {"reorderChatFolders", 1665299546},
    {"reorderInstalledStickerSets", 1074928158},
    {"reorderQuickReplyShortcuts", -2052799232},
    {"reorderSupergroupActiveUsernames", -1962466095},
    {"replacePrimaryChatInviteLink", 1067350941},
    {"replaceStickerInSet", -406311399},
    {"replaceVideoChatRtmpUrl", 558862304},
    {"reportAuthenticationCodeMissing", -1846555064},
    {"reportChat", -1071617544},
    {"reportChatPhoto", -646966648},
    {"reportChatSponsoredMessage", -868330562},
    {"reportMessageReactions", 919111719},
    {"reportPhoneNumberCodeMissing", -895175341},
    {"reportStory", -1688244069},
    {"reportSupergroupAntiSpamFalsePositive", -516050872},
    {"reportSupergroupSpam", -94825000},
    {"requestAuthenticationPasswordRecovery", 1393896118},
    {"requestPasswordRecovery", -13777582},
    {"requestQrCodeAuthentication", 1363496527},
    {"resendAuthenticationCode", -1506755656},
    {"resendEmailAddressVerificationCode", -1872416732},
    {"resendLoginEmailAddressCode", 292966933},
    {"resendMessages", -2010327226},
    {"resendPhoneNumberCode", 1808704551},
    {"resendRecoveryEmailAddressCode", 433483548},
    {"resetAllNotificationSettings", -174020359},
    {"resetAuthenticationEmailAddress", -415075796},
    {"resetInstalledBackgrounds", 1884553559},
    {"resetNetworkStatistics", 1646452102},
    {"resetPassword", -593589091},
    {"reuseStarSubscription", 778531905},
    {"revokeChatInviteLink", -776514135},
    {"revokeGroupCallInviteLink", 501589140},
    {"saveApplicationLogEvent", -811154930},
    {"searchBackground", -2130996959},
    {"searchCallMessages", -1942229221},
    {"searchChatMembers", -445823291},
    {"searchChatMessages", -539052602},
    {"searchChatRecentLocationMessages", 950238950},
    {"searchChats", -1879787060},
    {"searchChatsNearby", -196753377},
    {"searchChatsOnServer", -1158402188},
    {"searchContacts", -1794690715},
    {"searchEmojis", -1456187668},
    {"searchFileDownloads", 706611286},
    {"searchHashtags", 1043637617},
    {"searchInstalledStickerSets", 2120122276},
    {"searchMessages", 838452169},
    {"searchOutgoingDocumentMessages", -1071397762},
    {"searchPublicChat", 857135533},
    {"searchPublicChats", 970385337},
    {"searchPublicMessagesByTag", 630680746},
    {"searchPublicStoriesByLocation", 1596709256},
    {"searchPublicStoriesByTag", -1437593502},
    {"searchPublicStoriesByVenue", -686136790},
    {"searchQuote", 1751384351},
    {"searchRecentlyFoundChats", 1647445393},
    {"searchSavedMessages", -1969512554},
    {"searchSecretMessages", -852865892},
    {"searchStickerSet", 1157930222},
    {"searchStickerSets", 262801004},
    {"searchStickers", -1709577973},
    {"searchStringsByPrefix", -2023251463},
    {"searchUserByPhoneNumber", -343757368},
    {"searchUserByToken", -666766282},
    {"searchWebApp", -1241740747},
    {"sendAuthenticationFirebaseSms", 364994111},
    {"sendBotStartMessage", -1435877650},
    {"sendBusinessMessage", 159888387},
    {"sendBusinessMessageAlbum", 788608366},
    {"sendCallDebugInformation", 2019243839},
    {"sendCallLog", 1057638353},
    {"sendCallRating", -1402719502},
    {"sendCallSignalingData", 1412280732},
    {"sendChatAction", -2010910050},
    {"sendCustomRequest", 285045153},
    {"sendEmailAddressVerificationCode", -221621379},
    {"sendInlineQueryResultMessage", 1182553208},
    {"sendMessage", -533921303},
    {"sendMessageAlbum", -1985013029},
    {"sendPassportAuthorizationForm", 652160701},
    {"sendPaymentForm", -965855094},
    {"sendPhoneNumberCode", 1084112144},
    {"sendPhoneNumberFirebaseSms", 261910660},
    {"sendQuickReplyShortcutMessages", 232068765},
    {"sendStory", -424987902},
    {"sendWebAppCustomRequest", 922705352},
    {"sendWebAppData", -1423978996},
    {"setAccentColor", 1669974841},
    {"setAccountTtl", 701389032},
    {"setAlarm", -873497067},
    {"setApplicationVerificationToken", 927248261},
    {"setArchiveChatListSettings", -884650998},
    {"setAuthenticationEmailAddress", 1773323522},
    {"setAuthenticationPhoneNumber", 868276259},
    {"setAutoDownloadSettings", -353671948},
    {"setAutosaveSettings", 6846656},
    {"setBio", -1619582124},
    {"setBirthdate", 1319755160},
    {"setBotInfoDescription", 693574984},
    {"setBotInfoShortDescription", 982956771},
    {"setBotName", -761922959},
    {"setBotProfilePhoto", -1115272346},
    {"setBotUpdatesStatus", -1154926191},
    {"setBusinessAwayMessageSettings", 1232357484},
    {"setBusinessConnectedBot", -1393459472},
    {"setBusinessGreetingMessageSettings", -873120707},
    {"setBusinessLocation", -344717547},
    {"setBusinessMessageIsPinned", -15403536},
    {"setBusinessOpeningHours", -462379918},
    {"setBusinessStartPage", -1628616290},
    {"setChatAccentColor", 882857930},
    {"setChatActiveStoriesList", -521970415},
    {"setChatAvailableReactions", 267075078},
    {"setChatBackground", 246727678},
    {"setChatClientData", -827119811},
    {"setChatDescription", 1957213277},
    {"setChatDiscussionGroup", -918801736},
    {"setChatDraftMessage", 1683889946},
    {"setChatEmojiStatus", 1434982674},
    {"setChatLocation", -767091286},
    {"setChatMemberStatus", 81794847},
    {"setChatMessageAutoDeleteTime", -1505643265},
    {"setChatMessageSender", -1421513858},
    {"setChatNotificationSettings", 777199614},
    {"setChatPermissions", 2138507006},
    {"setChatPhoto", -377778941},
    {"setChatPinnedStories", -669062355},
    {"setChatProfileAccentColor", 1109896826},
    {"setChatSlowModeDelay", -540350914},
    {"setChatTheme", -1895234925},
    {"setChatTitle", 164282047},
    {"setCloseFriends", -1908013258},
    {"setCommands", -907165606},
    {"setCustomEmojiStickerSetThumbnail", -1122836246},
    {"setCustomLanguagePack", -296742819},
    {"setCustomLanguagePackString", 1316365592},
    {"setDatabaseEncryptionKey", -1204599371},
    {"setDefaultBackground", -1982748511},
    {"setDefaultChannelAdministratorRights", -234004967},
    {"setDefaultGroupAdministratorRights", 1700231016},
    {"setDefaultMessageAutoDeleteTime", -1772301460},
    {"setDefaultReactionType", 1694730813},
    {"setEmojiStatus", -1829224867},
    {"setFileGenerationProgress", 1836403518},
    {"setForumTopicNotificationSettings", 524498023},
    {"setGameScore", 2127359430},
    {"setGroupCallParticipantIsSpeaking", 927506917},
    {"setGroupCallParticipantVolumeLevel", -1753769944},
    {"setGroupCallTitle", -1228825139},
    {"setInactiveSessionTtl", 1570548048},
    {"setInlineGameScore", -948871797},
    {"setLocation", 93926257},
    {"setLogStream", -1364199535},
    {"setLogTagVerbosityLevel", -2095589738},
    {"setLogVerbosityLevel", -303429678},
    {"setLoginEmailAddress", 935019476},
    {"setMenuButton", -1269841599},
    {"setMessageFactCheck", -4309752},
    {"setMessageReactions", -372524900},
    {"setMessageSenderBlockList", -1987355503},
    {"setName", 1711693584},
    {"setNetworkType", -701635234},
    {"setNewChatPrivacySettings", 1774139215},
    {"setOption", 2114670322},
    {"setPassportElement", 2068173212},
    {"setPassportElementErrors", -2056754881},
    {"setPassword", -1193589027},
    {"setPersonalChat", -1068782668},
    {"setPinnedChats", -695640000},
    {"setPinnedForumTopics", -475084011},
    {"setPinnedSavedMessagesTopics", -194818924},
    {"setPollAnswer", -1399388792},
    {"setProfileAccentColor", -1986281112},
    {"setProfilePhoto", -2048260627},
    {"setQuickReplyShortcutName", 186709105},
    {"setReactionNotificationSettings", 1186124949},
    {"setReadDatePrivacySettings", 493913782},
    {"setRecoveryEmailAddress", -1981836385},
    {"setSavedMessagesTagLabel", -1338323696},
    {"setScopeNotificationSettings", -2049984966},
    {"setStickerEmojis", -638843855},
    {"setStickerKeywords", 137223565},
    {"setStickerMaskPosition", 1202280912},
    {"setStickerPositionInSet", 2075281185},
    {"setStickerSetThumbnail", 1677617458},
    {"setStickerSetTitle", 1693004706},
    {"setStoryPrivacySettings", -655801550},
    {"setStoryReaction", -1400156249},
    {"setSupergroupCustomEmojiStickerSet", 1328894639},
    {"setSupergroupStickerSet", -2056344215},
    {"setSupergroupUnrestrictBoostCount", 969814179},
    {"setSupergroupUsername", 1346325252},
    {"setTdlibParameters", -775883218},
    {"setUserPersonalProfilePhoto", 464136438},
    {"setUserPrivacySettingRules", -473812741},
    {"setUserSupportInfo", -2088986621},
    {"setUsername", 439901214},
    {"setVideoChatDefaultParticipant", -240749901},
    {"shareChatWithBot", -1504507166},
    {"sharePhoneNumber", 1097130069},
    {"shareUsersWithBot", -1574608333},
    {"startGroupCallRecording", 1757774971},
    {"startGroupCallScreenSharing", -884068051},
    {"startScheduledGroupCall", 1519938838},
    {"stopBusinessPoll", -1142218400},
    {"stopPoll", 1659374253},
    {"suggestUserProfilePhoto", -1788742557},
    {"synchronizeLanguagePack", -2065307858},
    {"terminateAllOtherSessions", 1874485523},
    {"terminateSession", -407385812},
    {"testCallBytes", -736011607},
    {"testCallEmpty", -627291626},
    {"testCallString", -1732818385},
    {"testCallVectorInt", -2137277793},
    {"testCallVectorIntObject", 1825428218},
    {"testCallVectorString", -408600900},
    {"testCallVectorStringObject", 1527666429},
    {"testGetDifference", 1747084069},
    {"testNetwork", -1343998901},
    {"testProxy", -1197366626},
    {"testReturnError", 455179506},
    {"testSquareInt", -60135024},
    {"testUseUpdate", 717094686},
    {"toggleAllDownloadsArePaused", 1251512322},
    {"toggleBotIsAddedToAttachmentMenu", -1906712934},
    {"toggleBotUsernameIsActive", 2036569097},
    {"toggleBusinessConnectedBotChatIsPaused", 1328957509},
    {"toggleChatDefaultDisableNotification", 314794002},
    {"toggleChatFolderTags", -2092209084},
    {"toggleChatHasProtectedContent", 975231309},
    {"toggleChatIsMarkedAsUnread", -986129697},
    {"toggleChatIsPinned", -1485429186},
    {"toggleChatIsTranslatable", -1812345889},
    {"toggleChatViewAsTopics", 724009948},
    {"toggleDownloadIsPaused", -947493099},
    {"toggleForumTopicIsClosed", -949712141},
    {"toggleForumTopicIsPinned", 1181543092},
    {"toggleGeneralForumTopicIsHidden", 1595741256},
    {"toggleGroupCallEnabledStartNotification", 707839826},
    {"toggleGroupCallIsMyVideoEnabled", -1624289030},
    {"toggleGroupCallIsMyVideoPaused", -478875239},
    {"toggleGroupCallMuteNewParticipants", 284082626},
    {"toggleGroupCallParticipantIsHandRaised", -1896127519},
    {"toggleGroupCallParticipantIsMuted", -1308093433},
    {"toggleGroupCallScreenSharingIsPaused", -1602530464},
    {"toggleHasSponsoredMessagesEnabled", 1963285740},
    {"togglePaidMessageReactionIsAnonymous", -1753949423},
    {"toggleSavedMessagesTopicIsPinned", -1588378164},
    {"toggleSessionCanAcceptCalls", 1819027208},
    {"toggleSessionCanAcceptSecretChats", 1000843390},
    {"toggleStoryIsPostedToChatPage", -300987649},
    {"toggleSupergroupCanHaveSponsoredMessages", -1098204302},
    {"toggleSupergroupHasAggressiveAntiSpamEnabled", 1748956943},
    {"toggleSupergroupHasHiddenMembers", -1537892918},
    {"toggleSupergroupIsAllHistoryAvailable", 1155110478},
    {"toggleSupergroupIsBroadcastGroup", 884089365},
    {"toggleSupergroupIsForum", -1771071990},
    {"toggleSupergroupJoinByRequest", 2111807454},
    {"toggleSupergroupJoinToSendMessages", -182022642},
    {"toggleSupergroupSignMessages", 572268491},
    {"toggleSupergroupUsernameIsActive", -1500811777},
    {"toggleUsernameIsActive", 1244098019},
    {"transferChatOwnership", 2006977043},
    {"translateMessageText", 1405427410},
    {"translateText", 623011058},
    {"unpinAllChatMessages", -1437805385},
    {"unpinAllMessageThreadMessages", -1211719936},
    {"unpinChatMessage", 2065448670},
    {"upgradeBasicGroupChatToSupergroupChat", 300488122},
    {"uploadStickerFile", 647385283},
    {"validateOrderInfo", -1248305201},
    {"viewMessages", 960236656},
    {"viewPremiumFeature", 192950706},
    {"viewTrendingStickerSets", -952416520},
    {"writeGeneratedFilePart", 214474389}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Status from_json(td_api::accountTtl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.days_, from.extract_field("days")));
  return Status::OK();
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

Status from_json(td_api::archiveChatListSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.archive_and_mute_new_chats_from_unknown_users_, from.extract_field("archive_and_mute_new_chats_from_unknown_users")));
  TRY_STATUS(from_json(to.keep_unmuted_chats_archived_, from.extract_field("keep_unmuted_chats_archived")));
  TRY_STATUS(from_json(to.keep_chats_from_folders_archived_, from.extract_field("keep_chats_from_folders_archived")));
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

Status from_json(td_api::autosaveSettingsScopePrivateChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::autosaveSettingsScopeGroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::autosaveSettingsScopeChannelChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::autosaveSettingsScopeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillSolid &to, JsonObject &from) {
  TRY_STATUS(from_json(to.color_, from.extract_field("color")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillGradient &to, JsonObject &from) {
  TRY_STATUS(from_json(to.top_color_, from.extract_field("top_color")));
  TRY_STATUS(from_json(to.bottom_color_, from.extract_field("bottom_color")));
  TRY_STATUS(from_json(to.rotation_angle_, from.extract_field("rotation_angle")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillFreeformGradient &to, JsonObject &from) {
  TRY_STATUS(from_json(to.colors_, from.extract_field("colors")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypeWallpaper &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_blurred_, from.extract_field("is_blurred")));
  TRY_STATUS(from_json(to.is_moving_, from.extract_field("is_moving")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypePattern &to, JsonObject &from) {
  TRY_STATUS(from_json(to.fill_, from.extract_field("fill")));
  TRY_STATUS(from_json(to.intensity_, from.extract_field("intensity")));
  TRY_STATUS(from_json(to.is_inverted_, from.extract_field("is_inverted")));
  TRY_STATUS(from_json(to.is_moving_, from.extract_field("is_moving")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypeFill &to, JsonObject &from) {
  TRY_STATUS(from_json(to.fill_, from.extract_field("fill")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypeChatTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_name_, from.extract_field("theme_name")));
  return Status::OK();
}

Status from_json(td_api::birthdate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.day_, from.extract_field("day")));
  TRY_STATUS(from_json(to.month_, from.extract_field("month")));
  TRY_STATUS(from_json(to.year_, from.extract_field("year")));
  return Status::OK();
}

Status from_json(td_api::blockListMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::blockListStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::botCommand &to, JsonObject &from) {
  TRY_STATUS(from_json(to.command_, from.extract_field("command")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::botCommandScopeAllPrivateChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::botCommandScopeAllGroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::botCommandScopeAllChatAdministrators &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChatAdministrators &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::botMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageScheduleAlways &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageScheduleOutsideOfOpeningHours &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageScheduleCustom &to, JsonObject &from) {
  TRY_STATUS(from_json(to.start_date_, from.extract_field("start_date")));
  TRY_STATUS(from_json(to.end_date_, from.extract_field("end_date")));
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.recipients_, from.extract_field("recipients")));
  TRY_STATUS(from_json(to.schedule_, from.extract_field("schedule")));
  TRY_STATUS(from_json(to.offline_only_, from.extract_field("offline_only")));
  return Status::OK();
}

Status from_json(td_api::businessConnectedBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.recipients_, from.extract_field("recipients")));
  TRY_STATUS(from_json(to.can_reply_, from.extract_field("can_reply")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureOpeningHours &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureQuickReplies &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureGreetingMessage &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureAwayMessage &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureAccountLinks &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureStartPage &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureEmojiStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureChatFolderTags &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessFeatureUpgradedStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessGreetingMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.recipients_, from.extract_field("recipients")));
  TRY_STATUS(from_json(to.inactivity_days_, from.extract_field("inactivity_days")));
  return Status::OK();
}

Status from_json(td_api::businessLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::businessOpeningHours &to, JsonObject &from) {
  TRY_STATUS(from_json(to.time_zone_id_, from.extract_field("time_zone_id")));
  TRY_STATUS(from_json(to.opening_hours_, from.extract_field("opening_hours")));
  return Status::OK();
}

Status from_json(td_api::businessOpeningHoursInterval &to, JsonObject &from) {
  TRY_STATUS(from_json(to.start_minute_, from.extract_field("start_minute")));
  TRY_STATUS(from_json(to.end_minute_, from.extract_field("end_minute")));
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

Status from_json(td_api::callProblemEcho &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemNoise &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemInterruptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemDistortedSpeech &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemSilentLocal &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemSilentRemote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemDropped &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemDistortedVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::callProblemPixelatedVideo &to, JsonObject &from) {
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

Status from_json(td_api::callbackQueryPayloadData &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadDataWithPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  return Status::OK();
}

Status from_json(td_api::chatActionTyping &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionChoosingSticker &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionChoosingLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionChoosingContact &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionStartPlayingGame &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVideoNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionWatchingAnimations &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::chatActionCancel &to, JsonObject &from) {
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
  TRY_STATUS(from_json(to.is_anonymous_, from.extract_field("is_anonymous")));
  return Status::OK();
}

Status from_json(td_api::chatAvailableReactionsAll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.max_reaction_count_, from.extract_field("max_reaction_count")));
  return Status::OK();
}

Status from_json(td_api::chatAvailableReactionsSome &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reactions_, from.extract_field("reactions")));
  TRY_STATUS(from_json(to.max_reaction_count_, from.extract_field("max_reaction_count")));
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

Status from_json(td_api::chatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
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

Status from_json(td_api::chatFolderIcon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::chatInviteLinkMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.joined_chat_date_, from.extract_field("joined_chat_date")));
  TRY_STATUS(from_json(to.via_chat_folder_invite_link_, from.extract_field("via_chat_folder_invite_link")));
  TRY_STATUS(from_json(to.approver_user_id_, from.extract_field("approver_user_id")));
  return Status::OK();
}

Status from_json(td_api::chatJoinRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  TRY_STATUS(from_json(to.bio_, from.extract_field("bio")));
  return Status::OK();
}

Status from_json(td_api::chatListMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatListArchive &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatListFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::chatLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusCreator &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_title_, from.extract_field("custom_title")));
  TRY_STATUS(from_json(to.is_anonymous_, from.extract_field("is_anonymous")));
  TRY_STATUS(from_json(to.is_member_, from.extract_field("is_member")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusAdministrator &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_title_, from.extract_field("custom_title")));
  TRY_STATUS(from_json(to.can_be_edited_, from.extract_field("can_be_edited")));
  TRY_STATUS(from_json(to.rights_, from.extract_field("rights")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.member_until_date_, from.extract_field("member_until_date")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusRestricted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_member_, from.extract_field("is_member")));
  TRY_STATUS(from_json(to.restricted_until_date_, from.extract_field("restricted_until_date")));
  TRY_STATUS(from_json(to.permissions_, from.extract_field("permissions")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusLeft &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusBanned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.banned_until_date_, from.extract_field("banned_until_date")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterAdministrators &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterMembers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterMention &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterRestricted &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterBanned &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterBots &to, JsonObject &from) {
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
  TRY_STATUS(from_json(to.use_default_show_story_sender_, from.extract_field("use_default_show_story_sender")));
  TRY_STATUS(from_json(to.show_story_sender_, from.extract_field("show_story_sender")));
  TRY_STATUS(from_json(to.use_default_disable_pinned_message_notifications_, from.extract_field("use_default_disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.disable_pinned_message_notifications_, from.extract_field("disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.use_default_disable_mention_notifications_, from.extract_field("use_default_disable_mention_notifications")));
  TRY_STATUS(from_json(to.disable_mention_notifications_, from.extract_field("disable_mention_notifications")));
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

Status from_json(td_api::chatPhotoSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.background_fill_, from.extract_field("background_fill")));
  return Status::OK();
}

Status from_json(td_api::chatPhotoStickerTypeRegularOrMask &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_id_, from.extract_field("sticker_set_id")));
  TRY_STATUS(from_json(to.sticker_id_, from.extract_field("sticker_id")));
  return Status::OK();
}

Status from_json(td_api::chatPhotoStickerTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::collectibleItemTypeUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::collectibleItemTypePhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  return Status::OK();
}

Status from_json(td_api::contact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  TRY_STATUS(from_json(to.vcard_, from.extract_field("vcard")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::date &to, JsonObject &from) {
  TRY_STATUS(from_json(to.day_, from.extract_field("day")));
  TRY_STATUS(from_json(to.month_, from.extract_field("month")));
  TRY_STATUS(from_json(to.year_, from.extract_field("year")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenFirebaseCloudMessaging &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  TRY_STATUS(from_json(to.encrypt_, from.extract_field("encrypt")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenApplePush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, from.extract_field("is_app_sandbox")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenApplePushVoIP &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, from.extract_field("is_app_sandbox")));
  TRY_STATUS(from_json(to.encrypt_, from.extract_field("encrypt")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenWindowsPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.access_token_, from.extract_field("access_token")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenMicrosoftPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.channel_uri_, from.extract_field("channel_uri")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenMicrosoftPushVoIP &to, JsonObject &from) {
  TRY_STATUS(from_json(to.channel_uri_, from.extract_field("channel_uri")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenWebPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.endpoint_, from.extract_field("endpoint")));
  TRY_STATUS(from_json(to.p256dh_base64url_, from.extract_field("p256dh_base64url")));
  TRY_STATUS(from_json(to.auth_base64url_, from.extract_field("auth_base64url")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenSimplePush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.endpoint_, from.extract_field("endpoint")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenUbuntuPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenBlackBerryPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenTizenPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reg_id_, from.extract_field("reg_id")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenHuaweiPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  TRY_STATUS(from_json(to.encrypt_, from.extract_field("encrypt")));
  return Status::OK();
}

Status from_json(td_api::draftMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  TRY_STATUS(from_json(to.input_message_text_, from.extract_field("input_message_text")));
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationAppleId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationGoogleId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeRegularStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeEmojiStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeChatPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::emojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  TRY_STATUS(from_json(to.expiration_date_, from.extract_field("expiration_date")));
  return Status::OK();
}

Status from_json(td_api::error &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  return Status::OK();
}

Status from_json(td_api::fileTypeNone &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeAnimation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeAudio &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeDocument &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeNotificationSound &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypePhotoStory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeProfilePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSecret &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSecretThumbnail &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSecure &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSticker &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeThumbnail &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeUnknown &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVideoStory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeWallpaper &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::firebaseAuthenticationSettingsAndroid &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::firebaseAuthenticationSettingsIos &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, from.extract_field("is_app_sandbox")));
  return Status::OK();
}

Status from_json(td_api::formattedText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.entities_, from.extract_field("entities")));
  return Status::OK();
}

Status from_json(td_api::forumTopicIcon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.color_, from.extract_field("color")));
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
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

Status from_json(td_api::groupCallVideoQualityThumbnail &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::groupCallVideoQualityMedium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::groupCallVideoQualityFull &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeLoginUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.forward_text_, from.extract_field("forward_text")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallback &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallbackWithPassword &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallbackGame &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeSwitchInline &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.target_chat_, from.extract_field("target_chat")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeBuy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButtonTypeStartBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameter_, from.extract_field("parameter")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundLocal &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_, from.extract_field("background")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundRemote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_id_, from.extract_field("background_id")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundPrevious &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::inputBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::inputBusinessStartPage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoPrevious &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_photo_id_, from.extract_field("chat_photo_id")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoStatic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, from.extract_field("animation")));
  TRY_STATUS(from_json(to.main_frame_timestamp_, from.extract_field("main_frame_timestamp")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsSaved &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_credentials_id_, from.extract_field("saved_credentials_id")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsNew &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  TRY_STATUS(from_json(to.allow_save_, from.extract_field("allow_save")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsApplePay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsGooglePay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inputFileId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  return Status::OK();
}

Status from_json(td_api::inputFileRemote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  return Status::OK();
}

Status from_json(td_api::inputFileLocal &to, JsonObject &from) {
  TRY_STATUS(from_json(to.path_, from.extract_field("path")));
  return Status::OK();
}

Status from_json(td_api::inputFileGenerated &to, JsonObject &from) {
  TRY_STATUS(from_json(to.original_path_, from.extract_field("original_path")));
  TRY_STATUS(from_json(to.conversion_, from.extract_field("conversion")));
  TRY_STATUS(from_json(to.expected_size_, from.extract_field("expected_size")));
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

Status from_json(td_api::inputInlineQueryResultArticle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.hide_url_, from.extract_field("hide_url")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, from.extract_field("thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, from.extract_field("thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
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

Status from_json(td_api::inputInlineQueryResultContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.contact_, from.extract_field("contact")));
  TRY_STATUS(from_json(to.thumbnail_url_, from.extract_field("thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, from.extract_field("thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, from.extract_field("thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
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

Status from_json(td_api::inputInlineQueryResultGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
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

Status from_json(td_api::inputInlineQueryResultVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.voice_note_url_, from.extract_field("voice_note_url")));
  TRY_STATUS(from_json(to.voice_note_duration_, from.extract_field("voice_note_duration")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInvoiceMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::inputInvoiceName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::inputInvoiceTelegram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::inputMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.link_preview_options_, from.extract_field("link_preview_options")));
  TRY_STATUS(from_json(to.clear_draft_, from.extract_field("clear_draft")));
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

Status from_json(td_api::inputMessageAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.audio_, from.extract_field("audio")));
  TRY_STATUS(from_json(to.album_cover_thumbnail_, from.extract_field("album_cover_thumbnail")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.performer_, from.extract_field("performer")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  return Status::OK();
}

Status from_json(td_api::inputMessageDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.document_, from.extract_field("document")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.disable_content_type_detection_, from.extract_field("disable_content_type_detection")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
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

Status from_json(td_api::inputMessageSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.video_, from.extract_field("video")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
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

Status from_json(td_api::inputMessageVideoNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.video_note_, from.extract_field("video_note")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.length_, from.extract_field("length")));
  TRY_STATUS(from_json(to.self_destruct_type_, from.extract_field("self_destruct_type")));
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

Status from_json(td_api::inputMessageLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.live_period_, from.extract_field("live_period")));
  TRY_STATUS(from_json(to.heading_, from.extract_field("heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, from.extract_field("proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.venue_, from.extract_field("venue")));
  return Status::OK();
}

Status from_json(td_api::inputMessageContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contact_, from.extract_field("contact")));
  return Status::OK();
}

Status from_json(td_api::inputMessageDice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  TRY_STATUS(from_json(to.clear_draft_, from.extract_field("clear_draft")));
  return Status::OK();
}

Status from_json(td_api::inputMessageGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  return Status::OK();
}

Status from_json(td_api::inputMessageInvoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invoice_, from.extract_field("invoice")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  TRY_STATUS(from_json(to.photo_url_, from.extract_field("photo_url")));
  TRY_STATUS(from_json(to.photo_size_, from.extract_field("photo_size")));
  TRY_STATUS(from_json(to.photo_width_, from.extract_field("photo_width")));
  TRY_STATUS(from_json(to.photo_height_, from.extract_field("photo_height")));
  TRY_STATUS(from_json_bytes(to.payload_, from.extract_field("payload")));
  TRY_STATUS(from_json(to.provider_token_, from.extract_field("provider_token")));
  TRY_STATUS(from_json(to.provider_data_, from.extract_field("provider_data")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.paid_media_, from.extract_field("paid_media")));
  TRY_STATUS(from_json(to.paid_media_caption_, from.extract_field("paid_media_caption")));
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

Status from_json(td_api::inputMessageStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::inputMessageForwarded &to, JsonObject &from) {
  TRY_STATUS(from_json(to.from_chat_id_, from.extract_field("from_chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.in_game_share_, from.extract_field("in_game_share")));
  TRY_STATUS(from_json(to.copy_options_, from.extract_field("copy_options")));
  return Status::OK();
}

Status from_json(td_api::inputMessageReplyToMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  return Status::OK();
}

Status from_json(td_api::inputMessageReplyToExternalMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  return Status::OK();
}

Status from_json(td_api::inputMessageReplyToStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
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

Status from_json(td_api::inputPaidMediaTypePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inputPaidMediaTypeVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.supports_streaming_, from.extract_field("supports_streaming")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPersonalDetails &to, JsonObject &from) {
  TRY_STATUS(from_json(to.personal_details_, from.extract_field("personal_details")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPassport &to, JsonObject &from) {
  TRY_STATUS(from_json(to.passport_, from.extract_field("passport")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementDriverLicense &to, JsonObject &from) {
  TRY_STATUS(from_json(to.driver_license_, from.extract_field("driver_license")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementIdentityCard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.identity_card_, from.extract_field("identity_card")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementInternalPassport &to, JsonObject &from) {
  TRY_STATUS(from_json(to.internal_passport_, from.extract_field("internal_passport")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementUtilityBill &to, JsonObject &from) {
  TRY_STATUS(from_json(to.utility_bill_, from.extract_field("utility_bill")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementBankStatement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bank_statement_, from.extract_field("bank_statement")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementRentalAgreement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rental_agreement_, from.extract_field("rental_agreement")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPassportRegistration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.passport_registration_, from.extract_field("passport_registration")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementTemporaryRegistration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.temporary_registration_, from.extract_field("temporary_registration")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementError &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceUnspecified &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.element_hash_, from.extract_field("element_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceDataField &to, JsonObject &from) {
  TRY_STATUS(from_json(to.field_name_, from.extract_field("field_name")));
  TRY_STATUS(from_json_bytes(to.data_hash_, from.extract_field("data_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFrontSide &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceReverseSide &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceSelfie &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceTranslationFile &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceTranslationFiles &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_hashes_, from.extract_field("file_hashes")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFile &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFiles &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_hashes_, from.extract_field("file_hashes")));
  return Status::OK();
}

Status from_json(td_api::inputPersonalDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.files_, from.extract_field("files")));
  TRY_STATUS(from_json(to.translation_, from.extract_field("translation")));
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

Status from_json(td_api::inputStoryArea &to, JsonObject &from) {
  TRY_STATUS(from_json(to.position_, from.extract_field("position")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  TRY_STATUS(from_json(to.address_, from.extract_field("address")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeFoundVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_id_, from.extract_field("query_id")));
  TRY_STATUS(from_json(to.result_id_, from.extract_field("result_id")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypePreviousVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.venue_provider_, from.extract_field("venue_provider")));
  TRY_STATUS(from_json(to.venue_id_, from.extract_field("venue_id")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeSuggestedReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  TRY_STATUS(from_json(to.is_flipped_, from.extract_field("is_flipped")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreaTypeWeather &to, JsonObject &from) {
  TRY_STATUS(from_json(to.temperature_, from.extract_field("temperature")));
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  TRY_STATUS(from_json(to.background_color_, from.extract_field("background_color")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreas &to, JsonObject &from) {
  TRY_STATUS(from_json(to.areas_, from.extract_field("areas")));
  return Status::OK();
}

Status from_json(td_api::inputStoryContentPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, from.extract_field("added_sticker_file_ids")));
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

Status from_json(td_api::inputTextQuote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.position_, from.extract_field("position")));
  return Status::OK();
}

Status from_json(td_api::inputThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.width_, from.extract_field("width")));
  TRY_STATUS(from_json(to.height_, from.extract_field("height")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeActiveSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeAttachmentMenuBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.target_chat_, from.extract_field("target_chat")));
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_name_, from.extract_field("background_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotAddToChannel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.administrator_rights_, from.extract_field("administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotStart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.autostart_, from.extract_field("autostart")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotStartInGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.administrator_rights_, from.extract_field("administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBusinessChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_name_, from.extract_field("link_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBuyStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChangePhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatBoost &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatFolderInvite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatFolderSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatInvite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeDefaultMessageAutoDeleteTimerSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeEditProfileSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.game_short_name_, from.extract_field("game_short_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeInstantView &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.fallback_url_, from.extract_field("fallback_url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeInvoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invoice_name_, from.extract_field("invoice_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeLanguageSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMainWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.is_compact_, from.extract_field("is_compact")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMessageDraft &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.contains_link_, from.extract_field("contains_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePassportDataRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.public_key_, from.extract_field("public_key")));
  TRY_STATUS(from_json(to.nonce_, from.extract_field("nonce")));
  TRY_STATUS(from_json(to.callback_url_, from.extract_field("callback_url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePhoneNumberConfirmation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hash_, from.extract_field("hash")));
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePremiumFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePremiumGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePremiumGiftCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePrivacyAndSecuritySettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, from.extract_field("server")));
  TRY_STATUS(from_json(to.port_, from.extract_field("port")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePublicChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_username_, from.extract_field("chat_username")));
  TRY_STATUS(from_json(to.draft_text_, from.extract_field("draft_text")));
  TRY_STATUS(from_json(to.open_profile_, from.extract_field("open_profile")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeQrCodeAuthentication &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeRestorePurchases &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_name_, from.extract_field("sticker_set_name")));
  TRY_STATUS(from_json(to.expect_custom_emoji_, from.extract_field("expect_custom_emoji")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_username_, from.extract_field("story_sender_username")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_name_, from.extract_field("theme_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeThemeSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUnknownDeepLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUnsupportedProxy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUserPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.draft_text_, from.extract_field("draft_text")));
  TRY_STATUS(from_json(to.open_profile_, from.extract_field("open_profile")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUserToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_username_, from.extract_field("chat_username")));
  TRY_STATUS(from_json(to.invite_hash_, from.extract_field("invite_hash")));
  TRY_STATUS(from_json(to.is_live_stream_, from.extract_field("is_live_stream")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, from.extract_field("bot_username")));
  TRY_STATUS(from_json(to.web_app_short_name_, from.extract_field("web_app_short_name")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.is_compact_, from.extract_field("is_compact")));
  return Status::OK();
}

Status from_json(td_api::invoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.price_parts_, from.extract_field("price_parts")));
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

Status from_json(td_api::jsonObjectMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.key_, from.extract_field("key")));
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueNull &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::jsonValueBoolean &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueArray &to, JsonObject &from) {
  TRY_STATUS(from_json(to.values_, from.extract_field("values")));
  return Status::OK();
}

Status from_json(td_api::jsonValueObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.members_, from.extract_field("members")));
  return Status::OK();
}

Status from_json(td_api::keyboardButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeText &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestPoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.force_regular_, from.extract_field("force_regular")));
  TRY_STATUS(from_json(to.force_quiz_, from.extract_field("force_quiz")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.restrict_user_is_bot_, from.extract_field("restrict_user_is_bot")));
  TRY_STATUS(from_json(to.user_is_bot_, from.extract_field("user_is_bot")));
  TRY_STATUS(from_json(to.restrict_user_is_premium_, from.extract_field("restrict_user_is_premium")));
  TRY_STATUS(from_json(to.user_is_premium_, from.extract_field("user_is_premium")));
  TRY_STATUS(from_json(to.max_quantity_, from.extract_field("max_quantity")));
  TRY_STATUS(from_json(to.request_name_, from.extract_field("request_name")));
  TRY_STATUS(from_json(to.request_username_, from.extract_field("request_username")));
  TRY_STATUS(from_json(to.request_photo_, from.extract_field("request_photo")));
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

Status from_json(td_api::keyboardButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::labeledPricePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.label_, from.extract_field("label")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
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

Status from_json(td_api::languagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.key_, from.extract_field("key")));
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::languagePackStringValueOrdinary &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
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

Status from_json(td_api::languagePackStringValueDeleted &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::linkPreviewOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_disabled_, from.extract_field("is_disabled")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.force_small_media_, from.extract_field("force_small_media")));
  TRY_STATUS(from_json(to.force_large_media_, from.extract_field("force_large_media")));
  TRY_STATUS(from_json(to.show_above_text_, from.extract_field("show_above_text")));
  return Status::OK();
}

Status from_json(td_api::location &to, JsonObject &from) {
  TRY_STATUS(from_json(to.latitude_, from.extract_field("latitude")));
  TRY_STATUS(from_json(to.longitude_, from.extract_field("longitude")));
  TRY_STATUS(from_json(to.horizontal_accuracy_, from.extract_field("horizontal_accuracy")));
  return Status::OK();
}

Status from_json(td_api::locationAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  TRY_STATUS(from_json(to.state_, from.extract_field("state")));
  TRY_STATUS(from_json(to.city_, from.extract_field("city")));
  TRY_STATUS(from_json(to.street_, from.extract_field("street")));
  return Status::OK();
}

Status from_json(td_api::logStreamDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::logStreamFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.path_, from.extract_field("path")));
  TRY_STATUS(from_json(to.max_file_size_, from.extract_field("max_file_size")));
  TRY_STATUS(from_json(to.redirect_stderr_, from.extract_field("redirect_stderr")));
  return Status::OK();
}

Status from_json(td_api::logStreamEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::maskPointForehead &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::maskPointEyes &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::maskPointMouth &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::maskPointChin &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::maskPosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.point_, from.extract_field("point")));
  TRY_STATUS(from_json(to.x_shift_, from.extract_field("x_shift")));
  TRY_STATUS(from_json(to.y_shift_, from.extract_field("y_shift")));
  TRY_STATUS(from_json(to.scale_, from.extract_field("scale")));
  return Status::OK();
}

Status from_json(td_api::messageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.time_, from.extract_field("time")));
  return Status::OK();
}

Status from_json(td_api::messageCopyOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.send_copy_, from.extract_field("send_copy")));
  TRY_STATUS(from_json(to.replace_caption_, from.extract_field("replace_caption")));
  TRY_STATUS(from_json(to.new_caption_, from.extract_field("new_caption")));
  TRY_STATUS(from_json(to.new_show_caption_above_media_, from.extract_field("new_show_caption_above_media")));
  return Status::OK();
}

Status from_json(td_api::messageSchedulingStateSendAtDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.send_date_, from.extract_field("send_date")));
  return Status::OK();
}

Status from_json(td_api::messageSchedulingStateSendWhenOnline &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSelfDestructTypeTimer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.self_destruct_time_, from.extract_field("self_destruct_time")));
  return Status::OK();
}

Status from_json(td_api::messageSelfDestructTypeImmediately &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSendOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  TRY_STATUS(from_json(to.from_background_, from.extract_field("from_background")));
  TRY_STATUS(from_json(to.protect_content_, from.extract_field("protect_content")));
  TRY_STATUS(from_json(to.update_order_of_installed_sticker_sets_, from.extract_field("update_order_of_installed_sticker_sets")));
  TRY_STATUS(from_json(to.scheduling_state_, from.extract_field("scheduling_state")));
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  TRY_STATUS(from_json(to.sending_id_, from.extract_field("sending_id")));
  TRY_STATUS(from_json(to.only_preview_, from.extract_field("only_preview")));
  return Status::OK();
}

Status from_json(td_api::messageSenderUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::messageSenderChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::messageSourceChatHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceMessageThreadHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceForumTopicHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceHistoryPreview &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceChatList &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceSearch &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceChatEventLog &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceNotification &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceScreenshot &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSourceOther &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkStatisticsEntryFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_type_, from.extract_field("file_type")));
  TRY_STATUS(from_json(to.network_type_, from.extract_field("network_type")));
  TRY_STATUS(from_json(to.sent_bytes_, from.extract_field("sent_bytes")));
  TRY_STATUS(from_json(to.received_bytes_, from.extract_field("received_bytes")));
  return Status::OK();
}

Status from_json(td_api::networkStatisticsEntryCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.network_type_, from.extract_field("network_type")));
  TRY_STATUS(from_json(to.sent_bytes_, from.extract_field("sent_bytes")));
  TRY_STATUS(from_json(to.received_bytes_, from.extract_field("received_bytes")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  return Status::OK();
}

Status from_json(td_api::networkTypeNone &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkTypeMobile &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkTypeMobileRoaming &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkTypeWiFi &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkTypeOther &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::newChatPrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_new_chats_from_unknown_users_, from.extract_field("allow_new_chats_from_unknown_users")));
  return Status::OK();
}

Status from_json(td_api::notificationSettingsScopePrivateChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::notificationSettingsScopeGroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::notificationSettingsScopeChannelChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optionValueBoolean &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::optionValueEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optionValueInteger &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::optionValueString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::orderInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  TRY_STATUS(from_json(to.shipping_address_, from.extract_field("shipping_address")));
  return Status::OK();
}

Status from_json(td_api::passportElementTypePersonalDetails &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypePassport &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeDriverLicense &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeIdentityCard &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeInternalPassport &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeAddress &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeUtilityBill &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeBankStatement &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeRentalAgreement &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypePassportRegistration &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeTemporaryRegistration &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypePhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::passportElementTypeEmailAddress &to, JsonObject &from) {
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

Status from_json(td_api::phoneNumberCodeTypeChange &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::phoneNumberCodeTypeVerify &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::phoneNumberCodeTypeConfirmOwnership &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hash_, from.extract_field("hash")));
  return Status::OK();
}

Status from_json(td_api::pollTypeRegular &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_multiple_answers_, from.extract_field("allow_multiple_answers")));
  return Status::OK();
}

Status from_json(td_api::pollTypeQuiz &to, JsonObject &from) {
  TRY_STATUS(from_json(to.correct_option_id_, from.extract_field("correct_option_id")));
  TRY_STATUS(from_json(to.explanation_, from.extract_field("explanation")));
  return Status::OK();
}

Status from_json(td_api::premiumFeatureIncreasedLimits &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureIncreasedUploadFileSize &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureImprovedDownloadSpeed &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureVoiceRecognition &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureDisabledAds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureUniqueReactions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureUniqueStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureCustomEmoji &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAdvancedChatManagement &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureProfileBadge &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureEmojiStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAnimatedProfilePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureForumTopicIcon &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAppIcons &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureRealTimeChatTranslation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureUpgradedStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureChatBoost &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureAccentColor &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureBackgroundForBoth &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureSavedMessagesTags &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureMessagePrivacy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureLastSeenTimes &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureBusiness &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureMessageEffects &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeSupergroupCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypePinnedChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeCreatedPublicChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeSavedAnimationCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeFavoriteStickerCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeChatFolderCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeChatFolderChosenChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypePinnedArchivedChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypePinnedSavedMessagesTopicCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeCaptionLength &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeBioLength &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeChatFolderInviteLinkCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeShareableChatFolderCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeActiveStoryCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeWeeklySentStoryCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeMonthlySentStoryCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeStoryCaptionLength &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeStorySuggestedReactionAreaCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeSimilarChatCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumSourceLimitExceeded &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_type_, from.extract_field("limit_type")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceBusinessFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceStoryFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeaturePriorityOrder &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureStealthMode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeaturePermanentViewsHistory &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureCustomExpirationDuration &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureSaveStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureLinksAndFormatting &to, JsonObject &from) {
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

Status from_json(td_api::proxyTypeHttp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json(to.http_only_, from.extract_field("http_only")));
  return Status::OK();
}

Status from_json(td_api::proxyTypeMtproto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_, from.extract_field("secret")));
  return Status::OK();
}

Status from_json(td_api::publicChatTypeHasUsername &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::publicChatTypeIsLocationBased &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_reaction_source_, from.extract_field("message_reaction_source")));
  TRY_STATUS(from_json(to.story_reaction_source_, from.extract_field("story_reaction_source")));
  TRY_STATUS(from_json(to.sound_id_, from.extract_field("sound_id")));
  TRY_STATUS(from_json(to.show_preview_, from.extract_field("show_preview")));
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSourceNone &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSourceContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionNotificationSourceAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionTypeEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::reactionTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::reactionTypePaid &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::readDatePrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.show_read_date_, from.extract_field("show_read_date")));
  return Status::OK();
}

Status from_json(td_api::replyMarkupRemoveKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_personal_, from.extract_field("is_personal")));
  return Status::OK();
}

Status from_json(td_api::replyMarkupForceReply &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_personal_, from.extract_field("is_personal")));
  TRY_STATUS(from_json(to.input_field_placeholder_, from.extract_field("input_field_placeholder")));
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

Status from_json(td_api::replyMarkupInlineKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rows_, from.extract_field("rows")));
  return Status::OK();
}

Status from_json(td_api::reportReasonSpam &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonViolence &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonPornography &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonChildAbuse &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonCopyright &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonUnrelatedLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonFake &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonIllegalDrugs &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonPersonalDetails &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reportReasonCustom &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendCodeReasonUserRequest &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendCodeReasonVerificationFailed &to, JsonObject &from) {
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::scopeAutosaveSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.autosave_photos_, from.extract_field("autosave_photos")));
  TRY_STATUS(from_json(to.autosave_videos_, from.extract_field("autosave_videos")));
  TRY_STATUS(from_json(to.max_video_file_size_, from.extract_field("max_video_file_size")));
  return Status::OK();
}

Status from_json(td_api::scopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mute_for_, from.extract_field("mute_for")));
  TRY_STATUS(from_json(to.sound_id_, from.extract_field("sound_id")));
  TRY_STATUS(from_json(to.show_preview_, from.extract_field("show_preview")));
  TRY_STATUS(from_json(to.use_default_mute_stories_, from.extract_field("use_default_mute_stories")));
  TRY_STATUS(from_json(to.mute_stories_, from.extract_field("mute_stories")));
  TRY_STATUS(from_json(to.story_sound_id_, from.extract_field("story_sound_id")));
  TRY_STATUS(from_json(to.show_story_sender_, from.extract_field("show_story_sender")));
  TRY_STATUS(from_json(to.disable_pinned_message_notifications_, from.extract_field("disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.disable_mention_notifications_, from.extract_field("disable_mention_notifications")));
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterAnimation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterAudio &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterDocument &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterPhotoAndVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterUrl &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterChatPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVoiceAndVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterMention &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterUnreadMention &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterUnreadReaction &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterFailedToSend &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterPinned &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::shippingOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.price_parts_, from.extract_field("price_parts")));
  return Status::OK();
}

Status from_json(td_api::starSubscriptionPricing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.period_, from.extract_field("period")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::starTransactionDirectionIncoming &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::starTransactionDirectionOutgoing &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerFormatWebp &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerFormatTgs &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerFormatWebm &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerTypeRegular &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerTypeMask &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerTypeCustomEmoji &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposePremiumSubscription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_restore_, from.extract_field("is_restore")));
  TRY_STATUS(from_json(to.is_upgrade_, from.extract_field("is_upgrade")));
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposeGiftedPremium &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposePremiumGiftCodes &to, JsonObject &from) {
  TRY_STATUS(from_json(to.boosted_chat_id_, from.extract_field("boosted_chat_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposePremiumGiveaway &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
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

Status from_json(td_api::storePaymentPurposeStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposeGiftedStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
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

Status from_json(td_api::storyFullId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sender_chat_id_, from.extract_field("sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::storyListMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storyListArchive &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsEveryone &to, JsonObject &from) {
  TRY_STATUS(from_json(to.except_user_ids_, from.extract_field("except_user_ids")));
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.except_user_ids_, from.extract_field("except_user_ids")));
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsCloseFriends &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storyPrivacySettingsSelectedUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionEnableArchiveAndMuteNewChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionCheckPassword &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionCheckPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionViewChecksHint &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionConvertToBroadcastGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionSetPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_delay_, from.extract_field("authorization_delay")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionUpgradePremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionRestorePremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionSubscribeToAnnualPremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionGiftPremiumForChristmas &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionSetBirthdate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionExtendPremium &to, JsonObject &from) {
  TRY_STATUS(from_json(to.manage_premium_subscription_url_, from.extract_field("manage_premium_subscription_url")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionExtendStarSubscriptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterRecent &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterAdministrators &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterSearch &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterRestricted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterBanned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterMention &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::targetChatCurrent &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::targetChatChosen &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_user_chats_, from.extract_field("allow_user_chats")));
  TRY_STATUS(from_json(to.allow_bot_chats_, from.extract_field("allow_bot_chats")));
  TRY_STATUS(from_json(to.allow_group_chats_, from.extract_field("allow_group_chats")));
  TRY_STATUS(from_json(to.allow_channel_chats_, from.extract_field("allow_channel_chats")));
  return Status::OK();
}

Status from_json(td_api::targetChatInternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposePremiumGiftCodes &to, JsonObject &from) {
  TRY_STATUS(from_json(to.boosted_chat_id_, from.extract_field("boosted_chat_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  TRY_STATUS(from_json(to.month_count_, from.extract_field("month_count")));
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

Status from_json(td_api::telegramPaymentPurposeStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::telegramPaymentPurposeGiftedStars &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.currency_, from.extract_field("currency")));
  TRY_STATUS(from_json(to.amount_, from.extract_field("amount")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
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

Status from_json(td_api::telegramPaymentPurposeJoinChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::testInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::testString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::textEntity &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.length_, from.extract_field("length")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMention &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeHashtag &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeCashtag &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBotCommand &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeUrl &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeEmailAddress &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypePhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBankCardNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBold &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeItalic &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeUnderline &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeStrikethrough &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeSpoiler &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypePre &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypePreCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_, from.extract_field("language")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeBlockQuote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeExpandableBlockQuote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypeTextUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMentionName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMediaTimestamp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.media_timestamp_, from.extract_field("media_timestamp")));
  return Status::OK();
}

Status from_json(td_api::textParseModeMarkdown &to, JsonObject &from) {
  TRY_STATUS(from_json(to.version_, from.extract_field("version")));
  return Status::OK();
}

Status from_json(td_api::textParseModeHTML &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::themeParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_color_, from.extract_field("background_color")));
  TRY_STATUS(from_json(to.secondary_background_color_, from.extract_field("secondary_background_color")));
  TRY_STATUS(from_json(to.header_background_color_, from.extract_field("header_background_color")));
  TRY_STATUS(from_json(to.bottom_bar_background_color_, from.extract_field("bottom_bar_background_color")));
  TRY_STATUS(from_json(to.section_background_color_, from.extract_field("section_background_color")));
  TRY_STATUS(from_json(to.section_separator_color_, from.extract_field("section_separator_color")));
  TRY_STATUS(from_json(to.text_color_, from.extract_field("text_color")));
  TRY_STATUS(from_json(to.accent_text_color_, from.extract_field("accent_text_color")));
  TRY_STATUS(from_json(to.section_header_text_color_, from.extract_field("section_header_text_color")));
  TRY_STATUS(from_json(to.subtitle_text_color_, from.extract_field("subtitle_text_color")));
  TRY_STATUS(from_json(to.destructive_text_color_, from.extract_field("destructive_text_color")));
  TRY_STATUS(from_json(to.hint_color_, from.extract_field("hint_color")));
  TRY_STATUS(from_json(to.link_color_, from.extract_field("link_color")));
  TRY_STATUS(from_json(to.button_color_, from.extract_field("button_color")));
  TRY_STATUS(from_json(to.button_text_color_, from.extract_field("button_text_color")));
  return Status::OK();
}

Status from_json(td_api::topChatCategoryUsers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryGroups &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryChannels &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryInlineBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryWebAppBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryCalls &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::topChatCategoryForwardChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowProfilePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowLinkInForwardedMessages &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowBio &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowBirthdate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowChatInvites &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowCalls &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowPeerToPeerCalls &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowFindingByPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowPremiumUsers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rules_, from.extract_field("rules")));
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

Status from_json(td_api::acceptCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.protocol_, from.extract_field("protocol")));
  return Status::OK();
}

Status from_json(td_api::acceptTermsOfService &to, JsonObject &from) {
  TRY_STATUS(from_json(to.terms_of_service_id_, from.extract_field("terms_of_service_id")));
  return Status::OK();
}

Status from_json(td_api::activateStoryStealthMode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::addBotMediaPreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  return Status::OK();
}

Status from_json(td_api::addChatFolderByInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::addChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.forward_limit_, from.extract_field("forward_limit")));
  return Status::OK();
}

Status from_json(td_api::addChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::addChatToList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  return Status::OK();
}

Status from_json(td_api::addContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contact_, from.extract_field("contact")));
  TRY_STATUS(from_json(to.share_phone_number_, from.extract_field("share_phone_number")));
  return Status::OK();
}

Status from_json(td_api::addCustomServerLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::addFavoriteSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::addFileToDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.priority_, from.extract_field("priority")));
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

Status from_json(td_api::addLogMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.verbosity_level_, from.extract_field("verbosity_level")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
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

Status from_json(td_api::addNetworkStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.entry_, from.extract_field("entry")));
  return Status::OK();
}

Status from_json(td_api::addPendingPaidMessageReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.use_default_is_anonymous_, from.extract_field("use_default_is_anonymous")));
  TRY_STATUS(from_json(to.is_anonymous_, from.extract_field("is_anonymous")));
  return Status::OK();
}

Status from_json(td_api::addProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, from.extract_field("server")));
  TRY_STATUS(from_json(to.port_, from.extract_field("port")));
  TRY_STATUS(from_json(to.enable_, from.extract_field("enable")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
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

Status from_json(td_api::addQuickReplyShortcutMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_name_, from.extract_field("shortcut_name")));
  TRY_STATUS(from_json(to.reply_to_message_id_, from.extract_field("reply_to_message_id")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::addQuickReplyShortcutMessageAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_name_, from.extract_field("shortcut_name")));
  TRY_STATUS(from_json(to.reply_to_message_id_, from.extract_field("reply_to_message_id")));
  TRY_STATUS(from_json(to.input_message_contents_, from.extract_field("input_message_contents")));
  return Status::OK();
}

Status from_json(td_api::addRecentSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::addRecentlyFoundChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::addSavedAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, from.extract_field("animation")));
  return Status::OK();
}

Status from_json(td_api::addSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sound_, from.extract_field("sound")));
  return Status::OK();
}

Status from_json(td_api::addStickerToSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::allowBotToSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
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

Status from_json(td_api::answerCustomQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_query_id_, from.extract_field("custom_query_id")));
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
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

Status from_json(td_api::answerPreCheckoutQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.pre_checkout_query_id_, from.extract_field("pre_checkout_query_id")));
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::answerShippingQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shipping_query_id_, from.extract_field("shipping_query_id")));
  TRY_STATUS(from_json(to.shipping_options_, from.extract_field("shipping_options")));
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::answerWebAppQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.web_app_query_id_, from.extract_field("web_app_query_id")));
  TRY_STATUS(from_json(to.result_, from.extract_field("result")));
  return Status::OK();
}

Status from_json(td_api::applyPremiumGiftCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::assignAppStoreTransaction &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.receipt_, from.extract_field("receipt")));
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::assignGooglePlayTransaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.package_name_, from.extract_field("package_name")));
  TRY_STATUS(from_json(to.store_product_id_, from.extract_field("store_product_id")));
  TRY_STATUS(from_json(to.purchase_token_, from.extract_field("purchase_token")));
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::banChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.member_id_, from.extract_field("member_id")));
  TRY_STATUS(from_json(to.banned_until_date_, from.extract_field("banned_until_date")));
  TRY_STATUS(from_json(to.revoke_messages_, from.extract_field("revoke_messages")));
  return Status::OK();
}

Status from_json(td_api::blockMessageSenderFromReplies &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.delete_message_, from.extract_field("delete_message")));
  TRY_STATUS(from_json(to.delete_all_messages_, from.extract_field("delete_all_messages")));
  TRY_STATUS(from_json(to.report_spam_, from.extract_field("report_spam")));
  return Status::OK();
}

Status from_json(td_api::boostChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.slot_ids_, from.extract_field("slot_ids")));
  return Status::OK();
}

Status from_json(td_api::canBotSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::canPurchaseFromStore &to, JsonObject &from) {
  TRY_STATUS(from_json(to.purpose_, from.extract_field("purpose")));
  return Status::OK();
}

Status from_json(td_api::canSendMessageToUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::canSendStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::canTransferOwnership &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::cancelDownloadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.only_if_pending_, from.extract_field("only_if_pending")));
  return Status::OK();
}

Status from_json(td_api::cancelPasswordReset &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::cancelPreliminaryUploadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::cancelRecoveryEmailAddressVerification &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::changeImportedContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contacts_, from.extract_field("contacts")));
  return Status::OK();
}

Status from_json(td_api::changeStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, from.extract_field("set_id")));
  TRY_STATUS(from_json(to.is_installed_, from.extract_field("is_installed")));
  TRY_STATUS(from_json(to.is_archived_, from.extract_field("is_archived")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationBotToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationEmailCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationPasswordRecoveryCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  return Status::OK();
}

Status from_json(td_api::checkChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::checkChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::checkChatUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::checkCreatedPublicChatsLimit &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::checkEmailAddressVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkLoginEmailAddressCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkPasswordRecoveryCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  return Status::OK();
}

Status from_json(td_api::checkPhoneNumberCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkPremiumGiftCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkQuickReplyShortcutName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::checkRecoveryEmailAddressCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::checkStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::cleanFileName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_name_, from.extract_field("file_name")));
  return Status::OK();
}

Status from_json(td_api::clearAllDraftMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.exclude_secret_chats_, from.extract_field("exclude_secret_chats")));
  return Status::OK();
}

Status from_json(td_api::clearAutosaveSettingsExceptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clearImportedContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clearRecentEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clearRecentReactions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clearRecentStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  return Status::OK();
}

Status from_json(td_api::clearRecentlyFoundChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clearSearchedForTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.clear_cashtags_, from.extract_field("clear_cashtags")));
  return Status::OK();
}

Status from_json(td_api::clickAnimatedEmojiMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::clickChatSponsoredMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::clickPremiumSubscriptionButton &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::close &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::closeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::closeSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, from.extract_field("secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::closeStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::closeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.web_app_launch_id_, from.extract_field("web_app_launch_id")));
  return Status::OK();
}

Status from_json(td_api::commitPendingPaidMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::confirmQrCodeAuthentication &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::confirmSession &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  return Status::OK();
}

Status from_json(td_api::createBasicGroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, from.extract_field("basic_group_id")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::createBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_info_, from.extract_field("link_info")));
  return Status::OK();
}

Status from_json(td_api::createCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.protocol_, from.extract_field("protocol")));
  TRY_STATUS(from_json(to.is_video_, from.extract_field("is_video")));
  return Status::OK();
}

Status from_json(td_api::createChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::createChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
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

Status from_json(td_api::createChatSubscriptionInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.subscription_pricing_, from.extract_field("subscription_pricing")));
  return Status::OK();
}

Status from_json(td_api::createForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.icon_, from.extract_field("icon")));
  return Status::OK();
}

Status from_json(td_api::createInvoiceLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invoice_, from.extract_field("invoice")));
  return Status::OK();
}

Status from_json(td_api::createNewBasicGroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, from.extract_field("message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::createNewSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
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

Status from_json(td_api::createPrivateChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::createSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, from.extract_field("secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::createSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.force_, from.extract_field("force")));
  return Status::OK();
}

Status from_json(td_api::createTemporaryPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json(to.valid_for_, from.extract_field("valid_for")));
  return Status::OK();
}

Status from_json(td_api::createVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.start_date_, from.extract_field("start_date")));
  TRY_STATUS(from_json(to.is_rtmp_stream_, from.extract_field("is_rtmp_stream")));
  return Status::OK();
}

Status from_json(td_api::deleteAccount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::deleteAllCallMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteAllRevokedChatInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.creator_user_id_, from.extract_field("creator_user_id")));
  return Status::OK();
}

Status from_json(td_api::deleteBotMediaPreviews &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.file_ids_, from.extract_field("file_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::deleteBusinessConnectedBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChatBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.restore_previous_, from.extract_field("restore_previous")));
  return Status::OK();
}

Status from_json(td_api::deleteChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.leave_chat_ids_, from.extract_field("leave_chat_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::deleteChatHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.remove_from_chat_list_, from.extract_field("remove_from_chat_list")));
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteChatMessagesByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.min_date_, from.extract_field("min_date")));
  TRY_STATUS(from_json(to.max_date_, from.extract_field("max_date")));
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteChatMessagesBySender &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChatReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::deleteCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::deleteDefaultBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.for_dark_theme_, from.extract_field("for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::deleteFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::deleteForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::deleteLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::deleteMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.revoke_, from.extract_field("revoke")));
  return Status::OK();
}

Status from_json(td_api::deletePassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::deleteProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.profile_photo_id_, from.extract_field("profile_photo_id")));
  return Status::OK();
}

Status from_json(td_api::deleteQuickReplyShortcut &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  return Status::OK();
}

Status from_json(td_api::deleteQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteRevokedChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedCredentials &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::deleteSavedMessagesTopicHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedMessagesTopicMessagesByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.min_date_, from.extract_field("min_date")));
  TRY_STATUS(from_json(to.max_date_, from.extract_field("max_date")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedOrderInfo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::deleteStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::deleteStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::destroy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::disableAllSupergroupUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::disableProxy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::discardCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.is_disconnected_, from.extract_field("is_disconnected")));
  TRY_STATUS(from_json(to.duration_, from.extract_field("duration")));
  TRY_STATUS(from_json(to.is_video_, from.extract_field("is_video")));
  TRY_STATUS(from_json(to.connection_id_, from.extract_field("connection_id")));
  return Status::OK();
}

Status from_json(td_api::disconnectAllWebsites &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::disconnectWebsite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.website_id_, from.extract_field("website_id")));
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

Status from_json(td_api::editBotMediaPreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  return Status::OK();
}

Status from_json(td_api::editBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  TRY_STATUS(from_json(to.link_info_, from.extract_field("link_info")));
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

Status from_json(td_api::editBusinessMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editBusinessMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
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

Status from_json(td_api::editChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::editChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
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

Status from_json(td_api::editChatSubscriptionInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::editCustomLanguagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.info_, from.extract_field("info")));
  return Status::OK();
}

Status from_json(td_api::editForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.edit_icon_custom_emoji_, from.extract_field("edit_icon_custom_emoji")));
  TRY_STATUS(from_json(to.icon_custom_emoji_id_, from.extract_field("icon_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageCaption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.show_caption_above_media_, from.extract_field("show_caption_above_media")));
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

Status from_json(td_api::editInlineMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
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

Status from_json(td_api::editMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::editMessageSchedulingState &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.scheduling_state_, from.extract_field("scheduling_state")));
  return Status::OK();
}

Status from_json(td_api::editMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
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

Status from_json(td_api::editQuickReplyMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editStarSubscription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.subscription_id_, from.extract_field("subscription_id")));
  TRY_STATUS(from_json(to.is_canceled_, from.extract_field("is_canceled")));
  return Status::OK();
}

Status from_json(td_api::editStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  TRY_STATUS(from_json(to.areas_, from.extract_field("areas")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  return Status::OK();
}

Status from_json(td_api::editStoryCover &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.cover_frame_timestamp_, from.extract_field("cover_frame_timestamp")));
  return Status::OK();
}

Status from_json(td_api::enableProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::endGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::endGroupCallRecording &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::endGroupCallScreenSharing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::finishFileGeneration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, from.extract_field("generation_id")));
  TRY_STATUS(from_json(to.error_, from.extract_field("error")));
  return Status::OK();
}

Status from_json(td_api::forwardMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.from_chat_id_, from.extract_field("from_chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.send_copy_, from.extract_field("send_copy")));
  TRY_STATUS(from_json(to.remove_caption_, from.extract_field("remove_caption")));
  return Status::OK();
}

Status from_json(td_api::getAccountTtl &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getActiveSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAllPassportElements &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getAllStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.return_only_main_emoji_, from.extract_field("return_only_main_emoji")));
  return Status::OK();
}

Status from_json(td_api::getAnimatedEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::getApplicationConfig &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getApplicationDownloadLink &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getArchiveChatListSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getArchivedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.offset_sticker_set_id_, from.extract_field("offset_sticker_set_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getAttachedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::getAttachmentMenuBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::getAuthorizationState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAutoDownloadSettingsPresets &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAutosaveSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAvailableChatBoostSlots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBackgroundUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getBankCardInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bank_card_number_, from.extract_field("bank_card_number")));
  return Status::OK();
}

Status from_json(td_api::getBasicGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, from.extract_field("basic_group_id")));
  return Status::OK();
}

Status from_json(td_api::getBasicGroupFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, from.extract_field("basic_group_id")));
  return Status::OK();
}

Status from_json(td_api::getBlockedMessageSenders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.block_list_, from.extract_field("block_list")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getBotInfoDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getBotInfoShortDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getBotMediaPreviewInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getBotMediaPreviews &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::getBotName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getBusinessChatLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_name_, from.extract_field("link_name")));
  return Status::OK();
}

Status from_json(td_api::getBusinessChatLinks &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBusinessConnectedBot &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBusinessConnection &to, JsonObject &from) {
  TRY_STATUS(from_json(to.connection_id_, from.extract_field("connection_id")));
  return Status::OK();
}

Status from_json(td_api::getBusinessFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  return Status::OK();
}

Status from_json(td_api::getCallbackQueryAnswer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::getCallbackQueryMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.callback_query_id_, from.extract_field("callback_query_id")));
  return Status::OK();
}

Status from_json(td_api::getChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatActiveStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatAdministrators &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatArchivedStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.from_story_id_, from.extract_field("from_story_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatAvailableMessageSenders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_channel_, from.extract_field("is_channel")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostLevelFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_channel_, from.extract_field("is_channel")));
  TRY_STATUS(from_json(to.level_, from.extract_field("level")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostStatus &to, JsonObject &from) {
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

Status from_json(td_api::getChatEventLog &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.from_event_id_, from.extract_field("from_event_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filters_, from.extract_field("filters")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::getChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderChatCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderChatsToLeave &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderDefaultIconName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, from.extract_field("folder")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderNewChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
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

Status from_json(td_api::getChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLinkCounts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
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

Status from_json(td_api::getChatInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.creator_user_id_, from.extract_field("creator_user_id")));
  TRY_STATUS(from_json(to.is_revoked_, from.extract_field("is_revoked")));
  TRY_STATUS(from_json(to.offset_date_, from.extract_field("offset_date")));
  TRY_STATUS(from_json(to.offset_invite_link_, from.extract_field("offset_invite_link")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatJoinRequests &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_request_, from.extract_field("offset_request")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatListsToAddChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.member_id_, from.extract_field("member_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageCalendar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.return_local_, from.extract_field("return_local")));
  return Status::OK();
}

Status from_json(td_api::getChatMessagePosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::getChatNotificationSettingsExceptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.compare_sound_, from.extract_field("compare_sound")));
  return Status::OK();
}

Status from_json(td_api::getChatPinnedMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatPostedToChatPageStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.from_story_id_, from.extract_field("from_story_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatRevenueStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getChatRevenueTransactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatRevenueWithdrawalUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getChatScheduledMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatSimilarChatCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.return_local_, from.extract_field("return_local")));
  return Status::OK();
}

Status from_json(td_api::getChatSimilarChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
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

Status from_json(td_api::getChatSponsoredMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getChatStoryInteractions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.prefer_forwards_, from.extract_field("prefer_forwards")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatsForChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatsToSendStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCloseFriends &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCollectibleItemInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getConnectedWebsites &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCountries &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCountryCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCountryFlagEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  return Status::OK();
}

Status from_json(td_api::getCreatedPublicChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getCurrentState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCurrentWeather &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::getCustomEmojiReactionAnimations &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCustomEmojiStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_ids_, from.extract_field("custom_emoji_ids")));
  return Status::OK();
}

Status from_json(td_api::getDatabaseStatistics &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDeepLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::getDefaultBackgroundCustomEmojiStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultChatEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultChatPhotoCustomEmojiStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultMessageAutoDeleteTime &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDefaultProfilePhotoCustomEmojiStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDisallowedChatEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getEmojiCategories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::getEmojiReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, from.extract_field("emoji")));
  return Status::OK();
}

Status from_json(td_api::getEmojiSuggestionsUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getExternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getExternalLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::getFavoriteStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::getFileDownloadedPrefixSize &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  return Status::OK();
}

Status from_json(td_api::getFileExtension &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mime_type_, from.extract_field("mime_type")));
  return Status::OK();
}

Status from_json(td_api::getFileMimeType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_name_, from.extract_field("file_name")));
  return Status::OK();
}

Status from_json(td_api::getForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::getForumTopicDefaultIcons &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getForumTopicLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::getForumTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_date_, from.extract_field("offset_date")));
  TRY_STATUS(from_json(to.offset_message_id_, from.extract_field("offset_message_id")));
  TRY_STATUS(from_json(to.offset_message_thread_id_, from.extract_field("offset_message_thread_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getGameHighScores &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getGiveawayInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getGreetingStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getGrossingWebAppBots &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getGroupCallInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.can_self_unmute_, from.extract_field("can_self_unmute")));
  return Status::OK();
}

Status from_json(td_api::getGroupCallStreamSegment &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.time_offset_, from.extract_field("time_offset")));
  TRY_STATUS(from_json(to.scale_, from.extract_field("scale")));
  TRY_STATUS(from_json(to.channel_id_, from.extract_field("channel_id")));
  TRY_STATUS(from_json(to.video_quality_, from.extract_field("video_quality")));
  return Status::OK();
}

Status from_json(td_api::getGroupCallStreams &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getGroupsInCommon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.offset_chat_id_, from.extract_field("offset_chat_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getImportedContactCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getInactiveSupergroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getInlineGameHighScores &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
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

Status from_json(td_api::getInstalledBackgrounds &to, JsonObject &from) {
  TRY_STATUS(from_json(to.for_dark_theme_, from.extract_field("for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::getInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  return Status::OK();
}

Status from_json(td_api::getInternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.is_http_, from.extract_field("is_http")));
  return Status::OK();
}

Status from_json(td_api::getInternalLinkType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::getJsonString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.json_value_, from.extract_field("json_value")));
  return Status::OK();
}

Status from_json(td_api::getJsonValue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.json_, from.extract_field("json")));
  return Status::OK();
}

Status from_json(td_api::getKeywordEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.input_language_codes_, from.extract_field("input_language_codes")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_database_path_, from.extract_field("language_pack_database_path")));
  TRY_STATUS(from_json(to.localization_target_, from.extract_field("localization_target")));
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  TRY_STATUS(from_json(to.key_, from.extract_field("key")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackStrings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  TRY_STATUS(from_json(to.keys_, from.extract_field("keys")));
  return Status::OK();
}

Status from_json(td_api::getLinkPreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.link_preview_options_, from.extract_field("link_preview_options")));
  return Status::OK();
}

Status from_json(td_api::getLocalizationTargetInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::getLogStream &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getLogTagVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  return Status::OK();
}

Status from_json(td_api::getLogTags &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getLogVerbosityLevel &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getLoginUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.button_id_, from.extract_field("button_id")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getLoginUrlInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.button_id_, from.extract_field("button_id")));
  return Status::OK();
}

Status from_json(td_api::getMainWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  TRY_STATUS(from_json(to.application_name_, from.extract_field("application_name")));
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

Status from_json(td_api::getMarkdownText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::getMe &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
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

Status from_json(td_api::getMessageAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.row_size_, from.extract_field("row_size")));
  return Status::OK();
}

Status from_json(td_api::getMessageEffect &to, JsonObject &from) {
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageEmbeddingCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.for_album_, from.extract_field("for_album")));
  return Status::OK();
}

Status from_json(td_api::getMessageFileType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_file_head_, from.extract_field("message_file_head")));
  return Status::OK();
}

Status from_json(td_api::getMessageImportConfirmationText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
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

Status from_json(td_api::getMessageLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  return Status::OK();
}

Status from_json(td_api::getMessageLocally &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageProperties &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessagePublicForwards &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getMessageReadDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getMessageThread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
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

Status from_json(td_api::getMessageViewers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::getNetworkStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_current_, from.extract_field("only_current")));
  return Status::OK();
}

Status from_json(td_api::getNewChatPrivacySettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::getOwnedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_sticker_set_id_, from.extract_field("offset_sticker_set_id")));
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

Status from_json(td_api::getPassportAuthorizationFormAvailableElements &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_form_id_, from.extract_field("authorization_form_id")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getPassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getPasswordState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPaymentForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, from.extract_field("input_invoice")));
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  return Status::OK();
}

Status from_json(td_api::getPaymentReceipt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getPhoneNumberInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_prefix_, from.extract_field("phone_number_prefix")));
  return Status::OK();
}

Status from_json(td_api::getPhoneNumberInfoSync &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.phone_number_prefix_, from.extract_field("phone_number_prefix")));
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

Status from_json(td_api::getPreferredCountryLanguage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, from.extract_field("country_code")));
  return Status::OK();
}

Status from_json(td_api::getPremiumFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  return Status::OK();
}

Status from_json(td_api::getPremiumGiftCodePaymentOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.boosted_chat_id_, from.extract_field("boosted_chat_id")));
  return Status::OK();
}

Status from_json(td_api::getPremiumLimit &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_type_, from.extract_field("limit_type")));
  return Status::OK();
}

Status from_json(td_api::getPremiumState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPremiumStickerExamples &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPremiumStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getProxies &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getProxyLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::getPushReceiverId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::getReadDatePrivacySettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecentEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecentInlineBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecentStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  return Status::OK();
}

Status from_json(td_api::getRecentlyOpenedChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getRecentlyVisitedTMeUrls &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, from.extract_field("referrer")));
  return Status::OK();
}

Status from_json(td_api::getRecommendedChatFolders &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecommendedChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecoveryEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getRemoteFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.remote_file_id_, from.extract_field("remote_file_id")));
  TRY_STATUS(from_json(to.file_type_, from.extract_field("file_type")));
  return Status::OK();
}

Status from_json(td_api::getRepliedMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getSavedAnimations &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSavedMessagesTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::getSavedMessagesTopicHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getSavedMessagesTopicMessageByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.date_, from.extract_field("date")));
  return Status::OK();
}

Status from_json(td_api::getSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_sound_id_, from.extract_field("notification_sound_id")));
  return Status::OK();
}

Status from_json(td_api::getSavedNotificationSounds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSavedOrderInfo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getScopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  return Status::OK();
}

Status from_json(td_api::getSearchedForTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_prefix_, from.extract_field("tag_prefix")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, from.extract_field("secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::getStarAdAccountUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  return Status::OK();
}

Status from_json(td_api::getStarGiftPaymentOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getStarGiveawayPaymentOptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStarPaymentOptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStarRevenueStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getStarSubscriptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_expiring_, from.extract_field("only_expiring")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
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

Status from_json(td_api::getStarWithdrawalUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getStatisticalGraph &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::getStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::getStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, from.extract_field("set_id")));
  return Status::OK();
}

Status from_json(td_api::getStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, from.extract_field("set_id")));
  return Status::OK();
}

Status from_json(td_api::getStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getStorageStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_limit_, from.extract_field("chat_limit")));
  return Status::OK();
}

Status from_json(td_api::getStorageStatisticsFast &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::getStoryAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.row_size_, from.extract_field("row_size")));
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

Status from_json(td_api::getStoryNotificationSettingsExceptions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStoryPublicForwards &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getStoryStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getSuggestedFileName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.directory_, from.extract_field("directory")));
  return Status::OK();
}

Status from_json(td_api::getSuggestedStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::getSuitableDiscussionChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSuitablePersonalChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSupergroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::getSupergroupFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::getSupergroupMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getSupportName &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSupportUser &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTemporaryPasswordState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTextEntities &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::getThemeParametersJsonString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  return Status::OK();
}

Status from_json(td_api::getThemedChatEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getThemedEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTimeZones &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTopChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.category_, from.extract_field("category")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getTrendingStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getUserChatBoosts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getUserFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getUserLink &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getUserPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.setting_, from.extract_field("setting")));
  return Status::OK();
}

Status from_json(td_api::getUserProfilePhotos &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getUserSupportInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatAvailableParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatRtmpUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getWebAppLinkUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.web_app_short_name_, from.extract_field("web_app_short_name")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  TRY_STATUS(from_json(to.application_name_, from.extract_field("application_name")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getWebAppUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  TRY_STATUS(from_json(to.application_name_, from.extract_field("application_name")));
  return Status::OK();
}

Status from_json(td_api::getWebPageInstantView &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.force_full_, from.extract_field("force_full")));
  return Status::OK();
}

Status from_json(td_api::hideContactCloseBirthdays &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::hideSuggestedAction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.action_, from.extract_field("action")));
  return Status::OK();
}

Status from_json(td_api::importContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contacts_, from.extract_field("contacts")));
  return Status::OK();
}

Status from_json(td_api::importMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_file_, from.extract_field("message_file")));
  TRY_STATUS(from_json(to.attached_files_, from.extract_field("attached_files")));
  return Status::OK();
}

Status from_json(td_api::inviteGroupCallParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::joinChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::joinChatByInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::joinGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.audio_source_id_, from.extract_field("audio_source_id")));
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  TRY_STATUS(from_json(to.is_muted_, from.extract_field("is_muted")));
  TRY_STATUS(from_json(to.is_my_video_enabled_, from.extract_field("is_my_video_enabled")));
  TRY_STATUS(from_json(to.invite_hash_, from.extract_field("invite_hash")));
  return Status::OK();
}

Status from_json(td_api::launchPrepaidGiveaway &to, JsonObject &from) {
  TRY_STATUS(from_json(to.giveaway_id_, from.extract_field("giveaway_id")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  TRY_STATUS(from_json(to.winner_count_, from.extract_field("winner_count")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::leaveChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::leaveGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::loadActiveStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_list_, from.extract_field("story_list")));
  return Status::OK();
}

Status from_json(td_api::loadChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::loadGroupCallParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::loadQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  return Status::OK();
}

Status from_json(td_api::loadQuickReplyShortcuts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::loadSavedMessagesTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::logOut &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::openChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::openChatSimilarChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.opened_chat_id_, from.extract_field("opened_chat_id")));
  return Status::OK();
}

Status from_json(td_api::openMessageContent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::openStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::openWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.theme_, from.extract_field("theme")));
  TRY_STATUS(from_json(to.application_name_, from.extract_field("application_name")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
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

Status from_json(td_api::parseMarkdown &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::parseTextEntities &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.parse_mode_, from.extract_field("parse_mode")));
  return Status::OK();
}

Status from_json(td_api::pinChatMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  TRY_STATUS(from_json(to.only_for_self_, from.extract_field("only_for_self")));
  return Status::OK();
}

Status from_json(td_api::pingProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::preliminaryUploadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_, from.extract_field("file")));
  TRY_STATUS(from_json(to.file_type_, from.extract_field("file_type")));
  TRY_STATUS(from_json(to.priority_, from.extract_field("priority")));
  return Status::OK();
}

Status from_json(td_api::processChatFolderNewChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.added_chat_ids_, from.extract_field("added_chat_ids")));
  return Status::OK();
}

Status from_json(td_api::processChatJoinRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.approve_, from.extract_field("approve")));
  return Status::OK();
}

Status from_json(td_api::processChatJoinRequests &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.approve_, from.extract_field("approve")));
  return Status::OK();
}

Status from_json(td_api::processPushNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::rateSpeechRecognition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_good_, from.extract_field("is_good")));
  return Status::OK();
}

Status from_json(td_api::readAllChatMentions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::readAllChatReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::readAllMessageThreadMentions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::readAllMessageThreadReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::readChatList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  return Status::OK();
}

Status from_json(td_api::readFilePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.count_, from.extract_field("count")));
  return Status::OK();
}

Status from_json(td_api::readdQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_name_, from.extract_field("shortcut_name")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::recognizeSpeech &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::recoverAuthenticationPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  TRY_STATUS(from_json(to.new_password_, from.extract_field("new_password")));
  TRY_STATUS(from_json(to.new_hint_, from.extract_field("new_hint")));
  return Status::OK();
}

Status from_json(td_api::recoverPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  TRY_STATUS(from_json(to.new_password_, from.extract_field("new_password")));
  TRY_STATUS(from_json(to.new_hint_, from.extract_field("new_hint")));
  return Status::OK();
}

Status from_json(td_api::refundStarPayment &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.telegram_payment_charge_id_, from.extract_field("telegram_payment_charge_id")));
  return Status::OK();
}

Status from_json(td_api::registerDevice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.other_user_ids_, from.extract_field("other_user_ids")));
  return Status::OK();
}

Status from_json(td_api::registerUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  return Status::OK();
}

Status from_json(td_api::removeAllFilesFromDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_active_, from.extract_field("only_active")));
  TRY_STATUS(from_json(to.only_completed_, from.extract_field("only_completed")));
  TRY_STATUS(from_json(to.delete_from_cache_, from.extract_field("delete_from_cache")));
  return Status::OK();
}

Status from_json(td_api::removeBusinessConnectedBotFromChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::removeChatActionBar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::removeContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::removeFavoriteSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::removeFileFromDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.delete_from_cache_, from.extract_field("delete_from_cache")));
  return Status::OK();
}

Status from_json(td_api::removeInstalledBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_id_, from.extract_field("background_id")));
  return Status::OK();
}

Status from_json(td_api::removeMessageReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  return Status::OK();
}

Status from_json(td_api::removeNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_group_id_, from.extract_field("notification_group_id")));
  TRY_STATUS(from_json(to.notification_id_, from.extract_field("notification_id")));
  return Status::OK();
}

Status from_json(td_api::removeNotificationGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_group_id_, from.extract_field("notification_group_id")));
  TRY_STATUS(from_json(to.max_notification_id_, from.extract_field("max_notification_id")));
  return Status::OK();
}

Status from_json(td_api::removePendingPaidMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::removeProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, from.extract_field("proxy_id")));
  return Status::OK();
}

Status from_json(td_api::removeRecentHashtag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hashtag_, from.extract_field("hashtag")));
  return Status::OK();
}

Status from_json(td_api::removeRecentSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::removeRecentlyFoundChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::removeSavedAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, from.extract_field("animation")));
  return Status::OK();
}

Status from_json(td_api::removeSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_sound_id_, from.extract_field("notification_sound_id")));
  return Status::OK();
}

Status from_json(td_api::removeSearchedForTag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  return Status::OK();
}

Status from_json(td_api::removeStickerFromSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::removeTopChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.category_, from.extract_field("category")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::reorderActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.usernames_, from.extract_field("usernames")));
  return Status::OK();
}

Status from_json(td_api::reorderBotActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.usernames_, from.extract_field("usernames")));
  return Status::OK();
}

Status from_json(td_api::reorderBotMediaPreviews &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.file_ids_, from.extract_field("file_ids")));
  return Status::OK();
}

Status from_json(td_api::reorderChatFolders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_ids_, from.extract_field("chat_folder_ids")));
  TRY_STATUS(from_json(to.main_chat_list_position_, from.extract_field("main_chat_list_position")));
  return Status::OK();
}

Status from_json(td_api::reorderInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.sticker_set_ids_, from.extract_field("sticker_set_ids")));
  return Status::OK();
}

Status from_json(td_api::reorderQuickReplyShortcuts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_ids_, from.extract_field("shortcut_ids")));
  return Status::OK();
}

Status from_json(td_api::reorderSupergroupActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.usernames_, from.extract_field("usernames")));
  return Status::OK();
}

Status from_json(td_api::replacePrimaryChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::replaceStickerInSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.old_sticker_, from.extract_field("old_sticker")));
  TRY_STATUS(from_json(to.new_sticker_, from.extract_field("new_sticker")));
  return Status::OK();
}

Status from_json(td_api::replaceVideoChatRtmpUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::reportAuthenticationCodeMissing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mobile_network_code_, from.extract_field("mobile_network_code")));
  return Status::OK();
}

Status from_json(td_api::reportChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::reportChatPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::reportChatSponsoredMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json_bytes(to.option_id_, from.extract_field("option_id")));
  return Status::OK();
}

Status from_json(td_api::reportMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  return Status::OK();
}

Status from_json(td_api::reportPhoneNumberCodeMissing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mobile_network_code_, from.extract_field("mobile_network_code")));
  return Status::OK();
}

Status from_json(td_api::reportStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::reportSupergroupAntiSpamFalsePositive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::reportSupergroupSpam &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  return Status::OK();
}

Status from_json(td_api::requestAuthenticationPasswordRecovery &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::requestPasswordRecovery &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::requestQrCodeAuthentication &to, JsonObject &from) {
  TRY_STATUS(from_json(to.other_user_ids_, from.extract_field("other_user_ids")));
  return Status::OK();
}

Status from_json(td_api::resendAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  return Status::OK();
}

Status from_json(td_api::resendEmailAddressVerificationCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendLoginEmailAddressCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  return Status::OK();
}

Status from_json(td_api::resendPhoneNumberCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  return Status::OK();
}

Status from_json(td_api::resendRecoveryEmailAddressCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetAllNotificationSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetAuthenticationEmailAddress &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetInstalledBackgrounds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetNetworkStatistics &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetPassword &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reuseStarSubscription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.subscription_id_, from.extract_field("subscription_id")));
  return Status::OK();
}

Status from_json(td_api::revokeChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::revokeGroupCallInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::saveApplicationLogEvent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::searchBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::searchCallMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.only_missed_, from.extract_field("only_missed")));
  return Status::OK();
}

Status from_json(td_api::searchChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  return Status::OK();
}

Status from_json(td_api::searchChatMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  TRY_STATUS(from_json(to.from_message_id_, from.extract_field("from_message_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  return Status::OK();
}

Status from_json(td_api::searchChatRecentLocationMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchChatsNearby &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::searchChatsOnServer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.input_language_codes_, from.extract_field("input_language_codes")));
  return Status::OK();
}

Status from_json(td_api::searchFileDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.only_active_, from.extract_field("only_active")));
  TRY_STATUS(from_json(to.only_completed_, from.extract_field("only_completed")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchHashtags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.prefix_, from.extract_field("prefix")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.only_in_channels_, from.extract_field("only_in_channels")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  TRY_STATUS(from_json(to.min_date_, from.extract_field("min_date")));
  TRY_STATUS(from_json(to.max_date_, from.extract_field("max_date")));
  return Status::OK();
}

Status from_json(td_api::searchOutgoingDocumentMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::searchPublicChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::searchPublicChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::searchPublicMessagesByTag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
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

Status from_json(td_api::searchPublicStoriesByTag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
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

Status from_json(td_api::searchQuote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.quote_, from.extract_field("quote")));
  TRY_STATUS(from_json(to.quote_position_, from.extract_field("quote_position")));
  return Status::OK();
}

Status from_json(td_api::searchRecentlyFoundChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
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

Status from_json(td_api::searchSecretMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.filter_, from.extract_field("filter")));
  return Status::OK();
}

Status from_json(td_api::searchStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::searchStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::searchStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.emojis_, from.extract_field("emojis")));
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

Status from_json(td_api::searchUserByPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.only_local_, from.extract_field("only_local")));
  return Status::OK();
}

Status from_json(td_api::searchUserByToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::searchWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.web_app_short_name_, from.extract_field("web_app_short_name")));
  return Status::OK();
}

Status from_json(td_api::sendAuthenticationFirebaseSms &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::sendBotStartMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.parameter_, from.extract_field("parameter")));
  return Status::OK();
}

Status from_json(td_api::sendBusinessMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  TRY_STATUS(from_json(to.protect_content_, from.extract_field("protect_content")));
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
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

Status from_json(td_api::sendCallDebugInformation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.debug_information_, from.extract_field("debug_information")));
  return Status::OK();
}

Status from_json(td_api::sendCallLog &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.log_file_, from.extract_field("log_file")));
  return Status::OK();
}

Status from_json(td_api::sendCallRating &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.rating_, from.extract_field("rating")));
  TRY_STATUS(from_json(to.comment_, from.extract_field("comment")));
  TRY_STATUS(from_json(to.problems_, from.extract_field("problems")));
  return Status::OK();
}

Status from_json(td_api::sendCallSignalingData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::sendChatAction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.action_, from.extract_field("action")));
  return Status::OK();
}

Status from_json(td_api::sendCustomRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.method_, from.extract_field("method")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::sendEmailAddressVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  return Status::OK();
}

Status from_json(td_api::sendInlineQueryResultMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.query_id_, from.extract_field("query_id")));
  TRY_STATUS(from_json(to.result_id_, from.extract_field("result_id")));
  TRY_STATUS(from_json(to.hide_via_bot_, from.extract_field("hide_via_bot")));
  return Status::OK();
}

Status from_json(td_api::sendMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::sendMessageAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_, from.extract_field("reply_to")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  TRY_STATUS(from_json(to.input_message_contents_, from.extract_field("input_message_contents")));
  return Status::OK();
}

Status from_json(td_api::sendPassportAuthorizationForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_form_id_, from.extract_field("authorization_form_id")));
  TRY_STATUS(from_json(to.types_, from.extract_field("types")));
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

Status from_json(td_api::sendPhoneNumberCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::sendPhoneNumberFirebaseSms &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::sendQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.sending_id_, from.extract_field("sending_id")));
  return Status::OK();
}

Status from_json(td_api::sendStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.content_, from.extract_field("content")));
  TRY_STATUS(from_json(to.areas_, from.extract_field("areas")));
  TRY_STATUS(from_json(to.caption_, from.extract_field("caption")));
  TRY_STATUS(from_json(to.privacy_settings_, from.extract_field("privacy_settings")));
  TRY_STATUS(from_json(to.active_period_, from.extract_field("active_period")));
  TRY_STATUS(from_json(to.from_story_full_id_, from.extract_field("from_story_full_id")));
  TRY_STATUS(from_json(to.is_posted_to_chat_page_, from.extract_field("is_posted_to_chat_page")));
  TRY_STATUS(from_json(to.protect_content_, from.extract_field("protect_content")));
  return Status::OK();
}

Status from_json(td_api::sendWebAppCustomRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.method_, from.extract_field("method")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::sendWebAppData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.button_text_, from.extract_field("button_text")));
  TRY_STATUS(from_json(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::setAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.accent_color_id_, from.extract_field("accent_color_id")));
  TRY_STATUS(from_json(to.background_custom_emoji_id_, from.extract_field("background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setAccountTtl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.ttl_, from.extract_field("ttl")));
  return Status::OK();
}

Status from_json(td_api::setAlarm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.seconds_, from.extract_field("seconds")));
  return Status::OK();
}

Status from_json(td_api::setApplicationVerificationToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.verification_id_, from.extract_field("verification_id")));
  TRY_STATUS(from_json(to.token_, from.extract_field("token")));
  return Status::OK();
}

Status from_json(td_api::setArchiveChatListSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setAuthenticationEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, from.extract_field("email_address")));
  return Status::OK();
}

Status from_json(td_api::setAuthenticationPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, from.extract_field("phone_number")));
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setAutoDownloadSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::setAutosaveSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setBio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bio_, from.extract_field("bio")));
  return Status::OK();
}

Status from_json(td_api::setBirthdate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.birthdate_, from.extract_field("birthdate")));
  return Status::OK();
}

Status from_json(td_api::setBotInfoDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  return Status::OK();
}

Status from_json(td_api::setBotInfoShortDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.short_description_, from.extract_field("short_description")));
  return Status::OK();
}

Status from_json(td_api::setBotName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::setBotProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::setBotUpdatesStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.pending_update_count_, from.extract_field("pending_update_count")));
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::setBusinessAwayMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.away_message_settings_, from.extract_field("away_message_settings")));
  return Status::OK();
}

Status from_json(td_api::setBusinessConnectedBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_, from.extract_field("bot")));
  return Status::OK();
}

Status from_json(td_api::setBusinessGreetingMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.greeting_message_settings_, from.extract_field("greeting_message_settings")));
  return Status::OK();
}

Status from_json(td_api::setBusinessLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::setBusinessMessageIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::setBusinessOpeningHours &to, JsonObject &from) {
  TRY_STATUS(from_json(to.opening_hours_, from.extract_field("opening_hours")));
  return Status::OK();
}

Status from_json(td_api::setBusinessStartPage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.start_page_, from.extract_field("start_page")));
  return Status::OK();
}

Status from_json(td_api::setChatAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.accent_color_id_, from.extract_field("accent_color_id")));
  TRY_STATUS(from_json(to.background_custom_emoji_id_, from.extract_field("background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setChatActiveStoriesList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_list_, from.extract_field("story_list")));
  return Status::OK();
}

Status from_json(td_api::setChatAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.available_reactions_, from.extract_field("available_reactions")));
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

Status from_json(td_api::setChatClientData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.client_data_, from.extract_field("client_data")));
  return Status::OK();
}

Status from_json(td_api::setChatDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.description_, from.extract_field("description")));
  return Status::OK();
}

Status from_json(td_api::setChatDiscussionGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.discussion_chat_id_, from.extract_field("discussion_chat_id")));
  return Status::OK();
}

Status from_json(td_api::setChatDraftMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.draft_message_, from.extract_field("draft_message")));
  return Status::OK();
}

Status from_json(td_api::setChatEmojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.emoji_status_, from.extract_field("emoji_status")));
  return Status::OK();
}

Status from_json(td_api::setChatLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::setChatMemberStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.member_id_, from.extract_field("member_id")));
  TRY_STATUS(from_json(to.status_, from.extract_field("status")));
  return Status::OK();
}

Status from_json(td_api::setChatMessageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, from.extract_field("message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::setChatMessageSender &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_sender_id_, from.extract_field("message_sender_id")));
  return Status::OK();
}

Status from_json(td_api::setChatNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setChatPermissions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.permissions_, from.extract_field("permissions")));
  return Status::OK();
}

Status from_json(td_api::setChatPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::setChatPinnedStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_ids_, from.extract_field("story_ids")));
  return Status::OK();
}

Status from_json(td_api::setChatProfileAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.profile_accent_color_id_, from.extract_field("profile_accent_color_id")));
  TRY_STATUS(from_json(to.profile_background_custom_emoji_id_, from.extract_field("profile_background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setChatSlowModeDelay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.slow_mode_delay_, from.extract_field("slow_mode_delay")));
  return Status::OK();
}

Status from_json(td_api::setChatTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.theme_name_, from.extract_field("theme_name")));
  return Status::OK();
}

Status from_json(td_api::setChatTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::setCloseFriends &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, from.extract_field("user_ids")));
  return Status::OK();
}

Status from_json(td_api::setCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.commands_, from.extract_field("commands")));
  return Status::OK();
}

Status from_json(td_api::setCustomEmojiStickerSetThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.custom_emoji_id_, from.extract_field("custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setCustomLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.info_, from.extract_field("info")));
  TRY_STATUS(from_json(to.strings_, from.extract_field("strings")));
  return Status::OK();
}

Status from_json(td_api::setCustomLanguagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  TRY_STATUS(from_json(to.new_string_, from.extract_field("new_string")));
  return Status::OK();
}

Status from_json(td_api::setDatabaseEncryptionKey &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.new_encryption_key_, from.extract_field("new_encryption_key")));
  return Status::OK();
}

Status from_json(td_api::setDefaultBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_, from.extract_field("background")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  TRY_STATUS(from_json(to.for_dark_theme_, from.extract_field("for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::setDefaultChannelAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.default_channel_administrator_rights_, from.extract_field("default_channel_administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::setDefaultGroupAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.default_group_administrator_rights_, from.extract_field("default_group_administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::setDefaultMessageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_auto_delete_time_, from.extract_field("message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::setDefaultReactionType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  return Status::OK();
}

Status from_json(td_api::setEmojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_status_, from.extract_field("emoji_status")));
  return Status::OK();
}

Status from_json(td_api::setFileGenerationProgress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, from.extract_field("generation_id")));
  TRY_STATUS(from_json(to.expected_size_, from.extract_field("expected_size")));
  TRY_STATUS(from_json(to.local_prefix_size_, from.extract_field("local_prefix_size")));
  return Status::OK();
}

Status from_json(td_api::setForumTopicNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
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

Status from_json(td_api::setGroupCallParticipantIsSpeaking &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.audio_source_, from.extract_field("audio_source")));
  TRY_STATUS(from_json(to.is_speaking_, from.extract_field("is_speaking")));
  return Status::OK();
}

Status from_json(td_api::setGroupCallParticipantVolumeLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.volume_level_, from.extract_field("volume_level")));
  return Status::OK();
}

Status from_json(td_api::setGroupCallTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::setInactiveSessionTtl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inactive_session_ttl_days_, from.extract_field("inactive_session_ttl_days")));
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

Status from_json(td_api::setLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::setLogStream &to, JsonObject &from) {
  TRY_STATUS(from_json(to.log_stream_, from.extract_field("log_stream")));
  return Status::OK();
}

Status from_json(td_api::setLogTagVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.new_verbosity_level_, from.extract_field("new_verbosity_level")));
  return Status::OK();
}

Status from_json(td_api::setLogVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.new_verbosity_level_, from.extract_field("new_verbosity_level")));
  return Status::OK();
}

Status from_json(td_api::setLoginEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.new_login_email_address_, from.extract_field("new_login_email_address")));
  return Status::OK();
}

Status from_json(td_api::setMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.menu_button_, from.extract_field("menu_button")));
  return Status::OK();
}

Status from_json(td_api::setMessageFactCheck &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  return Status::OK();
}

Status from_json(td_api::setMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reaction_types_, from.extract_field("reaction_types")));
  TRY_STATUS(from_json(to.is_big_, from.extract_field("is_big")));
  return Status::OK();
}

Status from_json(td_api::setMessageSenderBlockList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sender_id_, from.extract_field("sender_id")));
  TRY_STATUS(from_json(to.block_list_, from.extract_field("block_list")));
  return Status::OK();
}

Status from_json(td_api::setName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.first_name_, from.extract_field("first_name")));
  TRY_STATUS(from_json(to.last_name_, from.extract_field("last_name")));
  return Status::OK();
}

Status from_json(td_api::setNetworkType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::setNewChatPrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::setPassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.element_, from.extract_field("element")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::setPassportElementErrors &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.errors_, from.extract_field("errors")));
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

Status from_json(td_api::setPersonalChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::setPinnedChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::setPinnedForumTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_ids_, from.extract_field("message_thread_ids")));
  return Status::OK();
}

Status from_json(td_api::setPinnedSavedMessagesTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_ids_, from.extract_field("saved_messages_topic_ids")));
  return Status::OK();
}

Status from_json(td_api::setPollAnswer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.option_ids_, from.extract_field("option_ids")));
  return Status::OK();
}

Status from_json(td_api::setProfileAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.profile_accent_color_id_, from.extract_field("profile_accent_color_id")));
  TRY_STATUS(from_json(to.profile_background_custom_emoji_id_, from.extract_field("profile_background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  TRY_STATUS(from_json(to.is_public_, from.extract_field("is_public")));
  return Status::OK();
}

Status from_json(td_api::setQuickReplyShortcutName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::setReactionNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setReadDatePrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setRecoveryEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  TRY_STATUS(from_json(to.new_recovery_email_address_, from.extract_field("new_recovery_email_address")));
  return Status::OK();
}

Status from_json(td_api::setSavedMessagesTagLabel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, from.extract_field("tag")));
  TRY_STATUS(from_json(to.label_, from.extract_field("label")));
  return Status::OK();
}

Status from_json(td_api::setScopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, from.extract_field("scope")));
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.emojis_, from.extract_field("emojis")));
  return Status::OK();
}

Status from_json(td_api::setStickerKeywords &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.keywords_, from.extract_field("keywords")));
  return Status::OK();
}

Status from_json(td_api::setStickerMaskPosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.mask_position_, from.extract_field("mask_position")));
  return Status::OK();
}

Status from_json(td_api::setStickerPositionInSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.position_, from.extract_field("position")));
  return Status::OK();
}

Status from_json(td_api::setStickerSetThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.thumbnail_, from.extract_field("thumbnail")));
  TRY_STATUS(from_json(to.format_, from.extract_field("format")));
  return Status::OK();
}

Status from_json(td_api::setStickerSetTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  return Status::OK();
}

Status from_json(td_api::setStoryPrivacySettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.privacy_settings_, from.extract_field("privacy_settings")));
  return Status::OK();
}

Status from_json(td_api::setStoryReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.reaction_type_, from.extract_field("reaction_type")));
  TRY_STATUS(from_json(to.update_recent_reactions_, from.extract_field("update_recent_reactions")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupCustomEmojiStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.custom_emoji_sticker_set_id_, from.extract_field("custom_emoji_sticker_set_id")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.sticker_set_id_, from.extract_field("sticker_set_id")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupUnrestrictBoostCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.unrestrict_boost_count_, from.extract_field("unrestrict_boost_count")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
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

Status from_json(td_api::setUserPersonalProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::setUserPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.setting_, from.extract_field("setting")));
  TRY_STATUS(from_json(to.rules_, from.extract_field("rules")));
  return Status::OK();
}

Status from_json(td_api::setUserSupportInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.message_, from.extract_field("message")));
  return Status::OK();
}

Status from_json(td_api::setUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::setVideoChatDefaultParticipant &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.default_participant_id_, from.extract_field("default_participant_id")));
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

Status from_json(td_api::sharePhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
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

Status from_json(td_api::startGroupCallRecording &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.title_, from.extract_field("title")));
  TRY_STATUS(from_json(to.record_video_, from.extract_field("record_video")));
  TRY_STATUS(from_json(to.use_portrait_orientation_, from.extract_field("use_portrait_orientation")));
  return Status::OK();
}

Status from_json(td_api::startGroupCallScreenSharing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.audio_source_id_, from.extract_field("audio_source_id")));
  TRY_STATUS(from_json(to.payload_, from.extract_field("payload")));
  return Status::OK();
}

Status from_json(td_api::startScheduledGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::stopBusinessPoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::stopPoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::suggestUserProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.photo_, from.extract_field("photo")));
  return Status::OK();
}

Status from_json(td_api::synchronizeLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::terminateAllOtherSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::terminateSession &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  return Status::OK();
}

Status from_json(td_api::testCallBytes &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testCallEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::testCallString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorIntObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorStringObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testGetDifference &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::testNetwork &to, JsonObject &from) {
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

Status from_json(td_api::testReturnError &to, JsonObject &from) {
  TRY_STATUS(from_json(to.error_, from.extract_field("error")));
  return Status::OK();
}

Status from_json(td_api::testSquareInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testUseUpdate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::toggleAllDownloadsArePaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.are_paused_, from.extract_field("are_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleBotIsAddedToAttachmentMenu &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.is_added_, from.extract_field("is_added")));
  TRY_STATUS(from_json(to.allow_write_access_, from.extract_field("allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::toggleBotUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.is_active_, from.extract_field("is_active")));
  return Status::OK();
}

Status from_json(td_api::toggleBusinessConnectedBotChatIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_paused_, from.extract_field("is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleChatDefaultDisableNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.default_disable_notification_, from.extract_field("default_disable_notification")));
  return Status::OK();
}

Status from_json(td_api::toggleChatFolderTags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.are_tags_enabled_, from.extract_field("are_tags_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleChatHasProtectedContent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.has_protected_content_, from.extract_field("has_protected_content")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsMarkedAsUnread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_marked_as_unread_, from.extract_field("is_marked_as_unread")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, from.extract_field("chat_list")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsTranslatable &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_translatable_, from.extract_field("is_translatable")));
  return Status::OK();
}

Status from_json(td_api::toggleChatViewAsTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.view_as_topics_, from.extract_field("view_as_topics")));
  return Status::OK();
}

Status from_json(td_api::toggleDownloadIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  TRY_STATUS(from_json(to.is_paused_, from.extract_field("is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleForumTopicIsClosed &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.is_closed_, from.extract_field("is_closed")));
  return Status::OK();
}

Status from_json(td_api::toggleForumTopicIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleGeneralForumTopicIsHidden &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_hidden_, from.extract_field("is_hidden")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallEnabledStartNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.enabled_start_notification_, from.extract_field("enabled_start_notification")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallIsMyVideoEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.is_my_video_enabled_, from.extract_field("is_my_video_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallIsMyVideoPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.is_my_video_paused_, from.extract_field("is_my_video_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallMuteNewParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.mute_new_participants_, from.extract_field("mute_new_participants")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallParticipantIsHandRaised &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.is_hand_raised_, from.extract_field("is_hand_raised")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallParticipantIsMuted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.is_muted_, from.extract_field("is_muted")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallScreenSharingIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.is_paused_, from.extract_field("is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleHasSponsoredMessagesEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.has_sponsored_messages_enabled_, from.extract_field("has_sponsored_messages_enabled")));
  return Status::OK();
}

Status from_json(td_api::togglePaidMessageReactionIsAnonymous &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.is_anonymous_, from.extract_field("is_anonymous")));
  return Status::OK();
}

Status from_json(td_api::toggleSavedMessagesTopicIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_messages_topic_id_, from.extract_field("saved_messages_topic_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleSessionCanAcceptCalls &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  TRY_STATUS(from_json(to.can_accept_calls_, from.extract_field("can_accept_calls")));
  return Status::OK();
}

Status from_json(td_api::toggleSessionCanAcceptSecretChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, from.extract_field("session_id")));
  TRY_STATUS(from_json(to.can_accept_secret_chats_, from.extract_field("can_accept_secret_chats")));
  return Status::OK();
}

Status from_json(td_api::toggleStoryIsPostedToChatPage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_sender_chat_id_, from.extract_field("story_sender_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.is_posted_to_chat_page_, from.extract_field("is_posted_to_chat_page")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupCanHaveSponsoredMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.can_have_sponsored_messages_, from.extract_field("can_have_sponsored_messages")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupHasAggressiveAntiSpamEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.has_aggressive_anti_spam_enabled_, from.extract_field("has_aggressive_anti_spam_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupHasHiddenMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.has_hidden_members_, from.extract_field("has_hidden_members")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsAllHistoryAvailable &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.is_all_history_available_, from.extract_field("is_all_history_available")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsBroadcastGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsForum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.is_forum_, from.extract_field("is_forum")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupJoinByRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.join_by_request_, from.extract_field("join_by_request")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupJoinToSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.join_to_send_messages_, from.extract_field("join_to_send_messages")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupSignMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.sign_messages_, from.extract_field("sign_messages")));
  TRY_STATUS(from_json(to.show_message_sender_, from.extract_field("show_message_sender")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.is_active_, from.extract_field("is_active")));
  return Status::OK();
}

Status from_json(td_api::toggleUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  TRY_STATUS(from_json(to.is_active_, from.extract_field("is_active")));
  return Status::OK();
}

Status from_json(td_api::transferChatOwnership &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::translateMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.to_language_code_, from.extract_field("to_language_code")));
  return Status::OK();
}

Status from_json(td_api::translateText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.to_language_code_, from.extract_field("to_language_code")));
  return Status::OK();
}

Status from_json(td_api::unpinAllChatMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::unpinAllMessageThreadMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, from.extract_field("message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::unpinChatMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::upgradeBasicGroupChatToSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::uploadStickerFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.sticker_format_, from.extract_field("sticker_format")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::validateOrderInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, from.extract_field("input_invoice")));
  TRY_STATUS(from_json(to.order_info_, from.extract_field("order_info")));
  TRY_STATUS(from_json(to.allow_save_, from.extract_field("allow_save")));
  return Status::OK();
}

Status from_json(td_api::viewMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_ids_, from.extract_field("message_ids")));
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  TRY_STATUS(from_json(to.force_read_, from.extract_field("force_read")));
  return Status::OK();
}

Status from_json(td_api::viewPremiumFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, from.extract_field("feature")));
  return Status::OK();
}

Status from_json(td_api::viewTrendingStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_ids_, from.extract_field("sticker_set_ids")));
  return Status::OK();
}

Status from_json(td_api::writeGeneratedFilePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, from.extract_field("generation_id")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
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

void to_json(JsonValueScope &jv, const td_api::accountTtl &object) {
  auto jo = jv.enter_object();
  jo("@type", "accountTtl");
  jo("days", object.days_);
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

void to_json(JsonValueScope &jv, const td_api::addedReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "addedReactions");
  jo("total_count", object.total_count_);
  jo("reactions", ToJson(object.reactions_));
  jo("next_offset", object.next_offset_);
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

void to_json(JsonValueScope &jv, const td_api::animatedChatPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "animatedChatPhoto");
  jo("length", object.length_);
  if (object.file_) {
    jo("file", ToJson(*object.file_));
  }
  jo("main_frame_timestamp", object.main_frame_timestamp_);
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

void to_json(JsonValueScope &jv, const td_api::animations &object) {
  auto jo = jv.enter_object();
  jo("@type", "animations");
  jo("animations", ToJson(object.animations_));
}

void to_json(JsonValueScope &jv, const td_api::archiveChatListSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "archiveChatListSettings");
  jo("archive_and_mute_new_chats_from_unknown_users", JsonBool{object.archive_and_mute_new_chats_from_unknown_users_});
  jo("keep_unmuted_chats_archived", JsonBool{object.keep_unmuted_chats_archived_});
  jo("keep_chats_from_folders_archived", JsonBool{object.keep_chats_from_folders_archived_});
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

void to_json(JsonValueScope &jv, const td_api::attachmentMenuBotColor &object) {
  auto jo = jv.enter_object();
  jo("@type", "attachmentMenuBotColor");
  jo("light_color", object.light_color_);
  jo("dark_color", object.dark_color_);
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

void to_json(JsonValueScope &jv, const td_api::autoDownloadSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "autoDownloadSettings");
  jo("is_auto_download_enabled", JsonBool{object.is_auto_download_enabled_});
  jo("max_photo_file_size", object.max_photo_file_size_);
  jo("max_video_file_size", object.max_video_file_size_);
  jo("max_other_file_size", object.max_other_file_size_);
  jo("video_upload_bitrate", object.video_upload_bitrate_);
  jo("preload_large_videos", JsonBool{object.preload_large_videos_});
  jo("preload_next_audio", JsonBool{object.preload_next_audio_});
  jo("preload_stories", JsonBool{object.preload_stories_});
  jo("use_less_data_for_calls", JsonBool{object.use_less_data_for_calls_});
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

void to_json(JsonValueScope &jv, const td_api::autosaveSettingsException &object) {
  auto jo = jv.enter_object();
  jo("@type", "autosaveSettingsException");
  jo("chat_id", object.chat_id_);
  if (object.settings_) {
    jo("settings", ToJson(*object.settings_));
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

void to_json(JsonValueScope &jv, const td_api::availableReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "availableReaction");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("needs_premium", JsonBool{object.needs_premium_});
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

void to_json(JsonValueScope &jv, const td_api::background &object) {
  auto jo = jv.enter_object();
  jo("@type", "background");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("is_default", JsonBool{object.is_default_});
  jo("is_dark", JsonBool{object.is_dark_});
  jo("name", object.name_);
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  if (object.type_) {
    jo("type", ToJson(*object.type_));
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

void to_json(JsonValueScope &jv, const td_api::backgrounds &object) {
  auto jo = jv.enter_object();
  jo("@type", "backgrounds");
  jo("backgrounds", ToJson(object.backgrounds_));
}

void to_json(JsonValueScope &jv, const td_api::bankCardActionOpenUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "bankCardActionOpenUrl");
  jo("text", object.text_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::bankCardInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "bankCardInfo");
  jo("title", object.title_);
  jo("actions", ToJson(object.actions_));
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

void to_json(JsonValueScope &jv, const td_api::birthdate &object) {
  auto jo = jv.enter_object();
  jo("@type", "birthdate");
  jo("day", object.day_);
  jo("month", object.month_);
  jo("year", object.year_);
}

void to_json(JsonValueScope &jv, const td_api::BlockList &object) {
  td_api::downcast_call(const_cast<td_api::BlockList &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::blockListMain &object) {
  auto jo = jv.enter_object();
  jo("@type", "blockListMain");
}

void to_json(JsonValueScope &jv, const td_api::blockListStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "blockListStories");
}

void to_json(JsonValueScope &jv, const td_api::botCommand &object) {
  auto jo = jv.enter_object();
  jo("@type", "botCommand");
  jo("command", object.command_);
  jo("description", object.description_);
}

void to_json(JsonValueScope &jv, const td_api::botCommands &object) {
  auto jo = jv.enter_object();
  jo("@type", "botCommands");
  jo("bot_user_id", object.bot_user_id_);
  jo("commands", ToJson(object.commands_));
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

void to_json(JsonValueScope &jv, const td_api::botMediaPreview &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMediaPreview");
  jo("date", object.date_);
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
}

void to_json(JsonValueScope &jv, const td_api::botMediaPreviewInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMediaPreviewInfo");
  jo("previews", ToJson(object.previews_));
  jo("language_codes", ToJson(object.language_codes_));
}

void to_json(JsonValueScope &jv, const td_api::botMediaPreviews &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMediaPreviews");
  jo("previews", ToJson(object.previews_));
}

void to_json(JsonValueScope &jv, const td_api::botMenuButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMenuButton");
  jo("text", object.text_);
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::BotTransactionPurpose &object) {
  td_api::downcast_call(const_cast<td_api::BotTransactionPurpose &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::botTransactionPurposePaidMedia &object) {
  auto jo = jv.enter_object();
  jo("@type", "botTransactionPurposePaidMedia");
  jo("media", ToJson(object.media_));
  jo("payload", object.payload_);
}

void to_json(JsonValueScope &jv, const td_api::botTransactionPurposeInvoicePayment &object) {
  auto jo = jv.enter_object();
  jo("@type", "botTransactionPurposeInvoicePayment");
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
  jo("invoice_payload", base64_encode(object.invoice_payload_));
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

void to_json(JsonValueScope &jv, const td_api::businessBotManageBar &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessBotManageBar");
  jo("bot_user_id", object.bot_user_id_);
  jo("manage_url", object.manage_url_);
  jo("is_bot_paused", JsonBool{object.is_bot_paused_});
  jo("can_bot_reply", JsonBool{object.can_bot_reply_});
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

void to_json(JsonValueScope &jv, const td_api::businessChatLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessChatLinkInfo");
  jo("chat_id", object.chat_id_);
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
}

void to_json(JsonValueScope &jv, const td_api::businessChatLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessChatLinks");
  jo("links", ToJson(object.links_));
}

void to_json(JsonValueScope &jv, const td_api::businessConnectedBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessConnectedBot");
  jo("bot_user_id", object.bot_user_id_);
  if (object.recipients_) {
    jo("recipients", ToJson(*object.recipients_));
  }
  jo("can_reply", JsonBool{object.can_reply_});
}

void to_json(JsonValueScope &jv, const td_api::businessConnection &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessConnection");
  jo("id", object.id_);
  jo("user_id", object.user_id_);
  jo("user_chat_id", object.user_chat_id_);
  jo("date", object.date_);
  jo("can_reply", JsonBool{object.can_reply_});
  jo("is_enabled", JsonBool{object.is_enabled_});
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

void to_json(JsonValueScope &jv, const td_api::businessFeatures &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessFeatures");
  jo("features", ToJson(object.features_));
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

void to_json(JsonValueScope &jv, const td_api::businessLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessLocation");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("address", object.address_);
}

void to_json(JsonValueScope &jv, const td_api::businessMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessMessage");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  if (object.reply_to_message_) {
    jo("reply_to_message", ToJson(*object.reply_to_message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::businessMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessMessages");
  jo("messages", ToJson(object.messages_));
}

void to_json(JsonValueScope &jv, const td_api::businessOpeningHours &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessOpeningHours");
  jo("time_zone_id", object.time_zone_id_);
  jo("opening_hours", ToJson(object.opening_hours_));
}

void to_json(JsonValueScope &jv, const td_api::businessOpeningHoursInterval &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessOpeningHoursInterval");
  jo("start_minute", object.start_minute_);
  jo("end_minute", object.end_minute_);
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

void to_json(JsonValueScope &jv, const td_api::businessStartPage &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessStartPage");
  jo("title", object.title_);
  jo("message", object.message_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
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

void to_json(JsonValueScope &jv, const td_api::callId &object) {
  auto jo = jv.enter_object();
  jo("@type", "callId");
  jo("id", object.id_);
}

void to_json(JsonValueScope &jv, const td_api::callProtocol &object) {
  auto jo = jv.enter_object();
  jo("@type", "callProtocol");
  jo("udp_p2p", JsonBool{object.udp_p2p_});
  jo("udp_reflector", JsonBool{object.udp_reflector_});
  jo("min_layer", object.min_layer_);
  jo("max_layer", object.max_layer_);
  jo("library_versions", ToJson(object.library_versions_));
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

void to_json(JsonValueScope &jv, const td_api::callbackQueryAnswer &object) {
  auto jo = jv.enter_object();
  jo("@type", "callbackQueryAnswer");
  jo("text", object.text_);
  jo("show_alert", JsonBool{object.show_alert_});
  jo("url", object.url_);
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

void to_json(JsonValueScope &jv, const td_api::CanSendMessageToUserResult &object) {
  td_api::downcast_call(const_cast<td_api::CanSendMessageToUserResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::canSendMessageToUserResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendMessageToUserResultOk");
}

void to_json(JsonValueScope &jv, const td_api::canSendMessageToUserResultUserIsDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendMessageToUserResultUserIsDeleted");
}

void to_json(JsonValueScope &jv, const td_api::canSendMessageToUserResultUserRestrictsNewChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendMessageToUserResultUserRestrictsNewChats");
}

void to_json(JsonValueScope &jv, const td_api::CanSendStoryResult &object) {
  td_api::downcast_call(const_cast<td_api::CanSendStoryResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::canSendStoryResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendStoryResultOk");
}

void to_json(JsonValueScope &jv, const td_api::canSendStoryResultPremiumNeeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendStoryResultPremiumNeeded");
}

void to_json(JsonValueScope &jv, const td_api::canSendStoryResultBoostNeeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendStoryResultBoostNeeded");
}

void to_json(JsonValueScope &jv, const td_api::canSendStoryResultActiveStoryLimitExceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendStoryResultActiveStoryLimitExceeded");
}

void to_json(JsonValueScope &jv, const td_api::canSendStoryResultWeeklyLimitExceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendStoryResultWeeklyLimitExceeded");
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::canSendStoryResultMonthlyLimitExceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canSendStoryResultMonthlyLimitExceeded");
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::CanTransferOwnershipResult &object) {
  td_api::downcast_call(const_cast<td_api::CanTransferOwnershipResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "canTransferOwnershipResultOk");
}

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultPasswordNeeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "canTransferOwnershipResultPasswordNeeded");
}

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultPasswordTooFresh &object) {
  auto jo = jv.enter_object();
  jo("@type", "canTransferOwnershipResultPasswordTooFresh");
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::canTransferOwnershipResultSessionTooFresh &object) {
  auto jo = jv.enter_object();
  jo("@type", "canTransferOwnershipResultSessionTooFresh");
  jo("retry_after", object.retry_after_);
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
  jo("theme_name", object.theme_name_);
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

void to_json(JsonValueScope &jv, const td_api::ChatActionBar &object) {
  td_api::downcast_call(const_cast<td_api::ChatActionBar &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportSpam &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarReportSpam");
  jo("can_unarchive", JsonBool{object.can_unarchive_});
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportUnrelatedLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarReportUnrelatedLocation");
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarInviteMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarInviteMembers");
}

void to_json(JsonValueScope &jv, const td_api::chatActionBarReportAddBlock &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActionBarReportAddBlock");
  jo("can_unarchive", JsonBool{object.can_unarchive_});
  jo("distance", object.distance_);
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

void to_json(JsonValueScope &jv, const td_api::chatActiveStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatActiveStories");
  jo("chat_id", object.chat_id_);
  if (object.list_) {
    jo("list", ToJson(*object.list_));
  }
  jo("order", object.order_);
  jo("max_read_story_id", object.max_read_story_id_);
  jo("stories", ToJson(object.stories_));
}

void to_json(JsonValueScope &jv, const td_api::chatAdministrator &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAdministrator");
  jo("user_id", object.user_id_);
  jo("custom_title", object.custom_title_);
  jo("is_owner", JsonBool{object.is_owner_});
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
  jo("is_anonymous", JsonBool{object.is_anonymous_});
}

void to_json(JsonValueScope &jv, const td_api::chatAdministrators &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAdministrators");
  jo("administrators", ToJson(object.administrators_));
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

void to_json(JsonValueScope &jv, const td_api::chatBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBackground");
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  jo("dark_theme_dimming", object.dark_theme_dimming_);
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
  jo("min_speech_recognition_boost_level", object.min_speech_recognition_boost_level_);
  jo("min_sponsored_message_disable_boost_level", object.min_sponsored_message_disable_boost_level_);
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
  jo("can_recognize_speech", JsonBool{object.can_recognize_speech_});
  jo("can_disable_sponsored_messages", JsonBool{object.can_disable_sponsored_messages_});
}

void to_json(JsonValueScope &jv, const td_api::chatBoostLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostLink");
  jo("link", object.link_);
  jo("is_public", JsonBool{object.is_public_});
}

void to_json(JsonValueScope &jv, const td_api::chatBoostLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostLinkInfo");
  jo("is_public", JsonBool{object.is_public_});
  jo("chat_id", object.chat_id_);
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

void to_json(JsonValueScope &jv, const td_api::chatBoostSlots &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostSlots");
  jo("slots", ToJson(object.slots_));
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

void to_json(JsonValueScope &jv, const td_api::chatBoostStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBoostStatus");
  jo("boost_url", object.boost_url_);
  jo("applied_slot_ids", ToJson(object.applied_slot_ids_));
  jo("level", object.level_);
  jo("gift_code_boost_count", object.gift_code_boost_count_);
  jo("boost_count", object.boost_count_);
  jo("current_level_boost_count", object.current_level_boost_count_);
  jo("next_level_boost_count", object.next_level_boost_count_);
  jo("premium_member_count", object.premium_member_count_);
  jo("premium_member_percentage", object.premium_member_percentage_);
  jo("prepaid_giveaways", ToJson(object.prepaid_giveaways_));
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

void to_json(JsonValueScope &jv, const td_api::chatEvents &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEvents");
  jo("events", ToJson(object.events_));
}

void to_json(JsonValueScope &jv, const td_api::chatFolder &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolder");
  jo("title", object.title_);
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

void to_json(JsonValueScope &jv, const td_api::chatFolderIcon &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderIcon");
  jo("name", object.name_);
}

void to_json(JsonValueScope &jv, const td_api::chatFolderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderInfo");
  jo("id", object.id_);
  jo("title", object.title_);
  if (object.icon_) {
    jo("icon", ToJson(*object.icon_));
  }
  jo("color_id", object.color_id_);
  jo("is_shareable", JsonBool{object.is_shareable_});
  jo("has_my_invite_links", JsonBool{object.has_my_invite_links_});
}

void to_json(JsonValueScope &jv, const td_api::chatFolderInviteLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderInviteLink");
  jo("invite_link", object.invite_link_);
  jo("name", object.name_);
  jo("chat_ids", ToJson(object.chat_ids_));
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

void to_json(JsonValueScope &jv, const td_api::chatFolderInviteLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderInviteLinks");
  jo("invite_links", ToJson(object.invite_links_));
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

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkCount");
  jo("user_id", object.user_id_);
  jo("invite_link_count", object.invite_link_count_);
  jo("revoked_invite_link_count", object.revoked_invite_link_count_);
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkCounts &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkCounts");
  jo("invite_link_counts", ToJson(object.invite_link_counts_));
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
  jo("is_verified", JsonBool{object.is_verified_});
  jo("is_scam", JsonBool{object.is_scam_});
  jo("is_fake", JsonBool{object.is_fake_});
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkMember");
  jo("user_id", object.user_id_);
  jo("joined_chat_date", object.joined_chat_date_);
  jo("via_chat_folder_invite_link", JsonBool{object.via_chat_folder_invite_link_});
  jo("approver_user_id", object.approver_user_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinkMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinkMembers");
  jo("total_count", object.total_count_);
  jo("members", ToJson(object.members_));
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

void to_json(JsonValueScope &jv, const td_api::chatInviteLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinks");
  jo("total_count", object.total_count_);
  jo("invite_links", ToJson(object.invite_links_));
}

void to_json(JsonValueScope &jv, const td_api::chatJoinRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatJoinRequest");
  jo("user_id", object.user_id_);
  jo("date", object.date_);
  jo("bio", object.bio_);
}

void to_json(JsonValueScope &jv, const td_api::chatJoinRequests &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatJoinRequests");
  jo("total_count", object.total_count_);
  jo("requests", ToJson(object.requests_));
}

void to_json(JsonValueScope &jv, const td_api::chatJoinRequestsInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatJoinRequestsInfo");
  jo("total_count", object.total_count_);
  jo("user_ids", ToJson(object.user_ids_));
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

void to_json(JsonValueScope &jv, const td_api::chatLists &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatLists");
  jo("chat_lists", ToJson(object.chat_lists_));
}

void to_json(JsonValueScope &jv, const td_api::chatLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatLocation");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("address", object.address_);
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

void to_json(JsonValueScope &jv, const td_api::chatMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMembers");
  jo("total_count", object.total_count_);
  jo("members", ToJson(object.members_));
}

void to_json(JsonValueScope &jv, const td_api::chatMessageSender &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMessageSender");
  if (object.sender_) {
    jo("sender", ToJson(*object.sender_));
  }
  jo("needs_premium", JsonBool{object.needs_premium_});
}

void to_json(JsonValueScope &jv, const td_api::chatMessageSenders &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMessageSenders");
  jo("senders", ToJson(object.senders_));
}

void to_json(JsonValueScope &jv, const td_api::chatNearby &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatNearby");
  jo("chat_id", object.chat_id_);
  jo("distance", object.distance_);
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
  jo("use_default_show_story_sender", JsonBool{object.use_default_show_story_sender_});
  jo("show_story_sender", JsonBool{object.show_story_sender_});
  jo("use_default_disable_pinned_message_notifications", JsonBool{object.use_default_disable_pinned_message_notifications_});
  jo("disable_pinned_message_notifications", JsonBool{object.disable_pinned_message_notifications_});
  jo("use_default_disable_mention_notifications", JsonBool{object.use_default_disable_mention_notifications_});
  jo("disable_mention_notifications", JsonBool{object.disable_mention_notifications_});
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

void to_json(JsonValueScope &jv, const td_api::chatPhotos &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPhotos");
  jo("total_count", object.total_count_);
  jo("photos", ToJson(object.photos_));
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

void to_json(JsonValueScope &jv, const td_api::chatRevenueAmount &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueAmount");
  jo("cryptocurrency", object.cryptocurrency_);
  jo("total_amount", ToJson(JsonInt64{object.total_amount_}));
  jo("balance_amount", ToJson(JsonInt64{object.balance_amount_}));
  jo("available_amount", ToJson(JsonInt64{object.available_amount_}));
  jo("withdrawal_enabled", JsonBool{object.withdrawal_enabled_});
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

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransaction");
  jo("cryptocurrency", object.cryptocurrency_);
  jo("cryptocurrency_amount", ToJson(JsonInt64{object.cryptocurrency_amount_}));
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ChatRevenueTransactionType &object) {
  td_api::downcast_call(const_cast<td_api::ChatRevenueTransactionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeEarnings &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeEarnings");
  jo("start_date", object.start_date_);
  jo("end_date", object.end_date_);
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeWithdrawal &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeWithdrawal");
  jo("withdrawal_date", object.withdrawal_date_);
  jo("provider", object.provider_);
  if (object.state_) {
    jo("state", ToJson(*object.state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactionTypeRefund &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactionTypeRefund");
  jo("refund_date", object.refund_date_);
  jo("provider", object.provider_);
}

void to_json(JsonValueScope &jv, const td_api::chatRevenueTransactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatRevenueTransactions");
  jo("total_count", object.total_count_);
  jo("transactions", ToJson(object.transactions_));
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

void to_json(JsonValueScope &jv, const td_api::chatStatisticsAdministratorActionsInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsAdministratorActionsInfo");
  jo("user_id", object.user_id_);
  jo("deleted_message_count", object.deleted_message_count_);
  jo("banned_user_count", object.banned_user_count_);
  jo("restricted_user_count", object.restricted_user_count_);
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

void to_json(JsonValueScope &jv, const td_api::chatStatisticsInviterInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsInviterInfo");
  jo("user_id", object.user_id_);
  jo("added_member_count", object.added_member_count_);
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsMessageSenderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsMessageSenderInfo");
  jo("user_id", object.user_id_);
  jo("sent_message_count", object.sent_message_count_);
  jo("average_character_count", object.average_character_count_);
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

void to_json(JsonValueScope &jv, const td_api::chatTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTheme");
  jo("name", object.name_);
  if (object.light_settings_) {
    jo("light_settings", ToJson(*object.light_settings_));
  }
  if (object.dark_settings_) {
    jo("dark_settings", ToJson(*object.dark_settings_));
  }
}

void to_json(JsonValueScope &jv, const td_api::ChatTransactionPurpose &object) {
  td_api::downcast_call(const_cast<td_api::ChatTransactionPurpose &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::chatTransactionPurposePaidMedia &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTransactionPurposePaidMedia");
  jo("message_id", object.message_id_);
  jo("media", ToJson(object.media_));
}

void to_json(JsonValueScope &jv, const td_api::chatTransactionPurposeJoin &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTransactionPurposeJoin");
  jo("period", object.period_);
}

void to_json(JsonValueScope &jv, const td_api::chatTransactionPurposeReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTransactionPurposeReaction");
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::chatTransactionPurposeGiveaway &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatTransactionPurposeGiveaway");
  jo("giveaway_message_id", object.giveaway_message_id_);
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

void to_json(JsonValueScope &jv, const td_api::chats &object) {
  auto jo = jv.enter_object();
  jo("@type", "chats");
  jo("total_count", object.total_count_);
  jo("chat_ids", ToJson(object.chat_ids_));
}

void to_json(JsonValueScope &jv, const td_api::chatsNearby &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatsNearby");
  jo("users_nearby", ToJson(object.users_nearby_));
  jo("supergroups_nearby", ToJson(object.supergroups_nearby_));
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

void to_json(JsonValueScope &jv, const td_api::CheckStickerSetNameResult &object) {
  td_api::downcast_call(const_cast<td_api::CheckStickerSetNameResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::checkStickerSetNameResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkStickerSetNameResultOk");
}

void to_json(JsonValueScope &jv, const td_api::checkStickerSetNameResultNameInvalid &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkStickerSetNameResultNameInvalid");
}

void to_json(JsonValueScope &jv, const td_api::checkStickerSetNameResultNameOccupied &object) {
  auto jo = jv.enter_object();
  jo("@type", "checkStickerSetNameResultNameOccupied");
}

void to_json(JsonValueScope &jv, const td_api::closeBirthdayUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "closeBirthdayUser");
  jo("user_id", object.user_id_);
  if (object.birthdate_) {
    jo("birthdate", ToJson(*object.birthdate_));
  }
}

void to_json(JsonValueScope &jv, const td_api::closedVectorPath &object) {
  auto jo = jv.enter_object();
  jo("@type", "closedVectorPath");
  jo("commands", ToJson(object.commands_));
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

void to_json(JsonValueScope &jv, const td_api::connectedWebsites &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectedWebsites");
  jo("websites", ToJson(object.websites_));
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

void to_json(JsonValueScope &jv, const td_api::contact &object) {
  auto jo = jv.enter_object();
  jo("@type", "contact");
  jo("phone_number", object.phone_number_);
  jo("first_name", object.first_name_);
  jo("last_name", object.last_name_);
  jo("vcard", object.vcard_);
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::count &object) {
  auto jo = jv.enter_object();
  jo("@type", "count");
  jo("count", object.count_);
}

void to_json(JsonValueScope &jv, const td_api::countries &object) {
  auto jo = jv.enter_object();
  jo("@type", "countries");
  jo("countries", ToJson(object.countries_));
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

void to_json(JsonValueScope &jv, const td_api::createdBasicGroupChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "createdBasicGroupChat");
  jo("chat_id", object.chat_id_);
  if (object.failed_to_add_members_) {
    jo("failed_to_add_members", ToJson(*object.failed_to_add_members_));
  }
}

void to_json(JsonValueScope &jv, const td_api::currentWeather &object) {
  auto jo = jv.enter_object();
  jo("@type", "currentWeather");
  jo("temperature", object.temperature_);
  jo("emoji", object.emoji_);
}

void to_json(JsonValueScope &jv, const td_api::customRequestResult &object) {
  auto jo = jv.enter_object();
  jo("@type", "customRequestResult");
  jo("result", object.result_);
}

void to_json(JsonValueScope &jv, const td_api::databaseStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "databaseStatistics");
  jo("statistics", object.statistics_);
}

void to_json(JsonValueScope &jv, const td_api::date &object) {
  auto jo = jv.enter_object();
  jo("@type", "date");
  jo("day", object.day_);
  jo("month", object.month_);
  jo("year", object.year_);
}

void to_json(JsonValueScope &jv, const td_api::dateRange &object) {
  auto jo = jv.enter_object();
  jo("@type", "dateRange");
  jo("start_date", object.start_date_);
  jo("end_date", object.end_date_);
}

void to_json(JsonValueScope &jv, const td_api::datedFile &object) {
  auto jo = jv.enter_object();
  jo("@type", "datedFile");
  if (object.file_) {
    jo("file", ToJson(*object.file_));
  }
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::deepLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "deepLinkInfo");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("need_update_application", JsonBool{object.need_update_application_});
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

void to_json(JsonValueScope &jv, const td_api::downloadedFileCounts &object) {
  auto jo = jv.enter_object();
  jo("@type", "downloadedFileCounts");
  jo("active_count", object.active_count_);
  jo("paused_count", object.paused_count_);
  jo("completed_count", object.completed_count_);
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
}

void to_json(JsonValueScope &jv, const td_api::emailAddressAuthenticationCodeInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "emailAddressAuthenticationCodeInfo");
  jo("email_address_pattern", object.email_address_pattern_);
  jo("length", object.length_);
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

void to_json(JsonValueScope &jv, const td_api::emojiCategories &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiCategories");
  jo("categories", ToJson(object.categories_));
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

void to_json(JsonValueScope &jv, const td_api::emojiKeyword &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiKeyword");
  jo("emoji", object.emoji_);
  jo("keyword", object.keyword_);
}

void to_json(JsonValueScope &jv, const td_api::emojiKeywords &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiKeywords");
  jo("emoji_keywords", ToJson(object.emoji_keywords_));
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

void to_json(JsonValueScope &jv, const td_api::emojiStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatus");
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
  jo("expiration_date", object.expiration_date_);
}

void to_json(JsonValueScope &jv, const td_api::emojiStatuses &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatuses");
  jo("custom_emoji_ids", ToJson(JsonVectorInt64{object.custom_emoji_ids_}));
}

void to_json(JsonValueScope &jv, const td_api::emojis &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojis");
  jo("emojis", ToJson(object.emojis_));
}

void to_json(JsonValueScope &jv, const td_api::encryptedCredentials &object) {
  auto jo = jv.enter_object();
  jo("@type", "encryptedCredentials");
  jo("data", base64_encode(object.data_));
  jo("hash", base64_encode(object.hash_));
  jo("secret", base64_encode(object.secret_));
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

void to_json(JsonValueScope &jv, const td_api::error &object) {
  auto jo = jv.enter_object();
  jo("@type", "error");
  jo("code", object.code_);
  jo("message", object.message_);
}

void to_json(JsonValueScope &jv, const td_api::factCheck &object) {
  auto jo = jv.enter_object();
  jo("@type", "factCheck");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("country_code", object.country_code_);
}

void to_json(JsonValueScope &jv, const td_api::failedToAddMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "failedToAddMember");
  jo("user_id", object.user_id_);
  jo("premium_would_allow_invite", JsonBool{object.premium_would_allow_invite_});
  jo("premium_required_to_send_messages", JsonBool{object.premium_required_to_send_messages_});
}

void to_json(JsonValueScope &jv, const td_api::failedToAddMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "failedToAddMembers");
  jo("failed_to_add_members", ToJson(object.failed_to_add_members_));
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

void to_json(JsonValueScope &jv, const td_api::fileDownloadedPrefixSize &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileDownloadedPrefixSize");
  jo("size", object.size_);
}

void to_json(JsonValueScope &jv, const td_api::filePart &object) {
  auto jo = jv.enter_object();
  jo("@type", "filePart");
  jo("data", base64_encode(object.data_));
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

void to_json(JsonValueScope &jv, const td_api::formattedText &object) {
  auto jo = jv.enter_object();
  jo("@type", "formattedText");
  jo("text", object.text_);
  jo("entities", ToJson(object.entities_));
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

void to_json(JsonValueScope &jv, const td_api::forumTopicIcon &object) {
  auto jo = jv.enter_object();
  jo("@type", "forumTopicIcon");
  jo("color", object.color_);
  jo("custom_emoji_id", ToJson(JsonInt64{object.custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::forumTopicInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "forumTopicInfo");
  jo("message_thread_id", object.message_thread_id_);
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
}

void to_json(JsonValueScope &jv, const td_api::forumTopics &object) {
  auto jo = jv.enter_object();
  jo("@type", "forumTopics");
  jo("total_count", object.total_count_);
  jo("topics", ToJson(object.topics_));
  jo("next_offset_date", object.next_offset_date_);
  jo("next_offset_message_id", object.next_offset_message_id_);
  jo("next_offset_message_thread_id", object.next_offset_message_thread_id_);
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

void to_json(JsonValueScope &jv, const td_api::foundChatBoosts &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundChatBoosts");
  jo("total_count", object.total_count_);
  jo("boosts", ToJson(object.boosts_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::foundChatMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundChatMessages");
  jo("total_count", object.total_count_);
  jo("messages", ToJson(object.messages_));
  jo("next_from_message_id", object.next_from_message_id_);
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

void to_json(JsonValueScope &jv, const td_api::foundMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundMessages");
  jo("total_count", object.total_count_);
  jo("messages", ToJson(object.messages_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::foundPosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundPosition");
  jo("position", object.position_);
}

void to_json(JsonValueScope &jv, const td_api::foundPositions &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundPositions");
  jo("total_count", object.total_count_);
  jo("positions", ToJson(object.positions_));
}

void to_json(JsonValueScope &jv, const td_api::foundStories &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundStories");
  jo("total_count", object.total_count_);
  jo("stories", ToJson(object.stories_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::foundUsers &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundUsers");
  jo("user_ids", ToJson(object.user_ids_));
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

void to_json(JsonValueScope &jv, const td_api::game &object) {
  auto jo = jv.enter_object();
  jo("@type", "game");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("short_name", object.short_name_);
  jo("title", object.title_);
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("description", object.description_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
}

void to_json(JsonValueScope &jv, const td_api::gameHighScore &object) {
  auto jo = jv.enter_object();
  jo("@type", "gameHighScore");
  jo("position", object.position_);
  jo("user_id", object.user_id_);
  jo("score", object.score_);
}

void to_json(JsonValueScope &jv, const td_api::gameHighScores &object) {
  auto jo = jv.enter_object();
  jo("@type", "gameHighScores");
  jo("scores", ToJson(object.scores_));
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

void to_json(JsonValueScope &jv, const td_api::giveawayParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "giveawayParameters");
  jo("boosted_chat_id", object.boosted_chat_id_);
  jo("additional_chat_ids", ToJson(object.additional_chat_ids_));
  jo("winners_selection_date", object.winners_selection_date_);
  jo("only_new_members", JsonBool{object.only_new_members_});
  jo("has_public_winners", JsonBool{object.has_public_winners_});
  jo("country_codes", ToJson(object.country_codes_));
  jo("prize_description", object.prize_description_);
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

void to_json(JsonValueScope &jv, const td_api::groupCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCall");
  jo("id", object.id_);
  jo("title", object.title_);
  jo("scheduled_start_date", object.scheduled_start_date_);
  jo("enabled_start_notification", JsonBool{object.enabled_start_notification_});
  jo("is_active", JsonBool{object.is_active_});
  jo("is_rtmp_stream", JsonBool{object.is_rtmp_stream_});
  jo("is_joined", JsonBool{object.is_joined_});
  jo("need_rejoin", JsonBool{object.need_rejoin_});
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
  jo("record_duration", object.record_duration_);
  jo("is_video_recorded", JsonBool{object.is_video_recorded_});
  jo("duration", object.duration_);
}

void to_json(JsonValueScope &jv, const td_api::groupCallId &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallId");
  jo("id", object.id_);
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

void to_json(JsonValueScope &jv, const td_api::groupCallParticipantVideoInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallParticipantVideoInfo");
  jo("source_groups", ToJson(object.source_groups_));
  jo("endpoint_id", object.endpoint_id_);
  jo("is_paused", JsonBool{object.is_paused_});
}

void to_json(JsonValueScope &jv, const td_api::groupCallRecentSpeaker &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallRecentSpeaker");
  if (object.participant_id_) {
    jo("participant_id", ToJson(*object.participant_id_));
  }
  jo("is_speaking", JsonBool{object.is_speaking_});
}

void to_json(JsonValueScope &jv, const td_api::groupCallStream &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallStream");
  jo("channel_id", object.channel_id_);
  jo("scale", object.scale_);
  jo("time_offset", object.time_offset_);
}

void to_json(JsonValueScope &jv, const td_api::groupCallStreams &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallStreams");
  jo("streams", ToJson(object.streams_));
}

void to_json(JsonValueScope &jv, const td_api::groupCallVideoSourceGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallVideoSourceGroup");
  jo("semantics", object.semantics_);
  jo("source_ids", ToJson(object.source_ids_));
}

void to_json(JsonValueScope &jv, const td_api::hashtags &object) {
  auto jo = jv.enter_object();
  jo("@type", "hashtags");
  jo("hashtags", ToJson(object.hashtags_));
}

void to_json(JsonValueScope &jv, const td_api::httpUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "httpUrl");
  jo("url", object.url_);
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

void to_json(JsonValueScope &jv, const td_api::importedContacts &object) {
  auto jo = jv.enter_object();
  jo("@type", "importedContacts");
  jo("user_ids", ToJson(object.user_ids_));
  jo("importer_count", ToJson(object.importer_count_));
}

void to_json(JsonValueScope &jv, const td_api::inlineKeyboardButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineKeyboardButton");
  jo("text", object.text_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
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

void to_json(JsonValueScope &jv, const td_api::InlineQueryResult &object) {
  td_api::downcast_call(const_cast<td_api::InlineQueryResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultArticle &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultArticle");
  jo("id", object.id_);
  jo("url", object.url_);
  jo("hide_url", JsonBool{object.hide_url_});
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

void to_json(JsonValueScope &jv, const td_api::inlineQueryResults &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResults");
  jo("inline_query_id", ToJson(JsonInt64{object.inline_query_id_}));
  if (object.button_) {
    jo("button", ToJson(*object.button_));
  }
  jo("results", ToJson(object.results_));
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::inlineQueryResultsButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "inlineQueryResultsButton");
  jo("text", object.text_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
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
  jo("story_sender_chat_id", object.story_sender_chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::inputMessageForwarded &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageForwarded");
  jo("from_chat_id", object.from_chat_id_);
  jo("message_id", object.message_id_);
  jo("in_game_share", JsonBool{object.in_game_share_});
  if (object.copy_options_) {
    jo("copy_options", ToJson(*object.copy_options_));
  }
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
}

void to_json(JsonValueScope &jv, const td_api::inputMessageReplyToExternalMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageReplyToExternalMessage");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.quote_) {
    jo("quote", ToJson(*object.quote_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessageReplyToStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessageReplyToStory");
  jo("story_sender_chat_id", object.story_sender_chat_id_);
  jo("story_id", object.story_id_);
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

void to_json(JsonValueScope &jv, const td_api::InputPaidMediaType &object) {
  td_api::downcast_call(const_cast<td_api::InputPaidMediaType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::inputPaidMediaTypePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputPaidMediaTypePhoto");
}

void to_json(JsonValueScope &jv, const td_api::inputPaidMediaTypeVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputPaidMediaTypeVideo");
  jo("duration", object.duration_);
  jo("supports_streaming", JsonBool{object.supports_streaming_});
}

void to_json(JsonValueScope &jv, const td_api::inputTextQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputTextQuote");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("position", object.position_);
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
  jo("is_compact", JsonBool{object.is_compact_});
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
  jo("story_sender_username", object.story_sender_username_);
  jo("story_id", object.story_id_);
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
  jo("is_compact", JsonBool{object.is_compact_});
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

void to_json(JsonValueScope &jv, const td_api::invoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "invoice");
  jo("currency", object.currency_);
  jo("price_parts", ToJson(object.price_parts_));
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

void to_json(JsonValueScope &jv, const td_api::jsonObjectMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonObjectMember");
  jo("key", object.key_);
  if (object.value_) {
    jo("value", ToJson(*object.value_));
  }
}

void to_json(JsonValueScope &jv, const td_api::JsonValue &object) {
  td_api::downcast_call(const_cast<td_api::JsonValue &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::jsonValueNull &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonValueNull");
}

void to_json(JsonValueScope &jv, const td_api::jsonValueBoolean &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonValueBoolean");
  jo("value", JsonBool{object.value_});
}

void to_json(JsonValueScope &jv, const td_api::jsonValueNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonValueNumber");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::jsonValueString &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonValueString");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::jsonValueArray &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonValueArray");
  jo("values", ToJson(object.values_));
}

void to_json(JsonValueScope &jv, const td_api::jsonValueObject &object) {
  auto jo = jv.enter_object();
  jo("@type", "jsonValueObject");
  jo("members", ToJson(object.members_));
}

void to_json(JsonValueScope &jv, const td_api::keyboardButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButton");
  jo("text", object.text_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
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

void to_json(JsonValueScope &jv, const td_api::labeledPricePart &object) {
  auto jo = jv.enter_object();
  jo("@type", "labeledPricePart");
  jo("label", object.label_);
  jo("amount", object.amount_);
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

void to_json(JsonValueScope &jv, const td_api::languagePackString &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackString");
  jo("key", object.key_);
  if (object.value_) {
    jo("value", ToJson(*object.value_));
  }
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

void to_json(JsonValueScope &jv, const td_api::languagePackStrings &object) {
  auto jo = jv.enter_object();
  jo("@type", "languagePackStrings");
  jo("strings", ToJson(object.strings_));
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

void to_json(JsonValueScope &jv, const td_api::LinkPreviewAlbumMedia &object) {
  td_api::downcast_call(const_cast<td_api::LinkPreviewAlbumMedia &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewAlbumMediaPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewAlbumMediaPhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewAlbumMediaVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewAlbumMediaVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
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
  jo("story_sender_chat_id", object.story_sender_chat_id_);
  jo("story_id", object.story_id_);
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
}

void to_json(JsonValueScope &jv, const td_api::linkPreviewTypeVideoChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "linkPreviewTypeVideoChat");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("is_live_stream", JsonBool{object.is_live_stream_});
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

void to_json(JsonValueScope &jv, const td_api::localizationTargetInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "localizationTargetInfo");
  jo("language_packs", ToJson(object.language_packs_));
}

void to_json(JsonValueScope &jv, const td_api::location &object) {
  auto jo = jv.enter_object();
  jo("@type", "location");
  jo("latitude", object.latitude_);
  jo("longitude", object.longitude_);
  jo("horizontal_accuracy", object.horizontal_accuracy_);
}

void to_json(JsonValueScope &jv, const td_api::locationAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "locationAddress");
  jo("country_code", object.country_code_);
  jo("state", object.state_);
  jo("city", object.city_);
  jo("street", object.street_);
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

void to_json(JsonValueScope &jv, const td_api::logTags &object) {
  auto jo = jv.enter_object();
  jo("@type", "logTags");
  jo("tags", ToJson(object.tags_));
}

void to_json(JsonValueScope &jv, const td_api::logVerbosityLevel &object) {
  auto jo = jv.enter_object();
  jo("@type", "logVerbosityLevel");
  jo("verbosity_level", object.verbosity_level_);
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

void to_json(JsonValueScope &jv, const td_api::mainWebApp &object) {
  auto jo = jv.enter_object();
  jo("@type", "mainWebApp");
  jo("url", object.url_);
  jo("is_compact", JsonBool{object.is_compact_});
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
  jo("is_topic_message", JsonBool{object.is_topic_message_});
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
  if (object.reply_to_) {
    jo("reply_to", ToJson(*object.reply_to_));
  }
  jo("message_thread_id", object.message_thread_id_);
  jo("saved_messages_topic_id", object.saved_messages_topic_id_);
  if (object.self_destruct_type_) {
    jo("self_destruct_type", ToJson(*object.self_destruct_type_));
  }
  jo("self_destruct_in", object.self_destruct_in_);
  jo("auto_delete_in", object.auto_delete_in_);
  jo("via_bot_user_id", object.via_bot_user_id_);
  jo("sender_business_bot_user_id", object.sender_business_bot_user_id_);
  jo("sender_boost_count", object.sender_boost_count_);
  jo("author_signature", object.author_signature_);
  jo("media_album_id", ToJson(JsonInt64{object.media_album_id_}));
  jo("effect_id", ToJson(JsonInt64{object.effect_id_}));
  jo("has_sensitive_content", JsonBool{object.has_sensitive_content_});
  jo("restriction_reason", object.restriction_reason_);
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
  if (object.reply_markup_) {
    jo("reply_markup", ToJson(*object.reply_markup_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageAutoDeleteTime &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageAutoDeleteTime");
  jo("time", object.time_);
}

void to_json(JsonValueScope &jv, const td_api::messageCalendar &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageCalendar");
  jo("total_count", object.total_count_);
  jo("days", ToJson(object.days_));
}

void to_json(JsonValueScope &jv, const td_api::messageCalendarDay &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageCalendarDay");
  jo("total_count", object.total_count_);
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
}

void to_json(JsonValueScope &jv, const td_api::MessageContent &object) {
  td_api::downcast_call(const_cast<td_api::MessageContent &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageText &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageText");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  if (object.link_preview_) {
    jo("link_preview", ToJson(*object.link_preview_));
  }
  if (object.link_preview_options_) {
    jo("link_preview_options", ToJson(*object.link_preview_options_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageAnimation &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageAnimation");
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
  jo("has_spoiler", JsonBool{object.has_spoiler_});
  jo("is_secret", JsonBool{object.is_secret_});
}

void to_json(JsonValueScope &jv, const td_api::messageAudio &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageAudio");
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageDocument");
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messagePaidMedia &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePaidMedia");
  jo("star_count", object.star_count_);
  jo("media", ToJson(object.media_));
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
}

void to_json(JsonValueScope &jv, const td_api::messagePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
  jo("has_spoiler", JsonBool{object.has_spoiler_});
  jo("is_secret", JsonBool{object.is_secret_});
}

void to_json(JsonValueScope &jv, const td_api::messageSticker &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSticker");
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  jo("is_premium", JsonBool{object.is_premium_});
}

void to_json(JsonValueScope &jv, const td_api::messageVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("show_caption_above_media", JsonBool{object.show_caption_above_media_});
  jo("has_spoiler", JsonBool{object.has_spoiler_});
  jo("is_secret", JsonBool{object.is_secret_});
}

void to_json(JsonValueScope &jv, const td_api::messageVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageVideoNote");
  if (object.video_note_) {
    jo("video_note", ToJson(*object.video_note_));
  }
  jo("is_viewed", JsonBool{object.is_viewed_});
  jo("is_secret", JsonBool{object.is_secret_});
}

void to_json(JsonValueScope &jv, const td_api::messageVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageVoiceNote");
  if (object.voice_note_) {
    jo("voice_note", ToJson(*object.voice_note_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("is_listened", JsonBool{object.is_listened_});
}

void to_json(JsonValueScope &jv, const td_api::messageExpiredPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExpiredPhoto");
}

void to_json(JsonValueScope &jv, const td_api::messageExpiredVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExpiredVideo");
}

void to_json(JsonValueScope &jv, const td_api::messageExpiredVideoNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExpiredVideoNote");
}

void to_json(JsonValueScope &jv, const td_api::messageExpiredVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExpiredVoiceNote");
}

void to_json(JsonValueScope &jv, const td_api::messageLocation &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageLocation");
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  jo("live_period", object.live_period_);
  jo("expires_in", object.expires_in_);
  jo("heading", object.heading_);
  jo("proximity_alert_radius", object.proximity_alert_radius_);
}

void to_json(JsonValueScope &jv, const td_api::messageVenue &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageVenue");
  if (object.venue_) {
    jo("venue", ToJson(*object.venue_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageContact &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageContact");
  if (object.contact_) {
    jo("contact", ToJson(*object.contact_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageAnimatedEmoji &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageAnimatedEmoji");
  if (object.animated_emoji_) {
    jo("animated_emoji", ToJson(*object.animated_emoji_));
  }
  jo("emoji", object.emoji_);
}

void to_json(JsonValueScope &jv, const td_api::messageDice &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageDice");
  if (object.initial_state_) {
    jo("initial_state", ToJson(*object.initial_state_));
  }
  if (object.final_state_) {
    jo("final_state", ToJson(*object.final_state_));
  }
  jo("emoji", object.emoji_);
  jo("value", object.value_);
  jo("success_animation_frame_number", object.success_animation_frame_number_);
}

void to_json(JsonValueScope &jv, const td_api::messageGame &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGame");
  if (object.game_) {
    jo("game", ToJson(*object.game_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messagePoll &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePoll");
  if (object.poll_) {
    jo("poll", ToJson(*object.poll_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageStory");
  jo("story_sender_chat_id", object.story_sender_chat_id_);
  jo("story_id", object.story_id_);
  jo("via_mention", JsonBool{object.via_mention_});
}

void to_json(JsonValueScope &jv, const td_api::messageInvoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageInvoice");
  if (object.product_info_) {
    jo("product_info", ToJson(*object.product_info_));
  }
  jo("currency", object.currency_);
  jo("total_amount", object.total_amount_);
  jo("start_parameter", object.start_parameter_);
  jo("is_test", JsonBool{object.is_test_});
  jo("need_shipping_address", JsonBool{object.need_shipping_address_});
  jo("receipt_message_id", object.receipt_message_id_);
  if (object.paid_media_) {
    jo("paid_media", ToJson(*object.paid_media_));
  }
  if (object.paid_media_caption_) {
    jo("paid_media_caption", ToJson(*object.paid_media_caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageCall");
  jo("is_video", JsonBool{object.is_video_});
  if (object.discard_reason_) {
    jo("discard_reason", ToJson(*object.discard_reason_));
  }
  jo("duration", object.duration_);
}

void to_json(JsonValueScope &jv, const td_api::messageVideoChatScheduled &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageVideoChatScheduled");
  jo("group_call_id", object.group_call_id_);
  jo("start_date", object.start_date_);
}

void to_json(JsonValueScope &jv, const td_api::messageVideoChatStarted &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageVideoChatStarted");
  jo("group_call_id", object.group_call_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageVideoChatEnded &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageVideoChatEnded");
  jo("duration", object.duration_);
}

void to_json(JsonValueScope &jv, const td_api::messageInviteVideoChatParticipants &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageInviteVideoChatParticipants");
  jo("group_call_id", object.group_call_id_);
  jo("user_ids", ToJson(object.user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::messageBasicGroupChatCreate &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageBasicGroupChatCreate");
  jo("title", object.title_);
  jo("member_user_ids", ToJson(object.member_user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::messageSupergroupChatCreate &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSupergroupChatCreate");
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatChangeTitle &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatChangeTitle");
  jo("title", object.title_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatChangePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatChangePhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageChatDeletePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatDeletePhoto");
}

void to_json(JsonValueScope &jv, const td_api::messageChatAddMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatAddMembers");
  jo("member_user_ids", ToJson(object.member_user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::messageChatJoinByLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatJoinByLink");
}

void to_json(JsonValueScope &jv, const td_api::messageChatJoinByRequest &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatJoinByRequest");
}

void to_json(JsonValueScope &jv, const td_api::messageChatDeleteMember &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatDeleteMember");
  jo("user_id", object.user_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatUpgradeTo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatUpgradeTo");
  jo("supergroup_id", object.supergroup_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatUpgradeFrom &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatUpgradeFrom");
  jo("title", object.title_);
  jo("basic_group_id", object.basic_group_id_);
}

void to_json(JsonValueScope &jv, const td_api::messagePinMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePinMessage");
  jo("message_id", object.message_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageScreenshotTaken &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageScreenshotTaken");
}

void to_json(JsonValueScope &jv, const td_api::messageChatSetBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatSetBackground");
  jo("old_background_message_id", object.old_background_message_id_);
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  jo("only_for_self", JsonBool{object.only_for_self_});
}

void to_json(JsonValueScope &jv, const td_api::messageChatSetTheme &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatSetTheme");
  jo("theme_name", object.theme_name_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatSetMessageAutoDeleteTime &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatSetMessageAutoDeleteTime");
  jo("message_auto_delete_time", object.message_auto_delete_time_);
  jo("from_user_id", object.from_user_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatBoost &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatBoost");
  jo("boost_count", object.boost_count_);
}

void to_json(JsonValueScope &jv, const td_api::messageForumTopicCreated &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForumTopicCreated");
  jo("name", object.name_);
  if (object.icon_) {
    jo("icon", ToJson(*object.icon_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageForumTopicEdited &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForumTopicEdited");
  jo("name", object.name_);
  jo("edit_icon_custom_emoji_id", JsonBool{object.edit_icon_custom_emoji_id_});
  jo("icon_custom_emoji_id", ToJson(JsonInt64{object.icon_custom_emoji_id_}));
}

void to_json(JsonValueScope &jv, const td_api::messageForumTopicIsClosedToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForumTopicIsClosedToggled");
  jo("is_closed", JsonBool{object.is_closed_});
}

void to_json(JsonValueScope &jv, const td_api::messageForumTopicIsHiddenToggled &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForumTopicIsHiddenToggled");
  jo("is_hidden", JsonBool{object.is_hidden_});
}

void to_json(JsonValueScope &jv, const td_api::messageSuggestProfilePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSuggestProfilePhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageCustomServiceAction &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageCustomServiceAction");
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::messageGameScore &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGameScore");
  jo("game_message_id", object.game_message_id_);
  jo("game_id", ToJson(JsonInt64{object.game_id_}));
  jo("score", object.score_);
}

void to_json(JsonValueScope &jv, const td_api::messagePaymentSuccessful &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePaymentSuccessful");
  jo("invoice_chat_id", object.invoice_chat_id_);
  jo("invoice_message_id", object.invoice_message_id_);
  jo("currency", object.currency_);
  jo("total_amount", object.total_amount_);
  jo("is_recurring", JsonBool{object.is_recurring_});
  jo("is_first_recurring", JsonBool{object.is_first_recurring_});
  jo("invoice_name", object.invoice_name_);
}

void to_json(JsonValueScope &jv, const td_api::messagePaymentSuccessfulBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePaymentSuccessfulBot");
  jo("currency", object.currency_);
  jo("total_amount", object.total_amount_);
  jo("is_recurring", JsonBool{object.is_recurring_});
  jo("is_first_recurring", JsonBool{object.is_first_recurring_});
  jo("invoice_payload", base64_encode(object.invoice_payload_));
  jo("shipping_option_id", object.shipping_option_id_);
  if (object.order_info_) {
    jo("order_info", ToJson(*object.order_info_));
  }
  jo("telegram_payment_charge_id", object.telegram_payment_charge_id_);
  jo("provider_payment_charge_id", object.provider_payment_charge_id_);
}

void to_json(JsonValueScope &jv, const td_api::messagePaymentRefunded &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePaymentRefunded");
  if (object.owner_id_) {
    jo("owner_id", ToJson(*object.owner_id_));
  }
  jo("currency", object.currency_);
  jo("total_amount", object.total_amount_);
  jo("invoice_payload", base64_encode(object.invoice_payload_));
  jo("telegram_payment_charge_id", object.telegram_payment_charge_id_);
  jo("provider_payment_charge_id", object.provider_payment_charge_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageGiftedPremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiftedPremium");
  jo("gifter_user_id", object.gifter_user_id_);
  jo("receiver_user_id", object.receiver_user_id_);
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("cryptocurrency", object.cryptocurrency_);
  jo("cryptocurrency_amount", ToJson(JsonInt64{object.cryptocurrency_amount_}));
  jo("month_count", object.month_count_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messagePremiumGiftCode &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePremiumGiftCode");
  if (object.creator_id_) {
    jo("creator_id", ToJson(*object.creator_id_));
  }
  jo("is_from_giveaway", JsonBool{object.is_from_giveaway_});
  jo("is_unclaimed", JsonBool{object.is_unclaimed_});
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("cryptocurrency", object.cryptocurrency_);
  jo("cryptocurrency_amount", ToJson(JsonInt64{object.cryptocurrency_amount_}));
  jo("month_count", object.month_count_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  jo("code", object.code_);
}

void to_json(JsonValueScope &jv, const td_api::messageGiveawayCreated &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiveawayCreated");
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::messageGiveaway &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiveaway");
  if (object.parameters_) {
    jo("parameters", ToJson(*object.parameters_));
  }
  jo("winner_count", object.winner_count_);
  if (object.prize_) {
    jo("prize", ToJson(*object.prize_));
  }
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageGiveawayCompleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiveawayCompleted");
  jo("giveaway_message_id", object.giveaway_message_id_);
  jo("winner_count", object.winner_count_);
  jo("is_star_giveaway", JsonBool{object.is_star_giveaway_});
  jo("unclaimed_prize_count", object.unclaimed_prize_count_);
}

void to_json(JsonValueScope &jv, const td_api::messageGiveawayWinners &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiveawayWinners");
  jo("boosted_chat_id", object.boosted_chat_id_);
  jo("giveaway_message_id", object.giveaway_message_id_);
  jo("additional_chat_count", object.additional_chat_count_);
  jo("actual_winners_selection_date", object.actual_winners_selection_date_);
  jo("only_new_members", JsonBool{object.only_new_members_});
  jo("was_refunded", JsonBool{object.was_refunded_});
  if (object.prize_) {
    jo("prize", ToJson(*object.prize_));
  }
  jo("prize_description", object.prize_description_);
  jo("winner_count", object.winner_count_);
  jo("winner_user_ids", ToJson(object.winner_user_ids_));
  jo("unclaimed_prize_count", object.unclaimed_prize_count_);
}

void to_json(JsonValueScope &jv, const td_api::messageGiftedStars &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiftedStars");
  jo("gifter_user_id", object.gifter_user_id_);
  jo("receiver_user_id", object.receiver_user_id_);
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("cryptocurrency", object.cryptocurrency_);
  jo("cryptocurrency_amount", ToJson(JsonInt64{object.cryptocurrency_amount_}));
  jo("star_count", object.star_count_);
  jo("transaction_id", object.transaction_id_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageGiveawayPrizeStars &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiveawayPrizeStars");
  jo("star_count", object.star_count_);
  jo("transaction_id", object.transaction_id_);
  jo("boosted_chat_id", object.boosted_chat_id_);
  jo("giveaway_message_id", object.giveaway_message_id_);
  jo("is_unclaimed", JsonBool{object.is_unclaimed_});
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageContactRegistered &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageContactRegistered");
}

void to_json(JsonValueScope &jv, const td_api::messageUsersShared &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageUsersShared");
  jo("users", ToJson(object.users_));
  jo("button_id", object.button_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatShared &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatShared");
  if (object.chat_) {
    jo("chat", ToJson(*object.chat_));
  }
  jo("button_id", object.button_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageBotWriteAccessAllowed &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageBotWriteAccessAllowed");
  if (object.reason_) {
    jo("reason", ToJson(*object.reason_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageWebAppDataSent &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageWebAppDataSent");
  jo("button_text", object.button_text_);
}

void to_json(JsonValueScope &jv, const td_api::messageWebAppDataReceived &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageWebAppDataReceived");
  jo("button_text", object.button_text_);
  jo("data", object.data_);
}

void to_json(JsonValueScope &jv, const td_api::messagePassportDataSent &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePassportDataSent");
  jo("types", ToJson(object.types_));
}

void to_json(JsonValueScope &jv, const td_api::messagePassportDataReceived &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePassportDataReceived");
  jo("elements", ToJson(object.elements_));
  if (object.credentials_) {
    jo("credentials", ToJson(*object.credentials_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageProximityAlertTriggered &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageProximityAlertTriggered");
  if (object.traveler_id_) {
    jo("traveler_id", ToJson(*object.traveler_id_));
  }
  if (object.watcher_id_) {
    jo("watcher_id", ToJson(*object.watcher_id_));
  }
  jo("distance", object.distance_);
}

void to_json(JsonValueScope &jv, const td_api::messageUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageUnsupported");
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

void to_json(JsonValueScope &jv, const td_api::messageImportInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageImportInfo");
  jo("sender_name", object.sender_name_);
  jo("date", object.date_);
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

void to_json(JsonValueScope &jv, const td_api::messageLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageLink");
  jo("link", object.link_);
  jo("is_public", JsonBool{object.is_public_});
}

void to_json(JsonValueScope &jv, const td_api::messageLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageLinkInfo");
  jo("is_public", JsonBool{object.is_public_});
  jo("chat_id", object.chat_id_);
  jo("message_thread_id", object.message_thread_id_);
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("media_timestamp", object.media_timestamp_);
  jo("for_album", JsonBool{object.for_album_});
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

void to_json(JsonValueScope &jv, const td_api::messagePosition &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePosition");
  jo("position", object.position_);
  jo("message_id", object.message_id_);
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::messagePositions &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePositions");
  jo("total_count", object.total_count_);
  jo("positions", ToJson(object.positions_));
}

void to_json(JsonValueScope &jv, const td_api::messageProperties &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageProperties");
  jo("can_be_copied_to_secret_chat", JsonBool{object.can_be_copied_to_secret_chat_});
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
  jo("can_edit_scheduling_state", JsonBool{object.can_edit_scheduling_state_});
  jo("can_get_embedding_code", JsonBool{object.can_get_embedding_code_});
  jo("can_get_link", JsonBool{object.can_get_link_});
  jo("can_get_media_timestamp_links", JsonBool{object.can_get_media_timestamp_links_});
  jo("can_get_message_thread", JsonBool{object.can_get_message_thread_});
  jo("can_get_read_date", JsonBool{object.can_get_read_date_});
  jo("can_get_statistics", JsonBool{object.can_get_statistics_});
  jo("can_get_viewers", JsonBool{object.can_get_viewers_});
  jo("can_recognize_speech", JsonBool{object.can_recognize_speech_});
  jo("can_report_chat", JsonBool{object.can_report_chat_});
  jo("can_report_reactions", JsonBool{object.can_report_reactions_});
  jo("can_report_supergroup_spam", JsonBool{object.can_report_supergroup_spam_});
  jo("can_set_fact_check", JsonBool{object.can_set_fact_check_});
  jo("need_show_statistics", JsonBool{object.need_show_statistics_});
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

void to_json(JsonValueScope &jv, const td_api::messageReactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReactions");
  jo("reactions", ToJson(object.reactions_));
  jo("are_tags", JsonBool{object.are_tags_});
  jo("paid_reactors", ToJson(object.paid_reactors_));
  jo("can_get_added_reactions", JsonBool{object.can_get_added_reactions_});
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

void to_json(JsonValueScope &jv, const td_api::messageReplyInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReplyInfo");
  jo("reply_count", object.reply_count_);
  jo("recent_replier_ids", ToJson(object.recent_replier_ids_));
  jo("last_read_inbox_message_id", object.last_read_inbox_message_id_);
  jo("last_read_outbox_message_id", object.last_read_outbox_message_id_);
  jo("last_message_id", object.last_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::MessageReplyTo &object) {
  td_api::downcast_call(const_cast<td_api::MessageReplyTo &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageReplyToMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReplyToMessage");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  if (object.quote_) {
    jo("quote", ToJson(*object.quote_));
  }
  if (object.origin_) {
    jo("origin", ToJson(*object.origin_));
  }
  jo("origin_send_date", object.origin_send_date_);
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageReplyToStory &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReplyToStory");
  jo("story_sender_chat_id", object.story_sender_chat_id_);
  jo("story_id", object.story_id_);
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

void to_json(JsonValueScope &jv, const td_api::messageSenders &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSenders");
  jo("total_count", object.total_count_);
  jo("senders", ToJson(object.senders_));
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
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::messageSponsor &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSponsor");
  jo("url", object.url_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("info", object.info_);
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

void to_json(JsonValueScope &jv, const td_api::messageViewer &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageViewer");
  jo("user_id", object.user_id_);
  jo("view_date", object.view_date_);
}

void to_json(JsonValueScope &jv, const td_api::messageViewers &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageViewers");
  jo("viewers", ToJson(object.viewers_));
}

void to_json(JsonValueScope &jv, const td_api::messages &object) {
  auto jo = jv.enter_object();
  jo("@type", "messages");
  jo("total_count", object.total_count_);
  jo("messages", ToJson(object.messages_));
}

void to_json(JsonValueScope &jv, const td_api::minithumbnail &object) {
  auto jo = jv.enter_object();
  jo("@type", "minithumbnail");
  jo("width", object.width_);
  jo("height", object.height_);
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::networkStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "networkStatistics");
  jo("since_date", object.since_date_);
  jo("entries", ToJson(object.entries_));
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

void to_json(JsonValueScope &jv, const td_api::newChatPrivacySettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "newChatPrivacySettings");
  jo("allow_new_chats_from_unknown_users", JsonBool{object.allow_new_chats_from_unknown_users_});
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

void to_json(JsonValueScope &jv, const td_api::notificationGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationGroup");
  jo("id", object.id_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("chat_id", object.chat_id_);
  jo("total_count", object.total_count_);
  jo("notifications", ToJson(object.notifications_));
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

void to_json(JsonValueScope &jv, const td_api::notificationSounds &object) {
  auto jo = jv.enter_object();
  jo("@type", "notificationSounds");
  jo("notification_sounds", ToJson(object.notification_sounds_));
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

void to_json(JsonValueScope &jv, const td_api::ok &object) {
  auto jo = jv.enter_object();
  jo("@type", "ok");
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

void to_json(JsonValueScope &jv, const td_api::pageBlockListItem &object) {
  auto jo = jv.enter_object();
  jo("@type", "pageBlockListItem");
  jo("label", object.label_);
  jo("page_blocks", ToJson(object.page_blocks_));
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
}

void to_json(JsonValueScope &jv, const td_api::paidMediaUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidMediaUnsupported");
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

void to_json(JsonValueScope &jv, const td_api::passportAuthorizationForm &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportAuthorizationForm");
  jo("id", object.id_);
  jo("required_elements", ToJson(object.required_elements_));
  jo("privacy_policy_url", object.privacy_policy_url_);
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

void to_json(JsonValueScope &jv, const td_api::passportElements &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElements");
  jo("elements", ToJson(object.elements_));
}

void to_json(JsonValueScope &jv, const td_api::passportElementsWithErrors &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportElementsWithErrors");
  jo("elements", ToJson(object.elements_));
  jo("errors", ToJson(object.errors_));
}

void to_json(JsonValueScope &jv, const td_api::passportRequiredElement &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportRequiredElement");
  jo("suitable_elements", ToJson(object.suitable_elements_));
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

void to_json(JsonValueScope &jv, const td_api::paymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentOption");
  jo("title", object.title_);
  jo("url", object.url_);
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

void to_json(JsonValueScope &jv, const td_api::paymentResult &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentResult");
  jo("success", JsonBool{object.success_});
  jo("verification_url", object.verification_url_);
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

void to_json(JsonValueScope &jv, const td_api::personalDocument &object) {
  auto jo = jv.enter_object();
  jo("@type", "personalDocument");
  jo("files", ToJson(object.files_));
  jo("translation", ToJson(object.translation_));
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

void to_json(JsonValueScope &jv, const td_api::photo &object) {
  auto jo = jv.enter_object();
  jo("@type", "photo");
  jo("has_stickers", JsonBool{object.has_stickers_});
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  jo("sizes", ToJson(object.sizes_));
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

void to_json(JsonValueScope &jv, const td_api::point &object) {
  auto jo = jv.enter_object();
  jo("@type", "point");
  jo("x", object.x_);
  jo("y", object.y_);
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

void to_json(JsonValueScope &jv, const td_api::PollType &object) {
  td_api::downcast_call(const_cast<td_api::PollType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::pollTypeRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "pollTypeRegular");
  jo("allow_multiple_answers", JsonBool{object.allow_multiple_answers_});
}

void to_json(JsonValueScope &jv, const td_api::pollTypeQuiz &object) {
  auto jo = jv.enter_object();
  jo("@type", "pollTypeQuiz");
  jo("correct_option_id", object.correct_option_id_);
  if (object.explanation_) {
    jo("explanation", ToJson(*object.explanation_));
  }
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

void to_json(JsonValueScope &jv, const td_api::premiumFeatures &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumFeatures");
  jo("features", ToJson(object.features_));
  jo("limits", ToJson(object.limits_));
  if (object.payment_link_) {
    jo("payment_link", ToJson(*object.payment_link_));
  }
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

void to_json(JsonValueScope &jv, const td_api::premiumGiftCodePaymentOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumGiftCodePaymentOption");
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("winner_count", object.winner_count_);
  jo("month_count", object.month_count_);
  jo("store_product_id", object.store_product_id_);
  jo("store_product_quantity", object.store_product_quantity_);
}

void to_json(JsonValueScope &jv, const td_api::premiumGiftCodePaymentOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumGiftCodePaymentOptions");
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::premiumLimit &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimit");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("default_value", object.default_value_);
  jo("premium_value", object.premium_value_);
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

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeWeeklySentStoryCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeWeeklySentStoryCount");
}

void to_json(JsonValueScope &jv, const td_api::premiumLimitTypeMonthlySentStoryCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimitTypeMonthlySentStoryCount");
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

void to_json(JsonValueScope &jv, const td_api::profileAccentColor &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileAccentColor");
  jo("id", object.id_);
  if (object.light_theme_colors_) {
    jo("light_theme_colors", ToJson(*object.light_theme_colors_));
  }
  if (object.dark_theme_colors_) {
    jo("dark_theme_colors", ToJson(*object.dark_theme_colors_));
  }
  jo("min_supergroup_chat_boost_level", object.min_supergroup_chat_boost_level_);
  jo("min_channel_chat_boost_level", object.min_channel_chat_boost_level_);
}

void to_json(JsonValueScope &jv, const td_api::profileAccentColors &object) {
  auto jo = jv.enter_object();
  jo("@type", "profileAccentColors");
  jo("palette_colors", ToJson(object.palette_colors_));
  jo("background_colors", ToJson(object.background_colors_));
  jo("story_colors", ToJson(object.story_colors_));
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

void to_json(JsonValueScope &jv, const td_api::proxies &object) {
  auto jo = jv.enter_object();
  jo("@type", "proxies");
  jo("proxies", ToJson(object.proxies_));
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

void to_json(JsonValueScope &jv, const td_api::publicForwards &object) {
  auto jo = jv.enter_object();
  jo("@type", "publicForwards");
  jo("total_count", object.total_count_);
  jo("forwards", ToJson(object.forwards_));
  jo("next_offset", object.next_offset_);
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
  jo("is_pinned", JsonBool{object.is_pinned_});
}

void to_json(JsonValueScope &jv, const td_api::pushMessageContentText &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushMessageContentText");
  jo("text", object.text_);
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
  jo("theme_name", object.theme_name_);
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

void to_json(JsonValueScope &jv, const td_api::pushReceiverId &object) {
  auto jo = jv.enter_object();
  jo("@type", "pushReceiverId");
  jo("id", ToJson(JsonInt64{object.id_}));
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

void to_json(JsonValueScope &jv, const td_api::quickReplyMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "quickReplyMessages");
  jo("messages", ToJson(object.messages_));
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

void to_json(JsonValueScope &jv, const td_api::ReactionUnavailabilityReason &object) {
  td_api::downcast_call(const_cast<td_api::ReactionUnavailabilityReason &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::reactionUnavailabilityReasonAnonymousAdministrator &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionUnavailabilityReasonAnonymousAdministrator");
}

void to_json(JsonValueScope &jv, const td_api::reactionUnavailabilityReasonGuest &object) {
  auto jo = jv.enter_object();
  jo("@type", "reactionUnavailabilityReasonGuest");
}

void to_json(JsonValueScope &jv, const td_api::readDatePrivacySettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "readDatePrivacySettings");
  jo("show_read_date", JsonBool{object.show_read_date_});
}

void to_json(JsonValueScope &jv, const td_api::recommendedChatFolder &object) {
  auto jo = jv.enter_object();
  jo("@type", "recommendedChatFolder");
  if (object.folder_) {
    jo("folder", ToJson(*object.folder_));
  }
  jo("description", object.description_);
}

void to_json(JsonValueScope &jv, const td_api::recommendedChatFolders &object) {
  auto jo = jv.enter_object();
  jo("@type", "recommendedChatFolders");
  jo("chat_folders", ToJson(object.chat_folders_));
}

void to_json(JsonValueScope &jv, const td_api::recoveryEmailAddress &object) {
  auto jo = jv.enter_object();
  jo("@type", "recoveryEmailAddress");
  jo("recovery_email_address", object.recovery_email_address_);
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

void to_json(JsonValueScope &jv, const td_api::reportChatSponsoredMessageOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatSponsoredMessageOption");
  jo("id", base64_encode(object.id_));
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::ReportChatSponsoredMessageResult &object) {
  td_api::downcast_call(const_cast<td_api::ReportChatSponsoredMessageResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::reportChatSponsoredMessageResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatSponsoredMessageResultOk");
}

void to_json(JsonValueScope &jv, const td_api::reportChatSponsoredMessageResultFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatSponsoredMessageResultFailed");
}

void to_json(JsonValueScope &jv, const td_api::reportChatSponsoredMessageResultOptionRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatSponsoredMessageResultOptionRequired");
  jo("title", object.title_);
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::reportChatSponsoredMessageResultAdsHidden &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatSponsoredMessageResultAdsHidden");
}

void to_json(JsonValueScope &jv, const td_api::reportChatSponsoredMessageResultPremiumRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatSponsoredMessageResultPremiumRequired");
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

void to_json(JsonValueScope &jv, const td_api::rtmpUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "rtmpUrl");
  jo("url", object.url_);
  jo("stream_key", object.stream_key_);
}

void to_json(JsonValueScope &jv, const td_api::savedCredentials &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedCredentials");
  jo("id", object.id_);
  jo("title", object.title_);
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

void to_json(JsonValueScope &jv, const td_api::savedMessagesTags &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedMessagesTags");
  jo("tags", ToJson(object.tags_));
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

void to_json(JsonValueScope &jv, const td_api::scopeAutosaveSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "scopeAutosaveSettings");
  jo("autosave_photos", JsonBool{object.autosave_photos_});
  jo("autosave_videos", JsonBool{object.autosave_videos_});
  jo("max_video_file_size", object.max_video_file_size_);
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
  jo("show_story_sender", JsonBool{object.show_story_sender_});
  jo("disable_pinned_message_notifications", JsonBool{object.disable_pinned_message_notifications_});
  jo("disable_mention_notifications", JsonBool{object.disable_mention_notifications_});
}

void to_json(JsonValueScope &jv, const td_api::seconds &object) {
  auto jo = jv.enter_object();
  jo("@type", "seconds");
  jo("seconds", object.seconds_);
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

void to_json(JsonValueScope &jv, const td_api::SecretChatState &object) {
  td_api::downcast_call(const_cast<td_api::SecretChatState &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::secretChatStatePending &object) {
  auto jo = jv.enter_object();
  jo("@type", "secretChatStatePending");
}

void to_json(JsonValueScope &jv, const td_api::secretChatStateReady &object) {
  auto jo = jv.enter_object();
  jo("@type", "secretChatStateReady");
}

void to_json(JsonValueScope &jv, const td_api::secretChatStateClosed &object) {
  auto jo = jv.enter_object();
  jo("@type", "secretChatStateClosed");
}

void to_json(JsonValueScope &jv, const td_api::sentWebAppMessage &object) {
  auto jo = jv.enter_object();
  jo("@type", "sentWebAppMessage");
  jo("inline_message_id", object.inline_message_id_);
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

void to_json(JsonValueScope &jv, const td_api::sessions &object) {
  auto jo = jv.enter_object();
  jo("@type", "sessions");
  jo("sessions", ToJson(object.sessions_));
  jo("inactive_session_ttl_days", object.inactive_session_ttl_days_);
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

void to_json(JsonValueScope &jv, const td_api::shippingOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "shippingOption");
  jo("id", object.id_);
  jo("title", object.title_);
  jo("price_parts", ToJson(object.price_parts_));
}

void to_json(JsonValueScope &jv, const td_api::SpeechRecognitionResult &object) {
  td_api::downcast_call(const_cast<td_api::SpeechRecognitionResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::speechRecognitionResultPending &object) {
  auto jo = jv.enter_object();
  jo("@type", "speechRecognitionResultPending");
  jo("partial_text", object.partial_text_);
}

void to_json(JsonValueScope &jv, const td_api::speechRecognitionResultText &object) {
  auto jo = jv.enter_object();
  jo("@type", "speechRecognitionResultText");
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::speechRecognitionResultError &object) {
  auto jo = jv.enter_object();
  jo("@type", "speechRecognitionResultError");
  if (object.error_) {
    jo("error", ToJson(*object.error_));
  }
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

void to_json(JsonValueScope &jv, const td_api::sponsoredMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "sponsoredMessages");
  jo("messages", ToJson(object.messages_));
  jo("messages_between", object.messages_between_);
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

void to_json(JsonValueScope &jv, const td_api::starGiveawayPaymentOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starGiveawayPaymentOptions");
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::starGiveawayWinnerOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "starGiveawayWinnerOption");
  jo("winner_count", object.winner_count_);
  jo("won_star_count", object.won_star_count_);
  jo("is_default", JsonBool{object.is_default_});
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

void to_json(JsonValueScope &jv, const td_api::starPaymentOptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starPaymentOptions");
  jo("options", ToJson(object.options_));
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

void to_json(JsonValueScope &jv, const td_api::starRevenueStatus &object) {
  auto jo = jv.enter_object();
  jo("@type", "starRevenueStatus");
  jo("total_count", object.total_count_);
  jo("current_count", object.current_count_);
  jo("available_count", object.available_count_);
  jo("withdrawal_enabled", JsonBool{object.withdrawal_enabled_});
  jo("next_withdrawal_in", object.next_withdrawal_in_);
}

void to_json(JsonValueScope &jv, const td_api::starSubscription &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscription");
  jo("id", object.id_);
  jo("chat_id", object.chat_id_);
  jo("expiration_date", object.expiration_date_);
  jo("can_reuse", JsonBool{object.can_reuse_});
  jo("is_canceled", JsonBool{object.is_canceled_});
  jo("is_expiring", JsonBool{object.is_expiring_});
  jo("invite_link", object.invite_link_);
  if (object.pricing_) {
    jo("pricing", ToJson(*object.pricing_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starSubscriptionPricing &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscriptionPricing");
  jo("period", object.period_);
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::starSubscriptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starSubscriptions");
  jo("star_count", object.star_count_);
  jo("subscriptions", ToJson(object.subscriptions_));
  jo("required_star_count", object.required_star_count_);
  jo("next_offset", object.next_offset_);
}

void to_json(JsonValueScope &jv, const td_api::starTransaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransaction");
  jo("id", object.id_);
  jo("star_count", object.star_count_);
  jo("is_refund", JsonBool{object.is_refund_});
  jo("date", object.date_);
  if (object.partner_) {
    jo("partner", ToJson(*object.partner_));
  }
}

void to_json(JsonValueScope &jv, const td_api::StarTransactionPartner &object) {
  td_api::downcast_call(const_cast<td_api::StarTransactionPartner &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerTelegram &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerTelegram");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerAppStore &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerAppStore");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerGooglePlay &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerGooglePlay");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerFragment &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerFragment");
  if (object.withdrawal_state_) {
    jo("withdrawal_state", ToJson(*object.withdrawal_state_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerTelegramAds &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerTelegramAds");
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerBot");
  jo("user_id", object.user_id_);
  if (object.purpose_) {
    jo("purpose", ToJson(*object.purpose_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerBusiness &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerBusiness");
  jo("user_id", object.user_id_);
  jo("media", ToJson(object.media_));
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerChat");
  jo("chat_id", object.chat_id_);
  if (object.purpose_) {
    jo("purpose", ToJson(*object.purpose_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerUser");
  jo("user_id", object.user_id_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::starTransactionPartnerUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactionPartnerUnsupported");
}

void to_json(JsonValueScope &jv, const td_api::starTransactions &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransactions");
  jo("star_count", object.star_count_);
  jo("transactions", ToJson(object.transactions_));
  jo("next_offset", object.next_offset_);
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

void to_json(JsonValueScope &jv, const td_api::statisticalValue &object) {
  auto jo = jv.enter_object();
  jo("@type", "statisticalValue");
  jo("value", object.value_);
  jo("previous_value", object.previous_value_);
  jo("growth_rate_percentage", object.growth_rate_percentage_);
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
  jo("outline", ToJson(object.outline_));
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
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

void to_json(JsonValueScope &jv, const td_api::stickerSet &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerSet");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("title", object.title_);
  jo("name", object.name_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("thumbnail_outline", ToJson(object.thumbnail_outline_));
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

void to_json(JsonValueScope &jv, const td_api::stickerSetInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerSetInfo");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("title", object.title_);
  jo("name", object.name_);
  if (object.thumbnail_) {
    jo("thumbnail", ToJson(*object.thumbnail_));
  }
  jo("thumbnail_outline", ToJson(object.thumbnail_outline_));
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
  jo("size", object.size_);
  jo("covers", ToJson(object.covers_));
}

void to_json(JsonValueScope &jv, const td_api::stickerSets &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickerSets");
  jo("total_count", object.total_count_);
  jo("sets", ToJson(object.sets_));
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

void to_json(JsonValueScope &jv, const td_api::stickers &object) {
  auto jo = jv.enter_object();
  jo("@type", "stickers");
  jo("stickers", ToJson(object.stickers_));
}

void to_json(JsonValueScope &jv, const td_api::storageStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "storageStatistics");
  jo("size", object.size_);
  jo("count", object.count_);
  jo("by_chat", ToJson(object.by_chat_));
}

void to_json(JsonValueScope &jv, const td_api::storageStatisticsByChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "storageStatisticsByChat");
  jo("chat_id", object.chat_id_);
  jo("size", object.size_);
  jo("count", object.count_);
  jo("by_file_type", ToJson(object.by_file_type_));
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

void to_json(JsonValueScope &jv, const td_api::storageStatisticsFast &object) {
  auto jo = jv.enter_object();
  jo("@type", "storageStatisticsFast");
  jo("files_size", object.files_size_);
  jo("file_count", object.file_count_);
  jo("database_size", object.database_size_);
  jo("language_pack_database_size", object.language_pack_database_size_);
  jo("log_size", object.log_size_);
}

void to_json(JsonValueScope &jv, const td_api::stories &object) {
  auto jo = jv.enter_object();
  jo("@type", "stories");
  jo("total_count", object.total_count_);
  jo("stories", ToJson(object.stories_));
  jo("pinned_story_ids", ToJson(object.pinned_story_ids_));
}

void to_json(JsonValueScope &jv, const td_api::story &object) {
  auto jo = jv.enter_object();
  jo("@type", "story");
  jo("id", object.id_);
  jo("sender_chat_id", object.sender_chat_id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  jo("date", object.date_);
  jo("is_being_sent", JsonBool{object.is_being_sent_});
  jo("is_being_edited", JsonBool{object.is_being_edited_});
  jo("is_edited", JsonBool{object.is_edited_});
  jo("is_posted_to_chat_page", JsonBool{object.is_posted_to_chat_page_});
  jo("is_visible_only_for_self", JsonBool{object.is_visible_only_for_self_});
  jo("can_be_deleted", JsonBool{object.can_be_deleted_});
  jo("can_be_edited", JsonBool{object.can_be_edited_});
  jo("can_be_forwarded", JsonBool{object.can_be_forwarded_});
  jo("can_be_replied", JsonBool{object.can_be_replied_});
  jo("can_toggle_is_posted_to_chat_page", JsonBool{object.can_toggle_is_posted_to_chat_page_});
  jo("can_get_statistics", JsonBool{object.can_get_statistics_});
  jo("can_get_interactions", JsonBool{object.can_get_interactions_});
  jo("has_expired_viewers", JsonBool{object.has_expired_viewers_});
  if (object.repost_info_) {
    jo("repost_info", ToJson(*object.repost_info_));
  }
  if (object.interaction_info_) {
    jo("interaction_info", ToJson(*object.interaction_info_));
  }
  if (object.chosen_reaction_type_) {
    jo("chosen_reaction_type", ToJson(*object.chosen_reaction_type_));
  }
  if (object.privacy_settings_) {
    jo("privacy_settings", ToJson(*object.privacy_settings_));
  }
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
  jo("areas", ToJson(object.areas_));
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
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

void to_json(JsonValueScope &jv, const td_api::storyInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInfo");
  jo("story_id", object.story_id_);
  jo("date", object.date_);
  jo("is_for_close_friends", JsonBool{object.is_for_close_friends_});
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

void to_json(JsonValueScope &jv, const td_api::storyInteractionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteractionInfo");
  jo("view_count", object.view_count_);
  jo("forward_count", object.forward_count_);
  jo("reaction_count", object.reaction_count_);
  jo("recent_viewer_user_ids", ToJson(object.recent_viewer_user_ids_));
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

void to_json(JsonValueScope &jv, const td_api::storyInteractions &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteractions");
  jo("total_count", object.total_count_);
  jo("total_forward_count", object.total_forward_count_);
  jo("total_reaction_count", object.total_reaction_count_);
  jo("interactions", ToJson(object.interactions_));
  jo("next_offset", object.next_offset_);
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
  jo("sender_name", object.sender_name_);
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

void to_json(JsonValueScope &jv, const td_api::storyRepostInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyRepostInfo");
  if (object.origin_) {
    jo("origin", ToJson(*object.origin_));
  }
  jo("is_content_modified", JsonBool{object.is_content_modified_});
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

void to_json(JsonValueScope &jv, const td_api::SuggestedAction &object) {
  td_api::downcast_call(const_cast<td_api::SuggestedAction &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionEnableArchiveAndMuteNewChats &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionEnableArchiveAndMuteNewChats");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionCheckPassword &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionCheckPassword");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionCheckPhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionCheckPhoneNumber");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionViewChecksHint &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionViewChecksHint");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionConvertToBroadcastGroup &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionConvertToBroadcastGroup");
  jo("supergroup_id", object.supergroup_id_);
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionSetPassword &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionSetPassword");
  jo("authorization_delay", object.authorization_delay_);
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionUpgradePremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionUpgradePremium");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionRestorePremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionRestorePremium");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionSubscribeToAnnualPremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionSubscribeToAnnualPremium");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionGiftPremiumForChristmas &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionGiftPremiumForChristmas");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionSetBirthdate &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionSetBirthdate");
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionExtendPremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionExtendPremium");
  jo("manage_premium_subscription_url", object.manage_premium_subscription_url_);
}

void to_json(JsonValueScope &jv, const td_api::suggestedActionExtendStarSubscriptions &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionExtendStarSubscriptions");
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
  jo("is_verified", JsonBool{object.is_verified_});
  jo("has_sensitive_content", JsonBool{object.has_sensitive_content_});
  jo("restriction_reason", object.restriction_reason_);
  jo("is_scam", JsonBool{object.is_scam_});
  jo("is_fake", JsonBool{object.is_fake_});
  jo("has_active_stories", JsonBool{object.has_active_stories_});
  jo("has_unread_active_stories", JsonBool{object.has_unread_active_stories_});
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
  jo("slow_mode_delay", object.slow_mode_delay_);
  jo("slow_mode_delay_expires_in", object.slow_mode_delay_expires_in_);
  jo("can_enable_paid_reaction", JsonBool{object.can_enable_paid_reaction_});
  jo("can_get_members", JsonBool{object.can_get_members_});
  jo("has_hidden_members", JsonBool{object.has_hidden_members_});
  jo("can_hide_members", JsonBool{object.can_hide_members_});
  jo("can_set_sticker_set", JsonBool{object.can_set_sticker_set_});
  jo("can_set_location", JsonBool{object.can_set_location_});
  jo("can_get_statistics", JsonBool{object.can_get_statistics_});
  jo("can_get_revenue_statistics", JsonBool{object.can_get_revenue_statistics_});
  jo("can_get_star_revenue_statistics", JsonBool{object.can_get_star_revenue_statistics_});
  jo("can_toggle_aggressive_anti_spam", JsonBool{object.can_toggle_aggressive_anti_spam_});
  jo("is_all_history_available", JsonBool{object.is_all_history_available_});
  jo("can_have_sponsored_messages", JsonBool{object.can_have_sponsored_messages_});
  jo("has_aggressive_anti_spam_enabled", JsonBool{object.has_aggressive_anti_spam_enabled_});
  jo("has_paid_media_allowed", JsonBool{object.has_paid_media_allowed_});
  jo("has_pinned_stories", JsonBool{object.has_pinned_stories_});
  jo("my_boost_count", object.my_boost_count_);
  jo("unrestrict_boost_count", object.unrestrict_boost_count_);
  jo("sticker_set_id", ToJson(JsonInt64{object.sticker_set_id_}));
  jo("custom_emoji_sticker_set_id", ToJson(JsonInt64{object.custom_emoji_sticker_set_id_}));
  if (object.location_) {
    jo("location", ToJson(*object.location_));
  }
  if (object.invite_link_) {
    jo("invite_link", ToJson(*object.invite_link_));
  }
  jo("bot_commands", ToJson(object.bot_commands_));
  jo("upgraded_from_basic_group_id", object.upgraded_from_basic_group_id_);
  jo("upgraded_from_max_message_id", object.upgraded_from_max_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::tMeUrl &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrl");
  jo("url", object.url_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
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

void to_json(JsonValueScope &jv, const td_api::tMeUrls &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrls");
  jo("urls", ToJson(object.urls_));
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
  jo("allow_user_chats", JsonBool{object.allow_user_chats_});
  jo("allow_bot_chats", JsonBool{object.allow_bot_chats_});
  jo("allow_group_chats", JsonBool{object.allow_group_chats_});
  jo("allow_channel_chats", JsonBool{object.allow_channel_chats_});
}

void to_json(JsonValueScope &jv, const td_api::targetChatInternalLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "targetChatInternalLink");
  if (object.link_) {
    jo("link", ToJson(*object.link_));
  }
}

void to_json(JsonValueScope &jv, const td_api::temporaryPasswordState &object) {
  auto jo = jv.enter_object();
  jo("@type", "temporaryPasswordState");
  jo("has_password", JsonBool{object.has_password_});
  jo("valid_for", object.valid_for_);
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

void to_json(JsonValueScope &jv, const td_api::testBytes &object) {
  auto jo = jv.enter_object();
  jo("@type", "testBytes");
  jo("value", base64_encode(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::testInt &object) {
  auto jo = jv.enter_object();
  jo("@type", "testInt");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::testString &object) {
  auto jo = jv.enter_object();
  jo("@type", "testString");
  jo("value", object.value_);
}

void to_json(JsonValueScope &jv, const td_api::testVectorInt &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorInt");
  jo("value", ToJson(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::testVectorIntObject &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorIntObject");
  jo("value", ToJson(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::testVectorString &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorString");
  jo("value", ToJson(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::testVectorStringObject &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorStringObject");
  jo("value", ToJson(object.value_));
}

void to_json(JsonValueScope &jv, const td_api::text &object) {
  auto jo = jv.enter_object();
  jo("@type", "text");
  jo("text", object.text_);
}

void to_json(JsonValueScope &jv, const td_api::textEntities &object) {
  auto jo = jv.enter_object();
  jo("@type", "textEntities");
  jo("entities", ToJson(object.entities_));
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

void to_json(JsonValueScope &jv, const td_api::textQuote &object) {
  auto jo = jv.enter_object();
  jo("@type", "textQuote");
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("position", object.position_);
  jo("is_manual", JsonBool{object.is_manual_});
}

void to_json(JsonValueScope &jv, const td_api::themeSettings &object) {
  auto jo = jv.enter_object();
  jo("@type", "themeSettings");
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

void to_json(JsonValueScope &jv, const td_api::thumbnail &object) {
  auto jo = jv.enter_object();
  jo("@type", "thumbnail");
  if (object.format_) {
    jo("format", ToJson(*object.format_));
  }
  jo("width", object.width_);
  jo("height", object.height_);
  if (object.file_) {
    jo("file", ToJson(*object.file_));
  }
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

void to_json(JsonValueScope &jv, const td_api::timeZone &object) {
  auto jo = jv.enter_object();
  jo("@type", "timeZone");
  jo("id", object.id_);
  jo("name", object.name_);
  jo("utc_time_offset", object.utc_time_offset_);
}

void to_json(JsonValueScope &jv, const td_api::timeZones &object) {
  auto jo = jv.enter_object();
  jo("@type", "timeZones");
  jo("time_zones", ToJson(object.time_zones_));
}

void to_json(JsonValueScope &jv, const td_api::trendingStickerSets &object) {
  auto jo = jv.enter_object();
  jo("@type", "trendingStickerSets");
  jo("total_count", object.total_count_);
  jo("sets", ToJson(object.sets_));
  jo("is_premium", JsonBool{object.is_premium_});
}

void to_json(JsonValueScope &jv, const td_api::unconfirmedSession &object) {
  auto jo = jv.enter_object();
  jo("@type", "unconfirmedSession");
  jo("id", ToJson(JsonInt64{object.id_}));
  jo("log_in_date", object.log_in_date_);
  jo("device_model", object.device_model_);
  jo("location", object.location_);
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

void to_json(JsonValueScope &jv, const td_api::updateMessageLiveLocationViewed &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateMessageLiveLocationViewed");
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
  jo("theme_name", object.theme_name_);
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
  jo("chat_id", object.chat_id_);
  if (object.info_) {
    jo("info", ToJson(*object.info_));
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
  jo("message_thread_id", object.message_thread_id_);
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.action_) {
    jo("action", ToJson(*object.action_));
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
  jo("story_sender_chat_id", object.story_sender_chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateStorySendSucceeded &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStorySendSucceeded");
  if (object.story_) {
    jo("story", ToJson(*object.story_));
  }
  jo("old_story_id", object.old_story_id_);
}

void to_json(JsonValueScope &jv, const td_api::updateStorySendFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateStorySendFailed");
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

void to_json(JsonValueScope &jv, const td_api::updateChatThemes &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatThemes");
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

void to_json(JsonValueScope &jv, const td_api::updateTermsOfService &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateTermsOfService");
  jo("terms_of_service_id", object.terms_of_service_id_);
  if (object.terms_of_service_) {
    jo("terms_of_service", ToJson(*object.terms_of_service_));
  }
}

void to_json(JsonValueScope &jv, const td_api::updateUsersNearby &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateUsersNearby");
  jo("users_nearby", ToJson(object.users_nearby_));
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
  jo("star_count", object.star_count_);
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

void to_json(JsonValueScope &jv, const td_api::updates &object) {
  auto jo = jv.enter_object();
  jo("@type", "updates");
  jo("updates", ToJson(object.updates_));
}

void to_json(JsonValueScope &jv, const td_api::user &object) {
  auto jo = jv.enter_object();
  jo("@type", "user");
  jo("id", object.id_);
  jo("first_name", object.first_name_);
  jo("last_name", object.last_name_);
  if (object.usernames_) {
    jo("usernames", ToJson(*object.usernames_));
  }
  jo("phone_number", object.phone_number_);
  if (object.status_) {
    jo("status", ToJson(*object.status_));
  }
  if (object.profile_photo_) {
    jo("profile_photo", ToJson(*object.profile_photo_));
  }
  jo("accent_color_id", object.accent_color_id_);
  jo("background_custom_emoji_id", ToJson(JsonInt64{object.background_custom_emoji_id_}));
  jo("profile_accent_color_id", object.profile_accent_color_id_);
  jo("profile_background_custom_emoji_id", ToJson(JsonInt64{object.profile_background_custom_emoji_id_}));
  if (object.emoji_status_) {
    jo("emoji_status", ToJson(*object.emoji_status_));
  }
  jo("is_contact", JsonBool{object.is_contact_});
  jo("is_mutual_contact", JsonBool{object.is_mutual_contact_});
  jo("is_close_friend", JsonBool{object.is_close_friend_});
  jo("is_verified", JsonBool{object.is_verified_});
  jo("is_premium", JsonBool{object.is_premium_});
  jo("is_support", JsonBool{object.is_support_});
  jo("restriction_reason", object.restriction_reason_);
  jo("is_scam", JsonBool{object.is_scam_});
  jo("is_fake", JsonBool{object.is_fake_});
  jo("has_active_stories", JsonBool{object.has_active_stories_});
  jo("has_unread_active_stories", JsonBool{object.has_unread_active_stories_});
  jo("restricts_new_chats", JsonBool{object.restricts_new_chats_});
  jo("have_access", JsonBool{object.have_access_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("language_code", object.language_code_);
  jo("added_to_attachment_menu", JsonBool{object.added_to_attachment_menu_});
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
  jo("premium_gift_options", ToJson(object.premium_gift_options_));
  jo("group_in_common_count", object.group_in_common_count_);
  if (object.business_info_) {
    jo("business_info", ToJson(*object.business_info_));
  }
  if (object.bot_info_) {
    jo("bot_info", ToJson(*object.bot_info_));
  }
}

void to_json(JsonValueScope &jv, const td_api::userLink &object) {
  auto jo = jv.enter_object();
  jo("@type", "userLink");
  jo("url", object.url_);
  jo("expires_in", object.expires_in_);
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

void to_json(JsonValueScope &jv, const td_api::userPrivacySettingRules &object) {
  auto jo = jv.enter_object();
  jo("@type", "userPrivacySettingRules");
  jo("rules", ToJson(object.rules_));
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

void to_json(JsonValueScope &jv, const td_api::userSupportInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "userSupportInfo");
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("author", object.author_);
  jo("date", object.date_);
}

void to_json(JsonValueScope &jv, const td_api::UserType &object) {
  td_api::downcast_call(const_cast<td_api::UserType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::userTypeRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "userTypeRegular");
}

void to_json(JsonValueScope &jv, const td_api::userTypeDeleted &object) {
  auto jo = jv.enter_object();
  jo("@type", "userTypeDeleted");
}

void to_json(JsonValueScope &jv, const td_api::userTypeBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "userTypeBot");
  jo("can_be_edited", JsonBool{object.can_be_edited_});
  jo("can_join_groups", JsonBool{object.can_join_groups_});
  jo("can_read_all_group_messages", JsonBool{object.can_read_all_group_messages_});
  jo("has_main_web_app", JsonBool{object.has_main_web_app_});
  jo("is_inline", JsonBool{object.is_inline_});
  jo("inline_query_placeholder", object.inline_query_placeholder_);
  jo("need_location", JsonBool{object.need_location_});
  jo("can_connect_to_business", JsonBool{object.can_connect_to_business_});
  jo("can_be_added_to_attachment_menu", JsonBool{object.can_be_added_to_attachment_menu_});
  jo("active_user_count", object.active_user_count_);
}

void to_json(JsonValueScope &jv, const td_api::userTypeUnknown &object) {
  auto jo = jv.enter_object();
  jo("@type", "userTypeUnknown");
}

void to_json(JsonValueScope &jv, const td_api::usernames &object) {
  auto jo = jv.enter_object();
  jo("@type", "usernames");
  jo("active_usernames", ToJson(object.active_usernames_));
  jo("disabled_usernames", ToJson(object.disabled_usernames_));
  jo("editable_username", object.editable_username_);
}

void to_json(JsonValueScope &jv, const td_api::users &object) {
  auto jo = jv.enter_object();
  jo("@type", "users");
  jo("total_count", object.total_count_);
  jo("user_ids", ToJson(object.user_ids_));
}

void to_json(JsonValueScope &jv, const td_api::validatedOrderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "validatedOrderInfo");
  jo("order_info_id", object.order_info_id_);
  jo("shipping_options", ToJson(object.shipping_options_));
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

void to_json(JsonValueScope &jv, const td_api::videoChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoChat");
  jo("group_call_id", object.group_call_id_);
  jo("has_participants", JsonBool{object.has_participants_});
  if (object.default_participant_id_) {
    jo("default_participant_id", ToJson(*object.default_participant_id_));
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

void to_json(JsonValueScope &jv, const td_api::webAppInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "webAppInfo");
  jo("launch_id", ToJson(JsonInt64{object.launch_id_}));
  jo("url", object.url_);
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
